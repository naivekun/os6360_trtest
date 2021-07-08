/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssHwInit.h
*
* @brief CPSS definitions for HW init.
*
* @version   48
********************************************************************************
*/
#ifndef __prvCpssHwInit_h
#define __prvCpssHwInit_h

#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/config/cpssGenCfg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* This is used to indicate the specific data types (pointer fields or write
   only data) in the registers addresses struct */
#define PRV_CPSS_SW_PTR_BOOKMARK_CNS     0xFFFFFFFF

/* This is used to mark pointer fields or registers that are not initialized in
   the registers addresses struct */
#define PRV_CPSS_SW_PTR_ENTRY_UNUSED     0xFFFFFFFB

/* This is used to mark write only data in the registers addresses struct */
#define PRV_CPSS_SW_TYPE_WRONLY_CNS     0x1

/* This is used to mark dynamically allocated pointer fields in the registers
   addresses struct */
#define PRV_CPSS_SW_TYPE_PTR_FIELD_CNS  0x2

/**
* @enum PRV_CPSS_GEN_REG_DB_KIND_ENT
 *
 * @brief the enumeration indicate kind of registers DB used to store
 * registers addresses.
*/
typedef enum{

    /** @brief DB which was the only DB till SIP5
     *  (see PRV_CPSS_DXCH/PX_PP_REGS_ADDR_STC)
     */
    PRV_CPSS_GEN_REG_DB_KIND_LEGACY_E,

    /** @brief additional DB, SIP5 devices only
     *  (see PRV_CPSS_DXCH/PX_PP_REGS_ADDR_VER1_STC)
     */
    PRV_CPSS_GEN_REG_DB_KIND_SIP5_E,

    /** DFX registers DB. SIP5 only. */
    PRV_CPSS_GEN_REG_DB_KIND_DFX_E

} PRV_CPSS_GEN_REG_DB_KIND_ENT;

/*
 * typedef: enum PRV_CPSS_DXCH_UNIT_ENT
 *
 * Description: Defines silicon units.
 *
 * Enumerations:
 *
 *  PRV_CPSS_DXCH_UNIT___NOT_VALID___E        - not valid value (value 0)
 *
 *  PRV_CPSS_DXCH_UNIT_TTI_E                  - tti unit
 *  PRV_CPSS_DXCH_UNIT_PCL_E                  - pcl unit
 *  PRV_CPSS_DXCH_UNIT_L2I_E                  - l2 unit
 *  PRV_CPSS_DXCH_UNIT_FDB_E                  - fdb unit
 *  PRV_CPSS_DXCH_UNIT_EQ_E                   - eq unit
 *  PRV_CPSS_DXCH_UNIT_LPM_E                  - lpm unit
 *  PRV_CPSS_DXCH_UNIT_EGF_EFT_E              - egf_eft unit
 *  PRV_CPSS_DXCH_UNIT_EGF_QAG_E              - egf_qag unit
 *  PRV_CPSS_DXCH_UNIT_EGF_SHT_E              - egf_sht unit
 *  PRV_CPSS_DXCH_UNIT_HA_E                   - ha unit
 *  PRV_CPSS_DXCH_UNIT_ETS_E                  - ets unit
 *  PRV_CPSS_DXCH_UNIT_MLL_E                  - mll unit
 *  PRV_CPSS_DXCH_UNIT_IPLR_E                 - iplr unit
 *  PRV_CPSS_DXCH_UNIT_EPLR_E                 - eplr unit
 *  PRV_CPSS_DXCH_UNIT_IPVX_E                 - ipvx unit
 *  PRV_CPSS_DXCH_UNIT_IOAM_E                 - ioam unit
 *  PRV_CPSS_DXCH_UNIT_EOAM_E                 - eoam unit
 *  PRV_CPSS_DXCH_UNIT_TCAM_E                 - tcam unit
 *  PRV_CPSS_DXCH_UNIT_RXDMA_E                - rx_dma unit
 *  PRV_CPSS_DXCH_UNIT_EPCL_E                 - EPCL unit
 *  PRV_CPSS_DXCH_UNIT_TM_FCU_E               -  unit TM_FCU
 *  PRV_CPSS_DXCH_UNIT_TM_DROP_E              -  unit TM_DROP
 *  PRV_CPSS_DXCH_UNIT_TM_QMAP_E              -  unit TM_QMAP
 *  PRV_CPSS_DXCH_UNIT_TM_E                   -  unit TM
 *  PRV_CPSS_DXCH_UNIT_GOP_E                  -  unit GOP
 *  PRV_CPSS_DXCH_UNIT_GOP1_E                 -  unit GOP for SIP5 devices
 *                                               (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
 *  PRV_CPSS_DXCH_UNIT_MIB_E                  -  unit MIB
 *  PRV_CPSS_DXCH_UNIT_SERDES_E               -  unit SERDES
 *  PRV_CPSS_DXCH_UNIT_ERMRK_E                -  unit ERMRK
 *  PRV_CPSS_DXCH_UNIT_BM_E                   -  unit BM
 *  PRV_CPSS_DXCH_UNIT_TM_INGRESS_GLUE_E      -  unit TM_INGRESS_GLUE
 *  PRV_CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E       -  unit TM_EGRESS_GLUE
 *  PRV_CPSS_DXCH_UNIT_ETH_TXFIFO_E           -  unit ETH_TXFIFO
 *  PRV_CPSS_DXCH_UNIT_ILKN_TXFIFO_E          -  unit ILKN_TXFIFO
 *  PRV_CPSS_DXCH_UNIT_ILKN_E                 -  unit ILKN
 *  PRV_CPSS_DXCH_UNIT_LMS0_0_E               -  unit LMS0_0
 *  PRV_CPSS_DXCH_UNIT_LMS0_1_E               -  unit LMS0_1
 *  PRV_CPSS_DXCH_UNIT_LMS0_2_E               -  unit LMS0_2
 *  PRV_CPSS_DXCH_UNIT_LMS0_3_E               -  unit LMS0_3
 *  PRV_CPSS_DXCH_UNIT_TX_FIFO_E              -  unit TX_FIFO
 *  PRV_CPSS_DXCH_UNIT_BMA_E                  -  unit BMA
 *  PRV_CPSS_DXCH_UNIT_CNC_0_E                -  unit CNC_0
 *  PRV_CPSS_DXCH_UNIT_CNC_1_E                -  unit CNC_1
 *  PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E            -  unit TXQ_QUEUE
 *  PRV_CPSS_DXCH_UNIT_TXQ_LL_E               -  unit TXQ_LL
 *  PRV_CPSS_DXCH_UNIT_TXQ_PFC_E              -  unit TXQ_PFC
 *  PRV_CPSS_DXCH_UNIT_TXQ_QCN_E              -  unit TXQ_QCN
 *  PRV_CPSS_DXCH_UNIT_TXQ_DQ_E               -  unit TXQ_DQ
 *  PRV_CPSS_DXCH_UNIT_DFX_SERVER_E           -  unit DFX_SERVER
 *  PRV_CPSS_DXCH_UNIT_MPPM_E                 -  unit MPPM
 *  PRV_CPSS_DXCH_UNIT_LMS1_0_E               -  unit LMS1_0
 *  PRV_CPSS_DXCH_UNIT_LMS1_1_E               -  unit LMS1_1
 *  PRV_CPSS_DXCH_UNIT_LMS1_2_E               -  unit LMS1_2
 *  PRV_CPSS_DXCH_UNIT_LMS1_3_E               -  unit LMS1_3
 *  PRV_CPSS_DXCH_UNIT_LMS2_0_E               -  unit LMS2_0
 *  PRV_CPSS_DXCH_UNIT_LMS2_1_E               -  unit LMS2_1
 *  PRV_CPSS_DXCH_UNIT_LMS2_2_E               -  unit LMS2_2
 *  PRV_CPSS_DXCH_UNIT_LMS2_3_E               -  unit LMS2_3
 *  PRV_CPSS_DXCH_UNIT_MPPM_1_E               -  unit MPPM_1
 *  PRV_CPSS_DXCH_UNIT_CTU_0_E                -  unit CTU_0
 *  PRV_CPSS_DXCH_UNIT_CTU_1_E                -  unit CTU_1
 *  PRV_CPSS_DXCH_UNIT_TXQ_SHT_E              -  unit TXQ_SHT
 *  PRV_CPSS_DXCH_UNIT_TXQ_EGR0_E             -  unit TXQ_EGR0
 *  PRV_CPSS_DXCH_UNIT_TXQ_EGR1_E             -  unit TXQ_EGR1
 *  PRV_CPSS_DXCH_UNIT_TXQ_DIST_E             -  unit TXQ_DIST
 *  PRV_CPSS_DXCH_UNIT_IPLR_1_E               -  unit IPLR_1
 *  PRV_CPSS_DXCH_UNIT_TXDMA_E                -  unit TXDMA
 *  PRV_CPSS_DXCH_UNIT_MG_E                   -  unit MG
 *  PRV_CPSS_DXCH_UNIT_TCC_IPCL_E             -  unit TCC_IPCL
 *  PRV_CPSS_DXCH_UNIT_TCC_IPVX_E             -  unit TCC_IPVX
 *  PRV_CPSS_DXCH_UNIT_SMI_0_E                - SMI unit 0
 *  PRV_CPSS_DXCH_UNIT_SMI_1_E                - SMI unit 1
 *  PRV_CPSS_DXCH_UNIT_SMI_2_E                - SMI unit 2
 *  PRV_CPSS_DXCH_UNIT_SMI_3_E                - SMI unit 3
 *  PRV_CPSS_DXCH_UNIT_LED_0_E                - LED unit 0
 *  PRV_CPSS_DXCH_UNIT_LED_1_E                - LED unit 1
 *  PRV_CPSS_DXCH_UNIT_LED_2_E                - LED unit 2
 *  PRV_CPSS_DXCH_UNIT_LED_3_E                - LED unit 3
 *  PRV_CPSS_DXCH_UNIT_LED_4_E                - LED unit 4
 *
 *  PRV_CPSS_DXCH_UNIT_RXDMA1_E               - RXDMA   unit index 1 (bobk)
 *  PRV_CPSS_DXCH_UNIT_TXDMA1_E               - TXDMA   unit index 1 (bobk)
 *  PRV_CPSS_DXCH_UNIT_TX_FIFO1_E             - TX_FIFO unit index 1 (bobk)
 *  PRV_CPSS_DXCH_UNIT_ETH_TX_FIFO1_E          - ETH_TXFIFO1 unit index 1 (bobk)
 *
 *  PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E           - RXDMA GLUE unit (bobk)
 *  PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E           - TXDMA GLUE unit (bobk)
 *  PRV_CPSS_DXCH_UNIT_MPPM_E                 - MPPM unit (bobk)
 *
 *
 *   PRV_CPSS_DXCH_UNIT_RXDMA2_E,             - RxDma unit index 2 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_RXDMA3_E,             - RxDma unit index 3 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_RXDMA4_E,             - RxDma unit index 4 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_RXDMA5_E,             - RxDma unit index 5 (bobcat3)
 *
 *   PRV_CPSS_DXCH_UNIT_TXDMA2_E,             - TxDma unit index 2 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TXDMA3_E,             - TxDma unit index 3 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TXDMA4_E,             - TxDma unit index 4 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TXDMA5_E,             - TxDma unit index 5 (bobcat3)
 *
 *   PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,           - TxFifo unit index 2 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TX_FIFO3_E,           - TxFifo unit index 3 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TX_FIFO4_E,           - TxFifo unit index 4 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TX_FIFO5_E,           - TxFifo unit index 5 (bobcat3)
 *
 *   PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E,            - txq-DQ unit index 1 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TXQ_DQ2_E,            - txq-DQ unit index 2 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TXQ_DQ3_E,            - txq-DQ unit index 3 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TXQ_DQ4_E,            - txq-DQ unit index 4 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TXQ_DQ5_E,            - txq-DQ unit index 5 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_TAI_E                 - TAI for single PRV_CPSS_DXCH_UNIT_TAI_E device
 *   PRV_CPSS_DXCH_UNIT_TAI1_E                - TAI for single PRV_CPSS_DXCH_UNIT_TAI_E device of pipe 1 (bobcat3)
 *
 *   PRV_CPSS_DXCH_UNIT_EGF_SHT_1_E           - egf_sht unit of pipe 1 (bobcat3)
 *
 *   PRV_CPSS_DXCH_UNIT_TTI_1_E               - TTI pipe 1        (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_IPCL_1_E              - IPCL pipe 1       (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_L2I_1_E               - L2I pipe 1        (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_IPVX_1_E              - IPVX pipe 1       (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_IPLR_1_E              - IPLR pipe 1       (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_IPLR1_1_E             - IPLR1 pipe 1      (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_IOAM_1_E              - IOAM pipe 1       (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_MLL_1_E               - MLL pipe 1        (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_EQ_1_E                - EQ pipe 1         (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_EGF_EFT_1_E           - EGF_EFT pipe 1    (bobcat3)
 *
 *   PRV_CPSS_DXCH_UNIT_CNC_0_1_E             - CNC0 pipe 1       (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_CNC_1_1_E             - CNC1 pipe 1       (bobcat3)
 *
 *   PRV_CPSS_DXCH_UNIT_SERDES_1_E            - SERDES pipe 1     (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_HA_1_E                - HA pipe 1         (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_ERMRK_1_E             - ERMRK pipe 1      (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_EPCL_1_E              - EPCL pipe 1       (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_EPLR_1_E              - EPLR pipe 1       (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_EOAM_1_E              - EOAM pipe 1       (bobcat3)
 *
 *   PRV_CPSS_DXCH_UNIT_RX_DMA_GLUE_1_E       - RXDMA GLUE pipe 1 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_POE_E                 - POE               (Aldrin)
 *   PRV_CPSS_DXCH_UNIT_IA_E                  - Ingress aggregator (IA) pipe 0 (bobcat3)
 *   PRV_CPSS_DXCH_UNIT_IA_1_E                - Ingress aggregator (IA) pipe 1 (bobcat3)
 *
 *   PRV_CPSS_PX_UNIT_PHA_E                   - PX/Falcon device : PHA unit pipe 0
 *   PRV_CPSS_PX_UNIT_PHA_1_E                 - PX/Falcon device : PHA unit pipe 1
 *   PRV_CPSS_PX_UNIT_PCP_E                   - PX device : PCP unit
 *   PRV_CPSS_PX_UNIT_MCFC_E                  - PX device : MCFC unit
 *
 *   PRV_CPSS_DXCH_UNIT_SHARED_TABLE_E        - Falcon : shared tables : FDB,TS/ARP
 *
 *   PRV_CPSS_DXCH_UNIT_BMA_1_E               - Falcon : BMA in pipe 1 . dummy for simulation purposes
 *   PRV_CPSS_DXCH_UNIT_EGF_QAG_1_E           - Falcon : EGF_QAG in pipe 1 .
 *
 **********************************
 *   PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E - for devices without tiles. this is 'last valid' unit.
 *
 *   PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E      - Falcon : the base Id for units that exists in Tile 1.
 *                                              to this base any 'tile 0 ' unit can be ADDED to it.
 *   PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E      - Falcon : the base Id for units that exists in Tile 2.
 *                                              to this base any 'tile 0 ' unit can be ADDED to it.
 *   PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E      - Falcon : the base Id for units that exists in Tile 3.
 *                                              to this base any 'tile 0 ' unit can be ADDED to it.
 **********************************
 * Falcon Packet Buffer Subunits
 *  PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E                 - Cenrter and counters and Cell write arbiter
 *  PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E          - Cell SMB Write Arbiter
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E     - Packet write subunit0
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E        - Cell Read subunit0
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E        - Cell Read subunit0
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E      - Packet Read
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E      - Packet Read
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E     - Packet write subunit1
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E        - Cell Read subunit2
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E        - Cell Read subunit3
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E      - Packet Read
 *  PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E      - Packet Read
 *  PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E        - Next Pointer Memory Cluster0
 *  PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E        - Next Pointer Memory Cluster1
 *  PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_2_E        - Next Pointer Memory Cluster1
 *  PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E        - Shared Cell Memory Cluster0
 *  PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E        - Shared Cell Memory Cluster1
 *  PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E        - Shared Cell Memory Cluster2
 *  PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_1_E        - Shared Cell Memory Cluster3
 *  PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E        - Shared Cell Memory Cluster4
 *  PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_1_E        - Shared Cell Memory Cluster5
 **********************************
 *
 *  PRV_CPSS_DXCH_UNIT_LAST_E       - must be last
 *
 */

/* distance between 'enum of units' of 2 consecutive tiles */
#define PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS   (PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E + 1)

typedef enum {
    PRV_CPSS_DXCH_UNIT___NOT_VALID___E = 0,
    PRV_CPSS_DXCH_UNIT_TTI_E,
    PRV_CPSS_DXCH_UNIT_PCL_E,
    PRV_CPSS_DXCH_UNIT_L2I_E,
    PRV_CPSS_DXCH_UNIT_FDB_E,
    PRV_CPSS_DXCH_UNIT_EQ_E,
    PRV_CPSS_DXCH_UNIT_LPM_E,
    PRV_CPSS_DXCH_UNIT_EGF_EFT_E,
    PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
    PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    PRV_CPSS_DXCH_UNIT_HA_E,
    PRV_CPSS_DXCH_UNIT_ETS_E,
    PRV_CPSS_DXCH_UNIT_MLL_E,
    PRV_CPSS_DXCH_UNIT_IPLR_E,
    PRV_CPSS_DXCH_UNIT_EPLR_E,
    PRV_CPSS_DXCH_UNIT_IPVX_E,
    PRV_CPSS_DXCH_UNIT_IOAM_E,
    PRV_CPSS_DXCH_UNIT_EOAM_E,
    PRV_CPSS_DXCH_UNIT_TCAM_E,
    PRV_CPSS_DXCH_UNIT_RXDMA_E,
    PRV_CPSS_DXCH_UNIT_EPCL_E,
    PRV_CPSS_DXCH_UNIT_TM_FCU_E,
    PRV_CPSS_DXCH_UNIT_TM_DROP_E,
    PRV_CPSS_DXCH_UNIT_TM_QMAP_E,
    PRV_CPSS_DXCH_UNIT_TM_E,
    PRV_CPSS_DXCH_UNIT_GOP_E,
    PRV_CPSS_DXCH_UNIT_MIB_E,
    PRV_CPSS_DXCH_UNIT_SERDES_E,
    PRV_CPSS_DXCH_UNIT_ERMRK_E,
    PRV_CPSS_DXCH_UNIT_BM_E,
    PRV_CPSS_DXCH_UNIT_TM_INGRESS_GLUE_E,
    PRV_CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E,
    PRV_CPSS_DXCH_UNIT_ETH_TXFIFO_E,
    PRV_CPSS_DXCH_UNIT_ILKN_TXFIFO_E,
    PRV_CPSS_DXCH_UNIT_ILKN_E,
    PRV_CPSS_DXCH_UNIT_LMS0_0_E,
    PRV_CPSS_DXCH_UNIT_LMS0_1_E,
    PRV_CPSS_DXCH_UNIT_LMS0_2_E,
    PRV_CPSS_DXCH_UNIT_LMS0_3_E,
    PRV_CPSS_DXCH_UNIT_TX_FIFO_E,
    PRV_CPSS_DXCH_UNIT_BMA_E,
    PRV_CPSS_DXCH_UNIT_CNC_0_E,
    PRV_CPSS_DXCH_UNIT_CNC_1_E,
    PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    PRV_CPSS_DXCH_UNIT_TXQ_LL_E,
    PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,
    PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,
    PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,
    PRV_CPSS_DXCH_UNIT_DFX_SERVER_E,
    PRV_CPSS_DXCH_UNIT_MPPM_E,
    PRV_CPSS_DXCH_UNIT_LMS1_0_E,
    PRV_CPSS_DXCH_UNIT_LMS1_1_E,
    PRV_CPSS_DXCH_UNIT_LMS1_2_E,
    PRV_CPSS_DXCH_UNIT_LMS1_3_E,
    PRV_CPSS_DXCH_UNIT_LMS2_0_E,
    PRV_CPSS_DXCH_UNIT_LMS2_1_E,
    PRV_CPSS_DXCH_UNIT_LMS2_2_E,
    PRV_CPSS_DXCH_UNIT_LMS2_3_E,
    PRV_CPSS_DXCH_UNIT_MPPM_1_E,
    PRV_CPSS_DXCH_UNIT_CTU_0_E,
    PRV_CPSS_DXCH_UNIT_CTU_1_E,
    PRV_CPSS_DXCH_UNIT_TXQ_SHT_E ,
    PRV_CPSS_DXCH_UNIT_TXQ_EGR0_E,
    PRV_CPSS_DXCH_UNIT_TXQ_EGR1_E,
    PRV_CPSS_DXCH_UNIT_TXQ_DIST_E,
    PRV_CPSS_DXCH_UNIT_IPLR_1_E,
    PRV_CPSS_DXCH_UNIT_TXDMA_E,
    PRV_CPSS_DXCH_UNIT_MG_E,                /*NOTE: in Falcon is also known as MG_0_0 (sub unit in MNG unit) */
    PRV_CPSS_DXCH_UNIT_TCC_IPCL_E,
    PRV_CPSS_DXCH_UNIT_TCC_IPVX_E,

    /* RFU in CNM unit */
    PRV_CPSS_DXCH_UNIT_CNM_RFU_E,
    /* no more LMS unit : was split to SMI , LED units */
    PRV_CPSS_DXCH_UNIT_SMI_0_E,
    PRV_CPSS_DXCH_UNIT_SMI_1_E,
    PRV_CPSS_DXCH_UNIT_SMI_2_E,
    PRV_CPSS_DXCH_UNIT_SMI_3_E,

    /* no more LMS unit : was split to SMI , LED units */
    PRV_CPSS_DXCH_UNIT_LED_0_E,
    PRV_CPSS_DXCH_UNIT_LED_1_E,
    PRV_CPSS_DXCH_UNIT_LED_2_E,
    PRV_CPSS_DXCH_UNIT_LED_3_E,
    PRV_CPSS_DXCH_UNIT_LED_4_E,

    PRV_CPSS_DXCH_UNIT_RXDMA1_E,
    PRV_CPSS_DXCH_UNIT_TXDMA1_E,
    PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,
    PRV_CPSS_DXCH_UNIT_ETH_TXFIFO1_E,

    PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E,
    PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E,


    PRV_CPSS_DXCH_UNIT_RXDMA2_E,
    PRV_CPSS_DXCH_UNIT_RXDMA3_E,                      /* BC3: pipe 1 , Falcon : Pipe 0 */
    PRV_CPSS_DXCH_UNIT_RXDMA4_E,                      /* BC3: pipe 1 , Falcon : Pipe 1 */
    PRV_CPSS_DXCH_UNIT_RXDMA5_E,                      /* BC3: pipe 1 , Falcon : Pipe 1 */
    PRV_CPSS_DXCH_UNIT_RXDMA6_E,                      /* Falcon: pipe 1*/
    PRV_CPSS_DXCH_UNIT_RXDMA7_E,                      /* Falcon: pipe 1*/

    PRV_CPSS_DXCH_UNIT_TXDMA2_E,
    PRV_CPSS_DXCH_UNIT_TXDMA3_E,                      /* BC3: pipe 1 , Falcon : Pipe 0 */
    PRV_CPSS_DXCH_UNIT_TXDMA4_E,                      /* BC3: pipe 1 , Falcon : Pipe 1 */
    PRV_CPSS_DXCH_UNIT_TXDMA5_E,                      /* BC3: pipe 1 , Falcon : Pipe 1 */
    PRV_CPSS_DXCH_UNIT_TXDMA6_E,                      /* Falcon: pipe 1*/
    PRV_CPSS_DXCH_UNIT_TXDMA7_E,                      /* Falcon: pipe 1*/

    PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,
    PRV_CPSS_DXCH_UNIT_TX_FIFO3_E,                    /* BC3: pipe 1 , Falcon : Pipe 0 */
    PRV_CPSS_DXCH_UNIT_TX_FIFO4_E,                    /* BC3: pipe 1 , Falcon : Pipe 1 */
    PRV_CPSS_DXCH_UNIT_TX_FIFO5_E,                    /* BC3: pipe 1 , Falcon : Pipe 1 */
    PRV_CPSS_DXCH_UNIT_TX_FIFO6_E,                    /* Falcon: pipe 1*/
    PRV_CPSS_DXCH_UNIT_TX_FIFO7_E,                    /* Falcon: pipe 1*/

    PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E,
    PRV_CPSS_DXCH_UNIT_TXQ_DQ2_E,
    PRV_CPSS_DXCH_UNIT_TXQ_DQ3_E,                     /* BC3: pipe 1*/
    PRV_CPSS_DXCH_UNIT_TXQ_DQ4_E,                     /* BC3: pipe 1*/
    PRV_CPSS_DXCH_UNIT_TXQ_DQ5_E,                     /* BC3: pipe 1*/

    PRV_CPSS_DXCH_UNIT_MIB1_E,                        /* MIB pipe 1*/
    PRV_CPSS_DXCH_UNIT_GOP1_E,                        /* GOP pipe 1*/

    PRV_CPSS_DXCH_UNIT_TAI_E,
    PRV_CPSS_DXCH_UNIT_TAI1_E,

    PRV_CPSS_DXCH_UNIT_EGF_SHT_1_E,                   /* EGF SHT pipe 1*/

    PRV_CPSS_DXCH_UNIT_TTI_1_E                       ,/* TTI pipe 1 */
    PRV_CPSS_DXCH_UNIT_IPCL_1_E                      ,/* IPCL pipe 1 */
    PRV_CPSS_DXCH_UNIT_L2I_1_E                       ,/* L2I pipe 1 */
    PRV_CPSS_DXCH_UNIT_IPVX_1_E                      ,/* IPVX pipe 1 */
    PRV_CPSS_DXCH_UNIT_IPLR_0_1_E                    ,/* IPLR pipe 1 */
    PRV_CPSS_DXCH_UNIT_IPLR_1_1_E                    ,/* IPLR1 pipe 1 */
    PRV_CPSS_DXCH_UNIT_IOAM_1_E                      ,/* IOAM pipe 1 */
    PRV_CPSS_DXCH_UNIT_MLL_1_E                       ,/* MLL pipe 1 */
    PRV_CPSS_DXCH_UNIT_EQ_1_E                        ,/* EQ pipe 1 */
    PRV_CPSS_DXCH_UNIT_EGF_EFT_1_E                   ,/* EGF_EFT pipe 1 */
    PRV_CPSS_DXCH_UNIT_CNC_0_1_E                     ,/* CNC0 pipe 1 */
    PRV_CPSS_DXCH_UNIT_CNC_1_1_E                     ,/* CNC1 pipe 1 */
    PRV_CPSS_DXCH_UNIT_SERDES_1_E                    ,/* SERDES pipe 1 */
    PRV_CPSS_DXCH_UNIT_HA_1_E                        ,/* HA pipe 1 */
    PRV_CPSS_DXCH_UNIT_ERMRK_1_E                     ,/* ERMRK pipe 1 */
    PRV_CPSS_DXCH_UNIT_EPCL_1_E                      ,/* EPCL pipe 1 */
    PRV_CPSS_DXCH_UNIT_EPLR_1_E                      ,/* EPLR pipe 1 */
    PRV_CPSS_DXCH_UNIT_EOAM_1_E                      ,/* EOAM pipe 1 */
    PRV_CPSS_DXCH_UNIT_RX_DMA_GLUE_1_E               ,/* RXDMA GLUE pipe 1 */

    PRV_CPSS_DXCH_UNIT_POE_E,

    PRV_CPSS_DXCH_UNIT_TXQ_BMX_E,
    PRV_CPSS_DXCH_UNIT_LPM_1_E                       ,/* LPM pipe 1 */
    PRV_CPSS_DXCH_UNIT_IA_E                          ,/* IA pipe 0 */
    PRV_CPSS_DXCH_UNIT_IA_1_E                        ,/* IA pipe 1 */
    PRV_CPSS_DXCH_UNIT_EREP_E                        ,/* EREP pipe 0 */
    PRV_CPSS_DXCH_UNIT_EREP_1_E                      ,/* EREP pipe 1 */
    PRV_CPSS_DXCH_UNIT_PREQ_E                        ,/* PREQ pipe 0 */
    PRV_CPSS_DXCH_UNIT_PREQ_1_E                      ,/* PREQ pipe 1 */

    PRV_CPSS_DXCH_UNIT_PHA_E                         ,/* Falcon : PHA unit (pipe 0) */
    PRV_CPSS_DXCH_UNIT_PHA_1_E                       ,/* Falcon : PHA unit (pipe 1)  */

    PRV_CPSS_PX_UNIT_PHA_E                           ,/* PX device : PHA unit  */
    PRV_CPSS_PX_UNIT_PCP_E                           ,/* PX device : PCP unit */
    PRV_CPSS_PX_UNIT_MCFC_E                          ,/* PX device : MCFC unit */

    PRV_CPSS_DXCH_UNIT_SHM_E                         ,/*Falcon : configuration for shared memories : FDB,TS/ARP/LPM/EM */

    PRV_CPSS_DXCH_UNIT_MG_0_1_E                      ,/*Falcon : MG_0_1 (sub unit in MNG unit) */
    PRV_CPSS_DXCH_UNIT_MG_0_2_E                      ,/*Falcon : MG_0_2 (sub unit in MNG unit) */
    PRV_CPSS_DXCH_UNIT_MG_0_3_E                      ,/*Falcon : MG_0_3 (sub unit in MNG unit) */
    PRV_CPSS_DXCH_UNIT_MG_1_0_E                      ,/*Falcon : MG_1_0 (sub unit in MNG unit) */
    PRV_CPSS_DXCH_UNIT_MG_1_1_E                      ,/*Falcon : MG_1_1 (sub unit in MNG unit) */
    PRV_CPSS_DXCH_UNIT_MG_1_2_E                      ,/*Falcon : MG_1_2 (sub unit in MNG unit) */
    PRV_CPSS_DXCH_UNIT_MG_1_3_E                      ,/*Falcon : MG_1_3 (sub unit in MNG unit) */

    PRV_CPSS_DXCH_UNIT_BMA_1_E                       ,/*Falcon : BMA in pipe 1 . dummy for simulation purposes  */
    PRV_CPSS_DXCH_UNIT_EGF_QAG_1_E                     ,/*Falcon : EGF_QAG in pipe 1 .  */
    PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                       ,/*Falcon : Txq_PSI */
    PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                       ,/*Falcon : Txq_PDX */
    PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E                 ,/*Falcon : Txq_PDX_PAC_0 */
    PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E                 ,/*Falcon : Txq_PDX_PAC_1 */
    PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E                 ,/*Falcon : Txq_PFCC */
    PRV_CPSS_DXCH_UNIT_EM_E                            ,/*Falcon : EM */
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E                ,/*Falcon : Txq_PDS - pipe 0*/
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS0_E                ,/*Falcon : Txq_PDS - pipe 1*/
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS1_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS2_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS3_E                ,

    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E                ,/*Falcon : Txq_SDQ - pipe 0*/
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ0_E                ,/*Falcon : Txq_SDQ - pipe 1*/
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ1_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ2_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ3_E                ,

    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E                ,/*Falcon : Txq_QFC - pipe 0*/
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC0_E                ,/*Falcon : Txq_QFC - pipe 1*/
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC1_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC2_E                ,
    PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC3_E                ,

    PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI0_E          ,/*Falcon : TAI Slave - pipe 0*/
    PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI1_E          ,
    PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI0_E          ,/*Falcon : TAI Slave - pipe 1*/
    PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI1_E          ,

    PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0_E      ,/*Falcon : TXQ TAI Slave - pipe 0*/
    PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI1_E      ,
    PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI0_E      ,/*Falcon : TXQ TAI Slave - pipe 1*/
    PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI1_E      ,

    PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E                 ,/* Falcon Packet Buffer Subunits */
    PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E          ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E     ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E        ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E        ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E      ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E      ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E     ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E        ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E        ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E      ,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E      ,
    PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E        ,
    PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E        ,
    PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_2_E        ,
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E        ,
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E        ,
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E        ,
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_1_E        ,
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E        ,
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_1_E        ,

    PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E                  ,  /* Raven Base Address in Falcon (access to all units in Raven via D2D_CP unit)*/
    PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E                  ,  /* Raven Base Address in Falcon (access to all units in Raven via D2D_CP unit)*/
    PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E                  ,  /* Raven Base Address in Falcon (access to all units in Raven via D2D_CP unit)*/
    PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E                  ,  /* Raven Base Address in Falcon (access to all units in Raven via D2D_CP unit)*/

    PRV_CPSS_DXCH_UNIT_RAVEN_0_TAI0_E                  ,  /* Raven0 TAI base */
    PRV_CPSS_DXCH_UNIT_RAVEN_0_TAI1_E                  ,  
    PRV_CPSS_DXCH_UNIT_RAVEN_1_TAI0_E                  ,  /* Raven1 TAI base */
    PRV_CPSS_DXCH_UNIT_RAVEN_1_TAI1_E                  ,  
    PRV_CPSS_DXCH_UNIT_RAVEN_2_TAI0_E                  ,  /* Raven2 TAI base */
    PRV_CPSS_DXCH_UNIT_RAVEN_2_TAI1_E                  ,  
    PRV_CPSS_DXCH_UNIT_RAVEN_3_TAI0_E                  ,  /* Raven3 TAI base */
    PRV_CPSS_DXCH_UNIT_RAVEN_3_TAI1_E                  ,  

    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E                   , /*Falcon : pipe 0*/
    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E                   , /*Falcon : pipe 0*/
    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E                   , /*Falcon : pipe 0*/
    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E                   , /*Falcon : pipe 0*/
    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_4_E                   , /*Falcon : pipe 1*/
    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_5_E                   , /*Falcon : pipe 1*/
    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_6_E                   , /*Falcon : pipe 1*/
    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_7_E                   , /*Falcon : pipe 1*/

    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_0_IN_RAVEN_E       ,  /* Falcon : the Eagle hold D2D CP unit in the Raven memory space */
    PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_1_IN_RAVEN_E       ,  /* Falcon : the Eagle hold D2D CP unit in the Raven memory space */

    PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E          ,/* must be last for units that exists in 'tile 0' */

    /* base for Tile1 units */
    PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E               = PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS ,

    /* base for Tile2 units */
    PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E               = PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E + PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS,

    /* base for Tile3 units */
    PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E               = PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E + PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS,

    /* last of Tile3 units */
    PRV_CPSS_DXCH_UNIT_IN_TILE_3_LAST_E               = PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E + PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS - 1,

    PRV_CPSS_DXCH_UNIT_LAST_E /*must be last*/
}PRV_CPSS_DXCH_UNIT_ENT;


/**
* @enum PRV_CPSS_SUBUNIT_ENT
 *
 * @brief Defines silicon subunits - having multiple instances.
*/
typedef enum{

    /** @brief TAI (Time Application Interface) subunit
     *  pair per port group
     */
    PRV_CPSS_SUBUNIT_GOP_TAI_E,

    /** PTP Timestamp subunit (per port) */
    PRV_CPSS_SUBUNIT_GOP_PTP_E,

    /** @brief FCA (flow control aggregator)
     *  subunit (per port)
     */
    PRV_CPSS_SUBUNIT_GOP_FCA_E,

    /** PR (Packet Reassembly) subunit */
    PRV_CPSS_SUBUNIT_GOP_PR_E,

    PRV_CPSS_SUBUNIT_GOP_CG_E

} PRV_CPSS_SUBUNIT_ENT;

/*
 * Typedef: struct PRV_CPSS_BOOKMARK_STC
 *
 * Description: This is used to indicate the specific data types (pointer fields or write only data) in the registers addresses struct
 *
 * Fields:
 *
 *              bookmark - bookmark itself
 *              type - SW type (PRV_CPSS_SW_TYPE_WRONLY_CNS or PRV_CPSS_SW_TYPE_PTR_FIELD_CNS)
 *              size - size of bookmark
 *              nextPtr - pointer to the next bookmark
  */

/* The registers DB (PRV_CPSS_DXCH_PP_REGS_ADDR_STC) is used in several generic
   algorithm like registers dump or base address auto conversion. These
   algorithms require that the PRV_CPSS_DXCH_PP_REGS_ADDR_STC structure
   and PRV_CPSS_BOOKMARK_STC have packed members. This is native in 32 Bit
   machines but not true in 64 Bit machines.
   Use 4 byte alignment to guaranty that nextPtr will be right after size in
   both registers DB and in the PRV_CPSS_BOOKMARK_STC. */
#if __WORDSIZE == 64
#pragma pack(4)
#endif
typedef struct GT_PACKED bkmark
{
                GT_U32  bookmark;
                GT_U32  type;
                GT_U32  size;
                GT_U32  *nextPtr;
} PRV_CPSS_BOOKMARK_STC;
/* restore structure packing to default */
#if __WORDSIZE == 64
#pragma pack()
#endif
/**
* @struct PRV_CPSS_RE_INIT_RX_DESC_INFO_STC
 *
 * @brief The information block of a single Rx descriptors list used to re-initialise.
*/
typedef struct{

    void *rxDescBlock;

    /** total Number of Descriptors */
    GT_U32 totalDescNum;

    /** @brief Offset in the list of the descriptor to which the next returned
     *  buffer should be attached.
     */
    GT_U32 next2ReturnOffset;

    /** @brief Offset in the list of the descriptor from which the next packet
     *  will be fetched when an Rx interrupt is received.
     */
    GT_U32 next2ReceiveOffset;

    /** @brief Number of all descriptors in list.
     *  freeDescNum     - Number of free descriptors in list.
     */
    GT_U32 freeDescNum;

    /** @brief Number of reserved bytes before each buffer, to be
     *  kept for application and internal use.
     */
    GT_U32 headerOffset;

    /** @brief When set to GT_TRUE enabling the Rx SDMA on buffer
     *  release is forbidden.
     */
    GT_BOOL forbidQEn;

} PRV_CPSS_RE_INIT_RX_DESC_INFO_STC;

/**
* @struct PRV_CPSS_DMA_RE_INIT_INFO_STC
 *
 * @brief The structure that hold info for DMA related settings
 * done during 'PP logical init'.
 * This info needed for devices that doing re-init
 * without resetting the device HW.
*/
typedef struct{

    /** network interface configuration */
    CPSS_NET_IF_CFG_STC netIfCfg;

    /** Address Update Messages Queue CFG */
    CPSS_AUQ_CFG_STC auqCfg;

    /** Use separate Queue for FDB Upload Messages */
    GT_BOOL fuqUseSeparate;

    /** FDB Upload Messages Queue CFG */
    CPSS_AUQ_CFG_STC fuqCfg;

    PRV_CPSS_AU_DESC_CTRL_STC auqInternalInfo[CPSS_MAX_PORT_GROUPS_CNS];

    PRV_CPSS_AU_DESC_CTRL_STC fuqInternalInfo[CPSS_MAX_PORT_GROUPS_CNS];

    PRV_CPSS_AU_DESC_CTRL_STC secondaryAuqInternalInfo[CPSS_MAX_PORT_GROUPS_CNS];

    PRV_CPSS_RE_INIT_RX_DESC_INFO_STC rxDescInfo[NUM_OF_SDMA_QUEUES];

    PRV_CPSS_AU_DESC_CTRL_STC auq1InternalInfo[CPSS_MAX_PORT_GROUPS_CNS];

    PRV_CPSS_AU_DESC_CTRL_STC secondaryAuq1InternalInfo[CPSS_MAX_PORT_GROUPS_CNS];

    PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC auqDeadLockWa[CPSS_MAX_PORT_GROUPS_CNS];

    GT_U32 activeAuqIndex[CPSS_MAX_PORT_GROUPS_CNS];

} PRV_CPSS_DMA_RE_INIT_INFO_STC;

/**
* @enum PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT
 *
 * @brief Defines pre phase 1 init mode .
*/
typedef enum{

    /** regular pre phase 1 init mode */
    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_DEFAULT_INIT_E,

    /** no Pp initialization should be done */
    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_NO_PP_INIT_E

} PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT;

/**
* @internal prvCpssPrePhase1PpInit function
* @endinternal
*
* @brief   private (internal) function to make phase1 pre-init
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] prePhase1InitMode        - pre phase 1 init mode.
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssPrePhase1PpInit
(
    IN    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT     prePhase1InitMode
);


/**
* @internal prvCpssPrePhase1PpInitModeGet function
* @endinternal
*
* @brief   private (internal) function retrieving pre phase1 init mode
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] prePhase1InitModePtr     - pointer to pre phase 1 init mode .
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssPrePhase1PpInitModeGet
(
    OUT PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT     *prePhase1InitModePtr
);

/**
* @internal prvCpssPpConfigDevDbRelease function
* @endinternal
*
* @brief   private (internal) function to release all the DB of the device.
*         NOTE: function 'free' the allocated memory ,buffers, semaphores
*         and restore DB to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*/
GT_STATUS prvCpssPpConfigDevDbRelease
(
    IN    GT_U8     devNum
);

/**
* @internal prvCpssPpConfigDevDbBusBaseAddrAttach function
* @endinternal
*
* @brief   private (internal) function to attach the base address to devNum
*         this needed for the 'initialization after the DB release processing'
*         when the initialization for a device during 'phase 1' will be we will
*         know what 'old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] busBaseAddr              -  the address of the device on the interface bus (pci/pex/smi/twsi)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*/
GT_STATUS prvCpssPpConfigDevDbBusBaseAddrAttach
(
    IN GT_U8        devNum,
    IN GT_UINTPTR   busBaseAddr
);

/**
* @internal prvCpssPpConfigDevDbHwResetSet function
* @endinternal
*
* @brief   private (internal) function to state that the device did HW reset or that
*         the device finished the 'pp logic init' function
*         this needed for the 'initialization after the DB release processing'
*         when the initialization for a device during 'phase 1' will be we will
*         know what 'old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] didHwReset               - GT_TRUE - Did HW reset ,
*                                      GT_FALSE -  finished pp logical init.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssPpConfigDevDbHwResetSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   didHwReset
);

/**
* @internal prvCpssPpConfigDevDbHwResetGet function
* @endinternal
*
* @brief   private (internal) function get the state that the device did HW reset
*         or that the device finished the 'pp logic init' function.
*         this needed for the 'initialization after the DB release processing'
*         when the initialization for a device during 'phase 1' will be we will
*         know what 'old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] didHwResetPtr            - GT_TRUE - Did HW reset ,
*                                      GT_FALSE -  finished pp logical init.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigDevDbHwResetGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *didHwResetPtr
);

/**
* @internal prvCpssPpConfigDevDbRenumber function
* @endinternal
*
* @brief   the function set parameters for the driver to renumber it's DB.
*         the function "renumber" the current device number of the device to a
*         new device number.
*         NOTE:
*         this function MUST be called under 'Interrupts are locked'
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - The PP's "old" device number .
* @param[in] newDevNum                - The PP's "new" device number swap the DB to.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - the new device number is already used
*/
GT_STATUS prvCpssPpConfigDevDbRenumber
(
    IN GT_U8    oldDevNum,
    IN GT_U8    newDevNum
);

/**
* @internal prvCpssPpConfigDevInfoGet function
* @endinternal
*
* @brief   Private (internal) function returns device static information
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] devInfoPtr               - (pointer to) device information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
*/
GT_STATUS prvCpssPpConfigDevInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_GEN_CFG_DEV_INFO_STC   *devInfoPtr
);


/**
* @internal prvCpssPpConfigDevDbNetIfInitSet function
* @endinternal
*
* @brief   Private (internal) function to set if network initialization is needed or
*         if configuration will be based on previously saved re-initialization parameters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] needNetIfInit            - GT_TRUE  - Need network interface init
*                                      GT_FALSE - Network interface init using previously saved values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssPpConfigDevDbNetIfInitSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   needNetIfInit
);

/**
* @internal prvCpssPpConfigDevDbNetIfInitGet function
* @endinternal
*
* @brief   Private (internal) function to get whether network initialization is needed or
*         whether configuration will be based on previously saved re-initialization parameters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] needNetIfInitPtr         - (pointer to):
*                                      GT_TRUE  - Need network interface init
*                                      GT_FALSE - Network interface init using previously saved values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigDevDbNetIfInitGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *needNetIfInitPtr
);

/**
* @internal prvCpssPpConfigDevDbPrvNetInfInfoSet function
* @endinternal
*
* @brief   private (internal) function to :
*         set the Rx descriptors list private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] rxDescPtr                - (pointer to) the start of Rx descriptors block.
* @param[in] rxDescListPtr            - (pointer to)Rx descriptors list private info.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbPrvNetInfInfoSet
(
    IN GT_U8                        devNum,
    IN PRV_CPSS_RX_DESC_STC         *rxDescPtr,
    IN PRV_CPSS_RX_DESC_LIST_STC    *rxDescListPtr
);

/**
* @internal prvCpssPpConfigDevDbPrvNetInfInfoGet function
* @endinternal
*
* @brief   private (internal) function to :
*         get the Rx descriptors list private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] firstRxQueue             - first RX SDMA queue to get info
* @param[in] numOfRxQueues            - number of RX SDMA queues to get info
*
* @param[out] rxDescListPtr            - (pointer to) Rx descriptors list private info
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssPpConfigDevDbPrvNetInfInfoGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       firstRxQueue,
    IN GT_U32                       numOfRxQueues,
    OUT PRV_CPSS_RX_DESC_LIST_STC   *rxDescListPtr
);

/**
* @internal prvCpssPxHwUnitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] unitId                   - the ID of the address space unit.
*
* @param[out] errorPtr                 - (pointer to) indication that function did error.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32 prvCpssPxHwUnitBaseAddrGet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitId,
    OUT GT_BOOL                 *errorPtr
);

/**
* @internal prvCpssDefaultAddressUnusedSet function
* @endinternal
*
* @brief   This function set all the register addresses as 'unused'.
*
* @param[in,out] startPtr                 - pointer to start of register addresses
* @param[in] numOfRegs                - number of registers to initialize
* @param[in,out] startPtr                 - pointer to start of register addresses that where initialized
*/
void prvCpssDefaultAddressUnusedSet
(
    INOUT void   *startPtr,
    IN GT_U32   numOfRegs
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssHwInit_h */

