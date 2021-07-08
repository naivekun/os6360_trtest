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
* @file prvTgfPolicerECNMarkingCheck.c
*
* @brief Test of UP modification modes due to Egress policer.
*
* @version   3
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

#include <policer/prvTgfPolicerECNMarkingCheck.h>
#include <policer/prvTgfPolicerEgressUpModify.h>
#include <policer/prvTgfPolicerEarchMeterEntry.h>

#include <common/tgfCommon.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfBridgeGen.h>

#define PRV_TGF_VLANID_0_CNS 10

#define PRV_TGF_VLANID_1_CNS 11

/* port index 0 */
#define PRV_TGF_PORT_IDX_0_CNS     0
/* port index 1 */
#define PRV_TGF_PORT_IDX_1_CNS     1
/* port index 2 */
#define PRV_TGF_PORT_IDX_2_CNS     2
/* port index 3 */
#define PRV_TGF_PORT_IDX_3_CNS     3

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/******************************* Test packet **********************************/

/* L2 tunnel part */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x09, 0x0A},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C}                /* saMac */
};
  
/* VLAN_TAG 0 OUTER part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_0_CNS                          /* pri, cfi, VlanId */
};

/* VLAN_TAG 1 INNER part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    1,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* Expected packet's IPv4 - ECN (TOS) is 3 */
static TGF_PACKET_IPV4_STC prvTgfExpectedPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    3,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of double packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,          &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E,   &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,     &prvTgfPacketPayloadPart}
};

/* expected VLAN_TAG 1 OUTER part */
static TGF_PACKET_VLAN_TAG_STC prvTgfExpectedPacketVlanOuterTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
};

/* expected VLAN_TAG 0 INNER part */
static TGF_PACKET_VLAN_TAG_STC prvTgfExpectedPacketVlanInnerTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_0_CNS                    /* pri, cfi, VlanId */
};

/* PARTS of Expected double packet */
static TGF_PACKET_PART_STC prvTgfExpectedPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,          &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfExpectedPacketVlanOuterTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfExpectedPacketVlanInnerTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E,   &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfExpectedPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,     &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + 2*TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* Expected PACKET to compare info */
static TGF_PACKET_STC prvTgfExpectedPacketInfo = {
    PRV_TGF_PACKET_CRC_LEN_CNS,                                          /* totalLen */
    sizeof(prvTgfExpectedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfExpectedPacketPartArray                                        /* partsArray */
};

/**********************/
/* Restore parameters */
/**********************/

/* save for restore stage metering enable */
static GT_BOOL saveStageMeterEnable;

/* save for restore port metering enable */
static GT_BOOL savePortMeterEnable;

/* save for restore meter mode: port or flow */
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT saveMeterMode;

/* saved policer configuration */
static PRV_TGF_POLICER_ENTRY_STC            saveMeterEntry;

/**
* @internal prvTgfPolicerECNMarkingCheckVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPolicerECNMarkingCheckVlanInit
(
    IN GT_U16           vlanId
)
{
    GT_U32                      portIter  = 0;
    GT_U32                      portCount = 0;
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

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;

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
* @internal prvTgfPolicerECNMarkingConfigurationSet function
* @endinternal
*
* @brief   Test configurations
*/
GT_VOID prvTgfPolicerECNMarkingConfigurationSet()
{
    GT_STATUS rc;
    PRV_TGF_POLICER_QOS_PARAM_STC   qosParam;

    /* AUTODOC: Enable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_TRUE);

    /* AUTODOC: Create vlan on all ports */
    prvTgfPolicerECNMarkingCheckVlanInit(PRV_TGF_VLANID_0_CNS);

    /* AUTODOC: Save stage metering enable for restore */
    rc = prvTgfPolicerMeteringEnableGet(prvTgfDevNum, 
                                        PRV_TGF_POLICER_STAGE_EGRESS_E,
                                        &saveStageMeterEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum, 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 saveStageMeterEnable);

    /* AUTODOC: Enable stage for metering */
    rc = prvTgfPolicerMeteringEnableSet(PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);

    /* AUTODOC: Save port metering enable for restore */
    rc = prvTgfPolicerPortMeteringEnableGet(prvTgfDevNum, 
                                            PRV_TGF_POLICER_STAGE_EGRESS_E,
                                            prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                            &savePortMeterEnable);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerPortMeteringEnableGet: %d, %d, %d, %d",
                                 prvTgfDevNum, 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                 savePortMeterEnable);

    /* AUTODOC: Enable port for metering */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, 
                                            PRV_TGF_POLICER_STAGE_EGRESS_E,
                                            prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                            GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum, 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                 GT_TRUE);

    /* AUTODOC: Save metering mode for restore */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, 
                                        PRV_TGF_POLICER_STAGE_EGRESS_E,
                                        &saveMeterMode);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum, 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 saveMeterMode);   

    /* AUTODOC: Set metering mode for "port" mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E,
                                        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* Save Policer entry, configure counter set 1 */
    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                PRV_TGF_POLICER_STAGE_EGRESS_E,
                                prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                &saveMeterEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfPolicerEntryGet: %d", prvTgfDevNum);

    /* AUTODOC: Initial metering entry setting */
    prvTgfPolicerEgressInitEntrySet(PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_PORT_IDX_2_CNS);

    /* AUTODOC: Set L2 remark model to UP */
    rc = prvTgfPolicerL2RemarkModelSet(PRV_TGF_POLICER_STAGE_EGRESS_E,
                                       PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerL2RemarkModelSet %d, %d, %d", 
                                 prvTgfDevNum, 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E);

    /* AUTODOC: Configure Qos Remarking entry for UP Remarking and CL is RED */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));
    qosParam.up   = 1;
    qosParam.dp   = CPSS_DP_RED_E;

    rc = prvTgfPolicerEgressQosRemarkingEntrySet(PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                                 0,
                                                 CPSS_DP_RED_E,
                                                 &qosParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerEgressQosRemarkingEntrySet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerECNMarkingRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPolicerECNMarkingRestore()
{
    GT_STATUS rc;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    PRV_TGF_POLICER_QOS_PARAM_STC   qosParam;

    /* restore expected array */
    prvTgfExpectedPacketInfo.partsArray[4].partPtr = &prvTgfExpectedPacketIpv4Part;

    /* AUTODOC: Restore metering entry */
    rc = prvTgfPolicerEntrySet(PRV_TGF_POLICER_STAGE_EGRESS_E,
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &saveMeterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Restore L2 remark model to TC */
    rc = prvTgfPolicerL2RemarkModelSet(PRV_TGF_POLICER_STAGE_EGRESS_E,
                                       PRV_TGF_POLICER_L2_REMARK_MODEL_TC_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerL2RemarkModelSet %d, %d, %d", 
                                 prvTgfDevNum, 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 PRV_TGF_POLICER_L2_REMARK_MODEL_TC_E);

    /* AUTODOC: Clear Qos Remarking entry for UP Remarking and CL is RED */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));

    rc = prvTgfPolicerEgressQosRemarkingEntrySet(PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                                 0,
                                                 CPSS_DP_RED_E,
                                                 &qosParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerEgressQosRemarkingEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: Restore stage metering enable */
    rc = prvTgfPolicerMeteringEnableSet(PRV_TGF_POLICER_STAGE_EGRESS_E,
                                        saveStageMeterEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 saveStageMeterEnable);

    /* AUTODOC: Restore port metering enable */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, 
                                            PRV_TGF_POLICER_STAGE_EGRESS_E,
                                            prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                            savePortMeterEnable);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum, 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                 savePortMeterEnable);

    /* AUTODOC: Restore metering mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E,
                                        saveMeterMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 saveMeterMode);

    /* AUTODOC: Clear management counters */
    prvTgfPolicerManagementCountersReset(PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_POLICER_MNG_CNTR_SET1_E);

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* Invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_0_CNS);

    /* AUTODOC: Disable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_FALSE);
}

/**
* @internal prvTgfPolicerECNMarkingGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPolicerECNMarkingGenerateTraffic()
{
    GT_STATUS                       rc;
    GT_U32                          partsCount;
    GT_U32                          packetSize;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          plrMru;
    PRV_TGF_POLICER_ENTRY_STC           meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_BOOL                            enableECN = GT_FALSE;

    GT_U32          l1PacketSize;

    /* AUTODOC: Double tagged traffic start */
    /*Enable ECN marking*/
    rc = prvTgfPortEcnMarkingEnableSet(CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failed to enable ECN marking. prvTgfDxChPortEcnMarkingEnableSet: %d", rc);

    rc = prvTgfPortEcnMarkingEnableGet(CPSS_IP_PROTOCOL_IPV4_E, &enableECN);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failed to get ECN value from register. prvTgfDxChPortEcnMarkingEnableGet: %d", rc);

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "IPv4oIPv4, prvTgfPacketSizeGet: %d, %d",
                                 partsCount, packetSize);

    /* AUTODOC: build packet info */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    l1PacketSize = packetSize + 4/*crc*/ + 8/*ipg*/ + 12/*preamble*/;

    rc = prvTgfPolicerMruGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &plrMru);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruGet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, plrMru);

    /* AUTODOC: Phase 0 - Enable ECN marking and check ECN value to be marked (TOS byte = 3) */
    /* AUTODOC: Clear management counters */
    prvTgfPolicerManagementCountersReset(PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_POLICER_MNG_CNTR_SET1_E);

    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                               PRV_TGF_POLICER_STAGE_EGRESS_E,
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);
    /* AUTODOC: Meter Entry - Enable ECN marking */
    meterEntry.yellowEcnMarkingEnable = GT_TRUE;

    /* AUTODOC: Meter Entry - Set Metering based on packets per second */
    meterEntry.byteOrPacketCountingMode = 
        PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;

    /* AUTODOC: Meter Entry - Set TB to mark packet as YELLOW */
    meterEntry.tbParams.srTcmParams.cbs = plrMru;
    meterEntry.tbParams.srTcmParams.ebs = ((plrMru >= l1PacketSize) ? plrMru : l1PacketSize) + 1;

    /* AUTODOC: Meter Entry - No UP modify */
    meterEntry.modifyUp = PRV_TGF_POLICER_MODIFY_UP_DISABLE_E;

    rc = prvTgfPolicerEntrySet(PRV_TGF_POLICER_STAGE_EGRESS_E,
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Send packet - YELLOW */
    prvTgfPolicerEgressTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                PRV_TGF_PORT_IDX_2_CNS,
                                                &packetInfo, prvTgfBurstCount);

    /* AUTODOC: Verify packet flooding and capture of the packet */
    prvTgfPolicerEgressCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                     PRV_TGF_PORT_IDX_2_CNS,
                                     prvTgfBurstCount,
                                     0,
                                     &prvTgfExpectedPacketInfo,
                                     prvTgfBurstCount);

    /* AUTODOC: Expected management counters */
    /*          Yellow - 1 packet */
    prvTgfPolicerEarchEgrMeterEntryManagementCountCheck(PRV_TGF_POLICER_MNG_CNTR_YELLOW_E);

    /* AUTODOC: Phase 1 - Disable ECN marking in meter entry and check ECN value to be not changed */
    /* AUTODOC: Meter Entry - Disable ECN marking */
    meterEntry.yellowEcnMarkingEnable = GT_FALSE;
    rc = prvTgfPolicerEntrySet(PRV_TGF_POLICER_STAGE_EGRESS_E,
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Clear management counters */
    prvTgfPolicerManagementCountersReset(PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_POLICER_MNG_CNTR_SET1_E);

    /* AUTODOC: Send packet - YELLOW */
    prvTgfPolicerEgressTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                      PRV_TGF_PORT_IDX_2_CNS,
                                      &packetInfo, prvTgfBurstCount);

    /* change expected packet IP header to be like original */
    prvTgfExpectedPacketInfo.partsArray[4].partPtr = &prvTgfPacketIpv4Part;

    /* AUTODOC: Verify packet flooding and capture of the packet */
    prvTgfPolicerEgressCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                     PRV_TGF_PORT_IDX_2_CNS,
                                     prvTgfBurstCount,
                                     0,
                                     &prvTgfExpectedPacketInfo,
                                     prvTgfBurstCount);

    /* AUTODOC: Expected management counters */
    /*          Yellow - 1 packet */
    prvTgfPolicerEarchEgrMeterEntryManagementCountCheck(PRV_TGF_POLICER_MNG_CNTR_YELLOW_E);
}


