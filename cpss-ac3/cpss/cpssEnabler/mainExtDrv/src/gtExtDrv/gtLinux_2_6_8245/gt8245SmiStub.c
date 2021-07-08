/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

/*Includes*/

#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>

/**
* @internal extDrvSmiDevVendorIdGet function
* @endinternal
*
* @brief   This routine returns vendor Id of the given device.
*
* @param[in] instance                 - The requested device instance.
*
* @param[out] vendorId                 <- The device vendor Id.
* @param[out] devId                    <- The device Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvSmiDevVendorIdGet
(
    OUT GT_U16  *vendorId,
    OUT GT_U16  *devId,
    IN  GT_U32  instance
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiInitDriver function
* @endinternal
*
* @brief   Init the SMI interface
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiInitDriver
(
    GT_VOID
)
{
    return GT_NOT_SUPPORTED;
}


/**
* @internal hwIfSmiWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    return GT_FAIL;
}


/**
* @internal hwIfSmiReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiReadReg
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiTaskWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiTaskWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiTaskReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiTaskReadReg
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiInterruptWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiInterruptWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiInterruptReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiInterruptReadReg
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    return GT_FAIL;
}

/**
* @internal extDrvDirectSmiReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectSmiReadReg
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *value
)
{
    return GT_FAIL;
}
/**
* @internal extDrvDirectSmiWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*                                      dataPtr    - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectSmiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiTskRegRamRead function
* @endinternal
*
* @brief   Reads a memory map (contiguous memory) using SMI from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addr                     - Register address to start write the writing.
* @param[in] arrLen                   - The length of dataArr (the number of registers to read)
*
* @param[out] dataArr[]                - An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*
* @note - register address is incremented in 4 byte per register
*
*/
GT_STATUS hwIfSmiTskRegRamRead
(
    IN GT_U32       devSlvId,
    IN GT_U32       addr,
    OUT GT_U32      dataArr[],
    IN GT_U32       arrLen
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiTskRegRamWrite function
* @endinternal
*
* @brief   Writes a memory map (contiguous memory) using SMI from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addr                     - Register address to start write the reading.
* @param[in] arrLen                   - The size of addrArr/dataArr.
* @param[in] dataArr[]                - An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*
* @note - register address is incremented in 4 byte per register
*
*/
GT_STATUS hwIfSmiTskRegRamWrite
(
    IN GT_U32       devSlvId,
    IN GT_U32       addr,
    OUT GT_U32      dataArr[],
    IN GT_U32       arrLen
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiTskRegVecRead function
* @endinternal
*
* @brief   Reads SMI register vector from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addrArr[]                - Array of addresses to write to.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @param[out] dataArr[]                - An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiTskRegVecRead
(
    IN GT_U32       devSlvId,
    IN GT_U32       addrArr[],
    OUT GT_U32      dataArr[],
    IN GT_U32       arrLen
)
{
    return GT_FAIL;
}

/**
* @internal hwIfSmiTskRegVecWrite function
* @endinternal
*
* @brief   Writes SMI register vector from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addrArr[]                - Array of addresses to write to.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiTskRegVecWrite
(
    IN GT_U32       devSlvId,
    IN GT_U32       addrArr[],
    IN GT_U32       dataArr[],
    IN GT_U32       arrLen
)
{
    return GT_FAIL;
}



