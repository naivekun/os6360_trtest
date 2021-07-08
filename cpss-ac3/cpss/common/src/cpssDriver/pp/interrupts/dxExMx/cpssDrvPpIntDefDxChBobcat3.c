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
* @file cpssDrvPpIntDefDxChBobcat3.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh Bobcat3 devices
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
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsBobcat3.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>

extern GT_STATUS prvCpssDrvBobKXlgGopExtUnitsIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);

GT_STATUS prvCpssDrvBobcat3XlgGopExtUnitsIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_U32 port;
    GT_STATUS rc;

    rc = prvCpssDrvBobKXlgGopExtUnitsIsrRead(devNum, portGroupId, regAddr, dataPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    port = (regAddr >> 12) & 0x3F;/* 'local to pipe' port number is 0..36 */

    /* Check CG port interrupt summary cause */
    if ((port % 4) == 0 &&
        (port <= 32))/*port 36 not hold CG mac */
    {
        /* no need to read the CG register because is set ... we will read it again by the 'scan'
            that will make it read 'twice' (redundant!) */
        /* and if not read here ... we will read only once  by the 'scan' */

        /* CG port may got interrupt too. Add this indication to output XLG register dummy bit#8 */
        *dataPtr |= BIT_8 | BIT_0;
    }

    return GT_OK;
}

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS        \
    (0xFFFFFFFF & ~                                                             \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_BOBCAT3_FDB_AGE_VIA_TRIGGER_ENDED_E) |     \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_BOBCAT3_FDB_AU_PROC_COMPLETED_E)))

#define   fdb_SUB_TREE_MAC(bit)                                                 \
    /* FDBIntSum - FDB Interrupt Cause,                                      */ \
    /*   maskRcvIntrEn - both AUProcCompletedInt(bit of PRV_CPSS_BOBK_FDB_AU_PROC_COMPLETED_E) and AgeViaTriggerEnded(bit of PRV_CPSS_BOBK_FDB_AGE_VIA_TRIGGER_ENDED_E)*/\
    /*   should never be masked to avoid missed events situation.  */           \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_FDB_E, NULL, 0x430001b0, 0x430001b4,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_FDB_NUM_OF_HOP_EX_P_E,                                 \
        PRV_CPSS_BOBCAT3_FDB_ADDRESS_OUT_OF_RANGE_E,                            \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS,                           \
             0,NULL, NULL}

#define BOBCAT3_CNC_PIPE_OFFSET_MAC(instance, pipe)                             \
             ((instance)*0x01000000 + (pipe)*0x80000000)

#define   cnc_SUB_TREE_MAC(bit,instance,pipe)                                   \
    /* CNC-0  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,                 \
        0x0E000100 + BOBCAT3_CNC_PIPE_OFFSET_MAC(instance,pipe),                \
        0x0E000104 + BOBCAT3_CNC_PIPE_OFFSET_MAC(instance,pipe),                \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_CNC##instance##_SUM_WRAPAROUND_FUNC_SUM_E, \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_CNC##instance##_SUM_MISC_FUNC_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                  \
                                                                                \
        /* WraparoundFuncInterruptSum  */                                       \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x0E000190 + BOBCAT3_CNC_PIPE_OFFSET_MAC(instance,pipe),            \
            0x0E0001A4 + BOBCAT3_CNC_PIPE_OFFSET_MAC(instance,pipe),            \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CNC##instance##_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,   \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CNC##instance##_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* RateLimitFuncInterruptSum  */                                        \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x0E0001B8 + BOBCAT3_CNC_PIPE_OFFSET_MAC(instance,pipe),            \
            0x0E0001CC + BOBCAT3_CNC_PIPE_OFFSET_MAC(instance,pipe),            \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CNC##instance##_RATE_LIMIT_SUM_BLOCK0_RATE_LIMIT_FIFO_DROP_E,  \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CNC##instance##_RATE_LIMIT_SUM_BLOCK15_RATE_LIMIT_FIFO_DROP_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* MiscFuncInterruptSum  */                                             \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x0E0001E0 + BOBCAT3_CNC_PIPE_OFFSET_MAC(instance,pipe),            \
            0x0E0001E4 + BOBCAT3_CNC_PIPE_OFFSET_MAC(instance,pipe),            \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CNC##instance##_MISC_SUM_DUMP_FINISHED_E,             \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CNC##instance##_MISC_SUM_CNC_UPDATE_LOST_E,           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define BOBCAT3_PIPE_OFFSET_MAC(pipe)                                       \
            0x80000000 * (pipe)

#define   eft_SUB_TREE_MAC(bit, pipe)                                           \
        /* eft  */                                                                  \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_EFT_E, NULL, 0x0A0010A0 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x0A0010B0 + BOBCAT3_PIPE_OFFSET_MAC(pipe),  \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_EFT_SUM_EGRESS_WRONG_ADDR_E,              \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_EFT_SUM_INC_BUS_IS_TOO_SMALL_INT_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eoam_SUB_TREE_MAC(bit, pipe)                                          \
        /* egress OAM  */                                                           \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EOAM_E, NULL, 0x180000F0 + BOBCAT3_PIPE_OFFSET_MAC(pipe) , 0x180000F4 + BOBCAT3_PIPE_OFFSET_MAC(pipe),     \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_EGR_OAM_SUM_KEEP_ALIVE_AGING_E,           \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_EGR_OAM_SUM_TX_PERIOD_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_SUB_TREE_MAC(bit, pipe)                                          \
        /* EPCL  */                                                                 \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EPCL_E, NULL, 0x16000010 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x16000014 + BOBCAT3_PIPE_OFFSET_MAC(pipe),     \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_EPCL_SUM_REGFILE_ADDRESS_ERROR_E,         \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_EPCL_SUM_REGFILE_ADDRESS_ERROR_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eplr_SUB_TREE_MAC(bit, pipe)                                          \
        /* eplr  */                                                                 \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EPLR_E, NULL, 0x17000200 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x17000204 + BOBCAT3_PIPE_OFFSET_MAC(pipe),     \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_EPLR_SUM_DATA_ERROR_E,                                  \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_EPLR_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress STC interrupts leaf */
#define INGRESS_STC_NODE_FOR_16_PORTS_MAC(bit, index, pipe) \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09000620 + BOBCAT3_PIPE_OFFSET_MAC(pipe) + (index * 0x10) , 0x09000624 + BOBCAT3_PIPE_OFFSET_MAC(pipe) + (index * 0x10), \
            prvCpssDrvHwPpPortGroupIsrRead,                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                       \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_INGRESS_STC_PORT_0_INGRESS_SAMPLE_LOADED_E   + (index * 32),    \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_INGRESS_STC_PORT_15_INGRESS_SAMPLE_LOADED_E  + (index * 32),    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define   eqIngressStc_SUB_TREE_MAC(bit, pipe)                             \
        /* Ingress STC  */                                                     \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09000600 + BOBCAT3_PIPE_OFFSET_MAC(pipe) , 0x09000604 + BOBCAT3_PIPE_OFFSET_MAC(pipe),    \
            prvCpssDrvHwPpPortGroupIsrRead,                                   \
            prvCpssDrvHwPpPortGroupIsrWrite,                                  \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_ING_STC_SUM_REG0_INGRESS_SAMPLE_LOADED_E, \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_ING_STC_SUM_REG15_INGRESS_SAMPLE_LOADED_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                \
                                                                                    \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 1,    0, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 2,    1, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 3,    2, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 4,    3, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 5,    4, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 6,    5, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 7,    6, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 8,    7, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 9,    8, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(10,    9, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(11,   10, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(12,   11, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(13,   12, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(14,   13, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(15,   14, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(16,   15, pipe)

#define   eqIngressStc1_SUB_TREE_MAC(bit, pipe)                                     \
        /* Ingress STC1 */                                                          \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09000608 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x0900060C + BOBCAT3_PIPE_OFFSET_MAC(pipe),       \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_ING_STC_1_SUM_REG16_INGRESS_SAMPLE_LOADED_E,        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_ING_STC_1_SUM_REG31_INGRESS_SAMPLE_LOADED_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                \
                                                                                    \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 1,   16, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 2,   17, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 3,   18, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 4,   19, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 5,   20, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 6,   21, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 7,   22, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 8,   23, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 9,   24, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(10,   25, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(11,   26, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(12,   27, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(13,   28, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(14,   29, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(15,   30, pipe),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(16,   31, pipe)

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(index, pipe)                          \
        {(index+1), GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09010020 + BOBCAT3_PIPE_OFFSET_MAC(pipe)  + (index * 0x10) , 0x09010024 + BOBCAT3_PIPE_OFFSET_MAC(pipe)  + (index * 0x10), \
            prvCpssDrvHwPpPortGroupIsrRead,                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER_0_PKT_DROPED_E   + (index * 32),    \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER_15_PKT_DROPED_E  + (index * 32),    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eqSctRateLimiters_SUB_TREE_MAC(bit, pipe)                             \
        /* SCT Rate Limiters */                                                 \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09010000 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x09010004 + BOBCAT3_PIPE_OFFSET_MAC(pipe),   \
            prvCpssDrvHwPpPortGroupIsrRead,                                    \
            prvCpssDrvHwPpPortGroupIsrWrite,                                   \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_SCT_RATE_LIMITERS_REG0_CPU_CODE_RATE_LIMITER_E,       \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_SCT_RATE_LIMITERS_REG15_CPU_CODE_RATE_LIMITER_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},             \
                                                                               \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(0, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(1, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(2, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(3, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(4, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(5, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(6, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(7, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(8, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(9, pipe),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(10, pipe),                       \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(11, pipe),                       \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(12, pipe),                       \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(13, pipe),                       \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(14, pipe),                       \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(15, pipe)

#define   eq_SUB_TREE_MAC(bit, pipe)                                            \
    /* Pre-Egress Interrupt Summary Cause */                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09000058 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x0900005c + BOBCAT3_PIPE_OFFSET_MAC(pipe),       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_EQ_INGRESS_STC_E,                         \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_EQ_CRITICAL_ECC_ERROR_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                 \
                                                                                \
        eqIngressStc_SUB_TREE_MAC(1, pipe),                                     \
        eqSctRateLimiters_SUB_TREE_MAC(2, pipe),                                \
                                                                                \
        /* Critical ECC Error Int  */                                           \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x090000A0 + BOBCAT3_PIPE_OFFSET_MAC(pipe) , 0x090000A4 + BOBCAT3_PIPE_OFFSET_MAC(pipe) ,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CRITICAL_ECC_ERROR_CLEAR_DESC_CRITICAL_ECC_ONE_ERROR_E,     \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_CRITICAL_ECC_ERROR_INCREMENT_DESC_CRITICAL_ECC_TWO_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
                                                                                \
        eqIngressStc1_SUB_TREE_MAC(5, pipe)

#define   ermrk_SUB_TREE_MAC(bit, pipe)                                         \
    /* ERMRK  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_ERMRK_E, NULL, 0x15000004 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x15000008 + BOBCAT3_PIPE_OFFSET_MAC(pipe),    \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_ERMRK_SUM_REGFILE_ADDRESS_ERROR_E,        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE1_FULL_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ha_SUB_TREE_MAC(bit, pipe)                                                  \
   /* HA  */                                                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_HA_E, NULL, 0x14000300 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x14000304 + BOBCAT3_PIPE_OFFSET_MAC(pipe),       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_HA_SUM_REGFILE_ADDRESS_ERROR_E,           \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_HA_SUM_OVERSIZE_HEADER_SIZE_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ioam_SUB_TREE_MAC(bit, pipe)                                          \
    /* ingress OAM  */                                                          \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IOAM_E, NULL, 0x070000F0 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x070000F4 + BOBCAT3_PIPE_OFFSET_MAC(pipe),     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_INGR_OAM_SUM_KEEP_ALIVE_AGING_E,          \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_INGR_OAM_SUM_TX_PERIOD_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   iplr0_SUB_TREE_MAC(bit, pipe)                                         \
    /* iplr0  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPLR_E, NULL, 0x05000200 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x05000204 + BOBCAT3_PIPE_OFFSET_MAC(pipe),     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_IPLR0_DATA_ERROR_E,                       \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   iplr1_SUB_TREE_MAC(bit, pipe)                                         \
    /* iplr1  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPLR_1_E, NULL, 0x06000200 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x06000204 + BOBCAT3_PIPE_OFFSET_MAC(pipe),   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_IPLR1_SUM_DATA_ERROR_E,                                   \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_IPLR1_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   router_SUB_TREE_MAC(bit, pipe)                                              \
    /* router  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPVX_E, NULL, 0x04000970 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x04000974 + BOBCAT3_PIPE_OFFSET_MAC(pipe),     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_ROUTER_I_PV_X_BAD_ADDR_E,                                 \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_ROUTER_STG13_SIPSA_RETURNS_LFIFO_FULL_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   l2i_SUB_TREE_MAC(bit, pipe)                                            \
    /* L2 Bridge  */                                                       \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_L2I_E, NULL, 0x03002100 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x03002104 + BOBCAT3_PIPE_OFFSET_MAC(pipe), \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_BRIDGE_ADDRESS_OUT_OF_RANGE_E,       \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_BRIDGE_UPDATE_SECURITY_BREACH_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   mll_SUB_TREE_MAC(bit, pipe)                                                 \
    /* MLL  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_MLL_E, NULL, 0x08000030 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x08000034 + BOBCAT3_PIPE_OFFSET_MAC(pipe),      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_MLL_SUM_INVALID_IP_MLL_ACCESS_E,                          \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_MLL_SUM_FILE_ECC_2_ERROR_E,                              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   pcl_SUB_TREE_MAC(bit, pipe)                                            \
    /* pcl  */                                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_PCL_E, NULL, 0x02000004 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x02000008 + BOBCAT3_PIPE_OFFSET_MAC(pipe), \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_IPCL_SUM_MG_ADDR_OUT_OF_RANGE_E,                   \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_IPCL_SUM_LOOKUP2_FIFO_FULL_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   tti_SUB_TREE_MAC(bit, pipe)                                            \
    /* tti  */                                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TTI_E, NULL, 0x01000004 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x01000008 + BOBCAT3_PIPE_OFFSET_MAC(pipe), \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,                       \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_TTI_PORT_PROTOCOL_TABLE_WRONG_ADDRESS_INTERRUPT_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define PRV_CPSS_DXCH_UNIT_TXQ_DQ0_E     PRV_CPSS_DXCH_UNIT_TXQ_DQ_E

#define BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe)  (0x1000000 * (unit%3) + BOBCAT3_PIPE_OFFSET_MAC(pipe))

/* node for Flush Done Interrupt Cause */
#define FLUSH_DONE_FOR_30_PORTS_MAC(bit, unit, pipe) \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000610 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe) + ((bit-1) * 4), 0x0B000620 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe) + ((bit-1) * 4), \
                prvCpssDrvHwPpPortGroupIsrRead,                                        \
                prvCpssDrvHwPpPortGroupIsrWrite,                                       \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_FLUSH_DONE_SUM_PORT_0_E + ((bit-1) * 32),    \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_FLUSH_DONE_SUM_PORT_30_E  + ((bit-1) * 32),  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* node for egress STC interrupts leaf */
#define EGRESS_STC_NODE_FOR_30_PORTS_MAC(bit, unit, pipe) \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000630 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe) + ((bit-6) * 4), 0x0B000640 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe) + ((bit-6) * 4), \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_EGR_STC_PORT_0_E + ((bit-6) * 32),           \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_EGR_STC_PORT_30_E  + ((bit-6) * 32),         \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txqDqMemoryError_NODE_MAC(bit, unit, pipe)                                                    \
    /* TXQ-DQMemoryError  */                                                                            \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000650 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe), 0x0B000660 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe),   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E,      \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for TxQ DQ General Interrupt Cause */
#define txqDqGeneral_NODE_MAC(bit, unit, pipe)                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000670 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe), 0x0B000680 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe), \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E,         \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_GEN_SUM_DQ_BURST_ABSORB_FIFO_FULL_INT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txqDq_SUB_TREE_MAC(bit, unit, pipe)                                 \
    /*  TxQ DQ Interrupt Summary Cause  */                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000600 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe), 0x0B000604 + BOBCAT3_TXQ_DQ_UNIT_PIPE_OFFSET(unit, pipe), \
        prvCpssDrvHwPpPortGroupIsrRead,                                       \
        prvCpssDrvHwPpPortGroupIsrWrite,                                      \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_SUM_FLUSH_DONE0_SUM_E,   \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_TXQ_DQ##unit##_SUM_GENERAL_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 9, NULL, NULL},               \
                                                                              \
        FLUSH_DONE_FOR_30_PORTS_MAC(1, unit, pipe),                           \
        FLUSH_DONE_FOR_30_PORTS_MAC(2, unit, pipe),                           \
        FLUSH_DONE_FOR_30_PORTS_MAC(3, unit, pipe),                           \
        FLUSH_DONE_FOR_30_PORTS_MAC(4, unit, pipe),                           \
        /* PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */     \
        /* txqDqMemoryError_NODE_MAC(5, unit, pipe)  */                       \
        EGRESS_STC_NODE_FOR_30_PORTS_MAC(6, unit, pipe),                      \
        EGRESS_STC_NODE_FOR_30_PORTS_MAC(7, unit, pipe),                      \
        EGRESS_STC_NODE_FOR_30_PORTS_MAC(8, unit, pipe),                      \
        EGRESS_STC_NODE_FOR_30_PORTS_MAC(9, unit, pipe),                      \
        txqDqGeneral_NODE_MAC(10, unit, pipe)


#define   sht_SUB_TREE_MAC(bit, pipe)                                                 \
    /* sht  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_SHT_E, NULL, 0x36020010 + BOBCAT3_PIPE_OFFSET_MAC(pipe), 0x36020020 + BOBCAT3_PIPE_OFFSET_MAC(pipe),  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_SHT_SUM_SHT_WRONG_ADDR_E,                                 \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_SHT_SUM_SPANNING_TREE_STATE_TABLE_WRONG_ADDRESS_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   qag_SUB_TREE_MAC(bit)                                                 \
    /* QAG  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_QAG_E, NULL, 0x50F00010, 0x50F00020,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_QAG_SUM_QAG_WRONG_ADDR_E,                              \
        PRV_CPSS_BOBCAT3_QAG_SUM_EPORT_ATTRIBUTES_TABLE_WRONG_ADDRESS_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define BOBCAT3_LPM_UNIT_OFFSET_MAC(unit)                                       \
        0x1000000 * (unit)

#define ETA_SUB_TREE_MAC(bit, unit, instance)                                   \
    /* same address space as LPM */                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL,                              \
        0x44E00104 + BOBCAT3_LPM_UNIT_OFFSET_MAC(unit) + (0x00100000  * (instance)),                                 \
        0x44E00100 + BOBCAT3_LPM_UNIT_OFFSET_MAC(unit) + (0x00100000  * (instance)),                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_LPM##unit##_ETA_##instance##_ETA_WRONG_ADDRESS_E,      \
        PRV_CPSS_BOBCAT3_LPM##unit##_ETA_##instance##_ADDR_REPLACEMENT_INT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   lpm_SUB_TREE_MAC(bit, unit)                                           \
    /* LPM  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL, 0x44D00100 + BOBCAT3_LPM_UNIT_OFFSET_MAC(unit), 0x44D00110 + BOBCAT3_LPM_UNIT_OFFSET_MAC(unit),      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_LPM##unit##_LPM_GENERAL_SUM_E,                         \
        PRV_CPSS_BOBCAT3_LPM##unit##_ETA_1_SUM_E,                               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                  \
        /* LPM general*/                                                        \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL, 0x44D00120 + BOBCAT3_LPM_UNIT_OFFSET_MAC(unit), 0x44D00130 + BOBCAT3_LPM_UNIT_OFFSET_MAC(unit),   \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_LPM##unit##_GENERAL_LPM_WRONG_ADDR_E,              \
            PRV_CPSS_BOBCAT3_LPM##unit##_GENERAL_AGING_CACHE_FULL_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
            /* bit 2 , instance 0 */                                            \
            ETA_SUB_TREE_MAC(2, unit, 0),                                       \
            /* bit 3 , instance 1 */                                            \
            ETA_SUB_TREE_MAC(3, unit, 1)

#define   tcam_SUB_TREE_MAC(bit)                                                \
    /* tcam  */                                                                 \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TCAM_E, NULL, 0x52501004, 0x52501000,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_TCAM_SUM_TCAM_LOGIC_SUMMARY_E,                         \
        PRV_CPSS_BOBCAT3_TCAM_SUM_TCAM_LOGIC_SUMMARY_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1,NULL, NULL},                  \
        /* tcam Logic*/                                                         \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TCAM_E, NULL, 0x5250100C, 0x52501008,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_TCAM_LOGIC_SUM_CPU_ADDRESS_OUT_OF_RANGE_E,         \
            PRV_CPSS_BOBCAT3_TCAM_LOGIC_SUM_ACTION_OOR_ADDRESS_INT_E,           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   FuncUnitsIntsSum_SUB_TREE_MAC(bit)                                    \
    /* FuncUnitsIntsSum Interrupt Cause */                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000003f8, 0x000003fc,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_FUNCTIONAL_SUMMARY_FUNC_UNITS_PIPE0_CNC0_E,            \
        PRV_CPSS_BOBCAT3_FUNCTIONAL_SUMMARY_FUNC_UNITS_MG0_1_OUT2_E,            \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 29 , NULL, NULL},                        \
        cnc_SUB_TREE_MAC(1, 0, 0),                                              \
        cnc_SUB_TREE_MAC(2, 1, 0),                                              \
        eft_SUB_TREE_MAC(3, 0),                                                 \
        eoam_SUB_TREE_MAC(4, 0),                                                \
        epcl_SUB_TREE_MAC(5, 0),                                                \
        eplr_SUB_TREE_MAC(6, 0),                                                \
        eq_SUB_TREE_MAC(7, 0),                                                  \
        ermrk_SUB_TREE_MAC(8, 0),                                               \
        ha_SUB_TREE_MAC(9, 0),                                                  \
        ioam_SUB_TREE_MAC(10, 0),                                               \
        iplr0_SUB_TREE_MAC(11,0),                                               \
        iplr1_SUB_TREE_MAC(12, 0),                                              \
        router_SUB_TREE_MAC(13, 0),                                             \
        l2i_SUB_TREE_MAC(14, 0),                                                \
        mll_SUB_TREE_MAC(15, 0),                                                \
        pcl_SUB_TREE_MAC(16, 0),                                                \
        tti_SUB_TREE_MAC(17, 0),                                                \
        txqDq_SUB_TREE_MAC(18, 0, 0),                                           \
        txqDq_SUB_TREE_MAC(19, 1, 0),                                           \
        txqDq_SUB_TREE_MAC(20, 2, 0),                                           \
        sht_SUB_TREE_MAC(21, 0),                                                \
        qag_SUB_TREE_MAC(22),                                                   \
        lpm_SUB_TREE_MAC(23, 0),                                                \
        lpm_SUB_TREE_MAC(24, 1),                                                \
        lpm_SUB_TREE_MAC(25, 2),                                                \
        lpm_SUB_TREE_MAC(26, 3),                                                \
        fdb_SUB_TREE_MAC(27),                                                   \
        tcam_SUB_TREE_MAC(28),                                                  \
        /*Mg0_1_Int_Out0 */                                                     \
        {29, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00034, 1), \
                prvCpssDrvHwPpMg1ReadReg,                                      \
                prvCpssDrvHwPpMg1WriteReg,                                     \
                PRV_CPSS_BOBCAT3_MG1_GLOBAL_SUMMARY_TX_SDMA_SUM_E,             \
                PRV_CPSS_BOBCAT3_MG1_GLOBAL_SUMMARY_RX_SDMA_SUM_E,             \
                FILLED_IN_RUNTIME_CNS, 0, 0xFFFFFFFF, 2 , NULL, NULL},                \
                /* Tx SDMA  */                                                 \
            {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002818, 1),   \
                prvCpssDrvHwPpMg1ReadReg,                                      \
                prvCpssDrvHwPpMg1WriteReg,                                     \
                PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_REJECT_0_E,                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},         \
                                                                               \
            /* Rx SDMA  */                                                     \
            {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002814, 1),   \
                prvCpssDrvHwPpMg1ReadReg,                                      \
                prvCpssDrvHwPpMg1WriteReg,                                     \
                PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_BOBCAT3_MG1_RX_SDMA_PACKET_CNT_OF_E,                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLl_NODE_MAC(bit)                                                   \
    /* TXQ-LL  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x53112008, 0x5311200C,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_LL_SUM_ID_FIFO_OVERRUN_E,                              \
        PRV_CPSS_BOBCAT3_LL_SUM_BMX_NOT_READY_INT1_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLl_fifo_NODE_MAC(bit)                                              \
    /* TXQ-LL  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x5311202C, 0x53112030,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_LL_FIFO_EQ0_Q_WR_LATENCY_FIFO_FULL_SUM_E,              \
        PRV_CPSS_BOBCAT3_LL_FIFO_EQ_CTRL_TDM_LATENCY_FIFO_OVERRUN_SUM_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLl_SUB_TREE_MAC(bit)                                               \
    /* TXQ-LL  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x53112020, 0x53112024,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_LL_SUMMARY_SUM_LL_SUM_E,                               \
        PRV_CPSS_BOBCAT3_LL_SUMMARY_SUM_LL_FIFO_INTERRUPT_CAUSE_SUMMARY_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                 \
        txqLl_NODE_MAC(1),                                                      \
        txqLl_fifo_NODE_MAC(2)

#define   txqQcn_SUB_TREE_MAC(bit)                                              \
    /* TXQ-QCN  */                                                              \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QCN_E, NULL, 0x54000100, 0x54000110,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_QCN_SUM_CN_BUFFER_FIFO_OVERRUN_E,                      \
        PRV_CPSS_BOBCAT3_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* TXQ queue: High Port Descriptor Full Interrupt Cause */
#define BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_CAUSE_MAC(index) \
        (0x550B0000 + 0x4 * (index))

/* TXQ queue: High Port Descriptor Full Interrupt Mask */
#define BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MASK_MAC(index) \
        (0x550B0040 + 0x4 * (index))

/* TXQ queue: High Port Descriptor */
#define BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC(bitIndexInCaller,index, startPort, endPort)   \
            {bitIndexInCaller, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL,       \
              BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_CAUSE_MAC(index),    \
              BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MASK_MAC(index) ,       \
              prvCpssDrvHwPpPortGroupIsrRead,                                       \
              prvCpssDrvHwPpPortGroupIsrWrite,                                      \
              PRV_CPSS_BOBCAT3_TXQ_HIGH_PORT_DESC_FULL_##index##_INTERRUPT_CAUSE_##startPort##_SUM_E, \
              PRV_CPSS_BOBCAT3_TXQ_HIGH_PORT_DESC_FULL_##index##_INTERRUPT_CAUSE_##endPort##_SUM_E, \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* TXQ queue: High Port Descriptor Full Interrupt Cause */
#define BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_CAUSE_MAC(index) \
        (0x550B0080 + 0x4 * (index))

/* TXQ queue: High Port Descriptor Full Interrupt Mask */
#define BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MASK_MAC(index) \
        (0x550B00C0 + 0x4 * (index))

/* TXQ queue: High Port Descriptor */
#define BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC(bitIndexInCaller,index, startPort, endPort)   \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, \
              BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_CAUSE_MAC(index) , \
              BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MASK_MAC(index)  , \
              prvCpssDrvHwPpPortGroupIsrRead,                    \
              prvCpssDrvHwPpPortGroupIsrWrite,                   \
              PRV_CPSS_BOBCAT3_TXQ_HIGH_PORT_BUF_FULL_##index##_INTERRUPT_CAUSE_##startPort##_SUM_E, \
              PRV_CPSS_BOBCAT3_TXQ_HIGH_PORT_BUF_FULL_##index##_INTERRUPT_CAUSE_##endPort##_SUM_E, \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txqQueue_SUB_TREE_MAC(bit)                                            \
    /* TXQ-queue  */                                                            \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090000, 0x55090004,\
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_TXQ_SUM_GENERAL1_SUM_E,                                \
        PRV_CPSS_BOBCAT3_TXQ_SUM_PORT_BUFF_FULL2_SUM_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 12, NULL, NULL},                \
                                                                                \
        /* TXQ-queue : txqGenIntSum  */                                         \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090008, 0x5509000c,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_TXQ_GEN_SUM_GLOBAL_DESC_FULL_E,                    \
            PRV_CPSS_BOBCAT3_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        /* TXQ-queue : txqPortDesc0IntSum  */                                   \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090040, 0x55090044,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_0_E,              \
            PRV_CPSS_BOBCAT3_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_30_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortDesc1IntSum  */                                   \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090048, 0x5509004C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_31_E,             \
            PRV_CPSS_BOBCAT3_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_61_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortDesc2IntSum  */                                   \
        {4, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090050, 0x55090054,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_62_E,             \
            PRV_CPSS_BOBCAT3_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_92_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff0IntSum  */                                   \
        {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090060, 0x55090064,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_TXQ_PORT_BUFF0_SUM_BUFF_FULL_PORT0_E,              \
            PRV_CPSS_BOBCAT3_TXQ_PORT_BUFF0_SUM_BUFF_FULL_PORT30_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff1IntSum  */                                   \
        {6, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090068, 0x5509006C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_TXQ_PORT_BUFF1_SUM_BUFF_FULL_PORT31_E,             \
            PRV_CPSS_BOBCAT3_TXQ_PORT_BUFF1_SUM_BUFF_FULL_PORT61_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff2IntSum  */                                   \
        {7, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090070, 0x55090074,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_TXQ_PORT_BUFF2_SUM_BUFF_FULL_PORT62_E,             \
            PRV_CPSS_BOBCAT3_TXQ_PORT_BUFF2_SUM_BUFF_FULL_PORT92_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : General */                                       \
        {8, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090010, 0x55090014,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_TXQ_GENERAL_INC_FIFO_0_FULL_INT_SUM_E,                 \
            PRV_CPSS_BOBCAT3_TXQ_GENERAL_BYPASS_BURST_FIFO_FULL_INT5_SUM_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
        /* TXQ-queue : High Port Descriptor Full Interrupt Summary Cause */         \
        {9, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550B0100, 0x550B0104,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_TXQ_HIGH_PORT_DESC_FULL_INT_SUM0_SUM_E,                \
            PRV_CPSS_BOBCAT3_TXQ_HIGH_PORT_DESC_FULL_INT_SUM13_SUM_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 14, NULL, NULL},                \
                                                                                    \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 1,  0,  93, 123), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 2,  1, 124, 154), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 3,  2, 155, 185), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 4,  3, 186, 216), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 5,  4, 217, 247), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 6,  5, 248, 278), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 7,  6, 279, 309), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 8,  7, 310, 340), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 9,  8, 341, 371), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC(10,  9, 372, 402), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC(11, 10, 403, 433), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC(12, 11, 434, 464), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC(13, 12, 465, 495), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC(14, 13, 496, 526), \
        /* TXQ-queue : High Port Buffer Full Interrupt Summary Cause */                  \
        {10, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550B0108, 0x550B010C,      \
            prvCpssDrvHwPpPortGroupIsrRead,                                       \
            prvCpssDrvHwPpPortGroupIsrWrite,                                      \
            PRV_CPSS_BOBCAT3_TXQ_HIGH_PORT_BUF_FULL_INT_SUM0_SUM_E,               \
            PRV_CPSS_BOBCAT3_TXQ_HIGH_PORT_BUF_FULL_INT_SUM13_SUM_E,              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 14, NULL, NULL},                \
                                                                                  \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 1,  0,  93, 123), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 2,  1, 124, 154), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 3,  2, 155, 185), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 4,  3, 186, 216), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 5,  4, 217, 247), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 6,  5, 248, 278), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 7,  6, 279, 309), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 8,  7, 310, 340), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 9,  8, 341, 371), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC(10,  9, 372, 402), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC(11, 10, 403, 433), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC(12, 11, 434, 464), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC(13, 12, 465, 495), \
            BOBCAT3_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC(14, 13, 496, 526), \
                                                                                  \
        /* TXQ-queue :         Tail Drop Dequeue FIFO Full Interrupt Cause */            \
        {11, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090078, 0x5509007C, \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_TXQ_DEQUEUE_FIFO_FULL_MC_DESC_FIFO_FULL0_E,            \
            PRV_CPSS_BOBCAT3_TXQ_DEQUEUE_FIFO_FULL_DQ_TD_PARAM_FIFO_FULL5_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
                                                                                    \
        /* TXQ-queue : Crossing Interrupt summary cause */                          \
        {12, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090180, 0x55090184,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_BOBCAT3_TXQ_QUEUE_STATISTIC_LOW_CROSSING_INTERRUPT_CAUSE_QUEUE_GROUPS_0_30_E,      \
            PRV_CPSS_BOBCAT3_TXQ_QUEUE_STATISTIC_HIGH_CROSSING_INTERRUPT_CAUSE_QUEUE_GROUPS_62_71_SUM_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                     \
            /* TXQ-queue : Low_Crossing_Int_Sum_Ports_30_to_0*/                         \
            {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090080, 0x55090084,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_BOBCAT3_LOW_CROSSING_SUM_PORTS_30_TO_0_LOW_THRESHOLD_CROSSED_PORT0_SUM_E,      \
                PRV_CPSS_BOBCAT3_LOW_CROSSING_SUM_PORTS_30_TO_0_LOW_THRESHOLD_CROSSED_PORT30_SUM_E,     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
            /* TXQ-queue : Low_Crossing_Int_Sum_Ports_61_to_31*/                        \
            {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090090, 0x55090094,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_BOBCAT3_LOW_CROSSING_SUM_PORTS_61_TO_31_LOW_THRESHOLD_CROSSED_PORT31_SUM_E,    \
                PRV_CPSS_BOBCAT3_LOW_CROSSING_SUM_PORTS_61_TO_31_LOW_THRESHOLD_CROSSED_PORT61_SUM_E,    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                  \
            /* TXQ-queue : Low_Crossing_Int_Sum_Ports_71_to_62*/                        \
            {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550900A0, 0x550900A4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_BOBCAT3_LOW_CROSSING_SUM_PORTS_71_TO_62_LOW_THRESHOLD_CROSSED_PORT62_SUM_E,    \
                PRV_CPSS_BOBCAT3_LOW_CROSSING_SUM_PORTS_71_TO_62_LOW_THRESHOLD_CROSSED_PORT71_SUM_E,    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
                                                                                        \
            /* TXQ-queue : High_Crossing_Int_Sum_Ports_30_to_0*/                        \
            {4, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550900B0, 0x550900B4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_BOBCAT3_HIGH_CROSSING_SUM_PORTS_30_TO_0_HIGH_THRESHOLD_CROSSED_PORT0_SUM_E,  \
                PRV_CPSS_BOBCAT3_HIGH_CROSSING_SUM_PORTS_30_TO_0_HIGH_THRESHOLD_CROSSED_PORT30_SUM_E, \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
            /* TXQ-queue : High_Crossing_Int_Sum_Ports_61_to_31*/                        \
            {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550900C0, 0x550900C4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_BOBCAT3_HIGH_CROSSING_SUM_PORTS_61_TO_31_HIGH_THRESHOLD_CROSSED_PORT31_SUM_E, \
                PRV_CPSS_BOBCAT3_HIGH_CROSSING_SUM_PORTS_61_TO_31_HIGH_THRESHOLD_CROSSED_PORT61_SUM_E, \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                              \
            /* TXQ-queue : High_Crossing_Int_Sum_Ports_71_to_62*/                       \
            {6, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550900D0, 0x550900D4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_BOBCAT3_HIGH_CROSSING_SUM_PORTS_71_TO_62_HIGH_THRESHOLD_CROSSED_PORT62_SUM_E,  \
                PRV_CPSS_BOBCAT3_HIGH_CROSSING_SUM_PORTS_71_TO_62_HIGH_THRESHOLD_CROSSED_PORT71_SUM_E, \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqBmx_NODE_MAC(bit)                                                  \
    /* TXQ-BMX  */                                                              \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x56000028, 0x5600002C,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_BMX_ALMOST_FULL_INTERRUPT_E,                           \
        PRV_CPSS_BOBCAT3_BMX_FREE_ENTRY_FIFO_SER_ERROR4_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define   FuncUnits1IntsSum_SUB_TREE_MAC(bit)                                   \
/* FuncUnits1IntsSum  */                                                        \
{bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000003F4, 0x000003F0,           \
    prvCpssDrvHwPpPortGroupIsrRead,                                             \
    prvCpssDrvHwPpPortGroupIsrWrite,                                            \
    PRV_CPSS_BOBCAT3_FUNCTIONAL1_PIPE1_CNC0_INT_E,                              \
    PRV_CPSS_BOBCAT3_FUNCTIONAL1_FUNC_UNITS_1_INT_SUM_31_E,                     \
    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 25, NULL, NULL},                    \
                                                                                \
    cnc_SUB_TREE_MAC(1, 0, 1),                                                  \
    cnc_SUB_TREE_MAC(2, 1, 1),                                                  \
    eft_SUB_TREE_MAC(3, 1),                                                     \
    eoam_SUB_TREE_MAC(4, 1),                                                    \
    epcl_SUB_TREE_MAC(5, 1),                                                    \
    eplr_SUB_TREE_MAC(6, 1),                                                    \
    eq_SUB_TREE_MAC(7, 1),                                                      \
    ermrk_SUB_TREE_MAC(8, 1),                                                   \
    ha_SUB_TREE_MAC(9, 1),                                                      \
    ioam_SUB_TREE_MAC(10, 1),                                                   \
    iplr0_SUB_TREE_MAC(11, 1),                                                  \
    iplr1_SUB_TREE_MAC(12, 1),                                                  \
    router_SUB_TREE_MAC(13, 1),                                                 \
    l2i_SUB_TREE_MAC(14, 1),                                                    \
    mll_SUB_TREE_MAC(15, 1),                                                    \
    pcl_SUB_TREE_MAC(16, 1),                                                    \
    tti_SUB_TREE_MAC(17, 1),                                                    \
    txqDq_SUB_TREE_MAC(18, 3, 1),                                               \
    txqDq_SUB_TREE_MAC(19, 4, 1),                                               \
    txqDq_SUB_TREE_MAC(20, 5, 1),                                               \
    sht_SUB_TREE_MAC(21, 1),                                                    \
    txqLl_SUB_TREE_MAC(22),                                                     \
    txqQcn_SUB_TREE_MAC(23),                                                    \
    txqQueue_SUB_TREE_MAC(24),                                                  \
    txqBmx_NODE_MAC(26)


#define BOBCAT3_GOP_PIPE_OFFSET_MAC     0x400000
/*
    0x10000000 + 0x1000*g: where g (0-36) represents Network Ports from 0 to 36
*/
#define BOBCAT3_GIGA_PORT_BASE_MAC(port, pipe)                  \
        (0x10000000 + 0x1000*(port) + BOBCAT3_GOP_PIPE_OFFSET_MAC*(pipe))

#define BOBCAT3_XLG_PORT_BASE_MAC(port, pipe)                   \
        (0x100C0000 + 0x1000*(port) + BOBCAT3_GOP_PIPE_OFFSET_MAC*(pipe))

#define BOBCAT3_CG_PORT_BASE_MAC(port, pipe)                   \
        (0x10340000 + 0x1000*(port) + BOBCAT3_GOP_PIPE_OFFSET_MAC*(pipe))

/* XLG summary - cause */
#define BOBCAT3_XLG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port, pipe) \
        (BOBCAT3_XLG_PORT_BASE_MAC(port, pipe) + (0x58))

/* XLG summary - mask */
#define BOBCAT3_XLG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port, pipe) \
        (BOBCAT3_XLG_PORT_BASE_MAC(port, pipe) + (0x5c))

/* XLG - cause */
#define BOBCAT3_XLG_PORT_INTERRUPT_CAUSE_MAC(port, pipe) \
        (BOBCAT3_XLG_PORT_BASE_MAC(port, pipe) + (0x14))

/* XLG - mask */
#define BOBCAT3_XLG_PORT_INTERRUPT_MASK_MAC(port, pipe) \
        (BOBCAT3_XLG_PORT_BASE_MAC(port, pipe) + (0x18))

/* gig summary - cause */
#define BOBCAT3_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port, pipe) \
        (BOBCAT3_GIGA_PORT_BASE_MAC(port, pipe) + (0xA0))

/* gig summary - mask */
#define BOBCAT3_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port, pipe) \
        (BOBCAT3_GIGA_PORT_BASE_MAC(port, pipe) + (0xA4))

/* gig - cause */
#define BOBCAT3_GIGA_PORT_INTERRUPT_CAUSE_MAC(port, pipe) \
        (BOBCAT3_GIGA_PORT_BASE_MAC(port, pipe) + (0x20))

/* gig - mask */
#define BOBCAT3_GIGA_PORT_INTERRUPT_MASK_MAC(port, pipe) \
        (BOBCAT3_GIGA_PORT_BASE_MAC(port, pipe) + (0x24))

/* CG summary - cause */
#define BOBCAT3_CG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port, pipe) \
        (BOBCAT3_CG_PORT_BASE_MAC(port, pipe) + (0x48))

/* CG summary - mask */
#define BOBCAT3_CG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port, pipe) \
        (BOBCAT3_CG_PORT_BASE_MAC(port, pipe) + (0x4c))

/* CG - cause */
#define BOBCAT3_CG_PORT_INTERRUPT_CAUSE_MAC(port, pipe) \
        (BOBCAT3_CG_PORT_BASE_MAC(port, pipe) + (0x40))

/* CG - mask */
#define BOBCAT3_CG_PORT_INTERRUPT_MASK_MAC(port, pipe) \
        (BOBCAT3_CG_PORT_BASE_MAC(port, pipe) + (0x44))

/*
    0x10180800 + 0x1000*t: where t (0-36) represents Network PTP
*/
#define BOBCAT3_PTP_PORT_BASE_MAC(port, pipe)                 \
        (0x10180800 + 0x1000*(port) + BOBCAT3_GOP_PIPE_OFFSET_MAC*(pipe))

/* ptp - cause */
#define BOBCAT3_PTP_PORT_INTERRUPT_CAUSE_MAC(port, pipe) \
        (BOBCAT3_PTP_PORT_BASE_MAC(port, pipe) + (0x00))

/* ptp - mask */
#define BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) \
        (BOBCAT3_PTP_PORT_BASE_MAC(port, pipe) + (0x04))

/* PTP interrupts for giga/XLG port */
#define BOBCAT3_PTP_FOR_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port, pipe)   \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL, \
              BOBCAT3_PTP_PORT_INTERRUPT_CAUSE_MAC(port, pipe) , \
              BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe)  , \
              prvCpssDrvHwPpPortGroupIsrRead,                    \
              prvCpssDrvHwPpPortGroupIsrWrite,                   \
              PRV_CPSS_BOBCAT3_PIPE##pipe##_PTP_PORT_##port##_PTP_RX_TOD_SYNC_OND_OVERRUN_SYNC_E, \
              PRV_CPSS_BOBCAT3_PIPE##pipe##_PTP_PORT_##port##_MIB_FRAGMENT_INT_E,\
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define BOBCAT3_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) \
        (1/*not valid address*/ | (BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) ))

/* the fake node needed to skip 'duplication' in the tree checked by : prvCpssDrvExMxDxHwPpMaskRegInfoGet(...)  --> maskRegInfoGet(...) */
/* after calling prvCpssDrvExMxDxHwPpMaskRegInfoGet */
/* 1. replace the bobcat3MaskRegMapArr[] with CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS */
/* 2. replace the BOBCAT3_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) with BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) */
#define BOBCAT3_FAKE_PTP_FOR_CG_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port, pipe)   \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL, \
              BOBCAT3_PTP_PORT_INTERRUPT_CAUSE_MAC(port, pipe) , \
              BOBCAT3_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe)  , \
              prvCpssDrvHwPpPortGroupIsrRead,                    \
              prvCpssDrvHwPpPortGroupIsrWrite,                   \
              PRV_CPSS_BOBCAT3_PIPE##pipe##_PTP_PORT_##port##_PTP_RX_TOD_SYNC_OND_OVERRUN_SYNC_E, \
              PRV_CPSS_BOBCAT3_PIPE##pipe##_PTP_PORT_##port##_MIB_FRAGMENT_INT_E,\
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}



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

#define PRV_CPSS_DXCH_UNIT_RXDMA0_E         PRV_CPSS_DXCH_UNIT_RXDMA_E

#define PRV_BOBCAT3_RX_DMA_ADDR(addr, pipe, unit) \
        (addr) + 0x1000000 * (unit%3) + 0x80000000  * (pipe)

/* RXDMA : rxDmaScdma %n IntSum  */
#define BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(bit, port, unit, pipe)             \
         /*rxDmaSum0IntSum*/                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL,                   \
            PRV_BOBCAT3_RX_DMA_ADDR(0x19000E08, pipe, unit) + 4 * (port),           \
            PRV_BOBCAT3_RX_DMA_ADDR(0x1900120C, pipe, unit) + 4 * (port),           \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_SCDMA_##port##_SUM_EOP_DESC_FIFO_OVERRUN_E, \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_SCDMA_##port##_SUM_OS_DROP_E,               \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   rxDmaSum0_NODE_MAC(bit, pipe, unit)                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL, PRV_BOBCAT3_RX_DMA_ADDR(0x19001528, pipe, unit), PRV_BOBCAT3_RX_DMA_ADDR(0x1900152C, pipe, unit), \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_SUM_GENERAL_RX_DMA_S_SUMMARY_E,  \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_SUM_SC_DMA12_S_SUM_E,      \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 14 , NULL, NULL},                        \
                                                                                    \
           {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL, PRV_BOBCAT3_RX_DMA_ADDR(0x19000E00, pipe, unit), PRV_BOBCAT3_RX_DMA_ADDR(0x19000E04, pipe, unit), \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_INT0_RX_DMA_RF_ERR_E,      \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_INT0_COMMON_BUFFER_EMPTY5_E,  \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                         \
                                                                                    \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 2,  0, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 3,  1, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 4,  2, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 5,  3, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 6,  4, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 7,  5, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 8,  6, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 9,  7, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(10,  8, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(11,  9, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(12, 10, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(13, 11, unit, pipe),           \
            BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(14, 12, unit, pipe)



#define rxDma_SUB_TREE_MAC(bit, pipe, unit)                                                      \
    /* rxDmaIntSum */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL,                                          \
        PRV_BOBCAT3_RX_DMA_ADDR(0x19001548, pipe, unit), PRV_BOBCAT3_RX_DMA_ADDR(0x1900154c, pipe, unit),   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_SUM_RX_DMA_INTERRUPT_SUMMARY0_E,                   \
        PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_SUM_RX_DMA_INTERRUPT_SUMMARY0_E,                   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 1 , NULL, NULL},                                                 \
                                                                                                        \
        rxDmaSum0_NODE_MAC(1, pipe, unit)

#define PRV_BOBCAT3_TX_DMA_ADDR(addr, pipe, unit) \
        addr + 0x1000000 * (unit%3) + 0x80000000  * (pipe)

#define PRV_CPSS_DXCH_UNIT_TXDMA0_E         PRV_CPSS_DXCH_UNIT_TXDMA_E

/* TXDMA: SCDMA %p interrupts Cause */
#define BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(bit, port, unit, pipe)             \
         /*txDmaSum0IntSum*/                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                   \
            PRV_BOBCAT3_TX_DMA_ADDR(0x1C002100, pipe, unit) + 4 * (port),           \
            PRV_BOBCAT3_TX_DMA_ADDR(0x1C002300, pipe, unit) + 4 * (port),           \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_SCDMA_##port##_CT_SM_BC_BANKS_NUMBER_MISMATCH_INT_E, \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_SCDMA_##port##_ID_FIFO_OVERRUN_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define BOBCAT3_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(bit, port, unit, pipe)             \
         /*rxDmaSum0IntSum*/                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL,                   \
            PRV_BOBCAT3_RX_DMA_ADDR(0x19000E08, pipe, unit) + 4 * (port),           \
            PRV_BOBCAT3_RX_DMA_ADDR(0x1900120C, pipe, unit) + 4 * (port),           \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_SCDMA_##port##_SUM_EOP_DESC_FIFO_OVERRUN_E, \
            PRV_CPSS_BOBCAT3_PIPE##pipe##_RX_DMA##unit##_SCDMA_##port##_SUM_OS_DROP_E,               \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define txDma_SUB_TREE_MAC(bit, pipe, unit)                                                                     \
            /* txDmaIntSum */                                                                                   \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                                           \
                PRV_BOBCAT3_TX_DMA_ADDR(0x1C002040, pipe, unit), PRV_BOBCAT3_TX_DMA_ADDR(0x1C002044, pipe, unit),   \
                prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_SUM_SCDMAS_INTERRUPT_CAUSE_REG0_SUMMARY_E,         \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_SUM_RXDMA_UPDATES_FIFOS_OVERRUN_INTERRUPT_CAUSE_REG1_SUMMARY_E, \
                FILLED_IN_RUNTIME_CNS, 0, 0x0, 3 , NULL, NULL},                                                 \
                                                                                                                \
                /* SCDMAs Interrupt Cause Reg0 Summary */                                                       \
                {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                                         \
                    PRV_BOBCAT3_TX_DMA_ADDR(0x1C002050, pipe, unit), PRV_BOBCAT3_TX_DMA_ADDR(0x1C002054 , pipe, unit),           \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_SCDMA0_INTERRUPTS_SUM_SUM_E,                   \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_SCDMA12_INTERRUPTS_SUM_SUM_E,                  \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 13, NULL, NULL},                                    \
                                                                                                                \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 1,    0,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 2,    1,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 3,    2,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 4,    3,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 5,    4,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 6,    5,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 7,    6,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 8,    7,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 9,    8,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(10,    9,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(11,   10,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(12,   11,  unit, pipe),                            \
                    BOBCAT3_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(13,   12,  unit, pipe),                            \
                                                                                                                \
                /* TxDMA General Cause Reg1 */                                                                  \
                {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                                        \
                    PRV_BOBCAT3_TX_DMA_ADDR(0x1C002038, pipe, unit), PRV_BOBCAT3_TX_DMA_ADDR(0x1C00203C , pipe, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_GENENAL_REQUEST_EXECUTOR_NOT_READY_E,                   \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_GENENAL_BANK_REQUESTS_ON_AIR_REACHED_MAXIMAL_THRESHOLD_E,                  \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                /* RxDMA Updates FIFOs Overrun Interrupt Cause Reg1 */                                          \
                {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                                        \
                    PRV_BOBCAT3_TX_DMA_ADDR(0x1C002018, pipe, unit), PRV_BOBCAT3_TX_DMA_ADDR(0x1C00201C , pipe, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_XDMA_0_UPDATES_FIFOS_OVERRUN_INT_SUM_E,         \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_DMA##unit##_NPROG_5_UPDATES_FIFOS_OVERRUN_INT_SUM_E,        \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define PRV_BOBCAT3_TX_FIFO_ADDR(addr, pipe, unit) \
        addr + 0x1000000 * (unit%3) + 0x80000000  * (pipe)

#define PRV_CPSS_DXCH_UNIT_TX_FIFO0_E         PRV_CPSS_DXCH_UNIT_TX_FIFO_E

#define txFifo_SUB_TREE_MAC(bit, pipe, unit)                                                             \
            /* txFifoIntSum */                                                                                  \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                         \
                PRV_BOBCAT3_TX_FIFO_ADDR(0x1F000440, pipe, unit), PRV_BOBCAT3_TX_FIFO_ADDR(0x1F000444, pipe, unit),   \
                prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_TX_FIFO_GENERAL_REG1_SUMMARY_E,        \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_CAUSE_REG_1_SUMMARY_E, \
                FILLED_IN_RUNTIME_CNS, 0, 0x0, 4 , NULL, NULL},                                                 \
                                                                                                                \
                /* TxFIFO General Cause Reg1 */                                                                 \
                {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                       \
                    PRV_BOBCAT3_TX_FIFO_ADDR(0x1F000400, pipe, unit), PRV_BOBCAT3_TX_FIFO_ADDR(0x1F000404 , pipe, unit),           \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_GENERAL1_LATENCY_FIFO_NOT_READY_E,            \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_GENERAL1_CT_BC_IDDB_I_DS_RUN_OUT_E,           \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                /* Shifter SOP EOP FIFO Not Ready Interrupt Cause Reg1 */                                       \
                {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                       \
                    PRV_BOBCAT3_TX_FIFO_ADDR(0x1F000408, pipe, unit), PRV_BOBCAT3_TX_FIFO_ADDR(0x1F00040C , pipe, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_NOT_READY1_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E,                   \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_NOT_READY1_SHIFTER_12_SOP_EOP_FIFO_NOT_READY_E,                  \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                /* CT Byte Count Arrived Late Interrupt Cause Reg1*/                                            \
                {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                       \
                    PRV_BOBCAT3_TX_FIFO_ADDR(0x1F000420, pipe, unit), PRV_BOBCAT3_TX_FIFO_ADDR(0x1F000424 , pipe, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_SCDMA0_CT_BYTE_COUNT_ARRIVED_LATE_INT_SUM_E,  \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_SCDMA12_CT_BYTE_COUNT_ARRIVED_LATE_INT_SUM_E, \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                /* CT Modified Byte Count Is Too Short Interrupt Cause Reg1 */                                  \
                {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                       \
                    PRV_BOBCAT3_TX_FIFO_ADDR(0x1F000448, pipe, unit), PRV_BOBCAT3_TX_FIFO_ADDR(0x1F00044C , pipe, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_SCDMA0_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_INT_SUM_E,         \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_TX_FIFO##unit##_SCDMA12_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_INT_SUM_E,        \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* BM General Cause Reg1 */
#define   bm_gen1_SUB_TREE_MAC(bit)                                                         \
                {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x40000300, 0x40000304,      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                         \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                    PRV_CPSS_BOBCAT3_BM_GEN1_SUM_IN_PROG_CT_CLEAR_FIFO_OVERRUN_CORE_0_E,    \
                    PRV_CPSS_BOBCAT3_BM_GEN1_SUM_GLOBAL_RX_FULL_E,                          \
                    FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

/* BM General Cause Reg2 */
#define   bm_gen2_SUB_TREE_MAC(bit)                                                         \
                {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x40000308, 0x4000030C,      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                         \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                    PRV_CPSS_BOBCAT3_BM_GEN2_BUFFER_0_TERMINATION_COMPLETED_E,              \
                    PRV_CPSS_BOBCAT3_BM_GEN2_GLOBAL_BUFFER_COUNTER_UNDERFLOW_INTERRUPT_E,   \
                    FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   bm_SUB_TREE_MAC(bit)                                                         \
                /* bmSumIntSum */                                                      \
                {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x40000310, 0x40000314, \
                    prvCpssDrvHwPpPortGroupIsrRead,                                    \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                   \
                    PRV_CPSS_BOBCAT3_BM_SUM_SUM_BM_GENERAL_REG1_SUMMARY_E,             \
                    PRV_CPSS_BOBCAT3_BM_SUM_SUM_BM_GENERAL_REG2_SUMMARY_E,             \
                    FILLED_IN_RUNTIME_CNS, 0, 0x0, 2 , NULL, NULL},                    \
                    /* BM General Cause Reg1 */                                        \
                    bm_gen1_SUB_TREE_MAC(1),                                           \
                    /* BM General Cause Reg2 */                                        \
                    bm_gen2_SUB_TREE_MAC(2)

#define   mppm_SUB_TREE_MAC(bit)                                                \
    /* mppmIntSum */                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x510021B0, 0x510021B4,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_BOBCAT3_MPPM_SUM_RX_CLIENT_0_S_SUM_E,                          \
        PRV_CPSS_BOBCAT3_MPPM_SUM_GENERAL_SUM_E,                                \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 14, NULL, NULL},                         \
                                                                                \
        /* mppmRxClient 0 IntSum */                                             \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002008, 0x5100202C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_0_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E,\
            PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_0_SUM_CREDITS_COUNTER_UNDERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmRxClient 1 IntSum */                                             \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x5100200c, 0x51002030,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_1_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E,\
            PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_1_SUM_CREDITS_COUNTER_UNDERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmRxClient 2 IntSum */                                             \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002010, 0x51002034,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_2_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E,\
            PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_2_SUM_CREDITS_COUNTER_UNDERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 0 IntSum */                                             \
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002080, 0x510020b0,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            PRV_CPSS_BOBCAT3_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 1 IntSum */                                             \
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002084, 0x510020b4,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_TX_CLIENT_1_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            PRV_CPSS_BOBCAT3_MPPM_TX_CLIENT_1_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 2 IntSum */                                             \
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002088, 0x510020b8,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_TX_CLIENT_2_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            PRV_CPSS_BOBCAT3_MPPM_TX_CLIENT_2_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
                                                                                \
        /* mppmBank 0 IntSum */                                                 \
        {15, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002120, 0x51002170, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_BOBCAT3_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 1 IntSum */                                                 \
        {16, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002124, 0x51002174, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_BOBCAT3_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 2 IntSum */                                                 \
        {17, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002128, 0x51002178, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_BANK_2_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_BOBCAT3_MPPM_BANK_2_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 3 IntSum */                                                 \
        {18, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x5100212c, 0x5100217c, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_BANK_3_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_BOBCAT3_MPPM_BANK_3_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 4 IntSum */                                                 \
        {19, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002130, 0x51002180, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_BANK_4_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_BOBCAT3_MPPM_BANK_4_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 5 IntSum */                                                 \
        {20, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002134, 0x51002184, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_BANK_5_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_BOBCAT3_MPPM_BANK_5_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmEccIntSum */                                                     \
        {27, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002000, 0x51002004, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E,              \
            PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E,              \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmGenIntSum */                                                     \
        {29, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x510021c0, 0x510021c4, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_BOBCAT3_MPPM_GEN_SUM_MPPM_RF_ERR_E,                        \
            PRV_CPSS_BOBCAT3_MPPM_GEN_SUM_MPPM_RF_ERR_E,                        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   DataPathIntSum_SUB_TREE_MAC(bit)                                     \
    /* DataPathIntSum */                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000A4, 0x000000A8,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                       \
        PRV_CPSS_BOBCAT3_DATA_PATH_PIPE0_RXDMA0_RXDMA_INT_E,                   \
        PRV_CPSS_BOBCAT3_DATA_PATH_MG3_INT_OUT0_E,                      \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 25/*without bma*/ , NULL, NULL},        \
                                                                               \
        rxDma_SUB_TREE_MAC(1, 0, 0),                                        \
        rxDma_SUB_TREE_MAC(2, 0, 1),                                        \
        rxDma_SUB_TREE_MAC(3, 0, 2),                                        \
                                                                               \
        txDma_SUB_TREE_MAC(4, 0, 0),                                        \
        txDma_SUB_TREE_MAC(5, 0, 1),                                        \
        txDma_SUB_TREE_MAC(6, 0, 2),                                        \
                                                                               \
        txFifo_SUB_TREE_MAC(7, 0, 0),                                       \
        txFifo_SUB_TREE_MAC(8, 0, 1),                                       \
        txFifo_SUB_TREE_MAC(9, 0, 2),                                       \
                                                                               \
        rxDma_SUB_TREE_MAC(10, 1, 3),                                       \
        rxDma_SUB_TREE_MAC(11, 1, 4),                                       \
        rxDma_SUB_TREE_MAC(12, 1, 5),                                       \
                                                                               \
        txDma_SUB_TREE_MAC(13, 1, 3),                                       \
        txDma_SUB_TREE_MAC(14, 1, 4),                                       \
        txDma_SUB_TREE_MAC(15, 1, 5),                                       \
                                                                               \
        txFifo_SUB_TREE_MAC(16, 1, 3),                                      \
        txFifo_SUB_TREE_MAC(17, 1, 4),                                      \
        txFifo_SUB_TREE_MAC(18, 1, 5),                                      \
                                                                               \
        bm_SUB_TREE_MAC(19),                                                   \
        mppm_SUB_TREE_MAC(21),                                                 \
                                                                               \
        /* 0..7, GPIO_0_31_Interrupt Cause */                                  \
        {22, GT_TRUE, 0, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018114, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118), \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                     \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                    \
                PRV_CPSS_BOBCAT3_GPIO_0_0_E,                                   \
                PRV_CPSS_BOBCAT3_GPIO_0_7_E,                                   \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},        \
                                                                               \
        /* 8..15, GPIO_0_31_Interrupt Cause */                                 \
        {23, GT_TRUE, 8, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018114, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118), \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                     \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                    \
                PRV_CPSS_BOBCAT3_GPIO_0_8_E,                                   \
                PRV_CPSS_BOBCAT3_GPIO_0_15_E,                                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},        \
                                                                               \
        /* 16..23, GPIO_0_31_Interrupt Cause */                                \
        {24, GT_TRUE, 16, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018114, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118), \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                     \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                    \
                PRV_CPSS_BOBCAT3_GPIO_0_16_E,                                  \
                PRV_CPSS_BOBCAT3_GPIO_0_23_E,                                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},         \
                                                                               \
         /*Mg2_Int_Out0 */                                                     \
        {26, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00034, 2), \
                prvCpssDrvHwPpMg2ReadReg,                     \
                prvCpssDrvHwPpMg2WriteReg,                    \
                PRV_CPSS_BOBCAT3_MG2_GLOBAL_SUMMARY_TX_SDMA_SUM_E,             \
                PRV_CPSS_BOBCAT3_MG2_GLOBAL_SUMMARY_RX_SDMA_SUM_E,             \
                FILLED_IN_RUNTIME_CNS, 0, 0xFFFFFFFF, 2 , NULL, NULL},                \
                                                                               \
             /* Tx SDMA  */                                                    \
            {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002818, 2),   \
                prvCpssDrvHwPpMg2ReadReg,                                      \
                prvCpssDrvHwPpMg2WriteReg,                                     \
                PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_REJECT_0_E,                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},         \
                                                                               \
            /* Rx SDMA  */                                                     \
            {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002814, 2), \
                prvCpssDrvHwPpMg2ReadReg,                                      \
                prvCpssDrvHwPpMg2WriteReg,                                     \
                PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_BOBCAT3_MG2_RX_SDMA_PACKET_CNT_OF_E,                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},         \
                                                                               \
         /*Mg3_Int_Out0 */                                                     \
        {29, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00034, 3), \
                prvCpssDrvHwPpMg3ReadReg,                                      \
                prvCpssDrvHwPpMg3WriteReg,                                     \
                PRV_CPSS_BOBCAT3_MG3_GLOBAL_SUMMARY_TX_SDMA_SUM_E,             \
                PRV_CPSS_BOBCAT3_MG3_GLOBAL_SUMMARY_RX_SDMA_SUM_E,             \
                FILLED_IN_RUNTIME_CNS, 0, 0xFFFFFFFF, 2 , NULL, NULL},                \
                                                                               \
             /* Tx SDMA  */                                                    \
            {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002818, 3), \
                prvCpssDrvHwPpMg3ReadReg,                                      \
                prvCpssDrvHwPpMg3WriteReg,                                     \
                PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_REJECT_0_E,                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},         \
                                                                               \
            /* Rx SDMA  */                                                     \
            {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002814, 3), \
                prvCpssDrvHwPpMg3ReadReg,                                      \
                prvCpssDrvHwPpMg3WriteReg,                                     \
                PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_BOBCAT3_MG3_RX_SDMA_PACKET_CNT_OF_E,                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* summary of Giga mac that called from XLG mac */
#define BOBCAT3_GIGA_PORT_SUMMARY_FROM_XLG_INT_SUB_TREE_MAC(port, pipe)                         \
                {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                    \
                BOBCAT3_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port, pipe) ,                     \
                BOBCAT3_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port, pipe)  ,                     \
                prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E,                 \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E, /*see PTP issue below */ \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1/*see PTP issue below */,NULL, NULL},  \
                    /* interrupts of the giga mac */                                            \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                               \
                        BOBCAT3_GIGA_PORT_INTERRUPT_CAUSE_MAC(port, pipe) ,                     \
                        BOBCAT3_GIGA_PORT_INTERRUPT_MASK_MAC(port, pipe),                       \
                        prvCpssDrvHwPpPortGroupIsrRead,                                         \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                        PRV_CPSS_BOBCAT3_PIPE##pipe##_GIGA_PORT_##port##_LINK_STATUS_CHANGED_E, \
                        PRV_CPSS_BOBCAT3_PIPE##pipe##_GIGA_PORT_##port##_MIB_COUNTER_WRAPAROUND_E,      \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* 10180000 + 0x1000* k: where k (0-31)  */
#define BOBCAT3_MPCS_PORT_BASE_MAC(port, pipe)                 \
        (0x10180000 + 0x1000*(port) + BOBCAT3_GOP_PIPE_OFFSET_MAC*(pipe))

/* MPCS - cause */
#define BOBCAT3_MPCS_PORT_INTERRUPT_CAUSE_MAC(port, pipe) \
        (BOBCAT3_MPCS_PORT_BASE_MAC(port, pipe) + (0x08))

/* MPCS - mask */
#define BOBCAT3_MPCS_PORT_INTERRUPT_MASK_MAC(port, pipe) \
        (BOBCAT3_MPCS_PORT_BASE_MAC(port, pipe) + (0x0C))

#define BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port, pipe)\
    COMMON_BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port, pipe , 4)

#define COMMON_BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port, pipe,numSons)      \
                /* XLG - External Units Interrupts cause */                                         \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                         \
                BOBCAT3_XLG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port, pipe),                           \
                BOBCAT3_XLG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port, pipe),                            \
                prvCpssDrvBobcat3XlgGopExtUnitsIsrRead,                                             \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_XLG_PORT_##port##_SUM_XLG_PORT_INTERRUPT_SUMMARY_E,   \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_XLG_PORT_##port##_SUM_PTP_UNIT_INTERRUPT_SUMMARY_E + (numSons-4),   \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, numSons ,NULL, NULL},                       \
                    /* interrupts of the XLG mac */                                                 \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    BOBCAT3_XLG_PORT_INTERRUPT_CAUSE_MAC(port, pipe) ,                              \
                    BOBCAT3_XLG_PORT_INTERRUPT_MASK_MAC(port, pipe),                                \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_XLG_PORT_##port##_LINK_STATUS_CHANGED_E,          \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_XLG_PORT_##port##_PFC_SYNC_FIFO_OVERRUN_E,        \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                          \
                    /*bit2*/                                                                        \
                    BOBCAT3_GIGA_PORT_SUMMARY_FROM_XLG_INT_SUB_TREE_MAC(port, pipe),                \
                    /* interrupts of the MPCS mac (also called GB)*/                                \
                    { 5 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    BOBCAT3_MPCS_PORT_INTERRUPT_CAUSE_MAC(port, pipe) ,                             \
                    BOBCAT3_MPCS_PORT_INTERRUPT_MASK_MAC(port, pipe),                               \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_MPCS_PORT_##port##_ACCESS_ERROR_E,                \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_MPCS_PORT_##port##_GB_LOCK_SYNC_CHANGE_E,         \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                          \
                    /*bit 7*/                                                                       \
                    BOBCAT3_PTP_FOR_PORT_INT_ELEMENT_MAC(7, port, pipe)

#define BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port, pipe)                   \
                COMMON_BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port, pipe,5),    \
                    /*bit 8 (dummy bit - CG Ports interrupts support)*/                             \
                    BOBCAT3_CG_PORT_SUMMARY_INT_ELEMENT_MAC(8, port, pipe)

#define BOBCAT3_XLG_PIPE0_PORTS_0_30_SUMMARY_INT_SUB_TREE_MAC         \
                                                      /*bit, port */  \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  (  1,   0,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  2,   1,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  3,   2,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  4,   3,  0), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  (  5,   4,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  6,   5,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  7,   6,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  8,   7,  0), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  (  9,   8,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 10,   9,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 11,  10,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 12,  11,  0), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 13,  12,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 14,  13,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 15,  14,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 16,  15,  0), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 17,  16,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 18,  17,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 19,  18,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 20,  19,  0), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 21,  20,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 22,  21,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 23,  22,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 24,  23,  0), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 25,  24,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 26,  25,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 27,  26,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 28,  27,  0), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 29,  28,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 30,  29,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 31,  30,  0)

#define BOBCAT3_XLG_PIPE0_PORTS_31_35_SUMMARY_INT_SUB_TREE_MAC            \
                                                      /*bit, port */ \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  1,  31,  0), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  (  2,  32,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  3,  33,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  4,  34,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  5,  35,  0)

#define BOBCAT3_XLG_PIPE1_PORTS_0_25_SUMMARY_INT_SUB_TREE_MAC  \
                                                      /*bit, port */ \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  (  6,   0,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  7,   1,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  8,   2,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  9,   3,  1), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 10,   4,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 11,   5,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 12,   6,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 13,   7,  1), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 14,   8,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 15,   9,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 16,  10,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 17,  11,  1), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 18,  12,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 19,  13,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 20,  14,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 21,  15,  1), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 22,  16,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 23,  17,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 24,  18,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 25,  19,  1), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 26,  20,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 27,  21,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 28,  22,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 29,  23,  1), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 30,  24,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 31,  25,  1)

#define BOBCAT3_XLG_PIPE1_PORTS_26_35_SUMMARY_INT_SUB_TREE_MAC        \
                                                      /*bit, port */  \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 1,   26,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 2,   27,  1), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 3,   28,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 4,   29,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 5,   30,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 6,   31,  1), \
            BOBCAT3_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 7,   32,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 8,   33,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 9,   34,  1), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (10,   35,  1)

#define BOBCAT3_XLG_CPU_PORTS_SUMMARY_INT_SUB_TREE_MAC                    \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (11,   36,  0), \
            BOBCAT3_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (12,   36,  1)


#define BOBCAT3_CG_PORT_SUMMARY_INT_ELEMENT_MAC(bitIndexInCaller, port, pipe)                       \
                /* CG - External Units Interrupts cause */                                          \
                {bitIndexInCaller, GT_FALSE, PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                        \
                BOBCAT3_CG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port, pipe),                            \
                BOBCAT3_CG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port, pipe),                             \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_CG_PORT_##port##_SUM_CG_INT_SUMMARY_E,                \
                PRV_CPSS_BOBCAT3_PIPE##pipe##_CG_PORT_##port##_SUM_PTP_INT_SUMMARY_E,               \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2 ,NULL, NULL},                             \
                    /* interrupts of the CG  */                                                     \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    BOBCAT3_CG_PORT_INTERRUPT_CAUSE_MAC(port, pipe) ,                               \
                    BOBCAT3_CG_PORT_INTERRUPT_MASK_MAC(port, pipe),                                 \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_CG_PORT_##port##_RX_OVERSIZE_PACKET_DROP_E,       \
                    PRV_CPSS_BOBCAT3_PIPE##pipe##_CG_PORT_##port##_MLG_RX_FIFO_UNDERRUN3_INT_E,     \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                         \
                    /*bit 2*/                                                                       \
                    BOBCAT3_FAKE_PTP_FOR_CG_PORT_INT_ELEMENT_MAC(2, port, pipe)

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
        PRV_CPSS_BOBCAT3_AP_DOORBELL_MAIN_IPC_E,                            \
        PRV_CPSS_BOBCAT3_AP_DOORBELL_MAIN_RESERVED31_E,                     \
        FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0xFFFFF880, 6,NULL, NULL},       \
        /* AP_DOORBELL_PORT0_31_LINK_STATUS_CHANGE */                       \
        {4,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000012, 0x00000013,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_0_LINK_STATUS_CHANGE_E,       \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_31_LINK_STATUS_CHANGE_E,      \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT32_63_LINK_STATUS_CHANGE */                      \
        {5,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000014, 0x00000015,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_32_LINK_STATUS_CHANGE_E,      \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_63_LINK_STATUS_CHANGE_E,      \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT64_95_LINK_STATUS_CHANGE */                      \
        {6,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000016, 0x00000017,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_64_LINK_STATUS_CHANGE_E,      \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_95_LINK_STATUS_CHANGE_E,      \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT0_31_802_3_AP */                                 \
        {8,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000018, 0x00000019,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_0_802_3_AP_E,                 \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_31_802_3_AP_E,                \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT32_63_802_3_AP */                                \
        {9,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000001a, 0x0000001b,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_32_802_3_AP_E,                \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_63_802_3_AP_E,                \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT64_95_802_3_AP */                                \
        {10,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000001c, 0x0000001d, \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_64_802_3_AP_E,                \
            PRV_CPSS_BOBCAT3_AP_DOORBELL_PORT_95_802_3_AP_E,                \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL}

/*indication that the bobcat3IntrScanArr was initalized and ready for use */
/* NOTE : needed in BC3 due to the 'PTP fake' manipultion (look for 'fake' key word)*/
static GT_U32 bobcat3IntrScanArr_initDone = 0;

static /*const*/ PRV_CPSS_DRV_INTERRUPT_SCAN_STC bobcat3IntrScanArr[] =
{
    /* Global Interrupt Cause */
    {0, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000030, 0x00000034,
        prvCpssDrvHwPpPortGroupIsrRead,
        prvCpssDrvHwPpPortGroupIsrWrite,
        PRV_CPSS_BOBCAT3_GLOBAL_SUMMARY_FUNC_UNITS_SUM_E,
        PRV_CPSS_BOBCAT3_GLOBAL_SUMMARY_MG1_INTERNAL_SUM_E,
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 15, NULL, NULL},

        /* PEX */
        {1,GT_FALSE,0, NULL, PRV_CPSS_DRV_SCAN_PEX_CAUSE_REG_PLACE_HOLDER_CNS, PRV_CPSS_DRV_SCAN_PEX_MASK_REG_PLACE_HOLDER_CNS,
            prvCpssDrvHwPpPortGroupReadInternalPciReg,
            prvCpssDrvHwPpPortGroupWriteInternalPciReg,
            PRV_CPSS_BOBCAT3_PEX_RCV_A_E,
            PRV_CPSS_BOBCAT3_PEX_RCV_D_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},


        /* PEX ERROR */
        {2,GT_FALSE,0, NULL, PRV_CPSS_DRV_SCAN_PEX_CAUSE_REG_PLACE_HOLDER_CNS, PRV_CPSS_DRV_SCAN_PEX_MASK_REG_PLACE_HOLDER_CNS,
            prvCpssDrvHwPpPortGroupReadInternalPciReg,
            prvCpssDrvHwPpPortGroupWriteInternalPciReg,
            PRV_CPSS_BOBCAT3_PEX_DL_DWN_TX_ACC_ERR_E,
            PRV_CPSS_BOBCAT3_PEX_PEX_LINK_FAIL_E,
            FILLED_IN_RUNTIME_CNS, 0xFFDFDB, 0xFFFFFFFF, 0,NULL, NULL},

        FuncUnitsIntsSum_SUB_TREE_MAC(3),

        DataPathIntSum_SUB_TREE_MAC(4),

        /* ports0SumIntSum */
        {5,GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000080, 0x00000084,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_PIPE0_PORTS0_PORT_INT0_SUM_E,
            PRV_CPSS_BOBCAT3_PIPE0_PORTS0_PORT_INT30_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 31, NULL, NULL},

            /* XLG ports 0..30 bits 1..31 */
            BOBCAT3_XLG_PIPE0_PORTS_0_30_SUMMARY_INT_SUB_TREE_MAC,

        /* dfxIntSum */
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000ac, 0x000000b0,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_DFX_SUM_DFX_PIPE0_DFX_CLIENT_BC3_DQ_MACRO_CORE_CLK_INT_E,
            PRV_CPSS_BOBCAT3_DFX_SUM_DFX_DFX_CLIENT_BC3_MPPM_BANKS_MACRO_3_MPPM_CLK_INT_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}, /* without children yet */

        /* Miscellaneous */
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000038, 0x0000003C,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_MISC_TWSI_TIME_OUT_E,
            PRV_CPSS_BOBCAT3_MISC_SERDES_OUT_OF_RANGE_VIOLATION_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1,NULL, NULL},
            /* HostCpuDoorbellIntSum */
            {24, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000518, 0x0000051c,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_BOBCAT3_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,
                PRV_CPSS_BOBCAT3_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 1,NULL, NULL},

                AP_Doorbell_SUB_TREE_MAC(4),

        /* Tx SDMA  */
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, 0x00002818,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_0_E,
            PRV_CPSS_BOBCAT3_TX_SDMA_TX_REJECT_0_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /* Rx SDMA  */
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, 0x00002814,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_0_E,
            PRV_CPSS_BOBCAT3_RX_SDMA_PACKET_CNT_OF_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /* dfx1IntSum */
        {10, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000B8, 0x000000BC,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_DFX1_PIPE1_DFX_CLIENT_BC3_DQ_MACRO_CORE_CLK_INT_E,
            PRV_CPSS_BOBCAT3_DFX1_DFX_CLIENT_BC3_TCAM_LOGIC_MACRO_CORE_X2_CLK_1_INT_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},

        FuncUnits1IntsSum_SUB_TREE_MAC(11),

        /* xsmi0IntSum */
        {12, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030010, 0x00030014,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_MG_XSMI0_XG_SMI_WRITE_E,
            PRV_CPSS_BOBCAT3_MG_XSMI0_XG_SMI_WRITE_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},

        /* ports1SumIntSum */
        {16,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000150, 0x00000154,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_PIPE0_PORTS1_PORT_INT31_SUM_E,
            PRV_CPSS_BOBCAT3_PIPE1_PORTS1_PORT_INT25_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 31, NULL, NULL}
        ,
            /* Pipe 0 XLG ports 31..35 bits 1..5 */
            BOBCAT3_XLG_PIPE0_PORTS_31_35_SUMMARY_INT_SUB_TREE_MAC,
            /* Pipe 1 XLG ports 0..25 bits 6..31 */
            BOBCAT3_XLG_PIPE1_PORTS_0_25_SUMMARY_INT_SUB_TREE_MAC,

        /* ports2SumIntSum */
        {17,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000158, 0x0000015C,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_PIPE1_PORTS2_PORT_INT26_SUM_E,
            PRV_CPSS_BOBCAT3_PORTS2_IHB2MG_INT7_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 14, NULL, NULL},

            /* Pipe 1 XLG ports 26..35 bits 1..10 */
            BOBCAT3_XLG_PIPE1_PORTS_26_35_SUMMARY_INT_SUB_TREE_MAC,
            /* Pipe 0 CPU port, pipe 1 CPU port */
            BOBCAT3_XLG_CPU_PORTS_SUMMARY_INT_SUB_TREE_MAC,

            /* TAI0 Interrupt Cause */
            {13, GT_FALSE, PRV_CPSS_DXCH_UNIT_TAI_E, NULL, 0x57000000, 0x57000004,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_BOBCAT3_TAI_GENERATION_INT_E,
                PRV_CPSS_BOBCAT3_TAI_DECREMENT_LOST_INT_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

            /* TAI1 Interrupt Cause */
            {14, GT_FALSE, PRV_CPSS_DXCH_UNIT_TAI_E, NULL, 0x58000000, 0x58000004,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_BOBCAT3_TAI1_GENERATION_INT_E,
                PRV_CPSS_BOBCAT3_TAI1_DECREMENT_LOST_INT_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /* MG1 internal Interrupt Cause */
        {18,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000009C, 0x000000A0,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_BOBCAT3_MG1_INTERNAL_INTERRUPT_HOST_TO_CM3_DOORBELL_INTERRUPT1_SUM_E,
            PRV_CPSS_BOBCAT3_MG1_INTERNAL_INTERRUPT_CM3_SRAM_OOR_INT1_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},

};

/* number of elements in the array of bobcat3IntrScanArr[] */
#define BOBCAT3_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    (sizeof(bobcat3IntrScanArr)/sizeof(bobcat3IntrScanArr[0]))

#define BOBCAT3_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_BOBCAT3_LAST_INT_E / 32)

/* This array maps an interrupt index to its relevant interrupt mask reg    */
/* default value, cell i indicates the address for interrupts               */
/* (32 * i  --> 32 * (i + 1) - 1).                                          */
/* summary bits are turned on and the specific cause mask is enabled when   */
/* bounded by appl.                                                         */
static GT_U32 bobcat3MaskRegDefaultSummaryArr[BOBCAT3_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};

/* This array maps an interrupt index to its    */
/* relevant interrupt mask registers address,   */
/* cell i indicates the address for interrupts  */
/* (32 * i  --> 32 * (i + 1) - 1).              */
static GT_U32 bobcat3MaskRegMapArr[BOBCAT3_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};


/* get the first value of event in register (clear its 5 LSBits)*/
#define GET_FIRST_EVENT_IN_REG_MAC(_event)  ((_event) & (~0x1f))

/* the full 32 events for event that belongs to a register ,
   will have index equal to the 32 events of the register */
#define SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(_event) \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 0 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 1 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 2 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 3 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 4 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 5 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 6 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 7 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 8 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 9 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 10),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 11),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 12),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 13),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 14),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 15),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 16),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 17),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 18),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 19),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 20),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 21),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 22),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 23),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 24),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 25),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 26),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 27),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 28),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 29),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 30),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 31)


/* state for 37 ports that event hold extra param as : ((portNum) << port_offset) |  ext_param */
#define SET_EVENT_PER_37_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset) \
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
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 32, ((32<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 33, ((33<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 34, ((34<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 35, ((35<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 36, ((36<< port_offset) | (ext_param)))


/* state for 37 ports that event hold extra param as : ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 0)) << port_offset) |  ext_param */
#define SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(_pipe,_prefix,_postFix,ext_param,port_offset) \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 0 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 0) << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 1 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 1) << port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 2 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 2) << port_offset) | (ext_param))) ,   \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 3 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 3) << port_offset) | (ext_param))) ,   \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 4 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 4)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 5 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 5)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 6 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 6)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 7 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 7)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 8 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 8)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 9 , ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 9)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 10, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 10)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 11, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 11)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 12, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 12)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 13, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 13)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 14, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 14)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 15, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 15)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 16, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 16)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 17, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 17)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 18, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 18)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 19, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 19)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 20, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 20)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 21, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 21)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 22, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 22)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 23, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 23)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 24, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 24)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 25, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 25)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 26, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 26)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 27, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 27)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 28, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 28)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 29, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 29)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 30, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 30)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 31, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 31)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 32, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 32)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 33, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 33)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 34, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 34)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 35, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 35)<< port_offset) | (ext_param))) ,    \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 36, ((MARK_PER_PIPE_PORT_INT_MAC(_pipe, 36)<< port_offset) | (ext_param)))


#define SET_INDEX_EQUAL_EVENT_MMPCS_SIGNAL_DETECT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix,_pipe) \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      0,0,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      1,0,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      2,0,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      3,0,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      4,4,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      5,4,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      6,4,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      7,4,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      8,8,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,      9,8,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     10,8,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     11,8,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     12,12,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     13,12,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     14,12,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     15,12,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     16,16,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     17,16,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     18,16,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     19,16,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     20,20,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     21,20,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     22,20,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     23,20,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     24,24,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     25,25,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     26,26,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     27,27,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     28,28,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     29,29,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     30,30,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     31,31,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     32,32,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     33,33,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     34,34,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     35,35,     _pipe),    \
    SET_EVENT_PER_PIPE_PORT_WA_MAC(_prefix,_postFix,     36,36,     _pipe)

/* state for ports 0..36 that event hold extra index equal to event */
#define SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix,_pipe)   \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      0,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      1,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      2,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      3,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      4,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      5,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      6,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      7,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      8,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      9,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     10,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     11,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     12,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     13,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     14,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     15,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     16,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     17,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     18,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     19,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     20,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     21,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     22,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     23,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     24,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     25,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     26,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     27,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     28,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     29,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     30,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     31,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     32,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     33,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     34,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     35,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     36,      _pipe)

/* state for CG ports that event hold extra index equal to event */
#define SET_INDEX_EQUAL_EVENT_PER_CG_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix,_pipe)   \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      0,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      4,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,      8,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     12,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     16,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     20,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     24,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     28,      _pipe),    \
    SET_EVENT_PER_PIPE_PORT_MAC(_prefix,_postFix,     32,      _pipe)



/* state for ports 0..71 that event hold extra port number */
#define SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     0),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     1),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     2),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     3),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     4),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     5),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     6),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     7),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     8),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     9),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     10),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     11),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     12),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     13),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     14),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     15),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     16),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     17),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     18),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     19),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     20),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     21),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     22),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     23),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     24),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     25),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     26),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     27),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     28),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     29),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     30),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     31),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     32),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     33),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     34),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     35),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     36),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     37),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     38),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     39),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     40),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     41),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     42),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     43),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     44),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     45),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     46),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     47),   \
    SET_EVENT_PER_PORTS_48_71___ON_ALL_PORTS_MAC(_prefix,_postFix)

/* state for even ports 48,50,52..70 that event hold extra port number */
#define SET_EVENT_PER_PORTS_48_70_EVEN__ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     48),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     50),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     52),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     54),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     56),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     58),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     60),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     62),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     64),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     66),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     68),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     70)

/* state for odd ports 49,51,53..71 that event hold extra port number */
#define SET_EVENT_PER_PORTS_49_71_ODD__ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     49),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     51),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     53),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     55),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     57),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     59),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     61),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     63),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     65),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     67),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     69),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     71)

#define SET_EVENT_PER_PORTS_48_71___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
        SET_EVENT_PER_PORTS_48_70_EVEN__ON_ALL_PORTS_MAC(_prefix,_postFix),  \
        SET_EVENT_PER_PORTS_49_71_ODD__ON_ALL_PORTS_MAC(_prefix,_postFix)

/* state for ports 72..255 that event hold extra port number */
#define SET_EVENT_PER_PORTS_72_TO_255___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     72      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     73      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     74      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     75      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     76      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     77      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     78      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     79      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     80      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     81      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     82      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     83      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     84      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     85      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     86      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     87      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     88      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     89      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     90      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     91      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     92      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     93      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     94      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     95      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     96      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     97      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     98      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     99      ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     100     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     101     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     102     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     103     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     104     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     105     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     106     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     107     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     108     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     109     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     110     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     111     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     112     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     113     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     114     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     115     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     116     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     117     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     118     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     119     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     120     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     121     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     122     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     123     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     124     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     125     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     126     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     127     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     128     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     129     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     130     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     131     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     132     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     133     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     134     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     135     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     136     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     137     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     138     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     139     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     140     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     141     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     142     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     143     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     144     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     145     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     146     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     147     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     148     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     149     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     150     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     151     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     152     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     153     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     154     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     155     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     156     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     157     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     158     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     159     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     160     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     161     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     162     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     163     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     164     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     165     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     166     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     167     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     168     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     169     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     170     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     171     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     172     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     173     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     174     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     175     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     176     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     177     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     178     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     179     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     180     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     181     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     182     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     183     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     184     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     185     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     186     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     187     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     188     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     189     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     190     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     191     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     192     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     193     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     194     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     195     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     196     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     197     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     198     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     199     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     200     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     201     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     202     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     203     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     204     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     205     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     206     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     207     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     208     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     209     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     210     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     211     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     212     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     213     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     214     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     215     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     216     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     217     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     218     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     219     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     220     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     221     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     222     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     223     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     224     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     225     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     226     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     227     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     228     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     229     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     230     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     231     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     232     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     233     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     234     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     235     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     236     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     237     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     238     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     239     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     240     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     241     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     242     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     243     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     244     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     245     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     246     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     247     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     248     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     249     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     250     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     251     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     252     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     253     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     254     ),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     255     )

/* state for ALL 0..512 ports that event hold extra port number */
#define SET_EVENT_PER_512_PORTS_MAC(_prefix,_postFix)       \
    /* the first 71 ports */                                \
    SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix),           \
    /* the ports 72..512 */                                 \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       72    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       73    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       74    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       75    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       76    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       77    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       78    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       79    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       80    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       81    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       82    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       83    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       84    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       85    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       86    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       87    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       88    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       89    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       90    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       91    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       92    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       93    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       94    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       95    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       96    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       97    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       98    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       99    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      100    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      101    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      102    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      103    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      104    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      105    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      106    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      107    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      108    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      109    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      110    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      111    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      112    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      113    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      114    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      115    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      116    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      117    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      118    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      119    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      120    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      121    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      122    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      123    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      124    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      125    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      126    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      127    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      128    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      129    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      130    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      131    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      132    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      133    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      134    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      135    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      136    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      137    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      138    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      139    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      140    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      141    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      142    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      143    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      144    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      145    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      146    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      147    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      148    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      149    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      150    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      151    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      152    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      153    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      154    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      155    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      156    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      157    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      158    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      159    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      160    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      161    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      162    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      163    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      164    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      165    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      166    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      167    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      168    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      169    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      170    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      171    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      172    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      173    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      174    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      175    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      176    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      177    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      178    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      179    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      180    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      181    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      182    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      183    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      184    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      185    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      186    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      187    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      188    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      189    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      190    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      191    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      192    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      193    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      194    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      195    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      196    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      197    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      198    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      199    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      200    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      201    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      202    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      203    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      204    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      205    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      206    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      207    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      208    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      209    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      210    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      211    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      212    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      213    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      214    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      215    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      216    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      217    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      218    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      219    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      220    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      221    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      222    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      223    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      224    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      225    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      226    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      227    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      228    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      229    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      230    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      231    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      232    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      233    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      234    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      235    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      236    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      237    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      238    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      239    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      240    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      241    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      242    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      243    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      244    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      245    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      246    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      247    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      248    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      249    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      250    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      251    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      252    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      253    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      254    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      255    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      256    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      257    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      258    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      259    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      260    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      261    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      262    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      263    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      264    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      265    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      266    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      267    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      268    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      269    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      270    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      271    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      272    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      273    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      274    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      275    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      276    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      277    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      278    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      279    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      280    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      281    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      282    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      283    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      284    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      285    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      286    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      287    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      288    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      289    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      290    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      291    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      292    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      293    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      294    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      295    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      296    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      297    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      298    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      299    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      300    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      301    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      302    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      303    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      304    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      305    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      306    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      307    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      308    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      309    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      310    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      311    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      312    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      313    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      314    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      315    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      316    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      317    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      318    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      319    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      320    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      321    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      322    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      323    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      324    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      325    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      326    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      327    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      328    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      329    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      330    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      331    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      332    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      333    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      334    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      335    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      336    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      337    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      338    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      339    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      340    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      341    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      342    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      343    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      344    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      345    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      346    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      347    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      348    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      349    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      350    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      351    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      352    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      353    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      354    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      355    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      356    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      357    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      358    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      359    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      360    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      361    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      362    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      363    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      364    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      365    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      366    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      367    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      368    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      369    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      370    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      371    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      372    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      373    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      374    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      375    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      376    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      377    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      378    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      379    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      380    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      381    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      382    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      383    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      384    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      385    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      386    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      387    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      388    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      389    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      390    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      391    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      392    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      393    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      394    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      395    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      396    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      397    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      398    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      399    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      400    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      401    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      402    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      403    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      404    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      405    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      406    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      407    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      408    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      409    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      410    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      411    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      412    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      413    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      414    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      415    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      416    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      417    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      418    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      419    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      420    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      421    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      422    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      423    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      424    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      425    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      426    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      427    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      428    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      429    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      430    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      431    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      432    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      433    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      434    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      435    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      436    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      437    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      438    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      439    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      440    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      441    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      442    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      443    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      444    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      445    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      446    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      447    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      448    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      449    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      450    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      451    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      452    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      453    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      454    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      455    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      456    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      457    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      458    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      459    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      460    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      461    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      462    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      463    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      464    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      465    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      466    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      467    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      468    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      469    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      470    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      471    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      472    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      473    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      474    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      475    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      476    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      477    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      478    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      479    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      480    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      481    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      482    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      483    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      484    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      485    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      486    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      487    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      488    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      489    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      490    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      491    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      492    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      493    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      494    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      495    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      496    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      497    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      498    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      499    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      500    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      501    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      502    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      503    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      504    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      505    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      506    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      507    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      508    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      509    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      510    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      511    )

/* state for 72 ports that event hold extra param as : ((portNum) << port_offset) |  ext_param */
#define SET_EVENT_PER_72_PORTS_WITH_PORT_OFFSET_AND_PARAM___ON_ALL_PORTS_MAC(_prefix,_postFix,ext_param,port_offset) \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 0 , ((0 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 1 , ((1 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 2 , ((2 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 3 , ((3 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 4 , ((4 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 5 , ((5 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 6 , ((6 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 7 , ((7 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 8 , ((8 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 9 , ((9 << port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 10, ((10<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 11, ((11<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 12, ((12<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 13, ((13<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 14, ((14<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 15, ((15<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 16, ((16<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 17, ((17<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 18, ((18<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 19, ((19<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 20, ((20<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 21, ((21<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 22, ((22<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 23, ((23<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 24, ((24<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 25, ((25<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 26, ((26<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 27, ((27<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 28, ((28<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 29, ((29<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 30, ((30<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 31, ((31<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 32, ((32<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 33, ((33<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 34, ((34<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 35, ((35<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 36, ((36<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 37, ((37<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 38, ((38<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 39, ((39<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 40, ((40<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 41, ((41<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 42, ((42<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 43, ((43<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 44, ((44<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 45, ((45<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 46, ((46<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 47, ((47<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 48, ((48<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 49, ((49<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 50, ((50<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 51, ((51<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 52, ((52<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 53, ((53<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 54, ((54<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 55, ((55<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 56, ((56<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 57, ((57<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 58, ((58<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 59, ((59<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 60, ((60<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 61, ((61<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 62, ((62<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 63, ((63<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 64, ((64<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 65, ((65<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 66, ((66<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 67, ((67<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 68, ((68<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 69, ((69<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 70, ((70<< port_offset) | (ext_param))) ,\
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, 71, ((71<< port_offset) | (ext_param)))


/* state for a port that event hold extra port number */
/*__prefix - is between 'device' and "_PORT" */
/*_postFix - must include the "_E" */
/*convert local TXQ DQ port to global TXQ port using txq index*/
#define SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,_portNum, _dq)   \
    _prefix##_##PORT##_##_portNum##_##_postFix,       MARK_PER_PORT_INT_MAC((_portNum) + (_dq)*96)

/* state for ports 0..123 that event hold extra port number */
#define SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(_prefix,_postFix, _dq)        \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     0, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     1, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     2, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     3, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     4, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     5, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     6, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     7, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     8, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     9, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    10, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    11, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    12, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    13, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    14, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    15, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    16, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    17, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    18, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    19, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    20, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    21, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    22, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    23, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    24, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    25, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    26, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    27, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    28, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    29, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    30, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    31, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    32, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    33, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    34, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    35, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    36, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    37, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    38, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    39, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    40, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    41, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    42, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    43, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    44, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    45, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    46, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    47, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    48, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    49, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    50, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    51, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    52, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    53, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    54, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    55, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    56, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    57, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    58, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    59, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    60, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    61, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    62, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    63, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    64, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    65, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    66, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    67, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    68, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    69, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    70, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    71, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    72, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    73, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    74, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    75, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    76, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    77, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    78, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    79, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    80, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    81, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    82, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    83, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    84, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    85, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    86, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    87, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    88, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    89, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    90, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    91, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    92, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    93, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    94, _dq ), \
     SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    95, _dq )

/* state for ports 0..91 that event hold extra port number */
#define SET_EVENT_PER_92_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     72),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     73),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     74),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     75),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     76),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     77),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     78),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     79),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     80),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     81),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     82),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     83),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     84),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     85),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     86),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     87),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     88),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     89),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     90),   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     91)

/* Interrupt cause to unified event map for Bobcat3
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static GT_U32 bobcat3UniEvMapTableWithExtData[] = {
 /* Per Queue events */
 CPSS_PP_TX_BUFFER_QUEUE_E,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_0_E,  0,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_1_E,  1,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_2_E,  2,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_3_E,  3,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_4_E,  4,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_5_E,  5,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_6_E,  6,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_BUFFER_QUEUE_7_E,  7,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_0_E,  8,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_1_E,  9,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_2_E,  10,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_3_E,  11,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_4_E,  12,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_5_E,  13,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_6_E,  14,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_BUFFER_QUEUE_7_E,  15,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_0_E,  16,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_1_E,  17,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_2_E,  18,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_3_E,  19,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_4_E,  20,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_5_E,  21,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_6_E,  22,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_BUFFER_QUEUE_7_E,  23,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_0_E,  24,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_1_E,  25,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_2_E,  26,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_3_E,  27,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_4_E,  28,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_5_E,  29,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_6_E,  30,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_BUFFER_QUEUE_7_E,  31,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_ERR_QUEUE_E,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_ERROR_QUEUE_0_E,  0,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_ERROR_QUEUE_1_E,  1,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_ERROR_QUEUE_2_E,  2,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_ERROR_QUEUE_3_E,  3,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_ERROR_QUEUE_4_E,  4,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_ERROR_QUEUE_5_E,  5,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_ERROR_QUEUE_6_E,  6,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_ERROR_QUEUE_7_E,  7,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_ERROR_QUEUE_0_E,  8,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_ERROR_QUEUE_1_E,  9,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_ERROR_QUEUE_2_E,  10,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_ERROR_QUEUE_3_E,  11,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_ERROR_QUEUE_4_E,  12,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_ERROR_QUEUE_5_E,  13,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_ERROR_QUEUE_6_E,  14,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_ERROR_QUEUE_7_E,  15,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_ERROR_QUEUE_0_E,  16,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_ERROR_QUEUE_1_E,  17,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_ERROR_QUEUE_2_E,  18,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_ERROR_QUEUE_3_E,  19,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_ERROR_QUEUE_4_E,  20,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_ERROR_QUEUE_5_E,  21,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_ERROR_QUEUE_6_E,  22,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_ERROR_QUEUE_7_E,  23,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_ERROR_QUEUE_0_E,  24,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_ERROR_QUEUE_1_E,  25,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_ERROR_QUEUE_2_E,  26,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_ERROR_QUEUE_3_E,  27,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_ERROR_QUEUE_4_E,  28,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_ERROR_QUEUE_5_E,  29,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_ERROR_QUEUE_6_E,  30,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_ERROR_QUEUE_7_E,  31,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_END_E,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_END_QUEUE_0_E,  0,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_END_QUEUE_1_E,  1,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_END_QUEUE_2_E,  2,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_END_QUEUE_3_E,  3,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_END_QUEUE_4_E,  4,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_END_QUEUE_5_E,  5,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_END_QUEUE_6_E,  6,
    PRV_CPSS_BOBCAT3_TX_SDMA_TX_END_QUEUE_7_E,  7,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_END_QUEUE_0_E,  8,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_END_QUEUE_1_E,  9,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_END_QUEUE_2_E,  10,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_END_QUEUE_3_E,  11,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_END_QUEUE_4_E,  12,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_END_QUEUE_5_E,  13,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_END_QUEUE_6_E,  14,
    PRV_CPSS_BOBCAT3_MG1_TX_SDMA_TX_END_QUEUE_7_E,  15,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_END_QUEUE_0_E,  16,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_END_QUEUE_1_E,  17,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_END_QUEUE_2_E,  18,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_END_QUEUE_3_E,  19,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_END_QUEUE_4_E,  20,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_END_QUEUE_5_E,  21,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_END_QUEUE_6_E,  22,
    PRV_CPSS_BOBCAT3_MG2_TX_SDMA_TX_END_QUEUE_7_E,  23,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_END_QUEUE_0_E,  24,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_END_QUEUE_1_E,  25,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_END_QUEUE_2_E,  26,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_END_QUEUE_3_E,  27,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_END_QUEUE_4_E,  28,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_END_QUEUE_5_E,  29,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_END_QUEUE_6_E,  30,
    PRV_CPSS_BOBCAT3_MG3_TX_SDMA_TX_END_QUEUE_7_E,  31,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_0_E,     0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_1_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_2_E,     2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_3_E,     3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_4_E,     4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_5_E,     5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_6_E,     6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_BUFFER_QUEUE_7_E,     7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE0_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_ERROR_QUEUE_0_E,      0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE1_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_ERROR_QUEUE_1_E,      1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE2_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_ERROR_QUEUE_2_E,      2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE3_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_ERROR_QUEUE_3_E,      3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE4_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_ERROR_QUEUE_4_E,      4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE5_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_ERROR_QUEUE_5_E,      5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE6_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_ERROR_QUEUE_6_E,      6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE7_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RX_ERROR_QUEUE_7_E,      7,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_MAC_SFLOW_E,
    SET_EVENT_PER_512_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_INGRESS_STC, INGRESS_SAMPLE_LOADED_E),
    SET_EVENT_PER_512_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_INGRESS_STC, INGRESS_SAMPLE_LOADED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_EGRESS_SFLOW_E,
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_EGR_STC,E,0),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_EGR_STC,E,1),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_EGR_STC,E,2),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_EGR_STC,E,3),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_EGR_STC,E,4),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_EGR_STC,E,5),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_TXQ2_FLUSH_PORT_E,
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_FLUSH_DONE_SUM,E,0),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_FLUSH_DONE_SUM,E,1),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_FLUSH_DONE_SUM,E,2),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_FLUSH_DONE_SUM,E,3),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_FLUSH_DONE_SUM,E,4),
    SET_EVENT_PER_96_PORTS_FOR_DQ_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_FLUSH_DONE_SUM,E,5),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_AN_COMPLETED_E,
    /* GIGA ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,AN_COMPLETED_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,AN_COMPLETED_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_EEE_E,
    /* GIGA ports 0..36 */
    SET_EVENT_PER_37_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,PCS_RX_PATH_RECEIVED_LPI_E,0 ,8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,PCS_TX_PATH_RECEIVED_LPI_E,1 ,8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,MAC_RX_PATH_RECEIVED_LPI_E,2 ,8/*port<<8*/),

    SET_EVENT_PER_37_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,PCS_RX_PATH_RECEIVED_LPI_E,3 ,8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,PCS_TX_PATH_RECEIVED_LPI_E,4 ,8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,MAC_RX_PATH_RECEIVED_LPI_E,5 ,8/*port<<8*/),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
    /* GIGA ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,SYNC_STATUS_CHANGED_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,SYNC_STATUS_CHANGED_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
    /* XLG ports 0..36 */
    /*SET_INDEX_EQUAL_EVENT_MMPCS_SIGNAL_DETECT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_MPCS,SIGNAL_DETECT_CHANGED_E, 0),
    SET_INDEX_EQUAL_EVENT_MMPCS_SIGNAL_DETECT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_MPCS,SIGNAL_DETECT_CHANGED_E, 1),*/
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_MPCS,SIGNAL_DETECT_CHANGED_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_MPCS,SIGNAL_DETECT_CHANGED_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
   SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_MPCS,ALIGN_LOCK_LOST_E, 0),
   SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_MPCS,ALIGN_LOCK_LOST_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
   SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_MPCS,GB_LOCK_SYNC_CHANGE_E, 0),
   SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_MPCS,GB_LOCK_SYNC_CHANGE_E, 1),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
    /* GIGA ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,LINK_STATUS_CHANGED_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,LINK_STATUS_CHANGED_E, 1),
    /* XLG ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_XLG,LINK_STATUS_CHANGED_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_XLG,LINK_STATUS_CHANGED_E, 1),
    /* CG ports */
    SET_INDEX_EQUAL_EVENT_PER_CG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_CG,LINK_STATUS_CHANGED_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_CG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_CG,LINK_STATUS_CHANGED_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,
    /* GIGA ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,RX_FIFO_OVERRUN_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,RX_FIFO_OVERRUN_E, 1),
    /* XLG ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_XLG,RX_FIFO_OVERRUN_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_XLG,RX_FIFO_OVERRUN_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_UNDERRUN_E,
    /* GIGA ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,TX_UNDERRUN_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,TX_UNDERRUN_E, 1),
    /* XLG ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_XLG,TX_UNDERRUN_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_XLG,TX_UNDERRUN_E, 1),
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_PORT_PRBS_ERROR_E,
    /* GIGA ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,PRBS_ERROR_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,PRBS_ERROR_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,
    /* GIGA ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,TX_FIFO_OVERRUN_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,TX_FIFO_OVERRUN_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
    /* GIGA ports 0..36 */
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE0_GIGA,TX_FIFO_UNDERRUN_E, 0),
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_PIPE1_GIGA,TX_FIFO_UNDERRUN_E, 1),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_PORT_DESC_FULL_E,
    SET_EVENT_PER_92_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_TXQ_PORT_DESC_SUM_DESC_FULL,E),
 MARK_END_OF_UNI_EV_CNS,

CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,
    PRV_CPSS_BOBCAT3_PIPE0_ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,
    PRV_CPSS_BOBCAT3_PIPE0_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,
    PRV_CPSS_BOBCAT3_PIPE0_ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,
    PRV_CPSS_BOBCAT3_PIPE0_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,
    PRV_CPSS_BOBCAT3_PIPE1_ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,
    PRV_CPSS_BOBCAT3_PIPE1_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,
    PRV_CPSS_BOBCAT3_PIPE1_ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,
    PRV_CPSS_BOBCAT3_PIPE1_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,
    SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(0,PRV_CPSS_BOBCAT3_PIPE0_PTP,TIMESTAMP_QUEUE0_FULL_E,GTS_INT_EXT_PARAM_MAC(1,0,1),8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(0,PRV_CPSS_BOBCAT3_PIPE0_PTP,TIMESTAMP_QUEUE1_FULL_E,GTS_INT_EXT_PARAM_MAC(1,1,1),8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(1,PRV_CPSS_BOBCAT3_PIPE1_PTP,TIMESTAMP_QUEUE0_FULL_E,GTS_INT_EXT_PARAM_MAC(1,0,1),8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(1,PRV_CPSS_BOBCAT3_PIPE1_PTP,TIMESTAMP_QUEUE1_FULL_E,GTS_INT_EXT_PARAM_MAC(1,1,1),8/*port<<8*/),
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,
    PRV_CPSS_BOBCAT3_PIPE0_ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q0_E,  0,
    PRV_CPSS_BOBCAT3_PIPE0_ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q0_E,   1,
    PRV_CPSS_BOBCAT3_PIPE0_ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q1_E,  2,
    PRV_CPSS_BOBCAT3_PIPE0_ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q1_E,   3,
    PRV_CPSS_BOBCAT3_PIPE1_ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q0_E,  0,
    PRV_CPSS_BOBCAT3_PIPE1_ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q0_E,   1,
    PRV_CPSS_BOBCAT3_PIPE1_ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q1_E,  2,
    PRV_CPSS_BOBCAT3_PIPE1_ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q1_E,   3,
    SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(0,PRV_CPSS_BOBCAT3_PIPE0_PTP,NEW_TIMESTAMP_QUEUE0_E,GTS_INT_EXT_PARAM_MAC(1,0,1),8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(0,PRV_CPSS_BOBCAT3_PIPE0_PTP,NEW_TIMESTAMP_QUEUE1_E,GTS_INT_EXT_PARAM_MAC(1,1,1),8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(1,PRV_CPSS_BOBCAT3_PIPE1_PTP,NEW_TIMESTAMP_QUEUE0_E,GTS_INT_EXT_PARAM_MAC(1,0,1),8/*port<<8*/),
    SET_EVENT_PER_37_PORTS_WITH_PIPE_PORT_OFFSET_AND_PARAM_MAC(1,PRV_CPSS_BOBCAT3_PIPE1_PTP,NEW_TIMESTAMP_QUEUE1_E,GTS_INT_EXT_PARAM_MAC(1,1,1),8/*port<<8*/),
  MARK_END_OF_UNI_EV_CNS,

  /* Per Index events */
 CPSS_PP_GPP_E,
    PRV_CPSS_BOBCAT3_MISC_GPP_0_E,  0,
    PRV_CPSS_BOBCAT3_MISC_GPP_1_E,  1,
    PRV_CPSS_BOBCAT3_MISC_GPP_2_E,  2,
    PRV_CPSS_BOBCAT3_MISC_GPP_3_E,  3,
    PRV_CPSS_BOBCAT3_MISC_GPP_4_E,  4,
    PRV_CPSS_BOBCAT3_MISC_GPP_5_E,  5,
    PRV_CPSS_BOBCAT3_MISC_GPP_6_E,  6,
    PRV_CPSS_BOBCAT3_MISC_GPP_7_E,  7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,    0,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,    1,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,    2,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,    3,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,    4,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,    5,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,    6,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,    7,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,    8,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,    9,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E,  10,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E,  11,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E,  12,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E,  13,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E,  14,
    PRV_CPSS_BOBCAT3_PIPE0_CNC0_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  15,

    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,   16,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,   17,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,   18,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,   19,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,   20,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,   21,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,   22,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,   23,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,   24,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,   25,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E,  26,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E,  27,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E,  28,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E,  29,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E,  30,
    PRV_CPSS_BOBCAT3_PIPE0_CNC1_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  31,

    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,    0,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,    1,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,    2,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,    3,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,    4,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,    5,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,    6,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,    7,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,    8,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,    9,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E,  10,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E,  11,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E,  12,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E,  13,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E,  14,
    PRV_CPSS_BOBCAT3_PIPE1_CNC0_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  15,

    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,   16,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,   17,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,   18,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,   19,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,   20,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,   21,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,   22,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,   23,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,   24,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,   25,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E,  26,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E,  27,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E,  28,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E,  29,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E,  30,
    PRV_CPSS_BOBCAT3_PIPE1_CNC1_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  31,

 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_BM_MISC_E,
    PRV_CPSS_BOBCAT3_BM_GEN1_SUM_GLOBAL_RX_FULL_E,           4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_CNTR_OVERFLOW_E,
    PRV_CPSS_BOBCAT3_RX_SDMA_RESOURCE_ERROR_CNT_OF_E,   0,
    PRV_CPSS_BOBCAT3_RX_SDMA_BYTE_CNT_OF_E,             1,
    PRV_CPSS_BOBCAT3_RX_SDMA_PACKET_CNT_OF_E,           2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_MISC_E,
    PRV_CPSS_BOBCAT3_TXQ_GEN_SUM_QUEUE_WRONG_ADDR_E,    0,
    PRV_CPSS_BOBCAT3_TXQ_GEN_SUM_GLOBAL_DESC_FULL_E,    1,
    PRV_CPSS_BOBCAT3_TXQ_GEN_SUM_GLOBAL_BUFF_FULL_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_SCT_RATE_LIMITER_E,
    SET_EVENT_PER_INDEX____0_127___MAC(PRV_CPSS_BOBCAT3_PIPE0_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER,PKT_DROPED_E),
    SET_EVENT_PER_INDEX____0_127___MAC(PRV_CPSS_BOBCAT3_PIPE1_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER,PKT_DROPED_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,
    PRV_CPSS_BOBCAT3_PIPE0_IPLR0_ADDRESS_OUT_OF_MEMORY_E,       0,
    PRV_CPSS_BOBCAT3_PIPE0_EPLR_SUM_ADDRESS_OUT_OF_MEMORY_E,    1,
    PRV_CPSS_BOBCAT3_PIPE0_IPLR1_SUM_ADDRESS_OUT_OF_MEMORY_E,   2,

    PRV_CPSS_BOBCAT3_PIPE1_IPLR0_ADDRESS_OUT_OF_MEMORY_E,       0,
    PRV_CPSS_BOBCAT3_PIPE1_EPLR_SUM_ADDRESS_OUT_OF_MEMORY_E,    1,
    PRV_CPSS_BOBCAT3_PIPE1_IPLR1_SUM_ADDRESS_OUT_OF_MEMORY_E,   2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_DATA_ERR_E,
    PRV_CPSS_BOBCAT3_PIPE0_EPLR_SUM_DATA_ERROR_E,           1,
    PRV_CPSS_BOBCAT3_PIPE1_EPLR_SUM_DATA_ERROR_E,           1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,
    PRV_CPSS_BOBCAT3_PIPE0_IPLR0_IPFIX_WRAPAROUND_E,        0,
    PRV_CPSS_BOBCAT3_PIPE0_EPLR_SUM_IPFIX_WRAPAROUND_E,     1,
    PRV_CPSS_BOBCAT3_PIPE0_IPLR1_SUM_IPFIX_WRAPAROUND_E,    2,

    PRV_CPSS_BOBCAT3_PIPE1_IPLR0_IPFIX_WRAPAROUND_E,        0,
    PRV_CPSS_BOBCAT3_PIPE1_EPLR_SUM_IPFIX_WRAPAROUND_E,     1,
    PRV_CPSS_BOBCAT3_PIPE1_IPLR1_SUM_IPFIX_WRAPAROUND_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_ALARM_E,
    PRV_CPSS_BOBCAT3_PIPE0_IPLR0_IPFIX_ALARM_E,             0,
    PRV_CPSS_BOBCAT3_PIPE0_EPLR_SUM_IPFIX_ALARM_E,          1,
    PRV_CPSS_BOBCAT3_PIPE0_IPLR1_SUM_IPFIX_ALARM_E,         2,

    PRV_CPSS_BOBCAT3_PIPE1_IPLR0_IPFIX_ALARM_E,             0,
    PRV_CPSS_BOBCAT3_PIPE1_EPLR_SUM_IPFIX_ALARM_E,          1,
    PRV_CPSS_BOBCAT3_PIPE1_IPLR1_SUM_IPFIX_ALARM_E,         2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,
    PRV_CPSS_BOBCAT3_PIPE0_IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,       0,
    PRV_CPSS_BOBCAT3_PIPE0_EPLR_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1,
    PRV_CPSS_BOBCAT3_PIPE0_IPLR1_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,   2,

    PRV_CPSS_BOBCAT3_PIPE1_IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,       0,
    PRV_CPSS_BOBCAT3_PIPE1_EPLR_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1,
    PRV_CPSS_BOBCAT3_PIPE1_IPLR1_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,   2,

 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 0,

    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 1,

    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 2,

    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 0,

    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 1,

    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_LOOKUP0_FIFO_FULL_E,        0,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_LOOKUP1_FIFO_FULL_E,        1,
    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_LOOKUP2_FIFO_FULL_E,        2,

    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_LOOKUP0_FIFO_FULL_E,        0,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_LOOKUP1_FIFO_FULL_E,        1,
    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_LOOKUP2_FIFO_FULL_E,        2,
MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_EB_NA_FIFO_FULL_E,
    PRV_CPSS_BOBCAT3_FDB_NA_FIFO_FULL0_E    , 0 ,
    PRV_CPSS_BOBCAT3_FDB_NA_FIFO_FULL1_E    , 1 ,
    PRV_CPSS_BOBCAT3_FDB_NA_FIFO_FULL2_E    , 2 ,
    PRV_CPSS_BOBCAT3_FDB_NA_FIFO_FULL3_E    , 3 ,
    PRV_CPSS_BOBCAT3_FDB_NA_FIFO_FULL4_E    , 4 ,
    PRV_CPSS_BOBCAT3_FDB_NA_FIFO_FULL5_E    , 5 ,
    PRV_CPSS_BOBCAT3_FDB_NA_FIFO_FULL6_E    , 6 ,
    PRV_CPSS_BOBCAT3_FDB_NA_FIFO_FULL7_E    , 7 ,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
    PRV_CPSS_BOBCAT3_PIPE0_TTI_DATA_ERROR_INT0_E, 0,
    PRV_CPSS_BOBCAT3_PIPE0_TTI_DATA_ERROR_INT1_E, 1,
    PRV_CPSS_BOBCAT3_PIPE0_TTI_DATA_ERROR_INT2_E, 2,
    PRV_CPSS_BOBCAT3_PIPE0_TTI_DATA_ERROR_INT3_E, 3,

    PRV_CPSS_BOBCAT3_PIPE1_TTI_DATA_ERROR_INT0_E, 0,
    PRV_CPSS_BOBCAT3_PIPE1_TTI_DATA_ERROR_INT1_E, 1,
    PRV_CPSS_BOBCAT3_PIPE1_TTI_DATA_ERROR_INT2_E, 2,
    PRV_CPSS_BOBCAT3_PIPE1_TTI_DATA_ERROR_INT3_E, 3,
 MARK_END_OF_UNI_EV_CNS,

 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */
 CPSS_PP_CRITICAL_HW_ERROR_E,
/* The following event is in valid traffic case. The event is when allocation counter get to total buffers limit.
   This is legal situation for stress test scenarios.
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_RX_DMA0_SUM_GLOBAL_ALLOC_COUNTER_OVERFLOW_E), */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_RX_DMA0_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_RX_DMA0_INT0_IBUF_FIFO_OUT_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_RX_DMA1_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_RX_DMA1_INT0_IBUF_FIFO_OUT_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_RX_DMA2_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_RX_DMA2_INT0_IBUF_FIFO_OUT_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_RX_DMA3_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_RX_DMA3_INT0_IBUF_FIFO_OUT_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_RX_DMA4_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_RX_DMA4_INT0_IBUF_FIFO_OUT_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_RX_DMA5_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_RX_DMA5_INT0_IBUF_FIFO_OUT_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_0_SUM_CREDITS_COUNTER_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_0_SUM_RX_LB_TOKEN_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_0_SUM_RX_WR_DATA_TO_BANK_WITHOUT_VALID_BANK_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_0_SUM_RX_IDDB_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_0_SUM_RX_PHYSICAL_BANK_SYNC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_RX_CLIENT_0_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PARITY_SUM_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_CNTR_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_CNTR_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_OVERFLOW_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC0_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC1_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC2_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC3_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC4_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC5_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC6_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC7_UNDERFLOW_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_0_INVALID_BUFFER_CLEAR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_0_NEXT_TABLE_UPDATE_WAS_LOST_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMA_SUM_MC_CNT_COUNTER_FLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMA_SUM_SHIFTER_LATENCY_FIFO_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMA_SUM_MC_CNT_PARITY_ERROR_E),


    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_DMA0_SUM_RXDMA_UPDATES_FIFOS_OVERRUN_INTERRUPT_CAUSE_REG1_SUMMARY_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_DMA0_GENENAL_TX_Q_TO_TX_DMA_HA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_DMA0_GENENAL_TX_Q_TO_TX_DMA_DESC_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_1_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_2_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_3_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_4_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_5_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_6_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_7_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_8_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_9_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_10_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_11_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_NOT_READY1_SHIFTER_12_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_1_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_2_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_3_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_4_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_5_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_6_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_7_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_8_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_9_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_10_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_11_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO1_NOT_READY1_SHIFTER_12_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_1_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_2_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_3_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_4_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_5_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_6_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_7_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_8_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_9_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_10_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_11_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO2_NOT_READY1_SHIFTER_12_SOP_EOP_FIFO_NOT_READY_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_1_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_2_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_3_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_4_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_5_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_6_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_7_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_8_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_9_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_10_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_11_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_NOT_READY1_SHIFTER_12_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_1_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_2_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_3_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_4_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_5_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_6_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_7_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_8_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_9_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_10_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_11_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO4_NOT_READY1_SHIFTER_12_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_1_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_2_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_3_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_4_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_5_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_6_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_7_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_8_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_9_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_10_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_11_SOP_EOP_FIFO_NOT_READY_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO5_NOT_READY1_SHIFTER_12_SOP_EOP_FIFO_NOT_READY_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_DMA3_GENENAL_TX_Q_TO_TX_DMA_HA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_DMA3_GENENAL_TX_Q_TO_TX_DMA_DESC_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMX_FREE_ENTRY_FIFO_SER_ERROR0_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMX_FREE_ENTRY_FIFO_SER_ERROR1_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMX_FREE_ENTRY_FIFO_SER_ERROR2_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMX_FREE_ENTRY_FIFO_SER_ERROR3_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMX_FREE_ENTRY_FIFO_SER_ERROR4_E),

 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_DATA_INTEGRITY_ERROR_E,
    /* -- _ECC_ --*/

/*full register*/    SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_BOBCAT3_PIPE0_CRITICAL_ECC_ERROR_BM_CRITICAL_ECC_SUM_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TTI_SECOND_CRITICAL_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TTI_SECOND_CRITICAL_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_MLL_SUM_FILE_ECC_1_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_MLL_SUM_FILE_ECC_2_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_EFT_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_EFT_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),




#if 0
    PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),

#endif
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_DMA0_GENENAL_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_DMA0_GENENAL_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_GENERAL1_ECC_0_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_TX_FIFO0_GENERAL1_ECC_0_DOUBLE_ERROR_E),
    /*-- DATA_ERROR --*/

    /* no other _DATA_ERROR_ beside those that already bound in:
        CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
        CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
        CPSS_PP_POLICER_DATA_ERR_E
    */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_IPLR0_DATA_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE0_IPLR1_SUM_DATA_ERROR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TTI_SECOND_CRITICAL_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TTI_SECOND_CRITICAL_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_MLL_SUM_FILE_ECC_1_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_MLL_SUM_FILE_ECC_2_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_EFT_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_EFT_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),

#if 0
    PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
#endif
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_DMA3_GENENAL_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_DMA3_GENENAL_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_GENERAL1_ECC_0_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_TX_FIFO3_GENERAL1_ECC_0_DOUBLE_ERROR_E),
    /*-- DATA_ERROR --*/

    /* no other _DATA_ERROR_ beside those that already bound in:
        CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
        CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
        CPSS_PP_POLICER_DATA_ERR_E
    */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_IPLR0_DATA_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PIPE1_IPLR1_SUM_DATA_ERROR_E),


    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_TXQ_GEN_SUM_TD_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PARITY_SUM_PFC_PORT_GROUP0_PFC_IND_FIFO_ECC_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_QCN_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_SUM_ECC_ERROR_SUM_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E),

    /* -- _PARITY_ -- */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_PFC_PARITY_SUM_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E),
    /* BMA is internal unit -- SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_BMA_SUM_MC_CNT_PARITY_ERROR_E),*/

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_BOBCAT3_FUNCTIONAL1_SERVER_INT_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,
    PRV_CPSS_BOBCAT3_PIPE0_INGR_OAM_SUM_KEEP_ALIVE_AGING_E,       0,
    PRV_CPSS_BOBCAT3_PIPE0_EGR_OAM_SUM_KEEP_ALIVE_AGING_E,        1,
    PRV_CPSS_BOBCAT3_PIPE1_INGR_OAM_SUM_KEEP_ALIVE_AGING_E,       0,
    PRV_CPSS_BOBCAT3_PIPE1_EGR_OAM_SUM_KEEP_ALIVE_AGING_E,        1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
    PRV_CPSS_BOBCAT3_PIPE0_INGR_OAM_SUM_EXCESS_KEEPALIVE_E,       0,
    PRV_CPSS_BOBCAT3_PIPE0_EGR_OAM_SUM_EXCESS_KEEPALIVE_E,        1,
    PRV_CPSS_BOBCAT3_PIPE1_INGR_OAM_SUM_EXCESS_KEEPALIVE_E,       0,
    PRV_CPSS_BOBCAT3_PIPE1_EGR_OAM_SUM_EXCESS_KEEPALIVE_E,        1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,
    PRV_CPSS_BOBCAT3_PIPE0_INGR_OAM_SUM_INVALID_KEEPALIVE_HASH_E, 0,
    PRV_CPSS_BOBCAT3_PIPE0_EGR_OAM_SUM_INVALID_KEEPALIVE_HASH_E,  1,
    PRV_CPSS_BOBCAT3_PIPE1_INGR_OAM_SUM_INVALID_KEEPALIVE_HASH_E, 0,
    PRV_CPSS_BOBCAT3_PIPE1_EGR_OAM_SUM_INVALID_KEEPALIVE_HASH_E,  1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
    PRV_CPSS_BOBCAT3_PIPE0_INGR_OAM_SUM_RDI_STATUS_E,             0,
    PRV_CPSS_BOBCAT3_PIPE0_EGR_OAM_SUM_RDI_STATUS_E,              1,
    PRV_CPSS_BOBCAT3_PIPE1_INGR_OAM_SUM_RDI_STATUS_E,             0,
    PRV_CPSS_BOBCAT3_PIPE1_EGR_OAM_SUM_RDI_STATUS_E,              1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,
    PRV_CPSS_BOBCAT3_PIPE0_INGR_OAM_SUM_TX_PERIOD_E,              0,
    PRV_CPSS_BOBCAT3_PIPE0_EGR_OAM_SUM_TX_PERIOD_E,               1,
    PRV_CPSS_BOBCAT3_PIPE1_INGR_OAM_SUM_TX_PERIOD_E,              0,
    PRV_CPSS_BOBCAT3_PIPE1_EGR_OAM_SUM_TX_PERIOD_E,               1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
    PRV_CPSS_BOBCAT3_PIPE0_INGR_OAM_SUM_MEG_LEVEL_EXCEPTION_E,    0,
    PRV_CPSS_BOBCAT3_PIPE0_EGR_OAM_SUM_MEG_LEVEL_EXCEPTION_E,     1,
    PRV_CPSS_BOBCAT3_PIPE1_INGR_OAM_SUM_MEG_LEVEL_EXCEPTION_E,    0,
    PRV_CPSS_BOBCAT3_PIPE1_EGR_OAM_SUM_MEG_LEVEL_EXCEPTION_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,
    PRV_CPSS_BOBCAT3_PIPE0_INGR_OAM_SUM_SOURCE_ERFACE_EXCEPTION_E,   0,
    PRV_CPSS_BOBCAT3_PIPE0_EGR_OAM_SUM_SOURCE_ERFACE_EXCEPTION_E,    1,
    PRV_CPSS_BOBCAT3_PIPE1_INGR_OAM_SUM_SOURCE_ERFACE_EXCEPTION_E,   0,
    PRV_CPSS_BOBCAT3_PIPE1_EGR_OAM_SUM_SOURCE_ERFACE_EXCEPTION_E,    1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
    PRV_CPSS_BOBCAT3_PIPE0_INGR_OAM_SUM_ILLEGAL_OAM_ENTRY_INDEX_E,    0,
    PRV_CPSS_BOBCAT3_PIPE0_EGR_OAM_SUM_ILLEGAL_OAM_ENTRY_INDEX_E,     1,
    PRV_CPSS_BOBCAT3_PIPE1_INGR_OAM_SUM_ILLEGAL_OAM_ENTRY_INDEX_E,    0,
    PRV_CPSS_BOBCAT3_PIPE1_EGR_OAM_SUM_ILLEGAL_OAM_ENTRY_INDEX_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
    SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_AP_DOORBELL,LINK_STATUS_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_SRVCPU_PORT_802_3_AP_E,
    SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_BOBCAT3_AP_DOORBELL,802_3_AP_E),
 MARK_END_OF_UNI_EV_CNS,
 /* RX SDMA MG 1*/
 CPSS_PP_MG1_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_0_E,     8+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_1_E,     8+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_2_E,     8+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_3_E,     8+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_4_E,     8+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_5_E,     8+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_6_E,     8+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_BUFFER_QUEUE_7_E,     8+7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE0_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_ERROR_QUEUE_0_E,      8+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE1_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_ERROR_QUEUE_1_E,      8+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE2_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_ERROR_QUEUE_2_E,      8+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE3_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_ERROR_QUEUE_3_E,      8+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE4_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_ERROR_QUEUE_4_E,      8+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE5_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_ERROR_QUEUE_5_E,      8+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE6_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_ERROR_QUEUE_6_E,      8+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE7_E,
    PRV_CPSS_BOBCAT3_MG1_RX_SDMA_RX_ERROR_QUEUE_7_E,      8+7,
 MARK_END_OF_UNI_EV_CNS,

 /* RX SDMA MG 2*/
 CPSS_PP_MG2_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_0_E,     16+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_1_E,     16+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_2_E,     16+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_3_E,     16+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_4_E,     16+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_5_E,     16+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_6_E,     16+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_BUFFER_QUEUE_7_E,     16+7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE0_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_ERROR_QUEUE_0_E,      16+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE1_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_ERROR_QUEUE_1_E,      16+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE2_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_ERROR_QUEUE_2_E,      16+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE3_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_ERROR_QUEUE_3_E,      16+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE4_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_ERROR_QUEUE_4_E,      16+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE5_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_ERROR_QUEUE_5_E,      16+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE6_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_ERROR_QUEUE_6_E,      16+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE7_E,
    PRV_CPSS_BOBCAT3_MG2_RX_SDMA_RX_ERROR_QUEUE_7_E,      16+7,
 MARK_END_OF_UNI_EV_CNS,

 /* RX SDMA MG 3*/
 CPSS_PP_MG3_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_0_E,     24+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_1_E,     24+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_2_E,     24+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_3_E,     24+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_4_E,     24+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_5_E,     24+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_6_E,     24+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_BUFFER_QUEUE_7_E,     24+7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE0_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_ERROR_QUEUE_0_E,      24+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE1_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_ERROR_QUEUE_1_E,      24+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE2_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_ERROR_QUEUE_2_E,      24+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE3_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_ERROR_QUEUE_3_E,      24+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE4_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_ERROR_QUEUE_4_E,      24+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE5_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_ERROR_QUEUE_5_E,      24+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE6_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_ERROR_QUEUE_6_E,      24+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE7_E,
    PRV_CPSS_BOBCAT3_MG3_RX_SDMA_RX_ERROR_QUEUE_7_E,      24+7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E,
    PRV_CPSS_BOBCAT3_TAI_INCOMING_TRIGGER_INT_E,  0,
    PRV_CPSS_BOBCAT3_TAI1_INCOMING_TRIGGER_INT_E, 1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PTP_TAI_GENERATION_E,
    PRV_CPSS_BOBCAT3_TAI_GENERATION_INT_E,  0,
    PRV_CPSS_BOBCAT3_TAI1_GENERATION_INT_E, 1,
 MARK_END_OF_UNI_EV_CNS
};
/* Interrupt cause to unified event map for Bobcat3 with extended data size */
static GT_U32 bobcat3UniEvMapTableWithExtDataSize = (sizeof(bobcat3UniEvMapTableWithExtData)/(sizeof(GT_U32)));


/* Interrupt cause to unified event map for Bobcat3
 This Table is for unified event without extended data
*/
static GT_U32 bobcat3UniEvMapTable[][2] =
{
 {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_BOBCAT3_MISC_TWSI_TIME_OUT_E},
 {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_BOBCAT3_MISC_TWSI_STATUS_E},
 {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_BOBCAT3_MISC_ILLEGAL_ADDR_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},

 {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,   PRV_CPSS_BOBCAT3_PIPE0_BRIDGE_UPDATE_SECURITY_BREACH_E},
 {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,   PRV_CPSS_BOBCAT3_PIPE1_BRIDGE_UPDATE_SECURITY_BREACH_E},
 {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,          PRV_CPSS_BOBCAT3_FDB_NUM_OF_HOP_EX_P_E},
 {CPSS_PP_MAC_NA_LEARNED_E,              PRV_CPSS_BOBCAT3_FDB_NA_LEARNT_E},
 {CPSS_PP_MAC_NA_NOT_LEARNED_E,          PRV_CPSS_BOBCAT3_FDB_NA_NOT_LEARNT_E},
 {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,   PRV_CPSS_BOBCAT3_FDB_AGE_VIA_TRIGGER_ENDED_E},
 {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,    PRV_CPSS_BOBCAT3_FDB_AU_PROC_COMPLETED_E},
 {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,    PRV_CPSS_BOBCAT3_FDB_AU_MSG_TOCPU_READY_E},
 {CPSS_PP_MAC_NA_SELF_LEARNED_E,         PRV_CPSS_BOBCAT3_FDB_NA_SELF_LEARNED_E},
 {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,     PRV_CPSS_BOBCAT3_FDB_N_AFROM_CPU_LEARNED_E},
 {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,     PRV_CPSS_BOBCAT3_FDB_N_AFROM_CPU_DROPPED_E},
 {CPSS_PP_MAC_AGED_OUT_E,                PRV_CPSS_BOBCAT3_FDB_AGED_OUT_E},
 {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,     PRV_CPSS_BOBCAT3_FDB_AU_FIFO_TO_CPU_IS_FULL_E},
 {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_BOBCAT3_MISC_AUQ_PENDING_E},
 {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_BOBCAT3_MISC_AU_QUEUE_FULL_E},
 {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_BOBCAT3_MISC_AUQ_OVERRUN_E},
 {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_BOBCAT3_MISC_AUQ_ALMOST_FULL_E},
 {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_BOBCAT3_MISC_FUQ_PENDING_E},
 {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_BOBCAT3_MISC_FU_QUEUE_FULL_E},
 {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_BOBCAT3_MISC_GENXS_READ_DMA_DONE_E},
 {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_BOBCAT3_MISC_PEX_ADDR_UNMAPPED_E},
 {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_BOBCAT3_PIPE0_BRIDGE_ADDRESS_OUT_OF_RANGE_E},
 {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_BOBCAT3_PIPE1_BRIDGE_ADDRESS_OUT_OF_RANGE_E},

 {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,PRV_CPSS_BOBCAT3_PIPE0_TTI_CPU_ADDRESS_OUT_OF_RANGE_E},
 {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,PRV_CPSS_BOBCAT3_PIPE1_TTI_CPU_ADDRESS_OUT_OF_RANGE_E},

 {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,    PRV_CPSS_BOBCAT3_PIPE0_IPCL_SUM_MG_ADDR_OUT_OF_RANGE_E},
 {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,    PRV_CPSS_BOBCAT3_PIPE1_IPCL_SUM_MG_ADDR_OUT_OF_RANGE_E},
 {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_BOBCAT3_MISC_Z80_INTERRUPT_E},
 {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, PRV_CPSS_BOBCAT3_FDB_BLC_OVERFLOW_E},
 {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_BOBCAT3_AP_DOORBELL_MAIN_IPC_E}
};

/* Interrupt cause to unified event map for Bobcat3 without extended data size */
static GT_U32 bobcat3UniEvMapTableSize = (sizeof(bobcat3UniEvMapTable)/(sizeof(GT_U32)*2));


/**
* @internal setBobcat3DedicatedEventsConvertInfo function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*         Bobcat3 devices.
*/
static void setBobcat3DedicatedEventsConvertInfo(IN GT_U8   devNum)
{
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = bobcat3UniEvMapTableWithExtData;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = bobcat3UniEvMapTableWithExtDataSize;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableArr              = bobcat3UniEvMapTable;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableSize             = bobcat3UniEvMapTableSize;
    prvCpssDrvPpConfig[devNum]->intCtrl.numReservedPorts              = 0;

    return;
}

/**
* @internal prvCpssDrvInterruptBobcat3MaskSet function
* @endinternal
*
* @brief   masks/unmasks a given interrupt bit in the relevant
*         interrupt mask register.
*         Makes the same as prvCpssDrvInterruptMaskSet but in case of
*         interrupt PRV_CPSS_BOBCAT3_FUNCTIONAL1_SERVER_INT_E (reflects state of
*         summary bit of DFX Server interrupt summary register) additionally
*         masks/unmasks DFX Server, DFX Clients interrupts summary registers bits
*         responsible for DFX RAMs Data Integrity events.
* @param[in] evNode                   - The interrupt node representing the interrupt to be
*                                      unmasked.
* @param[in] operation                - The  to perform, mask or unmask the interrupt
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvCpssDrvInterruptBobcat3MaskSet
(
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC *evNode,
    IN CPSS_EVENT_MASK_SET_ENT      operation
)
{
    GT_STATUS rc;
    rc = prvCpssDrvInterruptMaskSet(evNode, operation);

    if (GT_OK == rc &&
        evNode->intCause   == PRV_CPSS_BOBCAT3_FUNCTIONAL1_SERVER_INT_E &&
        evNode->uniEvCause == CPSS_PP_DATA_INTEGRITY_ERROR_E)
    {
        CPSS_API_LOCK_MAC(evNode->devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        rc = prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSet(evNode->devNum,
                                                                    operation);

        CPSS_API_UNLOCK_MAC(evNode->devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    return rc;
}


/**
* @internal prvCpssDrvPpIntDefDxChBobcat3Init function
* @endinternal
*
* @brief   Interrupts initialization for the Bobcat3 devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChBobcat3Init
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
                            /* pointer to device interrupt info */

    ppRevision = ppRevision;

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    setBobcat3DedicatedEventsConvertInfo(devNum);

    if (!bobcat3IntrScanArr_initDone)
    {
    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
        rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
                CPSS_PP_FAMILY_DXCH_BOBCAT3_E,
                BOBCAT3_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
                bobcat3IntrScanArr,
                BOBCAT3_NUM_MASK_REGISTERS_CNS,
                bobcat3MaskRegDefaultSummaryArr,
                bobcat3MaskRegMapArr);
        if(rc != GT_OK)
        {
            /* error */
            return rc;
        }

        rc =  prvCpssDrvCgPtpIntTreeWa(BOBCAT3_NUM_ELEMENTS_IN_SCAN_TREE_CNS,bobcat3IntrScanArr,
                            BOBCAT3_NUM_MASK_REGISTERS_CNS,bobcat3MaskRegMapArr);
        if(rc != GT_OK)
        {
            /* error */
            return rc;
        }


        rc = prvCpssDrvPpIntUnMappedMgSetSkip(devNum, bobcat3MaskRegMapArr, BOBCAT3_NUM_MASK_REGISTERS_CNS);
        if(rc != GT_OK)
        {
            /* error */
            return rc;
        }

        bobcat3IntrScanArr_initDone = 1;
    }
    /* bobcat3 info */
    devInterruptInfoPtr =
        &prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT3_E];

    devInterruptInfoPtr->numOfInterrupts = PRV_CPSS_BOBCAT3_LAST_INT_E;
    devInterruptInfoPtr->maskRegistersAddressesArray = bobcat3MaskRegMapArr;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = bobcat3MaskRegDefaultSummaryArr;
    devInterruptInfoPtr->interruptsScanArray = bobcat3IntrScanArr;
    devInterruptInfoPtr->numOfScanElements = BOBCAT3_NUM_ELEMENTS_IN_SCAN_TREE_CNS;
    devInterruptInfoPtr->fdbTrigEndedId = 0;/* don't care */
    devInterruptInfoPtr->fdbTrigEndedCbPtr = NULL;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert;
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;
    devInterruptInfoPtr->intMaskSetFptr = prvCpssDrvInterruptBobcat3MaskSet;

    return GT_OK;
}

#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefBobcat3Print function
* @endinternal
*
* @brief   print the interrupts arrays info of Bobcat3 devices
*/
void  prvCpssDrvPpIntDefBobcat3Print(
    void
)
{
    cpssOsPrintf("Bobcat3 - start : \n");
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(BOBCAT3_NUM_ELEMENTS_IN_SCAN_TREE_CNS, bobcat3IntrScanArr,
                            BOBCAT3_NUM_MASK_REGISTERS_CNS, bobcat3MaskRegDefaultSummaryArr, bobcat3MaskRegMapArr);
    cpssOsPrintf("Bobcat3 - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefBobcat3Print_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for Bobcat3 devices the register info according to value in
*         PRV_CPSS_BOBCAT3_INT_CAUSE_ENT (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefBobcat3Print_regInfoByInterruptIndex(
    IN PRV_CPSS_BOBCAT3_INT_CAUSE_ENT   interruptId
)
{
    cpssOsPrintf("Bobcat3 - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_BOBCAT3_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId, PRV_CPSS_BOBCAT3_LAST_INT_E);
    }
    else
    {
        prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(BOBCAT3_NUM_ELEMENTS_IN_SCAN_TREE_CNS, bobcat3IntrScanArr, interruptId);
    }
    cpssOsPrintf("Bobcat3 - End regInfoByInterruptIndex : \n");
}
