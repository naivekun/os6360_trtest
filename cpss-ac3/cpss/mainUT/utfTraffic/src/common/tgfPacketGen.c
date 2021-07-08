/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORKFtunnel RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file tgfPacketGen.c
*
* @brief Generic API implementation for Packets
*
* @version   15
********************************************************************************
*/

#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    #define CHX_FAMILY

#endif /*COW_INTERNAL_COMPILE*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <../../cpssEnabler/mainCmd/h/cmdShell/common/cmdExtServices.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfTunnelGen.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#endif /*CHX_FAMILY*/
/******************************************************************************\
 *                             Define section                                 *
\******************************************************************************/

#define MAX_NUM_OF_VLAN_TAGS_CNS                             4
#define MAX_NUM_OF_MPLS_LABELS_CNS                           6 /* 4 labels + pw label + flow label */

#define DEFAULT_NUM_OF_VLAN_TAGS_CNS                         1
#define DEFAULT_NUM_OF_VLAN_TAGS_PASSENGER_CNS               1
#define DEFAULT_NUM_OF_MPLS_LABELS_CNS                       1
#define DEFAULT_IS_GRE_CNS                                   1
#define DEFAULT_TUNNEL_HEADER_FORCE_BYTES_TILL_PASSENGER_CNS 0
#define DEFAULT_IS_CRC_CNS                                   0
#define DEFAULT_IS_WILDCARD_CNS                              0


/******************************************************************************\
 *                            Global parameters section                        *
\******************************************************************************/

/* Allow pointers of const type to be place where const not defined */
#define REMOVE_CONST_PTR_CNS (GT_VOID*)

/* default values */
static GT_U32 prvTgfPacketNumberOfVlanTags                    = DEFAULT_NUM_OF_VLAN_TAGS_CNS;
static GT_U32 prvTgfPacketNumberOfVlanTagsPassenger           = DEFAULT_NUM_OF_VLAN_TAGS_PASSENGER_CNS;
static GT_U32 prvTgfPacketNumberOfMplsLabels                  = DEFAULT_NUM_OF_MPLS_LABELS_CNS;
static GT_BOOL prvTgfPacketIsGre                              = DEFAULT_IS_GRE_CNS;
static GT_U32 prvTgfPacketTunnelHeaderForceBytesTillPassenger = DEFAULT_TUNNEL_HEADER_FORCE_BYTES_TILL_PASSENGER_CNS;
static GT_BOOL prvTgfPacketIsCrc                              = DEFAULT_IS_CRC_CNS;
static GT_BOOL prvTgfPacketIsWildcard                         = DEFAULT_IS_WILDCARD_CNS;


/**************************** Generic Parts ***********************************/
/* L2 part of packet */
static const TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x01, 0x02, 0x03, 0x34, 0x02},               /* daMac */
    {0x00, 0x04, 0x05, 0x06, 0x07, 0x11}                /* saMac */
};
static TGF_PACKET_L2_STC use_prvTgfPacketL2Part;

/* L2 part of passenger packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_Passenger =
{
    {0x00, 0x09, 0x0A, 0x0B, 0x0C, 0x22},    /* daMac */
    {0x00, 0x0D, 0x0E, 0x0F, 0x00, 0x25}     /* saMac */
};
static TGF_PACKET_L2_STC use_prvTgfPacketL2Part_Passenger;

/* L2 part of passenger packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_Tunnel =
{
    {0x88, 0x77, 0x11, 0x11, 0x55, 0x66 },    /* daMac */
    {0x98, 0x32, 0x21, 0x19, 0x18, 0x06}     /* saMac */
};
static TGF_PACKET_L2_STC use_prvTgfPacketL2Part_Tunnel;

/* packet's ethertype */
static const TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
static TGF_PACKET_ETHERTYPE_STC use_prvTgfPacketIpv4EtherTypePart;

static const TGF_PACKET_ETHERTYPE_STC prvTgfPacketCapwapEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
static TGF_PACKET_ETHERTYPE_STC use_prvTgfPacketIpv4EtherTypePart;


/* packet's IPv4 */
static const TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

static TGF_PACKET_IPV4_STC use_prvTgfPacketIpv4Part;

/* packet's IPv4 over IPv4 */
static const TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part_Passenger = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's IPv4 tunnel header */
static const TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part_Tunnel = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x80,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x21,               /* timeToLive */
    0x11,               /* protocol UDP*/
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS ,             /* csum 0x9366*/
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
static TGF_PACKET_IPV4_STC use_prvTgfPacketIpv4Part_Tunnel;
/* packet's IPv4 */
static const TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part_Tunnel_Passenger = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x54,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};
static TGF_PACKET_IPV4_STC use_prvTgfPacketIpv4Part_Tunnel_Passenger;

/* packet's ethertype */
static const TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC use_prvTgfPacketIpv6EtherTypePart;

/* packet's IPv6 */
static const TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part_Tunnel =
{
    6,                                       /* version */
    0,                                       /* trafficClass */
    0,                                       /* flowLabel */
    0x6c,                                    /* payloadLen */
    0x11,                                    /* nextHeader */
    0x21,                                    /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};
static TGF_PACKET_IPV6_STC use_prvTgfPacketIpv6Part_Tunnel;

/* VLAN_TAG part of packet */
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {0x8100, 0, 0, 5};
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part_Tunnel = {0x8100, 0, 0, 6};
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag2Part = {0x5678, 0, 0, 6};
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag3Part = {0xACBD, 0, 0, 0x111};
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag4Part = {0xE1E1, 0, 0, 0x222};
static TGF_PACKET_VLAN_TAG_STC use_prvTgfPacketVlanTag1Part,use_prvTgfPacketVlanTag1Part_Tunnel,use_prvTgfPacketVlanTag2Part,use_prvTgfPacketVlanTag3Part,use_prvTgfPacketVlanTag4Part;

/* VLAN_TAG part of packet passenger */
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part_Passenger = {0x8100, 0, 0, 5};
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag2Part_Passenger = {0x1234, 0, 0, 6};
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag3Part_Passenger = {0x2222, 0, 0, 7};
static const TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag4Part_Passenger = {0x3333, 0, 0, 8};
static TGF_PACKET_VLAN_TAG_STC use_prvTgfPacketVlanTag1Part_Passenger,use_prvTgfPacketVlanTag2Part_Passenger,use_prvTgfPacketVlanTag3Part_Passenger,use_prvTgfPacketVlanTag4Part_Passenger;

/* GRE part */
static const TGF_PACKET_GRE_STC prvTgfPacketGrePart = {
    0,                  /* checkSumPresent */
    0,                  /* reserved */
    0,                  /* version */
    0x6558              /* protocol */
};
static TGF_PACKET_GRE_STC use_prvTgfPacketGrePart;

/* UDP part */
static const TGF_PACKET_UDP_STC prvTgfPacketUdpPart = {
    64212,                                                  /* udp src port */
    14519,                                                  /* udp dst port */
    108,                                                    /* udp length */
    0                       /* udp check sum */
};
static TGF_PACKET_UDP_STC use_prvTgfPacketUdpPart;


/* tunnel start generic template 8 byte */
static GT_U8 prvTgfTemplate8DataArr[] =
{
    0x00, 0x10, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* tunnel start generic template 16 byte */
static GT_U8 prvTgfTemplate16DataArr[] =
{
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
};
static const TGF_PACKET_TEMPLATE16_STC prvTgfPacketTemplatePart =
{
    sizeof(prvTgfTemplate16DataArr),                       /* dataLength */
    prvTgfTemplate16DataArr                                /* dataPtr */
};

static const TGF_PACKET_TEMPLATE8_STC prvTgfPacketTemplate8Part =
{
    sizeof(prvTgfTemplate8DataArr),                       /* dataLength */
    prvTgfTemplate8DataArr                                /* dataPtr */
};


static TGF_PACKET_TEMPLATE16_STC use_prvTgfPacketTemplatePart;
static TGF_PACKET_TEMPLATE8_STC use_prvTgfPacketTemplate8Part;

/* payload of the passenger  */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* payload part of the passenger */
static const TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};
static TGF_PACKET_PAYLOAD_STC use_prvTgfPacketPayloadPart;

/* dummy CRC of the passenger */
static GT_U8 prvTgfPacketCrcData_Passenger[] =
{0x43, 0x96 , 0xf2 , 0xac};

/* struct for any kind of wildcard/'Unknown format' */
static const TGF_PACKET_CRC_STC  prvTgfPacketCrcPart_Passenger =
{
    sizeof(prvTgfPacketCrcData_Passenger),                       /* dataLength */
    prvTgfPacketCrcData_Passenger                                /* dataPtr */
};
static TGF_PACKET_CRC_STC  use_prvTgfPacketCrcPart_Passenger;

/* dummy wildcard */
static const GT_U8 prvTgfPacketWildcardDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* struct for any kind of wildcard/'Unknown format' */
static TGF_PACKET_WILDCARD_STC  prvTgfPacketWildcardPart =
{
    sizeof(prvTgfPacketWildcardDataArr),              /* dataLength */
    REMOVE_CONST_PTR_CNS prvTgfPacketWildcardDataArr  /* dataPtr */
};
static TGF_PACKET_WILDCARD_STC  use_prvTgfPacketWildcardPart;

/* packet's MPLS ethertype */
static const TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart = {TGF_ETHERTYPE_8847_MPLS_TAG_CNS};
static TGF_PACKET_ETHERTYPE_STC use_prvTgfPacketMplsEtherTypePart;

