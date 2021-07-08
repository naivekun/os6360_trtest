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
* @file prvCpssDxChHwRegAddrVer1_ppDb.h
*
* @brief This file includes definitions of all different 'eArch' registers
* addresses to be used by all functions accessing PP registers.
* a variable of type PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC should be allocated
* for each PP type.
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_HW_REG_ADDR_VER1_PP_DB_H
#define __PRV_CPSS_DXCH_HW_REG_ADDR_VER1_PP_DB_H

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChModuleConfig.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>

#include <cpssDriver/pp/prvCpssDrvPpDefs.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* define invalid base address (use 0x1 as LSBits to allow cpssDriver to catch
  access to not valid registers/units and not crash on the device/simulation on
  non-exists memory)
  the cpssDriver will issue 'CPSS_LOG_ERROR_AND_RETURN_MAC' message on such access.
*/
#define NON_VALID_BASE_ADDR_CNS         0xFE000001
#define END_OF_TABLE_INDICATOR_CNS      0xFFFFFFFE

/* Lion2 units */
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TTI_CNS                    0x01000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_PCL_CNS                    0x0B800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_L2I_CNS                    0x02000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_FDB_CNS                    0x06000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EQ_CNS                     0x0B000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LPM_CNS                    NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EGF_EFT_CNS                NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EGF_QAG_CNS                NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EGF_SHT_CNS                NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_HA_CNS                     0x0E800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ETS_CNS                    (PRV_CPSS_DXCH_LION2_BASE_ADDRESS_HA_CNS + 0x800)
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MLL_CNS                    0x0C800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_IPLR_CNS                   0x0C000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EPLR_CNS                   0x07800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_IPVX_CNS                   0x02800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_IOAM_CNS                   NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EOAM_CNS                   NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TCAM_CNS                   NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_RXDMA_CNS                  0x0F000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EPCL_CNS                   0x0E000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_FCU_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_DROP_CNS                NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_QMAP_CNS                NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_CNS                     NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_GOP_CNS                    0x08800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MIB_CNS                    0x09000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_SERDES_CNS                 0x09800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ERMRK_CNS                  NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_BM_CNS                     0x03000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_INGRESS_GLUE_CNS        NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_EGRESS_GLUE_CNS         NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ETH_TXFIFO_CNS             NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ILKN_TXFIFO_CNS            NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ILKN_CNS                   NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS0_0_CNS                 0x04000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS0_1_CNS                 0x04800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS0_2_CNS                 0x05000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS0_3_CNS                 0x05800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TX_FIFO_CNS                NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_BMA_CNS                    0x03800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_CNC_0_CNS                  0x08000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_CNC_1_CNS                  NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_QUEUE_CNS              0x10000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_LL_CNS                 0x10800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_PFC_CNS                0x13800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_QCN_CNS                0x3F000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_DQ_CNS                 0x11000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_DFX_SERVER_CNS             0x01800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MPPM_CNS                   0x06800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS1_0_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS1_1_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS1_2_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS1_3_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS2_0_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS2_1_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS2_2_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS2_3_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MPPM_1_CNS                 0x0A000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_CTU_0_CNS                  0x07000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_CTU_1_CNS                  0x0A800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_SHT_CNS                0x11800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_EGR0_CNS               0x12000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_EGR1_CNS               0x12800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_DIST_CNS               0x13000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_IPLR_1_CNS                 0x00800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXDMA_CNS                  0x0F800000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MG_CNS                     0x00000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TCC_PCL_CNS                0x0D000000
#define PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TCC_IPVX_CNS               0x0D800000

/* the base address for non exists 'dummy' units :

   NOTE:
   1. this is 'dummy address' for simulation purposes , to be able to access to it .
   2. the CPU can not access to it !
   3. we use 'unused' memory space --> we can change it to any other unit space.
*/
#define UNIT_BASE_ADDR_MAC(unit_index)                  ((unit_index) << 23)

/* not used memory */
#define DUMMY_UNITS_BASE_ADDR_CNS(index)              0x70000000 + UNIT_BASE_ADDR_MAC(2*index)
/* Bobcat2 units */
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TTI_CNS                    0x01000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_PCL_CNS                    0x02000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_L2I_CNS                    0x03000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_FDB_CNS                    0x04000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EQ_CNS                     0x0D000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LPM_CNS                    0x58000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_EFT_CNS                0x35000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_QAG_CNS                0x3B000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_SHT_CNS                0x48000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_HA_CNS                     0x0F000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ETS_CNS                    (PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_HA_CNS + 0x800)
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MLL_CNS                    0x1D000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPLR_CNS                   0x0B000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EPLR_CNS                   0x0E000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPVX_CNS                   0x06000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IOAM_CNS                   0x1C000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EOAM_CNS                   0x1E000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCAM_CNS                   0x05000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_RXDMA_CNS                  0x17000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EPCL_CNS                   0x14000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_FCU_CNS                 0x08000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_DROP_CNS                0x09000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_QMAP_CNS                0x0A000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_CNS                     0x0C000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_GOP_CNS                    0x10000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MIB_CNS                    0x12000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SERDES_CNS                 0x13000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ERMRK_CNS                  0x15000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_BM_CNS                     0x16000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_INGRESS_GLUE_CNS        0x18000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_EGRESS_GLUE_CNS         0x19000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ETH_TXFIFO_CNS             0x1A000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ILKN_TXFIFO_CNS            0x1B000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ILKN_CNS                   0x1F000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_0_CNS                 0x21000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_1_CNS                 0x22000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_2_CNS                 0x23000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_3_CNS                 0x24000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TX_FIFO_CNS                0x27000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_BMA_CNS                    0x2D000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CNC_0_CNS                  0x2E000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CNC_1_CNS                  0x2F000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_QUEUE_CNS              0x3C000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_LL_CNS                 0x3D000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_PFC_CNS                0x3E000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_QCN_CNS                0x3F000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_DQ_CNS                 0x40000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_DFX_SERVER_CNS             NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MPPM_CNS                   0x47000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_0_CNS                 0x50000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_1_CNS                 0x51000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_2_CNS                 0x52000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_3_CNS                 0x53000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_0_CNS                 0x54000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_1_CNS                 0x55000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_2_CNS                 0x56000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_3_CNS                 0x57000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MPPM_1_CNS                 NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CTU_0_CNS                  NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CTU_1_CNS                  NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_SHT_CNS                NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_EGR0_CNS               NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_EGR1_CNS               NON_VALID_BASE_ADDR_CNS
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_DIST_CNS               DUMMY_UNITS_BASE_ADDR_CNS(4)
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPLR_1_CNS                 0x20000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXDMA_CNS                  0x26000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MG_CNS                     0x00000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCC_PCL_CNS                DUMMY_UNITS_BASE_ADDR_CNS(2)
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCC_IPVX_CNS               DUMMY_UNITS_BASE_ADDR_CNS(3)
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_0_E                    0x54000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_1_E                    0x55000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_2_E                    0x56000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_3_E                    0x57000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_0_E                    0x21000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_1_E                    0x22000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_2_E                    0x23000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_3_E                    0x24000000
#define PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_4_E                    0x50000000

#define PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_RXDMA1_E                      0x68000000
#define PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TXDMA1_E                      0x66000000
#define PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TX_FIFO1_E                    0x67000000
#define PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_ETH_TX_FIFO1_CNS              0x62000000
#define PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_RXDMA_GLUE_CNS                0x63000000
#define PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TXDMA_GLUE_CNS                0x64000000
#define PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_MPPM_CNS                      0x47000000
#define PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TAI_CNS                       0x65000000

/* aldrin units */
#define PRV_CPSS_DXCH_ALDRIN_BASE_ADDRESS_RXDMA2_E                    0x69000000
#define PRV_CPSS_DXCH_ALDRIN_BASE_ADDRESS_TXDMA2_E                    0x6B000000
#define PRV_CPSS_DXCH_ALDRIN_BASE_ADDRESS_TX_FIFO2_E                  0x6A000000

/* bobcat3 addresses */
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MG_CNS                 0x00000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TTI_CNS                0x01000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPCL_CNS               0x02000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_L2I_CNS                0x03000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPVX_CNS               0x04000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR_CNS               0x05000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR1_CNS              0x06000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IOAM_CNS               0x07000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MLL_CNS                0x08000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EQ_CNS                 0x09000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_EFT_CNS            0x0a000000

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_CNS             0x0b000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_1_CNS           0x0c000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_2_CNS           0x0d000000

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_0_CNS              0x0e000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_1_CNS              0x0f000000

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_CNS                0x10000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MIB_CNS                0x12000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_SERDES_CNS             0x13000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_HA_CNS                 0x14000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_ERMRK_CNS              0x15000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPCL_CNS               0x16000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPLR_CNS               0x17000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EOAM_CNS               0x18000000

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_CNS             0x19000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_1_CNS           0x1a000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_2_CNS           0x1b000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_CNS             0x1c000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_1_CNS           0x1d000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_2_CNS           0x1e000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_CNS            0x1f000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_1_CNS          0x20000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_2_CNS          0x21000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IA_0_CNS               0x22000000

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_0_CNS          0x27000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_1_CNS          0x28000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_0_CNS          0x29000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_1_CNS          0x2a000000

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_SHT_CNS            0x30000000 /* 1. was shared between pipes , but now it is 'per pipe'
                                                                                2. was in 0x48000000*/

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_BM_CNS                 0x40000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_BMA_CNS                0x41000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CPFC_CNS               0x42000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_FDB_CNS                0x43000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_CNS                0x44000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_1_CNS              0x45000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_2_CNS              0x46000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_3_CNS              0x47000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_QAG_CNS            0x50000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MPPM_CNS               0x51000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TCAM_CNS               0x52000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_LL_CNS             0x53000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_QCN_CNS            0x54000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_QUEUE_CNS          0x55000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_BMX_CNS            0x56000000
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TAI_CNS                0x57000000

/* Bobcat3 offset of pipe1 addresses from corresponding units in pipe 0*/
#define PRV_CPSS_DXCH_BOBCAT3_PIPE1_OFFSET_CNS                    0x80000000
#define BOBCAT3_PIPE1_MAC(unitAddr)     ((unitAddr) | PRV_CPSS_DXCH_BOBCAT3_PIPE1_OFFSET_CNS)

#define PRV_CPSS_DXCH_BOBCAT3_PIPE1_GOP_OFFSET_CNS                0x00400000
#define PRV_CPSS_DXCH_BOBCAT3_PIPE1_MIB_OFFSET_CNS                0x00800000
#define PRV_CPSS_DXCH_BOBCAT3_PIPE1_TAI_OFFSET_CNS                0x01000000
#define PRV_CPSS_DXCH_BOBCAT3_PIPE1_SERDES_OFFSET_CNS             0x00080000

#define BOBCAT3_SPECIAL_PIPE1_MAC(unitAddr, pipeOffset)     ((unitAddr) + pipeOffset)


#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_3_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_4_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_1_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_5_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_2_CNS)

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_3_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_4_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_1_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_5_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_2_CNS)

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_3_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_4_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_1_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_5_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_2_CNS)

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_3_CNS         BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_CNS  )
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_4_CNS         BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_1_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_5_CNS         BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_2_CNS)

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MIB1_CNS             BOBCAT3_SPECIAL_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MIB_CNS, PRV_CPSS_DXCH_BOBCAT3_PIPE1_MIB_OFFSET_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP1_CNS             BOBCAT3_SPECIAL_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_CNS, PRV_CPSS_DXCH_BOBCAT3_PIPE1_GOP_OFFSET_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TAI1_CNS             BOBCAT3_SPECIAL_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TAI_CNS, PRV_CPSS_DXCH_BOBCAT3_PIPE1_TAI_OFFSET_CNS)

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_SHT_1_CNS        BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_SHT_CNS)

#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TTI_1_CNS            BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TTI_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPCL_1_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPCL_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_L2I_1_CNS            BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_L2I_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPVX_1_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPVX_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR_0_1_CNS         BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR_1_1_CNS         BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR1_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IOAM_1_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IOAM_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MLL_1_CNS            BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MLL_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EQ_1_CNS             BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EQ_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_EFT_1_CNS        BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_EFT_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_0_1_CNS          BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_0_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_1_1_CNS          BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_1_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_SERDES_1_CNS         BOBCAT3_SPECIAL_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_SERDES_CNS, PRV_CPSS_DXCH_BOBCAT3_PIPE1_SERDES_OFFSET_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_HA_1_CNS             BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_HA_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_ERMRK_1_CNS          BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_ERMRK_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPCL_1_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPCL_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPLR_1_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPLR_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EOAM_1_CNS           BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EOAM_CNS)
#define PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IA_1_CNS             BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IA_0_CNS)

/* --------------------------------------------------*
 *  Unit base address defintion                      *
 *---------------------------------------------------*/
#define NON_VALID_ADDR_CNS (GT_U32)(~0)

typedef struct
{
    PRV_CPSS_DXCH_UNIT_ENT unitId;
    GT_U32                 unitBaseAdrr;  /* last line shall be  { PRV_CPSS_DXCH_UNIT_LAST_E , NON_VALID_ADDR_CNS } */
}PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC;

typedef struct
{
     CPSS_PP_FAMILY_TYPE_ENT                 devFamily;
     CPSS_PP_SUB_FAMILY_TYPE_ENT             devSubfamily;
     PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC * unitId2UnitBaseAdrrList;
     GT_U32                                * devUnitsBaseAddr;
}PRV_CPSS_DXCH_UNITID_UNITBASEADDR_x_TABLE2FILL_STC;


typedef struct
{
    GT_BOOL                                              isInit;
    PRV_CPSS_DXCH_UNITID_UNITBASEADDR_x_TABLE2FILL_STC * initTblList;
}PRV_CPSS_DXCH_UNITID_UNITBASEADDR_x_TABLE2FILL_WS_STC;


#define END_OF_TABLE_CNS   (GT_U32*)(GT_UINTPTR)0xFFFFFFFF
/* structure to hold the info about offset of registers array in the reg DB
   'PRV_CPSS_DXCH_PP_REGS_ADDR_STC'/'PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC' */
typedef struct{
    GT_U32*  registerOffsetPtr;  /* pointer offset from start of PRV_CPSS_DXCH_PP_REGS_ADDR_STC/PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC of register */
    GT_U32  numOfRegisters;  /* number of registers from the 'registerOffset' */
}PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC;

#define UNIT_OFFSET_MASK_CNS    0x00FFFFFF

/* structure to hold the info about offset of registers array in the reg DB
   'PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC' */
typedef struct{
    CPSS_DXCH_TABLE_ENT  tableType;  /* pointer offset from start of PRV_CPSS_DXCH_PP_MAC(_devNum)->accessTableInfoPtr []*/
    GT_U32  numOfEntries;/* number of entries from start of table
                            value 0 means use the info from tablesInfo : PRV_CPSS_DXCH_TABLES_INFO_STC*/
}PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC;

/**************************************************/
/* calculate the offset of registers array position in PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC from
   start of the structure */
/* NOTE: in VC10 when the macro hold " - (GT_U32*)NULL" --> error C2099: initializer is not a constant */
#define REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(regArrName)   \
    (((PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC*)NULL)->regArrName)

/* calculate the offset of registers array position in PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC from
   start of the structure */
/* NOTE: in VC10 when the macro hold " - (GT_U32*)NULL" --> error C2099: initializer is not a constant */
#define REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(regSingleName)   \
    &(((PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC*)NULL)->regSingleName)

#define REG_ARR_SIZE_REG_DB_1_STC_MAC(regArrName)   \
    sizeof(((PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC*)NULL)->regArrName)/4
/* the array of registers from 'regDB1' with the 'size' (number of registers) */
/* used as 'entry' in PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC */
#define REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(regArrName)   \
    (GT_U32*)REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(regArrName) , REG_ARR_SIZE_REG_DB_1_STC_MAC(regArrName)

/**************************************************/
/* calculate the offset of registers array position in PRV_CPSS_DXCH_PP_REGS_ADDR_STC from
   start of the structure */
/* NOTE: in VC10 when the macro hold " - (GT_U32*)NULL" --> error C2099: initializer is not a constant */
#define REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(regArrName)   \
    (((PRV_CPSS_DXCH_PP_REGS_ADDR_STC*)NULL)->regArrName)

#define REG_ARR_OFFSET_FROM_DFX_REG_DB_STC_MAC(regArrName)   \
    (((PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC*)NULL)->regArrName)

#define REG_ARR_SIZE_REG_DB_STC_MAC(regArrName)   \
    sizeof(((PRV_CPSS_DXCH_PP_REGS_ADDR_STC*)NULL)->regArrName)/4

/* the array of registers from 'regDB' with the 'size' (number of registers) */
/* used as 'entry' in PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC */
#define REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(regArrName)   \
    (GT_U32*)&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(regArrName) , REG_ARR_SIZE_REG_DB_STC_MAC(regArrName)



/* use DB of non manipulated unit ... because unit 0 hold addresses of all units */
#define NON_FIRST_UNIT_INDEX_CNS    1

/**
* @internal prvCpssDxChHwRegAddrIsAddrInTableArray function
* @endinternal
*
* @brief   check if register address is in one of the listed tables.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] infoArr                  - the array of listed tables to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the listed tables.
* @retval GT_FALSE                 - the register is not in the listed tables.
*/
GT_BOOL  prvCpssDxChHwRegAddrIsAddrInTableArray(
    IN GT_SW_DEV_NUM    devNum,
    IN PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC   *infoArr,
    IN GT_U32       regAddr
);
/**
* @internal prvCpssDxChHwRegAddrIsAddrInRegDb1Array function
* @endinternal
*
* @brief   check if register address is in the array of registers (PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] infoAddrOffset           - offset of the addresses in infoArr[] from the 'instance 0' of the unit.
*                                      NOTE: should be 0 for sip5 devices.
*                                      should have meaningfully value for sip6 devices
* @param[in] infoArr                  - the array of registers to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the array
* @retval GT_FALSE                 - the register is not in the array
*/
GT_BOOL  prvCpssDxChHwRegAddrIsAddrInRegDb1Array(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           infoAddrOffset,
    IN PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC   *infoArr,
    IN GT_U32       regAddr
);

/**
* @internal prvCpssDxChHwRegAddrIsAddrInRegDbArray function
* @endinternal
*
* @brief   check if register address is in the array of registers (PRV_CPSS_DXCH_PP_REGS_ADDR_STC)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] infoAddrOffset           - offset of the addresses in infoArr[] from the 'instance 0' of the unit.
*                                      NOTE: should be 0 for sip5 devices.
*                                      should have meaningfully value for sip6 devices
* @param[in] infoArr                  - the array of registers to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the array
* @retval GT_FALSE                 - the register is not in the array
*/
GT_BOOL  prvCpssDxChHwRegAddrIsAddrInRegDbArray(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           infoAddrOffset,
    IN PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC   *infoArr,
    IN GT_U32       regAddr
);

/**
* @internal prvCpssDxChHwRegAddrIsAddrInDfxRegDbArray function
* @endinternal
*
* @brief   check if register address is in the array of registers (PRV_CPSS_DXCH_RESET_AND_INIT_CTRL_REGS_ADDR_STC)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] infoAddrOffset           - offset of the addresses in infoArr[] from the 'instance 0' of the unit.
*                                      NOTE: should be 0 for sip5 devices.
*                                      should have meaningfully value for sip6 devices
* @param[in] infoArr                  - the array of registers to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the array
* @retval GT_FALSE                 - the register is not in the array
*/
GT_BOOL  prvCpssDxChHwRegAddrIsAddrInDfxRegDbArray(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           infoAddrOffset,
    IN PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC   *infoArr,
    IN GT_U32       regAddr
);


/**
* @internal prvCpssDxChUnitBaseTableGet function
* @endinternal
*
* @brief   get based address lists for all ASICs
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @param[out] devUnitsBaseAddrPtrPtr   - ptr to point to table
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChUnitBaseTableGet
(
    IN  GT_U8    devNum,
    OUT GT_U32 **devUnitsBaseAddrPtrPtr
);


/**
* @internal prvCpssDxChUnitBaseTableInit function
* @endinternal
*
* @brief   init based address lists for all ASICs
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChUnitBaseTableInit
(
    GT_VOID
);

#define FALCON_MAX_UNITS    (800)
extern PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChFalconUnitsIdUnitBaseAddrArr[FALCON_MAX_UNITS];
/**
* @internal prvDxChHwRegAddrFalconDbInit function
* @endinternal
*
* @brief   init the base address manager of the Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*                                       the unitId for the given address
*/
GT_STATUS prvDxChHwRegAddrFalconDbInit(GT_VOID);

/**
* @internal prvCpssDxChFalconHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChFalconHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
);

/**
* @internal prvCpssSip6OffsetFromFirstInstanceGet function
* @endinternal
*
* @brief   for any instance of unit that is duplicated get the address offset from
*         first instance of the unit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] instanceId               - the instance index in which the unitId exists.
* @param[in] instance0UnitId          - the unitId 'name' of first instance.
*                                       the address offset from instance 0
*/
GT_U32   prvCpssSip6OffsetFromFirstInstanceGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   instanceId,
    IN PRV_CPSS_DXCH_UNIT_ENT   instance0UnitId
);

/**
* @internal prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert function
* @endinternal
*
* @brief   Falcon : convert the global GOP MAC port number in device to local GOP MAC port
*         in the pipe , and the pipeId.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalMacPortNum         - the MAC global port number.
*
* @param[out] pipeIndexPtr             - (pointer to) the pipe Index of the MAC port
* @param[out] localMacPortNumPtr       - (pointer to) the MAC local port number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalMacPortNum,
    OUT GT_U32  *pipeIndexPtr,
    OUT GT_U32  *localMacPortNumPtr
);
/**
* @internal prvCpssFalconDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   Falcon : convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaGlobalNumToLocalNumInDpConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *dpIndexPtr,
    OUT GT_U32  *localDmaNumPtr
);
/**
* @internal prvCpssFalconDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaLocalNumInDpToGlobalNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    OUT GT_U32  *globalDmaNumPtr
);
/**
* @internal prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext function
* @endinternal
*
* @brief   Falcon : the pair of {dpIndex,localDmaNum} may support 'mux' of :
*                   SDMA CPU port or network CPU port
*          each of them hold different 'global DMA number'
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
* @param[out] globalDmaNumPtr          - (pointer to) the 'current' DMA global number.
*                                       if 'current' is GT_NA (0xFFFFFFFF) --> meaning need to 'get first'
*
* @param[out] globalDmaNumPtr          - (pointer to) the 'next' DMA global number.
*  GT_OK      on success
*  GT_NO_MORE on success but no more such global DMA port
*  GT_BAD_PARAM on bad param
*  GT_NOT_FOUND on non exists global DMA port
*/
GT_STATUS prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    INOUT GT_U32  *globalDmaNumPtr
);
/**
* @internal prvCpssFalconDmaGlobalDmaMuxed_getNext function
* @endinternal
*
* @brief   Falcon : support 'mux' of SDMA CPU port or network CPU port.
*          the function return 'next' global DMA port number that may share the
*          same local dma number in the same DP.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[out] globalDmaNumPtr          - (pointer to) the 'current' DMA global number.
*
* @param[out] globalDmaNumPtr          - (pointer to) the 'next' DMA global number.
*  GT_OK      on success
*  GT_NO_MORE on success but no more such global DMA port
*  GT_BAD_PARAM on bad param
*  GT_NOT_FOUND on non exists global DMA port
*/
GT_STATUS prvCpssFalconDmaGlobalDmaMuxed_getNext
(
    IN  GT_U8   devNum,
    INOUT GT_U32  *globalDmaNumPtr
);

/**
* @internal prvCpssFalconGlobalDpToTileAndLocalDp function
* @endinternal
*
* @brief   Falcon : convert the global DP number in device to local DP number and tile
*
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDpIndex             - the DP global number.
*
* @param[out] localDpIndexPtr               - (pointer to) the local Data Path (DP) Index
* @param[out] tileIndexPtr           - (pointer to) the  tile number
*                                       GT_OK on success
*/

GT_STATUS prvCpssFalconGlobalDpToTileAndLocalDp
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDpIndex,
    OUT  GT_U32 * localDpIndexPtr,
    OUT  GT_U32  * tileIndexPtr
);

/**
* @internal prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert function
* @endinternal
*
* @brief   Falcon : convert the global DMA number in device to TileId and to local
*         DMA number in the Local DataPath (DP index in tile).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] tileIdPtr                - (pointer to) the tile Id
*                                      dpIndexPtr          - (pointer to) the Data Path (DP) Index (local DP in the tile !!!)
*                                      localDmaNumPtr      - (pointer to) the DMA local number (local DMA in the DP !!!)
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *tileIdPtr,
    OUT GT_U32  *localDpIndexInTilePtr,
    OUT GT_U32  *localDmaNumInDpPtr
);

/**
* @internal prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the Local DataPath (DP index in tile),
*         to global DMA number in the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] tileId                   - The tile Id
* @param[in] localDpIndexInTile       - the Data Path (DP) Index (local DP in the tile !!!)
* @param[in] localDmaNumInDp          - the DMA local number (local DMA in the DP !!!)
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number in the device.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileId,
    IN  GT_U32  localDpIndexInTile,
    IN  GT_U32  localDmaNumInDp,
    OUT GT_U32  *globalDmaNumPtr
);


/**
* @internal prvCpssFalconInitParamsSet function
* @endinternal
*
* @brief   Falcon : init the very first settings in the DB of the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconInitParamsSet
(
    IN  GT_U8   devNum
);

/**
* @internal prvCpssSip6TileOffsetGet function
* @endinternal
*
* @brief   get the address offset in bytes of tile 'x' from tile 0.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] tileId                   - the  to get offset from tile 0.
*                                       the tile address offset in bytes from tile 0
*/
GT_U32   prvCpssSip6TileOffsetGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   tileId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*__PRV_CPSS_DXCH_HW_REG_ADDR_VER1_PP_DB_H*/



