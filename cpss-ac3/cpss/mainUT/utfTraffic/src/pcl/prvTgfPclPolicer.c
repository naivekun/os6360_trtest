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
* @file prvTgfPclPolicer.c
*
* @brief Specific PCL features testing
*
* @version   27
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
#include <common/tgfCosGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclPolicer.h>

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* packet's User Priority */
#define PRV_TGF_UP_PACKET_CNS         0

/* port's default User Priority */
#define PRV_TGF_UP_DEFAULT_CNS        7

/* yellow's User Priority */
#define PRV_TGF_UP_YELLOW_CNS         1

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port number to forward traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS      1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

/* mettering entry index  */
#define PRV_TGF_METTERING_ENTRY_INDEX_CNS  7

/* billing counters index  */
#define PRV_TGF_BILLING_COUNTERS_INDEX_CNS  3

/* billing flow Id based counting index base  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS  10

/* billing flow Id minimal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS  20

/* billing flow Id maximal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS  50

/* packet flow Id to set by PCL Action  */
#define PRV_TGF_PCL_ACTION_FLOW_ID_CNS  25

/* billing flow Id based counting index  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS   \
    (PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS \
    + PRV_TGF_PCL_ACTION_FLOW_ID_CNS                 \
    - PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS)

/* billing counters indexes for hierachical counting */
#define PRV_TGF_BILLING_COUNTERS_INDEX_0_0_CNS  1
#define PRV_TGF_BILLING_COUNTERS_INDEX_0_1_CNS  2
#define PRV_TGF_BILLING_COUNTERS_INDEX_1_CNS    3


#define EPCL_RULE_INDEX_CNS     prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(1)
#define IPCL_RULE_INDEX_CNS     prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0,1)


/****************** packet 1 (IPv4 TCP matched packet) ********************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* VLAN_TAG part of packet1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket1VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    PRV_TGF_UP_PACKET_CNS, 0, PRV_TGF_VLANID_CNS         /* pri, cfi, VlanId */
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    6,                  /* protocol */
    0x4C9B,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacket1TcpPart =
{
    1,                  /* src port */
    2,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    0,                  /* data offset */
    0,                  /* reserved */
    0x00,               /* flags */
    4096,               /* window */
    0xC1A9,             /* csum */
    0                   /* urgent pointer */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacket1TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* Policy Counters */
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntrGreen;
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntrRed;
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntrYellow;
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntrDrop;

/* Billing Counters */
static PRV_TGF_POLICER_BILLING_ENTRY_STC prvTgfBillingCntr;

/* Drop Precedence for each of packets in the test */
static GT_U32 prvTgfPacketDropPrecedence[8] = {0, 0, 1, 1, 1, 2 ,2, 2};

/* expected number of sent packets on PRV_TGF_FDB_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountRxFdb = 5;

/* expected number of transmitted packets on PRV_TGF_FDB_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountTxFdb = 5;

/* expected number of sent packets on PRV_TGF_RECEIVE_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountRx = 0;

/* expected number of transmitted packets on PRV_TGF_RECEIVE_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountTx = 0;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfPclPolicerQosUpSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPolicerQosUpSet
(
    IN GT_U32   portNum,
    IN GT_U8   qosProfileIdxPort,
    IN GT_BOOL restore
)
{
    GT_STATUS               rc = GT_OK;
    GT_U8                   qosProfileId;
    PRV_TGF_COS_PROFILE_STC cosProfile;
    CPSS_QOS_ENTRY_STC      portQosCfg;
    GT_BOOL                 useQosProfile = GT_TRUE;

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        useQosProfile = GT_FALSE;
    }

    /* set CoS profile entries with different UP and DSCP */
    for (qosProfileId = 0; (qosProfileId < 8); qosProfileId++)
    {
        /* define QoS Profile */
        cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));

        if (restore == GT_FALSE)
        {
            /* dropPrecedence and UP the same */
            cosProfile.dropPrecedence =
                (CPSS_DP_LEVEL_ENT)prvTgfPacketDropPrecedence[qosProfileId];
            cosProfile.userPriority   =
                (GT_U8)prvTgfPacketDropPrecedence[qosProfileId];
            cosProfile.trafficClass   = qosProfileId;
            cosProfile.dscp           = qosProfileId;
            cosProfile.exp            = qosProfileId;
        }
        if (useQosProfile == GT_TRUE)
        {
            rc = prvTgfCosProfileEntrySet(qosProfileId, &cosProfile);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosProfileEntrySet 1");
        }
        else
        {
            rc = prvTgfCosUpToQosParamMapSet(qosProfileId, &cosProfile);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosUpToQosParamMapSet 1");
        }
    }

    /* set port's default QoS Profile for not tagged packets or NO_TRUST mode */
    portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    if (restore == GT_FALSE)
    {
        portQosCfg.qosProfileId     = qosProfileIdxPort;
        portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    }
    else
    {
        /* set default configuration */
        portQosCfg.qosProfileId     = 0;
        portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    }

    rc = prvTgfCosPortQosConfigSet(portNum, &portQosCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosConfigSet");

    /* AUTODOC: for port 0 set Qos Trust Mode as TRUST_L2 */
    rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_TRUST_L2_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosTrustModeSet");

    if (useQosProfile == GT_TRUE)
    {
        for (qosProfileId = 0; (qosProfileId < 8); qosProfileId++)
        {
            if (restore == GT_FALSE)
            {
                /* set QoS Map for tagged packets with specified User Priority field */
                rc = prvTgfCosUpCfiDeiToProfileMapSet(0/*mappingTableIndex*/,PRV_TGF_COS_UP_PROFILE_INDEX_DEFAULT_CNS,
                    qosProfileId /*up*/, 0 /*cfiDeiBit*/, qosProfileId);
            }
            else
            {
                /* reset QoS Map for tagged packets with specified User Priority field */
                rc = prvTgfCosUpCfiDeiToProfileMapSet(0/*mappingTableIndex*/,PRV_TGF_COS_UP_PROFILE_INDEX_DEFAULT_CNS,
                    qosProfileId /*up*/, 0 /*cfiDeiBit*/, 0);
            }
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosUpCfiDeiToProfileMapSet");
        }
    }

    return rc;
}


/**
* @internal prvTgfDefConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfDefConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {0, 1, 0, 0};

    /* AUTODOC: create VLAN 5 with untagged ports [0,2,3], tagged port 1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    return rc;
};

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
    GT_BOOL                 usePortMode = GT_TRUE;

    /* AUTODOC: RESTORE CONFIGURATION: */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        usePortMode = GT_FALSE;
    }

    /* AUTODOC: restore policer counting mode */
    rc = prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_POLICER_COUNTING_DISABLE_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerCountingModeSet");

    if (usePortMode == GT_TRUE)
    {
        /* AUTODOC: restore policer Global stage mode */
        rc = prvTgfPolicerStageMeterModeSet(
            PRV_TGF_POLICER_STAGE_INGRESS_0_E,
            PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerStageMeterModeSet");
    }

    /* set the conformance level as GREEN */
    prvTgfPolicerConformanceLevelForce(0);

    /* AUTODOC: disable metering */
    rc = prvTgfPolicerMeteringEnableSet(policerStage, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerMeteringEnableSet");

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* AUTODOC: invalidate PCL rule 1 */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_EXT_E, prvTgfPclRuleIndex, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d");

    /* reset QoS configuration */
    /* AUTODOC: reset QoS profile entries [0..7] */
    /* AUTODOC: restore default QoS Profile for port 0 */
    rc = prvTgfPclPolicerQosUpSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_UP_DEFAULT_CNS, /*qosProfileIdxPort*/
            GT_TRUE);              /* reset configuration */
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPolicerQosUpSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfPclPolicerTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPolicerTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32                   portNumReceive;
    GT_U32                  sendIter;
    GT_U32                  dp;
    TGF_VFD_INFO_STC        vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG3_MAC("sending [%d] packets to port [%d] "
            "and FORWARDING to FDB port [%d]\n\n", 5,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS]);

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portNumReceive                = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNumReceive;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));
    vfdArray[0].mode   = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].cycleCount = 1;
    vfdArray[0].offset = 14; /* DA==6, SA==6, "0x8100"==2*/
    vfdArray[1].mode   = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].cycleCount = 1;
    vfdArray[1].offset = prvTgfPacket1Info.totalLen - 1;

    /* sending packets - matched and unmatched */
    for (sendIter = 0; (sendIter < 5); sendIter++)
    {

        /* set the conformance level as GREEN, RED or YELLOW */
        /* Simulation only                                   */
        dp = prvTgfPacketDropPrecedence[sendIter];
        prvTgfPolicerConformanceLevelForce(dp);

        /* set UP field of the packet */
        vfdArray[0].patternPtr[0] = (GT_U8)((sendIter << 5) & 0xE0);
        /* additional stamp */
        vfdArray[1].patternPtr[0] = (GT_U8)(sendIter & 0xFF);

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, &prvTgfPacket1Info,
            1 /*burstCount*/, 2 /*numVfd*/, vfdArray);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

        /* AUTODOC: send 5 TCP packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
        /* AUTODOC:   VID=5, UPs=[0x00,0x20,0x40,0x60,0x80] */
        /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
        /* AUTODOC:   srcport=1, dstPort=2 */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
    }

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* wait for 2 seconds in the GM */
        cpssOsTimerWkAfter(2000);
    }

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    /* print captured packets from receive port */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNumReceive);
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, portNumReceive);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPortCapturedPacketPrint");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
    return rc;
};

/**
* @internal prvTgfPclPolicerPclSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPolicerPclSet
(
    IN PRV_TGF_PCL_POLICER_ENABLE_ENT policerEnable,
    IN GT_U32                         policerId
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT patt;
    PRV_TGF_PCL_ACTION_STC      action;

    /* clear mask, pattern and action */
    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E /*ipv6Key*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclDefPortInit");

    /* set action */
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.policer.policerEnable = policerEnable;
    action.policer.policerId     = policerId;

    /* always set this flow id needed only for flow id based counting test*/
    action.flowId = PRV_TGF_PCL_ACTION_FLOW_ID_CNS;

    prvTgfPclRuleIndex = IPCL_RULE_INDEX_CNS;
    /* AUTODOC: set PCL rule 1 for all packets: */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd - FORWARD */
    /* AUTODOC:   POLICER_ENABLE_COUNTER_ONLY, policerId=3 <- for counter test */
    /* AUTODOC:   POLICER_ENABLE_METER_AND_COUNTER, policerId=7 <- for metering test */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        prvTgfPclRuleIndex, &mask, &patt, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

    return rc;
};

/**
* @internal prvTgfPclPolicerPclActionUpdate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPolicerPclActionUpdate
(
    IN PRV_TGF_PCL_POLICER_ENABLE_ENT policerEnable,
    IN GT_U32                         policerId
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_ACTION_STC      action;

    /* clear action */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* set action */
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.policer.policerEnable = policerEnable;
    action.policer.policerId     = policerId;

    /* always set this flow id needed only for flow id based counting test*/
    action.flowId = PRV_TGF_PCL_ACTION_FLOW_ID_CNS;

    /* AUTODOC: update PCL action of rule 1 for all packets: */
    /* AUTODOC:   cmd - FORWARD */
    /* AUTODOC:   policerEnable */
    /* AUTODOC:   policerId     */
    rc = prvTgfPclRuleActionUpdate(
        CPSS_PCL_RULE_SIZE_EXT_E,
        prvTgfPclRuleIndex, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleActionUpdate");

    return rc;
};

/**
* @internal prvTgfPclPolicerSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclPolicerSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_ENTRY_STC           metteringEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_U32                              qosProfileIndex = 0;
    GT_U8                               greenQosTableRemarkIndex;
    GT_U8                               yellowQosTableRemarkIndex;
    GT_U8                               redQosTableRemarkIndex;
    PRV_TGF_POLICER_COUNTING_MODE_ENT   countingMode;
    GT_BOOL                 useQosProfile = GT_TRUE;

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        useQosProfile = GT_FALSE;
        rc = prvTgfPolicerInit();
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPolicerInit: %d", policerStage);
    }

    countingMode = PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E;

    /* AUTODOC: config policer counting mode: */
    /* AUTODOC:   ingress Policer stage #0, billing or IPFIX counting */
    rc = prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        countingMode);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
        prvTgfDevNum,
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        countingMode);

    /* AUTODOC: config policer global stage mode: */
    /* AUTODOC:   ingress Policer stage #0, Flow-based metering */
    rc = prvTgfPolicerStageMeterModeSet(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);

    if (useQosProfile == GT_TRUE)
    {
        /* set QoS Remarking Entry */
        qosProfileIndex           = PRV_TGF_UP_YELLOW_CNS;
        greenQosTableRemarkIndex  = 2;
        yellowQosTableRemarkIndex = 2;
        redQosTableRemarkIndex    = 3;

        rc = prvTgfPolicerQosRemarkingEntrySet(
            policerStage, qosProfileIndex,
            greenQosTableRemarkIndex, yellowQosTableRemarkIndex, redQosTableRemarkIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerQosRemarkingEntrySet: %d", prvTgfDevNum);
    }

    /* AUTODOC: enable metering for ingress Policer stage #0 */
    rc = prvTgfPolicerMeteringEnableSet(policerStage, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerMeteringEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set metered Packet Size Mode as POLICER_PACKET_SIZE_L2_INCLUDE */
    rc = prvTgfPolicerPacketSizeModeSet(policerStage,
            CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerPacketSizeModeSet: %d", prvTgfDevNum);

    /* AUTODOC: create metering entry per SEND Port */
    /* all structure members not relevant to DXCH3 omitted */
    cpssOsMemSet(&metteringEntry, 0, sizeof(metteringEntry));
    metteringEntry.meterColorMode     = CPSS_POLICER_COLOR_AWARE_E;
    metteringEntry.qosProfile         = qosProfileIndex;
    metteringEntry.remarkMode         = PRV_TGF_POLICER_REMARK_MODE_L2_E;
    metteringEntry.meterMode          = PRV_TGF_POLICER_METER_MODE_SR_TCM_E;
    metteringEntry.mngCounterSet      = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    metteringEntry.tbParams.srTcmParams.cir = 0x00FFFFFF;
    metteringEntry.tbParams.srTcmParams.cbs = 0x00FFFFFF;
    metteringEntry.tbParams.srTcmParams.ebs = 0x00FFFFFF;
    metteringEntry.countingEntryIndex = PRV_TGF_BILLING_COUNTERS_INDEX_CNS;
    metteringEntry.modifyUp           = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    metteringEntry.modifyDscp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    metteringEntry.modifyDp           = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    metteringEntry.yellowPcktCmd      = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E ;
    metteringEntry.redPcktCmd         = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;

    rc = prvTgfPolicerEntrySet(
        policerStage,
        PRV_TGF_METTERING_ENTRY_INDEX_CNS,
        &metteringEntry, &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc,
            "prvTgfPolicerEntrySet: %d %d",
            prvTgfDevNum, PRV_TGF_METTERING_ENTRY_INDEX_CNS);

};
/**
* @internal prvTgfPolicerBillingCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Billing Counters
*/
static GT_VOID prvTgfPolicerBillingCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 billingEntryIndex,
    IN GT_U32 greenCntr,
    IN GT_U32 yellowCntr,
    IN GT_U32 redCntr
)
{
    GT_STATUS rc = GT_OK;

    /* get Policer Billing Counters */
    rc = prvTgfPolicerBillingEntryGet(
        prvTgfDevNum, policerStage, billingEntryIndex,
        GT_FALSE, &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerBillingEntryGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL3_STRING_MAC(
        greenCntr, prvTgfBillingCntr.greenCntr.l[0],
        "Stage[%d], Index[%d]: BillingCntr_Green = %d", policerStage, billingEntryIndex, prvTgfBillingCntr.greenCntr.l[0]);

    UTF_VERIFY_EQUAL3_STRING_MAC(
        yellowCntr, prvTgfBillingCntr.yellowCntr.l[0],
        "Stage[%d], Index[%d]: BillingCntr_Yellow = %d", policerStage, billingEntryIndex, prvTgfBillingCntr.yellowCntr.l[0]);

    UTF_VERIFY_EQUAL3_STRING_MAC(
        redCntr, prvTgfBillingCntr.redCntr.l[0],
        "Stage[%d], Index[%d]: BillingCntr_Red = %d", policerStage, billingEntryIndex, prvTgfBillingCntr.redCntr.l[0]);
}

/**
* @internal prvTgfPolicerManagementCountersGreenCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters Green
*/
static GT_VOID prvTgfPolicerManagementCountersGreenCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS rc = GT_OK;

    /* get Policer Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                                            &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
        duLowCntr, prvTgfMngCntrGreen.duMngCntr.l[0],
        "MenagementCntr_Green = %d",
        prvTgfMngCntrGreen.duMngCntr.l[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        duHiCntr, prvTgfMngCntrGreen.duMngCntr.l[1],
        "MenagementCntr_Green_1 = %d",
        prvTgfMngCntrGreen.duMngCntr.l[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        packetMngCntr, prvTgfMngCntrGreen.packetMngCntr,
        "MenagementCntr_Green_packet = %d",
        prvTgfMngCntrGreen.packetMngCntr);
}

/**
* @internal prvTgfPolicerManagementCountersYellowCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters Yellow
*/
static GT_VOID prvTgfPolicerManagementCountersYellowCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS                        rc = GT_OK;

    /* get Policer Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,
                                            &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
        duLowCntr, prvTgfMngCntrYellow.duMngCntr.l[0],
        "MenagementCntr_Yellow = %d",
        prvTgfMngCntrYellow.duMngCntr.l[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        duHiCntr, prvTgfMngCntrYellow.duMngCntr.l[1],
        "MenagementCntr_Yellow_1 = %d",
        prvTgfMngCntrYellow.duMngCntr.l[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        packetMngCntr, prvTgfMngCntrYellow.packetMngCntr,
        "MenagementCntr_Yellow_packet = %d",
        prvTgfMngCntrYellow.packetMngCntr);
}

/**
* @internal prvTgfPolicerManagementCountersRedCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters Red
*/
static GT_VOID prvTgfPolicerManagementCountersRedCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS                        rc = GT_OK;

    /* get Policer Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            PRV_TGF_POLICER_MNG_CNTR_RED_E,
                                            &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
        duLowCntr, prvTgfMngCntrRed.duMngCntr.l[0],
        "MenagementCntr_Red = %d",
        prvTgfMngCntrRed.duMngCntr.l[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        duHiCntr, prvTgfMngCntrRed.duMngCntr.l[1],
        "MenagementCntr_Red_1 = %d",
        prvTgfMngCntrRed.duMngCntr.l[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        packetMngCntr, prvTgfMngCntrRed.packetMngCntr,
        "MenagementCntr_Red_packet = %d",
        prvTgfMngCntrRed.packetMngCntr);
}

/**
* @internal prvTgfPolicerManagementCountersDropCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters Drop
*/
static GT_VOID prvTgfPolicerManagementCountersDropCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS                        rc = GT_OK;

    /* get Policer Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            PRV_TGF_POLICER_MNG_CNTR_DROP_E,
                                            &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
        duLowCntr, prvTgfMngCntrDrop.duMngCntr.l[0],
        "MenagementCntr_Drop = %d",
        prvTgfMngCntrDrop.duMngCntr.l[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        duHiCntr, prvTgfMngCntrDrop.duMngCntr.l[1],
        "MenagementCntr_Drop_1 = %d",
        prvTgfMngCntrDrop.duMngCntr.l[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        packetMngCntr, prvTgfMngCntrDrop.packetMngCntr,
        "MenagementCntr_Drop_packet = %d",
        prvTgfMngCntrDrop.packetMngCntr);
}

/**
* @internal prvTgfPolicerBillingCountersPrint function
* @endinternal
*
* @brief   Print the value of specified Billing Counters
*/
static GT_VOID prvTgfPolicerBillingCountersPrint
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32                         entryIndex
)
{
    GT_STATUS rc = GT_OK;

    /* get and print Policer Billing Counters */
    rc = prvTgfPolicerBillingEntryGet(
        prvTgfDevNum, policerStage, entryIndex,
        GT_FALSE, &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerBillingEntryGet: %d", prvTgfDevNum);
    PRV_UTF_LOG1_MAC("*** billingCntr_EntryIndex = %d ***\n", entryIndex);
    PRV_UTF_LOG1_MAC("billingCntr_Green  = %d\n", prvTgfBillingCntr.greenCntr.l[0]);
    PRV_UTF_LOG1_MAC("billingCntr_Yellow = %d\n", prvTgfBillingCntr.yellowCntr.l[0]);
    PRV_UTF_LOG1_MAC("billingCntr_Red    = %d\n\n", prvTgfBillingCntr.redCntr.l[0]);
}

/**
* @internal prvTgfPolicerBillingCountersReset function
* @endinternal
*
* @brief   Reset specified Billing Counters
*/
static GT_VOID prvTgfPolicerBillingCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32                         entryIndex
)
{
    GT_STATUS rc = GT_OK;

    /* clear Policer Billing Counters */
    cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
    prvTgfBillingCntr.billingCntrMode =
        PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
    prvTgfBillingCntr.packetSizeMode =
        CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

    rc = prvTgfPolicerBillingEntrySet(
        prvTgfDevNum, policerStage, entryIndex,
        &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerBillingEntrySet: %d %d %d",
        prvTgfDevNum, policerStage, entryIndex);
}

/**
* @internal prvTgfPolicerCountersPrint function
* @endinternal
*
* @brief   Print the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerCountersPrint
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    policyCntrPort0, policyCntrPort8;

    /* get and print Policer Billing Counters */
    prvTgfPolicerBillingCountersPrint(
        policerStage,
        PRV_TGF_BILLING_COUNTERS_INDEX_CNS);

    prvTgfPolicerBillingCountersPrint(
        policerStage,
        PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS);

    /* get and print Policer Management Counters GREEN */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    if (prvTgfMngCntrGreen.duMngCntr.l[0] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrGreen.duMngCntr.l[0] = %d\n", prvTgfMngCntrGreen.duMngCntr.l[0]);
    if (prvTgfMngCntrGreen.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrGreen.duMngCntr.l[1] = %d\n", prvTgfMngCntrGreen.duMngCntr.l[1]);
    if (prvTgfMngCntrGreen.packetMngCntr != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrGreen.packetMngCntr = %d\n", prvTgfMngCntrGreen.packetMngCntr);

    /* get and print Policer Management Counters YELLOW */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    if (prvTgfMngCntrYellow.duMngCntr.l[0] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrYellow.duMngCntr.l[0] = %d\n", prvTgfMngCntrYellow.duMngCntr.l[0]);
    if (prvTgfMngCntrYellow.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrYellow.duMngCntr.l[1] = %d\n", prvTgfMngCntrYellow.duMngCntr.l[1]);
    if (prvTgfMngCntrYellow.packetMngCntr != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrYellow.packetMngCntr = %d\n", prvTgfMngCntrYellow.packetMngCntr);

    /* get and print Policer Management Counters RED */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    if (prvTgfMngCntrRed.duMngCntr.l[0] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrRed.duMngCntr.l[0] = %d\n", prvTgfMngCntrRed.duMngCntr.l[0]);
    if (prvTgfMngCntrRed.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrRed.duMngCntr.l[1] = %d\n", prvTgfMngCntrRed.duMngCntr.l[1]);
    if (prvTgfMngCntrRed.packetMngCntr != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrRed.packetMngCntr = %d\n", prvTgfMngCntrRed.packetMngCntr);

    /* get and print Policer Management Counters DROP */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    if (prvTgfMngCntrDrop.duMngCntr.l[0] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrDrop.duMngCntr.l[0] = %d\n", prvTgfMngCntrDrop.duMngCntr.l[0]);
    if (prvTgfMngCntrDrop.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrDrop.duMngCntr.l[1] = %d\n", prvTgfMngCntrDrop.duMngCntr.l[1]);
    if (prvTgfMngCntrDrop.packetMngCntr != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrDrop.packetMngCntr = %d\n", prvTgfMngCntrDrop.packetMngCntr);

    /* get and print Policer Policy Counters */
    rc = prvTgfPolicerPolicyCntrGet(prvTgfDevNum, policerStage, 0, &policyCntrPort0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerPolicyCntrGet: %d %d", prvTgfDevNum, 0);
    PRV_UTF_LOG1_MAC("\npolicyCntrPort0 = %d\n", policyCntrPort0);

    rc = prvTgfPolicerPolicyCntrGet(prvTgfDevNum, policerStage, 8, &policyCntrPort8);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerPolicyCntrGet: %d %d", prvTgfDevNum, 8);
    PRV_UTF_LOG1_MAC("policyCntrPort8 = %d\n", policyCntrPort8);

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfPolicerCountersReset function
* @endinternal
*
* @brief   Resets the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS rc = GT_OK;

    cpssOsMemSet(&prvTgfMngCntrGreen, 0, sizeof(prvTgfMngCntrGreen));
    cpssOsMemSet(&prvTgfMngCntrRed, 0, sizeof(prvTgfMngCntrRed));
    cpssOsMemSet(&prvTgfMngCntrYellow, 0, sizeof(prvTgfMngCntrYellow));
    cpssOsMemSet(&prvTgfMngCntrDrop, 0, sizeof(prvTgfMngCntrDrop));

    /* AUTODOC: reset Policer Management Counters - GREEN, YELLOW, RED, DROP */
    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    /* AUTODOC: flush internal Write Back Cache of counting entries */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(policerStage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerCountingWriteBackCacheFlush FAILED, rc = [%d]", rc);
    }

    /* AUTODOC: reset Policer Billing Counters */
    cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
    prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        prvTgfBillingCntr.packetSizeMode =
            CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    }
    rc = prvTgfPolicerBillingEntrySet(
        prvTgfDevNum, policerStage,
        PRV_TGF_BILLING_COUNTERS_INDEX_CNS,
        &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);

    rc = prvTgfPolicerBillingEntrySet(
        prvTgfDevNum, policerStage,
        PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
        &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclPolicerMeteringTest function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclPolicerMeteringTest
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN PRV_TGF_PCL_POLICER_ENABLE_ENT policerEnable,
    IN GT_U32                         policerId
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    /* create vlan and ports */
    rc = prvTgfDefConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfDefConfigurationSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: disables ingress policy on FDB port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* set QoS configuration */
    /* AUTODOC: set 8 QoS profile entries [0..7] with: */
    /* AUTODOC:   UP=DP=[0,0,1,1,1,2,2,2], DSCP=TC=EXP=7 */

    /* AUTODOC: for port 0 set default QoS Profile with: */
    /* AUTODOC:   assignPrecedence=PRECEDENCE_SOFT */
    /* AUTODOC:   qosProfileId=7, enableModify UP\DSCP */
    /* AUTODOC: set QoS Map for tagged packets with specified UP fields [0..7] */
    rc = prvTgfPclPolicerQosUpSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_UP_DEFAULT_CNS, /*qosProfileIdxPort*/
            GT_FALSE);              /* set configuration */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPolicerQosUpSet");

    /* set PCL rule */
    rc = prvTgfPclPolicerPclSet(
        policerEnable, policerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRulesDefine: %d", prvTgfDevNum);

    /* set policer configuration */
    prvTgfPclPolicerSet(policerStage);

    /* -------------------------------------------------------------------------
     * 2. Generate traffic and Check counters
     */

    /* reset the value of specified Billing, Management and Policy Counters */
    prvTgfPolicerCountersReset(policerStage);

    /* generate traffic */
    rc = prvTgfPclPolicerTrafficGenerate();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPolicerTrafficGenerate: %d", prvTgfDevNum);

    /* AUTODOC: check counter of FDB port 1 - 5 Rx\Tx packets expected */
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        prvTgfPacketsCountRxFdb, prvTgfPacketsCountTxFdb,
        prvTgfPacket1Info.totalLen, 1);
    PRV_UTF_VERIFY_RC1(rc, "get another counters values.");

    /* AUTODOC: check counter on receive port 2 - no traffic */
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPacketsCountRx, prvTgfPacketsCountTx,
        prvTgfPacket1Info.totalLen, 1);
    PRV_UTF_VERIFY_RC1(rc, "get another counters values.");

    /* check return code for counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc1, "prvTgfPclPolicerTrafficGenerate: %d",
                                 prvTgfDevNum);

    /* print the value of specified Billing, Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclPolicerCounting function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerCounting
(
   GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    /* set three Yellow packets */
    prvTgfPacketDropPrecedence[0] = 0;
    prvTgfPacketDropPrecedence[1] = 0;
    prvTgfPacketDropPrecedence[2] = 1;
    prvTgfPacketDropPrecedence[3] = 1;
    prvTgfPacketDropPrecedence[4] = 2;

    /* expected number of packets */
    prvTgfPacketsCountRxFdb = 5;
    prvTgfPacketsCountTxFdb = 5;

    /* run the test */
    prvTgfPclPolicerMeteringTest(
        policerStage,
        PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_CNS);

    /* AUTODOC: verify Billing and Management Counters: */
    /* AUTODOC:   green=2, yellow=2, red=1 */
    prvTgfPolicerBillingCountersCheck(
        policerStage,
        PRV_TGF_BILLING_COUNTERS_INDEX_CNS,
        2, 2, 1);

    /* restore configuration */
    rc = prvTgfConfigurationRestore();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclPolicerMetering function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerMetering
(
   GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    packetSize;
    PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    /* set one Yellow and two RED packets */
    prvTgfPacketDropPrecedence[0] = 0;
    prvTgfPacketDropPrecedence[1] = 0;
    prvTgfPacketDropPrecedence[2] = 1;
    prvTgfPacketDropPrecedence[3] = 2;
    prvTgfPacketDropPrecedence[4] = 2;

    /* expected number of packets */
    prvTgfPacketsCountRxFdb = 3;
    prvTgfPacketsCountTxFdb = 3;

    /* set size of packets */
    packetSize = prvTgfPacket1Info.totalLen + TGF_CRC_LEN_CNS;

    /* run the test */
    prvTgfPclPolicerMeteringTest(
        policerStage,
        PRV_TGF_PCL_POLICER_ENABLE_METER_AND_COUNTER_E,
        PRV_TGF_METTERING_ENTRY_INDEX_CNS);

    /* AUTODOC: verify Billing and Management Counters: */
    /* AUTODOC:   green=2, yellow=1, red=2 */
    prvTgfPolicerBillingCountersCheck(
        policerStage,
        PRV_TGF_BILLING_COUNTERS_INDEX_CNS,
        2, 1, 2);
    prvTgfPolicerManagementCountersGreenCheck(policerStage, 2 * packetSize, 0, 2);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, packetSize, 0, 1);
    prvTgfPolicerManagementCountersRedCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 2 * packetSize, 0, 2);

    /* restore configuration */
    rc = prvTgfConfigurationRestore();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclPolicerCountingFlowIdBased function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerCountingFlowIdBased
(
   GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    PRV_TGF_POLICER_BILLING_INDEX_CFG_STC  billingIndexCfg;
    PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    billingIndexCfg.billingIndexMode       =
        PRV_TGF_POLICER_BILLING_INDEX_MODE_FLOW_ID_E;
    billingIndexCfg.billingFlowIdIndexBase =
        PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS;
    billingIndexCfg.billingMinFlowId =
        PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS;
    billingIndexCfg.billingMaxFlowId =
        PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS;

    rc = prvTgfPolicerFlowIdCountingCfgSet(
        policerStage, &billingIndexCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerFlowIdCountingCfgSet");

    /* set three Yellow packets */
    prvTgfPacketDropPrecedence[0] = 0;
    prvTgfPacketDropPrecedence[1] = 0;
    prvTgfPacketDropPrecedence[2] = 1;
    prvTgfPacketDropPrecedence[3] = 1;
    prvTgfPacketDropPrecedence[4] = 2;

    /* expected number of packets */
    prvTgfPacketsCountRxFdb = 5;
    prvTgfPacketsCountTxFdb = 5;

    /* run the test */
    prvTgfPclPolicerMeteringTest(
        policerStage,
        PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_CNS);

    /* AUTODOC: verify Billing and Management Counters: */
    /* AUTODOC:   green=2, yellow=2, red=1 */
    prvTgfPolicerBillingCountersCheck(
        policerStage,
        PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
        2, 2, 1);

    /* restore configuration */
    billingIndexCfg.billingIndexMode       =
        PRV_TGF_POLICER_BILLING_INDEX_MODE_STANDARD_E;
    billingIndexCfg.billingFlowIdIndexBase = 0;
    billingIndexCfg.billingMinFlowId = 0xFFFF;
    billingIndexCfg.billingMaxFlowId = 0xFFFF;

    rc = prvTgfPolicerFlowIdCountingCfgSet(
        policerStage, &billingIndexCfg);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerFlowIdCountingCfgSet");

    rc = prvTgfConfigurationRestore();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfPclPolicerTrafficSend function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPolicerTrafficSend
(
    IN GT_U32  packetNum,
    IN GT_BOOL resetCounters,
    IN GT_BOOL resetCapture,
    IN GT_BOOL printCapture
)
{
    GT_STATUS               rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32                   portNumReceive;

    /* AUTODOC: GENERATE TRAFFIC: */

    if (resetCounters != GT_FALSE)
    {
        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");
    }

    if (resetCapture != GT_FALSE)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");
    }

    /* setup receive portInterface for capturing */
    portNumReceive                = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNumReceive;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");


    /* sending packets - matched and unmatched */

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, &prvTgfPacket1Info,
        packetNum /*burstCount*/, 0 /*numVfd*/, NULL /*vfdArray*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: send 5 TCP packets on port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   VID=5, UPs=[0x00,0x20,0x40,0x60,0x80] */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcport=1, dstPort=2 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* wait for 2 seconds in the GM */
        cpssOsTimerWkAfter(2000);
    }

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    if (printCapture != GT_FALSE)
    {
        /* print captured packets from receive port */
        PRV_UTF_LOG1_MAC("====== Port [%d] capturing:\n", portNumReceive);
        rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, portNumReceive);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPortCapturedPacketPrint");
        PRV_UTF_LOG0_MAC("====== Port capturing end\n");
    }

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
    return rc;
};

/**
* @internal prvTgfPclPolicerCountingHierarchicalTest function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerCountingHierarchicalTest
(
   GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_HIERARCHICAL_TABLE_ENTRY_STC hierEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    /* create vlan and ports */
    rc = prvTgfDefConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfDefConfigurationSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: disables ingress policy on FDB port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* set PCL rule */
    rc = prvTgfPclPolicerPclSet(
        PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_0_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPolicerPclSet: %d", prvTgfDevNum);

    prvTgfPolicerMeteringEnableSet(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerMeteringEnableSet");

    /* AUTODOC: restore policer Global stage mode */
    rc = prvTgfPolicerStageMeterModeSet(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerStageMeterModeSet");

    cpssOsMemSet(&hierEntry, 0, sizeof(hierEntry));
    hierEntry.countingEnable = GT_TRUE;
    hierEntry.meteringEnable = GT_FALSE;
    hierEntry.policerPointer = PRV_TGF_BILLING_COUNTERS_INDEX_1_CNS;

    rc = prvTgfPolicerHierarchicalTableEntrySet(
        PRV_TGF_BILLING_COUNTERS_INDEX_0_0_CNS, &hierEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerHierarchicalTableEntrySet");

    rc = prvTgfPolicerHierarchicalTableEntrySet(
        PRV_TGF_BILLING_COUNTERS_INDEX_0_1_CNS, &hierEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerHierarchicalTableEntrySet");

    /* set policer configuration */
    rc = prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerCountingModeSet");

    rc = prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        PRV_TGF_POLICER_STAGE_INGRESS_1_E,
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerCountingModeSet");

    /* reset specified Billing Counters */
    prvTgfPolicerBillingCountersReset(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_0_CNS);

    prvTgfPolicerBillingCountersReset(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_1_CNS);

    prvTgfPolicerBillingCountersReset(
        PRV_TGF_POLICER_STAGE_INGRESS_1_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_1_CNS);

    /* -------------------------------------------------------------------------
     * 2. Generate traffic and Check counters
     */

    /* generate traffic */
    rc = prvTgfPclPolicerTrafficSend(2, GT_TRUE, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPolicerTrafficSend: %d", prvTgfDevNum);

    prvTgfPclPolicerPclActionUpdate(
        PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_1_CNS);

    /* generate traffic */
    rc = prvTgfPclPolicerTrafficSend(1, GT_FALSE, GT_FALSE, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPolicerTrafficSend: %d", prvTgfDevNum);

    /* AUTODOC: check counter of FDB port 1 - 3 Rx\Tx packets expected (captured)*/
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        3 /*prvTgfPacketsCountRxFdb*/, 3 /*prvTgfPacketsCountTxFdb*/,
        prvTgfPacket1Info.totalLen, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersCheck: %d", prvTgfDevNum);

    /* print counters */
    prvTgfPolicerBillingCountersPrint(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_0_CNS);

    prvTgfPolicerBillingCountersPrint(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_1_CNS);

    prvTgfPolicerBillingCountersPrint(
        PRV_TGF_POLICER_STAGE_INGRESS_1_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_1_CNS);

    /* AUTODOC: verify Billing Counters: */

    /* AUTODOC:  STAGE0, Entry0 green=2, yellow=0, red=0 */
    prvTgfPolicerBillingCountersCheck(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_0_CNS,
        2, 0, 0);

    /* AUTODOC:  STAGE0, Entry1 green=1, yellow=0, red=0 */
    prvTgfPolicerBillingCountersCheck(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_1_CNS,
        1, 0, 0);

    /* AUTODOC:  STAGE1, green=3, yellow=0, red=0 */
    prvTgfPolicerBillingCountersCheck(
        PRV_TGF_POLICER_STAGE_INGRESS_1_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_1_CNS,
        3, 0, 0);
}

/**
* @internal prvTgfPclPolicerCountingHierarchicalReset function
* @endinternal
*
* @brief   Reset test configuration
*/
GT_VOID prvTgfPclPolicerCountingHierarchicalReset
(
   GT_VOID
)
{
    PRV_TGF_POLICER_HIERARCHICAL_TABLE_ENTRY_STC hierEntry;

    cpssOsMemSet(&hierEntry, 0, sizeof(hierEntry));

    prvTgfPolicerHierarchicalTableEntrySet(
        PRV_TGF_BILLING_COUNTERS_INDEX_0_0_CNS, &hierEntry);

    prvTgfPolicerHierarchicalTableEntrySet(
        PRV_TGF_BILLING_COUNTERS_INDEX_0_1_CNS, &hierEntry);

    prvTgfPolicerBillingCountersReset(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_0_CNS);

    prvTgfPolicerBillingCountersReset(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_0_1_CNS);

    prvTgfPolicerBillingCountersReset(
        PRV_TGF_POLICER_STAGE_INGRESS_1_E,
        PRV_TGF_BILLING_COUNTERS_INDEX_1_CNS);

    prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_POLICER_COUNTING_DISABLE_E);

    prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        PRV_TGF_POLICER_STAGE_INGRESS_1_E,
        PRV_TGF_POLICER_COUNTING_DISABLE_E);

    prvTgfConfigurationRestore();
}

/* EGRESS Policer test */

/**
* @internal prvTgfPolicerCfiDpToCfiMappingSet function
* @endinternal
*
* @brief   Set DP to CFI mapping configuration
*/
static GT_VOID prvTgfPolicerCfiDpToCfiMappingSet
(
     IN GT_BOOL           enable,
     IN GT_U32             port,
     IN GT_U32            greenCfi,
     IN GT_U32            yellowCfi,
     IN GT_U32            redCfi
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: DP to CFI CONFIGURATION: */
     /* AUTODOC: enable DP to CFI mapping: */
    PRV_UTF_LOG0_MAC("======= enable DP to CFI mapping =======\n");
    rc = prvTgfCosPortDpToCfiDeiMapEnableSet(prvTgfDevNum, port, enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortDpToCfiDeiMapEnableSet");

    /* AUTODOC: set DP to CFI mapping: */
    PRV_UTF_LOG0_MAC("======= set DP to CFI mapping =======\n");

    rc = prvTgfCosDpToCfiDeiMapSet(CPSS_DP_GREEN_E, (GT_U8)greenCfi);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet");

    rc = prvTgfCosDpToCfiDeiMapSet(CPSS_DP_YELLOW_E, (GT_U8)yellowCfi);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet");

    rc = prvTgfCosDpToCfiDeiMapSet(CPSS_DP_RED_E, (GT_U8)redCfi);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet");
}

/**
* @internal prvTgfPclPolicerEgressPclSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPolicerEgressPclSet
(
    IN GT_U32                          port,
    IN PRV_TGF_PCL_POLICER_ENABLE_ENT policerEnable,
    IN GT_U32                         policerId
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT patt;
    PRV_TGF_PCL_ACTION_STC      action;

    /* clear mask, pattern and action */
    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    rc = prvTgfPclDefPortInit(
        port,
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E /*ipv6Key*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclDefPortInit");

    /* set action */
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy = GT_TRUE;
    action.policer.policerEnable = policerEnable;
    action.policer.policerId     = policerId;

    prvTgfPclRuleIndex = EPCL_RULE_INDEX_CNS;

    /* AUTODOC: set PCL rule 1 for all packets: */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd - FORWARD */
    /* AUTODOC:   POLICER_ENABLE_COUNTER_ONLY, policerId=3 <- for counter test */
    /* AUTODOC:   POLICER_ENABLE_METER_AND_COUNTER, policerId=7 <- for metering test */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
        prvTgfPclRuleIndex, &mask, &patt, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

    return rc;
};

/**
* @internal prvTgfPclPolicerEgressUpCfiVidCheck function
* @endinternal
*
* @brief   Checks traffic egress UP, CFI and VID in the Tag
*
* @param[in] egressUp                 - UP expected in egressed packets VLAN Tag
* @param[in] egressCfi                - CFI expected in egressed packets VLAN Tag
* @param[in] egressVid                - VID expected in egressed packets VLAN Tag
*                                       None
*/
static GT_VOID prvTgfPclPolicerEgressUpCfiVidCheck
(
    IN GT_U32  egressUp,
    IN GT_U32  egressCfi,
    IN GT_U32  egressVid
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = 14; /* 6 + 6 + 2 */
    vfdArray[0].cycleCount = 2;
    vfdArray[0].patternPtr[0] =
        (GT_U8)((egressUp << 5) | (egressCfi << 4) | (egressVid >> 8));
    vfdArray[0].patternPtr[1] = (GT_U8)(egressVid & 0xFF);

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
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfPclPolicerDp2CfiMapTest function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPolicerDp2CfiMapTest
(
   GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_POLICER_ENTRY_STC           meteringEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    PRV_TGF_POLICER_QOS_PARAM_STC       qosParam;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    /* create vlan and ports */
    /* AUTODOC: create VLAN as TAG in with all TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* set PCL rule */
    rc = prvTgfPclPolicerEgressPclSet(
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        PRV_TGF_PCL_POLICER_ENABLE_METER_ONLY_E,
        PRV_TGF_METTERING_ENTRY_INDEX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPolicerPclSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerMeteringEnableSet(
        PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerMeteringEnableSet");

    /* AUTODOC: restore policer Global stage mode */
    rc = prvTgfPolicerStageMeterModeSet(
        PRV_TGF_POLICER_STAGE_EGRESS_E,
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerStageMeterModeSet");

    /* AUTODOC: set metered Packet Size Mode as POLICER_PACKET_SIZE_L2_INCLUDE */
    rc = prvTgfPolicerPacketSizeModeSet(
        PRV_TGF_POLICER_STAGE_EGRESS_E,
        CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerPacketSizeModeSet: %d", prvTgfDevNum);

    /* AUTODOC: create metering entry per egress Port */
    /* AUTODOC: each packet must be marked YELLOW   */
    cpssOsMemSet(&meteringEntry, 0, sizeof(meteringEntry));
    meteringEntry.meterColorMode     = CPSS_POLICER_COLOR_BLIND_E;
    meteringEntry.remarkMode         = PRV_TGF_POLICER_REMARK_MODE_L3_E;
    meteringEntry.meterMode          = PRV_TGF_POLICER_METER_MODE_SR_TCM_E;
    meteringEntry.mngCounterSet      = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    meteringEntry.tbParams.srTcmParams.cir = 0;
    meteringEntry.tbParams.srTcmParams.cbs = 0;
    meteringEntry.tbParams.srTcmParams.ebs = 0x00FFFFFF;
    meteringEntry.modifyUp           = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.modifyDscp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.modifyDp           = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    meteringEntry.greenPcktCmd       = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meteringEntry.yellowPcktCmd      = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meteringEntry.redPcktCmd         = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;

    rc = prvTgfPolicerEntrySet(
        PRV_TGF_POLICER_STAGE_EGRESS_E,
        PRV_TGF_METTERING_ENTRY_INDEX_CNS,
        &meteringEntry, &tbParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerEntrySet");

    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));
    qosParam.dp = CPSS_DP_YELLOW_E;
    rc = prvTgfPolicerEgressQosRemarkingEntrySet(
        PRV_TGF_POLICER_REMARK_TABLE_TYPE_DSCP_E,
        ((prvTgfPacket1Ipv4Part.typeOfService >> 2) & 0x3F),
        CPSS_DP_YELLOW_E, &qosParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerEgressQosRemarkingEntrySet");

    prvTgfPolicerCfiDpToCfiMappingSet(
        GT_TRUE /*enable*/,
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        0 /*greenCfi*/,
        1 /*yellowCfi*/,
        0 /*redCfi*/);

    /* -------------------------------------------------------------------------
     * 2. Generate traffic and Check counters
     */

    /* generate traffic, keep captured packets for check */
    rc = prvTgfPclPolicerTrafficSend(
        1, GT_TRUE, GT_TRUE, GT_FALSE /*capturePrint*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPolicerTrafficSend: %d", prvTgfDevNum);

    /* AUTODOC: check counter of FDB port 1 - 3 Rx\Tx packets expected (captured)*/
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        1 /*prvTgfPacketsCountRxFdb*/, 1 /*prvTgfPacketsCountTxFdb*/,
        prvTgfPacket1Info.totalLen, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersCheck");

    prvTgfPclPolicerEgressUpCfiVidCheck(
        PRV_TGF_UP_PACKET_CNS  /*egressUp*/,
        1  /*egressCfi*/,
        PRV_TGF_VLANID_CNS  /*egressVid*/);
}

/**
* @internal prvTgfPclPolicerDp2CfiMapRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclPolicerDp2CfiMapRestore
(
   GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    PRV_TGF_POLICER_ENTRY_STC           meteringEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    PRV_TGF_POLICER_QOS_PARAM_STC       qosParam;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    /* create vlan and ports */
    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d");

    /* AUTODOC: invalidate PCL rule 1 */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_EXT_E, prvTgfPclRuleIndex, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: disable PCL on all ports */
    prvTgfPclPortsRestore(
    CPSS_PCL_DIRECTION_EGRESS_E,
    CPSS_PCL_LOOKUP_0_0_E);
    /* set PCL rule */

    rc = prvTgfPolicerMeteringEnableSet(
        PRV_TGF_POLICER_STAGE_EGRESS_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerMeteringEnableSet");

    /* AUTODOC: restore policer Global stage mode */
    rc = prvTgfPolicerStageMeterModeSet(
        PRV_TGF_POLICER_STAGE_EGRESS_E,
        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerStageMeterModeSet");

    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));
    rc = prvTgfPolicerEgressQosRemarkingEntrySet(
        PRV_TGF_POLICER_REMARK_TABLE_TYPE_DSCP_E,
        ((prvTgfPacket1Ipv4Part.typeOfService >> 2) & 0x3F),
        CPSS_DP_YELLOW_E, &qosParam);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerEgressQosRemarkingEntrySet");

    /* AUTODOC: erase metering entry  */
    cpssOsMemSet(&meteringEntry, 0, sizeof(meteringEntry));
    meteringEntry.modifyUp           = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.modifyDscp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.modifyDp           = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    rc = prvTgfPolicerEntrySet(
        PRV_TGF_POLICER_STAGE_EGRESS_E,
        PRV_TGF_METTERING_ENTRY_INDEX_CNS,
        &meteringEntry, &tbParams);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerEntrySet");

    prvTgfPolicerCfiDpToCfiMappingSet(
        GT_FALSE /*enable*/,
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        0 /*greenCfi*/,
        0 /*yellowCfi*/,
        0 /*redCfi*/);

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}




