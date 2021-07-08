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
* @file prvTgfPclLion2.c
*
* @brief Lion2 specific PCL features testing
*
* @version   5
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
#include <pcl/prvTgfPclLion2.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

/* initial VIDs in two tags */
#define PRV_TGF_INIT_VID0_CNS  2
#define PRV_TGF_INIT_VID1_CNS  10

/* IPCL modified VID */
#define PRV_TGF_NEW_IPCL_VID0_CNS  3
#define PRV_TGF_NEW_IPCL_VID1_CNS  11

/* EPCL modified VID */
#define PRV_TGF_NEW_EPCL_VID0_CNS  4
#define PRV_TGF_NEW_EPCL_VID1_CNS  12

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_CNS  1
#define PRV_TGF_EPCL_RULE_INDEX_CNS  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(3)

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_CNS  1
#define PRV_TGF_EPCL_PCL_ID_CNS  2

/**/
typedef enum
{
    PRV_CFG_OPTIONS_EXT_UDB_E = 1
} PRV_CFG_OPTIONS_ENT;

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
    0, /* tag1 UP */
    0, /* tag1 CFI*/
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
* @internal prvTgfPclLion2VidAndExtCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclLion2VidAndExtCountersEthReset
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
};

/**
* @internal prvTgfPclLion2VidAndExtConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclLion2VidAndExtConfigurationSet
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

    /* create a vlan on ports (0,0), (0,8), (0,18), (0,23) */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_INIT_VID0_CNS,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* create a vlan on ports (0,0), (0,8), (0,18), (0,23) */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_NEW_IPCL_VID0_CNS,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* create a vlan on ports (0,0), (0,8), (0,18), (0,23) */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_NEW_EPCL_VID0_CNS,
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

    /* set default vlanId */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set FDB MAC entry for packet */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        packet1TgfL2Part.daMac, PRV_TGF_INIT_VID0_CNS,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* set FDB MAC entry for packet */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        packet1TgfL2Part.daMac, PRV_TGF_NEW_IPCL_VID0_CNS,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxModeSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfPclLion2VidAndExtConfigurationIpclAdd function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration
*/
static GT_VOID prvTgfPclLion2VidAndExtConfigurationIpclAdd
(
    void
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;


    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
/*xxx    lookupCfg.enableKeyExtension     = GT_TRUE;*/
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* init PCL Engine for sending to port*/
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

#if 0
    if (cfgBitmap & PRV_CFG_OPTIONS_EXT_UDB_E)
    {
        /* used UDB23-39 that not muxet with other fields */
        for (i = 0; (i < 17); i++)
        {
            rc = prvTgfPclUserDefinedByteSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                CPSS_PCL_DIRECTION_INGRESS_E,
                (i + 23) /*udbIndex*/,
                PRV_TGF_PCL_OFFSET_L2_E,
                /*offset - the i-th byte of payload (after ethertype) */
                (GT_U8)(6/*DA*/ + 6/*SA*/
                 + 4/*Tag0*/ + 4/*Tag1*/
                 + 2/*ethType*/ + i));
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "prvTgfPclUserDefinedByteSet: index %d, offType %d",
                (i + 23), PRV_TGF_PCL_OFFSET_L2_E);
        }
    }
#endif
    /* Ingress rule */
    /* Forward, modify VID0 and VID1 */
    /* match fields                  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = PRV_TGF_IPCL_RULE_INDEX_CNS;

    mask.ruleStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleStdNotIp.common.pclId = PRV_TGF_IPCL_PCL_ID_CNS;
/*
    if (cfgBitmap & PRV_CFG_OPTIONS_EXT_UDB_E)
    {
        for (i = 0; (i < 17); i++)
        {
            mask.ruleStdNotIp.extension.extUdb[i] = 0xFF;
            pattern.ruleStdNotIp.extension.extUdb[i] =
                packet1TgfPayloadDataArr[i];
        }
    }
*/
    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_NEW_IPCL_VID0_CNS;
    action.vlan.ingressVlanId1Cmd =
        PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E;
    action.vlan.vlanId1 = PRV_TGF_NEW_IPCL_VID1_CNS;

    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex,
        0 CPSS_TBD_BOOKMARK,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclLion2VidAndExtConfigurationEpclAdd function
* @endinternal
*
* @brief   Adds Egress Pcl configuration
*/
static GT_VOID prvTgfPclLion2VidAndExtConfigurationEpclAdd
(
    void
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;


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

    /* init PCL Engine for receiving to port*/
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

#if 0
    if (cfgBitmap & PRV_CFG_OPTIONS_EXT_UDB_E)
    {
        /* used UDB0-2 that not muxed with other fields */
        for (i = 0; (i < 3); i++)
        {
            rc = prvTgfPclUserDefinedByteSet(
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                CPSS_PCL_DIRECTION_EGRESS_E,
                i /*udbIndex*/,
                PRV_TGF_PCL_OFFSET_L2_E,
                /*offset - the i-th byte of payload (after ethertype) */
                (GT_U8)(6/*DA*/ + 6/*SA*/
                 + 4/*Tag0*/ + 4/*Tag1*/
                 + 2/*ethType*/ + i));
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "prvTgfPclUserDefinedByteSet: index %d, offType %d",
                i, PRV_TGF_PCL_OFFSET_L2_E);
        }
    }
#endif
    /* Egress rule */
    /* Forward, modify VID0 and VID1 */
    /* match fields                  */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    ruleIndex  = PRV_TGF_EPCL_RULE_INDEX_CNS;

    mask.ruleEgrStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleEgrStdNotIp.common.pclId = PRV_TGF_EPCL_PCL_ID_CNS;

/* xxx CPSS_TBD_BOOKMARK    if (cfgBitmap & PRV_CFG_OPTIONS_EXT_UDB_E)
    {
        for (i = 0; (i < 3); i++)
        {
            mask.ruleEgrStdNotIp.extension.extUdb[i] = 0xFF;
            pattern.ruleEgrStdNotIp.extension.extUdb[i] =
                packet1TgfPayloadDataArr[i];
        }
    }*/

    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId0Cmd =
        PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        /*PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E*/;
    action.vlan.vlanId  = PRV_TGF_NEW_EPCL_VID0_CNS;
    action.vlan.egressVlanId1ModifyEnable = GT_TRUE;
    action.vlan.vlanId1 = PRV_TGF_NEW_EPCL_VID1_CNS;

    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex,
        0 CPSS_TBD_BOOKMARK,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclLion2VidAndExtConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclLion2VidAndExtConfigurationRestore
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

    /* restore default vlanId to all ports */
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
        PRV_TGF_NEW_IPCL_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_NEW_IPCL_VID0_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_NEW_EPCL_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_NEW_IPCL_VID0_CNS);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_INIT_VID0_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_NEW_IPCL_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_NEW_IPCL_VID0_CNS);
}

/**
* @internal prvTgfPclLion2VidAndExtIpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclLion2VidAndExtIpclRestore
(
    void
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

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_EXT_E,
        PRV_TGF_IPCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);
#if 0
    if (cfgBitmap & PRV_CFG_OPTIONS_EXT_UDB_E)
    {
        /* used UDB23-39 that not muxet with other fields */
        for (i = 0; (i < 17); i++)
        {
            rc = prvTgfPclUserDefinedByteSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                CPSS_PCL_DIRECTION_INGRESS_E,
                (i + 23) /*udbIndex*/,
                PRV_TGF_PCL_OFFSET_INVALID_E,
                0 /*offset*/);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "prvTgfPclUserDefinedByteSet: index %d, offType %d",
                (i + 23), PRV_TGF_PCL_OFFSET_INVALID_E);
        }
    }
#endif

}

/**
* @internal prvTgfPclLion2VidAndExtEpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclLion2VidAndExtEpclRestore
(
    void
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
#if 0
    /* used UDB0-2 that not muxed with other fields */
    for (i = 0; (i < 3); i++)
    {
        rc = prvTgfPclUserDefinedByteSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
            PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
            CPSS_PCL_DIRECTION_EGRESS_E,
            i /*udbIndex*/,
            PRV_TGF_PCL_OFFSET_INVALID_E,
            0/*offset*/);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclUserDefinedByteSet: index %d, offType %d",
            i, PRV_TGF_PCL_OFFSET_INVALID_E);
    }
#endif
}

/**
* @internal prvTgfPclLion2VidAndExtTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfPclLion2VidAndExtTrafficGenerate
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

    prvTgfPclLion2VidAndExtCountersEthReset();

    tgfTrafficTracePacketByteSet(GT_TRUE);

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
* @internal prvTgfPclLion2VidAndExtTrafficCheck function
* @endinternal
*
* @brief   Checks traffic egress VID0 and VID1 in the Tags
*/
GT_VOID prvTgfPclLion2VidAndExtTrafficCheck
(
    GT_U32 upCfiVid0,
    GT_U32 upCfiVid1
)
{
    GT_STATUS                       rc  = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[2];
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

    /* Tag1 */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].offset = 18; /* 6 + 6 + 4 + 2 */
    vfdArray[1].cycleCount = 2;
    vfdArray[1].patternPtr[0] = (GT_U8)((upCfiVid1 >> 8) & 0xFF);
    vfdArray[1].patternPtr[1] = (GT_U8)(upCfiVid1 & 0xFF);

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            2 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(
        3, numTriggersBmp, "found patterns bitmap");

}

/**
* @internal prvTgfPclLion2VidAndExtTrafficTest function
* @endinternal
*
* @brief   Full Egress Original VID field test
*/
GT_VOID prvTgfPclLion2VidAndExtTrafficTest
(
    GT_VOID
)
{
    prvTgfPclLion2VidAndExtConfigurationSet();

    PRV_UTF_LOG0_MAC(
        "*** Base Lion2 Vid and Ext IPCL test - no fields *** \n");

    prvTgfPclLion2VidAndExtConfigurationIpclAdd();

    prvTgfPclLion2VidAndExtTrafficGenerate();

    prvTgfPclLion2VidAndExtTrafficCheck(
        PRV_TGF_NEW_IPCL_VID0_CNS, PRV_TGF_NEW_IPCL_VID1_CNS);

    prvTgfPclLion2VidAndExtIpclRestore();

    PRV_UTF_LOG0_MAC(
        "*** Base Lion2 Vid and Ext EPCL test - no fields *** \n");

    prvTgfPclLion2VidAndExtConfigurationEpclAdd();

    prvTgfPclLion2VidAndExtTrafficGenerate();

    prvTgfPclLion2VidAndExtTrafficCheck(
        PRV_TGF_NEW_EPCL_VID0_CNS, PRV_TGF_NEW_EPCL_VID1_CNS);

    prvTgfPclLion2VidAndExtEpclRestore();
    PRV_UTF_LOG0_MAC(
        "*** UDBs Lion2 Vid and Ext IPCL test *** \n");

#if 0
    prvTgfPclLion2VidAndExtConfigurationIpclAdd(PRV_CFG_OPTIONS_EXT_UDB_E);

    prvTgfPclLion2VidAndExtTrafficGenerate();

    prvTgfPclLion2VidAndExtTrafficCheck(
        PRV_TGF_NEW_IPCL_VID0_CNS, PRV_TGF_NEW_IPCL_VID1_CNS);

    prvTgfPclLion2VidAndExtIpclRestore(PRV_CFG_OPTIONS_EXT_UDB_E);

    PRV_UTF_LOG0_MAC(
        "*** UDBs Lion2 Vid and Ext EPCL test *** \n");

    prvTgfPclLion2VidAndExtConfigurationEpclAdd(PRV_CFG_OPTIONS_EXT_UDB_E);

    prvTgfPclLion2VidAndExtTrafficGenerate();

    prvTgfPclLion2VidAndExtTrafficCheck(
        PRV_TGF_NEW_EPCL_VID0_CNS, PRV_TGF_NEW_EPCL_VID1_CNS);

    prvTgfPclLion2VidAndExtEpclRestore(PRV_CFG_OPTIONS_EXT_UDB_E);

    PRV_UTF_LOG0_MAC(
        "*** End of Lion2 Vid and Ext tests *** \n");
#endif

    prvTgfPclLion2VidAndExtConfigurationRestore();
}


