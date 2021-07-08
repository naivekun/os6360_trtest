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
* @file prvCpssDxChTxqPds.h
*
* @brief CPSS SIP6 TXQ PDS low level configurations.
*
* @version   1
********************************************************************************
*/


#ifndef __prvCpssDxChTxqPds
#define __prvCpssDxChTxqPds

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>


/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC
 *
 * @brief Descriptor counter at PDS per queue.
 *  Total_queue_counter = Tail Counter + Head counter + (Frag Counter*16) - (15-FIdx);
*/
typedef struct
{
        /** @brief Tail counter indicates the tail size (in descriptor resolution) of a long queue.
        *This is relevant only when the queue is long
     */

    GT_U32     tailCounter;

    /** @brief Frag counter indicates the fragment size of long queue (in frgaments resolution).
    *This is relevant only when the queue is long
    */

    GT_U32     fragCounter;

     /** @brief Head counter indicates the head size (in descriptor resolution) of a long queue.
     *   When the queue is short, it indicates the total queue length
    */

    GT_U32     headCounter;

      /** @brief Fragment Index. Indicating the descriptor index within the 128B PB cell in case of noDeAlloc read from the PB.
     *This is relevant only when the queue is long
    */
    GT_U32     fragIndex;

     /** @brief  Queue is Long
      * GT_FALSE = short; short; queue is short
      * GT_TRUE  = long; long; queue is long
     */
    GT_BOOL longQueue;
} PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC;


/**
* @internal prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet function
* @endinternal
*
* @brief   Counts the number of descriptors of all queues in both the cache and the PB
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -The number of tile (Applicable range 0..3)
* @param[in] pdsNum                   -the local number of PDS (APPLICABLE RANGES:0..7).
*
* @param[out] pdsDescCountPtr             - the number of descriptors of all queues in both the cache and the PB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT  GT_U32 * pdsDescCountPtr
);

/**
* @internal prvCpssDxChTxqFalconPdsCacheDescCounterGet function
* @endinternal
*
* @brief   Counts the number of descriptors of all queues in the cache (not including the PB)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -The number of tile (Applicable range 0..3)
* @param[in] pdsNum                   -the local number of PDS (APPLICABLE RANGES:0..7).
*
* @param[out] pdsDescCountPtr             - the number of descriptors of all queues in the cache (not including the PB)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxqFalconPdsCacheDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT  GT_U32 * pdsDescCountPtr
);

/**
* @internal prvCpssDxChTxqFalconPdsPerQueueCountersGet function
* @endinternal
*
* @brief   Counts the number of descriptors per queues in the cache and  the PB
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -The number of tile (Applicable range 0..3)
* @param[in] pdsNum                   -the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber                   -the local queue number (APPLICABLE RANGES:0..399).
*
* @param[out] perQueueDescCountPtr             - the number of descriptors at the queue
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsPerQueueCountersGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC * perQueueDescCountPtr
);

/**
* @internal prvCpssDxChTxqFalconPdsProfileSet function
* @endinternal
*
* @brief   Profile :
*         Profile_%p_Long_Queue_Enable - Enables writing a queue to the PB when it is getting long
*         Profile_%p_Long_Queue_Limit [0..8K] - When the total queue counter reaches this limit, a long queue is opened
*         Profile_%p_Head_Empty_Limit [0..8K] - When the head counter is below this limit, the head is considered “empty” and fragment read from PB is triggered
*         Length_Adjust_Constant_byte_count_Profile_%p_ [0..64] -
*         Sets the value which will be decremented or incremented from the packet's byte count
*         Length_Adjust_Sub_Profile_%p_ - Subtract enable bit - for profile <%p>
*         0x0 = ADD; ADD; When ADD, the value of constant byte count field is added to the descriptor byte count
*         0x1 = SUB; SUB; When SUB, the value of constant byte count field is subtracted from the descriptor byte count
*         Length_Adjust_Enable_profile_%p -
*         RW 0x0
*         Enable the length adjust
*         0x0 = Disable Length Adjust; Disable_Length_Adjust
*         0x1 = Enable Length Adjust; Enable_Length_Adjust
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -The number of tile (Applicable range 0..3)
* @param[in] pdsNum                   -the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] profile                  - index of the profile(APPLICABLE RANGES:0..15.)
* @param[in] profile                  - pds profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxqFalconPdsProfileSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 profileIndex,
    IN CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC profile
);

/**
* @internal prvCpssDxChTxqFalconPdsProfileGet function
* @endinternal
*
* @brief   Profile :
*         Profile_%p_Long_Queue_Enable - Enables writing a queue to the PB when it is getting long
*         Profile_%p_Long_Queue_Limit [0..8K] - When the total queue counter reaches this limit, a long queue is opened
*         Profile_%p_Head_Empty_Limit [0..8K] - When the head counter is below this limit, the head is considered “empty” and fragment read from PB is triggered
*         Length_Adjust_Constant_byte_count_Profile_%p_ [0..64] -
*         Sets the value which will be decremented or incremented from the packet's byte count
*         Length_Adjust_Sub_Profile_%p_ - Subtract enable bit - for profile <%p>
*         0x0 = ADD; ADD; When ADD, the value of constant byte count field is added to the descriptor byte count
*         0x1 = SUB; SUB; When SUB, the value of constant byte count field is subtracted from the descriptor byte count
*         Length_Adjust_Enable_profile_%p -
*         RW 0x0
*         Enable the length adjust
*         0x0 = Disable Length Adjust; Disable_Length_Adjust
*         0x1 = Enable Length Adjust; Enable_Length_Adjust
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -The number of tile (Applicable range 0..3)
* @param[in] pdsNum                   -the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] profileIndex                  - index of the profile(APPLICABLE RANGES:0..15.)
* @param[out] profilePtr                  - (pointer to)pds profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxqFalconPdsProfileGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 profileIndex,
    OUT  CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC * profilePtr
);
/**
* @internal prvCpssDxChTxqFalconPdsQueueProfileMapSet function
* @endinternal
*
* @brief   Set queue profile to PDS queue.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -The number of tile (Applicable range 0..3)
* @param[in] pdsNum                   -the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - the number of the queue(APPLICABLE RANGES:0..399)
* @param[in] profileIndex             - index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxqFalconPdsQueueProfileMapSet
    (
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN GT_U32 profileIndex
    );

/**
* @internal prvCpssDxChTxqFalconPdsQueueProfileMapGet function
* @endinternal
*
* @brief   Get queue profile that binded to  PDS queue.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -The number of tile (Applicable range 0..3)
* @param[in] pdsNum                   -the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - the number of the queue(APPLICABLE RANGES:0..399)
* @param[in] profileIndexPtr             - (pointer to) index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueProfileMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    OUT  GT_U32 * profileIndexPtr
);

/**
* @internal prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet function
* @endinternal
*
* @brief   Set limit on PDS capacity .When this limit is reached, stop accepting descriptors into the PDS.
*         The motivation is to reduce risk of PDS getting FULL
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] tileNum                  -The number of tile (Applicable range 0..3)
* @param[in] pdsNum                   the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] pdsSizeForPdx                   limit on PDS capacity for PDX
* @param[in] pdsSizeForPb                  - limit on PDS capacity for PB
* @param[in] pbFullLimit                  -       limit on PB for PDS(used for long queues mechanism
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 pdsSizeForPdx,
    IN GT_U32 pdsSizeForPb,
    IN GT_U32 pbFullLimit
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPds */

