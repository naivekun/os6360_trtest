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
* @file cpssDxChPortPfc.h
*
* @brief CPSS implementation for Priority Flow Control functionality.
*
* @version   23
********************************************************************************
*/

#ifndef __cpssDxChPortPfch
#define __cpssDxChPortPfch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>


/**
* @enum CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT
 *
 * @brief Enumeration of PFC Counting modes.
*/
typedef enum{

    /** Counting buffers. PFC thresholds are set in buffers. */
    CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E,

    /** Counting packets. PFC thresholds are set in packets. */
    CPSS_DXCH_PORT_PFC_COUNT_PACKETS_E

} CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT;

/**
* @enum CPSS_DXCH_PORT_PFC_ENABLE_ENT
 *
 * @brief Enumeration of PFC enable options.
*/
typedef enum{

    /** PFC triggering only enabled. */
    CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_ONLY_E,

    /** Both PFC triggering and response are enabled. */
    CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E

} CPSS_DXCH_PORT_PFC_ENABLE_ENT;

/**
* @* @enum CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT
 *
 * @brief  PFC RX type.8 TC or 16 TC(Applicable device :Falcon only)
 *
*/
typedef enum
{
    CPSS_DXCH_PORT_PFC_RESPONCE_MODE_8_TC_E,
    CPSS_DXCH_PORT_PFC_RESPONCE_MODE_16_TC_E
} CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT;

/**
* @enum CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT
 *
 * @brief Configuration type of occupied resources origin used for flow control
*/
typedef enum{
     /** @brief onfiguration type of occupied resources origin is packet buffer*/
    CPSS_PORT_TX_PFC_RESOURCE_MODE_GLOBAL_E,

    /** @brief onfiguration type of occupied resources origin is shared pool*/
    CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_E,

    /** @brief onfiguration type of occupied resources origin is shared pool,
                    in addition headroom occupancy  is subtrcted from the count*/
    CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
} CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT;

/**
* @enum CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT
 *
 * @brief Configuration type of avalable buffers amount used for flow control
*/
typedef enum{

    /** @brief  Configuration of avalable buffers amount  for global TC*/
    CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E,

    /** @brief  Configuration of avalable buffers amount  for Port/TC*/
    CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_PORT_TC_E,

     /** @brief  Configuration of avalable buffers amount  both for Global TC and Port/TC */
    CPSS_PORT_TX_PFC_AVAILEBLE_BUFF_CONF_MODE_GLOBAL_AND_PORT_TC_E
} CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT;


/**
* @enum CPSS_DXCH_PORT_PFC_XON_MODE_ENT
 *
 * @brief Configuration of hysteresys mode
*/
typedef enum{

    /**Offset; Offset; Xon Threshold = PFC_threshold - Offset_value
            PFC_threshold = guaranteed_buffers +alpha(available_buffers - consumed_buffers)  */
    CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E,

    /** Static Threshold.Xon Threshold = Offset_value  */
    CPSS_DXCH_PORT_PFC_XON_MODE_FIXED_E

} CPSS_DXCH_PORT_PFC_XON_MODE_ENT;


/**
* @struct CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC
 *
 * @brief PFC Profile configurations.
*/
typedef struct{

    /** @brief Xon threshold. */
    GT_U32 xonThreshold;

    /** @brief Xoff threshold.
     */
    GT_U32 xoffThreshold;

    /** @brief defines which part of the free buffers are available to the port or queue for XON
     *  (APPLICABLE DEVICES: Aldrin2.)
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT xonAlpha;

    /** @brief defines which part of the free buffers are available to the port or queue for XOFF.
     *  The XON/XOFF limits are calculated as follows:
     *  Limit = Guaranteed Buffers + Dynamic buffers
     *  The Dynamic Factor is calculated based on the amount of free buffers.
     *  Free Buffers = Total Available Buffers - Global Used Buffers
     *  Dynamic buffer for the queue is defined by following:
     *  queueNBuffs =xxxAlpha (Free Buffers/(sum of all alphas +1)))
     *  (APPLICABLE DEVICES: Aldrin2.)
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT xoffAlpha;

} CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC;

/**
* @struct CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC
 *
 * @brief Lossy drop configurations.
*/
typedef struct{

    /** @brief GT_TRUE: Enable drop of TO_CPU packets according to the
     *  lossy drop feature.
     *  GT_FALSE: Do not drop TO_CPU packets due to to lossy drop
     *  feature (even if its TC marked as drop).
     */
    GT_BOOL toCpuLossyDropEnable;

    /** @brief GT_TRUE: Enable drop of TO_TARGET_SNIFFER packets according to the
     *  lossy drop feature.
     *  GT_FALSE: Do not drop TO_TARGET_SNIFFER packets due to to lossy drop
     *  feature (even if its TC marked as drop).
     *  lossy drop feature.
     */
    GT_BOOL toTargetSnifferLossyDropEnable;

    /** @brief GT_TRUE: Enable drop of FROM_CPU packets according to the
     *  lossy drop feature.
     *  GT_FALSE: Do not drop FROM_CPU packets due to to lossy drop
     *  feature (even if its TC marked as drop).
     *  Comments:
     *  None.
     */
    GT_BOOL fromCpuLossyDropEnable;

} CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC;



/**
* @struct CPSS_DXCH_PFC_THRESHOLD_STC
 *
 * @brief  PFC threshold configuration
 * When enable, once relevant  counter is above the calculated threshold, PFC OFF message is sent .
*/

typedef struct
{

     /** @brief
        Alpha configuration used to Calculate the congestion factor.
        congestion_facor = (Alpha == 0) ? 0, else 2^(-6+Alpha).
     */
    GT_U32   alfa;

     /** @brief  Guaranteed Threshold configuration for PFC ON/OFF:
            Threshold = Guaranteed Threshold + congestion_factor X (Available buffers - PB congestion).
     */
    GT_U32  guaranteedThreshold;

} CPSS_DXCH_PFC_THRESHOLD_STC;

/**
* @struct CPSS_DXCH_PFC_HYSTERESIS_CONF_STC
 *
 * @brief  PFC hysteresis  configuration
 *
 */
typedef struct
{
     /** @brief
        Alpha configuration used to Calculate the congestion factor.
        congestion_facor = (Alpha == 0) ? 0, else 2^(-6+Alpha).
     */
    CPSS_DXCH_PORT_PFC_XON_MODE_ENT   xonMode;
     /** @brief  Xon Offset Value
     */
    GT_U32                             xonOffsetValue;

} CPSS_DXCH_PFC_HYSTERESIS_CONF_STC;


/**
* @internal cpssDxChPortPfcEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) response functionality.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcEnable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If PFC response is enabled, the shaper’s baseline must be
*       at least 0x3FFFC0, see:
*       cpssDxChPortTxShaperBaselineSet.
*       Note: Triggering cannot be disabled by this API.
*
*/
GT_STATUS cpssDxChPortPfcEnableSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_PFC_ENABLE_ENT pfcEnable
);

/**
* @internal cpssDxChPortPfcEnableGet function
* @endinternal
*
* @brief   Get the status of PFC response functionality.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcEnablePtr             - (pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering is enabled by default.
*
*/
GT_STATUS cpssDxChPortPfcEnableGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_PFC_ENABLE_ENT *pfcEnablePtr
);

/**
* @internal cpssDxChPortPfcProfileIndexSet function
* @endinternal
*
* @brief   Binds a source port to a PFC profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcProfileIndexSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
);

/**
* @internal cpssDxChPortPfcProfileIndexGet function
* @endinternal
*
* @brief   Gets the port's PFC profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
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
GT_STATUS cpssDxChPortPfcProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
);

/**
* @internal cpssDxChPortPfcProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcProfileCfgPtr         - pointer to PFC Profile configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcProfileQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
);

/**
* @internal cpssDxChPortPfcProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] pfcProfileCfgPtr         - pointer to PFC Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcProfileQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U8    tcQueue,
    OUT CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
);


/**
* @internal cpssDxChPortPfcCountingModeSet function
* @endinternal
*
* @brief   Sets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcCountMode             - PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcCountingModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  pfcCountMode
);

/**
* @internal cpssDxChPortPfcCountingModeGet function
* @endinternal
*
* @brief   Gets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcCountModePtr          - (pointer to) PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcCountingModeGet
(
    IN  GT_U8    devNum,
    OUT CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT  *pfcCountModePtr
);


/**
* @internal cpssDxChPortPfcGlobalDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC global drop.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable PFC global drop.
*                                      GT_FALSE: Disable PFC global drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To configure drop threshold use cpssDxChPortPfcGlobalQueueConfigSet.
*
*/
GT_STATUS cpssDxChPortPfcGlobalDropEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortPfcGlobalDropEnableGet function
* @endinternal
*
* @brief   Gets the current status of PFC global drop.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of PFC functionality
*                                      GT_TRUE:  PFC global drop enabled.
*                                      GT_FALSE: PFC global drop disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcGlobalDropEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortPfcGlobalQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] xoffThreshold            - XOFF threshold
*                                      (APPLICABLE RANGES: Lion, Lion2 0..0x7FF)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
* @param[in] dropThreshold            - Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped
*                                      (APPLICABLE RANGES: Lion, Lion2 0..0x7FF)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
* @param[in] xonThreshold             - XON threshold
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x1FFFFFFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. dropThreshold is used when PFC global drop is enabled.
*       See cpssDxChPortPfcGlobalDropEnableSet.
*       2. All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcGlobalQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue,
    IN GT_U32   xoffThreshold,
    IN GT_U32   dropThreshold,
    IN GT_U32   xonThreshold
);

/**
* @internal cpssDxChPortPfcGlobalQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] xoffThresholdPtr         - (pointer to) Xoff threshold.
* @param[out] dropThresholdPtr         - (pointer to) Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped.
* @param[out] xonThresholdPtr          - (pointer to) Xon threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcGlobalQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *dropThresholdPtr,
    OUT GT_U32   *xonThresholdPtr
);

/**
* @internal cpssDxChPortPfcTimerMapEnableSet function
* @endinternal
*
* @brief   Enables mapping of PFC timer to priority queue for given scheduler profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] enable                   - Determines whether PFC timer to Priority Queue map
*                                      is used.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssDxChPortTxBindPortToSchedulerProfileSet.
*
*/
GT_STATUS cpssDxChPortPfcTimerMapEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_BOOL                                 enable

);

/**
* @internal cpssDxChPortPfcTimerMapEnableGet function
* @endinternal
*
* @brief   Get the status of PFS timer to priority queue mapping for given
*         scheduler profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile Set
*
* @param[out] enablePtr                - (pointer to) status of PFC timer to Priority Queue
*                                      mapping.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcTimerMapEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT GT_BOOL                                *enablePtr

);

/**
* @internal cpssDxChPortPfcTimerToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7)
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_OUT_OF_RANGE          - on out of traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcTimerToQueueMapSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    IN  GT_U32    tcQueue

);

/**
* @internal cpssDxChPortPfcTimerToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7).
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcTimerToQueueMapGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    OUT GT_U32    *tcQueuePtr

);

/**
* @internal cpssDxChPortPfcShaperToPortRateRatioSet function
* @endinternal
*
* @brief   Sets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] shaperToPortRateRatio    - shaper rate to port speed ratio
*                                      in percentage (0..100).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range shaper rate to port speed ratio
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssDxChPortTxBindPortToSchedulerProfileSet.
*
*/
GT_STATUS cpssDxChPortPfcShaperToPortRateRatioSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                   tcQueue,
    IN  GT_U32                                  shaperToPortRateRatio

);

/**
* @internal cpssDxChPortPfcShaperToPortRateRatioGet function
* @endinternal
*
* @brief   Gets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] shaperToPortRateRatioPtr - (pointer to)shaper rate to port speed ratio
*                                      in percentage.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPortPfcShaperToPortRateRatioGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                   tcQueue,
    OUT GT_U32                                 *shaperToPortRateRatioPtr

);

/**
* @internal cpssDxChPortPfcForwardEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward PFC frames to the ingress pipe,
*                                      GT_FALSE: do not forward PFC frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
GT_STATUS cpssDxChPortPfcForwardEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL enable
);


/**
* @internal cpssDxChPortPfcForwardEnableGet function
* @endinternal
*
* @brief   Get status of PFC frames forwarding
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - current forward PFC frames status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
GT_STATUS cpssDxChPortPfcForwardEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortPfcLossyDropQueueEnableSet function
* @endinternal
*
* @brief   Enable/Disable lossy drop for packets with given traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] enable                   - GT_TRUE: Lossy - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are dropped.
*                                      - GT_FALSE: Lossless - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are not dropped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
GT_STATUS cpssDxChPortPfcLossyDropQueueEnableSet
(
    IN GT_U8     devNum,
    IN GT_U8     tcQueue,
    IN GT_BOOL   enable
);

/**
* @internal cpssDxChPortPfcLossyDropQueueEnableGet function
* @endinternal
*
* @brief   Get the status of lossy drop on given traffic class queue.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] enablePtr                (pointer to)
*                                      - GT_TRUE: Lossy - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are dropped.
*                                      - GT_FALSE: Lossless - when lossy drop threshold is
*                                      reached, packets assigned with given tcQueue are not dropped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChPortPfcLossyDropQueueEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortPfcLossyDropConfigSet function
* @endinternal
*
* @brief   Set lossy drop configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] lossyDropConfigPtr       - (pointer to) lossy drop configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
GT_STATUS cpssDxChPortPfcLossyDropConfigSet
(
    IN GT_U8     devNum,
    IN CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC *lossyDropConfigPtr
);

/**
* @internal cpssDxChPortPfcLossyDropConfigGet function
* @endinternal
*
* @brief   Get lossy drop configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] lossyDropConfigPtr       - (pointer to) lossy drop configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The device provides a burst absortion system designed to sustain contain
*       the packets in case of an incast type traffic (many-to-one).
*       Two thresholds are used:
*       1. Lossy drop - packets packets with lossy TC are dropped in
*       congested port group.
*       2. PFC Xoff threshold - PFC is sent to all of the ports in the port group
*       with all timers set to 0xFFFF.
*
*/
GT_STATUS cpssDxChPortPfcLossyDropConfigGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC *lossyDropConfigPtr
);

/**
* @internal cpssDxChPortPfcTerminateFramesEnableSet function
* @endinternal
*
* @brief   Enable/Disable the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   -  termination of flow control frames
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
GT_STATUS cpssDxChPortPfcTerminateFramesEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortPfcTerminateFramesEnableGet function
* @endinternal
*
* @brief   Get whether the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x) is enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) termination of flow control frames enabling
*                                      status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
GT_STATUS cpssDxChPortPfcTerminateFramesEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortPfcQueueCounterGet function
* @endinternal
*
* @brief   Get the number buffers/packets per proirity queue.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] cntPtr                   - (pointer to) number buffers/packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, tcQueue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Buffer or packet counting mode is set by cpssDxChPortPfcCountingModeSet.
*
*/
GT_STATUS cpssDxChPortPfcQueueCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    OUT GT_U32  *cntPtr
);


/**
* @internal cpssDxChPortPfcSourcePortToPfcCounterSet function
* @endinternal
*
* @brief   Set PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcSourcePortToPfcCounterSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  pfcCounterNum
);


/**
* @internal cpssDxChPortPfcSourcePortToPfcCounterGet function
* @endinternal
*
* @brief   Get PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] pfcCounterNumPtr         (pointer to) PFC counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPortPfcSourcePortToPfcCounterGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *pfcCounterNumPtr
);


/**
* @internal cpssDxChPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per PFC counter and traffic class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @param[out] pfcCounterValuePtr       - (pointer to) PFC counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
*@note    In SIP6 pfcCounterNum is  used as physical port number.
*@note    In case tcQueue equal 0xFF the counter value refer to all TC's consumed by port.
*/
GT_STATUS cpssDxChPortPfcCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr
);

/**
* @internal cpssDxChPortPfcXonMessageFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable  PFC XON Message Filter.
*                                      GT_FALSE: Disable PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Enable this feature in a system with periodic flow control XON messages.
*
*/
GT_STATUS cpssDxChPortPfcXonMessageFilterEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortPfcXonMessageFilterEnableGet function
* @endinternal
*
* @brief   Gets current status of the filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Enable  PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcXonMessageFilterEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortPfcDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for PFC.
*
* @note   APPLICABLE DEVICES:      Aldrin2,Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[in] availableBuff            - amount of buffers available for dynamic allocation for PFC.
*                                      (APPLICABLE RANGES: 0..0xFFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPfcDbaAvailableBuffSet
(
    IN  GT_U8            devNum,
    IN  GT_DATA_PATH_BMP dataPathBmp,
    IN  GT_U32           availableBuff
);

/**
 * @internal cpssDxChPortPfcDbaAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of buffers available for dynamic allocation for PFC.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2,Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *

 * @param[in] devNum                   - device number
 * @param[in] dataPathBmp              - bitmap of Data Paths
 *                                      (APPLICABLE DEVICES Falcon)
 *                                      NOTEs:
 *                                      1. for non multi data paths device this parameter is
 *                                      IGNORED.
 *                                      2. for multi data paths device:
 *                                      bitmap must be set with at least one bit representing
 *                                      valid data path(s). If a bit of non valid data path
 *                                      is set then function returns GT_BAD_PARAM.
 *                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
                                        3. read only from first data path of the bitmap.
 * @param[out] availableBuffPtr            - (pointer to)amount of buffers available for dynamic allocation for PFC.
 *                                      (APPLICABLE RANGES: 0..0xFFFFFF)

 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortPfcDbaAvailableBuffGet
(
 IN  GT_U8           devNum,
 IN  GT_DATA_PATH_BMP dataPathBmp,
 OUT GT_U32          *availableBuffPtr
);

/**
 * @internal cpssDxChPortPfcDbaModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable XON/XOFF Thresholds Dynamic Buffers Allocation (DBA) for PFC.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  DBA mode disabled
 *                                      GT_FALSE - DBA mode enabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssDxChPortPfcDbaModeEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
 );

/**
 * @internal cpssDxChPortPfcDbaModeEnableGet function
 * @endinternal
 *
 * @brief   Get state of Dynamic Buffers Allocation (DBA) for PFC.
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
GT_STATUS cpssDxChPortPfcDbaModeEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
 );

/**
* @internal  cpssDxChPortPfcGlobalPbLimitSet  function
* @endinternal
*
* @brief   Global limit on the PB occupied buffers, once this threshold is crossed, PFC OFF will be sent to all port.tc.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable sending XOFF once PB limit is crossed
* @param[in] globalPbLimit         -   packet buffer limit(Valid if enable equal GT_TRUE,must be greater then 512)
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssDxChPortPfcGlobalPbLimitSet
(
     IN  GT_U8     devNum,
     IN GT_BOOL    enable,
     IN  GT_U32    globalPbLimit
);

/**
* @internal  cpssDxChPortPfcGlobalPbLimitGet  function
* @endinternal
*
* @brief   Get global limit on the PB occupied buffers.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] enablePtr                   - (pointer to)enable sending XOFF once PB limit is crossed
* @param[out] globalPbLimitPtr         -   (pointer to)packet buffer limit
* @retval GT_OK                              - on success.
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGlobalPbLimitGet
(
     IN  GT_U8      devNum,
     OUT GT_BOOL    *enablePtr,
     OUT  GT_U32    *globalPbLimitPtr
);

/**
* @internal cpssDxChPortPfcGenerationEnableSet function
* @endinternal
*
* @brief   Global Enable/Disable PFC (Priority Flow Control) Generation  functionality.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] enable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGenerationEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortPfcGenerationEnableGet function
* @endinternal
*
* @brief   Get enable/disable PFC (Priority Flow Control) Generation  functionality.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] enablePtr                -(pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGenerationEnableGet
(
    IN GT_U8             devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal internal_cpssDxChPortPfcGlobalTcThresholdSet function
* @endinternal
*
* @brief   Set global TC enable and threshold
*  When enable, once Global TC counter is above the calculated threshold,
*  PFC OFF message is sent to all ports, for the specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - device number.
* @param[in] tc                        - Traffic class[0..7]
* @param[in] enable               - Global TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGlobalTcThresholdSet
(
     IN GT_U8                              devNum,
     IN GT_U32                             tc,
     IN GT_BOOL                             enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC         *thresholdCfgPtr,
     IN CPSS_DXCH_PFC_HYSTERESIS_CONF_STC   *hysteresisCfgPtr
);

/**
* @internal cpssDxChPortPfcGlobalTcThresholdGet function
* @endinternal
*
* @brief   Get global TC enable and threshold .
*  When enable, once Global TC counter is above the calculated threshold,
*  PFC OFF message is sent to all ports, for the specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - device number.
* @param[in] tc                        - Traffic class[0..7]
* @param[out] enable                - (pointer to) Global TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPortPfcGlobalTcThresholdGet
(
     IN  GT_U8                         devNum,
     IN  GT_U32                        tc,
     OUT GT_BOOL                       *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC   *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal cpssDxChPortPfcPortThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[in] enable                -          Global port PFC enable option.
* @param[in] thresholdCfgPtr -        (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcPortThresholdSet
(
     IN GT_U8                              devNum,
     IN GT_PHYSICAL_PORT_NUM               portNum,
     IN GT_BOOL                            enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     IN CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal cpssDxChPortPfcPortThresholdGet function
* @endinternal
*
* @brief   Gets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[out] enablePtr                -    (pointer to) Global port PFC enable option.
* @param[out] thresholdCfgPtr -        (pointer to) threshold struct
* @param[out] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcPortThresholdGet
(
     IN GT_U8                              devNum,
     IN GT_PHYSICAL_PORT_NUM               portNum,
     IN GT_BOOL                            *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal cpssDxChPortPfcPortTcThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port/tc limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port/TC  is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enable                       port/tc PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcPortTcThresholdSet
(
     IN  GT_U8                              devNum,
     IN  GT_PHYSICAL_PORT_NUM               portNum,
     IN  GT_U8                              tc,
     IN  GT_BOOL                            enable,
     IN  CPSS_DXCH_PFC_THRESHOLD_STC        *thresholdCfgPtr
);
/**
* @internal cpssDxChPortPfcPortTcThresholdGet function
* @endinternal
*
* @brief   Gets  PFC OFF port/tc limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port/TC  is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enablePtr                       (pointer to) port/tc PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcPortTcThresholdGet
(
     IN GT_U8                          devNum,
     IN GT_PHYSICAL_PORT_NUM           portNum,
     IN  GT_U8                         tc,
     IN GT_BOOL                        *enablePtr,
     IN CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr
);

/**
* @internal cpssDxChPortPfcResponceEnableSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enable                        port/tc PFC responce enable option.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcResponceEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tc,
    IN GT_BOOL                 enable
);

/**
* @internal cpssDxChPortPfcResponceEnableGet function
* @endinternal
*
* @brief   Get enable /Disable  PFC response status per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enablePtr                   (pointer to)port/tc PFC responce enable option.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcResponceEnableGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tc,
    IN GT_BOOL                 *enablePtr
);

/**
* @internal  cpssDxChPortPfcTcToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tcForPfcResponce                       -      Traffic class[0..15]
* @param[in] queue                                              queue number [0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcTcToQueueMapSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  tcForPfcResponce,
    IN GT_U32                  queue
);

/**
* @internal  cpssDxChPortPfcTcToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] portNum                                         physical port number
* @param[in] queue                                              queue number [0..15]
* @param[out] tcForPfcResponce                      (pointer to)Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcTcToQueueMapGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queue,
    IN GT_U32                  *tcForPfcResponcePtr
);

/**
* @internal  cpssDxChPortPfcReponceModeSet function
* @endinternal
*
* @brief   Set PFC responce mode (8TC per port or 16 TC per port)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] mode                                               responce mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcReponceModeSet
(
    IN  GT_U8                                devNum,
    IN  GT_DATA_PATH_BMP                     dataPathBmp,
    IN  CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT mode
);

/**
* @internal  cpssDxChPortPfcReponceModeGet function
* @endinternal
*
* @brief   Get PFC responce mode (8TC per port or 16 TC per port)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Falcon)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] modePtr                                              (pointer to ) responce mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcReponceModeGet
(
    IN  GT_U8                                devNum,
    IN  GT_DATA_PATH_BMP                     dataPathBmp,
    IN  CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT *modePtr
);

/**
* @internal  cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet
* @endinternal
*
* @brief   If enabled, an IEEE 802.3x PAUSE frame is treated as PFC with all eight timers valid
*              and the timer value equal to the Timer value in the 802.3x Pause frame.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] portNum                                         physical port number
* @param[in] enable                                              If equal GT_TRUE ,treat IEEE 802.3x PAUSE frame  as PFC
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_BOOL                  enable
);

/**
* @internal  cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet
* @endinternal
*
* @brief   Get Link Pause Compatibility Mode.If enabled, an IEEE 802.3x PAUSE frame is treated as PFC with all eight timers valid
*              and the timer value equal to the Timer value in the 802.3x Pause frame.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                                                 physical device number
* @param[in] portNum                                                physical port number
* @param[in] enablePtr                                              (Pointer to)Link Pause Compatibility Mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChPortPfcDbaTcAvailableBuffersSet
* @endinternal
*
* @brief   Set amount of   buffers available for dynamic allocation for PFC for specific TC.
*  This API can also aupdate value for Port/TC availeble buffers.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[in] availableBuffers - amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS  cpssDxChPortPfcDbaTcAvailableBuffersSet
(
   IN  GT_U8   devNum,
   IN  GT_DATA_PATH_BMP dataPathBmp,
   IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
   IN  GT_U8   tc,
   IN  GT_U32  availableBuffers
);

/**
* @internal cpssDxChPortPfcDbaTcAvailableBuffersGet
* @endinternal
*
* @brief   Get amount of   buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.

* @param[in] confMode       Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc       Traffic class [0..7]
* @param[out] availableBuffersPtr - (pointer to)amount of available buffers
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS  cpssDxChPortPfcDbaTcAvailableBuffersGet
(
   IN  GT_U8   devNum,
   IN  GT_DATA_PATH_BMP dataPathBmp,
   IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
   IN  GT_U8   tc,
   OUT  GT_U32  *availableBuffersPtr
);

/**
* @internal cpssDxChPortPfcTcResourceModeSet
* @endinternal
*
* @brief  Configure mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tc       Traffic class [0..7]
* @param[in] mode - resource mode ( PB/Pool/Pool with headroom subtraction)
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcTcResourceModeSet
(
    IN GT_U8                                 devNum,
    IN GT_U8                                 tc,
    IN CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT    mode
);

/**
* @internal cpssDxChPortPfcTcResourceModeGet
* @endinternal
*
* @brief  Get configured mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] tc       Traffic class [0..7]
* @param[in] modePtr - (pointer to)resource mode ( PB/Pool/Pool with headroom subtraction)
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortPfcTcResourceModeGet
(
    IN GT_U8                                    devNum,
    IN GT_U8                                    tc,
    IN CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT       *modePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortPfch */

