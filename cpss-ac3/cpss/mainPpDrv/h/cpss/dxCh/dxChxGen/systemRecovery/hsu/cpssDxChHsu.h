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
* @file cpssDxChHsu.h
*
* @brief CPSS DxCh HSU facility API.
*
* @version   3
********************************************************************************
*/

#ifndef __cpssDxChHsuh
#define __cpssDxChHsuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/* user want to make import/export by one iteration */
#define CPSS_DXCH_HSU_SINGLE_ITERATION_CNS 0xFFFFFFFF


/**
* @enum CPSS_DXCH_HSU_DATA_TYPE_ENT
 *
 * @brief Types of cpss data for export/import/size_calculations actions during
 * HSU process. It could be cpss internal data structures, shadow tables,
 * global variables, data per feature/mechanism.
*/
typedef enum{

    /** LPM DB HSU data type */
    CPSS_DXCH_HSU_DATA_TYPE_LPM_DB_E,

    /** Global HSU data type */
    CPSS_DXCH_HSU_DATA_TYPE_GLOBAL_E,

    /** ALL HSU data type */
    CPSS_DXCH_HSU_DATA_TYPE_ALL_E

} CPSS_DXCH_HSU_DATA_TYPE_ENT;

/*
 * Typedef: struct CPSS_DXCH_HSU_DATA_BLOCK_HEADER_STC
 *
 * Description: HSU data block header
 *
 * Fields:
 *      type            - type of HSU data block
 *      length          - length of stored data
 *      version         - stored data format version
 *      reserved1       - reserved for future needs
 *      reserved2       - reserved for future needs
 *
 */
typedef struct CPSS_DXCH_HSU_DATA_BLOCK_HEADER_STCT
{
    GT_U32                                  type;
    GT_U32                                  length;
    GT_U32                                  version;
    GT_U32                                  reserved1;
    GT_U32                                  reserved2;
}CPSS_DXCH_HSU_DATA_BLOCK_HEADER_STC;


/**
* @internal cpssDxChHsuBlockSizeGet function
* @endinternal
*
* @brief   This function gets the memory size needed to export the required HSU
*         data block.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dataType                 - hsu data type.
*
* @param[out] sizePtr                  - memory size needed to export required hsu data
*                                      block. (calculated in bytes)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS cpssDxChHsuBlockSizeGet
(
    IN   CPSS_DXCH_HSU_DATA_TYPE_ENT dataType,
    OUT  GT_U32                        *sizePtr
);

/**
* @internal cpssDxChHsuExport function
* @endinternal
*
* @brief   This function exports required HSU data block to survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dataType                 - hsu data type.
* @param[in,out] iteratorPtr              - pointer to iterator, to start - set iterator to 0.
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size supposed to be
*                                      exported in current iteration.The minimal value
*                                      is 1k bytes. The export by one single iteration
*                                      is performed when hsuBlockMemSizePtr  points to
*                                      CPSS_DXCH_HSU_SINGLE_ITERATION_CNS value.
*                                      Exact size calculation is done internally.
* @param[in] hsuBlockMemPtr           - pointer to HSU survived restart memory area
* @param[in,out] iteratorPtr              - the iterator - points to the point from which
*                                      process will be continued in future iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size that was not used
*                                      in current iteration.
*
* @param[out] exportCompletePtr        - GT_TRUE -  HSU data export is completed.
*                                      GT_FALSE - HSU data export is not completed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize, dataType.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChHsuExport
(
    IN     CPSS_DXCH_HSU_DATA_TYPE_ENT dataType,
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *exportCompletePtr
);

/**
* @internal cpssDxChHsuImport function
* @endinternal
*
* @brief   This function imports required HSU data block from survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dataType                 - hsu data type.
* @param[in,out] iteratorPtr              - pointer to iterator, to start - set iterator to 0.
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size supposed to be
*                                      imported in current iteration.The minimal value
*                                      is 1k bytes.
*                                      The import by one single iteration is performed
*                                      when hsuBlockMemSizePtr  points to
*                                      CPSS_DXCH_HSU_SINGLE_ITERATION_CNS value.
*                                      The size (saved in export operation) is retrieved
*                                      from hsu data block header.
* @param[in] hsuBlockMemPtr           - pointer to HSU survived restart memory area.
* @param[in,out] iteratorPtr              - the iterator - points to the point from which
*                                      process will be continued in future iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size that was not used
*                                      in current iteration.
*
* @param[out] importCompletePtr        - GT_TRUE -  HSU data import is completed.
*                                      GT_FALSE - HSU data import is not completed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChHsuImport
(
    IN     CPSS_DXCH_HSU_DATA_TYPE_ENT dataType,
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *importCompletePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChHsuh */

