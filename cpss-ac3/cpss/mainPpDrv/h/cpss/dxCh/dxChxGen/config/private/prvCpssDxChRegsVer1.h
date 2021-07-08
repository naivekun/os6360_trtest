/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChRegsVer1h.h
*
* DESCRIPTION:
*       This file includes the declaration of the structure to hold the
*       addresses of eArch PP registers for each different PP type.
*
*
* FILE REVISION NUMBER:
*       $Revision: 63 $
*
*******************************************************************************/
#ifndef __prvCpssDxChRegsVer1h
#define __prvCpssDxChRegsVer1h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegs.h>
/* number of Tiles that the device support */
#define MAX_TILE_CNS  4
/* number of DP (data path) per tile that the device support */
#define MAX_DP_PER_TILE_CNS  8
/* Falcon device tile offset */
#define FALCON_TILE_OFFSET_CNS 0x20000000
/* number of DP (data path) that the device support */
#define MAX_DP_CNS  (MAX_TILE_CNS * MAX_DP_PER_TILE_CNS)/* total 32 */

/* number of pipes per tile that the device support */
#define MAX_PIPES_PER_TILE_CNS  2
/* number of pipes that the device support */
#define MAX_PIPES_CNS  (MAX_TILE_CNS * MAX_PIPES_PER_TILE_CNS)/* total 8 */

/* port per DP --without-- the CPU port (network/SDMA) */
#define FALCON_PORTS_PER_DP_CNS    8

/* number of ravens in Falcon per one tile */
#define FALCON_RAVENS_PER_TILE     4

/* 9 TAI units per tile */
#define FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS        9

#define REGISTER_COVERED_CNS

/* falcon maximum speed profiles for tx configuration*/
#define MAX_SPEED_PROFILE_NUM_CNS  8

/* Check if 'offsetof' is defined by compiler and if not, define it */
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ( (GT_U32) ( (GT_UINTPTR) (&((TYPE *)0)->MEMBER) ) )
#endif

/* Falcon : 1024 registers for per port register */
/* BC3 : 512 registers for per port register */
#define PORTS_NUM_CNS       1024
#define PORTS_BMP_NUM_CNS   ((PORTS_NUM_CNS + 31) / 32)

/* short name to TTI unit registers */
#define PRV_DXCH_REG1_UNIT_TTI_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TTI

/* short name to L2I unit registers */
#define PRV_DXCH_REG1_UNIT_L2I_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->L2I

/* short name to FDB unit registers */
#define PRV_DXCH_REG1_UNIT_FDB_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->FDB

/* short name to EM unit registers */
#define PRV_DXCH_REG1_UNIT_EM_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EM

/* short name to PHA unit registers */
#define PRV_DXCH_REG1_UNIT_PHA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PHA

/* short name to EQ unit registers */
#define PRV_DXCH_REG1_UNIT_EQ_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EQ

/* short name to LPM unit registers */
#define PRV_DXCH_REG1_UNIT_LPM_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LPM

/* short name to EGF_eft unit registers */
#define PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EGF_eft

/* short name to EGF_qag unit registers */
#define PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EGF_qag

/* short name to EGF_sht unit registers */
#define PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EGF_sht

/* short name to HA unit registers */
#define PRV_DXCH_REG1_UNIT_HA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->HA

/* short name to ETS unit registers */
#define PRV_DXCH_REG1_UNIT_ETS_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->HA.ETS_temp

/* short name to MLL unit registers */
#define PRV_DXCH_REG1_UNIT_MLL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->MLL

/* short name to PCL unit (IPCL) registers */
#define PRV_DXCH_REG1_UNIT_PCL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PCL

/* short name to EPCL unit (EPCL) registers */
#define PRV_DXCH_REG1_UNIT_EPCL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EPCL

/* short name to RXDMA unit registers */
#define PRV_DXCH_REG1_UNIT_RXDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->rxDMA[0]

/* short name to sip6_RXDMA unit registers */
#define PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_rxDMA[0]

/* short name to sip6_RXDMA[index] unit registers */
#define PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_rxDMA[index]

/* LMU unit macro   */
/* _tile - tile number */
/* _raven - raven number */
/* _lmu - LMU number */
#define PRV_DXCH_LMU_NUM_MAC(_tile, _raven, _lmu) \
    (_tile) * FALCON_RAVENS_PER_TILE + (_raven) * 2 + (_lmu)
#define PRV_DXCH_REG1_UNIT_LMU_MAC(_devNum, _tile, _raven, _lmu)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LMU[PRV_DXCH_LMU_NUM_MAC(_tile, _raven, _lmu)]

/* short name to IA unit registers */
#define PRV_DXCH_REG1_UNIT_IA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->ingressAggregator[0]

/* short name to RXDMA specific unit registers */
#define PRV_DXCH_REG1_UNIT_RXDMA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->rxDMA[index]

/* short name to IA specific unit registers */
#define PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->ingressAggregator[index]

/* short name to TXDMA unit registers */
#define PRV_DXCH_REG1_UNIT_TXDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->txDMA[0]

#define PRV_DXCH_REG1_UNIT_SIP6_TXDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_txDMA[0]

/* short name to TXDMA specific unit registers */
#define PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->txDMA[index]

#define PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_txDMA[index]

/* short name to TX_FIFO unit registers */
#define PRV_DXCH_REG1_UNIT_TX_FIFO_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->txFIFO[0]

#define PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_txFIFO[0]

/* short name to TX_FIFO specific unit registers */
#define PRV_DXCH_REG1_UNIT_TX_FIFO_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->txFIFO[index]

#define PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_txFIFO[index]

/* short name to SIP_ETH_TXFIFO unit registers */
#define PRV_DXCH_REG1_UNIT_ETH_TXFIFO_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->SIP_ETH_TXFIFO

/* short name to SIP_ILKN_TXFIFO unit registers */
#define PRV_DXCH_REG1_UNIT_ILKN_TXFIFO_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->SIP_ILKN_TXFIFO


/* short name to ERMRK unit registers */
#define PRV_DXCH_REG1_UNIT_ERMRK_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->ERMRK

/* short name to OAM unit registers :
    _stage - 0 --> ingress    (CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E)
    _stage - 1 --> egress     (CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E)
*/
#define PRV_DXCH_REG1_UNIT_OAM_MAC(_devNum,_stage)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->OAMUnit[_stage]

/* TAI subunit                                                  */
/* _gop - Group Of Ports (port number can be converted to _gop) */
/* _id - TAI id in the GOP (0 or 1)                             */
#define PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(_devNum, _gop, _id)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.TAI[_gop][_id]

/* PTP (TSU) subunit - 1/10/100/200/400G ports */
/* _port - port number */
#define PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(_devNum, _port)   \
    ((PRV_CPSS_PORT_MTI_400_E != PRV_CPSS_DXCH_PORT_TYPE_MAC(_devNum, _port)) ?      \
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.PTP[_port][0] :               \
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.PTP[_port][1])


/* CG subunit */
/* CG Port */
#define PRV_DXCH_REG1_UNIT_CG_PORT_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT

/* CG Port MAC */
#define PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT.CG_PORT_MAC

/* CG Port MAC MIBS */
#define PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MIBS_REGS_ADDR_ARR_MAC(_devNum, _port)   \
    &(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[0])

/* CG Converters */
#define PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_CONVERTERS

/* CG_PORT wrapper - get the register address - cg port mac address0 register */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_ADDR0_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACaddress0;

/* CG_PORT wrapper - get the register address - cg port mac frame length */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_FRM_LNGTH_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACFrmLength;

/* CG_PORT wrapper - get the register address - cg port mac command config */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACCommandConfig;

/* CG_PORT wrapper - get the register address - cg port mac command config */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_TX_IPG_LENGTH_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACTxIpgLength;

/* CG_PORT wrapper - get the register address - cg port mac Rx crc opt */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACRxCrcOpt;

/* CG_PORT wrapper - get the register address - cg port mac status */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACStatus;

/* CG_PORT wrapper - get the register address - cg port mac StatN Config */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATN_CONFIG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACStatNConfig;

/* CG_PORT wrapper - get the register address - cg port mac MIBS */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_MIB_REG_ADDR_ENTRIES_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MIBS_REGS_ADDR_ARR_MAC(devNum, portNum);

/* CG_CONVERTERS wrapper - get the register address - cg port mac control register0 */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAControl0;

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters status */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_STATUS_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersStatus;

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters resets */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_RESETS_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersResets

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters ip status */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersIpStatus

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters ip status2 */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS2_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersIpStatus2

/* FCA subunit   */
/* _port - port/mac number */
#define PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.FCA[_port]

#define PRV_DXCH_REG_UNIT_GOP_FCA_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->macRegs.perPortRegs[_port].FCA

#define PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->macRegs.cpuPortRegs.FCA

/* MTI_EXT subunit           */
/* _port - global mac number */
#define PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_port].MTI_EXT

/* short name to PLR unit (PLR) registers */
#define PRV_DXCH_REG1_UNIT_ILKN_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.ILKN

/* short name to PLR unit (PLR) registers */
#define PRV_DXCH_REG1_UNIT_PLR_MAC(_devNum,_stage)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PLR[_stage]

/* short name to CNC unit registers */
#define PRV_DXCH_REG1_UNIT_CNC_MAC(_devNum, _instance)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->CNC[_instance]


/* short name to TXQ_DQ unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.dq[0]

/* short name to TXQ_DQ specific unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.dq[index]


/* short name to TXQ_LL unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_LL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.ll

/* short name to TXQ_PFC unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.pfc

/* short name to TXQ_QCN unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.qcn

/* short name to TXQ_BMX unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_BMX_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.bmx

/* short name to TXQ_Q unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.queue

/* short name to PREQ unit registers */
#define PRV_DXCH_REG1_UNIT_PREQ_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PREQ


/* short name to TM DROP unit registers */
#define PRV_DXCH_REG1_UNIT_TM_DROP_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TMDROP

/* short name to LMS unit registers */
#define PRV_DXCH_REG1_UNIT_LMS_MAC(_devNum,_instance)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.LMS[_instance]

/* short name to MG unit registers */
#define PRV_DXCH_REG1_UNIT_MG_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->MG

/* short name to TM QMAP unit registers */
#define PRV_DXCH_REG1_UNIT_TM_QMAP_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TMQMAP

/* short name to TM_INGR_GLUE unit registers */
#define PRV_DXCH_REG1_UNIT_TM_INGR_GLUE_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TM_INGR_GLUE

/* short name to TM_EGR_GLUE unit registers */
#define PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TM_EGR_GLUE

/* short name to TM_FCU unit registers */
#define PRV_DXCH_REG1_UNIT_TM_FCU_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TM_FCU

/* short name to BM unit registers */
#define PRV_DXCH_REG1_UNIT_BM_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->BM


typedef struct PRV_CPSS_DXCH_PP_TXQ_PIZZA_ARBITER_STCT  /* BC2 / BobK --- 85 registers, */
{                                                       /* Aldrin     --- 96 registres  */
    GT_U32    pizzaArbiterConfigReg[96];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_DXCH_PP_TXQ_PIZZA_ARBITER_STC;

typedef struct PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STCT
{
    GT_U32    pizzaArbiterConfigReg[85];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC;

typedef struct PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STCT  /* BobK   RxDmaGlue - 37  TxDma-Glue --- 37 */
{                                                           /* Aldrin RxDmaGlue - 78  TxDma-Glue --- 37 */
    GT_U32    pizzaArbiterConfigReg[78];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STC;

typedef struct PRV_CPSS_DXCH_PP_DP_TX_PIZZA_ARBITER_STCT /* falcon */
{
    GT_U32    pizzaArbiterConfigReg[256];
    GT_U32    pizzaArbiterCtrlReg;
}PRV_CPSS_DXCH_PP_DP_TX_PIZZA_ARBITER_STCT;


/*--------------------------------------
 * /Cider/EBU/BobK/BobK {Current}/Switching Core/MPPM/Units/<MPPM_IP> MPPM IP Units/<Pizza_arb> SIP_COMMON_MODULES_IP Units/Pizza Arbiter/Pizza Arbiter Configuration Register %n
 *    Register CPU 0x47000200 Pizza Arbiter Control Register
 *    Register CPU 0x47000204 Pizza Arbiter Status
 *    Register CPU 0x47000208 Pizza Arbiter Configuration Register %n (0x47000208 + n*0x4 : where n (0-92) represents reg number)
 *    Register CPU 0x47000400 Pizza Arbiter debug register (Internal)
 *-----------------------------------------*/
typedef struct PRV_CPSS_DXCH_PP_MPPM_PIZZA_ARBITER_STCT
{
    GT_U32    pizzaArbiterConfigReg[93];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_DXCH_PP_MPPM_PIZZA_ARBITER_STC;

/* FALCON 12.8T need 256+2+16 DMAs :
    256 network ports
    2   reduced network ports
    16  SDMA CPU ports
*/
#define PRV_CPSS_MAX_DMA_NUM_CNS  (256+2+16)
/*
 * Typedef: struct PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC
 *
 * Description: A struct including PP registers addresses for register access
 *              by different types of PPs.
 *
 * Fields: The struct includes fields by the names of PP registers, each
 *         field represents the respective register address.
 *
 * Notes:
 *          format of structure is similar to PRV_CPSS_DXCH_PP_REGS_ADDR_STC
 *
 */

/* This registers DB struct is used in several generic algorithm like registers
   dump. These algorithms require that this struct and PRV_CPSS_BOOKMARK_STC
   struct have packed members.
   Use 4 byte alignment to guaranty that nextPtr will be right after size in
   both this structure and in the PRV_CPSS_BOOKMARK_STC.
*/
#pragma pack(push, 4)
typedef struct
{
    struct /*TTI*/{

        struct /*VLANAssignment*/{

            GT_U32    ingrTPIDConfig[4]/*VLAN_Ethertype_index*/;
            GT_U32    ingrTagProfileTPIDSelect[2][2]/*Tag*//*Profile group*/;
            GT_U32    ingrTPIDTagType;

        }VLANAssignment;

        struct /*userDefinedBytes*/{

            GT_U32    IPCLTCPPortRangeComparatorConfig[8]/*TCPCmp*/;
            GT_U32    IPCLUDPPortRangeComparatorConfig[8]/*UDPCmp*/;

        }userDefinedBytes;

        struct /*TTIUnitGlobalConfigs*/{

            GT_U32    TTIUnitGlobalConfig;
            GT_U32    TTIUnitGlobalConfigForCCPorts;
            GT_U32    TTIUnitGlobalConfigExt;

        }TTIUnitGlobalConfigs;

        struct /*TTIEngine*/{

            GT_U32    TTIEngineInterruptCause;
            GT_U32    TTIEngineInterruptMask;
            GT_U32    TTIEngineConfig;
            GT_U32    TTIPCLIDConfig0;
            GT_U32    TTIPCLIDConfig1;
            GT_U32    TTILookupIPExceptionCommands;
            GT_U32    TTIPktTypeKeys[8]/*packet types*/;
            GT_U32    TTILookupIPExceptionCodes0;
            GT_U32    TTIKeySegmentMode0;
            GT_U32    TTIKeySegmentMode1;
            GT_U32    TTIKeyType;
            GT_U32    TTILookupMPLSExceptionCommands;
            GT_U32    TTILookupIPExceptionCodes1;
            GT_U32    TTIPacketTypeExactMatchProfileId[10]/*packet types pairs*/;/* from sip6 */

        }TTIEngine;

        struct /*trunkHashSimpleHash*/{

            GT_U32    trunkHashConfigReg0;
            GT_U32    trunkHashConfigReg1;
            GT_U32    trunkHashConfigReg2;

        }trunkHashSimpleHash;

        struct /*sohoCcfc*/{

            GT_U32    sohoCcfcGlobal;
            GT_U32    sohoCcfcSpeedToTimer[4]/*speed_index*/;

        }sohoCcfc;

        struct /*TRILLConfigs*/{

            GT_U32    TRILLGlobalConfigs0;
            GT_U32    TRILLGlobalConfigs1;
            GT_U32    TRILLExceptionCommands0;
            GT_U32    TRILLExceptionCommands1;
            GT_U32    TRILLGlobalConfigs2;

        }TRILLConfigs;

        struct /*TRILLAdjacency*/{

            GT_U32    adjacencyEntryReg2[128]/*entry*/;
            GT_U32    adjacencyEntryReg1[128]/*entry*/;
            GT_U32    adjacencyEntryReg0[128]/*entry*/;

        }TRILLAdjacency;

        struct /*PTP*/{

            GT_U32    _1588V1DomainDomainNumber[4][4]/*i*//*Domain*/;
            GT_U32    PTPExceptionCheckingModeDomain[5]/*Domain*/;
            GT_U32    piggyBackTSConfigs[8]/*group of ports*/;
            GT_U32    PTPConfigs;
            GT_U32    PTPOverUDPDestinationPorts;
            GT_U32    PTPExceptionsAndCPUCodeConfig;
            GT_U32    PTPEtherTypes;
            GT_U32    PTPExceptionsCntr;
            GT_U32    _1588V2DomainDomainNumber;
            GT_U32    timestampEtherTypes;
            GT_U32    timestampConfigs;

        }PTP;

        struct /*pseudoWire*/{

            GT_U32    MPLSOAMChannelTypeProfile[15]/*channel type*/;
            GT_U32    PWE3Config;
            GT_U32    PWTagModeExceptionConfig;

        }pseudoWire;

        struct /*protocolMatch*/{

            GT_U32    protocolsConfig[6]/*Protocols*/;
            GT_U32    protocolsEncapsulationConfig0;
            GT_U32    protocolsEncapsulationConfig1;

        }protocolMatch;

        struct /*portMap*/{

            GT_U32    portGroupMap[32]/*8 ports*/;
            GT_U32    portListMap[43]/*6 ports*/;

        }portMap;

        struct /*phyPortAndEPortAttributes*/{

            GT_U32    myPhysicalPortAttributes [288]/* index - myPhysicalPort */;
            GT_U32    myPhysicalPortAttributes2[288]/* same registers in second pipe TTI unit */;/* NOTE: NOT used by sip6 CPSS code !!! */
            GT_U32    remotePhysicalPortAssignmentGlobalConfig;

             /*SIP6 registers*/
            GT_U32    remotePhysicalPortAssignmentGlobalConfigExt;
        }phyPortAndEPortAttributes;

        struct /*MRUException*/{

            GT_U32    MRUSize[4]/*2 profiles*/;
            GT_U32    MRUCommandAndCode;

        }MRUException;

        struct /*MAC2ME*/{

            GT_U32    MAC2MEHighMask[128]/*Number of MAC*/;
            GT_U32    MAC2MELow[128]/*Number of MAC*/;
            GT_U32    MAC2MEHigh[128]/*Number of MAC*/;
            GT_U32    MAC2MELowMask[128]/*Number of MAC*/;
            GT_U32    MAC2MESrcInterface[128]/*Mac2Me entry*/;
            GT_U32    MAC2MESrcInterfaceSel[128]/*Mac2Me Entry*/;

        }MAC2ME;

        struct /*IPv6SolicitedNode*/{

            GT_U32    IPv6SolicitedNodeMcAddrConfigReg0;
            GT_U32    IPv6SolicitedNodeMcAddrConfigReg1;
            GT_U32    IPv6SolicitedNodeMcAddrConfigReg2;
            GT_U32    IPv6SolicitedNodeMcAddrConfigReg3;
            GT_U32    IPv6SolicitedNodeMcAddrMaskReg0;
            GT_U32    IPv6SolicitedNodeMcAddrMaskReg1;
            GT_U32    IPv6SolicitedNodeMcAddrMaskReg2;
            GT_U32    IPv6SolicitedNodeMcAddrMaskReg3;

        }IPv6SolicitedNode;

        struct /*FCOE*/{

            GT_U32    FCoEGlobalConfig;
            GT_U32    FCoEExceptionConfigs;
            GT_U32    FCoEExceptionCntrs0;
            GT_U32    FCoEExceptionCntrs1;
            GT_U32    FCoEExceptionCntrs2;

        }FCOE;

        struct /*duplicationConfigs*/{

            GT_U32    duplicationConfigs0;
            GT_U32    duplicationConfigs1;
            GT_U32    MPLSMCDALow;
            GT_U32    MPLSMCDAHigh;
            GT_U32    MPLSMCDALowMask;
            GT_U32    MPLSMCDAHighMask;

        }duplicationConfigs;

        GT_U32    TTIKRuleMatchCntr[12]/*TTICounter*/;
        GT_U32    TTIUserDefinedEtherTypes[4]/*index*/;
        GT_U32    TTIIPv6GREEtherType;
        GT_U32    TTIIPv4GREEthertype;
        GT_U32    oamLabelGlobalConfiguration0;
        GT_U32    oamLabelGlobalConfiguration1;
        GT_U32    specialEtherTypes;
        GT_U32    sourceIDAssignment0;
        GT_U32    sourceIDAssignment1;
        GT_U32    receivedFlowCtrlPktsCntr;
        GT_U32    preRouteExceptionMasks;
        GT_U32    PCLUserDefinedEtherTypes[4]/*index*/;
        GT_U32    MPLSEtherTypes;
        GT_U32    IPv6ExtensionValue;
        GT_U32    IPMinimumOffset;
        GT_U32    IPLengthChecks;
        GT_U32    ECCTwoErrorsCntr;
        GT_U32    ECCOneErrorCntr;
        GT_U32    DSAConfig;
        GT_U32    droppedFlowCtrlPktsCntr;
        GT_U32    CNAndFCConfig;
        GT_U32    CFMEtherType;
        GT_U32    TTIMetalFix;
        GT_U32    ttiCutThroughConfiguration;
        GT_U32    cutThroughByteCountExtractFailCntr;
        GT_U32    cutThroughHdrIntegrityExceptionCfg1;
        GT_U32    cutThroughHdrIntegrityExceptionCfg2;

        /*SIP6 registers*/
        GT_U32    ECNDroppedConfiguration;
    }TTI;


    struct /*L2I*/{

        struct /*VLANRangesConfig*/{

REGISTER_COVERED_CNS            GT_U32    ingrVLANRangeConfig;

        }VLANRangesConfig;

        struct /*VLANMRUProfilesConfig*/{

REGISTER_COVERED_CNS            GT_U32    VLANMRUProfileConfig[4]/*Register*/;

        }VLANMRUProfilesConfig;

        struct /*UDPBcMirrorTrapUDPRelayConfig*/{

REGISTER_COVERED_CNS            GT_U32    UDPBcDestinationPortConfigTable[12]/*Port*/;

        }UDPBcMirrorTrapUDPRelayConfig;

        struct /*securityBreachStatus*/{

REGISTER_COVERED_CNS            GT_U32    securityBreachStatus0;
REGISTER_COVERED_CNS            GT_U32    securityBreachStatus1;
REGISTER_COVERED_CNS            GT_U32    securityBreachStatus2;
REGISTER_COVERED_CNS            GT_U32    securityBreachStatus3;

        }securityBreachStatus;

        struct /*MACBasedQoSTable*/{

REGISTER_COVERED_CNS            GT_U32    MACQoSTableEntry[7]/*QoS Profile*/;

        }MACBasedQoSTable;

        struct /*layer2BridgeMIBCntrs*/{

REGISTER_COVERED_CNS            GT_U32    setVLANIngrFilteredPktCount[2]/*SetNum*/;
REGISTER_COVERED_CNS            GT_U32    cntrsSetConfig0[2]/*Set*/;
REGISTER_COVERED_CNS            GT_U32    cntrsSetConfig1[2]/*Set*/;
REGISTER_COVERED_CNS            GT_U32    setIncomingPktCount[2]/*SetNum*/;
REGISTER_COVERED_CNS            GT_U32    setSecurityFilteredPktCount[2]/*SetNum*/;
REGISTER_COVERED_CNS            GT_U32    setBridgeFilteredPktCount[2]/*SetNum*/;
REGISTER_COVERED_CNS            GT_U32    MACAddrCount0;
REGISTER_COVERED_CNS            GT_U32    MACAddrCount1;
REGISTER_COVERED_CNS            GT_U32    MACAddrCount2;
REGISTER_COVERED_CNS            GT_U32    hostOutgoingMcPktCount;
REGISTER_COVERED_CNS            GT_U32    hostOutgoingPktsCount;
REGISTER_COVERED_CNS            GT_U32    hostIncomingPktsCount;
REGISTER_COVERED_CNS            GT_U32    hostOutgoingBcPktCount;
REGISTER_COVERED_CNS            GT_U32    matrixSourceDestinationPktCount;

        }layer2BridgeMIBCntrs;

        struct /*IPv6McBridgingBytesSelection*/{

REGISTER_COVERED_CNS            GT_U32    IPv6MCBridgingBytesSelectionConfig;

        }IPv6McBridgingBytesSelection;

        struct /*ingrPortsRateLimitConfig*/{

            GT_U32    portSpeedOverrideEnable[8]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    MIISpeedGranularity[8]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    GMIISpeedGranularity[8]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    XGSpeedGranularity[8]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    CGSpeedGranularity[8]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    ingrRateLimitConfig0;
            GT_U32    ingrRateLimitConfig3;
            GT_U32    ingrRateLimitConfig2;
REGISTER_COVERED_CNS            GT_U32    ingrRateLimitConfig1;
REGISTER_COVERED_CNS            GT_U32    ingrRateLimitDropCntr310;
REGISTER_COVERED_CNS            GT_U32    ingrRateLimitDropCntr3932;

        }ingrPortsRateLimitConfig;

        struct /*globalEportConifguration*/{

REGISTER_COVERED_CNS            GT_U32    globalEPortValue;/* in SIP5 but not SIP6 */
REGISTER_COVERED_CNS            GT_U32    globalEPortMask; /* in SIP5 but not SIP6 */
            GT_U32    globalEportMinValue; /* in SIP6 */
            GT_U32    globalEportMaxValue; /* in SIP6 */
        }globalEportConifguration;

        struct /*ctrlTrafficToCPUConfig*/{

REGISTER_COVERED_CNS            GT_U32    IPv6McLinkLocalCPUCodeIndex[8][2]/*CPU Code Index_n*//*CPU Code Index_t*/;
            GT_U32    IPv6McLinkLocalReged[8]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    IPv6McLinkLocalConfig[32]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    IPv6ICMPMsgTypeConfig[2]/*ICMP_Msg_Type*/;
REGISTER_COVERED_CNS            GT_U32    IPv6ICMPCommand;
            GT_U32    IPv4McLinkLocalReged[8]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    IPv4McLinkLocalCPUCodeIndex[8][2]/*Register*//*Register_t*/;
REGISTER_COVERED_CNS            GT_U32    IPv4McLinkLocalConfig[32]/*Register*/;
            GT_U32    IEEEReservedMcReged[8]/*Register*/;
REGISTER_COVERED_CNS/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcCPUIndex[16]/*Register*/;
REGISTER_COVERED_CNS/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig7[16]/*Register*/;
REGISTER_COVERED_CNS/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig6[16]/*Register*/;
REGISTER_COVERED_CNS/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig5[16]/*Register*/;
REGISTER_COVERED_CNS/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig4[16]/*Register*/;
REGISTER_COVERED_CNS/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig3[16]/*Register*/;
REGISTER_COVERED_CNS/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig2[16]/*Register*/;
REGISTER_COVERED_CNS/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig1[16]/*Register*/;
REGISTER_COVERED_CNS            GT_U32    IEEEReservedMcConfig0[16]/*Register*/;

        }ctrlTrafficToCPUConfig;

        struct /*bridgeEngineInterrupts*/{

REGISTER_COVERED_CNS            GT_U32    bridgeInterruptCause;   /*prvCpssDrvPpIntDefDxChLion2Init*/
REGISTER_COVERED_CNS            GT_U32    bridgeInterruptMask;    /*prvCpssDrvPpIntDefDxChLion2Init*/

        }bridgeEngineInterrupts;

        struct /*bridgeEngineConfig*/{

REGISTER_COVERED_CNS            GT_U32    bridgeGlobalConfig0;
REGISTER_COVERED_CNS            GT_U32    bridgeGlobalConfig1;
REGISTER_COVERED_CNS            GT_U32    bridgeCommandConfig0;
REGISTER_COVERED_CNS            GT_U32    bridgeCommandConfig1;
            GT_U32    bridgeCommandConfig2;
REGISTER_COVERED_CNS            GT_U32    bridgeCommandConfig3;
REGISTER_COVERED_CNS            GT_U32    bridgeSecurityBreachDropCntrCfg0;
REGISTER_COVERED_CNS            GT_U32    bridgeSecurityBreachDropCntrCfg1;
            GT_U32    bridgecpuPortConfig;

        }bridgeEngineConfig;

        struct /*bridgeDropCntrAndSecurityBreachDropCntrs*/{

REGISTER_COVERED_CNS            GT_U32    globalSecurityBreachFilterCntr;
REGISTER_COVERED_CNS            GT_U32    portVLANSecurityBreachDropCntr;
REGISTER_COVERED_CNS            GT_U32    bridgeFilterCntr;

        }bridgeDropCntrAndSecurityBreachDropCntrs;

        struct /*bridgeAccessMatrix*/{

REGISTER_COVERED_CNS            GT_U32    bridgeAccessMatrixLine[8]/*SA Access Level*/;
REGISTER_COVERED_CNS            GT_U32    bridgeAccessMatrixDefault;

        }bridgeAccessMatrix;

        struct /*eVlanMirroringToAnalyser*/{

                GT_U32 eVlanMirroringToAnalyserEnable[PORTS_BMP_NUM_CNS];

        }eVlanMirroringToAnalyser;
        GT_U32    l2iBadAddressLatchReg;
    }L2I;

    struct /*EM*/ /* from sip6 */{

        struct /*EMGlobalConfiguration*/{

            GT_U32    EMGlobalConfiguration1;
            GT_U32    EMGlobalConfiguration2;
            GT_U32    EMCRCHashInitConfiguration;

        }EMGlobalConfiguration;

        struct /*EMIndirectAccess*/{

            GT_U32    EMIndirectAccessControl;
            GT_U32    EMIndirectAccessData[5]/*word*/;
            GT_U32    EMIndirectAccessAccess;

        }EMIndirectAccess;

        struct /*EMInterrupt*/{

                GT_U32    EMInterruptCauseReg;
                GT_U32    EMInterruptMaskReg;

        }EMInterrupt;

        struct /*EMHashResults*/{

                GT_U32    EMMultiHashCRCResultReg[8]/*reg_num*/;

        }EMhashResults;

        struct /*EMSchedulerConfiguration*/{

                GT_U32    EMSchedulerSWRRControl;
                GT_U32    EMSchedulerSWRRConfig;

        }EMSchedulerConfiguration;

    }EM;


    struct /*FDB*/{

        struct /*FDBCore*/{

            struct /*sourceAddrNotLearntCntr*/{

                GT_U32    learnedEntryDiscardsCount;

            }sourceAddrNotLearntCntr;

            struct /*FDB*/{

                GT_U32    MTCPipeDispatcher;

            }FDB;

            struct /*FDBInterrupt*/{

                GT_U32    FDBInterruptCauseReg;
                GT_U32    FDBInterruptMaskReg;

            }FDBInterrupt;

            struct /*FDBInternal*/{

                GT_U32    metalFix;
                GT_U32    FDBIdleState;

            }FDBInternal;

            struct /*FDBIndirectAccess*/{

                GT_U32    FDBIndirectAccessData[5]/*word*/;
                GT_U32    FDBIndirectAccessCtrl;
                GT_U32    FDBIndirectAccessAccess;

            }FDBIndirectAccess;

            struct /*FDBHashResults*/{

                GT_U32    FDBMultiHashCRCResultReg[8]/*reg_num*/;
                GT_U32    FDBNonMulti_hash_crc_result;
                GT_U32    FDBXorHash;

            }FDBHashResults;

            struct /*FDBGlobalConfig*/{

                GT_U32    FDBGlobalConfig;
                GT_U32    FDBGlobalConfig1;
                GT_U32    FDBCRCHashInitConfig;
                GT_U32    bridgeAccessLevelConfig;
                GT_U32    FDBAgingWindowSize;

                GT_U32    FDBGlobalConfig2;/*sip6*/
            }FDBGlobalConfig;

            struct /*FDBCntrsUpdate*/{

                GT_U32    FDBCntrsUpdateCtrl;

            }FDBCntrsUpdate;

            struct /*FDBBankCntrs*/{

                GT_U32    FDBBankCntr[16]/*BankNum*/;
                GT_U32    FDBBankRank0;
                GT_U32    FDBBankRank1;
                GT_U32    FDBBankRank2;
                GT_U32    FDBBankRank3;
                GT_U32    FDBBankRank4;
                GT_U32    FDBBankRank5;
                GT_U32    FDBBankRank6;
                GT_U32    FDBBankRank7;
                GT_U32    FDBBankRank8;
                GT_U32    FDBBankRank9;
                GT_U32    FDBBankRank13;
                GT_U32    FDBBankRank12;
                GT_U32    FDBBankRank11;
                GT_U32    FDBBankRank10;
                GT_U32    FDBBankRank14;
                GT_U32    FDBBankRank15;
                GT_U32    FDBBankCntrsCtrl;

            }FDBBankCntrs;

            struct /*FDBAddrUpdateMsgs*/{

                struct /*AUMsgToCPU*/{

                    GT_U32    AUMsgToCPU;
                    GT_U32    AUFIFOToCPUConfig;

                }AUMsgToCPU;

                struct /*AUMsgFromCPU*/{

                    GT_U32    AUMsgFromCPUWord[6]/*word*/;
                    GT_U32    AUMsgFromCPUCtrl;

                }AUMsgFromCPU;

            }FDBAddrUpdateMsgs;

            struct /*FDBAction*/{

                GT_U32    FDBActionGeneral;
                GT_U32    FDBAction0;
                GT_U32    FDBAction1;
                GT_U32    FDBAction2;
                GT_U32    FDBAction3;
                GT_U32    FDBAction4;
                GT_U32    FDBActionOldParameters;
                GT_U32    FDBActionNewParameters;

            }FDBAction;

            struct /*AUMsgRateLimitingCounter*/{

                GT_U32    AUMsgRateLimCnt;

            } AUMsgRateLimitingCounter;

            struct /*FDBScedulerConfiguration*/{

                GT_U32    FDBScedulerSWRRControl;
                GT_U32    FDBScedulerSWRRConfig;

            } FDBScedulerConfiguration;

        }FDBCore;

    }FDB;

    struct /*EQ*/{

        struct /*toCpuPktRateLimiters*/{

            GT_U32    CPUCodeRateLimiterDropCntr;
            GT_U32    toCpuRateLimiterPktCntr[255]/*Limiter*/;

        }toCpuPktRateLimiters;

        struct /*SCTRateLimitersInterrupt*/{

            GT_U32    CPUCodeRateLimitersInterruptCause[32]/*reg*/;
            GT_U32    CPUCodeRateLimitersInterruptMask[32]/*reg*/;
            GT_U32    CPUCodeRateLimitersInterruptSummary;
            GT_U32    CPUCodeRateLimitersInterruptSummaryMask;

        }SCTRateLimitersInterrupt;

        struct /*preEgrInterrupt*/{

            GT_U32    preEgrInterruptSummary;
            GT_U32    preEgrInterruptSummaryMask;

        }preEgrInterrupt;

        struct /*preEgrEngineGlobalConfig*/{

            GT_U32    duplicationOfPktsToCPUConfig[32]/*port*/;
            GT_U32    MTUTableEntry[4]/*port*/;
            GT_U32    preEgrEngineGlobalConfig;
            GT_U32    MTUGlobalConfig;
            GT_U32    lossyDropConfig;
            GT_U32    cscdTrunkHashBitSelectionConfig;
            GT_U32    targetEPortMTUExceededCntr;
            GT_U32    incomingWrrArbiterWeights;
            GT_U32    dupFifoWrrArbiterWeights;

        }preEgrEngineGlobalConfig;

        struct /*mirrToAnalyzerPortConfigs*/{

            GT_U32    mirrorInterfaceParameterReg[7]/*Parameter Number*/;
            GT_U32    analyzerPortGlobalConfig;
            GT_U32    ingrStatisticMirrToAnalyzerPortConfig;    /* not used in sip 6 */
            GT_U32    ingrAndEgrMonitoringToAnalyzerQoSConfig;

            GT_U32    ingrStatisticMirrToAnalyzerPortRatioConfig[7]/*analyzer*/;  /* new in sip 6 */
            GT_U32    ingrStatisticMirrToAnalyzerPortEnableConfig[7]/*analyzer*/; /* new in sip 6 */
        }mirrToAnalyzerPortConfigs;

        struct /*L2ECMP*/{

            GT_U32    ePortECMPEPortValue;      /* not used in sip 6 */
            GT_U32    ePortECMPEPortMask;       /* not used in sip 6 */
            GT_U32    ePortECMPLTTIndexBaseEPort;
            GT_U32    ePortECMPLFSRConfig;
            GT_U32    ePortECMPLFSRSeed;
            GT_U32    ePortECMPHashBitSelectionConfig;
            GT_U32    trunkLFSRConfig;
            GT_U32    trunkLFSRSeed;
            GT_U32    trunkHashBitSelectionConfig;

            GT_U32    ePortECMPEPortMinimum; /* new in sip 6 */
            GT_U32    ePortECMPEPortMaximum; /* new in sip 6 */
        }L2ECMP;

        struct /*ingrSTCInterrupt*/{

            GT_U32    ingrSTCInterruptCause[32]/*Port*/;
            GT_U32    ingrSTCInterruptMask[32]/*Port*/;
            GT_U32    ingrSTCInterruptSummary;
            GT_U32    ingrSTCInterruptSummaryMask;
            GT_U32    ingrSTCInterruptSummary1;
            GT_U32    ingrSTCInterruptSummary1Mask;

        }ingrSTCInterrupt;/* not used in sip 6 */

        struct /*ingrSTCConfig*/{

            GT_U32    ingrSTCConfig;

        }ingrSTCConfig;/* not used in sip 6 */

        struct /*ingrForwardingRestrictions*/{

            GT_U32    ingrForwardingRestrictionsDroppedPktsCntr;
            GT_U32    ingrForwardingRestrictionsDropCode;

        }ingrForwardingRestrictions;

        struct /*ingrDropCntr*/{

            GT_U32    ingrDropCntrConfig;
            GT_U32    ingrDropCntr;

        }ingrDropCntr;

        struct /*ingrDropCodesCntr*/{

            GT_U32    dropCodesCntrConfig;
            GT_U32    dropCodesCntr;

        }ingrDropCodesCntr;

        struct /*ingrDropCodeLatching*/{

            GT_U32    ingrDropCodeLatching;
            GT_U32    lastDropCode;

        }ingrDropCodeLatching;/* new in sip 6 */

        struct /*ePort*/{

            GT_U32    ePortGlobalConfig;
            GT_U32    protectionLOCWriteMask;
            GT_U32    protectionSwitchingRXExceptionConfig;

        }ePort;

        struct /*criticalECCCntrs*/{

            GT_U32    criticalECCConfig;          /* internal in sip 6 */
            GT_U32    BMClearCriticalECCErrorCntr;
            GT_U32    BMClearCriticalECCErrorParams;
            GT_U32    BMIncCriticalECCErrorCntr;
            GT_U32    BMIncCriticalECCErrorParams;
            GT_U32    BMCriticalECCInterruptCause;
            GT_U32    BMCriticalECCInterruptMask;

        }criticalECCCntrs;

        struct /*CPUTargetDeviceConfig*/{

            GT_U32    CPUTargetDeviceConfigReg0;
            GT_U32    CPUTargetDeviceConfigReg1;
            GT_U32    CPUTargetDeviceConfigReg2;
            GT_U32    CPUTargetDeviceConfigReg3;

        }CPUTargetDeviceConfig;

        struct /*applicationSpecificCPUCodes*/{

            GT_U32    IPProtocolCPUCodeEntry[4]/*Protocol*/;
            GT_U32    TCPUDPDestPortRangeCPUCodeEntryWord0[16]/*Entry*/;
            GT_U32    TCPUDPDestPortRangeCPUCodeEntryWord1[16]/*Entry*/;
            GT_U32    IPProtocolCPUCodeValidConfig;

        }applicationSpecificCPUCodes;

    }EQ;

    struct /*LPM*/{
        GT_U32    globalConfig;
        GT_U32    directAccessMode;
        GT_U32    exceptionStatus;
        GT_U32    LPMInterruptsCause;
        GT_U32    LPMInterruptsMask;
        GT_U32    LPMGeneralIntCause;
        GT_U32    LPMGeneralIntMask;
    }LPM;

    struct /*TCAM*/{

        struct /*tcamInterrupts*/{

            GT_U32    TCAMInterruptsSummaryMask;
            GT_U32    TCAMInterruptsSummaryCause;
            GT_U32    TCAMInterruptMask;
            GT_U32    TCAMInterruptCause;
            GT_U32    tcamParityErrorAddr;

        }tcamInterrupts;

        struct /*tcamArrayConf*/{

            GT_U32    tcamActiveFloors;

        }tcamArrayConf;

        struct /*parityDaemonRegs*/{

            GT_U32    parityDaemonCtrl;
            GT_U32    parityDaemonWriteAdressStart;
            GT_U32    parityDaemonWriteAdressEnd;
            GT_U32    parityDaemonWriteAdressJump;
            GT_U32    parityDaemonReadAdressStart;
            GT_U32    parityDaemonReadAdressEnd;
            GT_U32    parityDaemonReadAdressJump;
            GT_U32    parityDaemonDefaultWriteDataX0;
            GT_U32    parityDaemonDefaultWriteDataX1;
            GT_U32    parityDaemonDefaultWriteDataY0;
            GT_U32    parityDaemonDefaultWriteDataX2;
            GT_U32    parityDaemonDefaultWriteDataY1;
            GT_U32    parityDaemonDefaultWriteDataY2;
            GT_U32    parityDaemonStatus;

        }parityDaemonRegs;

        struct /*mgAccess*/{

            GT_U32    mgKeySizeAndTypeGroup[5]/*Tcam Group Num*/;
            GT_U32    mgHitGroupHitNum[5][4]/*Tcam Group Num*//*Hit Number*/;
            GT_U32    mgKeyGroupChunk[5][21]/*Tcam Group Num*//*Chunk Of The Key*/;
            GT_U32    mgCompRequest;
            GT_U32    mgKeyTypeUpdateReq;
            GT_U32    keyTypeData;

        }mgAccess;

        GT_U32    groupClientEnable[5]          /*Tcam Group num*/;
        GT_U32    tcamHitNumAndGroupSelFloor[12]/*Floor num*/;
        GT_U32    tcamGlobalConfig;
        GT_U32    tcamAnswersFloorSample;

        struct /*exactMatchProfilesTable*/{

            GT_U32    exactMatchPortMapping[2]/*Pipe*/;
            GT_U32    exactMatchProfileTableControl[16]/*Entry*/;
            GT_U32    exactMatchProfileTableMaskData[16][12]/*Entry/Word*/;
            GT_U32    exactMatchProfileTableDefaultActionData[16][8]/*Entry/Word*/;

        }exactMatchProfilesTable;

        GT_U32    exactMatchActionAssignment[16][31]/*entryIndex/Byte*/;

    }TCAM;

    struct /*EGF_eft*/{

        struct /*MCFIFO*/{

            struct /*MCFIFOConfigs*/{

                GT_U32    portToMcFIFOAttributionReg[64]/*reg_offset*/;
                GT_U32    portToHemisphereMapReg[PORTS_BMP_NUM_CNS]/*reg offset*/;
                GT_U32    MCFIFOGlobalConfigs;
                GT_U32    MCFIFOArbiterWeights0;
                GT_U32    lossyDropAndPfcUcThreshold;
                GT_U32    priorityMulticast;

            }MCFIFOConfigs;

        }MCFIFO;

        struct /*global*/{

            GT_U32    cpuPortMapOfReg[128]/*reg_offset*/;
            GT_U32    cpuPortDist;
            GT_U32    portIsolationLookup0;
            GT_U32    portIsolationLookup1;
            GT_U32    clearedUCDist;       /* Cleared Unicast Distribution */
            GT_U32    EFTInterruptsCause;
            GT_U32    EFTInterruptsMask;
            GT_U32    ECCConf;
            GT_U32    ECCStatus;

            /* new in sip 6.0 */
            GT_U32    bypassEGFTargetEPortAttributes[PORTS_BMP_NUM_CNS]/*Source Port div 32*/;
        }global;

        struct /*egrFilterConfigs*/{

            GT_U32    enFCTriggerByCNFrameOnPort[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    enCNFrameTxOnPort[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    unregedBCFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    unknownUCFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    unregedMCFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    OAMLoopbackFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    virtual2PhysicalPortRemap[64]/*offset*/;
            GT_U32    physicalPortLinkStatusMask[PORTS_BMP_NUM_CNS]/*reg offset*/;
            GT_U32    egrFiltersGlobal;
            GT_U32    egrFiltersEnable;

        }egrFilterConfigs;

        struct /*deviceMapConfigs*/{

            GT_U32    devMapTableAddrConstructProfile[8]/*profile_num*/;
            GT_U32    localSrcPortMapOwnDevEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    localTrgPortMapOwnDevEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    portAddrConstructMode[256]/*local physical source port*/;

        }deviceMapConfigs;

        struct /*counters*/{

            GT_U32    eftDescEcc1ErrorCntr;

        }counters;

        struct /*egress MIB counters*/
        {
            GT_U32    egrMIBCntrsPortSetConfig[2]/*Set*/;
            GT_U32    egrMIBCntrsSetConfig[2]/*Set*/;
            GT_U32    egrMIBMcFIFODroppedPktsCntr[2]/*Set*/;
            GT_U32    egrMIBBridgeEgrFilteredPktCntr[2]/*Set*/;
            GT_U32    egrMIBMcFilteredPktCntr[2]/*Set*/;
        } egrMIBCntrs;/* sip6 only */

    }EGF_eft;

    struct /*EGF_qag*/{

        struct /*uniMultiArb*/{

            GT_U32    uniMultiCtrl;
            GT_U32    uniWeights;
            GT_U32    multiWeights;
            GT_U32    multiShaperCtrl;

        }uniMultiArb;

        struct /*global*/{

            GT_U32    QAGInterruptsCause;
            GT_U32    QAGInterruptsMask;
            GT_U32    QAGMetalFix;

        }global;

        struct /*distributor*/{

            GT_U32    useVlanTag1ForTagStateReg[PORTS_BMP_NUM_CNS]/*reg_offset*/;
            GT_U32    toAnalyzerLoopback[8]/*Reg Offset*/;
            GT_U32    TCRemap[32]/*remapped_tc*/;
            GT_U32    stackRemapEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    perTrgPhyPortLbProfile[16]/*Reg Offset*/;
            GT_U32    perSrcPhyPortLbProfile[16]/*Reg Offset*/;
            GT_U32    loopbackForwardingGlobalConfig;
            GT_U32    loopbackEnableConfig[4];
            GT_U32    evlanMirrEnable[PORTS_BMP_NUM_CNS]/*Reg Offset*/;
            GT_U32    DPRemap[16]/*remapped_dp*/;
            GT_U32    distributorGeneralConfigs;

            /* new in sip 6.0 */
            GT_U32    TCLoopbackConfig;
            GT_U32    eVIDXActivityStatusWriteMask;
            GT_U32    voQAssignmentGeneralConfig0;
            GT_U32    pktHashForVoQAssignment0;
            GT_U32    pktHashForVoQAssignment1;
            GT_U32    voQAssignmentGeneralConfig1;
            GT_U32    cutThroughConfig;

        }distributor;

    }EGF_qag;

    struct /*EGF_sht*/{

        struct /*global*/{

            GT_U32    eportStpState[PORTS_BMP_NUM_CNS]/*Port div 8*/;
            GT_U32    eportEvlanFilteringEnable[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    SHTInterruptsCause;
            GT_U32    UCSrcIDFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    SHTEgrFiltersEnable;
            GT_U32    MCLocalEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    ignorePhySrcMcFilterEn[PORTS_BMP_NUM_CNS]/*Reg Offset*/;
            GT_U32    eportVlanEgrFiltering;
            GT_U32    eportStpStateMode[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    eportOddOnlySrcIdFilterEnable[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    eportRoutedForwardRestricted[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    eportPortIsolationMode[2*PORTS_BMP_NUM_CNS]/*Port div 16*/;
            GT_U32    SHTGlobalConfigs;
            GT_U32    eportIPMCRoutedFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    eportFromCPUForwardRestricted[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    SHTInterruptsMask;
            GT_U32    eportBridgedForwardRestricted[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    meshIdConfigs;
            GT_U32    ePortMeshId[PORTS_NUM_CNS/8];
            GT_U32    dropOnEportVid1MismatchEn[PORTS_BMP_NUM_CNS]; /* port div 32 */
            GT_U32    eportAssociatedVid1[PORTS_NUM_CNS/2];             /* port div 2 */

        }global;

    }EGF_sht;


    struct /*HA*/{

        GT_U32    invalidPTPHeaderCntr;
REGISTER_COVERED_CNS        GT_U32    TRILLEtherType;
REGISTER_COVERED_CNS        GT_U32    TPIDConfigReg[8]/*entry*/;
REGISTER_COVERED_CNS        GT_U32    routerMACSABaseReg1;
        GT_U32    PWCtrlWord[15]/*PW word*/;
        GT_U32    PWETreeEnable;
        GT_U32    invalidPTPOutgoingPiggybackCntr;
        GT_U32    PTPTimestampTagEthertype;
        GT_U32    PTPAndTimestampingExceptionConfig;
REGISTER_COVERED_CNS        GT_U32    MPLSEtherType;
        GT_U32    MPLSChannelTypeProfileReg7;
        GT_U32    MPLSChannelTypeProfileReg[7]/*Reg*/;
REGISTER_COVERED_CNS        GT_U32    IPLengthOffset;
REGISTER_COVERED_CNS        GT_U32    egrUDPPortRangeComparatorConfig[8]/*UDPCmp*/;
        GT_U32    invalidPTPIncomingPiggybackCntr;
        GT_U32    egrPolicyDIPSolicitationData1;
REGISTER_COVERED_CNS        GT_U32    ingrAnalyzerVLANTagConfig;
REGISTER_COVERED_CNS        GT_U32    IEtherType;
        GT_U32    haGlobalConfig1;
REGISTER_COVERED_CNS        GT_U32    haGlobalConfig;
        GT_U32    HAInterruptMask;
        GT_U32    HAInterruptCause;
        GT_U32    ethernetOverGREProtocolType;
REGISTER_COVERED_CNS        GT_U32    routerMACSABaseReg0;
REGISTER_COVERED_CNS        GT_U32    egrTCPPortRangeComparatorConfig[8]/*TCPCmp*/;
        GT_U32    egrPolicyDIPSolicitationMask3;
        GT_U32    egrPolicyDIPSolicitationMask2;
        GT_U32    egrPolicyDIPSolicitationMask1;
        GT_U32    egrPolicyDIPSolicitationMask0;
        GT_U32    egrPolicyDIPSolicitationData3;
        GT_U32    egrPolicyDIPSolicitationData2;
        GT_U32    tunnelStartFragmentIDConfig;
        GT_U32    egrPolicyDIPSolicitationData0;
REGISTER_COVERED_CNS        GT_U32    egrAnalyzerVLANTagConfig;
        GT_U32    dataECCStatus;
        GT_U32    CPIDReg1;
        GT_U32    CPIDReg0;
        GT_U32    congestionNotificationConfig;
        GT_U32    CNMHeaderConfig;
        GT_U32    BPEConfigReg2;
        GT_U32    BPEConfigReg1;
        GT_U32    haBadAddrLatchReg;/*manual added for Sip5_10 : Latch bad address in case of regfile bad address indication.*/
        GT_U32    NATConfig;        /*manual added for Sip5_10*/
        GT_U32    NATExceptionDropCounter;/*manual added for Sip5_10*/
        GT_U32    haGlobalConfig2;/*manual added for Sip5_25*/

        CPSS_TBD_BOOKMARK
        /* next manually added to stop us the
            PRV_CPSS_DXCH_PP_REGS_ADDR_STC::haRegs */
        struct ETS_temp_STCT{
REGISTER_COVERED_CNS            GT_U32  ptpGlobalConfig; /* Egress PTP Global Register */
REGISTER_COVERED_CNS            GT_U32  ptpEnableTimeStamp; /* Enable Time Stamping Register */
REGISTER_COVERED_CNS            GT_U32  ptpGlobalFifoCurEntry[2]; /* Global FIFO Current Entry */
REGISTER_COVERED_CNS            GT_U32  ptpTodCntrNanoSeconds; /* Egress TOD Counter - Nanoseconds */
REGISTER_COVERED_CNS            GT_U32  ptpTodCntrSeconds[2]; /* Egress TOD Counter - Seconds */
REGISTER_COVERED_CNS            GT_U32  ptpTodCntrShadowNanoSeconds; /* Egress TOD Counter Shadow - Nanoseconds */
REGISTER_COVERED_CNS            GT_U32  ptpTodCntrShadowSeconds[2]; /* Egress TOD Counter Shadow - Seconds */
        }ETS_temp;

        /* SIP6 Registers */
        GT_U32    PTPExceptionCPUCodeConfig;
        GT_U32    QCNModifications;
    }HA;

    struct /*MLL*/{

        struct /*multiTargetRateShape*/{

            GT_U32    multiTargetRateShapeConfig;
            GT_U32    multiTargetRateShapeConfig2;
            GT_U32    multiTargetMTUReg;
            GT_U32    tokenBucketBaseLine;

        }multiTargetRateShape;

        struct /*mcLinkedListMLLTables*/{

REGISTER_COVERED_CNS            GT_U32    qoSProfileToMultiTargetTCQueuesReg[64]/*Index*/;
            GT_U32    extQoSModeMCQueueSelectionReg[2]/*Index*/;

        }mcLinkedListMLLTables;

        struct /*multiTargetVsUcSDWRRAndStrictPriorityScheduler*/{

REGISTER_COVERED_CNS            GT_U32    mcUcSDWRRAndStrictPriorityConfig;

        }multiTargetVsUcSDWRRAndStrictPriorityScheduler;

        struct /*multiTargetTCQueuesAndArbitrationConfig*/{

REGISTER_COVERED_CNS            GT_U32    multitargetTCQueuesGlobalConfig;
REGISTER_COVERED_CNS            GT_U32    multitargetTCQueuesStrictPriorityEnableConfig; /* not used in sip 6 */
REGISTER_COVERED_CNS            GT_U32    multitargetTCQueuesWeightConfig;

            GT_U32    multitargetTCQueuesPriorityConfig;/* new in sip 6 */

        }multiTargetTCQueuesAndArbitrationConfig;

        struct /*multiTargetEPortMap*/{

            GT_U32    multiTargetEPortValue;  /* not used in sip 6 */
            GT_U32    multiTargetEPortMask;   /* not used in sip 6 */
            GT_U32    multiTargetEPortBase;
            GT_U32    ePortToEVIDXBase;

            GT_U32    multiTargetGlobalEPortMinValue;/* new in sip 6 */
            GT_U32    multiTargetGlobalEPortMaxValue;/* new in sip 6 */

        }multiTargetEPortMap;

        struct /*MLLOutInterfaceCntrs*/{

            GT_U32    L2MLLOutInterfaceCntrConfig1[2]/*Counter*/;
            GT_U32    L2MLLOutInterfaceCntrConfig[2]/*Counter*/;
REGISTER_COVERED_CNS            GT_U32    L2MLLValidProcessedEntriesCntr[3]/*Counter*/;
REGISTER_COVERED_CNS            GT_U32    L2MLLTTLExceptionCntr;
REGISTER_COVERED_CNS            GT_U32    L2MLLSkippedEntriesCntr;
            GT_U32    L2MLLSilentDropCntr;
            GT_U32    L2MLLOutMcPktsCntr[2]/*Counter*/;
            GT_U32    MLLSilentDropCntr;
REGISTER_COVERED_CNS            GT_U32    MLLMCFIFODropCntr;
            GT_U32    IPMLLSkippedEntriesCntr;
REGISTER_COVERED_CNS            GT_U32    IPMLLOutMcPktsCntr[2]/*Counter*/;
REGISTER_COVERED_CNS            GT_U32    IPMLLOutInterfaceCntrConfig1[2]/*Counter*/;
REGISTER_COVERED_CNS            GT_U32    IPMLLOutInterfaceCntrConfig[2]/*Counter*/;

        }MLLOutInterfaceCntrs;

        struct /*MLLGlobalCtrl*/{

REGISTER_COVERED_CNS            GT_U32    MLLGlobalCtrl;
REGISTER_COVERED_CNS            GT_U32    MLLLookupTriggerConfig;
REGISTER_COVERED_CNS            GT_U32    globalTTLExceptionConfig;
REGISTER_COVERED_CNS            GT_U32    globalEPortRangeConfig; /* not used in sip 6 */
REGISTER_COVERED_CNS            GT_U32    globalEPortRangeMask;   /* not used in sip 6 */
            GT_U32    IPMLLTableResourceSharingConfig;
            GT_U32    L2MLLTableResourceSharingConfig;
            GT_U32    MLLInterruptCauseReg;
            GT_U32    MLLInterruptMaskReg;
            GT_U32    MLLReplicatedTrafficCtrl;
            GT_U32    sourceBasedL2MLLFiltering;
            GT_U32    MLLEccStatus;
REGISTER_COVERED_CNS            GT_U32    MLLMCSourceID;

            GT_U32    L2mllGlobalEPortMinValue; /* new in sip 6 */
            GT_U32    L2mllGlobalEPortMaxValue; /* new in sip 6 */
        }MLLGlobalCtrl;

    }MLL;

    struct /*PCL*/{

        struct /*OAMConfigs*/{

            GT_U32    channelTypeOAMOpcode[15]/*channel type*/;
REGISTER_COVERED_CNS            GT_U32    OAMRDIConfigs;
            GT_U32    OAMMEGLevelModeConfigs;
            GT_U32    OAMRFlagConfigs;
            GT_U32    RBitAssignmentEnable;

        }OAMConfigs;

        struct /*HASH*/{

REGISTER_COVERED_CNS            GT_U32    pearsonHashTable[16]/*index div 4*/;
REGISTER_COVERED_CNS            GT_U32    CRCHashConfig;
REGISTER_COVERED_CNS            GT_U32    CRC32Salt[20]/*n*/;
REGISTER_COVERED_CNS            GT_U32    CRC32Seed;

        }HASH;

REGISTER_COVERED_CNS        GT_U32    ingrPolicyGlobalConfig;
        GT_U32    PCLUnitInterruptCause;
        GT_U32    PCLUnitInterruptMask;
        GT_U32    pclBadAddressLatchReg;
REGISTER_COVERED_CNS        GT_U32    policyEngineConfig;
REGISTER_COVERED_CNS        GT_U32    policyEngineUserDefinedBytesConfig;
REGISTER_COVERED_CNS        GT_U32    CFMEtherType;
        GT_U32    lookupSourceIDMask[3]/*lookup num*/;
REGISTER_COVERED_CNS        GT_U32    policyEngineUserDefinedBytesExtConfig;
        GT_U32    TCAMKeyTypeMap[2]/*reg num*/;
REGISTER_COVERED_CNS        GT_U32    L2L3IngrVLANCountingEnable[32]/*port div 32*/;
REGISTER_COVERED_CNS        GT_U32    countingModeConfig;

        /* SIP6 Registers */
REGISTER_COVERED_CNS        GT_U32    CRCHashModes;
REGISTER_COVERED_CNS        GT_U32    exactMatchProfileIdMapEntry[128]/*entry num*/;

    }PCL;

    struct /*EPCL*/{

REGISTER_COVERED_CNS        GT_U32    egrPolicyGlobalConfig;
        GT_U32    EPCLOAMPktDetection;
        GT_U32    EPCLOAMMEGLevelMode;
REGISTER_COVERED_CNS        GT_U32    EPCLOAMRDI;
        GT_U32    EPCLInterruptCause;
        GT_U32    EPCLInterruptMask;
        GT_U32    EPCLChannelTypeToOAMOpcodeMap[5]/*Set*/;
        GT_U32    EPCLReservedBitMask;
        GT_U32    TCAMKeyTypeMap;

        GT_U32    EPCLBadAddrLatchReg;
        GT_U32    oamRBbitConfigReg;
        GT_U32    EPCLFifoConfiguration;

        /* SIP6 Registers */
        GT_U32    PHAMetadataMask1;
        GT_U32    PHAMetadataMask2;
        GT_U32    PHAMetadataMask3;
        GT_U32    PHAMetadataMask4;
        GT_U32    PHAThreadNumMask;
        GT_U32    latencyMonitoringSamplingConfig[8];
        GT_U32    latencyMonitoringProfile2SamplingProfile[64];
        GT_U32    latencyMonitoringControl;
    }EPCL;

    struct /*rxDMA*/{

        struct /*singleChannelDMAConfigs*/{

            GT_U32    SCDMAConfig0[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
            GT_U32    SCDMAConfig1[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
            GT_U32    SCDMACTPktIndentification[PRV_CPSS_MAX_DMA_NUM_CNS]/*port*/;

        }singleChannelDMAConfigs;

        struct /*globalRxDMAConfigs*/{

            struct /*rejectConfigs*/{

                GT_U32    rejectPktCommand;
                GT_U32    rejectCPUCode;

            }rejectConfigs;

            struct /*globalConfigCutThrough*/{

                GT_U32    CTEtherTypeIdentification;
                GT_U32    CTUPEnable;
                GT_U32    CTGeneralConfiguration;

            }globalConfigCutThrough;

            struct /*buffersLimitProfiles*/{

                GT_U32    portsBuffersLimitProfileXONXOFFConfig[8];
                GT_U32    portsBuffersLimitProfileRxlimitConfig[8];
            }buffersLimitProfiles;

            struct /*preIngrPrioritizationConfStatus*/{

                GT_U32    VLANUPPriorityArrayProfile[4]/*profile*/;
                GT_U32    MPLSEtherTypeConf[2]/*MPLS*/;
                GT_U32    VIDConf[4]/*index*/;
                GT_U32    genericEtherType[4]/*index*/;
                GT_U32    qoSDSAPriorityArrayProfileSegment[4][8]/*profile*//*segment*/;
                GT_U32    preingressPrioritizationEnable;
                GT_U32    PIPPriorityDropGlobalCntrsClear;
                GT_U32    PIPVeryHighPriorityDropGlobalCntrLsb;
                GT_U32    PIPVeryHighPriorityDropGlobalCntrMsb;
                GT_U32    PIPPrioThresholds;
                GT_U32    PIPMedPriorityDropGlobalCntrMsb;
                GT_U32    PIPMedPriorityDropGlobalCntrLsb;
                GT_U32    PIPLowPriorityDropGlobalCntrMsb;
                GT_U32    VLANEtherTypeConf[4]/*index*/;
                GT_U32    MACDA2MsbBytesConf[4]/*MAC DA conf*/;
                GT_U32    PIPHighPriorityDropGlobalCntrLsb;
                GT_U32    MPLSEXPPriorityArrayProfile[4]/*profile*/;
                GT_U32    UPDSATagArrayProfile[4]/*profile*/;
                GT_U32    MACDAPriorityArrayProfile[4]/*profile*/;
                GT_U32    MACDAMask4LsbConf[4]/*mask*/;
                GT_U32    MACDAMask2MsbConf[4]/*mask*/;
                GT_U32    MACDA4LsbBytesConf[4]/*MAC DA conf*/;
                GT_U32    PIPLowPriorityDropGlobalCntrLSb;
                GT_U32    IPv6TCPriorityArrayProfileSegment[4][16]/*profile*//*segment*/;
                GT_U32    IPv6EtherTypeConf;
                GT_U32    IPv4ToSPriorityArrayProfileSegment[4][16]/*profile*//*segment*/;
                GT_U32    IPv4EtherTypeConf;
                GT_U32    PIPHighPriorityDropGlobalCntrMsb;
                GT_U32    PIPDropGlobalCounterTrigger;
                GT_U32    etherTypePriorityArrayProfile[4]/*profile*/;

                GT_U32    profileTPID[4]/*Profile*/;   /* new in sip6*/
                GT_U32    OSTPIDsConfReg[4]/*Profile*/;/* new in sip6*/
                GT_U32    latencySensitiveEtherTypeConf[4]/*index*/;/* new in sip6*/
                GT_U32    channelPIPConfigReg[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;/* new in sip6 - instead of SCDMA_n_PIP_Config */
            }preIngrPrioritizationConfStatus;

        }globalRxDMAConfigs;

        struct /* SCDMA Debug*/{
            GT_U32 portBuffAllocCnt[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
        }scdmaDebug;

        struct /*SIP_COMMON_MODULES*/{
            PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }SIP_COMMON_MODULES;

        struct /*interrupts*/{

            GT_U32    rxDMASCDMAInterruptMask [PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
            GT_U32    rxDMASCDMAInterruptCause[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
            GT_U32    rxDMAInterrupt0Cause;
            GT_U32    rxDMAInterrupt0Mask;
            GT_U32    rxDMAInterruptSummary0Cause;
            GT_U32    rxDMAInterruptSummary0Mask;
            GT_U32    rxDMAInterruptSummary1Cause;
            GT_U32    rxDMAInterruptSummary1Mask;
            GT_U32    rxDMAInterruptSummary2Cause;
            GT_U32    rxDMAInterruptSummary2Mask;
            GT_U32    rxDMAInterruptSummary3Cause;
            GT_U32    rxDMAInterruptSummary3Mask;
            GT_U32    rxDMAInterruptSummaryCause;
            GT_U32    rxDMAInterruptSummaryMask;

        }interrupts;

        struct /* debug*/{

            struct /* counters_and_counters_statuses */{

                GT_U32    currentNumOfPacketsStoredInMppm;

            }countersAndCountersStatuses;

        }debug;

        struct /*singleChannelDMAPip*/ {
            GT_U32    SCDMA_n_PIP_Config      [PRV_CPSS_MAX_DMA_NUM_CNS];
            GT_U32    SCDMA_n_Drop_Pkt_Counter[PRV_CPSS_MAX_DMA_NUM_CNS];
        }singleChannelDMAPip;

    }rxDMA[MAX_DP_CNS];

    struct /*ingressAggregator*/ {
        GT_U32  PIPPrioThresholds0;
        GT_U32  PIPPrioThresholds1;
        GT_U32  PIPEnable;
    }ingressAggregator[2];

    struct /*txDMA*/{

        struct /*txDMAThresholdsConfigs*/{

            GT_U32    RXDMAUpdatesFIFOsThresholds[1]/*rxdma*/;
            GT_U32    clearFIFOThreshold;
            GT_U32    headerReorderFifoThreshold;
            GT_U32    payloadReorderFifoThreshold;

        }txDMAThresholdsConfigs;

        struct /*txDMAPerSCDMAConfigs*/{

            GT_U32    burstLimiterSCDMA              [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    FIFOsThresholdsConfigsSCDMAReg1[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    txFIFOCntrsConfigsSCDMA        [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs                   [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs1                  [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs2                  [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA reg2*/;
            GT_U32    SCDMAMetalFixReg               [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMARateLimitResidueVectorThr [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
        }txDMAPerSCDMAConfigs;

        struct /*txDMAInterrupts*/{

            GT_U32    SCDMAIDFIFOOverrunInterruptCauseReg1;
            GT_U32    SCDMAIDFIFOOverrunInterruptMaskReg1;
            GT_U32    SCDMAIDFIFOOverrunInterruptCauseReg2;
            GT_U32    SCDMAIDFIFOOverrunInterruptMaskReg2;
            GT_U32    SCDMAWaitingForNextReplyInterruptMaskReg3;
            GT_U32    SCDMAIDFIFOOverrunInterruptMaskReg3;
            GT_U32    RXDMAUpdatesFIFOsOverrunInterruptCauseReg1;
            GT_U32    RXDMAUpdatesFIFOsOverrunInterruptMaskReg1;
            GT_U32    SCDMAWaitingForNextReplyInterruptCauseReg1;
            GT_U32    SCDMAWaitingForNextReplyInterruptMaskReg1;
            GT_U32    SCDMAWaitingForNextReplyInterruptCauseReg2;
            GT_U32    SCDMAWaitingForNextReplyInterruptMaskReg2;
            GT_U32    SCDMAWaitingForNextReplyInterruptCauseReg3;
            GT_U32    SCDMAIDFIFOOverrunInterruptCauseReg3;
            GT_U32    txDMAGeneralCauseReg1;
            GT_U32    txDMAGeneralMaskReg1;
            GT_U32    txDMASummaryCauseReg;
            GT_U32    txDMASummaryMaskReg;

        }txDMAInterrupts;

        struct /*txDMAGlobalConfigs*/{

            GT_U32    txDMAGlobalConfig1;
            GT_U32    txDMAGlobalConfig3;
            GT_U32    ingrCoreIDToMPPMMap;
            GT_U32    TXDMAMetalFixReg;
            GT_U32    srcSCDMASpeedTbl[10];
            GT_U32    localDevSrcPort2DMANumberMap[128];
        }txDMAGlobalConfigs;

        struct /*txDMADescCriticalFieldsECCConfigs*/{

            GT_U32    dataECCConfig;
            GT_U32    dataECCStatus;

        }txDMADescCriticalFieldsECCConfigs;

        struct /*txDMADebug*/{

            struct /*informativeDebug*/{

                GT_U32    nextBCFIFOFillLevel[1]/*core*/;
                GT_U32    nextBCFIFOMaxFillLevel[1]/*core*/;
                GT_U32    SCDMAStatusReg1          [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg2          [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAPrefFIFOFillLevel   [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAPrefFIFOMaxFillLevel[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    descOverrunReg1;
                GT_U32    descOverrunReg2;
                GT_U32    descOverrunReg3;
                GT_U32    clearFIFOFillLevel;
                GT_U32    clearFIFOMaxFillLevel;
                GT_U32    headerReorderFIFOFillLevel;
                GT_U32    headerReorderFIFOMaxFillLevel;
                GT_U32    payloadReorderFIFOFillLevel;
                GT_U32    payloadReorderFIFOMaxFillLevel;
                GT_U32    descIDFIFOFillLevel;
                GT_U32    descIDFIFOMaxFillLevel;
                GT_U32    debugBusCtrlReg;
                GT_U32    txDMADebugBus;
                GT_U32    incomingDescsCntrLsb;
                GT_U32    incomingDescsCntrMsb;
                GT_U32    outgoingDescsCntrLsb;
                GT_U32    outgoingDescsCntrMsb;
                GT_U32    outgoingClearRequestsCntrLsb;
                GT_U32    outgoingClearRequestsCntrMsb;
                GT_U32    MPPMReadRequestsCntr;
                GT_U32    MPPMReadAnswersCntrLsb;
                GT_U32    MPPMReadAnswersCntrMsb;
                GT_U32    outgoingPayloadsCntrLsb;
                GT_U32    outgoingPayloadsCntrMsb;
                GT_U32    outgoingPayloadsEOPCntrLsb;
                GT_U32    outgoingPayloadsEOPCntrMsb;
                GT_U32    outgoingHeadersEOPCntrLsb;
                GT_U32    outgoingHeadersEOPCntrMsb;
                GT_U32    outgoingCTBCUpdatesCntr;
                GT_U32    HADescIDDBFreeIDFIFOFillLevel;
                GT_U32    internalDescIDDBFreeIDFIFOFillLevel;
                GT_U32    headerReorderMemIDDBFreeIDFIFOFillLevel;
                GT_U32    payloadReorderMemIDDBFreeIDFIFOFillLevel;

            }informativeDebug;

        }txDMADebug;

        struct /*txDMAPizzaArb*/{
            PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }txDMAPizzaArb;

    }txDMA[MAX_DP_CNS];

    struct /*txFIFO*/{

        struct /*txFIFOShiftersConfig*/{

            GT_U32    SCDMAShiftersConf[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;

        }txFIFOShiftersConfig;

        struct /*txFIFOLinkedLists*/{

            struct /*payloadLinkedList*/{

                struct /*payloadLinkListRAM*/{

                    GT_U32    payloadLinkListAddr;

                }payloadLinkListRAM;

            }payloadLinkedList;

            struct /*headerLinkedList*/{

                struct /*headerLinkListRAM*/{

                    GT_U32    headerLinkListAddr;

                }headerLinkListRAM;

            }headerLinkedList;

        }txFIFOLinkedLists;

        struct /*txFIFOInterrupts*/{

            GT_U32    txFIFOGeneralCauseReg1;
            GT_U32    txFIFOGeneralMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg3;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg3;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg1;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg1;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg2;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg2;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg3;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg3;
            GT_U32    txFIFOSummaryCauseReg;
            GT_U32    txFIFOSummaryMaskReg;

        }txFIFOInterrupts;

        struct /*txFIFOGlobalConfig*/{

            GT_U32    SCDMAPayloadThreshold[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    txFIFOMetalFixReg;

        }txFIFOGlobalConfig;

        struct /*txFIFOEnd2EndECCConfigs*/{

            GT_U32    dataECCConfig[2]/*ecc*/;
            GT_U32    dataECCStatus[2]/*ecc*/;

        }txFIFOEnd2EndECCConfigs;

        struct /*txFIFODebug*/{

            struct /*informativeDebug*/{

                GT_U32    SCDMAStatusReg6[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg1[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg2[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg3[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg4[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg5[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    debugBusCtrlReg;
                GT_U32    txFIFODebugBus;
                GT_U32    LLEOPCntrLsb;
                GT_U32    LLEOPCntrMsb;
                GT_U32    incomingHeadersCntrLsb;
                GT_U32    incomingHeadersCntrMsb;
                GT_U32    headerIDDBFreeIDFIFOFillLevel;
                GT_U32    payloadIDDBFreeIDFIFOFillLevel;
                GT_U32    CTBCIDDBFreeIDFIFOFillLevel;

            }informativeDebug;

        }txFIFODebug;

        struct /*txFIFOPizzaArb*/{
            PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }txFIFOPizzaArb;

    }txFIFO[MAX_DP_CNS];

    struct /*SIP_ETH_TXFIFO*/{

        struct /*txFIFOShiftersConfig*/{

            GT_U32    SCDMAShiftersConf[73]/*scdma*/;

        }txFIFOShiftersConfig;

        struct /*txFIFOPizzaArb*/{
            PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }txFIFOPizzaArb;

        struct /*txFIFOLinkedLists*/{

            struct /*payloadLinkedList*/{

                struct /*payloadLinkListRAM*/{

                    GT_U32    payloadLinkListAddr;

                }payloadLinkListRAM;

            }payloadLinkedList;

            struct /*headerLinkedList*/{

                struct /*headerLinkListRAM*/{

                    GT_U32    headerLinkListAddr;

                }headerLinkListRAM;

            }headerLinkedList;

        }txFIFOLinkedLists;

        struct /*txFIFOInterrupts*/{

            GT_U32    txFIFOGeneralCauseReg1;
            GT_U32    txFIFOGeneralMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg3;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg3;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg1;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg1;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg2;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg2;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg3;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg3;
            GT_U32    txFIFOSummaryCauseReg;
            GT_U32    txFIFOSummaryMaskReg;

        }txFIFOInterrupts;

        struct /*txFIFOGlobalConfig*/{

            GT_U32    SCDMAPayloadThreshold[73]/*scdma*/;
            GT_U32    txFIFOGlobalConfigs;
            GT_U32    TXFIFOMetalFixReg;

        }txFIFOGlobalConfig;

        struct /*txFIFOEnd2EndECCConfigs*/{

            GT_U32    dataECCStatus[1]/*ecc*/;
            GT_U32    dataECCConfig[1]/*ecc*/;
            GT_U32    eccErrorCntr;
            GT_U32    ECCFailedAddr;
            GT_U32    eccErrorType;
            GT_U32    lockErronousEvent;

        }txFIFOEnd2EndECCConfigs;

        struct /*txFIFODebug*/{

            struct /*informativeDebug*/{

                GT_U32    SCDMAStatusReg6[73]/*scdma*/;
                GT_U32    SCDMAStatusReg1[73]/*scdma*/;
                GT_U32    SCDMAStatusReg5[73]/*scdma*/;
                GT_U32    SCDMAStatusReg4[73]/*scdma*/;
                GT_U32    SCDMAStatusReg3[73]/*scdma*/;
                GT_U32    SCDMAStatusReg2[73]/*scdma*/;
                GT_U32    debugBusCtrlReg;
                GT_U32    txFIFODebugBus;
                GT_U32    LLEOPCntr;
                GT_U32    incomingHeadersCntr;
                GT_U32    headerIDDBFreeIDFIFOFillLevel;
                GT_U32    payloadIDDBFreeIDFIFOFillLevel;
                GT_U32    CTBCIDDBFreeIDFIFOFillLevel;

            }informativeDebug;

            struct /*activeDebug*/{

                GT_U32    debugSCDMA[73]/*scdma*/;
                GT_U32    globalActiveDebugReg;
                GT_U32    globalActiveDebugConfReg;
                GT_U32    debugSCDMAConfReg;

            }activeDebug;

        }txFIFODebug;

    }SIP_ETH_TXFIFO[2];

    struct /*SIP_ILKN_TXFIFO*/{

        struct /*ilknTxFIFOInterrupts*/{

            GT_U32    ilknTxfifoGeneralInterruptCause;
            GT_U32    ilknTxfifoGeneralInterruptMask;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptCauseReg1;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptMaskReg1;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptCauseReg1;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptMaskReg1;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptCauseReg2;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptMaskReg2;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptCauseReg2;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptMaskReg2;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptCauseReg3;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptMaskReg3;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptCauseReg3;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptMaskReg3;
            GT_U32    ilknTxfifoUnderrunInterruptCauseReg1;
            GT_U32    ilknTxfifoUnderrunInterruptMaskReg1;
            GT_U32    ilknTxfifoUnderrunInterruptCauseReg2;
            GT_U32    ilknTxfifoUnderrunInterruptMaskReg2;
            GT_U32    ilknTxfifoUnderrunInterruptCauseReg3;
            GT_U32    ilknTxfifoUnderrunInterruptMaskReg3;
            GT_U32    ilknTxfifoSummaryCauseReg;
            GT_U32    ilknTxfifoSummaryMaskReg;

        }ilknTxFIFOInterrupts;

        struct /*ilknTxFIFOGlobalConfig*/{

            GT_U32    portDynamicMemoryAlocation[64]/*port_num*/;
            GT_U32    portTransmittingThreshold[64]/*port num*/;
            GT_U32    portChannelEnable[64]/*port_num*/;
            GT_U32    globalConfig;

        }ilknTxFIFOGlobalConfig;

        struct /*ilknTxFIFODebug*/{

            GT_U32    dataFifoFillLevelForPort[64]/*port_num*/;

        }ilknTxFIFODebug;

        struct /*ilknTxFIFOArbiter*/{

            GT_U32    pizzaSlicePortMap[1064]/*slice num*/;
            GT_U32    pizzaArbiterCtrlReg;
            GT_U32    pizzaArbiterStatusReg;

        }ilknTxFIFOArbiter;

    }SIP_ILKN_TXFIFO;

    struct /*ERMRK*/{

        GT_U32    ERMRKGlobalConfig;
        GT_U32    ERMRKInterruptCause;
        GT_U32    ERMRKInterruptMask;
        GT_U32    ERMRKPTPConfig;
        GT_U32    timestampingUDPChecksumMode;
        GT_U32    timestampQueueMsgType;
        GT_U32    timestampQueueEntryID;
        GT_U32    PTPNTPOffset;
        GT_U32    ERMRKUP2UPMapTable[12]/*set*/;
        GT_U32    ERMRKPTPInvalidTimestampCntr;
        GT_U32    ERMRKDP2CFITable;
        GT_U32    ERMRKEXP2EXPMapTable[12]/*set*/;
        GT_U32    timestampQueueEntryWord0[4]/*queue*/;
        GT_U32    timestampQueueEntryWord1[4]/*queue*/;
        GT_U32    timestampQueueEntryWord2[2]/*queue*/;
        GT_U32    timestampQueueEntryIDClear;
        GT_U32    ECCErrorInjection;
        GT_U32    ERMRKBadAddrLatch;

    }ERMRK;

    struct /*BM*/{

        struct /*BMInterrupts*/{

            GT_U32    BMGeneralCauseReg1;
            GT_U32    BMGeneralMaskReg1;
            GT_U32    BMGeneralCauseReg2;
            GT_U32    BMGeneralMaskReg2;
            GT_U32    BMSummaryCauseReg;
            GT_U32    BMSummaryMaskReg;

        }BMInterrupts;

        struct /*BMGlobalConfigs*/{

            GT_U32    BMBufferTermination[5]/*reg*/;
            GT_U32    BMGlobalConfig1;
            GT_U32    BMGlobalConfig2;
            GT_U32    BMGlobalConfig3;
            GT_U32    BMPauseConfig;
            GT_U32    BMBufferLimitConfig1;
            GT_U32    BMBufferLimitConfig2;
            GT_U32    BMMetalFixReg;

        }BMGlobalConfigs;

        struct /*BMDebug*/{

            GT_U32    rxDMANextUpdatesFIFOMaxFillLevel[6]/*core*/;
            GT_U32    rxDMANextUpdatesFIFOFillLevel[6]/*core*/;
            GT_U32    SCDMAInProgRegs[148]/*scdma*/;/*148 in bobk , 114 in bc2*/

        }BMDebug;

        struct /*BMCntrs*/{

            GT_U32    BMGlobalBufferCntr;
            GT_U32    rxDMAsAllocationsCntr;
            GT_U32    BMFreeBuffersInsideBMCntr;
            GT_U32    BMRxDMAPendingAllocationCntr;
            GT_U32    BMCTClearsInsideBMCntr;
            GT_U32    incomingCTClearsCntr;
            GT_U32    incomingRegularClearsCntr;
            GT_U32    nextTableAccessCntr;
            GT_U32    nextTableWriteAccessCntr;
            GT_U32    BMTerminatedBuffersCntr;
            GT_U32    BMLostClearRequestsCntr;

        }BMCntrs;

    }BM;

    struct /*OAMUnit*/{

        GT_U32    OAMGlobalCtrl;
        GT_U32    agingEntryGroupStatus[2]/*reg*/;
        GT_U32    summaryExceptionGroupStatus[2]/*reg*/;
        GT_U32    txPeriodExceptionGroupStatus[2]/*reg*/;
        GT_U32    sourceInterfaceMismatchCntr;
        GT_U32    OAMGlobalCtrl1;
        GT_U32    RDIStatusChangeExceptionGroupStatus[2]/*reg*/;
        GT_U32    RDIStatusChangeCntr;
        GT_U32    OAMUnitInterruptMask;
        GT_U32    OAMUnitInterruptCause;
        GT_U32    OAMTableAccessData[4]/*Reg*/;
        GT_U32    OAMTableAccessCtrl;
        GT_U32    OAMLossMeasurementOpcodes[4]/*reg*/;
        GT_U32    OAMLMCountedOpcodes[2]/*reg*/;
        GT_U32    OAMKeepAliveOpcodes[4]/*reg*/;
        GT_U32    OAMInvalidKeepaliveExceptionConfigs;
        GT_U32    agingPeriodEntryHigh[8]/*entry*/;
        GT_U32    dualEndedLM;
        GT_U32    OAMExceptionConfigs2;
        GT_U32    OAMExceptionConfigs1;
        GT_U32    OAMExceptionConfigs;
        GT_U32    OAMDelayMeasurementOpcodes[4]/*reg*/;
        GT_U32    MEGLevelGroupStatus[2]/*reg*/;
        GT_U32    MEGLevelExceptionCntr;
        GT_U32    sourceInterfaceGroupStatus[2]/*reg*/;
        GT_U32    invalidKeepaliveHashCntr;
        GT_U32    invalidKeepaliveGroupStatus[2]/*reg*/;
        GT_U32    hashBitsSelection;
        GT_U32    excessKeepaliveGroupStatus[2]/*reg*/;
        GT_U32    excessKeepaliveCntr;
        GT_U32    keepaliveAgingCntr;
        GT_U32    agingPeriodEntryLow[8]/*entry*/;
        GT_U32    txPeriodExceptionCntr;
        GT_U32    summaryExceptionCntr;

    }OAMUnit[2];

    struct /*GOP*/{

        struct {

            GT_U32    TAIInterruptCause;
            GT_U32    TAIInterruptMask;
            GT_U32    TAICtrlReg0;
            GT_U32    TAICtrlReg1;
            GT_U32    timeCntrFunctionConfig0;
            GT_U32    timeCntrFunctionConfig1;
            GT_U32    timeCntrFunctionConfig2;
            GT_U32    frequencyAdjustTimeWindow;
            GT_U32    TODStepNanoConfigHigh;
            GT_U32    TODStepNanoConfigLow;
            GT_U32    TODStepFracConfigHigh;
            GT_U32    TODStepFracConfigLow;
            GT_U32    timeAdjustmentPropagationDelayConfigHigh;
            GT_U32    timeAdjustmentPropagationDelayConfigLow;
            GT_U32    triggerGenerationTODSecHigh;
            GT_U32    triggerGenerationTODSecMed;
            GT_U32    triggerGenerationTODSecLow;
            GT_U32    triggerGenerationTODNanoHigh;
            GT_U32    triggerGenerationTODNanoLow;
            GT_U32    triggerGenerationTODFracHigh;
            GT_U32    triggerGenerationTODFracLow;
            GT_U32    timeLoadValueSecHigh;
            GT_U32    timeLoadValueSecMed;
            GT_U32    timeLoadValueSecLow;
            GT_U32    timeLoadValueNanoHigh;
            GT_U32    timeLoadValueNanoLow;
            GT_U32    timeLoadValueFracHigh;
            GT_U32    timeLoadValueFracLow;
            GT_U32    timeCaptureValue0SecHigh;
            GT_U32    timeCaptureValue0SecMed;
            GT_U32    timeCaptureValue0SecLow;
            GT_U32    timeCaptureValue0NanoHigh;
            GT_U32    timeCaptureValue0NanoLow;
            GT_U32    timeCaptureValue0FracHigh;
            GT_U32    timeCaptureValue0FracLow;
            GT_U32    timeCaptureValue1SecHigh;
            GT_U32    timeCaptureValue1SecMed;
            GT_U32    timeCaptureValue1SecLow;
            GT_U32    timeCaptureValue1NanoHigh;
            GT_U32    timeCaptureValue1NanoLow;
            GT_U32    timeCaptureValue1FracHigh;
            GT_U32    timeCaptureValue1FracLow;
            GT_U32    timeCaptureStatus;
            GT_U32    timeUpdateCntr;
            GT_U32    PClkClockCycleConfigLow;
            GT_U32    PClkClockCycleConfigHigh;
            GT_U32    incomingClockInCountingConfigLow;
            GT_U32    incomingClockInCountingConfigHigh;
            GT_U32    generateFunctionMaskSecMed;
            GT_U32    generateFunctionMaskSecLow;
            GT_U32    generateFunctionMaskSecHigh;
            GT_U32    generateFunctionMaskNanoLow;
            GT_U32    generateFunctionMaskNanoHigh;
            GT_U32    generateFunctionMaskFracLow;
            GT_U32    generateFunctionMaskFracHigh;
            GT_U32    externalClockPropagationDelayConfigLow;
            GT_U32    externalClockPropagationDelayConfigHigh;
            GT_U32    driftThresholdConfigLow;
            GT_U32    driftThresholdConfigHigh;
            GT_U32    driftAdjustmentConfigLow;
            GT_U32    driftAdjustmentConfigHigh;
            GT_U32    clockCycleConfigLow;
            GT_U32    clockCycleConfigHigh;
            GT_U32    captureTriggerCntr;

        } TAI[36][2]/* FALCON: 4 eagle(5 Pair each - 1 Master pair + 2 EPCL pairs + 4 single TXQ) + 16 Raven(1 pair each), 20+16=36*/;

        struct /*PTP[73]--Network PTPs[72] + ILKN PTP--*/{

            GT_U32    PTPInterruptCause;
            GT_U32    PTPInterruptMask;
            GT_U32    PTPGeneralCtrl;
            GT_U32    PTPTXTimestampQueue0Reg0;
            GT_U32    PTPTXTimestampQueue0Reg1;
            GT_U32    PTPTXTimestampQueue0Reg2;
            GT_U32    PTPTXTimestampQueue1Reg0;
            GT_U32    PTPTXTimestampQueue1Reg1;
            GT_U32    PTPTXTimestampQueue1Reg2;
            GT_U32    totalPTPPktsCntr;
            GT_U32    PTPv1PktCntr;
            GT_U32    PTPv2PktCntr;
            GT_U32    Y1731PktCntr;
            GT_U32    NTPTsPktCntr;
            GT_U32    NTPReceivePktCntr;
            GT_U32    NTPTransmitPktCntr;
            GT_U32    WAMPPktCntr;
            GT_U32    addCorrectedTimeActionPktCntr;
            GT_U32    NTPPTPOffsetHigh;
            GT_U32    noneActionPktCntr;
            GT_U32    forwardActionPktCntr;
            GT_U32    dropActionPktCntr;
            GT_U32    captureIngrTimeActionPktCntr;
            GT_U32    captureAddTimeActionPktCntr;
            GT_U32    captureAddIngrTimeActionPktCntr;
            GT_U32    captureAddCorrectedTimeActionPktCntr;
            GT_U32    captureActionPktCntr;
            GT_U32    addTimeActionPktCntr;
            GT_U32    addIngrTimeActionPktCntr;
            GT_U32    NTPPTPOffsetLow;
            GT_U32    txPipeStatusDelay;
            GT_U32    egressTimestampQueue; /* SIP_6 */

        }PTP[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS][2]/* Network PTPs + ILKN PTP; [0] - 1/10G/100G ports, [1] - 200G/400G ports - for SIP_6 and above */;

        struct /*CG*/{
            struct /*CG_CONVERTERS*/{
                GT_U32 CGMAControl0;
                GT_U32 CGMAControl1;
                GT_U32 CGMAConvertersStatus;
                GT_U32 CGMAConvertersResets;
                GT_U32 CGMAConvertersIpStatus;
                GT_U32 CGMAConvertersIpStatus2;
                GT_U32 CGMAConvertersFcControl0;
            }CG_CONVERTERS;
            struct /*CG_PORT*/{
                struct /*CG_PORT_MAC*/{
                    GT_U32 CGPORTMACCommandConfig;
                    GT_U32 CGPORTMACaddress0;
                    GT_U32 CGPORTMACaddress1;
                    GT_U32 CGPORTMACFrmLength;
                    GT_U32 CGPORTMACRxCrcOpt;
                    GT_U32 CGPORTMACStatus;
                    GT_U32 CGPORTMACStatNConfig;
                    GT_U32 CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_LAST_E];
                    GT_U32 CGPORTMACTxIpgLength;
                }CG_PORT_MAC;
            }CG_PORT;

        }CG[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

        struct /*MTI*/{
            struct /*MTI64*/{
                GT_U32 commandConfig;
                GT_U32 frmLength;
                GT_U32 macAddr0;
                GT_U32 macAddr1;
                GT_U32 status;
                GT_U32 cl01PauseQuanta;
                GT_U32 cl23PauseQuanta;
                GT_U32 cl45PauseQuanta;
                GT_U32 cl67PauseQuanta;
                GT_U32 cl01QuantaThresh;
                GT_U32 cl23QuantaThresh;
                GT_U32 cl45QuantaThresh;
                GT_U32 cl67QuantaThresh;
                GT_U32 rxPauseStatus;
            }MTI64_MAC;

            struct /*MTI400*/{
                GT_U32 commandConfig;
                GT_U32 frmLength;
                GT_U32 macAddr0;
                GT_U32 macAddr1;
                GT_U32 status;
                GT_U32 cl01PauseQuanta;
                GT_U32 cl23PauseQuanta;
                GT_U32 cl45PauseQuanta;
                GT_U32 cl67PauseQuanta;
                GT_U32 cl01QuantaThresh;
                GT_U32 cl23QuantaThresh;
                GT_U32 cl45QuantaThresh;
                GT_U32 cl67QuantaThresh;
                GT_U32 rxPauseStatus;
            }MTI400_MAC;

            struct /*MTI_EXT*/{
                GT_U32 portStatus;
                GT_U32 portControl;
                GT_U32 segPortControl; /* used for 200G/400G ports */
            }MTI_EXT;

        }MTI[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

        struct /*MTI*/{
            GT_U32 dataHi;
            GT_U32 status;
            GT_U32 config;
            GT_U32 control;
            GT_U32 clearValueLo;
            GT_U32 clearValueHi;
            GT_U32 dataHiCdc;
            GT_U32 MTI_PORT_MAC_MIBS_REG_ADDR_ARR[56];
        }MTI_STATISTICS[MAX_DP_CNS];

         struct /*MSDB*/{
            GT_U32 fcuControl;
            /*8 channels*/
            GT_U32 fcuChannelTXControl[9];
            GT_U32 fcuChannelRXControl[9];
            GT_U32 fcuRXTimer;
        }MSDB[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

        struct LMS_ST/*LMS*/{   /* Bobcat2 A0 */

            struct /*LMS0*/{

                struct /*LMS0Group0*/{

                    struct /*SMIConfig*/{

                        GT_U32    PHYAddrReg0forPorts0Through5;
                        GT_U32    PHYAutoNegConfigReg0;
                        GT_U32    SMI0Management;
                        GT_U32    LMS0MiscConfigs;

                    }SMIConfig;

                    struct /*portsMIBCntrsInterrupts*/{

                        GT_U32    portsGOP0MIBsInterruptCause;
                        GT_U32    portsGOP0MIBsInterruptMask;

                    }portsMIBCntrsInterrupts;

                    struct /*portsMACSourceAddr*/{

                        GT_U32    sourceAddrMiddle;
                        GT_U32    sourceAddrHigh;

                    }portsMACSourceAddr;

                    struct /*portsMACMIBCntrs*/{

                        GT_U32    MIBCntrsCtrlReg0forPorts0Through5;

                    }portsMACMIBCntrs;

                    struct /*portsAndMIBCntrsInterruptSummary*/{

                        GT_U32    MIBCntrsInterruptSummary;

                    }portsAndMIBCntrsInterruptSummary;

                    struct /*periodicFCRegs*/{

                        GT_U32    gigPortsPeriodicFCCntrMaxValueReg;

                    }periodicFCRegs;

                    struct /*LEDInterfaceConfig*/{

                        GT_U32    LEDInterface0CtrlReg0forPorts0Through11AndStackPort;
                        GT_U32    LEDInterface0Group01ConfigRegforPorts0Through11;
                        GT_U32    LEDInterface0Class01ManipulationRegforPorts0Through11;
                        GT_U32    LEDInterface0Class4ManipulationRegforPorts0Through11;
                        GT_U32    stackLEDInterface0Class04ManipulationRegforStackPorts;
                        GT_U32    stackLEDInterface0Class59ManipulationRegforStackPorts;
                        GT_U32    LEDInterface0FlexlinkPortsDebugSelectRegforStackPort;
                        GT_U32    LEDInterface0FlexlinkPortsDebugSelectReg1forStackPort;

                    }LEDInterfaceConfig;

                }LMS0Group0;

                struct /*LMS0Group1*/{

                    struct /*SMIConfig*/{

                        GT_U32    flowCtrlAdvertiseForFiberMediaSelectedConfigReg0forPorts0Through11;
                        GT_U32    PHYAddrReg1forPorts6Through11;
                        GT_U32    PHYAutoNegConfigReg1;

                    }SMIConfig;

                    struct /*portsMIBCntrsInterrupts*/{

                        GT_U32    triSpeedPortsGOP1MIBsInterruptCause;
                        GT_U32    triSpeedPortsGOP1MIBsInterruptMask;

                    }portsMIBCntrsInterrupts;

                    struct /*portsMACMIBCntrs*/{

                        GT_U32    MIBCntrsCtrlReg0forPorts6Through11;

                    }portsMACMIBCntrs;

                    struct /*portsAndMIBCntrsInterruptSummary*/{

                        GT_U32    portsMIBCntrsInterruptSummaryMask;
                        GT_U32    portsInterruptSummaryMask;

                    }portsAndMIBCntrsInterruptSummary;

                    struct /*periodicFCRegs*/{

                        GT_U32    twoAndHalfGigPortsPeriodicFCCntrMaxValueReg;

                    }periodicFCRegs;

                    struct /*LEDInterfaceConfig*/{

                        GT_U32    LEDInterface0CtrlReg1AndClass6ManipulationRegforPorts0Through11;
                        GT_U32    LEDInterface0Group23ConfigRegforPorts0Through11;
                        GT_U32    LEDInterface0Class23ManipulationRegforPorts0Through11;
                        GT_U32    LEDInterface0Class5ManipulationRegforPorts0Through11;
                        GT_U32    stackLEDInterface0Class1011ManipulationRegforStackPorts;
                        GT_U32    stackLEDInterface0Group01ConfigRegforStackPort;

                    }LEDInterfaceConfig;

                    GT_U32    stackPortsModeReg;

                }LMS0Group1;

            }LMS0;

            struct /*LMS1*/{

                struct /*LMS1Group0*/{

                    struct /*SMIConfig*/{

                        GT_U32    PHYAddrReg2forPorts12Through17;
                        GT_U32    PHYAutoNegConfigReg2;
                        GT_U32    SMI1Management;
                        GT_U32    LMS1MiscConfigs;

                    }SMIConfig;

                    struct /*portsMIBCntrsInterrupts*/{

                        GT_U32    portsGOP2MIBsInterruptCause;
                        GT_U32    portsGOP2MIBsInterruptMask;

                    }portsMIBCntrsInterrupts;

                    struct /*portsMACMIBCntrs*/{

                        GT_U32    MIBCntrsCtrlReg0forPorts12Through17;

                    }portsMACMIBCntrs;

                    struct /*portsInterruptSummary*/{

                        GT_U32    portsInterruptSummary;

                    }portsInterruptSummary;

                    struct /*LEDInterfaceConfig*/{

                        GT_U32    LEDInterface1CtrlReg0forPorts12Through23AndStackPort;
                        GT_U32    LEDInterface1Group01ConfigRegforPorts12Through23;
                        GT_U32    LEDInterface1Class01ManipulationRegforPorts12Through23;
                        GT_U32    LEDInterface1Class4ManipulationRegforPorts12Through23;
                        GT_U32    stackLEDInterface1Class04ManipulationRegforStackPorts;
                        GT_U32    stackLEDInterface1Class59ManipulationRegforStackPorts;
                        GT_U32    LEDInterface1FlexlinkPortsDebugSelectRegforStackPort;
                        GT_U32    LEDInterface1FlexlinkPortsDebugSelectReg1forStackPort;

                    }LEDInterfaceConfig;

                }LMS1Group0;

                struct /*LMS1Group1*/{

                    struct /*SMIConfig*/{

                        GT_U32    flowCtrlAdvertiseForFiberMediaSelectedConfigReg1forPorts12Through23;
                        GT_U32    PHYAddrReg3forPorts18Through23;
                        GT_U32    PHYAutoNegConfigReg3;

                    }SMIConfig;

                    struct /*portsMIBCntrsInterrupts*/{

                        GT_U32    portsGOP3AndStackMIBsInterruptCause;
                        GT_U32    portsGOP3AndStackMIBsInterruptMask;

                    }portsMIBCntrsInterrupts;

                    struct /*portsMACMIBCntrs*/{

                        GT_U32    MIBCntrsCtrlReg0forPorts18Through23;

                    }portsMACMIBCntrs;

                    struct /*periodicFCRegs*/{

                        GT_U32    stackPorts1GPeriodicFCCntrMaxValueReg;

                    }periodicFCRegs;

                    struct /*LEDInterfaceConfig*/{

                        GT_U32    LEDInterface1CtrlReg1AndClass6ManipulationforPorts12Through23;
                        GT_U32    LEDInterface1Group23ConfigRegforPorts12Through23;
                        GT_U32    LEDInterface1Class23ManipulationRegforPorts12Through23;
                        GT_U32    LEDInterface1Class5ManipulationRegforPorts12Through23;
                        GT_U32    stackLEDInterface1Class1011ManipulationRegforStackPort;
                        GT_U32    stackLEDInterface1Group01ConfigRegforStackPort;

                    }LEDInterfaceConfig;

                }LMS1Group1;

            }LMS1;

        }LMS[3];
        struct LED_ST    /* Bobcat2 B0 */
        {
            GT_U32 LEDControl;
            GT_U32 blinkGlobalControl;
            GT_U32 classGlobalControl;
            GT_U32 classesAndGroupConfig;
            GT_U32 classForcedData[6];
            GT_U32 portIndicationSelect[32];   /* BC2 -12 BobK  12 Aldrin 32 BC3 - 13 */
            GT_U32 portTypeConfig[2];          /* BC2   1 BobK   1 Aldrin  2 BC3   1  */
            GT_U32 portIndicationPolarity[8];  /* BC2 - 3 BobK   3 Aldrin  8 BC3 - 4  */
            GT_U32 classRawData[6];
        }LED[5];
        struct SMI_ST     /* Bobcat2 B0 */
        {
            GT_U32 SMIManagement;
            GT_U32 SMIMiscConfiguration;
            GT_U32 PHYAutoNegotiationConfig;
            GT_U32 PHYAddress[16];
        }SMI[4];

        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC FCA[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS]/*ports 0..71*/;

        struct /*PR*/{

            struct /*PRMIB*/{

                GT_U32    MIBChannelSelect;
                GT_U32    MIBGeneralConfigs;
                GT_U32    MIBWraparoundInterruptCause0;
                GT_U32    MIBWraparoundInterruptMask0;
                GT_U32    MIBWraparoundInterruptCause1;
                GT_U32    MIBWraparoundInterruptMask1;
                GT_U32    MIBWraparoundInterruptCause2;
                GT_U32    MIBWraparoundInterruptMask2;

                GT_U32    MIBCountersBase;
            }PRMIB;

            struct /*PRInterrupts*/{

                GT_U32    rxFullInterruptCause0;
                GT_U32    rxFullInterruptMask0;
                GT_U32    rxFullInterruptCause1;
                GT_U32    rxFullInterruptMask1;
                GT_U32    rxFullInterruptCause2;
                GT_U32    rxFullInterruptMask2;
                GT_U32    noSOPInterruptCause0;
                GT_U32    noSOPInterruptMask0;
                GT_U32    noSOPInterruptCause1;
                GT_U32    noSOPInterruptMask2;
                GT_U32    noSOPInterruptCause2;
                GT_U32    noSOPInterruptMask1;
                GT_U32    fillOverrunInterruptCause0;
                GT_U32    fillOverrunInterruptMask0;
                GT_U32    fillOverrunInterruptCause1;
                GT_U32    fillOverrunInterruptMask1;
                GT_U32    noEOPInterruptMask2;
                GT_U32    fillOverrunInterruptMask2;
                GT_U32    noEOPInterruptMask0;
                GT_U32    fillUnderrunInterruptMask0;
                GT_U32    fillUnderrunInterruptCause1;
                GT_U32    fillUnderrunInterruptMask1;
                GT_U32    fillUnderrunInterruptCause2;
                GT_U32    fillUnderrunInterruptMask2;
                GT_U32    PRGeneralCauseReg;
                GT_U32    PRGeneralMaskReg;
                GT_U32    noEOPInterruptCause0;
                GT_U32    fillUnderrunInterruptCause0;
                GT_U32    noEOPInterruptCause1;
                GT_U32    noEOPInterruptMask1;
                GT_U32    noEOPInterruptCause2;
                GT_U32    fillOverrunInterruptCause2;
                GT_U32    PRSummaryCauseReg;
                GT_U32    PRSummaryMaskReg;

            }PRInterrupts;

            struct /*PRFunctional*/{

                GT_U32    channelConfig[64]/*Channel*/;
                GT_U32    minMaxPkts;
                GT_U32    general;

            }PRFunctional;

        }PR;

        struct /* Interlaken Register File */
        {
            GT_U32    ILKN_COUNT_BLK_TYPE_ERR_H[8]/*index*/;
            GT_U32    ILKN_COUNT_BLK_TYPE_ERR_L[16]/*index*/;
            GT_U32    ILKN_STAT_EN;
            GT_U32    ILKN_RX_PKT_GEN;
            GT_U32    ILKN_REMOTE_LANE_STATUS;
            GT_U32    ILKN_PTR_RST;
            GT_U32    ILKN_MAC_CFG_0;
            GT_U32    ILKN_PCS_PARAM;
            GT_U32    ILKN_MAC_CFG_1;
            GT_U32    ILKN_MAC_CFG_2;
            GT_U32    ILKN_MAC_CFG_3;
            GT_U32    ILKN_MAC_CFG_4;
            GT_U32    ILKN_MAC_CFG_5;
            GT_U32    ILKN_PCS_DEBUG_1;
            GT_U32    ILKN_PCS_DEBUG_0;
            GT_U32    ILKN_PCS_CFG_2;
            GT_U32    ILKN_PCS_CFG_1;
            GT_U32    ILKN_PCS_CFG_0;
            GT_U32    ILKN_OVF_7;
            GT_U32    ILKN_OVF_6;
            GT_U32    ILKN_MAC_RL_CFG_1;
            GT_U32    ILKN_OVF_4;
            GT_U32    ILKN_MAC_CFG_12;
            GT_U32    ILKN_CH_FC_CFG_1;
            GT_U32    ILKN_OVF_1;
            GT_U32    ILKN_OVF_0;
            GT_U32    ILKN_MAC_RL_CFG_2;
            GT_U32    ILKN_INT_CLR;
            GT_U32    ILKN_MAC_RL_CFG_0;
            GT_U32    ILKN_MAC_CBFC_BM_0_1;
            GT_U32    ILKN_MAC_CFG_9;
            GT_U32    ILKN_MAC_CFG_8;
            GT_U32    ILKN_MAC_CFG_7;
            GT_U32    ILKN_MAC_CFG_6;
            GT_U32    ILKN_MAC_CBFC_BM_2_0;
            GT_U32    ILKN_MAC_CBFC_BM_2_1;
            GT_U32    ILKN_MAC_CBFC_BM_2_2;
            GT_U32    ILKN_MAC_CBFC_BM_3_0;
            GT_U32    ILKN_MAC_CBFC_BM_3_1;
            GT_U32    ILKN_MAC_CFG_11;
            GT_U32    ILKN_MAC_CFG_10;
            GT_U32    ILKN_WORD_LOCK_STATUS;
            GT_U32    ILKN_SYNC_LOCK_STATUS;
            GT_U32    ILKN_MAC_CBFC_BM_CNTL;
            GT_U32    ILKN_MAC_CBFC_BM_3_2;
            GT_U32    ILKN_ERR_GEN_STATUS;
            GT_U32    ILKN_PCS_DEBUG_DATA;
            GT_U32    ILKN_MAC_FC_STATUS;
            GT_U32    ILKN_PROT_ERR_STATUS;
            GT_U32    ILKN_LINK_STATUS;
            GT_U32    ILKN_MAC_CBFC_BM_1_2;
            GT_U32    ILKN_MAC_CBFC_BM_1_1;
            GT_U32    ILKN_MAC_CBFC_BM_1_0;
            GT_U32    ILKN_MAC_CBFC_BM_0_2;
            GT_U32    ILKN_EN;
            GT_U32    ILKN_MAC_CBFC_BM_0_0;
            GT_U32    ILKN_LL_FC_STATUS;
            GT_U32    ILKN_FIFO_RST;
            GT_U32    ILKN_LANE_SWAP;
            GT_U32    ILKN_PCS_DEBUG_2;
            GT_U32    ILKN_PCS_DEBUG_3;
            GT_U32    ILKN_PCS_DEBUG_4;
            GT_U32    ILKN_PCS_DEBUG_5;
            GT_U32    ILKN_COUNT_WORD_SYNC_ERR_L[16]/*index*/;
            GT_U32    ILKN_COUNT_WORD_SYNC_ERR_H[8]/*index*/;
            GT_U32    ILKN_COUNT_TX_PKT_LOW;
            GT_U32    ILKN_COUNT_TX_PKT_HIGH;
            GT_U32    ILKN_COUNT_TX_BYTE_LOW;
            GT_U32    ILKN_COUNT_TX_BYTE_HIGH;
            GT_U32    ILKN_COUNT_TX_BAD_PKT_LOW;
            GT_U32    ILKN_COUNT_TX_BAD_PKT_HIGH;
            GT_U32    ILKN_COUNT_RX_PKT_LOW;
            GT_U32    ILKN_COUNT_RX_PKT_HIGH;
            GT_U32    ILKN_COUNT_RX_CRC_ERROR_LOW;
            GT_U32    ILKN_COUNT_RX_CRC_ERROR_HIGH;
            GT_U32    ILKN_COUNT_RX_BYTE_LOW;
            GT_U32    ILKN_COUNT_RX_BYTE_HIGH;
            GT_U32    ILKN_COUNT_RX_BURST_SIZE_ERROR_L;
            GT_U32    ILKN_COUNT_RX_BURST_SIZE_ERROR_H;
            GT_U32    ILKN_COUNT_RX_BAD_PKT_LOW;
            GT_U32    ILKN_COUNT_RX_BAD_PKT_HIGH;
            GT_U32    ILKN_COUNT_RX_ALIGNMENT_FAILURE;
            GT_U32    ILKN_COUNT_RX_ALIGNMENT_ERROR;
            GT_U32    ILKN_COUNT_DIAG_CRC_ERR_L[16]/*index*/;
            GT_U32    ILKN_COUNT_DIAG_CRC_ERR_H[8]/*index*/;
            GT_U32    ILKN_OVF_2;
            GT_U32    ILKN_OVF_3;
            GT_U32    ILKN_CH_FC_CFG_2;
            GT_U32    ILKN_OVF_5;
            GT_U32    ILKN_CH_FC_CFG_0;
            GT_U32    ILKN_ALIGN_STATUS;
        }ILKN;

        struct /* Interlaken Wrapper Register File */
        {
            GT_U32    sdFcLedConvertersControl0;
            GT_U32    txdmaConverterControl0;
            GT_U32    rxdmaConverterPacketSizeRestriction;

        }ILKN_WRP;
    }GOP;

    struct /*IPvX*/{
        struct /*routerManagementCntrsSets*/{

            GT_U32    routerManagementInUcNonRoutedExceptionPktsCntrSet[4]/*set number*/;
            GT_U32    routerManagementInMcTrappedMirroredPktCntrSet[4]/*set number*/;
            GT_U32    routerManagementInUcPktsCntrSet[4]/*set number*/;
            GT_U32    routerManagementInMcPktsCntrSet[4]/*set number*/;
            GT_U32    routerManagementInUcNonRoutedNonException[4]/*set number*/;
            GT_U32    routerManagementInMcNonRoutedNonExceptionPktsCntrSet[4]/*set number*/;
            GT_U32    routerManagementInMcNonRoutedExceptionPktsCntrSet[4]/*set nmber*/;
            GT_U32    routerManagementInUcTrappedMirroredPktCntrSet[4]/*set number*/;
            GT_U32    routerManagementInMcRPFFailCntrSet[4]/*set number*/;
            GT_U32    routerManagementOutUcPktCntrSet[4]/*set number*/;
            GT_U32    routerManagementCntrsSetConfig0[4]/*set number*/;
            GT_U32    routerManagementCntrsSetConfig1[4]/*set number*/;
            GT_U32    routerManagementCntrsSetConfig2[4]/*set number*/;
            GT_U32    routerManagementIncomingPktCntr;
            GT_U32    routerManagementOutgoingPktCntr;
            GT_U32    routerManagementInSIPFilterCntr;

        }routerManagementCntrsSets;

        struct /*routerGlobalCtrl*/{

            GT_U32    routerGlobalCtrl0;
            GT_U32    routerMTUConfigReg[4]/*MTU Index*/;
            GT_U32    routerGlobalCtrl1;
            GT_U32    routerGlobalCtrl2;
            GT_U32    routerGlobalCtrl3;

        }routerGlobalCtrl;

        struct /*routerDropCntr*/{

            GT_U32    routerDropCntr;
            GT_U32    routerDropCntrConfig;

        }routerDropCntr;

        struct /*routerBridgedPktsExceptionCntr*/{

            GT_U32    routerBridgedPktExceptionsCntr;

        }routerBridgedPktsExceptionCntr;

        struct /*IPv6Scope*/{

            GT_U32    IPv6UcScopeTableReg[4]/*RegNum*/;
            GT_U32    IPv6McScopeTableReg[4]/*RegNum*/;
            GT_U32    IPv6UcScopePrefix[4];
            GT_U32    IPv6UcScopeLevel[4];

        }IPv6Scope;

        struct /*IPv6GlobalCtrl*/{

            GT_U32    IPv6CtrlReg0;
            GT_U32    IPv6CtrlReg1;

        }IPv6GlobalCtrl;

        struct /*IPv4GlobalCtrl*/{

            GT_U32    IPv4CtrlReg0;
            GT_U32    IPv4CtrlReg1;

        }IPv4GlobalCtrl;

        struct /*internalRouter*/{

            GT_U32    routerInterruptCause;
            GT_U32    routerInterruptMask;

        }internalRouter;

        struct /*FCoEGlobalCtrl*/{

            GT_U32    routerFCoEGlobalConfig;

        }FCoEGlobalCtrl;

        struct /*ECMPRoutingConfig*/{

            GT_U32    ECMPConfig;
            GT_U32    ECMPSeed;

        }ECMPRoutingConfig;

        struct /*CPUCodes*/{

            GT_U32    CPUCodes0;
            GT_U32    CPUCodes1;
            GT_U32    CPUCodes2;
            GT_U32    CPUCodes3;
            GT_U32    CPUCodes4;
            GT_U32    CPUCodes5;
            GT_U32    CPUCodes6;
            GT_U32    CPUCodes7;
            GT_U32    CPUCodes8;
            GT_U32    CPUCodes9;
            GT_U32    CPUCodes10;
            GT_U32    CPUCodes11;

        }CPUCodes;

        struct /*FdbIpLookup*/{
            GT_U32    FdbIpv4RouteLookupMask;
            GT_U32    FdbIpv6RouteLookupMask[4];
        }FdbIpLookup;

        struct /*FdbRouteLookup*/{
            GT_U32    FdbHostLookup;
        }FdbRouteLookup;

        GT_U32  routerFifoThreshold1Reg;/*Router FIFOs threshold 1*/

    }IPvX;

    struct /*PLR*/{

REGISTER_COVERED_CNS        GT_U32    policerInitialDP[64]/*QoSProfile*/;/* not used in sip6.0*/
/*treated bits 0-3,5-18,21-22, 24-28,31*/        GT_U32    policerCtrl0;
/*treated bits 5,7-9*/        GT_U32    policerCtrl1;
        GT_U32    globalBillingCntrIndexingModeConfig0;
        GT_U32    policerScanCtrl;
        GT_U32    policerPortMeteringReg[16]/*register*/;/* not used in sip6.0*/
        GT_U32    policerPerPortRemarkTableID[32]/*Port*/;
        GT_U32    policerOAM;
REGISTER_COVERED_CNS        GT_U32    policerMRU;
REGISTER_COVERED_CNS        GT_U32    IPFIXSecondsMSbTimerStampUpload;
        GT_U32    policerCountingBaseAddr;
        GT_U32    policerMeteringBaseAddr;
REGISTER_COVERED_CNS        GT_U32    policerIPFIXPktCountWAThreshold;
REGISTER_COVERED_CNS        GT_U32    policerIPFIXDroppedPktCountWAThreshold;
REGISTER_COVERED_CNS        GT_U32    policerIPFIXByteCountWAThresholdMSB;
REGISTER_COVERED_CNS        GT_U32    policerIPFIXByteCountWAThresholdLSB;
        GT_U32    policerInterruptMask;
        GT_U32    policerInterruptCause;
REGISTER_COVERED_CNS        GT_U32    policerTableAccessData[8]/*Reg*/;
REGISTER_COVERED_CNS        GT_U32    IPFIXSampleEntriesLog1;
REGISTER_COVERED_CNS        GT_U32    policerError;
REGISTER_COVERED_CNS        GT_U32    policerErrorCntr;
        GT_U32    meteringAndCountingRAMSize0;
        GT_U32    meteringAndCountingRAMSize1;
        GT_U32    meteringAndCountingRAMSize2;
REGISTER_COVERED_CNS        GT_U32    policerTableAccessCtrl;
        GT_U32    policerMeteringRefreshScanRateLimit;
        GT_U32    policerMeteringRefreshScanAddrRange;
REGISTER_COVERED_CNS        GT_U32    IPFIXSecondsLSbTimerStampUpload;
REGISTER_COVERED_CNS        GT_U32    policerShadow[2]/*RegNum*/;
REGISTER_COVERED_CNS        GT_U32    IPFIXSampleEntriesLog0;
REGISTER_COVERED_CNS        GT_U32    IPFIXNanoTimerStampUpload;
REGISTER_COVERED_CNS        GT_U32    IPFIXCtrl;
/*treated bits 0-1,5-7*/        GT_U32    hierarchicalPolicerCtrl;
        GT_U32    globalBillingCntrIndexingModeConfig1;
REGISTER_COVERED_CNS       GT_U32    portAndPktTypeTranslationTable[512 ]/*port<perPort>*/;/* not used in sip6.0*/
        GT_U32      policerBadAddressLatch;
        GT_U32      policerManagementCntrsTbl;  /* PLR Management Counters */
        GT_U32      policerCountingTbl;         /* PLR Counting Table */
        GT_U32      policerTimerTbl;            /* PLR Timer Table */
        GT_U32      ipfixWrapAroundAlertTbl;    /* PLR IPFIX wrap around alert */
        GT_U32      ipfixAgingAlertTbl;         /* PLR IPFIX aging alert */
        GT_U32      meteringAddressingModeCfg0;     /* Metering Addressing Mode Configuration 0 */
        GT_U32      meteringAddressingModeCfg1;     /* Metering Addressing Mode Configuration 1 */
        GT_U32      mef10_3_bucket0MaxRateArr[128]; /* MEF 10.3 Bucket0 Max Rate */
        GT_U32      mef10_3_bucket1MaxRateArr[128]; /* MEF 10.3 Bucket1 Max Rate */
        GT_U32      qosProfile2PriorityMapArr[128]; /* Qos Profile to Priority Map */ /* not used in sip6.0*/

        GT_U32      debug_LFSRSeed[2]; /* debug register */
        GT_U32      debug_PrngControl1;/* debug register */
        GT_U32      debug_PrngControl2;/* debug register */

        struct {
            /* new in sip 5.20*/
            GT_U32    countingConfigTableThreshold[7]/*thresh_num*/;
            GT_U32    countingConfigTableSegment[8]/*table_segment*/;

            GT_U32    statisticalCountingConfig0;
            GT_U32    statisticalCountingConfig1;
            GT_U32    meteringAnalyzerIndexes;
        }globalConfig;
    }PLR[3];

    struct /*CNC*/
    {
        struct /*perBlockRegs*/
        {
            struct /*wraparound*/
            {
                GT_U32    CNCBlockWraparoundStatusReg[16][4]/*Block*//*Entry*/;
            }wraparound;

            struct /*rangeCount*/
            {
                GT_U32    CNCBlockRangeCountEnable[2][16]/*group of ranges*//*block*/;
            }rangeCount;

            struct /*clientEnable*/
            {
                GT_U32    CNCBlockConfigReg0[16]/*Block*/;
            }clientEnable;

            struct /*entryMode*/
            {
                GT_U32    CNCBlocksCounterEntryModeRegister[2]/*Block/8*/;
            }entryMode;

            struct /*memory*/
            {
                GT_U32    baseAddress[16]/*Block memory base address*/;
            }memory;
        }perBlockRegs;

        struct /*globalRegs*/
        {
            GT_U32    CNCGlobalConfigReg;
            GT_U32    CNCFastDumpTriggerReg;
            GT_U32    CNCClearByReadValueRegWord0;
            GT_U32    CNCClearByReadValueRegWord1;
            GT_U32    CNCInterruptSummaryCauseReg;
            GT_U32    CNCInterruptSummaryMaskReg;
            GT_U32    wraparoundFunctionalInterruptCauseReg;
            GT_U32    wraparoundFunctionalInterruptMaskReg;
            GT_U32    rateLimitFunctionalInterruptCauseReg;
            GT_U32    rateLimitFunctionalInterruptMaskReg;
            GT_U32    miscFunctionalInterruptCauseReg;
            GT_U32    miscFunctionalInterruptMaskReg;
        }globalRegs;

    }CNC[2]/*CNC number*/;

    struct /*TXQ*/
    {
        struct
        {
            GT_U32 Credit_Value;
            GT_U32 PSI_Interrupt_Cause;
            GT_U32 PSI_Interrupt_Mask;
            GT_U32 PSI_Last_Address_Violation;
            GT_U32 PSI_Metal_Fix_Register;
            GT_U32 QmrSchPLenBursts;
            GT_U32 QmrSchBurstsSent;
            GT_U32 MSG_FIFO_Max_Peak;
            GT_U32 MSG_FIFO_Min_Peak;
            GT_U32 MSG_FIFO_Fill_Level;
            GT_U32 Queue_Status_Read_Request;
            GT_U32 Queue_Status_Read_Reply;
            GT_U32 Debug_CFG_Register;
        }
        psi_regs;

         struct
         {
            GT_U32 pdx_metal_fix;
            GT_U32 DB_Read_Control;
            GT_U32 QCX_Read_qu_db;
            GT_U32 QCX_Read_Port_DB;
            GT_U32 QCX_Read_Reorder_Fifos;
            GT_U32 pdx2pdx_interface_mapping;
            GT_U32 Interrupt_Cause;
            GT_U32 Interrupt_Mask;
            GT_U32 Address_Violation;
            GT_U32 pfcc_burst_fifo_thr;
            GT_U32 global_burst_fifo_thr;
            GT_U32 micro_Memory_Traffic_Counter;
            GT_U32 global_burst_fifo_available_entries;
            GT_U32 max_bank_fill_levels;
            GT_U32 drop_Counter;
            GT_U32 Drop_Counter_Mask;
            GT_U32 MIN_Global_Burst_Fifo_Available_Entries;
         }
        pdx;/*pdx_itx*/

    struct
    {
        GT_U32 global_pfcc_cfg;
        GT_U32 pool_CFG;
        GT_U32 PFCC_metal_fix;
        GT_U32 TC_to_pool_CFG;
        GT_U32 PFCC_Interrupt_Cause;
        GT_U32 PFCC_Interrupt_Mask;
        GT_U32 Address_Violation;
        GT_U32 PFCC_Cycle_Counter;
        GT_U32 Aggregation_Request_Gen_SM_State;
        GT_U32 Source_Port_Requested_For_Read;
        GT_U32 Source_Port_Counter;
        GT_U32 pb_occupancy_status;
    }
    pfcc;

    struct
    {
        GT_U32 pdx_pac_metal_fix;
        GT_U32 Interrupt_Cause;
        GT_U32 Interrupt_Mask;
        GT_U32 Address_Violation;
    }
    pdx_pac[2];


      struct
      {
        GT_U32 Max_PDS_size_limit_for_pdx;
        GT_U32 PDS_Metal_Fix;
        GT_U32 Global_PDS_CFG;
        GT_U32 FIFOs_Limits;
        GT_U32 FIFOs_DQ_Disable;
        GT_U32 Max_PDS_size_limit_for_PB;
        GT_U32 Max_Num_Of_Long_Queues;
        GT_U32 Tail_Size_for_PB_Wr;
        GT_U32 PID_Empty_Limit_for_PDX;
        GT_U32 PID_Empty_Limit_for_PB;
        GT_U32 NEXT_Empty_Limit_for_PDX;
        GT_U32 NEXT_Empty_Limit_for_PB;
        GT_U32 PB_Full_Limit;
        GT_U32 PDS_Interrupt_Debug_Cause;
        GT_U32 PDS_Interrupt_Debug_Mask;
        GT_U32 PDS_Last_Address_Violation;
        GT_U32 PDS_Interrupt_Summary_Mask;
        GT_U32 PDS_Interrupt_Summary_Cause;
        GT_U32 PDS_Interrupt_Functional_Mask;
        GT_U32 PDS_Interrupt_Functional_Cause;
        GT_U32 Total_PDS_Counter;
        GT_U32 PDS_Cache_Counter;
        GT_U32 Idle_Register;
        GT_U32 Free_Next_FIFO_Max_Peak;
        GT_U32 Free_Next_FIFO_Min_Peak;
        GT_U32 Free_PID_FIFO_Max_Peak;
        GT_U32 Free_PID_FIFO_Min_Peak;
        GT_U32 Data_Read_Order_FIFO_Max_Peak;
        GT_U32 Head_Answer_FIFO_Max_Peak;
        GT_U32 PB_Read_Line_Buff_FIFO_Max_Peak;
        GT_U32 PB_Read_Req_FIFO_Max_Peak;
        GT_U32 PB_Write_Req_FIFO_Max_Peak;
        GT_U32 PB_Write_Reply_FIFO_Max_Peak;
        GT_U32 PB_Read_FIFO_State_Return_FIFO_Max_Peak;
        GT_U32 Tail_Ans_FIFO_Max_Peak;
        GT_U32 PB_Wr_Line_Buff_Ctrl_FIFO_Max_Peak;
        GT_U32 PB_Wr_Line_Buff_Data_FIFO_Max_Peak;
        GT_U32 PB_FIFO_State_Read_Order_FIFO_Max_Peak;
        GT_U32 Frag_On_The_Air_Cntr_Queue_Status;
        GT_U32 Datas_BMX_Addr_Out_Of_Range_Port;
        GT_U32 PB_Write_FIFO_State_Return_FIFO_Max_Peak;
        GT_U32 PB_Read_Write_Order_FIFO_Max_Peak;
        GT_U32 FIFO_State_Latency_Max_Peak;
        GT_U32 FIFO_State_Latency_Min_Peak;
        GT_U32 Long_Queue_Counter;
        GT_U32 Long_Queue_Count_Max_Peak;
        GT_U32 PDS_Cache_Count_Max_Peak;
        GT_U32 Total_PDS_Count_Max_Peak;
        GT_U32 Active_Queue_Counter;
        GT_U32 Active_Queue_Count_Max_Peak;
    }pds[CPSS_DXCH_SIP6_PDS_UNITS_PER_TILE_MAC];



        struct
        {
            GT_U32 global_config;
            GT_U32 SDQ_Metal_Fix;
            GT_U32 SDQ_Interrupt_Functional_Cause;
            GT_U32 sdq_interrupt_functional_mask;
            GT_U32 SDQ_Interrupt_Summary_Mask;
            GT_U32 SDQ_Interrupt_Summary_Cause;
            GT_U32 SDQ_Interrupt_Debug_Mask;
            GT_U32 SDQ_Interrupt_Debug_Cause;
            GT_U32 QCN_Config;
            GT_U32 Sdq_Idle;
            GT_U32 Illegal_Bad_Address;
            GT_U32 QCN_Message_Drop_Counter;
            GT_U32 Queue_Dryer_FIFO_Max_Peak;
            GT_U32 PDQ_Message_Output_FIFO_Max_Peak;
            GT_U32 QCN_Message_Drop_Counter_Control;
            GT_U32 QCN_Message_FIFO_Max_Peak;
            GT_U32 QCN_Message_FIFO_Fill_Level;
            GT_U32 Elig_Func_Error_Capture;
            GT_U32 Select_Func_Error_Capture;
        }
        sdq[CPSS_DXCH_SIP6_SDQ_UNITS_PER_TILE_MAC];

        struct
        {
                GT_U32 global_pb_limit;
                GT_U32 QFC_Interrupt_functional_Cause;
                GT_U32 QFC_Interrupt_functional_Mask;
                GT_U32 Address_Violation;
                GT_U32 qfc_interrupt_debug_cause;
                GT_U32 QFC_Interrupt_Debug_Mask;
                GT_U32 QFC_Interrupt_Summary_Cause;
                GT_U32 QFC_Interrupt_Summary_Mask;
                GT_U32 qfc_metal_fix;
                GT_U32 Global_PFC_conf;
                GT_U32 PFC_messages_Drop_counter;
                GT_U32 global_PFC_status;
                GT_U32 Uburst_Event_FIFO_Max_Peak;
                GT_U32 Uburst_Event_FIFO_Min_Peak;
                GT_U32 PFC_Messages_counter;
                GT_U32 Xoff_Size_Indirect_Read_Access;
                GT_U32 HR_Counters_Indirect_Read_Access;
                GT_U32 Port_TC_HR_Counter_Min_Max_Conf;
                GT_U32 HR_Counter_Max_Peak;
                GT_U32 HR_Counter_Min_Peak;

                /*Added manually*/
                GT_U32 Counter_Table_0_Indirect_Read_Address;/*Queue Length Counters, address 0-399 (number of target queue)*/
                GT_U32 Counter_Table_0_Indirect_Read_Data;/*Queue Length Counters data*/
                GT_U32 Counter_Table_1_Indirect_Read_Address;/*Port Length Counters, address 0-8 (number of target port)*/
                GT_U32 Counter_Table_1_Indirect_Read_Data;/*Port Length Counters data*/
                GT_U32 Counter_Table_2_Indirect_Read_Address;/*PFC Counters, address 0-2063 (number of {port,tc} counter, IE port 1 TC 3 would be 8*1+3=11)*/
                GT_U32 Counter_Table_2_Indirect_Read_Data;/*PFC Counters data*/
            }

    qfc[CPSS_DXCH_SIP6_QFC_UNITS_PER_TILE_MAC];


        struct /*dq*/
        {
#define MAX_DQ_NUM_PORTS_CNS        SIP_5_20_DQ_NUM_PORTS_CNS
#define SIP_5_20_DQ_NUM_PORTS_CNS   96/* 96 ports per DQ (BC3 with 6 DQs) */
#define SIP_5_DQ_NUM_PORTS_CNS      72/* 72 ports per DQ (BC2,bobk with single DQ) */
            struct /*statisticalAndCPUTrigEgrMirrToAnalyzerPort*/
            {
                                            /* 512/8 = 64 (the BC3 hold 72) */
                GT_U32    egrAnalyzerEnableIndex[PORTS_NUM_CNS/8]/*physical Port/8 -> 8 ports per register */;
                GT_U32    STCStatisticalTxSniffConfig;
                GT_U32    egrCTMTrigger;
                GT_U32    CTMAnalyzerIndex;
                GT_U32    STCAnalyzerIndex;

            }statisticalAndCPUTrigEgrMirrToAnalyzerPort;

            struct /*shaper*/
            {

                GT_U32    portTokenBucketMode[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                GT_U32    portDequeueEnable[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                GT_U32    tokenBucketUpdateRate;
                GT_U32    tokenBucketBaseLine;
                GT_U32    CPUTokenBucketMTU;
                GT_U32    portsTokenBucketMTU;

            }shaper;

            struct /*scheduler*/
            {

                struct /*schedulerConfig*/
                {

                    GT_U32    conditionalMaskForPort[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                    GT_U32    schedulerConfig;
                    GT_U32    pktLengthForTailDropDequeue;
                    GT_U32    highSpeedPorts[2];

                }schedulerConfig;

                struct /*priorityArbiterWeights*/
                {

                    GT_U32    profileSDWRRGroup[16]/*Profile*/;
                    GT_U32    profileSDWRRWeightsConfigReg0[16]/*Profile*/;
                    GT_U32    profileSDWRRWeightsConfigReg1[16]/*Profile*/;
                    GT_U32    profileSDWRREnable[16]/*Profile*/;
                    GT_U32    portSchedulerProfile[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;

                }priorityArbiterWeights;

                struct /*portShaper*/
                {

                    GT_U32    portRequestMaskSelector[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                    GT_U32    portRequestMask;
                    GT_U32    fastPortShaperThreshold;

                }portShaper;

                struct /*portArbiterConfig*/
                {
                    /* BC2 / BobK --- 85 registers, */
                    /* Aldrin     --- 96 registres  */
                    GT_U32    portsArbiterPortWorkConservingEnable[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-port*/;
                    GT_U32    portsArbiterMap[96]/*Slice_Group*/;
                    GT_U32    portsArbiterConfig;
                    GT_U32    portsArbiterStatus;

                }portArbiterConfig;

            }scheduler;

            struct /*global*/
            {

                struct /*memoryParityError*/
                {

                    GT_U32    tokenBucketPriorityParityErrorCntr;
                    GT_U32    parityErrorBadAddr;

                }memoryParityError;

                struct /*interrupt*/
                {

                    GT_U32    flushDoneInterruptCause[4]/*pipe*/;
                    GT_U32    flushDoneInterruptMask[4]/*pipe*/;
                    GT_U32    egrSTCInterruptCause[4]/*pipe*/;
                    GT_U32    egrSTCInterruptMask[4]/*pipe*/;
                    GT_U32    txQDQInterruptSummaryCause;
                    GT_U32    txQDQInterruptSummaryMask;
                    GT_U32    txQDQMemoryErrorInterruptCause;
                    GT_U32    txQDQMemoryErrorInterruptMask;
                    GT_U32    txQDQGeneralInterruptCause;
                    GT_U32    txQDQGeneralInterruptMask;

                }interrupt;

                struct /*globalDQConfig*/
                {

                    GT_U32    portToDMAMapTable[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                    GT_U32    profileByteCountModification[16]/*Profile*/;
                    GT_U32    portBCConstantValue[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                    GT_U32    globalDequeueConfig;
                    GT_U32    BCForCutThrough;
                    GT_U32    creditCountersReset[3*6];

                }globalDQConfig;

                struct /*flushTrig*/
                {

                    GT_U32    portTxQFlushTrigger[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;

                }flushTrig;

                struct /*ECCCntrs*/
                {

                    GT_U32    DQIncECCErrorCntr;
                    GT_U32    DQClrECCErrorCntr;

                }ECCCntrs;

                struct /*dropCntrs*/
                {

                    GT_U32    egrMirroredDroppedCntr;
                    GT_U32    STCDroppedCntr;

                }dropCntrs;

                struct /*Credit_Counters*/
                {
                    GT_U32    txdmaPortCreditCounter[PRV_CPSS_MAX_DMA_NUM_CNS];
                }creditCounters;

            }global;

            struct /*flowCtrlResponse*/
            {

                GT_U32    schedulerProfilePFCTimerToPriorityQueueMapEnable[16]/*profile*/;
                GT_U32    schedulerProfilePriorityQueuePFCFactor[16][8]/*Profile*//*TC*/;
                GT_U32    schedulerProfileLLFCXOFFValue[16]/*profile*/;
                GT_U32    portToTxQPortMap[256]/*Source Port*/;
                GT_U32    PFCTimerToPriorityQueueMap[8]/*TC*/;
                GT_U32    flowCtrlResponseConfig;
                GT_U32    ttiToPfcResponseTableEntry;

            }flowCtrlResponse;

        }dq[6];

        struct /*ll*/
        {

            struct /*global*/{

                struct /*portsLinkEnableStatus*/{

                    GT_U32    portsLinkEnableStatus[18];

                }portsLinkEnableStatus;

                struct /*interrupts*/{

                    GT_U32    LLInterruptCause;
                    GT_U32    LLInterruptMask;

                }interrupts;

                struct /*globalLLConfig*/{

                    GT_U32    profilePriorityQueueToPoolAssociation[16]/*profile*/;
                    GT_U32    portProfile[72]/*Port*/;
                    GT_U32    highSpeedPort[8]/*port*/;

                }globalLLConfig;

                struct /*debugBus*/{

                    GT_U32    llMetalFix;

                }debugBus;

            }global;

        }ll;

        struct /*pfc*/
        {

            struct /*PFCInterrupts*/{

                GT_U32    PFCPortGroupGlobalOverUnderflowInterruptCause;
                GT_U32    PFCPortGroupGlobalOverUnderflowInterruptMask;
                GT_U32    PFCInterruptSummaryCause;
                GT_U32    PFCInterruptSummaryMask;
                GT_U32    PFCParityInterruptCause;
                GT_U32    PFCCntrsOverUnderFlowInterruptMask;
                GT_U32    PFCParityInterruptMask;
                GT_U32    PFCFIFOsOverrunsInterruptCause;
                GT_U32    PFCFIFOsOverrunsInterruptMask;
                GT_U32    PFCMsgsSetsOverrunsInterruptCause;
                GT_U32    PFCMsgsSetsOverrunsInterruptMask;
                GT_U32    PFCCntrsOverUnderFlowInterruptCause;

            }PFCInterrupts;

            GT_U32    LLFCTargetPortRemap[144]/*Port div 4*/;
            GT_U32    portFCMode[16]/*Port div 32*/;
            GT_U32    PFCSourcePortToPFCIndexMap[512]/*Global Port*/;
            GT_U32    PFCPortProfile[64]/*Port div 8*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry0[1]/*port group*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[1]/*port group*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[1]/*port group*/;
            GT_U32    PFCPortGroupEqFIFOOverrunCntr[1]/*port group*/;
            GT_U32    PFCPortGroupCntrsStatus[1]/*port group*/;
            GT_U32    PFCPortGroupCntrsParityErrorsCntr[1]/*port group*/;
            GT_U32    PFCGroupOfPortsTCXonThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCXoffThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCDropThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCCntr[1][8]/*port group*//*tc*/;
            GT_U32    PFCTriggerGlobalConfig;
         /* GT_U32    FCModeProfileTCXOffThresholds[64] - new registers. CPSS use CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E with the same format and number of entries */
         /* GT_U32    FCModeProfileTCXOnThresholds[64] - new registers. CPSS use CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E with the same format and number of entries */
            struct /*DBA - dynamic buffer allocation*/
            {
                GT_U32 PFCProfileTCAlphaThresholds[8][8]/*profile*//*TC*/;
                GT_U32 PFCAvailableBuffers;
                GT_U32 PFCDynamicBufferAllocationDisable;
            }dba;
        }pfc;

        struct /*qcn*/
        {

            GT_U32    CCFCSpeedIndex[72]/*Reg offset*/;
            GT_U32    CNSampleTbl[8]/*qFb 3 msb*/;
            GT_U32    profilePriorityQueueCNThreshold[16][8]/*profile*//*priority*/;
            GT_U32    feedbackMIN;
            GT_U32    feedbackMAX;
            GT_U32    CNGlobalConfig;
            GT_U32    enFCTriggerByCNFrameOnPort[16]/*Port div 32*/;
            GT_U32    enCNFrameTxOnPort[16]/*Port div 32*/;
            GT_U32    feedbackCalcConfigs;
            GT_U32    txqECCConf;
            GT_U32    descECCSingleErrorCntr;
            GT_U32    descECCDoubleErrorCntr;
            GT_U32    CNTargetAssignment;
            GT_U32    CNDropCntr;
            GT_U32    QCNInterruptCause;
            GT_U32    QCNInterruptMask;
            GT_U32    CNDescAttributes;
            GT_U32    CNBufferFIFOParityErrorsCntr;
            GT_U32    CNBufferFIFOOverrunsCntr;
            GT_U32    ECCStatus;
            GT_U32    cnAvailableBuffers;

        }qcn;

        struct /*queue*/
        {
            struct /*tailDrop*/{

                struct /*tailDropLimits*/{

                    GT_U32    poolBufferLimits[8]/*Priority*/;
                    GT_U32    poolDescLimits[8]/*Priority*/;
                    GT_U32    profilePortDescLimits[16]/*profile*/;
                    GT_U32    profilePortBufferLimits[16]/*profile*/;
                    GT_U32    globalDescsLimit;
                    GT_U32    globalBufferLimit;

                }tailDropLimits;

                struct /*tailDropCntrs*/{

                    GT_U32    portBuffersCntr[576]/*Port*/; /* for sip5.20 and above - 576, else 72 */
                    GT_U32    portDescCntr[576]/*Port*/;    /* for sip5.20 and above - 576, else 72 */
                    GT_U32    priorityDescCntr[8]/*TC*/;
                    GT_U32    priorityBuffersCntr[8]/*TC*/;
                    GT_U32    totalDescCntr;
                    GT_U32    totalBuffersCntr;

                }tailDropCntrs;

                struct /*tailDropConfig*/{

                    GT_U32    portTailDropCNProfile[576]/*Port*/;   /* for sip5.20 and above - 576, else 72 */
                    GT_U32    portEnqueueEnable[576]/*Port*/;       /* for sip5.20 and above - 576, else 72 */
                    GT_U32    profileTailDropConfig[16]/*Profile*/;
                    GT_U32    profilePriorityQueueToPoolAssociation[16]/*profile*/;
                    GT_U32    profilePriorityQueueEnablePoolUsage[8]/*Priority*/;
                    GT_U32    profilePoolWRTDEn[16]/*profile*/;
                    GT_U32    profileQueueWRTDEn[16]/*profile*/;
                    GT_U32    profilePortWRTDEn[16]/*profile*/;
                    GT_U32    profileMaxQueueWRTDEn[16]/*profile*/;
                    GT_U32    profileECNMarkEn[16]/*Profile*/;
                    GT_U32    profileCNMTriggeringEnable[16]/*Profile*/;
                    GT_U32    globalTailDropConfig;
                    GT_U32    pktLengthForTailDropEnqueue;
                    GT_U32    maxQueueWRTDMasks;
                    GT_U32    portWRTDMasks;
                    GT_U32    queueWRTDMasks;
                    GT_U32    poolWRTDMasks;
                    GT_U32    dynamicAvailableBuffers;

                }tailDropConfig;

                struct /*resourceHistogram*/{

                    struct /*resourceHistogramLimits*/{

                        GT_U32    resourceHistogramLimitReg1;
                        GT_U32    resourceHistogramLimitReg2;

                    }resourceHistogramLimits;

                    struct /*resourceHistogramCntrs*/{

                        GT_U32    resourceHistogramCntr[4]/*counter*/;

                    }resourceHistogramCntrs;

                }resourceHistogram;

                struct /*mcFilterLimits*/{

                    GT_U32    mirroredPktsToAnalyzerPortDescsLimit;
                    GT_U32    ingrMirroredPktsToAnalyzerPortBuffersLimit;
                    GT_U32    egrMirroredPktsToAnalyzerPortBuffersLimit;
                    GT_U32    mcDescsLimit;
                    GT_U32    mcBuffersLimit;

                }mcFilterLimits;

                struct /*muliticastFilterCntrs*/{

                    GT_U32    snifferDescsCntr;
                    GT_U32    ingrSnifferBuffersCntr;
                    GT_U32    egrSnifferBuffersCntr;
                    GT_U32    mcDescsCntr;
                    GT_U32    mcBuffersCntr;

                }muliticastFilterCntrs;

                struct /*FCUAndQueueStatisticsLimits*/{

                    GT_U32    XONLimit[8]/*SharedLimit*/;
                    GT_U32    XOFFLimit[8]/*SharedLimit*/;
                    GT_U32    queueXONLimitProfile[16]/*Profile*/;
                    GT_U32    queueXOFFLimitProfile[16]/*Profile*/;
                    GT_U32    portXONLimitProfile[16]/*Profile*/;
                    GT_U32    portXOFFLimitProfile[16]/*Profile*/;
                    GT_U32    globalXONLimit;
                    GT_U32    globalXOFFLimit;
                    GT_U32    FCUMode;

                }FCUAndQueueStatisticsLimits;

            }tailDrop;

            struct /*global*/{
                GT_U32  portBufferStat;
                GT_U32  queueBufferStat;

                struct /*interrupt*/{

                    struct /*queueStatistics*/{

                        GT_U32    lowCrossingStatusReg[18]/*Index*/;
                        GT_U32    highCrossingInterruptMaskPort[72]/*Port*/;
                        GT_U32    lowCrossingInterruptSummaryMaskPorts61To31;
                        GT_U32    lowCrossingInterruptSummaryMaskPorts30To0;
                        GT_U32    lowCrossingInterruptSummaryCausePorts71To62;
                        GT_U32    lowCrossingInterruptSummaryCausePorts61To31;
                        GT_U32    lowCrossingInterruptSummaryCausePorts30To0;
                        GT_U32    lowCrossingInterruptMaskPort[72]/*Port*/;
                        GT_U32    lowCrossingInterruptCausePort[72]/*Port*/;
                        GT_U32    highCrossingStatusReg[18]/*Index*/;
                        GT_U32    highCrossingInterruptSummaryMaskPorts71To62;
                        GT_U32    lowCrossingInterruptSummaryMaskPorts71To62;
                        GT_U32    highCrossingInterruptSummaryCausePorts30To0;
                        GT_U32    highCrossingInterruptSummaryMaskPorts30To0;
                        GT_U32    highCrossingInterruptSummaryCausePorts61To31;
                        GT_U32    highCrossingInterruptSummaryMaskPorts61To31;
                        GT_U32    highCrossingInterruptSummaryCausePorts71To62;
                        GT_U32    highCrossingInterruptCausePort[72]/*Port*/;

                    }queueStatistics;

                    GT_U32    transmitQueueInterruptSummaryCause;
                    GT_U32    transmitQueueInterruptSummaryMask;
                    GT_U32    generalInterrupt1Cause;
                    GT_U32    generalInterrupt1Mask;
                    GT_U32    portDescFullInterruptCause0;
                    GT_U32    portDescFullInterruptMask0;
                    GT_U32    portDescFullInterruptCause1;
                    GT_U32    portDescFullInterruptMask1;
                    GT_U32    portDescFullInterruptCause2;
                    GT_U32    portDescFullInterruptMask2;
                    GT_U32    portBufferFullInterruptCause0;
                    GT_U32    portBufferFullInterruptMask0;
                    GT_U32    portBufferFullInterruptCause1;
                    GT_U32    portBufferFullInterruptMask1;
                    GT_U32    portBufferFullInterruptCause2;
                    GT_U32    portBufferFullInterruptMask2;

                }interrupt;

                struct /*ECCCntrs*/{

                    GT_U32    TDClrECCErrorCntr;

                }ECCCntrs;

                struct /*dropCntrs*/{

                    GT_U32    clearPktsDroppedCntrPipe[4]/*Pipe*/;

                }dropCntrs;

                struct /*Debug Bus */{
                    GT_U32    queueMetalFix;
                }debugBus;

            }global;

            struct /*peripheralAccess*/{

                struct /*peripheralAccessMisc*/{

                    GT_U32    portGroupDescToEQCntr[4]/*port group*/;
                    GT_U32    peripheralAccessMisc;
                    GT_U32    QCNIncArbiterCtrl;

                }peripheralAccessMisc;

                struct /*egrMIBCntrs*/{

                    GT_U32    txQMIBCntrsPortSetConfig[2]/*Set*/;
                    GT_U32    txQMIBCntrsSetConfig[2]/*Set*/;
                    GT_U32    setTailDroppedPktCntr[2]/*Set*/;
                    GT_U32    setOutgoingUcPktCntr[2]/*Set*/;
                    GT_U32    setOutgoingMcPktCntr[2]/*Set*/;
                    GT_U32    setOutgoingBcPktCntr[2]/*Set*/;
                    GT_U32    setMcFIFO7_4DroppedPktsCntr[2]/*Set*/;
                    GT_U32    setMcFIFO3_0DroppedPktsCntr[2]/*Set*/;
                    GT_U32    setCtrlPktCntr[2]/*Set*/;
                    GT_U32    setBridgeEgrFilteredPktCntr[2]/*Set*/;

                }egrMIBCntrs;

                struct /*CNCModes*/{

                    GT_U32    CNCModes;

                }CNCModes;

            }peripheralAccess;

        }queue;

        struct /*bmx*/
        {

            GT_U32    almostFullDebugRegister;
            GT_U32    fillLevelDebugRegister;   /*Fill Level Debug Register*/

        }bmx;

    }TXQ;

    struct /*TMDROP*/
    {
        GT_U32    portsInterface[8]/*32 ports*/;
        GT_U32    TMDROPTMTCMap8To87[2]/*8 TM TC*/;
        GT_U32    globalConfigs;
        GT_U32    TMDROPInterruptCause;

        GT_U32    TMDROPInterruptMask;
    }TMDROP;

    struct /*TMQMAP*/
    {
        GT_U32    TC2TCMap8To16[6]/*8 TC2TC addresses*/;
        GT_U32    TMQMAPGlobalConfig;
        GT_U32    TMQMAPInterruptCause;
        GT_U32    TMQMAPInterruptMask;

    }TMQMAP;

    struct /*TM_INGR_GLUE*/{

        GT_U32    globalConfig;
        GT_U32    debug;
        GT_U32    TMIngrGlueInterruptCause;
        GT_U32    TMIngrGlueInterruptMask;

    }TM_INGR_GLUE;

    struct /*TM_EGR_GLUE*/
    {
        GT_U32    statisticalAgingTimeCntrMsb[100]/*statistical aging set*/;
        GT_U32    statisticalAgingTimeCntrLsb[100]/*statistical aging set*/;
        GT_U32    TMEgrGlueInterruptMask;
        GT_U32    TMEgrGlueInterruptCause;
        GT_U32    statisticalMinAgeTime[100]/*statistical aging set*/;
        GT_U32    statisticalMinAgeTimeShadow;
        GT_U32    statisticalMaxAgeTime[100]/*statistical aging set*/;
        GT_U32    statisticalMaxAgeTimeShadow;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptMask0;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptMask1;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptMask2;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptMask3;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptCause0;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptCause1;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptCause2;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptCause3;
        GT_U32    statisticalAgingTimeCntrsMsbShadow;
        GT_U32    statisticalAgingTimeCntrsLsbShadow;
        GT_U32    TMEgrGlueSummaryCause;
        GT_U32    TMEgrGlueSummaryMask;
        GT_U32    statisticalAgingQueueID[100]/*statistical aging set*/;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptMask0;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptMask1;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptMask2;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptMask3;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptCause0;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptCause1;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptCause2;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptCause3;
        GT_U32    statisticalAgingPktCntr[100]/*statistical aging set*/;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptMask0;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptMask1;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptMask2;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptMask3;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptCause0;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptCause1;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptCause2;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptCause3;
        GT_U32    statisticalAgingOctetCntrsMsbShadow;
        GT_U32    statisticalAgingOctetCntrsLsbShadow;
        GT_U32    statisticalAgingOctetCntrMsb[100]/*statistical aging set*/;
        GT_U32    statisticalAgingOctetCntrLsb[100]/*statistical aging set*/;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptMask0;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptMask1;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptMask2;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptMask3;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptCause0;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptCause1;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptCause2;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptCause3;
        GT_U32    statisticalAgingAgedOutPktCntrsShadow;
        GT_U32    statisticalAgingAgedOutPktCntr[100]/*statistical aging set*/;
        GT_U32    queueTimerConfig;
        GT_U32    queueTimer;
        GT_U32    portInterface0;
        GT_U32    portInterface1;
        GT_U32    portInterface2;
        GT_U32    perPortAgedOutPktCntr[192]/*TmPort*/;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask0;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask1;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask2;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask3;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask4;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask5;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask6;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause0;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause1;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause2;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause3;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause4;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause5;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause6;
        GT_U32    globalConfig;
        GT_U32    ECCStatus;
        GT_U32    dropPortAllOutgoingPkts[192]/*TmPort*/;
        GT_U32    dropAgedPktCntrConfig;
        GT_U32    dropAgedPktCntr;
        GT_U32    debug;

    }TM_EGR_GLUE;

    struct /*TM_FCU*/
    {
        GT_U32    TMEgrFlowCtrlCntrs[64]/*word*/;
        GT_U32    TMEgrFlowCtrlXOFFThresholds[64]/*word*/;
        GT_U32    TMEgrFlowCtrlXONThresholds[64]/*word*/;
        GT_U32    TMFCUGlobalConfigs;

    }TM_FCU;

    /* Added manually */
    struct /* MG */{

        struct /*XSMI*/{

            GT_U32 XSMIManagement;
            GT_U32 XSMIAddress;
            GT_U32 XSMIConfiguration;
            GT_U32 XSMIInterruptCause;
            GT_U32 XSMIInterruptMask;

        }XSMI[2];
        struct /*addressDecoding*/
        {
            GT_U32 baseAddress[6];
            GT_U32 windowSize[6];
            GT_U32 highAddressRemap[6];
            GT_U32 windowControl[6];

            GT_U32 UnitDefaultID;
        }addressDecoding;
        struct /*confiProcessor*/
        {
            GT_U32 memoryBase;
        }confiProcessor;
    }MG;

    struct PRV_CPSS_DXCH_MAC_PACKET_GEN_VER1_CONFIG_STC
    {
        GT_U32 macDa[3];
        GT_U32 macSa[3];
        GT_U32 etherType;
        GT_U32 vlanTag;
        GT_U32 packetLength;
        GT_U32 packetCount;
        GT_U32 ifg;
        GT_U32 macDaIncrementLimit;
        GT_U32 controlReg0;
        GT_U32 readAndWriteData;
        GT_U32 readAndWriteControl;
        GT_U32 dataPattern[32];
    }packGenConfig[74]; /* one packet generator per port for sip_5_15 (exclude sip_5_20); */
                        /* for other devices one packet generators per port group         */

    struct PRV_CPSS_DXCH_RX_DMA_GLUE_STCT
    {
        PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STC pizzaArbiter;
    }RX_DMA_GLUE;

    struct PRV_CPSS_DXCH_TX_DMA_GLUE_STCT
    {
        PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STC pizzaArbiter;
    }TX_DMA_GLUE;

    struct PRV_CPSS_DXCH_MPPM_STCT
    {
        PRV_CPSS_DXCH_PP_MPPM_PIZZA_ARBITER_STC pizzaArbiter;
        struct {
            GT_U32 statusFailedSyndrome;
            GT_U32 lastFailingBuffer;
            GT_U32 lastFailingSegment;
        } dataIntegrity;

    }MPPM;

    struct PRV_CPSS_DXCH_BMA_STCT
    {
        /* MCCNT Parity Error Counter1 Register */
        GT_U32  mcCntParityErrorCounter1;
    } BMA;

    struct PRV_CPSS_DXCH_EREP_STCT
    {
        struct
        {
            GT_U32 pchHeaderEnableBase; /*1024 registers*/
            GT_U32 replicationsArbiterConfigs;
            GT_U32 qcnConfigs0;
            GT_U32 qcnConfigs1;
            GT_U32 dropCodeConfigs;
            GT_U32 hbuArbiterConfigs;
            GT_U32 erepGlobalConfigs;
            GT_U32 dropCodeStatus;
        } configurations;
        struct
        {
            GT_U32 qcnOutgoingCounter;
            GT_U32 sniffOutgoingCounter;
            GT_U32 trapOutgoingCounter;
            GT_U32 mirrorOutgoingCounter;
            GT_U32 qcnFifoFullDroppedPacketsCounter;
            GT_U32 sniffFifoFullDroppedPacketsCounter;
            GT_U32 trapFifoFullDroppedPacketsCounter;
            GT_U32 mirrorFifoFullDroppedPacketsCounter;
            GT_U32 erepHeaderExceedCounter;
            GT_U32 incNotReadyCounter;
            GT_U32 mirrorReplicationCancelledDueIncBackpressure;
            GT_U32 sniffReplicationCancelledDueIncBackpressure;
            GT_U32 trapReplicationCancelledDueIncBackpressure;
            GT_U32 hbuPacketsOutgoingForwardCounter;
        } counters;
        struct
        {
            GT_U32 erepInterruptsCause;
            GT_U32 erepInterruptsMask;
        } interrupts;
    } EREP;

    struct PRV_CPSS_DXCH_PRE_Q_STCT
    {
        GT_U32 globalConfig;
        GT_U32 globalTailDropLimit;
        GT_U32 pool0TailDropLimit;
        GT_U32 pool1TailDropLimit;
        GT_U32 mcAvailableBuffers;
        GT_U32 mcTDConfigurationLowPrio;
        GT_U32 mcTDConfigurationHighPrio;
        GT_U32 tdPool0AvailableBuffers;
        GT_U32 tdPool1AvailableBuffers;
        GT_U32 globalTailDropConfig;
        GT_U32 queueMaxLimitConfig;
        GT_U32 globalQcnConfig;
        GT_U32 qcnPool0AvailableBuffers;
        GT_U32 qcnPool1AvailableBuffers;
        GT_U32 fbMin;
        GT_U32 fbMax;
        GT_U32 qcnTrigger[32];
        GT_U32 mirroringGlobalConfig;
        GT_U32 portMapping[128];
        struct /*egrMIBCntrs*/
        {
            GT_U32    egrMIBCntrsPortSetConfig[2]/*Set*/;
            GT_U32    egrMIBCntrsSetConfig[2]/*Set*/;
            GT_U32    egrMIBTailDroppedPktCntr[2]/*Set*/;
            GT_U32    egrMIBOutgoingUcPktCntr[2]/*Set*/;
            GT_U32    egrMIBOutgoingMcPktCntr[2]/*Set*/;
            GT_U32    egrMIBOutgoingBcPktCntr[2]/*Set*/;
            GT_U32    egrMIBCtrlPktCntr[2]/*Set*/;
        } egrMIBCntrs;
    } PREQ; /* sip6 only */

    struct /*sip6_rxDMA*/{

        struct /*status*/{

            GT_U32    pktDropReport;

        }status;

        struct /*configs*/{

            struct /*rejectConfigs*/{

                GT_U32    rejectPktCommand;
                GT_U32    rejectCPUCode;

            }rejectConfigs;

            struct /*dropThresholds*/{

                GT_U32    lowPrioDropThreshold0;
                GT_U32    lowPrioDropThreshold1;
                GT_U32    lowPrioDropMapConfig;

            }dropThresholds;

            struct /*dataAccumulator*/{

                GT_U32    PBFillLevelThresholds;
                GT_U32    maxTailBC;

            }dataAccumulator;

            struct /*cutThrough*/{

                GT_U32    CTGenericConfigs[4]/*Generic*/;
                GT_U32    channelCTConfig[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    CTGeneralConfig;
                GT_U32    CTErrorConfig;
                GT_U32    CTUPEnable;

            }cutThrough;

            struct /*channelConfig*/{

                GT_U32    channelToLocalDevSourcePort[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    PCHConfig[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    channelGeneralConfigs[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    channelCascadePort[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;

            }channelConfig;

        }configs;

        struct /*DFX*/{

            GT_U32    ECCErrorInjection;
            GT_U32    ECCErrorMode;

        }DFX;

        struct /*debug*/{

            GT_U32    headerMemoryFullDrop0;
            GT_U32    headerMemoryFullDrop1;

        }debug;

    }sip6_rxDMA[MAX_DP_CNS]/*RXDMA_UNIT*/;

    struct /*LMU*/ {
        GT_U32 averageCoefficient;
        GT_U32 profileStatisticsReadData[6];
        GT_U32 latencyOverThreshold_cause[17];
        GT_U32 latencyOverThreshold_mask[17];
        GT_U32 summary_cause;
        GT_U32 summary_mask;
        GT_U32 channelEnable;
    } LMU[MAX_TILE_CNS * FALCON_RAVENS_PER_TILE * 2];

    struct /* Packet Buffer DIP7 only (SIP6)*/
    {
        struct /* PB_CENTER */
        {
            GT_U32 npmRandomizerEnable;
            GT_U32 tileId;
            GT_U32 tileMode;
        } pbCenter;
        struct /* GPC_GRP_PACKET_WRITE */
        {
            /* each related to 4 GPC_PACKET_WRITE */
            GT_U32 npmRandomizerEnable;
        } gpcGrpPacketWrite[MAX_DP_PER_TILE_CNS / 4];
        struct /* GPC_PACKET_WRITE */
        {
            GT_U32 cfgEnable;
        } gpcPacketWrite[MAX_DP_PER_TILE_CNS];
        struct /* GPC_GRP_PACKET_READ */
        {
            /* each related to 2 GPC_PACKET_READ */
            GT_U32 badAddress;
        } gpcGrpPacketRead[MAX_DP_PER_TILE_CNS / 2];
        struct /* GPC_PACKET_READ */
        {
            GT_U32 channelEnable[10];
            GT_U32 channelReset[10];
            GT_U32 channelShaper[10];
            GT_U32 packetCountEnable;
            GT_U32 packetCountReset;
            GT_U32 fifoSegmentStart[10];
            GT_U32 fifoSegmentNext[20];
            GT_U32 cfgUpdateTrigger;
            GT_U32 arbiterPointHighSpeedChannel0En;
            GT_U32 arbiterPointGroup1[5];
            GT_U32 arbiterPointGroup2[5];
            GT_U32 arbiterCalendarSlot[32];
            GT_U32 arbiterCalendarMax;
        } gpcPacketRead[MAX_DP_PER_TILE_CNS];
        struct /* GPC_GRP_CELL_READ */
        {
            /* each related to 2 GPC_CELL_READ */
            GT_U32 badRegister;
        } gpcGrpCellRead[MAX_DP_PER_TILE_CNS / 2];
        struct /* GPC_CELL_READ */
        {
            GT_U32 initialization;
        } gpcCellRead[MAX_DP_PER_TILE_CNS];
        struct /* NPM_MC */
        { /* not ready*/
            GT_U32 cfgEnable;
            GT_U32 cfgUnit;
            GT_U32 cfgRefs;
            GT_U32 insList[4];
            GT_U32 intReinsertMask;
        } npmMc[3];
        struct /* SMB_MC */
        {
            GT_U32 cfgEnable;
        } smbMc[6];
        struct /* SMB_WRITE_ARBITER */
        {
            GT_U32 cfgEnable;
        } smbWriteArbiter;
        struct /* PB_COUNTER */
        { /* not ready */
            GT_U32 enablePbc;
            GT_U32 enableMasterMode;
            GT_U32 enableInterTileTransact;
            GT_U32 cfgTile;
            GT_U32 resizeSmbCells;
            GT_U32 resizeSmbMcCells;
            GT_U32 resizeSmbSumCells;
            GT_U32 resizeNpmWords;
            GT_U32 resizeNpmMcWords;
            GT_U32 resizeNpmSumWords;
            GT_U32 cfgFillThreshold;
            /* Counters of currently allocated cells/next-pointers             */
            /* Locals means in current tile, otherwize - global - in all tiles */
            /* Diff means difference between maximal and minimal value         */
            /* OvertimeMax and  OvertimeMin means Maximal or Minimal from the  */
            /* previous reading of the given counter                           */
            GT_U32 dbgSmbLocalMax;
            GT_U32 dbgSmbLocalMin;
            GT_U32 dbgSmbLocalSum;
            GT_U32 dbgSmbLocalMcSum;
            GT_U32 dbgNpmLocalMax;
            GT_U32 dbgNpmLocalMin;
            GT_U32 dbgNpmLocalSum;
            GT_U32 dbgNpmLocalMcSum;
            GT_U32 dbgSmbMax;
            GT_U32 dbgSmbMin;
            GT_U32 dbgSmbSum;
            GT_U32 dbgSmbMcSum;
            GT_U32 dbgSmbUcSum;
            GT_U32 dbgNpmMax;
            GT_U32 dbgNpmMin;
            GT_U32 dbgNpmSum;
            GT_U32 dbgNpmMcSum;
            GT_U32 dbgNpmUcSum;
            GT_U32 dbgSmbDiff;
            GT_U32 dbgNpmDiff;
            GT_U32 dbgSmbOvertimeMaxSum;
            GT_U32 dbgSmbOvertimeMaxMcSum;
            GT_U32 dbgSmbOvertimeMaxUcSum;
            GT_U32 dbgSmbOvertimeMinSum;
            GT_U32 dbgSmbOvertimeMinMcSum;
            GT_U32 dbgSmbOvertimeMinUcSum;
            GT_U32 dbgSmbOvertimeMaxDiff;
            GT_U32 dbgSmbOvertimeMinDiff;
            GT_U32 dbgNpmOvertimeMaxSum;
            GT_U32 dbgNpmOvertimeMaxMcSum;
            GT_U32 dbgNpmOvertimeMaxUcSum;
            GT_U32 dbgNpmOvertimeMinSum;
            GT_U32 dbgNpmOvertimeMinMcSum;
            GT_U32 dbgNpmOvertimeMinUcSum;
            GT_U32 dbgNpmOvertimeMaxDiff;
            GT_U32 dbgNpmOvertimeMinDiff;
        } pbCounter;
    } sip6_packetBuffer;

     struct /*sip6_txDMA*/{

         struct /*configs*/{

             struct /*globalConfigs*/{

                GT_U32    txDMAGlobalConfig;
                PRV_CPSS_DXCH_PP_DP_TX_PIZZA_ARBITER_STCT     pizzaArbiter;

             }globalConfigs;

             struct /*speedProfileConfigurations*/{

                 GT_U32    descFIFODepth[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    sdqMaxCredits[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    sdqThresholdBytes[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    maxWordCredits[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    maxCellsCredits[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    maxDescCredits[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    interPacketRateLimiter[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    interCellRateLimiter[MAX_SPEED_PROFILE_NUM_CNS];

             }speedProfileConfigs;

             struct /*channelConfigurations*/{

                  GT_U32    speedProfile[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    channelReset[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    descFIFOBase[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    interPacketRateLimiterConfig[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    interCellRateLimiterConfig[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    inerPacketRateLimiterConfig[PRV_CPSS_MAX_DMA_NUM_CNS];

             }channelConfigs;

         }configs;

         struct /*TDSConfigs*/{

              GT_U32    globalConfig;
              GT_U32    channelReset;
              GT_U32    cellRateLimiterConfig;
              GT_U32    cellRateLimiter;

          }TDSConfigs;


     }sip6_txDMA[MAX_DP_CNS]/*TXDMA_UNIT*/;

     struct /*sip6_txFIFO*/{

         struct /*configs*/{

                 struct /*globalConfigs*/{

                 GT_U32    globalConfig1;
                 GT_U32    packetsMaximalReadLatencyFromPacketPuffer;
                 GT_U32    wordBufferCmn8wr4rdBufferConfig;
                 PRV_CPSS_DXCH_PP_DP_TX_PIZZA_ARBITER_STCT     pizzaArbiter;

             }globalConfigs;

             struct /*speedProfileConfs*/{

                 GT_U32    descFIFODepth[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    tagFIFODepth[MAX_SPEED_PROFILE_NUM_CNS];

             }speedProfileConfigs;

              struct /*ChannelConfigs*/{

                  GT_U32    speedProfile[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    descFIFOBase[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    tagFIFOBase[PRV_CPSS_MAX_DMA_NUM_CNS];

              }channelConfigs;

          }configs;

           struct /*debug*/{

               GT_U32   statusBadAddr;
           }debug;

     }sip6_txFIFO[MAX_DP_CNS]/*TXFIFO_UNIT*/;

    struct /*SHM*/{

        GT_U32    block_port_select[40]/*SBM number*/;
        GT_U32    shm_engines_size_mode;
        GT_U32    bad_address_latch;
        GT_U32    shm_error_sum_cause;
        GT_U32    shm_error_sum_mask;
        GT_U32    lpm_illegal_address;
        GT_U32    fdb_illegal_address;
        GT_U32    em_illegal_address;
        GT_U32    arp_illegal_address;
        GT_U32    lpm_aging_illegal_address;

    }SHM;/*sip6 shared memories (FDB/EM/ARP-TS/LPM) */

    struct /*EagleD2dCpInRaven*/{
        struct {
            struct {
                GT_U32  cfgTapBroadcastEn;
            }D2D_CP_RF;
        }D2D_CP;
        struct {
            struct {
                struct {
                    struct {
                        GT_U32  BridgeWindowControl;
                        GT_U32  BridgeWindowBase;
                    }Bridge_Windows[8];
                }AXI_to_MBUS_Bridge;
            }D2D_CP_A2M;
        }D2D_CP_AMB;
    }EagleD2dCpInRaven[MAX_TILE_CNS];/*we have D2D-CP , 1 per tile (there are 2 per 'dual tile') */

    struct{
        struct /*pha_regs*/{

            struct /*PPAThreadConfigs*/{

                GT_U32    nullProcessingInstructionPointer;

            }PPAThreadConfigs;

            struct /*generalRegs*/{

                GT_U32    PHACtrl;
                GT_U32    PHAStatus;

            }generalRegs;

            struct /*errorsAndInterrupts*/{

                GT_U32    headerModificationViolationConfig;
                GT_U32    PPAClockDownErrorConfig;
                GT_U32    headerModificationViolationAttributes;
                GT_U32    PHABadAddrLatch;
                GT_U32    PHAInternalErrorCause;
                GT_U32    PHAInternalErrorMask;
                GT_U32    PHAInterruptSumCause;
                GT_U32    PHAInterruptSumMask;

            }errorsAndInterrupts;

            struct /*debugCounters*/{
                GT_U32    statisticalProcessingDebugCounterConfig;
                GT_U32    statisticalProcessingDebugCounter;
            }debugCounters;

        }pha_regs;

        struct /*PPA*/{

            struct /*ppa_regs*/{

                GT_U32    PPACtrl;
                GT_U32    PPAInterruptSumCause;
                GT_U32    PPAInterruptSumMask;
                GT_U32    PPAInternalErrorCause;
                GT_U32    PPAInternalErrorMask;
                GT_U32    PPABadAddrLatch;

            }ppa_regs;

        }PPA;

        struct /*PPG[3]*/{
            struct /*ppg_regs*/{

                GT_U32    PPGInterruptSumCause;
                GT_U32    PPGInterruptSumMask;
                GT_U32    PPGInternalErrorCause;
                GT_U32    PPGInternalErrorMask;
                GT_U32    PPGBadAddrLatch;

            }ppg_regs;

            struct /*PPN[9]*/{
                struct /*ppn_regs*/{
                    GT_U32    fwName;     /* hold 4 char of 'name' like : FLCN (Falcon) */
                    GT_U32    fwVersion;  /* hold 4 U8 of : year,month,version,debugValue (18 06 00 00 , means 2018-June ver00 debug00)*/
                    GT_U32    TODWord0[2]/*tod IF*/;
                    GT_U32    TODWord1[2]/*TOD IF*/;
                    GT_U32    TODWord2[2]/*TOD IF*/;
                    GT_U32    PPNDoorbell;
                    GT_U32    pktHeaderAccessOffset;
                    GT_U32    PPNInternalErrorCause;
                    GT_U32    PPNInternalErrorMask;
                    GT_U32    PPNBadAddrLatch;

                }ppn_regs;
            }PPN[9];

            /* manually added */
            GT_U32      PPG_IMEM_base_addr;/* base address of this memory (4 words per entry) */
        }PPG[3];

    }PHA;/* sip6 PHA unit */


}PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC;
/* restore alignment setting */
#pragma pack(pop)

/**
* @internal prvCpssDxChNonSip5HwRegAddrVer1Init function
* @endinternal
*
* @brief   This function initializes the registers structure for pre eArch devices.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDxChNonSip5HwRegAddrVer1Init
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChHwRegAddrVer1Init function
* @endinternal
*
* @brief   This function initializes the registers struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrVer1Init
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChHwResetAndInitControllerRegAddrInit function
* @endinternal
*
* @brief   This function initializes the Reset and Init Controller registers
*         struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwResetAndInitControllerRegAddrInit
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChXcat3HwResetAndInitControllerRegAddrInit function
* @endinternal
*
* @brief   This function initializes the Reset and Init Controller (DFX) registers
*         struct for xCat3 devices.
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3HwResetAndInitControllerRegAddrInit
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChCgPortDbInvalidate function
* @endinternal
*
* @brief   Invalidate or init CG port register database
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - CG MAC port number
* @param[in] cgUnitInvalidate         - invalidate status
*                                      GT_TRUE - invalidate DB for given MAC port
*                                      GT_FALSE - init DB for given MAC port
*                                       None
*/
GT_STATUS prvCpssDxChCgPortDbInvalidate
(
    IN  GT_U8 devNum,
    IN  GT_U32 portNum,
    IN  GT_BOOL cgUnitInvalidate
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChRegsVer1h */

