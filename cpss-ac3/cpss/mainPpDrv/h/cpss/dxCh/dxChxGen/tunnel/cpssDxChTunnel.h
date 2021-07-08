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
* @file cpssDxChTunnel.h
*
* @brief CPSS tunnel declarations.
*
* @version   37
********************************************************************************
*/

#ifndef __cpssDxChTunnelh
#define __cpssDxChTunnelh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>

/**
* @internal cpssDxChTunnelStartEntrySet function
* @endinternal
*
* @brief   Set a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
* @param[in] tunnelType               - type of the tunnel
* @param[in] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS cpssDxChTunnelStartEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerArpTunnelStartLineIndex,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *configPtr
);

/**
* @internal cpssDxChTunnelStartEntryGet function
* @endinternal
*
* @brief   Get a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
*
* @param[out] tunnelTypePtr            - points to the type of the tunnel
* @param[out] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS cpssDxChTunnelStartEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             routerArpTunnelStartLineIndex,
    OUT  CPSS_TUNNEL_TYPE_ENT               *tunnelTypePtr,
    OUT  CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *configPtr
);

/**
* @internal cpssDxChTunnelStartPortGroupEntrySet function
* @endinternal
*
* @brief   Set a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
* @param[in] tunnelType               - type of the tunnel
* @param[in] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS cpssDxChTunnelStartPortGroupEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              routerArpTunnelStartLineIndex,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *configPtr
);

/**
* @internal cpssDxChTunnelStartPortGroupEntryGet function
* @endinternal
*
* @brief   Get a tunnel start entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] routerArpTunnelStartLineIndex - line index for the tunnel start entry
*                                      in the router ARP / tunnel start / NAT table (for NAT capable devices)
*
* @param[out] tunnelTypePtr            - points to the type of the tunnel
* @param[out] configPtr                - points to tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry(for NAT capable devices)
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS cpssDxChTunnelStartPortGroupEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN   GT_U32                             routerArpTunnelStartLineIndex,
    OUT  CPSS_TUNNEL_TYPE_ENT               *tunnelTypePtr,
    OUT  CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *configPtr
);

/**
* @internal cpssDxChEthernetOverMplsTunnelStartTaggingSet function
* @endinternal
*
* @brief   Set the global Tagging state for the Ethernet passenger packet to
*         be Tagged or Untagged.
*         The Ethernet passenger packet may have a VLAN tag added, removed,
*         or modified prior to its Tunnel Start encapsulation.
*         The Ethernet passenger packet is treated according to the following
*         modification modes:
*         - Transmit the passenger packet without any modifications to its VLAN
*         tagged state (i.e. if it arrived untagged, transmit untagged; if it
*         arrived tagged, transmit tagged)
*         - Transmit the passenger packet with an additional (nested) VLAN
*         tag regardless of whether it was received tagged or untagged
*         - Transmit the passenger packet tagged (i.e. if it arrived untagged,
*         a tag is added; if it arrived tagged it is transmitted tagged with
*         the new VID assignment)
*         - Transmit the passenger packet untagged (i.e. if it arrived tagged
*         it is transmitted untagged; if it arrived untagged it is
*         transmitted untagged)
*         Based on the global Ethernet passenger tag mode and the Ingress Policy
*         Action Nested VLAN Access mode, the following list indicates how the
*         Ethernet passenger is modified prior to its being encapsulated by the
*         tunnel header:
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 1
*         Ethernet passenger tagging is not modified, regardless of
*         whether it is tagged or untagged.
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 0
*         Transmit Untagged. NOTE: If the Ethernet passenger packet
*         is tagged, the tag is removed. If the Ethernet passenger
*         packet is untagged, the packet is not modified.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 1
*         A new tag is added to the Ethernet passenger packet,
*         regardless of whether it is tagged or untagged.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 0
*         Transmit Tagged. NOTE: If the Ethernet passenger packet
*         is untagged, a tag is added. If the Ethernet passenger
*         packet is tagged, the existing tag VID is set to the
*         packet VID assignment.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   -  / disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTaggingSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChEthernetOverMplsTunnelStartTaggingGet function
* @endinternal
*
* @brief   Set the global Tagging state for the Ethernet passenger packet to
*         be Tagged or Untagged.
*         The Ethernet passenger packet may have a VLAN tag added, removed,
*         or modified prior to its Tunnel Start encapsulation.
*         The Ethernet passenger packet is treated according to the following
*         modification modes:
*         - Transmit the passenger packet without any modifications to its VLAN
*         tagged state (i.e. if it arrived untagged, transmit untagged; if it
*         arrived tagged, transmit tagged)
*         - Transmit the passenger packet with an additional (nested) VLAN
*         tag regardless of whether it was received tagged or untagged
*         - Transmit the passenger packet tagged (i.e. if it arrived untagged,
*         a tag is added; if it arrived tagged it is transmitted tagged with
*         the new VID assignment)
*         - Transmit the passenger packet untagged (i.e. if it arrived tagged
*         it is transmitted untagged; if it arrived untagged it is
*         transmitted untagged)
*         Based on the global Ethernet passenger tag mode and the Ingress Policy
*         Action Nested VLAN Access mode, the following list indicates how the
*         Ethernet passenger is modified prior to its being encapsulated by the
*         tunnel header:
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 1
*         Ethernet passenger tagging is not modified, regardless of
*         whether it is tagged or untagged.
*         - <Tunnel Start tag> is Untagged & <Nested VLAN Access> = 0
*         Transmit Untagged. NOTE: If the Ethernet passenger packet
*         is tagged, the tag is removed. If the Ethernet passenger
*         packet is untagged, the packet is not modified.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 1
*         A new tag is added to the Ethernet passenger packet,
*         regardless of whether it is tagged or untagged.
*         - <Tunnel Start tag> is Tagged & <Nested VLAN Access> = 0
*         Transmit Tagged. NOTE: If the Ethernet passenger packet
*         is untagged, a tag is added. If the Ethernet passenger
*         packet is tagged, the existing tag VID is set to the
*         packet VID assignment.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - points to enable / disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTaggingGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChEthernetOverMplsTunnelStartTagModeSet function
* @endinternal
*
* @brief   Set the vlan tag mode of the passanger packet for an
*         Ethernet-over-xxx tunnel start packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tagMode                  - tunnel start ethernet-over-x vlan tag mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTagModeSet

(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT tagMode
);

/**
* @internal cpssDxChEthernetOverMplsTunnelStartTagModeGet function
* @endinternal
*
* @brief   Get the vlan tag mode of the passanger packet for an
*         Ethernet-over-xxx tunnel start packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] tagModePtr               - pointer to tunnel start ethernet-over-x vlan tag mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_STATE             - on bad state.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*
* @note Despite its name, this API is applicable to all Tunnel Starts, not just MPLS.
*
*/
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTagModeGet

(
    IN  GT_U8                                                devNum,
    OUT CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT *tagModePtr
);

/**
* @internal cpssDxChTunnelStartPassengerVlanTranslationEnableSet function
* @endinternal
*
* @brief   Controls Egress Vlan Translation of Ethernet tunnel start passengers.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE: Ethernet tunnel start passengers are egress vlan
*                                      translated, regardless of the VlanTranslationEnable
*                                      configuration.
*                                      GT_FALSE: Ethernet tunnel start passengers are to be egress
*                                      vlan translated in accordance to the
*                                      VlanTranslationEnable configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*/
GT_STATUS cpssDxChTunnelStartPassengerVlanTranslationEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChTunnelStartPassengerVlanTranslationEnableGet function
* @endinternal
*
* @brief   Gets the Egress Vlan Translation of Ethernet tunnel start passengers.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - points enable state.
*                                      GT_TRUE: Ethernet tunnel start passengers are egress vlan
*                                      translated, regardless of the VlanTranslationEnable
*                                      configuration.
*                                      GT_FALSE: Ethernet tunnel start passengers are to be egress
*                                      vlan translated in accordance to the
*                                      VlanTranslationEnable configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_HW_ERROR              - on hardware error.
*/
GT_STATUS cpssDxChTunnelStartPassengerVlanTranslationEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet function
* @endinternal
*
* @brief   This feature allows overriding the <total length> in the IP header.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         This API enables this feature per port.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] enable                   - GT_TRUE: Add offset to tunnel total length
*                                      GT_FALSE: Don't add offset to tunnel total length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                port,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet function
* @endinternal
*
* @brief   The function gets status of the feature which allows overriding the
*         <total length> in the IP header.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         This API enables this feature per port.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] enablePtr                - points to enable/disable additional offset to tunnel total length
*                                      GT_TRUE: Add offset to tunnel total length
*                                      GT_FALSE: Don't add offset to tunnel total length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                port,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet function
* @endinternal
*
* @brief   This API sets the value for <IP Tunnel Total Length Offset>.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] additionToLength         - Ip tunnel total length offset (6 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range values
*/
GT_STATUS cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              additionToLength
);

/**
* @internal cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet function
* @endinternal
*
* @brief   This API gets the value for <IP Tunnel Total Length Offset>.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] additionToLengthPtr      - (pointer to) Ip tunnel total length offset (6 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *additionToLengthPtr
);

/**
* @internal cpssDxChTunnelStartMplsPwLabelPushEnableSet function
* @endinternal
*
* @brief   Enable/Disable MPLS PW label push.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelPushEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChTunnelStartMplsPwLabelPushEnableGet function
* @endinternal
*
* @brief   Returns if MPLS PW Label Push is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelPushEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChTunnelStartMplsPwLabelSet function
* @endinternal
*
* @brief   Set the MPLS PW label value to push incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] label                    - MPLS  (20 bits)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or label.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               label
);

/**
* @internal cpssDxChTunnelStartMplsPwLabelGet function
* @endinternal
*
* @brief   Returns the MPLS PW label value to push incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] labelPtr                 - (pointer to) MPLS label
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *labelPtr
);

/**
* @internal cpssDxChTunnelStartMplsFlowLabelEnableSet function
* @endinternal
*
* @brief   Enable/Disable MPLS flow label per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsFlowLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChTunnelStartMplsFlowLabelEnableGet function
* @endinternal
*
* @brief   Returns if MPLS flow Label is enabled or disabled per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsFlowLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChTunnelStartMplsFlowLabelTtlSet function
* @endinternal
*
* @brief   Set the MPLS flow label TTL
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ttl                      - the TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsFlowLabelTtlSet
(
    IN GT_U8                devNum,
    IN GT_U8                ttl
);

/**
* @internal cpssDxChTunnelStartMplsFlowLabelTtlGet function
* @endinternal
*
* @brief   Get the MPLS flow label TTL
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] ttlPtr                   - (pointer to) the TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsFlowLabelTtlGet
(
    IN  GT_U8           devNum,
    OUT GT_U8           *ttlPtr
);

/**
* @internal cpssDxChTunnelStartMplsPwLabelExpSet function
* @endinternal
*
* @brief   Set the EXP value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] exp                      - EXP value (APPLICABLE RANGES: 0...7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or exp.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelExpSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               exp
);

/**
* @internal cpssDxChTunnelStartMplsPwLabelExpGet function
* @endinternal
*
* @brief   Returns the EXP value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] expPtr                   - (pointer to) EXP value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelExpGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *expPtr
);

/**
* @internal cpssDxChTunnelStartMplsPwLabelTtlSet function
* @endinternal
*
* @brief   Set the TTL value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ttl                      - TTL value (APPLICABLE RANGES: 0...255)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port or ttl.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelTtlSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               ttl
);

/**
* @internal cpssDxChTunnelStartMplsPwLabelTtlGet function
* @endinternal
*
* @brief   Returns the TTL value to push to the PW label incase that <MPLS PW label push enable> = Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ttlPtr                   - (pointer to) TTL value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwLabelTtlGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *ttlPtr
);

/**
* @internal cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet function
* @endinternal
*
* @brief   If enabled, a Label that is based on the source ePort of the packet is
*         pushed onto the packet as the inner-most label. This control is accessed
*         with the target ePort.
*         The Label to push is determined by:
*         <Source based MPLS Label>,
*         <Source based MPLS Label EXP> and
*         <Source based MPLS Label TTL> that are accessed with the source ePort.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet function
* @endinternal
*
* @brief   Returns if Push source based MPLS Label is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);
/**
* @internal cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet function
* @endinternal
*
* @brief   If enabled, a Label that is based on the packet's eVLAN is pushed onto the
*         packet after the 'Source based Label' (if exists)
*         This control is accessed with the target ePort.
*         The Label to push is determined by:
*         <Service-ID> that is accessed with the eVLAN and
*         the global configuration <eVLAN Based MPLS Label TTL>.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet function
* @endinternal
*
* @brief   Returns if Push eVLAN based MPLS Label is enabled or disabled.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X;
*         Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChTunnelStartMplsPwControlWordSet function
* @endinternal
*
* @brief   Sets a Pseudo Wire control word
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word index
*                                      Bobcat2; Caelum; Bobcat3; Aldrin2, Aldrin, AC3X: (APPLICABLE RANGES: 0..14)
*                                      xCat3: (APPLICABLE RANGES: 0..6)
* @param[in] value                    - the PW word to write
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwControlWordSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_U32          value
);

/**
* @internal cpssDxChTunnelStartMplsPwControlWordGet function
* @endinternal
*
* @brief   Gets a Pseudo Wire control word
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word index
*                                      Bobcat2; Caelum; Bobcat3; Aldrin2, Aldrin, AC3X: (APPLICABLE RANGES: 0..14)
*                                      xCat3: (APPLICABLE RANGES: 0..6)
*
* @param[out] valuePtr                 - (pointer to) the read PW word
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwControlWordGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_U32          *valuePtr
);

/**
* @internal cpssDxChTunnelStartMplsPwETreeEnableSet function
* @endinternal
*
* @brief   Enable/Disable E-TREE assignment for a specified PW Control Word.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
* @param[in] enable                   - enable/disable the E-TREE assignment-TREE assignment
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwETreeEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_BOOL         enable
);

/**
* @internal cpssDxChTunnelStartMplsPwETreeEnableGet function
* @endinternal
*
* @brief   Return whether E-TREE assignment is enabled for a specified PW Control
*         Word.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
*
* @param[out] enablePtr                - (pointer to) the E-TREE assignment enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartMplsPwETreeEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChTunnelStartHeaderTpidSelectSet function
* @endinternal
*
* @brief   Function sets index of TPID tunnel-start header
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] tpidEntryIndex           - TPID table entry index (APPLICABLE RANGES: 0...7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when a TS entry is processed (from ingress
*       pipe assignment or from egress ePort assignment).
*       Applicable to all TS types
*
*/
GT_STATUS cpssDxChTunnelStartHeaderTpidSelectSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  GT_U32               tpidEntryIndex
);

/**
* @internal cpssDxChTunnelStartHeaderTpidSelectGet function
* @endinternal
*
* @brief   Function gets index of TPID tunnel-start header
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID table entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when a TS entry is processed (from ingress
*       pipe assignment or from egress ePort assignment).
*       Applicable to all TS types
*
*/
GT_STATUS cpssDxChTunnelStartHeaderTpidSelectGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    OUT GT_U32               *tpidEntryIndexPtr
);

/**
* @internal cpssDxChTunnelStartGenProfileTableEntrySet function
* @endinternal
*
* @brief   This API configures Generic Tunnel-start Profile table entry.
*         It serves as logical extension to the Generic IPv4/6 Tunnel-Start
*         when the IPv4/6 tunnel header includes either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
* @param[in] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
GT_STATUS cpssDxChTunnelStartGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    IN CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC        *profileDataPtr
);

/**
* @internal cpssDxChTunnelStartGenProfileTableEntryGet function
* @endinternal
*
* @brief   This API retrieves Generic Tunnel-start Profile table entry content.
*         It serves as logical extension to the Generic IPv4/6 Tunnel-Start
*         when the IPv4/6 tunnel header includes either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
*
* @param[out] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
GT_STATUS cpssDxChTunnelStartGenProfileTableEntryGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *profileDataPtr
);




/**
* @internal cpssDxChTunnelStartPortGroupGenProfileTableEntrySet function
* @endinternal
*
* @brief   This API configures Generic Tunnel-start Profile table entry.
*         Generic Tunnel-start Profile table entry serves as logical extension to
*         the Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes
*         either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
GT_STATUS cpssDxChTunnelStartPortGroupGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           profileIndex,
    IN CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC        *profileDataPtr
);

/**
* @internal cpssDxChTunnelStartPortGroupGenProfileTableEntryGet function
* @endinternal
*
* @brief   This API retrieves Generic Tunnel-start Profile table entry content.
*         Generic Tunnel-start Profile table entry serves as logical extension to
*         the Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes
*         either a GRE or UDP header.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*                                      (APPLICABLE RANGES: 0...7).
*
* @param[out] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Profile table entry template generates up to 16 bytes of data
*       after the 4B GRE header or after the 8B UDP header.
*       There are 8 profiles available to use. But if application is
*       going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
*       (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
*       tunnel types take place , and these tunnels don't require profile data generation - empty
*       profile should be reserved. In this case, in order to save empty profiles number, it is
*       recommended to utilize profile 7 as well.
*       Empty profile should be configured to prevent insertion of template data into tunnel header:
*       templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E.
*
*/
GT_STATUS cpssDxChTunnelStartPortGroupGenProfileTableEntryGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           profileIndex,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *profileDataPtr
);

/**
* @internal cpssDxChTunnelStartEntryExtensionSet function
* @endinternal
*
* @brief   Set tunnel start entry extension value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tsExtension              - tunnel start entry extension value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEntryExtensionSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               tsExtension
);

/**
* @internal cpssDxChTunnelStartEntryExtensionGet function
* @endinternal
*
* @brief   Get tunnel start entry extension value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tsExtensionPtr           - (pointer to) tunnel start entry extension value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEntryExtensionGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *tsExtensionPtr
);

/**
* @internal cpssDxChTunnelStartEgessVlanTableServiceIdSet function
* @endinternal
*
* @brief   Set egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id, used as index in the Egress Vlan Translation Table.
*                                      (APPLICABLE RANGES:0..8191).
* @param[in] vlanServiceId            - tunnel start entry extension value (APPLICABLE RANGES: 0...FFFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEgessVlanTableServiceIdSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32               vlanServiceId
);


/**
* @internal cpssDxChTunnelStartEgessVlanTableServiceIdGet function
* @endinternal
*
* @brief   Get egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id, used as index in the Egress Vlan Translation Table.
*                                      (APPLICABLE RANGES:0..8191).
*
* @param[out] vlanServiceIdPtr         - (pointer to) vlan service Id.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEgessVlanTableServiceIdGet
(
    IN  GT_U8                devNum,
    IN  GT_U16               vlanId,
    OUT GT_U32               *vlanServiceIdPtr
);

/**
* @internal cpssDxChTunnelStartEcnModeSet function
* @endinternal
*
* @brief   Set Tunnel-start ECN mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] mode                  - ECN mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or ECN mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEcnModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT  mode
);

/**
* @internal cpssDxChTunnelStartEcnModeGet function
* @endinternal
*
* @brief   Get Tunnel-start ECN mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] modePtr              - (pointer to) ECN mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTunnelStartEcnModeGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT *modePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTunnelh */

