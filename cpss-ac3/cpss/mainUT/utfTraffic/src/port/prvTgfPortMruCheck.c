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
* @file prvTgfPortMruCheck.c
*
* @brief Port Profile MRU check - basic
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <port/prvTgfPortMruCheck.h>
#include <common/tgfCutThrough.h>

/**********************************************************************
 * typedef: PRV_TGF_MRU_RESTORE_CONFIG
 *
 * Description: The Tail Drop profile Traffic Class parameters
 *
 * Fields:
 *   dftPhyMruValue  - MRU value per physical port.
 *   dftMruIndex     - MRU profile index per port.
 *   dftMruValue     - MRU value per ePort.
 *   dftCpuCode      - CPU code set
 *   dftCommand      - default command
 *********************************************************************/
static struct
{
    GT_U32                      dftPhyMruValue;
    GT_U32                      dftMruIndex;
    GT_U32                      dftMruValue;
    CPSS_NET_RX_CPU_CODE_ENT    dftCpuCode;
    CPSS_PACKET_CMD_ENT         dftCommand;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* MRU value per physical port*/
#define PRV_TGF_MRU_VALUE_PHY_CNS                   10240

/* default MRU value per ePort*/
#define PRV_TGF_MRU_VALUE_CNS                       0x3fff

/* MRU value per ePort */
#define PRV_TGF_MRU_VALUE_EPORT_CNS                 511

#define PRV_TGF_VLANID_1_CNS                        1

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS                   0
/* capturing port index  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS                2

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS                2

/* CPU code to be set in test*/
#define PRV_TGF_MRU_EXCEPTION_CPU_CODE_CNS          CPSS_NET_CONTROL_E

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* checking the result packets */

/* expected number of Rx packets on ports - for DROP command */
static GT_U8 prvTgfPacketsCountRxArrDrop[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{   /*0 18  36 54 */
    {1, 0, 1, 0}, /*pkt#0*/
    {1, 0, 0, 0}  /*pkt#1*/
};
/* expected number of Tx packets on ports */
static GT_U8 prvTgfPacketsCountTxArrDrop[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 1, 0},
    {1, 0, 0, 0}
};

/* expected number of Rx packets on ports - for Forward command */
static GT_U8 prvTgfPacketsCountRxArrForward[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{   /*0 8  18 23 */
    {1, 0, 1, 0}, /*na*/
    {1, 0, 1, 0}
};
/* expected number of Tx packets on ports */
static GT_U8 prvTgfPacketsCountTxArrForward[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 1, 0}, /*na*/
    {1, 0, 1, 0}
};

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadData1Arr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, /*48*/
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, /*64*/
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80,
    0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
    0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90,
    0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0,
    0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
    0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0,
    0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
    0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0,
    0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
    0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
    0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0,
    0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8,
    0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80,
    0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
    0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90,
    0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0,
    0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
    0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0,
    0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
    0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0,
    0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
    0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
    0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0,
    0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8,
    0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef
};

/* DATA of packet 2 */
static GT_U8 prvTgfPayloadData2Arr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, /*48*/
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, /*64*/
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80,
    0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
    0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90,
    0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0,
    0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
    0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0,
    0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
    0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0,
    0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
    0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
    0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0,
    0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8,
    0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80,
    0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
    0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90,
    0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0,
    0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
    0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0,
    0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
    0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0,
    0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
    0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
    0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0,
    0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8,
    0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xaa
};

/* PAYLOAD part first packet */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart1 =
{
    sizeof(prvTgfPayloadData1Arr),                     /* dataLength */
    prvTgfPayloadData1Arr                              /* dataPtr */
};

/* PAYLOAD part second packet */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart2 =
{
    sizeof(prvTgfPayloadData2Arr),                     /* dataLength */
    prvTgfPayloadData2Arr                              /* dataPtr */
};

/* PARTS of first tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPart1Array[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart1}
};

/* PARTS of second tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPart2Array[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart2}
};

/* LENGTH of first packet =511 */
#define PRV_TGF_TAGGED_PACKET1_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadData1Arr)

/* LENGTH of second packet =512 */
#define PRV_TGF_TAGGED_PACKET2_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadData2Arr)

/* First PACKET to send - size 511 */
static TGF_PACKET_STC prvTgfTaggedPacketInfo1 =
{
    PRV_TGF_TAGGED_PACKET1_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfTaggedPacketPart1Array) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPart1Array                                        /* partsArray */
};

/* Second PACKET to send - size 512 */
static TGF_PACKET_STC prvTgfTaggedPacketInfo2 =
{
    PRV_TGF_TAGGED_PACKET2_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfTaggedPacketPart2Array) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPart2Array                                        /* partsArray */
};

/* send packet info array */
static TGF_PACKET_STC* prvTgfPacketInfoArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    &prvTgfTaggedPacketInfo1, &prvTgfTaggedPacketInfo2
};

/******************************************************************************/

/* CUT THROUGH DEFAULTS */

#define UTF_VERIFY_EQUAL0_STRING_NO_RETURN(e, r, s)                                     \
    do {                                                                                \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                         \
        if (GT_FALSE == err) {                                                          \
            utfFailureMsgLog(s, (GT_UINTPTR *)NULL, 0);                                 \
        }                                                                               \
    } while(0)

static GT_BOOL cutThroughDefaultPortCtEnable;
static GT_BOOL cutThroughDefaultPortUntaggedCtEnable;

static GT_VOID prvTgfPortProfileMruCheckCutThroughDefaultsSet()
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
        cutThroughDefaultPortCtEnable         = GT_FALSE;
        cutThroughDefaultPortUntaggedCtEnable = GT_FALSE;
    }
    else
    {
        cutThroughDefaultPortCtEnable         = GT_TRUE;
        cutThroughDefaultPortUntaggedCtEnable = GT_TRUE;
    }
}

static int prvTgfPortProfileMruCheckCutThroughIsEnable
(
    GT_VOID
)
{
    switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily)
    {
        case CPSS_PP_FAMILY_CHEETAH_E:
        case CPSS_PP_FAMILY_CHEETAH2_E:
        case CPSS_PP_FAMILY_CHEETAH3_E:
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
        case CPSS_PP_FAMILY_DXCH_LION_E:
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
        case CPSS_PP_FAMILY_DXCH_LION2_E:
        case CPSS_PP_FAMILY_DXCH_LION3_E:
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            return 0;
        default: return 1;
    }
}

static GT_VOID prvTgfPortProfileMruCheckCutThroughPortsGenConfig
(
    GT_VOID
)
{
    GT_STATUS rc;

    if (prvTgfPortProfileMruCheckCutThroughIsEnable() == 0) return;

    rc = prvTgfCutThroughPortEnableSet(
        CPSS_CPU_PORT_NUM_CNS, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    rc = prvTgfCutThroughPortEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    rc = prvTgfCutThroughPortEnableSet(
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
}

static GT_VOID prvTgfPortProfileMruCheckCutThroughPortsGenRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    if (prvTgfPortProfileMruCheckCutThroughIsEnable() == 0) return;

    prvTgfPortProfileMruCheckCutThroughDefaultsSet();

    rc = prvTgfCutThroughPortEnableSet(
        CPSS_CPU_PORT_NUM_CNS,
        cutThroughDefaultPortCtEnable, cutThroughDefaultPortUntaggedCtEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    rc = prvTgfCutThroughPortEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        cutThroughDefaultPortCtEnable, cutThroughDefaultPortUntaggedCtEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
    rc = prvTgfCutThroughPortEnableSet(
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        cutThroughDefaultPortCtEnable, cutThroughDefaultPortUntaggedCtEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN(GT_OK, rc, "prvTgfCutThroughPortEnableSet");
}



/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPortProfileMruCheckConfigSet function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*                                       None
*/
GT_VOID prvTgfPortProfileMruCheckConfigSet
(
    IN GT_U32           profileId
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: save default MRU configuration for restore */
    rc = prvTgfPortMruGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfRestoreCfg.dftPhyMruValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortMruGet: %d", prvTgfRestoreCfg.dftPhyMruValue);

    rc = prvTgfPortProfileMruSizeGet(profileId, &prvTgfRestoreCfg.dftMruValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortProfileMruSizeGet: %d", prvTgfRestoreCfg.dftMruValue);

    rc = prvTgfPortMruProfileGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfRestoreCfg.dftMruIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortMruProfileGet: %d", prvTgfRestoreCfg.dftMruIndex);

    rc = prvTgfPortMruExceptionCPUCodeGet(&prvTgfRestoreCfg.dftCpuCode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortMruExceptionCPUCodeGet: %d", prvTgfRestoreCfg.dftCpuCode);

    rc = prvTgfPortMruExceptionCommandGet(&prvTgfRestoreCfg.dftCommand);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortMruExceptionCommandGet: %d", prvTgfRestoreCfg.dftCommand);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 2, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_VLANID_1_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: for MRU per physical port config maximum size 1522 */
    rc = prvTgfPortMruSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_MRU_VALUE_PHY_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruSet");

    /* AUTODOC: for MRU profile index <I> [0..7] set MRU value 511 */
    if(prvCpssDxChPortRemotePortCheck(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]))
    {
        /* On remote port, add 4 bytes for DSA Tag */
        rc = prvTgfPortProfileMruSizeSet(profileId, PRV_TGF_MRU_VALUE_EPORT_CNS + 4);
    }
    else
    {
        rc = prvTgfPortProfileMruSizeSet(profileId, PRV_TGF_MRU_VALUE_EPORT_CNS);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortProfileMruSizeSet");

    /* AUTODOC: for portNum 0, set MRU profile <I> */
    rc = prvTgfPortMruProfileSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], profileId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruProfileSet");

    /* AUTODOC: for frames that exceed the MRU size and are mirrored to CPU   */
    /* AUTODOC:     set the CPU command to be: */
    rc = prvTgfPortMruExceptionCPUCodeSet(PRV_TGF_MRU_EXCEPTION_CPU_CODE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruExceptionCPUCodeSet");

    /* AUTODOC: for profileId 0 -> set the CPU port Default profileId to 1*/
    if (profileId == 0) {
        rc = prvTgfPortProfileMruSizeSet(profileId+1, PRV_TGF_MRU_VALUE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortProfileMruSizeSet");

        rc = prvTgfPortMruProfileSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, profileId+1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruProfileSet");
    }

    prvTgfPortProfileMruCheckCutThroughPortsGenConfig();
}

/**
* @internal prvTgfPortProfileMruCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPortProfileMruCheckTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      portIter = 0;
    GT_U32                      sendIter = 0;
    GT_U32                      packetLen = 0;
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U8                       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                      packetActualLength = 0;
    GT_U8                       devNum;
    GT_U8                       queue;
    TGF_NET_DSA_STC             rxParam;

    /* AUTODOC: Configure COMMAND for packets with are oversized to DROP */
    rc = prvTgfPortMruExceptionCommandSet(CPSS_PACKET_CMD_DROP_SOFT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruExceptionCommandSet");

    /* AUTODOC: GENERATE TRAFFIC#1: */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter],
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send 2 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11 */
        /* AUTODOC:   payload sizes: */
        /* AUTODOC:     packet#1 = [511] bytes */
        /* AUTODOC:     packet#2 = [512] bytes */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* the packet is big - wait additional time */
        cpssOsTimerWkAfter(10);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic#1: */
        /* AUTODOC:   packet#1 (size=511) - received on port 36 */
        /* AUTODOC:   packet#2 (size=512) - dropped */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArrDrop[sendIter][portIter],
                                        prvTgfPacketsCountTxArrDrop[sendIter][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

    }

    /* AUTODOC: Configure COMMAND for packets with are oversized to FORWARD */
    rc = prvTgfPortMruExceptionCommandSet(CPSS_PACKET_CMD_FORWARD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruExceptionCommandSet");

    /* AUTODOC: GENERATE TRAFFIC#2: */
    /* send only the second packet - which is oversized - and check it was received */
    for (sendIter = 1; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter],
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send 1 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11 */
        /* AUTODOC:   payload sizes: */
        /* AUTODOC:     packet#2 = [513] bytes */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* the packet is big - wait additional time */
        cpssOsTimerWkAfter(10);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic#2: */
        /* AUTODOC:   packet#2 (size=513) - forwarded on port 36 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArrForward[sendIter][portIter],
                                        prvTgfPacketsCountTxArrForward[sendIter][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }
    }

    /* AUTODOC: Configure COMMAND for packets with are oversized to MIRROR to CPU */
    rc = prvTgfPortMruExceptionCommandSet(CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruExceptionCommandSet");

    /* AUTODOC: GENERATE TRAFFIC#3: */
    /* send only the second packet - which is oversized - and check it was received */
    for (sendIter = 1; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter],
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send 1 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11 */
        /* AUTODOC:   payload sizes: */
        /* AUTODOC:     packet#2 = [513] bytes */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* the packet is big - wait additional time */
        cpssOsTimerWkAfter(10);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic#2: */
        /* AUTODOC:   packet#2 (size=513) - forward */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArrForward[sendIter][portIter],
                                        prvTgfPacketsCountTxArrForward[sendIter][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* AUTODOC:   packet#2 received in CPU */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_TRUE, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");
        /* AUTODOC:   field CPU code is   PRV_TGF_MRU_EXCEPTION_CPU_CODE_CNS */
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of CPUcode in rxParams: %d, &d\n",
                                     rxParam.cpuCode, PRV_TGF_MRU_EXCEPTION_CPU_CODE_CNS );

    }
}

/**
* @internal prvTgfPortProfileMruCheckConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*                                       None
*/
GT_VOID prvTgfPortProfileMruCheckConfigRestore
(
    IN GT_U32           profileId
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfPortProfileMruCheckCutThroughPortsGenRestore();

    /* AUTODOC: restore default MRU value for MRU profile 1 */
    rc = prvTgfPortMruSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.dftPhyMruValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruSet");

    rc = prvTgfPortProfileMruSizeSet(profileId, prvTgfRestoreCfg.dftMruValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortProfileMruSizeSet");

    /* AUTODOC: restore MRU profile index for port */
    rc = prvTgfPortMruProfileSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.dftMruIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruProfileSet");

    /* AUTODOC: for profileId 0 -> restore the CPU port Default profileId to 0 */
    if (profileId == 0) {
        rc = prvTgfPortProfileMruSizeSet(profileId+1, prvTgfRestoreCfg.dftMruIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortProfileMruSizeSet");

        rc = prvTgfPortMruProfileSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruProfileSet");
    }

    /* AUTODOC: restore default CPU code for frames which fail the MRU check */
    rc = prvTgfPortMruExceptionCPUCodeSet(prvTgfRestoreCfg.dftCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruExceptionCPUCodeSet");

    /* AUTODOC: restore default exception command for frames which fail the MRU check */
    rc = prvTgfPortMruExceptionCommandSet(prvTgfRestoreCfg.dftCommand);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortMruExceptionCommandSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);
}


