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
* @file prvCpssDrvHwPpPortGroupCntl.h
*
* @brief CPSS Prestera driver structs definitions and functions declarations for
* hardware access, Per port group (supports'multi port groups' device)
*
*
* @version   3
********************************************************************************
*/

#ifndef __prvCpssDrvHwPpPortGroupCntlh
#define __prvCpssDrvHwPpPortGroupCntlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/extServices/os/gtOs/cpssOsSem.h>
#include <cpss/generic/events/cpssGenEventCtrl.h>
#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>


/*******************************************************************************
*   Note: about the 'portGroupId' parameter , in the next APIs.
*   portGroupId - The port group id. relevant only to 'multi-port-groups' devices.
*       when value is CPSS_PORT_GROUP_UNAWARE_MODE_CNS :
*       1. For write APIs CPSS_PORT_GROUP_UNAWARE_MODE_CNS, meaning function should write
*          all active  port groups, to the specified table entry
*          (same as there was no portGroupId parameter)
*       2. For read APIs CPSS_PORT_GROUP_UNAWARE_MODE_CNS, meaning function should read
*          register only from 1st active port group, from the specified table entry
*          (same as there was no portGroupId parameter).
*******************************************************************************/

/*******************************************************************************
* prvCpssDrvHwPpPortGroupGetDrv
*
* DESCRIPTION:
*       Read a register value from the given PP.
*       in the specific port group in the device
* INPUTS:
*       devNum      - The PP to read from.
*       portGroupId - The port group id. relevant only to 'multi-port-groups'
*                     devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                          (APPLICABLE DEVICES: Lion; Lion2)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer to device driver or NULL
*
* COMMENTS:
*       None.
*
*******************************************************************************/
CPSS_HW_DRIVER_STC* prvCpssDrvHwPpPortGroupGetDrv
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);


/**
* @internal prvCpssDrvHwPpPortGroupReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP.
*         in the specific port group in the device
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
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
GT_STATUS prvCpssDrvHwPpPortGroupReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
);



/**
* @internal prvCpssDrvHwPpPortGroupWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register.
*         in the specific port group in the device
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to write to.
*                                      data        - The value to be written.
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
GT_STATUS prvCpssDrvHwPpPortGroupWriteRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);



/**
* @internal prvCpssDrvHwPpPortGroupGetRegField function
* @endinternal
*
* @brief   Read a selected register field.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
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
GT_STATUS prvCpssDrvHwPpPortGroupGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
);



/**
* @internal prvCpssDrvHwPpPortGroupSetRegField function
* @endinternal
*
* @brief   Write value to selected register field.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
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
GT_STATUS prvCpssDrvHwPpPortGroupSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
);



/**
* @internal prvCpssDrvHwPpPortGroupReadRegBitMask function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*         in the specific port group in the device
* @param[in] devNum                   - PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
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
GT_STATUS prvCpssDrvHwPpPortGroupReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
);



/**
* @internal prvCpssDrvHwPpPortGroupWriteRegBitMask function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*         in the specific port group in the device
* @param[in] devNum                   - PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
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
GT_STATUS prvCpssDrvHwPpPortGroupWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    IN GT_U32   value
);



/**
* @internal prvCpssDrvHwPpPortGroupReadRam function
* @endinternal
*
* @brief   Read from PP's RAM.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
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
GT_STATUS prvCpssDrvHwPpPortGroupReadRam
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    OUT GT_U32  *data
);


/**
* @internal prvCpssDrvHwPpPortGroupWriteRamInReverse function
* @endinternal
*
* @brief   Writes to PP's RAM in reverse.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteRamInReverse
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
);

/**
* @internal prvCpssDrvHwPpPortGroupWriteRam function
* @endinternal
*
* @brief   Writes to PP's RAM.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
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
GT_STATUS prvCpssDrvHwPpPortGroupWriteRam
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
);


/**
* @internal prvCpssDrvHwPpPortGroupReadVec function
* @endinternal
*
* @brief   Read from PP's RAM a vector of addresses.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
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
GT_STATUS prvCpssDrvHwPpPortGroupReadVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
);

/**
* @internal prvCpssDrvHwPpPortGroupWriteVec function
* @endinternal
*
* @brief   Writes to PP's RAM a vector of addresses.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] addrArr[]                - Address offset to write to.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
);

/*******************************************************************************
* prvCpssDrvHwPpPortGroupIsrRead
*
* DESCRIPTION:
*       Read a register value using special interrupt address completion region.
*       in the specific port group in the device
*
* INPUTS:
*       devNum      - The PP to read from.
*       portGroupId - The port group id. relevant only to 'multi-port-groups'
*                     devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                          (APPLICABLE DEVICES: Lion; Lion2)
*       regAddr     - The register's address to read from.
*                     Note - regAddr should be < 0x1000000
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
GT_INLINE GT_STATUS prvCpssDrvHwPpPortGroupIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);



/**
* @internal prvCpssDrvHwPpPortGroupIsrWrite function
* @endinternal
*
* @brief   Write a register value using special interrupt address completion region
*         in the specific port group in the device
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to write to.
*                                      data        - The value to be written.
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
GT_STATUS prvCpssDrvHwPpPortGroupIsrWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);

/**
* @internal prvCpssDrvHwPpPortGroupReadInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
*         in the specific port group in the device
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupReadInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);


/**
* @internal prvCpssDrvHwPpPortGroupWriteInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
*         in the specific port group in the device
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to read from.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDrvHwPpPortGroupCntlh */



