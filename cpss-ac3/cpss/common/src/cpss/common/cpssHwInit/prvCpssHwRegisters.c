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
* @file prvCpssHwRegisters.c
*
* @brief Private API definition for register access of the DXCH and Pipe devices.
*
* Those APIs gives next benefit :
* for Lion (multi-port group device) with TXQ ver 1 (or above)
* - write of 'global config' into the TXQ may need to we written only in
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
* @version   33
********************************************************************************
*/


#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

#if ((defined ASIC_SIMULATION) && (defined EXMXPM_FAMILY) && (defined SPECIAL_PUMA3_DEBUG))
    #define ALLOW_EMPTY_PORT_GROUPS
#endif


#ifdef ALLOW_EMPTY_PORT_GROUPS
        /* the read/write register of 'non-valid' will try to loop on 0 port groups -->
            will not fail the function , but do nothing
            --> for puma3 initialization DEBUG ONLY */
    #define RETURN_VALUE_OF_MULTI_PORT_GROUPS_FAIL_CNS          GT_OK
#else  /*!ALLOW_EMPTY_PORT_GROUPS*/
    #define RETURN_VALUE_OF_MULTI_PORT_GROUPS_FAIL_CNS          GT_FAIL
#endif /*!ALLOW_EMPTY_PORT_GROUPS*/

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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    GT_BOOL     didBmpUpgrade;
    PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC tmp_additionalRegDup;/*used in case of OUT param is NULL */
    GT_BOOL     tmp_isAdditionalRegDupNeeded;                              /*used in case of OUT param is NULL */
    GT_BOOL     maskDrvPortGroups = GT_TRUE;

    if(additionalRegDupPtr == NULL)
    {
        /* the caller not care about the duplications ... only if exists  ...
           usually for multi-core devices with active cores */
        isAdditionalRegDupNeededPtr = &tmp_isAdditionalRegDupNeeded;
        additionalRegDupPtr = &tmp_additionalRegDup;
    }

    *isAdditionalRegDupNeededPtr = GT_FALSE;
    /* default value for BWC */
    additionalRegDupPtr->originalAddressIsNotValid = GT_FALSE;
    additionalRegDupPtr->use_portGroupsArr = GT_FALSE;
    additionalRegDupPtr->use_originalAddressPortGroup = GT_FALSE;
    additionalRegDupPtr->skipPortGroupsBmpMask = GT_FALSE;
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if(PRV_CPSS_PP_MAC(devNum)->duplicatedAddrGetFunc)
    {
        /* call to CB function to get the results */
        didBmpUpgrade = PRV_CPSS_PP_MAC(devNum)->duplicatedAddrGetFunc(devNum,
           portGroupId,regAddr,portGroupsBmpPtr,
           isAdditionalRegDupNeededPtr,additionalRegDupPtr,
           &maskDrvPortGroups);
    }
    else
    {
        didBmpUpgrade = GT_FALSE;
    }

    if (didBmpUpgrade == GT_FALSE)
    {
        return GT_FALSE;
    }

    if(additionalRegDupPtr->skipPortGroupsBmpMask == GT_TRUE)
    {
        /* skip the mask */
    }
    else
    if(maskDrvPortGroups == GT_TRUE)
    {
        /* mask the cpssDrv active ports only */
        *portGroupsBmpPtr &= prvCpssDrvPpConfig[devNum]->portGroupsInfo.activePortGroupsBmp;
    }
    else
    {
        /* mask the CPSS active ports only */
        *portGroupsBmpPtr &= PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
    }

    return GT_TRUE;
}

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
)
{
    return prvCpssHwPpPortGroupGetRegField(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,
        0,32,
        dataPtr);
}



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
)
{
    return prvCpssHwPpPortGroupSetRegField(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,
        0,32,
        value);
}



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
)
{
    return prvCpssHwPpPortGroupGetRegField(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,
        fieldOffset,fieldLength,fieldDataPtr);
}



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
)
{
    return prvCpssHwPpPortGroupSetRegField(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,
        fieldOffset,fieldLength,fieldData);
}



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
)
{
    return prvCpssHwPpPortGroupReadRegBitMask(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,
        mask,dataPtr);
}




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
)
{
    return prvCpssHwPpPortGroupWriteRegBitMask(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,
        mask,value);
}



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
)
{
    return prvCpssHwPpPortGroupReadRam(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        addr,length,dataPtr);
}


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
)
{
    return prvCpssHwPpPortGroupWriteRam(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        addr,length,dataPtr);
}

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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    return prvCpssHwPpPortGroupGetRegField(
        devNum, portGroupId,
        regAddr, 0, 32, dataPtr);
}

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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    return prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,
        regAddr,
        0,32,
        value);
}


/**
* @internal getFirstAdditionalRegister function
* @endinternal
 *
* @brief   Get the register address to be used from additionalRegDupPtr that
*          match the needed port group - generic (non-driver) API.
*
* @note   APPLICABLE DEVICES:      ALL DXCH.(function is device-less)
* @note   NOT APPLICABLE DEVICES:
*
* @retval regAddr                  - The register's address to use.
* @param[in] portGroupId           - The port group id. relevant only to 'multi-port-groups'
*                                      devices.
* @param[in] additionalRegDupPtr   - (pointer to) the additional registers info
*/
static GT_U32   getFirstAdditionalRegister(
    IN PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC  *additionalRegDupPtr,
    IN GT_U32   portGroupId
)
{
    GT_U32  index;

    if(additionalRegDupPtr->use_portGroupsArr == GT_FALSE)
    {
        index = 0;
    }
    else
    {
        for (index = 0 ; index < additionalRegDupPtr->numOfAdditionalAddresses;index++)
        {
            if(additionalRegDupPtr->portGroupsArr[index] == portGroupId)
            {
                break;
            }
        }

        if(index == additionalRegDupPtr->numOfAdditionalAddresses)
        {
            /* not found */
            index = 0;
        }
    }

    return additionalRegDupPtr->additionalAddressesArr[index];
}

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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    GT_STATUS   rc;
    GT_U32  mask  = BIT_MASK_MAC(fieldLength) << fieldOffset;
    GT_U32  value;

    rc = prvCpssHwPpPortGroupReadRegBitMask(devNum,portGroupId,regAddr,mask,&value);
    *fieldDataPtr = value >> fieldOffset;

    return rc;
}



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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    GT_U32  mask  = BIT_MASK_MAC(fieldLength) << fieldOffset;
    GT_U32  value =                 fieldData << fieldOffset;

    return prvCpssHwPpPortGroupWriteRegBitMask(devNum,portGroupId,regAddr,mask,value);
}


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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    GT_STATUS       rc;
    GT_PORT_GROUPS_BMP  portGroupsBmp;/*port groups bmp */
    GT_BOOL isAdditionalRegDupNeeded;
    PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC  additionalRegDup;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if(GT_FALSE ==
        prvCpssDuplicatedMultiPortGroupsGet(devNum,portGroupId,regAddr,&portGroupsBmp,
                &isAdditionalRegDupNeeded,&additionalRegDup))
    {
        return prvCpssDrvHwPpPortGroupReadRegBitMask(devNum,portGroupId,regAddr,mask,dataPtr);
    }

    if(isAdditionalRegDupNeeded == GT_TRUE &&
       (additionalRegDup.originalAddressIsNotValid == GT_TRUE ||
        (additionalRegDup.use_originalAddressPortGroup == GT_TRUE &&
             portGroupId != CPSS_PORT_GROUP_UNAWARE_MODE_CNS &&
         additionalRegDup.originalAddressPortGroup != portGroupId)))
    {
        /* replace the register address */
        regAddr = getFirstAdditionalRegister(&additionalRegDup,portGroupId);
    }

#ifdef ALLOW_EMPTY_PORT_GROUPS
    if(portGroupsBmp == 0)
    {
        return GT_OK;
    }
#endif /*ALLOW_EMPTY_PORT_GROUPS*/

    if(additionalRegDup.skipPortGroupsBmpMask == GT_TRUE)
    {
        /* called from prvCpssDrvHwPpDoReadOpDrv to support M1,MG2...*/

        /**************************/
        /* support MG multi units */
        /**************************/
        rc = prvCpssPpConfigBitmapFirstActiveBitGet(portGroupsBmp,&portGroupId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* get first port group in the BMP */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);
    }

    rc = prvCpssDrvHwPpPortGroupReadRegBitMask(devNum,portGroupId,regAddr,mask,dataPtr);

    return rc;
}




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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    GT_STATUS       rc = RETURN_VALUE_OF_MULTI_PORT_GROUPS_FAIL_CNS;
    GT_PORT_GROUPS_BMP  portGroupsBmp;/*port groups bmp */
    GT_BOOL isAdditionalRegDupNeeded;
    PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC  additionalRegDup;
    GT_U32  ii;
    GT_U32  firstActivePortGroup,lastActivePortGroup;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if(GT_FALSE ==
        prvCpssDuplicatedMultiPortGroupsGet(devNum,portGroupId,regAddr,&portGroupsBmp,
                &isAdditionalRegDupNeeded,&additionalRegDup))
    {
        return prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum,portGroupId,regAddr,mask,value);
    }

    if(additionalRegDup.skipPortGroupsBmpMask == GT_TRUE)
    {
        /* called from prvCpssDrvHwPpWriteRegBitMaskDrv to support M1,MG2...*/

        /**************************/
        /* support MG multi units */
        /**************************/

        firstActivePortGroup = 0;   /* no limit on start */
        lastActivePortGroup  = 31;  /* no limit on end   */
    }
    else
    {
        firstActivePortGroup = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
        lastActivePortGroup  = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup;
        portGroupsBmp       &= PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
    }

    /* loop on portGroupId , from firstActivePortGroup till lastActivePortGroup
       only those in portGroupsBmp */
    PRV_CPSS_GEN_START_LOOP_INDEX_MAC(devNum,portGroupId,firstActivePortGroup,lastActivePortGroup,portGroupsBmp)
    {
        if(isAdditionalRegDupNeeded == GT_TRUE &&
           (additionalRegDup.originalAddressIsNotValid == GT_TRUE ||
            (additionalRegDup.use_originalAddressPortGroup == GT_TRUE &&
             additionalRegDup.originalAddressPortGroup != portGroupId)))
        {
            /* the original address is not valid for use ! */
        }
        else
        {
            rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum,portGroupId,regAddr,mask,value);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if (isAdditionalRegDupNeeded == GT_TRUE)
        {
            for(ii = 0 ; ii < additionalRegDup.numOfAdditionalAddresses; ii++)
            {
                if(additionalRegDup.use_portGroupsArr == GT_TRUE &&
                   additionalRegDup.portGroupsArr[ii] != portGroupId)
                {
                    /* skip the accessing to those additional addresses that not
                       belong to the current port group */
                    continue;
                }
                rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum,portGroupId,additionalRegDup.additionalAddressesArr[ii],mask,value);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_GEN_END_LOOP_INDEX_MAC(devNum,portGroupId)

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "no port group found");
    }

    return rc;
}

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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    GT_STATUS       rc;
    GT_PORT_GROUPS_BMP  portGroupsBmp;/*port groups bmp */
    GT_BOOL isAdditionalRegDupNeeded;
    PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC  additionalRegDup;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if(GT_FALSE ==
        prvCpssDuplicatedMultiPortGroupsGet(
            devNum,portGroupId,addr,&portGroupsBmp,
            &isAdditionalRegDupNeeded,&additionalRegDup))
    {
        return prvCpssDrvHwPpPortGroupReadRam(
            devNum,portGroupId,addr,length,dataPtr);
    }

    if(isAdditionalRegDupNeeded == GT_TRUE &&
       (additionalRegDup.originalAddressIsNotValid == GT_TRUE ||
        (additionalRegDup.use_originalAddressPortGroup == GT_TRUE &&
             portGroupId != CPSS_PORT_GROUP_UNAWARE_MODE_CNS &&
         additionalRegDup.originalAddressPortGroup != portGroupId)))
    {
        /* replace the register address */
        addr = getFirstAdditionalRegister(&additionalRegDup,portGroupId);
    }

#ifdef ALLOW_EMPTY_PORT_GROUPS
    if(portGroupsBmp == 0)
    {
        return GT_OK;
    }
#endif /*ALLOW_EMPTY_PORT_GROUPS*/
    /* get first port group in the BMP */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum,portGroupsBmp,portGroupId);

    rc = prvCpssDrvHwPpPortGroupReadRam(
        devNum,portGroupId,addr,length,dataPtr);

    return rc;
}

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
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      (APPLICABLE DEVICES ExMxPm3)
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
)
{
    GT_STATUS       rc = RETURN_VALUE_OF_MULTI_PORT_GROUPS_FAIL_CNS;
    GT_PORT_GROUPS_BMP  portGroupsBmp;/*port groups bmp */
    GT_BOOL isAdditionalRegDupNeeded;
    PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC  additionalRegDup;
    GT_U32  ii;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if(GT_FALSE ==
        prvCpssDuplicatedMultiPortGroupsGet(
            devNum,portGroupId,addr,&portGroupsBmp,
            &isAdditionalRegDupNeeded,&additionalRegDup))
    {
        return prvCpssDrvHwPpPortGroupWriteRam(
            devNum,portGroupId,addr,length,dataPtr);
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum,portGroupsBmp,portGroupId)
    {
        if(isAdditionalRegDupNeeded == GT_TRUE &&
           (additionalRegDup.originalAddressIsNotValid == GT_TRUE ||
            (additionalRegDup.use_originalAddressPortGroup == GT_TRUE &&
             additionalRegDup.originalAddressPortGroup != portGroupId)))
        {
            /* the original address is not valid for use ! */
        }
        else
        {
            rc = prvCpssDrvHwPpPortGroupWriteRam(
                devNum,portGroupId,addr,length,dataPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if (isAdditionalRegDupNeeded == GT_TRUE)
        {
            for(ii = 0 ; ii < additionalRegDup.numOfAdditionalAddresses; ii++)
            {
                if(additionalRegDup.use_portGroupsArr == GT_TRUE &&
                   additionalRegDup.portGroupsArr[ii] != portGroupId)
                {
                    /* skip the accessing to those additional addresses that not
                       belong to the current port group */
                    continue;
                }
                rc = prvCpssDrvHwPpPortGroupWriteRam(
                    devNum,portGroupId,additionalRegDup.additionalAddressesArr[ii],length,dataPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum,portGroupsBmp,portGroupId)

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "no port group found");
    }

    return rc;
}

/**
* @internal prvCpssHwPpMgReadReg function
* @endinternal
*
* @brief   MG register : Read a register value from the management unit.
*           - generic (non-driver) API
* @param[in] devNum                   - The PP to read from.
* @param[in] mgNum                    - 'sip5' : The management unit to read from.
*                                       others : portGroupId (in 'switching core')
*
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
)
{
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* sip6 : calling the 'non-Drv' for the MG[0..max] */
        /* with port group == 'MG' */
        return prvCpssHwPpPortGroupReadRegister(devNum,mgNum,regAddr,data);
    }
    else
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* sip5 : calling the 'Drv' for the specific MG */
        return prvCpssDrvHwPpMgReadReg(devNum, mgNum, regAddr, data);
    }
    else
    {
        /* sip4 : calling the 'Drv' with port group == 'MG' */
        return prvCpssDrvHwPpPortGroupReadRegister(devNum,mgNum,regAddr,data);
    }
}

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
)
{
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* sip6 : calling the 'non-Drv' for the MG[0..max] */
        /* with port group == 'MG' */
        return prvCpssHwPpPortGroupWriteRegister(devNum,mgNum,regAddr,data);
    }
    else
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* sip5 : calling the 'Drv' for the specific MG */
        return prvCpssDrvHwPpMgWriteReg(devNum, mgNum, regAddr, data);
    }
    else
    {
        /* sip4 : calling the 'Drv' with port group == 'MG' */
        return prvCpssDrvHwPpPortGroupWriteRegister(devNum,mgNum,regAddr,data);
    }
}

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
)
{
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* sip6 : calling the 'non-Drv' for the MG[0..max] */
        /* with port group == 'MG' */
        return prvCpssHwPpPortGroupGetRegField(devNum,mgNum,regAddr,fieldOffset,fieldLength,fieldData);
    }
    else
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* sip5 : calling the 'Drv' for the specific MG */
        return prvCpssDrvHwPpMgGetRegField(devNum, mgNum, regAddr, fieldOffset,fieldLength,fieldData);
    }
    else
    {
        /* sip4 : calling the 'Drv' with port group == 'MG' */
        return prvCpssDrvHwPpPortGroupGetRegField(devNum,mgNum,regAddr,fieldOffset,fieldLength,fieldData);
    }
}

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
)
{
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* sip6 : calling the 'non-Drv' for the MG[0..max] */
        /* with port group == 'MG' */
        return prvCpssHwPpPortGroupSetRegField(devNum,mgNum,regAddr,fieldOffset,fieldLength,fieldData);
    }
    else
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* sip5 : calling the 'Drv' for the specific MG */
        return prvCpssDrvHwPpMgSetRegField(devNum, mgNum, regAddr, fieldOffset,fieldLength,fieldData);
    }
    else
    {
        /* sip4 : calling the 'Drv' with port group == 'MG' */
        return prvCpssDrvHwPpPortGroupSetRegField(devNum,mgNum,regAddr,fieldOffset,fieldLength,fieldData);
    }
}

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
)
{
    GT_STATUS   rc;
    GT_U32  busyIterator; /* busy wait iterator */
    GT_U32  regValue;     /* current register value*/

    busyIterator = 0;
    /* use "busy" wait */
    do
    {
        /* query the needed bit in the needed register in the specified port group*/
        rc = prvCpssHwPpMgReadReg(devNum, mgNum, regAddr, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* check that number of iteration not over the limit */
        PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,(busyIterator++));
    }
    while((regValue & regMask) && (onlyCheck == GT_FALSE));

    if(regValue & regMask)
    {
        /* the case of (onlyCheck == GT_FALSE) would have already returned GT_TIMEOUT
            from the macro of PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS */
        return /* do not register as error (up to the caller to register error if needed) */ GT_BAD_STATE;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS   rc;
    GT_U32  origValue,newValue;

    if(mask != 0xFFFFFFFF)
    {
        rc = prvCpssHwPpMgReadReg(devNum,mgNum,regAddr,&origValue);
        if(rc != GT_OK)
        {
            return rc;
        }
        newValue = (origValue & ~(mask)) | (value & mask);
    }
    else
    {
        newValue = value;
    }

    return prvCpssHwPpMgWriteReg(devNum,mgNum,regAddr,newValue);
}

