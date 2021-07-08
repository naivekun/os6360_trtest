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
* @file prvCpssDxChPortDynamicPizzaArbiterWS.c
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter Data structures (device specific)
*
* @version   93
********************************************************************************
*/
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPATypeDef.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>


#if 0


Bobcat2
+--------------+-----------------------------------+--------------------------------------------------+--------------------------------------------------------------------+------------------------------
|              |                       Control     |           Map                                    |                           Configuration                            |    Cider path
|              +------------+----------------------+------------+--------+--------------+-------------+------------------------+-------------------------------------------|
|   Unit       |            |          Fields      |            |        | Flds Statist | Single Fld  | Client                 |  Algo to be used                          |
|              | Register   +------+-------+-------|  Register  | number +------+-------+------+------|  Code                  |                                           |
|              |            | Slice| Map   |  WC   |            |        |  per | Total | Slice|Slice |                        |                                           |
|              |            |To-Run|Load-En|  mode |            |        |  reg |       |   Map | En  |                        |                                           |/Cider/EBU/BobK/BobK {Current}/Switching Core/
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+--------------------------------
| RXDMA-0      | 0x17002A00 |  0-8 | 14-14 | 15-15 | 0x17002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, no dummy slices        | RXDMA/Units/<RXDMA0_IP> RxDMA IP Units/<Pizza Arbiter> SIP_COMMON_MODULES_IP Units/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXDMA-0      | 0x26004000 |  0-8 | 14-14 | 15-15 | 0x26004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, no dummy slices        | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/<Pizza Arb> TxDMA Pizza Arb/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXFIFO-0     | 0x27000800 |  0-8 | 14-14 | 15-15 | 0x27000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, no dummy slices        | TX_FIFO/Units/<TXFIFO0_IP> TxFIFO IP Units/<Pizza Arb> TxFIFO Pizza Arb/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| Eth-TXFIFO-0 | 0x1A000800 |  0-8 | 14-14 | 15-15 | 0x1A000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, no dummy slices        | ETH_TX_FIFO/<ETH_TX_FIFO_0> SIP_ETH_TXFIFO_IP/Units/SIP_ETH_TXFIFO_IP Units/TxFIFO Pizza Arb/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| TXQ          | 0x40001500 |  0-8 | 12-12 |  9-9  | 0x40001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            | <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ_IP_dq/Scheduler/Port Arbiter Configuration/
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------

BobK Cetus/Caelum
+--------------+-----------------------------------+--------------------------------------------------+--------------------------------------------------------------------+------------------------------
|              |                       Control     |           Map                                    |                           Configuration                            |    Cider path
|              +------------+----------------------+------------+--------+--------------+-------------+------------------------+-------------------------------------------|
|   Unit       |            |          Fields      |            |        | Flds Statist | Single Fld  | Client                 |  Algo to be used                          |
|              | Register   +------+-------+-------|  Register  | number +------+-------+------+------|  Code                  |                                           |
|              |            | Slice| Map   |  WC   |            |        |  per | Total | Slice|Slice |                        |                                           |
|              |            |To-Run|Load-En|  mode |            |        |  reg |       |   Map | En  |                        |                                           |/Cider/EBU/BobK/BobK {Current}/Switching Core/
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+--------------------------------
| RXDMA-0      | 0x17002A00 |  0-8 | 14-14 | 15-15 | 0x17002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | RXDMA/Units/<RXDMA0_IP> RxDMA IP Units/<Pizza Arbiter> SIP_COMMON_MODULES_IP Units/Pizza Arbiter
| RXDMA-1      | 0x68002A00 |  0-8 | 14-14 | 15-15 | 0x68002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | RXDMA/Units/<RXDMA1_IP> RxDMA IP Units/<Pizza Arbiter> SIP_COMMON_MODULES_IP Units/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXDMA-0      | 0x26004000 |  0-8 | 14-14 | 15-15 | 0x26004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/<Pizza Arb> TxDMA Pizza Arb/Pizza Arbiter
| TXDMA-1      | 0x66004000 |  0-8 | 14-14 | 15-15 | 0x66004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/<Pizza Arb> TxDMA Pizza Arb/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXFIFO-0     | 0x27000800 |  0-8 | 14-14 | 15-15 | 0x27000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TX_FIFO/Units/<TXFIFO0_IP> TxFIFO IP Units/<Pizza Arb> TxFIFO Pizza Arb/Pizza Arbiter
| TXFIFO-1     | 0x67000800 |  0-8 | 14-14 | 15-15 | 0x67000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TX_FIFO/Units/<TXFIFO1_IP> TxFIFO IP Units/<Pizza Arb> TxFIFO Pizza Arb/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| Eth-TXFIFO-0 | 0x1A000800 |  0-8 | 14-14 | 15-15 | 0x1A000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | ETH_TX_FIFO/<ETH_TX_FIFO_0> SIP_ETH_TXFIFO_IP/Units/SIP_ETH_TXFIFO_IP Units/TxFIFO Pizza Arb/Pizza Arbiter
| Eth-TXFIFO-1 | 0x62000800 |  0-8 | 14-14 | 15-15 | 0x62000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | ETH_TX_FIFO/<ETH_TX_FIFO_1> SIP_ETH_TXFIFO_IP/Units/SIP_ETH_TXFIFO_IP Units/TxFIFO Pizza Arb/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| TXQ          | 0x40001500 |  0-8 | 12-12 |  9-9  | 0x40001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            | <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ_IP_dq/Scheduler/Port Arbiter Configuration/
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| RXDMA-Glue   | 0x63000000 |  0-7 |  9-9  | 10-10 | 0x63000008 |   37   |   5  |  390  | 0-0  | 1-1  | 0,1                    |  per total RxDMA BW on DP, no dummy slices| RxDMA Glue/Units/RxDMA Glue Pizza Arbiter
| TXDMA-Glue   | 0x64000000 |  0-7 |  9-9  | 10-10 | 0x64000008 |   37   |   5  |  185  | 0-0  | 1-1  | 0,1                    |  per total RxDMA BW on DP, no dummy slices| TxDMA Glue/Units/TxDMA Glue
| MPPM         | 0x47000200 |  0-8 | 14-14 | 15-15 | 0x47000208 |   93   |   4  |  372  | 0-6  | 7-7  | 0,1,2-RxDMM 3,4,5-TxDMA|  per DP BW, no dummy slices               | MPPM/Units/<MPPM_IP> MPPM IP Units/<Pizza_arb> SIP_COMMON_MODULES_IP Units/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------

Aldrin
+--------------+-----------------------------------+--------------------------------------------------+--------------------------------------------------------------------+------------------------------
|              |                       Control     |           Map                                    |                           Configuration                            |    Cider path
|              +------------+----------------------+------------+--------+--------------+-------------+------------------------+-------------------------------------------|
|   Unit       |            |          Fields      |            |        | Flds Statist | Single Fld  | Client                 |  Algo to be used                          |
|              | Register   +------+-------+-------|  Register  | number +------+-------+------+------|  Code                  |                                           |
|              |            | Slice| Map   |  WC   |            |        |  per | Total | Slice|Slice |                        |                                           |
|              |            |To-Run|Load-En|  mode |            |        |  reg |       |  Map | En   |                        |                                           |/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+--------------------------------
| RXDMA-0      | 0x17002A00 |  0-8 | 14-14 | 15-15 | 0x17002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | RXDMA/Units/<RXDMA_IP> RxDMA %a/<Pizza Arbiter> SIP_COMMON_MODULES_IP Units/Pizza Arbiter
| RXDMA-1      | 0x68002A00 |  0-8 | 14-14 | 15-15 |    ???     |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| RXDMA-2      | 0x69002A00 |  0-8 | 14-14 | 15-15 |    ???     |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXDMA-0      | 0x26004000 |  0-8 | 14-14 | 15-15 | 0x26004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TXDMA/Units/<TXDMA_IP> TxDMA %a/<Pizza Arb> TxDMA Pizza Arb/Pizza Arbiter
| TXDMA-1      | 0x66004000 |  0-8 | 14-14 | 15-15 |    ???     |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXDMA-2      | 0x6B004000 |  0-8 | 14-14 | 15-15 |    ???     |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXFIFO-0     | 0x27000800 |  0-8 | 14-14 | 15-15 | 0x27000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TX_FIFO/Units/<TXFIFO_IP> TxFIFO %a/<Pizza Arb> TxFIFO Pizza Arb/Pizza Arbiter/
| TXFIFO-1     |    ???     |  0-8 | 14-14 | 15-15 |    ???     |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXFIFO-2     |    ???     |  0-8 | 14-14 | 15-15 |    ???     |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| TXQ          | 0x40001500 |  0-8 | 12-12 |  9-9  | 0x40001650 |   96   |   4  |  388  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            | <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ_IP_dq/Scheduler/Port Arbiter Configuration/
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| MC-RXDMA     | 0x63000000 |  0-8 | 10-10 | 11-11 | 0x63000008 |   78   |   5  |  390  | 0-1  | 2-2  | 0,1,2                  |  per total RxDMA BW on DP, no dummy slices| <MC_RXDMA_ARB_IP> MC RXDMA ARB TLU/Units/MC RXDMA ARB Pizza Arbiter
| MC-TXDMA     | 0x64000000 |  0-7 |  9-9  | 10-10 | 0x64000008 |   37   |   5  |  185  | 0-1  | 2-2  | 0,1,2                  |  [0,1,2]             no per BW allocation | <DUAL_TXDMA_ARB_IP> DUAL_TXDMA_ARB/MC_TXDMA_ARB/Pizza Arbiter
| MPPM         | 0x47000200 |  0-8 | 14-14 | 15-15 | 0x47000208 |   93   |   4  |  372  | 0-6  | 7-7  | ???                    |  not configured currently                 | MPPM/Units/<MPPM_IP> MPPM IP Units/<Pizza_arb> SIP_COMMON_MODULES_IP Units/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------

#endif

/*-----------------------------------------*/
/* data declaration                        */
/*-----------------------------------------*/


/*-----------------------------------------------------------------------
 * list of pizza arbiter units to be configured depending on device type
 *     bc2
 *     bobk-pipe0-1
 *     bobk-pipe-1
 *-----------------------------------------------------------------------
 */
#define PRV_CPSS_DXCH_BC2_BOBK_PIPE0_FIRST_MAC_CNS   0
#define PRV_CPSS_DXCH_BC2_BOBK_PIPE0_LAST_MAC_CNS   47
#define PRV_CPSS_DXCH_BC2_BOBK_PIPE1_FIRST_MAC_CNS  48
#define PRV_CPSS_DXCH_BC2_BOBK_PIPE1_LAST_MAC_CNS   72



typedef struct
{
    CPSS_PORT_SPEED_ENT speedEnm;
    GT_U32              speedGbps;
}PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC;




typedef struct
{
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_U32                          tmEnable;
    CPSS_DXCH_PA_UNIT_ENT          *unitList2ConfigurePtr;
}PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC;



typedef struct
{
    CPSS_DXCH_PA_UNIT_ENT                     unit;
    PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN fun;
}PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC;


typedef GT_STATUS  (*prvCpssDxChBc2Mapping2UnitConvFUN)
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr,
    OUT GT_U32                            portArgArr[CPSS_DXCH_PA_UNIT_MAX_E]
);


/*-----------------------------------------------*/
/*  Pizza Arbiter Worksapce (object int params   */
/*-----------------------------------------------*/
/*
 * strct: PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC
 *          this params are used to initilize the specific PA object
 *
 * Description: init params for PA object
 *
 *    supportedUnitListPtr               - list of unit supported
 *                                                       (ended by CPSS_DXCH_PA_UNIT_UNDEFINED_E )
 *    unitListNotConfByPipeBwSetListPtr; - list of units not configured by cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet()
 *                                                       (ended by CPSS_DXCH_PA_UNIT_UNDEFINED_E )
 *    speedEnm2GbpsPtr                   - list of converstion CPSS speed to Gbps (ended { CPSS_PORT_SPEED_NA_E, 0}
 *    prvCpssDxChBc2Mapping2UnitConvFUN  - function that convert CPSS mapping to device Mapping
 *                                         e.g. at DMA      -- to corresponding data path,
 *                                              at DMA GLUE -- to client 0/1 at BobK
 *    mappingTm2UnitListPtr              - list of units to be configured at specific mapping with/wo Traffic Manager
 *                                                     ended     ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E, (GT_U32)(~0) ,   (CPSS_DXCH_PA_UNIT_ENT *)NULL}
 *    txQHighSpeedPortThresh_Q20           high speed port threshod by average distance between slices.
 *                                                if smaller, than port is decalred as HighSpeed
 *    unit_2_pizzaCompFun                - list of unit --> Pizza Algorithm (for aech configured unit the correspong Pizza Algorithm)
 *                                                     ended  ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E    , (BuildPizzaDistributionFUN)NULL   }
 *    mppmClientCodingList               - mppm client coding list <unit --> client code> ,
 *                                                      ended by { CPSS_DXCH_PA_UNIT_UNDEFINED_E, ~0 , ~0 }
 *   workConservingModeOnUnitListPtr     - list of unit where WorkConserving mode shall be enabled from start
 *                                                       (ended by CPSS_DXCH_PA_UNIT_UNDEFINED_E )
 *   tmUnitClientListPtr                 - list [unit x client id ] for TM ended by
 *                                              ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E, (GT_U32)~0 } or NULL (if no TM)
 *   removeRemoteFromTxQWC                - remove remote port TXQ client from WC list
 */

typedef struct
{
    PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC                *devBwPtr;
    CPSS_DXCH_PA_UNIT_ENT                            *supportedUnitListPtr;
    CPSS_DXCH_PA_UNIT_ENT                            *unitListConfByPipeBwSetListPtr;
    PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC          *speedEnm2GbpsPtr;
    prvCpssDxChBc2Mapping2UnitConvFUN                 mapping2UnitConvFun;
    PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC          *mappingTm2UnitListPtr;
    PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC               *unit_2_pizzaCompFun;
    PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;
    PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN          txQHighSpeedPortSetFun;
    PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC             *mppmClientCodingList;
    CPSS_DXCH_PA_UNIT_ENT                            *workConservingModeOnUnitListPtr;
    GT_BOOL                                           removeRemoteFromTxQWC;
    CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;
    PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC             *tmUnitClientListPtr;

}PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC;

/*-------------------------------------------------------------*
 *  BC2                                                        *
 *-------------------------------------------------------------*/

static const PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_bc2_A0_devBW[] =
{
     /*  cc ,sysBw, dataPathN,coreBWGbps[],  txqDqN, txqDqBW[]*/
     {  175 ,  58 ,     1,             {0},       1, {  58 } }
    ,{  221 ,  67 ,     1,             {0},       1, {  67 } }
    ,{  250 ,  84 ,     1,             {0},       1, {  84 } }
    ,{  362 , 120 ,     1,             {0},       1, { 120 } }
    ,{  365 , 120 ,     1,             {0},       1, { 120 } }  /* just for GM is used in BobK*/
    ,{    0 ,   0 ,     0,             {0},       0, {   0 } }
};

static const PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_bc2_B0_devBW[] =
{
     /*  cc ,sysBw,   dataPathN,coreBWGbps[],txqDqN, txqDqBW[]*/
     {  175 ,   58,          1,        { 0 },     1, {  58 }, }
    ,{  200 ,   67,          1,        { 0 },     1, {  67 }, }
    ,{  221 ,   67,          1,        { 0 },     1, {  67 }, }
    ,{  250 ,   84,          1,        { 0 },     1, {  84 }, }
    ,{  362 ,  121,          1,        { 0 },     1, { 121 }, }
    ,{  365 ,  121,          1,        { 0 },     1, { 121 }, }  /* just for GM is used in BobK*/
    ,{  521 ,  168,          1,        { 0 },     1, { 168 }, }
    ,{    0 ,    0,          0,        { 0 },     0, {   0 }, }
};



static const PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC prv_bc2_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC *)&prv_bc2_A0_devBW[0]     }
   ,{ 1,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC *)&prv_bc2_B0_devBW[0]     }
   ,{ (GT_U32)(-1),   (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC *)NULL                     }
};

static const  CPSS_DXCH_PA_UNIT_ENT   prv_bc2_UnitList_with_ilkn[] =            /* list of units used in BC2 just pipe 0 */
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const CPSS_DXCH_PA_UNIT_ENT   prv_bc2_with_ilkn_unitListConfByPipeBwSetList[] =
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


static const  CPSS_DXCH_PA_UNIT_ENT   prv_bc2_UnitList_no_ilkn[] =            /* list of units used in BC2 just pipe 0 */
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const CPSS_DXCH_PA_UNIT_ENT   prv_bc2_no_ilkn_unitListConfByPipeBwSetList[] =
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


static const  PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC prv_bc2_speedEnm2Gbps[] =
{
     { CPSS_PORT_SPEED_1000_E ,  1 }
    ,{ CPSS_PORT_SPEED_2500_E ,  3 }
    ,{ CPSS_PORT_SPEED_5000_E ,  5 }
    ,{ CPSS_PORT_SPEED_10000_E, 10 }
    ,{ CPSS_PORT_SPEED_12000_E, 12 }
    ,{ CPSS_PORT_SPEED_20000_E, 20 }
    ,{ CPSS_PORT_SPEED_40000_E, 40 }
    ,{ CPSS_PORT_SPEED_11800_E, 12 }
    ,{ CPSS_PORT_SPEED_47200_E, 48 }
    ,{ CPSS_PORT_SPEED_NA_E,     0 }
};

static GT_STATUS  prvCpssDxChBc2Mapping2UnitConvFun
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr,
    OUT GT_U32                            portArgArr[CPSS_DXCH_PA_UNIT_MAX_E]
)
{
    GT_U32 i;
#ifndef GM_USED
    PRV_CPSS_GEN_PP_CONFIG_STC *pDev;
#endif

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);




#ifndef GM_USED
    pDev = PRV_CPSS_PP_MAC(devNum);
    if (pDev->devSubFamily != CPSS_PP_SUB_FAMILY_NONE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
#endif

    for (i = 0; i < CPSS_DXCH_PA_UNIT_MAX_E; i++)
    {
        portArgArr[i] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    }
    portArgArr[CPSS_DXCH_PA_UNIT_RXDMA_0_E        ] = portMapShadowPtr->portMap.rxDmaNum;
    portArgArr[CPSS_DXCH_PA_UNIT_TXDMA_0_E        ] = portMapShadowPtr->portMap.txDmaNum;
    portArgArr[CPSS_DXCH_PA_UNIT_TXQ_0_E          ] = portMapShadowPtr->portMap.txqNum;
    portArgArr[CPSS_DXCH_PA_UNIT_TX_FIFO_0_E      ] = portMapShadowPtr->portMap.txDmaNum;
    portArgArr[CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E  ] = portMapShadowPtr->portMap.rxDmaNum;
    portArgArr[CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E ] = portMapShadowPtr->portMap.ilknChannel;

    return GT_OK;
}


/* unit to be configured at ethernet/CPU/remote port wo TM */
static const  CPSS_DXCH_PA_UNIT_ENT prv_configureUnitList_BC2_noTM_RxDMA_TxDMA_TxQ_TxFIFO[] =
{
    CPSS_DXCH_PA_UNIT_RXDMA_0_E,
    CPSS_DXCH_PA_UNIT_TXDMA_0_E,
    CPSS_DXCH_PA_UNIT_TXQ_0_E,
    CPSS_DXCH_PA_UNIT_TX_FIFO_0_E,
    CPSS_DXCH_PA_UNIT_UNDEFINED_E
};

/* unit to be configured at ethernet/CPU/remote port with TM */
static const  CPSS_DXCH_PA_UNIT_ENT prv_configureUnitList_BC2_withTM_RxDMA_TxDMA_TxQ_TxFIFO_EthTxFIFO[] =
{
    CPSS_DXCH_PA_UNIT_RXDMA_0_E,
    CPSS_DXCH_PA_UNIT_TXDMA_0_E,
    CPSS_DXCH_PA_UNIT_TXQ_0_E,
    CPSS_DXCH_PA_UNIT_TX_FIFO_0_E,
    CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E,
    CPSS_DXCH_PA_UNIT_UNDEFINED_E
};

static const  PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC prv_bc2_mapping_tm_2_unitlist[] =
{
     { CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_FALSE     ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BC2_noTM_RxDMA_TxDMA_TxQ_TxFIFO            [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_TRUE      ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BC2_withTM_RxDMA_TxDMA_TxQ_TxFIFO_EthTxFIFO[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_FALSE     ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BC2_noTM_RxDMA_TxDMA_TxQ_TxFIFO            [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_TRUE      ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BC2_withTM_RxDMA_TxDMA_TxQ_TxFIFO_EthTxFIFO[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E         , GT_FALSE     ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BC2_noTM_RxDMA_TxDMA_TxQ_TxFIFO            [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E         , GT_TRUE      ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BC2_noTM_RxDMA_TxDMA_TxQ_TxFIFO            [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_FALSE     ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BC2_noTM_RxDMA_TxDMA_TxQ_TxFIFO            [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_TRUE      ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BC2_withTM_RxDMA_TxDMA_TxQ_TxFIFO_EthTxFIFO[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E              , (GT_U32)(~0) ,   (CPSS_DXCH_PA_UNIT_ENT *)NULL                                         }
};

static const  PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC prv_bc2_unit_2_pizzaFun[] =
{
     { CPSS_DXCH_PA_UNIT_RXDMA_0_E        , BuildPizzaDistribution            }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E        , BuildPizzaDistribution            }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_0_E          , BuildTxQPizzaDistribution         }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E      , BuildPizzaDistribution            }
    ,{ CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E  , BuildPizzaDistribution            }
    ,{ CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E , BuildPizzaDistribution            }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E    , (PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN)NULL   }
};

static const  CPSS_DXCH_PA_UNIT_ENT   prv_bc2_workConservingModeOnUnitList[] =
{
     CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const  PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC  prv_bc2_tm_unit_client_list[] =
{
     { CPSS_DXCH_PA_UNIT_TXQ_0_E,               64 }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E,             73 }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E,           73 }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E, (GT_U32)~0 }

};

/*-------------------------------------------------------------*
 *  BobK Caelum Pipe-0 pipe-1                                  *
 *-------------------------------------------------------------*/
static const  PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_bobk_cealum_devBW[] =
{
     /*  cc ,sysBw, dataPathN,coreBWGbps[] txqDqN, txqDqBW[]*/
     {  167 ,   67,         2, {48,  67 },      1,   {  67} }
    ,{  200 ,  101,         2, {48, 101 },      1,   { 101} }
    ,{  250 ,  121,         2, {48, 121 },      1,   { 121} }
    ,{  365 ,  185,         2, {48, 185 },      1,   { 185} }
    ,{    0 ,    0,         0, { 0,   0 },      0,   {   0} }
};



static const  PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC prv_bobk_caelum_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)&prv_bobk_cealum_devBW[0]     }
   ,{ (GT_U32)(-1),   (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)NULL                          }
};


static const  CPSS_DXCH_PA_UNIT_ENT   prv_bobk_Caelum_Pipe0_Pipe1_UnitList[] =  /* list of units used in BobK pipe 0 + pipe 1 (Caelum) */
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_MPPM_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const  CPSS_DXCH_PA_UNIT_ENT   prv_bobk_Caelum_pipe0_pipe1_unitListConfByPipeBwSetList[] =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


static const  PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC prv_bobk_speedEnm2Gbps[] =
{
     { CPSS_PORT_SPEED_1000_E ,  1 }
    ,{ CPSS_PORT_SPEED_2500_E ,  3 }
    ,{ CPSS_PORT_SPEED_5000_E ,  5 }
    ,{ CPSS_PORT_SPEED_10000_E, 10 }
    ,{ CPSS_PORT_SPEED_12000_E, 12 }
    ,{ CPSS_PORT_SPEED_20000_E, 20 }
    ,{ CPSS_PORT_SPEED_29090_E, 40 }
    ,{ CPSS_PORT_SPEED_40000_E, 40 }
    ,{ CPSS_PORT_SPEED_11800_E, 12 }
    ,{ CPSS_PORT_SPEED_47200_E, 48 }
    ,{ CPSS_PORT_SPEED_NA_E,     0 }
};

/* unit to be configured at ethernet/CPU/remote port wo TM */
static const  CPSS_DXCH_PA_UNIT_ENT prv_configureUnitList_BobK_noTM_RxDMA_01_TxDMA_01_TxQ_TxFIFO_01[] =
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_MPPM_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E
};

/* unit to be configured at ethernet/CPU/remote port with TM */
static const  CPSS_DXCH_PA_UNIT_ENT prv_configureUnitList_BobK_withTM_RxDMA_01_TxDMA_01_TxQ_TxFIFO_01_EthTxFIFO_01[] =
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_MPPM_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E
};

static const  PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC prv_bobk_Caelum_pipe0_pipe1_mapping_tm_2_unitlist[] =
{
     { CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_noTM_RxDMA_01_TxDMA_01_TxQ_TxFIFO_01               [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_TRUE      , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_withTM_RxDMA_01_TxDMA_01_TxQ_TxFIFO_01_EthTxFIFO_01[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_noTM_RxDMA_01_TxDMA_01_TxQ_TxFIFO_01               [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_TRUE      , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_withTM_RxDMA_01_TxDMA_01_TxQ_TxFIFO_01_EthTxFIFO_01[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_noTM_RxDMA_01_TxDMA_01_TxQ_TxFIFO_01               [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_TRUE      , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_withTM_RxDMA_01_TxDMA_01_TxQ_TxFIFO_01_EthTxFIFO_01[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E              , (GT_U32)(~0) , (CPSS_DXCH_PA_UNIT_ENT *)NULL                                                      }
};


static GT_STATUS  prvCpssDxChBobK_Caelum_Pipe0_Pipe1_Mapping2UnitConvFun
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr,
    OUT GT_U32                            portArgArr[CPSS_DXCH_PA_UNIT_MAX_E]
)
{
    GT_U32 i;
#ifndef GM_USED
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;
#endif

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


#ifndef GM_USED
    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    if (pDev->genInfo.devSubFamily != CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
#endif

    for (i = 0; i < CPSS_DXCH_PA_UNIT_MAX_E; i++)
    {
        portArgArr[i] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    }


    if (portMapShadowPtr->portMap.rxDmaNum <= PRV_CPSS_DXCH_BC2_BOBK_PIPE0_LAST_MAC_CNS)
    {
        portArgArr[CPSS_DXCH_PA_UNIT_RXDMA_0_E      ] = portMapShadowPtr->portMap.rxDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E] = portMapShadowPtr->portMap.rxDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E   ] = 0;
    }
    else
    {
        portArgArr[CPSS_DXCH_PA_UNIT_RXDMA_1_E      ] = portMapShadowPtr->portMap.rxDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E] = portMapShadowPtr->portMap.rxDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E   ] = 1;
    }

    if (portMapShadowPtr->portMap.txDmaNum <= PRV_CPSS_DXCH_BC2_BOBK_PIPE0_LAST_MAC_CNS)
    {
        portArgArr[CPSS_DXCH_PA_UNIT_TXDMA_0_E      ] = portMapShadowPtr->portMap.txDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_TX_FIFO_0_E    ] = portMapShadowPtr->portMap.txDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E   ] = 0;

    }
    else
    {
        portArgArr[CPSS_DXCH_PA_UNIT_TXDMA_1_E      ] = portMapShadowPtr->portMap.txDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_TX_FIFO_1_E    ] = portMapShadowPtr->portMap.txDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E   ] = 1;
    }
    portArgArr[CPSS_DXCH_PA_UNIT_TXQ_0_E          ] = portMapShadowPtr->portMap.txqNum;
    return GT_OK;
}

static const  PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC prv_bobk_Caelum_pipe0_pipe1_unit_2_pizzaFun[] =
{
     { CPSS_DXCH_PA_UNIT_RXDMA_0_E       ,  BuildPizzaDistributionWithDummySlice         }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E       ,  BuildPizzaDistributionWithDummySlice         }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E     ,  BuildPizzaDistributionWithDummySlice         }
    ,{ CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E ,  BuildPizzaDistribution                       }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_0_E         ,  BuildTxQPizzaDistribution                    }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_1_E       ,  BuildPizzaDistributionWithDummySlice         }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E       ,  BuildPizzaDistributionWithDummySlice         }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E     ,  BuildPizzaDistributionWithDummySlice         }
    ,{ CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E ,  BuildPizzaDistribution                       }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E    ,  BuildPizzaDistribution                       }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E    ,  BuildPizzaDistribution                       }
    ,{ CPSS_DXCH_PA_UNIT_MPPM_E          ,  BuildPizzaDistribution                       }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E     , (PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN )NULL}
};

static const  PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC     prv_bobk_Caelum_pipe0_pipe1_mppmCoding[] =
{
     /* unit                           client code ,   client weight */
     {CPSS_DXCH_PA_UNIT_RXDMA_0_E   ,            0 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_RXDMA_1_E   ,            1 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_TXDMA_0_E   ,            2 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_TXDMA_1_E   ,            3 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_UNDEFINED_E , (GT_U32)(~0) ,  (GT_U32)(~0)}
};

static const  CPSS_DXCH_PA_UNIT_ENT   prv_bobk_Caelum_workConservingModeOnUnitList[] =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


static const  PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC  prv_bobk_caelum_tm_unit_client_list[] =
{
     { CPSS_DXCH_PA_UNIT_TXQ_0_E,             64 }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E,           73 }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E,         73 }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E, (GT_U32)~0 }

};


/*-------------------------------------------------------------*
 *  BobK Cetus pipe-1                                          *
 *-------------------------------------------------------------*/
static const  PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_bobk_Cetus_devBW[] =
{
     /*  cc ,sysBw, dataPathN,coreBWGbps[], txqDqN, txqDqBW[]*/
     {  167 ,   67,         2, { 0,  67 } ,      1,  {  67 } }
    ,{  200 ,  101,         2, { 0, 101 } ,      1,  { 101 } }
    ,{  250 ,  121,         2, { 0, 121 } ,      1,  { 121 } }
    ,{  365 ,  185,         2, { 0, 185 } ,      1,  { 185 } }
    ,{    0 ,    0,         0, { 0,   0 } ,      0,  {   0 } }
};

static const  PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC prv_bobk_cetus_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)&prv_bobk_Cetus_devBW[0]     }
   ,{ (GT_U32)(-1),   (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)NULL                         }
};


static const  CPSS_DXCH_PA_UNIT_ENT   prv_bobk_Cetus_Pipe_1_UnitList[] =  /* list of units used in BobK pipe 1 (Cetus) */
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_MPPM_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const  CPSS_DXCH_PA_UNIT_ENT   prv_bobk_Cetus_pipe1_unitListConfByPipeBwSetList[] =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


/* unit to be configured at ethernet/CPU/remote port wo TM */
static const  CPSS_DXCH_PA_UNIT_ENT prv_configureUnitList_BobK_Cetus_Pipe1_noTM_RxDMA_1_TxDMA_1_TxQ_TxFIFO_1[] =
{
     CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_MPPM_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E
};

/* unit to be configured at ethernet/CPU/remote port with TM */
static const  CPSS_DXCH_PA_UNIT_ENT prv_configureUnitList_BobK_Cetus_Pipe1_withTM_RxDMA_1_TxDMA_1_TxQ_TxFIFO_1_EthTxFIFO_1[] =
{
     CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_MPPM_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E
};

static const  PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC prv_bobk_Cetus_pipe1_mapping_tm_2_unitlist[] =
{
     { CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_Cetus_Pipe1_noTM_RxDMA_1_TxDMA_1_TxQ_TxFIFO_1              [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_TRUE      , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_Cetus_Pipe1_withTM_RxDMA_1_TxDMA_1_TxQ_TxFIFO_1_EthTxFIFO_1[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_Cetus_Pipe1_noTM_RxDMA_1_TxDMA_1_TxQ_TxFIFO_1              [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_TRUE      , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_Cetus_Pipe1_withTM_RxDMA_1_TxDMA_1_TxQ_TxFIFO_1_EthTxFIFO_1[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_Cetus_Pipe1_noTM_RxDMA_1_TxDMA_1_TxQ_TxFIFO_1              [0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_TRUE      , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_BobK_Cetus_Pipe1_withTM_RxDMA_1_TxDMA_1_TxQ_TxFIFO_1_EthTxFIFO_1[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E              , (GT_U32)(~0) , (CPSS_DXCH_PA_UNIT_ENT *)NULL                                                              }
};


static GT_STATUS  prvCpssDxChBobK_Cetus_Pipe1_Mapping2UnitConvFun
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr,
    OUT GT_U32                            portArgArr[CPSS_DXCH_PA_UNIT_MAX_E]
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    if (pDev->genInfo.devSubFamily != CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for (i = 0; i < CPSS_DXCH_PA_UNIT_MAX_E; i++)
    {
        portArgArr[i] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    }


    if (portMapShadowPtr->portMap.rxDmaNum <= PRV_CPSS_DXCH_BC2_BOBK_PIPE0_LAST_MAC_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    else
    {
        portArgArr[CPSS_DXCH_PA_UNIT_RXDMA_1_E      ] = portMapShadowPtr->portMap.rxDmaNum;
        portArgArr[CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E] = portMapShadowPtr->portMap.rxDmaNum; /* Eth-TxFifo sends packet to max therefore RXDMA number is used, since  */
        portArgArr[CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E   ] = 1;                                  /*    Eth-TxFIFO hard-wired to corresponding MAC */
    }

    if (portMapShadowPtr->portMap.txDmaNum <= PRV_CPSS_DXCH_BC2_BOBK_PIPE0_LAST_MAC_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    else
    {
        portArgArr[CPSS_DXCH_PA_UNIT_TXDMA_1_E      ] = portMapShadowPtr->portMap.txDmaNum; /* TXQ--> TxDMA , TXDMA is equal to RxDMA , when port does not pass trough TM */
        portArgArr[CPSS_DXCH_PA_UNIT_TX_FIFO_1_E    ] = portMapShadowPtr->portMap.txDmaNum; /* when port pass through TM txDMA of TM is in use */
        portArgArr[CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E   ] = 1;
    }
    portArgArr[CPSS_DXCH_PA_UNIT_TXQ_0_E          ] = portMapShadowPtr->portMap.txqNum;
    return GT_OK;
}

static const  PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC prv_bobk_Cetus_pipe1_unit_2_pizzaFun[] =
{
     { CPSS_DXCH_PA_UNIT_TXQ_0_E         ,  BuildTxQPizzaDistribution              }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_1_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E     ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E ,  BuildPizzaDistribution                 }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E    ,  BuildPizzaDistribution                 }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E    ,  BuildPizzaDistribution                 }
    ,{ CPSS_DXCH_PA_UNIT_MPPM_E          ,  BuildPizzaDistribution                 }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E     , (PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN )NULL}
};


static const  PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC     prv_bobk_Cetus_pipe1_mppmCoding[] =
{

     /* unit                           client code ,   client weight */
     {CPSS_DXCH_PA_UNIT_RXDMA_1_E   ,            1 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_TXDMA_1_E   ,            3 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_UNDEFINED_E , (GT_U32)(~0) ,  (GT_U32)(~0)}
};


static const  CPSS_DXCH_PA_UNIT_ENT   prv_bobk_Cetus_workConservingModeOnUnitList[] =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const  PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC  prv_bobk_cetus_tm_unit_client_list[] =
{
     { CPSS_DXCH_PA_UNIT_TXQ_0_E,             64 }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E,           73 }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E,         73 }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E, (GT_U32)~0 }
};



/*-------------------------------------------------------------*
 *  Aldrin (3xCetus)
 *-------------------------------------------------------------*/
static const  PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_aldrin_devBW[] =
{
     /*  cc ,sysBw, dataPathN,coreBWGbps[],       txqDqN, txqDqBW[]*/
     {  200 ,  121,      3, {    41,   41,   41 },    1, { 121 } }
    ,{  250 ,  241,      3, {    81,   81,   81 },    1, { 241 } }
    ,{  365 ,  321,      3, {   145,  145,  145 },    1, { 321 } }
    ,{  480 ,  384,      3, {   156,  156,  156 },    1, { 384 } }
    ,{    0 ,    0,      0, {     0,    0,    0 },    0, {   0 } }
};

static const  PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC prv_aldrin_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*) &prv_aldrin_devBW[0]   }
   ,{ (GT_U32)(-1),   (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*) NULL                   }
};


static const  CPSS_DXCH_PA_UNIT_ENT   prv_aldrin_UnitList[] =  /* list of units used in BobK pipe 1 (Cetus) */
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
     /* dp 0 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
     /* dp 1 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
     /* dp 2 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_2_E
    /* global */
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    /* ,CPSS_DXCH_PA_UNIT_MPPM_E */
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


static const  CPSS_DXCH_PA_UNIT_ENT   prv_aldrin_unitListConfByPipeBwSetList[] =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const  PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC prv_aldrin_speedEnm2Gbps[] =
{
     { CPSS_PORT_SPEED_1000_E ,  1 }
    ,{ CPSS_PORT_SPEED_2500_E ,  3 }
    ,{ CPSS_PORT_SPEED_5000_E ,  5 }
    ,{ CPSS_PORT_SPEED_10000_E, 10 }
    ,{ CPSS_PORT_SPEED_11800_E, 12 }
    ,{ CPSS_PORT_SPEED_12000_E, 12 }
    ,{ CPSS_PORT_SPEED_12500_E, 13 }
    ,{ CPSS_PORT_SPEED_20000_E, 20 }
    ,{ CPSS_PORT_SPEED_23600_E, 24 }
    ,{ CPSS_PORT_SPEED_25000_E, 25 }
    ,{ CPSS_PORT_SPEED_29090_E, 40 }
    ,{ CPSS_PORT_SPEED_40000_E, 40 }
    ,{ CPSS_PORT_SPEED_47200_E, 48 }
    ,{ CPSS_PORT_SPEED_50000_E, 50 }
    ,{ CPSS_PORT_SPEED_NA_E,     0 }
};


/* unit to be configured at ethernet/CPU/remote port wo TM */
static const  CPSS_DXCH_PA_UNIT_ENT prv_configureUnitList_aldrin_noTM_RxDMA_012_TxDMA_012_TxQ_TxFIFO_012_TxQ[] =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
     /* dp 0 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
     /* dp 1 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
     /* dp 2 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_2_E
    /* global */
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


static const  PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC prv_aldrin_mapping_tm_2_unitlist[] =
{
     { CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_aldrin_noTM_RxDMA_012_TxDMA_012_TxQ_TxFIFO_012_TxQ[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_aldrin_noTM_RxDMA_012_TxDMA_012_TxQ_TxFIFO_012_TxQ[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_FALSE     , (CPSS_DXCH_PA_UNIT_ENT *)&prv_configureUnitList_aldrin_noTM_RxDMA_012_TxDMA_012_TxQ_TxFIFO_012_TxQ[0] }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E              , (GT_U32)(~0) , (CPSS_DXCH_PA_UNIT_ENT *)NULL                                                              }
};


static GT_STATUS  prvCpssDxCh_Aldrin_Mapping2UnitConvFun
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr,
    OUT GT_U32                          portArgArr[CPSS_DXCH_PA_UNIT_MAX_E]
)
{
    GT_STATUS rc;
    GT_U32 i;
    /* PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;*/
    GT_U32      dpIndexRxDma;        /* data Path index for RxDMA port */
    GT_U32      dpIndexTxDma;        /* data Path index for TxDMA port */
    GT_U32      dpIndexTxFifo;       /* data Path index for TxFIFO port */
    GT_U32      rxDmaOnDP;           /* local rxDma index for RxDMA port */
    GT_U32      txDmaOnDP;           /* local txDma index for TxDMA port */
    GT_U32      txFifoOnDP;          /* local rxFifo index for TxFIFO port */

    typedef struct
    {
        CPSS_DXCH_PA_UNIT_ENT rxDma[3];
        CPSS_DXCH_PA_UNIT_ENT txDma[3];
        CPSS_DXCH_PA_UNIT_ENT tcFifo[3];
    }DP_2_DMA_UNIT_STC;

    static DP_2_DMA_UNIT_STC dp2dmaUnit=
    {
         {  CPSS_DXCH_PA_UNIT_RXDMA_0_E,   CPSS_DXCH_PA_UNIT_RXDMA_1_E,    CPSS_DXCH_PA_UNIT_RXDMA_2_E   }
        ,{  CPSS_DXCH_PA_UNIT_TXDMA_0_E,   CPSS_DXCH_PA_UNIT_TXDMA_1_E,    CPSS_DXCH_PA_UNIT_TXDMA_2_E   }
        ,{  CPSS_DXCH_PA_UNIT_TX_FIFO_0_E, CPSS_DXCH_PA_UNIT_TX_FIFO_1_E,  CPSS_DXCH_PA_UNIT_TX_FIFO_2_E }
    };

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /* pDev = PRV_CPSS_DXCH_PP_MAC(devNum); */

    for (i = 0; i < CPSS_DXCH_PA_UNIT_MAX_E; i++)
    {
        portArgArr[i] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    }

    /*-------------------------------------------------------------------*
     *  convert global rxDma,TxDma,TxFifo --> to [dp, local instance]    *
     *-------------------------------------------------------------------*/
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.rxDmaNum,     /*OUT*/&dpIndexRxDma, &rxDmaOnDP  );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txDmaNum,     /*OUT*/&dpIndexTxDma, &txDmaOnDP  );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txFifoPortNum,/*OUT*/&dpIndexTxFifo,&txFifoOnDP );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (dpIndexRxDma != dpIndexTxDma && dpIndexRxDma != dpIndexTxFifo)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }



    portArgArr[dp2dmaUnit.rxDma [dpIndexRxDma ]] = rxDmaOnDP;
    portArgArr[dp2dmaUnit.txDma [dpIndexTxDma ]] = txDmaOnDP;
    portArgArr[dp2dmaUnit.tcFifo[dpIndexTxFifo]] = txFifoOnDP;
    portArgArr[CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E  ] = dpIndexRxDma;
    portArgArr[CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E  ] = dpIndexTxDma;
    portArgArr[CPSS_DXCH_PA_UNIT_TXQ_0_E       ] = portMapShadowPtr->portMap.txqNum;
    return GT_OK;
}


static const  PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC prv_aldrin_unit_2_pizzaFun[] =
{
     { CPSS_DXCH_PA_UNIT_TXQ_0_E         ,  BuildTxQPizzaDistribution              }
     /* RxDMA */
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_0_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_1_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_2_E       ,  BuildPizzaDistributionWithDummySlice   }
     /* TxDMA */
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_2_E       ,  BuildPizzaDistributionWithDummySlice   }
     /* TxDMA */
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E     ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E     ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_2_E     ,  BuildPizzaDistributionWithDummySlice   }
    /* global units */
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E    ,  BuildPizzaDistribution                 }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E    ,  BuildPizzaDistribution_AldrinTxDMAGlue }
    ,{ CPSS_DXCH_PA_UNIT_MPPM_E          ,  NULL                                   } /* currently not configured */
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E     , (PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN )NULL}
};

/* not in use */
static const  PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC     prv_aldrin_mppmCoding[] =
{
     /* unit                           client code ,   client weight */
     {CPSS_DXCH_PA_UNIT_RXDMA_0_E   ,            0 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_RXDMA_1_E   ,            1 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_RXDMA_2_E   ,            2 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_TXDMA_0_E   ,            3 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_TXDMA_1_E   ,            4 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_TXDMA_2_E   ,            5 ,            1 }
    ,{CPSS_DXCH_PA_UNIT_UNDEFINED_E , (GT_U32)(~0) ,  (GT_U32)(~0)}
};


static const  CPSS_DXCH_PA_UNIT_ENT   prv_aldrin_workConservingModeOnUnitList[] =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


/*-------------------------------------------------------------*
 *  alleycat 3X ( Aldrin + Remote Ports) (3xCetus)
 *     BW at 64BC is used as TXQ BW
 *-------------------------------------------------------------*/

static const  PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_ac3X_devBW[] =
{
     /*  cc ,sysBw, dataPathN,coreBWGbps[],       txqDqN, txqDqBW[]*/
     {  200 ,  121,      3, {    41,   41,   41 },    1, {  67 } }
    ,{  250 ,  241,      3, {    81,   81,   81 },    1, {  84 } }
    ,{  365 ,  321,      3, {   145,  145,  145 },    1, { 122 } }
    ,{  480 ,  384,      3, {   147,  147,  147 },    1, { 161 } }
    ,{    0 ,    0,      0, {     0,    0,    0 },    0, {   0 } }
};

static const  PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC prv_ac3X_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)&prv_ac3X_devBW[0]   }
   ,{ (GT_U32)(-1),   (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)NULL                 }
};


static const  PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC prv_ac3X_unit_2_pizzaFun[] =
{
     { CPSS_DXCH_PA_UNIT_TXQ_0_E         ,  BuildPizzaDistributionTxQRemotesBoundedSliceN   }
     /* RxDMA */
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_0_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_1_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_2_E       ,  BuildPizzaDistributionWithDummySlice   }
     /* TxDMA */
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E       ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_2_E       ,  BuildPizzaDistributionWithDummySlice   }
     /* TxDMA */
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E     ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E     ,  BuildPizzaDistributionWithDummySlice   }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_2_E     ,  BuildPizzaDistributionWithDummySlice   }
    /* global units */
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E    ,  BuildPizzaDistribution                 }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E    ,  BuildPizzaDistribution_AldrinTxDMAGlue }
    ,{ CPSS_DXCH_PA_UNIT_MPPM_E          ,  NULL                                   } /* currently not configured */
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E     , (PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN )NULL}
};


/*-------------------------------------------------------------*
 *  alleycat 3+ ( Aldrin + Remote Ports) (3xCetus)
 *     BW at 64BC is used as TXQ BW
 *     Otherwise using Aldrin Si values.
 *-------------------------------------------------------------*/
static const  PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_ac3plus_devBW[] =
{
     /*  cc ,sysBw, dataPathN,coreBWGbps[],       txqDqN, txqDqBW[]*/
     {  200 ,  121,      3, {    41,   41,   41 },    1, {  67 } }
    ,{  250 ,  241,      3, {    81,   81,   81 },    1, {  84 } }
    ,{  365 ,  321,      3, {   145,  145,  145 },    1, { 122 } }
    ,{  480 ,  384,      3, {   156,  156,  156 },    1, { 161 } }
    ,{    0 ,    0,      0, {     0,    0,    0 },    0, {   0 } }
};

static const  PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC prv_ac3plus_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*) &prv_ac3plus_devBW[0]   }
   ,{ (GT_U32)(-1),   (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*) NULL                   }
};


/*----------------------------------------------------------*
 *  BC3                                                     *
 *----------------------------------------------------------*/
static const  PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_bc3_devBW[] =
{
     /*  cc ,sysBw, dataPathN,                   coreBWGbps[], txqDqN,    txqDqBW[]*/
     {  425 , 1140,         6, {190, 190, 190, 190, 190, 190},      6, {190, 190, 190, 190, 190, 190}}
    ,{  525 , 1860,         6, {310, 310, 310, 310, 310, 310},      6, {310, 310, 310, 310, 310, 310}}
    ,{  572 , 2040,         6, {340, 340, 340, 340, 340, 340},      6, {340, 340, 340, 340, 340, 340}}
    ,{  600 , 2040,         6, {340, 340, 340, 340, 340, 340},      6, {340, 340, 340, 340, 340, 340}}
    ,{  625 , 2040,         6, {340, 340, 340, 340, 340, 340},      6, {340, 340, 340, 340, 340, 340}}
    ,{    0 ,    0,         0, {  0,   0,   0,   0,   0,   0},      0, {  0,   0,   0,   0,   0,   0}}
};



static const  PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC prv_bc3_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)&prv_bc3_devBW[0]     }
   ,{ (GT_U32)(-1),   (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)NULL                  }
};

#ifndef GM_USED
static const  CPSS_DXCH_PA_UNIT_ENT   prv_bc3_UnitList[] =            /* list of units used in BC3 */
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_3_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_4_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_5_E
    /* TXQ units */
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_TXQ_1_E
    ,CPSS_DXCH_PA_UNIT_TXQ_2_E
    ,CPSS_DXCH_PA_UNIT_TXQ_3_E
    ,CPSS_DXCH_PA_UNIT_TXQ_4_E
    ,CPSS_DXCH_PA_UNIT_TXQ_5_E
    /* TxDMA Units */
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_3_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_4_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_5_E

    /* TxFifo Units */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_2_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_3_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_4_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_5_E
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E         /* last unit */
};
static const  CPSS_DXCH_PA_UNIT_ENT   prv_aldrin2_UnitList[] =            /* list of units used in Aldrin2 */
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_3_E
    /* TXQ units */
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,CPSS_DXCH_PA_UNIT_TXQ_1_E
    ,CPSS_DXCH_PA_UNIT_TXQ_2_E
    ,CPSS_DXCH_PA_UNIT_TXQ_3_E
    /* TxDMA Units */
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_3_E

    /* TxFifo Units */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_2_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_3_E

    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E         /* last unit */
};
#else /*GM_USED*/
static const  CPSS_DXCH_PA_UNIT_ENT   prv_bc3_UnitList[] =            /* list of units used in BC3 */
{
    /*
            the GM supports memory space only of 'DP[0] : rxdma,txdma,txfifo,txq'
    */

     CPSS_DXCH_PA_UNIT_RXDMA_0_E
/*
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_3_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_4_E
    ,CPSS_DXCH_PA_UNIT_RXDMA_5_E
*/
    /* TXQ units */
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
/*
    ,CPSS_DXCH_PA_UNIT_TXQ_1_E
    ,CPSS_DXCH_PA_UNIT_TXQ_2_E
    ,CPSS_DXCH_PA_UNIT_TXQ_3_E
    ,CPSS_DXCH_PA_UNIT_TXQ_4_E
    ,CPSS_DXCH_PA_UNIT_TXQ_5_E
*/
    /* TxDMA Units */
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
/*
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_2_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_3_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_4_E
    ,CPSS_DXCH_PA_UNIT_TXDMA_5_E
*/
    /* TxFifo Units */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
/*
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_2_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_3_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_4_E
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_5_E
*/
    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};
static const  CPSS_DXCH_PA_UNIT_ENT   prv_aldrin2_UnitList[] =            /* list of units used in BC3 */
{
    /*
            the GM supports memory space only of 'DP[0] : rxdma,txdma,txfifo,txq'
    */

     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    /* TXQ units */
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E
    /* TxDMA Units */
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E
    /* TxFifo Units */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E

    ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};
#endif /*GM_USED*/

static const  CPSS_DXCH_PA_UNIT_ENT   prv_bc3_unitListConfByPipeBwSetList[] =
{
     CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const  PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC prv_bc3_speedEnm2Gbps[]=
{
     {         CPSS_PORT_SPEED_1000_E,    1 }
    ,{         CPSS_PORT_SPEED_2500_E,    3 }
    ,{         CPSS_PORT_SPEED_5000_E,    5 }
    ,{        CPSS_PORT_SPEED_10000_E,   10 }
    ,{        CPSS_PORT_SPEED_11800_E,   12 }
    ,{        CPSS_PORT_SPEED_12500_E,   13 }
    ,{        CPSS_PORT_SPEED_20000_E,   20 }
    ,{        CPSS_PORT_SPEED_23600_E,   24 }
    ,{        CPSS_PORT_SPEED_25000_E,   25 }
    ,{        CPSS_PORT_SPEED_29090_E,   40 }
    ,{        CPSS_PORT_SPEED_40000_E,   40 }
    ,{        CPSS_PORT_SPEED_47200_E,   48 }
    ,{        CPSS_PORT_SPEED_50000_E,   50 }
    ,{         CPSS_PORT_SPEED_100G_E,  100 }
    ,{         CPSS_PORT_SPEED_107G_E,  107 }
    ,{         CPSS_PORT_SPEED_102G_E,  102 }
    ,{         CPSS_PORT_SPEED_52500_E,  53 }
    ,{         CPSS_PORT_SPEED_26700_E,  27 }
    ,{           CPSS_PORT_SPEED_NA_E,    0 }
};



static const  PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC prv_bc3_mapping_tm_2_unitlist[] =
{
     { CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_FALSE     ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_bc3_UnitList[0]           }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E         , GT_TRUE      ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_bc3_UnitList[0]           }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_FALSE     ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_bc3_UnitList[0]           }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E             , GT_TRUE      ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_bc3_UnitList[0]           }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E         , GT_FALSE     ,   (CPSS_DXCH_PA_UNIT_ENT *)NULL                           }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E         , GT_TRUE      ,   (CPSS_DXCH_PA_UNIT_ENT *)NULL                           }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_FALSE     ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_bc3_UnitList[0]           }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , GT_TRUE      ,   (CPSS_DXCH_PA_UNIT_ENT *)&prv_bc3_UnitList[0]           }
    ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E              , (GT_U32)(~0) ,   (CPSS_DXCH_PA_UNIT_ENT *)NULL                           }
};

static const  PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC prv_bc3_unit_2_pizzaFun[] =
{
     { CPSS_DXCH_PA_UNIT_RXDMA_0_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_1_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_2_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_3_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_4_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_5_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_0_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_1_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_2_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_3_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_4_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_5_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_2_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_3_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_4_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_5_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_2_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_3_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_4_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_5_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E   , (PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN)NULL}
};

