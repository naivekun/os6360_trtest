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
* @file cpssDxChCutThrough.h
*
* @brief CPSS DXCH Cut Through facility API.
*
* @version   13
********************************************************************************
*/
#ifndef __cpssDxChCutThroughh
#define __cpssDxChCutThroughh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>

/**
* @enum CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT
 *
 * @brief passenger parsing packet headers mode.
*/
typedef enum{

    /** @brief use calculated Packet Byte Count, QoS, Hash and PCL UDB’s. */
    CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_ALL_DATA_E,

    /** @brief use calculated Packet Byte Count only. */
    CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_BYTE_COUNT_ONLY_E

} CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT;

/**
* @enum CPSS_DXCH_EARLY_PROCESSING_MODE_ENT
 *
 *  @brief Early Processing mode of Cut Through packets.
 *  Related to all packets with triggered cut through.
*/
typedef enum{

    /** @brief Pure Cut Through mode. Packet descriptor sent to Control Pipe
     *  when header only received, and sent to egress immediately after Control Pipe Processing.
     *  The size of packet calculated by header and not adjusted by real size of received packet.
     */
    CPSS_DXCH_EARLY_PROCESSING_CUT_THROUGH_E,

    /** @brief Reduced latency Store and Forward mode. Packet descriptor sent to Control Pipe
     *  when header only received, but sent to egress only after it received all.
     *  The same behavior as Cut Through Terminated packets.
     *  The size of packet calculated by header but before sending to egress
     *  adjusted by real size of received packet.
     */
    CPSS_DXCH_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E

} CPSS_DXCH_EARLY_PROCESSING_MODE_ENT;

/**
* @struct CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC
 *
 * @brief Packet Header Integrity Check configuration.
*/
typedef struct{

    /** @brief minimal Boundary of Cut Through Packet Byte Count.
     *  (APPLICABLE_RANGES: 0..0x3FFF)
     */
    GT_U32 minByteCountBoundary;

    /** @brief maximal Boundary of Cut Through Packet Byte Count.
     *  (APPLICABLE_RANGES: 0..0x3FFF)
     */
    GT_U32 maxByteCountBoundary;

    /** @brief GT_TRUE/GT_FALSE - enable/disable check Cut Through Packet Byte Count
     *  by maximal and minimal boundaries.
     */
    GT_BOOL enableByteCountBoundariesCheck;

    /** @brief GT_TRUE/GT_FALSE - enable/disable check IPV4 header
     *  by IPV4 header checksum.
     */
    GT_BOOL enableIpv4HdrCheckByChecksum;

    /** @brief packet command (forward, mirror
     *  hard-drop, soft-drop, or trap-to-cpu)
     */
    CPSS_PACKET_CMD_ENT hdrIntergrityExceptionPktCmd;

    /** @brief The CPU/Drop code assigned to packets
     *  Mirrored to CPU or Trapped to CPU or Dropped due packet header Intergrity Exception
     */
    CPSS_NET_RX_CPU_CODE_ENT hdrIntergrityExceptionCpuCode;

} CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC;

/**
* @enum CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT
 *
 * @brief Bypass modes for Ingress PCL.
*/
typedef enum{

    /** No bypass for Ingress PCL. */
    CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E,

    /** @brief Lookup CPSS_PCL_LOOKUP_0_0_E is enabled, CPSS_PCL_LOOKUP_0_1_E and
     *  CPSS_PCL_LOOKUP_1_E are bypassed.
     */
    CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E,

    /** @brief For Ingress PCL engine only.
     *  Lookup CPSS_PCL_LOOKUP_0_0_E and CPSS_PCL_LOOKUP_0_1_E is enabled,
     *  CPSS_PCL_LOOKUP_1_E is bypassed.
     */
    CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E,

    /** Ingress PCL is completely bypassed. */
    CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E

} CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT;

/**
* @struct CPSS_DXCH_CUT_THROUGH_BYPASS_STC
 *
 * @brief used to describe Bypass mode per engine.
*/
typedef struct{

    /** @brief GT_TRUE
     *  - GT_FALSE - No bypass for router engine.
     */
    GT_BOOL bypassRouter;

    /** @brief GT_TRUE
     *  - GT_FALSE - No bypass for Ingress policer0 engine.
     */
    GT_BOOL bypassIngressPolicerStage0;

    /** @brief GT_TRUE
     *  - GT_FALSE - No bypass for Ingress policer1 engine.
     */
    GT_BOOL bypassIngressPolicerStage1;

    /** @brief GT_TRUE
     *  - GT_FALSE - No bypass for Egress policer engine.
     */
    GT_BOOL bypassEgressPolicer;

    /** @brief GT_TRUE
     *  - GT_FALSE - No bypass for Egress PCL engine.
     */
    GT_BOOL bypassEgressPcl;

    /** Bypass Ingress PCL engine mode. */
    CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT bypassIngressPcl;

    /** @brief GT_TRUE
     *  - GT_FALSE - No bypass for Ingress OAM engine.
     */
    GT_BOOL bypassIngressOam;

    /** @brief GT_TRUE
     *  - GT_FALSE - No bypass for Egress OAM engine.
     */
    GT_BOOL bypassEgressOam;

    /** @brief GT_TRUE
     *  - GT_FALSE - No bypass for MLL engine.
     */
    GT_BOOL bypassMll;

} CPSS_DXCH_CUT_THROUGH_BYPASS_STC;

/**
* @internal cpssDxChCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*         Bobcat3 device should be initialized with PHASE1_INIT_INFO.cutThroughEnable == GT_TRUE.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] enable                   - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[in] untaggedEnable           -  GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Used only if enable == GT_TRUE.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssDxChCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_STATE             - on enabling cut-through disabled on initialization
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssDxChCutThroughUpEnableSet.
*
*/
GT_STATUS cpssDxChCutThroughPortEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL  enable,
    IN GT_BOOL  untaggedEnable
);

/**
* @internal cpssDxChCutThroughPortEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode on the specified port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status:
*                                      - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[out] untaggedEnablePtr        - pointer to Cut Through forwarding mode status
*                                      for untagged packets.
*                                      Used only if enablePtr == GT_TRUE.
*                                      GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssDxChCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughPortEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_BOOL  *untaggedEnablePtr
);

/**
* @internal cpssDxChCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssDxChCutThroughPortEnableSet for it.
*
*/
GT_STATUS cpssDxChCutThroughUpEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    up,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChCutThroughUpEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode for tagged packets
*         with the specified UP.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status
*                                      for tagged packets, with the specified UP.:
*                                      - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or up
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughUpEnableGet
(
    IN  GT_U8    devNum,
    IN GT_U8     up,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] etherType0               - VLAN EtherType0 (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] etherType1               - VLAN EtherType1 (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughVlanEthertypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType0,
    IN GT_U32   etherType1
);

/**
* @internal cpssDxChCutThroughVlanEthertypeGet function
* @endinternal
*
* @brief   Get VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherType0Ptr            - Pointer to VLAN EtherType0
* @param[out] etherType1Ptr            - Pointer to VLAN EtherType1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughVlanEthertypeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *etherType0Ptr,
    OUT GT_U32   *etherType1Ptr
);

/**
* @internal cpssDxChCutThroughMinimalPacketSizeSet function
* @endinternal
*
* @brief   Set minimal packet size that is enabled for Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] size                     - minimal packet  in bytes for Cut Through (APPLICABLE RANGES: 129..16376).
*                                      Granularity - 8 bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When using cut-through to 1G port, and bypassing
*       of Router And Ingress Policer engines is disabled,
*       the minimal packet size should be 512 bytes.
*       When bypassing of Router And Ingress Policer engines is enabled,
*       the minimal cut-through packet size should be:
*       - for 10G or faster ports - at least 257 bytes.
*       - for ports slower than 10 G - at least 513 bytes
*
*/
GT_STATUS cpssDxChCutThroughMinimalPacketSizeSet
(
    IN GT_U8    devNum,
    IN GT_U32   size
);

/**
* @internal cpssDxChCutThroughMinimalPacketSizeGet function
* @endinternal
*
* @brief   Get minimal packet size that is enabled for Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] sizePtr                  - pointer to minimal packet size in bytes for Cut Through.
*                                      Granularity:
*                                      (APPLICABLE VALUES: Lion; Lion2 8 bytes)
*                                      (APPLICABLE VALUES: Bobcat3; Aldrin2; Falcon 128 bytes)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughMinimalPacketSizeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *sizePtr
);

/**
* @internal cpssDxChCutThroughMemoryRateLimitSet function
* @endinternal
*
* @brief   Set rate limiting of read operations from the memory
*         per target port in Cut Through mode according to the port speed.
*         To prevent congestion in egress pipe, buffer memory read operations
*         are rate limited according to the target port speed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] enable                   - GT_TRUE - rate limiting is enabled.
*                                      - GT_FALSE - rate limiting is disabled.
* @param[in] portSpeed                - port speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or portSpeed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Rate limit is recommended be enabled
*       on all egress ports for cut-through traffic.
*       Rate limit to the CPU port should be configured as a 1G port.
*       Bobcat3 : rate limit configuration is part of cpssDxChPortModeSpeedSet() API
*
*/
GT_STATUS cpssDxChCutThroughMemoryRateLimitSet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable,
    IN CPSS_PORT_SPEED_ENT  portSpeed
);

