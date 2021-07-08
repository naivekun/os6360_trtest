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
* @file prvTgfTunnelTermTtiDsaMetadata.c
*
* @brief Verify the functionality of Tunnel Term TTI Dsa Metadata
*
* @version   4
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

#include <tunnel/prvTgfTunnelTermTtiDsaMetadata.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   0
/* port index to receive  traffic n */
#define PRV_TGF_EGR_PORT_IDX_CNS   3

/* number of ports in VLans */
#define PRV_TGF_RX_SIDE_PORTS_COUNT_CNS 2
#define PRV_TGF_TX_SIDE_PORTS_COUNT_CNS 2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS 4

/* send VLAN */
#define PRV_TGF_VLANID_5_CNS          5
/* receive VLAN id */
#define PRV_TGF_VLANID_6_CNS          6

/* the TTI Rule index */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3)

/* DSA Tag fields to test */
#define PRV_TGF_DSA_PORT_CNS            0x1
#define PRV_TGF_DSA_DEV_CNS             0x2
#define PRV_TGF_DSA_SOURCE_ID_CNS       0x3
#define PRV_TGF_DSA_SOURCE_IS_TRUNK_CNS 1 << 6

/* offsets for testing DSA fields */
static GT_U8 portAnchorOffset          = 28;
static GT_U8 deviceAnchorOffset        = 30;
static GT_U8 sourceIdAnchorOffset      = 18;
static GT_U8 sourceIsTrunkAnchorOffset = 22;

/* number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

static PRV_TGF_TTI_ACTION_2_STC ttiAction;
static PRV_TGF_TTI_RULE_UNT     ttiPattern;
static PRV_TGF_TTI_RULE_UNT     ttiMask;

/* Parameters needed to be restored */
static GT_U32                            portsArrayForRestore[PRV_TGF_MAX_PORTS_NUM_CNS];
/************************* General packet's parts *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}                 /* saMac */
};

static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    0,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/ /* union fields are set in runtime due to 'Big endian' / 'little endian' issues */
    {
        /*trunkId*/
        0/*portNum*/
    },/*source;*/

    0,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            0/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    0,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */ /* union fields are set in runtime due to 'Big endian' / 'little endian' issues */
    {
        /*trunkId*/
        0/*portNum*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        0,/*vid --> filled in runtime */
        GT_FALSE,/*dropOnSource*/
        GT_FALSE/*packetIsLooped*/
    }/*commonParams*/

/*    union{
        TGF_DSA_DSA_TO_CPU_STC         toCpu;
        TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
        TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
        TGF_DSA_DSA_FORWARD_STC        forward;
    }dsaInfo;*/
};


/* Data of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermTtiDsaMetadataBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
static GT_VOID prvTgfTunnelTermTtiDsaMetadataBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;
    GT_U8     tagArray[] = {1, 1, 1, 1};

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with daMac, vlanId, send port */
     rc = prvTgfBrgDefFdbMacEntryOnPortSet(
         prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_5_CNS,
         prvTgfDevNum,
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
         GT_TRUE);

     UTF_VERIFY_EQUAL1_STRING_MAC(
         GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with daMac, vlanId, receive port */
     rc = prvTgfBrgDefFdbMacEntryOnPortSet(
         prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_6_CNS,
         prvTgfDevNum,
         prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
         GT_TRUE);

     UTF_VERIFY_EQUAL1_STRING_MAC(
         GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

     /* AUTODOC:set ingress port as cascade port */
     rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
         prvTgfDevNum,
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
         GT_TRUE);
     UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}

/**
* @internal prvTgfTunnelTermTtiDsaMetadataTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfTunnelTermTtiDsaMetadataTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    /* AUTODOC: set UDB byte for the key keyType: anchor Metadata, offset 22 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                     PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                     0,
                                     PRV_TGF_TTI_OFFSET_METADATA_E,
                                     portAnchorOffset);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: set the TTI Rule Pattern and Mask to match no packets */
    ttiPattern.udbArray[0] = 1 + PRV_TGF_DSA_PORT_CNS;
    ttiMask.udbArray[0]    = 0xFF;

    ttiAction.command                           = CPSS_PACKET_CMD_DROP_HARD_E;

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E on port 0 VLAN 5 with action: HARD DROP */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                       &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal prvTgfTunnelTermTtiDsaMetadataTtiPatternSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] dsaField                 - sets  to match
* @param[in] anchorOffset             - sets anchor offset for metadata fields
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfTunnelTermTtiDsaMetadataTtiPatternSet
(
    GT_U8 dsaField,
    GT_U8 anchorOffset
)
{
    GT_STATUS rc = GT_FAIL;

    PRV_UTF_LOG0_MAC("======= Setting TTI Pattern Configuration =======\n");

    ttiPattern.udbArray[0] = dsaField;

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E: anchor Metadata, offset <anchorOffset> */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                     PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                     0,
                                     PRV_TGF_TTI_OFFSET_METADATA_E,
                                     anchorOffset);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                       &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal prvTgfTunnelTermTtiDsaMetadataTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
* @param[in] srcIsTrunk               - DSA tag fields Source Is Trunk
*                                       None
*/
static GT_VOID prvTgfTunnelTermTtiDsaMetadataTrafficGenerate
(
    IN GT_BOOL  expectTraffic,
    IN GT_BOOL  srcIsTrunk
)
{
    GT_STATUS                       rc          = GT_FAIL;
    GT_U32                          portIter    = 0;
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

    prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    prvTgfPacketDsaTagPart_forward.dstInterface.devPort.portNum =
                                     prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart_forward.dstEport =  prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* NOTE: the 'Big Endian' is building the 'unions' in 'occurred' way ,
        so we must EXPLICITLY set 'union fields' to needed value */
    prvTgfPacketDsaTagPart_forward.source.portNum = PRV_TGF_DSA_PORT_CNS;
    prvTgfPacketDsaTagPart_forward.srcHwDev       = PRV_TGF_DSA_DEV_CNS;
    prvTgfPacketDsaTagPart_forward.srcId          = PRV_TGF_DSA_SOURCE_ID_CNS;
    prvTgfPacketDsaTagPart_forward.srcIsTrunk     = srcIsTrunk;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = 0;

    /* do this right before the send of packet */
    /* bind the DSA tag FORWARD part (since union is used .. can't do it in compilation time) */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    /*  Reset all Ethernet port's counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */
    numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;

    /* AUTODOC: send 1 ethernet packet from port 0 with: VID=5 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfPacketInfo, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        switch(portIter)
        {
           case PRV_TGF_SEND_PORT_IDX_CNS:
                numOfExpectedRxPackets = prvTgfBurstCount;
                break;
           case PRV_TGF_EGR_PORT_IDX_CNS:
                numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;
                break;
            default:
                numOfExpectedRxPackets = 0;
        }

        /* AUTODOC: verify counters */
        UTF_VERIFY_EQUAL3_STRING_MAC(numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0],
            "got wrong counters: port[%d], expected [%d], received[%d]\n",
             prvTgfPortsArray[portIter], numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0]);
    }
}

/**
* @internal prvTgfTunnelTermTtiDsaMetadataConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
static GT_VOID prvTgfTunnelTermTtiDsaMetadataConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;
    GT_U32    i = 0;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_MPLS */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC:set ingress port as cascade port */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    /* AUTODOC: Remove VLAN 5 members */
    for (i = 0; i < PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_5_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLANID_5_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: Remove VLAN 6 members */
    for (i = PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i < PRV_TGF_PORT_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_6_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLANID_6_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfTunnelTermTtiDsaMetadata function
* @endinternal
*
* @brief   Test of Tunnel Term TTI Dsa Metadata (main func)
*/
GT_VOID prvTgfTunnelTermTtiDsaMetadata
(
    GT_VOID
)
{
    /* AUTODOC: build ethernet dsa-tagged packet */

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(portsArrayForRestore, prvTgfPortsArray,
                 sizeof(portsArrayForRestore));
    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_SEND_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* AUTODOC: create VLAN 5 with tagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with tagged ports [2,3] */
    prvTgfTunnelTermTtiDsaMetadataBridgeConfigSet();

    /* AUTODOC: add TTI rule 1 with command HARD DROP and Pattern with invalid udbArray */
    prvTgfTunnelTermTtiDsaMetadataTtiConfigSet();

    /*------------------------------------------------------------*/
    /* AUTODOC: send packet, expect no TTI match, and traffic on port */
    PRV_UTF_LOG0_MAC("===== CASE 1: Send packet with no TTI match. Show the packet is received on port ====\n");
    prvTgfTunnelTermTtiDsaMetadataTrafficGenerate(GT_TRUE, GT_FALSE);

    /*------------------------------------------------------------*/
    /* AUTODOC: set TTI rule to match metadata SrcPort */
    /* AUTODOC: send packet, expect TTI match, and packet dropped */
    PRV_UTF_LOG0_MAC("===== CASE 2: Test of DSA SrcPort/SRCePortPORT/TrunkId ====\n");
    prvTgfTunnelTermTtiDsaMetadataTtiPatternSet(PRV_TGF_DSA_PORT_CNS, portAnchorOffset);
    prvTgfTunnelTermTtiDsaMetadataTrafficGenerate(GT_FALSE, GT_FALSE);

    /*------------------------------------------------------------*/
    /* AUTODOC: set TTI rule to match metadata SrcDev */
    /* AUTODOC: send packet, expect TTI match, and packet dropped */
    PRV_UTF_LOG0_MAC("===== CASE 3: Test of DSA Tag SrcDev ====\n");
    prvTgfTunnelTermTtiDsaMetadataTtiPatternSet(PRV_TGF_DSA_DEV_CNS, deviceAnchorOffset);
    prvTgfTunnelTermTtiDsaMetadataTrafficGenerate(GT_FALSE, GT_FALSE);

    /*------------------------------------------------------------*/
    /* AUTODOC: set TTI rule to match metadata SourceId */
    /* AUTODOC: send packet, expect TTI match, and packet dropped */
    PRV_UTF_LOG0_MAC("===== CASE 4: Test of DSA Source ID ====\n");
    prvTgfTunnelTermTtiDsaMetadataTtiPatternSet(PRV_TGF_DSA_SOURCE_ID_CNS, sourceIdAnchorOffset);
    prvTgfTunnelTermTtiDsaMetadataTrafficGenerate(GT_FALSE, GT_FALSE);

    /*------------------------------------------------------------*/
    /* AUTODOC: set TTI rule to match metadata SourceIsTrunk */
    /* AUTODOC: send packet, expect TTI match, and packet dropped */
    PRV_UTF_LOG0_MAC("===== CASE 5: Test Of DSA Tga Source Is Trunk ====\n");
    prvTgfTunnelTermTtiDsaMetadataTtiPatternSet(PRV_TGF_DSA_SOURCE_IS_TRUNK_CNS, sourceIsTrunkAnchorOffset);
    prvTgfTunnelTermTtiDsaMetadataTrafficGenerate(GT_FALSE, GT_TRUE);

    /*------------------------------------------------------------*/
    /* AUTODOC: test configuration restore */
    prvTgfTunnelTermTtiDsaMetadataConfigRestore();

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, portsArrayForRestore, sizeof(portsArrayForRestore));
}


