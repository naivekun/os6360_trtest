/*
 *  aikido_tool.c
 *
 *  Copyright (C) 2000 Steven J. Hill (sjhill@realitydiluted.com)
 *		  2003 Thomas Gleixner (tglx@linutronix.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Bug/ToDo:
 */

#define PROGRAM_NAME "aikido_tool"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>

#include <asm/types.h>
#include "mtd/mtd-user.h"
#include "common.h"
#include <libmtd.h>

#define AIKIDO_TOOL_VERSION "v0.0.2"

/* Return the mask including "numOfBits" bits. for 0..31 bits   */
#define BIT_MASK_0_31_MAC(numOfBits) (~(0xFFFFFFFF << (numOfBits)))

/* the macro of BIT_MASK_0_31_MAC() in VC will act in wrong way for 32 bits , and will
   result 0 instead of 0xffffffff

   so macro BIT_MASK_MAC - is improvement of BIT_MASK_0_31_MAC to support 32 bits
*/
#define BIT_MASK_MAC(numOfBits)    ((numOfBits) == 32 ? 0xFFFFFFFF : BIT_MASK_0_31_MAC(numOfBits))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK_MAC(8,2) = 0xFFFFFCFF                     */
#define FIELD_MASK_NOT_MAC(offset,len)                      \
        (~(BIT_MASK_MAC((len)) << (offset)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK_MAC(8,2) = 0x00000300                     */
#define FIELD_MASK_MAC(offset,len)                      \
        ( (BIT_MASK_MAC((len)) << (offset)) )

/* Returns the info located at the specified offset & length in data.   */
#define U32_GET_FIELD_MAC(data,offset,length)           \
        (((data) >> (offset)) & BIT_MASK_MAC(length))

/* Sets the field located at the specified offset & length in data.     */
#define U32_SET_FIELD_MAC(data,offset,length,val)           \
   (data) = (((data) & FIELD_MASK_NOT_MAC((offset),(length))) | ((val) <<(offset)))

/* for setting data of GT_U16.
   Sets the field located at the specified offset & length in data.     */
#define U16_SET_FIELD_MAC(data,offset,length,val)           \
   (data) = (GT_U16)(((data) & FIELD_MASK_NOT_MAC((offset),(length))) | ((val) <<(offset)))

/* Sets the field located at the specified offset & length in data.
    the field may be with value with more bits then length */
#define U32_SET_FIELD_MASKED_MAC(data,offset,length,val)           \
   (U32_SET_FIELD_MAC((data),(offset),(length),((val) & BIT_MASK_MAC(length))))


static void display_help(int status)
{
	fprintf(status == EXIT_SUCCESS ? stdout : stderr,
"Usage: aikido_tool [OPTION]\n"
"Read/Write aikido logical part.\n"
"  -r addr,  --read=addr    Read the data of this address.\n"
"  -w addr,  --write=addr   Write the data into this address.\n"
"  -b bitnum,--bit=bitnum   Specific bit number[0-7].\n"
"  -d data,  --data=data    Input data.\n"
"  -h,       --help         Display this help and exit.\n"
"  -v,       --version      Display version of aikido_tool.\n"
	);
	exit(status);
}

static void display_version(void)
{
	printf("%1$s " AIKIDO_TOOL_VERSION "\n",PROGRAM_NAME);
	exit(EXIT_SUCCESS);
}


/*static const char	*mtd_device;*/
static long  address = 0;
static long  in_data = -1;
static int	 bitnum = -1; 

static long long	inputsize = 0;
static bool		read_aikido = false;
static bool     write_aikido = false;
static bool     bit_aikido = false;
static bool     data_aikido = false;


/*#define AIKIDO_MTD "/dev/mtd3"*/

static void process_options(int argc, char * const argv[])
{
	int error = 0;
	int para_count = 0;

	for (;;) {
		int option_index = 0;
		static const char short_options[] = "hvr:w:b:d:";
		static const struct option long_options[] = {
			/* Order of these args with val==0 matters; see option_index. */
			{"version", no_argument, 0, "v"},
			{"help", no_argument, 0, 'h'},
			{"read", required_argument, 0, 'r'},
			{"write", required_argument, 0, 'w'},
			{"bit", required_argument, 0, 'b'},
			{"data", required_argument, 0, 'd'},			
			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, short_options,
				long_options, &option_index);
		if (c == EOF)
			break;
		else
			para_count++;
		
		switch (c) {
		case 'v':
			display_version();
			break;	
		case 'r':
			read_aikido = true;
			if(write_aikido == true) /*Can't input read and write at the same time*/
				display_help(EXIT_FAILURE); 

			address = simple_strtoll(optarg, &error);
			/*printf("address=%x",address);*/
			break;		
		case 'w':
			write_aikido = true;
			if(read_aikido == true) /*Can't input read and write at the same time*/
				display_help(EXIT_FAILURE);
			
			address = simple_strtoll(optarg, &error);
			/*printf("address=%x",address);*/
			break;		
		case 'b':
			bit_aikido=true;
			bitnum = atoi(optarg);
			/*printf("bitnum=%d",bitnum);*/
			break;
		case 'd':
			data_aikido = true;
			in_data = simple_strtoll(optarg, &error);
			/*printf("data=%d",in_data);*/
			break;
		case 'h':
			display_help(EXIT_SUCCESS);
			break;
		case '?':
			error++;
			break;
		}
	}

	if(para_count == 0)
		display_help(EXIT_FAILURE); 

	if (address < 0 || address > 255)
		errmsg_die("Can't specify device address out of 0x00 - 0xFF with option"
				" -%s: 0x%02x",read_aikido ? "r" : "w", address);

    if ( write_aikido == true && data_aikido == false)
	{
		if( bitnum < 0 || bitnum > 7)
			errmsg_die("No input data with opton -w (write)!");
    }

    if ( bit_aikido == true)
	{
		if( bitnum < 0 || bitnum > 7)
			errmsg_die("Can't specify bit number out of range 0-7 with option -b:"
				" %d", bitnum);
    }
	
	if(bit_aikido == true && write_aikido == true)
	{
		if ( in_data < 0 || in_data > 1 )
			errmsg_die("Can't specify data out of range 0-1 with option -b:"
								" %d", in_data);
	}

	if(bit_aikido == false && write_aikido == true)
	{
		if ( in_data < 0 || in_data > 255 )
			errmsg_die("Can't specify data out of range 0x00 - 0xFF with option -w:"
								" 0x%02x", in_data);
	}
						
}

int main(int argc, char * const argv[])
{
    mtd_info_t mtd_info;           // the MTD structure
    int i;
	char mtd_dev[12];
    int mtd_num;
	int fd = -1;

    unsigned char data_buf[4] = {0x00,0x00,0x00,0x00};
    unsigned char read_buf[4] = {0x00};               

	process_options(argc, argv);

	/*mtd_num=system("/usr/sbin/get_aikido_mtd");*/
	FILE *fp = popen("/usr/sbin/get_aikido_mtd", "r");

	fscanf(fp, "%d", &mtd_num);
	pclose(fp);
	
	sprintf(mtd_dev,"/dev/mtd%d",mtd_num);

	/*printf("mtd device = %s\n",mtd_dev);*/

	/*if ((fd = open(AIKIDO_MTD, O_RDWR)) == -1)
		sys_errmsg_die("%s", AIKIDO_MTD);*/

	if ((fd = open(mtd_dev, O_RDWR)) == -1)
		sys_errmsg_die("%s", mtd_dev);

#if 0	
    ioctl(fd, MEMGETINFO, &mtd_info);   // get the device info

    // dump it for a sanity check, should match what's in /proc/mtd
    printf("MTD Type: %x\nMTD total size: %x bytes\nMTD erase size: %x bytes\n",
         mtd_info.type, mtd_info.size, mtd_info.erasesize);
#endif

    if ( read_aikido == true )
    {
    		lseek(fd, address, SEEK_SET);          // go to the address
    		read(fd, read_buf, sizeof(read_buf)); // read 1 byte

			if ( bit_aikido == false )
				printf("Read Aikido 0x%02x = 0x%02x\n", address, (unsigned int)read_buf[0]);		
			else
				printf("Read Aikido 0x%02x bit %d = %d\n", address, bitnum, U32_GET_FIELD_MAC(read_buf[0],bitnum,1));

	}		
    else
    {
			
			if ( bit_aikido == false )
			{		
					
				data_buf[0] = in_data;
				lseek(fd, address, SEEK_SET);
				printf("Write Aikido 0x%02x = 0x%02x\n",address, (unsigned int)data_buf[0]);

				/*Debug messages
				     for(i=0;i<sizeof(data_buf);i++)
					printf("Write Aikido 0x%02x i %d = 0x%02x\n",address,i,data_buf[i]);	*/
				
	    		write(fd, data_buf, sizeof(data_buf)); // write our message
			}
			else
			{
		        
				lseek(fd, address, SEEK_SET);              
    			read(fd, read_buf, sizeof(read_buf)); //read original data

				/*printf ("Read original data 0x%02x\n", read_buf[0]);*/

				U32_SET_FIELD_MAC(read_buf[0], bitnum, 1, in_data); /*Change the bit number only*/

				data_buf[0] = read_buf[0];
				lseek(fd, address, SEEK_SET);
				printf("Write Aikido 0x%02x bit %d = %d\n",address,bitnum,in_data);

	    		write(fd, data_buf, sizeof(data_buf)); // write our message
                
			}
		    /*	
			lseek(fd, address, SEEK_SET);			   
			read(fd, read_buf, sizeof(read_buf)); //read original data
			// sanity check, now you see the message we wrote!    
         	     printf("buf[%d] = 0x%02x\n", 0, (unsigned int)read_buf[0]);				
			*/
    }
	

closeall:
	close(fd);
	
	return 0;
}

