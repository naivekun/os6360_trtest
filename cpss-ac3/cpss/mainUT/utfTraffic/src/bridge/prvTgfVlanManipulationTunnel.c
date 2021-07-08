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
* @file prvTgfVlanManipulationTunnel.c
*
* @brief VLAN manipulation
*
* @version   18
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <port/prvTgfPortFWS.h>
#include <bridge/prvTgfVlanManipulation.h>
#include <common/tgfCommon.h>
#include <common/tgfMirror.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <utf/private/prvUtfExtras.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    GT_STATUS sip5Emulate_IngressTpidProfileGet
    (
        IN  CPSS_ETHER_MODE_ENT  ethMode,
        IN  GT_U32               tpidBmp,
        OUT GT_U32               *profilePtr
    );
    GT_STATUS sip5Emulate_IngressTpidProfileClear
    (
        IN  CPSS_ETHER_MODE_ENT  ethMode,
        IN  GT_U32               profileIndex
    );


#endif /*CHX_FAMILY*/

/* for VC as usually used compiler - check mismatch of sizeof().  */
#ifdef _VISUALC
    #define CHECK_SIZE_MISMATCH
#endif

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default number of packets to send */
static GT_U32       prvTgfBurstCount   = 1;

static GT_U32   totalError = 0;

static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

#define ETHERTYPE_0x1234    0x1234
#define ETHERTYPE_0x5678    0x5678
#define ETHERTYPE_0xACBD    0xACBD
#define ETHERTYPE_0xE1E1    0xE1E1

/* VLAN_TAG part of packet */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlan1TagPart = {ETHERTYPE_0x1234,/* etherType */ 0, 0, 5 /* pri, cfi, VlanId */};
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlan2TagPart = {ETHERTYPE_0x5678,/* etherType */ 0, 0, 6 /* pri, cfi, VlanId */};
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlan3TagPart = {ETHERTYPE_0xACBD,/* etherType */ 0, 0, 0x111 /* pri, cfi, VlanId */};
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlan4TagPart = {ETHERTYPE_0xE1E1,/* etherType */ 0, 0, 0x222 /* pri, cfi, VlanId */};

/* packet's MPLS ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart = {TGF_ETHERTYPE_8847_MPLS_TAG_CNS};

/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabel1Part =
{
    1,                  /* label */
    0,                  /* experimental use */
    0,                  /* stack --> NOT end of stack (last bit = 0) */
    0x15                /* timeToLive */
};
/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabel2Part =
{
    2,                  /* label */
    0,                  /* experimental use */
    0,                  /* stack --> NOT end of stack (last bit = 0) */
    0x16                /* timeToLive */
};
/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabel3Part =
{
    3,                  /* label */
    0,                  /* experimental use */
    1,                  /* stack --> end of stack (last bit) */
    0x17                /* timeToLive */
};

static TGF_PACKET_L2_STC prvTgfPacketL2Part_Passenger =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x44}                 /* saMac */
};

#define ETHERTYPE_0x8765    0x8765
#define ETHERTYPE_0x4321    0x4321
#define ETHERTYPE_0xABAB    0xABAB

/* max entry index for TPID table */
#define PRV_TGF_MAX_ENTRY_INDEX_CNS  8

/* original PCL id for MPLS packets -- in the TTi lookup */
static GT_U32   origMplsTtiPclId = 0;

/* stored default TPID table entries */
static GT_U16   ethertypesTable_orig[PRV_TGF_MAX_ENTRY_INDEX_CNS] = {0};

/* ethertypes in the ethertypes table */
static GT_U16   ethertypesTable[PRV_TGF_MAX_ENTRY_INDEX_CNS] =
{
     TGF_ETHERTYPE_NON_VALID_TAG_CNS/* not valid */  /*index 0 */
    ,ETHERTYPE_0x1234       /*index 1 */
    ,ETHERTYPE_0x5678       /*index 2 */
    ,ETHERTYPE_0xACBD       /*index 3 */
    ,ETHERTYPE_0xE1E1       /*index 4 */
    ,ETHERTYPE_0x8765       /*index 5 */
    ,ETHERTYPE_0x4321       /*index 6 */
    ,ETHERTYPE_0xABAB       /*index 7 */
};

static GT_CHAR* testNames[PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST____LAST____E]  = {
    "PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E"
    ,"PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_E"
    ,"PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E"
};
static GT_CHAR* testPurpose[PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST____LAST____E]  = {
    "check tpid 0,1 reassign from the eport table after the re-parse (tunnel termination) of passenger , override the default eport assignment"
    ,"check tpid 0,1 reassign after the re-parse (tunnel termination) of passenger , override the default ingress port assignment"
    ,"check tpid 0,1 manipulation without tunnel termination"
};

/* bmp for test 1..3 */
static GT_U32   tpidBmpTag0[PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST____LAST____E] = {
     /* bmp for test 1 */
     BIT_1 /*ETHERTYPE_0x1234*/ | BIT_3 /*ETHERTYPE_0xACBD*/ | BIT_4 /*ETHERTYPE_0xE1E1*/ | BIT_6 /*ETHERTYPE_0x4321*/ | BIT_7 /*ETHERTYPE_0xABAB*/
     /* bmp for test 2 */
    ,BIT_2 /*ETHERTYPE_0x5678*/ | BIT_3 /*ETHERTYPE_0xACBD*/ | BIT_4 /*ETHERTYPE_0xE1E1*/ | BIT_5 /*ETHERTYPE_0x8765*/ | BIT_7 /*ETHERTYPE_0xABAB*/
     /* bmp for test 2 */
    ,0xFF/* ALL */
};

/* bmp for test 1..3 */
static GT_U32   tpidBmpTag1[PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST____LAST____E] = {
     /* bmp for test 1 */
     BIT_2 /*ETHERTYPE_0x5678*/ | BIT_3 /*ETHERTYPE_0xACBD*/ | BIT_4 /*ETHERTYPE_0xE1E1*/ | BIT_5 /*ETHERTYPE_0x8765*/ | BIT_7 /*ETHERTYPE_0xABAB*/
     /* bmp for test 2 */
    ,BIT_1 /*ETHERTYPE_0x1234*/ | BIT_3 /*ETHERTYPE_0xACBD*/ | BIT_4 /*ETHERTYPE_0xE1E1*/ | BIT_6 /*ETHERTYPE_0x4321*/ | BIT_7 /*ETHERTYPE_0xABAB*/
     /* bmp for test 2 */
    ,0xFF/* ALL */
};


static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlan1TagPart_Passenger = {ETHERTYPE_0x8765,/* etherType */ 0, 0, 8 /* pri, cfi, VlanId */};
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlan2TagPart_Passenger = {ETHERTYPE_0x4321,/* etherType */ 0, 0, 9 /* pri, cfi, VlanId */};
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlan3TagPart_Passenger = {ETHERTYPE_0xABAB,/* etherType */ 0, 0, 0x333 /* pri, cfi, VlanId */};

static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart_Passenger = {0x6666};

/* payload of the passenger  */
static GT_U8 prvTgfPayloadDataArr_Passenger[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* payload part of the passenger */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart_Passenger =
{
    sizeof(prvTgfPayloadDataArr_Passenger),                       /* dataLength */
    prvTgfPayloadDataArr_Passenger                                /* dataPtr */
};

/* dummy CRC of the passenger */
static GT_U8 prvTgfPacketCrcData_Passenger[] =
{0x43, 0x96 , 0xf2 , 0xac};

/* struct for any kind of wildcard/'Unknown format' */
static TGF_PACKET_WILDCARD_STC  prvTgfPacketCrcPart_Passenger =
{
    sizeof(prvTgfPacketCrcData_Passenger),                       /* dataLength */
    prvTgfPacketCrcData_Passenger                                /* dataPtr */
};

/* dummy CRC for captured packets (that not do TT) */
static GT_U8 prvTgfPacketCrcData_nonTTCopy[] =
{0x33, 0x33 , 0x33 , 0x33};

/* struct for any kind of wildcard/'Unknown format' */
static TGF_PACKET_WILDCARD_STC  prvTgfPacketCrcPart_nonTTCopy =
{
    sizeof(prvTgfPacketCrcData_nonTTCopy),                       /* dataLength */
    prvTgfPacketCrcData_nonTTCopy                                /* dataPtr */
};

/* Ethernet over  of MPLS with 3 labels */
static TGF_PACKET_PART_STC prvTgfEthernetOverMpls3LabelsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_SKIP_E,       NULL},/* reserve place for 'Push tag 0' --- this also reserved for the copy to the CPU after tunnel termination .. the CPU get extra tag */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan1TagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan2TagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan3TagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan4TagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabel1Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabel2Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabel3Part},

    /* the passenger */
        {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part_Passenger},
        {TGF_PACKET_PART_SKIP_E,       NULL},/* reserve place for 'Push tag 0' */
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan1TagPart_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan2TagPart_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan3TagPart_Passenger},
        {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Passenger},
        {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart_Passenger},
        {TGF_PACKET_PART_WILDCARD_E,  &prvTgfPacketCrcPart_Passenger},

    /* dummy CRC for captured packets (that not do TT) */
    {TGF_PACKET_PART_SKIP_E,  &prvTgfPacketCrcPart_nonTTCopy}
};

/* Ethernet over  of MPLS with 3 labels */
static TGF_PACKET_STC prvTgfEthernetOverMpls3LabelsPacket =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,      /* totalLen */
    sizeof(prvTgfEthernetOverMpls3LabelsPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfEthernetOverMpls3LabelsPacketPartArray                                        /* partsArray */
};

/* mirror to CPU */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlan1TagPart_mirrorToCpu = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,/* etherType */ 0, 0, 0 /* pri, cfi, VlanId */};

/* vid0 for the TTI action -- for untagged tag0 passenger */
#define TTI_ACTION_SET_VID0_FOR_UNTAGGED_PASSENGER  0x777
/* vid1 for the TTI action -- for untagged tag1 passenger */
#define TTI_ACTION_SET_VID1_FOR_UNTAGGED_PASSENGER  0x222


/* dummy expected packet */
static TGF_PACKET_PART_STC dummyExpectedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_SKIP_E,       NULL},/* reserve place for 'Push tag 0' --- this also reserved for the copy to the CPU after tunnel termination .. the CPU get extra tag */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan1TagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan2TagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan3TagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan4TagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabel1Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabel2Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabel3Part},

    /* the passenger */
        {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part_Passenger},
        {TGF_PACKET_PART_SKIP_E,       NULL},/* reserve place for 'Push tag 0' */
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan1TagPart_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan2TagPart_Passenger},
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlan3TagPart_Passenger},
        {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Passenger},
        {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart_Passenger},
        {TGF_PACKET_PART_WILDCARD_E,  &prvTgfPacketCrcPart_Passenger},

    /* dummy CRC for captured packets (that not do TT) */
    {TGF_PACKET_PART_SKIP_E,  &prvTgfPacketCrcPart_nonTTCopy}
};


/* dummy expected packet */
static TGF_PACKET_STC dummyExpectedPacket =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,      /* totalLen */
    sizeof(dummyExpectedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    dummyExpectedPacketPartArray                                        /* partsArray */
};

#define PORT_INDEX_FOR_MIRROR_TO_CPU_COPY   0
/* number of ports in the test */
#define NUM_PORTS_IN_TEST           12

#define SECOND_HEMISPHERE_CNS   64
/* actual port number are the value % 'numOfPorts' :

    xcat,xcat2, Aldrin, AC3X: 28 ports :
    6 , 16, 14 , 0  ,
    4 , 18, 20 , 10 ,
    12, 22,  8 , 2

    lion: 60 ports
    6 , 16, 38 , 52
    4 , 18, 44 --> 42 , 2 ,
    40, 50,  8 , 54

    lion2: 124 ports
*/
static GT_U8    portsForTest[NUM_PORTS_IN_TEST] =
{
    6,     16,       34 +SECOND_HEMISPHERE_CNS,       48 + SECOND_HEMISPHERE_CNS ,
    4 ,    18,       40 +SECOND_HEMISPHERE_CNS,       58 + SECOND_HEMISPHERE_CNS ,
    40,    50,       8                        ,       50 + SECOND_HEMISPHERE_CNS
};

/* Aldrin Z0: ports 0..4, 16..19, 32..35, 56..59, 64..71, 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    portsForTest_aldrinZ0[NUM_PORTS_IN_TEST] =
    {0,         1,          2,          3,
     16,       32,         18,         34,
     17,       19,         33,         35};

static GT_U32    orig_portsForTest[NUM_PORTS_IN_TEST];
static GT_U8    orig_prvTgfPortsNum = 0;


#define NOT_EXIST   0xFF
#define NOT_MEMBER  0xFF
static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT egressTagStates_vlan5[NUM_PORTS_IN_TEST] =
{
     PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E /*outer tag 1*/
    ,PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*outer tag 0*/
    ,PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E /*push*/
    ,PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E /*outer tag 0 inner tag 1*/
    ,PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E /*outer tag 1 inner tag 0*/
    ,PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E /*pop (pop outer)*/
    ,PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E /*untagged*/
    ,NOT_MEMBER/*Not member*/

    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/

};

static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT egressTagStates_vlan8[NUM_PORTS_IN_TEST] =
{
     PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E /*outer tag 0 inner tag 1*/
    ,PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E /*outer tag 1 inner tag 0*/
    ,PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*outer tag 0*/
    ,PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E /*outer tag 1*/
    ,NOT_MEMBER/*Not member*/
    ,PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E /*untagged*/
    ,PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E /*pop (pop outer)*/
    ,PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E /*push*/

    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/
};

static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT egressTagStates_vlan9[NUM_PORTS_IN_TEST] =
{
     PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E /*pop (pop outer)*/
    ,PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E /*untagged*/
    ,NOT_MEMBER/*Not member*/
    ,PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E /*outer tag 1*/
    ,PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E /*outer tag 0 inner tag 1*/
    ,PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E /*push*/
    ,PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*outer tag 0*/
    ,PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E /*outer tag 1 inner tag 0*/

    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/
    ,NOT_MEMBER/*Not member*/
};

typedef struct {
    GT_U16  vlanId;
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT   *taggingArrPtr;
}EGRESS_TAG_STATES_INFO;

static EGRESS_TAG_STATES_INFO egressTagStateArr[PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST____LAST____E] =
{
     {9, egressTagStates_vlan9 }
    ,{8, egressTagStates_vlan8 }
    ,{5, egressTagStates_vlan5 }
};


/* tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           6
/* PCLID for the rule - for the MPLS packets */
#define PRV_TGF_TTI_MPLS_PCL_ID_CNS           0x35F
/* new src ePort for reassign by the TTi action */
#define NEW_SRC_EPORT_CNS       ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1999)
/* new profile for tag 0 , on the bew arc EPORT */
static GT_U32   newTag0TpidProfile = 0;
#define NEW_TAG0_TPID_PROFILE   newTag0TpidProfile
/* new profile for tag 1 , on the bew arc EPORT */
static GT_U32   newTag1TpidProfile = 0;
#define NEW_TAG1_TPID_PROFILE   newTag1TpidProfile


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* flag to show if system is initialized*/
static GT_BOOL  prvTgfIsInitialized = GT_FALSE;

/* stored default egress etherTypes */
static struct
{
    GT_U16      etherType0;
    GT_U16      etherType1;
} prvTgfDefEgrEtherTypesStc;

/* stored default TPIDs */
static struct
{
    GT_U32      ingTag0TpidBmp;
    GT_U32      ingTag1TpidBmp;
    GT_U32      egrTag0TpidIndex;
    GT_U32      egrTag1TpidIndex;
} prvTgfDefTpidBmpStc[NUM_PORTS_IN_TEST];



/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal bridgeInit function
* @endinternal
*
* @brief   init bridge relate to the test.
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static GT_VOID bridgeInit
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
)
{
#ifdef CHX_FAMILY
    GT_U32                      ii;
#endif /*CHX_FAMILY*/
    GT_U16                      vlanId;
    GT_U32                      portIter  = 0;
    GT_U32                      portsCount;
    GT_U32                      ethIter   = 0;
    GT_STATUS                   rc;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32  egressTagStateIndex;/* index into egressTagStateArr[testId].taggingArrPtr[] */
    GT_U16  ethertype[2];
    GT_U32  tpidBmpArr[2];
    /* is the device supports push in the vlan entry command */
    GT_BOOL supportPush = prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS);

    vlanId = egressTagStateArr[testId].vlanId;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set vlan entry */
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
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    portsCount = prvTgfPortsNum;
    for (portIter = 0,egressTagStateIndex = 0; portIter < portsCount; portIter++)
    {
        if(portIter == senderPortIndex)
        {
            /* dummy setting since this port is the sender port */
            portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] =
                PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
        }
        else
        {
            if(GT_FALSE == supportPush &&
               egressTagStateArr[testId].taggingArrPtr[egressTagStateIndex] ==
                    PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E)
            {
                /* use valid value */
                egressTagStateArr[testId].taggingArrPtr[egressTagStateIndex] =
                    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
            }

            if(egressTagStateArr[testId].taggingArrPtr[egressTagStateIndex] != NOT_MEMBER)
            {
                portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] =
                    egressTagStateArr[testId].taggingArrPtr[egressTagStateIndex];

                egressTagStateIndex++;
            }
            else
            {
                egressTagStateIndex++;

                /* the port is not member */
                continue;
            }

        }

        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);
    }


    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);


    /* Configure ingress TPID table */
    for (ethIter = 0; ethIter < PRV_TGF_MAX_ENTRY_INDEX_CNS; ethIter++)
    {
        if (!prvTgfIsInitialized)
        {
            /* save default TPID table entry */
            rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,
                                           ethIter,
                                           &(ethertypesTable_orig[ethIter]));
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d, %d, %d",
                                         prvTgfDevNum, CPSS_DIRECTION_INGRESS_E,
                                         ethIter);
        }

        /* set TPID table entry */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,
                                       ethIter,
                                       ethertypesTable[ethIter]);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_INGRESS_E,
                                     ethIter, ethertypesTable[ethIter]);
    }

    if (!prvTgfIsInitialized)
    {
        /* save ingress TPID select for TAG0 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,
                                                 prvTgfPortsArray[portIter],
                                                 CPSS_VLAN_ETHERTYPE0_E,
                                                 &(prvTgfDefTpidBmpStc[portIter].ingTag0TpidBmp));
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE0_E);

            /* save ingress TPID select for TAG1 */
            rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,
                                                 prvTgfPortsArray[portIter],
                                                 CPSS_VLAN_ETHERTYPE1_E,
                                                 &(prvTgfDefTpidBmpStc[portIter].ingTag1TpidBmp));
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE1_E);
        }
    }

    if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E)
    {
        tpidBmpArr[0] = tpidBmpTag0[testId] & 0xF;/* only the first 4 bits */
        tpidBmpArr[1] = tpidBmpTag1[testId] & 0xF;/* only the first 4 bits */

#ifdef CHX_FAMILY
        for(ii = 2 ; ii < 6; ii++)
        {
            rc = sip5Emulate_IngressTpidProfileClear(CPSS_VLAN_ETHERTYPE0_E,ii);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "sip5Emulate_IngressTpidProfileClear: %d, %d, %d",
                                         prvTgfDevNum, CPSS_VLAN_ETHERTYPE0_E ,ii);
            rc = sip5Emulate_IngressTpidProfileClear(CPSS_VLAN_ETHERTYPE1_E,ii);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "sip5Emulate_IngressTpidProfileClear: %d, %d, %d",
                                         prvTgfDevNum, CPSS_VLAN_ETHERTYPE1_E , ii);
        }


        rc = sip5Emulate_IngressTpidProfileGet(CPSS_VLAN_ETHERTYPE0_E, tpidBmpTag0[testId] & 0xf0 , &NEW_TAG0_TPID_PROFILE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "sip5Emulate_IngressTpidProfileGet: %d, %d, %d",
                                     prvTgfDevNum, CPSS_VLAN_ETHERTYPE0_E , tpidBmpTag0[testId] & 0xf0);

        rc = sip5Emulate_IngressTpidProfileGet(CPSS_VLAN_ETHERTYPE1_E, tpidBmpTag1[testId] & 0xf0 , &NEW_TAG1_TPID_PROFILE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "sip5Emulate_IngressTpidProfileGet: %d, %d, %d",
                                     prvTgfDevNum, CPSS_VLAN_ETHERTYPE1_E , tpidBmpTag1[testId] & 0xf0);
#endif /*CHX_FAMILY*/

        /* enable NA to CPU on the new src eport */
        rc = prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum,NEW_SRC_EPORT_CNS,GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbNaToCpuPerPortSet: %d, %d, %d",
                                     prvTgfDevNum, NEW_SRC_EPORT_CNS, GT_TRUE);

        rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,
            NEW_SRC_EPORT_CNS,
            CPSS_VLAN_ETHERTYPE0_E,
            GT_FALSE,
            NEW_TAG0_TPID_PROFILE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d, %d, %d, %d",
                                     prvTgfDevNum, NEW_SRC_EPORT_CNS,
                                     CPSS_VLAN_ETHERTYPE0_E, NEW_TAG0_TPID_PROFILE);

        rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,
            NEW_SRC_EPORT_CNS,
            CPSS_VLAN_ETHERTYPE1_E,
            GT_FALSE,
            NEW_TAG1_TPID_PROFILE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d, %d, %d, %d",
                                     prvTgfDevNum, NEW_SRC_EPORT_CNS,
                                     CPSS_VLAN_ETHERTYPE1_E, NEW_TAG1_TPID_PROFILE);


        rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,
                NEW_TAG0_TPID_PROFILE,
                CPSS_VLAN_ETHERTYPE0_E,
                tpidBmpTag0[testId] & 0xf0);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d, %d, %d, %x",
                                     prvTgfDevNum, NEW_TAG0_TPID_PROFILE,
                                     CPSS_VLAN_ETHERTYPE0_E, (tpidBmpTag0[testId] & 0xf0));

        rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,
                NEW_TAG1_TPID_PROFILE,
                CPSS_VLAN_ETHERTYPE1_E,
                tpidBmpTag1[testId] & 0xf0);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d, %d, %d, %x",
                                     prvTgfDevNum, NEW_TAG0_TPID_PROFILE,
                                     CPSS_VLAN_ETHERTYPE1_E, (tpidBmpTag1[testId] & 0xf0));
    }
    else
    {
        tpidBmpArr[0] = tpidBmpTag0[testId];
        tpidBmpArr[1] = tpidBmpTag1[testId];
    }

    /* Set ingress TPID select for TAG0 */
    rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[senderPortIndex],
                                         CPSS_VLAN_ETHERTYPE0_E,
                                         tpidBmpArr[0]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d, %x",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex],
                                 CPSS_VLAN_ETHERTYPE0_E, tpidBmpArr[0]);

    /* Set ingress TPID select for TAG1 */
    rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[senderPortIndex],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         tpidBmpArr[1]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d, %x",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex],
                                 CPSS_VLAN_ETHERTYPE1_E, tpidBmpArr[1]);

    if (!prvTgfIsInitialized)
    {
        /* save egress TPID table */
        rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_EGRESS_E,
                                       0,
                                       &(prvTgfDefEgrEtherTypesStc.etherType0));
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E, 0);

        rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_EGRESS_E,
                                       1,
                                       &(prvTgfDefEgrEtherTypesStc.etherType1));
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E, 1);
    }

    ethertype[0] = (GT_U16)(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E ? ethertypesTable[6] :
                            testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_E ? ethertypesTable[5] :
                            ethertypesTable[1]) ;
    ethertype[1] = (GT_U16)(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E ? ethertypesTable[5] :
                            testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_E ? ethertypesTable[6] :
                            ethertypesTable[2]) ;


    /*  set egress TPID table to: Ethertype0 = ethertype[0], Ethertype1 = ethertype[1] */
    /*  (by default Ethertype2-7 remain 0x8100). */
    for (ethIter = 0; ethIter < 2; ethIter++)
    {
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                       ethIter,
                                       ethertype[ethIter]);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                     ethIter, ethertype[ethIter]);
    }

    if (!prvTgfIsInitialized)
    {
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* save egress TPID select for TAG0 */
            rc = prvTgfBrgVlanPortEgressTpidGet(prvTgfDevNum,
                                                prvTgfPortsArray[portIter],
                                                CPSS_VLAN_ETHERTYPE0_E,
                                                &(prvTgfDefTpidBmpStc[portIter].egrTag0TpidIndex));
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidGet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE0_E);

            /* save egress TPID select for TAG1 */
            rc = prvTgfBrgVlanPortEgressTpidGet(prvTgfDevNum,
                                                prvTgfPortsArray[portIter],
                                                CPSS_VLAN_ETHERTYPE1_E,
                                                &(prvTgfDefTpidBmpStc[portIter].egrTag1TpidIndex));
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidGet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE1_E);
        }
    }

    /* set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1. */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            CPSS_VLAN_ETHERTYPE0_E,
                                            0);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     CPSS_VLAN_ETHERTYPE0_E, 0);

        rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            CPSS_VLAN_ETHERTYPE1_E,
                                            1);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     CPSS_VLAN_ETHERTYPE1_E, 1);

        if(portIter != senderPortIndex)
        {
            /* set ingress TPID select for TAG0 -- not recognize tag 0 */
            rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                                 prvTgfPortsArray[portIter],
                                                 CPSS_VLAN_ETHERTYPE0_E,
                                                 0);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE0_E);

            /* set ingress TPID select for TAG0 -- not recognize tag 1 */
            rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                                 prvTgfPortsArray[portIter],
                                                 CPSS_VLAN_ETHERTYPE1_E,
                                                 0);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE1_E);
        }

    }

    /* system initialized successfully */
    prvTgfIsInitialized = GT_TRUE;
}

/**
* @internal tunnelInit function
* @endinternal
*
* @brief   init tunnel termination relate to the test.
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static GT_VOID tunnelInit
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT        testId ,
    IN GT_U32       senderPortIndex
)
{
    GT_STATUS   rc      ;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_2_STC      ruleAction;

    if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E)
    {
        /* we not need tunnel termination for this test */
        return;
    }

    rc = prvTgfTtiPclIdGet(prvTgfDevNum,PRV_TGF_TTI_KEY_MPLS_E,&origMplsTtiPclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_MPLS_E,PRV_TGF_TTI_MPLS_PCL_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_MPLS_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* clear entry */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&ruleAction,0, sizeof(ruleAction));


    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_TRUE;
    ruleAction.ttPassengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E;
    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.egressInterface.type = CPSS_INTERFACE_VID_E;
    ruleAction.egressInterface.vlanId = egressTagStateArr[testId].vlanId;
    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;

    /*ruleAction.bridgeBypass = GT_TRUE;*/
    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ruleAction.tag0VlanId = TTI_ACTION_SET_VID0_FOR_UNTAGGED_PASSENGER;/*0x777*/
    ruleAction.tag1VlanCmd = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
    ruleAction.tag1VlanId = TTI_ACTION_SET_VID1_FOR_UNTAGGED_PASSENGER;/*0x222*/

    if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E)
    {
        ruleAction.sourceEPortAssignmentEnable = GT_TRUE;
        ruleAction.sourceEPort = NEW_SRC_EPORT_CNS;
    }

    /* configure TTI rule */
    pattern.mpls.common.pclId    = PRV_TGF_TTI_MPLS_PCL_ID_CNS;
    pattern.mpls.common.srcIsTrunk = GT_FALSE;
    pattern.mpls.common.srcPortTrunk = prvTgfPortsArray[senderPortIndex];

    mask.mpls.common.pclId    = BIT_10 - 1;
    mask.mpls.common.srcIsTrunk = 1;
    mask.mpls.common.srcPortTrunk = BIT_7 - 1;

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_MPLS_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

}

/**
* @internal captureEgressTrafficOnPort function
* @endinternal
*
* @brief   start/stop capturing on specific port of the test .
*
* @param[in] testId                   - test ID
* @param[in] start                    - start/stop
*                                      GT_TRUE - start
*                                      GT_FALSE - stop
* @param[in] senderPortIndex          - index of the sending port
* @param[in] capturedPortIndex        - the captured port index
*                                       None
*/
static void captureEgressTrafficOnPort
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId,
    IN GT_BOOL      start,
    IN GT_U32       senderPortIndex,
    IN GT_U32       capturedPortIndex
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_PORT_NUM      port;
    GT_BOOL     captureMirrorCopyAndRegularCapture = GT_FALSE;
    GT_U32      numExpectedCapturedPackets = prvTgfBurstCount;

    if(capturedPortIndex == PORT_INDEX_FOR_MIRROR_TO_CPU_COPY)
    {
        /* NOTE: this must not be the sender index */
        if(senderPortIndex == capturedPortIndex)
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(senderPortIndex, capturedPortIndex,
                "captureEgressTrafficOnPort: NOTE: the sender index[%d]  must not be the captured index[%d]\n",
                                         senderPortIndex, capturedPortIndex);
            /* ERROR */
            return;
        }

        captureMirrorCopyAndRegularCapture = GT_TRUE;
        numExpectedCapturedPackets *= 2;/* 1 for the regular capture and 1 for the mirror copy */
    }

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[capturedPortIndex];

    if(captureMirrorCopyAndRegularCapture == GT_TRUE)
    {
        if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E)
        {
            /* need to set eport mirroring on the new src eport */
            port = NEW_SRC_EPORT_CNS;
        }
        else
        {
            port = prvTgfPortsArray[senderPortIndex];
        }

        /*the analyzer anyway will be set to 'CPU port' by the capture*/

        /* Enable/Disable Rx mirroring on ingress port. */
        rc = prvTgfMirrorRxPortSet(port, start, 0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                     port, start);
    }

    if(start ==GT_FALSE)
    {
        /* wait for packets come to CPU */
        (void) tgfTrafficGeneratorRxInCpuNumWait(numExpectedCapturedPackets ,
            (GT_TRUE == prvUtfIsGmCompilation() ? 1500 : 500),
            NULL);
    }

/*    portInterface.devPort.portNum = 59; -- AmitK HW debug only */

    /* Start/Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, start);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, start);


}


/**
* @internal buildExpectedTaggingFormat function
* @endinternal
*
* @brief   build expected tagging for the specific egress port in the specific test
*
* @param[in] testId                   - test ID
* @param[in] egressTagStateIndex      - index in egressTagStateArr[testId].taggingArrPtr[]
* @param[in] outerTagIndex            - index in dummyExpectedPacketPartArray of the outer tag
*
* @param[out] outerTagIndexPtr         - (pointer to) the index in dummyExpectedPacketPartArray
*                                      of the part that should be outer tag
*                                      value NOT_EXIST means there is no outer tag
* @param[out] innerTagIndexPtr         - (pointer to) the index in dummyExpectedPacketPartArray
*                                      of the part that should be inner tag
*                                      value NOT_EXIST means there is no inner tag
*                                       None
*/
static void buildExpectedTaggingFormat
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId,
    IN GT_U32       egressTagStateIndex,
    IN GT_U32       outerTagIndex,
    OUT GT_U32      *outerTagIndexPtr,
    OUT GT_U32      *innerTagIndexPtr
)
{
    GT_U32  ii;
    TGF_PACKET_VLAN_TAG_STC *currentVlanTagPartPtr;
    GT_BOOL     outerTagFoundAsTag0 = GT_FALSE;
    GT_BOOL     outerTagFoundAsTag1 = GT_FALSE;
    GT_BOOL     innerTagFoundAsTag0 = GT_FALSE;
    GT_BOOL     innerTagFoundAsTag1 = GT_FALSE;

    currentVlanTagPartPtr = dummyExpectedPacketPartArray[outerTagIndex].partPtr;

    /*******************************************************/
    /* look for the current outer tag as tag 0 recognition */
    /*******************************************************/
    for(ii = 0 ; ii < 8/*tpid BMP*/; ii++)
    {
        /*check tag 0 */
        if(tpidBmpTag0[testId] & (1 << ii))
        {
            if(currentVlanTagPartPtr->etherType == ethertypesTable[ii])
            {
                /* found match as tag 0 */
                outerTagFoundAsTag0 = GT_TRUE;
                break;
            }
        }
    }

    if(outerTagFoundAsTag0 == GT_FALSE)
    {
        /*******************************************************/
        /* look for the current outer tag as tag 1 recognition */
        /*******************************************************/

        for(ii = 0 ; ii < 8/*tpid BMP*/; ii++)
        {
            /*check tag 1 */
            if(tpidBmpTag1[testId] & (1 << ii))
            {
                if(currentVlanTagPartPtr->etherType == ethertypesTable[ii])
                {
                    /* found match as tag 1 */
                    outerTagFoundAsTag1 = GT_TRUE;
                    break;
                }
            }
        }
    }

    currentVlanTagPartPtr = dummyExpectedPacketPartArray[outerTagIndex + 1].partPtr;


    if(outerTagFoundAsTag0 == GT_FALSE)
    {
        /*******************************************************/
        /* look for the current inner tag as tag 0 recognition */
        /*******************************************************/
        for(ii = 0 ; ii < 8/*tpid BMP*/; ii++)
        {
            /*check tag 0 */
            if(tpidBmpTag0[testId] & (1 << ii))
            {
                if(currentVlanTagPartPtr->etherType == ethertypesTable[ii])
                {
                    /* found match as tag 0 */
                    innerTagFoundAsTag0 = GT_TRUE;
                    break;
                }
            }
        }
    }

    if(outerTagFoundAsTag1 == GT_FALSE &&
       innerTagFoundAsTag0 == GT_FALSE)
    {
        /*******************************************************/
        /* look for the current inner tag as tag 1 recognition */
        /*******************************************************/

        for(ii = 0 ; ii < 8/*tpid BMP*/; ii++)
        {
            /*check tag 1 */
            if(tpidBmpTag1[testId] & (1 << ii))
            {
                if(currentVlanTagPartPtr->etherType == ethertypesTable[ii])
                {
                    /* found match as tag 1 */
                    innerTagFoundAsTag1 = GT_TRUE;
                    break;
                }
            }
        }
    }


    *outerTagIndexPtr = NOT_EXIST;
    *innerTagIndexPtr = NOT_EXIST;
    dummyExpectedPacketPartArray[outerTagIndex - 1].type = TGF_PACKET_PART_SKIP_E;

    if(innerTagFoundAsTag0 == GT_FALSE && outerTagFoundAsTag0 == GT_FALSE)
    {
        /* not expected ! */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,"buildExpectedTaggingFormat: not expected : innerTagFoundAsTag0 == GT_FALSE && outerTagFoundAsTag0 == GT_FALSE");
        return;
    }

    if(innerTagFoundAsTag1 == GT_FALSE && outerTagFoundAsTag1 == GT_FALSE)
    {
        /* not expected ! */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,"buildExpectedTaggingFormat: not expected : innerTagFoundAsTag1 == GT_FALSE && outerTagFoundAsTag1 == GT_FALSE");
        return;
    }

    switch(egressTagStateArr[testId].taggingArrPtr[egressTagStateIndex])
    {
        case PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E:
            utfGeneralStateMessageSave(7,"expected tagging mode -- untagged ");
            /* no more to do */
            break;
        case PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E:
            /* tag 0 need to be outer */
            if(innerTagFoundAsTag0 == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- tag 0 : select the inner from ingress packet ");
                utfGeneralStateMessageSave(7,"expected tagging mode -- tag 0 : select the inner from ingress packet ");
                *outerTagIndexPtr = outerTagIndex + 1;
            }
            else
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- tag 0 : select the outer from ingress packet ");
                utfGeneralStateMessageSave(7,"expected tagging mode -- tag 0 : select the outer from ingress packet ");
                *outerTagIndexPtr = outerTagIndex;
            }
            break;
        case PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E:
            /* tag 1 need to be outer */
            if(innerTagFoundAsTag1 == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- tag 1 : select the inner from ingress packet ");
                utfGeneralStateMessageSave(7,"expected tagging mode -- tag 1 : select the inner from ingress packet ");
                *outerTagIndexPtr = outerTagIndex + 1;
            }
            else
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- tag 1 : select the outer from ingress packet ");
                utfGeneralStateMessageSave(7,"expected tagging mode -- tag 1 : select the outer from ingress packet ");
                *outerTagIndexPtr = outerTagIndex;
            }
            break;
        case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
            if(innerTagFoundAsTag0 == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- tag 0 outer tag 1 inner : tag 0 - select the inner from ingress packet , tag 1 : select outer");
                utfGeneralStateMessageSave(7,"expected tagging mode -- tag 0 outer tag 1 inner : tag 0 - select the inner from ingress packet , tag 1 : select outer");
                *innerTagIndexPtr = outerTagIndex;
                *outerTagIndexPtr = outerTagIndex + 1;
            }
            else
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- tag 0 outer tag 1 inner : tag 0 - select the outer from ingress packet , tag 1 : select inner");
                utfGeneralStateMessageSave(7,"expected tagging mode -- tag 0 outer tag 1 inner : tag 0 - select the outer from ingress packet , tag 1 : select inner");
                *outerTagIndexPtr = outerTagIndex;
                *innerTagIndexPtr = outerTagIndex + 1;
            }
            break;
        case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
            if(outerTagFoundAsTag1 == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- tag 1 outer tag 0 inner : tag 0 - select the inner from ingress packet , tag 1 : select outer");
                utfGeneralStateMessageSave(7,"expected tagging mode -- tag 1 outer tag 0 inner : tag 0 - select the inner from ingress packet , tag 1 : select outer");
                *outerTagIndexPtr = outerTagIndex;
                *innerTagIndexPtr = outerTagIndex + 1;
            }
            else
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- tag 1 outer tag 0 inner : tag 0 - select the inner from ingress packet , tag 1 : select outer");
                utfGeneralStateMessageSave(7,"expected tagging mode -- tag 1 outer tag 0 inner : tag 0 - select the inner from ingress packet , tag 1 : select outer");
                *innerTagIndexPtr = outerTagIndex;
                *outerTagIndexPtr = outerTagIndex + 1;
            }
            break;
        case PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E:
            /* modify the reserved place to be the same as tag0 */
            dummyExpectedPacketPartArray[outerTagIndex - 1].type = TGF_PACKET_PART_VLAN_TAG_E;
            if(innerTagFoundAsTag0 == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- push tag 0 : tag 0 - select the inner from ingress packet");
                utfGeneralStateMessageSave(7,"expected tagging mode -- push tag 0 : tag 0 - select the inner from ingress packet");
                dummyExpectedPacketPartArray[outerTagIndex - 1].partPtr =
                    dummyExpectedPacketPartArray[outerTagIndex + 1].partPtr;
            }
            else
            {
                PRV_UTF_LOG0_MAC("expected tagging mode -- push tag 0 : tag 0 - select the outer from ingress packet");
                utfGeneralStateMessageSave(7,"expected tagging mode -- push tag 0 : tag 0 - select the outer from ingress packet");
                dummyExpectedPacketPartArray[outerTagIndex - 1].partPtr =
                    dummyExpectedPacketPartArray[outerTagIndex].partPtr;
            }

            /* keep as is */
            *outerTagIndexPtr = outerTagIndex;
            *innerTagIndexPtr = outerTagIndex + 1;
            break;
        case PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E:
            /* pop the outer */
            PRV_UTF_LOG0_MAC("expected tagging mode -- pop outer : the outer from ingress packet");
            utfGeneralStateMessageSave(7,"expected tagging mode -- pop outer : the outer from ingress packet");
            *outerTagIndexPtr = outerTagIndex + 1;
            break;
        case PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E:
            /* keep as is */
            PRV_UTF_LOG0_MAC("expected tagging mode -- DO NOT MODIFY (as in ingress packet)");
            utfGeneralStateMessageSave(7,"expected tagging mode -- DO NOT MODIFY (as in ingress packet)");
            *outerTagIndexPtr = outerTagIndex;
            *innerTagIndexPtr = outerTagIndex + 1;
            break;

        default:
            return ;
    }


}

/**
* @internal checkCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test (can also be for the sending port).
*
* @param[in] testId                   - test ID
* @param[in] senderPortIndex          - index of the sending port
* @param[in] capturedPortIndex        - the captured port index to check
* @param[in] egressTagStateIndex      - index into egressTagStateArr[testId].taggingArrPtr[]
*                                      the expected tagging of the captured packet
*                                      relevant when senderPortIndex != capturedPortIndex
*                                       None
*/
static void checkCaptureEgressTrafficOnPort
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId,
    IN GT_U32       senderPortIndex,
    IN GT_U32       capturedPortIndex,
    IN GT_U32       egressTagStateIndex
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC *packetInfoPtr;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_U32  ii;
    GT_U32  outerTagIndex;/* the index in dummyExpectedPacketPartArray
                               of the part that should be outer tag
                               value NOT_EXIST means there is no outer tag*/
    GT_U32  innerTagIndex;/*the index in dummyExpectedPacketPartArray
                               of the part that should be inner tag
                               value NOT_EXIST means there is no inner tag*/
    GT_BOOL ttCopyToCpuWithAdditionalTag = GT_FALSE;/* packet that is TT (tunnel terminated) and will be send to CPU by ingress pipe,
                                   will be send with outer tag on top of the info in the DSA tag (similar to twice the same tag)
                                                     The errata is relevant for : ch2,3,xcat,2,lion,2,3,bobcat2. */
    GT_BOOL forceNewDsaToCpu = GT_FALSE;/* when 'ForceNewDsaToCpu' the extra tag in addition to the DSA exists anyway ,
                   but the DSA tag info will state 'not tagged' so the CPSS will
                   remove the extra tag ! , and we will get the actual original packet
                   with out any additional tags
                   The flag is relevant for : xcat2,lion,2,3,bobcat2. */

#ifdef CHECK_SIZE_MISMATCH
    if(sizeof(prvTgfEthernetOverMpls3LabelsPacketPartArray) != sizeof(dummyExpectedPacketPartArray))
    {
        rc = GT_BAD_STATE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"checkCaptureEgressTrafficOnPort: mismatch packets size");
        return;
    }
#endif /*CHECK_SIZE_MISMATCH*/

    if(GT_FALSE ==
       prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_LION_E | UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* the listed devices are supporting this behavior */
        ttCopyToCpuWithAdditionalTag = GT_TRUE;


        if(GT_FALSE ==
           prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT2_E | UTF_LION2_E | UTF_LION_E | UTF_CPSS_PP_E_ARCH_CNS))
        {
            /* the listed devices supports this API */
#ifdef CHX_FAMILY
            rc = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(prvTgfDevNum,&forceNewDsaToCpu);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChBrgVlanForceNewDsaToCpuEnableGet: failed \n");
#endif /*CHX_FAMILY*/

            if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum) ||
                /* !!! the capture is set to not recognize TPIDs on ingress !!!*/
                forceNewDsaToCpu == GT_TRUE)
            {
                /* when 'ForceNewDsaToCpu' the extra tag in addition to the DSA exists anyway ,
                   but the DSA tag info will state 'not tagged' so the CPSS will
                   remove the extra tag ! , and we will get the actual original packet
                   with out any additional tags*/
                ttCopyToCpuWithAdditionalTag = GT_FALSE;
            }
        }
    }


    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;

    if(capturedPortIndex == senderPortIndex)
    {
        /* the ingress port has ingress mirroring , so packet should come as
           sent */
        /* copy the content from the 'Original packet' before we modify the dummy */
        cpssOsMemCpy(dummyExpectedPacketPartArray,
            prvTgfEthernetOverMpls3LabelsPacketPartArray,
            sizeof(dummyExpectedPacketPartArray));

        packetInfoPtr = &dummyExpectedPacket;

        for(ii = 0 ; ii < packetInfoPtr->numOfParts; ii++)
        {
            if(packetInfoPtr->partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
            {
                /* reached the first outer tag */
                break;
            }
        }

        if(ii == packetInfoPtr->numOfParts)
        {
            /* not found the first tag ?! */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, 1,"checkCaptureEgressTrafficOnPort: not found first tag ?!");
            return;
        }

        if(ttCopyToCpuWithAdditionalTag &&
           (testId != PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E))/*those tests do tunnel termination */
        {
            /* this also reserved for the copy to the CPU after tunnel termination .. the CPU get extra tag */

            /* modify the reserved place to be the same as most outer tag */
            PRV_UTF_LOG0_MAC("expected tagging mode -- tunnel termination copy to CPU erratum : push extra tag");
            utfGeneralStateMessageSave(7,"expected tagging mode -- tunnel termination copy to CPU erratum : push extra tag");

            packetInfoPtr->partsArray[ii-1].type = TGF_PACKET_PART_VLAN_TAG_E;
            packetInfoPtr->partsArray[ii-1].partPtr = &prvTgfPacketVlan1TagPart_mirrorToCpu;
        }
        else
        {
            packetInfoPtr->partsArray[ii].partPtr = &prvTgfPacketVlan1TagPart_mirrorToCpu;
            PRV_UTF_LOG0_MAC("expected tagging mode -- AS ingress ");
            utfGeneralStateMessageSave(7,"expected tagging mode -- AS ingress ");
        }

        prvTgfPacketVlan1TagPart_mirrorToCpu = prvTgfPacketVlan1TagPart;

        if((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum) &&
               (testId != PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E)) ||
               (forceNewDsaToCpu == GT_TRUE))
        {
            /* the CPSS will get DSA info that state 'not tagged' so it will not
                need to 'restore' dummy 0x8100 ethertype */
            prvTgfPacketVlan1TagPart_mirrorToCpu.etherType = prvTgfPacketVlan1TagPart.etherType;
        }
        else
        {
            /* the CPSS will not know to restore the original ethertype --> so expect 0x8100 */
            prvTgfPacketVlan1TagPart_mirrorToCpu.etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
        }

        /* set the last part as CRC section of the mirrored packet */
        packetInfoPtr->partsArray[packetInfoPtr->numOfParts - 1].type = TGF_PACKET_PART_WILDCARD_E;

        utfGeneralStateMessageSave(4,"ingress mirror to CPU");
        utfGeneralStateMessageSave(5,"");
    }
    else
    if(egressTagStateArr[testId].taggingArrPtr[egressTagStateIndex] == NOT_MEMBER)
    {
        /* not checking this port , supposed to not be in the vlan */
        return;
    }
    else /* one of the egress ports that is member in vlan */
    {
        /* copy the content from the 'Original packet' before we modify the dummy */
        cpssOsMemCpy(dummyExpectedPacketPartArray,
            prvTgfEthernetOverMpls3LabelsPacketPartArray,
            sizeof(dummyExpectedPacketPartArray));

        packetInfoPtr = &dummyExpectedPacket;

        if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E)
        {
            /* no tunnel termination */

            /* set the last part as CRC section of the mirrored packet */
            packetInfoPtr->partsArray[packetInfoPtr->numOfParts - 1].type = TGF_PACKET_PART_WILDCARD_E;
        }
        else
        {
            /* tunnel termination --> passenger */

            packetInfoPtr->partsArray[0].type = TGF_PACKET_PART_SKIP_E;/* was TGF_PACKET_PART_L2_E */

            for(ii = 1/* skip first TGF_PACKET_PART_L2_E*/ ; ii < packetInfoPtr->numOfParts; ii++)
            {
                if(packetInfoPtr->partsArray[ii].type == TGF_PACKET_PART_L2_E)
                {
                    /* reached the passenger */
                    break;
                }

                /* modify the tunnel part to be skipped */
                packetInfoPtr->partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
            }

            if(ii == packetInfoPtr->numOfParts)
            {
                /* not found the start of passenger ?! */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, 1,"checkCaptureEgressTrafficOnPort: not found the start of passenger ?!");
                return;
            }
        }

        /* look for the first tag */
        for(ii = 0; ii < packetInfoPtr->numOfParts; ii++)
        {
            if(packetInfoPtr->partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
            {
                /* reached the first tag  */
                break;
            }
        }

        if(ii == packetInfoPtr->numOfParts)
        {
            /* not found first tag ?! */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, 1,"checkCaptureEgressTrafficOnPort: not found the first tag ?!");
            return;
        }

         buildExpectedTaggingFormat(testId,
            egressTagStateIndex,/* index in egressTagStateArr[testId].taggingArrPtr[] */
            ii,/* index in dummyExpectedPacketPartArray of the outer tag */
            &outerTagIndex,
            &innerTagIndex);

        utfGeneralStateMessageSave(4,"outerTagIndex[%x]",outerTagIndex);
        utfGeneralStateMessageSave(5,"innerTagIndex[%x]",innerTagIndex);

        if(outerTagIndex == NOT_EXIST)
        {
            dummyExpectedPacketPartArray[ii].type = TGF_PACKET_PART_SKIP_E;
        }
        else
        {
            dummyExpectedPacketPartArray[ii] =
                prvTgfEthernetOverMpls3LabelsPacketPartArray[outerTagIndex];
        }

        if(innerTagIndex == NOT_EXIST)
        {
            dummyExpectedPacketPartArray[ii + 1].type = TGF_PACKET_PART_SKIP_E;
        }
        else
        {
            dummyExpectedPacketPartArray[ii + 1] =
                prvTgfEthernetOverMpls3LabelsPacketPartArray[innerTagIndex];
        }
    }/* one of the egress ports that is member in vlan */

    portInterface.devPort.portNum = prvTgfPortsArray[capturedPortIndex];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            packetInfoPtr,
            prvTgfBurstCount,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);
}


/**
* @internal testPacketsSendAndCheck function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] testId                   - test ID
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static GT_VOID testPacketsSendAndCheck
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT                   testId,
    IN GT_U32                   senderPortIndex
)
{
    GT_STATUS rc          ;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    ;
    TGF_PACKET_STC *packetInfoPtr = &prvTgfEthernetOverMpls3LabelsPacket;
    GT_U32  egressTagStateIndex;/* index into egressTagStateArr[testId].taggingArrPtr[] */
    PRV_TGF_MAC_ENTRY_KEY_STC    macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC    macEntry;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    PRV_UTF_LOG0_MAC("======= reset mac counters on all tested ports. =======\n");
    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    PRV_UTF_LOG0_MAC("======= clear captured packets table in the CPU. =======\n");
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr , prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    for (portIter = 0,egressTagStateIndex = 0; portIter < portsCount; portIter++,egressTagStateIndex++)
    {
        utfGeneralStateMessageSave(3,"checked Port[%d]",prvTgfPortsArray[portIter]);
        utfGeneralStateMessageSave(6,"checked Port index[%d]",portIter);
        if(portIter == senderPortIndex)
        {
            /* contrast the increment of the index in case of the sender */
            egressTagStateIndex--;

            /* the check of the mirror to cpu copy done in below code of :
                if(portIter == PORT_INDEX_FOR_MIRROR_TO_CPU_COPY)
                {
                    ...
                }
            */
            continue;
        }
        else
        {
            if(egressTagStateArr[testId].taggingArrPtr[egressTagStateIndex] == NOT_MEMBER)
            {
                /* the port is not in Vlan .. don't check it */
                continue;
            }
        }

        /* start capturing on port */
        PRV_UTF_LOG1_MAC("======= start capturing on port [%d]of the test =======\n",prvTgfPortsArray[portIter]);
        captureEgressTrafficOnPort(testId,GT_TRUE,senderPortIndex,portIter);

        utfPrintKeepAlive();
        /* -------------------------------------------------------------------------
         * 1. Sending ethernet-over-MPLS
         */
        PRV_UTF_LOG1_MAC("======= Sending Ethernet-over-MPLS to ingress port [%d]=======\n",prvTgfPortsArray[senderPortIndex]);
        /*  send the packet to ingress port */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

        PRV_UTF_LOG1_MAC("======= stop capturing on port [%d]of the test =======\n",prvTgfPortsArray[portIter]);
        /* stop capturing on port */
        captureEgressTrafficOnPort(testId,GT_FALSE,senderPortIndex,portIter);

        PRV_UTF_LOG1_MAC("======= check captured egress traffic on port [%d]of the test =======\n",prvTgfPortsArray[portIter]);
        /*check captured egress traffic on all ports of the test also for the sending port.*/
        checkCaptureEgressTrafficOnPort(testId,senderPortIndex,portIter,egressTagStateIndex);

        if(portIter == PORT_INDEX_FOR_MIRROR_TO_CPU_COPY)
        {
            /* check also the mirror to CPU copy that come from the sender port */
            PRV_UTF_LOG1_MAC("======= check the mirror to CPU copy that come from the sender port [%d] of the test =======\n",prvTgfPortsArray[senderPortIndex]);
            /*check captured egress traffic on the sending port.*/
            checkCaptureEgressTrafficOnPort(testId,senderPortIndex,senderPortIndex,0/*not relevant for sender port*/);
        }

        /* clear table */
        PRV_UTF_LOG0_MAC("======= clear captured packets table in the CPU. =======\n");
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


        /*break; -- AmitK HW debug only */

        if((testId != PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E) &&
            GT_TRUE == prvUtfIsGmCompilation())
        {
            /* reduce tested cases iterations , due to performance */
            portIter            += 2;
            egressTagStateIndex += 2;
        }
    }

    utfGeneralStateMessageSave(3,"--");
    utfGeneralStateMessageSave(6,"--");

    PRV_UTF_LOG0_MAC("======= check FDB learned the mac in correct vlan =======\n");

    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E)
    {
        /* the learning according to MAC SA of outer mac */
        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther , prvTgfPacketL2Part.saMac ,sizeof(TGF_MAC_ADDR));
    }
    else /* the learning according to MAC SA of passenger */
    {
        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther , prvTgfPacketL2Part_Passenger.saMac ,sizeof(TGF_MAC_ADDR));
    }
    macEntryKey.key.macVlan.vlanId = egressTagStateArr[testId].vlanId;

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey,&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind: the mac not found in FDB on expected vlan %d",
                             egressTagStateArr[testId].vlanId);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal bridgeRestore function
* @endinternal
*
* @brief   restore bridge configurations.
*
* @param[in] testId                   - test ID
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static GT_VOID bridgeRestore
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
)
{
    GT_U16      vlanId;
    GT_U32      ethIter;
    GT_STATUS   rc      ;
    GT_U32      portIter;
    GT_U32    portsCount  = prvTgfPortsNum;

    TGF_PARAM_NOT_USED(senderPortIndex);

    vlanId = egressTagStateArr[testId].vlanId;

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: restore ingress\egress TPID tables for TAG0\TAG1 */
    /* AUTODOC: restore ingress\egress TPID select for TAG0\TAG1 */
    if (prvTgfIsInitialized)
    {
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* restore egress TPID select for TAG0 */
            rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                                prvTgfPortsArray[portIter],
                                                CPSS_VLAN_ETHERTYPE0_E,
                                                prvTgfDefTpidBmpStc[portIter].egrTag0TpidIndex);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE0_E);

            /* restore egress TPID select for TAG1 */
            rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                                prvTgfPortsArray[portIter],
                                                CPSS_VLAN_ETHERTYPE1_E,
                                                prvTgfDefTpidBmpStc[portIter].egrTag1TpidIndex);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE1_E);


            /* restore ingress TPID select for TAG0 */
            rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                                 prvTgfPortsArray[portIter],
                                                 CPSS_VLAN_ETHERTYPE0_E,
                                                 prvTgfDefTpidBmpStc[portIter].ingTag0TpidBmp);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE0_E);

            /* restore ingress TPID select for TAG1 */
            rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                                 prvTgfPortsArray[portIter],
                                                 CPSS_VLAN_ETHERTYPE1_E,
                                                 prvTgfDefTpidBmpStc[portIter].ingTag1TpidBmp);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         CPSS_VLAN_ETHERTYPE1_E);

        }

        /* restore egress TPID table */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                       0,
                                       prvTgfDefEgrEtherTypesStc.etherType0);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E, 0);

        /* restore egress TPID table */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                       1,
                                       prvTgfDefEgrEtherTypesStc.etherType1);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E, 1);

        /* restore ingress TPID table */
        for (ethIter = 0 ; ethIter < PRV_TGF_MAX_ENTRY_INDEX_CNS; ethIter++)
        {
            /* set TPID table entry */
            rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,
                                           ethIter,
                                           ethertypesTable_orig[ethIter]);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                         prvTgfDevNum, CPSS_DIRECTION_INGRESS_E,
                                         ethIter, ethertypesTable_orig[ethIter]);
        }
    }

    if(GT_TRUE == prvTgfCommonIsDeviceForce(prvTgfDevNum))
    {
        /* 'free' (set 0x8100) entries 0..7 in the egress,ingress table */
        prvTgfCommonUseLastTpid(GT_TRUE);
        for(portIter = 0 ; portIter < PRV_TGF_MAX_ENTRY_INDEX_CNS ; portIter ++)
        {
            /* set additional configuration */
            prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,
                                           portIter, TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
            prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                           portIter, TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
        }
        prvTgfCommonUseLastTpid(GT_FALSE);
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, vlanId);

    /* system restored */
    prvTgfIsInitialized = GT_FALSE;
}

/**
* @internal tunnelRestore function
* @endinternal
*
* @brief   restore tunnel configurations.
*
* @param[in] testId                   - test ID
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static GT_VOID tunnelRestore
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
)
{
    GT_STATUS   rc      ;

    if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E)
    {
        /* we not set any tunnel termination for this test */
        return;
    }

    /* restore PCL id for the MPLS packets */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_MPLS_E,origMplsTtiPclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");


    prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS,GT_FALSE);

    /* disable the TTI lookup for PRV_TGF_TTI_KEY_MPLS_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");
}

/**
* @internal mirrorRestore function
* @endinternal
*
* @brief   restore mirror configurations.
*
* @param[in] testId                   - test ID
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static GT_VOID mirrorRestore
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
)
{
    GT_STATUS       rc;
    GT_PORT_NUM      port;

    if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E)
    {
        /* need to set eport mirroring on the new src eport */
        port = NEW_SRC_EPORT_CNS;
    }
    else
    {
        port = prvTgfPortsArray[senderPortIndex];
    }

    /* Disable Rx mirroring on ingress port. */
    rc = prvTgfMirrorRxPortSet(port, GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 port, GT_FALSE);
}


/**
* @internal prvTgfBrgVlanManipulationEthernetOverMpls_isValid function
* @endinternal
*
* @brief   check if test is valid for the current device
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port (0..11)
*
* @retval GT_TRUE                  - test is valid for the device
* @retval GT_FALSE                 - test is not valid for the device
*/
GT_BOOL prvTgfBrgVlanManipulationEthernetOverMpls_isValid
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
)
{
#ifdef CHX_FAMILY
    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) == GT_FALSE)
    {
        if(testId == PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_PASSENGER_SRC_EPORT_REASSIGN_E)
        {
            return GT_FALSE;
        }
    }

    if(testId >= PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST____LAST____E)
    {
        return GT_FALSE;
    }

    if(senderPortIndex >= NUM_PORTS_IN_TEST)
    {
        return GT_FALSE;
    }

    return GT_TRUE;
#else
    TGF_PARAM_NOT_USED(testId);
    TGF_PARAM_NOT_USED(senderPortIndex);

    return GT_FALSE;
#endif
}

/**
* @internal prvTgfBrgVlanManipulationEthernetOverMplsConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port (0..11)
*                                       None
*/
GT_VOID prvTgfBrgVlanManipulationEthernetOverMplsConfigurationSet
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
)
{
    GT_U32  portIter;
    GT_U32  ii;

    PRV_UTF_LOG2_MAC("======= start test [%s] sender index[%d]=======\n",testNames[testId],senderPortIndex);
    PRV_UTF_LOG1_MAC("== test purpose[%s] =======\n",testPurpose[testId]);

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    utfGeneralStateMessageSave(1,"testId = %d",testId);
    utfGeneralStateMessageSave(2,"senderPortIndex = %d",senderPortIndex);

    orig_prvTgfPortsNum = prvTgfPortsNum;
    prvTgfPortsNum = NUM_PORTS_IN_TEST;

    totalError = 0;

    for(portIter = 0 ; portIter < prvTgfPortsNum; portIter++)
    {
        orig_portsForTest[portIter] = prvTgfPortsArray[portIter];

        if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
        {
            /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
            continue;
        }

        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType == CPSS_98DX8332_Z0_CNS)
        {
            prvTgfPortsArray[portIter] = portsForTest_aldrinZ0[portIter];
        }
        else
        {
            prvTgfPortsArray[portIter] = portsForTest[portIter];

            if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
            {
                if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
                    (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
                {
                    /* Aldrin have 32 ports but use 28 port devices list in order to not add new map */
                    prvTgfPortsArray[portIter] %= 28;
                }
                else
                {
                    /* Ports for Tests adapted for only first 48 ports of Bobcat2 */
                    /* because those are GIGA port that got proper initialization during cpssInitSystem */
                    /* the XLG ports need proper init */
                    /* next logic is to avoid port numbers collisions ! */
                    if(prvTgfPortsArray[portIter] > 48)
                    {
                        prvTgfPortsArray[portIter] -= 48;
                    }
                    prvTgfPortsArray[portIter] %= 47;
                }
            }
            else
            {
                prvTgfPortsArray[portIter] %= PRV_CPSS_PP_MAC(prvTgfDevNum)->numOfPorts;
            }

#ifdef CHX_FAMILY
            if((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) == GT_FALSE) &&
               (0 == PRV_CPSS_PHY_PORT_IS_EXIST_MAC(prvTgfDevNum,prvTgfPortsArray[portIter])) &&
               (prvTgfPortsArray[portIter] >= 2))
            {
                prvTgfPortsArray[portIter] -= 2;
            }
#endif /*CHX_FAMILY*/
        }
    }

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   senderPortIndex,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }


    /* check ports duplications */
    for(portIter = 0 ; portIter < prvTgfPortsNum; portIter++)
    {
        for(ii = 0 ; ii < portIter; ii++)
        {
            if(prvTgfPortsArray[portIter] == prvTgfPortsArray[ii])
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, GT_BAD_STATE, "ERROR : prvTgfPortsArray[%d] == prvTgfPortsArray[%d] = %d",
                                             portIter , ii , prvTgfPortsArray[portIter]);

                totalError = 1;
            }
        }
    }

    if(totalError)
    {
        return;
    }

    bridgeInit(testId,senderPortIndex);

    tunnelInit(testId,senderPortIndex);

}

/**
* @internal prvTgfBrgVlanManipulationEthernetOverMplsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port (0..11)
*                                       None
*/
GT_VOID prvTgfBrgVlanManipulationEthernetOverMplsTrafficGenerate
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
)
{
    if(totalError)
    {
        return;
    }

    testPacketsSendAndCheck(testId,senderPortIndex);
}

/**
* @internal prvTgfBrgVlanManipulationEthernetOverMplsConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] testId                   - id of the test
* @param[in] senderPortIndex          - index of the sending port (0..11)
*                                       None
*/
GT_VOID prvTgfBrgVlanManipulationEthernetOverMplsConfigurationRestore
(
    IN PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_TYPE_ENT       testId ,
    IN GT_U32       senderPortIndex
)
{
    GT_STATUS   rc;
    GT_U32  portIter;

    if(totalError == 0)
    {
        bridgeRestore(testId,senderPortIndex);
        tunnelRestore(testId,senderPortIndex);
        mirrorRestore(testId,senderPortIndex);
        /* clear table */
        PRV_UTF_LOG0_MAC("======= clear captured packets table in the CPU. =======\n");
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }

    for(portIter = 0 ; portIter < prvTgfPortsNum; portIter++)
    {
        prvTgfPortsArray[portIter] = orig_portsForTest[portIter];
    }

    prvTgfPortsNum = orig_prvTgfPortsNum;

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    PRV_UTF_LOG2_MAC("======= ended test [%s] sender index[%d]=======\n",testNames[testId],senderPortIndex);
}

void debug_ut_fail(void)
{
    GT_U32 portIter, ingTag0TpidBmp, ingTag1TpidBmp, egrTag0TpidIndex, egrTag1TpidIndex;
    GT_STATUS rc;

    /* print ingress TPID select for TAG0 */
    for (portIter = 0; portIter < NUM_PORTS_IN_TEST; portIter++)
    {
        rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,
                                             prvTgfPortsArray[portIter],
                                             CPSS_VLAN_ETHERTYPE0_E,
                                             &ingTag0TpidBmp);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     CPSS_VLAN_ETHERTYPE0_E);

        /* save ingress TPID select for TAG1 */
        rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,
                                             prvTgfPortsArray[portIter],
                                             CPSS_VLAN_ETHERTYPE1_E,
                                             &ingTag1TpidBmp);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     CPSS_VLAN_ETHERTYPE1_E);
        cpssOsPrintf(" Ingress Tpid0/1 0x%X 0x%X\n", ingTag0TpidBmp, ingTag1TpidBmp);


        /* save egress TPID select for TAG0 */
        rc = prvTgfBrgVlanPortEgressTpidGet(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            CPSS_VLAN_ETHERTYPE0_E,
                                            &egrTag0TpidIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidGet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     CPSS_VLAN_ETHERTYPE0_E);

        /* save egress TPID select for TAG1 */
        rc = prvTgfBrgVlanPortEgressTpidGet(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            CPSS_VLAN_ETHERTYPE1_E,
                                            &egrTag1TpidIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidGet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     CPSS_VLAN_ETHERTYPE1_E);

        cpssOsPrintf(" Egress Tpid0/1 0x%X 0x%X\n", egrTag0TpidIndex, egrTag1TpidIndex);

    }

}

