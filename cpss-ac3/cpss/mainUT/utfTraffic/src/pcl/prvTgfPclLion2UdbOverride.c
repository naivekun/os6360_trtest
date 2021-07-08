/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPclLoin2UdbOverride.c
*
* DESCRIPTION:
*       Lion2 specific UDB overriding fields testing
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfCosGen.h>
#include <pcl/prvTgfPclMiscellanous.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

/* initial VIDs in Tag0 */
#define PRV_TGF_INIT_VID0_CNS  2

/* initial VIDs in Tag1 */
#define PRV_TGF_INIT_VID1_CNS  11

/* initial Qos profile index */
#define PRV_TGF_INIT_QOS_PROFILE_INDEX_CNS  6

/* IPCL modified VID */
#define PRV_TGF_NEW_IPCL_RULE0_VID0_CNS  3
#define PRV_TGF_NEW_IPCL_RULE1_VID0_CNS  5

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE0_INDEX_CNS  1
#define PRV_TGF_IPCL_RULE1_INDEX_CNS  2

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_CNS  1

/* Trunk ID */
#define PRV_TGF_TRUNK_ID_CNS  7

/* L2 part of packet */
static TGF_PACKET_L2_STC packet1TgfL2Part = {
    {0x00, 0xAB, 0xCD, 0xEF, 0x34, 0x02},                /* daMac */
    {0x00, 0x12, 0x13, 0x14, 0x15, 0x55}                 /* saMac */
};

static TGF_PACKET_VLAN_TAG_STC packet1VlanTag0 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    0, /* tag0 UP */
    0, /* tag0 CFI*/
    PRV_TGF_INIT_VID0_CNS
};

static TGF_PACKET_VLAN_TAG_STC packet1VlanTag1 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    6, /* tag0 UP */
    1, /* tag0 CFI*/
    PRV_TGF_INIT_VID1_CNS
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
    {0x3333};

/* DATA of packet */
static GT_U8 packet1TgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC packet1TgfPayloadPart = {
    sizeof(packet1TgfPayloadDataArr),                 /* dataLength */
    packet1TgfPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet1 */
static TGF_PACKET_PART_STC packet1TgfPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E , &packet1VlanTag0},
    {TGF_PACKET_PART_VLAN_TAG_E , &packet1VlanTag1},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC packet1TgfInfo = {
    (TGF_L2_HEADER_SIZE_CNS
     + (2 * TGF_VLAN_TAG_SIZE_CNS)
     + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(packet1TgfPayloadDataArr)),                        /* totalLen */
    (sizeof(packet1TgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    packet1TgfPartArray                                          /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclLion2UdbOverrideCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclLion2UdbOverrideCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; (portIter < PRV_TGF_PORTS_NUM_CNS); portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(
            prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
}

/**
* @internal prvTgfPclLion2UdbOverrideConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclLion2UdbOverrideConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS               rc;
    CPSS_QOS_ENTRY_STC      portQosCfg;

    PRV_UTF_LOG1_MAC(
        "\nVLAN %d CONFIGURATION:\n",
        PRV_TGF_INIT_VID0_CNS);
    PRV_UTF_LOG4_MAC("  Port members: [%d], [%d], [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1],
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 2 with all tagged OUTER_TAG0_INNER_TAG1 ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_INIT_VID0_CNS,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 3 with all tagged OUTER_TAG0_INNER_TAG1 ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_NEW_IPCL_RULE0_VID0_CNS,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 5 with all tagged OUTER_TAG0_INNER_TAG1 ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_NEW_IPCL_RULE1_VID0_CNS,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 2 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set Qos Trust Mode as NO_TRUST for port 1 */
    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_QOS_PORT_NO_TRUST_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosTrustModeSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set port default Qos Profile */
    cpssOsMemSet(&portQosCfg, 0, sizeof(portQosCfg));
    portQosCfg.qosProfileId     = PRV_TGF_INIT_QOS_PROFILE_INDEX_CNS;
    portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

    /* AUTODOC: set QoS profile entriy for port 1 with: */
    /* AUTODOC:   qosProfileId 6 */
    /* AUTODOC:   enable modify UP\DSCP */
    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQosCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosConfigSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxModeSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclLion2UdbOverrideConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclLion2UdbOverrideConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_QOS_ENTRY_STC      portQosCfg;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);

    /* reset port default Qos Profile */
    cpssOsMemSet(&portQosCfg, 0, sizeof(portQosCfg));
    portQosCfg.qosProfileId     = 0;
    portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    /* AUTODOC: restore port 1 default QoS Profile */
    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQosCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosConfigSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: restore default Qos Trust Mode for port 1 */
    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_QOS_PORT_TRUST_L2_L3_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosTrustModeSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
        prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_INIT_VID0_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_NEW_IPCL_RULE0_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_NEW_IPCL_RULE0_VID0_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_NEW_IPCL_RULE1_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_NEW_IPCL_RULE1_VID0_CNS);
}

/**
* @internal prvTgfPclLion2UdbOverrideConfigurationIpclQosProfileAdd function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration
*/
static GT_VOID prvTgfPclLion2UdbOverrideConfigurationIpclQosProfileAdd
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;

    /* AUTODOC: configure User Defined Byte (UDB) idx 5: */
    /* AUTODOC:   format INGRESS_EXT_UDB */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType OFFSET_L3_MINUS_2, offset 0 */
    rc = prvTgfPclUserDefinedByteSet(
        ruleFormat, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E, 5 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 0 /*offset*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d, %d",
        prvTgfDevNum, 5);

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_EXT_UDB */
    /* AUTODOC:   ipv4Key INGRESS_EXT_UDB */
    /* AUTODOC:   ipv6Key INGRESS_EXT_UDB */
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Ingress rule UDB[] == Packet's ethertype[0] */
    /* Forward, modify VID0  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex  = PRV_TGF_IPCL_RULE0_INDEX_CNS;

    mask.ruleExtUdb.commonStd.pclId    = 0x3FF;
    pattern.ruleExtUdb.commonStd.pclId = PRV_TGF_IPCL_PCL_ID_CNS;

    mask.ruleExtUdb.udb[5]    = 0xFF;
    pattern.ruleExtUdb.udb[5] =
        (GT_U8)((prvTgfPacket1EtherTypePart.etherType >> 8) & 0xFF);

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_NEW_IPCL_RULE0_VID0_CNS;

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_EXT_UDB, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=1, udb=0x33 */
    /* AUTODOC:   enable modify VLAN, VID=3 */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);

    /* Ingress rule qosProfile == PRV_TGF_INIT_QOS_PROFILE_INDEX_CNS */
    /* Forward, modify VID0  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex  = PRV_TGF_IPCL_RULE1_INDEX_CNS;

    mask.ruleExtUdb.commonStd.pclId    = 0x3FF;
    pattern.ruleExtUdb.commonStd.pclId = PRV_TGF_IPCL_PCL_ID_CNS;

    mask.ruleExtUdb.qosProfile    = 0x7F;
    pattern.ruleExtUdb.qosProfile = PRV_TGF_INIT_QOS_PROFILE_INDEX_CNS;

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_NEW_IPCL_RULE1_VID0_CNS;

    /* AUTODOC: set PCL rule 2 with: */
    /* AUTODOC:   format INGRESS_EXT_UDB, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=1, qosProfile=6 */
    /* AUTODOC:   enable modify VLAN, VID=5 */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclLion2UdbOverrideIpclQosProfileRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclLion2UdbOverrideIpclQosProfileRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     =
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    interfaceInfo.devPort.portNum    =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_EXT_E,
        PRV_TGF_IPCL_RULE0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);

    /* invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_EXT_E,
        PRV_TGF_IPCL_RULE1_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate User Defined Byte (UDB) idx 5 */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E, 5 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_INVALID_E, 0 /*offset*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d, %d",
        prvTgfDevNum, 5);
}

/**
* @internal prvTgfPclLion2UdbOverrideConfigurationIpclTag1Add function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration
*/
static GT_VOID prvTgfPclLion2UdbOverrideConfigurationIpclTag1Add
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* AUTODOC: configure User Defined Byte (UDB) idx 16: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType OFFSET_L3_MINUS_2, offset 0 */
    rc = prvTgfPclUserDefinedByteSet(
        ruleFormat, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E, 16 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 0 /*offset*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d, %d",
        prvTgfDevNum, 16);

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Ingress rule UDB[] == Packet's ethertype[0] */
    /* Forward, modify VID0  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex  = PRV_TGF_IPCL_RULE0_INDEX_CNS;

    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_IPCL_PCL_ID_CNS;

    /*set UDB16 */
    mask.ruleStdNotIp.udb[16]    = 0xFF;
    pattern.ruleStdNotIp.udb[16] =
        (GT_U8)((prvTgfPacket1EtherTypePart.etherType >> 8) & 0xFF);

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_NEW_IPCL_RULE0_VID0_CNS;

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=1, udb=0x33 */
    /* AUTODOC:   enable modify VLAN, VID=3 */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);

    /* Ingress rule Tag1 fields */
    /* Forward, modify VID0  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex  = PRV_TGF_IPCL_RULE1_INDEX_CNS;

    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_IPCL_PCL_ID_CNS;

    mask.ruleStdNotIp.tag1Exist    = 1;
    pattern.ruleStdNotIp.tag1Exist = 1;
    mask.ruleStdNotIp.cfi1         = 1;
    pattern.ruleStdNotIp.cfi1      = packet1VlanTag1.cfi;
    mask.ruleStdNotIp.up1          = 0x7;
    pattern.ruleStdNotIp.up1       = packet1VlanTag1.pri;
    mask.ruleStdNotIp.vid1         = 0xFFF;
    pattern.ruleStdNotIp.vid1      = packet1VlanTag1.vid;

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_NEW_IPCL_RULE1_VID0_CNS;

    /* AUTODOC: set PCL rule 2 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=1, TAG1: UP=6, CFI=1, VID=11 */
    /* AUTODOC:   enable modify VLAN, VID=5 */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclLion2UdbOverrideIpclTag1Restore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclLion2UdbOverrideIpclTag1Restore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     =
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    interfaceInfo.devPort.portNum    =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E,
        PRV_TGF_IPCL_RULE0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);

    /* invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E,
        PRV_TGF_IPCL_RULE1_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate User Defined Byte (UDB) idx 16 */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E, 16 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_INVALID_E, 0 /*offset*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d, %d",
        prvTgfDevNum, 16);
}

/**
* @internal prvTgfPclLion2UdbOverrideTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfPclLion2UdbOverrideTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     =
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    interfaceInfo.devPort.portNum    =
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    prvTgfPclLion2UdbOverrideCountersEthReset();

    tgfTrafficTracePacketByteSet(GT_TRUE);

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:AB:CD:EF:34:02, SA=00:12:13:14:15:55 */
    /* AUTODOC:   TAG0: VID=2, TAG1: VID=11, UP=6, CFI=1 */
    /* AUTODOC:   EtherType=0x3333 */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &packet1TgfInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        500 /*captureOnTime*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture: %d",
        prvTgfDevNum);

    /* read counters */
    rc = prvTgfReadPortCountersEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfReadPortCountersEth");

    /* read counters */
    rc = prvTgfReadPortCountersEth(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfReadPortCountersEth");
}

/**
* @internal prvTgfPclLion2UdbOverrideTrafficCheck function
* @endinternal
*
* @brief   Checks traffic egress VID0 in the Tag
*/
GT_VOID prvTgfPclLion2UdbOverrideTrafficCheck
(
    GT_U32 upCfiVid0
)
{
    GT_STATUS                       rc  = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* Tag 0 */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = 14; /* 6 + 6 + 2 */
    vfdArray[0].cycleCount = 2;
    vfdArray[0].patternPtr[0] = (GT_U8)((upCfiVid0 >> 8) & 0xFF);
    vfdArray[0].patternPtr[1] = (GT_U8)(upCfiVid0 & 0xFF);

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            1 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(
        1, numTriggersBmp, "found patterns bitmap");

}

/**
* @internal prvTgfPclLion2UdbOverrideIpclQosProfileTrafficTest function
* @endinternal
*
* @brief   Ingress PCL SrcTrunkId field test
*/
GT_VOID prvTgfPclLion2UdbOverrideIpclQosProfileTrafficTest
(
    GT_VOID
)
{
    GT_STATUS                       rc, rc1  = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfPclLion2UdbOverrideConfigurationSet();

    PRV_UTF_LOG0_MAC(
        "*** Lion2UdbOverride IPCL test - qosProfile versus UDB[5] *** \n");

    prvTgfPclLion2UdbOverrideConfigurationIpclQosProfileAdd();

    /* AUTODOC: disable overriding UDB content */
    rc = tgfPclOverrideUserDefinedBytesEnableSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        PRV_TGF_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,
        GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfPclOverrideUserDefinedBytesEnableSet");

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclLion2UdbOverrideTrafficGenerate();

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   double tagged: TAG0 VID=3; TAG1 VID=11, UP=6, CFI=1 */
    prvTgfPclLion2UdbOverrideTrafficCheck(PRV_TGF_NEW_IPCL_RULE0_VID0_CNS);

    /* AUTODOC: enable overriding UDB content */
    rc = tgfPclOverrideUserDefinedBytesEnableSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        PRV_TGF_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,
        GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "tgfPclOverrideUserDefinedBytesEnableSet");

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclLion2UdbOverrideTrafficGenerate();

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   double tagged: TAG0 VID=5; TAG1 VID=11, UP=6, CFI=1 */
    prvTgfPclLion2UdbOverrideTrafficCheck(PRV_TGF_NEW_IPCL_RULE1_VID0_CNS);

    /* AUTODOC: disable overriding UDB content */
    rc = tgfPclOverrideUserDefinedBytesEnableSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        PRV_TGF_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,
        GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfPclOverrideUserDefinedBytesEnableSet");

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclLion2UdbOverrideIpclQosProfileRestore();

    PRV_UTF_LOG0_MAC(
        "*** End of Lion2UdbOverride IPCL test - qosProfile versus UDB[5] *** \n");

    prvTgfPclLion2UdbOverrideConfigurationRestore();

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfPclLion2UdbOverrideIpclTag1TrafficTest function
* @endinternal
*
* @brief   Ingress PCL SrcTrunkId field test
*/
GT_VOID prvTgfPclLion2UdbOverrideIpclTag1TrafficTest
(
    GT_VOID
)
{
    GT_STATUS                       rc, rc1  = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfPclLion2UdbOverrideConfigurationSet();

    PRV_UTF_LOG0_MAC(
        "*** Lion2UdbOverride IPCL test - Tag1 versus UDB[16] *** \n");

    prvTgfPclLion2UdbOverrideConfigurationIpclTag1Add();

    /* AUTODOC: disable overriding UDB content */
    rc = tgfPclOverrideUserDefinedBytesEnableSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_UDB_OVERRIDE_TYPE_TAG1_INFO_E,
        GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfPclOverrideUserDefinedBytesEnableSet");

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclLion2UdbOverrideTrafficGenerate();

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   double tagged: TAG0 VID=3; TAG1 VID=11, UP=6, CFI=1 */
    prvTgfPclLion2UdbOverrideTrafficCheck(PRV_TGF_NEW_IPCL_RULE0_VID0_CNS);

    /* AUTODOC: enable overriding UDB content */
    rc = tgfPclOverrideUserDefinedBytesEnableSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_UDB_OVERRIDE_TYPE_TAG1_INFO_E,
        GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "tgfPclOverrideUserDefinedBytesEnableSet");

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclLion2UdbOverrideTrafficGenerate();

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   double tagged: TAG0 VID=5; TAG1 VID=11, UP=6, CFI=1 */
    prvTgfPclLion2UdbOverrideTrafficCheck(PRV_TGF_NEW_IPCL_RULE1_VID0_CNS);

    /* AUTODOC: disable overriding UDB content */
    rc = tgfPclOverrideUserDefinedBytesEnableSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_UDB_OVERRIDE_TYPE_TAG1_INFO_E,
        GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfPclOverrideUserDefinedBytesEnableSet");

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclLion2UdbOverrideIpclTag1Restore();

    PRV_UTF_LOG0_MAC(
        "*** End of Lion2UdbOverride IPCL test - Tag1 versus UDB[16] *** \n");

    prvTgfPclLion2UdbOverrideConfigurationRestore();

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}


