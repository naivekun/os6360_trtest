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

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cpu/mvCpu.h"
#include "cntmr/mvCntmr.h"
#include "gpp/mvGpp.h"
#include "twsi/mvTwsi.h"
#include "pex/mvPex.h"
#include "device/mvDevice.h"
#include "neta/gbe/mvEthRegs.h"

#if defined(CONFIG_MV_ETH_NETA)
#include "neta/gbe/mvEthRegs.h"
#endif

#include "gpp/mvGppRegs.h"

/* defines  */
#undef MV_DEBUG
#ifdef MV_DEBUG
#define DB(x)	x
#define DB1(x)	x
#else
#define DB(x)
#define DB1(x)
#endif

extern MV_BOARD_INFO *marvellBC2BoardInfoTbl[];
extern MV_BOARD_INFO *customerBC2BoardInfoTbl[];
extern MV_BOARD_INFO *marvellAC3BoardInfoTbl[];
extern MV_BOARD_INFO *customerAC3BoardInfoTbl[];
extern MV_BOARD_INFO *marvellBOBKBoardInfoTbl[];
extern MV_BOARD_INFO *customerBOBKBoardInfoTbl[];
/* Global variables should be removed from BSS (set to a non-zero value)
   for avoiding memory corruption during early access upon code relocation */
static MV_BOARD_INFO *board = (MV_BOARD_INFO *)-1;

/* Locals */
static MV_DEV_CS_INFO *mvBoardGetDevEntry(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
/*******************************************************************************
* mvBoardisUsbPortConnected
*
* DESCRIPTION:
*	return True if requested USB type and port num exists on current board
*
* INPUT:
*	usbTypeID       - requested USB type : USB3_UNIT_ID / USB_UNIT_ID
*	usbPortNumbder  - requested USB port number (according to xHCI MAC port num)
*
* OUTPUT: None
*
* RETURN: MV_TRUE if requested port/type exist on board

*******************************************************************************/
MV_BOOL mvBoardIsUsbPortConnected(MV_UNIT_ID usbTypeID, MV_U8 usbPortNumber)
{
	/*BobCat2 SoC have no usb port
	AlleyCat3 & BobK SoC board has only one usb2 port */
#ifdef MV_USB
	if (usbTypeID == USB_UNIT_ID && usbPortNumber == 0)
		return MV_TRUE;
#endif
	return MV_FALSE;
}

/*******************************************************************************
* mvBoardIdIndexGet
*
* DESCRIPTION:
*	returns an index for board arrays with direct memory access, according to board id
*
* INPUT:
*       boardId.
*
* OUTPUT:
*       direct access index for board arrays
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_U32 mvBoardIdIndexGet(MV_U32 boardId)
{
/* Marvell Boards use 0x10 as base for Board ID: mask MSB to receive index for board ID*/
	return boardId & (BOARD_ID_INDEX_MASK - 1);
}

/*******************************************************************************
* mvBoardEnvInit - Init board
*
* DESCRIPTION:
*		In this function the board environment take care of device bank
*		initialization.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardEnvInit(MV_VOID)
{
	MV_U32 nandDev;
	MV_U32 norDev;

	mvBoardSet(mvBoardIdGet());

	nandDev = mvBoardGetDevCSNum(0, BOARD_DEV_NAND_FLASH);
	if (nandDev != 0xFFFFFFFF) {
		/* Set NAND interface access parameters */
		nandDev = BOOT_CS;
		MV_REG_WRITE(DEV_BANK_PARAM_REG(nandDev), board->nandFlashReadParams);
		MV_REG_WRITE(DEV_BANK_PARAM_REG_WR(nandDev), board->nandFlashWriteParams);
		MV_REG_WRITE(DEV_NAND_CTRL_REG, board->nandFlashControl);
	}

	norDev = mvBoardGetDevCSNum(0, BOARD_DEV_NOR_FLASH);
	if (norDev != 0xFFFFFFFF) {
		/* Set NOR interface access parameters */
		MV_REG_WRITE(DEV_BANK_PARAM_REG(norDev), board->norFlashReadParams);
		MV_REG_WRITE(DEV_BANK_PARAM_REG_WR(norDev), board->norFlashWriteParams);
		MV_REG_WRITE(DEV_BUS_SYNC_CTRL, 0x11);
	}

	/* Set GPP Out value */
	MV_REG_WRITE(GPP_DATA_OUT_REG(0), board->gppOutValLow);
	MV_REG_WRITE(GPP_DATA_OUT_REG(1), board->gppOutValMid);

	/* set GPP polarity */
	mvGppPolaritySet(0, 0xFFFFFFFF, board->gppPolarityValLow);
	mvGppPolaritySet(1, 0xFFFFFFFF, board->gppPolarityValMid);

	/* Set GPP Out Enable */
	mvGppTypeSet(0, 0xFFFFFFFF, board->gppOutEnValLow);
	mvGppTypeSet(1, 0xFFFFFFFF, board->gppOutEnValMid);

	if (mvCtrlDevFamilyIdGet(0) == MV_78460_DEV_ID) {
		MV_REG_WRITE(GPP_DATA_OUT_REG(2), board->gppOutValHigh);
		mvGppPolaritySet(2, 0xFFFFFFFF, board->gppPolarityValHigh);
		mvGppTypeSet(2, 0xFFFFFFFF, board->gppOutEnValHigh);
	}

#ifndef CONFIG_CUSTOMER_BOARD_SUPPORT
	mvBoardOobPortCfgSet();
#endif

}

/*******************************************************************************
* mvBoardModelGet - Get Board model
*
* DESCRIPTION:
*       This function returns 16bit describing board model.
*       Board model is constructed of one byte major and minor numbers in the
*       following manner:
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       String describing board model.
*
*******************************************************************************/
MV_U16 mvBoardModelGet(MV_VOID)
{
	MV_U8 modelId;
	if (MV_ERROR == mvBoardTwsiRead(BOARD_DEV_TWSI_PLD, 0, 0, &modelId)) {
		mvOsWarning();
		return INVALID_BOARD_ID;
	}

	return (MV_U16)modelId;
}
/*******************************************************************************
* mbBoardRevlGet - Get Board revision
*
* DESCRIPTION:
*       This function returns a 32bit describing the board revision.
*       Board revision is constructed of 4bytes. 2bytes describes major number
*       and the other 2bytes describes minor munber.
*       For example for board revision 3.4 the function will return
*       0x00030004.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       String describing board model.
*
*******************************************************************************/
MV_U16 mvBoardRevGet(MV_VOID)
{
	return mvBoardIdIndexGet(mvBoardIdGet()) & 0xFFFF;
}
/*******************************************************************************
* mvBoardNameGet - Get Board name
*
* DESCRIPTION:
*       This function returns a string describing the board model and revision.
*       String is extracted from board I2C EEPROMboard.
*
* INPUT:
*       None.
*
* OUTPUT:
*       pNameBuff - Buffer to contain board name string. Minimum size 32 chars.
*
* RETURN:
*
*       MV_ERROR if informantion can not be read.
*******************************************************************************/
MV_STATUS mvBoardNameGet(char *pNameBuff)
{
	mvOsSPrintf(pNameBuff, "%s", board->boardName);
	return MV_OK;
}

/*******************************************************************************
* mvBoardIsEthConnected - detect if a certain Ethernet port is Connected
*
* DESCRIPTION:
*	This routine returns true if a certain Ethernet port is Connected
*
* INPUT:
*	ethNum - index of the ethernet port requested
*
* OUTPUT:
*	None.
*
* RETURN:
*	MV_TRUE if the requested ethernet port is connected.
*
*******************************************************************************/
MV_BOOL mvBoardIsEthConnected(MV_U32 ethNum)
{
	return mvBoardIsGbEPortConnected(ethNum);
}

/*******************************************************************************
* mvBoardIsEthActive - this routine indicate which ports can be used by U-Boot
*
* DESCRIPTION:
*	This routine returns true if a certain Ethernet port is
*	Active and usable as a regular eth interface
*
* INPUT:
*	ethNum - index of the ethernet port requested
*
* OUTPUT:
*	None.
*
* RETURN:
*	MV_TRUE if the requested ethernet port is Active and usable.
*
*******************************************************************************/
MV_BOOL mvBoardIsEthActive(MV_U32 ethNum)
{
	/* for Msys, all connected ports are Active and usabe */
	return mvBoardIsEthConnected(ethNum);
}

/*******************************************************************************
* mvBoardIsPortInSgmii -
*
* DESCRIPTION:
*       This routine returns MV_TRUE for port number works in SGMII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in SGMII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInSgmii(MV_U32 ethPortNum)
{
	MV_U32 boardId = mvBoardIdGet();

	if (boardId == DB_78X60_AMC_ID && ethPortNum == 0)
		return MV_FALSE;
	return MV_TRUE;
}
/*******************************************************************************
* mvBoardIsPortInGmii -
*
* DESCRIPTION:
*       This routine returns MV_TRUE for port number works in GMII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in SGMII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInGmii(MV_U32 ethPortNum)
{
	return MV_FALSE;
}

/*******************************************************************************
* mvBoardPortTypeGet
*
* DESCRIPTION:
*       This routine returns port type
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None
*
* RETURN:
*       Mode of the port
*
*******************************************************************************/
MV_U32 mvBoardPortTypeGet(MV_U32 ethPortNum)
{
	if (mvBoardIsPortInSgmii(ethPortNum))
		return MV_PORT_TYPE_SGMII;
	else
		return MV_PORT_TYPE_RGMII;
}

/*******************************************************************************
* mvBoardIsPortInMii
*
* DESCRIPTION:
*	This routine returns MV_TRUE for port number works in MII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in MII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInMii(MV_U32 ethPortNum)
{
	return MV_FALSE;
}
/*******************************************************************************
* mvBoardSwitchCpuPortGet - Get the the Ethernet Switch CPU port
*
* DESCRIPTION:
*       This routine returns the Switch CPU port.
*
* INPUT:
*       switchIdx - index of the switch. Only 0 is supported.
*
* OUTPUT:
*       None.
*
* RETURN:
*       the Switch CPU port, -1 if the switch is not connected.
*
*******************************************************************************/
MV_32 mvBoardSwitchCpuPortGet(MV_U32 switchIdx)
{
	if ((board->switchInfoNum == 0) || (switchIdx >= board->switchInfoNum))
		return -1;

	return board->pSwitchInfo[switchIdx].cpuPort;
}

/*******************************************************************************
* mvBoardPhyAddrGet - Get the phy address
*
* DESCRIPTION:
*       This routine returns the Phy address of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing Phy address, -1 if the port number is wrong.
*
*******************************************************************************/
MV_32 mvBoardPhyAddrGet(MV_U32 ethPortNum)
{
	return board->pBoardMacInfo[ethPortNum].boardEthSmiAddr;
}

MV_BOOL mvBoardIsPortInRgmii(MV_U32 ethPortNum)
{
	if (mvCtrlDevFamilyIdGet(0) == MV_78460_DEV_ID)
		return !mvBoardIsPortInGmii(ethPortNum);
	return !mvBoardIsPortInGmii(ethPortNum) && !mvBoardIsPortInSgmii(ethPortNum);
}
/*******************************************************************************
* mvBoardQuadPhyAddr0Get - Get the PHY address
*
* DESCRIPTION:
*       This routine returns the PHY address of a given Ethernet port.
*       Required to initialize QUAD PHY through a specific PHY address
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing PHY address, -1 if the port number is wrong.
*
*******************************************************************************/
MV_32 mvBoardQuadPhyAddr0Get(MV_U32 ethPortNum)
{
	if (ethPortNum >= board->numBoardMacInfo) {
		DB(mvOsPrintf("%s: Error: invalid ethPortNum (%d)\n", __func__, ethPortNum));
		return MV_ERROR;
	}

	return board->pBoardMacInfo[ethPortNum].boardEthSmiAddr0;
}

/*******************************************************************************
* mvBoardMacSpeedGet - Get the Mac speed
*
* DESCRIPTION:
*       This routine returns the Mac speed if pre define of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BOARD_MAC_SPEED, -1 if the port number is wrong.
*
*******************************************************************************/
MV_BOARD_MAC_SPEED mvBoardMacSpeedGet(MV_U32 ethPortNum)
{
	return board->pBoardMacInfo[ethPortNum].boardMacSpeed;
}

/*******************************************************************************
* mvBoardTclkGet - Get the board Tclk (Controller clock)
*
* DESCRIPTION:
*       This routine extract the controller core clock.
*       This function uses the controller counters to make identification.
*		Note: In order to avoid interference, make sure task context switch
*		and interrupts will not occure during this function operation
*
* INPUT:
*       countNum - Counter number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32 mvBoardTclkGet(MV_VOID)
{
	/* U-Boot flow for MSYS can not read FamilyIdGet to derrive Tclk,
	since FamilyIdGet needs Board ID, via TWSI trasaction, which depends on Tclk reading.
	MSYS platforms use static Tclk=200MHz (not Sampled@Reset)*/
#ifdef MV_MSYS /* MV_MSYS is defined only in U-boot for MSYS platforms: AC3/BC2/BobK */
	return MV_BOARD_TCLK_200MHZ;
#else

	/* for Linux flow, FamilyIdGet can use BoardIDGet, since Board id is not readen from TWSI,
	but passed from U-Boot tags instead */

	if (mvCtrlDevFamilyIdGet(0) != MV_78460_DEV_ID)
		/* constant Tclock @ 200MHz (not Sampled@Reset) */
		return MV_BOARD_TCLK_200MHZ;

	if ((MV_REG_READ(MPP_SAMPLE_AT_RESET(0)) & MSAR_TCLK_MASK) != 0)
		return MV_BOARD_TCLK_200MHZ;
	else
		return MV_BOARD_TCLK_250MHZ;
#endif
}

/*******************************************************************************
* mvBoardSysClkGetAxp - Get the board SysClk of AXP (CPU bus clock , i.e. DDR clock)
*
* DESCRIPTION:
*       This routine extract the CPU bus clock.
*
* INPUT:
*       countNum - Counter number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32 mvBoardSysClkGetAxp(MV_VOID)
{
	MV_U32 idx;
	MV_U32 cpuFreqMhz, ddrFreqMhz;
	MV_CPU_ARM_CLK_RATIO clockRatioTbl[] = MV_DDR_L2_CLK_RATIO_TBL_AXP;

	idx = MSAR_DDR_L2_CLK_RATIO_IDX(MV_REG_READ(MPP_SAMPLE_AT_RESET(0)),
					MV_REG_READ(MPP_SAMPLE_AT_RESET(1)));

	if (clockRatioTbl[idx].vco2cpu != 0) {			/* valid ratio ? */
		cpuFreqMhz = mvCpuPclkGet() / 1000000;		/* obtain CPU freq */
		cpuFreqMhz *= clockRatioTbl[idx].vco2cpu;	/* compute VCO freq */
		ddrFreqMhz = cpuFreqMhz / clockRatioTbl[idx].vco2ddr;
		/* round up to integer MHz */
		if (((cpuFreqMhz % clockRatioTbl[idx].vco2ddr) * 10 / clockRatioTbl[idx].vco2ddr) >= 5)
			ddrFreqMhz++;

		return ddrFreqMhz * 1000000;
	} else
		return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardSysClkGet - Get the board SysClk (CPU bus clock , i.e. DDR clock)
*
* DESCRIPTION:
*       This routine extract the CPU bus clock.
*
* INPUT:
*       countNum - Counter number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32 mvBoardSysClkGet(MV_VOID)
{
	MV_U32		idx;
	MV_U32		freq_tbl_bc2[] = MV_CORE_CLK_TBL_BC2;
	MV_U32		freq_tbl_ac3[] = MV_CORE_CLK_TBL_AC3;
	MV_U32		freq_tbl_bobk_cetus[] = MV_CORE_CLK_TBL_BOBK_CETUS;
	MV_U32		freq_tbl_bobk_caelum[] = MV_CORE_CLK_TBL_BOBK_CAELUM;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family == MV_78460_DEV_ID)
		return mvBoardSysClkGetAxp();
	idx = MSAR_CORE_CLK(MV_DFX_REG_READ(DFX_DEVICE_SAR_REG(0)), MV_DFX_REG_READ(DFX_DEVICE_SAR_REG(1)));

	if (idx >= 7)
		return 0xFFFFFFFF;

	if (family == MV_BOBCAT2_DEV_ID)
		return freq_tbl_bc2[idx] * 1000000;
	else if (family == MV_ALLEYCAT3_DEV_ID)
		return freq_tbl_ac3[idx] * 1000000;
	else if (family == MV_BOBK_DEV_ID) {
		/* BobK family has two different flavors(Cetus/Caelum) with different settings */
		switch (mvCtrlModelGet() & ~BOBK_FLAVOR_MASK) {
		case MV_BOBK_CETUS_98DX4235_DEV_ID:
			return freq_tbl_bobk_cetus[idx] * 1000000;
			break;
		case MV_BOBK_CAELUM_98DX4203_DEV_ID:
			return freq_tbl_bobk_caelum[idx] * 1000000;
			break;
		default:
			mvOsPrintf("ERROR: Unknown Device ID %d, CORE freq get failed\n", mvCtrlModelGet());
			return 0xFFFFFFFF;
		}
	} else
		return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoarGpioPinGet - mvBoarGpioPinGet
*
* DESCRIPTION:
*
* INPUT:
*		gppClass - MV_BOARD_GPP_CLASS enum.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoarGpioPinNumGet(MV_BOARD_GPP_CLASS gppClass, MV_U32 index)
{
	MV_U32 i, indexFound = 0;

	for (i = 0; i < board->numBoardGppInfo; i++) {
		if (board->pBoardGppInfo[i].devClass == gppClass) {
			if (indexFound == index)
				return (MV_U32) board->pBoardGppInfo[i].gppPinNum;
			else
				indexFound++;
		}
	}
	return MV_ERROR;
}

/*******************************************************************************
* mvBoardReset - mvBoardReset
*
* DESCRIPTION:
*			Reset the board
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID mvBoardReset(MV_VOID)
{
	MV_32 resetPin;

	/* Get gpp reset pin if define */
	resetPin = mvBoardResetGpioPinGet();
	if (resetPin != MV_ERROR)
		MV_REG_BIT_RESET(GPP_DATA_OUT_REG((int)(resetPin/32)), (1 << (resetPin % 32)));
	else {
		/* No gpp reset pin was found, try to reset using system reset out */
		MV_REG_BIT_SET( CPU_RSTOUTN_MASK_REG , BIT0);
		MV_REG_BIT_SET( CPU_SYS_SOFT_RST_REG , BIT0);
	}
}

/*******************************************************************************
* mvBoardResetGpioPinGet - mvBoardResetGpioPinGet
*
* DESCRIPTION:
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardResetGpioPinGet(MV_VOID)
{
	return mvBoarGpioPinNumGet(BOARD_GPP_RESET, 0);
}

/*******************************************************************************
* mvBoardSDIOGpioPinGet - mvBoardSDIOGpioPinGet
*
* DESCRIPTION:
*	used for hotswap detection
* INPUT:
*	type - Type of SDIO GPP to get.
*
* OUTPUT:
*	None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardSDIOGpioPinGet(MV_BOARD_GPP_CLASS type)
{
	if ((type != BOARD_GPP_SDIO_POWER) && (type != BOARD_GPP_SDIO_DETECT) && (type != BOARD_GPP_SDIO_WP))
		return MV_FAIL;

	return mvBoarGpioPinNumGet(type, 0);
}

/*******************************************************************************
* mvBoardGpioIntMaskGet - Get GPIO mask for interrupt pins
*
* DESCRIPTION:
*		This function returns a 32-bit mask of GPP pins that connected to
*		interrupt generating sources on board.
*		For example if UART channel A is hardwired to GPP pin 8 and
*		UART channel B is hardwired to GPP pin 4 the fuinction will return
*		the value 0x000000110
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*		See description. The function return -1 if board is not identified.
*
*******************************************************************************/
MV_U32 mvBoardGpioIntMaskGet(MV_U32 gppGrp)
{
	switch (gppGrp) {
	case (0):
		return board->intsGppMaskLow;
		break;
	case (1):
		return board->intsGppMaskMid;
		break;
	case (2):
		return board->intsGppMaskHigh;
		break;
	default:
		return MV_ERROR;
	}
}

/*******************************************************************************
* mvBoardMppGet - Get board dependent MPP register value
*
* DESCRIPTION:
*	MPP settings are derived from board design.
*	MPP group consist of 8 MPPs. An MPP group represents MPP
*	control register.
*       This function retrieves board dependend MPP register value.
*
* INPUT:
*       mppGroupNum - MPP group number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit value describing MPP control register value.
*
*******************************************************************************/
MV_32 mvBoardMppGet(MV_U32 mppGroupNum)
{
	MV_U32 mppMod;

	mppMod = board->pBoardModTypeValue->boardMppMod;
	if (mppMod >= board->numBoardMppConfigValue)
		mppMod = 0; /* default */

	return board->pBoardMppConfigValue[mppMod].mppGroup[mppGroupNum];
}

/*******************************************************************************
* mvBoardGppConfigGet
*
* DESCRIPTION:
*	Get board configuration according to the input configuration GPP's.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	The value of the board configuration GPP's.
*
*******************************************************************************/
MV_U32 mvBoardGppConfigGet(void)
{
	MV_U32 gpp, i, result = 0;

	for (i = 0; i < board->numBoardGppInfo; i++) {
		if (board->pBoardGppInfo[i].devClass == BOARD_GPP_CONF) {
			gpp = board->pBoardGppInfo[i].gppPinNum;
			result <<= 1;
			result |= (mvGppValueGet(gpp >> 5, 1 << (gpp & 0x1F)) >> (gpp & 0x1F));
		}
	}
	return result;

}

/*******************************************************************************
* mvBoardIsGbEPortConnected
*
* DESCRIPTION:
*	Checks if a given GbE port is actually connected to the GE-PHY, internal Switch or any RGMII module.
*
* INPUT:
*	port - GbE port number (0 or 1).
*
* OUTPUT:
*       None.
*
* RETURN:
*	MV_TRUE if port is connected, MV_FALSE otherwise.
*
*******************************************************************************/
MV_BOOL mvBoardIsGbEPortConnected(MV_U32 ethPortNum)
{
	if ((ethPortNum < board->numBoardMacInfo) &&
		(board->pBoardMacInfo[ethPortNum].boardMacEnabled == MV_TRUE))
		return MV_TRUE;
	else
		return MV_FALSE;
}

/*******************************************************************************
* mvBoardCpldConfigurationGet
*
* DESCRIPTION:
*	build a string with CPLD configuration: Board and CPLD revision.
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	string in the following format: ", Rev <BOARD_REV>, CPLD Rev <CPLD_REV>"
*
*******************************************************************************/
MV_VOID mvBoardCpldConfigurationGet(char *str)
{
	MV_U8 cpldTwsiDev, cpldConfig;
	MV_U8 cpldBoardRevReg = CPLD_BOARD_REV_REG;
	MV_U16 boardModel;

	/* Prevent the caller function from printing the same string twice if this function fails */
	*str = 0;

	cpldTwsiDev = mvBoardTwsiAddrGet(BOARD_DEV_TWSI_PLD, 0);

	/* verify that CPLD device is available on current board, else return*/
	if (cpldTwsiDev == 0xff || mvTwsiProbe(cpldTwsiDev, mvBoardTclkGet()) != MV_TRUE)
		return;

	boardModel = mvBoardModelGet();
	if (boardModel == RD_MTL_BC2_PCB_ID)
		cpldBoardRevReg = CPLD_RD_MTL_BC2_BOARD_REV_REG;

	/* Read Board Revision */
	if (MV_ERROR == mvBoardTwsiRead(BOARD_DEV_TWSI_PLD, 0, cpldBoardRevReg, &cpldConfig)) {
		mvOsPrintf("\n%s: Error: failed reading board Revision from CPLD.\n", __func__);
		return;
	}
	sprintf(str, ", Rev %d" , cpldConfig & CPLD_BOARD_REV_MASK);

	/* Read CPLD Revision */
	if (boardModel != RD_MTL_BC2_PCB_ID) {
		if (MV_ERROR == mvBoardTwsiRead(BOARD_DEV_TWSI_PLD, 0, CPLD_REV_REG, &cpldConfig)) {
			mvOsPrintf("\n%s: Error: failed reading CPLD Revision from CPLD.\n", __func__);
			return;
		}
		sprintf(str, "%s, CPLD Rev %d", str, cpldConfig & CPLD_BOARD_REV_MASK);
	}
}

/* Board devices API managments */

/*******************************************************************************
* mvBoardGetDeviceNumber - Get number of device of some type on the board
*
* DESCRIPTION:
*
* INPUT:
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		number of those devices else the function returns 0
*
*
*******************************************************************************/
MV_32 mvBoardGetDevicesNumber(MV_BOARD_DEV_CLASS devClass)
{
	MV_U32 foundIndex = 0, devNum;

	for (devNum = START_DEV_CS; devNum < board->numBoardDeviceIf; devNum++) {
		if (board->pDevCsInfo[devNum].devClass == devClass)
			foundIndex++;
	}

	return foundIndex;
}

/*******************************************************************************
* mvBoardGetDeviceBaseAddr - Get base address of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*	Base address else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceBaseAddr(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry;

	devEntry = mvBoardGetDevEntry(devNum, devClass);
	if (devEntry != NULL)
		return mvCpuIfTargetWinBaseLowGet(DEV_TO_TARGET(devEntry->deviceCS));

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardGetDeviceBusWidth - Get Bus width of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		Bus width else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceBusWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry;

	devEntry = mvBoardGetDevEntry(devNum, devClass);
	if (devEntry != NULL)
		return devEntry->busWidth;

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardGetDeviceWidth - Get dev width of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		dev width else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry;

	devEntry = mvBoardGetDevEntry(devNum, devClass);
	if (devEntry != NULL)
		return devEntry->devWidth;

	return MV_ERROR;
}

/*******************************************************************************
* mvBoardGetDeviceWinSize - Get the window size of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		window size else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_32 mvBoardGetDeviceWinSize(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry;

	devEntry = mvBoardGetDevEntry(devNum, devClass);
	if (devEntry != NULL)
		return mvCpuIfTargetWinSizeGet(DEV_TO_TARGET(devEntry->deviceCS));

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardGetDevEntry - returns the entry pointer of a device on the board
*
* DESCRIPTION:
*
* INPUT:
*	devIndex - The device sequential number on the board
*	devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*	None.
*
* RETURN:
*	If the device is found on the board the then the functions returns the
*	dev number else the function returns 0x0
*
*******************************************************************************/
static MV_DEV_CS_INFO *mvBoardGetDevEntry(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_U32 foundIndex = 0, devIndex;

	for (devIndex = START_DEV_CS; devIndex < board->numBoardDeviceIf; devIndex++) {
		if (board->pDevCsInfo[devIndex].devClass == devClass) {
			if (foundIndex == devNum)
				return &(board->pDevCsInfo[devIndex]);
			foundIndex++;
		}
	}

	/* device not found */
	return NULL;
}

/*******************************************************************************
* mvBoardGetDevCSNum
*
* DESCRIPTION:
*	Return the device's chip-select number.
*
* INPUT:
*	devIndex - The device sequential number on the board
*	devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*	None.
*
* RETURN:
*	If the device is found on the board the then the functions returns the
*	dev number else the function returns 0x0
*
*******************************************************************************/
MV_U32 mvBoardGetDevCSNum(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry;

	devEntry = mvBoardGetDevEntry(devNum, devClass);
	if (devEntry != NULL)
		return devEntry->deviceCS;

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardGetDevBusNum
*
* DESCRIPTION:
*	Return the device's bus number.
*
* INPUT:
*	devIndex - The device sequential number on the board
*	devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*	None.
*
* RETURN:
*	If the device is found on the board the then the functions returns the
*	dev bus number else the function returns 0xFFFFFFFF
*
*******************************************************************************/
MV_U32 mvBoardGetDevBusNum(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry = mvBoardGetDevEntry(devNum, devClass);

	if (devEntry)
		return devEntry->busNum;

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardGetDevState
*
* DESCRIPTION:
*	Return the device's activity state.
*
* INPUT:
*	devIndex - The device sequential number on the board
*	devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*	None.
*
* RETURN:
*	If the device is found on the board the then the functions returns the
*	dev activity state else the function returns 0xFFFFFFFF
*
*******************************************************************************/
MV_BOOL mvBoardGetDevState(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO *devEntry = mvBoardGetDevEntry(devNum, devClass);

	if (devEntry)
		return devEntry->active;

	return 0xFFFFFFFF;
}

/*******************************************************************************
* mvBoardSetDevState
*
* DESCRIPTION:
*	Sets the device's activity state.
*
* INPUT:
*	devIndex - The device sequential number on the board
*   devType - The device type ( Flash,RTC , etc .. )
*   newState - requested deevice state
*
* OUTPUT:
*	None.
*
* RETURN:
*	If the device is found on the board the then the functions returns
*	MV_OK else MV_ERROR
*
*******************************************************************************/
MV_STATUS mvBoardSetDevState(MV_32 devNum, MV_BOARD_DEV_CLASS devClass, MV_BOOL newState)
{
	MV_DEV_CS_INFO *devEntry = mvBoardGetDevEntry(devNum, devClass);

	if (devEntry) {
		devEntry->active = newState;
		return MV_OK;
	} else
		return MV_ERROR;
}

/*******************************************************************************
* mvBoardTwsiAddrTypeGet -
*
* DESCRIPTION:
*	Return the TWSI address type for a given twsi device class.
*
* INPUT:
*	twsiClass - The TWSI device to return the address type for.
*	index	  - The TWSI device index (Pass 0 in case of a single
*		    device)
*
* OUTPUT:
*       None.
*
* RETURN:
*	The TWSI address type.
*
*******************************************************************************/
MV_U8 mvBoardTwsiAddrTypeGet(MV_BOARD_TWSI_CLASS twsiClass, MV_U32 index)
{
	int i;
	MV_U32 indexFound = 0;

	for (i = 0; i < board->numBoardTwsiDev; i++) {
		if (board->pBoardTwsiDev[i].devClass == twsiClass) {
			if (indexFound == index)
				return board->pBoardTwsiDev[i].twsiDevAddrType;
			else
				indexFound++;
		}
	}
	return (MV_ERROR);
}

/*******************************************************************************
* mvBoardTwsiAddrGet -
*
* DESCRIPTION:
*	Return the TWSI address for a given twsi device class.
*
* INPUT:
*	twsiClass - The TWSI device to return the address type for.
*	index	  - The TWSI device index (Pass 0 in case of a single
*		    device)
*
* OUTPUT:
*       None.
*
* RETURN:
*	The TWSI address.
*
*******************************************************************************/
MV_U8 mvBoardTwsiAddrGet(MV_BOARD_TWSI_CLASS twsiClass, MV_U32 index)
{
	int i;
	MV_U32 indexFound = 0;

	for (i = 0; i < board->numBoardTwsiDev; i++) {
		if (board->pBoardTwsiDev[i].devClass == twsiClass) {
			if (indexFound == index)
				return board->pBoardTwsiDev[i].twsiDevAddr;
			else
				indexFound++;
		}
	}
	return (0xFF);
}

/*******************************************************************************
* mvBoardNandWidthGet -
*
* DESCRIPTION: Get the width of the first NAND device in bytes
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN: 1, 2, 4 or MV_ERROR
*
*
*******************************************************************************/
MV_32 mvBoardNandWidthGet(void)
{
	MV_U32 devNum;
	MV_U32 devWidth;

	for (devNum = START_DEV_CS; devNum < board->numBoardDeviceIf; devNum++) {
		devWidth = mvBoardGetDeviceWidth(devNum, BOARD_DEV_NAND_FLASH);
		if (devWidth != MV_ERROR)
			return (devWidth / 8);
	}

	/* NAND wasn't found */
	return MV_ERROR;
}

/*******************************************************************************
* mvBoardSet - Set Board model
*
* DESCRIPTION:
*	Sets the global board structures and global board ID, according to boardId value
*	1. Detect correct MSYS family(Bobcat2/Alleycat3)
*	2. Detect Marvell/Customer board
*
* INPUT:
*	boardId :
*	- U-Boot : set boardID via mvBoardIdGet() - according to pre-compilation flag.
*	- Kernel : set boardID via tags received from U-Boot .
*
* OUTPUT:
*	None.
*
* RETURN:
*	void
*
*******************************************************************************/
#ifdef CONFIG_RD_AC3S
	static MV_U32 gBoardId = RD_MTL_AC3X_BXH_24G4G_AC3_ID;
#else
	static MV_U32 gBoardId = -1 ;
#endif

MV_VOID mvBoardSet(MV_U32 boardId)
{
	/* Marvell BobK Boards */
	if (boardId >= BOBK_MARVELL_BOARD_ID_BASE && boardId < BOBK_MARVELL_MAX_BOARD_ID) { /* Marvell Board */
		board = marvellBOBKBoardInfoTbl[mvBoardIdIndexGet(boardId)];
		gBoardId = boardId;
	/* Marvell Bobcat2 Boards */
	} else if (boardId >= BC2_MARVELL_BOARD_ID_BASE && boardId < BC2_MARVELL_MAX_BOARD_ID) { /* Marvell Board */
		board = marvellBC2BoardInfoTbl[mvBoardIdIndexGet(boardId)];
		gBoardId = boardId;
	/* Marvell AlleyCat3 Boards */
	} else if (boardId >= AC3_MARVELL_BOARD_ID_BASE && boardId < AC3_MARVELL_MAX_BOARD_ID) { /* Marvell Board */
		board = marvellAC3BoardInfoTbl[mvBoardIdIndexGet(boardId)];
		gBoardId = boardId;
	/* Customer BobK Boards */
	} else if (boardId >= BOBK_CUSTOMER_BOARD_ID_BASE && boardId < BOBK_CUSTOMER_MAX_BOARD_ID) {/*Customer Board*/
		board = customerBOBKBoardInfoTbl[mvBoardIdIndexGet(boardId)];
		gBoardId = boardId;
	/* Customer Bobcat2 Boards */
	} else if (boardId >= BC2_CUSTOMER_BOARD_ID_BASE && boardId < BC2_CUSTOMER_MAX_BOARD_ID) { /* Customer Board */
		board = customerBC2BoardInfoTbl[mvBoardIdIndexGet(boardId)];
		gBoardId = boardId;
	/* Customer AlleyCat3 Boards */
	} else if (boardId >= AC3_CUSTOMER_BOARD_ID_BASE && boardId < AC3_CUSTOMER_MAX_BOARD_ID) { /* Customer Board */
		board = customerAC3BoardInfoTbl[mvBoardIdIndexGet(boardId)];
		gBoardId = boardId;
	/* Marvell AXP-AMC Board */
	} else if (boardId == DB_78X60_AMC_ID) {
		/* This case should enter only for LSP 3.2/3.4 flow:
		 * This board ID is passed from AXP-AMC U-Boot 2011.12, and shared here only
		 * for Linux usage (AXP family is shared with MSYS family in LSP) */
		board = marvellAXPboardInfoTbl[0];
		gBoardId = boardId;
	} else {
		mvOsPrintf("%s: Error: wrong board Id (%d)\n", __func__, boardId);
#ifdef CONFIG_ALLEYCAT3
		gBoardId = AC3_CUSTOMER_BOARD_ID0;
		board = customerAC3BoardInfoTbl[gBoardId];
#elif defined CONFIG_BOBCAT2
		gBoardId = BC2_CUSTOMER_BOARD_ID0;
		board = customerBC2BoardInfoTbl[gBoardId];
#else
		gBoardId = BOBK_CETUS_CUSTOMER_BOARD_ID0;
		board = customerBOBKBoardInfoTbl[gBoardId];
#endif
		mvOsPrintf("Applying default Customer board ID (%d: %s)\n", gBoardId, board->boardName);
	}
}

/*******************************************************************************
* mvBoardIdGet - Get Board model
*
* DESCRIPTION:
*       This function returns board ID.
*       Board ID is 32bit word constructed of board model (16bit) and
*       board revision (16bit) in the following way: 0xMMMMRRRR.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit board ID number, '-1' if board is undefined.
*
*******************************************************************************/
MV_U32 mvBoardIdGet(MV_VOID)
{
	if (gBoardId != -1)
		return gBoardId;

#ifdef CONFIG_CUSTOMER_BOARD_SUPPORT

	#if defined CONFIG_ALLEYCAT3
		#ifdef CONFIG_CUSTOMER_BOARD_0
			gBoardId = AC3_CUSTOMER_BOARD_ID0;
		#elif CONFIG_CUSTOMER_BOARD_1
			gBoardId = AC3_CUSTOMER_BOARD_ID1;
		#endif
	#elif defined CONFIG_BOBCAT2
		#ifdef CONFIG_CUSTOMER_BOARD_0
			gBoardId = BC2_CUSTOMER_BOARD_ID0;
		#elif CONFIG_CUSTOMER_BOARD_1
			gBoardId = BC2_CUSTOMER_BOARD_ID1;
		#endif
	#else /* BOBK */
		#ifdef CONFIG_CUSTOMER_BOARD_0
			gBoardId = BOBK_CETUS_CUSTOMER_BOARD_ID0;
		#elif CONFIG_CUSTOMER_BOARD_1
			gBoardId = BOBK_CAELUM_CUSTOMER_BOARD_ID1;
		#endif
	#endif

#else	/* !CONFIG_CUSTOMER_BOARD_SUPPORT */

	MV_U8 readValue;
	MV_BOARD_INFO **mvBoardInfoTbl =
	#if defined CONFIG_ALLEYCAT3
		marvellAC3BoardInfoTbl;
	#elif defined CONFIG_BOBCAT2
		marvellBC2BoardInfoTbl;
	#else /* BOBK */
		marvellBOBKBoardInfoTbl;
	#endif

	/* Temporarily set generic board struct pointer, to set/get EEPROM i2c address, and read board ID */
	board = mvBoardInfoTbl[mvBoardIdIndexGet(MV_DEFAULT_BOARD_ID)];
	gBoardId = MV_DEFAULT_BOARD_ID; /* Terporary for usage by mvCtrlDevFamilyIdGet */

	if (mvBoardSarBoardIdGet(&readValue) != MV_OK) {
		mvOsPrintf("%s: Error obtaining Board ID from EEPROM (%d)\n", __func__, readValue);
		mvOsPrintf("%s: Setting default board to: %s\n", __func__, board->boardName);
		return MV_DEFAULT_BOARD_ID;
	}
	readValue = readValue & (BOARD_ID_INDEX_MASK - 1);

	if (readValue >= MV_MARVELL_BOARD_NUM) {
		mvOsPrintf("%s: Error: read wrong board ID (%d)\n", __func__, readValue);
		mvOsPrintf("%s: Setting default board to: %s\n", __func__, board->boardName);
		return MV_DEFAULT_BOARD_ID;
	}
	gBoardId = MARVELL_BOARD_ID_BASE + readValue;

#endif /* CONFIG_CUSTOMER_BOARD_SUPPORT */

	return gBoardId;
}

/*******************************************************************************
* mvBoardCpssBoardIdSet - Copy Board ID into Switch register for CPSS usage
*
* DESCRIPTION:
*       This function sets board ID in Switch register for CPSS
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK in success
*
*******************************************************************************/
MV_U32 mvBoardCpssBoardIdSet(MV_U8 boardId)
{
	const MV_U32	swBrdIdRegOffset = 0x7C; /* User defined register # 3 */
	const MV_U32	swBrdIdMask = 0xFF;
	MV_U32			swReg;

	if ((boardId & swBrdIdMask) != boardId) {
		mvOsPrintf("%s: Error: Board ID is too large!\n", __func__);
		return MV_ERROR;
	}

	swReg = MV_MEMIO_LE32_READ(SWITCH_REGS_VIRT_BASE | swBrdIdRegOffset);
	swReg |= (boardId & swBrdIdMask);
	MV_MEMIO_LE32_WRITE((SWITCH_REGS_VIRT_BASE | swBrdIdRegOffset), swReg);

	return MV_OK;
}

/*******************************************************************************
* mvBoardTwsiRead -
*
* DESCRIPTION:
*
* INPUT:
*       TWSI class id
*	device num - one of three devices
*	reg num - 0 or 1
*	pointer for output data
* OUTPUT:
*		reg value.
*
* RETURN:
*		status OK or Error
*
*******************************************************************************/
MV_STATUS mvBoardTwsiRead(MV_BOARD_TWSI_CLASS class1, MV_U8 devNum, MV_U8 regNum, MV_U8 *pData)
{
	MV_TWSI_SLAVE twsiSlave;
	MV_TWSI_ADDR slave;

	/* Read MPP module ID */
	DB(mvOsPrintf("Board: Read S@R device read\n"));
	twsiSlave.slaveAddr.address = mvBoardTwsiAddrGet(class1, devNum);
	if (0xFF == twsiSlave.slaveAddr.address)
		return MV_ERROR;
	twsiSlave.slaveAddr.type = mvBoardTwsiAddrTypeGet(class1, devNum);

	/* Use offset as command */
	twsiSlave.offset = regNum;
	twsiSlave.validOffset = MV_TRUE;

	/* in case the offset should be set to a TWSI slave which support
	 * 2 bytes offset, the offset setting will be done in 2 transactions.
	 * For accessing EEPROM, always using 2 bytes address offset
	 */
	if (class1 == BOARD_DEV_TWSI_INIT_EPROM)
		twsiSlave.moreThen256 = MV_TRUE;
	else
		twsiSlave.moreThen256 = MV_FALSE;

	/* TWSI init */
	slave.type = ADDR7_BIT;
	slave.address = 0;
	mvTwsiInit(0, TWSI_SPEED, mvBoardTclkGet(), &slave, 0);


	if (MV_OK != mvTwsiRead(0, &twsiSlave, pData, 1)) {
		DB(mvOsPrintf("Board: Read S@R fail\n"));
		return MV_ERROR;
	}
	DB(mvOsPrintf("Board: Read TWSI succeded,(0x%x)\n", *pData));

	return MV_OK;
}
/*******************************************************************************
* mvBoardTwsiWrite -
*
* DESCRIPTION:
*
* INPUT:
*       TWSI class id
*	device num - one of three devices
*	reg num - 0 or 1
*	input reg data
*
* OUTPUT:
*		None.
*
* RETURN:
*		status OK or Error
*******************************************************************************/
MV_STATUS mvBoardTwsiWrite(MV_BOARD_TWSI_CLASS class1, MV_U8 devNum, MV_U8 regNum, MV_U8 regVal)
{
	MV_TWSI_SLAVE twsiSlave;
	MV_TWSI_ADDR slave;

	/* Read MPP module ID */
	twsiSlave.slaveAddr.address = mvBoardTwsiAddrGet(class1, devNum);
	if (0xFF == twsiSlave.slaveAddr.address)
		return MV_ERROR;
	twsiSlave.slaveAddr.type = mvBoardTwsiAddrTypeGet(class1, devNum);
	twsiSlave.validOffset = MV_TRUE;
	DB(mvOsPrintf("Board: Write S@R device addr %x, type %x, data %x\n",
		      twsiSlave.slaveAddr.address, twsiSlave.slaveAddr.type, regVal));
	/* Use offset as command */
	twsiSlave.offset = regNum;

	/* in case the offset should be set to a TWSI slave which support
	 * 2 bytes offset, the offset setting will be done in 2 transactions.
	 * For accessing EEPROM, always using 2 bytes address offset
	 */
	if (class1 == BOARD_DEV_TWSI_INIT_EPROM)
		twsiSlave.moreThen256 = MV_TRUE;
	else
		twsiSlave.moreThen256 = MV_FALSE;

	/* TWSI init */
	slave.type = ADDR7_BIT;
	slave.address = 0;
	mvTwsiInit(0, TWSI_SPEED, mvBoardTclkGet(), &slave, 0);

	if (MV_OK != mvTwsiWrite(0, &twsiSlave, &regVal, 1)) {
		DB(mvOsPrintf("Board: Write S@R fail\n"));
		return MV_ERROR;
	}
	DB(mvOsPrintf("Board: Write S@R succeded\n"));

	return MV_OK;
}

/*******************************************************************************
* mvBoardTwsiSatRGet -
*
* DESCRIPTION:
*
* INPUT:
*	device num - one of three devices
*	reg num - 0 or 1
*
* OUTPUT:
*		None.
*
* RETURN:
*		reg value
*
*******************************************************************************/
MV_STATUS mvBoardTwsiSatRGet(MV_U8 devNum, MV_U8 regNum, MV_U8 *pData)
{
	return mvBoardTwsiRead(BOARD_DEV_TWSI_SATR, devNum, regNum, pData);
}

/*******************************************************************************
* mvBoardTwsiSatRSet -
*
* DESCRIPTION:
*
* INPUT:
*	devNum - one of three devices
*	regNum - 0 or 1
*	regVal - value
*
*
* OUTPUT:
*		None.
*
* RETURN:
*		reg value
*
*******************************************************************************/
MV_STATUS mvBoardTwsiSatRSet(MV_U8 devNum, MV_U8 regNum, MV_U8 regVal)
{
	return mvBoardTwsiWrite(BOARD_DEV_TWSI_SATR, devNum, regNum, regVal);
}

/*******************************************************************************
* SatR Configuration functions
*******************************************************************************/
MV_STATUS mvBoardCoreFreqGet(MV_U8 *value)
{
	MV_U8		sar0;
	MV_STATUS	rc1;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);
	MV_U32		fieldOffs = (family == MV_ALLEYCAT3_DEV_ID) ? 2 : 0;

	if ((family != MV_ALLEYCAT3_DEV_ID) && (family != MV_BOBCAT2_DEV_ID) && (family != MV_BOBK_DEV_ID)) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	rc1 = mvBoardTwsiSatRGet(1, 0, &sar0);
	if (MV_ERROR == rc1)
		return MV_ERROR;

	*value = (sar0 & (0x7 << fieldOffs)) >> fieldOffs;

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardCoreFreqSet(MV_U8 freqVal)
{
	MV_U8		sar0;
	MV_STATUS	rc1;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);
	MV_U32		fieldOffs = (family == MV_ALLEYCAT3_DEV_ID) ? 2 : 0;

	if ((family != MV_ALLEYCAT3_DEV_ID) && (family != MV_BOBCAT2_DEV_ID)
		&& (family != MV_BOBK_DEV_ID)) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	rc1 = mvBoardTwsiSatRGet(1, 0, &sar0);
	if (MV_ERROR == rc1)
		return MV_ERROR;

	sar0 &= ~(0x7 << fieldOffs);
	sar0 |= (freqVal & 0x7) << fieldOffs;

	if (MV_OK != mvBoardTwsiSatRSet(1, 0, sar0)) {
		DB(mvOsPrintf("Board: Write core Freq S@R fail\n"));
		return MV_ERROR;
	}


	DB(mvOsPrintf("Board: Write core FreqOpt S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************
* Read the new SW SatR field "bypass_coreclock" from EEPROM(0x50), reg#6 bits[2:0]
*******************************************************************************/
MV_STATUS mvBoardBypassCoreFreqGet(MV_U8 *value)
{
	MV_U8		sar0;
	MV_STATUS	rc1;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBK_DEV_ID) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	/* The Core Frequency in Bypass mode is taken from the first address-value pair of the EEPROM
	   initialization sequence, In order to support normal TWSI init sequence flow, the first pair
	   of DWORDS on EEPROM should contain an address (bytes 0-3) of some scratch pad register
	   (for instance an UART SCR) and a value (bytes 4-7), which will be partially interpreted
	   as Core Freq in bypass mode (bits[2:0] of byte 6)
	*/

	rc1 = mvBoardTwsiRead(BOARD_DEV_TWSI_INIT_EPROM, 0, 6, &sar0);
	if (MV_ERROR == rc1)
		return MV_ERROR;

	*value = (sar0 & 0x7);

	return MV_OK;
}

/*******************************************************************************
* Write the new SW SatR field "bypass_coreclock" to EEPROM(0x50), reg#6 bits[2:0]
*******************************************************************************/
MV_STATUS mvBoardBypassCoreFreqSet(MV_U8 freqVal)
{
	MV_U8		sar0;
	MV_STATUS	rc1;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBK_DEV_ID) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	/* The Core Frequency in Bypass mode is taken from the first address-value pair of the EEPROM
	   initialization sequence, In order to support normal TWSI init sequence flow, the first pair
	   of DWORDS on EEPROM should contain an address (bytes 0-3) of some scratch pad register
	   (for instance an UART SCR) and a value (bytes 4-7), which will be partially interpreted
	   as Core Freq in bypass mode (bits[2:0] of byte 6)
	*/

	rc1 = mvBoardTwsiRead(BOARD_DEV_TWSI_INIT_EPROM, 0, 6, &sar0);
	if (MV_ERROR == rc1)
		return MV_ERROR;

	sar0 &= ~0x7;
	sar0 |= (freqVal & 0x7);

	if (MV_OK != mvBoardTwsiWrite(BOARD_DEV_TWSI_INIT_EPROM, 0, 6, sar0)) {
		DB(mvOsPrintf("Board: Write Bypass core Freq S@R fail\n"));
		return MV_ERROR;
	}


	DB(mvOsPrintf("Board: Write Bypss core FreqOpt S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardCpuFreqGet(MV_U8 *value)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family == MV_BOBCAT2_DEV_ID) {
		MV_U8		sar2;

		/* BC2 */
		if ((MV_ERROR == mvBoardTwsiSatRGet(1, 0, &sar)) ||
			(MV_ERROR == mvBoardTwsiSatRGet(2, 0, &sar2)))
			return MV_ERROR;

		*value = ((((sar2 & 0x1)) << 2) | ((sar & 0x18) >> 3));

	} else if (family == MV_BOBK_DEV_ID) {
		MV_U8		sar2;

		/* BOBK */
		if ((MV_ERROR == mvBoardTwsiSatRGet(0, 0, &sar)) ||
			(MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar2)))
			return MV_ERROR;

		*value = ((((sar2 & 0x1)) << 2) | ((sar & 0x18) >> 3));

	} else if (family == MV_ALLEYCAT3_DEV_ID) {

		/* AC3 */
		if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
			return MV_ERROR;

		*value = sar & 0x7;

	} else {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardCpuFreqSet(MV_U8 freqVal)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family == MV_BOBCAT2_DEV_ID) {
		MV_U8		sar2;
		/* BC2 */
		if ((MV_ERROR == mvBoardTwsiSatRGet(1, 0, &sar)) ||
			(MV_ERROR == mvBoardTwsiSatRGet(2, 0, &sar2)))
			return MV_ERROR;

		sar  &= ~0x18;
		sar2 &= ~0x1;
		sar  |= ((freqVal & 0x03) << 3);
		sar2 |= ((freqVal & 0x04) >> 2);

		if (MV_OK != mvBoardTwsiSatRSet(1, 0, sar)) {
			DB(mvOsPrintf("Board: Write CpuFreq(1) S@R fail\n"));
			return MV_ERROR;
		}
		if (MV_OK != mvBoardTwsiSatRSet(2, 0, sar2)) {
			DB(mvOsPrintf("Board: Write CpuFreq(2) S@R fail\n"));
			return MV_ERROR;
		}

	} else if (family == MV_BOBK_DEV_ID) {
		MV_U8		sar2;
		/* BOBK */
		if ((MV_ERROR == mvBoardTwsiSatRGet(0, 0, &sar)) ||
			(MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar2)))
			return MV_ERROR;

		sar  &= ~0x18;
		sar2 &= ~0x1;
		sar  |= ((freqVal & 0x03) << 3);
		sar2 |= ((freqVal & 0x04) >> 2);

		if (MV_OK != mvBoardTwsiSatRSet(0, 0, sar)) {
			DB(mvOsPrintf("Board: Write CpuFreq(1) S@R fail\n"));
			return MV_ERROR;
		}
		if (MV_OK != mvBoardTwsiSatRSet(3, 0, sar2)) {
			DB(mvOsPrintf("Board: Write CpuFreq(2) S@R fail\n"));
			return MV_ERROR;
		}

	} else if (family == MV_ALLEYCAT3_DEV_ID) {

		/* AC3 */
		if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
			return MV_ERROR;

		sar  &= ~(0x7);
		sar  |= (freqVal & 0x7);

		if (MV_OK != mvBoardTwsiSatRSet(3, 0, sar)) {
			DB(mvOsPrintf("Board: Write CpuFreq S@R fail\n"));
			return MV_ERROR;
		}

	} else {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write CpuFreq S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardTmFreqGet(MV_U8 *value)
{
	MV_U8		sar2;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID && family != MV_BOBK_DEV_ID) {
		DB(mvOsPrintf("%s: AC3 controller family is not supported\n", __func__));
		return MV_ERROR; /* AC3 */
	}

	if (family == MV_BOBCAT2_DEV_ID) {

		/* BC2 */
		if (MV_ERROR == mvBoardTwsiSatRGet(2, 0, &sar2))
			return MV_ERROR;

		*value = ((sar2 & 0x0E) >> 1);

	} else if (family == MV_BOBK_DEV_ID) {

		/* BOBK */
		if (MV_ERROR == mvBoardTwsiSatRGet(0, 0, &sar2))
			return MV_ERROR;

		*value = (sar2 & 0x07);
	}

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardTmFreqSet(MV_U8 freqVal)
{
	MV_U8		sar2;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID && family != MV_BOBK_DEV_ID) {
		DB(mvOsPrintf("%s: AC3 controller family is not supported\n", __func__));
		return MV_ERROR; /* AC3 */
	}

	if (family == MV_BOBCAT2_DEV_ID) {

		/* BC2 */
		if (MV_ERROR == mvBoardTwsiSatRGet(2, 0, &sar2))
			return MV_ERROR;

		sar2 &= ~0xE;
		sar2 |= ((freqVal & 0x07) << 1);

		if (MV_OK != mvBoardTwsiSatRSet(2, 0, sar2)) {
			DB(mvOsPrintf("Board: Write TM-Freq S@R fail\n"));
			return MV_ERROR;
		}
	} else if (family == MV_BOBK_DEV_ID) {

		/* BOBK */
		if (MV_ERROR == mvBoardTwsiSatRGet(0, 0, &sar2))
			return MV_ERROR;

		sar2 &= ~0x7;
		sar2 |= (freqVal & 0x07);

		if (MV_OK != mvBoardTwsiSatRSet(0, 0, sar2)) {
			DB(mvOsPrintf("Board: Write TM-Freq S@R fail\n"));
			return MV_ERROR;
		}
	}

	/* BC2 or BOBK*/

	DB(mvOsPrintf("Board: Write TM-Freq S@R succeeded\n"));

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardJtagCpuGet(MV_U8 *value)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID && family != MV_BOBK_DEV_ID) {
		DB(mvOsPrintf("%s: AC3 controller family is not supported\n", __func__));
		return MV_ERROR; /* AC3 */
	}

	if (family == MV_BOBCAT2_DEV_ID) {

		/* BC2 */
		if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
			return MV_ERROR;

		*value = (sar & 0x8) >> 3;

	} else if (family == MV_BOBK_DEV_ID) {

		/* BOBK */
		if (MV_ERROR == mvBoardTwsiSatRGet(1, 0, &sar))
			return MV_ERROR;

		*value = (sar & 0x8) >> 3;
	}

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardJtagCpuSet(MV_U8 cpu)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID && family != MV_BOBK_DEV_ID) {
		DB(mvOsPrintf("%s: AC3 controller family is not supported\n", __func__));
		return MV_ERROR; /* AC3 */
	}

	if (family == MV_BOBCAT2_DEV_ID) {

		/* BC2 */
		if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
			return MV_ERROR;

		sar &= ~(0x8);
		sar |= ((cpu & 0x1) << 3);

		if (MV_OK != mvBoardTwsiSatRSet(3, 0, sar)) {
			DB(mvOsPrintf("Board: Write JTAG CPU S@R fail\n"));
			return MV_ERROR;
		}

	} else if (family == MV_BOBK_DEV_ID) {

		/* BOBK */
		if (MV_ERROR == mvBoardTwsiSatRGet(1, 0, &sar))
			return MV_ERROR;

		sar &= ~(0x8);
		sar |= ((cpu & 0x1) << 3);

		if (MV_OK != mvBoardTwsiSatRSet(1, 0, sar)) {
			DB(mvOsPrintf("Board: Write JTAG CPU S@R fail\n"));
			return MV_ERROR;
		}
	}

	DB(mvOsPrintf("Board: Write JTAG CPU  S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardPtpPllGet(MV_U8 *value)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID) {
		DB(mvOsPrintf("%s: Only Support BC2 controller family\n", __func__));
		return MV_ERROR; /* AC3/BOBK */
	}

	/* BC2 */
	if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
		return MV_ERROR;

	*value = (sar & 0x10) >> 4;

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardPtpPllSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID) {
		DB(mvOsPrintf("%s: Only Support BC2 controller family\n", __func__));
		return MV_ERROR; /* AC3/BOBK */
	}

	/* BC2 */
	if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
		return MV_ERROR;

	sar &= ~(0x10);
	sar |= ((val & 0x1) << 4);

	if (MV_OK != mvBoardTwsiSatRSet(3, 0, sar)) {
		DB(mvOsPrintf("Board: Write PTP PLL S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write PTP PLL  S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardOobPortConnectionGet(MV_U8 port, MV_U8 *value)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID) {
		DB(mvOsPrintf("%s: Only Support BC2 controller family\n", __func__));
		return MV_ERROR; /* AC3/BOBK */
	}

	if (port > 1) {
		DB(mvOsPrintf("%s: Unsupported OOB port number - %d\n", port, __func__));
		return MV_ERROR;
	}

	/* BC2 */
	if (MV_ERROR == mvBoardTwsiSatRGet(0, 1, &sar))
		return MV_ERROR;

	*value = (sar & (0x3 << (port << 1))) >> (port << 1);

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardOobPortConnectionSet(MV_U8 port, MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID) {
		DB(mvOsPrintf("%s: AC3 controller family is not supported\n", __func__));
		return MV_ERROR; /* AC3 */
	}

	if (port > 1) {
		DB(mvOsPrintf("%s: Unsupported OOB port number - %d\n", port, __func__));
		return MV_ERROR;
	}
	/* BC2 */
	if (MV_ERROR == mvBoardTwsiSatRGet(0, 1, &sar))
		return MV_ERROR;

	sar &= ~(0x3 << (port << 1));
	sar |= ((val & 0x3) << (port << 1));

	if (MV_OK != mvBoardTwsiSatRSet(0, 1, sar)) {
		DB(mvOsPrintf("Board: Write OOB port %d connection S@R fail\n", port));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write OOB port %d connection S@R succeeded\n", port));
	return MV_OK;
}



/*******************************************************************************/
MV_STATUS mvBoardForcePexGen1Get(MV_U8 *value)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID) {
		DB(mvOsPrintf("%s: AC3 controller family is not supported\n", __func__));
		return MV_ERROR; /* AC3 */
	}

	/* BC2 */
	if (MV_ERROR == mvBoardTwsiSatRGet(0, 1, &sar))
		return MV_ERROR;

	*value = (sar & 0x10) >> 4;

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardForcePexGen1Set(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_BOBCAT2_DEV_ID) {
		DB(mvOsPrintf("%s: AC3 controller family is not supported\n", __func__));
		return MV_ERROR; /* AC3 */
	}

	/* BC2 */
	if (MV_ERROR == mvBoardTwsiSatRGet(0, 1, &sar))
		return MV_ERROR;

	sar &= ~(0x10);
	sar |= ((val & 0x1) << 4);

	if (MV_OK != mvBoardTwsiSatRSet(0, 1, sar)) {
		DB(mvOsPrintf("Board: Write Force PCIe GEN1 S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write Force PCIe GEN1 S@R succeeded\n"));
	return MV_OK;
}


/*******************************************************************************/
MV_STATUS mvBoardBootDevGet(MV_U8 *value)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);
	MV_U8		twsiDevice = (family == MV_ALLEYCAT3_DEV_ID) ? 2 : 3;

	if ((family != MV_ALLEYCAT3_DEV_ID) && (family != MV_BOBCAT2_DEV_ID) && (family != MV_BOBK_DEV_ID)) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(twsiDevice, 0, &sar))
		return MV_ERROR;

	if (family == MV_BOBK_DEV_ID)
		*value = ((sar & 0x0E) >> 1);
	else
		*value = (sar & 0x7);

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardBootDevSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);
	MV_U8		twsiDevice = (family == MV_ALLEYCAT3_DEV_ID) ? 2 : 3;

	if ((family != MV_ALLEYCAT3_DEV_ID) && (family != MV_BOBCAT2_DEV_ID) && (family != MV_BOBK_DEV_ID)) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(twsiDevice, 0, &sar))
		return MV_ERROR;

	if (family == MV_BOBK_DEV_ID) {
		sar &= ~(0xE);
		sar |= ((val & 0x7) << 1);
	} else {
		sar &= ~(0x7);
		sar |= (val & 0x7);
	}

	if (MV_OK != mvBoardTwsiSatRSet(twsiDevice, 0, sar)) {
		DB(mvOsPrintf("Board: Write BootDev S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write BootDev S@R succeeded\n"));
	return MV_OK;
}
/*******************************************************************************/
MV_STATUS mvBoardDeviceIdGet(MV_U8 *value)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);
	MV_U8		twsiDevice = (family == MV_BOBK_DEV_ID) ? 2 : 0;

	if ((family != MV_ALLEYCAT3_DEV_ID) && (family != MV_BOBCAT2_DEV_ID) && (family != MV_BOBK_DEV_ID)) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(twsiDevice, 0, &sar))
			return MV_ERROR;

		*value = (sar & 0x1F);

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardDeviceIdSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);
	MV_U8		twsiDevice = (family == MV_BOBK_DEV_ID) ? 2 : 0;

	if ((family != MV_ALLEYCAT3_DEV_ID) && (family != MV_BOBCAT2_DEV_ID) && (family != MV_BOBK_DEV_ID)) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(twsiDevice, 0, &sar))
		return MV_ERROR;

	sar &= ~(0x1F);
	sar |= (val & 0x1F);

	if (MV_OK != mvBoardTwsiSatRSet(twsiDevice, 0, sar)) {
		DB(mvOsPrintf("Board: Write device-id S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write deviceid S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardDeviceNumGet(MV_U8 *value)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(1, 0, &sar))
		return MV_ERROR;

	*value = (sar & 0x3);

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardDeviceNumSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(1, 0, &sar))
		return MV_ERROR;

	sar &= ~(0x3);
	sar |= (val & 0x3);

	if (MV_OK != mvBoardTwsiSatRSet(1, 0, sar)) {
		DB(mvOsPrintf("Board: Write device-id S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write deviceid S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardPcieModeGet(MV_U8 *val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);
	MV_U8		bitOffset = (family == MV_ALLEYCAT3_DEV_ID) ? 3 : 4;

	if ((family != MV_ALLEYCAT3_DEV_ID) && (family != MV_BOBCAT2_DEV_ID) && (family != MV_BOBK_DEV_ID)) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	if ((family == MV_ALLEYCAT3_DEV_ID) || (family == MV_BOBCAT2_DEV_ID)) {
		if (MV_ERROR == mvBoardTwsiSatRGet(2, 0, &sar))
			return MV_ERROR;

		*val = (sar & (0x1 << bitOffset)) >> bitOffset;
	} else if (family == MV_BOBK_DEV_ID) {
		if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
			return MV_ERROR;

		*val = (sar & (0x1 << bitOffset)) >> bitOffset;
	}

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardPcieModeSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);
	MV_U8		bitOffset = (family == MV_ALLEYCAT3_DEV_ID) ? 3 : 4;

	if ((family != MV_ALLEYCAT3_DEV_ID) && (family != MV_BOBCAT2_DEV_ID) && (family != MV_BOBK_DEV_ID)) {
		DB(mvOsPrintf("%s: Controller family (0x%04x) is not supported\n", __func__, family));
		return MV_ERROR;
	}

	if ((family == MV_ALLEYCAT3_DEV_ID) || (family == MV_BOBCAT2_DEV_ID)) {
		if (MV_ERROR == mvBoardTwsiSatRGet(2, 0, &sar))
			return MV_ERROR;

		sar &= ~(0x1 << bitOffset);
		sar |= (val & 0x1) << bitOffset;

		if (MV_OK != mvBoardTwsiSatRSet(2, 0, sar)) {
			DB(mvOsPrintf("Board: Write pcimode S@R fail\n"));
			return MV_ERROR;
		}
	} else if (family == MV_BOBK_DEV_ID) {
		if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
			return MV_ERROR;

		sar &= ~(0x1 << bitOffset);
		sar |= (val & 0x1) << bitOffset;

		if (MV_OK != mvBoardTwsiSatRSet(3, 0, sar)) {
			DB(mvOsPrintf("Board: Write pcimode S@R fail\n"));
			return MV_ERROR;
		}
	}

	DB(mvOsPrintf("Board: Write pcimode S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardPcieClockGet(MV_U8 *val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(2, 0, &sar))
		return MV_ERROR;

	*val = (sar & (0x1 << 4)) >> 4;

	return MV_OK;
}
/*******************************************************************************/
MV_STATUS mvBoardPcieClockSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(2, 0, &sar))
		return MV_ERROR;

	sar &= ~(0x1 << 4);
	sar |= (val & 0x1) << 4;

	if (MV_OK != mvBoardTwsiSatRSet(2, 0, sar)) {
		DB(mvOsPrintf("Board: Write pciclock S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write pciclock S@R succeeded\n"));
	return MV_OK;
}

/*********************************************************************************/
/* Operates on the same fileds as mvBoardAvsModeGet(), but has different meaning */
/* The Misc. PLL VCO clock configured by this field valid for AC3 rev. A0 only.  */
/*********************************************************************************/
MV_STATUS mvBoardPllClockGet(MV_U8 *val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
		return MV_ERROR;

	*val = (sar & (0x1 << 3)) >> 3;

	return MV_OK;
}

/*********************************************************************************/
/* Operates on the same fileds as mvBoardAvsModeSet(), but has different meaning */
/* The Misc. PLL VCO clock configured by this field valid for AC3 rev. A0 only.  */
/*********************************************************************************/
MV_STATUS mvBoardPllClockSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
		return MV_ERROR;

	sar &= ~(0x1 << 3);
	sar |= (val & 0x1) << 3;

	if (MV_OK != mvBoardTwsiSatRSet(3, 0, sar)) {
		DB(mvOsPrintf("Board: Write pllclock S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write pcllclock S@R succeeded\n"));
	return MV_OK;
}

/**********************************************************************************/
/* Operates on the same fileds as mvBoardPllClockGet(), but has different meaning */
/* The AVS mode configured by this field valid on AC3 platforms starting rev. A1  */
/**********************************************************************************/
MV_STATUS mvBoardAvsModeGet(MV_U8 *val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
		return MV_ERROR;

	*val = (sar & (0x1 << 3)) >> 3;

	return MV_OK;
}

/**********************************************************************************/
/* Operates on the same fileds as mvBoardPllClockSet(), but has different meaning */
/* The AVS mode configured by this field valid on AC3 platforms starting rev. A1  */
/**********************************************************************************/
MV_STATUS mvBoardAvsModeSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
		return MV_ERROR;

	sar &= ~(0x1 << 3);
	sar |= (val & 0x1) << 3;

	if (MV_OK != mvBoardTwsiSatRSet(3, 0, sar)) {
		DB(mvOsPrintf("Board: Write avsmode S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write avsmode S@R succeeded\n"));
	return MV_OK;
}

/**********************************************************************************/
MV_STATUS mvBoardSmiI2c2AddrGet(MV_U8 *val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
		return MV_ERROR;

	*val = (sar & (0x1 << 4)) >> 4;

	return MV_OK;
}

/**********************************************************************************/
MV_STATUS mvBoardSmiI2c2AddrSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(3, 0, &sar))
		return MV_ERROR;

	sar &= ~(0x1 << 4);
	sar |= (val & 0x1) << 4;

	if (MV_OK != mvBoardTwsiSatRSet(3, 0, sar)) {
		DB(mvOsPrintf("Board: Write slaveaddr S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write slaveaddr S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardSarBoardIdGet(MV_U8 *value)
{
	MV_U8		boardId;

	/* The Board Id is taken from the first address-value pair of the EEPROM initialization sequence
	   In order to support normal TWSI init sequence flow, the first pair of DWORDS on EEPROM
	   should contain an address (bytes 0-3) of some scratch pad register (for instance an UART SCR)
	   and a value (bytes 4-7), which will be partially interpreted as Board ID (bits[7:0] of byte 7)
	*/
	if (MV_ERROR == mvBoardTwsiRead(BOARD_DEV_TWSI_INIT_EPROM, 0, 7, &boardId))
		return MV_ERROR;

	*value = (boardId & 0x7);

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardSarBoardIdSet(MV_U8 val)
{
	MV_U8		boardId;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family == MV_ALLEYCAT3_DEV_ID) {
		if (val >= AC3_MARVELL_BOARD_NUM) {
			mvOsPrintf("%s: Error: Unsupported board ID (%d)\n", __func__, val);
			return MV_ERROR;
		}
	} else if (family == MV_BOBK_DEV_ID) {
		if (val >= BOBK_MARVELL_BOARD_NUM) {
			mvOsPrintf("%s: Error: Unsupported board ID (%d)\n", __func__, val);
			return MV_ERROR;
		}
	} else if (family == MV_BOBCAT2_DEV_ID) {
		if (val >= BC2_MARVELL_BOARD_NUM) {
			mvOsPrintf("%s: Error: Unsupported board ID (%d)\n", __func__, val);
			return MV_ERROR;
		}
	}

	/* The Board Id is taken from the first address-value pair of the EEPROM initalization sequnce
	   In order to support normal TWSI init sequnce flow, the first pair of DWORDS on EEPROM
	   should contain an address (bytes 0-3) of some scratch pad register (for instance an UART SCR)
	   and a value (bytes 4-7), which will be partially interpreted as Board ID (bits[7:0] of byte 7)
	*/
	if (MV_ERROR == mvBoardTwsiRead(BOARD_DEV_TWSI_INIT_EPROM, 0, 7, &boardId))
		return MV_ERROR;

	boardId &= ~(0x7);
	boardId |= (val & 0x7);

	if (MV_OK != mvBoardTwsiWrite(BOARD_DEV_TWSI_INIT_EPROM, 0, 7, boardId)) {
		DB(mvOsPrintf("Board: Write boardid S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write boardid S@R succeeded\n"));
	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardDdrEccEnableGet(MV_U8 *val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(1, 1, &sar))
		return MV_ERROR;

	*val = sar & 0x1;

	return MV_OK;
}

/*******************************************************************************/
MV_STATUS mvBoardDdrEccEnableSet(MV_U8 val)
{
	MV_U8		sar;
	MV_U16		family = mvCtrlDevFamilyIdGet(0);

	if (family != MV_ALLEYCAT3_DEV_ID) {
		DB(mvOsPrintf("%s: BC2 controller family is not supported\n", __func__));
		return MV_ERROR; /* Not supported on BC2 */
	}

	if (MV_ERROR == mvBoardTwsiSatRGet(1, 1, &sar))
		return MV_ERROR;

	sar &= ~(0x1);
	sar |= (val != 0) ? 0x1 : 0x0;

	if (MV_OK != mvBoardTwsiSatRSet(1, 1, sar)) {
		DB(mvOsPrintf("Board: Write ddreccenable S@R fail\n"));
		return MV_ERROR;
	}

	DB(mvOsPrintf("Board: Write ddreccenable S@R succeeded\n"));
	return MV_OK;
}


/*******************************************************************************
* End of SatR Configuration functions
*******************************************************************************/

/*******************************************************************************
* mvBoardMppModulesScan
*
* DESCRIPTION:
*	Scan for modules connected through MPP lines.
*
* INPUT:
*	None.
*
* OUTPUT:
*	None
*
* RETURN:
*       MV_STATUS - MV_OK, MV_ERROR.
*
*******************************************************************************/
MV_STATUS mvBoardMppModulesScan(void)
{
	return MV_OK;
}

/*******************************************************************************
* mvBoardOtherModulesScan
*
* DESCRIPTION:
*	Scan for modules connected through SERDES/LVDS/... lines.
*
* INPUT:
*	None.
*
* OUTPUT:
*	None
*
* RETURN:
*       MV_STATUS - MV_OK, MV_ERROR.
*
*******************************************************************************/
MV_STATUS mvBoardOtherModulesScan(void)
{
	return MV_OK;
}
/*******************************************************************************
* mvBoardIsSwitchModuleConnected
*
* DESCRIPTION:
*	Check if switch module is connected to the board.
*
* INPUT:
*	None.
*
* OUTPUT:
*	None
*
* RETURN:
*       MV_TRUE / MV_FALSE
*
*******************************************************************************/
MV_BOOL mvBoardIsSwitchModuleConnected(void)
{
	return MV_FALSE;
}
/*******************************************************************************
* mvBoardPexInfoGet - Get board PEX Info
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*******************************************************************************/
MV_BOARD_PEX_INFO *mvBoardPexInfoGet(void)
{
	return &board->boardPexInfo;
}

/*******************************************************************************
* mvBoardConfigAutoDetectEnabled
*
* DESCRIPTION:
*	Indicate if the board supports auto configuration and detection of
*	modules. This is usually enabled for DB boards only.
*
* INPUT:
*	None.
*
* OUTPUT:
*	None.
*
* RETURN:
*       MV_TRUE if auto-config/detection is enabled.
*	MV_FALSE otherwise.
*
*******************************************************************************/
MV_BOOL mvBoardConfigAutoDetectEnabled()
{
	return board->configAutoDetect;
}

/*******************************************************************************
* mvBoardDebugLeg - Set the board debug Leds
*
* DESCRIPTION: turn on/off status leds.
* 	       Note: assume MPP leds are part of group 0 only.
*
* INPUT:
*       hexNum - Number to be displayed in hex by Leds.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardDebugLed(MV_U32 hexNum)
{
	MV_U32 val[MV_GPP_MAX_GROUP] = {0};
	MV_U32 mask[MV_GPP_MAX_GROUP] = {0};
	MV_U32 digitMask;
	MV_U32 i, pinNum, gppGroup;

	if (board->pLedGppPin == NULL)
		return;

	hexNum &= (1 << board->activeLedsNumber) - 1;

	for (i = 0, digitMask = 1; i < board->activeLedsNumber; i++, digitMask <<= 1) {
			pinNum = board->pLedGppPin[i];
			gppGroup = pinNum / 32;
			if (hexNum & digitMask)
				val[gppGroup]  |= (1 << (pinNum - gppGroup * 32));
			mask[gppGroup] |= (1 << (pinNum - gppGroup * 32));
	}

	for (gppGroup = 0; gppGroup < MV_GPP_MAX_GROUP; gppGroup++) {
		/* If at least one bit is set in the mask, update the whole GPP group */
		if (mask[gppGroup])
			mvGppValueSet(gppGroup, mask[gppGroup], board->ledsPolarity == 0 ?
					val[gppGroup] : ~val[gppGroup]);
	}

}
/*******************************************************************************
* mvBoardCpuCoresNumGet - Get number of CPU
*
* DESCRIPTION:
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       8bit number of CPU - 0 for UP
*
*******************************************************************************/
MV_U8 mvBoardCpuCoresNumGet(MV_VOID)
{
	return 0;
}

/*******************************************************************************
* mvBoardNandECCModeGet
*
* DESCRIPTION:
*	Obtain NAND ECC mode
*
* INPUT:
*	None.
*
* OUTPUT:
*	None.
*
* RETURN:
*	MV_NFC_ECC_MODE value
*
*******************************************************************************/
MV_NFC_ECC_MODE mvBoardNandECCModeGet()
{
#if defined(MV_NAND_4BIT_MODE)
	return MV_NFC_ECC_BCH_2K;
#elif defined(MV_NAND_8BIT_MODE)
	return MV_NFC_ECC_BCH_1K;
#elif defined(MV_NAND_12BIT_MODE)
	return MV_NFC_ECC_BCH_704B;
#elif defined(MV_NAND_16BIT_MODE)
	return MV_NFC_ECC_BCH_512B;
#else
	return MV_NFC_ECC_DISABLE;
#endif
}
/*******************************************************************************
* mvBoardCompatibleNameGet
*
* DESCRIPTION: return string containing 'compatible' property value
*		needed for Device Tree auto-update parsing sequence
*
* INPUT:  None
* OUTPUT: None.
*
* RETURN: length of returned string (including special delimiters)
*
*******************************************************************************/
MV_U8 mvBoardCompatibleNameGet(char *pNameBuff)
{
	MV_U8 len = 0;
	MV_U32 boardId;
	/* i.e: compatible = "marvell,msys", "marvell,msys-bc2", "marvell,msys-bc2-db", "marvell,armada-370-xp"; */
	len = sprintf(pNameBuff, "marvell,msys") + 1;
	/*
	 * append next strings after the NULL character that the previous
	 * sprintf wrote.  This is how a device tree stores multiple
	 * strings in a property.
	 */
	boardId = mvBoardIdGet();
	if (boardId >= BC2_MARVELL_BOARD_ID_BASE && boardId < BC2_MARVELL_MAX_BOARD_ID) {
		len += sprintf(pNameBuff + len, "marvell,msys-bc2") + 1;
		len += sprintf(pNameBuff + len, "marvell,msys-bc2-db") + 1;
	} else if (boardId >= AC3_MARVELL_BOARD_ID_BASE && boardId < AC3_MARVELL_MAX_BOARD_ID) {
		len += sprintf(pNameBuff + len, "marvell,msys-ac3") + 1;
		len += sprintf(pNameBuff + len, "marvell,msys-ac3-db") + 1;
	} else if (boardId >= BOBK_MARVELL_BOARD_ID_BASE && boardId < BOBK_MARVELL_MAX_BOARD_ID) {
		len += sprintf(pNameBuff + len, "marvell,msys-bobk") + 1;
		if (boardId == BOBK_CETUS_DB_ID) {
			len += sprintf(pNameBuff + len, "marvell,msys-bobk-cetus") + 1;
			len += sprintf(pNameBuff + len, "marvell,msys-bobk-cetus-db") + 1;
		} else if (boardId == BOBK_CAELUM_DB_ID) {
			len += sprintf(pNameBuff + len, "marvell,msys-bobk-caelum") + 1;
			len += sprintf(pNameBuff + len, "marvell,msys-bobk-caelum-db") + 1;
		} else if (boardId == BOBK_LEWIS_RD_ID) {
			len += sprintf(pNameBuff + len, "marvell,msys-bobk-lewis") + 1;
			len += sprintf(pNameBuff + len, "marvell,msys-bobk-lewis-rd") + 1;
		}
	}
	len += sprintf(pNameBuff + len, "marvell,armada-370-xp") + 1;

	return len;
}

MV_NAND_IF_MODE mvBoardNandIfGet()
{
	return NAND_IF_NFC;
}

/*******************************************************************************
* mvBoardOobPortCfgSet - Set the OOB ports connection according to SAR
*
* DESCRIPTION:
*		The OOB ports can be wired to either front panel connectors (internal
*		board ports) or to back panel connectors (plug-in module ports).
*		Additionally the OOB port can be disconnected from all board connectors.
*		The front panel connectors are wired to SERDES lines through external
*		analog MUX (located in CPLD), whcih allows either to select copper
*		ports (SMI managable PHYs) or QSGMII GBIC slots (inband managable).
*		When a particular OOB port SERDES is forwarded to front panel connectors
*		the MUX for this SERDES should select QSGMII connection.
*		The MUX configuration should not be altered for OOB port that uses
*		the back panel connector or defined as disconnected.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*
*
*******************************************************************************/
MV_STATUS mvBoardOobPortCfgSet(MV_VOID)
{
	MV_U8		portCfg;
	MV_U8		port;
	MV_STATUS	rVal;

	for (port = 0; port <= 1; port++) {

		rVal = mvBoardOobPortConnectionGet(port, &portCfg);
		if (rVal != MV_OK)
			return rVal;

		/* SAR defines OOB port connection as following:
		   0 - Back panel connectors (physical ports 20 and 21)
		   1 - Front panel connectors (physical ports 0 and 1)
		   2 - OOB [port disconnected
		   3 - Reserved
		*/
		/* Only write to IO expander MUX if the port is using front panel connector */
		if (portCfg == 1) {
			rVal = mvBoardTwsiRead(BOARD_DEV_TWSI_PCA9555_IO_EXPANDER, 0, 2, &portCfg);
			if (rVal != MV_OK) {
				DB(mvOsPrintf("%s: Error reading port %d config from IO expander\n", __func__, port));
				return rVal;
			}

			portCfg |= 0x1 << port; /* Redirect the port to QSGMII */

			rVal = mvBoardTwsiWrite(BOARD_DEV_TWSI_PCA9555_IO_EXPANDER, 0, 2, portCfg);
			if (rVal != MV_OK) {
				DB(mvOsPrintf("%s: Error writing port %d config to IO expander\n", __func__, port));
				return rVal;
			}
		}

	} /* For all OOB ports */

	return MV_OK;
}

/*******************************************************************************
* mvBoardisAmc
* DESCRIPTION: MSYS platform are not AMC compliant - return always false
*
* INPUT:  None
* OUTPUT: None.
* RETURN: MV_FALSE
*******************************************************************************/
MV_BOOL mvBoardisAmc(void)
{
	return MV_FALSE;
}

/*******************************************************************************
* mvBoardPPSmiIndexGet
* DESCRIPTION: Get the PP SMI index for the board
*
* INPUT:  None
* OUTPUT: PP SMI index.
* RETURN: MV_TRUE
*******************************************************************************/
MV_STATUS mvBoardPPSmiIndexGet(MV_U32 *index)
{
	*index = board->smiExternalPpIndex;
	return MV_OK;
}

/*******************************************************************************
* mvBoardUpdateConfigforDT
* DESCRIPTION: Update board configuration structure for DT update
*
* INPUT:  None.
* OUTPUT: None.
* RETURN: None.
*******************************************************************************/
MV_VOID mvBoardUpdateConfigforDT(MV_VOID)
{

}

#ifdef CONFIG_MMC
/*******************************************************************************
* mvBoardisSdioConnected
* DESCRIPTION: return true if SDIO connected on board
*
* INPUT:  None
* OUTPUT: None.
* RETURN: MV_TRUE:SDIO connected on board
*         MV_FALSE: else
*******************************************************************************/
MV_BOOL mvBoardisSdioConnected(void)
{
	return board->isSdMmcConnected;
}
#endif

/*******************************************************************************
* mvBoardGetModelName
*
* DESCRIPTION:
*       This function returns a string describing the board model.
*
* OUTPUT:
*       pNameBuff - Buffer to contain board model name string. Minimum size 128 chars.
*
*******************************************************************************/
void mvBoardGetModelName(char *pNameBuff)
{
	mvOsSPrintf(pNameBuff, "Marvell MSYS %s %s", board->modelName, board->boardName);
}

/*******************************************************************************
* mvBoardIsPpSmi
* DESCRIPTION:
*	checks whether PP_SMI is used or not
*
* INPUT:  None
* OUTPUT: None.
* RETURN: MV_TRUE PP_SMI is used
*	  MV_FALSE otherwise
*******************************************************************************/
MV_BOOL mvBoardIsPpSmi(void)
{
	return board->isSmiExternalPp;
}

/*******************************************************************************
* mvBoardPinCtrlNameGet
*
* DESCRIPTION:
*       This function returns the compatible string of pinctrl
*
* OUTPUT:
*       compatibleBuf - Buffer to contain pinctrl compatible string
*
*******************************************************************************/
void mvBoardPinCtrlNameGet(char *compatibleBuf)
{
	mvOsSPrintf(compatibleBuf, "marvell,bc2-ac3-pinctrl");
}

#ifdef MV_CM3
/*******************************************************************************
* mvBoardIsCm3
* DESCRIPTION:
*	checks whether CM3 is used or not
*
* INPUT:  None
* OUTPUT: None.
* RETURN: MV_TRUE CM3 is used
*	  MV_FALSE otherwise
*******************************************************************************/
MV_BOOL mvBoardIsCm3(void)
{
	return board->isCm3;
}

/*******************************************************************************
* mvBoardCm3CompatibleNameGet
*
* DESCRIPTION:
*       This function returns the compatible string of cm3
*
* OUTPUT:
*       compatibleBuf - Buffer to contain cm3 compatible string
*
*******************************************************************************/
void mvBoardCm3CompatibleNameGet(char *compatibleBuf)
{
	if (gBoardId >=  BOBK_MARVELL_BOARD_ID_BASE &&
			gBoardId < BOBK_MARVELL_MAX_BOARD_ID)
		mvOsSPrintf(compatibleBuf, "marvell,msys-bobk-cm3");
	else if (gBoardId >=  BC2_MARVELL_BOARD_ID_BASE &&
			gBoardId < BC2_MARVELL_MAX_BOARD_ID)
		mvOsSPrintf(compatibleBuf, "marvell,msys-bc2-cm3");

}
#endif
