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
* @file prvTgfPclTrunkHashVal.c
*
* @brief Ingress PCL Trunk Hash Value using
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclTrunkHashVal.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
#define LOOKUP_1_IPCL_RULE_INDEX_CNS     prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(1,0)

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
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

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x56},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOthEtherTypePart =
{0x3456};

/* VLAN tag1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    PRV_TGF_PCL_TEST_UP0  /*pri*/,
    PRV_TGF_PCL_TEST_CFI0 /*cfi*/,
    PRV_TGF_PCL_TEST_VID0 /*vid*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfEthOthPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOthEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of ETH_OTHER packet */
#define PRV_TGF_ETH_OTHER_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS \
    + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfEthOtherPacketInfo =
{
    PRV_TGF_ETH_OTHER_PACKET_LEN_CNS,              /* totalLen */
    (sizeof(prvTgfEthOthPacketPartArray)
        / sizeof(prvTgfEthOthPacketPartArray[0])), /* numOfParts */
    prvTgfEthOthPacketPartArray                    /* partsArray */
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
* @internal prvTgfPclTrunkHashVlanConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclTrunkHashVlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG1_MAC(
        "\nVLAN %d CONFIGURATION:\n",
        PRV_TGF_PCL_TEST_VID0);
    PRV_UTF_LOG4_MAC("  Port members: [%d], [%d], [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1],
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 5 with all ports with TAG0 cmd */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_TEST_VID0,
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with all ports with TAG0 cmd */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_MODIFIED_VLANID_0_CNS,
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

    /* AUTODOC: set PVID 5 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        PRV_TGF_PCL_TEST_VID0);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);
}

/**
* @internal prvTgfPclTrunkHashConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclTrunkHashConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable ingress policy on port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E, LOOKUP_1_IPCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);

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
        PRV_TGF_PCL_TEST_VID0);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_PCL_TEST_VID0);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_PCL_MODIFIED_VLANID_0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_PCL_MODIFIED_VLANID_0_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclTrunkHashTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfPclTrunkHashTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc         = GT_OK;

    /* AUTODOC: send 3 Ethernet packets on port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:56, SA=00:66:77:88:99:00 */
    /* AUTODOC:   VID=5, EtherType=0x3456 */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        &prvTgfEthOtherPacketInfo,
        PRV_TGF_PCL_BURST_COUNT_0_CNS      /*burstCount*/,
        0                                  /*numVfd*/,
        NULL                               /*vfdArray[]*/,
        prvTgfDevsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture: %d",
        prvTgfDevNum);
}

/**
* @internal prvTgfPclTrunkHashTrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] portIndex                - port index
* @param[in] egressVid                - VID found in egressed packets VLAN Tag
*                                       None
*/
GT_VOID prvTgfPclTrunkHashTrafficEgressVidCheck
(
    IN GT_U32  portIndex,
    IN GT_U16  egressVid
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[portIndex];
    portInterface.devPort.portNum = prvTgfPortsArray[portIndex];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = 14; /* 6 + 6 + 2 */
    vfdArray[0].cycleCount = 2;
    vfdArray[0].patternPtr[0] = (GT_U8)((egressVid >> 8) & 0xFF);
    vfdArray[0].patternPtr[1] = (GT_U8)(egressVid & 0xFF);

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
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "found patterns bitmap");

}

/**
* @internal prvTgfPclTrunkHashPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
static GT_VOID prvTgfPclTrunkHashPclConfigurationSet
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

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup_1 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_1_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

    /* set PCL rule 0 - CMD_DROP_HARD packet 1 - any Not Ipv4 with MAC DA = ... 34 02 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIndex                   = LOOKUP_1_IPCL_RULE_INDEX_CNS;
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    /* UDB17 that can comtain the trunk hash value */
    mask.ruleStdNotIp.udb[17]    = 0xFF;
    pattern.ruleStdNotIp.udb[17] = (GT_U8)
        (0x3F & (prvTgfPacketL2Part.daMac[5] ^ prvTgfPacketL2Part.saMac[5]));

    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    /* vlanCmd - for ExMxPm, modifyVlan for DxCh */
    action.vlan.vlanCmd         = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.modifyVlan      = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId          = PRV_TGF_PCL_MODIFIED_VLANID_0_CNS;
    action.vlan.precedence      =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    /* AUTODOC: set PCL rule 0 with: */
    /* AUTODOC:   format - INGRESS_STD_NOT_IP */
    /* AUTODOC:   cmd DROP_HARD */
    /* AUTODOC:   pattern MAC DA=XX:XX:XX:XX:34:02 */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);
}


/**
* @internal prvTgfPclTrunkHashTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic and check results
*/
static GT_VOID prvTgfPclTrunkHashTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS                               rc         = GT_OK;
    PRV_TGF_PCL_OVERRIDE_UDB_TRUNK_HASH_STC overrideTrunkHash;

    /* reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: GENERATE TRAFFIC: */
    overrideTrunkHash.trunkHashEnableStdNotIp   = GT_TRUE;
    overrideTrunkHash.trunkHashEnableStdIpv4L4  = GT_FALSE;
    overrideTrunkHash.trunkHashEnableStdUdb     = GT_FALSE;
    overrideTrunkHash.trunkHashEnableExtNotIpv6 = GT_FALSE;
    overrideTrunkHash.trunkHashEnableExtIpv6L2  = GT_FALSE;
    overrideTrunkHash.trunkHashEnableExtIpv6L4  = GT_FALSE;
    overrideTrunkHash.trunkHashEnableExtUdb     = GT_FALSE;

    /* AUTODOC: enable UDB override for Std Not Ip Key, by Trunk Hash value */
    rc = prvTgfPclOverrideUserDefinedBytesByTrunkHashSet(
        &overrideTrunkHash);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclOverrideUserDefinedBytesByTrunkHashSet: %d", prvTgfDevNum);

    prvTgfPclTrunkHashTrafficGenerate();

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   expected 3 packets with VID=6 in VLAN tag */
    prvTgfPclTrunkHashTrafficEgressVidCheck(
        PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS,
        PRV_TGF_PCL_MODIFIED_VLANID_0_CNS);

    /* AUTODOC: disable UDB override for Std Not Ip Key, by Trunk Hash value */
    overrideTrunkHash.trunkHashEnableStdNotIp   = GT_FALSE;

    rc = prvTgfPclOverrideUserDefinedBytesByTrunkHashSet(
        &overrideTrunkHash);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclOverrideUserDefinedBytesByTrunkHashSet: %d", prvTgfDevNum);

    prvTgfPclTrunkHashTrafficGenerate();

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   expected 3 packets with VID=5 in VLAN tag */
    prvTgfPclTrunkHashTrafficEgressVidCheck(
        PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS,
        PRV_TGF_PCL_TEST_VID0);
}

/**
* @internal prvTgfPclTrunkHashTrafficTest function
* @endinternal
*
* @brief   Full Trunk Hash value test
*/
GT_VOID prvTgfPclTrunkHashTrafficTest
(
    GT_VOID
)
{
    prvTgfPclTrunkHashVlanConfigurationSet();

    prvTgfPclTrunkHashPclConfigurationSet();

    prvTgfPclTrunkHashTrafficGenerateAndCheck();

    prvTgfPclTrunkHashConfigurationRestore();
}

