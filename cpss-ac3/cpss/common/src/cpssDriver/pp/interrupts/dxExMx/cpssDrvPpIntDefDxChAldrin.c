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
* @file cpssDrvPpIntDefDxChAldrin.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh Aldrin devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsAldrin.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>

#define PRV_ALDRIN_RX_DMA_0_ADDR(addr) (addr)
#define PRV_ALDRIN_RX_DMA_1_ADDR(addr) ((addr) + 0x51000000)
#define PRV_ALDRIN_RX_DMA_2_ADDR(addr) ((addr) + 0x52000000)
#define PRV_ALDRIN_RX_DMA_ADDR(addr, instance)  PRV_ALDRIN_RX_DMA_##instance##_ADDR(addr)

#define PRV_ALDRIN_TX_DMA_0_ADDR(addr) (addr)
#define PRV_ALDRIN_TX_DMA_1_ADDR(addr) ((addr) + 0x40000000)
#define PRV_ALDRIN_TX_DMA_2_ADDR(addr) ((addr) + 0x45000000)
#define PRV_ALDRIN_TX_DMA_ADDR(addr, instance)  PRV_ALDRIN_TX_DMA_##instance##_ADDR(addr)

#define PRV_ALDRIN_TX_FIFO_0_ADDR(addr) (addr)
#define PRV_ALDRIN_TX_FIFO_1_ADDR(addr) ((addr) + 0x40000000)
#define PRV_ALDRIN_TX_FIFO_2_ADDR(addr) ((addr) + 0x43000000)
#define PRV_ALDRIN_TX_FIFO_ADDR(addr, instance) PRV_ALDRIN_TX_FIFO_##instance##_ADDR(addr)

#define LAST_ENTRY_INT_CAUSE_CNS    0xFFFFFFFF


extern GT_STATUS prvCpssDrvBobKXlgGopExtUnitsIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);

/* BM General Cause Reg1 */
#define   bm_gen1_SUB_TREE_MAC(bit)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x16000300, 0x16000304,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                    \
        PRV_CPSS_ALDRIN_BM_GEN1_SUM_IN_PROG_CT_CLEAR_FIFO_OVERRUN_CORE_0_E, \
        PRV_CPSS_ALDRIN_BM_GEN1_SUM_GLOBAL_RX_FULL_E,                       \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

/* BM General Cause Reg2 */
#define   bm_gen2_SUB_TREE_MAC(bit)                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x16000308, 0x1600030C,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_BM_GEN2_SUM_CORE_0_RX_FULL_E,                           \
        PRV_CPSS_ALDRIN_BM_GEN2_SUM_CORE_5_RX_DMA_ALLOCATION_DELAYED_E,         \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   bm_SUB_TREE_MAC(bit)                                             \
    /* bmSumIntSum */                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x16000310, 0x16000314, \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_ALDRIN_BM_SUM_SUM_BM_GENERAL_REG1_SUMMARY_E,              \
        PRV_CPSS_ALDRIN_BM_SUM_SUM_BM_GENERAL_REG2_SUMMARY_E,              \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 2 , NULL, NULL},                    \
        /* BM General Cause Reg1 */                                        \
        bm_gen1_SUB_TREE_MAC(1),                                           \
        /* BM General Cause Reg2 */                                        \
        bm_gen2_SUB_TREE_MAC(2)

#define PRV_ALDRIN_RX_DMA_SCDMA_PORT(instance, port, name)                            \
               PRV_CPSS_ALDRIN_RX_DMA_##instance##_SCDMA_PORT_##port##_##name

#define PRV_CPSS_DXCH_UNIT_RXDMA0_E     PRV_CPSS_DXCH_UNIT_RXDMA_E

/* RXDMA : rxDmaScdma %n IntSum  */
#define ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port, instance)           \
             /*rxDmaSum0IntSum*/                                                           \
            {bitIndexInCaller, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##instance##_E, NULL,     \
                PRV_ALDRIN_RX_DMA_ADDR(0x17000E08, instance) + 4*(port),                   \
                PRV_ALDRIN_RX_DMA_ADDR(0x1700120C, instance) + 4*(port),                   \
                prvCpssDrvHwPpPortGroupIsrRead,                                            \
                prvCpssDrvHwPpPortGroupIsrWrite,                                           \
                PRV_ALDRIN_RX_DMA_SCDMA_PORT(instance, port, SUM_EOP_DESC_FIFO_OVERRUN_E), \
                PRV_ALDRIN_RX_DMA_SCDMA_PORT(instance, port, SUM_OS_DROP_E),               \
                FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define PRV_ALDRIN_RX_DMA(instance, name)                                             \
               PRV_CPSS_ALDRIN_RX_DMA_##instance##_##name
#define PRV_ALDRIN_RX_DMA0(instance, name)                                            \
               PRV_CPSS_ALDRIN_RX_DMA0_##instance##_##name

#if 0
/* RxDMA Interrupt 0 Cause. It describes local DMA 0.. 29 which are not used in Aldrin */
#define   rxDmaSum0_NODE_MAC(bit, instance)                              \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,            \
               PRV_ALDRIN_RX_DMA_ADDR(0x17000E00, instance),             \
               PRV_ALDRIN_RX_DMA_ADDR(0x17000E04, instance),             \
               prvCpssDrvHwPpPortGroupIsrRead,                           \
               prvCpssDrvHwPpPortGroupIsrWrite,                          \
               PRV_ALDRIN_RX_DMA0(instance, SUM_RX_DMA_RF_ERR_E),        \
               PRV_ALDRIN_RX_DMA0(instance, SUM_COMMON_BUFFER_EMPTY2_E), \
               FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}
#endif
#define rxDma_SUB_TREE_MAC(bit, instance)                                                               \
    /* rxDmaIntSum */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##instance##_E, NULL,                                       \
        PRV_ALDRIN_RX_DMA_ADDR(0x17001548, instance), PRV_ALDRIN_RX_DMA_ADDR(0x1700154c, instance),     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_ALDRIN_RX_DMA(instance, SUM_RX_DMA_INTERRUPT_SUMMARY0_E),                                   \
        PRV_ALDRIN_RX_DMA(instance, SUM_RX_DMA_INTERRUPT_SUMMARY3_E),                                   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 3 , NULL, NULL},                                                 \
        /* RxDMA Interrupt 0 Cause */                                                                   \
        /* rxDmaSum0_NODE_MAC(1, instance),  */                                                         \
        /*rxDmaSum1IntSum*/                                                                             \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_RX_DMA_ADDR(0x17001530, instance), PRV_ALDRIN_RX_DMA_ADDR(0x17001534, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_RX_DMA(instance, SUM1_SUM_SC_DMA56_S_SUM_E),                                     \
            PRV_ALDRIN_RX_DMA(instance, SUM1_SUM_SC_DMA59_S_SUM_E),                                     \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 4, NULL, NULL},                                              \
                                                 /* bit */ /*port*/                                     \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(27,      56,   instance),                           \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(28,      57,   instance),                           \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(29,      58,   instance),                           \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(30,      59,   instance),                           \
                                                                                                        \
         /*rxDmaSum2IntSum*/                                                                            \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_RX_DMA_ADDR(0x17001538, instance), PRV_ALDRIN_RX_DMA_ADDR(0x1700153C, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_RX_DMA(instance, SUM2_SUM_SC_DMA61_S_SUM_E),                                     \
            PRV_ALDRIN_RX_DMA(instance, SUM2_SUM_SC_DMA64_S_SUM_E),                                     \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 1 , NULL, NULL},                                             \
                                                 /* bit */ /*port*/                                     \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(4,       64,   instance),                          \
                                                                                                        \
         /*rxDmaSum3IntSum*/                                                                            \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_RX_DMA_ADDR(0x17001540, instance), PRV_ALDRIN_RX_DMA_ADDR(0x17001544, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_RX_DMA(instance, SUM3_SUM_SC_DMA65_S_SUM_E),                                     \
            PRV_ALDRIN_RX_DMA(instance, SUM3_SUM_SC_DMA72_S_SUM_E),                                     \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 8 , NULL, NULL},                                             \
                                                 /* bit */ /*port*/                                     \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(1,      65,    instance),                          \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(2,      66,    instance),                          \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(3,      67,    instance),                          \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(4,      68,    instance),                          \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(5,      69,    instance),                          \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(6,      70,    instance),                          \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(7,      71,    instance),                          \
            ALDRIN_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(8,      72,    instance)



#define PRV_ALDRIN_TX_DMA(instance, name)                   \
               PRV_CPSS_ALDRIN_TXDMA_##instance##_##name

#define PRV_CPSS_DXCH_UNIT_TXDMA0_E PRV_CPSS_DXCH_UNIT_TXDMA_E

#define   txDma_SUB_TREE_MAC(bit, instance)                                                             \
    /* txdmaSumIntSum */                                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                       \
        PRV_ALDRIN_TX_DMA_ADDR(0x26002040, instance), PRV_ALDRIN_TX_DMA_ADDR(0x26002044, instance),     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_ALDRIN_TX_DMA(instance, SUM_SUM_SC_DMA_ID_FIFO_OVERRUN_REG1_SUMMARY_E),                     \
        PRV_ALDRIN_TX_DMA(instance, SUM_SUM_RX_DMA_UPDATES_FIF_OS_OVERRUN_REG1_SUMMARY_E),              \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 8 , NULL, NULL},                                                 \
        /* txdmaFifoOverrun1IntSum */                                                                   \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_DMA_ADDR(0x26002000, instance), PRV_ALDRIN_TX_DMA_ADDR(0x26002004, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_TX_DMA(instance, FIFO_OVERRUN1_SUM_SC_DMA_0_ID_FIFO_OVERRUN_E),                  \
            PRV_ALDRIN_TX_DMA(instance, FIFO_OVERRUN1_SUM_SC_DMA_30_ID_FIFO_OVERRUN_E),                 \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                             \
        /* txdmaFifoOverrun2IntSum */                                                                   \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_DMA_ADDR(0x26002008, instance), PRV_ALDRIN_TX_DMA_ADDR(0x2600200C, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_TX_DMA(instance, FIFO_OVERRUN2_SUM_SC_DMA_31_ID_FIFO_OVERRUN_E),                 \
            PRV_ALDRIN_TX_DMA(instance, FIFO_OVERRUN2_SUM_SC_DMA_61_ID_FIFO_OVERRUN_E),                 \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                             \
        /* txdmaFifoOverrun3IntSum */                                                                   \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_DMA_ADDR(0x26002010, instance), PRV_ALDRIN_TX_DMA_ADDR(0x26002014, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_TX_DMA(instance, FIFO_OVERRUN3_SUM_SC_DMA_62_ID_FIFO_OVERRUN_E),                 \
            PRV_ALDRIN_TX_DMA(instance, FIFO_OVERRUN3_SUM_SC_DMA_73_ID_FIFO_OVERRUN_E),                 \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                             \
        /* txdmaWaitReply1IntSum */                                                                     \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_DMA_ADDR(0x26002020, instance), PRV_ALDRIN_TX_DMA_ADDR(0x26002024, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_TX_DMA(instance, WAIT_REPLY1_SUM_SC_DMA_0_WAITING_FOR_NEXT_REPLY_E),             \
            PRV_ALDRIN_TX_DMA(instance, WAIT_REPLY1_SUM_SC_DMA_30_WAITING_FOR_NEXT_REPLY_E),            \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                             \
        /* txdmaWaitReply2IntSum */                                                                     \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_DMA_ADDR(0x26002028, instance), PRV_ALDRIN_TX_DMA_ADDR(0x2600202C, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_TX_DMA(instance, WAIT_REPLY2_SUM_SC_DMA_31_WAITING_FOR_NEXT_REPLY_E),            \
            PRV_ALDRIN_TX_DMA(instance, WAIT_REPLY2_SUM_SC_DMA_61_WAITING_FOR_NEXT_REPLY_E),            \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                             \
        /* txdmaWaitReply3IntSum */                                                                     \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_DMA_ADDR(0x26002030, instance), PRV_ALDRIN_TX_DMA_ADDR(0x26002034, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_TX_DMA(instance, WAIT_REPLY3_SUM_SC_DMA_62_WAITING_FOR_NEXT_REPLY_E),            \
            PRV_ALDRIN_TX_DMA(instance, WAIT_REPLY3_SUM_SC_DMA_73_WAITING_FOR_NEXT_REPLY_E),            \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                             \
        /* txdmaGenenalIntSum */                                                                        \
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_DMA_ADDR(0x26002038, instance), PRV_ALDRIN_TX_DMA_ADDR(0x2600203c, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_TX_DMA(instance, GENERAL_SUM_REQUEST_EXECUTOR_NOT_READY_E),                      \
            PRV_ALDRIN_TX_DMA(instance, GENERAL_SUM_BANK_REQUESTS_ON_AIR_REACHED_MAXIMAL_THRESHOLD_E),  \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                             \
        /* txdmaRxDmaUpdate1IntSum */                                                                   \
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_DMA_ADDR(0x26002018, instance), PRV_ALDRIN_TX_DMA_ADDR(0x2600201c, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_ALDRIN_TX_DMA(instance, RX_DMA_UPDATE1_SUM_RX_DMA_0_UPDATES_FIF_OS_OVERRUN_E),          \
            PRV_ALDRIN_TX_DMA(instance, RX_DMA_UPDATE1_SUM_RX_DMA_1_UPDATES_FIF_OS_OVERRUN_E),          \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define PRV_ALDRIN_TX_FIFO(instance, name)                   \
               PRV_CPSS_ALDRIN_TX_FIFO_##instance##_##name

#define PRV_CPSS_DXCH_UNIT_TX_FIFO0_E   PRV_CPSS_DXCH_UNIT_TX_FIFO_E

#define   txFifo_SUB_TREE_MAC(bit, instance)                                                              \
    /* txFifoIntSum */                                                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##instance##_E, NULL,                                       \
        PRV_ALDRIN_TX_FIFO_ADDR(0x27000440, instance), PRV_ALDRIN_TX_FIFO_ADDR(0x27000444, instance),     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                   \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                  \
        PRV_ALDRIN_TX_FIFO(instance, SUM_TX_FIFO_GENERAL_REG1_SUMMARY_E),                                 \
        PRV_ALDRIN_TX_FIFO(instance, SUM_CT_BYTE_COUNT_ARRIVED_LATE_REG_3_SUMMARY_E),                     \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 4, NULL, NULL},                                                    \
                                                                                                          \
        /* txFifoGeberal1IntSum */                                                                        \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_FIFO_ADDR(0x27000400, instance), PRV_ALDRIN_TX_FIFO_ADDR(0x27000404, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            PRV_ALDRIN_TX_FIFO(instance, GENERAL1_SUM_LATENCY_FIFO_NOT_READY_E),                          \
            PRV_ALDRIN_TX_FIFO(instance, GENERAL1_SUM_CT_BC_IDDB_I_DS_RUN_OUT_E),                         \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                               \
        /* txFifoNotReady1IntSum */                                                                       \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_FIFO_ADDR(0x27000408, instance), PRV_ALDRIN_TX_FIFO_ADDR(0x2700040C, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            PRV_ALDRIN_TX_FIFO(instance, NOT_READY1_SUM_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E),              \
            PRV_ALDRIN_TX_FIFO(instance, NOT_READY1_SUM_SHIFTER_30_SOP_EOP_FIFO_NOT_READY_E),             \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                               \
        /* txFifoNotReady2IntSum */                                                                       \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_FIFO_ADDR(0x27000410, instance), PRV_ALDRIN_TX_FIFO_ADDR(0x27000414, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            PRV_ALDRIN_TX_FIFO(instance, NOT_READY2_SUM_SHIFTER_31_SOP_EOP_FIFO_NOT_READY_E),             \
            PRV_ALDRIN_TX_FIFO(instance, NOT_READY2_SUM_SHIFTER_61_SOP_EOP_FIFO_NOT_READY_E),             \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                                               \
        /* txFifoNotReady3IntSum */                                                                       \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##instance##_E, NULL,                                     \
            PRV_ALDRIN_TX_FIFO_ADDR(0x27000418, instance), PRV_ALDRIN_TX_FIFO_ADDR(0x2700041C, instance), \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            PRV_ALDRIN_TX_FIFO(instance, NOT_READY3_SUM_SHIFTER_62_SOP_EOP_FIFO_NOT_READY_E),             \
            PRV_ALDRIN_TX_FIFO(instance, NOT_READY3_SUM_SHIFTER_73_SOP_EOP_FIFO_NOT_READY_E),             \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   mppm_SUB_TREE_MAC(bit)                                                \
    /* mppmIntSum */                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x470021B0, 0x470021B4,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_MPPM_SUM_RX_CLIENT_0_S_SUM_E,                           \
        PRV_CPSS_ALDRIN_MPPM_SUM_ECC_ERROR_SUM_E,                               \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 14, NULL, NULL},                         \
                                                                                \
        /* mppmRxClient 0 IntSum */                                             \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002008, 0x4700202C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_0_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E, \
            PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_0_SUM_CREDITS_COUNTER_UNDERRUN_E,    \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmRxClient 1 IntSum */                                             \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x4700200c, 0x47002030,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_1_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E, \
            PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_1_SUM_CREDITS_COUNTER_UNDERRUN_E,    \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmRxClient 2 IntSum */                                             \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002010, 0x47002034,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_2_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E, \
            PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_2_SUM_CREDITS_COUNTER_UNDERRUN_E,    \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 0 IntSum */                                             \
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002080, 0x470020b0,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E,    \
            PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E,    \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 1 IntSum */                                             \
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002084, 0x470020b4,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_1_SUM_TX_RD_BURST_FIFO_OVERRUN_E,    \
            PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_1_SUM_TX_RD_BURST_FIFO_OVERRUN_E,    \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 2 IntSum */                                             \
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002088, 0x470020b8,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_2_SUM_TX_RD_BURST_FIFO_OVERRUN_E,    \
            PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_2_SUM_TX_RD_BURST_FIFO_OVERRUN_E,    \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
                                                                                \
        /* mppmBank 0 IntSum */                                                 \
        {15, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002120, 0x47002170, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            PRV_CPSS_ALDRIN_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 1 IntSum */                                                 \
        {16, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002124, 0x47002174, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            PRV_CPSS_ALDRIN_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 2 IntSum */                                                 \
        {17, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002128, 0x47002178, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_BANK_2_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            PRV_CPSS_ALDRIN_MPPM_BANK_2_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 3 IntSum */                                                 \
        {18, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x4700212c, 0x4700217c, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_BANK_3_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            PRV_CPSS_ALDRIN_MPPM_BANK_3_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 4 IntSum */                                                 \
        {19, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002130, 0x47002180, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_BANK_4_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            PRV_CPSS_ALDRIN_MPPM_BANK_4_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 5 IntSum */                                                 \
        {20, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002134, 0x47002184, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_BANK_5_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            PRV_CPSS_ALDRIN_MPPM_BANK_5_SUM_BANK_LATENCY_FIFO_OVERRUN_E,        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmEccIntSum */                                                     \
        {27, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x47002000, 0x47002004, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E,               \
            PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E,               \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmGenIntSum */                                                     \
        {27, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x470021c0, 0x470021c4, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_MPPM_GEN_SUM_MPPM_RF_ERR_E,                         \
            PRV_CPSS_ALDRIN_MPPM_GEN_SUM_MPPM_RF_ERR_E,                         \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}


#define   DataPathIntSum_SUB_TREE_MAC(bit)                             \
/* DataPathIntSum */                                                   \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000A4, 0x000000A8, \
    prvCpssDrvHwPpPortGroupIsrRead,                                    \
    prvCpssDrvHwPpPortGroupIsrWrite,                                   \
    PRV_CPSS_ALDRIN_DATA_PATH_S_SUMMARY_BM_E,                          \
    PRV_CPSS_ALDRIN_DATA_PATH_S_SUMMARY_TXFIFO_2_E,                    \
    FILLED_IN_RUNTIME_CNS, 0, 0x0, 11/*without bma*/ , NULL, NULL},    \
                                                                       \
    bm_SUB_TREE_MAC(1),                                                \
    mppm_SUB_TREE_MAC(3),                                              \
    rxDma_SUB_TREE_MAC(4, 0),                                          \
    txDma_SUB_TREE_MAC(5, 0),                                          \
    txFifo_SUB_TREE_MAC(6, 0),                                         \
    rxDma_SUB_TREE_MAC(7, 1) ,                                         \
    txDma_SUB_TREE_MAC(8, 1),                                          \
    txFifo_SUB_TREE_MAC(9, 1),                                         \
    rxDma_SUB_TREE_MAC(10, 2),                                         \
    txDma_SUB_TREE_MAC(11, 2),                                         \
    txFifo_SUB_TREE_MAC(12, 2)

#define   tti_SUB_TREE_MAC(bit)                                            \
    /* tti  */                                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TTI_E, NULL, 0x01000004, 0x01000008, \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_ALDRIN_TTI_SUM_CPU_ADDRESS_OUT_OF_RANGE_E,                \
        PRV_CPSS_ALDRIN_TTI_SUM_TTI1_ACCESS_DATA_ERROR_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   pcl_SUB_TREE_MAC(bit)                                            \
    /* pcl  */                                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_PCL_E, NULL, 0x02000004, 0x02000008, \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_ALDRIN_IPCL_SUM_MG_ADDR_OUT_OF_RANGE_E,                   \
        PRV_CPSS_ALDRIN_IPCL_SUM_LOOKUP2_FIFO_FULL_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   l2i_SUB_TREE_MAC(bit)                                            \
    /* L2 Bridge  */                                                       \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_L2I_E, NULL, 0x03002100, 0x03002104, \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_ALDRIN_BRIDGE_ADDRESS_OUT_OF_RANGE_E,                     \
        PRV_CPSS_ALDRIN_BRIDGE_UPDATE_SECURITY_BREACH_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS                                                      \
    (0xFFFFFFFF & ~                                                           \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_ALDRIN_FDB_AGE_VIA_TRIGGER_ENDED_E) | \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_ALDRIN_FDB_AU_PROC_COMPLETED_E)))

#define   fdb_SUB_TREE_MAC(bit)                                                                     \
    /* FDBIntSum - FDB Interrupt Cause,                                      */                     \
    /*   maskRcvIntrEn - both AUProcCompletedInt(bit of PRV_CPSS_ALDRIN_FDB_AU_PROC_COMPLETED_E) */ \
    /*   and AgeViaTriggerEnded(bit of PRV_CPSS_ALDRIN_FDB_AGE_VIA_TRIGGER_ENDED_E)*/               \
    /*       should never be masked to avoid missed events situation.  */                           \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_FDB_E, NULL, 0x040001b0, 0x040001b4,                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_ALDRIN_FDB_NUM_OF_HOP_EX_P_E,                                                      \
        PRV_CPSS_ALDRIN_FDB_ADDRESS_OUT_OF_RANGE_E,                                                 \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS, 0,NULL, NULL}

#define   tcam_SUB_TREE_MAC(bit)                                                \
    /* tcam  */                                                                 \
    {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TCAM_E, NULL, 0x05501004, 0x05501000,       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_TCAM_SUM_TCAM_LOGIC_SUMMARY_E,                            \
        PRV_CPSS_ALDRIN_TCAM_SUM_TCAM_LOGIC_SUMMARY_E,                            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1,NULL, NULL},                  \
        /* tcam Logic*/                                                         \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TCAM_E, NULL, 0x0550100c, 0x05501008,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_TCAM_LOGIC_SUM_CPU_ADDRESS_OUT_OF_RANGE_E,            \
            PRV_CPSS_ALDRIN_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   router_SUB_TREE_MAC(bit)                                              \
    /* router  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPVX_E, NULL, 0x06000970, 0x06000974,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_ROUTER_I_PV_X_BAD_ADDR_E,                                 \
        PRV_CPSS_ALDRIN_ROUTER_STG13_SIPSA_RETURNS_LFIFO_FULL_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define ETA_SUB_TREE_MAC(bit,instance)                                          \
    /* same address space as LPM */                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL,                              \
        0x58E00104 + (0x00100000 * (instance)),                                 \
        0x58E00100 + (0x00100000 * (instance)),                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_LPM_ETA_##instance##_ETA_WRONG_ADDRESS_E,                 \
        PRV_CPSS_ALDRIN_LPM_ETA_##instance##_ETA_WRONG_ADDRESS_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   lpm_SUB_TREE_MAC(bit)                                                 \
    /* LPM  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL, 0x58D00100, 0x58D00110,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_LPM_LPM_GENERAL_SUM_E,                                    \
        PRV_CPSS_ALDRIN_LPM_ETA_1_SUM_E,                                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                  \
        /* LPM general*/                                                        \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL, 0x58D00120, 0x58D00130,    \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_LPM_GENERAL_LPM_WRONG_ADDR_E,                         \
            PRV_CPSS_ALDRIN_LPM_GENERAL_AGING_CACHE_FULL_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* bit 2 , instance 0 */                                                \
        ETA_SUB_TREE_MAC(2,0),                                                  \
        /* bit 3 , instance 1 */                                                \
        ETA_SUB_TREE_MAC(3,1)

#define   iplr0_SUB_TREE_MAC(bit)                                               \
    /* iplr0  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPLR_E, NULL, 0x0B000200, 0x0B000204,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_IPLR0_DATA_ERROR_E,                                       \
        PRV_CPSS_ALDRIN_IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(index)                          \
            {(index+1), GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x0D010020  + (index * 0x10) , 0x0D010024  + (index * 0x10), \
                prvCpssDrvHwPpPortGroupIsrRead,                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                \
                PRV_CPSS_ALDRIN_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER_0_PKT_DROPED_E   + (index * 32),    \
                PRV_CPSS_ALDRIN_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER_15_PKT_DROPED_E  + (index * 32),    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eqSctRateLimiters_SUB_TREE_MAC(bit)                                   \
        /* SCT Rate Limiters */                                                 \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x0D010000, 0x0D010004,   \
             prvCpssDrvHwPpPortGroupIsrRead,                                    \
             prvCpssDrvHwPpPortGroupIsrWrite,                                   \
            PRV_CPSS_ALDRIN_SCT_RATE_LIMITERS_REG0_CPU_CODE_RATE_LIMITER_E,       \
            PRV_CPSS_ALDRIN_SCT_RATE_LIMITERS_REG15_CPU_CODE_RATE_LIMITER_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8,NULL, NULL},             \
                                                                                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(0),                         \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(1),                         \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(2),                         \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(3),                         \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(4),                         \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(5),                         \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(6),                         \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(7)

/* node for ingress STC interrupts leaf */
#define INGRESS_STC_NODE_FOR_16_PORTS_MAC(index) \
            {(index+1), GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x0d000620 + (index * 0x10) , 0x0d000624 + (index * 0x10), \
                prvCpssDrvHwPpPortGroupIsrRead,                                        \
                prvCpssDrvHwPpPortGroupIsrWrite,                                       \
                PRV_CPSS_ALDRIN_INGRESS_STC_PORT_0_INGRESS_SAMPLE_LOADED_E   + (index * 32),    \
                PRV_CPSS_ALDRIN_INGRESS_STC_PORT_15_INGRESS_SAMPLE_LOADED_E  + (index * 32),    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eqIngressStc_SUB_TREE_MAC(bit)                                               \
    /* Ingress STC  */                                                     \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x0d000600, 0x0d000604,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                   \
        prvCpssDrvHwPpPortGroupIsrWrite,                                  \
        PRV_CPSS_ALDRIN_ING_STC_SUM_REG0_INGRESS_SAMPLE_LOADED_E,         \
        PRV_CPSS_ALDRIN_ING_STC_SUM_REG15_INGRESS_SAMPLE_LOADED_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8,NULL, NULL},            \
                                                                           \
        INGRESS_STC_NODE_FOR_16_PORTS_MAC(0),                              \
        INGRESS_STC_NODE_FOR_16_PORTS_MAC(1),                              \
        INGRESS_STC_NODE_FOR_16_PORTS_MAC(2),                              \
        INGRESS_STC_NODE_FOR_16_PORTS_MAC(3),                              \
        INGRESS_STC_NODE_FOR_16_PORTS_MAC(4),                              \
        INGRESS_STC_NODE_FOR_16_PORTS_MAC(5),                              \
        INGRESS_STC_NODE_FOR_16_PORTS_MAC(6),                              \
        INGRESS_STC_NODE_FOR_16_PORTS_MAC(7)

#define   eq_SUB_TREE_MAC(bit)                                                  \
    /* Pre-Egress Interrupt Summary Cause */                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x0D000058, 0x0D00005C,       \
         prvCpssDrvHwPpPortGroupIsrRead,                                        \
         prvCpssDrvHwPpPortGroupIsrWrite,                                       \
        PRV_CPSS_ALDRIN_EQ_INGRESS_STC_E,                                         \
        PRV_CPSS_ALDRIN_EQ_CRITICAL_ECC_ERROR_E,                                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                  \
                                                                                \
        eqIngressStc_SUB_TREE_MAC(1),                                           \
        eqSctRateLimiters_SUB_TREE_MAC(2),                                      \
                                                                                \
        /* Critical ECC Error Int  */                                           \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x0D0000A0 , 0x0D0000A4 ,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_CRITICAL_ECC_ERROR_CLEAR_DESC_CRITICAL_ECC_ONE_ERROR_E,     \
            PRV_CPSS_ALDRIN_CRITICAL_ECC_ERROR_INCREMENT_DESC_CRITICAL_ECC_TWO_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eplr_SUB_TREE_MAC(bit)                                                \
    /* eplr  */                                                                 \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EPLR_E, NULL, 0x0E000200, 0x0E000204,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_EPLR_SUM_DATA_ERROR_E,                                    \
        PRV_CPSS_ALDRIN_EPLR_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ha_SUB_TREE_MAC(bit)                                                  \
   /* HA  */                                                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_HA_E, NULL, 0x0F000300, 0x0F000304,       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_HA_SUM_REGFILE_ADDRESS_ERROR_E,                           \
        PRV_CPSS_ALDRIN_HA_SUM_OVERSIZE_HEADER_SIZE_E,                            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_SUB_TREE_MAC(bit)                                                \
    /* EPCL  */                                                                 \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EPCL_E, NULL, 0x14000010, 0x14000014,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_EPCL_SUM_REGFILE_ADDRESS_ERROR_E,                         \
        PRV_CPSS_ALDRIN_EPCL_SUM_REGFILE_ADDRESS_ERROR_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define   ermrk_SUB_TREE_MAC(bit)                                               \
    /* ERMRK  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_ERMRK_E, NULL, 0x15000004, 0x15000008,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_ERMRK_SUM_REGFILE_ADDRESS_ERROR_E,                        \
        PRV_CPSS_ALDRIN_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE1_FULL_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eoam_SUB_TREE_MAC(bit)                                                \
    /* egress OAM  */                                                           \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EOAM_E, NULL, 0x1E0000F0, 0x1E0000F4,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_EGR_OAM_SUM_KEEP_ALIVE_AGING_E,                           \
        PRV_CPSS_ALDRIN_EGR_OAM_SUM_TX_PERIOD_E,                                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ioam_SUB_TREE_MAC(bit)                                                \
    /* ingress OAM  */                                                          \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IOAM_E, NULL, 0x1C0000F0, 0x1C0000F4,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_INGR_OAM_SUM_KEEP_ALIVE_AGING_E,                          \
        PRV_CPSS_ALDRIN_INGR_OAM_SUM_TX_PERIOD_E,                                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   mll_SUB_TREE_MAC(bit)                                                 \
    /* MLL  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_MLL_E, NULL, 0x1D000030, 0x1D000034,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_MLL_SUM_INVALID_IP_MLL_ACCESS_E,                          \
        PRV_CPSS_ALDRIN_MLL_SUM_FILE_ECC_2_ERROR_E,                              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   cnc_SUB_TREE_MAC(bit,instance)                                        \
    /* CNC-0  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,                 \
        0x2E000100 + ((instance)*0x01000000),                                   \
        0x2E000104 + ((instance)*0x01000000),                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_CNC##instance##_SUM_WRAPAROUND_FUNC_SUM_E,                \
        PRV_CPSS_ALDRIN_CNC##instance##_SUM_MISC_FUNC_SUM_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                  \
                                                                                \
        /* WraparoundFuncInterruptSum  */                                       \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x2E000190 + ((instance)*0x01000000),                               \
            0x2E0001A4 + ((instance)*0x01000000),                               \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_CNC##instance##_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,   \
            PRV_CPSS_ALDRIN_CNC##instance##_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* RateLimitFuncInterruptSum  */                                        \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x2E0001B8 + ((instance)*0x01000000),                               \
            0x2E0001CC + ((instance)*0x01000000),                               \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_CNC##instance##_RATE_LIMIT_SUM_BLOCK0_RATE_LIMIT_FIFO_DROP_E,  \
            PRV_CPSS_ALDRIN_CNC##instance##_RATE_LIMIT_SUM_BLOCK15_RATE_LIMIT_FIFO_DROP_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* MiscFuncInterruptSum  */                                             \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x2E0001E0 + ((instance)*0x01000000),                               \
            0x2E0001E4 + ((instance)*0x01000000),                               \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_CNC##instance##_MISC_SUM_DUMP_FINISHED_E,             \
            PRV_CPSS_ALDRIN_CNC##instance##_MISC_SUM_CNC_UPDATE_LOST_E,           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define   iplr1_SUB_TREE_MAC(bit)                                               \
    /* iplr1  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPLR_1_E, NULL, 0x20000200, 0x20000204,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_IPLR1_SUM_DATA_ERROR_E,                                   \
        PRV_CPSS_ALDRIN_IPLR1_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eft_SUB_TREE_MAC(bit)                                                 \
    /* eft  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_EFT_E, NULL, 0x350010A0, 0x350010B0,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_EFT_SUM_EGRESS_WRONG_ADDR_E,                              \
        PRV_CPSS_ALDRIN_EFT_SUM_INC_BUS_IS_TOO_SMALL_INT_E,                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   sht_SUB_TREE_MAC(bit)                                                 \
    /* sht  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_SHT_E, NULL, 0x4E020010, 0x4E020020,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_SHT_SUM_SHT_WRONG_ADDR_E,                                 \
        PRV_CPSS_ALDRIN_SHT_SUM_SPANNING_TREE_STATE_TABLE_WRONG_ADDRESS_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   qag_SUB_TREE_MAC(bit)                                                 \
    /* QAG  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_QAG_E, NULL, 0x3BF00010, 0x3BF00020,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_QAG_SUM_QAG_WRONG_ADDR_E,                                 \
        PRV_CPSS_ALDRIN_QAG_SUM_EPORT_ATTRIBUTES_TABLE_WRONG_ADDRESS_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* TXQ queue: low_crossing for port - cause */
#define ALDRIN_TXQ_QUEUE_LOW_CROSSING_PORT_INTERRUPT_CAUSE_MAC(port) \
        (0x3C090080 + 0x4*(port))

/* TXQ queue: low_crossing for port - mask */
#define ALDRIN_TXQ_QUEUE_LOW_CROSSING_PORT_INTERRUPT_MASK_MAC(port) \
        (0x3C090280 + 0x4*(port))

/* TXQ queue: low_crossing for port */
#define ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port)   \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, \
              ALDRIN_TXQ_QUEUE_LOW_CROSSING_PORT_INTERRUPT_CAUSE_MAC(port) , \
              ALDRIN_TXQ_QUEUE_LOW_CROSSING_PORT_INTERRUPT_MASK_MAC(port)  , \
              prvCpssDrvHwPpPortGroupIsrRead,                    \
              prvCpssDrvHwPpPortGroupIsrWrite,                   \
              PRV_CPSS_ALDRIN_LOW_CROSSING_PORT_##port##_LOW_THRESHOLD_CROSSED_FOR_PORT_QUEUE0_E, \
              PRV_CPSS_ALDRIN_LOW_CROSSING_PORT_##port##_LOW_THRESHOLD_CROSSED_FOR_PORT_QUEUE7_E,\
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* TXQ queue: high_crossing for port - cause */
#define ALDRIN_TXQ_QUEUE_HIGH_CROSSING_PORT_INTERRUPT_CAUSE_MAC(port) \
        (0x3C090480 + 0x4*(port))

/* TXQ queue: high_crossing for port - mask */
#define ALDRIN_TXQ_QUEUE_HIGH_CROSSING_PORT_INTERRUPT_MASK_MAC(port) \
        (0x3C090680 + 0x4*(port))

/* TXQ queue: high_crossing for port */
#define ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port)   \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, \
              ALDRIN_TXQ_QUEUE_HIGH_CROSSING_PORT_INTERRUPT_CAUSE_MAC(port) , \
              ALDRIN_TXQ_QUEUE_HIGH_CROSSING_PORT_INTERRUPT_MASK_MAC(port)  , \
              prvCpssDrvHwPpPortGroupIsrRead,                    \
              prvCpssDrvHwPpPortGroupIsrWrite,                   \
              PRV_CPSS_ALDRIN_HIGH_CROSSING_PORT_##port##_HIGH_THRESHOLD_CROSSED_FOR_PORT_QUEUE0_E, \
              PRV_CPSS_ALDRIN_HIGH_CROSSING_PORT_##port##_HIGH_THRESHOLD_CROSSED_FOR_PORT_QUEUE7_E,\
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqQueue_SUB_TREE_MAC(bit)                                            \
    /* TXQ-queue  */                                                            \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090000, 0x3C090004,\
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN_TXQ_SUM_GENERAL1_SUM_E,                                   \
        PRV_CPSS_ALDRIN_TXQ_SUM_PORT_BUFF_FULL2_SUM_E,                                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 13,NULL, NULL},                 \
                                                                                \
        /* TXQ-queue : txqGenIntSum  */                                         \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090008, 0x3C09000c,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_TXQ_GEN_SUM_GLOBAL_DESC_FULL_E,                       \
            PRV_CPSS_ALDRIN_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E,          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortDesc0IntSum  */                                   \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090040, 0x3C090044,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_0_E,                 \
            PRV_CPSS_ALDRIN_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_30_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortDesc1IntSum  */                                   \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090048, 0x3C09004C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_31_E,                \
            PRV_CPSS_ALDRIN_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_61_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortDesc2IntSum  */                                   \
        {4, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090050, 0x3C090054,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_62_E,                \
            PRV_CPSS_ALDRIN_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_71_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff0IntSum  */                                   \
        {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090060, 0x3C090064,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_TXQ_PORT_BUFF0_SUM_BUFF_FULL_PORT0_E,                 \
            PRV_CPSS_ALDRIN_TXQ_PORT_BUFF0_SUM_BUFF_FULL_PORT30_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff1IntSum  */                                   \
        {6, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090068, 0x3C09006C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_TXQ_PORT_BUFF1_SUM_BUFF_FULL_PORT31_E,                \
            PRV_CPSS_ALDRIN_TXQ_PORT_BUFF1_SUM_BUFF_FULL_PORT61_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff2IntSum  */                                   \
        {7, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090070, 0x3C090074,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN_TXQ_PORT_BUFF2_SUM_BUFF_FULL_PORT62_E,                \
            PRV_CPSS_ALDRIN_TXQ_PORT_BUFF2_SUM_BUFF_FULL_PORT71_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : Low_Crossing_Int_Sum_Ports_30_to_0*/                         \
        {8, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090880, 0x3C090884,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN_LOW_CROSSING_SUM_PORTS_30_TO_0_LOW_THRESHOLD_CROSSED_PORT0_SUM_E,  \
            PRV_CPSS_ALDRIN_LOW_CROSSING_SUM_PORTS_30_TO_0_LOW_THRESHOLD_CROSSED_PORT30_SUM_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 31,NULL, NULL},                             \
                                                                  /* bit */ /*port*/            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 1 ,   0  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 2 ,   1  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 3 ,   2  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 4 ,   3  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 5 ,   4  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 6 ,   5  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 7 ,   6  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 8 ,   7  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 9 ,   8  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 10,   9  ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 11,   10 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 12,   11 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 13,   12 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 14,   13 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 15,   14 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 16,   15 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 17,   16 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 18,   17 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 19,   18 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 20,   19 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 21,   20 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 22,   21 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 23,   22 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 24,   23 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 25,   24 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 26,   25 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 27,   26 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 28,   27 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 29,   28 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 30,   29 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 31,   30 ),            \
        /* TXQ-queue : Low_Crossing_Int_Sum_Ports_61_to_31*/                                    \
        {9, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090888, 0x3C09088C,              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
            PRV_CPSS_ALDRIN_LOW_CROSSING_SUM_PORTS_61_TO_31_LOW_THRESHOLD_CROSSED_PORT31_SUM_E,\
            PRV_CPSS_ALDRIN_LOW_CROSSING_SUM_PORTS_61_TO_31_LOW_THRESHOLD_CROSSED_PORT61_SUM_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 23,NULL, NULL},                             \
                                                                  /* bit */ /*port*/            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 1 ,   31 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 2 ,   32 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 3 ,   33 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 4 ,   34 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 5 ,   35 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 6 ,   36 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 7 ,   37 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 8 ,   38 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 9 ,   39 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 10,   40 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 11,   41 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 12,   42 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 13,   43 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 14,   44 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 15,   45 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 16,   46 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 17,   47 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 26,   56 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 27,   57 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 28,   58 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 29,   59 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 30,   60 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 31,   61 ),            \
                                                                                                \
        /* TXQ-queue : Low_Crossing_Int_Sum_Ports_71_to_62*/                                    \
        {10, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090890, 0x3C090894,             \
            prvCpssDrvHwPpPortGroupIsrRead,                                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
            PRV_CPSS_ALDRIN_LOW_CROSSING_SUM_PORTS_71_TO_62_LOW_THRESHOLD_CROSSED_PORT62_SUM_E,\
            PRV_CPSS_ALDRIN_LOW_CROSSING_SUM_PORTS_71_TO_62_LOW_THRESHOLD_CROSSED_PORT71_SUM_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8,NULL, NULL},                             \
                                                                  /* bit */ /*port*/            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 3 ,   64 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 4 ,   65 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 5 ,   66 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 6 ,   67 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 7 ,   68 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 8 ,   69 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 9 ,   70 ),            \
                ALDRIN_TXQ_QUEUE_LOW_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 10,   71 ),            \
                                                                                                \
        /* TXQ-queue : High_Crossing_Int_Sum_Ports_30_to_0*/                                    \
        {11, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C090898, 0x3C09089c,             \
            prvCpssDrvHwPpPortGroupIsrRead,                                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
            PRV_CPSS_ALDRIN_HIGH_CROSSING_SUM_PORTS_30_TO_0_HIGH_THRESHOLD_CROSSED_PORT0_SUM_E,  \
            PRV_CPSS_ALDRIN_HIGH_CROSSING_SUM_PORTS_30_TO_0_HIGH_THRESHOLD_CROSSED_PORT30_SUM_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 31,NULL, NULL},                               \
                                                                  /* bit */ /*port*/              \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 1 ,   0  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 2 ,   1  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 3 ,   2  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 4 ,   3  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 5 ,   4  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 6 ,   5  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 7 ,   6  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 8 ,   7  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 9 ,   8  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 10,   9  ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 11,   10 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 12,   11 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 13,   12 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 14,   13 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 15,   14 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 16,   15 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 17,   16 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 18,   17 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 19,   18 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 20,   19 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 21,   20 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 22,   21 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 23,   22 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 24,   23 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 25,   24 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 26,   25 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 27,   26 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 28,   27 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 29,   28 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 30,   29 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 31,   30 ),             \
        /* TXQ-queue : High_Crossing_Int_Sum_Ports_61_to_31*/                                     \
        {12, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C0908a0, 0x3C0908a4,               \
            prvCpssDrvHwPpPortGroupIsrRead,                                                       \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                      \
            PRV_CPSS_ALDRIN_HIGH_CROSSING_SUM_PORTS_61_TO_31_HIGH_THRESHOLD_CROSSED_PORT31_SUM_E,\
            PRV_CPSS_ALDRIN_HIGH_CROSSING_SUM_PORTS_61_TO_31_HIGH_THRESHOLD_CROSSED_PORT61_SUM_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 23,NULL, NULL},                               \
                                                                  /* bit */ /*port*/              \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 1 ,   31 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 2 ,   32 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 3 ,   33 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 4 ,   34 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 5 ,   35 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 6 ,   36 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 7 ,   37 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 8 ,   38 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 9 ,   39 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 10,   40 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 11,   41 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 12,   42 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 13,   43 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 14,   44 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 15,   45 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 16,   46 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 17,   47 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 26,   56 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 27,   57 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 28,   58 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 29,   59 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 30,   60 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 31,   61 ),             \
        /* TXQ-queue : High_Crossing_Int_Sum_Ports_71_to_62*/                                     \
        {13, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x3C0908a8, 0x3C0908ac,               \
            prvCpssDrvHwPpPortGroupIsrRead,                                                       \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                      \
            PRV_CPSS_ALDRIN_HIGH_CROSSING_SUM_PORTS_71_TO_62_HIGH_THRESHOLD_CROSSED_PORT62_SUM_E,\
            PRV_CPSS_ALDRIN_HIGH_CROSSING_SUM_PORTS_71_TO_62_HIGH_THRESHOLD_CROSSED_PORT71_SUM_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8,NULL, NULL},                               \
                                                                  /* bit */ /*port*/              \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 3 ,   64 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 4 ,   65 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 5 ,   66 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 6 ,   67 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 7 ,   68 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 8 ,   69 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 9 ,   70 ),             \
                ALDRIN_TXQ_QUEUE_HIGH_CROSSING_FOR_PORT_INT_ELEMENT_MAC( 10,   71 )

#define   txqLl_NODE_MAC(bit)                                               \
    /* TXQ-LL  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x3D0A0508, 0x3D0A050c,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                            \
        prvCpssDrvHwPpPortGroupIsrWrite,                                           \
        PRV_CPSS_ALDRIN_LL_SUM_ID_FIFO_OVERRUN_E,                                 \
        PRV_CPSS_ALDRIN_LL_SUM_LL_WRONG_ADDR_E,                                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLL_single_ECC_Error_SUB_TREE_MAC(bit)                             \
    /* LL_ECC_Error */                                            \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x3d0a0510, 0x3d0a0514, \
        prvCpssDrvHwPpPortGroupIsrRead,                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                       \
        PRV_CPSS_ALDRIN_LL_SINGLE_ECC_ERROR_ECC_SINGLE_ERROR_BANK_0_E,     \
        PRV_CPSS_ALDRIN_LL_SINGLE_ECC_ERROR_ECC_SINGLE_ERROR_BANK_15_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLL_double_ECC_Error_SUB_TREE_MAC(bit)                             \
    /* LL_ECC_Error */                                            \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x3d0a0518, 0x3d0a051c, \
        prvCpssDrvHwPpPortGroupIsrRead,                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                       \
        PRV_CPSS_ALDRIN_LL_DOUBLE_ECC_ERROR_ECC_DOUBLE_ERROR_BANK_0_E,     \
        PRV_CPSS_ALDRIN_LL_DOUBLE_ECC_ERROR_ECC_DOUBLE_ERROR_BANK_15_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLl_SUB_TREE_MAC(bit)                                               \
    /* TXQ-LL  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x3D0A0520, 0x3D0A0524,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                            \
        prvCpssDrvHwPpPortGroupIsrWrite,                                           \
        PRV_CPSS_ALDRIN_LL_SUMMARY_SUM_LL_SUM_E,                                  \
        PRV_CPSS_ALDRIN_LL_SUMMARY_SUM_ECC_DOUBLE_SUMMARY_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                     \
        txqLl_NODE_MAC(1),                                                      \
        txqLL_single_ECC_Error_SUB_TREE_MAC(2),                                 \
        txqLL_double_ECC_Error_SUB_TREE_MAC(3)

#define   txqPfc_SUB_TREE_MAC(bit)                                          \
    /* TXQ-PFC  */                                                                 \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x3E001300, 0x3E001304,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                            \
        prvCpssDrvHwPpPortGroupIsrWrite,                                           \
        PRV_CPSS_ALDRIN_PFC_SUM_PFC_PARITY_SUM_E,                              \
        PRV_CPSS_ALDRIN_PFC_SUM_PFC_MSGS_SETS_OVERRUNS_EGR3_0_SUM_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6,NULL, NULL},                     \
        /*pfcParityIntSum*/                                                        \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x3e001310, 0x3e001314,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                           \
            PRV_CPSS_ALDRIN_PFC_PARITY_SUM_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E,  \
            PRV_CPSS_ALDRIN_PFC_PARITY_SUM_PFC_WRONG_ADDR_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                     \
        /*PfcCntrOverUnderFlowIntSum*/                                                 \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x3e001320, 0x3e001324,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                           \
            PRV_CPSS_ALDRIN_PFC_CNTR_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_OVERFLOW_E,  \
            PRV_CPSS_ALDRIN_PFC_CNTR_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_UNDERFLOW_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                     \
        /*PfcPipe_0_GlobalOverUnderflowIntSum*/                                      \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x3e001370, 0x3e001390,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                           \
            PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC0_UNDERFLOW_E,  \
            PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC7_UNDERFLOW_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                     \
        /*PfcCcOverUnderFlowIntSum*/                                                 \
        {4, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x3e001328, 0x3e00132c,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                           \
            PRV_CPSS_ALDRIN_PFC_CC_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_CC_OVERFLOW_E,  \
            PRV_CPSS_ALDRIN_PFC_CC_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_CC_UNDERFLOW_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                     \
        /*PfcIndFifoOverrunsIntSum*/                                                 \
        {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x3e0013b0, 0x3e0013b4,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                           \
            PRV_CPSS_ALDRIN_PFC_IND_FIFO_OVERRUNS_SUM_PFC_PORT_GROUP0_FC_IND_FIFO_OVERRUN_E,  \
            PRV_CPSS_ALDRIN_PFC_IND_FIFO_OVERRUNS_SUM_PFC_PORT_GROUP0_FC_IND_FIFO_OVERRUN_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                     \
        /*PfcIndFifoOverrunsIntSum*/                                                 \
        {6, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x3e0013b8, 0x3e0013bC,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                           \
            PRV_CPSS_ALDRIN_PFC_MSGS_SETS_OVERRUNS_EGR3_0_SUM_PFC_PORT_GROUP0_QCN0_MSG_OVERRUN_E,  \
            PRV_CPSS_ALDRIN_PFC_MSGS_SETS_OVERRUNS_EGR3_0_SUM_PFC_PORT_GROUP0_EGR0_MSG_OVERRUN_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqQcn_SUB_TREE_MAC(bit)                                               \
    /* TXQ-QCN  */                                                                 \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QCN_E, NULL, 0x3F000100, 0x3F000110,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                            \
        prvCpssDrvHwPpPortGroupIsrWrite,                                           \
        PRV_CPSS_ALDRIN_QCN_SUM_CN_BUFFER_FIFO_OVERRUN_E,                         \
        PRV_CPSS_ALDRIN_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for egress STC interrupts leaf */
#define EGRESS_STC_NODE_FOR_30_PORTS_MAC(index) \
            {(index+6), GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_DQ_E, NULL, 0x40000630 + (index * 4) , 0x40000640 + (index * 4), \
                prvCpssDrvHwPpPortGroupIsrRead,                                        \
                prvCpssDrvHwPpPortGroupIsrWrite,                                       \
                PRV_CPSS_ALDRIN_TXQ_DQ_EGR_STC_PORT_0_E   + (index * 32),    \
                PRV_CPSS_ALDRIN_TXQ_DQ_EGR_STC_PORT_30_E  + (index * 32),    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqDq_SUB_TREE_MAC(bit)                                             \
    /*  TxQ DQ Interrupt Summary Cause  */                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_DQ_E, NULL, 0x40000600, 0x40000604, \
        prvCpssDrvHwPpPortGroupIsrRead,                                       \
        prvCpssDrvHwPpPortGroupIsrWrite,                                      \
        PRV_CPSS_ALDRIN_TXQ_DQ_SUM_FLUSH_DONE0_SUM_E,                         \
        PRV_CPSS_ALDRIN_TXQ_DQ_SUM_GENERAL_SUM_E,                             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                \
                                                                              \
        EGRESS_STC_NODE_FOR_30_PORTS_MAC(0), /*bit 6 */                       \
        EGRESS_STC_NODE_FOR_30_PORTS_MAC(1), /*bit 7 */                       \
        EGRESS_STC_NODE_FOR_30_PORTS_MAC(2) /*bit 8 */

#define   FuncUnitsIntsSum_SUB_TREE_MAC(bit)                             \
/* FuncUnitsIntsSum Interrupt Cause */                                   \
{bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000003f8, 0x000003FC,   \
    prvCpssDrvHwPpPortGroupIsrRead,                                      \
    prvCpssDrvHwPpPortGroupIsrWrite,                                     \
    PRV_CPSS_ALDRIN_FUNCTIONAL_S_SUMMARY_FUNCTIONAL_INTERRUPT_SUMMARY_E, \
    PRV_CPSS_ALDRIN_FUNCTIONAL_S_SUMMARY_FUNC_UNITS_SUM_29_E,            \
    FILLED_IN_RUNTIME_CNS, 0, 0x0, 27 , NULL, NULL},                     \
                                                                         \
    tti_SUB_TREE_MAC(1),                                                 \
    pcl_SUB_TREE_MAC(2),                                                 \
    l2i_SUB_TREE_MAC(3),                                                 \
    fdb_SUB_TREE_MAC(4),                                                 \
    tcam_SUB_TREE_MAC(5),                                                \
    router_SUB_TREE_MAC(6),                                              \
    lpm_SUB_TREE_MAC(7),                                                 \
    iplr0_SUB_TREE_MAC(8),                                               \
    eq_SUB_TREE_MAC(9),                                                  \
    eplr_SUB_TREE_MAC(10),                                               \
    ha_SUB_TREE_MAC(11),                                                 \
    epcl_SUB_TREE_MAC(12),                                               \
    ermrk_SUB_TREE_MAC(13),                                              \
    eoam_SUB_TREE_MAC(14),                                               \
    ioam_SUB_TREE_MAC(15),                                               \
    mll_SUB_TREE_MAC(16),                                                \
    cnc_SUB_TREE_MAC(17,0),/* cnc0 */                                    \
    cnc_SUB_TREE_MAC(18,1),/* cnc1 */                                    \
    iplr1_SUB_TREE_MAC(19),                                              \
    eft_SUB_TREE_MAC(20),                                                \
    sht_SUB_TREE_MAC(21),                                                \
    qag_SUB_TREE_MAC(22),                                                \
    txqQueue_SUB_TREE_MAC(23),                                           \
    txqLl_SUB_TREE_MAC(24),                                              \
    txqPfc_SUB_TREE_MAC(25),                                             \
    txqQcn_SUB_TREE_MAC(26),                                             \
    txqDq_SUB_TREE_MAC(27)

/* ports: 0..32 */
#define ALDRIN_GIGA_PORT_BASE_MAC(port) \
        (0x10000000 + 0x1000*(port))

/* ports: 0..31 */
#define ALDRIN_XLG_PORT_BASE_MAC(port)  \
        (0x100C0000 + 0x1000*(port))

/* XLG summary - cause */
#define ALDRIN_XLG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN_XLG_PORT_BASE_MAC(port) + (0x58))

/* XLG summary - mask */
#define ALDRIN_XLG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (ALDRIN_XLG_PORT_BASE_MAC(port) + (0x5c))

/* XLG - cause */
#define ALDRIN_XLG_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN_XLG_PORT_BASE_MAC(port) + (0x14))

/* XLG - mask */
#define ALDRIN_XLG_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN_XLG_PORT_BASE_MAC(port) + (0x18))

/* gig summary - cause */
#define ALDRIN_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN_GIGA_PORT_BASE_MAC(port) + (0xA0))

/* gig summary - mask */
#define ALDRIN_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (ALDRIN_GIGA_PORT_BASE_MAC(port) + (0xA4))

/* gig - cause */
#define ALDRIN_GIGA_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN_GIGA_PORT_BASE_MAC(port) + (0x20))

/* gig - mask */
#define ALDRIN_GIGA_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN_GIGA_PORT_BASE_MAC(port) + (0x24))

/* ports 0..32 */
#define ALDRIN_PTP_PORT_BASE_MAC(port)                 \
        (0x10180800 + 0x1000*(port))

/* ptp - cause */
#define ALDRIN_PTP_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN_PTP_PORT_BASE_MAC(port) + (0x00))

/* ptp - mask */
#define ALDRIN_PTP_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN_PTP_PORT_BASE_MAC(port) + (0x04))

/* PTP interrupts for giga/XLG port */
#define ALDRIN_PTP_FOR_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port)       \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,  \
              ALDRIN_PTP_PORT_INTERRUPT_CAUSE_MAC(port) ,                \
              ALDRIN_PTP_PORT_INTERRUPT_MASK_MAC(port)  ,                \
              prvCpssDrvHwPpPortGroupIsrRead,                            \
              prvCpssDrvHwPpPortGroupIsrWrite,                           \
              PRV_CPSS_ALDRIN_PTP_PORT_##port##_PTP_RX_FIFO_FULL_E,      \
              PRV_CPSS_ALDRIN_PTP_PORT_##port##_RX_SOP_IN_UNDER_RESET_E, \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* summary of Giga mac that called from MG */
#define ALDRIN_GIGA_PORT_SUMMARY_FROM_MG_INT_SUB_TREE_MAC(bitIndexInCaller,port)  \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,       \
                ALDRIN_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) ,              \
                ALDRIN_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port),                \
                prvCpssDrvHwPpPortGroupIsrRead,                                   \
                prvCpssDrvHwPpPortGroupIsrWrite,                                  \
                PRV_CPSS_ALDRIN_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E,            \
                PRV_CPSS_ALDRIN_GIGA_PORT_##port##_SUM_PTP_SUM_E,                 \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},           \
                    /* interrupts of the giga mac */                              \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                 \
                        ALDRIN_GIGA_PORT_INTERRUPT_CAUSE_MAC(port) ,              \
                        ALDRIN_GIGA_PORT_INTERRUPT_MASK_MAC(port),                \
                     prvCpssDrvHwPpPortGroupIsrRead,                              \
                     prvCpssDrvHwPpPortGroupIsrWrite,                             \
                     PRV_CPSS_ALDRIN_GIGA_PORT_##port##_LINK_STATUS_CHANGED_E,    \
                     PRV_CPSS_ALDRIN_GIGA_PORT_##port##_MIB_COUNTER_WRAPAROUND_E, \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},       \
                     /*bit 2*/                                                    \
                     ALDRIN_PTP_FOR_PORT_INT_ELEMENT_MAC(2,port)

#define   FuncUnits1IntsSum_SUB_TREE_MAC(bit)                              \
/* FuncUnits1IntsSum  */                                                   \
{bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000003F4, 0x000003F0,      \
    prvCpssDrvHwPpPortGroupIsrRead,                                        \
    prvCpssDrvHwPpPortGroupIsrWrite,                                       \
    PRV_CPSS_ALDRIN_FUNCTIONAL1_S_SUMMARY_POE2MSYS_E,                      \
    PRV_CPSS_ALDRIN_FUNCTIONAL1_S_SUMMARY_PEX_E,                           \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 7, NULL, NULL},                \
                                                                           \
    /* POE Interrupt Cause */                                              \
    {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_POE_E, NULL, 0x61000004, 0x61000008,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_ALDRIN_POE_INT_CAUSE_E,                                   \
        PRV_CPSS_ALDRIN_POE_WRONG_ADDR_INT_E,                              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},            \
                                                                           \
    /* 32 GPIO_32_63_Interrupt Cause */                                    \
    {4, GT_TRUE,32, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018150, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x0001815C), \
        prvCpssDrvHwPpPortGroupReadInternalPciReg,                         \
        prvCpssDrvHwPpPortGroupWriteInternalPciReg,                        \
        PRV_CPSS_ALDRIN_GPIO_1_32_E,                                       \
        PRV_CPSS_ALDRIN_GPIO_1_32_E,                                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},            \
                                                                           \
    /* 24..31, GPIO_0_31_Interrupt Cause */                                \
    {5, GT_TRUE,24, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018110, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x0001811C), \
        prvCpssDrvHwPpPortGroupReadInternalPciReg,                         \
        prvCpssDrvHwPpPortGroupWriteInternalPciReg,                        \
        PRV_CPSS_ALDRIN_GPIO_0_24_E,                                       \
        PRV_CPSS_ALDRIN_GPIO_0_31_E,                                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},            \
                                                                           \
    /* 16..23, GPIO_0_31_Interrupt Cause */                                \
    {6, GT_TRUE,16, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018110, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x0001811C), \
        prvCpssDrvHwPpPortGroupReadInternalPciReg,                         \
        prvCpssDrvHwPpPortGroupWriteInternalPciReg,                        \
        PRV_CPSS_ALDRIN_GPIO_0_16_E,                                       \
        PRV_CPSS_ALDRIN_GPIO_0_23_E,                                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},            \
                                                                           \
    /* 8..15, GPIO_0_31_Interrupt Cause */                                 \
    {7, GT_TRUE,8, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018110, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x0001811C), \
        prvCpssDrvHwPpPortGroupReadInternalPciReg,                         \
        prvCpssDrvHwPpPortGroupWriteInternalPciReg,                        \
        PRV_CPSS_ALDRIN_GPIO_0_8_E,                                        \
        PRV_CPSS_ALDRIN_GPIO_0_15_E,                                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},            \
                                                                           \
    /* 0..7, GPIO_0_31_Interrupt Cause */                                  \
    {8, GT_TRUE,0, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018110, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x0001811C), \
        prvCpssDrvHwPpPortGroupReadInternalPciReg,                         \
        prvCpssDrvHwPpPortGroupWriteInternalPciReg,                        \
        PRV_CPSS_ALDRIN_GPIO_0_0_E,                                        \
        PRV_CPSS_ALDRIN_GPIO_0_7_E,                                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},            \
                                                                           \
    /* PCI Express Interrupt Cause */                                      \
    {9, GT_FALSE,PRV_CPSS_DXCH_UNIT_LAST_E/* not in PP memory space !*/, NULL, 0x00041900, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00041910),  \
        prvCpssDrvHwPpPortGroupReadInternalPciReg,                         \
        prvCpssDrvHwPpPortGroupWriteInternalPciReg,                        \
        PRV_CPSS_ALDRIN_PEX_DL_DWN_TX_ACC_ERR_E,                           \
        PRV_CPSS_ALDRIN_PEX_RCV_MSI_E,                                     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* 0180400 + 0x1000* x: where x (0-31 in steps of 2)  */
#define ALDRIN_XPCS_PORT_BASE_MAC(port)                \
        (0x10180400 + 0x1000*(port))
/* XPCS - cause */
#define ALDRIN_XPCS_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN_XPCS_PORT_BASE_MAC(port) + (0x14))
/* XPCS - mask */
#define ALDRIN_XPCS_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (ALDRIN_XPCS_PORT_BASE_MAC(port) + (0x18))

/* base address of per port per lane registers */
#define ALDRIN_XPCS_PORT_PER_LANE_BASE_MAC(port,lane)                \
        (ALDRIN_XPCS_PORT_BASE_MAC(port) + 0x50 + 0x44*(lane))

/* XPCS - per port , per lane - cause */
#define ALDRIN_XPCS_PORT_LANE_SUMMARY_INTERRUPT_CAUSE_MAC(port,lane) \
        (ALDRIN_XPCS_PORT_PER_LANE_BASE_MAC(port,lane) + (0x10))

/* XPCS - per port , per lane - mask */
#define ALDRIN_XPCS_PORT_LANE_SUMMARY_INTERRUPT_MASK_MAC(port,lane) \
        (ALDRIN_XPCS_PORT_PER_LANE_BASE_MAC(port,lane) + (0x14))

#define ALDRIN_XPCS_FOR_PORT_INT_SUB_TREE_MAC(bitIndexInCaller,port) \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL, \
                ALDRIN_XPCS_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) , \
                ALDRIN_XPCS_PORT_SUMMARY_INTERRUPT_MASK_MAC(port)  , \
                prvCpssDrvHwPpPortGroupIsrRead,                    \
                prvCpssDrvHwPpPortGroupIsrWrite,                   \
                PRV_CPSS_ALDRIN_XPCS_PORT_##port##_LINK_STATUS_CHANGED_E,\
                PRV_CPSS_ALDRIN_XPCS_PORT_##port##_SUM_LANE3_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4,NULL, NULL},\
                    ALDRIN_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(7 ,port,0),\
                    ALDRIN_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(8 ,port,1),\
                    ALDRIN_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(9 ,port,2),\
                    ALDRIN_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(10,port,3)

/* per port per lane interrupt */
#define ALDRIN_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(bitIndexInCaller,port,lane) \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL, \
                ALDRIN_XPCS_PORT_LANE_SUMMARY_INTERRUPT_CAUSE_MAC(port,lane) , \
                ALDRIN_XPCS_PORT_LANE_SUMMARY_INTERRUPT_MASK_MAC(port,lane) , \
                prvCpssDrvHwPpPortGroupIsrRead,                    \
                prvCpssDrvHwPpPortGroupIsrWrite,                   \
                PRV_CPSS_ALDRIN_XPCS_PORT_##port##_LANE_##lane##_PRBS_ERROR_E,\
                PRV_CPSS_ALDRIN_XPCS_PORT_##port##_LANE_##lane##_DETECTED_IIAII_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* support XPCS interrupt summary (bit 4) : only even ports have it */
#define XPCS_BIT4_EXISTS_MAC(port)   (1-((port) & 1))

/* summary of Giga mac that called from XLG mac */
#define ALDRIN_GIGA_PORT_SUMMARY_FROM_XLG_INT_SUB_TREE_MAC(port)                                \
                {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                    \
                ALDRIN_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) ,                            \
                ALDRIN_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port)  ,                            \
                prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                PRV_CPSS_ALDRIN_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E,                          \
                PRV_CPSS_ALDRIN_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E, /*see PTP issue below */ \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1/*see PTP issue below */,NULL, NULL},  \
                    /* interrupts of the giga mac */                                            \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                               \
                        ALDRIN_GIGA_PORT_INTERRUPT_CAUSE_MAC(port) ,                            \
                        ALDRIN_GIGA_PORT_INTERRUPT_MASK_MAC(port),                              \
                     prvCpssDrvHwPpPortGroupIsrRead,                                            \
                     prvCpssDrvHwPpPortGroupIsrWrite,                                           \
                     PRV_CPSS_ALDRIN_GIGA_PORT_##port##_LINK_STATUS_CHANGED_E,                  \
                     PRV_CPSS_ALDRIN_GIGA_PORT_##port##_MIB_COUNTER_WRAPAROUND_E,               \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


/* 10180000 + 0x1000* k: where k (0-31)  */
#define ALDRIN_MPCS_PORT_BASE_MAC(port)                 \
        (0x10180000  + 0x1000*(port))

/* MPCS - cause */
#define ALDRIN_MPCS_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN_MPCS_PORT_BASE_MAC(port) + (0x08))

/* MPCS - mask */
#define ALDRIN_MPCS_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN_MPCS_PORT_BASE_MAC(port) + (0x0C))

#define ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port)                         \
                /* XLG - External Units Interrupts cause */                                         \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                         \
                ALDRIN_XLG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port),                                  \
                ALDRIN_XLG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port),                                   \
                prvCpssDrvBobKXlgGopExtUnitsIsrRead,                                                \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_ALDRIN_XLG_PORT_##port##_SUM_XLG_PORT_INTERRUPT_SUMMARY_E,                 \
                PRV_CPSS_ALDRIN_XLG_PORT_##port##_SUM_PTP_UNIT_INTERRUPT_SUMMARY_E,                 \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4+(XPCS_BIT4_EXISTS_MAC(port)),NULL, NULL}, \
                    /* interrupts of the XLG mac */                                                 \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    ALDRIN_XLG_PORT_INTERRUPT_CAUSE_MAC(port) ,                                     \
                    ALDRIN_XLG_PORT_INTERRUPT_MASK_MAC(port),                                       \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_ALDRIN_XLG_PORT_##port##_LINK_STATUS_CHANGED_E,                        \
                    PRV_CPSS_ALDRIN_XLG_PORT_##port##_PFC_SYNC_FIFO_OVERRUN_E,                      \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                          \
                    /*bit2*/                                                                        \
                    ALDRIN_GIGA_PORT_SUMMARY_FROM_XLG_INT_SUB_TREE_MAC(port),                       \
                    /* bit 4 - XPCS - will be added for even ports only */                          \
                    /* interrupts of the MPCS mac (also called GB)*/                                \
                    { 5 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    ALDRIN_MPCS_PORT_INTERRUPT_CAUSE_MAC(port) ,                                    \
                    ALDRIN_MPCS_PORT_INTERRUPT_MASK_MAC(port),                                      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_ALDRIN_MPCS_PORT_##port##_ACCESS_ERROR_E,                              \
                    PRV_CPSS_ALDRIN_MPCS_PORT_##port##_GB_LOCK_SYNC_CHANGE_E,                       \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                          \
                    /*bit 7*/                                                                       \
                    ALDRIN_PTP_FOR_PORT_INT_ELEMENT_MAC(7,port)

/* even port hold XPCS interrupts too */
#define ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port)         \
                ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port), \
                    ALDRIN_XPCS_FOR_PORT_INT_SUB_TREE_MAC(4,port)

#define ALDRIN_XLG_PORTS_0_27_SUMMARY_INT_SUB_TREE_MAC                 \
                                                          /*bit,port*/ \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  4 ,  0 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  5 ,  1 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  6 ,  2 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  7 ,  3 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  8 ,  4 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  9 ,  5 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 10 ,  6 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 11 ,  7 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 12 ,  8 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 13 ,  9 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 14 , 10 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 15 , 11 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 16 , 12 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 17 , 13 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 18 , 14 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 19 , 15 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 20 , 16 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 21 , 17 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 22 , 18 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 23 , 19 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 24 , 20 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 25 , 21 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 26 , 22 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 27 , 23 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 28 , 24 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 29 , 25 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 30 , 26 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 31 , 27 )


#define ALDRIN_XLG_PORTS_28_31_SUMMARY_INT_SUB_TREE_MAC                 \
                                                          /*bit,port*/  \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  1 ,  28 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  2 ,  29 ), \
            ALDRIN_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  3 ,  30 ), \
            ALDRIN_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  4 ,  31 )


GT_STATUS prvCpssGenericSrvCpuRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);
GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);
#define   AP_Doorbell_SUB_TREE_MAC(bit)                                     \
    /* AP_DOORBELL_MAIN */                                                  \
    {bit,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000010, 0x00000011,    \
        prvCpssGenericSrvCpuRegisterRead,                                   \
        prvCpssGenericSrvCpuRegisterWrite,                                  \
        PRV_CPSS_ALDRIN_AP_DOORBELL_MAIN_IPC_E,                             \
        PRV_CPSS_ALDRIN_AP_DOORBELL_MAIN_RESERVED31_E,                      \
        FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0xFFFFF880, 6,NULL, NULL},       \
        /* AP_DOORBELL_PORT0_31_LINK_STATUS_CHANGE */                       \
        {4,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000012, 0x00000013,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_0_LINK_STATUS_CHANGE_E,        \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_31_LINK_STATUS_CHANGE_E,       \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT32_63_LINK_STATUS_CHANGE */                      \
        {5,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000014, 0x00000015,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_32_LINK_STATUS_CHANGE_E,       \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_63_LINK_STATUS_CHANGE_E,       \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT64_95_LINK_STATUS_CHANGE */                      \
        {6,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000016, 0x00000017,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_64_LINK_STATUS_CHANGE_E,       \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_95_LINK_STATUS_CHANGE_E,       \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT0_31_802_3_AP */                                 \
        {8,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000018, 0x00000019,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_0_802_3_AP_E,                  \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_31_802_3_AP_E,                 \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT32_63_802_3_AP */                                \
        {9,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000001a, 0x0000001b,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_32_802_3_AP_E,                 \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_63_802_3_AP_E,                 \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT64_95_802_3_AP */                                \
        {10,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000001c, 0x0000001d, \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_64_802_3_AP_E,                 \
            PRV_CPSS_ALDRIN_AP_DOORBELL_PORT_95_802_3_AP_E,                 \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL}

static /*const*/ PRV_CPSS_DRV_INTERRUPT_SCAN_STC aldrinIntrScanArr[] =
{
    /* Global Interrupt Cause */
    {0, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000030, 0x00000034,
        prvCpssDrvHwPpPortGroupIsrRead,
        prvCpssDrvHwPpPortGroupIsrWrite,
        PRV_CPSS_ALDRIN_GLOBAL_S_SUMMARY_FUNC_UNITS_SUM_E,
        PRV_CPSS_ALDRIN_GLOBAL_S_SUMMARY_MG1_INTERNAL_SUM_E,
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 12, NULL, NULL},

        /* NOTE: the PEX and the DFX are not connected to MG but directly
         * to the MSYS (CPU periphery) */

        FuncUnitsIntsSum_SUB_TREE_MAC(3),

        DataPathIntSum_SUB_TREE_MAC(4),

        /* ports0SumIntSum */
        {5,GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000080, 0x00000084,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_PORTS0_SUM_TAI_MSM4_SUM_E,
            PRV_CPSS_ALDRIN_PORTS0_SUM_MSM4_PORTS_INT27_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 29, NULL, NULL}, /* bits 1,3 are internal */

            /* TAI Interrupt Cause */
            {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TAI_E, NULL, 0x65000000, 0x65000004,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_ALDRIN_TAI_GENERATION_INT_E,
                PRV_CPSS_ALDRIN_TAI_DECREMENT_LOST_INT_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

            /* XLG ports 0..27 bits 4..31 */
            ALDRIN_XLG_PORTS_0_27_SUMMARY_INT_SUB_TREE_MAC,

        /* dfxIntSum */
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000ac, 0x000000b0,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_DFX_SUM_DFX_INTERRUPT_SUM_1_E,
            PRV_CPSS_ALDRIN_DFX_SUM_DFX_INTERRUPT_SUM_31_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}, /* without children yet */

        /* Miscellaneous */
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000038, 0x0000003C,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_MISC_TWSI_TIME_OUT_E,
            PRV_CPSS_ALDRIN_MISC_SERDES_OUT_OF_RANGE_VIOLATION_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1,NULL, NULL},
            /* HostCpuDoorbellIntSum */
            {24, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000518, 0x0000051c,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_ALDRIN_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,
                PRV_CPSS_ALDRIN_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 1,NULL, NULL},

                AP_Doorbell_SUB_TREE_MAC(4),

        /* Tx SDMA  */
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, 0x00002818,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_0_E,
            PRV_CPSS_ALDRIN_TX_SDMA_TX_REJECT_0_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},
        /* Rx SDMA  */
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, 0x00002814,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_0_E,
            PRV_CPSS_ALDRIN_RX_SDMA_PACKET_CNT_OF_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /* dfx1IntSum */
        {10, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000b8, 0x000000bc,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_DFX1_SUM_DFX_1_INTERRUPT_SUM_1_E,
            PRV_CPSS_ALDRIN_DFX1_SUM_DFX_1_INTERRUPT_SUM_31_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}, /* without children yet */

        FuncUnits1IntsSum_SUB_TREE_MAC(11),

        /* xsmi0IntSum */
        {12, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030010, 0x00030014,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_MG_XSMI0_XG_SMI_WRITE_E,
            PRV_CPSS_ALDRIN_MG_XSMI0_XG_SMI_WRITE_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},

        /* xsmi1IntSum */
        {13, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00032010, 0x00032014,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_MG_XSMI1_XG_SMI_WRITE_E,
            PRV_CPSS_ALDRIN_MG_XSMI1_XG_SMI_WRITE_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},

        /* ports1SumIntSum */
        {16,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000150, 0x00000154,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN_PORTS1_SUM_MSM4_PORTS_INT28_SUM_E,
            PRV_CPSS_ALDRIN_PORTS1_SUM_CPU_PORT_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},

            /* XLG ports 28..31 bits 1..4 */
            ALDRIN_XLG_PORTS_28_31_SUMMARY_INT_SUB_TREE_MAC,

            /* CPU giga port  */
            ALDRIN_GIGA_PORT_SUMMARY_FROM_MG_INT_SUB_TREE_MAC(5 , 32)
};

/* number of elements in the array of aldrinIntrScanArr[] */
#define ALDRIN_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    (sizeof(aldrinIntrScanArr)/sizeof(aldrinIntrScanArr[0]))

#define ALDRIN_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_ALDRIN_LAST_INT_E / 32)

/* This array maps an interrupt index to its relevant interrupt mask reg    */
/* default value, cell i indicates the address for interrupts               */
/* (32 * i  --> 32 * (i + 1) - 1).                                          */
/* summary bits are turned on and the specific cause mask is enabled when   */
/* bounded by appl.                                                         */
static GT_U32 aldrinMaskRegDefaultSummaryArr[ALDRIN_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};

/* This array maps an interrupt index to its    */
/* relevant interrupt mask registers address,   */
/* cell i indicates the address for interrupts  */
/* (32 * i  --> 32 * (i + 1) - 1).              */
static GT_U32 aldrinMaskRegMapArr[ALDRIN_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};


/* get the first value of event in register (clear its 5 LSBits)*/
#define GET_FIRST_EVENT_IN_REG_MAC(_event)  ((_event) & (~0x1f))

/* the full 32 events for event that belongs to a register ,
   will have index equal to the 32 events of the register */
#define SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(_event)                     \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 0 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 1 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 2 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 3 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 4 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 5 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 6 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 7 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 8 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 9 ), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 10), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 11), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 12), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 13), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 14), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 15), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 16), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 17), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 18), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 19), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 20), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 21), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 22), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 23), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 24), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 25), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 26), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 27), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 28), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 29), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 30), \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 31)


#define SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(_prefix,_postFix) \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     56),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     57),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     58),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     59),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     64),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     65),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     66),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     67),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     68),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     69),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     70),  \
    SET_INDEX_EQUAL_EVENT_PER_PORT_MAC (_prefix,_postFix,     71)


/* state for even ports in range 0..31 that event hold extra port number */
#define SET_EVENT_PER_32_PORTS_EVEN_MAC(_prefix,_postFix) \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     0),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     2),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     4),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     6),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     8),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     10),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     12),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     14),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     16),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     18),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     20),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     22),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     24),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     26),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     28),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     30)


/* state for ports 0..31 that event hold extra port number */
#define SET_EVENT_PER_32_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_32_PORTS_EVEN_MAC(_prefix,_postFix), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     1),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     3),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     5),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     7),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     9),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     11),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     13),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     15),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     17),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     19),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     21),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     23),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     25),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     27),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     29),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     31)


/* state for ports 0..32 that event hold extra port number */
#define SET_EVENT_PER_33_PORTS_MAC(_prefix, _postFix)   \
    SET_EVENT_PER_32_PORTS_MAC(_prefix, _postFix),      \
    SET_EVENT_PER_PORT_MAC(_prefix, _postFix, 32)


/* state for ports 0..71 that event hold extra port number */
#define SET_EVENT_PER_72_PORTS_MAC(_prefix,_postFix)        \
    SET_EVENT_PER_33_PORTS_MAC(_prefix,_postFix),           \
    /* the port 33..71 */                                   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     33      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     34      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     35      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     36      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     37      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     38      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     39      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     40      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     41      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     42      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     43      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     44      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     45      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     46      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     47      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     48      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     49      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     50      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     51      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     52      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     53      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     54      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     55      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     56      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     57      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     58      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     59      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     60      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     61      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     62      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     63      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     64      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     65      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     66      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     67      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     68      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     69      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     70      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     71      )


/* state for ALL 0..127 ports that event hold extra port number */
#define SET_EVENT_PER_128_PORTS_MAC(_prefix,_postFix)       \
    /* the first 33 ports */                                \
    SET_EVENT_PER_72_PORTS_MAC(_prefix,_postFix),           \
    /* the ports 72..127 */                                 \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     72      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     73      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     74      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     75      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     76      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     77      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     78      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     79      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     80      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     81      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     82      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     83      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     84      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     85      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     86      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     87      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     88      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     89      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     90      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     91      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     92      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     93      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     94      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     95      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     96      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     97      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     98      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     99      ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     100     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     101     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     102     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     103     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     104     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     105     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     106     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     107     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     108     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     109     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     110     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     111     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     112     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     113     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     114     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     115     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     116     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     117     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     118     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     119     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     120     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     121     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     122     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     123     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     124     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     125     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     126     ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     127     )


#define SET_EVENT_PER_32_PORTS_EVEN_FOR_4_LANES_MAC(_prefix,_postFix) \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     0),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     2),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     4),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     6),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     8),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     10),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     12),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     14),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     16),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     18),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     20),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     22),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     24),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     26),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     28),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     30)

#define SET_EVENT_PER_INDEX____0_127___MAC(_prefix,_postFix)   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     0       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     1       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     2       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     3       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     4       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     5       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     6       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     7       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     8       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     9       ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     10      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     11      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     12      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     13      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     14      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     15      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     16      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     17      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     18      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     19      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     20      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     21      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     22      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     23      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     24      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     25      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     26      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     27      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     28      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     29      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     30      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     31      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     32      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     33      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     34      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     35      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     36      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     37      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     38      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     39      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     40      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     41      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     42      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     43      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     44      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     45      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     46      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     47      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     56      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     57      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     58      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     59      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     64      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     65      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     66      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     67      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     68      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     69      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     70      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     71      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     72      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     73      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     74      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     75      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     76      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     77      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     78      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     79      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     80      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     81      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     82      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     83      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     84      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     85      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     86      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     87      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     88      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     89      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     90      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     91      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     92      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     93      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     94      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     95      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     96      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     97      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     98      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     99      ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     100     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     101     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     102     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     103     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     104     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     105     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     106     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     107     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     108     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     109     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     110     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     111     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     112     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     113     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     114     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     115     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     116     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     117     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     118     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     119     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     120     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     121     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     122     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     123     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     124     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     125     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     126     ),   \
    SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     127     )


/* state for 72 ports that event hold extra param as : ((portNum) << port_offset) |  ext_param */
#define SET_EVENT_PER_33_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset) \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 0 , ((0 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 1 , ((1 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 2 , ((2 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 3 , ((3 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 4 , ((4 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 5 , ((5 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 6 , ((6 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 7 , ((7 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 8 , ((8 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 9 , ((9 << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 10, ((10<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 11, ((11<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 12, ((12<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 13, ((13<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 14, ((14<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 15, ((15<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 16, ((16<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 17, ((17<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 18, ((18<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 19, ((19<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 20, ((20<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 21, ((21<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 22, ((22<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 23, ((23<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 24, ((24<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 25, ((25<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 26, ((26<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 27, ((27<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 28, ((28<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 29, ((29<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 30, ((30<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 31, ((31<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 32, ((32<< port_offset) | (ext_param)))




/* Interrupt cause to unified event map for Aldrin
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static GT_U32 aldrinUniEvMapTableWithExtData[] = {
 /* Per Queue events */
 CPSS_PP_TX_BUFFER_QUEUE_E,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_0_E,  0,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_1_E,  1,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_2_E,  2,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_3_E,  3,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_4_E,  4,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_5_E,  5,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_6_E,  6,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_BUFFER_QUEUE_7_E,  7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_ERR_QUEUE_E,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_ERROR_QUEUE_0_E,  0,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_ERROR_QUEUE_1_E,  1,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_ERROR_QUEUE_2_E,  2,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_ERROR_QUEUE_3_E,  3,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_ERROR_QUEUE_4_E,  4,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_ERROR_QUEUE_5_E,  5,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_ERROR_QUEUE_6_E,  6,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_ERROR_QUEUE_7_E,  7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_END_E,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_END_QUEUE_0_E,  0,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_END_QUEUE_1_E,  1,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_END_QUEUE_2_E,  2,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_END_QUEUE_3_E,  3,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_END_QUEUE_4_E,  4,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_END_QUEUE_5_E,  5,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_END_QUEUE_6_E,  6,
    PRV_CPSS_ALDRIN_TX_SDMA_TX_END_QUEUE_7_E,  7,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_0_E,     0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_1_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_2_E,     2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_3_E,     3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_4_E,     4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_5_E,     5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_6_E,     6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_BUFFER_QUEUE_7_E,     7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE0_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_ERROR_QUEUE_0_E,      0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE1_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_ERROR_QUEUE_1_E,      1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE2_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_ERROR_QUEUE_2_E,      2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE3_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_ERROR_QUEUE_3_E,      3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE4_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_ERROR_QUEUE_4_E,      4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE5_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_ERROR_QUEUE_5_E,      5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE6_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_ERROR_QUEUE_6_E,      6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE7_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RX_ERROR_QUEUE_7_E,      7,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_MAC_SFLOW_E,
    SET_EVENT_PER_128_PORTS_MAC(PRV_CPSS_ALDRIN_INGRESS_STC, INGRESS_SAMPLE_LOADED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_EGRESS_SFLOW_E,
    SET_EVENT_PER_72_PORTS_MAC(PRV_CPSS_ALDRIN_TXQ_DQ_EGR_STC,E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_AN_COMPLETED_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_MAC(PRV_CPSS_ALDRIN_GIGA,AN_COMPLETED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_EEE_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN_GIGA,PCS_RX_PATH_RECEIVED_LPI_E,0 ,8/*port<<8*/),
    SET_EVENT_PER_33_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN_GIGA,PCS_TX_PATH_RECEIVED_LPI_E,1 ,8/*port<<8*/),
    SET_EVENT_PER_33_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN_GIGA,MAC_RX_PATH_RECEIVED_LPI_E,2 ,8/*port<<8*/),
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_MAC(PRV_CPSS_ALDRIN_GIGA,LINK_STATUS_CHANGED_E),
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,LINK_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_MAC(PRV_CPSS_ALDRIN_GIGA,RX_FIFO_OVERRUN_E),
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,RX_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_UNDERRUN_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_MAC(PRV_CPSS_ALDRIN_GIGA,TX_UNDERRUN_E),
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,TX_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_FC_STATUS_CHANGED_E,
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,FC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_ILLEGAL_SEQUENCE_E,
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,UNKNOWN_SEQUENCE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_FAULT_TYPE_CHANGE_E,
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,FAULT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_PORT_ADDRESS_OUT_OF_RANGE_E,
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,ADDRESS_OUT_OF_RANGE_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_PORT_NO_BUFF_PACKET_DROP_E,
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,NO_BUFF_PACKET_DROP_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_COUNT_COPY_DONE_E,
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,XG_COUNT_COPY_DONE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_COUNT_EXPIRED_E,
    /* XLG ports 0..31 */
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_XLG,XG_COUNT_EXPIRED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_MAC(PRV_CPSS_ALDRIN_GIGA,SYNC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PRBS_ERROR_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_MAC(PRV_CPSS_ALDRIN_GIGA,PRBS_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_MAC(PRV_CPSS_ALDRIN_GIGA,TX_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
    /* GIGA ports 0..32 */
    SET_EVENT_PER_33_PORTS_MAC(PRV_CPSS_ALDRIN_GIGA,TX_FIFO_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_TXQ2_FLUSH_PORT_E,
    SET_EVENT_PER_72_PORTS_MAC(PRV_CPSS_ALDRIN_TXQ_DQ_FLUSH_DONE_SUM,E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_PORT_DESC_FULL_E,
    SET_EVENT_PER_72_PORTS_MAC(PRV_CPSS_ALDRIN_TXQ_PORT_DESC_SUM_DESC_FULL,E),
 MARK_END_OF_UNI_EV_CNS,

 /* Per Port XPCS events */
 /* XPCS ports even ports between 0..31 */
 CPSS_PP_PORT_PCS_LINK_STATUS_CHANGED_E,
    SET_EVENT_PER_32_PORTS_EVEN_MAC(PRV_CPSS_ALDRIN_XPCS,LINK_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DESKEW_TIMEOUT_E,
    SET_EVENT_PER_32_PORTS_EVEN_MAC(PRV_CPSS_ALDRIN_XPCS,DESKEW_TIMEOUT_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DETECTED_COLUMN_IIAII_E,
    SET_EVENT_PER_32_PORTS_EVEN_MAC(PRV_CPSS_ALDRIN_XPCS,DETECTED_COLUMN_IIAII_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DESKEW_ERROR_E,
    SET_EVENT_PER_32_PORTS_EVEN_MAC(PRV_CPSS_ALDRIN_XPCS,DESKEW_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_PPM_FIFO_UNDERRUN_E,
    SET_EVENT_PER_32_PORTS_EVEN_MAC(PRV_CPSS_ALDRIN_XPCS,PPM_FIFO_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_PPM_FIFO_OVERRUN_E,
    SET_EVENT_PER_32_PORTS_EVEN_MAC(PRV_CPSS_ALDRIN_XPCS,PPM_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 /* Per Port Per lane MMPCS events */
 /* MMPCS ports even ports between 48..70 , 4 lanes per port*/
 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_MPCS,SIGNAL_DETECT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_MPCS,ALIGN_LOCK_LOST_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
    SET_EVENT_PER_32_PORTS_MAC(PRV_CPSS_ALDRIN_MPCS,GB_LOCK_SYNC_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_PRBS_ERROR_E,
    SET_EVENT_PER_32_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_ALDRIN_XPCS,PRBS_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_DISPARITY_ERROR_E,
    SET_EVENT_PER_32_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_ALDRIN_XPCS,DISPARITY_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SYMBOL_ERROR_E,
    SET_EVENT_PER_32_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_ALDRIN_XPCS,SYMBOL_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_CJR_PAT_ERROR_E,
    SET_EVENT_PER_32_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_ALDRIN_XPCS,CJR_PAT_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
    SET_EVENT_PER_32_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_ALDRIN_XPCS,SIGNAL_DETECT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
    SET_EVENT_PER_32_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_ALDRIN_XPCS,SYNC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_DETECTED_IIAII_E,
    SET_EVENT_PER_32_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_ALDRIN_XPCS,DETECTED_IIAII_E),
 MARK_END_OF_UNI_EV_CNS,

CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,
    PRV_CPSS_ALDRIN_ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,
    PRV_CPSS_ALDRIN_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,
    PRV_CPSS_ALDRIN_ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,
    PRV_CPSS_ALDRIN_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,
    SET_EVENT_PER_33_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN_PTP,TIMESTAMP_QUEUE0_FULL_E,GTS_INT_EXT_PARAM_MAC(1,0,1),8/*port<<8*/),
    SET_EVENT_PER_33_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN_PTP,TIMESTAMP_QUEUE1_FULL_E,GTS_INT_EXT_PARAM_MAC(1,1,1) ,8/*port<<8*/),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,
    PRV_CPSS_ALDRIN_ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q0_E,  0,
    PRV_CPSS_ALDRIN_ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q0_E,   1,
    PRV_CPSS_ALDRIN_ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q1_E,  2,
    PRV_CPSS_ALDRIN_ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q1_E,   3,
    SET_EVENT_PER_33_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN_PTP,NEW_TIMESTAMP_QUEUE0_E,GTS_INT_EXT_PARAM_MAC(1,0,1),8/*port<<8*/),
    SET_EVENT_PER_33_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN_PTP,NEW_TIMESTAMP_QUEUE1_E,GTS_INT_EXT_PARAM_MAC(1,1,1),8/*port<<8*/),
 MARK_END_OF_UNI_EV_CNS,

  /* Per Index events */
 CPSS_PP_GPP_E,
    /* GPP-GPIO on MG unit (on switch) */
    PRV_CPSS_ALDRIN_MISC_GPP_0_E,  0,
    PRV_CPSS_ALDRIN_MISC_GPP_1_E,  1,
    PRV_CPSS_ALDRIN_MISC_GPP_2_E,  2,
    PRV_CPSS_ALDRIN_MISC_GPP_3_E,  3,
    PRV_CPSS_ALDRIN_MISC_GPP_4_E,  4,
    PRV_CPSS_ALDRIN_MISC_GPP_5_E,  5,
    PRV_CPSS_ALDRIN_MISC_GPP_6_E,  6,
    PRV_CPSS_ALDRIN_MISC_GPP_7_E,  7,

    /* MPP-GPIO on the RUNIT (on pex) */
    PRV_CPSS_ALDRIN_GPIO_0_0_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  0   ,
    PRV_CPSS_ALDRIN_GPIO_0_1_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  1   ,
    PRV_CPSS_ALDRIN_GPIO_0_2_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  2   ,
    PRV_CPSS_ALDRIN_GPIO_0_3_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  3   ,
    PRV_CPSS_ALDRIN_GPIO_0_4_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  4   ,
    PRV_CPSS_ALDRIN_GPIO_0_5_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  5   ,
    PRV_CPSS_ALDRIN_GPIO_0_6_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  6   ,
    PRV_CPSS_ALDRIN_GPIO_0_7_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  7   ,
    PRV_CPSS_ALDRIN_GPIO_0_8_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  8   ,
    PRV_CPSS_ALDRIN_GPIO_0_9_E,     MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  9   ,
    PRV_CPSS_ALDRIN_GPIO_0_10_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  10  ,
    PRV_CPSS_ALDRIN_GPIO_0_11_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  11  ,
    PRV_CPSS_ALDRIN_GPIO_0_12_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  12  ,
    PRV_CPSS_ALDRIN_GPIO_0_13_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  13  ,
    PRV_CPSS_ALDRIN_GPIO_0_14_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  14  ,
    PRV_CPSS_ALDRIN_GPIO_0_15_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  15  ,
    PRV_CPSS_ALDRIN_GPIO_0_16_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  16  ,
    PRV_CPSS_ALDRIN_GPIO_0_17_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  17  ,
    PRV_CPSS_ALDRIN_GPIO_0_18_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  18  ,
    PRV_CPSS_ALDRIN_GPIO_0_19_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  19  ,
    PRV_CPSS_ALDRIN_GPIO_0_20_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  20  ,
    PRV_CPSS_ALDRIN_GPIO_0_21_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  21  ,
    PRV_CPSS_ALDRIN_GPIO_0_22_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  22  ,
    PRV_CPSS_ALDRIN_GPIO_0_23_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  23  ,
    PRV_CPSS_ALDRIN_GPIO_0_24_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  24  ,
    PRV_CPSS_ALDRIN_GPIO_0_25_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  25  ,
    PRV_CPSS_ALDRIN_GPIO_0_26_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  26  ,
    PRV_CPSS_ALDRIN_GPIO_0_27_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  27  ,
    PRV_CPSS_ALDRIN_GPIO_0_28_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  28  ,
    PRV_CPSS_ALDRIN_GPIO_0_29_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  29  ,
    PRV_CPSS_ALDRIN_GPIO_0_30_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  30  ,
    PRV_CPSS_ALDRIN_GPIO_0_31_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  31  ,
    PRV_CPSS_ALDRIN_GPIO_1_32_E,    MPP_RUNIT_RFU_OFFSET_IN_GPP_CNS +  32  ,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,    0,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,    1,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,    2,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,    3,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,    4,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,    5,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,    6,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,    7,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,    8,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,    9,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E,  10,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E,  11,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E,  12,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E,  13,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E,  14,
    PRV_CPSS_ALDRIN_CNC0_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  15,

    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,   16,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,   17,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,   18,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,   19,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,   20,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,   21,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,   22,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,   23,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,   24,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,   25,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E,  26,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E,  27,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E,  28,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E,  29,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E,  30,
    PRV_CPSS_ALDRIN_CNC1_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  31,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_BM_MISC_E,
    PRV_CPSS_ALDRIN_BM_GEN1_SUM_GLOBAL_RX_FULL_E,           4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_CNTR_OVERFLOW_E,
    PRV_CPSS_ALDRIN_RX_SDMA_RESOURCE_ERROR_CNT_OF_E,  0,
    PRV_CPSS_ALDRIN_RX_SDMA_BYTE_CNT_OF_E,            1,
    PRV_CPSS_ALDRIN_RX_SDMA_PACKET_CNT_OF_E,          2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_MISC_E,
    PRV_CPSS_ALDRIN_TXQ_GEN_SUM_QUEUE_WRONG_ADDR_E,    0,
    PRV_CPSS_ALDRIN_TXQ_GEN_SUM_GLOBAL_DESC_FULL_E,    1,
    PRV_CPSS_ALDRIN_TXQ_GEN_SUM_GLOBAL_BUFF_FULL_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_SCT_RATE_LIMITER_E,
    SET_EVENT_PER_INDEX____0_127___MAC(PRV_CPSS_ALDRIN_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER,PKT_DROPED_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,
    PRV_CPSS_ALDRIN_IPLR0_ADDRESS_OUT_OF_MEMORY_E,    0,
    PRV_CPSS_ALDRIN_EPLR_SUM_ADDRESS_OUT_OF_MEMORY_E,     1,
    PRV_CPSS_ALDRIN_IPLR1_SUM_ADDRESS_OUT_OF_MEMORY_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_DATA_ERR_E,
    /* PRV_CPSS_ALDRIN_IPLR0_DATA_ERROR_E,    0, --> bound in CPSS_PP_DATA_INTEGRITY_ERROR_E (like in Lion2) */
    PRV_CPSS_ALDRIN_EPLR_SUM_DATA_ERROR_E,     1,
    /* PRV_CPSS_ALDRIN_IPLR1_SUM_DATA_ERROR_E,    2, --> bound in CPSS_PP_DATA_INTEGRITY_ERROR_E (like in Lion2)*/
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,
    PRV_CPSS_ALDRIN_IPLR0_IPFIX_WRAPAROUND_E,    0,
    PRV_CPSS_ALDRIN_EPLR_SUM_IPFIX_WRAPAROUND_E,     1,
    PRV_CPSS_ALDRIN_IPLR1_SUM_IPFIX_WRAPAROUND_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_ALARM_E,
    PRV_CPSS_ALDRIN_IPLR0_IPFIX_ALARM_E,    0,
    PRV_CPSS_ALDRIN_EPLR_SUM_IPFIX_ALARM_E,     1,
    PRV_CPSS_ALDRIN_IPLR1_SUM_IPFIX_ALARM_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,
    PRV_CPSS_ALDRIN_IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    0,
    PRV_CPSS_ALDRIN_EPLR_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,     1,
    PRV_CPSS_ALDRIN_IPLR1_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 0,

    PRV_CPSS_ALDRIN_IPCL_SUM_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 1,

    PRV_CPSS_ALDRIN_IPCL_SUM_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_ALDRIN_IPCL_SUM_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,
    PRV_CPSS_ALDRIN_IPCL_SUM_LOOKUP0_FIFO_FULL_E, 0,
    PRV_CPSS_ALDRIN_IPCL_SUM_LOOKUP1_FIFO_FULL_E, 1,
    PRV_CPSS_ALDRIN_IPCL_SUM_LOOKUP2_FIFO_FULL_E, 2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_EB_NA_FIFO_FULL_E,
    PRV_CPSS_ALDRIN_FDB_NA_FIFO_FULL0_E    , 0 ,
    PRV_CPSS_ALDRIN_FDB_NA_FIFO_FULL1_E    , 1 ,
    PRV_CPSS_ALDRIN_FDB_NA_FIFO_FULL2_E    , 2 ,
    PRV_CPSS_ALDRIN_FDB_NA_FIFO_FULL3_E    , 3 ,
    PRV_CPSS_ALDRIN_FDB_NA_FIFO_FULL4_E    , 4 ,
    PRV_CPSS_ALDRIN_FDB_NA_FIFO_FULL5_E    , 5 ,
    PRV_CPSS_ALDRIN_FDB_NA_FIFO_FULL6_E    , 6 ,
    PRV_CPSS_ALDRIN_FDB_NA_FIFO_FULL7_E    , 7 ,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
     PRV_CPSS_ALDRIN_TTI_SUM_TTI0_ACCESS_DATA_ERROR_E, 0,
     PRV_CPSS_ALDRIN_TTI_SUM_TTI1_ACCESS_DATA_ERROR_E, 1,
 MARK_END_OF_UNI_EV_CNS,

 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */
 CPSS_PP_CRITICAL_HW_ERROR_E,
/* The following event is in valid traffic case. The event is when allocation counter get to total buffers limit.
   This is legal situation for stress test scenarios.
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_RX_DMA0_SUM_GLOBAL_ALLOC_COUNTER_OVERFLOW_E), */
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_0_SCDMA, SUM_FIRST_PACKET_CHUNK_WITHOUT_SOP_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_0_SCDMA, SUM_PACKET_WITH_NO_EOP_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_1_SCDMA, SUM_FIRST_PACKET_CHUNK_WITHOUT_SOP_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_1_SCDMA, SUM_PACKET_WITH_NO_EOP_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_2_SCDMA, SUM_FIRST_PACKET_CHUNK_WITHOUT_SOP_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_2_SCDMA, SUM_PACKET_WITH_NO_EOP_E),

/*  The following event is in valid traffic case. Should be verified in HW.
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_SCDMA, SUM_PORT_BUF_CNT_UNDERFLOW_E), */
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_0_SCDMA, SUM_PORT_BUF_CNT_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_0_SCDMA, SUM_SOP_DESC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_0_SCDMA, SUM_EOP_DESC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_1_SCDMA, SUM_PORT_BUF_CNT_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_1_SCDMA, SUM_SOP_DESC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_1_SCDMA, SUM_EOP_DESC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_2_SCDMA, SUM_SOP_DESC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_FOR_56_71_PORTS_MAC(PRV_CPSS_ALDRIN_RX_DMA_2_SCDMA, SUM_EOP_DESC_FIFO_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_0_SUM_CREDITS_COUNTER_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_0_SUM_RX_LB_TOKEN_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_0_SUM_RX_WR_DATA_TO_BANK_WITHOUT_VALID_BANK_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_0_SUM_RX_IDDB_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_0_SUM_RX_PHYSICAL_BANK_SYNC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_0_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_1_SUM_CREDITS_COUNTER_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_1_SUM_RX_LB_TOKEN_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_1_SUM_RX_WR_DATA_TO_BANK_WITHOUT_VALID_BANK_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_1_SUM_RX_IDDB_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_1_SUM_RX_PHYSICAL_BANK_SYNC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_1_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_1_SUM_TX_RD_BURST_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_2_SUM_CREDITS_COUNTER_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_2_SUM_RX_LB_TOKEN_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_2_SUM_RX_WR_DATA_TO_BANK_WITHOUT_VALID_BANK_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_2_SUM_RX_IDDB_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_2_SUM_RX_PHYSICAL_BANK_SYNC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_RX_CLIENT_2_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_TX_CLIENT_2_SUM_TX_RD_BURST_FIFO_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_BANK_2_SUM_BANK_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_BANK_3_SUM_BANK_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_BANK_4_SUM_BANK_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_BANK_5_SUM_BANK_LATENCY_FIFO_OVERRUN_E),


    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PARITY_SUM_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_CNTR_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_CNTR_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_OVERFLOW_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC0_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC1_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC2_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC3_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC4_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC5_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC6_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC7_UNDERFLOW_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_0_INVALID_BUFFER_CLEAR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_0_NEXT_TABLE_UPDATE_WAS_LOST_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BMA_SUM_MC_CNT_COUNTER_FLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BMA_SUM_SHIFTER_LATENCY_FIFO_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BMA_SUM_MC_CNT_PARITY_ERROR_E),

    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_0_FIFO_OVERRUN1_SUM_SC_DMA_ID_FIFO_OVERRUN_REG1_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_0_FIFO_OVERRUN2_SUM_SC_DMA_ID_FIFO_OVERRUN_REG2_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_0_FIFO_OVERRUN3_SUM_SC_DMA_ID_FIFO_OVERRUN_REG3_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_1_FIFO_OVERRUN1_SUM_SC_DMA_ID_FIFO_OVERRUN_REG1_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_1_FIFO_OVERRUN2_SUM_SC_DMA_ID_FIFO_OVERRUN_REG2_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_1_FIFO_OVERRUN3_SUM_SC_DMA_ID_FIFO_OVERRUN_REG3_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_2_FIFO_OVERRUN1_SUM_SC_DMA_ID_FIFO_OVERRUN_REG1_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_2_FIFO_OVERRUN2_SUM_SC_DMA_ID_FIFO_OVERRUN_REG2_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TXDMA_2_FIFO_OVERRUN3_SUM_SC_DMA_ID_FIFO_OVERRUN_REG3_SUMMARY_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_0_RX_DMA_UPDATE1_SUM_RX_DMA_0_UPDATES_FIF_OS_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_0_RX_DMA_UPDATE1_SUM_RX_DMA_1_UPDATES_FIF_OS_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_0_RX_DMA_UPDATE1_SUM_RX_DMA_2_UPDATES_FIF_OS_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_1_RX_DMA_UPDATE1_SUM_RX_DMA_0_UPDATES_FIF_OS_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_1_RX_DMA_UPDATE1_SUM_RX_DMA_1_UPDATES_FIF_OS_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_1_RX_DMA_UPDATE1_SUM_RX_DMA_2_UPDATES_FIF_OS_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_2_RX_DMA_UPDATE1_SUM_RX_DMA_0_UPDATES_FIF_OS_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_2_RX_DMA_UPDATE1_SUM_RX_DMA_1_UPDATES_FIF_OS_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_2_RX_DMA_UPDATE1_SUM_RX_DMA_2_UPDATES_FIF_OS_OVERRUN_E),


    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_0_GENERAL_SUM_TX_Q_TO_TX_DMA_HA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_0_GENERAL_SUM_TX_Q_TO_TX_DMA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_1_GENERAL_SUM_TX_Q_TO_TX_DMA_HA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_1_GENERAL_SUM_TX_Q_TO_TX_DMA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_2_GENERAL_SUM_TX_Q_TO_TX_DMA_HA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_2_GENERAL_SUM_TX_Q_TO_TX_DMA_DESC_OVERRUN_E),

    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_0_NOT_READY1_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG1_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_0_NOT_READY2_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG2_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_0_NOT_READY3_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG3_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_1_NOT_READY1_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG1_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_1_NOT_READY2_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG2_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_1_NOT_READY3_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG3_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_2_NOT_READY1_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG1_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_2_NOT_READY2_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG2_SUMMARY_E),
    /*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_TX_FIFO_2_NOT_READY3_SUM_SHIFTER_SOP_EOP_FIFO_OVERRUNT_REG3_SUMMARY_E),

 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_DATA_INTEGRITY_ERROR_E,
    /* -- _ECC_ --*/

/*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_CRITICAL_ECC_ERROR_BM_CRITICAL_ECC_SUM_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TTI_SUM_CRITICAL_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TTI_SUM_CRITICAL_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_HA_SUM_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_HA_SUM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MLL_SUM_FILE_ECC_1_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MLL_SUM_FILE_ECC_2_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_EFT_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_EFT_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXQ_GEN_SUM_TD_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_QCN_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXQ_DQ_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXQ_DQ_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXQ_DQ_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXQ_DQ_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_0_GENERAL_SUM_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_0_GENERAL_SUM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_1_GENERAL_SUM_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_1_GENERAL_SUM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_2_GENERAL_SUM_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXDMA_2_GENERAL_SUM_ECC_DOUBLE_ERROR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_0_GENERAL1_SUM_ECC_0_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_0_GENERAL1_SUM_ECC_1_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_0_GENERAL1_SUM_ECC_0_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_0_GENERAL1_SUM_ECC_1_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_1_GENERAL1_SUM_ECC_0_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_1_GENERAL1_SUM_ECC_1_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_1_GENERAL1_SUM_ECC_0_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_1_GENERAL1_SUM_ECC_1_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_2_GENERAL1_SUM_ECC_0_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_2_GENERAL1_SUM_ECC_1_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_2_GENERAL1_SUM_ECC_0_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TX_FIFO_2_GENERAL1_SUM_ECC_1_DOUBLE_ERROR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E),

    /* -- _PARITY_ -- */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PARITY_SUM_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_PFC_PARITY_SUM_PFC_PORT_GROUP0_PFC_IND_FIFO_ECC_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E),
#if 0
/*
 *      The interrupt should not be mapped to CPSS general events, and it should
 *      not be tied to a callback function.
*/
    /* 'WA' name : it is for 'reference' purposes */
    PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
#endif
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E),
    /* BMA is internal unit -- SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_BMA_SUM_MC_CNT_PARITY_ERROR_E),*/

    /*-- DATA_ERROR --*/

    /* no other _DATA_ERROR_ beside those that already bound in:
        CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
        CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
        CPSS_PP_POLICER_DATA_ERR_E
    */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_IPLR0_DATA_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN_IPLR1_SUM_DATA_ERROR_E),

     /* -- _DFX_ -- */
    /*full register*/ SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_DFX_SUM_DFX_INTERRUPT_SUMMARY_E),

    /* -- _DFX_1_ -- */
    /*full register*/ SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,
    PRV_CPSS_ALDRIN_INGR_OAM_SUM_KEEP_ALIVE_AGING_E,       0,
    PRV_CPSS_ALDRIN_EGR_OAM_SUM_KEEP_ALIVE_AGING_E,        1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
    PRV_CPSS_ALDRIN_INGR_OAM_SUM_EXCESS_KEEPALIVE_E,       0,
    PRV_CPSS_ALDRIN_EGR_OAM_SUM_EXCESS_KEEPALIVE_E,        1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,
    PRV_CPSS_ALDRIN_INGR_OAM_SUM_INVALID_KEEPALIVE_HASH_E, 0,
    PRV_CPSS_ALDRIN_EGR_OAM_SUM_INVALID_KEEPALIVE_HASH_E,  1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
    PRV_CPSS_ALDRIN_INGR_OAM_SUM_RDI_STATUS_E,             0,
    PRV_CPSS_ALDRIN_EGR_OAM_SUM_RDI_STATUS_E,              1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,
    PRV_CPSS_ALDRIN_INGR_OAM_SUM_TX_PERIOD_E,              0,
    PRV_CPSS_ALDRIN_EGR_OAM_SUM_TX_PERIOD_E,               1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
    PRV_CPSS_ALDRIN_INGR_OAM_SUM_MEG_LEVEL_EXCEPTION_E,    0,
    PRV_CPSS_ALDRIN_EGR_OAM_SUM_MEG_LEVEL_EXCEPTION_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,
    PRV_CPSS_ALDRIN_INGR_OAM_SUM_SOURCE_ERFACE_EXCEPTION_E,   0,
    PRV_CPSS_ALDRIN_EGR_OAM_SUM_SOURCE_ERFACE_EXCEPTION_E,    1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
    PRV_CPSS_ALDRIN_INGR_OAM_SUM_ILLEGAL_OAM_ENTRY_INDEX_E,    0,
    PRV_CPSS_ALDRIN_EGR_OAM_SUM_ILLEGAL_OAM_ENTRY_INDEX_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E,
    PRV_CPSS_ALDRIN_TAI_INCOMING_TRIGGER_INT_E, 0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PTP_TAI_GENERATION_E,
    PRV_CPSS_ALDRIN_TAI_GENERATION_INT_E, 0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_XSMI_WRITE_DONE_E,
 PRV_CPSS_ALDRIN_MG_XSMI0_XG_SMI_WRITE_E, 0,
 PRV_CPSS_ALDRIN_MG_XSMI1_XG_SMI_WRITE_E, 1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
    SET_EVENT_PER_72_PORTS_MAC(PRV_CPSS_ALDRIN_AP_DOORBELL,LINK_STATUS_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_SRVCPU_PORT_802_3_AP_E,
    SET_EVENT_PER_72_PORTS_MAC(PRV_CPSS_ALDRIN_AP_DOORBELL,802_3_AP_E),
 MARK_END_OF_UNI_EV_CNS

};
/* Interrupt cause to unified event map for ALDRIN with extended data size */
#define ALDRIN_NUM_UNI_EVENTS_CNS     (sizeof(aldrinUniEvMapTableWithExtData)/(sizeof(GT_U32)))


/* Interrupt cause to unified event map for ALDRIN
 This Table is for unified event without extended data
*/
static GT_U32 aldrinUniEvMapTable[][2] =
{
 {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_ALDRIN_MISC_TWSI_TIME_OUT_E},
 {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_ALDRIN_MISC_TWSI_STATUS_E},
 {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_ALDRIN_MISC_ILLEGAL_ADDR_E},

 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_ALDRIN_TXQ_DQ_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},

 {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,   PRV_CPSS_ALDRIN_BRIDGE_UPDATE_SECURITY_BREACH_E},
 {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,          PRV_CPSS_ALDRIN_FDB_NUM_OF_HOP_EX_P_E},
 {CPSS_PP_MAC_NA_LEARNED_E,              PRV_CPSS_ALDRIN_FDB_NA_LEARNT_E},
 {CPSS_PP_MAC_NA_NOT_LEARNED_E,          PRV_CPSS_ALDRIN_FDB_NA_NOT_LEARNT_E},
 {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,   PRV_CPSS_ALDRIN_FDB_AGE_VIA_TRIGGER_ENDED_E},
 {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,    PRV_CPSS_ALDRIN_FDB_AU_PROC_COMPLETED_E},
 {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,    PRV_CPSS_ALDRIN_FDB_AU_MSG_TOCPU_READY_E},
 {CPSS_PP_MAC_NA_SELF_LEARNED_E,         PRV_CPSS_ALDRIN_FDB_NA_SELF_LEARNED_E},
 {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,     PRV_CPSS_ALDRIN_FDB_N_AFROM_CPU_LEARNED_E},
 {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,     PRV_CPSS_ALDRIN_FDB_N_AFROM_CPU_DROPPED_E},
 {CPSS_PP_MAC_AGED_OUT_E,                PRV_CPSS_ALDRIN_FDB_AGED_OUT_E},
 {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,     PRV_CPSS_ALDRIN_FDB_AU_FIFO_TO_CPU_IS_FULL_E},
 {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_ALDRIN_MISC_AUQ_PENDING_E},
 {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_ALDRIN_MISC_AU_QUEUE_FULL_E},
 {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_ALDRIN_MISC_AUQ_OVERRUN_E},
 {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_ALDRIN_MISC_AUQ_ALMOST_FULL_E},
 {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_ALDRIN_MISC_FUQ_PENDING_E},
 {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_ALDRIN_MISC_FU_QUEUE_FULL_E},
 {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_ALDRIN_MISC_GENXS_READ_DMA_DONE_E},
 {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_ALDRIN_MISC_PEX_ADDR_UNMAPPED_E},
 {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_ALDRIN_BRIDGE_ADDRESS_OUT_OF_RANGE_E},
 {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,PRV_CPSS_ALDRIN_TTI_SUM_CPU_ADDRESS_OUT_OF_RANGE_E},

 {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,    PRV_CPSS_ALDRIN_IPCL_SUM_MG_ADDR_OUT_OF_RANGE_E},
 {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_ALDRIN_MISC_Z80_INTERRUPT_E},
 {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, PRV_CPSS_ALDRIN_FDB_BLC_OVERFLOW_E},/* new in Caelum; Aldrin; AC3X */

 {CPSS_PP_PEX_DL_DOWN_TX_ACC_ERR_E,      PRV_CPSS_ALDRIN_PEX_DL_DWN_TX_ACC_ERR_E},
 {CPSS_PP_PEX_MASTER_DISABLED_E,         PRV_CPSS_ALDRIN_PEX_MDIS_E},
 {CPSS_PP_PEX_ERROR_WR_TO_REG_E,         PRV_CPSS_ALDRIN_PEX_ERR_WR_TO_REG_E},
 {CPSS_PP_PEX_HIT_DEFAULT_WIN_ERR_E,     PRV_CPSS_ALDRIN_PEX_HIT_DFLT_WIN_ERR_E},
 {CPSS_PP_PEX_COR_ERROR_DET_E,           PRV_CPSS_ALDRIN_PEX_COR_ERR_DET_E},
 {CPSS_PP_PEX_NON_FATAL_ERROR_DET_E,     PRV_CPSS_ALDRIN_PEX_NF_ERR_DET_E},
 {CPSS_PP_PEX_FATAL_ERROR_DET_E,         PRV_CPSS_ALDRIN_PEX_F_ERR_DET_E},
 {CPSS_PP_PEX_DSTATE_CHANGED_E,          PRV_CPSS_ALDRIN_PEX_DSTATE_CHANGED_E},
 {CPSS_PP_PEX_BIST_E,                    PRV_CPSS_ALDRIN_PEX_BIST_E},
 {CPSS_PP_PEX_RCV_ERROR_FATAL_E,         PRV_CPSS_ALDRIN_PEX_RCV_ERR_FATAL_E},
 {CPSS_PP_PEX_RCV_ERROR_NON_FATAL_E,     PRV_CPSS_ALDRIN_PEX_RCV_ERR_NON_FATAL_E},
 {CPSS_PP_PEX_RCV_ERROR_COR_E,           PRV_CPSS_ALDRIN_PEX_RCV_ERR_COR_E},
 {CPSS_PP_PEX_RCV_CRS_E,                 PRV_CPSS_ALDRIN_PEX_RCV_CRS_E},
 {CPSS_PP_PEX_PEX_SLAVE_HOT_RESET_E,     PRV_CPSS_ALDRIN_PEX_PEX_SLV_HOT_RESET_E},
 {CPSS_PP_PEX_PEX_SLAVE_DISABLE_LINK_E,  PRV_CPSS_ALDRIN_PEX_PEX_SLV_DIS_LINK_E},
 {CPSS_PP_PEX_PEX_SLAVE_LOOPBACK_E,      PRV_CPSS_ALDRIN_PEX_PEX_SLV_LB_E},
 {CPSS_PP_PEX_PEX_LINK_FAIL_E,           PRV_CPSS_ALDRIN_PEX_PEX_LINK_FAIL_E},
 {CPSS_PP_PEX_RCV_A_E,                   PRV_CPSS_ALDRIN_PEX_RCV_A_E},
 {CPSS_PP_PEX_RCV_B_E,                   PRV_CPSS_ALDRIN_PEX_RCV_B_E},
 {CPSS_PP_PEX_RCV_C_E,                   PRV_CPSS_ALDRIN_PEX_RCV_C_E},
 {CPSS_PP_PEX_RCV_D_E,                   PRV_CPSS_ALDRIN_PEX_RCV_D_E},
 {CPSS_PP_BM_INVALID_ADDRESS_E,          PRV_CPSS_ALDRIN_MPPM_GEN_SUM_MPPM_RF_ERR_E},
 {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_ALDRIN_AP_DOORBELL_MAIN_IPC_E}
};

/* Interrupt cause to unified event map for ALDRIN without extended data size */
static GT_U32 aldrinUniEvMapTableSize = (sizeof(aldrinUniEvMapTable)/(sizeof(GT_U32)*2));


/**
* @internal setAldrinDedicatedEventsConvertInfo function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*         Aldrin devices.
* @param[in] devNum                   - device number.
* @param[in] uniEvMapTableWithExtDataPtr - pointer to unified events table
* @param[in] numUniEventsCns          - number of unified events
* @param[in] numReservedPorts         - number of ports per core
*                                       None.
*/
static void setAldrinDedicatedEventsConvertInfo
(
    IN GT_U8    devNum,
    IN GT_U32 * uniEvMapTableWithExtDataPtr,
    IN GT_U32   numUniEventsCns,
    IN GT_U32   numReservedPorts
)
{
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = uniEvMapTableWithExtDataPtr;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = numUniEventsCns;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableArr              = aldrinUniEvMapTable;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableSize             = aldrinUniEvMapTableSize;
    prvCpssDrvPpConfig[devNum]->intCtrl.numReservedPorts              = numReservedPorts;

    return;
}
/**
* @internal prvCpssDrvPpIntDefDxChAldrinInit function
* @endinternal
*
* @brief   Interrupts initialization for the Aldrin devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChAldrinInit
(
    IN GT_U8    devNum,
    IN GT_U32   ppRevision
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
                            /* pointer to device interrupt info */
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC * intrScanArrPtr;
    GT_U32 * maskRegDefaultSummaryArrPtr;
    GT_U32 * maskRegMapArrPtr;

    ppRevision = ppRevision;

    switch (prvCpssDrvPpConfig[devNum]->devType)
    {
        case CPSS_ALDRIN_DEVICES_CASES_MAC:
            devInterruptInfoPtr =
                &prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_ALDRIN_E];

            /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
            setAldrinDedicatedEventsConvertInfo(devNum, aldrinUniEvMapTableWithExtData, ALDRIN_NUM_UNI_EVENTS_CNS, 32);

            intrScanArrPtr = aldrinIntrScanArr;
            maskRegDefaultSummaryArrPtr = aldrinMaskRegDefaultSummaryArr;
            maskRegMapArrPtr = aldrinMaskRegMapArr;

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            CPSS_PP_FAMILY_DXCH_ALDRIN_E,
            ALDRIN_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
            intrScanArrPtr,
            ALDRIN_NUM_MASK_REGISTERS_CNS,
            maskRegDefaultSummaryArrPtr,
            maskRegMapArrPtr);

    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    devInterruptInfoPtr->numOfInterrupts = PRV_CPSS_ALDRIN_LAST_INT_E;
    devInterruptInfoPtr->maskRegistersAddressesArray = maskRegMapArrPtr;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = maskRegDefaultSummaryArrPtr;
    devInterruptInfoPtr->interruptsScanArray = intrScanArrPtr;
    devInterruptInfoPtr->numOfScanElements = ALDRIN_NUM_ELEMENTS_IN_SCAN_TREE_CNS;
    devInterruptInfoPtr->fdbTrigEndedId = 0;/* don't care */
    devInterruptInfoPtr->fdbTrigEndedCbPtr = NULL;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert;
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;


    return GT_OK;
}

#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefAldrinPrint function
* @endinternal
*
* @brief   print the interrupts arrays info of Aldrin devices
*/
void  prvCpssDrvPpIntDefAldrinPrint(
    void
)
{
    cpssOsPrintf("Aldrin - start : \n");
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(ALDRIN_NUM_ELEMENTS_IN_SCAN_TREE_CNS,aldrinIntrScanArr,
                            ALDRIN_NUM_MASK_REGISTERS_CNS,aldrinMaskRegDefaultSummaryArr,aldrinMaskRegMapArr);
    cpssOsPrintf("Aldrin - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefAldrinPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for Aldrin devices the register info according to value in
*         PRV_CPSS_BOBCAT2_INT_CAUSE_ENT (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefAldrinPrint_regInfoByInterruptIndex(
    IN PRV_CPSS_ALDRIN_INT_CAUSE_ENT   interruptId
)
{
    cpssOsPrintf("Aldrin - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_ALDRIN_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId , PRV_CPSS_ALDRIN_LAST_INT_E);
    }
    else
    {
        prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(ALDRIN_NUM_ELEMENTS_IN_SCAN_TREE_CNS,aldrinIntrScanArr,interruptId);
    }
    cpssOsPrintf("Aldrin - End regInfoByInterruptIndex : \n");
}
