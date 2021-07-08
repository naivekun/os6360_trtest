/*
    util.c - helper functions
    Copyright (C) 2006-2009  Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#include "util.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include "i2cbusses.h"
#include <signal.h>
#include "wnc-tool.h"

unsigned long funcs;
char filename[20];

extern int i2cbus;
extern unsigned char i2c_read_buff[2048];
extern char i2c_read_buff_str[4096];
extern int file, force, res;

#if 0
#define DEFAULT_NUM_PAGES    8            /* we default to a 24C16 eeprom which has 8 pages */
#define BYTES_PER_PAGE       256          /* one eeprom page is 256 byte */
#define MAX_BYTES            8            /* max number of bytes to write in one chunk */
       /* ... note: 24C02 and 24C01 only allow 8 bytes to be written in one chunk.   *
        *  if you are going to write 24C04,8,16 you can change this to 16            */

/* write len bytes (stored in buf) to eeprom at address addr, page-offset offset */
/* if len=0 (buf may be NULL in this case) you can reposition the eeprom's read-pointer */
/* return 0 on success, -1 on failure */
int eeprom_write(int fd,
		 unsigned int addr,
		 unsigned int offset,
		 unsigned char *buf,
		 unsigned char len
){
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg             i2cmsg;
	int i;
	char _buf[MAX_BYTES + 1];
	memset(_buf, 0, sizeof(_buf));
	if(len>MAX_BYTES){
	    fprintf(stderr,"I can only write MAX_BYTES bytes at a time!\n");
	    return -1;
	}
	if(len+offset >256){
	    fprintf(stderr,"Sorry, len(%d)+offset(%d) > 256 (page boundary)\n",
			len,offset);
	    return -1;
	}
//WNC_Roger
//	_buf[0]=offset;    /* _buf[0] is the offset into the eeprom page! */
	_buf[0]=offset>>8 ;    /* _buf[0] is the offset into the eeprom page! */
	_buf[1]=offset&0xFF ;    /* _buf[0] is the offset into the eeprom page! */
//WNC
	for(i=0;i<len;i++) /* copy buf[0..n] -> _buf[1..n+1] */
//	    _buf[1+i]=buf[i];
	    _buf[2+i]=buf[i];
	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;
	i2cmsg.addr  = addr;
	printf("i2cmsg.addr=%xh\n", i2cmsg.addr);
	i2cmsg.flags = 0;
	i2cmsg.len   = 1+len;
	i2cmsg.buf   = _buf;
	printf("i2cmsg.buf[0,1,2]=%02x%02x%02xh\n", _buf[0], _buf[1], _buf[2]);
	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
	    perror("ioctl()");
	    fprintf(stderr,"ioctl returned %d\n",i);
	    return -1;
	}
	if(len>0)
	    fprintf(stderr,"Wrote %d bytes to eeprom at 0x%02x, offset %08x\n",
		    len,addr,offset);
	else
	    fprintf(stderr,"Positioned pointer in eeprom at 0x%02x to offset %08x\n",
		    addr,offset);
	return 0;
}

/* read len bytes stored in eeprom at address addr, offset offset in array buf */
/* return -1 on error, 0 on success */
int eeprom_read(int fd,
		 unsigned int addr,
		 unsigned int offset,
		 unsigned char *buf,
		 unsigned char len
){
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg             i2cmsg;
	int i;
	if(len>MAX_BYTES){
	    fprintf(stderr,"I can only write MAX_BYTES bytes at a time!\n");
	    return -1;
	}
	if(eeprom_write(fd,addr,offset,NULL,0)<0)
	    return -1;
	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;
	i2cmsg.addr  = addr;
	printf("i2cmsg.addr=%xh\n", i2cmsg.addr);
	i2cmsg.flags = I2C_M_RD;
	i2cmsg.len   = len;
	i2cmsg.buf   = buf;
	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
	    perror("ioctl()");
	    fprintf(stderr,"ioctl returned %d\n",i);
	    return -1;
	}
	fprintf(stderr,"Read %d bytes from eeprom at 0x%02x, offset %08x\n",
		len,addr,offset);
	return 0;
}
#endif

void wnc_i2c_dev_open(void){
	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
	if (file < 0) {
		exit(1);
	}
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		close(file);
		exit(1);
	}
	if (!(funcs & (I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_READ_BYTE))) {
		fprintf(stderr,
			"Error: Bus doesn't support detection commands\n");
		close(file);
		exit(1);
	}
	if (!(funcs & I2C_FUNC_SMBUS_QUICK)) {
		fprintf(stderr, "Error: Can't use SMBus Quick Write command "
			"on this bus\n");
		close(file);
		exit(1);
	}
	if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE)) {
		fprintf(stderr, "Error: Can't use SMBus Receive Byte command "
			"on this bus\n");
		close(file);
		exit(1);
	}
	if (!(funcs & I2C_FUNC_SMBUS_QUICK))
		fprintf(stderr, "Warning: Can't use SMBus Quick Write "
			"command, will skip some addresses\n");
	if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE))
		fprintf(stderr, "Warning: Can't use SMBus Receive Byte "
			"command, will skip some addresses\n");
	//printf("%s file %x\n",__func__, file);
}

void wnc_i2c_dev_close(void){
	if (file){
		close(file);
		//printf("%s file %x\n",__func__, file);
		file=0;
	}
}

void wnc_i2c_dev_scan(void){
	int first = 0x3, last = 0x7F;
	printf("scanning bus:%s\n", filename);
	if ((file==0) || scan_i2c_bus(MODE_AUTO, first, last))
		printf("error!!\n");
}

int wnc_i2c_dev_write_byte_data(int address, unsigned char daddress, unsigned char val) {
	int ret;
	if ((file==0) || set_slave_addr(file, address, force))
		exit(1);
	if (daddress)
		ret = i2c_smbus_write_byte_data(file, daddress, val);
	else
		ret = i2c_smbus_write_byte(file, val);		
	
	if (ret)
		printf("%xh write_byte_data %x %s\n",address, val, ret==0?"ok":"ng");
	return ret;
}

char* wnc_i2c_dev_read_bytes_inc(unsigned char address, unsigned char daddress, int bytes, int show_hex) {
	int i;
	memset(i2c_read_buff, 0, sizeof(i2c_read_buff));
	memset(i2c_read_buff_str, 0, sizeof(i2c_read_buff_str));
	if ((file==0) || set_slave_addr(file, address, force))
		exit(1);
	for (i=0; i<bytes; i++) {
		//if (daddress+i)
			res = i2c_smbus_write_byte(file, daddress+i);
		res = i2c_smbus_read_byte(file);
		i2c_read_buff[i] = res;
		if (show_hex)
			sprintf(i2c_read_buff_str, "%s%02X ", i2c_read_buff_str, res);
		else
			sprintf(i2c_read_buff_str, "%s%d ", i2c_read_buff_str, res);
	}
	return i2c_read_buff_str;
}

int wnc_i2c_dev_read_byte_data(unsigned char address, unsigned char daddress) {

	int ret = -1;
	
	
	if ((file==0) || set_slave_addr(file, address, force))
		exit(1);
	/*if (daddress)*/ //Remove this statement to support 0x00 address
		ret = i2c_smbus_write_byte(file, daddress);
	
	if (ret < 0)
		fprintf(stderr, "Warning - write failed\n");
	ret = i2c_smbus_read_byte(file);
		
	return ret;
}


char* wnc_i2c_dev_read_bytes(unsigned char address, unsigned char daddress, int bytes, int show_hex) {
	int i;
	memset(i2c_read_buff, 0, sizeof(i2c_read_buff));
	memset(i2c_read_buff_str, 0, sizeof(i2c_read_buff_str));
	if ((file==0) || set_slave_addr(file, address, force))
		exit(1);
	if (daddress)
		res = i2c_smbus_write_byte(file, daddress);
	for (i=0; i<bytes; i++) {
		if (res < 0)
			fprintf(stderr, "Warning - write failed\n");
		res = i2c_smbus_read_byte(file);
		i2c_read_buff[i] = res;
		if (show_hex)
			sprintf(i2c_read_buff_str, "%s%02X", i2c_read_buff_str, res);
		else
			sprintf(i2c_read_buff_str, "%s%d", i2c_read_buff_str, res);
	}
	return i2c_read_buff_str;
}


char* wnc_i2c_dev_read_char(unsigned char address, unsigned char daddress, int bytes) {
	int i;
	int ret;
	memset(i2c_read_buff, 0, sizeof(i2c_read_buff));
	memset(i2c_read_buff_str, 0, sizeof(i2c_read_buff_str));
	if ((file==0) || set_slave_addr(file, address, force))
		exit(1);
	for (i=0; i<bytes; i++) {
		//if (daddress+i)
		ret = i2c_smbus_read_byte_data(file, daddress+i); /*Use repeat start between write command and read data*/
		//ret = i2c_smbus_write_byte(file, daddress+i);
		//ret = i2c_smbus_read_byte(file);
		i2c_read_buff[i] = ret;

		if (ret < 0)
		{	
			sprintf(i2c_read_buff_str, "%s%c", i2c_read_buff_str,88 & 0xff); //88=X 
		}
		else if ((ret & 0xff) == 0x00 || (ret & 0xff) == 0xff)
		{	
			//sprintf(i2c_read_buff_str, "%s%c", i2c_read_buff_str,46 & 0xff); //46=. 
		}
		else if ((ret & 0xff) < 32 || (ret & 0xff) >= 127) 
		{	
			//sprintf(i2c_read_buff_str, "%s%c", i2c_read_buff_str,63 & 0xff); //63=? 
		}
		else
			sprintf(i2c_read_buff_str, "%s%c", i2c_read_buff_str, ret & 0xff);
			
	}
	return i2c_read_buff_str;
}



unsigned int log2_(int n) {
     int logValue = -1;
     while (n) {
         logValue++;
         n >>= 1;
     }
     return logValue;
}

void intHandler(int dummy) {
    (void)dummy;
    printf("Ctrl+C...\n");
    wnc_i2c_dev_close();
}

int scan_i2c_bus(int mode, int first, int last){
	int i, j;
	int cmd;
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	for (i = 0; i < 128; i += 16) {
		printf("%02x: ", i);
		for(j = 0; j < 16; j++) {
			fflush(stdout);
			switch (mode) { /* Select detection command for this address */
			default:
				cmd = mode;
				break;
			case MODE_AUTO:
				if ((i+j >= 0x30 && i+j <= 0x37) || (i+j >= 0x50 && i+j <= 0x5F))
				 	cmd = MODE_READ;
				else
					cmd = MODE_QUICK;
				break;
			}
			//if (i+j==BC3_MUX0_ADDR ||i+j==BC3_MUX1_ADDR ||i+j==BC3_MUX2_ADDR ||i+j==BC3_MUX3_ADDR ||i+j==BC3_MUX4_ADDR ||i+j==BC3_MUX5_ADDR ||i+j==BC3_MUX6_ADDR ||i+j==BC3_MUX7_ADDR )
			/* Skip unwanted addresses */
			if (i+j < first || i+j > last
			 || (cmd == MODE_READ && !(funcs & I2C_FUNC_SMBUS_READ_BYTE))
			 || (cmd == MODE_QUICK && !(funcs & I2C_FUNC_SMBUS_QUICK))) {
				printf("   ");
				continue;
			}
			/* Set slave address */
			if (ioctl(file, I2C_SLAVE, i+j) < 0) {
				if (errno == EBUSY) {
					printf("UU ");
					continue;
				} else {
					fprintf(stderr, "Error: Could not set address to 0x%02x: %s\n", i+j, strerror(errno));
					return -1;
				}
			}
			/* Probe this address */
			switch (cmd) {
			default: /* MODE_QUICK */
				res = i2c_smbus_write_quick(file, I2C_SMBUS_WRITE); /* corrupt Atmel AT24RF08 EEPROM */
				break;
			case MODE_READ:
				res = i2c_smbus_read_byte(file); /* lock SMBus on various write-only chips (mainly clock chips) */
				break;
			}
			if (res < 0)
				printf("-- ");
			else
				printf("%02x ", i+j);
		}
		printf("\n");
	}
	return 0;
}

int check_i2c_address(int mode, int addr){

	int cmd;
	int ret=0;

		if( addr < 0x3 || addr > 0x7f)
		{
			fprintf(stderr, "Error: address %02x out of range (0x03-0x77)!\n",addr);
			return -1;
		}

		switch (mode) { /* Select detection command for this address */
		default:
			cmd = mode;
			break;
		case MODE_AUTO:
			if ((addr >= 0x30 && addr <= 0x37) || (addr >= 0x50 && addr <= 0x5F))
			 	cmd = MODE_READ;
			else
				cmd = MODE_QUICK;
			break;
		}

		/* Set slave address */
		if (ioctl(file, I2C_SLAVE, addr) < 0) {
			if (errno == EBUSY) {
				fprintf(stderr, "Error: Could not set address to 0x%02x: Busy\n", addr);
				return -1;
			} else {
				fprintf(stderr, "Error: Could not set address to 0x%02x: %s\n", addr, strerror(errno));
				return -1;
			}
		}
		
		/* Probe this address */
		switch (cmd) {
		default: /* MODE_QUICK */
			res = i2c_smbus_write_quick(file, I2C_SMBUS_WRITE); /* corrupt Atmel AT24RF08 EEPROM */
			break;
		case MODE_READ:
			res = i2c_smbus_read_byte(file); /* lock SMBus on various write-only chips (mainly clock chips) */
			break;
		}
		
		if (res < 0)
		{
			//printf("0x%02x is NOT present.\n", addr);
			ret = 0;	
		}	
		else
		{
			//printf("0x%02x is present.\n", addr);
			ret = 1;
		}	

    fflush(stdout);
	
	return ret;
}



/* Return 1 if we should continue, 0 if we should abort */
int user_ack(int def)
{
	char s[2];
	int ret;

	if (!fgets(s, 2, stdin))
		return 0; /* Nack by default */

	switch (s[0]) {
	case 'y':
	case 'Y':
		ret = 1;
		break;
	case 'n':
	case 'N':
		ret = 0;
		break;
	default:
		ret = def;
	}

	/* Flush extra characters */
	while (s[0] != '\n') {
		int c = fgetc(stdin);
		if (c == EOF) {
			ret = 0;
			break;
		}
		s[0] = c;
	}

	return ret;
}

