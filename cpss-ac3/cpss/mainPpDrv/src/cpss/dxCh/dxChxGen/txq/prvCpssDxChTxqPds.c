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
* @file prvCpssDxChTxqPds.c
*
* @brief CPSS SIP6 TXQ PDS low level configurations.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/prvCpssFalconTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPds.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>






/** Synced to Cider /Cider/EBU-IP/TXQ_IP/SIP7.0 (Falcon)/TXQ_SCN_IP/TXQ_SCN_IP {7.0.6}/TXQ_PDS/PDS  */






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
)
{
    GT_U32 regAddr,tileOffset ;
    GT_U32 regValue = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(pdsSizeForPdx>= (1<<TXQ_PDS_MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(pdsSizeForPdx>= (1<<TXQ_PDS_MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(pbFullLimit>= (1<<TXQ_PDS_PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Max_PDS_size_limit_for_pdx;

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_OFFSET,
        TXQ_PDS_MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_SIZE,
        pdsSizeForPdx);


    rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);
    if(rc!=GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Max_PDS_size_limit_for_PB;
    regAddr+=tileOffset;

    regValue =0;

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_OFFSET,
        TXQ_PDS_MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_SIZE,
        pdsSizeForPb);

    rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);
    if(rc!=GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PB_Full_Limit;
    regAddr+=tileOffset;

    if(prvCpssDxchFalconCiderVersionGet()>PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E)
    {
        regValue =0;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_OFFSET,
            TXQ_PDS_PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_SIZE,
            pdsSizeForPb);

        rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);
     }

    return rc;
}

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
* @param[in] profileIndex                  - index of the profile(APPLICABLE RANGES:0..15.)
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
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    if(profileIndex>=PDS_PROFILE_MAX_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(profile.longQueueParameters.headEmptyLimit>=(1<<TXQ_PDS_PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(profile.longQueueParameters.longQueueLimit>=(1<<TXQ_PDS_PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(profile.lengthAdjustParameters.lengthAdjustByteCount>=(1<<TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }



    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_OFFSET,
        TXQ_PDS_PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE,
        profile.longQueueParameters.headEmptyLimit);


    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TXQ_PDS_PROFILE_HEAD_EMPTY_LIMIT_E+pdsNum+ tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
        profileIndex,
        &regValue);

    if(rc!=GT_OK)
    {
        return rc;
    }

    regValue = 0;

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_OFFSET,
        TXQ_PDS_PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE,
        profile.longQueueParameters.longQueueLimit);


    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TXQ_PDS_PROFILE_LONG_QUEUE_LIMIT_E+pdsNum+ tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
        profileIndex,
        &regValue);

    if(rc!=GT_OK)
    {
        return rc;
    }

    regValue = 0;



    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_OFFSET,
        TXQ_PDS_PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_SIZE,
        profile.longQueueParameters.longQueueEnable?1:0);



    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TXQ_PDS_PROFILE_LONG_QUEUE_ENABLE_E+pdsNum+ tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
        profileIndex,
        &regValue);

    if(rc!=GT_OK)
    {
        return rc;
    }

    regValue = 0;



    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_OFFSET,
        TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_SIZE,
        profile.lengthAdjustParameters.lengthAdjustEnable);



    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_OFFSET,
        TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_SIZE,
        profile.lengthAdjustParameters.lengthAdjustSubstruct);




    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_OFFSET,
        TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE,
        profile.lengthAdjustParameters.lengthAdjustByteCount);


    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_E+pdsNum+ tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
        profileIndex,
        &regValue);

    return rc;


}

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
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    if(profileIndex>=PDS_PROFILE_MAX_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_PDS_PROFILE_HEAD_EMPTY_LIMIT_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        profileIndex,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_PDS_PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_OFFSET,
                                       TXQ_PDS_PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE,
                                        &(profilePtr->longQueueParameters.headEmptyLimit));
    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_PDS_PROFILE_LONG_QUEUE_LIMIT_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        profileIndex,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_PDS_PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_OFFSET,
                                       TXQ_PDS_PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE,
                                        &(profilePtr->longQueueParameters.longQueueLimit));
    if(rc!=GT_OK)
    {
        return rc;
    }


     rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_PDS_PROFILE_LONG_QUEUE_ENABLE_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        profileIndex,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                       TXQ_PDS_PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_OFFSET,
                                       TXQ_PDS_PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_SIZE,
                                        &regValue);


    if(rc!=GT_OK)
    {
        return rc;
    }

    profilePtr->longQueueParameters.longQueueEnable = (regValue==1)?GT_TRUE:GT_FALSE;


    rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
                                        profileIndex,
                                        &regValue);

          if(rc == GT_OK)
          {
                     profilePtr->lengthAdjustParameters.lengthAdjustEnable = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                         TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_OFFSET,
                          TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_SIZE);

                     profilePtr->lengthAdjustParameters.lengthAdjustSubstruct= (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                         TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_OFFSET,
                          TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_SIZE);

                     profilePtr->lengthAdjustParameters.lengthAdjustByteCount= (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                         TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_OFFSET,
                         TXQ_PDS_PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE);
          }



    return rc;



}




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
 )
{
    GT_U32 regValue = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(profileIndex>=PDS_PROFILE_MAX_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_OFFSET,
        TXQ_PDS_QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_SIZE,
        profileIndex);


    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TXQ_PDS_QUEUE_PDS_PROFILE_E+pdsNum+ tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
        queueNumber,
        &regValue);
    return rc;

}

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
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);



    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TXQ_PDS_QUEUE_PDS_PROFILE_E+pdsNum+ tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
        queueNumber,
        &regValue);

    if(rc == GT_OK)
     {

        *profileIndexPtr  = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                         TXQ_PDS_QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_OFFSET,
                          TXQ_PDS_QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_SIZE);
    }

    return rc;

}


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
)
{
    GT_U32 regAddr,tileOffset ;
    GT_U32 regValue = 0;
    GT_STATUS rc;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC * regAddrData;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    regAddrData = &(PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1);

    regAddr = regAddrData->TXQ.pds[pdsNum].Total_PDS_Counter;

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc == GT_OK)
     {
        *pdsDescCountPtr = U32_GET_FIELD_MAC(regValue,TXQ_PDS_TOTAL_PDS_COUNTER_TOTAL_PDS_COUNTER_FIELD_OFFSET,
            TXQ_PDS_TOTAL_PDS_COUNTER_TOTAL_PDS_COUNTER_FIELD_SIZE);
     }

    return rc;
}


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
)
{
    GT_U32 regAddr,tileOffset ;
    GT_U32 regValue = 0;
    GT_STATUS rc;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC * regAddrData;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    regAddrData = &(PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1);

    regAddr = regAddrData->TXQ.pds[pdsNum].PDS_Cache_Counter;

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc == GT_OK)
     {
        *pdsDescCountPtr = U32_GET_FIELD_MAC(regValue,TXQ_PDS_PDS_CACHE_COUNTER_PDS_CACHE_COUNTER_FIELD_OFFSET,
            TXQ_PDS_PDS_CACHE_COUNTER_PDS_CACHE_COUNTER_FIELD_SIZE);
     }

    return rc;
}

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
)
{
     GT_STATUS rc = GT_OK;
 #ifndef GM_USED
     GT_U32 regValue[2],size,value;
 #endif
     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
     TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
     CPSS_NULL_PTR_CHECK_MAC(perQueueDescCountPtr);

#ifdef GM_USED
    queueNumber = queueNumber;/*avoid  warning*/
    perQueueDescCountPtr->headCounter =0;
    perQueueDescCountPtr->longQueue=GT_FALSE;
#else
      rc = prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
                                    queueNumber,
                                    regValue);

      if(rc == GT_OK)
      {
                 perQueueDescCountPtr->longQueue = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_PER_QUEUE_COUNTERS_LONG_FIELD_OFFSET,
                      TXQ_PDS_PER_QUEUE_COUNTERS_LONG_FIELD_SIZE);

                 perQueueDescCountPtr->fragIndex = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_PER_QUEUE_COUNTERS_FIDX_FIELD_OFFSET,
                      TXQ_PDS_PER_QUEUE_COUNTERS_FIDX_FIELD_SIZE);

                  perQueueDescCountPtr->headCounter = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_PER_QUEUE_COUNTERS_HEAD_COUNTER_FIELD_OFFSET,
                      TXQ_PDS_PER_QUEUE_COUNTERS_HEAD_COUNTER_FIELD_SIZE);

                   perQueueDescCountPtr->fragCounter = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_PER_QUEUE_COUNTERS_FRAG_COUNTER_FIELD_OFFSET,
                      TXQ_PDS_PER_QUEUE_COUNTERS_FRAG_COUNTER_FIELD_SIZE);

                     size = 32 - TXQ_PDS_PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_OFFSET;

                      perQueueDescCountPtr->tailCounter = 0;

                    perQueueDescCountPtr->tailCounter= U32_GET_FIELD_MAC(regValue[0],
                            TXQ_PDS_PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_OFFSET,
                            size);

                    value =  U32_GET_FIELD_MAC(regValue[1],
                                0,
                             TXQ_PDS_PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_SIZE- size);

                    perQueueDescCountPtr->tailCounter|= ((value)<<size);


      }
#endif
          return rc;
}

