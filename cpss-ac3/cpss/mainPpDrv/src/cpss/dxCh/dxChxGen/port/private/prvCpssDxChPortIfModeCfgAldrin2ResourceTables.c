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
* @file prvCpssDxChPortIfModeCfgAldrin2ResourceTables.c
*
* @brief CPSS resource tables for Aldrin2
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

/* 107G  speed added with the same values that 110G  has */
/* 11.8G speed added with the same values that 12.5G has */
/* 11.8G speed represented as 12000 in all resource tables in CPSS          */
/* prvCpssDxChCaelumPortResourcesSpeedIndexGet converts speed name to value */
/* by common prv_SpeedEnmSpeedMbpsArr table that contains this value.       */

/*------------------------------------------------------------*
 * This file was auto-generated at 5/11/2017 17:34:16 *
 *------------------------------------------------------------*/
const PRV_CPSS_DXCH_SPEED_PLACE_STC prv_ARM2_dp01_speedPlaceArr[] =
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
    ,{         CPSS_PORT_SPEED_26700_E,  26700,   10 }
    ,{        CPSS_PORT_SPEED_29090_E,   29090,   11 }
    ,{        CPSS_PORT_SPEED_40000_E,   40000,   12 }
    ,{        CPSS_PORT_SPEED_47200_E,   48000,   13 }
    ,{        CPSS_PORT_SPEED_50000_E,   50000,   14 }
    ,{        CPSS_PORT_SPEED_52500_E,   52500,   15 }
    ,{         CPSS_PORT_SPEED_100G_E,  100000,   16 }
    ,{         CPSS_PORT_SPEED_102G_E,  102000,   17 }
    ,{         CPSS_PORT_SPEED_107G_E,  107000,   18 }
    ,{           CPSS_PORT_SPEED_NA_E,       0,    0 }
};


const PRV_CPSS_DXCH_CORECLOCK_PLACE_STC prv_ARM2_coreClockPlaceArr[] =
{    /* real cc, computation, idx */
      {     600,     600,   1 }
     ,{     525,     525,   2 }
     ,{     450,     450,   3 }
     ,{       0,       0,   0 }
};


/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_TxQDescriptorCredit : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp01_TXDMA_SCDMA_TxQDescriptorCredit_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  29090,  40000,  48000,  50000,  52500, 100000, 102000, 107000, 110000, 120000 }
    ,{    600,      5,      5,      6,      9,      8,      8,     12,     11,     14,     14,     15,     19,     17,     22,     19,     37,     33,     33,     33,     35 }
    ,{    525,      5,      5,      7,     10,      9,      9,     13,     12,     15,     15,     16,     20,     19,     24,     22,     42,     37,     37,     37,     39 }
    ,{    450,      5,      5,      6,      9,      9,      9,     13,     13,     16,     16,     17,     21,     21,     26,     23,     45,     39,     39,     39,     43 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_burstAlmostFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp01_TXDMA_SCDMA_burstAlmostFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  29090,  40000,  48000,  50000,  52500, 100000, 102000, 107000, 110000, 120000 }
    ,{    600,      2,      2,      3,      4,      4,      4,      6,      6,      7,      7,      7,      9,     10,     11,     11,     19,     20,     20,     20,     22 }
    ,{    525,      2,      2,      3,      4,      4,      4,      6,      7,      7,      7,      8,     10,     11,     12,     13,     22,     23,     23,     23,     24 }
    ,{    450,      2,      2,      3,      4,      5,      5,      6,      7,      8,      8,      8,     11,     13,     13,     14,     23,     25,     25,     25,     27 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_burstFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp01_TXDMA_SCDMA_burstFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  29090,  40000,  48000,  50000,  52500, 100000, 102000, 107000, 110000, 120000 }
    ,{    600,      5,      5,      6,      9,      8,      8,     12,     11,     14,     14,     15,     19,     17,     22,     19,     37,     33,     33,     33,     35 }
    ,{    525,      5,      5,      7,     10,      9,      9,     13,     12,     15,     15,     16,     20,     19,     24,     22,     42,     37,     37,     37,     39 }
    ,{    450,      5,      5,      6,      9,      9,      9,     13,     13,     16,     16,     17,     21,     21,     26,     23,     45,     39,     39,     39,     43 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_TxFIFOHeaderCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp01_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000, 10000,  12000,  12500,  20000,  24000,  25000,  26700,  29090,  40000,  48000,  50000,  52500, 100000, 102000, 107000, 110000, 120000 }
    ,{    600,      4,      6,      9,    19,     18,     18,     34,     33,     42,     42,     47,     65,     64,     82,     73,    158,    140,    140,    140,    153 }
    ,{    525,      4,      6,     10,    20,     20,     20,     37,     37,     46,     46,     51,     73,     69,     89,     79,    174,    155,    155,    155,    170 }
    ,{    450,      4,      6,     11,    23,     23,     23,     44,     43,     54,     54,     59,     85,     83,    107,     94,    206,    184,    184,    184,    201 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_TxFIFOPayloadCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp01_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  29090,  40000,  48000,  50000,  52500, 100000, 102000, 107000, 110000, 120000 }
    ,{    600,      6,      8,     12,     22,     22,     22,     39,     39,     48,     48,     54,     74,     74,     93,     85,    178,    162,    162,    162,    177 }
    ,{    525,      6,      8,     13,     23,     24,     24,     42,     43,     52,     52,     58,     82,     79,    100,     91,    194,    177,    177,    177,    194 }
    ,{    450,      6,      8,     14,     27,     27,     27,     50,     50,     61,     61,     67,     96,     96,    120,    109,    231,    212,    212,    212,    231 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXFIFO_SCDMA_PayloadStartTransmThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp01_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  26700,  29090,  40000,  48000,  50000,  52500, 100000, 102000, 107000, 110000,120000 }
    ,{    600,      3,      3,      4,      4,      5,      5,      6,      7,      7,      7,      8,     10,     11,     12,     13,     21,     23,     23,     23,    25 }
    ,{    525,      3,      3,      4,      4,      5,      5,      6,      7,      7,      7,      8,     10,     11,     12,     13,     21,     23,     23,     23,    25 }
    ,{    450,      3,      3,      4,      5,      5,      5,      7,      8,      8,      8,      9,     12,     14,     14,     16,     26,     29,     29,     29,    31 }
};


/*------------------------------------------------------------*
 * This file was auto-generated at 5/11/2017 17:34:16 *
 *------------------------------------------------------------*/
const PRV_CPSS_DXCH_SPEED_PLACE_STC prv_ARM2_dp23_speedPlaceArr[] =
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
    ,{        CPSS_PORT_SPEED_29090_E,   29090,   10 }
    ,{        CPSS_PORT_SPEED_40000_E,   40000,   11 }
    ,{        CPSS_PORT_SPEED_47200_E,   48000,   12 }
    ,{        CPSS_PORT_SPEED_50000_E,   50000,   13 }
    ,{        CPSS_PORT_SPEED_52500_E,   52500,   14 }
    ,{           CPSS_PORT_SPEED_NA_E,       0,    0 }
};


/* values for 55G copied from DP01 */

/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_TxQDescriptorCredit : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp23_TXDMA_SCDMA_TxQDescriptorCredit_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  29090,  40000,  48000,  50000, 52500 }
    ,{    600,      5,      5,      6,      9,      8,      8,     12,     12,     14,     14,     19,     18,     23,    19 }
    ,{    525,      5,      5,      6,      9,      9,      9,     13,     12,     15,     15,     21,     20,     24,    22 }
    ,{    450,      5,      5,      6,     10,      9,      9,     15,     14,     16,     16,     23,     21,     26,    23 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_burstAlmostFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp23_TXDMA_SCDMA_burstAlmostFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  29090,  40000,  48000,  50000,  52500 }
    ,{    600,      2,      2,      3,      4,      4,      4,      6,      7,      7,      7,      9,     11,     11,     11 }
    ,{    525,      2,      2,      3,      4,      4,      4,      6,      7,      7,      7,     10,     12,     12,     13 }
    ,{    450,      2,      2,      3,      4,      5,      5,      7,      8,      8,      8,     11,     13,     13,     14 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_burstFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp23_TXDMA_SCDMA_burstFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  29090,  40000,  48000,  50000,  52500 }
    ,{    600,      5,      5,      6,      9,      8,      8,     12,     12,     14,     14,     19,     18,     23,     19 }
    ,{    525,      5,      5,      6,      9,      9,      9,     13,     12,     15,     15,     21,     20,     24,     22 }
    ,{    450,      5,      5,      6,     10,      9,      9,     15,     14,     16,     16,     23,     21,     26,     23 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_TxFIFOHeaderCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp23_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  29090,  40000,  48000,  50000,  52500 }
    ,{    600,      4,      5,      9,     18,     18,     18,     34,     33,     43,     43,     67,     63,     81,     73 }
    ,{    525,      3,      6,      9,     20,     20,     20,     37,     37,     47,     47,     72,     71,     89,     79 }
    ,{    450,      4,      7,     11,     24,     24,     24,     45,     43,     54,     54,     85,     83,    107,     94 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXDMA_SCDMA_TxFIFOPayloadCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp23_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  29090,  40000,  48000,  50000,  52500 }
    ,{    600,      6,      7,     11,     21,     22,     22,     39,     39,     49,     49,     76,     73,     92,     85 }
    ,{    525,      5,      8,     11,     23,     24,     24,     42,     43,     53,     53,     81,     81,    100,     91 }
    ,{    450,      6,      9,     14,     28,     28,     28,     51,     50,     61,     61,     96,     96,    120,    109 }
};
/*-----------------------------------------------------------------------------------*
 * ARM2 TXFIFO_SCDMA_PayloadStartTransmThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 ARM2_dp23_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS] =
{
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  20000,  24000,  25000,  29090,  40000,  48000,  50000,  52500 }
    ,{    600,      3,      3,      3,      4,      5,      5,      6,      7,      7,      7,     10,     11,     12,     13 }
    ,{    525,      3,      3,      3,      4,      5,      5,      6,      7,      7,      7,     10,     11,     12,     13 }
    ,{    450,      3,      3,      4,      5,      5,      5,      7,      8,      8,      8,     12,     14,     14,     16 }
};


