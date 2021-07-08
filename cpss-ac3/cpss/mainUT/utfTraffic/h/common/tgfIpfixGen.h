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
* @file tgfIpfixGen.h
*
* @brief Generic API for IPFIX
*
* @version   3
********************************************************************************
*/
#ifndef __tgfIpfixGenh
#define __tgfIpfixGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#endif /* CHX_FAMILY */

/* length of array to retrieve alarm events */
#define PRV_TGF_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS 16
        
/**
* @enum PRV_TGF_IPFIX_SAMPLING_ACTION_ENT
 *
 * @brief Enumeration for IPFIX sampling action.
*/
typedef enum{

    /** @brief Alarm, each sampled packet will
     *  trigger an interrupt to cpu.
     */
    PRV_TGF_IPFIX_SAMPLING_ACTION_ALARM_E,

    /** @brief Mirror, each sampled packet will
     *  change packet command to Mirror.
     */
    PRV_TGF_IPFIX_SAMPLING_ACTION_MIRROR_E

} PRV_TGF_IPFIX_SAMPLING_ACTION_ENT;

/**
* @enum PRV_TGF_IPFIX_SAMPLING_DIST_ENT
 *
 * @brief Enumeration for IPFIX sampling distribution.
*/
typedef enum{

    /** @brief Deterministic, the random
     *  offset is deterministic and
     *  equals zero.
     */
    PRV_TGF_IPFIX_SAMPLING_DIST_DETERMINISTIC_E,

    /** @brief Random, the random_offset
     *  is a unified distributed
     *  random number which is
     *  picked for each packet.
     */
    PRV_TGF_IPFIX_SAMPLING_DIST_RANDOM_E

} PRV_TGF_IPFIX_SAMPLING_DIST_ENT;

/**
* @enum PRV_TGF_IPFIX_SAMPLING_MODE_ENT
 *
 * @brief Enumeration for IPFIX sampling mode.
*/
typedef enum{

    /** Disable sampling. */
    PRV_TGF_IPFIX_SAMPLING_MODE_DISABLE_E,

    /** Packet count based sampling. */
    PRV_TGF_IPFIX_SAMPLING_MODE_PACKET_E,

    /** Byte count based sampling. */
    PRV_TGF_IPFIX_SAMPLING_MODE_BYTE_E,

    /** Time based sampling. */
    PRV_TGF_IPFIX_SAMPLING_MODE_TIME_E

} PRV_TGF_IPFIX_SAMPLING_MODE_ENT;

/**
* @enum PRV_TGF_IPFIX_DROP_COUNT_MODE_ENT
 *
 * @brief Enumeration for IPFIX drop counter selected mode.
*/
typedef enum{

    /** @brief The drop counter is updated
     *  only for packets dropped due
     *  to the Policer engine.
     */
    PRV_TGF_IPFIX_DROP_COUNT_MODE_METER_ONLY_E,

    /** @brief The drop counter is updated
     *  for drops up to and
     *  including the Policer engine.
     */
    PRV_TGF_IPFIX_DROP_COUNT_MODE_ALL_E

} PRV_TGF_IPFIX_DROP_COUNT_MODE_ENT;

/**
* @enum PRV_TGF_IPFIX_WRAPAROUND_ACTION_ENT
 *
 * @brief Enumeration for IPFIX counter action when wraparound occurs.
*/
typedef enum{

    /** @brief Freeze the counter at the
     *  threshold value and stop
     *  performing any updates.
     */
    PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E,

    /** @brief Subtract the threshold value
     *  from the current value of the
     *  counter and continue updates.
     */
    PRV_TGF_IPFIX_WRAPAROUND_ACTION_CLEAR_E

} PRV_TGF_IPFIX_WRAPAROUND_ACTION_ENT;

/**
* @enum PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT
 *
 * @brief Enumeration for loading of IPFIX time stamp method.
*/
typedef enum{

    /** @brief IPfix timer is
     *  uploaded to the exact
     *  value of time stamp
     *  upload registers.
     */
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E,

    /** @brief IPfix timer is
     *  incremented by the
     *  value of time stamp
     *  upload registers.
     */
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E

} PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT;

/**
* @struct PRV_TGF_IPFIX_ENTRY_STC
 *
 * @brief The IPFIX entry.
*/
typedef struct{

    /** @brief Time stamp from RTC counter. 16 bits.
     *  packetCounter    - packets counter. 30 bits.
     */
    GT_U32 timeStamp;

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
    PRV_TGF_IPFIX_SAMPLING_ACTION_ENT samplingAction;

    /** @brief Defines the distribution range of the
     *  randomOffset. Relevant only when randomFlag is
     *  random. 6 bits.
     */
    GT_U32 logSamplingRange;

    /** @brief Determines the random offset used for the
     *  reference value calculation.
     */
    PRV_TGF_IPFIX_SAMPLING_DIST_ENT randomFlag;

    /** Sampling base method. */
    PRV_TGF_IPFIX_SAMPLING_MODE_ENT samplingMode;

    /** @brief 2 lsb bits of the cpu code for sampled packets.
     *  Relevant only when samplingAction is Mirror.
     *  Comment:
     */
    GT_U32 cpuSubCode;

} PRV_TGF_IPFIX_ENTRY_STC;

/**
* @struct PRV_TGF_IPFIX_WRAPAROUND_CFG_STC
 *
 * @brief IPFIX wraparound configuration.
*/
typedef struct{

    /** @brief The action when a counter reaches its
     *  threshold value.
     */
    PRV_TGF_IPFIX_WRAPAROUND_ACTION_ENT action;

    /** Drop packet counter threshold. 30 bits. */
    GT_U32 dropThreshold;

    /** Packets counter threshold. 30 bits. */
    GT_U32 packetThreshold;

    /** @brief Bytes counter threshold. 36 bits.
     *  Comment:
     */
    GT_U64 byteThreshold;

} PRV_TGF_IPFIX_WRAPAROUND_CFG_STC;

/**
* @struct PRV_TGF_IPFIX_TIMER_STC
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

} PRV_TGF_IPFIX_TIMER_STC;

/**
* @struct PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC
 *
 * @brief IPFIX timestamp upload configuration.
*/
typedef struct{

    /** @brief timestamp upload mode.
     *  ipfixTimer - IPFIX timer structure.
     *  Comment:
     */
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT uploadMode;

    PRV_TGF_IPFIX_TIMER_STC timer;

} PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC;

/**
* @internal prvTgfIpfixEntrySet function
* @endinternal
*
* @brief   Sets IPFIX entry.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
* @param[in] ipfixEntryPtr            - pointer to the IPFIX Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - on value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixEntrySet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                            entryIndex,
    IN  PRV_TGF_IPFIX_ENTRY_STC           *ipfixEntryPtr
);

/**
* @internal prvTgfIpfixEntryGet function
* @endinternal
*
* @brief   Gets IPFIX entry.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - index of IPFIX Entry.
* @param[in] reset                    -  flag:
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
*/
GT_STATUS prvTgfIpfixEntryGet
(
    IN  GT_U8                             devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                            entryIndex,
    IN  GT_BOOL                           reset,
    OUT PRV_TGF_IPFIX_ENTRY_STC           *ipfixEntryPtr
);

/**
* @internal prvTgfIpfixDropCountModeSet function
* @endinternal
*
* @brief   Configures IPFIX drop counting mode.
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
GT_STATUS prvTgfIpfixDropCountModeSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_IPFIX_DROP_COUNT_MODE_ENT    mode
);

/**
* @internal prvTgfIpfixDropCountModeGet function
* @endinternal
*
* @brief   Gets IPFIX drop counting mode.
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
GT_STATUS prvTgfIpfixDropCountModeGet
(
    IN GT_U8                                  devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    OUT PRV_TGF_IPFIX_DROP_COUNT_MODE_ENT     *modePtr
);

/**
* @internal prvTgfIpfixCpuCodeSet function
* @endinternal
*
* @brief   Sets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] cpuCode                  - A cpu code to set the 6 most significant bits.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or cpu code.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixCpuCodeSet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN CPSS_NET_RX_CPU_CODE_ENT       cpuCode
);

/**
* @internal prvTgfIpfixCpuCodeGet function
* @endinternal
*
* @brief   Gets the 6 most significant bits of the CPU Code in case of mirroring.
*         The two least significant bits are taken from the IPfix entry.
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
GT_STATUS prvTgfIpfixCpuCodeGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT      *cpuCodePtr
);

/**
* @internal prvTgfIpfixAgingEnableSet function
* @endinternal
*
* @brief   Enables or disabled the activation of aging for IPfix.
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
GT_STATUS prvTgfIpfixAgingEnableSet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfIpfixAgingEnableGet function
* @endinternal
*
* @brief   Gets enabling status of aging for IPfix.
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
GT_STATUS prvTgfIpfixAgingEnableGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal prvTgfIpfixWraparoundConfSet function
* @endinternal
*
* @brief   Configures IPFIX wraparound parameters.
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
*/
GT_STATUS prvTgfIpfixWraparoundConfSet
(
    IN GT_U8                                  devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    IN PRV_TGF_IPFIX_WRAPAROUND_CFG_STC       *confPtr
);

/**
* @internal prvTgfIpfixWraparoundConfGet function
* @endinternal
*
* @brief   Get IPFIX wraparound configuration.
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
GT_STATUS prvTgfIpfixWraparoundConfGet
(
    IN GT_U8                                  devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT         stage,
    OUT PRV_TGF_IPFIX_WRAPAROUND_CFG_STC      *confPtr
);

/**
* @internal prvTgfIpfixWraparoundStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of wraparound entries indexes.
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
GT_STATUS prvTgfIpfixWraparoundStatusGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                         startIndex,
    IN GT_U32                         endIndex,
    IN GT_BOOL                        reset,
    OUT GT_U32                        *bmpPtr
);

/**
* @internal prvTgfIpfixAgingStatusGet function
* @endinternal
*
* @brief   Retrieves a bitmap of aged entries indexes.
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
GT_STATUS prvTgfIpfixAgingStatusGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32                         startIndex,
    IN GT_U32                         endIndex,
    IN GT_BOOL                        reset,
    OUT GT_U32                        *bmpPtr
);

/**
* @internal prvTgfIpfixTimestampUploadSet function
* @endinternal
*
* @brief   Configures IPFIX timestamp upload per stages.
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
*       "prvTgfIpfixTimestampUploadTrigger".
*
*/
GT_STATUS prvTgfIpfixTimestampUploadSet
(
    IN GT_U8                                      devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    IN PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC    *uploadPtr
);

/**
* @internal prvTgfIpfixTimestampUploadGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload configuration for a stage.
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
GT_STATUS prvTgfIpfixTimestampUploadGet
(
    IN GT_U8                                      devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT             stage,
    OUT PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC   *uploadPtr
);

/**
* @internal prvTgfIpfixTimestampUploadTrigger function
* @endinternal
*
* @brief   Triggers the IPFIX timestamp upload operation.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_STATE             - if previous upload operation is not
*                                       finished yet.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixTimestampUploadTrigger
(
    IN GT_U8    devNum
);

/**
* @internal prvTgfIpfixTimestampUploadStatusGet function
* @endinternal
*
* @brief   Gets IPFIX timestamp upload operation status.
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
GT_STATUS prvTgfIpfixTimestampUploadStatusGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *uploadStatusPtr
);

/**
* @internal prvTgfIpfixTimerGet function
* @endinternal
*
* @brief   Gets IPFIX timer
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
GT_STATUS prvTgfIpfixTimerGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT PRV_TGF_IPFIX_TIMER_STC       *timerPtr
);

/**
* @internal prvTgfIpfixAlarmEventsGet function
* @endinternal
*
* @brief   Retrieves up to 16 IPFIX entries indexes where alarm events (due to
*         sampling) occured.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] eventsArr[PRV_TGF_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS] - array of alarm events, each valid element contains
*                                      the index of IPFIX entry which caused the alarm.
* @param[out] eventsNumPtr             - The number of valid entries in eventsPtr list.
*                                      Range: 0..15
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixAlarmEventsGet
(
    IN GT_U8                          devNum,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    OUT GT_U32                        eventsArr[PRV_TGF_IPFIX_ALARM_EVENTS_ARRAY_LENGTH_CNS],
    OUT GT_U32                        *eventsNumPtr
);

/**
* @internal prvTgfIpfixTimestampToCpuEnableSet function
* @endinternal
*
* @brief   Configures all TO_CPU DSA tags to include a Timestamp.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - GT_TRUE: TO_CPU DSA tag contains a Timestamp.
*                                      GT_FALSE: TO_CPU DSA tag does not contain a Timestamp.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS prvTgfIpfixTimestampToCpuEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfIpfixTimestampToCpuEnableGet function
* @endinternal
*
* @brief   Gets enabling status of TO_CPU DSA tags including a timestamp.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
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
GT_STATUS prvTgfIpfixTimestampToCpuEnableGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal prvTgfIpfixIsResetOnReadErratum function
* @endinternal
*
*
* @retval GT_TRUE                  - on erratum exist
* @retval GT_FALSE                 - on erratum does not exist
*/
GT_BOOL prvTgfIpfixIsResetOnReadErratum
(
    IN GT_U8     devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfIpfixGenh */


