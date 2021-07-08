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
* @file prvTgfPclTargetPortSelect.c
*
* @brief EPCL Target Port Selection test
*
* @version   5
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
#include <common/tgfCscdGen.h>

#include <pcl/prvTgfPclTargetPortSelect.h>

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

/* EPCL rule & action index */
#define EPCL_MATCH_INDEX_CNS      prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(100)

/* local target eport */
#define PRV_TGF_TARGET_EPORT_CNS (IS_AC3X_DEV_MAC(prvTgfDevNum)?1023:2000)

/* final target device & port */
#define PRV_TGF_DSA_DEV_CNS     0xd
#define PRV_TGF_DSA_PORT_CNS    0x2F

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/************************* General packet's parts *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}                 /* saMac */
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

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* PARTS of expected packet */
static TGF_PACKET_PART_STC prvTgfExpectedPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of expected packet */
#define PRV_TGF_EXPECTED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Expected PACKET to compare */
static TGF_PACKET_STC prvTgfExpextedPacketInfo =
{
    PRV_TGF_EXPECTED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfExpectedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfExpectedPacketPartArray                                        /* partsArray */
};

/******************************************************************************/

/* RESTORE VALUES */
static CPSS_CSCD_PORT_TYPE_ENT cscdPortTypeForRestore;
static PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT   portSelectionModeForRestore;
static CPSS_INTERFACE_INFO_STC physicalInfoForRestore;
static CPSS_CSCD_LINK_TYPE_STC cascadeLinkForRestore;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEnForRestore;

/**
* @internal prvTgfPclTargetPortSelectTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPclTargetPortSelectTestVlanInit
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
                                            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;

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
* @internal prvTgfPclTargetPortSelectTestIpclRulesAndActionsSet function
* @endinternal
*
* @brief   Set PCL rules and actions
*
* @param[in] targetPort               - target port value to be used in EPCL rule
*                                       None
*/
static GT_VOID prvTgfPclTargetPortSelectTestIpclRulesAndActionsSet
(
    GT_U32 targetPort
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: EPCL action - Modify UP */
    action.egressPolicy =  GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.qos.egressUp0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
    action.qos.up       = PRV_TGF_UP_MODIFY_CNS;

    /* AUTODOC: EPCL Mask for Target Port */
    mask.ruleEgrExtIpv4RaclVacl.egressIpCommon.trgPhysicalPort = 0xFF;

    /* AUTODOC: EPCL Pattern for Target Port */
    pattern.ruleEgrExtIpv4RaclVacl.egressIpCommon.trgPhysicalPort = targetPort;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,
            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
                                 EPCL_MATCH_INDEX_CNS, targetPort);
}

/**
* @internal prvTgfPclTargetPortSelectTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfPclTargetPortSelectTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;
    CPSS_INTERFACE_INFO_STC physicalInfo;
    CPSS_CSCD_LINK_TYPE_STC cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN with port 0 & 1 as members. */
    prvTgfPclTargetPortSelectTestVlanInit(PRV_TGF_VLANID_CNS);

    /* AUTODOC: save cscd port1 type for restore */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                               CPSS_PORT_DIRECTION_TX_E, &cscdPortTypeForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet: %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_TX_E,
                                 cscdPortTypeForRestore);

    /* AUTODOC: set cscd port1 type for TX */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                               CPSS_CSCD_PORT_DSA_MODE_EXTEND_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d, %d, %d",
                                 CPSS_PORT_DIRECTION_TX_E,
                                 prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                 CPSS_CSCD_PORT_DSA_MODE_EXTEND_E);

    /* AUTODOC: save target port selection mode for restore */
    rc = prvTgfPclEgressTargetPortSelectionModeGet(prvTgfDevNum, &portSelectionModeForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclEgressTargetPortSelectionModeGet: %d %d",
                                 prvTgfDevNum, portSelectionModeForRestore);

    /* AUTODOC: set target port selection mode to LOCAL */
    rc = prvTgfPclEgressTargetPortSelectionModeSet(
                        PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressTargetPortSelectionModSet: %d",
                                 PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);

    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* AUTODOC: Set FDB entry with local ePort */
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.isStatic = GT_TRUE;
    prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.portNum = PRV_TGF_TARGET_EPORT_CNS;
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* AUTODOC: save e2phy mapping for restore */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_TARGET_EPORT_CNS,
                                                           &physicalInfoForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = PRV_TGF_DSA_DEV_CNS;
    physicalInfo.devPort.portNum = PRV_TGF_DSA_PORT_CNS;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TARGET_EPORT_CNS,
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", prvTgfDevNum);

    /* AUTODOC: save cascade mapping for restore */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS,
                                  &cascadeLinkForRestore, &srcPortTrunkHashEnForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS);

    cascadeLink.linkNum = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    srcPortTrunkHashEn = PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;

    /* AUTODOC: set cascade mapping for remote device\port */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS,
                                  &cascadeLink, srcPortTrunkHashEn, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS);

    /* AUTODOC: Init EPCL Engine for port 1 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E  /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E);


}

/**
* @internal prvTgfPclTargetPortSelectTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPclTargetPortSelectTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate EPCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E,
                                     EPCL_MATCH_INDEX_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_EXT_E,
                                 EPCL_MATCH_INDEX_CNS,
                                 GT_FALSE);

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

    /* AUTODOC: restore cscd port1 type */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                               cscdPortTypeForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d, %d, %d",
                                 CPSS_PORT_DIRECTION_RX_E,
                                 prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                 cscdPortTypeForRestore);

    /* AUTODOC: restore source port selection mode */
    rc = prvTgfPclEgressTargetPortSelectionModeSet(portSelectionModeForRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressTargetPortSelectionModSet: %d",
                                 portSelectionModeForRestore);

    /* AUTODOC: restore e2phy mapping */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_TARGET_EPORT_CNS,
                                                           &physicalInfoForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);

    /* AUTODOC: restore cascade mapping */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS,
                                  &cascadeLinkForRestore, srcPortTrunkHashEnForRestore, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclTargetPortSelectTestPacketSend function
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
static GT_VOID prvTgfPclTargetPortSelectTestPacketSend
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
* @internal prvTgfPclTargetPortSelectTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPclTargetPortSelectTestGenerateTraffic
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_HW_DEV_NUM                   hwDevNum;



    GT_U32  actualCapturedNumOfPackets;

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];

    /* AUTODOC: build expected DSA tag for compare*/
    cpssOsMemSet(&prvTgfPacketDsaTagPart.dsaInfo.forward, 0,
                 sizeof(prvTgfPacketDsaTagPart.dsaInfo.forward));
    prvTgfPacketDsaTagPart.dsaInfo.forward.dstInterface.type =
                                                        CPSS_INTERFACE_PORT_E;
    prvTgfPacketDsaTagPart.dsaInfo.forward.dstInterface.devPort.hwDevNum =
                                                            PRV_TGF_DSA_DEV_CNS;
    prvTgfPacketDsaTagPart.dsaInfo.forward.dstInterface.devPort.portNum =
                                                            PRV_TGF_DSA_PORT_CNS;
    prvTgfPacketDsaTagPart.dsaInfo.forward.srcIsTagged =
                                                PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;
    prvTgfPacketDsaTagPart.dsaInfo.forward.source.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart.dsaInfo.forward.origSrcPhy.portNum = prvTgfPacketDsaTagPart.dsaInfo.forward.source.portNum;


    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);
    prvTgfPacketDsaTagPart.dsaInfo.forward.srcHwDev = hwDevNum;
    prvTgfPacketDsaTagPart.dsaInfo.forward.egrFilterRegistered = GT_TRUE; /* Known UC */

    /* AUTODOC: Set Rule match local target port */
    prvTgfPclTargetPortSelectTestIpclRulesAndActionsSet(
                            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS]);

    /* AUTODOC: Send packet                                     */
    /* AUTODOC: - expect match in EPCL -> packet UP modified        */
    prvTgfPclTargetPortSelectTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            PRV_TGF_RECV_PORT_IDX_CNS,
                                            &prvTgfPacketInfo);

    prvTgfPacketDsaTagPart.commonParams.vpt = PRV_TGF_UP_MODIFY_CNS;
    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &prvTgfExpextedPacketInfo,
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
    prvTgfPacketDsaTagPart.commonParams.vpt = 0;

    /* AUTODOC: Set target port selection mode to FINAL */
    rc = prvTgfPclEgressTargetPortSelectionModeSet(
                    PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressTargetPortSelectionModSet: %d",
                                 PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E);

    /* AUTODOC: Send packet                                     */
    /* AUTODOC: - No match in EPCL -> packet UP NOT modified        */
    prvTgfPclTargetPortSelectTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            PRV_TGF_RECV_PORT_IDX_CNS,
                                            &prvTgfPacketInfo);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &prvTgfExpextedPacketInfo,
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

    /* AUTODOC: Set Rule match final target port */
    prvTgfPclTargetPortSelectTestIpclRulesAndActionsSet(PRV_TGF_DSA_PORT_CNS);

    /* AUTODOC: Send DSA packet                                     */
    /* AUTODOC: - expect match in EPCL -> packet UP modified        */
    prvTgfPclTargetPortSelectTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            PRV_TGF_RECV_PORT_IDX_CNS,
                                            &prvTgfPacketInfo);

    prvTgfPacketDsaTagPart.commonParams.vpt = PRV_TGF_UP_MODIFY_CNS;
    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &prvTgfExpextedPacketInfo,
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
    prvTgfPacketDsaTagPart.commonParams.vpt = 0;

    /* AUTODOC: Set traget port selection mode to LOCAL */
    rc = prvTgfPclEgressTargetPortSelectionModeSet(
                    PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressTargetPortSelectionModSet: %d",
                                 PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);

    /* AUTODOC: Send DSA packet                                     */
    /* AUTODOC: - No match in EPCL -> packet UP NOT modified        */
    prvTgfPclTargetPortSelectTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            PRV_TGF_RECV_PORT_IDX_CNS,
                                            &prvTgfPacketInfo);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &prvTgfExpextedPacketInfo,
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
}

/**
* @internal prvTgfPclTargetPortSelectTest function
* @endinternal
*
* @brief   EPCL Target Port Selecting test
*/
GT_VOID prvTgfPclTargetPortSelectTest
(
    GT_VOID
)
{
    prvTgfPclTargetPortSelectTestConfigurationSet();

    prvTgfPclTargetPortSelectTestGenerateTraffic();

    prvTgfPclTargetPortSelectTestConfigurationRestore();
}

