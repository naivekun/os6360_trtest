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
* @file cpssDxChBrgPrvEdgeVlan.h
*
* @brief cpss Dx-Ch implementation for Prestera Private Edge VLANs.
*
*
* @version   9
********************************************************************************
*/
#ifndef __cpssDxChBrgPrvEdgeVlanh
#define __cpssDxChBrgPrvEdgeVlanh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal cpssDxChBrgPrvEdgeVlanEnable function
* @endinternal
*
* @brief   This function enables/disables the Private Edge VLAN on
*         specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE to  the feature,
*                                      GT_FALSE to disable the feature
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPrvEdgeVlanEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgPrvEdgeVlanEnableGet function
* @endinternal
*
* @brief   Get status of the Private Edge VLAN on
*         specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE to enable the feature,
*                                      GT_FALSE to disable the feature
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgPrvEdgeVlanEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChBrgPrvEdgeVlanPortEnable function
* @endinternal
*
* @brief   Enable/Disable a specified port to operate in Private Edge VLAN mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number to set state.
* @param[in] enable                   - GT_TRUE for enabled, GT_FALSE for disabled
* @param[in] dstPort                  - the destination "uplink" port to which all traffic
*                                      received on srcPort&srcDev is forwarded.  This parameter
*                                      is ignored if disabling the mode.
* @param[in] dstHwDev                 - the destination "uplink" physical HW device to which all
*                                      traffic received on srcPort&srcDev is forwarded.  This
*                                      parameter is ignored if disabling the mode.
* @param[in] dstTrunk                 - the destination is a trunk member. This parameter
*                                      is ignored if disabling the mode.
*                                      GT_TRUE - dstPort hold value of trunkId , and dstHwDev is
*                                      ignored
*                                      GT_FALSE - dstPort hold value of port in device dstHwDev
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad portNum or devNum or dstPort or dstHwDev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPrvEdgeVlanPortEnable
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable,
    IN GT_PORT_NUM      dstPort,
    IN GT_HW_DEV_NUM    dstHwDev,
    IN GT_BOOL          dstTrunk
);

/**
* @internal cpssDxChBrgPrvEdgeVlanPortEnableGet function
* @endinternal
*
* @brief   Get enabling/disabling status to operate in Private Edge VLAN mode
*         and destination parameters for a specified port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number to set state.
*
* @param[out] enablePtr                - pointer to status of enabling/disabling a specified port
*                                      to operate in Private Edge VLAN mode.
*                                      GT_TRUE for enabled,
*                                      GT_FALSE for disabled
* @param[out] dstPortPtr               - pointer to the destination "uplink" port to which
*                                      all trafficreceived on srcPort&srcDev is forwarded.
*                                      This parameter is ignored if disabling the mode.
* @param[out] dstHwDevPtr              - pointer to the destination "uplink" physical HW device to
*                                      which all traffic received on srcPort&srcDev is forwarded.
*                                      This parameter is ignored if disabling the mode.
* @param[out] dstTrunkPtr              - pointer to the destination is a trunk member.
*                                      This parameter is ignored if disabling the mode.
*                                      GT_TRUE - dstPort hold value of trunkId , and dstHwDev is
*                                      ignored
*                                      GT_FALSE - dstPort hold value of port in device dstHwDev
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad portNum or devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPrvEdgeVlanPortEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr,
    OUT GT_PORT_NUM     *dstPortPtr,
    OUT GT_HW_DEV_NUM   *dstHwDevPtr,
    OUT GT_BOOL         *dstTrunkPtr
);

/**
* @internal cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet function
* @endinternal
*
* @brief   Enable/Disable per port forwarding control taffic to Private Edge
*         VLAN Uplink. PVE port can be disabled from trapping or mirroring
*         bridged packets to the CPU. In this case, as long as the packet is not
*         assigned a HARD DROP or SOFT DROP, the packet is unconditionally
*         assigned a FORWARD command with the ingress ports configured
*         PVE destination (PVLAN Uplink).
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port to set
* @param[in] enable                   - GT_TRUE  - forward control traffic to PVE Uplink
*                                      GT_FALSE - not forward control traffic to PVE Uplink,
*                                      trapped or mirrored to CPU packets are sent to
*                                      CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet function
* @endinternal
*
* @brief   Get Enable state per Port forwarding control taffic to Private Edge
*         VLAN Uplink. PVE port can be disabled from trapping or mirroring
*         bridged packets to the CPU. In this case, as long as the packet is not
*         assigned a HARD DROP or SOFT DROP, the packet is unconditionally
*         assigned a FORWARD command with the ingress ports configured
*         PVE destination (PVLAN Uplink).
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port to set.
*
* @param[out] enablePtr                GT_TRUE  - forward control traffic to PVE Uplink
*                                      GT_FALSE - not forward control traffic to PVE Uplink,
*                                      trapped or mirrored to CPU packets are sent to
*                                      CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgPrvEdgeVlanh */

