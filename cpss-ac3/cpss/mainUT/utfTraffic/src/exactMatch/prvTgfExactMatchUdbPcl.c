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
* @file prvTgfExactMatchDefaultActionUdbPcl.c
*
* @brief Test Exact Match Default Action functionality with PCL Action Type
*        and UDB key
*
* @version
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

#include <pcl/prvTgfPclUdbOnlyKeys.h>
#include <exactMatch/prvTgfExactMatchDefaultActionUdbPcl.h>


#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS 5

#if 0
/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   2;


/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE
};
#endif

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfOamGen.h>
#include <pcl/prvTgfPclUdbOnlyKeys.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS         2

/* PCL rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS             8

#define PRV_TGF_PCL_RULE_INDEX_CNS           0 /* shiri - see if can remove PRV_TGF_PCL_RULE_IDX_CNS*/

/* TTI rule index */
#define PRV_TGF_TTI_RULE_IDX_CNS             2

/* ARP Entry index */
#define PRV_TGF_ARP_ENTRY_IDX_CNS            3

/* TTI ID */
#define PRV_TGF_TTI_ID_CNS                   1

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS                   0x55

/* original VLAN Id */
#define PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS   0x10

/* original VLAN Id */
#define PRV_TGF_PASSENGER_TAG1_VLAN_ID_CNS   0x11

/* original VLAN Id */
#define PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS      0x21

/* original VLAN Id */
#define PRV_TGF_TUNNEL_TAG1_VLAN_ID_CNS      0x22

/* lookup1 VLAN Id */
#define PRV_TGF_PCL_NEW_VLAN_ID_CNS          0x31

/* Action Flow Id  */
#define PRV_TGF_TTI_ACTION_FLOW_ID_CNS       0x25

/* Action Flow Id  */
#define PRV_TGF_PCL_ACTION_FLOW_ID_CNS       0x26

/* Router Source MAC_SA index  */
#define PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS  0x12

/* TCP Sorce port  */
#define PRV_TGF_PCL_TCP_SRC_PORT_CNS         0x7654

/* TCP Destination port  */
#define PRV_TGF_PCL_TCP_DST_PORT_CNS         0xFECB

/* UDP Sorce port  */
#define PRV_TGF_PCL_UDP_SRC_PORT_CNS         0x4567

/* UDP Destination port  */
#define PRV_TGF_PCL_UDP_DST_PORT_CNS         0xBCEF

/* IPV6 Sorce IP  */
#define PRV_TGF_PCL_IPV6_SRC_IP_CNS          {0x23, 0x45, 0x67, 0x89, 0x87, 0x65, 0x43, 0x21}

/* IPV6 Destination IP  */
#define PRV_TGF_PCL_IPV6_DST_IP_CNS          {0x01, 0x2E, 0x3D, 0x4C, 0x5B, 0x6A, 0x79, 0x88}

/********************************************************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part =
{
    { 0x00, 0x12, 0x34, 0x56, 0x78, 0x9A },                /* daMac */
    { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 }                 /* saMac */
};

#if 0
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part =
{
    { 0x00, 0xFE, 0xDC, 0xBA, 0x98, 0x76 },                /* daMac */
    { 0x00, 0xBB, 0xAA, 0x11, 0x22, 0x33 }                 /* saMac */
};
#endif
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,              /* etherType */
    0, 0, PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,              /* etherType */
    0, 0, PRV_TGF_PASSENGER_TAG1_VLAN_ID_CNS      /* pri, cfi, VlanId */
};
#if 0
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketTunnelVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,           /* etherType */
    0, 0, PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketTunnelVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,           /* etherType */
    0, 0, PRV_TGF_TUNNEL_TAG1_VLAN_ID_CNS      /* pri, cfi, VlanId */
};


/* LLC Non SNAP Length part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketLlcNonSnapLengthPart =
{
    0x0100
};
#endif

/* Ethernet Type part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOth1EthertypePart =
{
    0x3456
};

#if 0
/* Ethernet Type part of MPLS packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart =
{
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* ethertype part of IPV6 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};
#endif

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

#if 0
/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketPassengerUdpPart =
{
    PRV_TGF_PCL_UDP_SRC_PORT_CNS,                  /* src port */
    PRV_TGF_PCL_UDP_DST_PORT_CNS,                  /* dst port */
    sizeof(prvTgfPacketPayloadDataArr),            /* length   */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS              /* csum     */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketPassengerTcpPart =
{
    PRV_TGF_PCL_TCP_SRC_PORT_CNS,                  /* src port */
    PRV_TGF_PCL_TCP_DST_PORT_CNS,                  /* dst port */
    1,                                             /* sequence number */
    2,                                             /* acknowledgment number */
    0,                                             /* data offset */
    0,                                             /* reserved */
    0x00,                                          /* flags */
    4096,                                          /* window */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    0                   /* urgent pointer */
};

/* packet's IPv6 header */
static TGF_PACKET_IPV6_STC prvTgfPacketPassengerIpv6OtherHeaderPart =
{
    6,                                  /* version */
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    sizeof(prvTgfPacketPayloadDataArr), /* payloadLen */
    0x3b,                               /* nextHeader */
    0x40,                               /* hopLimit */
    PRV_TGF_PCL_IPV6_SRC_IP_CNS,        /* srcAddr */
    PRV_TGF_PCL_IPV6_DST_IP_CNS         /* dstAddr */
};

/* packet's IPv6 header */
static TGF_PACKET_IPV6_STC prvTgfPacketPassengerIpv6UdpHeaderPart =
{
    6,                                                              /* version */
    0,                                                              /* trafficClass */
    0,                                                              /* flowLabel */
    (sizeof(prvTgfPacketPayloadDataArr) + TGF_UDP_HEADER_SIZE_CNS), /* payloadLen */
    TGF_PROTOCOL_UDP_E,                                             /* nextHeader */
    0x40,                                                           /* hopLimit */
    PRV_TGF_PCL_IPV6_SRC_IP_CNS,                                    /* srcAddr */
    PRV_TGF_PCL_IPV6_DST_IP_CNS                                     /* dstAddr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketPassengerIpv4OtherHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    0,                                                               /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)), /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    4,                                                               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    { 21, 22, 23, 24 },                                                /* srcAddr */
    { 1,  1,  1,  3 }                                                 /* dstAddr */
};

/* packet's IPv4 UDP*/
static TGF_PACKET_IPV4_STC prvTgfPacketPassengerIpv4UdpHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    0,                                                               /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    TGF_PROTOCOL_UDP_E,                                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    { 32, 42, 52, 62 },                                                /* srcAddr */
    { 1,  2,  3,  4 }                                                 /* dstAddr */
};


/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketPassengerIpv4TcpHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    0,                                                               /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    TGF_PROTOCOL_TCP_E,                                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    { 32, 42, 52, 62 },                                                /* srcAddr */
    { 1,  2,  3,  4 }                                                 /* dstAddr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketTunnelIpv4OtherHeaderPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    (TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x29,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 16, 17, 18, 19 },   /* srcAddr */
    { 4,  5,  6,  7 }    /* dstAddr */
};
#endif


/* Double Tagged Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketNotTunneled2tagsEthOthPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Double Tagged Ethernet Other Packet Info */
static TGF_PACKET_STC prvTgfPacketNotTunneled2tagsEthOthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketNotTunneled2tagsEthOthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketNotTunneled2tagsEthOthPartsArray                                        /* partsArray */
};

#if 0
/* Double Tagged IPV6 Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketNotTunneled2tagsIpv6OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV6 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketNotTunneled2tagsIpv6OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                   /* totalLen */
    sizeof(prvTgfPacketNotTunneled2tagsIpv6OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketNotTunneled2tagsIpv6OthPartsArray                                        /* partsArray */
};

/* MPLS Tunnel Header */
static TGF_PACKET_MPLS_STC prvTgfPacketTunnelMplsLabel0 =
{
    1111 /* Addressing Label */,
    0/*exp*/,
    1/*stack - last*/,
    100/*timeToLive*/
};

/* Double Tagged MPLS Packet Tunneled IPV4 Other Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketMplsTunneled2tagsIpv4OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketTunnelMplsLabel0},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged MPLS Packet Tunneled IPV4 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketMplsTunneled2tagsIpv4OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_MPLS_HEADER_SIZE_CNS
     + TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketMplsTunneled2tagsIpv4OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMplsTunneled2tagsIpv4OthPartsArray                                        /* partsArray */
};

/* Double Tagged IPV6 Packet Tunneled L2 IPV6 Other Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketIpv4TunneledL2Ipv6OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTunnelIpv4OtherHeaderPart},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV6 Packet Tunneled L2 IPV6 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketIpv4TunneledL2Ipv6OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS
     + TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketIpv4TunneledL2Ipv6OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv4TunneledL2Ipv6OthPartsArray                                        /* partsArray */
};


/* Double Tagged IPV4 Other Passenger Packet Parts Array */
static TGF_PACKET_PART_STC prvTgfPacketPassenger2tagsIpv4OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV4 Other Passenger Packet Info */
static TGF_PACKET_STC prvTgfPacketPassenger2tagsIpv4OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                 /* totalLen */
    sizeof(prvTgfPacketPassenger2tagsIpv4OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassenger2tagsIpv4OthPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV6 Other Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerIpv6OthPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart },
    { TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6OtherHeaderPart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Double Tagged IPV6 Packet Tunneled L2 IPV6 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerIpv6OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerIpv6OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerIpv6OthPartsArray                                        /* partsArray */
};

/* Double Tagged MPLS Tunneled Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketMplsTunneled2tagsEthOthPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart },
    { TGF_PACKET_PART_MPLS_E,      &prvTgfPacketTunnelMplsLabel0 },
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Double Tagged MPLS Tunneled Ethernet Other Packet Info */
static TGF_PACKET_STC prvTgfPacketMplsTunneled2tagsEthOthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_MPLS_HEADER_SIZE_CNS
     + TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketMplsTunneled2tagsEthOthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMplsTunneled2tagsEthOthPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV4 TCP Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPasenger2tagsIpv4TcpPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart },
    { TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4TcpHeaderPart },
    { TGF_PACKET_PART_TCP_E,       &prvTgfPacketPassengerTcpPart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Double Tagged Packet IPV4 TCP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassenger2tagsIpv4TcpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPasenger2tagsIpv4TcpPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPasenger2tagsIpv4TcpPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV4 UDP Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerIpv4UdpPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart },
    { TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4UdpHeaderPart },
    { TGF_PACKET_PART_UDP_E,       &prvTgfPacketPassengerUdpPart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Double Tagged IPV4 Packet UDP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerIpv4UdpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerIpv4UdpPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerIpv4UdpPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV6 UDP Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerIpv6UdpPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart },
    { TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6UdpHeaderPart },
    { TGF_PACKET_PART_UDP_E,       &prvTgfPacketPassengerUdpPart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Double Tagged IPV6 Packet Tunneled L2 IPV6 UDP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerIpv6UdpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerIpv6UdpPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerIpv6UdpPartsArray                                        /* partsArray */
};

/* Single Tagged LLC Non SNAP Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerLlcNonSnapPartsArray[] = {
    { TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part },  /* type, partPtr */
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part },
    { TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part },
    { TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketLlcNonSnapLengthPart },
    { TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart }
};

/* Single Tagged LLC Non SNAP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerLlcNonSnapInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS +
     +sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerLlcNonSnapPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerLlcNonSnapPartsArray                                        /* partsArray */
};
#endif



static GT_U32   currentRuleIndex = 0;

static void setRuleIndex(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum
)
{
    GT_U32  lookupId;

    if(direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        currentRuleIndex =  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_PCL_RULE_IDX_CNS); ;
    }
    else
    {
        lookupId = lookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                             0 ;/*CPSS_PCL_LOOKUP_0_E*/

        currentRuleIndex =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,PRV_TGF_PCL_RULE_IDX_CNS);
    }
}

/**
* @internal prvTgfExactMatchPclUdbMaskPatternBuild function
* @endinternal
*
* @brief   Build UDB : Pattern and Mask according to packet sent
*
* @param[in] udbAmount      - UDB Amount
* @param[in] packetPtr      - (pointer to) Info of the packet used in the test
* @param[in] offsetInPacket - offset of matched data in the packet
* @param[out]udbMaskPtr     - (pointer to) mask
* @param[out]udbPatternPtr  - (pointer to) pattern
*/
GT_VOID prvTgfExactMatchPclUdbMaskPatternBuild
(
    IN  GT_U32                 udbAmount,
    IN  TGF_PACKET_STC         *packetPtr,
    IN  GT_U32                 offsetInPacket,
    OUT GT_U8                  *udbMaskPtr,
    OUT GT_U8                  *udbPatternPtr
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           i;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    GT_U8                            *patternPtr;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */

    numOfBytesInBuff = sizeof(packetBuffer);
    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficEnginePacketBuild FAILED: %d", prvTgfDevNum);

    patternPtr = &(packetBuffer[offsetInPacket]);

    for (i = 0; (i < udbAmount); i++)
    {
        udbMaskPtr[i]    = 0xFF;
        udbPatternPtr[i] = patternPtr[i];
    }
}


/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyGenericTest function
* @endinternal
*
* @brief   Generic Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] offsetType               - offset Type
* @param[in] udbAmount                - UDB Amount
* @param[in] udbIndexLow              - The lowest UDB Index in the UDB range
* @param[in] udbOffsetLow             - UDB Offset Base of the field (the lowest byte offset)
* @param[in] udbOrderAscend           - UDB Order Ascend (GT_TRUE) or Descend (GT_FALSE)
* @param[in] packetPtr                - (pointer to) Info of the packet used in the test
* @param[in] offsetInPacket           - offset of matched data in the packet
*                                       None
*/
GT_VOID prvTgfExactMatchPclUdbOnlyKeysGenericConfig
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexLow,
    IN GT_U32                           udbOffsetLow,
    IN GT_BOOL                          udbOrderAscend,
    IN TGF_PACKET_STC                   *packetPtr,
    IN GT_U32                           offsetInPacket
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    GT_U8                            *udbMaskPtr;
    GT_U8                            *udbPatternPtr;
    GT_U32                           i;
    GT_U32                           udbCfgIndexBase;
    GT_32                            udbCfgOffsetIncrement;
    GT_U32                           udbCfgOffsetBase;
    GT_U32                           udbSelIndexBase;
    GT_32                            udbSelIndexIncrement;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    GT_U8                            *patternPtr;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */

    udbCfgIndexBase       = udbIndexLow;
    if (udbOrderAscend == GT_TRUE)
    {
        udbCfgOffsetBase      = udbOffsetLow;
        udbCfgOffsetIncrement = 1;
        udbSelIndexBase       = udbIndexLow;
        udbSelIndexIncrement  = 1;
    }
    else
    {
        /* assumed udbOrderAscend == GT_FALSE */
        udbCfgOffsetBase      = udbOffsetLow + udbAmount - 1;
        udbCfgOffsetIncrement = -1;
        udbSelIndexBase       = udbIndexLow + udbAmount - 1;
        udbSelIndexIncrement  = -1;
    }

    numOfBytesInBuff = sizeof(packetBuffer);
    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficEnginePacketBuild FAILED: %d", prvTgfDevNum);

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPacketChecksumUpdate FAILED: %d", prvTgfDevNum);

    patternPtr = &(packetBuffer[offsetInPacket]);

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysPortVlanFdbSet FAILED: %d", prvTgfDevNum);

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    if (direction != CPSS_PCL_DIRECTION_EGRESS_E)
    {
        udbMaskPtr    = mask.ruleIngrUdbOnly.udb;
        udbPatternPtr = pattern.ruleIngrUdbOnly.udb;
    }
    else
    {
        udbMaskPtr    = mask.ruleEgrUdbOnly.udb;
        udbPatternPtr = pattern.ruleEgrUdbOnly.udb;
    }

    for (i = 0; (i < udbAmount); i++)
    {
        udbMaskPtr[i]    = 0xFF;
        udbPatternPtr[i] = patternPtr[i];
    }

    setRuleIndex(direction,lookupNum);

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        direction,
        lookupNum,
        packetTypeIndex, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        ruleFormat,
        &mask, &pattern);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysPclLegacyCfgSet FAILED: %d", prvTgfDevNum);

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        direction,
        packetType,
        offsetType,
        udbCfgIndexBase,
        udbAmount,
        udbCfgOffsetBase,
        udbCfgOffsetIncrement);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet FAILED: %d", prvTgfDevNum);

    rc = prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        lookupNum, packetType, ruleFormat,
        0 /*udbReplaceBitmap*/,
        udbAmount,
        udbSelIndexBase,
        udbSelIndexIncrement);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet FAILED: %d", prvTgfDevNum);
}
/**
* @internal prvTgfExactMatchUdb40EthOthreL2PclGenericConfig function
* @endinternal
*
* @brief   Set PCL generic test configuration, not related to Exact Match
*          Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet
*
* @note 40 first bytes from L2 header beginning.
*/
GT_VOID prvTgfExactMatchUdb40EthOthreL2PclGenericConfig
(
    GT_VOID
)
{
    GT_U32 offsetInPacket = 0;

    prvTgfExactMatchPclUdbOnlyKeysGenericConfig(
        CPSS_PCL_DIRECTION_INGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                       /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E    /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/        /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E    /*ruleFormat*/,
        PRV_TGF_PCL_OFFSET_L2_E                     /*offsetType*/,
        40                                          /*udbAmount*/,
        10                                          /*udbIndexLow*/,
        0                                           /*udbOffsetLow*/,
        GT_TRUE                                     /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsEthOthInfo     /*packetPtr*/,
        offsetInPacket                              /*offsetInPacket*/);
}
/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyGenericTest function
* @endinternal
*
* @brief   Generic Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] offsetType               - offset Type
* @param[in] udbAmount                - UDB Amount
* @param[in] udbIndexLow              - The lowest UDB Index in the UDB range
*/
GT_VOID prvTgfExactMatchPclUdbOnlyKeysGenericRestore
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexLow
)
{
    GT_U32      udbCfgIndexBase;

    udbCfgIndexBase = udbIndexLow;

     /* Restore PCL generic configuration */
    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        direction,
        lookupNum,
        ruleSize);

    udbCfgIndexBase = udbIndexLow;
    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        direction,
        packetType,
        udbCfgIndexBase,
        udbAmount);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset(
        lookupNum, packetType, ruleFormat);
}
/**
* @internal prvTgfExactMatchUdb40EthOthreL2PclGenericRestore function
* @endinternal
*
* @brief   Restore PCL generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchUdb40EthOthreL2PclGenericRestore
(
    GT_VOID
)
{
    prvTgfExactMatchPclUdbOnlyKeysGenericRestore(
        CPSS_PCL_DIRECTION_INGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                       /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E    /*packetType*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_40_BYTES_E               /*ruleSize*/,
        40                                          /*udbAmount*/,
        10                                          /*udbIndexLow*/);
}
/**
* @internal prvTgfExactMatchUdbOnlyKeysTrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] egressVid         - VID found in egressed packets VLAN Tag
* @param[in] checkMsb          - to check High bits of VID and UP None
* @param[in] expectNoTraffic   - GT_TRUE:  No traffic
*                                GT_FALSE: Expect Traffic
* @param[in] expectFdbForwarding - GT_TRUE:  FBB forwarding
*                                GT_FALSE: NO FBB forwarding*
*
*/
static GT_VOID prvTgfExactMatchUdbOnlyKeysTrafficEgressVidCheck
(
    IN GT_U16  egressVid,
    IN GT_BOOL checkMsb,
    IN GT_BOOL expectNoTraffic,
    IN GT_BOOL expectFdbForwarding
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;
    GT_U32                          egressVidExpected;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    if (expectFdbForwarding==GT_FALSE)
    {
        egressVidExpected=egressVid;
    }
    else
    {
        egressVidExpected = PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS;
    }

    if (checkMsb == GT_FALSE)
    {
        vfdArray[0].offset = 15; /* 6 + 6 + 3 */
        vfdArray[0].cycleCount = 1;
        vfdArray[0].patternPtr[0] = (GT_U8)(egressVidExpected & 0xFF);
    }
    else
    {
        vfdArray[0].offset = 14; /* 6 + 6 + 2 */
        vfdArray[0].cycleCount = 2;
        vfdArray[0].patternPtr[0] = (GT_U8)((egressVidExpected >> 8) & 0xFF);
        vfdArray[0].patternPtr[1] = (GT_U8)(egressVidExpected & 0xFF);
    }

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            1 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    if(expectNoTraffic==GT_TRUE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "NOT received patterns bitmap");
        PRV_UTF_LOG1_MAC("Packet is Dropped as expected, expectNoTraffic=%d\n\n",expectNoTraffic);
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
        1, numTriggersBmp, "received patterns bitmap");
        PRV_UTF_LOG1_MAC("Packet is redirected to port with egress VID in the Tag =%d\n\n",egressVidExpected);
    }
}

/**
* @internal prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped or dropped
* @param[in] expectNoTraffic   - GT_TRUE:  No traffic
*                                GT_FALSE: Expect Traffic
* @param[in] expectFdbForwarding - GT_TRUE:  FBB forwarding
*                                GT_FALSE: NO FBB forwarding
*/
GT_VOID prvTgfExactMatchUdb40EthOthreL2PclTrafficGenerate
(
    GT_BOOL     expectNoTraffic,
    GT_BOOL     expectFdbForwarding
)
{
    /* AUTODOC: Double Tagged Packet */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketNotTunneled2tagsEthOthInfo);/*packetPtr*/

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfExactMatchUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS,
                                                     GT_TRUE,/*check 16 bit*/
                                                     expectNoTraffic,
                                                     expectFdbForwarding);
}
