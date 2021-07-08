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
* @file prvTgfPclUdbFlds.c
*
* @brief Specific PCL features testing
*
* @version   33
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <pcl/prvTgfPclUdbFlds.h>

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS          2

/* number of common tests */
#define PRV_TGF_STD_EXT_NUM_CNS       5

/* number of STD tests */
#define PRV_TGF_STD_NUM_CNS           1

/* number of EXT tests */
#define PRV_TGF_EXT_NUM_CNS           1

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            1

/* second port number to send traffic to */
#define PRV_TGF_SECOND_SEND_PORT_IDX_CNS     3

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS         2

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS         0x55

#define PRV_UTF_VERIFY_RC1(rc, name)                                         \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
        rc1 = rc;                                                            \
    }

/******************************* packet 1 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};
/* VLAN_TAG part of packet1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket1VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                             /* pri, cfi, VlanId */
};

/* VLAN_TAG part of packet3 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket3VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    5, 0, 1                                              /* pri, cfi, VlanId */
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
    {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0x1c,               /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/* PARTS of packet IPv4 - packet3 */
static TGF_PACKET_PART_STC prvTgfPacket3PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket3VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacket3Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket3PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket3PartArray                                        /* partsArray */
};


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
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
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

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfUdpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket3VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketUdpIpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of UDP packet */
#define PRV_TGF_UDP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* UDP packet to send */
static TGF_PACKET_STC prvTgfUdpPacketInfo =
{
    PRV_TGF_UDP_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfUdpPacketPartArray) / sizeof(prvTgfUdpPacketPartArray[0]), /* numOfParts */
    prvTgfUdpPacketPartArray                                                /* partsArray */
};
/******************************************************************************/

/******************************* packet 2 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket2L2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* srcMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* dstMac */
};

/* DATA of packet */
static GT_U8 prvTgfPacket2PayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPacket2PayloadDataArr),            /* dataLength */
    prvTgfPacket2PayloadDataArr                     /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacket2L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacket2PayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacket2Info = {
    (TGF_L2_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket2PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket2PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket2PartArray                                        /* partsArray */
};

/****************************** packet ARP ************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketArpL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                 /* saMac */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketArpEtherTypePart = {
    TGF_ETHERTYPE_0806_ARP_TAG_CNS
};
/* packet's ARP header */
static TGF_PACKET_ARP_STC prvTgfPacketArpPart = {
    0x01, 0x800, 0x06, 0x04, 0x01,          /* hwType, protType, hwLen, protLen, opCode */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* srcMac */
    {0x01, 0x01, 0x01, 0x01},               /* srcIp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* dstMac */
    {0x00, 0x00, 0x00, 0x00}                /* dstIp */
};
/* packet's payload part */
static GT_U8 prvTgfPacketArpPayloadDataArr[] = {
    0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketArpPayloadPart = {
    sizeof(prvTgfPacketArpPayloadDataArr),               /* dataLength */
    prvTgfPacketArpPayloadDataArr                        /* dataPtr */
};

/* PARTS of packet ARP */
static TGF_PACKET_PART_STC prvTgfPacketArpArray[] = {
    {TGF_PACKET_PART_L2_E, &prvTgfPacketArpL2Part},     /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketArpEtherTypePart},
    {TGF_PACKET_PART_ARP_E, &prvTgfPacketArpPart},
    {TGF_PACKET_PART_PAYLOAD_E, &prvTgfPacketArpPayloadPart}
};

/* PACKET ARP to send */
static TGF_PACKET_STC prvTgfPacketArpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_ARP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketArpPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketArpArray) / sizeof(TGF_PACKET_PART_STC),     /* numOfParts */
    prvTgfPacketArpArray                                            /* partsArray */
};

/******************************* packet IPv4 *******************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketIpv4L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x04},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x44}                 /* saMac */
};
/* VLAN_TAG part of packet1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketIpv4VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                             /* pri, cfi, VlanId */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
    {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

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
    4,                  /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacketIpv4PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketIpv4PayloadPart = {
    sizeof(prvTgfPacketIpv4PayloadDataArr),                 /* dataLength */
    prvTgfPacketIpv4PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacketIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketIpv4L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketIpv4VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv4PayloadPart}
};

/* PACKET IPv4 to send */
static TGF_PACKET_STC prvTgfPacketIpv4Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketIpv4PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketIpv4PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv4PartArray                                        /* partsArray */
};

/******************************* packet IPv6 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketIpv6L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x05},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* packet's ethertype for IPv6 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};
/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                                       /* version */
    0,                                       /* trafficClass */
    0,                                       /* flowLabel */
    0x02,                                    /* payloadLen */
    0x3b,                                    /* nextHeader */
    0x40,                                    /* hopLimit */
    {0x1111, 0, 0, 0, 0, 0, 0x0000, 0x1111}, /* TGF_IPV6_ADDR srcAddr */
    {0x1111, 0, 0, 0, 0, 0, 0x0000, 0x3322}  /* TGF_IPV6_ADDR dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacketIpv6PayloadDataArr[] = {
    0x00, 0x01
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketIpv6PayloadPart = {
    sizeof(prvTgfPacketIpv6PayloadDataArr),                 /* dataLength */
    prvTgfPacketIpv6PayloadDataArr                          /* dataPtr */
};
/* PARTS of packet IPv6 */
static TGF_PACKET_PART_STC prvTgfPacketIpv6PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketIpv6L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv6PayloadPart}
};

/* PACKET IPv6 to send */
static TGF_PACKET_STC prvTgfPacketIpv6Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketIpv6PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketIpv6PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv6PartArray                                        /* partsArray */
};

/***************************** packet MAC-in-MAC ******************************/

/*-------- Backbone packet's part ----------*/

/* Backbone L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketMimBackboneL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x05},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                 /* saMac */
};

/* Backbone VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketMimBackboneVlanTagPart = {
    TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,                 /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/*-------- Service packet's part -----------*/

/* Service packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMimServiceEtherTypePart = {
    TGF_ETHERTYPE_88E7_MIM_TAG_CNS
};

/* Service Data of packet */
static GT_U8 prvTgfPacketMimServicePayloadDataArr[] = {
    0xa0, 0x12, 0x34, 0x56
};

/* Service Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketMimServicePayloadPart = {
    sizeof(prvTgfPacketMimServicePayloadDataArr), /* dataLength */
    prvTgfPacketMimServicePayloadDataArr          /* dataPtr */
};

/*-------- Packet part ---------------------*/

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketMimPartArray[] = { /* type, partPtr */
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketMimBackboneL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketMimBackboneVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMimServiceEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketMimServicePayloadPart},

    {TGF_PACKET_PART_L2_E,        &prvTgfPacketIpv4L2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketIpv4VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv4PayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS                                                 \
    2 * TGF_L2_HEADER_SIZE_CNS + 2 * TGF_VLAN_TAG_SIZE_CNS +                   \
    2 * TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS +                    \
    sizeof(prvTgfPacketMimServicePayloadDataArr) +                             \
    sizeof(prvTgfPacketIpv4PayloadDataArr)

/* PACKET MIM to send */
static TGF_PACKET_STC prvTgfPacketMimInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                         /* totalLen */
    sizeof(prvTgfPacketMimPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMimPartArray                                        /* partsArray */
};

/************************** packet IPv4 over IPv4 *****************************/

/* packet's Tunnel IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketTunnelIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};
/* packet's IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketTunnelIpv4OverIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x16,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacketTunnelPayloadDataArr[] = {
    0x00, 0x01
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketTunnelPayloadPart = {
    sizeof(prvTgfPacketTunnelPayloadDataArr),                       /* dataLength */
    prvTgfPacketTunnelPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketTunnelPartArray[] = { /* type, partPtr */
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketIpv4L2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketIpv4VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTunnelIpv4Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTunnelIpv4OverIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketTunnelPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_TUNNEL_LEN_CNS                                                 \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS +                           \
    TGF_ETHERTYPE_SIZE_CNS + 2 * TGF_IPV4_HEADER_SIZE_CNS +                    \
    sizeof(prvTgfPacketTunnelPayloadDataArr)

/* PACKET IPv4 over IPv4 to send */
static TGF_PACKET_STC prvTgfPacketTunnelIpv4OverIpv4Info = {
    PRV_TGF_PACKET_TUNNEL_LEN_CNS,                                            /* totalLen */
    sizeof(prvTgfPacketTunnelPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketTunnelPartArray                                        /* partsArray */
};


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId    = 0;

/* stored default MIM B-EtherType in TPID table */
static GT_U16   prvTgfDefEtherType = 0;

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

static const GT_CHAR prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "sending matched packet to port [%d] ...\n",
    "sending unmatched packet to port [%d] ...\n"
};
static TGF_PACKET_STC *prvTgfPacketInfoArray[] = {
    &prvTgfPacket1Info, &prvTgfPacket2Info
};

/* expected size of sent packets on PRV_TGF_RECEIVE_PORT_IDX_CNS port */
static GT_U32 prvTgfPacketsSize   = 0;

/* expected number of sent packets on PRV_TGF_RECEIVE_PORT_IDX_CNS port */
static GT_U8 prvTgfPacketsCountRx = 0;

/* expected number of transmitted packets on PRV_TGF_RECEIVE_PORT_IDX_CNS port */
static GT_U8 prvTgfPacketsCountTx = 2;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfDefConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfDefConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* create a vlan on ports (0,0), (0,8), (0,18), (0,23) */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* set default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    return rc;
};

/**
* @internal prvTgfMacToMeTableReset function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfMacToMeTableReset
(
    IN GT_VOID
)
{
    GT_STATUS                 rc, rc1 = GT_OK;
    PRV_TGF_TTI_MAC_VLAN_STC  macToMeValue, macToMeMask;
    GT_U32                    index;

    /* full mask for VLAN and MAC */
    macToMeMask.mac.arEther[0] =
    macToMeMask.mac.arEther[1] =
    macToMeMask.mac.arEther[2] =
    macToMeMask.mac.arEther[3] =
    macToMeMask.mac.arEther[4] =
    macToMeMask.mac.arEther[5] = 0xFF;
    macToMeMask.vlanId = 0xFFF;

    /* use 0 MAC DA and VID = 4095 */
    macToMeValue.mac.arEther[0] =
    macToMeValue.mac.arEther[1] =
    macToMeValue.mac.arEther[2] =
    macToMeValue.mac.arEther[3] =
    macToMeValue.mac.arEther[4] =
    macToMeValue.mac.arEther[5] = 0;
    macToMeValue.vlanId = 0xFFF;

    /* restore default of MAC to ME entry 0-7 */
    for (index = 0; (index < 8); index++)
    {
        rc = prvTgfTtiMacToMeSet(index, &macToMeValue, &macToMeMask);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiMacToMeSet");
    }

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfConfigurationRestore
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipKey
)
{
    CPSS_PCL_DIRECTION_ENT           direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    GT_U32   portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_STATUS rc, rc1 = GT_OK;

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily
        != CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* disable send port for IPv4 TTI lookup */
        rc = prvTgfTtiPortLookupEnableSet(
                                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiPortLookupEnableSet");
    }

    /* disable ingress policy on ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    if (ipKey == PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E)
    {
        /* invalidate standart PCL rules */
        rc = prvTgfPclRuleValidStatusSet(
                CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
    }
    else
    {
        /* invalidate extended PCL rules */
        rc = prvTgfPclRuleValidStatusSet(
                CPSS_PCL_RULE_SIZE_EXT_E, prvTgfPclRuleIndex, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d");

    /* restore default of MAC to ME entry 0-7 */
    prvTgfMacToMeTableReset();

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");


    /* enables ingress policy per devices */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1( rc, "prvTgfPclIngressPolicyEnable");

    /* enables ingress policy per port */
    rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_FALSE);
    PRV_UTF_VERIFY_RC1( rc, "prvTgfPclPortIngressPolicyEnable");

    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E;

    direction  = CPSS_PCL_DIRECTION_INGRESS_E;
    lookupNum  = CPSS_PCL_LOOKUP_0_E;
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            portNum, direction, lookupNum, 0 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_RC1( rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_0");


    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            portNum, direction, lookupNum, 1 /*sublookup*/, accessMode);
    PRV_UTF_VERIFY_RC1( rc, "prvTgfPclPortLookupCfgTabAccessModeSet 0_1");

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* configure accessMode for lookup 1 */
        direction  = CPSS_PCL_DIRECTION_INGRESS_E;
        lookupNum  = CPSS_PCL_LOOKUP_1_E;
        accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                portNum, direction, lookupNum, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_RC1( rc, "prvTgfPclPortLookupCfgTabAccessModeSet 1_0");
    };

    return rc1;
};

/**
* @internal prvTgfTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U32    sendIter;
    GT_U32     portNumSend;
    GT_U32     portNumReceive;
    CPSS_INTERFACE_INFO_STC portInterface;

    portNumSend    = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    portNumReceive = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* reset counters */
    rc = prvTgfCountersEthReset();
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCountersEthReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNumReceive;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    /* sending two packets - matched and unmatched */
    for (sendIter = 0; sendIter < PRV_TGF_MAX_ITER_CNS; sendIter++)
    {
        PRV_UTF_LOG1_MAC(prvTgfIterationNameArray[sendIter], portNumSend);

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, prvTgfPacketInfoArray[sendIter],
            1 /*burstCount*/, 0 /*numVfd*/, NULL);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNumSend);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
    }


    /* print captured packets from receive port */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNumReceive);
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, portNumReceive);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPortCapturedPacketPrint");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
    return rc;
};

/**
* @internal prvTgfTrafficGenerateTwoPorts function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficGenerateTwoPorts
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    sendIter;
    GT_U32     ports[2];

    ports[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ports[1] = prvTgfPortsArray[PRV_TGF_SECOND_SEND_PORT_IDX_CNS];

    /* reset counters */
    rc = prvTgfCountersEthReset();
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCountersEthReset");

    /* sending two packets - matched and unmatched */
    for (sendIter = 0; sendIter < PRV_TGF_MAX_ITER_CNS; sendIter++)
    {
        PRV_UTF_LOG1_MAC(
            prvTgfIterationNameArray[sendIter],
            ports[sendIter % 2]);

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, prvTgfPacketInfoArray[sendIter],
            1 /*burstCount*/, 0 /*numVfd*/, NULL);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, ports[sendIter % 2]);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
    }

    return rc;
};

/**
* @internal prvTgfTunnelTermMimInit function
* @endinternal
*
* @brief   Set initial test settings:
*         - Enable port 0 for MIM TTI lookup
*         - Set MIM TTI key lookup MAC mode to Mac DA
*         - Set TTI rule
*/
static GT_VOID prvTgfTunnelTermMimInit
(
    GT_VOID
)
{
    GT_STATUS              rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT   patt;
    PRV_TGF_TTI_RULE_UNT   mask;
    PRV_TGF_TTI_ACTION_STC action;

    /* clear entries */
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* set MIM EtherType */
    rc = prvTgfTtiMimEthTypeSet(TGF_ETHERTYPE_88E7_MIM_TAG_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMimEthTypeSet");

    /* get MIM B-EtherType in TPID table */
    rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E, 1, &prvTgfDefEtherType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanTpidEntryGet: 0x%x",
                                 TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);

    /* set MIM B-EtherType in TPID table */
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E, 1,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanTpidEntrySet: 0x%x",
                                 TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS);

    /* enable send port for MIM TTI lookup */
    rc = prvTgfTtiPortLookupEnableSet(
                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                            PRV_TGF_TTI_KEY_MIM_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* set MIM TTI key lookup MAC mode */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_MIM_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* configure TTI rule action */
    action.tunnelTerminate     = GT_TRUE;
    action.passengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E;
    action.command             = CPSS_PACKET_CMD_FORWARD_E;
    action.redirectCommand     = PRV_TGF_TTI_NO_REDIRECT_E;
    action.tag0VlanCmd         = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    action.tag0VlanId          = PRV_TGF_VLANID_CNS;
    action.tunnelStart         = GT_FALSE;

    action.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    action.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    action.interfaceInfo.devPort.portNum =
                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* configure TTI rule mask */
    cpssOsMemSet(&mask.mim.common.mac, 0xFF, sizeof(mask.mim.common.mac));
    mask.mim.common.pclId = 0x3FF;
    mask.mim.common.vid   = 0xFFF;
    mask.mim.iSid         = 0xFFFF;
    mask.mim.iUp          = 0x7;
    mask.mim.iDp          = 0xF;

    /* configure TTI rule pattern */
    cpssOsMemCpy(&patt.mim.common.mac, prvTgfPacketMimBackboneL2Part.daMac,
           sizeof(patt.mim.common.mac));
    patt.mim.common.vid   = PRV_TGF_VLANID_CNS;
    patt.mim.common.pclId = 4;
    patt.mim.iSid         = 0x123456;
    patt.mim.iUp          = 5;
    patt.mim.iDp          = 0;

    /* set TTI rule */
    rc = prvTgfTtiRuleSet(0, PRV_TGF_TTI_KEY_MIM_E, &patt, &mask, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
}

/**
* @internal prvTgfTunnelTermIpv4Init function
* @endinternal
*
* @brief   Set initial test settings:
*         - Enable port 0 for IPv4 TTI lookup
*         - Set IPv4 TTI key lookup MAC mode to Mac DA
*         - Set TTI rule
*/
static GT_VOID prvTgfTunnelTermIpv4Init
(
    GT_VOID
)
{
    GT_STATUS              rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT   patt;
    PRV_TGF_TTI_RULE_UNT   mask;
    PRV_TGF_TTI_ACTION_STC action;

    /* clear entries */
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* enable send port for IPv4 TTI lookup */
    rc = prvTgfTtiPortLookupEnableSet(
                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                            PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* set IPv4 TTI key lookup MAC mode */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* configure TTI rule action */
    action.tunnelTerminate     = GT_TRUE;
    action.passengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4_E;
    action.command             = CPSS_PACKET_CMD_FORWARD_E;
    action.redirectCommand     = PRV_TGF_TTI_NO_REDIRECT_E;
    action.tag0VlanCmd         = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    action.tag0VlanId          = PRV_TGF_VLANID_CNS;
    action.tunnelStart         = GT_FALSE;

    action.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    action.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    action.interfaceInfo.devPort.portNum =
                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* configure TTI rule mask */
    cpssOsMemSet(&mask.ipv4.common.mac, 0xFF, sizeof(mask.ipv4.common.mac));
    mask.ipv4.common.pclId = 0x3FF;
    mask.ipv4.common.vid   = 0xFFF;

    /* configure TTI rule pattern */
    cpssOsMemCpy(&patt.ipv4.common.mac, prvTgfPacketIpv4L2Part.daMac,
           sizeof(patt.ipv4.common.mac));
    patt.ipv4.common.vid   = PRV_TGF_VLANID_CNS;
    patt.ipv4.common.pclId = 1;

    /* set TTI rule */
    rc = prvTgfTtiRuleSet(0, PRV_TGF_TTI_KEY_IPV4_E, &patt, &mask, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
}

/**
* @internal prvTgfTrafficPclOtherFeaturesConfig function
* @endinternal
*
* @brief   Configure other feature like MAC-to-ME
*/
static GT_STATUS prvTgfTrafficPclOtherFeaturesConfig
(
    IN PRV_TGF_PCL_RULE_FORMAT_ENT      fieldIndex
)
{
    GT_STATUS rc = GT_OK;
    switch (fieldIndex)
    {
        case PRV_TGF_PCL_RULE_FORMAT_MAC_TO_ME_E:
        {
            GT_U32 macToMeIndex = 0;
            PRV_TGF_TTI_MAC_VLAN_STC  macToMeValue, macToMeMask;

            /* restore default of MAC to ME entry 0-7 */
            prvTgfMacToMeTableReset();

            /* set o-th entry */
            macToMeMask.mac.arEther[0] =
            macToMeMask.mac.arEther[1] =
            macToMeMask.mac.arEther[2] =
            macToMeMask.mac.arEther[3] =
            macToMeMask.mac.arEther[4] =
            macToMeMask.mac.arEther[5] = 0xFF;
            macToMeMask.vlanId = 0; /* don't care VLAN */

            /* use MAC DA of matched packet */
            macToMeValue.mac.arEther[0] = prvTgfPacket1L2Part.daMac[0];
            macToMeValue.mac.arEther[1] = prvTgfPacket1L2Part.daMac[1];
            macToMeValue.mac.arEther[2] = prvTgfPacket1L2Part.daMac[2];
            macToMeValue.mac.arEther[3] = prvTgfPacket1L2Part.daMac[3];
            macToMeValue.mac.arEther[4] = prvTgfPacket1L2Part.daMac[4];
            macToMeValue.mac.arEther[5] = prvTgfPacket1L2Part.daMac[5];
            macToMeValue.vlanId = 0; /* don't care VLAN */


            /* configure MAC-to-ME rules */
            rc = prvTgfTtiMacToMeSet(macToMeIndex, &macToMeValue, &macToMeMask);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("! prvTgfTtiMacToMeSet: failed rc = %d\n", rc);
            }

            break;
        }
        case PRV_TGF_PCL_RULE_FORMAT_IS_L2_VALID_E:
        {
            prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);
            if (prvTgfPacketInfoArray[0] == &prvTgfPacketMimInfo)
                prvTgfTunnelTermMimInit();
            else
                prvTgfTunnelTermIpv4Init();
            prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

            break;
        }
        default:

            break;
    }
    return rc;
}

/**
* @internal prvTgfTrafficPclRulesDefine function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficPclRulesDefine
(
    IN PRV_TGF_PCL_RULE_FORMAT_ENT      fieldIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipKey,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT   *maskPtr,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT   *pattPtr,
    INOUT PRV_TGF_PCL_ACTION_STC        *actionPtr
)
{
    GT_STATUS rc = GT_OK;

    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC          *maskStdPtr = NULL;
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC          *pattStdPtr = NULL;
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_UDB_STC  *maskUdbPtr = NULL;
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_UDB_STC  *pattUdbPtr = NULL;
    PRV_TGF_PCL_RULE_FORMAT_EXT_UDB_STC         *maskExtUdbPtr;
    PRV_TGF_PCL_RULE_FORMAT_EXT_UDB_STC         *pattExtUdbPtr;

    cpssOsMemSet(maskPtr,   0, sizeof(*maskPtr));
    cpssOsMemSet(pattPtr,   0, sizeof(*pattPtr));
    cpssOsMemSet(actionPtr, 0, sizeof(*actionPtr));

    /* default state - may be overriden below */
    prvTgfPacketInfoArray[0] = &prvTgfPacket1Info;
    prvTgfPacketInfoArray[1] = &prvTgfPacket2Info;
    prvTgfPacketsSize        = prvTgfPacketInfoArray[0]->totalLen;
    prvTgfPacketsCountRx     = 1;
    prvTgfPacketsCountTx     = 1;

    /* tests send pare of packets: matched and unmatched */
    /* matched - dropped, unmatched forwarded            */
    if (PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E == ipKey)
    {
        maskStdPtr = &maskPtr->ruleStdUdb.commonStd;
        pattStdPtr = &pattPtr->ruleStdUdb.commonStd;

        maskUdbPtr = &maskPtr->ruleStdUdb.commonUdb;
        pattUdbPtr = &pattPtr->ruleStdUdb.commonUdb;
    }
    else if (PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E == ipKey) {
        maskStdPtr = &maskPtr->ruleExtUdb.commonStd;
        pattStdPtr = &pattPtr->ruleExtUdb.commonStd;

        maskUdbPtr = &maskPtr->ruleExtUdb.commonIngrUdb;
        pattUdbPtr = &pattPtr->ruleExtUdb.commonIngrUdb;
    }
    else
    {
        PRV_UTF_LOG1_MAC("!!! prvTgfTrafficSetPcl: Unsupported ipKey = %d !!!\n", ipKey);
        return GT_BAD_PARAM;
    }

    maskExtUdbPtr = &maskPtr->ruleExtUdb;
    pattExtUdbPtr = &pattPtr->ruleExtUdb;

    PRV_UTF_LOG0_MAC("------- commonIngrUdb.");

    /* to separate from TTI, PCL Id of TTI == 1 */
    maskStdPtr->pclId = 0x03FF;
    pattStdPtr->pclId = PRV_TGF_PCL_ID_CNS;

    switch (fieldIndex)
    {
        /* Note1: The value of the field depends of the packet only.
         * No additional configuration needed.
         */
        case PRV_TGF_PCL_RULE_FORMAT_VID_E:
            /* VLAN ID assigned to the packet */
            PRV_UTF_LOG0_MAC("vid");
            maskStdPtr->vid = 0x0FFF;
            pattStdPtr->vid = prvTgfPacket1VlanTagPart.vid;
            prvTgfPacketInfoArray[0] = &prvTgfPacket1Info;
            prvTgfPacketInfoArray[1] = &prvTgfPacket3Info;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_UP_E:
            /* The packet's 802.1p User Priority field */
            PRV_UTF_LOG0_MAC("up");
            maskStdPtr->up = 7;
            pattStdPtr->up = prvTgfPacket1VlanTagPart.pri;
            prvTgfPacketInfoArray[0] = &prvTgfPacket1Info;
            prvTgfPacketInfoArray[1] = &prvTgfPacket3Info;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_IS_IP_E:
            /* An indication that the packet is IP
             * 0 = Non IP packet. 1 = IPv4/6 packet
             */
            PRV_UTF_LOG0_MAC("isIp");
            maskStdPtr->isIp = 1;
            pattStdPtr->isIp = 1;
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv4Info;
            prvTgfPacketInfoArray[1] = &prvTgfPacketArpInfo;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_DSCP_OR_EXP_E:
            /* IP DSCP or MPLS EXP */
            PRV_UTF_LOG0_MAC("dscpOrExp");
            maskUdbPtr->dscpOrExp = 0x3F;
            pattUdbPtr->dscpOrExp = (GT_U8)
                ((prvTgfPacket1Ipv4Part.typeOfService >> 2) & 0x3F);
            break;

        case PRV_TGF_PCL_RULE_FORMAT_IS_UDB_VALID_E:
            /* Use L4 offset for not IP packet to cause invalid. */
            /* 0 = At least 1 user-defined byte couldn't be parsed.
             * 1 = All user-defined bytes used in this search key were successfully parsed.
             * The device may fail to extract a UDB due to various reasons:
             * - Invalid anchor type (e.g., L4 anchor applied to a non-IP packet)
             * - Out of packet (i.e., the anchor plus the offset point to a byte beyond the end of the packet)
             * - Out of parsing range (i.e., the anchor plus the offset point to a byte beyond the 128 bytes
             * capability of the parser)
             */
            PRV_UTF_LOG0_MAC("isUdbValid");
            maskUdbPtr->isUdbValid = 1;
            pattUdbPtr->isUdbValid = 1;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_PKT_TAGGING_E:
            /* The packet's VLAN Tag format. Valid when <IsL2Valid> = 1.
             * 0 = Untagged
             * 1 = Reserved.
             * 2 = PriorityTagged
             * 3 = IsTagged
             */
            PRV_UTF_LOG0_MAC("pktTagging");
            maskUdbPtr->pktTagging = 3;
            pattUdbPtr->pktTagging = 3;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_L3_OFFSET_INVALIDATE_E:
            /* Use LLC packet to cause invalid. */
            /* This flag indicates that the Layer 3 offset was successfully found.
             * 0=Valid; 1=Invalid
             * Layer 3 offset was not found, i.e., the packet is LLC-SNAP.
             */
            PRV_UTF_LOG0_MAC("l3OffsetInvalid");
            maskUdbPtr->l3OffsetInvalid = 1;
            pattUdbPtr->l3OffsetInvalid = 1;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_L4_PROTOCOL_TYPE_E:
            /* 0=Other/Ivalid; 1=TCP; 2=ICMP; 3=UDP; */
            PRV_UTF_LOG0_MAC("l4ProtocolType");
            maskUdbPtr->l4ProtocolType = 3;
            pattUdbPtr->l4ProtocolType = 3;
            prvTgfPacketInfoArray[0] = &prvTgfUdpPacketInfo;
            /* use non UDP IPV4 packet for not match */
            prvTgfPacketInfoArray[1] = &prvTgfPacketIpv4Info;

            break;
        case PRV_TGF_PCL_RULE_FORMAT_PKT_TYPE_E:
            /*  1 - IPv4 TCP,
             *  2 - IPv4 UDP,
             *  4 - MPLS,
             *  8 - IPv4 Fragment,
             * 16 - IPv4 Other,
             * 32 - Ethernet Other,
             * 64 - IPv6,
             *128 - UDE,
             */
            PRV_UTF_LOG0_MAC("pktType");
            maskUdbPtr->pktType = 0xFF;
            pattUdbPtr->pktType = 16;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_IP_HEADER_OK_E:
            /* Use IP version To cause invalid */
            /* 0 - invalid, 1 - valid
             * For IPv4 or IPv6 packets must be correct:
             * - The packet length >= 20 or 40 bytes
             * - The IP checksum
             * - The <version> field must be 4 or 6
             * - The IP header length field, <IHL> >= 20 or 40 bytes
             * - The <IP total length field> >= <IHL>*4
             * - DIP!= SIP
             * - The entire IPv4 or IPv4 header is within the first 128 bytes of the packet
             * NOTE: For non-IP packets the device sets this flag to a random value.
             * To configure the packet length check criteria,
             * set the <IPLength CheckMode> field in the
             * Policy Global Configuration Register (Table 1319 p. 1925).
             */
            PRV_UTF_LOG0_MAC("ipHeaderOk");
            maskUdbPtr->l4ProtocolType = 1;
            pattUdbPtr->l4ProtocolType = 1;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_MAC_DA_TYPE_E:
            /* 0 = Known and Unknown Unicast
             * 1 = Known and Unknown Multicast
             * 2 = NonARP BC; Not ARP Broadcast packet
             * 3 = ARP BC; ARP Broadcast packet
             */
            PRV_UTF_LOG0_MAC("macDaType");
            maskUdbPtr->macDaType = 3;
            pattUdbPtr->macDaType = 0;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_L4_OFFSET_INVALID_E:
            /* This flag indicates that the Layer 4 offset was successfully found.
             * 0=Valid; 1=Invalid
             * VALID if L3 is valid, and the L4 header can be parsed,
             * i.e. it is within min(packet length, 128B
             */
            PRV_UTF_LOG0_MAC("l4OffsetInvalid");
            maskUdbPtr->l4OffsetInvalid = 1;
            pattUdbPtr->l4OffsetInvalid = 1;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_L2_ENCAPSULATION_E:
            /* 0 = LLC not Snap; 1 = Ethernet V2; 2 = reserved; 3 = LLC Snap
             */
            PRV_UTF_LOG0_MAC("l2Encapsulation");
            maskUdbPtr->l2Encap = 3;
            pattUdbPtr->l2Encap = 1;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_EH_E:
            /* Indicates that an IPv6 extension exists
             * 0 = Non-IPv6 packet or IPv6 extension header does not exists.
             * 1 = Packet is IPv6 and extension header exists.
             */
            PRV_UTF_LOG0_MAC("isIpv6Eh");
            maskUdbPtr->isIpv6Eh = 1;
            pattUdbPtr->isIpv6Eh = 1;
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv6Info;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_HOP_BY_HOP_E:
            /* Indicates that the IPv6 Original Extension Header is hop-by-hop
             * 0 = Non-IPv6 packet or IPv6 extension header type is not Hop-by-Hop Option Header.
             * 1 = Packet is IPv6 and extension header type is Hop-by-Hop Option Header
             */
            PRV_UTF_LOG0_MAC("isIpv6HopByHop");
            maskUdbPtr->isIpv6HopByHop = 1;
            pattUdbPtr->isIpv6HopByHop = 1;
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv6Info;
            break;

        /* Note2: The value of field does not depend of packet
         * PCL Configuration Table or ingress port selection.
         */
        case PRV_TGF_PCL_RULE_FORMAT_PCL_ID_E:
            /* Set PCL ID in PCL Configuration table. */
            /* PCL-ID assigned to this lookup */
            PRV_UTF_LOG0_MAC("pclId");
            maskStdPtr->pclId = 0x03FF;
            pattStdPtr->pclId = (GT_U16)
                PRV_TGF_PCL_DEFAULT_ID_MAC(
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    CPSS_PCL_LOOKUP_0_E,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            prvTgfPacketsCountRx = 0;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_SOURCE_PORT_E:
            /* Use two ingress ports. */
            /* The port number from which the packet ingressed the device.
             * Port 63 is the CPU port
             */
            PRV_UTF_LOG0_MAC("sourcePort");
            maskStdPtr->sourcePort =
                (UTF_GET_MAX_PHY_PORTS_NUM_MAC(prvTgfDevNum) < 64)
                ? 0x3F : 0xFF;
            pattStdPtr->sourcePort =
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] &
                    maskStdPtr->sourcePort;
            prvTgfPacketsCountRx = 0;
            break;

        /* Note3: Needed complicated test scenario.
         * TODO:
         */
        case PRV_TGF_PCL_RULE_FORMAT_MAC_TO_ME_E:
            /* use TTI tests for MAC To ME */
            /* 1 - packet matched by MAC-To-Me lookup, 0 - not matched */
            PRV_UTF_LOG0_MAC("macToMe");
            maskStdPtr->macToMe = 1;
            pattStdPtr->macToMe = 1;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_IS_L2_VALID_E:
            /* use TTI test for "IP-Over-xxx is tunnel-terminated" */
            /* 0 = Layer 2 fields in the Ingress PCL not valid
             * 1 = Layer 2 field in the Ingress PCL key is valid.
             * Layer 2 fields are not valid in the IPCL keys
             * when IP-Over-xxx is tunnel-terminated and its key is based
             * on the passenger IP packet which does not included Layer 2 header.
             */
            PRV_UTF_LOG0_MAC("isL2Valid");
            maskUdbPtr->isL2Valid = 1;
            pattUdbPtr->isL2Valid = 0;

            prvTgfPacketInfoArray[0] = &prvTgfPacketTunnelIpv4OverIpv4Info;
            prvTgfPacketInfoArray[1] = &prvTgfPacket1Info;
            prvTgfPacketsSize        = prvTgfPacketInfoArray[1]->totalLen;

            break;

        /* std tests only */
        case PRV_TGF_PCL_RULE_FORMAT_IS_IPV4_E:
            /* 0 = Non IPv4 packet. 1 = IPv4 packet */
            PRV_UTF_LOG0_MAC("isIpv4");
            maskStdPtr->isIpv4 = 1;
            pattStdPtr->isIpv4 = 1;
            break;

        /* ext tests only */
        case PRV_TGF_PCL_RULE_FORMAT_IS_IPV6_E:
            /* 0 = Non IPv6 packet. 1 = IPv6 packet. */
            PRV_UTF_LOG0_MAC("isIpv6");
            maskExtUdbPtr->commonExt.isIpv6 = 1;
            pattExtUdbPtr->commonExt.isIpv6 = 1;
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv6Info;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_IP_PROTOCOL_E:
            /* IP protocol/Next Header type. */
            PRV_UTF_LOG0_MAC("ipProtocol");
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv6Info;
            maskExtUdbPtr->commonExt.ipProtocol = 0xFF;
            pattExtUdbPtr->commonExt.ipProtocol =
                (GT_U8)prvTgfPacketIpv6Part.nextHeader;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_31_0_E:
            /* IPV6 SIP bits 31:0 or IPV4 SIP */
            PRV_UTF_LOG0_MAC("sipBits31_0");
            cpssOsMemSet(maskExtUdbPtr->sipBits31_0, 0xFF,
                         sizeof(maskExtUdbPtr->sipBits31_0));
            pattExtUdbPtr->sipBits31_0[0] =
                (GT_U8)(prvTgfPacketIpv6Part.srcAddr[6] >> 8);
            pattExtUdbPtr->sipBits31_0[1] =
                (GT_U8)(prvTgfPacketIpv6Part.srcAddr[6] & 0xFF);
            pattExtUdbPtr->sipBits31_0[2] =
                (GT_U8)(prvTgfPacketIpv6Part.srcAddr[7] >> 8);
            pattExtUdbPtr->sipBits31_0[3] =
                (GT_U8)(prvTgfPacketIpv6Part.srcAddr[7] & 0xFF);
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv6Info;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_79_32_OR_MAC_SA_E:
            /* IPV6 SIP bits 79:32 for IPV6 packets
             * MAC_SA for not IPV6 packets
             */
            PRV_UTF_LOG0_MAC("sipBits79_32orMacSa");
            cpssOsMemSet(maskExtUdbPtr->macSaOrSipBits79_32, 0xFF,
                         sizeof(maskExtUdbPtr->macSaOrSipBits79_32));
            cpssOsMemCpy(pattExtUdbPtr->macSaOrSipBits79_32, prvTgfPacketIpv4L2Part.saMac,
                         sizeof(prvTgfPacketIpv4L2Part.saMac));
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv4Info;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_SIP_BITS_127_80_OR_MAC_DA_E:
            /* IPV6 SIP bits 127:80 for IPV6 packets
             * MAC_DA for not IPV6 packets
             */
            PRV_UTF_LOG0_MAC("sipBits127_80orMacDa");
            cpssOsMemSet(maskExtUdbPtr->macDaOrSipBits127_80, 0xFF,
                         sizeof(maskExtUdbPtr->macDaOrSipBits127_80));
            cpssOsMemCpy(pattExtUdbPtr->macDaOrSipBits127_80, prvTgfPacketIpv4L2Part.daMac,
                         sizeof(prvTgfPacketIpv4L2Part.daMac));
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv4Info;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_DIP_BITS_127_112_E:
            /* IPV6 DIP bits 127:112
             */
            PRV_UTF_LOG0_MAC("dipBits127_112");
            cpssOsMemSet(maskExtUdbPtr->dipBits127_112, 0xFF,
                         sizeof(maskExtUdbPtr->dipBits127_112));
            pattExtUdbPtr->dipBits127_112[0] =
                (GT_U8)(prvTgfPacketIpv6Part.dstAddr[0] >> 8);
            pattExtUdbPtr->dipBits127_112[1] =
                (GT_U8)(prvTgfPacketIpv6Part.dstAddr[0] & 0xFF);
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv6Info;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_DIP_BITS_31_0_E:
            /* IPV6 DIP bits 31:0 or IPV4 DIP
             */
            PRV_UTF_LOG0_MAC("dipBits31_0");
            cpssOsMemSet(maskExtUdbPtr->dipBits31_0, 0xFF,
                         sizeof(maskExtUdbPtr->dipBits31_0));
            pattExtUdbPtr->dipBits31_0[0] =
                (GT_U8)(prvTgfPacketIpv6Part.dstAddr[6] >> 8);
            pattExtUdbPtr->dipBits31_0[1] =
                (GT_U8)(prvTgfPacketIpv6Part.dstAddr[6] & 0xFF);
            pattExtUdbPtr->dipBits31_0[2] =
                (GT_U8)(prvTgfPacketIpv6Part.dstAddr[7] >> 8);
            pattExtUdbPtr->dipBits31_0[3] =
                (GT_U8)(prvTgfPacketIpv6Part.dstAddr[7] & 0xFF);
            prvTgfPacketInfoArray[0] = &prvTgfPacketIpv6Info;
            break;

        default:
            PRV_UTF_LOG1_MAC("!!! prvTgfPclUdbFldsTest: Unsupported fieldIndex = %d !!!\n", fieldIndex);
            return GT_BAD_PARAM;
    }

    if (PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E == ipKey)
        PRV_UTF_LOG0_MAC(" field, STD Key TEST -------\n");
    else
        PRV_UTF_LOG0_MAC(" field, EXT Key TEST -------\n");

    /* action deny */
    actionPtr->pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;

    return rc;
};


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclUdbFlds function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] fieldIndex               -
* @param[in] ipKey                    -
*                                       None
*/
GT_VOID prvTgfPclUdbFlds
(
    IN PRV_TGF_PCL_RULE_FORMAT_ENT      fieldIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipKey
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      patt;
    PRV_TGF_PCL_ACTION_STC           action;

    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    rc = prvTgfDefConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfDefConfigurationSet: %d", prvTgfDevNum);

    if ((ipKey != PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E)
        && (ipKey != PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, GT_FAIL, "key must be STD_UDB or EXT_UDB: %d", ipKey);
    }

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_ID_CNS,
        ipKey /*nonIpKey*/,
        ipKey /*ipv4Key*/,
        ipKey /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Set PCL rules
     */

    /* define mask, pattern and action */
    rc = prvTgfTrafficPclRulesDefine(fieldIndex, ipKey, &mask, &patt, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRulesDefine: %d, %d, %d",
        prvTgfDevNum, fieldIndex, ipKey);

    /* set PCL rule only if mask, patt and action will be correctly set */
    if (GT_OK == rc)
    {
        rc = prvTgfPclRuleSet(
            ipKey, prvTgfPclRuleIndex, &mask, &patt, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
            prvTgfDevNum, prvTgfPclRuleIndex, fieldIndex);
    }

    /* ---------------
     * 4. Configure other features like MAC-to-ME
    */
    rc = prvTgfTrafficPclOtherFeaturesConfig(fieldIndex);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclOtherFeaturesConfig: %d, %d, %d",
        prvTgfDevNum, fieldIndex, ipKey);

    /* -------------------------------------------------------------------------
     * 3. Generate traffic and Check counters
     */

    /* generate traffic */
    rc = prvTgfTrafficGenerate();

    /* check counter of receive port */
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPacketsCountRx, prvTgfPacketsCountTx,
        prvTgfPacketsSize - TGF_VLAN_TAG_SIZE_CNS, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "get another counters values.");

    /* -------------------------------------------------------------------------
     * 4. Restore configuration
     */
    rc = prvTgfConfigurationRestore(ipKey);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclUdbFldsTwoPorts function
* @endinternal
*
* @brief   Set test configuration sent packets from different ports
*
* @param[in] fieldIndex               -
* @param[in] ipKey                    -
*                                       None
*/
GT_VOID prvTgfPclUdbFldsTwoPorts
(
    IN PRV_TGF_PCL_RULE_FORMAT_ENT      fieldIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipKey
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      patt;
    PRV_TGF_PCL_ACTION_STC           action;

    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    rc = prvTgfDefConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfDefConfigurationSet: %d", prvTgfDevNum);

    if ((ipKey != PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E)
        && (ipKey != PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, GT_FAIL, "key must be STD_UDB or EXT_UDB: %d", ipKey);
    }

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_ID_CNS,
        ipKey /*nonIpKey*/,
        ipKey /*ipv4Key*/,
        ipKey /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* init PCL Engine for second send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SECOND_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_ID_CNS,
        ipKey /*nonIpKey*/,
        ipKey /*ipv4Key*/,
        ipKey /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Set PCL rules
     */

    /* difine mask, pattern and action */
    rc = prvTgfTrafficPclRulesDefine(fieldIndex, ipKey, &mask, &patt, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRulesDefine: %d, %d, %d",
        prvTgfDevNum, fieldIndex, ipKey);

    /* set PCL rule only if mask, patt and action will be correctly set */
    if (GT_OK == rc)
    {
        rc = prvTgfPclRuleSet(
            ipKey, prvTgfPclRuleIndex, &mask, &patt, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
            prvTgfDevNum, prvTgfPclRuleIndex, fieldIndex);
    }

    /* -------------------------------------------------------------------------
     * 3. Generate traffic and Check counters
     */

    /* generate traffic */
    rc = prvTgfTrafficGenerateTwoPorts();

    /* check counter of receive port */
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPacketsCountRx, prvTgfPacketsCountTx,
        prvTgfPacketsSize - TGF_VLAN_TAG_SIZE_CNS, 1);

    /* -------------------------------------------------------------------------
     * 4. Restore configuration
     */
    rc = prvTgfConfigurationRestore(ipKey);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);
}