/**
* @internal cpssDxChCutThroughMemoryRateLimitGet function
* @endinternal
*
* @brief   Get rate limiting of read operations from the memory
*         per target port in Cut Through mode.
*         To prevent congestion in egress pipe, buffer memory read operations
*         are rate limited according to the target port speed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] enablePtr                - pointer to rate limiting mode status.
*                                      - GT_TRUE - rate limiting is enabled.
*                                      - GT_FALSE - rate limiting is disabled.
* @param[out] portSpeedPtr             - pointer to port speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughMemoryRateLimitGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr,
    OUT CPSS_PORT_SPEED_ENT  *portSpeedPtr
);

/**
* @internal cpssDxChCutThroughBypassRouterAndPolicerEnableSet function
* @endinternal
*
* @brief   Enable / Disable bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE -  bypassing of Router and Ingress Policer
*                                      engines is enabled.
*                                      - GT_FALSE - bypassing of Router and Ingress Policer
*                                      engines is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughBypassRouterAndPolicerEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChCutThroughBypassRouterAndPolicerEnableGet function
* @endinternal
*
* @brief   Get mode of bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to bypassing the Router and Ingress
*                                      Policer engines status.
*                                      - GT_TRUE  -  bypassing of Router and Ingress Policer
*                                      engines is enabled.
*                                      - GT_FALSE -  bypassing of Router and Ingress Policer
*                                      engines is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughBypassRouterAndPolicerEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChCutThroughBypassModeSet function
* @endinternal
*
* @brief   Set Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] bypassModePtr            - pointer to Bypass mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, Bypass Ingress PCL mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughBypassModeSet
(
    IN GT_U8                             devNum,
    IN CPSS_DXCH_CUT_THROUGH_BYPASS_STC  *bypassModePtr
);

/**
* @internal cpssDxChCutThroughBypassModeGet function
* @endinternal
*
* @brief   Get Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] bypassModePtr            - pointer to Bypass mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughBypassModeGet
(
    IN  GT_U8                              devNum,
    OUT CPSS_DXCH_CUT_THROUGH_BYPASS_STC   *bypassModePtr
);

/**
* @internal cpssDxChCutThroughPortGroupMaxBuffersLimitSet function
* @endinternal
*
* @brief   Set max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] buffersLimit             - Buffers limit enable.
*                                      - GT_TRUE  - enable.
*                                      - GT_FALSE - disable.
* @param[in] buffersLimit             - Max buffers limit for Cut-Through traffic.
*                                      The resolution is one buffer.
*                                      (APPLICABLE RANGES: Lion2 0..0x3FFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong maxSharedBuffersLimit
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*       buffersLimit = (buffers number per port group) – (guard band).
*       where guard band is (MTU) (number of cut-through ports in port group)
*
*/
GT_STATUS cpssDxChCutThroughPortGroupMaxBuffersLimitSet
(
    IN GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN GT_BOOL                          buffersLimitEnable,
    IN GT_U32                           buffersLimit
);

/**
* @internal cpssDxChCutThroughPortGroupMaxBuffersLimitGet function
* @endinternal
*
* @brief   Get max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] buffersLimitEnablePtr    - (pointer to)Buffers limit enable.
*                                      - GT_TRUE  - enable.
*                                      - GT_FALSE - disable.
* @param[out] buffersLimitPtr          - (pointer to)Max buffers limit for Cut-Through traffic.
*                                      The resolution is one buffer.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*       buffersLimit = (buffers number per port group) – (guard band).
*       where guard band is (MTU) (number of cut-through ports in port group)
*
*/
GT_STATUS cpssDxChCutThroughPortGroupMaxBuffersLimitGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    OUT GT_BOOL                         *buffersLimitEnablePtr,
    OUT GT_U32                          *buffersLimitPtr
);

/**
* @internal cpssDxChCutThroughEarlyProcessingModeSet function
* @endinternal
*
* @brief   Set Early Processing mode of Cut Through packets.
*          Set Reduced Latency Store and Forward mode or Pure Cut Through mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[in] mode                        - early processing mode of cut through packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughEarlyProcessingModeSet
(
    IN GT_U8                                      devNum,
    IN CPSS_DXCH_EARLY_PROCESSING_MODE_ENT        mode
);

/**
* @internal cpssDxChCutThroughEarlyProcessingModeGet function
* @endinternal
*
* @brief   Get Early Processing mode of Cut Through packets.
*          Get Reduced Latency Store and Forward mode or Pure Cut Through mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[out] modePtr                    - (pointer to) early processing mode of cut through packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughEarlyProcessingModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_EARLY_PROCESSING_MODE_ENT        *modePtr
);

/**
* @internal cpssDxChCutThroughUdeCfgSet function
* @endinternal
*
* @brief   Set Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[in] udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[in] udeCutThroughEnable         - GT_TRUE/GT_FALSE enable/disable Cut Through mode for UDE packets.
* @param[in] udeByteCount                - default CT Byte Count for UDE packets.
*                                          (APPLICABLE RANGES: 0..0x3FFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range udeByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughUdeCfgSet
(
    IN GT_U8       devNum,
    IN GT_U32      udeIndex,
    IN GT_BOOL     udeCutThroughEnable,
    IN GT_U32      udeByteCount
);

/**
* @internal cpssDxChCutThroughUdeCfgGet function
* @endinternal
*
* @brief   Get Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                      - device number
* @param[in]  udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[out] udeCutThroughEnablePtr      - pointer to GT_TRUE/GT_FALSE
*                                           enable/disable Cut Through mode for UDE packets.
* @param[out] udeByteCountPtr             - pointer to default CT Byte Count for UDE packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughUdeCfgGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      udeIndex,
    OUT GT_BOOL     *udeCutThroughEnablePtr,
    OUT GT_U32      *udeByteCountPtr
);

/**
* @internal cpssDxChCutThroughPassengerParsingModeSet function
* @endinternal
*
* @brief   Set Cut Through passenger packet header parsing mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] parseMode              - passenger parse mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughPassengerParsingModeSet
(
    IN GT_U8                                              devNum,
    IN CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   parsingMode
);

/**
* @internal cpssDxChCutThroughPassengerParsingModeGet function
* @endinternal
*
* @brief   Get Cut Through passenger packet header parsing mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                 - device number
* @param[out] parseModePtr           - pointer to passenger parse mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughPassengerParsingModeGet
(
    IN  GT_U8                                              devNum,
    OUT CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   *parsingModePtr
);

/**
* @internal cpssDxChCutThroughByteCountExtractFailsCounterGet function
* @endinternal
*
* @brief   Get counter of fails extracting CT Packet Byte Count by packet header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum    - device number
* @param[out] countPtr  - pointer to count of packet Byte Count extracting fails.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughByteCountExtractFailsCounterGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *countPtr
);

/**
* @internal cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet function
* @endinternal
*
* @brief   Set configuration Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum - device number
* @param[in] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet
(
    IN GT_U8                                                     devNum,
    IN CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  *cfgPtr
);

/**
* @internal cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet function
* @endinternal
*
* @brief   Get configuration of Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum  - device number
* @param[out] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet
(
    IN  GT_U8                                                     devNum,
    OUT CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  *cfgPtr
);

/**
* @internal cpssDxChCutThroughPortByteCountUpdateSet function
* @endinternal
*
* @brief   Set configuration for updating byte count per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] portNum                   - physical port number including CPU port.
* @param[in] toSubtractOrToAdd         - GT_TRUE - to subtract, GT_FALSE to add.
* @param[in] subtractedOrAddedValue    - value to subtract or to add to packet length
*                                        (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on out of range subtractedOrAddedValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughPortByteCountUpdateSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL               toSubtractOrToAdd,
    IN GT_U32                subtractedOrAddedValue
);

/**
* @internal cpssDxChCutThroughPortByteCountUpdateGet function
* @endinternal
*
* @brief   Get configuration for updating byte count per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                    - device number
* @param[in]  portNum                   - physical port number including CPU port.
* @param[out] toSubtractOrToAddPtr      - pointer to GT_TRUE - to subtract, GT_FALSE to add.
* @param[out] subtractedOrAddedValuePtr - pointer to value to subtract or to add to packet length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughPortByteCountUpdateGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL               *toSubtractOrToAddPtr,
    OUT GT_U32                *subtractedOrAddedValuePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ____cpssDxChCutThroughh */





