/********************************************************************************
 *              (c), Copyright 2018, Marvell International Ltd.                 *
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
 * @file tgfQcnGen.h
 *
 * @brief Generic API for Qcn Trigger configuration.
 *
 * @version   1
 ********************************************************************************
 */
#ifndef __tgfQcnGenh
#define __tgfQcnGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>

/**
 * @enum PRV_TGF_PORT_CN_MODE_ENT
 *
 * @brief Enable QCN Processing
 * Indicates how a received CN Message is processed by the device.
*/
typedef enum{

    /** Disable */
    PRV_TGF_PORT_CN_MODE_DISABLE_E = GT_FALSE,

    /** @brief Enable:
     *  Data extracted from header is sent along with the packet descriptor.
     */
    PRV_TGF_PORT_CN_MODE_ENABLE_E  = GT_TRUE,

    /** @brief Reaction Point:
     *  Packet is dropped and data extracted from header is sent out of band.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2)
     */
    PRV_TGF_PORT_CN_MODE_REACTION_POINT_E = 2

} PRV_TGF_PORT_CN_MODE_ENT;

/**
 * @enum PRV_TGF_PORT_CN_LIMIT_ENT
 *
 * @brief Enable QCN Limit
 * Indicates how a QCN is triggered based on queue/port limit.
*/
typedef enum{

    /** @brief Queue Limit
     *  QCN is trigger based on Queue Limits..
     */
    PRV_TGF_PORT_CN_QUEUE_LIMIT_E,

    /** @brief Port Limit
     *  QCN is triggered based on Port Profile Limits..
     *  (APPLICABLE DEVICES: Falcon)
     */
    PRV_TGF_PORT_CN_PORT_LIMIT_E,

    /** @brief Queue/Port Limit
     *  QCN is triggered based on either Queue/Port Limits.
     *  (APPLICABLE DEVICES: Falcon)
     */
    PRV_TGF_PORT_CN_QUEUE_AND_PORT_LIMIT_E

} PRV_TGF_PORT_CN_LIMIT_ENT;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfPortCnModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification (CN) mechanism.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - Enable/disable congestion notification mechanism.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Do not use the global enable/disable CCFC under traffic - instead use
*       prvTgfPortCnQueueAwareEnableSet to enable/disable queue for BCN.
*
*/
GT_STATUS prvTgfPortCnModeEnableSet
(
    IN GT_U8                      devNum,
    IN PRV_TGF_PORT_CN_MODE_ENT   enable
);

/**
* @internal prvTgfPortCnModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of Congestion Notification (CN) mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of Congestion Notification mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnModeEnableGet
(
    IN  GT_U8                      devNum,
    OUT CPSS_DXCH_PORT_CN_MODE_ENT *enablePtr
);

/**
* @internal prvTgfPortCnMessageTriggeringStateSet function
* @endinternal
*
* @brief   Sets CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] qcnSampleInterval        -  parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
* @param[in] qlenOld                  - a snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
GT_STATUS prvTgfPortCnMessageTriggeringStateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    IN  GT_U32                  qcnSampleInterval,
    IN  GT_U32                  qlenOld
);

/**
* @internal prvTgfPortCnMessageTriggeringStateGet function
* @endinternal
*
* @brief   Gets CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] qcnSampleIntervalPtr     -  (pointer to) parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
* @param[out] qlenOldPtr               - (pointer to) snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
GT_STATUS prvTgfPortCnMessageTriggeringStateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_U32                  *qcnSampleIntervalPtr,
    OUT GT_U32                  *qlenOldPtr
);

/**
* @internal prvTgfPortCnEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CN frames.
*         This EtherType also inserted in the CN header of generated CN frames.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] etherType                - EtherType for CN frames (0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range etherType values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CN EtherType must be unique in the system.
*
*/
GT_STATUS prvTgfPortCnEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType
);

/**
* @internal prvTgfPortCnEtherTypeGet function
* @endinternal
*
* @brief   Gets the EtherType for CN frames. The EtherType inserted in the CN
*         header.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherTypePtr             - (pointer to) EtherType for CN frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnEtherTypeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *etherTypePtr
);

/**
* @internal prvTgfPortCnProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS prvTgfPortCnProfileQueueConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
);

/**
* @internal prvTgfPortCnProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnProfileQueueConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U8                               tcQueue,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
);

/**
* @internal prvTgfPortCnFbCalcConfigSet function
* @endinternal
*
* @brief   Sets Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To implement CCFC functionality set fbCalcCfgPtr fields to:
*       deltaEnable = 0
*
*/
GT_STATUS prvTgfPortCnFbCalcConfigSet
(
    IN GT_U8                                  devNum,
    IN CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
);

/**
* @internal prvTgfPortCnFbCalcConfigGet function
* @endinternal
*
* @brief   Gets Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnFbCalcConfigGet
(
    IN  GT_U8                                  devNum,
    OUT CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
);

/**
* @internal prvTgfPortCnSampleEntrySet function
* @endinternal
*
* @brief   Sets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
* @param[in] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.To implement CCFC functionality set entryPtr fields to:
*       interval = (MTU+256b)/16
*       randBitmap = 0
*       2.To achieve uniform distribution of random values, clear the interval
*       to be randomized to 0. For example, if interval = 0xF0 and
*       randBitmap = 0x0F then the sampling interval is a random value
*       in [0xF0,0xFF].
*
*/
GT_STATUS prvTgfPortCnSampleEntrySet
(
    IN GT_U8                                  devNum,
    IN GT_U8                                  entryIndex,
    IN CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
);

/**
* @internal prvTgfPortCnSampleEntryGet function
* @endinternal
*
* @brief   Gets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
*
* @param[out] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnSampleEntryGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U8                                  entryIndex,
    OUT CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
);

/**
* @internal prvTgfPortCnQueueStatusModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable queue status mode. When enabled, CNM is sent to a
*         configurable target that examines the queue status.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[in] targetHwDev              Target HW device number for the CNM.
*                                      Relevant only if enable==GT_TRUE.
* @param[in] targetPort               - Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range targetDev or targetPort
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnQueueStatusModeEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable,
    IN GT_HW_DEV_NUM    targetHwDev,
    IN GT_PORT_NUM      targetPort
);

/**
* @internal prvTgfPortCnQueueStatusModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of queue status mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of queue status mode.
*                                      GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[out] targetHwDevPtr           (pointer to) Target HW device number for the CNM.
*                                      Relevant only if enable==GT_TRUE.
* @param[out] targetPortPtr            - (pointer to) Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnQueueStatusModeEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr,
    OUT GT_HW_DEV_NUM   *targetHwDevPtr,
    OUT GT_PORT_NUM     *targetPortPtr
);

/**
* @internal prvTgfPortTxBindPortToDpSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfPortTxBindPortToDpSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT  profileSet
);

/**
* @internal prvTgfPortTxBindPortToDpGet function
* @endinternal
*
* @brief   Get Drop Profile Set according to a Physical Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] profileSetPtr            - Pointer to the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxBindPortToDpGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT   *profileSetPtr
);

/**
* @internal prvTgfPortCnMessageGenerationConfigSet function
* @endinternal
*
* @brief   Sets CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnMessageGenerationConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
);

/**
* @internal prvTgfPortCnMessageGenerationConfigGet function
* @endinternal
*
* @brief   Gets CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @param[out] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnMessageGenerationConfigGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
);

/**
 * @internal prvTgfPortCnDbaModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable Dynamic Buffers Allocation (DBA) mode of resource
 *         management.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  DBA mode enabled
 *                                      GT_FALSE - DBA mode disabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnDbaModeEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
 );

/**
 * @internal prvTgfPortCnDbaModeEnableGet function
 * @endinternal
 *
 * @brief   Get state of Dynamic Buffers Allocation (DBA) mode of resource
 *         management.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] enablePtr                - pointer to DBA mode state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnDbaModeEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
);

/**
 * @internal prvTgfPortCnDbaAvailableBuffSet function
 * @endinternal
 *
 * @brief   Set amount of buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
 * @param[in] devNum                   - device number
 * @param[in] availableBuff            - amount of buffers available for dynamic allocation for
 *                                      CN (APPLICABLE RANGES: 0..0xFFFFF).
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnDbaAvailableBuffSet
(
 IN  GT_U8           devNum,
 IN  GT_U32          availableBuff
);

/**
 * @internal prvTgfPortCnDbaAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic allocation for CN.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnDbaAvailableBuffGet
(
 IN  GT_U8           devNum,
 OUT GT_U32          *availableBuffPtr
);

/**
* @internal prvTgfPortCnTerminationEnableSet function
* @endinternal
*
* @brief   Enable/Disable termination of CNM (Congestion Notification Message) on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnTerminationEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL                enable
);

/**
* @internal prvTgfPortCnTerminationEnableGet function
* @endinternal
*
* @brief   Gets the status of CNM termination on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnTerminationEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr
);

/**
* @internal prvTgfPortCnDbaPoolAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of pool buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] poolNum                  - pool number for QCN DBA buffers
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] poolAvailableBuff        - amount of pool buffers available for dynamic allocation for CN.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong pool number or device
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnDbaPoolAvailableBuffSet
(
    IN  GT_U8                  devNum,
    IN  GT_U32                poolNum,
    IN  GT_U32                poolAvailableBuff
);

/**
* @internal prvTgfPortCnDbaPoolAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of pool buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] poolNum                  - pool number for QCN DBA buffers
*                                      (APPLICABLE RANGES: 0..1)
* @param[out] poolAvailableBuffPtr    - pointer to amount of pool buffers available for dynamic allocation for CN.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong pool number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnDbaPoolAvailableBuffGet
(
    IN  GT_U8                  devNum,
    IN  GT_U32                 poolNum,
    OUT GT_U32                 *poolAvailableBuffPtr
);

/**
* @internal prvTgfPortCnQcnTriggerSet function
* @endinternal
*
* @brief   Set the QCN trigger thresholds.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - index to trigger threshold table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong index number or device
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnQcnTriggerSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC *qcnTriggerPtr
);

/**
* @internal prvTgfPortCnQcnTriggerGet function
* @endinternal
*
* @brief   Get the configured QCN trigger thresholds.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - index to trigger threshold table
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong index number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnQcnTriggerGet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 index,
    OUT  CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC   *qcnTriggerPtr
);

/**
* @internal prvTgfPortCnCNMEnableSet function
* @endinternal
*
* @brief   Enable/disable CNM generation for CN untagged frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE  - CNM generation enabled
*                                       GT_FALSE - CNM generation disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfPortCnCNMEnableSet
(
    IN GT_U8     devNum,
    IN  GT_BOOL  enable
);

/**
* @internal prvTgfPortCnCNMEnableGet function
* @endinternal
*
* @brief   Get state of CNM enable for CN untagged frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[out] enablePtr               - pointer to CNM enable state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnCNMEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfPortCnProfilePortConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given port profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS prvTgfPortCnProfilePortConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
);

/**
* @internal prvTgfPortCnProfilePortConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given port profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[out] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnProfilePortConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
);

/**
* @internal prvTgfPortCnCongestedQPriorityLocationSet function
* @endinternal
*
* @brief   Sets location of the congested q/priority bits in the CNM header for
*          parsing and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                        - device number.
* @param[in] congestedQPriorityLocation    - congested q/priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnCongestedQPriorityLocationSet
(
    IN GT_U8                                               devNum,
    IN CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT congestedQPriorityLocation
);

/**
* @internal prvTgfPortCnCongestedQPriorityLocationGet function
* @endinternal
*
* @brief   Gets location of congested q/priority bits in CNM header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         - device number.
* @param[out] congestedQPriorityLocationPtr - (pointer to) congested q/priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnCongestedQPriorityLocationGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT *congestedQPriorityLocationPtr
);

/**
* @internal prvTgfPortTxTailDropGlobalParamsSet function
* @endinternal
*
* @brief   Set amount of available buffers for QCN DBA.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  Device number
* @param[in] resourceMode             -  Defines which free buffers resource is used to calculate the
*                                       dynamic buffer limit for the Port/Queue limits(global or pool)
* @param[in] globalAvailableBuffers   -  Global amount of available buffers for dynamic buffers allocation
* @param[in] pool0AvailableBuffers    -  Pool 0 amount of available buffers for dynamic buffers allocation
* @param[in] pool1AvailableBuffers    -  Pool 1 amount of available buffers for dynamic buffers allocation
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropGlobalParamsSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT  resourceMode,
    IN  GT_U32                                    globalAvailableBuffers,
    IN  GT_U32                                    pool0AvailableBuffers,
    IN  GT_U32                                    pool1AvailableBuffers
);

/**
* @internal prvTgfPortTxTailDropGlobalParamsGet function
* @endinternal
*
* @brief   Get amount of available buffers for QCN DBA.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  Device number
* @param[in] resourceMode             -  Defines which free buffers resource is used to calculate the
*                                       dynamic buffer limit for the Port/Queue limits(global or pool)
* @param[in] globalAvailableBuffers   - (pointer to) Global amount of available buffers for dynamic buffers allocation
* @param[in] pool0AvailableBuffers    - (pointer to) Pool 0 amount of available buffers for dynamic buffers allocation
* @param[in] pool1AvailableBuffers    - (pointer to) Pool 1 amount of available buffers for dynamic buffers allocation
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropGlobalParamsGet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT  *resourceMode,
    IN  GT_U32                                    *globalAvailableBuffers,
    IN  GT_U32                                    *pool0AvailableBuffers,
    IN  GT_U32                                    *pool1AvailableBuffers
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPortGenh */


