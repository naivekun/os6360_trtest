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
* @file prvCpssDrvHwCntl.h
*
* @brief CPSS Prestera driver structs definitions and functions declarations for
* hardware access.
*
* @version   10
********************************************************************************
*/

#ifndef __prvCpssDrvHwCntlh
#define __prvCpssDrvHwCntlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssHwInfo.h>
#include <cpss/extServices/os/gtOs/cpssOsSem.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>

/* offset of the address completion from the PCI base address*/
#define ADDR_COMP_REG 0

/* offset of the address completion for PEX 8 completion regions */
#define PEX_MBUS_ADDR_COMP_REG_MAC(_index) (0x120 + (4 * _index))

/* address of the device Id and the revision id register address */
#define PRV_CPSS_DEVICE_ID_AND_REV_REG_ADDR_CNS    0x4c

/**
* @struct PRV_CPSS_DRV_RAM_BURST_INFO_STC
 *
 * @brief Holds information regarding the bursts that should be performed
 * when access the different Pp's Rams.
*/
typedef struct{

    /** Base address of the Ram. */
    GT_U32 ramBase;

    /** @brief Mask to be used to check if a given address falls into
     *  this Ram.
     */
    GT_U32 ramBaseMask;

    /** Number of words to be written in each burst. */
    GT_U8 burstSize;

    /** @brief Resolution of the addresses to be used when addressing
     *  this Ram (see comment).
     *  Comment:
     *  1. addrResolution: This is the parameter by which the Ram offset is
     *  multiplied when accessing it by burst, if this
     *  is different than 1, then a block write / read from
     *  Ram should be divided into several bursts in size
     *  'burstSize'.
     */
    GT_U8 addrResolution;

} PRV_CPSS_DRV_RAM_BURST_INFO_STC;


/* Maximal size of a memory block to be read / written in   */
/* burst (in 4-bytes words).                                */
#define PRV_CPSS_DRV_MAX_BURST_BLOCK_SIZE_CNS    256
#define PRV_CPSS_DRV_MAX_BURST_EXTRA_ALLIGN_CNS  16

#ifdef ASIC_SIMULATION
/**
* @struct PRV_SIM_DRV_STRUCT_PP_HW_STC
 *
 * @brief Simulation related data - For performing Read/Write accesses to PP's .
*/
typedef struct{

    /** simulation device ID */
    GT_U8 devId;

    /** @brief enable/disable address completion
     *  for specific device.
     *  Comments:
     */
    GT_BOOL addrCompletEnable;

} PRV_SIM_DRV_STRUCT_PP_HW_STC;

#endif /* ASIC_SIMULATION */

/**
* @struct PRV_CPSS_DRV_ADDR_COMPL_REGION_INFO_STC
 *
 * @brief Holds per device information, required for HW access.
*/
typedef struct{

    /** 6 MSB address bits to be used */
    GT_U32 value;

    /** @brief register address(out of 3 existing registers in HW.
     *  Only second register holding region 5, and third one, holding regions 6
     *  and 7 are relevant
     *  Comments:
     */
    GT_U32 regAddr;

} PRV_CPSS_DRV_ADDR_COMPL_REGION_INFO_STC;

/**
* @struct PRV_CPSS_DRV_PEX_ADDR_COMPL_LRU_STC
 *
 * @brief Hardware access related data - For performing Read/Write accesses to PP's .
*/
typedef struct{

    /** Permutations array's current row index */
    GT_U32 curPermArrRowIndex;

    /** Number of dynamic regions out of 8 available */
    GT_U32 numOfDynamicRegions;

    PRV_CPSS_DRV_ADDR_COMPL_REGION_INFO_STC regionInfo[9];

    GT_U32 *permArrayPtr;

} PRV_CPSS_DRV_PEX_ADDR_COMPL_LRU_STC;

/**
* @struct PRV_CPSS_DRV_STRUCT_PP_PORT_GROUP_HW_STC
 *
 * @brief Hardware access related data - For performing Read/Write accesses to PP's .
 * This info is 'per port-group'
*/
typedef struct
{
    GT_U32              addrCompletShadow[8];
    GT_U32              isrAddrCompletionRegionsBmp;
    GT_U32              isrAddrRegionUsed;
    GT_U32              isrAddrRegionMin;
    GT_U32              isrAddrRegionMax;
    GT_U32              appAddrCompletionRegionsBmp;
    GT_U32              appAddrRegionUsed;
    GT_U32              appAddrRegionMin;
    GT_U32              appAddrRegionMax;
    CPSS_OS_MUTEX       hwComplSem;
    GT_U8               compIdx;
    PRV_CPSS_DRV_RAM_BURST_INFO_STC   *ramBurstInfo;
    GT_U8               ramBurstInfoLen;
    GT_U32              lastWriteAddr[2];

    GT_BOOL             doReadAfterWrite;
    GT_U32              dummyReadAfterWriteRegAddr;

#ifdef ASIC_SIMULATION
    PRV_SIM_DRV_STRUCT_PP_HW_STC simInfo;
#endif /* ASIC_SIMULATION */

} PRV_CPSS_DRV_STRUCT_PP_PORT_GROUP_HW_STC;



/**
* @internal prvCpssDrvHwCntlInit function
* @endinternal
*
* @brief   This function initializes the Hw control structure of a given PP.
*
* @param[in] devNum                   - The PP's device number to init the structure for.
* @param[in] portGroupId              - The port group id.
*                                      relevant only to 'multi-port-group' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      baseAddr        - The PP base address on the host interface bus.
*                                      internalPciBase - Base address to which the internal pci registers
*                                      are mapped to.
* @param[in] isDiag                   - Is this initialization is for diagnostics purposes
*                                      (GT_TRUE), or is it a final initialization of the Hw
*                                      Cntl unit (GT_FALSE)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 1. In case isDiag == GT_TRUE, no semaphores are initialized.
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwCntlInit
(
    IN GT_U8        devNum,
    IN GT_U32       portGroupId,
    IN CPSS_HW_INFO_STC *hwInfo,
    IN GT_BOOL      isDiag
);

/**
* @internal prvCpssDrvHwPpReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
);



/**
* @internal prvCpssDrvHwPpWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] regAddr                  - The register's address to write to.
*                                      data    - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpWriteRegister
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 value
);



/**
* @internal prvCpssDrvHwPpGetRegField function
* @endinternal
*
* @brief   Read a selected register field.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
);



/**
* @internal prvCpssDrvHwPpSetRegField function
* @endinternal
*
* @brief   Write value to selected register field.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpSetRegField
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData
);



/**
* @internal prvCpssDrvHwPpReadRegBitMask function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @param[in] devNum                   - PP device number to read from.
* @param[in] regAddr                  - Register address to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*       GalTis:
*       Command - hwPpReadRegBitMask
*
*/
GT_STATUS prvCpssDrvHwPpReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
);



/**
* @internal prvCpssDrvHwPpWriteRegBitMask function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] regAddr                  - Register address to write to.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*       GalTis:
*       Command - hwPpWriteRegBitMask
*
*/
GT_STATUS prvCpssDrvHwPpWriteRegBitMask
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 mask,
    IN GT_U32 value
);



/**
* @internal prvCpssDrvHwPpReadRam function
* @endinternal
*
* @brief   Read from PP's RAM.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] addr                     - Address offset to read from.
* @param[in] length                   - Number of Words (4 byte) to read.
*
* @param[out] data                     - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       GalTis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpReadRam
(
    IN GT_U8 devNum,
    IN GT_U32 addr,
    IN GT_U32 length,
    OUT GT_U32  *data
);


/**
* @internal prvCpssDrvHwPpWriteRamInReverse function
* @endinternal
*
* @brief   Writes to PP's RAM in reverse.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpWriteRamInReverse
(
    IN GT_U8 devNum,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *data
);

/**
* @internal prvCpssDrvHwPpWriteRam function
* @endinternal
*
* @brief   Writes to PP's RAM.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       GalTis:
*       Table - PPWrite
*
*/
GT_STATUS prvCpssDrvHwPpWriteRam
(
    IN GT_U8 devNum,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *data
);


/**
* @internal prvCpssDrvHwPpReadVec function
* @endinternal
*
* @brief   Read from PP's RAM a vector of addresses.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] addrArr[]                - Address array to read from.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @param[out] dataArr[]                - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpReadVec
(
    IN GT_U8    devNum,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
);

/**
* @internal prvCpssDrvHwPpWriteVec function
* @endinternal
*
* @brief   Writes to PP's RAM a vector of addresses.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] addrArr[]                - Address offset to write to.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpWriteVec
(
    IN GT_U8    devNum,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
);

/*******************************************************************************
* prvCpssDrvHwPpIsrRead
*
* DESCRIPTION:
*       Read a register value using special interrupt address completion region.
*
* INPUTS:
*       devNum  - The PP to read from.
*       regAddr - The register's address to read from.
*                 Note - regAddr should be < 0x1000000
*
* OUTPUTS:
*       dataPtr - Includes the register value.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on hardware error
*       GT_NOT_INITIALIZED - if the driver was not initialized
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
* GalTis:
*       None.
*
*******************************************************************************/
GT_INLINE GT_STATUS prvCpssDrvHwPpIsrRead
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
);



/**
* @internal prvCpssDrvHwPpIsrWrite function
* @endinternal
*
* @brief   Write a register value using special interrupt address completion region
*
* @param[in] devNum                   - The PP to write to.
* @param[in] regAddr                  - The register's address to write to.
*                                      data    - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 
*       GalTis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpIsrWrite
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 value
);

/**
* @internal prvCpssDrvHwPpReadInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpReadInternalPciReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);


/**
* @internal prvCpssDrvHwPpWriteInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpWriteInternalPciReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal prvCpssDrvHwPpResetAndInitControllerReadReg function
* @endinternal
*
* @brief   Read a register value from the Reset and Init Controller.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpResetAndInitControllerReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);

/**
* @internal prvCpssDrvHwPpResetAndInitControllerGetRegField function
* @endinternal
*
* @brief   Read a selected register field from the Reset and Init Controller.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpResetAndInitControllerGetRegField
 (
    IN  GT_U8   devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
);

/**
* @internal prvCpssDrvHwPpResetAndInitControllerWriteReg function
* @endinternal
*
* @brief   Write to the Reset and Init Controller given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpResetAndInitControllerWriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal prvCpssDrvHwPpResetAndInitControllerSetRegField function
* @endinternal
*
* @brief   Write value to selected register field of the Reset and Init Controller.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS prvCpssDrvHwPpResetAndInitControllerSetRegField
 (
    IN  GT_U8   devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
);

/**
* @internal prvCpssDrvHwPpMgReadReg function
* @endinternal
*
* @brief   Read a register value from the management unit.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] mgNum                    - The management unit to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMgReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);

/**
* @internal prvCpssDrvHwPpMgWriteReg function
* @endinternal
*
* @brief   Write to the management unit given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] mgNum                    - The management unit to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMgWriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal prvCpssDrvHwPpMgGetRegField function
* @endinternal
*
* @brief   Read a selected register field from the Management Unit.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] mgNum                    - The management unit number.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMgGetRegField
 (
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    OUT GT_U32  *fieldData
);

/**
* @internal prvCpssDrvHwPpMgSetRegField function
* @endinternal
*
* @brief   Write value to selected register field of the Management Unit.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] mgNum                    - The management unit number.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS prvCpssDrvHwPpMgSetRegField
 (
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    IN  GT_U32  fieldData
);

/**
* @internal prvCpssDrvHwPpResourceReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - (pointer to) memory for the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDrvHwPpResourceReadRegister
(
    IN GT_U8                          devNum,
    IN GT_U32                         portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT  resourceType,
    IN GT_U32                         regAddr,
    OUT GT_U32                        *dataPtr
);

/**
* @internal prvCpssDrvHwPpResourceWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] value                    - The  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDrvHwPpResourceWriteRegister
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           value
);

/**
* @internal prvCpssDrvHwPpResourceGetRegField function
* @endinternal
*
* @brief   Read a selected register field. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldDataPtr             - (pointer to) Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDrvHwPpResourceGetRegField
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           fieldOffset,
    IN GT_U32                           fieldLength,
    OUT GT_U32                          *fieldDataPtr
);

/**
* @internal prvCpssDrvHwPpResourceSetRegField function
* @endinternal
*
* @brief   Write value to selected register field. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDrvHwPpResourceSetRegField
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           fieldOffset,
    IN GT_U32                           fieldLength,
    IN GT_U32                           fieldData
);

/**
* @internal prvCpssDrvHwPpResourceWriteRegBitMask function
* @endinternal
*
* @brief   write a selected register bitmasked value.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpResourceWriteRegBitMask
(

    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           mask,
    IN GT_U32                           value
);

/**
* @internal prvCpssDrvHwPpMg1ReadReg function
* @endinternal
*
* @brief   Read a register value from the management unit #1.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/

GT_STATUS prvCpssDrvHwPpMg1ReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
);

/**
* @internal prvCpssDrvHwPpMg1WriteReg function
* @endinternal
*
* @brief   Write to the management unit 1 given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMg1WriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal prvCpssDrvHwPpMg2ReadReg function
* @endinternal
*
* @brief   Read a register value from the management unit #2.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/

GT_STATUS prvCpssDrvHwPpMg2ReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
);
/**
* @internal prvCpssDrvHwPpMg2WriteReg function
* @endinternal
*
* @brief   Write to the management unit 2 given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMg2WriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal prvCpssDrvHwPpMg3ReadReg function
* @endinternal
*
* @brief   Read a register value from the management unit #3.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/

GT_STATUS prvCpssDrvHwPpMg3ReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
);

/**
* @internal prvCpssDrvHwPpMg3WriteReg function
* @endinternal
*
* @brief   Write to the management unit #3 given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMg3WriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDrvHwCntlh */

