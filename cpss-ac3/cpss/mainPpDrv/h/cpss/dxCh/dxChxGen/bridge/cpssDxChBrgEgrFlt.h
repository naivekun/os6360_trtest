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
* @file cpssDxChBrgEgrFlt.h
*
* @brief Egress filtering facility DxCh cpss implementation
*
* @version   18
********************************************************************************
*/
#ifndef __cpssDxChBrgEgrFlth
#define __cpssDxChBrgEgrFlth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT
 *
 * @brief This enum defines ePort VLAN Egress Filtering VID selection mode.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef enum{

    /** Tag1 VLAN ID is used */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_TAG1_E,

    /** The eVLAN is used */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_EVLAN_E,

    /** The original VLAN is used */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ORIG_VLAN_E

} CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT
 *
 * @brief This enum defines ePort VLAN Egress Filtering Table access mode.
 * The table is an array of 2^20 bits seen as matrix with ePort and mappedVid
 * coordinates. The described mode is the matrix dimensions.
 * Each bit of the matrix means "port is a VLAN member".
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef enum{

    /** 256 ports 4K VLANs */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_256_E,

    /** 512 ports 2K VLANs */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_512_E,

    /** 1K ports  1K VLANs */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_1K_E,

    /** 2K ports  512 VLANs */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_2K_E,

    /** 4K ports  256 VLANs */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_4K_E,

    /** 8K ports  128 VLANs */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_8K_E,

    /** 16K ports 64 VLANs */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_16K_E,

    /** 32K ports 32 VLANs */
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_32K_E

} CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT
 *
 * @brief This enum defines Physical Port Link Status Mask.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef enum{

    /** Packets to this physical port are not filtered. */
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E = GT_FALSE,

    /** Packets to this physical port are always filtered. */
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E = GT_TRUE,

    /** @brief Filtering is performed based on the port link state in.
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X.
     */
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_LINK_STATE_BASED_E,

} CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT;

/**
* @internal cpssDxChBrgPortEgrFltUnkEnable function
* @endinternal
*
* @brief   Enable/Disable egress Filtering for bridged Unknown Unicast packets
*         on the specified egress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - CPU port, port number
* @param[in] enable                   - GT_TRUE: Unknown Unicast packets are filtered and are
*                                      not forwarded to this port.
*                                      GT_FALSE: Unknown Unicast packets are not filtered and may
*                                      be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgrFltUnkEnable
(
    IN GT_U8            dev,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgPortEgrFltUnkEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         for bridged Unknown Unicast packets on the specified egress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - CPU port, port number
*
* @param[out] enablePtr                - points to (enable/disable) bridged unknown unicast packets filtering
*                                      GT_TRUE:  Unknown Unicast packets are filtered and are
*                                      not forwarded to this port.
*                                      GT_FALSE: Unknown Unicast packets are not filtered and may
*                                      be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgrFltUnkEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgPortEgrFltUregMcastEnable function
* @endinternal
*
* @brief   Enable/Disable egress Filtering for bridged Unregistered Multicast packets
*         on the specified egress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - CPU port, port number
* @param[in] enable                   - GT_TRUE: Unregistered Multicast packets are filtered and
*                                      are not forwarded to this port.
*                                      GT_FALSE:Unregistered Multicast packets are not filtered
*                                      and may be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgrFltUregMcastEnable
(
    IN GT_U8            dev,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgPortEgrFltUregMcastEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         for bridged Unregistered Multicast packets on the specified egress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - CPU port, port number
*
* @param[out] enablePtr                - points to (enable/disable) bridged unregistered multicast packets filtering
*                                      GT_TRUE:  Unregistered Multicast packets are filtered and
*                                      are not forwarded to this port.
*                                      GT_FALSE: Unregistered Multicast packets are not filtered
*                                      and may be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgrFltUregMcastEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgVlanEgressFilteringEnable function
* @endinternal
*
* @brief   Enable/Disable VLAN Egress Filtering on specified device for Bridged
*         Known Unicast packets.
*         If enabled the VLAN egress filter verifies that the egress port is a
*         member of the VID assigned to the packet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] enable                   - GT_TRUE: VLAN egress filtering verifies that the egress
*                                      port is a member of the packet's VLAN classification
*                                      GT_FLASE: the VLAN egress filtering check is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note VLAN egress filtering is required by 802.1Q, but if desired, this
*       mechanism can be disabled, thus allowing "leaky VLANs".
*
*/
GT_STATUS cpssDxChBrgVlanEgressFilteringEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgVlanEgressFilteringEnableGet function
* @endinternal
*
* @brief   This function gets the VLAN Egress Filtering current state (enable/disable)
*         on specified device for Bridged Known Unicast packets.
*         If enabled the VLAN egress filter verifies that the egress port is a
*         member of the VID assigned to the packet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
*
* @param[out] enablePtr                - points to (enable/disable) bridged known unicast packets filtering
*                                      GT_TRUE:   VLAN egress filtering verifies that the egress
*                                      port is a member of the packet's VLAN classification
*                                      GT_FLASE: the VLAN egress filtering check is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgRoutedUnicastEgressFilteringEnable function
* @endinternal
*
* @brief   Enable/Disable VLAN egress filtering on Routed Unicast packets.
*         If disabled, the destination port may or may not be a member of the VLAN.
*         If enabled, the VLAN egress filter verifies that the egress port is a
*         member of the VID assigned to the packet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] enable                   - GT_TRUE: Egress filtering is enabled
*                                      GT_FLASE: Egress filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgRoutedUnicastEgressFilteringEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);


/**
* @internal cpssDxChBrgRoutedUnicastEgressFilteringEnableGet function
* @endinternal
*
* @brief   This function gets the VLAN Egress Filtering current state (enable/disable)
*         on Routed Unicast packets.
*         If disabled, the destination port may or may not be a member of the VLAN.
*         If enabled, the VLAN egress filter verifies that the egress port is a
*         member of the VID assigned to the packet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
*
* @param[out] enablePtr                - points to (enable/disable) routed unicast packets filtering
*                                      GT_TRUE:  Egress filtering is enabled
*                                      GT_FLASE: Egress filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgRoutedUnicastEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgRoutedSpanEgressFilteringEnable function
* @endinternal
*
* @brief   Enable/Disable STP egress Filtering on Routed Packets.
*         If disabled the packet may be forwarded to its egress port,
*         regardless of its Span State.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] enable                   - GT_TRUE: Span state filtering is enabled
*                                      GT_FLASE: Span state filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgRoutedSpanEgressFilteringEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgRoutedSpanEgressFilteringEnableGet function
* @endinternal
*
* @brief   This function gets the STP egress Filtering current state (enable/disable)
*         on Routed packets.
*         If disabled the packet may be forwarded to its egress port,
*         regardless of its Span State.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
*
* @param[out] enablePtr                - points to (enable/disable) routed packets filtering
*                                      GT_TRUE:  Span state filtering is enabled
*                                      GT_FLASE: Span state filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgRoutedSpanEgressFilteringEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
);


/**
* @internal cpssDxChBrgPortEgrFltUregBcEnable function
* @endinternal
*
* @brief   Enables or disables egress filtering of unregistered broadcast packets.
*         Unregistered broadcast packets are:
*         - packets with destination MAC ff-ff-ff-ff-ff-ff
*         - destination MAC address lookup not finds matching entry
*         - packets were not routed
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  -  filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are dropped
*                                      GT_FALSE - disable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are not dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgrFltUregBcEnable
(
    IN GT_U8            dev,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgPortEgrFltUregBcEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         of unregistered broadcast packets.
*         Unregistered broadcast packets are:
*         - packets with destination MAC ff-ff-ff-ff-ff-ff
*         - destination MAC address lookup not finds matching entry
*         - packets were not routed
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - points to (enable/disable) unregistered broadcast packets filtering
*                                      GT_TRUE  - enable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are dropped
*                                      GT_FALSE - disable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are not dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgrFltUregBcEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgPortEgressMcastLocalEnable function
* @endinternal
*
* @brief   Enable/Disable sending Multicast packets back to its source
*         port on the local device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - Multicast packets may be sent back to
*                                      their source port on the local device.
*                                      GT_FALSE - Multicast packets are not sent back to
*                                      their source port on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For xCat and above devices to enable local switching of Multicast,
*       unknown Unicast, and Broadcast traffic, both egress port configuration
*       and the field in the VLAN entry (by function
*       cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*/
GT_STATUS cpssDxChBrgPortEgressMcastLocalEnable
(
    IN GT_U8            dev,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgPortEgressMcastLocalEnableGet function
* @endinternal
*
* @brief   This function gets current state (enable/disable) for sending
*         Multicast packets back to its source port on the local device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - points to (enable/disable) sending Multicast packets back to its source
*                                      GT_TRUE  - Multicast packets may be sent back to
*                                      their source port on the local device.
*                                      GT_FALSE - Multicast packets are not sent back to
*                                      their source port on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgressMcastLocalEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgPortEgrFltIpMcRoutedEnable function
* @endinternal
*
* @brief   Enable/Disable egress filtering for IP Multicast Routed packets
*         on the specified egress port.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE: IP Multicast Routed packets are filtered and are
*                                      not forwarded to this port.
*                                      GT_FALSE: IP Multicast Routed packets are not filtered and may
*                                      be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgrFltIpMcRoutedEnable
(
    IN GT_U8            dev,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgPortEgrFltIpMcRoutedEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         for IP Multicast Routed packets on the specified egress port.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - points to (enable/disable) IP Multicast Routed packets filtering
*                                      GT_TRUE:  IP Multicast Routed packets are filtered and are
*                                      not forwarded to this port.
*                                      GT_FALSE: IP Multicast Routed packets are not filtered and may
*                                      be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgPortEgrFltIpMcRoutedEnableGet
(
    IN  GT_U8           dev,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgEgrFltVlanPortFilteringEnableSet function
* @endinternal
*
* @brief   Enable/disable per ePort , the physical Port Vlan egress filtering.
*         (The physical port that is associated with the ePort)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - disable ePort VLAN Egress Filtering.
*                                      GT_TRUE  - enable ePort VLAN Egress Filtering.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortFilteringEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgEgrFltVlanPortFilteringEnableGet function
* @endinternal
*
* @brief   Get Enable/disable per ePort , the physical Port Vlan egress filtering.
*         (The physical port that is associated with the ePort)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - disable ePort VLAN Egress Filtering.
*                                      GT_TRUE  - enable ePort VLAN Egress Filtering.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortFilteringEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal cpssDxChBrgEgrFltVlanPortVidSelectModeSet function
* @endinternal
*
* @brief   Set ePort VLAN Egress Filtering VID selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vidSelectMode            - VID selection mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortVidSelectModeSet
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT  vidSelectMode
);

/**
* @internal cpssDxChBrgEgrFltVlanPortVidSelectModeGet function
* @endinternal
*
* @brief   Get ePort VLAN Egress Filtering VID selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] vidSelectModePtr         - (pointer to) VID selection mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on unexpected HW contents
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortVidSelectModeGet
(
    IN  GT_U8                                                devNum,
    OUT CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT  *vidSelectModePtr
);

/**
* @internal cpssDxChBrgEgrFltVlanPortVidMappingSet function
* @endinternal
*
* @brief   Set ePort VLAN Egress Filtering VID Mapping Table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vid                      - source VLAN Id.
* @param[in] vidIndex                 - target VLAN Id.
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortVidMappingSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           vid,
    IN  GT_U32           vidIndex
);

/**
* @internal cpssDxChBrgEgrFltVlanPortVidMappingGet function
* @endinternal
*
* @brief   Get ePort VLAN Egress Filtering VID Mapping Table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vid                      - source VLAN Id.
*
* @param[out] vidIndexPtr              - (pointer to)target VLAN Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortVidMappingGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           vid,
    OUT GT_U32           *vidIndexPtr
);

/**
* @internal cpssDxChBrgEgrFltVlanPortAccessModeSet function
* @endinternal
*
* @brief   Set ePort VLAN Egress Filtering Table access mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] fltTabAccessMode         - Filtering Table access mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortAccessModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT fltTabAccessMode
);

/**
* @internal cpssDxChBrgEgrFltVlanPortAccessModeGet function
* @endinternal
*
* @brief   Get ePort VLAN Egress Filtering Table access mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] fltTabAccessModePtr      - (pointer to)
*                                      Filtering Table access mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortAccessModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT *fltTabAccessModePtr
);

/**
* @internal cpssDxChBrgEgrFltVlanPortMemberSet function
* @endinternal
*
* @brief   Set ePort VLAN Egress Filter Table ePort-VLAN membership.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] vidIndex                 - VID index (after mapping)
* @param[in] portNum                  - egress port number
* @param[in] isMember                 - GT_FALSE - ePort is not VLAN member, packet will be filtered.
*                                      GT_TRUE  - ePort is VLAN member, packet will not be filtered.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - portNum is out of range based on the
*                                       egress evlan access mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortMemberSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           vidIndex,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          isMember
);

/**
* @internal cpssDxChBrgEgrFltVlanPortMemberGet function
* @endinternal
*
* @brief   Get ePort VLAN Egress Filter Table ePort-VLAN membership.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] vidIndex                 - VID index (after mapping)
* @param[in] portNum                  - egress port number
*
* @param[out] isMemberPtr              - (pointer to)
*                                      GT_FALSE - ePort is not VLAN member, packet will be filtered.
*                                      GT_TRUE  - ePort is VLAN member, packet will not be filtered.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - portNum is out of range based on the
*                                       egress evlan access mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanPortMemberGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           vidIndex,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *isMemberPtr
);


/**
* @internal cpssDxChBrgEgrMeshIdConfigurationSet function
* @endinternal
*
* @brief   Set configuration of Mesh ID for the egress ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: MESH ID filtering is not enabled
*                                      GT_TRUE:  MESH ID filtering is enabled
* @param[in] meshIdOffset             - The MESH ID assigned to a packet is conveyed in the SrcID
*                                      assigned to the packet.
*                                      This configuration specifies the location of the LSB of
*                                      the MESH ID in the SrcID field assigned to a packet
*                                      (APPLICABLE RANGES: 0..11)
* @param[in] meshIdSize               - Specifies the number of bits that are used for
*                                      the MESH ID in the SrcID field.
*                                      Together with <MESH ID offset>, the MESH ID assigned to
*                                      a packet can be extracted:
*                                      Packet's MESH ID = <SST ID> (<MESH ID size> - 1 +
*                                      <MESH ID offset> : <MESH ID offset>)
*                                      (APPLICABLE RANGES: 1..4)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on wrong meshIdOffset and meshIdSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrMeshIdConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable,
    IN GT_U32                   meshIdOffset,
    IN GT_U32                   meshIdSize
);

/**
* @internal cpssDxChBrgEgrMeshIdConfigurationGet function
* @endinternal
*
* @brief   Get configuration of Mesh ID for the egress ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: MESH ID filtering is not enabled
*                                      GT_TRUE:  MESH ID filtering is enabled
* @param[out] meshIdOffsetPtr          - (pointer to) The MESH ID assigned to a packet is conveyed
*                                      in the SrcID assigned to the packet.
*                                      This configuration specifies the location of the LSB of
*                                      the MESH ID in the SrcID field assigned to a packet
*                                      (APPLICABLE RANGES: 0..11)
* @param[out] meshIdSizePtr            - (pointer to) Specifies the number of bits that are used
*                                      for the MESH ID in the SrcID field.
*                                      Together with <MESH ID offset>, the MESH ID assigned to
*                                      a packet can be extracted:
*                                      Packet's MESH ID = <SST ID> (<MESH ID size> - 1 +
*                                      <MESH ID offset> : <MESH ID offset>)
*                                      (APPLICABLE RANGES: 1..4)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrMeshIdConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_U32                   *meshIdOffsetPtr,
    OUT GT_U32                   *meshIdSizePtr
);


/**
* @internal cpssDxChBrgEgrPortMeshIdSet function
* @endinternal
*
* @brief   Set the Mesh ID number for Source-based (Split Horizon)
*         Filtering per ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] meshId                   - The Egress Mesh ID number for Source-based (Split Horizon)
*                                      Filtering per ePort. (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_OUT_OF_RANGE          - on wrong meshId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrPortMeshIdSet
(
    IN GT_U8           devNum,
    IN GT_PORT_NUM     portNum,
    IN GT_U32          meshId
);

/**
* @internal cpssDxChBrgEgrPortMeshIdGet function
* @endinternal
*
* @brief   Get the Mesh ID number for Source-based (Split Horizon)
*         Filtering per ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] meshIdPtr                (pointer to) The Egress Mesh ID number for Source-based
*                                      (Split Horizon) Filtering per ePort.(APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrPortMeshIdGet
(
    IN GT_U8           devNum,
    IN GT_PORT_NUM     portNum,
    OUT GT_U32         *meshIdPtr
);

/**
* @internal cpssDxChBrgEgrFltPortVid1FilteringEnableSet function
* @endinternal
*
* @brief   Enable/Disable port VID1 Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - disable filtering
*                                      GT_TRUE  - enable filtering
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltPortVid1FilteringEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgEgrFltPortVid1FilteringEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of port VID1 Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - the filtering is enabled
*                                      GT_TRUE  - the filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltPortVid1FilteringEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal cpssDxChBrgEgrFltPortVid1Set function
* @endinternal
*
* @brief   Set port associated VID1. Relevant to egress VID1 Filtering per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] vid1                     - VID1 associated with port (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on vid1 value out of an applicable range
*/
GT_STATUS cpssDxChBrgEgrFltPortVid1Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U16           vid1
);

/**
* @internal cpssDxChBrgEgrFltPortVid1Get function
* @endinternal
*
* @brief   Get port associated VID1. Relevant to egress VID1 Filtering per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] vid1Ptr                  - (pointer to) VID1 associated with port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL vid1Ptr
*/
GT_STATUS cpssDxChBrgEgrFltPortVid1Get
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_U16           *vid1Ptr
);

/**
* @internal cpssDxChBrgEgrFltPortLinkEnableSet function
* @endinternal
*
* @brief   Enable/disable egress Filtering for 'Link state' on specified port on specified device.
*         Set the port as 'force link up' (no filtering) , as 'force link down' (filtering),
*         or based on the port link state
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*                                      portLinkState
*                                      - physical port link status state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltPortLinkEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState
);

/**
* @internal cpssDxChBrgEgrFltPortLinkEnableGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable/link state)
*         for 'Link state' on specified port on specified device.
*         is the port as 'force link up' (no filtering) , as 'force link down' (filtering),
*         or based on the port link state
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum, Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] portLinkStatusStatePtr
*                                      - (pointer to) physical port link status state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltPortLinkEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT * portLinkStatusStatePtr
);

/**
* @internal cpssDxChBrgEgrFltVlanEPortFilteringEnableSet function
* @endinternal
*
* @brief   Enable/Disable ePort VLAN Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress ePort number
* @param[in] enable                   - GT_FALSE - disable ePort VLAN Egress Filtering.
*                                      GT_TRUE  - enable ePort VLAN Egress Filtering.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChBrgEgrFltVlanEPortFilteringEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgEgrFltVlanEPortFilteringEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of ePort VLAN Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress ePort number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - disable ePort VLAN Egress Filtering.
*                                      GT_TRUE  - enable ePort VLAN Egress Filtering.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEgrFltVlanEPortFilteringEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgEgrFlth */

