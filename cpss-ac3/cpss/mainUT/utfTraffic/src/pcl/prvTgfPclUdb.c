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
* @file prvTgfPclUdb.c
*
* @brief User Defined Bytes tests
*
* @version   36
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
#include <common/tgfPclGen.h>
#include <common/tgfVntGen.h>
#include <common/tgfCosGen.h>
#include <pcl/prvTgfPclUdb.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS      0
#define PRV_TGF_RECEIVE_PORT_IDX_CNS   1

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS          5

/* VRF Id */
#define PRV_TGF_VRFID_CNS           0x0123

/* Qos Profile */
#define PRV_TGF_QOS_PROFILE_CNS     0x33

/* user defined ethertype */
#define PRV_TGF_UDE_TAG_CNS         0x8888

#define EPCL_RULE_INDEX_CNS     prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(0)


/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/************************* General packet's parts *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x11}                 /* saMac */
};

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* ethertype part of IPV6 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* ethertype part of ETH_OTHER packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherOtherTypePart = {0x2222};

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

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

/******************* Packet with not matched UDBs *************************/

/* UDP packet for not matched UDBs */
static GT_U8 prvTgfPacketNotMatchPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketNotMatchPayloadPart =
{
    sizeof(prvTgfPacketNotMatchPayloadDataArr), /* dataLength */
    prvTgfPacketNotMatchPayloadDataArr          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketNotMatchPartArray[] =
{
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketNotMatchPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_NOT_MATCH_LEN_CNS sizeof(prvTgfPacketNotMatchPayloadDataArr)

/* Length of packet with CRC */
#define PRV_TGF_PACKET_NOT_MATCH_CRC_LEN_CNS  PRV_TGF_PACKET_NOT_MATCH_LEN_CNS + TGF_CRC_LEN_CNS

/* packet to send */
static TGF_PACKET_STC prvTgfPacketNotMatchInfo =
{
    PRV_TGF_PACKET_NOT_MATCH_LEN_CNS,                                                 /* totalLen */
    sizeof(prvTgfPacketNotMatchPartArray) / sizeof(prvTgfPacketNotMatchPartArray[0]), /* numOfParts */
    prvTgfPacketNotMatchPartArray                                                     /* partsArray */
};

/******************************************************************************/

/******************************* TCP packet **********************************/

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketTcpIpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x42,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    6,                  /* protocol */
    0x4C87,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    123456,             /* sequence number */
    234567,             /* acknowledgment number */
    5,                  /* data offset */
    0x50,               /* reserved */
    0x10,               /* flags */
    4096,               /* window */
    0xFAF6,             /* csum */
    0                   /* urgent pointer */
};

/* PARTS of packet TCP */
static TGF_PACKET_PART_STC prvTgfTcpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTcpIpPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of TCP packet */
#define PRV_TGF_TCP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of TCP packet with CRC */
#define PRV_TGF_TCP_PACKET_CRC_LEN_CNS  PRV_TGF_TCP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* TCP packet to send */
static TGF_PACKET_STC prvTgfTcpPacketInfo =
{
    PRV_TGF_TCP_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfTcpPacketPartArray) / sizeof(prvTgfTcpPacketPartArray[0]), /* numOfParts */
    prvTgfTcpPacketPartArray                                                /* partsArray */
};
/******************************************************************************/

/******************************* UDP packet **********************************/

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacketUdpIpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x36,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    17,                 /* protocol */
    0x4C88,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    0x22,               /* length */
    0xD172              /* csum */
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfUdpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketUdpIpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of UDP packet */
#define PRV_TGF_UDP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of UDP packet with CRC */
#define PRV_TGF_UDP_PACKET_CRC_LEN_CNS  PRV_TGF_UDP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* UDP packet to send */
static TGF_PACKET_STC prvTgfUdpPacketInfo =
{
    PRV_TGF_UDP_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfUdpPacketPartArray) / sizeof(prvTgfUdpPacketPartArray[0]), /* numOfParts */
    prvTgfUdpPacketPartArray                                                /* partsArray */
};
/******************************************************************************/

/******************************* IPv4 packet **********************************/

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OtherPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    4,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C99,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4FragmentPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    1,                  /* flags */    /* more fragments     */
    0x200,              /* offset */   /* not first fragment */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C99,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet IPv4 - IPV4 OTHER */
static TGF_PACKET_PART_STC prvTgfIpv4OtherPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet IPv4 - IPV4 FRAGMENT */
static TGF_PACKET_PART_STC prvTgfIpv4FragmentPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4FragmentPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of IPv4 packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of IPv4 packet with CRC */
#define PRV_TGF_IPV4_PACKET_CRC_LEN_CNS  PRV_TGF_IPV4_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* IPv4 Other packet to send */
static TGF_PACKET_STC prvTgfIpv4OtherPacketInfo =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                      /* totalLen */
    (sizeof(prvTgfIpv4OtherPacketPartArray)
        / sizeof(prvTgfIpv4OtherPacketPartArray[0])), /* numOfParts */
    prvTgfIpv4OtherPacketPartArray                    /* partsArray */
};

/* IPv4 Fragment packet to send */
static TGF_PACKET_STC prvTgfIpv4FragmentPacketInfo =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                         /* totalLen */
    (sizeof(prvTgfIpv4FragmentPacketPartArray)
        / sizeof(prvTgfIpv4FragmentPacketPartArray[0])), /* numOfParts */
    prvTgfIpv4FragmentPacketPartArray                    /* partsArray */
};

/******************************************************************************/

/***************************** Ethernet packet ********************************/

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfEthernetPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherOtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of Ethernet packet */
#define PRV_TGF_ETHERNET_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Length of Ethernet packet with CRC */
#define PRV_TGF_ETHERNET_PACKET_CRC_LEN_CNS  \
    PRV_TGF_ETHERNET_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* Ethernet packet to send */
static TGF_PACKET_STC prvTgfEthernetPacketInfo =
{
    PRV_TGF_ETHERNET_PACKET_LEN_CNS,                                                  /* totalLen */
    (sizeof(prvTgfEthernetPacketPartArray)
        / sizeof(prvTgfEthernetPacketPartArray[0])), /* numOfParts */
    prvTgfEthernetPacketPartArray                                                     /* partsArray */
};
/******************************************************************************/

/******************************* MPLS packet **********************************/

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart = {TGF_ETHERTYPE_8847_MPLS_TAG_CNS};

/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsPart =
{
    1,                  /* label */
    0,                  /* experimental use */
    1,                  /* stack */
    0x40                /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of MPLS packet */
#define PRV_TGF_MPLS_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_MPLS_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadLongDataArr)

/* Length of MPLS packet with CRC */
#define PRV_TGF_MPLS_PACKET_CRC_LEN_CNS  \
    PRV_TGF_MPLS_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* MPLS packet to send */
static TGF_PACKET_STC prvTgfMplsPacketInfo =
{
    PRV_TGF_MPLS_PACKET_LEN_CNS,                 /* totalLen */
    (sizeof(prvTgfMplsPacketPartArray)
        / sizeof(prvTgfMplsPacketPartArray[0])), /* numOfParts */
    prvTgfMplsPacketPartArray                    /* partsArray */
};
/******************************************************************************/

/******************************* UDE packet ***********************************/

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketUsedDefinedEtherTypePart = {PRV_TGF_UDE_TAG_CNS};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfUdePacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketUsedDefinedEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of UDE packet */
#define PRV_TGF_UDE_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Length of UDE packet with CRC */
#define PRV_TGF_UDE_PACKET_CRC_LEN_CNS  PRV_TGF_UDE_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* UDE packet to send */
static TGF_PACKET_STC prvTgfUdePacketInfo =
{
    PRV_TGF_UDE_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfUdePacketPartArray) / sizeof(prvTgfUdePacketPartArray[0]), /* numOfParts */
    prvTgfUdePacketPartArray                                                /* partsArray */
};
/******************************************************************************/

/******************************* IPv6 packet **********************************/

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part =
{
    6,                                  /* version */
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    0x2E,                               /* payloadLen */
    41,                                 /* nextHeader */
    0x40,                               /* hopLimit */
    {22, 22, 22, 22, 22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  1,  1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet IPv6 */
static TGF_PACKET_PART_STC prvTgfIpv6PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of IPv6 packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of IPv6 packet with CRC */
#define PRV_TGF_IPV6_PACKET_CRC_LEN_CNS  PRV_TGF_IPV6_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* IPv6 packet to send */
static TGF_PACKET_STC prvTgfIpv6PacketInfo =
{
    PRV_TGF_IPV6_PACKET_LEN_CNS,                                              /* totalLen */
    sizeof(prvTgfIpv6PacketPartArray) / sizeof(prvTgfIpv6PacketPartArray[0]), /* numOfParts */
    prvTgfIpv6PacketPartArray                                                 /* partsArray */
};
/******************************************************************************/

/***************************** IPv6 TCP packet ********************************/

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6IpPartForTcp =
{
    6,                                  /* version */
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    0x2E,                               /* payloadLen */
    6,                                  /* TCP protocol number */
    0x40,                               /* hopLimit */
    {22, 22, 22, 22, 22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  1,  1,  1,  1,  3}    /* dstAddr */
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6IpPartForTcpWrongIpVer =
{
    8,                                  /* version */ /*wrong version*/
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    0x2E,                               /* payloadLen */
    6,                                  /* TCP protocol number */
    0x40,                               /* hopLimit */
    {22, 22, 22, 22, 22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  1,  1,  1,  1,  3}    /* dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketIpv6TcpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    123456,             /* sequence number */
    234567,             /* acknowledgment number */
    5,                  /* data offset */
    0x50,               /* reserved */
    0x10,               /* flags */
    4096,               /* window */
    0xFAF6,             /* csum */
    0                   /* urgent pointer */
};

/* PARTS of packet IPv6 */
static TGF_PACKET_PART_STC prvTgfIpv6TcpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6IpPartForTcp},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketIpv6TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of IPv6 packet */
#define PRV_TGF_IPV6_TCP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of IPv6 packet with CRC */
#define PRV_TGF_IPV6_TCP_PACKET_CRC_LEN_CNS  PRV_TGF_IPV6_TCP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* IPv6 packet to send */
static TGF_PACKET_STC prvTgfIpv6TcpPacketInfo =
{
    PRV_TGF_IPV6_TCP_PACKET_LEN_CNS,                                                /* totalLen */
    sizeof(prvTgfIpv6TcpPacketPartArray) / sizeof(prvTgfIpv6TcpPacketPartArray[0]), /* numOfParts */
    prvTgfIpv6TcpPacketPartArray                                                    /* partsArray */
};

/* Length of long UDP packet */
#define PRV_TGF_LONG_UDP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS \
              + TGF_UDP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadLongDataArr)

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfUdpLongPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketUdpIpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};
/* UDP packet to send */
static TGF_PACKET_STC prvTgfUdpLongPacketInfo =
{
    PRV_TGF_LONG_UDP_PACKET_LEN_CNS,             /* totalLen */
    sizeof(prvTgfUdpLongPacketPartArray) /
        sizeof(prvTgfUdpLongPacketPartArray[0]), /* numOfParts */
    prvTgfUdpLongPacketPartArray                 /* partsArray */
};
/* Not match packets */
/* Length of IPv6 packet */
#define PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS \
          + TGF_TCP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadLongDataArr)
static TGF_PACKET_PART_STC prvTgfIpv6TcpLongPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6IpPartForTcp},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketIpv6TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};
/* IPv6 packet to send */
static TGF_PACKET_STC prvTgfIpv6TcpLongPacketInfo =
{
    PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS,            /* totalLen */
    sizeof(prvTgfIpv6TcpLongPacketPartArray) /
        sizeof(prvTgfIpv6TcpLongPacketPartArray[0]), /* numOfParts */
    prvTgfIpv6TcpLongPacketPartArray                 /* partsArray */
};

static TGF_PACKET_PART_STC prvTgfIpv6TcpLongPacketPartArrayWrongIpVer[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6IpPartForTcpWrongIpVer},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketIpv6TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* IPv6 WrongIpVer packet to send */
static TGF_PACKET_STC prvTgfIpv6TcpLongPacketInfoWrongIpVer =
{
    PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS,            /* totalLen */
    sizeof(prvTgfIpv6TcpLongPacketPartArrayWrongIpVer) /
        sizeof(prvTgfIpv6TcpLongPacketPartArrayWrongIpVer[0]), /* numOfParts */
    prvTgfIpv6TcpLongPacketPartArrayWrongIpVer                 /* partsArray */
};

/* Data of not matched packet (long version) */
static GT_U8 prvTgfNotMatchPacketPayloadLongDataArr[] =
{
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfNotMatchPacketLongPayloadPart =
{
    sizeof(prvTgfNotMatchPacketPayloadLongDataArr), /* dataLength */
    prvTgfNotMatchPacketPayloadLongDataArr          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfEthernetNotMatchPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherOtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfNotMatchPacketLongPayloadPart}
};

/* Ethernet packet to send */
static TGF_PACKET_STC prvTgfEthernetNotMatchPacketInfo =
{
    PRV_TGF_ETHERNET_PACKET_LEN_CNS,                         /* totalLen */
    (sizeof(prvTgfEthernetNotMatchPacketPartArray)
        / sizeof(prvTgfEthernetNotMatchPacketPartArray[0])), /* numOfParts */
    prvTgfEthernetNotMatchPacketPartArray                    /* partsArray */
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfUdpNotMatchPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketUdpIpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfNotMatchPacketLongPayloadPart}
};
/* UDP packet to send */
static TGF_PACKET_STC prvTgfUdpNotMatchPacketInfo =
{
    PRV_TGF_LONG_UDP_PACKET_LEN_CNS,                 /* totalLen */
    sizeof(prvTgfUdpNotMatchPacketPartArray) /
        sizeof(prvTgfUdpNotMatchPacketPartArray[0]), /* numOfParts */
    prvTgfUdpNotMatchPacketPartArray                 /* partsArray */
};
/*----------------------------------------------------------------------------*/

static TGF_PACKET_PART_STC prvTgfIpv6TcpNotMatchPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6IpPartForTcp},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketIpv6TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfNotMatchPacketLongPayloadPart}
};
/* IPv6 packet to send */
static TGF_PACKET_STC prvTgfIpv6TcpNotMatchPacketInfo =
{
    PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS,                /* totalLen */
    sizeof(prvTgfIpv6TcpNotMatchPacketPartArray) /
        sizeof(prvTgfIpv6TcpNotMatchPacketPartArray[0]), /* numOfParts */
    prvTgfIpv6TcpNotMatchPacketPartArray                 /* partsArray */
};
/******************************************************************************/


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* is VLAN and FDB entry already written */
static GT_BOOL  prvTgfIsVlanFdbInitialized = GT_FALSE;

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfDxChPclQosProfileAndTrustModeSet function
* @endinternal
*
* @brief   Set Qos profile Qos Trust Mode for ingess port:
*
* @param[in] qosProfile               - qos Profile
* @param[in] trustMode                - packet DSCP and UP trust mode
*                                       None
*/
static GT_STATUS prvTgfDxChPclQosProfileAndTrustModeSet
(
    IN GT_U32                               qosProfile,
    IN CPSS_QOS_PORT_TRUST_MODE_ENT         trustMode
)
{
    GT_STATUS           rc = GT_OK;
    CPSS_QOS_ENTRY_STC  portQosCfg;

    cpssOsMemSet(&portQosCfg, 0, sizeof(CPSS_QOS_ENTRY_STC));
    portQosCfg.qosProfileId = qosProfile;
    portQosCfg.enableModifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    portQosCfg.enableModifyUp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    portQosCfg.assignPrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQosCfg);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], trustMode);

    return rc;
}

/**
* @internal prvTgfPclUdbTestInit function
* @endinternal
*
* @brief   Set initial test settings:
*         - set VLAN entry
*         - set default port VLAN ID
*         - init PCL engine
*         - configure the User Defined Bytes
*         - sets the Policy rule
* @param[in] ruleIndex                - index of the rule in the TCAM
* @param[in] ruleFormat               - format of the Rule
* @param[in] packetType               - packet Type
* @param[in] offsetType               - the type of offset
* @param[in] useVrId                  - override VR ID
* @param[in] useQosProfile            - override QoS profile
* @param[in] packetInfoPtr            - (pointer to) packet fields info
*                                       None
*/
GT_VOID prvTgfPclUdbTestInit
(
    IN GT_U32                               ruleIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT          packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT          offsetType,
    IN GT_BOOL                              useVrId,
    IN GT_BOOL                              useQosProfile,
    IN TGF_PACKET_STC                      *packetInfoPtr
)
{
    GT_STATUS                         rc     = GT_OK;
    GT_U32                            udbIdx = 0;
    GT_U32                            udbAbsIdx = 0;
    GT_U32                            offset = 0;
    GT_U32                            startOffset = 0;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_ACTION_STC            action;
    PRV_TGF_PCL_OVERRIDE_UDB_STC      udbOverride;
    static GT_U8                      packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS];/* static to avoid stack overflow */
    GT_U32                            numOfBytesInPacketBuff;/* number of bytes in packetBuff*/ /* save space of CRC*/
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ethOthKey;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ipv4Key;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ipv6Key;
    GT_U32                            udbOffset;
    GT_U32                            packetByteOffset;
    GT_U32                            ipHdrChkSumOffset;
    GT_BOOL                           isIpv4;
    GT_BOOL                           isIpv6;
    PRV_TGF_PCL_OFFSET_TYPE_ENT       orig_offsetType = offsetType;


    /* check if FDB and VLAN init is needed */
    if (GT_FALSE == prvTgfIsVlanFdbInitialized)
    {
        /* get default vlanId */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        prvTgfIsVlanFdbInitialized = GT_TRUE;
    }

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d", prvTgfDevNum);

    /* default */
    ethOthKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ipv4Key   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    ipv6Key   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    isIpv4    = GT_FALSE;
    isIpv6    = GT_FALSE;
    /* override default */
    switch (packetType)
    {
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E:
            ipv4Key = ruleFormat;
            isIpv4  = GT_TRUE;
            break ;
        case PRV_TGF_PCL_PACKET_TYPE_IPV6_E:
            ipv6Key = ruleFormat;
            isIpv6  = GT_TRUE;
            break ;
        default:
        case PRV_TGF_PCL_PACKET_TYPE_MPLS_E:
        case PRV_TGF_PCL_PACKET_TYPE_UDE_E:
        case PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E:
            ethOthKey = ruleFormat;
            break ;
    }

    /* two bytes of IPV4 Header check sum must be bypassed */
    /* the value updated at egress before loopback         */
    /* 0xFF is out of PCL range - default value            */
    offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    ipHdrChkSumOffset =
        (isIpv4 != GT_FALSE) ? (offset + 10) : 0xFFFFFFFF;

    numOfBytesInPacketBuff = TGF_RX_BUFFER_MAX_SIZE_CNS - 4;/* save space of CRC*/
    /* build packet from input fields */
    rc = tgfTrafficEnginePacketBuild(packetInfoPtr, packetBuff,&numOfBytesInPacketBuff,NULL,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficEnginePacketBuild: %d", prvTgfDevNum);

    if (PRV_TGF_PCL_OFFSET_INVALID_E != offsetType)
    {
        /* init PCL Engine for send port */
        rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                  CPSS_PCL_LOOKUP_0_E,
                                  ethOthKey, ipv4Key, ipv6Key);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* calc offset */
    switch (offsetType)
    {
        case PRV_TGF_PCL_OFFSET_L2_E:
            offset = 0;
            startOffset = 0;
            break;

        case PRV_TGF_PCL_OFFSET_L3_E:
            /* begins just from ethertype */
            offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
            startOffset = 0;
            break;

        case PRV_TGF_PCL_OFFSET_L4_E:
            /* default */
            offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
            if (isIpv4 != GT_FALSE)
            {
                offset = TGF_L2_HEADER_SIZE_CNS
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS;
            }
            if (isIpv6 != GT_FALSE)
            {
                offset = TGF_L2_HEADER_SIZE_CNS
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS;
            }
            startOffset = 0;
            break;

        case PRV_TGF_PCL_OFFSET_L3_MINUS_2_E:
            /* begins just from ethertype */
            offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS - 2;
            startOffset = 0;
            break;

        case PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E:
            /* begins just from ethertype */
            offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS - 2;
            startOffset = 0;
            break;

        case PRV_TGF_PCL_OFFSET_INVALID_E:
            /* to invalidate UDBs */
            offset = 0;
            startOffset = 0;
            break;

        default:
            offset = 0;
            startOffset = 0;
            rc = GT_BAD_PARAM;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Get invalid offsetType: %d", offsetType);
    }

    /* clear config structures */
    cpssOsMemSet(&mask,        0, sizeof(mask));
    cpssOsMemSet(&pattern,     0, sizeof(pattern));
    cpssOsMemSet(&action,      0, sizeof(action));
    cpssOsMemSet(&udbOverride, 0, sizeof(udbOverride));

    /* Default state */

    if (useVrId != GT_FALSE)
    {
    /* set VRF Id 0 for default VLAN 5 */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_VLANID_CNS ,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d", prvTgfDevNum);
    }

    if ((useVrId != GT_FALSE) || (useQosProfile != GT_FALSE))
    {
        /* overriding of  User Defined Bytes */
        rc = prvTgfPclOverrideUserDefinedBytesSet(&udbOverride);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPclOverrideUserDefinedBytesSet: %d",
            prvTgfDevNum);
    }

    if (useQosProfile != GT_FALSE)
    {
    /* set 0 - Qos profile to SEND port */
        rc = prvTgfDxChPclQosProfileAndTrustModeSet(
            0, CPSS_QOS_PORT_TRUST_L2_L3_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfDxChPclQosProfileAndTrustModeSet: %d",
            prvTgfDevNum);
    }

    /* configure UBDs - "break" inside */
    for (udbIdx = 0; (1); udbIdx++)
    {
        udbOffset = startOffset + udbIdx;

        if  ((offset <= ipHdrChkSumOffset)
            && ((udbOffset + offset) >= ipHdrChkSumOffset))
        {
            /* bypass IPV4 header check sum */
            udbOffset += 2;
        }

        packetByteOffset = offset + udbOffset;

        if(UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) &&
           orig_offsetType == PRV_TGF_PCL_OFFSET_L2_E &&
            udbIdx >= TGF_L2_HEADER_SIZE_CNS)
        {
            /* assumption that L3 starts right after the 12B mac addresses + 2B etherType */
            offsetType = PRV_TGF_PCL_OFFSET_L3_MINUS_2_E;
            udbOffset -= TGF_L2_HEADER_SIZE_CNS;
        }

        rc = prvTgfPclUdbIndexConvert(
            ruleFormat, udbIdx, &udbAbsIdx);
        if (rc != GT_OK)
        {
            /* end of UDBs of the specified rule format */
            break;
        }

        /* configures the User Defined Byte */
        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType,
            CPSS_PCL_DIRECTION_INGRESS_E,
            udbAbsIdx /*udbIndex*/,
            offsetType, (GT_U8)udbOffset/*offset*/);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc,
            "prvTgfPclUserDefinedByteSet: Idx %d, OffType %d, Off %d\n",
            udbAbsIdx, offsetType, udbOffset);

        /* set pattern and mask */
        switch (ruleFormat)
        {
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                pattern.ruleStdUdb.udb[udbAbsIdx] = packetBuff[packetByteOffset];
                mask.ruleStdUdb.udb[udbAbsIdx]    = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
                pattern.ruleStdIpL2Qos.udb[udbAbsIdx] = packetBuff[packetByteOffset];
                mask.ruleStdIpL2Qos.udb[udbAbsIdx]    = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
                pattern.ruleStdIpv4L4.udb[udbAbsIdx] = packetBuff[packetByteOffset];
                mask.ruleStdIpv4L4.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
                pattern.ruleStdNotIp.udb[udbAbsIdx] = packetBuff[packetByteOffset];
                mask.ruleStdNotIp.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
                pattern.ruleExtNotIpv6.udb[udbAbsIdx] = packetBuff[packetByteOffset];
                mask.ruleExtNotIpv6.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                pattern.ruleExtUdb.udb[udbAbsIdx] = packetBuff[packetByteOffset];
                mask.ruleExtUdb.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
                pattern.ruleExtIpv6L2.udb[udbAbsIdx] = packetBuff[packetByteOffset];
                mask.ruleExtIpv6L2.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
                pattern.ruleExtIpv6L4.udb[udbAbsIdx] = packetBuff[packetByteOffset];
                mask.ruleExtIpv6L4.udb[udbAbsIdx] = 0xFF;
                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Get invalid ruleFormat: %d", ruleFormat);
        }
    }

    /* overriding of  User Defined Bytes if needed */
    if (GT_TRUE == useVrId || GT_TRUE == useQosProfile)
    {
        if (GT_TRUE == useVrId)
        {
            /* configure UDB override structure */
            switch (ruleFormat)
            {
                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
                    udbOverride.vrfIdLsbEnableStdNotIp = GT_TRUE;
                    udbOverride.vrfIdMsbEnableStdNotIp = GT_TRUE;
                    break;

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
                    udbOverride.vrfIdLsbEnableStdIpL2Qos = GT_TRUE;
                    udbOverride.vrfIdMsbEnableStdIpL2Qos = GT_TRUE;
                    break;

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
                    udbOverride.vrfIdLsbEnableStdIpv4L4 = GT_TRUE;
                    udbOverride.vrfIdMsbEnableStdIpv4L4 = GT_TRUE;
                    break;

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
                    udbOverride.vrfIdLsbEnableExtNotIpv6 = GT_TRUE;
                    udbOverride.vrfIdMsbEnableExtNotIpv6 = GT_TRUE;
                    break;

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
                    udbOverride.vrfIdLsbEnableExtIpv6L2 = GT_TRUE;
                    udbOverride.vrfIdMsbEnableExtIpv6L2 = GT_TRUE;
                    break;

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
                    udbOverride.vrfIdLsbEnableExtIpv6L4 = GT_TRUE;
                    udbOverride.vrfIdMsbEnableExtIpv6L4 = GT_TRUE;
                    break;

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                    udbOverride.vrfIdLsbEnableStdUdb   = GT_TRUE;
                    udbOverride.vrfIdMsbEnableStdUdb   = GT_TRUE;
                    break;

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                    udbOverride.vrfIdLsbEnableExtUdb   = GT_TRUE;
                    udbOverride.vrfIdMsbEnableExtUdb   = GT_TRUE;
                    break;

                default:
                    rc = GT_BAD_PARAM;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Get invalid ruleFormat: %d", ruleFormat);
            }
        }

        if (GT_TRUE == useQosProfile)
        {
            /* configure UDB override structure */
            switch (ruleFormat)
            {

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                    udbOverride.qosProfileEnableStdUdb = GT_TRUE;
                    break;

                case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                    udbOverride.qosProfileEnableExtUdb = GT_TRUE;
                    break;

                default:
                    rc = GT_BAD_PARAM;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Get invalid ruleFormat: %d", ruleFormat);
            }
        }

        /* overriding of  User Defined Bytes */
        rc = prvTgfPclOverrideUserDefinedBytesSet(&udbOverride);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclOverrideUserDefinedBytesSet: %d", prvTgfDevNum);
    }

    if (GT_TRUE == useVrId)
    {
        /* overrige VRF Id related UDB values */
        switch (ruleFormat)
        {
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                pattern.ruleStdUdb.udb[0] = /* UDB0 */
                    (GT_U8)(PRV_TGF_VRFID_CNS & 0xFF);
                pattern.ruleStdUdb.udb[1] = /* UDB1 */
                    (GT_U8)((PRV_TGF_VRFID_CNS >> 8) & 0x0F);
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
                pattern.ruleStdIpL2Qos.udb[18] = /* UDB18 */
                    (GT_U8)(PRV_TGF_VRFID_CNS & 0xFF);
                pattern.ruleStdIpL2Qos.udb[19] = /* UDB19 */
                    (GT_U8)((PRV_TGF_VRFID_CNS >> 8) & 0x0F);
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
                pattern.ruleStdIpv4L4.udb[20] = /* UDB20 */
                    (GT_U8)(PRV_TGF_VRFID_CNS & 0xFF);
                pattern.ruleStdIpv4L4.udb[21] = /* UDB21 */
                    (GT_U8)((PRV_TGF_VRFID_CNS >> 8) & 0x0F);
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
                pattern.ruleStdNotIp.udb[15] = /* UDB15 */
                    (GT_U8)(PRV_TGF_VRFID_CNS & 0xFF);
                pattern.ruleStdNotIp.udb[16] = /* UDB16 */
                    (GT_U8)((PRV_TGF_VRFID_CNS >> 8) & 0x0F);
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
                pattern.ruleExtNotIpv6.udb[5] = /* UDB5 */
                    (GT_U8)(PRV_TGF_VRFID_CNS & 0xFF);
                pattern.ruleExtNotIpv6.udb[1] = /* UDB1 */
                    (GT_U8)((PRV_TGF_VRFID_CNS >> 8) & 0x0F);
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                pattern.ruleExtUdb.udb[1] = /* UDB1 */
                    (GT_U8)(PRV_TGF_VRFID_CNS & 0xFF);
                pattern.ruleExtUdb.udb[2] = /* UDB2 */
                    (GT_U8)((PRV_TGF_VRFID_CNS >> 8) & 0x0F);
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
                pattern.ruleExtIpv6L2.udb[11] = /* UDB11 */
                    (GT_U8)(PRV_TGF_VRFID_CNS & 0xFF);
                pattern.ruleExtIpv6L2.udb[6] = /* UDB6 */
                    (GT_U8)((PRV_TGF_VRFID_CNS >> 8) & 0x0F);
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
                pattern.ruleExtIpv6L4.udb[14] = /* UDB14 */
                    (GT_U8)(PRV_TGF_VRFID_CNS & 0xFF);
                pattern.ruleExtIpv6L4.udb[12] = /* UDB12 */
                    (GT_U8)((PRV_TGF_VRFID_CNS >> 8) & 0x0F);
                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Get invalid ruleFormat: %d", ruleFormat);
        }

        /* set tested VRF Id for IN TTI Action  */
        prvTgfTunnelTermEtherTypeVrfIdAssignConfigurationSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                             PRV_TGF_VRFID_CNS,
                                                             packetInfoPtr->partsArray[0].partPtr);
    }

    if (GT_TRUE == useQosProfile)
    {
        /* overrige Qos Profile Id related UDB values */
        switch (ruleFormat)
        {
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                pattern.ruleStdUdb.udb[2] = /* UDB2 */
                    (GT_U8)PRV_TGF_QOS_PROFILE_CNS;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                pattern.ruleExtUdb.udb[5] = /* UDB5 */
                    (GT_U8)PRV_TGF_QOS_PROFILE_CNS;
                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Get invalid ruleFormat: %d", ruleFormat);
        }

        /* set tested Qos profile to SEND port */
        rc = prvTgfDxChPclQosProfileAndTrustModeSet(
            PRV_TGF_QOS_PROFILE_CNS, CPSS_QOS_PORT_NO_TRUST_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfDxChPclQosProfileAndTrustModeSet: %d", prvTgfDevNum);
    }


    /* set action cmd */
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;

    if (PRV_TGF_PCL_OFFSET_INVALID_E != offsetType)
    {
        /* sets the Policy Rule */
        rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d", prvTgfDevNum);
    }
}
/**
* @internal prvTgfPclUdbTestInitExt function
* @endinternal
*
* @brief   Set initial test settings:
*         - set VLAN entry
*         - set default port VLAN ID
*         - init PCL engine
*         - configure all User Defined Bytes
*         - sets the Policy rule
* @param[in] ruleIndex                - index of the rule in the TCAM
* @param[in] ruleFormat               - format of the Rule
* @param[in] packetType               - packet Type
* @param[in] offsetType               - the type of offset
* @param[in] direction                - PCL Rule direction
* @param[in] packetInfoPtr            - (pointer to) packet fields info
*                                       None
*/
static GT_VOID prvTgfPclUdbTestInitExt
(
    IN GT_U32                               ruleIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT          packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT          offsetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN TGF_PACKET_STC                      *packetInfoPtr
)
{
    GT_STATUS                         rc     = GT_OK;
    GT_U32                            udbIdx = 0;
    GT_U32                            udbAbsIdx = 0;
    GT_U32                            offset = 0;
    GT_U32                            startOffset = 0;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_ACTION_STC            action;
    static GT_U8                      packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS];
    GT_U32                            numOfBytesInPacketBuff;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ethOthKey;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ipv4Key;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ipv6Key;
    GT_U32                            udbOffset;
    GT_U32                            packetByteOffset;
    GT_U32                            ipHdrChkSumOffset;
    GT_BOOL                           isIpv4;
    GT_BOOL                           isIpv6;
    PRV_TGF_PCL_OFFSET_TYPE_ENT       orig_offsetType = offsetType;



    /* check if FDB and VLAN init is needed */
    if (GT_FALSE == prvTgfIsVlanFdbInitialized)
    {
        /* get default vlanId */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK,rc,"prvTgfBrgVlanPortVidGet: %d, %d",
                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        prvTgfIsVlanFdbInitialized = GT_TRUE;
    }

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 prvTgfDevNum);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
               prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    /* default */
    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        ethOthKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        ipv4Key   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
        ipv6Key   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    }
    else
    {
        ethOthKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
        ipv4Key   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
        ipv6Key   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    }
    isIpv4    = GT_FALSE;
    isIpv6    = GT_FALSE;
    /* override default */
    switch (packetType)
    {
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E:
            ipv4Key = ruleFormat;
            isIpv4  = GT_TRUE;
            break ;
        case PRV_TGF_PCL_PACKET_TYPE_IPV6_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV6_TCP_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E:
        case PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E:
            ipv6Key = ruleFormat;
            isIpv6  = GT_TRUE;
            break ;
        default:
        case PRV_TGF_PCL_PACKET_TYPE_MPLS_E:
        case PRV_TGF_PCL_PACKET_TYPE_UDE_E:
        case PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E:
            ethOthKey = ruleFormat;
            break ;
    }

    /* two bytes of IPV4 Header check sum must be bypassed */
    /* the value updated at egress before loopback         */
    /* 0xFF is out of PCL range - default value            */
    offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    ipHdrChkSumOffset =
        (isIpv4 != GT_FALSE) ? (offset + 10) : 0xFFFFFFFF;

    numOfBytesInPacketBuff = TGF_RX_BUFFER_MAX_SIZE_CNS - 4;/*save space of CRC*/
    /* build packet from input fields */
    rc = tgfTrafficEnginePacketBuild(packetInfoPtr, packetBuff,
                                     &numOfBytesInPacketBuff, NULL, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficEnginePacketBuild: %d",
                                 prvTgfDevNum);

    if (PRV_TGF_PCL_OFFSET_INVALID_E != offsetType)
    {
        /* init PCL Engine for send port */
        rc = prvTgfPclDefPortInit(prvTgfPortsArray[direction],
                                  direction,
                                  CPSS_PCL_LOOKUP_0_E,
                                  ethOthKey, ipv4Key, ipv6Key);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
            prvTgfPortsArray[direction]);
    }

    /* calc offset */
    switch (offsetType)
    {
        case PRV_TGF_PCL_OFFSET_L2_E:
            offset = 0;
            startOffset = 0;
            break;

        case PRV_TGF_PCL_OFFSET_L3_E:
            /* begins just from ethertype */
            offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
            startOffset = 0;
            break;

        case PRV_TGF_PCL_OFFSET_L4_E:
            /* default */
            offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
            startOffset = 2;
            if (isIpv4 != GT_FALSE)
            {
                offset = TGF_L2_HEADER_SIZE_CNS
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS;
                startOffset = 8; /* UdpPart */
            }
            if (isIpv6 != GT_FALSE)
            {
                offset = TGF_L2_HEADER_SIZE_CNS
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS;
                startOffset = 21; /* Ipv6TcpPart */
            }
            break;

        case PRV_TGF_PCL_OFFSET_L3_MINUS_2_E:
            /* begins just from ethertype */
            offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS - 2;
            startOffset = 2;
            break;

        case PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E:
            /* begins just from ethertype */
            offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS - 2;
            startOffset = 0;
            break;

        case PRV_TGF_PCL_OFFSET_INVALID_E:
            /* to invalidate UDBs */
            offset = 0;
            startOffset = 0;
            break;

        default:
            offset = 0;
            startOffset = 0;
            rc = GT_BAD_PARAM;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"Get invalid offsetType: %d",
                                         offsetType);
    }

    /* clear config structures */
    cpssOsMemSet(&mask,        0, sizeof(mask));
    cpssOsMemSet(&pattern,     0, sizeof(pattern));
    cpssOsMemSet(&action,      0, sizeof(action));

    /* configure UDBs - "break" inside */
    for (udbIdx = 0; (1); udbIdx++)
    {
        udbOffset = startOffset + udbIdx;
        if  ((offset <= ipHdrChkSumOffset)
            && ((udbOffset + offset) >= ipHdrChkSumOffset))
        {
            /* bypass IPV4 header check sum */
            udbOffset += 2;
        }

        packetByteOffset = offset + udbOffset;

        if(UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) &&
           orig_offsetType == PRV_TGF_PCL_OFFSET_L2_E &&
            udbIdx >= TGF_L2_HEADER_SIZE_CNS)
        {
            /* assumption that L3 starts right after the 12B mac addresses + 2B etherType */
            offsetType = PRV_TGF_PCL_OFFSET_L3_MINUS_2_E;
            udbOffset -= TGF_L2_HEADER_SIZE_CNS;
        }

        rc = prvTgfPclUdbIndexExtConvert(
            ruleFormat, udbIdx, &udbAbsIdx);
        if (rc != GT_OK)
        {
            /* end of UDBs of the specified rule format */
            break;
        }

        /* configures the User Defined Byte */
        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType,
            direction,
            udbAbsIdx /*udbIndex*/,
            offsetType, (GT_U8)udbOffset/*offset*/);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc,
            "prvTgfPclUserDefinedByteSet: Idx %d, OffType %d, Off %d\n",
            udbAbsIdx, offsetType, udbOffset);

        /* set pattern and mask */
        switch (ruleFormat)
        {
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
                pattern.ruleStdNotIp.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleStdNotIp.udb[udbAbsIdx] = 0xFF;
                break;
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
                pattern.ruleStdIpL2Qos.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleStdIpL2Qos.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
                pattern.ruleStdIpv4L4.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleStdIpv4L4.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
                pattern.ruleStdIpv6Dip.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleStdIpv6Dip.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
                pattern.ruleExtNotIpv6.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleExtNotIpv6.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
                pattern.ruleExtIpv6L2.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleExtIpv6L2.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
                pattern.ruleExtIpv6L4.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleExtIpv6L4.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                pattern.ruleStdUdb.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleStdUdb.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
                pattern.ruleExtUdb.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleExtUdb.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
                pattern.ruleStdIpv4RoutedAclQos.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleStdIpv4RoutedAclQos.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
                pattern.ruleExtIpv4PortVlanQos.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleExtIpv4PortVlanQos.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
                pattern.ruleUltraIpv6PortVlanQos.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleUltraIpv6PortVlanQos.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
                pattern.ruleUltraIpv6RoutedAclQos.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleUltraIpv6RoutedAclQos.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
                pattern.ruleEgrStdNotIp.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleEgrStdNotIp.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
                pattern.ruleEgrStdIpL2Qos.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleEgrStdIpL2Qos.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
                pattern.ruleEgrStdIpv4L4.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleEgrStdIpv4L4.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
                pattern.ruleEgrExtNotIpv6.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleEgrExtNotIpv6.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
                pattern.ruleEgrExtIpv6L2.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleEgrExtIpv6L2.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
                pattern.ruleEgrExtIpv6L4.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleEgrExtIpv6L4.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
                pattern.ruleEgrExtIpv4RaclVacl.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleEgrExtIpv4RaclVacl.udb[udbAbsIdx] = 0xFF;
                break;

            case PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
                pattern.ruleEgrUltraIpv6RaclVacl.udb[udbAbsIdx] =
                                                 packetBuff[packetByteOffset];
                mask.ruleEgrUltraIpv6RaclVacl.udb[udbAbsIdx] = 0xFF;
                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                      "Get invalid ruleFormat: %d", ruleFormat);
        }
    }

    /* set action cmd */
    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* AUTODOC: add FDB entry with MAC  VLAN, ePort  */
        rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            GT_TRUE);
        action.egressPolicy =  GT_TRUE;
    }
    else
    {
        action.egressPolicy =  GT_FALSE;
    }

    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;

    if (PRV_TGF_PCL_OFFSET_INVALID_E != offsetType)
    {
        /* sets the Policy Rule */
        rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                     prvTgfDevNum);
    }
}

/**
* @internal prvTgfPclUdbTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclUdbTestPacketSend
(
    IN  GT_U32                         portNum,
    IN  TGF_PACKET_STC               *packetInfoPtr
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portsCount = prvTgfPortsNum;
    GT_U32          portIter   = 0;


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

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d", prvTgfDevNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* Delay 100 milliseconds - give to sent packet to pass all pilelines in PP */
    cpssOsTimerWkAfter(100);
}

/**
* @internal prvTgfPclUdbTestReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*
* @param[in] vlanId                   -  to be cleared
* @param[in] ruleSize                 - size of Rule
* @param[in] ruleIndex                - index of the rule to be invalidated in the TCAM
*                                       None
*/
static GT_VOID prvTgfPclUdbTestReset
(
    IN GT_U16                   vlanId,
    IN CPSS_PCL_RULE_SIZE_ENT   ruleSize,
    IN GT_U32                   ruleIndex
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;


    PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     = prvTgfDevNum;
    interfaceInfo.devPort.portNum    = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.lookupType             = PRV_TGF_PCL_LOOKUP_TYPE_SINGLE_E;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.externalLookup         = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(ruleSize, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d", prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, vlanId);
}

/**
* @internal prvTgfPclTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
* @param[in] prvTgfPacketInfoPtr      - (pointer to) the packet info
* @param[in] packetSize               - packet size
*                                       None
*/
static GT_VOID prvTgfPclTrafficGenerate
(
    IN TGF_PACKET_STC   *prvTgfPacketInfoPtr,
    IN GT_U32            packetSize
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* send packet to be matched */
    prvTgfPclUdbTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfPacketInfoPtr);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS == portIter) ? 1 : 0),
            /*Tx*/((PRV_TGF_SEND_PORT_IDX_CNS == portIter) ? 1 : 0),
            packetSize,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* AUTODOC: send packet to be not matched on port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:04:05, SA=06:07:08:09:0A:0B */
    prvTgfPclUdbTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketNotMatchInfo);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify to receive packet on all ports */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS == portIter) ? 1 : 0),
            /*Tx*/1,
            prvTgfPacketNotMatchInfo.totalLen,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}
/**
* @internal prvTgfPclUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
* @param[in] prvTgfPacketInfoPtr      - (pointer to) the packet info
* @param[in] prvTgfNotMatchPacketInfoPtr - (pointer to) the not match packet info
* @param[in] packetSize               - packet size
*                                       None
*/
static GT_VOID prvTgfPclUdbTrafficGenerate
(
    IN TGF_PACKET_STC   *prvTgfPacketInfoPtr,
    IN TGF_PACKET_STC   *prvTgfNotMatchPacketInfoPtr,
    IN GT_U32            packetSize
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* send packet to be matched */
    prvTgfPclUdbTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                               prvTgfPacketInfoPtr);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            /*Tx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            packetSize,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* AUTODOC: send packet to be not matched on port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:04:05, SA=06:07:08:09:0A:0B */
    prvTgfPclUdbTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                               prvTgfNotMatchPacketInfoPtr);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify to receive packet on all ports */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS == portIter) ? 1 : 0),
            /*Tx*/1,
            packetSize,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}
/**
* @internal prvTgfPclUdbEgressTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
* @param[in] prvTgfPacketInfoPtr      - (pointer to) the packet info
* @param[in] prvTgfNotMatchPacketInfoPtr - (pointer to) the not match packet info
*                                       None
*/
static GT_VOID prvTgfPclUdbEgressTrafficGenerate
(
    IN TGF_PACKET_STC   *prvTgfPacketInfoPtr,
    IN TGF_PACKET_STC   *prvTgfNotMatchPacketInfoPtr
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;
    GT_U16                          pVid;
    GT_BOOL                         pVidForce;
    GT_U32                          expectedRxPkt;

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    rc = prvTgfEthCounterLoopbackEgrPort(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*loopback*/, &pVid, &pVidForce);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCounterLoopbackEgrPort\n");

    /* send packet to be matched */
    prvTgfPclUdbTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                               prvTgfPacketInfoPtr);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* dropped packet */
        expectedRxPkt = 0;
        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            expectedRxPkt = 1;
        }

        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthIngressPacketsCounterCheck(
            prvTgfDevsArray[portIter],
            prvTgfPortsArray[portIter],
            expectedRxPkt);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthIngressPacketsCounterCheck: %d, %d\n",
            prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
    }

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* AUTODOC: send packet to be not matched on port 0 with: */
    /* AUTODOC:   DA=00:01:02:03:04:05, SA=06:07:08:09:0A:0B */
    prvTgfPclUdbTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                               prvTgfNotMatchPacketInfoPtr);

    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* passed packet */
        expectedRxPkt = 0;
        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            expectedRxPkt = 1;
        }
        if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
        {
            expectedRxPkt = 1;
        }

        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthIngressPacketsCounterCheck(
            prvTgfDevsArray[portIter],
            prvTgfPortsArray[portIter],
            expectedRxPkt);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthIngressPacketsCounterCheck: %d, %d\n",
            prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
    }

    rc = prvTgfEthCounterLoopbackEgrPort(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_FALSE /*loopback*/, &pVid, &pVidForce);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCounterLoopbackEgrPort\n");
}

/**
* @internal prvTgfPclAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 TCP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
* @param[in] prvTgfPacketInfoPtr      - (pointer to) the packet info
* @param[in] packetSize               - packet size
*                                       None
*/
GT_VOID prvTgfPclAdditionalTrafficGenerate
(
    IN TGF_PACKET_STC   *prvTgfPacketInfoPtr,
    IN GT_U32            packetSize
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;

    PRV_UTF_LOG0_MAC("======= Generating Traffic (UDB invalid) =======\n");

    /* send packet to be matched */
    prvTgfPclUdbTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfPacketInfoPtr);

    PRV_UTF_LOG0_MAC("======= Checking Counters (UDB invalid) =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify to receive packet on all ports */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS == portIter) ? 1 : 0),
            /*Tx*/1,
            packetSize,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfPclAdditionalEgressTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 TCP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
* @param[in] prvTgfPacketInfoPtr      - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclAdditionalEgressTrafficGenerate
(
    IN TGF_PACKET_STC   *prvTgfPacketInfoPtr
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;
    GT_U16                          pVid;
    GT_BOOL                         pVidForce;
    GT_U32                          expectedRxPkt;

    PRV_UTF_LOG0_MAC("======= Generating Traffic (UDB invalid) =======\n");

    rc = prvTgfEthCounterLoopbackEgrPort(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*loopback*/, &pVid, &pVidForce);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCounterLoopbackEgrPort\n");

    /* send packet to be matched */
    prvTgfPclUdbTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfPacketInfoPtr);

    PRV_UTF_LOG0_MAC("======= Checking Counters (UDB invalid) =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* passed packet */
        expectedRxPkt = 0;
        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            expectedRxPkt = 1;
        }
        if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
        {
            expectedRxPkt = 1;
        }

        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthIngressPacketsCounterCheck(
            prvTgfDevsArray[portIter],
            prvTgfPortsArray[portIter],
            expectedRxPkt);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthIngressPacketsCounterCheck: %d, %d\n",
            prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);
    }

    rc = prvTgfEthCounterLoopbackEgrPort(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_FALSE /*loopback*/, &pVid, &pVidForce);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCounterLoopbackEgrPort\n");
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclIpv4TcpConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4TcpConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_UDB */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_UDB */
    /* AUTODOC:   packet type IPV4_TCP, idx [0..15] */
    /* AUTODOC:   offsetType PCL_OFFSET_L4, offset [0..15] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_UDB, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [34..49] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         GT_FALSE, GT_FALSE, &prvTgfTcpPacketInfo);
}

/**
* @internal prvTgfPclIpv4TcpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 TCP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 TCP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4TcpTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send TCP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclTrafficGenerate(
        &prvTgfTcpPacketInfo, PRV_TGF_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4TcpAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4TcpAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_UDB */
    /* AUTODOC:   packet type IPV4_TCP, idx [0..15] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..15] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfTcpPacketInfo);
}

/**
* @internal prvTgfPclIpv4TcpAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 TCP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4TcpAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send TCP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfTcpPacketInfo, PRV_TGF_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4TcpConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4TcpConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclIpv4UdpConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4UdpConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   packet type IPV4_UDP, idx [18, 19] */
    /* AUTODOC:   offsetType PCL_OFFSET_L4, offset [0..1] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [34,35] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         GT_FALSE, GT_FALSE, &prvTgfUdpPacketInfo);
}

/**
* @internal prvTgfPclIpv4UdpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4UdpTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclTrafficGenerate(
        &prvTgfUdpPacketInfo, PRV_TGF_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4UdpAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4UdpAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   packet type IPV4_UDP, idx [18, 19] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..1] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfUdpPacketInfo);
}

/**
* @internal prvTgfPclIpv4UdpAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4UdpAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdpPacketInfo, PRV_TGF_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4UdpConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4UdpConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclIpv4FragmentConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4FragmentConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_FRAGMENT, idx [20,21,22] */
    /* AUTODOC:   offsetType OFFSET_L3_MINUS_2, offset [0..2] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12,13,14] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
                         GT_FALSE, GT_FALSE, &prvTgfIpv4FragmentPacketInfo);
}

/**
* @internal prvTgfPclIpv4FragmentTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4FragmentTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 (DF) packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclTrafficGenerate(
        &prvTgfIpv4FragmentPacketInfo, PRV_TGF_IPV4_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4FragmentAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4FragmentAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_FRAGMENT, idx [20,21,22] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..2] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfIpv4FragmentPacketInfo);
}

/**
* @internal prvTgfPclIpv4FragmentAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4FragmentAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 (DF) packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv4FragmentPacketInfo, PRV_TGF_IPV4_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4FragmentConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4FragmentConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclIpv4OtherConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4OtherConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_UDB */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 15 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_UDB */
    /* AUTODOC:   packet type IPV4_OTHER, idx [0..15] */
    /* AUTODOC:   offsetType OFFSET_L3_MINUS_2, offset [0..11, 14..17] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_UDB, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..23, 26..29] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
                         GT_FALSE, GT_FALSE, &prvTgfIpv4OtherPacketInfo);
}

/**
* @internal prvTgfPclIpv4OtherTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4OtherTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclTrafficGenerate(
        &prvTgfIpv4OtherPacketInfo, PRV_TGF_IPV4_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4OtherAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4OtherAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 15 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_UDB */
    /* AUTODOC:   packet type IPV4_OTHER, idx [0..15] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..15] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfIpv4OtherPacketInfo);
}

/**
* @internal prvTgfPclIpv4OtherAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4OtherAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv4OtherPacketInfo, PRV_TGF_IPV4_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4OtherConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4OtherConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclEthernetOtherConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclEthernetOtherConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type IPV4_OTHER, idx [15,15,17] */
    /* AUTODOC:   offsetType OFFSET_L2, offset [0..2] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [0,1,2] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_L2_E,
                         GT_FALSE, GT_FALSE, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclEthernetOtherTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclEthernetOtherTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send Ethernet packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x2222 */
    prvTgfPclTrafficGenerate(
        &prvTgfEthernetPacketInfo, PRV_TGF_ETHERNET_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclEthernetOtherAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclEthernetOtherAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type IPV4_OTHER, idx [15,15,17] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..2] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclEthernetOtherAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclEthernetOtherAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send Ethernet packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x2222 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfEthernetPacketInfo, PRV_TGF_ETHERNET_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclEthernetOtherConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclEthernetOtherConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclMplsConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_MPLS_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclMplsConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 6 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   packet type MPLS, idx [0..5] */
    /* AUTODOC:   offsetType OFFSET_MPLS_MINUS_2, offset [0..5] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..17] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
                         PRV_TGF_PCL_PACKET_TYPE_MPLS_E, PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E,
                         GT_FALSE, GT_FALSE, &prvTgfMplsPacketInfo);
}

/**
* @internal prvTgfPclMplsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclMplsTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send MPLS packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   label=1, exp=0, stack=1, ttl=0x40 */
    prvTgfPclTrafficGenerate(
        &prvTgfMplsPacketInfo, PRV_TGF_MPLS_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclMplsAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_MPLS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclMplsAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 6 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   packet type MPLS, idx [0..5] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..5] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
                         PRV_TGF_PCL_PACKET_TYPE_MPLS_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfMplsPacketInfo);
}

/**
* @internal prvTgfPclMplsAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclMplsAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send MPLS packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   label=1, exp=0, stack=1, ttl=0x40 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfMplsPacketInfo, PRV_TGF_MPLS_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclMplsConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclMplsConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclUdeConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_UDE_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclUdeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_UDB */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_UDB */
    /* AUTODOC:   packet type UDE, idx [0..15] */
    /* AUTODOC:   offsetType OFFSET_L2, offset [0..15] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_UDB, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [0..15] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_UDE_E, PRV_TGF_PCL_OFFSET_L2_E,
                         GT_FALSE, GT_FALSE, &prvTgfUdePacketInfo);

    /* AUTODOC: set the EtherType 0x8888 to identify UDE */
    rc = prvTgfVntCfmEtherTypeSet(PRV_TGF_UDE_TAG_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntCfmEtherTypeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_UDE_TAG_CNS);
}

/**
* @internal prvTgfPclUdeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclUdeTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDE packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x8888 */
    prvTgfPclTrafficGenerate(
        &prvTgfUdePacketInfo, PRV_TGF_UDE_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclUdeAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_UDE_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclUdeAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_UDB */
    /* AUTODOC:   packet type UDE, idx [0..15] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..15] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_UDE_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclUdeAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclUdeAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDE packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x8888 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdePacketInfo, PRV_TGF_UDE_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclUdeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclUdeConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclIpv6ConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv6ConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    /* AUTODOC: configure 6 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L2 */
    /* AUTODOC:   packet type IPV6, idx [6..11] */
    /* AUTODOC:   offsetType OFFSET_L3_MINUS_2, offset [0..5] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L2, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..17] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
                         GT_FALSE, GT_FALSE, &prvTgfIpv6PacketInfo);
}

/**
* @internal prvTgfPclIpv6TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv6TrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    prvTgfPclTrafficGenerate(
        &prvTgfIpv6PacketInfo, PRV_TGF_IPV6_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv6AdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv6AdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 6 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L2 */
    /* AUTODOC:   packet type IPV6, idx [6..11] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..5] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfIpv6PacketInfo);
}

/**
* @internal prvTgfPclIpv6AdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv6AdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv6PacketInfo, PRV_TGF_IPV6_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv6ConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv6ConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclIpv6TcpConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv6TcpConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L4 */
    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L4 */
    /* AUTODOC:   packet type IPV6, idx [12,13,14] */
    /* AUTODOC:   offsetType OFFSET_L4, offset [0,1,2] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L4, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [54,55,56] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         GT_FALSE, GT_FALSE, &prvTgfIpv6TcpPacketInfo);
}

/**
* @internal prvTgfPclIpv6TcpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv6TcpTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclTrafficGenerate(
        &prvTgfIpv6TcpPacketInfo, PRV_TGF_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv6TcpAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv6TcpAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L4 */
    /* AUTODOC:   packet type IPV6, idx [12,13,14] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0,1,2] */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_FALSE, &prvTgfIpv6TcpPacketInfo);
}

/**
* @internal prvTgfPclIpv6TcpAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv6TcpAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv6TcpPacketInfo, PRV_TGF_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv6TcpConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv6TcpConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclEthernetOtherVridConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L2_E
*         - set useVrId to GT_TRUE
*         - set rule index to 0
*/
GT_VOID prvTgfPclEthernetOtherVridConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_UDB */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: set VRF Id 0 for default VLAN 5 */
    /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_UDB */
    /* AUTODOC:   packet type ETHERNET_OTHER, idx [0..15] */
    /* AUTODOC:   offsetType OFFSET_L2, offset [0..15] */

    /* AUTODOC: override User Defined Bytes with by predefined key: */
    /* AUTODOC:   standard UDB Key, VRF-ID's LSB in UDB0 */
    /* AUTODOC:   standard UDB Key, VRF-ID's MSB in UDB1 */
    /* AUTODOC: override VRF Id related UDB values */
    /* AUTODOC: set tested VRF Id for IN TTI Action: */
    /* AUTODOC:   port 0, VRF ID 0x0123 */

    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_UDB, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [0..15] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_L2_E,
                         GT_TRUE, GT_FALSE, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclEthernetOtherVridTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclEthernetOtherVridTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send Ethernet packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x2222 */
    prvTgfPclTrafficGenerate(
        &prvTgfEthernetPacketInfo, PRV_TGF_ETHERNET_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclEthernetOtherVridAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set useVrId to GT_TRUE
*         - set rule index to 0
*/
GT_VOID prvTgfPclEthernetOtherVridAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L4 */
    /* AUTODOC:   packet type IPV6, idx [12,13,14] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0,1,2] */

    /* AUTODOC: override User Defined Bytes with by predefined key: */
    /* AUTODOC:   standard UDB Key, VRF-ID's LSB in UDB0 */
    /* AUTODOC:   standard UDB Key, VRF-ID's MSB in UDB1 */
    /* AUTODOC: override VRF Id related UDB values */
    /* AUTODOC: set tested VRF Id for IN TTI Action: */
    /* AUTODOC:   port 0, VRF ID 0x0123 */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_TRUE, GT_FALSE, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclEthernetOtherVridAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclEthernetOtherVridAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send Ethernet packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x2222 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfEthernetPacketInfo, PRV_TGF_ETHERNET_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclEthernetOtherVridConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclEthernetOtherVridConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);

    /* AUTODOC: restore TTI config */
    prvTgfTunnelTermEtherTypeVrfIdAssignConfigRestore(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfPclIpv4UdpQosConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set useQosProfile to GT_TRUE
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4UdpQosConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_EXT_UDB */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */

    /* AUTODOC: set Qos Trust Mode QOS_PORT_TRUST_L2_L3 for ingess port */
    /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_UDB */
    /* AUTODOC:   packet type IPV4_UDP, idx [0..15] */
    /* AUTODOC:   offsetType OFFSET_L3_MINUS_2, offset [0..11, 14..17] */

    /* AUTODOC: override User Defined Bytes with by predefined key: */
    /* AUTODOC:   Extended UDB key, QoS profile in UDB5 */
    /* AUTODOC: set Qos Trust Mode QOS_PORT_NO_TRUST for ingess port */

    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_UDB, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..23, 26..29] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
                         GT_FALSE, GT_TRUE, &prvTgfUdpPacketInfo);
}

/**
* @internal prvTgfPclIpv4UdpQosTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4UdpQosTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclTrafficGenerate(
        &prvTgfUdpPacketInfo, PRV_TGF_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4UdpQosAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set useQosProfile to GT_TRUE
*         - set rule index to 0
*/
GT_VOID prvTgfPclIpv4UdpQosAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: set Qos Trust Mode QOS_PORT_TRUST_L2_L3 for ingess port */
    /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_UDB */
    /* AUTODOC:   packet type IPV4_UDP, idx [0..15] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..11, 14..17] */

    /* AUTODOC: override User Defined Bytes with by predefined key: */
    /* AUTODOC:   Extended UDB key, QoS profile in UDB5 */
    /* AUTODOC: set Qos Trust Mode QOS_PORT_NO_TRUST for ingess port */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         GT_FALSE, GT_TRUE, &prvTgfUdpPacketInfo);
}

/**
* @internal prvTgfPclIpv4UdpQosAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclIpv4UdpQosAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdpPacketInfo, PRV_TGF_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclIpv4UdpQosConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclIpv4UdpQosConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclRuleStdNotIpUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdNotIpUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType L3_MINUS_2 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [0,1,2] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleStdNotIpUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdNotIpUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send Ethernet packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x2222 */
    prvTgfPclUdbTrafficGenerate(
                   &prvTgfEthernetPacketInfo, &prvTgfEthernetNotMatchPacketInfo,
                   PRV_TGF_ETHERNET_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdNotIpUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdNotIpUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleStdNotIpUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdNotIpUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send Ethernet packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x2222 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfEthernetPacketInfo, PRV_TGF_ETHERNET_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdNotIpUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdNotIpUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}
/**
* @internal prvTgfPclRuleStdIpL2QosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - UDB bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleStdIpL2QosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbTrafficGenerate(
                          &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo,
                          PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdIpL2QosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleStdIpL2QosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdpLongPacketInfo, PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdIpL2QosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdIpL2QosUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclRuleStdIpv4L4UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4, cmd DROP_HARD */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleStdIpv4L4UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 (DF) packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclUdbTrafficGenerate(
                          &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo,
                          PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdIpv4L4UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP, idx [20,21,22] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..2] */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleStdIpv4L4UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 (DF) packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdpLongPacketInfo, PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdIpv4L4UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdIpv4L4UdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclRuleIngrStdUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleIngrStdUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_UDB */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_UDB */
    /* AUTODOC:   packet type IPV4_UDP, idx [0..15] */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_UDB, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..23, 26..29] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleIngrStdUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleIngrStdUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclUdbTrafficGenerate(
                         &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo,
                         PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleIngrStdUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleIngrStdUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 15 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_UDB */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
               PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
               CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleIngrStdUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleIngrStdUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdpLongPacketInfo, PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleIngrStdUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleIngrStdUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclRuleExtNotIpv6UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType OFFSET_L3_MINUS_2 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..17] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleExtNotIpv6UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send MPLS packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   label=1, exp=0, stack=1, ttl=0x40 */
    prvTgfPclUdbTrafficGenerate(
                    &prvTgfEthernetPacketInfo, &prvTgfEthernetNotMatchPacketInfo,
                    PRV_TGF_ETHERNET_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleExtNotIpv6UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
          PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_INVALID_E,
          CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleExtNotIpv6UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send MPLS packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   label=1, exp=0, stack=1, ttl=0x40 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfEthernetPacketInfo, PRV_TGF_ETHERNET_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleExtNotIpv6UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleExtNotIpv6UdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclRuleIngrExtUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleIngrExtUdbConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_UDB */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_UDB */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_UDB, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [0..15] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);

    /* AUTODOC: set the EtherType 0x8888 to identify UDE */
    rc = prvTgfVntCfmEtherTypeSet(PRV_TGF_UDE_TAG_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntCfmEtherTypeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_UDE_TAG_CNS);
}

/**
* @internal prvTgfPclRuleIngrExtUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleIngrExtUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDE packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x8888 */
    prvTgfPclUdbTrafficGenerate(
                          &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo,
                          PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleIngrExtUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleIngrExtUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_UDB */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleIngrExtUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleIngrExtUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDE packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x8888 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdpLongPacketInfo, PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleIngrExtUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleIngrExtUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclRuleExtIpv6L2UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L2 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L2, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..17] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleExtIpv6L2UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    prvTgfPclUdbTrafficGenerate(
                  &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpNotMatchPacketInfo,
                  PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleExtIpv6L2UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 6 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L2 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleExtIpv6L2UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleExtIpv6L2UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleExtIpv6L2UdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclRuleExtIpv6L4UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L4 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L4 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L4, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [54,55,56] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleExtIpv6L4UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbTrafficGenerate(
                  &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpNotMatchPacketInfo,
                  PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleExtIpv6L4UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV6_L4 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleExtIpv6L4UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleExtIpv6L4UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleExtIpv6L4UdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclRuleStdIpv6DipUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L4 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IPV6_DIP */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_IPV6_DIP, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [54,55,56] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleStdIpv6DipUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbTrafficGenerate(
                  &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpNotMatchPacketInfo,
                  PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdIpv6DipUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IPV6_DIP */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleStdIpv6DipUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdIpv6DipUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdIpv6DipUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}

/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L4 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_ULTRA_IPV6_PORT_VLAN_QOS */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_ULTRA_IPV6_PORT_VLAN_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [54,55,56] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbTrafficGenerate(
                  &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpNotMatchPacketInfo,
                  PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_ULTRA_IPV6_PORT_VLAN_QOS */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleUltraIpv6PortVlanQosUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}
/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L4 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [54,55,56] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbTrafficGenerate(
                  &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpNotMatchPacketInfo,
                  PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, PRV_TGF_LONG_IPV6_TCP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleUltraIpv6RoutedAclQosUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, 0);
}
/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IPV4_ROUTED_ACL_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_STD_IPV4_ROUTED_ACL_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [34,35] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbTrafficGenerate(
                          &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo,
                          PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_STD_IPV4_ROUTED_ACL_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdpLongPacketInfo, PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleStdIpv4RoutedAclQosUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}

/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_IPV4_PORT_VLAN_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_IPV4_PORT_VLAN_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [34,35] bytes */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbTrafficGenerate(
                          &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo,
                          PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_ULTRA_IPV6_PORT_VLAN_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_INGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be not matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalTrafficGenerate(
        &prvTgfUdpLongPacketInfo, PRV_TGF_LONG_UDP_PACKET_LEN_CNS);
}

/**
* @internal prvTgfPclRuleExtIpv4PortVlanQosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleExtIpv4PortVlanQosUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, 0);
}
/**
* @internal prvTgfPclRuleEgrStdNotIpUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType L3_MINUS_2 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_STD_NOT_IP, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [0,1,2] bytes */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdNotIpUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send Ethernet packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x2222 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfEthernetPacketInfo, &prvTgfEthernetNotMatchPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdNotIpUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdNotIpUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send Ethernet packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   EtherType=0x2222 */
    prvTgfPclAdditionalEgressTrafficGenerate(
        &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdNotIpUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrStdNotIpUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, EPCL_RULE_INDEX_CNS);
}
/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_STD_IP_L2_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - UDB bytes */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalEgressTrafficGenerate(
        &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdIpL2QosUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrStdIpL2QosUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, EPCL_RULE_INDEX_CNS);
}

/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IPV4_L4 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_STD_IPV4_L4, cmd DROP_HARD */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 (DF) packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_STD_IPV4_L4 */
    /* AUTODOC:   packet type IPV4_UDP, idx [20,21,22] */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID, offset [0..2] */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IPv4 packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv4 (DF) packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    prvTgfPclAdditionalEgressTrafficGenerate(
        &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrStdIpv4L4UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrStdIpv4L4UdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, EPCL_RULE_INDEX_CNS);
}


/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L3_MINUS_2_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType OFFSET_L3_MINUS_2 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_EXT_NOT_IPV6, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..17] bytes */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
                         PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send MPLS packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   label=1, exp=0, stack=1, ttl=0x40 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfEthernetPacketInfo, &prvTgfEthernetNotMatchPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E
*         - set key type to PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   packet type ETHERNET_OTHER */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,
          PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, PRV_TGF_PCL_OFFSET_INVALID_E,
          CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send MPLS packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   label=1, exp=0, stack=1, ttl=0x40 */
    prvTgfPclAdditionalEgressTrafficGenerate(
        &prvTgfEthernetPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtNotIpv6UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtNotIpv6UdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, EPCL_RULE_INDEX_CNS);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_EXT_IPV6_L2 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_EXT_IPV6_L2, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..17] bytes */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpNotMatchPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 6 User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_EXT_IPV6_L2 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    prvTgfPclAdditionalEgressTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, EPCL_RULE_INDEX_CNS);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L4 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_EXT_IPV6_L4 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_EXT_IPV6_L4, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [54,55,56] bytes */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpNotMatchPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_EXT_IPV6_L4 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalEgressTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L4UdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L4UdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, EPCL_RULE_INDEX_CNS);
}

/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_EXT_IPV4_PORT_VLAN_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_EXT_IPV4_PORT_VLAN_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [34,35] bytes */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 UDP packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfUdpLongPacketInfo, &prvTgfUdpNotMatchPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 2 User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_ULTRA_IPV6_PORT_VLAN_QOS */
    /* AUTODOC:   packet type IPV4_UDP */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 UDP packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send UDP packet to be not matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=22.22.22.22, dstIP=1.1.1.3 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalEgressTrafficGenerate(
        &prvTgfUdpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtIpv4RaclVaclUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_STD_E, EPCL_RULE_INDEX_CNS);
}

/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L4 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_ULTRA_IPV6_PORT_VLAN_QOS */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_ULTRA_IPV6_PORT_VLAN_QOS, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [54,55,56] bytes */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpNotMatchPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_INVALID_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration (UDB invalid) =======\n");

    /* AUTODOC: configure 3 User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_ULTRA_IPV6_PORT_VLAN_QOS */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType PCL_OFFSET_INVALID */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_INVALID_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbAdditionalTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    /* AUTODOC:   srcPort=8, dstPort=0 */
    prvTgfPclAdditionalEgressTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo);
}

/**
* @internal prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrUltraIpv6RaclVaclUdbConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, EPCL_RULE_INDEX_CNS);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_IPV6_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L4_E
*         - set rule index to 0
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   egress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    /* AUTODOC: configure User Defined Bytes (UDBs): */
    /* AUTODOC:   format EGRESS_EXT_IPV6_L2 */
    /* AUTODOC:   packet type IPV6 */
    /* AUTODOC:   offsetType OFFSET_L4 */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format EGRESS_EXT_IPV6_L2, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [12..17] bytes */
    prvTgfPclUdbTestInitExt(EPCL_RULE_INDEX_CNS, PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,
                         PRV_TGF_PCL_PACKET_TYPE_IPV6_E, PRV_TGF_PCL_OFFSET_L4_E,
                         CPSS_PCL_DIRECTION_EGRESS_E, &prvTgfIpv6TcpLongPacketInfoWrongIpVer);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:11:22:33:44:55,
*         macSa = 00:66:77:88:99:11,
*         Success Criteria:
*         Packet is captured on port 0
*         Send to device's port 0 Ethernet packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: send IPv6 packet to be matched on port 0 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:66:77:88:99:11 */
    /* AUTODOC:   srcIP=0016:0016:0016:0016:0016:0016:0016:0016 */
    /* AUTODOC:   dstIP=0001:0001:0001:0001:0001:0001:0001:0003 */
    prvTgfPclUdbEgressTrafficGenerate(
        &prvTgfIpv6TcpLongPacketInfo, &prvTgfIpv6TcpLongPacketInfoWrongIpVer);
}

/**
* @internal prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclRuleEgrExtIpv6L2UdbWrongIpVerConfigRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfPclUdbTestReset(PRV_TGF_VLANID_CNS, CPSS_PCL_RULE_SIZE_EXT_E, EPCL_RULE_INDEX_CNS);
}



