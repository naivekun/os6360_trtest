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
* @file cpssDxChLogicalTargetMapping.h
*
* @brief Logical Target mapping.
*
* @version   8
********************************************************************************
*/

#ifndef __cpssDxChlogicalTargetMappingh
#define __cpssDxChlogicalTargetMappingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>

/**
* @enum CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT
 *
 * @brief Mode in which the Logical Ports Egress VLAN member table is
 * accessed, according to the amount of VLANs and Logical Ports
 * in the system.
*/
typedef enum{

    /** 4K VLANs and 32 Logical Ports in the system */
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_4K_VID_32_LP_E,

    /** 2K VLANs and 64 Logical Ports in the system */
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_2K_VID_64_LP_E,

    /** 1K VLANs and 128 Logical Ports in the system */
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_1K_VID_128_LP_E,

    /** 512 VLANs and 256 Logical Ports in the system */
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_512_VID_256_LP_E,

    /** 256 VLANs and 512 Logical Ports in the system */
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_256_VID_512_LP_E,

    /** 128 VLANs and 1K Logical Ports in the system */
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_128_VID_1K_LP_E,

    /** 64 VLANs and 2K Logical Ports in the system */
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_64_VID_2K_LP_E

} CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT;

/**
* @struct CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC
 *
 * @brief Defines the interface info
*/
typedef struct{

    /** Defines the interface info */
    CPSS_DXCH_OUTPUT_INTERFACE_STC outputInterface;

    /** @brief If set to GT_TRUE, apply Egress VLAN filtering
     *  according to LP Egress VLAN member table config.
     */
    GT_BOOL egressVlanFilteringEnable;

    /** @brief Enable/Disable the egress VLAN tag state for the
     *  passenger Ethernet packet for this Pseudowire.
     */
    GT_BOOL egressVlanTagStateEnable;

    /** @brief If <egressVlanTagStateEnable> is set to GT_TRUE,
     *  defines the egress VLAN tag state for the passenger
     *  Ethernet packet for this Pseudowire.
     */
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT egressVlanTagState;

    /** @brief Index to the Egress Logical Port TPID table,
     *  where each entry defines Tag0 TPID and Tag1 TPID.
     */
    GT_U32 egressTagTpidIndex;

    /** If set to GT_TRUE, override VID0 assignment based on target logical port. */
    GT_BOOL assignVid0Enable;

    /** The new VID0 assignment. Relevant only if <assignVid0Enable> is set. */
    GT_U16 vid0;

} CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC;

/**
* @internal cpssDxChLogicalTargetMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable logical port mapping feature on the specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE:  enable
*                                      GT_FALSE: disable .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLogicalTargetMappingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);


/**
* @internal cpssDxChLogicalTargetMappingEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status logical port mapping feature on the
*         specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - Pointer to the enable/disable state.
*                                      GT_TRUE : enable,
*                                      GT_FALSE: disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLogicalTargetMappingEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *enablePtr
);



/**
* @internal cpssDxChLogicalTargetMappingDeviceEnableSet function
* @endinternal
*
* @brief   Enable/disable a target device to be considered as a logical device
*         on the specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] logicalDevNum            - logical device number.
*                                      xCat (except revision C0), Lion, xCat2, Lion2: (APPLICABLE RANGES: 24..31)
*                                      in xCat starting from revision c0, xCat3: (APPLICABLE RANGES: 0..31)
* @param[in] enable                   - GT_TRUE:  enable
*                                      GT_FALSE: disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLogicalTargetMappingDeviceEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     logicalDevNum,
    IN  GT_BOOL   enable
);


/**
* @internal cpssDxChLogicalTargetMappingDeviceEnableGet function
* @endinternal
*
* @brief   Get Enable/disable status of target device to be considered as a logical device
*         on the specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] logicalDevNum            - logical device number.
*                                      xCat (except revision C0), Lion, xCat2, Lion2: (APPLICABLE RANGES: 24..31)
*                                      in xCat starting from revision c0, xCat3: (APPLICABLE RANGES: 0..31)
*
* @param[out] enablePtr                - Pointer to the  Enable/disable state.
*                                      GT_TRUE : enable,
*                                      GT_FALSE: disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLogicalTargetMappingDeviceEnableGet
(
    IN   GT_U8     devNum,
    IN  GT_U8      logicalDevNum,
    OUT  GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChLogicalTargetMappingTableEntrySet function
* @endinternal
*
* @brief   Set logical target mapping table entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] logicalDevNum            - logical device number.
*                                      xCat (except revision C0), Lion, xCat2, Lion2: (APPLICABLE RANGES: 24..31)
*                                      in xCat starting from revision c0, xCat3: (APPLICABLE RANGES: 0..31)
* @param[in] logicalPortNum           - logical port number.
*                                      (APPLICABLE RANGES: 0..63)
* @param[in] logicalPortMappingTablePtr - points to logical Port Mapping  entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum, logicalPortNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on the memebers of virtual port entry struct out of range.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS cpssDxChLogicalTargetMappingTableEntrySet
(
    IN GT_U8                           devNum,
    IN GT_U8                           logicalDevNum,
    IN GT_U8                           logicalPortNum,
    IN CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  *logicalPortMappingTablePtr
);

/**
* @internal cpssDxChLogicalTargetMappingTableEntryGet function
* @endinternal
*
* @brief   Get logical target mapping table entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] logicalDevNum            - logical device number.
*                                      xCat (except revision C0), Lion, xCat2, Lion2: (APPLICABLE RANGES: 24..31)
*                                      in xCat starting from revision c0, xCat3: (APPLICABLE RANGES: 0..31)
* @param[in] logicalPortNum           - logical port number.
*                                      (APPLICABLE RANGES: 0..63)
*
* @param[out] logicalPortMappingTablePtr - points to logical Port Mapping  entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum, logicalPortNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on the memebers of virtual port entry struct out of range.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS cpssDxChLogicalTargetMappingTableEntryGet
(
    IN GT_U8                            devNum,
    IN GT_U8                            logicalDevNum,
    IN GT_U8                            logicalPortNum,
    OUT CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  *logicalPortMappingTablePtr
);

/**
* @internal cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet function
* @endinternal
*
* @brief   This function sets the mode in which the Logical Port Egress VLAN member
*         table is accessed, according to the amount of VIDs and Logical Ports in
*         the system.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] mode                     - value to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeSet
(
    IN GT_U8                                                                  devNum,
    IN CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT    mode
);

/**
* @internal cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet function
* @endinternal
*
* @brief   This function gets the mode in which the Logical Port Egress VLAN member
*         table is accessed, according to the amount of VIDs and Logical Ports in
*         the system.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] modePtr                  - (pointer to) value to get.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressVlanMemberAccessModeGet
(
    IN  GT_U8                                                                devNum,
    OUT CPSS_DXCH_LOGICAL_TARGET_MAPPING_EGRESS_VLAN_MEMBER_ACCESS_MODE_ENT *modePtr
);

/**
* @internal cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet function
* @endinternal
*
* @brief   This function sets the number of packets dropped due to Logical Port
*         Egress VLAN Filtering.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] counter                  - value to set counter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterSet
(
    IN GT_U8    devNum,
    IN GT_U32   counter
);

/**
* @internal cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet function
* @endinternal
*
* @brief   This function gets the number of packets dropped due to Logical Port
*         Egress VLAN Filtering.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] counterPtr               - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressVlanFilteringDropCounterGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *counterPtr
);

/**
* @internal cpssDxChLogicalTargetMappingEgressMappingEntrySet function
* @endinternal
*
* @brief   Function sets the mapping of any discrete VIDs of the 4K VLAN range,
*         into a continuous range of VIDs.
*         Used to accommodate the cases where the 12-bit VLAN-ID must be mapped
*         into a smaller internal VID index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Mapping Table.
*                                      (Applicable Range 0..4095)
* @param[in] vlanIdIndex              - VLAN id index, use as value in the Mapping Table.
*                                      (Applicable Range 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressMappingEntrySet
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN GT_U32                       vlanIdIndex
);

/**
* @internal cpssDxChLogicalTargetMappingEgressMappingEntryGet function
* @endinternal
*
* @brief   Function gets the mapping of any discrete VIDs of the 4K VLAN range,
*         into a continuous range of VIDs.
*         Used to accommodate the cases where the 12-bit VLAN-ID must be mapped
*         into a smaller internal VID index.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Mapping Table.
*                                      (Applicable Range 0..4095)
*
* @param[out] vlanIdIndexPtr           - (pointer to) VLAN id index, use as value in
*                                      the Mapping Table. (Applicable Range 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressMappingEntryGet
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    OUT GT_U32                      *vlanIdIndexPtr
);

/**
* @internal cpssDxChLogicalTargetMappingEgressMemberSet function
* @endinternal
*
* @brief   Function sets whether target logical port is a member of a given VLAN.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] vlanIdIndex              - VLAN id index, use as value in the Mapping Table.
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] logicalDevNum            - logical target device number. (APPLICABLE RANGES: 0..31)
* @param[in] logicalPortNum           - logical target port number. (APPLICABLE RANGES: 0..63)
* @param[in] isMember                 - whether target logical port is member of a given VLAN.
*                                      GT_TRUE: target logical port is member of a given VLAN
*                                      GT_FALSE: target logical port isn't member of a given VLAN
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressMemberSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       vlanIdIndex,
    IN GT_U8                        logicalDevNum,
    IN GT_U8                        logicalPortNum,
    IN GT_BOOL                      isMember
);

/**
* @internal cpssDxChLogicalTargetMappingEgressMemberGet function
* @endinternal
*
* @brief   Function gets whether target logical port is a member of a given VLAN.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] vlanIdIndex              - VLAN id index, use as value in the Mapping Table.
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] logicalDevNum            - logical target device number. (APPLICABLE RANGES: 0..31)
* @param[in] logicalPortNum           - logical target port number. (APPLICABLE RANGES: 0..63)
*
* @param[out] isMemberPtr              - (Pointer to) whether target logical port is member
*                                      of a give VLAN.
*                                      GT_TRUE: target logical port is member of a given VLAN
*                                      GT_FALSE: target logical port is not member of a given VLAN
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressMemberGet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       vlanIdIndex,
    IN  GT_U8                        logicalDevNum,
    IN  GT_U8                        logicalPortNum,
    OUT GT_BOOL                     *isMemberPtr
);

/**
* @internal cpssDxChLogicalTargetMappingEgressMemberTableClear function
* @endinternal
*
* @brief   Function clears Logical Port Egress VLAN member table.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChLogicalTargetMappingEgressMemberTableClear
(
    IN GT_U8                        devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChlogicalTargetMapping */


