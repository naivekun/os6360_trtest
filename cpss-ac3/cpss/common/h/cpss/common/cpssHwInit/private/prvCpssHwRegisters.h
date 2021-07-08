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
* @file prvCpssHwRegisters.h
*
* @brief Private API definition for register access of the generic device.
*
* Those APIs gives next benefit :
* for multi-port group device
* - write of 'global config' into special units may need to we written only in
* some of the port groups (not all)
* - write of 'per port' may need to we written to other port group then
* specified according to 'dest port'
*
* still SOME functionality MUST NOT use those APIs , like:
* - interrupts handling --> needed per specific port group
* - prvCpssPortGroupsCounterSummary(...) , prvCpssPortGroupBusyWait(...)
* and maybe others...
* - maybe others
*
*
* @version   6
********************************************************************************
*/

#ifndef __prvCpssHwRegistersh
#define __prvCpssHwRegistersh

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* indication that the CPSS API level need to call directly to the driver and bypass the
   'shim layer' of the 'duplicated cores' / 'duplicated units' because the caller is
   handling unique case that is not generic. */
#define PRV_CPSS_MUST_DIRECT_CALL_TO_DRIVER_INDICATION_CNS


/* max number of supported duplicated units */
/* The Falcon 12.8T hold 32 instances of DP units */
#define PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS    32
/**
* @struct PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC
 *
 * @brief structure to hold info about the 'additional' registers addresses
 * that needs the same 'read'/'write' as the 'original' address.
*/
typedef struct{

    /** number of (0..PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS */
    GT_U32 numOfAdditionalAddresses;

    GT_U32 additionalAddressesArr[PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS];

    /** @brief indication that the 'original' address is not valid.
     *  GT_TRUE - the 'original' address is not valid.
     *  GT_FALSE - the 'original' address is valid.
     */
    GT_BOOL originalAddressIsNotValid;

    /** @brief indication to use portGroupsArr[].
     *  GT_TRUE - use portGroupsArr[].
     *  GT_FALSE - do not use portGroupsArr[].
     */
    GT_BOOL use_portGroupsArr;

    GT_U32 portGroupsArr[PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS];

    /** @brief indication to use originalAddressPortGroup.
     *  GT_TRUE - use originalAddressPortGroup.
     *  GT_FALSE - do not use originalAddressPortGroup.
     */
    GT_BOOL use_originalAddressPortGroup;

    /** the 'port group' that the 'original' address belongs to. */
    GT_U32 originalAddressPortGroup;

    /* indication that need to skip PortGroupsBmp Mask done by prvCpssDuplicatedMultiPortGroupsGet(...) */
    GT_BOOL skipPortGroupsBmpMask;
} PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC;

/* init structure PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC */
#define PRV_CPSS_INIT_VALUE_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_CNS {0,{0}}

/* Falcon MG0 base address on tile 0 */
#define PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS  0x1D000000

/* Falcon DFX base address on tile 0 */
#define PRV_CPSS_FALCON_DFX_BASE_ADDRESS_CNS  0x1BE00000

/*******************************************************************************
* PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_GET_FUNC
*
* DESCRIPTION:
*       prototype to function that:
*       Check if register address is duplicated in the device.
*       and if duplicated will supply info about the additional duplicated addresses.
*
* APPLICABLE DEVICES:
*        Any.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - the device number
*       portGroupId - The port group id. relevant only to 'multi-port-groups'
*                     devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*       regAddr     - register address to access.
*
* OUTPUTS:
*       portGroupsBmpPtr  - (pointer to) the updated port groups
*                       relevant only when function returns GT_TRUE
*       isAdditionalRegDupNeededPtr - (pointer to)
*             GT_TRUE - the caller need to duplicate the value to additional
*                       register in all 'port groups bmp'.
*             GT_FALSE - the caller NOT need to duplicate the value to additional
*                       register.
*       additionalRegDupPtr - (pointer to)
*                       register address of additional register duplication.
*                       Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
*       maskDrvPortGroupsPtr = (pointer to)
*                       GT_TRUE  - mask the cpssDrv active ports only
*                       GT_FALSE - mask the CPSS    active ports only
*
* RETURNS:
*       GT_TRUE     - the caller need to use the updated 'port groups bmp'
*       GT_FALSE    - the caller NOT need to use the updated 'port groups bmp'
*
* COMMENTS:
*
*
*******************************************************************************/
typedef GT_BOOL (*PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_GET_FUNC)
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);


/**
* @internal prvCpssDuplicatedMultiPortGroupsGet function
* @endinternal
*
* @brief   Check if register address is duplicated in multi-port groups device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
*                                      can be NULL --> meaning 'don't care'
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
*                                      can be NULL --> meaning 'don't care'
*                                      NOTE: when additionalRegDupPtr->originalAddressIsNotValid == GT_TRUE
*                                      the 'regAddr' (original address) should not be accessed !!!
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssDuplicatedMultiPortGroupsGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr
);

/**
* @internal prvCpssHwPpReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP to read from.
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
GT_STATUS prvCpssHwPpReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    OUT GT_U32  *dataPtr
);

/**
* @internal prvCpssHwPpWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] value                    - The  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssHwPpWriteRegister
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 value
);

/**
* @internal prvCpssHwPpGetRegField function
* @endinternal
*
* @brief   Read a selected register field. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to read from.
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
GT_STATUS prvCpssHwPpGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldDataPtr
);

/**
* @internal prvCpssHwPpSetRegField function
* @endinternal
*
* @brief   Write value to selected register field. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssHwPpSetRegField
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData
);



/**
* @internal prvCpssHwPpReadRegBitMask function
* @endinternal
*
* @brief   Reads the unmasked bits of a register. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssHwPpReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
);



/**
* @internal prvCpssHwPpWriteRegBitMask function
* @endinternal
*
* @brief   Writes the unmasked bits of a register. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssHwPpWriteRegBitMask
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 mask,
    IN GT_U32 value
);

/**
* @internal prvCpssHwPpReadRam function
* @endinternal
*
* @brief   Read from PP's RAM. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] addr                     - Address offset to read from.
* @param[in] length                   - Number of Words (4 byte) to read.
*
* @param[out] dataPtr                  - (pointer to) An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssHwPpReadRam
(
    IN  GT_U8   devNum,
    IN  GT_U32  addr,
    IN  GT_U32  length,
    OUT GT_U32  *dataPtr
);

/**
* @internal prvCpssHwPpWriteRam function
* @endinternal
*
* @brief   Writes to PP's RAM. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] dataPtr                  - (pointer to) the array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssHwPpWriteRam
(
    IN GT_U8   devNum,
    IN GT_U32  addr,
    IN GT_U32  length,
    IN GT_U32  *dataPtr
);

/**
* @internal prvCpssHwPpPortGroupReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP. - generic (non-driver) API
*         in the specific port group in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
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
GT_STATUS prvCpssHwPpPortGroupReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *dataPtr
);

/**
* @internal prvCpssHwPpPortGroupWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register. - generic (non-driver) API
*         in the specific port group in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to write to.
* @param[in] value                    - The  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssHwPpPortGroupWriteRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);

/**
* @internal prvCpssHwPpPortGroupGetRegField function
* @endinternal
*
* @brief   Read a selected register field. - generic (non-driver) API
*         in the specific port group in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
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
GT_STATUS prvCpssHwPpPortGroupGetRegField
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   fieldOffset,
    IN  GT_U32   fieldLength,
    OUT GT_U32   *fieldDataPtr
);

/**
* @internal prvCpssHwPpPortGroupSetRegField function
* @endinternal
*
* @brief   Write value to selected register field. - generic (non-driver) API
*         in the specific port group in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
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
GT_STATUS prvCpssHwPpPortGroupSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
);



/**
* @internal prvCpssHwPpPortGroupReadRegBitMask function
* @endinternal
*
* @brief   Reads the unmasked bits of a register. - generic (non-driver) API
*         in the specific port group in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - Register address to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssHwPpPortGroupReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
);



/**
* @internal prvCpssHwPpPortGroupWriteRegBitMask function
* @endinternal
*
* @brief   Writes the unmasked bits of a register. - generic (non-driver) API
*         in the specific port group in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - Register address to write to.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssHwPpPortGroupWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    IN GT_U32   value
);

/**
* @internal prvCpssHwPpPortGroupReadRam function
* @endinternal
*
* @brief   Read from PP's RAM. - generic (non-driver) API
*         in the specific port group in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] addr                     - Address offset to read from.
* @param[in] length                   - Number of Words (4 byte) to read.
*
* @param[out] dataPtr                  - (pointer to) An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssHwPpPortGroupReadRam
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    OUT GT_U32  *dataPtr
);

/**
* @internal prvCpssHwPpPortGroupWriteRam function
* @endinternal
*
* @brief   Writes to PP's RAM. - generic (non-driver) API
*         in the specific port group in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] dataPtr                  - (pointer to) An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssHwPpPortGroupWriteRam
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *dataPtr
);

/**
* @internal prvCpssHwPpMgReadReg function
* @endinternal
*
* @brief   MG register : Read a register value from the management unit.
*           - generic (non-driver) API
* @param[in] devNum                   - The PP to read from.
* @param[in] mgNum                    - 'sip5' : The management unit to read from.
*                                       others : portGroupId (in 'switching core')
* @param[in] regAddr                  - The register address in MG0 !
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpMgReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);

/**
* @internal prvCpssHwPpMgWriteReg function
* @endinternal
*
* @brief   MG register : Write to the management unit given register.
*           - generic (non-driver) API
* @param[in] devNum                   - The PP to write to.
* @param[in] mgNum                    - 'sip5' : The management unit to read from.
*                                       others : portGroupId (in 'switching core')
* @param[in] regAddr                  - The register address in MG0 !
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpMgWriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal prvCpssHwPpMgGetRegField function
* @endinternal
*
* @brief   MG register : Read a selected register field from the Management Unit.
*           - generic (non-driver) API
* @param[in] devNum                   - The PP device number to read from.
* @param[in] mgNum                    - 'sip5' : The management unit to read from.
*                                       others : portGroupId (in 'switching core')
* @param[in] regAddr                  - The register address in MG0 !
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
GT_STATUS prvCpssHwPpMgGetRegField
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    OUT GT_U32  *fieldData
);

/**
* @internal prvCpssHwPpMgSetRegField function
* @endinternal
*
* @brief   MG register : Write value to selected register field of the Management Unit.
*           - generic (non-driver) API
* @param[in] devNum                   - The PP device number to write to.
* @param[in] mgNum                    - 'sip5' : The management unit to read from.
*                                       others : portGroupId (in 'switching core')
* @param[in] regAddr                  - The register address in MG0 !
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
GT_STATUS prvCpssHwPpMgSetRegField
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    IN  GT_U32  fieldData
);

/**
* @internal prvCpssHwPpMgRegBusyWaitByMask function
* @endinternal
*
* @brief   MG register : function do 'Busy wait' on specific mask of the register.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mgNum                    - 'sip5' : The management unit to read from.
*                                       others : portGroupId (in 'switching core')
* @param[in] regAddr                  - The register address in MG0 !
* @param[in] regMask                  - the mask of the register that we wait for to clear
* @param[in] onlyCheck                - do we want only to check the current status , or to wait
*                                      until ready
*                                      GT_TRUE - check status , without busy wait
*                                      GT_FALSE - loop on the value until value reached
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - when onlyCheck == GT_FALSE and the busy wait time expired
* @retval GT_BAD_STATE             - when onlyCheck == GT_TRUE and the value was not reached
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpMgRegBusyWaitByMask
(
    IN GT_U8                            devNum,
    IN GT_U32                           mgNum,
    IN GT_U32                           regAddr,
    IN GT_U32                           regMask,
    IN GT_BOOL                          onlyCheck
);

/**
* @internal prvCpssHwPpMgWriteRegBitMask function
* @endinternal
*
* @brief  MG register :  Writes the unmasked bits of a register.
*         - generic (non-driver) API
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] mgNum                    - 'sip5' : The management unit to read from.
*                                       others : portGroupId (in 'switching core')
* @param[in] regAddr                  - The register address in MG0 !
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssHwPpMgWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   mgNum,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    IN GT_U32   value
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssHwRegistersh */

