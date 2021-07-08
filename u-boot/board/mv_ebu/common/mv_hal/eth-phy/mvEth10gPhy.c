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

#include "mvOs.h"
#include "mvEthPhy.h"
#include "mvEth10gPhy.h"
#include "mvEthPhyXsmi.h"

#define PHY_TIMEOUT 10000
static MV_ETHPHY_HAL_DATA eth10gPhyHalData;

/*******************************************************************************
* mvEth10gPhyHalInit -
*
* DESCRIPTION:
*       Initialize the ethernet 10G phy unit HAL.
*
* INPUT:
*       halData	- Ethernet PHY HAL data.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK on success, MV_ERROR otherwise.
*
*******************************************************************************/
MV_STATUS mvEth10gPhyHalInit(MV_ETHPHY_HAL_DATA *halData)
{
	mvOsMemcpy(&eth10gPhyHalData, halData, sizeof(MV_ETHPHY_HAL_DATA));

	return MV_OK;
}

/*******************************************************************************
* mvEthX2242PPhyBasicInit -
*
* DESCRIPTION:
*       Init the 88X2242P phy to 10GBaseR.
*
* INPUT:
*       phyAddr - the XSMI address of the PHY.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK on success, MV_ERROR otherwise.
*
******************************************************************************/
MV_STATUS mvEthX2242PPhyBasicInit(MV_U32 ethPortNum)
{
	MV_U32 phyAddr;

	phyAddr = eth10gPhyHalData.phyAddr[ethPortNum];

	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0XF016, 0X0008);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF013, 0x0ff0);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF012, 0x0dbc);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF014, 0x0000);
	/* Line Side Ports 1,2,3 shut down */
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF400, 0x0008);
	/* Host Side Ports 1,2,3 shut down */
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF401, 0x0008);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF403, 0x0000);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF002, 0x7172);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0x9041, 0x0001);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0x81FB, 0x8a84);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0x8060, 0x6b6b);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB841, 0xe000);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2014);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2114);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2214);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2314);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2414);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2514);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2614);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2474);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2826);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2938);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2A4A);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2b5C);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2C6D);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2D7E);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2E8F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x2F9F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3014);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3126);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3238);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x334A);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x345C);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x356D);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x367E);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x378F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x389F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x399F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3A9F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3B9F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3C9F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3D9F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3E9F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E6, 0x3F9F);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1E, 0xB1E7, 0xC000);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF003, 0x8080);
	/* Port 1/2/3 Line Side and host Side, Power down and Soft Reset */
	mvEthPhyXsmiRegWrite(phyAddr + 1, 0X1F, 0xF003, 0xC0C0);
	mvEthPhyXsmiRegWrite(phyAddr + 2, 0X1F, 0xF003, 0xC0C0);
	mvEthPhyXsmiRegWrite(phyAddr + 3, 0X1F, 0xF003, 0xC0C0);
	mvEthPhyXsmiRegWrite(phyAddr, 0X1F, 0xF012, 0x0CBC);

	return MV_OK;
}

void initSgmiiModeLineSide(MV_U32 phyAddr)
{

	/*Line side WAs*/
	mvEthPhyXsmiRegWrite(phyAddr, 0x1E, 0xB841, 0xE000);


	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2014, 0x3FFF); /*b xx10 0000 0001 0100 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2114, 0x3FFF); /*b xx10 0001 0001 0100 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2214, 0x3FFF); /*b xx10 0010 0001 0100 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2314, 0x3FFF); /*b xx10 0011 0001 0100 */

	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2414, 0x3FFF); /*b xx10 0100 0001 0100 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2514, 0x3FFF); /*b xx10 0101 0001 0100 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2614, 0x3FFF); /*b xx10 0110 0001 0100 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2714, 0x3FFF); /*b xx10 0111 0001 0100 */

	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2826, 0x3FFF); /*b xx10 1000 0010 0110 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2938, 0x3FFF); /*b xx10 1001 0011 1000 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2A4A, 0x3FFF); /*b xx10 1010 0100 1010 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2B5C, 0x3FFF); /*b xx10 1011 0101 1100 */

	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2C6D, 0x3FFF); /*b xx10 1100 0110 1101 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2D7E, 0x3FFF); /*b xx10 1101 0111 1110 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2E8F, 0x3FFF); /*b xx10 1110 1000 1111 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x2F9F, 0x3FFF); /*b xx10 1111 1001 1111 */

	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3014, 0x3FFF); /*b xx11 0000 0001 0100 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3126, 0x3FFF); /*b xx11 0001 0010 0110 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3238, 0x3FFF); /*b xx11 0010 0011 1000 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x334A, 0x3FFF); /*b xx11 0011 0100 1010 */

	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x345C, 0x3FFF); /*b xx11 0100 0101 1100 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x356D, 0x3FFF); /*b xx11 0101 0110 1101 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x367E, 0x3FFF); /*b xx11 0110 0111 1110 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x378F, 0x3FFF); /*b xx11 0111 1000 1111 */

	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x389F, 0x3FFF); /*b xx11 1000 1001 1111 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x399F, 0x3FFF); /*b xx11 1001 1001 1111 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3A9F, 0x3FFF); /*b xx11 1010 1001 1111 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3B9F, 0x3FFF); /*b xx11 1011 1001 1111 */

	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3C9F, 0x3FFF); /*b xx11 1100 1001 1111 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3D9F, 0x3FFF); /*b xx11 1101 1001 1111 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3E9F, 0x3FFF); /*b xx11 1110 1001 1111 */
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E6, 0x3F9F, 0x3FFF); /*b xx11 1111 1001 1111 */

	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB1E7, 0xc000, 0xC000); /*b 11xx xxxx xxxx xxxx */

}

/*
 *1G-1G initialization for the 2242p phy
 *
 *
 */
MV_STATUS mvEthX2242PPhy1GInit(MV_U32 ethPortNum)
{
	MV_U32 phyAddr;

	phyAddr = eth10gPhyHalData.phyAddr[ethPortNum];

	/* Chip HW Reset */
	mvEthPhyXsmiRegWrite(phyAddr, 0x1F, 0xF404, 0x4000);

	udelay(1000);

	/*1GX-1GX mode programming - for port 0 & for port 2*/
	mvEthPhyXsmiRegWrite(phyAddr, 0x1F, 0xF002, 0x7b7f);
	mvEthPhyXsmiRegWrite(phyAddr + 2, 0x1F, 0xF002, 0x7b7f);

	udelay(1000);

	/* Host side WAs*/
	mvEthPhyXsmiRegWrite(phyAddr, 0x1E, 0x9041, 0x0001);
	mvEthPhyXsmiRegWrite(phyAddr, 0x1E, 0x81D4, 0x0006);
	mvEthPhyXsmiRegWrite(phyAddr, 0x1E, 0x8194, 0x0600);

	/* check if  EthX2242 phy is connected to XSMI PHY lane1 on the board*/
	if (eth10gPhyHalData.xsmiPhyLaneNum == 1) {
		/* power up PCS of secondary lane(lane1, host side) */
		mvEthPhyXsmiRegWrite(phyAddr, 0x4, 0x2200, 0x1140);
		/* Power down PCS of primary lane(lane0, host side) */
		mvEthPhyXsmiRegWrite(phyAddr, 0x4, 0x2000, 0x1940);
		/* Power up secondary port0 (line side) */
		mvEthPhyXsmiRegWrite(phyAddr, 0x1f, 0xf403, 0xEFE);
		/* configure PCS SGMII for secondary port */
		mvEthPhyXsmiRegWrite(phyAddr, 0x1f, 0xf004, 0x7F);
		/* connect source N0(line side) to source M0(secondary lane, host side) */
		mvEthPhyXsmiRegWrite(phyAddr, 0x1f, 0xf400, 0x00C);
		/* disable all primary lanes(line side) */
		mvEthPhyXsmiRegWrite(phyAddr, 0x1f, 0xf401, 0x000);
		/* connect source M0(secondary) to N0 */
		mvEthPhyXsmiRegWrite(phyAddr, 0x1f, 0xf402, 0x008);
	}

	initSgmiiModeLineSide(phyAddr);

	mvEthPhyXsmiRegWrite(phyAddr, 0x1E, 0xB1AA, 0x0);
	mvEthPhyXsmiRegWrite(phyAddr, 0x1E, 0xB1AB, 0x0);
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0xB000, 0x8000, 0x8000);

	/*PCS Reset, port - 0 - 3 */
	mvEthPhyXsmiRegWrite(phyAddr, 0x1F, 0xF003, 0x8080);
	mvEthPhyXsmiRegWrite(phyAddr + 1, 0x1F, 0xF003, 0x8080);
	mvEthPhyXsmiRegWrite(phyAddr + 2, 0x1F, 0xF003, 0x8080);
	mvEthPhyXsmiRegWrite(phyAddr + 3, 0x1F, 0xF003, 0x8080);

	udelay(1000);

	/*TX_DISABLE write - port 0-3 */
	mvEthPhyXsmiRegWrite(phyAddr, 0x1F, 0xF012, 0x0CBC);
	mvEthPhyXsmiRegWrite(phyAddr + 1, 0x1F, 0xF012, 0x0CBC);
	mvEthPhyXsmiRegWrite(phyAddr + 2, 0x1F, 0xF012, 0x0CBC);
	mvEthPhyXsmiRegWrite(phyAddr + 3, 0x1F, 0xF012, 0x0CBC);

	return MV_OK;
}

MV_STATUS mvEth10gPhyWaitRxauiLink(MV_U32 ethPortNum)
{
	MV_U32 phyAddr;
	MV_U16 regVal, timeout = 0;

	phyAddr = eth10gPhyHalData.phyAddr[ethPortNum];

	/* wait until link is up */
	do {
		udelay(1000);
		mvEthPhyXsmiRegRead(phyAddr, 0x3, 0x1, &regVal);
		timeout++;
	} while (((regVal & 0x4) == 0) && (timeout < PHY_TIMEOUT));

	if (timeout >= PHY_TIMEOUT) {
		mvOsPrintf("\nFAILED to get link on PHY\n");
		return MV_FAIL;
	}

	return MV_OK;
}

MV_STATUS mvEth10gPhyWaitSgmiiLink(MV_U32 ethPortNum)
{
	MV_U32 phyAddr;
	MV_U16 regVal, timeout = 0;

	phyAddr = eth10gPhyHalData.phyAddr[ethPortNum];

	/* wait until link is up */
	do {
		udelay(1000);
		mvEthPhyXsmiRegRead(phyAddr, 0x3, 0x2001, &regVal);
		timeout++;
	} while (((regVal & 0x4) == 0) && (timeout < PHY_TIMEOUT));

	if (timeout >= PHY_TIMEOUT) {
		mvOsPrintf("\nFAILED to get link on PHY\n");
		return MV_FAIL;
	}

	return MV_OK;
}

/*
 *2.5G-2.5G initialization for the 2242p phy
 *
 *
 */
MV_STATUS mvEthX2242PPhy2_5GInit(MV_U32 ethPortNum)
{
	MV_U32 phyAddr;

	phyAddr = eth10gPhyHalData.phyAddr[ethPortNum];

	/* Chip HW Reset */
	mvEthPhyXsmiRegWrite(phyAddr, 0x1F, 0xF404, 0x4000);

	udelay(1000);
	/*2.5G-2.5G mode programming*/
	mvEthPhyXsmiRegWrite(phyAddr, 0x1F, 0xF002, 0x7676);
	mvEthPhyXsmiRegWrite(phyAddr + 1, 0x1F, 0xF002, 0x7676);
	mvEthPhyXsmiRegWrite(phyAddr + 2, 0x1F, 0xF002, 0x7676);
	mvEthPhyXsmiRegWrite(phyAddr + 3, 0x1F, 0xF002, 0x7676);

	udelay(1000);

	/* Host side WAs*/
	mvEthPhyXsmiRegWrite(phyAddr, 0x1E, 0x9041, 0x0001);
	mvEthPhyXsmiRegWriteMask(phyAddr, 0x1E, 0x8066, 0x8000, 0x8000);
	mvEthPhyXsmiRegWrite(phyAddr, 0x1E, 0x818F, 0x0600);

	initSgmiiModeLineSide(phyAddr);

	/*PCS Reset, port - 0 - 3 */
	mvEthPhyXsmiRegWrite(phyAddr, 0x1F, 0xF003, 0x8080);
	mvEthPhyXsmiRegWrite(phyAddr + 1, 0x1F, 0xF003, 0x8080);
	mvEthPhyXsmiRegWrite(phyAddr + 2, 0x1F, 0xF003, 0x8080);
	mvEthPhyXsmiRegWrite(phyAddr + 3, 0x1F, 0xF003, 0x8080);

	udelay(1000);

	return MV_OK;
}

MV_STATUS initSgmiiMode(MV_U32 ethPortNum, PHY_10G_MODE mode)
{

	switch (mode) {
	case SGMII_1G_1G_MODE:
		return mvEthX2242PPhy1GInit(ethPortNum);
	case SGMII_2_5G_2_5G_MODE:
		return mvEthX2242PPhy2_5GInit(ethPortNum);
	default:
		mvOsPrintf("Error - unknown SGMII mode\n");
	}
	return MV_FAIL;
}

/*******************************************************************************
* mvEth10gPhyInit -
*
* DESCRIPTION:
*       Initialize the ethernet 10G phy unit.
*
* INPUT:
*       ethPortNum - The port number on which to initialize the PHY.
*	eeeEnable  - Whether to enable EEE or not.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK on success, MV_ERROR otherwise.
*
*******************************************************************************/
MV_STATUS mvEth10gPhyInit(MV_U32 ethPortNum, PHY_10G_MODE phyMode, MV_BOOL eeeEnable)
{
	MV_U32     phyAddr = 0;
	MV_U16     deviceId;
	MV_U16     id1, id2;

	if ((phyMode == SGMII_1G_1G_MODE) || (phyMode == SGMII_2_5G_2_5G_MODE))
		return initSgmiiMode(ethPortNum, phyMode);

	if (ethPortNum != ((MV_U32) -1))
		phyAddr = eth10gPhyHalData.phyAddr[ethPortNum];

	/* Reads ID1 */
	if (mvEthPhyXsmiRegRead(phyAddr, 2, 2, &id1) != MV_OK) {
		mvOsPrintf("%s: Port%d: phyAddr=0x%x -  phy read id1 failed\n", __func__, ethPortNum, phyAddr);
		return MV_ERROR;
	}

	/* Reads ID2 */
	if (mvEthPhyXsmiRegRead(phyAddr, 2, 3, &id2) != MV_OK) {
		mvOsPrintf("%s: Port%d: phyAddr=0x%x -  phy read id2 failed\n", __func__, ethPortNum, phyAddr);
		return MV_ERROR;
	}

	if (!MV_IS_MARVELL_OUI(id1, id2)) {
		mvOsPrintf("%s: Port%d: phyAddr=0x%x, Not Marvell PHY id1 %x id2 %x\n",
				__func__, ethPortNum, phyAddr, id1, id2);
		return MV_ERROR;
	}

	deviceId = (id2 & PHY_MODEL_MASK) >> PHY_MODEL_OFFSET;
	switch (deviceId) {
	case MV_PHY_88X2242P:
		mvEthX2242PPhyBasicInit(ethPortNum);
		break;
	default:
		mvOsPrintf("%s: Unknown Device(%#x). Initialization failed\n", __func__ , deviceId);
		return MV_ERROR;
	}
	return MV_OK;
}

