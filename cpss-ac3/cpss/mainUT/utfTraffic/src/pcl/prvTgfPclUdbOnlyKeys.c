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
* @file prvTgfPclUdbOnlyKeys.c
*
* @brief SIP5 specific PCL UDB Only keys testing
*
* @version   12
********************************************************************************
*/

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

/* PCL id for restore procedure */
static GT_U32 savePclId;
/********************************************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part =
{
    {0x00, 0xFE, 0xDC, 0xBA, 0x98, 0x76},                /* daMac */
    {0x00, 0xBB, 0xAA, 0x11, 0x22, 0x33}                 /* saMac */
};

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

/* Ethernet Type part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOth1EthertypePart =
{
    0x3456
};

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
    {21, 22, 23, 24},                                                /* srcAddr */
    { 1,  1,  1,  3}                                                 /* dstAddr */
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
    {32, 42, 52, 62},                                                /* srcAddr */
    { 1,  2,  3,  4}                                                 /* dstAddr */
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
    {32, 42, 52, 62},                                                /* srcAddr */
    { 1,  2,  3,  4}                                                 /* dstAddr */
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
    {16, 17, 18, 19},   /* srcAddr */
    { 4,  5,  6,  7}    /* dstAddr */
};

/* Double Tagged Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketNotTunneled2tagsEthOthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged Ethernet Other Packet Info */
static TGF_PACKET_STC prvTgfPacketNotTunneled2tagsEthOthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketNotTunneled2tagsEthOthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketNotTunneled2tagsEthOthPartsArray                                        /* partsArray */
};

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
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
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
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketTunnelMplsLabel0},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
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
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4TcpHeaderPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketPassengerTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
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
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4UdpHeaderPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketPassengerUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV4 Packet UDP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerIpv4UdpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS)+ TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerIpv4UdpPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerIpv4UdpPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV6 UDP Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerIpv6UdpPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6UdpHeaderPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketPassengerUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
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
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketLlcNonSnapLengthPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Single Tagged LLC Non SNAP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerLlcNonSnapInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS +
     + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerLlcNonSnapPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerLlcNonSnapPartsArray                                        /* partsArray */
};

/* backup configuration for restore after the test */
static GT_U32   origRouterMacSaIndex = 0;
static GT_ETHERADDR origMacSaAddr = {{0}};

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
* @internal prvTgfPclUdbOnlyKeysPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclUdbOnlyKeysPortVlanFdbSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN as TAG in Passanger with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as TAG in Tunnel with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as PCL assigned VID with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_NEW_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Learn MAC_DA of Passenger with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketPassengerL2Part.daMac,
        PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of Tunnel with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketTunnelL2Part.daMac,
        PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of Passenger with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketPassengerL2Part.daMac,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of Tunnel with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketTunnelL2Part.daMac,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclUdbOnlyKeysPortVlanFdbReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in Passanger */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in Tunnel */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as PCL assigned */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_PCL_NEW_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysVidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
GT_VOID prvTgfPclUdbOnlyKeysVidTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: Transmit Packets With Capture*/

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPacketInfoPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
}

/**
* @internal prvTgfPclUdbOnlyKeysTrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] egressVid                - VID found in egressed packets VLAN Tag
* @param[in] checkMsb                 - to check High bits of VID and UP
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysTrafficEgressVidCheck
(
    IN GT_U16  egressVid,
    IN GT_BOOL checkMsb
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    if (checkMsb == GT_FALSE)
    {
        vfdArray[0].offset = 15; /* 6 + 6 + 3 */
        vfdArray[0].cycleCount = 1;
        vfdArray[0].patternPtr[0] = (GT_U8)(egressVid & 0xFF);
    }
    else
    {
        vfdArray[0].offset = 14; /* 6 + 6 + 2 */
        vfdArray[0].cycleCount = 2;
        vfdArray[0].patternPtr[0] = (GT_U8)((egressVid >> 8) & 0xFF);
        vfdArray[0].patternPtr[1] = (GT_U8)(egressVid & 0xFF);
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
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSet function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *patternPtr
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormatArr[3];
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           portIndex;

    portIndex =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_RECEIVE_PORT_IDX_CNS
            : PRV_TGF_SEND_PORT_IDX_CNS;

    ruleFormatArr[0] =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    ruleFormatArr[1] =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    ruleFormatArr[2] = ruleFormatArr[1];

    ruleFormatArr[pktType] = ruleFormat;


    /* AUTODOC: Init and configure all needed PCL configuration per port Lookup0 */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[portIndex],
        direction, lookupNum,
        PRV_TGF_PCL_ID_CNS,
        ruleFormatArr[0] /*nonIpKey*/,
        ruleFormatArr[1] /*ipv4Key*/,
        ruleFormatArr[2] /*ipv6Key*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclDefPortInitExt1");

    /* AUTODOC: action - TAG0 vlan modification */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        action.egressPolicy = GT_TRUE;
        action.vlan.egressVlanId0Cmd =
            PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }
    else
    {
        action.egressPolicy = GT_FALSE;
        action.vlan.vlanCmd =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }

    setRuleIndex(direction,lookupNum);

    /* AUTODOC: PCL rule: Write Rule */
    rc = prvTgfPclRuleSet(
        ruleFormat,
        currentRuleIndex,
        maskPtr, patternPtr, &action);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclRuleSet");

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgReset function
* @endinternal
*
* @brief   Restore Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleSize                 - Rule Size
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_PCL_RULE_SIZE_ENT          ruleSize
)
{
    GT_STATUS rc,rc1 = GT_OK;

    /* AUTODOC: restore PCL configuration table entry */
    prvTgfPclPortsRestore(direction, lookupNum);

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(
        ruleSize,
        currentRuleIndex,
        GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet function
* @endinternal
*
* @brief   Set PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] offsetType               - offset Type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
* @param[in] udbOffsetBase            - udb Offset Base
* @param[in] udbOffsetIncrement       - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbOffsetBase,
    IN GT_32                            udbOffsetIncrement
)
{
    GT_STATUS                            rc;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat;
    GT_U8                                offset;
    GT_U32                               udbIndex;
    GT_U32                               i;

    ruleFormat =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;
        offset = (GT_U8)(udbOffsetBase + (i * udbOffsetIncrement));

        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType, direction,udbIndex, offsetType, offset);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedByteSet");
    }

    return GT_OK;
}


/**
* @internal prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset function
* @endinternal
*
* @brief   Reset PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount
)
{
    GT_STATUS                            rc, rc1 = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat;
    GT_U32                               udbIndex;
    GT_U32                               i;

    ruleFormat =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;

        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType, direction, udbIndex,
            PRV_TGF_PCL_OFFSET_INVALID_E, 0/*offset*/);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclUserDefinedByteSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysTtiTunnelTermConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] tunnelPacketType         - Tunnel Packet Type (to enable lookup)
* @param[in] passengerPacketType      - Passenger Packet Type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbOnlyKeysTtiTunnelTermConfigSet
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType,
    IN PRV_TGF_TTI_PASSENGER_TYPE_ENT passengerPacketType
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));


    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, given passenger type */
    ttiAction.tunnelTerminate                   = GT_TRUE;
    ttiAction.ttPassengerPacketType             = passengerPacketType;
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    switch (tunnelPacketType)
    {
        case PRV_TGF_TTI_KEY_MPLS_E:
            ttiMask.mpls.common.pclId    = 0x3FF;
            ttiPattern.mpls.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_IPV4_E:
            ttiMask.ipv4.common.pclId    = 0x3FF;
            ttiPattern.ipv4.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        default: break;
    }

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, tunnelPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, tunnelPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    prvTgfPclTunnelTermForceVlanModeEnableSet(GT_FALSE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclTunnelTermForceVlanModeEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysTtiTunnelTermConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @param[in] tunnelPacketType         - Tunnel Packet Type (to enable lookup)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbOnlyKeysTtiTunnelTermConfigReset
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType
)
{
    GT_STATUS                rc, rc1 = GT_OK;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_FALSE);

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");

    rc = prvTgfPclTunnelTermForceVlanModeEnableSet(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclTunnelTermForceVlanModeEnableSet");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysTtiTunnelStartConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] originalPacketType       - original Packet Type (to enable lookup)
* @param[in] passengerPacketType      - Passenger Packet Type
* @param[in] tunnelType               - tunnel Type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbOnlyKeysTtiTunnelStartConfigSet
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT          originalPacketType,
    IN PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT passengerPacketType,
    IN CPSS_TUNNEL_TYPE_ENT              tunnelType
)
{
    GT_STATUS                      rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC       ttiAction;
    PRV_TGF_TTI_RULE_UNT           ttiPattern;
    PRV_TGF_TTI_RULE_UNT           ttiMask;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;
    GT_ETHERADDR                   macSaAddr;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: set the TTunnel/ARP Table entry */

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    switch (tunnelType)
    {
        case CPSS_TUNNEL_X_OVER_MPLS_E:
            tunnelEntry.mplsCfg.tagEnable    = GT_TRUE;
            tunnelEntry.mplsCfg.vlanId       = prvTgfPacketTunnelVlanTag0Part.vid;
            cpssOsMemCpy(
                tunnelEntry.mplsCfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
            tunnelEntry.mplsCfg.numLabels    = 1;
            tunnelEntry.mplsCfg.ttl          = prvTgfPacketTunnelMplsLabel0.timeToLive;
            tunnelEntry.mplsCfg.exp1         = prvTgfPacketTunnelMplsLabel0.exp;
            tunnelEntry.mplsCfg.label1       = prvTgfPacketTunnelMplsLabel0.label;
            break;
        case CPSS_TUNNEL_X_OVER_IPV4_E:
        case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
        case CPSS_TUNNEL_GENERIC_IPV4_E:
            tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
            tunnelEntry.ipv4Cfg.vlanId           = prvTgfPacketTunnelVlanTag0Part.vid;
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

            /* tunnel next hop MAC DA, IP DA, IP SA */
            cpssOsMemCpy(
                tunnelEntry.ipv4Cfg.macDa.arEther,
                prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
            cpssOsMemCpy(
                tunnelEntry.ipv4Cfg.destIp.arIP,
                prvTgfPacketTunnelIpv4OtherHeaderPart.dstAddr, sizeof(TGF_IPV4_ADDR));
            cpssOsMemCpy(
                tunnelEntry.ipv4Cfg.srcIp.arIP,
                prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr, sizeof(TGF_IPV4_ADDR));
            break;
        default:
            rc = GT_BAD_PARAM;
            PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "not supported tunnel type");
            return rc;
    }

    rc = prvTgfTunnelStartEntrySet(
        PRV_TGF_ARP_ENTRY_IDX_CNS, tunnelType, &tunnelEntry);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTunnelStartEntrySet");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));


    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, given passenger type */
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum    = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.bridgeBypass                      = GT_TRUE;
    ttiAction.tunnelStart                       = GT_TRUE;
    ttiAction.tsPassengerPacketType             = passengerPacketType;
    ttiAction.tunnelStartPtr                    = PRV_TGF_ARP_ENTRY_IDX_CNS;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.keepPreviousQoS                   = GT_TRUE;
    ttiAction.mplsCommand                       = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
    ttiAction.mplsLabel                         = prvTgfPacketTunnelMplsLabel0.label;
    ttiAction.mplsTtl                           = prvTgfPacketTunnelMplsLabel0.timeToLive;


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    switch (originalPacketType)
    {
        case PRV_TGF_TTI_KEY_MPLS_E:
            ttiMask.mpls.common.pclId    = 0x3FF;
            ttiPattern.mpls.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_IPV4_E:
            ttiMask.ipv4.common.pclId    = 0x3FF;
            ttiPattern.ipv4.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_ETH_E:
            ttiMask.eth.common.pclId    = 0x3FF;
            ttiPattern.eth.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_MIM_E:
            ttiMask.mim.common.pclId    = 0x3FF;
            ttiPattern.mim.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        default: break;
    }

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        originalPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, originalPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, originalPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, originalPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet(
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: set MAC_SA of tunnel header */

    macSaAddr.arEther[0] =  prvTgfPacketTunnelL2Part.saMac[0];
    macSaAddr.arEther[1] =  prvTgfPacketTunnelL2Part.saMac[1];
    macSaAddr.arEther[2] =  prvTgfPacketTunnelL2Part.saMac[2];
    macSaAddr.arEther[3] =  prvTgfPacketTunnelL2Part.saMac[3];
    macSaAddr.arEther[4] =  prvTgfPacketTunnelL2Part.saMac[4];
    macSaAddr.arEther[5] =  prvTgfPacketTunnelL2Part.saMac[5];

    rc = prvTgfIpRouterPortGlobalMacSaIndexGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        &origRouterMacSaIndex);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaGet(
        prvTgfDevNum,
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
        &origMacSaAddr);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterGlobalMacSaGet");

    rc = prvTgfIpRouterGlobalMacSaSet(
        prvTgfDevNum,
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
        &macSaAddr);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterGlobalMacSaSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysTtiTunnelStartConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @param[in] originalPacketType       - origina Packet Type (to enable lookup)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbOnlyKeysTtiTunnelStartConfigReset
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT          originalPacketType
)
{
    GT_STATUS                      rc, rc1 = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        originalPacketType, GT_FALSE);

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, originalPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");

    prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet(
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    rc = prvTgfTunnelStartEntrySet(
        PRV_TGF_ARP_ENTRY_IDX_CNS, CPSS_TUNNEL_X_OVER_IPV4_E, &tunnelEntry);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTunnelStartEntrySet");

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        origRouterMacSaIndex);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaSet(
        prvTgfDevNum,
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
        &origMacSaAddr);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpRouterGlobalMacSaSet");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet function
* @endinternal
*
* @brief   Set UDB Only PCL Key UDB Range selection
*         Replaced UDB positions ommited.
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
* @param[in] udbReplaceBitmap         - bitmap of replace UDBs (12 bits ingress, 2 bits egress)
* @param[in] udbAmount                - udb Amount
* @param[in] udbIndexBase             - udb Index Base
*                                      udbOffsetIncrement  - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           udbReplaceBitmap,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexBase,
    IN GT_32                            udbIndexIncrement
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    GT_U32                              udbInKeyMapped;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    udbInKeyMapped = 0;

    for (i = 0; (i < udbAmount); i++)
    {
        if (udbReplaceBitmap & (1 << i))
        {
            if (i == 0) udbSelect.egrUdb01Replace      = GT_TRUE;
            if (i == 1) udbSelect.egrUdbBit15Replace   = GT_TRUE;
            if (i < 12) udbSelect.ingrUdbReplaceArr[i] = GT_TRUE;
        }
        else
        {
            udbSelect.udbSelectArr[i] =
                udbIndexBase + (udbIndexIncrement * udbInKeyMapped);
            udbInKeyMapped ++;
        }
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, lookupNum,
        &udbSelect);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedBytesSelectSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset function
* @endinternal
*
* @brief   Reset UDB Only PCL Key UDB selection
*
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat
)
{
    GT_STATUS                           rc;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, lookupNum,
        &udbSelect);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedBytesSelectSet");

    return GT_OK;
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
* @param[in] ruleSize                 - rule Size
* @param[in] offsetType               - offset Type
* @param[in] udbAmount                - UDB Amount
* @param[in] udbIndexLow              - The lowest UDB Index in the UDB range
* @param[in] udbOffsetLow             - UDB Offset Base of the field (the lowest byte offset)
* @param[in] udbOrderAscend           - UDB Order Ascend (GT_TRUE) or Descend (GT_FALSE)
* @param[in] packetPtr                - (pointer to) Info of the packet used in the test
* @param[in] offsetInPacket           - offset of matched data in the packet
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyGenericTest
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
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
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    patternPtr = &(packetBuffer[offsetInPacket]);

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

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

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        direction,
        lookupNum,
        packetTypeIndex, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        ruleFormat,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        direction,
        packetType,
        offsetType,
        udbCfgIndexBase,
        udbAmount,
        udbCfgOffsetBase,
        udbCfgOffsetIncrement);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        lookupNum, packetType, ruleFormat,
        0 /*udbReplaceBitmap*/,
        udbAmount,
        udbSelIndexBase,
        udbSelIndexIncrement);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged Packet */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(packetPtr);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        direction,
        lookupNum,
        ruleSize);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        direction,
        packetType,
        udbCfgIndexBase,
        udbAmount);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset(
        lookupNum, packetType, ruleFormat);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngressMetadataTest function
* @endinternal
*
* @brief   Test on Ingress PCL metadata
*
* @note Metadata[207:192] - Ethernet type
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressMetadataTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    /* UDB 40-41 */
    mask.ruleExtNotIpv6.udb[40] = 0xFF;
    mask.ruleExtNotIpv6.udb[41] = 0xFF;
    pattern.ruleExtNotIpv6.udb[40] =
        (GT_U8)(prvTgfPacketEthOth1EthertypePart.etherType & 0xFF);
    pattern.ruleExtNotIpv6.udb[41] =
        (GT_U8)((prvTgfPacketEthOth1EthertypePart.etherType >> 8) & 0xFF);

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        0, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        PRV_TGF_PCL_OFFSET_METADATA_E,
        40 /*udbIndexBase*/,
        2  /*udbAmount*/,
        24 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Send Double Tagged Ethernet Other Packet without Tunnel header */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketNotTunneled2tagsEthOthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_EXT_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        40 /*udbIndexBase*/,
        2  /*udbAmount*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgressMetadataTest function
* @endinternal
*
* @brief   Test on Egress PCL metadata
*
* @note Metadata[207:196] - Original VID
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressMetadataTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    /* UDB 30-31 */
    mask.ruleEgrExtIpv6L4.udb[30] = 0xF0;
    mask.ruleEgrExtIpv6L4.udb[31] = 0xFF;
    pattern.ruleEgrExtIpv6L4.udb[30] =
        (GT_U8)((prvTgfPacketPassengerVlanTag0Part.vid << 4) & 0xF0);
    pattern.ruleEgrExtIpv6L4.udb[31] =
        (GT_U8)((prvTgfPacketPassengerVlanTag0Part.vid >> 4) & 0xFF);

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        2, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_E,
        PRV_TGF_PCL_OFFSET_METADATA_E,
        30 /*udbIndexBase*/,
        2  /*udbAmount*/,
        24 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Send Double Tagged IPV6 Other Packet without Tunnel header */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketNotTunneled2tagsIpv6OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_EXT_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_E,
        30 /*udbIndexBase*/,
        2  /*udbAmount*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngressTunnelL2Test function
* @endinternal
*
* @brief   Test on Ingress PCL Tunnel L2 offset type
*
* @note Tunnel Header MAC_SA
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressTunnelL2Test
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    /* UDB 20-22, 31-33 */
    mask.ruleStdIpv4L4.udb[20]         = 0xFF;
    mask.ruleStdIpv4L4.udb[21]         = 0xFF;
    mask.ruleStdIpv4L4.udb[22]         = 0xFF;
    mask.ruleStdIpv4L4.udb[31]    = 0xFF;
    mask.ruleStdIpv4L4.udb[32]    = 0xFF;
    mask.ruleStdIpv4L4.udb[33]    = 0xFF;

    pattern.ruleStdIpv4L4.udb[20]      = prvTgfPacketTunnelL2Part.saMac[0];
    pattern.ruleStdIpv4L4.udb[21]      = prvTgfPacketTunnelL2Part.saMac[1];
    pattern.ruleStdIpv4L4.udb[22]      = prvTgfPacketTunnelL2Part.saMac[2];
    pattern.ruleStdIpv4L4.udb[31] = prvTgfPacketTunnelL2Part.saMac[3];
    pattern.ruleStdIpv4L4.udb[32] = prvTgfPacketTunnelL2Part.saMac[4];
    pattern.ruleStdIpv4L4.udb[33] = prvTgfPacketTunnelL2Part.saMac[5];

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        1, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,
        20 /*udbIndexBase*/,
        3  /*udbAmount*/,
        6 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,
        31 /*udbIndexBase*/,
        3  /*udbAmount*/,
        9 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysTtiTunnelTermConfigSet(
        PRV_TGF_TTI_KEY_MPLS_E,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Send Double Tagged IPV4 Other Packet with MPLS Tunnel header */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketMplsTunneled2tagsIpv4OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_STD_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        20 /*udbIndexBase*/,
        3  /*udbAmount*/);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        31 /*udbIndexBase*/,
        3  /*udbAmount*/);

    rc = prvTgfPclUdbOnlyKeysTtiTunnelTermConfigReset(
        PRV_TGF_TTI_KEY_MPLS_E);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngressTunnelL3Test function
* @endinternal
*
* @brief   Test on Ingress PCL Tunnel L3 offset type
*
* @note Tunnel Header DIP[15:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressTunnelL3Test
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* UDB 18-19 */
    mask.ruleStdIpL2Qos.udb[18]         = 0xFF;
    mask.ruleStdIpL2Qos.udb[19]         = 0xFF;

    pattern.ruleStdIpL2Qos.udb[18]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.dstAddr[2];
    pattern.ruleStdIpL2Qos.udb[19]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.dstAddr[3];

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        2, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
        18 /*udbIndexBase*/,
        2  /*udbAmount*/,
        20 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysTtiTunnelTermConfigSet(
        PRV_TGF_TTI_KEY_IPV4_E,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged IPV6 Packet Tunneled L2 IPV6 Other */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketIpv4TunneledL2Ipv6OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_STD_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
        18 /*udbIndexBase*/,
        2  /*udbAmount*/);

    rc = prvTgfPclUdbOnlyKeysTtiTunnelTermConfigReset(
        PRV_TGF_TTI_KEY_IPV4_E);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgressTunnelL2Test function
* @endinternal
*
* @brief   Test on Egress PCL Tunnel L2 offset type
*
* @note Tunnel Header MAC_DA[15:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressTunnelL2Test
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* UDB 4-5 */
    mask.ruleEgrStdIpL2Qos.udb[4]         = 0xFF;
    mask.ruleEgrStdIpL2Qos.udb[5]         = 0xFF;

    pattern.ruleEgrStdIpL2Qos.udb[4]      =
        (GT_U8)prvTgfPacketTunnelL2Part.daMac[4];
    pattern.ruleEgrStdIpL2Qos.udb[5]      =
        (GT_U8)prvTgfPacketTunnelL2Part.daMac[5];

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        1, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,
        4 /*udbIndexBase*/,
        2  /*udbAmount*/,
        4 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysTtiTunnelStartConfigSet(
        PRV_TGF_TTI_KEY_IPV4_E,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E,
        CPSS_TUNNEL_X_OVER_MPLS_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged Packet IPV4 Other */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketPassenger2tagsIpv4OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_STD_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        4 /*udbIndexBase*/,
        2  /*udbAmount*/);

    rc = prvTgfPclUdbOnlyKeysTtiTunnelStartConfigReset(
        PRV_TGF_TTI_KEY_IPV4_E);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL Tunnel L3 Minus 2 offset type
*
* @note Tunnel Header SIP[31:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* UDB 20-23 */
    mask.ruleEgrExtIpv6L2.udb[20]         = 0xFF;
    mask.ruleEgrExtIpv6L2.udb[21]         = 0xFF;
    mask.ruleEgrExtIpv6L2.udb[22]         = 0xFF;
    mask.ruleEgrExtIpv6L2.udb[23]         = 0xFF;

    pattern.ruleEgrExtIpv6L2.udb[20]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr[0];
    pattern.ruleEgrExtIpv6L2.udb[21]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr[1];
    pattern.ruleEgrExtIpv6L2.udb[22]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr[2];
    pattern.ruleEgrExtIpv6L2.udb[23]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr[3];

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        2, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
        20 /*udbIndexBase*/,
        4  /*udbAmount*/,
        14 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysTtiTunnelStartConfigSet(
        PRV_TGF_TTI_KEY_ETH_E,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E,
        CPSS_TUNNEL_GENERIC_IPV4_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged IPV6 Packet Tunneled L2 IPV6 Other */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketPassengerIpv6OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_EXT_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,
        20 /*udbIndexBase*/,
        4  /*udbAmount*/);

    rc = prvTgfPclUdbOnlyKeysTtiTunnelStartConfigReset(
        PRV_TGF_TTI_KEY_ETH_E);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB10 L4 offset type on IPV4 UDP packet
*
* @note 10 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV4_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E         /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_10_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        10                                         /*udbAmount*/,
        40                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassengerIpv4UdpInfo          /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB10 L4 offset type on IPV4 TCP packet
*
* @note 10 first bytes from TCP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV4_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E         /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_10_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        10                                         /*udbAmount*/,
        40                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4TcpInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 20 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV6_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E         /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_20_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        20                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_FALSE                                   /*udbOrderAscend*/,
        &prvTgfPacketPassengerIpv6UdpInfo          /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB30 MplsMinus2 offset type on Ethernet over MPLS packet
*
* @note 20 first bytes from MPLS Ethernet Type beginning.
*       MPLS offset supported by HW only up to 20 bytes
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_MPLS_E             /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E          /*offsetType*/,
        20                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketMplsTunneled2tagsEthOthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet
*
* @note 40 first bytes from L2 header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket = 0;

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                       /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E    /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/        /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_40_BYTES_E               /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L2_E                     /*offsetType*/,
        40                                          /*udbAmount*/,
        10                                          /*udbIndexLow*/,
        0                                           /*udbOffsetLow*/,
        GT_TRUE                                     /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsEthOthInfo     /*packetPtr*/,
        offsetInPacket                              /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 L3Minus2 offset type on IPV6 Other packet
*
* @note 50 first bytes from IPV6 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E       /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_50_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        50                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_FALSE                                   /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsIpv6OthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60 L3Minus2 offset type on IPV4 Other packet
*
* @note 50 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E       /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_60_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        50                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4OthInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60_NO_FIXED L3Minus2 offset type on IPV4 Other
*         packet
*
* @note 60 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E       /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_60_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        60                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4OthInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB10 L4 offset type on IPV4 TCP packet
*
* @note 10 first bytes from TCP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV4_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E         /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_10_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        10                                         /*udbAmount*/,
        40                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4TcpInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 20 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV6_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E         /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_20_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        20                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_FALSE                                   /*udbOrderAscend*/,
        &prvTgfPacketPassengerIpv6UdpInfo          /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB30 MplsMinus2 offset type on ETH over MPLS packet.
*
* @note 20 first bytes from MPLS Ethernet Type beginning.
*       MPLS offset supported by HW only up to 20 bytes
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_MPLS_E             /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E          /*offsetType*/,
        20                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketMplsTunneled2tagsEthOthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB40 L2 offset type on ETH Other packet.
*
* @note 40 first bytes from L2 Header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket = 0;

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E   /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_40_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L2_E                    /*offsetType*/,
        40                                         /*udbAmount*/,
        10                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsEthOthInfo    /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB50 L3Minus2 offset type on IPV6 Other packet.
*
* @note 50 first bytes from IPV6 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E       /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_50_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        50                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_FALSE                                   /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsIpv6OthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB60 L3Minus2 offset type on IPV4 Other packet.
*
* @note 50 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E       /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_60_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        50                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4OthInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB30 L3Minus2 offset type on Ethernet over MPLS packet
*
* @note 30 first bytes from MPLS Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_MPLS_HEADER_SIZE_CNS) - 2;

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_MPLS_E             /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        30                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketMplsTunneled2tagsEthOthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB30 L3Minus2 offset type on ETH over MPLS packet.
*
* @note 30 first bytes from MPLS Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_MPLS_HEADER_SIZE_CNS) - 2;

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_MPLS_E             /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        30                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketMplsTunneled2tagsEthOthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest function
* @endinternal
*
* @brief   Test Egress PCL on LLC Non SNAP packet.
*
* @note Used 10 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
            (TGF_L2_HEADER_SIZE_CNS
             + (2 * TGF_VLAN_TAG_SIZE_CNS) + 3);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E   /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_10_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        10                                         /*udbAmount*/,
        40                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassengerLlcNonSnapInfo       /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/*====================================================*/
/* sample of using POST_TEST_EXIT for trace           */
/*====================================================*/

#if 0

static GT_VOID checkTtiEnableOnEgressPort
(
    IN const GT_CHAR*          suitNamePtr,
    IN const GT_CHAR*          testNamePtr
)
{
    GT_STATUS rc0, rc1, rc2;
    GT_BOOL   enableEth;
    GT_BOOL   enableIpv4;
    GT_BOOL   enableMpls;

    /* pach */
    rc0 = cpssDxChTtiPortLookupEnableGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_ETH_E, &enableEth);

    rc1 = cpssDxChTtiPortLookupEnableGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_IPV4_E, &enableIpv4);

    rc2 = cpssDxChTtiPortLookupEnableGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_MPLS_E, &enableMpls);

    cpssOsPrintf(
        "\n<<<<<POST_TEST_EXIT %s.%s rc: %d, enable: %d >>>>>\n",
        suitNamePtr, testNamePtr,
        (rc0 + rc1 + rc2), (enableEth + enableIpv4 + enableMpls));
}

void prvTgfPclUdbOnlyKeysBindCheckTtiEnableOnEgressPort()
{
    utfDebugPostTestExitBind(checkTtiEnableOnEgressPort);
}

#endif

