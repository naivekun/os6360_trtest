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
* @file cpssTmNodesStatus.h
*
* @brief TM nodes status reading APIs
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmNodesStatush
#define __cpssTmNodesStatush

#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal cpssTmQueueStatusRead function
* @endinternal
*
* @brief   Read Queue status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmQueueStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
);


/**
* @internal cpssTmAnodeStatusRead function
* @endinternal
*
* @brief   Read A-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmAnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
);


/**
* @internal cpssTmBnodeStatusRead function
* @endinternal
*
* @brief   Read B-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmBnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
);


/**
* @internal cpssTmCnodeStatusRead function
* @endinternal
*
* @brief   Read C-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmCnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
);


/**
* @internal cpssTmPortStatusRead function
* @endinternal
*
* @brief   Read Port status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmPortStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_PORT_STATUS_STC     *statusPtr
);


/**
* @internal cpssTmDropQueueLengthGet function
* @endinternal
*
* @brief   Read Queue drop length.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Node  (Port/C/B/A/Queue).
* @param[in] index                    - Node index.
*
* @param[out] instantQueueLengthPtr    - (pointer to) Current Queue length structure.
* @param[out] averageQueueLengthPtr    - (pointer to) Average Queue length structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDropQueueLengthGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_TM_LEVEL_ENT           level,
    IN  GT_U32                      index,
    OUT GT_U32                      *instantQueueLengthPtr,
    OUT GT_U32                      *averageQueueLengthPtr
);


#endif 	    /* __cpssTmNodesStatush */


