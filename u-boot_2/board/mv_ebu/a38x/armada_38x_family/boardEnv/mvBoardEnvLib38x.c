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

/* defines  */
#undef MV_DEBUG
#ifdef MV_DEBUG
#define DB(x)   x
#else
#define DB(x)
#endif

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
	if (mvBoardIsPortInRgmii(ethPortNum))
		return MV_PORT_TYPE_RGMII;
	return MV_PORT_TYPE_UNKNOWN;
}

/*******************************************************************************
* mvBoardIsUsb3PortDevice
* DESCRIPTION: return true USB3 port is in device mode
*
* INPUT:  port		- port number
* OUTPUT: None.
* RETURN: MV_TRUE: if port is set to device mode
*         MV_FALSE: otherwise
*******************************************************************************/
MV_BOOL mvBoardIsUsb3PortDevice(MV_U32 port)
{
	MV_U32 boardId, satrReadResult;

	if (port < 0 || port >= MV_USB3_MAX_HOST_PORTS)
		return MV_FALSE;

	boardId = mvBoardIdGet();
	/* since 'usb3port0' and 'usb3port1' are only supported on
	 * DB-BP and DB-6821-BP, return MV_FALSE if we are not on these boards */
	if (!(boardId == DB_68XX_ID || boardId == DB_BP_6821_ID))
		return MV_FALSE;

	satrReadResult = mvBoardSatRRead((MV_U32)MV_SATR_DB_USB3_PORT0 + port);
	if (satrReadResult != MV_ERROR && satrReadResult)
		return MV_TRUE;

	return MV_FALSE;
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
	if (ethPortNum < mvCtrlEthMaxPortGet())
		return mvCtrlPortIsSerdesSgmii(ethPortNum);

	return MV_FALSE;
}

/*******************************************************************************
* mvBoardIsPortInGmii
*
* DESCRIPTION:
*	This routine returns MV_TRUE for port number works in GMII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in GMII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInGmii(MV_U32 ethPortNum)
{
	/* If module MII connected return port MII as GMII for NETA init configuration */
	if (mvBoardIsPortInMii(ethPortNum))
		return MV_TRUE;
	return MV_FALSE;
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
	/* On DB board if MII module detected then port 0 is MII */
	if ((mvBoardIsModuleConnected(MV_MODULE_MII)) && (ethPortNum == 0))
		return MV_TRUE;
	return MV_FALSE;
}

/*******************************************************************************
* mvBoardIsPortInRgmii
*
* DESCRIPTION:
*       This routine returns MV_TRUE for port number works in RGMII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in RGMII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInRgmii(MV_U32 ethPortNum)
{
	if (ethPortNum < mvCtrlEthMaxPortGet())
		return mvCtrlPortIsRgmii(ethPortNum);

	return MV_FALSE;
}

/*******************************************************************************
* mvBoardDevStateUpdate -
*
* DESCRIPTION:
*	Update Board devices state (active/passive) according to boot source
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardFlashDeviceUpdate(MV_VOID)
{
	MV_BOARD_BOOT_SRC	bootSrc = mvBoardBootDeviceGet();
	MV_U32				numOfDevices;
	MV_U8				devNum;
	MV_U32				devBus;

	/* Assume that the board sctructure sets SPI flash active as the default boot device */
	if (bootSrc == MSAR_0_BOOT_NAND_NEW) {
		/* Activate first NAND device */
		mvBoardSetDevState(0, BOARD_DEV_NAND_FLASH, MV_TRUE);

		/* Deactivate all SPI0 devices */
		numOfDevices = mvBoardGetDevicesNumber(BOARD_DEV_SPI_FLASH);
		for (devNum = 0; devNum < numOfDevices; devNum++) {
			devBus = mvBoardGetDevBusNum(devNum, BOARD_DEV_SPI_FLASH);
			if (devBus == 0)
				mvBoardSetDevState(devNum, BOARD_DEV_SPI_FLASH, MV_FALSE);
		}

	} else if (bootSrc == MSAR_0_BOOT_NOR_FLASH) {

		/* Activate first NOR device */
		mvBoardSetDevState(0, BOARD_DEV_NOR_FLASH, MV_TRUE);

		/* Deactivate all SPI devices */
		numOfDevices = mvBoardGetDevicesNumber(BOARD_DEV_SPI_FLASH);
		for (devNum = 0; devNum < numOfDevices; devNum++)
			mvBoardSetDevState(devNum, BOARD_DEV_SPI_FLASH, MV_FALSE);

		/* Deactivate all NAND devices */
		numOfDevices = mvBoardGetDevicesNumber(BOARD_DEV_NAND_FLASH);
		for (devNum = 0; devNum < numOfDevices; devNum++)
			mvBoardSetDevState(devNum, BOARD_DEV_NAND_FLASH, MV_FALSE);

	}
}

/*******************************************************************************
* mvBoardPcieModulesInfoUpdate - Update Board information PCIe modules structures
*
* DESCRIPTION:
*	Update board information according to detection of 'dbserdes1/2' & 'sgmiimode'
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardPcieModulesInfoUpdate(MV_VOID)
{
	MV_U32 sgmiiPhyAddr;
	/* 'SatR' PCIe modules configuration ('dbserdes1', 'dbserdes2' , 'sgmiimode') */
	sgmiiPhyAddr = mvBoardSatRRead(MV_SATR_SGMII_MODE) ? 0x10 : -1;

	switch (mvBoardSatRRead(MV_SATR_DB_SERDES1_CFG)) {
	case 0x3: /* SGMII port 0 */
		mvBoardPhyAddrSet(0, sgmiiPhyAddr);
		break;
	case 0x4: /* SGMII port 1 */
		mvBoardPhyAddrSet(1, sgmiiPhyAddr);
		break;
	case 0x6: /* QSGMII */
		sgmiiPhyAddr = 0x8; /* 0x8 = SMI address of 1st Quad PHY */
		mvBoardPhyAddrSet(0, sgmiiPhyAddr);
		mvBoardPhyAddrSet(1, sgmiiPhyAddr + 1);
		mvBoardPhyAddrSet(2, sgmiiPhyAddr + 2);
		mvBoardQuadPhyAddr0Set(0, sgmiiPhyAddr);
		mvBoardQuadPhyAddr0Set(1, sgmiiPhyAddr);
		mvBoardQuadPhyAddr0Set(2, sgmiiPhyAddr);
		break;
	}

	if (mvBoardSatRRead(MV_SATR_DB_SERDES2_CFG) == 0x3) /* SGMII port 1 */
		mvBoardPhyAddrSet(1, sgmiiPhyAddr);
}
/*******************************************************************************
* mvBoardInfoUpdate - Update Board information structures according to auto-detection.
*
* DESCRIPTION:
*	Update board information according to detection using TWSI bus.
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardInfoUpdate(MV_VOID)
{
	MV_U32 reg;

	switch (mvBoardIdGet()) {
	case DB_BP_6821_ID:
		/* test if SLM-1426 (SGMII-2) module is connected */
		if ((mvBoardIsModuleConnected(MV_MODULE_DB381_SGMII)))
			mvBoardPhyAddrSet(2, -1);

		if ((mvBoardIsModuleConnected(MV_MODULE_DB381_MMC_8BIT_ON_BOARD)))
			mvBoardSdioConnectionSet(MV_TRUE);

		mvBoardIoExpanderUpdate();
		mvBoardPcieModulesInfoUpdate();	/* if PCIe modules are configured (via 'SatR') */

		/* Update MPP group types and values according to board configuration */
		mvBoardMppIdUpdate();
		mvBoardFlashDeviceUpdate();
		break;
	case DB_GP_68XX_ID:
		mvBoardMppIdUpdate();
		mvBoardIoExpanderUpdate();
		break;
	case RD_NAS_68XX_ID:
	case RD_AP_68XX_ID:
		mvBoardIoExpanderUpdate();
		/* SGMII utilizes in-Band SMI access, no SMI address is used (set -1 to disable MAC SMI polling)*/
		if (mvBoardSatRRead(MV_SATR_RD_SERDES4_CFG) == 1) /* 0 = USB3.  1 = SGMII. */
			mvBoardPhyAddrSet(1, -1);
		break;
	case DB_68XX_ID:
		if ((mvBoardIsModuleConnected(MV_MODULE_MII)))	/* MII Module uses different PHY address */
			mvBoardPhyAddrSet(0, 8);	/*set SMI address 8 for port 0*/

		if (mvBoardIsModuleConnected(MV_MODULE_SPDIF_DEVICE) ||
				mvBoardIsModuleConnected(MV_MODULE_I2S_DEVICE)) {
			/* TDM, Audio and Sdio have mpp's conflict
			 * --> disable Sdio and TDM when SPDIF is connected */
			mvBoardAudioConnectionSet(MV_TRUE);
			mvBoardSdioConnectionSet(MV_FALSE);
			mvBoardTdmConnectionSet(MV_FALSE);
		} else if (mvBoardSatRRead(MV_SATR_TDM_CONNECTED) == 0) {
			/* if Audio modules detected, skip reading TDM from SatR */

			/* TDM, Audio and Sdio have mpp's conflict
			 * --> disable Audio and Sdio when TDM is connected */
			mvBoardTdmConnectionSet(MV_TRUE);
			mvBoardAudioConnectionSet(MV_FALSE);
			mvBoardSdioConnectionSet(MV_FALSE);
			/* update DT: used for Linux only */
			mvBoardSetDevState(1, BOARD_DEV_SPI_FLASH, MV_TRUE);
		}

		/* Update MPP group types and values according to board configuration */
		mvBoardMppIdUpdate();
		/* board on test mode  */
		reg = MV_REG_READ(MPP_SAMPLE_AT_RESET) & BIT20;
		if (reg) {
			/* if board on test mode reset MPP19 */
			reg = mvBoardMppGet(2);
			reg &= 0xffff0fff;
			mvBoardMppSet(2, reg);
		}

		mvBoardPcieModulesInfoUpdate();	/* if PCIe modules are configured (via 'SatR') */
		mvBoardFlashDeviceUpdate();
		mvBoardModuleSwitchInfoUpdate(mvBoardIsModuleConnected(MV_MODULE_SWITCH));
		mvBoardIoExpanderUpdate();
		break;
	case DB_AMC_6820_ID:
		/* nothing to be updated at run-time for AMC board */
		break;
	default:
		mvOsPrintf("%s: Error: Auto detection update sequence is not supported by current board.\n" , __func__);
	}
}

/*******************************************************************************
* mvBoardIsEthConnected - detect if a certain Ethernet port is active
*
* DESCRIPTION:
*	This routine returns true if a certain Ethernet port is active
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
* mvBoardMppModuleTypePrint
*
* DESCRIPTION:
*	Print on-board detected modules.
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardMppModuleTypePrint(MV_VOID)
{
	int i;
	char *moduleStr[MV_MODULE_TYPE_MAX_MODULE] = { \
		"MII",                                  \
		"TDM",                                  \
		"AUDIO I2S",                     \
		"AUDIO SPDIF",                   \
		"NOR 16bit",                                  \
		"NAND 16bit",                                 \
		"SDIO 4bit",                                 \
		"SGMII",                                 \
		"DB-381 SLM-1426 (SGMII-2)",                        \
		"SWITCH",			\
	};
	mvOsOutput("Board configuration detected:\n");

	for (i = 0; i < MV_MODULE_TYPE_MAX_MODULE; i++) {
		if (mvBoardIsModuleConnected(1 << i))
			mvOsOutput("       %s module.\n", moduleStr[i]);
	}
}

/*******************************************************************************
* mvBoardMppIdUpdate - Update MPP ID's according to modules auto-detection.
*
* DESCRIPTION:
*	Update MPP ID's according to on-board modules as detected using TWSI bus.
*	Update board information for changed mpp values
	Must run AFTER mvBoardNetComplexInfoUpdate
*
* INPUT:
*	None.
*
* OUTPUT:
*       None.
*
* RETURN:
*	None.
*
*******************************************************************************/
MV_VOID mvBoardMppIdUpdate(MV_VOID)
{
	struct _mvBoardMppModule miiModule[3] = MPP_MII_MODULE;
	struct _mvBoardMppModule norModule[6] = MPP_NOR_MODULE;
	struct _mvBoardMppModule nandModule[6] = MPP_NAND_MODULE;
	struct _mvBoardMppModule sdioModule[4] = MPP_SDIO_MODULE;
	struct _mvBoardMppModule tdmModule[2] = MPP_TDM_MODULE;
	struct _mvBoardMppModule audioModule = MPP_AUDIO_MODULE;
	struct _mvBoardMppModule nandOnBoard[4] = MPP_NAND_ON_BOARD;
	struct _mvBoardMppModule mini_pcie0_OnBoard = MPP_GP_MINI_PCIE0;
	struct _mvBoardMppModule mini_pcie1_OnBoard = MPP_GP_MINI_PCIE1;
	struct _mvBoardMppModule mini_pcie0_pcie1OnBoard = MPP_GP_MINI_PCIE0_PCIE1;
	struct _mvBoardMppModule mmcOnDb381Board = MPP_MMC_DB381_MODULE;
	MV_U8 miniPcie0_sata0_selector, miniPcie1_sata1_selector;

	switch (mvBoardIdGet()) {
	case DB_BP_6821_ID:
		if (mvBoardIsModuleConnected(MV_MODULE_NAND_ON_BOARD))
			mvModuleMppUpdate(4, nandOnBoard);
		if (mvBoardIsModuleConnected(MV_MODULE_DB381_MMC_8BIT_ON_BOARD))
			mvModuleMppUpdate(1, &mmcOnDb381Board);
		break;
	case DB_68XX_ID:
		if (mvBoardIsModuleConnected(MV_MODULE_MII))
			mvModuleMppUpdate(3, miiModule);

		if (mvBoardIsModuleConnected(MV_MODULE_NOR))
			mvModuleMppUpdate(6, norModule);

		if (mvBoardIsModuleConnected(MV_MODULE_NAND))
			mvModuleMppUpdate(6, nandModule);

		if (mvBoardIsModuleConnected(MV_MODULE_SDIO))
			mvModuleMppUpdate(4, sdioModule);

		/* Microsemi LE88266DLC (VE880 series) */
		if (mvBoardIsModuleConnected(MV_MODULE_SLIC_TDM_DEVICE))
			mvModuleMppUpdate(2, tdmModule);

		if (mvBoardIsModuleConnected(MV_MODULE_I2S_DEVICE) ||
				mvBoardIsModuleConnected(MV_MODULE_SPDIF_DEVICE))
			mvModuleMppUpdate(1, &audioModule);

		if (mvBoardIsModuleConnected(MV_MODULE_NAND_ON_BOARD))
			mvModuleMppUpdate(4, nandOnBoard);

		/* Sillab SI32261-F */
		if (mvBoardIsTdmConnected() == MV_TRUE)
			mvModuleMppUpdate(2, tdmModule);
		break;
	case DB_GP_68XX_ID:
		miniPcie0_sata0_selector = mvBoardSatRRead(MV_SATR_GP_SERDES1_CFG); /* 0 = SATA0 , 1 = PCIe0 */
		miniPcie1_sata1_selector = mvBoardSatRRead(MV_SATR_GP_SERDES2_CFG); /* 0 = SATA1 , 1 = PCIe1 */
		if (miniPcie0_sata0_selector == 1 && miniPcie1_sata1_selector == 1)
			mvModuleMppUpdate(1, &mini_pcie0_pcie1OnBoard);
		else if (miniPcie0_sata0_selector == 1)
			mvModuleMppUpdate(1, &mini_pcie0_OnBoard);
		else if (miniPcie1_sata1_selector == 1)
			mvModuleMppUpdate(1, &mini_pcie1_OnBoard);
		break;
	}
}

/*******************************************************************************
* mvBoardIoExpanderUpdate
*
* DESCRIPTION:
*	Update IO expander data in board structures only
*
** INPUT:
*	None.
*
* OUTPUT:
*	None.
*
* RETURN:
*	MV_OK - on success,
*	MV_ERROR - wriet to twsi failed.
*
*******************************************************************************/
MV_STATUS mvBoardIoExpanderUpdate(MV_VOID)
{
	MV_U8 ioValue, ioValue2;
	MV_U8 boardId = mvBoardIdGet();
	MV_U32 serdesCfg = MV_ERROR;

	/* - Verify existence of IO expander on board (reg#2 must be set: output data config for reg#0)
	 * - fetch IO expander 2nd output config value to modify */
	if (mvBoardIoExpanderStructGet(0, 2, &ioValue) == MV_ERROR)
		return MV_OK;

	/* update IO expander struct values before writing via I2C */
	if (boardId == DB_GP_68XX_ID) {
		/* fetch IO exp. 2nd value to modify - MV_ERROR: no IO expander struct entry to update*/
		if (mvBoardIoExpanderStructGet(1, 6, &ioValue2) == MV_ERROR)
			return MV_OK;

		/* check SerDes lane 1,2,5 configuration ('gpserdes1/2/5') */
		serdesCfg = mvBoardSatRRead(MV_SATR_GP_SERDES5_CFG);
		if (serdesCfg != MV_ERROR) {
			if (serdesCfg == 0) /* 0 = USB3.  1 = SGMII. */
				ioValue |= 1 ;	/* Setting USB3.0 interface: configure IO as output '1' */
			else {
				ioValue &= ~1;		/* Setting SGMII interface:  configure IO as output '0' */
				ioValue2 &= ~BIT5;	/* Set Tx disable for SGMII */
			}
			mvBoardIoExpanderStructSet(1, 6, ioValue2);
		}

		serdesCfg = mvBoardSatRRead(MV_SATR_GP_SERDES1_CFG);
		if (serdesCfg == 1) { /* 0 = SATA0 , 1 = PCIe0 */
			ioValue |= BIT1 ;	/* Setting PCIe0 (mini): PCIe0_SATA0_SEL(out) = 1 (PCIe)  */
			ioValue &= ~BIT2;	/* disable SATA0 power: PWR_EN_SATA0(out) = 0 */
		}

		serdesCfg = mvBoardSatRRead(MV_SATR_GP_SERDES2_CFG);
		if (serdesCfg == 1) { /* 0 = SATA1 , 1 = PCIe1 */
			ioValue |= BIT6 ;	/* Setting PCIe1 (mini): PCIe1_SATA1_SEL(out) = 1 (PCIe)  */
			ioValue &= ~BIT3;	/* disable SATA1 power: PWR_EN_SATA1(out) = 0 */
		}

		mvBoardIoExpanderStructSet(0, 2, ioValue);
	}

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
	MV_BOARD_INFO *board = mvBoardInfoStructureGet();

	if (!board) {
		DB(mvOsPrintf("%s: Error: board structure not initialized\n", __func__));
		return;
	}

	mvOsSPrintf(pNameBuff, "Marvell Armada 38x %s %s", board->modelName, board->boardName);
}
