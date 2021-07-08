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
* @file cpssTmNodesTree.h
*
* @brief TM Tree APIs
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmNodesTreeh
#define __cpssTmNodesTreeh

#include <cpss/common/cpssTypes.h>

/**
* @internal cpssTmTreeChangeStatus function
* @endinternal
*
* @brief   Change the tree DeQ status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] status                   - Tree status.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmTreeChangeStatus
(
    IN GT_U8    devNum,
    IN GT_BOOL  status
);



/**
* @internal cpssTmTreeStatusGet function
* @endinternal
*
* @brief   Retrieve tree DeQ status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] statusPtr                -(pointer to)  Tree status.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL statusPtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmTreeStatusGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *statusPtr
);


/**
* @internal cpssTmTreeDwrrPrioSet function
* @endinternal
*
* @brief   Change the tree DWRR priority.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] prios[8]                 - Priority array pointer(pointer to GT_BOOL[8])
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Set prios[i] = GT_FALSE/GT_TRUE, if DWRR for prio [i] is disabled/enabled.
*
*/
GT_STATUS cpssTmTreeDwrrPrioSet
(
    IN GT_U8      devNum,
	IN GT_BOOL	  prios[8]
);

/**
* @internal cpssTmTreeDwrrPrioGet function
* @endinternal
*
* @brief   Retrieve tree DWRR priority.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] prios[8]                 - Status per priority array pointer (pointer to GT_BOOL[8]).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PTR               - on NULL prios.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note prios[i] = GT_FALSE/GT_TRUE, if DWRR for prio [i] is disabled/enabled.
*
*/
GT_STATUS cpssTmTreeDwrrPrioGet
(
    IN  GT_U8    devNum,
	OUT GT_BOOL	 prios[8]
);

#endif 	    /* __cpssTmNodesTreeh */