static GT_STATUS  prvCpssDxChBc3Mapping2UnitConvFun
(
    IN GT_U8                              devNum,
    IN CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr,
    OUT GT_U32                            portArgArr[CPSS_DXCH_PA_UNIT_MAX_E]
)
{
    GT_STATUS rc;
    GT_U32 i;
    /* PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;*/
    GT_U32      dpIndexRxDma;        /* data Path index for RxDMA port */
    GT_U32      dpIndexTxDma;        /* data Path index for TxDMA port */
    GT_U32      dpIndexTxFifo;       /* data Path index for TxFIFO port */
    GT_U32      rxDmaOnDP;           /* local rxDma index for RxDMA port */
    GT_U32      txDmaOnDP;           /* local txDma index for TxDMA port */
    GT_U32      txFifoOnDP;          /* local rxFifo index for TxFIFO port */
    GT_U32      txqDqIdx;
    GT_U32      txq;

    typedef struct
    {
        CPSS_DXCH_PA_UNIT_ENT rxDma [6];
        CPSS_DXCH_PA_UNIT_ENT txDma [6];
        CPSS_DXCH_PA_UNIT_ENT txFifo[6];
        CPSS_DXCH_PA_UNIT_ENT txQ   [6];
    }DP_2_DMA_UNIT_STC;

    static DP_2_DMA_UNIT_STC dp2dmaUnit=
    {
         {  CPSS_DXCH_PA_UNIT_RXDMA_0_E,   CPSS_DXCH_PA_UNIT_RXDMA_1_E,    CPSS_DXCH_PA_UNIT_RXDMA_2_E,   CPSS_DXCH_PA_UNIT_RXDMA_3_E,   CPSS_DXCH_PA_UNIT_RXDMA_4_E,    CPSS_DXCH_PA_UNIT_RXDMA_5_E    }
        ,{  CPSS_DXCH_PA_UNIT_TXDMA_0_E,   CPSS_DXCH_PA_UNIT_TXDMA_1_E,    CPSS_DXCH_PA_UNIT_TXDMA_2_E,   CPSS_DXCH_PA_UNIT_TXDMA_3_E,   CPSS_DXCH_PA_UNIT_TXDMA_4_E,    CPSS_DXCH_PA_UNIT_TXDMA_5_E    }
        ,{  CPSS_DXCH_PA_UNIT_TX_FIFO_0_E, CPSS_DXCH_PA_UNIT_TX_FIFO_1_E,  CPSS_DXCH_PA_UNIT_TX_FIFO_2_E, CPSS_DXCH_PA_UNIT_TX_FIFO_3_E, CPSS_DXCH_PA_UNIT_TX_FIFO_4_E,  CPSS_DXCH_PA_UNIT_TX_FIFO_5_E  }
        ,{  CPSS_DXCH_PA_UNIT_TXQ_0_E,     CPSS_DXCH_PA_UNIT_TXQ_1_E,      CPSS_DXCH_PA_UNIT_TXQ_2_E,     CPSS_DXCH_PA_UNIT_TXQ_3_E,     CPSS_DXCH_PA_UNIT_TXQ_4_E,      CPSS_DXCH_PA_UNIT_TXQ_5_E      }
    };

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /* pDev = PRV_CPSS_DXCH_PP_MAC(devNum); */

    for (i = 0; i < CPSS_DXCH_PA_UNIT_MAX_E; i++)
    {
        portArgArr[i] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    }

    /*-------------------------------------------------------------------*
     *  convert global rxDma,TxDma,TxFifo --> to [dp, local instance]    *
     *-------------------------------------------------------------------*/
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.rxDmaNum,     /*OUT*/&dpIndexRxDma, &rxDmaOnDP  );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txDmaNum,     /*OUT*/&dpIndexTxDma, &txDmaOnDP  );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txFifoPortNum,/*OUT*/&dpIndexTxFifo,&txFifoOnDP );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (dpIndexRxDma != dpIndexTxDma && dpIndexRxDma != dpIndexTxFifo)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPpResourcesTxqGlobal2LocalConvert(devNum,portMapShadowPtr->portMap.txqNum,/*OUT*/&txqDqIdx,&txq);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    portArgArr[dp2dmaUnit.rxDma [dpIndexRxDma ]] = rxDmaOnDP;
    portArgArr[dp2dmaUnit.txDma [dpIndexTxDma ]] = txDmaOnDP;
    portArgArr[dp2dmaUnit.txFifo[dpIndexTxFifo]] = txFifoOnDP;
    portArgArr[dp2dmaUnit.txQ   [txqDqIdx     ]] = txq;
    return GT_OK;
}

static CPSS_DXCH_PA_UNIT_ENT   prv_bc3_workConservingModeOnUnitList[] =
{
    CPSS_DXCH_PA_UNIT_TXQ_0_E
#ifndef GM_USED
   ,CPSS_DXCH_PA_UNIT_TXQ_1_E
   ,CPSS_DXCH_PA_UNIT_TXQ_2_E
   ,CPSS_DXCH_PA_UNIT_TXQ_3_E
   ,CPSS_DXCH_PA_UNIT_TXQ_4_E
   ,CPSS_DXCH_PA_UNIT_TXQ_5_E
#endif  /*GM_USED*/
   ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


/*----------------------------------------------------------
 *  WS related function
 *----------------------------------------------------------*/
GT_BOOL prvCpssDxChPortPaWorSpaceInitStatusGet
(
    INOUT PRV_CPSS_DXCH_PA_WS_SET_STC     *paWsPtr
)
{
    return paWsPtr->isInit;
}

GT_VOID prvCpssDxChPortPaWorSpaceInitStatusSet
(
    INOUT PRV_CPSS_DXCH_PA_WS_SET_STC     *paWsPtr
)
{
    paWsPtr->isInit = GT_TRUE;
}


GT_VOID prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit
(
    INOUT PRV_CPSS_DXCH_PA_WORKSPACE_STC * paWs,
    IN    PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC * wsInitParamsPtr
)
{
    GT_U32      unitIdx;
    CPSS_DXCH_PA_UNIT_ENT unitType;
    GT_U32      i;

    /*------------------------------------*
     * rex x dev_bw copy
     *------------------------------------*/
    paWs->devBwPtr = wsInitParamsPtr->devBwPtr;
    /*-----------------------------------------------------------------------------------*/
    /* BC2  workspace with ILKN init                                                     */
    /*-----------------------------------------------------------------------------------*/

    cpssOsMemSet(paWs->prv_speedEnt2MBitConvArr,  0,sizeof(paWs->prv_speedEnt2MBitConvArr));
    for (i = 0 ; wsInitParamsPtr->speedEnm2GbpsPtr[i].speedEnm != CPSS_PORT_SPEED_NA_E; i++)
    {
        CPSS_PORT_SPEED_ENT speedEnm;
        GT_U32              speedGbps;
        /* get enumerator/ speed */
        speedEnm  = wsInitParamsPtr->speedEnm2GbpsPtr[i].speedEnm;
        speedGbps = wsInitParamsPtr->speedEnm2GbpsPtr[i].speedGbps;
        /* write to proper place */
        paWs->prv_speedEnt2MBitConvArr[speedEnm] = PRV_CPSS_DXCH_PA_BW_COEFF*speedGbps;
    }

    paWs->prv_DeviceUnitListPtr = wsInitParamsPtr->supportedUnitListPtr;
    paWs->prv_unitListConfigByPipeBwSetPtr = wsInitParamsPtr->unitListConfByPipeBwSetListPtr;

    /*-----------------------------*/
    /* init map of supported units */
    /*-----------------------------*/
    for (unitIdx = 0 ; unitIdx < CPSS_DXCH_PA_UNIT_MAX_E; unitIdx++)
    {
        paWs->prv_DeviceUnitListBmp[unitIdx]             = GT_FALSE;
        paWs->prv_unitListConfigByPipeBwSetBmp[unitIdx]  = GT_FALSE;
    }
    for (unitIdx = 0 ; paWs->prv_DeviceUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWs->prv_DeviceUnitListPtr[unitIdx];
        paWs->prv_DeviceUnitListBmp[unitType] = GT_TRUE;
    }

    for (unitIdx = 0 ; wsInitParamsPtr->unitListConfByPipeBwSetListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = wsInitParamsPtr->unitListConfByPipeBwSetListPtr[unitIdx];
        paWs->prv_unitListConfigByPipeBwSetBmp[unitType] = GT_TRUE;
    }

    paWs->mapping2unitConvFunPtr = wsInitParamsPtr->mapping2UnitConvFun;

    /*-----------------------------------------------------------------------------------*/
    /* define list of configuration units for each mapping type  x TM usage(True/False)  */
    /* for interlaken channel Eth_TX_FIFO not needed                                     */
    /*-----------------------------------------------------------------------------------*/
    cpssOsMemSet(&paWs->prv_mappingType2UnitConfArr,0,sizeof(paWs->prv_mappingType2UnitConfArr));
    for (i = 0 ; wsInitParamsPtr->mappingTm2UnitListPtr[i].mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E ; i++)
    {
        CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType = prv_bc2_mapping_tm_2_unitlist[i].mappingType;
        GT_U32                          tmEnable    = prv_bc2_mapping_tm_2_unitlist[i].tmEnable;
        paWs->prv_mappingType2UnitConfArr[mappingType][tmEnable] =  wsInitParamsPtr->mappingTm2UnitListPtr[i].unitList2ConfigurePtr;
    }

    cpssOsMemSet(&paWs->prv_unit2PizzaAlgoFunArr,0,sizeof(paWs->prv_unit2PizzaAlgoFunArr));
    for ( i = 0 ; wsInitParamsPtr->unit_2_pizzaCompFun[i].unit !=  CPSS_DXCH_PA_UNIT_UNDEFINED_E ; i++)
    {
        PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN fun;
        unitType = wsInitParamsPtr->unit_2_pizzaCompFun[i].unit;
        fun      = wsInitParamsPtr->unit_2_pizzaCompFun[i].fun;
        paWs->prv_unit2PizzaAlgoFunArr[unitType] = fun;
    }
    /*----------------------------------------------*
     * high speed port threshold and functions      *
     *----------------------------------------------*/
    paWs->prv_txQHighSpeedPortThreshParams = wsInitParamsPtr->txQHighSpeedPortThreshParams;
    cpssOsMemSet(&paWs->prv_unit2HighSpeedPortConfFunArr,0,sizeof(&paWs->prv_unit2HighSpeedPortConfFunArr));
    paWs->prv_unit2HighSpeedPortConfFunArr[ CPSS_DXCH_PA_UNIT_TXQ_0_E ] = wsInitParamsPtr->txQHighSpeedPortSetFun;
    /*----------------------------------------*
     * MPPM coding scheme                     *
     *----------------------------------------*/
    paWs->mppmClientCodingListPtr =  wsInitParamsPtr->mppmClientCodingList;
    /*--------------------------------------------*
     * unit list where work conserving mode on    *
     *--------------------------------------------*/
    paWs->workConservingModeOnUnitListPtr = wsInitParamsPtr->workConservingModeOnUnitListPtr;
    paWs->removeRemoteFromTxQWC = wsInitParamsPtr->removeRemoteFromTxQWC;
    /* */
    paWs->cpuSpeedEnm = wsInitParamsPtr->cpuSpeedEnm;
    paWs->tmUnitClientListPtr   = wsInitParamsPtr->tmUnitClientListPtr;
}

/*-----------------------------------------------------------------*
 * High Speed port handling                                        *
 *-----------------------------------------------------------------*
 *
 *  BC2 :
 *     if avg dist is less than 4  , port is declared as high speed
 *  BobK :
 *     ----------------------------------------------------------*
 *     Core Clock: 365MHz:                                       *
 *     o    48G and 40G should be configured as fast port        *
 *     Core Clock: 250MHz:                                       *
 *     o    48G, 40G and 24G should be configured as fast port   *
 *     Core Clock: 200MHz:                                       *
 *     o    40G, 24G and 20G should be configured as fast port   *
 *     o    48G is not supported in this core clock.             *
 *     Core Clock: 167MHz:                                       *
 *     o    24G and 20G should be configured as fast port        *
 *     o    48G and 40G are not supported in this core clock.    *
 *     ----------------------------------------------------------*
 *     +-----+---------+-------+----------+-------+
 *     | CLK | BW Gbps | SPEED | AVG DIST | CLASS |
 *     +-----+---------+-------+----------+-------+
 *     | 167 |    67   |   48  |   1.3958 |  HIGH |
 *     | 167 |    67   |   40  |   1.6750 |  HIGH |
 *     | 200 |   100   |   48  |   2.0833 |  HIGH |
 *     | 200 |   100   |   40  |   2.5000 |  HIGH |
 *     | 250 |   121   |   48  |   2.5208 |  HIGH |
 *     | 250 |   121   |   40  |   3.0250 |  HIGH |
 *     | 167 |    67   |   20  |   3.3500 |  HIGH |
 *     | 365 |   185   |   48  |   3.8542 |  HIGH |
 *     | 365 |   185   |   40  |   4.6250 |  HIGH |
 *     | 200 |   100   |   20  |   5.0000 |  HIGH |
 *     | 250 |   121   |   20  |   6.0500 |  HIGH |
 *     +-----+---------+-------+----------+-------+   threshold 6.25 is enough
 *     | 167 |    67   |   10  |   6.7000 |   low |
 *     | 365 |   185   |   20  |   9.2500 |   low |
 *     | 200 |   100   |   10  |  10.0000 |   low |
 *     | 250 |   121   |   10  |  12.1000 |   low |
 *     | 365 |   185   |   10  |  18.5000 |   low |
 *     +-----+---------+-------+----------+-------+
 *
 *-----------------------------------------------------------------*/
#define BC2_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS     3
#define BOBK_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS    6
#define ALDRIN_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS  1  /* no high speed ports */

#define Q20_FIXP_MUL (1<<20)


#define BC2_HIGH_SPEED_PORT_THRESH_Q20_CNS     (GT_32)(4*Q20_FIXP_MUL)
#define BOBK_HIGH_SPEED_PORT_THRESH_Q20_CNS    (GT_32)(6.25*Q20_FIXP_MUL)
#define ALDRIN_HIGH_SPEED_PORT_THRESH_Q20_CNS  (GT_32)(1*Q20_FIXP_MUL)   /* no high speed ports configured at PA
                                                                            all high speed ports are configured at init time
                                                                            based on port capacity i.e. all ports supporting 40G
                                                                          */


static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_bc2_ws_init_params_no_ilkn =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr              */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_bc2_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bc2_UnitList_no_ilkn[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bc2_no_ilkn_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;     */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_bc2_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;  */,                                          prvCpssDxChBc2Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_bc2_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun   */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_bc2_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{
                                                                                               BC2_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS
                                                                                              ,BC2_HIGH_SPEED_PORT_THRESH_Q20_CNS
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN  txQHighSpeedPortSetFun */,NULL/* prvCpssDxChPortDynamicPATxQHighSpeedPortSet */
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList  */,NULL /* no MPPM */
    /* CPSS_DXCH_PA_UNIT_ENT           *workConservingModeOnUnitListPtr */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bc2_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC; */,                                          GT_FALSE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;   */,                                          CPSS_PORT_SPEED_1000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC          *tmUnitClientList  */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)   &prv_bc2_tm_unit_client_list[0]

};

static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_bc2_ws_init_params_with_ilkn =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr              */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_bc2_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bc2_UnitList_with_ilkn[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bc2_with_ilkn_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;     */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_bc2_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;  */,                                          prvCpssDxChBc2Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_bc2_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;  */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_bc2_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{
                                                                                               BC2_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS
                                                                                              ,BC2_HIGH_SPEED_PORT_THRESH_Q20_CNS
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN   txQHighSpeedPortSetFun */,NULL/* prvCpssDxChPortDynamicPATxQHighSpeedPortSet */
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList   */,NULL /* no MPPM */
    /* CPSS_DXCH_PA_UNIT_ENT           *workConservingModeOnUnitListPtr; */,(CPSS_DXCH_PA_UNIT_ENT *)                &prv_bc2_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC;  */,                                         GT_FALSE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                         CPSS_PORT_SPEED_1000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC          *tmUnitClientList   */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)  &prv_bc2_tm_unit_client_list[0]
};


static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_bobk_caelum_pipe_0_pipe_1 =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr               */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_bobk_caelum_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr   */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bobk_Caelum_Pipe0_Pipe1_UnitList[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bobk_Caelum_pipe0_pipe1_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;      */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_bobk_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;   */,                                          prvCpssDxChBobK_Caelum_Pipe0_Pipe1_Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr  */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_bobk_Caelum_pipe0_pipe1_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;   */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_bobk_Caelum_pipe0_pipe1_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{
                                                                                               BOBK_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS
                                                                                              ,BOBK_HIGH_SPEED_PORT_THRESH_Q20_CNS
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN   txQHighSpeedPortSetFun */,NULL /* prvCpssDxChPortDynamicPATxQHighSpeedPortSet*/
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList   */,(PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC *) &prv_bobk_Caelum_pipe0_pipe1_mppmCoding[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *workConservingModeOnUnitListPtr;*/,(CPSS_DXCH_PA_UNIT_ENT *)                &prv_bobk_Caelum_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC;  */,                                         GT_FALSE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                         CPSS_PORT_SPEED_1000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC          *tmUnitClientList   */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)  &prv_bobk_caelum_tm_unit_client_list[0]
};

static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_bobk_cetus_pipe_1 =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr               */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_bobk_cetus_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr   */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bobk_Cetus_Pipe_1_UnitList[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bobk_Cetus_pipe1_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;      */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_bobk_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;   */,                                          prvCpssDxChBobK_Cetus_Pipe1_Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr  */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_bobk_Cetus_pipe1_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;   */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_bobk_Cetus_pipe1_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{
                                                                                               BOBK_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS
                                                                                              ,BOBK_HIGH_SPEED_PORT_THRESH_Q20_CNS
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN   txQHighSpeedPortSetFun */,NULL /* prvCpssDxChPortDynamicPATxQHighSpeedPortSet */
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList   */,(PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC *) &prv_bobk_Cetus_pipe1_mppmCoding[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *workConservingModeOnUnitListPtr;*/,(CPSS_DXCH_PA_UNIT_ENT *)                &prv_bobk_Cetus_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC;  */,                                         GT_FALSE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                         CPSS_PORT_SPEED_1000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC           *tmUnitClientList  */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)  &prv_bobk_cetus_tm_unit_client_list[0]

};


static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_aldrin_ws_init =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr               */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_aldrin_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr   */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_aldrin_UnitList[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_aldrin_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;      */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_aldrin_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;   */,                                          prvCpssDxCh_Aldrin_Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr  */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_aldrin_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;   */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_aldrin_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{
                                                                                               ALDRIN_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS
                                                                                              ,ALDRIN_HIGH_SPEED_PORT_THRESH_Q20_CNS
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN   txQHighSpeedPortSetFun */,NULL /* prvCpssDxChPortDynamicPATxQHighSpeedPortSet */
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList   */,(PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC *) NULL /* &prv_aldrin_mppmCoding[0]  */
    /* CPSS_DXCH_PA_UNIT_ENT            *workConservingModeOnUnitListPtr;*/,(CPSS_DXCH_PA_UNIT_ENT *)                &prv_aldrin_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC;  */,                                         GT_FALSE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                         CPSS_PORT_SPEED_1000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC           *tmUnitClientList  */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)  (PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)NULL
};


static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_alleyCat3X_ws =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr               */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_ac3X_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr   */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_aldrin_UnitList[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_aldrin_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;      */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_aldrin_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;   */,                                          prvCpssDxCh_Aldrin_Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr  */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_aldrin_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;   */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_ac3X_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{
                                                                                               ALDRIN_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS
                                                                                              ,ALDRIN_HIGH_SPEED_PORT_THRESH_Q20_CNS
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN   txQHighSpeedPortSetFun */,NULL /* prvCpssDxChPortDynamicPATxQHighSpeedPortSet */
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList   */,(PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC *) NULL /* &prv_aldrin_mppmCoding[0]  */
    /* CPSS_DXCH_PA_UNIT_ENT            *workConservingModeOnUnitListPtr;*/,(CPSS_DXCH_PA_UNIT_ENT *)                &prv_aldrin_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC;  */,                                         GT_TRUE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                         CPSS_PORT_SPEED_1000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC           *tmUnitClientList  */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)  (PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)NULL
};

static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_alleyCat3plus_ws =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr               */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_ac3plus_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr   */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_aldrin_UnitList[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_aldrin_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;      */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_aldrin_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;   */,                                          prvCpssDxCh_Aldrin_Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr  */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_aldrin_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;   */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_ac3X_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{
                                                                                               ALDRIN_MIN_INTERSLICE_GAP_OPTIMAL_DISTR_CNS
                                                                                              ,ALDRIN_HIGH_SPEED_PORT_THRESH_Q20_CNS
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN   txQHighSpeedPortSetFun */,NULL /* prvCpssDxChPortDynamicPATxQHighSpeedPortSet */
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList   */,(PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC *) NULL /* &prv_aldrin_mppmCoding[0]  */
    /* CPSS_DXCH_PA_UNIT_ENT            *workConservingModeOnUnitListPtr;*/,(CPSS_DXCH_PA_UNIT_ENT *)                &prv_aldrin_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC;  */,                                         GT_TRUE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                         CPSS_PORT_SPEED_1000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC           *tmUnitClientList  */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)  (PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)NULL
};



static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_bc3_ws_init_params =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr               */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_bc3_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr   */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bc3_UnitList[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bc3_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;      */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_bc3_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;   */,                                          prvCpssDxChBc3Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr  */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_bc3_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;   */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_bc3_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{          /* there is no high speed ports */
                                                                                               0
                                                                                              ,0
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN   txQHighSpeedPortSetFun */,NULL /* prvCpssDxChPortDynamicPATxQHighSpeedPortSet */
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList   */,(PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC *) NULL /* no MPPM */
    /* CPSS_DXCH_PA_UNIT_ENT            *workConservingModeOnUnitListPtr;*/,(CPSS_DXCH_PA_UNIT_ENT *)                &prv_bc3_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC;  */,                                         GT_FALSE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                         CPSS_PORT_SPEED_10000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC          *tmUnitClientList   */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)  (PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)NULL  /* no TM */
};

/*----------------------------------------------------------*
 *  ALDRIN2                                                    *
 *----------------------------------------------------------*/
static const  PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC  prv_aldrin2_devBW[] =
{
     /*  cc ,sysBw, dataPathN,                   coreBWGbps[], txqDqN,    txqDqBW[]*/
     {  450 ,  900,         4, {240, 240, 210, 210,   0,   0},  4, {240, 240, 210, 210,   0,   0}}
    ,{  525 , 1050,         4, {300, 300, 240, 240,   0,   0},  4, {300, 300, 240, 240,   0,   0}}
    ,{  600 , 1220,         4, {340, 340, 310, 310,   0,   0},  4, {340, 340, 310, 310,   0,   0}}
    ,{    0 ,    0,         0, {  0,   0,   0,   0,   0,   0},  0, {  0,   0,   0,   0,   0,   0}}
};

static const  PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC prv_aldrin2_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)&prv_aldrin2_devBW[0]    }
   ,{ (GT_U32)(-1),   (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)NULL                  }
};

static const  PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC prv_aldrin2_unit_2_pizzaFun[] =
{
     { CPSS_DXCH_PA_UNIT_RXDMA_0_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_1_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_2_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_3_E     , BuildPizzaDistributionWithDummySlice  }

    ,{ CPSS_DXCH_PA_UNIT_TXQ_0_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_1_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_2_E       , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_3_E       , BuildPizzaDistributionWithDummySlice  }

    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_2_E     , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_3_E     , BuildPizzaDistributionWithDummySlice  }

    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_2_E   , BuildPizzaDistributionWithDummySlice  }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_3_E   , BuildPizzaDistributionWithDummySlice  }

    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E   , (PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN)NULL}
};
static CPSS_DXCH_PA_UNIT_ENT   prv_aldrin2_workConservingModeOnUnitList[] =
{
    CPSS_DXCH_PA_UNIT_TXQ_0_E
#ifndef GM_USED
   ,CPSS_DXCH_PA_UNIT_TXQ_1_E
   ,CPSS_DXCH_PA_UNIT_TXQ_2_E
   ,CPSS_DXCH_PA_UNIT_TXQ_3_E
#endif /*GM_USED*/
   ,CPSS_DXCH_PA_UNIT_UNDEFINED_E           /* last unit */
};


static PRV_CPSS_DXCH_PA_WS_INIT_PARAMS_STC        prv_aldrin2_ws_init_params =
{
    /* PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC         *devBwPtr               */ (PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC*)      &prv_aldrin2_rev_x_devBW[0]
    /* CPSS_DXCH_PA_UNIT_ENT                     *supportedUnitListPtr   */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_aldrin2_UnitList[0]
    /* CPSS_DXCH_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr  */,(CPSS_DXCH_PA_UNIT_ENT*)                  &prv_bc3_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;      */,(PRV_CPSS_DXCH_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_bc3_speedEnm2Gbps[0]
    /* prvCpssDxChBc2Mapping2UnitConvFUN          mapping2UnitConvFun;   */,                                          prvCpssDxChBc3Mapping2UnitConvFun
    /* PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC   *mappingTm2UnitListPtr  */,(PRV_CPSS_DXCH_MAPPING_TM_2_UNITLIST_STC*)&prv_bc3_mapping_tm_2_unitlist[0]
    /* PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;   */,(PRV_CPSS_DXCH_UNIT_2_PIZZA_FUN_STC *)    &prv_aldrin2_unit_2_pizzaFun[0]
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC   txQHighSpeedPortThreshParams;*/ ,{          /* there is no high speed ports */
                                                                                               0
                                                                                              ,0
                                                                                          }
    /* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN   txQHighSpeedPortSetFun */,NULL /* prvCpssDxChPortDynamicPATxQHighSpeedPortSet */
    /* PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC      *mppmClientCodingList   */,(PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC *) NULL /* no MPPM */
    /* CPSS_DXCH_PA_UNIT_ENT            *workConservingModeOnUnitListPtr;*/,(CPSS_DXCH_PA_UNIT_ENT *)                &prv_aldrin2_workConservingModeOnUnitList[0]
    /* GT_BOOL                                   removeRemoteFromTxQWC;  */,                                         GT_FALSE
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                         CPSS_PORT_SPEED_10000_E
    /* PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC          *tmUnitClientList   */,(PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)  (PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC*)NULL  /* no TM */
};

static PRV_CPSS_DXCH_PA_WS_SET_STC                prv_paWsSet;

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterWSGet function
* @endinternal
*
* @brief   Get Pizza Work space (object) by device
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] workSpacePtrPtr          - pointer to pointer on workspace.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSGet
(
    IN  GT_U8                            devNum,
    OUT PRV_CPSS_DXCH_PA_WORKSPACE_STC **workSpacePtrPtr
)
{
    PRV_CPSS_DXCH_PA_WORKSPACE_STC *paWsPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    CPSS_NULL_PTR_CHECK_MAC(workSpacePtrPtr);

    paWsPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr;

    *workSpacePtrPtr = paWsPtr;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterWSSupportedUnitListGet function
* @endinternal
*
* @brief   Get list of supported units
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSSupportedUnitListGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_DXCH_PA_UNIT_ENT **supportedUnitListPtrPtr
)
{
    PRV_CPSS_DXCH_PA_WORKSPACE_STC *paWsPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(supportedUnitListPtrPtr);

    paWsPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr;

    *supportedUnitListPtrPtr = paWsPtr->prv_DeviceUnitListPtr;
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSdevBwGet
(
    IN  GT_U8                    devNum,
    OUT PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC **devBwPtrPtr
)
{
    GT_U32 revision;
    GT_U32 i;
    GT_U32 coreClockMHz;
    PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC * rev_x_devBWPtr;
    GT_U32  lastFoundRevision;
    GT_BOOL isRevFound;
    PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC *devBWPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(devBwPtrPtr);

    *devBwPtrPtr = (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC*)NULL;

    rev_x_devBWPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr->devBwPtr;

    revision     = PRV_CPSS_PP_MAC(devNum)->revision;
    coreClockMHz = PRV_CPSS_PP_MAC(devNum)->coreClock;
    /*-------------------------------------------------------------------
     * find gretest revision smaller or equal to ASIC revision
     *-------------------------------------------------------------------*/
    isRevFound = GT_FALSE;
    lastFoundRevision = (GT_U32)-1;
    devBWPtr = (PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC *)NULL;
    for (i = 0 ;  rev_x_devBWPtr[i].devBWPtr != NULL; i++)
    {
        if (revision >= rev_x_devBWPtr[i].revision)
        {
            if (isRevFound == GT_FALSE)
            {
                isRevFound = GT_TRUE;
                lastFoundRevision = rev_x_devBWPtr[i].revision;
                devBWPtr = rev_x_devBWPtr[i].devBWPtr;
            }
            else /* (isRevFound == GT_TRUE) */
            {
                if (rev_x_devBWPtr[i].revision > lastFoundRevision)
                {
                    lastFoundRevision = rev_x_devBWPtr[i].revision;
                    devBWPtr = rev_x_devBWPtr[i].devBWPtr;
                }
            }
        }
    }
    if (devBWPtr == NULL)
    {
        return GT_OK;
    }
    /*------------------------------------------------
     *  find core clock
     *------------------------------------------------*/
    for (i = 0 ; devBWPtr[i].coreClock != 0 ; i++)
    {
        if (devBWPtr[i].coreClock == coreClockMHz)
        {
            *devBwPtrPtr = &devBWPtr[i]; /* coreclock found */
            break;
        }
    }
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWScpuPortSpeedGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_PORT_SPEED_ENT     *cpuSpeedEnmPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(cpuSpeedEnmPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        *cpuSpeedEnmPtr =  CPSS_PORT_SPEED_10000_E;
    }
    else
    {
        *cpuSpeedEnmPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr->cpuSpeedEnm;
    }

    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSIsAnyRemotePortDefined
(
    IN GT_U8      devNum,
    OUT GT_BOOL  *isRemoteDefPtr
)
{
    GT_STATUS            rc;
    GT_U32               maxPortNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    CPSS_NULL_PTR_CHECK_MAC(isRemoteDefPtr);

    rc = prvCpssDxChCfgAreRemotePortsBound(devNum,/*OUT*/isRemoteDefPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (GT_TRUE == *isRemoteDefPtr)
    {
        return GT_OK;
    }

    /* no remotes are defined , but may be they exist in mapping */

    maxPortNum = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        /* find local physical port number of CPU port */
        rc =  prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (GT_TRUE == portMapShadowPtr->valid)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
            {
                *isRemoteDefPtr = GT_TRUE;
                return GT_OK;
            }
        }
    }
    *isRemoteDefPtr = GT_FALSE;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterWSInit function
* @endinternal
*
* @brief   Init Pizza Arbiter WSs (objects)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSInit
(
    IN  GT_U8                   devNum
)
{
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;
    GT_BOOL isRemoteDefined;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (GT_FALSE == prvCpssDxChPortPaWorSpaceInitStatusGet(&prv_paWsSet))
    {
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_BC2_with_ilkn,              &prv_bc2_ws_init_params_with_ilkn);
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_BC2_wo_ilkn,                &prv_bc2_ws_init_params_no_ilkn);
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_BobK_Caelum_pipe0_pipe1,    &prv_bobk_caelum_pipe_0_pipe_1);
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_BobK_Cetus_pipe1,           &prv_bobk_cetus_pipe_1);
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_Aldrin,                     &prv_aldrin_ws_init);
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_AC3X,                       &prv_alleyCat3X_ws);
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_AC3plus,                    &prv_alleyCat3plus_ws);
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_BC3,                        &prv_bc3_ws_init_params);
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_ALDRIN2,                       &prv_aldrin2_ws_init_params);
        /* currently Falcon as BC3 */
        CPSS_TBD_BOOKMARK_FALCON
        prvCpssDxChPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_Falcon,                       &prv_bc3_ws_init_params);

        prvCpssDxChPortPaWorSpaceInitStatusSet(&prv_paWsSet);
    }


    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = NULL; /* PA data still are not selected */
    if (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum))
    {
        if (pDev->hwInfo.gop_ilkn.supported)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_BC2_with_ilkn;
        }
        else
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_BC2_wo_ilkn;
        }
    }
    else if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
    {
        #ifndef GM_USED
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_BobK_Cetus_pipe1;
        #else
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_BC2_wo_ilkn;
        #endif
    }
    else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
    {
        #ifndef GM_USED
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_BobK_Caelum_pipe0_pipe1;
        #else
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_BC2_wo_ilkn;
        #endif
    }
    else if(PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterWSIsAnyRemotePortDefined(devNum,/*OUT*/&isRemoteDefined);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (GT_FALSE == isRemoteDefined)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_Aldrin;
        }
        else
        if (PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX3248_CNS)
        {
            /* It should be done only for DX3248 (AC3+) PNs. */
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_AC3plus;
        }
        else
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_AC3X;
        }
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_BC3;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_ALDRIN2;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* currently Falcon as BC3 */
        CPSS_TBD_BOOKMARK_FALCON
        PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_Falcon;
    }

    /* selection was done ? */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paWsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Pizza Arbiter WS selection");
    }

    return GT_OK;
}


