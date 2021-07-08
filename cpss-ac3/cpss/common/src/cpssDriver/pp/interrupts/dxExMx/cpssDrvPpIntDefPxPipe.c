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
* @file cpssDrvPpIntDefPxPipe.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- PX PIPE devices
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
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsPxPipe.h>

/* set values that will be updated/filled during runtime (initialization stage)*/
#define FILLED_IN_RUNTIME_CNS       0

extern GT_STATUS prvCpssGenericSrvCpuRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);

extern GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);

/******************************************************/
/* called when the GIG/XLG/CG MAC generated interrupt */
/******************************************************/
static GT_STATUS prvCpssDrvPipeXlgGopExtUnitsIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_U32 port;
    GT_STATUS rc;

    /* read External Units Interrupt Cause register */
    rc = prvCpssDrvHwPpPortGroupIsrRead(devNum, portGroupId, regAddr, dataPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* ALL the Pipe ports have GIG MAC */

    /* emulate checking on the Gig MAC interrupt . Add this indication to output XLG register bit#2 */
    *dataPtr |= BIT_2 | BIT_0;

    port = (regAddr >> 12) & 0x1F;

    /* Check CG port interrupt summary cause */
    if ((port == 12))/*port 12 hold CG mac */
    {
        /* no need to read the CG register because is set ... we will read it again by the 'scan'
            that will make it read 'twice' (redundant!) */
        /* and if not read here ... we will read only once  by the 'scan' */

        /* CG port may got interrupt too. Add this indication to output XLG register dummy bit#8 */
        *dataPtr |= BIT_8 | BIT_0;
    }

    return GT_OK;
}

/************* START HERE **********/

/* 10180000 + 0x1000* k: where k (0-31)  */
#define PIPE_MPCS_PORT_BASE_MAC(port)                 \
        (0x10180000  + 0x1000*(port))

/* MPCS - cause */
#define PIPE_MPCS_PORT_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_MPCS_PORT_BASE_MAC(port) + (0x08))

/* MPCS - mask */
#define PIPE_MPCS_PORT_INTERRUPT_MASK_MAC(port) \
        (PIPE_MPCS_PORT_BASE_MAC(port) + (0x0C))


/* ports: 0..15 */
#define PIPE_GIGA_PORT_BASE_MAC(port) \
        (0x10000000 + 0x1000*(port))

/* ports: 0..15 */
#define PIPE_XLG_PORT_BASE_MAC(port)  \
        (0x100C0000 + 0x1000*(port))

/* XLG summary - cause */
#define PIPE_XLG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_XLG_PORT_BASE_MAC(port) + (0x58))

/* XLG summary - mask */
#define PIPE_XLG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (PIPE_XLG_PORT_BASE_MAC(port) + (0x5c))

/* XLG - cause */
#define PIPE_XLG_PORT_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_XLG_PORT_BASE_MAC(port) + (0x14))

/* XLG - mask */
#define PIPE_XLG_PORT_INTERRUPT_MASK_MAC(port) \
        (PIPE_XLG_PORT_BASE_MAC(port) + (0x18))

/* gig summary - cause */
#define PIPE_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_GIGA_PORT_BASE_MAC(port) + (0xA0))

/* gig summary - mask */
#define PIPE_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (PIPE_GIGA_PORT_BASE_MAC(port) + (0xA4))

/* gig - cause */
#define PIPE_GIGA_PORT_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_GIGA_PORT_BASE_MAC(port) + (0x20))

/* gig - mask */
#define PIPE_GIGA_PORT_INTERRUPT_MASK_MAC(port) \
        (PIPE_GIGA_PORT_BASE_MAC(port) + (0x24))

/* ports 0..15 */
#define PIPE_PTP_PORT_BASE_MAC(port)                 \
        (0x10180800 + 0x1000*(port))

/* ptp - cause */
#define PIPE_PTP_PORT_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_PTP_PORT_BASE_MAC(port) + (0x00))

/* ptp - mask */
#define PIPE_PTP_PORT_INTERRUPT_MASK_MAC(port) \
        (PIPE_PTP_PORT_BASE_MAC(port) + (0x04))

/* 0180400 + 0x1000* x: where x (0-15 in steps of 2)  */
#define PIPE_XPCS_PORT_BASE_MAC(port)                \
        (0x10180400 + 0x1000*(port))

/* base address of per port per lane registers */
#define PIPE_XPCS_PORT_PER_LANE_BASE_MAC(port,lane)                \
        (PIPE_XPCS_PORT_BASE_MAC(port) + 0x50 + 0x44*(lane))

/* XPCS - per port , per lane - cause */
#define PIPE_XPCS_PORT_LANE_SUMMARY_INTERRUPT_CAUSE_MAC(port,lane) \
        (PIPE_XPCS_PORT_PER_LANE_BASE_MAC(port,lane) + (0x10))

/* XPCS - per port , per lane - mask */
#define PIPE_XPCS_PORT_LANE_SUMMARY_INTERRUPT_MASK_MAC(port,lane) \
        (PIPE_XPCS_PORT_PER_LANE_BASE_MAC(port,lane) + (0x14))

/* XPCS - cause */
#define PIPE_XPCS_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_XPCS_PORT_BASE_MAC(port) + (0x14))
/* XPCS - mask */
#define PIPE_XPCS_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (PIPE_XPCS_PORT_BASE_MAC(port) + (0x18))

#define PIPE_CG_PORT_BASE_MAC(port)                     \
        (0x10340000 + 0x1000*(port))

/* CG summary - cause */
#define PIPE_CG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_CG_PORT_BASE_MAC(port) + (0x48))

/* CG summary - mask */
#define PIPE_CG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (PIPE_CG_PORT_BASE_MAC(port) + (0x4c))

/* CG - cause */
#define PIPE_CG_PORT_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_CG_PORT_BASE_MAC(port) + (0x40))

/* CG - mask */
#define PIPE_CG_PORT_INTERRUPT_MASK_MAC(port) \
        (PIPE_CG_PORT_BASE_MAC(port) + (0x44))

/* CG 2 - cause */
#define PIPE_CG2_PORT_INTERRUPT_CAUSE_MAC(port) \
        (PIPE_CG_PORT_BASE_MAC(port) + (0x38))

/* CG 2 - mask */
#define PIPE_CG2_PORT_INTERRUPT_MASK_MAC(port) \
        (PIPE_CG_PORT_BASE_MAC(port) + (0x3C))

/* PTP interrupts for giga/XLG port */
#define PIPE_PTP_FOR_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port)       \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,  \
              PIPE_PTP_PORT_INTERRUPT_CAUSE_MAC(port) ,                \
              PIPE_PTP_PORT_INTERRUPT_MASK_MAC(port)  ,                \
              prvCpssDrvHwPpPortGroupIsrRead,                            \
              prvCpssDrvHwPpPortGroupIsrWrite,                           \
              PRV_CPSS_PIPE_PTP_PORT_##port##_PTP_RX_FIFO_FULL_E,      \
              PRV_CPSS_PIPE_PTP_PORT_##port##_RX_SOP_IN_UNDER_RESET_E, \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


/* support XPCS interrupt summary (bit 4) : only even ports have it */
#define XPCS_BIT4_EXISTS_MAC(port)   (1-((port) & 1))
/* support CG interrupt summary (dummy bit 8) : only port 12 have it */
#define CG_PORT_EXISTS_MAC(port)   ((port) == 12)

/* summary of Giga mac that called from XLG mac */
#define PIPE_GIGA_PORT_SUMMARY_FROM_XLG_INT_SUB_TREE_MAC(port)                                \
                {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                    \
                PIPE_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) ,                            \
                PIPE_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port)  ,                            \
                prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                PRV_CPSS_PIPE_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E,                          \
                PRV_CPSS_PIPE_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E, /*see PTP issue below */ \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1/*see PTP issue below */,NULL, NULL},  \
                    /* interrupts of the giga mac */                                            \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                               \
                        PIPE_GIGA_PORT_INTERRUPT_CAUSE_MAC(port) ,                            \
                        PIPE_GIGA_PORT_INTERRUPT_MASK_MAC(port),                              \
                     prvCpssDrvHwPpPortGroupIsrRead,                                            \
                     prvCpssDrvHwPpPortGroupIsrWrite,                                           \
                     PRV_CPSS_PIPE_GIGA_PORT_##port##_LINK_STATUS_CHANGED_E,                  \
                     PRV_CPSS_PIPE_GIGA_PORT_##port##_MIB_COUNTER_WRAPAROUND_E,               \
                     FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


/* per port per lane interrupt */
#define PIPE_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(bitIndexInCaller,port,lane) \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL, \
                PIPE_XPCS_PORT_LANE_SUMMARY_INTERRUPT_CAUSE_MAC(port,lane) , \
                PIPE_XPCS_PORT_LANE_SUMMARY_INTERRUPT_MASK_MAC(port,lane) , \
                prvCpssDrvHwPpPortGroupIsrRead,                    \
                prvCpssDrvHwPpPortGroupIsrWrite,                   \
                PRV_CPSS_PIPE_XPCS_PORT_##port##_LANE_##lane##_PRBS_ERROR_E,\
                PRV_CPSS_PIPE_XPCS_PORT_##port##_LANE_##lane##_DETECTED_IIAII_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define PIPE_XPCS_FOR_PORT_INT_SUB_TREE_MAC(bitIndexInCaller,port) \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL, \
                PIPE_XPCS_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) , \
                PIPE_XPCS_PORT_SUMMARY_INTERRUPT_MASK_MAC(port)  , \
                prvCpssDrvHwPpPortGroupIsrRead,                    \
                prvCpssDrvHwPpPortGroupIsrWrite,                   \
                PRV_CPSS_PIPE_XPCS_PORT_##port##_LINK_STATUS_CHANGED_E,\
                PRV_CPSS_PIPE_XPCS_PORT_##port##_SUM_LANE3_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4,NULL, NULL},\
                    PIPE_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(7 ,port,0),\
                    PIPE_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(8 ,port,1),\
                    PIPE_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(9 ,port,2),\
                    PIPE_XPCS_FOR_PORT_LANE_INT_ELEMENT_MAC(10,port,3)

#define PIPE_CG_PORT_INT_SUB_TREE_MAC(bitIndexInCaller, port)                                       \
                /* CG - External Units Interrupts cause */                                          \
                {bitIndexInCaller, GT_FALSE, PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                        \
                PIPE_CG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port),                                     \
                PIPE_CG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port),                                      \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_PIPE_CG_PORT_12_SUM_CG_PORT_INTERRUPT_SUMMARY_E,                           \
                PRV_CPSS_PIPE_CG_PORT_12_SUM_CG2_PORT_INTERRUPT_SUMMARY_E,                          \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2 ,NULL, NULL},                             \
                    /* interrupts of the CG  */                                                     \
                    { 1 , GT_FALSE, PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                 \
                    PIPE_CG_PORT_INTERRUPT_CAUSE_MAC(port),                                         \
                    PIPE_CG_PORT_INTERRUPT_MASK_MAC(port),                                          \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_PIPE_CG_PORT_12_RX_OVERSIZE_PACKET_DROP_E,                             \
                    PRV_CPSS_PIPE_CG_PORT_12_PACKET_IS_LARGER_THAN_JUMBO_E,                         \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                         \
                    /* interrupts of the CG 2 */                                                    \
                    { 3 , GT_FALSE, PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                 \
                    PIPE_CG2_PORT_INTERRUPT_CAUSE_MAC(port),                                        \
                    PIPE_CG2_PORT_INTERRUPT_MASK_MAC(port),                                         \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_PIPE_CG_2_PORT_12_CG_RX_PREAMB_SER_ERROR_E,                            \
                    PRV_CPSS_PIPE_CG_2_PORT_12_CG_TX_FIFO_SER_ERROR_E,                              \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port)                           \
                /* XLG - External Units Interrupts cause */                                         \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                         \
                PIPE_XLG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port),                                  \
                PIPE_XLG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port),                                   \
                prvCpssDrvPipeXlgGopExtUnitsIsrRead,                                                \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_PIPE_XLG_PORT_##port##_SUM_XLG_PORT_INTERRUPT_SUMMARY_E,                 \
                PRV_CPSS_PIPE_XLG_PORT_##port##_SUM_PTP_UNIT_INTERRUPT_SUMMARY_E,                 \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4+(XPCS_BIT4_EXISTS_MAC(port))+(CG_PORT_EXISTS_MAC(port)),NULL, NULL}, \
                    /* interrupts of the XLG mac */                                                 \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    PIPE_XLG_PORT_INTERRUPT_CAUSE_MAC(port) ,                                     \
                    PIPE_XLG_PORT_INTERRUPT_MASK_MAC(port),                                       \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_PIPE_XLG_PORT_##port##_LINK_STATUS_CHANGED_E,                        \
                    PRV_CPSS_PIPE_XLG_PORT_##port##_PFC_SYNC_FIFO_OVERRUN_E,                      \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                          \
                    /*bit2*/                                                                        \
                    PIPE_GIGA_PORT_SUMMARY_FROM_XLG_INT_SUB_TREE_MAC(port),                       \
                    /* bit 4 - XPCS - will be added for even ports only */                          \
                    /* interrupts of the MPCS mac (also called GB)*/                                \
                    { 5 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    PIPE_MPCS_PORT_INTERRUPT_CAUSE_MAC(port) ,                                    \
                    PIPE_MPCS_PORT_INTERRUPT_MASK_MAC(port),                                      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_PIPE_MPCS_PORT_##port##_ACCESS_ERROR_E,                              \
                    PRV_CPSS_PIPE_MPCS_PORT_##port##_GB_LOCK_SYNC_CHANGE_E,                       \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                          \
                    /*bit 7*/                                                                       \
                    PIPE_PTP_FOR_PORT_INT_ELEMENT_MAC(7,port)                                       \
                    /* bit 8 - CG - will be added for port 12 */

/* even port hold XPCS interrupts too */
#define PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port)          \
                PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port),  \
                    PIPE_XPCS_FOR_PORT_INT_SUB_TREE_MAC(4,port)

#define PIPE_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port)            \
                PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port), \
                /* dummy bit 8 */                                                   \
                PIPE_CG_PORT_INT_SUB_TREE_MAC(8, port)

#define PIPE_PORTS_0_15_SUMMARY_INT_SUB_TREE_MAC                                    \
                                                          /*bit,port*/ \
            PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  1 ,  0 ),                \
            PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  2 ,  1 ),                \
            PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  3 ,  2 ),                \
            PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  4 ,  3 ),                \
            PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  5 ,  4 ),                \
            PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  6 ,  5 ),                \
            PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  7 ,  6 ),                \
            PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  (  8 ,  7 ),                \
            PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC (  9 ,  8 ),                \
            PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 10 ,  9 ),                \
            PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 11 , 10 ),                \
            PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 12 , 11 ),                \
            PIPE_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC   ( 13 , 12 ),                \
            PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 14 , 13 ),                \
            PIPE_XLG_EVEN_PORT_SUMMARY_INT_SUB_TREE_MAC ( 15 , 14 ),                \
            PIPE_XLG_ODD_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 16 , 15 )

#define   txqLl_fifo_NODE_MAC(bit)                                              \
    /* TXQ-LL FIFO */                                                           \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x1F11202C, 0x1F112030,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_EQ0_WRITE_LATENCY_FIFO_FULL_E,                            \
        PRV_CPSS_PIPE_EQ_CTRL_TDM_LATENCY_FIFO_OVERRUN_E,                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLl_NODE_MAC(bit)                                                   \
    /* TXQ-LL  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x1F112008, 0x1F11200C,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_ID_FIFO_OVERRUN_INT_E,                                    \
        PRV_CPSS_PIPE_BMX_NOT_READY_INT1_E,                                     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txqLl_SUB_TREE_MAC(bit)                                               \
    /* TXQ-LL  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x1F112020, 0x1F112024,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_LL_INTERRUPT_CAUSE_SUM_E,                                 \
        PRV_CPSS_PIPE_LL_FIFO_INTERRUPT_CAUSE_SUM_E,                            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                 \
        txqLl_NODE_MAC(1),                                                      \
        txqLl_fifo_NODE_MAC(2)

#define   txqQueue_SUB_TREE_MAC(bit)                                            \
    /* TXQ-queue  */                                                            \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x1E090000, 0x1E090004,\
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_TXQ_QUEUE_GENERALINT1_SUM_E,                              \
        PRV_CPSS_PIPE_TXQ_QUEUE_STATISTIC_INT_SUM_E,                            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                 \
                                                                                \
        /* TXQ-queue : txqGenIntSum  */                                         \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x1E090008, 0x1E09000C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_TXQ_GLOBAL_DESC_FULL_INT_E,                           \
            PRV_CPSS_PIPE_TXQ_TD_CLR_ECC_TWO_ERROR_DETECTED_INT_E,              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        /* TXQ-queue : txqPortDesc0IntSum  */                                   \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x1E090040, 0x1E090044,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_TXQ_DESC_FULLPORT0_E,                                 \
            PRV_CPSS_PIPE_TXQ_DESC_FULLPORT19_E,                                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff0IntSum  */                                   \
        {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x1E090060, 0x1E090064,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_TXQ_BUFF_FULLPORT0_E,                                 \
            PRV_CPSS_PIPE_TXQ_BUFF_FULLPORT19_E,                                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue :         Tail Drop Dequeue FIFO Full Interrupt Cause */   \
        {11, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x1E090078, 0x1E09007C, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_TXQ_TAIL_DROP_DQ_ULTICAST_DESC_FIFO_FULL_INT_0_E,     \
            PRV_CPSS_PIPE_TXQ_TAIL_DROP_DQ_PARAM_FIFO_FULL_INTERRUP1_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : Crossing Interrupt summary cause */                      \
        {12, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x1E090180, 0x1E090184,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_TXQ_STATISTIC_LOW_CROSSING_INT_CAUSE_QUEUE_GROUPS_0_30_SUM_E, \
            PRV_CPSS_PIPE_TXQ_STATISTIC_HIGH_CROSSING_IN_CAUSE_QUEUE_GROUPS_0_30_SUM_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                     \
            /* TXQ-queue : Low_Crossing_Int_Sum_Ports_30_to_0*/                         \
            {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x1E090080, 0x1E090084,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                \
                PRV_CPSS_PIPE_TXQ_LOW_THRESHOLD_CROSSED_FOR_QUEUES_GROUP_0_E,   \
                PRV_CPSS_PIPE_TXQ_LOW_THRESHOLD_CROSSED_FOR_QUEUES_GROUP_2_E,   \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
            /* TXQ-queue : High_Crossing_Int_Sum_Ports_30_to_0*/                \
            {4, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x1E0900B0, 0x1E0900B4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                \
                PRV_CPSS_PIPE_TXQ_HIGH_THRESHOLD_CROSSED_FOR_QUEUES_GROUP_0_E,  \
                PRV_CPSS_PIPE_TXQ_HIGH_THRESHOLD_CROSSED_FOR_QUEUES_GROUP_2_E,  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqQcn_NODE_MAC(bit)                                                  \
    /* TXQ-QCN  */                                                              \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QCN_E, NULL, 0x20000100, 0x20000110,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_TXQ_QCN_BUFFER_FIFO_OVERRUN_INT_E,                        \
        PRV_CPSS_PIPE_TXQ_QCN_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define PRV_CPSS_DXCH_UNIT_TXQ_DQ0_E     PRV_CPSS_DXCH_UNIT_TXQ_DQ_E
#define PIPE_TXQ_DQ_UNIT_OFFSET(unit)  (0x1000000 * (unit))

/* node for TxQ DQ General Interrupt Cause */
#define txqDqGeneral_NODE_MAC(bit, unit)                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x21000670 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), 0x21000680 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), \
        prvCpssDrvHwPpPortGroupIsrRead,                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                    \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_EGRESS_MIRORR_DESC_DROP_E,             \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_BURST_ABSORB_FIFO_FULL_INT_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* node for egress STC interrupts leaf */
#define EGRESS_STC_NODE_FOR_10_PORTS_MAC(bit, unit)                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x21000630 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), 0x21000640 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), \
        prvCpssDrvHwPpPortGroupIsrRead,                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                    \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_PORT0_EGRESS_SAMPLE_LOADED_INT_E,      \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_PORT9_EGRESS_SAMPLE_LOADED_INT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define txqDqMemoryError_NODE_MAC(bit, unit)                                \
    /* TXQ-DQMemoryError  */                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x21000650 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), 0x21000660 + PIPE_TXQ_DQ_UNIT_OFFSET(unit),   \
        prvCpssDrvHwPpPortGroupIsrRead,                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                    \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_TOKEN_BUCKET_PRIO_PARITY_ERROR_E,      \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_TOKEN_BUCKET_PRIO_PARITY_ERROR_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for Flush Done Interrupt Cause */
#define FLUSH_DONE_FOR_10_PORTS_MAC(bit, unit)                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x21000610 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), 0x21000620 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), \
        prvCpssDrvHwPpPortGroupIsrRead,                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                    \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_TXQ_FLUSH_DONE_PORT0_E,                \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_TXQ_FLUSH_DONE_PORT9_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define txqDq_SUB_TREE_MAC(bit, unit)                                       \
    /*  TxQ DQ Interrupt Summary Cause  */                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x21000600 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), 0x21000604 + PIPE_TXQ_DQ_UNIT_OFFSET(unit), \
        prvCpssDrvHwPpPortGroupIsrRead,                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                    \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_FLUSH_DONE_SUM_E,                      \
        PRV_CPSS_PIPE_TXQ_DQ##unit##_GENERAL_INTERRUPT_SUM_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},             \
                                                                            \
        FLUSH_DONE_FOR_10_PORTS_MAC(1, unit),                               \
        txqDqMemoryError_NODE_MAC(5, unit),                                 \
        EGRESS_STC_NODE_FOR_10_PORTS_MAC(6, unit),                          \
        txqDqGeneral_NODE_MAC(10, unit)

#define   txqPfc_SUB_TREE_MAC(bit)                                                  \
    /* TXQ-PFC  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x1A001300, 0x1A001304,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_PIPE_TXQ_PFC_PARITY_INT_SUM_E,                                     \
        PRV_CPSS_PIPE_TXQ_PFC_MSGS_SETS_OVERRUNS_EGR3_0_CAUSE_INT_SUM_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6,NULL, NULL},                      \
        /*pfcParityIntSum*/                                                         \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x1A001310, 0x1A001314,    \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E,                \
            PRV_CPSS_PIPE_TXQ_PFC_WRONG_ADDR_INT_E,                                 \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
        /*PfcCntrOverUnderFlowIntSum*/                                              \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x1A001320, 0x1A001324,    \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_OVERFLOW_INT_E,                       \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_UNDERFLOW_INT_E,                      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
        /*PfcPipe_0_GlobalOverUnderflowIntSum*/                                     \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x1A001370, 0x1A001390,    \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC0_UNDERFLOW_INT_E,       \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC7_OVERFLOW_INT_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
        /*PfcCcOverUnderFlowIntSum*/                                                \
        {4, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x1A001328, 0x1A00132C,    \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_CC_OVERFLOW_INT_E,                    \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_CC_UNDERFLOW_INT_E,                   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
        /*PfcIndFifoOverrunsIntSum*/                                                \
        {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x1A0013B0, 0x1A0013B4,    \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_FC_IND_FIFO_OVERRUN_INT_E,            \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_FC_IND_FIFO_OVERRUN_INT_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
        /*PfcIndFifoOverrunsIntSum*/                                                \
        {6, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PFC_E, NULL, 0x1A0013B8, 0x1A0013BC,    \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_QCN0_MSG_OVERRUN_INT_E,               \
            PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_EGR0_MSG_OVERRUN_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqMcfc_NODE_MAC(bit)                                                 \
    /* MCFC  */                                                                 \
    {bit, GT_FALSE,PRV_CPSS_PX_UNIT_MCFC_E, NULL, 0x1B00005C, 0x1B000060,       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_MCFC_CPU_ADDRESS_OUT_OF_RANGE_E,                          \
        PRV_CPSS_PIPE_MCFC_CPU_ADDRESS_OUT_OF_RANGE_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   cnc_SUB_TREE_MAC(bit)                                                 \
    /* CNC */                                                                   \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL, 0x07000100, 0x07000104,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_WRAPAROUND_FUNC_INTERRUPT_SUM_E,                          \
        PRV_CPSS_PIPE_MISC_FUNC_INTERRUPT_SUM_E,                                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                  \
                                                                                \
        /* WraparoundFuncInterruptSum  */                                       \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL, 0x07000190, 0x070001A4,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_CNC_BLOCK0_WRAPAROUND_E,                              \
            PRV_CPSS_PIPE_CNC_BLOCK1_WRAPAROUND_E,                              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* RateLimitFuncInterruptSum  */                                        \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL, 0x070001B8, 0x070001CC,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_BLOCK0_RATE_LIMIT_FIFO_DROP_E,                        \
            PRV_CPSS_PIPE_BLOCK1_RATE_LIMIT_FIFODROP_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* MiscFuncInterruptSum  */                                             \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL, 0x070001E0, 0x070001E4,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_DUMP_FINISHED_E,                                      \
            PRV_CPSS_PIPE_CNC_UPDATE_LOST_E,                                    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define PIPE_PPG_OFFSET(ppg)            (0x80000 * (ppg))
#define PIPE_PHA_PPN_OFFSET(ppg, ppn)   (PIPE_PPG_OFFSET(ppg) + 0x4000 * (ppn))

#define PHA_PPN_NODE_MAC(bit, ppg, ppn)                                         \
    /* PPN Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_PX_UNIT_PHA_E, NULL,                               \
        0x0F003040 + PIPE_PHA_PPN_OFFSET(ppg, ppn),                             \
        0x0F003044 + PIPE_PHA_PPN_OFFSET(ppg, ppn),                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_PPG_##ppg##_PPN_##ppn##_HOST_UNMAPPED_ACCESS_E,           \
        PRV_CPSS_PIPE_PPG_##ppg##_PPN_##ppn##_DOORBELL_INTERRUPT_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define phaPpg_SUB_TREE_MAC(bit, ppg)                                           \
    /* PPG Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_PX_UNIT_PHA_E, NULL,                               \
        0x0F07FF00 + PIPE_PPG_OFFSET(ppg),                                      \
        0x0F07FF04 + PIPE_PPG_OFFSET(ppg),                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_PPG_##ppg##_FUNCTIONAL_SUMMARY_INTERRUPT_E,               \
        PRV_CPSS_PIPE_PPG_##ppg##_PPN_7_SUMMARY_INTERRUPT_E,                    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 9, NULL, NULL},                 \
        /*  PPG Internal Error Cause */                                         \
        {1, GT_FALSE,PRV_CPSS_PX_UNIT_PHA_E, NULL,                              \
            0x0F07FF10 + PIPE_PPG_OFFSET(ppg),                                  \
            0x0F07FF14 + PIPE_PPG_OFFSET(ppg),                                  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_PPG_##ppg##_UNMAPPED_HOST_ACCESS_ERROR_E,             \
            PRV_CPSS_PIPE_PPG_##ppg##_UNMAPPED_HOST_ACCESS_ERROR_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        PHA_PPN_NODE_MAC(2, ppg, 0),                                            \
        PHA_PPN_NODE_MAC(3, ppg, 1),                                            \
        PHA_PPN_NODE_MAC(4, ppg, 2),                                            \
        PHA_PPN_NODE_MAC(5, ppg, 3),                                            \
        PHA_PPN_NODE_MAC(6, ppg, 4),                                            \
        PHA_PPN_NODE_MAC(7, ppg, 5),                                            \
        PHA_PPN_NODE_MAC(8, ppg, 6),                                            \
        PHA_PPN_NODE_MAC(9, ppg, 7)


#define   phaPpa_SUB_TREE_MAC(bit)                                              \
    /* PHA PPA */                                                               \
    {bit, GT_FALSE,PRV_CPSS_PX_UNIT_PHA_E, NULL, 0x0F7EFF10, 0x0F7EFF14,        \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_PPA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,              \
        PRV_CPSS_PIPE_PPG_3_SUMMARY_INTERRUPT_E,                                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                 \
        /* PPA Internal Error Cause */                                          \
        {1, GT_FALSE,PRV_CPSS_PX_UNIT_PHA_E, NULL, 0x0F7EFF20, 0x0F7EFF24,      \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_PPA_UNMAPPED_HOST_ACCESS_E,                           \
            PRV_CPSS_PIPE_PPA_UNMAPPED_HOST_ACCESS_E,                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        phaPpg_SUB_TREE_MAC(2, 0),                                              \
        phaPpg_SUB_TREE_MAC(3, 1),                                              \
        phaPpg_SUB_TREE_MAC(4, 2),                                              \
        phaPpg_SUB_TREE_MAC(5, 3)

#define   phaSerErr_NODE_MAC(bit)                                               \
    /* PHA SER Error Cause */                                                   \
    {bit, GT_FALSE, PRV_CPSS_PX_UNIT_PHA_E, NULL, 0x0F7FFF40, 0x0F7FFF44,       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_HEADER_SINGLE_ERROR_E,                                    \
        PRV_CPSS_PIPE_HEADER_DOUBLE_ERROR_E,                                    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   phaInternal_NODE_MAC(bit)                                             \
    /* PHA Internal Error Cause */                                              \
    {bit, GT_FALSE, PRV_CPSS_PX_UNIT_PHA_E, NULL, 0x0F7FFF48, 0x0F7FFF4C,       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_PHA_UNMAPPED_HOST_ACCESS_E,                               \
        PRV_CPSS_PIPE_PHA_UNMAPPED_HOST_ACCESS_E,                               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   pha_SUB_TREE_MAC(bit)                                                 \
    /* PHA  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_PX_UNIT_PHA_E, NULL, 0x0F7FFF60, 0x0F7FFF64,        \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_PHA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,              \
        PRV_CPSS_PIPE_PHA_SUMMARY_INTERRUPT_E,                                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                 \
        phaInternal_NODE_MAC(1),                                                \
        phaSerErr_NODE_MAC(2),                                                  \
        phaPpa_SUB_TREE_MAC(3)

#define   pcp_NODE_MAC(bit)                                                     \
    /* PCP  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_PX_UNIT_PHA_E, NULL, 0x0E005000, 0x0E005004,        \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_PCP_CPU_ADDRESS_OUT_OF_RANGE_E,                           \
        PRV_CPSS_PIPE_PCP_PACKET_TYPE_KEY_LOOK_UP_MISS_E,                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   FuncUnitsIntsSum_SUB_TREE_MAC(bit)                                    \
    /* FuncUnitsIntsSum Interrupt Cause */                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000003f8, 0x000003fc,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_TXQ_LL_INT_E,                                             \
        PRV_CPSS_PIPE_SERVER_INT_E,                                             \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 10, NULL, NULL},                         \
        txqLl_SUB_TREE_MAC(7),                                                  \
        txqQueue_SUB_TREE_MAC(8),                                               \
        txqQcn_NODE_MAC(9),                                                     \
        txqDq_SUB_TREE_MAC(10, 0),                                              \
        txqDq_SUB_TREE_MAC(11, 1),                                              \
        txqPfc_SUB_TREE_MAC(12),                                                \
        txqMcfc_NODE_MAC(13),                                                   \
        cnc_SUB_TREE_MAC(14),                                                   \
        pcp_NODE_MAC(15),                                                       \
        pha_SUB_TREE_MAC(16)

#define txFifo_SUB_TREE_MAC(bit)                                                    \
    /* txFifoIntSum */                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL, 0x04000440, 0x04000444,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_PIPE_TXFIFO_GENERAL_CAUSE_REG1_SUMMARY_E,                          \
        PRV_CPSS_PIPE_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_CAUSE_REG_1_SUMMARY_E,    \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 4 , NULL, NULL},                             \
                                                                                    \
        /* TxFIFO General Cause Reg1 */                                             \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL, 0x04000400, 0x04000404,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_LATENCY_FIFO_NOT_READY_INT_E,                             \
            PRV_CPSS_PIPE_CT_BC_IDDB_IDS_RUN_OUT_INT_E,                             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                 \
        /* Shifter SOP EOP FIFO Not Ready Interrupt Cause Reg1 */                   \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL, 0x04000408, 0x0400040C,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_INT_E,                   \
            PRV_CPSS_PIPE_SHIFTER_16_SOP_EOP_FIFO_NOT_READY_INT_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                 \
        /* CT Byte Count Arrived Late Interrupt Cause Reg1*/                        \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL, 0x04000420, 0x04000424,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_SCDMA_0_CT_BYTE_COUNT_ARRIVED_LATE_INT_E,                 \
            PRV_CPSS_PIPE_SCDMA_16_CT_BYTE_COUNT_ARRIVED_LATE_INT_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                 \
        /* CT Modified Byte Count Is Too Short Interrupt Cause Reg1 */              \
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL, 0x04000448, 0x0400044C,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_SCDMA_0_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_INT_E,        \
            PRV_CPSS_PIPE_SCDMA_16_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_INT_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* RXDMA : rxDmaScdma %n IntSum  */
#define RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(bit, port)                                 \
         /*rxDmaSum0IntSum*/                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                           \
            0x01000E08 + (4 * (port)),                                              \
            0x0100120C + (4 * (port)),                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_EOP_DESC_FIFO_OVERRUN_##port##_E,                         \
            PRV_CPSS_PIPE_PORT_##port##_CT_GAP_REJECT_INT_E,                        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   rxDmaSum0_SUB_TREE_MAC(bit)                                               \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL, 0x01001528, 0x0100152C,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_RXDMA_GENERAL_RXDMA_INTERRUPTS_SUMMARY_E,                 \
            PRV_CPSS_PIPE_RXDMA_SCDMA16_INTERRUPTS_SUM_E,                           \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 18 , NULL, NULL},                        \
                                                                                    \
           {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL, 0x01000E00, 0x01000E04,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_PIPE_RXDMA_RF_ERR_E,                                           \
            PRV_CPSS_PIPE_COMMON_BUFFER_EMPTY_GROUP0_E,                             \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                         \
                                                                                    \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 2,  0),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 3,  1),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 4,  2),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 5,  3),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 6,  4),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 7,  5),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 8,  6),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 9,  7),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(10,  8),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(11,  9),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(12, 10),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(13, 11),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(14, 12),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(15, 13),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(16, 14),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(17, 15),                               \
            RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(18, 16)

#define rxDma_SUB_TREE_MAC(bit)                                                     \
    /* rxDmaIntSum */                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL, 0x01001548, 0x0100154C,       \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_PIPE_RXDMA_INTERRUPT_SUM0_E,                                       \
        PRV_CPSS_PIPE_RXDMA_INTERRUPT_SUM0_E,                                       \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 1 , NULL, NULL},                             \
                                                                                    \
        rxDmaSum0_SUB_TREE_MAC(1)

/* BM General Cause Reg1 */
#define   bm_gen1_SUB_TREE_MAC(bit)                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x17000300, 0x17000304,          \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_PIPE_IN_PROG_CT_CLEAR_FIFO_OVERRUN_CORE_0_INTERRUPT_E,             \
        PRV_CPSS_PIPE_BM_GLOBAL_RX_FULL_INTERRUPT_E,                                \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

/* BM General Cause Reg2 */
#define   bm_gen2_SUB_TREE_MAC(bit)                                                 \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x17000308, 0x1700030C,          \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_PIPE_BM_GENERAL_INT_REG_RESERVED2_1_E,                             \
        PRV_CPSS_PIPE_BM_GLOBAL_BUFFER_COUNTER_UNDERFLOW_INTERRUPT_E,               \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   bm_SUB_TREE_MAC(bit)                                                      \
    /* bmSumIntSum */                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x17000310, 0x17000314,          \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_PIPE_BM_GENERAL_CAUSE_REG1_SUMMARY_E,                              \
        PRV_CPSS_PIPE_BM_GENERAL_CAUSE_REG2_SUMMARY_E,                              \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 2 , NULL, NULL},                             \
        /* BM General Cause Reg1 */                                                 \
        bm_gen1_SUB_TREE_MAC(1),                                                    \
        /* BM General Cause Reg2 */                                                 \
        bm_gen2_SUB_TREE_MAC(2)

#define   txDma_SUB_TREE_MAC(bit)                                                       \
    /* txdmaSumIntSum */                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA_E, NULL, 0x02002040, 0x02002044,           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_PIPE_TXDMA_SCDMAS_INTERRUPT_CAUSE_REG0_SUMMARY_E,                      \
        PRV_CPSS_PIPE_TXDMA_RXDMA_UPDATES_FIFOS_OVERRUN_INTERRUPT_CAUSE_REG1_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 3, NULL, NULL},                                  \
                                                                                        \
        /* SCDMAs Interrupt Cause Reg0 Summary */                                       \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA_E, NULL, 0x02002050, 0x02002054,         \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_PIPE_SCDMA0_INTERRUPTS_SUM_E,                                      \
            PRV_CPSS_PIPE_SCDMA16_INTERRUPTS_SUM_E,                                     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                     \
        /* TxDMA General Cause Reg1 */                                                  \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA_E, NULL, 0x02002038, 0x0200203C,         \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_PIPE_TXDMA_REQUEST_EXECUTOR_NOT_READY_E,                           \
            PRV_CPSS_PIPE_TXDMA_BANK_REQUESTS_ON_AIR_REACHED_MAXIMAL_THRESHOLD_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                     \
        /* RxDMA Updates FIFOs Overrun Interrupt Cause Reg1 */                          \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA_E, NULL, 0x02002018, 0x0200201C,         \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_PIPE_RXDMA_0_UPDATES_FIFOS_OVERRUN_INT_E,                          \
            PRV_CPSS_PIPE_RXDMA_INPROG_0_UPDATES_FIFOS_OVERRUN_INT_E,                   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   mppm_SUB_TREE_MAC(bit)                                                \
    /* mppmIntSum */                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x150021B0, 0x150021B4,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_MPPM_RX_CLIENT_0_INTERRUPTS_CAUSE_SUM_E,                  \
        PRV_CPSS_PIPE_MPPM_GENERAL_INTERRUPT_SUM_E,                             \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 6, NULL, NULL},                          \
                                                                                \
        /* mppmRxClient 0 IntSum */                                             \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x15002008, 0x1500202c,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_MPPM_RX_INT_IDDB_WR_ID_FIFO_UNDERRUN_E,               \
            PRV_CPSS_PIPE_MPPM_CREDITS_COUNTER_UNDERRUN_E,                      \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 0 IntSum */                                             \
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x15002080, 0x150020B0,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_MPPM_TX_INT_RD_BURST_FIFO_OVERRUN_E,                  \
            PRV_CPSS_PIPE_MPPM_TX_INT_RD_BURST_FIFO_OVERRUN_E,                  \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 0 IntSum */                                                 \
        {15, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x15002120, 0x15002170, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_MPPM_BANK0_INT_LATENCY_FIFO_OVERRUN_E,                \
            PRV_CPSS_PIPE_MPPM_BANK0_INT_LATENCY_FIFO_OVERRUN_E,                \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 1 IntSum */                                                 \
        {16, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x15002124, 0x15002174, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_MPPM_BANK1_INT_LATENCY_FIFO_OVERRUN_E,                \
            PRV_CPSS_PIPE_MPPM_BANK1_INT_LATENCY_FIFO_OVERRUN_E,                \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmEccIntSum */                                                     \
        {27, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x15002000, 0x15002004, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_DOUBLE_ERROR_E,                      \
            PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_SINGLE_ERROR_E,                      \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmGenIntSum */                                                     \
        {29, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x150021C0, 0x150021C4, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_PIPE_MPPM_RF_ERR_E,                                        \
            PRV_CPSS_PIPE_MPPM_RF_ERR_E,                                        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   bma_SUB_TREE_MAC(bit)                                                \
    /* bmaIntSum */                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BMA_E, NULL, 0x18059000, 0x18059004,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                       \
        PRV_CPSS_PIPE_BMA_INT_SUMMARY_E,                                       \
        PRV_CPSS_PIPE_BMA_FINAL_CLEAR_UDB_FIFO0_FULL_E,                        \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define   DataPathIntSum_SUB_TREE_MAC(bit)                                      \
    /* DataPathIntSum */                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000A4, 0x000000A8,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_PIPE_TXFIFO_INT_E,                                             \
        PRV_CPSS_PIPE_MPPM_INT_E,                                               \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 6, NULL, NULL},                          \
        txFifo_SUB_TREE_MAC(1),                                                 \
        rxDma_SUB_TREE_MAC(2),                                                  \
        bm_SUB_TREE_MAC(3),                                                     \
        bma_SUB_TREE_MAC(4),                                                    \
        txDma_SUB_TREE_MAC(5),                                                  \
        mppm_SUB_TREE_MAC(6)

#define   AP_Doorbell_SUB_TREE_MAC(bit)                                     \
    /* AP_DOORBELL_MAIN */                                                  \
    {bit,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000010, 0x00000011,    \
        prvCpssGenericSrvCpuRegisterRead,                                   \
        prvCpssGenericSrvCpuRegisterWrite,                                  \
        PRV_CPSS_PIPE_AP_DOORBELL_MAIN_IPC_E,                               \
        PRV_CPSS_PIPE_AP_DOORBELL_MAIN_RESERVED31_E,                        \
        FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0xFFFFF880, 2,NULL, NULL},       \
        /* AP_DOORBELL_PORT0_31_LINK_STATUS_CHANGE */                       \
        {4,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000012, 0x00000013,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_PIPE_AP_DOORBELL_PORT_0_LINK_STATUS_CHANGE_E,          \
            PRV_CPSS_PIPE_AP_DOORBELL_PORT_31_LINK_STATUS_CHANGE_E,         \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT0_31_802_3_AP */                                 \
        {8,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000018, 0x00000019,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_PIPE_AP_DOORBELL_PORT_0_802_3_AP_E,                    \
            PRV_CPSS_PIPE_AP_DOORBELL_PORT_31_802_3_AP_E,                   \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL}

static /*const*/ PRV_CPSS_DRV_INTERRUPT_SCAN_STC pipeIntrScanArr[] =
{
    /* Global Interrupt Cause */
    {0, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000030, 0x00000034,
        prvCpssDrvHwPpPortGroupIsrRead,
        prvCpssDrvHwPpPortGroupIsrWrite,
        PRV_CPSS_PIPE_GLOBAL_S_SUMMARY_FUNC_UNITS_SUM_E,
        PRV_CPSS_PIPE_GLOBAL_S_SUMMARY_MG1_INTERNAL_SUM_E,
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 7, NULL, NULL},

        /* NOTE: the PEX and the DFX are not connected to MG but directly
         * to the MSYS (CPU periphery) */

        FuncUnitsIntsSum_SUB_TREE_MAC(3),

        DataPathIntSum_SUB_TREE_MAC(4),

        /* ports0SumIntSum */
        {5,GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000080, 0x00000084,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_PIPE_PORTS0_SUM_PORTS_SUMMARY_E+1,
            PRV_CPSS_PIPE_PORTS0_SUM_PORTS_SUMMARY_E+1+16,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},

            /* XLG ports 0..15 bits 1..16 */
            PIPE_PORTS_0_15_SUMMARY_INT_SUB_TREE_MAC,

        /* dfxIntSum */
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000ac, 0x000000b0,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_PIPE_DFX_CLIENT_MGMT_MACRO_CORE_CLK_INT_E,
            PRV_CPSS_PIPE_DFX_CLIENT_CG_MACRO_MAC_CLK_INT_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}, /* without children yet */

        /* MG Internal */
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000038, 0x0000003C,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_PIPE_I2C_TIME_OUT_INTERRUPT_E,
            PRV_CPSS_PIPE_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},
            /* HostCpuDoorbellIntSum */
            {24, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000518, 0x0000051c,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_PIPE_CPU_DOORBELL_DATA_TO_HOST_CPU1_E,
                PRV_CPSS_PIPE_CPU_DOORBELL_DATA_TO_HOST_CPU31_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 1, NULL, NULL},

            AP_Doorbell_SUB_TREE_MAC(4),

        /* Tx SDMA  */
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, 0x00002818,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_0_E,
            PRV_CPSS_PIPE_TX_SDMA_TX_REJECT_0_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /* Rx SDMA  */
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, 0x00002814,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_0_E,
            PRV_CPSS_PIPE_RX_SDMA_PACKET_CNT_OF_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /*FuncUnits1IntsSum_SUB_TREE_MAC(11),*/
};

/* number of elements in the array of pipeIntrScanArr[] */
#define PIPE_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    (sizeof(pipeIntrScanArr)/sizeof(pipeIntrScanArr[0]))

#define PIPE_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_PIPE_LAST_INT_E / 32)

/* This array maps an interrupt index to its relevant interrupt mask reg    */
/* default value, cell i indicates the address for interrupts               */
/* (32 * i  --> 32 * (i + 1) - 1).                                          */
/* summary bits are turned on and the specific cause mask is enabled when   */
/* bounded by appl.                                                         */
static GT_U32 pipeMaskRegDefaultSummaryArr[PIPE_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};

/* This array maps an interrupt index to its    */
/* relevant interrupt mask registers address,   */
/* cell i indicates the address for interrupts  */
/* (32 * i  --> 32 * (i + 1) - 1).              */
static GT_U32 pipeMaskRegMapArr[PIPE_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};


/* state for even ports in range 0..31 that event hold extra port number */
#define SET_EVENT_PER_16_PORTS_EVEN_MAC(_prefix,_postFix) \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     0),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     2),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     4),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     6),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     8),      \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     10),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     12),     \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     14)


/* state for ports 0..15 that event hold extra port number */
#define SET_EVENT_PER_16_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_16_PORTS_EVEN_MAC(_prefix,_postFix), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     1),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     3),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     5),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     7),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     9),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     11),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     13),  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     15)

#define SET_EVENT_PER_16_PORTS_EVEN_FOR_4_LANES_MAC(_prefix,_postFix) \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     0),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     2),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     4),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     6),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     8),      \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     10),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     12),     \
    SET_EVENT_PER_PORT_FOR_4_LANES_MAC(_prefix,_postFix,     14)

/* Maximal PPN events = PPN (0..7) * PPG (0..3) */
#define PPN_EVENT_TOTAL    (8 * 4)

#define SET_PPN_EVENT_MAC(ppg, ppn)   \
    PRV_CPSS_PIPE_PPG_##ppg##_PPN_##ppn##_HOST_UNMAPPED_ACCESS_E,   ((8 + (ppg << 3)) | (ppn)),                           \
    PRV_CPSS_PIPE_PPG_##ppg##_PPN_##ppn##_CORE_UNMAPPED_ACCESS_E,   ((8 + (PPN_EVENT_TOTAL * 1) + (ppg << 3)) | (ppn)),   \
    PRV_CPSS_PIPE_PPG_##ppg##_PPN_##ppn##_NEAR_EDGE_IMEM_ACCESS_E,  ((8 + (PPN_EVENT_TOTAL * 2) + (ppg << 3)) | (ppn)),   \
    PRV_CPSS_PIPE_PPG_##ppg##_PPN_##ppn##_DOORBELL_INTERRUPT_E,     ((8 + (PPN_EVENT_TOTAL * 3) + (ppg << 3)) | (ppn))

#define SET_PPG_EVENT_PER_8_PPN_MAC(ppg)    \
    SET_PPN_EVENT_MAC(ppg, 0),          \
    SET_PPN_EVENT_MAC(ppg, 1),          \
    SET_PPN_EVENT_MAC(ppg, 2),          \
    SET_PPN_EVENT_MAC(ppg, 3),          \
    SET_PPN_EVENT_MAC(ppg, 4),          \
    SET_PPN_EVENT_MAC(ppg, 5),          \
    SET_PPN_EVENT_MAC(ppg, 6),          \
    SET_PPN_EVENT_MAC(ppg, 7)

/* state for 16 ports that event hold extra param as : ((portNum) << port_offset) |  ext_param */
#define SET_EVENT_PER_16_PORTS_WITH_PORT_OFFSET_AND_PARAM___ON_ALL_PORTS_MAC(_prefix,_postFix,ext_param,port_offset) \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 0,  ((0 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 1,  ((1 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 2,  ((2 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 3,  ((3 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 4,  ((4 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 5,  ((5 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 6,  ((6 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 7,  ((7 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 8,  ((8 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 9,  ((9 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 10, ((10 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 11, ((11 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 12, ((12 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 13, ((13 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 14, ((14 << (port_offset)) | (ext_param))),\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 15, ((15 << (port_offset)) | (ext_param)))

/* Interrupt cause to unified event map for Pipe
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static GT_U32 pipeUniEvMapTableWithExtData[] = {
 /* Per Queue events */
 CPSS_PP_TX_BUFFER_QUEUE_E,
    PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_0_E,              0,
    PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_1_E,              1,
    PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_2_E,              2,
    PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_3_E,              3,
    PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_4_E,              4,
    PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_5_E,              5,
    PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_6_E,              6,
    PRV_CPSS_PIPE_TX_SDMA_TX_BUFFER_QUEUE_7_E,              7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_ERR_QUEUE_E,
    PRV_CPSS_PIPE_TX_SDMA_TX_ERROR_QUEUE_0_E,               0,
    PRV_CPSS_PIPE_TX_SDMA_TX_ERROR_QUEUE_1_E,               1,
    PRV_CPSS_PIPE_TX_SDMA_TX_ERROR_QUEUE_2_E,               2,
    PRV_CPSS_PIPE_TX_SDMA_TX_ERROR_QUEUE_3_E,               3,
    PRV_CPSS_PIPE_TX_SDMA_TX_ERROR_QUEUE_4_E,               4,
    PRV_CPSS_PIPE_TX_SDMA_TX_ERROR_QUEUE_5_E,               5,
    PRV_CPSS_PIPE_TX_SDMA_TX_ERROR_QUEUE_6_E,               6,
    PRV_CPSS_PIPE_TX_SDMA_TX_ERROR_QUEUE_7_E,               7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_END_E,
    PRV_CPSS_PIPE_TX_SDMA_TX_END_QUEUE_0_E,                 0,
    PRV_CPSS_PIPE_TX_SDMA_TX_END_QUEUE_1_E,                 1,
    PRV_CPSS_PIPE_TX_SDMA_TX_END_QUEUE_2_E,                 2,
    PRV_CPSS_PIPE_TX_SDMA_TX_END_QUEUE_3_E,                 3,
    PRV_CPSS_PIPE_TX_SDMA_TX_END_QUEUE_4_E,                 4,
    PRV_CPSS_PIPE_TX_SDMA_TX_END_QUEUE_5_E,                 5,
    PRV_CPSS_PIPE_TX_SDMA_TX_END_QUEUE_6_E,                 6,
    PRV_CPSS_PIPE_TX_SDMA_TX_END_QUEUE_7_E,                 7,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_0_E,     0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_1_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_2_E,     2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_3_E,     3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_4_E,     4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_5_E,     5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_6_E,     6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_BUFFER_QUEUE_7_E,     7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE0_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_ERROR_QUEUE_0_E,      0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE1_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_ERROR_QUEUE_1_E,      1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE2_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_ERROR_QUEUE_2_E,      2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE3_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_ERROR_QUEUE_3_E,      3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE4_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_ERROR_QUEUE_4_E,      4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE5_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_ERROR_QUEUE_5_E,      5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE6_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_ERROR_QUEUE_6_E,      6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE7_E,
    PRV_CPSS_PIPE_RX_SDMA_RX_ERROR_QUEUE_7_E,      7,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
    /* GIGA ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_GIGA,LINK_STATUS_CHANGED_E),
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,LINK_STATUS_CHANGED_E),
    /* CG port 12 */
    SET_EVENT_PER_PORT_MAC(PRV_CPSS_PIPE_CG, LINK_STATUS_CHANGED_E, 12),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,
    /* GIGA ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_GIGA,RX_FIFO_OVERRUN_E),
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,RX_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_UNDERRUN_E,
    /* GIGA ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_GIGA,TX_UNDERRUN_E),
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,TX_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_FC_STATUS_CHANGED_E,
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,FC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_ILLEGAL_SEQUENCE_E,
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,UNKNOWN_SEQUENCE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_FAULT_TYPE_CHANGE_E,
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,FAULT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_PORT_ADDRESS_OUT_OF_RANGE_E,
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,ADDRESS_OUT_OF_RANGE_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_PORT_NO_BUFF_PACKET_DROP_E,
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,NO_BUFF_PACKET_DROP_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_COUNT_COPY_DONE_E,
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,XG_COUNT_COPY_DONE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_COUNT_EXPIRED_E,
    /* XLG ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_XLG,XG_COUNT_EXPIRED_E),
 MARK_END_OF_UNI_EV_CNS,

CPSS_PP_PORT_EEE_E,
    /* GIGA ports 0..15 */
    SET_EVENT_PER_16_PORTS_WITH_PORT_OFFSET_AND_PARAM___ON_ALL_PORTS_MAC(PRV_CPSS_PIPE_GIGA, PCS_RX_PATH_RECEIVED_LPI_E, 0, 8/*port<<8*/),
    SET_EVENT_PER_16_PORTS_WITH_PORT_OFFSET_AND_PARAM___ON_ALL_PORTS_MAC(PRV_CPSS_PIPE_GIGA, PCS_TX_PATH_RECEIVED_LPI_E, 1, 8/*port<<8*/),
    SET_EVENT_PER_16_PORTS_WITH_PORT_OFFSET_AND_PARAM___ON_ALL_PORTS_MAC(PRV_CPSS_PIPE_GIGA, MAC_RX_PATH_RECEIVED_LPI_E, 2, 8/*port<<8*/),
MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
    /* GIGA ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_GIGA,SYNC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PRBS_ERROR_E,
    /* GIGA ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_GIGA,PRBS_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,
    /* GIGA ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_GIGA,TX_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
    /* GIGA ports 0..15 */
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_GIGA,TX_FIFO_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 /* Per Port XPCS events */
 /* XPCS ports even ports between 0..15 */
 CPSS_PP_PORT_PCS_LINK_STATUS_CHANGED_E,
    SET_EVENT_PER_16_PORTS_EVEN_MAC(PRV_CPSS_PIPE_XPCS,LINK_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DESKEW_TIMEOUT_E,
    SET_EVENT_PER_16_PORTS_EVEN_MAC(PRV_CPSS_PIPE_XPCS,DESKEW_TIMEOUT_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DETECTED_COLUMN_IIAII_E,
    SET_EVENT_PER_16_PORTS_EVEN_MAC(PRV_CPSS_PIPE_XPCS,DETECTED_COLUMN_IIAII_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_DESKEW_ERROR_E,
    SET_EVENT_PER_16_PORTS_EVEN_MAC(PRV_CPSS_PIPE_XPCS,DESKEW_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_PPM_FIFO_UNDERRUN_E,
    SET_EVENT_PER_16_PORTS_EVEN_MAC(PRV_CPSS_PIPE_XPCS,PPM_FIFO_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_PPM_FIFO_OVERRUN_E,
    SET_EVENT_PER_16_PORTS_EVEN_MAC(PRV_CPSS_PIPE_XPCS,PPM_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 /* Per Port Per lane MMPCS events */
 /* MMPCS ports even ports between 0..15, 4 lanes per port*/
 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
    SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_MPCS,SIGNAL_DETECT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
   SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_MPCS,ALIGN_LOCK_LOST_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
   SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_MPCS,GB_LOCK_SYNC_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_PRBS_ERROR_E,
    SET_EVENT_PER_16_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_PIPE_XPCS,PRBS_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_DISPARITY_ERROR_E,
    SET_EVENT_PER_16_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_PIPE_XPCS,DISPARITY_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SYMBOL_ERROR_E,
    SET_EVENT_PER_16_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_PIPE_XPCS,SYMBOL_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_CJR_PAT_ERROR_E,
    SET_EVENT_PER_16_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_PIPE_XPCS,CJR_PAT_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
    SET_EVENT_PER_16_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_PIPE_XPCS,SIGNAL_DETECT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
    SET_EVENT_PER_16_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_PIPE_XPCS,SYNC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_LANE_DETECTED_IIAII_E,
    SET_EVENT_PER_16_PORTS_EVEN_FOR_4_LANES_MAC(PRV_CPSS_PIPE_XPCS,DETECTED_IIAII_E),
 MARK_END_OF_UNI_EV_CNS,

  /* Per Index events */
 CPSS_PP_GPP_E,
    PRV_CPSS_PIPE_GPP0_INTERRUPT_CAUSE_E,                       0,
    PRV_CPSS_PIPE_GPP1_INTERRUPT_CAUSE_E,                       1,
    PRV_CPSS_PIPE_GPP2_INTERRUPT_CAUSE_E,                       2,
    PRV_CPSS_PIPE_GPP3_INTERRUPT_CAUSE_E,                       3,
    PRV_CPSS_PIPE_GPP4_INTERRUPT_CAUSE_E,                       4,
    PRV_CPSS_PIPE_GPP5_INTERRUPT_CAUSE_E,                       5,
    PRV_CPSS_PIPE_GPP6_INTERRUPT_CAUSE_E,                       6,
    PRV_CPSS_PIPE_GPP7_INTERRUPT_CAUSE_E,                       7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
    PRV_CPSS_PIPE_CNC_BLOCK0_WRAPAROUND_E,                      0,
    PRV_CPSS_PIPE_CNC_BLOCK1_WRAPAROUND_E,                      1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_BM_MISC_E,
    PRV_CPSS_PIPE_BM_GLOBAL_RX_FULL_INTERRUPT_E,                4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_CNTR_OVERFLOW_E,
    PRV_CPSS_PIPE_RX_SDMA_RESOURCE_ERROR_CNT_OF_E,              0,
    PRV_CPSS_PIPE_RX_SDMA_BYTE_CNT_OF_E,                        1,
    PRV_CPSS_PIPE_RX_SDMA_PACKET_CNT_OF_E,                      2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_MISC_E,
    PRV_CPSS_PIPE_TXQ_QUEUE_WRONG_ADDR_INT_E,                   0,
    PRV_CPSS_PIPE_TXQ_GLOBAL_DESC_FULL_INT_E,                   1,
    PRV_CPSS_PIPE_TXQ_GLOBAL_BUFF_FULL_INT_E,                   2,
 MARK_END_OF_UNI_EV_CNS,

 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */
    /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */
 CPSS_PP_CRITICAL_HW_ERROR_E,
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_CREDIT_COUNTER_OVERFLOW_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_IBUF_FIFO_OUT_OVERRUN_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_CREDITS_COUNTER_UNDERRUN_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_RX_INT_LB_TOKEN_FIFO_OVERRUN_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_RX_INT_WR_DATA_TO_BANK_WITHOUT_VALID_BANK_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_RX_INT_IDDB_LATENCY_FIFO_OVERRUN_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_RX_INT_PHYSICAL_BANK_SYNC_FIFO_OVERRUN_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_RX_INT_IDDB_WR_ID_FIFO_UNDERRUN_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_TX_INT_RD_BURST_FIFO_OVERRUN_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_BANK0_INT_LATENCY_FIFO_OVERRUN_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_BANK1_INT_LATENCY_FIFO_OVERRUN_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_UNDERFLOW_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_OVERFLOW_INT_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC0_UNDERFLOW_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC1_UNDERFLOW_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC2_UNDERFLOW_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC3_UNDERFLOW_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC4_UNDERFLOW_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC5_UNDERFLOW_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC6_UNDERFLOW_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_GLOBAL_PFC_TC7_UNDERFLOW_INT_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_BM_CORE_0_INVALID_BUFFER_CLEAR_INTERRUPT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_BM_CORE_0_NEXT_TABLE_UPDATE_WAS_LOST_INTERRUPT_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_BMA_MC_CNT_COUNTER_FLOW_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_BMA_SHIFTER_LATENCY_FIFO_OVERFLOW_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_BMA_MC_CNT_PARITY_ERROR_E),


   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXDMA_RXDMA_UPDATES_FIFOS_OVERRUN_INTERRUPT_CAUSE_REG1_SUMMARY_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXDMA_TXQ_TO_TXDMA_HA_DESC_OVERRUN_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXDMA_TXQ_TO_TXDMA_INT_DESC_OVERRUN_INT_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_1_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_2_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_3_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_4_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_5_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_6_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_7_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_8_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_9_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_10_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_11_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_12_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_13_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_14_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_15_SOP_EOP_FIFO_NOT_READY_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_SHIFTER_16_SOP_EOP_FIFO_NOT_READY_INT_E),

 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_DATA_INTEGRITY_ERROR_E,
    /* -- _ECC_ --*/

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_DQ0_INC_ECC_ONE_ERROR_CORRECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_DQ0_INC_ECC_TWO_ERROR_DETECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_DQ0_CLR_ECC_ONE_ERROR_CORRECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_DQ0_CLR_ECC_TWO_ERROR_DETECTED_INT_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_DQ1_INC_ECC_ONE_ERROR_CORRECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_DQ1_INC_ECC_TWO_ERROR_DETECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_DQ1_CLR_ECC_ONE_ERROR_CORRECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_DQ1_CLR_ECC_TWO_ERROR_DETECTED_INT_E),


   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXDMA_ECC_SINGLE_ERROR_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXDMA_ECC_DOUBLE_ERROR_INT_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_TD_CLR_ECC_ONE_ERROR_CORRECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_TD_CLR_ECC_TWO_ERROR_DETECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_PFC_IND_FIFO_ECC_ERR_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_QCN_DESC_ECC_SINGLE_ERROR_DETECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_QCN_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_INTERRUPT_ECC_ERROR_CAUSE_SUM_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_DOUBLE_ERROR_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_SINGLE_ERROR_E),

   /* -- _PARITY_ -- */
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E),

   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_TXQ_QCN_BUFFER_FIFO_PARITY_ERR_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_BM_CORE_0_VALID_TABLE_PARITY_ERROR_INTERRUPT_E),

 MARK_END_OF_UNI_EV_CNS,
CPSS_PP_DATA_INTEGRITY_ERROR_E,
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_INTERRUPT_SUMMARY_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_MGMT_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_MGMT_MACRO_MBUS_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_MPPM_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_RXDMA_TXFIFO_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_TXDMA_BM_BMA_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_CP_PHA_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_CP_PHA_MACRO_PP_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_CP_ING_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_TXQ_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_MSPG_10G0_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_MSPG_10G1_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_MSPG_10G2_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_MSPG_25G3_MACRO_CORE_CLK_INT_E),
   SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_PIPE_DFX_CLIENT_CG_MACRO_MAC_CLK_INT_E),
 MARK_END_OF_UNI_EV_CNS,

    /*****************************************************************************/
    /* the MG1 not hold SDMA option ! --> so not adding the SDMA events of 'MG1' */
    /*****************************************************************************/

 CPSS_PP_PIPE_PCP_E,
   PRV_CPSS_PIPE_PCP_CPU_ADDRESS_OUT_OF_RANGE_E,      0,
   PRV_CPSS_PIPE_PCP_PACKET_TYPE_KEY_LOOK_UP_MISS_E,  1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PHA_E,
   PRV_CPSS_PIPE_PHA_UNMAPPED_HOST_ACCESS_E,          0,
   PRV_CPSS_PIPE_HEADER_SINGLE_ERROR_E,               1,
   PRV_CPSS_PIPE_HEADER_DOUBLE_ERROR_E,               2,
   PRV_CPSS_PIPE_PPA_UNMAPPED_HOST_ACCESS_E,          3,
   PRV_CPSS_PIPE_PPG_0_UNMAPPED_HOST_ACCESS_ERROR_E,  4,
   PRV_CPSS_PIPE_PPG_1_UNMAPPED_HOST_ACCESS_ERROR_E,  5,
   PRV_CPSS_PIPE_PPG_2_UNMAPPED_HOST_ACCESS_ERROR_E,  6,
   PRV_CPSS_PIPE_PPG_3_UNMAPPED_HOST_ACCESS_ERROR_E,  7,
   SET_PPG_EVENT_PER_8_PPN_MAC(0),
   SET_PPG_EVENT_PER_8_PPN_MAC(1),
   SET_PPG_EVENT_PER_8_PPN_MAC(2),
   SET_PPG_EVENT_PER_8_PPN_MAC(3),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
  SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_AP_DOORBELL,LINK_STATUS_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_SRVCPU_PORT_802_3_AP_E,
  SET_EVENT_PER_16_PORTS_MAC(PRV_CPSS_PIPE_AP_DOORBELL,802_3_AP_E),
 MARK_END_OF_UNI_EV_CNS

};
/* Interrupt cause to unified event map for PIPE with extended data size */
#define PIPE_NUM_UNI_EVENTS_CNS     (sizeof(pipeUniEvMapTableWithExtData)/sizeof(GT_U32))


/* Interrupt cause to unified event map for PIPE
 This Table is for unified event without extended data
*/
static GT_U32 pipeUniEvMapTable[][2] =
{
 {CPSS_PP_PEX_DL_DOWN_TX_ACC_ERR_E,      PRV_CPSS_PIPE_PEX_DL_DWN_TX_ACC_ERR_E},
 {CPSS_PP_PEX_MASTER_DISABLED_E,         PRV_CPSS_PIPE_PEX_MDIS_E},
 {CPSS_PP_PEX_ERROR_WR_TO_REG_E,         PRV_CPSS_PIPE_PEX_ERR_WR_TO_REG_E},
 {CPSS_PP_PEX_HIT_DEFAULT_WIN_ERR_E,     PRV_CPSS_PIPE_PEX_HIT_DFLT_WIN_ERR_E},
 {CPSS_PP_PEX_COR_ERROR_DET_E,           PRV_CPSS_PIPE_PEX_COR_ERR_DET_E},
 {CPSS_PP_PEX_NON_FATAL_ERROR_DET_E,     PRV_CPSS_PIPE_PEX_NF_ERR_DET_E},
 {CPSS_PP_PEX_FATAL_ERROR_DET_E,         PRV_CPSS_PIPE_PEX_F_ERR_DET_E},
 {CPSS_PP_PEX_DSTATE_CHANGED_E,          PRV_CPSS_PIPE_PEX_DSTATE_CHANGED_E},
 {CPSS_PP_PEX_BIST_E,                    PRV_CPSS_PIPE_PEX_BIST_E},
 {CPSS_PP_PEX_RCV_ERROR_FATAL_E,         PRV_CPSS_PIPE_PEX_RCV_ERR_FATAL_E},
 {CPSS_PP_PEX_RCV_ERROR_NON_FATAL_E,     PRV_CPSS_PIPE_PEX_RCV_ERR_NON_FATAL_E},
 {CPSS_PP_PEX_RCV_ERROR_COR_E,           PRV_CPSS_PIPE_PEX_RCV_ERR_COR_E},
 {CPSS_PP_PEX_RCV_CRS_E,                 PRV_CPSS_PIPE_PEX_RCV_CRS_E},
 {CPSS_PP_PEX_PEX_SLAVE_HOT_RESET_E,     PRV_CPSS_PIPE_PEX_PEX_SLV_HOT_RESET_E},
 {CPSS_PP_PEX_PEX_SLAVE_DISABLE_LINK_E,  PRV_CPSS_PIPE_PEX_PEX_SLV_DIS_LINK_E},
 {CPSS_PP_PEX_PEX_SLAVE_LOOPBACK_E,      PRV_CPSS_PIPE_PEX_PEX_SLV_LB_E},
 {CPSS_PP_PEX_PEX_LINK_FAIL_E,           PRV_CPSS_PIPE_PEX_PEX_LINK_FAIL_E},
 {CPSS_PP_PEX_RCV_A_E,                   PRV_CPSS_PIPE_PEX_RCV_A_E},
 {CPSS_PP_PEX_RCV_B_E,                   PRV_CPSS_PIPE_PEX_RCV_B_E},
 {CPSS_PP_PEX_RCV_C_E,                   PRV_CPSS_PIPE_PEX_RCV_C_E},
 {CPSS_PP_PEX_RCV_D_E,                   PRV_CPSS_PIPE_PEX_RCV_D_E},
 {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_PIPE_I2C_TIME_OUT_INTERRUPT_E},
 {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_PIPE_I2C_STATUS_INTERRUPT_E},
 {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_PIPE_ILLEGAL_ADDRESS_INTERRUPT_E},

 {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_PIPE_AU_QUEUE_PENDING_E},
 {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_PIPE_AU_QUEUE_FULL_E},
 {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_PIPE_AU_QUEUE_OVERRUN_E},
 {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_PIPE_AU_QUEUE_ALMOST_FULL_E},
 {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_PIPE_FU_QUEUE_PENDING_E},
 {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_PIPE_FU_QUEUE_FULL_E},
 {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_PIPE_MG_READ_DMA_DONE_E},
 {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_PIPE_PCIE_ADDRESS_UNMAPPED_E},
 {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_PIPE_Z80_INTERRUPT_E},
 {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_PIPE_AP_DOORBELL_MAIN_IPC_E}
};

/* Interrupt cause to unified event map for PIPE without extended data size */
static GT_U32 pipeUniEvMapTableSize = (sizeof(pipeUniEvMapTable)/(sizeof(GT_U32)*2));

/**
* @internal pipeIntCauseToUniEvConvertDedicatedTables function
* @endinternal
*
* @brief   Converts Interrupt Cause event to unified event type. - For PX-Pipe device.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - The port group Id.
* @param[in] intCauseIndex            - The interrupt cause to convert.
*
* @param[out] uniEvPtr                 - (pointer to)The unified event type.
* @param[out] extDataPtr               - (pointer to)The event extended data.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_NOT_FOUND             - the interrupt cause to convert was not found
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum
*
* @note code copied from chIntCauseToUniEvConvertDedicatedTables(...) .
*
*/
static GT_STATUS pipeIntCauseToUniEvConvertDedicatedTables
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      intCauseIndex,
    OUT GT_U32                      *uniEvPtr,
    OUT GT_U32                      *extDataPtr
)
{
    GT_U32   ii; /* iterator                     */
    GT_U32   *tableWithExtDataPtr;
    GT_U32   tableWithoutExtDataSize,tableWithExtDataSize;
    PRV_CPSS_DRV_EVENT_INFO_TYPE   *tableWithoutExtDataPtr;
    GT_U32   tmpUniEvent;
    GT_U32   tmpExtData;
    GT_U32   numReservedPorts;

    CPSS_NULL_PTR_CHECK_MAC(uniEvPtr);
    CPSS_NULL_PTR_CHECK_MAC(extDataPtr);

    *uniEvPtr   = CPSS_UNI_RSRVD_EVENT_E;
    *extDataPtr = 0;

    /* get the tables */
    tableWithExtDataPtr     = prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataArr;
    tableWithExtDataSize    = prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataSize;
    tableWithoutExtDataPtr  = prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableArr;
    tableWithoutExtDataSize = prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableSize;
    numReservedPorts        = prvCpssDrvPpConfig[devNum]->intCtrl.numReservedPorts;

    if(tableWithoutExtDataPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* Search the map table for unified event without extended data */
    for (ii=0; ii < tableWithoutExtDataSize;ii++)
    {
        if (tableWithoutExtDataPtr[ii][1] == intCauseIndex)
        {
            /* found */
            *uniEvPtr = tableWithoutExtDataPtr[ii][0];
            /*muliPortGroupIndicationCheckAndConvert(portGroupId,*uniEvPtr,extDataPtr);*/
            if(CPSS_PP_PORT_802_3_AP_E == *uniEvPtr)
            {
                *uniEvPtr += portGroupId;
            }
            return GT_OK;
        }
    }

    ii = 0;
    /* Search the map table for unified event with extended data */
    while (ii < tableWithExtDataSize)
    {
        /* remember the uni event */
        tmpUniEvent = tableWithExtDataPtr[ii++];
        while (tableWithExtDataPtr[ii] != MARK_END_OF_UNI_EV_CNS)
        {
            if (tableWithExtDataPtr[ii] ==  intCauseIndex)
            {
                /* found */
                *uniEvPtr = tmpUniEvent;
                tmpExtData = tableWithExtDataPtr[ii+1];
                if ((tmpUniEvent == CPSS_PP_DATA_INTEGRITY_ERROR_E) || (tmpUniEvent == CPSS_PP_CRITICAL_HW_ERROR_E) ||
                    (tmpUniEvent == CPSS_PP_PIPE_PCP_E) || (tmpUniEvent == CPSS_PP_PHA_E))
                {
                    /* These unified events use interrupt enum as extData.
                       There is no conversion is used. */
                    *extDataPtr = tmpExtData;
                }
                else if(tmpUniEvent == CPSS_PP_PORT_EEE_E)/* patch because 'port indication' not supports '<< 8' */
                {
                    GT_U32  portNum = U32_GET_FIELD_MAC(tmpExtData,8,8);/* from bit 8 take 8 bits */
                    GT_U32  extVal  = U32_GET_FIELD_MAC(tmpExtData,0,8);/* from bit 0 take 8 bits */

                    /* convert the port to 'global port' */
                    /* each port group has it's reserved ports */
                    portNum += (numReservedPorts * portGroupId);

                    /* rebuild the '*extDataPtr' */
                    *extDataPtr = extVal | (portNum << 8);
                }
                else
                {
                    *extDataPtr = CLEAR_MARK_INT_MAC(tmpExtData);
                    if (IS_MARKED_PER_PORT_PER_LANE_INT_MAC(tmpExtData))
                    {
                        /* each port group has 16 reserved ports */
                        /* each port has 256 reserved lanes */
                        *extDataPtr +=
                            (LANE_PORT_TO_EXT_DATA_CONVERT(numReservedPorts,0) * portGroupId);
                    }
                    else if (IS_MARKED_PER_PORT_INT_MAC(tmpExtData))
                    {
                       /* each port group has 16 reserved ports */
                        *extDataPtr += (numReservedPorts * portGroupId);
                    }
                }

                /*muliPortGroupIndicationCheckAndConvert(portGroupId,*uniEvPtr,extDataPtr);*/
                return GT_OK;
            }
            ii +=2;
        }
        ii++;
    }

    /*DBG_INFO(("file:%s line:%d event not found intCause = %d\n", __FILE__,
              __LINE__, intCauseIndex));*/

    return /* do not register as error to the LOG */GT_NOT_FOUND;
}

/**
* @internal setPipeDedicatedEventsConvertInfo function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*         PIPE devices.
* @param[in] devNum                   - device number.
* @param[in] uniEvMapTableWithExtDataPtr - pointer to unified events table
* @param[in] numUniEventsCns          - number of unified events
* @param[in] numReservedPorts         - number of ports per core
*                                       None.
*/
static void setPipeDedicatedEventsConvertInfo
(
    IN GT_U8    devNum,
    IN GT_U32 * uniEvMapTableWithExtDataPtr,
    IN GT_U32   numUniEventsCns,
    IN GT_U32   numReservedPorts
)
{
    /* those params used by pipeIntCauseToUniEvConvertDedicatedTables(...) */

    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = uniEvMapTableWithExtDataPtr;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = numUniEventsCns;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableArr              = pipeUniEvMapTable;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableSize             = pipeUniEvMapTableSize;
    prvCpssDrvPpConfig[devNum]->intCtrl.numReservedPorts              = numReservedPorts;

    return;
}
/**
* @internal prvCpssDrvPpIntDefPxPipeInit function
* @endinternal
*
* @brief   Interrupts initialization for the PX PIPE devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefPxPipeInit
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
        case CPSS_PIPE_ALL_DEVICES_CASES_MAC:
            devInterruptInfoPtr =
                &prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_PX_PIPE_E];

            /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
            setPipeDedicatedEventsConvertInfo(devNum, pipeUniEvMapTableWithExtData, PIPE_NUM_UNI_EVENTS_CNS, 32);

            intrScanArrPtr = pipeIntrScanArr;
            maskRegDefaultSummaryArrPtr = pipeMaskRegDefaultSummaryArr;
            maskRegMapArrPtr = pipeMaskRegMapArr;

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
            CPSS_PX_FAMILY_PIPE_E,
            PIPE_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
            intrScanArrPtr,
            PIPE_NUM_MASK_REGISTERS_CNS,
            maskRegDefaultSummaryArrPtr,
            maskRegMapArrPtr);

    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    rc = prvCpssDrvPpIntUnMappedMgSetSkip(devNum, pipeMaskRegMapArr, PIPE_NUM_MASK_REGISTERS_CNS);
    if (rc != GT_OK)
    {
        /* error */
        return rc;
    }

    devInterruptInfoPtr->numOfInterrupts = PRV_CPSS_PIPE_LAST_INT_E;
    devInterruptInfoPtr->maskRegistersAddressesArray = maskRegMapArrPtr;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = maskRegDefaultSummaryArrPtr;
    devInterruptInfoPtr->interruptsScanArray = intrScanArrPtr;
    devInterruptInfoPtr->numOfScanElements = PIPE_NUM_ELEMENTS_IN_SCAN_TREE_CNS;
    devInterruptInfoPtr->fdbTrigEndedId = 0;/* don't care */
    devInterruptInfoPtr->fdbTrigEndedCbPtr = NULL;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &pipeIntCauseToUniEvConvertDedicatedTables;
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;


    return GT_OK;
}

#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefPipePrint function
* @endinternal
*
* @brief   print the interrupts arrays info of PIPE devices
*/
void  prvCpssDrvPpIntDefPipePrint(
    void
)
{
    cpssOsPrintf("Pipe - start : \n");
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(PIPE_NUM_ELEMENTS_IN_SCAN_TREE_CNS,pipeIntrScanArr,
                            PIPE_NUM_MASK_REGISTERS_CNS,pipeMaskRegDefaultSummaryArr,pipeMaskRegMapArr);
    cpssOsPrintf("Pipe - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefPipePrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for PIPE devices the register info according to value in
*         PRV_CPSS_PIPE_INT_CAUSE_ENT (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefPipePrint_regInfoByInterruptIndex(
    IN PRV_CPSS_PIPE_INT_CAUSE_ENT   interruptId
)
{
    cpssOsPrintf("Pipe - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_PIPE_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId , PRV_CPSS_PIPE_LAST_INT_E);
    }
    else
    {
        prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(PIPE_NUM_ELEMENTS_IN_SCAN_TREE_CNS,pipeIntrScanArr,interruptId);
    }
    cpssOsPrintf("Pipe - End regInfoByInterruptIndex : \n");
}
