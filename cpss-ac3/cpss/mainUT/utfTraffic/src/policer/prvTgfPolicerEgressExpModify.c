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
* @file prvTgfPolicerEgressExpModify.c
*
* @brief Test for Exp modification modes due to Egress policer.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <policer/prvTgfPolicerEgressExpModify.h>

#include <common/tgfCommon.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfBridgeGen.h>

/* VLAN Id */
#define PRV_TGF_VLANID_CNS          5

/* MPLS labels */
#define PRV_TGF_IPV4_O_MPLS_LABEL_CNS   49

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
static TGF_PACKET_L2_STC prvTgfPacketL2TunnelPart = {
    {0x00, 0x00, 0x00, 0x00, 0x09, 0x0A},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C}                /* saMac */
};
  
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* MPLS ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsMplsEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

/* MPLS part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMplsPart = {
    PRV_TGF_IPV4_O_MPLS_LABEL_CNS,      /* label */
    1,                                  /* exp */
    1,                                  /* stack */
    0xFF                                /* timeToLive */
};

/* MPLS part */
static TGF_PACKET_MPLS_STC prvTgfExpectedPacketMplsMplsPart = {
    PRV_TGF_IPV4_O_MPLS_LABEL_CNS,      /* label */
    1,                                  /* exp */
    1,                                  /* stack */
    0xFF                                /* timeToLive */
};

/* IPv4 passenfer part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4PassengerPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0   ,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
    { 17,  34,  51,  68},   /* srcAddr */
    { 85, 102, 119, 136}    /* dstAddr */
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

/* PARTS of IPv4 over MPLS packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv4OverMplsPartArray[] = {
    {TGF_PACKET_PART_L2_E,          &prvTgfPacketL2TunnelPart},/*type, partPtr*/
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,   &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,        &prvTgfPacketMplsMplsPart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfPacketIpv4PassengerPart},
    {TGF_PACKET_PART_PAYLOAD_E,     &prvTgfPacketPayloadPart}
};

/* PARTS of IPv4 over MPLS packet */
static TGF_PACKET_PART_STC prvTgfExpectedPacketIpv4OverMplsPartArray[] = {
    {TGF_PACKET_PART_L2_E,          &prvTgfPacketL2TunnelPart},/*type, partPtr*/
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,   &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,        &prvTgfExpectedPacketMplsMplsPart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfPacketIpv4PassengerPart},
    {TGF_PACKET_PART_PAYLOAD_E,     &prvTgfPacketPayloadPart}
};

/* Length of IPv4 packet */
#define PRV_TGF_MPLS_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_MPLS_HEADER_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS \
    + sizeof(prvTgfPayloadDataArr)

/* Length of MPLS packet with CRC */
#define PRV_TGF_MPLS_PACKET_CRC_LEN_CNS  PRV_TGF_MPLS_PACKET_LEN_CNS \
        + TGF_CRC_LEN_CNS
/* PACKET to send info */
static TGF_PACKET_STC prvTgfExpectedPacketInfo = {
    PRV_TGF_MPLS_PACKET_LEN_CNS,                              /* totalLen */
    sizeof(prvTgfExpectedPacketIpv4OverMplsPartArray) / 
    sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfExpectedPacketIpv4OverMplsPartArray                   /* partsArray */
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
* @internal prvTgfPolicerEgressExpModifyTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPolicerEgressExpModifyTestVlanInit
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

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, prvTgfPortsArray[portIter]);
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = 
                                               PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;

        /* reset counters and set force link up */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

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
* @internal prvTgfPolicerEgressExpModifyTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] sendPortIndex            - sending port indexing place in test array ports.
* @param[in] recPortIndex             - receiving port indexing place in test array ports.
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPolicerEgressExpModifyTestPacketSend
(
    IN GT_U32           sendPortIndex,
    IN GT_U32           recPortIndex,
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      portIter;
    CPSS_INTERFACE_INFO_STC portInterface;
   
    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, 
                                                                          NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[recPortIndex];

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, 
                                         TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, 
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                           portInterface.devPort.hwDevNum, 
                           portInterface.devPort.portNum, GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, 
                                   prvTgfPortsArray[sendPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);

    cpssOsTimerWkAfter(10);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, 
                                        TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, 
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                            portInterface.devPort.hwDevNum, 
                            portInterface.devPort.portNum, GT_FALSE);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPolicerEgressExpModifyTestCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] sendPortIndex            - sending port index
*                                      recPortindx     - receiving port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPolicerEgressExpModifyTestCheckCounters
(
    IN GT_U32   sendPortIndex,
    IN GT_U32   recPortIndex,
    IN GT_U32   expectedValue,
    IN GT_U32   callIdentifier
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32  actualCapturedNumOfPackets;

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[recPortIndex];

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], 
                                       GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, 
                                     "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if (sendPortIndex == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL4_STRING_MAC(prvTgfBurstCount, 
                                         portCntrs.goodPktsRcv.l[0],
          "Rx another goodPktsRcv counter %d, than expected %d, on port %d, %d",
                                         portCntrs.goodPktsRcv.l[0], 
                                         prvTgfBurstCount,
                                         prvTgfPortsArray[portIter], 
                                         callIdentifier);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(expectedValue, 
                                         portCntrs.goodPktsSent.l[0],
         "Tx another goodPktsSent counter %d, than expected %d, on port %d, %d",
                                         portCntrs.goodPktsSent.l[0], 
                                         expectedValue,
                                         prvTgfPortsArray[portIter], 
                                         callIdentifier);

            if(recPortIndex == portIter)
            {
                /* print captured packets and check TriggerCounters */
                rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                        &portInterface,
                        &prvTgfExpectedPacketInfo,
                        prvTgfBurstCount,/*numOfPackets*/
                        0/*vfdNum*/,
                        NULL /*vfdArray*/,
                        NULL, /* bytesNum's skip list */
                        0,    /* length of skip list */
                        &actualCapturedNumOfPackets,
                        NULL/*onFirstPacketNumTriggersBmpPtr*/);

                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                            "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                            "port = %d, rc = 0x%02X, %d\n", 
                            portInterface.devPort.portNum, rc, callIdentifier);
            }
        }
    }
}

/**
* @internal prvTgfPolicerEgressExpModifyInitEntrySet function
* @endinternal
*
* @brief   Initial metering entry setting
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerEgressExpModifyInitEntrySet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_ENTRY_STC   meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;

    /* Save Policer entry, configure counter set 1 */
    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                stage,
                                prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                &saveMeterEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfPolicerEntryGet: %d", prvTgfDevNum);

    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.policerEnable = GT_TRUE;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET1_E;

    /* AUTODOC: Metering according to L2 */
    meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

    meterEntry.modifyExp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyTc = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.remarkMode = PRV_TGF_POLICER_REMARK_MODE_L3_E;

    /* AUTODOC: Setting the initial metering entry */
    rc = prvTgfPolicerEntrySet(stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);
}

/**
* @internal prvTgfPolicerEgressExpModifyConfigurationSet function
* @endinternal
*
* @brief   Test configurations
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerEgressExpModifyConfigurationSet
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
    prvTgfPolicerEgressExpModifyTestVlanInit(PRV_TGF_VLANID_CNS);

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
    rc = prvTgfPolicerPortMeteringEnableGet(prvTgfDevNum, stage, 
                                       prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                       &savePortMeterEnable);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                           "prvTgfPolicerPortMeteringEnableGet: %d, %d, %d, %d",
                                 prvTgfDevNum, 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                                 savePortMeterEnable);

    /* AUTODOC: Enable port for metering */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, stage, 
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

    /* AUTODOC: Initial metering entry setting */
    prvTgfPolicerEgressExpModifyInitEntrySet(stage);

    /* AUTODOC: Configure Qos Remarking entry for Exp Remarking and CL is RED */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));
    qosParam.exp = 0;
    qosParam.dp   = CPSS_DP_RED_E;

    rc = prvTgfPolicerEgressQosRemarkingEntrySet(
                                        PRV_TGF_POLICER_REMARK_TABLE_TYPE_EXP_E,
                                        1, CPSS_DP_RED_E, &qosParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                   "prvTgfPolicerEgressQosRemarkingEntrySet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerEgressExpModifyRestore function
* @endinternal
*
* @brief   Test restore configurations
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerEgressExpModifyRestore
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

    /* AUTODOC: Clear Qos Remarking entry for UP Remarking and CL is RED */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));

    rc = prvTgfPolicerEgressQosRemarkingEntrySet(
                                      PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                      0, CPSS_DP_RED_E, &qosParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                   "prvTgfPolicerEgressQosRemarkingEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: Restore stage metering enable */
    rc = prvTgfPolicerMeteringEnableSet(stage, 
                                        saveStageMeterEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage, 
                                 saveStageMeterEnable);

    /* AUTODOC: Restore port metering enable */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, stage, 
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
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", 
                                 prvTgfDevNum, GT_TRUE);

    /* Invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: Disable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_FALSE);
}

/**
* @internal prvTgfPolicerEgressExpModifyTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPolicerEgressExpModifyTestGenerateTraffic
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
    partsCount = sizeof(prvTgfPacketIpv4OverMplsPartArray) / 
                 sizeof(prvTgfPacketIpv4OverMplsPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketIpv4OverMplsPartArray, partsCount, 
                                                                   &packetSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "IPv4oIPv4, prvTgfPacketSizeGet: %d, %d",
                                 partsCount, packetSize);

    /* AUTODOC: build packet info */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketIpv4OverMplsPartArray;

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
    prvTgfPolicerManagementCountersReset(stage,
                                    PRV_TGF_POLICER_MNG_CNTR_SET1_E);

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

    /* AUTODOC: No DSCP modify */
    meterEntry.modifyExp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    /*meterEntry.modifyDscp = PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_E;*/
    /*meterEntry.modifyDscp = PRV_TGF_POLICER_MODIFY_DSCP_ENABLE_INNER_E;*/
    
    rc = prvTgfPolicerEntrySet(stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Send packet - RED */
    prvTgfPolicerEgressExpModifyTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                PRV_TGF_PORT_IDX_2_CNS,
                                                &packetInfo);

    /* AUTODOC: Verify packet flooding */
    prvTgfPolicerEgressExpModifyTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                   PRV_TGF_PORT_IDX_2_CNS,
                                                   prvTgfBurstCount, 0);

    /* AUTODOC: Expected managenet counters */
    cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][0] = l1PacketSize;
    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][1] = 0;
    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][2] = 1;

    prvTgfPolicerFullManagementCountersCheck(stage,
                                                PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                                expMngCounters,
                                                callIndex++/*0*/);

    /* Phase 1 */
    /* AUTODOC: Clear management counters */
    prvTgfPolicerManagementCountersReset(stage,
                                    PRV_TGF_POLICER_MNG_CNTR_SET1_E);

    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                               stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Exp modify */

    meterEntry.modifyExp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    prvTgfExpectedPacketMplsMplsPart.exp = 0;

    rc = prvTgfPolicerEntrySet(stage, 
                               prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d", 
                                 stage, 
                                 prvTgfPortsArray[PRV_TGF_PORT_IDX_2_CNS]);

    /* AUTODOC: Send packet - RED */
    prvTgfPolicerEgressExpModifyTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                PRV_TGF_PORT_IDX_2_CNS,
                                                &packetInfo);

    /* AUTODOC: Verify packet flooding */
    prvTgfPolicerEgressExpModifyTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                   PRV_TGF_PORT_IDX_2_CNS,
                                                   prvTgfBurstCount, 0);

    prvTgfExpectedPacketMplsMplsPart.exp = 1;

    prvTgfPolicerFullManagementCountersCheck(stage,
                                                PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                                expMngCounters,
                                                callIndex++/*1*/);
}

/**
* @internal prvTgfPolicerEgressExpModifyTest function
* @endinternal
*
* @brief   Egress Exp modify by policer test
*/
GT_VOID prvTgfPolicerEgressExpModifyTest
(
    GT_VOID
)
{
    /* AUTODOC: Test configurations */
    prvTgfPolicerEgressExpModifyConfigurationSet(
                                                PRV_TGF_POLICER_STAGE_EGRESS_E);

    /* AUTODOC: Test traffic and checks */
    prvTgfPolicerEgressExpModifyTestGenerateTraffic(
                                                PRV_TGF_POLICER_STAGE_EGRESS_E);

    /* AUTODOC: Restore configurations */
    prvTgfPolicerEgressExpModifyRestore(PRV_TGF_POLICER_STAGE_EGRESS_E);
}

