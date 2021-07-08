/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* gtHwCntlStandBy.c
*
* DESCRIPTION:
*       Prestera driver Hardware read and write functions implementation for Stand
*       By mode of work of CPU in context of High Availability feature.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>

/*******************************************************************************
* Global Hw configuration params.
*******************************************************************************/


/*******************************************************************************
* Intenal Macros and definitions
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


/*******************************************************************************
* Forward declarations for driver object bound functions
*******************************************************************************/

/**
* @internal driverHwPciCntlInit function
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
static GT_STATUS driverHwPciCntlInit
(
    IN GT_U8        devNum,
    IN GT_U32       portGroupId,
    IN GT_BOOL      isDiag
)
{
    GT_U8   i;
    char   name[20];
    GT_UINTPTR  address;     /* Physical address for PCI transaction */

    HWCTRL(devNum,portGroupId).compIdx            = 2;
    HWCTRL(devNum,portGroupId).lastWriteAddr[0]   = 0;
    HWCTRL(devNum,portGroupId).lastWriteAddr[1]   = 0;

    /* Reset Hw Address Completion          */
    address = BASEADDR(devNum,portGroupId) + ADDR_COMP_REG;
    *(volatile GT_U32 *)(address) = 0;
    *(volatile GT_U32 *)(address) = 0;
    *(volatile GT_U32 *)(address) = 0;
    GT_SYNC;                                                      \

    /* Update Address Completion shadow     */
    for (i = 0; i < 4; i++)
    {
        HWCTRL(devNum,portGroupId).addrCompletShadow[i] = 0;
    }

    if ((isDiag == GT_FALSE) && (HWCTRL(devNum,portGroupId).hwComplSem == (CPSS_OS_MUTEX)0))
    {
        cpssOsSprintf (name,"hwComplSem-%d_%ld",devNum,portGroupId);
        cpssOsMutexCreate (name,&(HWCTRL(devNum,portGroupId).hwComplSem));
    }

    return GT_OK;
}



/**
* @internal hwPpPciReadRegister function
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
static GT_STATUS hwPpPciReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;

    *data = 0;
    return GT_OK;
}




/**
* @internal hwPpPciWriteRegister function
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
static GT_STATUS hwPpPciWriteRegister
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;
    value = value;

    return GT_OK;
}


/**
* @internal hwPpPciGetRegField function
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
static GT_STATUS hwPpPciGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;
    fieldOffset = fieldOffset;
    fieldLength = fieldLength;

    *fieldData = 0;
    return GT_OK;
}


/**
* @internal hwPpPciSetRegField function
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
static GT_STATUS hwPpPciSetRegField
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;
    fieldOffset = fieldOffset;
    fieldLength = fieldLength;
    fieldData = fieldData;

    return GT_OK;
}


/**
* @internal hwPpPciReadRegBitMask function
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
static GT_STATUS hwPpPciReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;
    mask = mask;

    *dataPtr = 0;
    return GT_OK;
}




/**
* @internal hwPpPciWriteRegBitMask function
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
static GT_STATUS hwPpPciWriteRegBitMask
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 mask,
    IN GT_U32 value
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;
    mask = mask;
    value = value;

    return GT_OK;
}


/**
* @internal hwPpPciReadRam function
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
static GT_STATUS hwPpPciReadRam
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    OUT GT_U32  *data
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    addr = addr;
    length = length;

    *data = 0;
    return GT_OK;
}



/**
* @internal hwPpPciWriteRam function
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
static GT_STATUS hwPpPciWriteRam
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

    return GT_OK;
}


/**
* @internal hwPpPciWriteRamInReverse function
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
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPciWriteRamInReverse
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

    return GT_OK;
}

/**
* @internal hwPpPciIsrRead function
* @endinternal
*
* @brief   Read a register value using special interrupt address completion region.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
*                                      Note: regAddr should be < 0x1000000
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPciIsrRead
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;
    dataPtr = dataPtr;

    return GT_OK;
}
/**
* @internal hwPpPciIsrWrite function
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
static GT_STATUS hwPpPciIsrWrite
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;
    value = value;

    return GT_OK;
}
/**
* @internal hwPpPciReadInternalPciReg function
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
static GT_STATUS hwPpPciReadInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;

    *data = 0;
    return GT_OK;
}


/**
* @internal hwPpPciWriteInternalPciReg function
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
static GT_STATUS hwPpPciWriteInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    /* fix warnings */
    devNum = devNum;
    portGroupId = portGroupId;
    regAddr = regAddr;
    data = data;

    return GT_OK;
}


/**
* @internal hwPpPciResetAndInitCtrlReadRegister function
* @endinternal
*
* @brief   This function reads a register from the Reset and Init Controller.
*
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPciResetAndInitCtrlReadRegister
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    /* fix warnings */
    devNum = devNum;
    regAddr = regAddr;

    *data = 0;

    return GT_OK;
}


/**
* @internal hwPpPciResetAndInitCtrlWriteRegister function
* @endinternal
*
* @brief   This function writes a register to the Reset and Init Controller.
*
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to wrire to.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPciResetAndInitCtrlWriteRegister
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    /* fix warnings */
    devNum = devNum;
    regAddr = regAddr;
    data = data;

    return GT_OK;
}


/**
* @internal hwPpPciResetAndInitCtrlGetRegField function
* @endinternal
*
* @brief   This function reads a selected register field from the Reset and Init
*         Controller.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - the read field data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPciResetAndInitCtrlGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    /* fix warnings */
    devNum = devNum;
    regAddr = regAddr;
    fieldOffset = fieldOffset;
    fieldLength = fieldLength;

    *fieldData = 0;

    return GT_OK;
}


/**
* @internal hwPpPciResetAndInitCtrlSetRegField function
* @endinternal
*
* @brief   This function writes a selected register field to the Reset and Init
*         Controller.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPciResetAndInitCtrlSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
    /* fix warnings */
    devNum = devNum;
    regAddr = regAddr;
    fieldOffset = fieldOffset;
    fieldLength = fieldLength;
    fieldData = fieldData;

    return GT_OK;
}


/**
* @internal prvCpssDrvHwPciStandByDriverObjectInit function
* @endinternal
*
* @brief   This function creates and initializes PCI device driver object for
*         a standby CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if the driver object have been created before
*/
GT_STATUS prvCpssDrvHwPciStandByDriverObjectInit
(
     void
)
{
     /* driver object initialization */
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwCntlInit          = driverHwPciCntlInit;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpReadReg         = hwPpPciReadRegister;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpWriteReg        = hwPpPciWriteRegister;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpGetRegField     = hwPpPciGetRegField;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpSetRegField     = hwPpPciSetRegField;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpReadRegBitMask  = hwPpPciReadRegBitMask;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpWriteRegBitMask = hwPpPciWriteRegBitMask;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpReadRam         = hwPpPciReadRam;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpWriteRam        = hwPpPciWriteRam;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpWriteRamRev     = hwPpPciWriteRamInReverse;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpIsrRead         = hwPpPciIsrRead;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpIsrWrite        = hwPpPciIsrWrite;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpReadIntPciReg   = hwPpPciReadInternalPciReg;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpWriteIntPciReg  = hwPpPciWriteInternalPciReg;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpResetAndInitControllerReadReg = 
                                           hwPpPciResetAndInitCtrlReadRegister;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpResetAndInitControllerWriteReg = 
                                           hwPpPciResetAndInitCtrlWriteRegister;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpResetAndInitControllerGetRegField = 
                                           hwPpPciResetAndInitCtrlGetRegField;
    prvCpssDrvMngInfPciHaStandbyPtr->drvHwPpResetAndInitControllerSetRegField = 
                                           hwPpPciResetAndInitCtrlSetRegField;

    return GT_OK;
}



/********************************************************************************
* !!!!!!!!!!!!!!!!!!!!! FOR DEBUG PURPOSES ONLY !!!!!!!!!!!!!!!!!!!!!!!!!!
********************************************************************************/


