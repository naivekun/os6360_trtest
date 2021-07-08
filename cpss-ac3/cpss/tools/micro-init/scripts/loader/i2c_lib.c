/*******************************************************************************
*                Copyright 2016, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
********************************************************************************
* i2c_lib.c
*******************************************************************************/

#include    <linux/i2c.h>
#include    <linux/i2c-dev.h>
#include    <fcntl.h>
#include    <stropts.h>
#include    <string.h>
#include    <stdio.h>
#include    <sys/mman.h>


#include    "i2c_lib.h"

#define MAX_NUM_I2C_BUSSES      1
#define BUS_ID_0                0
#define I2C_PREFIX_DEV_NAME     "/dev/i2c"
#define MAX_LEN_I2C_MSG         128

static int i2c_bus_fds[MAX_NUM_I2C_BUSSES];

extern int i2c_receive(
	uint8_t    slave_address,   /* the target device slave address on I2C bus */
	uint32_t   buffer_size,     /* buffer length */
	uint8_t    bus_id,          /* the I2C bus id */
	uint8_t   *buffer           /* received buffer */
)
{
	int ret;
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg[1];

	/*
	 * Only a single transaction - receive
	 */
	i2c_data.msgs = msg;
	i2c_data.nmsgs = 1;    /* two i2c_msg */

	i2c_data.msgs[0].addr = slave_address;
	i2c_data.msgs[0].flags = I2C_M_RD;      /* read command */
	i2c_data.msgs[0].len = buffer_size;
	i2c_data.msgs[0].buf = buffer;

	ret = ioctl(i2c_bus_fds[bus_id], I2C_RDWR, &i2c_data);

	return ret;
}


extern int i2c_receive_with_offset (
	uint8_t     slave_address,   /* the target device slave address on I2C bus */
	uint32_t    buffer_size,     /* buffer length */
	uint8_t     bus_id,          /* the I2C bus id */
	enum        i2c_offset_type_ENT   offset_type,            /* receive mode */
	uint32_t    offset,          /* internal offset to read from */
	uint8_t    *buffer           /* received buffer */
)
{
    int ret, msg_index = 0, offset_len = 1;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg[2];
    char tmp[2];
    char *pTmp = tmp;

	/* If offset type is 8 or 16 bits,
	 * two messages are required within this transaction.
	 * First determine offset address
	 * Second - receive data.
	 */
	i2c_data.msgs = msg;

	i2c_data.msgs[msg_index].addr = slave_address;
	i2c_data.nmsgs = 1;                   /* single i2c_msg */

	switch(offset_type) {
	case i2c_offset_type_none_E:
		break;

	case i2c_offset_type_16_E:
		/* "Write" part of the transaction */
		*pTmp = (char)((offset >> 8) & 0xFF);
		pTmp++;
		offset_len = 2;
	case i2c_offset_type_8_E:
		i2c_data.nmsgs = 2;                   /* two i2c_msg */
		i2c_data.msgs[msg_index].flags = 0;   /* No flags for write command */
		i2c_data.msgs[msg_index].len = offset_len;
		i2c_data.msgs[msg_index].buf =  (__u8 *)tmp;
		*pTmp = (char)(offset & 0xFF);
		msg_index++;
		break;
	}
	/*
	 * "Read" part of the transaction
	 */
	i2c_data.msgs[msg_index].addr = slave_address;
	i2c_data.msgs[msg_index].flags = I2C_M_RD;  /* read command */
	i2c_data.msgs[msg_index].len = buffer_size;
	i2c_data.msgs[msg_index].buf = (__u8 *)buffer;

	ret = ioctl(i2c_bus_fds[bus_id], I2C_RDWR, &i2c_data);
	return ret;
}

extern int i2c_transmit_with_offset(
	uint8_t     slave_address,   /* the target device slave address on I2C bus */
	uint32_t    buffer_size,     /* buffer length */
	uint8_t     bus_id,          /* the I2C bus id if only one bus then bus_id=0*/
	enum        i2c_offset_type_ENT  offset_type,            /* receive mode */
	uint32_t    offset,          /* internal offset to write to */
	uint8_t    *buffer           /* transmited buffer */
	)
{
	int ret;
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg;
	uchar buf[MAX_LEN_I2C_MSG];
	uchar *pBuf = buf;

	if (buffer_size > MAX_LEN_I2C_MSG -3 )
		return -1;

	switch (offset_type) {
	case i2c_offset_type_none_E:
		break;

	case i2c_offset_type_16_E:
		*pBuf = (uchar)((offset >> 8) & 0xFF);
		pBuf++;
		/* fallthrough */
	case i2c_offset_type_8_E:
		*pBuf = (uchar)(offset & 0xFF);
		pBuf++;
	}

	memcpy(pBuf, buffer, buffer_size);
	buffer_size += (pBuf - buf);
	/*
	 * write operation
	 */
	i2c_data.msgs = &msg;
	i2c_data.nmsgs = 1;   /* use one message structure */

	i2c_data.msgs[0].addr = slave_address;
	i2c_data.msgs[0].flags = 0;   /* don't need flags */
	i2c_data.msgs[0].len = buffer_size;
	i2c_data.msgs[0].buf = (__u8 *)buf;

	/*
	 * ioctl() processes read & write.
	 * Operation is determined by flags field of i2c_msg
	 */
    ret = ioctl(i2c_bus_fds[bus_id], I2C_RDWR, &i2c_data);
    return ret;
}
/*$ END OF FuncName */



extern int i2c_init(void)
{
	int status = 0, i, fd, *map;
	char i2c_dev_name[20];

	if ((fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
		return -1;
	map = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xf1000000);

	if (map == MAP_FAILED) {
		 return -1;
	}
	*(volatile unsigned int *)(map + 0x18140) = 0x1000;
	munmap(map, 0x100000);
	close(fd);
 

	for (i = 0; i < MAX_NUM_I2C_BUSSES; i++) {
		sprintf(i2c_dev_name, "%s-%d", I2C_PREFIX_DEV_NAME, i);
		i2c_bus_fds[i] = open(i2c_dev_name, O_RDWR);
		if (i2c_bus_fds[i] < 0) {
			status = -1;
			break;
		}
	}

	return status;
}

