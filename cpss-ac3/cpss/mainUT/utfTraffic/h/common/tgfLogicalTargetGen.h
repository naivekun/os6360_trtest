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
* @file tgfLogicalTargetGen.h
*
* @brief Generic API for Logical Target Mapping
*
* @version   1
********************************************************************************
*/
#ifndef __tgfLogicalTargetGenh
#define __tgfLogicalTargetGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfCommon.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfBridgeGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/logicalTarget/cpssDxChLogicalTargetMapping.h>
#endif /* CHX_FAMILY */

/**
* @struct PRV_TGF_OUTPUT_INTERFACE_STC
 *
 * @brief Defines the interface info
*/

typedef struct
{
    GT_BOOL                 isTunnelStart;

    struct{
        PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT passengerPacketType;
        GT_U32                              ptr;
    }tunnelStartInfo;

    CPSS_INTERFACE_INFO_STC physicalInterface;
} PRV_TGF_OUTPUT_INTERFACE_STC;

/**
* @struct PRV_TGF_LOGICAL_TARGET_MAPPING_STC
 *
 * @brief Defines the interface info
*/
typedef struct{

    /** Defines the interface info */
    PRV_TGF_OUTPUT_INTERFACE_STC outputInterface;

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
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT egressVlanTagState;

    /** @brief Index to the Egress Logical Port TPID table,
     *  where each entry defines Tag0 TPID and Tag1 TPID.
     */
    GT_U32 egressTagTpidIndex;

    /** If set to GT_TRUE, override VID0 assignment based on target logical port. */
    GT_BOOL assignVid0Enable;

    /** The new VID0 assignment. Relevant only if <assignVid0Enable> is set. */
    GT_U16 vid0;

} PRV_TGF_LOGICAL_TARGET_MAPPING_STC;

/**
* @internal prvTgfLogicalTargetMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable logical port mapping feature on the specified device.
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
GT_STATUS prvTgfLogicalTargetMappingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);


/**
* @internal prvTgfLogicalTargetMappingEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status logical port mapping feature on the
*         specified device.
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
GT_STATUS prvTgfLogicalTargetMappingEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *enablePtr
);



/**
* @internal prvTgfLogicalTargetMappingDeviceEnableSet function
* @endinternal
*
* @brief   Enable/disable a target device to be considered as a logical device
*         on the specified device.
* @param[in] devNum                   - physical device number
* @param[in] logicalTargetDevNum      - logical target device number.
* @param[in] enable                   - GT_TRUE:  enable
*                                      GT_FALSE: disable .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfLogicalTargetMappingDeviceEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     logicalTargetDevNum,
    IN  GT_BOOL   enable
);


/**
* @internal prvTgfLogicalTargetMappingDeviceEnableGet function
* @endinternal
*
* @brief   Get Enable/disable status of target device to be considered as a logical device
*         on the specified device.
* @param[in] devNum                   - physical device number
* @param[in] logicalTargetDevNum      - logical target device number.
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
GT_STATUS prvTgfLogicalTargetMappingDeviceEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     logicalTargetDevNum,
    OUT  GT_BOOL   *enablePtr
);

/**
* @internal prvTgfLogicalTargetMappingTableEntrySet function
* @endinternal
*
* @brief   Set logical target mapping table entry.
*
* @param[in] devNum                   - physical device number.
* @param[in] logicalTargetDevNum      - logical traget device number.
* @param[in] logicalTargetPortNum     - logical target port number.
* @param[in] logicalTargetMappingEntryPtr - points to logical target Mapping  entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum, logicalPortNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on the memebers of virtual port entry struct out of range.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS prvTgfLogicalTargetMappingTableEntrySet
(
    IN GT_U8                                devNum,
    IN GT_U8                                logicalTargetDevNum,
    IN GT_U8                                logicalTargetPortNum,
    IN PRV_TGF_LOGICAL_TARGET_MAPPING_STC   *logicalTargetMappingEntryPtr
);

/**
* @internal prvTgfLogicalTargetMappingTableEntryGet function
* @endinternal
*
* @brief   Get logical target mapping table entry.
*
* @param[in] devNum                   - physical device number.
* @param[in] logicalTargetDevNum      - logical target device number.
* @param[in] logicalTargetPortNum     - logical target port number.
*
* @param[out] logicalTargetMappingEntryPtr - points to logical target Mapping  entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum, logicalPortNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on the memebers of virtual port entry struct out of range.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS prvTgfLogicalTargetMappingTableEntryGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                logicalTargetDevNum,
    IN GT_U8                                logicalTargetPortNum,
    OUT PRV_TGF_LOGICAL_TARGET_MAPPING_STC  *logicalTargetMappingEntryPtr
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfLogicalTargetGenh */


