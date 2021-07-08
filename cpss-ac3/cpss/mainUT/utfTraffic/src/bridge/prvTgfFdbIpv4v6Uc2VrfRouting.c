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
* @file prvTgfFdbIpv4v6Uc2VrfRouting.c
*
* @brief FDB IPv4 and ipv6 UC Routing
*
* @version   3
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

#include <bridge/prvTgfFdbIpv4v6Uc2VrfRouting.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* VLAN 5 Id */
#define PRV_TGF_VLANID_5_CNS           5

/* VLAN 6 Id */
#define PRV_TGF_VLANID_6_CNS           6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS         4

/* port index to send traffic to for ipv4 and ipv6 packet */
#define PRV_TGF_IPV4_SEND_PORT_IDX_CNS 0
#define PRV_TGF_IPV6_SEND_PORT_IDX_CNS 2

/* port index to receive traffic from for ipv4 and ipv6 packet */
#define PRV_TGF_EGR_IPV4_PORT_IDX_CNS  3
#define PRV_TGF_EGR_IPV6_PORT_IDX_CNS  1

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_1_CNS  1
#define PRV_TGF_MAC_TO_ME_INDEX_2_CNS  2

#define PRV_TGF_COUNT_SET_CNS          CPSS_IP_CNT_SET0_E

static GT_U32 prvTgfLpmDBId          = 0;

/* the Virtual Router index for ipv4 and ipv6 packet */
static GT_U32        prvUtfIpv4VrfId = 4;
static GT_U32        prvUtfIpv6VrfId = 6;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32       prvTgfRouterIpv4ArpIndex  = 1;
static GT_U32       prvTgfRouterIpv6ArpIndex  = 2;

/* fdb entry index for ipv4 and ipv6 packet */
static GT_U32       prvTgfIpv4UcFdbEntryIndex;
static GT_U32       prvTgfIpv6UcFdbEntryIndex;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* default next hop packet command used for restore */
static CPSS_PACKET_CMD_ENT prvTgfNhPacketCmdGet;

/* default arp mac addr used for restore for ipv4 and ipv6 packet */
static GT_ETHERADDR        prvTgfIpv4ArpMacAddrGet;
static GT_ETHERADDR        prvTgfIpv6ArpMacAddrGet;

/* default hash mode used for restore */
static PRV_TGF_MAC_HASH_FUNC_MODE_ENT   prvTgfHashModeGet;

/* default on port routing state used for restore for ipv4 and ipv6 packet */
static GT_BOOL              prvTgfIpv4UcPortRoutingEnableGet;
static GT_BOOL              prvTgfIpv6UcPortRoutingEnableGet;

/* default on port DIP lookup state used for restore for ipv4 and ipv6 packet */
static GT_BOOL              prvTgfIpv4FdbRoutingPortEnableGet;
static GT_BOOL              prvTgfIpv6FdbRoutingPortEnableGet;

/******************************* Test IPv4 packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                 /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                       /* pri, cfi, VlanId */
};
/* packet's ipv4 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's ipv6 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfIpv4ArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfIpv6ArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x77};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfIpv4PacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************* Test IPv6 packet **********************************/

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfIpv6PacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_6_CNS                       /* pri, cfi, VlanId */
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfIpv6PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketIpv6PayloadPart = {
    sizeof(prvTgfIpv6PayloadDataArr),                       /* dataLength */
    prvTgfIpv6PayloadDataArr                                /* dataPtr */
};

/* PARTS of IPv6 packet */
static TGF_PACKET_PART_STC prvTgfIpv6PacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfIpv6PacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv6PayloadPart}
};

/**
* @internal prvTgfFdbIpv4v6Uc2VrfRoutingVlanConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
static GT_VOID prvTgfFdbIpv4v6Uc2VrfRoutingVlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    GT_U8                    tagArray[] = {1, 1, 1, 1};
    PRV_TGF_TTI_MAC_VLAN_STC macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC macToMeMask;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS,
            prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_VLANID_5_CNS;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table entry index 1: */
    /* AUTODOC:   DA=00:00:00:00:34:02, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_1_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");

    macToMePattern.vlanId = PRV_TGF_VLANID_6_CNS;
    /* AUTODOC: config MAC to ME table entry index 1: */
    /* AUTODOC:   DA=00:00:00:00:34:02, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_2_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");

}

/**
* @internal prvTgfFdbIpv4v6Uc2VrfRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] macEntryPtr              - (pointer to) mac entry
*                                       None
*/
static GT_VOID prvTgfFdbIpv4v6Uc2VrfRoutingRouteConfigurationSet
(
    PRV_TGF_BRG_MAC_ENTRY_STC *macEntryPtr
)
{
    GT_STATUS rc = GT_OK;

    rc =  prvTgfBrgFdbMacEntryIndexFind(&(macEntryPtr->key),&prvTgfIpv4UcFdbEntryIndex);
    if (rc == GT_NOT_FOUND)
    {
        rc = GT_OK;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind_MultiHash: %d", prvTgfDevNum);

    rc = prvTgfBrgFdbMacEntryWrite(prvTgfIpv4UcFdbEntryIndex, GT_FALSE, macEntryPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    rc =  prvTgfBrgFdbBankCounterUpdate(prvTgfDevNum, prvTgfIpv4UcFdbEntryIndex%16,GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterUpdate: %d", prvTgfIpv4UcFdbEntryIndex%16);
}

/**
* @internal prvTgfFdbIpv4v6Uc2VrfGenericIpv4RoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Ipv4 Route Configuration
*/
static GT_VOID prvTgfFdbIpv4v6Uc2VrfGenericIpv4RoutingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: get current  CRC HASH mode */
    rc =  prvTgfBrgFdbHashModeGet(prvTgfDevNum, &prvTgfHashModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeGet");

    /* AUTODOC: set CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,
                                  PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    /* AUTODOC: get enable Unicast IPv4 Routing status on port 0 */
    rc = prvTgfIpPortRoutingEnableGet(PRV_TGF_IPV4_SEND_PORT_IDX_CNS,
                                      CPSS_IP_UNICAST_E,
                                      CPSS_IP_PROTOCOL_IPV4_E,
                                      &prvTgfIpv4UcPortRoutingEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnableGet");

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_IPV4_SEND_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_IPV4_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Get enable/disable status of  using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableGet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_IPV4_SEND_PORT_IDX_CNS],
                                              CPSS_IP_PROTOCOL_IPV4_E,
                                              &prvTgfIpv4FdbRoutingPortEnableGet );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableGet");

    /* AUTODOC: Enable using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_IPV4_SEND_PORT_IDX_CNS],
                                              CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_IPV4_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_5_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);
}

/**
* @internal prvTgfFdbIpv4v6Uc2VrfGenericIpv6RoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Ipv6 Route Configuration
*/
static GT_VOID prvTgfFdbIpv4v6Uc2VrfGenericIpv6RoutingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: get enable Unicast IPv4 Routing status on port 2 */
    rc = prvTgfIpPortRoutingEnableGet(PRV_TGF_IPV6_SEND_PORT_IDX_CNS,
                                      CPSS_IP_UNICAST_E,
                                      CPSS_IP_PROTOCOL_IPV6_E,
                                      &prvTgfIpv6UcPortRoutingEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnableGet");

    /* AUTODOC: Get enable/disable status of using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableGet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_IPV6_SEND_PORT_IDX_CNS],
                                              CPSS_IP_PROTOCOL_IPV6_E,
                                              &prvTgfIpv6FdbRoutingPortEnableGet );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableGet");

    /* AUTODOC: enable Unicast IPv6 Routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_IPV6_SEND_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_IPV6_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Enable using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_IPV6_SEND_PORT_IDX_CNS],
                                              CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_IPV6_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv6 Unicast Routing on Vlan 6 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_6_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS);
}

/**
* @internal prvTgfFdbBasicIpv4Ipv6Uc2VrfRoutingIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Ipv4 FDB Route Configuration
*/
static GT_VOID prvTgfFdbBasicIpv4Ipv6Uc2VrfRoutingIpv4RouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defUcRouteEntryInfo;

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfIpv4ArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, prvTgfRouterIpv4ArpIndex, &prvTgfIpv4ArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterIpv4ArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: get Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdGet(prvTgfDevNum, &prvTgfNhPacketCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdGet");

    /* AUTODOC: set Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum, CPSS_PACKET_CMD_ROUTE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_VLANID_5_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_VLANID_5_CNS, prvUtfIpv4VrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    /* AUTODOC: clear entry key */
    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));

    /* AUTODOC: fill a destination IP address for the prefix */
    cpssOsMemCpy(entryKey.key.ipv4Unicast.dip, prvTgfPacketIpv4Part.dstAddr, sizeof(entryKey.key.ipv4Unicast.dip));

    /* set key virtual router */
    entryKey.key.ipv4Unicast.vrfId = prvUtfIpv4VrfId;

    /* AUTODOC: set  ipv4 uc route entry in FDB*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(&macEntry.key.key.ipv4Unicast, &entryKey.key.ipv4Unicast,sizeof(macEntry.key.key.ipv4Unicast));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    macEntry.fdbRoutingInfo.ttlHopLimitDecEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirror = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex = 0;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.qosProfileIndex = 0;
    macEntry.fdbRoutingInfo.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.countSet = PRV_TGF_COUNT_SET_CNS;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.dipAccessLevel = 0;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.mtuProfileIndex = 0;
    macEntry.fdbRoutingInfo.isTunnelStart = GT_FALSE;
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_VLANID_6_CNS;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer = 0;
    macEntry.fdbRoutingInfo.nextHopARPPointer = prvTgfRouterIpv4ArpIndex;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_IPV4_PORT_IDX_CNS];

    prvTgfFdbIpv4v6Uc2VrfRoutingRouteConfigurationSet(&macEntry);

    cpssOsMemSet(&defUcRouteEntryInfo, 0, sizeof(defUcRouteEntryInfo));

    /* AUTODOC: create Virtual Routers with default route entries. */
    rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfIpv4VrfId,
                                     &defUcRouteEntryInfo, NULL, NULL, NULL, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd");
}

/**
* @internal prvTgfFdbBasicIpv4Ipv6Uc2VrfRoutingIpv6RouteConfigurationSet function
* @endinternal
*
* @brief   Set Ipv6 FDB Route Configuration
*/
static GT_VOID prvTgfFdbBasicIpv4Ipv6Uc2VrfRoutingIpv6RouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            ipv6ArpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    GT_U32                                  nextHopDataBankNumber;
    GT_U32                                  fdbEntryIndex;
    GT_U32                                  offset = 0;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defUcRouteEntryInfo;

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:77 to the Router ARP Table */
    cpssOsMemCpy(ipv6ArpMacAddr.arEther, prvTgfIpv6ArpMac, sizeof(TGF_MAC_ADDR));

    /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum,prvTgfRouterIpv6ArpIndex,&prvTgfIpv6ArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterIpv6ArpIndex, &ipv6ArpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_VLANID_6_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_VLANID_6_CNS, prvUtfIpv6VrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS);

    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));

    /* fill a destination IP address for the prefix */
    for (offset = 0; offset < 8; offset++)
    {
        entryKey.key.ipv6Unicast.dip[offset << 1]       = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[offset] >> 8);
        entryKey.key.ipv6Unicast.dip[(offset << 1) + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[offset];
    }

    /* set key virtual router */
    entryKey.key.ipv6Unicast.vrfId = prvUtfIpv6VrfId;

    /* AUTODOC: calculate index for ipv6 uc route data entry in FDB */

    /* set entry type */
    entryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    rc =  prvTgfBrgFdbMacEntryIndexFind(&entryKey,&fdbEntryIndex); /* data */
    if (rc == GT_NOT_FOUND)
    {
        rc = GT_OK;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind: %d",
                                 fdbEntryIndex);

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        prvTgfIpv6UcFdbEntryIndex = fdbEntryIndex;
        nextHopDataBankNumber = prvTgfIpv6UcFdbEntryIndex % 16;
    }
    else
    {
        /* On SIP-6, IPV6 UC Address entry must be in even bank, and Data entry immediately follows */
        UTF_VERIFY_EQUAL1_STRING_MAC(0, fdbEntryIndex % 2, "prvTgfBrgFdbMacEntryIndexFind: %d", fdbEntryIndex);
        prvTgfIpv6UcFdbEntryIndex = fdbEntryIndex + 1;

        nextHopDataBankNumber = 0;
    }

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(&macEntry.key.key.ipv6Unicast, &entryKey.key.ipv6Unicast,sizeof(macEntry.key.key.ipv6Unicast));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
    macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    macEntry.fdbRoutingInfo.ttlHopLimitDecEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirror = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex = 0;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.qosProfileIndex = 0;
    macEntry.fdbRoutingInfo.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.countSet = PRV_TGF_COUNT_SET_CNS;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.dipAccessLevel = 0;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.mtuProfileIndex = 0;
    macEntry.fdbRoutingInfo.isTunnelStart = GT_FALSE;
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_VLANID_5_CNS;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer = 0;
    macEntry.fdbRoutingInfo.nextHopARPPointer = prvTgfRouterIpv6ArpIndex;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_IPV6_PORT_IDX_CNS];
    macEntry.fdbRoutingInfo.nextHopDataBankNumber = 0;
    macEntry.fdbRoutingInfo.scopeCheckingEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;

    /* AUTODOC: Write the new entry in Hardware MAC address table in specified index */
    rc = prvTgfBrgFdbMacEntryWrite(prvTgfIpv6UcFdbEntryIndex, GT_FALSE, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    rc =  prvTgfBrgFdbBankCounterUpdate(prvTgfDevNum, (prvTgfIpv6UcFdbEntryIndex % 16), GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterUpdate: %d", (prvTgfIpv6UcFdbEntryIndex)%16);

    /* AUTODOC: calculate index for ipv6 uc route address entry in FDB */

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        /* set entry type */
        entryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
        rc =  prvTgfBrgFdbMacEntryIndexFind(&entryKey,&fdbEntryIndex); /* address */
        if (rc == GT_NOT_FOUND)
        {
            rc = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind: %d", fdbEntryIndex);
    }

    prvTgfIpv6UcFdbEntryIndex = fdbEntryIndex;

    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    macEntry.fdbRoutingInfo.nextHopDataBankNumber = nextHopDataBankNumber;

    /* AUTODOC: Write the new entry in Hardware MAC address table in specified index */
    rc = prvTgfBrgFdbMacEntryWrite(prvTgfIpv6UcFdbEntryIndex, GT_FALSE, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    rc =  prvTgfBrgFdbBankCounterUpdate(prvTgfDevNum, (prvTgfIpv6UcFdbEntryIndex % 16), GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterUpdate: %d", (prvTgfIpv6UcFdbEntryIndex)%16);

    cpssOsMemSet(&defUcRouteEntryInfo, 0, sizeof(defUcRouteEntryInfo));

    /* AUTODOC: create Virtual Router with default route entries */
    rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfIpv6VrfId,
                                     NULL, &defUcRouteEntryInfo, NULL, NULL, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd");
}

/**
* @internal prvTgfFdbIpv4v6Uc2VrfRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*                                       None
*/
static GT_VOID prvTgfFdbIpv4v6Uc2VrfRoutingTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    TGF_MAC_ADDR                    arpMacAddr;
    GT_U32       egressPortIndex = 0xFF;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        if(prvTgfPortsArray[portIter] == nextHopPortNum)
        {
            egressPortIndex = portIter;
            break;
        }
    }

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     *
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP couters and set ROUTE_ENTRY mode */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* setup next hop portInterface for capturing */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = nextHopPortNum;

    /* enable capture on next hop port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, TGF_CAPTURE_MODE_MIRRORING_E);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    switch (egressPortIndex)
    {
        case PRV_TGF_EGR_IPV6_PORT_IDX_CNS:
            partsCount = sizeof(prvTgfIpv6PacketPartArray) / sizeof(prvTgfIpv6PacketPartArray[0]);
            packetPartsPtr = prvTgfIpv6PacketPartArray;
        break;
        default:
            partsCount = sizeof(prvTgfIpv4PacketPartArray) / sizeof(prvTgfIpv4PacketPartArray[0]);
            packetPartsPtr = prvTgfIpv4PacketPartArray;
    }

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=1.1.1.1 */
    /* AUTODOC:   dstIP=1.1.1.3 for regular test, 1.1.1.3 or 3.2.2.3 for bulk test */

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);
    cpssOsTimerWkAfter(10);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if ( (prvTgfPortsArray[portIter] == sendPortNum) ||
                              (portIter  == egressPortIndex))
        {
            /* packetSize is not changed */
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
        }
        else
        {
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

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

    /* get trigger counters where packet has MAC DA as prvTgfIpv4ArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

    switch (egressPortIndex)
    {
        case PRV_TGF_EGR_IPV4_PORT_IDX_CNS:
            cpssOsMemCpy(arpMacAddr, &prvTgfIpv4ArpMac, sizeof(TGF_MAC_ADDR));
            cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));
            break;
        default: /* PRV_TGF_EGR_IPV6_PORT_IDX_CNS */
            cpssOsMemCpy(arpMacAddr, &prvTgfIpv6ArpMac, sizeof(TGF_MAC_ADDR));
            cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));
    }

    /* disable capture on nexthope port , before check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, TGF_CAPTURE_MODE_MIRRORING_E);

    /* check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
        /* number of triggers should be according to number of transmit*/
        UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                arpMacAddr[0], arpMacAddr[1], arpMacAddr[2],
                arpMacAddr[3], arpMacAddr[4], arpMacAddr[5]);

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* disable the PCL exclude vid capturing */
    rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_FALSE,0/*don't care*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                 prvTgfDevNum, GT_FALSE);
}

/**
* @internal prvTgfFdbIpv4v6Uc2VrfGenericRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore generic routing test configuration
*/
static GT_VOID prvTgfFdbIpv4v6Uc2VrfGenericRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                 rc = GT_OK;
    PRV_TGF_TTI_MAC_VLAN_STC  macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC  macToMeMask;

    /* AUTODOC: RESTORE GENERIC ROUTING CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore generic routing config: ==================\n\n");

    /* AUTODOC: remove virtual routers*/
    rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfIpv4VrfId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpLpmVirtualRouterDel: %d",
                                 prvTgfDevNum);

    rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfIpv6VrfId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpLpmVirtualRouterDel: %d",
                                 prvTgfDevNum);

    /* AUTODOC: restore CRC  HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,prvTgfHashModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    /* AUTODOC: restore Unicast IPv4 Routing  enable status on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_IPV4_SEND_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E,
                                   prvTgfIpv4UcPortRoutingEnableGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_IPV4_SEND_PORT_IDX_CNS]);

    /* AUTODOC: restore Unicast IPv6 Routing  enable status on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_IPV6_SEND_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E,
                                   prvTgfIpv6UcPortRoutingEnableGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_IPV6_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_5_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                             prvTgfDevNum,PRV_TGF_VLANID_5_CNS);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 6 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_6_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS);

    /* AUTODOC: restore status of using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_IPV4_SEND_PORT_IDX_CNS],
                                              CPSS_IP_PROTOCOL_IPV4_E,
                                              prvTgfIpv4FdbRoutingPortEnableGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_IPV4_SEND_PORT_IDX_CNS]);

    /* AUTODOC: restore status of using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_IPV6_SEND_PORT_IDX_CNS],
                                              CPSS_IP_PROTOCOL_IPV6_E,
                                              prvTgfIpv6FdbRoutingPortEnableGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_IPV6_SEND_PORT_IDX_CNS]);

    /* invalidate and reset vlan entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate");
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: Invalidate MAC to ME table entry index 1 */

    /* full mask for VLAN and MAC */
    macToMeMask.mac.arEther[0] =
    macToMeMask.mac.arEther[1] =
    macToMeMask.mac.arEther[2] =
    macToMeMask.mac.arEther[3] =
    macToMeMask.mac.arEther[4] =
    macToMeMask.mac.arEther[5] = 0xFF;
    macToMeMask.vlanId = 0xFFF;

    /* use 0 MAC DA and VID = 4095 */
    macToMePattern.mac.arEther[0] =
    macToMePattern.mac.arEther[1] =
    macToMePattern.mac.arEther[2] =
    macToMePattern.mac.arEther[3] =
    macToMePattern.mac.arEther[4] =
    macToMePattern.mac.arEther[5] = 0;
    macToMePattern.vlanId = 0xFFF;

    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_1_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");

    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_2_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");
}

/**
* @internal prvTgfFdbIpv4v6Uc2VrfRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfFdbIpv4v6Uc2VrfRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc                = GT_OK;
    GT_BOOL         ucRouteDelStatus;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore a ARP MAC  to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterIpv4ArpIndex, &prvTgfIpv4ArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore a ARP MAC  to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterIpv6ArpIndex, &prvTgfIpv6ArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet(prvTgfDevNum, prvTgfNhPacketCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* save fdb routing uc delete status*/
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableGet(prvTgfDevNum, &ucRouteDelStatus);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableGet: %d", prvTgfDevNum);

    /* enable fdb uc route entries to be deleted */
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* restore FDB uc route entry delete status */
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableSet(prvTgfDevNum, ucRouteDelStatus);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableSet: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

GT_VOID prvTgfFdbIpv4v6Uc2VrfRouting()
{

    /* Set Base configuration */
    prvTgfFdbIpv4v6Uc2VrfRoutingVlanConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbIpv4v6Uc2VrfGenericIpv4RoutingConfigurationSet();
    prvTgfFdbIpv4v6Uc2VrfGenericIpv6RoutingConfigurationSet();

    /* Set FDB Route configuration */
    prvTgfFdbBasicIpv4Ipv6Uc2VrfRoutingIpv4RouteConfigurationSet();
    prvTgfFdbBasicIpv4Ipv6Uc2VrfRoutingIpv6RouteConfigurationSet();

    /* Send traffic to VLAN 5. Expect routed packet to VLAN 6 */
    prvTgfFdbIpv4v6Uc2VrfRoutingTrafficGenerate(prvTgfPortsArray[PRV_TGF_IPV4_SEND_PORT_IDX_CNS],
                                                prvTgfPortsArray[PRV_TGF_EGR_IPV4_PORT_IDX_CNS]);

    /* Send traffic to VLAN 6. Expect routed packet to VLAN 5 */
    prvTgfFdbIpv4v6Uc2VrfRoutingTrafficGenerate(prvTgfPortsArray[PRV_TGF_IPV6_SEND_PORT_IDX_CNS],
                                                prvTgfPortsArray[PRV_TGF_EGR_IPV6_PORT_IDX_CNS]);

    /* Restore generic route configuration */
    prvTgfFdbIpv4v6Uc2VrfGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv4v6Uc2VrfRoutingConfigurationRestore();
}



