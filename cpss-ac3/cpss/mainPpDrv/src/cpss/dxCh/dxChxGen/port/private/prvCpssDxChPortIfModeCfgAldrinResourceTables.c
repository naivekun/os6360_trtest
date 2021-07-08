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
* @file prvCpssDxChPortIfModeCfgAldrinResourceTables.c
*
* @brief CPSS resource tables for Aldrin
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
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgResource.h>


/*------------------------------------------------------------*
 * This file was auto-generated at 1/12/2016 9:49:33 *
 *------------------------------------------------------------*/
const PRV_CPSS_DXCH_SPEED_PLACE_STC prv_Aldrin_speedPlaceArr[] = 
{
     {         CPSS_PORT_SPEED_1000_E,    1000,    1 }
    ,{         CPSS_PORT_SPEED_2500_E,    2500,    2 }
    ,{         CPSS_PORT_SPEED_5000_E,    5000,    3 }
    ,{        CPSS_PORT_SPEED_10000_E,   10000,    4 }
    ,{        CPSS_PORT_SPEED_11800_E,   12000,    5 }
    ,{        CPSS_PORT_SPEED_12500_E,   12500,    6 }
    ,{        CPSS_PORT_SPEED_20000_E,   20000,    8 }
    ,{        CPSS_PORT_SPEED_23600_E,   24000,    9 }
    ,{        CPSS_PORT_SPEED_25000_E,   25000,   10 }
    ,{        CPSS_PORT_SPEED_40000_E,   40000,   11 }
    ,{        CPSS_PORT_SPEED_47200_E,   48000,   12 }
    ,{        CPSS_PORT_SPEED_50000_E,   50000,   14 }
    ,{           CPSS_PORT_SPEED_NA_E,       0,    0 }
};


const PRV_CPSS_DXCH_CORECLOCK_PLACE_STC prv_Aldrin_coreClockPlaceArr[] = 
{    /* real cc, computation, idx */
      {     200,     200,   1 }
     ,{     250,     250,   2 }
     ,{     365,     365,   3 }
     ,{     480,     480,   4 }
     ,{       0,       0,   0 }
};


/*-----------------------------------------------------------------------------------*
 * Aldrin TXDMA_SCDMA_TxQDescriptorCredit : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 Aldrin_TXDMA_SCDMA_TxQDescriptorCredit_Arr[5][15] = 
{ 
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  13000,  20000,  24000,  25000,  40000,  48000,  49000,  50000 }
    ,{    200,      3,      4,      6,     13,     12,     12,     13,     21,     20,     21,     39,     37,     37,     38 }
    ,{    250,      4,      5,      7,     13,     11,     12,     13,     21,     20,     20,     36,     34,     34,     36 }
    ,{    365,      4,      4,      6,     10,      9,      9,     10,     16,     15,     15,     26,     25,     26,     26 }
    ,{    480,      4,      4,      5,      8,      8,      8,      8,     13,     12,     12,     21,     20,     21,     21 }
}; 
/*-----------------------------------------------------------------------------------*
 * Aldrin TXDMA_SCDMA_burstAlmostFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 Aldrin_TXDMA_SCDMA_burstAlmostFullThreshold_Arr[5][15] = 
{ 
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  13000,  20000,  24000,  25000,  40000,  48000,  49000,  50000 }
    ,{    200,      2,      3,      4,      7,      8,      8,      8,     12,     14,     14,     21,     24,     24,     25 }
    ,{    250,      3,      3,      5,      7,      8,      8,      9,     12,     13,     13,     19,     23,     23,     24 }
    ,{    365,      3,      3,      4,      6,      6,      6,      7,      9,     10,     10,     14,     17,     17,     17 }
    ,{    480,      3,      3,      4,      5,      5,      5,      5,      7,      8,      8,     11,     13,     14,     14 }
}; 
/*-----------------------------------------------------------------------------------*
 * Aldrin TXDMA_SCDMA_burstFullThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 Aldrin_TXDMA_SCDMA_burstFullThreshold_Arr[5][15] = 
{ 
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  13000,  20000,  24000,  25000,  40000,  48000,  49000,  50000 }
    ,{    200,      3,      4,      6,     13,     12,     12,     13,     21,     20,     21,     39,     37,     37,     38 }
    ,{    250,      4,      5,      7,     13,     11,     12,     13,     21,     20,     20,     36,     34,     34,     36 }
    ,{    365,      4,      4,      6,     10,      9,      9,     10,     16,     15,     15,     26,     25,     26,     26 }
    ,{    480,      4,      4,      5,      8,      8,      8,      8,     13,     12,     12,     21,     20,     21,     21 }
}; 
/*-----------------------------------------------------------------------------------*
 * Aldrin TXDMA_SCDMA_TxFIFOHeaderCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 Aldrin_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr[5][15] = 
{ 
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  13000,  20000,  24000,  25000,  40000,  48000,  49000,  50000 }
    ,{    200,      4,      9,     18,     42,     41,     42,     44,     82,     80,     83,    162,      0,      0,      0 }
    ,{    250,      4,      8,     15,     35,     33,     35,     36,     68,     66,     68,    133,    127,    130,    133 }
    ,{    365,      4,      6,     12,     25,     24,     25,     26,     48,     46,     48,     92,     90,     92,     94 }
    ,{    480,      3,      5,      9,     19,     18,     19,     20,     36,     35,     37,     71,     68,     70,     71 }
}; 
/*-----------------------------------------------------------------------------------*
 * Aldrin TXDMA_SCDMA_TxFIFOPayloadCreditThreshold : core clock (MHz) x  speed (Mbps)
 *------------------------- ----------------------------------------------------------*/
const GT_U32 Aldrin_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr[5][15] = 
{ 
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  13000,  20000,  24000,  25000,  40000,  48000,  49000,  50000 }
    ,{    200,      4,     10,     20,     46,     46,     47,     50,     91,     90,     94,    179,     21,     21,     22 }
    ,{    250,      4,      9,     17,     39,     37,     40,     41,     75,     75,     77,    147,    144,    147,    151 }
    ,{    365,      5,      7,     13,     28,     27,     28,     29,     53,     52,     54,    102,    102,    104,    106 }
    ,{    480,      3,      5,     10,     21,     20,     21,     22,     40,     40,     42,     78,     77,     79,     80 }
}; 
/*-----------------------------------------------------------------------------------*
 * Aldrin TXFIFO_SCDMA_PayloadStartTransmThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
const GT_U32 Aldrin_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr[5][15] = 
{ 
     {      0,   1000,   2500,   5000,  10000,  12000,  12500,  13000,  20000,  24000,  25000,  40000,  48000,  49000,  50000 }
    ,{    200,      1,      2,      3,      5,      6,      6,      7,     10,     11,     12,     18,     22,     22,     23 }
    ,{    250,      1,      2,      3,      5,      5,      6,      6,      8,     10,     10,     15,     18,     18,     19 }
    ,{    365,      2,      2,      2,      4,      4,      4,      4,      6,      7,      7,     11,     13,     13,     13 }
    ,{    480,      1,      1,      2,      3,      3,      3,      3,      5,      6,      6,      8,     10,     10,     10 }
}; 


