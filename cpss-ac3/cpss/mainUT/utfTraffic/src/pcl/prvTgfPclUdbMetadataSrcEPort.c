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
* @file prvTgfPclUdbMetadataSrcEPort.c
*
* @brief User Defined Bytes test for matching source ePort using metadata ancor
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

#include <pcl/prvTgfPclUdbMetadataSrcEPort.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   0

/* VLAN 0 Id */
#define PRV_TGF_VLANID_0_CNS          5

/* VLAN 1 Id */
#define PRV_TGF_VLANID_1_CNS       4094

/* main sending port index */
#define MAIN_SEND_PORT_IDX             1

/* secondary sending port index */
#define SECONDARY_SEND_PORT_IDX             2

/* assign source ePort for main sending port */
#define ASSIGN_SOURCE_EPORT (IS_AC3X_DEV_MAC(prvTgfDevNum)?1023:0x1555)

/* IPCL rule & action index */
#define IPCL_MATCH_INDEX        10

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/************************* General packet's parts *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0xAA}                 /* saMac */
};

/* VLAN_TAG 0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_0_CNS                            /* pri, cfi, VlanId */
};

/* VLAN_TAG 1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_1_CNS                            /* pri, cfi, VlanId */
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
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + 2*TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_CRC_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************/

/* default ePort value to restore */
GT_PORT_NUM defaultAssignEPortSave;


/**
* @internal prvTgfPclUdbMetadataSrcEportTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPclUdbMetadataSrcEportTestVlanInit
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
* @internal prvTgfPclUdbMetadataSrcEportTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfPclUdbMetadataSrcEportTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      portIter;
    GT_U32      defaultEPortsNum;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 0 with all ports as members. */
    prvTgfPclUdbMetadataSrcEportTestVlanInit(PRV_TGF_VLANID_0_CNS);

    /* AUTODOC: save default, default ePort value */
    rc = prvTgfCfgPortDefaultSourceEportNumberGet(
            prvTgfDevNum,
            prvTgfPortsArray[MAIN_SEND_PORT_IDX],
            &defaultAssignEPortSave);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCfgPortDefaultSourceEportNumberGet: %d, %d", 
            prvTgfDevNum, prvTgfPortsArray[MAIN_SEND_PORT_IDX]);

    /* AUTODOC: set new assigned default ePort value */
    rc = prvTgfCfgPortDefaultSourceEportNumberSet(
            prvTgfDevNum,
            prvTgfPortsArray[MAIN_SEND_PORT_IDX],
            ASSIGN_SOURCE_EPORT);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfCfgPortDefaultSourceEportNumberSet: %d, %d, %d", 
            prvTgfDevNum, prvTgfPortsArray[MAIN_SEND_PORT_IDX],
            ASSIGN_SOURCE_EPORT);

    /* AUTODOC: Init IPCL Engine for all vlan ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[portIter],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", 
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Get deafult eports range */
    rc = prvUtfMaxDefaultEPortNumberGet(&defaultEPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvUtfMaxDefaultEPortNumberGet: %d", defaultEPortsNum);

    /* AUTODOC: Set IPCL configuration table access mode for deafult eport (based on assigned ePort) */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(((defaultEPortsNum-1) & ASSIGN_SOURCE_EPORT),
                                                CPSS_PCL_DIRECTION_INGRESS_E,
                                                CPSS_PCL_LOOKUP_0_E,
                                                0,
                                                PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                 ASSIGN_SOURCE_EPORT,
                                 CPSS_PCL_DIRECTION_INGRESS_E,
                                 CPSS_PCL_LOOKUP_0_E,
                                 0,
                                 PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

    /* AUTODOC: Enables ingress policy for assign source ePort */
    rc = prvTgfPclEPortIngressPolicyEnable(prvTgfDevNum, ASSIGN_SOURCE_EPORT, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d, %d",
                                 prvTgfDevNum, ASSIGN_SOURCE_EPORT, GT_TRUE);

    /* AUTODOC: Metadata UDBs configuration for source ePort in ethernet*/
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E, 
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        39 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_METADATA_E, 
        5/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        39, PRV_TGF_PCL_OFFSET_METADATA_E, 5);

    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E, 
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        40 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_METADATA_E, 
        6/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        40, PRV_TGF_PCL_OFFSET_METADATA_E, 6);
}

/**
* @internal prvTgfPclUdbMetadataSrcEportTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPclUdbMetadataSrcEportTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      portIter;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E, 
                                     IPCL_MATCH_INDEX, 
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_EXT_E, 
                                 IPCL_MATCH_INDEX, 
                                 GT_FALSE);

    

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for all vlan ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[portIter], GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                     prvTgfPortsArray[portIter], GT_FALSE);
    }

    /* AUTODOC: Disables ingress policy for assign source ePort */
    rc = prvTgfPclEPortIngressPolicyEnable(prvTgfDevNum, ASSIGN_SOURCE_EPORT, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d, %d",
                                 prvTgfDevNum, ASSIGN_SOURCE_EPORT, GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);
    

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_0_CNS);

    /* AUTODOC: restore default ePort value */
    rc = prvTgfCfgPortDefaultSourceEportNumberSet(
            prvTgfDevNum,
            prvTgfPortsArray[MAIN_SEND_PORT_IDX],
            defaultAssignEPortSave);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfCfgPortDefaultSourceEportNumberSet: %d, %d, %d", 
            prvTgfDevNum, prvTgfPortsArray[MAIN_SEND_PORT_IDX],
            defaultAssignEPortSave);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclUdbMetadataSrcEportTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] portIndex                - port indexing place in test array ports.
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclUdbMetadataSrcEportTestPacketSend
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
* @internal prvTgfPclUdbMetadataSrcEportTestCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] sendPortIndex            -  sending port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPclUdbMetadataSrcEportTestCheckCounters
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
* @internal prvTgfTrafficPclUdbMetadataSrcEportTestRulesAndActionsSet function
* @endinternal
*
* @brief   Set IPCL action and rule
*
* @param[in] sourceEPort              - source ePort value to be matched in the UDBs
*                                       None
*/
static GT_VOID prvTgfTrafficPclUdbMetadataSrcEportTestRulesAndActionsSet
(
    GT_U32 sourceEPort 
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /* AUTODOC: mask for source ePort */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleExtNotIpv6.udb[39] = 0xFF;
    mask.ruleExtNotIpv6.udb[40] = 0x1F;

    /* AUTODOC: pattern for source ePort */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleExtNotIpv6.udb[39] = ((GT_U8)sourceEPort & 0xFF);
    pattern.ruleExtNotIpv6.udb[40] = ((GT_U8)(sourceEPort >> 8) &0x1F);

    /* AUTODOC: action - hard drop */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = PRV_TGF_PACKET_CMD_DROP_HARD_E;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
            IPCL_MATCH_INDEX, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 IPCL_MATCH_INDEX);
}


/**
* @internal prvTgfPclUdbMetadataSrcEportTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPclUdbMetadataSrcEportTestGenerateTraffic
(
    GT_VOID
)
{
    /* AUTODOC: Send packet to port[1] */
    prvTgfPclUdbMetadataSrcEportTestPacketSend(MAIN_SEND_PORT_IDX,
                                               &prvTgfPacketInfo);

    /* AUTODOC: Verify packet flooding in vlan */
    prvTgfPclUdbMetadataSrcEportTestCheckCounters(MAIN_SEND_PORT_IDX,
                                                  prvTgfBurstCount, 1);

    /* AUTODOC: Configure PCL rule for source ePort matching on UDBs */
    prvTgfTrafficPclUdbMetadataSrcEportTestRulesAndActionsSet(
        ASSIGN_SOURCE_EPORT);

    /* AUTODOC: Send packet to port[1] */
    prvTgfPclUdbMetadataSrcEportTestPacketSend(MAIN_SEND_PORT_IDX,
                                               &prvTgfPacketInfo);
    /* AUTODOC: Verify packet drop */
    prvTgfPclUdbMetadataSrcEportTestCheckCounters(MAIN_SEND_PORT_IDX,
                                                  0, 2);

    /* AUTODOC: Send packet to port[2] */
    prvTgfPclUdbMetadataSrcEportTestPacketSend(SECONDARY_SEND_PORT_IDX,
                                               &prvTgfPacketInfo);

    /* AUTODOC: Verify packet flooded in vlan */
    prvTgfPclUdbMetadataSrcEportTestCheckCounters(SECONDARY_SEND_PORT_IDX,
                                                  prvTgfBurstCount, 3);

    /* AUTODOC: Change source ePort rule value on UDBs to cancel matching */
    prvTgfTrafficPclUdbMetadataSrcEportTestRulesAndActionsSet(
        ASSIGN_SOURCE_EPORT-1);

    /* AUTODOC: Send packet to port[1] */
    prvTgfPclUdbMetadataSrcEportTestPacketSend(MAIN_SEND_PORT_IDX,
                                               &prvTgfPacketInfo);

    /* AUTODOC: Verify packet flooded in vlan */
    prvTgfPclUdbMetadataSrcEportTestCheckCounters(MAIN_SEND_PORT_IDX,
                                                  prvTgfBurstCount, 4);
}

/**
* @internal prvTgfPclUdbMetadataSrcEportTest function
* @endinternal
*
* @brief   IPCL lookup for matching source ePort based on metadata test
*/
GT_VOID prvTgfPclUdbMetadataSrcEportTest
(
    GT_VOID
)
{
    prvTgfPclUdbMetadataSrcEportTestConfigurationSet();

    prvTgfPclUdbMetadataSrcEportTestGenerateTraffic();

    prvTgfPclUdbMetadataSrcEportTestConfigurationRestore();
}

