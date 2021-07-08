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
* @file cpssDriverPpHw.c
*
* @brief CPSS PP Driver Hardware Access API
*
* @version   9
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpssDriver/log/private/prvCpssDriverLog.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>


extern GT_BOOL  prvCpssDrvTraceHwWrite[PRV_CPSS_MAX_PP_DEVICES_CNS];
extern GT_BOOL  prvCpssDrvTraceHwRead[PRV_CPSS_MAX_PP_DEVICES_CNS];
extern GT_BOOL  prvCpssDrvTraceHwDelay[PRV_CPSS_MAX_PP_DEVICES_CNS];

/* Object that holds callback function to HW access */
extern CPSS_DRV_HW_ACCESS_OBJ_STC prvCpssHwAccessObj;

/**
* @internal internal_cpssDrvPpHwRegisterRead function
* @endinternal
*
* @brief   Read a register value from the given PP.
*
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
*/
static GT_STATUS internal_cpssDrvPpHwRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    return prvCpssDrvHwPpPortGroupReadRegister(devNum,portGroupId, regAddr, data);
}

/**
* @internal cpssDrvPpHwRegisterRead function
* @endinternal
*
* @brief   Read a register value from the given PP.
*
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
*/
GT_STATUS cpssDrvPpHwRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRegisterRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, data));

    rc = internal_cpssDrvPpHwRegisterRead(devNum, portGroupId, regAddr, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, regAddr, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwRegisterWrite function
* @endinternal
*
* @brief   Write to a PP's given register.
*
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
*/
static GT_STATUS internal_cpssDrvPpHwRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
     return prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, regAddr, value);
}

/**
* @internal cpssDrvPpHwRegisterWrite function
* @endinternal
*
* @brief   Write to a PP's given register.
*
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
*/
GT_STATUS cpssDrvPpHwRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRegisterWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, value));

    rc = internal_cpssDrvPpHwRegisterWrite(devNum, portGroupId, regAddr, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, regAddr, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwRegFieldGet function
* @endinternal
*
* @brief   Read a selected register field.
*
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
*/
static GT_STATUS internal_cpssDrvPpHwRegFieldGet
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    return prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId, regAddr,
                                     fieldOffset, fieldLength, fieldData);
}

/**
* @internal cpssDrvPpHwRegFieldGet function
* @endinternal
*
* @brief   Read a selected register field.
*
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
*/
GT_STATUS cpssDrvPpHwRegFieldGet
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRegFieldGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, fieldOffset, fieldLength, fieldData));

    rc = internal_cpssDrvPpHwRegFieldGet(devNum, portGroupId, regAddr, fieldOffset, fieldLength, fieldData);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, regAddr, fieldOffset, fieldLength, fieldData));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwRegFieldSet function
* @endinternal
*
* @brief   Write value to selected register field.
*
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
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
static GT_STATUS internal_cpssDrvPpHwRegFieldSet
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData

)
{
     return prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId, regAddr,
                                      fieldOffset, fieldLength, fieldData);
}

/**
* @internal cpssDrvPpHwRegFieldSet function
* @endinternal
*
* @brief   Write value to selected register field.
*
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
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS cpssDrvPpHwRegFieldSet
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRegFieldSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, fieldOffset, fieldLength, fieldData));

    rc = internal_cpssDrvPpHwRegFieldSet(devNum, portGroupId, regAddr, fieldOffset, fieldLength, fieldData);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, regAddr, fieldOffset, fieldLength, fieldData));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwRegBitMaskRead function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
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
*
*/
static GT_STATUS internal_cpssDrvPpHwRegBitMaskRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
)
{
    return prvCpssDrvHwPpPortGroupReadRegBitMask(devNum,portGroupId, regAddr, mask, dataPtr);
}

/**
* @internal cpssDrvPpHwRegBitMaskRead function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
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
*
*/
GT_STATUS cpssDrvPpHwRegBitMaskRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRegBitMaskRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, mask, dataPtr));

    rc = internal_cpssDrvPpHwRegBitMaskRead(devNum, portGroupId, regAddr, mask, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, regAddr, mask, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwRegBitMaskWrite function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
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
*
*/
static GT_STATUS internal_cpssDrvPpHwRegBitMaskWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    IN GT_U32   value
)
{
    return prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum,portGroupId, regAddr, mask, value);
}

/**
* @internal cpssDrvPpHwRegBitMaskWrite function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
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
*
*/
GT_STATUS cpssDrvPpHwRegBitMaskWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    IN GT_U32   value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRegBitMaskWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, mask, value));

    rc = internal_cpssDrvPpHwRegBitMaskWrite(devNum, portGroupId, regAddr, mask, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, regAddr, mask, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwRamRead function
* @endinternal
*
* @brief   Read from PP's RAM.
*
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
*/
static GT_STATUS internal_cpssDrvPpHwRamRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    OUT GT_U32  *data
)
{
    return prvCpssDrvHwPpPortGroupReadRam(devNum,portGroupId, addr, length, data);
}

/**
* @internal cpssDrvPpHwRamRead function
* @endinternal
*
* @brief   Read from PP's RAM.
*
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
*/
GT_STATUS cpssDrvPpHwRamRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRamRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addr, length, data));

    rc = internal_cpssDrvPpHwRamRead(devNum, portGroupId, addr, length, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, addr, length, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwRamWrite function
* @endinternal
*
* @brief   Writes to PP's RAM.
*
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
static GT_STATUS internal_cpssDrvPpHwRamWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
)
{
    return prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId, addr, length, data);
}

/**
* @internal cpssDrvPpHwRamWrite function
* @endinternal
*
* @brief   Writes to PP's RAM.
*
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
GT_STATUS cpssDrvPpHwRamWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRamWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addr, length, data));

    rc = internal_cpssDrvPpHwRamWrite(devNum, portGroupId, addr, length, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, addr, length, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwVectorRead function
* @endinternal
*
* @brief   Read from PP's RAM a vector of addresses.
*
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
static GT_STATUS internal_cpssDrvPpHwVectorRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
)
{
    return prvCpssDrvHwPpPortGroupReadVec(devNum,portGroupId, addrArr, dataArr, arrLen);
}

/**
* @internal cpssDrvPpHwVectorRead function
* @endinternal
*
* @brief   Read from PP's RAM a vector of addresses.
*
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
GT_STATUS cpssDrvPpHwVectorRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwVectorRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addrArr, dataArr, arrLen));

    rc = internal_cpssDrvPpHwVectorRead(devNum, portGroupId, addrArr, dataArr, arrLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, addrArr, dataArr, arrLen));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwVectorWrite function
* @endinternal
*
* @brief   Writes to PP's RAM a vector of addresses.
*
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
static GT_STATUS internal_cpssDrvPpHwVectorWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
)
{
    return prvCpssDrvHwPpPortGroupWriteVec(devNum,portGroupId, addrArr, dataArr, arrLen);
}

/**
* @internal cpssDrvPpHwVectorWrite function
* @endinternal
*
* @brief   Writes to PP's RAM a vector of addresses.
*
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
GT_STATUS cpssDrvPpHwVectorWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwVectorWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addrArr, dataArr, arrLen));

    rc = internal_cpssDrvPpHwVectorWrite(devNum, portGroupId, addrArr, dataArr, arrLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, addrArr, dataArr, arrLen));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwRamInReverseWrite function
* @endinternal
*
* @brief   Writes to PP's RAM in reverse.
*
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
static GT_STATUS internal_cpssDrvPpHwRamInReverseWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
)
{
    return prvCpssDrvHwPpPortGroupWriteRamInReverse(devNum,portGroupId, addr, length, data);
}

/**
* @internal cpssDrvPpHwRamInReverseWrite function
* @endinternal
*
* @brief   Writes to PP's RAM in reverse.
*
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
GT_STATUS cpssDrvPpHwRamInReverseWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwRamInReverseWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addr, length, data));

    rc = internal_cpssDrvPpHwRamInReverseWrite(devNum, portGroupId, addr, length, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, addr, length, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* cpssDrvPpHwIsrRead
*
* DESCRIPTION:
*       Read a register value using special interrupt address completion region.
*
* INPUTS:
*       devNum      - The PP to read from.
*       portGroupId - The port group id. relevant only to 'multi-port-groups'
*                     devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                          (APPLICABLE DEVICES: Lion; Lion2)
*       regAddr     - The register's address to read from.
*                     Note: regAddr should be < 0x1000000
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
*******************************************************************************/
GT_INLINE GT_STATUS cpssDrvPpHwIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    return prvCpssDrvHwPpPortGroupIsrRead(devNum,portGroupId, regAddr, dataPtr);
}

/**
* @internal cpssDrvPpHwIsrWrite function
* @endinternal
*
* @brief   Write a register value using special interrupt address completion region
*
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
*/
GT_STATUS cpssDrvPpHwIsrWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
    return prvCpssDrvHwPpPortGroupIsrWrite(devNum,portGroupId, regAddr, value);
}

/**
* @internal cpssDrvPpHwInternalPciRegRead function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
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
GT_STATUS cpssDrvPpHwInternalPciRegRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    return prvCpssDrvHwPpPortGroupReadInternalPciReg(devNum,portGroupId, regAddr, data);
}

/**
* @internal cpssDrvPpHwInternalPciRegWrite function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
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
GT_STATUS cpssDrvPpHwInternalPciRegWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    return prvCpssDrvHwPpPortGroupWriteInternalPciReg(devNum,portGroupId, regAddr, data);
}

/**
* @internal internal_cpssDrvHwPpResetAndInitControllerReadReg function
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
static GT_STATUS internal_cpssDrvHwPpResetAndInitControllerReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    return prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, data);
}

/**
* @internal cpssDrvHwPpResetAndInitControllerReadReg function
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
GT_STATUS cpssDrvHwPpResetAndInitControllerReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvHwPpResetAndInitControllerReadReg);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, data));

    rc = internal_cpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regAddr, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvHwPpResetAndInitControllerWriteReg function
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
static GT_STATUS internal_cpssDrvHwPpResetAndInitControllerWriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
     return prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr, data);
}

/**
* @internal cpssDrvHwPpResetAndInitControllerWriteReg function
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
GT_STATUS cpssDrvHwPpResetAndInitControllerWriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvHwPpResetAndInitControllerWriteReg);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, data));

    rc = internal_cpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regAddr, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvHwPpResetAndInitControllerGetRegField function
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
static GT_STATUS internal_cpssDrvHwPpResetAndInitControllerGetRegField
(
    IN  GT_U8   devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    return prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                                     fieldOffset, fieldLength, fieldData);
}

/**
* @internal cpssDrvHwPpResetAndInitControllerGetRegField function
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
GT_STATUS cpssDrvHwPpResetAndInitControllerGetRegField
 (
    IN  GT_U8   devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvHwPpResetAndInitControllerGetRegField);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, fieldOffset, fieldLength, fieldData));

    rc = internal_cpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr, fieldOffset, fieldLength, fieldData);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regAddr, fieldOffset, fieldLength, fieldData));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvHwPpResetAndInitControllerSetRegField function
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
static GT_STATUS internal_cpssDrvHwPpResetAndInitControllerSetRegField
(
    IN  GT_U8   devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
     return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr,
                                      fieldOffset, fieldLength, fieldData);
}

/**
* @internal cpssDrvHwPpResetAndInitControllerSetRegField function
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
GT_STATUS cpssDrvHwPpResetAndInitControllerSetRegField
 (
    IN  GT_U8   devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvHwPpResetAndInitControllerSetRegField);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, fieldOffset, fieldLength, fieldData));

    rc = internal_cpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, fieldOffset, fieldLength, fieldData);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regAddr, fieldOffset, fieldLength, fieldData));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwTraceEnable function
* @endinternal
*
* @brief   This function enables/disables to trace hw access.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] traceType                - trace access type.
* @param[in] enable                   - GT_TRUE:  hw access info tracing
*                                      GT_FALSE: disable hw access info tracing
*                                       None.
*/
static GT_STATUS internal_cpssDrvPpHwTraceEnable
(
    IN GT_U8                        devNum,
    IN CPSS_DRV_HW_TRACE_TYPE_ENT   traceType,
    IN GT_BOOL                      enable
)
{
    switch (traceType)
    {
        case CPSS_DRV_HW_TRACE_TYPE_READ_E:
            prvCpssDrvTraceHwRead[devNum] = enable;
            break;
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[unterminated_case] */
        case CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E: /*No break*/
            prvCpssDrvTraceHwDelay[devNum]=enable;
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[fallthrough] */
        case CPSS_DRV_HW_TRACE_TYPE_WRITE_E:
            prvCpssDrvTraceHwWrite[devNum] = enable;
            if (enable==GT_FALSE) /*If only write is disabled so is delay for the device*/
            {
                prvCpssDrvTraceHwDelay[devNum]=enable;
            }
            break;
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[unterminated_case] */
        case CPSS_DRV_HW_TRACE_TYPE_ALL_E: /*No break*/
            prvCpssDrvTraceHwDelay[devNum]=enable;
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[fallthrough] */
        case CPSS_DRV_HW_TRACE_TYPE_BOTH_E:
            prvCpssDrvTraceHwRead[devNum] = enable;
            prvCpssDrvTraceHwWrite[devNum] = enable;
            if (enable==GT_FALSE) /*If only write is disabled so is delay for the device*/
            {
                prvCpssDrvTraceHwDelay[devNum]=enable;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDrvPpHwTraceEnable function
* @endinternal
*
* @brief   This function enables/disables to trace hw access.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] traceType                - trace access type.
* @param[in] enable                   - GT_TRUE:  hw access info tracing
*                                      GT_FALSE: disable hw access info tracing
*                                       None.
*/
GT_STATUS cpssDrvPpHwTraceEnable
(
    IN GT_U8                        devNum,
    IN CPSS_DRV_HW_TRACE_TYPE_ENT   traceType,
    IN GT_BOOL                      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwTraceEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, traceType, enable));

    rc = internal_cpssDrvPpHwTraceEnable(devNum, traceType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, traceType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* cpssDrvHwPpHwInfoStcPtrGet
*
* DESCRIPTION:
*       Read pointer to CPSS_HW_INFO_STC for a given devNum/portGroupId
*
* INPUTS:
*       devNum  - The PP device number to read from.
*       portGroupId  - The port group Id. relevant only to 'multi-port-groups' devices.
*                          (APPLICABLE DEVICES: Lion; Lion2)
*                 supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to CPSS_HW_INFO_STC
*       NULL - if not set/invalid parameter
*
* COMMENTS:
*       None.
*
*******************************************************************************/
CPSS_HW_INFO_STC* cpssDrvHwPpHwInfoStcPtrGet
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId
)
{
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        return NULL;
    if (prvCpssDrvPpConfig[devNum] == NULL)
        return NULL;
    if (portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        portGroupId = 0;
    if (portGroupId >= CPSS_MAX_PORT_GROUPS_CNS)
        return NULL;
    if (prvCpssDrvPpConfig[devNum]->hwInfo[portGroupId].busType == CPSS_HW_INFO_BUS_TYPE_NONE_E)
        return NULL;
    return &(prvCpssDrvPpConfig[devNum]->hwInfo[portGroupId]);
}

/**
* @internal internal_cpssDrvHwAccessObjectBind function
* @endinternal
*
* @brief   The function binds/unbinds a calbacck routines for HW access.
*
* @param[in] hwAccessObjPtr           - HW access object pointer.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - un bind callback routines.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS internal_cpssDrvHwAccessObjectBind
(
    IN CPSS_DRV_HW_ACCESS_OBJ_STC *hwAccessObjPtr,
    IN GT_BOOL                     bind
)
{

    if(bind)
        cpssOsMemCpy( &prvCpssHwAccessObj, hwAccessObjPtr, sizeof(CPSS_DRV_HW_ACCESS_OBJ_STC));
    else
        cpssOsMemSet( &prvCpssHwAccessObj, 0, sizeof(CPSS_DRV_HW_ACCESS_OBJ_STC));

    return GT_OK;
}

/**
* @internal cpssDrvHwAccessObjectBind function
* @endinternal
*
* @brief   The function binds/unbinds a calbacck routines for HW access.
*
* @param[in] hwAccessObjPtr           - HW access object pointer.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - un bind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssDrvHwAccessObjectBind
(
    IN CPSS_DRV_HW_ACCESS_OBJ_STC *hwAccessObjPtr,
    IN GT_BOOL                     bind
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvHwAccessObjectBind);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, hwAccessObjPtr, bind));

    rc = internal_cpssDrvHwAccessObjectBind(hwAccessObjPtr, bind);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, hwAccessObjPtr, bind));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDrvPpHwResourceReadRegister function
* @endinternal
*
* @brief   Read a register value from a resource.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] resourceType             - The resource type.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDrvPpHwResourceReadRegister
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroupId,
    IN  CPSS_DRV_HW_RESOURCE_TYPE_ENT   resourceType,
    IN  GT_U32                          regAddr,
    OUT GT_U32                          *dataPtr
)
{
    return prvCpssDrvHwPpResourceReadRegister(devNum, portGroupId, resourceType, regAddr, dataPtr);
}

/**
* @internal cpssDrvPpHwResourceReadRegister function
* @endinternal
*
* @brief   Read a register value from a resource.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] resourceType             - The resource type.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDrvPpHwResourceReadRegister
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroupId,
    IN  CPSS_DRV_HW_RESOURCE_TYPE_ENT   resourceType,
    IN  GT_U32                          regAddr,
    OUT GT_U32                          *dataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwResourceReadRegister);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, resourceType, regAddr, dataPtr));

    rc = internal_cpssDrvPpHwResourceReadRegister(devNum, portGroupId, resourceType, regAddr, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, resourceType, regAddr, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDrvPpHwResourceWriteRegister function
* @endinternal
*
* @brief   Write to a resource given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] resourceType             - The resource type.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDrvPpHwResourceWriteRegister
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroupId,
    IN  CPSS_DRV_HW_RESOURCE_TYPE_ENT   resourceType,
    IN  GT_U32                          regAddr,
    IN  GT_U32                          data
)
{
     return prvCpssDrvHwPpResourceWriteRegister(devNum, portGroupId, resourceType, regAddr, data);
}

/**
* @internal cpssDrvPpHwResourceWriteRegister function
* @endinternal
*
* @brief   Write to a resource given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] resourceType             - The resource type.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDrvPpHwResourceWriteRegister
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroupId,
    IN  CPSS_DRV_HW_RESOURCE_TYPE_ENT   resourceType,
    IN  GT_U32                          regAddr,
    IN  GT_U32                          data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDrvPpHwResourceWriteRegister);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, resourceType, regAddr, data));

    rc = internal_cpssDrvPpHwResourceWriteRegister(devNum, portGroupId, resourceType, regAddr, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, resourceType, regAddr, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



