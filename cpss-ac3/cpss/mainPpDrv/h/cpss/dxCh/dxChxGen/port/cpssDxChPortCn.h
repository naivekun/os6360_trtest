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
* @file cpssDxChPortCn.h
*
* @brief CPSS DXCH Port Congestion Notification API.
*
* @version   23
********************************************************************************
*/

#ifndef __cpssDxChPortCnh
#define __cpssDxChPortCnh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/generic/port/cpssPortTx.h>

/**
* @enum CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT
 *
 * @brief Enumeration of packet length used by the CN frames triggering logic.
*/
typedef enum{

    /** Original packet length in bytes */
    CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E,

    /** 1.5KB */
    CPSS_DXCH_PORT_CN_LENGTH_1_5_KB_E,

    /** 2KB */
    CPSS_DXCH_PORT_CN_LENGTH_2_KB_E,

    /** 10KB */
    CPSS_DXCH_PORT_CN_LENGTH_10_KB_E

} CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT;

/**
* @enum CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT
 *
 * @brief Enumeration that indicates the locations of the priority/speed
 * bits in the CNM header. (Defines the index in table ....)
*/
typedef enum{

    /** @brief priority/speed bits are in CPID[2:0]. */
    CPSS_DXCH_PORT_CN_PRIORITY_SPEED_CPID_E,

    /** @brief priority/speed bits are in CNheader<Encapsulated UP> field.
     */
    CPSS_DXCH_PORT_CN_SDU_UP_E,

    /** @brief priority/speed bits are in the location of
     *  the speed bits in xCAT CCFC
     */
    CPSS_DXCH_PORT_CN_SPEED_E

} CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT;

/**
* @enum CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT
 *
 * @brief Enumeration describes congestion notification message type
*/
typedef enum{

    /** @brief Quantized Congestion Notification protocol message */
    CPSS_DXCH_PORT_CN_MESSAGE_TYPE_QCN_E,

    /** @brief Cross Chip Flow Control message */
    CPSS_DXCH_PORT_CN_MESSAGE_TYPE_CCFC_E

} CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT;

/**
* @enum CPSS_DXCH_PORT_CN_MODE_ENT
 *
 * @brief Enable QCN Processing
 * Indicates how a received CN Message is processed by the device.
*/
typedef enum{

    /** @brief Disable */
    CPSS_DXCH_PORT_CN_MODE_DISABLE_E = GT_FALSE,

    /** @brief Enable:
     *  Data extracted from header is sent along with the packet descriptor.
     */
    CPSS_DXCH_PORT_CN_MODE_ENABLE_E  = GT_TRUE,

    /** @brief Reaction Point:
     *  Packet is dropped and data extracted from header is sent out of band.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PORT_CN_MODE_REACTION_POINT_E = 2

} CPSS_DXCH_PORT_CN_MODE_ENT;


/**
* @struct CPSS_PORT_CN_PROFILE_CONFIG_STC
 *
 * @brief CN Profile configurations.
*/
typedef struct{

    /** @brief GT_FALSE - CN Blind: Congestion in this queue
     *                    does not generate CN messages.
     *  GT_TRUE - CN Aware: This queue is enabled
     *            for triggering CN messages.
     */
    GT_BOOL cnAware;

    /** @brief buffer limit threshold for triggering CN frame.
     */
    GT_U32 threshold;

    /** @brief configurable factor, which dictates which factor
     *  of the free buffers are available to the port or
     *  queue. The greater the Alpha, the more resources
     *  can be used, and the packet memory utilization
     *  increases. However, greater Alpha results in
     *  slower adaptation to workload changes.
     *
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alpha;

} CPSS_PORT_CN_PROFILE_CONFIG_STC;


/**
* @struct CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC
 *
 * @brief CN Sample Interval Table entry.
*/
typedef struct{

    /** @brief CN sampling interval in resolution of 16 bytes.
     *  (0..0xFFFF)
     */
    GT_U32 interval;

    /** @brief bitmap that indicates which sampling interval bits
     *  are randomize. 0 - don't randomize; 1 - randomize.
     *  (0..0xFFFF)
     */
    GT_U32 randBitmap;

} CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC;

/**
* @struct CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC
 *
 * @brief Feedback calculation configurations.
*/
typedef struct{

    /** @brief Exponent weight for Fb calculation.(-8..7) */
    GT_32 wExp;

    /** @brief bits that are used for qFb calculation.
     *  qFb = Fb[fbLsb+5:fbLsb] (0..0x1F)
     */
    GT_U32 fbLsb;

    /** @brief GT_TRUE: enables Delta (Qlen-QlenOld)
     *         inclusion in Fb calculation
     *  GT_FALSE: Delta (Qlen-QlenOld) is not included
     *  in Fb calculation.
     */
    GT_BOOL deltaEnable;

    /** @brief Minimum value of Fb.
     *  (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0xFFFFFF)
     *  (APPLICABLE RANGES: Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X 0..0xFFFFF )
     *  If |Fb| < FbMin then |Fb| is set to 0.
     */
    GT_U32 fbMin;

    /** @brief Maximum value of |Fb|.
     *  (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0xFFFFFF)
     *  (APPLICABLE RANGES: Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X 0..0xFFFFF )
     *  If |Fb| > FbMax then |Fb| is set to FbMax.
     */
    GT_U32 fbMax;

} CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC;

/**
* @struct CPSS_PORT_CNM_GENERATION_CONFIG_STC
 *
 * @brief CNM generation configurations
*/
typedef struct{

    /** @brief The QoS Profile associated with CNM. (0..0x7F)
     *  (APPLICABLE DEVICES: Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 qosProfileId;

    /** @brief Value of DSA<routed> bit associated with CNM.
     *  Setting this flag to '1' allows the egress device
     *  in a cascaded system to modify the CNM MAC-SA (0,1)
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 isRouted;

    /** @brief Allow to override UP assignment.
     *  GT_FALSE: UP is copied from the sampled frame
     *  GT_TRUE: UP is cnmUp configuration.
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL overrideUp;

    /** @brief The UP assigned to CNM. Used when overrideUp==GT_TRUE
     *  (APPLICABLE DEVICES: Lion; Lion2)
     */
    GT_U8 cnmUp;

    /** @brief The VID associated with CNM if the sampled frame is
     *  untagged.(0..0xFFF)
     *  (APPLICABLE DEVICES: Lion; xCat2; Lion2)
     */
    GT_U16 defaultVlanId;

    /** @brief Scale factor of Qoffset, Qdelta fields in CNM.
     *  The device calculates these fields in buffer resolution.
     *  This configuration provides the conversion between
     *  the buffer size and 64 Bytes. Valid values: 256,512.
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 scaleFactor;

    /** @brief CNM-header<Version> field for CNMs generated by
     *  the device. (0..0xF)
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 version;

    /** @brief CNM-header<CPID[63:10]> field for CNMs generated
     *  by the device (network order).
     * (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 cpidMsb[7];

    /** @brief Enables generation of CNM if the sample frame is not CN
     *  GT_FALSE: Disable; CNM is generated only if the
     *  sampled frame is CN-tagged
     *  GT_TRUE: Enable; CNM is generated regardless of the
     *  CN tag state of the sampled frame.
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL cnUntaggedEnable;

    /** @brief GT_FALSE: QCN is CN
     *  GT_TRUE: QCN is always CN-tagged.A CN-tag is added if the sampled
     *  frame is not CN-tagged.
     *  Relevant only in QCN mode.
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL forceCnTag;

    /** @brief CN-tag FlowID used when a CN-tag is added to QCN frames generated
     *  by the device.Relevant only in QCN mode. (0..0xFFFF)
     *  (APPLICABLE DEVICES: Lion; Lion2)
     */
    GT_U32 flowIdTag;

    /** @brief GT_FALSE: The CNM is 0-padded up to total length of 64B.
     *  GT_TRUE: Append sampled packet. The CNM header is followed by the
     *  SDU starting from the SDU-DA.The entire CNM is truncated to 128B.
     *  Relevant only in QCN mode.
     *  (APPLICABLE DEVICES: Lion; xCat2; Lion2; Aldrin2)
     */
    GT_BOOL appendPacket;

    /** @brief source ID number
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 sourceId;

    /** @brief GT_FALSE: Don't keep the source info from qcn descriptor
     *  GT_TRUE: Keep the source info from the qcn descriptor
     *  (APPLICABLE DEVICES: Aldrin2)
     */
    GT_BOOL keepQcnSrcInfo;

    /** @brief Qcn Q Delta to be sent in generated QCN headers.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_U16 qcnQDelta;

    /** @brief Qcn Q Offset to be sent in generated QCN headers.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_U16 qcnQOffset;

} CPSS_PORT_CNM_GENERATION_CONFIG_STC;

/**
 * @struct CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC
 *
 * @brief QCN Trigger Threshold to define the probability for
 * triggering a QCN frame upon congestion. Total of 64 entries
 * with 2 fields per entry as below.
 */
typedef struct{

    /** @brief Trigger Threshold 0 for QCN trigger.
     *  (0..0x3FFF)
     */
    GT_U32 qcnTriggerTh0;

    /** @brief Trigger Threshold 1 for QCN trigger..
     *  (0..0x3FFF)
     */
    GT_U32 qcnTriggerTh1;

} CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC;


/**
* @enum CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT
 *
 * @brief Enumeration that indicates the locations of the congested queue/priority
 * bits in the CNM header. (Defines the index in table ....)
*/
typedef enum{

    /** @brief congested q/speed bits are in queue offset of the packet. */
    CPSS_DXCH_PORT_CN_CONG_Q_NUM_E,

    /** @brief congested q/speed bits are in network TC of the packet.
     */
    CPSS_DXCH_PORT_CN_QCN_ORIG_PRIO_E

} CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT;


/**
* @internal cpssDxChPortCnModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification (CN) mechanism.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
*       cpssDxChPortCnQueueAwareEnableSet to enable/disable queue for BCN.
*
*/
GT_STATUS cpssDxChPortCnModeEnableSet
(
    IN GT_U8                      devNum,
    IN CPSS_DXCH_PORT_CN_MODE_ENT enable
);

/**
* @internal cpssDxChPortCnModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of Congestion Notification (CN) mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnModeEnableGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_MODE_ENT *enablePtr
);

/**
* @internal cpssDxChPortCnProfileSet function
* @endinternal
*
* @brief   Binds a port to a Congestion Notification profile.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To disable port for triggering CN frames bind port to profile with
*       all thresholds set to maximum.
*       2. On Lion device CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS cpssDxChPortCnProfileSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
);

/**
* @internal cpssDxChPortCnProfileGet function
* @endinternal
*
* @brief   Gets the port's Congestion Notification profile.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] profileIndexPtr          - (pointer to) profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
);

/**
* @internal cpssDxChPortCnProfileQueueThresholdSet function
* @endinternal
*
* @brief   Sets the buffer threshold for triggering CN frame for a given tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] threshold                - buffer limit  for triggering CN frame on a given
*                                      tc queue (0..0x1FFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to CN profile in Lion use
*       cpssDxChPortCnProfileQueueConfigSet.
*
*/
GT_STATUS cpssDxChPortCnProfileQueueThresholdSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN GT_U32   threshold
);

/**
* @internal cpssDxChPortCnProfileQueueThresholdGet function
* @endinternal
*
* @brief   Gets the buffer threshold for triggering CN frame for a given tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] thresholdPtr             - (pointer to) buffer limit threshold for
*                                      triggering CN frame on a given tc queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileQueueThresholdGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *thresholdPtr
);


/**
* @internal cpssDxChPortCnProfileThresholdSet function
* @endinternal
*
* @brief   Sets the buffer threshold for triggering CN frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
* @param[in] threshold                - buffer limit threshold
*                                      for triggering CN frame (0..0x1FFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile index
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileThresholdSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   threshold
);

/**
* @internal cpssDxChPortCnProfileThresholdGet function
* @endinternal
*
* @brief   Gets the buffer threshold for triggering CN frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index (0..7).
*
* @param[out] thresholdPtr             - (pointer to) buffer limit threshold for
*                                      triggering CN frame.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number or profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileThresholdGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profileIndex,
    OUT GT_U32  *thresholdPtr
);

/**
* @internal cpssDxChPortCnQueueAwareEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification awareness on a given
*         tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] enable                   - GT_TRUE: CN aware. Packets enqueued to tcQueue are enabled to
*                                      trigger Congestion Notification frames.
*                                      GT_FALSE: CN blind. Packets enqueued to tcQueue never trigger
*                                      Congestion Notification frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnQueueAwareEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortCnQueueAwareEnableGet function
* @endinternal
*
* @brief   Gets the status of Congestion Notification awareness on a given
*         tc queue.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] enablePtr                - (pointer to) status of Congestion Notification awareness
*                                      GT_TRUE: CN aware. Packets enqueued to tcQueue are enabled to
*                                      trigger Congestion Notification frames.
*                                      GT_FALSE: CN blind. Packets enqueued to tcQueue never trigger
*                                      Congestion Notification frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnQueueAwareEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tcQueue,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortCnFrameQueueSet function
* @endinternal
*
* @brief   Set traffic class queue associated with CN frames generated by device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue associated with CN frames (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN frames must be enqueued to priority queues disabled to trigger CN.
*       Bobcat2; Caelum; Bobcat3; Aldrin - The device use 'traffic class' from qosProfileId set by
*       cpssDxChPortCnMessageGenerationConfigSet()
*
*/
GT_STATUS cpssDxChPortCnFrameQueueSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue
);

/**
* @internal cpssDxChPortCnFrameQueueGet function
* @endinternal
*
* @brief   Get traffic class queue associated with CN frames generated by device.
*         xCat; xCat3; Lion; xCat2; Lion2.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] devNum                   - device number.
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue associated with CN frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnFrameQueueGet
(
    IN  GT_U8    devNum,
    OUT GT_U8    *tcQueuePtr
);

/**
* @internal cpssDxChPortCnEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CN frames.
*         This EtherType also inserted in the CN header of generated CN frames.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType
);

/**
* @internal cpssDxChPortCnEtherTypeGet function
* @endinternal
*
* @brief   Gets the EtherType for CN frames. The EtherType inserted in the CN
*         header.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnEtherTypeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *etherTypePtr
);

/**
* @internal cpssDxChPortCnSpeedIndexSet function
* @endinternal
*
* @brief   Sets port speed index of the egress port. The device inserts this index
*         in the generated CCFC frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] portSpeedIndex           - port speed index (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range port speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Panic Pause uses Port Speed Index 0 and 7 to indicate XON and XOFF.
*       Congested ports should use Port Speed Index 1 to 6 if
*       Panic Pause is used.
*
*/
GT_STATUS cpssDxChPortCnSpeedIndexSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   portSpeedIndex
);

/**
* @internal cpssDxChPortCnSpeedIndexGet function
* @endinternal
*
* @brief   Gets port speed index of the egress port. The device inserts this index
*         in the generated CCFC frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  -      - port number.
*
* @param[out] portSpeedIndexPtr        - (pointer to) port speed index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnSpeedIndexGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32   *portSpeedIndexPtr
);


/**
* @internal cpssDxChPortCnFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable generation of flow control packets as result of CN
*         frame termination or Panic Pause.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE:  enable. Flow control packets can be issued.
*                                      GT_FALSE: disable. Flow control packets can't be issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnFcEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortCnFcEnableGet function
* @endinternal
*
* @brief   Gets the status of generation of flow control packets as result of CN
*         frame termination or Panic Pause on a given port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  enable. Flow control packets is issued.
*                                      GT_FALSE: disable. Flow control packets is not issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnFcEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortCnPauseTriggerEnableSet function
* @endinternal
*
* @brief   Enable the CN triggering engine to trigger a pause frame.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE:  enable.
*                                      GT_FALSE: disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable proper operation of CCFC, this configuration must be
*       enabled for all network ports and disabled for all cascade ports.
*
*/
GT_STATUS cpssDxChPortCnPauseTriggerEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortCnPauseTriggerEnableGet function
* @endinternal
*
* @brief   Gets the status triggering engine a pause frame on a given port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE:  enable.
*                                      GT_FALSE: disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPauseTriggerEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortCnFcTimerSet function
* @endinternal
*
* @brief   Sets timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
*                                      On AC3X remote ports: index (APPLICABLE RANGES: 0..63) (See Comment 4.)
* @param[in] timer                    - 802.3x/PFC pause time (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_OUT_OF_RANGE          - on out of range pause time value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. xCat,xCat3: Panic Pause uses Port Speed Index 0 and 7 to indicate
*       XON and XOFF. Port timer configurations 0 and 7 must be configured
*       to 0x0 and 0xFFFF if Panic Pause is used.
*       2. xCat,xCat3: index is always congested port speed index (CCFC mode).
*       3. Lion, xCat2: index determined by
*       cpssDxChPortCnPrioritySpeedLocationSet.
*       4. AC3X: On remote ports, index is comprised of
*       (MSB) 2 bits of Speed code,
*       2 bits of qFB,
*       1 bit of qDelta sign
*       (LSB) 1 bit of qDelta absolute value > 255.
*
*/
GT_STATUS cpssDxChPortCnFcTimerSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   index,
    IN GT_U32   timer
);

/**
* @internal cpssDxChPortCnFcTimerGet function
* @endinternal
*
* @brief   Gets timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
*                                      On AC3X remote ports: index (APPLICABLE RANGES: 0..63) (See Comment 3.)
*
* @param[out] timerPtr                 - (pointer to) 802.3x/PFC Pause time
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. xCat,xCat3: index is always congested port speed index (CCFC mode).
*       2. Lion, xCat2: index determined by
*       cpssDxChPortCnPrioritySpeedLocationSet.
*       3. AC3X: On remote ports, index is comprised of
*       (MSB) 2 bits of Speed code,
*       2 bits of qFB,
*       1 bit of qDelta sign
*       (LSB) 1 bit of qDelta absolute value > 255.
*
*/
GT_STATUS cpssDxChPortCnFcTimerGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32   index,
    OUT GT_U32   *timerPtr
);


/**
* @internal cpssDxChPortCnPanicPauseThresholdsSet function
* @endinternal
*
* @brief   Sets the thresholds for triggering a Panic Pause: Global XOFF/XON frame
*         over all ports enabled to send a Panic Pause.
*         A global XOFF frame is triggered if the number of buffers occupied
*         by the CN aware frames crosses up xoffLimit. A global XON frame is
*         triggered if the number of buffers occupied by the CN aware frames
*         crosses down xonLimit.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable Panic Pause.
*                                      GT_FALSE: Disable Panic Pause.
* @param[in] xoffLimit                - X-OFF limit (0..1FFF).Relevant only if enable==GT_TRUE.
* @param[in] xonLimit                 - X-ON limit (0..1FFF).Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range xoffLimit or
*                                       xonLimit value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Panic Pause uses Port Speed Index 0 and 7 to indicate XON and XOFF.
*       Port timer configurations 0 and 7 must be configured
*       to 0x0 and 0xFFFF, see cpssDxChPortCnFcTimerSet.
*       Congested ports should use Port Speed Index 1 to 6,
*       see cpssDxChPortCnSpeedIndexSet.
*       2. Panic Pause cannot operate together with the IEEE 802.3x triggers.
*
*/
GT_STATUS cpssDxChPortCnPanicPauseThresholdsSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   xoffLimit,
    IN GT_U32   xonLimit
);

/**
* @internal cpssDxChPortCnPanicPauseThresholdsGet function
* @endinternal
*
* @brief   Gets the thresholds for triggering a Panic Pause: Global XOFF/XON frame
*         over all ports enabled to send a Panic Pause.
*         A global XOFF frame is triggered if the number of buffers occupied
*         by the CN aware frames crosses up xoffLimit. A global XON frame is
*         triggered if the number of buffers occupied by the CN aware frames
*         crosses down xonLimit.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to):
*                                      GT_TRUE: Enable Panic Pause.
*                                      GT_FALSE: Disable Panic Pause.
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value.
*                                      Relevant only if enablePtr==GT_TRUE.
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value.
*                                      Relevant only if enablePtr==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPanicPauseThresholdsGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *xoffLimitPtr,
    OUT GT_U32   *xonLimitPtr
);

/**
* @internal cpssDxChPortCnTerminationEnableSet function
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
GT_STATUS cpssDxChPortCnTerminationEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortCnTerminationEnableGet function
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
GT_STATUS cpssDxChPortCnTerminationEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortCnProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on wrong values of alpha
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
GT_STATUS cpssDxChPortCnProfileQueueConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
);

/**
* @internal cpssDxChPortCnProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
* @retval GT_BAD_STATE             - on wrong values of alpha
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfileQueueConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U8                               tcQueue,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
);

/**
* @internal cpssDxChPortCnQueueStatusModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable queue status mode. When enabled, CNM is sent to a
*         configurable target that examines the queue status.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnQueueStatusModeEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable,
    IN GT_HW_DEV_NUM    targetHwDev,
    IN GT_PORT_NUM      targetPort
);

/**
* @internal cpssDxChPortCnQueueStatusModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of queue status mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnQueueStatusModeEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr,
    OUT GT_HW_DEV_NUM   *targetHwDevPtr,
    OUT GT_PORT_NUM     *targetPortPtr
);

/**
* @internal cpssDxChPortCnSampleEntrySet function
* @endinternal
*
* @brief   Sets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnSampleEntrySet
(
    IN GT_U8    devNum,
    IN GT_U8    entryIndex,
    IN CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
);

/**
* @internal cpssDxChPortCnSampleEntryGet function
* @endinternal
*
* @brief   Gets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnSampleEntryGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    entryIndex,
    OUT CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
);

/**
* @internal cpssDxChPortCnFbCalcConfigSet function
* @endinternal
*
* @brief   Sets Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnFbCalcConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
);

/**
* @internal cpssDxChPortCnFbCalcConfigGet function
* @endinternal
*
* @brief   Gets Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnFbCalcConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
);

/**
* @internal cpssDxChPortCnPacketLengthSet function
* @endinternal
*
* @brief   Sets packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] packetLength             - packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or packet length.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configuring a value other than CPSS_DXCH_PORT_CN_LENGTH_ORIG_PACKET_E
*       should be used when the packet length is unknown, e.g. Cut-through mode.
*
*/
GT_STATUS cpssDxChPortCnPacketLengthSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT packetLength
);

/**
* @internal cpssDxChPortCnPacketLengthGet function
* @endinternal
*
* @brief   Gets packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] packetLengthPtr          - (pointer to) packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPacketLengthGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT  *packetLengthPtr
);

/**
* @internal cpssDxChPortCnMessageGenerationConfigSet function
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
GT_STATUS cpssDxChPortCnMessageGenerationConfigSet
(
    IN GT_U8    devNum,
    IN CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
);

/**
* @internal cpssDxChPortCnMessageGenerationConfigGet function
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
GT_STATUS cpssDxChPortCnMessageGenerationConfigGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
);



/**
* @internal cpssDxChPortCnPrioritySpeedLocationSet function
* @endinternal
*
* @brief   Sets location of the priority/speed bits in the CNM header for parsing
*         and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] prioritySpeedLocation    - priority speed location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPrioritySpeedLocationSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT prioritySpeedLocation
);

/**
* @internal cpssDxChPortCnPrioritySpeedLocationGet function
* @endinternal
*
* @brief   Gets location of the index (priority/speed) bits in the CNM header
*         for parsing and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @param[out] prioritySpeedLocationPtr - (pointer to) priority speed location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPrioritySpeedLocationGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT *prioritySpeedLocationPtr
);

/**
* @internal cpssDxChPortCnMessageTypeSet function
* @endinternal
*
* @brief   Sets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] mType                    - CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnMessageTypeSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT   mType
);


/**
* @internal cpssDxChPortCnMessageTypeGet function
* @endinternal
*
* @brief   Gets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
*
* @param[out] mTypePtr                 - (pointer to) CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnMessageTypeGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT   *mTypePtr
);


/**
* @internal cpssDxChPortCnMessageTriggeringStateSet function
* @endinternal
*
* @brief   Sets CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
*                                      Aldrin2: (APPLICABLE RANGES: 0..0xFFFFF)
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
GT_STATUS cpssDxChPortCnMessageTriggeringStateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    IN  GT_U32                  qcnSampleInterval,
    IN  GT_U32                  qlenOld
);


/**
* @internal cpssDxChPortCnMessageTriggeringStateGet function
* @endinternal
*
* @brief   Gets CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortCnMessageTriggeringStateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_U32                  *qcnSampleIntervalPtr,
    OUT GT_U32                  *qlenOldPtr
);

/**
* @internal cpssDxChPortCnPauseTimerMapTableEntrySet function
* @endinternal
*
* @brief   Sets Pause Timer Map Table Entry.
*         Table index is QCN-qFb or CCFC Speed bits.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
*
* @param[in] devNum                   - device number.
* @param[in] index                    - table entry index.
*                                      (APPLICABLE RANGES: 0..63)
* @param[in] pauseTimer               - pause timer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on wrong pauseTimer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPauseTimerMapTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U32                  pauseTimer
);

/**
* @internal cpssDxChPortCnPauseTimerMapTableEntryGet function
* @endinternal
*
* @brief   Gets Pause Timer Map Table Entry.
*         Table index is QCN-qFb or CCFC Speed bits.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
*
* @param[in] devNum                   - device number.
* @param[in] index                    - table entry index.
*                                      (APPLICABLE RANGES: 0..63)
*
* @param[out] pauseTimerPtr            - (pointer to)pause timer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_BAD_PTR               - on NULL Pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnPauseTimerMapTableEntryGet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  index,
    OUT  GT_U32                  *pauseTimerPtr
);

/**
 * @internal cpssDxChPortCnDbaAvailableBuffSet function
 * @endinternal
 *
 * @brief   Set amount of buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
 * @param[in] devNum                   - device number
 * @param[in] availableBuff            - amount of buffers available for dynamic allocation for CN.
 *                                      (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaAvailableBuffSet
(
 IN  GT_U8           devNum,
 IN  GT_U32          availableBuff
);

/**
 * @internal cpssDxChPortCnDbaAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] availableBuffPtr        - pointer to amount of buffers available for dynamic allocation for CN.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaAvailableBuffGet
(
 IN  GT_U8           devNum,
 OUT GT_U32          *availableBuffPtr
);

/**
 * @internal cpssDxChPortCnDbaModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable Dynamic Buffers Allocation (DBA) for QCN.
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
GT_STATUS cpssDxChPortCnDbaModeEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
 );

/**
 * @internal cpssDxChPortCnDbaModeEnableGet function
 * @endinternal
 *
 * @brief   Get state of Dynamic Buffers Allocation (DBA) for QCN.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] enablePtr               - pointer to DBA mode state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaModeEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
 );

/**
 * @internal cpssDxChPortCnDbaPoolAvailableBuffSet function
 * @endinternal
 *
 * @brief   Set amount of pool buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] poolNum                  - pool number for QCN DBA buffers
 *                                      (APPLICABLE RANGES: 0..1)
 * @param[in] poolAvailableBuff        - amount of pool buffers available for dynamic allocation for CN.
 *                                      (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaPoolAvailableBuffSet
(
 IN  GT_U8           devNum,
 IN  GT_U32          poolNum,
 IN  GT_U32          poolAvailableBuff
);

/**
 * @internal cpssDxChPortCnDbaPoolAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of pool buffers available for dynamic allocation for CN.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] poolNum                  - pool number for QCN DBA buffers
 *                                      (APPLICABLE RANGES: 0..1)
 * @param[out] poolAvailableBuffPtr    - pointer to amount of pool buffers available for dynamic allocation for CN.
 *                                      (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnDbaPoolAvailableBuffGet
(
 IN  GT_U8           devNum,
 IN  GT_U32          poolNum,
 OUT GT_U32          *poolAvailableBuffPtr
);

/**
 * @internal cpssDxChPortCnQcnTriggerSet function
 * @endinternal
 *
 * @brief  Set the QCN trigger thresholds.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] index                    - index to trigger threshold table
 *                                      (APPLICABLE RANGES: 0..31)
 * @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnQcnTriggerSet
(
 IN  GT_U8                                            devNum,
 IN  GT_U32                                           index,
 IN  CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC             *qcnTriggerPtr
 );

/**
 * @internal cpssDxChPortCnQcnTriggerGet function
 * @endinternal
 *
 * @brief   Get the configured QCN trigger thresholds.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                - device number
 * @param[in] index                 - index to trigger threshold table
 *                                   (APPLICABLE RANGES: 0..31)
 * @param[in] qcnTriggerPtr         - (pointer to) qcn trigger threshold
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnQcnTriggerGet
(
 IN   GT_U8                                           devNum,
 IN   GT_U32                                          index,
 OUT  CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC            *qcnTriggerPtr
 );

/**
 * @internal cpssDxChPortCnCNMEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable CNM generation for CN untagged frames.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  CNM generation enabled
 *                                      GT_FALSE - CNM generation disabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnCNMEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
 );

/**
 * @internal cpssDxChPortCnCNMEnableGet function
 * @endinternal
 *
 * @brief   Get state of CNM enable for CN untagged frames.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[out] enablePtr               - pointer to CNM enable state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortCnCNMEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
 );

/**
* @internal cpssDxChPortCnProfilePortConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given port profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on wrong values of alpha
* @retval GT_BAD_PARAM             - on wrong device number, profile set
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS cpssDxChPortCnProfilePortConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
);

/**
* @internal cpssDxChPortCnProfilePortConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given port profile.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[out] cnProfileCfgPtr         - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong values of alpha
* @retval GT_BAD_PARAM             - on wrong device number, profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnProfilePortConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
);

/**
* @internal cpssDxChPortCnCongestedQPriorityLocationSet function
* @endinternal
*
* @brief   Sets location of the congested q/priority bits in the CNM header for parsing
*         and generation of CNM.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                        - device number.
* @param[in] congestedQPriorityLocation    - congested q/priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCnCongestedQPriorityLocationSet
(
    IN GT_U8                                               devNum,
    IN CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT congestedQPriorityLocation
);

/**
* @internal cpssDxChPortCnCongestedQPriorityLocationGet function
* @endinternal
*
* @brief   Gets location of the index (congested q/priority) bits in the CNM header
*         for parsing and generation of CNM.
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
GT_STATUS cpssDxChPortCnCongestedQPriorityLocationGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT *congestedQPriorityLocationPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortCnh */

