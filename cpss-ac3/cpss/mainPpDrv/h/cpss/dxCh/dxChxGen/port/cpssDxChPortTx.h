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
* @file cpssDxChPortTx.h
*
* @brief CPSS implementation for configuring the Physical Port Tx Traffic Class
* Queues.
* This covers:
* - queuing enable/disable
* - transmit enable/disable
* - scheduling algorithms, bandwidth management
* - shaping on queues and the logical port
* - drop algorithms for congestion handling
*
* @version   62
********************************************************************************
*/

#ifndef __cpssDxChPortTxh
#define __cpssDxChPortTxh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/generic/cos/cpssCosTypes.h>



#define PRV_CPSS_DXCH_TXQREV1_TOKEN_BCKT_UPD_RATE_MIN_CNS        1
#define PRV_CPSS_DXCH_TXQREV1_TOKEN_BCKT_UPD_RATE_MAX_CNS        15
#define PRV_CPSS_DXCH_TXQREV1_SLOW_TOKEN_BCKT_UPD_RATIO_MIN_CNS  1
#define PRV_CPSS_DXCH_TXQREV1_SLOW_TOKEN_BCKT_UPD_RATIO_MAX_CNS  16
#define PRV_CPSS_DXCH_TXQREV1_TOKENS_REFILL_MAX_CNS              0xFFF
#define PRV_CPSS_DXCH_TXQREV1_SIP5_TOKENS_REFILL_MAX_CNS         0x3FFFF

/**
* @enum CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT
*
* @brief Token bucket maximum transmission unit (MTU)
*/
typedef enum{

    /** 1.5K */
    CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_1_5K_E,

    /** 2K */
    CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_2K_E,

    /** 10K */
    CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_10K_E

} CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT;

/**
* @enum CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT
*
* @brief Enumeration of shared pool allocation policy
*/
typedef enum{

    /** Unconstrained. A port may consume the entire shared pool. */
    CPSS_DXCH_PORT_TX_SHARED_POLICY_UNCONSTRAINED_E,

    /** @brief Constrained. The Port Tail Drop Threshold limits the total
     *  number of resources occupied by a port.
     */
    CPSS_DXCH_PORT_TX_SHARED_POLICY_CONSTRAINED_E

} CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT;

/**
* @enum CPSS_DXCH_PORT_TX_SHAPER_GRANULARITY_ENT
*
* @brief Enumeration of shaper granularity.
*/
typedef enum{

    /** @brief Granularity is 64 core clock cycles.
     *  (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
     */
    CPSS_DXCH_PORT_TX_SHAPER_GRANULARITY_64_CORE_CLOCKS_E,

    /** Granularity is 1024 core clock cycles. */
    CPSS_DXCH_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E,

    /** @brief Granularity is 128 core clock cycles.
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    CPSS_DXCH_PORT_TX_SHAPER_GRANULARITY_128_CORE_CLOCKS_E,

    /** @brief Granularity is 256 core clock cycles.
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    CPSS_DXCH_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E

} CPSS_DXCH_PORT_TX_SHAPER_GRANULARITY_ENT;


/**
* @struct CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC
*
* @brief Shaper configurations.
*/
typedef struct{

    /** @brief tokens rate for shapers(1..15).
     *  Granularity of this field defined by portsTokensRateGran.
     */
    GT_U32 tokensRate;

    /** @brief slow rate ratio. Tokens Update rate for ports with
     *  slow rate divided to the ratio (1..16).
     */
    GT_U32 slowRateRatio;

    /** Defines the granularity tokensRate. */
    CPSS_DXCH_PORT_TX_SHAPER_GRANULARITY_ENT tokensRateGran;

    /** @brief packet length in bytes for decreasing the shaper token
     *  bucket in packet based mode for non-CPU ports (0..0xFFFFFF).
     */
    GT_U32 portsPacketLength;

    /** @brief packet length in bytes for decreasing the
     *  shaper token bucket in packet based mode for CPU port (0..0xFFFFFF).
     *  Comments:
     *  None.
     */
    GT_U32 cpuPacketLength;

} CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC;

/**
* @enum CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT
*
* @brief Enumeration of how packet buffer consumption is
* calculated for Tail Drop and Scheduling.
*/
typedef enum{

    /** @brief The packet's buffer count is the actual number of buffers
     *  occupied by the received packet.
     */
    CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E,

    /** The packet's buffer count is a fixed configurable value. */
    CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E

} CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT;

/**
* @enum CPSS_PORT_TX_SHARED_DP_MODE_ENT
*
* @brief Drop Precedence (DP) enable mode to use the shared pool.
*/
typedef enum{

    /** @brief None of the packets (with any DP) can use the shared pool.
     *  Compatible with DxCh3, xCat and xCat3 Disable mode.
     */
    CPSS_PORT_TX_SHARED_DP_MODE_DISABLE_E = GT_FALSE,

    /** @brief All packets (with any DP) can use the shared pool.
     *  Compatible with DxCh3, xCat and xCat3 Enable mode.
     */
    CPSS_PORT_TX_SHARED_DP_MODE_ALL_E = GT_TRUE,

    /** Only packets with DP0 can use the shared pool. */
    CPSS_PORT_TX_SHARED_DP_MODE_DP0_E,

    /** Packets with DP0 and DP1 can use the shared pool. */
    CPSS_PORT_TX_SHARED_DP_MODE_DP0_DP1_E

} CPSS_PORT_TX_SHARED_DP_MODE_ENT;

/**
* @enum CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT
*
* @brief Enumeration of BC (Byte Count) Change enable options.
*/
typedef enum{

    /** BC change is disabled. */
    CPSS_DXCH_PORT_TX_BC_CHANGE_DISABLE_ALL_E,

    /** BC change is enabled for shaper only. */
    CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_SHAPER_ONLY_E,

    /** BC change is enabled for scheduler only. */
    CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_SCHEDULER_ONLY_E,

    /** BC change is enabled for both shaper and scheduler. */
    CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E

} CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT;


/**
* @struct CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC
*
* @brief Tail Drop limits enabling for Weigthed Random Tail Drop.
*/
typedef struct{

    /** @brief enable/disable WRTD for (Queue,DP) descriptor/buffer
     *  limits. (APPLICABLE DEVICES: Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL tcDpLimit;

    /** @brief enable/disable Port descriptor/buffer limits.
     *  (APPLICABLE DEVICES: Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL portLimit;

    /** @brief enable/disable Queue descriptor/buffer limits.
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL tcLimit;

    /** @brief enable/disable Shared Pool descriptor/buffer limits.
     *  (APPLICABLE DEVICES: Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL sharedPoolLimit;

} CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC;

/**
* @struct CPSS_PORT_TX_WRTD_MASK_LSB_STC
*
* @brief The Tail Drop WRTD masked lsbs' for each limit type.
* (APPLICABLE RANGES: Lion 0..9; xCat2 0..14)
*/
typedef struct{

    /** @brief The number of lsbs' masked for WRTD for (Queue,DP) limits.
     *  (APPLICABLE DEVICES: Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
     */
    GT_U32 tcDp;

    /** @brief The number of lsbs' masked for WRTD for Port limits.
     *  (APPLICABLE DEVICES: Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
     */
    GT_U32 port;

    /** @brief The number of lsbs' masked/bitmask  for WRTD for Queue limits. For Falcon represent bitmask.
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tc;

    /** @brief The number of lsbs' masked/bitmask  for WRTD  for Shared Pool limits. For Falcon represent bitmask.
     *  (APPLICABLE DEVICES: Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 pool;

    /** @brief The bitmask for WRTD for multicast limits.
     *  (APPLICABLE DEVICES:  Falcon)
     */
    GT_U32 mcast;

} CPSS_PORT_TX_WRTD_MASK_LSB_STC;

/**
* @struct CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC
*
* @brief The Multicast arbiter weights configured to the
* Multicast FIFOs.
* (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief the weight given to Multicast descriptors forwarded
     *  to MC FIFO 0.
     *  (APPLICABLE RANGES: 0..255)
     */
    GT_U32 mcFifo0;

    /** @brief the weight given to Multicast descriptors forwarded
     *  to MC FIFO 1.
     *  (APPLICABLE RANGES: 0..255)
     */
    GT_U32 mcFifo1;

    /** @brief the weight given to Multicast descriptors forwarded
     *  to MC FIFO 2.
     *  (APPLICABLE RANGES: 0..255)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 mcFifo2;

    /** @brief the weight given to Multicast descriptors forwarded
     *  to MC FIFO 3.
     *  (APPLICABLE RANGES: 0..255)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 mcFifo3;

} CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC;



/**
* @struct CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC
*
* @brief The profile parameters regsrding long queue managment.
*  Applicable devices : Falcon
*/

typedef struct
{

     /** @brief
   *Enables writing a queue to the PB when it is getting long
   */

     GT_BOOL longQueueEnable;

       /** @brief
    * When the head counter is below this limit, the head is considered “empty” and fragment read from PB is triggered
    */

    GT_U32 headEmptyLimit;

     /** @brief
    * When the total queue counter reaches this limit, a long queue is opened.
    */
    GT_U32 longQueueLimit;

} CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC;


/**
* @struct CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC
 *
 * @brief The profile parameters regsrding length adjust managment.
 *  Applicable devices : Falcon
*/

typedef struct
{
      /** @brief
    *If equals GT_TRUE the length adjust is enabled ,else disabled.
    */

        GT_BOOL lengthAdjustEnable;


    /** @brief
    *If equals GT_TRUE the value of constant byte count field is subtracted from the descriptor byte count ,else
    * the value of constant byte count field is added to the descriptor byte count. Applicable only if lengthAdjustEnable set to GT_TRUE.
    */

     GT_BOOL lengthAdjustSubstruct;

     /** @brief
    *Sets the value which will be decremented or incremented from the packet's byte count.Applicable only if lengthAdjustEnable set to GT_TRUE.
    */
     GT_U32 lengthAdjustByteCount;


} CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC;


/**
* @struct CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC
 *
 * @brief The profile parameters regarding queue profile.
 *  Applicable devices : Falcon
*/

typedef struct
{

    /** @brief
    * Parameters regarding length adjust.
    */
     CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC lengthAdjustParameters;

     /** @brief
    * Parameters regarding long queue managment.
    */

    CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC longQueueParameters;

} CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC;


/**
* @internal cpssDxChPortTxInit function
* @endinternal
*
* @brief   Init port Tx configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In SALSA family the first profile (profile_0) is used for CPU port and
*       the second one (profile_1) for network ports.
*
*/
GT_STATUS cpssDxChPortTxInit
(
    IN  GT_U8    devNum
);

/**
* @internal cpssDxChPortTxQueueEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing on all Tx queues on a specific device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE, Global enqueuing enabled
*                                      GT_FALSE, Global enqueuing disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQueueEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of enqueuing on all Tx queues on a specific device
*         (enabled/disabled).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - GT_TRUE, Global enqueuing enabled
*                                      GT_FALSE, Global enqueuing disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQueueEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChPortTxQueueingEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing to a Traffic Class queue
*         on the specified port of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this device (0..7)
* @param[in] enable                   - GT_TRUE, enable enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQueueingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortTxQueueingEnableGet function
* @endinternal
*
* @brief   Get the status of enqueuing to a Traffic Class queue
*         on the specified port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this device (0..7)
*
* @param[out] enablePtr                - GT_TRUE, enable enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQueueingEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChPortTxQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission from a Traffic Class queue
*         on the specified port of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port (0..7)
* @param[in] enable                   - GT_TRUE, enable transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQueueTxEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortTxQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission from a Traffic Class queue
*         on the specified port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port (0..7)
*
* @param[out] enablePtr                - GT_TRUE, enable transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQueueTxEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChPortTxFlushQueuesSet function
* @endinternal
*
* @brief   Flush all the traffic class queues on the specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  physical or CPU port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxFlushQueuesSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum
);

/**
* @internal cpssDxChPortTxShaperEnableSet function
* @endinternal
*
* @brief   Enable/Disable Token Bucket rate shaping on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] enable                   - GT_TRUE, enable Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Flow control must be disabled before disabling the shaper.
*
*/
GT_STATUS cpssDxChPortTxShaperEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortTxShaperEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable Token Bucket rate shaping status on specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] enablePtr                - Pointer to Token Bucket rate shaping status.
*                                      - GT_TRUE, enable Shaping
*                                      - GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT  GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortTxShaperProfileSet function
* @endinternal
*
* @brief   Set Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16M burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second
*                                      according to port shaper mode.
*                                      Packet per second relevant only for Lion and above.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For DxCh1, DxCh2, DxCh3, xCat and xCat2:
*       For port number in the range 0..23, whether G or XG type, the global
*       shaper used parameters are those referenced as gig ones -
*       "gigPortsTokensRate" and "gigPortsSlowRateRatio" inputs for
*       "cpssDxChPortTxShaperGlobalParamsSet" API.
*       2. For xCat-A3 and xCat2:
*       For port number in the range 24..27, whether G or XG type, the global
*       shaper used parameters are determined by
*       "cpssDxChPortTxShaperOnStackAsGigEnableSet".
*       3. For Lion:
*       If the shaper mode on a given port is packet based and the packet
*       length (portsPacketLength/cpuPacketLength) configuration isn't null,
*       maxRatePtr units are packets per second,
*       otherwise maxRatePtr units are Kbps.
*
*/
GT_STATUS cpssDxChPortTxShaperProfileSet
(
    IN    GT_U8     devNum,
    IN    GT_PHYSICAL_PORT_NUM  portNum,
    IN    GT_U16    burstSize,
    INOUT GT_U32    *maxRatePtr
);

/**
* @internal cpssDxChPortTxShaperProfileGet function
* @endinternal
*
* @brief   Get Token Bucket Shaper Profile on specified port of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] burstSizePtr             - (pointer to) burst size in units of 4K bytes
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
* @param[out] enablePtr                - (pointer to) Token Bucket rate shaping status.
*                                      - GT_TRUE - Token Bucket rate shaping is enabled.
*                                      - GT_FALSE - Token Bucket rate shaping is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Lion:
*       If the shaper mode on a given port is packet based and the packet
*       length (portsPacketLength/cpuPacketLength) configuration isn't null,
*       maxRatePtr units are packets per second,
*       otherwise maxRatePtr units are Kbps.
*
*/
GT_STATUS cpssDxChPortTxShaperProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U16  *burstSizePtr,
    OUT GT_U32  *maxRatePtr,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortTxQShaperEnableSet function
* @endinternal
*
* @brief   Enable/Disable shaping of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this port (0..7)
* @param[in] enable                   - GT_TRUE,  shaping on this traffic queue
*                                      GT_FALSE, disable shaping on this traffic queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQShaperEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortTxQShaperEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable shaping status
*         of transmitted traffic from a specified Traffic
*         Class Queue and specified port of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this port (0..7)
*
* @param[out] enablePtr                - Pointer to  shaping status
*                                      - GT_TRUE, enable shaping on this traffic queue
*                                      - GT_FALSE, disable shaping on this traffic queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQShaperEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChPortTxQShaperProfileSet function
* @endinternal
*
* @brief   Set Shaper Profile for Traffic Class Queue of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this Logical Port
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16M burst size)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or
*                                      or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For DxCh1, DxCh2, DxCh3, xCat and xCat2:
*       For port number in the range 0..23, whether G or XG type, the global
*       shaper used parameters are those referenced as gig ones -
*       "gigPortsTokensRate" and "gigPortsSlowRateRatio" inputs for
*       "cpssDxChPortTxShaperGlobalParamsSet" API.
*       2. For xCat-A3 and xCat2:
*       For port number in the range 24..27, whether G or XG type, the global
*       shaper used parameters are determined by
*       "cpssDxChPortTxShaperOnStackAsGigEnableSet".
*       3. For Lion:
*       If the shaper mode on a given port is packet based and the packet
*       length (portsPacketLength/cpuPacketLength) configuration isn't null,
*       maxRatePtr units are packets per second,
*       otherwise maxRatePtr units are Kbps.
*
*/
GT_STATUS cpssDxChPortTxQShaperProfileSet
(
    IN    GT_U8     devNum,
    IN    GT_PHYSICAL_PORT_NUM  portNum,
    IN    GT_U8     tcQueue,
    IN    GT_U16    burstSize,
    INOUT GT_U32    *maxRatePtr
);

/**
* @internal cpssDxChPortTxQShaperProfileGet function
* @endinternal
*
* @brief   Get Shaper Profile for Traffic Class Queue of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] burstSizePtr             - (pointer to) burst size in units of 4K bytes
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps
*                                      or packets per second.
* @param[out] enablePtr                - (pointer to) Token Bucket rate shaping status.
*                                      - GT_TRUE - Token Bucket rate shaping is enabled.
*                                      - GT_FALSE - Token Bucket rate shaping is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Lion:
*       If the shaper mode on a given port is packet based and the packet
*       length (portsPacketLength/cpuPacketLength) configuration isn't null,
*       maxRatePtr units are packets per second,
*       otherwise maxRatePtr units are Kbps.
*
*/
GT_STATUS cpssDxChPortTxQShaperProfileGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8    tcQueue,
    OUT GT_U16   *burstSizePtr,
    OUT GT_U32   *maxRatePtr,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortTxBindPortToSchedulerProfileSet function
* @endinternal
*
* @brief   Bind a port to scheduler profile set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - The Profile Set in which the scheduler's parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxBindPortToSchedulerProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
);

/**
* @internal cpssDxChPortTxBindPortToSchedulerProfileGet function
* @endinternal
*
* @brief   Get scheduler profile set that is binded to the port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] profileSetPtr            - The Profile Set in which the scheduler's parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxBindPortToSchedulerProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  *profileSetPtr
);

/**
* @internal cpssDxChPortTx4TcTailDropProfileSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..7).
* @param[in] tailDropProfileParamsPtr -
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTx4TcTailDropProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    IN    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *tailDropProfileParamsPtr
);

/**
* @internal cpssDxChPortTx4TcTailDropProfileGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..7).
*
* @param[out] tailDropProfileParamsPtr - Pointer to
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTx4TcTailDropProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT   CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *tailDropProfileParamsPtr
);

/**
* @internal cpssDxChPortTxTailDropProfileSet function
* @endinternal
*
* @brief Configures port dynamic limit alpha and set maximal port's limits of
*       buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                       Class Drop Parameters is associated
* @param[in] portAlpha                - ratio of the free buffers used for the port
*                                       thresholds (APPLICABLE DEVICES: Aldrin2; Falcon).
* @param[in] portMaxBuffLimit         - maximal number of buffers for a port.
*                                      (APPLICABLE RANGES: DxCh1,DxCh2 0..4095;
*                                       DxCh3,xCat,xCat3,Lion,xCat2 0..16383;
*                                       Bobcat2,Caelum,Bobcat3,Aldrin,Aldrin2 0..0xFFFFF)
* @param[in] portMaxDescrLimit        - maximal number of descriptors for a port.
*                                      (APPLICABLE RANGES: DxCh1,DxCh2 0..4095;
*                                       DxCh3,xCat,xCat3,Lion,xCat2,Aldrin2 0..16383;
*                                       Bobcat2,Caelum,Bobcat3,Aldrin 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropProfileSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    IN  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT portAlpha,
    IN  GT_U32                               portMaxBuffLimit,
    IN  GT_U32                               portMaxDescrLimit
);

/**
* @internal cpssDxChPortTxTailDropProfileGet function
* @endinternal
*
* @brief Get port dynamic limit alpha and set maximal port's limits of buffers
*       and descriptors.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                       Class Drop Parameters is associated
*
* @param[out] portAlphaPtr            - Pointer to ratio of the free buffers
*                                       used for the port thresholds
*                                       (APPLICABLE DEVICES: Aldrin2; Falcon).
* @param[out] portMaxBuffLimitPtr     - Pointer to maximal number of buffers for a port
* @param[out] portMaxDescrLimitPtr    - Pointer to maximal number of descriptors for a port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropProfileGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    OUT CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT *portAlphaPtr,
    OUT GT_U32                               *portMaxBuffLimitPtr,
    OUT GT_U32                               *portMaxDescrLimitPtr
);

/**
* @internal cpssDxChPortTxSniffedPcktDescrLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for mirrored packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] rxSniffMaxDescNum        - The number of descriptors allocated for packets
*                                      forwarded to the ingress analyzer port due to
*                                      mirroring.
*                                      For DxCh1,DxCh2: range 0..0xFFF
*                                      For DxCh3,xCat,xCat3,xCat2: range 0..0x3FFF
*                                      For Lion: range 0..0xFFFF
* @param[in] txSniffMaxDescNum        - The number of descriptors allocated for packets
*                                      forwarded to the egress analyzer port due to
*                                      mirroring.
*                                      For DxCh,DxCh2: range 0..0xFFF
*                                      For DxCh3,xCat,xCat3,xCat2: range 0..0x3FFF
*                                      For Lion: range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS cpssDxChPortTxSniffedPcktDescrLimitSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 rxSniffMaxDescNum,
    IN    GT_U32                 txSniffMaxDescNum
);

/**
* @internal cpssDxChPortTxSniffedPcktDescrLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for mirrored packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxSniffMaxDescNumPtr     - (pointer to) The number of descriptors allocated
*                                      for packets forwarded to the ingress analyzer
*                                      port due to mirroring.
* @param[out] txSniffMaxDescNumPtr     - (pointer to) The number of descriptors allocated
*                                      for packets forwarded to the egress analyzer
*                                      port due to mirroring.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSniffedPcktDescrLimitGet
(
    IN    GT_U8                  devNum,
    OUT   GT_U32                *rxSniffMaxDescNumPtr,
    OUT   GT_U32                *txSniffMaxDescNumPtr
);

/**
* @internal cpssDxChPortTxMcastPcktDescrLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] mcastMaxDescNum          - The number of descriptors allocated for multicast
*                                      packets.
*                                      For all devices except Lion, Bobcat2, Caelum, Aldrin, AC3X: in units of 128
*                                      descriptors, the actual number descriptors
*                                      allocated will be 128  mcastMaxDescNum.
*                                      For Lion, Bobcat2, Caelum, Aldrin, AC3X, Aldrin2: actual descriptors
*                                      number (granularity of 1).
*                                      For DxCh1,DxCh2,DxCh3,xCat,xCat3,xCat2: range 0..0xF80
*                                      For Lion, Bobcat2, Caelum, Aldrin, AC3X, Aldrin2: range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcastPcktDescrLimitSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 mcastMaxDescNum
);

/**
* @internal cpssDxChPortTxMcastPcktDescrLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                  - physical device number
*
* @param[out] mcastMaxDescNumPtr     - (pointer to) the number of descriptors allocated
*                                      for multicast packets.
*                                      For all devices except Lion, Bobcat2, Caelum, Aldrin, AC3X: in units of 128
*                                      descriptors, the actual number descriptors
*                                      allocated will be 128  mcastMaxDescNum.
*                                      For Lion, Bobcat2, Caelum, Aldrin, AC3X: actual descriptors
*                                      number (granularity of 1).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcastPcktDescrLimitGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *mcastMaxDescNumPtr
);

/**
* @internal cpssDxChPortTxWrrGlobalParamSet function
* @endinternal
*
* @brief   Set global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] wrrMode                  - Transmit Scheduler count mode.
*                                      (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.)
* @param[in] wrrMtu                   - MTU for DSWRR, resolution of WRR weights for byte based
*                                      Transmit Scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh1, DxCh2, DxCh3, xCat and xCat2 only CPSS_PORT_TX_WRR_MTU_2K_E
*       and CPSS_PORT_TX_WRR_MTU_8K_E are supported.
*
*/
GT_STATUS cpssDxChPortTxWrrGlobalParamSet
(
    IN    GT_U8                     devNum,
    IN    CPSS_PORT_TX_WRR_MODE_ENT wrrMode,
    IN    CPSS_PORT_TX_WRR_MTU_ENT  wrrMtu
);

/**
* @internal cpssDxChPortTxWrrGlobalParamGet function
* @endinternal
*
* @brief   Get global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] wrrModePtr               - (pointer to) Transmit Scheduler count mode.
* @param[out] wrrMtuPtr                - (pointer to) MTU for DSWRR, resolution of WRR weights
*                                      for byte based Transmit Scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChPortTxWrrGlobalParamGet
(
    IN    GT_U8                      devNum,
    OUT   CPSS_PORT_TX_WRR_MODE_ENT *wrrModePtr,
    OUT   CPSS_PORT_TX_WRR_MTU_ENT  *wrrMtuPtr
);

/**
* @internal cpssDxChPortTxShaperGlobalParamsSet function
* @endinternal
*
* @brief   Set Global parameters for shaper.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] xgPortsTokensRate        - tokens rate for XG ports' shapers (1..15)
* @param[in] gigPortsTokensRate       - tokens rate for Tri Speed ports' shapers.(1..15)
* @param[in] gigPortsSlowRateRatio    - slow rate ratio for Tri Speed ports.
*                                      Tokens Update rate for ports with slow rate
*                                      divided to the ratio. (1..16)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperGlobalParamsSet
(
    IN    GT_U8                   devNum,
    IN    GT_U32                  xgPortsTokensRate,
    IN    GT_U32                  gigPortsTokensRate,
    IN    GT_U32                  gigPortsSlowRateRatio
);

/**
* @internal cpssDxChPortTxShaperGlobalParamsGet function
* @endinternal
*
* @brief   Get Global parameters for shaper.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] xgPortsTokensRatePtr     - (pointer to) tokens rate for XG ports' shapers
* @param[out] gigPortsTokensRatePtr    - (pointer to) tokens rate for Tri Speed ports' shapers
* @param[out] gigPortsSlowRateRatioPtr - (pointer to) slow rate ratio for Tri Speed ports.
*                                      Tokens Update rate for ports with slow rate divided
*                                      to the ratio.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperGlobalParamsGet
(
    IN    GT_U8          devNum,
    OUT   GT_U32         *xgPortsTokensRatePtr,
    OUT   GT_U32         *gigPortsTokensRatePtr,
    OUT   GT_U32         *gigPortsSlowRateRatioPtr
);

/**
* @internal cpssDxChPortTxQWrrProfileSet function
* @endinternal
*
* @brief   Set Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue on this Port (0..7)
* @param[in] wrrWeight                - proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this
*                                      Arbitration Group - resolution is 1/255.
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - if weight will be less then port's MTU (maximum transmit unit) there
*       is possibility for empty WRR loops for given TC queue, but to not
*       tight user and for future ASIC's where this problem will be solved
*       check of wrrWeight 256 > MTU not implemented
*       - There is errata for Cheetah FEr#29. weigh cannot be = 255 for some
*       modes. The functions limits weight accordingly.
*
*/
GT_STATUS cpssDxChPortTxQWrrProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  GT_U8                                   wrrWeight,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
);

/**
* @internal cpssDxChPortTxQWrrProfileGet function
* @endinternal
*
* @brief   Get Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue on this Port (0..7)
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
*
* @param[out] wrrWeightPtr             - Pointer to proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this
*                                      Arbitration Group - resolution is 1/255.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQWrrProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT  GT_U8                                  *wrrWeightPtr
);

/**
* @internal cpssDxChPortTxQArbGroupSet function
* @endinternal
*
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7)
* @param[in] arbGroup                 - scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device or arbGroup
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQArbGroupSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_Q_ARB_GROUP_ENT            arbGroup,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
);

/**
* @internal cpssDxChPortTxQArbGroupGet function
* @endinternal
*
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7)
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQArbGroupGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT CPSS_PORT_TX_Q_ARB_GROUP_ENT            *arbGroupPtr
);

/**
* @internal cpssDxChPortTxBindPortToDpSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortTxBindPortToDpSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT  profileSet
);

/**
* @internal cpssDxChPortTxBindPortToDpGet function
* @endinternal
*
* @brief   Get Drop Profile Set according to a Physical Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
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
GT_STATUS cpssDxChPortTxBindPortToDpGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT   *profileSetPtr
);

/**
* @internal cpssDxChPortTxDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated per specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxDescNumberGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U16      *numberPtr
);

/**
* @internal cpssDxChPortTxBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxBufNumberGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *numPtr
);

/**
* @internal cpssDxChPortTx4TcDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] trafClass                - trafiic class (0..7)
*
* @param[out] numberPtr                - (pointer to) the number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: Do not call this API under traffic for Lion devices
*
*/
GT_STATUS cpssDxChPortTx4TcDescNumberGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8       trafClass,
    OUT GT_U16      *numberPtr
);

/**
* @internal cpssDxChPortTx4TcBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] trafClass                - trafiic class (0..7)
*
* @param[out] numPtr                   - (pointer to) the number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: Do not call this API under traffic for Lion devices
*
*/
GT_STATUS cpssDxChPortTx4TcBufNumberGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8       trafClass,
    OUT GT_U16      *numPtr
);

/**
* @internal cpssDxChPortTxToCpuShaperModeSet function
* @endinternal
*
* @brief   Set Shaper mode packet or byte based for CPU port shapers
*         Shapers are configured by cpssDxChPortTxShaperProfileSet or
*         cpssDxChPortTxQShaperProfileSet
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] mode                     - shaper mode: byte count or packet number based ackets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxToCpuShaperModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_SHAPER_MODE_ENT   mode
);

/**
* @internal cpssDxChPortTxToCpuShaperModeGet function
* @endinternal
*
* @brief   Get Shaper mode packet or byte based for CPU port shapers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] modePtr                  - (pointer to) shaper mode: byte count or packet number based ackets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssDxChPortTxToCpuShaperModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PORT_TX_DROP_SHAPER_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChPortTxSharingGlobalResourceEnableSet function
* @endinternal
*
* @brief   Enable/Disable sharing of resources for enqueuing of packets.
*         The shared resources configuration is set by
*         cpssDxChPortTxSharedGlobalResourceLimitsSet
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE - enable TX queue resourses sharing
*                                      GT_FALSE - disable TX queue resources sharing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSharingGlobalResourceEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChPortTxSharingGlobalResourceEnableGet function
* @endinternal
*
* @brief   Get enable/disable sharing of resources for enqueuing of packets.
*         The shared resources configuration is set by
*         cpssDxChPortTxSharedGlobalResourceLimitsSet
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE   - enable TX queue resourses sharing
*                                      GT_FALSE - disable TX queue resources sharing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSharingGlobalResourceEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChPortTxSharedGlobalResourceLimitsSet function
* @endinternal
*
* @brief   Configuration of shared resources for enqueuing of packets.
*         The using of shared resources is set
*         by cpssDxChPortTxSharingGlobalResourceEnableSet
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] sharedBufLimit           - The number of buffers in all Transmit queues that
*                                      can be shared between eligible packets.
*                                      For DxCh2: range 0..4095
*                                      For DxCh3,xCat,xCat3,xCat2: range 0..16380
* @param[in] sharedDescLimit          - The number of descriptors that can be shared
*                                      between all eligible packets.
*                                      For DxCh2: range 0..4095
*                                      For DxCh3,xCat,xCat3,xCat2: range 0..16380
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3,xCat,xCat3,xCat2: sharedBufLimit and sharedDescLimit are rounded up
*       to the nearest multiple of 4 since the corresponding fields in the
*       Transmit Queue Resource Sharing register are in 4 buffers\descriptors
*       resolution.
*
*/
GT_STATUS cpssDxChPortTxSharedGlobalResourceLimitsSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  sharedBufLimit,
    IN  GT_U32  sharedDescLimit
);

/**
* @internal cpssDxChPortTxSharedGlobalResourceLimitsGet function
* @endinternal
*
* @brief   Get the configuration of shared resources for enqueuing of packets.
*         The using of shared resources is set
*         by cpssDxChPortTxSharingGlobalResourceEnableSet
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] sharedBufLimitPtr        - (pointer to) The number of buffers in all
*                                      Transmit queues that
*                                      can be shared between eligible packets.
* @param[out] sharedDescLimitPtr       - (pointer to) The number of descriptors that
*                                      can be shared between all eligible packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSharedGlobalResourceLimitsGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *sharedBufLimitPtr,
    OUT GT_U32  *sharedDescLimitPtr
);

/**
* @internal cpssDxChPortTxSharedPolicySet function
* @endinternal
*
* @brief   Sets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] policy                   - shared pool allocation policy.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or wrong policy
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSharedPolicySet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT  policy
);

/**
* @internal cpssDxChPortTxSharedPolicyGet function
* @endinternal
*
* @brief   Gets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] policyPtr                - (pointer to) shared pool allocation policy.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSharedPolicyGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT  *policyPtr
);

/**
* @internal cpssDxChPortTxWatchdogEnableGet function
* @endinternal
*
* @brief   Get Tx Port Watchdog status and timeout
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] enablePtr                - (pointer to) the enable / disable state
* @param[out] timeoutPtr               - (pointer to) the watchdog timeout
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - enablePtr or timeoutPtr is a null pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxWatchdogEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U16      *timeoutPtr
);

/**
* @internal cpssDxChPortTxWatchdogEnableSet function
* @endinternal
*
* @brief   Enable/Disable Watchdog on specified port of specified port of
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] enable                   - GT_TRUE, enable Watchdog
*                                      GT_FALSE, disable Watchdog
* @param[in] timeout                  - Watchdog  in milliseconds (1..4K ms)
*                                      If disabling the Watchdog, this parameter is ignored
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxWatchdogEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL   enable,
    IN  GT_U16    timeout
);

/**
* @internal prvCpssDxChPortIpmBridgeCopyGetDroppedWaTailDropMaxLimitsSet function
* @endinternal
*
* @brief   Set tail drop max limit for WA of IPM Bridge Copy Dropped errata
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChPortIpmBridgeCopyGetDroppedWaTailDropMaxLimitsSet
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssDxChPortIpmBridgeCopyGetDroppedWaTailDropLimitsRestore function
* @endinternal
*
* @brief   Restore tail drop limit for WA of IPM Bridge Copy Dropped errata
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChPortIpmBridgeCopyGetDroppedWaTailDropLimitsRestore
(
    IN GT_U8 devNum
);

/**
* @internal cpssDxChPortTxTailDropUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
* @param[in] enable                   -  GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropUcEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChPortTxTailDropUcEnableGet function
* @endinternal
*
* @brief   Get enable/disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
*
* @param[out] enablePtr                -  pointer to tail drop status:
*                                      GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropUcEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortTxBufferTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Tail Drop according to the number of buffers in the
*         queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   -  GT_TRUE  -  Enables Tail Drop according to the number of
*                                      buffers in the queues. Tail drop use both
*                                      decsriptiors and buffers limits.
*                                      GT_FALSE - Tail drop use only descriptor limits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxBufferTailDropEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChPortTxBufferTailDropEnableGet function
* @endinternal
*
* @brief   Get enable/disable Tail Drop status according to the number of buffers
*         in the queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                -  pointer to Tail Drop status:
*                                      GT_TRUE  - Enables Tail Drop according to the number of
*                                      buffers in the queues. Tail drop use both
*                                      decsriptiors and buffers limits.
*                                      GT_FALSE - Tail drop use only descriptor limits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxBufferTailDropEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortTxBuffersSharingMaxLimitSet function
* @endinternal
*
* @brief   Sets the maximal number of shared buffers in a Tail Drop system.
*         When the total number of buffers exceeds this threshold, all of the
*         shared buffers are currently used and packets are queued according to
*         their target queue guaranteed limits.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] limit                    - maximal number of shared buffers in a Tail Drop system.
*                                      For DxCh2: range 0..0xFFF
*                                      To disable Buffers Sharing for Transmit queues
*                                      set this field to 0xFFF.
*                                      For DxCh3,xCat,xCat3,xCat2: range 0..0x3FFF
*                                      To disable Buffers Sharing for Transmit queues
*                                      set this field to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxBuffersSharingMaxLimitSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      limit
);

/**
* @internal cpssDxChPortTxBuffersSharingMaxLimitGet function
* @endinternal
*
* @brief   Gets the maximal number of shared buffers in a Tail Drop system.
*         When the total number of buffers exceeds this threshold, all of the
*         shared buffers are currently used and packets are queued according to
*         their target queue guaranteed limits.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] limitPtr                 -  pointer to maximal number of shared buffers
*                                      in a Tail Drop system
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxBuffersSharingMaxLimitGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *limitPtr
);

/**
* @internal cpssDxChPortTxDp1SharedEnableSet function
* @endinternal
*
* @brief   Enable/Disable packets with DP1 (Red) to use the shared
*         descriptors / buffers pool.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE   - Allow DP1 (Red) in shared pool.
*                                      GT_FALSE  - Disallow DP1 to be shared.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxDp1SharedEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChPortTxDp1SharedEnableGet function
* @endinternal
*
* @brief   Gets current status of shared descriptors / buffer pool usage
*         for packets with DP1 (Red).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - pointer to current status of
*                                      shared  descroptors / Buffers pool usage:
*                                      - GT_TRUE   - Allow DP1 (Red) in shared pool.
*                                      - GT_FALSE  - Allow DP0 (Green) only in shared pool.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxDp1SharedEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChPortTxTcSharedProfileEnableSet function
* @endinternal
*
* @brief   Enable/Disable usage of the shared descriptors / buffer pool for
*         packets with the traffic class (tc) that are transmited via a
*         port that is associated with the Profile (pfSet).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
* @param[in] enableMode               - Drop Precedence (DPs) enabled mode for sharing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTcSharedProfileEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT       pfSet,
    IN  GT_U8                                   tc,
    IN  CPSS_PORT_TX_SHARED_DP_MODE_ENT         enableMode
);

/**
* @internal cpssDxChPortTxTcSharedProfileEnableGet function
* @endinternal
*
* @brief   Gets usage of the shared descriptors / buffer pool status for
*         packets with the traffic class (tc) that are transmited via a
*         port that is associated with the Profile (pfSet).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
*
* @param[out] enableModePtr            - (pointer to) Drop Precedence (DPs) enabled mode for
*                                      sharing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTcSharedProfileEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT       pfSet,
    IN  GT_U8                                   tc,
    OUT CPSS_PORT_TX_SHARED_DP_MODE_ENT         *enableModePtr
);

/**
* @internal cpssDxChPortTxShaperTokenBucketMtuSet function
* @endinternal
*
* @brief   Set Token bucket maximum transmission unit (MTU).
*         The token bucket MTU defines the minimum number of tokens required to
*         permit a packet to be transmitted (i.e., conforming).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] mtu                      - MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mtu
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperTokenBucketMtuSet
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT mtu
);

/*******************************************************************************
* cpssDxChPortTxShaperTokenBucketMtuGet
*
*       Get Token bucket maximum transmission unit (MTU).
*       The token bucket MTU defines the minimum number of tokens required to
*       permit a packet to be transmitted (i.e., conforming).
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* NOT APPLICABLE DEVICES:
*        Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* INPUTS:
*       devNum   - physical device number
*
* OUTPUTS:
*       mtuPtr   -  pointer to MTU for egress rate shaper
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - on wrong device number
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_BAD_STATE             - on invalid hardware value read
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPortTxShaperTokenBucketMtuGet
(
    IN  GT_U8                                  devNum,
    OUT CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT *mtuPtr
);

/**
* @internal cpssDxChPortTxRandomTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Random Tail drop Threshold, to overcome synchronization.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   -  GT_TRUE  -  Enable Random Tail drop Threshold.
*                                      GT_FALSE -  Disable Random Tail drop Threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxRandomTailDropEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChPortTxRandomTailDropEnableGet function
* @endinternal
*
* @brief   Get Random Tail drop Threshold status.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                -  pointer to Random Tail drop Threshold status:
*                                      GT_TRUE  - Random Tail drop Threshold enabled.
*                                      GT_FALSE - Random Tail drop Threshold disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxRandomTailDropEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChTxPortAllShapersDisable function
* @endinternal
*
* @brief   Disables all ports and queues shapers for specified device in minimum delay.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTxPortAllShapersDisable
(
    IN  GT_U8    devNum
);

/**
* @internal cpssDxChTxPortShapersDisable function
* @endinternal
*
* @brief   Disable Shaping on Port and all it's queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] port                     - physical or CPU  number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTxPortShapersDisable
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM  port
);

/**
* @internal cpssDxChPortTxSchedulerDeficitModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable scheduler deficit mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE: Enable Scheduler Deficit mode.
*                                      GT_FALSE: Disable Scheduler Deficit mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Deficit mode should be enabled to support line-rate scheduling
*       in 40/100 Gbps ports.
*       2. Request Masks must be properly configured if deficit mode
*       is enabled, see:
*       cpssDxChPortTxRequestMaskSet
*       cpssDxChPortTxRequestMaskIndexSet.
*       3. If Deficit scheduling is enabled, the shaper's baseline must be
*       at least 8MTU, see:
*       cpssDxChPortTxShaperBaselineSet
*
*/
GT_STATUS cpssDxChPortTxSchedulerDeficitModeEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortTxSchedulerDeficitModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of scheduler deficit mode.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to) status of scheduler deficit mode
*                                      GT_TRUE: Scheduler deficit mode enabled.
*                                      GT_FALSE: Scheduler Deficit mode disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSchedulerDeficitModeEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortTxRequestMaskSet function
* @endinternal
*
* @brief   Sets request mask value to given mask index.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] maskIndex                - request mask index (0..3).
* @param[in] maskValue                - request mask value defines the minimum number of core clock
*                                      cycles between two consecutive service grants
*                                      to the same port (3..255).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or mask index
* @retval GT_OUT_OF_RANGE          - on out of range mask value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Request masks used only when deficit mode is enabled, see:
*       cpssDxChPortTxSchedulerDeficitModeEnableSet.
*       2. To bind a port to request mask index use:
*       cpssDxChPortTxRequestMaskIndexSet.
*
*/
GT_STATUS cpssDxChPortTxRequestMaskSet
(
    IN GT_U8    devNum,
    IN GT_U32   maskIndex,
    IN GT_U32   maskValue
);

/**
* @internal cpssDxChPortTxRequestMaskGet function
* @endinternal
*
* @brief   Gets request mask value to given mask index.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] maskIndex                - request mask index (0..3).
*
* @param[out] maskValuePtr             - (pointer to) request mask value defines the minimum number
*                                      of core clock cycles between two consecutive service
*                                      grants to the same port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or mask index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxRequestMaskGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   maskIndex,
    OUT GT_U32  *maskValuePtr
);

/**
* @internal cpssDxChPortTxRequestMaskIndexSet function
* @endinternal
*
* @brief   Binds a port to a request mask.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] maskIndex                - mask index (0..3).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range mask index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxRequestMaskIndexSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32   maskIndex
);

/**
* @internal cpssDxChPortTxRequestMaskIndexGet function
* @endinternal
*
* @brief   Gets the port's request mask.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] maskIndexPtr             - (pointer to) mask index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxRequestMaskIndexGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32   *maskIndexPtr
);
/**
* @internal cpssDxChPortTxShaperModeSet function
* @endinternal
*
* @brief   Set Shaper mode, packet or byte based for given port.
*         Shapers are configured by cpssDxChPortTxShaperProfileSet or
*         cpssDxChPortTxQShaperProfileSet
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number (CPU port supported as well as regular ports).
* @param[in] mode                     - shaper mode: byte count or packet number based.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperModeSet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_TX_DROP_SHAPER_MODE_ENT   mode
);

/**
* @internal cpssDxChPortTxShaperModeGet function
* @endinternal
*
* @brief   Get Shaper mode, packet or byte based for given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number (CPU port supported as well as regular ports).
*
* @param[out] modePtr                  - (pointer to) shaper mode: byte count or packet number based.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperModeGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_TX_DROP_SHAPER_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChPortTxShaperBaselineSet function
* @endinternal
*
* @brief   Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] baseline                 - Token Bucket Baseline value in bytes(0..0xFFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU.
*       2. If PFC response is enabled, the Baseline must be
*       at least 0x3FFFC0, see:
*       cpssDxChPortPfcEnableSet.
*       3. If Deficit scheduling is enabled, the Baseline must be
*       at least 8MTU, see:
*       cpssDxChPortTxSchedulerDeficitModeEnableSet.
*       4. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC. portsPacketLength
*       CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC. cpuPacketLength
*       see:
*       cpssDxChPortTxShaperConfigurationSet.
*
*/
GT_STATUS cpssDxChPortTxShaperBaselineSet
(
    IN GT_U8    devNum,
    IN GT_U32   baseline
);

/**
* @internal cpssDxChPortTxShaperBaselineGet function
* @endinternal
*
* @brief   Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective queue/port
*         is not served.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] baselinePtr              - (pointer to) Token Bucket Baseline value in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperBaselineGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *baselinePtr
);

/**
* @internal cpssDxChPortTxShaperConfigurationSet function
* @endinternal
*
* @brief   Set global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] configsPtr               - (pointer to) shaper configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       configsPtr->portsPacketLength
*       configsPtr->cpuPacketLength
*       see:
*       cpssDxChPortTxShaperBaselineSet.
*
*/
GT_STATUS cpssDxChPortTxShaperConfigurationSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC *configsPtr
);

/**
* @internal cpssDxChPortTxShaperConfigurationGet function
* @endinternal
*
* @brief   Get global configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] configsPtr               - (pointer to) shaper configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChPortTxShaperConfigurationGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC *configsPtr
);

/**
* @internal cpssDxChPortTxTailDropBufferConsumptionModeSet function
* @endinternal
*
* @brief   Sets the packet buffer consumption mode and its parameter.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - packet buffer consumption mode.
*                                      (APPLICABLE DEVICES Lion; Lion2.)
* @param[in] length                   - Defines the number of buffers consumed by a packet for
*                                      Tail Drop and Scheduling.
*                                      Range: 0..63
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on length out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function should be called only when traffic disabled
*       Mode configuration in Bobcat2,Caelum,Bobcat3; Aldrin2,Aldrin performed per profile.
*
*/
GT_STATUS cpssDxChPortTxTailDropBufferConsumptionModeSet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT mode,
    IN  GT_U32                                                  length
);

/**
* @internal cpssDxChPortTxTailDropBufferConsumptionModeGet function
* @endinternal
*
* @brief   Gets the packet buffer consumption mode and its parameter.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - (pointer to) packet buffer consumption mode.
*                                      (Not applicable for Bobcat2,Caelum,Bobcat3,Aldrin)
* @param[out] lengthPtr                - (pointer to ) the number of buffers consumed by a packet
*                                      for Tail Drop and Scheduling. Relevant only for mode
*                                      CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or wrong mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropBufferConsumptionModeGet
(
    IN  GT_U8                                                   devNum,
    OUT CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT *modePtr,
    OUT GT_U32                                                  *lengthPtr
);

/**
* @internal cpssDxChPortTxTcProfileSharedPoolSet function
* @endinternal
*
* @brief   Sets the shared pool associated for traffic class and Profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
* @param[in] poolNum                  - shared pool associated, range 0..7.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTcProfileSharedPoolSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   pfSet,
    IN  GT_U8                               tc,
    IN  GT_U32                              poolNum
);

/**
* @internal cpssDxChPortTxTcProfileSharedPoolGet function
* @endinternal
*
* @brief   Gets the shared pool associated for traffic class and Profile.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
*
* @param[out] poolNumPtr               - (pointer to) shared pool associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTcProfileSharedPoolGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   pfSet,
    IN  GT_U8                               tc,
    OUT GT_U32                              *poolNumPtr
);

/**
* @internal cpssDxChPortTxMcastBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] mcastMaxBufNum           - The number of buffers allocated for multicast
*                                      packets. Range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range mcastMaxBufNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcastBuffersLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mcastMaxBufNum
);


/**
* @internal cpssDxChPortTxMcastBuffersLimitGet function
* @endinternal
*
* @brief   Get maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] mcastMaxBufNumPtr        - (pointer to) the number of buffers allocated
*                                      for multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcastBuffersLimitGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *mcastMaxBufNumPtr
);

/**
* @internal cpssDxChPortTxSniffedBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for mirrored packets.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] rxSniffMaxBufNum         - The number of buffers allocated for packets
*                                      forwarded to the ingress analyzer port due to
*                                      mirroring.
*                                      (APPLICABLE RANGES: Lion, Lion2: 0..0xFFFF,
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X: 0..0xFFFFF)
* @param[in] txSniffMaxBufNum         - The number of buffers allocated for packets
*                                      forwarded to the egress analyzer port due to
*                                      mirroring.
*                                      (APPLICABLE RANGES: Lion, Lion2: 0..0xFFFF,
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS cpssDxChPortTxSniffedBuffersLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  rxSniffMaxBufNum,
    IN  GT_U32  txSniffMaxBufNum
);

/**
* @internal cpssDxChPortTxSniffedBuffersLimitGet function
* @endinternal
*
* @brief   Get maximal buffers limits for mirrored packets.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxSniffMaxBufNumPtr      - (pointer to) The number of buffers allocated
*                                      for packets forwarded to the ingress analyzer
*                                      port due to mirroring.
* @param[out] txSniffMaxBufNumPtr      - (pointer to) The number of buffers allocated
*                                      for packets forwarded to the egress analyzer
*                                      port due to mirroring.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSniffedBuffersLimitGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *rxSniffMaxBufNumPtr,
    OUT GT_U32  *txSniffMaxBufNumPtr
);

/**
* @internal cpssDxChPortTxSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*         APPLICABLE DEVICES:
*          Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES: xCat3;
*         INPUTS:
*         devNum   - physical device number
*         poolNum  - Shared pool number. Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Range 0..7
*                                                                   Falcon :Range 0..1
*         maxBufNum - The number of buffers allocated for a shared pool.
*            Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;  Range 0..0x3FFF.
*            Falcon:Range 0..0xFFFFF.
*         maxDescNum - The number of descriptors allocated for a shared pool.
*          NOT APPLICABLE DEVICES:Falcon
*         APPLICABLE RANGES:
*         Lion2, Bobcat2, Caelum, Aldrin, AC3X: 0..0x3FFF.
*
* @note   APPLICABLE DEVICES:       Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - Shared pool number.
*                                                               Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Range 0..7
*                                                               Falcon :Range 0..1
* @param[in] maxBufNum                - The number of buffers allocated for a shared pool.
*                                       Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: Range 0..0x3FFF.
*                                       Falcon:Range   0..0xFFFFF
* @param[in] maxDescNum               - The number of descriptors allocated for a shared pool.
*                                      APPLICABLE RANGES:
*                                      Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X: 0..0x3FFF.
*                                      APPLICABLE RANGES:
*                                      Bobcat3; Aldrin2;: 0..0x3F7F.
*                                     NOT APPLICABLE DEVICES:Falcon
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS cpssDxChPortTxSharedPoolLimitsSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  poolNum,
    IN  GT_U32  maxBufNum,
    IN  GT_U32  maxDescNum
);

/**
* @internal cpssDxChPortTxSharedPoolLimitsGet function
* @endinternal
*
* @brief   Get maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:       Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - Shared pool number.
*                                                               Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Range 0..7
*                                                               Falcon :Range 0..1

*
* @param[out] maxBufNumPtr             - (pointer to) The number of buffers allocated for a
*                                      shared pool.
* @param[out] maxDescNumPtr            - (pointer to) The number of descriptors allocated
*                                      for a shared pool.NOT APPLICABLE DEVICES:Falcon
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSharedPoolLimitsGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  poolNum,
    OUT GT_U32  *maxBufNumPtr,
    OUT GT_U32  *maxDescNumPtr
);

/**
* @internal cpssDxChPortTxProfileWeightedRandomTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Weighted Random Tail Drop Threshold to overcome
*         synchronization.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] dp                       - Drop Precedence
*                                      (APPLICABLE RANGES: Lion, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..2; xCat2 0..1)
* @param[in] tc                       - the Traffic Class, range 0..7.
*                                      (APPLICABLE DEVICES xCat2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
* @param[in] enablersPtr              - (pointer to) Tail Drop limits enabling
*                                      for Weigthed Random Tail Drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxProfileWeightedRandomTailDropEnableSet
(
    IN GT_U8                                    devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT        pfSet,
    IN GT_U32                                   dp,
    IN GT_U8                                    tc,
    IN CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC *enablersPtr
);

/**
* @internal cpssDxChPortTxProfileWeightedRandomTailDropEnableGet function
* @endinternal
*
* @brief   Get Random Tail drop Threshold status.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] dp                       - Drop Precedence
*                                      (APPLICABLE RANGES: Lion, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..2; xCat2 0..1)
* @param[in] tc                       - the Traffic Class, range 0..7.
*                                      (APPLICABLE DEVICES xCat2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
*
* @param[out] enablersPtr              - (pointer to) Tail Drop limits enabling
*                                      for Weigthed Random Tail Drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxProfileWeightedRandomTailDropEnableGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT        pfSet,
    IN  GT_U32                                   dp,
    IN  GT_U8                                    tc,
    OUT CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC *enablersPtr
);

/**
* @internal cpssDxChPortTxTailDropWrtdMasksSet function
* @endinternal
*
* @brief   Sets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - physical device number
* @param[in] maskLsbPtr               - WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropWrtdMasksSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_TX_WRTD_MASK_LSB_STC  *maskLsbPtr
);

/**
* @internal cpssDxChPortTxTailDropWrtdMasksGet function
* @endinternal
*
* @brief   Gets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - physical device number
*
* @param[out] maskLsbPtr               - (pointer to) WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropWrtdMasksGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_PORT_TX_WRTD_MASK_LSB_STC  *maskLsbPtr
);

/**
* @internal cpssDxChPortTxResourceHistogramThresholdSet function
* @endinternal
*
* @brief   Sets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrNum                  - Histogram Counter number. Range 0..3.
* @param[in] threshold                - If the Global Descriptors Counter exceeds this Threshold,
*                                      the Histogram Counter is incremented by 1.
*                                      Range 0..0x3FFF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxResourceHistogramThresholdSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  cntrNum,
    IN  GT_U32  threshold
);

/**
* @internal cpssDxChPortTxResourceHistogramThresholdGet function
* @endinternal
*
* @brief   Gets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrNum                  - Histogram Counter number. Range 0..3.
*
* @param[out] thresholdPtr             - (pointer to) If the Global Descriptors Counter
*                                      exceeds this Threshold, the Histogram Counter
*                                      is incremented by 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxResourceHistogramThresholdGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  cntrNum,
    OUT GT_U32  *thresholdPtr
);

/**
* @internal cpssDxChPortTxResourceHistogramCounterGet function
* @endinternal
*
* @brief   Gets Histogram Counter.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrNum                  - Histogram Counter number. Range 0..3.
*
* @param[out] cntrPtr                  - (pointer to) Histogram Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Histogram Counter is cleared on read.
*
*/
GT_STATUS cpssDxChPortTxResourceHistogramCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  cntrNum,
    OUT GT_U32  *cntrPtr
);

/**
* @internal cpssDxChPortTxGlobalDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxGlobalDescNumberGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *numberPtr
);

/**
* @internal cpssDxChPortTxGlobalBufNumberGet function
* @endinternal
*
* @brief   Gets total number of virtual buffers enqueued.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*
*/
GT_STATUS cpssDxChPortTxGlobalBufNumberGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *numberPtr
);

/**
* @internal cpssDxChPortTxMcastDescNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target descriptors allocated.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS cpssDxChPortTxMcastDescNumberGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *numberPtr
);

/**
* @internal cpssDxChPortTxMcastBufNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target buffers allocated (virtual buffers).
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*       Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS cpssDxChPortTxMcastBufNumberGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *numberPtr
);

/**
* @internal cpssDxChPortTxSniffedDescNumberGet function
* @endinternal
*
* @brief   Gets total number of mirrored descriptors allocated.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxNumberPtr              - number of ingress mirrored descriptors.
* @param[out] txNumberPtr              - number of egress mirrored descriptors.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSniffedDescNumberGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *rxNumberPtr,
    OUT GT_U32  *txNumberPtr
);

/**
* @internal cpssDxChPortTxSniffedBufNumberGet function
* @endinternal
*
* @brief   Gets total number of mirrored buffers allocated.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxNumberPtr              - number of ingress mirrored buffers.
* @param[out] txNumberPtr              - number of egress mirrored buffers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSniffedBufNumberGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *rxNumberPtr,
    OUT GT_U32  *txNumberPtr
);

/**
* @internal cpssDxChPortTxSharedResourceDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated from the shared descriptors
*         pool.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSharedResourceDescNumberGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tcQueue,
    OUT GT_U32  *numberPtr
);

/**
* @internal cpssDxChPortTxSharedResourceBufNumberGet function
* @endinternal
*
* @brief   Gets the number of virtual buffers enqueued from the shared buffer pool.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSharedResourceBufNumberGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tcQueue,
    OUT GT_U32  *numberPtr
);

/**
* @internal cpssDxChPortTxShaperOnStackAsGigEnableSet function
* @endinternal
*
* @brief   Enables/Disables the shaper on stacking ports (24-27) to operate as
*         Tri Speed ports' shapers.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE, stacking ports shapers operates
*                                      as Tri Speed ports' shapers.
*                                      GT_FALSE, stacking ports shapers operates as XG ports.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When "Stack As Gig" enabled, for a port number in the range 24..27,
*       whether Gig or XG type, the global shaper used parameters are those
*       referenced as gig ones -
*       "gigPortsTokensRate" and "gigPortsSlowRateRatio" inputs for
*       "cpssDxChPortTxShaperGlobalParamsSet" API.
*
*/
GT_STATUS cpssDxChPortTxShaperOnStackAsGigEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortTxShaperOnStackAsGigEnableGet function
* @endinternal
*
* @brief   Gets the current state (enable/disable) of the shaper's mode on
*         stacking ports (24-27).
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE, stacking ports shapers operates
*                                      as Tri Speed ports' shapers.
*                                      GT_FALSE, stacking ports shapers operates as XG ports.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperOnStackAsGigEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortProfileTxByteCountChangeEnableSet function
* @endinternal
*
* @brief   Enables/Disables profile Byte Count Change of the packet length by per port
*         constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - scheduler profile index
* @param[in] bcMode                   - byte count change enable options
* @param[in] bcOp                     - addition or subtraction of the per port value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile, bcMode, bcOp
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortProfileTxByteCountChangeEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profile,
    IN  CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT  bcMode,
    IN  CPSS_ADJUST_OPERATION_ENT               bcOp
);

/**
* @internal cpssDxChPortProfileTxByteCountChangeEnableGet function
* @endinternal
*
* @brief   Get profile Byte Count Change mode.
*         The byte count change mode changes the length of a packet by per port
*         constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - scheduler profile index
*
* @param[out] bcModePtr                - (pointer to) byte count change enable options.
* @param[out] bcOpPtr                  - (pointer to) addition or subtraction of the per port value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortProfileTxByteCountChangeEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profile,
    OUT CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT  *bcModePtr,
    OUT CPSS_ADJUST_OPERATION_ENT               *bcOpPtr
);


/**
* @internal cpssDxChPortTxByteCountChangeEnableSet function
* @endinternal
*
* @brief   Enables/Disables Byte Count Change of the packet length by per port
*         constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] bcChangeEnable           - Byte Count Change enable opition.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or BC change option
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxByteCountChangeEnableSet
(
    IN  GT_U8     devNum,
    IN  CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT bcChangeEnable
);

/**
* @internal cpssDxChPortTxByteCountChangeEnableGet function
* @endinternal
*
* @brief   Gets the Byte Count Change enable opition.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] bcChangeEnablePtr        - (pointer to) Byte Count Change enable opition.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxByteCountChangeEnableGet
(
    IN  GT_U8     devNum,
    OUT CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT *bcChangeEnablePtr
);


/**
* @internal cpssDxChPortTxByteCountChangeValueSet function
* @endinternal
*
* @brief   Sets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] bcValue                  - number of bytes added to the length of every frame
*                                      for shaping/scheduling rate calculation.
*                                      (APPLICABLE RANGES: xCat, xCat3, xCat2 -63..63;
*                                      Bobcat2 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To globally enable/disable Byte Count Change,
*       use cpssDxChPortTxByteCountChangeEnableSet
*       2. To update bcValue for given port Byte Count Change globally disabled.
*       This can cause side effects to shaper/scheduler behavior
*       on other ports if performed under traffic.
*
*/
GT_STATUS cpssDxChPortTxByteCountChangeValueSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_32     bcValue
);

/**
* @internal cpssDxChPortTxByteCountChangeValueGet function
* @endinternal
*
* @brief   Gets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] bcValuePtr               - (pointer to) number of bytes added to the length of
*                                      every frame for shaping/scheduling rate calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxByteCountChangeValueGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_32    *bcValuePtr
);

/**
* @internal cpssDxChPortTxGlobalDescLimitSet function
* @endinternal
*
* @brief   Set limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:     Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3;.
*
* @param[in] devNum                   - device number.
* @param[in] limit                    - total descriptor limit value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on limit out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Improper configuration of descriptors limit may affect entire system behavior
*
*/
GT_STATUS cpssDxChPortTxGlobalDescLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  limit
);

/**
* @internal cpssDxChPortTxGlobalDescLimitGet function
* @endinternal
*
* @brief   Get limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:     Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3;.
*
* @param[in] devNum                   - device number.
*
* @param[out] limitPtr                 -  pointer to total descriptor limit value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxGlobalDescLimitGet
(
    IN  GT_U8   devNum,
    OUT GT_U32 *limitPtr
);

/**
* @internal cpssDxChPortTxMcFifoSet function
* @endinternal
*
* @brief   Enable/Disable selected MC FIFO for specified egress port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mcFifo                   - MC FIFO number (APPLICABLE RANGES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2 0..3;
*                                                                          Falcon 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev, port or mcFifo
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcFifoSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U32        mcFifo
);

/**
* @internal cpssDxChPortTxMcFifoGet function
* @endinternal
*
* @brief   Gets state (Enable/Disable) of selected MC FIFO for specified egress port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] mcFifoPtr                - (pointer to) MC FIFO number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong HW configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcFifoGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_U32       *mcFifoPtr
);

/**
* @internal cpssDxChPortTxMcFifoArbiterWeigthsSet function
* @endinternal
*
* @brief   Configure the Multicast arbiter weights for MC FIFOs.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] weigthsPtr               - (pointer to) Multicast arbiter weights
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev
* @retval GT_OUT_OF_RANGE          - FIFO weight out of range
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcFifoArbiterWeigthsSet
(
    IN GT_U8        devNum,
    IN CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC  *weigthsPtr
);

/**
* @internal cpssDxChPortTxMcFifoArbiterWeigthsGet function
* @endinternal
*
* @brief   Get the Multicast arbiter weights for MC FIFOs configuration.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] weigthsPtr               - (pointer to) Multicast arbiter weights
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcFifoArbiterWeigthsGet
(
    IN GT_U8        devNum,
    OUT CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC  *weigthsPtr
);

/**
* @internal cpssDxChPortTxQueueGlobalTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission of specified device.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQueueGlobalTxEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortTxQueueGlobalTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQueueGlobalTxEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
);


/**
* @internal cpssDxChPortTxMcShaperMaskSet function
* @endinternal
*
* @brief   Function sets the multicast shaper mask value.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - physical device number
* @param[in] mask                     -  value (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong mask
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcShaperMaskSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mask
);


/**
* @internal cpssDxChPortTxMcShaperMaskGet function
* @endinternal
*
* @brief   Function gets the multicast shaper mask value.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - physical device number
*
* @param[out] maskPtr                  - (pointer to) mask value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxMcShaperMaskGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *maskPtr
);

/**
* @internal cpssDxChPortTxTailDropProfileBufferConsumptionModeSet function
* @endinternal
*
* @brief   Sets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - tail drop profile.
* @param[in] mode                     - buffer consumption mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropProfileBufferConsumptionModeSet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT                       profile,
    IN  CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT mode
);

/**
* @internal cpssDxChPortTxTailDropProfileBufferConsumptionModeGet function
* @endinternal
*
* @brief   Gets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - tail drop profile
*
* @param[out] modePtr                  - (pointer to) buffer consumption mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssDxChPortTxTailDropProfileBufferConsumptionModeGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT                       profile,
    OUT CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT *modePtr
);

/**
* @internal cpssDxChPortTxSchedulerProfileBufferConsumptionModeSet function
* @endinternal
*
* @brief   Sets the packet buffer consumption mode per Scheduler profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile.
* @param[in] mode                     - buffer consumption mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSchedulerProfileBufferConsumptionModeSet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT                  profile,
    IN  CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT mode
);

/**
* @internal cpssDxChPortTxSchedulerProfileBufferConsumptionModeGet function
* @endinternal
*
* @brief   Gets the packet buffer consumption mode per Scheduler profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile.
*
* @param[out] modePtr                  - (pointer to) buffer consumption mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssDxChPortTxSchedulerProfileBufferConsumptionModeGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT                  profile,
    OUT CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT *modePtr
);

/**
* @internal cpssDxChPortTxShaperAvbModeEnableSet function
* @endinternal
*
* @brief   Set Shaper AVB Mode of specified port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical or CPU port number
* @param[in] avbModeEnable            - AVB mode
*                                      GT_TRUE - AVB mode enabled
*                                      GT_FALSE - AVB mode disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxShaperAvbModeEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 avbModeEnable
);


/**
* @internal cpssDxChPortTxShaperAvbModeEnableGet function
* @endinternal
*
* @brief   Get status of Shaper AVB Mode of specified port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] avbModeEnablePtr         (pointer to) AVB mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssDxChPortTxShaperAvbModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *avbModeEnablePtr
);


/**
* @internal cpssDxChPortTxQShaperAvbModeEnableSet function
* @endinternal
*
* @brief   Set Shaper AVB Mode for Traffic Class Queue of specified port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this Logical Port
* @param[in] avbModeEnable            - AVB mode
*                                      GT_TRUE - AVB mode enabled
*                                      GT_FALSE - AVB mode disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, portNum, tc
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxQShaperAvbModeEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    IN  GT_BOOL                 avbModeEnable
);

/**
* @internal cpssDxChPortTxQShaperAvbModeEnableGet function
* @endinternal
*
* @brief   Get status of Shaper AVB Mode for Traffic Class Queue of specified port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this Logical Port
*
* @param[out] avbModeEnablePtr         (pointer to) AVB mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, portNum, tc
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssDxChPortTxQShaperAvbModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_BOOL                 *avbModeEnablePtr
);


/**
* @internal cpssDxChPortTxSchedulerProfileCountModeSet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile.
* @param[in] wrrMode                  - scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxSchedulerProfileCountModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profile,
    IN  CPSS_PORT_TX_WRR_MODE_ENT               wrrMode
);

/**
* @internal cpssDxChPortTxSchedulerProfileCountModeGet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile.
*
* @param[out] wrrModePtr               - (pointer to) scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssDxChPortTxSchedulerProfileCountModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profile,
    OUT CPSS_PORT_TX_WRR_MODE_ENT               *wrrModePtr
);


/**
* @internal cpssDxChPortTxTailDropMaskSharedBuffEnableSet function
* @endinternal
*
* @brief   Enable/Disable shared buffer indication masking.
*         When enabled, the tail drop mechanism will ignore
*         the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE - Enable buffer fill level masking.
*                                      GT_FALSE -Disable buffer fill level masking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropMaskSharedBuffEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChPortTxTailDropMaskSharedBuffEnableGet function
* @endinternal
*
* @brief   Gets status of shared buffer indication masking.
*         When enabled, the tail drop mechanism will ignore
*         the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to) shared buffer indication masking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssDxChPortTxTailDropMaskSharedBuffEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);


/**
* @internal cpssDxChPortTxTailDropSharedBuffMaxLimitSet function
* @endinternal
*
* @brief   Set max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] maxSharedBufferLimit     - maximal shared buffer limit
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on illegal maxSharedBufferLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropSharedBuffMaxLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  maxSharedBufferLimit
);


/**
* @internal cpssDxChPortTxTailDropSharedBuffMaxLimitGet function
* @endinternal
*
* @brief   Get max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] maxSharedBufferLimitPtr  - (pointer to) maximal shared buffer limit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssDxChPortTxTailDropSharedBuffMaxLimitGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *maxSharedBufferLimitPtr
);

/**
* @internal cpssDxChPortTx4TcMcBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of MC buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] trafClass                - trafiic class
*
* @param[out] numPtr                   - (pointer to) the number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTx4TcMcBufNumberGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8       trafClass,
    OUT GT_U32      *numPtr
);

/**
* @internal prvCpssDxChPortTxQStatusGet function
* @endinternal
*
* @brief   dumps the related TxQ parameters in the system.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS prvCpssDxChPortTxQStatusGet
(
    GT_U8 devNum
);

/**
* @internal prvCpssDxChPortTxTailDropResourceAllocationGet function
* @endinternal
*
* @brief   Dumps tail drop configuration showing the current resources allocated per port/TC.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssDxChPortTxTailDropResourceAllocationGet
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChPortTxTailDropSharePoolEnPerProfileGet function
* @endinternal
*
* @brief   Dumps tail drop configuration related to share pools.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
*/
GT_STATUS prvCpssDxChPortTxTailDropSharePoolEnPerProfileGet
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChPortTxTailDropGlobalConfigurationGet function
* @endinternal
*
* @brief   Dumps global tail drop configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS prvCpssDxChPortTxTailDropGlobalConfigurationGet
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChPortTxTailDropProfilesGet function
* @endinternal
*
* @brief   Dumps Tail Drop configuration related tail drop configuration and resource allocation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
*/
GT_STATUS prvCpssDxChPortTxTailDropProfilesGet
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChPortTxTailDropPortToTDProfileGet function
* @endinternal
*
* @brief   Dumps tail drop configuration showing the port and its tail drop profile.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
*/
GT_STATUS prvCpssDxChPortTxTailDropPortToTDProfileGet
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChPortTxSchedulerConfigGet function
* @endinternal
*
* @brief   dumps the related WRTD parameters in the system.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS prvCpssDxChPortTxSchedulerConfigGet
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChPortTxTailDropWRTDinfoGet function
* @endinternal
*
* @brief   dumps the related WRTD parameters in the system.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS prvCpssDxChPortTxTailDropWRTDinfoGet
(
    IN  GT_U8 devNum
);

/* values for the tail drop dump bitmap */
#define PRV_CPSS_DXCH_DUMP_TAIL_DROP_GLOBAL_CONFIG              0x1
#define PRV_CPSS_DXCH_DUMP_TAIL_DROP_PROFILES                   0x2
#define PRV_CPSS_DXCH_DUMP_TAIL_DROP_PORT_TO_TD_PROFILE         0x4
#define PRV_CPSS_DXCH_DUMP_TAIL_DROP_SCHEDULER_CONFIG           0x8
#define PRV_CPSS_DXCH_DUMP_TAIL_DROP_WRTD_INFO                  0x10
#define PRV_CPSS_DXCH_DUMP_TAIL_DROP_RESOURCE_ALLOCATION        0x20
#define PRV_CPSS_DXCH_DUMP_TAIL_DROP_SHARE_POOL                 0x40
#define PRV_CPSS_DXCH_DUMP_TX_Q_STATUS                          0x80

/**
* @internal cpssDxChPortTxQueueDumpAll function
* @endinternal
*
* @brief   Dumps all related Tx Queue information.
*         That includes the static configuration and resources allocated at run-time
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
* @param[in] tailDropDumpBmp          - bitmap to indicate what info to show (0xff for all)
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_BAD_PARAM             - on tailDropDumpBmp not in range.
*/
GT_STATUS cpssDxChPortTxQueueDumpAll
(
    IN  GT_U8 devNum,
    IN  GT_U32 tailDropDumpBmp
);

/**
 * @internal cpssDxChPortTxBufferStatisticsEnableSet function
 * @endinternal
 *
 * @brief  Enable/disable port buffer statistics.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X.
 *
 * @param[in] devNum                   - device number
 * @param[in] enPortStat               - enable port statistics
 *                                       GT_FALSE - disable port buffer statistics.
 *                                       GT_TRUE  - enable port buffer statistics.
 * @param[in] enQueueStat              - enable port queue statistics
 *                                       GT_FALSE - disable port queue buffer statistics.
 *                                       GT_TRUE  - enable port queue buffer statistics.
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS cpssDxChPortTxBufferStatisticsEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_BOOL              enPortStat,
    IN  GT_BOOL              enQueueStat
);

/**
 * @internal cpssDxChPortTxBufferStatisticsEnableGet function
 * @endinternal
 *
 * @brief  Get enable status of port buffer statistics.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X.
 *
 * @param[in]  devNum                   - device number
 *
 * @param[out] enPortStatPtr            - (pointer to) enabled status of
 *                                        port buffer statistics
 * @param[out] enQueueStatPtr           - (pointer to) enabled status of
 *                                        queue buffer statistics
 *
 * @retval GT_OK                        - on success
 * @retval GT_BAD_PTR                   - on NULL pointer
 * @retval GT_BAD_PARAM                 - wrong value in any of the parameters
 * @retval GT_HW_ERROR                  - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
 */
GT_STATUS cpssDxChPortTxBufferStatisticsEnableGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *enPortStatPtr,
    OUT GT_BOOL              *enQueueStatPtr
);

/**
 * @internal cpssDxChPortTxMaxBufferFillLevelSet function
 * @endinternal
 *
 * @brief  Set max port buffer fill level per port.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] maxBuffFillLvl           - max buffer fill level (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS cpssDxChPortTxMaxBufferFillLevelSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               maxBuffFillLvl
);

/**
 * @internal cpssDxChPortTxMaxBufferFillLevelGet function
 * @endinternal
 *
 * @brief  Get max port buffer fill level per port.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 *
 * @param[out] maxBuffFillLvlPtr       - (pointer to)max buffer fill level
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS cpssDxChPortTxMaxBufferFillLevelGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *maxBuffFillLvlPtr
);

/**
 * @internal cpssDxChPortTxQueueMaxBufferFillLevelSet function
 * @endinternal
 *
 * @brief  Set max port queue buffer fill level per port.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] tc                       - traffic class queue (0..7)
 * @param[in] maxBuffFillLvl           - max buffer fill level (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS cpssDxChPortTxQueueMaxBufferFillLevelSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    IN  GT_U32               maxBuffFillLvl
);

/**
 * @internal cpssDxChPortTxQueueMaxBufferFillLevelGet function
 * @endinternal
 *
 * @brief  Get max port queue buffer fill level per port.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] tc                       - traffic class queue (0..7)
 *
 * @param[out] maxBuffFillLvlPtr       - (pointer to)max buffer fill level
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS cpssDxChPortTxQueueMaxBufferFillLevelGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    OUT GT_U32               *maxBuffFillLvlPtr
);


/**
 * @internal cpssDxChPortTxQueueProfileSet function
 * @endinternal
 *
 * @brief  Set queue profile( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] profileIndex           - index of profile (0..15)
 * @param[in] profile                      - queue profile parameters
 *
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */

GT_STATUS cpssDxChPortTxQueueProfileSet
(
    IN  GT_U8   devNum,
    IN GT_U32  profileIndex,
    IN CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC profile
);

/**
 * @internal cpssDxChPortTxQueueProfileGet function
 * @endinternal
 *
 * @brief Get queue profile( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] profileIndex           - index of profile (0..15)
 * @param[out] profilePtr                      - (pointer to)queue profile parameters
 *
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */

GT_STATUS cpssDxChPortTxQueueProfileGet
(
    IN  GT_U8   devNum,
    IN GT_U32  profileIndex,
    IN CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC * profilePtr
);

/**
 * @internal cpssDxChPortTxBindQueueToProfileSet function
 * @endinternal
 *
 * @brief Bind queue to queue  profile( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[in] profileIndex           - index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS cpssDxChPortTxBindQueueToProfileSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    IN GT_U32             profileIndex
);
/**
 * @internal cpssDxChPortTxBindQueueToProfileGet function
 * @endinternal
 *
 * @brief Get binded  queue  profile index( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[out] profileIndexPtr           - (pointer to)index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS cpssDxChPortTxBindQueueToProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    IN GT_U32         *    profileIndexPtr
);

/**
 * @internal cpssDxChPortTxMcFifoPriorityAttributesSet function
 * @endinternal
 *
 * @brief   Sets priority multicast attributes for specified MC FIFO.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
 *                                  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] mcFifo                   - MC FIFO number (APPLICABLE RANGE: 0..1)
 * @param[in] threshold                - threshold for acceptance of non-priority multicast frames
 *                                       (APPLICABLE RANGE: 0..0xF)
 * @param[in] minTc                    - minimum TC which is classified as priority multicast
 *                                       (APPLICABLE RANGE: 0..7)
 * @param[in] maxDp                    - maximum DP which is classified as priority multicast
 *
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_OUT_OF_RANGE             - on data out of range
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 **/
GT_STATUS cpssDxChPortTxMcFifoPriorityAttributesSet
(
    IN GT_U8                devNum,
    IN GT_U32               mcFifo,
    IN GT_U32               threshold,
    IN GT_U8                minTc,
    IN CPSS_DP_LEVEL_ENT    maxDp
);

/**
 * @internal cpssDxChPortTxMcFifoPriorityAttributesGet function
 * @endinternal
 *
 * @brief   Gets priority multicast attributes for specified MC FIFO.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
 *                                  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number
 * @param[in] mcFifo                   - MC FIFO number (APPLICABLE RANGE: 0..1)
 * @param[in] thresholdPtr             - (pointer to) threshold for acceptance of non-priority multicast frames
 * @param[in] minTcPtr                 - (pointer to) minimum TC which is classified as priority multicast
 * @param[in] maxDpPtr                 - (pointer to) maximum DP which is classified as priority multicast
 *
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxMcFifoPriorityAttributesGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              mcFifo,
    OUT GT_U32              *thresholdPtr,
    OUT GT_U8               *minTcPtr,
    OUT CPSS_DP_LEVEL_ENT   *maxDpPtr
);

/**
 * @internal  cpssDxChPortTxMcastBuffersPriorityLimitSet function
 * @endinternal
 *
 * @brief   Set  the guaranteed buffers limit and ratio of the free buffers for multicast packets.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -          Device number
 * @param[in] mcastPriority                   - MC priority(low or high)
 * @param[in] mcastGuaranteedLimit      Guaranteed buffers limit
 * @param[in] alpha                 -                 Ratio of the free buffers
 *
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxMcastBuffersPriorityLimitSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT  mcastPriority,
    IN  GT_U32                                mcastGuaranteedLimit,
    IN  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT  alpha
);

/**
 * @internal  cpssDxChPortTxMcastBuffersPriorityLimitGet function
 * @endinternal
 *
 * @brief   Get  the guaranteed buffers limit and ratio of the free buffers for multicast packets.
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                        Device number
 * @param[in] mcastPriority                   -          MC priority(low or high)
 * @param[out] mcastGuaranteedLimitPtr      (pointer to)Guaranteed buffers limit
 * @param[out] alphaPtr                                     (pointer to)Ratio of the free buffers
 *
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxMcastBuffersPriorityLimitGet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT   mcastPriority,
    OUT  GT_U32                                *mcastGuaranteedLimitPtr,
    OUT  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT  *alphaPtr
);
/**
 * @internal  cpssDxChPortTxMcastAvailableBuffersSet function
 * @endinternal
 *
 * @brief   Set  the total available buffers for Multicast dynamic buffers allocation
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in] mcastAvailableBuffersNum         Total available buffers for Multicast dynamic buffers allocation
 *
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxMcastAvailableBuffersSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mcastAvailableBuffersNum
);
/**
 * @internal  cpssDxChPortTxMcastAvailableBuffersGet function
 * @endinternal
 *
 * @brief  Get  the total available buffers for Multicast dynamic buffers allocation
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in] mcastAvailableBuffersNumPtr         (pointer to)Total available buffers for Multicast dynamic buffers allocation
 *
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxMcastAvailableBuffersGet
(
    IN  GT_U8   devNum,
    OUT  GT_U32  *mcastAvailableBuffersNumPtr
);

/**
 * @internal  cpssDxChPortTxTcMapToSharedPoolSet function
 * @endinternal
 *
 * @brief  Set tail drop mapping between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in] tc                                                    Traffic class [0..7]
 *@param[in]  poolNum                                        Pool id[0..1]
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxTcMapToSharedPoolSet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               tc,
    IN  GT_U32                              poolNum
);

/**
 * @internal  cpssDxChPortTxTcMapToSharedPoolGet function
 * @endinternal
 *
 * @brief  Get tail drop mapping between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in] tc                                                    Traffic class [0..7]
 *@param[in]  poolNumPtr                                   (Pointer to)Pool id[0..1]
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxTcMapToSharedPoolGet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               tc,
    IN  GT_U32                              *poolNumPtr
);
/**
 * @internal  cpssDxChPortTxTailDropGlobalParamsSet function
 * @endinternal
 *
 * @brief  Set amount of available buffers for dynamic buffers allocation
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[in] resourceMode                              Defines which free buffers resource is used to calculate the
*                                                                                dynamic buffer limit for the Port/Queue limits(global or pool)
 *@param[in]  globalAvailableBuffers                 Global  amount of available buffers for dynamic buffers allocation
 *@param[in]  pool0AvailableBuffers                 Pool 0  amount of available buffers for dynamic buffers allocation
 *@param[in]  pool1AvailableBuffers                 Pool 1    amount of available buffers for dynamic buffers allocation
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxTailDropGlobalParamsSet
(
    IN    GT_U8                                     devNum,
    IN    CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT  resourceMode,
    IN    GT_U32                                    globalAvailableBuffers,
    IN    GT_U32                                    pool0AvailableBuffers,
    IN    GT_U32                                    pool1AvailableBuffers
);
/**
 * @internal  cpssDxChPortTxTailDropGlobalParamsGet function
 * @endinternal
 *
 * @brief  Get amount of available buffers for dynamic buffers allocation
 *
 * @note   APPLICABLE DEVICES:      Falcon.
 * @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                         Device number
 * @param[out] resourceMode                              Defines which free buffers resource is used to calculate the
*                                                                                dynamic buffer limit for the Port/Queue limits(global or pool)
 *@param[out]  globalAvailableBuffers                (pointer to)Global  amount of available buffers for dynamic buffers allocation
 *@param[out]  pool0AvailableBuffers                 (pointer to)Pool 0  amount of available buffers for dynamic buffers allocation
 *@param[out]  pool1AvailableBuffers                 (pointer to)Pool 1    amount of available buffers for dynamic buffers allocation
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssDxChPortTxTailDropGlobalParamsGet
(
    IN    GT_U8                                      devNum,
    IN    CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT  *resourceModePtr,
    IN    GT_U32                                    *globalAvailableBuffersPtr,
    IN    GT_U32                                    *pool0AvailableBuffersPtr,
    IN    GT_U32                                    *pool1AvailableBuffersPtr
);
/**
* @internal cpssDxChPortTx4TcTailDropWredProfileSet function
* @endinternal
*
* @brief   Set tail drop WRED profile  parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..7).
* @param[in] tailDropWredProfileParamsPtr -
*                                      the Drop Profile WRED Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTx4TcTailDropWredProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    IN    CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS *tailDropWredProfileParamsPtr
);
/**
* @internal cpssDxChPortTx4TcTailDropWredProfileGet function
* @endinternal
*
* @brief   Get tail drop WRED profile  parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..7).
* @param[in] tailDropWredProfileParamsPtr -
*                                      (pointer to)the Drop Profile WRED Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTx4TcTailDropWredProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT    CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS *tailDropWredProfileParamsPtr
);

/**
* @internal cpssDxChPortTxTailDropWredProfileSet function
* @endinternal
*
* @brief   Set tail drop port WRED profile  parameters .
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
*
* @param[in] tailDropWredProfileParamsPtr -
*                                      the Drop Profile WRED Parameters to associate
*                                      with the port in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropWredProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC     *tailDropWredProfileParamsPtr
);

/**
* @internal cpssDxChPortTxTailDropWredProfileGet function
* @endinternal
*
* @brief   Get tail drop port WRED profile  parameters .
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
*
* @param[in] tailDropWredProfileParamsPtr -
*                                      the Drop Profile WRED Parameters to associate
*                                      with the port  in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropWredProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    OUT   CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC     *tailDropWredProfileParamsPtr
);

/**
* @internal cpssDxChPortTxTailDropCommandSet function
* @endinternal
*
* @brief   Set tail drop command and drop code
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] cmd                             -Tail drop comand
* @param[in] tailDropCode                             -Tail drop code
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropCommandSet
(
    IN GT_U8                     devNum,
    IN CPSS_PACKET_CMD_ENT       cmd,
    IN CPSS_NET_RX_CPU_CODE_ENT  tailDropCode
);
/**
* @internal cpssDxChPortTxTailDropCommandGet function
* @endinternal
*
* @brief   Set tail drop command and drop code
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[out] cmdPtr                             -(pointer to)Tail drop comand
* @param[out] tailDropCodePtr               -(pointer to)Tail drop code
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssDxChPortTxTailDropCommandGet
(
    IN GT_U8                     devNum,
    IN CPSS_PACKET_CMD_ENT       *cmdPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT  *tailDropCodePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortTxh */

