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
* @file prvTgfTunnelIpv4Ecn.c
*
* @brief Tunnel Term: Ipv4 over Ipv4 - Basic
*
* @version   41
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelIpv4Ecn.h>
#include <tunnel/prvTgfTunnelTermIpv4overIpv4.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* virtual bridge VLAN Id */
#define PRV_TGF_VIRTUAL_BRIDGE_VLANID_CNS  0x1005
#define PRV_TGF_VIRTUAL_BRIDGE_VLANID1_CNS 0x1092

#define PRV_TGF_DEFAULT_VLAN_CNS          0x92

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS       1

/* block number */
#define PRV_TGF_CNC0_BLOCK_NUM_CNS        1
#define PRV_TGF_CNC1_BLOCK_NUM_CNS        2

/* counter number */
#define PRV_TGF_CNC0_COUNTER_NUM_CNS      10
#define PRV_TGF_CNC1_COUNTER_NUM_CNS      22

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3
#define PRV_TGF_NEXTHOPE1_PORT_IDX_CNS    2
#define PRV_TGF_NEXTHOPE2_PORT_IDX_CNS    1

#define PRV_TGF_NOT_ECT_CNS               0
#define PRV_TGF_ECT_1_CNS                 1
#define PRV_TGF_ECT_0_CNS                 2
#define PRV_TGF_CE_CNS                    3

#define EXPECTED_CPU_CODE_CNS             511

/* the TTI Rule index */
#define prvTgfTtiRuleIndex  ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum)) ? \
(prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3)) : 1)

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* VLANs array */
static GT_U32         prvTgfVlanArray[3] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS, 0};

/* VLANs array */
static GT_U32         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS
};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* stored default Vlan ID */
static GT_U16        prvTgfDefVlanId = 0;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* the Mask for MAC address */
static TGF_MAC_ADDR  prvTgfFullMaskMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* the Mask for IP address */
static TGF_IPV4_ADDR prvTgfFullMaskIp = {255, 255, 255, 255};

/* the test configuration number */
static GT_U32        prvTgfTestConfigNum        = 0;

/* the Virtual Router index */
static GT_U32        prvUtfVrfId                = 0;

/* The TTI rule and action */
static PRV_TGF_TTI_ACTION_STC   ttiAction = {0};
static PRV_TGF_TTI_RULE_UNT     ttiPattern;
static PRV_TGF_TTI_RULE_UNT     ttiMask;

/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32        prvTgfRouterArpTunnelStartLineIndex = 8;

/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvTgfTunnelMacDa = {0x88, 0x77, 0x11, 0x11, 0x55, 0x66};

/* Tunnel destination IP */
static TGF_IPV4_ADDR prvTgfTunnelDstIp = {1, 1, 1, 2};

/* Tunnel source IP */
static TGF_IPV4_ADDR prvTgfTunnelSrcIp = {2, 2, 3, 3};

/******************************* Test packet **********************************/

/* L2 part of unicast packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* Unicast packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* checksum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* packet's IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OverIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* checksum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of unicast packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OverIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelIpv4EcnBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
* @param[in] testNum                  - test number (each test has it's own configuration)
*/
GT_VOID prvTgfTunnelIpv4EcnBaseConfigurationSet
(
    GT_U32     vrfId,
    GT_U32     testNum
)
{
    GT_BOOL                     isTagged  = GT_FALSE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_U32                      vlanCount = 2;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence;

    /* save test configuration number */
    prvTgfTestConfigNum = testNum;

    /* save virtual router index */
    prvUtfVrfId = vrfId;

    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */
    if (testNum == 0 /* basic test */ )
    {
        /* AUTODOC: get default vlanId */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

        /* AUTODOC: set PVID 5 for port 0 */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    }
    /* AUTODOC: tti rule key is PRV_TGF_TTI_RULE_UDB_30_E */
    else if (testNum == 1 /* check UDB configuarion */)
    {
        /* AUTODOC: get default vlanId */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

        /* AUTODOC: set PVID 5 for port 0 */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    }
     /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E Check CPSS_DXCH_TTI_VLAN_MODIFY_ALL_FOR_VIRTUAL_BRIDGE_E part 1 */
    else if (testNum == 2 /* Check CPSS_DXCH_TTI_VLAN_MODIFY_ALL_FOR_VIRTUAL_BRIDGE_E part 1 */)
    {
        vlanCount = 3;
        prvTgfVlanArray[2] = PRV_TGF_VIRTUAL_BRIDGE_VLANID_CNS;

        prvTgfVlanPerPortArray[2] = PRV_TGF_VIRTUAL_BRIDGE_VLANID_CNS;

        /* AUTODOC: get default vlanId */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

        /* AUTODOC: set PVID 5 for port 0 */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    }
     /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E Check CPSS_DXCH_TTI_VLAN_MODIFY_ALL_FOR_VIRTUAL_BRIDGE_E part 2 */
    else /* Check CPSS_DXCH_TTI_VLAN_MODIFY_ALL_FOR_VIRTUAL_BRIDGE_E part 2 */
    {
        vlanCount = 3;
        prvTgfVlanArray[2] = PRV_TGF_VIRTUAL_BRIDGE_VLANID1_CNS;

        prvTgfVlanPerPortArray[2] = PRV_TGF_VIRTUAL_BRIDGE_VLANID1_CNS;

        /* AUTODOC: get default vlanId */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

        /* AUTODOC: set PVID 0x92 for port 0 */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_DEFAULT_VLAN_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_DEFAULT_VLAN_CNS);

        /* AUTODOC: enable PVID forcing on port 0 */
        rc = prvTgfBrgVlanPortForcePvidEnable(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfBrgVlanPortForcePvidEnable: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    }
    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* AUTODOC: set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

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
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, (GT_U16)prvTgfVlanArray[vlanIter],
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* AUTODOC: add ports to vlan member */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, (GT_U16)prvTgfVlanPerPortArray[portIter],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                     prvTgfPortsArray[portIter], isTagged);
    }

    /* AUTODOC: set SOFT VID Precedence for port 0 */
    precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], precedence);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], precedence);

    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* AUTODOC: create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));


    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.sourceId                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02/01:00:5E:04:04:04 for UC/MC test, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    if (testNum == 2 || testNum == 3)
    {
        /* AUTODOC: enable VLAN based MAC learning for VLAN 0x92 */

        rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_DEFAULT_VLAN_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

        /* AUTODOC: create a macEntry with .daRoute = GT_TRUE */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId         = PRV_TGF_DEFAULT_VLAN_CNS;
        macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
        macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
        macEntry.isStatic                       = GT_TRUE;
        macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.daRoute                        = GT_TRUE;
        macEntry.sourceId                       = 0;
        macEntry.userDefined                    = 0;
        macEntry.daQosIndex                     = 0;
        macEntry.saQosIndex                     = 0;
        macEntry.daSecurityLevel                = 0;
        macEntry.saSecurityLevel                = 0;
        macEntry.appSpecificCpuCode             = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

        /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02/01:00:5E:04:04:04 for UC/MC test, VLAN 5, port 1 */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
    }

    rc = prvTgfTunnelEcnPacketCommandSet(prvTgfDevNum, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelEcnPacketCommandSet: %d", prvTgfDevNum);

    rc = prvTgfTunnelEcnCpuCodeSet(prvTgfDevNum, EXPECTED_CPU_CODE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelEcnCpuCodeSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelIpv4EcnLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfTunnelIpv4EcnLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths;
    GT_U32                                  ii;


    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. AUTODOC: Set Inlif Entry - that needs for Puma devices
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
    inlifEntry.vrfId             = prvUtfVrfId;

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

    /* AUTODOC: call api function */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_EXTERNAL_E, 4096, &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        if (2 == prvTgfTestConfigNum)
        {
            /* AUTODOC: update VRF Id in vlan[PRV_TGF_VIRTUAL_BRIDGE_VLANID_CNS] */
            rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_VIRTUAL_BRIDGE_VLANID_CNS, prvUtfVrfId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                         prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

            /* AUTODOC: enable IPv4 Unicast Routing on Vlan 0x1005 */
            rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VIRTUAL_BRIDGE_VLANID_CNS,
                                           CPSS_IP_UNICAST_E,
                                           CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        }

        if (3 == prvTgfTestConfigNum)
        {
            /* AUTODOC: update VRF Id in vlan[PRV_TGF_VIRTUAL_BRIDGE_VLANID1_CNS] */
            rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_VIRTUAL_BRIDGE_VLANID1_CNS, prvUtfVrfId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                         prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

            /* AUTODOC: enable IPv4 Unicast Routing on Vlan 0x1092 */
            rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VIRTUAL_BRIDGE_VLANID1_CNS,
                                           CPSS_IP_UNICAST_E,
                                           CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        }
    }

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_TRUE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 8;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcRouteEntryInfo, 0,
                         sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* define max number of paths */
        if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            numOfPaths = 0;
        }
        else
        {
            numOfPaths = 1;
        }

        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                ipLttEntryPtr = &defIpv4UcRouteEntryInfo.ipLttEntry;
            }
            if (ii == 1)
            {
                ipLttEntryPtr = &defIpv6UcRouteEntryInfo.ipLttEntry;
            }
            /* set defUcRouteEntryInfo */
            ipLttEntryPtr->routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
            ipLttEntryPtr->numOfPaths               = numOfPaths;
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        }

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. AUTODOC: Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* AUTODOC: fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4OverIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelIpv4EcnPbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*/
static GT_VOID prvTgfTunnelIpv4EcnPbrConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  ii;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *defUcNextHopInfoPtr = NULL;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcNextHopInfo;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");


    /* -------------------------------------------------------------------------
     * 1. AUTODOC: PCL Config
     */

    /* AUTODOC: init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E,/* PBR must be in lookup 1 ! */
        PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. AUTODOC: IP Config
     */

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: write ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        /* set next hop to all 0 and change only values other than 0 */
        cpssOsMemSet(&defIpv4UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* ipv4 uc and ipv6 uc default route */
        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                defUcNextHopInfoPtr = &defIpv4UcNextHopInfo;
            }
            else
            {
                defUcNextHopInfoPtr = &defIpv6UcNextHopInfo;
            }
            defUcNextHopInfoPtr->pclIpUcAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            defUcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
            defUcNextHopInfoPtr->pclIpUcAction.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            defUcNextHopInfoPtr->pclIpUcAction.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute.doIpUcRoute = GT_TRUE;
        }

        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId,
                                         prvUtfVrfId,
                                         &defIpv4UcNextHopInfo,
                                         &defIpv6UcNextHopInfo,
                                         NULL, NULL, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 3. AUTODOC: Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* AUTODOC: fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = CPSS_NET_CONTROL_E;
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = PRV_TGF_NEXTHOPE_VLANID_CNS;
    nextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute         = GT_TRUE;
    nextHopInfo.pclIpUcAction.ipUcRoute.arpDaIndex          = prvTgfRouterArpIndex;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* AUTODOC: fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4OverIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelIpv4EcnRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelIpv4EcnRouteConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            prvTgfTunnelIpv4EcnPbrConfigurationSet();
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            prvTgfTunnelIpv4EcnLttRouteConfigurationSet();
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }
}

/**
* @internal prvTgfTunnelIpv4EcnTtiBasicRule function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] pclId                    - pclId
* @param[in] macAddrPtr               - pointer to mac address
* @param[in] sipPtr                   - pointer to SIP
* @param[in] dipPtr                   - pointer to DIP
* @param[in] vid                      - vlan id
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
*                                       None
*/
GT_VOID prvTgfTunnelIpv4EcnTtiBasicRule
(
    IN GT_U32                       pclId,
    IN TGF_MAC_ADDR                 *macAddrPtr,
    IN TGF_IPV4_ADDR                *sipPtr,
    IN TGF_IPV4_ADDR                *dipPtr,
    IN GT_U16                       vid,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiMaskPtr
)
{
    /* AUTODOC: set TTI Pattern */
    cpssOsMemCpy(ttiPatternPtr->ipv4.common.mac.arEther, macAddrPtr, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(ttiPatternPtr->ipv4.srcIp.arIP, sipPtr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiPatternPtr->ipv4.destIp.arIP, dipPtr, sizeof(TGF_IPV4_ADDR));

    ttiPatternPtr->ipv4.common.pclId            = pclId;
    ttiPatternPtr->ipv4.common.srcIsTrunk       = 0;
    ttiPatternPtr->ipv4.common.srcPortTrunk     = 0;
    ttiPatternPtr->ipv4.common.vid              = vid;
    ttiPatternPtr->ipv4.common.isTagged         = GT_TRUE;
    ttiPatternPtr->ipv4.common.dsaSrcIsTrunk    = GT_FALSE;
    ttiPatternPtr->ipv4.common.dsaSrcPortTrunk  = 0;
    ttiPatternPtr->ipv4.common.dsaSrcDevice     = prvTgfDevNum;
    ttiPatternPtr->ipv4.tunneltype              = 0;
    ttiPatternPtr->ipv4.isArp                   = GT_FALSE;

    /* AUTODOC: set TTI Mask */
    cpssOsMemCpy(ttiMaskPtr, ttiPatternPtr, sizeof(PRV_TGF_TTI_RULE_UNT));
    cpssOsMemCpy(ttiMaskPtr->ipv4.common.mac.arEther, prvTgfFullMaskMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(ttiMaskPtr->ipv4.srcIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(ttiMaskPtr->ipv4.destIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));
    ttiMaskPtr->ipv4.common.srcIsTrunk = 1;
}

/**
* @internal prvTgfTunnelIpv4EcnTtiBasicConfiguration function
* @endinternal
*
* @brief   Set TTI Basic Configuration
*
* @param[in] vid                      - vlan id
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
*                                       None
*/
static GT_VOID prvTgfTunnelIpv4EcnTtiBasicConfiguration
(
    IN GT_U16                       vid,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiMaskPtr
)
{
    GT_STATUS                rc;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */
    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */

    /* AUTODOC: enable the TTI lookup for IPv4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: save the current lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

    /* AUTODOC: set lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: enable IPv4 TTI lookup for only tunneled packets received on port 0 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet");

    /* build basic rule*/
    prvTgfTunnelIpv4EcnTtiBasicRule(
        1,/*pclId*/
        &prvTgfPacketL2Part.daMac,
        &prvTgfPacketIpv4Part.srcAddr,
        &prvTgfPacketIpv4Part.dstAddr,
        vid,
        ttiPatternPtr, ttiMaskPtr);
    return;
}
/**
* @internal prvTgfTunnelIpv4EcnTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] testNum                  - test number (each test has it's own configuration)
*/
GT_VOID prvTgfTunnelIpv4EcnTtiConfigurationSet
(
    IN GT_U32     testNum
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ttiActionGet;
    PRV_TGF_TTI_RULE_UNT     ttiPatternGet;
    PRV_TGF_TTI_RULE_UNT     ttiMaskGet;
    GT_U8                    pclId = 0x1;
    PRV_TGF_TTI_RULE_UNT     ttiPattern_forUdbs;
    PRV_TGF_TTI_RULE_UNT     ttiMask_forUdbs;

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern_forUdbs, 0, sizeof(ttiPattern_forUdbs));
    cpssOsMemSet(&ttiMask_forUdbs, 0, sizeof(ttiMask_forUdbs));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for IPV4 TCAM location */

    /* AUTODOC: set TTI Action */
    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ttiAction.tunnelTerminate               = GT_TRUE;
    ttiAction.passengerPacketType           = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.copyTtlFromTunnelHeader       = GT_FALSE;
    ttiAction.mirrorToIngressAnalyzerEnable = GT_FALSE;
    ttiAction.policerIndex                  = 0;
    ttiAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiAction.vlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    ttiAction.vlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.vlanId                        = 0;
    ttiAction.nestedVlanEnable              = GT_FALSE;
    ttiAction.tunnelStart                   = GT_FALSE;
    ttiAction.tunnelStartPtr                = 0;
    ttiAction.routerLookupPtr               = 0;
    ttiAction.vrfId                         = prvUtfVrfId;
    ttiAction.sourceIdSetEnable             = GT_FALSE;
    ttiAction.sourceId                      = 0;
    ttiAction.bindToPolicer                 = GT_FALSE;
    ttiAction.qosPrecedence                 = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.qosTrustMode                  = PRV_TGF_TTI_QOS_KEEP_PREVIOUS_E;
    ttiAction.qosProfile                    = 0;
    ttiAction.modifyUpEnable                = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
    ttiAction.modifyDscpEnable              = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
    ttiAction.up                            = 0;
    ttiAction.remapDSCP                     = GT_FALSE;
    ttiAction.vntl2Echo                     = GT_FALSE;
    ttiAction.bridgeBypass                  = GT_FALSE;
    ttiAction.actionStop                    = GT_FALSE;
    ttiAction.activateCounter               = GT_FALSE;
    ttiAction.counterIndex                  = 0;

    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */
    if (testNum == 0 /* basic test */ )
    {
        /* AUTODOC: set basic configuration for TTI */
        prvTgfTunnelIpv4EcnTtiBasicConfiguration(PRV_TGF_SEND_VLANID_CNS, &ttiPattern, &ttiMask);

        /* AUTODOC: add TTI rule 1 with IPv4 key on port 0 VLAN 5 with action: */
        /* AUTODOC:   cmd FORWARD */
        /* AUTODOC:   IPv4 passenger packet type */
        /* AUTODOC:   DA=00:00:00:00:34:02/01:00:5E:04:04:04 for UC/MC test */
        /* AUTODOC:   srcIp=10.10.10.10, dstIp=4.4.4.4/230.4.4.4 for UC/MC test */
        rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                              &ttiPattern, &ttiMask, &ttiAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

        /* AUTODOC: check added TTI Rule */
        cpssOsMemSet(&ttiPatternGet, 0, sizeof(ttiPatternGet));
        cpssOsMemSet(&ttiMaskGet, 0, sizeof(ttiMaskGet));
        cpssOsMemSet(&ttiActionGet, 0, sizeof(ttiActionGet));

        rc = prvTgfTtiRuleGet(prvTgfDevNum, prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                              &ttiPatternGet, &ttiMaskGet, &ttiActionGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleGet");

        PRV_UTF_LOG3_MAC(" Pattern&Mask .ipv4.common.vid = %d, %d\n" \
                         " Action .vlanPrecedence = %d\n",
                         ttiPatternGet.ipv4.common.vid,
                         ttiMaskGet.ipv4.common.vid,
                         ttiActionGet.vlanPrecedence);
    }

    /* AUTODOC: tti rule key is PRV_TGF_TTI_RULE_UDB_30_E */
    else    /* testNum == 1 - check UDB configuarion */
    {
        /* AUTODOC: enable the TTI lookup for UDB at the port 0 */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

        /* build legacy ipv4 style rule */
        prvTgfTunnelIpv4EcnTtiBasicRule(
            pclId,
            &prvTgfPacketL2Part.daMac,
            &prvTgfPacketIpv4Part.srcAddr,
            &prvTgfPacketIpv4Part.dstAddr,
            PRV_TGF_SEND_VLANID_CNS,
            &ttiPattern, &ttiMask);
        /* convert the legacy ipv4 pattern/mask to UDBs style pattern/mask */
        rc = prvTgfTunnelTermEmulateLegacyIpv4ForUdbOnlyIpv4PatternMaskBuild(
                &ttiPattern,
                &ttiMask ,
                &ttiPattern_forUdbs ,
                &ttiMask_forUdbs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelTermEmulateLegacyIpv4ForUdbOnlyIpv4PatternMaskBuild: %d", prvTgfDevNum);

        PRV_UTF_LOG0_MAC("======= Setting UDB rule into TCAM =======\n");

        /* AUTODOC: add TTI rule 1 with UDB key on port 0 VLAN 5 with action: */
        /* AUTODOC:   cmd FORWARD */
        /* AUTODOC:   UDB passenger packet type */
        /* AUTODOC:   DA=00:00:00:00:34:02/01:00:5E:04:04:04 for UC/MC test */
        /* AUTODOC:   srcIp=10.10.10.10, dstIp=4.4.4.4/230.4.4.4 for UC/MC test */
        rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_RULE_UDB_30_E,
                              &ttiPattern_forUdbs, &ttiMask_forUdbs, &ttiAction);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

        /* AUTODOC: check added TTI Rule */
        cpssOsMemSet(&ttiPatternGet, 0, sizeof(ttiPatternGet));
        cpssOsMemSet(&ttiMaskGet, 0, sizeof(ttiMaskGet));
        cpssOsMemSet(&ttiActionGet, 0, sizeof(ttiActionGet));

        rc = prvTgfTtiRuleGet(prvTgfDevNum, prvTgfTtiRuleIndex, PRV_TGF_TTI_RULE_UDB_30_E,
                              &ttiPatternGet, &ttiMaskGet, &ttiActionGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleGet: %d", prvTgfDevNum);

        /* set the UDBs that needed for the 'traffic type' ipv4-other to get key like the legacy IPv4 */
        prvTgfTunnelTermEmulateLegacyIpv4ForUdbOnlyIpv4KeyNeededUdbsSet(PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E);

        rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, pclId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

        rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfTunnelIpv4EcnTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] ecnMode        - ECN mode
* @param[in] outerEcn       - ECN of outer packet
* @param[in] passengerEcn   - ECN of passenger packet
*
*/
GT_VOID prvTgfTunnelIpv4EcnTrafficGenerate
(
    PRV_TGF_TUNNEL_START_ECN_MODE_ENT   ecnMode,
    GT_U8                               outerEcn,
    GT_U8                               passengerEcn
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U8                           expectedEcnArray[4][4] = {
                                                                    { PRV_TGF_NOT_ECT_CNS, PRV_TGF_ECT_1_CNS, PRV_TGF_ECT_0_CNS, PRV_TGF_CE_CNS },
                                                                    { PRV_TGF_NOT_ECT_CNS, PRV_TGF_ECT_1_CNS, PRV_TGF_ECT_1_CNS, PRV_TGF_CE_CNS },
                                                                    { PRV_TGF_NOT_ECT_CNS, PRV_TGF_ECT_1_CNS, PRV_TGF_ECT_0_CNS, PRV_TGF_CE_CNS },
                                                                    { PRV_TGF_NOT_ECT_CNS, PRV_TGF_CE_CNS,    PRV_TGF_CE_CNS,    PRV_TGF_CE_CNS }
                                                                 };
    GT_U8                           expectedOuterEcn;
    GT_U8                           expectedPassengerEcn;
    CPSS_PACKET_CMD_ENT             expectedPacketCmd;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;

    /* AUTODOC: GENERATE TRAFFIC: */

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* 1. Setup ECN mode and calculate expected ECN values */
    rc = prvTgfTunnelStartEcnModeSet(prvTgfDevNum, ecnMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEcnModeSet");

    expectedPassengerEcn = expectedEcnArray[outerEcn][passengerEcn];
    if (PRV_TGF_TUNNEL_START_ECN_NORMAL_MODE_E == ecnMode)
        expectedOuterEcn = expectedPassengerEcn;
    else
        expectedOuterEcn = PRV_TGF_NOT_ECT_CNS;

    if (outerEcn == PRV_TGF_CE_CNS && passengerEcn == PRV_TGF_NOT_ECT_CNS)
        expectedPacketCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    else
        expectedPacketCmd = CPSS_PACKET_CMD_FORWARD_E;

    /* -------------------------------------------------------------------------
     * 2. Setup counters and enable capturing
     */

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* AUTODOC: enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

    if (CPSS_PACKET_CMD_TRAP_TO_CPU_E == expectedPacketCmd)
    {
        /* start Rx capture */
        rc = tgfTrafficTableRxStartCapture(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficTableRxStartCapture");
    }

    /* -------------------------------------------------------------------------
     * 3. Generating Traffic
     */

    prvTgfPacketIpv4Part.typeOfService = outerEcn;
    prvTgfPacketIpv4OverIpv4Part.typeOfService = passengerEcn;
    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02/01:00:5E:04:04:04 for UC/MC test, SA=00:00:00:00:00:11 */
    /* AUTODOC:   VID=5, srcIP=10.10.10.10, dstIP=4.4.4.4/230.4.4.4 for UC/MC test */
    /* AUTODOC:   passenger srcIP=2.2.2.2, dstIP=1.1.1.3 */

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 4. Get Ethernet Counters
     */
    /* AUTODOC: verify to get IPv4 packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:06 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.1.1.3 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                        prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:

                /* AUTODOC: packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

        case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:

                /* packetSize is changed. The new packet will be without VLAN_TAG and IPV4_HEADER */
                expectedPacketSize = (packetSize + 2*TGF_CRC_LEN_CNS);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = (expectedPacketCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ? 0 : expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = (expectedPacketCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ? 0 : prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = (expectedPacketCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ? 0 : prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (expectedPacketCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ? 0 : expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = (expectedPacketCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ? 0 : prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = (expectedPacketCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ? 0 : prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

            default:

                /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isOk, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
    * 5. Check CPU code for trapped packets
    */
    if (CPSS_PACKET_CMD_TRAP_TO_CPU_E == expectedPacketCmd)
    {
        /* stop Rx capture */
        rc = tgfTrafficTableRxStartCapture(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficTableRxStartCapture");

        /* get first entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                        GT_TRUE, GT_TRUE, packetBuff,
                                        &buffLen, &packetActualLength,
                                        &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                    "tgfTrafficGeneratorRxInCpuGet");

        PRV_UTF_LOG1_MAC("CPU Code %d\n", rxParam.cpuCode);
        /* check CPU code */
        UTF_VERIFY_EQUAL0_STRING_MAC(EXPECTED_CPU_CODE_CNS, rxParam.cpuCode, "Wrong CPU Code");

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                        GT_FALSE, GT_TRUE, packetBuff,
                                        &buffLen, &packetActualLength,
                                        &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");
        return;
    }

    /* -------------------------------------------------------------------------
    * 6. Get Trigger Counters
    */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* AUTODOC: get trigger counters where packet has MAC DA as prvTgfArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);

    /* AUTODOC: check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    /* AUTODOC: check if captured packet has the same MAC DA as prvTgfArpMac */
    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfTunnelMacDa[0], prvTgfTunnelMacDa[1], prvTgfTunnelMacDa[2],
            prvTgfTunnelMacDa[3], prvTgfTunnelMacDa[4], prvTgfTunnelMacDa[5]);

    /* check outer ECN value */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                       + TGF_ETHERTYPE_SIZE_CNS + 1;
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] = expectedOuterEcn;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
            "\n   ECN value in tunnel IPv4 header must be: 0x%02x",
            vfdArray[0].patternPtr[0]);

    /* check passenger ECN value */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                       + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS + 1;
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] = expectedPassengerEcn;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
            "\n   ECN value in passenger IPv4 header must be: 0x%02x",
            vfdArray[0].patternPtr[0]);

    /* -------------------------------------------------------------------------
     * 7. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfTunnelIpv4EcnTest function
* @endinternal
*
* @brief   Test ECN mode
*
* @param[in] ecnMode    - ECN mode
*
*/
GT_VOID prvTgfTunnelIpv4EcnTest
(
    PRV_TGF_TUNNEL_START_ECN_MODE_ENT    ecnMode
)
{
    GT_U8 outerEcn;
    GT_U8 passengerEcn;

    outerEcn = 3;
    passengerEcn = 0;
    for (outerEcn = 0; outerEcn < 4; outerEcn++)
    {
        for (passengerEcn = 0; passengerEcn < 4; passengerEcn++)
        {
            prvTgfTunnelIpv4EcnTrafficGenerate(ecnMode, outerEcn, passengerEcn);
        }
    }
}


/**
* @internal prvTgfTunnelIpv4EcnConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelIpv4EcnConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = 3;
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 3. Restore TTI Configuration
     */

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the IPv4 TTI lookup for only tunneled packets received on port 0 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore the lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the TTI lookup for IPv4 at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* AUTODOC: fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4OverIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);


    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                    prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        if (prvTgfTestConfigNum == 2)
        {
            /* AUTODOC: disable IPv4 Unicast Routing on Vlan 0x1005 */
            rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VIRTUAL_BRIDGE_VLANID_CNS,
                                            CPSS_IP_UNICAST_E,
                                            CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        }

        if (prvTgfTestConfigNum == 3)
        {
            /* AUTODOC: disable IPv4 Unicast Routing on Vlan 0x1092 */
            rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VIRTUAL_BRIDGE_VLANID1_CNS,
                                            CPSS_IP_UNICAST_E,
                                            CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        }
    }

    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterDel: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: restore default Precedence on port 0 */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);

    /* AUTODOC: invalidate VLANs 5,6, 0x1005, 0x1092 */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* AUTODOC: invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate((GT_U16)prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

     /* AUTODOC: disable PVID forcing on port 0 */
    rc = prvTgfBrgVlanPortForcePvidEnable(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_FALSE);

    /* AUTODOC: set default ECN packet command */
    rc = prvTgfTunnelEcnPacketCommandSet(prvTgfDevNum, CPSS_PACKET_CMD_FORWARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelEcnPacketCommandSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelIpv4EcnIpv4TunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelIpv4EcnIpv4TunnelConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    CPSS_TUNNEL_TYPE_ENT           tunnelTypeGet;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        tunnelTypeGet = CPSS_TUNNEL_GENERIC_IPV4_E;
    }
    else
    {
        tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
        tunnelTypeGet = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
    }

    cpssOsMemSet(&tunnelEntry,0,sizeof(tunnelEntry));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId           = PRV_TGF_NEXTHOPE_VLANID_CNS;
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
    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfTunnelDstIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfTunnelSrcIp, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType=IPV4_OVER_IPV4 */
    /* AUTODOC:   vlanId=6, ttl=33, ethType=TUNNEL_GRE0_ETHER_TYPE */
    /* AUTODOC:   DA=88:77:11:11:55:66, srcIp=2.2.3.3, dstIp=1.1.1.2 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex, tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /*** Get a tunnel start entry ***/
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex, &tunnelTypeGet, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_NEXTHOPE_VLANID_CNS, tunnelEntry.ipv4Cfg.vlanId,
                                 "tunnelEntry.ipv4Cfg.vlanId: %d", tunnelEntry.ipv4Cfg.vlanId);

    /* Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv4Cfg.macDa.arEther[5]: 0x%2X",
                                 tunnelEntry.ipv4Cfg.macDa.arEther[5]);
}
