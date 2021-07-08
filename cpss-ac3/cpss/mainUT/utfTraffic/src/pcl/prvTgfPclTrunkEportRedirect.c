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
* @file prvTgfPclTrunkEportRedirect.c
*
* @brief FDB learning with trunk and eport modification test
*
* @version   2
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
#include <common/tgfL2MllGen.h>
#include <common/tgfTrunkGen.h>
#include <pcl/prvTgfPclMiscellanous.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* initial VIDs in two tags */
#define PRV_TGF_INIT_VID0_CNS  2

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_CNS  1

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_CNS  1

/* Trunk ID */
#define PRV_TGF_TRUNK_ID_CNS  2
/* ePort value */
#define PRV_TGF_TRG_EPORT_VALUE_MAC  20

/* value to ignore the IPCL Cfg parameter */
#define PRV_IPCL_CFG_INVALID_EPORT_CNS      0xFFFFFF
#define PRV_IPCL_CFG_INVALID_EVID_CNS       0
#define PRV_IPCL_CFG_INVALID_EVIDX_CNS      0
#define PRV_IPCL_CFG_INVALID_ESRCEPORT_CNS  0xFFFFFF


/* L2 part of packet 1*/
static TGF_PACKET_L2_STC packet1TgfL2Part = {
    {0x00, 0x8a, 0x5c, 0x71, 0x90, 0x36},                /* daMac */
    {0x00, 0xfe, 0x98, 0x76, 0x43, 0x55}                 /* saMac */
};
static TGF_PACKET_VLAN_TAG_STC packet1VlanTag0 =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    0, /* tag0 UP */
    0, /* tag0 CFI*/
    PRV_TGF_INIT_VID0_CNS
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
    {0x3333};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket2EtherTypePart =
    {0x2222};

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
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC packet1TgfInfo = {
    (TGF_L2_HEADER_SIZE_CNS
     + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(packet1TgfPayloadDataArr)),                       /* totalLen */
    (sizeof(packet1TgfPartArray) / sizeof(TGF_PACKET_PART_STC)),/* numOfParts */
    packet1TgfPartArray                                         /* partsArray */
};

/* PARTS of packet2 */
static TGF_PACKET_PART_STC packet2TgfPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E , &packet1VlanTag0},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket2EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC packet2TgfInfo = {
    (TGF_L2_HEADER_SIZE_CNS
     + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(packet1TgfPayloadDataArr)),                       /* totalLen */
    (sizeof(packet2TgfPartArray) / sizeof(TGF_PACKET_PART_STC)),/* numOfParts */
    packet2TgfPartArray                                         /* partsArray */
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
* @internal prvTgfPclTrunkEportRedirectConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclTrunkEportRedirectConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    CPSS_TRUNK_MEMBER_STC            trunkMembers[1];

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);

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

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID 2 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: create trunk ID 2 with port 0 */
    trunkMembers[0].hwDevice =  prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    trunkMembers[0].port   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    rc = prvTgfTrunkMembersSet(
        PRV_TGF_TRUNK_ID_CNS,
        1,                              /*numOfEnabledMembers*/
        trunkMembers,                   /*enabledMembersArray*/
        0,                              /*numOfDisabledMembers*/
        (CPSS_TRUNK_MEMBER_STC*)NULL);  /*disabledMembersArray*/

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfTrunkMembersSet");
}

/**
* @internal prvTgfPclTrunkEportRedirectConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclTrunkEportRedirectConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);

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

    /* AUTODOC: invalidate trunk entry*/

    rc = prvTgfTrunkMembersSet(
        PRV_TGF_TRUNK_ID_CNS,
        0,                              /*numOfEnabledMembers*/
        (CPSS_TRUNK_MEMBER_STC*)NULL,   /*enabledMembersArray*/
        0,                              /*numOfDisabledMembers*/
        (CPSS_TRUNK_MEMBER_STC*)NULL);  /*disabledMembersArray*/

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfTrunkMembersSet");


   /* AUTODOC : disable all PCL configurations on the ports */
   prvTgfPclPortsRestoreAll();

}

/**
* @internal prvTgfPclTrunkEportRedirectConfigurationIpclAdd function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration
*
* @param[in] ePort                    - send port index
* @param[in] eSrcPort                 - eport value to reassign
*                                       None
*/
static GT_VOID prvTgfPclTrunkEportRedirectConfigurationIpclAdd
(
    IN GT_PORT_NUM ePort,
    IN GT_PORT_NUM eSrcPort
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
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: init PCL Engine for send port 0: */
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

    mask.ruleStdNotIp.common.pclId          = 0x3FF;
    pattern.ruleStdNotIp.common.pclId       = PRV_TGF_IPCL_PCL_ID_CNS;

    mask.ruleStdNotIp.common.sourcePort     = 0xFF;
    pattern.ruleStdNotIp.common.sourcePort  = prvTgfPortsArray[ePort] & 0xFF;

    for (i = 0; (i < 6); i++)
    {
        mask.ruleStdNotIp.macDa.arEther[i] = 0xFF;
        mask.ruleStdNotIp.macSa.arEther[i] = 0xFF;
        pattern.ruleStdNotIp.macDa.arEther[i] =
            packet1TgfL2Part.daMac[i];
        pattern.ruleStdNotIp.macSa.arEther[i] =
            packet1TgfL2Part.saMac[i];
    }
    mask.ruleStdNotIp.etherType    = 0xFFFF;
    pattern.ruleStdNotIp.etherType = /*prvTgfPacket1EtherTypePart.etherType*/0x3333;

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;

    action.sourcePort.assignSourcePortEnable = GT_TRUE;
    action.sourcePort.sourcePortValue = eSrcPort;


    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern DA=00:8A:5C:71:90:36, SA=00:FE:98:76:43:55 */
    /* AUTODOC:   pattern pclId=1, EtherType=0x3333 */
    /* AUTODOC: add PCL action: */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclTrunkEportRedirectIpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclTrunkEportRedirectIpclRestore
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
* @internal prvTgfPclTrunkEportRedirectCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclTrunkEportRedirectCountersEthReset
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
* @internal prvTgfPclTrunkEportRedirectTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclTrunkEportRedirectTrafficGenerate
(
    IN TGF_PACKET_STC   *prvTgfPacketInfoPtr
)
{
    GT_STATUS                       rc         = GT_OK;

    prvTgfPclTrunkEportRedirectCountersEthReset();

    tgfTrafficTracePacketByteSet(GT_TRUE);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPacketInfoPtr, 1 /*burstCount*/, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:8A:5C:71:90:36, SA=00:FE:98:76:43:55 */
    /* AUTODOC:   VID=2, EtherType=0x3333 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfPclTrunkEportRedirectTrafficCheck function
* @endinternal
*
* @brief   Checks FDB entry values after learning
*
* @param[in] trunkId                  - expected value of  after reassignment
* @param[in] srcPort                  - expected value of  after reassignment
*                                       None
*/
GT_VOID prvTgfPclTrunkEportRedirectTrafficCheck
(
    GT_U32 trunkId,
    GT_U32 srcPort
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC     macEntryKey;
    GT_STATUS       rc;
    /*
        AUTODOC: Check FDB entries phase - read dynamic FDB entries and verify
                 MACs, VLANs, ports
    */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_INIT_VID0_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther,
                 packet1TgfL2Part.saMac, 6);
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* AUTODOC: get FDB entry */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d",
                                 prvTgfDevNum);

    /* compare entryType */
    rc = (macEntry.key.entryType == PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E) ? GT_OK :
                                                                        GT_FAIL;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "vlanId: %d",
                                 macEntry.key.entryType);

    /* AUTODOC: compare MAC address to packet's MAC SA */
    rc = cpssOsMemCmp(macEntry.key.key.macVlan.macAddr.arEther,
              macEntryKey.key.macVlan.macAddr.arEther,
              sizeof(TGF_MAC_ADDR)) == 0 ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "MAC address[0]: %d",
                     macEntry.key.key.macVlan.macAddr.arEther[0]);

    /* AUTODOC: verify vlanId */
    rc = (macEntry.key.key.macVlan.vlanId == macEntryKey.key.macVlan.vlanId) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "vlanId: %d",
                                 macEntryKey.key.macVlan.vlanId);

    /* compare dstInterface.type */
    rc = ((macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) ||
          (macEntry.dstInterface.type == CPSS_INTERFACE_TRUNK_E)) ? GT_OK :
                                                                    GT_FAIL;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.type: %d",
                                 macEntry.dstInterface.type);

    /* AUTODOC: verify dstInterface */
    if (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E)
    {
        rc = (macEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) ? GT_OK :
                                                                      GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "dstInterface.devPort.hwDevNum: %d",
                                     macEntry.dstInterface.devPort.hwDevNum);

        rc = (macEntry.dstInterface.devPort.portNum == srcPort) ? GT_OK :
                                                                  GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "dstInterface.devPort.portNum: %d",
                                     macEntry.dstInterface.devPort.portNum);
    }
    else if (macEntry.dstInterface.type == CPSS_INTERFACE_TRUNK_E)
    {
    /* AUTODOC: verify Trunk Id */
        rc = (macEntry.dstInterface.trunkId == trunkId) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "dstInterface.trunkId: %d",
                                     macEntry.dstInterface.type);
    }
}

/**
* @internal prvTgfPclTrunkEportRedirectCustomerTest function
* @endinternal
*
* @brief   Trunk and Eport reassignment in FDB learning test
*/
GT_VOID prvTgfPclTrunkEportRedirectCustomerTest
(
    GT_VOID
)
{

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC( "*** TrunkEportRedirect FDB learning test  *** \n");

    prvTgfPclTrunkEportRedirectConfigurationSet();

    prvTgfPclTrunkEportRedirectConfigurationIpclAdd( PRV_TGF_SEND_PORT_IDX_CNS,
                                                   PRV_TGF_TRG_EPORT_VALUE_MAC);

    prvTgfPacket1EtherTypePart.etherType = 0x3333;
    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclTrunkEportRedirectTrafficGenerate(&packet1TgfInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC: check FDB learning */
    prvTgfPclTrunkEportRedirectTrafficCheck( 0, PRV_TGF_TRG_EPORT_VALUE_MAC);

    /* AUTODOC: Invalidate traffic: */

    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: Send not matched traffic: */
    prvTgfPclTrunkEportRedirectTrafficGenerate(&packet2TgfInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC: check FDB learning */
    prvTgfPclTrunkEportRedirectTrafficCheck( PRV_TGF_TRUNK_ID_CNS,
                                             PRV_TGF_SEND_PORT_IDX_CNS);

    prvTgfPclTrunkEportRedirectIpclRestore();

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclTrunkEportRedirectConfigurationRestore();

    PRV_UTF_LOG0_MAC( "*** End of TrunkEportRedirect FDB learning test *** \n");
}

