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
* @file cpssDxChBrgGen.h
*
* @brief CPSS DxCh Bridge Generic APIs.
*
*
* @version   44
********************************************************************************
*/
#ifndef __cpssDxChBrgGenh
#define __cpssDxChBrgGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

/**
* @enum CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT
 *
 * @brief Enumeration of Bridge Bypass mode.
*/
typedef enum{

    /** bypass the whole bridge module except the SA learning. */
    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E,

    /** bypass only the forwarding decision of the bridge. */
    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E

} CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT
 *
 * @brief Enumeration of byte-count modes for MTU check feature.
*/
typedef enum{

    /** L3 bytecount is used for MTU check. */
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E,

    /** L2 bytecount is used for MTU check. */
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E

} CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT;

/**
* @enum CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT
 *
 * @brief Enumeration of IPM classification mode.
*/
typedef enum{

    /** @brief Packet is considered an IPM packet (IPv6 or IPv4) only
     *  according to MAC address.
     */
    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_E,

    /** @brief Packet is considered an IPM packet (IPv6 or IPv4)
     *  according MAC address and ethertype.
     */
    CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_MAC_DA_AND_ETHERTYPE_E

} CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT;

/**
* @enum CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT
 *
 * @brief Enumeration of ARP broadcast command mode.
*/
typedef enum{

    /** set ARP broadcast command for all ports */
    CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E,

    /** set ARP broadcast command for all VLANs */
    CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E

} CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT;

/**
* @struct CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC
 *
 * @brief Describe rate limit parameters for DxCh devices.
*/
typedef struct{

    /** drop mode (soft or hard) */
    CPSS_DROP_MODE_TYPE_ENT dropMode;

    /** rate mode (bytes or packets) */
    CPSS_RATE_LIMIT_MODE_ENT rMode;

    /** size of time window for 10Mbps port speed */
    GT_U32 win10Mbps;

    /** size of time window for 100Mbps port speed */
    GT_U32 win100Mbps;

    /** size of time window for 1000Mbps port speed */
    GT_U32 win1000Mbps;

    /** @brief size of time window for 10Gbps port speed
     *  Comments:
     *  Window sizes specified in microseconds
     *  1000 Mbps: (APPLICABLE RANGES: 256..16384) uSec  granularity - 256 uSec
     *  100 Mbps: (APPLICABLE RANGES: 256..131072) uSec  granularity - 256 uSec
     *  10  Mbps: (APPLICABLE RANGES: 256..1048570) uSec granularity - 256 uSec
     *  10  Gbps: (APPLICABLE RANGES: 25.6..104857) uSec granularity - 25.6 uSec
     *  For DxCh3 and above: The granularity doesn't depend from PP core clock.
     *  For DxCh1, DxCh2:
     *  Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
     *  for they changed linearly with the clock value.
     */
    GT_U32 win10Gbps;

} CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC;

/**
* @struct CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC
 *
 * @brief Describe a port rate limit parameters.
*/
typedef struct{

    /** enable Bc packets rate limiting */
    GT_BOOL enableBc;

    /** @brief enable Mc packets rate limiting
     *  for xCat2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 it controls
     *  unregistered MC packets only
     */
    GT_BOOL enableMc;

    /** @brief enable Mc registered packets rate limiting
     *  (APPLICABLE DEVICES: xCat2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL enableMcReg;

    /** enable Uc Unknown packets rate limiting */
    GT_BOOL enableUcUnk;

    /** enable Uc Known  packets rate limiting */
    GT_BOOL enableUcKnown;

    /** @brief rate limit specified in 64 bytes/packets
     *  depends on ratelimit mode
     *  (APPLICABLE RANGES: 0..65535)
     *  Comments:
     *  None
     */
    GT_U32 rateLimit;

} CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC;

/**
* @internal cpssDxChBrgGenIgmpSnoopEnable function
* @endinternal
*
* @brief   Enable/disable trapping all IPv4 IGMP packets to the CPU, based on
*         their Ingress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] status                   - GT_TRUE for enable and GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIgmpSnoopEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      status
);

/**
* @internal cpssDxChBrgGenIgmpSnoopEnableGet function
* @endinternal
*
* @brief   Get status of trapping all IPv4 IGMP packets to the CPU, based on
*         their Ingress port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] statusPtr                - (pointer to ) GT_TRUE for enable and GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIgmpSnoopEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *statusPtr
);

/**
* @internal cpssDxChBrgGenDropIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenDropIpMcEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  state
);

/**
* @internal cpssDxChBrgGenDropIpMcEnableGet function
* @endinternal
*
* @brief   Get status of discarding all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] statePtr                 - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenDropIpMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *statePtr
);

/**
* @internal cpssDxChBrgGenDropNonIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet
*         packets with a MAC Multicast DA (but not the Broadcast MAC address)
*         not corresponding to the IP Multicast range
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenDropNonIpMcEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  state
);

/**
* @internal cpssDxChBrgGenDropNonIpMcEnableGet function
* @endinternal
*
* @brief   Get status of discarding all non-Control-classified Ethernet
*         packets with a MAC Multicast DA (but not the Broadcast MAC address)
*         not corresponding to the IP Multicast range
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] statePtr                 - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenDropNonIpMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *statePtr
);

/**
* @internal cpssDxChBrgGenDropInvalidSaEnable function
* @endinternal
*
* @brief   Drop all Ethernet packets with MAC SA that are Multicast (MACDA[40] = 1)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE,  filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenDropInvalidSaEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgGenDropInvalidSaEnableGet function
* @endinternal
*
* @brief   Get status of droping all Ethernet packets with MAC SA that are
*         Multicast (MACDA[40] = 1)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenDropInvalidSaEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgGenUcLocalSwitchingEnable function
* @endinternal
*
* @brief   Enable/disable local switching back through the ingress interface
*         for for known Unicast packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk
*                                      are assigned with a soft drop command.
*                                      GT_TRUE  = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk may be
*                                      forwarded back to their source.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. If the packet is received from a cascade port, the packets source
*       location is taken from the FORWARD DSA tag and not according to the
*       local device and port number.
*       Routed packets are not subject to local switching configuration
*       constraints.
*       2. For xCat and above devices to enable local switching of known
*       Unicast traffic, both ingress port configuration and VLAN entry (by
*       function cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*/
GT_STATUS cpssDxChBrgGenUcLocalSwitchingEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgGenUcLocalSwitchingEnableGet function
* @endinternal
*
* @brief   Get status of local switching back through the ingress interface
*         for for known Unicast packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_FALSE = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk
*                                      are assigned with a soft drop command.
*                                      GT_TRUE  = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk may be
*                                      forwarded back to their source.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note 1. If the packet is received from a cascade port, the packets source
*       location is taken from the FORWARD DSA tag and not according to the
*       local device and port number.
*       Routed packets are not subject to local switching configuration
*       constraints.
*       2. For xCat and above devices to enable local switching of known
*       Unicast traffic, both ingress port configuration and VLAN entry (by
*       function cpssDxChBrgVlanLocalSwitchingEnableSet) must be enabled.
*
*/
GT_STATUS cpssDxChBrgGenUcLocalSwitchingEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChBrgGenIgmpSnoopModeSet function
* @endinternal
*
* @brief   Set global trap/mirror mode for IGMP snoop on specified device.
*         Relevant when IGMP Trap disabled by cpssDxChBrgGenIgmpSnoopEnable
*         and IGMP Trap/Mirror enabled on a VLAN by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Trap mode: All IGMP packets are trapped to the CPU
*                                      regardless of their type.
*                                      IGMP Snoop mode: Query messages are mirrored to the CPU.
*                                      Non-Query messages are Trapped to the CPU.
*                                      IGMP Router mode: All IGMP packets are mirrored to the CPU,
*                                      regardless of their type.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or IGMP snooping mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIgmpSnoopModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_IGMP_SNOOP_MODE_ENT     mode
);

/**
* @internal cpssDxChBrgGenIgmpSnoopModeGet function
* @endinternal
*
* @brief   Get status of global trap/mirror mode for IGMP snoop on specified device.
*         Relevant when IGMP Trap disabled by cpssDxChBrgGenIgmpSnoopEnable
*         and IGMP Trap/Mirror enabled on a VLAN by function
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to)Trap/Mirror mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or IGMP snooping mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenIgmpSnoopModeGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_IGMP_SNOOP_MODE_ENT     *modePtr
);

/**
* @internal cpssDxChBrgGenArpBcastToCpuCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU ARP Broadcast packets for all VLANs or ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmdMode                  - command interface mode
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, command mode or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For ARP broadcast received on VLAN - IPv4/IPv6 Control Traffic To CPU must be enabled by function
*       cpssDxChBrgVlanIpCntlToCpuSet
*       For ARP broadcast received on port - ARP Broadcast Trap To CPU must be enabled by function
*       cpssDxChBrgGenArpTrapEnable
*
*/
GT_STATUS cpssDxChBrgGenArpBcastToCpuCmdSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    IN CPSS_PACKET_CMD_ENT              cmd
);

/**
* @internal cpssDxChBrgGenArpBcastToCpuCmdGet function
* @endinternal
*
* @brief   Get status of CPU ARP Broadcast packets for all VLANs or ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmdMode                  - command interface mode
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, command mode or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenArpBcastToCpuCmdGet
(
    IN  GT_U8                           devNum,
    IN CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    OUT CPSS_PACKET_CMD_ENT             *cmdPtr
);

/**
* @internal cpssDxChBrgGenRipV1MirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring of IPv4 RIPv1 control messages to the CPU
*         for specified device for all VLANs, with IPv4/IPv6 Control Traffic
*         To CPU Enable set by function cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE = Forward normally
*                                      GT_TRUE  = Mirror-to-CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenRipV1MirrorToCpuEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgGenRipV1MirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get status of mirroring of IPv4 RIPv1 control messages to the CPU
*         for specified device for all VLANs, with IPv4/IPv6 Control Traffic
*         To CPU Enable set by function cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE = Forward normally
*                                      GT_TRUE  = Mirror-to-CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenRipV1MirrorToCpuEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgGenIeeeReservedMcastTrapEnable function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE -
*                                      Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE -
*                                      Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIeeeReservedMcastTrapEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgGenIeeeReservedMcastTrapEnableGet function
* @endinternal
*
* @brief   Get status of trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE -
*                                      Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE -
*                                      Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenIeeeReservedMcastTrapEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
* @endinternal
*
* @brief   Enables forwarding, trapping, or mirroring to the CPU any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..3).
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
*                                      The parameter defines profile (table number)
*                                      for the 256 protocols. Profile bound per port by
*                                      <cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet>.
* @param[in] protocol                 - specifies the Multicast protocol
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, protocol or control packet command
*                                       of profileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh1, DxCh2, DxCh3 devices:
*       BPDU packets with MAC DA = 01-80-C2-00-00-00 are not affect
*       by this mechanism. BPDUs are implicitly trapped to the CPU
*       if the ingress port span tree is not disabled.
*
*/
GT_STATUS cpssDxChBrgGenIeeeReservedMcastProtCmdSet
(
    IN GT_U8                devNum,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgGenIeeeReservedMcastProtCmdGet function
* @endinternal
*
* @brief   Gets command (forwarding, trapping, or mirroring to the CPU) any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..3).
*                                      (APPLICABLE DEVICES xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
*                                      The parameter defines profile (table number)
*                                      for the 256 protocols. Profile bound per port by
*                                      <cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet>.
* @param[in] protocol                 - specifies the Multicast protocol
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, protocol or control packet command
*                                       of profileIndex.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIeeeReservedMcastProtCmdGet
(
    IN GT_U8                devNum,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    OUT CPSS_PACKET_CMD_ENT *cmdPtr
);

/**
* @internal cpssDxChBrgGenCiscoL2ProtCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to the CPU packets,
*         with MAC DA = 0x01-00-0C-xx-xx-xx running CISCO Layer 2 proprietary
*         protocols for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenCiscoL2ProtCmdSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgGenCiscoL2ProtCmdGet function
* @endinternal
*
* @brief   Get status (trapping or mirroring) of the CPU packets,
*         with MAC DA = 0x01-00-0C-xx-xx-xx running CISCO Layer 2 proprietary
*         protocols for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenCiscoL2ProtCmdGet
(
    IN  GT_U8                devNum,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
);

/**
* @internal cpssDxChBrgGenIpV6IcmpTrapEnable function
* @endinternal
*
* @brief   Enable/disable IpV6 ICMP trapping per port for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE -
*                                      IPv6 ICMP packets received on this port are not trapped
*                                      to the CPU. These packets may be trapped or mirrored
*                                      to the CPU based on the VID assigned to the packet
*                                      see cpssDxChBrgVlanIpV6IcmpToCpuEnable function
*                                      GT_TRUE -
*                                      IPv6 ICMP packets with a ICMP type that matches one of the
*                                      ICMP types configured in the IPv6 ICMP Message Type
*                                      Configuration Register<n> by function
*                                      cpssDxChBrgGenIcmpv6MsgTypeSet are trapped to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpV6IcmpTrapEnable
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgGenIpV6IcmpTrapEnableGet function
* @endinternal
*
* @brief   Gets status of IpV6 ICMP trapping per port for specified device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE -
*                                      IPv6 ICMP packets received on this port are not trapped
*                                      to the CPU. These packets may be trapped or mirrored
*                                      to the CPU based on the VID assigned to the packet
*                                      see cpssDxChBrgVlanIpV6IcmpToCpuEnable function
*                                      GT_TRUE -
*                                      IPv6 ICMP packets with a ICMP type that matches one of the
*                                      ICMP types configured in the IPv6 ICMP Message Type
*                                      Configuration Register<n> by function
*                                      cpssDxChBrgGenIcmpv6MsgTypeSet are trapped to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChBrgGenIpV6IcmpTrapEnableGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgGenIcmpv6MsgTypeSet function
* @endinternal
*
* @brief   Set trap/mirror/forward command for specified ICMP message type.
*         To enable ICMPv6 trapping/mirroring on a VLAN interface,
*         set the <IPv6ICMP ToCPUEn> bit in the VLAN<n> Entry by function
*         cpssDxChBrgVlanIpV6IcmpToCpuEnable
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the  of the message type(up to eight IPv6 ICMP types)
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] msgType                  - ICMP message type
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
*                                       or index > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIcmpv6MsgTypeSet
(
    IN GT_U8                devNum,
    IN GT_U32               index,
    IN GT_U8                msgType,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgGenIcmpv6MsgTypeGet function
* @endinternal
*
* @brief   Get status of trap/mirror/forward command for specified ICMP message type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the  of the message type(up to eight IPv6 ICMP types)
*                                      (APPLICABLE RANGES: 0..7)
*
* @param[out] msgTypePtr               - ICMP message type
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
*                                       or index > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenIcmpv6MsgTypeGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               index,
    OUT GT_U8                *msgTypePtr,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
);

/**
* @internal cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring to CPU for IP Link Local Control protocol.
*         IPv4 and IPv6 define link-local Multicast addresses that are used
*         by various protocols.
*         In IPv4, the link-local IANA Multicast range is 224.0.0.0/24.
*         In IPv6, the link-local IANA Multicast range is FF02::/16.
*         Relevant only if IP Control to CPU enabled by
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
* @param[in] enable                   - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or IP protocol version
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      enable
);

/**
* @internal cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of mirroring to CPU
*         for IP Link Local Control protocol.
*         IPv4 and IPv6 define link-local Multicast addresses that are used
*         by various protocols.
*         In IPv4, the link-local IANA Multicast range is 224.0.0.0/24.
*         In IPv6, the link-local IANA Multicast range is FF02::/16.
*         Relevant only if IP Control to CPU enabled by
*         cpssDxChBrgVlanIpCntlToCpuSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
*
* @param[out] enablePtr                - pointer to enable/disable status of mirroring to CPU.
*                                      GT_TRUE  - mirroring to CPU for IP Link Local
*                                      Control protocol is enabled.
*                                      GT_FALSE - mirroring to CPU for IP Link Local
*                                      Control protocol is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or IP protocol version
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable function
* @endinternal
*
* @brief   Enable/disable mirroring to CPU for IP Link Local Control protocol.
*         IPv4 Multicast addresses in the range 224.0.0.x and IPv6 Multicast
*         addresses in the range FF:02::/16 are reserved by IANA
*         for link-local control protocols.
*         If the VLAN is enabled for IP Control to CPU by
*         cpssDxChBrgVlanIpCntlToCpuSet and IPv4/6 Link Local Mirror enabled
*         by cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
*         any of the IPv4/6 Multicast packets with a DIP in this range
*         may be mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocol                 - IPv4 or IPv6 protocol version
* @param[in] protocol                 - link local IP protocol
* @param[in] enable                   - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, IP protocol version or
*                                       Link Local IP protocol
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_U8                        protocol,
    IN GT_BOOL                      enable
);

/**
* @internal cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet function
* @endinternal
*
* @brief   Get status of mirroring to CPU for IP Link Local Control protocol.
*         IPv4 Multicast addresses in the range 224.0.0.x and IPv6 Multicast
*         addresses in the range FF:02::/16 are reserved by IANA
*         for link-local control protocols.
*         If the VLAN is enabled for IP Control to CPU by
*         cpssDxChBrgVlanIpCntlToCpuSet and IPv4/6 Link Local Mirror enabled
*         by cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable,
*         any of the IPv4/6 Multicast packets with a DIP in this range
*         may be mirrored to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] protocol                 - IPv4 or IPv6 protocol version
* @param[in] protocol                 - link local IP protocol
*
* @param[out] enablePtr                - enable/disable mirroring to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, IP protocol version or
*                                       Link Local IP protocol
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN  GT_U8                        protocol,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal cpssDxChBrgGenIpV6SolicitedCmdSet function
* @endinternal
*
* @brief   Set trap/mirror command for IPv6 Neighbor Solicitation Protocol.
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cmd                      - supported commands:   CPSS_PACKET_CMD_FORWARD_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or packet control command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpV6SolicitedCmdSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgGenIpV6SolicitedCmdGet function
* @endinternal
*
* @brief   Get status of trap/mirror command for IPv6 Neighbor Solicitation Protocol.
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or packet control command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS cpssDxChBrgGenIpV6SolicitedCmdGet
(
    IN  GT_U8                devNum,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
);

/**
* @internal cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet function
* @endinternal
*
* @brief   Set IPv6 Solicited-Node Multicast Address Prefix Configuration and Mask
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] addressPtr               - (pointer to)IP address
* @param[in] maskPtr                  - (pointer to) mask of the address
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet
(
    IN GT_U8        devNum,
    IN GT_IPV6ADDR  *addressPtr,
    IN GT_IPV6ADDR  *maskPtr
);

/**
* @internal cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet function
* @endinternal
*
* @brief   Get IPv6 Solicited-Node Multicast Address Prefix Configuration and Mask
*         If the VLAN entry, indexed by the VID assigned to the packet,
*         IP Control to CPU enabled by cpssDxChBrgVlanIpCntlToCpuSet,
*         IPv6 Neighbor Solicitation messages may be trapped or mirrored
*         to the CPU, if the packets DIP matches the configured prefix
*         and according to the setting of IPv6 Neighbor Solicited Node Command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] addressPtr               - (pointer to)IP address
* @param[out] maskPtr                  - (pointer to) mask of the address
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet
(
    IN  GT_U8        devNum,
    OUT GT_IPV6ADDR  *addressPtr,
    OUT GT_IPV6ADDR  *maskPtr
);

/**
* @internal cpssDxChBrgGenRateLimitGlobalCfgSet function
* @endinternal
*
* @brief   Configures global ingress rate limit parameters - rate limiting mode,
*         Four configurable global time window periods are assigned to ports as
*         a function of their operating speed:
*         10 Gbps, 1 Gbps, 100 Mbps, and 10 Mbps.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported windows for DxCh devices:
*       1000 Mbps: range - 256-16384 uSec  granularity - 256 uSec
*       100 Mbps: range - 256-131072 uSec  granularity - 256 uSec
*       10  Mbps: range - 256-1048570 uSec granularity - 256 uSec
*       10  Gbps: range - 25.6-104857 uSec granularity - 25.6 uSec
*       For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
GT_STATUS cpssDxChBrgGenRateLimitGlobalCfgSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC     *brgRateLimitPtr
);

/**
* @internal cpssDxChBrgGenRateLimitGlobalCfgGet function
* @endinternal
*
* @brief   Get global ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
GT_STATUS cpssDxChBrgGenRateLimitGlobalCfgGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC    *brgRateLimitPtr
);

/**
* @internal cpssDxChBrgGenPortRateLimitSet function
* @endinternal
*
* @brief   Configures port ingress rate limit parameters
*         Each port maintains rate limits for unknown unicast packets,
*         known unicast packets, multicast packets and broadcast packets,
*         single configurable limit threshold value, and a single internal counter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] portGfgPtr               - pointer to rate limit configuration for a port
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_OUT_OF_RANGE          - on out of range rate limit values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitSet
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     port,
    IN CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC    *portGfgPtr
);

/**
* @internal cpssDxChBrgGenPortRateLimitGet function
* @endinternal
*
* @brief   Get port ingress rate limit parameters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] portGfgPtr               - pointer to rate limit configuration for a port
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitGet
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     port,
    OUT CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC   *portGfgPtr
);

/**
* @internal cpssDxChBrgGenPortRateLimitTcpSynSet function
* @endinternal
*
* @brief   Enable or disable TCP SYN packets' rate limit for a port.
*         Value of rate limit is configured by cpssDxChBrgGenPortRateLimitSet and
*         cpssDxChBrgGenRateLimitGlobalCfgSet
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] enable                   - enable/disable TCP SYN packets rate limit.
*                                      GT_TRUE - enable rate limit, GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitTcpSynSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     port,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChBrgGenPortRateLimitTcpSynGet function
* @endinternal
*
* @brief   Get the status of TCP SYN packets' rate limit for a port (enable/disable).
*         Value of rate limit can be read by cpssDxChBrgGenPortRateLimitGet and
*         cpssDxChBrgGenRateLimitGlobalCfgGet.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
*
* @param[out] enablePtr                - pointer to TCP SYN packets rate limit status.
*                                      GT_TRUE - enable rate limit, GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitTcpSynGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    port,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChBrgGenUdpBcDestPortCfgSet function
* @endinternal
*
* @brief   Configure UDP Broadcast Destination Port configuration table.
*         It's possible to configure several UDP destination ports with their Trap
*         or Mirror to CPU command and CPU code. See datasheet of device for maximal
*         number of UDP destination ports, which may be configured. The feature may
*         be enabled or disabled per VLAN by cpssDxChBrgVlanUdpBcPktsToCpuEnable
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*                                      (APPLICABLE RANGES: 0..11)
* @param[in] udpPortNum               - UDP destination port number
* @param[in] cpuCode                  - CPU code for packets trapped or mirrored by the feature.
*                                      Acceptable CPU codes are: CPSS_NET_UDP_BC_MIRROR_TRAP0_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP1_E, CPSS_NET_UDP_BC_MIRROR_TRAP2_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP3_E
* @param[in] cmd                      - packet command for UDP Broadcast packets
*                                      either CPSS_DXCH_BRG_GEN_CNTRL_TRAP_E
*                                      or CPSS_DXCH_BRG_GEN_CNTRL_MIRROR_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or CPU code or cmd or
*                                       entryIndex > 11
* @retval GT_HW_ERROR              - failed to write to hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgSet
(
   IN GT_U8                     devNum,
   IN GT_U32                    entryIndex,
   IN GT_U16                    udpPortNum,
   IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode,
   IN CPSS_PACKET_CMD_ENT       cmd
);

/**
* @internal cpssDxChBrgGenUdpBcDestPortCfgGet function
* @endinternal
*
* @brief   Get UDP Broadcast Destination Port configuration table.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*                                      (APPLICABLE RANGES: 0..11)
*
* @param[out] validPtr                 - (pointer to) entry validity:
*                                      GT_TRUE  - valid entry,
*                                      GT_FALSE - invalid entry.
* @param[out] udpPortNumPtr            - (pointer to) UDP destination port number
* @param[out] cpuCodePtr               - (pointer to) CPU code for packets trapped or mirrored
*                                      by the feature. Acceptable CPU codes are:
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP0_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP1_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP2_E,
*                                      CPSS_NET_UDP_BC_MIRROR_TRAP3_E
* @param[out] cmdPtr                   - (pointer to) packet command for UDP Broadcast packets
*                                      either CPSS_DXCH_BRG_GEN_CNTRL_TRAP_E
*                                      or CPSS_DXCH_BRG_GEN_CNTRL_MIRROR_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or entryIndex > 11
* @retval GT_HW_ERROR              - failed to read from hw.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgGet
(
   IN GT_U8                     devNum,
   IN GT_U32                    entryIndex,
   OUT GT_BOOL                  *validPtr,
   OUT GT_U16                   *udpPortNumPtr,
   OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr,
   OUT CPSS_PACKET_CMD_ENT      *cmdPtr
);

/**
* @internal cpssDxChBrgGenUdpBcDestPortCfgInvalidate function
* @endinternal
*
* @brief   Invalidate UDP Broadcast Destination Port configuration table.
*         All fields in entry will be reset.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - index in the global UDP Broadcast Port Table, allows to
*                                      application to configure up to 12 UDP ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or intryIndex > 11
* @retval GT_HW_ERROR              - failed to write to hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenUdpBcDestPortCfgInvalidate
(
   IN GT_U8                                     devNum,
   IN GT_U32                                    entryIndex
);

/**
* @internal cpssDxChBrgGenBpduTrapEnableSet function
* @endinternal
*
* @brief   Enable or disable trapping of BPDU Multicast MAC address 01-80-C2-00-00-00
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - trap packets to CPU with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 on a ingress port whose span state
*                                      is not disabled.
*                                      GT_FALSE - packets with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 are not affected by ingress port's span state
*                                      and forwarded as standard Multicast frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenBpduTrapEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChBrgGenBpduTrapEnableGet function
* @endinternal
*
* @brief   Get trapping status of BPDU Multicast MAC address 01-80-C2-00-00-00
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - trap packets to CPU with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 on a ingress port whose span state
*                                      is not disabled.
*                                      GT_FALSE - packets with BPDU Multicast MAC address
*                                      01-80-C2-00-00-00 are not affected by ingress port's span state
*                                      and forwarded as standard Multicast frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenBpduTrapEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgGenArpTrapEnable function
* @endinternal
*
* @brief   Enable/disable trapping ARP Broadcast packets to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] enable                   - GT_TRUE - ARP Broadcast packets received on this port
*                                      are trapped to the CPU.
*                                      GT_FALSE - ARP Broadcast packets received on this port
*                                      are not trapped to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenArpTrapEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgGenArpTrapEnableGet function
* @endinternal
*
* @brief   Get trapping ARP Broadcast packets to the CPU status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                -  pointer to trapping ARP Broadcast packets status
*                                      GT_TRUE - ARP Broadcast packets received on this port
*                                      are trapped to the CPU.
*                                      GT_FALSE - ARP Broadcast packets received on this port
*                                      are not trapped to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenArpTrapEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgGenDropIpMcModeSet function
* @endinternal
*
* @brief   Set drop mode for IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop IP multicast is enabled.
*       See cpssDxChBrgGenDropIpMcEnable.
*
*/
GT_STATUS cpssDxChBrgGenDropIpMcModeSet
(
    IN GT_U8                    devNum,
    IN CPSS_DROP_MODE_TYPE_ENT  mode
);

/**
* @internal cpssDxChBrgGenDropIpMcModeGet function
* @endinternal
*
* @brief   Get drop mode for IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop IP multicast is enabled.
*       See cpssDxChBrgGenDropIpMcEnable.
*
*/
GT_STATUS cpssDxChBrgGenDropIpMcModeGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_DROP_MODE_TYPE_ENT  *modePtr
);



/**
* @internal cpssDxChBrgGenDropNonIpMcModeSet function
* @endinternal
*
* @brief   Set drop mode for non-IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop non-IP multicast is enabled.
*       See cpssDxChBrgGenDropNonIpMcEnable.
*
*/
GT_STATUS cpssDxChBrgGenDropNonIpMcModeSet
(
    IN GT_U8                    devNum,
    IN CPSS_DROP_MODE_TYPE_ENT  mode
);

/**
* @internal cpssDxChBrgGenDropNonIpMcModeGet function
* @endinternal
*
* @brief   Get drop mode for non-IP Multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to drop mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when Drop non-IP multicast is enabled.
*       See cpssDxChBrgGenDropNonIpMcEnable.
*
*/
GT_STATUS cpssDxChBrgGenDropNonIpMcModeGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_DROP_MODE_TYPE_ENT  *modePtr
);

/**
* @internal cpssDxChBrgGenCfiRelayEnableSet function
* @endinternal
*
* @brief   Enable / Disable CFI bit relay.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  Tagged packets transmitted from the device have
*                                      have their CFI bit set according to received
*                                      packet CFI bit if the received packet was tagged
*                                      or set to 0 if the received packet is untagged.
*                                      - GT_FALSE - Tagged packets transmitted from the device
*                                      have their CFI bit set to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenCfiRelayEnableSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable
);

/**
* @internal cpssDxChBrgGenCfiRelayEnableGet function
* @endinternal
*
* @brief   Get CFI bit relay status.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                -  pointer to CFI bit relay status.
*                                      - GT_TRUE -  Tagged packets transmitted from the device have
*                                      have their CFI bit set according to received
*                                      packet CFI bit if the received packet was tagged
*                                      or set to 0 if the received packet is untagged.
*                                      - GT_FALSE - Tagged packets transmitted from the device
*                                      have their CFI bit set to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenCfiRelayEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet function
* @endinternal
*
* @brief   Select the IEEE Reserved Multicast profile (table) associated with port
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
* @param[in] profileIndex             - profile index. The parameter defines profile (table
*                                      number) for the 256 protocols (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port
* @retval GT_OUT_OF_RANGE          - for profileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               profileIndex
);

/**
* @internal cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet function
* @endinternal
*
* @brief   Get the IEEE Reserved Multicast profile (table) associated with port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*
* @param[out] profileIndexPtr          - (pointer to) profile index. The parameter
*                                      defines profile (table number) for the 256 protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or profileId
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    OUT GT_U32              *profileIndexPtr
);

/**
* @internal cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet function
* @endinternal
*
* @brief   Enable/disable dropping all ingress untagged packets received
*         by specified port with byte count more then 1518 bytes.
*         This feature is additional to cpssDxChPortMruSet that sets
*         MRU at the MAC level and is not aware of packet tagging.
*         If MAC level MRU is 1522, untagged packets with size 1519..1522 bytes
*         are not dropped at the MAC level as needed.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_FALSE -
*                                      untagged packets with size more then 1518 bytes not dropped
*                                      GT_TRUE -
*                                      untagged packets with size more then 1518 bytes dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet function
* @endinternal
*
* @brief   Get Enable/disable status of dropping all ingress untagged packets received
*         by specified port with byte count more then 1518 bytes.
*         This feature is additional to cpssDxChPortMruSet that sets
*         MRU at the MAC level and is not aware of packet tagging.
*         If MAC level MRU is 1522, untagged packets with size 1519..1522 bytes
*         are not dropped at the MAC level as needed.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU port)
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE -
*                                      untagged packets with size more then 1518 bytes not dropped
*                                      GT_TRUE -
*                                      untagged packets with size more then 1518 bytes dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet function
* @endinternal
*
* @brief   Set the oversize packet command for untagged packets with length bigger
*         than 1518 bytes
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable/disable filtering of untagged packets with length bigger than
*       1518 bytes on ePort, use cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet
*
*/
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
);

/**
* @internal cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet function
* @endinternal
*
* @brief   Get the oversize packet command for untagged packets with length bigger
*         than 1518 bytes
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To get the filtering status on ePort of untagged packets with length bigger
*       than 1518 bytes, use cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet
*
*/
GT_STATUS cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
);

/**
* @internal cpssDxChBrgGenRateLimitSpeedCfgEnableSet function
* @endinternal
*
* @brief   Enable/Disable the port speed configuration that will be used by the
*         Bridge ingress rate limiter, regardless of the actual speed in which
*         a port actually works.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE -  Port speed for bridge rate limit
*                                      calculations is according to the actual
*                                      port speed.
*                                      GT_TRUE - Port speed for bridge rate limit
*                                      calculations is taken from a per-port configuration
*                                      cpssDxChBrgGenPortRateLimitSpeedGranularitySet()
*                                      and NOT from the port actual speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
GT_STATUS cpssDxChBrgGenRateLimitSpeedCfgEnableSet
(
    IN GT_U8      devNum,
    IN GT_BOOL    enable
);

/**
* @internal cpssDxChBrgGenRateLimitSpeedCfgEnableGet function
* @endinternal
*
* @brief   Get the status of the port speed configuration (Enabled/Disabled) that
*         will be used by the Bridge ingress rate limiter, regardless of the actual
*         speed in which a port actually works.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_FALSE -  Port speed for bridge rate limit
*                                      calculations is according to the actual
*                                      port speed.
*                                      GT_TRUE -   Port speed for bridge rate limit
*                                      calculations is taken from a per-port configuration
*                                      cpssDxChBrgGenPortRateLimitSpeedGranularitySet()
*                                      and NOT from the port actual speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
GT_STATUS cpssDxChBrgGenRateLimitSpeedCfgEnableGet
(
    IN  GT_U8      devNum,
    OUT GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChBrgGenPortRateLimitSpeedGranularitySet function
* @endinternal
*
* @brief   Configure 100Mbps/10Mbps/1Gbps/10Gbps port speed for the use of the bridge
*         ingress rate limit calculations.
*         If port speed for Bridge ingress rate limit calculations is enabled,
*         cpssDxChBrgGenRateLimitSpeedCfgEnableSet() API, then this configuration
*         is used.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      (APPLICABLE RANGES: xCat, xCat3 0..23)
* @param[in] speedGranularity         - Port speed granularity
*                                      (10Mbps/100Mbps/1Gbps/10Gbps) that
*                                      will be used for bridge ingress rate limit
*                                      calculations.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum or speedGranularity
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
GT_STATUS cpssDxChBrgGenPortRateLimitSpeedGranularitySet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_SPEED_ENT      speedGranularity
);

/**
* @internal cpssDxChBrgGenPortRateLimitSpeedGranularityGet function
* @endinternal
*
* @brief   Get port speed configuration. This configuration is used for bridge
*         ingress rate limit calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      (APPLICABLE RANGES: xCat, xCat3 0..23)
*
* @param[out] speedGranularityPtr      - (pointer to) port speed granularity
*                                      (10Mbps/100Mbps/1Gbps/10Gbps) that will be
*                                      used for bridge ingress rate limit calculations.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat & xCat3 devices the feature is not relevant for Flex link ports.
*
*/
GT_STATUS cpssDxChBrgGenPortRateLimitSpeedGranularityGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_SPEED_ENT         *speedGranularityPtr
);

/**
* @internal cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet function
* @endinternal
*
* @brief   Enable / Disable excluding unregistered IPv4 / IPv6 Link Local Multicast
*         packets from the per VLAN Unregistered IPv4 / IPv6 Multicast command.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
* @param[in] enable                   - GT_TRUE -
*                                      exclude unregistered IPv4 / IPv6
*                                      Link Local Multicast packets from the per VLAN
*                                      Unregistered IPv4 / IPv6 Multicast command
*                                      - GT_FALSE -
*                                      Unregistered link Local Multicast packets
*                                      are handled according to the
*                                      Unregistered IPv4 / IPv6 Multicast command.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, protocolStack
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      enable
);

/**
* @internal cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet function
* @endinternal
*
* @brief   Get enable / disable status for excluding unregistered IPv4 / IPv6
*         Link Local Multicast packets from the per VLAN
*         Unregistered IPv4 / IPv6 Multicast command.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - IPv4 or IPv6 protocol version
*
* @param[out] enablePtr                - pointer to status for excluding unregistered
*                                      IPv4 / IPv6 Link Local Multicast packets from the
*                                      per VLAN Unregistered IPv4 / IPv6 Multicast command.
*                                      - GT_TRUE -
*                                      exclude unregistered IPv4 / IPv6
*                                      Link Local Multicast packets from the per VLAN
*                                      Unregistered IPv4 / IPv6 Multicast command
*                                      - GT_FALSE -
*                                      Unregistered link Local Multicast packets
*                                      are handled according to the
*                                      Unregistered IPv4 / IPv6 Multicast command.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, protocolStack
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - learning is performed also for IEEE MC
*                                      trapped packets.
*                                      - GT_FALSE - No learning is performed for IEEE MC
*                                      trapped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet function
* @endinternal
*
* @brief   Get status of MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU (enabled/disabled).
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                pointer to status of MAC SA learning on reserved IEEE
*                                      MC packets trapped packets:
*                                      - GT_TRUE - learning is performed also for IEEE MC
*                                      trapped packets.
*                                      - GT_FALSE - No learning is performed for IEEE MC
*                                      trapped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgGenPortArpMacSaMismatchDropEnable function
* @endinternal
*
* @brief   Enable/disable configuration for ARP MAC SA check per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is dropped.
*                                      GT_FALSE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is not dropped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The global ARP MAC SA configuration must remain. When both global
*       configuration is enabled and per port configuration is enabled the
*       ARP MAC SA mismatch check is performed and if fails packet is dropped.
*
*/
GT_STATUS cpssDxChBrgGenPortArpMacSaMismatchDropEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet function
* @endinternal
*
* @brief   Get configuration for ARP MAC SA check per port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is dropped.
*                                      GT_FALSE - if ARP MAC SA mismatch check is performed and it
*                                      fails packet is not dropped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet function
* @endinternal
*
* @brief   Set Unknown UC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] cmd                      - Unknown DA command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet function
* @endinternal
*
* @brief   Get Unknown UC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cmdPtr                   - (pointer to) Unknown DA command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
);

/**
* @internal cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet function
* @endinternal
*
* @brief   Set Unregistered MC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] cmd                      - Unregistered MC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet function
* @endinternal
*
* @brief   Get Unregistered MC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] cmdPtr                   - (pointer to) Unregistered MC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
);

/**
* @internal cpssDxChBrgGenIngressPortBcFilterDaCommandSet function
* @endinternal
*
* @brief   Set BC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] cmd                      - BC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortBcFilterDaCommandSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgGenIngressPortBcFilterDaCommandGet function
* @endinternal
*
* @brief   Get BC DA filter command per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] cmdPtr                   - (pointer to) BC command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIngressPortBcFilterDaCommandGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_PACKET_CMD_ENT     *cmdPtr
);

/**
* @internal cpssDxChBrgGenPortIpControlTrapEnableSet function
* @endinternal
*
* @brief   Set IPV4/6 Control Trap Enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ipCntrlType              - IP control type
*                                      CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIpControlTrapEnableSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT       ipCntrlType
);

/**
* @internal cpssDxChBrgGenPortIpControlTrapEnableGet function
* @endinternal
*
* @brief   Get IPV4/6 Control Trap Enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ipCntrlTypePtr           - (pointer to) IP control type
*                                      CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortIpControlTrapEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      *ipCntrlTypePtr
);

/**
* @internal cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet function
* @endinternal
*
* @brief   Set BC UDP Trap Mirror enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Enable/disable UDP Trap Mirror.
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet function
* @endinternal
*
* @brief   Get BC UDP Trap Mirror enable Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                - UDP Trap Mirror status
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgGenPortRateLimitDropCntrEnableSet function
* @endinternal
*
* @brief   Enable/Disable port for counting packets dropped due to ingress rate
*         limiting by the Ingress Rate Limit Drop Counter
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  counting of packets dropped due to
*                                      ingress rate.
*                                      GT_FALSE: disable counting of packets dropped due to
*                                      ingress rate.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitDropCntrEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChBrgGenPortRateLimitDropCntrEnableGet function
* @endinternal
*
* @brief   Get port status (enabled/disabled) for counting packets dropped due
*         to ingress rate limiting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE: enable counting of packets dropped due to
*                                      ingress rate.
*                                      GT_FALSE: disable counting of packets dropped due to
*                                      ingress rate.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenPortRateLimitDropCntrEnableGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChBrgGenRateLimitDropCntrSet function
* @endinternal
*
* @brief   Set rate limit drop counter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] value                    - the  to set the rate limit drop counter.
*                                      the value size is 40 bits.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range drop counter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenRateLimitDropCntrSet
(
    IN GT_U8        devNum,
    IN GT_U64       value
);

/**
* @internal cpssDxChBrgGenRateLimitDropCntrGet function
* @endinternal
*
* @brief   Get rate limit drop counter.
*         The counter counts packets or bytes according to ingress rate limit mode
*         set by cpssDxChBrgGenRateLimitGlobalCfgSet.
*         The counter counts packets/bytes that are dropped due to ingress rate
*         limitting on all ports, where the drop rate limit counting is enabled
*         by cpssDxChBrgGenPortRateLimitDropCntrEnableSet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) the value of rate limit drop counter
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenRateLimitDropCntrGet
(
    IN  GT_U8        devNum,
    OUT GT_U64       *valuePtr
);

/**
* @internal cpssDxChBrgGenBypassModeSet function
* @endinternal
*
* @brief   Set Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
GT_STATUS cpssDxChBrgGenBypassModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT    mode
);

/**
* @internal cpssDxChBrgGenBypassModeGet function
* @endinternal
*
* @brief   Get Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
GT_STATUS cpssDxChBrgGenBypassModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChBrgGenMtuConfigSet function
* @endinternal
*
* @brief   Enable/disable Target ePort MTU check feature.
*         and configure the following parameters:
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mtuCheckEnable           - target ePort MTU check feature enabled/disabled
*                                      GT_TRUE  MTU check enabled, GT_FALSE - MTU check disabled
* @param[in] byteCountMode            - byte-count mode for MTU
* @param[in] exceptionCommand         command assigned to frames that exceed the
*                                      default ePort MTU size
* @param[in] exceptionCpuCode         CPU/drop code assigned to frame which fails MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, byteCountMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenMtuConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_BOOL                                      mtuCheckEnable,
    IN CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    byteCountMode,
    IN CPSS_PACKET_CMD_ENT                          exceptionCommand,
    IN CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode
);

/**
* @internal cpssDxChBrgGenMtuConfigGet function
* @endinternal
*
* @brief   Get the target ePort MTU feature parameters:
*         - the enable/disable target ePort MTU check feature.
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] mtuCheckEnablePtr        - (pointer to) enable/disable target ePort MTU feature.
*                                      GT_TRUE  MTU check enabled, GT_FALSE - MTU check disabled.
* @param[out] byteCountModePtr         - (pointer to) MTU byte-count mode
* @param[out] exceptionCommandPtr      - (pointer to) exception cmd for frames which
*                                      fails MTU check
* @param[out] exceptionCpuCodePtr      - (pointer to) CPU/drop code assigned to frame
*                                      which fails the MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong HW value
*/
GT_STATUS cpssDxChBrgGenMtuConfigGet
(
    IN  GT_U8                                         devNum,
    OUT GT_BOOL                                      *mtuCheckEnablePtr,
    OUT CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT    *byteCountModePtr,
    OUT CPSS_PACKET_CMD_ENT                          *exceptionCommandPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT                     *exceptionCpuCodePtr
);


/**
* @internal cpssDxChBrgGenMtuPortProfileIdxSet function
* @endinternal
*
* @brief   Set MTU profile index for the given target ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mtuProfileIdx            - index to global MTU entry
*                                      (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_OUT_OF_RANGE      mtuProfileIdx out of range
*/
GT_STATUS cpssDxChBrgGenMtuPortProfileIdxSet
(
    IN GT_U8         devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_U32        mtuProfileIdx
);

/**
* @internal cpssDxChBrgGenMtuPortProfileIdxGet function
* @endinternal
*
* @brief   Get MTU profile index for the given ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] mtuProfileIdxPtr         - (pointer to)index to the global MTU entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenMtuPortProfileIdxGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    portNum,
    OUT GT_U32         *mtuProfileIdxPtr
);

/**
* @internal cpssDxChBrgGenMtuExceedCntrGet function
* @endinternal
*
* @brief   Get target ePort MTU exceeded counter.
*         The number of packets that triggered the Target ePort MTU
*         exceeded exception check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) the value of target ePort MTU exceeded counter.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenMtuExceedCntrGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *valuePtr
);

/**
* @internal cpssDxChBrgGenMtuProfileSet function
* @endinternal
*
* @brief   Set an MTU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
* @param[in] mtuSize                  - max transmit packet size in bytes
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range mtuSize
*/
GT_STATUS cpssDxChBrgGenMtuProfileSet
(
    IN GT_U8    devNum,
    IN GT_U32   profile,
    IN GT_U32   mtuSize
);

/**
* @internal cpssDxChBrgGenMtuProfileGet function
* @endinternal
*
* @brief   Get an MTU size for ePort profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
*
* @param[out] mtuSizePtr               - (pointer to) max transmit packet size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgGenMtuProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profile,
    OUT GT_U32  *mtuSizePtr
);

/**
* @internal cpssDxChBrgGenIpmClassificationModeSet function
* @endinternal
*
* @brief   Configue IPM classification mode.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] mode                     - IPM classification mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpmClassificationModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   mode
);

/**
* @internal cpssDxChBrgGenIpmClassificationModeGet function
* @endinternal
*
* @brief   Get IPM classification mode.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] modePtr                  - (pointer to)IPM classification mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgGenIpmClassificationModeGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT   *modePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgGenh */

