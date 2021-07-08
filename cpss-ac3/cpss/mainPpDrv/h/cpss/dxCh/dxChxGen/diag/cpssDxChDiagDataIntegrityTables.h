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
* @file cpssDxChDiagDataIntegrityTables.h
*
* @brief API definition for tables access
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChDiagDataIntegrityTablesh
#define __cpssDxChDiagDataIntegrityTablesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/private/prvCpssMisc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>

/**
* @struct CPSS_DXCH_LOGICAL_TABLE_SHADOW_INFO_STC
*
* @brief This struct defines logical table shadow info entry
*/
typedef struct{

    IN CPSS_DXCH_LOGICAL_TABLE_ENT   logicalTableName;

    IN CPSS_DXCH_SHADOW_TYPE_ENT     shadowType;

    OUT GT_U32                        numOfBytes;

    OUT GT_STATUS                     isSupported;

} CPSS_DXCH_LOGICAL_TABLE_SHADOW_INFO_STC;

/**
* @struct CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC
*
* @brief This struct defines logical table shadow entry
*/
typedef struct{

    IN GT_U32                                     numOfDataIntegrityElements;

    INOUT CPSS_DXCH_LOGICAL_TABLE_SHADOW_INFO_STC *logicalTablesArr;

} CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC;

/**
* @internal cpssDxChDiagDataIntegrityTableScan function
* @endinternal
*
* @brief   Data integrity SW daemon API
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] locationPtr              - (pointer to) location information,
*                                      ram location is not supported
* @param[in] numOfEntries             - number of entries to read (0xFFFFFFFF means till end of table)
*                                      NOTE: for 'logical table' this value is ignored and taken from :
* @param[in] locationPtr
*
* @param[out] nextEntryIndexPtr        - (pointer to) next index
* @param[out] wasWrapAroundPtr         - (pointer to) was wrap around indication
*                                      NOTE: ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagDataIntegrityTableScan
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC *locationPtr,
    IN  GT_U32                                numOfEntries,
    OUT GT_U32                               *nextEntryIndexPtr,
    OUT GT_BOOL                              *wasWrapAroundPtr
);
/**
* @internal cpssDxChDiagDataIntegrityTableEntryFix function
* @endinternal
*
* @brief   Data integrity : fix specific table entry that is corrupted in the HW,
*         using CPSS shadow for this table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] locationPtr              - (pointer to) location information
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_EMPTY                 - the DFX location is not mapped to CPSS HW table.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagDataIntegrityTableEntryFix
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC *locationPtr
);

/**
* @internal cpssDxChDiagDataIntegrityShadowEntryInfoGet function
* @endinternal
*
* @brief   This function calculates size of memory required for given shadow entry
*         the function allow to understand which tables can be given as parameter to
*         cpssDxChHwPpPhase1Init(...) for parameters :
*         CPSS_DXCH_PP_PHASE1_INIT_INFO_STC::numOfDataIntegrityElements
*         CPSS_DXCH_PP_PHASE1_INIT_INFO_STC::dataIntegrityShadowPtr
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in,out] tablesInfoPtr        - in : info for which logical tables need to
*                                            calculate size of shadow
*                                       out: info filled with size per supported table
*
* @param[out] totalMemSizePtr          - (pointer to) total memory size required for the shadow
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagDataIntegrityShadowEntryInfoGet
(
    IN     GT_U8                                     devNum,
    INOUT  CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC *tablesInfoPtr,
    OUT    GT_U32                                   *totalMemSizePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChDiagDataIntegrityTablesh */
