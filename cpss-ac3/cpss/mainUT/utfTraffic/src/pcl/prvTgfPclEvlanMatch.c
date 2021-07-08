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
* @file prvTgfPclEvlanMatch.c
*
* @brief ePort and eVlan specific PCL features testing
*
* @version   6
********************************************************************************
*/

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
#include <pcl/prvTgfPclMiscellanous.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

/* initial VIDs in two tags */
#define PRV_TGF_INIT_VID0_CNS  2
#define PRV_TGF_INIT_VID1_CNS  100

/* IPCL modified VID */
#define PRV_TGF_NEW_IPCL_VID1_CNS  3

/* EPCL modified VID */
#define PRV_TGF_NEW_EPCL_VID1_CNS  4

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_CNS  1
#define PRV_TGF_EPCL_RULE_INDEX_CNS  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(3)

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_CNS  1
#define PRV_TGF_EPCL_PCL_ID_CNS  2

/* eVID value */
#define PRV_TGF_EVID_CNS  0x0FFC

/* L2 part of packet */
static TGF_PACKET_L2_STC packet1TgfL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
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
    0, /* tag0 UP */
    0, /* tag0 CFI*/
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
     + TGF_VLAN_TAG_SIZE_CNS
     + TGF_VLAN_TAG_SIZE_CNS
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

/* saved Vlan Range */
static GT_U16   saveVlanRange;



/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclEvlanMatchCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclEvlanMatchCountersEthReset
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
* @internal prvTgfPclEvlanMatchConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclEvlanMatchConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;

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

    /* AUTODOC: create VLAN 0x1FFC with all tagged OUTER_TAG0_INNER_TAG1 ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_EVID_CNS,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    rc = prvTgfBrgVlanRangeGet(&saveVlanRange);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanRangeGet: %d");

    /* AUTODOC: config valid VLAN range to 0x1FFD */
    rc = prvTgfBrgVlanRangeSet(PRV_TGF_EVID_CNS + 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanRangeSet");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 0x1FFC for port 1 */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_EVID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable PVID forcing on port 1 */
    rc = prvTgfBrgVlanPortForcePvidEnable(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortForcePvidEnable: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxModeSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC  VLAN, ePort  */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        packet1TgfL2Part.daMac, PRV_TGF_EVID_CNS,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

}

/**
* @internal prvTgfPclEvlanMatchConfigurationIpclAdd function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration
*/
static GT_VOID prvTgfPclEvlanMatchConfigurationIpclAdd
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
    GT_U32                           i;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
/* xxx   lookupCfg.enableKeyExtension     = GT_TRUE;*/
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
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

    /* Ingress rule */
    /* Forward, modify VID0  */
    /* match fields                  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = PRV_TGF_IPCL_RULE_INDEX_CNS;

    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_IPCL_PCL_ID_CNS;

    /* configures both bits[11:0] in the key */
    /* and bits[15:12] in the extension      */
    mask.ruleStdNotIp.common.vid    = 0xFFF;
    pattern.ruleStdNotIp.common.vid = PRV_TGF_EVID_CNS;

    for (i = 0; (i < 6); i++)
    {
        mask.ruleStdNotIp.macDa.arEther[i] = 0xFF;
        mask.ruleStdNotIp.macSa.arEther[i] = 0xFF;
        pattern.ruleStdNotIp.macDa.arEther[i] =
            packet1TgfL2Part.daMac[i];
        pattern.ruleStdNotIp.macSa.arEther[i] =
            packet1TgfL2Part.saMac[i];
    }

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.ingressVlanId1Cmd =
        PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E;
    action.vlan.vlanId1     = PRV_TGF_NEW_IPCL_VID1_CNS;

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern DA=00:00:00:00:34:02, SA=00:00:00:00:00:55, VID=0x1FFC */
    /* AUTODOC:   enable modify VID1 VLAN_ID1_CMD_ALL, VID=3 */
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex,
        0 CPSS_TBD_BOOKMARK,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEvlanMatchConfigurationEpclAdd function
* @endinternal
*
* @brief   Adds Egress Pcl configuration
*/
static GT_VOID prvTgfPclEvlanMatchConfigurationEpclAdd
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
    GT_U32                           i;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
/*xxx    lookupCfg.enableKeyExtension     = GT_TRUE;*/
    lookupCfg.pclId                  = PRV_TGF_EPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: init PCL Engine for receive port 2: */
    /* AUTODOC:   egress direction, lookup0 */
    /* AUTODOC:   nonIpKey EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key EGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key EGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* Egress rule */
    /* Forward, modify VID0  */
    /* match fields          */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    ruleIndex  = PRV_TGF_EPCL_RULE_INDEX_CNS;

    mask.ruleEgrStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleEgrStdNotIp.common.pclId = PRV_TGF_EPCL_PCL_ID_CNS;

    for (i = 0; (i < 6); i++)
    {
        mask.ruleEgrStdNotIp.macDa.arEther[i] = 0xFF;
        mask.ruleEgrStdNotIp.macSa.arEther[i] = 0xFF;
        pattern.ruleEgrStdNotIp.macDa.arEther[i] =
            packet1TgfL2Part.daMac[i];
        pattern.ruleEgrStdNotIp.macSa.arEther[i] =
            packet1TgfL2Part.saMac[i];
    }

    /* configures both bits[11:0] in the key */
    /* and bits[15:12] in the extension      */
    mask.ruleEgrStdNotIp.common.vid      = 0xFFF;
    pattern.ruleEgrStdNotIp.common.vid = PRV_TGF_EVID_CNS;

    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId1ModifyEnable = GT_TRUE;
    action.vlan.vlanId1  = PRV_TGF_NEW_EPCL_VID1_CNS;

    /* AUTODOC: set PCL rule 3 with: */
    /* AUTODOC:   format EGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern DA=00:00:00:00:34:02, SA=00:00:00:00:00:55, VID=0x1FFC */
    /* AUTODOC:   enable modify egress VID , VID=4 */
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex,
        0 CPSS_TBD_BOOKMARK,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEvlanMatchConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEvlanMatchConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: disable PVID forcing on port 1 */
    rc = prvTgfBrgVlanPortForcePvidEnable(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortForcePvidEnable: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
        prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: restore default VLAN range */
    rc = prvTgfBrgVlanRangeSet(saveVlanRange);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanRangeSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_INIT_VID0_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_EVID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_NEW_IPCL_VID1_CNS);
}

/**
* @internal prvTgfPclEvlanMatchIpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEvlanMatchIpclRestore
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
        PRV_TGF_IPCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfPclEvlanMatchEpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEvlanMatchEpclRestore
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
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    interfaceInfo.devPort.portNum    =
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_EXT_E,
        PRV_TGF_EPCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfPclEvlanMatchTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfPclEvlanMatchTrafficGenerate
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

    prvTgfPclEvlanMatchCountersEthReset();

    tgfTrafficTracePacketByteSet(GT_TRUE);

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:55 */
    /* AUTODOC:   TAG0 VID=2, TAG1 VID=100, EtherType=0x3333 */
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
* @internal prvTgfPclEvlanMatchTrafficCheck function
* @endinternal
*
* @brief   Checks traffic egress VID1 in the Tag
*/
GT_VOID prvTgfPclEvlanMatchTrafficCheck
(
    GT_U32 upCfiVid1
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
    vfdArray[0].offset = 18; /* 6 + 6 + 4 + 2 */
    vfdArray[0].cycleCount = 2;
    vfdArray[0].patternPtr[0] = (GT_U8)((upCfiVid1 >> 8) & 0xFF);
    vfdArray[0].patternPtr[1] = (GT_U8)(upCfiVid1 & 0xFF);

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
* @internal prvTgfPclEvlanMatchIpclTrafficTest function
* @endinternal
*
* @brief   Ingress PCL Rule+extension vid field test
*/
GT_VOID prvTgfPclEvlanMatchIpclTrafficTest
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfPclEvlanMatchConfigurationSet();

    PRV_UTF_LOG0_MAC(
        "*** EvlanMatch IPCL test - no fields *** \n");

    prvTgfPclEvlanMatchConfigurationIpclAdd();

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEvlanMatchTrafficGenerate();

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   double tagged packet-TAG0 VID=0xFFC, TAG1 VID=3 received on port 2 */
    prvTgfPclEvlanMatchTrafficCheck(PRV_TGF_NEW_IPCL_VID1_CNS);

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEvlanMatchIpclRestore();

    PRV_UTF_LOG0_MAC(
        "*** End of EvlanMatch IPCL test - no fields *** \n");

    prvTgfPclEvlanMatchConfigurationRestore();
}

/**
* @internal prvTgfPclEvlanMatchEpclTrafficTest function
* @endinternal
*
* @brief   Ingress PCL Rule+extension vid field test
*/
GT_VOID prvTgfPclEvlanMatchEpclTrafficTest
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfPclEvlanMatchConfigurationSet();

    PRV_UTF_LOG0_MAC(
        "*** EvlanMatch EPCL test - no fields *** \n");

    prvTgfPclEvlanMatchConfigurationEpclAdd();

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEvlanMatchTrafficGenerate();

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   double tagged packet-TAG0 VID=0xFFC, TAG1 VID=4 received on port 2 */
    prvTgfPclEvlanMatchTrafficCheck(PRV_TGF_NEW_EPCL_VID1_CNS);

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEvlanMatchEpclRestore();

    PRV_UTF_LOG0_MAC(
        "*** End of EvlanMatch EPCL test - no fields *** \n");

    prvTgfPclEvlanMatchConfigurationRestore();
}



