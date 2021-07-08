/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/



#include <gtExtDrv/drivers/gtHwIfDrv.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>

#ifdef IMPL_FPGA
#include <private/USBToI2C/USBtoI2Cpro.h>
#endif

#if !(defined (ASIC_SIMULATION))
/*#warning "simulation should integrate into the PSS code"*/
#else
#include <asicSimulation/SCIB/scib.h>
#endif


/*On FPGA board all devices on the I2C bus are greater/equal to 0xB0 */
#define RHODES_STARTING_DEV (176)

#ifdef IMPL_FPGA

#define MV_ON_READ_SET_BITS31(data)    (data[0] |= 0x80)
#define MV_ON_WRITE_RESET_BIT31(data)  (data[0] &= 0x7F)
#define MV_RESET_BIT_30(data)          (data[0] &= 0xBF)

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
#endif
/**
* @internal extDrvMgmtReadRegister function
* @endinternal
*
* @brief   Managment read from GT devices register via specified hardware
*         interface.
* @param[in] mgmtIf                   - Managmnet hardware interface to device.
* @param[in] devId                    - Device id defined by interface for accessing device.
*                                      union depends on type of hardware interface.
* @param[in] regAddr                  - The device register address to read.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note For the white mode application the function calls for the scib service.
*       Galtis:
*       None.
*
*/
GT_STATUS extDrvMgmtReadRegister
(
    IN GT_MGMT_IF_TYPE      mgmtIf,
    IN GT_DEV_ID            devId,
    IN GT_U32               regAddr,
    OUT GT_U32              *dataPtr
)
{

#ifdef IMPL_FPGA
    
        GT_STATUS       rc;
        GT_U8           regCharAddr[4];
        GT_U8           regCharData[4];
        GT_U8           readValue[4];
        GT_U8           regCharAddrData[8];
        GT_U8           size;

        /*Phase 1: Master Drives Address and Data over TWSI*/
        mvLongToChar(regAddr, regCharAddr);
        MV_ON_READ_SET_BITS31(regCharAddr);
        MV_RESET_BIT_30(regCharAddr);
    
        size = (sizeof(regCharAddr) / sizeof(regCharAddr[0]));

        rc = I2CWrite(devId.i2cBaseAddr, size, &regCharAddr, 1);
        
        rc = I2CRead(devId.i2cBaseAddr, size, &readValue, 1);

        mvCharToLong(readValue, dataPtr);

        return rc;
    

#endif 

    scibReadMemory(devId.devNum, regAddr, 1, dataPtr);

    return (GT_OK);
}

/**
* @internal extDrvMgmtWriteRegister function
* @endinternal
*
* @brief   Managment write to GT devices registers via specified hardware
*         interface.
* @param[in] mgmtIf                   - Managmnet hardware interface to device.
* @param[in] devId                    - Device id defined by interface for accessing device.
*                                      union depends on type of hardware interface.
* @param[in] regAddr                  - The device register address to write.
* @param[in] data                     - Data word to write to device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note For the white mode application the function calls for the scib service.
*       Galtis:
*       None.
*
*/
GT_STATUS extDrvMgmtWriteRegister
(
    IN GT_MGMT_IF_TYPE      mgmtIf,
    IN GT_DEV_ID            devId,
    IN GT_U32               regAddr,
    IN GT_U32               data
)
{
   
#ifdef IMPL_FPGA
        
        
        GT_STATUS rc;
        GT_U8           regCharAddr[4];
        GT_U8           regCharData[4];
        GT_U8           regCharAddrData[8];
        GT_U8           size; 

        /*Phase 1: Master Drives Address and Data over TWSI*/
        mvLongToChar(regAddr, regCharAddr);
        MV_ON_WRITE_RESET_BIT31(regCharAddr);
        MV_RESET_BIT_30(regCharAddr);
    
        mvLongToChar(data, regCharData);
        mvConcatCharArray(regCharAddr,regCharData,regCharAddrData);

        size = (sizeof(regCharAddrData) / sizeof(regCharAddrData[0]));
        rc = I2CWrite(devId.i2cBaseAddr, size, &regCharAddrData, 1);
        

        return rc;
    
#endif 
   
    
    scibWriteMemory(devId.devNum, regAddr, 1, &data);

    return (GT_OK);
}

/**
* @internal extDrvMgmtIsrReadRegister function
* @endinternal
*
* @brief   Managment read from GT devices register, guaranteed to be non blocked
*         access via specified hardware interface.
* @param[in] mgmtIf                   - Managmnet hardware interface to device.
* @param[in] devId                    - Device id defined by interface for accessing device.
*                                      union depends on type of hardware interface.
* @param[in] regAddr                  - The device register address to read.
*
* @param[out] data                     - Data word read from device, is valid only
*                                      if status == GT_OK.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note For the white mode application the function calls for the scib service.
*       Galtis:
*       None.
*
*/
GT_STATUS extDrvMgmtIsrReadRegister
(
    IN GT_MGMT_IF_TYPE      mgmtIf,
    IN GT_DEV_ID            devId,
    IN GT_U32               regAddr,
    OUT GT_U32              *data
)
{
    scibReadMemory(devId.devNum, regAddr, 1, data);

    return (GT_OK);
}


/**
* @internal extDrvMgmtIsrWriteRegister function
* @endinternal
*
* @brief   Managment write to GT devices register, guaranteed to be non blocked
*         access via specified hardware interface.
* @param[in] mgmtIf                   - Managmnet hardware interface to device.
* @param[in] devId                    - Device id defined by interface for accessing device.
*                                      union depends on type of hardware interface.
* @param[in] regAddr                  - The device register address to write.
* @param[in] data                     - Data word to write to device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note For the white mode application the function calls for the scib service.
*       Galtis:
*       None.
*
*/
GT_STATUS extDrvMgmtIsrWriteRegister
(
    IN GT_MGMT_IF_TYPE      mgmtIf,
    IN GT_DEV_ID            devId,
    IN GT_U32               regAddr,
    IN GT_U32               data
)
{
    scibWriteMemory(devId.devNum, regAddr, 1, &data);

    return (GT_OK);
}





