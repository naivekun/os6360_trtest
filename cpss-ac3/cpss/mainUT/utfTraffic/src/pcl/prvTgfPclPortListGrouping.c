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
* @file prvTgfPclPortListGrouping.c
*
* @brief Test for Port List and group mode.
*
* @version   1
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

#include <common/tgfTunnelGen.h>
#include <pcl/prvTgfPclPortListGrouping.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* VLAN Id */
#define PRV_TGF_VLANID_CNS          15

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* IPCL rule & action index */
#define PRV_TGF_IPCL_IDX_CNS   515

/* Port List mapping data */
#define PRV_TGF_PORT_LIST_PORT_GROUP_CNS    0xA
#define PRV_TGF_PORT_LIST_PORT_OFFSET_CNS   17

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/******************************* Test packet **********************************/

/* L2 part */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},               /* daMac */
    {0x00, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}                /* saMac */
};
  
/* VLAN_TAG part */
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

/*************************** Restore config ***********************************/
GT_BOOL portListEnableForRestore;
GT_BOOL portListGroupingEnableForRestore;
GT_BOOL portListPortMappingEnable;
GT_U32 portListPortMappingGroup;
GT_U32 portListPortMappingOffset;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclPortListGroupingTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry and add ports.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPclPortListGroupingTestVlanInit
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

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;

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
* @internal prvTgfPclPortListGroupingTestIpclRulesAndActionsSet function
* @endinternal
*
* @brief   Set IPCL rules and actions
*
* @param[in] portListBmpPattern       - pattern of port list bitmap to be used in PCL rule.
* @param[in] portListBmpMask          - mask of port list bitmap to be used in PCL rule.
*                                       None
*/
static GT_VOID prvTgfPclPortListGroupingTestIpclRulesAndActionsSet
(
    GT_U32  portListBmpPattern,
    GT_U32  portListBmpMask
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: action - hard drop */
    action.pktCmd = PRV_TGF_PACKET_CMD_DROP_HARD_E;

    /* AUTODOC: Mask for Port List */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.common.portListBmp.ports[0] = portListBmpMask;

    /* AUTODOC: Pattern for Port List*/
    pattern.ruleStdNotIp.common.portListBmp.ports[0] = portListBmpPattern;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            PRV_TGF_IPCL_IDX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d,"
                                 "pattern 0x%x, mask 0x%x",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                 PRV_TGF_IPCL_IDX_CNS,
                                 portListBmpPattern,
                                 portListBmpMask);  
}


/**
* @internal prvTgfPclPortListGroupingTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfPclPortListGroupingTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN with all ports as members. */
    prvTgfPclPortListGroupingTestVlanInit(PRV_TGF_VLANID_CNS);

    /* AUTODOC: Save Port List enabling status for restore */
    rc = prvTgfPclLookupCfgPortListEnableGet(prvTgfDevNum,
                                             CPSS_PCL_DIRECTION_INGRESS_E,
                                             CPSS_PCL_LOOKUP_0_E,
                                             0,
                                             &portListEnableForRestore);
    UTF_VERIFY_EQUAL4_STRING_MAC(
            GT_OK, rc, "prvTgfPclLookupCfgPortListEnableGet: %d, %d, %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            0,
            portListEnableForRestore);

    /* AUTODOC: Save Port List Group mode enabling status for restore */
    rc = prvTgfPclPortListGroupingEnableGet(prvTgfDevNum,
                                            CPSS_PCL_DIRECTION_INGRESS_E,
                                            &portListGroupingEnableForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortListGroupingEnableGet: %d, %d, %d", 
            prvTgfDevNum,
            CPSS_PCL_DIRECTION_INGRESS_E,
            portListGroupingEnableForRestore);

    /* AUTODOC: Save Port Mapping data for restore */
    rc = prvTgfPclPortListPortMappingGet(prvTgfDevNum,
                                         CPSS_PCL_DIRECTION_INGRESS_E,
                                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         &portListPortMappingEnable,
                                         &portListPortMappingGroup,
                                         &portListPortMappingOffset);
    UTF_VERIFY_EQUAL5_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortListPortMappingGet: %d, %d, %d, %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            portListPortMappingEnable,
            portListPortMappingGroup,
            portListPortMappingOffset);

    /* AUTODOC: Init IPCL Engine */
    rc = prvTgfPclDefPortInit(
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
         CPSS_PCL_DIRECTION_INGRESS_E,
         CPSS_PCL_LOOKUP_0_E,
         PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
         PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
         PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", 
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfPclPortListGroupingTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPclPortListGroupingTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 
                                     PRV_TGF_IPCL_IDX_CNS, 
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E, 
                                 PRV_TGF_IPCL_IDX_CNS, 
                                 GT_FALSE);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy on port */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: Restore Port List enabling status */
    rc = prvTgfPclLookupCfgPortListEnableSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                             CPSS_PCL_LOOKUP_0_E,
                                             0,
                                             portListEnableForRestore);
    UTF_VERIFY_EQUAL4_STRING_MAC(
            GT_OK, rc, "prvTgfPclLookupCfgPortListEnableSet: %d, %d, %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            0,
            portListEnableForRestore);

    /* AUTODOC: Restore Port List Group mode enabling status */
    rc = prvTgfPclPortListGroupingEnableSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                            portListGroupingEnableForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortListGroupingEnableSet: %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            portListGroupingEnableForRestore);

    /* AUTODOC: Restore Port Mapping data */
    rc = prvTgfPclPortListPortMappingSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         portListPortMappingEnable,
                                         portListPortMappingGroup,
                                         portListPortMappingOffset);
    UTF_VERIFY_EQUAL5_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortListPortMappingSet: %d, %d, %d, %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            portListPortMappingEnable,
            portListPortMappingGroup,
            portListPortMappingOffset);
    
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
}


/**
* @internal prvTgfPclPortListGroupingTestCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] sendPortIndex            -  sending port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPclPortListGroupingTestCheckCounters
(
    IN GT_U32           sendPortIndex,
    IN GT_U32           expectedValue,
    IN GT_U32           callIdentifier
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if (sendPortIndex == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL4_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "Rx another goodPktsRcv counter %d, than expected %d, on port %d, %d",
                                         portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(expectedValue, portCntrs.goodPktsSent.l[0],
                                         "Tx another goodPktsSent counter %d, than expected %d, on port %d, %d",
                                         portCntrs.goodPktsSent.l[0], expectedValue,
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
    }
}


/**
* @internal prvTgfPclPortListGroupingTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] portIndex                - port indexing place in test array ports.
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclPortListGroupingTestPacketSend
(
    IN GT_U32           portIndex,
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      portIter;
   
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

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[portIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIndex]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPclPortListGroupingTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPclPortListGroupingTestGenerateTraffic
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    GT_U32                          partsCount;
    GT_U32                          packetSize;
    TGF_PACKET_STC                  packetInfo;

    /* AUTODOC: Test traffic start */
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

    /* AUTODOC: Port List enabling */
    rc = prvTgfPclLookupCfgPortListEnableSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                             CPSS_PCL_LOOKUP_0_E,
                                             0,
                                             GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(
            GT_OK, rc, "prvTgfPclLookupCfgPortListEnableSet: %d, %d, %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            0,
            portListEnableForRestore);

    /* AUTODOC: Port Mapping data setting - group 0xA, offset 17 */
    rc = prvTgfPclPortListPortMappingSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         GT_TRUE,
                                         PRV_TGF_PORT_LIST_PORT_GROUP_CNS,
                                         PRV_TGF_PORT_LIST_PORT_OFFSET_CNS);
    UTF_VERIFY_EQUAL5_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortListPortMappingSet: %d, %d, %d, %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_TRUE,
            PRV_TGF_PORT_LIST_PORT_GROUP_CNS,
            PRV_TGF_PORT_LIST_PORT_OFFSET_CNS);

    /* AUTODOC: Rule set with port list pattern (0x0) and mask (0x0)*/
    prvTgfPclPortListGroupingTestIpclRulesAndActionsSet(0x0, 0x0);

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                               0, 0);

    /* AUTODOC: Rule set with port list pattern (0x0) and mask (0xFFFFFFF)*/
    prvTgfPclPortListGroupingTestIpclRulesAndActionsSet(0x0, 0xFFFFFFF);

    /* AUTODOC: Send packet - expect NO match -> FLOOD */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet flooding in vlan */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             prvTgfBurstCount, 1);

    /* AUTODOC: Rule set with port list pattern (0x0020000) and mask (0xFFFFFFF)*/
    prvTgfPclPortListGroupingTestIpclRulesAndActionsSet(
                        (1<<PRV_TGF_PORT_LIST_PORT_OFFSET_CNS), 
                        0xFFFFFFF);

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                               0, 2);

    /* AUTODOC: Port List Group mode enabling */
    rc = prvTgfPclPortListGroupingEnableSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                            GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortListGroupingEnableSet: %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            GT_TRUE);

    /* AUTODOC: Send packet - expect NO match -> FLOOD */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet flooding in vlan */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             prvTgfBurstCount, 3);

    /* AUTODOC: Rule set with port list pattern (0xA020000) and mask (0xFFFFFFF)*/
    prvTgfPclPortListGroupingTestIpclRulesAndActionsSet(
        (PRV_TGF_PORT_LIST_PORT_GROUP_CNS<<24) + (1<<PRV_TGF_PORT_LIST_PORT_OFFSET_CNS), 
        0xFFFFFFF);

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                               0, 4);

    /* AUTODOC: Port Mapping data "disabling" - group 0xA, offset 17 */
    rc = prvTgfPclPortListPortMappingSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         GT_FALSE,
                                         PRV_TGF_PORT_LIST_PORT_GROUP_CNS,
                                         PRV_TGF_PORT_LIST_PORT_OFFSET_CNS);
    UTF_VERIFY_EQUAL5_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortListPortMappingSet: %d, %d, %d, %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_FALSE,
            PRV_TGF_PORT_LIST_PORT_GROUP_CNS,
            PRV_TGF_PORT_LIST_PORT_OFFSET_CNS);

    /* AUTODOC: Send packet - expect NO match -> FLOOD */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet flooding in vlan */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             prvTgfBurstCount, 5);

    /* AUTODOC: Rule set with port list pattern (0x0020000) and mask (0xFFFFFFF)*/
    prvTgfPclPortListGroupingTestIpclRulesAndActionsSet(
                            (1<<PRV_TGF_PORT_LIST_PORT_OFFSET_CNS), 
                            0xFFFFFFF);

    /* AUTODOC: Send packet - expect NO match -> FLOOD */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet flooding in vlan */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             prvTgfBurstCount, 6);

    /* AUTODOC: Rule set with port list pattern (0xA000000) and mask (0xFFFFFFF)*/
    prvTgfPclPortListGroupingTestIpclRulesAndActionsSet(
                            (PRV_TGF_PORT_LIST_PORT_GROUP_CNS<<24), 
                            0xFFFFFFF);

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                               0, 7);

    /* AUTODOC: Port List Group mode disabling */
    rc = prvTgfPclPortListGroupingEnableSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                            GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortListGroupingEnableSet: %d, %d", 
            CPSS_PCL_DIRECTION_INGRESS_E,
            GT_FALSE);

    /* AUTODOC: Send packet - expect NO match -> FLOOD */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet flooding in vlan */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             prvTgfBurstCount, 8);

    /* AUTODOC: Rule set with port list pattern (0x0) and mask (0xFFFFFFF)*/
    prvTgfPclPortListGroupingTestIpclRulesAndActionsSet(0x0, 0xFFFFFFF);

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclPortListGroupingTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                            &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclPortListGroupingTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                               0, 9);

    /* AUTODOC: Test traffic end */
}

/**
* @internal prvTgfPclPortListGroupingTest function
* @endinternal
*
* @brief   Port list and group mode test
*/
GT_VOID prvTgfPclPortListGroupingTest
(
    GT_VOID
)
{
    prvTgfPclPortListGroupingTestConfigurationSet();

    prvTgfPclPortListGroupingTestGenerateTraffic();

    prvTgfPclPortListGroupingTestConfigurationRestore();
}


