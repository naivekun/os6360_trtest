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
* @file prvCpssDxChPortIfModeCfgBobKResource.h
*
* @brief CPSS BobK implementation for Port interface mode resource configuration.
*
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PORT_IF_MODE_CFG_BOBK_RESOURCE_H
#define __PRV_CPSS_DXCH_PORT_IF_MODE_CFG_BOBK_RESOURCE_H


#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

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
    | TXFIFO-SCDMA Outgoing bus width               |TxFIFO-0| 0x27000600 | 0-73 |  1 |  74 |  3-5  | TX_FIFO/Units/<TXFIFO0_IP> TxFIFO %a/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
    |                                               |TxFIFO-1| 0x67000600 | 0-73 |  1 |  74 |  3-5  | TX_FIFO/Units/<TXFIFO1_IP> TxFIFO %a/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
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
    | TXFIFO-SCDMA Outgoing bus width               |TxFIFO-0| 0x27000600 | 0-73 |  1 |  74 |  3-5  | TX_FIFO/Units/<TXFIFO_IP> TxFIFO %a/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
    |                                               |TxFIFO-1| 0x67000600 | 0-73 |  1 |  74 |  3-5  |
    |                                               |TxFIFO-2| 0x6A000600 | 0-73 |  1 |  74 |  3-5  |
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
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DIV_0_32   0x00000000
#define DIV_1_32   0x80000000
#define DIV_2_32   0x80008000
#define DIV_3_32   0x80200400
#define DIV_4_32   0x80808080
#define DIV_5_32   0x82081040
#define DIV_6_32   0x84208420
#define DIV_7_32   0x88442210
#define DIV_8_32   0x88888888
#define DIV_9_32   0x91224488
#define DIV_10_32  0x92489248
#define DIV_11_32  0xa4924924
#define DIV_12_32  0xa4a4a4a4
#define DIV_13_32  0xa94a5294
#define DIV_14_32  0xaa54aa54
#define DIV_15_32  0xaaaa5554
#define DIV_16_32  0xaaaaaaaa
#define DIV_17_32  0xd555aaaa
#define DIV_18_32  0xd5aad5aa
#define DIV_19_32  0xd6b5ad6a
#define DIV_20_32  0xdadadada
#define DIV_21_32  0xdb6db6da
#define DIV_22_32  0xedb6edb6
#define DIV_23_32  0xeeddbb76
#define DIV_24_32  0xeeeeeeee
#define DIV_25_32  0xf7bbddee
#define DIV_26_32  0xfbdefbde
#define DIV_27_32  0xfdf7efbe
#define DIV_28_32  0xfefefefe
#define DIV_29_32  0xffdffbfe
#define DIV_30_32  0xfffefffe
#define DIV_31_32  0xfffffffe



#define PRV_CPSS_DXCH_PORT_RESOURCE_BAD_VALUE_CNS               0xFFFFFBAD

typedef struct
{
    GT_U32  rxdmaScdmaIncomingBusWidth;
    GT_U32  txdmaCreditValue;
    GT_U32  txdmaScdmaBurstFullThrshold;                /* BC3 */
    GT_U32  txdmaScdmaBurstAmostFullThrshold;           /* BC3 */
    GT_U32  txdmaScdmaRateLimitIntegerThreshold;        /* BC3 */
    GT_U32  txdmaScdmaRateLimitResidueVectThreshold;    /* BC3 */
    GT_U32  txdmaTxfifoHeaderCounterThresholdScdma;
    GT_U32  txdmaTxfifoPayloadCounterThresholdScdma;
    GT_U32  txfifoScdmaPayloadStartTransmThreshold;
    GT_U32  txfifoScdmaShiftersOutgoingBusWidth;
    GT_U32  ethTxfifoOutgoingBusWidth;
    GT_U32  ethTxfifoScdmaPayloadThreshold;
}PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC;


typedef struct
{
    GT_BOOL                                 valid;
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC  res;
}PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_DATABASE_STC;


typedef enum
{
    /* fields */
    PRV_CAELUM_REG_FIELDS_TXQ_Q_PORT_DESC_COUNTER_E,
    PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_PORT_CREDIT_COUNTER_E,
    PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
    /* resource fileds */
    PRV_CAELUM_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E,
    PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E,
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E,
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E,        /* BC3 only */
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E,               /* BC3 only */
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E,       /* BC3 only */
    PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E,  /* BC3 only */
    PRV_CAELUM_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E,              /* BC3 only */
    PRV_CAELUM_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E,              /* BC3 only */

    PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E,
    PRV_CAELUM_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E,
    PRV_CAELUM_REG_FIELDS_ETH_TXFIFO_OUTGOING_BUS_WIDTH_E,
    PRV_CAELUM_REG_FIELDS_ETH_TXFIFO_SCDMA_PAYLOAD_THRESHOLD_E,
    PRV_CAELUM_REG_FIELDS_XLG_MAC_FIFO_TX_READ_THRESHOLD_E,                 /* BC3 only */
    PRV_CAELUM_REG_FIELDS_LAST_E

}PRV_CAELUM_REG_FIELDS_ENT;


/**
* @internal prvCpssDxChCaelumPortResourceConfigGet function
* @endinternal
*
* @brief   Get resources allocated to the port
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
GT_STATUS prvCpssDxChCaelumPortResourceConfigGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_STC      *resourcePtr
);

/**
* @internal prvCpssDxChCaelumPortResourcesConfig function
* @endinternal
*
* @brief   Caelum port resource configuration
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
GT_STATUS prvCpssDxChCaelumPortResourcesConfig
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssDxChFalconPortResourcesInit function
* @endinternal
 @brief    Initialize data structure for port resource allocation.
*       In falcon no data is nedded for port resource allocation and the function just return.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2;
*         Bobcat3; Caelum; Aldrin; AC3X; Aldrin2; Falcon
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFalconPortResourcesInit
(
    IN    GT_U8                   devNum
);

/**
* @internal prvCpssDxChCaelumPortResourcesInit function
* @endinternal
*
* @brief   Initialize data structure for port resource allocation
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortResourcesInit
(
    IN    GT_U8                   devNum
);

/**
* @internal prvCpssDxChCaelumPortRegAddrGet function
* @endinternal
*
* @brief   Caelum port resource registers get
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
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
);

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
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortRegFieldSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CAELUM_REG_FIELDS_ENT   fieldCode,
    IN  GT_U32                      fieldValue
);

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
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
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
);

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
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortAllDataPathRegFieldSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CAELUM_REG_FIELDS_ENT   fieldCode,
    IN  GT_U32                      fieldValue
);


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
);

/**
* @internal prvCpssDxChCaelumPortResourceStaticTmSet function
* @endinternal
*
* @brief   Caelum TM port resource configuration
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon; Aldrin
*
* @param[in] devNum                   - physical device number
* @param[in] tmBandwidthMbps          - system TM bandwidth
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCaelumPortResourceStaticTmSet
(
    IN GT_U8    devNum,
    IN  GT_U32  tmBandwidthMbps
);

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
*/
GT_STATUS prvCpssDxChCaelumPortResourceDynamicTmSet
(
    IN GT_U8    devNum,
    IN  GT_U32  tmBandwidthMbps
);


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
);

/**
* @internal prvCpssDxChCaelumPortResourcesSpeedIndexGet function
* @endinternal
*
* @brief   Gets speed index in database
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

