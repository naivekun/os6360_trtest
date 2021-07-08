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
* @file prvTgfPclPortGroupPolicer.c
*
* @brief Specific PCL features testing
*
* @version   19
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
#include <pcl/prvTgfPclPortGroupPolicer.h>

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

/* index for tagged port */
#define PRV_TGF_TAGGED_PORT_IDX_CNS   1

/* mettering entry index  */
#define PRV_TGF_METTERING_ENTRY_INDEX_CNS  7

/* billing counters index  */
#define PRV_TGF_BILLING_COUNTERS_INDEX_CNS  3

#define PRV_UTF_VERIFY_RC1(rc, name)                                         \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
        rc1 = rc;                                                            \
    }

#define PRV_UTF_PRINT_RESULT_FIELD(fldcaption, resname, fldname)                                     \
    for (sendIter = prvTgfSendIterMin; sendIter < prvTgfSendIterMax; sendIter += prvTgfSendIterStep) \
    {                                                                                                \
        if (sendIter == prvTgfSendIterMin)                                                           \
            PRV_UTF_LOG1_MAC("\n  %-20s = ", fldcaption);                                             \
        PRV_UTF_LOG1_MAC(" %2d", prvTgfTestResults##resname[sendIter].fldname);                    \
    }

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

typedef struct
{
    GT_BOOL checkBillingCounters;
    GT_BOOL checkMngCounters;
    GT_U32  billingPacketSize;
    GT_U32  billingGreenPktNum;
    GT_U32  billingYellowPktNum;
    GT_U32  billingRedPktNum;
    GT_U32  mngPacketSize;
    GT_U32  mngGreenPktNum;
    GT_U32  mngYellowPktNum;
    GT_U32  mngRedPktNum;
    GT_U32  mngDropPktNum;
    GT_U32  totalPasssedPktNum;
} TGF_PLR_RESULT_PATTERN_STC;

/* Drop Precedence for each of packets in the test */
static GT_U32 prvTgfPacketDropPrecedence[8] =
    {0, 0, 0, 0, 0, 0, 0, 0};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

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
        rc = prvTgfCosProfileEntrySet(qosProfileId, &cosProfile);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosProfileEntrySet 1");
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

    return rc;
}

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
    GT_U32                         i;
    PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore policer counting mode */
    rc = prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_POLICER_COUNTING_DISABLE_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerCountingModeSet");

    /* AUTODOC: restore policer Global stage mode */
    rc = prvTgfPolicerStageMeterModeSet(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerStageMeterModeSet");

    /* set the conformance level as GREEN */
    prvTgfPolicerConformanceLevelForce(0);

    /* AUTODOC: disable metering */
    rc = prvTgfPolicerMeteringEnableSet(policerStage, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerMeteringEnableSet");

    for (i = 0; (i < 4); i++)
    {
        /* AUTODOC: disable ingress policy on all ports */
        rc = prvTgfPclPortIngressPolicyEnable(
            prvTgfPortsArray[i], GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");
    }

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


    for (i = 0; (i < 4); i++)
    {
        /* AUTODOC: reset QoS profile entries [0..7] */
        /* AUTODOC: restore default QoS Profile for all ports */
        rc = prvTgfPclPolicerQosUpSet(
                prvTgfPortsArray[i],
                0, /*qosProfileIdxPort*/
                GT_TRUE);              /* reset configuration */
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPolicerQosUpSet");
    }

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
    IN GT_U32 sendPortIndex,
    IN GT_U32 receivePortIndex
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
            prvTgfPortsArray[sendPortIndex],
            prvTgfPortsArray[receivePortIndex]);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* create a macEntry for packet 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[receivePortIndex], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portNumReceive                = prvTgfPortsArray[receivePortIndex];
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

        /* AUTODOC: send 5 TCP packets on sendPort with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
        /* AUTODOC:   VID=5, UPs=[0x00,0x20,0x40,0x60,0x80] */
        /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
        /* AUTODOC:   srcport=1, dstPort=2 */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
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
    IN GT_U32                          ingressPort
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

    /* AUTODOC: init PCL Engine for sendPort: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    rc = prvTgfPclDefPortInit(
        ingressPort,
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E /*ipv6Key*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclDefPortInit");

    /* set action */
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.policer.policerEnable = policerEnable;
    action.policer.policerId     =
        (policerEnable == PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E)
            ? PRV_TGF_BILLING_COUNTERS_INDEX_CNS
            : PRV_TGF_METTERING_ENTRY_INDEX_CNS;

    /* AUTODOC: set PCL rule 1 for all packets: */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd - FORWARD */
    /* AUTODOC:   POLICER_ENABLE_COUNTER_ONLY, policerId=3 <- for counting test */
    /* AUTODOC:   POLICER_ENABLE_METER_AND_COUNTER, policerId=7 <- for metering test */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        prvTgfPclRuleIndex, &mask, &patt, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

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
    GT_U32                              qosProfileIndex;
    GT_U8                               greenQosTableRemarkIndex;
    GT_U8                               yellowQosTableRemarkIndex;
    GT_U8                               redQosTableRemarkIndex;
    PRV_TGF_POLICER_COUNTING_MODE_ENT   countingMode;

    countingMode = PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E;

    /* AUTODOC: config policer counting mode: */
    /* AUTODOC:   ingress Policer stage #0, billing or IPFIX counting */
    rc = prvTgfPolicerCountingModeSet(
        prvTgfDevNum, policerStage, countingMode);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
        prvTgfDevNum, policerStage, countingMode);

    /* AUTODOC: config policer global stage mode: */
    /* AUTODOC:   ingress Policer stage #0, Flow-based metering */
    rc = prvTgfPolicerStageMeterModeSet(
        policerStage,
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
        policerStage,
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);

    /* set QoS Remarking Entry */
    qosProfileIndex           = PRV_TGF_UP_YELLOW_CNS;
    greenQosTableRemarkIndex  = 2;
    yellowQosTableRemarkIndex = 2;
    redQosTableRemarkIndex    = 3;

    rc = prvTgfPolicerQosRemarkingEntrySet(
        policerStage, qosProfileIndex,
        greenQosTableRemarkIndex, yellowQosTableRemarkIndex, redQosTableRemarkIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerQosRemarkingEntrySet: %d", prvTgfDevNum);

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
* @internal prvTgfPolicerAllCountersGet function
* @endinternal
*
* @brief   Get the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerAllCountersGet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS rc = GT_OK;

    /* get Policer Billing Counters */
    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum, policerStage,
            PRV_TGF_BILLING_COUNTERS_INDEX_CNS, GT_FALSE, &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerBillingEntryGet: %d", prvTgfDevNum);

    /* get Policer Management Counters GREEN */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* get Policer Management Counters YELLOW */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    /* get Policer Management Counters RED */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    /* get Policer Management Counters DROP */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerCountersCheck
(
    TGF_PLR_RESULT_PATTERN_STC  *patternPtr
)
{
    if (patternPtr->checkBillingCounters != GT_FALSE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            (patternPtr->billingGreenPktNum * patternPtr->billingPacketSize),
            prvTgfBillingCntr.greenCntr.l[0],
                "prvTgfBillingCntr.greenCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            (patternPtr->billingYellowPktNum * patternPtr->billingPacketSize),
            prvTgfBillingCntr.yellowCntr.l[0],
                "prvTgfBillingCntr.yellowCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            (patternPtr->billingRedPktNum * patternPtr->billingPacketSize),
            prvTgfBillingCntr.redCntr.l[0],
                "prvTgfBillingCntr.redCnt is wrong");
    }

    if (patternPtr->checkMngCounters != GT_FALSE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            patternPtr->mngGreenPktNum,
            prvTgfMngCntrGreen.packetMngCntr,
                "prvTgfMngCntrGreen.packetMngCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            (patternPtr->mngGreenPktNum * patternPtr->mngPacketSize),
            prvTgfMngCntrGreen.duMngCntr.l[0],
                "prvTgfMngCntrGreen.duMngCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            patternPtr->mngYellowPktNum,
            prvTgfMngCntrYellow.packetMngCntr,
                "prvTgfMngCntrYellow.packetMngCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            (patternPtr->mngYellowPktNum * patternPtr->mngPacketSize),
            prvTgfMngCntrYellow.duMngCntr.l[0],
                "prvTgfMngCntrYellow.duMngCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            patternPtr->mngRedPktNum,
            prvTgfMngCntrRed.packetMngCntr,
                "prvTgfMngCntrRed.packetMngCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            (patternPtr->mngRedPktNum * patternPtr->mngPacketSize),
            prvTgfMngCntrRed.duMngCntr.l[0],
                "prvTgfMngCntrRed.duMngCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            patternPtr->mngDropPktNum,
            prvTgfMngCntrDrop.packetMngCntr,
                "prvTgfMngCntrDrop.packetMngCntr is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(
            (patternPtr->mngDropPktNum * patternPtr->mngPacketSize),
            prvTgfMngCntrDrop.duMngCntr.l[0],
                "prvTgfMngCntrDrop.duMngCntr is wrong");

    }
}

/**
* @internal prvTgfPolicerCountersPrint function
* @endinternal
*
* @brief   Print the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerCountersPrint
(
    GT_VOID
)
{
    /* print Policer Billing Counters */
    PRV_UTF_LOG1_MAC("billingCntr_Green  = %d\n", prvTgfBillingCntr.greenCntr.l[0]);
    PRV_UTF_LOG1_MAC("billingCntr_Yellow = %d\n", prvTgfBillingCntr.yellowCntr.l[0]);
    PRV_UTF_LOG1_MAC("billingCntr_Red    = %d\n", prvTgfBillingCntr.redCntr.l[0]);

    /* print Policer Management Counters GREEN */
    PRV_UTF_LOG1_MAC("\nmanagementCntr_Green  = %d", prvTgfMngCntrGreen.packetMngCntr);
    PRV_UTF_LOG1_MAC(", Bytes = %d", prvTgfMngCntrGreen.duMngCntr.l[0]);
    if (prvTgfMngCntrGreen.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC(", %d", prvTgfMngCntrGreen.duMngCntr.l[1]);

    /* print Policer Management Counters YELLOW */
    PRV_UTF_LOG1_MAC("\nmanagementCntr_Yellow = %d", prvTgfMngCntrYellow.packetMngCntr);
    PRV_UTF_LOG1_MAC(", Bytes = %d", prvTgfMngCntrYellow.duMngCntr.l[0]);
    if (prvTgfMngCntrYellow.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC(", %d", prvTgfMngCntrYellow.duMngCntr.l[1]);

    /* print Policer Management Counters RED */
    PRV_UTF_LOG1_MAC("\nmanagementCntr_Red    = %d", prvTgfMngCntrRed.packetMngCntr);
    PRV_UTF_LOG1_MAC(", Bytes = %d", prvTgfMngCntrRed.duMngCntr.l[0]);
    if (prvTgfMngCntrRed.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC(", %d", prvTgfMngCntrRed.duMngCntr.l[1]);

    /* print Policer Management Counters DROP */
    PRV_UTF_LOG1_MAC("\nmanagementCntr_Drop   = %d", prvTgfMngCntrDrop.packetMngCntr);
    PRV_UTF_LOG1_MAC(", Bytes = %d", prvTgfMngCntrDrop.duMngCntr.l[0]);
    if (prvTgfMngCntrDrop.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC(", = %d", prvTgfMngCntrDrop.duMngCntr.l[1]);

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
    rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum, policerStage,
           PRV_TGF_BILLING_COUNTERS_INDEX_CNS, &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclPortGroupPolicerTestIter function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclPortGroupPolicerTestIter
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN PRV_TGF_PCL_POLICER_ENABLE_ENT policerEnable,
    IN GT_U32                         sendPortIndex,
    IN GT_U32                         receivePortIndex,
    IN TGF_PLR_RESULT_PATTERN_STC     *patternPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32     sendPortNum;
    GT_U32     receivePortNum;
    GT_U32    expectedRcvPackets;

    expectedRcvPackets = patternPtr->totalPasssedPktNum;

    sendPortNum     = prvTgfPortsArray[sendPortIndex];
    receivePortNum  = prvTgfPortsArray[receivePortIndex];

    /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(sendPortNum);

    /* AUTODOC: SETUP CONFIGURATION: */
    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    /* create vlan and ports */
    rc = prvTgfDefConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfDefConfigurationSet: %d", prvTgfDevNum);

    /* AUTODOC: disables ingress policy on sendPort port */
    rc = prvTgfPclPortIngressPolicyEnable(
            receivePortNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* set QoS configuration */
    /* AUTODOC: set 8 QoS profile entries [0..7] with: */
    /* AUTODOC:   UP=DP=[0,0,0,0,0,0,0,0], DSCP=TC=EXP=7 */

    /* AUTODOC: for port 0 set default QoS Profile with: */
    /* AUTODOC:   assignPrecedence=PRECEDENCE_SOFT */
    /* AUTODOC:   qosProfileId=7, enableModify UP\DSCP */
    /* AUTODOC: set QoS Map for tagged packets with specified UP fields [0..7] */
    rc = prvTgfPclPolicerQosUpSet(
            sendPortNum,
            PRV_TGF_UP_DEFAULT_CNS, /*qosProfileIdxPort*/
            GT_FALSE   /* restore */);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPolicerQosUpSet");

    /* set PCL rule */
    rc = prvTgfPclPolicerPclSet(policerEnable, sendPortNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRulesDefine: %d", prvTgfDevNum);

    /* set policer configuration */
    prvTgfPclPolicerSet(policerStage);

    /* restore PortGroupsBmp mode */
    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* -------------------------------------------------------------------------
     * 2. Generate traffic and Check counters
     */

    /* reset the value of specified Billing, Management and Policy Counters */
    prvTgfPolicerCountersReset(policerStage);

    /* generate traffic */
    rc = prvTgfPclPolicerTrafficGenerate(sendPortIndex, receivePortIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortGroupPolicerTrafficGenerate: %d", prvTgfDevNum);

    /* AUTODOC: check counter on receivePort - 5 Rx\Tx packets expected */
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
        receivePortNum,
        expectedRcvPackets, expectedRcvPackets,
        prvTgfPacket1Info.totalLen -
        ((prvTgfPortsArray[receivePortIndex] == receivePortNum) &&
         (prvTgfPortsArray[PRV_TGF_TAGGED_PORT_IDX_CNS] != receivePortNum)) * TGF_VLAN_TAG_SIZE_CNS, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck: %d, %d",
                                 prvTgfDevNum, receivePortNum);

    /* get the value of specified Billing, Management and Policy Counters */
    prvTgfPolicerAllCountersGet(policerStage);

    /* print the value of specified Billing, Management and Policy Counters */
    prvTgfPolicerCountersPrint();

    prvTgfPolicerCountersCheck(patternPtr);
}

/**
* @internal prvTgfPclPortGroupPolicerTest function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclPortGroupPolicerTest
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN PRV_TGF_PCL_POLICER_ENABLE_ENT policerEnable,
    IN TGF_PLR_RESULT_PATTERN_STC     *patternPtr
)
{
    GT_STATUS rc;
    GT_U32    sendIter;
    GT_U32    rcvPortIdx;

    /* set port numbers for using in the test
     * portGroup0:  0  1  2  3   4  5  6  7   8  9 10 11    12 13 14 15
     * portGroup1: 16 17 18 19  20 21 22 23  24 25 26 27    28 29 30 31
     * portGroup2: 32 33 34 35  36 37 38 39  40 41 42 43    44 45 46 47
     * portGroup3: 48 49 50 51  52 53 54 55  56 57 58 59    60 61 62 63
     */

    for (sendIter = 0; (sendIter < 4); sendIter++)
    {
        rcvPortIdx = ((sendIter + 1) & 0x03);

        /* AUTODOC: Run test iteration with: */
        /* AUTODOC:   sendPorts=[0,1,2,3], receivePorts=[1,2,3,0] */
        prvTgfPclPortGroupPolicerTestIter(
            policerStage, policerEnable,
            sendIter /* send port index */,
            rcvPortIdx /* receive port index */,
            patternPtr);
        /* AUTODOC: verify Billing and Management Counters: */
        /* AUTODOC:   green=2, yellow=2, red=1 -> for counting test */
        /* AUTODOC:   green=2, yellow=1, red=2 -> for metering test */

        /* PortGroupsBmp API used only in ...PolicerTestIter     */
        /* Forbid them even if PolicerTestIter failed and broken */
        usePortGroupsBmp  = GT_FALSE;
        currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* restore configuration */
        rc = prvTgfConfigurationRestore();
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);
    }
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclPortGroupPolicerCounting function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPortGroupPolicerCounting
(
   GT_VOID
)
{
    TGF_PLR_RESULT_PATTERN_STC cntrPattern;

    cntrPattern.checkBillingCounters = GT_TRUE;
    cntrPattern.billingPacketSize = 1;
    cntrPattern.billingGreenPktNum = 2;
    cntrPattern.billingYellowPktNum = 2;
    cntrPattern.billingRedPktNum = 1;

    cntrPattern.checkMngCounters = GT_FALSE;
    cntrPattern.mngPacketSize = 0;
    cntrPattern.mngGreenPktNum = 0;
    cntrPattern.mngYellowPktNum = 0;
    cntrPattern.mngRedPktNum = 0;
    cntrPattern.mngDropPktNum = 0;

    cntrPattern.totalPasssedPktNum = 5;

    prvTgfPacketDropPrecedence[0] = 0;
    prvTgfPacketDropPrecedence[1] = 0;
    prvTgfPacketDropPrecedence[2] = 1;
    prvTgfPacketDropPrecedence[3] = 1;
    prvTgfPacketDropPrecedence[4] = 2;

    /* run the test */
    prvTgfPclPortGroupPolicerTest(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E,
        &cntrPattern);

}

/**
* @internal prvTgfPclPortGroupPolicerMetering function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPortGroupPolicerMetering
(
   GT_VOID
)
{
    TGF_PLR_RESULT_PATTERN_STC cntrPattern;

    cntrPattern.checkBillingCounters = GT_TRUE;
    cntrPattern.billingPacketSize = 1;
    cntrPattern.billingGreenPktNum = 2;
    cntrPattern.billingYellowPktNum = 1;
    cntrPattern.billingRedPktNum = 2;

    cntrPattern.checkMngCounters = GT_TRUE;
    cntrPattern.mngPacketSize =
        prvTgfPacket1Info.totalLen + TGF_CRC_LEN_CNS;
    cntrPattern.mngGreenPktNum = 2;
    cntrPattern.mngYellowPktNum = 1;
    cntrPattern.mngRedPktNum = 0;
    cntrPattern.mngDropPktNum = 2;

    cntrPattern.totalPasssedPktNum = 3;

    prvTgfPacketDropPrecedence[0] = 0;
    prvTgfPacketDropPrecedence[1] = 0;
    prvTgfPacketDropPrecedence[2] = 1;
    prvTgfPacketDropPrecedence[3] = 2;
    prvTgfPacketDropPrecedence[4] = 2;

    /* run the test */
    prvTgfPclPortGroupPolicerTest(
        PRV_TGF_POLICER_STAGE_INGRESS_0_E,
        PRV_TGF_PCL_POLICER_ENABLE_METER_AND_COUNTER_E,
        &cntrPattern);
}



