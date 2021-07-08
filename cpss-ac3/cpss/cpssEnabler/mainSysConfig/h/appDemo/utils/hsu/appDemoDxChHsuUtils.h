/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDxChHsuUtils.h
*
* @brief App demo DxCh appdemo HSU API header.
*
* @version   3
********************************************************************************
*/

#ifndef __appDemoDxChHsuUtilsh
#define __appDemoDxChHsuUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

/**
* @internal appDemoDxChHsuOldImagePreUpdatePreparation function
* @endinternal
*
* @brief   This function perform following steps of HSU process:
*         1. Disable interrupts
*         2. Set systemRecoveryMode
*         3. export of all hsu datatypes
*         4. warm restart
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] isMultipleIteration      - GT_TRUE - multiIteration HSU operation
*                                      GT_FALSE - single iteration HSU operation
* @param[in] origIterationSize        - hsu iteration size
* @param[in] systemRecoveryMode       - hsu mode defines if application want to process
*                                      messages arrived during HSU
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_RESOURCE           - no place to save the request
*/
GT_STATUS appDemoDxChHsuOldImagePreUpdatePreparation
(
  IN GT_BOOL                               isMultipleIteration,
  IN GT_U32                                origIterationSize,
  IN CPSS_SYSTEM_RECOVERY_MODE_STC         systemRecoveryMode
);

/**
* @internal appDemoDxChHsuNewImageUpdateAndSynch function
* @endinternal
*
* @brief   This function perform following steps of HSU process:
*         1. import of all hsu datatypes
*         2. setting HSU state in "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E"
*         4. open pp interrupts
*         3. enable interrupts
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] isMultipleIteration      - GT_TRUE - multiIteration HSU operation
*                                      GT_FALSE - single iteration HSU operation
* @param[in] origIterationSize        - hsu iteration size
* @param[in] hsuDataSize              - hsu import data size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_RESOURCE           - no place to save the request
*
* @note When appDemoDxChHsuOldImagePreUpdatePreparation is finished and new image
*       have arisen after warm restart, user should perform
*       cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_INIT_STATE_E), cpssInitSystem and only
*       after that call appDemoDxChHsuNewImageUpdateAndSynch.
*
*/
GT_STATUS appDemoDxChHsuNewImageUpdateAndSynch
(
  IN GT_BOOL                               isMultipleIteration,
  IN GT_U32                                origIterationSize,
  IN GT_U32                                hsuDataSize
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoDxChHsuUtilsh */



