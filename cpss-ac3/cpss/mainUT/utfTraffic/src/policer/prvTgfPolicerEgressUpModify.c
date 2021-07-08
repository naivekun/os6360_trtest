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
* @file prvTgfPolicerEgressUpModify.c
*
* @brief Test of UP modification modes due to Egress policer.
*
* @version   6
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

#include <policer/prvTgfPolicerEgressUpModify.h>

#include <common/tgfCommon.h>
#include <common/tgfPolicerGen.h>
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
    {TGF_PACKET_PART_PAYLOAD_E,     &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + 2*TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send info */
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
* @internal prvTgfPolicerEgressUpModifyTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPolicerEgressUpModifyTestVlanInit
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
* @internal prvTgfPolicerEgressUpModifyManagementCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters
*/
static GT_VOID prvTgfPolicerEgressUpModifyManagementCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       policerStage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT    mngCntrSet,
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT   mngCntrType,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr,
    IN GT_U32 callIndex
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            mngCntrSet,
                                            mngCntrType,
                                            &prvTgfMngCntr);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "%d: prvTgf3PolicerManagementCountersGet: %d", callIndex, prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL5_STRING_MAC(
        duLowCntr, prvTgfMngCntr.duMngCntr.l[0],
        "%d: prvTgfMngCntr.duMngCntr.l[0] = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        callIndex, prvTgfMngCntr.duMngCntr.l[0], policerStage, mngCntrSet, mngCntrType);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        duHiCntr, prvTgfMngCntr.duMngCntr.l[1],
        "%d: prvTgfMngCntr.duMngCntr.l[1] = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        callIndex, prvTgfMngCntr.duMngCntr.l[1], policerStage, mngCntrSet, mngCntrType);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        packetMngCntr, prvTgfMngCntr.packetMngCntr,
        "%d: prvTgfMngCntr.packetMngCntr = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        callIndex, prvTgfMngCntr.packetMngCntr, policerStage, mngCntrSet, mngCntrType);
}

/**
* @internal prvTgfPolicerEgressUpModifyFullManagementCountersCheck function
* @endinternal
*
* @brief   Check Management and Billing counters value
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
* @param[in] expMngCounters[4]        - expected Management counters value
* @param[in] callIndex                - reference to this check sequence call
*                                       None
*/
static GT_VOID prvTgfPolicerEgressUpModifyFullManagementCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT     mngSet,
    IN GT_U32                               expMngCounters[4][3],
    IN GT_U32                               callIndex
)
{
    prvTgfPolicerEgressUpModifyManagementCountersCheck(stage, mngSet,
                                     PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][0],
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][1],
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][2],
                                     callIndex);

    prvTgfPolicerEgressUpModifyManagementCountersCheck(stage, mngSet,
                                     PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][0],
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][1],
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][2],
                                     callIndex);

    prvTgfPolicerEgressUpModifyManagementCountersCheck(stage, mngSet,
                                     PRV_TGF_POLICER_MNG_CNTR_RED_E,
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][0],
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][1],
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][2],
                                     callIndex);

    prvTgfPolicerEgressUpModifyManagementCountersCheck(stage, mngSet,
                                     PRV_TGF_POLICER_MNG_CNTR_DROP_E,
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][0],
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][1],
                                     expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][2],
                                     callIndex);
}

/**
* @internal prvTgfPolicerEgressUpModifyConfigurationSet function
* @endinternal
*
* @brief   Test configurations
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerEgressUpModifyConfigurationSet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_QOS_PARAM_STC   qosParam;

    /* AUTODOC: Enable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_TRUE);

    /* AUTODOC: Create vlan on all ports */
    prvTgfPolicerEgressUpModifyTestVlanInit(PRV_TGF_VLANID_0_CNS);

    /* AUTODOC: Save stage metering enable for restore */
    rc = prvTgfPolicerMeteringEnableGet(prvTgfDevNum, 
                                        stage, 
                                        &saveStageMeterEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum, 
                                 stage, 
                                 saveStageMeterEnable);

    /* AUTODOC: Enable stage for metering */
    rc = prvTgfPolicerMeteringEnableSet(stage, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage, GT_TRUE);

    /* AUTODOC: Save port metering enable for restore */
    rc = prvTgfPolicerPortMeteringEnableGet(prvTgfDevNum, 
                                            stage, 
                                            prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                            &savePortMeterEnable);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerPortMeteringEnableGet: %d, %d, %d, %d",
                                 prvTgfDevNum, 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                 savePortMeterEnable);

    /* AUTODOC: Enable port for metering */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, 
                                            stage, 
                                            prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                            GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum, 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                 GT_TRUE);

    /* AUTODOC: Save metering mode for restore */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, 
                                        stage, 
                                        &saveMeterMode);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum, 
                                 stage, 
                                 saveMeterMode);   

    /* AUTODOC: Set metering mode for "port" mode */
    rc = prvTgfPolicerStageMeterModeSet(stage, 
                                        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage, 
                                 PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* Save Policer entry, configure counter set 1 */
    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                stage,
                                prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                &saveMeterEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfPolicerEntryGet: %d", prvTgfDevNum);

    /* AUTODOC: Initial metering entry setting */
    prvTgfPolicerEgressInitEntrySet(stage, PRV_TGF_PORT_IDX_2_CNS);

    /* AUTODOC: Set L2 remark model to UP */
    rc = prvTgfPolicerL2RemarkModelSet(stage, 
                                       PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerL2RemarkModelSet %d, %d, %d", 
                                 prvTgfDevNum, 
                                 stage,
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
* @internal prvTgfPolicerEgressUpModifyRestore function
* @endinternal
*
* @brief   Test restore configurations
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerEgressUpModifyRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    PRV_TGF_POLICER_QOS_PARAM_STC   qosParam;

    /* AUTODOC: Restore metering entry */
    rc = prvTgfPolicerEntrySet(stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &saveMeterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Restore L2 remark model to TC */
    rc = prvTgfPolicerL2RemarkModelSet(stage, 
                                       PRV_TGF_POLICER_L2_REMARK_MODEL_TC_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerL2RemarkModelSet %d, %d, %d", 
                                 prvTgfDevNum, 
                                 stage,
                                 PRV_TGF_POLICER_L2_REMARK_MODEL_TC_E);

    /* AUTODOC: Clear Qos Remarking entry for UP Remarking and CL is RED */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));

    rc = prvTgfPolicerEgressQosRemarkingEntrySet(PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                                 0,
                                                 CPSS_DP_RED_E,
                                                 &qosParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerEgressQosRemarkingEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: Restore stage metering enable */
    rc = prvTgfPolicerMeteringEnableSet(stage, 
                                        saveStageMeterEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage, 
                                 saveStageMeterEnable);

    /* AUTODOC: Restore port metering enable */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, 
                                            stage, 
                                            prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                            savePortMeterEnable);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum, 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                 savePortMeterEnable);

    /* AUTODOC: Restore metering mode */
    rc = prvTgfPolicerStageMeterModeSet(stage, 
                                        saveMeterMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage, 
                                 saveMeterMode);

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
* @internal prvTgfPolicerEgressUpModifyTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPolicerEgressUpModifyTestGenerateTraffic
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS                       rc;
    GT_U32                          partsCount;
    GT_U32                          packetSize;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          plrMru;
    PRV_TGF_POLICER_ENTRY_STC           meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_U32                              expMngCounters[4][3];
    GT_U32                              callIndex = 0;
    GT_U32                              l1PacketSize;

    /* AUTODOC: Double tagged traffic start */
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

    /* exclude CRC if need */
    if (GT_TRUE == prvTgfPolicerEgressIsByteCountCrcExclude())
    {
        l1PacketSize -= 4;
    }
    /* include DSA on remote port */
    if ((stage == PRV_TGF_POLICER_STAGE_EGRESS_E) &&
        (prvCpssDxChPortRemotePortCheck(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS])))
    {
        l1PacketSize += 4;
    }

    rc = prvTgfPolicerMruGet(prvTgfDevNum, stage, &plrMru);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruGet: %d, %d, %d",
                                 prvTgfDevNum, stage, plrMru);

    /* Phase 0 */
    /* AUTODOC: Clear management counters */
    prvTgfPolicerManagementCountersReset(stage, PRV_TGF_POLICER_MNG_CNTR_SET1_E);

    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                               stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Set Metering based on packets per second */
    meterEntry.byteOrPacketCountingMode = 
        PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;

    /* AUTODOC: Set TB to mark packet as RED */
    meterEntry.tbParams.srTcmParams.cbs = plrMru;
    meterEntry.tbParams.srTcmParams.ebs = plrMru;

    /* AUTODOC: No UP modify */
    meterEntry.modifyUp = PRV_TGF_POLICER_MODIFY_UP_DISABLE_E;
    /*meterEntry.modifyUp = PRV_TGF_POLICER_MODIFY_UP_ENABLE_E;*/
    /*meterEntry.modifyUp = PRV_TGF_POLICER_MODIFY_UP_ENABLE_TAG0_E;*/

    rc = prvTgfPolicerEntrySet(stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Send packet - RED */
    prvTgfPolicerEgressTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                PRV_TGF_PORT_IDX_2_CNS,
                                                &packetInfo,
                                                prvTgfBurstCount);

    /* AUTODOC: Verify packet flooding */
    prvTgfPolicerEgressCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                     PRV_TGF_PORT_IDX_2_CNS,
                                     prvTgfBurstCount,
                                     0,
                                     &prvTgfExpectedPacketInfo,
                                     prvTgfBurstCount);

    /* AUTODOC: Expected managenet counters */
    cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][0] = l1PacketSize;
    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][1] = 0;
    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][2] = 1;

    prvTgfPolicerEgressUpModifyFullManagementCountersCheck(stage,
                                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                                           expMngCounters,
                                                           callIndex++/*0*/);

    /* Phase 1 */
    /* AUTODOC: Clear management counters */
    prvTgfPolicerManagementCountersReset(stage, PRV_TGF_POLICER_MNG_CNTR_SET1_E);

    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                               stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Outer UP modify */
    meterEntry.modifyUp = PRV_TGF_POLICER_MODIFY_UP_ENABLE_E;
    prvTgfExpectedPacketVlanOuterTag1Part.pri = 1;

    rc = prvTgfPolicerEntrySet(stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Send packet - RED */
    prvTgfPolicerEgressTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                PRV_TGF_PORT_IDX_2_CNS,
                                                &packetInfo,
                                                prvTgfBurstCount);

    /* AUTODOC: Verify packet flooding */
    prvTgfPolicerEgressCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                     PRV_TGF_PORT_IDX_2_CNS,
                                     prvTgfBurstCount,
                                     0,
                                     &prvTgfExpectedPacketInfo,
                                     prvTgfBurstCount);

    prvTgfExpectedPacketVlanOuterTag1Part.pri = 0;

    prvTgfPolicerEgressUpModifyFullManagementCountersCheck(stage,
                                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                                           expMngCounters,
                                                           callIndex++/*1*/);
    /* Phase 2 */
    /* AUTODOC: Clear management counters */
    prvTgfPolicerManagementCountersReset(stage, PRV_TGF_POLICER_MNG_CNTR_SET1_E);

    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                               stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: UP0 modify */
    meterEntry.modifyUp = PRV_TGF_POLICER_MODIFY_UP_ENABLE_TAG0_E;
    prvTgfExpectedPacketVlanInnerTag0Part.pri = 1;

    rc = prvTgfPolicerEntrySet(stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Send packet - RED */
    prvTgfPolicerEgressTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                PRV_TGF_PORT_IDX_2_CNS,
                                                &packetInfo,
                                                prvTgfBurstCount);

    /* AUTODOC: Verify packet flooding */
    prvTgfPolicerEgressCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                     PRV_TGF_PORT_IDX_2_CNS,
                                     prvTgfBurstCount,
                                     0,
                                     &prvTgfExpectedPacketInfo,
                                     prvTgfBurstCount);

    prvTgfExpectedPacketVlanInnerTag0Part.pri = 0;

    prvTgfPolicerEgressUpModifyFullManagementCountersCheck(stage,
                                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                                           expMngCounters,
                                                           callIndex++/*2*/);
}

/**
* @internal prvTgfPolicerEgressUpModifyTest function
* @endinternal
*
* @brief   Egress UP modify by policer test
*/
GT_VOID prvTgfPolicerEgressUpModifyTest
(
    GT_VOID
)
{
    /* AUTODOC: Test configurations */
    prvTgfPolicerEgressUpModifyConfigurationSet(PRV_TGF_POLICER_STAGE_EGRESS_E);

    /* AUTODOC: Test traffic and checks */
    prvTgfPolicerEgressUpModifyTestGenerateTraffic(PRV_TGF_POLICER_STAGE_EGRESS_E);

    /* AUTODOC: Restore configurations */
    prvTgfPolicerEgressUpModifyRestore(PRV_TGF_POLICER_STAGE_EGRESS_E);
}

