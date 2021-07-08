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
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtSmiDrv.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>
#include <asicSimulation/SCIB/scib.h>

static GT_U32 simulDeviceIds[32]; /* array with correspond indexes in scibDb */
/**
* @internal hwIfSmiIndirectRead function
* @endinternal
*
* @brief   This function reads data from a device in the secondary MII bus.
*
* @param[in] devSmiAddr               - The SMI Device id for PP.
* @param[in] regSmiAddr               - The SMI Device id for register of PP.
* @param[in] regAddr                  - The SMI register address
* @param[in] valuePtr                 - The storage where register date to be saved.
*
* @retval GT_OK                    - on success
* @retval GT_TIMEOUT               - on timeout
* @retval GT_FAIL                  - on error
*/
static GT_STATUS hwIfSmiIndirectRead (
    GT_U32  devSmiAddr,
    GT_U32  regSmiAddr,
    GT_U32  regAddr,
    GT_U32* valuePtr
)
{
    GT_U32     simData;
    GT_U32     outData;

    scibSmiRegRead(simulDeviceIds[devSmiAddr], regSmiAddr, regAddr,&simData);
    outData = simData;
    * valuePtr = outData;

    return GT_OK;
}
/**
* @internal hwIfSmiIndirectWrite function
* @endinternal
*
* @brief   This function writes data from a device in the secondary MII bus.
*
* @param[in] devSmiAddr               - The SMI Device id for PP.
* @param[in] regSmiAddr               - The SMI Device id for register of PP.
* @param[in] regAddr                  - The SMI register address
* @param[in] value                    - The register's value.
*
* @retval GT_OK                    - on success
* @retval GT_TIMEOUT               - on timeout
* @retval GT_FAIL                  - on error
*/
static GT_STATUS hwIfSmiIndirectWrite (
    GT_U32  devSmiAddr,
    GT_U32  regSmiAddr,
    GT_U32  regAddr,
    GT_U32  value
)
{
    scibSmiRegWrite(simulDeviceIds[devSmiAddr],  regSmiAddr, regAddr, value);
    return GT_OK;
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
    GT_U32 hwId;

    for (hwId = 0; hwId < 32; hwId++)
    {
        simulDeviceIds[hwId] = scibGetDeviceId(hwId);
    }
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
*
* @note For SOHO PPs the regAddr consist of 2 parts:
*       - 16 MSB is SMI device ID for register
*       - 16 LSB is register address within SMI device ID for register
*
*/
GT_STATUS hwIfSmiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_STATUS rc;
    GT_U32 regSmiDevId;
    GT_U32 regSmiRegAddr;

    /* get internal info for register */
    regSmiRegAddr = regAddr & 0xffff;
    regSmiDevId   = regAddr >> 16;

    /* The SOHO may work in two addressing modes:
       - Single mode. In this mode only 1 SMI Slave PP resides on the SMI bus.
         And register may be accessed directly by it regSmiDevId.
       - Multiple mode. In this mode several SMI Slave PPs reside on the SMI
         bus. Particular PP may be accesed only by its devSlvId. To
         access to registers inderect method must be used.
       The value of devSlvId defines addressing mode:
          0 < devSlvId <= 31             - Multiple mode and indirect access
          devSlvId == 0 or devSlvId > 31 - Single mode and direct access
    */

    /* devSlvId == 0 - is also valid SMI dev Id */
    if (devSlvId > 31)
    {
        scibSmiRegWrite(0, regSmiDevId, regSmiRegAddr, value);
        rc = GT_OK;
    }
    else
    {
        rc  = hwIfSmiIndirectWrite(devSlvId, regSmiDevId,regSmiRegAddr,value);
    }

    return rc;
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
    GT_STATUS rc;
    GT_U32 regSmiDevId;
    GT_U32 regSmiRegAddr;

    /* get internal info for register */
    regSmiRegAddr = regAddr & 0xffff;
    regSmiDevId   = regAddr >> 16;

    /* The SOHO may work in two addressing modes:
       - Single mode. In this mode only 1 SMI Slave PP resides on the SMI bus.
         And register may be accessed directly by it regSmiDevId.
       - Multiple mode. In this mode several SMI Slave PPs reside on the SMI
         bus. Particular PP may be accesed only by its devSlvId. To
         access to registers inderect method must be used.
       The value of devSlvId defines addressing mode:
          0 < devSlvId <= 31             - Multiple mode and indirect access
          devSlvId == 0 or devSlvId > 31 - Single mode and direct access
    */

    /* devSlvId == 0 - is also valid SMI dev Id */
    if (devSlvId > 31)
    {

        GT_U32     simData;
        GT_U32     outData;

        scibSmiRegRead(0, regSmiDevId, regSmiRegAddr, &simData);
        outData = simData;
        *dataPtr = outData;
        rc = GT_OK;
    }
    else
    {
        rc  = hwIfSmiIndirectRead(devSlvId, regSmiDevId,regSmiRegAddr,dataPtr);
    }

    return rc;
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

#ifdef DEBUG
    if (checkaddr == regAddr)
    {
        logMsg("hwIfSmiTaskWriteReg address 0x%08x, 0x%08x\n",
               (int)regAddr, (int)value,
               0,0,0,0);
        taskSuspend(0);
    }

    if (logHwReadWriteFlag)
    {
        postIt(devSlvId, "W", regAddr, value);
    }
#endif
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

#ifdef DEBUG
    if (checkaddr == regAddr)
    {
        logMsg("hwIfSmiTaskReadReg address 0x%08x, 0x%08x\n",
               (int)regAddr, (int)*dataPtr,
               0,0,0,0);
        taskSuspend(0);
    }

    if (logHwReadWriteFlag)
    {
        postIt(devSlvId, "R", regAddr, *dataPtr);
    }
#endif
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
    GT_STATUS       rc;

    for (i = 0; i < arrLen; i++, addr += 4)
    {
        rc = hwIfSmiReadReg(devSlvId, addr, &dataArr[i]);

        if (GT_OK != rc)
        {
            return rc;
        }
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
    GT_STATUS       rc;

    for (i = 0; i < arrLen; i++, addr += 4)
    {
        rc = hwIfSmiWriteReg(devSlvId, addr, dataArr[i]);

        if (GT_OK != rc)
        {
            return rc;
        }
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
    GT_STATUS       rc;

    for (i = 0; i < arrLen; i++)
    {
        rc = hwIfSmiReadReg(devSlvId, addrArr[i], &dataArr[i]);

        if (GT_OK != rc)
        {
            return rc;
        }
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
    GT_STATUS       rc;

    for (i = 0; i < arrLen; i++)
    {
        rc = hwIfSmiWriteReg(devSlvId, addrArr[i], dataArr[i]);

        if (GT_OK != rc)
        {
            return rc;
        }
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

#ifdef DEBUG
    if (checkaddr == regAddr)
    {
        logMsg("hwIfSmiInterruptWriteReg address 0x%08x, 0x%08x\n",
               (int)regAddr, (int)value,
               0,0,0,0);
        if (! intContext())
        {
            taskSuspend(0);
        }
    }

    if (logHwReadWriteFlag)
    {
        postIt(devSlvId, "W", regAddr, value);
    }
#endif

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

    retVal = hwIfSmiReadReg(devSlvId, regAddr, dataPtr);

#ifdef DEBUG
    if (checkaddr == regAddr)
    {
        logMsg("hwIfSmiInterruptReadReg address 0x%08x, 0x%08x\n",
               (int)regAddr, (int)*dataPtr,
               0,0,0,0);
        if (!intContext())
        {
            taskSuspend(0);
        }
    }

    if (logHwReadWriteFlag)
    {
        postIt(devSlvId, "W", regAddr, *dataPtr);
    }
#endif
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
    GT_U32      vendor;
    GT_U32      device;

    /* Prestera is little endian */
    scibReadMemory(instance, 0x00000050, 1, &vendor);

    scibReadMemory(instance, 0x0000004c, 1, &device);

    *vendorId   = (GT_U16)(vendor & 0xffff);
    *devId      = (GT_U16)((device>>4) & 0xffff);

    return GT_OK;
}

#ifdef DEBUG
int traceAddress(GT_U32 add)
{
    checkaddr = add;
    return 0;
}

int untraceAddress(GT_U32 add)
{
    checkaddr = 0xBAD0ADD0;
    return 0;
}
#endif
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
    GT_U32    data;

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





