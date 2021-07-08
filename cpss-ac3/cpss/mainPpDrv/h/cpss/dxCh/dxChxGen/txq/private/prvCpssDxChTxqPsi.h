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
* @file prvCpssDxChTxqPsi.h
*
* @brief CPSS SIP6 TXQ PSI low level configurations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqPsi
#define __prvCpssDxChTxqPsi

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssFalconPsiMapPortToPdqQueuesSet function
* @endinternal
*
* @brief   Map port to pdq queues
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] psiNum                   -   current PSI id(APPLICABLE RANGES:0..3).
* @param[in] pNodeIndex             -        -index of Pnode representing the local port
* @param[in] firstQueueInDp           - the first pdq queue of DP
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS prvCpssFalconPsiMapPortToPdqQueuesSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 pNodeIndex,
    IN  GT_U32 firstQueueInDp
);
/**
* @internal prvCpssFalconPsiMapPortToPdqQueuesGet function
* @endinternal
*
* @brief   Read port mapping to pdq queues
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] psiNum                   -   current PSI id(APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -  SDQ that being mapped number (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             -        -local port number (APPLICABLE RANGES:0..8).
*
* @param[out] pdqLowQueueNumPtr        - the first pdq queue
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/

GT_STATUS prvCpssFalconPsiMapPortToPdqQueuesGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 psiNum,
    IN   GT_U32 sdqNum,
    IN   GT_U32 localPortNum,
    OUT  GT_U32 * pdqLowQueueNumPtr
);

/**
* @internal prvCpssFalconPsiCreditValueSet function
* @endinternal
*
* @brief   Set credit value that is being given to SDQ from PDQ
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] psiNum                   -   current PSI id(APPLICABLE RANGES:0..3).
* @param[in] creditValue              -  credit value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/

GT_STATUS  prvCpssFalconPsiCreditValueSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 creditValue
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPsi */

