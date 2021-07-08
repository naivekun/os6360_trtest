/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoLion2TrunkWa.h
*
* @brief Includes configurations of trunks for cut through WA.
*
* @version   2
********************************************************************************
*/
#ifndef __appDemoLion2TrunkWah
#define __appDemoLion2TrunkWah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Uplink ports number */
#define APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS             6
/* Relay ports number */
#define APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS             24

/**
* @internal appDemoLion2TrunkWaInit function
* @endinternal
*
* @brief   Initialization of trunk WA.
*         Set first trunk ID and first source ID for WA usage.
* @param[in] devNum                   - device number
* @param[in] firstReservedTrunkId     - first reserved trunk ID
* @param[in] firstReservedSourceId    - first reserved Source ID
* @param[in] uplinkPortsMembersArr[APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS] - uplink ports array
* @param[in] relayPortsMemebersArr[APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS] - relay ports array
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Application should reserve 7 sequential trunk IDs and 6 sequential source IDs
*
*/
GT_STATUS appDemoLion2TrunkWaInit
(
    IN GT_U8 devNum,
    IN GT_TRUNK_ID firstReservedTrunkId,
    IN GT_U32 firstReservedSourceId,
    IN GT_U32 uplinkPortsMembersArr[APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS],
    IN GT_U32 relayPortsMemebersArr[APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS]
);

/**
* @internal appDemoLion2TrunkCascadeTrunkPortsSet function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] portsMembersPtr          - (pointer to) local ports bitmap to be members of the
*                                      cascade trunk.
*                                      NULL - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      not-NULL - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are ports in the bitmap that not supported by
*                                       the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or number
*                                       of ports (in the bitmap) larger then the number of
*                                       entries in the 'Designated trunk table'
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
);

/**
* @internal appDemoLion2TrunkMembersSet function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as disabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of disabled members in the array.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when the sum of number of enabled members + number of
*                                       disabled members exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
);

/**
* @internal appDemoLion2TrunkMemberAdd function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
* @param[in] devNum                   - the device number on which to add member to the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to add to the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_FULL                  - trunk already contains maximum supported members
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal appDemoLion2TrunkMemberDisable function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
* @param[in] devNum                   - the device number on which to disable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to disable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMemberDisable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal appDemoLion2TrunkMemberEnable function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
* @param[in] devNum                   - the device number on which to enable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to enable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMemberEnable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal appDemoLion2TrunkMemberRemove function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal appDemoLion2TrunkUpdatedPortsGet function
* @endinternal
*
* @brief   Get added and removed ports for specified trunk
*
* @param[in] devNum                   - device number.
* @param[in] trunkId                  - trunk ID.
* @param[in] portsMembersPtr          - (pointer to) trunk member ports
*
* @param[out] addedToTrunkPortsBmpPtr  - (pointer to) added ports
* @param[out] removedFromTrunkPortsBmpPtr - (pointer to) removed ports
*/
GT_VOID appDemoLion2TrunkUpdatedPortsGet
(
    IN GT_U8        devNum,
    IN GT_TRUNK_ID  trunkId,
    IN CPSS_PORTS_BMP_STC * portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC * addedToTrunkPortsBmpPtr,
    OUT CPSS_PORTS_BMP_STC * removedFromTrunkPortsBmpPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoLion2TrunkWah */


