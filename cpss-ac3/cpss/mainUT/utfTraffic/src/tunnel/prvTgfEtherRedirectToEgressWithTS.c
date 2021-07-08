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
* @file prvTgfEtherRedirectToEgressWithTS.c
*
* @brief Ethernet packet redirect to egress with TS
*
* @version   10
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
#include <tunnel/prvTgfEtherRedirectToEgressWithTS.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* GRE header size (in bytes) */
#define TGF_GRE_HEADER_SIZE_CNS           4

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id */
#define PRV_TGF_VLANID_6_CNS            6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           2

/* number of vfd's */
#define PRV_TGF_VFD_NUM_CNS             2

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32 prvTgfRouterArpTunnelStartLineIndex = 8;

/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvTgfTunnelMacDa = {0x88, 0x77, 0x11, 0x11, 0x55, 0x66};

/* Tunnel destination IP */
static TGF_IPV4_ADDR prvTgfTunnelDstIp = {1, 1, 1, 2};

/* Tunnel source IP */
static TGF_IPV4_ADDR prvTgfTunnelSrcIp = {2, 2, 3, 3};

/* expected number of packets on ports */
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_PORTS_NUM_CNS] = { 1, 1, 0, 0 };
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_PORTS_NUM_CNS] = { 1, 1, 0, 0 };

/****************************** Test packet (ETH) *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},   /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}    /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS              /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {0x600};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3e,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xff,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/***************** Incapsulation ethernet frame **********************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartInt =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x25}     /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPartInt =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS              /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartInt = {0x0600};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    /* incapsulation ethernet frame part */
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartInt},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPartInt},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartInt},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS * 2  + \
    TGF_VLAN_TAG_SIZE_CNS  * 2  + \
    TGF_ETHERTYPE_SIZE_CNS * 2  + \
    TGF_IPV4_HEADER_SIZE_CNS    + \
    sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                  /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
    PRV_TGF_INLIF_PORT_MODE_ENT   inlifMode;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfEtherRedirectToEgressWithTsBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfEtherRedirectToEgressWithTsInilifConfigSet function
* @endinternal
*
* @brief   Set inlif configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsInilifConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: Set inlif configuration on port 0 */
    prvTgfTunnelTermInilifConfigurationSet(PRV_TGF_SEND_PORT_IDX_CNS,&prvTgfRestoreCfg.inlifMode);
}

/**
* @internal prvTgfEtherRedirectToEgressWithTsTunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsTunnelConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    CPSS_TUNNEL_TYPE_ENT           tunnelTypeGet;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        tunnelTypeGet = CPSS_TUNNEL_GENERIC_IPV4_E;
    }
    else
    {
        tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
        tunnelTypeGet = CPSS_TUNNEL_X_OVER_IPV4_E; /* eliminate Coverity warning */
    }

    cpssOsMemSet(&tunnelEntry,0,sizeof(tunnelEntry));

    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId           = PRV_TGF_VLANID_6_CNS;
    tunnelEntry.ipv4Cfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.up               = 0;
    tunnelEntry.ipv4Cfg.dscpMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.dscp             = 0;
    tunnelEntry.ipv4Cfg.dontFragmentFlag = GT_FALSE;
    tunnelEntry.ipv4Cfg.ttl              = 33;
    tunnelEntry.ipv4Cfg.autoTunnel       = GT_FALSE;
    tunnelEntry.ipv4Cfg.autoTunnelOffset = 0;
    tunnelEntry.ipv4Cfg.ethType          = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    tunnelEntry.ipv4Cfg.cfi              = 0;
    tunnelEntry.ipv4Cfg.retainCrc        = GT_FALSE;
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    }

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfTunnelDstIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfTunnelSrcIp, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=IPV4_OVER_IPV4 */
    /* AUTODOC:   vlanId=6, ttl=33, ethType=TUNNEL_GRE0_ETHER_TYPE */
    /* AUTODOC:   upMarkMode=dscpMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   DA=88:77:11:11:55:66, srcIp=2.2.3.3, dstIp=1.1.1.2 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex, tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /*** Get a tunnel start entry ***/
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex, &tunnelTypeGet, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_VLANID_6_CNS, tunnelEntry.ipv4Cfg.vlanId,
                                 "tunnelEntry.ipv4Cfg.vlanId: %d", tunnelEntry.ipv4Cfg.vlanId);

    /* Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv4Cfg.macDa.arEther[5]: 0x%2X",
                                 tunnelEntry.ipv4Cfg.macDa.arEther[5]);
}

/**
* @internal prvTgfEtherRedirectToEgressWithTsTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_MAC_MODE_ENT    macMode = PRV_TGF_TTI_MAC_MODE_DA_E;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_STC      ruleAction = {0};

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));

    /* AUTODOC: enable the TTI lookup for TTI_KEY_ETH at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* save the current lookup Mac mode for Ethernet and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_FALSE;
    ruleAction.passengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    ruleAction.copyTtlFromTunnelHeader = GT_FALSE;

    ruleAction.mplsCmd = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
    ruleAction.bridgeBypass = GT_TRUE;

    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;

    ruleAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ruleAction.tunnelStart    = GT_TRUE;
    ruleAction.tunnelStartPtr = prvTgfRouterArpTunnelStartLineIndex;

    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag0VlanId  = 5;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
    ruleAction.keepPreviousQoS = GT_TRUE;
    ruleAction.modifyUpEnable = PRV_TGF_TTI_MODIFY_UP_ENABLE_E;

    ruleAction.userDefinedCpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;

    /* configure TTI rule */
    pattern.eth.common.pclId    = 3;
    pattern.eth.common.srcIsTrunk = GT_FALSE;
    pattern.eth.common.srcPortTrunk = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    cpssOsMemCpy((GT_VOID*)pattern.eth.common.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(pattern.eth.common.mac));
    pattern.eth.common.vid      = PRV_TGF_VLANID_5_CNS;
    pattern.eth.common.isTagged = GT_TRUE;


    /* configure TTI rule mask */
    mask.eth.common.pclId = 0xFFF;
    mask.eth.common.srcIsTrunk = GT_TRUE;
    mask.eth.common.srcPortTrunk = 0xFFFF;
    cpssOsMemSet((GT_VOID*)mask.eth.common.mac.arEther, 0xFF,
                 sizeof(mask.eth.common.mac.arEther));
    mask.eth.common.vid = 0xFFFF;
    mask.eth.common.isTagged = GT_TRUE;

    /* AUTODOC: add TTI rule 2 with: */
    /* AUTODOC:   key TTI_KEY_ETH, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
    /* AUTODOC:   pattern pclId=3, srcPort=0, vid=5 */
    /* AUTODOC:   pattern MAC=00:00:00:00:34:02 */
    /* AUTODOC:   action Tunnel Terminate, PASSENGER_ETHERNET_NO_CRC, port=1 */
    /* AUTODOC:   tag0VlanCmd=DO_NOT_MODIFY, vid=0, tag1VlanCmd=VLAN_MODIFY_UNTAGGED */
    /* AUTODOC:   modifyUpEnable=MODIFY_UP_ENABLE, userDefinedCpuCode=UDP_BC_MIRROR_TRAP3_E */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
}

/**
* @internal prvTgfEtherRedirectToEgressWithTsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portIter    = 0;
    GT_U32          packetIter  = 0;
    GT_U32          numTriggers = 0;
    TGF_VFD_INFO_STC         vfdArray[PRV_TGF_VFD_NUM_CNS];
    GT_U32 packetSize[PRV_TGF_PORTS_NUM_CNS] = {0};
    CPSS_INTERFACE_INFO_STC  portInterface;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* prepare cupture table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 3 Ethernet over IPv4 tunneled packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    /* AUTODOC:   passenger DA=00:00:00:00:00:22, SA=00:00:00:00:00:25, VID=5 */
    /* AUTODOC:   passenger EtherType=0x0600 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get 3 Ethernet over IPv4 packets on port 1 with: */
    /* AUTODOC:   DA=88:77:11:11:55:66, SA=00:00:00:00:00:05, VID=6 */
    /* AUTODOC:   srcIP=2.2.3.3, dstIP=1.1.1.2 */
    /* AUTODOC:   EtherType=0x6558 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   passenger srcIP=10.10.10.10, dstIP=4.4.4.4 */
    /* AUTODOC:   passenger DA=00:00:00:00:00:22, SA=00:00:00:00:00:25, VID=5 */
    /* AUTODOC:   passenger EtherType=0x0600 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            /* if this is Puma device */

            /* calculate packet length */

            packetSize[0] = PRV_TGF_PACKET_LEN_CNS;
            packetSize[1] = PRV_TGF_PACKET_LEN_CNS +
                            TGF_L2_HEADER_SIZE_CNS +
                            TGF_VLAN_TAG_SIZE_CNS +
                            TGF_ETHERTYPE_SIZE_CNS +
                            TGF_IPV4_HEADER_SIZE_CNS +
                            TGF_GRE_HEADER_SIZE_CNS;
        }
        else
        {
            /* calculate packet length */
            packetSize[0] = PRV_TGF_PACKET_LEN_CNS;
            packetSize[1] = PRV_TGF_PACKET_LEN_CNS +
                            TGF_L2_HEADER_SIZE_CNS +
                            TGF_ETHERTYPE_SIZE_CNS +
                            TGF_IPV4_HEADER_SIZE_CNS +
                            TGF_GRE_HEADER_SIZE_CNS;
        }


        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    prvTgfPacketsCountTxArr[portIter],
                                    prvTgfPacketsCountRxArr[portIter],
                                    packetSize[portIter], prvTgfBurstCount);


        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* check da mac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset     = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));

    /* check ethertype */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset     = sizeof(TGF_MAC_ADDR)*2 + TGF_VLAN_TAG_SIZE_CNS;
    vfdArray[1].cycleCount = TGF_ETHERTYPE_SIZE_CNS;

    vfdArray[1].patternPtr[0] = (GT_U8)((TGF_ETHERTYPE_0800_IPV4_TAG_CNS >> 8) & 0xFF);
    vfdArray[1].patternPtr[1] = (GT_U8)(TGF_ETHERTYPE_0800_IPV4_TAG_CNS & 0xFF);

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                PRV_TGF_VFD_NUM_CNS, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                    portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
    {
        for(packetIter = 0; packetIter < prvTgfBurstCount; packetIter++)
        {

            /* check if captured packet has all correct vfd's */
            UTF_VERIFY_EQUAL1_STRING_MAC((1 << PRV_TGF_VFD_NUM_CNS) - 1,
                                         numTriggers & ((1 << PRV_TGF_VFD_NUM_CNS) - 1),
                                         "   Errors while triggers summary result checking: packetIter =  %d\n", packetIter);

            /* check if captured packet has the same MAC DA as prvTgfArpMac */
            UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, numTriggers & BIT_0,
                                         "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                         prvTgfTunnelMacDa[0],
                                         prvTgfTunnelMacDa[1],
                                         prvTgfTunnelMacDa[2],
                                         prvTgfTunnelMacDa[3],
                                         prvTgfTunnelMacDa[4],
                                         prvTgfTunnelMacDa[5]);

            /* check if captured packet has the same Ethertype as prvTgfArpMac */
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_1) >> 1,
                                         "\n   Ethertype of captured packet must be: %04X",
                                         TGF_ETHERTYPE_0800_IPV4_TAG_CNS);

            /* prepare next iteration */
            numTriggers = numTriggers >> PRV_TGF_VFD_NUM_CNS;
        }
    }
}

/**
* @internal prvTgfEtherRedirectToEgressWithTsConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: invalidate TTI rule 2 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

   /* AUTODOC: restore default inlif mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS, prvTgfRestoreCfg.inlifMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifPortModeSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC_MODE_DA for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for TTI_KEY_ETH at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}


