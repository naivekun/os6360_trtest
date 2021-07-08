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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "kerneldrv/include/presteraSmiGlob.h"

/* file descriptor returnd by openning the PP device driver */
extern GT_32 gtPpFd;

#define STUB_FAIL printf("stub function %s returning MV_NOT_SUPPORTED\n", \
                         __FUNCTION__) ; return GT_FAIL

#ifdef SMI_DBG
static GT_U32 smiDbgPrint = 0;
extern GT_U32 smiDbgPrintEn(IN GT_U32  enable)
{
    GT_U32  oldState = smiDbgPrint;
    smiDbgPrint = enable;

    return oldState;
}
#endif

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
  SMI_REG smiReg;

  smiReg.slvId   = devSlvId;
  smiReg.regAddr = regAddr;
  smiReg.value   = value;

#ifdef SMI_DBG  
  if(smiDbgPrint)
  {
      printf("hwIfSmiWriteReg:slvId=0x%x,regAddr=0x%x,value=0x%x\n", 
             smiReg.slvId, smiReg.regAddr, smiReg.value);
  }
#endif

  if (ioctl(gtPpFd, PRESTERA_SMI_IOC_WRITEREG, &smiReg))
  {
    fprintf(stderr, "Fail to write reg 0x%x errno(%s)\n",
            smiReg.regAddr, strerror(errno));
    return GT_FAIL;
  }
     
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
  SMI_REG smiReg;
  
  smiReg.slvId   = devSlvId;
  smiReg.regAddr = regAddr;
  
  if (ioctl(gtPpFd, PRESTERA_SMI_IOC_READREG, &smiReg))
  {
    fprintf(stderr, "Fail to read reg 0x%x errno(%s)\n",
            smiReg.regAddr, strerror(errno));
    return GT_FAIL;
  }
  
  *dataPtr = smiReg.value;
  
#ifdef SMI_DBG
    if(smiDbgPrint)
    {
        printf("hwIfSmiReadReg:slvId=0x%x,regAddr=0x%x,value=0x%x\n", 
               smiReg.slvId, smiReg.regAddr, *dataPtr);
    }
#endif

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
    return retVal;
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
    STUB_FAIL;
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
    STUB_FAIL;
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
    SMI_REG_RAM_STC smiRegRam;

    smiRegRam.devSlvId = devSlvId;
    smiRegRam.addr     = addr;
    smiRegRam.dataArr  = dataArr;
    smiRegRam.arrLen   = arrLen;

    if (ioctl (gtPpFd, PRESTERA_SMI_IOC_READREGRAM, &smiRegRam))
    {
        fprintf(stderr, "Fail to read reg Ram 0x%x errno(%s)\n",
                        smiRegRam.addr, strerror(errno));
        return GT_FAIL;
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
    SMI_REG_RAM_STC smiRegRam;

    smiRegRam.devSlvId = devSlvId;
    smiRegRam.addr     = addr;
    smiRegRam.dataArr  = dataArr;
    smiRegRam.arrLen   = arrLen;

    if (ioctl (gtPpFd, PRESTERA_SMI_IOC_WRITEREGRAM, &smiRegRam))
    {
        fprintf(stderr, "Fail to write reg Ram 0x%x errno(%s)\n",
                        smiRegRam.addr, strerror(errno));
        return GT_FAIL;
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
    GT_U32      i;  /* iterator */
    GT_STATUS   rc = GT_OK; /* return code */

    for(i = 0; i < arrLen; i++)
    {
        rc = hwIfSmiTaskReadReg(devSlvId,addrArr[i],&dataArr[i]);
        if(rc != GT_OK)
            break;
    }

    return rc;
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
    GT_U32      i;  /* iterator */
    GT_STATUS   rc = GT_OK; /* return code */

    for(i = 0; i < arrLen; i++)
    {
        rc = hwIfSmiTaskWriteReg(devSlvId,addrArr[i],dataArr[i]);
        if(rc != GT_OK)
            break;
    }

    return rc;
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

#ifdef XCAT_DRV
  if (instance != 0x10) /* we only deal with the first device */
    return GT_FAIL;
  instance = 0;
#endif

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
  
  *vendorId   = vendor & 0xffff;
  *devId      = (device>>4) & 0xffff;
  
  return GT_OK;
}

/**
* @internal extDrvXSmiInit function
* @endinternal
*
* @brief   Inits XSMI subsystem
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
* @retval GT_NOT_IMPLEMENTED       - on not implemented
*/
GT_STATUS extDrvXSmiInit
(
    void
)
{
    return GT_NOT_IMPLEMENTED;
}

/**
* @internal extDrvXSmiReadReg function
* @endinternal
*
* @brief   Reads a register using XSMI Address
*
* @param[in] phyId                    -  ID
* @param[in] devAddr                  - (Clause 45) Device Address (page number in case of PHY)
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - pointer to data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
* @retval GT_BAD_PARAM             - bad param
* @retval GT_NOT_IMPLEMENTED       - on not implemented
*/
GT_STATUS extDrvXSmiReadReg
(
    IN  GT_U32  phyId,
    IN  GT_U32  devAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
   (void)phyId;
   (void)devAddr;
   (void)regAddr;
   *dataPtr = 0;

   return GT_NOT_IMPLEMENTED;;
}

/**
* @internal extDrvXSmiWriteReg function
* @endinternal
*
* @brief   Writes a register using XSMI Address
*
* @param[in] phyId                    - phy ID
* @param[in] devAddr                  - (Clause 45) Device Address (page number in case of PHY)
* @param[in] regAddr                  - Register address to write.
* @param[in] value                    - Data write to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
* @retval GT_BAD_PARAM             - bad param
* @retval GT_NOT_IMPLEMENTED       - on not implemented
*/
GT_STATUS extDrvXSmiWriteReg
(
    IN GT_U32 phyId,
    IN  GT_U32 devAddr,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    (void)phyId;
    (void)devAddr;
    (void)regAddr;
    (void)value;

    return GT_NOT_IMPLEMENTED;
}


