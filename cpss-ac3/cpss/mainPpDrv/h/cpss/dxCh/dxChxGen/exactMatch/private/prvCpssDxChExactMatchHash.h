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
* @file prvcpssDxChExactMatchHashh.h
*
* @brief Hash calculate for EXACT MATCH table implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __prvcpssDxChExactMatchHashh
#define __prvcpssDxChExactMatchHashh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>


/* size of data for Exact Match hash in bits   */
#define PRV_CPSS_DXCH_EXACT_MATCH_HASH_DATA_SIZE_VALUE_CNS                  376

/* multiHash init value  */
#define PRV_CPSS_DXCH_EXACT_MATCH_HASH_CRC_INIT_VALUE_CNS                   0xFFFFFFFF

/* Enum values represent size of Exact Match Table */
typedef enum{

    CPSS_EXACT_MATCH_TABLE_SIZE_4KB      ,
    CPSS_EXACT_MATCH_TABLE_SIZE_8KB      ,
    CPSS_EXACT_MATCH_TABLE_SIZE_16KB     ,
    CPSS_EXACT_MATCH_TABLE_SIZE_32KB     ,
    CPSS_EXACT_MATCH_TABLE_SIZE_64KB     ,
    CPSS_EXACT_MATCH_TABLE_SIZE_128KB    ,
    CPSS_EXACT_MATCH_TABLE_SIZE_256KB    ,
    CPSS_EXACT_MATCH_TABLE_SIZE_512KB    ,

    CPSS_EXACT_MATCH_TABLE_SIZE_LAST_E    /* last value */

}CPSS_EXACT_MATCH_TABLE_SIZE_ENT;

/* Enum values represent num of Multiple Hash Tables */
typedef enum{

    CPSS_EXACT_MATCH_MHT_4_E    ,         /* Four Multiple Hash Tables */
    CPSS_EXACT_MATCH_MHT_8_E    ,         /* Eight Multiple Hash Tables */
    CPSS_EXACT_MATCH_MHT_16_E  ,         /* Sixteen Multiple Hash Tables */

    CPSS_EXACT_MATCH_MHT_LAST_E           /* last value */

}CPSS_EXACT_MATCH_MHT_ENT;

/**
* @internal prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                           - device number
* @param[in] entryKeyPtr                      - (pointer to) entry key
* @param[in] numberOfElemInCrcMultiHashArrPtr - (pointer to) number of valid 
*                                                elements in the exactMatchCrcMultiHashArr
* @param[out] exactMatchCrcMultiHashArr[]     - (array of) 'multi hash' CRC results.
*                                               index in this array is entry inside the bank
*                                               + bank Id' 
*                                               size of exactMatchCrcMultiHashArr must be 16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: the function doesn't access the HW,
*       and do only SW calculations.
*/
GT_STATUS prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
   IN  GT_U8                               devNum,
   IN  CPSS_DXCH_EXACT_MATCH_KEY_STC      *entryKeyPtr,
   OUT GT_U32                             *numberOfElemInCrcMultiHashArrPtr,
   OUT GT_U32                              crcMultiHashArr[]

);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvcpssDxChExactMatchHashh */
