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
* @file prvCpssDxChPortIfModeCfgBC3ResourceTables.c
*
* @brief CPSS resource tables for BC3
*
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2B0ResourceList.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2B0ResourceTables.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgResource.h>


#define ANY_CC (GT_U32)(~0)

/*------------------------------------------------------------------------------------------------
 *  1. reg = GOP/XLG_MAC_IP/Units/<XLG_MAC_IP> XLG MAC IP %a Pipe %t/Port FIFOs Thresholds Configuration
 *     field : TxRdThr  [11-16]
 *      Port Speed (G)       1 2.5  5 10 12 20 24 25 28 40 48 50 55 100 120
 *      Port Threshold      17  17 17 17 17 17 17 17 17 17 17 17 17   3   3
 *-------------------------------------------------------------------------------------------------*/
const GT_U32 BC3_XLG_MAC_TxReadThreshold_Arr[2][PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  40000,  48000,  50000, 100000, 102000, 107000, 110000, 113000, 120000 }
    ,{ ANY_CC,     17,     17,     17,     17,     17,     17,     17,     17,     17,     17,     17,     17,     17,      3,      3,      3,      3,      3,      3 }
};


/*------------------------------------------------------------*
 * This file was auto-generated at 16/1/2017 15:49:60 *
 *------------------------------------------------------------*/
const PRV_CPSS_DXCH_SPEED_PLACE_STC prv_BC3_speedPlaceArr[] =
{
     {         CPSS_PORT_SPEED_1000_E,    1000,    1 }
    ,{         CPSS_PORT_SPEED_2500_E,    2500,    2 }
    ,{         CPSS_PORT_SPEED_5000_E,    5000,    3 }
    ,{        CPSS_PORT_SPEED_10000_E,   10000,    4 }
    ,{        CPSS_PORT_SPEED_11800_E,   12000,    5 }
    ,{        CPSS_PORT_SPEED_12500_E,   12500,    6 }
    ,{        CPSS_PORT_SPEED_20000_E,   20000,    7 }
    ,{        CPSS_PORT_SPEED_23600_E,   24000,    8 }
    ,{        CPSS_PORT_SPEED_25000_E,   25000,    9 }
    ,{        CPSS_PORT_SPEED_26700_E,   26700,   10 }
    ,{        CPSS_PORT_SPEED_40000_E,   40000,   11 }
    ,{        CPSS_PORT_SPEED_47200_E,   48000,   12 }
    ,{        CPSS_PORT_SPEED_50000_E,   50000,   13 }
    ,{         CPSS_PORT_SPEED_100G_E,  100000,   14 }
    ,{         CPSS_PORT_SPEED_102G_E,  102000,   15 }
    ,{         CPSS_PORT_SPEED_107G_E,  107000,   16 }
    ,{           CPSS_PORT_SPEED_NA_E,       0,    0 }
};


const PRV_CPSS_DXCH_CORECLOCK_PLACE_STC prv_BC3_coreClockPlaceArr[] =
{    /* real cc, computation, idx */
      {     425,     425,   1 }
     ,{     525,     525,   2 }
     ,{     572,     572,   3 }
     ,{     600,     600,   4 }
     ,{     625,     625,   5 }
     ,{       0,       0,   0 }
};


/*-----------------------------------------------------------------------------------*
 * BC3 TXDMA_SCDMA_TxQDescriptorCredit : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 BC3_TXDMA_SCDMA_TxQDescriptorCredit_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  40000,  48000,  50000, 100000, 102000, 107000,  110000, 113000, 120000 }
    ,{    425,      5,      5,      6,     10,      9,      9,     14,     13,     14,     14,     22,     21,     27,     46,     40,     40,     43,     42,     44 }
    ,{    525,      5,      5,      7,     10,      9,      9,     13,     12,     13,     13,     20,     20,     24,     41,     35,     35,     36,     37,     39 }
    ,{    572,      5,      5,      7,      9,      8,      8,     12,     11,     12,     12,     19,     18,     22,     39,     33,     33,     33,     34,     36 }
    ,{    600,      5,      5,      6,      9,      8,      8,     12,     11,     12,     12,     18,     17,     21,     37,     32,     32,     32,     32,     34 }
    ,{    625,      5,      5,      6,      9,      8,      8,     12,     11,     11,     11,     17,     16,     20,     36,     31,     31,     31,     31,     33 }
};
/*-----------------------------------------------------------------------------------*
 * BC3 TXDMA_SCDMA_burstAlmostFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 BC3_TXDMA_SCDMA_burstAlmostFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  40000,  48000,  50000, 100000, 102000, 107000, 110000, 113000, 120000 }
    ,{    425,      2,      2,      3,      4,      5,      5,      7,      7,      8,      8,     11,     13,     13,     24,     25,     25,     27,     27,     28 }
    ,{    525,      2,      2,      3,      4,      5,      5,      6,      7,      7,      7,     10,     12,     12,     21,     22,     22,     22,     23,     24 }
    ,{    572,      2,      2,      3,      4,      4,      4,      6,      6,      7,      7,      9,     10,     11,     20,     21,     21,     21,     21,     22 }
    ,{    600,      2,      2,      3,      4,      4,      4,      6,      6,      6,      6,      9,     10,     10,     19,     20,     20,     20,     20,     21 }
    ,{    625,      2,      2,      3,      4,      4,      4,      5,      6,      6,      6,      8,     10,     10,     18,     19,     19,     19,     19,     21 }
};
/*-----------------------------------------------------------------------------------*
 * BC3 TXDMA_SCDMA_burstFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 BC3_TXDMA_SCDMA_burstFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  40000,  48000,  50000, 100000, 102000, 107000, 110000, 113000, 120000 }
    ,{    425,      5,      5,      6,     10,      9,      9,     14,     13,     14,     14,     22,     21,     27,     46,     40,     40,     43,     42,     44 }
    ,{    525,      5,      5,      7,     10,      9,      9,     13,     12,     13,     13,     20,     20,     24,     41,     35,     35,     36,     37,     39 }
    ,{    572,      5,      5,      7,      9,      8,      8,     12,     11,     12,     12,     19,     18,     22,     39,     33,     33,     33,     34,     36 }
    ,{    600,      5,      5,      6,      9,      8,      8,     12,     11,     12,     12,     18,     17,     21,     37,     32,     32,     32,     32,     34 }
    ,{    625,      5,      5,      6,      9,      8,      8,     12,     11,     11,     11,     17,     16,     20,     36,     31,     31,     31,     31,     33 }
};
/*-----------------------------------------------------------------------------------*
 * BC3 TXDMA_SCDMA_TxFIFOHeaderCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 BC3_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  40000,  48000,  50000, 100000, 102000, 107000, 110000, 113000, 120000 }
    ,{    425,      4,      7,     11,     25,     24,     25,     49,     46,     48,     48,     92,     90,    115,    228,    197,    197,    204,    208,    219 }
    ,{    525,      4,      6,     10,     22,     21,     22,     40,     38,     41,     41,     76,     75,     95,    188,    162,    162,    166,    171,    182 }
    ,{    572,      4,      6,     10,     20,     20,     20,     39,     36,     38,     38,     72,     71,     91,    180,    154,    154,    157,    161,    171 }
    ,{    600,      4,      6,     10,     21,     19,     19,     37,     36,     38,     38,     71,     69,     89,    174,    149,    149,    153,    158,    167 }
    ,{    625,      4,      5,      9,     20,     19,     20,     36,     34,     36,     36,     69,     66,     85,    170,    145,    145,    149,    153,    163 }
};
/*-----------------------------------------------------------------------------------*
 * BC3 TXDMA_SCDMA_TxFIFOPayloadCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 BC3_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  40000,  48000,  50000, 100000, 102000, 107000, 110000, 113000, 120000 }
    ,{    425,      6,      9,     14,     30,     29,     31,     57,     55,     58,     58,    107,    107,    133,    263,    234,    234,    242,    247,    261 }
    ,{    525,      6,      8,     13,     26,     26,     27,     47,     46,     50,     50,     89,     90,    110,    217,    193,    193,    198,    204,    217 }
    ,{    572,      6,      8,     13,     24,     25,     25,     46,     44,     47,     47,     85,     86,    106,    209,    185,    185,    189,    194,    206 }
    ,{    600,      6,      8,     13,     25,     24,     24,     44,     44,     47,     47,     84,     84,    104,    203,    180,    180,    185,    191,    202 }
    ,{    625,      6,      7,     12,     24,     24,     25,     43,     42,     44,     44,     82,     81,    100,    199,    176,    176,    181,    186,    198 }
};
/*-----------------------------------------------------------------------------------*
 * BC3 TXFIFO_SCDMA_PayloadStartTransmThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 BC3_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  40000,  48000,  50000, 100000, 102000, 107000, 110000, 113000, 120000 }
    ,{    425,      3,      3,      4,      6,      6,      7,      9,     10,     11,     11,     16,     18,     19,     36,     38,     38,     39,     40,     43 }
    ,{    525,      3,      3,      4,      5,      6,      6,      8,      9,     10,     10,     14,     16,     16,     30,     32,     32,     33,     34,     36 }
    ,{    572,      3,      3,      4,      5,      6,      6,      8,      9,     10,     10,     14,     16,     16,     30,     32,     32,     33,     34,     36 }
    ,{    600,      3,      3,      4,      5,      6,      6,      8,      9,     10,     10,     14,     16,     16,     30,     32,     32,     33,     34,     36 }
    ,{    625,      3,      3,      4,      5,      6,      6,      8,      9,      9,      9,     14,     16,     16,     30,     32,     32,     33,     34,     36 }
};
/*-----------------------------------------------------------------------------------*
 * BC3 TXDMA_SCDMA_RateLimitIntegerPart : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 BC3_TXDMA_SCDMA_RateLimitIntegerPart_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  40000,  48000,  50000, 100000, 102000, 107000, 110000, 113000, 120000 }
    ,{    425,    435,    174,     87,     43,     36,     34,     21,     18,     17,     17,     10,      9,      8,      4,      4,      4,      3,      3,      3 }
    ,{    525,    537,    215,    107,     53,     44,     43,     26,     22,     21,     21,     13,     11,     10,      5,      5,      5,      4,      4,      4 }
    ,{    572,    585,    234,    117,     58,     48,     46,     29,     24,     23,     23,     14,     12,     11,      5,      5,      5,      5,      5,      4 }
    ,{    600,    614,    245,    122,     61,     51,     49,     30,     25,     24,     24,     15,     12,     12,      6,      5,      5,      5,      5,      5 }
    ,{    625,    640,    256,    128,     64,     53,     51,     32,     26,     25,     25,     16,     13,     12,      6,      5,      5,      5,      5,      5 }
};
/*-----------------------------------------------------------------------------------*
 * BC3 TXDMA_SCDMA_RateLimitResidueVector : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 BC3_TXDMA_SCDMA_RateLimitResidueVector_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS] =
{
     {      0,      1000,      2500,      5000,     10000,     12000,     12500,     20000,     24000,     25000,     26700,     40000,     48000,     50000,    100000,    102000,    107000,    110000,    113000,    120000 }
    ,{    425, DIV_6_32 , DIV_2_32 , DIV_1_32 , DIV_16_32, DIV_8_32 , DIV_26_32, DIV_24_32, DIV_4_32 , DIV_13_32, DIV_13_32, DIV_28_32, DIV_2_32 , DIV_22_32, DIV_11_32, DIV_2_32 , DIV_2_32 , DIV_30_32, DIV_27_32, DIV_20_32 }
    ,{    525, DIV_19_32, DIV_1_32 , DIV_16_32, DIV_24_32, DIV_25_32, DIV_0_32 , DIV_28_32, DIV_12_32, DIV_16_32, DIV_16_32, DIV_14_32, DIV_6_32 , DIV_24_32, DIV_12_32, DIV_0_32 , DIV_0_32 , DIV_28_32, DIV_24_32, DIV_15_32 }
    ,{    572, DIV_23_32, DIV_9_32 , DIV_4_32 , DIV_18_32, DIV_25_32, DIV_27_32, DIV_9_32 , DIV_12_32, DIV_13_32, DIV_13_32, DIV_20_32, DIV_6_32 , DIV_22_32, DIV_27_32, DIV_15_32, DIV_15_32, DIV_10_32, DIV_5_32 , DIV_28_32 }
    ,{    600, DIV_12_32, DIV_24_32, DIV_28_32, DIV_14_32, DIV_6_32 , DIV_4_32 , DIV_23_32, DIV_19_32, DIV_18_32, DIV_18_32, DIV_11_32, DIV_25_32, DIV_9_32 , DIV_4_32 , DIV_23_32, DIV_23_32, DIV_18_32, DIV_13_32, DIV_3_32  }
    ,{    625, DIV_0_32 , DIV_0_32 , DIV_0_32 , DIV_0_32 , DIV_10_32, DIV_6_32 , DIV_0_32 , DIV_21_32, DIV_19_32, DIV_19_32, DIV_0_32 , DIV_10_32, DIV_25_32, DIV_12_32, DIV_31_32, DIV_31_32, DIV_26_32, DIV_21_32, DIV_10_32 }
};




