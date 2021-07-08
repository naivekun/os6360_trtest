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
* @file prvTgfCutThrough.c
*
* @brief CPSS Cut-Through
*
* @version   11
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <common/tgfIpGen.h>
#include <common/tgfCutThrough.h>
#include <common/tgfPclGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfOamGen.h>
#include <cutThrough/prvTgfCutThrough.h>
#include <l2mll/prvTgfL2MllUtils.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS                5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to receive traffic to */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS      1

/* another port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS 1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3

/* UP in packet */
#define PRV_TGF_UP_CNS                    0

/* UP in packet */
#define PRV_TGF_MRU_INDEX_CNS             0

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS      2

/* default MRU value */
#define PRV_TGF_DEF_MRU_VALUE_CNS         0x5F2

/* inlif index */
#define PRV_TGF_INLIF_INDEX_CNS           0x1000

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* maximal MRU value */
static GT_U32  prvTgfMaxMruValue = 0xFFFF;

/* the Arp Address index of the Router ARP Table */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the virtual router id */
static GT_U32        prvTgfVrId                = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* received port indexes array */
static GT_U8 prvTgfRcvPortsIdxArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_NEXTHOPE_PORT_IDX_CNS, PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS
};

/* expected number of packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 1},
    {1, 1, 0, 0},
    {1, 1, 1, 1},
    {1, 0, 0, 0}
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},          /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}           /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,               /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfIpv4PacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x40,                              /* totalLen */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0xC0,                              /* timeToLive */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr */
    { 1,  1,  1,  3}                   /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of IPV4 packet */
static TGF_PACKET_PART_STC prvTgfIpv4PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfIpv4PacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv4 packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +  \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* IPv4 PACKET to send */
static TGF_PACKET_STC prvTgfIpv4PacketInfo =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfIpv4PacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfIpv4PacketPartArray                                        /* partsArray */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfIpv6PacketEtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x40,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x2222, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of IPV6 packet */
static TGF_PACKET_PART_STC prvTgfIpv6PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfIpv6PacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv6 packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +  \
    TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* IPv6 PACKET to send */
static TGF_PACKET_STC prvTgfIpv6PacketInfo =
{
    PRV_TGF_IPV6_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfIpv6PacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfIpv6PacketPartArray                                        /* partsArray */
};

/* LLC packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfLlcPacketEtherTypePart = {0x0060};

/* PARTS of LLC packet */
static TGF_PACKET_PART_STC prvTgfLlcPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfLlcPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of LLC packet */
#define PRV_TGF_LLC_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +  \
    sizeof(prvTgfPayloadDataArr)

/* LLC PACKET to send */
static TGF_PACKET_STC prvTgfLlcPacketInfo =
{
    PRV_TGF_LLC_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfLlcPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfLlcPacketPartArray                                        /* partsArray */
};

/* UDE packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfUdePacketEtherTypePart = {0x3333};

/* PARTS of UDE packet */
static TGF_PACKET_PART_STC prvTgfUdePacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfUdePacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of UDE packet */
#define PRV_TGF_UDE_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +  \
    sizeof(prvTgfPayloadDataArr)

/* UDE PACKET to send */
static TGF_PACKET_STC prvTgfUdePacketInfo =
{
    PRV_TGF_LLC_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfUdePacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfUdePacketPartArray                                        /* partsArray */
};

/* IPV4 over MPLS*/

/* MPLS labels */
#define PRV_TGF_IPV4_O_MPLS_LABEL_CNS   49

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfMplsPacketEtherTypePart = {TGF_ETHERTYPE_8847_MPLS_TAG_CNS};

/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls1Part =
{
    PRV_TGF_IPV4_O_MPLS_LABEL_CNS,    /* label */
    1,                                /* experimental use */
    1,                                /* stack */
    0xFF                              /* timeToLive */
};

/* PARTS of the MPLS packet with L3 only passenger */
static TGF_PACKET_PART_STC prvTgfIpv4OverMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfMplsPacketEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls1Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_IPV4_OVER_MPLS_PACKET_LEN_CNS                                \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* Packet to send */
static TGF_PACKET_STC prvTgfIpv4OverMplsPacketInfo =
{
    PRV_TGF_IPV4_OVER_MPLS_PACKET_LEN_CNS,                    /* totalLen */
    sizeof(prvTgfIpv4OverMplsPacketPartArray)
        / sizeof(prvTgfIpv4OverMplsPacketPartArray[0]),        /* numOfParts */
    prvTgfIpv4OverMplsPacketPartArray                          /* partsArray */
};

/* PARTS of the MPLS packet with L2 in passenger */
static TGF_PACKET_PART_STC prvTgfIpv4WithL2OverMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfMplsPacketEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls1Part},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfIpv4PacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_IPV4_WITH_L2_OVER_MPLS_PACKET_LEN_CNS                            \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS + \
    + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS                            \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* Packet to send */
static TGF_PACKET_STC prvTgfIpv4WithL2OverMplsPacketInfo =
{
    PRV_TGF_IPV4_WITH_L2_OVER_MPLS_PACKET_LEN_CNS,                    /* totalLen */
    sizeof(prvTgfIpv4WithL2OverMplsPacketPartArray)
        / sizeof(prvTgfIpv4WithL2OverMplsPacketPartArray[0]),        /* numOfParts */
    prvTgfIpv4WithL2OverMplsPacketPartArray                          /* partsArray */
};

static CPSS_PCL_LOOKUP_NUMBER_ENT prvTgfPclLookUpNumArr[] =
{
    CPSS_PCL_LOOKUP_NUMBER_0_E,
    CPSS_PCL_LOOKUP_NUMBER_1_E,
    CPSS_PCL_LOOKUP_NUMBER_2_E,
    CPSS_PCL_LOOKUP_NUMBER_0_E
};

static CPSS_PCL_DIRECTION_ENT prvTgfPclDirectionArr[] =
{
    CPSS_PCL_DIRECTION_INGRESS_E,
    CPSS_PCL_DIRECTION_INGRESS_E,
    CPSS_PCL_DIRECTION_INGRESS_E,
    CPSS_PCL_DIRECTION_EGRESS_E,
};

/* To be filled in runtime */
static GT_U32 prvTgfCncBlocks[4];

/* CNC clients list */
static PRV_TGF_CNC_CLIENT_ENT prvTgfCncClients[] =
{
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E,   /* IPCL lookup 0 */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E, /* IPCL lookup 1 */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E,   /* IPCL lookup 2 */
    PRV_TGF_CNC_CLIENT_EGRESS_PCL_E              /* EPCL lookup   */
};

static TGF_IPV4_ADDR prvTgfDstIpArr[] =
{
    {1, 1, 1, 3}, /*[0]*/
    {1, 1, 1, 4}, /*[1]*/
    {1, 1, 1, 5}, /*[2]*/
    {1, 1, 1, 6}, /*[3]*/
    {1, 1, 1, 7}  /*[4]*/
};

/* Bypass configurations set */
static PRV_TGF_CUT_THROUGH_BYPASS_STC prvTgfPclBypassArr[] =
{
    {/*[0]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_FALSE, /* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E, /* IPCL mode */
        GT_FALSE,/* IOAM */
        GT_FALSE,/* EOAM */
        GT_FALSE /* MLL */
    },
    {/*[1]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_TRUE, /* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E, /* IPCL mode */
        GT_FALSE,/* IOAM */
        GT_FALSE,/* EOAM */
        GT_FALSE /* MLL */
    },

    {/*[2]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_FALSE,/* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E, /* IPCL mode */
        GT_FALSE,/* IOAM */
        GT_FALSE,/* EOAM */
        GT_FALSE /* MLL */
    },
    {/*[3]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_FALSE,/* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E, /* IPCL mode */
        GT_FALSE,/* IOAM */
        GT_FALSE,/* EOAM */
        GT_FALSE /* MLL */
    },
    {/*[4]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_FALSE,/* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E, /* IPCL mode */
        GT_FALSE,/* IOAM */
        GT_FALSE,/* EOAM */
        GT_FALSE /* MLL */
    }
};

/* Expected values for each bypass configuration set */
static GT_U8 prvTgfExpectedCncCounters[][5][4] =
{

    { /* No bypass enabled */
        {1, 0, 0, 0}, /*[0]*/
        {0, 1, 0, 0}, /*[1]*/
        {0, 0, 1, 0}, /*[2]*/
        {0, 0, 0, 1}, /*[3]*/
        {0, 0, 0, 0}  /*[4]*/
    },
    { /* Egress PCL */
        {1, 0, 0, 0}, /*[0]*/
        {0, 1, 0, 0}, /*[1]*/
        {0, 0, 1, 0}, /*[2]*/
        {0, 0, 0, 0}, /*[3]*/
        {0, 0, 0, 0}  /*[4]*/
    },
    { /* Ingress PCL One lookup */
        {0, 0, 0, 0}, /*[0]*/
        {0, 0, 0, 0}, /*[1]*/
        {0, 0, 1, 0}, /*[2]*/
        {0, 0, 0, 1}, /*[3]*/
        {0, 0, 0, 0}  /*[4]*/
    },
    { /* Ingress PCL Two lookups */
        {0, 0, 0, 0}, /*[0]*/
        {0, 1, 0, 0}, /*[1]*/
        {0, 0, 1, 0}, /*[2]*/
        {0, 0, 0, 1}, /*[3]*/
        {0, 0, 0, 0}  /*[4]*/
     },
    { /* Ingress PCL full bypass */
        {0, 0, 0, 0}, /*[0]*/
        {0, 0, 0, 0}, /*[1]*/
        {0, 0, 0, 0}, /*[2]*/
        {0, 0, 0, 1}, /*[3]*/
        {0, 0, 0, 0}  /*[4]*/
     }
};

/* Macros to odetermine element count in static array */
#define PRV_TGF_ELEMENTS_NUM(_x) (sizeof(_x)/sizeof((_x)[0]))

static  L2_MLL_ENTRY_SHORT_INFO_STC     testMllDb[] =
{
    {TEST_INDEX_BASE_MAC(0,0),       0,    GT_FALSE,   CPSS_INTERFACE_PORT_E,2},
    {TEST_INDEX_BASE_MAC(0,0),       0,    GT_TRUE,    CPSS_INTERFACE_PORT_E,3},
    {NOT_VALID_ENTRY_CNS,            0,    0,          0,                    0}
};

#define  TEST_MLL_DB_NUM_ENTRIES_CNS    ((sizeof(testMllDb))/(sizeof(testMllDb[0])))

static GT_U32 testMllDb_restoreInfo[TEST_MLL_DB_NUM_ENTRIES_CNS] =
{
    0
    /* filled in runtime according to actual MLL indexes used */
};

static GT_U32 prvTgfExpectedMllPackets[][4] =
{
    {1, 0, 1, 1},
    {1, 0, 0, 0}
};

static GT_U32 restoreIndexRange[4];

/* Policer bypassing expected counter values */
static GT_U32 expectedPolicerCounters[][3] =
{
    {1, 1, 2}, /* Bypass IPLR0 - FALSE */
    {0, 1, 2}, /* Bypass IPLR0 - TRUE */
    {1, 1, 2}, /* Bypass IPLR1 - FALSE */
    {1, 0, 2}, /* Bypass IPLR1 - TRUE */
    {1, 1, 2}, /* Bypass EPLR  - FALSE */
    {1, 1, 0}  /* Bypass EPLR  - TRUE */
};

/* OAM base flow ID */
#define PRV_TGF_BASEFLOW_ID_CNS            1024

/* IPCL/EPCL Action Flow ID*/
#define PRV_TGF_IPCL_ACTION_FLOW_ID_CNS    1025
#define PRV_TGF_EPCL_ACTION_FLOW_ID_CNS    1026

/* IPCL Rule index for OAM packet detection */
#define PRV_TGF_OAM_IPCL_RULE_IDX_CNS      0x0000

/* EPCL Rule index for OAM packet detection */
#define PRV_TGF_OAM_EPCL_RULE_IDX_CNS      0x0001

/* OAM table entry */
#define PRV_IOAM_TABLE_ENTRY_INDEX_CNS      (PRV_TGF_IPCL_ACTION_FLOW_ID_CNS -\
    PRV_TGF_BASEFLOW_ID_CNS)
#define PRV_EOAM_TABLE_ENTRY_INDEX_CNS      (PRV_TGF_EPCL_ACTION_FLOW_ID_CNS -\
    PRV_TGF_BASEFLOW_ID_CNS)

/* Bypass configurations set */
static PRV_TGF_CUT_THROUGH_BYPASS_STC prvTgfOamBypassArr[] =
{
    {/*[0]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_FALSE, /* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E, /* IPCL mode */
        GT_FALSE,/* IOAM */
        GT_FALSE,/* EOAM */
        GT_FALSE /* MLL */
    },
    {/*[1]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_FALSE, /* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E, /* IPCL mode */
        GT_TRUE,/* IOAM */
        GT_TRUE,/* EOAM */
        GT_FALSE /* MLL */
    },
    {/*[2]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_FALSE, /* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E, /* IPCL mode */
        GT_FALSE,/* IOAM */
        GT_TRUE,/* EOAM */
        GT_FALSE /* MLL */
    },
    {/*[3]*/
        GT_FALSE,/* Router */
        GT_FALSE,/* Ingress Policer 0 */
        GT_FALSE,/* Ingress Policer 1 */
        GT_FALSE,/* Egress Policer*/
        GT_FALSE, /* EgressPCL */
        PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E, /* IPCL mode */
        GT_TRUE,/* IOAM */
        GT_FALSE,/* EOAM */
        GT_FALSE /* MLL */
    }
};

static GT_U32 prvTgfExpectedOamCounters[][2] =
{
    {1, 1},
    {0, 0},
    {1, 0},
    {0, 1}
};

static struct {
    PRV_TGF_OAM_ENTRY_STC            egrEntry;
    PRV_TGF_OAM_ENTRY_STC            ingEntry;
    PRV_TGF_OAM_EXCEPTION_CONFIG_STC ingExcept;
    PRV_TGF_OAM_EXCEPTION_CONFIG_STC egrExcept;
    GT_U32                           egrBase;
    GT_U32                           ingBase;
} bypassOamRestore;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* ===================================================================== */

#define UTF_VERIFY_EQUAL0_STRING_NO_RETURN(e, r, s)                                     \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            utfFailureMsgLog(s, (GT_UINTPTR *)NULL, 0);                                 \
        }                                                                               \
    } while(0)

/* CUT THROUGH DEFAULTS */

static PRV_TGF_EARLY_PROCESSING_MODE_ENT cutThroughDefaultEarlyProcessingMode;
static GT_BOOL cutThroughDefaultPortCtEnable;
static GT_BOOL cutThroughDefaultPortUntaggedCtEnable;
static GT_BOOL cutThroughDefaultUpCtEnable;

static GT_VOID cutThroughDefaultsSet()
{
#ifdef GM_USED
    GT_U32 gmUsed = 1;
#else
    GT_U32 gmUsed = 0;
#endif

    if ((! PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
       ||     (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E  >= prvCpssDxchFalconCiderVersionGet())
       || /* gm*/ (1 == gmUsed))
    {
        cutThroughDefaultEarlyProcessingMode  = PRV_TGF_EARLY_PROCESSING_CUT_THROUGH_E;
        cutThroughDefaultPortCtEnable         = GT_FALSE;
        cutThroughDefaultPortUntaggedCtEnable = GT_FALSE;
        cutThroughDefaultUpCtEnable           = GT_FALSE;
    }
    else
    {
        cutThroughDefaultEarlyProcessingMode  = PRV_TGF_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E;
        cutThroughDefaultPortCtEnable         = GT_TRUE;
        cutThroughDefaultPortUntaggedCtEnable = GT_TRUE;
        cutThroughDefaultUpCtEnable           = GT_FALSE;
    }
}

GT_VOID prvTgfCutThroughPortsGenConfig
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8  up;

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCutThroughEarlyProcessingModeSet(PRV_TGF_EARLY_PROCESSING_CUT_THROUGH_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughEarlyProcessingModeSet");
    }
    rc = prvTgfCutThroughPortEnableSet(
        CPSS_CPU_PORT_NUM_CNS, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    rc = prvTgfCutThroughPortEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    rc = prvTgfCutThroughPortEnableSet(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    for (up = 0; (up < 8); up++)
    {
        /* AUTODOC: enable tagged packets with any UP to be Cut Through */
        rc = prvTgfCutThroughUpEnableSet(up, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughUpEnableSet");
    }
}

GT_VOID prvTgfCutThroughPortsGenRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8  up;

    cutThroughDefaultsSet();

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCutThroughEarlyProcessingModeSet(cutThroughDefaultEarlyProcessingMode);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughEarlyProcessingModeSet");
    }

    /* AUTODOC: disable Cut Through forwarding on port 0 */
    rc = prvTgfCutThroughPortEnableSet(
        CPSS_CPU_PORT_NUM_CNS,
        cutThroughDefaultPortCtEnable, cutThroughDefaultPortUntaggedCtEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    rc = prvTgfCutThroughPortEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        cutThroughDefaultPortCtEnable, cutThroughDefaultPortUntaggedCtEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    rc = prvTgfCutThroughPortEnableSet(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        cutThroughDefaultPortCtEnable, cutThroughDefaultPortUntaggedCtEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortEnableSet");

    for (up = 0; (up < 8); up++)
    {
        /* AUTODOC: disable tagged packets with any UP to be Cut Through */
        rc = prvTgfCutThroughUpEnableSet(up, cutThroughDefaultUpCtEnable);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughUpEnableSet");
    }
}

/* ===================================================================== */
/**
* @internal prvTgfCutThroughBypassRouterAndPolicerBaseConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Create 2 Vlan entries -- (port[0], port[1]) and (port[2], port[3]);
*         - Enable Cut-Through;
*         - Set MRU value for a VLAN MRU profile;
*         - Create FDB entry.
*/
GT_VOID prvTgfCutThroughBypassRouterAndPolicerBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_NEXTHOPE_VLANID_CNS);

    prvTgfCutThroughPortsGenConfig();

    /* AUTODOC: set max MRU for VLAN MRU profile 0 */
    rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, prvTgfMaxMruValue);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet: %d%d",
                                 PRV_TGF_MRU_INDEX_CNS, prvTgfMaxMruValue);

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_CNS;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];
    macEntry.isStatic                     = GT_TRUE;
    macEntry.daRoute                      = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCutThroughLttConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*                                       None
*/
static GT_VOID prvTgfCutThroughLttConfigurationSet
(
    GT_U8    sendPortIndex,
    GT_U32    nextHopPortNum
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;


    /* AUTODOC: SETUP ROUTE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    /* clear Inlif Entry */
    cpssOsMemSet(&inlifEntry, 0, sizeof(inlifEntry));

    /* fill Inlif Entry */
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
    inlifEntry.ipv4UcRouteEnable         = GT_TRUE;

    /* set Inlif Entry */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_EXTERNAL_E, PRV_TGF_INLIF_INDEX_CNS, &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: enable IPv4 Unicast Routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, sendPortIndex);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table 1 */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = nextHopPortNum;
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;

    /* AUTODOC: add UC route entry 3 with: */
    /* AUTODOC:   cmd=PACKET_CMD_ROUTE */
    /* AUTODOC:   nextHopVlan=6, nextHopPort=3, nextHopArpPtr=1 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvTgfVrId, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCutThroughPbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*                                       None
*/
static GT_VOID prvTgfCutThroughPbrConfigurationSet
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;


    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(sendPortNum,
                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                  CPSS_PCL_LOOKUP_1_E, /* PBR must be in lookup 1 ! */
                                  PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
                                  PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, sendPortNum);

    /* write ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* fill a nexthope info for the prefix */
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
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = nextHopPortNum;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvTgfVrId, ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfCutThroughBypassRouterAndPolicerRouteConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    /* set config for PBR or LTT routing mode */
    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
            /* set PBR configuration */
            prvTgfCutThroughPbrConfigurationSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            /* set LTT Route configuration */
            prvTgfCutThroughLttConfigurationSet(PRV_TGF_SEND_PORT_IDX_CNS,
                                                prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);
            break;

        default:
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM, "Invalid routing mode: %d", routingMode);
    }
}

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         - Send traffic;
*         - Check routing mechanism is working;
*         - Enable bypassing the Router and Ingress Policer engines;
*         - Send traffic;
*         - Check that routing mechanism is ignored.
*/
GT_VOID prvTgfCutThroughBypassRouterAndPolicerTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc         = GT_OK;
    GT_U32      portIter   = 0;
    GT_U32      packetIter = 0;
    GT_U32      packetLen  = 0;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate through 2 stages: */
    for (packetIter = 0; packetIter < PRV_TGF_SEND_PACKETS_NUM_CNS; packetIter++)
    {
        /* -------------------------------------------------------------------------
         * 1. Setup counters and enable capturing
         */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            /* reset ethernet counters */
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* -------------------------------------------------------------------------
         * 2. Generating Traffic
         */

        /* AUTODOC: send IPv4 packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
        /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.3 */
        rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                              &prvTgfIpv4PacketInfo, prvTgfBurstCount, 0, NULL,
                                              prvTgfDevNum,
                                              prvTgfPortsArray[prvTgfRcvPortsIdxArr[packetIter]],
                                              TGF_CAPTURE_MODE_MIRRORING_E, 10);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of prvTgfTransmitPacketsWithCapture: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* -------------------------------------------------------------------------
         * 3. Get Ethernet Counters
         */

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   Stage#0 - IPv4 packet on nextHop port 3 with: */
        /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:06 */
        /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.3 */
        /* AUTODOC:   Stage#1 - IPv4 packet on FDB port 1 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
        /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.3 */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            /* calculate packet length */
            packetLen = prvTgfIpv4PacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS * (portIter != PRV_TGF_SEND_PORT_IDX_CNS);

            /* ckeck ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        (GT_U8) ((portIter == PRV_TGF_SEND_PORT_IDX_CNS) ||
                                                 (portIter == prvTgfRcvPortsIdxArr[packetIter])),
                                        prvTgfPacketsCountRxTxArr[packetIter][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* print captured packet */
        rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[prvTgfRcvPortsIdxArr[packetIter]]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortCapturedPacketPrint: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfRcvPortsIdxArr[packetIter]]);

        /*  AUTODOC: enable bypassing Router and Ingress Policer engines for Stage#1 */
        rc = prvTgfCutThroughBypassRouterAndPolicerEnableSet(GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCutThroughBypassRouterAndPolicerEnableSet: %d",
                                     GT_TRUE);
    }
}

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassRouterAndPolicerConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPADDR   ipAddr;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvTgfVrId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: disable bypassing Router and Ingress Policer engines */
    rc = prvTgfCutThroughBypassRouterAndPolicerEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCutThroughBypassRouterAndPolicerEnableSet: %d",
                                 GT_FALSE);

    prvTgfCutThroughPortsGenRestore();

    /* AUTODOC: restore default MRU for VLAN MRU profile 0 */
    rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, 0x5F2);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet: %d%d",
                                 PRV_TGF_MRU_INDEX_CNS, PRV_TGF_DEF_MRU_VALUE_CNS);

    /* invalidate default VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* invalidate nexthop VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOPE_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_NEXTHOPE_VLANID_CNS);
}

/**
* @internal prvTgfCutThroughBypassRouterConfig function
* @endinternal
*
* @brief   Configure test
*/
GT_VOID prvTgfCutThroughBypassRouterConfig
(
    GT_VOID
)
{
    prvTgfMaxMruValue = PRV_TGF_DEF_MRU_VALUE_CNS;

    /* AUTODOC: Configure routing */
    prvTgfCutThroughBypassRouterAndPolicerBaseConfigurationSet();

    /* AUTODOC: set LTT entry for routing */
    prvTgfCutThroughLttConfigurationSet(PRV_TGF_SEND_PORT_IDX_CNS,
        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

}

/**
* @internal prvTgfCutThroughBypassRouterTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfCutThroughBypassRouterTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc         = GT_OK;
    GT_U32      portIter   = 0;
    GT_U32      packetIter = 0;
    GT_U32      packetLen  = 0;
    PRV_TGF_CUT_THROUGH_BYPASS_STC bypassMode;

    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: Iterate through 2 stages: */
    for (packetIter = 0; packetIter < PRV_TGF_SEND_PACKETS_NUM_CNS; packetIter++)
    {
        /* -------------------------------------------------------------------------
         * 1. Setup counters and enable capturing
         */
        /* reset ethernet counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d",
            prvTgfDevNum);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* -------------------------------------------------------------------------
         * 2. Generating Traffic
         */
        rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                          prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                          &prvTgfIpv4PacketInfo, prvTgfBurstCount, 0, NULL,
                          prvTgfDevNum,
                          prvTgfPortsArray[prvTgfRcvPortsIdxArr[packetIter]],
                          TGF_CAPTURE_MODE_MIRRORING_E, 10);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of prvTgfTransmitPacketsWithCapture: %d, &d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* -------------------------------------------------------------------------
         * 3. Get Ethernet Counters
         */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            /* calculate packet length */
            if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)
                && (portIter != PRV_TGF_SEND_PORT_IDX_CNS))
            {
                /* Cut Through calculated packet size */
                packetLen = 12/*MACs*/ + 2/*Ethertype*/ + prvTgfPacketIpv4Part.totalLen;
            }
            else
            {
                /* ingress packet size without VLAN tag */
                packetLen = prvTgfIpv4PacketInfo.totalLen
                    - (TGF_VLAN_TAG_SIZE_CNS * (portIter != PRV_TGF_SEND_PORT_IDX_CNS));
            }
            /* prvTgfEthCountersCheck function packetSize parameter not includes 4 bytes of CRC */
            /* minimal frame is 64 byte, last 4 bytes is CRC                                    */
            if (packetLen < 60)
            {
                packetLen = 60;
            }

            /* ckeck ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[portIter],
                    (GT_U8) ((portIter == PRV_TGF_SEND_PORT_IDX_CNS) ||
                    (portIter == prvTgfRcvPortsIdxArr[packetIter])),
                    prvTgfPacketsCountRxTxArr[packetIter][portIter],
                    packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "ERROR of prvTgfEthCountersCheck");
        }
        prvTgfCutThroughBypassModeGet(prvTgfDevNum, &bypassMode);
        bypassMode.bypassRouter = GT_TRUE;
        prvTgfCutThroughBypassModeSet(prvTgfDevNum, &bypassMode);
    }
    prvTgfCutThroughBypassModeGet(prvTgfDevNum, &bypassMode);
    bypassMode.bypassRouter = GT_FALSE;
    prvTgfCutThroughBypassModeSet(prvTgfDevNum, &bypassMode);

}

/**
* @internal prvTgfCutThroughBypassRouterRestore function
* @endinternal
*
* @brief   Restore old configuration
*/
GT_VOID prvTgfCutThroughBypassRouterRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPADDR   ipAddr;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvTgfVrId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    prvTgfCutThroughPortsGenRestore();

    /* AUTODOC: restore default MRU for VLAN MRU profile 0 */
    rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, 0x5F2);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet: %d%d",
                                 PRV_TGF_MRU_INDEX_CNS, PRV_TGF_DEF_MRU_VALUE_CNS);

    /* invalidate default VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* invalidate nexthop VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOPE_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_NEXTHOPE_VLANID_CNS);

}

/**
* @internal prvTgfCutThroughSetPclRule function
* @endinternal
*
* @brief   Set PCL rule, which filters out packet by specific IP address in IPv4
*         packet.
* @param[in] ruleIdx                  - rule index
* @param[in] ipAddr                   - IP address to filter out
* @param[in] direction                - ingress/egress PCL engine
* @param[in] lookupNum                - lookup kind, valid only for ingress PCL
*                                       None
*/
static GT_VOID prvTgfCutThroughSetPclRule
(
    GT_U32                     ruleIdx,
    TGF_IPV4_ADDR              ipAddr,
    CPSS_PCL_DIRECTION_ENT     direction,
    CPSS_PCL_LOOKUP_NUMBER_ENT lookupNum

)
{
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      patt;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_STATUS                        rc;
    GT_U32                           lookupId = 0; /* IPCL stage number */


    /* AUTODOC: Set action = HARD DROP */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    action.matchCounter.matchCounterIndex = 0;
    action.matchCounter.enableMatchCount = GT_TRUE;

    if(CPSS_PCL_DIRECTION_INGRESS_E == direction)
    {
        /* AUTODOC: Offset rule for a specific lookup kind */
        switch(lookupNum)
        {
            case CPSS_PCL_LOOKUP_NUMBER_0_E:
                lookupId = 0;
                break;
            case CPSS_PCL_LOOKUP_NUMBER_1_E:
                lookupId = 1;
                break;
            case CPSS_PCL_LOOKUP_NUMBER_2_E:
                lookupId = 2;
                break;
            default:
                /* NOT REACHED */
                break;
        }

        /* AUTODOC: Create mask for full IP address */
        cpssOsMemSet(&mask, 0, sizeof(mask));
        mask.ruleStdIpv4L4.common.pclId = 0x3FF;
        cpssOsMemSet(&mask.ruleStdIpv4L4.dip, 0xFF,
            sizeof(mask.ruleStdIpv4L4.dip));

        /* AUTODOC: Use ipAddress as key */
        cpssOsMemSet(&patt, 0, sizeof(patt));
        patt.ruleStdIpv4L4.common.pclId = (GT_U16)
            PRV_TGF_PCL_DEFAULT_ID_MAC(direction, lookupNum,
                                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]),
        cpssOsMemCpy(
            patt.ruleStdIpv4L4.dip.arIP,
            ipAddr,
            sizeof(TGF_IPV4_ADDR)
        );

        /* AUTODOC: Set rule */
        rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
            prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId, ruleIdx),
            &mask, &patt, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleSet: %d,  %d",
            prvTgfDevNum, ruleIdx
        );
    }
    else
    {
        /* Mark that action as one for EPCL */
        action.egressPolicy = GT_TRUE;

        /* AUTODOC: set mask for egress IPv4 adddress  */
        cpssOsMemSet(&mask, 0, sizeof(mask));
        mask.ruleEgrStdIpv4L4.common.pclId = 0x3FF;
        cpssOsMemSet(&mask.ruleEgrStdIpv4L4.dip, 0xFF,
            sizeof(mask.ruleEgrStdIpv4L4.dip));

        /* AUTODOC: set IPv4 address as key */
        cpssOsMemSet(&patt, 0, sizeof(patt));
        patt.ruleEgrStdIpv4L4.common.pclId = (GT_U16)
            PRV_TGF_PCL_DEFAULT_ID_MAC(direction, lookupNum,
                                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]),
        cpssOsMemCpy(
            patt.ruleEgrStdIpv4L4.dip.arIP,
            ipAddr,
            sizeof(TGF_IPV4_ADDR)
        );

        /* AUTODOC: Set rule */
        rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,
            prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ruleIdx),
            &mask, &patt, &action);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleSet: %d,  %d",
            prvTgfDevNum, ruleIdx
        );
    }
}

/**
* @internal prvTgfCutThroughBypassPclConfig function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfCutThroughBypassPclConfig
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       tagArray[] = {1,1,1,1};
    GT_U32      indexRangesBmp[4];
    GT_U32      ii;
    GT_U32      jj;

    /* AUTODOC: create VLAN 5*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
        NULL, tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryWithPortsSet: %d, %d", prvTgfDevNum,
        PRV_TGF_VLANID_CNS);

    /* AUTODOC: Init CNC blocks */
    indexRangesBmp[0] = 0xFF;
    indexRangesBmp[1] = 0;
    indexRangesBmp[2] = 0;
    indexRangesBmp[3] = 0;

    /* AUTODOC: Save default client range values */
    rc = prvTgfCncBlockClientRangesGet(
        prvTgfCncBlocks[0], prvTgfCncClients[0], restoreIndexRange);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
        "prvTgfCncBlockClientRangesSet");

    for(ii = 0; ii < PRV_TGF_ELEMENTS_NUM(prvTgfCncClients); ii++)
    {
        if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
        {
            switch (prvTgfCncClients[ii])
            {
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
                /* PCL stage is bypassed */
                continue;
            default:
                /* Do nothing */
                break;
            }
        }

        prvTgfCncBlocks[ii] = prvTgfCncFineTuningBlockAmountGet() - ii - 1;

        /* AUTODOC: Bind CNC client to counter block */
        rc = prvTgfCncBlockClientEnableSet(
            prvTgfCncBlocks[ii],
            prvTgfCncClients[ii],
            GT_TRUE
        );
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfCncBlockClientEnableSet");

        /* AUTODOC: Set index ranges for blocks */
        rc = prvTgfCncBlockClientRangesSet(
            prvTgfCncBlocks[ii], prvTgfCncClients[ii], indexRangesBmp);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfCncBlockClientRangesSet");
    }

    /* AUTODOC: Configure ports and PCL rules */
    for(jj = 0; jj < PRV_TGF_ELEMENTS_NUM(prvTgfPclLookUpNumArr); jj++)
    {
        /* Configure rules for IPCL */
        if(prvTgfPclDirectionArr[jj] == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            if ((prvTgfPclLookUpNumArr[jj] == CPSS_PCL_LOOKUP_NUMBER_2_E) &&
                (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass))
            {
                /* PCL stage is bypassed */
                continue;
            }

            /* Configuring rule */
            prvTgfCutThroughSetPclRule(
                jj + 1,
                prvTgfDstIpArr[jj],
                prvTgfPclDirectionArr[jj],
                prvTgfPclLookUpNumArr[jj]
            );

            /* Configuring port */
            rc = prvTgfPclDefPortInit(
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                prvTgfPclDirectionArr[jj],
                prvTgfPclLookUpNumArr[jj],
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E
            );
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum
            );
        }
        else /* Configure rules for EPCL */
        {
            /* Configure rule */
            prvTgfCutThroughSetPclRule(
                jj + 1,
                prvTgfDstIpArr[jj],
                prvTgfPclDirectionArr[jj],
                prvTgfPclLookUpNumArr[jj]
            );

            /* Configure port */
            rc = prvTgfPclDefPortInit(
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS],
                prvTgfPclDirectionArr[jj],
                prvTgfPclLookUpNumArr[jj],
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
            );
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum
            );            /* Configure port */
        }
    }

}

/**
* @internal prvTgfCutThroughBypassPclTrafficGenerate function
* @endinternal
*
* @brief   Generate and check testing traffic
*/
GT_VOID prvTgfCutThroughBypassPclTrafficGenerate
(
    GT_VOID
)
{
    GT_U32                  ii;
    GT_U32                  jj;
    GT_U32                  kk;
    GT_STATUS               rc;
    PRV_TGF_CNC_COUNTER_STC ctr;

    /* AUTODOC: Setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4PacketInfo,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: Iterate through bypass configurations */
    for(ii = 0; ii < PRV_TGF_ELEMENTS_NUM(prvTgfExpectedCncCounters); ii++)
    {
        /* AUTODOC: Setup bypassing */
        prvTgfCutThroughBypassModeSet(prvTgfDevNum, &prvTgfPclBypassArr[ii]);

        for(jj = 0; jj < PRV_TGF_ELEMENTS_NUM(prvTgfExpectedCncCounters[0]); jj++)
        {
            PRV_UTF_LOG2_MAC("\nStage -- %s, bypass -- %s\n",
                                prvTgfDstIpArr[jj][3] == 7 ?
                                    "None" :
                                    prvTgfPclDirectionArr[jj] == CPSS_PCL_DIRECTION_EGRESS_E ?
                                        "EPCL" :
                                    prvTgfPclLookUpNumArr[jj] == CPSS_PCL_LOOKUP_NUMBER_0_E ?
                                        "IPCL0" :
                                    prvTgfPclLookUpNumArr[jj] == CPSS_PCL_LOOKUP_NUMBER_1_E ?
                                        "IPCL1" :
                                    prvTgfPclLookUpNumArr[jj] == CPSS_PCL_LOOKUP_NUMBER_2_E ?
                                        "IPCL2" :
                                        "Unknown",
                                prvTgfPclBypassArr[ii].bypassEgressPcl ?
                                    "EPCL" :
                                    prvTgfPclBypassArr[ii].bypassIngressPcl ==
                                            PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E ?
                                        "None" :
                                    prvTgfPclBypassArr[ii].bypassIngressPcl ==
                                            PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E ?
                                        "IPCL One Lookup" :
                                    prvTgfPclBypassArr[ii].bypassIngressPcl ==
                                            PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E ?
                                        "IPCL Two Lookup" :
                                    prvTgfPclBypassArr[ii].bypassIngressPcl ==
                                            PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E ?
                                        "IPCL Full" :
                                        "Unknown");
            cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfDstIpArr[jj],
                sizeof(prvTgfDstIpArr[0]));

            /* AUTODOC: Transmit */
            /* Start transmitting */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvTgfStartTransmitingEth");

            /* AUTODOC: Check CNC counters */
            for(kk = 0; kk < PRV_TGF_ELEMENTS_NUM(prvTgfCncClients); kk++)
            {
                if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
                {
                    switch (prvTgfCncClients[kk])
                    {
                    case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
                    case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:
                    case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
                    case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
                    case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
                        /* PCL stage is bypassed */
                        continue;
                    default:
                        /* Do nothing */
                        break;
                    }
                }

                rc = prvTgfCncCounterGet(
                    prvTgfCncBlocks[kk],
                    0,
                    PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
                    &ctr
                );
                PRV_UTF_LOG2_MAC("CNC counter #%d: %d\n", kk,
                    ctr.packetCount.l[0]);
                UTF_VERIFY_EQUAL0_STRING_MAC(
                    prvTgfExpectedCncCounters[ii][jj][kk],
                    ctr.packetCount.l[0],
                    "Unexpected packet count"
                );
            }
        }
    }
    /* AUTODOC: Restore bypassing */
    prvTgfCutThroughBypassModeSet(prvTgfDevNum, &prvTgfPclBypassArr[0]);
}

/**
* @internal prvTgfCutThroughBypassPclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassPclRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 ii;
    GT_U32 ruleIdx;
    GT_U32 lookupNum = 0;

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
        prvTgfDevNum, GT_TRUE);

    /* AUTODOC: Reset ethernet counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfEthCountersReset: %d", prvTgfDevNum);

    for(ii = 0; ii < PRV_TGF_ELEMENTS_NUM(prvTgfCncClients); ii++)
    {
        if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
        {
            switch (prvTgfCncClients[ii])
            {
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
                /* PCL stage is bypassed */
                continue;
            default:
                /* Do nothing */
                break;
            }
        }

        /* AUTODOC: UnBind CNC client from counter block */
        rc = prvTgfCncBlockClientEnableSet(
            prvTgfCncBlocks[ii],
            prvTgfCncClients[ii],
            GT_FALSE
        );
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfCncBlockClientEnableSet");

        /* AUTODOC: Restore default client ranges for blocks */
        rc = prvTgfCncBlockClientRangesSet(
            prvTgfCncBlocks[ii], prvTgfCncClients[ii], restoreIndexRange);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfCncBlockClientRangesSet");
    }

    /* AUTODOC: Invalidate rules */
    for(ii = 0; ii < PRV_TGF_ELEMENTS_NUM(prvTgfPclDirectionArr); ii++)
    {
        if(CPSS_PCL_DIRECTION_INGRESS_E == prvTgfPclDirectionArr[ii])
        {
            if ((prvTgfPclLookUpNumArr[ii] == CPSS_PCL_LOOKUP_NUMBER_2_E) &&
                (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass))
            {
                /* PCL stage is bypassed */
                continue;
            }

            switch(prvTgfPclLookUpNumArr[ii])
            {
                case CPSS_PCL_LOOKUP_NUMBER_0_E:
                    lookupNum = 0;
                    break;
                case CPSS_PCL_LOOKUP_NUMBER_1_E:
                    lookupNum = 1;
                    break;
                case CPSS_PCL_LOOKUP_NUMBER_2_E:
                    lookupNum = 2;
                    break;
                default:
                    /* NOT REACHED */
                    break;
            }
            ruleIdx = prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupNum,ii);
        }
        else
        {
            ruleIdx = prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ii); /* Rule offset for EPCL*/
        }
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
            ruleIdx, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: "
            "Rule size = [%d], rule index = [%d]",
            CPSS_PCL_RULE_SIZE_STD_E, ruleIdx);
    }

    /* AUTODOC: Deconfigure PCL-related port configuration */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: Invalidate default VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfCutThroughBypassMllConfig function
* @endinternal
*
* @brief   Configure test
*/
GT_VOID prvTgfCutThroughBypassMllConfig
(
    GT_VOID
)
{
    GT_STATUS                            rc;
    GT_U32                               portsArray[1];

    /* AUTODOC: create VLAN 5 with untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
        NULL, NULL, 4);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryWithPortsSet: %d, %d", prvTgfDevNum,
        PRV_TGF_VLANID_CNS);

    /* AUTODOC: Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: create VIDX 1 with ports [1] */
    portsArray[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];
    rc = prvTgfBrgVidxEntrySet(1, portsArray, NULL, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
        prvTgfDevNum, 1);

    /* AUTODOC: Add FDB entry */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac,
        PRV_TGF_VLANID_CNS, 4096, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: "
        "%d, %d, %d", PRV_TGF_VLANID_CNS, 4096, GT_TRUE);

    /* AUTODOC: set LTT and L2 MLL entries needed by the test */
    prvTgfL2MllLConfigSet(testMllDb,testMllDb_restoreInfo,
        MLL_INDEX_TEST_MODE_REFERENCE_FROM_END_OF_TALBE_E);

}

/**
* @internal prvTgfCutThroughBypassMllTrafficGenerate function
* @endinternal
*
* @brief   Generate and check testing traffic
*/
GT_VOID prvTgfCutThroughBypassMllTrafficGenerate
(
    GT_VOID
)
{
    GT_U32                         portIter;
    GT_STATUS                      rc;
    PRV_TGF_CUT_THROUGH_BYPASS_STC mllBypass;
    GT_U32                         ii;

    for(ii = 0; ii < 2; ii++)
    {
        /* AUTODOC: Send Ethernet packet */
        prvTgfL2MllUtilsPacketSend(&prvTgfIpv4PacketInfo, prvTgfBurstCount,
            PRV_TGF_SEND_PORT_IDX_CNS);

        /* AUTODOC: Check counters */
        for(portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            if(PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[portIter],
                    prvTgfExpectedMllPackets[ii][portIter],
                    prvTgfExpectedMllPackets[ii][portIter],
                    PRV_TGF_IPV4_PACKET_LEN_CNS,
                    prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "Got unexpected counters");
            }
            else
            {
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[portIter], 0,
                    prvTgfExpectedMllPackets[ii][portIter],
                    PRV_TGF_IPV4_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS,
                    prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "Got unexpected counters");
            }
        }

        /* AUTODOC: Reset counters */
        prvTgfL2MllUtilsResetAllEthernetCounters();

        /* AUTODOC: Setup MLL bypassing */
        cpssOsMemSet(&mllBypass, 0, sizeof(mllBypass));
        mllBypass.bypassMll = GT_TRUE;
        rc = prvTgfCutThroughBypassModeSet(prvTgfDevNum, &mllBypass);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCutThroughBypassModeSet:"
            " %d", prvTgfDevNum);
    }
    /* AUTODOC: Setup MLL bypassing */
    mllBypass.bypassMll = GT_FALSE;
    rc = prvTgfCutThroughBypassModeSet(prvTgfDevNum, &mllBypass);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCutThroughBypassModeSet:"
        " %d", prvTgfDevNum);

}

/**
* @internal prvTgfCutThroughBypassMllRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassMllRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
        prvTgfDevNum, GT_TRUE);

    /* AUTODOC: Clear eVidx 1 */
    rc = prvTgfBrgVidxEntrySet(1, NULL, NULL, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
        prvTgfDevNum, 1);

    /* AUTODOC: Clear LTT and L2 MLL entries that were set by the test */
    prvTgfL2MllLConfigReset(testMllDb,testMllDb_restoreInfo);

    /* AUTODOC: Restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: Invalidate default VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

}

/**
* @internal prvTgfCutThroughBypassPolicerConfig function
* @endinternal
*
* @brief   Configure test
*/
GT_VOID prvTgfCutThroughBypassPolicerConfig
(
    GT_VOID
)
{
    GT_STATUS                            rc = GT_OK;
    GT_U32                               ii;
    PRV_TGF_POLICER_ENTRY_STC            policerEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT  policerMeterTbParams;

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
        prvTgfDevNum);

    /* AUTODOC: Enable Metering and Counting on all stages for two ports */
    for (ii = PRV_TGF_POLICER_STAGE_INGRESS_0_E; ii < PRV_TGF_POLICER_STAGE_NUM;
         ii++)
    {
        rc = prvTgfPolicerMeteringEnableSet(ii, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

        rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, ii,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerCountingModeSet: %d %d %d %d", prvTgfDevNum, ii,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);

        rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, ii,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerCountingModeSet: %d %d %d %d", prvTgfDevNum, ii,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_TRUE);

        cpssOsMemSet(&policerEntry, 0, sizeof(policerEntry));
        cpssOsMemSet(&policerMeterTbParams, 0, sizeof(policerMeterTbParams));
        policerEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
        policerEntry.modifyDp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        policerEntry.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        policerEntry.modifyExp  = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        policerEntry.modifyTc   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        policerEntry.modifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

        rc = prvTgfPolicerEntrySet(
            ii, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &policerEntry, &policerMeterTbParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerEntrySet: %d", prvTgfDevNum);

        rc = prvTgfPolicerEntrySet(
            ii, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS],
            &policerEntry, &policerMeterTbParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerEntrySet: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfCutThroughResetPolicerCounters function
* @endinternal
*
* @brief   Sets all color counters for IPLR0, IPLR1, EPLR to 0
*/
static GT_VOID prvTgfCutThroughResetPolicerCounters
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    jj;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;

    cpssOsMemSet(&prvTgfMngCntr, 0, sizeof(prvTgfMngCntr));

    for(ii = PRV_TGF_POLICER_STAGE_INGRESS_0_E; ii < PRV_TGF_POLICER_STAGE_NUM;
        ii++)
    {
        for(jj = PRV_TGF_POLICER_MNG_CNTR_GREEN_E;
            jj < PRV_TGF_POLICER_MNG_CNTR_DROP_E + 1; jj++)
        {
            rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, ii,
                PRV_TGF_POLICER_MNG_CNTR_SET0_E, jj, &prvTgfMngCntr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);
        }
    }
}

/**
* @internal prvTgfCutThroughGetPolicerCounters function
* @endinternal
*
* @brief   Reads sums of all color counters for IPLR0, IPLR1, EPLR to array
*
* @param[in] cntrsArr[]               - array, each element of which is sum of color counters for
*                                      corresponding policer.
*                                       None
*/
static GT_VOID prvTgfCutThroughGetPolicerCounters
(
    IN GT_U32 cntrsArr[]
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    jj;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;

    for(ii = PRV_TGF_POLICER_STAGE_INGRESS_0_E; ii < PRV_TGF_POLICER_STAGE_NUM;
        ii++)
    {
        /* AUTODOC: Get policer counters */
        cntrsArr[ii] = 0;
        for(jj = PRV_TGF_POLICER_MNG_CNTR_GREEN_E;
            jj < PRV_TGF_POLICER_MNG_CNTR_DROP_E; jj++)
        {
            rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, ii,
                PRV_TGF_POLICER_MNG_CNTR_SET0_E, jj, &prvTgfMngCntr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);
            cntrsArr[ii] += prvTgfMngCntr.packetMngCntr;
        }

    }
}

/**
* @internal prvTgfCutThroughBypassPolicerTrafficGenerate function
* @endinternal
*
* @brief   Generate and check testing traffic
*/
GT_VOID prvTgfCutThroughBypassPolicerTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    jj;
    GT_U32    cntrValues[PRV_TGF_POLICER_STAGE_NUM];
    PRV_TGF_CUT_THROUGH_BYPASS_STC bypassConfig;

    /* AUTODOC: Setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4PacketInfo,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    for(ii = PRV_TGF_POLICER_STAGE_INGRESS_0_E; ii < PRV_TGF_POLICER_STAGE_NUM;
        ii++)
    {
        for(jj = GT_FALSE; jj <= GT_TRUE; jj++)
        {
            PRV_UTF_LOG2_MAC("Stage -- %d, bypass -- %d\n", ii, jj);
            cpssOsMemSet(&bypassConfig, 0, sizeof(bypassConfig));
            switch(ii)
            {
                case PRV_TGF_POLICER_STAGE_INGRESS_0_E:
                    bypassConfig.bypassIngressPolicerStage0 = jj;
                    break;
                case PRV_TGF_POLICER_STAGE_INGRESS_1_E:
                    bypassConfig.bypassIngressPolicerStage1 = jj;
                    break;
                case PRV_TGF_POLICER_STAGE_EGRESS_E:
                    bypassConfig.bypassEgressPolicer = jj;
                    break;
            }

            rc = prvTgfCutThroughBypassModeSet(prvTgfDevNum, &bypassConfig);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfCutThroughBypassModeSet: %d", prvTgfDevNum);

            /* AUTODOC: Reset ethernet counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfEthCountersReset: %d", prvTgfDevNum);

            /* AUTODOC: Reset policer counters */
            prvTgfCutThroughResetPolicerCounters();

            /* AUTODOC: Send Packet from port portNum */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "ERROR of StartTransmitting: %d, %d", prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            cpssOsTimerWkAfter(10);

            /* AUTODOC: Get policer counters */
            prvTgfCutThroughGetPolicerCounters(cntrValues);
            PRV_UTF_LOG3_MAC("Counters: IPLR0=%d; IPLR1=%d; EPLR=%d\n",
                cntrValues[0], cntrValues[1], cntrValues[2]);

            /* AUTODOC: Check policer counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedPolicerCounters[ii * 2 + jj][0], cntrValues[0],
                "Unexpected counter IPLR0");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedPolicerCounters[ii * 2 + jj][1], cntrValues[1],
                "Unexpected counter IPLR1");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedPolicerCounters[ii * 2 + jj][2], cntrValues[2],
                "Unexpected counter EPLR");
        }
    }
    cpssOsMemSet(&bypassConfig, 0, sizeof(bypassConfig));
    rc = prvTgfCutThroughBypassModeSet(prvTgfDevNum, &bypassConfig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCutThroughBypassModeSet:"
        " %d", prvTgfDevNum);

}

/**
* @internal prvTgfCutThroughBypassPolicerRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassPolicerRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;

    /* AUTODOC: Reset ethernet counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d",
        prvTgfDevNum);

    /* AUTODOC: Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
        prvTgfDevNum, GT_TRUE);

    /* AUTODOC: Disable Metering and Counting on all stages for two ports */
    for (ii = PRV_TGF_POLICER_STAGE_INGRESS_0_E; ii < PRV_TGF_POLICER_STAGE_NUM;
         ii++)
    {
        rc = prvTgfPolicerMeteringEnableSet(ii, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

        rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, ii,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerCountingModeSet: %d %d %d %d", prvTgfDevNum, ii,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

        rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, ii,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerCountingModeSet: %d %d %d %d", prvTgfDevNum, ii,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    }

    /* AUTODOC: Invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d, %d", prvTgfDevNum,
        PRV_TGF_VLANID_CNS);

}

/**
* @internal prvTgfCutThroughBypassOamConfig function
* @endinternal
*
* @brief   Configure test
*/
GT_VOID prvTgfCutThroughBypassOamConfig
(
    GT_VOID
)
{
    GT_STATUS                         rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;
    PRV_TGF_OAM_ENTRY_STC             oamEntry;
    PRV_TGF_OAM_EXCEPTION_CONFIG_STC  oamExceptionConfig;

    /* AUTODOC: Create Vlan 5 on ports with indexs 0, 1, 2, 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
                    prvTgfPortsArray, NULL, NULL,
                    prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfBrgDefVlanEntryWithPortsSet: Vlan %d",
                    PRV_TGF_VLANID_CNS);

    /* AUTODOC: Enable OAM PCL rules on ports 0(IOAM), 1(EOAM) */
    /* Configuring sender port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E
    );
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum
    );

    /* Configuring flood port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
    );
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum
    );

    /* AUTODOC: Setup PCL rules for IOAM and EOAM */
    /* Set IOAM PCL rule */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable = GT_TRUE;
    action.oam.oamProfile       = 1;
    action.flowId               = PRV_TGF_IPCL_ACTION_FLOW_ID_CNS;
    action.egressPolicy         = GT_FALSE;

    /* Create mask for full IP address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&mask.ruleStdIpv4L4.sip, 0xFF,
        sizeof(mask.ruleStdIpv4L4.sip));

    /* Use IP address as key */
    cpssOsMemSet(&patt, 0, sizeof(patt));
    cpssOsMemCpy(
        patt.ruleStdIpv4L4.sip.arIP,
        prvTgfPacketIpv4Part.srcAddr,
        sizeof(TGF_IPV4_ADDR)
    );

    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0, PRV_TGF_OAM_IPCL_RULE_IDX_CNS),
        &mask, &patt, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d %d",
        prvTgfDevNum, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_OAM_IPCL_RULE_IDX_CNS);

    /* Set EOAM PCL rule */
    action.egressPolicy = GT_TRUE;
    action.flowId = PRV_TGF_EPCL_ACTION_FLOW_ID_CNS;

    /* Create mask for full IP address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&mask.ruleEgrStdIpv4L4.sip, 0xFF,
        sizeof(mask.ruleEgrStdIpv4L4.sip));

    /* Use IP address as key */
    cpssOsMemSet(&patt, 0, sizeof(patt));
    cpssOsMemCpy(
        patt.ruleEgrStdIpv4L4.sip.arIP,
        prvTgfPacketIpv4Part.srcAddr,
        sizeof(TGF_IPV4_ADDR)
    );

    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,
        prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_OAM_EPCL_RULE_IDX_CNS),
        &mask, &patt, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d %d",
        prvTgfDevNum, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_OAM_EPCL_RULE_IDX_CNS);

    /* AUTODOC: Enable OAM processing */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamEnableSet: IOAM enable");
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamEnableSet: EOAM enable");

    /* Save Flow Id base */
    rc = prvTgfOamTableBaseFlowIdGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        &bypassOamRestore.ingBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");
    rc = prvTgfOamTableBaseFlowIdGet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        &bypassOamRestore.egrBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");

    /* AUTODOC: Set OAM flow Id base */
    rc = prvTgfOamTableBaseFlowIdSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        PRV_TGF_BASEFLOW_ID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");
    rc = prvTgfOamTableBaseFlowIdSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_BASEFLOW_ID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");

    /* Save OAM entries */
    rc = prvTgfOamEntryGet(
        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        PRV_IOAM_TABLE_ENTRY_INDEX_CNS,
        &bypassOamRestore.ingEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfOamEntryGet");

    rc = prvTgfOamEntryGet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_EOAM_TABLE_ENTRY_INDEX_CNS,
        &bypassOamRestore.egrEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfOamEntryGet");

    /* AUTODOC: Configure IOAM and EOAM tables entry */
    cpssOsMemSet(&oamEntry, 0, sizeof(oamEntry));
    oamEntry.sourceInterface.type = CPSS_INTERFACE_PORT_E;
    oamEntry.sourceInterface.devPort.hwDevNum = prvTgfDevNum;
    oamEntry.sourceInterface.devPort.portNum = /* Intentionally not source */
        prvTgfPortsArray[3];
    oamEntry.sourceInterfaceCheckEnable = GT_TRUE;
    oamEntry.sourceInterfaceCheckMode =
        PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;

    rc = prvTgfOamEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        PRV_IOAM_TABLE_ENTRY_INDEX_CNS,
        &oamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfOamEntrySet");

    rc = prvTgfOamEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_EOAM_TABLE_ENTRY_INDEX_CNS,
        &oamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfOamEntrySet");

    /* Save OAM exceptions */
    rc = prvTgfOamExceptionConfigGet(
        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
        &bypassOamRestore.ingExcept
    );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamExceptionConfigGet");

    rc = prvTgfOamExceptionConfigGet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
        &bypassOamRestore.egrExcept
    );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamExceptionConfigGet");

    /* AUTODOC: Configure OAM exception to drop a packet */
    cpssOsMemSet(&oamExceptionConfig, 0, sizeof(oamExceptionConfig));
    oamExceptionConfig.command = CPSS_PACKET_CMD_FORWARD_E;
    oamExceptionConfig.summaryBitEnable = GT_TRUE;

    rc = prvTgfOamExceptionConfigSet(
        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
        &oamExceptionConfig
    );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamExceptionConfigSet");

    rc = prvTgfOamExceptionConfigSet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
        &oamExceptionConfig
    );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamExceptionConfigSet");
}

/**
* @internal prvTgfCutThroughBypassOamTrafficGenerate function
* @endinternal
*
* @brief   Generate and check testing traffic
*/
GT_VOID prvTgfCutThroughBypassOamTrafficGenerate
(
    GT_VOID
)
{
    GT_U32                  ii;
    GT_STATUS               rc;
    GT_U32                  egrExCnt;
    GT_U32                  ingExCnt;

    /* AUTODOC: Setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4PacketInfo,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: Iterate through bypass configurations */
    for(ii = 0; ii < PRV_TGF_ELEMENTS_NUM(prvTgfExpectedOamCounters); ii++)
    {
        /* AUTODOC: Setup bypassing */
        prvTgfCutThroughBypassModeSet(prvTgfDevNum, &prvTgfOamBypassArr[ii]);

        /* AUTODOC: Transmit */
        /* Start transmitting */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfStartTransmitingEth");

        prvTgfOamExceptionCounterGet(
            PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
            PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
            &ingExCnt
        );

        prvTgfOamExceptionCounterGet(
            PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
            PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
            &egrExCnt
        );

        PRV_UTF_LOG1_MAC("Ingress exception counter: %d\n", ingExCnt);
        PRV_UTF_LOG1_MAC("Egress exception counter: %d\n", egrExCnt);
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpectedOamCounters[ii][0], ingExCnt,
            "Unexpected ingress OAM counters");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpectedOamCounters[ii][1], egrExCnt,
            "Unexpected egress OAM counters");

    }

    /* AUTODOC: Reset ethernet counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d",
        prvTgfDevNum);

    /* AUTODOC: Restore bypassing */
    prvTgfCutThroughBypassModeSet(prvTgfDevNum, &prvTgfOamBypassArr[0]);
}

/**
* @internal prvTgfCutThroughBypassOamRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassOamRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Restore bypassing after generate traffic */
    prvTgfCutThroughBypassModeSet(prvTgfDevNum, &prvTgfOamBypassArr[0]);

    /* AUTODOC: Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
        prvTgfDevNum, GT_TRUE);

    /* AUTODOC: Restore OAM exception */
    rc = prvTgfOamExceptionConfigSet(
        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
        &bypassOamRestore.ingExcept
    );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamExceptionConfigSet");

    rc = prvTgfOamExceptionConfigSet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
        &bypassOamRestore.egrExcept
    );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamExceptionConfigSet");

    /* AUTODOC: Restore OAM entries */
    rc = prvTgfOamEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        PRV_IOAM_TABLE_ENTRY_INDEX_CNS,
        &bypassOamRestore.ingEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfOamEntrySet");

    rc = prvTgfOamEntrySet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        PRV_EOAM_TABLE_ENTRY_INDEX_CNS,
        &bypassOamRestore.egrEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfOamEntrySet");

    /* AUTODOC: Restore Flow Id base */
    rc = prvTgfOamTableBaseFlowIdSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
        bypassOamRestore.ingBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");
    rc = prvTgfOamTableBaseFlowIdSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
        bypassOamRestore.egrBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");

    /* AUTODOC: Disable OAM processing */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamEnableSet: IOAM disable");
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamEnableSet: EOAM disable");

    /* AUTODOC: Invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
        prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_OAM_EPCL_RULE_IDX_CNS), GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: "
        "Rule size = [%d], rule index = [%d]",
        CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_OAM_EPCL_RULE_IDX_CNS);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
        prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0, PRV_TGF_OAM_IPCL_RULE_IDX_CNS), GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: "
        "Rule size = [%d], rule index = [%d]",
        CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_OAM_IPCL_RULE_IDX_CNS);

    /* AUTODOC: Deconfigure PCL-related port configuration */
    prvTgfPclPortsRestoreAll();

    /* Invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d",
        PRV_TGF_VLANID_CNS
    );
}

GT_VOID prvTgfCutThroughByteCountGenConfig
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 portsArr[2];

    portsArr[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    portsArr[1] = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(
        PRV_TGF_VLANID_CNS, portsArr, NULL, NULL, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    prvTgfCutThroughPortsGenConfig();

    /* Cause writting ingress packets with size different from calculated */
    /* to Packet Buffer without error marking                             */
    rc = prvTgfDxChCutThroughRxErrorConfigSet(
        GT_FALSE /*paddedPacketEofWithError*/, GT_FALSE /*truncatedPacketEofWithError*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfDxChCutThroughRxErrorConfigSet");

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxModeSet");

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

GT_VOID prvTgfCutThroughByteCountGenRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    prvTgfCutThroughPortsGenRestore();

    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 1 /*vlanId*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: Invalidate default VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

GT_VOID prvTgfSendPacketAndGetByteCount
(
    IN  GT_U32          sendPortNum,
    IN  GT_U32          recievePortNum,
    IN  GT_U32          cncBlock,
    IN  TGF_PACKET_STC *packetInfoPtr,
    IN  GT_U32          spippedPacketPartsBitmap,
    IN  GT_U32          patternPacketLen,
    IN  GT_U32          comparePacketLen,
    OUT GT_U32         *byteCountPtr
)
{
    GT_STATUS                       rc;
    PRV_TGF_CNC_COUNTER_STC         counter;
    GT_U32                          indexRangesBmp[4];
    GT_U32                          actualCapturedNumOfPackets;
    TGF_PACKET_STC                  comparePacketInfo;
    GT_U32                           nonComparedBytesArr[1];

    GT_U32                          i;
    TGF_PACKET_PART_STC             packetPartsArr[32];
    GT_U32                          maxPacketParts =
        (sizeof(packetPartsArr) / sizeof(packetPartsArr[0]));

    /* range of bytes bypassed compare from comparePacketLen to actual end of packet */
    nonComparedBytesArr[0] = ((0xFFFF << 16) | comparePacketLen);

    comparePacketInfo.numOfParts = 0;
    comparePacketInfo.partsArray = &(packetPartsArr[0]);
    for (i = 0; (i < packetInfoPtr->numOfParts); i++)
    {
        if ((1 << i) & spippedPacketPartsBitmap) continue;
        packetPartsArr[comparePacketInfo.numOfParts] = packetInfoPtr->partsArray[i];
        comparePacketInfo.numOfParts ++;
        if (comparePacketInfo.numOfParts >= maxPacketParts) break;
    }
    comparePacketInfo.totalLen = ((patternPacketLen < 64) ? 64 : patternPacketLen);

    /*AUTODOC: defaults that not restored */
    rc = prvTgfCncClientByteCountModeSet(
        PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E,
        PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncClientByteCountModeSet");

    rc = prvTgfCncCounterFormatSet(
        cncBlock, PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncCounterFormatSet");

    /* AUTODOC: enable counting for INGRESS_SRC_EPORT client for port */
    rc = prvTgfCncPortClientEnableSetByDevPort(
        prvTgfDevNum, sendPortNum,
        PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E,
        GT_TRUE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncPortClientEnableSet");

    /* AUTODOC: configuration that will be restored */

    indexRangesBmp[0] = 1;
    indexRangesBmp[1] = 0;
    indexRangesBmp[2] = 0;
    indexRangesBmp[3] = 0;
    rc = prvTgfCncBlockClientRangesSet(
        cncBlock, PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncBlockClientRangesSet");

    rc = prvTgfCncBlockClientEnableSet(
        cncBlock, PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E, GT_TRUE/*enable*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncBlockClientEnableSet");

    /* AUTODOC: clear counter */
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;
    counter.byteCount.l[0]   = 0;
    counter.byteCount.l[1]   = 0;
    rc = prvTgfCncCounterSet(
        cncBlock, sendPortNum/*counterIdx*/,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncCounterGet");

    tgfTrafficTracePacketByteSet(GT_TRUE);

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum, sendPortNum,
        packetInfoPtr,
        1      /*burstCount*/,
        0 /*vfdArraySize*/, NULL /*vfdArray*/,
        prvTgfDevNum, recievePortNum,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200 /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
        NULL /*portInterfacePtr*/,
        &comparePacketInfo /*packetInfoPtr*/,
        1 /*numOfPackets*/,
        0 /*numVfd*/,
        NULL /*vfdArray*/,
        nonComparedBytesArr /*byteNumMaskList*/, 1 /*byteNumMaskListLen*/,
        &actualCapturedNumOfPackets,
        NULL /*onFirstPacketNumTriggersBmpPtr*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureCompare");

    /* AUTODOC: get result */
    rc = prvTgfCncCounterGet(
        cncBlock, sendPortNum/*counterIdx*/,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncCounterGet");
    *byteCountPtr = counter.byteCount.l[0];

    /* AUTODOC: restore */

    /* AUTODOC: enable counting for INGRESS_SRC_EPORT client for port */
    rc = prvTgfCncPortClientEnableSetByDevPort(
        prvTgfDevNum, sendPortNum,
        PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E,
        GT_FALSE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncPortClientEnableSet");

    indexRangesBmp[0] = 0;
    indexRangesBmp[1] = 0;
    indexRangesBmp[2] = 0;
    indexRangesBmp[3] = 0;
    rc = prvTgfCncBlockClientRangesSet(
        cncBlock, PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncBlockClientRangesSet");

    rc = prvTgfCncBlockClientEnableSet(
        cncBlock, PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E, GT_FALSE/*enable*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCncBlockClientEnableSet");
}

static GT_U32 prvTgfCutThroughByteCountTestSkipPacketsBmp = 0;

void prvTgfCutThroughByteCountTestSkipPacketsBmpSet(GT_U32 bmp)
{
    prvTgfCutThroughByteCountTestSkipPacketsBmp = bmp;
}

/**
* @internal prvTgfCutThroughByteCountTest function
* @endinternal
*
* @brief   Test calculated byte count
*/
GT_VOID prvTgfCutThroughByteCountTest
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    byteCount;
    GT_U32    expectedByteCount;
    GT_U32    egressByteCount;
    GT_U32    udeConfiguredByteCount;
    GT_U32    packetIndex;

    udeConfiguredByteCount = 0x80;
    packetIndex = 0;

    prvTgfCutThroughByteCountGenConfig();

    /* all packets with wrong payload length in header */

    if (((1 << packetIndex) & prvTgfCutThroughByteCountTestSkipPacketsBmp) == 0)
    {
        /* IPV4 tagged */
        expectedByteCount =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS + prvTgfPacketIpv4Part.totalLen + 4 /*checksum*/;
        egressByteCount =
            TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
            + prvTgfPacketIpv4Part.totalLen + 4 /*checksum*/;
        prvTgfSendPacketAndGetByteCount(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            0 /*cncBlock*/,
            &prvTgfIpv4PacketInfo,
            2 /*spippedPacketPartsBitmap*/,
            egressByteCount /*patternPacketLen*/,
            (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
            &byteCount);
        PRV_UTF_LOG1_MAC("IPV4 byteCount: 0x%08X\n", byteCount);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");
    }

    packetIndex ++;
    if (((1 << packetIndex) & prvTgfCutThroughByteCountTestSkipPacketsBmp) == 0)
    {
        /* IPV6 untagged */
        expectedByteCount =
            TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
            + TGF_IPV6_HEADER_SIZE_CNS + prvTgfPacketIpv6Part.payloadLen + 4 /*checksum*/;
        egressByteCount = expectedByteCount;
        prvTgfSendPacketAndGetByteCount(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            0 /*cncBlock*/,
            &prvTgfIpv6PacketInfo,
            0 /*spippedPacketPartsBitmap*/,
            egressByteCount /*patternPacketLen*/,
            (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
            &byteCount);
        PRV_UTF_LOG1_MAC("IPV6 byteCount: 0x%08X\n", byteCount);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");
    }

    packetIndex ++;
    if (((1 << packetIndex) & prvTgfCutThroughByteCountTestSkipPacketsBmp) == 0)
    {
        /* LLC untagged */
        expectedByteCount =
            TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
            + prvTgfLlcPacketEtherTypePart.etherType + 4 /*checksum*/;
        egressByteCount = expectedByteCount;
        prvTgfSendPacketAndGetByteCount(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            0 /*cncBlock*/,
            &prvTgfLlcPacketInfo,
            0 /*spippedPacketPartsBitmap*/,
            egressByteCount /*patternPacketLen*/,
            (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
            &byteCount);
        PRV_UTF_LOG1_MAC("LLC byteCount: 0x%08X\n", byteCount);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");
    }

    packetIndex ++;
    if (((1 << packetIndex) & prvTgfCutThroughByteCountTestSkipPacketsBmp) == 0)
    {
        /* configure UDE cut-through */
        rc = prvTgfDxChCutThroughUdeCfgSet(
            0 /*udeIndex*/,
            prvTgfUdePacketEtherTypePart.etherType /*udeEthertype*/,
            GT_TRUE /*udeCutThroughEnable*/,
            udeConfiguredByteCount /*udeByteCount*/);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfDxChCutThroughUdeCfgSet");

        /* UDE untagged */
        expectedByteCount = udeConfiguredByteCount;
        egressByteCount   = expectedByteCount;
        prvTgfSendPacketAndGetByteCount(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            0 /*cncBlock*/,
            &prvTgfUdePacketInfo,
            0 /*spippedPacketPartsBitmap*/,
            egressByteCount /*patternPacketLen*/,
            (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
            &byteCount);
        PRV_UTF_LOG1_MAC("UDE byteCount: 0x%08X\n", byteCount);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");

        /* restore UDE cut-through */
        rc = prvTgfDxChCutThroughUdeCfgSet(
            0 /*udeIndex*/,
            0 /*udeEthertype*/,
            GT_FALSE /*udeCutThroughEnable*/,
            0 /*udeByteCount*/);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfDxChCutThroughUdeCfgSet");
    }

    packetIndex ++;
    if (((1 << packetIndex) & prvTgfCutThroughByteCountTestSkipPacketsBmp) == 0)
    {
        /* increase bytecount by 5 */
        rc = prvTgfCutThroughPortByteCountUpdateSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_FALSE /*toSubtractOrToAdd*/,
            5 /*subtractedOrAddedValue*/);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortByteCountUpdateSet");

        /* IPV4 tagged with per-port adjust increasing  */
        expectedByteCount =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS + prvTgfPacketIpv4Part.totalLen + 5 + 4 /*checksum*/;
        egressByteCount   =
            TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
            + prvTgfPacketIpv4Part.totalLen + 5 /*adjust*/ + 4 /*checksum*/;
        prvTgfSendPacketAndGetByteCount(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            0 /*cncBlock*/,
            &prvTgfIpv4PacketInfo,
            2 /*spippedPacketPartsBitmap*/,
            egressByteCount /*patternPacketLen*/,
            (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
            &byteCount);
        PRV_UTF_LOG1_MAC("IPV4 byteCount increased by 5: 0x%08X\n", byteCount);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");
    }

    packetIndex ++;
    if (((1 << packetIndex) & prvTgfCutThroughByteCountTestSkipPacketsBmp) == 0)
    {
        /* decrease bytecount by 3 */
        rc = prvTgfCutThroughPortByteCountUpdateSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_TRUE /*toSubtractOrToAdd*/,
            3 /*subtractedOrAddedValue*/);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortByteCountUpdateSet");

        /* IPV4 tagged with per-port adjust decreasing */
        expectedByteCount =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS + prvTgfPacketIpv4Part.totalLen - 3 /*adjust*/ + 4 /*checksum*/;
        egressByteCount   =
            TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
            + prvTgfPacketIpv4Part.totalLen - 3 /*adjust*/ + 4 /*checksum*/;
        prvTgfSendPacketAndGetByteCount(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            0 /*cncBlock*/,
            &prvTgfIpv4PacketInfo,
            2 /*spippedPacketPartsBitmap*/,
            egressByteCount /*patternPacketLen*/,
            (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
            &byteCount);
        PRV_UTF_LOG1_MAC("IPV4 byteCount decreased by 3: 0x%08X\n", byteCount);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");
    }

    /* restore adjusting bytecount */
    rc = prvTgfCutThroughPortByteCountUpdateSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        GT_FALSE /*toSubtractOrToAdd*/,
        0 /*subtractedOrAddedValue*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortByteCountUpdateSet");

    prvTgfCutThroughByteCountGenRestore();
}

static GT_STATUS prvTgfCutThroughTtiTunnelTermConfigSet
(
    IN GT_U32  port,
    IN GT_U32  ruleIndex,
    IN GT_U32  ttiMplsPclId,
    IN GT_BOOL transitOrTunnelTerm,
    IN GT_BOOL passengerWithL2
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
    if (transitOrTunnelTerm == GT_FALSE)
    {
        ttiAction.tunnelTerminate                   = GT_TRUE;
    }
    else
    {
        if (passengerWithL2 == GT_FALSE)
        {
            ttiAction.passengerParsingOfTransitMplsTunnelMode =
                PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E;
        }
        else
        {
            ttiAction.passengerParsingOfTransitMplsTunnelMode =
                PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E;
        }
    }
    if (passengerWithL2 == GT_FALSE)
    {
        ttiAction.ttPassengerPacketType             = PRV_TGF_TTI_PASSENGER_IPV4_E;
    }
    else
    {
        ttiAction.ttPassengerPacketType             = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    }
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    ttiMask.mpls.common.pclId    = 0x3FF;
    ttiPattern.mpls.common.pclId = ttiMplsPclId;

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        port, PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, ttiMplsPclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        ruleIndex, PRV_TGF_TTI_KEY_MPLS_E,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    prvTgfPclTunnelTermForceVlanModeEnableSet(GT_FALSE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclTunnelTermForceVlanModeEnableSet");

    return GT_OK;
}

static GT_STATUS prvTgfCutThroughTtiTunnelTermConfigRestore
(
    IN GT_U32 port,
    IN GT_U32 ruleIndex
)
{
    GT_STATUS                rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting TTI Restore =======\n");
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        port, PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, 0);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(ruleIndex, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRuleValidStatusSet");

    return GT_OK;
}

/**
* @internal prvTgfCutThroughByteCountIpv4OverMplsTest function
* @endinternal
*
* @brief   Test calculated byte count for IPV4 over MPLS packet
*/
GT_VOID prvTgfCutThroughByteCountIpv4OverMplsTest
(
    IN GT_BOOL transitOrTunnelTerm,
    IN GT_BOOL passengerWithL2
)
{
    GT_STATUS rc;
    GT_U32    byteCount;
    GT_U32    expectedByteCount;
    GT_U32    egressByteCount;
    GT_U32    ttiRuleIndex = 6;
    GT_U32    ttiMplsPclId = 1;
    GT_U32    spippedPacketPartsBitmap;

    PRV_UTF_LOG2_MAC(
        "IPV4 over MPLS transitOrTunnelTerm %d passengerWithL2 %d \n",
        transitOrTunnelTerm, passengerWithL2);

    prvTgfCutThroughByteCountGenConfig();

    rc = prvTgfCutThroughTtiTunnelTermConfigSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], ttiRuleIndex, ttiMplsPclId,
        transitOrTunnelTerm, passengerWithL2);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
        GT_OK, rc, "prvTgfCutThroughTtiTunnelTermConfigSet");

    /* configure MPLS as UDE cut-through */
    /* default udeByteCount to be obtained by TTI */
    rc = prvTgfDxChCutThroughUdeCfgSet(
        0 /*udeIndex*/,
        prvTgfMplsPacketEtherTypePart.etherType /*udeEthertype == MPLS*/,
        GT_TRUE /*udeCutThroughEnable*/,
        0x3FFF /*udeByteCount*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfDxChCutThroughUdeCfgSet");

    if (passengerWithL2 == GT_FALSE)
    {
        /* IPV4 over MPLS untagged L3 only passenger */
        if (transitOrTunnelTerm == GT_FALSE)
        {
            PRV_UTF_LOG0_MAC("IPV4 over MPLS Test for Tunnel termination of L3 only passenger\n");
            PRV_UTF_LOG0_MAC("Not supported\n");
            return;
        }
        else
        {
            expectedByteCount =
                TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS
                + prvTgfPacketIpv4Part.totalLen + 4 /*checksum*/;
            egressByteCount   = expectedByteCount;
            spippedPacketPartsBitmap = 0;
        }
        prvTgfSendPacketAndGetByteCount(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            0 /*cncBlock*/,
            &prvTgfIpv4OverMplsPacketInfo,
            spippedPacketPartsBitmap,
            egressByteCount /*patternPacketLen*/,
            (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
            &byteCount);
        PRV_UTF_LOG1_MAC("IPV4 over MPLS byteCount: 0x%08X\n", byteCount);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");
    }
    else
    {
        /* IPV4 over MPLS untagged L2 in passenger */
        if (transitOrTunnelTerm == GT_FALSE)
        {
            expectedByteCount =
                TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
                + prvTgfPacketIpv4Part.totalLen + 4 /*checksum*/;
            egressByteCount   = expectedByteCount;
            spippedPacketPartsBitmap = 7;
        }
        else
        {
            expectedByteCount =
                TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS
                + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
                + prvTgfPacketIpv4Part.totalLen + 4 /*checksum*/;
            egressByteCount   = expectedByteCount;
            spippedPacketPartsBitmap = 0;
        }
        prvTgfSendPacketAndGetByteCount(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            0 /*cncBlock*/,
            &prvTgfIpv4WithL2OverMplsPacketInfo,
            spippedPacketPartsBitmap,
            egressByteCount /*patternPacketLen*/,
            (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
            &byteCount);
        PRV_UTF_LOG1_MAC("IPV4 over MPLS byteCount: 0x%08X\n", byteCount);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");
    }

    /* restore UDE cut-through */
    rc = prvTgfDxChCutThroughUdeCfgSet(
        0 /*udeIndex*/,
        0 /*udeEthertype*/,
        GT_FALSE /*udeCutThroughEnable*/,
        0 /*udeByteCount*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfDxChCutThroughUdeCfgSet");

    rc = prvTgfCutThroughTtiTunnelTermConfigRestore(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], ttiRuleIndex);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
        GT_OK, rc, "prvTgfCutThroughTtiTunnelTermConfigRestore");

    prvTgfCutThroughByteCountGenRestore();
}

/**
* @internal prvTgfCutThroughByteCountUdeCtTerminationTest_defByteCount function
* @endinternal
*
* @brief   Test calculated byte count UDE packet with assigned byte count 0x3FFF
*/
GT_VOID prvTgfCutThroughByteCountUdeCtTerminationTest_defByteCount
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    byteCount;
    GT_U32    expectedByteCount;
    GT_U32    egressByteCount;
    GT_U32    extractBcFailsCount;
    GT_U32    profileIter;

    /* AUTODOC: enable Cut Through forwarding on port 0 */
    prvTgfCutThroughByteCountGenConfig();

    /* configure UDE cut-through default udeByteCount */
    /* to be obtained by EFT_QAG and terminated       */
    rc = prvTgfDxChCutThroughUdeCfgSet(
        0 /*udeIndex*/,
        prvTgfUdePacketEtherTypePart.etherType /*udeEthertype*/,
        GT_TRUE /*udeCutThroughEnable*/,
        0x3FFF /*udeByteCount*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfDxChCutThroughUdeCfgSet");

    /* AUTODOC: set MRU for each profiles to 0x3FFF) */
    for (profileIter = 0 ; profileIter < 8 ; profileIter++ )
    {
        rc = prvTgfBrgVlanMruProfileValueSet(profileIter, 0x3FFF);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");
    }

    /* UDE packet */
    expectedByteCount = 0x3FFF;
    egressByteCount = prvTgfUdePacketInfo.totalLen + 4 /*checksum*/;
    prvTgfSendPacketAndGetByteCount(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        0 /*cncBlock*/,
        &prvTgfUdePacketInfo,
        0 /*spippedPacketPartsBitmap*/,
        egressByteCount /*patternPacketLen*/,
        (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
        &byteCount);
    PRV_UTF_LOG1_MAC("UDE with default assigned bytecount: byteCount: 0x%08X\n", byteCount);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");

    rc = prvTgfDxChCutThroughByteCountExtractFailsCounterGet(&extractBcFailsCount);
    PRV_UTF_LOG1_MAC("extractBcFailsCount: %d\n", extractBcFailsCount);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
        GT_OK, rc, "prvTgfDxChCutThroughByteCountExtractFailsCounterGet");

    /* AUTODOC: restore MRU for each profiles to 0x5F2) */
    for (profileIter = 0 ; profileIter < 8 ; profileIter++ )
    {
        rc = prvTgfBrgVlanMruProfileValueSet(profileIter, PRV_TGF_DEF_MRU_VALUE_CNS);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");
    }

    /* restore UDE cut-through */
    rc = prvTgfDxChCutThroughUdeCfgSet(
        0 /*udeIndex*/,
        0 /*udeEthertype*/,
        GT_FALSE /*udeCutThroughEnable*/,
        0 /*udeByteCount*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfDxChCutThroughUdeCfgSet");

    prvTgfCutThroughByteCountGenRestore();
}

/**
* @internal prvTgfCutThroughTerminateEpclConfigSet function
* @endinternal
*
* @brief   Set test EPCL configuration
*/
static GT_VOID prvTgfCutThroughTerminateEpclConfigSet
(
    GT_U32 portNum,
    GT_U32 ruleIndex,
    GT_U16 pclId
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;

    ruleIndex                   = 0;
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: init PCL Engine for send port 2: */
    /* AUTODOC:   egress direction, lookup_0 */
    /* AUTODOC:   nonIpKey EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key EGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key EGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInitExt1(
        portNum,
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        pclId,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d, %d",
        prvTgfDevNum, portNum);

    /* set PCL rule 0 - CMD_DROP_HARD packet 1 - any Not Ipv4 with MAC DA = ... 34 02 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    mask.ruleEgrExtIpv6L2.common.pclId    = 0x3FF;
    pattern.ruleEgrExtIpv6L2.common.pclId = pclId;

    action.egressPolicy            = GT_TRUE;
    action.pktCmd                  = CPSS_PACKET_CMD_FORWARD_E;
    action.terminateCutThroughMode = GT_TRUE;

    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfCutThroughTerminateEpclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfCutThroughTerminateEpclRestore
(
    GT_U32 portNum,
    GT_U32 ruleIndex
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;

    /* disable egress policy per port */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum, portNum,
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum   = prvTgfDevNum;
    interfaceInfo.devPort.portNum    = portNum;

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.lookupType             = PRV_TGF_PCL_LOOKUP_TYPE_SINGLE_E;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.externalLookup         = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfPclRuleValidStatusSet");
}

/**
* @internal prvTgfCutThroughIpv6CtTerminationTest_EPCL function
* @endinternal
*
* @brief   Test EPCL caused Cut Through termination
*/
GT_VOID prvTgfCutThroughIpv6CtTerminationTest_EPCL
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    byteCount;
    GT_U32    expectedByteCount;
    GT_U32    egressByteCount;
    GT_U32    extractBcFailsCount;
    GT_U32    ruleIndex = 12;
    GT_U16    pclId = 0x123;

    /* AUTODOC: enable Cut Through forwarding on port 0 */
    prvTgfCutThroughByteCountGenConfig();

    prvTgfCutThroughTerminateEpclConfigSet(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        ruleIndex, pclId);

    /* IPV6 untagged */
    expectedByteCount =
        TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
        + TGF_IPV6_HEADER_SIZE_CNS + prvTgfPacketIpv6Part.payloadLen + 4 /*checksum*/;
    egressByteCount = prvTgfIpv6PacketInfo.totalLen;
    prvTgfSendPacketAndGetByteCount(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        0 /*cncBlock*/,
        &prvTgfIpv6PacketInfo,
        0 /*spippedPacketPartsBitmap*/,
        egressByteCount /*patternPacketLen*/,
        (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
        &byteCount);
    PRV_UTF_LOG1_MAC("IPV6 byteCount: 0x%08X\n", byteCount);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");

    rc = prvTgfDxChCutThroughByteCountExtractFailsCounterGet(&extractBcFailsCount);
    PRV_UTF_LOG1_MAC("extractBcFailsCount: %d\n", extractBcFailsCount);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
        GT_OK, rc, "prvTgfDxChCutThroughByteCountExtractFailsCounterGet");

    prvTgfCutThroughTerminateEpclRestore(
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        ruleIndex);

    prvTgfCutThroughByteCountGenRestore();
}

/**
* @internal prvTgfCutThroughIpv4CtTerminationTest_SlowToFast function
* @endinternal
*
* @brief   Test Slow to Fast port caused Cut Through termination
*/
GT_VOID prvTgfCutThroughIpv4CtTerminationTest_SlowToFast
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    GT_U32                          byteCount;
    GT_U32                          expectedByteCount;
    GT_U32                          egressByteCount;
    GT_U32                          extractBcFailsCount;
    CPSS_PORT_INTERFACE_MODE_ENT    sendPortIfMode;
    CPSS_PORT_SPEED_ENT             sendPortspeed;
    CPSS_PORT_INTERFACE_MODE_ENT    receivePortIfMode;
    CPSS_PORT_SPEED_ENT             receivePortspeed;
    GT_U32                          i;
    GT_U32                          speedChanged;
    static struct
    {
        CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
        CPSS_PORT_SPEED_ENT             speed;
    } setSpeedArr[] =
    {
        {CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_25000_E},
        {CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_50000_E},
        {CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E},
        {CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_100G_E}
    };
    static GT_U32 setSpeedArrSize = (sizeof(setSpeedArr) / sizeof(setSpeedArr[0]));

    rc = prvTgfPortModeSpeedGet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &sendPortIfMode, &sendPortspeed);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortModeSpeedGet");
    rc = prvTgfPortModeSpeedGet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        &receivePortIfMode, &receivePortspeed);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortModeSpeedGet");

    speedChanged = 0;
    /* attempt to slow the send port */
    for (i = 0; (i < setSpeedArrSize); i++)
    {
        if (prvCpssCommonPortSpeedEnumToMbPerSecConvert(setSpeedArr[i].speed)
            < prvCpssCommonPortSpeedEnumToMbPerSecConvert(sendPortspeed))
        {
            rc = prvTgfPortModeSpeedSet(
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                setSpeedArr[i].ifMode, setSpeedArr[i].speed);
            UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
                GT_OK, rc, "prvTgfPortModeSpeedSet");
            speedChanged = 1;
            break;
        }
    }
    if (speedChanged == 0)
    {
        /* attempt to fast the receive port */
        for (i = 0; (i < setSpeedArrSize); i++)
        {
            if (prvCpssCommonPortSpeedEnumToMbPerSecConvert(setSpeedArr[i].speed)
                > prvCpssCommonPortSpeedEnumToMbPerSecConvert(receivePortspeed))
            {
                rc = prvTgfPortModeSpeedSet(
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                    setSpeedArr[i].ifMode, setSpeedArr[i].speed);
                UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
                    GT_OK, rc, "prvTgfPortModeSpeedSet");
                speedChanged = 1;
                break;
            }
        }
    }

    /* AUTODOC: enable Cut Through forwarding on port 0 */
    prvTgfCutThroughByteCountGenConfig();

    /* IPV4 tagged */
    expectedByteCount =
        TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS + prvTgfPacketIpv4Part.totalLen + 4 /*checksum*/;
    egressByteCount = prvTgfIpv4PacketInfo.totalLen;
    prvTgfSendPacketAndGetByteCount(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        0 /*cncBlock*/,
        &prvTgfIpv4PacketInfo,
        2 /*spippedPacketPartsBitmap*/,
        egressByteCount /*patternPacketLen*/,
        (egressByteCount - 4 /*checksum*/) /*comparePacketLen*/,
        &byteCount);
    PRV_UTF_LOG1_MAC("IPV4 byteCount: 0x%08X\n", byteCount);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(expectedByteCount, byteCount, "");

    rc = prvTgfDxChCutThroughByteCountExtractFailsCounterGet(&extractBcFailsCount);
    PRV_UTF_LOG1_MAC("extractBcFailsCount: %d\n", extractBcFailsCount);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
        GT_OK, rc, "prvTgfDxChCutThroughByteCountExtractFailsCounterGet");

    prvTgfCutThroughByteCountGenRestore();

    rc = prvTgfPortModeSpeedSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        sendPortIfMode, sendPortspeed);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
        GT_OK, rc, "prvTgfPortModeSpeedSet");
    rc = prvTgfPortModeSpeedSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        receivePortIfMode, receivePortspeed);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(
        GT_OK, rc, "prvTgfPortModeSpeedSet");

}


