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
* @file tgfCosTrustQosMappingTableSelect.c
*
* @brief Use User Priority as Mapping Table Index.
*
* @version   5
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>
#include <common/tgfCosGen.h>
#include <cos/tgfCosTrustQosMappingTableSelect.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS  5

/* VLAN Id 6 */
#define PRV_TGF_VLANID_6_CNS  6

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS 0

/* Traffic Classes used in test */
#define PRV_TGF_COS_TEST_TC0  3

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* default value for ipv4 total length field */
#define PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS 0x54

/* Qos Profile used in test */
GT_U32 qosProfileIndex = 10;

/* default number of packets to send */
GT_U32 prvTgfBurstCount = 1;
CPSS_QOS_PORT_TRUST_MODE_ENT modeGet;
PRV_TGF_COS_PROFILE_STC      profileEntryGet;
GT_BOOL                      useUpAsIndexGet;
GT_U32 mappingTableIndexGet;
GT_U32 profileIndex2UpGet;
CPSS_QOS_ENTRY_STC  portQoSAttrCfgGet;

/* vlan tag type */
CPSS_VLAN_TAG_TYPE_ENT vlanTagTypeGet;

/* packet ipv4part */
TGF_PACKET_IPV4_STC     ipv4PartGet;

/*  IPv4 packet to send */
TGF_PACKET_STC prvTgfIpv4PacketInfo;

/*  IPv4 modified packet to compare */
TGF_PACKET_STC               packetInfo;
GT_U8                        up                = 6; /* incoming up */
GT_U32                       mappingTableIndex = 8;
GT_U8                        cfiDeiBit         = 0;
CPSS_QOS_ENTRY_STC           portQoSAttrCfg;
PRV_TGF_TTI_ACTION_2_STC     ruleAction;
PRV_TGF_TTI_RULE_UNT         pattern;
PRV_TGF_TTI_RULE_UNT         mask;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal tgfCosTrustQosMappingTableSelectBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID tgfCosTrustQosMappingTableSelectBridgeConfigSet
(
     GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS,
                                           prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS,
                                           prvTgfPortsArray + 2, NULL,
                                           tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_VLANID_5_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d",
                                 prvTgfDevNum);
}

/**
* @internal tgfCosTrustQosMappingTableSelectBuildPacket function
* @endinternal
*
* @brief   Build Packet
*
* @note Packet description:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID tgfCosTrustQosMappingTableSelectBuildPacket
(
   GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr    = {0};
    TGF_PACKET_VLAN_TAG_STC *vlanTagPartPtr = {0};

    /* AUTODOC: get default IPv4 packet */
    rc = prvTgfPacketIpv4PacketDefaultPacketGet(&packetInfo.numOfParts,
                                                &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketDefaultPacketGet");

    /* AUTODOC: get packet header part*/
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E,
                                   &packetInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    ipv4PartPtr->totalLen = PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS;
    ipv4PartPtr->typeOfService = 0;

    /* AUTODOC: changed Ipv4 header part*/
    rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_IPV4_E,0,
                                             ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_VLAN_TAG_E, &packetInfo,
                                   (GT_VOID *)&vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");
    vlanTagPartPtr->pri = 6;
    vlanTagPartPtr->vid = 5;

    /* add vlan tag with new up and vid */
    rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E, 0,
                                             vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    prvTgfIpv4PacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfIpv4PacketInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfIpv4PacketInfo.partsArray = packetInfo.partsArray;
}

/**
* @internal tgfCosTrustQosMappingTableSelectCaseConfigurationSet function
* @endinternal
*
* @brief   Set specific configuration for two tests
*
* @param[in] testNumber               0 - test uses mapping table index
*                                      1 - user priority is used to select table
*                                       None
*/
GT_VOID tgfCosTrustQosMappingTableSelectCaseConfigurationSet
(
    IN GT_U32 testNumber)
{
GT_STATUS rc = GT_OK;

    if (testNumber == 1)
    {
        mappingTableIndex = up;
    }
    else
    {
        mappingTableIndex = 8;
    }

    /* test one: set default eport trust mapping table select 8 */
    /* test two: set default eport trust mapping table select 6 */
    rc = prvTgfCosPortTrustQosMappingTableIndexSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    (testNumber) ? GT_TRUE : GT_FALSE,
                                    mappingTableIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                            "prvTgfCosPortTrustQosMappingTableIndexSet: %d, %d",
                            prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* test one: set mapped to up QoS profile in mapping table 8 */
    /* test two: set mapped to up QoS profile in mapping table 6 */
    rc = prvTgfCosUpCfiDeiToProfileMapSet(mappingTableIndex, 0, up, cfiDeiBit,
                                          qosProfileIndex);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                             "prvTgfCosUpCfiDeiToProfileMapSet: %d, %d, %d, %d",
                             prvTgfDevNum,up, cfiDeiBit, qosProfileIndex);

    /* test one: use mapping table index as table selector */
    /* test two: allow use UP as mapping table index  */
    ruleAction.qosUseUpAsIndexEnable = (testNumber) ? GT_TRUE : GT_FALSE;

    /* set TTI rule */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_IPV4_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d",
                                 prvTgfDevNum);
}

/**
* @internal tgfCosTrustQosMappingTableSelectCommonConfigurationSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID tgfCosTrustQosMappingTableSelectCommonConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                    rc                = GT_OK;
    PRV_TGF_COS_PROFILE_STC      profileEntry      = {0, 0, 0, 0, 0};
    CPSS_QOS_PORT_TRUST_MODE_ENT mode              = CPSS_QOS_PORT_TRUST_L2_E;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    mode = CPSS_QOS_PORT_TRUST_L2_E;
    /* save current trust mode */
    rc = prvTgfCosPortQosTrustModeGet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      &modeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfCosPortQosTrustModeGet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set new CPSS_QOS_PORT_TRUST_L2_E trust mode */
    rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      mode);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfDevNum,
                                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                mode);

    /* save current vlan tag type to trust*/
    rc = prvTgfCosL2TrustModeVlanTagSelectGet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    GT_TRUE, &vlanTagTypeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfCosL2TrustModeVlanTagSelectGet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set vlan tag0 to trust */
    rc = prvTgfCosL2TrustModeVlanTagSelectSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                              GT_FALSE, CPSS_VLAN_TAG0_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                             "prvTgfCosL2TrustModeVlanTagSelectSet: %d, %d, %d",
                             prvTgfDevNum,
                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_VLAN_TAG0_E);

    /* save default eport trust mapping table  */
    rc = prvTgfCosPortTrustQosMappingTableIndexGet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &useUpAsIndexGet, &mappingTableIndexGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                            "prvTgfCosPortTrustQosMappingTableIndexGet: %d, %d",
                            prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* test one: save mapped to up QoS profile in mapping table 0 */
    rc = prvTgfCosUpCfiDeiToProfileMapGet(prvTgfDevNum, mappingTableIndex,
                                          0, up, cfiDeiBit, &profileIndex2UpGet);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfCosUpCfiDeiToProfileMapGet: %d, %d, %d , %d",
                                 prvTgfDevNum, profileIndex2UpGet, up, cfiDeiBit);

    /* save QoS profile entry */
     rc = prvTgfCosProfileEntryGet(prvTgfDevNum, qosProfileIndex,
                                   &profileEntryGet);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntryGet");

     /* set QoS profile 10 */
     profileEntry.dropPrecedence = CPSS_DP_GREEN_E;
     profileEntry.dscp = 0;
     profileEntry.exp = 2;
     profileEntry.trafficClass = 3;
     profileEntry.userPriority = 7;

     rc = prvTgfCosProfileEntrySet(qosProfileIndex, &profileEntry);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

     /* save QoS port attributes */
     rc = prvTgfCosPortQosConfigGet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &portQoSAttrCfgGet);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigGet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

     /* set QoS port attributes */
     portQoSAttrCfg.qosProfileId     = qosProfileIndex;
     portQoSAttrCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
     portQoSAttrCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
     portQoSAttrCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

     rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &portQoSAttrCfg);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet: %d, %d",
                                  prvTgfDevNum,
                                  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask, 0, sizeof(mask));

    pattern.ipv4.common.vid = 5;
    mask.ipv4.common.vid = 0xFFFF;

    /* AUTODOC: enable TTI lookup for port 0, key PRV_TGF_TTI_KEY_IPV4_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_FALSE;
    ruleAction.ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ruleAction.command               = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.qosProfile            = 0;
    ruleAction.redirectCommand       = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.tunnelStart           = GT_FALSE;
    ruleAction.tag0VlanCmd           = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;

    ruleAction.trustUp         = GT_TRUE;
    ruleAction.trustExp        = GT_FALSE;
    ruleAction.trustDscp       = GT_FALSE;
    ruleAction.keepPreviousQoS = GT_FALSE;

    ruleAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    ruleAction.tag0VlanCmd                     = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ruleAction.tag0VlanId                      = 0x6;

    ruleAction.qosMappingTableIndex  = 8;
    ruleAction.bridgeBypass          = GT_TRUE;
}

/**
* @internal tgfCosTrustQosMappingTableSelectTrafficOnPort function
* @endinternal
*
* @brief   Check traffic on egress port
*/
GT_VOID tgfCosTrustQosMappingTableSelectTrafficOnPort
(
    GT_U32 capturedPort
)
{
    GT_STATUS               rc            = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];
    GT_U32                  numTriggers   = 0;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = capturedPort;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* Prepare VFD array entry for Identification field of IPv4 header */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 14;
    vfdArray[0].cycleCount = sizeof(GT_U8);
    vfdArray[0].modeExtraInfo=0;
    vfdArray[0].incValue=1;
    vfdArray[0].patternPtr[0]=0xE0;
    vfdArray[0].cycleCount=1;

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1,
                                                        vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                               "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                               " port = %d, rc = 0x%02X\n",
                               portInterface.devPort.portNum, rc);

    /* check if captured packet has the same pattern as vfdArray */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,
                                 "Packet VLAN TAG0/1 pattern is wrong");
}

/**
* @internal tgfCosTrustQosMappingTableSelectTrafficGenerate function
* @endinternal
*
* @brief   Generate Traffic
*/
GT_VOID tgfCosTrustQosMappingTableSelectTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32          portIter    = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC:   send 1  IPv4  packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfIpv4PacketInfo, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* check captured egress traffic on port */
    tgfCosTrustQosMappingTableSelectTrafficOnPort(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    return;
}

/**
* @internal tgfCosTrustQosMappingTableSelectConfigurationRestore function
* @endinternal
*
* @brief   Restore Base Configuration
*/
GT_VOID tgfCosTrustQosMappingTableSelectConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;
    GT_U8              up = 6;
    GT_U8              cfiDeiBit = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* restore CPSS_QOS_PORT_TRUST_L2_E trust mode */
    rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      modeGet);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfCosPortQosTrustModeSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 modeGet);

    /* restore vlan tag0 to no trust*/
    rc = prvTgfCosL2TrustModeVlanTagSelectSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                              GT_TRUE, vlanTagTypeGet);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                             "prvTgfCosL2TrustModeVlanTagSelectSet: %d, %d, %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             CPSS_VLAN_TAG0_E);

     /* restore default trust mapping table index */
     rc = prvTgfCosPortTrustQosMappingTableIndexSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                    useUpAsIndexGet, 0);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                            "prvTgfCosPortTrustQosMappingTableIndexSet: %d, %d",
                            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

     /* restore mapped to up QoS profile */
     rc = prvTgfCosUpCfiDeiToProfileMapSet(6, 0, up, cfiDeiBit,
                                           0);
     UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                             "prvTgfCosUpCfiDeiToProfileMapSet: %d, %d, %d, %d",
                             prvTgfDevNum,up, cfiDeiBit, qosProfileIndex);

     /* restore QoS profile entry */
     rc = prvTgfCosProfileEntrySet(qosProfileIndex, &profileEntryGet);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    /* restore port QoS attributes */
    rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                   &portQoSAttrCfgGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d",
                                 prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d",
                                 prvTgfDevNum);

    /* AUTODOC: invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for port 0, key PRV_TGF_TTI_KEY_IPV4_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

}

