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
#include <gtExtDrv/drivers/pssBspApis.h>
#include <gtExtDrv/drivers/gtHwIfOp.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>

#if (defined CHX_FAMILY || defined SAL_FAMILY)
    #define DX_FAMILY
#endif /*(defined CHX_FAMILY || defined SAL_FAMILY)*/

/* Constants */
#ifdef GT_SMI

#ifdef DX_FAMILY

/* speific for salsa/cheetah */
#define  SMI_WRITE_ADDRESS_MSB_REGISTER   (0x00)
#define  SMI_WRITE_ADDRESS_LSB_REGISTER   (0x01)
#define  SMI_WRITE_DATA_MSB_REGISTER      (0x02)
#define  SMI_WRITE_DATA_LSB_REGISTER      (0x03)

#define  SMI_READ_ADDRESS_MSB_REGISTER    (0x04)
#define  SMI_READ_ADDRESS_LSB_REGISTER    (0x05)
#define  SMI_READ_DATA_MSB_REGISTER       (0x06)
#define  SMI_READ_DATA_LSB_REGISTER       (0x07)

#define  SMI_STATUS_REGISTER              (0x1f)

#define SMI_STATUS_WRITE_DONE             (0x02)
#define SMI_STATUS_READ_READY             (0x01)

#define SMI_TIMEOUT_COUNTER  (1000)
#endif /* DX_FAMILY */
#endif /* GT_SMI */

#ifdef GT_SMI
/* SMI mutex semaphore */
static GT_SEM smiLockSem = NULL;

#ifdef DEBUG
extern int taskSuspend(int tid);
extern int logMsg(char*,int,int,int,int,int,int);
extern int intContext(void);
extern int wdbUserEvtPost
(
    char * event              /* event string to send */
);
static int logHwReadWriteFlag = 0;
int logHwReadWriteSet() {logHwReadWriteFlag = 1; return 0;}
int logHwReadWriteUnSet() {logHwReadWriteFlag = 0; return 0;}
static void postIt(GT_U32 pp, GT_U8 *op, GT_U32 add, GT_U32 val)
{
    char buffer[128];
    osSprintf(buffer, "PP %d %s ADD-0x%08X VAL-0x%08X", pp, op, add, val);
    wdbUserEvtPost(buffer);
}
static GT_U32 checkaddr = 0xBAD0ADD0;
#endif /* DEBUG */

/* Prototype for extDrvSmiDevVendorIdGet */
GT_STATUS hwIfSmiTaskReadReg
(
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
);

#endif /* GT_SMI */

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
#ifdef GT_SMI
#if defined(DX_FAMILY)
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

    *vendorId   = vendor & 0xffff;
    *devId      = (device>>4) & 0xffff;

    return GT_OK;
#else /* !DX_FAMILY */
    return GT_NOT_SUPPORTED;
#endif
#else /* !GT_SMI*/
    return GT_NOT_SUPPORTED;
#endif
}

#ifndef CPU_EMULATED_BUS
#ifdef GT_SMI
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
#if defined(DX_FAMILY)

    GT_STATUS           rc;
    GT_U32              msb;
    GT_U32              lsb;
#ifdef  SMI_WAIT_FOR_STATUS_DONE
    register GT_U32     timeOut;
    GT_U32              stat;
#endif /* SMI_WAIT_FOR_STATUS_DONE */

    /* write addr to read */
    msb = regAddr >> 16;
    lsb = regAddr & 0xFFFF;
    rc = bspSmiWriteReg(devSmiAddr,0,SMI_READ_ADDRESS_MSB_REGISTER,msb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = bspSmiWriteReg(devSmiAddr,0,SMI_READ_ADDRESS_LSB_REGISTER,lsb);
    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef  SMI_WAIT_FOR_STATUS_DONE
    /* wait for read done */
    for (timeOut = SMI_TIMEOUT_COUNTER; ; timeOut--)
    {
        rc = bspSmiReadReg(devSmiAddr, 0, SMI_STATUS_REGISTER, &stat);
        if (rc != GT_OK)
        {
            return rc;
        }

        if ((stat & SMI_STATUS_READ_READY) != 0)
        {
            break;
        }

        if (0 == timeOut)
        {
            return GT_FAIL;
        }
    }
#endif /* SMI_WAIT_FOR_STATUS_DONE */

    /* read data */
    rc = bspSmiReadReg(devSmiAddr,0,SMI_READ_DATA_MSB_REGISTER,&msb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = bspSmiReadReg(devSmiAddr,0,SMI_READ_DATA_LSB_REGISTER,&lsb);
    if (rc != GT_OK)
    {
        return rc;
    }

    *valuePtr = ((msb & 0xFFFF) << 16) | (lsb & 0xFFFF);

    return GT_OK;
    /* DX_FAMILY*/
#else
    return GT_NOT_SUPPORTED;
#endif /* !DX_FAMILY */
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
#if defined(DX_FAMILY)

    GT_STATUS           rc;
    GT_U32              msb;
    GT_U32              lsb;
#ifdef  SMI_WAIT_FOR_STATUS_DONE
    register GT_U32     timeOut;
    GT_U32              stat;

    /* wait for write done */
    for (timeOut = SMI_TIMEOUT_COUNTER; ; timeOut--)
    {
        rc = bspSmiReadReg(devSmiAddr, 0, SMI_STATUS_REGISTER, &stat);
        if (rc != GT_OK)
        {
            return rc;
        }

        if ((stat & SMI_STATUS_WRITE_DONE) != 0)
        {
            break;
        }

        if (0 == timeOut)
        {
            return GT_FAIL;
        }
    }
#endif /* SMI_WAIT_FOR_STATUS_DONE */

    /* write addr to write */
    msb = regAddr >> 16;
    lsb = regAddr & 0xFFFF;
    rc = bspSmiWriteReg(devSmiAddr,0,SMI_WRITE_ADDRESS_MSB_REGISTER,msb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = bspSmiWriteReg(devSmiAddr,0,SMI_WRITE_ADDRESS_LSB_REGISTER,lsb);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* write data to write */
    msb = value >> 16;
    lsb = value & 0xFFFF;
    rc = bspSmiWriteReg(devSmiAddr,0,SMI_WRITE_DATA_MSB_REGISTER,msb);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = bspSmiWriteReg(devSmiAddr,0,SMI_WRITE_DATA_LSB_REGISTER,lsb);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
#else /* !DX_FAMILY */
    return GT_NOT_SUPPORTED;
#endif
}
#endif /* GT_SMI */
#endif /* CPU_EMULATED_BUS */

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
#ifdef GT_SMI
    GT_STATUS             rc;
    bspSmiAccessMode_ENT  smiAccessMode; /* relevant for SOHO only */

    rc = bspSmiInitDriver(&smiAccessMode);

    return rc;

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
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
#ifdef CPU_EMULATED_BUS
    GT_SMI_OPER_STC smiOp;
    GT_HW_IF_UNT    hwIf;
    GT_STATUS retVal;

    hwIf.smiOper = &smiOp;

    smiOp.opCode        = GT_WRITE_SMI_BUFFER_E;
    smiOp.smiDeviceAddr = devSlvId;
    smiOp.regAddress    = regAddr;
    smiOp.smiData       = value;

    retVal = extDrvHwIfOperExec(1, &hwIf, NULL);

    return retVal;
#else
#ifdef GT_SMI
#if defined(DX_FAMILY)

    return hwIfSmiIndirectWrite(devSlvId, 0, regAddr, value);
    /* DX_FAMILY */
#else /* !DX_FAMILY */
    return GT_NOT_SUPPORTED;
#endif

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
#endif /* CPU_EMULATED_BUS */
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
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiReadReg
(
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *valuePtr
)
{
#ifdef CPU_EMULATED_BUS
    GT_STATUS       retVal;
    GT_SMI_OPER_STC smiOp;
    GT_HW_IF_UNT    hwIf;

    hwIf.smiOper = &smiOp;

    smiOp.opCode        = GT_READ_SMI_BUFFER_E;
    smiOp.smiDeviceAddr = devSlvId;
    smiOp.regAddress    = regAddr;

    retVal = extDrvHwIfOperExec(1, &hwIf, NULL);

    *valuePtr = smiOp.smiData;

    return retVal;
#else
#ifdef GT_SMI
#if defined(DX_FAMILY)

    return hwIfSmiIndirectRead(devSlvId, 0, regAddr, valuePtr);
    /* DX_FAMILY */
#else /* !DX_FAMILY */
    return GT_NOT_SUPPORTED;
#endif

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
#endif /* CPU_EMULATED_BUS */
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
#ifdef GT_SMI
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
#endif /* DEBUG */

    if (NULL == smiLockSem)
    {
        /* create semaphore for SMI read/write routines */
        if(osSemBinCreate("smiLock", OS_SEMB_FULL, &smiLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if (osSemWait(smiLockSem, OS_WAIT_FOREVER) != GT_OK)
    {
        return GT_FAIL;
    }
    retVal = hwIfSmiWriteReg(devSlvId, regAddr, value);
    osSemSignal(smiLockSem);

    return retVal;
#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
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
#ifdef GT_SMI
    GT_STATUS retVal;

    if (NULL == smiLockSem)
    {
        /* create semaphore for SMI read/write routines */
        if(osSemBinCreate("smiLock", OS_SEMB_FULL, &smiLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if (osSemWait(smiLockSem, OS_WAIT_FOREVER) != GT_OK)
    {
        return GT_FAIL;
    }

    retVal = hwIfSmiReadReg(devSlvId, regAddr, dataPtr);
    osSemSignal(smiLockSem);

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
#endif /* DEBUG */
    return retVal;

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
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
#ifdef GT_SMI
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
#endif /* debug */

    if (NULL == smiLockSem)
    {
        /* create semaphore for SMI read/write routines */
        if(osSemBinCreate("smiLock", OS_SEMB_FULL, &smiLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if (osSemWait(smiLockSem, OS_WAIT_FOREVER) != GT_OK)
    {
        return GT_FAIL;
    }

    retVal = hwIfSmiWriteReg(devSlvId, regAddr, value);
    osSemSignal(smiLockSem);

    return retVal;

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
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
#ifdef GT_SMI
    GT_STATUS retVal;

    if (NULL == smiLockSem)
    {
        /* create semaphore for SMI read/write routines */
        if(osSemBinCreate("smiLock", OS_SEMB_FULL, &smiLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if (osSemWait(smiLockSem, OS_WAIT_FOREVER) != GT_OK)
    {
        return GT_FAIL;
    }

    retVal = hwIfSmiReadReg(devSlvId, regAddr, dataPtr);
    osSemSignal(smiLockSem);

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
#endif /* DEBUG */

    return retVal;

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
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
#ifdef GT_SMI
    return bspSmiReadReg(devSlvId,0,regAddr,value);

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
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
#ifdef GT_SMI
    return bspSmiWriteReg(devSlvId,0,regAddr,value);
#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
}


#if defined(GT_SMI) || defined(CPU_EMULATED_BUS)
/**
* @internal hwIfSmiRegVecWrite function
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
*
* @note For SOHO PPs the regAddr consist of 2 parts:
*       - 16 MSB is SMI device ID for register
*       - 16 LSB is register address within SMI device ID for register
*
*/
static GT_STATUS hwIfSmiRegVecWrite
(
    IN GT_U32       devSlvId,
    IN GT_U32       addrArr[],
    IN GT_U32       dataArr[],
    IN GT_U32       arrLen
)
{
#ifdef CPU_EMULATED_BUS
    GT_SMI_OPER_STC smiOp[MAX_REG_CNT];
    GT_HW_IF_UNT    hwIf;
    GT_STATUS       retVal;
    GT_U32          i, opIdx;

    hwIf.smiOper = smiOp;
    i = 0;

    while (i < arrLen)
    {
        for (opIdx = 0; opIdx < MAX_REG_CNT && i < arrLen; i++, opIdx++)
        {
            smiOp[opIdx].opCode        = GT_WRITE_SMI_BUFFER_E;
            smiOp[opIdx].smiDeviceAddr = devSlvId;
            smiOp[opIdx].regAddress    = addrArr[i];
            smiOp[opIdx].smiData       = dataArr[i];
        }

        retVal = extDrvHwIfOperExec(opIdx, &hwIf, NULL);
        if (GT_OK != retVal)
        {
            return retVal;
        }
    }

    return GT_OK;
#else
#ifdef GT_SMI
#if defined(DX_FAMILY)
    GT_STATUS   rc;
    GT_U32      i;

    for (i = 0; i < arrLen; i++)
    {
        rc = hwIfSmiIndirectWrite(devSlvId, 0, addrArr[i], dataArr[i]);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
    /* DX_FAMILY */
#else /* !DX_FAMILY */
    return GT_NOT_SUPPORTED;
#endif

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
#endif /* CPU_EMULATED_BUS */
}


/**
* @internal hwIfSmiRegVecRead function
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
static GT_STATUS hwIfSmiRegVecRead
(
    IN GT_U32       devSlvId,
    IN GT_U32       addrArr[],
    OUT GT_U32      dataArr[],
    IN GT_U32       arrLen
)
{
#ifdef CPU_EMULATED_BUS
    GT_STATUS       retVal;
    GT_SMI_OPER_STC smiOp[MAX_REG_CNT];
    GT_HW_IF_UNT    hwIf;
    GT_U32          i, j, opIdx;

    hwIf.smiOper = smiOp;
    i = 0;

    while (i < arrLen)
    {
        for (opIdx = 0; opIdx < MAX_REG_CNT && i < arrLen; i++, opIdx++)
        {
            smiOp[opIdx].opCode        = GT_READ_SMI_BUFFER_E;
            smiOp[opIdx].smiDeviceAddr = devSlvId;
            smiOp[opIdx].regAddress    = addrArr[i];
        }

        retVal = extDrvHwIfOperExec(opIdx, &hwIf, NULL);
        if (GT_OK != retVal)
        {
            return retVal;
        }

        for (opIdx--, j = i-1; ; opIdx--, j--)
        {
            dataArr[j] = smiOp[opIdx].smiData;

            if (0 == opIdx)
            {
                break;
            }
        }
    }

    return GT_OK;
#else
#ifdef GT_SMI
#if defined(DX_FAMILY)
    GT_STATUS   rc;
    GT_U32      i;

    for (i = 0; i < arrLen; i++)
    {
        rc = hwIfSmiIndirectRead(devSlvId, 0, addrArr[i], &dataArr[i]);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
    /* DX_FAMILY */
#else /* !DX_FAMILY */
    return GT_NOT_SUPPORTED;
#endif

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
#endif /* CPU_EMULATED_BUS */
}


/**
* @internal hwIfSmiRegRamWrite function
* @endinternal
*
* @brief   Writes a memory map (contiguous memory) using SMI from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addr                     - Register address to start write the reading.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*
* @note For SOHO PPs the regAddr consist of 2 parts:
*       - 16 MSB is SMI device ID for register
*       - 16 LSB is register address within SMI device ID for register
*
*/
static GT_STATUS hwIfSmiRegRamWrite
(
    IN GT_U32       devSlvId,
    IN GT_U32       addr,
    IN GT_U32       dataArr[],
    IN GT_U32       arrLen
)
{
#ifdef CPU_EMULATED_BUS
    GT_SMI_OPER_STC smiOp[MAX_REG_CNT];
    GT_HW_IF_UNT    hwIf;
    GT_STATUS       retVal;
    GT_U32          i, opIdx;

    hwIf.smiOper = smiOp;
    i = 0;

    while (i < arrLen)
    {
        for (opIdx = 0;
             opIdx < MAX_REG_CNT && i < arrLen;
             i++, opIdx++, addr += 4)
        {
            smiOp[opIdx].opCode        = GT_WRITE_SMI_BUFFER_E;
            smiOp[opIdx].smiDeviceAddr = devSlvId;
            smiOp[opIdx].regAddress    = addr;
            smiOp[opIdx].smiData       = dataArr[i];
        }

        retVal = extDrvHwIfOperExec(opIdx, &hwIf, NULL);
        if (GT_OK != retVal)
        {
            return retVal;
        }
    }

    return GT_OK;
#else
#ifdef GT_SMI
#if defined(DX_FAMILY)
    GT_STATUS   rc;
    GT_U32      i;

    for (i = 0; i < arrLen; i++, addr += 4)
    {
        rc = hwIfSmiIndirectWrite(devSlvId, 0, addr, dataArr[i]);

        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
    /* DX_FAMILY */
#else /* !DX_FAMILY */
    return GT_NOT_SUPPORTED;
#endif

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
#endif /* CPU_EMULATED_BUS */
}


/**
* @internal hwIfSmiRegRamRead function
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
*/
static GT_STATUS hwIfSmiRegRamRead
(
    IN GT_U32       devSlvId,
    IN GT_U32       addr,
    OUT GT_U32      dataArr[],
    IN GT_U32       arrLen
)
{
#ifdef CPU_EMULATED_BUS
    GT_STATUS       retVal;
    GT_SMI_OPER_STC smiOp[MAX_REG_CNT];
    GT_HW_IF_UNT    hwIf;
    GT_U32          i, j, opIdx;

    hwIf.smiOper = smiOp;
    i = 0;

    while (i < arrLen)
    {
        for (opIdx = 0;
             opIdx < MAX_REG_CNT && i < arrLen;
             i++, opIdx++, addr += 4)
        {
            smiOp[opIdx].opCode        = GT_READ_SMI_BUFFER_E;
            smiOp[opIdx].smiDeviceAddr = devSlvId;
            smiOp[opIdx].regAddress    = addr;
        }

        retVal = extDrvHwIfOperExec(opIdx, &hwIf, NULL);
        if (GT_OK != retVal)
        {
            return retVal;
        }

        for (opIdx--, j = i-1; ; opIdx--, j--)
        {
            dataArr[j] = smiOp[opIdx].smiData;

            if (0 == opIdx)
            {
                break;
            }
        }
    }

    return GT_OK;
#else
#ifdef GT_SMI
#if defined(DX_FAMILY)
    GT_STATUS   rc;
    GT_U32      i;

    for (i = 0; i < arrLen; i++, addr += 4)
    {
        rc = hwIfSmiIndirectRead(devSlvId, 0, addr, &dataArr[i]);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
    /* DX_FAMILY */
#else /* !DX_FAMILY */
    return GT_NOT_SUPPORTED;
#endif

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
#endif /* CPU_EMULATED_BUS */
}
#endif /* defined(GT_SMI) || defined(CPU_EMULATED_BUS) */


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
#ifdef GT_SMI
    GT_STATUS   retVal;

#ifdef DEBUG
    {
        GT_U32 i;

        for (i = 0; i < arrLen; i++)
        {
            if (checkaddr == addrArr[i])
            {
                logMsg("hwIfSmiTskRegVecWrite address 0x%08x, 0x%08x\n",
                       (int)addrArr[i], (int)dataArr[i],
                       0,0,0,0);
                taskSuspend(0);
            }

            if (logHwReadWriteFlag)
            {
                postIt(devSlvId, "W", addrArr[i], dataArr[i]);
            }
        }
    }
#endif /* DEBUG */

    if (NULL == smiLockSem)
    {
        /* create semaphore for SMI read/write routines */
        if(osSemBinCreate("smiLock", OS_SEMB_FULL, &smiLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if (osSemWait(smiLockSem, OS_WAIT_FOREVER) != GT_OK)
    {
        return GT_FAIL;
    }

    retVal = hwIfSmiRegVecWrite(devSlvId, addrArr, dataArr, arrLen);
    osSemSignal(smiLockSem);

    return retVal;
#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
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
#ifdef GT_SMI
    GT_STATUS   retVal;

    if (NULL == smiLockSem)
    {
        /* create semaphore for SMI read/write routines */
        if(osSemBinCreate("smiLock", OS_SEMB_FULL, &smiLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if (osSemWait(smiLockSem, OS_WAIT_FOREVER) != GT_OK)
    {
        return GT_FAIL;
    }
    retVal = hwIfSmiRegVecRead(devSlvId, addrArr, dataArr, arrLen);
    osSemSignal(smiLockSem);

#ifdef DEBUG
    {
        GT_U32 i;

        for (i = 0; i < arrLen; i++)
        {
            if (checkaddr == addrArr[i])
            {
                logMsg("hwIfSmiTskRegVecRead address 0x%08x, 0x%08x\n",
                       (int)addrArr[i], (int)dataArr[i],
                       0,0,0,0);
                taskSuspend(0);
            }

            if (logHwReadWriteFlag)
            {
                postIt(devSlvId, "R", addrArr[i], dataArr[i]);
            }
        }
    }
#endif /* DEBUG */
    return retVal;

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
}


/**
* @internal hwIfSmiTskRegRamWrite function
* @endinternal
*
* @brief   Writes a memory map (contiguous memory) using SMI from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addr                     - Register address to start write the reading.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
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
#ifdef GT_SMI
    GT_STATUS   retVal;

#ifdef DEBUG
    {
        GT_U32  i;
        GT_U32  tempAddr;

        tempAddr = addr;

        for (i = 0; i < arrLen; i++, tempAddr += 4)
        {
            if (checkaddr == tempAddr)
            {
                logMsg("hwIfSmiTskRegRamWrite address 0x%08x, 0x%08x\n",
                       (int)tempAddr, (int)dataArr[i],
                       0,0,0,0);
                taskSuspend(0);
            }

            if (logHwReadWriteFlag)
            {
                postIt(devSlvId, "W", tempAddr, dataArr[i]);
            }
        }
    }
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
#endif /* DEBUG */
    if (NULL == smiLockSem)
    {
        /* create semaphore for SMI read/write routines */
        if(osSemBinCreate("smiLock", OS_SEMB_FULL, &smiLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if (osSemWait(smiLockSem, OS_WAIT_FOREVER) != GT_OK)
    {
        return GT_FAIL;
    }

    retVal = hwIfSmiRegRamWrite(devSlvId, addr, dataArr, arrLen);
    osSemSignal(smiLockSem);

    return retVal;
#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
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
#ifdef GT_SMI
    GT_STATUS   retVal;

    if (NULL == smiLockSem)
    {
        /* create semaphore for SMI read/write routines */
        if(osSemBinCreate("smiLock", OS_SEMB_FULL, &smiLockSem) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    if (osSemWait(smiLockSem, OS_WAIT_FOREVER) != GT_OK)
    {
        return GT_FAIL;
    }

    retVal = hwIfSmiRegRamRead(devSlvId, addr, dataArr, arrLen);
    osSemSignal(smiLockSem);

#ifdef DEBUG
    {
        GT_U32 i;
        GT_U32  tempAddr;

        tempAddr = addr;

        for (i = 0; i < arrLen; i++, tempAddr += 4)
        {
            if (checkaddr == tempAddr)
            {
                logMsg("hwIfSmiTskRegRamRead address 0x%08x, 0x%08x\n",
                       (int)tempAddr, (int)dataArr[i],
                       0,0,0,0);
                taskSuspend(0);
            }

            if (logHwReadWriteFlag)
            {
                postIt(devSlvId, "R", tempAddr, dataArr[i]);
            }
        }
    }
#endif /* DEBUG */
    return retVal;

#else /* !GT_SMI */
    return GT_NOT_SUPPORTED;
#endif /* GT_SMI */
}


#ifdef GT_SMI
#ifdef DEBUG
#include <stdio.h>

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

GT_STATUS smidwr
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_STATUS rc;
    rc = bspSmiWriteReg(devSlvId,0,regAddr,value);
    printf("write Direct SMI (%08lX) address -> %08lX data -> %08lX \n",
           devSlvId,regAddr,value);

    return rc;
}


GT_STATUS smidrd
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr
)
{
    GT_U32 value = 0;
    GT_STATUS rc;
    rc = bspSmiReadReg(devSlvId,0,regAddr,&value);
    printf("read Direct SMI (%08lX) address -> %08lX data -> %08lX \n",
           devSlvId,regAddr,value);

    return rc;
}

#endif /* DEBUG */
#endif /* GT_SMI */



