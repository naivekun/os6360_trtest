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
* @file prvTgfComplicatedPbrIpResize.c
*
* @brief Complicated PBR IP Resize
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <gtOs/gtOsMem.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTcamManagerGen.h>
#include <ip/prvTgfComplicatedPbrIpResize.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOP_VLANID_CNS        6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port index to receive traffic from */
#define PRV_TGF_RCV1_PORT_IDX_CNS         1

/* port index to receive traffic from */
#define PRV_TGF_RCV2_PORT_IDX_CNS         2

/* port index to receive traffic from */
#define PRV_TGF_RCV3_PORT_IDX_CNS         3

/* number of IPv4 prefixes in prefix array */
#define PRV_TGF_IPV4_PREFIX_NUM_CNS       3

/* number of IPv6 prefixes in prefix array */
#define PRV_TGF_IPV6_PREFIX_NUM_CNS       3

/* number of prefixes in prefix array */
#define PRV_TGF_PREFIX_NUM_CNS            (PRV_TGF_IPV4_PREFIX_NUM_CNS + \
                                           PRV_TGF_IPV6_PREFIX_NUM_CNS)

/* number of ports to receive traffic */
#define PRV_TGF_RCV_PORTS_NUM_CNS         3

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the Virtual Router id for LPM Table */
static GT_U32        prvTgfVrId                = 0;

/* (pointer to) TCAM Manager handler */
static GT_PTR        prvTgfTcamMngrHandlerPtr  = NULL;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* array of IPv4 Dst addresses */
static TGF_IPV4_ADDR prvTgfIpv4DstArr[PRV_TGF_IPV4_PREFIX_NUM_CNS + 1] =
{
    {1, 1, 1, 2},
    {1, 1, 1, 3},
    {1, 1, 1, 4},
    {1, 1, 1, 5}
};

/* array of IPv6 Dst addresses */
static TGF_IPV6_ADDR prvTgfIpv6DstArr[PRV_TGF_IPV6_PREFIX_NUM_CNS + 1] =
{
    {0x1122, 0, 0, 0, 0, 0, 0xCCDD, 0xEEFC},
    {0x1122, 0, 0, 0, 0, 0, 0xCCDD, 0xEEFD},
    {0x1122, 0, 0, 0, 0, 0, 0xCCDD, 0xEEFE},
    {0x1122, 0, 0, 0, 0, 0, 0xCCDD, 0xEEFF}
};

/* port indexes to receive traffic */
static GT_U8 prvTgfPortsIdxArr[PRV_TGF_RCV_PORTS_NUM_CNS] =
{
    PRV_TGF_RCV1_PORT_IDX_CNS,
    PRV_TGF_RCV2_PORT_IDX_CNS,
    PRV_TGF_RCV3_PORT_IDX_CNS
};

/* expected number of Rx\Tx packets */
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {6, 2, 2, 2}
};

/**************************** Test IPv4 packet ********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr */
    { 1,  1,  1,  2}                   /* dstAddr */
};

/* DATA of packet len = 22 (0x16) */
static GT_U8 prvTgfIpv4PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketIpv4PayloadPart =
{
    sizeof(prvTgfIpv4PayloadDataArr),                       /* dataLength */
    prvTgfIpv4PayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv4PartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv4PayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_IPV4_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfIpv4PayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketIpv4Info =
{
    PRV_TGF_PACKET_IPV4_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketIpv4PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv4PartArray                                        /* partsArray */
};

/**************************** Test IPv6 packet ********************************/

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part =
{
    6,                                       /* version */
    0,                                       /* trafficClass */
    0,                                       /* flowLabel */
    0x02,                                    /* payloadLen */
    0x3b,                                    /* nextHeader */
    0x40,                                    /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xCCDD, 0xEEFF}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfIpv6PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketIpv6PayloadPart =
{
    sizeof(prvTgfIpv6PayloadDataArr),                       /* dataLength */
    prvTgfIpv6PayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv6PartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv6PayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_IPV6_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfIpv6PayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketIpv6Info =
{
    PRV_TGF_PACKET_IPV6_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketIpv6PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv6PartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U16                                      vid;
} prvTgfRestoreCfg;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfComplicatedPbrIpResizeBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfComplicatedPbrIpResizeBaseConfigSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_TCAM_MANAGER_RANGE_STC  tcamManagerRange;
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntry;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* add external TCAM maneger with max range */
    tcamManagerRange.firstLine = 0;
    tcamManagerRange.lastLine  = 818/*(1 << 18) - 1*/;

    /* AUTODOC: add external TCAM maneger with max range [0..818] */
    rc = prvTgfIpLpmDBExtTcamManagerAdd(prvTgfLpmDBId, &tcamManagerRange,
                                        &prvTgfTcamMngrHandlerPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDBExtTcamManagerAdd: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 5 with untagged port 0 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                           prvTgfPortsArray, NULL, NULL, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports 1,2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
                                           (prvTgfPortsArray + 1), NULL, NULL, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* Save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, &prvTgfRestoreCfg.vid);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* clear macEntry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* set macEntry with .daRoute = GT_TRUE */
    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 0 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfComplicatedPbrIpResizeRouteConfigSet function
* @endinternal
*
* @brief   Set PBR Configuration
*/
GT_VOID prvTgfComplicatedPbrIpResizeRouteConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    GT_U32                                      prefixIter = 0;
    GT_U32                                      byteIter = 0;
    GT_BOOL                                     partitionEnable = GT_FALSE;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmMngrCapcityCfg;
    PRV_TGF_TCAM_MANAGER_RANGE_STC              tcamMngrRange;
    GT_ETHERADDR                                arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        nextHopInfo;
    GT_IPADDR                                   ipv4Addr = {0};
    GT_IPV6ADDR                                 ipv6Addr = {{0}};


    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    /* clear entries */
    cpssOsMemSet(&indexesRange,          0, sizeof(indexesRange));
    cpssOsMemSet(&tcamLpmMngrCapcityCfg, 0, sizeof(tcamLpmMngrCapcityCfg));

    /* get LpmDB capacity */
    rc = prvTgfIpLpmDBCapacityGet(prvTgfLpmDBId,
                                  &partitionEnable,
                                  &indexesRange,
                                  &tcamLpmMngrCapcityCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDBCapacityGet");

    PRV_UTF_LOG0_MAC("\n======= CapacityGet before CapacityUpdate =======\n");
    PRV_UTF_LOG1_MAC("   partitionEnable = %d\n",          partitionEnable);
    PRV_UTF_LOG1_MAC("   indexesRange.firstIndex = %d\n",  indexesRange.firstIndex);
    PRV_UTF_LOG1_MAC("   indexesRange.lastIndex = %d\n",   indexesRange.lastIndex);
    PRV_UTF_LOG1_MAC("   numOfIpv4Prefixes = %d\n",        tcamLpmMngrCapcityCfg.numOfIpv4Prefixes);
    PRV_UTF_LOG1_MAC("   numOfIpv4McSourcePrefixes = %d\n",tcamLpmMngrCapcityCfg.numOfIpv4McSourcePrefixes);
    PRV_UTF_LOG1_MAC("   numOfIpv6Prefixes = %d\n",        tcamLpmMngrCapcityCfg.numOfIpv6Prefixes);

    /* set index ranges and number of prefixes */
    indexesRange.firstIndex = 0;
    indexesRange.lastIndex  = indexesRange.firstIndex + PRV_TGF_PREFIX_NUM_CNS / 4;

    tcamLpmMngrCapcityCfg.numOfIpv4Prefixes         = PRV_TGF_IPV4_PREFIX_NUM_CNS + 1;
    tcamLpmMngrCapcityCfg.numOfIpv4McSourcePrefixes = 0;
    tcamLpmMngrCapcityCfg.numOfIpv6Prefixes         = PRV_TGF_IPV6_PREFIX_NUM_CNS + 1;

    /* AUTODOC: update LpmDB capacity: */
    /* AUTODOC:   index range 0..1 */
    /* AUTODOC:   num IPv4 prefixes 4, num IPv6 prefixes 4  */
    rc = prvTgfIpLpmDBCapacityUpdate(prvTgfLpmDBId,
                                     &indexesRange,
                                     &tcamLpmMngrCapcityCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDBCapacityUpdate");

    /* update TCAM ranges */
    tcamMngrRange.firstLine = 0;
    tcamMngrRange.lastLine  = tcamMngrRange.firstLine + PRV_TGF_PREFIX_NUM_CNS - 1;

    /* AUTODOC: update TCAM ranges to 0..5 */
    rc = prvTgfTcamManagerRangeUpdate(prvTgfTcamMngrHandlerPtr,
                                      &tcamMngrRange,
                                      PRV_TGF_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamManagerRangeUpdate");

    /* get LpmDB capacity */
    rc = prvTgfIpLpmDBCapacityGet(prvTgfLpmDBId,
                                  &partitionEnable,
                                  &indexesRange,
                                  &tcamLpmMngrCapcityCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmDBCapacityGet");

    PRV_UTF_LOG0_MAC("\n======= CapacityGet after CapacityUpdate =======\n");
    PRV_UTF_LOG1_MAC("   partitionEnable = %d\n",          partitionEnable);
    PRV_UTF_LOG1_MAC("   indexesRange.firstIndex = %d\n",  indexesRange.firstIndex);
    PRV_UTF_LOG1_MAC("   indexesRange.lastIndex = %d\n",   indexesRange.lastIndex);
    PRV_UTF_LOG1_MAC("   numOfIpv4Prefixes = %d\n",        tcamLpmMngrCapcityCfg.numOfIpv4Prefixes);
    PRV_UTF_LOG1_MAC("   numOfIpv4McSourcePrefixes = %d\n",tcamLpmMngrCapcityCfg.numOfIpv4McSourcePrefixes);
    PRV_UTF_LOG1_MAC("   numOfIpv6Prefixes = %d\n",        tcamLpmMngrCapcityCfg.numOfIpv6Prefixes);

    PRV_UTF_LOG0_MAC("\n==== Setting PBR Configuration ====\n\n");

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                  CPSS_PCL_LOOKUP_1_E, /* PBR must be in lookup 1 ! */
                                  PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* AUTODOC: enable IPv4\IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* set ARP MAC address */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(arpMacAddr.arEther));

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* clear nexthope info */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    /* set nexthope info for the prefix */
    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = CPSS_NET_CONTROL_E;
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = PRV_TGF_NEXTHOP_VLANID_CNS;
    nextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute         = GT_TRUE;
    nextHopInfo.pclIpUcAction.ipUcRoute.arpDaIndex          = prvTgfRouterArpIndex;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;

    /* AUTODOC: add 3 IPv4 UC prefixes: */
    /* AUTODOC:   1.1.1.2/32, 1.1.1.3/32, 1.1.1.4/32 */
    for (prefixIter = 0; prefixIter < PRV_TGF_IPV4_PREFIX_NUM_CNS; prefixIter++)
    {
        /* set Dst IP address and port */
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[prvTgfPortsIdxArr[prefixIter]];
        cpssOsMemCpy(ipv4Addr.arIP, prvTgfIpv4DstArr[prefixIter], sizeof(ipv4Addr.arIP));

        /* add IPv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvTgfVrId,
                                        ipv4Addr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
    }

    /* AUTODOC: add 3 IPv6 UC prefixes: */
    /* AUTODOC:   1122:0000:0000:0000:0000:0000:CCDD:EEFC/128 */
    /* AUTODOC:   1122:0000:0000:0000:0000:0000:CCDD:EEFD/128 */
    /* AUTODOC:   1122:0000:0000:0000:0000:0000:CCDD:EEFE/128 */
    for (prefixIter = 0; prefixIter < PRV_TGF_IPV6_PREFIX_NUM_CNS; prefixIter++)
    {
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[prvTgfPortsIdxArr[prefixIter]];

        /* set Dst IP address for IPv6 */
        for (byteIter = 0; byteIter < 8; byteIter++)
        {
            ipv6Addr.arIP[byteIter * 2]     = (GT_U8)(prvTgfIpv6DstArr[prefixIter][byteIter] >> 8);
            ipv6Addr.arIP[byteIter * 2 + 1] = (GT_U8) prvTgfIpv6DstArr[prefixIter][byteIter];
        }

        /* add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvTgfVrId,
                                        ipv6Addr, 128, &nextHopInfo,
                                        GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);
    }

    /* set Dst IP address for new prefix */
    cpssOsMemCpy(ipv4Addr.arIP, prvTgfIpv4DstArr[PRV_TGF_IPV4_PREFIX_NUM_CNS],
                 sizeof(ipv4Addr.arIP));

    for (byteIter = 0; byteIter < 8; byteIter++)
    {
        ipv6Addr.arIP[byteIter * 2]     = (GT_U8)(prvTgfIpv6DstArr[PRV_TGF_IPV6_PREFIX_NUM_CNS][byteIter] >> 8);
        ipv6Addr.arIP[byteIter * 2 + 1] = (GT_U8) prvTgfIpv6DstArr[PRV_TGF_IPV6_PREFIX_NUM_CNS][byteIter];
    }

    /* AUTODOC: try to add IPv4 UC prefix -- error LPM DB is full */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvTgfVrId,
                                    ipv4Addr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_FULL, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC: try to add IPv6 UC prefix -- error LPM DB is full */
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvTgfVrId,
                                    ipv6Addr, 128, &nextHopInfo,
                                    GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_FULL, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfComplicatedPbrIpResizeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfComplicatedPbrIpResizeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc          = GT_OK;
    GT_U32                      portIter    = 0;
    GT_U32                      packetIter  = 0;
    GT_U32                      byteIter    = 0;
    GT_U32                      packetLen   = 0;
    GT_U32                      numTriggers = 0;
    GT_U32                      expTriggers = 0;
    CPSS_INTERFACE_INFO_STC     portInterface;
    TGF_VFD_INFO_STC            vfdArray[3] = {{0}};


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* enable capturing */
    for (packetIter = 0; packetIter < PRV_TGF_RCV_PORTS_NUM_CNS; packetIter++)
    {
        /* set port interface */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[prvTgfPortsIdxArr[packetIter]];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfPortsIdxArr[packetIter]]);
    }

    /* setup IPv4 packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketIpv4Info, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    PRV_UTF_LOG0_MAC("======= Generating IPv4 Traffic =======\n");

    /* AUTODOC: send 3 IPv4 packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=1.1.1.1 dstIP=[1.1.1.2|1.1.1.3|1.1.1.4] */
    for (packetIter = 0; packetIter < PRV_TGF_IPV4_PREFIX_NUM_CNS; packetIter++)
    {
        /* set packet Dst IP */
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfIpv4DstArr[packetIter],
                     sizeof(prvTgfPacketIpv4Part.dstAddr));

        /* send packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* setup IPv6 packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketIpv6Info, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    PRV_UTF_LOG0_MAC("======= Generating IPv6 Traffic =======\n");

    /* AUTODOC: send 3 IPv6 packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=6545:0000:0000:0000:0000:0000:0000:3212 */
    /* AUTODOC:   dstIP_1=1122:0000:0000:0000:0000:0000:CCDD:EEFC/128 */
    /* AUTODOC:   dstIP_2=1122:0000:0000:0000:0000:0000:CCDD:EEFD/128 */
    /* AUTODOC:   dstIP_3=1122:0000:0000:0000:0000:0000:CCDD:EEFE/128 */
    for (packetIter = 0; packetIter < PRV_TGF_IPV6_PREFIX_NUM_CNS; packetIter++)
    {
        /* set packet Dst IP */
        cpssOsMemCpy(prvTgfPacketIpv6Part.dstAddr, prvTgfIpv6DstArr[packetIter],
                     sizeof(prvTgfPacketIpv6Part.dstAddr));

        /* send packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    packetIter = PRV_TGF_RCV_PORTS_NUM_CNS;

    /* disable capturing */
    while (packetIter-- > 0)
    {
        /* set port interface */
        portInterface.devPort.portNum = prvTgfPortsArray[prvTgfPortsIdxArr[packetIter]];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfPortsIdxArr[packetIter]]);
    }

    /* check ETH counter of ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* calculate packet length */
        packetLen = prvTgfPacketIpv4Info.totalLen - TGF_VLAN_TAG_SIZE_CNS * (portIter != PRV_TGF_SEND_PORT_IDX_CNS);

        /* AUTODOC: verify traffic:*/
        /* AUTODOC:   3 IPv4 packets - received on ports 1,2,3 */
        /* AUTODOC:   3 IPv6 packets - received on ports 1,2,3 */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    prvTgfPacketsCountRxTxArr[0][portIter],
                                    prvTgfPacketsCountRxTxArr[0][portIter],
                                    packetLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /*
        set expected triggers number -- 110 101:
          - VFD0 should be matcher for Packet0 and Packet1
          - VFD1 should be matcher only for Packet0
          - VFD2 should be matcher only for Packet1
    */
    expTriggers = 43;

    /* set VFD 0 -- check DA MAC */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* set VFD 1 -- cehck IPv4 Dst IP */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +
                         TGF_IPV4_HEADER_SIZE_CNS - sizeof(TGF_IPV4_ADDR);
    vfdArray[1].cycleCount = sizeof(TGF_IPV4_ADDR);

    /* set VFD 2 -- cehck IPv6 Dst IP */
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
    vfdArray[2].modeExtraInfo = 0;
    vfdArray[2].offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +
                         TGF_IPV6_HEADER_SIZE_CNS - sizeof(TGF_IPV6_ADDR);
    vfdArray[2].cycleCount = sizeof(TGF_IPV6_ADDR);

    /* check triggers */
    for (portIter = 0; portIter < PRV_TGF_RCV_PORTS_NUM_CNS; portIter++)
    {
        /* set port interface */
        portInterface.devPort.portNum = prvTgfPortsArray[prvTgfPortsIdxArr[portIter]];

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* set VFD 1 -- cehck IPv4 Dst IP */
        cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfIpv4DstArr[portIter], sizeof(TGF_IPV4_ADDR));

        /* set VFD 2 -- cehck IPv6 Dst IP */
        for (byteIter = 0; byteIter < 8; byteIter++)
        {
            vfdArray[2].patternPtr[byteIter * 2]     = (GT_U8)(prvTgfIpv6DstArr[portIter][byteIter] >> 8);
            vfdArray[2].patternPtr[byteIter * 2 + 1] = (GT_U8) prvTgfIpv6DstArr[portIter][byteIter];
        }

        /* get num of triggers */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, sizeof(vfdArray)/sizeof(vfdArray[0]),
                                                            vfdArray, &numTriggers);
        PRV_UTF_LOG1_MAC("Get numTriggers = %d\n\n", numTriggers);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* compare triggers */
        UTF_VERIFY_EQUAL0_STRING_MAC(expTriggers, numTriggers, "Get wrong number of triggers!");
    }
}

/**
* @internal prvTgfComplicatedPbrIpResizeConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfComplicatedPbrIpResizeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush all UC Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, prvTgfVrId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixesFlush");

    /* AUTODOC: flush all UC Ipv6 prefix */
    rc = prvTgfIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId, prvTgfVrId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixesFlush");

    /* AUTODOC: disable UC IPv4\IPv6 routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);

    /* invalidate SEND VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_SEND_VLANID_CNS);

    /* invalidate nexthop VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_NEXTHOP_VLANID_CNS);

    /* AUTODOC: remove external TCAM Manager */
    rc = prvTgfIpLpmDBExtTcamManagerRemove(prvTgfLpmDBId, prvTgfTcamMngrHandlerPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDBExtTcamManagerRemove");
}


