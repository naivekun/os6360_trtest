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
* @file prvTgfDetailedMimTunnelStart.c
*
* @brief Verify the functionality of UP marking in MIM tunnel start
*
* @version   23
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
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfIpGen.h>
#include <tunnel/prvTgfDetailedMimTunnelStart.h>
#if (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#else
#include <cpss/exMxPm/exMxPmGen/qos/cpssExMxPmQos.h>
#endif


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number  */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* default VLAN Id 5 */
#define PRV_TGF_VLANID_5_CNS        5

/* default VLAN Id 6 */
#define PRV_TGF_VLANID_6_CNS        6

/* MAC in MAC payload size (in bytes) */
#define TGF_MIM_PAYLOAD_SIZE_CNS    4

/* I-TAG size  */
#define TGF_MIM_ITAG_SIZE_CNS       4

/* I-SID size  */
#define TGF_MIM_ISID_SIZE_CNS       3

/* default tunnel start pointer */
#define PRV_TGF_TS_PTR_CNS          1

/* default tunnel start entry index */
#define PRV_TGF_TS_INDEX_CNS        1

/* default TPID table entry index */
#define PRV_TGF_TPID_INDEX_CNS      1

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS       2

/* default number of tunnel start entry extension value */
#define PRV_TGF_TUNNEL_START_SERVISE_ID_CNS 0x123456

/* default number of tunnel start entry extension value */
#define PRV_TGF_EVLAN_SERVISE_ID_CNS        0xAFEFEA

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/*************************** Packet to send ***********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};

/* packet's Ipv4 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    0x4C9D,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* Data of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

/************************* Packet parts ***************************************/

/* PARTS of the packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS                                                 \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +  \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* Packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                           /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]), /* numOfParts */
    prvTgfPacketPartArray                                             /* partsArray */
};


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* MAC for Tunnel Start Entry */
static TGF_MAC_ADDR prvTgfStartEntryMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x11};

/* MAC for Tunnel Start Entry in BDA mode check */
static TGF_MAC_ADDR prvTgfStartEntryBdaMac = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* stored default eVLAN service ID */
static GT_U32   prvTgfDefEVlanServiceId = 0;

/* stored default TS entry */
static struct
{
    GT_U16                         etherType;
} prvTgfDefTunnelStartEntryStc;

/* default MAC in MAC I-Tag Ethertype (802.1ah) */
static GT_U32 prvTgfDefITagEthType;

/* the EtherType to be recognized tagged packets */
static GT_U16 prvTgfDefEtherType0;

/* the EtherType to be recognized tagged packets */
static GT_U16 prvTgfDefEtherType1;

/* stored default inlifMode */
static PRV_TGF_INLIF_PORT_MODE_ENT     inlifMode;

/* stored global default action qos parameters.*/
static PRV_TGF_TTI_ACTION_QOS_STC prvTgfDefaultQosParams;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfBridgeConfigSet function
* @endinternal
*
* @brief   Set initial test settings:
*         - Create VLAN 5 with ports 0, 8
*         - Create VLAN 6 with ports 18, 23
*         - Set VLAN 5 as port 0 default VLAN ID (PVID)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, NULL, prvTgfPortsNum / 2);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, prvTgfPortsNum / 2);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* get default port vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidGet");

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_5_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    return rc;
}

/**
* @internal prvTgfInlifConfigSet function
* @endinternal
*
* @brief   Set initial test settings:
*         - Create inlif on port 0
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfInlifConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;

#ifdef CHX_FAMILY
    /* inlif configuration not relevant for DX */
   return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

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
    inlifEntry.mplsRouteEnable   = GT_FALSE;
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

#endif /* EXMXPM_FAMILY */
}
/**
* @internal prvTgfTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
* @param[in] macMode                  - MAC mode to use
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfTtiConfigSet
(
    IN  PRV_TGF_TTI_MAC_MODE_ENT      macMode
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_STC      ruleAction = {0};

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, macMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeSet");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));
    cpssOsMemSet((GT_VOID*) &pattern,    0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,       0, sizeof(mask));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_FALSE;
    ruleAction.passengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E;
    ruleAction.tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;

    ruleAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ruleAction.tunnelStart    = GT_TRUE;
    ruleAction.tunnelStartPtr = PRV_TGF_TS_PTR_CNS;

    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag0VlanId  = PRV_TGF_VLANID_6_CNS;

    ruleAction.vlanCmd = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ruleAction.vlanId  = PRV_TGF_VLANID_6_CNS;

    ruleAction.modifyUpEnable = PRV_TGF_TTI_MODIFY_UP_ENABLE_E;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;

    ruleAction.userDefinedCpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;
    ruleAction.bridgeBypass       = GT_TRUE;
    ruleAction.ingressPipeBypass  = GT_TRUE;
    ruleAction.actionStop         = GT_TRUE;

    /* configure TTI rule */
    switch (macMode)
    {
        case PRV_TGF_TTI_MAC_MODE_DA_E:
            cpssOsMemCpy((GT_VOID*)pattern.eth.common.mac.arEther,
                         (GT_VOID*)prvTgfPacketL2Part.daMac,
                         sizeof(pattern.eth.common.mac.arEther));
            break;

        case PRV_TGF_TTI_MAC_MODE_SA_E:
            cpssOsMemCpy((GT_VOID*)pattern.eth.common.mac.arEther,
                         (GT_VOID*)prvTgfPacketL2Part.saMac,
                         sizeof(pattern.eth.common.mac.arEther));
            break;

        default:
            rc = GT_BAD_PARAM;
            PRV_UTF_VERIFY_GT_OK(rc, "Get invalid macMode");
    }

    pattern.eth.common.vid      = PRV_TGF_VLANID_5_CNS;
    pattern.eth.common.pclId    = 3;
    pattern.eth.common.isTagged = GT_TRUE;

    /* configure TTI rule mask */
    cpssOsMemSet((GT_VOID*)mask.eth.common.mac.arEther, 0xFF,
                 sizeof(mask.eth.common.mac.arEther));
    cpssOsMemSet((GT_VOID*) &(mask.eth.common.vid), 0xFF,
                 sizeof(mask.eth.common.vid));
    cpssOsMemSet((GT_VOID*) &(mask.eth.common.pclId), 0xFF,
                 sizeof(mask.eth.common.pclId));

    /* AUTODOC: add TTI rule 2 with: */
    /* AUTODOC:   key ETH, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
    /* AUTODOC:   pattern ETH pclId=3, vid=5 */
    /* AUTODOC:   action Tunnel Start, PASSENGER_ETHERNET_CRC, port=3 */
    /* AUTODOC:   enable modift UP, tag1UpCommand TAG1_UP_ASSIGN_VLAN1_UNTAGGED */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                          &pattern, &mask, &ruleAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleSet");

    return rc;
}

/**
* @internal prvTgfTunnelStartConfigSet function
* @endinternal
*
* @brief   Set TS test settings:
*         - Tunnel type: MIM
*         - Tag enable: true
*         - VLAN: 6
*         - UP mark mode: according to entry
*         - UP: 0x3
*         - MAC: 00:00:00:00:00:11
*         - TTL: 64
*         - Retain CRC: true
*         - iSid assign mode: according to entry
*         - iSid: 0x123456
*         - iUP mark mode: according to entry
*         - iUP: 0x5
*         - iDP mark mode: according to entry
*         - iDP: 0
* @param[in] qosType                  - type of QoS
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfTunnelStartConfigSet
(
    IN CPSS_COS_PARAM_ENT       qosType
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT  tsConfig;


    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        /* if this is a puma device */
        rc = prvTgfTunnelStartMimITagEtherTypeGet(prvTgfDevNum,&prvTgfDefITagEthType);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartMimITagEtherTypeGet");

        rc = prvTgfTunnelStartMimITagEtherTypeSet(TGF_ETHERTYPE_88E7_MIM_TAG_CNS);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartMimITagEtherTypeSet");

        /* save default egress EtherType0  */
        prvTgfBrgVlanEgressEtherTypeGet(prvTgfDevNum,CPSS_VLAN_ETHERTYPE0_E,&prvTgfDefEtherType0);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEgressEtherTypeGet");

        /* AUTODOC: set egress EtherType0 0x88A8 */
        prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE0_E,TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEgressEtherTypeSet");

         /* save default egress EtherType1  */
        prvTgfBrgVlanEgressEtherTypeGet(prvTgfDevNum,CPSS_VLAN_ETHERTYPE1_E,&prvTgfDefEtherType1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEgressEtherTypeGet");

        /* AUTODOC: set egress EtherType1 0x88A8 */
        prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEgressEtherTypeSet");

    }
    else
    {
        /* save default egress TPID table entry */
        prvTgfBrgVlanEgressEtherTypeGet(prvTgfDevNum,CPSS_VLAN_ETHERTYPE1_E,&(prvTgfDefTunnelStartEntryStc.etherType));
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEgressEtherTypeGet");

        /* AUTODOC: add egress TPID entry 1 with EtherType 0x88A8 */
        prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEgressEtherTypeSet");

        /* AUTODOC: set TPID index 1 for egress port 3 */
        rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                            CPSS_VLAN_ETHERTYPE0_E, PRV_TGF_TPID_INDEX_CNS);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortEgressTpidSet");
    }

    /* AUTODOC: set MIM EtherType to 0x88E7 */
    rc = prvTgfTtiMimEthTypeSet(TGF_ETHERTYPE_88E7_MIM_TAG_CNS) ;
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMimEthTypeSet");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &tsConfig, 0, sizeof(tsConfig));

    /* configure TTI entry */
    tsConfig.mimCfg.tagEnable      = GT_TRUE;
    tsConfig.mimCfg.vlanId         = PRV_TGF_VLANID_6_CNS;
    tsConfig.mimCfg.up             = 0x3;
    tsConfig.mimCfg.ttl            = 0x40;

#ifdef CHX_FAMILY
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfDevNum,
                                               PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E))
    {
        tsConfig.mimCfg.retainCrc      = GT_FALSE;
    }
    else
    {
        tsConfig.mimCfg.retainCrc      = GT_TRUE;
    }
    tsConfig.mimCfg.upMarkMode     = CPSS_COS_PARAM_UP_E == qosType ? PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E : PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tsConfig.mimCfg.iDpMarkMode    = CPSS_COS_PARAM_DP_E == qosType ? PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E : PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;

#else /* EXMXPM_FAMILY */

    tsConfig.mimCfg.retainCrc      = GT_TRUE;

    tsConfig.mimCfg.upMarkMode     = CPSS_COS_PARAM_UP_E == qosType ? PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E : PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tsConfig.mimCfg.iDpMarkMode    = CPSS_COS_PARAM_DP_E == qosType ? PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E : PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
#endif

    tsConfig.mimCfg.iSidAssignMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tsConfig.mimCfg.iSid           = 0x123456;
    tsConfig.mimCfg.iUpMarkMode    = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tsConfig.mimCfg.iUp            = 0x5;
    tsConfig.mimCfg.iDp            = 0x0;

    cpssOsMemCpy((GT_VOID*) tsConfig.mimCfg.macDa.arEther,
                 (GT_VOID*) prvTgfStartEntryMac,
                 sizeof(tsConfig.mimCfg.macDa.arEther));

    /* set TTI entry */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TS_INDEX_CNS, CPSS_TUNNEL_MAC_IN_MAC_E, &tsConfig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntrySet");

    return rc;
}

/**
* @internal prvTgfTunnelStartTestInit function
* @endinternal
*
* @brief   Set initial test settings:
*         - Create VLAN 5 with ports 0, 8
*         - Create VLAN 6 with ports 18, 23
*         - Set VLAN 5 as port 0 default VLAN ID (PVID)
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*         - Set tunnel start entry
* @param[in] macMode                  - MAC mode to use
* @param[in] qosType                  - type of QoS
*                                       None
*/
static GT_VOID prvTgfTunnelStartTestInit
(
    IN PRV_TGF_TTI_MAC_MODE_ENT       macMode,
    IN CPSS_COS_PARAM_ENT             qosType
)
{
    GT_STATUS               rc         = GT_OK;
    GT_U8                   index      = 0;
    PRV_TGF_COS_PROFILE_STC cosProfile = {0};
    CPSS_QOS_ENTRY_STC      portQosCfg = {0};
    GT_BOOL                 useQosProfile = GT_TRUE;
    PRV_TGF_TTI_ACTION_QOS_STC      defaultQosParams;

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        useQosProfile = GT_FALSE;
    }

    /* set bridge configuration */
    rc = prvTgfBridgeConfigSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBridgeConfigSet: %d", prvTgfDevNum);

    /* set Inlif configuration */
    rc = prvTgfInlifConfigSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifConfigSet: %d", prvTgfDevNum);

    /* set TS configuration */
    rc = prvTgfTunnelStartConfigSet(qosType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartConfigSet: %d", prvTgfDevNum);

    /* set TTI configuration */
    rc = prvTgfTtiConfigSet(macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiConfigSet: %d", prvTgfDevNum);

    /* set QoS profile */
    if (qosType <= CPSS_COS_PARAM_DP_E)
    {
        /* configure cos profile */
        cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
        cosProfile.userPriority   = 0x7;

        /* set cos configuration */
        index = 0;

        if (useQosProfile == GT_TRUE)
        {
            rc = prvTgfCosProfileEntrySet(index, &cosProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);
        }
        else
        {
            rc = prvTgfCosUpToQosParamMapSet(index, &cosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosUpToQosParamMapSet");

            rc = prvTgfCosPortDefaultTcSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],0);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultTcSet");

            rc = prvTgfCosPortDefaultDpSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],CPSS_DP_GREEN_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultDpSet");

            defaultQosParams.exp=0;
            defaultQosParams.qosParams.keepPreviousQoS=GT_TRUE;
            defaultQosParams.qosParams.trustUp=GT_TRUE;
            defaultQosParams.qosParams.trustDscp=GT_TRUE;
            defaultQosParams.qosParams.trustExp=GT_TRUE;
            defaultQosParams.qosParams.enableUpToUpRemapping=GT_TRUE;
            defaultQosParams.qosParams.enableDscpToDscpRemapping=GT_TRUE;
            defaultQosParams.qosParams.enableCfiToDpMapping=GT_TRUE;
            defaultQosParams.qosParams.up=7;
            defaultQosParams.qosParams.dscp=0;
            defaultQosParams.qosParams.tc=0;
            defaultQosParams.qosParams.dp=CPSS_DP_GREEN_E;
            defaultQosParams.qosParams.modifyDscp=GT_FALSE;
            defaultQosParams.qosParams.modifyUp=GT_TRUE;
            defaultQosParams.qosParams.modifyExp=GT_TRUE;
            defaultQosParams.qosParams.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

            /* AUTODOC: save global default action qos parameters.*/
            rc = prvTgfTtiRuleDefaultActionGet(prvTgfDevNum,&prvTgfDefaultQosParams);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleDefaultActionGet: %d", prvTgfDevNum);

            /* AUTODOC: config cos profile with: */
            /* AUTODOC: DP=GREEN, UP=7 TC=DSCP=EXP=0 */
            /* when the action entry format is standard the QOS attributes
               are taken from the global default configuration*/
            rc =prvTgfTtiRuleDefaultActionSet(&defaultQosParams);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleDefaultActionSet: %d", prvTgfDevNum);

        }

        /* configure QoS attributes */
        portQosCfg.qosProfileId     = index;
        portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

        rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &portQosCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet: %d", prvTgfDevNum);

        if (CPSS_COS_PARAM_DP_E == qosType)
        {
            if (useQosProfile == GT_TRUE)
            {
                /* maps Drop Precedence to Drop Eligibility Indicator bit */
                rc = prvTgfCosDpToCfiDeiMapSet(CPSS_DP_GREEN_E, 0);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosDpToCfiDeiMapSet: %d\n", prvTgfDevNum);
            }
        }
    }
}

/**
* @internal prvTgfTunnelStartTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfTunnelStartTestPacketSend
(
    IN  GT_U32                         portNum,
    IN  TGF_PACKET_STC               *packetInfoPtr
)
{
    GT_STATUS               rc            = GT_OK;
    GT_U32                  portsCount    = prvTgfPortsNum;
    GT_U32                  portIter      = 0;


    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* -------------------------------------------------------------------------
     * 1. Sending unknown unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unknown unicast =======\n");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d", prvTgfDevNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfTunnelStartTestReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*/
static GT_VOID prvTgfTunnelStartTestReset
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_QOS_ENTRY_STC              qosEntry;
    PRV_TGF_COS_PROFILE_STC         qosProfile;


    PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_MIM */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MIM_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);


    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        /* if this is a puma device */

        /* AUTODOC: restore default iTagEthType */
        rc = prvTgfTunnelStartMimITagEtherTypeSet(prvTgfDefITagEthType);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartMimITagEtherTypeSet: %d", prvTgfDevNum);

        /* AUTODOC: restore default egress EtherType0 */
        rc =  prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE0_E,prvTgfDefEtherType0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet: %d", prvTgfDevNum);

        /* AUTODOC: restore default egress EtherType1 */
        rc =  prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,prvTgfDefEtherType1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet: %d", prvTgfDevNum);

        /* AUTODOC: restore global default action qos parameters.*/
        rc = prvTgfTtiRuleDefaultActionSet(&prvTgfDefaultQosParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleDefaultActionSet: %d", prvTgfDevNum);

    }
    else
    {
         /* AUTODOC: restore default TPID table entry 1 */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                       PRV_TGF_TPID_INDEX_CNS,
                                       prvTgfDefTunnelStartEntryStc.etherType);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d", prvTgfDevNum);

    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d", prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS);

    /* AUTODOC: restore default inlif port mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,inlifMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfInlifPortModeSet");


    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));

    cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));
    qosEntry.assignPrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    qosEntry.enableModifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.enableModifyUp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.qosProfileId = 0;

    rc = prvTgfCosProfileEntrySet(0, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &qosEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");


    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                                            UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E))
    {
        /* Restore egress e-vlan table service Id */
        rc = prvTgfTunnelStartEgessVlanTableServiceIdSet(prvTgfDevNum, PRV_TGF_VLANID_5_CNS, prvTgfDefEVlanServiceId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEgessVlanTableServiceIdSet");
    }

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelStartUpMarkingModeConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - write QoS profile
*/
GT_VOID prvTgfTunnelStartUpMarkingModeConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* init configuration */
    prvTgfTunnelStartTestInit(PRV_TGF_TTI_MAC_MODE_DA_E, CPSS_COS_PARAM_UP_E);
    /* AUTODOC: add TS entry 1 with: */
    /* AUTODOC:   tunnelType=MAC_IN_MAC */
    /* AUTODOC:   vlanId=6, ttl=0x40 */
    /* AUTODOC:   upMarkMode=MARK_FROM_PACKET_QOS_PROFILE, up=0x3 */
    /* AUTODOC:   iSid=0x123456, iUp=0x5, iDp=0 */
    /* AUTODOC:   DA=00:00:00:00:00:11 */

    /* AUTODOC: config Cos Profile entry 0 with: */
    /* AUTODOC:   DP=GREEN, UP=0x7 */

    /* AUTODOC: config QoS attributes for port 3: */
    /* AUTODOC:   qosProfileId=0 */
    /* AUTODOC:   enable modify UP, disable modify DSCP */
}

/**
* @internal prvTgfTunnelStartUpMarkingModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 MIM tunneled packet:
*         macSa = 00:00:00:00:00:22,
*         macDa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelStartUpMarkingModeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;
    GT_BOOL                         isEqualCntrs = GT_FALSE;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[3];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* configure portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* send packet to be matched */
    /* AUTODOC: send IPv4 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfTunnelStartTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* AUTODOC: verify to get MIM packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:11, SA=00:00:00:00:00:05 */
    /* AUTODOC:   EtherType=0x88A8, UP=7, VID=6 */
    /* AUTODOC:   MIM EtherType=0x88E7 */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* clear expected counters */
        cpssOsMemSet(&expectedCntrs, 0, sizeof(expectedCntrs));

        /* calculate expected counters */
        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            expectedCntrs.goodOctetsSent.l[0] = PRV_TGF_PACKET_CRC_LEN_CNS * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.goodOctetsRcv.l[0]  = PRV_TGF_PACKET_CRC_LEN_CNS * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
        }

        /* calculate expected counters */
        if (PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {

#ifdef CHX_FAMILY
            if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfDevNum,
                                                        PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E))
            {
                expectedCntrs.goodOctetsSent.l[0] = (PRV_TGF_PACKET_CRC_LEN_CNS +
                                                     TGF_CRC_LEN_CNS +
                                                     TGF_L2_HEADER_SIZE_CNS +
                                                     TGF_VLAN_TAG_SIZE_CNS +
                                                     TGF_ETHERTYPE_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.goodOctetsRcv.l[0]  = (PRV_TGF_PACKET_CRC_LEN_CNS +
                                                     TGF_CRC_LEN_CNS +
                                                     TGF_L2_HEADER_SIZE_CNS +
                                                     TGF_VLAN_TAG_SIZE_CNS +
                                                     TGF_ETHERTYPE_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
            }
            else
            {
                expectedCntrs.goodOctetsSent.l[0] = (PRV_TGF_PACKET_CRC_LEN_CNS +
                                                     TGF_L2_HEADER_SIZE_CNS +
                                                     TGF_VLAN_TAG_SIZE_CNS +
                                                     TGF_ETHERTYPE_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.goodOctetsRcv.l[0]  =  (PRV_TGF_PACKET_CRC_LEN_CNS +
                                                     TGF_L2_HEADER_SIZE_CNS +
                                                     TGF_VLAN_TAG_SIZE_CNS +
                                                     TGF_ETHERTYPE_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
            }
#else /* EXMXPM_FAMILY */
            expectedCntrs.goodOctetsSent.l[0] = (PRV_TGF_PACKET_CRC_LEN_CNS +
                                                         TGF_CRC_LEN_CNS +
                                                         TGF_L2_HEADER_SIZE_CNS +
                                                         TGF_VLAN_TAG_SIZE_CNS +
                                                         TGF_ETHERTYPE_SIZE_CNS) * prvTgfBurstCount;
            expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
            expectedCntrs.goodOctetsRcv.l[0]  = (PRV_TGF_PACKET_CRC_LEN_CNS +
                                                 TGF_CRC_LEN_CNS +
                                                 TGF_L2_HEADER_SIZE_CNS +
                                                 TGF_VLAN_TAG_SIZE_CNS +
                                                 TGF_ETHERTYPE_SIZE_CNS) * prvTgfBurstCount;
            expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
            expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
#endif
        }

        /* check Rx\Tx counters */
        PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqualCntrs, GT_TRUE, "get another counters values.");

        /* print expected values if not equal */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
    }

    /* set interface info */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* clear triggers */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* calculate triggers */
    vfdArray[0].mode       = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR));

    vfdArray[1].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].offset        = TGF_L2_HEADER_SIZE_CNS;
    vfdArray[1].cycleCount    = TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[1].patternPtr[0] = (TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS >> 8) & 0xFF;
    vfdArray[1].patternPtr[1] = TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS & 0xFF;

    vfdArray[2].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].offset        = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
    vfdArray[2].cycleCount    = TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[2].patternPtr[0] = (TGF_ETHERTYPE_88E7_MIM_TAG_CNS >> 8) & 0xFF;
    vfdArray[2].patternPtr[1] = TGF_ETHERTYPE_88E7_MIM_TAG_CNS & 0xFF;

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 3, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = (rc == GT_NO_MORE ? GT_OK : rc) || (!(numTriggers == 7));
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, numTriggers);
}

/**
* @internal prvTgfTunnelStartUpMarkingModeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartUpMarkingModeConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfTunnelStartTestReset();
}

/**
* @internal prvTgfTunnelStartMimEtherTypeConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*/
GT_VOID prvTgfTunnelStartMimEtherTypeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* init configuration */
    prvTgfTunnelStartTestInit(PRV_TGF_TTI_MAC_MODE_DA_E, CPSS_COS_PARAM_DP_E + 1);
    /* AUTODOC: add TS entry 1 with: */
    /* AUTODOC:   tunnelType=MAC_IN_MAC */
    /* AUTODOC:   vlanId=6, ttl=0x40 */
    /* AUTODOC:   upMarkMode=MARK_FROM_ENTRY, up=0x3 */
    /* AUTODOC:   iSid=0x123456, iUp=0x5, iDp=0 */
    /* AUTODOC:   DA=00:00:00:00:00:11 */

    /* AUTODOC: set MIM EtherType to 0x88E7 */
    rc = prvTgfTtiMimEthTypeSet(TGF_ETHERTYPE_88E7_MIM_TAG_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMimEthTypeSet: %d\n", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelStartMimEtherTypeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macSa = 00:00:00:00:00:22,
*         macDa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelStartMimEtherTypeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter;
    GT_U32                  packetSize;
    GT_U32                  numTriggers = 0;
    TGF_VFD_INFO_STC        vfdArray[2];
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8 expectedRxPackets[PRV_TGF_PORTS_NUM_CNS] = {1,  0,  0,  1};
    GT_U8 expectedTxPackets[PRV_TGF_PORTS_NUM_CNS] = {1,  0,  0,  1};

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* enable capturing of receive port */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* send packet to be matched */
    /* AUTODOC: send IPv4 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfTunnelStartTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* disable capturing of receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* AUTODOC: verify to get MIM packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:11, SA=00:00:00:00:00:05 */
    /* AUTODOC:   EtherType=0x88A8, UP=3, VID=6 */
    /* AUTODOC:   MIM EtherType=0x88E7 */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        if (PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {

#ifdef CHX_FAMILY
             if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfDevNum,
                                                         PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E))
             {
                /* packet after tunnel start in VLAN 6 */
                packetSize = prvTgfPacketInfo.totalLen + TGF_L2_HEADER_SIZE_CNS
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_MIM_PAYLOAD_SIZE_CNS + TGF_CRC_LEN_CNS;
             }
             else
             {
                 /* packet after tunnel start in VLAN 6 */
                packetSize = prvTgfPacketInfo.totalLen + TGF_L2_HEADER_SIZE_CNS
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_MIM_PAYLOAD_SIZE_CNS;
             }

#else /* EXMXPM_FAMILY */

             /* packet after tunnel start in VLAN 6 */
             packetSize = prvTgfPacketInfo.totalLen + TGF_L2_HEADER_SIZE_CNS
                   + TGF_ETHERTYPE_SIZE_CNS + TGF_MIM_PAYLOAD_SIZE_CNS + TGF_CRC_LEN_CNS;
#endif
        }
        else
        {
            /* original packet before tunneling in VLAN 5 */
            packetSize = prvTgfPacketInfo.totalLen;
        }

        rc = prvTgfEthCountersCheck(
                prvTgfDevNum,
                prvTgfPortsArray[portIter],
                expectedRxPackets[portIter],
                expectedTxPackets[portIter],
                packetSize, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "get another counters values.");
    }

    /* set packet's pattern */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set ether type for 'Service provider vlan tag' */
    vfdArray[0].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset        = TGF_L2_HEADER_SIZE_CNS;
    vfdArray[0].cycleCount    = TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[0].patternPtr[0] = (TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS >> 8) & 0xFF;
    vfdArray[0].patternPtr[1] = TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS & 0xFF;

    /* set ether type for MIM */
    vfdArray[1].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].offset        = TGF_L2_HEADER_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS;
    vfdArray[1].cycleCount    = TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[1].patternPtr[0] = (TGF_ETHERTYPE_88E7_MIM_TAG_CNS >> 8) & 0xFF;
    vfdArray[1].patternPtr[1] = TGF_ETHERTYPE_88E7_MIM_TAG_CNS & 0xFF;

    /* check trigger counters and print captured packets from receive port */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface, 2, vfdArray, &numTriggers);
    UTF_VERIFY_EQUAL5_STRING_MAC(3, numTriggers, "tgfTrafficGenerator"
            "PortTxEthTriggerCountersGet: rc = %d\n"
            "captured packet has fields don't matched with expected fields:\n"
            "    offset_0x%02x: 0x%x\n"
            "    offset_0x%02x: 0x%x",
            rc, TGF_L2_HEADER_SIZE_CNS, TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,
            TGF_L2_HEADER_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS,
            TGF_ETHERTYPE_88E7_MIM_TAG_CNS);
}

/**
* @internal prvTgfTunnelStartMimEtherTypeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartMimEtherTypeConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfTunnelStartTestReset();
}

/**
* @internal prvTgfTunnelStartIdeiMarkingModeConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*/
GT_VOID prvTgfTunnelStartIdeiMarkingModeConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* init configuration */
    prvTgfTunnelStartTestInit(PRV_TGF_TTI_MAC_MODE_DA_E, CPSS_COS_PARAM_DP_E);
    /* AUTODOC: add TS entry 1 with: */
    /* AUTODOC:   tunnelType=MAC_IN_MAC */
    /* AUTODOC:   vlanId=6, ttl=0x40 */
    /* AUTODOC:   upMarkMode=MARK_FROM_ENTRY, up=0x3 */
    /* AUTODOC:   iSid=0x123456, iUp=0x5, iDp=0 */
    /* AUTODOC:   DA=00:00:00:00:00:11 */

    /* AUTODOC: config Cos Profile entry 0 with: */
    /* AUTODOC:   DP=GREEN, UP=0x7 */

    /* AUTODOC: config QoS attributes for port 3: */
    /* AUTODOC:   qosProfileId=0 */
    /* AUTODOC:   enable modify UP, disable modify DSCP */

    /* AUTODOC: map DP GREEN to Drop Eligibility Indicator bit 0 */
}

/**
* @internal prvTgfTunnelStartIdeiMarkingModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macSa = 00:00:00:00:00:22,
*         macDa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelStartIdeiMarkingModeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter;
    GT_U32    packetSize;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8 expectedRxPackets[PRV_TGF_PORTS_NUM_CNS] = {1,  0,  0,  1};
    GT_U8 expectedTxPackets[PRV_TGF_PORTS_NUM_CNS] = {1,  0,  0,  1};

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* enable capturing of receive port */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* AUTODOC: send IPv4 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfTunnelStartTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* disable capturing of receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* AUTODOC: verify to get MIM packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:11, SA=00:00:00:00:00:05 */
    /* AUTODOC:   EtherType=0x88A8, VID=6 */
    /* AUTODOC:   MIM EtherType=0x88E7 */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        if (PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {
#ifdef CHX_FAMILY
            if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfDevNum,
                                                         PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E))
             {
                /* packet after tunnel start in VLAN 6 */
                packetSize = prvTgfPacketInfo.totalLen + TGF_L2_HEADER_SIZE_CNS
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_MIM_PAYLOAD_SIZE_CNS + TGF_CRC_LEN_CNS;
            }
            else
            {
                /* packet after tunnel start in VLAN 6 */
                packetSize = prvTgfPacketInfo.totalLen + TGF_L2_HEADER_SIZE_CNS
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_MIM_PAYLOAD_SIZE_CNS;
            }
#else /* EXMXPM_FAMILY */

           /* packet after tunnel start in VLAN 6 */
            packetSize = prvTgfPacketInfo.totalLen + TGF_L2_HEADER_SIZE_CNS
                + TGF_ETHERTYPE_SIZE_CNS + TGF_MIM_PAYLOAD_SIZE_CNS + TGF_CRC_LEN_CNS;
#endif
        }
        else
        {
            /* original packet before tunneling in VLAN 5 */
            packetSize = prvTgfPacketInfo.totalLen;
        }

        rc = prvTgfEthCountersCheck(
                prvTgfDevNum,
                prvTgfPortsArray[portIter],
                expectedRxPackets[portIter],
                expectedTxPackets[portIter],
                packetSize, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "get another counters values.");
    }

    /* print captured packets from receive port */
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortCapturedPacketPrint: %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);
}

/**
* @internal prvTgfTunnelStartIdeiMarkingModeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartIdeiMarkingModeConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfTunnelStartTestReset();
}

/**
* @internal prvTgfTunnelStartMimMacSaAssignmentConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*/
GT_VOID prvTgfTunnelStartMimMacSaAssignmentConfigSet
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* init configuration */
    prvTgfTunnelStartTestInit(PRV_TGF_TTI_MAC_MODE_DA_E, CPSS_COS_PARAM_DP_E);
}

/**
* @internal prvTgfTunnelStartISidAndBdaConfigSet function
* @endinternal
*
* @brief   Set TS test settings:
*         - Tunnel type: MIM
*         - Tag enable: true
*         - VLAN: 6
*         - UP mark mode: according to entry
*         - UP: 0x3
*         - MAC: 00:00:00:00:00:11
*         - TTL: 64
*         - Retain CRC: true
*         - iSid assign mode: according to entry
*         - iSid: 0x123456
*         - iUP mark mode: according to entry
*         - iUP: 0x5
*         - iDP mark mode: according to entry
*         - iDP: 0
* @param[in] mimISidAssignMode        - tunnel start MIM I-SID assign mode
* @param[in] mimBDaAssignMode         - tunnel start MIM B-DA (Backbone destination address)
*                                      assign mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfTunnelStartISidAndBdaConfigSet
(
        IN PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode,
        IN PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT  tsConfig;


    /* AUTODOC: set MIM EtherType to 0x88E7 */
    rc = prvTgfTtiMimEthTypeSet(TGF_ETHERTYPE_88E7_MIM_TAG_CNS) ;
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMimEthTypeSet");

    /* save default TPID table entry */
    rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_EGRESS_E,
                                   PRV_TGF_TPID_INDEX_CNS,
                                   &(prvTgfDefTunnelStartEntryStc.etherType));
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanTpidEntryGet");

    /* AUTODOC: add egress TPID entry 1 with EtherType 0x88A8 */
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                   PRV_TGF_TPID_INDEX_CNS,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanTpidEntrySet");

    /* AUTODOC: set TPID index 1 for egress port 3 */
    rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        CPSS_VLAN_ETHERTYPE0_E, PRV_TGF_TPID_INDEX_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortEgressTpidSet");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &tsConfig, 0, sizeof(tsConfig));

    /* configure TTI entry */
    tsConfig.mimCfg.tagEnable      = GT_TRUE;
    tsConfig.mimCfg.vlanId         = PRV_TGF_VLANID_6_CNS;
    tsConfig.mimCfg.upMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tsConfig.mimCfg.up             = 0x3;
    tsConfig.mimCfg.ttl            = 0x40;

    tsConfig.mimCfg.retainCrc      = GT_TRUE;


    tsConfig.mimCfg.iSidAssignMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tsConfig.mimCfg.iSid           = PRV_TGF_TUNNEL_START_SERVISE_ID_CNS;
    tsConfig.mimCfg.iUpMarkMode    = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tsConfig.mimCfg.iUp            = 0x5;
    tsConfig.mimCfg.iDpMarkMode    = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tsConfig.mimCfg.iDp            = 0x0;

    tsConfig.mimCfg.mimISidAssignMode = mimISidAssignMode;
    tsConfig.mimCfg.mimBDaAssignMode = mimBDaAssignMode;

    cpssOsMemCpy((GT_VOID*) tsConfig.mimCfg.macDa.arEther,
                 (GT_VOID*) prvTgfStartEntryBdaMac,
                 sizeof(tsConfig.mimCfg.macDa.arEther));

    /* set TTI entry */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TS_INDEX_CNS, CPSS_TUNNEL_MAC_IN_MAC_E, &tsConfig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntrySet");

    /* Get egress vlan table service Id to restore config at the end of the test */
    rc = prvTgfTunnelStartEgessVlanTableServiceIdGet(prvTgfDevNum, PRV_TGF_VLANID_5_CNS, &prvTgfDefEVlanServiceId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEgessVlanTableServiceIdGet");

    /*  AUTODOC: set egress vlan table service Id */
    rc = prvTgfTunnelStartEgessVlanTableServiceIdSet(prvTgfDevNum, PRV_TGF_VLANID_5_CNS, PRV_TGF_EVLAN_SERVISE_ID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEgessVlanTableServiceIdSet");

    return rc;
}

/**
* @internal prvTgfTunnelStartISidAndBdaModeTestInit function
* @endinternal
*
* @brief   Set initial test settings:
*         - Create VLAN 5 with portIdx 0, 1
*         - Create VLAN 6 with portIdx 2, 3
*         - Set VLAN 5 as port 0 default VLAN ID (PVID)
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*         - Set tunnel start entry
* @param[in] mimISidAssignMode        - tunnel start MIM I-SID assign mode
* @param[in] mimBDaAssignMode         - tunnel start MIM B-DA (Backbone destination address)
*                                      assign mode
*                                       None
*/
static GT_VOID prvTgfTunnelStartISidAndBdaModeTestInit
(
    IN PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode,
    IN PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode
)
{
    GT_STATUS               rc         = GT_OK;


    /* set bridge configuration */
    rc = prvTgfBridgeConfigSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBridgeConfigSet: %d", prvTgfDevNum);

    /* set TS configuration */
    rc = prvTgfTunnelStartISidAndBdaConfigSet(mimISidAssignMode, mimBDaAssignMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartISidAndBdaConfigSet: %d", prvTgfDevNum);

    /* set TTI configuration */
    rc = prvTgfTtiConfigSet(PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiConfigSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfTunnelStartMimISidAndBdaSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
* @param[in] mimISidAssignMode        - tunnel start MIM I-SID assign mode
* @param[in] mimBDaAssignMode         - tunnel start MIM B-DA (Backbone destination address)
*                                      assign mode
*                                       None
*/
GT_VOID prvTgfTunnelStartMimISidAndBdaSet
(
    IN PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode,
    IN PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* init configuration */
    prvTgfTunnelStartISidAndBdaModeTestInit(mimISidAssignMode, mimBDaAssignMode);
    /* AUTODOC: add TS entry 1 with: */
    /* AUTODOC:   tunnelType=MAC_IN_MAC */
    /* AUTODOC:   vlanId=6, ttl=0x40 */
    /* AUTODOC:   upMarkMode=MARK_FROM_ENTRY, up=0x3 */
    /* AUTODOC:   iSid=0x123456, iUp=0x5, iDp=0 */
    /* AUTODOC:   DA=00:00:00:00:00:11 */
}

/**
* @internal prvTgfTunnelStartMimISidAndBdaTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macSa = 00:00:00:00:00:22,
*         macDa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelStartMimISidAndBdaTrafficGenerate
(
    IN PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode,
    IN PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter;
    GT_U32    packetSize;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32                  numTriggers = 0;
    TGF_VFD_INFO_STC        vfdArray[2];
    GT_U8 expectedRxPackets[PRV_TGF_PORTS_NUM_CNS] = {1,  0,  0,  1};
    GT_U8 expectedTxPackets[PRV_TGF_PORTS_NUM_CNS] = {1,  0,  0,  1};

    TGF_MAC_ADDR   expectedMac = {0};
    GT_U32         expectedIsidTag = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* enable capturing of receive port */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* AUTODOC: send IPv4 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=5 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfTunnelStartTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* disable capturing of receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* AUTODOC: verify to get MIM packet on port 3 with: */
    /* AUTODOC:   EtherType=0x88A8, VID=6 */
    /* AUTODOC:   MIM EtherType=0x88E7 */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        if (PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {
            /* packet after tunnel start in VLAN 6 */
            packetSize = prvTgfPacketInfo.totalLen + TGF_L2_HEADER_SIZE_CNS
                + TGF_ETHERTYPE_SIZE_CNS + TGF_MIM_PAYLOAD_SIZE_CNS + TGF_CRC_LEN_CNS;
        }
        else
        {
            /* original packet before tunneling in VLAN 5 */
            packetSize = prvTgfPacketInfo.totalLen;
        }

        rc = prvTgfEthCountersCheck(
                prvTgfDevNum,
                prvTgfPortsArray[portIter],
                expectedRxPackets[portIter],
                expectedTxPackets[portIter],
                packetSize, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "get another counters values.");
    }


    /* defining cheked parameters depending on I-SID and BDA issign mode */
    if(mimBDaAssignMode == PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E)
    {
        cpssOsMemCpy(expectedMac, prvTgfStartEntryBdaMac, sizeof(TGF_MAC_ADDR));
    }
    else if(mimBDaAssignMode == PRV_TGF_CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E)
    {
        expectedMac[0] = prvTgfStartEntryBdaMac[0];
        expectedMac[1] = prvTgfStartEntryBdaMac[1];
        expectedMac[2] = prvTgfStartEntryBdaMac[2];
        expectedMac[3] = (PRV_TGF_TUNNEL_START_SERVISE_ID_CNS >> 16) & 0xFF;
        expectedMac[4] = (PRV_TGF_TUNNEL_START_SERVISE_ID_CNS >> 8) & 0xFF;
        expectedMac[5] = (PRV_TGF_TUNNEL_START_SERVISE_ID_CNS) & 0xFF;
    }

    if(mimISidAssignMode == PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E)
    {
        expectedIsidTag = PRV_TGF_TUNNEL_START_SERVISE_ID_CNS;
    }
    else if(mimISidAssignMode == PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E)
    {
        expectedIsidTag = PRV_TGF_EVLAN_SERVISE_ID_CNS;
    }

    /* set packet's pattern */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set ether type for 'Service provider vlan tag' */
    vfdArray[0].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset        = 0;
    vfdArray[0].cycleCount    = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, expectedMac, sizeof(TGF_MAC_ADDR));

    /* set ether type for MIM */
    vfdArray[1].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].offset        = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS +
                                TGF_ETHERTYPE_SIZE_CNS + TGF_MIM_ITAG_SIZE_CNS -
                                TGF_MIM_ISID_SIZE_CNS;
    vfdArray[1].cycleCount    = TGF_MIM_ISID_SIZE_CNS;
    vfdArray[1].patternPtr[0] = (GT_U8)((expectedIsidTag >> 16) & 0xFF);
    vfdArray[1].patternPtr[1] = (GT_U8)((expectedIsidTag >> 8) & 0xFF);
    vfdArray[1].patternPtr[2] = (GT_U8)((expectedIsidTag) & 0xFF);

    /* check trigger counters and print captured packets from receive port */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface, 2, vfdArray, &numTriggers);
    UTF_VERIFY_EQUAL1_STRING_MAC(3, numTriggers, "tgfTrafficGenerator"
            "PortTxEthTriggerCountersGet: rc = %d\n"
            "captured packet has fields don't matched with expected fields:\n", rc);

}

/**
* @internal prvTgfTunnelStartMimISidAndBdaConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartMimISidAndBdaConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc=GT_OK;
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfTunnelStartTestReset();

    /* AUTODOC: set TPID index 0 for egress port 3 */
    rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        CPSS_VLAN_ETHERTYPE0_E, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet");
}


