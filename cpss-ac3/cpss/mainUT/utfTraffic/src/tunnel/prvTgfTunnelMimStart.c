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
* @file prvTgfTunnelMimStart.c
*
* @brief Tunnel Start: Mac In Mac Functionality
*
* @version   34
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelMimStart.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS              1

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* port number to send traffic to in default VLAN */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number of unused port in default VLAN */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* port number to send traffic to */
#define PRV_TGF_FREE_PORT_IDX_CNS         2

/* port number to receive traffic from in nextHop VLAN */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS      3

/* mac SA index in global mac sa table */
#define PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS          7


#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/************************ common parts of packets **************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x32,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/************************* packet 1 (IPv4 packet) **************************/

/* PARTS of packet 1 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};
/* PACKET 1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    ( TGF_L2_HEADER_SIZE_CNS
    + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
    + sizeof(prvTgfPacket1PayloadDataArr)),                       /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* MAC for Tunnel Start Entry */
static TGF_MAC_ADDR prvTgfStartEntryMac = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x11
};

static const GT_CHAR prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "sending packet to port [%d] ...\n"
};
static TGF_PACKET_STC *prvTgfPacketInfoArray[] = {
    &prvTgfPacket1Info
};

/* stored default inlifMode */
static PRV_TGF_INLIF_PORT_MODE_ENT     inlifMode;

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* PCL Rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* TTI Rule index */
static GT_U32   prvTgfTtiRuleIndex = 1;

/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32   prvTgfTunnelStartLineIndex = 8;

/* define expected number of received packets (Sent packets) */
static GT_U8 prvTgfPacketsCountRxArray[][PRV_TGF_PORTS_NUM_CNS] = {
/* ports:  SEND VLN1 VLN2 RECEIVE */
             {1,  0,  0,  1}
};

/* define expected number of transmitted packets */
static GT_U8 prvTgfPacketsCountTxArray[][PRV_TGF_PORTS_NUM_CNS] = {
/* ports:  SEND VLN1 VLN2 RECEIVE */
             {1,  0,  0,  1}
};

/* value of MacSa */
static TGF_MAC_ADDR prvTgfGlobalMacSaArr={0x00, 0x00, 0x00, 0x00, 0x00, 0x09};


/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT        macMode;
    GT_U32                          controlWordvalue;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCountersIpReset function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCountersIpReset
(
    GT_VOID
)
{
    GT_U32       portIter;
    GT_STATUS rc = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset IP couters and set CPSS_DXCH_IP_CNT_SET_ROUTE_ENTRY_MODE_E mode */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCountersIpSet");
    }

    return rc;
};

/**
* @internal prvTgfCountersIpCheck function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCountersIpCheck
(
    GT_VOID
)
{
    GT_STATUS                  rc = GT_OK;
    GT_U32                     portIter;
    PRV_TGF_IP_COUNTER_SET_STC ipCounters;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);

        /* check IP counters */
        rc = prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCountersIpGet");
    }

    PRV_UTF_LOG0_MAC("\n");

    return rc;
};

/**
* @internal prvTgfPortCaptureEnable function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPortCaptureEnable
(
    IN GT_U32   portNum,
    IN GT_BOOL start
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNum;

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, start);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    return rc;
};

/**
* @internal prvTgfPortCapturePacketPrint function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPortCapturePacketPrint
(
    IN GT_U32        portNum,
    IN TGF_MAC_ADDR macDaPattern,
    IN GT_BOOL      checkMacSa,
    IN TGF_MAC_ADDR macSaPattern,
    OUT GT_U32     *numTriggersPtr
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    CPSS_NULL_PTR_CHECK_MAC(numTriggersPtr);
    *numTriggersPtr = 0;

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNum);

    /* set nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNum;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set pattern as MAC to select captured packets */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, macDaPattern, sizeof(TGF_MAC_ADDR));


    if(checkMacSa == GT_TRUE)
    {
        /* set pattern as MAC to select captured packets */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[1].patternPtr, macSaPattern, sizeof(TGF_MAC_ADDR));
    }

    /* check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, checkMacSa?2:1, vfdArray, numTriggersPtr);
    rc = rc == GT_NO_MORE ? GT_OK : rc;

    return rc;
};

/**
* @internal prvTgfVlanSet function
* @endinternal
*
* @brief   Set VLAN entry
*
* @param[in] vlanId                   -  to be configured
* @param[in] portsMembers             - VLAN's ports Members
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfVlanSet
(
    IN GT_U16          vlanId,
    IN CPSS_PORTS_BMP_STC portsMembers
)
{
    GT_STATUS                          rc = GT_OK;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PORTS_BMP_STC                 portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC          vlanInfo;

    /* clear ports tagging */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    vlanInfo.autoLearnDisable = GT_TRUE; /* working in controlled learning */
    vlanInfo.naMsgToCpuEn     = GT_TRUE; /* working in controlled learning */

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEntryWrite");

    return rc;
};

/**
* @internal prvTgfDefBrigeConfSet function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefBrigeConfSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORTS_BMP_STC              vlanMembers = {{0, 0}};

    /* clear entry */
    cpssOsMemSet(&vlanMembers, 0, sizeof(vlanMembers));

    /* ports 0, 8 are VLAN_1 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS]);

    prvTgfVlanSet(PRV_TGF_SEND_VLANID_CNS, vlanMembers);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfVlanSet");

    /* clear entry */
    cpssOsMemSet(&vlanMembers, 0, sizeof(vlanMembers));

    /* ports 18, 23 are VLAN_2 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_FREE_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    prvTgfVlanSet(PRV_TGF_NEXTHOPE_VLANID_CNS, vlanMembers);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfVlanSet");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidGet");

    /* set default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");

    /* create a macEntry for original packet 1 in VLAN 5 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_SEND_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* create a macEntry for tunneled packet 1 in VLAN 6 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_NEXTHOPE_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
};

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfConfigurationRestore
(
    IN GT_BOOL isPclSet,
    IN GT_BOOL isTtiSet
)
{
    GT_STATUS rc = GT_OK;

    /* invalidate PCL rules */
    if (isPclSet)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfPclRuleValidStatusSet");
        prvTgfPclRestore();
    }

    /* invalidate TTI rules */
    if (isTtiSet)
    {
        rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiRuleValidStatusSet");
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");

    /* restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: restore default inlif port mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,inlifMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfInlifPortModeSet");

    return rc;
};

/**
* @internal prvTgfTrafficGenerate function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    sendIter;

    /* sending packets */
    for (sendIter = 0; sendIter < PRV_TGF_MAX_ITER_CNS; sendIter++)
    {
        PRV_UTF_LOG1_MAC(
            prvTgfIterationNameArray[sendIter],
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, prvTgfPacketInfoArray[sendIter],
            1 /*burstCount*/, 0 /*numVfd*/, NULL);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
    }

    return rc;
};

/**
* @internal prvTgfDefPclRuleSet function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefPclRuleSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT patt;
    PRV_TGF_PCL_ACTION_STC      action;

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclDefPortInit");

    /* difine mask, pattern and action */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.ruleStdIpL2Qos.macDa.arEther, 0xFF,
            sizeof(mask.ruleStdIpL2Qos.macDa.arEther));

    cpssOsMemSet(&patt, 0, sizeof(patt));
    cpssOsMemCpy(patt.ruleStdIpL2Qos.macDa.arEther, prvTgfPacket1L2Part.daMac,
            sizeof(prvTgfPacket1L2Part.daMac));

    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    action.redirect.data.outIf.outlifType = PRV_TGF_OUTLIF_TYPE_TUNNEL_E;
    action.redirect.data.outIf.outlifPointer.tunnelStartPtr.tunnelType =
        PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
    action.redirect.data.outIf.outlifPointer.tunnelStartPtr.ptr = prvTgfTunnelStartLineIndex;

    /* set PCL rule */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        prvTgfPclRuleIndex, &mask, &patt, &action);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");

    return rc;
};

/**
* @internal prvTgfInlifConfigSet function
* @endinternal
*
* @brief   Set initial test settings:
*         - Create inlif on port 0
* @param[in] mplsRouteEnable          -  GT_TRUE: enable MLPS in inlif
*                                      GT_FALSE: disable MPLS in inlif
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfInlifConfigSet
(
    GT_BOOL         mplsRouteEnable
)
{
#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(mplsRouteEnable);

    /* not relevant for DxCh, always return GT_OK */
    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;

    PRV_UTF_LOG0_MAC("======= Setting Inlif Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Set Inlif Entry - that needs for Puma devices
     */

    /* Bridge Section */
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.naStormPreventionEnable   = GT_FALSE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkDaUcCommand            = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
    inlifEntry.untaggedMruIndex          = 0;
    inlifEntry.unregNonIpMcCommand       = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpMcCommand          = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpv4BcCommand        = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregNonIpv4BcCommand     = CPSS_PACKET_CMD_FORWARD_E;

    /* Router Section */
    inlifEntry.ipv4UcRouteEnable = GT_TRUE;
    inlifEntry.ipv4McRouteEnable = GT_FALSE;
    inlifEntry.ipv6UcRouteEnable = GT_FALSE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = mplsRouteEnable;
    inlifEntry.vrfId             = 0;

    /* SCT Section */
    inlifEntry.ipv4IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.ipv6IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipSecurityProfile              = 0;
    inlifEntry.ipv4IgmpToCpuEnable            = GT_FALSE;
    inlifEntry.ipv6IcmpToCpuEnable            = GT_FALSE;
    inlifEntry.udpBcRelayEnable               = GT_FALSE;
    inlifEntry.arpBcToCpuEnable               = GT_FALSE;
    inlifEntry.arpBcToMeEnable                = GT_FALSE;
    inlifEntry.ripv1MirrorEnable              = GT_FALSE;
    inlifEntry.ipv4LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6NeighborSolicitationEnable = GT_FALSE;

    /* Misc Section */
    inlifEntry.mirrorToAnalyzerEnable = GT_FALSE;
    inlifEntry.mirrorToCpuEnable      = GT_FALSE;

     /* save inlif port mode */
    rc = prvTgfInlifPortModeGet(PRV_TGF_SEND_PORT_IDX_CNS, &inlifMode);
    PRV_UTF_VERIFY_GT_OK(rc, "ERROR of prvTgfInlifPortModeGet");

    /* set inlif mode to support single input logical interface for port 0  */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,
                                   PRV_TGF_INLIF_PORT_MODE_PORT_E);
    PRV_UTF_VERIFY_GT_OK(rc, "ERROR of prvTgfInlifPortModeSet");

    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             &inlifEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfInlifEntrySet");


     /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpPortRoutingEnable");

    /* Force-Vlan-Mode-on-TT */
   /* rc = prvTgfInlifIntForceVlanModeOnTtSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifIntForceVlanModeOnTtSet");
*/
    /* Disable VLAN Egress Filtering on specified device for Bridged Known Unicast packets*/
    rc = prvTgfBrgVlanEgressFilteringEnable(GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "ERROR of prvTgfInlifIntForceVlanModeOnTtSet");

    return rc;

#endif /*EXMXPM_FAMILY*/
}

/**
* @internal prvTgfTtiEnable function
* @endinternal
*
* @brief   This function enables the TTI lookup
*         and sets the lookup Mac mode
*         for the specified key type
* @param[in] portNum                  - port number
* @param[in] ttiKeyType               - TTI key type
* @param[in] macMode                  - MAC mode to use
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTtiEnable
(
    IN  GT_U32                    portNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT ttiKeyType,
    IN  PRV_TGF_TTI_MAC_MODE_ENT macMode,
    IN  GT_BOOL                  isTtiEnable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ethType;

    if (GT_TRUE == isTtiEnable)
    {
        /* set MIM EtherType */
        ethType = TGF_ETHERTYPE_88E7_MIM_TAG_CNS;

        /* save current Eth TTI key lookup MAC mode */
        rc = prvTgfTtiMacModeGet(prvTgfDevNum, ttiKeyType, &prvTgfRestoreCfg.macMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeGet");
    }
    else
    {
        /* restore default MIM EtherType */
        ethType = TGF_ETHERTYPE_88E7_MIM_TAG_CNS;

        /* restore Eth TTI key lookup MAC mode */
        macMode = prvTgfRestoreCfg.macMode;
    }

    /* set MIM EtherType */
    rc = prvTgfTtiMimEthTypeSet(ethType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMimEthTypeSet");

    /* enable/disable port for ttiKeyType TTI lookup */
    rc = prvTgfTtiPortLookupEnableSet(portNum, ttiKeyType, isTtiEnable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* set TTI lookup MAC mode */
    rc = prvTgfTtiMacModeSet(ttiKeyType, macMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeSet");

    return rc;
};

/**
* @internal prvTgfDefTtiEthRuleSet function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefTtiEthRuleSet
(
    GT_VOID
)
{
    GT_STATUS              rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT   ttiPattern;
    PRV_TGF_TTI_RULE_UNT   ttiMask;

    /* cpssDxChTtiRuleSet(CPSS_DXCH_TTI_ETH_RULE_STC, CPSS_DXCH_TTI_ACTION_2_STC) */

    /* set action - redirect to port 23 */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    ttiAction.tunnelTerminate               = GT_FALSE;
    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.tag0VlanCmd                   = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                    = PRV_TGF_NEXTHOPE_VLANID_CNS;
    ttiAction.tunnelStart                   = GT_TRUE;
    ttiAction.tunnelStartPtr                = prvTgfTunnelStartLineIndex;

    ttiAction.vlanCmd                       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.vlanId                        = PRV_TGF_NEXTHOPE_VLANID_CNS;

    /* set TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemCpy(ttiPattern.eth.common.mac.arEther, prvTgfPacket1L2Part.daMac, sizeof(TGF_MAC_ADDR));
    ttiPattern.eth.common.vid      = PRV_TGF_SEND_VLANID_CNS;
    ttiPattern.eth.common.pclId    = 3;

    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(ttiMask.eth.common.mac.arEther, 0xFF, sizeof(TGF_MAC_ADDR));
    ttiMask.eth.common.vid         = 0x0F;
    ttiMask.eth.common.pclId       = 0x3FF;

    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleSet");

    /* debug - Check TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    rc = prvTgfTtiRuleGet(prvTgfDevNum, prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleGet");

    return rc;
};

/**
* @internal prvTgfDefTunnelStartEntryMimSet function
* @endinternal
*
* @brief   Set a default MacInMac tunnel start entry
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefTunnelStartEntryMimSet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    /* set a Tunnel Start Entry
     * cpssDxChTunnelStartEntrySet(,,CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC)
     */
    tunnelType = CPSS_TUNNEL_MAC_IN_MAC_E;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.mimCfg.tagEnable        = GT_TRUE;
    tunnelEntry.mimCfg.vlanId           = PRV_TGF_NEXTHOPE_VLANID_CNS;
    tunnelEntry.mimCfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.mimCfg.up               = 0x3;
    tunnelEntry.mimCfg.ttl              = 64;
    cpssOsMemCpy(tunnelEntry.mimCfg.macDa.arEther, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR));
    tunnelEntry.mimCfg.retainCrc        = GT_FALSE;
    tunnelEntry.mimCfg.iSid             = 0x12345;
    tunnelEntry.mimCfg.iSidAssignMode   = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.mimCfg.iUp              = 0x5;
    tunnelEntry.mimCfg.iUpMarkMode      = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.mimCfg.iDp              = 0;
    tunnelEntry.mimCfg.iDpMarkMode      = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    
    if((PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum) == GT_TRUE)||
        (PRV_CPSS_SIP_5_16_CHECK_MAC(prvTgfDevNum) == GT_TRUE)||                
            (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) == GT_TRUE))
    {
        tunnelEntry.mimCfg.mimBSaAssignMode = PRV_TGF_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E;
    }
    

    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartLineIndex, tunnelType, &tunnelEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntrySet");

    /* debug - check Tunnel Start Entry - vlanId and macDa fields */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfTunnelStartLineIndex, &tunnelType, &tunnelEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntryGet");

    /* check tunnelType */
    rc = (CPSS_TUNNEL_MAC_IN_MAC_E == tunnelType)? GT_OK : GT_FAIL;
    if (GT_OK != rc)
        PRV_UTF_LOG1_MAC("tunnelType: %d\n", tunnelType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntryGet");

    /* check VLAN Id field */
    rc = (PRV_TGF_NEXTHOPE_VLANID_CNS == tunnelEntry.mimCfg.vlanId)? GT_OK : GT_FAIL;
    if (GT_OK != rc)
        PRV_UTF_LOG1_MAC("tunnelEntry.mimCfg.vlanId: %d\n", tunnelEntry.mimCfg.vlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntryGet");

    /* check MAC DA field */
    rc = cpssOsMemCmp(tunnelEntry.mimCfg.macDa.arEther, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FAIL;
    if (GT_OK != rc)
        PRV_UTF_LOG1_MAC("tunnelEntry.mimCfg.macDa.arEther[5]: 0x%2X\n", tunnelEntry.mimCfg.macDa.arEther[5]);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntryGet");

    return rc;
};



/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelMimStart function
* @endinternal
*
* @brief   MIM Tunnel Start
*/
GT_VOID prvTgfTunnelMimStart
(
    IN PRV_TGF_TUNNEL_MIM_START_TYPE_ENT startType
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter;
    GT_U32    packetSize;
    GT_BOOL   isPclSet = GT_FALSE;
    GT_BOOL   isTtiSet = GT_FALSE;
    GT_U32    numTriggers = 0;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    GT_ETHERADDR                    macSaAddr;
    GT_ETHERADDR                    macSaAddrSave;
    GT_BOOL                         checkMacSa = GT_FALSE;
    GT_U32                          routerMacSaIndexSave;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    PRV_UTF_LOG1_MAC("\nVLAN %d CONFIGURATION:\n", PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",
        prvTgfPortsArray[0], prvTgfPortsArray[1]);

    PRV_UTF_LOG1_MAC("VLAN %d CONFIGURATION:\n", PRV_TGF_NEXTHOPE_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",
        prvTgfPortsArray[2], prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("FDB CONFIGURATION:\n");
    PRV_UTF_LOG1_MAC("  MAC DA ...:34:02 to port [%d]\n", prvTgfPortsArray[1]);
    PRV_UTF_LOG1_MAC("  MAC DA ...:00:11 to port [%d]\n", prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("PCL CONFIGURATION:\n");
    if(PRV_TGF_TUNNEL_MIM_START_FROM_PCL_E == startType)
        PRV_UTF_LOG0_MAC("  forward the packet to tunnel start egress interface.\n");
    else
        PRV_UTF_LOG0_MAC("  CLEAR\n");

    PRV_UTF_LOG0_MAC("TTI CONFIGURATION:\n");
    PRV_UTF_LOG2_MAC("  FORWARD packet with MAC DA ..:34:02 to port [%d]\n"
        "  in VLAN %d with MIM tunnel (MAC DA must be ...:00:11)\n",
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);

    PRV_UTF_LOG0_MAC("TRAFFIC GENERATION:\n");
    PRV_UTF_LOG1_MAC("  Sending packet to port [%d]\n",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    PRV_UTF_LOG0_MAC("EXPECTED RESULTS:\n");
    PRV_UTF_LOG1_MAC("  packet is not forwarded in VLAN %d\n",
        PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  packet is forwarded to port [%d] VLAN %d with MIM tunnel\n\n",
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);

    /* -------------------------------------------------------------------------
    * 1. Set configuration
    */

    if((PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum) == GT_TRUE)||
        (PRV_CPSS_SIP_5_16_CHECK_MAC(prvTgfDevNum) == GT_TRUE)||                
        (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) == GT_TRUE))
    {
        checkMacSa = GT_TRUE;
    }

    /* set VLANs */
    rc = prvTgfDefBrigeConfSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefBrigeConfSet: %d", prvTgfDevNum);

    /* set PCL rules */
    if(PRV_TGF_TUNNEL_MIM_START_FROM_PCL_E == startType && GT_OK == rc)
    {
        rc = prvTgfDefPclRuleSet();
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfDefPclRuleSet: %d", prvTgfDevNum);
        isPclSet = (GT_OK == rc);
    }

    /* Set inlif configuration */
    if (GT_OK == rc) {
        rc = prvTgfInlifConfigSet(GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifConfigSet: %d", prvTgfDevNum);
    }

    /* enable TTI lookup for Eth key and MAC DA mode */
    if (GT_OK == rc) {
        rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_TTI_KEY_ETH_E,
            PRV_TGF_TTI_MAC_MODE_DA_E,
            GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);
    }

    /* set TTI rules for Eth key */
    if (GT_OK == rc) {
        rc = prvTgfDefTtiEthRuleSet();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTtiEthRuleSet: %d", prvTgfDevNum);
        isTtiSet = (GT_OK == rc);
    }

    /* set a MIM Tunnel Start Entry */
    if (GT_OK == rc) {
        rc = prvTgfDefTunnelStartEntryMimSet();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTunnelStartEntryMimSet: %d", prvTgfDevNum);
    }

    if(checkMacSa == GT_TRUE)
    {

    /* AUTODOC: configure MAC SA (00:00:00:00:00:09) to global MAC SA table into index 7 */

        rc = cpssDxChIpRouterGlobalMacSaGet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &macSaAddrSave);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpRouterGlobalMacSaGet: %d", prvTgfDevNum);


        cpssOsMemCpy(macSaAddr.arEther, prvTgfGlobalMacSaArr, sizeof(TGF_MAC_ADDR));
        rc = cpssDxChIpRouterGlobalMacSaSet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &macSaAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpRouterGlobalMacSaSet: %d", prvTgfDevNum);



        /* AUTODOC: configure index to global MAC SA table in eport attribute table */
        rc =  prvTgfIpRouterPortGlobalMacSaIndexGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],&routerMacSaIndexSave);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet: %d", prvTgfDevNum);

        rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet: %d", prvTgfDevNum);
    }


    /* -------------------------------------------------------------------------
    * 2. Generate traffic and Check counters
    */

    if (rc == GT_OK)
    {
        /* get routing mode */
        rc = prvTgfIpRoutingModeGet(&routingMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP counters */
            rc = prvTgfCountersIpReset();
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfCountersIpReset: %d", prvTgfDevNum);
        }

        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* select IPv4 packet to send */
        prvTgfPacketInfoArray[0] = &prvTgfPacket1Info;

        /* enable capturing of receive port */
        rc = prvTgfPortCaptureEnable(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* generate traffic */
        rc = prvTgfTrafficGenerate();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrafficGenerate: %d", prvTgfDevNum);

        /* disable capturing of receive port */
        rc = prvTgfPortCaptureEnable(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* check ETH counters */
        for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
        {
            if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
            {
                /* packet after tunnel start in VLAN 6 */
                packetSize = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + 2
                    + prvTgfPacketInfoArray[0]->totalLen + TGF_CRC_LEN_CNS;
            }
            else
            {
                /* original packet before tunneling in VLAN 5 */
                packetSize = prvTgfPacketInfoArray[0]->totalLen;
            }

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(
                prvTgfDevNum,
                prvTgfPortsArray[portIter],
                prvTgfPacketsCountRxArray[0][portIter],
                prvTgfPacketsCountTxArray[0][portIter],
                packetSize, 1 /*burstCount*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* check IP counters */
            rc = prvTgfCountersIpCheck();
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfEthCountersCheck: %d", prvTgfDevNum);
        }



        /* print captured packets from receive port with prvTgfStartEntryMac */
        rc = prvTgfPortCapturePacketPrint(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfStartEntryMac,checkMacSa,prvTgfGlobalMacSaArr, &numTriggers);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfPortCapturePacketPrint:"
            " port = %d, rc = 0x%02X\n",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], rc);

        if(checkMacSa == GT_TRUE)
        {

            /* check if captured packet has the same MAC SA as prvTgfGlobalMacSaArr */
            UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers>>1,
                "\n   MAC SA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
                prvTgfGlobalMacSaArr[0], prvTgfGlobalMacSaArr[1], prvTgfGlobalMacSaArr[2],
                prvTgfGlobalMacSaArr[3], prvTgfGlobalMacSaArr[4], prvTgfGlobalMacSaArr[5]);


            numTriggers&=~(0x2);
        }

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
            prvTgfStartEntryMac[0], prvTgfStartEntryMac[1], prvTgfStartEntryMac[2],
            prvTgfStartEntryMac[3], prvTgfStartEntryMac[4], prvTgfStartEntryMac[5]);


        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPortCapturePacketPrint: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
    * 3. Restore configuration
    */

    /* restore TTI lookup and MAC DA mode for Eth key */
    rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_ETH_E,
        PRV_TGF_TTI_MAC_MODE_DA_E,
        GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);

    if(checkMacSa == GT_TRUE)
    {
        rc = cpssDxChIpRouterGlobalMacSaSet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &macSaAddrSave);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpRouterGlobalMacSaSet: %d", prvTgfDevNum);

        rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],routerMacSaIndexSave);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet: %d", prvTgfDevNum);
    }

    /* restore common configuration */
    rc = prvTgfConfigurationRestore(isPclSet, isTtiSet);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


