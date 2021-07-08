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
* @file prvCpssGenHsu.h
*
* @brief Includes generic HSU definitions.
*
*
* @version   9
********************************************************************************
*/

#ifndef __prvCpssGenHsuh
#define __prvCpssGenHsuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/systemRecovery/hsu/cpssGenHsu.h>

/**
* @struct PRV_CPSS_HSU_DATA_BLOCK_HEADER_STC
 *
 * @brief HSU data block header
*/
typedef struct{

    /** type of HSU data block */
    CPSS_HSU_DATA_TYPE_ENT type;

    /** length of stored data, including this header (in bytes) */
    GT_U32 length;

    /** stored data format version */
    GT_U32 version;

    /** reserved for future needs */
    GT_U32 reserved1;

    /** reserved for future needs */
    GT_U32 reserved2;

} PRV_CPSS_HSU_DATA_BLOCK_HEADER_STC;


/**
* @enum PRV_CPSS_HSU_ACTION_TYPE_ENT
 *
 * @brief It describes import/export action.
 * values:
 * PRV_CPSS_HSU_EXPORT_E         - export action
 * PRV_CPSS_HSU_IMPORT_E         - import action
*/
typedef enum{

    PRV_CPSS_HSU_EXPORT_E,

    PRV_CPSS_HSU_IMPORT_E

} PRV_CPSS_HSU_ACTION_TYPE_ENT;


/**
* @enum PRV_CPSS_GEN_HSU_DATA_STAGE_ENT
 *
 * @brief It represents an global shadow iterator stage
 * values:
 * PRV_CPSS_GEN_HSU_CPSS_STAGE_E        - global cpss generic stage
 * PRV_CPSS_GEN_HSU_DRV_STAGE_E         - global driver stage
 * PRV_CPSS_GEN_HSU_LAST_STAGE_E        - global last stage
*/
typedef enum{

    PRV_CPSS_GEN_HSU_CPSS_STAGE_E,

    PRV_CPSS_GEN_HSU_DRV_STAGE_E,

    PRV_CPSS_GEN_HSU_LAST_STAGE_E

} PRV_CPSS_GEN_HSU_DATA_STAGE_ENT;

/**
* @struct PRV_CPSS_HSU_GEN_ITERATOR_STC
 *
 * @brief This struct that holds HSU generic shadow iterator
*/
typedef struct{

    /** iterator magic number */
    GT_U32 magic;

    /** @brief the current generic stage
     *  currStageMemPtr      - the current memory address we're working on
     */
    PRV_CPSS_GEN_HSU_DATA_STAGE_ENT currStage;

    GT_U8 *currStageMemPtr;

    /** @brief memory size remained till the end of stage.
     *  currentStageHsuBlockMemPtr - HSU memory address
     */
    GT_U32 currentStageRemainedSize;

    GT_U8 *currentStageHsuBlockMemPtr;

} PRV_CPSS_HSU_GEN_ITERATOR_STC;



/**
* @internal prvCpssGenHsuExportImport function
* @endinternal
*
* @brief   This function exports/imports Generic HSU data to/from survived restart
*         memory area.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3.
* @note   NOT APPLICABLE DEVICES:  ExMx.
*
* @param[in] actionType               - PRV_CPSS_HSU_EXPORT_E - export action
*                                      PRV_CPSS_HSU_IMPORT_E - import action
* @param[in,out] currentIterPtr           - points to the current iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer hsu block data size supposed to be exported
*                                      in current iteration.
* @param[in] hsuBlockMemPtrPtr        - pointer to HSU survived restart memory area
* @param[in,out] currentIterPtr           - points to the current iteration
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size exported in current iteration.
*
* @param[out] accumSizePtr             - points to accumulated size
* @param[out] actiontCompletePtr       - GT_TRUE - export/import action is complete
*                                      GT_FALSE - otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize, wrong iterator.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssGenHsuExportImport
(
    IN     PRV_CPSS_HSU_ACTION_TYPE_ENT            actionType,
    INOUT  PRV_CPSS_HSU_GEN_ITERATOR_STC           *currentIterPtr,
    INOUT  GT_U32                                  *hsuBlockMemSizePtr,
    IN     GT_U8                                   **hsuBlockMemPtrPtr,
    OUT    GT_U32                                  *accumSizePtr,
    OUT    GT_BOOL                                 *actiontCompletePtr
);


/**
* @internal prvCpssGenHsuExportImportDataHandling function
* @endinternal
*
* @brief   This function handle import/export generic data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3.
* @note   NOT APPLICABLE DEVICES:  ExMx.
*
* @param[in] actionType               - PRV_CPSS_HSU_EXPORT_E - export action
*                                      PRV_CPSS_HSU_IMPORT_E - import action
* @param[in,out] currentIterPtr           - points to the current iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer hsu block data size supposed to be exported
*                                      in current iteration.
* @param[in,out] hsuBlockMemPtrPtr        - pointer to HSU survived restart memory area
* @param[in,out] currentIterPtr           - points to the current iteration
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size exported in current iteration.
* @param[in,out] hsuBlockMemPtrPtr        - pointer to HSU survived restart memory area
*
* @param[out] accumSizePtr             - points to accumulated size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize, wrong iterator.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssGenHsuExportImportDataHandling
(
    IN     PRV_CPSS_HSU_ACTION_TYPE_ENT            actionType,
    INOUT  PRV_CPSS_HSU_GEN_ITERATOR_STC           *currentIterPtr,
    INOUT  GT_U32                                  *hsuBlockMemSizePtr,
    INOUT  GT_U8                                   **hsuBlockMemPtrPtr,
    OUT    GT_U32                                  *accumSizePtr
);

/**
* @internal prvCpssGenGlobalDataSizeGet function
* @endinternal
*
* @brief   This function calculate size of generic CPSS and driver data
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3.
* @note   NOT APPLICABLE DEVICES:  ExMx.
*
*
* @param[out] sizePtr                  -  size calculated in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note none.
*
*/
GT_STATUS prvCpssGenGlobalDataSizeGet
(
    OUT   GT_U32    *sizePtr
);

/**
* @internal prvCpssDxChHsuDataBlockVersionGet function
* @endinternal
*
* @brief   This function gets the current version of HSU data blocks
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3.
* @note   NOT APPLICABLE DEVICES:  ExMx.
*
*
* @param[out] versionPtr               - hsu data block version
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuDataBlockVersionGet
(
    OUT   GT_U32    *versionPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenHsuh */


