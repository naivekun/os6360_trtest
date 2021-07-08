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
* @file cpssDxChCatchUpDbg.c
*
* @brief CPSS DxCh CatchUp debug utilities
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUpDbg.h>

/**
* @internal internal_cpssDxChCatchUpValidityCheckEnable function
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
static GT_STATUS internal_cpssDxChCatchUpValidityCheckEnable
(
    IN  GT_U8   devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    return prvCpssDxChSystemRecoveryCatchUpValidityCheckEnable(devNum);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCatchUpValidityCheckEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChCatchUpValidityCheckEnable(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCatchUpValidityCheck function
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
static GT_STATUS internal_cpssDxChCatchUpValidityCheck
(
    IN  GT_U8   devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    return prvCpssDxChSystemRecoveryCatchUpValidityCheck(devNum);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCatchUpValidityCheck);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChCatchUpValidityCheck(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


