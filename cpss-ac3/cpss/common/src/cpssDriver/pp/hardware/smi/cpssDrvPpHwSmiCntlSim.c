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
* @file cpssDrvPpHwSmiCntlSim.c
*
* @brief Prestera driver Hardware Simulation for SMI read and write functions
* implementation.
*
* @version   7
********************************************************************************
*/
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <asicSimulation/SCIB/scib.h>

extern GT_BOOL  prvCpssDrvTraceHwWrite[PRV_CPSS_MAX_PP_DEVICES_CNS];
extern GT_BOOL  prvCpssDrvTraceHwRead[PRV_CPSS_MAX_PP_DEVICES_CNS];

#ifdef HW_DEBUG
#define debugPrint(_x)      cpssOsPrintf _x
#else
#define debugPrint(_x)
#endif  /* VLAN_DEBUG */

/* in SMI -- no swap needed */
#define HW_BYTE_SWAP(devNum,portGroupId,data)   data
/*******************************************************************************
* Global Hw configuration params.
*******************************************************************************/
/*******************************************************************************
* Internal Macros and definitions
*******************************************************************************/
#define HWCTRL(_devNum,_portGroupId) \
    (prvCpssDrvPpConfig[_devNum]->hwCtrl[_portGroupId])
#define HWINFO(_devNum,_portGroupId) \
    (prvCpssDrvPpConfig[_devNum]->hwInfo[_portGroupId])
#define BASEADDR(_devNum,_portGroupId) (HWINFO(_devNum,_portGroupId).resource.switching.start)


/*******************************************************************************
* External usage environment parameters
*******************************************************************************/
/*******************************************************************************
* Forward functions declarations.
*******************************************************************************/

/**
* @internal driverHwSmiCntlInit function
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
*       Galtis:
*       None.
*
*/
static GT_STATUS driverHwSmiCntlInit
(
    IN GT_U8        devNum,
    IN GT_U32       portGroupId,
    IN GT_BOOL      isDiag
)
{
    GT_U32  simuDevId;

    (void)isDiag;

    simuDevId = scibGetDeviceId(BASEADDR(devNum,portGroupId));

    HWCTRL(devNum,portGroupId).compIdx            = (GT_U8)simuDevId;
    HWCTRL(devNum,portGroupId).lastWriteAddr[0]   = 0;
    HWCTRL(devNum,portGroupId).lastWriteAddr[1]   = 0;
    cpssExtDrvHwIfSmiInitDriver();

    return GT_OK;
}
/**
* @internal hwPpSmiReadRegister function
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
*/
static GT_STATUS hwPpSmiReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    GT_U32    temp;
    GT_STATUS retVal;

    debugPrint(("Read from dev %d, reg 0x%x,",devNum, regAddr));

    scibReadMemory( HWCTRL(devNum,portGroupId).compIdx,regAddr,1, &temp);

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
        if (retVal != GT_OK) 
        {
            return retVal;
        }
    }

    return GT_OK;
}




/**
* @internal hwPpSmiWriteRegister function
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
static GT_STATUS hwPpSmiWriteRegister
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_U32    temp;
    GT_STATUS retVal;

    debugPrint(("Write to device %d, reg 0x%x, data 0x%x.\n",devNum,regAddr,
                value));

    temp = HW_BYTE_SWAP(devNum, portGroupId,value);
    scibWriteMemory(HWCTRL(devNum,portGroupId).compIdx, regAddr, 1, &temp);

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
        if (retVal != GT_OK) 
        {
            return retVal;
        }
    }

    return GT_OK;
}

/**
* @internal hwPpSmiGetRegField function
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
static GT_STATUS hwPpSmiGetRegField
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

    retVal = hwPpSmiReadRegister(devNum,portGroupId, regAddr,&data);

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldData = (GT_U32)((data & mask) >> fieldOffset);

    debugPrint(("data 0x%x.\n", *fieldData));

    return retVal;
}


/**
* @internal hwPpSmiSetRegField function
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
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
static GT_STATUS hwPpSmiSetRegField
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
data 0x%x.\n",devNum, regAddr, fieldOffset, fieldLength, fieldData));


    retVal = hwPpSmiReadRegister( devNum, portGroupId,regAddr,&data);
    if (GT_OK != retVal)
    {
        return retVal;
    }
    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    data &= ~mask;   /* turn the field off */
    /* insert the new value of field in its place */
    data |= ((fieldData << fieldOffset) & mask);

    retVal = hwPpSmiWriteRegister( devNum, portGroupId,regAddr, data);

    return retVal;
}




/**
* @internal hwPpSmiReadRegBitMask function
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
static GT_STATUS hwPpSmiReadRegBitMask
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

    retVal = hwPpSmiReadRegister(devNum,portGroupId, regAddr,&data);
    *dataPtr = (data) & mask;

    debugPrint((" data is 0x%x.\n", *dataPtr));
    return retVal;


}

/**
* @internal hwPpSmiWriteRegBitMask function
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
static GT_STATUS hwPpSmiWriteRegBitMask
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
        retVal = hwPpSmiWriteRegister(devNum, portGroupId,regAddr, value);
        return retVal;
    }
    /* else read modify write */
    retVal = hwPpSmiReadRegister(devNum,portGroupId, regAddr, &data);
    if (GT_OK != retVal)
    {
        return retVal;
    }
    data &= (~mask);            /* Turn the field off.                        */
    data |= (value & mask);     /* Insert the new value of field in its place.*/

    retVal = hwPpSmiWriteRegister(devNum,portGroupId, regAddr, data);

    return retVal;
}


/**
* @internal hwPpSmiReadRam function
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
static GT_STATUS hwPpSmiReadRam
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    OUT GT_U32  *data
)
{
    GT_U32    i;
    GT_U32    temp;
    GT_U32    startAddr;
    GT_STATUS retVal;

    startAddr = addr;

    for (i = 0; i < length; i++)
    {
        scibReadMemory(HWCTRL(devNum,portGroupId).compIdx,
                                    addr, 1, &temp);

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
                                       startAddr, 
                                       length, /* length to trace the data in words */
                                       data);
        if (retVal != GT_OK) 
        {
            return retVal;
        }
    }

    return GT_OK;
}



/**
* @internal hwPpSmiWriteRam function
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
static GT_STATUS hwPpSmiWriteRam
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *data
)
{
    GT_U32    i;
    GT_U32    temp;
    GT_U32    startAddr;
    GT_STATUS retVal;

#ifdef HW_DEBUG
    for( i = 0; i < length; i++)
    {
        debugPrint(("\hwPpWriteRam on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addr + i * 4, data[i]));
    }
#endif

    startAddr = addr;
    for (i = 0; i < length; i++)
    {
        temp = HW_BYTE_SWAP(devNum, portGroupId,data[i]);
        scibWriteMemory(HWCTRL(devNum,portGroupId).compIdx,
                                     addr, 1, &temp);

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
                                        startAddr, 
                                        length, /* length to trace the data in words */
                                        data,
	                                    0xFFFFFFFF);
        if (retVal != GT_OK) 
        {
            return retVal;
        }
    }

    return GT_OK;
}


/**
* @internal hwPpSmiReadVec function
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
static GT_STATUS hwPpSmiReadVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
)
{
    GT_U32      i;
    GT_STATUS   retVal;


    for (i = 0; i < arrLen; i++)
    {
        scibReadMemory(HWCTRL(devNum,portGroupId).compIdx,
                       addrArr[i], 1, &dataArr[i]);

        dataArr[i] = HW_BYTE_SWAP(devNum, portGroupId,dataArr[i]);
    }

#ifdef HW_DEBUG
    for( i = 0; i < arrLen; i++)
    {
        debugPrint(("hwPpSmiReadVec on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addrArr[i], dataArr[i]));
    }
#endif

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE) 
    {
        for( i = 0; i < arrLen; i++)
        {
            /* trace HW read access */
            retVal = cpssTraceHwAccessRead(devNum, 
                                           portGroupId, 
                                           GT_FALSE, /* ISR context */
                                           CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                           addrArr[i], 
                                           1, /* length to trace the data in words */
                                           &dataArr[i]);
            if (retVal != GT_OK) 
            {
                return retVal;
            }
        }
    }

    return GT_OK;
}


/**
* @internal hwPpSmiWriteVec function
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
static GT_STATUS hwPpSmiWriteVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
)
{
    GT_U32    i;
    GT_STATUS retVal;

#ifdef HW_DEBUG
    for( i = 0; i < arrLen; i++)
    {
        debugPrint(("hwPpSmiWriteVec on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addrArr[i], dataArr[i]));
    }
#endif

    for (i = 0; i < arrLen; i++)
    {
        dataArr[i] = HW_BYTE_SWAP(devNum, portGroupId,dataArr[i]);
        scibWriteMemory(HWCTRL(devNum,portGroupId).compIdx,
                                     addrArr[i], 1, &dataArr[i]);
    }

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE) 
    {
        for( i = 0; i < arrLen; i++)
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
            if (retVal != GT_OK) 
            {
                return retVal;
            }
        }
    }

    return GT_OK;
}


/*******************************************************************************
* hwPpSmiIsrRead
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
static GT_INLINE GT_STATUS hwPpSmiIsrRead
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
)
{
    GT_U32    temp;
    GT_STATUS retVal;

    scibReadMemory(HWCTRL(devNum,portGroupId).compIdx,
                                     regAddr, 1, &temp);

    *dataPtr = HW_BYTE_SWAP(devNum, portGroupId,temp);

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE) 
    {
        /* trace HW read access */
        retVal = cpssTraceHwAccessRead(devNum, 
                                       portGroupId, 
                                       GT_TRUE,  /* ISR context */
                                       CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                       regAddr, 
                                       1, /* length to trace the data in words */
                                       dataPtr);
        if (retVal != GT_OK) 
        {
            return retVal;
        }
    }

    return GT_OK;
}


/**
* @internal hwPpSmiIsrWrite function
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
static GT_STATUS hwPpSmiIsrWrite
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_U32    temp;
    GT_STATUS retVal;

    debugPrint(("Write to device %d, reg 0x%x, data 0x%x.\n",devNum,regAddr,
                value));

    temp = HW_BYTE_SWAP(devNum, portGroupId,value);

    scibWriteMemory(HWCTRL(devNum,portGroupId).compIdx, regAddr, 1, &temp);

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE) 
    {
        /* trace HW write access */
        retVal = cpssTraceHwAccessWrite(devNum, 
                                        portGroupId, 
                                        GT_TRUE,  /* ISR context */
                                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* Address space */
                                        regAddr, 
                                        1, /* length to trace the data in words */
                                        &value,
	                                    0xFFFFFFFF);
        if (retVal != GT_OK) 
        {
            return retVal;
        }
    }

    return GT_OK;
}

/**
* @internal hwPpSmiReadInternalPciReg function
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
*/
static GT_STATUS hwPpSmiReadInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS retVal;

    /* here we go directlly to the "BUS" */
    scibReadMemory(HWCTRL(devNum,portGroupId).compIdx,
                              regAddr, 1, data);

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE) 
    {
        /* trace HW read access */
        retVal = cpssTraceHwAccessRead(devNum, 
                                       portGroupId, 
                                       GT_FALSE, /* ISR context */
                                       CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PCI_PEX_E,  /* Address space */
                                       regAddr, 
                                       1, /* length to trace the data in words */
                                       data);
        if (retVal != GT_OK) 
        {
            return retVal;
        }
    }

    return GT_OK;
}


/**
* @internal hwPpSmiWriteInternalPciReg function
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
static GT_STATUS hwPpSmiWriteInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_STATUS retVal;

    /* here we go directlly to the "BUS" */
    scibWriteMemory(HWCTRL(devNum,portGroupId).compIdx,
                               regAddr, 1, &data);

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE) 
    {
        /* trace HW write access */
        retVal = cpssTraceHwAccessWrite(devNum, 
                                        portGroupId, 
                                        GT_FALSE,  /* ISR context */
                                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PCI_PEX_E,   /* Address space */
                                        regAddr, 
                                        1, /* length to trace the data in words */
                                        &data,
	                                    0xFFFFFFFF);
        if (retVal != GT_OK) 
        {
            return retVal;
        }
    }

    return GT_OK;
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
    (void)devNum;
    (void)portGroupId;
    (void)addr;
    (void)length;
    (void)data;

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDrvHwSmiDriverObjectInit function
* @endinternal
*
* @brief   This function creates and initializes SMI device driver object
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if the driver object have been created before
*/
GT_STATUS prvCpssDrvHwSmiDriverObjectInit
(
     void
)
{
#ifdef INCL_EXDXMX_DRIVER
     /* driver object initialization */
    prvCpssDrvMngInfSmiPtr->drvHwCntlInit          = driverHwSmiCntlInit;
    prvCpssDrvMngInfSmiPtr->drvHwPpReadReg         = hwPpSmiReadRegister;
    prvCpssDrvMngInfSmiPtr->drvHwPpWriteReg        = hwPpSmiWriteRegister;
    prvCpssDrvMngInfSmiPtr->drvHwPpGetRegField     = hwPpSmiGetRegField;
    prvCpssDrvMngInfSmiPtr->drvHwPpSetRegField     = hwPpSmiSetRegField;
    prvCpssDrvMngInfSmiPtr->drvHwPpReadRegBitMask  = hwPpSmiReadRegBitMask;
    prvCpssDrvMngInfSmiPtr->drvHwPpWriteRegBitMask = hwPpSmiWriteRegBitMask;
    prvCpssDrvMngInfSmiPtr->drvHwPpReadRam         = hwPpSmiReadRam;
    prvCpssDrvMngInfSmiPtr->drvHwPpWriteRam        = hwPpSmiWriteRam;
    prvCpssDrvMngInfSmiPtr->drvHwPpReadVec         = hwPpSmiReadVec;
    prvCpssDrvMngInfSmiPtr->drvHwPpWriteVec        = hwPpSmiWriteVec;
    prvCpssDrvMngInfSmiPtr->drvHwPpWriteRamRev     = dummyWriteRamReverse;
    prvCpssDrvMngInfSmiPtr->drvHwPpIsrRead         = hwPpSmiIsrRead;
    prvCpssDrvMngInfSmiPtr->drvHwPpIsrWrite        = hwPpSmiIsrWrite;
    prvCpssDrvMngInfSmiPtr->drvHwPpReadIntPciReg   = hwPpSmiReadInternalPciReg;
    prvCpssDrvMngInfSmiPtr->drvHwPpWriteIntPciReg  = hwPpSmiWriteInternalPciReg;
#endif /* INCL_EXDXMX_DRIVER */
    return GT_OK;
}