/* packet's MPLS */
static const TGF_PACKET_MPLS_STC prvTgfPacketMplsLabel1Part =
{
    0xAB,                  /* label */
    0,                  /* experimental use */
    0x01,                  /* stack --> YES end of stack (last bit = 1) */
    0x15                /* timeToLive */
};
static TGF_PACKET_MPLS_STC use_prvTgfPacketMplsLabel1Part;

/* packet's MPLS */
static const TGF_PACKET_MPLS_STC prvTgfPacketMplsLabel2Part =
{
    2,                  /* label */
    0,                  /* experimental use */
    0,                  /* stack --> NOT end of stack (last bit = 0) */
    0x16                /* timeToLive */
};
static TGF_PACKET_MPLS_STC use_prvTgfPacketMplsLabel2Part;

/* packet's MPLS */
static const TGF_PACKET_MPLS_STC prvTgfPacketMplsLabel3Part =
{
    3,                  /* label */
    0,                  /* experimental use */
    1,                  /* stack --> end of stack (last bit) */
    0x17                /* timeToLive */
};
static TGF_PACKET_MPLS_STC use_prvTgfPacketMplsLabel3Part;

/* packet's MPLS */
static const TGF_PACKET_MPLS_STC prvTgfPacketMplsLabel4Part =
{
    4,                  /* label */
    0,                  /* experimental use */
    1,                  /* stack --> end of stack (last bit) */
    0x18                /* timeToLive */
};
static TGF_PACKET_MPLS_STC use_prvTgfPacketMplsLabel4Part;

/* packet's PW Label */
static const TGF_PACKET_MPLS_STC prvTgfPacketPwLabelPart =
{
    0x70,               /* label */
    6,                  /* experimental use */
    0,                  /* stack --> end of stack (last bit) */
    0x33                /* timeToLive */
};
static TGF_PACKET_MPLS_STC use_prvTgfPacketPwLabelPart;

/* packet's FLOW Label */
static const TGF_PACKET_MPLS_STC prvTgfPacketFlowLabelPart =
{
    0,                  /* label */
    0,                  /* experimental use */
    0,                  /* stack --> end of stack (last bit) */
    0x44                /* timeToLive */
};
static TGF_PACKET_MPLS_STC use_prvTgfPacketFlowLabelPart;

/* packet's CW Label */
static GT_U8 prvTgfPacketCwLabel[] =
{0x0, 0x11 , 0x22 , 0x33};

static const TGF_PACKET_WILDCARD_STC  prvTgfPacketCwLabelPart =
{
    sizeof(prvTgfPacketCwLabel),                       /* dataLength */
    prvTgfPacketCwLabel                                /* dataPtr */
};
static TGF_PACKET_WILDCARD_STC  use_prvTgfPacketCwLabelPart;

static const TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart_Passenger = {0x6666};
static TGF_PACKET_ETHERTYPE_STC use_prvTgfPacketEtherTypePart_Passenger;

static const TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {0x0600};
static TGF_PACKET_ETHERTYPE_STC use_prvTgfPacketEtherTypePart;

/**************************** Ethernet ****************************************/
/* PARTS of packet */
static const TGF_PACKET_PART_STC prvTgfPacketEthPartArray[] = {
    {TGF_PACKET_PART_L2_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E, REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E, REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E, REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E, REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E,REMOVE_CONST_PTR_CNS &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC save_use_prvTgfPacketEthPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &use_prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &use_prvTgfPacketPayloadPart}
};


#define NUM_PARTS_ETH_CNS (sizeof(prvTgfPacketEthPartArray) / sizeof(prvTgfPacketEthPartArray[0]))
#define USE_NUM_PARTS_ETH_CNS (sizeof(save_use_prvTgfPacketEthPartArray) / sizeof(save_use_prvTgfPacketEthPartArray[0]))

static TGF_PACKET_PART_STC use_prvTgfPacketEthPartArray[USE_NUM_PARTS_ETH_CNS];

/**************************** Ipv4 over Ipv4 **********************************/
/* PARTS of packet */
static const TGF_PACKET_PART_STC prvTgfPacketIpv4OverIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_GRE_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketGrePart},
    {TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E,
                                  REMOVE_CONST_PTR_CNS &prvTgfPacketWildcardPart}, /* for  ttiAction.ttHeaderLength/ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable fields */

        /* the passenger */
        {TGF_PACKET_PART_IPV4_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4Part_Passenger},
        {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart},
        {TGF_PACKET_PART_CRC_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketCrcPart_Passenger}
};


/**************************** Ethernet over Ipv4 ******************************/
/* PARTS of packet */
static const TGF_PACKET_PART_STC prvTgfPacketEthOverIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_GRE_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketGrePart},
    {TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E,
                                  REMOVE_CONST_PTR_CNS &prvTgfPacketWildcardPart}, /* for  ttiAction.ttHeaderLength/ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable fields */

        /* the passenger */
        {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag2Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag3Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag4Part_Passenger},
        {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart},
        {TGF_PACKET_PART_CRC_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketCrcPart_Passenger}
};

static const TGF_PACKET_PART_STC prvTgfPacketIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4Part_Tunnel_Passenger},
    {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart}
};

static const TGF_PACKET_PART_STC  prvTgfPacketGenIpv4TemplatePartArray[] = {
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part_Tunnel},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part_Tunnel},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4Part_Tunnel},
    {TGF_PACKET_PART_UDP_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_TEMPLATE_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketTemplatePart},
    {TGF_PACKET_PART_IPV4_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4Part_Tunnel_Passenger},
    {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart}
};


static const TGF_PACKET_PART_STC  prvTgfPacketGenIpv4Template8PartArray[] = {
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part_Tunnel},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part_Tunnel},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4Part_Tunnel},
    {TGF_PACKET_PART_UDP_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_TEMPLATE_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketTemplate8Part},
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart}
};


static const TGF_PACKET_PART_STC  prvTgfPacketGenIpv6TemplatePartArray[] = {
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part_Tunnel},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part_Tunnel},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv6Part_Tunnel},
    {TGF_PACKET_PART_UDP_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_GRE_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketGrePart},
    {TGF_PACKET_PART_TEMPLATE_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketTemplatePart},
    {TGF_PACKET_PART_IPV4_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv4Part_Tunnel_Passenger},
    {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart}
};

static const TGF_PACKET_PART_STC  prvTgfPacketGenIpv6Template8PartArray[] = {
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part_Tunnel},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part_Tunnel},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv6Part_Tunnel},
    {TGF_PACKET_PART_UDP_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_TEMPLATE_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketTemplate8Part},
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart}
};

static const TGF_PACKET_PART_STC prvTgfPacketIpv6PartArray[] = {
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketIpv6Part_Tunnel},
    {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart}
};


#define NUM_PARTS_ETH_OVER_IPV4_CNS (sizeof(prvTgfPacketEthOverIpv4PartArray) / sizeof(prvTgfPacketEthOverIpv4PartArray[0]))
#define NUM_PARTS_IPV4_CNS (sizeof(prvTgfPacketIpv4PartArray) / sizeof(prvTgfPacketIpv4PartArray[0]))
#define NUM_PARTS_IPV6_CNS (sizeof(prvTgfPacketIpv6PartArray) / sizeof(prvTgfPacketIpv6PartArray[0]))
#define NUM_PARTS_GEN_IPV4_UDP_TEMPLATE_CNS (sizeof(prvTgfPacketGenIpv4TemplatePartArray) / sizeof(prvTgfPacketGenIpv4TemplatePartArray[0]))
#define NUM_PARTS_GEN_IPV4_UDP_TEMPLATE8_CNS (sizeof(prvTgfPacketGenIpv4Template8PartArray) / sizeof(prvTgfPacketGenIpv4Template8PartArray[0]))
#define NUM_PARTS_GEN_IPV6_TEMPLATE_CNS (sizeof(prvTgfPacketGenIpv6TemplatePartArray) / sizeof(prvTgfPacketGenIpv6TemplatePartArray[0]))
#define NUM_PARTS_GEN_IPV6_UDP_TEMPLATE8_CNS (sizeof(prvTgfPacketGenIpv6Template8PartArray) / sizeof(prvTgfPacketGenIpv6Template8PartArray[0]))

static TGF_PACKET_PART_STC use_prvTgfPacketEthOverIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &use_prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &use_prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_GRE_E,       &use_prvTgfPacketGrePart},
    {TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E,
                                  &use_prvTgfPacketWildcardPart}, /* for  ttiAction.ttHeaderLength/ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable fields */

        /* the passenger */
        {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag2Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag3Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag4Part_Passenger},
        {TGF_PACKET_PART_PAYLOAD_E,   &use_prvTgfPacketPayloadPart},
        {TGF_PACKET_PART_CRC_E,       &use_prvTgfPacketCrcPart_Passenger}
};
#define USE_NUM_PARTS_ETH_OVER_IPV4_CNS (sizeof(use_prvTgfPacketEthOverIpv4PartArray) / sizeof(use_prvTgfPacketEthOverIpv4PartArray[0]))

static TGF_PACKET_PART_STC use_prvTgfPacketIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag4Part},

    {TGF_PACKET_PART_ETHERTYPE_E, &use_prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &use_prvTgfPacketIpv4Part_Tunnel_Passenger},
    {TGF_PACKET_PART_PAYLOAD_E,   &use_prvTgfPacketPayloadPart}
};
#define USE_NUM_PARTS_IPV4_CNS (sizeof(use_prvTgfPacketIpv4PartArray) / sizeof(use_prvTgfPacketIpv4PartArray[0]))

static TGF_PACKET_PART_STC use_prvTgfPacketGenIpv4TemplatePartArray[] = {
    {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part_Tunnel},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part_Tunnel},
    {TGF_PACKET_PART_ETHERTYPE_E, &use_prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &use_prvTgfPacketIpv4Part_Tunnel},
    {TGF_PACKET_PART_UDP_E,       &use_prvTgfPacketUdpPart},
    {TGF_PACKET_PART_TEMPLATE_E,  &use_prvTgfPacketTemplatePart},
    {TGF_PACKET_PART_IPV4_E,      &use_prvTgfPacketIpv4Part_Tunnel_Passenger},
    {TGF_PACKET_PART_PAYLOAD_E,   &use_prvTgfPacketPayloadPart}
};
#define USE_NUM_PARTS_GEN_IPV4_UDP_TEMPLATE_CNS (sizeof(use_prvTgfPacketGenIpv4TemplatePartArray) / sizeof(use_prvTgfPacketGenIpv4TemplatePartArray[0]))

static TGF_PACKET_PART_STC use_prvTgfPacketGenIpv4Template8PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part_Tunnel},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part_Tunnel},
    {TGF_PACKET_PART_ETHERTYPE_E, &use_prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &use_prvTgfPacketIpv4Part_Tunnel},
    {TGF_PACKET_PART_UDP_E,       &use_prvTgfPacketUdpPart},
    {TGF_PACKET_PART_TEMPLATE_E,  &use_prvTgfPacketTemplate8Part},
    {TGF_PACKET_PART_L2_E  ,      &use_prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &use_prvTgfPacketPayloadPart}
};

#define USE_NUM_PARTS_GEN_IPV4_UDP_TEMPLATE8_CNS (sizeof(use_prvTgfPacketGenIpv4Template8PartArray) / sizeof(use_prvTgfPacketGenIpv4Template8PartArray[0]))

static TGF_PACKET_PART_STC use_prvTgfPacketGenIpv6TemplatePartArray[] = {
    {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part_Tunnel},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part_Tunnel},
    {TGF_PACKET_PART_ETHERTYPE_E, &use_prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &use_prvTgfPacketIpv6Part_Tunnel},
    {TGF_PACKET_PART_UDP_E,       &use_prvTgfPacketUdpPart},
    {TGF_PACKET_PART_GRE_E,       &use_prvTgfPacketGrePart},
    {TGF_PACKET_PART_TEMPLATE_E,  &use_prvTgfPacketTemplatePart},
    {TGF_PACKET_PART_IPV4_E,      &use_prvTgfPacketIpv4Part_Tunnel_Passenger},
    {TGF_PACKET_PART_PAYLOAD_E,   &use_prvTgfPacketPayloadPart}
};

#define USE_NUM_PARTS_GEN_IPV6_TEMPLATE_CNS (sizeof(use_prvTgfPacketGenIpv6TemplatePartArray) / sizeof(use_prvTgfPacketGenIpv6TemplatePartArray[0]))

static TGF_PACKET_PART_STC use_prvTgfPacketGenIpv6Template8PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part_Tunnel},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part_Tunnel},
    {TGF_PACKET_PART_ETHERTYPE_E, &use_prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &use_prvTgfPacketIpv6Part_Tunnel},
    {TGF_PACKET_PART_UDP_E,       &use_prvTgfPacketUdpPart},
    {TGF_PACKET_PART_TEMPLATE_E,  &use_prvTgfPacketTemplate8Part},
    {TGF_PACKET_PART_L2_E  ,      &use_prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &use_prvTgfPacketPayloadPart}
};

#define USE_NUM_PARTS_GEN_IPV6_UDP_TEMPLATE8_CNS (sizeof(use_prvTgfPacketGenIpv6Template8PartArray) / sizeof(use_prvTgfPacketGenIpv6Template8PartArray[0]))

/**************************** Ethernet over MPLS ******************************/
/* PARTS of packet */
static const TGF_PACKET_PART_STC prvTgfEthernetOverMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketMplsLabel1Part},
    {TGF_PACKET_PART_MPLS_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketMplsLabel2Part},
    {TGF_PACKET_PART_MPLS_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketMplsLabel3Part},
    {TGF_PACKET_PART_MPLS_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketMplsLabel4Part},
    {TGF_PACKET_PART_MPLS_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketPwLabelPart},
    {TGF_PACKET_PART_MPLS_E,      REMOVE_CONST_PTR_CNS &prvTgfPacketFlowLabelPart},
    {TGF_PACKET_PART_WILDCARD_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketCwLabelPart},

    /* the passenger */
        {TGF_PACKET_PART_L2_E,        REMOVE_CONST_PTR_CNS &prvTgfPacketL2Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag1Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag2Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag3Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  REMOVE_CONST_PTR_CNS &prvTgfPacketVlanTag4Part_Passenger},
        {TGF_PACKET_PART_ETHERTYPE_E, REMOVE_CONST_PTR_CNS &prvTgfPacketEtherTypePart_Passenger},
        {TGF_PACKET_PART_PAYLOAD_E,   REMOVE_CONST_PTR_CNS &prvTgfPacketPayloadPart},
        {TGF_PACKET_PART_CRC_E,       REMOVE_CONST_PTR_CNS &prvTgfPacketCrcPart_Passenger}
};

#define NUM_PARTS_ETH_OVER_MPLS_CNS (sizeof(prvTgfEthernetOverMplsPacketPartArray) / sizeof(prvTgfEthernetOverMplsPacketPartArray[0]))

static TGF_PACKET_PART_STC save_use_prvTgfEthernetOverMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,         &use_prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,   &use_prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,   &use_prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,   &use_prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,   &use_prvTgfPacketVlanTag4Part},
    {TGF_PACKET_PART_ETHERTYPE_E,  &use_prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,       &use_prvTgfPacketMplsLabel1Part},
    {TGF_PACKET_PART_MPLS_E,       &use_prvTgfPacketMplsLabel2Part},
    {TGF_PACKET_PART_MPLS_E,       &use_prvTgfPacketMplsLabel3Part},
    {TGF_PACKET_PART_MPLS_E,       &use_prvTgfPacketMplsLabel4Part},
    {TGF_PACKET_PART_MPLS_E,       &use_prvTgfPacketPwLabelPart},
    {TGF_PACKET_PART_MPLS_E,       &use_prvTgfPacketFlowLabelPart},
    {TGF_PACKET_PART_WILDCARD_E,   &use_prvTgfPacketCwLabelPart},

    /* the passenger */
        {TGF_PACKET_PART_L2_E,         &use_prvTgfPacketL2Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,   &use_prvTgfPacketVlanTag1Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,   &use_prvTgfPacketVlanTag2Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,   &use_prvTgfPacketVlanTag3Part_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,   &use_prvTgfPacketVlanTag4Part_Passenger},
        {TGF_PACKET_PART_ETHERTYPE_E,  &use_prvTgfPacketEtherTypePart_Passenger},
        {TGF_PACKET_PART_PAYLOAD_E,    &use_prvTgfPacketPayloadPart},
        {TGF_PACKET_PART_CRC_E,        &use_prvTgfPacketCrcPart_Passenger}
};

#define USE_NUM_PARTS_ETH_OVER_MPLS_CNS (sizeof(save_use_prvTgfEthernetOverMplsPacketPartArray) / sizeof(save_use_prvTgfEthernetOverMplsPacketPartArray[0]))

static TGF_PACKET_PART_STC use_prvTgfEthernetOverMplsPacketPartArray[USE_NUM_PARTS_ETH_OVER_MPLS_CNS];

/*******************ipv6 packet******************/
static TGF_PACKET_PART_STC use_prvTgfPacketIpv6PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &use_prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &use_prvTgfPacketVlanTag4Part},

    {TGF_PACKET_PART_ETHERTYPE_E, &use_prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &use_prvTgfPacketIpv6Part_Tunnel},
    {TGF_PACKET_PART_PAYLOAD_E,   &use_prvTgfPacketPayloadPart}
};

/**
* @internal prvTgfPacketHeaderPartGet function
* @endinternal
*
* @brief   Gets packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] packetInfoPtr            - (pointer to) packet's info
*
* @param[out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - Item not found
*/
GT_STATUS prvTgfPacketHeaderPartGet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    TGF_PACKET_STC      *packetInfoPtr,
    OUT   GT_VOID             **packetHeaderPartPtr
)
{

    GT_U32 ii = 0;
    while ((packetInfoPtr->partsArray[ii].type != packetHeaderPartType) && (packetInfoPtr->numOfParts >= ii))
        ii++;

    if (packetInfoPtr->partsArray[ii].type == packetHeaderPartType)
    {
        *packetHeaderPartPtr = packetInfoPtr->partsArray[ii].partPtr;
        return GT_OK;
    }

    return GT_NOT_FOUND;
}

/**
* @internal prvTgfPacketCopyParts function
* @endinternal
*
* @brief   Gets Eth over Ipv4 default packet.
*
* @param[in,out] targetPartPtr            - (pointer to) target part.
* @param[in] srcPartPtr               - (pointer to) source part.
* @param[in] partType                 - type of packet part
* @param[in,out] targetPartPtr            - (pointer to) target part.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvTgfPacketCopyParts
(
    INOUT TGF_PACKET_PART_STC    *targetPartPtr,
    IN  const TGF_PACKET_PART_STC  *srcPartPtr,
    IN  TGF_PACKET_PART_ENT      partType
)
{
    CPSS_NULL_PTR_CHECK_MAC(srcPartPtr);
    CPSS_NULL_PTR_CHECK_MAC(targetPartPtr);

    targetPartPtr->type = srcPartPtr->type;

    switch(partType)
    {
    case TGF_PACKET_PART_L2_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_L2_STC));
        break;
    case TGF_PACKET_PART_VLAN_TAG_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_VLAN_TAG_STC));
        break;
    case TGF_PACKET_PART_ETHERTYPE_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_ETHERTYPE_STC));
        break;
    case TGF_PACKET_PART_MPLS_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_MPLS_STC));
        break;
    case TGF_PACKET_PART_IPV4_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_IPV4_STC));
        break;
    case TGF_PACKET_PART_IPV6_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_IPV6_STC));
        break;
    case TGF_PACKET_PART_WILDCARD_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_WILDCARD_STC));
        break;
    case TGF_PACKET_PART_PAYLOAD_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_PAYLOAD_STC));
        break;
    case TGF_PACKET_PART_GRE_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_GRE_STC));
        break;
    case TGF_PACKET_PART_CRC_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_CRC_STC));
        break;
    case TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_TUNNEL_HEADER_FORCE_TILL_PASSENGER_STC));
        break;
    case TGF_PACKET_PART_UDP_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_UDP_STC));
        break;
    case TGF_PACKET_PART_TEMPLATE_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(TGF_PACKET_TEMPLATE16_STC));
        break;
    case TGF_PACKET_PART_SKIP_E:
        cpssOsMemCpy(targetPartPtr->partPtr,srcPartPtr->partPtr,sizeof(GT_PTR));
        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal prvTgfPacketNumberOfVlanTagsSet function
* @endinternal
*
* @brief   Sets number of vlan tags in tunnel or passenger header.
*
* @param[in] isPassenger              - whether this is passenger or tunnel
* @param[in] numberOfVlanTags         - number of vlan tags
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Default value for global parameters prvTgfPacketNumberOfVlanTagsPassenger
*       and prvTgfPacketNumberOfVlanTags is 1.
*
*/
GT_STATUS prvTgfPacketNumberOfVlanTagsSet
(
    IN    GT_BOOL    isPassenger,
    IN    GT_U32     numberOfVlanTags
)
{
    if (isPassenger)
    {
        prvTgfPacketNumberOfVlanTagsPassenger = numberOfVlanTags;
    }
    else
    {
        prvTgfPacketNumberOfVlanTags = numberOfVlanTags;
    }

    return GT_OK;
}

/**
* @internal prvTgfPacketNumberOfMplsLabelsSet function
* @endinternal
*
* @brief   Sets number of MPLS labels in header.
*
* @param[in] numberOfMplsLabels       - number of MPLS labels
*                                       None.
*
* @note Default value for global parameter prvTgfPacketNumberOfMplsLabels is 1
*
*/
GT_STATUS prvTgfPacketNumberOfMplsLabelsSet
(
    IN    GT_BOOL  numberOfMplsLabels
)
{
    prvTgfPacketNumberOfMplsLabels = numberOfMplsLabels;

    return GT_OK;
}

/**
* @internal prvTgfPacketIsGreSet function
* @endinternal
*
* @brief   Sets whether the packet has GRE part in header.
*
* @param[in] isGre                    - Whether GRE part is included in packet header
*                                       None.
*
* @note Default value for global parameter prvTgfPacketIsGre is GT_TRUE
*
*/
GT_VOID prvTgfPacketIsGreSet
(
    IN    GT_BOOL  isGre
)
{
    prvTgfPacketIsGre = isGre;
}

/**
* @internal prvTgfPacketIsCrcSet function
* @endinternal
*
* @brief   Sets whether the packet has CRC part in header.
*
* @param[in] isCrc                    - Whether CRC part is included in packet header
*                                       None.
*
* @note Default value for global parameter prvTgfPacketIsCrc is GT_FALSE
*
*/
GT_VOID prvTgfPacketIsCrcSet
(
    IN    GT_BOOL  isCrc
)
{
    prvTgfPacketIsCrc = isCrc;
}

/**
* @internal prvTgfPacketIsWildcardSet function
* @endinternal
*
* @brief   Sets whether the packet has wildcard part in header.
*
* @param[in] isWildcard               - Whether wildcard part is included in packet header
*                                       None.
*
* @note Default value for global parameter prvTgfPacketIsWildcard is GT_FALSE
*
*/
GT_VOID prvTgfPacketIsWildcardSet
(
    IN    GT_BOOL  isWildcard
)
{
    prvTgfPacketIsWildcard = isWildcard;
}

/**
* @internal prvTgfPacketTunnelHeaderForceTillPassengersSet function
* @endinternal
*
* @brief   Sets number of MPLS labels in header.
*
* @param[in] numberOfTunnelHeaderAdditionalBytes - number of additional bytes when
*                                      before passenger
*                                       None.
*
* @note Default value for global parameter prvTgfPacketTunnelHeaderForceTillPassenger is 0
*
*/
GT_VOID prvTgfPacketTunnelHeaderForceTillPassengersSet
(
    IN    GT_U32  numberOfTunnelHeaderAdditionalBytes
)
{
    prvTgfPacketTunnelHeaderForceBytesTillPassenger = numberOfTunnelHeaderAdditionalBytes;
}

/**
* @internal prvTgfPacketRestoreDefaultParameters function
* @endinternal
*
* @brief   Sets default values for global parametsers.
*/
GT_VOID prvTgfPacketRestoreDefaultParameters
(
    GT_VOID
)
{
    prvTgfPacketNumberOfVlanTags                    = DEFAULT_NUM_OF_VLAN_TAGS_CNS;
    prvTgfPacketNumberOfVlanTagsPassenger           = DEFAULT_NUM_OF_VLAN_TAGS_PASSENGER_CNS;
    prvTgfPacketNumberOfMplsLabels                  = DEFAULT_NUM_OF_MPLS_LABELS_CNS;
    prvTgfPacketIsGre                               = DEFAULT_IS_GRE_CNS;
    prvTgfPacketTunnelHeaderForceBytesTillPassenger = DEFAULT_TUNNEL_HEADER_FORCE_BYTES_TILL_PASSENGER_CNS;
    prvTgfPacketIsCrc                               = DEFAULT_IS_CRC_CNS;

    return;
}


/**
* @internal prvTgfPAcketIpv4OverIpv4PacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Ipv4 over Ipv4 packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] isPassenger              - whether this is passenger or tunnel
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       passenger:
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*       TGF_PACKET_PART_CRC_E
*
*/
GT_STATUS prvTgfPAcketIpv4OverIpv4PacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_BOOL              isPassenger,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
)
{
    /* to avoid compilation warnings */
    TGF_PARAM_NOT_USED(packetHeaderPartType);
    TGF_PARAM_NOT_USED(isPassenger);
    TGF_PARAM_NOT_USED(partIndex);
    TGF_PARAM_NOT_USED(packetHeaderPartPtr);

    return GT_OK;
}


/**
* @internal prvTgfPacketEthOverIpv4PacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Eth over Ipv4 packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] isPassenger              - whether this is passenger or tunnel
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_FOUND             - on not found
*
* @note Packet definision -
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_GRE_E
*       TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_PAYLOAD_E
*       TGF_PACKET_PART_CRC_E
*
*/
GT_STATUS prvTgfPacketEthOverIpv4PacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_BOOL              isPassenger,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
)
{
    GT_U32 ii = 0;

    /* tunnel header */
    if (isPassenger == GT_FALSE)
    {
        switch (packetHeaderPartType)
        {
        case TGF_PACKET_PART_L2_E:
            /* find first L2 part in tunnel */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_L2_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii].partPtr =
                (TGF_PACKET_L2_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_VLAN_TAG_E:
            /* vlan index is bigger then number of defined vlans */
            if (prvTgfPacketNumberOfVlanTags < partIndex)
            {
                return GT_BAD_PARAM;
            }
            /* find first vlan part in tunnel */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii + partIndex].partPtr =
                (TGF_PACKET_VLAN_TAG_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_ETHERTYPE_E:
            /* find first ethertype part in tunnel */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_ETHERTYPE_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii].partPtr =
                (TGF_PACKET_ETHERTYPE_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_IPV4_E:
            /* find first ipv4 part in tunnel */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_IPV4_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii].partPtr =
                (TGF_PACKET_IPV4_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_GRE_E:
            /* find first gre part in tunnel */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_GRE_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii].partPtr =
                (TGF_PACKET_PAYLOAD_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E:
            /* find first tunnel header additional bytes part in tunnel */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii].partPtr =
                (TGF_PACKET_PAYLOAD_STC *)packetHeaderPartPtr;
            break;
        default:
            break;
        }
    }
    else /* passenger */
    {
        switch (packetHeaderPartType)
        {
        case TGF_PACKET_PART_L2_E:
            /* find first L2 part in tunnel */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_L2_E))
                ii++;
            /* continue to find second L2 part in passenger */
            ii++;
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_L2_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii].partPtr =
                (TGF_PACKET_L2_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_VLAN_TAG_E:
            /* vlan index is bigger then number of defined vlans */
            if (prvTgfPacketNumberOfVlanTagsPassenger < partIndex)
            {
                return GT_BAD_PARAM;
            }
            /* find first vlan part in tunnel */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            /* continue to find second vlan part in passenger */
            ii += MAX_NUM_OF_VLAN_TAGS_CNS + 1;
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E))
                ii++;
            use_prvTgfPacketEthOverIpv4PartArray[ii + partIndex].partPtr =
                (TGF_PACKET_VLAN_TAG_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_PAYLOAD_E:
            /* find payload part in passenger */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_PAYLOAD_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii].partPtr =
                (TGF_PACKET_PAYLOAD_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_CRC_E:
            /* find first CRC part in passenger */
            while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_CRC_E))
                ii++;
            if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
            {
                return GT_NOT_FOUND;
            }
            use_prvTgfPacketEthOverIpv4PartArray[ii].partPtr =
                (TGF_PACKET_WILDCARD_STC *)packetHeaderPartPtr;
            break;
        default:
            break;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfPacketIpv4OverIpv4PacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Ipv4 over Ipv4 default packet.
*
* @param[in] isGre                    - whether this is GRE packet.
*
* @param[out] packetPtr                - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketIpv4OverIpv4PacketDefaultPacketGet
(
    IN  GT_BOOL                   isGre,
    OUT TGF_PACKET_PART_STC       *packetPtr
)
{
    /* to avoid compilation warnings */
    TGF_PARAM_NOT_USED(isGre);
    TGF_PARAM_NOT_USED(packetPtr);

    return GT_OK;
}


/**
* @internal prvTgfPacketEthernetPacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Ethernet default packet.
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketEthernetPacketDefaultPacketGet
(
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
)
{
    GT_U32 ii,jj;

    GT_U32 userSize = USE_NUM_PARTS_ETH_CNS;
    GT_U32 formalSize = NUM_PARTS_ETH_CNS;
    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* check that new parts weren't added to the default static packet but not to the 'use' default packet */
    if(userSize != formalSize)
    {
        PRV_UTF_LOG2_MAC("Wrong number of parts %d - %d\n", USE_NUM_PARTS_ETH_CNS, NUM_PARTS_ETH_CNS);
        return GT_BAD_SIZE;
    }
    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii<NUM_PARTS_ETH_CNS; ii++)
    {
        /*restore 'orig pointers' of local DB */
        use_prvTgfPacketEthPartArray[ii] = save_use_prvTgfPacketEthPartArray[ii];

        prvTgfPacketCopyParts(&use_prvTgfPacketEthPartArray[ii],
                              &prvTgfPacketEthPartArray[ii],
                              prvTgfPacketEthPartArray[ii].type);
    }

    /* remove all the tunnel vlan tags that are not needed to this test */
    /* find first vlan part in tunnel */
    ii = 0;
    while (use_prvTgfPacketEthPartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
        ii++;
    for (jj = 0; jj<MAX_NUM_OF_VLAN_TAGS_CNS; jj++)
    {
        if (jj >= prvTgfPacketNumberOfVlanTags)
        {
            use_prvTgfPacketEthPartArray[jj + ii].type = TGF_PACKET_PART_SKIP_E;
        }
        else
        {
            use_prvTgfPacketEthPartArray[jj + ii].type = TGF_PACKET_PART_VLAN_TAG_E;
        }
    }

    *numOfPartsPtr = NUM_PARTS_ETH_CNS;
    *packetPartArrayPtr = use_prvTgfPacketEthPartArray;

    return GT_OK;
}




/**
* @internal prvTgfPacketIpv4PacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Ipv4 default packet.
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketIpv4PacketDefaultPacketGet
(
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
)
{
    GT_U32 ii,jj;
    GT_U32 userSize = USE_NUM_PARTS_ETH_OVER_IPV4_CNS;
    GT_U32 formalSize = NUM_PARTS_ETH_OVER_IPV4_CNS;
    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* check that new parts weren't added to the default static packet but not to the 'use' default packet */
    if(userSize != formalSize)
    {
        PRV_UTF_LOG2_MAC("Wrong number of parts %d - %d\n", USE_NUM_PARTS_ETH_OVER_IPV4_CNS, NUM_PARTS_ETH_OVER_IPV4_CNS);
        return GT_BAD_SIZE;
    }

    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii<NUM_PARTS_IPV4_CNS; ii++)
    {
        prvTgfPacketCopyParts(&use_prvTgfPacketIpv4PartArray[ii],
                              &prvTgfPacketIpv4PartArray[ii],
                              prvTgfPacketIpv4PartArray[ii].type);
    }

    /* remove all the tunnel vlan tags that are not needed to this test */
    /* find first vlan part in tunnel */
    ii = 0;
    while (use_prvTgfPacketIpv4PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
        ii++;
    for (jj = 0; jj<MAX_NUM_OF_VLAN_TAGS_CNS; jj++)
    {
        if (jj >= prvTgfPacketNumberOfVlanTagsPassenger)
        {
            use_prvTgfPacketIpv4PartArray[jj + ii].type = TGF_PACKET_PART_SKIP_E;
        }
        else
        {
            use_prvTgfPacketIpv4PartArray[jj + ii].type = TGF_PACKET_PART_VLAN_TAG_E;
        }
    }

    *numOfPartsPtr = NUM_PARTS_IPV4_CNS;
    *packetPartArrayPtr = use_prvTgfPacketIpv4PartArray;

    return GT_OK;
}

/**
* @internal prvTgfPacketIpv6PacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Ipv6 default packet.
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketIpv6PacketDefaultPacketGet
(
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
)
{
    GT_U32 ii,jj;
    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii<NUM_PARTS_IPV6_CNS; ii++)
    {
        prvTgfPacketCopyParts(&use_prvTgfPacketIpv6PartArray[ii],
                              &prvTgfPacketIpv6PartArray[ii],
                              prvTgfPacketIpv6PartArray[ii].type);
    }

    /* remove all the tunnel vlan tags that are not needed to this test */
    /* find first vlan part in tunnel */
    ii = 0;
    while (use_prvTgfPacketIpv6PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
        ii++;
    for (jj = 0; jj<MAX_NUM_OF_VLAN_TAGS_CNS; jj++)
    {
        if (jj >= prvTgfPacketNumberOfVlanTagsPassenger)
        {
            use_prvTgfPacketIpv6PartArray[jj + ii].type = TGF_PACKET_PART_SKIP_E;
        }
        else
        {
            use_prvTgfPacketIpv6PartArray[jj + ii].type = TGF_PACKET_PART_VLAN_TAG_E;
        }
    }

    *numOfPartsPtr = NUM_PARTS_IPV6_CNS;
    *packetPartArrayPtr = use_prvTgfPacketIpv6PartArray;

    return GT_OK;
}

/**
* @internal prvTgfPacketEthernetPacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Ethernet packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_STATUS prvTgfPacketEthernetPacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
)
{
    GT_U32 ii = 0;

    /* tunnel header */
    switch (packetHeaderPartType)
    {
    case TGF_PACKET_PART_L2_E:
        /* find first L2 part in tunnel */
        while (use_prvTgfPacketEthPartArray[ii].type != TGF_PACKET_PART_L2_E)
            ii++;
        use_prvTgfPacketEthPartArray[ii].partPtr =
            (TGF_PACKET_L2_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_VLAN_TAG_E:
        /* vlan index is bigger then number of defined vlans */
        if (prvTgfPacketNumberOfVlanTags < partIndex)
        {
            return GT_BAD_PARAM;
        }
        /* find first vlan part in tunnel */
        while (use_prvTgfPacketEthPartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
            ii++;
        use_prvTgfPacketEthPartArray[ii + partIndex].partPtr =
            (TGF_PACKET_VLAN_TAG_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_ETHERTYPE_E:
        /* find first Ethertype part in tunnel */
        while (use_prvTgfPacketEthPartArray[ii].type != TGF_PACKET_PART_ETHERTYPE_E)
            ii++;
        use_prvTgfPacketEthPartArray[ii].partPtr =
            (TGF_PACKET_ETHERTYPE_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_PAYLOAD_E:
        /* find payload part in passenger */
        while (use_prvTgfPacketEthPartArray[ii].type != TGF_PACKET_PART_PAYLOAD_E)
            ii++;
        use_prvTgfPacketEthPartArray[ii].partPtr =
            (TGF_PACKET_PAYLOAD_STC *)packetHeaderPartPtr;
        break;

    default:
        break;
    }
    return GT_OK;
}



/**
* @internal prvTgfPacketIpv4PacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Ipv4 packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_STATUS prvTgfPacketIpv4PacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
)
{
    GT_U32 ii = 0;

    /* tunnel header */
    switch (packetHeaderPartType)
    {
    case TGF_PACKET_PART_L2_E:
        /* find first L2 part in tunnel */
        while (use_prvTgfPacketIpv4PartArray[ii].type != TGF_PACKET_PART_L2_E)
            ii++;
        use_prvTgfPacketIpv4PartArray[ii].partPtr =
            (TGF_PACKET_L2_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_VLAN_TAG_E:
        /* vlan index is bigger then number of defined vlans */
        if (prvTgfPacketNumberOfVlanTags < partIndex)
        {
            return GT_BAD_PARAM;
        }
        /* find first vlan part in tunnel */
        while (use_prvTgfPacketIpv4PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
            ii++;
        use_prvTgfPacketIpv4PartArray[ii + partIndex].partPtr =
            (TGF_PACKET_VLAN_TAG_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_ETHERTYPE_E:
        /* find first ethertype part in tunnel */
        while (use_prvTgfPacketIpv4PartArray[ii].type != TGF_PACKET_PART_ETHERTYPE_E)
            ii++;
        use_prvTgfPacketIpv4PartArray[ii].partPtr =
            (TGF_PACKET_ETHERTYPE_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_IPV4_E:
        /* find first ipv4 part in tunnel */
        while (use_prvTgfPacketIpv4PartArray[ii].type != TGF_PACKET_PART_IPV4_E)
            ii++;
        use_prvTgfPacketIpv4PartArray[ii].partPtr =
            (TGF_PACKET_IPV4_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_PAYLOAD_E:
        /* find payload part in passenger */
        while (use_prvTgfPacketIpv4PartArray[ii].type != TGF_PACKET_PART_PAYLOAD_E)
            ii++;
        use_prvTgfPacketIpv4PartArray[ii].partPtr =
            (TGF_PACKET_PAYLOAD_STC *)packetHeaderPartPtr;
        break;

    default:
        break;
    }
    return GT_OK;
}
/**
* @internal prvTgfPacketIpv6PacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Ipv6 packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV6_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_STATUS prvTgfPacketIpv6PacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
)
{
    GT_U32 ii = 0;

    /* tunnel header */
    switch (packetHeaderPartType)
    {
    case TGF_PACKET_PART_L2_E:
        /* find first L2 part in tunnel */
        while (use_prvTgfPacketIpv6PartArray[ii].type != TGF_PACKET_PART_L2_E)
            ii++;
        use_prvTgfPacketIpv6PartArray[ii].partPtr =
            (TGF_PACKET_L2_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_VLAN_TAG_E:
        /* vlan index is bigger then number of defined vlans */
        if (prvTgfPacketNumberOfVlanTags < partIndex)
        {
            return GT_BAD_PARAM;
        }
        /* find first vlan part in tunnel */
        while (use_prvTgfPacketIpv6PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
            ii++;
        use_prvTgfPacketIpv6PartArray[ii + partIndex].partPtr =
            (TGF_PACKET_VLAN_TAG_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_ETHERTYPE_E:
        /* find first ethertype part in tunnel */
        while (use_prvTgfPacketIpv6PartArray[ii].type != TGF_PACKET_PART_ETHERTYPE_E)
            ii++;
        use_prvTgfPacketIpv6PartArray[ii].partPtr =
            (TGF_PACKET_ETHERTYPE_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_IPV6_E:
        /* find first ipv4 part in tunnel */
        while (use_prvTgfPacketIpv6PartArray[ii].type != TGF_PACKET_PART_IPV6_E)
            ii++;
        use_prvTgfPacketIpv6PartArray[ii].partPtr =
            (TGF_PACKET_IPV6_STC *)packetHeaderPartPtr;
        break;
    case TGF_PACKET_PART_PAYLOAD_E:
        /* find payload part in passenger */
        while (use_prvTgfPacketIpv6PartArray[ii].type != TGF_PACKET_PART_PAYLOAD_E)
            ii++;
        use_prvTgfPacketIpv6PartArray[ii].partPtr =
            (TGF_PACKET_PAYLOAD_STC *)packetHeaderPartPtr;
        break;

    default:
        break;
    }
    return GT_OK;
}

/**
* @internal prvTgfPacketGenericIpv4TemplateDefaultPacketGet function
* @endinternal
*
* @brief   Gets generic IPV4 tunneled with UDP template data packet.
*
* @param[in] ip_header_protocol       - ip header protocol
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketGenericIpv4TemplateDefaultPacketGet
(
    IN GT_U32                     ip_header_protocol,
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
)
{
    GT_U32 ii;
    GT_U32 userSize = USE_NUM_PARTS_GEN_IPV4_UDP_TEMPLATE_CNS;
    GT_U32 formalSize = NUM_PARTS_GEN_IPV4_UDP_TEMPLATE_CNS;

    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* check that new parts weren't added to the default static packet but not to the 'use' default packet */
    if(userSize != formalSize)
    {
        PRV_UTF_LOG2_MAC("Wrong number of parts %d - %d\n", USE_NUM_PARTS_GEN_IPV4_UDP_TEMPLATE_CNS, NUM_PARTS_GEN_IPV4_UDP_TEMPLATE_CNS);
        return GT_BAD_SIZE;
    }


    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii<NUM_PARTS_GEN_IPV4_UDP_TEMPLATE_CNS; ii++)
    {
        prvTgfPacketCopyParts(&use_prvTgfPacketGenIpv4TemplatePartArray[ii],
                              &prvTgfPacketGenIpv4TemplatePartArray[ii],
                              prvTgfPacketGenIpv4TemplatePartArray[ii].type);
    }
    /* remove UDP or GRE part that is not needed to this test */

    ii = 0;
    while (use_prvTgfPacketGenIpv4TemplatePartArray[ii].type != TGF_PACKET_PART_UDP_E)
        ii++;
    if ( (ip_header_protocol == 0) || (ip_header_protocol == 1)||(ip_header_protocol == 3))
    {
        use_prvTgfPacketGenIpv4TemplatePartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }

    if (ip_header_protocol == 0)
    {
        ii = 0;
        while (use_prvTgfPacketGenIpv4TemplatePartArray[ii].type != TGF_PACKET_PART_TEMPLATE_E)
            ii++;
        use_prvTgfPacketGenIpv4TemplatePartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }


    *numOfPartsPtr = NUM_PARTS_GEN_IPV4_UDP_TEMPLATE_CNS;
    *packetPartArrayPtr = use_prvTgfPacketGenIpv4TemplatePartArray;

    return GT_OK;
}

/**
* @internal prvTgfPacketGenericIpv4Template8DefaultPacketGet function
* @endinternal
*
* @brief   Gets generic IPV4 tunneled with UDP template data packet.
*
* @param[in] ip_header_protocol       - ip header protocol
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketGenericIpv4Template8DefaultPacketGet
(
    IN GT_U32                     ip_header_protocol,
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
)
{
    GT_U32 ii;
    GT_U32 userSize = USE_NUM_PARTS_GEN_IPV4_UDP_TEMPLATE8_CNS;
    GT_U32 formalSize = NUM_PARTS_GEN_IPV4_UDP_TEMPLATE8_CNS;

    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* check that new parts weren't added to the default static packet but not to the 'use' default packet */
    if(userSize != formalSize)
    {
        PRV_UTF_LOG2_MAC("Wrong number of parts %d - %d\n", USE_NUM_PARTS_GEN_IPV4_UDP_TEMPLATE8_CNS, NUM_PARTS_GEN_IPV4_UDP_TEMPLATE8_CNS);
        return GT_BAD_SIZE;
    }


    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii < NUM_PARTS_GEN_IPV4_UDP_TEMPLATE8_CNS; ii++)
    {
        prvTgfPacketCopyParts(&use_prvTgfPacketGenIpv4Template8PartArray[ii],
                              &prvTgfPacketGenIpv4Template8PartArray[ii],
                              prvTgfPacketGenIpv4Template8PartArray[ii].type);
    }
    /* remove UDP or GRE part that is not needed to this test */

    ii = 0;
    while (use_prvTgfPacketGenIpv4Template8PartArray[ii].type != TGF_PACKET_PART_UDP_E)
        ii++;
    if ( (ip_header_protocol == 0) || (ip_header_protocol == 1) )
    {
        use_prvTgfPacketGenIpv4Template8PartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }

    if (ip_header_protocol == 0)
    {
        ii = 0;
        while (use_prvTgfPacketGenIpv4Template8PartArray[ii].type != TGF_PACKET_PART_TEMPLATE_E)
            ii++;
        use_prvTgfPacketGenIpv4Template8PartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }


    *numOfPartsPtr = NUM_PARTS_GEN_IPV4_UDP_TEMPLATE8_CNS;
    *packetPartArrayPtr = use_prvTgfPacketGenIpv4Template8PartArray;

    return GT_OK;
}

/**
* @internal prvTgfPacketGenericIpv6TemplateDefaultPacketGet function
* @endinternal
*
* @brief   Gets generic IPV6 tunneled with UDP/GRE, template data packet.
*
* @param[in] ip_header_protocol       - ip header protocol
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketGenericIpv6TemplateDefaultPacketGet
(
    IN GT_U32                     ip_header_protocol,
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
)
{
    GT_U32 ii;
    GT_U32 userSize = USE_NUM_PARTS_GEN_IPV6_TEMPLATE_CNS;
    GT_U32 formalSize = NUM_PARTS_GEN_IPV6_TEMPLATE_CNS;

    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* check that new parts weren't added to the default static packet but not to the 'use' default packet */
    if(userSize != formalSize)
    {
        PRV_UTF_LOG2_MAC("Wrong number of parts %d - %d\n", USE_NUM_PARTS_GEN_IPV6_TEMPLATE_CNS, NUM_PARTS_GEN_IPV6_TEMPLATE_CNS);
        return GT_BAD_SIZE;
    }


    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii<NUM_PARTS_GEN_IPV6_TEMPLATE_CNS; ii++)
    {
        prvTgfPacketCopyParts(&use_prvTgfPacketGenIpv6TemplatePartArray[ii],
                              &prvTgfPacketGenIpv6TemplatePartArray[ii],
                              prvTgfPacketGenIpv6TemplatePartArray[ii].type);
    }

    /* remove UDP or GRE part that is not needed to this test */

    ii = 0;
    while (use_prvTgfPacketGenIpv6TemplatePartArray[ii].type != TGF_PACKET_PART_UDP_E)
        ii++;
    if ( (ip_header_protocol == 0) || (ip_header_protocol == 1)||(ip_header_protocol == 3) )
    {
        use_prvTgfPacketGenIpv6TemplatePartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }
    ii = 0;
    while (use_prvTgfPacketGenIpv6TemplatePartArray[ii].type != TGF_PACKET_PART_GRE_E)
        ii++;
    if ( (ip_header_protocol == 0) || (ip_header_protocol == 2)||(ip_header_protocol == 3) )
    {
        use_prvTgfPacketGenIpv6TemplatePartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }
    if (ip_header_protocol == 0)
    {
        ii = 0;
        while (use_prvTgfPacketGenIpv6TemplatePartArray[ii].type != TGF_PACKET_PART_TEMPLATE_E)
            ii++;
        use_prvTgfPacketGenIpv6TemplatePartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }

    *numOfPartsPtr = NUM_PARTS_GEN_IPV6_TEMPLATE_CNS;
    *packetPartArrayPtr = use_prvTgfPacketGenIpv6TemplatePartArray;

    return GT_OK;
}

/**
* @internal prvTgfPacketGenericIpv6Template8DefaultPacketGet function
* @endinternal
*
* @brief   Gets generic IPV6 tunneled with UDP template data packet.
*
* @param[in] ip_header_protocol       - ip header protocol
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketGenericIpv6Template8DefaultPacketGet
(
    IN GT_U32                     ip_header_protocol,
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
)
{
    GT_U32 ii;
    GT_U32 userSize = USE_NUM_PARTS_GEN_IPV6_UDP_TEMPLATE8_CNS;
    GT_U32 formalSize = NUM_PARTS_GEN_IPV6_UDP_TEMPLATE8_CNS;

    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* check that new parts weren't added to the default static packet but not to the 'use' default packet */
    if(userSize != formalSize)
    {
        PRV_UTF_LOG2_MAC("Wrong number of parts %d - %d\n", USE_NUM_PARTS_GEN_IPV6_UDP_TEMPLATE8_CNS, NUM_PARTS_GEN_IPV6_UDP_TEMPLATE8_CNS);
        return GT_BAD_SIZE;
    }


    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii < NUM_PARTS_GEN_IPV6_UDP_TEMPLATE8_CNS; ii++)
    {
        prvTgfPacketCopyParts(&use_prvTgfPacketGenIpv6Template8PartArray[ii],
                              &prvTgfPacketGenIpv6Template8PartArray[ii],
                              prvTgfPacketGenIpv6Template8PartArray[ii].type);
    }
    /* remove UDP or GRE part that is not needed to this test */

    ii = 0;
    while (use_prvTgfPacketGenIpv6Template8PartArray[ii].type != TGF_PACKET_PART_UDP_E)
        ii++;
    if ( (ip_header_protocol == 0) || (ip_header_protocol == 1) )
    {
        use_prvTgfPacketGenIpv6Template8PartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }

    if (ip_header_protocol == 0)
    {
        ii = 0;
        while (use_prvTgfPacketGenIpv6Template8PartArray[ii].type != TGF_PACKET_PART_TEMPLATE_E)
            ii++;
        use_prvTgfPacketGenIpv6Template8PartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }


    *numOfPartsPtr = NUM_PARTS_GEN_IPV6_UDP_TEMPLATE8_CNS;
    *packetPartArrayPtr = use_prvTgfPacketGenIpv6Template8PartArray;

    return GT_OK;
}


/**
* @internal prvTgfPacketEthOverIpv4PacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Eth over Ipv4 default packet.
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketEthOverIpv4PacketDefaultPacketGet
(
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
)
{
    GT_U32 ii,jj;
    GT_U32 userSize = USE_NUM_PARTS_ETH_OVER_IPV4_CNS;
    GT_U32 formalSize = NUM_PARTS_ETH_OVER_IPV4_CNS;

    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* check that new parts weren't added to the default static packet but not to the 'use' default packet */
    if(userSize != formalSize)
    {
        PRV_UTF_LOG2_MAC("Wrong number of parts %d - %d\n", USE_NUM_PARTS_ETH_OVER_IPV4_CNS, NUM_PARTS_ETH_OVER_IPV4_CNS);
        return GT_BAD_SIZE;
    }


    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii<NUM_PARTS_ETH_OVER_IPV4_CNS; ii++)
    {
        prvTgfPacketCopyParts(&use_prvTgfPacketEthOverIpv4PartArray[ii],
                              &prvTgfPacketEthOverIpv4PartArray[ii],
                              prvTgfPacketEthOverIpv4PartArray[ii].type);
    }

    /* remove all the tunnel vlan tags that are not needed to this test */
    /* find first vlan part in tunnel */
    ii = 0;
    while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E))
        ii++;

    for (jj = 0; jj<MAX_NUM_OF_VLAN_TAGS_CNS; jj++)
    {
        if ((jj + ii) == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
        {
            return GT_NOT_FOUND;
        }
        if (jj >= prvTgfPacketNumberOfVlanTags)
        {
            use_prvTgfPacketEthOverIpv4PartArray[jj + ii].type = TGF_PACKET_PART_SKIP_E;
        }
        else
        {
            use_prvTgfPacketEthOverIpv4PartArray[jj + ii].type = TGF_PACKET_PART_VLAN_TAG_E;
        }
    }

    /* remove all the passenger vlan tags that are not needed to this test */
    /* continue to find first vlan part in passenger */
    ii += MAX_NUM_OF_VLAN_TAGS_CNS + 1;
    while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E))
        ii++;
    for (jj = 0; jj<MAX_NUM_OF_VLAN_TAGS_CNS; jj++)
    {
        if ((jj + ii) == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
        {
            return GT_NOT_FOUND;
        }
        if (jj >= prvTgfPacketNumberOfVlanTagsPassenger)
        {
            use_prvTgfPacketEthOverIpv4PartArray[jj + ii].type = TGF_PACKET_PART_SKIP_E;
        }
        else
        {
            use_prvTgfPacketEthOverIpv4PartArray[jj + ii].type = TGF_PACKET_PART_VLAN_TAG_E;
        }
    }

    /* remove gre part that is not needed to this test */
    /* find first gre part in tunnel */
    ii = 0;
    while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_GRE_E))
        ii++;
    if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
    {
        return GT_NOT_FOUND;
    }
    if (prvTgfPacketIsGre == GT_FALSE)
    {
        use_prvTgfPacketEthOverIpv4PartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }
    else
    {
        use_prvTgfPacketEthOverIpv4PartArray[ii].type = TGF_PACKET_PART_GRE_E;
    }

    /* remove payload part that is not needed to this test */
    /* find first additional header part in tunnel */
    ii = 0;
    while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E))
        ii++;
    if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
    {
        return GT_NOT_FOUND;
    }
    if (prvTgfPacketTunnelHeaderForceBytesTillPassenger == 0)
    {
        use_prvTgfPacketEthOverIpv4PartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }
    else
    {
        use_prvTgfPacketEthOverIpv4PartArray[ii].type = TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E;
    }

    /* remove crc part that is not needed to this test */
    /* find first crc part in passenger */
    ii = 0;
    while ((ii < USE_NUM_PARTS_ETH_OVER_IPV4_CNS) && (use_prvTgfPacketEthOverIpv4PartArray[ii].type != TGF_PACKET_PART_CRC_E))
        ii++;
    if (ii == USE_NUM_PARTS_ETH_OVER_IPV4_CNS)
    {
        return GT_NOT_FOUND;
    }
    if (prvTgfPacketIsCrc == GT_FALSE)
    {
        use_prvTgfPacketEthOverIpv4PartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }
    else
    {
        use_prvTgfPacketEthOverIpv4PartArray[ii].type = TGF_PACKET_PART_CRC_E;
    }

    *numOfPartsPtr = NUM_PARTS_ETH_OVER_IPV4_CNS;
    *packetPartArrayPtr = use_prvTgfPacketEthOverIpv4PartArray;

    return GT_OK;
}

/***************MPLS****************/


/**
* @internal prvTgfPacketEthernetOverMplsPacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Mpls default packet.
*
* @param[in] passengerEthertype       - ethertype for the passenger of the MPLS tunnel.
*                                      value 0x0800 --> means IPv4 as L2 of the ethernet.
*                                      else --> means regular L2 ethernet (no L3).
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketEthernetOverMplsPacketDefaultPacketGet
(
    IN TGF_ETHER_TYPE               passengerEthertype,
    OUT GT_U32                      *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC         *packetPartArrayPtr[]
)
{
    GT_U32 ii,jj;
    GT_U32 userSize = USE_NUM_PARTS_ETH_OVER_MPLS_CNS;
    GT_U32 formalSize = NUM_PARTS_ETH_OVER_MPLS_CNS;

    CPSS_NULL_PTR_CHECK_MAC(numOfPartsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetPartArrayPtr);

    /* to avoid compilation warnings */
    passengerEthertype = passengerEthertype;

    /* check that new parts weren't added to the default static packet but not to the 'use' default packet */
    if(userSize != formalSize)
    {
        PRV_UTF_LOG2_MAC("Wrong number of parts %d - %d\n", USE_NUM_PARTS_ETH_OVER_MPLS_CNS, NUM_PARTS_ETH_OVER_MPLS_CNS);
        return GT_BAD_SIZE;
    }

    /* copy parts from the default static const packet to the 'use' packet */
    for(ii=0; ii<NUM_PARTS_ETH_OVER_MPLS_CNS; ii++)
    {
        /*restore 'orig pointers' of local DB */
        use_prvTgfEthernetOverMplsPacketPartArray[ii] = save_use_prvTgfEthernetOverMplsPacketPartArray[ii];
        /*restore 'orig values' of local DB */
        prvTgfPacketCopyParts(&use_prvTgfEthernetOverMplsPacketPartArray[ii],
                              &prvTgfEthernetOverMplsPacketPartArray[ii],
                              prvTgfEthernetOverMplsPacketPartArray[ii].type);
    }

    /* remove all the tunnel vlan tags that are not needed to this test */
    /* find first vlan part in tunnel */
    ii = 0;
    while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
        ii++;
    for (jj = 0; jj<MAX_NUM_OF_VLAN_TAGS_CNS; jj++)
    {
        if (jj >= prvTgfPacketNumberOfVlanTags)
            use_prvTgfEthernetOverMplsPacketPartArray[jj + ii].type = TGF_PACKET_PART_SKIP_E;
        else
            use_prvTgfEthernetOverMplsPacketPartArray[jj + ii].type = TGF_PACKET_PART_VLAN_TAG_E;
    }

    /* remove all the tunnel mpls labels that are not needed to this test */
    /* continue to find first label part in tunnel */
    ii += jj;
    while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_MPLS_E)
        ii++;
    for (jj = 0; jj<MAX_NUM_OF_MPLS_LABELS_CNS; jj++)
    {
        if (jj >= prvTgfPacketNumberOfMplsLabels)
            use_prvTgfEthernetOverMplsPacketPartArray[jj + ii].type = TGF_PACKET_PART_SKIP_E;
        else
            use_prvTgfEthernetOverMplsPacketPartArray[jj + ii].type = TGF_PACKET_PART_MPLS_E;
    }

    /* remove all the tunnel wildcards that are not needed to this test */
    /* continue to find wildcard part in tunnel */
    ii += jj;
    while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_WILDCARD_E)
        ii++;
    if (prvTgfPacketIsWildcard == GT_FALSE)
    {
        use_prvTgfEthernetOverMplsPacketPartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }
    else
    {
        use_prvTgfEthernetOverMplsPacketPartArray[ii].type = TGF_PACKET_PART_WILDCARD_E;
    }

    /* remove all the passenger vlan tags that are not needed to this test */
    /* continue to find first vlan part in passenger */
    ii ++;
    while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
        ii++;
    for (jj = 0; jj<MAX_NUM_OF_VLAN_TAGS_CNS; jj++)
    {
        if (jj >= prvTgfPacketNumberOfVlanTagsPassenger)
            use_prvTgfEthernetOverMplsPacketPartArray[jj + ii].type = TGF_PACKET_PART_SKIP_E;
        else
            use_prvTgfEthernetOverMplsPacketPartArray[jj + ii].type = TGF_PACKET_PART_VLAN_TAG_E;
    }

    /* we are the end of the vlan tags of the passenger */
    ii += jj;

    /* remove crc part that is not needed to this test */
    /* find first crc part in passenger */
    ii = 0;
    while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_CRC_E)
        ii++;
    if (prvTgfPacketIsCrc == GT_FALSE)
    {
        use_prvTgfEthernetOverMplsPacketPartArray[ii].type = TGF_PACKET_PART_SKIP_E;
    }
    else
    {
        use_prvTgfEthernetOverMplsPacketPartArray[ii].type = TGF_PACKET_PART_CRC_E;
    }

    *numOfPartsPtr = NUM_PARTS_ETH_OVER_MPLS_CNS;
    *packetPartArrayPtr = use_prvTgfEthernetOverMplsPacketPartArray;

    return GT_OK;
}

/**
* @internal prvTgfPacketEthOverMplsPacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Eth over Mpls packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] isPassenger              - whether this is passenger or tunnel
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_MPLS_E
*       TGF_PACKET_PART_MPLS_E
*       TGF_PACKET_PART_MPLS_E
*       TGF_PACKET_PART_MPLS_E (PW LABEL)
*       TGF_PACKET_PART_MPLS_E (FLOW LABEL)
*       TGF_PACKET_PART_WILDCARD_E (CW LABEL)
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_PAYLOAD_E
*       TGF_PACKET_PART_CRC_E
*
*/
GT_STATUS prvTgfPacketEthOverMplsPacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_BOOL              isPassenger,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
)
{
    GT_U32 ii = 0;

    /* tunnel header */
    if (isPassenger == GT_FALSE)
    {
        switch (packetHeaderPartType)
        {
        case TGF_PACKET_PART_L2_E:
            /* find first L2 part in tunnel */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_L2_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii].partPtr =
                (TGF_PACKET_L2_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_VLAN_TAG_E:
            /* vlan index is bigger then number of defined vlans */
            if (prvTgfPacketNumberOfVlanTags < partIndex)
            {
                return GT_BAD_PARAM;
            }
            /* find first vlan part in tunnel */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii + partIndex].partPtr =
                (TGF_PACKET_VLAN_TAG_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_ETHERTYPE_E:
            /* find first ethertype part in tunnel */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_ETHERTYPE_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii].partPtr =
                (TGF_PACKET_ETHERTYPE_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_MPLS_E:
            /* find first mpls part in tunnel */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_MPLS_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii + partIndex].partPtr =
                (TGF_PACKET_MPLS_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_WILDCARD_E:
            /* find first wildcard part in tunnel */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_WILDCARD_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii + partIndex].partPtr =
                (TGF_PACKET_WILDCARD_STC *)packetHeaderPartPtr;
            break;
        default:
            break;
        }
    }
    else /* passenger */
    {
        switch (packetHeaderPartType)
        {
        case TGF_PACKET_PART_L2_E:
            /* find first L2 part in tunnel */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_L2_E)
                ii++;
            /* continue to find second L2 part in passenger */
            ii++;
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_L2_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii].partPtr =
                (TGF_PACKET_L2_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_VLAN_TAG_E:
            /* vlan index is bigger then number of defined vlans */
            if (prvTgfPacketNumberOfVlanTagsPassenger < partIndex)
            {
                return GT_BAD_PARAM;
            }
            /* find first vlan part in tunnel */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
                ii++;
            /* continue to find second vlan part in passenger */
            ii += MAX_NUM_OF_VLAN_TAGS_CNS + 1;
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_VLAN_TAG_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii + partIndex].partPtr =
                (TGF_PACKET_VLAN_TAG_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_ETHERTYPE_E:
            /* find first ethertype part in tunnel */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_ETHERTYPE_E)
                ii++;
            /* continue to find second ethertype part in passenger */
            ii++;
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_ETHERTYPE_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii].partPtr =
                (TGF_PACKET_ETHERTYPE_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_PAYLOAD_E:
            /* find payload part in passenger */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_PAYLOAD_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii].partPtr =
                (TGF_PACKET_PAYLOAD_STC *)packetHeaderPartPtr;
            break;
        case TGF_PACKET_PART_CRC_E:
            /* find first CRC part in passenger */
            while (use_prvTgfEthernetOverMplsPacketPartArray[ii].type != TGF_PACKET_PART_CRC_E)
                ii++;
            use_prvTgfEthernetOverMplsPacketPartArray[ii].partPtr =
                (TGF_PACKET_WILDCARD_STC *)packetHeaderPartPtr;
            break;
        default:
            break;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfPacketGenericIpvTemplateSetGenericProtocol function
* @endinternal
*
* @brief   Set protocol number for generic ipv4/ipv6 tunneled packets
*
* @param[in] protocolNumber           - 8 bits protocol number
* @param[in] isIpv6                   - GT_TRUE if this is generic ipv6 tunnel ,else generic ipv4 tunnel
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfPacketGenericIpvTemplateSetGenericProtocol
(
    IN GT_U32                     protocolNumber,
    IN GT_BOOL                    isIpv6

)
{
    GT_U32 ii;
    TGF_PACKET_IPV4_STC * ipv4Ptr;
    TGF_PACKET_IPV6_STC * ipv6Ptr;

    if(isIpv6 == GT_FALSE)
    {

        ii = 0;
        while (use_prvTgfPacketGenIpv4TemplatePartArray[ii].type != TGF_PACKET_PART_IPV4_E)
            ii++;

       ipv4Ptr = use_prvTgfPacketGenIpv4TemplatePartArray[ii].partPtr;

       ipv4Ptr->protocol = (TGF_PROT)protocolNumber;

       ipv4Ptr->totalLen = 0x78;
    }
    else
    {
        ii = 0;
        while (use_prvTgfPacketGenIpv6TemplatePartArray[ii].type != TGF_PACKET_PART_IPV6_E)
            ii++;

       ipv6Ptr = use_prvTgfPacketGenIpv6TemplatePartArray[ii].partPtr;

       ipv6Ptr->nextHeader = (TGF_NEXT_HEADER)protocolNumber;

       ipv6Ptr->payloadLen = 0x64;
    }

   return GT_OK;
}



