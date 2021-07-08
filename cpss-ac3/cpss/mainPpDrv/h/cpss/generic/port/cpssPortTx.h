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
* @file cpssPortTx.h
*
* @brief CPSS definitions for port Tx Traffic Class Queues.
*
* @version   13
********************************************************************************
*/

#ifndef __cpssPortTxh
#define __cpssPortTxh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT
 *
 * @brief Enumeration of Tx Queue scheduler profile set
*/
typedef enum{

    /** @brief Tx Queue scheduler profile 1
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_1_E = 0,

    /** @brief Tx Queue scheduler profile 2
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_2_E,

    /** @brief Tx Queue scheduler profile 3
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_3_E,

    /** @brief Tx Queue scheduler profile 4
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_4_E,

    /** @brief Tx Queue scheduler profile 5
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_5_E,

    /** @brief Tx Queue scheduler profile 6
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_6_E,

    /** @brief Tx Queue scheduler profile 7
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_7_E,

    /** @brief Tx Queue scheduler profile 8
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_8_E,

    /** @brief Tx Queue scheduler profile 9
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_9_E,

    /** @brief Tx Queue scheduler profile 10
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_10_E,

    /** @brief Tx Queue scheduler profile 11
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_11_E,

    /** @brief Tx Queue scheduler profile 12
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_12_E,

    /** @brief Tx Queue scheduler profile 13
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_13_E,

    /** @brief Tx Queue scheduler profile 14
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_14_E,

    /** @brief Tx Queue scheduler profile 15
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_15_E,

    /** @brief Tx Queue scheduler profile 16
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_SCHEDULER_PROFILE_16_E

} CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT;

/**
* @enum CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
 *
 * @brief The Tail Drop Dynamic Buffers Allocation (DBA) ratio of the free buffers
 * used for queue thresholds.
 * Alpha = 1 enables a single congested port to get 50% of free packet
 * buffers. Higher Alphas will give more weight for a single queue allowing
 * better utilization of the packet memory.
 * Amount of buffers for dynamic allocation per port calculated as:
 * PortAlpha
 * DynamicFactor = FreeBuffers -------------------------------------
 * 1 + SumOfAlphaOfAllDestinationPorts
*/
typedef enum{

    /** ratio is 0 */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,

    /** @brief ratio is 0.125
     *  (APPLICABLE DEVICES: Bobcat3.)
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E,

    /** ratio is 0.25 */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E,

    /** ratio is 0.5 */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E,

    /** ratio is 1 */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E,

    /** ratio is 2 */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E,

    /** ratio is 4 */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E,

    /** @brief ratio is 8
     *  (APPLICABLE DEVICES: Bobcat3.)
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E,

    /** @brief ratio is 0.03125
         *  (APPLICABLE DEVICES:  Falcon)
         */
   CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E,


    /** @brief ratio is 0.0625
         *  (APPLICABLE DEVICES:  Falcon)
         */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E,


        /** @brief ratio is 16
         *  (APPLICABLE DEVICES:  Falcon)
         */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E,
        /** @brief ratio is 32
         *  (APPLICABLE DEVICES: Falcon)
         */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E

} CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT;

/**
* @struct CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS
*
* @brief The Tail Drop profile Traffic Class parameters
*/
typedef struct{

    /** @brief the number of buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for GREEN packets
     *  For Bobcat2 if shared counting enabled - unicast and multicast
     *  buffers are counted in same counter.
     *  For Bobcat3, when DBA is enabled and it is used for profile
     *  used by the DBA feature, this is DP0 guarantee value.
     */
    GT_U32 dp0MaxBuffNum;

    /** @brief the number of buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for YELLOW packets
     *  For Bobcat2 if shared counting enabled - unicast and multicast
     *  buffers are counted in same counter.
     *  For Bobcat3, when DBA is enabled and it is used for profile
     *  used by the DBA feature, this is DP1 guarantee value.
     *  For xCat3, this field is invalid and instead dp2MaxBuffNum is used for
     *  YELLOW packets.
     */
    GT_U32 dp1MaxBuffNum;

    /** @brief the number of buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for RED packets
     *  For Bobcat2 if shared counting enabled - unicast and multicast
     *  buffers are counted in same counter.
     *  For Bobcat3, when DBA is enabled and it is used for profile
     *  used by the DBA feature, this is DP2 guarantee value.
     *  For xCat3, YELLOW and RED packets share the same buffer limits and this
     *  field is used even for YELLOW packets.
     */
    GT_U32 dp2MaxBuffNum;

    /** @brief the number of descriptors that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile. for GREEN packets
     */
    GT_U32 dp0MaxDescrNum;

    /** @brief the number of descriptors that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile. for Yellow packets
     *  For xCat3, this field is invalid and instead dp2MaxDescrNum is used for
     *  YELLOW packets.
     */
    GT_U32 dp1MaxDescrNum;

    /** @brief the number of descriptors that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile. for RED packets
     *  For xCat3, YELLOW and RED packets share the same descriptor limits and this
     *  field is used even for YELLOW packets.
     */
    GT_U32 dp2MaxDescrNum;

    /** @brief the number of buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tcMaxBuffNum;

    /** @brief the number of descriptors that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile. (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tcMaxDescrNum;

    /** @brief the number of multicast buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for GREEN packets
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     */
    GT_U32 dp0MaxMCBuffNum;

    /** @brief the number of multicast buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for YELLOW packets
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     */
    GT_U32 dp1MaxMCBuffNum;

    /** @brief the number of multicast buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for RED packets
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     */
    GT_U32 dp2MaxMCBuffNum;

    /** @brief disable unicast and multicast traffic counting in the same buffer counters.
     *  GT_TRUE - separate unicast and multicast counting
     *  GT_FALSE - shared unicast and multicast counting
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     */
    GT_BOOL sharedUcAndMcCountersDisable;

    /** @brief the ratio of the free buffers that can be used for the queue
     *  threshold. For GREEN packets
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp0QueueAlpha;

    /** @brief the ratio of the free buffers that can be used for the queue
     *  threshold. For YELLOW packets
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp1QueueAlpha;

    /** @brief the ratio of the free buffers that can be used for the queue
     *  threshold. For RED packets
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp2QueueAlpha;
} CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS;

/**
* @enum CPSS_PORT_TX_WRR_MODE_ENT
 *
 * @brief Enumeration of Transmit Scheduler mode
*/
typedef enum{

    /** byte mode */
    CPSS_PORT_TX_WRR_BYTE_MODE_E,

    /** packet mode */
    CPSS_PORT_TX_WRR_PACKET_MODE_E

} CPSS_PORT_TX_WRR_MODE_ENT;


/**
* @enum CPSS_PORT_TX_WRR_MTU_ENT
 *
 * @brief Enumeration of MTU for DSWRR, resolution of WRR weights for byte
 * based Transmit Scheduler count mode
*/
typedef enum{

    /** Resolution is 256 bytes */
    CPSS_PORT_TX_WRR_MTU_256_E,

    /** Resolution is 2K bytes */
    CPSS_PORT_TX_WRR_MTU_2K_E,

    /** Resolution is 8K bytes */
    CPSS_PORT_TX_WRR_MTU_8K_E,

    /** Resolution is 64 bytes */
    CPSS_PORT_TX_WRR_MTU_64_E,

    /** Resolution is 128 bytes */
    CPSS_PORT_TX_WRR_MTU_128_E,

    /** Resolution is 512 bytes */
    CPSS_PORT_TX_WRR_MTU_512_E,

    /** Resolution is 1K bytes */
    CPSS_PORT_TX_WRR_MTU_1K_E,

    /** Resolution is 4K bytes */
    CPSS_PORT_TX_WRR_MTU_4K_E,

    /** Resolution is 16K bytes */
    CPSS_PORT_TX_WRR_MTU_16K_E,

    /** Resolution is 32K bytes */
    CPSS_PORT_TX_WRR_MTU_32K_E,

    /** Resolution is 64K bytes */
    CPSS_PORT_TX_WRR_MTU_64K_E

} CPSS_PORT_TX_WRR_MTU_ENT;

/**
* @enum CPSS_PORT_TX_Q_ARB_GROUP_ENT
 *
 * @brief The Traffic Class Queue Arbritration Group
*/
typedef enum{

    /** @brief Weighted Round Robin Arbitration
     *  Group 0
     */
    CPSS_PORT_TX_WRR_ARB_GROUP_0_E = 0,

    /** @brief Weighted Round Robin Arbitration
     *  Group 1
     */
    CPSS_PORT_TX_WRR_ARB_GROUP_1_E,

    /** Strict Priority Arbitration Group */
    CPSS_PORT_TX_SP_ARB_GROUP_E

} CPSS_PORT_TX_Q_ARB_GROUP_ENT;


/**
* @enum CPSS_PORT_TX_Q_DROP_MODE_ENT
 *
 * @brief The queue drop algorithm mode on a Logical Port
*/
typedef enum{

    /** @brief Enqueue packets using General RED
     *  (WRED with 3 levels drop precedence)
     *  for alltraffic queues on this logical
     *  port
     */
    CPSS_PORT_TX_GRED_DROP_MODE_E = 0,

    /** @brief Enqueue packets using 3-level Tail
     *  Drop(according to drop precedence) for
     *  all traffic queues on this logical
     *  port
     */
    CPSS_PORT_TX_TAIL_DROP_MODE_E

} CPSS_PORT_TX_Q_DROP_MODE_ENT;


/**
* @struct CPSS_PORT_TX_Q_DROP_PROFILE_PARAMS_STC
 *
 * @brief The profile parameters used by 3-level WRED (GRED) or
 * 3-level Tail Drop mechanism for a
 * Logical Port Traffic Class Queue
*/
typedef struct{

    /** @brief The Average_Queue_Length minimal threshold
     *  for GREEN packets in units of 256 bytes
     */
    GT_U16 dp0MinThreshold;

    /** @brief The Average_Queue_Length minimal threshold
     *  for YELLOW packets in units of 256 bytes
     */
    GT_U16 dp1MinThreshold;

    /** @brief The Average_Queue_Length minimal threshold
     *  for RED packets in units of 256 bytes
     */
    GT_U16 dp2MinThreshold;

    /** @brief The Average_Queue_Length maximum threshold
     *  for GREEN packets in units of 256 bytes
     */
    GT_U16 dp0MaxThreshold;

    /** @brief The Average_Queue_Length maximum threshold
     *  for Yellow packets in units of 256 bytes
     */
    GT_U16 dp1MaxThreshold;

    /** @brief The Average_Queue_Length maximum threshold
     *  for RED packets in units of 256 bytes
     */
    GT_U16 dp2MaxThreshold;

    /** @brief The GREEN traffic mark probability exponential factor.
     *  The mark probability is 1/(2DP1_Max_Prob). Range 1-15.
     */
    GT_U8 dp0MaxProb;

    /** @brief The Yellow traffic mark probability exponential factor.
     *  The mark probability is 1/(2DP1_Max_Prob). Range 1-15.
     */
    GT_U8 dp1MaxProb;

    /** @brief The RED traffic mark probability exponential factor.
     *  The mark probability is 1/(2DP1_Max_Prob).Range 1-15.
     */
    GT_U8 dp2MaxProb;

    /** @brief The average queue length weight factor. Range 1
     *  Recommended value: 9
     *  Comments:
     *  When operating in 3-level Tail Drop mode (not GRED), only the
     *  xxxMaxThreshold parameters are relevant.
     *  When operation in GRED mode, all the parameters are relevant.
     */
    GT_U8 RedQWeightFactor;

} CPSS_PORT_TX_Q_DROP_PROFILE_PARAMS_STC;


/**
* @enum CPSS_PORT_TX_DROP_PROFILE_SET_ENT
 *
 * @brief There are Drop Profile sets of parameters.
 * This enumeration specifies which Drop Profile set
 * a given Logical Port is associated with.
*/
typedef enum{

    /** @brief Drop Profile Set #1
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_1_E = 0,

    /** @brief Drop Profile Set #2
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_2_E,

    /** @brief Drop Profile Set #3
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_3_E,

    /** @brief Drop Profile Set #4
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_4_E,

    /** @brief Drop Profile Set #5
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_5_E,

    /** @brief Drop Profile Set #6
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_6_E,

    /** @brief Drop Profile Set #7
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_7_E,

    /** @brief Drop Profile Set #8
     *  (APPLICABLE DEVICES: Puma2; Puma3; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_8_E,

    /** @brief Drop Profile Set #9
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_9_E,

    /** @brief Drop Profile Set #10
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_10_E,

    /** @brief Drop Profile Set #11
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_11_E,

    /** @brief Drop Profile Set #12
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_12_E,

    /** @brief Drop Profile Set #13
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_13_E,

    /** @brief Drop Profile Set #14
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_14_E,

    /** @brief Drop Profile Set #15
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_15_E,

    /** @brief Drop Profile Set #16
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_PORT_TX_DROP_PROFILE_16_E

} CPSS_PORT_TX_DROP_PROFILE_SET_ENT;

/**
* @enum CPSS_PORT_TX_DROP_SHAPER_MODE_ENT
 *
 * @brief Token Bucket Shaper mode
*/
typedef enum{

    /** @brief shaper rate is based on the packet's
     *  Byte Count
     */
    CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E,

    /** @brief shaper rate is based on the number of
     *  packets
     */
    CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E

} CPSS_PORT_TX_DROP_SHAPER_MODE_ENT;

/**
* @enum CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT
 *
 * @brief Multicast priority
*/
typedef enum{

    /** @brief  Low priority multicast
     */
    CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E,

    /** @brief Hi  priority multicast
     */
    CPSS_PORT_TX_DROP_MCAST_PRIORITY_HI_E

} CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT;


/**
* @enum CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT
 *
 * @brief Defines which free buffers resource is used to calculate the
 *            dynamic buffer limit for the Port/Queue limits
*/
typedef enum{

    /** @brief  Global resource mode
     */
    CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E,
    /** @brief  Pool  resource mode
     */
    CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E
} CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT;

/**
* @enum CPSS_PORT_TX_TAIL_DROP_PROBABILITY_ENT
 *
 * @brief Define probability to drop packets in WRED for Queue
 *   length reaching the maximal threshold value
*/
typedef enum{
    /** @brief  Drop probability 100 percent upon reaching threshold
     */
    CPSS_PORT_TX_TAIL_DROP_PROBABILITY_100_E,
 /** @brief  Drop probability 89 percent upon reaching threshold
     */
    CPSS_PORT_TX_TAIL_DROP_PROBABILITY_89_E,
        /** @brief  Drop probability 73 percent upon reaching threshold
     */
    CPSS_PORT_TX_TAIL_DROP_PROBABILITY_73_E,
        /** @brief  Drop probability 67 percent upon reaching threshold
     */
    CPSS_PORT_TX_TAIL_DROP_PROBABILITY_67_E,
        /** @brief  Drop probability 50 percent upon reaching threshold
     */
    CPSS_PORT_TX_TAIL_DROP_PROBABILITY_50_E,
        /** @brief  Drop probability 25 percent upon reaching threshold
     */
    CPSS_PORT_TX_TAIL_DROP_PROBABILITY_25_E
} CPSS_PORT_TX_TAIL_DROP_PROBABILITY_ENT;

/**
* @struct CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC
 *
 * @brief Define WRED parameters per table entry
*/

typedef struct{
   /** @brief  The guaranteed amount of buffers
    */
   GT_U32                                guaranteedLimit;
   /** @brief  The size of the WRED early congestion detection window
    */
   GT_U32                                 wredSize;
   /** @brief  Define probability to drop packets in WRED for length reaching the maximal threshold value
    */
   CPSS_PORT_TX_TAIL_DROP_PROBABILITY_ENT probability;
} CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC;


/* @struct CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC
 *
 * @brief Define WRED parameters per traffic class  entry
*/
typedef struct{
  /** @brief drop precedence 0 WRED attributes
    */
   CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC dp0WredAttributes;
  /** @brief drop precedence 1 WRED attributes
    */
   CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC dp1WredAttributes;
  /** @brief drop precedence 2 WRED attributes
    */
   CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC dp2WredAttributes;
} CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPortTxh */


