/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
	used to endorse or promote products derived from this software without
	specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#ifndef __INCmvCtrlEnvRegsh
#define __INCmvCtrlEnvRegsh

#include "mvCtrlEnvSpec.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* CV Support */
#define PEX0_MEM0 	PEX0_MEM
#define PCI0_MEM0	PEX0_MEM

/* Controller revision info */
#define PCI_CLASS_CODE_AND_REVISION_ID		0x008
#define PCCRIR_REVID_OFFS			0		/* Revision ID */
#define PCCRIR_REVID_MASK			(0xff << PCCRIR_REVID_OFFS)


/* Coherent Fabric Control and Status */
#define MV_COHERENCY_FABRIC_CTRL_REG		(MV_COHERENCY_FABRIC_OFFSET + 0x0)
#define MV_COHERENCY_FABRIC_CFG_REG		(MV_COHERENCY_FABRIC_OFFSET + 0x4)

/* CIB registers offsets */
#define MV_CIB_CTRL_CFG_REG			(MV_COHERENCY_FABRIC_OFFSET + 0x80)

/* PMU_NFABRIC PMU_NFABRIC PMU_UNIT_SERVICE Units */
#define MV_L2C_NFABRIC_PM_CTRL_CFG_REG		(MV_PMU_NFABRIC_UNIT_SERV_OFFSET + 0x4)
#define MV_L2C_NFABRIC_PM_CTRL_CFG_PWR_DOWN	(1 << 20)

#define MV_L2C_NFABRIC_PWR_DOWN_FLOW_CTRL_REG	(MV_PMU_NFABRIC_UNIT_SERV_OFFSET + 0x8)

#define PM_CONTROL_AND_CONFIG_REG(cpu)		(MV_CPU_PMU_UNIT_SERV_OFFSET(cpu) + 0x4)
#define PM_CONTROL_AND_CONFIG_DFS_REQ		(1 << 18)
#define PM_CONTROL_AND_CONFIG_PWDDN_REQ		(1 << 16)
#define PM_CONTROL_AND_CONFIG_L2_PWDDN		(1 << 20)

#define PM_STATUS_AND_MASK_REG(cpu)		(MV_CPU_PMU_UNIT_SERV_OFFSET(cpu) + 0xc)
#define PM_STATUS_AND_MASK_CPU_IDLE_WAIT	(1 << 16)
#define PM_STATUS_AND_MASK_SNP_Q_EMPTY_WAIT	(1 << 17)
#define PM_STATUS_AND_MASK_IRQ_WAKEUP		(1 << 20)
#define PM_STATUS_AND_MASK_FIQ_WAKEUP		(1 << 21)
#define PM_STATUS_AND_MASK_DBG_WAKEUP		(1 << 22)
#define PM_STATUS_AND_MASK_IRQ_MASK		(1 << 24)
#define PM_STATUS_AND_MASK_FIQ_MASK		(1 << 25)

#define PM_EVENT_STATUS_AND_MASK_REG(cpu)		(MV_CPU_PMU_UNIT_SERV_OFFSET(cpu) + 0x20)
#define PM_EVENT_STATUS_AND_MASK_DFS_DONE_OFFS			1
#define PM_EVENT_STATUS_AND_MASK_DFS_DONE_MASK_OFFS		17

#define PM_CPU_BOOT_ADDR_REDIRECT(cpu)		(MV_CPU_PMU_UNIT_SERV_OFFSET(cpu) + 0x24)


/* TODO - verify, the manual has no description */
#define PMC_CPUSTOPMEM_OFFS(id)			((id) < 4 ? (id) * 3 : 0)
#define PMC_CPUSTOPMEM_MASK(id)			(7 << PMC_CPUSTOPMEM_OFFS(id))
#define PMC_CPUSTOPMEM_EN(id)			(0 << PMC_CPUSTOPMEM_OFFS(id))
#define PMC_CPUSTOPMEM_STOP(id)			(1 << PMC_CPUSTOPMEM_OFFS(id))


/*  Power Management Clock Gating Control Register	*/
#define POWER_MNG_CTRL_REG			0x18220
#define L2C_MTCMOS_CONTROL_0_REG    0x22F00
#define L2C_MTCMOS_CONTROL_1_REG    0x22F04

#define PMC_PEXSTOPCLOCK_OFFS			5
#define PMC_PEXSTOPCLOCK_MASK			(1 << PMC_PEXSTOPCLOCK_OFFS)
#define PMC_PEXSTOPCLOCK_EN			(1 << PMC_PEXSTOPCLOCK_OFFS)
#define PMC_PEXSTOPCLOCK_STOP			(0 << PMC_PEXSTOPCLOCK_OFFS)

#define PMC_SDIOSTOPCLOCK_OFFS			17
#define PMC_SDIOSTOPCLOCK_MASK			(1 << PMC_SDIOSTOPCLOCK_OFFS)
#define PMC_SDIOSTOPCLOCK_EN			(1 << PMC_SDIOSTOPCLOCK_OFFS)
#define PMC_SDIOSTOPCLOCK_STOP			(0 << PMC_SDIOSTOPCLOCK_OFFS)

#define PMC_RUNITSTOPCLOCK_OFFS			24
#define PMC_RUNITSTOPCLOCK_MASK			(1 << PMC_RUNITSTOPCLOCK_OFFS)
#define PMC_RUNITSTOPCLOCK_EN			(1 << PMC_RUNITSTOPCLOCK_OFFS)
#define PMC_RUNITSTOPCLOCK_STOP			(0 << PMC_RUNITSTOPCLOCK_OFFS)

#define PMC_XORSTOPCLOCK_OFFS			22
#define PMC_XORSTOPCLOCK_MASK			(1 << PMC_XORSTOPCLOCK_OFFS)
#define PMC_XORSTOPCLOCK_EN			(1 << PMC_XORSTOPCLOCK_OFFS)
#define PMC_XORSTOPCLOCK_STOP			(0 << PMC_XORSTOPCLOCK_OFFS)

#define PMC_GESTOPCLOCK_OFFS(port)		(4 - (port))
#define PMC_GESTOPCLOCK_MASK(port)		(1 << PMC_GESTOPCLOCK_OFFS(port))
#define PMC_GESTOPCLOCK_EN(port)		(1 << PMC_GESTOPCLOCK_OFFS(port))
#define PMC_GESTOPCLOCK_STOP(port)		(0 << PMC_GESTOPCLOCK_OFFS(port))

/*  Power Management Clock Gating Control Register - AXP only */

#define AXP_PMC_TDMSTOPCLOCK_OFFS		25
#define AXP_PMC_TDMSTOPCLOCK_MASK		(1 << AXP_PMC_TDMSTOPCLOCK_OFFS)
#define AXP_PMC_TDMSTOPCLOCK_EN			(1 << AXP_PMC_TDMSTOPCLOCK_OFFS)
#define AXP_PMC_TDMSTOPCLOCK_STOP		(0 << AXP_PMC_TDMSTOPCLOCK_OFFS)

#define AXP_PMC_PEXSTOPCLOCK_OFFS(port)		((port) < 8 ? (5 + (port)) : (18 + (port)))
#define AXP_PMC_PEXSTOPCLOCK_MASK(port)		(1 << AXP_PMC_PEXSTOPCLOCK_OFFS(port))
#define AXP_PMC_PEXSTOPCLOCK_EN(port)		(1 << AXP_PMC_PEXSTOPCLOCK_OFFS(port))
#define AXP_PMC_PEXSTOPCLOCK_STOP(port)		(0 << AXP_PMC_PEXSTOPCLOCK_OFFS(port))

#define AXP_PMC_USBSTOPCLOCK_OFFS(port)		((port) < 3 ? (18 + (port)) : 0)
#define AXP_PMC_USBSTOPCLOCK_MASK(port)		(1 << AXP_PMC_USBSTOPCLOCK_OFFS(port))
#define AXP_PMC_USBSTOPCLOCK_EN(port)		(1 << AXP_PMC_USBSTOPCLOCK_OFFS(port))
#define AXP_PMC_USBSTOPCLOCK_STOP(port)		(0 << AXP_PMC_USBSTOPCLOCK_OFFS(port))

#define AXP_PMC_SDIOSTOPCLOCK_OFFS		17
#define AXP_PMC_SDIOSTOPCLOCK_MASK		(1 << AXP_PMC_SDIOSTOPCLOCK_OFFS)
#define AXP_PMC_SDIOSTOPCLOCK_EN		(1 << AXP_PMC_SDIOSTOPCLOCK_OFFS)
#define AXP_PMC_SDIOSTOPCLOCK_STOP		(0 << AXP_PMC_SDIOSTOPCLOCK_OFFS)

#define AXP_PMC_RUNITSTOPCLOCK_OFFS		24
#define AXP_PMC_RUNITSTOPCLOCK_MASK		(1 << AXP_PMC_RUNITSTOPCLOCK_OFFS)
#define AXP_PMC_RUNITSTOPCLOCK_EN		(1 << AXP_PMC_RUNITSTOPCLOCK_OFFS)
#define AXP_PMC_RUNITSTOPCLOCK_STOP		(0 << AXP_PMC_RUNITSTOPCLOCK_OFFS)

#define AXP_PMC_XORSTOPCLOCK_OFFS		22
#define AXP_PMC_XORSTOPCLOCK_MASK		(1 << AXP_PMC_XORSTOPCLOCK_OFFS)
#define AXP_PMC_XORSTOPCLOCK_EN			(1 << AXP_PMC_XORSTOPCLOCK_OFFS)
#define AXP_PMC_XORSTOPCLOCK_STOP		(0 << AXP_PMC_XORSTOPCLOCK_OFFS)

#define AXP_PMC_SATASTOPCLOCK_OFFS(ch)		(ch == 0 ? 14 : 29)
#define AXP_PMC_SATASTOPCLOCK_MASK(ch)		(3 << AXP_PMC_SATASTOPCLOCK_OFFS(ch))
#define AXP_PMC_SATASTOPCLOCK_EN(ch)		(3 << AXP_PMC_SATASTOPCLOCK_OFFS(ch))
#define AXP_PMC_SATASTOPCLOCK_STOP(ch)		(0 << AXP_PMC_SATASTOPCLOCK_OFFS(ch))

#define AXP_PMC_CESASTOPCLOCK_OFFS		23
#define AXP_PMC_CESASTOPCLOCK_MASK		(1 << AXP_PMC_CESASTOPCLOCK_OFFS)
#define AXP_PMC_CESASTOPCLOCK_EN		(1 << AXP_PMC_CESASTOPCLOCK_OFFS)
#define AXP_PMC_CESASTOPCLOCK_STOP		(0 << AXP_PMC_CESASTOPCLOCK_OFFS)

#define AXP_PMC_GESTOPCLOCK_OFFS(port)		((port) < 4 ? (4 - (port)) : 0)
#define AXP_PMC_GESTOPCLOCK_MASK(port)		(1 << AXP_PMC_GESTOPCLOCK_OFFS(port))
#define AXP_PMC_GESTOPCLOCK_EN(port)		(1 << AXP_PMC_GESTOPCLOCK_OFFS(port))
#define AXP_PMC_GESTOPCLOCK_STOP(port)		(0 << AXP_PMC_GESTOPCLOCK_OFFS(port))

#define AXP_PMC_NETASTOPCLOCK_OFFS		13
#define AXP_PMC_NETASTOPCLOCK_MASK		(1 << AXP_PMC_NETASTOPCLOCK_OFFS)
#define AXP_PMC_NETASTOPCLOCK_EN		(1 << AXP_PMC_NETASTOPCLOCK_OFFS)
#define AXP_PMC_NETASTOPCLOCK_STOP		(0 << AXP_PMC_NETASTOPCLOCK_OFFS)

#define AXP_PMC_LCDSTOPCLOCK_OFFS		16
#define AXP_PMC_LCDSTOPCLOCK_MASK		(1 << AXP_PMC_LCDSTOPCLOCK_OFFS)
#define AXP_PMC_LCDSTOPCLOCK_EN			(1 << AXP_PMC_LCDSTOPCLOCK_OFFS)
#define AXP_PMC_LCDSTOPCLOCK_STOP		(0 << AXP_PMC_LCDSTOPCLOCK_OFFS)

#define AXP_PMC_IDMASTOPCLOCK_OFFS		21
#define AXP_PMC_IDMASTOPCLOCK_MASK		(1 << AXP_PMC_IDMASTOPCLOCK_OFFS)
#define AXP_PMC_IDMASTOPCLOCK_EN		(1 << AXP_PMC_IDMASTOPCLOCK_OFFS)
#define AXP_PMC_IDMASTOPCLOCK_STOP		(0 << AXP_PMC_IDMASTOPCLOCK_OFFS)

#define AXP_PMC_DDRSTOPCLOCK_OFFS		28
#define AXP_PMC_DDRSTOPCLOCK_MASK		(1 << AXP_PMC_DDRSTOPCLOCK_OFFS)
#define AXP_PMC_DDRSTOPCLOCK_EN			(1 << AXP_PMC_DDRSTOPCLOCK_OFFS)
#define AXP_PMC_DDRSTOPCLOCK_STOP		(0 << AXP_PMC_DDRSTOPCLOCK_OFFS)

/* dummy defenition, used for SGMII capable interfaces */
#define SGMII_SERDES_CFG_REG(port)		(0)

/* Controler environment registers offsets */

#define MPP_CONTROL_REG(id)			(0x18000 + (id * 4))

/* Switch core registers */
#define MV_PP_ADDCOMP_CNCTRL 0x00000140
#define MV_PP_ADDCOMP_REG(n) (0x00000120 + 4*n)
#define MV_PP_ADDCOMP_MODE_BIT			16
#define MV_PP_ADDCOMP_MODE_MASK			(1 << MV_PP_ADDCOMP_MODE_BIT)

#define MV_PP_SMI_MISC_CONFIG_OFFSET 0x4
#define MV_PP_SMI_INVERT_MDC_BIT          11
#define MV_PP_SMI_INVERT_MDC_MASK         (1 << MV_PP_SMI_INVERT_MDC_BIT)

#define MV_PP_ETH_ADDCOMP_INDEX 1
#define MV_PP_ETH_REG_BASE_NORNAL (SWITCH_REGS_VIRT_BASE | MV_PP_ETH_ADDCOMP_INDEX<<19)
#define MV_PP_ETH_REG_BASE_LEGACY (SWITCH_REGS_VIRT_BASE | MV_PP_ETH_ADDCOMP_INDEX<<24)

/* Sample at Reset - AXP only */
#define MPP_SAMPLE_AT_RESET(id)			(0x18230 + (id * 4))

/* Controller environment and SERDERS registers - AXP only */
#define SERDES_LINE_MUX_REG_0_7			0x18270
#define GEN_PURP_RES_2_REG			0x182F8

/* Dragonite Register */
#define DRAGONITE_CTRL_REG			0x1c
#define DRAGONITE_POE_CAUSE_IRQ_REG		0x64
#define DRAGONITE_POE_MASK_IRQ_REG		0x68
#define DRAGONITE_HOST2POE_IRQ_REG		0x6c
#define DRAGONITE_DEBUGGER_REG			0xF8290

/* Sample at Reset */
#define DFX_DEVICE_SAR_REG(x)			(0xf8200 +(x*4))

#define MSAR_CORE_CLK(sar1, sar2)	(((sar2) >> 21) & 0x7)		/* PLL 0 config */
#define MSAR_CPU_DDR_CLK(sar1, sar2)	(((sar2) >> 18) & 0x7)		/* PLL 1 config */
#define MSAR_DEVICE_MODE(sar1, sar2)	(((sar1) >>  0) & 0xFF)		/* DEVICE ID field */

#define MSAR_BC2_BOOT_MODE(sar1, sar2)	(((sar1) >> 13) & 0x7)		/* boot from */
#define MSAR_BC2_TM_CLK(sar1, sar2)	(((sar2) >> 15) & 0x7)		/* PLL 2 config */

#define MSAR_AC3_BOOT_MODE(sar1, sar2)	(((sar1) >> 11) & 0x7)		/* boot from */
#define MSAR_AC3_TM_CLK(sar1, sar2)	(((sar2) >> 17) & 0x1)		/* PLL 2 config */

#define MSAR_BOBK_BOOT_MODE(sar1, sar2)	(((sar1) >> 13) & 0x7)		/* boot from */
#define MSAR_BOBK_TM_CLK(sar1, sar2)	(((sar2) >> 15) & 0x7)		/* PLL 2 config */

#define SAR1_BOOT_FROM_NOR			0
#define SAR1_BOOT_FROM_NAND			1
#define SAR1_BOOT_FROM_UART			2
#define SAR1_BOOT_FROM_SPI			3
#define SAR1_BOOT_FROM_PCI			4
#define SAR1_BOOT_FROM_NO_BOOT			5
#define SAR1_BOOT_FROM_UART_DBG			6
#define SAR1_BOOT_FROM_DISABLE			7

#define DFX_CORE_DIVCLK_CONTROL0_REG		0xF8268 /*/Cider/EBU/Bobcat2/Bobcat2 {Current}/
							Reset and Init Controller/
								DFX Server Units -
								BC2 specific registers/Device Control 6 */
#define CORE_DIVCLK_RELOAD_FORCE_OFFS		21
#define CORE_DIVCLK_RELOAD_FORCE_MASK		(0x7F << CORE_DIVCLK_RELOAD_FORCE_OFFS)
#define CORE_DIVCLK_RELOAD_FORCE_VAL		(0x40 << CORE_DIVCLK_RELOAD_FORCE_OFFS)

#define DFX_CORE_DIVCLK_RELOAD_REG		0xF8270 /*/Cider/EBU/Bobcat2/Bobcat2 {Current}/
								Reset and Init Controller/
								DFX Server Units - BC2 specific registers/
								Device Control 8	*/

#define CORE_DIVCLK_RELOAD_RATIO_OFFS		10
#define CORE_DIVCLK_RELOAD_RATIO_MASK		(1 << CORE_DIVCLK_RELOAD_RATIO_OFFS)

#define NAND_ECC_DIVCKL_RATIO_OFFS		6
#define NAND_ECC_DIVCKL_RATIO_MASK		(0xF << NAND_ECC_DIVCKL_RATIO_OFFS)
#define NAND_ECC_DIVCKL_RATIO_VAL		(0x8 << NAND_ECC_DIVCKL_RATIO_OFFS)

#define CORE_DIV_CLK_CTRL(num)			(DFX_CORE_DIVCLK_CONTROL0_REG + ((num) * 0x4))

#define DFX_TEMPERATURE_SENSOR_LSB_CTRL_REG		0xF8070
#define DFX_TEMPERATURE_SENSOR_MSB_CTRL_REG		0xF8074
#define DFX_TEMPERATURE_SENSOR_STATUS_REG		0xF8078

/* Core Divider Clock Control - AXP only */
#define AXP_CORE_DIV_CLK_CTRL(num)		(0x18740 + ((num) * 0x8))

#define AXP_CORE_DIVCLK_RELOAD_FORCE_OFFS	0
#define AXP_CORE_DIVCLK_RELOAD_FORCE_MASK	(0xFF << AXP_CORE_DIVCLK_RELOAD_FORCE_OFFS)
#define AXP_CORE_DIVCLK_RELOAD_FORCE_VAL	(0x2 << AXP_CORE_DIVCLK_RELOAD_FORCE_OFFS)

#define AXP_NAND_ECC_DIVCLK_RATIO_OFFS		8
#define AXP_NAND_ECC_DIVCLK_RATIO_MASK		(0x3F << AXP_NAND_ECC_DIVCLK_RATIO_OFFS)

#define AXP_CORE_DIVCLK_RELOAD_RATIO_OFFS	8
#define AXP_CORE_DIVCLK_RELOAD_RATIO_MASK	(1 << AXP_CORE_DIVCLK_RELOAD_RATIO_OFFS)

/* definition for caculate Temperature */
#define TEMPERATURE_OFFSET        (596)
#define TEMPERATURE_FACTOR        (2154)
#define TEMPERATURE_FACTOR_DIV    (1000)

#define TSMC_UNIT_CONTROL_OFS		28
#define TSMC_UNIT_CONTROL_MASK		(7 << TSMC_UNIT_CONTROL_OFS)
#define TSMC_UNIT_CONTROL_SENSOR0	0
#define TSMC_UNIT_CONTROL_SENSOR1       1
#define TSMC_UNIT_CONTROL_SENSOR2       2
#define TSMC_UNIT_CONTROL_SENSOR3       3
#define TSMC_UNIT_CONTROL_SENSOR4       4
#define TSMC_UNIT_CONTROL_HOTTEST 	6
#define TSMC_UNIT_CONTROL_AVR 	6

#define TSEN_SW_RESET_OFS 		7	/* SW reset */
#define TSEN_SW_RESETMSK 		(1 << TSEN_SW_RESET_OFS)
#define TSEN_PWD_OFS 			6	/* power down */
#define TSEN_PWD_MSK 			(1 << TSEN_PWD_OFS)
#define TSEN_SLEEP_OFS 			5
#define TSEN_SLEEP_MSK 			(1 << TSEN_SLEEP_OFS)

#define TSEN_OTF_CALIB_OFS 	4
#define TSEN_OTF_CALIB_MSK 	(1 << TSEN_OTF_CALIB_OFS)

/*********************************/
/* SoC Device Multiplex Register */
/*********************************/
#define SOC_DEV_MUX_REG				0x18208

/*****************/
/* PUP registers */
/*****************/
#define PUP_EN_REG				0x1864C

/*****************/
/*  registers */
/*****************/
#define I2C_CONFIC_DEBUG_REG	0x1108c
#define	ICDR_UINIT_ID_MASK	0x0F
#define ICDR_UNIT_ID_4_DFX	0x0f

#define CPLD_RD_MTL_BC2_BOARD_REV_REG	2
#define CPLD_BOARD_REV_REG	1
#define CPLD_BOARD_REV_MASK	0x7
#define CPLD_REV_REG		2
#define CPLD_REV_MASK		0x1f

#define DEV_ID_REG			0x1823C
#define DEVICE_ID_OFFS			0
#define DEVICE_ID_MASK			0xFFFF
#define DEVICE_STEP_OFFS		8
#define DEVICE_STEP_MASK		0x3

/* for BobK devID, Cetus: 0xBE00 Caelum:0xBC00, the higher 6bits are the same.
so update mask to 0x3FF, for BC2 flavor(0xF4XX), and AC3 flavor(0xFC00),
the higher 6bits are also the same, so can work normally */
#define DEVICE_FLAVOR_MASK		0x3FF

#define BOBK_FLAVOR_MASK		0xFF	/* only for Cetus & Caelum flavor */
#define DEV_REV_ID_REG			0xF8244
#define REVISON_ID_OFFS			28
#define REVISON_ID_MASK			0xF0000000

/* Extract CPU, L2, DDR clocks SAR value from
 * SAR bits 24-27 - AXP only
 */
#define MSAR_CPU_CLK_IDX(sar0, sar1)		((((sar0) >> 21) & 0x7) + ((((sar1) >> 20) & 1) << 3))
#define MSAR_CPU_CLK_TWSI(sar0, sar1)		((((sar0) >> 2)  & 0x7) + (((sar1) & 1) << 3))
#define MSAR_DDR_L2_CLK_RATIO_IDX(sar0, sar1)	((((sar0) >> 24) & 0xF) + ((((sar1) >> 19) & 1) << 4))
#define MSAR_DDR_L2_CLK_RATIO_TWSI(sar0)	(((sar0) >> 1)  & 0xF)

/* SAR TCLK bit - AXP only */
#define MSAR_TCLK_OFFS				28
#define MSAR_TCLK_MASK				(0x1 << MSAR_TCLK_OFFS)

/* PLL(CPU) input clock - AXP only */
#define AXP_PLL_IN_CLK			_2GHz

#ifndef MV_ASMLANGUAGE

#define MV_CPU_CLK_TBL_AXP { 1000, 1066, 1200, 1333, 1500, 1666, 1800, 2000,\
			      600,  667,  800, 1600, 2133, 2200, 2400, 0 }

/*		cpu	l2c	hclk	ddr	*/
#define MV_DDR_L2_CLK_RATIO_TBL_AXP { \
/*00*/	{	1,	1,	4,	2	},\
/*01*/	{	1,	2,	2,	2	},\
/*02*/	{	2,	2,	6,	3	},\
/*03*/	{	2,	2,	3,	3	},\
/*04*/	{	1,	2,	3,	3	},\
/*05*/	{	1,	2,	4,	2	},\
/*06*/	{	1,	1,	2,	2	},\
/*07*/	{	2,	3,	6,	6	},\
/*08*/	{	2,	3,	5,	5	},\
/*09*/	{	1,	2,	6,	3	},\
/*10*/	{	2,	4,	10,	5	},\
/*11*/	{	1,	3,	6,	6	},\
/*12*/	{	1,	2,	4,	4	},\
/*13*/	{	1,	3,	6,	3	},\
/*14*/	{	1,	2,	5,	5	},\
/*15*/	{	2,	2,	5,	5	},\
/*16*/	{	1,	1,	3,	3	},\
/*17*/	{	2,	5,	10,	10	},\
/*18*/	{	1,	3,	8,	4	},\
/*19*/	{	1,	1,	2,	1	},\
/*20*/	{	2,	3,	6,	3	},\
/*21*/	{	1,	2,	8,	4	},\
/*22*/	{	2,	5,	10,	5	} \
}

/* This structure refrect registers:
 * Serdes 0-7 selectors		0x18270
 * and Serdes 8-15 selectors	0x18274
 * AXP only
 */

#define SERDES_CFG_AXP { \
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 0 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 1 */	\
	{0, 1, -1 ,  2, -1, -1, -1, -1}, /* Lane 2 */	\
	{0, 1, -1 , -1,  2, -1, -1,  3}, /* Lane 3 */	\
	{0, 1,  2 , -1, -1,  3, -1, -1}, /* Lane 4 */	\
	{0, 1,  2 , -1,  3, -1, -1,  4}, /* Lane 5 */	\
	{0, 1,  2 ,  4, -1,  3, -1, -1}, /* Lane 6 */	\
	{0, 1, -1 ,  2, -1, -1,  3, -1}, /* Lane 7 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 8 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 9 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 10 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 11 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 12 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 13 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}, /* Lane 14 */	\
	{0, 1, -1 , -1, -1, -1, -1, -1}	 /* Lane 15 */	\
}
/* These macros help units to identify a target Mport Arbiter group */
#define MV_TARGET_IS_DRAM(target)   \
		((target >= SDRAM_CS0) && (target <= SDRAM_CS3))

#define MV_TARGET_IS_PEX0(target)   \
		((target >= PEX0_MEM) && (target <= PEX0_IO))
#define MV_TARGET_IS_PEX1(target)   \
		((target >= (MV_8)PEX1_MEM) && (target <= (MV_8)PEX1_IO))
#define MV_TARGET_IS_PEX2(target)   \
		((target >= (MV_8)PEX2_MEM) && (target <= (MV_8)PEX2_IO))
#define MV_TARGET_IS_PEX3(target)   \
		((target >= (MV_8)PEX3_MEM) && (target <= (MV_8)PEX3_IO))
#define MV_TARGET_IS_PEX4(target)   \
		((target >= (MV_8)PEX4_MEM) && (target <= (MV_8)PEX4_IO))
#define MV_TARGET_IS_PEX5(target)   \
		((target >= (MV_8)PEX5_MEM) && (target <= (MV_8)PEX5_IO))
#define MV_TARGET_IS_PEX6(target)   \
		((target >= (MV_8)PEX6_MEM) && (target <= (MV_8)PEX6_IO))
#define MV_TARGET_IS_PEX7(target)   \
		((target >= (MV_8)PEX7_MEM) && (target <= (MV_8)PEX7_IO))
#define MV_TARGET_IS_PEX8(target)   \
		((target >= (MV_8)PEX8_MEM) && (target <= (MV_8)PEX8_IO))
#define MV_TARGET_IS_PEX9(target)   \
		((target >= (MV_8)PEX9_MEM) && (target <= (MV_8)PEX9_IO))

#define MV_TARGET_IS_PEX(target)	((target >= PEX0_MEM) && (target <= (MV_8)PEX9_IO))

#define MV_TARGET_IS_DEVICE(target)	((target >= DEVICE_CS0) && (target <= DEVICE_CS3))

#define MV_PCI_DRAM_BAR_TO_DRAM_TARGET(bar)   0

#define MV_CHANGE_BOOT_CS(target) target

#define TCLK_TO_COUNTER_RATIO   1   /* counters running in Tclk */

#define START_DEV_CS   		DEV_CS0
#define DEV_TO_TARGET(dev)	((dev) + START_DEV_CS)

#define PCI_IF0_MEM0		PEX0_MEM
#define PCI_IF0_IO		PEX0_IO

/* This enumerator defines the Marvell controller target ID  (see Address map) */
typedef enum _mvTargetId {
	DRAM_TARGET_ID   = 0,	/* Port 0 -> DRAM interface		*/
	DEV_TARGET_ID    = 1,	/* Port 1 -> Device port, BootROM, SPI	*/
	SWITCH_TARGET_ID = 3,	/* Port 3 -> Switching Core Adapter/units */
	PEX0_TARGET_ID   = 4,	/* Port 4 -> PCI Express 0		*/
	USB_TARGET_ID    = 5,	/* Port 5 -> USB unit			*/
	DFX_TARGET_ID    = 8,	/* Port 8 -> DFX Server			*/
	CRYPT_TARGET_ID  = 9,	/* Port 9 --> Crypto Engine SRAM	*/
	DRAGONITE_TARGET_ID = 10, /* Port 10 -> Dragonite co-processor	*/
	PNC_BM_TARGET_ID = 12,	/* Port 12 -> PNC + BM Unit		*/
	MAX_TARGETS_ID
} MV_TARGET_ID;


/*
	This enum should reflect the units numbers in register
	space which we will need when accessing the HW
*/

typedef enum {
	PEX0_0x4	= 0,
	PEX0_1x4	= 1,
	PEX0_2x4	= 2,
	PEX0_3x4	= 3,
	PEX1_0x4	= 4,
	PEX1_1x4	= 5,
	PEX1_2x4	= 6,
	PEX1_3x4	= 7,
	PEX2_0x4	= 8,
	PEX3_0x4	= 9,
	PEXIF_MAX	= 10
} MV_PEXIF_INDX;

typedef struct {
	MV_U32		cpuFreq;
	MV_U32		ddrFreq;
	MV_U32		pllClk;
	MV_STATUS	internalFreq;
} MV_CPUDDR_MODE;

typedef struct {
	MV_U32		tmFreq;
	MV_U32		ddr3Freq;
	MV_STATUS	internalFreq;
} MV_TM_MODE;

#define MV_CORE_CLK_TBL_BC2	{	\
		360, 220,			\
		250, 400,			\
		175, 520,			\
		450					\
	}

#define MV_CORE_CLK_TBL_BOBK_CETUS	{	\
		365, 220,			\
		250, 200,			\
		167					\
	}

#define MV_CORE_CLK_TBL_BOBK_CAELUM	{	\
		365, 220,			\
		250, 200,			\
		167, 133			\
	}

#define MV_BYPASS_CORE_CLK_TBL_BOBK	{	\
		365, 220,			\
		250, 200,			\
		167					\
	}

#define MV_CORE_CLK_TBL_AC3	{	\
		290, 250,			\
		222, 167,			\
		200, 133,			\
		360					\
	}


#define MV_CPU_DDR_CLK_TBL_BC2 {	\
	{ 400, 400,  800, MV_FALSE},	\
	{ 533, 533, 1600, MV_TRUE},	\
	{ 667, 667, 2000, MV_FALSE},	\
	{ 800, 800, 1600, MV_FALSE},	\
	{1000, 500, 2000, MV_TRUE},	\
	{1200, 600, 2400, MV_TRUE},	\
	{1333, 666, 1600, MV_TRUE}	\
}

#define MV_CPU_DDR_CLK_TBL_BOBK_CETUS {	\
	{ 400, 400,  400, MV_FALSE},	\
	{1000, 667, 2000, MV_FALSE},	\
	{ 667, 667, 2000, MV_FALSE},	\
	{ 800, 800,  800, MV_FALSE},	\
	{1200, 800, 2400, MV_FALSE},	\
	{ 800, 400,  800, MV_FALSE},	\
	{ 800, 800,  800, MV_TRUE}	\
}

#define MV_CPU_DDR_CLK_TBL_BOBK_CAELUM {	\
	{ 400, 400,  400, MV_FALSE},	\
	{1000, 667, 2000, MV_TRUE},	\
	{ 667, 667, 2000, MV_FALSE},	\
	{ 800, 800,  800, MV_FALSE},	\
	{1200, 800, 2400, MV_TRUE},	\
	{ 800, 400,  800, MV_FALSE},	\
	{ 800, 800,  800, MV_TRUE}	\
}

#define MV_CPU_DDR_CLK_TBL_AC3 {	\
	{ 400, 400, 400, MV_FALSE},	\
	{   0,   0, 800, MV_TRUE},	\
	{ 667, 667, 667, MV_FALSE},	\
	{ 800, 800, 800, MV_FALSE},	\
	{   0,   0, 800, MV_TRUE},	\
	{ 800, 400, 400, MV_FALSE},	\
	{   0,   0, 800, MV_TRUE},	\
	{   0,   0, 800, MV_TRUE}	\
}

#define MV_TM_CLK_TBL	{	\
	{   0,   0, MV_FALSE},	\
	{ 400, 800, MV_FALSE},	\
	{ 466, 933, MV_FALSE},	\
	{ 333, 667, MV_FALSE},	\
	{   0,   0, MV_TRUE},	\
	{  -1, 800, MV_TRUE},	\
	{  -1, 933, MV_TRUE}	\
}

#endif /* MV_ASMLANGUAGE */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmvCtrlEnvRegsh */
