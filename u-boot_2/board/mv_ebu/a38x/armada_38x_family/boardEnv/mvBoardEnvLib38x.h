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

*	Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

*	Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

*	Neither the name of Marvell nor the names of its contributors may be
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum _boardMacSpeed {
	BOARD_MAC_SPEED_10M,
	BOARD_MAC_SPEED_100M,
	BOARD_MAC_SPEED_1000M,
	BOARD_MAC_SPEED_2000M,
	BOARD_MAC_SPEED_AUTO,
	BOARD_MAC_UNCONNECTED
} MV_BOARD_MAC_SPEED;

typedef struct _boardMacInfo {
	MV_BOARD_MAC_SPEED boardMacSpeed;
	MV_32 boardEthSmiAddr;
	MV_32 boardEthSmiAddr0;
	MV_BOOL boardMacEnabled;
} MV_BOARD_MAC_INFO;

enum {
	MV_PORT_TYPE_SGMII,
	MV_PORT_TYPE_QSGMII,
	MV_PORT_TYPE_RGMII,
	MV_PORT_TYPE_UNKNOWN = -1,
};

typedef enum _mvModuleTypeID {
	MV_MODULE_NO_MODULE                     = 0x000,/* MII board SLM 1362   */
	MV_MODULE_MII                           = BIT0, /* MII board SLM 1362   */
	MV_MODULE_SLIC_TDM_DEVICE               = BIT1, /* TDM board SLM 1360   */
	MV_MODULE_I2S_DEVICE                    = BIT2, /* I2S board SLM 1360   */
	MV_MODULE_SPDIF_DEVICE                  = BIT3, /* SPDIF board SLM 1360 */
	MV_MODULE_NOR                           = BIT4, /* NOR board SLM 1361   */
	MV_MODULE_NAND                          = BIT5, /* NAND board SLM 1361  */
	MV_MODULE_SDIO                          = BIT6, /* SDIO board SLM 1361  */
	MV_MODULE_SGMII                         = BIT7, /* SGMII board SLM 1364 */
	MV_MODULE_DB381_SGMII                   = BIT8, /* DB-381 SGMII SLM 1426 */
	MV_MODULE_SWITCH                        = BIT9, /* SWITCH board SLM 1375 */
	MV_MODULE_NAND_ON_BOARD                 = BIT10,/* ON board nand - detected via S@R bootsrc */
	MV_MODULE_DB381_MMC_8BIT_ON_BOARD       = BIT11,/* ON board MMC 8bit - detected via S@R bootsrc */
	MV_MODULE_TYPE_MAX_MODULE               = 10,	/* Max detected module */
	MV_MODULE_TYPE_MAX_OPTION               = 11	/* Max module options */
} MV_MODULE_TYPE_ID;


/* {{ConfigID,		     twsi-ID, Offset,	 ID,	isActiveForBoard[]}} */
#define MV_MODULE_INFO { \
{ MV_MODULE_MII,		0x1,	0,	 0x4,	{ 0, 1, 0, 0, 0, 0, 0} }, \
{ MV_MODULE_SLIC_TDM_DEVICE,	0x0,	0,	 0x1,	{ 0, 1, 0, 0, 0, 0, 0} }, \
{ MV_MODULE_I2S_DEVICE,		0x1,	0,	 0x3,	{ 0, 1, 0, 0, 0, 0, 0} }, \
{ MV_MODULE_SPDIF_DEVICE,	0x1,	0,	 0x2,	{ 0, 1, 0, 0, 0, 0, 0} }, \
{ MV_MODULE_NOR,		0x4,	0,	 0xF,	{ 0, 1, 0, 0, 0, 0, 0} }, \
{ MV_MODULE_NAND,		0x4,	0,	 0x1,	{ 0, 1, 0, 0, 0, 0, 0} }, \
{ MV_MODULE_SDIO,		0x4,	0,	 0x2,	{ 0, 1, 0, 0, 0, 0, 0} }, \
{ MV_MODULE_SGMII,		0x2,	0,	 0xF,	{ 0, 1, 0, 0, 0, 0, 0} }, \
{ MV_MODULE_DB381_SGMII,	0x0,	0,	 0x2,	{ 0, 0, 0, 0, 0, 1, 0} }, \
{ MV_MODULE_SWITCH,		0x5,	0,	 0xE,	{ 0, 1, 0, 0, 0, 0, 0} }, \
};

#define MPP_MII_MODULE		{ {0, 0x10111111}, {1, 0x11111111}, {2, 0x11211111} }
#define MPP_TDM_MODULE		{ {6, 0x55333333}, {7, 0x00004444} }
#define MPP_AUDIO_MODULE	{6, 0x55444444}
#define MPP_NOR_MODULE		{ {0, 0x55111111}, {1, 0x15555555}, {2, 0x55566011}, \
				  {3, 0x55555055}, {4, 0x55555555}, {5, 0x40045525 } }
#define MPP_NAND_MODULE		{ {0, 0x55111111}, {1, 0x15555555}, {2, 0x55266011}, \
				  {3, 0x25255051}, {4, 0x25555555}, {5, 0x40042555 } }
#define MPP_SDIO_MODULE		{ {2, 0x11466011}, {3, 0x22242011}, {4, 0x44400002}, {5, 0x40042024 } }
#define MPP_NAND_ON_BOARD	{ {2, 0x55266011}, {3, 0x25252051}, {4, 0x25255555}, {5, 0x40042565 } }
#define MPP_GP_MINI_PCIE0	{2, 0x11240011}
#define MPP_GP_MINI_PCIE1	{2, 0x11204011}
#define MPP_GP_MINI_PCIE0_PCIE1	{2, 0x11200011}
#define MPP_MMC_DB381_MODULE	{3, 0x00044444}

typedef enum _mvSatRTypeID {
/*  "Bios" Device  */
	MV_SATR_CPU_DDR_L2_FREQ,
	MV_SATR_CORE_CLK_SELECT,
	MV_SATR_CPU1_ENABLE,
	MV_SATR_SSCG_DISABLE,
	/*  SW parameters: */
	MV_SATR_DDR4_SELECT,
	MV_SATR_DDR_BUS_WIDTH,
	MV_SATR_DDR_ECC_ENABLE,
	MV_SATR_DDR_ECC_PUP_SEL,
	MV_SATR_SGMII_SPEED,
	MV_SATR_BOOT_DEVICE,
	MV_SATR_BOOT2_DEVICE,
	MV_SATR_BOARD_ID,
	MV_SATR_BOARD_ECO_VERSION,
	MV_SATR_DB_USB3_PORT0,
	MV_SATR_DB_USB3_PORT1,
	MV_SATR_RD_SERDES4_CFG,
	MV_SATR_GP_SERDES5_CFG,
	MV_SATR_DB_SERDES1_CFG,
	MV_SATR_DB_SERDES2_CFG,
	MV_SATR_SGMII_MODE,
	MV_SATR_DEVICE_ID,
	MV_SATR_DEVICE_ID2,
	MV_SATR_GP_SERDES1_CFG,
	MV_SATR_GP_SERDES2_CFG,
	MV_SATR_FULL_FLAVOR,
	MV_SATR_TDM_CONNECTED,
	MV_SATR_TDM_PLACE_HOLDER,
	MV_SATR_BOARD_SPEED,
	MV_SATR_AVS_SKIP,
	MV_SATR_PEX_FORCE_GEN1,
	MV_SATR_MAX_OPTION,
} MV_SATR_TYPE_ID;

/*						bit    TWSI           Reg	board	*/
/*   name		SATR-ID			Mask  offset  devID   num	active	*/
#define MV_SAR_INFO { \
{"freq",	MV_SATR_CPU_DDR_L2_FREQ,	0x1F,	0,	1,	0,	{1, 1, 1, 1, 1, 0, 1}, SATR_SWAP_BIT},\
{"coreclock",	MV_SATR_CORE_CLK_SELECT,	0x04,	2,	3,	0,	{0, 1, 0, 0, 0, 1, 0}, 0},\
{"cpusnum",	MV_SATR_CPU1_ENABLE,		0x10,	4,	3,	0,	{0, 0, 0, 0, 0, 1, 0}, 0},\
{"sscg",	MV_SATR_SSCG_DISABLE,		0x08,	3,	3,	0,	{0, 1, 0, 0, 0, 1, 0}, 0},\
{"ddr4select",	MV_SATR_DDR4_SELECT,		0x20,	5,	4,	1,	{0, 1, 0, 0, 0, 0, 0}, SATR_READ_ONLY},\
{"ddrbuswidth",	MV_SATR_DDR_BUS_WIDTH,		0x08,	3,	0,	0,	{1, 1, 1, 1, 1, 0, 1}, 0},\
{"ddreccenable",	MV_SATR_DDR_ECC_ENABLE,	0x10,	4,	0,	0,	{1, 1, 1, 1, 1, 1, 1}, 0},\
{"ddreccpupselect",	MV_SATR_DDR_ECC_PUP_SEL, 0x20,	5,	0,	0,	{0, 1, 0, 0, 0, 0, 1}, 0},\
{"sgmiispeed", MV_SATR_SGMII_SPEED,		0x40,	6,	0,	0,	{1, 1, 1, 1, 1, 1, 0}, 0},\
{"bootsrc",	MV_SATR_BOOT_DEVICE,		0x3,	0,	3,	0,	{0, 1, 0, 0, 0, 1, 0}, SATR_SWAP_BIT},\
{"boarsrc2",	MV_SATR_BOOT2_DEVICE,		0x1E,	1,	2,	0,	{0, 1, 0, 0, 0, 1, 0}, SATR_SWAP_BIT},\
{"boardid",	MV_SATR_BOARD_ID,		0x7,	0,	0,	0,	{1, 1, 1, 1, 1, 1, 1}, 0},\
{"ecoversion",	MV_SATR_BOARD_ECO_VERSION,	0xff,	0,	0,	1,	{0, 1, 0, 0, 0, 0, 0}, SATR_READ_ONLY},\
{"usb3port0",	MV_SATR_DB_USB3_PORT0,		0x1,	0,	0,	2,	{0, 1, 0, 0, 0, 1, 0}, 0},\
{"usb3port1",	MV_SATR_DB_USB3_PORT1,		0x2,	1,	0,	2,	{0, 1, 0, 0, 0, 1, 0}, 0},\
{"rdserdes4",	MV_SATR_RD_SERDES4_CFG,		0x4,	2,	1,	1,	{1, 0, 1, 0, 0, 0, 0}, 0},\
{"gpserdes5",	MV_SATR_GP_SERDES5_CFG,		0x4,	2,	1,	1,	{0, 0, 0, 0, 1, 0, 0}, 0},\
{"dbserdes1",	MV_SATR_DB_SERDES1_CFG,		0x7,	0,	0,	1,	{0, 1, 0, 0, 0, 1, 0}, 0},\
{"dbserdes2",	MV_SATR_DB_SERDES2_CFG,		0x38,	3,	0,	1,	{0, 1, 0, 0, 0, 1, 0}, 0},\
{"gpserdes1",	MV_SATR_GP_SERDES1_CFG,		0x4,	2,	0,	2,	{0, 0, 0, 0, 1, 0, 0}, 0},\
{"gpserdes2",	MV_SATR_GP_SERDES2_CFG,		0x8,	3,	0,	2,	{0, 0, 0, 0, 1, 0, 0}, 0},\
{"sgmiimode",	MV_SATR_SGMII_MODE,		0x40,	6,	0,	1,	{0, 1, 0, 0, 1, 1, 1}, 0},\
{"devid",	MV_SATR_DEVICE_ID,		0x1,	0,	2,	0,	{0, 1, 0, 0, 0, 0, 0}, 0},\
{"devid2",	MV_SATR_DEVICE_ID2,		0x10,	4,	3,	0,	{0, 1, 0, 0, 0, 0, 0}, 0},\
{"flavor",	MV_SATR_FULL_FLAVOR,		0x10,	4,	0,	2,	{0, 1, 0, 1, 1, 1, 0}, 0},\
{"tdm",		MV_SATR_TDM_CONNECTED,		0x20,	5,	0,	2,	{0, 1, 0, 0, 0, 0, 0}, 0},\
{"tdmplaceholder",	MV_SATR_TDM_PLACE_HOLDER,	0x40,	6,	0,	2,	{0, 0, 0, 0, 0, 0, 0}, 0},\
{"avsskip",	MV_SATR_AVS_SKIP,		0x80,	7,	0,	2,	{0, 1, 0, 1, 1, 1, 1}, 0},\
{"forcegen1",	MV_SATR_PEX_FORCE_GEN1,		0x4,	2,	0,	3,	{0, 0, 0, 0, 0, 0, 1}, 0},\
{"boardspeed",	MV_SATR_BOARD_SPEED,		0x3,	0,	0,	3,	{0, 1, 0, 1, 1, 0, 1}, SATR_READ_ONLY},\
{"max_option",	MV_SATR_MAX_OPTION,		0x0,	0,	0,	0,	{0, 0, 0, 0, 0, 0, 0}, 0},\
};
/* tdm place holder is used for future support for multiple tdm devices */
/* avsskip is used to skip selecting AVS from EFUSE:
 *	- used in binary header by Marvell boards only*/

/* extra SAR table, for different board implementations:
 * in case a field is used on 2 boards with different i2c mapping */
#define MV_SAR_INFO2 { \
{"freq",	MV_SATR_CPU_DDR_L2_FREQ,	0x1E,	1,	1,	0,	{0, 0, 0, 0, 0, 1, 0},  SATR_SWAP_BIT},\
{"coreclock",	MV_SATR_CORE_CLK_SELECT,	0x08,	3,	2,	0,	{0, 0, 0, 0, 1, 0, 0}, 0},\
{"sscg",	MV_SATR_SSCG_DISABLE,		0x10,	4,	2,	0,	{0, 0, 0, 0, 1, 0, 0}, 0},\
{"devid",	MV_SATR_DEVICE_ID,		0x3,	0,	2,	0,	{0, 0, 0, 0, 1, 0, 0}, 0},\
{ "max_option",	MV_SATR_MAX_OPTION,		0x0,	0,	0,	0,	{0, 0, 0, 0, 0, 0, 0}, 0},\
};

#define MV_SATR_BOOT2_VALUE_MASK	0xF
#define MV_SATR_BOOT2_VALUE_OFFSET	2
#define MV_SATR_DEVICE_ID2_VALUE_MASK	1
#define MV_SATR_DEVICE_ID2_VALUE_OFFSET	1

#ifdef CONFIG_CMD_BOARDCFG
#define MV_BOARD_CONFIG_MAX_BYTE_COUNT	8
#define MV_BOARD_CONFIG_DEFAULT_VALUE	{0x1921d0a1, 0x4 }

typedef enum _mvConfigTypeID {
	MV_CONFIG_EXAMPLE0,
	MV_CONFIG_TYPE_MAX_OPTION,
	MV_CONFIG_TYPE_CMD_DUMP_ALL,
	MV_CONFIG_TYPE_CMD_SET_DEFAULT
} MV_CONFIG_TYPE_ID;

#define MV_EEPROM_CONFIG_INFO { \
{ MV_CONFIG_EXAMPLE0,	0x7,		0,	0,	{1, 1, 1, 1} }, \
};

#define MV_BOARD_CONFIG_CMD_STR "example0\n\n"
#define MV_BOARD_CONFIG_CMD_MAX_OPTS 5

/*MV_CMD_TYPE_ID,		command name,		Name,			numOfValues,	Possible Values */
#define MV_BOARD_CONFIG_CMD_INFO {\
{MV_CONFIG_EXAMPLE0,	"example0",	"Example #0",	5,					\
	{"Option0", "Option1", "Option2", "Option3", "Option4"} },				\
};

#endif /* CONFIG_CMD_BOARDCFG */

#ifdef __cplusplus
}
#endif  /* __cplusplus */
