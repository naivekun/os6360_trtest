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
* @file cpssDxChIpfix.h
*
* @brief CPSS APIs declarations and definitions for IP Flow Information Export
* (IPFIX).
*
* @version   7
********************************************************************************
*/

#ifndef __cpssDxChIpfixh
#define __cpssDxChIpfixh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>

/* length of array to retrieve alarm events */
#define CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS   16

/**
* @enum CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT
 *
 * @brief Enumeration for IPFIX sampling action.
*/
typedef enum{

    /** @brief Alarm, each sampled packet will
     *  trigger an interrupt to cpu.
     */
    CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E,

    /** @brief Mirror, each sampled packet will
     *  change packet command to Mirror.
     */
    CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E

} CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT;

/**
* @enum CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT
 *
 * @brief Enumeration for IPFIX sampling distribution.
*/
typedef enum{

    /** @brief Deterministic, the random
     *  offset is deterministic and
     *  equals zero.
     */
    CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E,

    /** @brief Random, the random_offset
     *  is a unified distributed
     *  random number which is
     *  picked for each packet.
     */
    CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E

} CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT;

/**
* @enum CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT
 *
 * @brief Enumeration for IPFIX sampling mode.
*/
typedef enum{

    /** Disable sampling. */
    CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E,

    /** Packet count based sampling. */
    CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E,

    /** Byte count based sampling. */
    CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E,

    /** Time based sampling. */
    CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E

} CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT;

/**
* @enum CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT
 *
 * @brief Enumeration for IPFIX drop counter selected mode.
*/
typedef enum{

    /** @brief The drop counter is updated
     *  only for packets dropped due
     *  to the Policer engine.
     */
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E,

    /** @brief The drop counter is updated
     *  for drops up to and
     *  including the Policer engine.
     */
    CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E

} CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT;

/**
* @enum CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT
 *
 * @brief Enumeration for IPFIX counter action when wraparound occurs.
*/
typedef enum{

    /** @brief Freeze the counter at the
     *  threshold value and stop
     *  performing any updates.
     */
    CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E,

    /** @brief Subtract the threshold value
     *  from the current value of the
     *  counter and continue updates.
     */
    CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E

} CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT;

/**
* @enum CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT
 *
 * @brief Enumeration for loading of IPFIX time stamp method.
*/
typedef enum{

    /** @brief IPfix timer is
     *  uploaded to the exact
     *  value of time stamp
     *  upload registers.
     */
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E,

    /** @brief IPfix timer is
     *  incremented by the
     *  value of time stamp
     *  upload registers.
     */
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E

} CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT;

/**
* @struct CPSS_DXCH_IPFIX_ENTRY_STC
 *
 * @brief The IPFIX entry.
*/
typedef struct{

    /** Time stamp from RTC counter. 16 bits. */
    GT_U32 timeStamp;

    /** packets counter. 30 bits. */
    GT_U32 packetCount;

    /** bytes counter. 36 bits. */
    GT_U64 byteCount;

    /** Drop packet counter. 30 bits. */
    GT_U32 dropCounter;

    /** Picked random value after each sampling. 32 bits. */
    GT_U32 randomOffset;

    /** @brief Once the packet is sampled, it contains the
     *  summary of the previous lastSampledValue +
     *  samplingWindow. 36 bits.
     */
    GT_U64 lastSampledValue;

    /** @brief Window used for Reference value calculation. The
     *  units of this window are according to
     *  samplingMode. 36 bits.
     */
    GT_U64 samplingWindow;

    /** Command done on sampled packets. */
    CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT samplingAction;

    /** @brief Defines the distribution range of the
     *  randomOffset. Relevant only when randomFlag is
     *  random. 6 bits.
     */
    GT_U32 logSamplingRange;

    /** @brief Determines the random offset used for the
     *  reference value calculation.
     */
    CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT randomFlag;

    /** Sampling base method. */
    CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT samplingMode;

    /** @brief 2 lsb bits of the cpu code for sampled packets.
     *  Relevant only when samplingAction is Mirror.
     *  Comment:
     */
    GT_U32 cpuSubCode;

} CPSS_DXCH_IPFIX_ENTRY_STC;

/**
* @struct CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC
 *
 * @brief IPFIX wraparound configuration.
*/
typedef struct{

    /** @brief The action when a counter reaches its
     *  threshold value.
     */
    CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT action;

    /** Drop packet counter threshold. 30 bits. */
    GT_U32 dropThreshold;

    /** Packets counter threshold. 30 bits. */
    GT_U32 packetThreshold;

    /** @brief Bytes counter threshold. 36 bits. This Threshold
     *  minimal allowed value is 0x40000 and 18 LSBs should
     *  be 0.
     *  Comment:
     */
    GT_U64 byteThreshold;

} CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC;

/**
* @struct CPSS_DXCH_IPFIX_TIMER_STC
 *
 * @brief IPFIX timer.
*/
typedef struct{

    /** IPFIX nano second timer. 30 bits. */
    GT_U32 nanoSecondTimer;

    /** @brief IPFIX second timer. 64 bits.
     *  Comment:
     */
    GT_U64 secondTimer;

} CPSS_DXCH_IPFIX_TIMER_STC;

/**
* @struct CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC
 *
 * @brief IPFIX timestamp upload configuration.
*/
typedef struct{

    /** timestamp upload mode. */
    CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT uploadMode;

    /** @brief IPFIX timer structure.
     *  Comment:
     */
    CPSS_DXCH_IPFIX_TIMER_STC timer;

} CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC;

/**
* @internal cpssDxChIpfixEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In order to set IPFIX entry under traffic perform the following
*       algorithm(none-zero setting of IPFIX Counters while enabled\still
*       under traffic may cause data coherency problems):
*       - disconnect entry from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entry back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxCh3PolicerBillingEntrySet".
*
*/
GT_STATUS cpssDxChIpfixEntrySet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
);

/**
* @internal cpssDxChIpfixEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag (please refer to COMMENTS section for further
*                                      explanations):
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read entry operation only.
*
* @param[out] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note When counters are read in clear on read mode (reset == GT_TRUE), The
*       following actions are done on the different IPFIX entry fields:
*       - The <Packet Counter>, <Byte Counter>, and <Dropped Packet Counter>
*       fields are cleared.
*       - The <Timestamp> field is updated to the current time.
*       - The <Last Sampled Value> is cleared.
*
*/
GT_STATUS cpssDxChIpfixEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  GT_BOOL                             reset,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
);

/**
* @internal cpssDxChIpfixDropCountModeSet function
* @endinternal
*
* @brief   Configures IPFIX drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - drop counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixDropCountModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT      mode
);

/**
* @internal cpssDxChIpfixDropCountModeGet function
* @endinternal
*
* @brief   Gets IPFIX drop counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - pointer to drop counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixDropCountModeGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT     *modePtr
);

/**
* @internal cpssDxChIpfixCpuCodeSet function
* @endinternal
*
* @brief   Sets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] cpuCode                  - A cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or cpu code.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixCpuCodeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_NET_RX_CPU_CODE_ENT         cpuCode
);

/**
* @internal cpssDxChIpfixCpuCodeGet function
* @endinternal
*
* @brief   Gets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] cpuCodePtr               - pointer to a cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixCpuCodeGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT        *cpuCodePtr
);

/**
* @internal cpssDxChIpfixAgingEnableSet function
* @endinternal
*
* @brief   Enables or disabled the activation of aging for IPfix.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE for enable, GT_FALSE for disable.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixAgingEnableSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_BOOL                          enable
);

/**
* @internal cpssDxChIpfixAgingEnableGet function
* @endinternal
*
* @brief   Gets enabling status of aging for IPfix.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - pointer to: GT_TRUE for enable, GT_FALSE for disable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixAgingEnableGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal cpssDxChIpfixWraparoundConfSet function
* @endinternal
*
* @brief   Configures IPFIX wraparound parameters.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] confPtr                  - pointer to wraparound configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, action or threshold.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Since there is a HW limitation that wraparound byte threshold 18 LSBs
*       must be set to 0, the function ignores the 18 LSBs input value and set
*       them to 0.
*
*/
GT_STATUS cpssDxChIpfixWraparoundConfSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC       *confPtr
);

/**
* @internal cpssDxChIpfixWraparoundConfGet function
* @endinternal
*
* @brief   Get IPFIX wraparound configuration.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] confPtr                  - pointer to wraparound configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixWraparoundConfGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC      *confPtr
);

/**
* @internal cpssDxChIpfixWraparoundStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get wraparound indication on.
* @param[in] endIndex                 - the last entry index to get wraparound indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication wraparound.
*                                      each bit indicate: 0 - no WA occured, 1 - WA occured.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS cpssDxChIpfixWraparoundStatusGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
);

/**
* @internal cpssDxChIpfixAgingStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get aging indication on.
* @param[in] endIndex                 - the last entry index to get aging indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication aging.
*                                      each bit indicate: 0 - no packet recieved on flow (aged).
*                                      1 - packet recieved on flow.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS cpssDxChIpfixAgingStatusGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
);

/**
* @internal cpssDxChIpfixTimestampUploadSet function
* @endinternal
*
* @brief   Configures IPFIX timestamp upload per stages.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] uploadPtr                - pointer to timestamp upload configuration of a stage.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or upload mode.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Actual upload operation is triggered by
*       "cpssDxChIpfixTimestampUploadTrigger".
*
*/
GT_STATUS cpssDxChIpfixTimestampUploadSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC    *uploadPtr
);

/**
* @internal cpssDxChIpfixTimestampUploadGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload configuration for a stage.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] uploadPtr                - pointer to timestamp upload configuration of a stage.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimestampUploadGet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC   *uploadPtr
);

/**
* @internal cpssDxChIpfixTimestampUploadTrigger function
* @endinternal
*
* @brief   Triggers the IPFIX timestamp upload operation.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_STATE             - if previous upload operation is not
*                                       finished yet.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_STAGE_INGRESS_0_E triggering controls
*       CPSS_DXCH_POLICER_STAGE_INGRESS_1 & CPSS_DXCH_POLICER_STAGE_EGRESS_E
*       as well, all uploads are done simultenously.
*
*/
GT_STATUS cpssDxChIpfixTimestampUploadTrigger
(
    IN GT_U8    devNum
);

/**
* @internal cpssDxChIpfixTimestampUploadStatusGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload operation status.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] uploadStatusPtr          - pointer to indication whether upload was done.
*                                      GT_TRUE: upload is finished.
*                                      GT_FALSE: upload still in progess.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimestampUploadStatusGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *uploadStatusPtr
);

/**
* @internal cpssDxChIpfixTimerGet function
* @endinternal
*
* @brief   Gets IPFIX timer
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] timerPtr                 - pointer to IPFIX timer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimerGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_DXCH_IPFIX_TIMER_STC       *timerPtr
);

/**
* @internal cpssDxChIpfixAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] - array of alarm events, each valid element contains
*                                      the index of IPFIX entry which caused the alarm.
* @param[out] eventsNumPtr             - The number of valid entries in eventsPtr list.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixAlarmEventsGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                          eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                          *eventsNumPtr
);

/**
* @internal cpssDxChIpfixPortGroupEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In order to set IPFIX entry under traffic perform the following
*       algorithm(none-zero setting of IPFIX Counters while enabled\still
*       under traffic may cause data coherency problems):
*       - disconnect entry from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entry back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxCh3PolicerBillingEntrySet".
*
*/
GT_STATUS cpssDxChIpfixPortGroupEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
);

/**
* @internal cpssDxChIpfixPortGroupEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag (please refer to COMMENTS section for further
*                                      explanations):
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read entry operation only.
*
* @param[out] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note When counters are read in clear on read mode (reset == GT_TRUE), The
*       following actions are done on the different IPFIX entry fields:
*       - The <Packet Counter>, <Byte Counter>, and <Dropped Packet Counter>
*       fields are cleared.
*       - The <Timestamp> field is updated to the current time.
*       - The <Last Sampled Value> is cleared.
*
*/
GT_STATUS cpssDxChIpfixPortGroupEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  GT_BOOL                             reset,
    OUT CPSS_DXCH_IPFIX_ENTRY_STC           *ipfixEntryPtr
);

/**
* @internal cpssDxChIpfixPortGroupWraparoundStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get wraparound indication on.
* @param[in] endIndex                 - the last entry index to get wraparound indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication wraparound.
*                                      each bit indicate: 0 - no WA occured, 1 - WA occured.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS cpssDxChIpfixPortGroupWraparoundStatusGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
);

/**
* @internal cpssDxChIpfixPortGroupAgingStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startIndex               - the first entry index to get aging indication on.
* @param[in] endIndex                 - the last entry index to get aging indication on.
* @param[in] reset                    - clear the bits after read:
*                                      GT_TRUE - clear,
*                                      GT_FALSE - don't clear.
*
* @param[out] bmpPtr                   - pointer to a bitmap indication aging.
*                                      each bit indicate: 0 - no packet recieved on flow (aged).
*                                      1 - packet recieved on flow.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note It is application responsibility to allocate the buffer size for the
*       bitmap(bmpPtr). Since the buffer is made from GT_U32 elements, each
*       element contains 32 indications.
*
*/
GT_STATUS cpssDxChIpfixPortGroupAgingStatusGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                           startIndex,
    IN GT_U32                           endIndex,
    IN GT_BOOL                          reset,
    OUT GT_U32                          *bmpPtr
);

/**
* @internal cpssDxChIpfixPortGroupAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] - array of alarm events, each valid element contains
*                                      the index of IPFIX entry which caused the alarm.
* @param[out] eventsNumPtr             - The number of valid entries in eventsPtr list.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixPortGroupAlarmEventsGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                          eventsArr[CPSS_DXCH_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                          *eventsNumPtr
);

/**
* @internal cpssDxChIpfixTimestampToCpuEnableSet function
* @endinternal
*
* @brief   Configures all TO_CPU DSA tags to include a Timestamp.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimestampToCpuEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChIpfixTimestampToCpuEnableGet function
* @endinternal
*
* @brief   Gets enabling status of TO_CPU DSA tags including a Timestamp.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChIpfixTimestampToCpuEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpfixh */



