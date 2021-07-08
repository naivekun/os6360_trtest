/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChExactMatch.h
*
* @brief Common private Exact Match declarations.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChExactMatchh
#define __prvCpssDxChExactMatchh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>

/* min/max profileId value */
#define PRV_CPSS_DXCH_EXACT_MATCH_MIN_PROFILE_ID_VAL_CNS                    1
#define PRV_CPSS_DXCH_EXACT_MATCH_MAX_PROFILE_ID_VAL_CNS                    15

/* min/max expander action index value */
#define PRV_CPSS_DXCH_EXACT_MATCH_MIN_EXPANDED_ACTION_INDEX_VAL_CNS         0
#define PRV_CPSS_DXCH_EXACT_MATCH_MAX_EXPANDED_ACTION_INDEX_VAL_CNS         15

#define PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_NUM_OF_ENTRIES_VAL_CNS    16

/* the size of an action in words */
#define PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS                           8

/* macro to validate the value of profile ID value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CHECK_PROFILE_ID_VALUE_MAC(profileId)      \
    if(((profileId) < PRV_CPSS_DXCH_EXACT_MATCH_MIN_PROFILE_ID_VAL_CNS) ||   \
       ((profileId) > PRV_CPSS_DXCH_EXACT_MATCH_MAX_PROFILE_ID_VAL_CNS))     \
    {                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);       \
    }

/* macro to validate the value of expanded action index */
#define PRV_CPSS_DXCH_EXACT_MATCH_CHECK_EXPANDED_ACTION_INDEX_VALUE_MAC(expandedActionIndex)    \
    if((expandedActionIndex) > PRV_CPSS_DXCH_EXACT_MATCH_MAX_EXPANDED_ACTION_INDEX_VAL_CNS)     \
    {                                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                          \
    }

/* Exact Match number of banks  (4,8,16)  */
#define PRV_CPSS_DXCH_EXACT_MATCH_NUM_OF_BANKS_GET_MAC(dev,swValue)    \
     swValue = PRV_CPSS_DXCH_PP_MAC(dev)->exactMatchInfo.exactMatchNumOfBanks

/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_EXPANDER_INFO_STC
 *
*  @brief Structure represent the DB of the CPSS for exact match
*  Expander Table in that device INFO "PER DEVICE"
*/
typedef struct{

    /** @brief entry index
     */
    GT_U32 expandedActionValid;

    /** @brief entry index
     */
    GT_U32 expandedActionIndex;

    /** @brief count the number of Exact Match entries defined on
     *         expandedActionIndex
     */
    GT_U32 exactMatchEntriesCounter;

    /** @brief the action type TTI/PCL/EPCL
     */
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   actionType;

    /** @brief the action data to be used in case the useExpanded
     *         flag is false.
     */
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT    actionData;

    /** @brief if useExpanded of a field is true then the data will
     *         be taken from actionData, else it will be taken from
     *         the reduced entry in the Exact Match Table
     */
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT expandedActionOrigin;

    /** @brief Reduced mask to be used when configuring the Exact
     *         Match Entry.
     */
    GT_U32   reducedMaskArr[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];

} PRV_CPSS_DXCH_EXACT_MATCH_EXPANDER_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_STC
 *
*  @brief Structure represent the DB of the CPSS for exact match
*  in that device INFO "PER DEVICE"
*/
typedef struct{

    /** @brief init should be done only once
     */
    GT_BOOL initDone;

    /** @brief array with 16 elements for each Entry in the
     *  Expander Action Table
     */
    PRV_CPSS_DXCH_EXACT_MATCH_EXPANDER_INFO_STC  ExpanderInfoArr[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_NUM_OF_ENTRIES_VAL_CNS];

    /** @brief holds the number of Exact Match banks
    */
    GT_U32 exactMatchNumOfBanks;

} PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_STC;


/**
* @internal prvCpssDxChExactMatchDbInit function
* @endinternal
*
* @brief   the function init Exact Match Shadow DB
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum            - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChExactMatchDbInit
(
    IN GT_U8                                            devNum
);

/**
* @internal prvCpssDxChTtiActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
* @param[in] ttiUseExpandedPtr        -  points to tti use
*                                        Expander flags
*
* @param[out] hwFormatArray            - tti action in hardware format (5 words)
* @param[out] exactMatchReducedHwMaskArray  - tti reduced action
*       mask used for exact match entry (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Logic2HwFormatAndExactMatchReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC                           *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC *ttiUseExpandedPtr,
    OUT GT_U32                                               *hwFormatArray,
    OUT GT_U32                                               *exactMatchReducedHwMaskArray
);

/**
* @internal prvCpssDxChTtiActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
*
* @param[out] hwFormatArray            - tti action in hardware format (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Logic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC         *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
);

/**
* @internal prvCpssDxChTtiActionType2Hw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from hardware
*          format to logic format.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum          - device number
* @param[in] hwFormatArray   - tti action in hardware format
*                              (5 words)
*
* @param[out] logicFormatPtr - points to tti action in logic
*                              format
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Hw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  GT_U32                             *hwFormatArray,
    OUT CPSS_DXCH_TTI_ACTION_2_STC         *logicFormatPtr
);

/**
* @internal prvCpssDxChPclActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                     - device number
* @param[in] logicFormatPtr             - points to pcl action in logic
*                                       format
* @param[in] pclUseExpandedPtr          - points to pcl use Expander
*                                         flags
* @param[out] hwFormatArray            - pcl action in hardware
*                                        format (5 words)
* @param[out] exactMatchReducedHwMaskArray  - pcl reduced action
*                                       mask used for exact match entry (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPclActionLogic2HwFormatAndExactMatchReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC                             *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC *pclUseExpandedPtr,
    OUT GT_U32                                               *hwFormatArray,
    OUT GT_U32                                               *exactMatchReducedHwMaskArray
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChExactMatchh */

