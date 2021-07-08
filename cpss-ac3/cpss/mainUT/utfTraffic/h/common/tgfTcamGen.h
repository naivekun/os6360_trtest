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
* @file tgfTcamGen.h
*
* @brief Generic API for TCAM
*
* @version   2
********************************************************************************
*/
#ifndef __tgfTcamGenh
#define __tgfTcamGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#endif /* CHX_FAMILY */

#include <common/tgfCommon.h>
#include <utf/private/prvUtfExtras.h>

#define  HARD_WIRE_TCAM_MAC(dev) \
    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ? 1 : 0)

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_TCAM_CLIENT_ENT
 *
 * @brief TCAM clients.
*/
typedef enum{

    /** TCAM client ingress policy 0 (first lookup) */
    PRV_TGF_TCAM_IPCL_0_E,

    /** TCAM client ingress policy 1 (second lookup) */
    PRV_TGF_TCAM_IPCL_1_E,

    /** TCAM client ingress policy 2 (third lookup) */
    PRV_TGF_TCAM_IPCL_2_E,

    /** TCAM client egress policy */
    PRV_TGF_TCAM_EPCL_E,

    /** TCAM client tunnel termination */
    PRV_TGF_TCAM_TTI_E

} PRV_TGF_TCAM_CLIENT_ENT;

/**
* @struct PRV_TGF_TCAM_BLOCK_INFO_STC
 *
 * @brief Defines the group id and lookup Number for TCAM block.
 * Block size - 6 banks per block -> 2 blocks per floor
 * APPLICABLE DEVICES: Bobcat2.
 * Block size - 2 banks per block -> 6 blocks per floor
 * APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*/
typedef struct{

    /** TCAM group ID (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4) */
    GT_U32 group;

    /** lookup number (APPLICABLE RANGES: 0..3) */
    GT_U32 hitNum;

} PRV_TGF_TCAM_BLOCK_INFO_STC;

#define PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS    CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS
/**
* @internal prvTgfTcamClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] tcamClient               - TCAM client.
* @param[in] tcamGroup                - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] enable                   - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTcamClientGroupSet
(
    IN  PRV_TGF_TCAM_CLIENT_ENT tcamClient,
    IN  GT_U32                  tcamGroup,
    IN  GT_BOOL                 enable
);

/**
* @internal prvTgfTcamClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] tcamClient               - TCAM client.
*
* @param[out] tcamGroupPtr             - (pointer to) TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[out] enablePtr                - (pointer to) TCAM client status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTcamClientGroupGet
(
    IN  PRV_TGF_TCAM_CLIENT_ENT tcamClient,
    OUT GT_U32                  *tcamGroupPtr,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal prvTgfTcamIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         floorIndex     - floor index (APPLICABLE RANGES: 0..11)
*         floorInfoArr    - array of group ID and lookup number for all TCAM blocks in floor.
*         Index 0 represent banks 0-5 and index 1 represent banks 6-11.
*         APPLICABLE DEVICES: Bobcat2, Bobcat3, Aldrin2, Falcon.
*         Index 0 represent banks 0-1, index 1 represent banks 2-3,
*         index 2 represent banks 4-5, index 3 represent banks 6-7,
*         index 4 represent banks 8-9, index 5 represent banks 10-11.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS] - array of group ID and lookup number for all TCAM blocks in floor.
*                                      Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                      APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon.
*                                      Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                      index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                      index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                      APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U32                           floorIndex,
    IN PRV_TGF_TCAM_BLOCK_INFO_STC      floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS]
);

/**
* @internal prvTgfTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         floorIndex     - floor index (APPLICABLE RANGES: 0..11)
*         OUTPUTS:
*         floorInfoArr    - array of group ID and lookup number for all TCAM blocks in floor.
*         Index 0 represent banks 0-5 and index 1 represent banks 6-11.
*         APPLICABLE DEVICES: Bobcat2, Bobcat3, Aldrin2, Falcon.
*         Index 0 represent banks 0-1, index 1 represent banks 2-3,
*         index 2 represent banks 4-5, index 3 represent banks 6-7,
*         index 4 represent banks 8-9, index 5 represent banks 10-11.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS] - array of group ID and lookup number for all TCAM blocks in floor.
*                                      Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                      APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon.
*                                      Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                      index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                      index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                      APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTcamIndexRangeHitNumAndGroupGet
(
    IN GT_U32                               floorIndex,
    OUT PRV_TGF_TCAM_BLOCK_INFO_STC         floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS]
);

/**
* @internal prvTgfTcamFloorsNumGet function
* @endinternal
*
* @brief   Return number of TCAM floors
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       number of TCAM floors
*/
GT_U32 prvTgfTcamFloorsNumGet
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTcamGenh */

