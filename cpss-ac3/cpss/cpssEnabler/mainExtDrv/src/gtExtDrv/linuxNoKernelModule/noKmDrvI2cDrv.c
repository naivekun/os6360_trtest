/*******************************************************************************
*                Copyright 2018, MARVELL SEMICONDUCTOR, LTD.                   *
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
*/
/**
********************************************************************************
* @file noKmDrvI2cDrv.c
*
* @brief I2C driver
*
* @version   1
********************************************************************************
*/

#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
#include    <linux/i2c.h>
#include    <linux/i2c-dev.h>
#include    <fcntl.h>
#include    <stropts.h>
#include    <string.h>
#include    <stdio.h>
#include    <unistd.h>
#include    <sys/mman.h>

#include <gtExtDrv/drivers/gtI2cDrv.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#define I2C_PREFIX_DEV_NAME     "/dev/i2c"
#define MAX_LEN_I2C_MSG         128

typedef struct CPSS_HW_DRIVER_I2C_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;

} CPSS_HW_DRIVER_I2C_STC;

/**
* @internal intDrvI2cReceiveWithOffset function
* @endinternal
*
* @brief   This routine receive I2C data with optional offset.
*
* @param[in] slave_address - the target device slave address.
* @param[in] buffer_size   - buffer length.
* @param[in] bus_id        - the I2C bus id.
* @param[in] offset_type   - offset mode.
* @param[in] offset        - internal offset in slave to read
*       from.
*
* @param[out] buffer     - received buffer with the read data.
*
* @retval GT_OK                - on success,
* @retval GT_NO_RESOURCE       - fail to open bus (file) driver,
* @retval GT_FAIL              - othersise.
*/
static GT_STATUS intDrvI2cReceiveWithOffset(
	GT_U8     slave_address,
	GT_U32    buffer_size,
	GT_U8     bus_id,
	EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
	GT_U32    offset,
	GT_U8    *buffer
)
{
    int fd;
    int msg_index = 0, offset_len = 1;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg[2];
    char tmp[2];
    char *pTmp = tmp;
    char i2c_dev_name[20];
    GT_STATUS rc = GT_OK;

	/* If offset type is 8 or 16 bits,
	 * two messages are required within this transaction.
	 * First determine offset address
	 * Second - receive data.
	 */
	i2c_data.msgs = msg;

	i2c_data.msgs[msg_index].addr = slave_address;
	i2c_data.nmsgs = 1;                   /* single i2c_msg */

	switch(offset_type) {
	case EXT_DRV_I2C_OFFSET_TYPE_NONE_E:
		break;

	case EXT_DRV_I2C_OFFSET_TYPE_16_E:
		/* "Write" part of the transaction */
		*pTmp = (char)((offset >> 8) & 0xFF);
		pTmp++;
		offset_len = 2;
	case EXT_DRV_I2C_OFFSET_TYPE_8_E:
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

    /*
	 * ioctl() processes read.
	 * Operation is determined by flags field of i2c_msg
	 */
    sprintf(i2c_dev_name, "%s-%d", I2C_PREFIX_DEV_NAME, bus_id);
    fd = open(i2c_dev_name, O_RDWR);
    if (fd < 0)
        return GT_NO_RESOURCE;

	if( ioctl(fd, I2C_RDWR, &i2c_data) < 0 )
        rc = GT_FAIL;

    if ( close(fd) < 0 )
        rc = GT_FAIL;

    return rc;
}

/**
* @internal intDrvI2cTransmitWithOffset function
* @endinternal
*
* @brief   This routine transmit I2C data with optional offset.
*
* @param[in] slave_address - the target device slave address.
* @param[in] buffer_size   - buffer length.
* @param[in] bus_id        - the I2C bus id.
* @param[in] offset_type   - offset mode.
* @param[in] offset        - internal offset in slave to write
*       to.
* @param[in] buffer        - buffer with the data to write.
*
* @retval GT_OK                - on success,
* @retval GT_NO_RESOURCE       - fail to open bus (file) driver,
* @retval GT_FAIL              - othersise.
*/
static GT_STATUS intDrvI2cTransmitWithOffset(
	GT_U8     slave_address,
	GT_U32    buffer_size,
	GT_U8     bus_id,
	EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
	GT_U32    offset,
	GT_U8    *buffer
)
{
    int fd;
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg;
	GT_U8 buf[MAX_LEN_I2C_MSG];
	GT_U8 *pBuf = buf;
    char i2c_dev_name[20];
    GT_STATUS rc = GT_OK;

	if (buffer_size > MAX_LEN_I2C_MSG -3 )
		return -1;

	switch (offset_type) {
	case EXT_DRV_I2C_OFFSET_TYPE_NONE_E:
		break;

	case EXT_DRV_I2C_OFFSET_TYPE_16_E:
		*pBuf = (GT_U8)((offset >> 8) & 0xFF);
		pBuf++;
		/* fallthrough */
	case EXT_DRV_I2C_OFFSET_TYPE_8_E:
		*pBuf = (GT_U8)(offset & 0xFF);
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
	 * ioctl() processes write.
	 * Operation is determined by flags field of i2c_msg
	 */
    sprintf(i2c_dev_name, "%s-%d", I2C_PREFIX_DEV_NAME, bus_id);
    fd = open(i2c_dev_name, O_RDWR);
    if (fd < 0)
        return GT_NO_RESOURCE;

	if( ioctl(fd, I2C_RDWR, &i2c_data) < 0 )
        rc = GT_FAIL;

    if ( close(fd) < 0 )
        rc = GT_FAIL;

    return rc;
}

/**
* @internal intDrvI2cInit function
* @endinternal
*
* @brief   initialize the i2c driver
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note Linux only
*
*/
static GT_STATUS intDrvI2cInit(
    GT_VOID
)
{
    return GT_OK;
}

/* Driver AS (Address Space) fields: */
/* bits[7:0]   - Slave Address       */
/* bits[15:8]  - Bus ID              */
/* bits[31:16] - Offset Type         */
static GT_STATUS intDrvI2cRead(
    IN  CPSS_HW_DRIVER_I2C_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U8     slave_address;   /* the target device slave address on I2C bus */
    GT_U32    buffer_size;     /* buffer length */
    GT_U8     bus_id;          /* the I2C bus id if only one bus then bus_id=0*/
    EXT_DRV_I2C_OFFSET_TYPE_ENT  offset_type;            /* receive mode */
    GT_U32    offset;          /* internal offset to write to */
    GT_U8    *buffer;          /* transmited buffer */
    GT_STATUS  rc = GT_OK;

    buffer_size = count;
    offset = regAddr;
    buffer = (GT_U8 *)dataPtr;
    slave_address = (addrSpace & 0xFF);
    bus_id = ((addrSpace & 0xFF00) >> 8);
    offset_type = ((addrSpace & 0xFFFF0000) >> 16);

    cpssOsMutexLock(drv->mtx);

    rc = intDrvI2cReceiveWithOffset(slave_address,
                                    buffer_size,
                                    bus_id,
                                    offset_type,
                                    offset,
                                    buffer);

    cpssOsMutexUnlock(drv->mtx);

    return rc;
}

/* Driver AS (Address Space) fields: */
/* bits[7:0]   - Slave Address       */
/* bits[15:8]  - Bus ID              */
/* bits[31:16] - Offset Type         */
static GT_STATUS intDrvI2cWriteMask(
    IN  CPSS_HW_DRIVER_I2C_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask GT_UNUSED
)
{
    GT_U8     slave_address;   /* the target device slave address on I2C bus */
    GT_U32    buffer_size;     /* buffer length */
    GT_U8     bus_id;          /* the I2C bus id if only one bus then bus_id=0*/
    EXT_DRV_I2C_OFFSET_TYPE_ENT  offset_type;            /* receive mode */
    GT_U32    offset;          /* internal offset to write to */
    GT_U8    *buffer;          /* transmited buffer */
    GT_STATUS  rc = GT_OK;

#ifdef _VISUALC
    (void)mask;
#endif

    buffer_size = count;
    offset = regAddr;
    buffer = (GT_U8 *)dataPtr;
    slave_address = (addrSpace & 0xFF);
    bus_id = ((addrSpace & 0xFF00) >> 8);
    offset_type = ((addrSpace & 0xFFFF0000) >> 16);

    cpssOsMutexLock(drv->mtx);

    rc = intDrvI2cTransmitWithOffset(slave_address,
                                     buffer_size,
                                     bus_id,
                                     offset_type,
                                     offset,
                                     buffer);

    cpssOsMutexUnlock(drv->mtx);

    return rc;
}

static GT_STATUS intDrvI2cDestroy(
    IN  CPSS_HW_DRIVER_I2C_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal extDrvI2cRead function
* @endinternal
*
* @brief   This routine receive I2C data with optional offset.
*
* @param[in] drv           - driver pointer.
* @param[in] bus_id        - the I2C bus id.
* @param[in] slave_address - the target device slave address.
* @param[in] offset_type   - offset mode.
* @param[in] offset        - internal offset in slave to read
*       from.
* @param[in] buffer_size   - buffer length.
*
* @param[out] buffer     - received buffer with the read data.
*
* @retval GT_OK                - on success,
* @retval GT_NOT_INITIALIZED   - driver not initialized,
* @retval GT_NO_RESOURCE       - fail to open bus (file) driver,
* @retval GT_FAIL              - othersise.
*/
GT_STATUS extDrvI2cRead(
    IN  CPSS_HW_DRIVER_STC *drv,
    IN  GT_U8     bus_id,
    IN  GT_U8     slave_address,
    IN  EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN  GT_U32    offset,
    IN  GT_U32    buffer_size,
    OUT GT_U8    *buffer
)
{
    GT_U32  addrSpace;

    if ( NULL == drv) {
        return GT_NOT_INITIALIZED;
    }

    addrSpace = slave_address + (bus_id << 8) + (offset_type << 16);

    return(drv->read(drv, addrSpace, offset, (GT_U32 *)buffer, buffer_size));
}

/**
* @internal extDrvI2cWrite function
* @endinternal
*
* @brief   This routine transmit I2C data with optional offset.
*
* @param[in] drv           - driver pointer.
* @param[in] bus_id        - the I2C bus id.
* @param[in] slave_address - the target device slave address.
* @param[in] offset_type   - offset mode.
* @param[in] offset        - internal offset in slave to write
*       to.
* @param[in] buffer_size   - buffer length.
* @param[in] buffer        - buffer with the data to write.
*
* @retval GT_OK                - on success,
* @retval GT_NOT_INITIALIZED   - driver not initialized,
* @retval GT_NO_RESOURCE       - fail to open bus (file) driver,
* @retval GT_FAIL              - othersise.
*/
GT_STATUS extDrvI2cWrite(
    IN CPSS_HW_DRIVER_STC *drv,
    IN GT_U8     bus_id,
    IN GT_U8     slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32    offset,
    IN GT_U32    buffer_size,
    IN GT_U8    *buffer
)
{
    GT_U32  addrSpace;

    if ( NULL == drv) {
        return GT_NOT_INITIALIZED;
    }

    addrSpace = slave_address + (bus_id << 8) + (offset_type << 16);

    return(drv->writeMask(drv, addrSpace, offset, (GT_U32 *)buffer, buffer_size, 0));
}

/**
* @internal extDrvI2cCreateDrv function
* @endinternal
*
* @param[in] parent  - parent driver
*
* @brief Create I2C driver
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *extDrvI2cCreateDrv(
    IN CPSS_HW_DRIVER_STC *parent
)
{
    CPSS_HW_DRIVER_I2C_STC *drv;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)intDrvI2cRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)intDrvI2cWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)intDrvI2cDestroy;

    if( GT_OK != intDrvI2cInit())
    {
        return NULL;
    }

    cpssOsMutexCreate("extDrvI2c", &(drv->mtx));

    return (CPSS_HW_DRIVER_STC*)drv;
}

