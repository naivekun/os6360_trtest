
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <i2c_lib.h>
#include <mvTwsi.h>

/***********************************************************************
 * CM3 processor tool:
 *
 *   CM3 SRAM is located at offset 0x80000 of the PP address space.
 *   PP register 0x500 controls CM3:
 *      bit [28] - CM3_Enable
 *      bit [20] - init RAM
 *      bit [19] - CM3 clock disable
 **********************************************************************/

#define COMP_REG(_x) (0x120+(_x)*4)
#define BITS19_31(a) (a & 0xfff80000)
#define BITS0_18(a) (a & 0x7ffff)
#define COMP_REG_OFF(reg) (reg<<19)
#define COMP_REG_CTRL_REG 0x140 
/* 32bit byte swap. For example 0x11223344 -> 0x44332211                    */
#define MV_BYTE_SWAP_32BIT(X) ((((X)&0xff)<<24) |                       \
                               (((X)&0xff00)<<8) |                      \
                               (((X)&0xff0000)>>8) |                    \
                               (((X)&0xff000000)>>24))

/* to simplify code bus,dev,func*/
#define BDF pciBus, pciDev, pciFunc
#define BDF_DECL \
    uint32_t  pciBus, \
    uint32_t  pciDev, \
    uint32_t  pciFunc

const uint32_t pciBus = 1;
const uint32_t pciDev = 0;
const uint32_t pciFunc = 0;

void *pp_space_ptr = NULL;
void *dfx_space_ptr = NULL;
void *mg_space_ptr = NULL;

uint32_t direct_0_or_through_pci_1;

static void pp_write_reg(int32_t offset, uint32_t value)
{
	*(volatile uint32_t *)(pp_space_ptr + offset) = value;
}

static uint32_t pp_read_reg(int32_t offset)
{
	return *(volatile uint32_t *)(pp_space_ptr + offset);
}


static void enable_cm3(unsigned int dev_id, bool enable)
{
	uint32_t reg;

	reg = pp_read_reg(0x500);
	if (enable) {
		reg |= (1 << 28); /* Enable CM3 */
        if (0xbe00 != dev_id) {
    		reg |= (1 << 29); /* Enable CM3 */
        } else {
            reg |= (1 << 19); /* bit 19: CM3_PROC_CLK_DIS */
        }
	} else {
		reg &= ~(1 << 28); /* disable CM3 */
        if (0xbe00 != dev_id) {
            reg &= ~(1 << 29); /* disable CM3 */
        } else {
            reg |= 0x00100000; /* bit 20: init ram */
            reg &= 0xfff7ffff; /* bit 19: CM3_PROC_CLK_DIS */ 
        }
    }

	pp_write_reg(0x500, reg);

    if (0xbe00 == dev_id && !enable) {
        reg = pp_read_reg(0x54);
        reg |= 1; // bit 0: CM3_METAL_FIX_JTAG_EN
        pp_write_reg(0x54, reg);
    }
}

int sysfs_pci_open(
	BDF_DECL,
	const char *name,
	int     flags,
	int     *fd
)
{
	char fname[128];
	if (pciBus > 255 || pciDev > 31 || pciFunc > 7)
		return -1;
	sprintf(fname, "/sys/bus/pci/devices/0000:%02x:%02x.%x/%s",
			pciBus, pciDev, pciFunc, name);
	*fd = open(fname, flags);
	if (*fd < 0) {
		perror(fname);
		return -1;
	}
	return 0;
}

int sysfs_pci_map(
	const char *res_name,
	int flags,
	int   *fd,
	void **vaddr
)
{
	int rc;
	struct stat st;

	rc = sysfs_pci_open(BDF, res_name, flags, fd);
	if (rc != 0) {
		perror(res_name);
		return rc;
	}

	if (fstat(*fd, &st) < 0) {
		close(*fd);
		return -1;
	}
	*vaddr = mmap(NULL,
				st.st_size,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				*fd,
				(off_t)0);
	if (MAP_FAILED == *vaddr) {
		perror("mmap");
		close(*fd);
		return -1;
	}
	printf("%s mapped to %p, size=0x%x\n", res_name, *vaddr, (unsigned)st.st_size);
	return 0;
}

void sysfs_pci_write(void* vaddr, int offset, uint32_t value)
{
	/*printf("Write 0x%08x to %p\n", (uint32_t)value, (void*)(((uintptr_t)vaddr)+offset));*/
	*((volatile uint32_t*)(((uintptr_t)vaddr)+offset)) = (uint32_t) value;
}

uint32_t sysfs_pci_read(void* vaddr, int offset)
{
	return(*((volatile uint32_t*)(((uintptr_t)vaddr)+offset)));
}

int eeprom_write_2_words(unsigned char i2c_addr, int offset, uint32_t data0, uint32_t data1) {
	char buffer[8];

	*(uint32_t *)(buffer) = MV_BYTE_SWAP_32BIT(data0);
	*(uint32_t *)(buffer + 4) = MV_BYTE_SWAP_32BIT(data1);
	if( 0 == direct_0_or_through_pci_1 ) {
		if (0 < i2c_transmit_with_offset(i2c_addr, 8, 0, i2c_offset_type_16_E, offset, buffer)) {
			printf("w 0x%08x 0x%08x\n", data0, data1);
			return 0;
		} else {
			printf("i2c write to 0x%02x offset 0x%08x failed\n", i2c_addr, offset);
			return -1;
		}
	}
	else /* 1 == direct_0_or_through_pci_1 */
	{	
		MV_TWSI_SLAVE mv_twsi_slave;

		mv_twsi_slave.slaveAddr.address = i2c_addr << 1;
			mv_twsi_slave.slaveAddr.type = ADDR7_BIT;
			mv_twsi_slave.validOffset = 1;           
			mv_twsi_slave.offset = offset; 
			mv_twsi_slave.offset_length = 2;

		if ( MV_OK == mv_twsi_master_transceive( buffer, 8, &mv_twsi_slave, MV_TWSI_TRANSMIT) ) {
			printf("w 0x%08x 0x%08x\n", data0, data1);
			return 0;
		} else { /* MV_FAIL */
			printf("i2c write to 0x%02x offset 0x%08x failed\n", i2c_addr, offset);
			return -1;
		}
	}
}

int main(int argc, char *argv[])
{
	char copy_buf[256];
	struct stat fw_stat;
	uint32_t reg;

	int fd;
	FILE *f;
	int i = 0, offset;
	unsigned long long start, end, flags;
	int rc;

	int fw_fd = 0, xbar_fd, mg_fd;
	void *cm3_sram_ptr = NULL, *xbar_space_ptr = NULL;

	// sysfs_read_resource physical
	unsigned long long res2, res4;
	uint8_t i2c_addr;
    uint32_t dev_id; 

    uint32_t cm3_offset;
    uint32_t cm3_size;

	rc = sysfs_pci_open(BDF, "resource", O_RDONLY, &fd);
	// printf("%s: %d\n", __FUNCTION__, __LINE__);
	if (rc != 0)
		return rc;


	f = fdopen(fd, "r");
	if (f == NULL)
		return -1;
	while (!feof(f))
	{
		if (fscanf(f, "%lli %lli %lli", &start, &end, &flags) != 3)
			break;
        if (i == 2)
			res2 = start;
		if (i == 4)
			res4 = start;
		i++;
	}
	fclose(f);

	printf("res2 (pysical PP bar 2 addr): %llx\n", res2);

	// Config Aldrin PeX Window 0 to see Switching-Core
	rc = sysfs_pci_map("resource0", O_RDWR, &mg_fd, &mg_space_ptr);
	if (rc != 0)
		return rc;

	sysfs_pci_write(mg_space_ptr, 0x41820, 0);
	sysfs_pci_write(mg_space_ptr, 0x41824, res2);
	sysfs_pci_write(mg_space_ptr, 0x41828, 0);
	sysfs_pci_write(mg_space_ptr, 0x4182C, 0);
	sysfs_pci_write(mg_space_ptr, 0x41820, 0x03ff0031);
	sysfs_pci_write(mg_space_ptr, 0x41830, 0);
	sysfs_pci_write(mg_space_ptr, 0x41834, res4);
	sysfs_pci_write(mg_space_ptr, 0x41838, 0);
	sysfs_pci_write(mg_space_ptr, 0x4183C, 0);
	sysfs_pci_write(mg_space_ptr, 0x41830, 0x001f0083);
	printf("\n");

	/* Init: create mappings for PP and XBAR */
	rc = sysfs_pci_map("resource2", O_RDWR | O_SYNC, &mg_fd, &pp_space_ptr);
	if (rc < 0)
		return -1;

	/* Init: create mappings for DFX */
	rc = sysfs_pci_map("resource4", O_RDWR | O_SYNC, &mg_fd, &dfx_space_ptr);
	if (rc < 0)
		return -1;


/*AC3,bobk
    cm3_sram_ptr=pp_space_prt + 0x40000; */
/*Aldrin
#define CM3_OFFSET  0x40000 //was 0x80000 for aldrin, changed to test bobk cm3 1/15/17
#define CM3_SIZE    0x1F800 //was 0x30000 for aldrin, changed to test bobk cm3 1/15/17*/

    dev_id = (pp_read_reg(0x4c) >> 4) & 0xffff;
    if (dev_id == 0xbe00) {
        /* bobK */
        cm3_offset = 0x40000;
        cm3_size = 0x1F800; 
    } else {
        /* Aldrin / BC3 / pipe */
        cm3_offset = 0x80000 ;
        cm3_size = 0x30000;
    }
    //printf("\n##### dev_id = 0x%x\n",dev_id);

	pp_write_reg(COMP_REG(7), htole32(BITS19_31(cm3_offset)>>19) );
	cm3_sram_ptr = pp_space_ptr + COMP_REG_OFF(7) + BITS0_18(cm3_offset)/* actually 0 */;
	reg = pp_read_reg(COMP_REG_CTRL_REG);
	reg &= ~(1 << 16); /* Set 8 region completion register mode */
	pp_write_reg(COMP_REG_CTRL_REG, reg);

	/* Check inputs */
	if (argc <= 1) {
		printf("Usage:\n");
		printf("       %s <file> - load FW from binary image file to CM3 SRAM and run it\n", argv[0]);
		printf("       %s -w <offset> <value> - write PP register\n", argv[0]);
		printf("       %s -r <offset> - print PP register\n", argv[0]);
		printf("       %s -e <i2c address> <file> - Load FW to EEPROM at specified I2C address\n", argv[0]);
		printf("       %s -p <i2c address> <file> - Load FW to EEPROM at specified I2C address through PCIe\n", argv[0]);
		return 0;
	}

	/* PP write command */
	if (!strcmp(argv[1], "-w")) {
		pp_write_reg(strtoul(argv[2], NULL, 0), strtoul(argv[3], NULL, 0));
		return 0;
	}

	/* PP read command */
	if (!strcmp(argv[1], "-r")) {
		printf("0x%08x\n", pp_read_reg(strtoul(argv[2], NULL, 0)));
		return 0;
	}

	/* EEPROM program command */
	if (!strcmp(argv[1], "-e") || !strcmp(argv[1], "-p")) {

		

		if (!strcmp(argv[1], "-e")) {
			direct_0_or_through_pci_1 = 0;
		} else { /* (!strcmp(argv[1], "-p")) */
			direct_0_or_through_pci_1 = 1;
		}

		if (argc < 4) {
			if (0 == direct_0_or_through_pci_1) {
				printf("Usage: %s -e <i2c address> <file> - Load FW to EEPROM at specified I2C address\n", argv[0]);
			} else { /* (1 == direct_0_or_through_pci_1) */
				printf("Usage: %s -p <i2c address> <file> - Load FW to EEPROM at specified I2C address through PCIe\n", argv[0]);  
			}
			return -1;
		}

		i2c_addr = strtoul(argv[2], NULL, 0);

		fw_fd = open(argv[3], O_RDONLY);
		if (fw_fd <= 0) {
			fprintf(stderr, "Cannot open %s file.\n", argv[3]);
			return -1;
		}

		if (0 == direct_0_or_through_pci_1) {
			i2c_init();
			if (0 < i2c_receive_with_offset (i2c_addr, 0x20, 0, i2c_offset_type_16_E, 0, copy_buf)) {
				printf("i2c read from 0x53 offset 0: 0x%08x\n", *(uint32_t *)copy_buf);
			} else {
				printf("i2c failed\n");
			}
		} else { /* (1 == direct_0_or_through_pci_1) */
			uint32_t mv_rc;
			mv_rc = mv_twsi_init(100000, 250000000 /*250MHZ*/);
			if ( 90000 < mv_rc ) {
				printf("mv_twsi_init passed, actual freq %d\n", mv_rc);
			} else {
				printf("mv_twsi_init failed, actual freq %d\n", mv_rc);
				return -1;
			}
		}

		i = 0x30080002;
		offset = 0;
		eeprom_write_2_words(i2c_addr, offset, 0x8001320c, 0x00010000);
		while (true) {
			rc = read(fw_fd, &reg, 4);
			offset += 8;
			if (rc < 4) {  /* Handle last bytes of file */
				while (rc < 4)
					*(uint8_t *)(&reg + rc++) = 0;
				rc = 0;
			}
			eeprom_write_2_words(i2c_addr, offset, i, reg);
			if (rc < 4)
				break;
			i += 4;
		}
		eeprom_write_2_words(i2c_addr, offset + 8,  0x30000502, 0x300e1a80); /* Take CM3 out of reset */
		eeprom_write_2_words(i2c_addr, offset + 16, 0xffffffff, 0xffffffff); /* Mark end of data */

		return 0;
	}


	/* File load command */
	fw_fd = open(argv[1], O_RDONLY);
	if (fw_fd <= 0) {
		fprintf(stderr, "Cannot open %s file.\n", argv[1]);
		return -1;
	}

    enable_cm3(dev_id, 0); /* Disable CM3 */

	fstat(fw_fd, &fw_stat);
	if (fw_stat.st_size > cm3_size) {
		printf("Error - file too large (%ld), we have only %dKB space\n", fw_stat.st_size, cm3_size/1024);
		close(fw_fd);
		return -1;
	}

    // printf("%s: %d\n", __FUNCTION__, __LINE__);
	read(fw_fd, cm3_sram_ptr, fw_stat.st_size);
	msync(cm3_sram_ptr, fw_stat.st_size, MS_SYNC);
	close(fw_fd);

	printf("successfully loaded file %s, size %ld\n", argv[1], fw_stat.st_size);

	enable_cm3(dev_id, 1); /* Enable CM3 */

	return 0;
}


