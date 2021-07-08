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
* @file prvTgfCscdPortStackAggregation.c
*
* @brief CPSS Cascading
*
* @version   17
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <cscd/prvTgfCscd.h>
#include <cscd/prvTgfCscdPortStackAggregation.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS       4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX0_CNS   0

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX1_CNS   1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX2_CNS   2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX3_CNS   3

/* index of local egress port */
#define LOCAL_EGRESS_PORT_IDX0_CNS   0

/* index of local egress port */
#define LOCAL_EGRESS_PORT_IDX2_CNS   2

/* index of local egress port */
#define LOCAL_EGRESS_PORT_IDX3_CNS   3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* saved VID */
static GT_U16 prvTgfDefVlanId = 0;

#define PRV_TGF_CSCD_TARGET_DEV_NUM 15

#define PRV_TGF_CSCD_TARGET_PORT_NUM 6

#define PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM 28

#define PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM 58

#define PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID 28

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           2

/* array of source MACs for the tests */
static TGF_MAC_ADDR prvTgfSaMacArr[] = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x45},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x56},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x67} };

/* array of destination MACs for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[] = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x45},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x56},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x67}};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x56},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    GT_FALSE , /*srcIsTagged*/
    8,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        5/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            PRV_TGF_CSCD_TARGET_DEV_NUM,/*devNum*/
            PRV_TGF_CSCD_TARGET_PORT_NUM/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPortValid*/
    0,/*dstEport*/
    0,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
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
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_VLANID_CNS,/*vid*/
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

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x81, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* cascaded link number */
#define CSCD_LINK_IDX         3

/* cascaded link number */
#define CSCD_LINK_IDX_ANOTHER 0

/* FDB MAC entry */
static PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;


/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT     lookupMode;
    CPSS_CSCD_LINK_TYPE_STC             cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT    srcPortTrunkHashEn;
    PRV_TGF_TTI_MAC_MODE_ENT            macMode;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfCscdPortStackAggregationFullPathCheckCounters function
* @endinternal
*
* @brief   Function checks MAC counters
*
* @param[in] sendPortIndex            - port to check RX counter
* @param[in] egressPortIndex          - port to check TX counter
*                                      if value is 0xFFFF_FFFF - unknown unicast
*                                      otherwise specific port
*                                       None
*/
static GT_VOID prvTgfCscdPortStackAggregationFullPathCheckCounters
(
    IN GT_U32   sendPortIndex,
    IN GT_U32   egressPortIndex,
    IN GT_BOOL  isMultiDest
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

    /* AUTODOC: verify to get Ethernet packets on all ports with: */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        if(portIter == sendPortIndex)
        {
            /* because of loopback we have 1 packet TX and 1 packet RX */
            expectedCntrs.goodPktsRcv.l[0]   = prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]  = prvTgfBurstCount;
        }
        else if(portIter == egressPortIndex)
        {
            /* because of loopback (port with capture) we have 1 packet TX and 1 packet RX */
            expectedCntrs.goodPktsRcv.l[0]   = prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]  = prvTgfBurstCount;
        }
        else if((isMultiDest == GT_TRUE) && (portIter != egressPortIndex))
        {
            /* ports without capture that transmit multidestination traffic */
            expectedCntrs.goodPktsRcv.l[0]   = 0;
            expectedCntrs.goodPktsSent.l[0]  = prvTgfBurstCount;
        }
        else
        {
            expectedCntrs.goodPktsRcv.l[0]   = 0;
            expectedCntrs.goodPktsSent.l[0]   = 0;
        }

        isOk =
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");

            PRV_UTF_LOG0_MAC("Current values:\n");
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", portCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", portCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }
}


/**
* @internal prvTgfCscdPortStackAggregationFullPathTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] packetInfoPtr            - PACKET to send
* @param[in] vfdNum                   - VDF number
* @param[in] vfdArray[]               - VDF array
*                                       None
*/
static GT_VOID prvTgfCscdPortStackAggregationFullPathTestPacketSend
(
    IN GT_U32                   sendPortIndex,
    IN GT_U32                   egressPortIndex,
    IN TGF_PACKET_STC          *packetInfoPtr,
    IN GT_U8                    vfdNum,
    IN TGF_VFD_INFO_STC         vfdArray[]
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U32                      numTriggers = 0;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_TRUE);

    /* AUTODOC: send Ethernet packet from port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:00:56, SA=00:00:00:00:00:02, VID=5 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, vfdNum, vfdArray, &numTriggers);
    rc = (rc == GT_NO_MORE) ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* check if captured packet has the same pattern as vfdArray */
    UTF_VERIFY_EQUAL0_STRING_MAC((1 << vfdNum) - 1, numTriggers,
                                 "Packet DSA TAG pattern is wrong");

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}


/**
* @internal prvTgfCscdPortStackAggregationFullTestConfig function
* @endinternal
*
* @brief   Function configures stack aggregation parameters per iteration.
*/
static GT_VOID prvTgfCscdPortStackAggregationFullTestConfig
(
    GT_U32                  portIndex
)
{
    GT_STATUS                   rc;             /* return code */

    /* AUTODOC: enable for port[portIndex] stack aggregation */
    rc = prvTgfCscdPortStackAggregationEnableSet(prvTgfDevNum, prvTgfPortsArray[portIndex], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortStackAggregationEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIndex]);

    /* AUTODOC: set for port[egressPortIndex] stack aggregation configuration: dev, port, srcId */
    rc = prvTgfCscdPortStackAggregationConfigSet(prvTgfDevNum,
                                                 prvTgfPortsArray[portIndex],
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM,
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM,
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfCscdPortStackAggregationConfigSet: %d %d %d %d %d",
                                            prvTgfDevNum,
                                            prvTgfPortsArray[portIndex],
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM,
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM,
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID);

}

/**
* @internal prvTgfCscdPortStackAggregationFullPathTtiRuleConfig function
* @endinternal
*
* @brief   Function configures TTI rule.
*/
static GT_VOID prvTgfCscdPortStackAggregationFullPathTtiRuleConfig
(
    IN  GT_U32                  portIndex,
    IN  CPSS_INTERFACE_TYPE_ENT interfaceType
)
{
    GT_STATUS                   rc;             /* return code */
    PRV_TGF_TTI_ACTION_STC      ttiAction;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;
    GT_U32                      ttiIndex;


    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[portIndex], PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: port = %d", prvTgfPortsArray[portIndex]);

    /* save the current lookup Mac mode for Ethernet and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* set the TTI Rule Pattern, Mask and Action for Ether Type Key */

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet((GT_VOID*) &ttiPattern,0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,   0, sizeof(ttiMask));

    /* set TTI Action */
    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = PRV_TGF_TTI_LOGICAL_PORT_ASSIGN_E;
    ttiAction.tunnelTerminate               = GT_FALSE;
    ttiAction.vrfId                         = 0;
    ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ttiAction.passengerPacketType           = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ttiAction.copyTtlFromTunnelHeader       = GT_FALSE;
    ttiAction.mirrorToIngressAnalyzerEnable = GT_FALSE;
    ttiAction.policerIndex                  = 0;
    ttiAction.interfaceInfo.type            = interfaceType;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[portIndex];
    ttiAction.vlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    ttiAction.vlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.vlanId                        = 0;
    ttiAction.nestedVlanEnable              = GT_FALSE;
    ttiAction.tunnelStart                   = GT_FALSE;
    ttiAction.tunnelStartPtr                = 0;
    ttiAction.routerLookupPtr               = 0;
    ttiAction.sourceIdSetEnable             = GT_FALSE;
    ttiAction.sourceId                      = 0;
    ttiAction.bindToPolicer                 = GT_FALSE;
    ttiAction.qosPrecedence                 = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.qosTrustMode                  = PRV_TGF_TTI_QOS_KEEP_PREVIOUS_E;
    ttiAction.qosProfile                    = 0;
    ttiAction.modifyUpEnable                = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
    ttiAction.modifyDscpEnable              = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
    ttiAction.up                            = 0;
    ttiAction.remapDSCP                     = GT_FALSE;
    ttiAction.vntl2Echo                     = GT_FALSE;
    ttiAction.bridgeBypass                  = GT_FALSE;
    ttiAction.actionStop                    = GT_FALSE;
    ttiAction.activateCounter               = GT_FALSE;
    ttiAction.counterIndex                  = 0;

    /* set TTI Pattern */
    ttiPattern.eth.common.pclId    = 3;
    ttiPattern.eth.common.srcIsTrunk = (interfaceType == CPSS_INTERFACE_TRUNK_E) ? GT_TRUE : GT_FALSE;
    ttiPattern.eth.common.srcPortTrunk = prvTgfPortsArray[portIndex];

    /* set TTI Mask */
    cpssOsMemSet((GT_VOID*) &(ttiMask.eth.common.pclId), 0xFF,
                 sizeof(ttiMask.eth.common.pclId));
    cpssOsMemSet((GT_VOID*) &(ttiMask.eth.common.srcPortTrunk), 0xFF,
                 sizeof(ttiMask.eth.common.srcPortTrunk));
    ttiMask.eth.common.srcIsTrunk = GT_TRUE;

    ttiIndex = (interfaceType == CPSS_INTERFACE_PORT_E) ? portIndex : (portIndex + 20);

    rc = prvTgfTtiRuleSet(ttiIndex, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

}


/**
* @internal prvTgfCscdPortStackAggregationTestConfig function
* @endinternal
*
* @brief   Function configures stack aggregation parameters per iteration.
*/
static GT_VOID prvTgfCscdPortStackAggregationTestConfig
(
    GT_U32 egressPortIndex
)
{
    GT_STATUS                   rc;             /* return code */
    CPSS_CSCD_LINK_TYPE_STC     cascadeLink;    /* cascade link */

    /* AUTODOC: set port[egressPortIndex] to cascaded mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[egressPortIndex], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIndex]);

    /* AUTODOC: enable for port[egressPortIndex] stack aggregation */
    rc = prvTgfCscdPortStackAggregationEnableSet(prvTgfDevNum, prvTgfPortsArray[egressPortIndex], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortStackAggregationEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIndex]);

    /* AUTODOC: set for port[egressPortIndex] stack aggregation configuration: dev, port, srcId */
    rc = prvTgfCscdPortStackAggregationConfigSet(prvTgfDevNum,
                                                 prvTgfPortsArray[egressPortIndex],
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM,
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM,
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfCscdPortStackAggregationConfigSet: %d %d %d %d %d",
                                            prvTgfDevNum,
                                            prvTgfPortsArray[egressPortIndex],
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM,
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM,
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID);

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_CSCD_TARGET_DEV_NUM, PRV_TGF_CSCD_TARGET_PORT_NUM,
                                  &(prvTgfRestoreCfg.cascadeLink), &(prvTgfRestoreCfg.srcPortTrunkHashEn));
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_CSCD_TARGET_DEV_NUM, PRV_TGF_CSCD_TARGET_PORT_NUM);

    /* Set the cascade map table */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[egressPortIndex];

    /* AUTODOC: config CSCD map table with: */
    /* AUTODOC:   trgDevNum=15, trgPortNum=6, */
    /* AUTODOC:   cscdLink linkType=PORT, port[egressPortIndex] */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_CSCD_TARGET_DEV_NUM,
                                  PRV_TGF_CSCD_TARGET_PORT_NUM, &cascadeLink, 0, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d%d%d%d",
                                 PRV_TGF_CSCD_TARGET_DEV_NUM, PRV_TGF_CSCD_TARGET_PORT_NUM,
                                 CPSS_CSCD_LINK_TYPE_PORT_E, cascadeLink.linkNum);

}

/**
* @internal prvTgfCscdPortStackAggregationTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] packetInfoPtr            - PACKET to send
* @param[in] vfdNum                   - VDF number
* @param[in] vfdArray[]               - VDF array
*                                       None
*/
static GT_VOID prvTgfCscdPortStackAggregationTestPacketSend
(
    IN GT_U32                   egressPortIndex,
    IN TGF_PACKET_STC          *packetInfoPtr,
    IN GT_U8                    vfdNum,
    IN TGF_VFD_INFO_STC         vfdArray[]
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U32                      numTriggers = 0;

    if(egressPortIndex == PRV_TGF_SEND_PORT_IDX0_CNS)
    {
        return;
    }

    prvTgfCscdPortStackAggregationTestConfig(egressPortIndex);

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_TRUE);

    /* AUTODOC: send Ethernet packet from port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:00:56, SA=00:00:00:00:00:02, VID=5 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, vfdNum, vfdArray, &numTriggers);
    rc = (rc == GT_NO_MORE) ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* check if captured packet has the same pattern as vfdArray */
    UTF_VERIFY_EQUAL0_STRING_MAC((1 << vfdNum) - 1, numTriggers,
                                 "Packet DSA TAG pattern is wrong");

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfCscdPortStackAggregationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set FDB entry with device number 31, port 27, MAC 00:00:00:00: 00:56.
*         - Set lookup mode to Port for accessing the Device Map table.
*         - Set the cascade map table
*         - Target device 0, target port 0, link type port, link number 23.
*/
GT_VOID prvTgfCscdPortStackAggregationConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                 rc = GT_OK;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* set unioun fields to avoid LE/BE bugs */
    prvTgfPacketDsaTagPart_forward.source.portNum = 5;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set port[0] to cascaded mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);

    /* AUTODOC: enable for port[0] stack aggregation */
    rc = prvTgfCscdPortStackAggregationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortStackAggregationEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);

    /* AUTODOC: set for port[0] stack aggregation configuration: dev, port, srcId */
    rc = prvTgfCscdPortStackAggregationConfigSet(prvTgfDevNum,
                                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS],
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM,
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM,
                                                 PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfCscdPortStackAggregationConfigSet: %d %d %d %d %d",
                                            prvTgfDevNum,
                                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS],
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM,
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM,
                                            PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID);

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* Set FDB entry with device number 15, port 6,  MAC 00:00:00:00: 00:56 */
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.isStatic = GT_TRUE;
    prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum = PRV_TGF_CSCD_TARGET_DEV_NUM;
    prvTgfMacEntry.dstInterface.devPort.portNum = PRV_TGF_CSCD_TARGET_PORT_NUM;
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:56, VLAN 5, device number 15, port 6 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* save the current lookup mode */
    rc = prvTgfCscdDevMapLookupModeGet(prvTgfDevNum, &(prvTgfRestoreCfg.lookupMode));
    if (rc == GT_NOT_APPLICABLE_DEVICE)
    {
        /* such device support only TRG_DEV mode */
        prvTgfRestoreCfg.lookupMode = PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;
    }
    else
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set TRG_DEV_TRG_PORT lookup mode for accessing Device Map table */
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d",
                                 PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);

}

/**
* @internal prvTgfCscdPortStackAggregationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Set FDB entry with device number 15, port 6, MAC 00:00:00:00: 00:56.
*         Set the cascade map table
*         - Target device 15, target port 6, link type port, link number 18.
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 18 with updated DSA tag (src info).
*         Set FDB entry with device number 15, port 6, MAC 00:00:00:00: 00:56.
*         Set the cascade map table
*         - Target device 15, target port 6, link type port, link number ??.
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports ?? with updated DSA tag (src info).
*/
GT_VOID prvTgfCscdPortStackAggregationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter;
    GT_U32       i;
    GT_U32       dsaIndexPosition = 0;
    TGF_VFD_INFO_STC        vfdArray[1];
    TGF_PACKET_DSA_TAG_STC  dsaInfoExpected;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 8;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;

    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    cpssOsMemCpy(&dsaInfoExpected, &prvTgfPacketDsaTagPart, sizeof(dsaInfoExpected));

    dsaInfoExpected.dsaInfo.forward.srcHwDev = PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM;
    dsaInfoExpected.dsaInfo.forward.srcId = PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID;
    dsaInfoExpected.dsaInfo.forward.source.portNum = PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM;

    prvTgfTrafficGeneratorPacketDsaTagBuild(&dsaInfoExpected, vfdArray[0].patternPtr);

    /* AUTODOC: GENERATE TRAFFIC: part1 */
    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* send packet */
        prvTgfCscdPortStackAggregationTestPacketSend(portIter, &prvTgfPacketInfo, 1, vfdArray);

    }

    /* AUTODOC: GENERATE TRAFFIC: part2 */
    /* AUTODOC: GENERATE TRAFFIC: send packet to non cascaded port 0 without DSA tag */

    for(i = 0; i < prvTgfPacketInfo.numOfParts; ++i)
    {
        if(prvTgfPacketPartArray[i].type == TGF_PACKET_PART_DSA_TAG_E)
        {
            dsaIndexPosition = i;
            prvTgfPacketPartArray[i].type = TGF_PACKET_PART_SKIP_E;
            prvTgfPacketPartArray[i].partPtr = NULL;
            break;
        }
    }

    prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], CPSS_CSCD_PORT_NETWORK_E);

    /* update expected DSA tag */
    vfdArray[0].patternPtr[3] = 0x1; /* VLAN */
    vfdArray[0].patternPtr[2] = 0x30; /* UP */
    vfdArray[0].patternPtr[6] = 0x1F; /* vidx */
    vfdArray[0].patternPtr[7] = 0xFF; /* vidx */


    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: set port[0] to cascaded mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);

    /* send packet */
    prvTgfCscdPortStackAggregationTestPacketSend(LOCAL_EGRESS_PORT_IDX2_CNS, &prvTgfPacketInfo, 1, vfdArray);

    /* restore packet configuraion */

    prvTgfPacketPartArray[dsaIndexPosition].type = TGF_PACKET_PART_DSA_TAG_E;
    prvTgfPacketPartArray[dsaIndexPosition].partPtr = &prvTgfPacketDsaTagPart;
}

/**
* @internal prvTgfCscdPortStackAggregationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCscdPortStackAggregationConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore default stack aggregation mode */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        prvTgfCscdPortStackAggregationEnableSet(prvTgfDevNum, prvTgfPortsArray[portIter], GT_FALSE);
        if(portIter == PRV_TGF_SEND_PORT_IDX0_CNS)
        {
            prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[portIter], CPSS_CSCD_PORT_NETWORK_E);
        }
        else
        {
            prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[portIter], CPSS_CSCD_PORT_NETWORK_E);
        }
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore default CSCD map table */
    /* restore cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_CSCD_TARGET_DEV_NUM, PRV_TGF_CSCD_TARGET_PORT_NUM,
                                  &(prvTgfRestoreCfg.cascadeLink), prvTgfRestoreCfg.srcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_CSCD_TARGET_DEV_NUM, PRV_TGF_CSCD_TARGET_PORT_NUM);

    /* AUTODOC: restore default lookup mode for accessing Device Map table */
    rc = prvTgfCscdDevMapLookupModeSet(prvTgfRestoreCfg.lookupMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapLookupModeSet: %d",
                                 prvTgfRestoreCfg.lookupMode);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


/**
* @internal prvTgfCscdPortStackAggregationFullPathConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set TTI rule: action, pattern and mask
*         - Set stack aggregation enabled and configure.
*         - Set lookup mode to Port for accessing the Device Map table.
*         - Set the cascade map table
*         - Target device 0, target port 0, link type port, link number 23.
*/
GT_VOID prvTgfCscdPortStackAggregationFullPathConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    GT_U32                    portIter;

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* set unioun fields to avoid LE/BE bugs */
    prvTgfPacketDsaTagPart_forward.source.portNum = 5;

    /* AUTODOC: set TTI action, pattern and mask */
    prvTgfCscdPortStackAggregationFullPathTtiRuleConfig(PRV_TGF_SEND_PORT_IDX0_CNS, CPSS_INTERFACE_PORT_E);
    prvTgfCscdPortStackAggregationFullPathTtiRuleConfig(PRV_TGF_SEND_PORT_IDX2_CNS, CPSS_INTERFACE_PORT_E);
    prvTgfCscdPortStackAggregationFullPathTtiRuleConfig(PRV_TGF_SEND_PORT_IDX3_CNS, CPSS_INTERFACE_PORT_E);

    /* AUTODOC: set stack aggregation configuration + port cascade mode */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        prvTgfCscdPortStackAggregationFullTestConfig(portIter);
    }

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);



}

/**
* @internal prvTgfCscdPortStackAggregationFullPathTrafficSend function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port[0] packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 18.
*/
static GT_VOID prvTgfCscdPortStackAggregationFullPathTrafficSend
(
    IN  CPSS_INTERFACE_TYPE_ENT interfaceType
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      dsaIndexPosition = 0;
    GT_U32                      i;
    TGF_VFD_INFO_STC            vfdArray[1];
    TGF_PACKET_DSA_TAG_STC      dsaInfoExpected;
    GT_U32                      portIter;
    GT_BOOL                     isOk;
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;

    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;

    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    cpssOsMemCpy(&dsaInfoExpected, &prvTgfPacketDsaTagPart, sizeof(dsaInfoExpected));

    dsaInfoExpected.dsaInfo.forward.srcHwDev = PRV_TGF_CSCD_STACK_AGGREGATOR_DEV_NUM;
    dsaInfoExpected.dsaInfo.forward.srcId = PRV_TGF_CSCD_STACK_AGGREGATOR_SRC_ID;
    dsaInfoExpected.dsaInfo.forward.source.portNum = PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM;
    if(interfaceType == CPSS_INTERFACE_TRUNK_E)
    {
        dsaInfoExpected.dsaInfo.forward.source.trunkId = PRV_TGF_CSCD_STACK_AGGREGATOR_PORT_NUM;
    }

    prvTgfTrafficGeneratorPacketDsaTagBuild(&dsaInfoExpected, vfdArray[0].patternPtr);

    if(interfaceType == CPSS_INTERFACE_TRUNK_E)
    {
        /* remove isSrcTrunk bit */
        vfdArray[0].patternPtr[1] &= ~0x4;
    }

    vfdArray[0].patternPtr[6] = 0x1F; /* vidx */
    vfdArray[0].patternPtr[7] = 0xFF; /* vidx */

    /* AUTODOC: GENERATE TRAFFIC: part1 */

    /* send unknown unicast from port_index = 0 */
    /* capture and packet inspection are performed for port_index = 2 */
    /* for other ports MAC counters are checked - 1 packet is expected */

    /* Additional configuration */
    /* Set source and destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX0_CNS], sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[LOCAL_EGRESS_PORT_IDX2_CNS], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: set stack aggregation configuration + port cascade mode */

    /* AUTODOC: set disable bridge bypass for port[sendPortIndex] */
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortBridgeBypassEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);

    /* AUTODOC: set port[sendPortIndex] to cascaded mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);
    /* AUTODOC: set port[egressPortIndex] to cascaded mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX2_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX2_CNS]);


    /* clear counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfCscdPortStackAggregationFullPathTestPacketSend(PRV_TGF_SEND_PORT_IDX0_CNS,
                                                         LOCAL_EGRESS_PORT_IDX2_CNS,
                                                         &prvTgfPacketInfo,
                                                         1,
                                                         vfdArray);

    /* check counters */
    prvTgfCscdPortStackAggregationFullPathCheckCounters(PRV_TGF_SEND_PORT_IDX0_CNS, LOCAL_EGRESS_PORT_IDX2_CNS, GT_TRUE);

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX0_CNS], sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* check FDB learning: port and device should based on TTI action not on DSA tag info */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    isOk = (prvTgfMacEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) &&
           (prvTgfMacEntry.dstInterface.devPort.portNum == prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isOk, "prvTgfBrgFdbMacEntryGet: on port %d", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);

    /* AUTODOC: set port[sendPortIndex] to network mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);
    /* AUTODOC: set port[egressPortIndex] to network mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX2_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX2_CNS]);


    /* AUTODOC: GENERATE TRAFFIC: part2 */

    /* send unknown unicast from port_index = 0 */
    /* capture and packet inspection are performed for port_index = 3 */
    /* for other ports MAC counters are checked - 1 packet is expected */

    /* Additional configuration */
    /* Set source and destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX0_CNS], sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[LOCAL_EGRESS_PORT_IDX3_CNS], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: set disable bridge bypass for port[sendPortIndex] */
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortBridgeBypassEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);

    /* AUTODOC: set port[sendPortIndex] to cascaded mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);
    /* AUTODOC: set port[egressPortIndex] to cascaded mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX3_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX3_CNS]);


    /* clear counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfCscdPortStackAggregationFullPathTestPacketSend(PRV_TGF_SEND_PORT_IDX0_CNS,
                                                         LOCAL_EGRESS_PORT_IDX3_CNS,
                                                         &prvTgfPacketInfo,
                                                         1,
                                                         vfdArray);

    prvTgfCscdPortStackAggregationFullPathCheckCounters(PRV_TGF_SEND_PORT_IDX0_CNS, LOCAL_EGRESS_PORT_IDX3_CNS, GT_TRUE);

    /* AUTODOC: set port[sendPortIndex] to network mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX0_CNS]);
    /* AUTODOC: set port[egressPortIndex] to network mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX3_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX3_CNS]);


    /* AUTODOC: GENERATE TRAFFIC: part3 */

    /* send unicast from port_index = 2 */
    /* capture and packet inspection are performed for port_index = 0 */
    /* for other ports MAC counters are checked - no packets are expected */

    /* Additional configuration */
    /* Set source and destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX2_CNS], sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[LOCAL_EGRESS_PORT_IDX0_CNS], sizeof(prvTgfPacketL2Part.daMac));

    vfdArray[0].patternPtr[6] = 0x0; /* port 0 */
    vfdArray[0].patternPtr[7] = 0x0; /* dev 0 */

    /* AUTODOC: set disable bridge bypass for port[sendPortIndex] */
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX2_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortBridgeBypassEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX2_CNS]);

    /* AUTODOC: set port[sendPortIndex] to cascaded mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX2_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX2_CNS]);
    /* AUTODOC: set port[egressPortIndex] to cascaded mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS]);

    /* clear counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfCscdPortStackAggregationFullPathTestPacketSend(PRV_TGF_SEND_PORT_IDX2_CNS,
                                                         LOCAL_EGRESS_PORT_IDX0_CNS,
                                                         &prvTgfPacketInfo,
                                                         1,
                                                         vfdArray);

    prvTgfCscdPortStackAggregationFullPathCheckCounters(PRV_TGF_SEND_PORT_IDX2_CNS, LOCAL_EGRESS_PORT_IDX0_CNS, GT_FALSE);

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX2_CNS], sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* check FDB learning: port and device should based on TTI action not on DSA tag info */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    isOk = (prvTgfMacEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) &&
           (prvTgfMacEntry.dstInterface.devPort.portNum == prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX2_CNS]);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isOk, "prvTgfBrgFdbMacEntryGet: on port %d", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX2_CNS]);

    /* AUTODOC: set port[sendPortIndex] to network mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX2_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX2_CNS]);
    /* AUTODOC: set port[egressPortIndex] to network mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS]);


    /* AUTODOC: GENERATE TRAFFIC: part4 */

    /* send unicast from port_index = 3 */
    /* capture and packet inspection are performed for port_index = 0 */
    /* for other ports MAC counters are checked - no packets are expected */

    /* Additional configuration */
    /* Set source and destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX3_CNS], sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[LOCAL_EGRESS_PORT_IDX0_CNS], sizeof(prvTgfPacketL2Part.daMac));

    vfdArray[0].patternPtr[6] = 0x0; /* port 0 */
    vfdArray[0].patternPtr[7] = 0x0; /* dev 0 */

    /* AUTODOC: set disable bridge bypass for port[sendPortIndex] */
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX3_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortBridgeBypassEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX3_CNS]);

    /* AUTODOC: set port[sendPortIndex] to cascaded mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX3_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX3_CNS]);
    /* AUTODOC: set port[egressPortIndex] to cascaded mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS]);

    /* clear counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfCscdPortStackAggregationFullPathTestPacketSend(PRV_TGF_SEND_PORT_IDX3_CNS,
                                                         LOCAL_EGRESS_PORT_IDX0_CNS,
                                                         &prvTgfPacketInfo,
                                                         1,
                                                         vfdArray);

    prvTgfCscdPortStackAggregationFullPathCheckCounters(PRV_TGF_SEND_PORT_IDX3_CNS, LOCAL_EGRESS_PORT_IDX0_CNS, GT_FALSE);

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX3_CNS], sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* check FDB learning: port and device should based on TTI action not on DSA tag info */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    isOk = (prvTgfMacEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) &&
           (prvTgfMacEntry.dstInterface.devPort.portNum == prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX3_CNS]);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isOk, "prvTgfBrgFdbMacEntryGet: on port %d", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX3_CNS]);


    /* AUTODOC: set port[sendPortIndex] to network mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX3_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX3_CNS]);
    /* AUTODOC: set port[egressPortIndex] to network mode (TX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS]);

    /* AUTODOC: GENERATE TRAFFIC: part5 */

    /* send unicast from NON cascaded port_index = 1 */
    /* capture and packet inspection are performed for port_index = 0 */
    /* for other ports MAC counters are checked - no packets are expected */

    /* Additional configuration */
    /* Set source and destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX1_CNS], sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[LOCAL_EGRESS_PORT_IDX0_CNS], sizeof(prvTgfPacketL2Part.daMac));

    vfdArray[0].patternPtr[2] = 0x30; /* UP */
    vfdArray[0].patternPtr[4] = 0x31; /* port 0 */
    vfdArray[0].patternPtr[6] = 0x00; /* port 0 */
    vfdArray[0].patternPtr[7] = 0x00; /* dev 0 */

    /* AUTODOC: set disable bridge bypass for port[sendPortIndex] */
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX1_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortBridgeBypassEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX1_CNS]);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX1_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX1_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX1_CNS]);

    for(i = 0; i < prvTgfPacketInfo.numOfParts; ++i)
    {
        if(prvTgfPacketPartArray[i].type == TGF_PACKET_PART_DSA_TAG_E)
        {
            dsaIndexPosition = i;
            prvTgfPacketPartArray[i].type = TGF_PACKET_PART_SKIP_E;
            prvTgfPacketPartArray[i].partPtr = NULL;
            break;
        }
    }

    /* AUTODOC: set port[sendPortIndex] to cascaded mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS], CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS]);

    /* clear counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfCscdPortStackAggregationFullPathTestPacketSend(PRV_TGF_SEND_PORT_IDX1_CNS,
                                                         LOCAL_EGRESS_PORT_IDX0_CNS,
                                                         &prvTgfPacketInfo,
                                                         1,
                                                         vfdArray);

    prvTgfCscdPortStackAggregationFullPathCheckCounters(PRV_TGF_SEND_PORT_IDX1_CNS, LOCAL_EGRESS_PORT_IDX0_CNS, GT_FALSE);

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX1_CNS], sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* check FDB learning: port and device should based on TTI action not on DSA tag info */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    isOk = (prvTgfMacEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) &&
           (prvTgfMacEntry.dstInterface.devPort.portNum == prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX1_CNS]);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isOk, "prvTgfBrgFdbMacEntryGet: on port %d", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX1_CNS]);

    /* AUTODOC: set port[sendPortIndex] to network mode (RX direction) */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS], CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[LOCAL_EGRESS_PORT_IDX0_CNS]);

    /* restore packet configuraion */
    prvTgfPacketPartArray[dsaIndexPosition].type = TGF_PACKET_PART_DSA_TAG_E;
    prvTgfPacketPartArray[dsaIndexPosition].partPtr = &prvTgfPacketDsaTagPart;
}

/**
* @internal prvTgfCscdPortStackAggregationFullPathTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port[0] packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 18.
*/
GT_VOID prvTgfCscdPortStackAggregationFullPathTrafficGenerate
(
    GT_VOID
)
{
    /*AUTODOC: send packet from port interface*/
    prvTgfCscdPortStackAggregationFullPathTrafficSend(CPSS_INTERFACE_PORT_E);

    /*AUTODOC: restore default settings */
    prvTgfCscdPortStackAggregationFullPathConfigurationRestore();

    /*AUTODOC: start second test: send packet from trunk interface */
    prvTgfCscdPortStackAggregationFullPathConfigurationSet();

    prvTgfPacketDsaTagPart_forward.srcIsTrunk = GT_TRUE;
    prvTgfPacketDsaTagPart_forward.source.trunkId = 5;

    /* Lion2 B0 bug: isSrcTrunk bit is not cleared,
       therefore TTI trunk rules should be added */
    prvTgfCscdPortStackAggregationFullPathTtiRuleConfig(PRV_TGF_SEND_PORT_IDX0_CNS, CPSS_INTERFACE_TRUNK_E);
    prvTgfCscdPortStackAggregationFullPathTtiRuleConfig(PRV_TGF_SEND_PORT_IDX2_CNS, CPSS_INTERFACE_TRUNK_E);
    prvTgfCscdPortStackAggregationFullPathTtiRuleConfig(PRV_TGF_SEND_PORT_IDX3_CNS, CPSS_INTERFACE_TRUNK_E);

    prvTgfCscdPortStackAggregationFullPathTrafficSend(CPSS_INTERFACE_TRUNK_E);

    prvTgfPacketDsaTagPart_forward.srcIsTrunk = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.source.portNum = 5;
}

/**
* @internal prvTgfCscdPortStackAggregationFullPathConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCscdPortStackAggregationFullPathConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      portIter = 0;

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        prvTgfCscdPortStackAggregationEnableSet(prvTgfDevNum, prvTgfPortsArray[portIter], GT_FALSE);

        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[portIter], PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d %d", prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: invalidate port TTI rules */
        rc = prvTgfTtiRuleValidStatusSet(portIter, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, portIter);

        /* AUTODOC: invalidate trunk TTI rules */
        rc = prvTgfTtiRuleValidStatusSet(portIter+20, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, portIter+20);

    }


    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[PRV_TGF_SEND_PORT_IDX0_CNS], sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[LOCAL_EGRESS_PORT_IDX2_CNS], sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX1_CNS], prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}




