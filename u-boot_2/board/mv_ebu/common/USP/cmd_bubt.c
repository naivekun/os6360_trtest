/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

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

*******************************************************************************/

#include <config.h>
#include <common.h>
#include "mvCommon.h"
#include <command.h>
#include <net.h>
#include <environment.h>

#if defined(MV_INCLUDE_USB)
#include <usb.h>
#endif

#include <fs.h>

#define	DESTINATION_STRING	10

#if defined(CONFIG_CMD_NAND)
#include <nand.h>
extern nand_info_t nand_info[];       /* info for NAND chips */
#endif

#ifdef CONFIG_CMD_SF
#include <spi_flash.h>
extern struct spi_flash *flash;
#endif

#ifdef CONFIG_CMD_FLASH
#include <flash.h>
extern flash_info_t flash_info[];       /* info for FLASH chips */
#endif

#if defined(MV_MMC_BOOT)
#include <mmc.h>
extern ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt, const void *src);
#endif

#if 0 /* def MV_NOR_BOOT */
static unsigned int flash_in_which_sec(flash_info_t *fl,unsigned int offset)
{
	unsigned int sec_num;
	if(NULL == fl)
		return 0xFFFFFFFF;

	for( sec_num = 0; sec_num < fl->sector_count ; sec_num++){
		/* If last sector*/
		if (sec_num == fl->sector_count -1)
		{
			if((offset >= fl->start[sec_num]) &&
			   (offset <= (fl->size + fl->start[0] - 1)) )
			{
				return sec_num;
			}

		}
		else
		{
			if((offset >= fl->start[sec_num]) &&
			   (offset < fl->start[sec_num + 1]) )
			{
				return sec_num;
			}

		}
	}
	/* return illegal sector Number */
	return 0xFFFFFFFF;

}
#endif /* !defined(MV_NOR_BOOT) */

#ifdef MV_INCLUDE_USB
/*
 * Load u-boot bin file from usb device
 * file_name is the name of u-boot file
 */
int load_from_usb(const char* file_name)
{
	const char *addr_str;
	unsigned long addr;
	int filesize = 0;

	usb_stop();
	printf("(Re)start USB...\n");

	if (usb_init() < 0) {
		printf("usb_init failed\n");
		return 0;
	}

	/* try to recognize storage devices immediately */
	if (-1 == usb_stor_scan(1)) {
		printf("USB storage device not found\n");
		return 0;
	}

	/* always load from usb 0 */
	if (fs_set_blk_dev("usb", "0", FS_TYPE_ANY))
	{
		printf("USB 0 not found\n");
		return 0;
	}

	addr_str = getenv("loadaddr");
	if (addr_str != NULL)
		addr = simple_strtoul(addr_str, NULL, 16);
	else
		addr = CONFIG_SYS_LOAD_ADDR;

	filesize = fs_read(file_name, addr, 0, 0);
	return filesize;
}
#endif

/*
 * Extract arguments from bubt command line
 * argc, argv are the input arguments of bubt command line
 * loadfrom is a pointer to the extracted argument: from where to load the u-boot bin file
 * destination_burn is a pointer to a string which denotes the bubt interface
 */
MV_STATUS fetch_bubt_cmd_args(int argc, char * const argv[], int *loadfrom, char *destination_burn)
{
	*loadfrom = 0;
	strcpy(destination_burn,"default");
	/* bubt */
	if (argc < 2) {
		copy_filename (BootFile, "u-boot.bin", sizeof(BootFile));
		printf("Using default filename \"u-boot.bin\" \n");
	}
	/* "bubt filename" or "bubt destination"*/
	else if (argc == 2) {
		if ((0 == strcmp(argv[1], "spi")) || (0 == strcmp(argv[1], "nand"))
				|| (0 == strcmp(argv[1], "nor")))
		{
			strcpy(destination_burn, argv[1]);
			copy_filename (BootFile, "u-boot.bin", sizeof(BootFile));
			printf("Using default filename \"u-boot.bin\" \n");
		}
		else
		{
			copy_filename (BootFile, argv[1], sizeof(BootFile));
		}
	}
	/* "bubt filename destination" or "bubt destination source" */
	else if (argc == 3) {
		if ((0 == strcmp(argv[1], "spi")) || (0 == strcmp(argv[1], "nand"))
				|| (0 == strcmp(argv[1], "nor")))
		{
			strcpy(destination_burn, argv[1]);
			copy_filename (BootFile, "u-boot.bin", sizeof(BootFile));
			printf("Using default filename \"u-boot.bin\" \n");

			if (0 == strcmp("usb", argv[2])) {
#ifdef MV_INCLUDE_USB
				*loadfrom = 1;
#else
				printf("Error: USB is not supported on current machine\n");
				return MV_ERROR;
#endif
			}

		}
		else
		{
			strcpy(destination_burn, argv[2]);

			copy_filename (BootFile, argv[1], sizeof(BootFile));
		}
	}
	/* "bubt filename destination source" */
	else
	{
		strcpy(destination_burn, argv[2]);

		copy_filename (BootFile, argv[1], sizeof(BootFile));

		if (0 == strcmp("usb", argv[3])) {
#ifdef MV_INCLUDE_USB
			*loadfrom = 1;
#else
			printf("Error: USB is not supported on current machine\n");
			return MV_ERROR;
#endif
		}
	}
	return MV_OK;
}

/*
 * Load u-boot bin file into ram from external device: tftp, usb or other devices
 * loadfrom specifies the source device: tftp, usb or other devices
 * (currently only tftp and usb are supported )
 */
int fetch_uboot_file (int loadfrom)
{
	int filesize = 0;
	switch (loadfrom) {
#ifdef MV_INCLUDE_USB
		case 1:
		{
			filesize = load_from_usb(BootFile);
			if (filesize <= 0)
			{
				printf("Failed to read file %s\n", BootFile);
				return 0;
			}
			break;
		}
#endif
		default:
		{
			filesize = NetLoop(TFTPGET);
			printf("Checking file size:");
			if (filesize == -1)
			{
				printf("\t[Fail]\n");
				return 0;
			}
			break;
		}
	}
	return filesize;
}

#if defined(MV_NAND_BOOT) || defined(MV_NAND)
/* Boot from NAND flash */
/* Write u-boot image into the nand flash */
int nand_burn_uboot_cmd(cmd_tbl_t *cmdtp, int flag, int loadfrom, int argc, char * const argv[])
{
	int filesize = 0;
	MV_U32 ret = 0;
	extern char console_buffer[];
	nand_info_t *nand = &nand_info[0];
	size_t blocksize = nand_info[0].erasesize;
	size_t env_offset = CONFIG_ENV_OFFSET_NAND;
	size_t size = CONFIG_UBOOT_SIZE;
	size_t offset = 0;

	/* Align U-Boot size to currently used blocksize */
	size = ((size + (blocksize - 1)) & (~(blocksize-1)));

#if defined(CONFIG_SKIP_BAD_BLOCK)
	int i = 0;
	int sum = 0;

	while(i * blocksize < nand_info[0].size) {
		if (!nand_block_isbad(&nand_info[0], (i * blocksize)))
			sum += blocksize;
		else {
			sum = 0;
			offset = (i + 1) * blocksize;
		}

		if (sum >= size)
			break;
		i++;
	}
#endif

	/* verify requested source is valid */
	if ((filesize = fetch_uboot_file (loadfrom)) <= 0)
		return 0;

	if (filesize > CONFIG_UBOOT_SIZE) {
		printf("Boot image is too big. Maximum size is %d bytes\n", CONFIG_UBOOT_SIZE);
		return 0;
	}
	printf("\t[Done]\n");
	printf("Override Env parameters to default? [y/N]");
	readline(" ");
	if( strcmp(console_buffer,"Y") == 0 ||
	    strcmp(console_buffer,"yes") == 0 ||
	    strcmp(console_buffer,"y") == 0 ) {

		printf("Erasing 0x%x - 0x%x:",env_offset, env_offset + CONFIG_ENV_RANGE_NAND);
		nand_erase(nand, env_offset, CONFIG_ENV_RANGE_NAND);
		printf("\t[Done]\n");
	}

	printf("Erasing 0x%x - 0x%x: ", offset, offset + size);
	nand_erase(nand, offset, size);
	printf("\t[Done]\n");

	printf("Writing image to NAND:");
	ret = nand_write(nand, offset, &size, (u_char *)load_addr);
	if (ret)
		printf("\t[Fail]\n");
	else
		printf("\t[Done]\n");

	return 1;
}

#endif /* defined(CONFIG_NAND_BOOT) */

#if defined(MV_SPI_BOOT) || defined(MV_INCLUDE_SPI)

/* Boot from SPI flash */
/* Write u-boot image into the SPI flash */
int spi_burn_uboot_cmd(cmd_tbl_t *cmdtp, int flag, int loadfrom, int argc, char * const argv[])
{
	int filesize = 0;
	MV_U32 ret = 0;
	extern char console_buffer[];
	load_addr = CONFIG_SYS_LOAD_ADDR;

	if(!flash) {
		flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
								CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
		if (!flash) {
			printf("Failed to probe SPI Flash\n");
			set_default_env("!spi_flash_probe() failed");
			return 0;
		}
	}

	/* verify requested source is valid */
	if ((filesize = fetch_uboot_file (loadfrom)) <= 0)
		return 0;

	printf("\t\t[Done]\n");
	printf("Override Env parameters to default? [y/N]");
	readline(" ");

#ifdef CONFIG_SPI_FLASH_PROTECTION
	printf("Unprotecting flash:");
	spi_flash_protect(flash, 0);
	printf("\t\t[Done]\n");
#endif
	if( strcmp(console_buffer,"Y") == 0 ||
	    strcmp(console_buffer,"yes") == 0 ||
	    strcmp(console_buffer,"y") == 0 ) {

		printf("Erasing 0x%x - 0x%x:",CONFIG_ENV_OFFSET_SPI, CONFIG_ENV_OFFSET_SPI + CONFIG_ENV_SIZE_SPI);
		spi_flash_erase(flash, CONFIG_ENV_OFFSET_SPI, CONFIG_ENV_SIZE_SPI);
		printf("\t[Done]\n");
	}
	if (filesize > CONFIG_ENV_OFFSET_SPI)
	{
		printf("Error: Image size (%x) exceeds environment variables offset (%x). ",filesize,CONFIG_ENV_OFFSET);
		return 0;
	}
	printf("Erasing 0x%x - 0x%x: ",0, 0 + CONFIG_ENV_OFFSET_SPI);
	spi_flash_erase(flash, 0, CONFIG_ENV_OFFSET_SPI);
	printf("\t\t[Done]\n");

	printf("Writing image to flash:");
	ret = spi_flash_write(flash, 0, filesize, (const void *)load_addr);

	if (ret)
		printf("\t\t[Err!]\n");
	else
		printf("\t\t[Done]\n");

#ifdef CONFIG_SPI_FLASH_PROTECTION
	printf("Protecting flash:");
	spi_flash_protect(flash, 1);
	printf("\t\t[Done]\n");
#endif
	return 1;
}

#endif


#if defined(MV_NOR_BOOT) || (MV_INCLUDE_NOR)

/* Boot from Nor flash */
int nor_burn_uboot_cmd(cmd_tbl_t *cmdtp, int flag, int loadfrom, int argc, char * const argv[])
{
	int filesize = 0;
	MV_U32 ret = 0;
	extern char console_buffer[];
	load_addr = CONFIG_SYS_LOAD_ADDR;
//	MV_U32 s_first,s_end,env_sec;

	ulong stop_addr;
	ulong start_addr;

//	s_first = flash_in_which_sec(&flash_info[0], CONFIG_SYS_MONITOR_BASE);
//	s_end = flash_in_which_sec(&flash_info[0], CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN -1);
//	env_sec = flash_in_which_sec(&flash_info[0], CONFIG_ENV_ADDR);


	/* verify requested source is valid */
	if ((filesize = fetch_uboot_file (loadfrom)) <= 0)
		return 0;

	printf("\t[Done]\n");
	printf("Override Env parameters to default? [y/N]");
	readline(" ");
	if( strcmp(console_buffer,"Y") == 0 ||
		   strcmp(console_buffer,"yes") == 0 ||
		   strcmp(console_buffer,"y") == 0 ) {

		start_addr = CONFIG_ENV_ADDR;
		stop_addr = start_addr + CONFIG_ENV_SIZE - 1;

		printf("Erasing sector 0x%x:",CONFIG_ENV_OFFSET);
		flash_sect_protect (0, start_addr, stop_addr);

		flash_sect_erase (start_addr, stop_addr);

		flash_sect_protect (1, start_addr, stop_addr);
		printf("\t[Done]\n");
	}

	start_addr = NOR_CS_BASE;
	stop_addr = start_addr + CONFIG_ENV_OFFSET - 1;

	flash_sect_protect (0, start_addr, stop_addr);

	printf("Erasing 0x%x - 0x%x: ", (unsigned int)start_addr, (unsigned int)(start_addr + CONFIG_ENV_OFFSET));
	flash_sect_erase (start_addr, stop_addr);
	printf("\t[Done]\n");

	printf("Writing image to NOR:");
	ret = flash_write((char *)CONFIG_SYS_LOAD_ADDR, start_addr, filesize);

	if (ret)
	   printf("\t[Err!]\n");
	else
	   printf("\t[Done]\n");

	flash_sect_protect (1, start_addr, stop_addr);
	return 1;
}

#endif /* MV_NOR_BOOT */

#if defined(MV_MMC_BOOT)

/* Boot from SD/MMC/eMMC */
/* Write u-boot image into SD/MMC/eMMC device */
int mmc_burn_uboot_cmd(cmd_tbl_t *cmdtp, int flag, int loadfrom, int argc, char * const argv[])
{
	int filesize = 0;
	extern char console_buffer[];
	load_addr = CONFIG_SYS_LOAD_ADDR;
	lbaint_t	start_lba;
	lbaint_t	blk_count;
	ulong		blk_written;
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct mmc	*mmc;

	start_lba = CONFIG_ENV_ADDR / CONFIG_ENV_SECT_SIZE;
	blk_count = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;

	mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);
	if (!mmc) {
		printf("No SD/MMC/eMMC card found\n");
		return 1;
	}

	if (mmc_init(mmc)) {
		printf("%s(%d) init failed\n", IS_SD(mmc) ? "SD" : "MMC", CONFIG_SYS_MMC_ENV_DEV);
		return 1;
	}

#ifdef CONFIG_SYS_MMC_ENV_PART
	if (CONFIG_SYS_MMC_ENV_PART != mmc->part_num) {
		if (mmc_switch_part(CONFIG_SYS_MMC_ENV_DEV, CONFIG_SYS_MMC_ENV_PART)) {
			printf("MMC partition switch failed\n");
			return 1;
		}
	}
#endif

	/* verify requested source is valid */
	if ((filesize = fetch_uboot_file (loadfrom)) <= 0)
		return 0;

	printf("\t\t[Done]\n");
	printf("Override Env parameters to default? [y/N]");
	readline(" ");

	if( strcmp(console_buffer,"Y")   == 0 ||
	    strcmp(console_buffer,"yes") == 0 ||
	    strcmp(console_buffer,"y")   == 0 ) {

		printf("Erasing 0x"LBAF" blocks starting at sector 0x"LBAF" :", blk_count, start_lba);
		memset(buf, 0, CONFIG_ENV_SIZE);
		blk_written = mmc_bwrite(CONFIG_SYS_MMC_ENV_DEV, start_lba, blk_count, buf);
		if (blk_written != blk_count) {
			printf("\t[FAIL] - erased %#lx blocks\n", blk_written);
			return 0;
		} else
			printf("\t[Done]\n");
	}
	if (filesize > CONFIG_ENV_OFFSET) {
		printf("Error: Image size (%x) exceeds environment variables offset (%x). ", filesize, CONFIG_ENV_OFFSET);
		return 0;
	}

	/* SD reserves LBA-0 for MBR and boots from LBA-1, MMC/eMMC boots from LBA-0 */
	start_lba = IS_SD(mmc) ? 1 : 0;
	blk_count = filesize / CONFIG_ENV_SECT_SIZE;
	if (filesize % CONFIG_ENV_SECT_SIZE)
		blk_count += 1;

	printf("Writing image to %s(%d) at LBA 0x"LBAF" (0x"LBAF" blocks):",
		   IS_SD(mmc) ? "SD" : "MMC", CONFIG_SYS_MMC_ENV_DEV, start_lba, blk_count);
	blk_written = mmc_bwrite(CONFIG_SYS_MMC_ENV_DEV, start_lba, blk_count, (char *)CONFIG_SYS_LOAD_ADDR);
	if (blk_written != blk_count) {
		printf("\t[FAIL] - written %#lx blocks\n", blk_written);
		return 0;
	} else
		printf("\t[Done]\n");

#ifdef CONFIG_SYS_MMC_ENV_PART
	if (CONFIG_SYS_MMC_ENV_PART != mmc->part_num)
		mmc_switch_part(CONFIG_SYS_MMC_ENV_DEV, mmc->part_num);
#endif

	return 1;
}

#endif

int burn_uboot_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char	destination_burn[DESTINATION_STRING];
	int	loadfrom = 0; /* 0 - from tftp, 1 - from USB */

	memset(destination_burn, '\0', sizeof(destination_burn));

	if(fetch_bubt_cmd_args(argc, argv, &loadfrom,destination_burn) != MV_OK)
		return 0;

#if defined(MV_CROSS_FLASH_BOOT)
	if (0 == strcmp(destination_burn, "nand")) {
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
		return nand_burn_uboot_cmd(cmdtp, flag, loadfrom, argc, argv);
#endif
		printf("\t[FAIL] - u-boot does not support a write to %s interface.\n",destination_burn);
		return 0;
	}

	if (0 == strcmp(destination_burn, "spi")) {
#if defined(MV_INCLUDE_SPI) || defined (MV_SPI_BOOT)
		return spi_burn_uboot_cmd(cmdtp, flag, loadfrom, argc, argv);
#endif
		printf("\t[FAIL] - u-boot does not support a write to %s interface.\n",destination_burn);
		return 0;
	}

	if (0 == strcmp(destination_burn, "nor")) {
#if defined(MV_INCLUDE_NOR) || defined (MV_NOR_BOOT)
		return nor_burn_uboot_cmd(cmdtp, flag, loadfrom, argc, argv);
#endif
		printf("\t[FAIL] - u-boot does not support a write to %s interface.\n",destination_burn);
		return 0;
	}
#endif

#if defined(MV_NAND_BOOT)
		return nand_burn_uboot_cmd(cmdtp, flag, loadfrom, argc, argv);
#endif
#if defined(MV_SPI_BOOT)
		return spi_burn_uboot_cmd(cmdtp, flag, loadfrom, argc, argv);
#endif
#if defined(MV_NOR_BOOT)
		return nor_burn_uboot_cmd(cmdtp, flag, loadfrom, argc, argv);
#endif
#if defined(MV_MMC_BOOT)
		return mmc_burn_uboot_cmd(cmdtp, flag, loadfrom, argc, argv);
#endif

		return 1;
}

#if defined(MV_MMC_BOOT)
U_BOOT_CMD(
		bubt,      3,     0,      burn_uboot_cmd,
		"bubt	- Burn an image on the Boot device.\n",
		" file-name \n"
		"[file-name] [source] \n"
		"\tBurn a binary image on the Boot Device, default file-name is u-boot.bin .\n"
		"\tsource can be tftp or usb, default is tftp.\n"
);
#else
U_BOOT_CMD(
		bubt,      4,     0,      burn_uboot_cmd,
		"bubt	- Burn an image on the Boot flash device.\n",
		"[file-name] [destination [source]] \n"
		"\tBurn a binary image on the Boot Device, default file-name is u-boot.bin .\n"
		"\tsource can be tftp or usb, default is tftp.\n"
		"\texample: bubt u-boot.bin nand tftp\n"
		);
#endif
