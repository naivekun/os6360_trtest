/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChTmGlueDrop.h
*
* @brief Traffic Manager Glue - TM drop.
*
* @version   3
********************************************************************************
*/

#ifndef __cpssDxChTmGlueDroph
#define __cpssDxChTmGlueDroph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ENT
 *
 * @brief This mode defines whether to drop all RED packets or
 * to drop RED packets according to the Traffic Manager recommendation.
*/
typedef enum{

    /** Drop all RED packets. */
    CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_DROP_ALL_RED_E,

    /** Drop RED packets according to TM response */
    CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ASK_TM_RESP_E

} CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ENT;

/**
* @struct CPSS_DXCH_TM_GLUE_DROP_MASK_STC
 *
 * @brief Configuration for TM drop recommendation.
*/
typedef struct{

    /** @brief indicates whether to use the Q tail drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL qTailDropUnmask;

    /** @brief indicates whether to use the QWRED drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL qWredDropUnmask;

    /** @brief indicates whether to use the A node tail
     *  drop recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL aTailDropUnmask;

    /** @brief indicates whether to use the A node WRED drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL aWredDropUnmask;

    /** @brief indicates whether to use the B node tail drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL bTailDropUnmask;

    /** @brief indicates whether to use a B node WRED drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL bWredDropUnmask;

    /** @brief indicates whether to use a C node tail drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL cTailDropUnmask;

    /** @brief indicates whether to use a C node WRED drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL cWredDropUnmask;

    /** @brief indicates whether to use the Port tail drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL portTailDropUnmask;

    /** @brief indicates whether to use the Port WRED drop
     *  recommendation for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL portWredDropUnmask;

    /** @brief indicates whether to use the OOR
     *  (Out of resources) for the dropping decision.
     *  GT_FALSE = Mask. Do not consider for Drop.
     *  GT_TRUE = UnMask. Consider for Drop.
     */
    GT_BOOL outOfResourceDropUnmask;

    /** @brief defines whether to drop all RED packets
     *  or to drop RED packets according to the TM recommendation.
     *  COMMENTS:
     */
    CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ENT redPacketsDropMode;

} CPSS_DXCH_TM_GLUE_DROP_MASK_STC;


/**
* @internal cpssDxChTmGlueDropQueueProfileIdSet function
* @endinternal
*
* @brief   The function maps every Queue Id to its Profile Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - TM queue id
*                                      (APPLICABLE RANGES: 0..16383).
* @param[in] profileId                - per queue profile Id
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on wrong profile id
*/
GT_STATUS cpssDxChTmGlueDropQueueProfileIdSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    queueId,
    IN  GT_U32    profileId
);

/**
* @internal cpssDxChTmGlueDropQueueProfileIdGet function
* @endinternal
*
* @brief   The function gets the Profile Id of a given QueueId.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - TM queue id
*                                      (APPLICABLE RANGES: 0..16383).
*
* @param[out] profileIdPtr             - (pointer to) profile Id per Queue
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDropQueueProfileIdGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    queueId,
    OUT GT_U32   *profileIdPtr
);

/**
* @internal cpssDxChTmGlueDropProfileDropMaskSet function
* @endinternal
*
* @brief   The function configures the Drop Masking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - configuration per profile Id
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tc                       - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
* @param[in] dropMaskCfgPtr           - (pointer to) drop mask configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDropProfileDropMaskSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   profileId,
    IN  GT_U32                                   tc,
    IN  CPSS_DXCH_TM_GLUE_DROP_MASK_STC         *dropMaskCfgPtr
);

/**
* @internal cpssDxChTmGlueDropProfileDropMaskGet function
* @endinternal
*
* @brief   The function gets the Drop Mask configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] profileId                - configuration per profile Id
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tc                       - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
*
* @param[out] dropMaskCfgPtr           - (pointer to) structure for drop mask configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDropProfileDropMaskGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   profileId,
    IN  GT_U32                                   tc,
    OUT CPSS_DXCH_TM_GLUE_DROP_MASK_STC         *dropMaskCfgPtr
);


/**
* @internal cpssDxChTmGlueDropTcToCosSet function
* @endinternal
*
* @brief   The function configures TM TC to COS mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmTc                     - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
* @param[in] cos                      - mapped COS value for TM-TC
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on wrong cos
*/
GT_STATUS cpssDxChTmGlueDropTcToCosSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmTc,
    IN  GT_U32    cos
);

/**
* @internal cpssDxChTmGlueDropTcToCosGet function
* @endinternal
*
* @brief   The function retrieves the TM TC to COS mapping COS value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tmTc                     - TM traffic class
*                                      (APPLICABLE RANGES: 0..15).
*
* @param[out] cosPtr                   - (pointer to) mapped COS value for TM-TC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTmGlueDropTcToCosGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmTc,
    OUT GT_U32   *cosPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTmGlueDroph */



