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
* @file prvTgfPclWriteRuleInvalid.c
*
* @brief Specific PCL features testing
*
* @version   8
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
#include <common/tgfCosGen.h>

/* 8 qos profiles 0-7 configired */
/* tc = up = dscp = exp = index  */
/* dp = green                    */

/* port's default profile index */
#define PRV_TGF_QOS_PROFILE_DEFAULT_CNS        7

/* port's new qos profile index */
#define PRV_TGF_QOS_PROFILE_NEW_CNS            6

/* map's new VID */
#define PRV_TGF_VID_NEW_CNS            10

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port number to forward traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS      1

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* indexes of QoS profiles for using in PCL rules and UP,DSCP patterns */
static GT_U16 prvTgfQosProfileId[8] = {0, 1, 2, 3, 4, 5, 6, 7};

/* indexes of PCL Rules */
static GT_U32  relative_prvTgfPclIndex[2]  = {1, 12};
static GT_U32  ipcl_lookup[2]              = {0,  2};
static GT_U32  prvTgfPclIndex[2];
static GT_BOOL prvTgfPclExt[2]   = {GT_FALSE, GT_TRUE};

/*********************** packet (Eth Other packet) ************************/

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
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

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

/* the L2 part of the packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOthEtherTypePart =
{0x3456};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfEthOthPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOthEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of ETH_OTHER packet */
#define PRV_TGF_ETH_OTHER_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + \
    + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfEthOtherPacketInfo =
{
    PRV_TGF_ETH_OTHER_PACKET_LEN_CNS,              /* totalLen */
    (sizeof(prvTgfEthOthPacketPartArray)
        / sizeof(prvTgfEthOthPacketPartArray[0])), /* numOfParts */
    prvTgfEthOthPacketPartArray                    /* partsArray */
};



/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclWriteRuleInvalidQosUpSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclWriteRuleInvalidQosUpSet
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
    GT_BOOL                 useQosProfile = GT_TRUE;

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        useQosProfile = GT_FALSE;
    }

    if (useQosProfile == GT_TRUE)
    {
        rc = prvTgfCosPortQosTrustModeSet(portNum, CPSS_QOS_PORT_NO_TRUST_E);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortQosTrustModeSet");
    }
    else
    {
        GT_U8                   up;
        GT_BOOL                 modifyEnable;
        /* for EXMXPM family */

        up = (GT_U8)((restore == GT_FALSE)
             ? qosProfileIdxPort : 0);
        modifyEnable = (restore == GT_FALSE) ? GT_TRUE : GT_FALSE;

        PRV_UTF_LOG3_MAC(
            "prvTgfCosPortDefaultUpSet: port %d, enable %d, up %d\n",
            portNum, modifyEnable, up);

        rc = prvTgfCosPortModifyQoSParamEnableSet(
            portNum, PRV_TGF_COS_PARAM_UP_E, modifyEnable);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortModifyQoSParamEnableSet");

        rc = prvTgfCosPortDefaultUpSet(portNum, up);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCosPortDefaultUpSet");

        return GT_OK;
    }


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
    GT_STATUS rc, rc1 = GT_OK;
    GT_U8     i;
    GT_U32    pclCount;
    CPSS_PCL_RULE_SIZE_ENT ruleSize;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable ingress policy per device */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

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
    rc = prvTgfPclWriteRuleInvalidQosUpSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_QOS_PROFILE_DEFAULT_CNS, /*qosProfileIdxPort*/
            GT_TRUE);               /* reset */
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclWriteRuleInvalidQosUpSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfPclWriteRuleInvalidTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclWriteRuleInvalidTrafficGenerate
(
    IN  TGF_PACKET_STC *packetInfoPtr,
    IN  GT_U32           portNumSend,
    IN  GT_U32           portNumReceive,
    IN  GT_U8           packetsCountRx,
    IN  GT_U8           packetsCountTx,
    IN  GT_U32          packetSize,
    IN  GT_U32          expectedUp,
    IN  GT_U32          expectedVid,
    OUT GT_U32         *numTriggersPtr
)
{
    GT_STATUS               rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    *numTriggersPtr = 0;

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

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNumSend);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

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


    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNumReceive);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface, 1, vfdArray, numTriggersPtr);
    if (GT_NO_MORE != rc && GT_OK != rc)
        rc1 = rc;

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    return rc1;
};

/**
* @internal prvTgfPclWriteRuleInvalidPclCfgTblSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclWriteRuleInvalidPclCfgTblSet
(
    IN GT_U32                         portNum
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_BOOL                   isDeviceInited = GT_FALSE;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    CPSS_PCL_DIRECTION_ENT           direction;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;

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
    rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortIngressPolicyEnable");

    /* -------------------------------------------------------------------------
     * 2. Configure lookup0_0 ingress entry in VLAN area (idx1 = 4190)
     * Configure lookup0_1 ingress entry in VLAN area (idx2 = 4191)
     */

    /* configure accessModes for lookups 0_0 and 0_1 */
    direction  = CPSS_PCL_DIRECTION_INGRESS_E;
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* AUTODOC: configure lookup0 ingress entry by Port area 0 */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            portNum, direction,
            CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_0");

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: configure lookup1 ingress entry by Port area 0 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                portNum, direction,
                CPSS_PCL_LOOKUP_1_E, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_1");
    }
    else
    {
        /* AUTODOC: configure lookup0_1 ingress entry by Port area 0 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                portNum, direction,
                CPSS_PCL_LOOKUP_0_E, 1 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_1");
    }

    /* set PCL configuration table for VLAN */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = portNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 0;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: set PCL config table for port 0 lookup0 with: */
    /* AUTODOC:   nonIpKey=INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key=INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key=INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, direction, CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet 0");

    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E;

    /* AUTODOC: set PCL config table for port 0 lookup1 with: */
    /* AUTODOC:   nonIpKey=INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key=INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key=INGRESS_STD_IP_L2_QOS */
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, direction, CPSS_PCL_LOOKUP_1_E, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet 1");
    }
    else
    {
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, direction, CPSS_PCL_LOOKUP_0_1_E, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet 1");
    }

    return rc;
}

/**
* @internal prvTgfPclWriteRuleInvalidPclRuleSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclWriteRuleInvalidPclRuleSet
(
    IN GT_U32                                pclRuleIndex,
    IN GT_BOOL                               extRule,
    IN GT_U32                                srcPort,
    IN GT_BOOL                               macIsSa,
    IN GT_U8                                 mac[6],
    IN PRV_TGF_PCL_ACTION_QOS_STC            *qosPtr,
    IN PRV_TGF_PCL_ACTION_VLAN_STC           *vlanPtr
)
{
    GT_STATUS                         rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;

    ruleFormat =
        (extRule == GT_FALSE)
            ? PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    if (extRule == GT_FALSE)
    {
        mask.ruleExtNotIpv6.common.sourcePort =
            (UTF_GET_MAX_PHY_PORTS_NUM_MAC(prvTgfDevNum) < 64)
            ? 0x3F : 0xFF;
        patt.ruleStdNotIp.common.sourcePort = (GT_U8)srcPort;
        if (macIsSa == GT_FALSE)
        {
            cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther,  0xFF, 6);
            cpssOsMemCpy(patt.ruleStdNotIp.macDa.arEther,  mac,  6);
        }
        else
        {
            cpssOsMemSet(mask.ruleStdNotIp.macSa.arEther,  0xFF, 6);
            cpssOsMemCpy(patt.ruleStdNotIp.macSa.arEther,  mac,  6);
        }
    }
    else
    {
        mask.ruleExtNotIpv6.common.sourcePort =
            (UTF_GET_MAX_PHY_PORTS_NUM_MAC(prvTgfDevNum) < 64)
            ? 0x3F : 0xFF;
        patt.ruleExtNotIpv6.common.sourcePort = (GT_U8)srcPort;
        if (macIsSa == GT_FALSE)
        {
            cpssOsMemSet(mask.ruleExtNotIpv6.macDa.arEther,  0xFF, 6);
            cpssOsMemCpy(patt.ruleExtNotIpv6.macDa.arEther,  mac,  6);
        }
        else
        {
            cpssOsMemSet(mask.ruleExtNotIpv6.macSa.arEther,  0xFF, 6);
            cpssOsMemCpy(patt.ruleExtNotIpv6.macSa.arEther,  mac,  6);
        }
    }

    cpssOsMemCpy(&action.qos,          qosPtr,       sizeof(*qosPtr));
    cpssOsMemCpy(&action.vlan,         vlanPtr,      sizeof(*vlanPtr));

    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, pclRuleIndex,
        PRV_TGF_PCL_RULE_OPTION_WRITE_INVALID_E,
        &mask, &patt, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

    return rc;
}

/**
* @internal prvTgfPclWriteRuleInvalidCheck function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclWriteRuleInvalidCheck
(
    IN  GT_BOOL matchBothLookups
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;
    GT_U32    expectedVid;
    GT_U32    expectedUp;
    GT_U32    sendPort;

    PRV_UTF_LOG1_MAC("Check match: %d\n", matchBothLookups);

    sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* update the packet's MACs */
    if (matchBothLookups == GT_FALSE)
    {
        expectedUp = PRV_TGF_QOS_PROFILE_DEFAULT_CNS;
        expectedVid = PRV_TGF_VLANID_CNS;
    }
    else
    {
        expectedUp = PRV_TGF_QOS_PROFILE_NEW_CNS;
        expectedVid = PRV_TGF_VID_NEW_CNS;
    }

    /* AUTODOC: send Ethernet packet on port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   EtherType=0x3456 */
    rc = prvTgfPclWriteRuleInvalidTrafficGenerate(
        &prvTgfEthOtherPacketInfo,
        sendPort,
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        1 /*sent*/, 1 /*received*/,
        (prvTgfEthOtherPacketInfo.totalLen + 4/*vlan tag*/),
        expectedUp, expectedVid, &numTriggers);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclWriteRuleInvalidTrafficGenerate: %d", prvTgfDevNum);

    /* check triggers */
    UTF_VERIFY_EQUAL2_STRING_MAC(1, numTriggers,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
        "   !!! Captured packet has wrong UP Or VID fields !!!\n"
        "   !!! Map QoS functionality FAILED                 !!!\n"
        "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/*******************************************************************************
* prvTgfPclWriteRuleInvalid
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
GT_VOID prvTgfPclWriteRuleInvalid
(
    GT_VOID
)
{
    GT_STATUS                            rc, rc1 = GT_OK;
    PRV_TGF_PCL_ACTION_QOS_STC           qosNone;
    PRV_TGF_PCL_ACTION_VLAN_STC          vlanNone;
    PRV_TGF_PCL_ACTION_QOS_STC           qosNewUp;
    PRV_TGF_PCL_ACTION_VLAN_STC          vlanNewVid;
    CPSS_PCL_RULE_SIZE_ENT               ruleSize;
    GT_U32      ii;

    for(ii = 0 ; ii< 2 ; ii++)
    {
        prvTgfPclIndex[ii] = (GT_U16)prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(ipcl_lookup[ii] , relative_prvTgfPclIndex[ii]);
    }

    /* AUTODOC: SETUP CONFIGURATION: */
    cpssOsMemSet(&qosNone, 0, sizeof(qosNone));
    qosNone.profileAssignIndex = GT_FALSE;

    cpssOsMemSet(&qosNewUp, 0, sizeof(qosNewUp));
    /* for DXCH */
    qosNewUp.profileAssignIndex = GT_TRUE;
    qosNewUp.profileIndex = PRV_TGF_QOS_PROFILE_NEW_CNS;
    qosNewUp.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    /* for EXMXPM */
    qosNewUp.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    qosNewUp.up       = PRV_TGF_QOS_PROFILE_NEW_CNS;
    qosNewUp.profilePrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    cpssOsMemSet(&vlanNone, 0, sizeof(vlanNone));
    /* for DXCH */
    vlanNone.modifyVlan   = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
    /* for EXMXPM */
    vlanNone.vlanCmd      = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
    vlanNone.vidOrInLif   = PRV_TGF_VLANID_CNS;
    vlanNone.nestedVlan   = GT_FALSE;
    vlanNone.precedence   = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    cpssOsMemSet(&vlanNewVid, 0, sizeof(vlanNewVid));
    /* for DXCH */
    vlanNewVid.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    vlanNewVid.vlanId     = PRV_TGF_VID_NEW_CNS;
    vlanNewVid.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    /* for EXMXPM */
    vlanNewVid.vlanCmd    = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    vlanNewVid.vidOrInLif = PRV_TGF_VID_NEW_CNS;
    vlanNewVid.nestedVlan = GT_FALSE;
    vlanNewVid.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;


    /* -------------------------------------------------------------------------
     * 1. Set Base configuration
     */

    /* set common configuration */
    rc = prvTgfDefConfigurationSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfDefConfigurationSet");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: set 8 CoS profile entries with: */
    /* AUTODOC:   qosProfileId [0..7] */
    /* AUTODOC:   DP=GREEN, UP=TC=DSCP=EXP=[0..7] */
    /* AUTODOC: enable modify UP\DSCP for port 0 */
    /* AUTODOC: set Qos Trust Mode as NO_TRUST for port 0 */
    rc = prvTgfPclWriteRuleInvalidQosUpSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_QOS_PROFILE_DEFAULT_CNS, /*qosProfileIdxPort*/
            GT_FALSE);               /* set */
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclWriteRuleInvalidQosUpSet");

    /* init PCL and set 2 PCL Lookups 0, 1 */
    rc = prvTgfPclWriteRuleInvalidPclCfgTblSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfPclWriteRuleInvalidPclCfgTblSet");

    /* AUTODOC: set INVALID PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern sourcePort=0, MAC DA=00:00:00:00:34:02 */
    /* AUTODOC:   enable modify UP, QoS idx 6 */
    rc = prvTgfPclWriteRuleInvalidPclRuleSet(
        prvTgfPclIndex[0],
        prvTgfPclExt[0] /*extRule*/,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]/*srcPort*/,
        GT_FALSE /*isMacSa*/,
        prvTgfPacketL2Part.daMac,
        &qosNewUp,
        &vlanNone);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfPclWriteRuleInvalidPclRuleSet rule0");

    /* AUTODOC: set INVALID PCL rule 12 with: */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd=FORWARD */
    /* AUTODOC:   pattern sourcePort=0, MAC SA=00:00:00:00:00:11 */
    /* AUTODOC:   enable modify VLAN, VID 10 */
    rc = prvTgfPclWriteRuleInvalidPclRuleSet(
        prvTgfPclIndex[1],
        prvTgfPclExt[1] /*extRule*/,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]/*srcPort*/,
        GT_TRUE /*isMacSa*/,
        prvTgfPacketL2Part.saMac,
        &qosNone,
        &vlanNewVid);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfPclWriteRuleInvalidPclRuleSet rule1");

    /* AUTODOC: GENERATE TRAFFIC: */
    /* -------------------------------------------------------------------------
     * 2. Generate traffic
     */

    prvTgfPclWriteRuleInvalidCheck(GT_FALSE /*matchBothLookups*/);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   tagged packet UP=7, VID=5 received on FDB port 1 */

    ruleSize =
        (prvTgfPclExt[0] == GT_FALSE)
            ? CPSS_PCL_RULE_SIZE_STD_E
            : CPSS_PCL_RULE_SIZE_EXT_E;

    /* AUTODOC: validate PCL rule 1 */
    rc = prvTgfPclRuleValidStatusSet(
        ruleSize, prvTgfPclIndex[0], GT_TRUE);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfPclRuleValidStatusSet");

    ruleSize =
        (prvTgfPclExt[1] == GT_FALSE)
            ? CPSS_PCL_RULE_SIZE_STD_E
            : CPSS_PCL_RULE_SIZE_EXT_E;

    /* AUTODOC: validate PCL rule 12 */
    rc = prvTgfPclRuleValidStatusSet(
        ruleSize, prvTgfPclIndex[1], GT_TRUE);
    PRV_UTF_VERIFY_RC1(
        rc, "prvTgfPclRuleValidStatusSet");

    prvTgfPclWriteRuleInvalidCheck(GT_TRUE /*matchBothLookups*/);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   tagged packet UP=6, VID=10 received on FDB port 1 */

    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    rc = prvTgfConfigurationRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfConfigurationRestore");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}


