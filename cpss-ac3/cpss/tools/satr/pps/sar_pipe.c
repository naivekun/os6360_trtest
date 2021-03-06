
#include "sar_sw_lib.h"

/* help descriptions */
static char
        *h_spi_addr_w = "The Address width for SPI transactions\n"
            "0 = 24bit\n"
            "1 = 32bit\n",
        *h_cm3_0_boot_e = "Enables CM3-0 to perform self boot from SPI-Flash\n"
            "0 - Disable self boot\n"
            "1 - Enable self boot\n",
        *h_cm3_1_boot_e = "Enables CM3-1 to perform self boot from SPI-Flash\n"
            "0 - Disable self boot\n"
            "1 - Enable self boot\n",
        *h_eeprom_en = "Enable EEPROm read\n"
            "0 = Disable reading the EEPROM before CPU boot\n"
            "1 = Enable reading the EEPROM before CPU boot\n",
        *h_dev_mod = "Device mode\n"
            "No official options",
	    *h_pll0 = "Core clock and PHA (programmable header alteration) clock frequencies\n"
		    "\t0 = core 500 MHz, PHA 1000MHz \n"
		    "\t1 = core 450 MHz , PHA 892MHz \n"
		    "\t2 = core 287.5 MHz , PHA 892MHz \n"
		    "\t3 = core 350 MHz , PHA 892MHz \n"
		    "\t4 -7 = Reserved \n",
	    *h_boardid = "BoardID\n"
            "0-7 boardId\n",
        *h_pci_ref_clk = "PCIe Ref clock source\n"
            "0 = External 100MHz input from PEX_CLK_P/N\n"
            "1 = Internally generated by PLL\n";

/* PCA9560PW	is used for all SatRs configurations (0x4c, 0x4d, 0x4f, 0x4e)
 * PCA9555	is used for all Serdes configurations (0x20)
 */
struct satr_info pipe_satr_info[] = {
/*	  name		    twsi_addr twsi_reg field_off bit_mask moreThen256 default  help		pca9555*/
	{"dev-mod"      ,	0x4e,	0,		0,		    0xf,	MV_FALSE,	0x0,	&h_dev_mod,	    MV_FALSE},
	{"EEPROM-En"    ,	0x4e,	0,		4,		    0x1,	MV_FALSE,	0x0,	&h_eeprom_en,	MV_FALSE},
	{"pll0-config"  , 	0x4d,	0,		0,		    0x7,	MV_FALSE,	0x1,	&h_pll0,	    MV_FALSE},
	{"CM3-0-boot-en", 	0x4d,	0,		3,		    0x1,	MV_FALSE,	0x0,	&h_cm3_0_boot_e,MV_FALSE},
    {"CM3-1-boot-en", 	0x4d,	0,		4,		    0x1,	MV_FALSE,	0x0,	&h_cm3_1_boot_e,MV_FALSE},
	{"boardid"      ,	0x4c,	0,		0,		    0x7,	MV_FALSE,	0x0,	&h_boardid,	    MV_FALSE},
    {"SPI-addr-width",	0x4c,	0,		3,		    0x1,	MV_FALSE,	0x1,	&h_spi_addr_w,	MV_FALSE},
    {"PCI-ref-clk"  ,   0x4c,	0,		4,		    0x1,	MV_FALSE,	0x0,	&h_pci_ref_clk,	MV_FALSE},
	/* the "LAST entry should be always last - it is used for SatR max options calculation */
	{"LAST"        ,	0x0,	0,		0,		    0x0,	MV_FALSE,	0x0,	NULL,		MV_FALSE},
};

