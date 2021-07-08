/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDrvPpHwTwsiCntl.c
*
* @brief Prestera driver Hardware read and write functions implementation.
*
* @version   9
********************************************************************************
*/
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>

extern GT_BOOL  prvCpssDrvTraceHwWrite[PRV_CPSS_MAX_PP_DEVICES_CNS];
extern GT_BOOL  prvCpssDrvTraceHwRead[PRV_CPSS_MAX_PP_DEVICES_CNS];

#ifdef HW_DEBUG
#define debugPrint(_x)      cpssOsPrintf _x
#else
#define debugPrint(_x)
#endif  /* VLAN_DEBUG */


/* in TWSI -- no swap needed */
#define HW_BYTE_SWAP(devNum,portGroupId,data)   data
/*******************************************************************************
* Global Hw configuration params.
*******************************************************************************/
#define HWCTRL(_devNum,_portGroupId) \
    (prvCpssDrvPpConfig[_devNum]->hwCtrl[_portGroupId])
#define HWINFO(_devNum,_portGroupId) \
    (prvCpssDrvPpConfig[_devNum]->hwInfo[_portGroupId])
#define BASEADDR(_devNum,_portGroupId) (HWINFO(_devNum,_portGroupId).resource.switching.start)


/*******************************************************************************
* Intenal Macros and definitions
*******************************************************************************/

/*******************************************************************************
* External usage environment parameters
*******************************************************************************/

/**
* @internal driverHwTwsiCntlInit function
* @endinternal
*
* @brief   This function initializes the Hw control structure of a given PP.
*
* @param[in] devNum                   - The PP's device number to init the structure for.
* @param[in] portGroupId              - The port group Id.
* @param[in] isDiag                   - Is this initialization is for diagnostics purposes
*                                      (GT_TRUE), or is it a final initialization of the Hw
*                                      Cntl unit (GT_FALSE)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note 1. In case isDiag == GT_TRUE, no semaphores are initialized.
*       2. All parameters besides devNum not relevant for TWSI init (only PCI),
*       as a result by default set to NULL.
*       Galtis:
*       None.
*
*/
static GT_STATUS driverHwTwsiCntlInit
(
    IN GT_U8        devNum,
    IN GT_U32       portGroupId,
    IN GT_BOOL      isDiag
)
{
    /* fix warnings */
    isDiag = isDiag;

    HWCTRL(devNum,portGroupId).compIdx            = 0;
    HWCTRL(devNum,portGroupId).lastWriteAddr[0]   = 0;
    HWCTRL(devNum,portGroupId).lastWriteAddr[1]   = 0;
    cpssExtDrvHwIfTwsiInitDriver();

    return GT_OK;
}
/**
* @internal hwPpTwsiReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note extDrv doesn't recognize the devNum,
*       therefore gets "BASEADDR(devNum,portGroupId)".
*
*/
static GT_STATUS hwPpTwsiReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS retVal;
    GT_U32    temp;


    debugPrint(("Read from dev %d, reg 0x%x,",devNum, regAddr));

    PRV_CPSS_INT_SCAN_LOCK();

    retVal = cpssExtDrvHwIfTwsiReadReg(BASEADDR(devNum,portGroupId),
                             regAddr, &temp);

    /* unlock */
    PRV_CPSS_INT_SCAN_UNLOCK();

    *data = HW_BYTE_SWAP(devNum, portGroupId,temp);
    debugPrint((" data 0x%x.\n", *data));

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE) 
    {
        /* trace HW read access */
        retVal = cpssTraceHwAccessRead(devNum, 
                                       portGroupId, 
                                       GT_FALSE, /* ISR context */
                                       CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                       regAddr, 
                                       1, /* length to trace the data in words */
                                       data);
    }

    return retVal;
}




/**
* @internal hwPpTwsiWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to write to.
*                                      data    - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTwsiWriteRegister
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_STATUS retVal;
    GT_U32    temp;

    debugPrint(("Write to device %d, reg 0x%x, data 0x%x.\n",devNum,regAddr,
                value));

    temp = HW_BYTE_SWAP(devNum, portGroupId,value);

    /* lock */
    PRV_CPSS_INT_SCAN_LOCK();

    retVal = cpssExtDrvHwIfTwsiWriteReg(BASEADDR(devNum,portGroupId),
                                 regAddr, temp);

    /* unlock */
    PRV_CPSS_INT_SCAN_UNLOCK();

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE) 
    {
        /* trace HW write access */
        retVal = cpssTraceHwAccessWrite(devNum, 
                                        portGroupId, 
                                        GT_FALSE, /* ISR context */
                                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                        regAddr, 
                                        1, /* length to trace the data in words */
                                        &value,
	                                    0xFFFFFFFF);
    }

    return retVal;
}

/**
* @internal hwPpTwsiGetRegField function
* @endinternal
*
* @brief   Read a selected register field.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTwsiGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)

{
    GT_STATUS retVal;
    GT_U32 data, mask;

    debugPrint(("GetField from dev %d, reg 0x%x, offset %d, length %d,",
                devNum, regAddr, fieldOffset, fieldLength));

    retVal = hwPpTwsiReadRegister(devNum, portGroupId,regAddr,&data);

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldData = (GT_U32)((data & mask) >> fieldOffset);

    debugPrint(("data 0x%x.\n", *fieldData));

    return retVal;
}



/**
* @internal hwPpTwsiSetRegField function
* @endinternal
*
* @brief   Write value to selected register field.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note this function actually reads the register value modifies the requested
*       field and writes the new value back to the HW.
*
*/
static GT_STATUS hwPpTwsiSetRegField
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData

)
{
    GT_STATUS retVal;
    GT_U32 data, mask;

    debugPrint(("SetField to dev %d, reg 0x%x, offset %d, length %d, \
                data 0x%x.\n",devNum, regAddr, fieldOffset,
                fieldLength, fieldData));

    retVal = hwPpTwsiReadRegister( devNum, portGroupId,regAddr,&data);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    data &= ~mask;   /* turn the field off */
    /* insert the new value of field in its place */
    data |= ((fieldData << fieldOffset) & mask);

    retVal = hwPpTwsiWriteRegister( devNum, portGroupId,regAddr, data);

    return retVal;

}




/**
* @internal hwPpTwsiReadRegBitMask function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @param[in] devNum                   - PP device number to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - Register address to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
static GT_STATUS hwPpTwsiReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
)
{
    GT_STATUS retVal;
    GT_U32 data;

    debugPrint(("\nhwPpReadRegBitMask on device %d, from register 0x%x, \
                mask 0x%x,", devNum, regAddr, mask));

    retVal = hwPpTwsiReadRegister(devNum, portGroupId,regAddr,&data);

    *dataPtr = (data) & mask;

    debugPrint((" data is 0x%x.\n", *dataPtr));
    return retVal;


}

/**
* @internal hwPpTwsiWriteRegBitMask function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - Register address to write to.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
static GT_STATUS hwPpTwsiWriteRegBitMask
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 mask,
    IN GT_U32 value
)
{
    GT_STATUS retVal;
    GT_U32 data;


    debugPrint(("\nhwPpWriteRegBitMask on device %d, from register 0x%x, \
                mask 0x%x, data is 0x%x.\n",
                devNum, regAddr, mask, value));



    /* in case mask = 0xffffffff call hwWrite only */
    if(mask == 0xffffffff)
    {
        retVal = hwPpTwsiWriteRegister(devNum, portGroupId,regAddr, value);

        return retVal;
    }
    /* else read modify write */
    retVal = hwPpTwsiReadRegister(devNum, portGroupId,regAddr, &data);
    if (GT_OK != retVal)
    {
        return retVal;
    }
    data &= (~mask);            /* Turn the field off.                        */
    data |= (value & mask);     /* Insert the new value of field in its place.*/

    retVal = hwPpTwsiWriteRegister(devNum, portGroupId,regAddr, data);

   return retVal;

}


/**
* @internal hwPpTwsiReadRam function
* @endinternal
*
* @brief   Read from PP's RAM.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] addr                     - Address offset to read from.
* @param[in] length                   - Number of Words (4 byte) to read.
*
* @param[out] data                     - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTwsiReadRam
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    OUT GT_U32  *data
)
{
    GT_STATUS retVal;
    GT_U32    i;
    GT_U32    temp;
    GT_U32    baseAddress;


    baseAddress = BASEADDR(devNum,portGroupId);

    for (i = 0; i < length; i++)
    {
        retVal = cpssExtDrvHwIfTwsiReadReg(baseAddress, addr, &temp);

        if (GT_OK != retVal)
        {
            return retVal;
        }
        data[i] = HW_BYTE_SWAP(devNum, portGroupId,temp);
        addr += 4;
    }

#ifdef HW_DEBUG
    for( i = 0; i < length; i++)
    {
        debugPrint(("\hwPpReadRam on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addr + 4 * i, data[i]));
    }
#endif

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE) 
    {
        /* trace HW read access */
        retVal = cpssTraceHwAccessRead(devNum, 
                                       portGroupId, 
                                       GT_FALSE, /* ISR context */
                                       CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                       addr, 
                                       length, /* length to trace the data in words */
                                       data);
        if (GT_OK != retVal)
        {
            return retVal;
        }
    }

    return GT_OK;
}



/**
* @internal hwPpTwsiWriteRam function
* @endinternal
*
* @brief   Writes to PP's RAM.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTwsiWriteRam
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *data
)
{
    GT_STATUS retVal;
    GT_U32    i;
    GT_U32    temp;
    GT_U32    baseAddress;

#ifdef HW_DEBUG
    for( i = 0; i < length; i++)
    {
        debugPrint(("\hwPpWriteRam on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addr + i * 4, data[i]));
    }
#endif

    baseAddress = BASEADDR(devNum,portGroupId);
    for (i = 0; i < length; i++)
    {
        temp = HW_BYTE_SWAP(devNum,portGroupId,data[i]);

        retVal = cpssExtDrvHwIfTwsiWriteReg(baseAddress, addr, temp);

        if (GT_OK != retVal)
        {
            return retVal;
        }
        addr += 4;
    }

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE) 
    {
        /* trace HW write access */
        retVal = cpssTraceHwAccessWrite(devNum, 
                                        portGroupId, 
                                        GT_FALSE, /* ISR context */
                                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                        addr, 
                                        length,   /* length to trace the data in words */
                                        data,
	                                    0xFFFFFFFF);
    }

    return GT_OK;
}


/**
* @internal hwPpTwsiReadVec function
* @endinternal
*
* @brief   Read from PP's RAM a vector of addresses.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] addrArr[]                - Address array to read from.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @param[out] dataArr[]                - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTwsiReadVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
)
{
    GT_STATUS   retVal;
    GT_U32      i;
    GT_U32      baseAddress;


    baseAddress = BASEADDR(devNum,portGroupId);

    for (i = 0; i < arrLen; i++)
    {
        retVal = cpssExtDrvHwIfTwsiReadReg(baseAddress, addrArr[i], &dataArr[i]);

        if (GT_OK != retVal)
        {
            return retVal;
        }
        dataArr[i] = HW_BYTE_SWAP(devNum, portGroupId,dataArr[i]);
    }

#ifdef HW_DEBUG
    for( i = 0; i < arrLen; i++)
    {
        debugPrint(("hwPpTwsiReadVec on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addrArr[i], dataArr[i]));
    }
#endif

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE) 
    {
        for (i = 0; i < arrLen; i++)
        {
            /* trace HW read access */
            retVal = cpssTraceHwAccessRead(devNum, 
                                           portGroupId, 
                                           GT_FALSE, /* ISR context */
                                           CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                           addrArr[i], 
                                           1, /* length to trace the data in words */
                                           &dataArr[i]);
            if (GT_OK != retVal)
            {
                return retVal;
            }
        }

    }

    return GT_OK;
}


/**
* @internal hwPpTwsiWriteVec function
* @endinternal
*
* @brief   Writes to PP's RAM a vector of addresses.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] addrArr[]                - Address offset to write to.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTwsiWriteVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
)
{
    GT_STATUS   retVal;
    GT_U32      i;
    GT_U32      baseAddress;

#ifdef HW_DEBUG
    for( i = 0; i < arrLen; i++)
    {
        debugPrint(("hwPpTwsiWriteVec on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addrArr[i], dataArr[i]));
    }
#endif

    baseAddress = BASEADDR(devNum,portGroupId);
    for (i = 0; i < arrLen; i++)
    {
        dataArr[i] = HW_BYTE_SWAP(devNum,portGroupId,dataArr[i]);

        retVal = cpssExtDrvHwIfTwsiWriteReg(baseAddress, addrArr[i], dataArr[i]);

        if (GT_OK != retVal)
        {
            return retVal;
        }
    }
    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE) 
    {
        for (i = 0; i < arrLen; i++)
        {
            /* trace HW write access */
            retVal = cpssTraceHwAccessWrite(devNum, 
                                            portGroupId, 
                                            GT_FALSE, /* ISR context */
                                            CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                            addrArr[i], 
                                            1, /* length to trace the data in words */
                                            &dataArr[i],
		                                    0xFFFFFFFF);
            if (GT_OK != retVal)
            {
                return retVal;
            }
        }

    }

    return GT_OK;
}


/*******************************************************************************
* hwPpTwsiIsrRead
*
* DESCRIPTION:
*       Read a register value using special interrupt address completion region.
*
* INPUTS:
*       devNum  - The PP to read from.
*       portGroupId  - The port group Id.
*       regAddr - The register's address to read from.
*                 Note: regAddr should be < 0x1000000
*
* OUTPUTS:
*       dataPtr - Includes the register value.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_INLINE GT_STATUS hwPpTwsiIsrRead
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
)
{
    GT_STATUS retVal;
    GT_U32    temp;


    retVal = cpssExtDrvHwIfTwsiReadReg(BASEADDR(devNum,portGroupId),
                                     regAddr, &temp);

    *dataPtr = HW_BYTE_SWAP(devNum, portGroupId,temp);

    retVal = GT_OK;

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE) 
    {
        /* trace HW read access */
        retVal = cpssTraceHwAccessRead(devNum, 
                                       portGroupId, 
                                       GT_TRUE, /* ISR context */
                                       CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                       regAddr, 
                                       1, /* length to trace the data in words */
                                       dataPtr);
    }

    return retVal;
}


/**
* @internal hwPpTwsiIsrWrite function
* @endinternal
*
* @brief   Write a register value using special interrupt address completion region
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to write to.
*                                      data    - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTwsiIsrWrite
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_STATUS retVal;
    GT_U32    temp;

    debugPrint(("Write to device %d, reg 0x%x, data 0x%x.\n",devNum,regAddr,
                value));

    temp = HW_BYTE_SWAP(devNum, portGroupId,value);

    retVal = cpssExtDrvHwIfTwsiWriteReg(BASEADDR(devNum,portGroupId),
                                      regAddr, temp);

    retVal = GT_OK;

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE) 
    {
        /* trace HW write access */
        retVal = cpssTraceHwAccessWrite(devNum, 
                                       portGroupId, 
                                       GT_TRUE, /* ISR context */
                                       CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                       regAddr, 
                                       1, /* length to trace the data in words */
                                       &value,
                                       0xFFFFFFFF);
    }
    return retVal;
}

/**
* @internal hwPpTwsiReadInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note "inheritance" from PCI handling.
*
*/
static GT_STATUS hwPpTwsiReadInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    return  hwPpTwsiReadRegister(devNum, portGroupId,regAddr, data);
}


/**
* @internal hwPpTwsiWriteInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpTwsiWriteInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
   return  hwPpTwsiWriteRegister(devNum,portGroupId,regAddr,data);
}

/**
* @internal dummyWriteRamReverse function
* @endinternal
*
* @brief   Writes to PP's RAM in reverse.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_NOT_SUPPORTED         - always
*/
static GT_STATUS dummyWriteRamReverse
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *data
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    addr = addr;
    length = length;
    data = data;

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDrvHwTwsiDriverObjectInit function
* @endinternal
*
* @brief   This function creates and initializes Twsi device driver object
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if the driver object have been created before
*/
GT_STATUS prvCpssDrvHwTwsiDriverObjectInit
(
     void
)
{
#ifdef INCL_EXDXMX_DRIVER
     /* driver object initialization */
    prvCpssDrvMngInfTwsiPtr->drvHwCntlInit          = driverHwTwsiCntlInit;
    prvCpssDrvMngInfTwsiPtr->drvHwPpReadReg         = hwPpTwsiReadRegister;
    prvCpssDrvMngInfTwsiPtr->drvHwPpWriteReg        = hwPpTwsiWriteRegister;
    prvCpssDrvMngInfTwsiPtr->drvHwPpGetRegField     = hwPpTwsiGetRegField;
    prvCpssDrvMngInfTwsiPtr->drvHwPpSetRegField     = hwPpTwsiSetRegField;
    prvCpssDrvMngInfTwsiPtr->drvHwPpReadRegBitMask  = hwPpTwsiReadRegBitMask;
    prvCpssDrvMngInfTwsiPtr->drvHwPpWriteRegBitMask = hwPpTwsiWriteRegBitMask;
    prvCpssDrvMngInfTwsiPtr->drvHwPpReadRam         = hwPpTwsiReadRam;
    prvCpssDrvMngInfTwsiPtr->drvHwPpWriteRam        = hwPpTwsiWriteRam;
    prvCpssDrvMngInfTwsiPtr->drvHwPpReadVec         = hwPpTwsiReadVec;
    prvCpssDrvMngInfTwsiPtr->drvHwPpWriteVec        = hwPpTwsiWriteVec;
    prvCpssDrvMngInfTwsiPtr->drvHwPpWriteRamRev     = dummyWriteRamReverse;
    prvCpssDrvMngInfTwsiPtr->drvHwPpIsrRead         = hwPpTwsiIsrRead;
    prvCpssDrvMngInfTwsiPtr->drvHwPpIsrWrite        = hwPpTwsiIsrWrite;
    prvCpssDrvMngInfTwsiPtr->drvHwPpReadIntPciReg   = hwPpTwsiReadInternalPciReg;
    prvCpssDrvMngInfTwsiPtr->drvHwPpWriteIntPciReg  = hwPpTwsiWriteInternalPciReg;
#endif /* INCL_EXDXMX_DRIVER */

    return GT_OK;
}




