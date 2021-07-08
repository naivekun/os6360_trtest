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
* @file prvTgfPclSourceIdMask.c
*
* @brief Source ID assignment masking in PCL action test
*
* @version   4
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
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>

#include <pcl/prvTgfPclSourceIdMask.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   0

/* port number to receive traffic from */
#define PRV_TGF_RECV_PORT_IDX_CNS   1

/* VLAN Id */
#define PRV_TGF_VLANID_CNS          5
#define PRV_TGF_PORTS_IN_VLAN_CNS   2

#define PRV_TGF_UP_MODIFY_CNS       6

/* PCL rule & action index */
#define IPCL_MATCH_INDEX_0_CNS     17
#define IPCL_MATCH_INDEX_1_CNS     43
#define EPCL_MATCH_INDEX_CNS      prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(108)

/* Source Id data & mask */
static GT_U32   tempSourceId  = 0xAAA;
static GT_U32   tempSourceId2 = 0xFFF;
#define SOURCE_ID_DATA_CNS     tempSourceId
#define SOURCE_ID_DATA2_CNS    tempSourceId2

#define SOURCE_ID_MASK_CNS     0xFFF

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/************************* General packet's parts *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}                 /* saMac */
};

/* VLAN_TAG 0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
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
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************************************************************/

/* RESTORE VALUES */

GT_U32 sourceIdForRestore;

/**
* @internal prvTgfPclSourceIdMaskTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPclSourceIdMaskTestVlanInit
(
    IN GT_U16           vlanId
)
{
    GT_U32                      portIter  = 0;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

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
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;
    vlanInfo.fidValue              = vlanId;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    for (portIter = 0; portIter < PRV_TGF_PORTS_IN_VLAN_CNS; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] =
                                  PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;

        /* reset counters and set force link up */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfPclSourceIdMaskTestIpclRulesAndActionsSet function
* @endinternal
*
* @brief   Set PCL rules and actions
*/
static GT_VOID prvTgfPclSourceIdMaskTestIpclRulesAndActionsSet
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

    /* AUTODOC: IPCL action - enable assign source ID */
    action.sourceId.assignSourceId = GT_TRUE;

    /* AUTODOC: IPCL action - assign source ID 0xAAA*/
    action.sourceId.sourceIdValue = SOURCE_ID_DATA_CNS;

    /* AUTODOC: IPCL Mask for MAC SA */
    cpssOsMemSet(mask.ruleStdNotIp.macSa.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: IPCL Pattern for MAC SA */
    cpssOsMemCpy(pattern.ruleStdNotIp.macSa.arEther,
                 prvTgfPacketL2Part.saMac,
                 sizeof(GT_ETHERADDR));

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            IPCL_MATCH_INDEX_0_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                 IPCL_MATCH_INDEX_0_CNS);

    /* AUTODOC: Change MAC SA to MAC SA 1 */
    prvTgfPacketL2Part.saMac[5] += 1;

    /* AUTODOC: IPCL action - assign source ID 0xFFF*/
    action.sourceId.sourceIdValue = SOURCE_ID_DATA2_CNS;

    /* AUTODOC: IPCL Pattern for MAC SA 1 */
    cpssOsMemCpy(pattern.ruleStdNotIp.macSa.arEther,
                 prvTgfPacketL2Part.saMac,
                 sizeof(GT_ETHERADDR));

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            IPCL_MATCH_INDEX_1_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                 IPCL_MATCH_INDEX_1_CNS);

    /* AUTODOC: Change back MAC SA 1 to MAC SA */
    prvTgfPacketL2Part.saMac[5] -= 1;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: EPCL action - Modify UP */
    action.egressPolicy =  GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.qos.egressUp0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
    action.qos.up       = PRV_TGF_UP_MODIFY_CNS;

    /* AUTODOC: EPCL Mask for Source ID */
    mask.ruleEgrExtNotIpv6.common.sourceId = SOURCE_ID_MASK_CNS;

    /* AUTODOC: EPCL Pattern for Source ID */
    pattern.ruleEgrExtNotIpv6.common.sourceId = SOURCE_ID_DATA_CNS;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
                                 EPCL_MATCH_INDEX_CNS);
}

/**
* @internal prvTgfPclSourceIdMaskTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfPclSourceIdMaskTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN with port 0 & 1 as members. */
    prvTgfPclSourceIdMaskTestVlanInit(PRV_TGF_VLANID_CNS);

    /* AUTODOC: Init IPCL Engine for port 0 */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E     /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);

    /* AUTODOC: Init EPCL Engine for port 1 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E  /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E);

    /* AUTODOC: Set PCL (IPCL & EPCL) rules & actions */
    prvTgfPclSourceIdMaskTestIpclRulesAndActionsSet();

    /* AUTODOC: Save Source ID mask for restore */
    rc = prvTgfPclSourceIdMaskGet(prvTgfDevNum,
                                  CPSS_PCL_LOOKUP_0_E,
                                  &sourceIdForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 CPSS_PCL_LOOKUP_0_E,
                                 sourceIdForRestore);

    /* AUTODOC: Set Source ID mask to 0xFFF */
    rc = prvTgfPclSourceIdMaskSet(CPSS_PCL_LOOKUP_0_E, SOURCE_ID_MASK_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskSet: %d, %d",
                                 CPSS_PCL_LOOKUP_0_E, SOURCE_ID_MASK_CNS);
}

/**
* @internal prvTgfPclSourceIdMaskTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPclSourceIdMaskTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Restore Source ID mask */
    rc = prvTgfPclSourceIdMaskSet(CPSS_PCL_LOOKUP_0_E, sourceIdForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskSet: %d, %d",
                                 CPSS_PCL_LOOKUP_0_E, sourceIdForRestore);

    /* AUTODOC: invalidate IPCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     IPCL_MATCH_INDEX_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 IPCL_MATCH_INDEX_0_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     IPCL_MATCH_INDEX_1_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 IPCL_MATCH_INDEX_1_CNS,
                                 GT_FALSE);

    /* AUTODOC: invalidate EPCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E,
                                     EPCL_MATCH_INDEX_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_EXT_E,
                                 EPCL_MATCH_INDEX_CNS,
                                 GT_FALSE);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for ingress port */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);


    /* AUTODOC: Clear EPCL Engine configuration tables */
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: Disables PCL egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable: %d",
                                 GT_FALSE);


    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclSourceIdMaskTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] sendPortIndex            - port indexing place in test array ports traffic
*                                      is transmitted to.
* @param[in] recvPortIndex            - port indexing place in test array ports traffic
*                                      is received from.
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclSourceIdMaskTestPacketSend
(
    IN GT_U32           sendPortIndex,
    IN GT_U32           recvPortIndex,
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      portIter;
    CPSS_INTERFACE_INFO_STC  portInterface;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[recvPortIndex];

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum,
                                 portInterface.devPort.portNum,
                                 GT_TRUE);
    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum,
                                 portInterface.devPort.portNum,
                                 GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n",
                     portInterface.devPort.portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPclSourceIdMaskTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPclSourceIdMaskTestGenerateTraffic
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    GT_U32                          partsCount;
    GT_U32                          packetSize;
    TGF_PACKET_STC                  packetInfo;
    CPSS_INTERFACE_INFO_STC         portInterface;

    GT_U32  actualCapturedNumOfPackets;

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet: %d, %d",
                                 partsCount, packetSize);

    /* AUTODOC: build packet info */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];

    /* AUTODOC: Send packet with MAC SA                             */
    /* AUTODOC: - expect match in IPCL -> source ID assign to OxAAA */
    /* AUTODOC: - expect match in EPCL -> packet UP modified        */
    prvTgfPclSourceIdMaskTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                        PRV_TGF_RECV_PORT_IDX_CNS,
                                        &packetInfo);

    prvTgfPacketVlanPart.pri = PRV_TGF_UP_MODIFY_CNS;
    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &packetInfo,
                                        prvTgfBurstCount,
                                        /*numOfPackets*/ 0/*vfdNum*/,
                                        NULL /*vfdArray*/,
                                        NULL, /* bytesNum's skip list */
                                        0,    /* length of skip list */
                                        &actualCapturedNumOfPackets,
                                        NULL/*onFirstPacketNumTriggersBmpPtr*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureCompare: %d",
                                 portInterface.devPort.portNum);
    prvTgfPacketVlanPart.pri = 0;

    /* AUTODOC: Change MAC SA to MAC SA 1 */
    prvTgfPacketL2Part.saMac[5] += 1;

    /* AUTODOC: Send packet with MAC SA 1                           */
    /* AUTODOC: - expect match in IPCL -> source ID assign to OxFFF */
    /* AUTODOC: - no match in EPCL     -> packet UP NOT modified    */
    prvTgfPclSourceIdMaskTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                        PRV_TGF_RECV_PORT_IDX_CNS,
                                        &packetInfo);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &packetInfo,
                                        prvTgfBurstCount,
                                        /*numOfPackets*/ 0/*vfdNum*/,
                                        NULL /*vfdArray*/,
                                        NULL, /* bytesNum's skip list */
                                        0,    /* length of skip list */
                                        &actualCapturedNumOfPackets,
                                        NULL/*onFirstPacketNumTriggersBmpPtr*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureCompare: %d",
                                 portInterface.devPort.portNum);

    /* AUTODOC: Set Source ID mask to 0xAAA */
    rc = prvTgfPclSourceIdMaskSet(CPSS_PCL_LOOKUP_0_E, SOURCE_ID_DATA_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskSet: %d, %d",
                                 CPSS_PCL_LOOKUP_0_E, SOURCE_ID_DATA_CNS);

    /* AUTODOC: Send packet with MAC SA 1                           */
    /* AUTODOC: - expect match in IPCL -> source ID assign to OxAAA */
    /* AUTODOC: - expect match in EPCL -> packet UP modified        */
    prvTgfPclSourceIdMaskTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                        PRV_TGF_RECV_PORT_IDX_CNS,
                                        &packetInfo);

    prvTgfPacketVlanPart.pri = PRV_TGF_UP_MODIFY_CNS;
    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &packetInfo,
                                        prvTgfBurstCount,
                                        /*numOfPackets*/ 0/*vfdNum*/,
                                        NULL /*vfdArray*/,
                                        NULL, /* bytesNum's skip list */
                                        0,    /* length of skip list */
                                        &actualCapturedNumOfPackets,
                                        NULL/*onFirstPacketNumTriggersBmpPtr*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureCompare: %d",
                                 portInterface.devPort.portNum);
    prvTgfPacketVlanPart.pri = 0;

    /* AUTODOC: Change back MAC SA 1 to MAC SA */
    prvTgfPacketL2Part.saMac[5] -= 1;
}

/**
* @internal prvTgfPclSourceIdMaskTest function
* @endinternal
*
* @brief   Source ID assignment masking in PCL action test
*/
GT_VOID prvTgfPclSourceIdMaskTest
(
    GT_VOID
)
{
    /* fit into valid range */
    tempSourceId  %= (PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(prvTgfDevNum) + 1);
    tempSourceId2 %= (PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(prvTgfDevNum) + 1);

    prvTgfPclSourceIdMaskTestConfigurationSet();

    prvTgfPclSourceIdMaskTestGenerateTraffic();

    prvTgfPclSourceIdMaskTestConfigurationRestore();
}


