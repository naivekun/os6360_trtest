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
* @file cpssDxChCatchUpDbg.h
*
* @brief CPSS DxCh CatchUp debug utilities
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChCatchUpDbgh
#define __cpssDxChCatchUpDbgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>

/**
* @internal cpssDxChCatchUpValidityCheckEnable function
* @endinternal
*
* @brief   Enable CatchUp validity check
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note This function should be called before catchUp.
*       Correct use of validity check:
*       1. Run cpssDxChCatchUpValidityCheckEnable
*       2. Perform catchUp
*       3. Run cpssDxChCatchUpValidityCheck
*       Pre-condition for using this function:
*       CatchUp parameters in PP data structure must be equal to the HW values
*
*/
GT_STATUS cpssDxChCatchUpValidityCheckEnable
(
    IN  GT_U8   devNum
);

/**
* @internal cpssDxChCatchUpValidityCheck function
* @endinternal
*
* @brief   Validates that SW params hold the correct value from HW after CatchUp
*         process.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note This function should be called after catchUp.
*       Correct use of validity check:
*       1. Run cpssDxChCatchUpValidityCheckEnable
*       2. Perform catchUp
*       3. Run cpssDxChCatchUpValidityCheck
*
*/
GT_STATUS cpssDxChCatchUpValidityCheck
(
    IN  GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChCatchUpDbgh */


