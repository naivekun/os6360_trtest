/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPclQuadLookup.c
*
* DESCRIPTION:
*       PCL Quad lookup advanced UTs.
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
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
#include <common/tgfTcamGen.h>
#include <common/tgfCosGen.h>

#include <pcl/prvTgfPclEgrQuadLookup.h>

#define DEBUG_DROP 0

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID0_CNS          1967
#define PRV_TGF_VLANID1_CNS          1968
#define PRV_TGF_VLANID0_MODIFY_CNS   1969
#define PRV_TGF_VLANID1_MODIFY_CNS   1970

#define PRV_TGF_UP_MODIFY_CNS     5;
/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packets *********************************/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x19},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID0_CNS                           /* pri, cfi, VlanId */
};

/* VLAN_TAG1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID1_CNS                           /* pri, cfi, VlanId */
};

static TGF_PACKET_VLAN_TAG_STC prvTgfExpectedPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID0_CNS                           /* pri, cfi, VlanId */
};

static TGF_PACKET_VLAN_TAG_STC prvTgfExpectedPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID1_CNS                           /* pri, cfi, VlanId */
};
/* DATA of bypass packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of expected packet */
static TGF_PACKET_PART_STC prvTgfExpectedPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfExpectedPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfExpectedPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_CRC_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfPacketPartArray                                       /* partsArray */
};
/* Expected PACKET info */
static TGF_PACKET_STC prvTgfExpectedPacketInfo = {
    PRV_TGF_PACKET_CRC_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfExpectedPacketPartArray) /
        sizeof(TGF_PACKET_PART_STC),                                /* numOfParts */
    prvTgfExpectedPacketPartArray                               /* partsArray */
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* traffic generation sending port */
#define ING_PORT_IDX_CNS    1

/* target port */
#define EGR_PORT_IDX_CNS    0

/* PCL TCAM group binding */
#define EPCL_TCAM_GROUP_CNS  (HARD_WIRE_TCAM_MAC(prvTgfDevNum) ? 4 : 2)

/* PCL rule relative index for tests - 1024 standard rules on floor.
   Each row has 4 standard rules with relative indexes 0,1,2,3
   Devices with 3 floors use only two first floors:
   HIT_0, HIT_1 are in floor 0 - e.g. indexes 0..1 - HIT_0 indexes 2..3 - HIT_1
   HIT_2, HIT_3 are in floor 1 - e.g indexes 1024..1025 - HIT_2 indexes 1026..1027 - HIT_3
   */
#define PCL_RULE_INDEX_HIT_0_CNS prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet((prvTgfTcamFloorsNumGet() <= 3)? 1:1)
#define PCL_RULE_INDEX_HIT_1_CNS prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet((prvTgfTcamFloorsNumGet() <= 3)? 2:1025)
#define PCL_RULE_INDEX_HIT_2_CNS prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet((prvTgfTcamFloorsNumGet() <= 3)? 1024:2049)
#define PCL_RULE_INDEX_HIT_3_CNS prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet((prvTgfTcamFloorsNumGet() <= 3)? 1027:3073)



/* TCAM floor info saved for restore */
static PRV_TGF_TCAM_BLOCK_INFO_STC saveFloorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS][PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];

/* PCL TCAM group binding saved for restore */
static GT_U32 tcamGroup;
static GT_BOOL tcamGroupEnable;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclQuadLookupTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPclQuadLookupTestVlanInit
(
    IN GT_U16           vlanId
)
{
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0,
                 sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

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
    vlanInfo.floodVidxMode        =
                         PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    =
                                  PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;
    vlanInfo.fidValue              = vlanId;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[ING_PORT_IDX_CNS]);

    portsTaggingCmd.portsCmd[prvTgfPortsArray[ING_PORT_IDX_CNS]] =
                                  PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;

    /* reset counters and set force link up */
    rc = prvTgfResetCountersEth(prvTgfDevNum,
                                prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS]);

    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[EGR_PORT_IDX_CNS]);

    portsTaggingCmd.portsCmd[prvTgfPortsArray[EGR_PORT_IDX_CNS]] =
                                  PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;

    /* reset counters and set force link up */
    rc = prvTgfResetCountersEth(prvTgfDevNum,
                                prvTgfPortsArray[EGR_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[EGR_PORT_IDX_CNS]);

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d",
                                 vlanId);
}

/**
* @internal prvTgfPclEgressQuadLookupTcamSegmentModeSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclEgressQuadLookupTcamSegmentModeSet
(
    IN PRV_TGF_PCL_TCAM_SEGMENT_MODE_ENT    tcamSegmentMode
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];
    lookupCfg.enableLookup        = GT_TRUE;

    lookupCfg.groupKeyTypes.nonIpKey =
                                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    lookupCfg.tcamSegmentMode = tcamSegmentMode;

    return prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                              CPSS_PCL_LOOKUP_0_E, &lookupCfg);
}

/**
* @internal prvTgfTrafficPclRulesAndActionsSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfTrafficPclRulesAndActionsSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.ruleEgrStdNotIp.macDa.arEther, 0xFF,
                 sizeof(mask.ruleEgrStdNotIp.macDa.arEther));

    /* AUTODOC: pattern for DST MAC address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemCpy(pattern.ruleEgrStdNotIp.macDa.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: action0 - tag0 vlan value change */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.egressPolicy    =  GT_TRUE;
    action.pktCmd          = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId0Cmd =
                               PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
    action.vlan.vlanId     = PRV_TGF_VLANID0_MODIFY_CNS;

    rc = prvTgfPclRuleSet( PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                           PCL_RULE_INDEX_HIT_0_CNS,
                           &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 PCL_RULE_INDEX_HIT_0_CNS);

    /* AUTODOC: action1 - tag1 vlan value change */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.egressPolicy    =  GT_TRUE;
    action.pktCmd          = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId1ModifyEnable = GT_TRUE;
    action.vlan.vlanId1    = PRV_TGF_VLANID1_MODIFY_CNS;

    rc = prvTgfPclRuleSet( PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                           PCL_RULE_INDEX_HIT_1_CNS,
                           &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 PCL_RULE_INDEX_HIT_1_CNS);

    /* AUTODOC: action2 - UP0 change in tag0*/
    cpssOsMemSet(&action, 0, sizeof(action));
    action.egressPolicy =  GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.qos.egressUp0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E;
    action.qos.up       = PRV_TGF_UP_MODIFY_CNS;

    rc = prvTgfPclRuleSet( PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                           PCL_RULE_INDEX_HIT_2_CNS,
                           &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 PCL_RULE_INDEX_HIT_2_CNS);

    /* AUTODOC: action3 - UP1 change in tag1*/
    cpssOsMemSet(&action, 0, sizeof(action));
    action.egressPolicy =  GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.qos.egressUp1ModifyEnable = GT_TRUE;
    action.qos.up1      = PRV_TGF_UP_MODIFY_CNS;

    rc = prvTgfPclRuleSet( PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                           PCL_RULE_INDEX_HIT_3_CNS,
                           &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 PCL_RULE_INDEX_HIT_3_CNS);

}

/**
* @internal prvTgfTrafficPclEgressRulesValidate function
* @endinternal
*
* @brief   Validate the PCL rules
*/
GT_VOID prvTgfTrafficPclEgressRulesValidate
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: validate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_0_CNS,
                                     GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_0_CNS,
                                 GT_TRUE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_1_CNS,
                                     GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_1_CNS,
                                 GT_TRUE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_2_CNS,
                                     GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_2_CNS,
                                 GT_TRUE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_3_CNS,
                                     GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_3_CNS,
                                 GT_TRUE);
}

/**
* @internal prvTgfPclQuadLookupTestPacketSend function
* @endinternal
*
* @brief   Function sends packet and check results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclQuadLookupTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the
      ethertype of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount,
                             0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum,
                                 portInterfacePtr->devPort.portNum,
                                 GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum,
                                 portInterfacePtr->devPort.portNum,
                                 GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n",
                     portInterfacePtr->devPort.portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPclQuadLookupTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packetand performs trace.
*/
static GT_VOID prvTgfPclQuadLookupTestSendAndCheck
(
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_STATUS        expectedRcRxToCpu,
    IN GT_U32           callIdentifier
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    GT_U32  actualCapturedNumOfPackets;

    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           devNum;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];

    /* AUTODOC: send packet. */
    prvTgfPclQuadLookupTestPacketSend(&portInterface, packetInfoPtr);

    /* check counters */

    /* read Rx counter */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[ING_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS],
                                 callIdentifier);

    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                 "get another goodPktsRcv counter than expected on port %d, %d",
                            prvTgfPortsArray[ING_PORT_IDX_CNS], callIdentifier);

    /* read Tx counter */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                 callIdentifier);

    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                "get another goodPktsSent counter than expected on port %d, %d",
                                 prvTgfPortsArray[EGR_PORT_IDX_CNS],
                                 callIdentifier);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &prvTgfExpectedPacketInfo,
                                        prvTgfBurstCount,
                                        /*numOfPackets*/ 0/*vfdNum*/,
                                        NULL /*vfdArray*/,
                                        NULL, /* bytesNum's skip list */
                                        0,    /* length of skip list */
                                        &actualCapturedNumOfPackets,
                                        NULL/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
 "tgfTrafficGeneratorPortTxEthTriggerCountersGet: port = %d, rc = 0x%02X, %d\n",
                                 portInterface.devPort.portNum, rc,
                                 callIdentifier);

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E, GT_TRUE,
                                       GT_TRUE, packetBuff, &buffLen,
                                       &packetActualLength, &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(expectedRcRxToCpu, rc,
                                 "tgfTrafficGeneratorRxInCpuGet %d",
                                 callIdentifier);

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E, GT_FALSE,
                                       GT_TRUE, packetBuff, &buffLen,
                                       &packetActualLength, &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc,
                                 "tgfTrafficGeneratorRxInCpuGet %d",
                                       callIdentifier);
}

/**
* @internal prvTgfPclEgressQuadLookupTestConfigurationSet function
* @endinternal
*
* @brief   Egress PCL Quad lookup test initial configurations
*/
GT_VOID prvTgfPclEgressQuadLookupTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      floorIndex;
    GT_U32      blockNumber;
    PRV_TGF_TCAM_BLOCK_INFO_STC testFloorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];
    GT_U32      numBanksForHitNumGranularity = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.tcam.numBanksForHitNumGranularity;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 1967 with port 0,1. */
    prvTgfPclQuadLookupTestVlanInit(PRV_TGF_VLANID0_CNS);

    /* AUTODOC: create VLAN 1968 with port 0,1. */
    prvTgfPclQuadLookupTestVlanInit(PRV_TGF_VLANID1_CNS);

    /* AUTODOC: create VLAN 1969 with port 0,1. */
    prvTgfPclQuadLookupTestVlanInit(PRV_TGF_VLANID0_MODIFY_CNS);

    /* AUTODOC: create VLAN 1970 with port 0,1. */
    prvTgfPclQuadLookupTestVlanInit(PRV_TGF_VLANID1_MODIFY_CNS);

    /* AUTODOC: Save TCAM floor info for restore */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupGet(floorIndex,
                                              &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupGet: %d",
            floorIndex);
    }

    if(prvTgfTcamFloorsNumGet() <= 3)
    {
        /* use only two floor 0, 1:
           HIT_0, HIT_1 are in floor 0
           HIT_2, HIT_3 are in floor 1*/
        for( floorIndex = 0 ; floorIndex < 2 ; floorIndex++ )
        {
            for (blockNumber = 0; blockNumber < numBanksForHitNumGranularity; blockNumber++)
            {
                testFloorInfoArr[blockNumber].hitNum = floorIndex*2 + ((blockNumber < (numBanksForHitNumGranularity/2))? 0 : 1);
                testFloorInfoArr[blockNumber].group = EPCL_TCAM_GROUP_CNS;
            }
            rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                                       &testFloorInfoArr[0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
        }
    }
    else
    {
        /* AUTODOC: Clear TCAM floor info for Quad lookup */
        for (blockNumber = 0; blockNumber < numBanksForHitNumGranularity; blockNumber++)
        {
            testFloorInfoArr[blockNumber].group = 0;
            testFloorInfoArr[blockNumber].hitNum = 0;
        }

        if (numBanksForHitNumGranularity == PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS)
        {
            /* AUTODOC: Configure Caelum TCAM floor info for Quad lookup
               Block #0 has no effect since PCL standard key rule will be configured in banks 3..5 (Blocks #1 and #2) */
            testFloorInfoArr[1].group =
            testFloorInfoArr[2].group = EPCL_TCAM_GROUP_CNS;
        }
        else
        {
            /* AUTODOC: Configure TCAM floor info for Quad lookup */
            for (blockNumber = 0; blockNumber < numBanksForHitNumGranularity; blockNumber++)
            {
                testFloorInfoArr[blockNumber].group = EPCL_TCAM_GROUP_CNS;
            }
        }
        for( floorIndex = 0 ; floorIndex < 4 ; floorIndex++ )
        {
            for (blockNumber = 0; blockNumber < numBanksForHitNumGranularity; blockNumber++)
            {
                testFloorInfoArr[blockNumber].hitNum = floorIndex;
            }

            rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                                       &testFloorInfoArr[0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
        }

        /* update the AppDemo that new ranges are used by the test.
        needed for proper functionality of function :
            prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet() */
        appDemoDxChTcamClientSectionSet(CPSS_DXCH_TCAM_EPCL_E,
            0,/*appDemo not support EPCL per hitNum ... so use 0 */
            0,/*baseIndex*/
            4*_3K);/*4 floors -> numOfIndexes*/
    }

    if(!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Save PCL TCAM group binding for restore */
        rc = prvTgfTcamClientGroupGet(PRV_TGF_TCAM_EPCL_E, &tcamGroup,
                                      &tcamGroupEnable);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupGet: %d, %d, %d",
                PRV_TGF_TCAM_EPCL_E, tcamGroup, tcamGroupEnable);

        /* AUTODOC: PCL TCAM group binding */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_EPCL_E, EPCL_TCAM_GROUP_CNS,
                                      GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                PRV_TGF_TCAM_EPCL_E, EPCL_TCAM_GROUP_CNS, GT_TRUE);
    }

    /* AUTODOC: Init EPCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[EGR_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclEgressQuadLookupTestConfigurationRestore function
* @endinternal
*
* @brief   Egress PCL Quad lookup restore configurations
*/
GT_VOID prvTgfPclEgressQuadLookupTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      floorIndex;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_0_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_1_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_1_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_2_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_2_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_3_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_3_CNS,
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

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID0_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID1_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID1_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID0_MODIFY_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID0_MODIFY_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID1_MODIFY_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID1_MODIFY_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore TCAM floor info */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                              &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
            floorIndex);
    }

    /* restore TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
    appDemoDxChTcamSectionsRestore();


    if(!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Restore PCL TCAM group binding */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_EPCL_E, tcamGroup,
                                      tcamGroupEnable);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                PRV_TGF_TCAM_EPCL_E, tcamGroup, tcamGroupEnable);
    }
}

/**
* @internal prvTgfPclEgressQuadLookupTest function
* @endinternal
*
* @brief   EPCL Quad lookup test
*/
GT_VOID prvTgfPclEgressQuadLookupTest
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      sendCallInst = 0;

    prvTgfPclEgressQuadLookupTestConfigurationSet();

    /* AUTODOC: Verify VLAN flooding (no PCL rules yet defined) */
    prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo,
                                        GT_NO_MORE, sendCallInst++/*0*/);

    /* AUTODOC: Configure "Quad" PCL rules */
    prvTgfTrafficPclRulesAndActionsSet();

    /* SIP_5_20 only '4 TCAM mode' exists */
    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Configure TCAM Segment Mode to 1_TCAM */
        rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                                           PRV_TGF_PCL_TCAM_SEGMENT_MODE_1_TCAM_E );

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet");

        /* AUTODOC: PCL action0 only */
        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_MODIFY_CNS;

        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*1*/);

        /* AUTODOC: Configure TCAM Segment Mode to 2_TCAMS_2_AND_2 */
        rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                                   PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet");

        /* AUTODOC: PCL action0 & action2 */

        prvTgfExpectedPacketVlanTag0Part.pri = PRV_TGF_UP_MODIFY_CNS;

        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*2*/);

        prvTgfExpectedPacketVlanTag0Part.pri = 0;

        /* AUTODOC: Configure TCAM Segment Mode to 2_TCAMS_1_AND_3 */
        rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                                  PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E );
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet");

        /* AUTODOC: PCL action0 & action1 */

        prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_MODIFY_CNS;

        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*3*/);

        prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_CNS;

        /* AUTODOC: Configure TCAM Segment Mode to 2_TCAMS_3_AND_1 */
        rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                                   PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet");

        /* AUTODOC: PCL action0 & action3 */

        prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;

        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*4*/);

        prvTgfExpectedPacketVlanTag1Part.pri = 0;
    }
    else
    {
        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_MODIFY_CNS;
    }


    /* AUTODOC: Configure TCAM Segment Mode to 4_TCAMS */
    rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                                      PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet");

    prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_MODIFY_CNS;
    prvTgfExpectedPacketVlanTag0Part.pri = PRV_TGF_UP_MODIFY_CNS;
    prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;

    /* AUTODOC: PCL action0, action1, action2 & action3 */
    prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                        sendCallInst++/*5*/);

    prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_CNS;
    prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_CNS;
    prvTgfExpectedPacketVlanTag0Part.pri = 0;
    prvTgfExpectedPacketVlanTag1Part.pri = 0;

    prvTgfPclEgressQuadLookupTestConfigurationRestore();
}

/**
* @internal prvTgfPclEgressQuadLookupTestWithInvalid function
* @endinternal
*
* @brief   EPCL Quad lookup test with invalid rules
*/
GT_VOID prvTgfPclEgressQuadLookupTestWithInvalid
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      sendCallInst = 0;

    prvTgfPclEgressQuadLookupTestConfigurationSet();

    /* AUTODOC: Configure "Quad" PCL rules */
    prvTgfTrafficPclRulesAndActionsSet();

    /* SIP_5_20 only '4 TCAM mode' exists */
    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Configure TCAM Segment Mode to 1_TCAM */
        rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                            PRV_TGF_PCL_TCAM_SEGMENT_MODE_1_TCAM_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet: %d",
                PRV_TGF_PCL_TCAM_SEGMENT_MODE_1_TCAM_E);

        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_MODIFY_CNS;
        /* AUTODOC: PCL action0 only */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*1*/);

        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_CNS;

        /* AUTODOC: invalidate PCL rule0 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_0_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_0_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_MODIFY_CNS;
        /* AUTODOC: PCL action1 only */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*2*/);

        prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_CNS;

        /* AUTODOC: invalidate PCL rule1 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_1_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_1_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag0Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action2 only */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*3*/);

        prvTgfExpectedPacketVlanTag0Part.pri = 0;

        /* AUTODOC: invalidate PCL rule2 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_2_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_2_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action3 only */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*4*/);

        prvTgfExpectedPacketVlanTag1Part.pri = 0;

        /* AUTODOC: invalidate PCL rule3 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_3_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_3_CNS,
                                     GT_FALSE);

        /* AUTODOC: Flood to vlan */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*5*/);

        prvTgfTrafficPclEgressRulesValidate();

        /* AUTODOC: Configure TCAM Segment Mode to 2_TCAMS_2_AND_2 */
        rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                            PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet: %d",
                PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E);

        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_MODIFY_CNS;
        prvTgfExpectedPacketVlanTag0Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action0 & action2 */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*6*/);

        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_CNS;
        prvTgfExpectedPacketVlanTag0Part.pri = 0;

        /* AUTODOC: invalidate PCL rule0 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_0_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_0_CNS,
                                     GT_FALSE);

        /* AUTODOC: invalidate PCL rule2 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_2_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_2_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_MODIFY_CNS;
        prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action1 & action3 */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*7*/);

        prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_CNS;
        prvTgfExpectedPacketVlanTag1Part.pri = 0;

        /* AUTODOC: invalidate PCL rule1 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_1_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_1_CNS,
                                     GT_FALSE);

        /* AUTODOC: invalidate PCL rule3 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_3_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_3_CNS,
                                     GT_FALSE);

        /* AUTODOC: Flood to vlan */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*8*/);

        prvTgfTrafficPclEgressRulesValidate();

        /* AUTODOC: Configure TCAM Segment Mode to 2_TCAMS_1_AND_3 */
        rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                            PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet: %d",
                PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E);

        /* AUTODOC: invalidate PCL rule1 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_1_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_1_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_MODIFY_CNS;
        prvTgfExpectedPacketVlanTag0Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action0 & action2 */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*9*/);

        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_CNS;
        prvTgfExpectedPacketVlanTag0Part.pri = 0;

        /* AUTODOC: invalidate PCL rule2 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_2_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_2_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_MODIFY_CNS;
        prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action0 & action3 */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*10*/);

        prvTgfExpectedPacketVlanTag0Part.vid = PRV_TGF_VLANID0_CNS;
        prvTgfExpectedPacketVlanTag1Part.pri = 0;

        prvTgfTrafficPclEgressRulesValidate();

        /* AUTODOC: Configure TCAM Segment Mode to 2_TCAMS_3_AND_1 */
        rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                            PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclEgressQuadLookupTcamSegmentModeSet: %d",
                PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E);

        /* AUTODOC: invalidate PCL rule0 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_0_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_0_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_MODIFY_CNS;
        prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action1 & action3 */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*11*/);

        prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_CNS;
        prvTgfExpectedPacketVlanTag1Part.pri = 0;

        /* AUTODOC: invalidate PCL rule1 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_1_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_1_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag0Part.pri = PRV_TGF_UP_MODIFY_CNS;
        prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action2 & action3 */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*12*/);

        prvTgfExpectedPacketVlanTag0Part.pri = 0;
        prvTgfExpectedPacketVlanTag1Part.pri = 0;

        /* AUTODOC: invalidate PCL rule2 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PCL_RULE_INDEX_HIT_2_CNS,
                                         GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_2_CNS,
                                     GT_FALSE);

        prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
        /* AUTODOC: PCL action3 only */
        prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                            sendCallInst++/*13*/);

        prvTgfExpectedPacketVlanTag1Part.pri = 0;
    }

    prvTgfTrafficPclEgressRulesValidate();

    /* AUTODOC: Configure TCAM Segment Mode to 4_TCAMS */
    rc = prvTgfPclEgressQuadLookupTcamSegmentModeSet(
                        PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
                            GT_OK, rc,
                            "prvTgfPclEgressQuadLookupTcamSegmentModeSet: %d",
                            PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E);

    /* AUTODOC: invalidate PCL rule0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_0_CNS,
                                 GT_FALSE);

    prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_MODIFY_CNS;
    prvTgfExpectedPacketVlanTag0Part.pri = PRV_TGF_UP_MODIFY_CNS;
    prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
    /* AUTODOC: PCL action1, action2 & action3 */
    prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                        sendCallInst++/*14*/);

    prvTgfExpectedPacketVlanTag1Part.vid = PRV_TGF_VLANID1_CNS;
    prvTgfExpectedPacketVlanTag0Part.pri = 0;
    prvTgfExpectedPacketVlanTag1Part.pri = 0;


    /* AUTODOC: invalidate PCL rule1 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_1_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_1_CNS,
                                 GT_FALSE);

    prvTgfExpectedPacketVlanTag0Part.pri = PRV_TGF_UP_MODIFY_CNS;
    prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
    /* AUTODOC: PCL action2 & action3 */
    prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                        sendCallInst++/*15*/);

    prvTgfExpectedPacketVlanTag0Part.pri = 0;
    prvTgfExpectedPacketVlanTag1Part.pri = 0;

    /* AUTODOC: invalidate PCL rule2 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_2_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_2_CNS,
                                 GT_FALSE);

    prvTgfExpectedPacketVlanTag1Part.pri = PRV_TGF_UP_MODIFY_CNS;
    /* AUTODOC: PCL action3 only */
    prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                        sendCallInst++/*16*/);

    prvTgfExpectedPacketVlanTag1Part.pri = 0;

    /* AUTODOC: invalidate PCL rule3 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_HIT_3_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_HIT_3_CNS,
                                 GT_FALSE);

    /* AUTODOC: Flood to vlan */
    prvTgfPclQuadLookupTestSendAndCheck(&prvTgfPacketInfo, GT_NO_MORE,
                                        sendCallInst++/*17*/);

    prvTgfPclEgressQuadLookupTestConfigurationRestore();
}

