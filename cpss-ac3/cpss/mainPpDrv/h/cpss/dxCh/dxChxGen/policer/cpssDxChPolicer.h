
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
* @file cpssDxChPolicer.h
*
* @brief CPSS DxCh Ingress Policing Engine API
*
* @version   9
********************************************************************************
*/

#ifndef __cpssDxChPolicerh
#define __cpssDxChPolicerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/policer/cpssGenPolicerTypes.h>


/**
* @struct CPSS_DXCH_POLICER_TB_PARAMS_STC
 *
 * @brief Structure for Policer Token Bucket Parameters
*/
typedef struct{

    /** commited information rate in kilobits/sec (kilo = 1000) */
    GT_U32 cir;

    /** commited burst size in bytes */
    GT_U32 cbs;

} CPSS_DXCH_POLICER_TB_PARAMS_STC;

/**
* @enum CPSS_DXCH_POLICER_CMD_ENT
 *
 * @brief Enumeration of Policer Commands is applied to packets that were
 * classified as Red (out-of-profile) by the traffic meter.
*/
typedef enum{

    /** performs no action on the packet. */
    CPSS_DXCH_POLICER_CMD_NONE_E,

    /** @brief the packet is SOFT or HARD dropped
     *  according to a global configuration
     *  set by cpssDxChPolicerDropRedModeSet.
     */
    CPSS_DXCH_POLICER_CMD_DROP_RED_E,

    /** @brief preserve forwarding decision, but
     *  modify QoS setting according to QoS
     *  parameters in this entry
     */
    CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E,

    /** @brief preserve forwarding
     *  decision, but their QoS parameters
     *  setting is modified as follows:
     *  - QoSProfile is extracted from the
     *  Policers QoS Remarking and
     *  Initial DP Table Entry indexed by the
     *  QoSProfile assigned to the packet by
     *  the previous QoS assignment
     *  engines in the ingress pipe.
     *  - ModifyUP is modified by this entry.
     *  - ModifyDSCP is modified by this entry
     */
    CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E

} CPSS_DXCH_POLICER_CMD_ENT;

/**
* @struct CPSS_DXCH_POLICER_ENTRY_STC
 *
 * @brief Structure for Policer Entry.
*/
typedef struct{

    /** @brief Policer Enable
     *  GT_FALSE - Policer Disabled. Preserve all QoS fields,
     *  don't perform Token Bucket algorithm,
     *  and don't update counters.
     *  GT_TRUE - Policer enabled.
     */
    GT_BOOL policerEnable;

    /** @brief Policer's meter color mode
     *  CPSS_POLICER_COLOR_BLIND_E - The packet's conformance
     *  level (i.e., conforming or non-conforming)
     *  is determined according to the Token Bucket
     *  meter result only.
     *  CPSS_POLICER_COLOR_AWARE_E - The packet's conformance
     *  level (i.e., conforming or non-conforming)
     *  is determined according to the Token Bucket meter
     *  result and the incoming QosProfile Drop Precedence
     *  parameter extracted from the Policers QoS Remarking
     *  and Initial DP Table as follows:
     *  - If the Token Bucket meter result is conforming, and
     *  <IntialDP> = Green the packet is marked as conforming.
     *  - If the Token Bucket meter result is non-conforming, or
     *  <IntialDP> = Red the packet is marked as non-conforming.
     */
    CPSS_POLICER_COLOR_MODE_ENT meterColorMode;

    /** token bucket parameters */
    CPSS_DXCH_POLICER_TB_PARAMS_STC tbParams;

    /** @brief enables counting In Profile and Out of Profile packets,
     *  using one of the 16 counters sets
     *  GT_FALSE - Counting is disabled.
     *  GT_TRUE - Counting is enabled. The counters set used
     *  is the counters set pointed to by counterSetIndex.
     */
    GT_BOOL counterEnable;

    /** @brief policing counters set index (APPLICABLE RANGES: 0..15)
     *  relevant when counterEnable == GT_TRUE
     */
    GT_U32 counterSetIndex;

    /** @brief policer command
     *  Out-of-profile action is applied to packets that were
     *  classified as Red (out-of-profile) by the traffic meter.
     *  cmd == CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E or
     *  cmd == CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E
     *  Relevant for IPv4/IPv6 packets only.Enables modification
     *  of the packet's DSCP field.
     *  When this field is set to
     *  CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E, the packet's DSCP
     *  field is modified to the DSCP extracted from the
     *  QoS Profile Table Entry when the packet is transmitted.
     *  cmd == CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E or
     *  cmd == CPSS_DXCH_POLICER_CMD_QOS_PROFILE_MARK_BY_TABLE_E
     *  Relevant for packets that are transmitted VLAN Tagged or
     *  packets that are transmitted via cascading ports with a
     *  DSA tag in a FORWARD format.
     *  Enables the modification of the packet's IEEE 802.1p
     *  User Priority field. When this field is set to
     *  CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E, the packet's
     *  802.1p User Priority field is set to the UP extracted
     *  from the QoS Profile Table Entry when the packet is
     *  transmitted, regardless of the incoming packet tag format
     *  (Tagged or Untagged).
     */
    CPSS_DXCH_POLICER_CMD_ENT cmd;

    /** @brief The QoSProfile assigned to non
     *  when cmd == CPSS_DXCH_POLICER_CMD_QOS_MARK_BY_ENTRY_E
     */
    GT_U32 qosProfile;

    /** @brief modify Dscp mode
     *  Relevant for non-conforming packets when
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyDscp;

    /** @brief modify Up mode
     *  Relevant for non-conforming packets when
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyUp;

} CPSS_DXCH_POLICER_ENTRY_STC;

/**
* @struct CPSS_DXCH_POLICER_COUNTERS_STC
 *
 * @brief Structure for Policer Counters Entry.
*/
typedef struct{

    /** Out */
    GT_U32 outOfProfileBytesCnt;

    /** In */
    GT_U32 inProfileBytesCnt;

} CPSS_DXCH_POLICER_COUNTERS_STC;


/**
* @internal cpssDxChPolicerInit function
* @endinternal
*
* @brief   Init Traffic Conditioner facility on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP's device number.
*
* @retval GT_OK                    - on success, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - on illegal devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerInit
(
    IN  GT_U8       devNum
);


/**
* @internal cpssDxChPolicerPacketSizeModeSet function
* @endinternal
*
* @brief   Configure Policing mode for non tunnel terminated packets
*         The Policy engine provides the following modes to define packet size:
*         - Layer 1 metering. Packet size includes the
*         entire packet + IPG + preamble.
*         - Layer 2 metering. Packet size includes the entire packet,
*         including Layer 2 header and CRC.
*         - Layer 3 metering. Packet size includes Layer 3 information only,
*         excluding Layer 2 header and CRC.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Policing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerPacketSizeModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_POLICER_PACKET_SIZE_MODE_ENT        mode
);

/**
* @internal cpssDxChPolicerDropRedModeSet function
* @endinternal
*
* @brief   Set the type of the Policer drop action for red packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] dropRedMode              - Red Drop mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or dropRedMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerDropRedModeSet
(
    IN  GT_U8                      devNum,
    IN  CPSS_DROP_MODE_TYPE_ENT    dropRedMode
);

/**
* @internal cpssDxChPolicerPacketSizeModeForTunnelTermSet function
* @endinternal
*
* @brief   Set the policing counting mode for tunnel terminated packets
*         The Policy engine provides the following modes to define packet size:
*         - Layer 1 metering. Packet size includes the
*         entire packet + IPG + preamble.
*         - Layer 2 metering. Packet size includes the entire packet,
*         including Layer 2 header and CRC.
*         - Passenger packet metering.
*         For IP-Over-x packets, the counting includes the passenger packet's
*         BC, excluding the tunnel header and the packets CRC
*         For, Ethernet-Over-x packets, the counting includes the passenger
*         packet's BC, excluding the tunnel header and if packet includes Two
*         CRC patterns (one for the inner packet and one for the outer packets)
*         also excluding the outer CRC
*
* @note   APPLICABLE DEVICES:      DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - policing  for tunnel terminated packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerPacketSizeModeForTunnelTermSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_POLICER_PACKET_SIZE_MODE_ENT   mode
);

/**
* @internal cpssDxChPolicerTokenBucketModeSet function
* @endinternal
*
* @brief   Set the policer token bucket mode and policer MRU.
*         To implement the token bucket algorithm, each of the 256 policers
*         incorporates a Bucket Size Counter (BucketSizeCnt).
*         This counter is incremented with tokens, according to the configured
*         policer rate (CIR) up to a maximal value of the configured
*         Policer Burst Size (CBS)
*         The Byte Count of each conforming packet is decremented from the counter.
*         When a new packet arrives, according to this configuration, the packet
*         conformance is checked according to one of the following modes:
*         - Strict Rate Limiter - If BucketSizeCnt > Packet's Byte Count the packet
*         is conforming else, it is out of profile.
*         - Loose Rate Limiter - If BucketSizeCnt > MRU the packet is conforming
*         else, it is out of profile.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Policer Token Bucket mode.
* @param[in] mru                      - Policer MRU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerTokenBucketModeSet
(
    IN GT_U8                      devNum,
    IN CPSS_POLICER_TB_MODE_ENT   mode,
    IN CPSS_POLICER_MRU_ENT       mru
);

/**
* @internal cpssDxChPolicerEntrySet function
* @endinternal
*
* @brief   Set Policer Entry configuration
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - policer entry index
* @param[in] entryPtr                 - pointer to policer entry
*
* @param[out] tbParamsPtr              - pointer to actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or entry parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEntrySet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            entryIndex,
    IN  CPSS_DXCH_POLICER_ENTRY_STC       *entryPtr,
    OUT CPSS_DXCH_POLICER_TB_PARAMS_STC   *tbParamsPtr
);


/**
* @internal cpssDxChPolicerEntryGet function
* @endinternal
*
* @brief   Get Policer Entry parameters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - policer entry index
*
* @param[out] entryPtr                 - pointer to policer entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_DXCH_POLICER_ENTRY_STC     *entryPtr
);

/**
* @internal cpssDxChPolicerEntryInvalidate function
* @endinternal
*
* @brief   Invalidate Policer Entry
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - policer entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEntryInvalidate
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  entryIndex
);

/**
* @internal cpssDxChPolicerEntryMeterParamsCalculate function
* @endinternal
*
* @brief   Calculate Token Bucket parameters in the Application format without
*         HW update.
*         The token bucket parameters are returned as output values. This is due to
*         the hardware rate resolution, the exact rate or burst size requested may
*         not be honored. The returned value gives the user the actual parameters
*         configured in the hardware.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tbInParamsPtr            - pointer to Token bucket input parameters
*
* @param[out] tbOutParamsPtr           - pointer to Token bucket output paramters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEntryMeterParamsCalculate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_TB_PARAMS_STC        *tbInParamsPtr,
    OUT CPSS_DXCH_POLICER_TB_PARAMS_STC        *tbOutParamsPtr
);

/**
* @internal cpssDxChPolicerCountersGet function
* @endinternal
*
* @brief   Get Policer Counters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - policing counters set index (APPLICABLE RANGES: 0..15)
*
* @param[out] countersPtr              - pointer to counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerCountersGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            counterSetIndex,
    OUT CPSS_DXCH_POLICER_COUNTERS_STC    *countersPtr

);

/**
* @internal cpssDxChPolicerCountersSet function
* @endinternal
*
* @brief   Set Policer Counters.
*         To reset counters use zero values.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] counterSetIndex          - policing counters set index (APPLICABLE RANGES: 0..15)
* @param[in] countersPtr              - pointer to counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerCountersSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          counterSetIndex,
    IN  CPSS_DXCH_POLICER_COUNTERS_STC  *countersPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPolicerh */


