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
* @file prvCpssDxChPortIfModeCfgBobkResource.c
*
* @brief CPSS BobK implementation for Port interface mode resource configuration.
*
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgResource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2B0ResourceTables.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

#if 0
BobK Cetus/Caelum:
-----------------
Fields:
    +-----------------------------------------------+-------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |       |     Register      |    Fld           |
    +    Field                                      | Unit  +------------+------+----+-----+-------+    Cider
    |                                               |       |  address   |   N  |per |total|  fld  |
    |                                               |       |            |      |reg |     |       |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TxQ-Descriptor-Counter                        | TxQ   | 0x3C0A4010 | 0-71 |  1 |  72 |  0-13 | <TxQ_IP>TXQ_IP/<TxQ_IP> TXQ_QUEUE/Units/TXQ_IP_queue/Tail Drop/Tail Drop Counters/Port<%n> Desc Counter
    +-----------------------------------------------+-------+------------+------+----+-----+-------+----------------------------------------------------------------------------------------------------------
    | TXQ-Credit_Counters_Reset                     | TxQ   | 0x40000260 | 0-2  | 32 |  96 |  0-0  | <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ_IP_dq/Global/Global DQ Config/Credit Counters Reset <%n>
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA reset_desc_credits_counter_scdma  |TxDMA-0| 0x2600301C | 0-73 |  1 |  74 |  0-0  | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 2
    |                                               |TxDMA-1| 0x6600301C | 0-73 |  1 |  74 |  0-0  | TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 2
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA header txfifo counter             |TxDMA-0| 0x26005200 | 0-73 |  1 |  74 |  0-9  | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    |                                               |TxDMA-1| 0x66005200 | 0-73 |  1 |  74 |  0-9  | TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA payload txfifo counter            |TxDMA-0| 0x26005200 | 0-73 |  1 |  74 | 10-19 | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    |                                               |TxDMA-1| 0x66005200 | 0-73 |  1 |  74 | 10-19 | TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------


Resources:
    +-----------------------------------------------+--------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           |
    +    Field                                      | Unit   +------------+------+----+-----+-------+    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |
    |                                               |        |            |      |reg |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | RXDNA-SCDMA Interface     width               |RxDMA-0 | 0x17000804 | 0-72 |  1 |  73 |  0-2  | RXDMA/Units/<RXDMA0_IP> RxDMA IP Units/Single Channel DMA Configurations/SCDMA %n Configuration 0
    |                                               |RxDMA-1 | 0x68000804 | 0-72 |  1 |  73 |  0-2  | RXDMA/Units/<RXDMA1_IP> RxDMA IP Units/Single Channel DMA Configurations/SCDMA %n Configuration 0
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA desc_credits_scdma                |TxDMA-0 | 0x26003000 | 0-73 |  1 |  74 |  0-8  | TXDMA/Units/<TXDMA0_IP> TxDMA %a/TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
    |                                               |TxDMA-1 | 0x66003000 | 0-73 |  1 |  74 |  0-8  | TXDMA/Units/<TXDMA1_IP> TxDMA %a/TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA TxFIFO Header Counter Threshold   |TxDMA-0 | 0x2600300C | 0-73 |  1 |  74 |  0-9  | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    |                                               |TxDMA-1 | 0x6600300C | 0-73 |  1 |  74 |  0-9  | TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA TxFIFO Payload Counter Threshold  |TxDMA-0 | 0x2600300C | 0-73 |  1 |  74 | 10-19 | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    |                                               |TxDMA-1 | 0x6600300C | 0-73 |  1 |  74 | 10-19 | TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA Burst full threshold              |TxDMA-0 | 0x26003018 | 0-73 |  1 |  74 |  0-15 | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    |                                               |TxDMA-1 | 0x66003018 | 0-73 |  1 |  74 |  0-15 | TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA Burst almost full threshold       |TxDMA-0 | 0x26003018 | 0-73 |  1 |  74 | 16-31 | TXDMA/Units/<TXDMA0_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    |                                               |TxDMA-1 | 0x66003018 | 0-73 |  1 |  74 | 16-31 | TXDMA/Units/<TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA TxFIFO Payload Min Threshold      |TxFIFO-0| 0x27000008 | 0-73 |  1 |  74 |  0-6  | TX_FIFO/Units/<TXFIFO0_IP> TxFIFO %a/TxFIFO Global Configuration/SCDMA %p Payload Threshold
    |                    for transmission           |TxFIFO-1| 0x67000008 | 0-73 |  1 |  74 |  0-6  | TX_FIFO/Units/<TXFIFO1_IP> TxFIFO %a/TxFIFO Global Configuration/SCDMA %p Payload Threshold
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXFIFO-SCDMA Outgoing bus width               |TxFIFO-0| 0x27000600 | 0-73 |  1 |  74 |  0-2  | TX_FIFO/Units/<TXFIFO0_IP> TxFIFO %a/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
    |                                               |TxFIFO-1| 0x67000600 | 0-73 |  1 |  74 |  0-2  | TX_FIFO/Units/<TXFIFO1_IP> TxFIFO %a/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------

GOPS THRESHOLD CONFIGURATIONS
    +-----------------------------------------------+--------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           |
    +    Field                                      | Unit   +------------+------+----+-----+-------+    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |
    |                                               |        |            |      |reg |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | Giga Mac Port Fifo Min Threshold              |Giga-Mac| 0x1000001C | 0-47 |  1 |  48 |  6-13 | GOP/Units/GOP/<Gige MAC IP> Gige MAC IP Units%g/Port Internal/Port FIFO Configuration Register 1
    |                                               |        |            |56-59 |  1 |  4  |       |
    |                                               |        |            |62-62 |  1 |  1  |       |
    |                                               |        |            |64-71 |  1 |  8  |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------

Aldrin:
------

Fields:
    +-----------------------------------------------+-------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |       |     Register      |    Fld           |
    +    Field                                      | Unit  +------------+------+----+-----+-------+    Cider
    |                                               |       |  address   |   N  |per |total|  fld  |
    |                                               |       |            |      |reg |     |       |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TxQ-Descriptor-Counter                        | TxQ   | 0x3C0A4010 | 0-71 |  1 |  72 |  0-13 | <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_QUEUE/Units/TXQ_IP_queue/Tail Drop/Tail Drop Counters/Port<%n> Desc Counter
    +-----------------------------------------------+-------+------------+------+----+-----+-------+----------------------------------------------------------------------------------------------------------
    | TXQ-Credit_Counters_Reset                     | TxQ   | 0x40000260 | 0-2  | 32 |  96 |  0-0  | <TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ_IP_dq/Global/Global DQ Config/Credit Counters Reset <%n>
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA reset_desc_credits_counter_scdma  |TxDMA-0| 0x2600301C | 0-73 |  1 |  74 |  0-0  | TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 2
    |                                               |TxDMA-1| 0x6600301C | 0-73 |  1 |  74 |  0-0  |
    |                                               |TxDMA-2| 0x6A00301C | 0-73 |  1 |  74 |  0-0  |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA header txfifo counter             |TxDMA-0| 0x26005200 | 0-73 |  1 |  74 |  0-9  | TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    |                                               |TxDMA-1| 0x66005200 | 0-73 |  1 |  74 |  0-9  |
    |                                               |TxDMA-2| 0x6A005200 | 0-73 |  1 |  74 |  0-9  |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA payload txfifo counter            |TxDMA-0| 0x26005200 | 0-73 |  1 |  74 | 10-19 | TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    |                                               |TxDMA-1| 0x66005200 | 0-73 |  1 |  74 | 10-19 |
    |                                               |TxDMA-2| 0x6A005200 | 0-73 |  1 |  74 | 10-19 |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------

Resources:
    +-----------------------------------------------+--------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           |
    +    Field                                      | Unit   +------------+------+----+-----+-------+    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |
    |                                               |        |            |      |reg |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | RXDNA-SCDMA Interface     width               |RxDMA-0 | 0x17000804 | 0-72 |  1 |  73 |  0-2  | <RXDMA_IP> RxDMA %a/Single Channel DMA Configurations/SCDMA %n Configuration 0
    |                                               |RxDMA-1 | 0x68000804 | 0-72 |  1 |  73 |  0-2  |
    |                                               |RxDMA-2 | 0x69000804 | 0-72 |  1 |  73 |  0-2  |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA desc_credits_scdma                |TxDMA-0 | 0x26003000 | 0-73 |  1 |  74 |  0-8  | TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
    |                                               |TxDMA-1 | 0x66003000 | 0-73 |  1 |  74 |  0-8  |
    |                                               |TxDMA-2 | 0x6A003000 | 0-73 |  1 |  74 |  0-8  |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA TxFIFO Header Counter Threshold   |TxDMA-0 | 0x2600300C | 0-73 |  1 |  74 |  0-9  | TXDMA/Units/<TXDMA_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    |                                               |TxDMA-1 | 0x6600300C | 0-73 |  1 |  74 |  0-9  |
    |                                               |TxDMA-2 | 0x6A00300C | 0-73 |  1 |  74 |  0-9  |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA TxFIFO Payload Counter Threshold  |TxDMA-0 | 0x2600300C | 0-73 |  1 |  74 | 10-19 | TXDMA/Units/<TXDMA_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    |                                               |TxDMA-1 | 0x6600300C | 0-73 |  1 |  74 | 10-19 |
    |                                               |TxDMA-2 | 0x6A00300C | 0-73 |  1 |  74 |  0-9  |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA Burst almost full threshold       |TxDMA-0 | 0x26003018 | 0-73 |  1 |  74 |  0-15 | TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    |                                               |TxDMA-1 | 0x66003018 | 0-73 |  1 |  74 |  0-15 |
    |                                               |TxDMA-2 | 0x6A003018 | 0-73 |  1 |  74 |  0-15 |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA Burst almost full threshold       |TxDMA-0 | 0x26003018 | 0-73 |  1 |  74 | 16-31 | TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    |                                               |TxDMA-1 | 0x66003018 | 0-73 |  1 |  74 | 16-31 |
    |                                               |TxDMA-2 | 0x6A003018 | 0-73 |  1 |  74 | 16-31 |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA TxFIFO Payload Min Threshold      |TxFIFO-0| 0x27000008 | 0-73 |  1 |  74 |  0-6  | TX_FIFO/Units/<TXFIFO_IP> TxFIFO %a/TxFIFO Global Configuration/SCDMA %p Payload Threshold
    |                    for transmission           |TxFIFO-1| 0x67000008 | 0-73 |  1 |  74 |  0-6  |
    |                                               |TxFIFO-2| 0x6A000008 | 0-73 |  1 |  74 |  0-6  |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TXFIFO-SCDMA Outgoing bus width               |TxFIFO-0| 0x27000600 | 0-73 |  1 |  74 |  0-2  | TX_FIFO/Units/<TXFIFO_IP> TxFIFO %a/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
    |                                               |TxFIFO-1| 0x67000600 | 0-73 |  1 |  74 |  0-2  |
    |                                               |TxFIFO-2| 0x6A000600 | 0-73 |  1 |  74 |  0-2  |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------

GOPS THRESHOLD CONFIGURATIONS
    +-----------------------------------------------+--------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           |
    +    Field                                      | Unit   +------------+------+----+-----+-------+    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |
    |                                               |        |            |      |reg |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | Giga Mac Port Fifo Min Threshold              |Giga-Mac| 0x1000001C | 0-32 |  1 |  33 |  6-13  | GOP/Units/GOP/<Gige MAC IP> Gige MAC IP Units%g/Port Internal/Port FIFO Configuration Register 1
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------


BC3:
----

Global configurations:
    +-----------------------------------------------+-------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |       |     Register      |    Fld           |
    +    Field                                      | Unit  +------------+------+----+-----+-------+    Cider
    |                                               |       |  address   |   N  |per |total|  fld  |
    |                                               |       |            |      |reg |     |       |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
?   | SCDMA %n CT Packet Identification             |RxDMA-0| 0x19000B00 | 0-12 |  1 |  13 |  0-6  | <RxDMA_IP> RxDMA IP TLU/Units/RxDMA %a Pipe %b/Single Channel DMA Configurations/SCDMA %n CT Packet Indentification
    |                                               |RxDMA-1| 0x1A000B00 | 0-12 |  1 |  13 |  0-6  | possible values  1,      for system where all ports are 10G and lower (HW restriction -- see cider ?)
    |                                               |RxDMA-2| 0x1B000B00 | 0-12 |  1 |  13 |  0-6  |                  2(def)
    |                                               |RxDMA-3| 0x99000B00 | 0-12 |  1 |  13 |  0-6  |
    |                                               |RxDMA-4| 0x9A000B00 | 0-12 |  1 |  13 |  0-6  |
    |                                               |RxDMA-5| 0x9B000B00 | 0-12 |  1 |  13 |  0-6  |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+----------------------------------------------------------------------------------------------------------
?   | SCDMA %p guarenteed BC for CT                 |TxDMA-0| 0x1C003010 | 0-12 |  1 |  13 |  0-9  | <TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 1
    |                                               |TxDMA-1| 0x1D003010 | 0-12 |  1 |  13 |  0-9  | possible values  128,    for system where all ports are 10G and lower (HW restriction -- see cider ?)
    |                                               |TxDMA-2| 0x1E003010 | 0-12 |  1 |  13 |  0-9  |                  256 (def)
    |                                               |TxDMA-3| 0x9C003010 | 0-12 |  1 |  13 |  0-9  |
    |                                               |TxDMA-4| 0x9D003010 | 0-12 |  1 |  13 |  0-9  |
    |                                               |TxDMA-5| 0x9E003010 | 0-12 |  1 |  13 |  0-9  |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+----------------------------------------------------------------------------------------------------------
?   | Local Dev Src Port to DMA Number Map %i       |TxDMA-0| 0x1C000020 | 0-127|  4 | 512 |  0-7  | <TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Global Configurations/Local Dev Src Port to DMA Number Map %i
    |                                               |TxDMA-1| 0x1D000020 | 0-127|  4 | 512 |  0-7  |
    |                                               |TxDMA-2| 0x1E000020 | 0-127|  4 | 512 |  0-7  |
    |  MAPPING ?                                    |TxDMA-3| 0x9C000020 | 0-127|  4 | 512 |  0-7  |
    |                                               |TxDMA-4| 0x9D000020 | 0-127|  4 | 512 |  0-7  |
    |                                               |TxDMA-5| 0x9E000020 | 0-127|  4 | 512 |  0-7  |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+----------------------------------------------------------------------------------------------------------



Fields:
    +-----------------------------------------------+--------+-------------------+------------------+--------+-------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           | Is     |
    +    Field                                      | Unit   +------------+------+----+-----+-------+changed |   Cider
    |                                               |        |  address   |   N  |per |total|  fld  |        |
    |                                               |        |            |      |reg |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------
+   | TxQ-Descriptor-Counter                        |TxQ-IP  | 0x550A4010 | 0-71 |  1 |  72 |  0-15 |fld     |<TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_QUEUE/Units/TXQ_IP_queue/Tail Drop/Tail Drop Counters/Port<%n> Desc Counter
    |                                               |        |            |      |    |     |       |width   |wait for TXQ decriptor credit SCDMA %p (see below)
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXQ-Credit_Counters_Reset                     |TxQ-DQ  | 0x0B000260 | 0-2  | 32 |  96 |  0-0  |        |<TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ Dequeue %a Pipe %b/Global/Global DQ Config/Credit Counters Reset <%n>
    |                                               |        | 0x0C000260 | 0-2  | 32 |  96 |  0-0  |        |
    |                                               |        | 0x0D000260 | 0-2  | 32 |  96 |  0-0  |        |
    |                                               |        | 0x8B000260 | 0-2  | 32 |  96 |  0-0  |        |
    |                                               |        | 0x8C000260 | 0-2  | 32 |  96 |  0-0  |        |
    |                                               |        | 0x8D000260 | 0-2  | 32 |  96 |  0-0  |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA reset_desc_credits_counter_scdma  |TxDMA-0 | 0x1C00301C | 0-12 |  1 |  13 |  0-0  |        |TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 2
    |                                               |TxDMA-1 | 0x1D00301C | 0-12 |  1 |  13 |  0-0  |        |
    |                                               |TxDMA-2 | 0x1E00301C | 0-12 |  1 |  13 |  0-0  |        |
    |                                               |TxDMA-3 | 0x9C00301C | 0-12 |  1 |  13 |  0-0  |        |
    |                                               |TxDMA-4 | 0x9D00301C | 0-12 |  1 |  13 |  0-0  |        |
    |                                               |TxDMA-5 | 0x9E00301C | 0-12 |  1 |  13 |  0-0  |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA header txfifo counter             |TxDMA-0 | 0x1C005200 | 0-12 |  1 |  13 |  0-9  |        |<TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    |                                               |TxDMA-1 | 0x1D005200 | 0-12 |  1 |  13 |  0-9  |        |wait for 0
    |                                               |TxDMA-2 | 0x1E005200 | 0-12 |  1 |  13 |  0-9  |        |
    |                                               |TxDMA-3 | 0x9C005200 | 0-12 |  1 |  13 |  0-9  |        |
    |                                               |TxDMA-4 | 0x9D005200 | 0-12 |  1 |  13 |  0-9  |        |
    |                                               |TxDMA-5 | 0x9E005200 | 0-12 |  1 |  13 |  0-9  |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA payload txfifo counter            |TxDMA-0 | 0x1C005200 | 0-12 |  1 |  13 | 10-19 |        |<TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    |                                               |TxDMA-1 | 0x1D005200 | 0-12 |  1 |  13 | 10-19 |        |wait for 0
    |                                               |TxDMA-2 | 0x1E005200 | 0-12 |  1 |  13 | 10-19 |        |
    |                                               |TxDMA-0 | 0x9C005200 | 0-12 |  1 |  13 | 10-19 |        |
    |                                               |TxDMA-1 | 0x9D005200 | 0-12 |  1 |  13 | 10-19 |        |
    |                                               |TxDMA-2 | 0x9E005200 | 0-12 |  1 |  13 | 10-19 |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------

Resources:
    +-----------------------------------------------+--------+-------------------+------------------+--------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           | Is     |
    +    Field                                      | Unit   +------------+------+----+-----+-------+changed |    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |        |
    |                                               |        |            |      |reg |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | RXDNA-SCDMA Interface     width               |RxDMA-0 | 0x19000804 | 0-12 |  1 |  13 |  0-2  |        | <RxDMA_IP> RxDMA IP TLU/Units/RxDMA %a Pipe %b/Single Channel DMA Configurations/SCDMA %n Configuration 0
    |                                               |RxDMA-1 | 0x1A000804 | 0-12 |  1 |  13 |  0-2  |        |
    |                                               |RxDMA-2 | 0x1B000804 | 0-12 |  1 |  13 |  0-2  |        |
    |                                               |RxDMA-3 | 0x99000804 | 0-12 |  1 |  13 |  0-2  |        |
    |                                               |RxDMA-4 | 0x9A000804 | 0-12 |  1 |  13 |  0-2  |        |
    |                                               |RxDMA-5 | 0x9B000804 | 0-12 |  1 |  13 |  0-2  |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA desc_credits_scdma                |TxDMA-0 | 0x19003000 | 0-12 |  1 |  13 |  0-8  |        | <TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
    |                                               |TxDMA-1 | 0x1A003000 | 0-12 |  1 |  13 |  0-8  |        |
    |                                               |TxDMA-2 | 0x1B003000 | 0-12 |  1 |  13 |  0-8  |        |
    |                                               |TxDMA-3 | 0x99003000 | 0-12 |  1 |  13 |  0-8  |        |
    |                                               |TxDMA-4 | 0x9A003000 | 0-12 |  1 |  13 |  0-8  |        |
    |                                               |TxDMA-5 | 0x9B003000 | 0-12 |  1 |  13 |  0-8  |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA TxFIFO Header Counter Threshold   |TxDMA-0 | 0x1900300C | 0-12 |  1 |  13 |  0-9  |        | <TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    |                                               |TxDMA-1 | 0x1A00300C | 0-12 |  1 |  13 |  0-9  |        |
    |                                               |TxDMA-2 | 0x1B00300C | 0-12 |  1 |  13 |  0-9  |        |
    |                                               |TxDMA-3 | 0x9900300C | 0-12 |  1 |  13 |  0-9  |        |
    |                                               |TxDMA-4 | 0x9A00300C | 0-12 |  1 |  13 |  0-9  |        |
    |                                               |TxDMA-5 | 0x9B00300C | 0-12 |  1 |  13 |  0-9  |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA TxFIFO Payload Counter Threshold  |TxDMA-0 | 0x1900300C | 0-12 |  1 |  13 | 10-19 |        | <TXDMA_IP> TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    |                                               |TxDMA-1 | 0x1A00300C | 0-12 |  1 |  13 | 10-19 |        |
    |                                               |TxDMA-2 | 0x1B00300C | 0-12 |  1 |  13 | 10-19 |        |
    |                                               |TxDMA-3 | 0x9900300C | 0-12 |  1 |  13 | 10-19 |        |
    |                                               |TxDMA-4 | 0x9A00300C | 0-12 |  1 |  13 | 10-19 |        |
    |                                               |TxDMA-5 | 0x9B00300C | 0-12 |  1 |  13 | 10-19 |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA Burst almost full threshold       |TxDMA-0 | 0x19003018 | 0-12 |  1 |  13 |  0-15 |        | TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    |                                               |TxDMA-1 | 0x1A003018 | 0-12 |  1 |  13 |  0-15 |        |
    |                                               |TxDMA-2 | 0x1B003018 | 0-12 |  1 |  13 |  0-15 |        |
    |                                               |TxDMA-3 | 0x99003018 | 0-12 |  1 |  13 |  0-15 |        |
    |                                               |TxDMA-4 | 0x9A003018 | 0-12 |  1 |  13 |  0-15 |        |
    |                                               |TxDMA-5 | 0x9B003018 | 0-12 |  1 |  13 |  0-15 |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA Burst almost full threshold       |TxDMA-0 | 0x19003018 | 0-12 |  1 |  13 | 16-31 |        | TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    |                                               |TxDMA-1 | 0x1A003018 | 0-12 |  1 |  13 | 16-31 |        |
    |                                               |TxDMA-2 | 0x1B003018 | 0-12 |  1 |  13 | 16-31 |        |
    |                                               |TxDMA-1 | 0x99003018 | 0-12 |  1 |  13 | 16-31 |        |
    |                                               |TxDMA-2 | 0x9A003018 | 0-12 |  1 |  13 | 16-31 |        |
    |                                               |TxDMA-2 | 0x9B003018 | 0-12 |  1 |  13 | 16-31 |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA TxFIFO Payload Min Threshold      |TxFIFO-0| 0x1F000008 | 0-12 |  1 |  13 |  0-6  |        | TxFIFO IP TLU/Units/TxFIFO %a Pipe %b/TxFIFO Global Configuration/SCDMA %p Payload Threshold
    |                    for transmission           |TxFIFO-1| 0x20000008 | 0-12 |  1 |  13 |  0-6  |        |
    |                                               |TxFIFO-2| 0x21000008 | 0-12 |  1 |  13 |  0-6  |        |
    |                                               |TxFIFO-3| 0x9F000008 | 0-12 |  1 |  13 |  0-6  |        |
    |                                               |TxFIFO-4| 0xA0000008 | 0-12 |  1 |  13 |  0-6  |        |
    |                                               |TxFIFO-5| 0xA1000008 | 0-12 |  1 |  13 |  0-6  |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXFIFO-SCDMA Outgoing bus width               |TxFIFO-0| 0x1F000600 | 0-12 |  1 |  13 |  0-2  | coding | TxFIFO IP TLU/Units/TxFIFO %a Pipe %b/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
    |                                               |TxFIFO-1| 0x20000600 | 0-12 |  1 |  13 |  0-2  |changed |
    |                                               |TxFIFO-2| 0x21000600 | 0-12 |  1 |  13 |  0-2  |        | 0x3 -- 8B  == 1G, 2.5G, 5G, 10G, 12G, 25G
    |                                               |TxFIFO-1| 0x9F000600 | 0-12 |  1 |  13 |  0-2  |        | 0x5 -- 32B == 20G, 24G, 50G
    |                                               |TxFIFO-2| 0xA0000600 | 0-12 |  1 |  13 |  0-2  |        | 0x6 -- 64B == 40G, 48G, 100G, 110G, 120G
    |                                               |TxFIFO-2| 0xA1000600 | 0-12 |  1 |  13 |  0-2  |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+----------------------------------------------------------------------------------------------------

GOPS THRESHOLD CONFIGURATIONS
    +-----------------------------------------------+--------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           |
    +    Field                                      | Unit   +------------+------+----+-----+-------+    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |
    |                                               |        |            |      |reg |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
?   | Giga Mac Port Fifo Min Threshold              |GigeMac0| 0x1000001C | 0-36 |  1 |  37 |  6-13 | GOP/GIGE_MAC_IP/Units/<GIGE_MAC_IP> Gige MAC IP %a Pipe %t/Port Internal/Port FIFO Configuration Register 1
    |                                               |GigeMac1| 0x1400001C | 0-36 |  1 |  37 |  6-13 |  The following conditions need to exist: TxFifoHiWM > TxFifoLowWM > TxFifoMinTh
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
+   | XLG Mac Port Fifo Min Threshold (TxRdThr      |XLGMac_0| 0x1000001C | 0-36 |  1 |  37 | 11-15 | GOP/XLG_MAC_IP/Units/<XLG_MAC_IP> XLG MAC IP %a Pipe %t/Port FIFOs Thresholds Configuration
    |                                               |XLGMac_0| 0x1400001C | 0-36 |  1 |  37 | 11-15 |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------

DMA speed
    +-----------------------------------------------+--------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           |
    +    Field                                      | Unit   +------------+------+----+-----+-------+    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |
    |                                               |        |            |      |reg |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TRG port speed                                |TxDMA-0 | 0x1C003010 | 0-12 |  1 | 13  | 16-19 | TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 1
    |                                               |TxDMA-1 | 0x1D003010 | 0-12 |  1 | 13  | 16-19 |
    |                                               |TxDMA-2 | 0x1E003010 | 0-12 |  1 | 13  | 16-19 |
    |                                               |TxDMA-3 | 0x9C003010 | 0-12 |  1 | 13  | 16-19 |
    |                                               |TxDMA-4 | 0x9D003010 | 0-12 |  1 | 13  | 16-19 |
    |                                               |TxDMA-5 | 0x9E003010 | 0-12 |  1 | 13  | 16-19 |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | SRC port speed                                |TxDMA-0 | 0x1C003010 | 0-127|  4 | 512 |  0- 7 | TXDMA IP TLU/Units/TxDMA %a Pipe %z/TxDMA Global Configurations/Src DMA Speed Table %n
    |                                               |TxDMA-1 | 0x1D003010 | 0-127|  4 | 512 |  0- 7 |
    |                                               |TxDMA-2 | 0x1E003010 | 0-127|  4 | 512 |  0- 7 |  for each DP
    |                                               |TxDMA-3 | 0x9C003010 | 0-127|  4 | 512 |  0- 7 |      configure SRS DMA speed TBL
    |                                               |TxDMA-4 | 0x9D003010 | 0-127|  4 | 512 |  0- 7 |  end
    |                                               |TxDMA-5 | 0x9E003010 | 0-127|  4 | 512 |  0- 7 |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------

#endif


typedef struct PRV_CPSS_DXCH_SPEED_ENM_SPEED_Mbps_STCT
{
    CPSS_PORT_SPEED_ENT speedEnm;
    GT_U32              speedMbps;
}PRV_CPSS_DXCH_SPEED_ENM_SPEED_Mbps_STC;



PRV_CPSS_DXCH_SPEED_ENM_SPEED_Mbps_STC prv_SpeedEnmSpeedMbpsArr[] =
{
     {           CPSS_PORT_SPEED_10_E   ,   1000 }
    ,{          CPSS_PORT_SPEED_100_E   ,   1000 }
    ,{         CPSS_PORT_SPEED_1000_E   ,   1000 }
    ,{         CPSS_PORT_SPEED_2500_E   ,   2500 }
    ,{         CPSS_PORT_SPEED_5000_E   ,   5000 }
    ,{        CPSS_PORT_SPEED_10000_E   ,  10000 }
    ,{        CPSS_PORT_SPEED_11800_E   ,  12000 }
    ,{        CPSS_PORT_SPEED_12500_E   ,  12500 }
    ,{        CPSS_PORT_SPEED_20000_E   ,  20000 }
    ,{        CPSS_PORT_SPEED_23600_E   ,  24000 }
    ,{        CPSS_PORT_SPEED_25000_E   ,  25000 }
    ,{        CPSS_PORT_SPEED_26700_E   ,  26700 }
    ,{        CPSS_PORT_SPEED_29090_E   ,  40000 } /* for 29.09G the speed value is set to CPSS_PORT_SPEED_40000_E */
    ,{        CPSS_PORT_SPEED_40000_E   ,  40000 }
    ,{        CPSS_PORT_SPEED_47200_E   ,  48000 }
    ,{        CPSS_PORT_SPEED_50000_E   ,  50000 }
    ,{        CPSS_PORT_SPEED_52500_E   ,  52500 } /* for aldrin2 */
    ,{         CPSS_PORT_SPEED_100G_E   , 100000 }
    ,{         CPSS_PORT_SPEED_102G_E   , 102000 }
    ,{         CPSS_PORT_SPEED_107G_E   , 107000 }
};



#define PRV_CPSS_DXCH_CAELUM_DP0_TXQ_CREDITS_CNS                144
#define PRV_CPSS_DXCH_CAELUM_DP1_TXQ_CREDITS_CNS                144

#define PRV_CPSS_DXCH_CAELUM_DP0_TXFIFO_HEADERS_CREDITS_CNS     276
#define PRV_CPSS_DXCH_CAELUM_DP1_TXFIFO_HEADERS_CREDITS_CNS     363

#define PRV_CPSS_DXCH_CAELUM_DP0_TXFIFO_PAYLOAD_CREDITS_CNS     387
#define PRV_CPSS_DXCH_CAELUM_DP1_TXFIFO_PAYLOAD_CREDITS_CNS     447

#define PRV_CPSS_DXCH_CETUS_DP1_TXQ_CREDITS_CNS                 144
#define PRV_CPSS_DXCH_CETUS_DP1_TXFIFO_HEADERS_CREDITS_CNS      363
#define PRV_CPSS_DXCH_CETUS_DP1_TXFIFO_PAYLOAD_CREDITS_CNS      447

#define PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS 48

#define PRV_CPSS_DXCH_ALDRIN_DP_TXQ_CREDITS_CNS                 144
#define PRV_CPSS_DXCH_ALDRIN_DP_TXFIFO_HEADERS_CREDITS_CNS      363
#define PRV_CPSS_DXCH_ALDRIN_DP_TXFIFO_PAYLOAD_CREDITS_CNS      447

#define PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS                373
#define PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS     651
#define PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS     768



PRV_CPSS_DXCH_SPEED_PLACE_STC prv_Cetus_speedPlaceArr[] =
{
     {    CPSS_PORT_SPEED_1000_E,    1000,    1 }
    ,{    CPSS_PORT_SPEED_2500_E,    2500,    2 }
    ,{    CPSS_PORT_SPEED_5000_E,    5000,    3 }
    ,{   CPSS_PORT_SPEED_10000_E,   10000,    4 }
    ,{   CPSS_PORT_SPEED_11800_E,   12000,    5 }
    ,{   CPSS_PORT_SPEED_20000_E,   20000,    7 }
    ,{   CPSS_PORT_SPEED_40000_E,   40000,   10 }
    ,{   CPSS_PORT_SPEED_47200_E,   48000,   11 }
    ,{      CPSS_PORT_SPEED_NA_E,       0,    0 }
};


PRV_CPSS_DXCH_CORECLOCK_PLACE_STC prv_Cetus_coreClockPlaceArr[] =
{    /* real cc, computation, idx */
      {     167,        167,   1 }
     ,{     200,        200,   2 }
     ,{     250,        250,   3 }
     ,{     365,        365,   4 }
     ,{       0,          0,   0 }
};

/*-----------------------------------------------------------------------------------*
 * TXDMA_SCDMA_TxQDescriptorCredit : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
GT_U32 prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditCore0Arr[5][3] =
{
     {      0,   1000,   2500 }
    ,{    167,      3,      4 }
    ,{    200,      3,      4 }
    ,{    250,      3,      4 }
    ,{    365,      2,      3 }
};

GT_U32 prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditCore1Arr[5][14] =
{
     {   0, 1000, 2500, 5000, 10000, 12000, 13000, 20000, 24000, 25000, 40000, 48000, 49000, 75000 }
    ,{ 167,    3,    5,    7,    13,    12,    12,    23,    21,    22,    42,    40,    40,    60 }
    ,{ 200,    3,    5,    7,    13,    12,    12,    21,    20,    20,    38,    35,    36,    52 }
    ,{ 250,    3,    5,    6,    11,    10,    10,    18,    16,    16,    30,    28,    29,    43 }
    ,{ 365,    3,    4,    6,     9,     8,     8,    14,    13,    13,    23,    21,    21,    30 }
};

/*-----------------------------------------------------------------------------------*
 * TXDMA_SCDMA_TxFIFOHeaderCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
GT_U32 prvCpssDxChBobkTxdmaScdmaTxFifoHeaderCreditThresholdCore0Arr[5][3] =
{
     {      0, 1000, 2500 }
    ,{    167,    5,   10 }
    ,{    200,    4,    9 }
    ,{    250,    3,    7 }
    ,{    365,    2,    5 }
};

GT_U32 prvCpssDxChBobkTxdmaScdmaTxFifoHeaderCreditThresholdCore1Arr[5][14] =
{
     {   0, 1000, 2500, 5000, 10000, 12000, 13000, 20000, 24000, 25000, 40000, 48000, 49000, 75000 }
    ,{ 167,    4,    9,   17,    42,    41,    41,    83,    81,    82,   166,   161,   163,   251 }
    ,{ 200,    4,    8,   15,    36,    35,    35,    70,    69,    70,   140,   136,   137,   211 }
    ,{ 250,    3,    6,   13,    30,    28,    29,    58,    56,    56,   114,   110,   112,   172 }
    ,{ 365,    3,    5,    9,    22,    21,    21,    41,    40,    40,    81,    78,    79,   122 }
};

/*-----------------------------------------------------------------------------------*
 * TXDMA_SCDMA_TxFIFOPayloadCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
GT_U32 prvCpssDxChBobkTxdmaScdmaTxFifoPayloadCreditThresholdCore0Arr[5][3] =
{
     {   0, 1000, 2500 }
    ,{ 167,    7,   13 }
    ,{ 200,    6,   11 }
    ,{ 250,    5,    9 }
    ,{ 365,    4,    7 }
};


GT_U32 prvCpssDxChBobkTxdmaScdmaTxFifoPayloadCreditThresholdCore1Arr[5][14] =
{
     {   0, 1000, 2500,  5000, 10000, 12000, 13000, 20000, 24000, 25000, 40000, 48000, 49000, 75000 }
    ,{ 167,    6,   12,    21,    48,    47,     0,    93,     0,     0,   184,   182,     0,     0 }
    ,{ 200,    6,   10,    18,    41,    41,     0,    78,     0,     0,   155,   154,     0,     0 }
    ,{ 250,    5,    8,    16,    34,    33,     0,    65,     0,     0,   126,   125,     0,     0 }
    ,{ 365,    5,    7,    11,    25,    25,     0,    46,     0,     0,    90,    88,     0,     0 }
};

/*-----------------------------------------------------------------------------------*
 * TXFIFO_SCDMA_PayloadStartTransmThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
GT_U32 prvCpssDxChBobkTxFifoScdmaPayloadThresholdCore0Arr[5][3] =
{
     {   0, 1000, 2500 }
    ,{ 167,    2,    3 }
    ,{ 200,    2,    2 }
    ,{ 250,    2,    2 }
    ,{ 365,    2,    2 }
};


GT_U32 prvCpssDxChBobkTxFifoScdmaPayloadThresholdCore1Arr[5][14] =
{
     {   0, 1000, 2500, 5000, 10000, 12000, 13000, 20000, 24000, 25000, 40000, 48000, 49000, 75000 }
    ,{ 167,    2,    3,    4,     6,     6,     0,    10,     0,     0,    18,    21,     0,     0 }
    ,{ 200,    2,    2,    3,     5,     6,     0,     8,     0,     0,    15,    18,     0,     0 }
    ,{ 250,    2,    2,    3,     4,     5,     0,     7,     0,     0,    12,    15,     0,     0 }
    ,{ 365,    2,    2,    2,     3,     4,     0,     5,     0,     0,     9,    10,     0,     0 }
};

GT_U32 otherLatencyDiffArrayCore1[5][2] =
{
     {   0,    0}
    ,{ 167,   41}
    ,{ 200,   38}
    ,{ 250,   33}
    ,{ 365,   21}
};

GT_U32 ipgBytesArr[2][14] =
{
     { 0, 1000, 2500, 5000, 10000, 12000, 13000, 20000, 24000, 25000, 40000, 48000, 49000, 75000 }
    ,{ 0,   20 ,  20,   20,     4,    20,    20,     4,    20,    20,     4,    20,    20,    20 }
};

GT_U32 maxBwArr[5][3] =
{
     {   0,    1,    2 }
    ,{ 167,   48,   67 }
    ,{ 200,   48,  101 }
    ,{ 250,   48,  121 }
    ,{ 365,   48,  185 }
};

GT_U32 cetusTmBw[5][2] =
{
     {   0,       1}
    ,{ 167,   25000}
    ,{ 200,   25000}
    ,{ 250,   25000}
    ,{ 365,   25000}
};

GT_U32 caelumTmBw[5][2] =
{
     {   0,       1}
    ,{ 167,   52000}
    ,{ 200,   63000}
    ,{ 250,   75000}
    ,{ 365,   75000}
};

GT_U32 prvCpssDxChBobkWorstCaseAck2AckDiffCore0Arr[5][3] =
{
     {      0, 1000, 2500 }
    ,{    167,   48,   20 }
    ,{    200,   48,   20 }
    ,{    250,   48,   20 }
    ,{    365,   48,   20 }
};

GT_U32 prvCpssDxChBobkWorstCaseAck2AckDiffCore1Arr[5][14] =
{
     {   0, 1000, 2500, 5000, 10000, 12000, 13000, 20000, 24000, 25000, 40000, 48000, 49000, 75000 }
    ,{ 167,   67,   36,   21,    10,     8,     7,     5,     4,     4,     3,     2,     2,     1 }
    ,{ 200,  101,   56,   31,    16,    13,    12,     9,     8,     7,     5,     4,     4,     2 }
    ,{ 250,  121,   66,   37,    19,    15,    14,    10,     8,     7,     5,     4,     4,     3 }
    ,{ 365,  185,   99,   58,    28,    23,    21,    15,    13,    12,     8,     6,     6,     4 }
};



/**************************************************************************************
*
*    TM port resource part
*
**************************************************************************************/

/*-----------------------------------------------------------------------------------*
 * TXDMA_SCDMA_TxQDescriptorCredit : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
GT_U32 prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditTmArr[5][186] =
{
     {      0,   1000,   2000,   3000,   4000,   5000,   6000,   7000,   8000,   9000,  10000,  11000,  12000,  13000,  14000,  15000,  16000,  17000,  18000,  19000,  20000,  21000,  22000,  23000,  24000,  25000,  26000,  27000,  28000,  29000,  30000,  31000,  32000,  33000,  34000,  35000,  36000,  37000,  38000,  39000,  40000,  41000,  42000,  43000,  44000,  45000,  46000,  47000,  48000,  49000,  50000,  51000,  52000,  53000,  54000,  55000,  56000,  57000,  58000,  59000,  60000,  61000,  62000,  63000,  64000,  65000,  66000,  67000,  68000,  69000,  70000,  71000,  72000,  73000,  74000,  75000,  76000,  77000,  78000,  79000,  80000,  81000,  82000,  83000,  84000,  85000,  86000,  87000,  88000,  89000,  90000,  91000,  92000,  93000,  94000,  95000,  96000,  97000,  98000,  99000, 100000, 101000, 102000, 103000, 104000, 105000, 106000, 107000, 108000, 109000, 110000, 111000, 112000, 113000, 114000, 115000, 116000, 117000, 118000, 119000, 120000, 121000, 122000, 123000, 124000, 125000, 126000, 127000, 128000, 129000, 130000, 131000, 132000, 133000, 134000, 135000, 136000, 137000, 138000, 139000, 140000, 141000, 142000, 143000, 144000, 145000, 146000, 147000, 148000, 149000, 150000, 151000, 152000, 153000, 154000, 155000, 156000, 157000, 158000, 159000, 160000, 161000, 162000, 163000, 164000, 165000, 166000, 167000, 168000, 169000, 170000, 171000, 172000, 173000, 174000, 175000, 176000, 177000, 178000, 179000, 180000, 181000, 182000, 183000, 184000, 185000 }
    ,{    167,      3,      4,      6,      6,      7,      8,      9,     10,     10,     13,     12,     12,     13,     14,     15,     16,     17,     17,     18,     23,     20,     20,     21,     21,     22,     23,     24,     25,     26,     27,     28,     28,     28,     29,     30,     31,     32,     33,     34,     42,     35,     36,     37,     37,     37,     38,     39,     40,     41,     41,     42,     43,     44,     45,     46,     46,     47,     48,     49,     50,     51,     51,     52,     53,     54,     55,     54,     53,     53,     54,     55,     56,     56,     57,     58,     59,     60,     60,     61,     62,     63,     63,     64,     65,     66,     66,     67,     68,     69,     69,     70,     71,     72,     73,     73,     74,     75,     76,     76,     77,     78,     79,     79,     80,     81,     82,     82,     83,     84,     85,     86,     86,     87,     88,     89,     89,     90,     91,     92,     92,     93,     94,     95,     96,     96,     97,     98,     99,     99,    100,    101,    102,    102,    103,    104,    105,    105,    106,    107,    108,    109,    109,    110,    111,    112,    112,    113,    114,    115,    115,    116,    117,    118,    119,    119,    120,    121,    122,    122,    123,    124,    125,    125,    126,    127,    128,    128,    129,    130,    131,    132,    132,    133,    134,    135,    135,    136,    137,    138,    138,    139,    140,    141,    142,    142 }
    ,{    200,      3,      5,      6,      6,      7,      8,      9,      9,     10,     13,     11,     12,     12,     13,     14,     15,     15,     16,     16,     21,     18,     19,     19,     20,     20,     21,     21,     22,     23,     24,     24,     25,     25,     26,     27,     28,     28,     29,     29,     38,     30,     31,     32,     33,     33,     34,     35,     35,     36,     36,     37,     37,     38,     39,     39,     40,     41,     41,     42,     43,     44,     44,     45,     46,     46,     47,     46,     47,     48,     48,     49,     50,     50,     51,     52,     53,     53,     54,     55,     55,     56,     57,     57,     58,     59,     59,     60,     61,     61,     62,     63,     63,     64,     65,     66,     66,     67,     68,     68,     69,     67,     66,     66,     67,     68,     68,     69,     70,     70,     71,     72,     72,     73,     73,     74,     75,     75,     76,     77,     77,     78,     79,     79,     80,     80,     81,     82,     82,     83,     84,     84,     85,     86,     86,     87,     88,     88,     89,     89,     90,     91,     91,     92,     93,     93,     94,     95,     95,     96,     96,     97,     98,     98,     99,    100,    100,    101,    102,    102,    103,    104,    104,    105,    105,    106,    107,    107,    108,    109,    109,    110,    111,    111,    112,    112,    113,    114,    114,    115,    116,    116,    117,    118,    118,    119 }
    ,{    250,      3,      5,      6,      6,      6,      7,      7,      8,      9,     11,     10,     10,     10,     11,     12,     12,     13,     13,     14,     18,     15,     15,     16,     16,     16,     18,     18,     18,     19,     20,     20,     21,     22,     22,     22,     23,     23,     24,     24,     30,     25,     26,     26,     27,     28,     28,     29,     28,     29,     30,     30,     31,     31,     32,     33,     33,     34,     34,     34,     34,     35,     36,     36,     37,     37,     38,     38,     39,     40,     40,     41,     41,     42,     42,     43,     43,     44,     45,     45,     44,     45,     45,     46,     46,     47,     48,     48,     49,     49,     50,     50,     51,     51,     52,     53,     53,     54,     54,     55,     55,     56,     56,     57,     57,     58,     59,     59,     60,     60,     61,     61,     62,     62,     63,     63,     64,     65,     65,     66,     66,     65,     63,     63,     64,     64,     65,     66,     66,     67,     67,     68,     68,     69,     69,     70,     70,     71,     71,     72,     72,     73,     73,     74,     74,     75,     75,     76,     76,     77,     77,     78,     78,     79,     79,     80,     80,     81,     81,     82,     82,     83,     83,     84,     84,     85,     85,     86,     86,     87,     88,     88,     89,     89,     90,     90,     91,     91,     92,     92,     93,     93,     94,     94,     95,     95 }
    ,{    365,      3,      4,      5,      5,      6,      6,      6,      7,      7,      9,      8,      8,      8,      9,      9,     10,     10,     11,     11,     14,     12,     12,     13,     13,     13,     14,     14,     14,     15,     15,     15,     16,     16,     17,     17,     17,     18,     18,     18,     23,     19,     19,     20,     20,     20,     21,     21,     21,     21,     22,     22,     23,     23,     23,     24,     24,     25,     25,     26,     25,     26,     26,     26,     27,     27,     28,     28,     29,     29,     29,     30,     30,     31,     30,     30,     31,     31,     32,     32,     32,     33,     33,     34,     34,     34,     35,     35,     36,     36,     35,     36,     36,     37,     37,     37,     38,     38,     38,     39,     39,     40,     40,     40,     41,     41,     42,     42,     42,     43,     43,     43,     44,     44,     45,     45,     45,     46,     46,     47,     47,     47,     48,     47,     47,     47,     48,     48,     49,     49,     49,     50,     50,     50,     51,     51,     52,     52,     52,     53,     53,     53,     54,     54,     55,     55,     55,     56,     56,     56,     57,     57,     58,     58,     58,     59,     59,     59,     60,     60,     61,     61,     61,     62,     62,     62,     63,     63,     64,     64,     64,     65,     65,     65,     66,     66,     67,     67,     67,     68,     68,     68,     69,     69,     70,     68 }
};


/*-----------------------------------------------------------------------------------*
 * TXDMA_SCDMA_TxFIFOHeaderCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
GT_U32 prvCpssDxChBobkTxdmaScdmaTxFifoHeaderCreditThresholdTmArr[5][186] =
{
     {      0,   1000,   2000,   3000,   4000,   5000,   6000,   7000,   8000,   9000,  10000,  11000,  12000,  13000,  14000,  15000,  16000,  17000,  18000,  19000,  20000,  21000,  22000,  23000,  24000,  25000,  26000,  27000,  28000,  29000,  30000,  31000,  32000,  33000,  34000,  35000,  36000,  37000,  38000,  39000,  40000,  41000,  42000,  43000,  44000,  45000,  46000,  47000,  48000,  49000,  50000,  51000,  52000,  53000,  54000,  55000,  56000,  57000,  58000,  59000,  60000,  61000,  62000,  63000,  64000,  65000,  66000,  67000,  68000,  69000,  70000,  71000,  72000,  73000,  74000,  75000,  76000,  77000,  78000,  79000,  80000,  81000,  82000,  83000,  84000,  85000,  86000,  87000,  88000,  89000,  90000,  91000,  92000,  93000,  94000,  95000,  96000,  97000,  98000,  99000, 100000, 101000, 102000, 103000, 104000, 105000, 106000, 107000, 108000, 109000, 110000, 111000, 112000, 113000, 114000, 115000, 116000, 117000, 118000, 119000, 120000, 121000, 122000, 123000, 124000, 125000, 126000, 127000, 128000, 129000, 130000, 131000, 132000, 133000, 134000, 135000, 136000, 137000, 138000, 139000, 140000, 141000, 142000, 143000, 144000, 145000, 146000, 147000, 148000, 149000, 150000, 151000, 152000, 153000, 154000, 155000, 156000, 157000, 158000, 159000, 160000, 161000, 162000, 163000, 164000, 165000, 166000, 167000, 168000, 169000, 170000, 171000, 172000, 173000, 174000, 175000, 176000, 177000, 178000, 179000, 180000, 181000, 182000, 183000, 184000, 185000 }
    ,{    167,      4,      8,     11,     14,     17,     21,     24,     27,     31,     42,     38,     41,     44,     47,     51,     54,     58,     61,     65,     83,     71,     74,     77,     81,     84,     88,     91,     94,     98,    101,    104,    107,    111,    114,    117,    121,    124,    128,    131,    166,    138,    141,    144,    148,    151,    155,    158,    161,    165,    168,    171,    175,    178,    181,    185,    188,    192,    195,    198,    202,    205,    208,    212,    215,    218,    222,    224,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 }
    ,{    200,      4,      7,      9,     12,     15,     17,     21,     23,     26,     36,     32,     35,     38,     40,     43,     46,     49,     51,     54,     70,     60,     63,     66,     69,     71,     74,     76,     79,     82,     85,     88,     90,     94,     97,     99,    102,    105,    108,    111,    140,    116,    119,    122,    125,    128,    131,    133,    136,    139,    141,    144,    147,    150,    152,    155,    158,    161,    164,    166,    169,    172,    175,    178,    180,    183,    186,    189,    192,    195,    197,    200,    203,    206,    209,    211,    214,    217,    220,    223,    225,    228,    231,    234,    237,    240,    242,    245,    248,    251,    254,    256,    259,    262,    265,    268,    270,    273,    276,    279,    282,    285,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 }
    ,{    250,      3,      5,      8,     10,     13,     14,     17,     19,     21,     30,     26,     28,     31,     33,     35,     38,     39,     42,     44,     58,     49,     51,     53,     55,     58,     60,     62,     65,     67,     69,     72,     74,     76,     78,     80,     83,     85,     87,     90,    114,     94,     97,     99,    101,    104,    106,    108,    110,    112,    115,    117,    119,    122,    124,    126,    128,    131,    133,    135,    138,    140,    142,    144,    147,    149,    151,    154,    156,    158,    160,    163,    165,    167,    170,    172,    174,    176,    179,    181,    183,    186,    188,    190,    192,    195,    197,    199,    202,    204,    206,    208,    211,    213,    215,    218,    220,    222,    224,    227,    229,    231,    234,    236,    238,    240,    243,    245,    247,    250,    252,    254,    256,    259,    261,    263,    266,    268,    270,    272,    275,    276,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 }
    ,{    365,      3,      5,      5,      7,      9,     11,     12,     14,     16,     22,     19,     21,     22,     23,     25,     27,     28,     30,     31,     41,     35,     37,     39,     40,     41,     43,     44,     46,     47,     49,     51,     53,     54,     56,     57,     59,     60,     62,     64,     81,     67,     69,     71,     72,     74,     75,     77,     78,     80,     82,     83,     84,     86,     88,     89,     91,     93,     94,     96,     97,     99,    100,    102,    104,    105,    107,    108,    110,    112,    113,    115,    117,    118,    120,    122,    123,    125,    126,    128,    130,    131,    133,    135,    136,    138,    139,    141,    143,    144,    146,    147,    149,    151,    152,    154,    155,    157,    159,    160,    162,    164,    165,    167,    168,    170,    172,    173,    175,    176,    178,    180,    181,    183,    185,    186,    188,    189,    191,    193,    194,    196,    197,    199,    201,    202,    204,    206,    207,    209,    210,    212,    214,    215,    217,    218,    220,    222,    223,    225,    227,    228,    230,    231,    233,    235,    236,    238,    239,    241,    243,    244,    246,    248,    249,    251,    252,    254,    256,    257,    259,    260,    262,    264,    265,    267,    269,    270,    272,    273,    275,    277,    278,    280,    281,    283,    285,    286,    288,    290,    291,    293,    294,    296,    298,    299 }
};


/*-----------------------------------------------------------------------------------*
 * TXDMA_SCDMA_TxFIFOPayloadCreditThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
GT_U32 prvCpssDxChBobkTxdmaScdmaTxFifoPayloadCreditThresholdTmArr[5][186] =
{
     {      0,   1000,   2000,   3000,   4000,   5000,   6000,   7000,   8000,   9000,  10000,  11000,  12000,  13000,  14000,  15000,  16000,  17000,  18000,  19000,  20000,  21000,  22000,  23000,  24000,  25000,  26000,  27000,  28000,  29000,  30000,  31000,  32000,  33000,  34000,  35000,  36000,  37000,  38000,  39000,  40000,  41000,  42000,  43000,  44000,  45000,  46000,  47000,  48000,  49000,  50000,  51000,  52000,  53000,  54000,  55000,  56000,  57000,  58000,  59000,  60000,  61000,  62000,  63000,  64000,  65000,  66000,  67000,  68000,  69000,  70000,  71000,  72000,  73000,  74000,  75000,  76000,  77000,  78000,  79000,  80000,  81000,  82000,  83000,  84000,  85000,  86000,  87000,  88000,  89000,  90000,  91000,  92000,  93000,  94000,  95000,  96000,  97000,  98000,  99000, 100000, 101000, 102000, 103000, 104000, 105000, 106000, 107000, 108000, 109000, 110000, 111000, 112000, 113000, 114000, 115000, 116000, 117000, 118000, 119000, 120000, 121000, 122000, 123000, 124000, 125000, 126000, 127000, 128000, 129000, 130000, 131000, 132000, 133000, 134000, 135000, 136000, 137000, 138000, 139000, 140000, 141000, 142000, 143000, 144000, 145000, 146000, 147000, 148000, 149000, 150000, 151000, 152000, 153000, 154000, 155000, 156000, 157000, 158000, 159000, 160000, 161000, 162000, 163000, 164000, 165000, 166000, 167000, 168000, 169000, 170000, 171000, 172000, 173000, 174000, 175000, 176000, 177000, 178000, 179000, 180000, 181000, 182000, 183000, 184000, 185000 }
    ,{    167,      6,     10,     14,     17,     21,     25,     28,     32,     36,     48,     44,     47,     51,     54,     59,     62,     66,     70,     74,     93,     81,     85,     88,     92,     96,    100,    104,    107,    111,    115,    118,    122,    126,    129,    133,    137,    141,    145,    148,    184,    156,    160,    163,    168,    171,    175,    179,    182,    187,    190,    193,    198,    201,    205,    209,    212,    217,    220,    224,    228,    232,    235,    239,    243,    246,    251,    253,     29,     30,     30,     31,     31,     31,     32,     32,     33,     33,     33,     34,     34,     35,     35,     36,     36,     36,     37,     37,     38,     38,     38,     39,     39,     40,     40,     40,     41,     41,     42,     42,     43,     43,     43,     44,     44,     45,     45,     45,     46,     46,     47,     47,     47,     48,     48,     49,     49,     49,     50,     50,     51,     51,     52,     52,     52,     53,     53,     54,     54,     54,     55,     55,     56,     56,     56,     57,     57,     58,     58,     59,     59,     59,     60,     60,     61,     61,     61,     62,     62,     63,     63,     63,     64,     64,     65,     65,     65,     66,     66,     67,     67,     68,     68,     68,     69,     69,     70,     70,     70,     71,     71,     72,     72,     72,     73,     73,     74,     74,     75,     75,     75,     76,     76,     77,     77,     77 }
    ,{    200,      6,      9,     12,     15,     18,     21,     25,     27,     31,     41,     37,     41,     44,     46,     50,     53,     56,     59,     62,     78,     69,     72,     75,     79,     81,     84,     87,     90,     93,     97,    100,    102,    107,    110,    112,    116,    119,    122,    126,    155,    132,    135,    138,    142,    145,    148,    151,    154,    157,    160,    163,    166,    170,    172,    175,    179,    182,    185,    188,    191,    194,    198,    201,    203,    207,    210,    213,    217,    220,    222,    226,    229,    232,    236,    238,    241,    245,    248,    252,    254,    257,    261,    264,    267,    271,    273,    276,    280,    283,    286,    289,    292,    295,    299,    302,    304,    308,    311,    314,    318,    321,     36,     37,     37,     37,     38,     38,     38,     39,     39,     39,     40,     40,     40,     41,     41,     41,     42,     42,     43,     43,     43,     44,     44,     44,     45,     45,     45,     46,     46,     46,     47,     47,     47,     48,     48,     48,     49,     49,     49,     50,     50,     50,     51,     51,     51,     52,     52,     52,     53,     53,     53,     54,     54,     54,     55,     55,     56,     56,     56,     57,     57,     57,     58,     58,     58,     59,     59,     59,     60,     60,     60,     61,     61,     61,     62,     62,     62,     63,     63,     63,     64,     64,     64,     65 }
    ,{    250,      5,      7,     10,     13,     16,     17,     20,     23,     25,     34,     31,     33,     36,     38,     41,     44,     45,     48,     51,     65,     56,     59,     61,     63,     66,     69,     71,     74,     76,     79,     82,     84,     87,     89,     91,     94,     97,     99,    102,    126,    107,    110,    112,    115,    118,    120,    122,    125,    127,    130,    132,    135,    138,    140,    143,    145,    148,    150,    153,    156,    158,    160,    163,    166,    168,    171,    174,    176,    178,    181,    184,    186,    188,    192,    194,    196,    199,    202,    204,    206,    210,    212,    214,    216,    220,    222,    224,    228,    230,    232,    234,    238,    240,    242,    245,    248,    250,    252,    256,    258,    260,    263,    266,    268,    270,    273,    276,    278,    281,    284,    286,    288,    291,    294,    296,    299,    301,    304,    306,    309,    311,     35,     35,     35,     36,     36,     36,     36,     37,     37,     37,     38,     38,     38,     38,     39,     39,     39,     40,     40,     40,     40,     41,     41,     41,     41,     42,     42,     42,     43,     43,     43,     43,     44,     44,     44,     44,     45,     45,     45,     46,     46,     46,     46,     47,     47,     47,     47,     48,     48,     48,     49,     49,     49,     49,     50,     50,     50,     50,     51,     51,     51,     52,     52,     52 }
    ,{    365,      5,      7,      7,      9,     11,     14,     15,     17,     19,     25,     23,     25,     26,     27,     29,     31,     33,     35,     36,     46,     40,     43,     45,     46,     47,     49,     51,     53,     54,     56,     58,     60,     62,     64,     65,     67,     68,     71,     73,     90,     76,     78,     81,     82,     84,     85,     87,     88,     91,     93,     94,     95,     97,    100,    101,    103,    105,    106,    109,    110,    112,    113,    115,    117,    119,    121,    122,    124,    126,    128,    130,    132,    133,    135,    138,    139,    141,    142,    144,    146,    148,    150,    152,    153,    155,    157,    159,    161,    162,    164,    166,    168,    170,    171,    173,    174,    177,    179,    180,    182,    184,    186,    188,    189,    191,    193,    195,    197,    198,    200,    202,    203,    206,    208,    209,    211,    212,    215,    217,    218,    220,    221,    224,    226,    227,    229,    231,    232,    235,    236,    238,    240,    241,    244,    245,    247,    249,    250,    253,    255,    256,    258,    259,    261,    264,    265,    267,    268,    270,    273,    274,    276,    278,    279,    282,    283,    285,    287,    288,    290,    292,    294,    296,    297,    299,    302,    303,    305,    306,    308,    311,    312,    314,    315,    317,    319,    321,    323,    325,    326,    328,    330,    332,    334,    335 }
};


/*-----------------------------------------------------------------------------------*
 * TXFIFO_SCDMA_PayloadStartTransmThreshold : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
GT_U32 prvCpssDxChBobkTxFifoScdmaPayloadThresholdTmArr[5][186] =
{
     {      0,   1000,   2000,   3000,   4000,   5000,   6000,   7000,   8000,   9000,  10000,  11000,  12000,  13000,  14000,  15000,  16000,  17000,  18000,  19000,  20000,  21000,  22000,  23000,  24000,  25000,  26000,  27000,  28000,  29000,  30000,  31000,  32000,  33000,  34000,  35000,  36000,  37000,  38000,  39000,  40000,  41000,  42000,  43000,  44000,  45000,  46000,  47000,  48000,  49000,  50000,  51000,  52000,  53000,  54000,  55000,  56000,  57000,  58000,  59000,  60000,  61000,  62000,  63000,  64000,  65000,  66000,  67000,  68000,  69000,  70000,  71000,  72000,  73000,  74000,  75000,  76000,  77000,  78000,  79000,  80000,  81000,  82000,  83000,  84000,  85000,  86000,  87000,  88000,  89000,  90000,  91000,  92000,  93000,  94000,  95000,  96000,  97000,  98000,  99000, 100000, 101000, 102000, 103000, 104000, 105000, 106000, 107000, 108000, 109000, 110000, 111000, 112000, 113000, 114000, 115000, 116000, 117000, 118000, 119000, 120000, 121000, 122000, 123000, 124000, 125000, 126000, 127000, 128000, 129000, 130000, 131000, 132000, 133000, 134000, 135000, 136000, 137000, 138000, 139000, 140000, 141000, 142000, 143000, 144000, 145000, 146000, 147000, 148000, 149000, 150000, 151000, 152000, 153000, 154000, 155000, 156000, 157000, 158000, 159000, 160000, 161000, 162000, 163000, 164000, 165000, 166000, 167000, 168000, 169000, 170000, 171000, 172000, 173000, 174000, 175000, 176000, 177000, 178000, 179000, 180000, 181000, 182000, 183000, 184000, 185000 }
    ,{    167,      2,      2,      3,      3,      4,      4,      4,      5,      5,      6,      6,      6,      7,      7,      8,      8,      8,      9,      9,     10,     10,     11,     11,     11,     12,     12,     13,     13,     13,     14,     14,     15,     15,     15,     16,     16,     17,     17,     17,     18,     18,     19,     19,     20,     20,     20,     21,     21,     22,     22,     22,     23,     23,     24,     24,     24,     25,     25,     26,     26,     27,     27,     27,     28,     28,     29,     29,     29,     30,     30,     31,     31,     31,     32,     32,     33,     33,     33,     34,     34,     35,     35,     36,     36,     36,     37,     37,     38,     38,     38,     39,     39,     40,     40,     40,     41,     41,     42,     42,     43,     43,     43,     44,     44,     45,     45,     45,     46,     46,     47,     47,     47,     48,     48,     49,     49,     49,     50,     50,     51,     51,     52,     52,     52,     53,     53,     54,     54,     54,     55,     55,     56,     56,     56,     57,     57,     58,     58,     59,     59,     59,     60,     60,     61,     61,     61,     62,     62,     63,     63,     63,     64,     64,     65,     65,     65,     66,     66,     67,     67,     68,     68,     68,     69,     69,     70,     70,     70,     71,     71,     72,     72,     72,     73,     73,     74,     74,     75,     75,     75,     76,     76,     77,     77,     77 }
    ,{    200,      2,      2,      3,      3,      3,      4,      4,      4,      5,      5,      5,      6,      6,      6,      7,      7,      7,      8,      8,      8,      9,      9,      9,     10,     10,     10,     11,     11,     11,     12,     12,     12,     13,     13,     13,     14,     14,     14,     15,     15,     16,     16,     16,     17,     17,     17,     18,     18,     18,     19,     19,     19,     20,     20,     20,     21,     21,     21,     22,     22,     22,     23,     23,     23,     24,     24,     24,     25,     25,     25,     26,     26,     26,     27,     27,     27,     28,     28,     29,     29,     29,     30,     30,     30,     31,     31,     31,     32,     32,     32,     33,     33,     33,     34,     34,     34,     35,     35,     35,     36,     36,     36,     37,     37,     37,     38,     38,     38,     39,     39,     39,     40,     40,     40,     41,     41,     41,     42,     42,     43,     43,     43,     44,     44,     44,     45,     45,     45,     46,     46,     46,     47,     47,     47,     48,     48,     48,     49,     49,     49,     50,     50,     50,     51,     51,     51,     52,     52,     52,     53,     53,     53,     54,     54,     54,     55,     55,     56,     56,     56,     57,     57,     57,     58,     58,     58,     59,     59,     59,     60,     60,     60,     61,     61,     61,     62,     62,     62,     63,     63,     63,     64,     64,     64,     65 }
    ,{    250,      2,      2,      2,      3,      3,      3,      3,      4,      4,      4,      5,      5,      5,      5,      6,      6,      6,      6,      7,      7,      7,      8,      8,      8,      8,      9,      9,      9,      9,     10,     10,     10,     11,     11,     11,     11,     12,     12,     12,     12,     13,     13,     13,     14,     14,     14,     14,     15,     15,     15,     15,     16,     16,     16,     17,     17,     17,     17,     18,     18,     18,     18,     19,     19,     19,     20,     20,     20,     20,     21,     21,     21,     21,     22,     22,     22,     23,     23,     23,     23,     24,     24,     24,     24,     25,     25,     25,     26,     26,     26,     26,     27,     27,     27,     27,     28,     28,     28,     29,     29,     29,     29,     30,     30,     30,     30,     31,     31,     31,     32,     32,     32,     32,     33,     33,     33,     33,     34,     34,     34,     35,     35,     35,     35,     36,     36,     36,     36,     37,     37,     37,     38,     38,     38,     38,     39,     39,     39,     40,     40,     40,     40,     41,     41,     41,     41,     42,     42,     42,     43,     43,     43,     43,     44,     44,     44,     44,     45,     45,     45,     46,     46,     46,     46,     47,     47,     47,     47,     48,     48,     48,     49,     49,     49,     49,     50,     50,     50,     50,     51,     51,     51,     52,     52,     52 }
    ,{    365,      2,      2,      2,      2,      2,      3,      3,      3,      3,      3,      4,      4,      4,      4,      4,      4,      5,      5,      5,      5,      5,      6,      6,      6,      6,      6,      7,      7,      7,      7,      7,      7,      8,      8,      8,      8,      8,      9,      9,      9,      9,      9,     10,     10,     10,     10,     10,     10,     11,     11,     11,     11,     11,     12,     12,     12,     12,     12,     13,     13,     13,     13,     13,     13,     14,     14,     14,     14,     14,     15,     15,     15,     15,     15,     16,     16,     16,     16,     16,     16,     17,     17,     17,     17,     17,     18,     18,     18,     18,     18,     19,     19,     19,     19,     19,     19,     20,     20,     20,     20,     20,     21,     21,     21,     21,     21,     22,     22,     22,     22,     22,     22,     23,     23,     23,     23,     23,     24,     24,     24,     24,     24,     25,     25,     25,     25,     25,     25,     26,     26,     26,     26,     26,     27,     27,     27,     27,     27,     28,     28,     28,     28,     28,     28,     29,     29,     29,     29,     29,     30,     30,     30,     30,     30,     31,     31,     31,     31,     31,     31,     32,     32,     32,     32,     32,     33,     33,     33,     33,     33,     34,     34,     34,     34,     34,     34,     35,     35,     35,     35,     35,     36,     36,     36,     36 }
};

GT_U32 prvCpssDxChBobkWorstCaseAck2AckDiffCore1TmArr[5][186] =
{
    {     0,   1000,   2000,   3000,   4000,   5000,   6000,   7000,   8000,   9000,  10000,  11000,  12000,  13000,  14000,  15000,  16000,  17000,  18000,  19000,  20000,  21000,  22000,  23000,  24000,  25000,  26000,  27000,  28000,  29000,  30000,  31000,  32000,  33000,  34000,  35000,  36000,  37000,  38000,  39000,  40000,  41000,  42000,  43000,  44000,  45000,  46000,  47000,  48000,  49000,  50000,  51000,  52000,  53000,  54000,  55000,  56000,  57000,  58000,  59000,  60000,  61000,  62000,  63000,  64000,  65000,  66000,  67000,  68000,  69000,  70000,  71000,  72000,  73000,  74000,  75000,  76000,  77000,  78000,  79000,  80000,  81000,  82000,  83000,  84000,  85000,  86000,  87000,  88000,  89000,  90000,  91000,  92000,  93000,  94000,  95000,  96000,  97000,  98000,  99000, 100000, 101000, 102000, 103000, 104000, 105000, 106000, 107000, 108000, 109000, 110000, 111000, 112000, 113000, 114000, 115000, 116000, 117000, 118000, 119000, 120000, 121000, 122000, 123000, 124000, 125000, 126000, 127000, 128000, 129000, 130000, 131000, 132000, 133000, 134000, 135000, 136000, 137000, 138000, 139000, 140000, 141000, 142000, 143000, 144000, 145000, 146000, 147000, 148000, 149000, 150000, 151000, 152000, 153000, 154000, 155000, 156000, 157000, 158000, 159000, 160000, 161000, 162000, 163000, 164000, 165000, 166000, 167000, 168000, 169000, 170000, 171000, 172000, 173000, 174000, 175000, 176000, 177000, 178000, 179000, 180000, 181000, 182000, 183000, 184000, 185000 }
   ,{   167,     67,     46,     36,     25,     21,     18,     15,     14,     12,     10,     10,      8,      7,      8,      8,      7,      7,      6,      6,      5,      6,      5,      5,      4,      4,      4,      4,      4,      4,      4,      4,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      1,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 }
   ,{   200,    101,     70,     56,     38,     31,     27,     23,     21,     19,     16,     15,     13,     12,     12,     12,     11,     10,      9,      9,      9,      8,      8,      8,      8,      7,      7,      6,      6,      6,      6,      6,      6,      5,      5,      5,      5,      5,      5,      4,      5,      4,      4,      4,      4,      4,      4,      4,      4,      4,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      1,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 }
   ,{   250,    121,     84,     66,     45,     37,     32,     27,     24,     23,     19,     18,     15,     14,     14,     14,     13,     12,     11,     11,     10,     10,      9,      9,      8,      7,      8,      8,      7,      7,      7,      7,      7,      7,      6,      6,      6,      6,      6,      5,      5,      5,      5,      5,      5,      5,      5,      5,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      1,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 }
   ,{   365,    185,    127,     99,     69,     58,     49,     41,     37,     32,     28,     28,     23,     21,     22,     20,     19,     18,     18,     16,     15,     15,     15,     14,     13,     12,     13,     12,     12,     11,     10,     10,     10,     10,     10,      9,      9,      9,      8,      8,      8,      8,      8,      8,      7,      7,      7,      7,      6,      6,      7,      6,      6,      6,      6,      6,      6,      6,      6,      6,      5,      5,      5,      5,      5,      5,      5,      5,      5,      5,      5,      5,      5,      5,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      1 }
};


GT_U32 ipgBytesTmArr[2][186] =
{
    {     0,   1000,   2000,   3000,   4000,   5000,   6000,   7000,   8000,   9000,  10000,  11000,  12000,  13000,  14000,  15000,  16000,  17000,  18000,  19000,  20000,  21000,  22000,  23000,  24000,  25000,  26000,  27000,  28000,  29000,  30000,  31000,  32000,  33000,  34000,  35000,  36000,  37000,  38000,  39000,  40000,  41000,  42000,  43000,  44000,  45000,  46000,  47000,  48000,  49000,  50000,  51000,  52000,  53000,  54000,  55000,  56000,  57000,  58000,  59000,  60000,  61000,  62000,  63000,  64000,  65000,  66000,  67000,  68000,  69000,  70000,  71000,  72000,  73000,  74000,  75000,  76000,  77000,  78000,  79000,  80000,  81000,  82000,  83000,  84000,  85000,  86000,  87000,  88000,  89000,  90000,  91000,  92000,  93000,  94000,  95000,  96000,  97000,  98000,  99000, 100000, 101000, 102000, 103000, 104000, 105000, 106000, 107000, 108000, 109000, 110000, 111000, 112000, 113000, 114000, 115000, 116000, 117000, 118000, 119000, 120000, 121000, 122000, 123000, 124000, 125000, 126000, 127000, 128000, 129000, 130000, 131000, 132000, 133000, 134000, 135000, 136000, 137000, 138000, 139000, 140000, 141000, 142000, 143000, 144000, 145000, 146000, 147000, 148000, 149000, 150000, 151000, 152000, 153000, 154000, 155000, 156000, 157000, 158000, 159000, 160000, 161000, 162000, 163000, 164000, 165000, 166000, 167000, 168000, 169000, 170000, 171000, 172000, 173000, 174000, 175000, 176000, 177000, 178000, 179000, 180000, 181000, 182000, 183000, 184000, 185000 }
   ,{     0,     20,     20,     20,     20,     20,     20,     20,     20,     20,      4,     20,     20,     20,     20,     20,     20,     20,     20,     20,      4,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,      4,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20,     20 }
};


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* speed index both to source DMA and target DMA */
/* Used for Cut Through                          */
static const GT_U32 Aldrin2_txDma_speedIndex[] =
{
    1000,   /* 0  */
    2500,   /* 1  */
    5000,   /* 2  */
    10000,  /* 3  */
    12500,  /* 4  */
    20000,  /* 5  */
    24000,  /* 6  */
    25000,  /* 7  */
    29090,  /* 8  */
    40000,  /* 9  */
    48000,  /* 10 */
    50000,  /* 11 */
    53500,  /* 12 */
    100000, /* 13 */
    102000, /* 14 */
    107000  /* 15 */
};

static const GT_U32 Bc3_txDma_speedIndex[] =
{
    1000,   /* 0  */
    2500,   /* 1  */
    5000,   /* 2  */
    10000,  /* 3  */
    12000,  /* 4  */
    12500,  /* 5  */
    20000,  /* 6  */
    24000,  /* 7  */
    25000,  /* 8  */
    40000,  /* 9  */
    48000,  /* 10 */
    50000,  /* 11 */
    100000, /* 12 */
    102000,  /* 13 */
    107000  /* 14 */
};

/**
* @internal prvCpssDxChBc3TxDmaSpeedHwIndexGet function
* @endinternal
*
* @brief   Gets HW value of speed index used for CT low to fast packets in TXDMA
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in]  devNum                - device number
* @param[in]  speedInMbps           - port speed nn Mbps
* @param[out] speedHwIndexPtr       - pointer to HW speed index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong clock
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChBc3TxDmaSpeedHwIndexGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 speedInMbps,
    OUT  GT_U32 *speedHwIndexPtr
)
{
    const GT_U32 *speedsArrPtr;
    GT_U32 speedsNum;
    GT_U32 index;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        speedsArrPtr = Bc3_txDma_speedIndex;
        speedsNum = (sizeof(Bc3_txDma_speedIndex) / sizeof(Bc3_txDma_speedIndex[0]));
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        speedsArrPtr = Aldrin2_txDma_speedIndex;
        speedsNum = (sizeof(Aldrin2_txDma_speedIndex) / sizeof(Aldrin2_txDma_speedIndex[0]));
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    for (index = 0; (index < speedsNum); index++)
    {
        if (speedInMbps == speedsArrPtr[index]) break;
        if (speedInMbps < speedsArrPtr[index])
        {
            /*speed index may be not exact for Non Cut Through mode*/
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->cutThroughEnable != GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        }
    }
    *speedHwIndexPtr = index;
    return GT_OK;
}

/*---------------------------*
 *  Aldrin Tables Start      *
 *---------------------------*/
/*-----------------------------------------------------------------------------*
 * This tables are for HW simulation (old) supporing 11 speeds
 *   {   1000,    1 }
 *  ,{   2500,    2 }
 *  ,{  10000,    3 }
 *  ,{  12000,    4 }
 *  ,{  12500,    5 }
 *  ,{  20000,    6 }
 *  ,{  24000,    7 }
 *  ,{  25000,    8 }
 *  ,{  40000,    9 }
 *  ,{  48000,   10 }
 *  ,{  50000,   11 }
 * when switching to Matlab generated change speedN  to 14
 *-----------------------------------------------------------------------------*/

extern const PRV_CPSS_DXCH_SPEED_PLACE_STC     prv_Aldrin_speedPlaceArr    [];
extern const PRV_CPSS_DXCH_CORECLOCK_PLACE_STC prv_Aldrin_coreClockPlaceArr[];

/*-----------------------------------------------------------------------------------*
 * Aldrin resource table : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
extern const GT_U32 Aldrin_TXDMA_SCDMA_TxQDescriptorCredit_Arr         [5][15];
extern const GT_U32 Aldrin_TXDMA_SCDMA_burstAlmostFullThreshold_Arr    [5][15];
extern const GT_U32 Aldrin_TXDMA_SCDMA_burstFullThreshold_Arr          [5][15];
extern const GT_U32 Aldrin_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr [5][15];
extern const GT_U32 Aldrin_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr[5][15];
extern const GT_U32 Aldrin_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr[5][15];

/*---------------------------*
 *  BC3 Tables Start      *
 *---------------------------*/

extern const PRV_CPSS_DXCH_SPEED_PLACE_STC     prv_BC3_speedPlaceArr    [];
extern const PRV_CPSS_DXCH_CORECLOCK_PLACE_STC prv_BC3_coreClockPlaceArr[];

/*-----------------------------------------------------------------------------------*
 * BC3 resource table : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
extern const GT_U32 BC3_TXDMA_SCDMA_TxQDescriptorCredit_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];
extern const GT_U32 BC3_TXDMA_SCDMA_burstAlmostFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];
extern const GT_U32 BC3_TXDMA_SCDMA_burstFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];
extern const GT_U32 BC3_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];
extern const GT_U32 BC3_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];
extern const GT_U32 BC3_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];
extern const GT_U32 BC3_TXDMA_SCDMA_RateLimitIntegerPart_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];
extern const GT_U32 BC3_TXDMA_SCDMA_RateLimitResidueVector_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];
extern const GT_U32 BC3_XLG_MAC_TxReadThreshold_Arr[2][PRV_CPSS_DXCH_PORT_RESOURCE_BC3_SPEEDS_NUM_CNS];

/*---------------------------*
 *  Aldrin2 Tables Start     *
 *---------------------------*/

extern const PRV_CPSS_DXCH_SPEED_PLACE_STC     prv_ARM2_dp01_speedPlaceArr    [];
extern const PRV_CPSS_DXCH_SPEED_PLACE_STC     prv_ARM2_dp23_speedPlaceArr    [];
extern const PRV_CPSS_DXCH_CORECLOCK_PLACE_STC prv_ARM2_coreClockPlaceArr[];

/*-----------------------------------------------------------------------------------*
 * Aldrin2 resource table : core clock (MHz) x  speed (Mbps)
 *-----------------------------------------------------------------------------------*/
extern const GT_U32 ARM2_dp01_TXDMA_SCDMA_TxQDescriptorCredit_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp01_TXDMA_SCDMA_burstAlmostFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp01_TXDMA_SCDMA_burstFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp01_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp01_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp01_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP01_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp23_TXDMA_SCDMA_TxQDescriptorCredit_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp23_TXDMA_SCDMA_burstAlmostFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp23_TXDMA_SCDMA_burstFullThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp23_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp23_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS];
extern const GT_U32 ARM2_dp23_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_CLOCK_RATES_NUM_CNS]
    [PRV_CPSS_DXCH_PORT_RESOURCE_ALDRIN2_DP23_SPEEDS_NUM_CNS];


#ifdef __cplusplus
}
#endif /* __cplusplus */



/**
* @internal prvCpssDxChCaelumPortRegAddrGet function
* @endinternal
*
* @brief   Caelum port resource registers get
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] fieldCode                - port field code name
*
* @param[out] regAddrPtr               - (pointer to) register address
* @param[out] fieldOffsetPtr           - (pointer to) register field offset
* @param[out] fieldLenPtr              - (pointer to) register field length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChCaelumPortRegAddrGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CAELUM_REG_FIELDS_ENT   fieldCode,
    OUT GT_U32                      *regAddrPtr,
    OUT GT_U32                      *fieldOffsetPtr,
    OUT GT_U32                      *fieldLenPtr
)
{
    GT_STATUS   rc;                  /* return code */
    GT_U32      dpIndexRxDma;        /* data Path index for RxDMA port */
    GT_U32      dpIndexTxDma;        /* data Path index for TxDMA port */
    GT_U32      dpIndexTxFifo;       /* data Path index for TxFIFO port */
    GT_U32      rxDmaOnDP;           /* local rxDma index for RxDMA port */
    GT_U32      txDmaOnDP;           /* local txDma index for TxDMA port */
    GT_U32      txFifoOnDP;          /* local rxFifo index for TxFIFO port */
    GT_U32      txqDqIdx;            /* txqDq idx */
    GT_U32      txqCreditResetPort;  /* port number for TxQ Credit reset register */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldLenPtr);

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*--------------------------------------------------------------------*
    * Reset the Credit Counter of TxDMA Ports                            *
    * On BobK  : using global TxDMA Port <%n> Credit Counter [n=0..73]   *
    * On Aldrin: using global TxDMA Port <%n> Credit Counter [n=0..73]   *
    * On BC3   : using local TxDMA Port <%n> Credit Counter [n=0..12]    *
    *--------------------------------------------------------------------*/
    txqCreditResetPort = portMapShadowPtr->portMap.txDmaNum;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)) /* Falcon */
    {
        dpIndexRxDma  = 0;
        dpIndexTxDma  = 0;
        dpIndexTxFifo = 0;
        txqDqIdx      = 0;
        rxDmaOnDP  = portMapShadowPtr->portMap.rxDmaNum;
        txDmaOnDP  = portMapShadowPtr->portMap.txDmaNum;
        txFifoOnDP = portMapShadowPtr->portMap.txFifoPortNum;

        /* NOTE: all cases that will call 'TXQ-DQ' related credits will FAIL !!! */
    }
    else
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
    {
        dpIndexRxDma  = 0;
        dpIndexTxDma  = 0;
        dpIndexTxFifo = 0;
        txqDqIdx      = 0;
        rxDmaOnDP  = portMapShadowPtr->portMap.rxDmaNum;
        txDmaOnDP  = portMapShadowPtr->portMap.txDmaNum;
        txFifoOnDP = portMapShadowPtr->portMap.txFifoPortNum;

        if(PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E == fieldCode)
        {
            rc = prvCpssDxChPpResourcesTxqGlobal2LocalConvert(devNum,portMapShadowPtr->portMap.txqNum,/*OUT*/&txqDqIdx,NULL);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txDmaNum,/*OUT*/NULL, &txqCreditResetPort);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.rxDmaNum,     /*OUT*/&dpIndexRxDma, &rxDmaOnDP  );
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txDmaNum,     /*OUT*/&dpIndexTxDma, &txDmaOnDP  );
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txFifoPortNum,/*OUT*/&dpIndexTxFifo,&txFifoOnDP );
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPpResourcesTxqGlobal2LocalConvert(devNum,portMapShadowPtr->portMap.txqNum,/*OUT*/&txqDqIdx,NULL);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

#ifdef GM_USED
    dpIndexRxDma = 0;
    dpIndexTxDma = 0;
    dpIndexTxFifo = 0;
#endif

    switch(fieldCode)
    {
        case PRV_CAELUM_REG_FIELDS_TXQ_Q_PORT_DESC_COUNTER_E:
            PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);
            PRV_CPSS_DXCH_PORT_DESCR_COUNT_REG_MAC(devNum, portNum, regAddrPtr); /* bad , next ASIC will be unclear*/
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                *fieldOffsetPtr = 0;
                *fieldLenPtr = 16;
            }
            else
            {
                *fieldOffsetPtr = 0;
                *fieldLenPtr = 14;
            }
            break;

        case PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_PORT_CREDIT_COUNTER_E: /* BC3/Aldrin usage ???*/
            PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);
            /*--------------------------------------------------------------------*
            * On BobK  : using global TxDMA Port <%n> Credit Counter [n=0..73]   *
            * On Aldrin: using global TxDMA Port <%n> Credit Counter [n=0..73]   *
            * On BC3   : using local TxDMA Port <%n> Credit Counter [n=0..12]    *
            *--------------------------------------------------------------------*/
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,txqDqIdx).global.creditCounters.txdmaPortCreditCounter[portMapShadowPtr->portMap.txDmaNum];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 12;
            break;

        case PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E:
            PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);
            /*--------------------------------------------------------------------*
            * Reset the Credit Counter of TxDMA Ports                            *
            * On BobK  : using global TxDMA Port <%n> Credit Counter [n=0..73]   *
            * On Aldrin: using global TxDMA Port <%n> Credit Counter [n=0..73]   *
            * On BC3   : using local TxDMA Port <%n> Credit Counter [n=0..12]    *
            *--------------------------------------------------------------------*/
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,txqDqIdx).global.globalDQConfig.creditCountersReset[txqCreditResetPort/32];
            *fieldOffsetPtr =  (txqCreditResetPort % 32) ;
            *fieldLenPtr = 1;
            break;

        case PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMADebug.informativeDebug.SCDMAStatusReg1[txDmaOnDP];
            *fieldOffsetPtr = 10;
            *fieldLenPtr = 10;
            break;

        case PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMADebug.informativeDebug.SCDMAStatusReg1[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 10;
            break;

        case PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.SCDMAConfigs2[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 1;
            break;
        /*--------------------------------------------------*
         * resources                                        *
         *--------------------------------------------------*/
        case PRV_CAELUM_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_RXDMA_INDEX_MAC(devNum, dpIndexRxDma).singleChannelDMAConfigs.SCDMAConfig0[rxDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 3;
            break;

        case PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 9;
            break;

        case PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[txDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 10;
            break;

        case PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[txDmaOnDP];
            *fieldOffsetPtr = 10;
            *fieldLenPtr = 10;
            break;

        case PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E:       /* BC3 only */
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.burstLimiterSCDMA[txDmaOnDP];
                *fieldOffsetPtr = 0;
                *fieldLenPtr = 16;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E:              /* BC3 only */
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.burstLimiterSCDMA[txDmaOnDP];
                *fieldOffsetPtr =16;
                *fieldLenPtr = 16;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case     PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E:      /* BC3 only */
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.SCDMAConfigs1[txDmaOnDP];
                *fieldOffsetPtr =20;
                *fieldLenPtr   = 10;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E: /* BC3 only */
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.SCDMARateLimitResidueVectorThr[txDmaOnDP];
                *fieldOffsetPtr = 0;
                *fieldLenPtr    = 32;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case PRV_CAELUM_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E:
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAGlobalConfigs.srcSCDMASpeedTbl[txDmaOnDP/8];
                *fieldOffsetPtr = (txDmaOnDP%8)*4;
                *fieldLenPtr = 4;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case PRV_CAELUM_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E:
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(devNum, dpIndexTxDma).txDMAPerSCDMAConfigs.SCDMAConfigs1[txDmaOnDP];
                *fieldOffsetPtr = 16;
                *fieldLenPtr = 4;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TX_FIFO_INDEX_MAC(devNum, dpIndexTxFifo).txFIFOGlobalConfig.SCDMAPayloadThreshold[txFifoOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 7;
            break;

        case PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_TX_FIFO_INDEX_MAC(devNum, dpIndexTxFifo).txFIFOShiftersConfig.SCDMAShiftersConf[txFifoOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 3;
            break;

        case PRV_CAELUM_REG_FIELDS_ETH_TXFIFO_OUTGOING_BUS_WIDTH_E:
            *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SIP_ETH_TXFIFO[dpIndexRxDma].txFIFOShiftersConfig.SCDMAShiftersConf[rxDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 3;
            break;

        case PRV_CAELUM_REG_FIELDS_ETH_TXFIFO_SCDMA_PAYLOAD_THRESHOLD_E:
            *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->SIP_ETH_TXFIFO[dpIndexRxDma].txFIFOGlobalConfig.SCDMAPayloadThreshold[rxDmaOnDP];
            *fieldOffsetPtr = 0;
            *fieldLenPtr = 8;
            break;
        case PRV_CAELUM_REG_FIELDS_XLG_MAC_FIFO_TX_READ_THRESHOLD_E:
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMapShadowPtr->portMap.macNum].xlgPortFIFOsThresholdsConfig;
                *fieldOffsetPtr = 11;
                *fieldLenPtr    = 5;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChCaelumPortRegFieldSet function
* @endinternal
*
* @brief   Caelum port resource register field set
*
* @note   APPLICABLE DEVICES:      Caelum, Aldrin, AC3X,  Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] fieldCode                - field code name
* @param[in] fieldValue               - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortRegFieldSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CAELUM_REG_FIELDS_ENT   fieldCode,
    IN  GT_U32                      fieldValue
)
{
    GT_STATUS   rc;             /*return code*/
    GT_U32      regAddr;        /*register address*/
    GT_U32      fieldOffset;    /*register field offset*/
    GT_U32      fieldLen;       /*register field length*/

    rc = prvCpssDxChCaelumPortRegAddrGet(devNum, portNum, fieldCode,
                                         &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLen, fieldValue);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChCaelumPortRegFieldGet function
* @endinternal
*
* @brief   Caelum port resource register field get
*
* @note   APPLICABLE DEVICES:      Caelum, Aldrin, AC3X,  Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] fieldCode                - field code name
*
* @param[out] fieldValuePtr            - (pointer to) field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChCaelumPortRegFieldGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CAELUM_REG_FIELDS_ENT   fieldCode,
    OUT GT_U32                      *fieldValuePtr
)
{
    GT_STATUS   rc;             /*return code*/
    GT_U32      regAddr;        /*register address*/
    GT_U32      fieldOffset;    /*register field offset*/
    GT_U32      fieldLen;       /*register field length*/

    CPSS_NULL_PTR_CHECK_MAC(fieldValuePtr);

    rc = prvCpssDxChCaelumPortRegAddrGet(devNum, portNum, fieldCode,
                                         &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, fieldValuePtr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChCaelumPortAllDataPathRegFieldSet function
* @endinternal
*
* @brief   Caelum port resource register field set on all data pathes
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X, Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] fieldCode                - field code name
* @param[in] fieldValue               - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortAllDataPathRegFieldSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CAELUM_REG_FIELDS_ENT   fieldCode,
    IN  GT_U32                      fieldValue
)
{
    GT_STATUS   rc;             /*return code*/
    GT_U32      regAddr;        /*register address*/
    GT_U32      fieldOffset;    /*register field offset*/
    GT_U32      fieldLen;       /*register field length*/

    rc = prvCpssDxChCaelumPortRegAddrGet(devNum, portNum, fieldCode,
                                         &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLen, fieldValue);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}




/**
* @internal prvCpssDxChCaelumPortResourcesSpeedIndexGet function
* @endinternal
*
* @brief   Gets speed index in database
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - data path number
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortResourcesSpeedIndexGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               dpIndex,
    IN  CPSS_PORT_SPEED_ENT  speed,
    OUT GT_U32               *speedIndexPtr,
    OUT GT_U32               *speedValuePtr
)
{
    GT_U32 i;
    GT_U32                         speedMbps;
    GT_U32                         speedIdx;
    const PRV_CPSS_DXCH_SPEED_PLACE_STC *tablePtr;

    CPSS_NULL_PTR_CHECK_MAC(speedIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(speedValuePtr);

    /*------------------------------------*
     * find speed in Mbps                 *
     * then find speed index in tables    *
     *------------------------------------*/
    speedMbps = 0;
    for (i = 0 ; i < sizeof(prv_SpeedEnmSpeedMbpsArr)/sizeof(prv_SpeedEnmSpeedMbpsArr[0]); i++)
    {
        if (prv_SpeedEnmSpeedMbpsArr[i].speedEnm == speed)
        {
            speedMbps = prv_SpeedEnmSpeedMbpsArr[i].speedMbps;
            break;
        }
    }
    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum) )
    {
        tablePtr = &prv_Cetus_speedPlaceArr[0];
    }
    else if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        tablePtr = &prv_Aldrin_speedPlaceArr[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
             PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        tablePtr = &prv_BC3_speedPlaceArr[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        switch (dpIndex)
        {
        case 0:
        case 1:
            tablePtr = &prv_ARM2_dp01_speedPlaceArr[0];
            break;
        case 2:
        case 3:
            tablePtr = &prv_ARM2_dp23_speedPlaceArr[0];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    speedIdx = 0;
    for (i = 0 ; tablePtr[i].speedMbps != 0 ; i++)
    {
        if (tablePtr[i].speedMbps == speedMbps)
        {
            speedIdx = tablePtr[i].placeInArr;
            break;
        }
    }
    if (speedIdx == 0) /* not found */
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *speedIndexPtr = speedIdx;
    *speedValuePtr = speedMbps;

    return GT_OK;
}


/**
* @internal prvCpssDxChCaelumPortResourcesClockIndexGet function
* @endinternal
*
* @brief   Gets core clock index in database
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] coreClock                - device core clock
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong clock
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourcesClockIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  coreClock,
    OUT GT_U32  *clockIndexPtr
)
{
    GT_U32 i;
    const PRV_CPSS_DXCH_CORECLOCK_PLACE_STC * tablePtr;
    CPSS_NULL_PTR_CHECK_MAC(clockIndexPtr);

    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
    {
        tablePtr = &prv_Cetus_coreClockPlaceArr[0];
    }
    else if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        tablePtr = &prv_Aldrin_coreClockPlaceArr[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        tablePtr = &prv_BC3_coreClockPlaceArr[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        tablePtr = &prv_ARM2_coreClockPlaceArr[0];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; tablePtr[i].real_coreClockMHz != 0 ; i++)
    {
        if (tablePtr[i].real_coreClockMHz == coreClock)
        {
            *clockIndexPtr = tablePtr[i].placeInArr;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/*--------------------------------------------------------------------------------------*
 *  find at least one port connected to TM                                              *
 *  if there is no ports mapped to TM, the determination of TM bandwidth is meaningless *
 *--------------------------------------------------------------------------------------*/
GT_STATUS prvCpssDxChCaelumTmAnyPortGet
(
    IN   GT_U8 devNum,
    OUT  GT_PHYSICAL_PORT_NUM *portNumPtr
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    for(portNum = 0; portNum < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); portNum++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortResourceTmBandwidthSet: error in prvCpssDxChPortPhysicalPortMapShadowDBGet for port = %d\n", portNum);
        }
        if((portMapShadowPtr->valid == GT_TRUE) && (portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE))
        {
            *portNumPtr = portNum;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"cpssDxChPortResourceTmBandwidthSet: there are no ports connected to TM\n");
}

/**
* @internal prvCpssDxChCaelumPortResourceConfigSet function
* @endinternal
*
* @brief   Set resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourceConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_PHYSICAL_PORT_NUM                         portNum,
    IN PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC       *resourcePtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    /* The configuration is not applicable under fast boot flag for all ports but CPU */
    if ((systemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E) &&
        (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        return GT_OK;
    }

    /************************************************************************/
    /* 1. TxDMA 73(TM) - TxQ Credits                                        */
    /************************************************************************/
    if(resourcePtr->txdmaCreditValue != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
    {
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                              resourcePtr->txdmaCreditValue);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /************************************************************************/
    /* 2. TxDMA 73(TM) - TxFIFO Header threshold                            */
    /************************************************************************/
    if(resourcePtr->txdmaTxfifoHeaderCounterThresholdScdma != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
    {
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
                                              resourcePtr->txdmaTxfifoHeaderCounterThresholdScdma);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /************************************************************************/
    /* 3. TxDMA 73(TM) - TxFIFO Payload threshold                           */
    /************************************************************************/
    if(resourcePtr->txdmaTxfifoPayloadCounterThresholdScdma != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
    {
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
                                              resourcePtr->txdmaTxfifoPayloadCounterThresholdScdma);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /************************************************************************/
    /* 4. TxFIFO 73(TM) - SCDMA Payload threshold                           */
    /************************************************************************/
    if(resourcePtr->txfifoScdmaPayloadStartTransmThreshold != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
    {
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E,
                                              resourcePtr->txfifoScdmaPayloadStartTransmThreshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /************************************************************************/
    /* 5. TxFIFO 73(TM) - TXFIFO Outgoing Bus Width                         */
    /************************************************************************/
    if(resourcePtr->txfifoScdmaShiftersOutgoingBusWidth != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
    {
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E,
                                              resourcePtr->txfifoScdmaShiftersOutgoingBusWidth);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /************************************************************************/
    /* 6. RxDMA (port_X) - RxDMA IF_WIDTH                                   */
    /************************************************************************/
    if(resourcePtr->rxdmaScdmaIncomingBusWidth != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
    {
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E,
                                              resourcePtr->rxdmaScdmaIncomingBusWidth);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }


    if(portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
    {
        /************************************************************************/
        /* 7. Eth-TXFIFO IF width                                               */
        /************************************************************************/
        if(resourcePtr->ethTxfifoOutgoingBusWidth != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
        {
            rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                                  PRV_CAELUM_REG_FIELDS_ETH_TXFIFO_OUTGOING_BUS_WIDTH_E,
                                                  resourcePtr->ethTxfifoOutgoingBusWidth);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        /************************************************************************/
        /* 8. Eth-TXFIFO Payload StartTrasmThreshold */
        /************************************************************************/
        if(resourcePtr->ethTxfifoScdmaPayloadThreshold != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
        {
            rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                                  PRV_CAELUM_REG_FIELDS_ETH_TXFIFO_SCDMA_PAYLOAD_THRESHOLD_E,
                                                  resourcePtr->ethTxfifoScdmaPayloadThreshold);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChCaelumPortResourceConfigGet function
* @endinternal
*
* @brief   Get resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortResourceConfigGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC      *resourcePtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;


    resourcePtr->rxdmaScdmaIncomingBusWidth              = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaCreditValue                        = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaBurstFullThrshold             = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaBurstAmostFullThrshold        = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaRateLimitIntegerThreshold     = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaScdmaRateLimitResidueVectThreshold = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaTxfifoHeaderCounterThresholdScdma  = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txdmaTxfifoPayloadCounterThresholdScdma = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txfifoScdmaPayloadStartTransmThreshold  = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->txfifoScdmaShiftersOutgoingBusWidth     = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->ethTxfifoOutgoingBusWidth               = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    resourcePtr->ethTxfifoScdmaPayloadThreshold          = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;

    /* 1. TxDMA 73(TM) - TxQ Credits */
    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                          &resourcePtr->txdmaCreditValue);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* 2. TxDMA 73(TM) - TxFIFO Header threshold */
    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
                                          &resourcePtr->txdmaTxfifoHeaderCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* 3. TxDMA 73(TM) - TxFIFO Payload threshold */
    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
                                          &resourcePtr->txdmaTxfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* 4. TxFIFO 73(TM) - SCDMA Payload threshold */
    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E,
                                          &resourcePtr->txfifoScdmaPayloadStartTransmThreshold);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* 5. TxFIFO 73(TM) - TXFIFO Outgoing Bus Width */
    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E,
                                          &resourcePtr->txfifoScdmaShiftersOutgoingBusWidth);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /* 6. RxDMA (port_X) - RxDMA IF_WIDTH */
    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E,
                                          &resourcePtr->rxdmaScdmaIncomingBusWidth);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
    {
        rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E, /*OUT*/&resourcePtr->txdmaScdmaBurstAmostFullThrshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E, /*OUT*/&resourcePtr->txdmaScdmaBurstFullThrshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E,     /*OUT*/&resourcePtr->txdmaScdmaRateLimitIntegerThreshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E,/*OUT*/&resourcePtr->txdmaScdmaRateLimitResidueVectThreshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
    {
        /* 8. Eth-TXFIFO IF width */
        rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_ETH_TXFIFO_OUTGOING_BUS_WIDTH_E,
                                              &resourcePtr->ethTxfifoOutgoingBusWidth);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }


        /* 9. Eth-TXFIFO Payload StartTrasmThreshold */
        rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_ETH_TXFIFO_SCDMA_PAYLOAD_THRESHOLD_E,
                                              &resourcePtr->ethTxfifoScdmaPayloadThreshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        resourcePtr->ethTxfifoOutgoingBusWidth = 0;
        resourcePtr->ethTxfifoScdmaPayloadThreshold = 0;
    }

    return GT_OK;
}


/*---------------------------------------------*
 * Dynamic Resource Calculate Caelum           *
 *---------------------------------------------*/
static GT_U32 prvCpssDxChRoundUpPositive
(
    IN  float a
)
{
    if((a-(GT_U32)(a)) > 0)
        return (GT_U32)(a+1);
    else
        return (GT_U32)a;
}


static GT_U32 prvCpssDxChDiffRoundUp
(
    IN  GT_U32 a,
    IN  GT_U32 b
)
{
    return (a + b - 1) / b;
}


/**
* @internal prvCpssDxChCaelumPortResourcesCalculationDynamic function
* @endinternal
*
* @brief   Caelum port new resource calculations (dynamic mode)
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port interface speed
*
* @param[out] portResourcePtr          - (pointer to) port resource struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourcesCalculationDynamic
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_SPEED_ENT                     speed,
    IN  GT_BOOL                                 isExistingPort,
    OUT PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC  *portResourcePtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      dpIndex;                /* data path number of portNum */
    GT_U32      speedIndex;             /* index in speed DB */
    GT_U32      coreClockIndex;         /* index in clock DB */
    GT_U32      txdmaCreditValue;       /* TXDMA descriptor credit number */
    GT_U32      mppmMinLatency=64;      /* MPPM minimum latency */
    GT_U32      perBankFifoSize=35;     /* per bank FIFO size */
    GT_U32      othersLatencyCore0=301; /* other latency DP_0 */
    GT_U32      othersLatencyCore1=281; /* other latency DP_1 */
    GT_U32      othersLatency;          /* other latency */
    GT_U32      worstCaseAck2AckDiff;   /* ACK to ACK diff per speed per core clock */
    GT_U32      osBw;                   /* OS bandwidth per core clock */
    float       transmitTime64BC;
    GT_U32      mppmMaxLatency;         /* MPPM maximum latency */
    GT_U32      maxLatency;             /* maximum latency */
    GT_U32      speedValueMbps;
    GT_U32      headerCounterThreshold;
    GT_U32      payloadCounterThreshold;
    GT_U32      txfifoScdmaShiftersOutgoingBusWidth;
    GT_U32      rxdmaScdmaIncomingBusWidth;
    GT_U32      mppmLatencyVariance;
    float       minThresholdForTransmission;
    GT_U32      coreClock;
    GT_U32      sumPortSpeed;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    float       speedValueGbps;


    CPSS_NULL_PTR_CHECK_MAC(portResourcePtr);

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    dpIndex = portMapShadowPtr->portMap.txDmaNum >= PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS;

    rc = prvCpssDxChCaelumPortResourcesSpeedIndexGet(devNum,dpIndex,speed, &speedIndex, &speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChCaelumPortResourcesClockIndexGet(devNum,PRV_CPSS_PP_MAC(devNum)->coreClock, &coreClockIndex);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch(speed)
    {
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
            txfifoScdmaShiftersOutgoingBusWidth = PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E;
            rxdmaScdmaIncomingBusWidth = PRV_CPSS_RXDMA_IfWidth_64_E;
            break;

        case CPSS_PORT_SPEED_10000_E:
        case CPSS_PORT_SPEED_12000_E:
        case CPSS_PORT_SPEED_12500_E:
        case CPSS_PORT_SPEED_11800_E:
        case CPSS_PORT_SPEED_2500_E:
        case CPSS_PORT_SPEED_5000_E:
            txfifoScdmaShiftersOutgoingBusWidth = PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E;
            rxdmaScdmaIncomingBusWidth = PRV_CPSS_RXDMA_IfWidth_64_E;
            break;

        case CPSS_PORT_SPEED_20000_E:
        case CPSS_PORT_SPEED_25000_E:
        case CPSS_PORT_SPEED_29090_E:
        case CPSS_PORT_SPEED_40000_E:
        case CPSS_PORT_SPEED_47200_E:
            txfifoScdmaShiftersOutgoingBusWidth = PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E;
            rxdmaScdmaIncomingBusWidth = PRV_CPSS_RXDMA_IfWidth_256_E;
            break;

        case CPSS_PORT_SPEED_75000_E:
            txfifoScdmaShiftersOutgoingBusWidth = PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E;
            rxdmaScdmaIncomingBusWidth = PRV_CPSS_RXDMA_IfWidth_512_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /*--------------------------------------------------------------------*
     *  correct for CPU txFifo-outgoing bus width DMA 72 has width 8Bytes *
     *--------------------------------------------------------------------*/
    if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        txfifoScdmaShiftersOutgoingBusWidth = PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E;
    }

    if(portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
    {
        /* for TM enabled port no need to calculate resources */

        portResourcePtr->rxdmaScdmaIncomingBusWidth          = rxdmaScdmaIncomingBusWidth;
        portResourcePtr->ethTxfifoOutgoingBusWidth           = txfifoScdmaShiftersOutgoingBusWidth;
        portResourcePtr->ethTxfifoScdmaPayloadThreshold      = 0xA0;

        portResourcePtr->txdmaCreditValue                        = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        portResourcePtr->txdmaTxfifoHeaderCounterThresholdScdma  = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        portResourcePtr->txdmaTxfifoPayloadCounterThresholdScdma = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        portResourcePtr->txfifoScdmaPayloadStartTransmThreshold  = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        portResourcePtr->txfifoScdmaShiftersOutgoingBusWidth     = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;

        return GT_OK;
    }
    else
    {
        portResourcePtr->rxdmaScdmaIncomingBusWidth = rxdmaScdmaIncomingBusWidth;
        portResourcePtr->txfifoScdmaShiftersOutgoingBusWidth = txfifoScdmaShiftersOutgoingBusWidth;
    }

    /*
        1. TXDMA desc credits - each DMA, according to its speed, requires a certain amount of descriptors
           allocated space inside the TXDMA to be able to support FWS (desc_credits_scdma%p).

        TXDMA 0
        Register:   <TXDMA0_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
        Fields:     BITS 0-8: Descriptor Credits SCDMA %p
        Limitation: SUM(DescriptorCreditsScdma_cc {PortSpeed,CoreFreq}) <= 144

        TXDMA 1
        Register:   <TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
        Fields:     BITS 0-8: Descriptor Credits SCDMA %p
        Limitation: SUM(DescriptorCreditsScdma_cc {PortSpeed,CoreFreq}) <= 144

    */
    if((portMapShadowPtr->portMap.macNum < PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS) &&
       (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        txdmaCreditValue = prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditCore0Arr[coreClockIndex][speedIndex];

        /* if port exists - no need to update overall speed bandwidth,
           resources for this port just should be recalculated following other port add/delete */
        if(isExistingPort == GT_FALSE)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[0] += speedValueMbps;
        }
        sumPortSpeed = PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[0];

        othersLatency  = othersLatencyCore0;
        worstCaseAck2AckDiff = prvCpssDxChBobkWorstCaseAck2AckDiffCore0Arr[coreClockIndex][speedIndex];

        osBw = maxBwArr[coreClockIndex][1];
    }
    else
    {
        txdmaCreditValue = prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditCore1Arr[coreClockIndex][speedIndex];

        /* if port exists - no need to update overall speed bandwidth,
           resources for this port just should be recalculated following other port add/delete */
        if(isExistingPort == GT_FALSE)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1] += speedValueMbps;
        }
        sumPortSpeed = PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1];

        othersLatency  = othersLatencyCore1 - otherLatencyDiffArrayCore1[coreClockIndex][1];
        worstCaseAck2AckDiff = prvCpssDxChBobkWorstCaseAck2AckDiffCore1Arr[coreClockIndex][speedIndex];

        osBw = maxBwArr[coreClockIndex][2];
    }

    portResourcePtr->txdmaCreditValue = txdmaCreditValue;

    /*
        2. TXFIFO header and payload credits - each DMA, according to its speed,
           requires a certain amount of space allocated inside the TXDMA for both
           headers and payload to achieve FWS and avoid under-runs

        TXDMA 0
        Register:   <TXDMA0_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
        Fields:     BITS 0-9: TxFIFO Header Counter Threshold SCDMA %p
                    BITS 10-19: TxFIFO Payload Counter Threshold SCDMA %p
        Limitation: SUM(Header {PortSpeed,CoreFreq}) <= 276
                    SUM(Payload {PortSpeed,CoreFreq}) <= 387

        TXDMA 1
        Register:   <TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
        Fields:     BITS 0-9: TxFIFO Header Counter Threshold SCDMA %p
                    BITS 10-19: TxFIFO Payload Counter Threshold SCDMA %p
        Limitation: SUM(Header {PortSpeed,CoreFreq}) <= 363
                    SUM(Payload {PortSpeed,CoreFreq}) <= 447

    */
    speedValueGbps = (float)speedValueMbps / 1000;

    transmitTime64BC = (float)(((64+ipgBytesArr[1][speedIndex]) * 8 ) / ((float)speedValueMbps / 1000));

    /* round up (perBankFifoSize*osBw*2 / sumPortSpeed in Gbps) */
    if (sumPortSpeed == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"division on 0 something wrong , it can't be 0");
    }
    mppmMaxLatency = mppmMinLatency + prvCpssDxChDiffRoundUp((perBankFifoSize*osBw*2*1000), sumPortSpeed);

    maxLatency = ((mppmMaxLatency + othersLatency + worstCaseAck2AckDiff) - ((mppmMaxLatency + othersLatency + worstCaseAck2AckDiff)%worstCaseAck2AckDiff)) * 1000;
    /* round up (maxLatency / coreClock) */
    maxLatency = prvCpssDxChDiffRoundUp(maxLatency, PRV_CPSS_PP_MAC(devNum)->coreClock);

    /* round up (maxLatency / transmitTime64BC) */
    if (transmitTime64BC == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"division on 0 something wrong , it can't be 0");
    }
    headerCounterThreshold = prvCpssDxChDiffRoundUp(maxLatency, (GT_U32)transmitTime64BC);

    portResourcePtr->txdmaTxfifoHeaderCounterThresholdScdma = headerCounterThreshold;

    mppmLatencyVariance = mppmMaxLatency - mppmMinLatency;


    coreClock = PRV_CPSS_PP_MAC(devNum)->coreClock;

    minThresholdForTransmission = (float)prvCpssDxChRoundUpPositive((float)prvCpssDxChRoundUpPositive((float)mppmLatencyVariance / coreClock * 1000) * speedValueGbps / 1024) +
                                  (float)prvCpssDxChRoundUpPositive(2 * ((float)osBw/prvCpssDxChRoundUpPositive(speedValueGbps)) * 1000 / coreClock * speedValueGbps / 1024);

    payloadCounterThreshold = headerCounterThreshold + (GT_U32)minThresholdForTransmission;
    portResourcePtr->txdmaTxfifoPayloadCounterThresholdScdma = payloadCounterThreshold;

    /*
        3. Payload Threshold - to avoid under-runs due to variance in the
           latency of read responses from the MPPM - the TXFIFO is required
           to start transmitting the packet only after reaching a certain
           threshold. Each DMA speeds require a different threshold
           (scdma_%p_payload_threshold).
    */
    portResourcePtr->txfifoScdmaPayloadStartTransmThreshold = (GT_U32)minThresholdForTransmission;

    return GT_OK;
}


/**
* @internal prvCpssDxChCaelumPortResourcesCalculationDynamicTm function
* @endinternal
*
* @brief   Caelum port new resource calculations
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] speedMbps                - port speed in Mbps
*
* @param[out] portResourcePtr          - (pointer to) port resource struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, speedMbps
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourcesCalculationDynamicTm
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  speedMbps,
    OUT PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC  *portResourcePtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      coreClockIndex;         /* index in clock DB */
    GT_U32      mppmMinLatency=64;      /* MPPM minimum latency */
    GT_U32      perBankFifoSize=35;     /* per bank FIFO size */
    GT_U32      othersLatencyCore1=281; /* other latency DP_1 */
    GT_U32      othersLatency;          /* other latency */
    GT_U32      worstCaseAck2AckDiff;   /* ACK to ACK diff per speed per core clock */
    GT_U32      osBw;                   /* OS bandwidth per core clock */
    float       transmitTime64BC;
    GT_U32      mppmMaxLatency;         /* MPPM maximum latency */
    GT_U32      maxLatency;             /* maximum latency */
    GT_U32      headerCounterThreshold;
    GT_U32      payloadCounterThreshold;
    GT_U32      mppmLatencyVariance;
    float       minThresholdForTransmission;
    GT_U32      coreClock;
    GT_U32      sumPortSpeedMbps;
    float       speedValueGbps;
    GT_U32      ipg;



    CPSS_NULL_PTR_CHECK_MAC(portResourcePtr);

    rc = prvCpssDxChCaelumPortResourcesClockIndexGet(devNum,PRV_CPSS_PP_MAC(devNum)->coreClock, &coreClockIndex);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    portResourcePtr->rxdmaScdmaIncomingBusWidth = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    portResourcePtr->ethTxfifoOutgoingBusWidth = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    portResourcePtr->ethTxfifoScdmaPayloadThreshold = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;

    portResourcePtr->txfifoScdmaShiftersOutgoingBusWidth = PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E;

    /*
        1. TXDMA desc credits - each DMA, according to its speed, requires a certain amount of descriptors
           allocated space inside the TXDMA to be able to support FWS (desc_credits_scdma%p).

    */

    portResourcePtr->txdmaCreditValue = prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditTmArr[coreClockIndex][speedMbps / 1000];

    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1] -= PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps;
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1] += speedMbps;
    sumPortSpeedMbps = PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1];

    othersLatency  = othersLatencyCore1 - otherLatencyDiffArrayCore1[coreClockIndex][1];
    worstCaseAck2AckDiff = prvCpssDxChBobkWorstCaseAck2AckDiffCore1TmArr[coreClockIndex][speedMbps / 1000];

    osBw = maxBwArr[coreClockIndex][2];

    /*
        2. TXFIFO header and payload credits - each DMA, according to its speed,
           requires a certain amount of space allocated inside the TXDMA for both
           headers and payload to achieve FWS and avoid under-runs
    */
    speedValueGbps = (float)speedMbps / 1000;

    ipg = ipgBytesTmArr[1][speedMbps / 1000];

    transmitTime64BC = ((float)((64+ipg) * 8 ) / (GT_U32)(speedValueGbps));

    /* round up (perBankFifoSize*osBw*2 / sumPortSpeed in Gbps) */
    if (sumPortSpeedMbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"division on 0 something wrong , it can't be 0");
    }
    mppmMaxLatency = mppmMinLatency + prvCpssDxChDiffRoundUp((perBankFifoSize*osBw*2*1000), sumPortSpeedMbps);

    maxLatency = ((mppmMaxLatency + othersLatency + worstCaseAck2AckDiff) - ((mppmMaxLatency + othersLatency + worstCaseAck2AckDiff)%worstCaseAck2AckDiff)) * 1000;
    /* round up (maxLatency / coreClock) */
    maxLatency = prvCpssDxChDiffRoundUp(maxLatency, PRV_CPSS_PP_MAC(devNum)->coreClock);

    /* round up (maxLatency / transmitTime64BC) */
    if (transmitTime64BC == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"division on 0 something wrong , it can't be 0");
    }
    headerCounterThreshold = prvCpssDxChDiffRoundUp(maxLatency, (GT_U32)transmitTime64BC);

    portResourcePtr->txdmaTxfifoHeaderCounterThresholdScdma = headerCounterThreshold;

    mppmLatencyVariance = mppmMaxLatency - mppmMinLatency;


    coreClock = PRV_CPSS_PP_MAC(devNum)->coreClock;

    minThresholdForTransmission = (float)prvCpssDxChRoundUpPositive((float)prvCpssDxChRoundUpPositive((float)mppmLatencyVariance / coreClock * 1000) * speedValueGbps / 1024) +
                                  (float)prvCpssDxChRoundUpPositive(2 * ((float)osBw/prvCpssDxChRoundUpPositive(speedValueGbps)) * 1000 / coreClock * speedValueGbps / 1024);

    payloadCounterThreshold = headerCounterThreshold + (GT_U32)minThresholdForTransmission;
    portResourcePtr->txdmaTxfifoPayloadCounterThresholdScdma = payloadCounterThreshold;

    /*
        3. Payload Threshold - to avoid under-runs due to variance in the
           latency of read responses from the MPPM - the TXFIFO is required
           to start transmitting the packet only after reaching a certain
           threshold. Each DMA speeds require a different threshold
           (scdma_%p_payload_threshold).
    */
    portResourcePtr->txfifoScdmaPayloadStartTransmThreshold = (GT_U32)minThresholdForTransmission;

    return GT_OK;
}


static PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_DATABASE_STC  portResDb[CPSS_MAX_PORTS_NUM_CNS];

/**
* @internal prvCpssDxChCaelumPortResourcesLimitationCheck function
* @endinternal
*
* @brief   Caelum port new resource calculations
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourcesLimitationCheck
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum
)
{
    GT_STATUS rc;                       /* return code */
    GT_PHYSICAL_PORT_NUM port;          /* port loop iterator */
    GT_U32 usedDescCredits = 0;      /* credits counter */
    GT_U32 usedHeaderCredits = 0;    /* header counter */
    GT_U32 usedPayloadCredits = 0;   /* payload counter */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowIteratorPtr;
    GT_U32 dpIndex;
    GT_BOOL isTmCounted = GT_FALSE;

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(portMapShadowPtr->portMap.macNum < PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS)
    {
        dpIndex = 0;
    }
    else
    {
        dpIndex = 1;
    }
    if(portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
    {
        dpIndex = 1;
    }

    for(port = 0; (port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)) &&
                                        (port < CPSS_MAX_PORTS_NUM_CNS); port++)
    {
        if(portResDb[port].valid == GT_TRUE)
        {

            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, port, /*OUT*/&portMapShadowIteratorPtr);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            /* resources of TM ports should be counted only once */
            if((isTmCounted == GT_FALSE) || (portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_FALSE))
            {
                usedDescCredits    += portResDb[port].res.txdmaCreditValue;
                usedHeaderCredits  += portResDb[port].res.txdmaTxfifoHeaderCounterThresholdScdma;
                usedPayloadCredits += portResDb[port].res.txdmaTxfifoPayloadCounterThresholdScdma;
            }

            if(portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_TRUE)
            {
                isTmCounted = GT_TRUE;
            }
        }
    }

    if(usedDescCredits > PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.maxDescCredits[dpIndex])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "prvCpssDxChCaelumPortResourcesLimitationCheck: usedDescCredits for DP%d out of range %d\n", dpIndex, usedDescCredits);
    }
    if(usedHeaderCredits > PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.maxHeaderCredits[dpIndex])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "prvCpssDxChCaelumPortResourcesLimitationCheck: usedHeaderCredits for DP%d out of range %d\n", dpIndex, usedHeaderCredits);
    }
    if(usedPayloadCredits > PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.maxPayloadCredits[dpIndex])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "prvCpssDxChCaelumPortResourcesLimitationCheck: usedPayloadCredits for DP%d out of range %d\n", dpIndex, usedPayloadCredits);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChCaelumPortResourcesConfigDynamic function
* @endinternal
*
* @brief   Caelum port resource dynamic configuration
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourcesConfigDynamic
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS                               rc;                 /* return code */
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC  portNewResource;    /* new port resource */
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC  portCurrResource;   /* current port resource */
    GT_U32                                  speedIndex;         /* index in speed DB */
    GT_U32                                  speedValueMbps;     /* speed value in Mbps */
    GT_BOOL                                 isExists;           /* flag that shows if port exists or not */
    CPSS_DXCH_DETAILED_PORT_MAP_STC       *portMapShadowPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC       *portMapShadowIteratorPtr;
    GT_PHYSICAL_PORT_NUM                    port;
    GT_U32                                  dpIndex;
    CPSS_PORT_SPEED_ENT                     currentPortSpeed;
    GT_BOOL                                 isPollingNeeded = GT_FALSE; /* flag that causes to all existing ports
                                                                           polling if overall bandwidth was increased */
    GT_U32                                  counterValue;
    GT_U32                                  timeout;

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    ifMode = ifMode;
    cpssOsMemSet(&portNewResource, 0, sizeof(portNewResource));
    cpssOsMemSet(&portCurrResource, 0, sizeof(portCurrResource));

    dpIndex = portMapShadowPtr->portMap.macNum >= PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS;

    rc = prvCpssDxChCaelumPortResourcesSpeedIndexGet(devNum,dpIndex,speed, &speedIndex, &speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMapShadowPtr->portMap.macNum) != CPSS_PORT_INTERFACE_MODE_NA_E) &&
       (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        isExists = GT_TRUE;
    }
    else
    {
        isExists = GT_FALSE;
    }

    /***************************************
        Resource calculation section:
    ****************************************/

    /* Calculate resources for the given port (new or already exists) */
    rc = prvCpssDxChCaelumPortResourcesCalculationDynamic(devNum, portNum, speed, isExists, &portNewResource);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (portMapShadowPtr->portMap.trafficManagerEn != GT_TRUE)
    {

        /* Recalculate all existing ports according to new overall bandwidth */
        for(port = 0; port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); port++)
        {
            /* Get Port Mapping DataBase */
            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, port, /*OUT*/&portMapShadowIteratorPtr);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            if(portMapShadowIteratorPtr->valid != GT_TRUE)
            {
                portResDb[port].valid = GT_FALSE;
                continue;
            }
            if(port == portNum)
            {
                portResDb[port].valid = GT_TRUE;
                portResDb[port].res = portNewResource;
                continue;
            }

            if(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMapShadowIteratorPtr->portMap.macNum) == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                portResDb[port].valid = GT_FALSE;
                continue;
            }

            /*
                   __ DP0 __                __ DP1 __
                  |         |              |         |
               TM_port  Non_TM_port     TM_port  Non_TM_port
                 (1)        (2)            (3)      (4)


                1. Add/remove port does not cause to resource reconfiguration.
                2. Add/remove port cause to DP0 non TM ports reconfiguration.
                3. Add/remove port does not cause to resource reconfiguration.
                4. Add/remove port cause to DP0 TM ports, DP1 TM and nonTM reconfiguration.

            */

            if((portMapShadowIteratorPtr->portMap.macNum < PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS) &&
               (portMapShadowIteratorPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
            {
                if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[0] ==
                   PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[0])
                {
                    if(portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_FALSE)
                    {
                        portResDb[port].valid = GT_FALSE;
                        continue;
                    }
                }
                else
                {
                    if(portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_TRUE)
                    {
                        portResDb[port].valid = GT_FALSE;
                        continue;
                    }
                }
            }
            else
            {
                if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] ==
                   PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1])
                {
                    portResDb[port].valid = GT_FALSE;
                    continue;
                }
            }

            currentPortSpeed = (port == CPSS_CPU_PORT_NUM_CNS) ? CPSS_PORT_SPEED_1000_E : PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMapShadowIteratorPtr->portMap.macNum);

            if(portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_FALSE)
            {
                rc = prvCpssDxChCaelumPortResourcesCalculationDynamic(devNum, port, currentPortSpeed, GT_TRUE, &(portResDb[port].res));
                if(rc != GT_OK)
                {
                    portResDb[port].valid = GT_FALSE;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                rc = prvCpssDxChCaelumPortResourcesCalculationDynamicTm(devNum,
                                                                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps,
                                                                        &(portResDb[port].res));
                if(rc != GT_OK)
                {
                    portResDb[port].valid = GT_FALSE;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            portResDb[port].valid = GT_TRUE;

        }

        /*********************************************************
            Limitation check section:
        *********************************************************/
       rc = prvCpssDxChCaelumPortResourcesLimitationCheck(devNum, portNum);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }


        /*********************************************************
            Configuration section(new and existing ports):
        *********************************************************/
        if(portMapShadowPtr->portMap.macNum < PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[0] >
               PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[0])
            {
                isPollingNeeded = GT_TRUE;
            }

            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[0] += speedValueMbps;
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[0] !=
               PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[0])
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "prvCpssDxChCaelumPortResourcesConfigDynamic: temporary and current overall speed are not the same for DP_0\n");
            }
            dpIndex = 0;
        }
        else
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1] >
               PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1])
            {
                isPollingNeeded = GT_TRUE;
            }

            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] += speedValueMbps;
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] !=
               PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1])
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "prvCpssDxChCaelumPortResourcesConfigDynamic: temporary and current overall speed are not the same for DP_1\n");
            }
            dpIndex = 1;
        }

        /*
            - configure all ports with new resource configurations
            - update DBs related to overall bandwidth and resource allocations
        */
        for(port = 0; (port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)) &&
                                        (port < CPSS_MAX_PORTS_NUM_CNS); port++)
        {
            if(portResDb[port].valid)
            {
                /* header and payload polling for existing ports, if overall bandwidth was increased */
                if((isPollingNeeded == GT_TRUE) && (port == portNum))
                {
                    continue;
                }

                /* if port was defined before it is needed to update global resource DB,
                  or if given port was exist and need to be reconfigured  */
                if((port != portNum) || (isExists == GT_TRUE))
                {
                    rc = prvCpssDxChCaelumPortResourceConfigGet(devNum, port, &portCurrResource);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }

                rc = prvCpssDxChCaelumPortResourceConfigSet(devNum, port, &(portResDb[port].res));
                if(GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                if((port != portNum) || (isExists == GT_TRUE))
                {
                    rc = prvCpssDxChPortResourcesConfigDbDelete(devNum, dpIndex,
                                                                portCurrResource.txdmaCreditValue,
                                                                portCurrResource.txdmaTxfifoHeaderCounterThresholdScdma,
                                                                portCurrResource.txdmaTxfifoPayloadCounterThresholdScdma);
                    if(GT_OK != rc)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }

                rc = prvCpssDxChPortResourcesConfigDbAdd(devNum, dpIndex,
                                                         portResDb[port].res.txdmaCreditValue,
                                                         portResDb[port].res.txdmaTxfifoHeaderCounterThresholdScdma,
                                                         portResDb[port].res.txdmaTxfifoPayloadCounterThresholdScdma);
                if(GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

            }
        }

        /************************************/
        /*      Existing ports polling      */
        /************************************/

        /* if port was defined before it is needed to update global resource DB,
          or if given port was exist and need to be reconfigured  */
        cpssOsBzero((GT_VOID*) &portCurrResource, sizeof(portCurrResource));
        if(isExists == GT_TRUE)
        {
            rc = prvCpssDxChCaelumPortResourceConfigGet(devNum, portNum, &portCurrResource);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        if(((PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedDescCredits[dpIndex]    - portCurrResource.txdmaCreditValue                        +  portResDb[portNum].res.txdmaCreditValue)                        <= PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.maxDescCredits[dpIndex]) &&
           ((PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedHeaderCredits[dpIndex]  - portCurrResource.txdmaTxfifoHeaderCounterThresholdScdma  +  portResDb[portNum].res.txdmaTxfifoHeaderCounterThresholdScdma)  <= PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.maxHeaderCredits[dpIndex]) &&
           ((PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedPayloadCredits[dpIndex] - portCurrResource.txdmaTxfifoPayloadCounterThresholdScdma +  portResDb[portNum].res.txdmaTxfifoPayloadCounterThresholdScdma) <= PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.maxPayloadCredits[dpIndex]))

        {
            isPollingNeeded = GT_FALSE;
        }

        if(isPollingNeeded == GT_TRUE)
        {
            for(port = 0; (port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)) &&
                                        (port < CPSS_MAX_PORTS_NUM_CNS); port++)
            {
                if(portResDb[port].valid)
                {
                    if(port == portNum)
                    {
                        continue;
                    }

                    for(timeout = 100; timeout > 0; timeout--)
                    {
                        rc = prvCpssDxChCaelumPortRegFieldGet(devNum, port,
                                                              PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E,
                                                              &counterValue);
                        if(rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }

#ifdef GM_USED
                        /* the registers not exists in GM ... and return '0xbadad' */
                        counterValue = 0;
#endif /*GM_USED*/
                        if(counterValue <= portResDb[port].res.txdmaTxfifoHeaderCounterThresholdScdma)
                            break;
                        else
                            cpssOsTimerWkAfter(10);
                    }

                    if(0 == timeout)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
                    }


                    for(timeout = 100; timeout > 0; timeout--)
                    {
                        rc = prvCpssDxChCaelumPortRegFieldGet(devNum, port,
                                                              PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E,
                                                              &counterValue);


#ifdef GM_USED
                        /* the registers not exists in GM ... and return '0xbadad' */
                        counterValue = 0;
#endif /*GM_USED*/
                        if(counterValue <= portResDb[port].res.txdmaTxfifoPayloadCounterThresholdScdma)
                            break;
                        else
                            cpssOsTimerWkAfter(10);
                    }

                    if(0 == timeout)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
                    }

                }
            }
        }

        rc = prvCpssDxChCaelumPortResourceConfigSet(devNum, portNum, &(portResDb[portNum].res));
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(isExists == GT_TRUE)
        {
                rc = prvCpssDxChPortResourcesConfigDbDelete(devNum, dpIndex,
                                                        portCurrResource.txdmaCreditValue,
                                                        portCurrResource.txdmaTxfifoHeaderCounterThresholdScdma,
                                                        portCurrResource.txdmaTxfifoPayloadCounterThresholdScdma);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

            rc = prvCpssDxChPortResourcesConfigDbAdd(devNum, dpIndex,
                                                 portResDb[portNum].res.txdmaCreditValue,
                                                 portResDb[portNum].res.txdmaTxfifoHeaderCounterThresholdScdma,
                                                 portResDb[portNum].res.txdmaTxfifoPayloadCounterThresholdScdma);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        rc = prvCpssDxChCaelumPortResourceConfigSet(devNum, portNum, &portNewResource);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}


/* just for test , manual configuration */
typedef struct
{
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PORT_SPEED_ENT             speed;
    GT_U32                          txqCredits;
}PORT_MANUAL_TXQ_CREDIT_STC;


PORT_MANUAL_TXQ_CREDIT_STC prv_txqPortManualCredit_BobK = { (GT_U32)(~0),(CPSS_PORT_SPEED_ENT)(-1), 0};

GT_STATUS prvCpssDxChBobK_Port_TxQManualCreditSet
(
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_U32                          txqCredits
)
{
    prv_txqPortManualCredit_BobK.portNum     = portNum;
    prv_txqPortManualCredit_BobK.speed       = speed;
    prv_txqPortManualCredit_BobK.txqCredits  = txqCredits;
    return GT_OK;
}


typedef struct
{
    CPSS_PORT_SPEED_ENT                      speed;
    PRV_CPSS_RXDMA_IfWidth_ENT               rxDmaIfWidth;
    PRV_CPSS_TxFIFO_OutGoungBusWidth_ENT     txFifoOutGoingBusWidth;
}PRV_CPSS_DXCH_BOBK_SPEED_2_RES_STC;


PRV_CPSS_DXCH_BOBK_SPEED_2_RES_STC prv_bobk_speed2resourceInitList[] =
{
      {    CPSS_PORT_SPEED_10_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E  }
     ,{   CPSS_PORT_SPEED_100_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E  }
     ,{  CPSS_PORT_SPEED_1000_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E  }
     ,{  CPSS_PORT_SPEED_2500_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_5000_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_10000_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_11800_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_20000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_40000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_47200_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_75000_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_100G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_102G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_107G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{    CPSS_PORT_SPEED_NA_E ,  PRV_CPSS_RXDMA_IfWidth_MAX_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_MAX_E }
};

PRV_CPSS_DXCH_BOBK_SPEED_2_RES_STC prv_aldrin_speed2resourceInitList[] =
{
      {    CPSS_PORT_SPEED_10_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E  }
     ,{   CPSS_PORT_SPEED_100_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E  }
     ,{  CPSS_PORT_SPEED_1000_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E  }
     ,{  CPSS_PORT_SPEED_2500_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_5000_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_10000_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_11800_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_12500_E ,   PRV_CPSS_RXDMA_IfWidth_64_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_20000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_23600_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_25000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_40000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_47200_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_50000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_75000_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_100G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_102G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_107G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{    CPSS_PORT_SPEED_NA_E ,  PRV_CPSS_RXDMA_IfWidth_MAX_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_MAX_E }
};


PRV_CPSS_DXCH_BOBK_SPEED_2_RES_STC prv_bc3_speed2resourceInitList[] =
{
      {    CPSS_PORT_SPEED_10_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{   CPSS_PORT_SPEED_100_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_1000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_2500_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_5000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_10000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_11800_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_12500_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_20000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_23600_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_25000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_26700_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_40000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_47200_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_50000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_52500_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_75000_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_100G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_102G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_107G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{    CPSS_PORT_SPEED_NA_E ,  PRV_CPSS_RXDMA_IfWidth_MAX_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_MAX_E }
};

PRV_CPSS_DXCH_BOBK_SPEED_2_RES_STC prv_aldrin2_speed2resourceInitList[] =
{
      {    CPSS_PORT_SPEED_10_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{   CPSS_PORT_SPEED_100_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_1000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_2500_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{  CPSS_PORT_SPEED_5000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_10000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_11800_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_12500_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_20000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_23600_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_25000_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_26700_E ,  PRV_CPSS_RXDMA_IfWidth_64_E  , PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E  }
     ,{ CPSS_PORT_SPEED_40000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_47200_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_50000_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_52500_E ,  PRV_CPSS_RXDMA_IfWidth_256_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_32B_E }
     ,{ CPSS_PORT_SPEED_75000_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_100G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_102G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{  CPSS_PORT_SPEED_107G_E ,  PRV_CPSS_RXDMA_IfWidth_512_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E }
     ,{    CPSS_PORT_SPEED_NA_E ,  PRV_CPSS_RXDMA_IfWidth_MAX_E , PRV_CPSS_TxFIFO_OutGoungBusWidth_MAX_E }
};


GT_STATUS prvCpssDxChCaelumPortResourcesConfigStaticBusWidthGet
(
    IN GT_U8   devNum,
    IN GT_U32  portNum,
    IN CPSS_PORT_SPEED_ENT speed,
    OUT GT_U32 *rxdmaScdmaIncomingBusWidthPtr,
    OUT GT_U32 *txfifoScdmaShiftersOutgoingBusWidthPtr
)
{
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portMapShadowPtr;
    PRV_CPSS_DXCH_BOBK_SPEED_2_RES_STC *listPtr;
    GT_U32 i;
    GT_STATUS rc;

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
    {
        listPtr = &prv_bobk_speed2resourceInitList[0];
    }
    else if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        listPtr = &prv_aldrin_speed2resourceInitList[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        listPtr = &prv_bc3_speed2resourceInitList[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        listPtr = &prv_aldrin2_speed2resourceInitList[0];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }


    for (i = 0 ; listPtr[i].speed != CPSS_PORT_SPEED_NA_E; i++)
    {
        if (listPtr[i].speed == speed)
        {
            *rxdmaScdmaIncomingBusWidthPtr          = listPtr[i].rxDmaIfWidth;
            *txfifoScdmaShiftersOutgoingBusWidthPtr = listPtr[i].txFifoOutGoingBusWidth;
            /*--------------------------------------------------------------------*
             *  correct for CPU txFifo-outgoing bus width DMA 72 has width 8Bytes *
             *--------------------------------------------------------------------*/
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                *txfifoScdmaShiftersOutgoingBusWidthPtr = PRV_CPSS_TxFIFO_OutGoungBusWidth_8B_E;
            }
            if(PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
            {
                if (portMapShadowPtr->portMap.macNum == 32 && speed == CPSS_PORT_SPEED_2500_E)
                {
                    *txfifoScdmaShiftersOutgoingBusWidthPtr = PRV_CPSS_TxFIFO_OutGoungBusWidth_1B_E;
                }
            }
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}


static GT_VOID prvCpssDxChPortResources_TXDMA_SCDMA_RateLimit
(
    IN  GT_U32      speedValueMbps,
    IN  GT_U32      coreClockInMHz,
    OUT GT_U32     *rateLimitIntegerPartPtr,
    OUT GT_U32     *rateLimitResidueVectorPtr
)
{
    static const GT_U32 enum_arr[] =
    {
        DIV_0_32,  DIV_1_32,  DIV_2_32,  DIV_3_32,  DIV_4_32,  DIV_5_32,  DIV_6_32,  DIV_7_32,
        DIV_8_32,  DIV_9_32,  DIV_10_32, DIV_11_32, DIV_12_32, DIV_13_32, DIV_14_32, DIV_15_32,
        DIV_16_32, DIV_17_32, DIV_18_32, DIV_19_32, DIV_20_32, DIV_21_32, DIV_22_32, DIV_23_32,
        DIV_24_32, DIV_25_32, DIV_26_32, DIV_27_32, DIV_28_32, DIV_29_32, DIV_30_32, DIV_31_32
    };
    GT_U32 rate_limit_mul32 =  ((32 * 128 * 8 * coreClockInMHz) / speedValueMbps);

    *rateLimitIntegerPartPtr = rate_limit_mul32 / 32;
    *rateLimitResidueVectorPtr = enum_arr[rate_limit_mul32 % 32];
}


/**
* @internal prvCpssDxChCaelumPortResourcesConfigStatic function
* @endinternal
*
* @brief   Cetus port resource static configuration
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourcesConfigStatic
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      txdmaCreditValue;   /* TXDMA descriptor credit number */
    GT_U32      txfifoHeaderCounterThresholdScdma;
    GT_U32      txfifoPayloadCounterThresholdScdma;
    GT_U32      txfifoScdmaPayloadThreshold;
    GT_U32      txfifoScdmaBurstFullThrshold;
    GT_U32      txfifoScdmaBurstAlmostFullThrshold;
    GT_U32      txdmaScdmaRateLimitIntegerThr;
    GT_U32      txdmaScdmaRateLimitResidueVectorThr;
    GT_U32      txfifoScdmaShiftersOutgoingBusWidth;
    GT_U32      rxdmaScdmaIncomingBusWidth;
    GT_U32      xlgMacTxReadThresh;
    GT_U32      speedValueMbps;
    GT_U32      speedIndex;         /* index in speed DB */
    GT_U32      coreClockIndex;     /* index in clock DB */
    GT_U32      speedHwIndex;       /* HW speed index */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32      dpIndexTxDma;
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC newTmResource;

    ifMode = ifMode;
    cpssOsMemSet(&newTmResource, 0, sizeof(PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC));

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* dpIndexTxDma = portMapShadowPtr->portMap.txDmaNum >= PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS; */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txDmaNum,/*OUT*/&dpIndexTxDma,NULL);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /* convert port speed to DB index and speed value in Mbps */
    rc = prvCpssDxChCaelumPortResourcesSpeedIndexGet(devNum,dpIndexTxDma,speed, &speedIndex, &speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* convert core clock to DB index */
    rc = prvCpssDxChCaelumPortResourcesClockIndexGet(devNum,PRV_CPSS_PP_MAC(devNum)->coreClock, &coreClockIndex);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChCaelumPortResourcesConfigStaticBusWidthGet(devNum,portNum,speed,/*OUT*/&rxdmaScdmaIncomingBusWidth,&txfifoScdmaShiftersOutgoingBusWidth);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    if(portMapShadowPtr->portMap.trafficManagerEn == GT_FALSE)
    {
        txdmaCreditValue                     = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        txfifoHeaderCounterThresholdScdma    = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        txfifoPayloadCounterThresholdScdma   = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        txfifoScdmaPayloadThreshold          = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        txfifoScdmaBurstFullThrshold         = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        txfifoScdmaBurstAlmostFullThrshold   = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        txdmaScdmaRateLimitIntegerThr        = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        txdmaScdmaRateLimitResidueVectorThr  = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        xlgMacTxReadThresh                   = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        /* GT_U32  dpIndex; */
        /* GT_U32  localDmaNum;*/
        if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
        {
            switch (dpIndexTxDma)
            {
                case 0:
                    txdmaCreditValue                   = prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditCore0Arr         [coreClockIndex][speedIndex];
                    txfifoHeaderCounterThresholdScdma  = prvCpssDxChBobkTxdmaScdmaTxFifoHeaderCreditThresholdCore0Arr [coreClockIndex][speedIndex];
                    txfifoPayloadCounterThresholdScdma = prvCpssDxChBobkTxdmaScdmaTxFifoPayloadCreditThresholdCore0Arr[coreClockIndex][speedIndex];
                    txfifoScdmaPayloadThreshold        = prvCpssDxChBobkTxFifoScdmaPayloadThresholdCore0Arr           [coreClockIndex][speedIndex];
                break;
                case 1:
                    txdmaCreditValue                   = prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditCore1Arr         [coreClockIndex][speedIndex];
                    txfifoHeaderCounterThresholdScdma  = prvCpssDxChBobkTxdmaScdmaTxFifoHeaderCreditThresholdCore1Arr [coreClockIndex][speedIndex];
                    txfifoPayloadCounterThresholdScdma = prvCpssDxChBobkTxdmaScdmaTxFifoPayloadCreditThresholdCore1Arr[coreClockIndex][speedIndex];
                    txfifoScdmaPayloadThreshold        = prvCpssDxChBobkTxFifoScdmaPayloadThresholdCore1Arr           [coreClockIndex][speedIndex];
                break;
                default:
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
            }
        }
        else  if(PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum)) /* in aldrin all cores are symmetrical, therefore single table is used */
        {
            txdmaCreditValue                   = Aldrin_TXDMA_SCDMA_TxQDescriptorCredit_Arr                   [coreClockIndex][speedIndex];
            txfifoHeaderCounterThresholdScdma  = Aldrin_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr           [coreClockIndex][speedIndex];
            txfifoPayloadCounterThresholdScdma = Aldrin_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr          [coreClockIndex][speedIndex];
            txfifoScdmaPayloadThreshold        = Aldrin_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr          [coreClockIndex][speedIndex];
        }
        /* in bobcat3 all cores are symmetrical, therefore single table is used */
        else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
           PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            txdmaCreditValue                     = BC3_TXDMA_SCDMA_TxQDescriptorCredit_Arr                   [coreClockIndex][speedIndex];
            txfifoHeaderCounterThresholdScdma    = BC3_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr           [coreClockIndex][speedIndex];
            txfifoPayloadCounterThresholdScdma   = BC3_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr          [coreClockIndex][speedIndex];
            txfifoScdmaPayloadThreshold          = BC3_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr          [coreClockIndex][speedIndex];
            txfifoScdmaBurstFullThrshold         = BC3_TXDMA_SCDMA_burstFullThreshold_Arr                    [coreClockIndex][speedIndex];
            txfifoScdmaBurstAlmostFullThrshold   = BC3_TXDMA_SCDMA_burstAlmostFullThreshold_Arr              [coreClockIndex][speedIndex];
            txdmaScdmaRateLimitIntegerThr        = BC3_TXDMA_SCDMA_RateLimitIntegerPart_Arr                  [coreClockIndex][speedIndex];
            txdmaScdmaRateLimitResidueVectorThr  = BC3_TXDMA_SCDMA_RateLimitResidueVector_Arr                [coreClockIndex][speedIndex];
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                if(PRV_CPSS_GE_PORT_GE_ONLY_E != PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portMapShadowPtr->portMap.macNum))
                {
                    xlgMacTxReadThresh            = BC3_XLG_MAC_TxReadThreshold_Arr [1][speedIndex];
                }
            }
        }
        else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            /* Cores are not symmetrical in Aldrin2 */
            switch (dpIndexTxDma)
            {
                case 0:
                case 1:
                    txdmaCreditValue                     = ARM2_dp01_TXDMA_SCDMA_TxQDescriptorCredit_Arr                   [coreClockIndex][speedIndex];
                    txfifoHeaderCounterThresholdScdma    = ARM2_dp01_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr           [coreClockIndex][speedIndex];
                    txfifoPayloadCounterThresholdScdma   = ARM2_dp01_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr          [coreClockIndex][speedIndex];
                    txfifoScdmaPayloadThreshold          = ARM2_dp01_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr          [coreClockIndex][speedIndex];
                    txfifoScdmaBurstFullThrshold         = ARM2_dp01_TXDMA_SCDMA_burstFullThreshold_Arr                    [coreClockIndex][speedIndex];
                    txfifoScdmaBurstAlmostFullThrshold   = ARM2_dp01_TXDMA_SCDMA_burstAlmostFullThreshold_Arr              [coreClockIndex][speedIndex];
                    break;
                case 2:
                case 3:
                    txdmaCreditValue                     = ARM2_dp23_TXDMA_SCDMA_TxQDescriptorCredit_Arr                   [coreClockIndex][speedIndex];
                    txfifoHeaderCounterThresholdScdma    = ARM2_dp23_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr           [coreClockIndex][speedIndex];
                    txfifoPayloadCounterThresholdScdma   = ARM2_dp23_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr          [coreClockIndex][speedIndex];
                    txfifoScdmaPayloadThreshold          = ARM2_dp23_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr          [coreClockIndex][speedIndex];
                    txfifoScdmaBurstFullThrshold         = ARM2_dp23_TXDMA_SCDMA_burstFullThreshold_Arr                    [coreClockIndex][speedIndex];
                    txfifoScdmaBurstAlmostFullThrshold   = ARM2_dp23_TXDMA_SCDMA_burstAlmostFullThreshold_Arr              [coreClockIndex][speedIndex];
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            prvCpssDxChPortResources_TXDMA_SCDMA_RateLimit(
                speedValueMbps,
                PRV_CPSS_PP_MAC(devNum)->coreClock,
                &txdmaScdmaRateLimitIntegerThr,
                &txdmaScdmaRateLimitResidueVectorThr);
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                if(PRV_CPSS_GE_PORT_GE_ONLY_E != PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portMapShadowPtr->portMap.macNum))
                {
                    xlgMacTxReadThresh            = BC3_XLG_MAC_TxReadThreshold_Arr [1][speedIndex];
                }
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if (prv_txqPortManualCredit_BobK.portNum == portNum && prv_txqPortManualCredit_BobK.speed == speed)
        {
            txdmaCreditValue = prv_txqPortManualCredit_BobK.txqCredits;
        }
    }
    else /* TM */
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"prvCpssDxChCaelumPortResourcesConfigStatic: please configure system TM bandwidth (portNum = %d)\n", portNum);
        }

        newTmResource.rxdmaScdmaIncomingBusWidth          = rxdmaScdmaIncomingBusWidth;
        newTmResource.ethTxfifoOutgoingBusWidth           = txfifoScdmaShiftersOutgoingBusWidth;
        newTmResource.ethTxfifoScdmaPayloadThreshold      = 0xA0;

        newTmResource.txdmaCreditValue                        = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        newTmResource.txdmaTxfifoHeaderCounterThresholdScdma  = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        newTmResource.txdmaTxfifoPayloadCounterThresholdScdma = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        newTmResource.txfifoScdmaPayloadStartTransmThreshold  = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        newTmResource.txfifoScdmaShiftersOutgoingBusWidth = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
        rc = prvCpssDxChCaelumPortResourceConfigSet(devNum, portNum, &newTmResource);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        return GT_OK;
    }

    rc = prvCpssDxChPortResourcesConfigDbAvailabilityCheck(devNum, dpIndexTxDma, txdmaCreditValue, txfifoHeaderCounterThresholdScdma, txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if ((systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E) ||
        (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        /*
            1. TXDMA desc credits - each DMA, according to its speed, requires a certain amount of descriptors
               allocated space inside the TXDMA to be able to support FWS (desc_credits_scdma%p).

            TXDMA 1
            Register:   <TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
            Fields:     BITS 0-8: Descriptor Credits SCDMA %p
            Limitation: SUM(DescriptorCreditsScdma_cc {PortSpeed,CoreFreq}) <= 144

        */

        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                              txdmaCreditValue);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E");
        }
        /*
            2. TXFIFO header and payload credits - each DMA, according to its speed,
               requires a certain amount of space allocated inside the TXDMA for both
               headers and payload to achieve FWS and avoid under-runs

            TXDMA 1
            Register:   <TXDMA1_IP> TxDMA IP Units/TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
            Fields:     BITS 0-9: TxFIFO Header Counter Threshold SCDMA %p
                        BITS 10-19: TxFIFO Payload Counter Threshold SCDMA %p
            Limitation: SUM(Header {PortSpeed,CoreFreq}) <= 363
                        SUM(Payload {PortSpeed,CoreFreq}) <= 447

        */
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
                                              txfifoHeaderCounterThresholdScdma);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E");
        }

        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
                                              txfifoPayloadCounterThresholdScdma);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E");
        }

        /*----------------------------------------------------------------*
         *  burst almost full threhold and burts full threshold           *
         *     TXDMA/Units/<TXDMA_IP> TxDMA %a/TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
                Fields:     BITS  0-15: TXDMA SCDMA burst full threhold %p
                            BITS 16-31: TXDMA SCDMA burst almost full threhold %p
         *----------------------------------------------------------------*/
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
           PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                                  PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E,
                                                  txfifoScdmaBurstFullThrshold);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E");
            }
            rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                                  PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E,
                                                  txfifoScdmaBurstAlmostFullThrshold);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E");
            }
            rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                                  PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E,
                                                  txdmaScdmaRateLimitIntegerThr);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E");
            }
            rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                                  PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E,
                                                  txdmaScdmaRateLimitResidueVectorThr);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E");
            }

            rc = prvCpssDxChBc3TxDmaSpeedHwIndexGet(
                devNum, speedValueMbps, &speedHwIndex);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc, "prvCpssDxChBc3TxDmaSpeedHwIndexGet");
            }

            /* for CutThrough configurations  src and dst DMA speed */
            rc = prvCpssDxChCaelumPortRegFieldSet(
                devNum, portNum,
                PRV_CAELUM_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E,
                speedHwIndex);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E");
            }

            rc = prvCpssDxChCaelumPortAllDataPathRegFieldSet(
                devNum, portNum,
                PRV_CAELUM_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E,
                speedHwIndex);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc, "prvCpssDxChCaelumPortAllDataPathRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E");
            }
            if (xlgMacTxReadThresh != PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS)
            {
                rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                                        PRV_CAELUM_REG_FIELDS_XLG_MAC_FIFO_TX_READ_THRESHOLD_E,xlgMacTxReadThresh);
                if(GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_XLG_MAC_FIFO_TX_READ_THRESHOLD_E");
                }
            }
        }

        /*
            3. Payload Threshold - to avoid under-runs due to variance in the
               latency of read responses from the MPPM - the TXFIFO is required
               to start transmitting the packet only after reaching a certain
               threshold. Each DMA speeds require a different threshold
               (scdma_%p_payload_threshold).
        */
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E,
                                              txfifoScdmaPayloadThreshold);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /*
            4. TXFIFO Outgoing Bus Width - Each transmit DMA speed requires
               a different interface width to the GOP (outgoing_bus_%p_width).

            TXDMA 1
            Register:   <TXFIFO1_IP> TxFIFO IP Units/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
            Fields:     BITS 0-2: outgoing_bus_%p_width

            5. RXDMA Incoming Bus Width - Each receive DMA speed requires a different interface width to the GOP.

            TXDMA 1
            Register:   RxDMA IP Units/Single Channel DMA Configurations/SCDMA %n Configuration 0/if_width<<%n>>
            Fields:     BITS 0-2: outgoing_bus_%p_width
        */

        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E,
                                              txfifoScdmaShiftersOutgoingBusWidth);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                              PRV_CAELUM_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E,
                                              rxdmaScdmaIncomingBusWidth);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }



    }

    /* update DB */
    rc = prvCpssDxChPortResourcesConfigDbCoreOverallSpeedAdd(devNum,dpIndexTxDma,speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortResourcesConfigDbAdd(devNum,dpIndexTxDma,txdmaCreditValue,txfifoHeaderCounterThresholdScdma,txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCaelumPortResourcesConfig function
* @endinternal
*
* @brief   Caelum port resource configuration
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortResourcesConfig
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   rc;                 /* return code */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_BOOL isSupported;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if((speed < CPSS_PORT_SPEED_10_E) || (speed >= CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* resources are computed on 1G base , so move the speed to 1G resolution */
    if (speed == CPSS_PORT_SPEED_10_E || speed == CPSS_PORT_SPEED_100_E)
    {
        speed = CPSS_PORT_SPEED_1000_E;
    }

    if (speed == CPSS_PORT_SPEED_29090_E)
    {
        speed = CPSS_PORT_SPEED_40000_E;
    }

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        rc = prvCpssCommonPortInterfaceSpeedGet(devNum,portMapShadowPtr->portMap.macNum,ifMode,speed,/*OUT*/&isSupported);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (isSupported == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }


    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        /* Cetus static configuration */
        rc = prvCpssDxChCaelumPortResourcesConfigStatic(devNum, portNum, ifMode, speed);
    }
    else
    {
        /* Caelum dynamic configuration */
        rc = prvCpssDxChCaelumPortResourcesConfigDynamic(devNum, portNum, ifMode, speed);
    }
    return rc;
}


/**
* @internal PRV_CPSS_DP_RESOURCE_LIMIT_STC function
* @endinternal
*
* @brief   Initialize data structure for port resource allocation
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
*/
typedef struct
{
    GT_U32 dpIdx;
    GT_U32 txqCredits;
    GT_U32 txFifoHeaders;
    GT_U32 txFifoPayloads;
}PRV_CPSS_DP_RESOURCE_LIMIT_STC;

#define BAD_VALUE (GT_U32)(~0)


PRV_CPSS_DP_RESOURCE_LIMIT_STC bobk_cetus_dp_res[] =
{
     {         0,                                        0,                                                  0,                                                  0 }
    ,{         1,  PRV_CPSS_DXCH_CETUS_DP1_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_CETUS_DP1_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_CETUS_DP1_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{ BAD_VALUE,                                BAD_VALUE,                                          BAD_VALUE,                                          BAD_VALUE }
};


PRV_CPSS_DP_RESOURCE_LIMIT_STC bobk_caelum_dp_res[] =
{
     {         0,  PRV_CPSS_DXCH_CAELUM_DP0_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_CAELUM_DP0_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_CAELUM_DP0_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         1,  PRV_CPSS_DXCH_CAELUM_DP1_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_CAELUM_DP1_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_CAELUM_DP1_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{ BAD_VALUE,                                 BAD_VALUE,                                           BAD_VALUE,                                           BAD_VALUE }
};

PRV_CPSS_DP_RESOURCE_LIMIT_STC aldrin_dp_res[] =
{
     {         0,  PRV_CPSS_DXCH_ALDRIN_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_ALDRIN_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_ALDRIN_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         1,  PRV_CPSS_DXCH_ALDRIN_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_ALDRIN_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_ALDRIN_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         2,  PRV_CPSS_DXCH_ALDRIN_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_ALDRIN_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_ALDRIN_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{ BAD_VALUE,                                BAD_VALUE,                                          BAD_VALUE,                                          BAD_VALUE }
};

PRV_CPSS_DP_RESOURCE_LIMIT_STC bobcat3_dp_res[] =
{
     {         0,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         1,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         2,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         3,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         4,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         5,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{ BAD_VALUE,                                 BAD_VALUE,                                           BAD_VALUE,                                          BAD_VALUE }
};

static PRV_CPSS_DP_RESOURCE_LIMIT_STC aldrin2_dp_res[] =
{
     {         0,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         1,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         2,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{         3,  PRV_CPSS_DXCH_BOBCAT3_DP_TXQ_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_HEADERS_CREDITS_CNS, PRV_CPSS_DXCH_BOBCAT3_DP_TXFIFO_PAYLOAD_CREDITS_CNS }
    ,{ BAD_VALUE,                                 BAD_VALUE,                                           BAD_VALUE,                                          BAD_VALUE }
};

GT_STATUS prvCpssDxChFalconPortResourcesInit
(
    IN    GT_U8                   devNum
)
{

   PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
   return GT_OK;
}



GT_STATUS prvCpssDxChCaelumPortResourcesInit
(
    IN    GT_U8                   devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DP_RESOURCE_LIMIT_STC *listPtr;
    GT_U32 i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* prvCpssDxChBcat2Resources[CPSS_PORT_SPEED_NA_E][PRV_BC2_CORE_CLOCK_MAX_IDX_E] =  */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                                  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
                                                  CPSS_XCAT3_E | CPSS_LION_E | CPSS_LION2_E |CPSS_XCAT2_E);

    cpssOsBzero((GT_VOID*)portResDb, CPSS_MAX_PORTS_NUM_CNS * sizeof(portResDb[0]));

    /*---------------------------------*/
    /* select list according to device */
    /*---------------------------------*/
    if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
    {
        listPtr = &bobk_caelum_dp_res[0];
    }
    else if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
    {
        listPtr = &bobk_cetus_dp_res[0];
    }
    else if(PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        listPtr = &aldrin_dp_res[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        listPtr = &bobcat3_dp_res[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        listPtr = &aldrin2_dp_res[0];
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* currently Falcon as BC3 */
        CPSS_TBD_BOOKMARK_FALCON
        listPtr = &bobcat3_dp_res[0];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; listPtr[i].dpIdx != BAD_VALUE; i++)
    {
        rc = prvCpssDxChPortResourcesConfigDbInit(devNum,
                                                    listPtr[i].dpIdx,
                                                    listPtr[i].txqCredits,
                                                    listPtr[i].txFifoHeaders,
                                                    listPtr[i].txFifoPayloads);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "DP %d credits threshold config",listPtr[i].dpIdx);
        }
    }

    return GT_OK;
}



/**
* @internal prvCpssDxChCaelumPortResourcesReleaseStatic function
* @endinternal
*
* @brief   Remove static resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourcesReleaseStatic
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      txdmaCreditValue;
    GT_U32      speedIndex;
    GT_U32      speedValueMbps;
    GT_U32      txfifoHeaderCounterThresholdScdma;
    GT_U32      txfifoPayloadCounterThresholdScdma;
    GT_U32      dpIndexTxDma;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    CPSS_PORT_SPEED_ENT speed;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    /* dpIndexTxDma = portMapShadowPtr->portMap.txDmaNum >= PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS; */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txDmaNum,/*OUT*/&dpIndexTxDma,NULL);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    if(portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
    {
        /* for TM mapped ports there is no need to release resources because it is defined
           via global system resource bandwidth API - cpssDxChPortResourceTmBandwidthSet */
        return GT_OK;
    }

    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                          &txdmaCreditValue);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
                                          &txfifoHeaderCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                         PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
                                         &txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /*
        1. In the TXQ - Reset the desc credits counters of the port (Credit_Counters_Reset_%n).
           (not self cleared - after reset should be restored to normal mode)
    */
    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum) &&
        portMapShadowPtr->portMap.txqNum != GT_NA) /* extended-cascade port has no TXQ , therefore no credit counter reset */
    {
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum, portNum,
                                              PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
                                              1);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }


        /* Not self cleared - should be restored to 0 */
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum, portNum,
                                              PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
                                              0);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    /*
        2. In the TXDMA - configure TXQ descriptor credits threshold to 0 to avoid
           sending credits to the TXQ (desc_credits_scdma%p).
    */

    rc = prvCpssDxChCaelumPortRegFieldSet(devNum, portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                          0);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*
        3. Reset the TXDMA scdma desc credit counter (reset_desc_credits_counter_scdma%p).
           (not self cleared - after reset should be restored to normal mode)
    */
    rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
                                          1);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Not self cleared - should be restored to 0 */
    rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
                                          0);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /* update DB (DP_1) */
    /* convert port speed to DB index and speed value in Mbps */
    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMapShadowPtr->portMap.macNum);
    if (speed == CPSS_PORT_SPEED_29090_E)
    {
        speed = CPSS_PORT_SPEED_40000_E;
    }
    rc = prvCpssDxChCaelumPortResourcesSpeedIndexGet(devNum,dpIndexTxDma, PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMapShadowPtr->portMap.macNum), &speedIndex, &speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssDxChPortResourcesConfigDbCoreOverallSpeedDelete(devNum,dpIndexTxDma,speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortResourcesConfigDbDelete(devNum,dpIndexTxDma,txdmaCreditValue,txfifoHeaderCounterThresholdScdma,txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return rc;
}


/**
* @internal prvCpssDxChCaelumPortResourcesReleaseDynamic function
* @endinternal
*
* @brief   Remove dynamic resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCaelumPortResourcesReleaseDynamic
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      txdmaCreditValue;
    GT_U32      txfifoHeaderCounterThresholdScdma;
    GT_U32      txfifoPayloadCounterThresholdScdma;
    GT_U32      dpIndexTxDma;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32      speedValueMbps;
    GT_U32      speedIndex;
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC  portCurrResource;   /* current port resource */
    CPSS_DXCH_DETAILED_PORT_MAP_STC       *portMapShadowIteratorPtr;
    GT_PHYSICAL_PORT_NUM                    port;
    GT_U32                                  dpIndex;
    CPSS_PORT_SPEED_ENT                     currentPortSpeed;

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    dpIndexTxDma = portMapShadowPtr->portMap.txDmaNum >= PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS;

    if(portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
    {
        /* for TM mapped ports there is no need to release resources because it is defined
           via global system resource bandwidth API - cpssDxChPortResourceTmBandwidthSet */
        return GT_OK;
    }

    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                          &txdmaCreditValue);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
                                          &txfifoHeaderCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChCaelumPortRegFieldGet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
                                          &txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*
        1. In the TXQ - Reset the desc credits counters of the port (Credit_Counters_Reset_%n).
           (not self cleared - after reset should be restored to normal mode)
    */
    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum) &&
        portMapShadowPtr->portMap.txqNum != GT_NA) /* extended-cascade port has no TXQ , therefore no credit counter reset */
    {
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum, portNum,
                                              PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
                                              1);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* Not self cleared - should be restored to 0 */
        rc = prvCpssDxChCaelumPortRegFieldSet(devNum, portNum,
                                              PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
                                              0);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /*
        2. In the TXDMA - configure TXQ descriptor credits threshold to 0 to avoid
           sending credits to the TXQ (desc_credits_scdma%p).
    */

    rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                          0);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*
        3. Reset the TXDMA scdma desc credit counter (reset_desc_credits_counter_scdma%p).
           (not self cleared - after reset should be restored to normal mode)
    */
    rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
                                          1);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Not self cleared - should be restored to 0 */
    rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
                                          0);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /******************************************************************************************
        Recalculate port resources for all existing ports in current DP
    ******************************************************************************************/

    cpssOsMemSet(&portCurrResource, 0, sizeof(portCurrResource));

    rc = prvCpssDxChCaelumPortResourcesSpeedIndexGet(devNum,dpIndexTxDma,PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMapShadowPtr->portMap.macNum), &speedIndex, &speedValueMbps);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* skip the second core that should not be recalculated */
    if((portMapShadowPtr->portMap.macNum < PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS) &&
       (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[0] -= speedValueMbps;
        dpIndex = 0;
    }
    else
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1] -= speedValueMbps;
        dpIndex = 1;
    }

    rc = prvCpssDxChPortResourcesConfigDbDelete(devNum,dpIndexTxDma,txdmaCreditValue,txfifoHeaderCounterThresholdScdma,txfifoPayloadCounterThresholdScdma);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /* Recalculate all existing ports according to new overall bandwidth */
    for(port = 0; port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); port++)
    {
        /* Get Port Mapping DataBase */
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, port, /*OUT*/&portMapShadowIteratorPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if(portMapShadowIteratorPtr->valid != GT_TRUE)
        {
            portResDb[port].valid = GT_FALSE;
            continue;
        }
        if(port == portNum)
        {
            portResDb[port].valid = GT_FALSE;
            continue;
        }

        if(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMapShadowIteratorPtr->portMap.macNum) == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            portResDb[port].valid = GT_FALSE;
            continue;
        }

        /*
               __ DP0 __                __ DP1 __
              |         |              |         |
           TM_port  Non_TM_port     TM_port  Non_TM_port
             (1)        (2)            (3)      (4)


            1. Add/remove port does not cause to resource reconfiguration.
            2. Add/remove port cause to DP0 non TM ports reconfiguration.
            3. Add/remove port does not cause to resource reconfiguration.
            4. Add/remove port cause to DP0 TM ports, DP1 TM and nonTM reconfiguration.

        */
        if((portMapShadowIteratorPtr->portMap.macNum < PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS) &&
           (portMapShadowIteratorPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[0] ==
               PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[0])
            {
                if(portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_FALSE)
                {
                    portResDb[port].valid = GT_FALSE;
                    continue;
                }
            }
            else
            {
                if(portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_TRUE)
                {
                    portResDb[port].valid = GT_FALSE;
                    continue;
                }
            }
        }
        else
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] ==
               PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1])
            {
                portResDb[port].valid = GT_FALSE;
                continue;
            }
        }



        currentPortSpeed = (port == CPSS_CPU_PORT_NUM_CNS) ? CPSS_PORT_SPEED_1000_E : PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMapShadowIteratorPtr->portMap.macNum);

        if(portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_FALSE)
        {
            rc = prvCpssDxChCaelumPortResourcesCalculationDynamic(devNum, port, currentPortSpeed, GT_TRUE, &(portResDb[port].res));
            if(rc != GT_OK)
            {
                portResDb[port].valid = GT_FALSE;
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            rc = prvCpssDxChCaelumPortResourcesCalculationDynamicTm(devNum,
                                                                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps,
                                                                    &(portResDb[port].res));
            if(rc != GT_OK)
            {
                portResDb[port].valid = GT_FALSE;
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        portResDb[port].valid = GT_TRUE;
    }

    /*
        - configure all ports with new resource configurations
        - update DBs related to overall bandwidth and resource allocations
    */
    for(port = 0; (port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)) &&
                                        (port < CPSS_MAX_PORTS_NUM_CNS); port++)
    {
        if(portResDb[port].valid)
        {
            rc = prvCpssDxChCaelumPortResourceConfigGet(devNum, port, &portCurrResource);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChCaelumPortResourceConfigSet(devNum, port, &(portResDb[port].res));
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChPortResourcesConfigDbDelete(devNum, dpIndex,
                                                        portCurrResource.txdmaCreditValue,
                                                        portCurrResource.txdmaTxfifoHeaderCounterThresholdScdma,
                                                        portCurrResource.txdmaTxfifoPayloadCounterThresholdScdma);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChPortResourcesConfigDbAdd(devNum, dpIndex,
                                                     portResDb[port].res.txdmaCreditValue,
                                                     portResDb[port].res.txdmaTxfifoHeaderCounterThresholdScdma,
                                                     portResDb[port].res.txdmaTxfifoPayloadCounterThresholdScdma);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    if((portMapShadowPtr->portMap.macNum < PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS) &&
       (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[0] -= speedValueMbps;
    }
    else
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] -= speedValueMbps;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChCaelumPortResourcesRelease function
* @endinternal
*
* @brief   Remove resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortResourcesRelease
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum
)
{
    GT_STATUS   rc;                 /* return code */

    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        /* Cetus static release configuration */
        rc = prvCpssDxChCaelumPortResourcesReleaseStatic(devNum, portNum);
    }
    else
    {
        /* Caelum dynamic release configuration */
        rc = prvCpssDxChCaelumPortResourcesReleaseDynamic(devNum, portNum);
    }

    return rc;
}


/**
* @internal prvCpssDxChCaelumPortResourceStaticTmSet function
* @endinternal
*
* @brief   Caelum TM port resource configuration
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tmBandwidthMbps          - system TM bandwidth
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note:
*       - API should not be called under traffic
*       - zero bandwidth will release resources of TM for non TM ports
*
*/
GT_STATUS prvCpssDxChCaelumPortResourceStaticTmSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps
)
{
    GT_STATUS                                   rc;                         /* return code */
    GT_U32                                      coreClockIndex;             /* index in clock DB */
    GT_U32                                      maxTmBw;                    /* maximal TM BW */
    GT_U32                                      dpIndexTxDma;               /* DP number */
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC      tmResource;                 /* resource structure */
    CPSS_DXCH_DETAILED_PORT_MAP_STC           *portMapShadowPtr = NULL;   /* pointer to mapping database */
    GT_PHYSICAL_PORT_NUM                        portNum;                    /* port number loop iterator */
    GT_U32                                      currBandwidth;              /* current TM bandwidth */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /*------------------------------------------------------------*
     * get maximal TM bw capacity depended on clock (HW constain) *
     *------------------------------------------------------------*/
    rc = prvCpssDxChCaelumPortResourcesClockIndexGet(devNum,PRV_CPSS_PP_MAC(devNum)->coreClock, &coreClockIndex);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourceTmSet: error in prvCpssDxChCaelumPortResourcesClockIndexGet\n");
    }

    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
    {
        /*CETUS device*/
        maxTmBw = cetusTmBw[coreClockIndex][1];
    }
    else
    {
        /*CAELUM device*/
        maxTmBw = caelumTmBw[coreClockIndex][1];
    }
    if(tmBandwidthMbps > maxTmBw)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"prvCpssDxChCaelumPortResourceTmSet: out of range: tmBandwidthMbps = %d\n", tmBandwidthMbps);
    }

    /* if bandwidth equals 0 - release all TM resources */
    cpssOsMemSet(&tmResource, 0, sizeof(PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC));
    if(tmBandwidthMbps != 0)
    {
        tmResource.txdmaCreditValue                        = prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditTmArr[coreClockIndex][tmBandwidthMbps / 1000];
        tmResource.txdmaTxfifoHeaderCounterThresholdScdma  = prvCpssDxChBobkTxdmaScdmaTxFifoHeaderCreditThresholdTmArr[coreClockIndex][tmBandwidthMbps / 1000];
        tmResource.txdmaTxfifoPayloadCounterThresholdScdma = prvCpssDxChBobkTxdmaScdmaTxFifoPayloadCreditThresholdTmArr[coreClockIndex][tmBandwidthMbps / 1000];
        tmResource.txfifoScdmaPayloadStartTransmThreshold  = prvCpssDxChBobkTxFifoScdmaPayloadThresholdTmArr[coreClockIndex][tmBandwidthMbps / 1000];
        tmResource.txfifoScdmaShiftersOutgoingBusWidth = PRV_CPSS_TxFIFO_OutGoungBusWidth_64B_E; /*Tx FIFO TM port (73) is always use 512 bit width, static*/
    }

    /*-----------------------------------------------------------*
     * Get any TM mapped port, whether it is initialized or not,  *
     * BUT                                                       *
     *    do not configure egress port related configurations    *
     *-----------------------------------------------------------*/
    rc = prvCpssDxChCaelumTmAnyPortGet(devNum,/*OUT*/&portNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourceTmSet: error in prvCpssDxChCaelumTmAnyPortGet() : no TM mapped ports");
    }

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* do not configure egress port related configurations */
    tmResource.rxdmaScdmaIncomingBusWidth      = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    tmResource.ethTxfifoOutgoingBusWidth       = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    tmResource.ethTxfifoScdmaPayloadThreshold  = PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS;
    rc = prvCpssDxChCaelumPortResourceConfigSet(devNum, portNum, &tmResource);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourceTmSet: error in prvCpssDxChCaelumTmPortResourceConfigSet");
    }

    dpIndexTxDma = portMapShadowPtr->portMap.txDmaNum >= PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps != 0)
    {
        currBandwidth = PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps;

        rc = prvCpssDxChPortResourcesConfigDbDelete(devNum,
                                                    dpIndexTxDma,
                                                    prvCpssDxChBobkTxdmaScdmaTxQDescriptorCreditTmArr[coreClockIndex][currBandwidth / 1000],
                                                    prvCpssDxChBobkTxdmaScdmaTxFifoHeaderCreditThresholdTmArr[coreClockIndex][currBandwidth / 1000],
                                                    prvCpssDxChBobkTxdmaScdmaTxFifoPayloadCreditThresholdTmArr[coreClockIndex][currBandwidth / 1000]);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* Update resource DB with new TM BW */
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps = tmBandwidthMbps;
    rc = prvCpssDxChPortResourcesConfigDbAdd(devNum, dpIndexTxDma, tmResource.txdmaCreditValue, tmResource.txdmaTxfifoHeaderCounterThresholdScdma, tmResource.txdmaTxfifoPayloadCounterThresholdScdma);

    return rc;
}

/**
* @internal prvCpssDxChCaelumPortResourceDynamicTmSet function
* @endinternal
*
* @brief   Caelum TM port resource configuration
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tmBandwidthMbps          - system TM bandwidth
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note:
*       - API should not be called under traffic
*       - zero bandwidth will release resources of TM for non TM ports
*
*/
GT_STATUS prvCpssDxChCaelumPortResourceDynamicTmSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps
)
{
    GT_STATUS                               rc;                             /* return code */
    GT_U32                                  coreClockIndex;                 /* index in clock DB */
    GT_U32                                  maxTmBw;                        /* maximal TM BW */
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC  tmResource;                     /* resource structure */
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC  portCurrResource;               /* current port resource */
    CPSS_DXCH_DETAILED_PORT_MAP_STC       *portMapShadowPtr = NULL;       /* pointer to mapping database */
    CPSS_DXCH_DETAILED_PORT_MAP_STC       *portMapShadowIteratorPtr = NULL; /* pointer to mapping database */
    GT_PHYSICAL_PORT_NUM                    portNum;                        /* port number loop iterator */
    GT_PHYSICAL_PORT_NUM                    port;                           /* loop iterator */
    CPSS_PORT_SPEED_ENT                     currentPortSpeed;               /* port speed */
    GT_BOOL                                 isPollingNeeded = GT_FALSE;     /* flag that causes to all existing ports
                                                                               polling if overall bandwidth was increased */
    GT_U32                                  counterValue;
    GT_U32                                  timeout;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /*------------------------------------------------------------*
     * get maximal TM bw capacity depended on clock (HW constain) *
     *------------------------------------------------------------*/
    rc = prvCpssDxChCaelumPortResourcesClockIndexGet(devNum,PRV_CPSS_PP_MAC(devNum)->coreClock, &coreClockIndex);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourceDynamicTmSet: error in prvCpssDxChCaelumPortResourcesClockIndexGet\n");
    }

    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
    {
        /*CETUS device*/
        maxTmBw = cetusTmBw[coreClockIndex][1];
    }
    else
    {
        /*CAELUM device*/
        maxTmBw = caelumTmBw[coreClockIndex][1];
    }
    if(tmBandwidthMbps > maxTmBw)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"prvCpssDxChCaelumPortResourceDynamicTmSet: out of range: tmBandwidthMbps = %d\n", tmBandwidthMbps);
    }


    /*-----------------------------------------------------------*
     * Get any TM mapped port, whether it is initialized or not,  *
     * BUT                                                       *
     *    do not configure egress port related configurations    *
     *-----------------------------------------------------------*/
    rc = prvCpssDxChCaelumTmAnyPortGet(devNum,/*OUT*/&portNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourceDynamicTmSet: error in prvCpssDxChCaelumTmAnyPortGet() : no TM mapped ports");
    }

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&tmResource, 0, sizeof(PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC));
    if(tmBandwidthMbps != 0)
    {
        rc = prvCpssDxChCaelumPortResourcesCalculationDynamicTm(devNum, tmBandwidthMbps, &tmResource);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(tmBandwidthMbps > PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps)
        {
            isPollingNeeded = GT_TRUE;
        }
    }
    else
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1] -= PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps;
    }

    /* Recalculate all existing ports according to new overall bandwidth */
    for(port = 0; port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); port++)
    {
        /* Get Port Mapping DataBase */
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, port, /*OUT*/&portMapShadowIteratorPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if(portMapShadowIteratorPtr->valid != GT_TRUE)
        {
            portResDb[port].valid = GT_FALSE;
            continue;
        }

        if(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMapShadowIteratorPtr->portMap.macNum) == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            if((port == portNum) && (portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_TRUE))
            {
                portResDb[port].valid = GT_TRUE;
                portResDb[port].res = tmResource;
                continue;
            }
            portResDb[port].valid = GT_FALSE;
            continue;
        }

        if(port == portNum)
        {
            portResDb[port].valid = GT_TRUE;
            portResDb[port].res = tmResource;
            continue;
        }

        /* it is not needed to reconfig existing TM ports */
        if(portMapShadowIteratorPtr->portMap.trafficManagerEn == GT_TRUE)
        {
            portResDb[port].valid = GT_FALSE;
            continue;
        }

        /* skip the second core that should not be recalculated */
        if((portMapShadowIteratorPtr->portMap.macNum < PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS) &&
           (portMapShadowIteratorPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
        {
            portResDb[port].valid = GT_FALSE;
            continue;
        }

        currentPortSpeed = (port == CPSS_CPU_PORT_NUM_CNS) ? CPSS_PORT_SPEED_1000_E : PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMapShadowIteratorPtr->portMap.macNum);

        rc = prvCpssDxChCaelumPortResourcesCalculationDynamic(devNum, port, currentPortSpeed, GT_TRUE, &(portResDb[port].res));
        if(rc != GT_OK)
        {
            portResDb[port].valid = GT_FALSE;
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        portResDb[port].valid = GT_TRUE;
    }


    /*********************************************************
        Limitation check section:
    *********************************************************/
    rc = prvCpssDxChCaelumPortResourcesLimitationCheck(devNum, portNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /*********************************************************
        Configuration section(new and existing ports):
    *********************************************************/
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] -= PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps;
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] += tmBandwidthMbps;
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[1] !=
       PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[1])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "prvCpssDxChCaelumPortResourceDynamicTmSet: temporary and current overall speed are not the same for DP_1\n");
    }


    /*
        - configure all ports with new resource configurations
        - update DBs related to overall bandwidth and resource allocations
    */
    for(port = 0; (port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)) &&
                                        (port < CPSS_MAX_PORTS_NUM_CNS); port++)
    {
        /* if port was defined before it is needed to update global resource DB,
          or if given port was exist and need to be reconfigured  */
        if((portResDb[port].valid) && (port != portNum))
        {
            rc = prvCpssDxChCaelumPortResourceConfigGet(devNum, port, &portCurrResource);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChCaelumPortResourceConfigSet(devNum, port, &(portResDb[port].res));
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChPortResourcesConfigDbDelete(devNum, 1,
                                                        portCurrResource.txdmaCreditValue,
                                                        portCurrResource.txdmaTxfifoHeaderCounterThresholdScdma,
                                                        portCurrResource.txdmaTxfifoPayloadCounterThresholdScdma);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChPortResourcesConfigDbAdd(devNum, 1,
                                                     portResDb[port].res.txdmaCreditValue,
                                                     portResDb[port].res.txdmaTxfifoHeaderCounterThresholdScdma,
                                                     portResDb[port].res.txdmaTxfifoPayloadCounterThresholdScdma);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

        }
    }

    /* need to wait will all existing port resources will be less or equal to the new thresholds,
       and only then configure TM port */
    if(isPollingNeeded == GT_TRUE)
    {
        for(port = 0; (port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)) &&
                                        (port < CPSS_MAX_PORTS_NUM_CNS); port++)
        {
            if(portResDb[port].valid)
            {
                if(port == portNum)
                {
                    continue;
                }

                for(timeout = 100; timeout > 0; timeout--)
                {
                    rc = prvCpssDxChCaelumPortRegFieldGet(devNum, port,
                                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E,
                                                          &counterValue);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }

#ifdef GM_USED
                    /* the registers not exists in GM ... and return '0xbadad' */
                    counterValue = 0;
#endif /*GM_USED*/
                    if(counterValue <= portResDb[port].res.txdmaTxfifoHeaderCounterThresholdScdma) break;
                    else cpssOsTimerWkAfter(10);
                }

                if(0 == timeout)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
                }


                for(timeout = 100; timeout > 0; timeout--)
                {
                    rc = prvCpssDxChCaelumPortRegFieldGet(devNum, port,
                                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E,
                                                          &counterValue);


#ifdef GM_USED
                    /* the registers not exists in GM ... and return '0xbadad' */
                    counterValue = 0;
#endif /*GM_USED*/
                    if(counterValue <= portResDb[port].res.txdmaTxfifoPayloadCounterThresholdScdma) break;
                    else cpssOsTimerWkAfter(10);
                }

                if(0 == timeout)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
                }

            }
        }
    }

    /* TM port resource configuration */
    rc = prvCpssDxChCaelumPortResourceConfigGet(devNum, portNum, &portCurrResource);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChCaelumPortResourceConfigSet(devNum, portNum, &tmResource);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourceTmSet: error in prvCpssDxChCaelumTmPortResourceConfigSet");
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps != 0)
    {
        rc = prvCpssDxChPortResourcesConfigDbDelete(devNum, 1,
                                                    portCurrResource.txdmaCreditValue,
                                                    portCurrResource.txdmaTxfifoHeaderCounterThresholdScdma,
                                                    portCurrResource.txdmaTxfifoPayloadCounterThresholdScdma);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* Update resource DB with new TM BW */
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps = tmBandwidthMbps;
    rc = prvCpssDxChPortResourcesConfigDbAdd(devNum, 1, tmResource.txdmaCreditValue, tmResource.txdmaTxfifoHeaderCounterThresholdScdma, tmResource.txdmaTxfifoPayloadCounterThresholdScdma);

    return rc;
}


/**
* @internal prvCpssDxChCaelumPortResourceTmGet function
* @endinternal
*
* @brief   Get Caelum TM port bandwidth
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] tmBandwidthMbpsPtr       - (pointer to) system TM bandwidth
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChCaelumPortResourceTmGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *tmBandwidthMbpsPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(tmBandwidthMbpsPtr);

    *tmBandwidthMbpsPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.trafficManagerCumBWMbps;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortResourcesRateLimitSet function
* @endinternal
*
* @brief    Jumbo with loopback issue:
*           Due to the loopback in the MAC, the S/D-PCS is no
*           longer limiting the rate of the port to exactly
*           10Gbps. The port speed is determined by the
*           MAC_CLK/Divider (in this case 833Mhz/4 * 64 =
*           13.33Gbps). Because the rate limiter of the loopback
*           port was enabled for exactly 10Gbps, the port got an
*           underrun (when rate limiter was disabled or shaped
*           to port speed of 13.33, there are no more issue).
*           Solution: As long as there is no Cut-Through traffic
*           towards the loopback port - disable the loopback
*           port rate limiter By writing 0x2 into TxDMA %a/TxDMA
*           Per SCDMA Configurations/SCDMA %p Configurations Reg
*           1/<SCDMA %p Rlimit Integer Thr>
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; ;
*         Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port interface speed
* @param[in] enable                   - enable rate limit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortResourcesRateLimitSet
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_SPEED_ENT             speed,
    IN GT_BOOL                         enable
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      txdmaScdmaRateLimitIntegerThr;
    GT_U32      txdmaScdmaRateLimitResidueVectorThr;
    GT_U32      speedValueMbps;
    GT_U32      speedIndex;         /* index in speed DB */
    GT_U32      coreClockIndex;     /* index in clock DB */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32      dpIndexTxDma;

    if((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) || (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (enable)
    {
        /* dpIndexTxDma = portMapShadowPtr->portMap.txDmaNum >= PRV_CPSS_DXCH_BOBK_DP1_FIRST_PORT_NUM_CNS; */
        rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMapShadowPtr->portMap.txDmaNum,/*OUT*/&dpIndexTxDma,NULL);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* convert port speed to DB index and speed value in Mbps */
        rc = prvCpssDxChCaelumPortResourcesSpeedIndexGet(devNum,dpIndexTxDma,speed, &speedIndex, &speedValueMbps);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* convert core clock to DB index */
        rc = prvCpssDxChCaelumPortResourcesClockIndexGet(devNum,PRV_CPSS_PP_MAC(devNum)->coreClock, &coreClockIndex);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            txdmaScdmaRateLimitIntegerThr        = BC3_TXDMA_SCDMA_RateLimitIntegerPart_Arr[coreClockIndex][speedIndex];
        }
        else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            prvCpssDxChPortResources_TXDMA_SCDMA_RateLimit(
                speedValueMbps,
                PRV_CPSS_PP_MAC(devNum)->coreClock,
                &txdmaScdmaRateLimitIntegerThr,
                &txdmaScdmaRateLimitResidueVectorThr);
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
    else
    {   /* to disable rate limit per port we set value 2*/
        txdmaScdmaRateLimitIntegerThr = 2;
    }

    rc = prvCpssDxChCaelumPortRegFieldSet(devNum,portNum,
                                          PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E,
                                          txdmaScdmaRateLimitIntegerThr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChCaelumPortRegFieldSet() : PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E");
    }
    return GT_OK;
}

