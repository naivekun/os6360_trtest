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
* @file genDrvTwsiHwCtrl.c
*
* @brief API implementation for TWSI facilities.
*
*/

/*Includes*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>  /* for NAME_MAX */
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include <gtExtDrv/drivers/pssBspApis.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsTimer.h>

#include "kerneldrv/include/presteraGlob.h"

/* file descriptor returnd by openning the PP *nix device driver */
static GT_32 gtI2cFd = -1;
static GT_U32 i2cCurrentSlaveId;

/* Macroes definitions */
#define TWSI_SLAVE_ADDR(data)     (data)

#define MV_ON_READ_SET_BITS31(data)    (data[0] |= 0x80)
#define MV_ON_WRITE_RESET_BIT31(data)  (data[0] &= 0x7F)
#define MV_RESET_BIT_30(data)          (data[0] &= 0xBF)

static GT_STATUS mvLongToChar (IN GT_U32 src, 
                               OUT GT_U8 dst[4]);

static GT_STATUS mvCharToLong (IN GT_U8 src[4], 
                               OUT GT_U32 *dst);

static GT_STATUS mvConcatCharArray (IN GT_U8 src0[4], 
                                    IN GT_U8 src1[4], 
                                    OUT GT_U8 dst[8]);

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
        fprintf(stderr, "Error: Could not set slave address: %s\n", strerror(errno));
        return GT_FAIL;
    }

    i2cCurrentSlaveId = devSlvId;

    osTimerWkAfter(10);

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
    GT_U8           regCharAddr[4];
    GT_U8           regCharData[4];
    GT_U8           regCharAddrData[8];
    struct GT_TwsiReadWrite_STC twsiWritePrm;

    /*Phase 1: Master Drives Address and Data over TWSI*/
    mvLongToChar (regAddr, regCharAddr);
    MV_ON_WRITE_RESET_BIT31 (regCharAddr);
    MV_RESET_BIT_30 (regCharAddr);

    mvLongToChar (value, regCharData);
    mvConcatCharArray(regCharAddr,regCharData,regCharAddrData);

    if (ioctl (gtI2cFd, PRESTERA_IOC_TWSIWAITNOBUSY) < 0)
    {
        fprintf(stderr, "hwIfTwsiWriteReg(PRESTERA_IOC_TWSIWAITNOBUSY) fail errno(%s)\n",strerror(errno));
        return GT_FAIL;
    }

    twsiWritePrm.devId  = TWSI_SLAVE_ADDR(devSlvId);
    twsiWritePrm.len    = 8;
    twsiWritePrm.pData  = (mv_kmod_uintptr_t)((uintptr_t)regCharAddrData);
    twsiWritePrm.stop   = GT_TRUE;

    if (ioctl (gtI2cFd, PRESTERA_IOC_TWSIWRITE, &twsiWritePrm) < 0)
    {
        fprintf(stderr, "hwIfTwsiWriteReg(PRESTERA_IOC_TWSIWRITE) fail errno(%s)\n",strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
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
    GT_U8           twsiRdDataBuff[4];
    GT_U8           regCharAddr[4];       
    struct GT_TwsiReadWrite_STC twsiWritePrm;

    /*PHASE 1: Write Structure, Master drives regAddr over TWSI*/
    mvLongToChar(regAddr, regCharAddr);
    MV_ON_READ_SET_BITS31 (regCharAddr);
    MV_RESET_BIT_30 (regCharAddr);
    
    if (ioctl (gtI2cFd, PRESTERA_IOC_TWSIWAITNOBUSY) < 0)
    {
        fprintf(stderr, "hwIfTwsiWriteReg(PRESTERA_IOC_TWSIWAITNOBUSY) fail errno(%s)\n",strerror(errno));
        return GT_FAIL;
    }

    twsiWritePrm.devId  = TWSI_SLAVE_ADDR(devSlvId);
    twsiWritePrm.len    = 4;
    twsiWritePrm.pData  = (mv_kmod_uintptr_t)((uintptr_t)regCharAddr);
    twsiWritePrm.stop   = GT_FALSE;

    if (ioctl (gtI2cFd, PRESTERA_IOC_TWSIWRITE, &twsiWritePrm) < 0)
    {
        fprintf(stderr, "hwIfTwsiWriteReg(PRESTERA_IOC_TWSIWRITE) fail errno(%s)\n",strerror(errno));
        return GT_FAIL;
    }

    twsiWritePrm.devId  = TWSI_SLAVE_ADDR(devSlvId);
    twsiWritePrm.len    = 4;
    twsiWritePrm.pData  = (mv_kmod_uintptr_t)((uintptr_t)twsiRdDataBuff);
    twsiWritePrm.stop   = GT_TRUE;

    if (ioctl (gtI2cFd, PRESTERA_IOC_TWSIREAD, &twsiWritePrm) < 0)
    {
        fprintf(stderr, "hwIfTwsiWriteReg(PRESTERA_IOC_TWSIREAD) fail errno(%s)\n",strerror(errno));
        return GT_FAIL;
    }
    
    mvCharToLong(twsiRdDataBuff, dataPtr);

    return GT_OK;
}


/**
* @internal mvLongToChar function
* @endinternal
*
* @brief   Transforms unsigned long int type to 4 separate chars.
*
* @param[in] src                      - source unsigned long integer.
*
* @param[out] dst[4]                   - Array of 4 chars
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note MSB is copied to dst[0]!!
*
*/
static GT_STATUS mvLongToChar (IN GT_U32 src, 
                               OUT GT_U8 dst[4])
{
    GT_U32 i;

    for (i = 4 ; i > 0 ; i--)
    {
        dst[i-1] = (GT_U8) src & 0xFF;
        src>>=8;
    }

  return GT_OK;
}

/**
* @internal mvCharToLong function
* @endinternal
*
* @brief   Transforms an array of 4 separate chars to unsigned long integer type
*
* @param[in] src[4]                   - Source Array of 4 chars
*
* @param[out] dst                      - Unsigned long integer number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note MSB resides in src[0]!!
*
*/
static GT_STATUS mvCharToLong (IN GT_U8 src[4], 
                               OUT GT_U32 *dst)
{
    GT_U32  i;
    GT_U32  tempU32 = 0x00000000;    

    for (i = 4 ; i > 0 ; i--)
    {
        tempU32 += ((GT_U32)src[i-1]) << (8*(4-i));
    }

  *dst = tempU32;
  return GT_OK;
}


/**
* @internal mvConcatCharArray function
* @endinternal
*
* @brief   Concatinate 2 Arrays of Chars to one Array of chars
*
* @param[in] src0[4]                  - Source Array of 4 chars long
* @param[in] src1[4]                  - Source Array of 4 chars long
*
* @param[out] dst[8]                   - Conacatinated Array of 8 chars long {src1,src0}
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
static GT_STATUS mvConcatCharArray (IN GT_U8 src0[4], 
                                    IN GT_U8 src1[4], 
                                    OUT GT_U8 dst[8])
{
    GT_U32 i=0,j; /*Source and Dest Counters*/

    for (j=0; j < 8; j++)
    {
        if (j < 4)
            dst[j] = src0[i++];
        else if (j == 4)
        {
            dst[j] = src1[0];
            i=1;
        } else
            dst[j] = src1[i++];
    }
    return GT_OK;
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
        fprintf(stderr, "failed to open /dev/i2c-*: %s\n", strerror(errno));
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
        fprintf(stderr, "Error: Failed to to perform SMBUS IOCTL READ transaction: %s\n", strerror(errno));
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
        fprintf(stderr, "Error: Failed to to perform SMBUS IOCTL WRITE transaction: %s\n", strerror(errno));
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
        fprintf(stderr, "Error: Failed to to perform SMBUS IOCTL READ transaction: %s\n", strerror(errno));
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
        fprintf(stderr, "Error: Failed to to perform SMBUS IOCTL READ transaction: %s\n", strerror(errno));
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
        fprintf(stderr, "Error: Failed to to perform SMBUS IOCTL WRITE transaction: %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* Read data */
    args.read_write = I2C_SMBUS_READ;
    args.command = regAddr; /* Byte Address */
    args.size = I2C_SMBUS_BYTE_DATA;
    args.data = &data;
    if (ioctl(gtI2cFd, I2C_SMBUS, &args) < 0)
    {
        fprintf(stderr, "Error: Failed to to perform SMBUS IOCTL READ transaction: %s\n", strerror(errno));
        return GT_FAIL;
    }

    *dataPtr = data.byte;

    return GT_OK;
}



