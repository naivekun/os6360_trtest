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
* @file prvTgfVlanManipulation.c
*
* @brief VLAN manipulation
*
* @version   31
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

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrityTables.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <port/prvTgfPortFWS.h>
#include <bridge/prvTgfVlanManipulation.h>
#include <common/tgfCommon.h>
#include <utf/private/prvUtfExtras.h>

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgStp.h>
#elif defined CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#endif

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

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* TAG1 VLAN Id */
#define PRV_TGF_VLANID1_CNS 10

/* TAG VLAN Id */
#define PRV_TGF_VLANID2_CNS 25

/* TAG VLAN Id */
#define PRV_TGF_VLANID3_CNS 4000

/* TAG VLAN Id */
#define PRV_TGF_VLANID4_CNS 100

/* default vid1 - for untagged TAG1 packets */
#define PRV_TGF_DEFAULT_VID1_CNS 0xFED


/* ether types */
#define PRV_TGF_ETHERTYPE_0_CNS  BIT_0
#define PRV_TGF_ETHERTYPE_1_CNS  BIT_1
#define PRV_TGF_ETHERTYPE_2_CNS  BIT_2
#define PRV_TGF_ETHERTYPE_3_CNS  BIT_3
#define PRV_TGF_ETHERTYPE_4_CNS  BIT_4
#define PRV_TGF_ETHERTYPE_5_CNS  BIT_5
#define PRV_TGF_ETHERTYPE_6_CNS  BIT_6
#define PRV_TGF_ETHERTYPE_7_CNS  BIT_7

/* max entry index for TPID table */
#define PRV_TGF_MAX_ENTRY_INDEX_CNS  8

/* ingress port index */
static GT_U32 prvTgfIngrPortIdx = 3;
#define INGR_PORT_IDX_MAC prvTgfIngrPortIdx

/* egress port index */
static GT_U32 prvTgfEgrPortIdx = 2;
#define EGR_PORT_IDX_MAC prvTgfEgrPortIdx

/* default number of packets to send */
static GT_U32       prvTgfBurstCount   = 1;

/* array of source MACs for the tests */
static TGF_MAC_ADDR prvTgfSaMacArr[] = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x04} };

/* array of destination MACs for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[] = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x12},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x13},
                                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x14} };

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* srcMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* dstMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* Second VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,                 /* etherType */
    0, 0, PRV_TGF_VLANID1_CNS                           /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/************************ Double tagged packet ********************************/

/* PARTS of double tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketDoubleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of double tagged packet */
#define PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + 2 * TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of double tagged packet with CRC */
#define PRV_TGF_PACKET_DOUBLE_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Double tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoDoubleTag =
{
    PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketDoubleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketDoubleTagPartArray                                        /* partsArray */
};

/************************ Single tagged packet ********************************/

/* PARTS of single tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketSingleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of single tagged packet */
#define PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of single tagged packet with CRC */
#define PRV_TGF_PACKET_SINGLE_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Single tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoSingleTag =
{
    PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketSingleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketSingleTagPartArray                                        /* partsArray */
};

/************************ Untagged packet ********************************/

/* PARTS of untagged packet */
static TGF_PACKET_PART_STC prvTgfPacketUntagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of untagged packet */
#define PRV_TGF_PACKET_UNTAG_LEN_CNS TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of untagged packet with CRC */
#define PRV_TGF_PACKET_UNTAG_CRC_LEN_CNS  PRV_TGF_PACKET_UNTAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Untagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoUntag =
{
    PRV_TGF_PACKET_UNTAG_LEN_CNS,                                /* totalLen */
    sizeof(prvTgfPacketUntagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketUntagPartArray                                        /* partsArray */
};


/*****************************************************************/
/* Full wire speed packet for VLAN and STG table workaround test */
/*****************************************************************/
/* L2 part of packet FWS */
static TGF_PACKET_L2_STC prvTgfPacketFWSL2Part = {
    {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x11},               /* dstMac */
    {0x00, 0x88, 0x99, 0x33, 0x44, 0x11}                /* srcMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataFWSArr[] = {
    0x12, 0x34, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
    0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
    0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadFWSPart = {
    sizeof(prvTgfPayloadDataFWSArr),                       /* dataLength */
    prvTgfPayloadDataFWSArr                                /* dataPtr */
};

/* PARTS of packet FWS */
static TGF_PACKET_PART_STC prvTgfPacketFWSPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketFWSL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadFWSPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_FWS_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataFWSArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_FWS_CRC_LEN_CNS  PRV_TGF_PACKET_FWS_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET FWS to send */
static TGF_PACKET_STC prvTgfPacketFWSInfo = {
    PRV_TGF_PACKET_FWS_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketFWSPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketFWSPartArray                                        /* partsArray */
};

/* Dummy VLAN id */
#define PRV_TGF_DUMMY_VLANID_CNS (PRV_TGF_VLANID_CNS + 100)

/* Number of iterations for  */
#define PRV_TGF_WA_ITER_NUM_CNS 10

/* Size is number of STG entries , according to 'tested' system */
#ifdef EXMXPM_FAMILY
    #define PRV_TFG_STG_ENTRY_SIZE_CNS      CPSS_EXMXPM_STG_ENTRY_SIZE_CNS
#elif defined CHX_FAMILY
    #define PRV_TFG_STG_ENTRY_SIZE_CNS      CPSS_DXCH_STG_ENTRY_SIZE_CNS
#else
    #define PRV_TFG_STG_ENTRY_SIZE_CNS      8/* should not use it ! */
#endif /**/

#define PRV_TFG_STG_DEFAULT_ENTRY_IDX_CNS 0
#define PRV_TFG_STG_DUMMY_ENTRY_IDX_CNS 1

/************************************************************************/
/* End of full wire speed packet for VLAN and STG table workaround test */
/************************************************************************/

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* flag to show if system is initialized*/
static GT_BOOL  prvTgfIsInitialized = GT_FALSE;

/* stored default PVID */
static GT_U16   prvTgfPvid = 0;

/* stored default PVID1 */
static GT_U16   origIngressDefaultVid1 = 0;

/* current default PVID1 */
static GT_U16   currentIngressDefaultVid1 = 0;


/* stored default VLAN Range */
static GT_U16   prvTgfDefVidRange = 0;

/* stored default TPID table entries */
static GT_U16   prvTgfDefEtherTypeArr[PRV_TGF_MAX_ENTRY_INDEX_CNS] = {0};

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
} prvTgfDefTpidBmpStc;

/* indication that we check the egress port as dsa tag port */
static GT_BOOL              testEgressPortAsDsa = GT_FALSE;
/* The DSA tag type for the egress port - relevant when testEgressPortAsDsa = GT_TRUE */
static TGF_DSA_TYPE_ENT     testEgressDsaType = TGF_DSA_LAST_TYPE_E;
/* what is the vlan tag state for the the egress port */
static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT testEgressDsaVlanTagSate = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;

typedef enum{
    unknown_e
    ,tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_E
    ,tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_E
    ,tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_E
}DSA_TEST_ENT;

static DSA_TEST_ENT dsaTestName = unknown_e;

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_dsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x20,0x10,0x05,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x20,0x00,0x1f,0xff,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0,0,0,0,0,0},             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0,0,0,0,0,0},             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_eDsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x20,0x10,0x05,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0xa0,0x00,0x1f,0xff,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0x80,0x00,0x00,0x00,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0x06,0x00,0x12,0x40,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_dsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x90,0x10,0x0a,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x00,0x00,0x1f,0xff,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0,0,0,0,0,0},             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0,0,0,0,0,0},             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};
/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_eDsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x90,0x10,0x0a,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x80,0x00,0x1f,0xff,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0x80,0x00,0x00,0x01,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0x06,0x00,0x09,0x40,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_dsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xd0,0x90,0x1f,0xed,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x00,0x00,0x1f,0xff,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0,0,0,0,0,0},             {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0,0,0,0,0,0},             {0,0,0,0,0,0} , 4/*cycleCount*/ , NULL,0,0}
};

/* the expected DSA words from the egress port */
static TGF_VFD_INFO_STC     tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_eDsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x90,0x1f,0xed,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x80,0x00,0x1f,0xff,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*2 /*offset*/, {0x80,0x00,0x00,0x01,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*3 /*offset*/, {0x02,0x00,0x09,0x40,0,0}, {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/**
* @enum DATA_INTEGRITY_STAGE_ENT
 *
 * @brief This enum defines vlan test data integrity stages
 * NOTE : the Enum supports combination of 'stages'
*/
typedef enum{

    /** the test not involve data integrity issues. */
    DATA_INTEGRITY_STAGE_NONE_E                 = 0,

    /** testing data integrity issues in the ingress pipe. */
    DATA_INTEGRITY_STAGE_INGRESS_PIPE_E         = BIT_0,

    /** testing data integrity issues in the egress pipe - part 1 */
    DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_1_E   = BIT_1,

    /** testing data integrity issues in the egress pipe - part 2 */
    DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_2_E   = BIT_2,

    DATA_INTEGRITY_STAGE_EGRESS_PIPE_E = (DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_1_E |  DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_2_E)

} DATA_INTEGRITY_STAGE_ENT;

static DATA_INTEGRITY_STAGE_ENT errorInjectionStage = DATA_INTEGRITY_STAGE_NONE_E;
static CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  errorInterrupts[128];
static GT_U32   numOfErrorInterrupts = 0;
static GT_U8 strNameBuffer[120] = "";
#ifdef CPSS_LOG_ENABLE
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[];
extern GT_U32 prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT;
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT[];
extern GT_U32 prvCpssLogEnum_size_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT;

#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer) \
    PRV_CPSS_LOG_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)

#else

#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)
#endif

typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);
extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC    *dxChDataIntegrityEventIncrementFunc;
static DXCH_DATA_INTEGRITY_EVENT_CB_FUNC    *savedDataIntegrityHandler;
static CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC    errorLocationInfo;
static CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
static CPSS_DXCH_TABLE_ENT dataIntegrity_expectedHwTablesIngress[] =
    {CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
     CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,/* happen 2(bc2), 4(bc3) times */
     CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E,
     /* must be last */
     CPSS_DXCH_TABLE_LAST_E};
static CPSS_DXCH_TABLE_ENT dataIntegrity_expectedHwTablesEgress_part1[] =
    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,/* happen 3(bc2), 2(bc3) times */
     CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
     CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
     /* must be last */
     CPSS_DXCH_TABLE_LAST_E};

static CPSS_DXCH_TABLE_ENT dataIntegrity_expectedHwTablesEgress_part2[] =
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,/* happen 6(bc2), 3(aldrin, bobk), 1(bc3) times */
     /* must be last */
     CPSS_DXCH_TABLE_LAST_E};

static CPSS_DXCH_TABLE_ENT dataIntegrity_expectedHwTablesEgress_part1_bc3[] =
    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,/* happen 2 (bc3) times */
     CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
     CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
     CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
     /* must be last */
     CPSS_DXCH_TABLE_LAST_E};



/**
* @internal hwInfoEventErrorHandlerFunc function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*         Store HW related info from event to global structure.
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS hwInfoEventErrorHandlerFunc
(
    IN GT_U8                                     dev,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr

)
{
    CPSS_NULL_PTR_CHECK_MAC(eventPtr);
    if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }

    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(eventPtr->location.hwEntryInfo.hwTableType,
                                                CPSS_DXCH_TABLE_ENT, strNameBuffer);

    PRV_UTF_LOG0_MAC("======DATA INTEGRITY ERROR EVENT======\n");
    PRV_UTF_LOG1_MAC("Event type        :  %d\n",   eventPtr->eventsType);
    PRV_UTF_LOG1_MAC("HW table index    :  %d\n",   eventPtr->location.hwEntryInfo.hwTableEntryIndex);
    PRV_UTF_LOG4_MAC("RAM table index   :  %d {%d, %d, %d}\n",
                     eventPtr->location.ramEntryInfo.ramRow,
                     eventPtr->location.ramEntryInfo.memLocation.dfxPipeId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxClientId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId);
    PRV_UTF_LOG1_MAC("HW table type     :  %s\n",   strNameBuffer);

    errorInterrupts[numOfErrorInterrupts] = *eventPtr;

    numOfErrorInterrupts++;

    return GT_OK;
}

/**
* @internal prvTgfEgressPortCascadeSet function
* @endinternal
*
* @brief   set/unset the egress port as cascade with DSA tag .
*
* @param[in] portInterfacePtr         - (pointer to) port interface (port only is legal param)
* @param[in] enable                   - enable/disable the cascade port.
* @param[in] egrDsaType               - type of DSAtag on egress port
*                                       None
*/
GT_VOID prvTgfEgressPortCascadeSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                  enable,
    IN TGF_DSA_TYPE_ENT         egrDsaType
)
{
    GT_STATUS rc;
    CPSS_CSCD_PORT_TYPE_ENT cscdPortType = 0;
    GT_U8                   egressDevice;
    GT_PHYSICAL_PORT_NUM    egressPort  ;

    /* set the egress port to be DSA tagged */
    switch(egrDsaType)
    {
        case TGF_DSA_2_WORD_TYPE_E:
            cscdPortType = CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E;
            break;
        case TGF_DSA_4_WORD_TYPE_E:
            cscdPortType = CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E;
            break;
        default:
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "setEgressPortCascade: not supported DSA type \n");
            return;
    }

    egressDevice = (GT_U8)portInterfacePtr->devPort.hwDevNum;
    egressPort   = portInterfacePtr->devPort.portNum;

    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
        egressDevice,egressPort, enable,
        enable ? cscdPortType : CPSS_CSCD_PORT_NETWORK_E);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "tgfTrafficGeneratorEgressCscdPortEnableSet:  cscdPortType[%d] \n",
            cscdPortType);
}

/**
* @internal prvTgfVlanManipulationTestAsDsa function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*
* @param[in] egressDsaType            - type of DSA tag on egress port
*                                       None
*/
GT_STATUS prvTgfVlanManipulationTestAsDsa
(
    TGF_DSA_TYPE_ENT    egressDsaType
)
{
    testEgressPortAsDsa = GT_TRUE;
    testEgressDsaType   = egressDsaType;
    return GT_OK;
}

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfBrgVlanManipulationTestInit function
* @endinternal
*
* @brief   Set VLAN entry with received tag command, set global ingress/egress
*         TPID tables.
* @param[in] vlanId                   -  to be configured
* @param[in] tagCmd                   - VLAN tag command
* @param[in] etherTypeNum             - number of entries in array
* @param[in] etherTypeArr[]           - array of Ethertypes
* @param[in] tag0TpidBmp              - bitmap represent entries in the TPID Table for TAG0
* @param[in] tag1TpidBmp              - bitmap represent entries in the TPID Table for TAG1
*                                       None
*/
static GT_VOID prvTgfBrgVlanManipulationTestInit
(
    IN GT_U16                            vlanId,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd,
    IN GT_U32                            etherTypeNum,
    IN GT_U16                            etherTypeArr[],
    IN GT_U32                            tag0TpidBmp,
    IN GT_U32                            tag1TpidBmp
)
{
    GT_U32                      portIter  = 0;
    GT_U32                      portCount = 0;
    GT_U32                      ethIter   = 0;
    GT_U32                      index     = 0;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    if (!prvTgfIsInitialized)
    {
        /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
         * configured cascade) are not remote ports */
        if (testEgressPortAsDsa &&
            UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
        {
            GT_STATUS   rc;
            rc = prvTgfDefPortsArrange(GT_FALSE,
                                       EGR_PORT_IDX_MAC,
                                       -1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
        }
    }

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

    if(vlanId == PRV_TGF_VLANID_CNS)
    {
        /* the vlan that used for egress vlan tag state ??? */
        testEgressDsaVlanTagSate = tagCmd;
    }

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = tagCmd;
    }

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);


    /* Configure ingress TPID table */
    for (ethIter = 0, index = 0; ethIter < PRV_TGF_MAX_ENTRY_INDEX_CNS; ethIter++)
    {
        if ((1 == (tag0TpidBmp >> ethIter & 1)) || (1 == (tag1TpidBmp >> ethIter & 1)))
        {
            /* check index range in array of ethertypes */
            if (index > etherTypeNum)
            {
                PRV_UTF_LOG1_MAC("WARNING: index [%d] is out of range!\n", index);

                break;
            }

            if (!prvTgfIsInitialized)
            {
                /* save default TPID table entry */
                rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,
                                               ethIter,
                                               &(prvTgfDefEtherTypeArr[index]));
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d, %d, %d",
                                             prvTgfDevNum, CPSS_DIRECTION_INGRESS_E,
                                             ethIter);
            }

            /* set TPID table entry */
            rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,
                                           ethIter,
                                           etherTypeArr[index++]);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                         prvTgfDevNum, CPSS_DIRECTION_INGRESS_E,
                                         ethIter, etherTypeArr[index - 1]);
        }
    }

    if (!prvTgfIsInitialized)
    {
        for(index = 2 ; index < 6; index++)
        {
            rc = sip5Emulate_IngressTpidProfileClear(CPSS_VLAN_ETHERTYPE0_E,index);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "sip5Emulate_IngressTpidProfileClear: %d, %d, %d",
                                         prvTgfDevNum, CPSS_VLAN_ETHERTYPE0_E ,index);
            rc = sip5Emulate_IngressTpidProfileClear(CPSS_VLAN_ETHERTYPE1_E,index);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "sip5Emulate_IngressTpidProfileClear: %d, %d, %d",
                                         prvTgfDevNum, CPSS_VLAN_ETHERTYPE1_E , index);
        }

        /* save ingress TPID select for TAG0 */
        rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,
                                             prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                             CPSS_VLAN_ETHERTYPE0_E,
                                             &(prvTgfDefTpidBmpStc.ingTag0TpidBmp));
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE0_E);

        /* save ingress TPID select for TAG1 */
        rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,
                                             prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                             CPSS_VLAN_ETHERTYPE1_E,
                                             &(prvTgfDefTpidBmpStc.ingTag1TpidBmp));
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE1_E);
    }

    /* Set ingress TPID select for TAG0 */
    rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                         CPSS_VLAN_ETHERTYPE0_E,
                                         tag0TpidBmp);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                 CPSS_VLAN_ETHERTYPE0_E, tag0TpidBmp);

    /* Set ingress TPID select for TAG1 */
    rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         tag1TpidBmp);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                 CPSS_VLAN_ETHERTYPE1_E, tag1TpidBmp);

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

    /* AUTODOC: set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8 */
    /*  (by default Ethertype2-7 remain 0x8100). */
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                   0,
                                   0x9100);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                 prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                 0, 0x9100);

    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                   1,
                                   0x98a8);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                 prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                 1, 0x98a8);

    if (!prvTgfIsInitialized)
    {
        /* save egress TPID select for TAG0 */
        rc = prvTgfBrgVlanPortEgressTpidGet(prvTgfDevNum,
                                            prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                            CPSS_VLAN_ETHERTYPE0_E,
                                            &(prvTgfDefTpidBmpStc.egrTag0TpidIndex));
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidGet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE0_E);

        /* save egress TPID select for TAG1 */
        rc = prvTgfBrgVlanPortEgressTpidGet(prvTgfDevNum,
                                            prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                            CPSS_VLAN_ETHERTYPE1_E,
                                            &(prvTgfDefTpidBmpStc.egrTag1TpidIndex));
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE1_E);
    }

    /* AUTODOC: set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1. */
    rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                        prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                        CPSS_VLAN_ETHERTYPE0_E,
                                        0);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d, %d, %d",
                                 prvTgfDevNum, 0,
                                 CPSS_VLAN_ETHERTYPE0_E, 0);

    rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                        prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                        CPSS_VLAN_ETHERTYPE1_E,
                                        1);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d, %d, %d",
                                 prvTgfDevNum, 0,
                                 CPSS_VLAN_ETHERTYPE1_E, 1);


    if (!prvTgfIsInitialized)
    {
        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
        {
            /* AUTODOC: get orig default VID1 for non tag1 tagged packets */
            rc = prvTgfBrgVlanPortVid1Get(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], CPSS_DIRECTION_INGRESS_E, &origIngressDefaultVid1);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVid1Get: %d, %d",
                                         prvTgfPortsArray[INGR_PORT_IDX_MAC], CPSS_DIRECTION_INGRESS_E);

            currentIngressDefaultVid1 = PRV_TGF_DEFAULT_VID1_CNS;
            /* AUTODOC: set default VID1 to 0xFED for non tag1 tagged packets */
            rc = prvTgfBrgVlanPortVid1Set(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], CPSS_DIRECTION_INGRESS_E, currentIngressDefaultVid1);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVid1Set: %d, %d ,%d",
                                         prvTgfPortsArray[INGR_PORT_IDX_MAC], CPSS_DIRECTION_INGRESS_E , currentIngressDefaultVid1);

        }
    }




    /* system initialized successfully */
    prvTgfIsInitialized = GT_TRUE;
}

/**
* @internal prvTgfBrgVlanManipulationTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface (port only is legal param)
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] vfdNum                   - VFD number
* @param[in] vfdArray[]               - VFD array with expected results
* @param[in] srcDstIndex              - index in Src and Dst MAC address array
*                                       None
*/
static GT_VOID prvTgfBrgVlanManipulationTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC          *packetInfoPtr,
    IN GT_U8                    vfdNum,
    IN TGF_VFD_INFO_STC         vfdArray[],
    IN GT_U8                    srcDstIndex
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;
    GT_U32    numTriggers = 0;
    GT_U8                    vfdNum_new = 0;
    TGF_VFD_INFO_STC         vfdArray_new[10];
    TGF_VFD_INFO_STC         *currentVfdPtr;
    TGF_VFD_INFO_STC         *currentDsaWordsVfdPtr;
    GT_U32  ii;
    GT_U32  outerVlanTag;
    GT_U32  dsaVlanTag;
    GT_U32 sourceEport = prvTgfPortsArray[INGR_PORT_IDX_MAC];
    GT_U32 origSourceEport;       /* Original source port in case of Remote port */
    GT_BOOL expectNoEgressPacket  = ((errorInjectionStage == DATA_INTEGRITY_STAGE_NONE_E ||
                                      errorInjectionStage == DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_2_E) ?
                                     /* inspite of errors in CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E
                                      * the packet reaches a destination port. */
                                     GT_FALSE : GT_TRUE);
    CPSS_PORT_MAC_COUNTER_SET_STC portCounters;

#ifdef ASIC_SIMULATION
    expectNoEgressPacket = GT_FALSE;
#endif /*ASIC_SIMULATION*/

    if(testEgressPortAsDsa == GT_TRUE)
    {
        prvTgfEgressPortCascadeSet(portInterfacePtr, GT_TRUE, testEgressDsaType);
        currentVfdPtr = &vfdArray_new[0];

        if((vfdNum + 4) >= (sizeof(vfdArray_new) / sizeof(vfdArray_new[0])))
        {
            rc = GT_FULL;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "need to large num vfd \n");
            return;
        }

        ii = vfdNum;

        if(dsaTestName == tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_E)
        {
            if(testEgressDsaType != TGF_DSA_4_WORD_TYPE_E)
            {
                currentDsaWordsVfdPtr = &tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_dsaWordsVfd[0];
            }
            else
            {
                currentDsaWordsVfdPtr = &tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_eDsaWordsVfd[0];
            }
        }
        else
        if(dsaTestName == tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_E)
        {
            if(testEgressDsaType != TGF_DSA_4_WORD_TYPE_E)
            {
                currentDsaWordsVfdPtr = &tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_dsaWordsVfd[0];
            }
            else
            {
                currentDsaWordsVfdPtr = &tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_eDsaWordsVfd[0];
            }
        }
        else
        if(dsaTestName == tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_E)
        {
            if(testEgressDsaType != TGF_DSA_4_WORD_TYPE_E)
            {
                currentDsaWordsVfdPtr = &tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_dsaWordsVfd[0];
                if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
                {
                    /* <SrcTagged> not indicate tag1 , but tag 0 only (tag 1 was recognized at ingress tag 0 not)*/
                    currentDsaWordsVfdPtr->patternPtr[0] |= 1<<5;/*bit 29 word 0*/

                    /* those device not support setting the 'default port vid1' and are with value 0 */
                    currentDsaWordsVfdPtr->patternPtr[2] &= 0xF0;
                    currentDsaWordsVfdPtr->patternPtr[3]  = 0x00;
                }
            }
            else
            {
                currentDsaWordsVfdPtr = &tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_eDsaWordsVfd[0];
            }
        }
        else
        {
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "test not supported \n");
            return;
        }

        /* align source ePort with real value:
           Src ePort[16:0] = {Word2[12:3], Word1[30:29], Word0[23:19]}
           bits 0:4 of srcEport - Word0[23:19] */
        currentDsaWordsVfdPtr[0].patternPtr[1] &= 7;
        currentDsaWordsVfdPtr[0].patternPtr[1] |= ((sourceEport & 0x1f) << 3);

        if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
        {
            /* bit 5 of srcPort - Word1[29] */
            currentDsaWordsVfdPtr[1].patternPtr[0] &= 0xDF;
            currentDsaWordsVfdPtr[1].patternPtr[0] |= (((sourceEport & 0x20) >> 5) << 5);
        }
        else
        {
            /* SIP_5 supports bits 5:6 of srcEport - Word1[30:29] */
            currentDsaWordsVfdPtr[1].patternPtr[0] &= 0x9F;
            currentDsaWordsVfdPtr[1].patternPtr[0] |= (((sourceEport & 0x60) >> 5) << 5);

            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
            {
                /* <Skip SA lookup> bit 30 in word 2 always '1' in control learning */
                currentDsaWordsVfdPtr[2].patternPtr[0] |= (1 << 6);
            }
        }

        vfdArray_new[ii++] = currentDsaWordsVfdPtr[0];
        vfdArray_new[ii++] = currentDsaWordsVfdPtr[1];
        if(testEgressDsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            origSourceEport =
                prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(prvTgfDevNum,sourceEport);

            currentDsaWordsVfdPtr[3].patternPtr[3] &= 0x7F;
            currentDsaWordsVfdPtr[3].patternPtr[3] |= ((origSourceEport & 1) << 7);

            currentDsaWordsVfdPtr[3].patternPtr[2] = (origSourceEport >> 1) & 0xFF;

            currentDsaWordsVfdPtr[3].patternPtr[1] &= 0xF8;
            currentDsaWordsVfdPtr[3].patternPtr[1] |= ((origSourceEport >> 9) & 0x7);

            vfdArray_new[ii++] = currentDsaWordsVfdPtr[2];
            vfdArray_new[ii++] = currentDsaWordsVfdPtr[3];
        }

        vfdNum_new = (GT_U8)ii;

        for(ii = 0 ; ii < vfdNum; ii++)
        {
            vfdArray_new[ii] = vfdArray[ii];
            /* check if need to set offsets after the DSA tag bytes */
            if(vfdArray_new[ii].offset >= 12)
            {
                if(vfdArray_new[ii].offset == 12 &&
                   testEgressDsaVlanTagSate != PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E)
                {
                    /* the DSA tag will hold the outer vlan in it , but without the 'ethertype' */
                    outerVlanTag = vfdArray[ii].patternPtr[2] << 8 | vfdArray[ii].patternPtr[3];
                    dsaVlanTag = currentDsaWordsVfdPtr[0].patternPtr[2] << 8 | currentDsaWordsVfdPtr[0].patternPtr[3];

                    /* check that the outerVlanTag is like the 'expected DSA' */
                    /* but ignore the CFI bit ! */
                    if((outerVlanTag & 0xEFFF) != (dsaVlanTag & 0xEFFF))
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(outerVlanTag, dsaVlanTag, "(outerVlanTag[0x%x] & 0xFEFF) != (dsaVlanTag[0x%x] & 0xEFFF) \n",
                            outerVlanTag,dsaVlanTag);
                    }

                    /* indication to ignore this VFD -- redundancy with the DSA info */
                    vfdArray_new[ii].cycleCount    = 1;
                    vfdArray_new[ii].offset        = currentDsaWordsVfdPtr[0].offset;
                    vfdArray_new[ii].patternPtr[0] = currentDsaWordsVfdPtr[0].patternPtr[0];
                }
                else
                {
                    vfdArray_new[ii].offset += (vfdNum_new - vfdNum) * 4;/*4 bytes in each added DSA's VFDs */

                    if(testEgressDsaVlanTagSate != PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E)
                    {
                        /* the DSA hold the outer tag , so not full DSA size */
                        vfdArray_new[ii].offset -= 4;
                    }
                }
            }
        }
    }
    else
    {
        vfdNum_new = vfdNum;
        currentVfdPtr = &vfdArray[0];
    }

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

    /* set source and destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[srcDstIndex], sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[srcDstIndex], sizeof(prvTgfPacketL2Part.daMac));

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    if(expectNoEgressPacket == GT_TRUE)
    {
        /* we not expects packets to pass the ingress pipe. No capturing */
    }
    else
    {
        /* clear table */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* Start capturing Tx packets */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                     portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_TRUE);
    }

    /* AUTODOC: send unknown UC packet to ingress port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC]);

    if(expectNoEgressPacket == GT_TRUE)
    {
        /* we not expects packets to pass the ingress pipe */
        cpssOsTimerWkAfter(100);
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],GT_TRUE,&portCounters);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
            if(portIter == INGR_PORT_IDX_MAC)
            {
                continue;
            }

            UTF_VERIFY_EQUAL2_STRING_MAC(0, portCounters.goodPktsSent.l[0],
                "prvTgfReadPortCountersEth: %d, %d : expected 'no packets to egress'",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }
    else
    {
        /* wait for packets come to CPU */
        (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

        /* Stop capturing Tx packets */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                     portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_FALSE);

        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

        /* print captured packets and check TriggerCounters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
                portInterfacePtr, vfdNum_new, currentVfdPtr, &numTriggers);
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     " port = %d, rc = 0x%02X\n", portInterfacePtr->devPort.portNum, rc);

        /* check if captured packet has the same pattern as vfdArray */
        UTF_VERIFY_EQUAL0_STRING_MAC((1 << vfdNum_new) - 1, numTriggers,
                                     "Packet VLAN TAG0/1 pattern is wrong");
    }

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* restore values */
    if(testEgressPortAsDsa == GT_TRUE)
    {
        prvTgfEgressPortCascadeSet(portInterfacePtr, GT_FALSE, testEgressDsaType);
    }

}

/**
* @internal prvTgfBrgVlanManipulationTestTableReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*
* @param[in] vlanId                   -  to be cleared
* @param[in] etherTypeNum             - number of entries in array
* @param[in] tag0TpidBmp              - bitmap represent entries in the TPID Table for TAG0
* @param[in] tag1TpidBmp              - bitmap represent entries in the TPID Table for TAG1
*                                       None
*/
static GT_VOID prvTgfBrgVlanManipulationTestTableReset
(
    IN GT_U16                         vlanId,
    IN GT_U32                         etherTypeNum,
    IN GT_U32                         tag0TpidBmp,
    IN GT_U32                         tag1TpidBmp
)
{
    GT_U32      ethIter = 0;
    GT_U32      index   = 0;
    GT_STATUS   rc      = GT_OK;

    /* restore values */
    testEgressPortAsDsa = GT_FALSE;
    testEgressDsaType   = TGF_DSA_LAST_TYPE_E;


    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore ingress\egress TPID tables for TAG0\TAG1 */
    /* AUTODOC: restore ingress\egress TPID select for TAG0\TAG1 */
    if (prvTgfIsInitialized)
    {
        /* restore egress TPID select for TAG0 */
        rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                            prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                            CPSS_VLAN_ETHERTYPE0_E,
                                            prvTgfDefTpidBmpStc.egrTag0TpidIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE0_E);

        /* restore egress TPID select for TAG1 */
        rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                            prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                            CPSS_VLAN_ETHERTYPE1_E,
                                            prvTgfDefTpidBmpStc.egrTag1TpidIndex);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[EGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE1_E);

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

        /* restore ingress TPID select for TAG0 */
        rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                             prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                             CPSS_VLAN_ETHERTYPE0_E,
                                             prvTgfDefTpidBmpStc.ingTag0TpidBmp);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE0_E);

        /* save ingress TPID select for TAG1 */
        rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                             prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                             CPSS_VLAN_ETHERTYPE1_E,
                                             prvTgfDefTpidBmpStc.ingTag1TpidBmp);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE1_E);

        /* restore ingress TPID table */
        for (ethIter = 0, index = 0; ethIter < PRV_TGF_MAX_ENTRY_INDEX_CNS; ethIter++)
        {
            if ((1 == (tag0TpidBmp >> ethIter & 1)) || (1 == (tag1TpidBmp >> ethIter & 1)))
            {
                /* check index range in array of ethertypes */
                if (index > etherTypeNum)
                {
                    PRV_UTF_LOG1_MAC("WARNING: index [%d] is out of range!\n", index);

                    break;
                }

                /* set TPID table entry */
                rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,
                                               ethIter,
                                               prvTgfDefEtherTypeArr[index++]);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                             prvTgfDevNum, CPSS_DIRECTION_INGRESS_E,
                                             ethIter, prvTgfDefEtherTypeArr[index - 1]);
            }
        }

        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
        {
            /* AUTODOC: restore default VID1 for non tag1 tagged packets */
            rc = prvTgfBrgVlanPortVid1Set(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], CPSS_DIRECTION_INGRESS_E, origIngressDefaultVid1);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVid1Set: %d, %d ,%d",
                                         prvTgfPortsArray[INGR_PORT_IDX_MAC], CPSS_DIRECTION_INGRESS_E , origIngressDefaultVid1);
        }
    }

    if(GT_TRUE == prvTgfCommonIsDeviceForce(prvTgfDevNum))
    {
        /* 'free' (set 0x8100) entries 0..7 in the egress,ingress table */
        prvTgfCommonUseLastTpid(GT_TRUE);
        for(index = 0 ; index < PRV_TGF_MAX_ENTRY_INDEX_CNS ; index ++)
        {
            /* set additional configuration */
            prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,
                                           index, TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
            prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                           index, TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
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

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
static void dataIntegrityErrorInjectionConfigSet(IN GT_BOOL injectEnable)
{
    GT_STATUS   st;

    /* AUTODOC: enable/disabe error injection to vlan table */
    st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(prvTgfDevNum,
        &errorLocationInfo,
        injectMode,
        injectEnable);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
}

/* check that expected interrupts arrived and fix the reported HW entries */
static void dataIntegrityInterruptsCheckAndFix(void)
{
    GT_STATUS   st;
    GT_U32  ii,jj;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC        fixLocation;
    CPSS_DXCH_TABLE_ENT *dataIntegrity_expectedHwTablesPtr = NULL;
    GT_U32  expectedNumInterrupts = 0;
    GT_U32  expectedLogicalIndex;
    GT_U32  expectedLogicalIndex2;
    GT_U32  foundIndexesBmp = 0; /* bit #0==1 if expectedLogicalIndex is found
                                    bit #1==1 if expectedLogicalIndex2 is found */
    CPSS_DXCH_LOGICAL_TABLE_INFO_STC *ltabInfoPtr;
    CPSS_DXCH_TABLE_ENT hwTable;
    DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *savedCallbackFn;

    expectedLogicalIndex = PRV_TGF_VLANID_CNS;
    expectedLogicalIndex2 = expectedLogicalIndex;

    /* bobcat3: the table CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E HW
     * can be in two modes: 4K vlan x 512 ports,  8K vlans x 256 ports .
     * In second case the table holds two entries per single line
     * So line reading triggers interrupts for two entries. */
    hwTable = CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E;
    if (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(prvTgfDevNum) &&
        !PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(prvTgfDevNum, hwTable))
    {
        GT_32 delta = PRV_TABLE_INFO_PTR_GET_MAC(prvTgfDevNum, hwTable)->maxNumOfEntries;

        expectedLogicalIndex2 = (expectedLogicalIndex2 > delta) ?
            expectedLogicalIndex2 - delta :
            expectedLogicalIndex2 + delta;
    }

    if(errorInjectionStage & DATA_INTEGRITY_STAGE_INGRESS_PIPE_E)
    {
        dataIntegrity_expectedHwTablesPtr = &dataIntegrity_expectedHwTablesIngress[0];
        /* port bitmap related number of RAMs depends on type of device.
           Align expected events number accordingly. */
        expectedNumInterrupts =
            (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(prvTgfDevNum)) ? 4 :
            (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(prvTgfDevNum)) ? 6 :
            3; /* bobk, aldrin */

    }
    else
    if(errorInjectionStage & DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_1_E)
    {
        /* port bitmap related number of RAMs depends on type of device.
           Align expected events number accordingly. */
        if (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(prvTgfDevNum))
        {
            dataIntegrity_expectedHwTablesPtr = &dataIntegrity_expectedHwTablesEgress_part1_bc3[0];
            expectedNumInterrupts = 5;
        }
        else if (PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(prvTgfDevNum))
        {
            dataIntegrity_expectedHwTablesPtr = &dataIntegrity_expectedHwTablesEgress_part1_bc3[0];
            expectedNumInterrupts = 4;
        }
        else
        {
            /* bobcat2, caelum, aldrin */
            dataIntegrity_expectedHwTablesPtr = &dataIntegrity_expectedHwTablesEgress_part1[0];
            expectedNumInterrupts = (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(prvTgfDevNum)) ? 5 : 3;
        }

    }
    else
    if(errorInjectionStage & DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_2_E)
    {
        /* port bitmap related number of RAMs depends on type of device.
           Align expected events number accordingly. Not for bobcat3
        */
        dataIntegrity_expectedHwTablesPtr = &dataIntegrity_expectedHwTablesEgress_part2[0];
        expectedNumInterrupts =
            (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(prvTgfDevNum)) ? 6 :
            3; /* bobk, aldrin */
    }
    else /*DATA_INTEGRITY_STAGE_NONE_E or (DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_2_E and bobcat3) */
    {
        dataIntegrity_expectedHwTablesPtr = NULL;
        expectedNumInterrupts = 0;
    }


    /* after the packet should hit the vlan entries */
    /* there should be interrupts generated ... lets collect them */
    cpssOsTimerWkAfter(300);
#ifdef ASIC_SIMULATION
    if(expectedNumInterrupts)
    {
        numOfErrorInterrupts = 1;
        expectedNumInterrupts = numOfErrorInterrupts;
    }
    cpssOsMemSet(&errorInterrupts[0],0,sizeof(errorInterrupts[0]));
    errorInterrupts[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
    errorInterrupts[0].memoryUseType = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_CONFIGURATION_E;
    errorInterrupts[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;
    errorInterrupts[0].location.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    errorInterrupts[0].location.hwEntryInfo.hwTableType = dataIntegrity_expectedHwTablesPtr ?
                        dataIntegrity_expectedHwTablesPtr[0] :
                        0;
    errorInterrupts[0].location.hwEntryInfo.hwTableEntryIndex = 5;
    errorInterrupts[0].location.logicalEntryInfo.numOfLogicalTables = 1;
    errorInterrupts[0].location.logicalEntryInfo.logicaTableInfo[0].logicalTableType = CPSS_DXCH_LOGICAL_TABLE_VLAN_E;
    errorInterrupts[0].location.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = expectedLogicalIndex;
    errorInterrupts[0].location.logicalEntryInfo.logicaTableInfo[0].numEntries = 1;

#endif /*ASIC_SIMULATION*/
    if(numOfErrorInterrupts != expectedNumInterrupts)
    {
        cpssOsPrintf("ERROR: got [%d] and not [%d] Expected Data integrity error(s) due to 'traffic' passing in 'defected memories' \n",
            numOfErrorInterrupts ,expectedNumInterrupts);
        UTF_VERIFY_EQUAL0_PARAM_MAC(expectedNumInterrupts, numOfErrorInterrupts);
    }
    else
    {
        cpssOsPrintf("Good : got [%d] interrupts about Data integrity errors \n",
            numOfErrorInterrupts);

        UTF_VERIFY_EQUAL0_STRING_MAC(expectedNumInterrupts,numOfErrorInterrupts,
            "expected --number of interrupts-- ");

        for(ii = 0 ; ii < numOfErrorInterrupts; ii++)
        {
            /* check that all the 'logical tables' converted to the proper type and index */
            ltabInfoPtr = &errorInterrupts[ii].location.logicalEntryInfo.logicaTableInfo[0];
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_LOGICAL_TABLE_VLAN_E,
                                         ltabInfoPtr->logicalTableType,
                                         "expected CPSS_DXCH_LOGICAL_TABLE_VLAN_E");
            UTF_VERIFY_EQUAL0_STRING_MAC(1,
                errorInterrupts[ii].location.logicalEntryInfo.numOfLogicalTables,
                "expected --single-- CPSS_DXCH_LOGICAL_TABLE_VLAN_E");

            /* check logical indexes. */
            if (CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E ==
                errorInterrupts[ii].location.hwEntryInfo.hwTableType)
            {
                if (ltabInfoPtr->logicalTableEntryIndex == expectedLogicalIndex)
                {
                    foundIndexesBmp |= 1;
                }
                else if (ltabInfoPtr->logicalTableEntryIndex == expectedLogicalIndex2)
                {
                    foundIndexesBmp |= 2;
                }
                else
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(expectedLogicalIndex,
                                                 ltabInfoPtr->logicalTableEntryIndex,
                                                 "expected logical index - got wrong index ");
                }
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(expectedLogicalIndex,
                                             ltabInfoPtr->logicalTableEntryIndex,
                                             "expected logical index - got wrong index ");
            }


            if(dataIntegrity_expectedHwTablesPtr)
            {
                for(jj = 0 ; dataIntegrity_expectedHwTablesPtr[jj] != CPSS_DXCH_TABLE_LAST_E ;jj++)
                {
                    if(errorInterrupts[ii].location.hwEntryInfo.hwTableType ==
                        dataIntegrity_expectedHwTablesPtr[jj])
                    {
                        break;
                    }
                }
                UTF_VERIFY_NOT_EQUAL0_STRING_MAC(CPSS_DXCH_TABLE_LAST_E,
                    dataIntegrity_expectedHwTablesPtr[jj],
                    "expected -- to find HW table in the 'list' (but not found)");
            }
        }
        if (expectedLogicalIndex != expectedLogicalIndex2 && foundIndexesBmp)
        {

            UTF_VERIFY_EQUAL1_STRING_MAC(foundIndexesBmp, 0x3,
                                         "expected two logical indexes. Found only one (bitmap: %x)",
                                         foundIndexesBmp);
        }
    }

    cpssOsMemSet(&fixLocation,0,sizeof(fixLocation));
    /* AUTODOC: call to fix the data integrity errors in 'HW level' */
    fixLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;


    /* cpssDxChDiagDataIntegrityTableEntryFix can produce interrupts
     * by itself fo tables containing several entries in single hw line.
     * Disable the callback to avoid printing this 'garbage' interrupts. */
    savedCallbackFn                     = dxChDataIntegrityEventIncrementFunc;
    dxChDataIntegrityEventIncrementFunc = NULL;

    for(ii = 0 ; ii < numOfErrorInterrupts; ii++)
    {
        fixLocation.portGroupsBmp = errorInterrupts[ii].location.portGroupsBmp;
        fixLocation.info.hwEntryInfo = errorInterrupts[ii].location.hwEntryInfo;

        st = cpssDxChDiagDataIntegrityTableEntryFix(prvTgfDevNum,
            &fixLocation);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* reset it before stating next stage */

    /* Wait while 'garbage' interrupts handling is finished */
    cpssOsTimerWkAfter(50);

    /* restore callback and reset number of interrupts */
    dxChDataIntegrityEventIncrementFunc = savedCallbackFn;
    numOfErrorInterrupts = 0;
}

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Untagged" tagging command for all ports in VLAN.
*         - Build VLAN Entry with VID = 10 and portsMember = 0, 8, 18, 23 and
*         tagging command "TAG0" for all ports in VLAN to be sure that
*         decision is according to VID = 5.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
* @param[in] isWithErrorInjection     - indication that the test need to run with 'error
*                                      injection' to vlan memories
*                                       None
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationSet
(
    GT_BOOL isWithErrorInjection
)
{
    GT_STATUS   st;
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    errorInjectionStage = (isWithErrorInjection == GT_TRUE) ?
            (DATA_INTEGRITY_STAGE_INGRESS_PIPE_E | DATA_INTEGRITY_STAGE_EGRESS_PIPE_E) :
             DATA_INTEGRITY_STAGE_NONE_E;

    cpssOsMemSet(&errorLocationInfo,0,sizeof(errorLocationInfo));
    errorLocationInfo.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    errorLocationInfo.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    errorLocationInfo.info.logicalEntryInfo.numOfLogicalTables = 1;
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType =
        CPSS_DXCH_LOGICAL_TABLE_VLAN_E;
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = 0;/*not relevant in this API */
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].numEntries = 1;/*not relevant in this API */

    if(errorInjectionStage != DATA_INTEGRITY_STAGE_NONE_E)
    {
        savedDataIntegrityHandler = dxChDataIntegrityEventIncrementFunc;
        dxChDataIntegrityEventIncrementFunc = hwInfoEventErrorHandlerFunc;
        numOfErrorInterrupts = 0;
        /* AUTODOC: unmask interrupts from the DFX */
        st = cpssDxChDiagDataIntegrityEventMaskSet(prvTgfDevNum, &errorLocationInfo,
            CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
            CPSS_EVENT_UNMASK_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        dataIntegrityErrorInjectionConfigSet(GT_TRUE);
    }

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress port = 0, egress port = 1 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 0;
    prvTgfEgrPortIdx = 1;

    /* AUTODOC: set ingress TPID table config: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Untagged" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    if(errorInjectionStage != DATA_INTEGRITY_STAGE_NONE_E)
    {
        /* AUTODOC: enable error injection to vlan table (make sure that the next vlan entry also gets errors) */
        /* Enable error injection */
        dataIntegrityErrorInjectionConfigSet(GT_TRUE);
    }

    /* AUTODOC: create VLAN 10 with all ports, "TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID1_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    if(errorInjectionStage != DATA_INTEGRITY_STAGE_NONE_E)
    {
        /* AUTODOC: DISABLE : error injection to vlan table */
        /* DISABLE error injection */
        dataIntegrityErrorInjectionConfigSet(GT_FALSE);
    }
}


/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 10
*         Success Criteria:
*         Output packet has no any tags
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdDoubleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[3];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 10 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 6;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfDaMacArr[0], vfdArray[0].cycleCount);

    vfdArray[1].cycleCount = 6;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 6;
    cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfSaMacArr[0], vfdArray[1].cycleCount);

    vfdArray[2].cycleCount = 6;
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].modeExtraInfo = 0;
    vfdArray[2].offset = 12;
    cpssOsMemCpy(vfdArray[2].patternPtr, prvTgfPayloadDataArr, vfdArray[2].cycleCount);

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 3, vfdArray, 0);
    /* AUTODOC: verify output packet has no any tags */

    if(errorInjectionStage != DATA_INTEGRITY_STAGE_NONE_E)
    {
        /* check that the interrupts as expected */
        /* and fix the errors in the interrupted entries */
        dataIntegrityInterruptsCheckAndFix();

        /* AUTODOC: set data integrity 'state' to 'egress only' */
        errorInjectionStage = DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_1_E;

        /* send packet .. expected to drop (because ingress pipe dropped the
           packet previously so egress pipe was not reached )*/
        prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 3, vfdArray, 0);

        /* check that the interrupts as expected */
        /* and fix the errors in the interrupted entries */
        dataIntegrityInterruptsCheckAndFix();

        if (!(PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(prvTgfDevNum) ||
              PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(prvTgfDevNum)))
        {
            /* in case of bobcat3 all injected memories triggered already on
               the stage EGRESS_PIPE_PART_1_E */

            /* AUTODOC: set data integrity 'state' to 'egress only' */
            errorInjectionStage = DATA_INTEGRITY_STAGE_EGRESS_PIPE_PART_2_E;

            /* send packet .. expected to drop (because egress pipe dropped the
               packet previously so egress pipe was not reached )*/
            prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 3, vfdArray, 0);

            /* check that the interrupts as expected */
            /* and fix the errors in the interrupted entries */
            dataIntegrityInterruptsCheckAndFix();
        }

        /* AUTODOC: set data integrity 'state' to 'none' because now we expect no problem with the traffic */
        errorInjectionStage = DATA_INTEGRITY_STAGE_NONE_E;

        /* send packet .. expected to drop (because ingress pipe dropped the
           packet previously so egress pipe was not reached )*/
        prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 3, vfdArray, 0);

        /* check that no interrupts received */
        dataIntegrityInterruptsCheckAndFix();
    }
}

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   st;
    if(errorInjectionStage != DATA_INTEGRITY_STAGE_NONE_E)
    {
        /* AUTODOC: Restore dxChDataIntegrityEventIncrementFunc */
        dxChDataIntegrityEventIncrementFunc = savedDataIntegrityHandler;
        numOfErrorInterrupts = 0;
        /* AUTODOC: restore : mask interrupts from the DFX */
        st = cpssDxChDiagDataIntegrityEventMaskSet(prvTgfDevNum, &errorLocationInfo,
            CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
            CPSS_EVENT_MASK_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID1_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Untagged" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 1, egress portIdx = 2 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 1;
    prvTgfEgrPortIdx  = 2;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Untagged" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_2_CNS,
                                      PRV_TGF_ETHERTYPE_3_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has no any tags
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdSingleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[3];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 6;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfDaMacArr[0], vfdArray[0].cycleCount);

    vfdArray[1].cycleCount = 6;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 6;
    cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfSaMacArr[0], vfdArray[1].cycleCount);

    vfdArray[2].cycleCount = 6;
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].modeExtraInfo = 0;
    vfdArray[2].offset = 12;
    cpssOsMemCpy(vfdArray[2].patternPtr, prvTgfPayloadDataArr, vfdArray[2].cycleCount);

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 3, vfdArray, 0);
    /* AUTODOC: verify output packet has no any tags */
}

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS,2,
                                            PRV_TGF_ETHERTYPE_2_CNS,
                                            PRV_TGF_ETHERTYPE_3_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Set PVID to 5 – for untagged packet
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Untagged" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationSet
(
    GT_VOID
)
{
    GT_U16      etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 2, egress portIdx = 3 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 2;
    prvTgfEgrPortIdx = 3;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Untagged" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* store default PVID */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], &prvTgfPvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d", prvTgfPortsArray[INGR_PORT_IDX_MAC]);

    /* AUTODOC: set PVID to 5 for untagged packets */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12
*         Success Criteria:
*         Output packet has no any tags
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdUntagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[3];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with no tag */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 6;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfDaMacArr[0], vfdArray[0].cycleCount);

    vfdArray[1].cycleCount = 6;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 6;
    cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfSaMacArr[0], vfdArray[1].cycleCount);

    vfdArray[2].cycleCount = 6;
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].modeExtraInfo = 0;
    vfdArray[2].offset = 12;
    cpssOsMemCpy(vfdArray[2].patternPtr, prvTgfPayloadDataArr, vfdArray[2].cycleCount);

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoUntag, 3, vfdArray, 0);
    /* AUTODOC: verify output packet has no any tags */
}

/**
* @internal prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    /* AUTODOC: restore default PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 25 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Set VLAN RANGE to FFF (by default FFE)
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U16      vlan0Id = 25;
    GT_U16      vlan1Id = 4095;
    GT_U16      etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 3, egress portIdx = 2 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 3;
    prvTgfEgrPortIdx = 2;

    prvTgfPacketVlanTag0Part.vid = vlan0Id;
    prvTgfPacketVlanTag1Part.vid = vlan1Id;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 25 with all ports, "TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(vlan0Id,
                                      PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* store default VLAN range */
    rc = prvTgfBrgVlanRangeGet(&prvTgfDefVidRange);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeGet: %d", prvTgfDevNum);

    /* AUTODOC: set VLAN range to 0xFFF */
    rc = prvTgfBrgVlanRangeSet(0xFFF);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet: %d, %d", prvTgfDevNum, 0xFFF);
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 25
*         TAG1: ethertype = 0x88a8, vlan = 4095
*         Success Criteria:
*         Output packet has one tag TAG0 with
*         ethertype = 0x9100, vlan = 25
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdDoubleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 25 */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 4095 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x19;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 25 */
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U16      vlan0Id = 25;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLANID_CNS;
    prvTgfPacketVlanTag1Part.vid = PRV_TGF_VLANID1_CNS;

    prvTgfBrgVlanManipulationTestTableReset(vlan0Id, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    /* AUTODOC: restore default VLAN range */
    rc = prvTgfBrgVlanRangeSet(prvTgfDefVidRange);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVidRange);
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 2, egress portIdx = 1 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 2;
    prvTgfEgrPortIdx = 1;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has one tag TAG0 with
*         ethertype = 0x9100, vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdSingleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Set PVID to 5 – for untagged packet
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdUntagConfigurationSet
(
    GT_VOID
)
{
    GT_U16      etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 1, egress portIdx = 0 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 1;
    prvTgfEgrPortIdx = 0;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* store default PVID */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], &prvTgfPvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d", prvTgfPortsArray[INGR_PORT_IDX_MAC]);

    /* AUTODOC: set PVID to 5 for untagged packets */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12
*         Success Criteria:
*         Output packet has one tag TAG0 with
*         ethertype = 0x9100, vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdUntagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with no tag */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x20; /* VPT is 1 because of CPSS default */
    vfdArray[0].patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoUntag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
}

/**
* @internal prvTgfBrgVlanManipulationTag0CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag0CmdUntagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    /* AUTODOC: restore default PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 3, egress portIdx = 0 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 3;
    prvTgfEgrPortIdx = 0;

    /* set VLAN entry, TPID global tables */
    if(GT_TRUE == prvTgfCommonIsDeviceForce(prvTgfDevNum))
    {
        /* don't use index 6,7 in the ingress table -- reserved for egress
           ethertypes over cascade/ring ports */
        prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                          PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
                                          2, etherTypeArr,
                                          PRV_TGF_ETHERTYPE_4_CNS,
                                          PRV_TGF_ETHERTYPE_5_CNS);
    }
    else
    {
        /* AUTODOC: set ingress TPID table: */
        /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

        /* AUTODOC: create VLAN 5 with all ports, "TAG1" cmd */
        prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                          PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
                                          2, etherTypeArr,
                                          PRV_TGF_ETHERTYPE_6_CNS,
                                          PRV_TGF_ETHERTYPE_7_CNS);
    }
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 10
*         Success Criteria:
*         Output packet has one tag TAG1 with
*         ethertype = 0x98a8, vlan = 10
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdDoubleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 10*/

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x98;
    vfdArray[0].patternPtr[1] = 0xa8;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x0a;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 10 */
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_6_CNS,
                                            PRV_TGF_ETHERTYPE_7_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 3, egress portIdx = 1 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 3;
    prvTgfEgrPortIdx = 1;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: set VLAN entry on all ports - vid 5, TAG1 cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has one tag TAG1 with
*         ethertype = 0x98a8, vlan = 0
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdSingleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x98;
    vfdArray[0].patternPtr[1] = 0xa8;
    vfdArray[0].patternPtr[2] = (GT_U8)(currentIngressDefaultVid1 >> 8);
    vfdArray[0].patternPtr[3] = (GT_U8)(currentIngressDefaultVid1 >> 0);

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 0 */
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG1" tagging command for all ports in VLAN.
*         - Set PVID to 5 – for untagged packet
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdUntagConfigurationSet
(
    GT_VOID
)
{
    GT_U16      etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 3, egress portIdx = 2 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 3;
    prvTgfEgrPortIdx = 2;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "TAG1" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* store default PVID */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], &prvTgfPvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d", prvTgfPortsArray[INGR_PORT_IDX_MAC]);

    /* AUTODOC: set PVID to 5 for untagged packets */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12
*         Success Criteria:
*         Output packet has one tag TAG1 with
*         ethertype = 0x98a8, vlan = 0
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdUntagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with no tag */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x98;
    vfdArray[0].patternPtr[1] = 0xa8;
    vfdArray[0].patternPtr[2] = (GT_U8)(currentIngressDefaultVid1 >> 8);
    vfdArray[0].patternPtr[3] = (GT_U8)(currentIngressDefaultVid1 >> 0);

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoUntag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 0 */
}

/**
* @internal prvTgfBrgVlanManipulationTag1CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationTag1CmdUntagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    /* AUTODOC: restore default PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag0, Inner Tag1" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS     rc = GT_OK;
    GT_HW_DEV_NUM hwDevNum;

   /* AUTODOC: SETUP CONFIGURATION: */
    dsaTestName = tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_E ;
   /* AUTODOC: set ingress portIdx = 2, egress portIdx = 0 */
   /* change ingress and egress ports */
   prvTgfIngrPortIdx = 2;
   prvTgfEgrPortIdx = 0;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Outer TAG0 Inner TAG1" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    /* set srcHwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_dsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_eDsaWordsVfd[0].patternPtr[0], hwDevNum);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG1: ethertype = 0x88a8, vlan = 10
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG1 with ethertype = 0x98a8,
*         vlan = 10
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 10 */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x05;

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x98;
    vfdArray[1].patternPtr[1] = 0xa8;
    vfdArray[1].patternPtr[2] = 0x00;
    vfdArray[1].patternPtr[3] = 0x0a;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 10 */
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag0, Inner Tag1" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

   /* AUTODOC: SETUP CONFIGURATION: */

   /* AUTODOC: set ingress portIdx = 2, egress portIdx = 1 */
   /* change ingress and egress ports */
   prvTgfIngrPortIdx = 2;
   prvTgfEgrPortIdx = 1;


    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Outer TAG0 Inner TAG1" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG1 with ethertype = 0x98a8,
*         vlan = 0
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x05;

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x98;
    vfdArray[1].patternPtr[1] = 0xa8;
    vfdArray[1].patternPtr[2] = (GT_U8)(currentIngressDefaultVid1 >> 8);
    vfdArray[1].patternPtr[3] = (GT_U8)(currentIngressDefaultVid1 >> 0);

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 0 */
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag0, Inner Tag1" for all ports in VLAN.
*         - Set PVID to 5 - for untagged packet
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationSet
(
    GT_VOID
)
{
    GT_U16      etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 2, egress portIdx = 3 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 2;
    prvTgfEgrPortIdx = 3;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Outer TAG0 Inner TAG1" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* store default PVID */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], &prvTgfPvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d", prvTgfPortsArray[INGR_PORT_IDX_MAC]);

    /* AUTODOC: set PVID to 5 for untagged packets */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Output packet has two tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG1 with ethertype = 0x98a8,
*         vlan = 0
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with no tag */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x20; /* VPT is 1 because of CPSS default */
    vfdArray[0].patternPtr[3] = 0x05;

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x98;
    vfdArray[1].patternPtr[1] = 0xa8;
    vfdArray[1].patternPtr[2] = (GT_U8)(currentIngressDefaultVid1 >> 8);
    vfdArray[1].patternPtr[3] = (GT_U8)(currentIngressDefaultVid1 >> 0);

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoUntag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 0 */
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    /* AUTODOC: restore default PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Outer Tag1, Inner Tag0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS     rc = GT_OK;
    GT_HW_DEV_NUM hwDevNum;

    /* AUTODOC: SETUP CONFIGURATION: */
    dsaTestName = tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_E ;

    /* AUTODOC: set ingress portIdx = 1, egress portIdx = 0 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 1;
    prvTgfEgrPortIdx = 0;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Outer TAG1 Inner TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    /* set srcHwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_dsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_eDsaWordsVfd[0].patternPtr[0], hwDevNum);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG1: ethertype = 0x88a8, vlan = 10
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG1 with
*         ethertype = 0x98a8, vlan = 10 and TAG0 with ethertype = 0x9100,
*         vlan = 5.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 10 */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x98;
    vfdArray[0].patternPtr[1] = 0xa8;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x0a;

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x91;
    vfdArray[1].patternPtr[1] = 0x00;
    vfdArray[1].patternPtr[2] = 0x00;
    vfdArray[1].patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 10 */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag1, Inner Tag0" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS     rc = GT_OK;
    GT_HW_DEV_NUM hwDevNum;

    /* AUTODOC: SETUP CONFIGURATION: */

    dsaTestName = tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_E;
    /* AUTODOC: set ingress portIdx = 1, egress portIdx = 2 */

    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 1;
    prvTgfEgrPortIdx = 2;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Outer TAG1 Inner TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    /* set srcHwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_dsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_eDsaWordsVfd[0].patternPtr[0], hwDevNum);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG1 with
*         ethertype = 0x98a8, vlan = 0 and TAG0 with ethertype = 0x9100,
*         vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* AUTODOC: GENERATE TRAFFIC: */

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x98;
    vfdArray[0].patternPtr[1] = 0xa8;
    vfdArray[0].patternPtr[2] = (GT_U8)(currentIngressDefaultVid1 >> 8);
    vfdArray[0].patternPtr[3] = (GT_U8)(currentIngressDefaultVid1 >> 0);

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x91;
    vfdArray[1].patternPtr[1] = 0x00;
    vfdArray[1].patternPtr[2] = 0x00;
    vfdArray[1].patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 0 */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Outer Tag1, Inner Tag0" for all ports in VLAN.
*         - Set PVID to 5 - for untagged packet
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationSet
(
    GT_VOID
)
{
    GT_U16      etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 1, egress portIdx = 3 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 1;
    prvTgfEgrPortIdx = 3;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Outer TAG1 Inner TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* store default PVID */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], &prvTgfPvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d", prvTgfPortsArray[INGR_PORT_IDX_MAC]);

    /* AUTODOC: set PVID to 5 for untagged packets */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Output packet has two tags in the following order: TAG1 with
*         ethertype = 0x98a8, vlan = 0 and TAG0 with ethertype = 0x9100,
*         vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with no tag */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x98;
    vfdArray[0].patternPtr[1] = 0xa8;
    vfdArray[0].patternPtr[2] = (GT_U8)(currentIngressDefaultVid1 >> 8);
    vfdArray[0].patternPtr[3] = (GT_U8)(currentIngressDefaultVid1 >> 0);

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x91;
    vfdArray[1].patternPtr[1] = 0x00;
    vfdArray[1].patternPtr[2] = 0x20; /* VPT is 1 because of CPSS default */
    vfdArray[1].patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoUntag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 0 */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
}

/**
* @internal prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    /* AUTODOC: restore default PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23 and
*         tagging command "Push Tag0" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 0, egress portIdx = 1 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 0;
    prvTgfEgrPortIdx = 1;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Push TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 10
*         Success Criteria:
*         Output packet has tree tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG0 with ethertype = 0x8100,
*         vlan = 5, TAG1 with ethertype = 0x88a8, vlan = 10.
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdDoubleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[3];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 10 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x05;

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x81;
    vfdArray[1].patternPtr[1] = 0x00;
    vfdArray[1].patternPtr[2] = 0x00;
    vfdArray[1].patternPtr[3] = 0x05;

    vfdArray[2].cycleCount = 4;
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].modeExtraInfo = 0;
    vfdArray[2].offset = 20;
    vfdArray[2].patternPtr[0] = 0x88;
    vfdArray[2].patternPtr[1] = 0xa8;
    vfdArray[2].patternPtr[2] = 0x00;
    vfdArray[2].patternPtr[3] = 0x0a;


    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 3, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
    /* AUTODOC:   TAG0 with ethertype = 0x8100, vid = 5 */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 10 */
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Push Tag0" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 0, egress portIdx = 2 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 0;
    prvTgfEgrPortIdx = 2;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Push TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet has two tags in the following order: TAG0 with
*         ethertype = 0x9100, vlan = 5 and TAG0 with ethertype = 0x8100,
*         vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdSingleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x05;

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x81;
    vfdArray[1].patternPtr[1] = 0x00;
    vfdArray[1].patternPtr[2] = 0x00;
    vfdArray[1].patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
    /* AUTODOC:   TAG1 with ethertype = 0x8100, vid = 5 */
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Push Tag0" for all ports in VLAN.
*         - Set PVID to 5 - for untagged packet
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationSet
(
    GT_VOID
)
{
    GT_U16      etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                   TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS   rc = 0;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 0, egress portIdx = 3 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 0;
    prvTgfEgrPortIdx = 3;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Push TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* store default PVID */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], &prvTgfPvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d", prvTgfPortsArray[INGR_PORT_IDX_MAC]);

    /* AUTODOC: set PVID to 5 for untagged packets */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdUntagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Output packet has one tag: TAG0 with ethertype = 0x9100, vlan = 5
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdUntagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with no tag */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x20; /* VPT is 1 because of CPSS default */
    vfdArray[0].patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoUntag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
}

/**
* @internal prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    /* AUTODOC: restore default PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfPortsArray[INGR_PORT_IDX_MAC], prvTgfPvid);
}

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23 and
*         tagging command "Pop Outer Tag" for all ports in VLAN.
*         - Build VLAN Entry with VID = 10 and portsMember = 0, 8, 18, 23 and
*         tagging command "UNTAGGED" for all ports in VLAN to be sure that
*         decision is according to VID = 5.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};
    GT_STATUS   rc = 0;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 3, egress portIdx = 0 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 3;
    prvTgfEgrPortIdx = 0;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Pop Outer TAG" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);

    /* AUTODOC: create VLAN 10 with all ports, "Untagged" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID1_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);


    /* AUTODOC: override egress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* For SIP5 devices "Pop Outer Tag" requires the egress
       and ingress TPID tables to be configured identically */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                       0,
                                       etherTypeArr[0]);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                     0, etherTypeArr[0]);

        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                       1,
                                       etherTypeArr[1]);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                     1, etherTypeArr[1]);
    }

}

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 10
*         Success Criteria:
*         Output packet has one tag: TAG1 with
*         ethertype = 0x98a8,vlan = 10.
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 10 */

    /* the Pop Outer command does not changes Inner VLAN tag including TPID.
      So packet will enter to captured port and will be recognized as:
      TAG0:  absent, use VLAN assignment
      TAG1:  ethertype = 0x88a8, vlan = 10
      Captured (mirrored to CPU) packet will change TAG1 TPID to 0x8100 */
    vfdArray[0].patternPtr[0] = 0x81;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x0a;

    if(GT_FALSE ==
       prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT2_E | UTF_LION2_E | UTF_LION_E | UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* the listed devices supports this API */
#ifdef CHX_FAMILY
        GT_STATUS   rc;
        GT_BOOL     forceNewDsaToCpu;

        rc = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(prvTgfDevNum,&forceNewDsaToCpu);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChBrgVlanForceNewDsaToCpuEnableGet: failed \n");

        if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)||
            /* the CPU will get the vlan tags outside from the DSA */
            /* so the 'orig ethertype of outer vlan' is preserved. */

            /* !!! the capture is set to not recognize TPIDs on ingress !!!*/
            forceNewDsaToCpu == GT_TRUE)
        {
            vfdArray[0].patternPtr[0] = 0x88;
            vfdArray[0].patternPtr[1] = 0xa8;
        }
#endif /*CHX_FAMILY*/
    }

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 10 */
}

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);

    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID1_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8
*         - Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and tagging command "Pop Outer Tag" for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[2] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                              TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 3, egress portIdx = 1 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 3;
    prvTgfEgrPortIdx = 1;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, TAG1 is Ethertype1 = 0x88a8 */

    /* AUTODOC: create VLAN 5 with all ports, "Pop Outer TAG" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E,
                                      2, etherTypeArr,
                                      PRV_TGF_ETHERTYPE_0_CNS,
                                      PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Success Criteria:
*         Output packet is untagged
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 6;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;

    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPayloadDataArr, TGF_VFD_PATTERN_BYTES_NUM_CNS);

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has no any tags */
}

/**
* @internal prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 2,
                                            PRV_TGF_ETHERTYPE_0_CNS,
                                            PRV_TGF_ETHERTYPE_1_CNS);
}

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to:
*         Ethertype0 = 0x8100,
*         Ethertype1 = 0x88a8,
*         Ethertype2 = 0x5000,
*         Ethertype3 = 0xA0A0,
*         Ethertype4 = 0x5050,
*         Ethertype5 = 0x2525,
*         Ethertype6 = 0x5555,
*         Ethertype7 = 0xAAAA,
*         - Set ingress TPID select to: TAG0 is Ethertype0, Ethertype3, Ethertype5,
*         Ethertype6, Ethertype7, TAG1 is Ethertype1, Ethertype2, Ethertype4.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Build VLAN Entry with VID = 25 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Build VLAN Entry with VID = 4000 and portsMember = 0, 8, 18, 23
*         and "TAG0" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[8] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS, TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,
                               0x5000, 0xA0A0, 0x5050, 0x2525, 0x5555, 0xAAAA};
    GT_U8 etherTypeTag0 = PRV_TGF_ETHERTYPE_0_CNS | PRV_TGF_ETHERTYPE_3_CNS |
                          PRV_TGF_ETHERTYPE_5_CNS | PRV_TGF_ETHERTYPE_6_CNS |
                          PRV_TGF_ETHERTYPE_7_CNS;
    GT_U8 etherTypeTag1 = PRV_TGF_ETHERTYPE_1_CNS | PRV_TGF_ETHERTYPE_2_CNS |
                          PRV_TGF_ETHERTYPE_4_CNS;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 3, egress portIdx = 2 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 3;
    prvTgfEgrPortIdx = 2;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, Ethertype3 = 0xA0A0, Ethertype5 = 0x2525, Ethertype6 = 0x5555, Ethertype7 = 0xAAAA */
    /* AUTODOC:   TAG1 is Ethertype1 = 0x88a8, Ethertype2 = 0x5000, Ethertype4 = 0x5050 */

    /* AUTODOC: create VLAN 5 with all ports, "TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                                      8, etherTypeArr,
                                      etherTypeTag0, etherTypeTag1);

    /* AUTODOC: create VLAN 25 with all ports, "TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID2_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                                      8, etherTypeArr,
                                      etherTypeTag0, etherTypeTag1);

    /* AUTODOC: create VLAN 4000 with all ports, "TAG0" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID3_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                                      8, etherTypeArr,
                                      etherTypeTag0, etherTypeTag1);
}

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment1TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:03,
*         macSa = 00:00:00:00:00:13,
*         TAG0: ethertype = 0x8100, vlan = 25
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:04,
*         macSa = 00:00:00:00:00:14,
*         TAG0: ethertype = 0x8100, vlan = 4000
*         Success Criteria:
*         First packet have TAG0: ethertype =0x9100, vlan = 5
*         Second packet have TAG0: ethertype =0x9100, vlan = 25
*         Third packet have TAG0: ethertype =0x9100, vlan = 4000
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment1TrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc        = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0xA0A0, vlan = 25 */

    /* set TAG0 in Packet */
    prvTgfPacketVlanTag0Part.etherType = 0xA0A0;
    prvTgfPacketVlanTag0Part.vid       = PRV_TGF_VLANID2_CNS;

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].patternPtr[3] = 0x19;

    if (prvCpssDxChPortRemotePortCheck(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC]))
    {
        /* Only one TPID is supported at Remote ports */
        /* Set ingress TPID select for TAG0 */
        rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                             prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                             CPSS_VLAN_ETHERTYPE0_E,
                                             PRV_TGF_ETHERTYPE_3_CNS);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE0_E, PRV_TGF_ETHERTYPE_3_CNS);
    }
    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 1, vfdArray, 1);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 25 */

    /* remove casting after adding code to support new eArch */
    CPSS_TBD_BOOKMARK_EARCH

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0xAAAA, vlan = 4000 */

    /* set TAG0 in Packet */
    if(GT_TRUE == prvTgfCommonIsDeviceForce((GT_U8)portInterface.devPort.hwDevNum))
    {
        prvTgfPacketVlanTag0Part.etherType = 0x2525;/*use value not in index 6 or 7 0xAAAA */ ;
    }
    else
    {
        prvTgfPacketVlanTag0Part.etherType = 0xAAAA;
    }

    prvTgfPacketVlanTag0Part.vid       = PRV_TGF_VLANID3_CNS;

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].patternPtr[2] = 0x0F;
    vfdArray[0].patternPtr[3] = 0xA0;

    if (prvCpssDxChPortRemotePortCheck(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC]))
    {
        /* Only one TPID is supported at Remote ports */
        /* Set ingress TPID select for TAG0 */
        rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                             prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                             CPSS_VLAN_ETHERTYPE0_E,
                                             PRV_TGF_ETHERTYPE_7_CNS);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE0_E, PRV_TGF_ETHERTYPE_7_CNS);
    }
    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoSingleTag, 1, vfdArray, 2);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 4000 */
}

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationRestore
(
    GT_VOID
)
{
    GT_U8 etherTypeTag0 = PRV_TGF_ETHERTYPE_0_CNS | PRV_TGF_ETHERTYPE_3_CNS |
                          PRV_TGF_ETHERTYPE_5_CNS | PRV_TGF_ETHERTYPE_6_CNS |
                          PRV_TGF_ETHERTYPE_7_CNS;
    GT_U8 etherTypeTag1 = PRV_TGF_ETHERTYPE_1_CNS | PRV_TGF_ETHERTYPE_2_CNS |
                          PRV_TGF_ETHERTYPE_4_CNS;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore default TAG0 in Packet */
    prvTgfPacketVlanTag0Part.etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
    prvTgfPacketVlanTag0Part.vid       = PRV_TGF_VLANID_CNS;

    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 8,
                                            etherTypeTag0, etherTypeTag1);

    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID2_CNS, 8,
                                            etherTypeTag0, etherTypeTag1);

    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID3_CNS, 8,
                                            etherTypeTag0, etherTypeTag1);
}

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to:
*         Ethertype0 = 0x8100,
*         Ethertype1 = 0x88a8,
*         Ethertype2 = 0x5000,
*         Ethertype3 = 0xA0A0,
*         Ethertype4 = 0x5050,
*         Ethertype5 = 0x2525,
*         Ethertype6 = 0x5555,
*         Ethertype7 = 0xAAAA,
*         - Set ingress TPID select to: TAG0 is Ethertype0, Ethertype3, Ethertype5,
*         Ethertype6, Ethertype7, TAG1 is Ethertype1, Ethertype2, Ethertype4.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "TAG1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[8] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS, TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,
                               0x5000, 0xA0A0, 0x5050, 0x2525, 0x5555, 0xAAAA};
    GT_U8 etherTypeTag0 = PRV_TGF_ETHERTYPE_0_CNS | PRV_TGF_ETHERTYPE_3_CNS |
                          PRV_TGF_ETHERTYPE_5_CNS | PRV_TGF_ETHERTYPE_6_CNS |
                          PRV_TGF_ETHERTYPE_7_CNS;
    GT_U8 etherTypeTag1 = PRV_TGF_ETHERTYPE_1_CNS | PRV_TGF_ETHERTYPE_2_CNS |
                          PRV_TGF_ETHERTYPE_4_CNS;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 2, egress portIdx = 0 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 2;
    prvTgfEgrPortIdx = 0;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, Ethertype3 = 0xA0A0, Ethertype5 = 0x2525, Ethertype6 = 0x5555, Ethertype7 = 0xAAAA */
    /* AUTODOC:   TAG1 is Ethertype1 = 0x88a8, Ethertype2 = 0x5000, Ethertype4 = 0x5050 */

    /* AUTODOC: create VLAN 5 with all ports, "TAG1" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
                                      8, etherTypeArr,
                                      etherTypeTag0, etherTypeTag1);
}

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment2TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 25
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:03,
*         macSa = 00:00:00:00:00:13,
*         TAG0: ethertype = 0xA0A0, vlan = 25
*         TAG1: ethertype = 0x5050, vlan = 100
*         Success Criteria:
*         First packet have TAG1: ethertype =0x98a8, vlan = 25
*         Second packet have TAG1: ethertype =0x98a8, vlan = 100
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment2TrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc        = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 25 */

    /* set TAG1 in Packet */
    prvTgfPacketVlanTag1Part.vid = PRV_TGF_VLANID2_CNS;

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x98;
    vfdArray[0].patternPtr[1] = 0xa8;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x19;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 1, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 25 */

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0xA0A0, vlan = 25 */
    /* AUTODOC:   TAG1: ethertype = 0x5050, vlan = 100 */

    /* set TAG0 in Packet */
    prvTgfPacketVlanTag0Part.etherType = 0xA0A0;

    /* set TAG1 in Packet */
    prvTgfPacketVlanTag1Part.etherType = 0x5050;
    prvTgfPacketVlanTag1Part.vid       = PRV_TGF_VLANID4_CNS;

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].patternPtr[3] = 0x64;

    if (prvCpssDxChPortRemotePortCheck(prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC]))
    {
        /* Only one TPID is supported at Remote ports */
        /* Set ingress TPID select for TAG0 */
        rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                             prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                             CPSS_VLAN_ETHERTYPE0_E,
                                             PRV_TGF_ETHERTYPE_3_CNS);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[INGR_PORT_IDX_MAC],
                                     CPSS_VLAN_ETHERTYPE0_E, PRV_TGF_ETHERTYPE_3_CNS);
    }
    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 1, vfdArray, 1);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 100 */
}

/**
* @internal prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationRestore
(
    GT_VOID
)
{
    GT_U8 etherTypeTag0 = PRV_TGF_ETHERTYPE_0_CNS | PRV_TGF_ETHERTYPE_3_CNS |
                          PRV_TGF_ETHERTYPE_5_CNS | PRV_TGF_ETHERTYPE_6_CNS |
                          PRV_TGF_ETHERTYPE_7_CNS;
    GT_U8 etherTypeTag1 = PRV_TGF_ETHERTYPE_1_CNS | PRV_TGF_ETHERTYPE_2_CNS |
                          PRV_TGF_ETHERTYPE_4_CNS;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore default TAG0 in Packet */
    prvTgfPacketVlanTag0Part.etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
    prvTgfPacketVlanTag0Part.vid       = PRV_TGF_VLANID_CNS;

    /* restore default TAG1 in Packet */
    prvTgfPacketVlanTag1Part.etherType = TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS;
    prvTgfPacketVlanTag1Part.vid       = PRV_TGF_VLANID1_CNS;

    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 8,
                                            etherTypeTag0, etherTypeTag1);
}

/**
* @internal prvTgfBrgVlanManipulationChangeEgressTpidConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set ingress TPID table to:
*         Ethertype0 = 0x8100,
*         Ethertype1 = 0x88a8,
*         Ethertype2 = 0x5000,
*         Ethertype3 = 0xA0A0,
*         Ethertype4 = 0x5050,
*         Ethertype5 = 0x2525,
*         Ethertype6 = 0x5555,
*         Ethertype7 = 0xAAAA,
*         - Set ingress TPID select to: TAG0 is Ethertype0, Ethertype3, Ethertype5,
*         Ethertype6, Ethertype7, TAG1 is Ethertype1, Ethertype2, Ethertype4.
*         - Build VLAN Entry with VID = 5 and portsMember = 0, 8, 18, 23
*         and "Outer Tag0, Inner Tag1" tagging command for all ports in VLAN.
*         - Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8
*         - Set egress TPID select to: TAG0 is Ethertype0, TAG1 is Ethertype1.
*/
GT_VOID prvTgfBrgVlanManipulationChangeEgressTpidConfigurationSet
(
    GT_VOID
)
{
    GT_U16 etherTypeArr[8] = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS, TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,
                               0x5000, 0xA0A0, 0x5050, 0x2525, 0x5555, 0xAAAA};
    GT_U8 etherTypeTag0 = PRV_TGF_ETHERTYPE_0_CNS | PRV_TGF_ETHERTYPE_3_CNS |
                          PRV_TGF_ETHERTYPE_5_CNS | PRV_TGF_ETHERTYPE_6_CNS |
                          PRV_TGF_ETHERTYPE_7_CNS;
    GT_U8 etherTypeTag1 = PRV_TGF_ETHERTYPE_1_CNS | PRV_TGF_ETHERTYPE_2_CNS |
                          PRV_TGF_ETHERTYPE_4_CNS;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: set ingress portIdx = 2, egress portIdx = 1 */
    /* change ingress and egress ports */
    prvTgfIngrPortIdx = 2;
    prvTgfEgrPortIdx = 1;

    /* AUTODOC: set ingress TPID table: */
    /* AUTODOC:   TAG0 is Ethertype0 = 0x8100, Ethertype3 = 0xA0A0, Ethertype5 = 0x2525, Ethertype6 = 0x5555, Ethertype7 = 0xAAAA */
    /* AUTODOC:   TAG1 is Ethertype1 = 0x88a8, Ethertype2 = 0x5000, Ethertype4 = 0x5050 */

    /* AUTODOC: create VLAN 5 with all ports, "Outer TAG0 Inner TAG1" cmd */
    prvTgfBrgVlanManipulationTestInit(PRV_TGF_VLANID_CNS,
                                      PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,
                                      8, etherTypeArr,
                                      etherTypeTag0, etherTypeTag1);
}

/**
* @internal prvTgfBrgVlanManipulationChangeEgressTpidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 100
*         Additional configuration:
*         Set egress TPID table to: Ethertype0 = 0x2222 and Ethertype1 = 0x3333
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:02,
*         macSa = 00:00:00:00:00:12,
*         TAG0: ethertype = 0x8100, vlan = 5
*         TAG1: ethertype = 0x88a8, vlan = 100
*         Success Criteria:
*         First packet have TAG0: ethertype =0x9100, vlan = 5;
*         TAG1: ethertype =0x98a8, vlan = 100;
*         Second packet have TAG0: ethertype =0x2222, vlan = 5;
*         TAG1: ethertype =0x3333, vlan = 100;
*/
GT_VOID prvTgfBrgVlanManipulationChangeEgressTpidTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];
    GT_STATUS               rc = GT_OK;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_MAC];

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 100 */

    /* set TAG1 in Packet */
    prvTgfPacketVlanTag1Part.vid = PRV_TGF_VLANID4_CNS;

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].cycleCount = 4;
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].patternPtr[0] = 0x91;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x05;

    vfdArray[1].cycleCount = 4;
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 16;
    vfdArray[1].patternPtr[0] = 0x98;
    vfdArray[1].patternPtr[1] = 0xa8;
    vfdArray[1].patternPtr[2] = 0x00;
    vfdArray[1].patternPtr[3] = 0x64;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x9100, vid = 5 */
    /* AUTODOC:   TAG1 with ethertype = 0x98a8, vid = 100 */

    /* remove casting after adding code to support new eArch */
    CPSS_TBD_BOOKMARK_EARCH

    if(GT_TRUE == prvTgfCommonIsDeviceForce((GT_U8)portInterface.devPort.hwDevNum))
    {
        /* 'free' (set 0x8100) entries 6,7 in the ingress table for entries for the egress :
        for cascade,ring ports */

        prvTgfCommonUseLastTpid(GT_TRUE);
        /* set additional configuration */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,
                                       6, TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
        prvTgfCommonUseLastTpid(GT_FALSE);

        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                     6, TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

        prvTgfCommonUseLastTpid(GT_TRUE);
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,
                                       7, TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
        prvTgfCommonUseLastTpid(GT_FALSE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                     prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                     7, TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    }


    /* set additional configuration */
    /* AUTODOC: set egress TPID table to: Ethertype0 = 0x2222 */
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                   0, 0x2222);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                 prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                 0, 0x2222);

    /* AUTODOC: set egress TPID table to: Ethertype1 = 0x3333 */
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,
                                   1, 0x3333);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %d, %d, %d",
                                 prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                                 1, 0x3333);

    /* AUTODOC: setup packet with: */
    /* AUTODOC:   TAG0: ethertype = 0x8100, vlan = 5 */
    /* AUTODOC:   TAG1: ethertype = 0x88a8, vlan = 10 */

    /* set VFD array of expected fields value in received packet*/
    vfdArray[0].patternPtr[0] = 0x22;
    vfdArray[0].patternPtr[1] = 0x22;

    vfdArray[1].patternPtr[0] = 0x33;
    vfdArray[1].patternPtr[1] = 0x33;

    /* send packet */
    prvTgfBrgVlanManipulationTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag, 2, vfdArray, 0);
    /* AUTODOC: verify output packet has: */
    /* AUTODOC:   TAG0 with ethertype = 0x2222, vid = 5 */
    /* AUTODOC:   TAG1 with ethertype = 0x3333, vid = 100 */
}

/**
* @internal prvTgfBrgVlanManipulationChangeEgressTpidConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanManipulationChangeEgressTpidConfigurationRestore
(
    GT_VOID
)
{
    GT_U8 etherTypeTag0 = PRV_TGF_ETHERTYPE_0_CNS | PRV_TGF_ETHERTYPE_3_CNS |
                          PRV_TGF_ETHERTYPE_5_CNS | PRV_TGF_ETHERTYPE_6_CNS |
                          PRV_TGF_ETHERTYPE_7_CNS;
    GT_U8 etherTypeTag1 = PRV_TGF_ETHERTYPE_1_CNS | PRV_TGF_ETHERTYPE_2_CNS |
                          PRV_TGF_ETHERTYPE_4_CNS;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore default TAG0 in Packet */
    prvTgfPacketVlanTag0Part.etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
    prvTgfPacketVlanTag0Part.vid       = PRV_TGF_VLANID_CNS;

    /* restore default TAG1 in Packet */
    prvTgfPacketVlanTag1Part.etherType = TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS;
    prvTgfPacketVlanTag1Part.vid       = PRV_TGF_VLANID1_CNS;

    prvTgfBrgVlanManipulationTestTableReset(PRV_TGF_VLANID_CNS, 8,
                                            etherTypeTag0, etherTypeTag1);
}

/**
* @internal prvTgfBrgVlanTableWriteWorkaroundTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send full-wire speed traffic
*         Additional configuration:
*         Write all existing VLAN entries in the loop
*         Success Criteria:
*         There are no discarded egress packets under traffic.
*/
GT_VOID prvTgfBrgVlanTableWriteWorkaroundTrafficGenerate
(
    GT_VOID
)
{
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  dummyPortsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_INFO_STC           dummyVlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  dummyPortsTaggingCmd;
    GT_STATUS                           rc = GT_OK;
    GT_U32                              iter = 0;
    GT_U16                              vlanIdIter = 0;
    GT_BOOL                             isValid = GT_FALSE;
    CPSS_PORT_EGRESS_CNTR_STC           egrCntr = {0};

    /* Init section */
    cpssOsMemSet(&dummyVlanInfo, 0, sizeof(dummyVlanInfo));
    cpssOsMemSet(&dummyVlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    cpssOsMemSet(&dummyPortsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&dummyPortsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);

    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: reset egress counters */
    rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");

    /* AUTODOC: configure and generate FWS: */
    /* AUTODOC:   force link UP and configure maximal speed on all ports */
    /* AUTODOC:   create FDB and VLAN loop for each speed group */
    /* AUTODOC:   enable tail drop mode */
    /* AUTODOC:   configure WireSpeed mode */
    /* AUTODOC: for each speed group and port send packet with: */
    /* AUTODOC:   DA=00:AA:BB:CC:DD:11, SA=00:88:99:33:44:11 */
    rc = prvTgfFWSLoopConfigTrafficGenerate(PRV_TGF_VLANID_CNS, &prvTgfPacketFWSInfo, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", PRV_TGF_VLANID_CNS);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: reset egress counters after traffic generation for SIP_5_20 devices */
        /*  Traffic generation result in multiple link change events and egress filter reconfiguration.
           Some packets will be dropped because of it and increment brgEgrFilterDisc. Need to clean counters. */
        rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");
    }

    utfPrintKeepAlive();

    /* set additional configuration */
    for (iter = 0; iter < PRV_TGF_WA_ITER_NUM_CNS; ++iter)
    {
        for (vlanIdIter = PRV_TGF_VLANID_CNS; vlanIdIter < prvTgfFWSPortsNumGet()/* number of ports equals to number of VLANs in FWS test */; ++vlanIdIter)
        {
            /* AUTODOC: read VLAN entries - from 5 to number of ports */
            rc = prvTgfBrgVlanEntryRead(prvTgfDevNum,
                                        vlanIdIter,
                                        &portsMembers,
                                        &portsTagging,
                                        &isValid,
                                        &vlanInfo,
                                        &portsTaggingCmd);


            /*********************************************************************/
            /* AUTODOC: write dummy VLAN entry (105) in order to reset registers */
            /*********************************************************************/
            rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                         PRV_TGF_DUMMY_VLANID_CNS,
                                         &dummyPortsMembers,
                                         NULL,
                                         &dummyVlanInfo,
                                         &dummyPortsTaggingCmd);

            /* AUTODOC: write previous VLAN entries */
            rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                         vlanIdIter,
                                         &portsMembers,
                                         &portsTagging,
                                         &vlanInfo,
                                         &portsTaggingCmd);
        }
    }

    utfPrintKeepAlive();

    rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");

    /* AUTODOC: verify number of Bridge Egress filtered packets is 0 */
    rc = (egrCntr.brgEgrFilterDisc == 0) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "found non-zero discarded packets %d", egrCntr.brgEgrFilterDisc);

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

}

/**
* @internal prvTgfBrgVlanTableWriteWorkaroundConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanTableWriteWorkaroundConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: reset FWS test configuration: */
    /* AUTODOC:   disable forcing on all ports */
    /* AUTODOC:   restore default VLAN for all ports */
    /* AUTODOC:   clean VLAN table */
    /* AUTODOC:   restore FC mode */
    prvTgfFWSRestore();

    /* invalidate dummy vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_DUMMY_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DUMMY_VLANID_CNS);

}

/**
* @internal prvTgfBrgStgTableWriteWorkaroundTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send full-wire speed traffic
*         Write dummy STG entry with groupId = 1 configured with block command
*         Write default STG entry with groupId = 0 without changes
*         Success Criteria:
*         There are no discarded egress packets under traffic.
*/
GT_VOID prvTgfBrgStgTableWriteWorkaroundTrafficGenerate
(
    GT_VOID
)
{
    /* Default and dummy STP port configuration */
    GT_U32                      stpEntryArr [PRV_TFG_STG_ENTRY_SIZE_CNS] = {0};
    GT_U32                      dummyStpEntryArr [PRV_TFG_STG_ENTRY_SIZE_CNS] = {0};
    GT_STATUS                   rc = GT_OK;
    GT_U32                      iter = 0;
    CPSS_PORT_EGRESS_CNTR_STC   egrCntr = {0};

    /* Init section */
    /*Set block/learning mode */
    dummyStpEntryArr[0] = 0x55555555;
    dummyStpEntryArr[1] = 0x55555555;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: reset egress counters */
    rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");

    /* AUTODOC: configure and generate FWS: */
    /* AUTODOC:   force link UP and configure maximal speed on all ports */
    /* AUTODOC:   create FDB and VLAN loop for each speed group */
    /* AUTODOC:   enable tail drop mode */
    /* AUTODOC:   configure WireSpeed mode */
    /* AUTODOC: for each speed group and port send packet with: */
    /* AUTODOC:   DA=00:AA:BB:CC:DD:11, SA=00:88:99:33:44:11 */
    rc = prvTgfFWSLoopConfigTrafficGenerate(PRV_TGF_VLANID_CNS, &prvTgfPacketFWSInfo, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", PRV_TGF_VLANID_CNS);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: reset egress counters after traffic generation for SIP_5_20 devices */
        /*  Traffic generation result in multiple link change events and egress filter reconfiguration.
           Some packets will be dropped because of it and increment brgEgrFilterDisc. Need to clean counters. */
        rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");
    }

    utfPrintKeepAlive();

    /* set additional configuration */
    for (iter = 0; iter < PRV_TGF_WA_ITER_NUM_CNS; ++iter)
    {
        /* AUTODOC: read STG entry with index 0 */
        /* Suppose that default VLAN entry is configured with stgId = 0 */
        rc = prvTgfBrgStpEntryRead(prvTgfDevNum,
                                   PRV_TFG_STG_DEFAULT_ENTRY_IDX_CNS,
                                   stpEntryArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpEntryRead: stpId %d", PRV_TFG_STG_DEFAULT_ENTRY_IDX_CNS);

        /******************************************************************************/
        /* AUTODOC: write dummy STG (with idx = 1) entry in order to change registers */
        /******************************************************************************/
        rc = prvTgfBrgStpEntryWrite(prvTgfDevNum,
                                    PRV_TFG_STG_DUMMY_ENTRY_IDX_CNS,
                                    dummyStpEntryArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpEntryWrite: stpId %d", PRV_TFG_STG_DUMMY_ENTRY_IDX_CNS);

        /* AUTODOC: write STG entry with index 0 */
        rc = prvTgfBrgStpEntryWrite(prvTgfDevNum,
                                    PRV_TFG_STG_DEFAULT_ENTRY_IDX_CNS,
                                    stpEntryArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpEntryWrite: stpId %d", PRV_TFG_STG_DEFAULT_ENTRY_IDX_CNS);
    }

    utfPrintKeepAlive();

    rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");

    /* AUTODOC: verify number of Bridge Egress filtered packets is 0 */
    rc = (egrCntr.brgEgrFilterDisc == 0) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "found non-zero discarded packets %d", egrCntr.brgEgrFilterDisc);

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

}

/**
* @internal prvTgfBrgStgTableWriteWorkaroundConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgStgTableWriteWorkaroundConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          dummyStpEntryArr[PRV_TFG_STG_ENTRY_SIZE_CNS];

    /* AUTODOC: RESTORE CONFIGURATION: */
    cpssOsMemSet(dummyStpEntryArr, 0, PRV_TFG_STG_ENTRY_SIZE_CNS * sizeof(GT_U32));

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: reset FWS test configuration: */
    /* AUTODOC:   disable forcing on all ports */
    /* AUTODOC:   restore default VLAN for all ports */
    /* AUTODOC:   clean VLAN table */
    /* AUTODOC:   restore FC mode */
    prvTgfFWSRestore();

    /* AUTODOC: invalidate dummy STG entry (and reset STG entry) */
    rc = prvTgfBrgStpEntryWrite(prvTgfDevNum,
                                PRV_TFG_STG_DUMMY_ENTRY_IDX_CNS,
                                dummyStpEntryArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpEntryWrite: stpId %d", PRV_TFG_STG_DUMMY_ENTRY_IDX_CNS);

}

extern GT_U32 prvTgfPortMaxSpeedForce_xcat3AllowXGPortsSR_LR(IN GT_U32   allow);

/**
* @internal prvTgfFdbTableReadUnderWireSpeedTrafficGenerate function
* @endinternal
*
* @brief   Verify that there are no discarded packets during read access by the
*         CPU to the FDB.
*         Run full-wire speed traffic test on 10G ports (4 ports, 4 VLAN entries)
*         static mac addresses that relate to macSA,DA
*         Check bridge drop counters - must be 0
*/
GT_VOID prvTgfFdbTableReadUnderWireSpeedTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      iter = 0;
    GT_U32                      fdbIter;
    GT_BOOL                      tmp_valid;
    GT_BOOL                      tmp_skip;
    GT_BOOL                      tmp_aged;
    GT_HW_DEV_NUM                tmp_hwDevNum;
    PRV_TGF_BRG_MAC_ENTRY_STC    tmp_macEntry;
    CPSS_BRIDGE_INGRESS_CNTR_STC        ingressCntr;
    GT_U32    packetSize = PRV_TGF_PACKET_FWS_CRC_LEN_CNS; /* packet size */
    GT_U32    portRxRateArr[CPSS_MAX_PORTS_NUM_CNS];       /* rate of packets per port */
    GT_BOOL   cpuExpectedToGetTimeout = PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum) ? GT_FALSE : GT_TRUE;
    GT_U32    timeoutCounter;
    GT_U32    timeoutCounter_maxAllowed = 2;
    GT_U32    orig_maxIterationsOfBusyWait = PRV_CPSS_PP_MAC(prvTgfDevNum)->maxIterationsOfBusyWait;
    GT_U32    orig_xcat3AllowXGPortsSR_LR = prvTgfPortMaxSpeedForce_xcat3AllowXGPortsSR_LR(1);

    cpssOsMemSet(portRxRateArr, 0, sizeof(portRxRateArr));

    /* AUTODOC: get Bridge ingress counters: to reset it */
    rc = prvTgfBrgCntBridgeIngressCntrsGet(prvTgfDevNum, PRV_TGF_BRG_CNTR_SET_ID_0_E, &ingressCntr);

    /* AUTODOC: configure and generate FWS: */
    /* AUTODOC:   force link UP and configure maximal speed on all ports */
    /* AUTODOC:   create FDB and VLAN loop for each speed group */
    /* AUTODOC:   enable tail drop mode */
    /* AUTODOC:   configure WireSpeed mode */
    /* AUTODOC: for each speed group and port send packet with: */
    /* AUTODOC:   DA=00:AA:BB:CC:DD:11, SA=00:88:99:33:44:11 */
    rc = prvTgfFWSLoopConfigTrafficGenerate(PRV_TGF_VLANID_CNS, &prvTgfPacketFWSInfo, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", PRV_TGF_VLANID_CNS);

    utfPrintKeepAlive();

    /* NOTE: the check of FWS traffic will start by doing sleep of 800 ms
       (at prvTgfFWSTrafficCheck(...))*/

    /* check FWS traffic */
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("WARNING : prvTgfFWSTrafficCheck failed !!!rc[%d] : packetSize %d \n",
            rc , packetSize);
    }

    if(cpuExpectedToGetTimeout == GT_TRUE)
    {
        /* reduce the time that make CPSS to understand that we got timeout */
        PRV_CPSS_PP_MAC(prvTgfDevNum)->maxIterationsOfBusyWait /= 20;
    }

    timeoutCounter = 0;
    /* start stress the FDB by reading from it */
    for (iter = 0; iter < PRV_TGF_WA_ITER_NUM_CNS; ++iter)
    {
        for (fdbIter = 0; fdbIter < _1K ; ++fdbIter)
        {
            rc = prvTgfBrgFdbMacEntryRead(fdbIter, &tmp_valid, &tmp_skip, &tmp_aged,
                                          &tmp_hwDevNum, &tmp_macEntry);
            if(rc == GT_TIMEOUT)
            {
                PRV_UTF_LOG1_MAC("\n == prvTgfBrgFdbMacEntryRead: GT_TIMEOUT at index %d \n", fdbIter);
                timeoutCounter++;
                if(timeoutCounter > timeoutCounter_maxAllowed)
                {
                    break;
                }
            }

            if(cpuExpectedToGetTimeout == GT_TRUE && rc == GT_TIMEOUT)
            {
                /* we expect timeout because the PP not give priority */
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryRead: failed at index [%d]", fdbIter);
            }
        }

        if(timeoutCounter > timeoutCounter_maxAllowed)
        {
            break;
        }
    }

    PRV_CPSS_PP_MAC(prvTgfDevNum)->maxIterationsOfBusyWait = orig_maxIterationsOfBusyWait;
    prvTgfPortMaxSpeedForce_xcat3AllowXGPortsSR_LR(orig_xcat3AllowXGPortsSR_LR);

    utfPrintKeepAlive();

    /* AUTODOC: get Bridge ingress counters: verify no drops */
    rc = prvTgfBrgCntBridgeIngressCntrsGet(prvTgfDevNum, PRV_TGF_BRG_CNTR_SET_ID_0_E, &ingressCntr);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, ingressCntr.gtBrgVlanIngFilterDisc,
        "gtBrgVlanIngFilterDisc : found non-zero discarded packets [%d]", ingressCntr.gtBrgVlanIngFilterDisc);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, ingressCntr.gtBrgSecFilterDisc,
        "gtBrgSecFilterDisc : found non-zero discarded packets [%d]", ingressCntr.gtBrgSecFilterDisc);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, ingressCntr.gtBrgLocalPropDisc,
        "gtBrgLocalPropDisc : found non-zero discarded packets [%d]", ingressCntr.gtBrgLocalPropDisc);

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

}


/**
* @internal prvTgfFdbTableReadUnderWireSpeedConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbTableReadUnderWireSpeedConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;


    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: reset FWS test configuration: */
    /* AUTODOC:   disable forcing on all ports */
    /* AUTODOC:   restore default VLAN for all ports */
    /* AUTODOC:   clean VLAN table */
    /* AUTODOC:   restore FC mode */
    prvTgfFWSRestore();

}
