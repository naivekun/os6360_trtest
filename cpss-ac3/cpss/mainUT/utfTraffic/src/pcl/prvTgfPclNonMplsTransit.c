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
* @file prvTgfPclNonMplsTransit.c
*
* @brief Test for passenger parsing of Non-MPLS transit Tunnel Packets.
*
* @version   6
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

#include <common/tgfTunnelGen.h>
#include <pcl/prvTgfPclNonMplsTransit.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* VLAN Id */
#define PRV_TGF_VLANID_CNS          313

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* TTI & IPCL rule & action indexes */
#define PRV_TGF_TTI_IPV4_O_IPV4_IDX_CNS   1
#define PRV_TGF_TTI_ETH_O_IPV4_IDX_CNS  100
#define PRV_TGF_PCL_IPV4_O_IPV4_IDX_CNS  10
#define PRV_TGF_PCL_ETH_O_IPV4_IDX_CNS   20

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/******************************* Test packet **********************************/

/* L2 tunnel part */
static TGF_PACKET_L2_STC prvTgfPacketL2TunnelPart = {
    {0x00, 0x00, 0x00, 0x00, 0x09, 0x0A},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C}                /* saMac */
};

/* L2 passenger part */
static TGF_PACKET_L2_STC prvTgfPacketL2PassengerPart = {
    {0x00, 0x00, 0x00, 0x00, 0x99, 0xAA},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0xBB, 0xCC}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* IPv4 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* IPv4 tunnel part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4TunnelPart = {
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
    { 1, 2, 3, 4},   /* srcAddr */
    { 5, 6, 7, 8}    /* dstAddr */
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

/* PARTS of IPv4 over IPv4 packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv4OverIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,          &prvTgfPacketL2TunnelPart},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,   &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfPacketIpv4TunnelPart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfPacketIpv4PassengerPart},
    {TGF_PACKET_PART_PAYLOAD_E,     &prvTgfPacketPayloadPart}
};

/* PARTS of Ethernet over IPv4 packet */
static TGF_PACKET_PART_STC prvTgfPacketEthOverIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,          &prvTgfPacketL2TunnelPart},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,   &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfPacketIpv4TunnelPart},
    {TGF_PACKET_PART_L2_E,          &prvTgfPacketL2PassengerPart},
    {TGF_PACKET_PART_PAYLOAD_E,     &prvTgfPacketPayloadPart}
};

/*************************** Restore config ***********************************/



/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclNonMplsTransitTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry and add ports.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfPclNonMplsTransitTestVlanInit
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
* @internal prvTgfPclNonMplsTransitTtiRuleAndActionBuild function
* @endinternal
*
* @brief   Build TTI rule and action
*
* @param[in] index                    - rule and action indexes
* @param[in] sipAddr                  - (pointer to) sip addrress to match
* @param[in] dipAddr                  - (pointer to) dip addrress to match
* @param[in] passengerType            - tunnel passenger type
*                                       None
*/
static GT_VOID prvTgfPclNonMplsTransitTtiRuleAndActionBuild
(
    IN GT_U32                           index,
    IN TGF_IPV4_ADDR                    *sipAddr,
    IN TGF_IPV4_ADDR                    *dipAddr,
    IN PRV_TGF_TTI_PASSENGER_TYPE_ENT   passengerType
)
{
    GT_STATUS   rc;

    PRV_TGF_TTI_ACTION_2_STC    ttiAction;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    cpssOsMemCpy(ttiPattern.ipv4.srcIp.arIP, sipAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiPattern.ipv4.destIp.arIP, dipAddr, sizeof(TGF_IPV4_ADDR));

    cpssOsMemSet(ttiMask.ipv4.srcIp.arIP, 0xFF, sizeof(TGF_IPV4_ADDR));
    cpssOsMemSet(ttiMask.ipv4.destIp.arIP, 0xFF, sizeof(TGF_IPV4_ADDR));

    ttiAction.tunnelTerminate = GT_FALSE;
    ttiAction.ttPassengerPacketType = passengerType;
    ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = GT_TRUE;
    ttiAction.ttHeaderLength = TGF_IPV4_HEADER_SIZE_CNS;

    rc = prvTgfTtiRule2Set(index, PRV_TGF_TTI_KEY_IPV4_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d, %d",
                                 index, PRV_TGF_TTI_KEY_IPV4_E);
}


/**
* @internal prvTgfPclNonMplsTransitTestTtiConfiguration function
* @endinternal
*
* @brief   Set TTI configurations - enabling, rules & actions
*/
static GT_VOID prvTgfPclNonMplsTransitTestTtiConfiguration
(
    GT_VOID
)
{
    GT_STATUS   rc;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable the TTI lookup for IPv4 on port */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);

    /* AUTODOC: create rule & action for IPv4-over-IPv4 */
    prvTgfPclNonMplsTransitTtiRuleAndActionBuild(
        PRV_TGF_TTI_IPV4_O_IPV4_IDX_CNS,
        &prvTgfPacketIpv4TunnelPart.srcAddr,
        &prvTgfPacketIpv4TunnelPart.dstAddr,
        PRV_TGF_TTI_PASSENGER_IPV4_E);

    /* AUTODOC: Change tunnel dip to match different entries */
    prvTgfPacketIpv4TunnelPart.dstAddr[0] += 1;

    /* AUTODOC: create rule & action for Ethernet-over-IPv4 */
    prvTgfPclNonMplsTransitTtiRuleAndActionBuild(
        PRV_TGF_TTI_ETH_O_IPV4_IDX_CNS,
        &prvTgfPacketIpv4TunnelPart.srcAddr,
        &prvTgfPacketIpv4TunnelPart.dstAddr,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E);

    /* AUTODOC: restore tunnel dip */
    prvTgfPacketIpv4TunnelPart.dstAddr[0] -= 1;
}


/**
* @internal prvTgfPclNonMplsTransitTestIpclRulesAndActionsSet function
* @endinternal
*
* @brief   Set IPCL rules and actions
*/
static GT_VOID prvTgfPclNonMplsTransitTestIpclRulesAndActionsSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: action - hard drop */
    action.pktCmd = PRV_TGF_PACKET_CMD_DROP_HARD_E;

    /* AUTODOC: Mask for isIp, isIpv4, sip, dip & UDBs */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdIpv4L4.common.isIp = 1;
    mask.ruleStdIpv4L4.common.isIpv4 = 1;
    cpssOsMemSet(mask.ruleStdIpv4L4.sip.arIP, 0xFF, sizeof(GT_IPADDR));
    cpssOsMemSet(mask.ruleStdIpv4L4.dip.arIP, 0xFF, sizeof(GT_IPADDR));
    cpssOsMemSet(mask.ruleStdIpv4L4.udb + 31, 0xFF, 4);

    /* AUTODOC: Pattern for IPv4-over-IPv4 */
    pattern.ruleStdIpv4L4.common.isIp = 1;
    pattern.ruleStdIpv4L4.common.isIpv4 = 1;
    cpssOsMemCpy(pattern.ruleStdIpv4L4.sip.arIP,
                 prvTgfPacketIpv4TunnelPart.srcAddr,
                 sizeof(GT_IPADDR));
    cpssOsMemCpy(pattern.ruleStdIpv4L4.dip.arIP,
                 prvTgfPacketIpv4TunnelPart.dstAddr,
                 sizeof(GT_IPADDR));
    pattern.ruleStdIpv4L4.udb[31] = prvTgfPacketIpv4PassengerPart.srcAddr[0];
    pattern.ruleStdIpv4L4.udb[32] = prvTgfPacketIpv4PassengerPart.srcAddr[1];
    pattern.ruleStdIpv4L4.udb[33] = prvTgfPacketIpv4TunnelPart.srcAddr[0];
    pattern.ruleStdIpv4L4.udb[34] = prvTgfPacketIpv4TunnelPart.srcAddr[1];

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
            PRV_TGF_PCL_IPV4_O_IPV4_IDX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                                 PRV_TGF_PCL_IPV4_O_IPV4_IDX_CNS);

    /* AUTODOC: Change tunnel dip to match different entries */
    prvTgfPacketIpv4TunnelPart.dstAddr[0] += 1;

    /* AUTODOC: Pattern for Ethernet-over-IPv4 */
    cpssOsMemCpy(pattern.ruleStdIpv4L4.dip.arIP,
                 prvTgfPacketIpv4TunnelPart.dstAddr,
                 sizeof(GT_IPADDR));

    /* AUTODOC: restore tunnel dip */
    prvTgfPacketIpv4TunnelPart.dstAddr[0] -= 1;

    pattern.ruleStdIpv4L4.udb[31] = prvTgfPacketL2PassengerPart.daMac[4];
    pattern.ruleStdIpv4L4.udb[32] = prvTgfPacketL2PassengerPart.daMac[5];
    pattern.ruleStdIpv4L4.udb[33] = prvTgfPacketL2TunnelPart.daMac[4];
    pattern.ruleStdIpv4L4.udb[34] = prvTgfPacketL2TunnelPart.daMac[5];

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
            PRV_TGF_PCL_ETH_O_IPV4_IDX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                                 PRV_TGF_PCL_ETH_O_IPV4_IDX_CNS);
}


/**
* @internal prvTgfPclNonMplsTransitTestUdbConiguration function
* @endinternal
*
* @brief   Set UDB configurations.
*/
static GT_VOID prvTgfPclNonMplsTransitTestUdbConiguration
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: UDB 31 for IPv4 packet contains passenger first SIP byte */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        31 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
        14/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        31, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 14);

    /* AUTODOC: UDB 32 for IPv4 packet contains passenger second SIP byte */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        32 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
        15/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        32, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 15);

    /* AUTODOC: UDB 33 for IPv4 packet contains tunnel first SIP byte */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        33 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
        14/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        33, PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 14);

    /* AUTODOC: UDB 34 for IPv4 packet contains tunnel second SIP byte */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        34 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
        15/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        34, PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 15);

    /* AUTODOC: UDB 31 for Ethernet packet contains passenger 5th MAC byte */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        31 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_L2_E,
        4/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        31, PRV_TGF_PCL_OFFSET_L2_E, 4);

    /* AUTODOC: UDB 32 for Ethernet packet contains passenger 6th MAC byte */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        32 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_L2_E,
        5/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        32, PRV_TGF_PCL_OFFSET_L2_E, 5);

    /* AUTODOC: UDB 33 for Ethernet packet contains tunnel 5th MAC byte */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        33 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,
        4/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        33, PRV_TGF_PCL_OFFSET_TUNNEL_L2_E, 4);

    /* AUTODOC: UDB 34 for Ethernet packet contains tunnel 6th MAC byte */
    rc = prvTgfPclUserDefinedByteSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        34 /*udbIndex*/,
        PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,
        5/*offset*/);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc,
        "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                     "OffsetType %d, Offset %d\n",
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        34, PRV_TGF_PCL_OFFSET_TUNNEL_L2_E, 5);
}


/**
* @internal prvTgfPclNonMplsTransitTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfPclNonMplsTransitTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN with all ports as members. */
    prvTgfPclNonMplsTransitTestVlanInit(PRV_TGF_VLANID_CNS);

    /* AUTODOC: TTI configurations */
    prvTgfPclNonMplsTransitTestTtiConfiguration();

    /* AUTODOC: Init IPCL Engine - parsing according to Standard IPv4 L4 key */
    rc = prvTgfPclDefPortInit(
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
         CPSS_PCL_DIRECTION_INGRESS_E,
         CPSS_PCL_LOOKUP_0_E,
         PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
         PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
         PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: IPCL rules & actions setting */
    prvTgfPclNonMplsTransitTestIpclRulesAndActionsSet();

    /* AUTODOC: UDB assignment configurations*/
    prvTgfPclNonMplsTransitTestUdbConiguration();
}

/**
* @internal prvTgfPclNonMplsTransitTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPclNonMplsTransitTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_PCL_IPV4_O_IPV4_IDX_CNS ,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_EXT_E,
                                 PRV_TGF_PCL_IPV4_O_IPV4_IDX_CNS ,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_PCL_ETH_O_IPV4_IDX_CNS ,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_EXT_E,
                                 PRV_TGF_PCL_ETH_O_IPV4_IDX_CNS ,
                                 GT_FALSE);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy on port */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_IPV4_O_IPV4_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d",
                                 PRV_TGF_TTI_IPV4_O_IPV4_IDX_CNS, GT_FALSE);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_ETH_O_IPV4_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d",
                                 PRV_TGF_TTI_ETH_O_IPV4_IDX_CNS, GT_FALSE);

    /* AUTODOC: Disable the TTI lookup for IPv4 on port */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


/**
* @internal prvTgfPclNonMplsTransitTestCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] sendPortIndex            -  sending port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPclNonMplsTransitTestCheckCounters
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
* @internal prvTgfPclNonMplsTransitTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] portIndex                - port indexing place in test array ports.
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclNonMplsTransitTestPacketSend
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
* @internal prvTgfPclNonMplsTransitTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPclNonMplsTransitTestGenerateTraffic
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    GT_U32                          partsCount;
    GT_U32                          packetSize;
    TGF_PACKET_STC                  packetInfo;

    /* AUTODOC: IPv4-over-IPv4 traffic start */
    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketIpv4OverIpv4PartArray) / sizeof(prvTgfPacketIpv4OverIpv4PartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketIpv4OverIpv4PartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "IPv4oIPv4, prvTgfPacketSizeGet: %d, %d",
                                 partsCount, packetSize);

    /* AUTODOC: build packet info */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketIpv4OverIpv4PartArray;

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclNonMplsTransitTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                          &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclNonMplsTransitTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             0, 0);

    /* AUTODOC: Change passenger sip[3] */
    prvTgfPacketIpv4PassengerPart.srcAddr[3] += 1;

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclNonMplsTransitTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                          &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclNonMplsTransitTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             0, 1);

    /* AUTODOC: Change passenger sip[1] */
    prvTgfPacketIpv4PassengerPart.srcAddr[1] += 1;

    /* AUTODOC: Send packet - expect NO match -> FLOOD */
    prvTgfPclNonMplsTransitTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                          &packetInfo);

    /* AUTODOC: Verify packet flooding in vlan */
    prvTgfPclNonMplsTransitTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             prvTgfBurstCount, 2);

    /* AUTODOC: Restore passenger sip[1] & sip[3] */
    prvTgfPacketIpv4PassengerPart.srcAddr[1] -= 1;
    prvTgfPacketIpv4PassengerPart.srcAddr[3] -= 1;

    /* AUTODOC: IPv4-over-IPv4 traffic end */

    /* AUTODOC: Eth-over-IPv4 traffic start */

    /* AUTODOC: Change tunnel dip to match different entries */
    prvTgfPacketIpv4TunnelPart.dstAddr[0] += 1;

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketEthOverIpv4PartArray) / sizeof(prvTgfPacketEthOverIpv4PartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketEthOverIpv4PartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "EthoIPv4, prvTgfPacketSizeGet: %d, %d",
                                 partsCount, packetSize);

    /* AUTODOC: build packet info */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketEthOverIpv4PartArray;

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclNonMplsTransitTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                          &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclNonMplsTransitTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             0, 0);

    /* AUTODOC: Change passenger macDa[3] */
    prvTgfPacketL2PassengerPart.daMac[3] += 1;

    /* AUTODOC: Send packet - expect match -> DROP */
    prvTgfPclNonMplsTransitTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                          &packetInfo);

    /* AUTODOC: Verify packet drop */
    prvTgfPclNonMplsTransitTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             0, 1);

    /* AUTODOC: Change passenger macDa[5] */
    prvTgfPacketL2PassengerPart.daMac[5] += 1;

    /* AUTODOC: Send packet - expect NO match -> FLOOD */
    prvTgfPclNonMplsTransitTestPacketSend(PRV_TGF_SEND_PORT_IDX_CNS,
                                          &packetInfo);

    /* AUTODOC: Verify packet flooding in vlan */
    prvTgfPclNonMplsTransitTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                             prvTgfBurstCount, 2);

    /* AUTODOC: Restore passenger macDa[3] & macDa[5] */
    prvTgfPacketL2PassengerPart.daMac[3] -= 1;
    prvTgfPacketL2PassengerPart.daMac[5] -= 1;

    /* AUTODOC: restore tunnel dip */
    prvTgfPacketIpv4TunnelPart.dstAddr[0] -= 1;

    /* AUTODOC: Eth-over-IPv4 traffic end */
}

/**
* @internal prvTgfPclNonMplsTransitTest function
* @endinternal
*
* @brief   IPCL Non-MPLS transit tunnel test
*/
GT_VOID prvTgfPclNonMplsTransitTest
(
    GT_VOID
)
{
    prvTgfPclNonMplsTransitTestConfigurationSet();

    prvTgfPclNonMplsTransitTestGenerateTraffic();

    prvTgfPclNonMplsTransitTestConfigurationRestore();
}


