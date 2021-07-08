/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPortDynamicPizzaArbiterDrv.c
*
* DESCRIPTION:
*       bobcat2 dynamic (algorithmic) pizza arbiter unit driver
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitDrv.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

#if 0


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


BC3
+--------------+-----------------------------------+--------------------------------------------------+--------------------------------------------------------------------+------------------------------
|              |                       Control     |           Map                                    |                           Configuration                            |    Cider path
|              +------------+----------------------+------------+--------+--------------+-------------+------------------------+-------------------------------------------|
|   Unit       |            |          Fields      |            |        | Flds Statist | Single Fld  | Client                 |  Algo to be used                          |
|              | Register   +------+-------+-------|  Register  | number +------+-------+------+------|  Code                  |                                           |
|              |            | Slice| Map   |  WC   |            |        |  per | Total | Slice|Slice |                        |                                           |
|              |            |To-Run|Load-En|  mode |            |        |  reg |       |  Map | En   |                        |                                           |/Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+--------------------------------
| RXDMA-0      | 0x19002A00 |  0-8 | 14-14 | 15-15 | 0x19002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | RXDMA/Units/<RXDMA_IP> RxDMA %a/<Pizza Arbiter> SIP_COMMON_MODULES_IP Units/Pizza Arbiter
| RXDMA-1      | 0x1A002A00 |  0-8 | 14-14 | 15-15 | 0x1A002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| RXDMA-2      | 0x1B002A00 |  0-8 | 14-14 | 15-15 | 0x1B002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| RXDMA-3      | 0x99002A00 |  0-8 | 14-14 | 15-15 | 0x99002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| RXDMA-4      | 0x9A002A00 |  0-8 | 14-14 | 15-15 | 0x9A002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| RXDMA-5      | 0x9B002A00 |  0-8 | 14-14 | 15-15 | 0x9B002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXDMA-0      | 0x1c004000 |  0-8 | 14-14 | 15-15 | 0x1c004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TXDMA/Units/<TXDMA_IP> TxDMA %a/<Pizza Arb> TxDMA Pizza Arb/Pizza Arbiter
| TXDMA-1      | 0x1d004000 |  0-8 | 14-14 | 15-15 | 0x1d004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXDMA-2      | 0x1e004000 |  0-8 | 14-14 | 15-15 | 0x1e004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXDMA-3      | 0x9c004000 |  0-8 | 14-14 | 15-15 | 0x9c004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXDMA-4      | 0x9d004000 |  0-8 | 14-14 | 15-15 | 0x9d004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXDMA-5      | 0x9e004000 |  0-8 | 14-14 | 15-15 | 0x9e004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXFIFO-0     | 0x1F000800 |  0-8 | 14-14 | 15-15 | 0x1F000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TX_FIFO/Units/<TXFIFO_IP> TxFIFO %a/<Pizza Arb> TxFIFO Pizza Arb/Pizza Arbiter/
| TXFIFO-1     | 0x20000800 |  0-8 | 14-14 | 15-15 | 0x20000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXFIFO-2     | 0x21000800 |  0-8 | 14-14 | 15-15 | 0x21000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXFIFO-3     | 0x9F000800 |  0-8 | 14-14 | 15-15 | 0x9F000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXFIFO-4     | 0xA0000800 |  0-8 | 14-14 | 15-15 | 0xA0000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
| TXFIFO-5     | 0xA1000800 |  0-8 | 14-14 | 15-15 | 0xA1000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      |
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| TXQ-0        | 0x0B001500 |  0-8 | 12-12 |  9-9  | 0x0B001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            | <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ_IP_dq/Scheduler/Port Arbiter Configuration/
| TXQ-1        | 0x0C001500 |  0-8 | 12-12 |  9-9  | 0x0C001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            |
| TXQ-2        | 0x0D001500 |  0-8 | 12-12 |  9-9  | 0x0D001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            |
| TXQ-3        | 0x8B001500 |  0-8 | 12-12 |  9-9  | 0x8B001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            |
| TXQ-4        | 0x8C001500 |  0-8 | 12-12 |  9-9  | 0x8C001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            |
| TXQ-5        | 0x8D001500 |  0-8 | 12-12 |  9-9  | 0x8D001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per TXQ BW, with dummy slices            |
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------


#endif



/*-------------------------------------------------------------------------
 *   Level UnitId :
 *        bobcat2PizzaArbiterUnitDrvSet
 *        bobcat2PizzaArbiterUnitDrvGet
 *        bobcat2PizzaArbiterUnitDrvWorkConservingModeGet
 *        bobcat2PizzaArbiterUnitDrvWorkConservingModeSet
 *        bobcat2PizzaArbiterUnitDrvInit
 *
 *   Level UnitDescription :
 *        internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(IN unitId, OUT unitDescrPtr);
 *        internal_bobcat2PizzaArbiterUnitDescrDrvInit
 *        internal_bobcat2PizzaArbiterUnitDescrDrvGet
 *        internal_bobcat2PizzaArbiterUnitDescrDrvSet
 *        internal_bobcat2PizzaArbiterUnitDescrDrvWorkConservingModeGet
 *        internal_bobcat2PizzaArbiterUnitDescrDrvWorkConservingModeSet
 *
 *   Data structures
 *        Unit Description List
 *-------------------------------------------------------------------------
 */




#define FLD_OFF(STR,fld)      (GT_U32)offsetof(STR,fld)
#define FLD_OFFi(STR,idx,fld) idx*sizeof(STR) + offsetof(STR,fld)


typedef struct
{
    GT_U32 ctrlRegSliceNum2Run_Offs;
    GT_U32 ctrlRegSliceNum2Run_Len ;

    GT_U32 ctrlRegMapLoadEn_Offs    ;
    GT_U32 ctrlRegMapLoadEn_Len     ;

    GT_U32 ctrlRegWorkConservEn_Offs;
    GT_U32 ctrlRegWorkConservEn_Len;
}PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC;

typedef struct
{
    GT_U32 paMapRegNum    ;
    GT_U32 paMapFldPerReg ;
    GT_U32 paMapTotalFlds ;

    GT_U32 paMapRegWorkSliceMapOffs ;
    GT_U32 paMapRegWorkSliceMapLen  ;
    GT_U32 paMapRegWorkSliceEnOffs ;
    GT_U32 paMapRegWorkSliceEnLen  ;
}PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC;

typedef struct
{
    CPSS_DXCH_PA_UNIT_ENT                         unitType;
    GT_U32                                        ctrlRegBaseAddrOffs;
    PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC *ctrlRegFldDefPtr;
    GT_U32                                        mapRegBaseAddrOffs;
    PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC  *mapRegFldDefPtr;
}PRV_CPSS_DXCH_PA_UNIT_DEF_STC;


static PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG =
{
     PRV_CPSS_DXCH_BC2_PA_CTRLREG_SLICE_NUM_2_RUN_OFFS_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_SLICE_NUM_2_RUN_LEN_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_MAP_LOAD_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_MAP_LOAD_EN_LEN_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_WORK_CONSERV_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_WORK_CONSERV_EN_LEN_CNS
};

static PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC txQ_CTRL_REG =
{
     PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_SLICE_NUM_2_RUN_OFFS_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_SLICE_NUM_2_RUN_LEN_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_MAP_LOAD_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_MAP_LOAD_EN_LEN_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_WORK_CONSERV_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_WORK_CONSERV_EN_LEN_CNS
};

static PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC ilknTxFifo_CTRL_REG =
{
     PRV_CPSS_DXCH_BC2_PA_CTRLREG_ILKN_FX_FIFO_SLICE_NUM_2_RUN_OFFS_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_ILKN_FX_FIFO_SLICE_NUM_2_RUN_LEN_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_ILKN_FX_FIFO_MAP_LOAD_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_BC2_PA_CTRLREG_ILKN_FX_FIFO_MAP_LOAD_EN_LEN_CNS
    ,(GT_U32)(~0)                                                                /* Work Conserving does not exists  */
    ,(GT_U32)(~0)
};


static PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC bobk_rxDmaGlue_txDmaGlue_CTRL_REG =
{
     PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_SLICE_NUM_2_RUN_OFFS_CNS
    ,PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_SLICE_NUM_2_RUN_LEN_CNS
    ,PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_MAP_LOAD_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_MAP_LOAD_EN_LEN_CNS
    ,PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_WORK_CONSERV_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_WORK_CONSERV_EN_LEN_CNS
};

static PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC aldrin_MC_rxDma_CTRL_REG =
{
     PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_SLICE_NUM_2_RUN_OFFS_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_SLICE_NUM_2_RUN_LEN_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_MAP_LOAD_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_MAP_LOAD_EN_LEN_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_WORK_CONSERV_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_WORK_CONSERV_EN_LEN_CNS
};

static PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC aldrin_MC_txDma_CTRL_REG =
{
     PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_SLICE_NUM_2_RUN_OFFS_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_SLICE_NUM_2_RUN_LEN_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_MAP_LOAD_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_MAP_LOAD_EN_LEN_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_WORK_CONSERV_EN_OFFS_CNS
    ,PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_WORK_CONSERV_EN_LEN_CNS
};



static PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC aldrin_TxQ_MAP_REG =
{
    /* paMapRegNum              */  PRV_CPSS_DXCH_Aldrin_PA_TXQ_MAP_REGNUM_CNS
    /* paMapFldPerReg           */ ,PRV_CPSS_DXCH_Aldrin_PA_TXQ_MAP_FLD_PER_REG_CNS
    /* paMapTotalFlds           */ ,PRV_CPSS_DXCH_Aldrin_PA_TXQ_MAP_TOTAL_FLDS_REG_CNS
    /* paMapRegWorkSliceMapOffs */ ,PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICEMAP_OFFS_CNS
    /* paMapRegWorkSliceMapLen  */ ,PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICEMAP_LEN_CNS
    /* paMapRegWorkSliceEnOffs  */ ,PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICE_EN_OFFS_CNS
    /* paMapRegWorkSliceEnLen   */ ,PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICE_EN_LEN_CNS
};


static PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG =
{
    /* paMapRegNum              */  PRV_CPSS_DXCH_BC2_PA_MAP_REGNUM_CNS
    /* paMapFldPerReg           */ ,PRV_CPSS_DXCH_BC2_PA_MAP_FLD_PER_REG_CNS
    /* paMapTotalFlds           */ ,PRV_CPSS_DXCH_BC2_PA_MAP_TOTAL_FLDS_REG_CNS
    /* paMapRegWorkSliceMapOffs */ ,PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICEMAP_OFFS_CNS
    /* paMapRegWorkSliceMapLen  */ ,PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICEMAP_LEN_CNS
    /* paMapRegWorkSliceEnOffs  */ ,PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICE_EN_OFFS_CNS
    /* paMapRegWorkSliceEnLen   */ ,PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICE_EN_LEN_CNS
};

static PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC ilknTxFifo_MAP_REG =
{
    /* paMapRegNum              */  PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAP_FLD_PER_REG_CNS
    /* paMapFldPerReg           */ ,PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAP_REGNUM_CNS
    /* paMapTotalFlds           */ ,PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAP_TOTAL_FLDS_REG_CNS
    /* paMapRegWorkSliceMapOffs */ ,PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAPREG_WORK_SLICEMAP_OFFS_CNS
    /* paMapRegWorkSliceMapLen  */ ,PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAPREG_WORK_SLICEMAP_LEN_CNS
    /* paMapRegWorkSliceEnOffs  */ ,PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAPREG_WORK_SLICE_EN_OFFS_CNS
    /* paMapRegWorkSliceEnLen   */ ,PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAPREG_WORK_SLICE_EN_LEN_CNS
};


static PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC bobk_rxDmaGlue_txDmaGlue_MAP_REG =
{
    /* paMapRegNum              */  PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_REGNUM_CNS
    /* paMapFldPerReg           */ ,PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_FLD_PER_REG_CNS
    /* paMapTotalFlds           */ ,PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_TOTAL_FLDS_REG_CNS
    /* paMapRegWorkSliceMapOffs */ ,PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_OFFS_CNS
    /* paMapRegWorkSliceMapLen  */ ,PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_LEN_CNS
    /* paMapRegWorkSliceEnOffs  */ ,PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_OFFS_CNS
    /* paMapRegWorkSliceEnLen   */ ,PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_LEN_CNS
};


static PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC aldrin_rxDmaGlue_MAP_REG =
{
    /* paMapRegNum              */  PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAP_REGNUM_CNS
    /* paMapFldPerReg           */ ,PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAP_FLD_PER_REG_CNS
    /* paMapTotalFlds           */ ,PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAP_TOTAL_FLDS_REG_CNS
    /* paMapRegWorkSliceMapOffs */ ,PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAPREG_WORK_SLICEMAP_OFFS_CNS
    /* paMapRegWorkSliceMapLen  */ ,PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAPREG_WORK_SLICEMAP_LEN_CNS
    /* paMapRegWorkSliceEnOffs  */ ,PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAPREG_WORK_SLICE_EN_OFFS_CNS
    /* paMapRegWorkSliceEnLen   */ ,PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAPREG_WORK_SLICE_EN_LEN_CNS
};


static PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC aldrin_txDmaGlue_MAP_REG =
{
    /* paMapRegNum              */  PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAP_REGNUM_CNS
    /* paMapFldPerReg           */ ,PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAP_FLD_PER_REG_CNS
    /* paMapTotalFlds           */ ,PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAP_TOTAL_FLDS_REG_CNS
    /* paMapRegWorkSliceMapOffs */ ,PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_OFFS_CNS
    /* paMapRegWorkSliceMapLen  */ ,PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_LEN_CNS
    /* paMapRegWorkSliceEnOffs  */ ,PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_OFFS_CNS
    /* paMapRegWorkSliceEnLen   */ ,PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_LEN_CNS
};


static PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC MPPM_MAP_REG =
{
    /* paMapRegNum              */  PRV_CPSS_DXCH_BobK_PA_MPPM_MAP_REGNUM_CNS
    /* paMapFldPerReg           */ ,PRV_CPSS_DXCH_BobK_PA_MPPM_MAP_FLD_PER_REG_CNS
    /* paMapTotalFlds           */ ,PRV_CPSS_DXCH_BobK_PA_MPPM_MAP_TOTAL_FLDS_REG_CNS
    /* paMapRegWorkSliceMapOffs */ ,PRV_CPSS_DXCH_BobK_PA_MPPM_MAPREG_WORK_SLICEMAP_OFFS_CNS
    /* paMapRegWorkSliceMapLen  */ ,PRV_CPSS_DXCH_BobK_PA_MPPM_MAPREG_WORK_SLICEMAP_LEN_CNS
    /* paMapRegWorkSliceEnOffs  */ ,PRV_CPSS_DXCH_BobK_PA_MPPM_MAPREG_WORK_SLICE_EN_OFFS_CNS
    /* paMapRegWorkSliceEnLen   */ ,PRV_CPSS_DXCH_BobK_PA_MPPM_MAPREG_WORK_SLICE_EN_LEN_CNS
};


/*---------------------------------------------------------*
 * unit definition
 *---------------------------------------------------------*/

/*---------------------------------------------------------*
 * RXDMA unit definition
 *---------------------------------------------------------*/

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_rxDma_0_Def =
{
     CPSS_DXCH_PA_UNIT_RXDMA_0_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[0].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[0].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_rxDma_1_Def =
{
     CPSS_DXCH_PA_UNIT_RXDMA_1_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[1].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[1].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_rxDma_2_Def =
{
     CPSS_DXCH_PA_UNIT_RXDMA_2_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[2].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[2].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_rxDma_3_Def =
{
     CPSS_DXCH_PA_UNIT_RXDMA_3_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[3].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[3].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_rxDma_4_Def =
{
     CPSS_DXCH_PA_UNIT_RXDMA_4_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[4].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[4].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_rxDma_5_Def =
{
     CPSS_DXCH_PA_UNIT_RXDMA_5_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[5].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, rxDMA[5].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};


/*---------------------------------------------------------*
 * TXQ unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bc2_bobk_txQDef =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterConfig)
    ,&txQ_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterMap[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};


static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_aldrin_txQDef =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterConfig)
    ,&txQ_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterMap[0])
    ,&aldrin_TxQ_MAP_REG
};


static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bc3_txQDef_0 =
{
     CPSS_DXCH_PA_UNIT_TXQ_0_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterConfig)
    ,&txQ_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterMap[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};


static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bc3_txQDef_1 =
{
     CPSS_DXCH_PA_UNIT_TXQ_1_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[1].scheduler.portArbiterConfig.portsArbiterConfig)
    ,&txQ_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[1].scheduler.portArbiterConfig.portsArbiterMap[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bc3_txQDef_2 =
{
     CPSS_DXCH_PA_UNIT_TXQ_2_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[2].scheduler.portArbiterConfig.portsArbiterConfig)
    ,&txQ_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[2].scheduler.portArbiterConfig.portsArbiterMap[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bc3_txQDef_3 =
{
     CPSS_DXCH_PA_UNIT_TXQ_3_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[3].scheduler.portArbiterConfig.portsArbiterConfig)
    ,&txQ_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[3].scheduler.portArbiterConfig.portsArbiterMap[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bc3_txQDef_4 =
{
     CPSS_DXCH_PA_UNIT_TXQ_4_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[4].scheduler.portArbiterConfig.portsArbiterConfig)
    ,&txQ_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[4].scheduler.portArbiterConfig.portsArbiterMap[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bc3_txQDef_5 =
{
     CPSS_DXCH_PA_UNIT_TXQ_5_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[5].scheduler.portArbiterConfig.portsArbiterConfig)
    ,&txQ_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TXQ.dq[5].scheduler.portArbiterConfig.portsArbiterMap[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};


/*---------------------------------------------------------*
 * TXDMA unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txDma_0_Def =
{
     CPSS_DXCH_PA_UNIT_TXDMA_0_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[0].txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[0].txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txDma_1_Def =
{
     CPSS_DXCH_PA_UNIT_TXDMA_1_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[1].txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[1].txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txDma_2_Def =
{
     CPSS_DXCH_PA_UNIT_TXDMA_2_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[2].txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[2].txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txDma_3_Def =
{
     CPSS_DXCH_PA_UNIT_TXDMA_3_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[3].txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[3].txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txDma_4_Def =
{
     CPSS_DXCH_PA_UNIT_TXDMA_4_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[4].txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[4].txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txDma_5_Def =
{
     CPSS_DXCH_PA_UNIT_TXDMA_5_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[5].txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txDMA[5].txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

/*---------------------------------------------------------*
 * TXFIFO unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txFifo_0_Def =
{
     CPSS_DXCH_PA_UNIT_TX_FIFO_0_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[0].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[0].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txFifo_1_Def =
{
     CPSS_DXCH_PA_UNIT_TX_FIFO_1_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[1].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[1].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txFifo_2_Def =
{
     CPSS_DXCH_PA_UNIT_TX_FIFO_2_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[2].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[2].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txFifo_3_Def =
{
     CPSS_DXCH_PA_UNIT_TX_FIFO_3_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[3].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[3].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txFifo_4_Def =
{
     CPSS_DXCH_PA_UNIT_TX_FIFO_4_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[4].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[4].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};


static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_txFifo_5_Def =
{
     CPSS_DXCH_PA_UNIT_TX_FIFO_5_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[5].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, txFIFO[5].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

/*---------------------------------------------------------*
 * Eth-TXFIFO unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_ethTxFifoDef =
{
     CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, SIP_ETH_TXFIFO[0].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, SIP_ETH_TXFIFO[0].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_ethTxFifo_1_Def =
{
     CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, SIP_ETH_TXFIFO[1].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, SIP_ETH_TXFIFO[1].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&rxDma_TxDma_TxQ_TxFifo_EthTxFifo_MAP_REG
};

/*---------------------------------------------------------*
 * Ilkn-TXFIFO unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_ilknTxFifoDef =
{
     CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, SIP_ILKN_TXFIFO.ilknTxFIFOArbiter.pizzaArbiterCtrlReg)
    ,&ilknTxFifo_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, SIP_ILKN_TXFIFO.ilknTxFIFOArbiter.pizzaSlicePortMap[0])
    ,&ilknTxFifo_MAP_REG
};


/*---------------------------------------------------------*
 * Global (System level) unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bobk_rxDmaGlue_Def =
{
     CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, RX_DMA_GLUE.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&bobk_rxDmaGlue_txDmaGlue_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, RX_DMA_GLUE.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&bobk_rxDmaGlue_txDmaGlue_MAP_REG
};


static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_bobk_txDmaGlue_Def =
{
     CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TX_DMA_GLUE.pizzaArbiter.pizzaArbiterCtrlReg )
    ,&bobk_rxDmaGlue_txDmaGlue_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TX_DMA_GLUE.pizzaArbiter.pizzaArbiterConfigReg[0] )
    ,&bobk_rxDmaGlue_txDmaGlue_MAP_REG
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_aldrin_rxDmaGlue_Def =
{
     CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, RX_DMA_GLUE.pizzaArbiter.pizzaArbiterCtrlReg)
    ,&aldrin_MC_rxDma_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, RX_DMA_GLUE.pizzaArbiter.pizzaArbiterConfigReg[0])
    ,&aldrin_rxDmaGlue_MAP_REG
};


static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_aldrin_txDmaGlue_Def =
{
     CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TX_DMA_GLUE.pizzaArbiter.pizzaArbiterCtrlReg )
    ,&aldrin_MC_txDma_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, TX_DMA_GLUE.pizzaArbiter.pizzaArbiterConfigReg[0] )
    ,&aldrin_txDmaGlue_MAP_REG
};



static PRV_CPSS_DXCH_PA_UNIT_DEF_STC prv_MPPM_Def =
{
     CPSS_DXCH_PA_UNIT_MPPM_E
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, MPPM.pizzaArbiter.pizzaArbiterCtrlReg )
    ,&rxDma_TxDma_TxFifo_EthTxFifo_MPPM_CTRL_REG
    ,FLD_OFF(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC, MPPM.pizzaArbiter.pizzaArbiterConfigReg[0] )
    ,&MPPM_MAP_REG
};




/*---------------------------------------*
 * data structurus                       *
 *---------------------------------------*/

typedef struct
{
    CPSS_PP_FAMILY_TYPE_ENT           family;
    CPSS_PP_SUB_FAMILY_TYPE_ENT       subfamily;
    PRV_CPSS_DXCH_PA_UNIT_DEF_STC    **unitDefListPtr;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC     *unitsDescrAllPtr;
}PRV_CPSS_DXCH_DEV_X_UNITDEFLIST;


typedef struct PRV_CPSS_DXCH_BC2_PA_UNITS_DRV_STC
{
    GT_BOOL                         isInitilaized;
    PRV_CPSS_DXCH_DEV_X_UNITDEFLIST *dev_x_unitDefList;
}PRV_CPSS_DXCH_BC2_PA_UNITS_DRV_STC;




static GT_STATUS internal_bobcat2PizzaArbiterSpecificPPUnitDescrDrvInit
(
    IN  PRV_CPSS_DXCH_PA_UNIT_DEF_STC      **prv_paUnitDef_All,
    IN  GT_U32                               size,
    OUT PRV_CPSS_DXCH_BC2_PA_UNIT_STC       *unitsDescrArr
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_PA_UNIT_DEF_STC  *pUnitDefPtr;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC  *unitsDescrPtr;

    /* init all possible definitions as invalide */
    for (i = 0 ; i < size; i++)
    {
        cpssOsMemSet(&unitsDescrArr[i],0xFF,sizeof(unitsDescrArr[i]));
        unitsDescrArr[i].unit = CPSS_DXCH_PA_UNIT_UNDEFINED_E;
    }

    /* init just specified units */
    for (i = 0 ;  prv_paUnitDef_All[i] != NULL ; i++)
    {
        pUnitDefPtr = prv_paUnitDef_All[i];
        unitsDescrPtr = &unitsDescrArr[pUnitDefPtr->unitType];

        if (unitsDescrPtr->unit == pUnitDefPtr->unitType)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* module already initialized !!! something bad  */
        }
        unitsDescrPtr->unit                                = pUnitDefPtr->unitType;
        unitsDescrPtr->ctrlReg.baseAddressPtrOffs          = pUnitDefPtr->ctrlRegBaseAddrOffs;
        unitsDescrPtr->ctrlReg.fldSliceNum2run.offset      = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegSliceNum2Run_Offs;
        unitsDescrPtr->ctrlReg.fldSliceNum2run.len         = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegSliceNum2Run_Len;
        unitsDescrPtr->ctrlReg.fldMapLoadEn.offset         = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegMapLoadEn_Offs;
        unitsDescrPtr->ctrlReg.fldMapLoadEn.len            = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegMapLoadEn_Len;
        unitsDescrPtr->ctrlReg.fldWorkConservModeEn.offset = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegWorkConservEn_Offs;
        unitsDescrPtr->ctrlReg.fldWorkConservModeEn.len    = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegWorkConservEn_Len;

        unitsDescrPtr->mapReg.baseAddressPtrOffs           = pUnitDefPtr->mapRegBaseAddrOffs;
        unitsDescrPtr->mapReg.totalReg                     = pUnitDefPtr->mapRegFldDefPtr->paMapRegNum;
        unitsDescrPtr->mapReg.numberFldsPerReg             = pUnitDefPtr->mapRegFldDefPtr->paMapFldPerReg;
        unitsDescrPtr->mapReg.totalFlds                    = pUnitDefPtr->mapRegFldDefPtr->paMapTotalFlds;
        unitsDescrPtr->mapReg.fldSlice2PortMap.offset      = pUnitDefPtr->mapRegFldDefPtr->paMapRegWorkSliceMapOffs;
        unitsDescrPtr->mapReg.fldSlice2PortMap.len         = pUnitDefPtr->mapRegFldDefPtr->paMapRegWorkSliceMapLen;
        unitsDescrPtr->mapReg.fldSliceEn.offset            = pUnitDefPtr->mapRegFldDefPtr->paMapRegWorkSliceEnOffs;
        unitsDescrPtr->mapReg.fldSliceEn.len               = pUnitDefPtr->mapRegFldDefPtr->paMapRegWorkSliceEnLen;
    }
    return GT_OK;
}



/*------------------------------------------------------------*/
/* Unit Desctition Level                                      */
/*   - Init                                                   */
/*   - Set (All)                                              */
/*   - Get (All) (still absent)                               */
/*------------------------------------------------------------*/
static GT_STATUS bobcat2PizzaArbiterUnitDrvRegAddrGet
(
    IN GT_U8     devNum,
    IN GT_U32    offsInStr,
    OUT GT_U32  **regAddrPtrPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    GT_U8                                *regsAddrVer1_U8;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(regAddrPtrPtr);

    regsAddrVer1 = &PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1;
    regsAddrVer1_U8 = (GT_U8 *)regsAddrVer1;
    *regAddrPtrPtr = (GT_U32*)&regsAddrVer1_U8[offsInStr];
    return GT_OK;
}

static GT_STATUS bobcat2PizzaArbiterUnitDrvCtrlRegAddr
(
    IN GT_U8                         devNum,
    IN PRV_CPSS_DXCH_BC2_PA_UNIT_STC *unitDescrPtr,
    OUT GT_U32                       *regAddrPtr
)
{
    GT_STATUS rc;
    GT_U32    *regBaseAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);


    rc = bobcat2PizzaArbiterUnitDrvRegAddrGet(devNum,unitDescrPtr->ctrlReg.baseAddressPtrOffs,/*OUT*/&regBaseAddrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    *regAddrPtr = *regBaseAddrPtr;
    if (*regAddrPtr == 0xfffffffb)  /* bad address stored at DB */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


static GT_STATUS bobcat2PizzaArbiterUnitDrvMapRegAddrFldOffsLenGet
(
    IN GT_U8                         devNum,
    IN PRV_CPSS_DXCH_BC2_PA_UNIT_STC *unitDescrPtr,
    IN GT_U32                         sliceId,
    OUT GT_U32                       *regAddrPtr,
    OUT GT_U32                       *fldLenPtr,
    OUT GT_U32                       *fldOffsPtr
)
{
    GT_STATUS rc;
    GT_U32    *regBaseAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fldLenPtr );
    CPSS_NULL_PTR_CHECK_MAC(fldOffsPtr);


    rc = bobcat2PizzaArbiterUnitDrvRegAddrGet(devNum,unitDescrPtr->mapReg.baseAddressPtrOffs,/*OUT*/&regBaseAddrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (sliceId >= unitDescrPtr->mapReg.totalFlds)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regBaseAddrPtr[sliceId / unitDescrPtr->mapReg.numberFldsPerReg];
    *fldLenPtr  = unitDescrPtr->mapReg.fldSlice2PortMap.len + unitDescrPtr->mapReg.fldSliceEn.len;
    *fldOffsPtr = (*fldLenPtr) * (sliceId % unitDescrPtr->mapReg.numberFldsPerReg);
    return GT_OK;
}

static GT_STATUS internal_bobcat2PizzaArbiterUnitDescrDrvGet
(
    IN  GT_U8                                             devNum,
    IN  PRV_CPSS_DXCH_BC2_PA_UNIT_STC                    *unitDescrPtr,
    OUT CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC *unitStatePtr
)
{
    GT_STATUS   rc;
    GT_U32      sliceId;
    GT_U32      regAddrCtrl;
    GT_U32      regAddrMap;
    GT_U32      fldOffs;
    GT_U32      fldLen;
    GT_U32      fldVal;
    GT_PORT_NUM portNum;
    GT_U32      portNumMask;
    GT_U32      workConservModeEn;
    GT_U32      sliceNum;
    GT_U32      mapLoadEn;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    CPSS_NULL_PTR_CHECK_MAC(unitStatePtr);

    /*---------------------------*/
    /* control register          */
    /*---------------------------*/
    rc = bobcat2PizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.offset,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.len,
                                            &mapLoadEn);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.len,
                                            &workConservModeEn);


    /*------------------*/
    /* slice number get */
    /*------------------*/
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.offset,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.len,
                                            &sliceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    unitStatePtr->totalSlicesOnUnit     = unitDescrPtr->mapReg.totalFlds;
    unitStatePtr->totalConfiguredSlices = sliceNum;
    unitStatePtr->workConservingBit     = (GT_BOOL)workConservModeEn;
    unitStatePtr->pizzaMapLoadEnBit     = (GT_BOOL)mapLoadEn;

    /*-----------------------------*/
    /* mapping register            */
    /*-----------------------------*/
    /*----------------------*/
    /* enable mapped slices */
    /*----------------------*/
    for (sliceId = 0; sliceId < unitDescrPtr->mapReg.totalFlds; sliceId++)
    {
        rc = bobcat2PizzaArbiterUnitDrvMapRegAddrFldOffsLenGet(devNum,unitDescrPtr,sliceId,
                                                                /*OUT*/&regAddrMap,&fldLen,&fldOffs);
        if (rc != GT_OK)
        {
            return rc;
        }

        fldVal  = 0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddrMap,
                                                fldOffs, fldLen, &fldVal);
        if (rc != GT_OK)
        {
            return rc;
        }
        portNumMask = (1 << unitDescrPtr->mapReg.fldSlice2PortMap.len) - 1;
        portNum  = (fldVal>>unitDescrPtr->mapReg.fldSlice2PortMap.offset) & portNumMask;

        unitStatePtr->slice_occupied_by[sliceId] = portNum;
        if (0 != (fldVal >>unitDescrPtr->mapReg.fldSliceEn.offset))
        {
            unitStatePtr->slice_enable[sliceId] = GT_TRUE;
        }
        else
        {
            unitStatePtr->slice_enable[sliceId] = GT_FALSE;
        }
    }
    return GT_OK;
}

static GT_STATUS internal_bobcat2PizzaArbiterUnitDescrDrvSet
(
    IN GT_U8                          devNum,
    IN PRV_CPSS_DXCH_BC2_PA_UNIT_STC *unitDescrPtr,
    IN GT_U32                         sliceNum,
    IN GT_U32                        *slice2PortMapArr
)
{
    GT_STATUS rc;
    GT_U32    sliceId;
    GT_U32    regAddrCtrl;
    GT_U32    regAddrMap;
    GT_U32    fldOffs;
    GT_U32    fldLen;
    GT_U32    fldVal;
    GT_PORT_NUM maxPort;
    GT_PORT_NUM portNum;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    if (sliceNum > 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(slice2PortMapArr);
        if (sliceNum > unitDescrPtr->mapReg.totalFlds)
        {
            /* return code changed from GT_BAD_PARAM to GT_NO_RESOURCE */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
    }
    if (unitDescrPtr->unit == CPSS_DXCH_PA_UNIT_TXQ_0_E)
    {
        unitDescrPtr->unit = unitDescrPtr->unit;
    }

    /*---------------------------*/
    /* load disable              */
    /*---------------------------*/
    rc = bobcat2PizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,/*OUT*/&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.offset,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.len,
                                            0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*----------------------------------------------*/
    /* Check PZ Map Load Hold bit[2] to become 0x1  */
    /*----------------------------------------------*/
#if 0
    if (unitDescrPtr->unit == CPSS_DXCH_PA_UNIT_TXQ_0_E)
    {
        GT_U32 mapLoadHold;
        GT_U32 regAddrStatus;
        PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;

        regsAddrVer1 = &PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1;



        regAddrStatus = regsAddrVer1->TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterStatus;

        do
        {
            #ifndef ASIC_SIMULATION
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddrStatus,
                                                    2,
                                                    1,
                                                    /*OUT*/&mapLoadHold);
            #else
                mapLoadHold = 1;
            #endif
        }while (mapLoadHold != 1);

    }
#endif

    /*------------------*/
    /* slice number set */
    /*------------------*/
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.offset,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.len,
                                            sliceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*-----------------------------*/
    /* mapping register            */
    /*----------------------*/
    /* enable mapped slices */
    /*----------------------*/
    if (slice2PortMapArr != NULL)
    {
        maxPort = 1<<unitDescrPtr->mapReg.fldSlice2PortMap.len;
        for (sliceId = 0; sliceId < sliceNum ; sliceId++)
        {
            rc = bobcat2PizzaArbiterUnitDrvMapRegAddrFldOffsLenGet(devNum,unitDescrPtr,sliceId,
                                                                    /*OUT*/&regAddrMap,&fldLen,&fldOffs);
            if (rc != GT_OK)
            {
                return rc;
            }

            fldVal = 0;
            portNum = slice2PortMapArr[sliceId];
            if (portNum != PA_INVALID_PORT_CNS)
            {
                if (portNum >= maxPort)
                {
                    if(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
                    {
                        /* patch for SIP_5_20 */
                        CPSS_TBD_BOOKMARK_BOBCAT3
                        /* allow the writing , any value is OK since
                           simulation/GM not care about it */
                        portNum %= maxPort;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }
                fldVal  = fldVal | (portNum << unitDescrPtr->mapReg.fldSlice2PortMap.offset);
                fldVal  = fldVal | (1 << unitDescrPtr->mapReg.fldSliceEn.offset);
            }

            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddrMap,
                                                    fldOffs, fldLen, fldVal);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /*---------------------------*/
    /* disable non-mapped slices */
    /*---------------------------*/
    for (sliceId = sliceNum; sliceId < unitDescrPtr->mapReg.totalFlds; sliceId++)
    {
        rc = bobcat2PizzaArbiterUnitDrvMapRegAddrFldOffsLenGet(devNum,unitDescrPtr,sliceId,
                                                                /*OUT*/&regAddrMap,&fldLen,&fldOffs);
        if (rc != GT_OK)
        {
            return rc;
        }

        fldVal  = 0;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddrMap,
                                                fldOffs, fldLen, fldVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /*---------------------------*/
    /* load enable               */
    /*---------------------------*/
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.offset,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.len,
                                            1);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

static GT_STATUS internal_bobcat2PizzaArbiterUnitDescrDrvSlicesMapGet
(
    IN  GT_U8                                            devNum,
    IN  PRV_CPSS_DXCH_BC2_PA_UNIT_STC                    *unitDescrPtr,
    IN  GT_U32                                           slice2PortMapArrSize,
    OUT GT_U32                                           *sliceNumPtr,
    OUT GT_U32                                           *slice2PortMapArrPtr
)
{
    GT_STATUS   rc;
    GT_U32      sliceId;
    GT_U32      regAddrCtrl;
    GT_U32      regAddrMap;
    GT_U32      fldOffs;
    GT_U32      fldLen;
    GT_U32      fldVal;
    GT_PORT_NUM portNum;
    GT_U32      portNumMask;
    GT_U32      sliceNum;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    CPSS_NULL_PTR_CHECK_MAC(sliceNumPtr);
    /* slice2PortMapArrPtr == NULL is valid */

    /*---------------------------*/
    /* control register          */
    /*---------------------------*/
    rc = bobcat2PizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*------------------*/
    /* slice number get */
    /*------------------*/
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.offset,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.len,
                                            &sliceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    *sliceNumPtr = sliceNum;

    if ((slice2PortMapArrSize == 0) || (slice2PortMapArrPtr == 0))
    {
        return GT_OK;
    }

    /*-----------------------------*/
    /* mapping register            */
    /*-----------------------------*/
    for (sliceId = 0; sliceId < sliceNum; sliceId++)
    {
        if (sliceId >= slice2PortMapArrSize)
        {
            /* output array end reached */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }

        rc = bobcat2PizzaArbiterUnitDrvMapRegAddrFldOffsLenGet(devNum,unitDescrPtr,sliceId,
                                                                /*OUT*/&regAddrMap,&fldLen,&fldOffs);
        if (rc != GT_OK)
        {
            return rc;
        }

        fldVal  = 0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddrMap,
                                                fldOffs, fldLen, &fldVal);
        if (rc != GT_OK)
        {
            return rc;
        }
        portNumMask = (1 << unitDescrPtr->mapReg.fldSlice2PortMap.len) - 1;
        portNum  = (fldVal>>unitDescrPtr->mapReg.fldSlice2PortMap.offset) & portNumMask;

        if (0 != (fldVal >>unitDescrPtr->mapReg.fldSliceEn.offset))
        {
            slice2PortMapArrPtr[sliceId] = portNum;
        }
        else
        {
            slice2PortMapArrPtr[sliceId] = PA_INVALID_PORT_CNS;
        }
    }
    return GT_OK;
}

GT_STATUS internal_bobcat2PizzaArbiterUnitDescrDrvWorkConservingModeSet
(
    IN GT_U8    devNum,
    IN PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr,
    IN GT_BOOL  status
)
{
    GT_STATUS rc;
    GT_U32    regAddrCtrl;
    GT_U32    value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    /*------------------*/
    /* control register */
    /*------------------*/
    rc = bobcat2PizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    value = (GT_U32)status;
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.len,
                                            value);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS internal_bobcat2PizzaArbiterUnitDescrDrvWorkConservingModeGet
(
    IN GT_U8    devNum,
    IN PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr,
    IN GT_BOOL *statusPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddrCtrl;
    GT_U32    value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    CPSS_NULL_PTR_CHECK_MAC(statusPtr);
    /*------------------*/
    /* control register */
    /*------------------*/
    rc = bobcat2PizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.len,
                                            &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *statusPtr = (GT_BOOL)value;
    return GT_OK;
}

GT_STATUS internal_bobcat2PizzaArbiterUnitDrvInit
(
    IN GT_U8                          devNum,
    IN PRV_CPSS_DXCH_BC2_PA_UNIT_STC *unitDescrPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddrCtrl;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    /*------------------*/
    /* control register */
    /*------------------*/
    rc = bobcat2PizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.offset,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.len,
                                            0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.len,
                                            0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*---------------------------*/
    /* disable non-mapped slices */
    /*---------------------------*/
    rc = internal_bobcat2PizzaArbiterUnitDescrDrvSet(devNum,unitDescrPtr,0,(GT_U32*)NULL);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}




/*-------------------------------------------------------------------------
 *   Level UnitId :
 *        bobcat2PizzaArbiterUnitDrvSet
 *        bobcat2PizzaArbiterUnitDrvGet
 *        bobcat2PizzaArbiterUnitDrvWorkConservingModeSet
 *        bobcat2PizzaArbiterUnitDrvWorkConservingModeGet
 *        bobcat2PizzaArbiterUnitDrvInit
 *-------------------------------------------------------------------------*/
static PRV_CPSS_DXCH_PA_UNIT_DEF_STC *prv_paUnitDef_All_bc2[] =
{
     /* RxDma */
     &prv_rxDma_0_Def
     /* TxQ */
    ,&prv_bc2_bobk_txQDef
     /* TxDma */
    ,&prv_txDma_0_Def
     /* TxFifo */
    ,&prv_txFifo_0_Def
     /* Eth-TxFifo */
    ,&prv_ethTxFifoDef
     /* Ilkn-TxFifo */
    ,&prv_ilknTxFifoDef
    ,(PRV_CPSS_DXCH_PA_UNIT_DEF_STC *)NULL
};


static PRV_CPSS_DXCH_PA_UNIT_DEF_STC *prv_paUnitDef_All_bobk[] =
{
     /* RxDma */
     &prv_rxDma_0_Def
    ,&prv_rxDma_1_Def
     /* TxQ */
    ,&prv_bc2_bobk_txQDef
     /* TxDma */
    ,&prv_txDma_0_Def
    ,&prv_txDma_1_Def
     /* TxFifo */
    ,&prv_txFifo_0_Def
    ,&prv_txFifo_1_Def
     /* Eth-TxFifo */
    ,&prv_ethTxFifoDef
    ,&prv_ethTxFifo_1_Def
     /* global  */
    ,&prv_bobk_rxDmaGlue_Def
    ,&prv_bobk_txDmaGlue_Def
    ,&prv_MPPM_Def
    ,(PRV_CPSS_DXCH_PA_UNIT_DEF_STC *)NULL
};


static PRV_CPSS_DXCH_PA_UNIT_DEF_STC *prv_paUnitDef_All_aldrin[] =
{
     /* RxDma */
     &prv_rxDma_0_Def
    ,&prv_rxDma_1_Def
    ,&prv_rxDma_2_Def
     /* TxQ */
    ,&prv_aldrin_txQDef
     /* TxDma */
    ,&prv_txDma_0_Def
    ,&prv_txDma_1_Def
    ,&prv_txDma_2_Def
     /* TxFifo */
    ,&prv_txFifo_0_Def
    ,&prv_txFifo_1_Def
    ,&prv_txFifo_2_Def
     /* global  RxDMA Glue/TxDMA glue/ NO MPPM in constrain to BobK */
    ,&prv_aldrin_rxDmaGlue_Def
    ,&prv_aldrin_txDmaGlue_Def
    ,(PRV_CPSS_DXCH_PA_UNIT_DEF_STC *)NULL
};

static PRV_CPSS_DXCH_PA_UNIT_DEF_STC *prv_paUnitDef_All_bc3[] =
{
     /* RxDma */
     &prv_rxDma_0_Def
    ,&prv_rxDma_1_Def
    ,&prv_rxDma_2_Def
    ,&prv_rxDma_3_Def
    ,&prv_rxDma_4_Def
    ,&prv_rxDma_5_Def
     /* TxQ */
    ,&prv_bc3_txQDef_0
    ,&prv_bc3_txQDef_1
    ,&prv_bc3_txQDef_2
    ,&prv_bc3_txQDef_3
    ,&prv_bc3_txQDef_4
    ,&prv_bc3_txQDef_5
     /* TxDma */
    ,&prv_txDma_0_Def
    ,&prv_txDma_1_Def
    ,&prv_txDma_2_Def
    ,&prv_txDma_3_Def
    ,&prv_txDma_4_Def
    ,&prv_txDma_5_Def
     /* TxFifo */
    ,&prv_txFifo_0_Def
    ,&prv_txFifo_1_Def
    ,&prv_txFifo_2_Def
    ,&prv_txFifo_3_Def
    ,&prv_txFifo_4_Def
    ,&prv_txFifo_5_Def
    ,(PRV_CPSS_DXCH_PA_UNIT_DEF_STC *)NULL
};
static PRV_CPSS_DXCH_PA_UNIT_DEF_STC *prv_paUnitDef_All_aldrin2[] =
{
     /* RxDma */
     &prv_rxDma_0_Def
    ,&prv_rxDma_1_Def
    ,&prv_rxDma_2_Def
    ,&prv_rxDma_3_Def
     /* TxQ */
    ,&prv_bc3_txQDef_0
    ,&prv_bc3_txQDef_1
    ,&prv_bc3_txQDef_2
    ,&prv_bc3_txQDef_3
     /* TxDma */
    ,&prv_txDma_0_Def
    ,&prv_txDma_1_Def
    ,&prv_txDma_2_Def
    ,&prv_txDma_3_Def
     /* TxFifo */
    ,&prv_txFifo_0_Def
    ,&prv_txFifo_1_Def
    ,&prv_txFifo_2_Def
    ,&prv_txFifo_3_Def

    ,(PRV_CPSS_DXCH_PA_UNIT_DEF_STC *)NULL
};



PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bc2     [CPSS_DXCH_PA_UNIT_MAX_E];
PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bobk    [CPSS_DXCH_PA_UNIT_MAX_E];
PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_aldrin  [CPSS_DXCH_PA_UNIT_MAX_E];
PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bc3     [CPSS_DXCH_PA_UNIT_MAX_E];
PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_aldrin2    [CPSS_DXCH_PA_UNIT_MAX_E];

PRV_CPSS_DXCH_DEV_X_UNITDEFLIST    prv_dev_unitDefList[] =
{
     { CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_bc2   [0]   , &prv_unitsDescrAll_bc2   [0] }
    ,{ CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E, &prv_paUnitDef_All_bobk  [0]   , &prv_unitsDescrAll_bobk  [0] }
    ,{ CPSS_PP_FAMILY_DXCH_ALDRIN_E,  CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_aldrin[0]   , &prv_unitsDescrAll_aldrin[0] }
    ,{ CPSS_PP_FAMILY_DXCH_AC3X_E,    CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_aldrin[0]   , &prv_unitsDescrAll_aldrin[0] }
    ,{ CPSS_PP_FAMILY_DXCH_BOBCAT3_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_bc3   [0]   , &prv_unitsDescrAll_bc3   [0] }
    ,{ CPSS_PP_FAMILY_DXCH_ALDRIN2_E, CPSS_PP_SUB_FAMILY_NONE_E,      &prv_paUnitDef_All_aldrin2   [0]  , &prv_unitsDescrAll_aldrin2  [0] }
    /* currently Falcon as BC3 */
    CPSS_TBD_BOOKMARK_FALCON
    ,{ CPSS_PP_FAMILY_DXCH_FALCON_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_bc3   [0]   , &prv_unitsDescrAll_bc3   [0] }
    ,{ CPSS_MAX_FAMILY,               CPSS_BAD_SUB_FAMILY,               NULL                    , NULL}
};

PRV_CPSS_DXCH_BC2_PA_UNITS_DRV_STC prv_paUnitsDrv =
{
    GT_FALSE,
    &prv_dev_unitDefList[0]
};


/*---------------------------------------*
 * function                              *
 *---------------------------------------*/
static GT_STATUS internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet
(
    IN GT_U8                            devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT           unitId,
    OUT PRV_CPSS_DXCH_BC2_PA_UNIT_STC **unitDescrPtrPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtrPtr);

    if (unitId < 0 || unitId >= CPSS_DXCH_PA_UNIT_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (prv_paUnitsDrv.isInitilaized == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something bad, driver is called to get data BEFORE it is initialized */
    }

    *unitDescrPtrPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitDrvList[unitId];
    return GT_OK;
}

GT_STATUS bobcat2PizzaArbiterUnitDrvSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId,
    IN GT_U32   sliceNum,
    IN GT_U32  *slice2PortMapArr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    CPSS_NULL_PTR_CHECK_MAC(slice2PortMapArr);

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (unitDescrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvSet(devNum,unitDescrPtr,sliceNum,slice2PortMapArr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS bobcat2PizzaArbiterUnitDrvSlicesMapGet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PA_UNIT_ENT                           unitId,
    IN  GT_U32                                         slice2PortMapArrSize,
    OUT GT_U32                                         *sliceNumPtr,
    OUT GT_U32                                         *slice2PortMapArrPtr /* can be NULL */
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
        | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    CPSS_NULL_PTR_CHECK_MAC(sliceNumPtr);

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(
        devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvSlicesMapGet(
        devNum, unitDescrPtr, slice2PortMapArrSize,
        sliceNumPtr, slice2PortMapArrPtr);
    return rc;
}

GT_STATUS bobcat2PizzaArbiterUnitDrvSlicesMapMaxSizeGet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PA_UNIT_ENT                           unitId,
    OUT GT_U32                                         *sliceMapMaxSizePtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
        | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    CPSS_NULL_PTR_CHECK_MAC(sliceMapMaxSizePtr);

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(
        devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    *sliceMapMaxSizePtr = unitDescrPtr->mapReg.totalFlds;
    return GT_OK;
}

GT_STATUS bobcat2PizzaArbiterUnitDrvGet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PA_UNIT_ENT                           unitId,
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC * unitStatePtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    CPSS_NULL_PTR_CHECK_MAC(unitStatePtr);
    rc = internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvGet(devNum,unitDescrPtr,unitStatePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS bobcat2PizzaArbiterUnitDrvWorkConservingModeSet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId,
    IN GT_BOOL               status
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum, unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset >= 32)
    {
        /* unit does not support the configuration */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvWorkConservingModeSet(devNum,unitDescrPtr,status);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS bobcat2PizzaArbiterUnitDrvWorkConservingModeGet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId,
    IN GT_BOOL              *statusPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvWorkConservingModeGet(devNum,unitDescrPtr,statusPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;

}

/**
* @internal bobcat2PizzaArbiterUnitDrvIsWorkConservingModeSupported function
* @endinternal
*
* @brief   Check if unit supports work conserving mode configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] unitId                   - unit ID
*
* @param[out] isSupportedPtr           - GT_FALSE - unit does not support configuration
*                                      - GT_TRUE - unit supports configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, unitId
* @retval GT_FAIL                  - on error
*/
GT_STATUS bobcat2PizzaArbiterUnitDrvIsWorkConservingModeSupported
(
    IN  GT_U8                 devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT unitId,
    OUT GT_BOOL              *isSupportedPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    *isSupportedPtr = (unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset >= 32) ? GT_FALSE : GT_TRUE;
    return GT_OK;
}

GT_STATUS bobcat2PizzaArbiterUnitDrvInit
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDescrPtr;
    GT_U32 i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (prv_paUnitsDrv.isInitilaized == GT_FALSE)
    {
        for (i = 0 ; prv_paUnitsDrv.dev_x_unitDefList[i].family != CPSS_MAX_FAMILY; i++)
        {
            rc = internal_bobcat2PizzaArbiterSpecificPPUnitDescrDrvInit(prv_paUnitsDrv.dev_x_unitDefList[i].unitDefListPtr,
                                                                        CPSS_DXCH_PA_UNIT_MAX_E,
                                                                        /*OUT*/prv_paUnitsDrv.dev_x_unitDefList[i].unitsDescrAllPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        prv_paUnitsDrv.isInitilaized = GT_TRUE;
    }
    /*-------------------------------*
     * find for specific device      *
     *-------------------------------*/
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitDrvList == NULL)
    {
        for (i = 0 ; prv_paUnitsDrv.dev_x_unitDefList[i].family != CPSS_MAX_FAMILY; i++)
        {
            if (   prv_paUnitsDrv.dev_x_unitDefList[i].family == PRV_CPSS_PP_MAC(devNum)->devFamily
                && prv_paUnitsDrv.dev_x_unitDefList[i].subfamily == PRV_CPSS_PP_MAC(devNum)->devSubFamily)
            {
                break;
            }
        }
        if (prv_paUnitsDrv.dev_x_unitDefList[i].family == CPSS_MAX_FAMILY)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paUnitDrvList = prv_paUnitsDrv.dev_x_unitDefList[i].unitsDescrAllPtr;
    }

    rc = internal_bobcat2PizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum, unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
    {
        rc = internal_bobcat2PizzaArbiterUnitDrvInit(devNum,unitDescrPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}



/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet function
* @endinternal
*
* @brief   Configure TxQ port work conserving mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] txqPort                  - txq port
* @param[in] status                   - enable (1) disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet
(
    IN GT_U8                     devNum,
    IN GT_U32                    txqPort,
    IN GT_BOOL                   status
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    dpIdx;
    GT_U32    localTxqPort;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E && PRV_CPSS_PP_MAC(devNum)->revision == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssDxChPpResourcesTxqGlobal2LocalConvert(devNum,txqPort,/*OUT*/&dpIdx,&localTxqPort);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dpIdx).scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[localTxqPort];
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                            PRV_CPSS_DXCH_BC2B0_PORT_WORK_CONSERV_OFFS_CNS,
                                            PRV_CPSS_DXCH_BC2B0_PORT_WORK_CONSERV_LEN_CNS,
                                            (GT_U32)status);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet function
* @endinternal
*
* @brief   Configure TxQ port work conserving mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] txqPort                  - txq port
*
* @param[out] statusPtr                enable (1) disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    txqPort,
    OUT GT_BOOL                  *statusPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    value;
    GT_U32    dpIdx;
    GT_U32    localTxqPort;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E && PRV_CPSS_PP_MAC(devNum)->revision == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    rc = prvCpssDxChPpResourcesTxqGlobal2LocalConvert(devNum,txqPort,/*OUT*/&dpIdx,&localTxqPort);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dpIdx).scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[localTxqPort];
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddr,
                                            PRV_CPSS_DXCH_BC2B0_PORT_WORK_CONSERV_OFFS_CNS,
                                            PRV_CPSS_DXCH_BC2B0_PORT_WORK_CONSERV_LEN_CNS,
                                            &value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (value > GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    *statusPtr = (GT_BOOL)value;
    return GT_OK;
}






