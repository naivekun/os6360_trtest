/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

/*Includes*/

#include <gtExtDrv/os/extDrvOs.h>
#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtSmiDrv.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>
#include <asicSimulation/SCIB/scib.h>
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
    return GT_OK;
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
    scibWriteMemory(devSlvId, regAddr, 1, &value);

    return GT_OK;
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
    scibReadMemory(devSlvId, regAddr, 1, dataPtr);

    return GT_OK;
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
    GT_STATUS retVal;

    retVal = hwIfSmiWriteReg(devSlvId, regAddr, value);

    return retVal;
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
    GT_STATUS retVal;

    retVal = hwIfSmiReadReg(devSlvId, regAddr, dataPtr);

    return retVal;
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
    GT_U32          i;

    for (i = 0; i < arrLen; i++, addr += 4)
    {
        scibReadMemory(devSlvId, addr, 1, &dataArr[i]);
    }

    return GT_OK;
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
    GT_U32          i;

    for (i = 0; i < arrLen; i++, addr += 4)
    {
        scibWriteMemory(devSlvId, addr, 1, &dataArr[i]);
    }

    return GT_OK;
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
    GT_U32          i;

    for (i = 0; i < arrLen; i++)
    {
        scibReadMemory(devSlvId, addrArr[i], 1, &dataArr[i]);
    }

    return GT_OK;
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
    GT_U32          i;

    for (i = 0; i < arrLen; i++)
    {
        scibWriteMemory(devSlvId, addrArr[i], 1, &dataArr[i]);
    }

    return GT_OK;
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
    GT_STATUS retVal;
    GT_U32 intKey;
    intKey = 0;

    retVal = hwIfSmiWriteReg(devSlvId, regAddr, value);

    return retVal;
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
    GT_STATUS retVal;
    GT_U32 intKey;
    intKey = 0;

    retVal = hwIfSmiReadReg(devSlvId, regAddr, dataPtr);

    return retVal;
}
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
    GT_STATUS   retVal;
    GT_U32      vendor;
    GT_U32      device;

    /* Prestera is little endian */
    retVal = hwIfSmiTaskReadReg(instance, 0x00000050, &vendor);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    retVal = hwIfSmiTaskReadReg(instance, 0x0000004C, &device);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    *vendorId   = (GT_U16)(vendor & 0xffff);
    *devId      = (GT_U16)((device>>4) & 0xffff);

    return GT_OK;
}

/**
* @internal extDrvSmiFindDev function
* @endinternal
*
* @brief   This routine returns the next instance of the given device (defined by
*         vendorId & devId).
* @param[in] vendorId                 - The device vendor Id.
* @param[in] devId                    - The device Id.
* @param[in] instance                 - The requested device instance.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvSmiFindDev
(
    IN  GT_U16  vendorId,
    IN  GT_U16  devId,
    IN  GT_U32  instance,
    OUT GT_U32  *devSel
)
{
    GT_STATUS retVal;
    GT_U32 i, found;
    GT_U32 vendorField, vendorMask;
    GT_U32 deviceField, deviceMask;
    GT_U32 vendor;
    GT_U32 device;

    vendorField = vendorId;
    vendorMask  = 0xffff;
    deviceField = devId<<4;
    deviceMask  = 0xffff0;

    /* Prestera is little endian */
    /* CPU is ? */
    found = 0;
    for (i = 0; i < 32; i++)
    {
        retVal = hwIfSmiTaskReadReg(i, 0x00000050, &vendor);
        if (GT_OK != retVal)
        {
            return retVal;
        }
        if ((vendor & vendorMask) != vendorField)
        {
            continue;
        }
        retVal = hwIfSmiTaskReadReg(i, 0x0000004C, &device);
        if (GT_OK != retVal)
        {
            return retVal;
        }
        if ((device & deviceMask) != deviceField)
        {
            continue;
        }
        if (found == instance)
        {
            *devSel = i;
            return GT_OK;
        }
        found++;
    }

    return GT_FAIL;
}


/**
* @internal hwIfSmiTaskWriteRegField function
* @endinternal
*
* @brief   Write value to selected register field.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to write to.
*                                      fieldOffset - The start bit number in the register.
*                                      fieldLength - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note this function actually read the register value modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS hwIfSmiTaskWriteRegField
(
    IN GT_U32 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 mask,
    IN GT_U32 fieldData

)
{
    GT_STATUS retVal;
    GT_U32           data;

    retVal = hwIfSmiTaskReadReg( devNum, regAddr,&data);
    if (GT_OK != retVal)
    {
        return retVal;
    }
    data &= ~mask;   /* turn the field off */
    /* insert the new value of field in its place */
    data |= (fieldData  & mask);

    retVal = hwIfSmiTaskWriteReg( devNum, regAddr, data);

        return retVal;
}



