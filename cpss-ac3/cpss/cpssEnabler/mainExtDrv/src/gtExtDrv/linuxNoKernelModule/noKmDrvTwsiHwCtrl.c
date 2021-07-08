/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file noKmDrvTwsiHwCtrl.c
*
* @brief API implementation for TWSI facilities.
*
*/

#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include "prvNoKmDrv.h"


/* file descriptor returnd by openning the PP *nix device driver */
static GT_32 gtI2cFd = -1;
static GT_U32 i2cCurrentSlaveId;

/* Macroes definitions */
#define TWSI_SLAVE_ADDR(data)     (data)

#define MV_ON_READ_SET_BITS31(data)    (data[0] |= 0x80)
#define MV_ON_WRITE_RESET_BIT31(data)  (data[0] &= 0x7F)
#define MV_RESET_BIT_30(data)          (data[0] &= 0xBF)


#define I2C_SLAVE       0x0703  /* Use this slave address */
#define I2C_SMBUS       0x0720  /* SMBus transfer */
#define I2C_SMBUS_BLOCK_MAX 32  /* As specified in SMBus standard */

#define I2C_SMBUS_READ      1       /* smbus_access read or write markers */
#define I2C_SMBUS_WRITE     0       /* smbus_access read or write markers */

#define I2C_SMBUS_QUICK         0
#define I2C_SMBUS_BYTE          1
#define I2C_SMBUS_BYTE_DATA     2
#define I2C_SMBUS_WORD_DATA     3
#define I2C_SMBUS_PROC_CALL     4
#define I2C_SMBUS_BLOCK_DATA        5
#define I2C_SMBUS_I2C_BLOCK_BROKEN  6
#define I2C_SMBUS_BLOCK_PROC_CALL   7       /* SMBus 2.0 */
#define I2C_SMBUS_I2C_BLOCK_DATA    8

#define I2C_RDWR        0x0707  /* Combined R/W transfer (one STOP only) */

/* This is the structure as used in the I2C_SMBUS ioctl call */
struct i2c_smbus_ioctl_data {
    unsigned char read_write;
    unsigned char command;
    unsigned long size;
    union i2c_smbus_data *data;
};


union i2c_smbus_data {
    unsigned char byte;
    unsigned short word;
    unsigned char block[I2C_SMBUS_BLOCK_MAX + 2]; /* block[0] is used for length */
                                                /* and one more for PEC */
};

struct i2c_msg {
    GT_U16 addr;    /* slave address            */
    GT_U16 flags;
#define I2C_M_TEN       0x0010  /* this is a ten bit chip address */
#define I2C_M_RD        0x0001  /* read data, from slave to master */
#define I2C_M_NOSTART       0x4000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR  0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK    0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK     0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN      0x0400  /* length will be first received byte */
    GT_U16 len;     /* msg length               */
    GT_U8 *buf;     /* pointer to msg data          */
};

/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
    struct i2c_msg *msgs;   /* pointers to i2c_msgs */
    GT_U32 nmsgs;           /* number of i2c_msgs */
};


GT_STATUS hwIfTwsiSetSlaveId
(
    IN GT_U32 devSlvId
)
{
    if(devSlvId == i2cCurrentSlaveId)
    {
        return GT_OK;
    }

    if (ioctl(gtI2cFd, I2C_SLAVE, devSlvId) < 0)
    {
        perror("Error: Could not set I2C slave address");
        return GT_FAIL;
    }

    i2cCurrentSlaveId = devSlvId;

    usleep(10000); /* 10ms */

    return GT_OK;
}

/**
* @internal hwIfTwsiWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using TWSI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
GT_STATUS hwIfTwsiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)value;
    return GT_NOT_SUPPORTED;
}


/**
* @internal hwIfTwsiReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using TWSI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfTwsiReadReg
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)dataPtr;
    return GT_NOT_SUPPORTED;
}


/**
* @internal hwIfTwsiInitDriver function
* @endinternal
*
* @brief   Init the TWSI interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
GT_STATUS hwIfTwsiInitDriver
(
    GT_VOID
)
{
    int status;

    if (gtI2cFd >= 0)
        return GT_OK;

    status = system("cat /proc/cpuinfo | grep -q 'Armada XP'");
    if (status) {   /* Not Armada XP */
        /* Open i2c-0 file for MSYS handling */
        gtI2cFd = open("/dev/i2c-0", O_RDWR);
    } else {
        /* Open i2c-1 file for ARMADA_XP handling */
        gtI2cFd = open("/dev/i2c-1", O_RDWR);
    }

    if (gtI2cFd < 0)
    {
        perror("failed to open /dev/i2c-*");
        return GT_FAIL;
    }

    i2cCurrentSlaveId = 0xFFFFFFFF;

    return GT_OK;
}


/**
* @internal hwIfTwsiWriteData function
* @endinternal
*
* @brief   Generic TWSI Write operation.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] dataPtr                  - (pointer to) data to be send on the TWSI.
* @param[in] dataPtr                  - should include start offset
* @param[in] dataLen                  - number of bytes to send on the TWSI (from dataPtr).
*                                      range: 1-9
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS hwIfTwsiWriteData
(
    IN GT_U32 devSlvId,
    IN GT_U8  *dataPtr,
    IN GT_U8  dataLen
)
{
    union i2c_smbus_data data;
    struct i2c_smbus_ioctl_data args;
    GT_U32 regAddr;
    GT_U32 i;

    if( dataPtr == NULL )
    {
        return GT_BAD_PTR;
    }

    if( dataLen < 1 || dataLen > 9 )
    {
        return GT_BAD_PARAM;
    }

    if(GT_OK != hwIfTwsiSetSlaveId(devSlvId))
    {
        return GT_FAIL;
    }

    regAddr = dataPtr[0];

    /* Write data */
    args.read_write = I2C_SMBUS_WRITE;
    args.command = regAddr; /* Byte Address */
    args.size = I2C_SMBUS_BLOCK_DATA;

    data.block[0] = dataLen;
    for (i = 1; i <= dataLen; i++)
    {
        data.block[i] = dataPtr[i];
    }

    args.data = &data;

    if (ioctl(gtI2cFd, I2C_SMBUS, &args) < 0)
    {
        perror("Error: Failed to to perform SMBUS IOCTL READ transaction");
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal hwIfTwsiReadData function
* @endinternal
*
* @brief   Generic TWSI Read operation.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] dataLen                  - number of bytes to recieve on the TWSI (into dataPtr).
*                                      range: 1-8.
* @param[in,out] dataPtr                  - [0] should include start offset
* @param[in,out] dataPtr                  - (pointer to) data to be send on the TWSI.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS hwIfTwsiReadData
(               
    IN GT_U32 devSlvId,
    IN GT_U8  dataLen,
    INOUT GT_U8 *dataPtr
)
{
    union i2c_smbus_data data;
    struct i2c_smbus_ioctl_data args;
    GT_U32 regAddr;
    GT_U32 i;

    if( dataPtr == NULL )
    {
        return GT_BAD_PTR;
    }

    if( dataLen < 1 || dataLen > 9 )
    {
        return GT_BAD_PARAM;
    }

    if(GT_OK != hwIfTwsiSetSlaveId(devSlvId))
    {
        return GT_FAIL;
    }

    regAddr = dataPtr[0];

    /* Write command */
    args.read_write = I2C_SMBUS_WRITE;
    args.command = regAddr; /* Byte Address */
    args.size = I2C_SMBUS_BYTE;
    args.data = NULL;
    if (ioctl(gtI2cFd, I2C_SMBUS, &args) < 0)
    {
        perror("Error: Failed to to perform SMBUS IOCTL WRITE transaction");
        return GT_FAIL;
    }

    /* Read data block */
    args.read_write = I2C_SMBUS_READ;
    args.command = regAddr; /* Byte Address */
    args.size = I2C_SMBUS_BLOCK_DATA;

    data.block[0] = dataLen;
    args.data = &data;

    if (ioctl(gtI2cFd, I2C_SMBUS, &args) < 0)
    {
        perror("Error: Failed to to perform SMBUS IOCTL READ transaction");
        return GT_FAIL;
    }

    for (i = 1; i <= data.block[0]; i++)
    {
        dataPtr[i] = data.block[i];
    }

    return GT_OK;
}

/**
* @internal hwIfTwsiWriteByte function
* @endinternal
*
* @brief   Writes the 8 bits of a register using TWSI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
GT_STATUS hwIfTwsiWriteByte
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    union i2c_smbus_data data;
    struct i2c_smbus_ioctl_data args;

    if(GT_OK != hwIfTwsiSetSlaveId(devSlvId))
    {
        return GT_FAIL;
    }

    /* Write data */
    args.read_write = I2C_SMBUS_WRITE;
    args.command = regAddr; /* Byte Address */
    args.size = I2C_SMBUS_BYTE_DATA;

    data.byte = (unsigned char) value;

    args.data = &data;
    if (ioctl(gtI2cFd, I2C_SMBUS, &args) < 0)
    {
        perror("Error: Failed to to perform SMBUS IOCTL READ transaction");
        return GT_FAIL;
    }

    return GT_OK;
}


/**
* @internal hwIfTwsiReadByte function
* @endinternal
*
* @brief   Reads the 8 bits of a register using TWSI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfTwsiReadByte
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    union i2c_smbus_data data;
    struct i2c_smbus_ioctl_data args;

    if(GT_OK != hwIfTwsiSetSlaveId(devSlvId))
    {
        return GT_FAIL;
    }

    /* Write command */
    args.read_write = I2C_SMBUS_WRITE;
    args.command = regAddr; /* Byte Address */
    args.size = I2C_SMBUS_BYTE;
    args.data = NULL;
    if (ioctl(gtI2cFd, I2C_SMBUS, &args) < 0)
    {
        perror("Error: Failed to to perform SMBUS IOCTL WRITE transaction");
        return GT_FAIL;
    }

    /* Read data */
    args.read_write = I2C_SMBUS_READ;
    args.command = regAddr; /* Byte Address */
    args.size = I2C_SMBUS_BYTE_DATA;
    args.data = &data;
    if (ioctl(gtI2cFd, I2C_SMBUS, &args) < 0)
    {
        perror("Error: Failed to to perform SMBUS IOCTL READ transaction");
        return GT_FAIL;
    }

    *dataPtr = data.byte;

    return GT_OK;
}


/**
* @internal prvExtDrvHwIfTwsiReadByteWithOffset function
* @endinternal
*
* @brief   TODO
*
* @param[in] slave_address            - the target device slave address on I2C bus
* @param[in] buffer                   - buffer length
* @param[in] offset                   - None, 8_bit or 16_bit
* @param[in] offset                   - internal  to read from
*
* @param[out] buffer                   - received buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvExtDrvHwIfTwsiReadByteWithOffset(
  IN  GT_U8    slave_address,
  IN  GT_U32   buffer_size,
  IN  PRV_EXT_DRV_I2C_OFFSET_TYPE_ENT offset_type,
  IN  GT_U32   offset,
  OUT  GT_U8  *buffer
)
{
    int ret, msg_index = 0, offset_len = 1;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg[2];
    char tmp[2];
    char *pTmp = tmp;
    GT_STATUS RetVal;

    /* If offset type is 8 or 16 bits,
     * two messages are required within this transaction.
     * First determine offset address
     * Second - receive data.
     */
    i2c_data.msgs = msg;

    i2c_data.msgs[msg_index].addr = slave_address;
    i2c_data.nmsgs = 1;                   /* single i2c_msg */

    switch(offset_type) {
    case PRV_EXT_DRV_I2C_OFFSET_TYPE_NONE_E:
        break;

    case PRV_EXT_DRV_I2C_OFFSET_TYPE_16_E:
        /* "Write" part of the transaction */
        *pTmp = (char)((offset >> 8) & 0xFF);
        pTmp++;
        offset_len = 2;
    case PRV_EXT_DRV_I2C_OFFSET_TYPE_8_E:
        i2c_data.nmsgs = 2;                   /* two i2c_msg */
        i2c_data.msgs[msg_index].flags = 0;   /* No flags for write command */
        i2c_data.msgs[msg_index].len = offset_len;
        i2c_data.msgs[msg_index].buf =  (GT_U8 *)tmp;
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
    i2c_data.msgs[msg_index].buf = (GT_U8 *)buffer;

    ret = ioctl(gtI2cFd, I2C_RDWR, &i2c_data);
    RetVal = ret < 0 ? GT_FAIL : GT_OK;

    return RetVal;
}



