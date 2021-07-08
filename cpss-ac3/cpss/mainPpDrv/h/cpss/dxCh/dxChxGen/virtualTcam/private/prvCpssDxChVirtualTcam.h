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
* @file prvCpssDxChVirtualTcam.h
*
* @brief The CPSS DXCH High Level Virtual TCAM Manager private functions
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChVirtualTcam_h
#define __prvCpssDxChVirtualTcam_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

/**
* @internal prvCpssDxChVirtualTcamHsuSizeGet function
* @endinternal
*
* @brief   Gets the memory size needed for Virtual TCAM feature internal DB HSU export information.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] sizePtr                  - points to memory size needed for export data,
*                                      including HSU data block header (calculated in bytes)
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamHsuSizeGet
(
    OUT  GT_U32      *sizePtr
);

/**
* @internal prvCpssDxChVirtualTcamHsuExport function
* @endinternal
*
* @brief   Exports Virtual TCAM feature internal DB HSU data.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the Virtual TCAM feature internal DB
*                                      to track the export progress.
*                                      At first iteration set (iteratorPtr) = 0
*                                      At next iterations supply value returned by
*                                      previous iteration.
*                                      Relevant only when single operation export is not used
* @param[in,out] hsuBlockMemSizePtr       != CPSS_HSU_SINGLE_ITERATION_CNS)
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block supplied
*                                      for the current iteration (in bytes);
*                                      - Size must be at least 1k bytes.
*                                      - Use the value CPSS_HSU_SINGLE_ITERATION_CNS value
*                                      to indicate single iteration export. In such case
*                                      it is assumed that hsuBlockMemPtr is large enough
*                                      for the entire HSU data.
* @param[in] hsuBlockMemPtr           - points to memory block to hold HSU data in
*                                      current iteration
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the Virtual TCAM feature internal DB
*                                      to track the export progress.
*                                      Relevant only if exportCompletePtr == GT_FALSE
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block that was
*                                      not used in the current iteration (in bytes)
*                                      Relevant only when single iteration export is not used
*                                      (input valus was not CPSS_HSU_SINGLE_ITERATION_CNS)
*
* @param[out] exportCompletePtr        - GT_TRUE:  HSU data export is completed.
*                                      GT_FALSE: HSU data export is not completed.
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*
* @note 1. Several export processes can be performed simultaneously.
*       2. It is assumed that in case of multiple iterations export operation,
*       there are no calls to other Virtual TCAM feature internal DB APIs
*       until export operation is completed. Such calls may modify the data
*       stored within the Virtual TCAM feature internal DB and
*       cause unexpected behavior.
*
*/
GT_STATUS prvCpssDxChVirtualTcamHsuExport
(
    INOUT GT_UINTPTR    *iteratorPtr,
    INOUT GT_U32        *hsuBlockMemSizePtr,
    IN    GT_U8         *hsuBlockMemPtr,
    OUT   GT_BOOL       *exportCompletePtr
);

/**
* @internal prvCpssDxChVirtualTcamHsuImport function
* @endinternal
*
* @brief   Imports Virtual TCAM feature internal DB HSU data.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the Virtual TCAM feature internal DB
*                                      to track the import progress.
*                                      At first iteration set (iteratorPtr) = 0
*                                      At next iterations supply value returned by
*                                      previous iteration.
*                                      Relevant only when single operation import is not used
* @param[in,out] hsuBlockMemSizePtr       != CPSS_HSU_SINGLE_ITERATION_CNS)
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block supplied
*                                      for the current iteration (in bytes);
*                                      - Size must be at least 1k bytes.
*                                      - Use the value CPSS_HSU_SINGLE_ITERATION_CNS value
*                                      to indicate single iteration import. In such case
*                                      it is assumed that hsuBlockMemPtr contains all HSU data.
* @param[in] hsuBlockMemPtr           - points to memory block that holds HSU data for
*                                      current iteration
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the Virtual TCAM feature internal DB
*                                      to track the import progress.
*                                      Relevant only if importCompletePtr == GT_FALSE
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block that was
*                                      not used in the current iteration (in bytes)
*                                      Relevant only when single iteration import is not used
*                                      (input valus was not CPSS_HSU_SINGLE_ITERATION_CNS)
*
* @param[out] importCompletePtr        - GT_TRUE:  HSU data import is completed.
*                                      GT_FALSE: HSU data import is not completed.
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*
* @note 1. The import function will override all the Virtual TCAM feature internal DB
*       data and set it to the data of the Virtual TCAM feature internal DB that
*       performed the export at the time of the export. All changes made between
*       the export and the import will be discarded.
*       2. It is assumed that in case of multiple iterations import operation,
*       there are no calls to other Virtual TCAM APIs until import is
*       completed. Such calls may modify the data stored within the
*       Virtual TCAM feature internal DB and cause unexpected behavior.
*       3. It is assumed that no different import processes are performed
*       simultaneously.
*
*/
GT_STATUS prvCpssDxChVirtualTcamHsuImport
(
    INOUT GT_UINTPTR    *iteratorPtr,
    INOUT GT_U32        *hsuBlockMemSizePtr,
    IN    GT_U8         *hsuBlockMemPtr,
    OUT   GT_BOOL       *importCompletePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChVirtualTcam_h */


