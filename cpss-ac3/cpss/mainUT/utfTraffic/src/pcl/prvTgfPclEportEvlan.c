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
* @file prvTgfPclEportEvlan.c
*
* @brief ePort and eVlan specific PCL features testing
*
* @version   10
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
#include <common/tgfTunnelGen.h>
#include <pcl/prvTgfPclMiscellanous.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

/* initial VIDs in two tags */
#define PRV_TGF_INIT_VID0_CNS  2

/* IPCL modified VID */
#define PRV_TGF_NEW_IPCL_VID0_CNS  3

/* EPCL modified VID */
#define PRV_TGF_NEW_EPCL_VID0_CNS  4

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_CNS  1
#define PRV_TGF_EPCL_RULE_INDEX_CNS  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(3)

/* PCL rule indexes */
#define PRV_TGF_TTI_RULE_INDEX_CNS  8

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_CNS  1
#define PRV_TGF_EPCL_PCL_ID_CNS  2

/* ePort values */
#define PRV_TGF_SRC_EPORT_CNS  ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x17FF)
#define PRV_TGF_TRG_EPORT_CNS  ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x17FE)




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
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC packet1TgfInfo = {
    (TGF_L2_HEADER_SIZE_CNS
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

/* stored target ePort Physical Info */
CPSS_INTERFACE_INFO_STC          saveTargetPhyInfo;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclEportEvlanTtiSrcEportSet function
* @endinternal
*
* @brief   Configure Tunnel rule and action
*
* @param[in] ttiRuleIndex             - tti Rule Index
* @param[in] srcPhyPort               - source physical port
* @param[in] sourceEPort              - source ePort
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPclEportEvlanTtiSrcEportSet
(
    IN GT_U32              ttiRuleIndex,
    IN GT_U32              srcPhyPort,
    IN GT_U32              sourceEPort
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC      ttiAction;
    PRV_TGF_TTI_ACTION_2_STC    ttiAction2;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;

    /* AUTODOC: enable TTI lookup for Ethernet key on port 1 */
    rc = prvTgfTtiPortLookupEnableSet(
        srcPhyPort, PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* set TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    ttiMask.eth.common.srcPortTrunk    = 0xFF;
    ttiPattern.eth.common.srcPortTrunk = srcPhyPort;

    /* set TTI mask - dummy, will be overridden */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    ttiAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;

    /* AUTODOC: add TTI Rule 8 with: */
    /* AUTODOC:   key=TTI_KEY_ETH, srcPortTrunk=1 */
    /* AUTODOC:   tag1VlanCmd=MODIFY_UNTAGGED */
    rc = prvTgfTtiRuleSet(ttiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleSet");

    cpssOsMemSet(&ttiAction2, 0, sizeof(ttiAction2));
    ttiAction2.tunnelTerminate             = GT_FALSE;
    ttiAction2.command                     = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2.redirectCommand             = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction2.tag0VlanCmd                 = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction2.sourceEPortAssignmentEnable = GT_TRUE;
    ttiAction2.sourceEPort                 = sourceEPort;

    /* AUTODOC: add TTI Rule action for Rule 8 with: */
    /* AUTODOC:   TT=GT_FALSE, cmd=FORWARD */
    /* AUTODOC:   tag1VlanCmd=MODIFY_UNTAGGED */
    /* AUTODOC:   sourceEPort=0x7FF */
    rc = prvTgfTtiRuleAction2Set(ttiRuleIndex, &ttiAction2);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleAction2Set");
    return GT_OK;
}

/**
* @internal prvTgfPclEportEvlanTtiSrcEportReset function
* @endinternal
*
* @brief   Configure Tunnel rule and action
*
* @param[in] ttiRuleIndex             - tti Rule Index
* @param[in] srcPhyPort               - source physical port
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPclEportEvlanTtiSrcEportReset
(
    IN GT_U32 ttiRuleIndex,
    IN GT_U32 srcPhyPort
)
{
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: invalidate TTI Rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(ttiRuleIndex, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: disable TTI lookup on port 1 */
    rc = prvTgfTtiPortLookupEnableSet(
        srcPhyPort, PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclEportEvlanMapEport2PhySet function
* @endinternal
*
* @brief   Maps ePort to physical interface
*
* @param[in] devNum                   - devNum
* @param[in] portNum                  -  ePort
* @param[in] phyInfoPtr               -  (pointer to) inteface to map
*
* @param[out] savePhyInfoPtr           - (pointer to) inteface to save previous mapping
*                                      may be NULL to skip saving
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPclEportEvlanMapEport2PhySet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_INTERFACE_INFO_STC          *phyInfoPtr,
    OUT CPSS_INTERFACE_INFO_STC          *savePhyInfoPtr
)
{
    GT_STATUS rc;

    if (savePhyInfoPtr != NULL)
    {
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(
            devNum, portNum, savePhyInfoPtr);
        PRV_UTF_VERIFY_GT_OK(
            rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    }

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(
        devNum, portNum, phyInfoPtr);
    PRV_UTF_VERIFY_GT_OK(
        rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclEportEvlanCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclEportEvlanCountersEthReset
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
* @internal prvTgfPclEportEvlanConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclEportEvlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    CPSS_INTERFACE_INFO_STC          targetPhyInfo;

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
        PRV_TGF_NEW_IPCL_VID0_CNS,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 4 with all tagged OUTER_TAG0_INNER_TAG1 ports */
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

    /* AUTODOC: set PVID 2 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_INIT_VID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    cpssOsMemSet(&targetPhyInfo, 0, sizeof(targetPhyInfo));
    targetPhyInfo.type = CPSS_INTERFACE_PORT_E;
    targetPhyInfo.devPort.hwDevNum =
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    targetPhyInfo.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* AUTODOC: map ePort 0x7FE to physical port 2 */
    rc = prvTgfPclEportEvlanMapEport2PhySet(
        prvTgfDevNum,
        PRV_TGF_TRG_EPORT_CNS,
        &targetPhyInfo,
        &saveTargetPhyInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclEportEvlanMapEport2PhySet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 2, ePort 0x7FE */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        packet1TgfL2Part.daMac, PRV_TGF_INIT_VID0_CNS,
        prvTgfDevNum,
        PRV_TGF_TRG_EPORT_CNS,
        GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 3, ePort 0x7FE */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        packet1TgfL2Part.daMac, PRV_TGF_NEW_IPCL_VID0_CNS,
        prvTgfDevNum,
        PRV_TGF_TRG_EPORT_CNS,
        GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    rc = prvTgfPclEportEvlanTtiSrcEportSet(
        PRV_TGF_TTI_RULE_INDEX_CNS,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_SRC_EPORT_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclEportEvlanTtiSrcEportSet: %d", prvTgfDevNum);

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxModeSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfPclEportEvlanConfigurationIpclAdd function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration
*/
static GT_VOID prvTgfPclEportEvlanConfigurationIpclAdd
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

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_CNS;
/*xxx    lookupCfg.enableKeyExtension     = GT_TRUE;*/
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

    /* AUTODOC: enables ingress policy for ePort 0x7FF */
    rc = prvTgfPclEPortIngressPolicyEnable(
        prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
        prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS);

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


/* xxx
   mask.ruleStdNotIp.extension.srcPort    = 0xFFFFF;
    pattern.ruleStdNotIp.extension.srcPort = PRV_TGF_SRC_EPORT_CNS;
    */


    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_NEW_IPCL_VID0_CNS;

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=1, srcPort=0x7FF */
    /* AUTODOC:   enable modify VLAN, VID=3 */
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex,
        0 CPSS_TBD_BOOKMARK,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEportEvlanConfigurationEpclAdd function
* @endinternal
*
* @brief   Adds Egress Pcl configuration
*/
static GT_VOID prvTgfPclEportEvlanConfigurationEpclAdd
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

    /* AUTODOC: enable egress policy for (physical port that hold) ePort 0x7FE and non-tunneled packets */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet: %d, %d", prvTgfDevNum,
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E);

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

/* xxx CPSS_TBD_BOOKMARK    mask.ruleEgrStdNotIp.extension.srcPort    = 0xFFFFF;
    pattern.ruleEgrStdNotIp.extension.srcPort = PRV_TGF_SRC_EPORT_CNS;

    mask.ruleEgrStdNotIp.extension.trgPort    = 0xFFFFF;
    pattern.ruleEgrStdNotIp.extension.trgPort = PRV_TGF_TRG_EPORT_CNS;*/

    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId0Cmd =
        PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
    action.vlan.vlanId  = PRV_TGF_NEW_EPCL_VID0_CNS;

    /* AUTODOC: set PCL rule 3 with: */
    /* AUTODOC:   format EGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=2, srcPort=0x7FF, trgPort=0x7FE */
    /* AUTODOC:   enable modify OUTER_TAG, VID=4 */
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex,
        0 CPSS_TBD_BOOKMARK,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEportEvlanConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEportEvlanConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;

    rc = prvTgfPclEportEvlanTtiSrcEportReset(
        PRV_TGF_TTI_RULE_INDEX_CNS,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclEportEvlanTtiSrcEportReset: %d", prvTgfDevNum);

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

    /* AUTODOC: restore ePort mapping to physical port */
    rc = prvTgfPclEportEvlanMapEport2PhySet(
        prvTgfDevNum,
        PRV_TGF_TRG_EPORT_CNS,
        &saveTargetPhyInfo,
        NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclEportEvlanMapEport2PhySet: %d", prvTgfDevNum);

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

}

/**
* @internal prvTgfPclEportEvlanIpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEportEvlanIpclRestore
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

    /* AUTODOC: disable ingress policy on ePort 0x7FF */
    rc = prvTgfPclEPortIngressPolicyEnable(
        prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
        prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS);

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_EXT_E,
        PRV_TGF_IPCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfPclEportEvlanEpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEportEvlanEpclRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;

    /* AUTODOC: disable egress policy for ePort 0x7FE and non-tunneled packets */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet: %d, %d", prvTgfDevNum,
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E);

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

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_EXT_E,
        PRV_TGF_EPCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfPclEportEvlanEpclRestore1 function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEportEvlanEpclRestore1
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;

    /* AUTODOC: Restore default access mode on ePort */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_INDEX_E;
    interfaceInfo.index              = PRV_TGF_TRG_EPORT_CNS & 0xFFF;

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEportEvlanEpclRestore();
}

/**
* @internal prvTgfPclEportEvlanConfigurationIpclAdd1 function
* @endinternal
*
* @brief   Adds Ingress Pcl configuration for ePort access mode
*/
static GT_VOID prvTgfPclEportEvlanConfigurationIpclAdd1
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

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;


    /* AUTODOC:   init PCL Engine for send port 1: */
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

    /* AUTODOC: Enables ingress policy on ePort */
    rc = prvTgfPclEPortIngressPolicyEnable(
            prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclEPortIngressPolicyEnable: %d, %d, %d",
        prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS, GT_TRUE);

    /* AUTODOC:  Enable ePort access mode and set PCL configuration entry */
    rc = prvTgfPclEportAccessModeInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_SRC_EPORT_CNS & 0xFFF, /* ePort (12 LSbits) */
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclEportAccessModeInit: %d, %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SRC_EPORT_CNS & 0xFFF);

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

    action.egressPolicy = GT_FALSE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId     = PRV_TGF_NEW_IPCL_VID0_CNS;

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=1, srcPort=0x7FF */
    /* AUTODOC:   enable modify VLAN, VID=3 */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEportEvlanConfigurationEpclAdd1 function
* @endinternal
*
* @brief   Adds Egress Pcl configuration
*/
static GT_VOID prvTgfPclEportEvlanConfigurationEpclAdd1
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

    /* AUTODOC: enable egress policy for (physical port that hold) ePort 0x7FE and non-tunneled packets */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet: %d, %d", prvTgfDevNum,
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E);

    /* AUTODOC:  Enable ePort access mode and set PCL configuration entry */
    rc = prvTgfPclEportAccessModeInit(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_TRG_EPORT_CNS & 0xFFF, /* ePort (12 LSbits) */
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclEportAccessModeInit: %d, %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SRC_EPORT_CNS);

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

    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId0Cmd =
        PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
    action.vlan.vlanId  = PRV_TGF_NEW_EPCL_VID0_CNS;

    /* AUTODOC: set PCL rule 3 with: */
    /* AUTODOC:   format EGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=2, srcPort=0x7FF, trgPort=0x7FE */
    /* AUTODOC:   enable modify OUTER_TAG, VID=4 */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfPclEportEvlanIpclRestore1 function
* @endinternal
*
* @brief   Restore test configuration for ePort acces mode
*/
static GT_VOID prvTgfPclEportEvlanIpclRestore1
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

    /* AUTODOC: Restore default access mode on port */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    interfaceInfo.type              = CPSS_INTERFACE_INDEX_E;
    interfaceInfo.index             = PRV_TGF_SRC_EPORT_CNS & 0xFFF; /* ePort (12 LSbits) */

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

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEportEvlanIpclRestore();
}
/**
* @internal prvTgfPclEportEvlanTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfPclEportEvlanTrafficGenerate
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

    prvTgfPclEportEvlanCountersEthReset();

    tgfTrafficTracePacketByteSet(GT_TRUE);

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:55 */
    /* AUTODOC:   VID=2, EtherType=0x3333 */
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
* @internal prvTgfPclEportEvlanTrafficCheck function
* @endinternal
*
* @brief   Checks traffic egress VID0 in the Tag
*/
GT_VOID prvTgfPclEportEvlanTrafficCheck
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
* @internal prvTgfPclEportEvlanIpclTrafficTest function
* @endinternal
*
* @brief   Ingress PCL extension srcPort (ePort) field test
*/
GT_VOID prvTgfPclEportEvlanIpclTrafficTest
(
    GT_VOID
)
{
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfPclEportEvlanConfigurationSet();

    PRV_UTF_LOG0_MAC(
                    "*** EportEvlan IPCL test - no fields *** \n");

    prvTgfPclEportEvlanConfigurationIpclAdd();

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEportEvlanTrafficGenerate();

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   double tagged packet VID=3 received on port 2 */
    prvTgfPclEportEvlanTrafficCheck(PRV_TGF_NEW_IPCL_VID0_CNS);

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEportEvlanIpclRestore();

    PRV_UTF_LOG0_MAC(
                    "*** End of EportEvlan IPCL test - no fields *** \n");

    PRV_UTF_LOG0_MAC(
                    "*** EportEvlan IPCL test (ePort access mode) - no fields *** \n");

    prvTgfPclEportEvlanConfigurationIpclAdd1();

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEportEvlanTrafficGenerate();

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   double tagged packet VID=3 received on port 2 */
    prvTgfPclEportEvlanTrafficCheck(PRV_TGF_NEW_IPCL_VID0_CNS);

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEportEvlanIpclRestore1();

    PRV_UTF_LOG0_MAC(
                    "*** End of EportEvlan IPCL test (ePort access mode) - no fields *** \n");


    prvTgfPclEportEvlanConfigurationRestore();

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfPclEportEvlanEpclTrafficTest function
* @endinternal
*
* @brief   Egress PCL extension srcPort and trgPort(ePort) fields test
*/
GT_VOID prvTgfPclEportEvlanEpclTrafficTest
(
    GT_VOID
)
{
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfPclEportEvlanConfigurationSet();

    PRV_UTF_LOG0_MAC(
                    "*** EportEvlan EPCL test - no fields *** \n");

    prvTgfPclEportEvlanConfigurationEpclAdd();

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEportEvlanTrafficGenerate();

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   double tagged packet VID=4 received on port 2 */
    prvTgfPclEportEvlanTrafficCheck(PRV_TGF_NEW_EPCL_VID0_CNS);

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEportEvlanEpclRestore();

    PRV_UTF_LOG0_MAC(
                    "*** End of EportEvlan EPCL test - no fields *** \n");

    PRV_UTF_LOG0_MAC(
        "*** EportEvlan EPCL test (ePort access mode) - no fields *** \n");

    prvTgfPclEportEvlanConfigurationEpclAdd1();

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEportEvlanTrafficGenerate();

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   double tagged packet VID=4 received on port 2 */
    prvTgfPclEportEvlanTrafficCheck(PRV_TGF_NEW_EPCL_VID0_CNS);

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclEportEvlanEpclRestore1();

    PRV_UTF_LOG0_MAC(
        "*** End of EportEvlan EPCL test (ePort access mode) - no fields *** \n");

    prvTgfPclEportEvlanConfigurationRestore();

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


