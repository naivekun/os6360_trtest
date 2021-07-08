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
* @file prvTgfPclFourLookups.c
*
* @brief Specific PCL features testing
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>

/* IPCL lookup0_0 - redirect      */
/* IPCL lookup0_1 - modify Up     */
/* IPCL lookup1   - movify VID    */
/* EPCL           - modify Dscp   */

/* 8 qos profiles 0-7 configired */
/* tc = up = dscp = exp = index  */
/* dp = green                    */

/* port's default profile index */
#define PRV_TGF_QOS_PROFILE_DEFAULT_CNS        7

/* port's new profile index (for UP) */
#define PRV_TGF_QOS_PROFILE_NEW_CNS            6

/* port's default Dscp index */
#define PRV_TGF_DSCP_DEFAULT_CNS        1

/* port's new Dscp index */
#define PRV_TGF_DSCP_NEW_CNS        31

/* map's new VID */
#define PRV_TGF_VID_NEW_CNS            10

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port number to forward traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS      1

/* port number to redirtect traffic to */
#define PRV_TGF_REDIRECT_PORT_IDX_CNS      2

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* indexes of QoS profiles for using in PCL rules and UP,DSCP patterns */
static GT_U16 prvTgfQosProfileId[8] = {0, 1, 2, 3, 4, 5, 6, 7};

/* indexes of PCL Rules */
static GT_U32  relative_prvTgfPclIndex[4]  = {1, 12, 3, 8/*EPCL*/};
static GT_U32  prvTgfPclIndex[4];
static GT_BOOL prvTgfPclExt[4]   = {GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE};

/******************************* IPv4 packet 1 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketIpv4L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
    {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    (PRV_TGF_DSCP_DEFAULT_CNS << 2), /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C9D,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacketIpv4PayloadDataArr[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketIpv4PayloadPart = {
    sizeof(prvTgfPacketIpv4PayloadDataArr),                 /* dataLength */
    prvTgfPacketIpv4PayloadDataArr                          /* dataPtr */
};
/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacketIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketIpv4L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv4PayloadPart}
};

/* LENGTH of packet 1 */
#define PRV_TGF_PACKET_IPV4_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketIpv4PayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketIpv4Info = {
    PRV_TGF_PACKET_IPV4_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketIpv4PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv4PartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclFourLookupsQosUpSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclFourLookupsQosUpSet
(
    IN GT_U32 portNum,
    IN GT_U8 qosProfileIdxPort,
    IN GT_BOOL restore
)
{
    GT_STATUS               rc = GT_OK;
    GT_U8                   qosProfileId;
    GT_U32                  qosProfileCount;
    PRV_TGF_COS_PROFILE_STC cosProfile;
    CPSS_QOS_ENTRY_STC      portQosCfg;

    qosProfileCount = sizeof(prvTgfQosProfileId)
                    / sizeof(prvTgfQosProfileId[0]);

    /* set CoS profile entries with different UP and DSCP */
    for (qosProfileId = 0; qosProfileId < qosProfileCount; qosProfileId++)
    {
        /* reset to default */
        cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));

        if (restore == GT_FALSE)
        {   /* define QoS Profile */
            cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
            cosProfile.userPriority   = prvTgfQosProfileId[qosProfileId];
            cosProfile.trafficClass   = prvTgfQosProfileId[qosProfileId];
            cosProfile.dscp           = prvTgfQosProfileId[qosProfileId];
            cosProfile.exp            = prvTgfQosProfileId[qosProfileId];
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
        portQosCfg.qosProfileId     = 0;
        portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    }

    rc = prvTgfCosPortQosConfigSet(portNum, &portQosCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosConfigSet");

    rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_NO_TRUST_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosTrustModeSet");

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
    GT_U8     tagArray[] = {1, 1, 1, 1};

    /* AUTODOC: create VLAN 5 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* AUTODOC: create VLAN 10 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VID_NEW_CNS,
            prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
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
    GT_STATUS                        rc, rc1 = GT_OK;
    GT_U8                            i;
    GT_U32                           pclCount;
    CPSS_PCL_RULE_SIZE_ENT           ruleSize;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* AUTODOC: disable egress policy on port 2 */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_REDIRECT_PORT_IDX_CNS],
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPclPacketTypesSet FAILED");

    /* AUTODOC: disable ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: disable egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPolicyEnable");

    /* AUTODOC: restore PCL table entry's lookup */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
    interfaceInfo.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup = GT_FALSE;
    lookupCfg.dualLookup = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet I0");

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_1_E, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet I1");
    }

    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
    interfaceInfo.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_REDIRECT_PORT_IDX_CNS];

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup = GT_FALSE;
    lookupCfg.dualLookup = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet E0");

    pclCount = sizeof(prvTgfPclIndex) / sizeof(prvTgfPclIndex[0]);

    /* AUTODOC: invalidate PCL rules */
    for (i = 0; (i < pclCount); i++)
    {
        ruleSize =
            (prvTgfPclExt[i] == GT_FALSE)
                ? CPSS_PCL_RULE_SIZE_STD_E
                : CPSS_PCL_RULE_SIZE_EXT_E;

        rc = prvTgfPclRuleValidStatusSet(
            ruleSize, prvTgfPclIndex[i], GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("\n[TGF]: prvTgfPclRuleValidStatusSet FAILED,"
                " rc = [%d], pclId = %d\n", rc, prvTgfPclIndex[i]);
            rc1 = rc;
        }
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VID_NEW_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: reset QoS configuration */
    rc = prvTgfPclFourLookupsQosUpSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_QOS_PROFILE_DEFAULT_CNS, /*qosProfileIdxPort*/
            GT_TRUE);               /* reset */
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclFourLookupsQosUpSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfPclFourLookupsTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclFourLookupsTrafficGenerate
(
    IN  TGF_PACKET_STC *packetInfoPtr,
    IN  GT_U32           portNumSend,
    IN  GT_U32           portNumReceive,
    IN  GT_U8           packetsCountRx,
    IN  GT_U8           packetsCountTx,
    IN  GT_U32          packetSize,
    IN  GT_U32          expectedUp,
    IN  GT_U32          expectedVid,
    IN  GT_U32          expectedDscp,
    OUT GT_U32         *numTriggersPtr
)
{
    GT_STATUS               rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    *numTriggersPtr = 0;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNumReceive;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr,
                             1 /*burstCount*/, 0 /*numVfd*/, NULL);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: send IPv4 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNumSend);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    /* check ETH counter of FDB port */
    rc = prvTgfEthCountersCheck(prvTgfDevNum, portNumReceive,
        packetsCountRx, packetsCountTx, packetSize, 1);
    PRV_UTF_VERIFY_RC1(rc, "get another counters values.");

    /* check if there is captured packet with specified UP and DSCP fields */
    vfdArray[0].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset        = TGF_L2_HEADER_SIZE_CNS
                              + TGF_ETHERTYPE_SIZE_CNS; /*UP*/
    vfdArray[0].cycleCount    = 2;
    vfdArray[0].patternPtr[0] =
        (GT_U8) (((expectedUp & 0x07) << 5) | ((expectedVid >> 8) & 0x0F));
    vfdArray[0].patternPtr[1] = (GT_U8)(expectedVid & 0xFF);

    vfdArray[1].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset        = /*DSCP of tagged packet*/
        TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + 1 /* offset in Ipv4 header */ ;
    vfdArray[1].cycleCount    = 1;
    vfdArray[1].patternPtr[0] = (GT_U8)((expectedDscp & 0x3F) << 2);


    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNumReceive);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface, 1, vfdArray, numTriggersPtr);
    if (GT_NO_MORE != rc && GT_OK != rc)
        rc1 = rc;

    return rc1;
};

/**
* @internal prvTgfPclFourLookupsPclCfgTblSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclFourLookupsPclCfgTblSet
(
    IN GT_U32                         ingrPortNum,
    IN GT_U32                         egrPortNum,
    IN GT_BOOL                       extKey[3] /* I0, I1, E0*/
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_BOOL                   isDeviceInited = GT_FALSE;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;

    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ingrStdKey[3] =
    {
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
    };
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ingrExtKey[3] =
    {
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
    };
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT egrStdKey[3] =
    {
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
    };
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT egrExtKey[3] =
    {
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E
    };

    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT *keyPtr;


    /* -------------------------------------------------------------------------
     * 1. Enable PCL
     */

    if (GT_FALSE == isDeviceInited)
    {
        /* init PCL */
        rc = prvTgfPclInit();
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclInit");

        /* do not init next time */
        isDeviceInited = GT_TRUE;
    }

    /* AUTODOC: enables ingress policy per devices */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: enables ingress policy for port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(ingrPortNum, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortIngressPolicyEnable");

    /* configure accessModes for all four lookups */
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* AUTODOC: configure lookup0 ingress entry by Port area 0 */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            ingrPortNum, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet I0_0");

    /* AUTODOC: configure lookup0, sublookup 1 ingress entry by Port area 0 */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            ingrPortNum, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E, 1 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet I0_1");

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: configure lookup1 ingress entry by Port area 0 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                ingrPortNum, CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_1_E, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet I1");
    }

    /* AUTODOC: configure lookup0 egress entry by Port area 0 */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            egrPortNum, CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet E0");

    /* IPCL Lookup0 */

    keyPtr = (extKey[0] == GT_FALSE) ? ingrStdKey : ingrExtKey;

    /* set PCL configuration table for port */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = ingrPortNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 0;
    lookupCfg.dualLookup             = GT_TRUE;
    lookupCfg.pclIdL01               = 1;
    lookupCfg.groupKeyTypes.nonIpKey = keyPtr[0];
    lookupCfg.groupKeyTypes.ipv4Key  = keyPtr[1];
    lookupCfg.groupKeyTypes.ipv6Key  = keyPtr[2];

    /* AUTODOC: set PCL config table for port 0 lookup0 with: */
    /* AUTODOC:   nonIpKey=INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key=INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key=INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet I0");

    /* IPCL Lookup1 */

    keyPtr = (extKey[1] == GT_FALSE) ? ingrStdKey : ingrExtKey;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 2;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey = keyPtr[0];
    lookupCfg.groupKeyTypes.ipv4Key  = keyPtr[1];
    lookupCfg.groupKeyTypes.ipv6Key  = keyPtr[2];

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: set PCL config table for port 0 lookup1 with: */
        /* AUTODOC:   nonIpKey=INGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv4Key=INGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv6Key=INGRESS_EXT_IPV6_L2 */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_1_E, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet I1");
    }

    /* enables egress policy per devices */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum, egrPortNum,
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPclPacketTypesSet FAILED");

    /* EPCL Lookup0 */

    keyPtr = (extKey[2] == GT_FALSE) ? egrStdKey : egrExtKey;

    /* set PCL configuration table for port */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = egrPortNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 3;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey = keyPtr[0];
    lookupCfg.groupKeyTypes.ipv4Key  = keyPtr[1];
    lookupCfg.groupKeyTypes.ipv6Key  = keyPtr[2];

    /* AUTODOC: set PCL config table for port 0 lookup1 with: */
    /* AUTODOC:   nonIpKey=EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key=EGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key=EGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet E0");

    return rc;
}

/**
* @internal prvTgfPclFourLookupsPclRuleSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclFourLookupsPclRuleSet
(
    IN GT_U32                                pclRuleIndex,
    IN GT_BOOL                               extRule,
    IN GT_BOOL                               egrRule,
    IN GT_U32                                pclId,
    IN PRV_TGF_PCL_ACTION_REDIRECT_STC       *redirectPtr,
    IN PRV_TGF_PCL_ACTION_QOS_STC            *qosPtr,
    IN PRV_TGF_PCL_ACTION_VLAN_STC           *vlanPtr
)
{
    GT_STATUS                         rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    if (extRule == GT_FALSE)
    {
        if (egrRule == GT_FALSE)
        {
            ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
            mask.ruleStdIpL2Qos.common.pclId = 0x3FF;
            patt.ruleStdIpL2Qos.common.pclId = (GT_U16)pclId;
        }
        else
        {
            ruleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
            mask.ruleEgrStdIpL2Qos.common.pclId = 0x3FF;
            patt.ruleEgrStdIpL2Qos.common.pclId = (GT_U16)pclId;
        }
    }
    else
    {
        if (egrRule == GT_FALSE)
        {
            ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
            mask.ruleExtNotIpv6.common.pclId = 0x3FF;
            patt.ruleExtNotIpv6.common.pclId = (GT_U16)pclId;
        }
        else
        {
            ruleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E;
            mask.ruleEgrExtNotIpv6.common.pclId = 0x3FF;
            patt.ruleEgrExtNotIpv6.common.pclId = (GT_U16)pclId;
        }
    }

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy = egrRule;

    cpssOsMemCpy(&action.redirect,  redirectPtr,      sizeof(*redirectPtr));
    cpssOsMemCpy(&action.qos,       qosPtr,           sizeof(*qosPtr));
    cpssOsMemCpy(&action.vlan,      vlanPtr,          sizeof(*vlanPtr));

    rc = prvTgfPclRuleSet(
        ruleFormat, pclRuleIndex, &mask, &patt, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

    return rc;
}

/**
* @internal prvTgfPclFourLookupsCheck function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclFourLookupsCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;
    GT_U32    expectedVid;
    GT_U32    expectedUp;
    GT_U32    expectedDscp;

    expectedUp   = PRV_TGF_QOS_PROFILE_NEW_CNS;
    expectedVid  = PRV_TGF_VID_NEW_CNS;
    expectedDscp = PRV_TGF_DSCP_NEW_CNS;

    /* send packet and check ETH counters */
    rc = prvTgfPclFourLookupsTrafficGenerate(
        &prvTgfPacketIpv4Info,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_REDIRECT_PORT_IDX_CNS],
        1 /*sent*/, 1 /*received*/,
        (prvTgfPacketIpv4Info.totalLen + 4/*vlan tag*/),
        expectedUp, expectedVid, expectedDscp, &numTriggers);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclFourLookupsTrafficGenerate: %d", prvTgfDevNum);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   tagged packet UP=6, DSCP=31, VID=10 received on port 2 */

    /* check triggers */
    UTF_VERIFY_EQUAL2_STRING_MAC(1, numTriggers,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
        "   !!! Captured packet has wrong UP Or Dscp Or VID fields !!!\n"
        "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/*******************************************************************************
* prvTgfPclFourLookups
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclFourLookups
(
    GT_VOID
)
{
    GT_STATUS                            rc, rc1 = GT_OK;
    PRV_TGF_PCL_ACTION_REDIRECT_STC      qosRedirectNone;
    PRV_TGF_PCL_ACTION_REDIRECT_STC      qosRedirectPort;
    PRV_TGF_PCL_ACTION_QOS_STC           qosNone;
    PRV_TGF_PCL_ACTION_VLAN_STC          vlanNone;
    PRV_TGF_PCL_ACTION_QOS_STC           qosNewUp;
    PRV_TGF_PCL_ACTION_VLAN_STC          vlanNewVid;
    PRV_TGF_PCL_ACTION_QOS_STC           qosEgrNewDscp;
    GT_BOOL                              extKey[3]; /* I0, I1, E0 */
    GT_U32      ii;

    for(ii = 0 ; ii< 4 ; ii++)
    {
        if(ii < 3)
        {
            prvTgfPclIndex[ii] = (GT_U16)prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(ii%3 , relative_prvTgfPclIndex[ii]);
        }
        else
        {
            prvTgfPclIndex[ii] = (GT_U16)prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(relative_prvTgfPclIndex[ii]);
        }
    }



    /* AUTODOC: SETUP CONFIGURATION: */
    cpssOsMemSet(&qosRedirectNone, 0, sizeof(qosRedirectNone));
    qosRedirectNone.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_NONE_E;

    cpssOsMemSet(&qosRedirectPort, 0, sizeof(qosRedirectPort));
    qosRedirectPort.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    qosRedirectPort.data.outIf.outlifType = PRV_TGF_OUTLIF_TYPE_LL_E;
    qosRedirectPort.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
    qosRedirectPort.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    qosRedirectPort.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_REDIRECT_PORT_IDX_CNS];

    cpssOsMemSet(&qosNone, 0, sizeof(qosNone));
    qosNone.profileAssignIndex = GT_FALSE;

    cpssOsMemSet(&qosNewUp, 0, sizeof(qosNewUp));
    qosNewUp.profileAssignIndex = GT_TRUE;
    qosNewUp.profileIndex = PRV_TGF_QOS_PROFILE_NEW_CNS;
    qosNewUp.modifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    qosNewUp.egressDscpCmd = PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;

    cpssOsMemSet(&vlanNone, 0, sizeof(vlanNone));
    vlanNone.vlanCmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;

    cpssOsMemSet(&vlanNewVid, 0, sizeof(vlanNewVid));
    vlanNewVid.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    vlanNewVid.vlanId     = PRV_TGF_VID_NEW_CNS;
    vlanNewVid.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    cpssOsMemSet(&qosEgrNewDscp, 0, sizeof(qosEgrNewDscp));
    qosEgrNewDscp.dscp = PRV_TGF_DSCP_NEW_CNS;
    qosEgrNewDscp.egressDscpCmd =
        PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E;

    /* -------------------------------------------------------------------------
     * 1. Set Base configuration
     */

    /* set common configuration */
    rc = prvTgfDefConfigurationSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfDefConfigurationSet");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketIpv4L2Part.daMac,
            PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: set 8 CoS profile entries with: */
    /* AUTODOC:   qosProfileId [0..7] */
    /* AUTODOC:   DP=GREEN, UP=TC=DSCP=EXP=[0..7] */
    /* AUTODOC: enable modify UP\DSCP for port 0 */
    /* AUTODOC: set Qos Trust Mode as NO_TRUST for port 0 */
    rc = prvTgfPclFourLookupsQosUpSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_QOS_PROFILE_DEFAULT_CNS, /*qosProfileIdxPort*/
            GT_FALSE);               /* set */
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclFourLookupsQosUpSet");

    /* AUTODOC: enable egress Policy */
    prvTgfPclEgressPolicyEnable(GT_TRUE);

    /* init PCL and set 2 PCL Lookups I0,I1,E0 */
    extKey[0] = prvTgfPclExt[0]; /* prvTgfPclExt[1] is as [0] */
    extKey[1] = prvTgfPclExt[2];
    extKey[2] = prvTgfPclExt[3];
    rc = prvTgfPclFourLookupsPclCfgTblSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_REDIRECT_PORT_IDX_CNS],
            extKey);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfPclFourLookupsPclCfgTblSet");

    /* IPCL lookup0_0 - redirect      */
    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd=FORWARD */
    /* AUTODOC:   pattern pclId = 0 */
    /* AUTODOC:   redirect to port 2 */
    rc = prvTgfPclFourLookupsPclRuleSet(
        prvTgfPclIndex[0],
        prvTgfPclExt[0] /*extRule*/,
        GT_FALSE /*egrRule*/,
        0 /*pclId*/,
        &qosRedirectPort,
        &qosNone,
        &vlanNone);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfPclFourLookupsPclRuleSet rule0");

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* IPCL lookup0_1 - modify Up     */
        /* AUTODOC: set PCL rule 12 with: */
        /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd=FORWARD */
        /* AUTODOC:   pattern pclId = 1 */
        /* AUTODOC:   enable modify UP, QoS idx 6 */
        rc = prvTgfPclFourLookupsPclRuleSet(
            prvTgfPclIndex[1],
            prvTgfPclExt[1] /*extRule*/,
            GT_FALSE /*egrRule*/,
            1 /*pclId*/,
            &qosRedirectNone,
            &qosNewUp,
            &vlanNone);
        PRV_UTF_VERIFY_RC1(
            rc, "prvTgfPclFourLookupsPclRuleSet rule1");

        /* IPCL lookup1   - movify VID    */
        /* AUTODOC: set PCL rule 3 with: */
        /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd=FORWARD */
        /* AUTODOC:   pattern pclId = 2 */
        /* AUTODOC:   enable modify VLAN, VID 10 */
        rc = prvTgfPclFourLookupsPclRuleSet(
            prvTgfPclIndex[2],
            prvTgfPclExt[2] /*extRule*/,
            GT_FALSE /*egrRule*/,
            2 /*pclId*/,
            &qosRedirectNone,
            &qosNone,
            &vlanNewVid);
        PRV_UTF_VERIFY_RC1(
            rc, "prvTgfPclFourLookupsPclRuleSet rule1");
    }
    else
    {
        /* IPCL lookup0_1 - modify Up     */
        /* AUTODOC: set PCL rule 12 with: */
        /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd=FORWARD */
        /* AUTODOC:   pattern pclId = 1 */
        /* AUTODOC:   enable modify VLAN, modify UP, QoS idx 6 */
        rc = prvTgfPclFourLookupsPclRuleSet(
            prvTgfPclIndex[1],
            prvTgfPclExt[1] /*extRule*/,
            GT_FALSE /*egrRule*/,
            1 /*pclId*/,
            &qosRedirectNone,
            &qosNewUp,
            &vlanNewVid);
        PRV_UTF_VERIFY_RC1(
            rc, "prvTgfPclFourLookupsPclRuleSet rule1");
    }

    /* EPCL           - modify Dscp   */
    /* AUTODOC: set PCL rule 8 with: */
    /* AUTODOC:   format EGRESS_STD_IP_L2_QOS, cmd=FORWARD */
    /* AUTODOC:   pattern pclId = 3 */
    /* AUTODOC:   enable modify DSCP, dscp=31 */
    rc = prvTgfPclFourLookupsPclRuleSet(
        prvTgfPclIndex[3],
        prvTgfPclExt[3] /*extRule*/,
        GT_TRUE /*egrRule*/,
        3 /*pclId*/,
        &qosRedirectNone,
        &qosEgrNewDscp,
        &vlanNone);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfPclFourLookupsPclRuleSet rule1");

    /* -------------------------------------------------------------------------
     * 2. Generate traffic
     */

    prvTgfPclFourLookupsCheck();

    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    rc = prvTgfConfigurationRestore();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfConfigurationRestore");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}


