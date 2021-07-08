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
* @file prvTgfPclEgressMirroring.c
*
* @brief Pcl Egress Mirroring packet cmd tests
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
#include <common/tgfMirror.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <pcl/prvTgfPclEgressMirroring.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>

/* analyzer port number */
#define PRV_TGF_ANALYZER_PORT_IDX_CNS     3

/* amount of sent packets */
#define PRV_TGF_PACKET_NUM_CNS            1

/* analyzer index */
#define PRV_TGF_ANALYZER_INDEX_CNS        2

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/* interface of an analyzer bound to an rx-port */
PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC     orig_tgfPortInf;

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

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfPclEgressMirroringCfgSet function
* @endinternal
*
* @brief  Configure Egress PCL Rule with:
*         action pkt cmd : CPSS_PACKET_CMD_TRAP_FORWARD_E
*/
GT_VOID prvTgfPclEgressMirroringCfgSet
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

    /* AUTODOC: init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                        = CPSS_NET_IP_HDR_ERROR_E;
    action.egressPolicy                          = GT_TRUE;
    action.mirror.mirrorToTxAnalyzerPortEn       = GT_TRUE; 
    action.mirror.egressMirrorToAnalyzerIndex    = PRV_TGF_ANALYZER_INDEX_CNS;
    action.mirror.egressMirrorToAnalyzerMode     = CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEgressMirroringGenerate function
* @endinternal
*
* @brief  Configure to send packet and check traffic sent from analyzer
*/
GT_VOID prvTgfPclEgressMirroringTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc  = GT_OK;    
    CPSS_INTERFACE_INFO_STC         portInterface;    
    GT_U8                           dev      = 0;
    GT_U8                           queue    = 0;
    GT_BOOL                         getFirst = GT_TRUE;
    GT_U32                          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                          packetActualLength = 0;
    TGF_NET_DSA_STC                 rxParam;
    static GT_U8                    packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC tgfPortInf;    
    
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");           
    
    tgfPortInf.interface.type             = CPSS_INTERFACE_PORT_E;
    tgfPortInf.interface.devPort.hwDevNum = prvTgfDevNum;
    tgfPortInf.interface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, PRV_TGF_ANALYZER_INDEX_CNS, &orig_tgfPortInf);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d",
                                 prvTgfDevNum); 

    prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_INDEX_CNS, &tgfPortInf);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d",
                                 PRV_TGF_ANALYZER_INDEX_CNS); 

    /* AUTODOC: Start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: enable capture on AnalyzerPort */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);

    /* AUTODOC: Generate traffic */

    rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, &prvTgfPacketInfo,
            prvTgfBurstCount, 0 , NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: Send packet to port 1 */
    rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EPCL_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth");
    
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    rc = cpssOsMemCmp(&prvTgfPacketL2Part.daMac, &(packetBuff[0]), sizeof(TGF_MAC_ADDR)); /* compare dstMac address */   
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");    

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

GT_VOID prvTgfPclEgressMirroringCfgRestore
(
    GT_VOID
)
{
    GT_STATUS                         rc,rc1 = GT_OK;    

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

    prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_INDEX_CNS, &orig_tgfPortInf);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d",
                                 PRV_TGF_ANALYZER_INDEX_CNS);

    rc = prvTgfFdbDefCfgRestore();
    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}
