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
* @file cpssDxChExactMatch.c
*
* @brief CPSS Exact Match implementation.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHash.h>

/* the size of default action in words */
#define PRV_CPSS_DXCH_EXACT_MATCH_DEFAULT_ACTION_SIZE_CNS           8

/* the size of an action in words */
#define PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS                   8

/* the size of expanded action in bytes */
#define PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS          31

/* the size of reduced action in bytes */
#define PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS           8

/* the max size of exact match entry in words 4 banks each with 115 bits
   each bank wide is 4 words */
#define PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_MAX_SIZE_CNS                16

/* each bank wide is 4 words */
#define PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS      4

#define PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC(devNum,index)                     \
    if(index >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum)   \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, \
            prvCpssLogErrorMsgExactMatchIndexOutOfRange, devNum, index);

/* Converts key size to number of Exact Match banks value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(_numOfBanks, _keySize)\
    switch (_keySize)                                                                   \
    {                                                                                   \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:                                       \
            _numOfBanks = 1;                                                            \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:                                      \
            _numOfBanks = 2;                                                            \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:                                      \
            _numOfBanks = 3;                                                            \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:                                      \
            _numOfBanks = 4;                                                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* Converts number of Exact Match banks to key size value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_NUM_OF_BANKS_TO_KEY_SIZE_VAL_MAC(_keySize, _numOfBanks)\
    switch (_numOfBanks)                                                                \
    {                                                                                   \
        case 1:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;                             \
            break;                                                                      \
        case 2:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;                            \
            break;                                                                      \
        case 3:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;                            \
            break;                                                                      \
        case 4:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);              \
    }

/* Converts key size to HW value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_HW_VAL_MAC(_hwVal, _keySize)      \
    switch (_keySize)                                                                   \
    {                                                                                   \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:                                       \
            _hwVal = 0;                                                                 \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:                                      \
            _hwVal = 1;                                                                 \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:                                      \
            _hwVal = 2;                                                                 \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:                                      \
            _hwVal = 3;                                                                 \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* Converts HW value key size value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_HW_TO_KEY_SIZE_VAL_MAC(_keySize, _hwVal)\
    switch (_hwVal)                                                                \
    {                                                                                   \
        case 0:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;                             \
            break;                                                                      \
        case 1:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;                            \
            break;                                                                      \
        case 2:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;                            \
            break;                                                                      \
        case 3:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);              \
    }

extern GT_STATUS ttiGetIndexForKeyType
(
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             isLegacyKeyTypeValid,
    OUT GT_U32                              *indexPtr
);

extern GT_STATUS sip5PclUDBEntryIndexGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    OUT GT_U32                               *entryIndexPtr
);

/**
* @internal prvCpssDxChExpandedActionToHwformat function
* @endinternal
*
* @brief   Convert SW configuration to HW values.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum            - device number
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @param[out] hwValuesArr      - values to set to HW
* @param[out] hwReducedMaskArr - reduce entry mask to keep in DB
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChExpandedActionToHwformat
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    *actionPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *expandedActionOriginPtr,
    OUT GT_U32                                              hwValuesArr[],
    OUT GT_U32                                              hwReducedMaskArr[]
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i=0;
    GT_U32    numberOfReducedBytesUsed=0;
    GT_U32    numberOfExpandedWordUsed=0;
    GT_U32    currentHwExpandValueWord=0; /* one word to be updated in the expanded entry */

    GT_U32    hwExpandValueArr[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS];

    GT_U32    tempHwActionArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* 8 words - 256 bits of data */
    GT_U32    tempHwReducedActionMaskWordsArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* 8 words - 256 bits of data */
    GT_U8     tempHwReducedActionMaskBytesArray[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS+1];/* 32 bytes  */


    /* the function takes the SW values given by the user and
       convert them to TTI HW values.

       the HW consist of 31 elements of 9 bits:                                                                                                                                           .
       1 bit is for deciding where to take the data from,                                                                                                                                 .
       8 bits of data that can be the data from the TTI actionPtr                                                                                                                            .
       parameter or the byte number in the Reduced action

       NOTE: only 8 bytes can be configured to be taken from the
       Exact Match reduced entry */

    /* reset expanded entry array */
    cpssOsMemSet(hwExpandValueArr, 0, sizeof(hwExpandValueArr));

    cpssOsMemSet(tempHwActionArray, 0, sizeof(tempHwActionArray));
    cpssOsMemSet(tempHwReducedActionMaskWordsArray, 0, sizeof(tempHwReducedActionMaskWordsArray));
    cpssOsMemSet(tempHwReducedActionMaskBytesArray, 0, sizeof(tempHwReducedActionMaskBytesArray));

    /* convert TTI action to HW format */
    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        /* buid HW data according to TTI action */
        rc = prvCpssDxChTtiActionType2Logic2HwFormatAndExactMatchReducedMask(devNum,
                                                                         &actionPtr->ttiAction,
                                                                         &expandedActionOriginPtr->ttiExpandedActionOrigin,
                                                                         tempHwActionArray,
                                                                         tempHwReducedActionMaskWordsArray);
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        /* buid HW data according to PCL action */
        rc = prvCpssDxChPclActionLogic2HwFormatAndExactMatchReducedMask(devNum,
                                                                    &actionPtr->pclAction,
                                                                    &expandedActionOriginPtr->pclExpandedActionOrigin,
                                                                    tempHwActionArray,
                                                                    tempHwReducedActionMaskWordsArray);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "prvCpssDxChExpandedActionToHwformat wrong actionType \n");
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    /* copy reduce mask from word to byte representation */
    cpssOsMemCpy(tempHwReducedActionMaskBytesArray,tempHwReducedActionMaskWordsArray,sizeof(tempHwReducedActionMaskWordsArray));

    for (i=0;i<PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS;i++)
    {
        if(numberOfReducedBytesUsed>8)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: Can not configure more then 8 bytes in reduce entry \n");
        }
        if (tempHwReducedActionMaskBytesArray[i]!=0)
        {
            numberOfReducedBytesUsed++;
        }
    }

    numberOfReducedBytesUsed=0;
    numberOfExpandedWordUsed=0;
    for (i=0;i<PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS;i++)
    {
        /* reset values for new loop */
        currentHwExpandValueWord=0;

        if (tempHwReducedActionMaskBytesArray[i]!=0)
        {
            /* set the expanded value to hold the reduced byte number */
            U32_SET_FIELD_MAC(currentHwExpandValueWord, 0, 8, numberOfReducedBytesUsed);
            U32_SET_FIELD_MAC(currentHwExpandValueWord, 8, 1, 1);/* used reduce */
            hwExpandValueArr[numberOfExpandedWordUsed] =(currentHwExpandValueWord&0x1FF); /* 9 bits of data */

            numberOfReducedBytesUsed++;
            numberOfExpandedWordUsed++;
        }
        else
        {
            /* build the Expanded entry */
            U32_SET_FIELD_MAC(currentHwExpandValueWord, 0, 8, (tempHwActionArray[i/4]>>((i%4)*8))&0xFF);
            U32_SET_FIELD_MAC(currentHwExpandValueWord, 8, 1, 0);/* used expanded */
            hwExpandValueArr[numberOfExpandedWordUsed] =(currentHwExpandValueWord&0x1FF); /* 9 bits of data */
            numberOfExpandedWordUsed++;
        }
    }

    /* update output parameter */
    cpssOsMemCpy(hwValuesArr, hwExpandValueArr, sizeof(hwExpandValueArr));
    cpssOsMemCpy(hwReducedMaskArr, tempHwReducedActionMaskWordsArray, sizeof(tempHwReducedActionMaskWordsArray));

    return rc;
}
/**
* @internal prvCpssDxChExpandedActionUpdateShadow function
* @endinternal
*
* @brief   Update SW values in shadow
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @param[in] hwValuesArr       - values to set to HW
* @param[in] hwReducedMaskArr  - reduce entry mask to keep in DB
* @param[in] hwReducedValuesArr- values to keep in DB
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChExpandedActionUpdateShadow
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT                     *actionPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *expandedActionOriginPtr,
    IN GT_U32                                               hwReducedMaskArr[]
)
{
    GT_STATUS rc = GT_OK;

    /* NOTE:
        if we are doing override It is the user responsability to
       verify no Exact Match entry is defined on the Expander Action */

    PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionValid=GT_TRUE;

    /* keep the values in the shadow */
    cpssOsMemCpy(&PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionType,&actionType,sizeof(actionType));
    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        cpssOsMemCpy(&PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData.ttiAction,
                     &actionPtr->ttiAction,
                     sizeof(CPSS_DXCH_TTI_ACTION_2_STC));
        cpssOsMemCpy(&PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionOrigin.ttiExpandedActionOrigin,
                 &expandedActionOriginPtr->ttiExpandedActionOrigin,
                 sizeof(CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC));
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        cpssOsMemCpy(&PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData.pclAction,
                     &actionPtr->pclAction,
                     sizeof(CPSS_DXCH_PCL_ACTION_STC));
        cpssOsMemCpy(&PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionOrigin.pclExpandedActionOrigin,
                 &expandedActionOriginPtr->pclExpandedActionOrigin,
                 sizeof(CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC ));
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "illegal actionType\n");
        break;

    }
    cpssOsMemCpy(
        &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].reducedMaskArr,
        hwReducedMaskArr,
        sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].reducedMaskArr));

    return rc;
}

/**
* @internal prvCpssDxChExpandedActionHwToSwValidityMatch
*           function
* @endinternal
*
* @brief   the function check that the HW values corresponds with the Shadow kept in the Add operation
       and return the SW values kept in the shadow
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] hwValueArr        - HW values
* @param[out] actionPtr        - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @param[in] hwValuesArr       - values to set to HW
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChExpandedActionHwToSwValidityMatch
(
    IN GT_U8                                             devNum,
    IN GT_U32                                            expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT             actionType,
    IN GT_U32                                            hwValueArr[],
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *expandedActionOriginPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL isEqual = GT_FALSE;
    GT_U32  hwValuesDbArr[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS];
    GT_U32  reducedMaskArr[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];

    /* get the data from DB */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionValid!=GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "The Expander Action Entry is not valid \n");
    }

    cpssOsMemSet(hwValuesDbArr, 0, sizeof(hwValuesDbArr));
    cpssOsMemSet(reducedMaskArr, 0, sizeof(reducedMaskArr));

    rc = prvCpssDxChExpandedActionToHwformat(devNum,
                                             actionType,
                                             &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData,
                                             &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionOrigin,
                                             hwValuesDbArr,
                                             reducedMaskArr);

    isEqual = (0 == cpssOsMemCmp((GT_VOID*) hwValueArr,
                                 (GT_VOID*) hwValuesDbArr,
                                 sizeof(hwValuesDbArr))) ? GT_TRUE : GT_FALSE;

    if (isEqual == GT_TRUE)
    {
        switch (actionType)
        {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
            /* return the Action and Reduce flags keept in the DB */
            cpssOsMemCpy(&actionPtr->ttiAction,
                         &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData.ttiAction,
                         sizeof(CPSS_DXCH_TTI_ACTION_2_STC));
            cpssOsMemCpy(expandedActionOriginPtr,
                         &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionOrigin.ttiExpandedActionOrigin,
                         sizeof(CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC));
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
            /* return the Action and Reduce flags keept in the DB */
            cpssOsMemCpy(&actionPtr->pclAction,
                         &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData.pclAction,
                         sizeof(CPSS_DXCH_PCL_ACTION_STC));
            cpssOsMemCpy(expandedActionOriginPtr,
                         &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionOrigin.pclExpandedActionOrigin,
                         sizeof(CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC ));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "prvCpssDxChExpandedActionToHwformat wrong actionType \n");
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "SW and HW validity fail - mismatch \n");
    }
    return rc;
}

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
)
{
    GT_U32 rc=GT_OK;
    GT_U32 i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.initDone==GT_TRUE)
        return GT_OK;

    for (i=0;i<PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_NUM_OF_ENTRIES_VAL_CNS;i++)
    {
        /* set all entries to be false
           the first time we set the entry it changed to true forever */
        PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[i].expandedActionValid = GT_FALSE;

        /* set element in place i to have expandedActionIndex i ,  i = from 0 to 15 */
        PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[i].expandedActionIndex = i;

        /* set all expanded fields to GT_TRUE */
        cpssOsMemSet(&(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[i].expandedActionOrigin),
                     0,
                     sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[i].expandedActionOrigin));

        /* reset all actionData fields */
        cpssOsMemSet(&(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[i].actionData),
                     0,
                     sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[i].actionData));

        /* reset all reducedMaskArr fields */
        cpssOsMemSet(&(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[i].reducedMaskArr),
                     0,
                     sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[i].reducedMaskArr));
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.initDone=GT_TRUE;

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchTtiProfileIdSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile Id for TTI keyType
*
* NOTE: Client lookup for given ttiLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileId    - Exact Match profile identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_OUT_OF_RANGE       - parameter not in valid range.
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: legacy key type not supported:
*       CPSS_DXCH_TTI_KEY_IPV4_E,
*       CPSS_DXCH_TTI_KEY_MPLS_E,
*       CPSS_DXCH_TTI_KEY_ETH_E,
*       CPSS_DXCH_TTI_KEY_MIM_E
*
*/
GT_STATUS internal_cpssDxChExactMatchTtiProfileIdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    IN  GT_BOOL                             enableExactMatchLookup,
    IN  GT_U32                              profileId
)
{

    GT_STATUS                           rc;
    GT_U32                              regAddr;
    GT_U32                              entryIndex;
    GT_U32                              value;
    GT_U32                              ttiHitNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT    clientType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch (ttiLookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
            ttiHitNum=0;
            break;
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            ttiHitNum=1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
   /* verify that the client we are configuring match the lookup client value
       set by the API cpssDxChExactMatchClientLookupSet */

    /* pipe 0 and pipe 1 are configured in the set API with the same value,
       Need to read only one of them */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.exactMatchPortMapping[0];

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, ttiHitNum*4, 4, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
    case 0:
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
        break;
    case 1:
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
        break;
    case 2:
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E;
        break;
    case 3:
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    if (clientType != CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal configuration for TTI packet type profile Id \n");
    }

    if (enableExactMatchLookup==GT_TRUE)
    {
        if((profileId > 15)||(profileId==0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    rc = ttiGetIndexForKeyType(keyType, GT_FALSE, &entryIndex);
    if (rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPacketTypeExactMatchProfileId[entryIndex / 2];

    if (enableExactMatchLookup==GT_TRUE)
    {
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 8 * (entryIndex % 2) + 4 * ttiHitNum, 4, profileId);
    }
    else
    {
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 8 * (entryIndex % 2) + 4 * ttiHitNum, 4, 0);
    }
    return rc;
}

/**
* @internal cpssDxChExactMatchTtiProfileIdSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile Id for TTI keyType
*
* NOTE: Client lookup for given ttiLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileId    - Exact Match profile identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_OUT_OF_RANGE       - parameter not in valid range.
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: legacy key type not supported:
*       CPSS_DXCH_TTI_KEY_IPV4_E,
*       CPSS_DXCH_TTI_KEY_MPLS_E,
*       CPSS_DXCH_TTI_KEY_ETH_E,
*       CPSS_DXCH_TTI_KEY_MIM_E
*
*/
GT_STATUS cpssDxChExactMatchTtiProfileIdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    IN  GT_BOOL                             enableExactMatchLookup,
    IN  GT_U32                              profileId
)
{

    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchTtiProfileIdSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId));

    rc = internal_cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchTtiProfileIdGet function
* @endinternal
*
* @brief   Gets the  Exact Match Profile Id form TTI keyType
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[out] profileIdPtr  - (pointer to) Exact Match profile
*                             identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                  - on success
* @retval GT_HW_ERROR            - on hardware error
* @retval GT_OUT_OF_RANGE        - parameter not in valid range.
* @retval GT_BAD_PARAM           - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChExactMatchTtiProfileIdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    OUT GT_BOOL                             *enableExactMatchLookupPtr,
    OUT GT_U32                              *profileIdPtr
)
{

    GT_STATUS rc;
    GT_U32      regAddr;
    GT_U32      entryIndex;
    GT_U32      value;
    GT_U32      ttiHitNum;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enableExactMatchLookupPtr);
    CPSS_NULL_PTR_CHECK_MAC(profileIdPtr);

    switch (ttiLookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
            ttiHitNum=0;
            break;
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            ttiHitNum=1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = ttiGetIndexForKeyType(keyType, GT_FALSE, &entryIndex);
    if (rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIEngine.TTIPacketTypeExactMatchProfileId[entryIndex / 2];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 8 * (entryIndex % 2) + 4*ttiHitNum, 4, &value);

    if (rc == GT_OK)
    {
        if (value==0)
        {
            *enableExactMatchLookupPtr = GT_FALSE;
            *profileIdPtr=0;
        }
        else
        {
            *enableExactMatchLookupPtr = GT_TRUE;
            *profileIdPtr = (GT_U32)value;
        }
    }

    return rc;
}

/**
* @internal cpssDxChExactMatchTtiProfileIdGet function
* @endinternal
*
* @brief   Gets the  Exact Match Profile Id form TTI keyType
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[out] profileIdPtr  - (pointer to) Exact Match profile
*                             identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                  - on success
* @retval GT_HW_ERROR            - on hardware error
* @retval GT_OUT_OF_RANGE        - parameter not in valid range.
* @retval GT_BAD_PARAM           - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChExactMatchTtiProfileIdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    OUT GT_BOOL                             *enableExactMatchLookupPtr,
    OUT GT_U32                              *profileIdPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchTtiProfileIdGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, keyType, ttiLookupNum ,enableExactMatchLookupPtr, profileIdPtr));

    rc = internal_cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,enableExactMatchLookupPtr, profileIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, keyType, ttiLookupNum ,enableExactMatchLookupPtr, profileIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchPclProfileIdSet function
* @endinternal
*
* @brief  Sets the Exact Match Profile Id for PCL packet type
*
* NOTE: Client lookup for given pclLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress
* @param[in] packetType               - PCL packet type
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[out] enableExactMatchLookup- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileId - Exact Match profile identifier
*       (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChExactMatchPclProfileIdSet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   IN  GT_BOOL                             enableExactMatchLookup,
   IN  GT_U32                              profileId
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  regAddr;
    GT_U32                  packetEntryIndex;
    GT_U32                  EntryIndex;
    CPSS_DXCH_TABLE_ENT     tableType=CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E ;
    GT_U32                  hwValue;
    GT_U32                  value;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT clientType;
    GT_U32                  pclHitNum;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch (pclLookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
            pclHitNum=0;
            break;
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            pclHitNum=1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (subProfileId > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* verify that the client we are configuring match the lookup client value
       set by the API cpssDxChExactMatchClientLookupSet */

    /* pipe 0 and pipe 1 are configured in the set API with the same value,
       Need to read only one of them */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.exactMatchPortMapping[0];

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, pclHitNum*4, 4, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
    case 0:
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
        break;
    case 1:
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
        break;
    case 2:
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E;
        break;
    case 3:
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
    case CPSS_PCL_DIRECTION_INGRESS_E:
        if((clientType!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E)&&
           (clientType!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal configuration for PCL ingress packet type profile Id \n");
        }
        break;
    case CPSS_PCL_DIRECTION_EGRESS_E:
        if(clientType!=CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal configuration for PCL egress packet type profile Id \n");
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (enableExactMatchLookup==GT_TRUE)
    {
        if((profileId > 15)||(profileId==0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        hwValue = profileId;
    }
    else
    {
        hwValue=0;
    }

    /* Calculate entry index by packet type + subProfile Id */
    rc = sip5PclUDBEntryIndexGet(devNum, packetType, &packetEntryIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    EntryIndex = ((packetEntryIndex & 0xF)<<3)|(subProfileId & 0x7);
    switch (direction)
    {
        case CPSS_PCL_DIRECTION_INGRESS_E:
            regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).exactMatchProfileIdMapEntry[EntryIndex];
            rc =  prvCpssHwPpSetRegField(devNum,
                                         regAddr,
                                         4*pclHitNum,
                                         4,
                                         hwValue);
            break;
        case CPSS_PCL_DIRECTION_EGRESS_E:
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                 tableType,
                                                 EntryIndex,
                                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                 4*pclHitNum,
                                                 4,
                                                 hwValue);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return rc;
}

/**
* @internal cpssDxChExactMatchPclProfileIdSet function
* @endinternal
*
* @brief  Sets the Exact Match Profile Id for PCL packet type
*
* NOTE: Client lookup for given pclLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress
* @param[in] packetType               - PCL packet type
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[out] enableExactMatchLookup- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileId - Exact Match profile identifier
*       (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPclProfileIdSet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   IN  GT_BOOL                             enableExactMatchLookup,
   IN  GT_U32                              profileId
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchPclProfileIdSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction ,packetType,  subProfileId, pclLookupNum, enableExactMatchLookup, profileId));

    rc = internal_cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType,  subProfileId, pclLookupNum, enableExactMatchLookup, profileId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction ,packetType, subProfileId, pclLookupNum, enableExactMatchLookup, profileId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchPclProfileIdGet function
* @endinternal
*
* @brief  Gets the Exact Match Profile Id form PCL packet type
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress
* @param[in] packetType               - PCL packet type
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileIdPtr - (pointer to)Exact Match profile
*       identifier (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChExactMatchPclProfileIdGet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   OUT GT_BOOL                             *enableExactMatchLookupPtr,
   OUT GT_U32                              *profileIdPtr
)
{
    GT_STATUS               rc;
    GT_U32                  regAddr;
    GT_U32                  packetEntryIndex;
    GT_U32                  EntryIndex;
    GT_U32                  value;
    CPSS_DXCH_TABLE_ENT     tableType=CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E ;
    GT_U32                  pclHitNum;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enableExactMatchLookupPtr);
    CPSS_NULL_PTR_CHECK_MAC(profileIdPtr);

    switch (pclLookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
            pclHitNum=0;
            break;
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            pclHitNum=1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (subProfileId > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Calculate entry index by packet type + subProfile Id */
    rc = sip5PclUDBEntryIndexGet(devNum, packetType, &packetEntryIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    EntryIndex = ((packetEntryIndex & 0xF)<<3)|(subProfileId & 0x7);
    switch (direction)
    {
        case CPSS_PCL_DIRECTION_INGRESS_E:
            regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).exactMatchProfileIdMapEntry[EntryIndex];
            rc = prvCpssHwPpGetRegField(devNum,
                                        regAddr,
                                        4*pclHitNum,
                                        4,
                                        &value);
            break;
        case CPSS_PCL_DIRECTION_EGRESS_E:
            rc = prvCpssDxChReadTableEntryField(devNum,
                                                tableType,
                                                EntryIndex,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                4*pclHitNum,
                                                4,
                                                &value);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (rc == GT_OK)
    {
        if (value==0)
        {
            *enableExactMatchLookupPtr=GT_FALSE;
            *profileIdPtr=0;
        }
        else
        {
            *enableExactMatchLookupPtr=GT_TRUE;
            *profileIdPtr = (GT_U32)value;
        }
    }
    return rc;
}

/**
* @internal cpssDxChExactMatchPclProfileIdGet function
* @endinternal
*
* @brief  Gets the Exact Match Profile Id form PCL packet type
*         and Sub Profile Id
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] packetType               - PCL packet type
* @param[in] direction                - ingress/egress
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: 0..7)
* @param[in] pclLookupNum             - pcl lookup number
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileIdPtr - (pointer to)Exact Match profile
*       identifier (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPclProfileIdGet
(
   IN  GT_U8                               devNum,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   OUT GT_BOOL                             *enableExactMatchLookupPtr,
   OUT GT_U32                              *profileIdPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchPclProfileIdGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction ,packetType, subProfileId, pclLookupNum, enableExactMatchLookupPtr, profileIdPtr));

    rc = internal_cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum, enableExactMatchLookupPtr, profileIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction ,packetType, subProfileId, pclLookupNum, enableExactMatchLookupPtr, profileIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchClientLookupSet function
* @endinternal
*
* @brief   Set global configuration to determine the client of
*          the first Exact Match lookup and the client of the
*          second Exact Match lookup.
*
*   NOTE: this API should be called before configuring
*      Exact Match Profile Id for TTI keyType (cpssDxChExactMatchTtiProfileIdSet)or
*      Exact Match Profile Id for PCL/EPCL packet type (cpssDxChExactMatchPclProfileIdSet)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum               - the device number
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientType           - client type (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChExactMatchClientLookupSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    IN  CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType
)
{
    GT_STATUS rc=GT_OK;               /* function return code         */
    GT_U32    regAddr,pipeNum;
    GT_U32    clientIndex;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch (exactMatchLookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
            clientIndex=0;
            break;
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            clientIndex=1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (clientType>=CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    for (pipeNum = 0; pipeNum < 2; pipeNum++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.exactMatchPortMapping[pipeNum];
        rc =  prvCpssHwPpSetRegField(devNum, regAddr, 4*clientIndex, 4, clientType);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
   return rc;
}

/**
* @internal cpssDxChExactMatchClientLookupSet function
* @endinternal
*
* @brief   Set global configuration to determine the client of
*          the first Exact Match lookup and the client of the
*          second Exact Match lookup.
*
*   NOTE: this API should be called before configuring
*      Exact Match Profile Id for TTI keyType (cpssDxChExactMatchTtiProfileIdSet)or
*      Exact Match Profile Id for PCL/EPCL packet type (cpssDxChExactMatchPclProfileIdSet)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum               - the device number
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientType           - client type (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchClientLookupSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    IN  CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchClientLookupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exactMatchLookupNum, clientType));

    rc = internal_cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, exactMatchLookupNum, clientType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchClientLookupGet function
* @endinternal
*
* @brief   Get global configuration that determine the client of
*          the first Exact Match lookup and the client of the
*          second Exact Match lookup.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum               - the device number
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[out] clientTypePtr        - (pointer to) client type
*                                   (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChExactMatchClientLookupGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    OUT CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            *clientTypePtr
)
{
    GT_STATUS rc=GT_OK;               /* function return code         */
    GT_U32    regAddr;
    GT_U32    value;
    GT_U32    clientIndex;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(clientTypePtr);

    switch (exactMatchLookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
            clientIndex=0;
            break;
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            clientIndex=1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* pipe 0 and pipe 1 are configured in the set API with the same value,
       Need to read only one of them */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.exactMatchPortMapping[0];

    rc =  prvCpssHwPpGetRegField(devNum, regAddr, 4*clientIndex, 4, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
    case 0:
        *clientTypePtr = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
        break;
    case 1:
        *clientTypePtr = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
        break;
    case 2:
        *clientTypePtr = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E;
        break;
    case 3:
        *clientTypePtr = CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChExactMatchClientLookupGet function
* @endinternal
*
* @brief   Get global configuration that determine the client of
*          the first Exact Match lookup and the client of the
*          second Exact Match lookup.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in]  devNum               - the device number
* @param[in]  exactMatchLookupNum  - exact match lookup number
* @param[out] clientTypePtr        - (pointer to) client type
*                                   (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChExactMatchClientLookupGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    OUT CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            *clientTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchClientLookupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exactMatchLookupNum,*clientTypePtr));

    rc = internal_cpssDxChExactMatchClientLookupGet(devNum,exactMatchLookupNum,clientTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, exactMatchLookupNum,*clientTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchActivityBitEnableSet
*           function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum              - device number
* @param[in] enable              - GT_TRUE - enable refreshing
*                                  GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchActivityBitEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc=GT_OK;               /* function return code         */
    GT_U32    regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EM.EMGlobalConfiguration.EMGlobalConfiguration1;
    rc =  prvCpssHwPpSetRegField(devNum,regAddr,20,1,BOOL2BIT_MAC(enable));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "fail in cpssDxChExactMatchActivityBitEnableSet\n");
    }

    return rc;
}

/**
* @internal cpssDxChExactMatchActivityBitEnableSet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] enable             - GT_TRUE - enable refreshing
*                                 GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchActivityBitEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchActivityBitEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChExactMatchActivityBitEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchActivityBitEnableGet
*           function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] enablePtr - (pointer to)
*                               GT_TRUE - enable refreshing
*                               GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchActivityBitEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc=GT_OK;               /* function return code         */
    GT_U32    regAddr;
    GT_U32    value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EM.EMGlobalConfiguration.EMGlobalConfiguration1;
    rc =  prvCpssHwPpGetRegField(devNum,regAddr,20,1,&value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "fail in cpssDxChExactMatchActivityBitEnableGet\n");
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChExactMatchActivityBitEnableGet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] enablePtr          - (pointer to)
*                               GT_TRUE - enable refreshing
*                               GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchActivityBitEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchActivityBitEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChExactMatchActivityBitEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChExactMatchActivityStatusGet
*           function
* @endinternal
*
* @brief   Return the Exact Match activity bit for a given
*          entry.The bit is set by the device when the entry is
*          matched in the Exact Match Lookup. The bit is reset
*          by the CPU as part of the aging process.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in]  devNum             - device number
* @param[in] portGroupsBmp       - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in]  exactMatchEntryIndex - Exact Match entry index
* @param[in]  exactMatchClearActivity - set activity bit to 0
* @param[out] exactMatchActivityStatusPtr  - (pointer to)
*                  GT_FALSE = Not Refreshed; next_age_pass;
*                  Entry was matched since the last reset;
*                  GT_TRUE = Refreshed; two_age_pass; Entry was
*                  not matched since the last reset;
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum 
* @retval GT_NOT_FOUND             - on entry not found 
* @retval GT_OUT_OF_RANGE          - on exactMatchEntryIndex out of range
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchActivityStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  exactMatchEntryIndex,
    IN  GT_BOOL                 exactMatchClearActivity,
    OUT GT_BOOL                 *exactMatchActivityStatusPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32    value=0;
    GT_BOOL   exactMatchValidEntry=GT_FALSE;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

    CPSS_NULL_PTR_CHECK_MAC(exactMatchActivityStatusPtr);

    *exactMatchActivityStatusPtr=GT_FALSE;

    /* loop on all active port groups in the bmp */
    /* the value of exactMatchActivityStatusPtr is an Or operation on all portGroups in the Bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        /* read valid bit, entryType bit, Activity bit  */
        rc = prvCpssDxChPortGroupReadTableEntryField(devNum,portGroupId,
                                     CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                     exactMatchEntryIndex,
                                     0, /* fieldWordNum */
                                     0, /* fieldOffset  */
                                     3, /* fieldLength  */
                                     &value);/* fieldValue   */
        if(rc != GT_OK)
        {
            return rc;
        }

        /* check the entry is valid and the entry type is Key+Action*/
        if ((value&0x1)==1 && ((value>>1)&0x1)==0)
        {
            /* the API should return the Activity bit that is an OR operation on all portGroupsBmp */
            exactMatchValidEntry = GT_TRUE;

            /* check if the Activity bit was set to 1 */
            if (((value>>2)&0x1)==1)
            {
                *exactMatchActivityStatusPtr = GT_TRUE;

                if (exactMatchClearActivity==GT_TRUE)
                {
                    /* write 0 to Activity bit  */
                    rc = prvCpssDxChPortGroupWriteTableEntryField(devNum,portGroupId,
                                                 CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                                 exactMatchEntryIndex,
                                                 0, /* fieldWordNum */
                                                 2, /* fieldOffset  */
                                                 1, /* fieldLength  */
                                                 0);/* fieldValue   */
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    if(exactMatchValidEntry==GT_FALSE)
    {
        rc = GT_NOT_FOUND;
        CPSS_LOG_INFORMATION_MAC("cpssDxChExactMatchActivityStatusGet - Exact Match Entry[%d] is not valid\n", exactMatchEntryIndex);
    }

    return rc;
}

/**
* @internal cpssDxChExactMatchActivityStatusGet function
* @endinternal
*
* @brief   Return the Exact Match activity bit for a given
*          entry.The bit is set by the device when the entry is
*          matched in the Exact Match Lookup. The bit is reset
*          by the CPU as part of the aging process.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in]  devNum             - device number
* @param[in] portGroupsBmp       - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in]  exactMatchEntryIndex - Exact Match entry index
* @param[in]  exactMatchClearActivity - set activity bit to 0
* @param[out] exactMatchActivityStatusPtr  - (pointer to)
*                  GT_FALSE = Not Refreshed; next_age_pass;
*                  Entry was matched since the last reset;
*                  GT_TRUE = Refreshed; two_age_pass; Entry was
*                  not matched since the last reset;
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum 
* @retval GT_NOT_FOUND             - on entry not found 
* @retval GT_OUT_OF_RANGE          - on exactMatchEntryIndex out of range
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchActivityStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  exactMatchEntryIndex,
    IN  GT_BOOL                 exactMatchClearActivity,
    OUT GT_BOOL                 *exactMatchActivityStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchActivityStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp,exactMatchEntryIndex,
                            exactMatchClearActivity,exactMatchActivityStatusPtr));

    rc = internal_cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp, exactMatchEntryIndex,
                                                      exactMatchClearActivity,
                                                      exactMatchActivityStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum,portGroupsBmp,exactMatchEntryIndex,
                              exactMatchClearActivity,exactMatchActivityStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChExactMatchProfileKeyParamsSet
*           function
* @endinternal
*
* @brief   Sets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match
*            profile identifier (APPLICABLE RANGES: 1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchProfileKeyParamsSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC     *keyParamsPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    regAddr;
    GT_U32    i=0;/* for loop */
    GT_U32    value;
    GT_U32    wordNum;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(keyParamsPtr);

    /* check validity of exactMatchProfileIndex */
    PRV_CPSS_DXCH_EXACT_MATCH_CHECK_PROFILE_ID_VALUE_MAC(exactMatchProfileIndex);

    /* check validity of keyStart (6 bits)*/
    if(keyParamsPtr->keyStart >= BIT_6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* check validity of keySize */
    PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_HW_VAL_MAC(value, (keyParamsPtr->keySize));

    /* set ketStart and KeySize fields from control table */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.exactMatchProfileTableControl[exactMatchProfileIndex];
    /* KeySize */
    rc = prvCpssHwPpSetRegField(devNum,regAddr,0,2,value);
    if (rc!=GT_OK)
    {
        return rc;
    }
    /* KeyStart */
    rc = prvCpssHwPpSetRegField(devNum,regAddr,4,6,keyParamsPtr->keyStart);
    if (rc!=GT_OK)
    {
        return rc;
    }

    /* loop on 47 bytes of data */
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i=i+4)
    {
        wordNum = i/4;/* word 0 to word 11 - 12 bytes in data mask */

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.
                                                exactMatchProfileTableMaskData[exactMatchProfileIndex][wordNum];

        /* there are only 47 bytes valid- byte 48 (last byte in word 12) is not part of the mask */
        if ((i+3)<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS)
        {
            value = keyParamsPtr->mask[i + 3];
        }
        else
        {
            value = 0;
        }
        value = (value<<8)|(keyParamsPtr->mask[i+2]);
        value = (value<<8)|(keyParamsPtr->mask[i+1]);
        value = (value<<8)|(keyParamsPtr->mask[i]);



        rc = prvCpssHwPpWriteRegister(devNum, regAddr, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return rc;
}
/**
* @internal cpssDxChExactMatchProfileKeyParamsSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match
*            profile identifier (APPLICABLE RANGES: 1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchProfileKeyParamsSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC     *keyParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchProfileKeyParamsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exactMatchProfileIndex,keyParamsPtr));

    rc = internal_cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex,keyParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum,exactMatchProfileIndex,keyParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchProfileKeyParamsGet
*           function
* @endinternal
*
* @brief   Gets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                 identifier (APPLICABLE RANGES:1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchProfileKeyParamsGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          exactMatchProfileIndex,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *keyParamsPtr
)
{
    GT_STATUS   rc=GT_OK;
    GT_U32      regAddr;
    GT_U32      i=0;/* for loop */
    GT_U32      value;
    GT_U32      wordNum;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(keyParamsPtr);

    /* check validity of exactMatchProfileIndex */
    PRV_CPSS_DXCH_EXACT_MATCH_CHECK_PROFILE_ID_VALUE_MAC(exactMatchProfileIndex);

    cpssOsMemSet(keyParamsPtr->mask,0,sizeof(keyParamsPtr->mask));

     /* get ketStart and KeySize fields from control table */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.exactMatchProfileTableControl[exactMatchProfileIndex];

    /* KeySize */
    rc = prvCpssHwPpGetRegField(devNum,regAddr,0,2,&value);
    if (rc!=GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_HW_TO_KEY_SIZE_VAL_MAC((keyParamsPtr->keySize), value);

    /* KeyStart */
    rc = prvCpssHwPpGetRegField(devNum,regAddr,4,6,&value);
    if (rc!=GT_OK)
    {
        return rc;
    }
    keyParamsPtr->keyStart = value;

    /* loop on 47 bytes of data */
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i=i+4)
    {
        wordNum = i/4;/* word 0 to word 11 - 12 bytes in data mask */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.
                                                exactMatchProfileTableMaskData[exactMatchProfileIndex][wordNum];

        /* read a word of HW data and translate it to 4 bytes of SW data */
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* there are only 47 bytes valid- byte 48 (last byte in word 12) is not part of the mask */
        if ((i+3)<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS)
        {
            keyParamsPtr->mask[i+3] = (GT_U8)((value>>24)& 0XFF);
        }
        keyParamsPtr->mask[i+2]   = (GT_U8)((value>>16)& 0XFF);
        keyParamsPtr->mask[i+1] = (GT_U8)((value>>8)& 0XFF);
        keyParamsPtr->mask[i] = (GT_U8)(value & 0XFF);
    }

    return rc;
}
/**
* @internal cpssDxChExactMatchProfileKeyParamsGet function
* @endinternal
*
* @brief   Gets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                 identifier (APPLICABLE RANGES:1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchProfileKeyParamsGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          exactMatchProfileIndex,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *keyParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchProfileKeyParamsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exactMatchProfileIndex,keyParamsPtr));

    rc = internal_cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex,keyParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum,exactMatchProfileIndex,keyParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchProfileDefaultActionSet
*           function
* @endinternal
*
* @brief   Sets the default Action in case there is no match in the
*          Exact Match lookup and in the TCAM lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                    identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[in] actionPtr          - (pointer to)Exact Match Action
* @param[in] defActionEn        - Enable using Profile
*                               Table default Action in case
*                               there is no match in the Exact
*                               Match lookup and in the TCAM
*                               lookup
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchProfileDefaultActionSet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN GT_BOOL                                  defActionEn
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    regAddr;
    GT_U32    hwDefaultActionArray[PRV_CPSS_DXCH_EXACT_MATCH_DEFAULT_ACTION_SIZE_CNS];
    GT_U32    i=0;/* for loop */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* check validity of exactMatchProfileIndex */
    PRV_CPSS_DXCH_EXACT_MATCH_CHECK_PROFILE_ID_VALUE_MAC(exactMatchProfileIndex);

    if (defActionEn==GT_TRUE)
    {
        CPSS_NULL_PTR_CHECK_MAC(actionPtr);

        /* set defaut entry */
        switch (actionType)
        {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
            /* convert TTI action to HW format */
            rc = prvCpssDxChTtiActionType2Logic2HwFormat(devNum,
                                                         &(actionPtr->ttiAction),
                                                         hwDefaultActionArray);

            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
            /* convert PCL action to HW format */
            rc = prvCpssDxChPclTcamRuleActionSw2HwConvert(devNum,
                                                          &(actionPtr->pclAction),
                                                          hwDefaultActionArray);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* loop on 8 words of data */
        for (i=0;i<PRV_CPSS_DXCH_EXACT_MATCH_DEFAULT_ACTION_SIZE_CNS;i++)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.
                                                    exactMatchProfileTableDefaultActionData[exactMatchProfileIndex][i];
            rc = prvCpssHwPpWriteRegister(devNum,regAddr,hwDefaultActionArray[i]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* set enable defaut field from control table */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.exactMatchProfileTableControl[exactMatchProfileIndex];
    rc = prvCpssHwPpSetRegField(devNum,regAddr,17,1,BOOL2BIT_MAC(defActionEn));
    if (rc!=GT_OK)
    {
        return rc;
    }
    return rc;
}


/**
* @internal cpssDxChExactMatchProfileDefaultActionSet function
* @endinternal
*
* @brief   Sets the default Action in case there is no match in the
*          Exact Match lookup and in the TCAM lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                    identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[in] actionPtr          - (pointer to)Exact Match Action
* @param[in] defActionEn        - Enable using Profile
*                               Table default Action in case
*                               there is no match in the Exact
*                               Match lookup and in the TCAM
*                               lookup
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchProfileDefaultActionSet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN GT_BOOL                                  defActionEn
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchProfileDefaultActionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exactMatchProfileIndex,actionType,
                            actionPtr,defActionEn));

    rc = internal_cpssDxChExactMatchProfileDefaultActionSet(devNum, exactMatchProfileIndex,
                                                            actionType,actionPtr,defActionEn);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum,exactMatchProfileIndex,actionType,
                            actionPtr,defActionEn));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchProfileDefaultActionGet
*           function
* @endinternal
*
* @brief   Gets the default Action in case there is no match in
*          the Exact Match lookup and in the TCAM lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                     identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[in] actionPtr          - (pointer to)Exact Match Action
* @param[in] defActionEnPtr     - (pointer to) Enable using
*                               Profile Table default Action in
*                               case there is no match in the
*                               Exact Match lookup and in the
*                               TCAM lookup
*
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_BAD_PTR            - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchProfileDefaultActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   exactMatchProfileIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    OUT GT_BOOL                                  *defActionEnPtr
)
{
    GT_STATUS   rc=GT_OK;
    GT_U32      regAddr;
    GT_U32      hwDefaultActionArray[PRV_CPSS_DXCH_EXACT_MATCH_DEFAULT_ACTION_SIZE_CNS];
    GT_U32      i=0;/* for loop */
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    CPSS_NULL_PTR_CHECK_MAC(defActionEnPtr);

    /* check validity of exactMatchProfileIndex */
    PRV_CPSS_DXCH_EXACT_MATCH_CHECK_PROFILE_ID_VALUE_MAC(exactMatchProfileIndex);

    /* loop on 8 words of data */
    for (i=0;i<PRV_CPSS_DXCH_EXACT_MATCH_DEFAULT_ACTION_SIZE_CNS;i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.
                                                exactMatchProfileTableDefaultActionData[exactMatchProfileIndex][i];
        rc = prvCpssHwPpReadRegister(devNum,regAddr,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        hwDefaultActionArray[i] = value;
    }

    /* get defaut entry */
    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        /* convert TTI action to SW format */
        rc = prvCpssDxChTtiActionType2Hw2LogicFormat(devNum,
                                                     hwDefaultActionArray,
                                                     &(actionPtr->ttiAction));

        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        /* convert PCL action to SW format */
        rc = prvCpssDxChPclTcamRuleActionHw2SwConvert(devNum,
                                                      CPSS_PCL_DIRECTION_INGRESS_E,
                                                      hwDefaultActionArray,
                                                      &(actionPtr->pclAction));
        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        /* convert EPCL action to SW format */
        rc = prvCpssDxChPclTcamRuleActionHw2SwConvert(devNum,
                                                      CPSS_PCL_DIRECTION_EGRESS_E,
                                                      hwDefaultActionArray,
                                                      &(actionPtr->pclAction));
        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get enable defaut field from control table */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchProfilesTable.exactMatchProfileTableControl[exactMatchProfileIndex];
    rc = prvCpssHwPpGetRegField(devNum,regAddr,17,1,&value);
    if (rc!=GT_OK)
    {
        return rc;
    }
    *defActionEnPtr = BIT2BOOL_MAC(value);
    return rc;
}

/**
* @internal cpssDxChExactMatchProfileDefaultActionGet function
* @endinternal
*
* @brief   Gets the default Action in case there is no match in
*          the Exact Match lookup and in the TCAM lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] exactMatchProfileIndex - Exact Match profile
*                     identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[out] actionPtr         - (pointer to)Exact Match Action
* @param[out] defActionEnPtr    - (pointer to) Enable using
*                               Profile Table default Action in
*                               case there is no match in the
*                               Exact Match lookup and in the
*                               TCAM lookup
*
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_BAD_PTR            - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchProfileDefaultActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   exactMatchProfileIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    OUT GT_BOOL                                  *defActionEnPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchProfileDefaultActionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, exactMatchProfileIndex,actionType,
                            actionPtr,defActionEnPtr));

    rc = internal_cpssDxChExactMatchProfileDefaultActionGet(devNum, exactMatchProfileIndex,
                                                            actionType,actionPtr,defActionEnPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum,exactMatchProfileIndex,actionType,
                            actionPtr,defActionEnPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChExactMatchExpandedActionSet
*           function
* @endinternal
*
* @brief  Sets the expanded action for Exact Match Profile. The
*         API also sets for each action attribute whether to
*         take it from the Exact Match entry action or from
*         the Exact Match profile
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: This API must be called before configuration of
*         Exact Match Entry by API cpssDxChExactMatchEntrySet
*
*         The Expander Action Entry cannot be changed if an
*         Exact Match Rule is using this entry
*/
static GT_STATUS internal_cpssDxChExactMatchExpandedActionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *expandedActionOriginPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    regAddr;
    GT_U32    i=0;/* for loop */
    GT_U32    expandedValueArr[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS];
    GT_U32    reducedMaskArr[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    CPSS_NULL_PTR_CHECK_MAC(expandedActionOriginPtr);

    /* check validity of expandedActionIndex 0-15 */
    PRV_CPSS_DXCH_EXACT_MATCH_CHECK_EXPANDED_ACTION_INDEX_VALUE_MAC(expandedActionIndex);

    cpssOsMemSet(expandedValueArr, 0, sizeof(expandedValueArr));
    cpssOsMemSet(reducedMaskArr, 0, sizeof(reducedMaskArr));

     /* if we are doing override we should verify no Exact Match entry is defined on the Expander Action */
    if((PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionValid==GT_TRUE)&&
       (PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].exactMatchEntriesCounter!=0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "The Expander Action Entry cannot be changed if an Exact Match Rule is using this entry\n");
    }

    rc = prvCpssDxChExpandedActionToHwformat(devNum,actionType,actionPtr,expandedActionOriginPtr,expandedValueArr,reducedMaskArr);
    if (rc  != GT_OK)
    {
        return rc;
    }

    /* exactMatchActionAssignment[16][31] Packet Type/Byte*/
    for (i=0; i<PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS; i++)
    {

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchActionAssignment[expandedActionIndex][i];
         /* Exact Match Action data
            the action data for byte i
            0=origin, default data
            1=bits[0:2] mapped index from Exact Match

            Exact Match Action Type
            Determines whether this byte is assigned from
            the reduces action or from the default value
            */
        rc = prvCpssHwPpSetRegField(devNum,regAddr,0,9,expandedValueArr[i]);
        if (rc!=GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChExpandedActionUpdateShadow(devNum,expandedActionIndex,
                                               actionType,actionPtr,expandedActionOriginPtr,
                                               reducedMaskArr);
    if (rc  != GT_OK)
    {
        return rc;
    }
    return rc;
}

/**
* @internal cpssDxChExactMatchExpandedActionSet function
* @endinternal
*
* @brief  Sets the expanded action for Exact Match Profile. The
*         API also sets for each action attribute whether to
*         take it from the Exact Match entry action or from
*         the Exact Match profile
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: This API must be called before configuration of
*         Exact Match Entry by API cpssDxChExactMatchEntrySet
*
*         The Expander Action Entry cannot be changed if an
*         Exact Match Rule is using this entry
*/
GT_STATUS cpssDxChExactMatchExpandedActionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *expandedActionOriginPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchExpandedActionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, expandedActionIndex,actionType,
                            actionPtr,expandedActionOriginPtr));

    rc = internal_cpssDxChExactMatchExpandedActionSet(devNum, expandedActionIndex,actionType,
                            actionPtr,expandedActionOriginPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum,expandedActionIndex,actionType,
                            actionPtr,expandedActionOriginPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchExpandedActionGet
*           function
* @endinternal
*
* @brief   Gets the action for Exact Match in case of a match in
*          Exact Match lookup The API also sets for each action
*          attribute whether to take it from the Exact Match
*          entry action or from the Exact Match profile
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex -Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[out] actionPtr         -(pointer to)Exact Match Action
* @param[out]expandedActionOriginPtr - (pointer to) Whether to
*                   use the action attributes from the Exact
*                   Match rule action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchExpandedActionGet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *expandedActionOriginPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    regAddr;
    GT_U32    i=0;/* for loop */
    GT_U32    value[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    CPSS_NULL_PTR_CHECK_MAC(expandedActionOriginPtr);

    /* check validity of expandedActionIndex 0-15 */
    PRV_CPSS_DXCH_EXACT_MATCH_CHECK_EXPANDED_ACTION_INDEX_VALUE_MAC(expandedActionIndex);

    cpssOsMemSet(value, 0, sizeof(value));

    /* exactMatchActionAssignment[16][31] Packet Type/Byte*/
    for (i=0; i<PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS; i++)
    {

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchActionAssignment[expandedActionIndex][i];
         /* Exact Match Action data
            the action data for byte i
            0=origin, default data
            1=bits[0:2] mapped index from Exact Match

            Exact Match Action Type
            Determines whether this byte is assigned from
            the reduces action or from the default value*/
        rc = prvCpssHwPpGetRegField(devNum,regAddr,0,9,&value[i]);
        if (rc!=GT_OK)
        {
            return rc;
        }
    }

    /* the function check that the HW values corresponds with the Shadow kept in the Add operation
       and return the SW values kept in the shadow */
    rc = prvCpssDxChExpandedActionHwToSwValidityMatch(devNum,expandedActionIndex,actionType,value,actionPtr,expandedActionOriginPtr);
    if (rc  != GT_OK)
    {
       return rc;
    }

    return rc;
}
/**
* @internal cpssDxChExactMatchExpandedActionGet function
* @endinternal
*
* @brief   Gets the action for Exact Match in case of a match in
*          Exact Match lookup The API also sets for each action
*          attribute whether to take it from the Exact Match
*          entry action or from the Exact Match profile
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex -Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
* @param[in] actionType        - Exact Match Action Type
* @param[out]actionPtr         -(pointer to)Exact Match Action
* @param[out]expandedActionOriginPtr - (pointer to) Whether to
*                   use the action attributes from the Exact
*                   Match rule action or from the profile
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchExpandedActionGet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *expandedActionOriginPtr
)
{
        GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchExpandedActionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, expandedActionIndex,actionType,
                            actionPtr,expandedActionOriginPtr));

    rc = internal_cpssDxChExactMatchExpandedActionGet(devNum, expandedActionIndex,actionType,
                            actionPtr,expandedActionOriginPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum,expandedActionIndex,actionType,
                            actionPtr,expandedActionOriginPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal prvCpssDxChExactMatchTtiActionValidity function
* @endinternal
*
* @brief  Check that the Exact Match TTI Action correspond with
*         the TTI Action defined in the Profile Expander Table
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum        - device number
* @param[in] logicFormatPtr -points to Exact Match Entry - tti
*                            action in logic format
* @param[in] dbLogicFormatPtr-points to Expanded Entry - tti
*       action in logic format (keept in DB)
* @param[in] ttiExpandedActionOriginPtr-points to Expanded flags
*       keept in DB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvCpssDxChExactMatchTtiActionValidity
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_TTI_ACTION_2_STC                           *logicFormatPtr,
    IN  CPSS_DXCH_TTI_ACTION_2_STC                           *dbLogicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC *ttiExpandedActionOriginPtr
)
{
    GT_STATUS   rc=GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbLogicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(ttiExpandedActionOriginPtr);

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->command != dbLogicFormatPtr->command)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in command field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionUserDefinedCpuCode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->userDefinedCpuCode != dbLogicFormatPtr->userDefinedCpuCode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in userDefinedCpuCode field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMirrorToIngressAnalyzerIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if ((logicFormatPtr->mirrorToIngressAnalyzerEnable != dbLogicFormatPtr->mirrorToIngressAnalyzerEnable)||
            (logicFormatPtr->mirrorToIngressAnalyzerIndex != dbLogicFormatPtr->mirrorToIngressAnalyzerIndex))
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirrorToIngressAnalyzerEnable/mirrorToIngressAnalyzerIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRedirectCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->redirectCommand != dbLogicFormatPtr->redirectCommand)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirectCommand field \n");
        }
    }
    if(logicFormatPtr->redirectCommand==CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionEgressInterface==GT_FALSE)
        {
            /* check that the fields are equal */
            if  (logicFormatPtr->egressInterface.type != dbLogicFormatPtr->egressInterface.type)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.type field \n");
            }
            switch(logicFormatPtr->egressInterface.type)
            {
            case CPSS_INTERFACE_PORT_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.devPort.portNum != dbLogicFormatPtr->egressInterface.devPort.portNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.devPort.portNum  field \n");
                }
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.devPort.hwDevNum != dbLogicFormatPtr->egressInterface.devPort.hwDevNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.devPort.hwDevNum  field \n");
                }
                break;
            case CPSS_INTERFACE_TRUNK_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.trunkId != dbLogicFormatPtr->egressInterface.trunkId)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.trunkId field \n");
                }
                break;
            case CPSS_INTERFACE_VIDX_E:
                /* check that the fields are equal */
                if (logicFormatPtr->egressInterface.vidx != dbLogicFormatPtr->egressInterface.vidx)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.vidx  field \n");
                }
                break;
            case CPSS_INTERFACE_VID_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.vlanId != dbLogicFormatPtr->egressInterface.vlanId)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.vlanId field \n");
                }
                break;
            case CPSS_INTERFACE_DEVICE_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.hwDevNum != dbLogicFormatPtr->egressInterface.hwDevNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.hwDevNum field \n");
                }
                break;
            case CPSS_INTERFACE_FABRIC_VIDX_E:
                 /* check that the fields are equal */
                if (logicFormatPtr->egressInterface.fabricVidx != dbLogicFormatPtr->egressInterface.fabricVidx)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.fabricVidx field \n");
                }
                break;
            case CPSS_INTERFACE_INDEX_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.index != dbLogicFormatPtr->egressInterface.index)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.index field \n");
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface \n");
                break;
            }
        }

        if (ttiExpandedActionOriginPtr->overwriteExpandedActionVntl2Echo==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vntl2Echo != dbLogicFormatPtr->vntl2Echo)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vntl2Echo field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelStart==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->tunnelStart != dbLogicFormatPtr->tunnelStart)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tunnelStart field \n");
            }
        }
        if (logicFormatPtr->tunnelStart==GT_FALSE)
        {
            if (ttiExpandedActionOriginPtr->overwriteExpandedActionArpPtr == GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->arpPtr != dbLogicFormatPtr->arpPtr)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in arpPtr field \n");
                }
            }
        }
        else
        {
            if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelIndex==GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->tunnelStartPtr != dbLogicFormatPtr->tunnelStartPtr)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tunnelStartPtr field \n");
                }
            }
            if (ttiExpandedActionOriginPtr->overwriteExpandedActionTsPassengerPacketType==GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->tsPassengerPacketType != dbLogicFormatPtr->tsPassengerPacketType)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tsPassengerPacketType field \n");
                }
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyMacDa==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->modifyMacDa != dbLogicFormatPtr->modifyMacDa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in modifyMacDa field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyMacSa==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->modifyMacSa != dbLogicFormatPtr->modifyMacSa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in modifyMacSa field \n");
            }
        }
    }
    if(logicFormatPtr->redirectCommand==CPSS_DXCH_TTI_VRF_ID_ASSIGN_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionVrfId==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vrfId != dbLogicFormatPtr->vrfId)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vrfId field \n");
            }
        }
    }
    if(logicFormatPtr->redirectCommand==CPSS_DXCH_TTI_NO_REDIRECT_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionFlowId==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->flowId != dbLogicFormatPtr->flowId)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in flowId field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpclUdbConfigTable==GT_FALSE)
        {
            /* check that the fields are equal */
            if ((logicFormatPtr->iPclUdbConfigTableEnable != dbLogicFormatPtr->iPclUdbConfigTableEnable)||
                (logicFormatPtr->iPclUdbConfigTableIndex != dbLogicFormatPtr->iPclUdbConfigTableIndex))
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in iPclUdbConfigTableEnable/iPclUdbConfigTableIndex field \n");
            }
        }
    }
    if(logicFormatPtr->redirectCommand==CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionRouterLttPtr==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->routerLttPtr!= dbLogicFormatPtr->routerLttPtr)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in routerLttPtr field \n");
            }
        }
    }
    if(logicFormatPtr->redirectCommand!=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl2OverrideConfigIndex == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->pcl1OverrideConfigIndex != dbLogicFormatPtr->pcl1OverrideConfigIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pcl1OverrideConfigIndex field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl1OverrideConfigIndex==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->pcl0_1OverrideConfigIndex != dbLogicFormatPtr->pcl0_1OverrideConfigIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pcl0_1OverrideConfigIndex field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl0OverrideConfigIndex==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->pcl0OverrideConfigIndex != dbLogicFormatPtr->pcl0OverrideConfigIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pcl0OverrideConfigIndex field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpclConfigIndex==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->iPclConfigIndex!= dbLogicFormatPtr->iPclConfigIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in iPclConfigIndex field \n");
            }
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToCentralCounter==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bindToCentralCounter != dbLogicFormatPtr->bindToCentralCounter)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bindToCentralCounter field \n");
        }
    }
    if (logicFormatPtr->bindToCentralCounter==GT_TRUE)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionCentralCounterIndex == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->centralCounterIndex != dbLogicFormatPtr->centralCounterIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in centralCounterIndex field \n");
            }
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToPolicerMeter==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bindToPolicerMeter != dbLogicFormatPtr->bindToPolicerMeter)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bindToPolicerMeter field \n");
        }
    }
    if (logicFormatPtr->bindToPolicerMeter==GT_TRUE)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToPolicer == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->bindToPolicer != dbLogicFormatPtr->bindToPolicer)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bindToPolicer field \n");
            }
        }
    }
    if ((logicFormatPtr->bindToPolicerMeter==GT_TRUE)||
        (logicFormatPtr->bindToCentralCounter==GT_TRUE))
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionPolicerIndex==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->policerIndex != dbLogicFormatPtr->policerIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in policerIndex field \n");
            }
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceIdSetEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceIdSetEnable != dbLogicFormatPtr->sourceIdSetEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceIdSetEnable field \n");
        }
    }
    if (logicFormatPtr->sourceIdSetEnable==GT_TRUE)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceId == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->sourceId != dbLogicFormatPtr->sourceId)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceId field \n");
            }
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionActionStop==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->actionStop != dbLogicFormatPtr->actionStop)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in actionStop field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBridgeBypass==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bridgeBypass != dbLogicFormatPtr->bridgeBypass)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bridgeBypass field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIngressPipeBypass==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ingressPipeBypass != dbLogicFormatPtr->ingressPipeBypass)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ingressPipeBypass field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanPrecedence==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag0VlanPrecedence!= dbLogicFormatPtr->tag0VlanPrecedence)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag0VlanPrecedence field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionNestedVlanEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->nestedVlanEnable != dbLogicFormatPtr->nestedVlanEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in nestedVlanEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanCmd==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag0VlanCmd != dbLogicFormatPtr->tag0VlanCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag0VlanCmd field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag0VlanId != dbLogicFormatPtr->tag0VlanId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag0VlanId field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1VlanCmd==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag1VlanCmd != dbLogicFormatPtr->tag1VlanCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag1VlanCmd field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1VlanId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag1VlanId != dbLogicFormatPtr->tag1VlanId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag1VlanId field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosPrecedence==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qosPrecedence != dbLogicFormatPtr->qosPrecedence)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qosPrecedence field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosProfile==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qosProfile != dbLogicFormatPtr->qosProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qosProfile field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyDscp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->modifyDscp != dbLogicFormatPtr->modifyDscp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in modifyDscp field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyTag0==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->modifyTag0Up != dbLogicFormatPtr->modifyTag0Up)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in modifyTag0Up field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionKeepPreviousQoS==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->keepPreviousQoS != dbLogicFormatPtr->keepPreviousQoS)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in keepPreviousQoS field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustUp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->trustUp != dbLogicFormatPtr->trustUp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in trustUp field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustDscp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->trustDscp != dbLogicFormatPtr->trustDscp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in trustDscp field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustExp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->trustExp != dbLogicFormatPtr->trustExp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in trustExp field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRemapDscp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->remapDSCP != dbLogicFormatPtr->remapDSCP)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in remapDSCP field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0Up==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag0Up != dbLogicFormatPtr->tag0Up)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag0Up field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1UpCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag1UpCommand != dbLogicFormatPtr->tag1UpCommand)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag1UpCommand field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1Up==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag1Up != dbLogicFormatPtr->tag1Up)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag1Up field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtPassengerPacketType==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ttPassengerPacketType != dbLogicFormatPtr->ttPassengerPacketType)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ttPassengerPacketType field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCopyTtlExpFromTunnelHeader==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->copyTtlExpFromTunnelHeader != dbLogicFormatPtr->copyTtlExpFromTunnelHeader)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in copyTtlExpFromTunnelHeader field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelTerminate==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tunnelTerminate != dbLogicFormatPtr->tunnelTerminate)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tunnelTerminate field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mplsCommand != dbLogicFormatPtr->mplsCommand)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mplsCommand field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionHashMaskIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->hashMaskIndex!= dbLogicFormatPtr->hashMaskIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in hashMaskIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosMappingTableIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qosMappingTableIndex != dbLogicFormatPtr->qosMappingTableIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qosMappingTableIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsLLspQoSProfileEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mplsLLspQoSProfileEnable != dbLogicFormatPtr->mplsLLspQoSProfileEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mplsLLspQoSProfileEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsTtl==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mplsTtl != dbLogicFormatPtr->mplsTtl)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mplsTtl field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionEnableDecrementTtl==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->enableDecrementTtl != dbLogicFormatPtr->enableDecrementTtl)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in enableDecrementTtl field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceEPortAssignmentEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceEPortAssignmentEnable != dbLogicFormatPtr->sourceEPortAssignmentEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceEPortAssignmentEnable field \n");
        }
    }
    if(logicFormatPtr->sourceEPortAssignmentEnable==GT_TRUE)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceEPort==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->sourceEPort != dbLogicFormatPtr->sourceEPort)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceEPort field \n");
            }
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtHeaderLength==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ttHeaderLength != dbLogicFormatPtr->ttHeaderLength)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ttHeaderLength field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIsPtpPacket==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->isPtpPacket != dbLogicFormatPtr->isPtpPacket)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in isPtpPacket field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamTimeStampEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamTimeStampEnable != dbLogicFormatPtr->oamTimeStampEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamTimeStampEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPtpTriggerType==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ptpTriggerType != dbLogicFormatPtr->ptpTriggerType)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ptpTriggerType field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPtpOffset==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ptpOffset != dbLogicFormatPtr->ptpOffset)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ptpOffset field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamOffsetIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamOffsetIndex != dbLogicFormatPtr->oamOffsetIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamOffsetIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable==GT_FALSE)
    {
        /* No Support in CPSS */
        /* check that the fields are equal
        if (logicFormatPtr-> != dbLogicFormatPtr->)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in field \n");
        }
        */
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProcessEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamProcessEnable != dbLogicFormatPtr->oamProcessEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamProcessEnablefield \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCwBasedPw==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->cwBasedPw != dbLogicFormatPtr->cwBasedPw)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in cwBasedPw field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtlExpiryVccvEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ttlExpiryVccvEnable != dbLogicFormatPtr->ttlExpiryVccvEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ttlExpiryVccvEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPwe3FlowLabelExist==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->pwe3FlowLabelExist != dbLogicFormatPtr->pwe3FlowLabelExist)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pwe3FlowLabelExist field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPwCwBasedETreeEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->pwCwBasedETreeEnable != dbLogicFormatPtr->pwCwBasedETreeEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pwCwBasedETreeEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosUseUpAsIndexEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qosUseUpAsIndexEnable != dbLogicFormatPtr->qosUseUpAsIndexEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qosUseUpAsIndexEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRxIsProtectionPath==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->rxIsProtectionPath != dbLogicFormatPtr->rxIsProtectionPath)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in rxIsProtectionPath field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRxProtectionSwitchEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->rxProtectionSwitchEnable != dbLogicFormatPtr->rxProtectionSwitchEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in rxProtectionSwitchEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSetMacToMe==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->setMacToMe != dbLogicFormatPtr->setMacToMe)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in setMacToMe field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProfile==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamProfile != dbLogicFormatPtr->oamProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamProfile field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionApplyNonDataCwCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->applyNonDataCwCommand != dbLogicFormatPtr->applyNonDataCwCommand)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in applyNonDataCwCommand field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionContinueToNextTtiLookup==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->continueToNextTtiLookup != dbLogicFormatPtr->continueToNextTtiLookup)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in continueToNextTtiLookup field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamChannelTypeToOpcodeMappingEnable != dbLogicFormatPtr->oamChannelTypeToOpcodeMappingEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamChannelTypeToOpcodeMappingEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->passengerParsingOfTransitMplsTunnelMode != dbLogicFormatPtr->passengerParsingOfTransitMplsTunnelMode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in passengerParsingOfTransitMplsTunnelMode field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr-> passengerParsingOfTransitNonMplsTransitTunnelEnable != dbLogicFormatPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in passengerParsingOfTransitNonMplsTransitTunnelEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSkipFdbSaLookupEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->skipFdbSaLookupEnable != dbLogicFormatPtr->skipFdbSaLookupEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in skipFdbSaLookupEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ipv6SegmentRoutingEndNodeEnable != dbLogicFormatPtr->ipv6SegmentRoutingEndNodeEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ipv6SegmentRoutingEndNodeEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelHeaderStartL4Enable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tunnelHeaderStartL4Enable != dbLogicFormatPtr->tunnelHeaderStartL4Enable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tunnelHeaderStartL4Enable field \n");
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChExactMatchPclActionValidity function
* @endinternal
*
* @brief  Check that the Exact Match PCL Action correspond with
*         the PCL Action defined in the Profile Expander Table
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum        - device number
* @param[in] logicFormatPtr -points to Exact Match Entry - tti
*                            action in logic format
* @param[in] dbLogicFormatPtr-points to Expanded Entry - tti
*       action in logic format (keept in DB)
* @param[in] pcloverwriteExpandedPtr-points to Expanded flags
*       keept in DB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvCpssDxChExactMatchPclActionValidity
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC                                *logicFormatPtr,
    IN  CPSS_DXCH_PCL_ACTION_STC                                *dbLogicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC    *pclExpandedActionOriginPtr
)
{
   GT_STATUS   rc=GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbLogicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(pclExpandedActionOriginPtr);

    if (pclExpandedActionOriginPtr->overwriteExpandedActionUserDefinedCpuCode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.cpuCode != dbLogicFormatPtr->mirror.cpuCode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.cpuCode field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPktCmd==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->pktCmd != dbLogicFormatPtr->pktCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pktCmd field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionRedirectCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->redirect.redirectCmd != dbLogicFormatPtr->redirect.redirectCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.redirectCmd field \n");
        }
    }

    if(logicFormatPtr->redirect.redirectCmd==CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        if (pclExpandedActionOriginPtr->overwriteExpandedActionEgressInterface==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.outInterface.type != dbLogicFormatPtr->redirect.data.outIf.outInterface.type )
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.type  field \n");
            }
            switch(logicFormatPtr->redirect.data.outIf.outInterface.type)
            {
            case CPSS_INTERFACE_PORT_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.devPort.portNum != dbLogicFormatPtr->redirect.data.outIf.outInterface.devPort.portNum )
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.devPort.portNum  field \n");
                }
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.devPort.hwDevNum  != dbLogicFormatPtr->redirect.data.outIf.outInterface.devPort.hwDevNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.devPort.hwDevNum field \n");
                }
                break;
            case CPSS_INTERFACE_TRUNK_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.trunkId  != dbLogicFormatPtr->redirect.data.outIf.outInterface.trunkId)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.trunkId field \n");
                }
                break;
            case CPSS_INTERFACE_VIDX_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.vidx != dbLogicFormatPtr->redirect.data.outIf.outInterface.vidx )
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.vidx  field \n");
                }
                break;
            case CPSS_INTERFACE_VID_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.vlanId != dbLogicFormatPtr->redirect.data.outIf.outInterface.vlanId )
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.vlanId  field \n");
                }
                break;
            case CPSS_INTERFACE_DEVICE_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.hwDevNum != dbLogicFormatPtr->redirect.data.outIf.outInterface.hwDevNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.hwDevNum field \n");
                }
                break;
            case CPSS_INTERFACE_FABRIC_VIDX_E:
                 /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.fabricVidx != dbLogicFormatPtr->redirect.data.outIf.outInterface.fabricVidx)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.fabricVidx field \n");
                }
                break;
            case CPSS_INTERFACE_INDEX_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.index != dbLogicFormatPtr->redirect.data.outIf.outInterface.index)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.index field \n");
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in logicFormatPtr->redirect.data.outIf.outInterface.typefield \n");
                break;
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionVntl2Echo==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.vntL2Echo != dbLogicFormatPtr->redirect.data.outIf.vntL2Echo)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.vntL2Echo field \n");
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyMacDA==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.modifyMacDa != dbLogicFormatPtr->redirect.data.outIf.modifyMacDa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.modifyMacDa field \n");
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyMacSA==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.modifyMacSa != dbLogicFormatPtr->redirect.data.outIf.modifyMacSa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.vntL2Echo field \n");
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionTunnelStart==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.tunnelStart != dbLogicFormatPtr->redirect.data.outIf.tunnelStart)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.tunnelStart field \n");
            }
        }
        if (logicFormatPtr->redirect.data.outIf.tunnelStart==GT_TRUE)
        {
            if (pclExpandedActionOriginPtr->overwriteExpandedActionTunnelIndex == GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.tunnelPtr != dbLogicFormatPtr->redirect.data.outIf.tunnelPtr)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in logicFormatPtr->redirect.data.outIf.tunnelPtr field \n");
                }
            }
            if (pclExpandedActionOriginPtr->overwriteExpandedActionTsPassengerPacketType==GT_FALSE)
            {
               /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.tunnelType != dbLogicFormatPtr->redirect.data.outIf.tunnelType)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.tunnelType field \n");
                }
            }
        }
        else
        {
            if (pclExpandedActionOriginPtr->overwriteExpandedActionArpPtr==GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.arpPtr != dbLogicFormatPtr->redirect.data.outIf.arpPtr)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.arpPtr field \n");
                }
            }
        }
    }
    if(logicFormatPtr->redirect.redirectCmd==CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
    {
        if(pclExpandedActionOriginPtr->overwriteExpandedActionMacSa==GT_FALSE)
        {
            if(cpssOsMemCmp(logicFormatPtr->redirect.data.modifyMacSa.macSa.arEther,
                         dbLogicFormatPtr->redirect.data.modifyMacSa.macSa.arEther,
                         sizeof(dbLogicFormatPtr->redirect.data.modifyMacSa.macSa.arEther))!=0)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.modifyMacSa.macSa field \n");
            }
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.modifyMacSa.arpPtr != dbLogicFormatPtr->redirect.data.modifyMacSa.arpPtr)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.modifyMacSa.arpPtr field \n");
            }
        }
    }
    if(logicFormatPtr->redirect.redirectCmd==CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
    {
        if (pclExpandedActionOriginPtr->overwriteExpandedActionRouterLttPtr==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.routerLttIndex != dbLogicFormatPtr->redirect.data.routerLttIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.routerLttIndex field \n");
            }
        }
    }
    if(logicFormatPtr->redirect.redirectCmd==CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
    {
        if (pclExpandedActionOriginPtr->overwriteExpandedActionVrfId==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.vrfId != dbLogicFormatPtr->redirect.data.vrfId )
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.vrfId field \n");
            }
        }
    }

    if(logicFormatPtr->redirect.redirectCmd==CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E)
    {
        if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyMacDA==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.noRedirect.modifyMacDa != dbLogicFormatPtr->redirect.data.noRedirect.modifyMacDa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.noRedirect.modifyMacDa field \n");
            }
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.noRedirect.arpPtr != dbLogicFormatPtr->redirect.data.noRedirect.arpPtr)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.noRedirect.arpPtr field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionActionStop==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->actionStop != dbLogicFormatPtr->actionStop)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in actionStop field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSetMacToMe==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->setMacToMe != dbLogicFormatPtr->setMacToMe)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in setMacToMe field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPCL1OverrideConfigIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->lookupConfig.pcl1OverrideConfigIndex != dbLogicFormatPtr->lookupConfig.pcl1OverrideConfigIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in lookupConfig.pcl1OverrideConfigIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPCL2OverrideConfigIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->lookupConfig.pcl0_1OverrideConfigIndex != dbLogicFormatPtr->lookupConfig.pcl0_1OverrideConfigIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in lookupConfig.pcl0_1OverrideConfigIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionIPCLConfigurationIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->lookupConfig.ipclConfigIndex != dbLogicFormatPtr->lookupConfig.ipclConfigIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in lookupConfig.ipclConfigIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPolicerEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->policer.policerEnable != dbLogicFormatPtr->policer.policerEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in policer.policerEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPolicerIndex==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->policer.policerId != dbLogicFormatPtr->policer.policerId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in policer.policerId field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.mirrorTcpRstAndFinPacketsToCpu != dbLogicFormatPtr->mirror.mirrorTcpRstAndFinPacketsToCpu)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.mirrorTcpRstAndFinPacketsToCpu field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirrorMode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.egressMirrorToAnalyzerMode != dbLogicFormatPtr->mirror.egressMirrorToAnalyzerMode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.egressMirrorToAnalyzerMode field \n");
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirror==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.mirrorToRxAnalyzerPort != dbLogicFormatPtr->mirror.mirrorToRxAnalyzerPort)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.mirrorToRxAnalyzerPort field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.egressMirrorToAnalyzerIndex != dbLogicFormatPtr->mirror.egressMirrorToAnalyzerIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.egressMirrorToAnalyzerIndex field \n");
        }

        /* check that the fields are equal */
        if (logicFormatPtr->mirror.mirrorToTxAnalyzerPortEn != dbLogicFormatPtr->mirror.mirrorToTxAnalyzerPortEn)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.mirrorToTxAnalyzerPortEn field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.ingressMirrorToAnalyzerIndex != dbLogicFormatPtr->mirror.ingressMirrorToAnalyzerIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.ingressMirrorToAnalyzerIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionBypassBridge==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->bypassBridge != dbLogicFormatPtr->bypassBridge)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bypassBridge field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionBypassIngressPipe==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bypassIngressPipe != dbLogicFormatPtr->bypassIngressPipe)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bypassIngressPipe field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionNestedVlanEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.ingress.nestedVlan != dbLogicFormatPtr->vlan.ingress.nestedVlan)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.nestedVlan field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlanPrecedence==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.ingress.precedence != dbLogicFormatPtr->vlan.ingress.precedence)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.precedence field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan0Command==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.ingress.modifyVlan != dbLogicFormatPtr->vlan.ingress.modifyVlan)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.modifyVlan field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.egress.vlanCmd != dbLogicFormatPtr->vlan.egress.vlanCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.egress.vlanCmd field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan0==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.ingress.vlanId != dbLogicFormatPtr->vlan.ingress.vlanId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.vlanId field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.egress.vlanId != dbLogicFormatPtr->vlan.egress.vlanId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.egress.vlanId field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan1Command==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->vlan.ingress.vlanId1Cmd != dbLogicFormatPtr->vlan.ingress.vlanId1Cmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.vlanId1Cmd field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.egress.vlanId1ModifyEnable != dbLogicFormatPtr->vlan.egress.vlanId1ModifyEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.egress.vlanId1ModifyEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan1==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.ingress.vlanId1 != dbLogicFormatPtr->vlan.ingress.vlanId1)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.vlanId1 field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->vlan.egress.vlanId1 != dbLogicFormatPtr->vlan.egress.vlanId1)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.egress.vlanId1 field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourceIdEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceId.assignSourceId != dbLogicFormatPtr->sourceId.assignSourceId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceId.assignSourceId field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourceId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceId.sourceIdValue != dbLogicFormatPtr->sourceId.sourceIdValue)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceId.sourceIdValue field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMatchCounterEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->matchCounter.enableMatchCount != dbLogicFormatPtr->matchCounter.enableMatchCount)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in matchCounter,enableMatchCount field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMatchCounterIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->matchCounter.matchCounterIndex != dbLogicFormatPtr->matchCounter.matchCounterIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in matchCounter.matchCounterIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQosProfileMakingEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qos.ingress.profileAssignIndex != dbLogicFormatPtr->qos.ingress.profileAssignIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.profileAssignIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQosPrecedence==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qos.ingress.profilePrecedence != dbLogicFormatPtr->qos.ingress.profilePrecedence)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.profilePrecedence field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQoSProfile==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qos.ingress.profileIndex != dbLogicFormatPtr->qos.ingress.profileIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.profileIndex field \n");
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionQoSModifyDSCP==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qos.ingress.modifyDscp != dbLogicFormatPtr->qos.ingress.modifyDscp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.modifyDscp field \n");
        }
         /* check that the fields are equal */
        if (logicFormatPtr->qos.egress.modifyDscp != dbLogicFormatPtr->qos.egress.modifyDscp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.modifyDscp field \n");
        }

    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp0==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qos.egress.up != dbLogicFormatPtr->qos.egress.up)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.up field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyUp0==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qos.egress.modifyUp != dbLogicFormatPtr->qos.egress.modifyUp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.modifyUp field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyUp1==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qos.ingress.modifyUp != dbLogicFormatPtr->qos.ingress.modifyUp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.modifyUp field \n");
        }
         /* check that the fields are equal */
        if (logicFormatPtr->qos.egress.up1ModifyEnable != dbLogicFormatPtr->qos.egress.up1ModifyEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.up1ModifyEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp1Command==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->qos.ingress.up1Cmd != dbLogicFormatPtr->qos.ingress.up1Cmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.up1Cmd field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp1==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qos.ingress.up1 != dbLogicFormatPtr->qos.ingress.up1)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.up1 field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->qos.egress.up1 != dbLogicFormatPtr->qos.egress.up1)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.up1 field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamTimestampEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oam.timeStampEnable != dbLogicFormatPtr->oam.timeStampEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oam.timeStampEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamOffsetIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oam.offsetIndex != dbLogicFormatPtr->oam.offsetIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oam.offsetIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamProcessingEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oam.oamProcessEnable != dbLogicFormatPtr->oam.oamProcessEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oam.oamProcessEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamProfile==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oam.oamProfile != dbLogicFormatPtr->oam.oamProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oam.oamProfile field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionFlowId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->flowId != dbLogicFormatPtr->flowId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in flowId field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourcePortEnable==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->sourcePort.assignSourcePortEnable != dbLogicFormatPtr->sourcePort.assignSourcePortEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourcePort.assignSourcePortEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourcePort==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourcePort.sourcePortValue != dbLogicFormatPtr->sourcePort.sourcePortValue)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourcePort.sourcePortValue field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionLatencyMonitor==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->latencyMonitor.latencyProfile != dbLogicFormatPtr->latencyMonitor.latencyProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in latencyMonitor.latencyProfile field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionLatencyMonitorEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->latencyMonitor.monitoringEnable != dbLogicFormatPtr->latencyMonitor.monitoringEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in latencyMonitor.monitoringEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSkipFdbSaLookup==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->skipFdbSaLookup != dbLogicFormatPtr->skipFdbSaLookup)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in skipFdbSaLookup field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionTriggerInterrupt==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->triggerInterrupt != dbLogicFormatPtr->triggerInterrupt)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in triggerInterrupt field \n");
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaMetadataAssignEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        /* TBD pha fields should be updated after implementation will be added
        if (logicFormatPtr->epclPhaInfo.phaThreadType != dbLogicFormatPtr->epclPhaInfo.phaThreadType)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadType field \n");
        }*/
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaMetadata==GT_FALSE)
    {
        /* check that the fields are equal */
        /* TBD pha fields should be updated after implementation will be added
        if (logicFormatPtr->epclPhaInfo.phaThreadUnion != dbLogicFormatPtr->epclPhaInfo.phaThreadUnion)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadUnion field \n");
        }*/
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaThreadNumberAssignEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        /* TBD pha fields should be updated after implementation will be added
        if (logicFormatPtr->epclPhaInfo.phaThreadIdAssignmentMode != dbLogicFormatPtr->epclPhaInfo.phaThreadIdAssignmentMode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadIdAssignmentMode field \n");
        }*/
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaThreadNumber==GT_FALSE)
    {
        /* check that the fields are equal */
        /* TBD pha fields should be updated after implementation will be added
        if (logicFormatPtr->epclPhaInfo.phaThreadId != dbLogicFormatPtr->epclPhaInfo.phaThreadId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadId field \n");
        }*/
    }

    return rc;
}

/**
* @internal prvCpssDxChExactMatchActionValidity function
* @endinternal
*
* @brief  Check that the Exact Match Action correspond with the
*         Action defined in the Profile Expander Table
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum        - device number
* @param[in] expandedActionIndex-Exact Match profile identifier
*                     (APPLICABLE RANGES:1..15)
* @param[in] actionPtr -points to Exact Match Entry - action in
*                   logic format
* @param[out] exactMatchHwActionArray- the full Hw
*                   representation for logicFormatPtr
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvCpssDxChExactMatchActionValidity
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    *actionPtr,
    OUT GT_U32                                              exactMatchHwActionArray[]
)
{
    GT_U32 rc = GT_OK;

    /* Exact Match Action Validity */
    /* get defaut entry */
    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
       /* convert TTI action to HW format */
        rc = prvCpssDxChTtiActionType2Logic2HwFormat(devNum,
                                                     &(actionPtr->ttiAction),
                                                     exactMatchHwActionArray);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Check that the Exact Match TTI Action correspond with
           the TTI Action defined in the Profile Expander Table */
        rc = prvCpssDxChExactMatchTtiActionValidity(devNum,
                                                    &(actionPtr->ttiAction),
                                                    &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData.ttiAction,
                                                    &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionOrigin.ttiExpandedActionOrigin);

        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        /* convert PCL action to HW format */
        rc = prvCpssDxChPclTcamRuleActionSw2HwConvert(devNum,
                                                      &(actionPtr->pclAction),
                                                      exactMatchHwActionArray);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Check that the Exact Match PCL Action correspond with
           the PCL Action defined in the Profile Expander Table */
        rc = prvCpssDxChExactMatchPclActionValidity(devNum,
                                            &(actionPtr->pclAction),
                                            &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData.pclAction,
                                            &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionOrigin.pclExpandedActionOrigin);

        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal prvCpssDxChExactMatchIndexValidity function
* @endinternal
*
* @brief   check validity of Exact Match index
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] index                 - Exact match entry index
* @param[in] keySize               - Exact match key size
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_VALUE             - on wrong HW value
*/
static GT_STATUS prvCpssDxChExactMatchIndexValidity
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT   keySize
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr=0;
    GT_U32    bankNum=0, mask=0;
    GT_U32    value=0;

    /* index can not be bigger the max num of Exact Match lines */
    if (index >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Exact Match 'index' must be in range [0..%d] but got[%d]" ,
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum - 1 , index);
    }

    /* get number of banks in the system */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EM.EMGlobalConfiguration.EMGlobalConfiguration2;
    rc =  prvCpssHwPpGetRegField(devNum,regAddr,2,2,&value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "fail in internal_cpssDxChExactMatchPortGroupEntrySet\n");
    }
    switch (value)
    {
    case 0:/* Four Multiple Hash Tables */
        /* 4 banks are used : bank=0..3
           number of bank is represented in 2 bits ==> mask is 0x3 */
        mask = 0x3;
        break;
    case 1:/* Eight Multiple Hash Tables */
        /* 8 banks are used: bank=0..7
           number of bank is represented in 3 bits ==> mask is 0x7 */
        mask = 0x7;
        break;
    case 2:/* Sixteen Multiple Hash Tables  */
        /* 16 banks are used: bank=0..15
           number of bank is represented in 4 bits ==> mask is 0xF */
        mask = 0xF;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    bankNum = index & mask; /* the index is built from the lineNumber+bankNumber */

    /* The index to the Exact Match entry should be valid according to the keySize.
       If the keySize is 5B (use 1 bank) then all indexes are legal
       If the keySize is 19B (use 2 banks) then only indexes%2=0 are legal
       If the keySize is 33B (use 3 banks) then only indexes%3=0 are legal
       If the keySize is 47B (use 4 banks) then only indexes%4=0 are legal */

    if ((((keySize)==CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E)&&(bankNum%4!=0))||
        (((keySize)==CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E)&&(bankNum%3!=0))||
        (((keySize)==CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E)&&(bankNum%2!=0)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The Exact Match Index DO NOT correspond with the keySize\n");
    }

    return rc;
}

/**
* @internal prvCpssDxChExactMatchReducedToHwformat function
* @endinternal
*
* @brief    build the reduce entry to set into HW
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] expandedActionIndex   - Expanded entry index
* @param[in] exactMatchHwActionArray - Exact match HW Action
* @param[out]reducedHwActionArr - Exact match HW Reduced Action
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_VALUE             - on wrong HW value
*/
static GT_STATUS prvCpssDxChExactMatchReducedToHwformat
(
    IN  GT_U8        devNum,
    IN  GT_U32       expandedActionIndex,
    IN  GT_U32       exactMatchHwActionArray[],
    OUT GT_U8        reducedHwActionArr[]
)
{
    GT_U32  i=0;
    GT_U32  currentHwReducedValueByte=0; /* one byte to be updated in the reduced entry */
    GT_U32  numberOfReducedBytesUsed=0;
    GT_U8   exactMatchReducedHwMaskBytesArray[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS+1];/* 32 bytes of mask  */

    cpssOsMemSet(exactMatchReducedHwMaskBytesArray, 0, sizeof(exactMatchReducedHwMaskBytesArray));

     /* copy reduce mask from word to byte representation */
    cpssOsMemCpy(exactMatchReducedHwMaskBytesArray,
                 &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].reducedMaskArr,
                 sizeof(exactMatchReducedHwMaskBytesArray));


    /* build the reduce entry to set into HW*/
    for (i=0;
          (i<PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS &&
           numberOfReducedBytesUsed <= PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS);
          i++)
    {
        /* reset values for new loop */
        currentHwReducedValueByte=0;

        if (exactMatchReducedHwMaskBytesArray[i]!=0)
        {
            /* build the Reduced entry - the exactMatchHwActionArray already have the expanded and the reduced data.
               we made sure that the expanded fields were updated exactly as they were set in the Expanded Table */
            U32_SET_FIELD_MAC(currentHwReducedValueByte, 0, 8, (exactMatchHwActionArray[i/4]>>((i%4)*8))&0xFF);

            /* get the reduced byte keept in the DB and combine it */
            reducedHwActionArr[numberOfReducedBytesUsed] = (GT_U8)(currentHwReducedValueByte&0xFF);
            numberOfReducedBytesUsed++;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchReducedToSWformat function
* @endinternal
*
* @brief    build the reduce entry from HW into SW format
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] expandedActionIndex   - Expanded entry index
* @param[in] actionType            - the action type
* @param[in]reducedHwActionArr - Exact match HW Reduced Action
* @param[out] exactMatchHwActionArray - Exact match HW Action
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_VALUE             - on wrong HW value
*/
static GT_STATUS prvCpssDxChExactMatchReducedToSWformat
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN  GT_U8                                    reducedHwActionArr[],
    OUT GT_U32                                   exactMatchHwActionArray[]
)
{
    GT_STATUS rc= GT_OK;
    GT_U32  i=0;
    GT_U32  numberOfReducedBytesUsed=0;
    GT_U8   exactMatchReducedHwMaskBytesArray[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS+1];/* 32 bytes of mask  */

    cpssOsMemSet(exactMatchReducedHwMaskBytesArray, 0, sizeof(exactMatchReducedHwMaskBytesArray));

     /* copy reduce mask from word to byte representation */
    cpssOsMemCpy(exactMatchReducedHwMaskBytesArray,
                 &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].reducedMaskArr,
                 sizeof(exactMatchReducedHwMaskBytesArray));

    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
       /* convert TTI action to HW format */
        rc = prvCpssDxChTtiActionType2Logic2HwFormat(devNum,
                  &(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData.ttiAction),
                  exactMatchHwActionArray);
        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        /* convert PCL action to HW format */
        rc = prvCpssDxChPclTcamRuleActionSw2HwConvert(devNum,
                &(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionData.pclAction),
                exactMatchHwActionArray);
        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* build full action from the reduce entry in HW
       and values from expanded entry */
    for (i=0;i<PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS;i++)
    {
        if (exactMatchReducedHwMaskBytesArray[i]!=0)
        {
            if(numberOfReducedBytesUsed > PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error - more then supported reduce bytes\n");
            }
            /* get the Expanded byte keept in the DB and combine it */
            U32_SET_FIELD_MAC((exactMatchHwActionArray[i/4]),((i%4)*8), 8, reducedHwActionArr[numberOfReducedBytesUsed]);
            numberOfReducedBytesUsed++;
        }
    }
    return rc;
}

/**
* @internal prvDxChExactMatchEntryToHwformat function
* @endinternal
*
* @brief  build entry hw format key + ReducedAction
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum              - device number
* @param[in] expandedActionIndex - Expanded entry index
* @param[in] entryPtr         - (pointer to)Exact Match SW entry
* @param[in]reducedHwActionArr- Exact match HW Reduced Action
* @param[out]hwDataArr        - Exact match HW entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_VALUE             - on wrong HW value
*/
static GT_STATUS prvDxChExactMatchEntryToHwformat
(
    IN GT_U8                            devNum,
    IN  GT_U32                          expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC  *entryPtr,
    IN  GT_U8                           reducedHwActionArr[],
    OUT GT_U32                          hwDataArr[]
)
{
    GT_U32 value;

    /*  bit 0
        0x0 = invalid
        0x1 = valid */
    SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E,
                                         1);/* set valid bit to 1*/
    /*  bit 1
        Defines the Exact Match entry type
        0x0 = Exact Match Key and Action
        0x1 = Exact Match Key Only*/
    SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E,
                                         0);/* first element always have Key+Action Type */
    /*  bit 2
        The bit is set by the device when the entry is matched in the Exact Match Lookup.
        0x0 = Not Refreshed
        0x1 = Refreshed */
    SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_AGE_E,
                                         0);/* clear Activity bit */
    /* bit 3-4
        Defines the key size
        0x0 = Size 5B; size_5_byte; key size is 5B; ;
        0x1 = Size 19B; size_19_byte; key size is 19B; ;
        0x2 = Size 33B; size_33_byte; key size is 33B; ;
        0x3 = Size 47B; size_47_byte; key size is 47B; ; */

    PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_HW_VAL_MAC(value, (entryPtr->key.keySize));

    SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_SIZE_E,
                                         value);
    /* bit 5
       Specifies which Exact Match lookup this entry is configured to.
       0x0 = First Lookup
       0x1 = Second Lookup*/
    switch(entryPtr->lookupNum)
    {
    case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
        value = 0;
        break;
    case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
        value = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "fail in prvDxChExactMatchEntryToHwformat illegal lookupNum\n");
    }
    SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_LOOKUP_NUM_E,
                                         value);
    /* bit 6-45
       Bit [39:0] of the search key*/
    SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_SET_MAC(devNum,hwDataArr,(&entryPtr->key.pattern[0]));

    /* bits 46-113 Reduced Action bits. Those action bits are expanded by the Action Expander.
       bits 46-109 - reduced action 64 bits
       bits 110-113 - expandedActionIndex 4 bits*/
     SIP6_EXACT_MATCH_ENTRY_FIELD_REDUCED_ACTION_SET_MAC(devNum,
                                                         hwDataArr,
                                                         (&reducedHwActionArr[0]),
                                                         expandedActionIndex);

    if (entryPtr->key.keySize>CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E)
    {
        /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
        /*  bit 0
            0x0 = invalid
            0x1 = valid */
        SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,&hwDataArr[4],
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E,
                                         1);/* set valid bit to 1 */
        /*  bit 1
            Defines the Exact Match entry type
            0x0 = Exact Match Key and Action
            0x1 = Exact Match Key Only*/
        SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,&hwDataArr[4],
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E,
                                         1);/* second element Key only Type */
        /* bits 2-113
           Bits from the Concatenated Key.
           The location of the bits depend on the location of the entry in the Concatenated Chain, as follows:
           First Entry after the action: Bits [151:40] of the Concatenated Key
           Second Entry after the action: Bits [263:152] of the Concatenated Key
           Third Entry after the action: Bits [375:264] of the Concatenated Key*/

        /*First Entry after the action: Bits [151:40] of the Concatenated Key*/
        SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_ONLY_SET_MAC(devNum,
                                                      (&hwDataArr[4]),
                                                      (&entryPtr->key.pattern[5]));
    }
    if (entryPtr->key.keySize>CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E)
    {
        /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E */
        /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
        /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
        /*  bit 0
            0x0 = invalid
            0x1 = valid */
        SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,&hwDataArr[8],
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E,
                                         1);/* set valid bit to 1 */
        /*  bit 1
            Defines the Exact Match entry type
            0x0 = Exact Match Key and Action
            0x1 = Exact Match Key Only*/
        SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,&hwDataArr[8],
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E,
                                         1);/* second element Key only Type */
        /* bits 2-113
           Bits from the Concatenated Key.
           The location of the bits depend on the location of the entry in the Concatenated Chain, as follows:
           First Entry after the action: Bits [151:40] of the Concatenated Key
           Second Entry after the action: Bits [263:152] of the Concatenated Key
           Third Entry after the action: Bits [375:264] of the Concatenated Key*/

        /*Second Entry after the action: Bits [263:152] of the Concatenated Key*/
        SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_ONLY_SET_MAC(devNum,
                                                      (&hwDataArr[8]),
                                                      (&entryPtr->key.pattern[19]));
    }
    if (entryPtr->key.keySize>CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E)
    {
        /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E */
         /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E */
        /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
        /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
        /*  bit 0
            0x0 = invalid
            0x1 = valid */
        SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,&hwDataArr[12],
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E,
                                         1);/* set valid bit to 1 */
        /*  bit 1
            Defines the Exact Match entry type
            0x0 = Exact Match Key and Action
            0x1 = Exact Match Key Only*/
        SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,&hwDataArr[12],
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E,
                                         1);/* second element Key only Type */
        /* bits 2-113
           Bits from the Concatenated Key.
           The location of the bits depend on the location of the entry in the Concatenated Chain, as follows:
           First Entry after the action: Bits [151:40] of the Concatenated Key
           Second Entry after the action: Bits [263:152] of the Concatenated Key
           Third Entry after the action: Bits [375:264] of the Concatenated Key*/

        /*Third Entry after the action: Bits [375:264] of the Concatenated Key*/
        SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_ONLY_SET_MAC(devNum,
                                                      (&hwDataArr[12]),
                                                      (&entryPtr->key.pattern[33]));
    }
    return GT_OK;
}


/**
* @internal prvDxChExactMatchEntryToSwformat function
* @endinternal
*
* @brief  build entry SW format key + ReducedAction
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] hwDataArr        - Exact match HW entry
* @param[in] fullEntryEnable  - GT_TRUE: examine full ExactMatch
*                               GT_TRUE: examine Key+Action bank
*                               in the ExactMatch Entry
* @param[in] validPtr         -( pointer to) is the entry valid
*                               or not
* @param[out]entryPtr         - (pointer to)Exact Match SW entry
* @param[out]reducedHwActionArr- Exact match HW Reduced Action

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_VALUE             - on wrong HW value
*/
static GT_STATUS prvDxChExactMatchEntryToSwformat
(
    IN GT_U8                            devNum,
    IN GT_U32                           hwDataArr[],
    IN GT_BOOL                          fullEntryEnable,
    OUT GT_BOOL                         *validPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC *entryPtr,
    OUT GT_U8                           reducedHwActionArr[],
    OUT GT_U32                          *expandedActionIndexPtr

)
{
    GT_U32 value;

    /*  bit 0
        0x0 = invalid
        0x1 = valid */
    SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E,
                                         value);/* get valid bit to 1*/
    if (value!=1)
    {
       *validPtr=GT_FALSE;
       return GT_OK;
    }

    *validPtr=GT_TRUE;

    /*  bit 1
        Defines the Exact Match entry type
        0x0 = Exact Match Key and Action
        0x1 = Exact Match Key Only*/
    SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E,
                                         value);/* first element always have Key+Action Type */

    if (value!=0)
    {
        /* the entry is invalid for get operation
           it is a MUST to have valid bit on and EntryTpe=0 to be a valid entry for read operation */
         *validPtr=GT_FALSE;
         return GT_OK;
    }
    /*  bit 2
        The bit is set by the device when the entry is matched in the Exact Match Lookup.
        0x0 = Not Refreshed
        0x1 = Refreshed */
    /*SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_AGE_E,
                                         value);*//* clear Activity bit */

    /* bit 3-4
        Defines the key size
        0x0 = Size 5B; size_5_byte; key size is 5B;
        0x1 = Size 19B; size_19_byte; key size is 19B;
        0x2 = Size 33B; size_33_byte; key size is 33B;
        0x3 = Size 47B; size_47_byte; key size is 47B; */
    SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_SIZE_E,
                                         value);
    PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_HW_TO_KEY_SIZE_VAL_MAC((entryPtr->key.keySize), value);

    /* bit 5
       Specifies which Exact Match lookup this entry is configured to.
       0x0 = First Lookup
       0x1 = Second Lookup*/
    SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
                                         SIP6_EXACT_MATCH_TABLE_FIELDS_LOOKUP_NUM_E,
                                         value);
    switch(value)
    {
    case 0:
        entryPtr->lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        break;
    case 1:
        entryPtr->lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "fail in prvDxChExactMatchEntryToHwformat illegal lookupNum\n");
    }

    /* bit 6-45
       Bit [39:0] of the search key*/
    SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_GET_MAC(devNum,hwDataArr,(&entryPtr->key.pattern[0]));

    /* bits 46-113 Reduced Action bits. Those action bits are expanded by the Action Expander.
       bits 46-109 - reduced action 64 bits
       bits 110-113 - expandedActionIndex 4 bits*/
    SIP6_EXACT_MATCH_ENTRY_FIELD_REDUCED_ACTION_GET_MAC(devNum,hwDataArr,(&reducedHwActionArr[0]),(*expandedActionIndexPtr));

    if(fullEntryEnable==GT_TRUE)
    {
        if (entryPtr->key.keySize>CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E)
        {
            /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
            /*  bit 0
                0x0 = invalid
                0x1 = valid */
            SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,(&hwDataArr[4]),
                                                 SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E,
                                                 value);/* get valid bit to 1*/

            if (value!=1)
            {
                /* invalid entry */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Exact Match invalid entry\n");
            }
            /*  bit 1
                Defines the Exact Match entry type
                0x0 = Exact Match Key and Action
                0x1 = Exact Match Key Only*/
            SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,(&hwDataArr[4]),
                                                 SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E,
                                                 value);/* second element Key only Type */

            if (value!=1)
            {
                /* invalid entry */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Exact Match invalid Exact Match entry type\n");
            }

            /* bits 2-113
               Bits from the Concatenated Key.
               The location of the bits depend on the location of the entry in the Concatenated Chain, as follows:
               First Entry after the action: Bits [151:40] of the Concatenated Key
               Second Entry after the action: Bits [263:152] of the Concatenated Key
               Third Entry after the action: Bits [375:264] of the Concatenated Key*/

            /*First Entry after the action: Bits [151:40] of the Concatenated Key*/
            SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_ONLY_GET_MAC(devNum,
                                                          (&hwDataArr[4]),
                                                          (&entryPtr->key.pattern[5]));
        }
        if (entryPtr->key.keySize>CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E)
        {
            /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E */
            /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
            /*  bit 0
                0x0 = invalid
                0x1 = valid */
            SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,(&hwDataArr[8]),
                                                 SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E,
                                                 value);/* get valid bit to 1*/

            if (value!=1)
            {
                /* invalid entry */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Exact Match invalid entry\n");
            }
            /*  bit 1
                Defines the Exact Match entry type
                0x0 = Exact Match Key and Action
                0x1 = Exact Match Key Only*/
            SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,(&hwDataArr[8]),
                                                 SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E,
                                                 value);/* third element Key only Type */

            if (value!=1)
            {
                /* invalid entry */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Exact Match invalid Exact Match entry type\n");
            }

            /* bits 2-113
               Bits from the Concatenated Key.
               The location of the bits depend on the location of the entry in the Concatenated Chain, as follows:
               First Entry after the action: Bits [151:40] of the Concatenated Key
               Second Entry after the action: Bits [263:152] of the Concatenated Key
               Third Entry after the action: Bits [375:264] of the Concatenated Key*/

            /*Second Entry after the action: Bits [263:152] of the Concatenated Key*/
            SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_ONLY_GET_MAC(devNum,
                                                          (&hwDataArr[8]),
                                                          (&entryPtr->key.pattern[19]));
        }
        if (entryPtr->key.keySize>CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E)
        {
            /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E */
             /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E */
            /* build next bank for size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
            /*  bit 0
                0x0 = invalid
                0x1 = valid */
            SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,(&hwDataArr[12]),
                                                 SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E,
                                                 value);/* get valid bit to 1*/

            if (value!=1)
            {
                /* invalid entry */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Exact Match invalid entry\n");
            }
            /*  bit 1
                Defines the Exact Match entry type
                0x0 = Exact Match Key and Action
                0x1 = Exact Match Key Only*/
            SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum,(&hwDataArr[12]),
                                                 SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E,
                                                 value);/* third element Key only Type */

            if (value!=1)
            {
                /* invalid entry */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Exact Match invalid Exact Match entry type\n");
            }

            /* bits 2-113
               Bits from the Concatenated Key.
               The location of the bits depend on the location of the entry in the Concatenated Chain, as follows:
               First Entry after the action: Bits [151:40] of the Concatenated Key
               Second Entry after the action: Bits [263:152] of the Concatenated Key
               Third Entry after the action: Bits [375:264] of the Concatenated Key*/

            /*Third Entry after the action: Bits [375:264] of the Concatenated Key*/
            SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_ONLY_GET_MAC(devNum,
                                                          (&hwDataArr[12]),
                                                          (&entryPtr->key.pattern[33]));
        }
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChExactMatchPortGroupEntrySet
*           function
* @endinternal
*
* @brief   Sets the exact match entry and its action
*
*   NOTE: this API should be called when there is a valid entry
*   in expandedActionProfile in the Profile Expander Table
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] index        - entry index in the exact match table
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] expandedActionIndex-Exact Match profile identifier
*                     (APPLICABLE RANGES:1..15)
* @param[in] entryPtr      - (pointer to)Exact Match entry
* @param[in] actionType    - Exact Match Action Type(TTI or PCL)
* @param[in] actionPtr     - (pointer to)Exact Match Action (TTI
*                            Action or PCL Action)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchPortGroupEntrySet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    IN GT_U32                                   expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC          *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U8     reducedActionHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS];/* the final 8 bytes to set the HW */
    GT_U32    exactMatchReducedHwArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* 8 words - 256 bits of reduced data */

    GT_U32    hwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_MAX_SIZE_CNS];/* hw data to set entryType/Age/keySize/lookupNum/key/reduccedAction/ */
    GT_U32    singleHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* data for one bank - 115 bits */
    GT_U32    startWord, entryIndexPerBank;
    GT_U32    bankNum=0,numOfBanks=0;

    GT_U32    exactMatchHwActionArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* the full Hw representation for actionPtr */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

    /* check validity of expandedActionIndex 0-15 */
    PRV_CPSS_DXCH_EXACT_MATCH_CHECK_EXPANDED_ACTION_INDEX_VALUE_MAC(expandedActionIndex);

    cpssOsMemSet(reducedActionHwDataArr, 0, sizeof(reducedActionHwDataArr));
    cpssOsMemSet(exactMatchReducedHwArray, 0, sizeof(exactMatchReducedHwArray));
    cpssOsMemSet(exactMatchHwActionArray, 0, sizeof(exactMatchHwActionArray));
    cpssOsMemSet(hwDataArr, 0, sizeof(hwDataArr));
    cpssOsMemSet(singleHwDataArr, 0, sizeof(singleHwDataArr));

    /* this API should be called when there is a valid entry
       in expandedActionProfile in the Profile Expander Table*/
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].expandedActionValid==GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The Exact Match Entry Rule cannot be defined before a suitable "
                                                    "entry is defined for expandedActionIndex in the Profile Expander Table\n");
    }
     /* Check we are assigning the correct expandedActionIndex according to actionType */
    if(actionType!=PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].actionType)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The Exact Match Entry actionType do not match the actionType "
                                                    " defined for expandedActionIndex in the Profile Expander Table\n");
    }

    /* check validity of Exact Match Entry index */
    rc = prvCpssDxChExactMatchIndexValidity(devNum,index,entryPtr->key.keySize);
    if (rc  != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "illegal Exact Match Index \n");
    }

    /* Check that the Exact Match Action correspond with the
       Action defined in the Profile Expander Table */
    rc = prvCpssDxChExactMatchActionValidity(devNum,expandedActionIndex,actionType,actionPtr,exactMatchHwActionArray);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* build the reduce entry to set into HW*/
    rc = prvCpssDxChExactMatchReducedToHwformat(devNum,expandedActionIndex,exactMatchHwActionArray,reducedActionHwDataArr);
    if (rc  != GT_OK)
    {
        return rc;
    }

    /* build entry hw format key + ReducedAction */
    rc = prvDxChExactMatchEntryToHwformat(devNum,expandedActionIndex,entryPtr,reducedActionHwDataArr,hwDataArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, (entryPtr->key.keySize));

        for (bankNum=0; bankNum<numOfBanks; bankNum++)
        {
            startWord = bankNum*4;
            entryIndexPerBank = index + bankNum;

            cpssOsMemCpy(singleHwDataArr, &hwDataArr[startWord], sizeof(singleHwDataArr));

            /* write entry to specific index format */
            rc = prvCpssDxChPortGroupWriteTableEntry(devNum,portGroupId,
                                            CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                            entryIndexPerBank,
                                            &singleHwDataArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return rc;
}

/**
* @internal cpssDxChExactMatchPortGroupEntrySet function
* @endinternal
*
* @brief   Sets the exact match entry and its action
*
*   NOTE: this API should be called when there is a valid entry
*   in expandedActionProfile in the Profile Expander Table
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] index        - entry index in the exact match table
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] expandedActionIndex-Exact Match profile identifier
*                     (APPLICABLE RANGES:1..15)
* @param[in] entryPtr      - (pointer to)Exact Match entry
* @param[in] actionType    - Exact Match Action Type(TTI or PCL)
* @param[in] actionPtr     - (pointer to)Exact Match Action (TTI
*                            Action or PCL Action)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPortGroupEntrySet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    IN GT_U32                                   expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC          *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchPortGroupEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, expandedActionIndex,
                            entryPtr,actionType,actionPtr));

    rc = internal_cpssDxChExactMatchPortGroupEntrySet(devNum, portGroupsBmp, index,
                                                      expandedActionIndex,entryPtr,actionType,actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, portGroupsBmp, index, expandedActionIndex,
                            entryPtr,actionType,actionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchPortGroupEntryGet
*           function
* @endinternal
*
* @brief   Gets the exact match entry and its action
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                 NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] index - entry index in the exact match table
* @param[out]validPtr   - (pointer to) is the entry valid or not
* @param[out] actionType - Exact Match Action Type(TTI or PCL)
* @param[out] actionPtr - (pointer to)Exact Match Action
*                         (TTI Action or PCL Action)
* @param[out] entryPtr  - (pointer to)Exact Match entry
* @param[out] expandedActionIndexPtr-(pointer to)Exact Match
*             profile identifier (APPLICABLE RANGES:1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchPortGroupEntryGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    OUT GT_BOOL                                 *validPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                  *expandedActionIndexPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U8     reducedActionHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS];/* the final 8 bytes to set the HW */
    GT_U32    exactMatchReducedHwArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* 8 words - 256 bits of reduced data */

    GT_U32    hwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_MAX_SIZE_CNS];/* hw data to set entryType/Age/keySize/lookupNum/key/reduccedAction/ */
    GT_U32    singleHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* data for one bank - 115 bits */
    GT_U32    startWord, entryIndexPerBank;
    GT_U32    bankNum=0,numOfBanks=0;

    GT_U32    exactMatchHwActionArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* the full Hw representation for actionPtr */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_NULL_PTR_CHECK_MAC(actionTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    CPSS_NULL_PTR_CHECK_MAC(expandedActionIndexPtr);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

    cpssOsMemSet(reducedActionHwDataArr, 0, sizeof(reducedActionHwDataArr));
    cpssOsMemSet(exactMatchReducedHwArray, 0, sizeof(exactMatchReducedHwArray));
    cpssOsMemSet(exactMatchHwActionArray, 0, sizeof(exactMatchHwActionArray));
    cpssOsMemSet(hwDataArr, 0, sizeof(hwDataArr));
    cpssOsMemSet(singleHwDataArr, 0, sizeof(singleHwDataArr));

    cpssOsMemSet(actionPtr, 0, sizeof(actionPtr));
    cpssOsMemSet(entryPtr, 0, sizeof(entryPtr));
    cpssOsMemSet(entryPtr->key.pattern, 0, sizeof(entryPtr->key.pattern));

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC(devNum,index);

    /* Read single bank of specific index */
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                    CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                    index,
                                    &singleHwDataArr[0]);
    if(rc != GT_OK)
    {
        return rc;
    }
    cpssOsMemCpy(&hwDataArr[0], singleHwDataArr, sizeof(singleHwDataArr));

    /* build part of the entry SW format ReducedAction, keySize and expandedActionIndexPtr */
    rc = prvDxChExactMatchEntryToSwformat(devNum,hwDataArr,GT_FALSE,validPtr,entryPtr,reducedActionHwDataArr,expandedActionIndexPtr);
    if((rc != GT_OK)||(*validPtr==GT_FALSE))
    {
        return rc;
    }

    /* this API should be called when there is a valid entry
       in expandedActionProfile in the Profile Expander Table*/
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[(*expandedActionIndexPtr)].expandedActionValid==GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "The Exact Match Entry Rule cannot be read before a suitable "
                                                    "entry is defined for expandedActionIndex in the Profile Expander Table\n");
    }
    else
    {
        *actionTypePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[(*expandedActionIndexPtr)].actionType;
    }


    PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, (entryPtr->key.keySize));

    /* first bank was already treated, start from second bank */
    for (bankNum=1; bankNum<numOfBanks; bankNum++)
    {
        startWord = bankNum*4;
        entryIndexPerBank = index + bankNum;

        /* Read entry to specific index format */
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                        CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                        entryIndexPerBank,
                                        &singleHwDataArr[0]);
        if(rc != GT_OK)
        {
            return rc;
        }
        cpssOsMemCpy(&hwDataArr[startWord], singleHwDataArr, sizeof(singleHwDataArr));
    }

   /* build entry SW format key + ReducedAction */
    rc = prvDxChExactMatchEntryToSwformat(devNum,hwDataArr,GT_TRUE,validPtr,entryPtr,reducedActionHwDataArr,expandedActionIndexPtr);
    if((rc != GT_OK)||(*validPtr==GT_FALSE))
    {
        return rc;
    }

    /* build the reduce entry set into HW*/
    rc = prvCpssDxChExactMatchReducedToSWformat(devNum,*expandedActionIndexPtr,*actionTypePtr,reducedActionHwDataArr,exactMatchHwActionArray);
    if (rc  != GT_OK)
    {
        return rc;
    }

    /* translate exactMatchHwActionArray to SW Action entry */

    switch (*actionTypePtr)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        /* convert TTI action to SW format */
        rc = prvCpssDxChTtiActionType2Hw2LogicFormat(devNum,
                                                     exactMatchHwActionArray,
                                                     &(actionPtr->ttiAction));

        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        /* convert PCL action to SW format */
        rc = prvCpssDxChPclTcamRuleActionHw2SwConvert(devNum,
                                                      CPSS_PCL_DIRECTION_INGRESS_E,
                                                      exactMatchHwActionArray,
                                                      &(actionPtr->pclAction));
        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        /* convert EPCL action to SW format */
        rc = prvCpssDxChPclTcamRuleActionHw2SwConvert(devNum,
                                                      CPSS_PCL_DIRECTION_EGRESS_E,
                                                      exactMatchHwActionArray,
                                                      &(actionPtr->pclAction));
        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChExactMatchPortGroupEntryGet function
* @endinternal
*
* @brief   Gets the exact match entry and its action
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                 NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] index - entry index in the exact match table
* @param[out]validPtr   - (pointer to) is the entry valid or not
* @param[out] actionType - Exact Match Action Type(TTI or PCL)
* @param[out] actionPtr - (pointer to)Exact Match Action
*                         (TTI Action or PCL Action)
* @param[out] entryPtr  - (pointer to)Exact Match entry
* @param[out] expandedActionIndexPtr-(pointer to)Exact Match
*             profile identifier (APPLICABLE RANGES:1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPortGroupEntryGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    OUT GT_BOOL                                 *validPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                  *expandedActionIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchPortGroupEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index,
                            validPtr,actionTypePtr,actionPtr,entryPtr,expandedActionIndexPtr));

    rc = internal_cpssDxChExactMatchPortGroupEntryGet(devNum, portGroupsBmp, index,
                                  validPtr,actionTypePtr,actionPtr,entryPtr,expandedActionIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, portGroupsBmp, index,
                            validPtr,actionTypePtr,actionPtr,entryPtr,expandedActionIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchPortGroupEntryInvalidate
*           function
* @endinternal
*
* @brief   Invalidate the exact match entry
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index - entry index in the exact match table
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChExactMatchPortGroupEntryInvalidate
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32    bankNum=0,numOfBanks=0,entryIndexPerBank=0,value=0;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT       keySize;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

    PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC(devNum,index);

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
         /* read valid bit, entryType bit, keySize  */
        rc = prvCpssDxChPortGroupReadTableEntryField(devNum,portGroupId,
                                     CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                     index,
                                     0, /* fieldWordNum */
                                     0, /* fieldOffset  */
                                     5, /* fieldLength  */
                                     &value);/* fieldValue   */
        if(rc != GT_OK)
        {
            return rc;
        }

        /* check if the entry is NOT valid - nothing to do, entry is already invalid */
        if ((value&0x1)==0)
        {
            return GT_OK;
        }
        /* check if the entry type is NOT Key+Action - error*/
        if (((value>>1)&0x1)==1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "Can not invalidate the entry: illegal HW entryType or entry not valid\n");
        }

        /* get the keySize in the HW */
        PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_HW_TO_KEY_SIZE_VAL_MAC(keySize, ((value>>3)&0x3));

        /* check validity of Exact Match Entry index */
        rc = prvCpssDxChExactMatchIndexValidity(devNum,index,keySize);
        if (rc  != GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal Exact Match Index \n");
        }

        /* delete banks in reverse order */
        PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, keySize);


        for (bankNum = numOfBanks; bankNum > 0; bankNum--)
        {
            entryIndexPerBank = index + (bankNum-1);

            /* write 0 to valid bit in each bank */
            rc = prvCpssDxChPortGroupWriteTableEntryField(devNum,portGroupId,
                                         CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                         entryIndexPerBank,
                                         0, /* fieldWordNum */
                                         0, /* fieldOffset - valid bit 0*/
                                         1, /* fieldLength  */
                                         0);/* fieldValue   */
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return rc;
}

/**
* @internal cpssDxChExactMatchPortGroupEntryInvalidate function
* @endinternal
*
* @brief   Invalidate the exact match entry
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index - entry index in the exact match table
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchPortGroupEntryInvalidate
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN GT_U32                               index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchPortGroupEntryInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index));

    rc = internal_cpssDxChExactMatchPortGroupEntryInvalidate(devNum, portGroupsBmp, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, portGroupsBmp, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchHashCrcMultiResultsByParamsCalc function
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
static GT_STATUS internal_cpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_STC   *entryKeyPtr,
    OUT GT_U32                          *numberOfElemInCrcMultiHashArrPtr,
    OUT GT_U32                          exactMatchCrcMultiHashArr[]
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(entryKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(numberOfElemInCrcMultiHashArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(exactMatchCrcMultiHashArr);

    return  prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,
                                                                 entryKeyPtr,
                                                                 numberOfElemInCrcMultiHashArrPtr,
                                                                 exactMatchCrcMultiHashArr);
}

/**
* @internal cpssDxChExactMatchHashCrcMultiResultsByParamsCalc function
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
*
* The output of the function is indexes that should be
* used for setting the Exact Match entry with cpssDxChExactMatchPortGroupEntrySet.
* Entry with Key size 5B  occupy 1 entry
* Entry with Key size 19B occupy 2 entries
* Entry with Key size 33B occupy 3 entries
* Entry with Key size 47B occupy 4 entries
*
* The logic is as follow according to the key size and bank number.
* keySize   exactMatchSize    function output
* 5 bytes     4 banks         x,y,z,w      (4 separate indexes)
* 19 bytes    4 banks         x,x+1,y,y+1  (2 indexes to be used in pairs)
* 33 bytes    4 banks         x,x+1,x+2    (1 indexes to be used for a single entry)
* 47 bytes    4 banks         x,x+1,x+2,x+3(1 indexes to be used for a single entry)
*
* keySize   exactMatchSize    function output
* 5 bytes     8 banks         x,y,z,w,a,b,c,d            (8 separate indexes)
* 19 bytes    8 banks         x,x+1,y,y+1,z,z+1,w,w+1    (4 indexes to be used in pairs)
* 33 bytes    8 banks         x,x+1,x+2,y,y+1,y+2        (2 indexes to be used for a single entry)
* 47 bytes    8 banks         x,x+1,x+2,x+3,y,y+1,y+2,y+3(2 indexes to be used for a single entry)
*
* and so on for 16 banks
*
* The cpssDxChExactMatchPortGroupEntrySet should get only the first index to be used in the API
* The API set the consecutive indexes according to the key size
*
* keySize   exactMatchSize    function input
* 5 bytes     4 banks         x or y or z or w(4 separate indexes options)
* 19 bytes    4 banks         x or y          (2 indexes options)
* 33 bytes    4 banks         x               (1 indexes option)
* 47 bytes    4 banks         x               (1 indexes option)
*
* keySize   exactMatchSize    function output
* 5 bytes     8 banks         x or y or z or w or a or b or c or d(8 separate indexes options)
* 19 bytes    8 banks         x or y or z or w(4 indexes options)
* 33 bytes    8 banks         x or y          (2 indexes options)
* 47 bytes    8 banks         x or y          (2 indexes options)
*
*/
GT_STATUS cpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_STC   *entryKeyPtr,
    OUT GT_U32                          *numberOfElemInCrcMultiHashArrPtr,
    OUT GT_U32                          exactMatchCrcMultiHashArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchHashCrcMultiResultsByParamsCalc);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryKeyPtr,numberOfElemInCrcMultiHashArrPtr,exactMatchCrcMultiHashArr));

    rc = internal_cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, entryKeyPtr,
                                                                    numberOfElemInCrcMultiHashArrPtr,
                                                                    exactMatchCrcMultiHashArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum,entryKeyPtr,numberOfElemInCrcMultiHashArrPtr,exactMatchCrcMultiHashArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvUtilCpssDxChExactMatchIsValid function
* @endinternal
*
* @brief   Return validity of exact match entry
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index - entry index in the exact match table
* @param[out] isValidPtr - (pointer to) validity state
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvUtilCpssDxChExactMatchIsValid
(
    IN  GT_U8                                    devNum,
    IN  GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN  GT_U32                                   index,
    OUT GT_BOOL                                  *isValidPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32    bankNum=0,numOfBanks=0,entryIndexPerBank=0,value=0;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT       keySize;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

    PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC(devNum,index);

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        /* read valid bit, entryType bit, keySize  */
        rc = prvCpssDxChPortGroupReadTableEntryField(devNum,portGroupId,
                                     CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                     index,
                                     0, /* fieldWordNum */
                                     0, /* fieldOffset  */
                                     5, /* fieldLength  */
                                     &value);/* fieldValue   */
        if(rc != GT_OK)
        {
            *isValidPtr=GT_FALSE;
            return rc;
        }

        /* check if the entry is NOT valid - nothing to do, entry is already invalid */
        if ((value&0x1)==0)
        {
            *isValidPtr=GT_FALSE;
            return GT_OK;
        }
        /* check if the entry type is NOT Key+Action - error*/
        if (((value>>1)&0x1)==1)
        {
            *isValidPtr=GT_FALSE;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "Can not invalidate the entry: illegal HW entryType or entry not valid\n");
        }

        /* get the keySize in the HW */
        PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_HW_TO_KEY_SIZE_VAL_MAC(keySize, ((value>>3)&0x3));

        /* check validity of Exact Match Entry index */
        rc = prvCpssDxChExactMatchIndexValidity(devNum,index,keySize);
        if (rc  != GT_OK)
        {
            *isValidPtr=GT_FALSE;
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal Exact Match Index \n");
        }

        /* check all indexes related to the entry are also valid (no need to check first bank) */
        PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, keySize);


        for (bankNum = numOfBanks; bankNum > 1; bankNum--)
        {
            entryIndexPerBank = index + (bankNum-1);

            /* read valid bit, entryType bit  */
            rc = prvCpssDxChPortGroupReadTableEntryField(devNum,portGroupId,
                                     CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                     entryIndexPerBank,
                                     0, /* fieldWordNum */
                                     0, /* fieldOffset  */
                                     2, /* fieldLength  */
                                     &value);/* fieldValue   */

            if(rc != GT_OK)
            {
                *isValidPtr=GT_FALSE;
                return rc;
            }
            /* check if the entry type is NOT Key only - error*/
            if (((value>>1)&0x1)!=1)
            {
                *isValidPtr=GT_FALSE;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "Can not invalidate the entry: illegal HW entryType or entry not valid\n");
            }

        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    *isValidPtr=GT_TRUE;
    return rc;
}

/**
* @internal prvUtilCpssDxChExactMatchRuleGet function
* @endinternal
*
* @brief   Gets exact match entry and its action in HW format
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum       - device number
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                 NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] index - entry index in the exact match table
* @param[out]validPtr   - (pointer to) is the entry valid or not
* @param[out] actionType - Exact Match Action Type(TTI or PCL)
* @param[out] actionPtr - (pointer to)Exact Match HW Action
* @param[out] reducedActionPtr - (pointer to)Exact Match Reduced
*                                HW Action
* @param[out] entryPtr  - (pointer to)Exact Match entry
* @param[out] expandedActionIndexPtr-(pointer to)Exact Match
*             profile identifier (APPLICABLE RANGES:1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS prvUtilCpssDxChExactMatchRuleGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    OUT GT_BOOL                                 *validPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT GT_U32                                  *actionPtr,
    OUT GT_U8                                   *reducedActionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                  *expandedActionIndexPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U8     reducedActionHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS];/* the final 8 bytes to set the HW */
    GT_U32    exactMatchReducedHwArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* 8 words - 256 bits of reduced data */

    GT_U32    hwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_MAX_SIZE_CNS];/* hw data to set entryType/Age/keySize/lookupNum/key/reduccedAction/ */
    GT_U32    singleHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* data for one bank - 115 bits */
    GT_U32    startWord, entryIndexPerBank;
    GT_U32    bankNum=0,numOfBanks=0;

    GT_U32    exactMatchHwActionArray[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* the full Hw representation for actionPtr */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_NULL_PTR_CHECK_MAC(actionTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    CPSS_NULL_PTR_CHECK_MAC(expandedActionIndexPtr);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

    cpssOsMemSet(reducedActionHwDataArr, 0, sizeof(reducedActionHwDataArr));
    cpssOsMemSet(exactMatchReducedHwArray, 0, sizeof(exactMatchReducedHwArray));
    cpssOsMemSet(exactMatchHwActionArray, 0, sizeof(exactMatchHwActionArray));
    cpssOsMemSet(hwDataArr, 0, sizeof(hwDataArr));
    cpssOsMemSet(singleHwDataArr, 0, sizeof(singleHwDataArr));

    cpssOsMemSet(actionPtr, 0, sizeof(actionPtr));
    cpssOsMemSet(entryPtr, 0, sizeof(entryPtr));
    cpssOsMemSet(entryPtr->key.pattern, 0, sizeof(entryPtr->key.pattern));

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC(devNum,index);

    /* Read single bank of specific index */
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                    CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                    index,
                                    &singleHwDataArr[0]);
    if(rc != GT_OK)
    {
        return rc;
    }
    cpssOsMemCpy(&hwDataArr[0], singleHwDataArr, sizeof(singleHwDataArr));

    /* build part of the entry SW format ReducedAction, keySize and expandedActionIndexPtr */
    rc = prvDxChExactMatchEntryToSwformat(devNum,hwDataArr,GT_FALSE,validPtr,entryPtr,reducedActionHwDataArr,expandedActionIndexPtr);
    if((rc != GT_OK)||(*validPtr==GT_FALSE))
    {
        return rc;
    }

    /* this API should be called when there is a valid entry
       in expandedActionProfile in the Profile Expander Table*/
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[(*expandedActionIndexPtr)].expandedActionValid==GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "The Exact Match Entry Rule cannot be read before a suitable "
                                                    "entry is defined for expandedActionIndex in the Profile Expander Table\n");
    }
    else
    {
        *actionTypePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[(*expandedActionIndexPtr)].actionType;
    }


    PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, (entryPtr->key.keySize));

    /* first bank was already treated, start from second bank */
    for (bankNum=1; bankNum<numOfBanks; bankNum++)
    {
        startWord = bankNum*4;
        entryIndexPerBank = index + bankNum;

        /* Read entry to specific index format */
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                        CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                        entryIndexPerBank,
                                        &singleHwDataArr[0]);
        if(rc != GT_OK)
        {
            return rc;
        }
        cpssOsMemCpy(&hwDataArr[startWord], singleHwDataArr, sizeof(singleHwDataArr));
    }

   /* build entry SW format key + ReducedAction */
    rc = prvDxChExactMatchEntryToSwformat(devNum,hwDataArr,GT_TRUE,validPtr,entryPtr,reducedActionHwDataArr,expandedActionIndexPtr);
    if((rc != GT_OK)||(*validPtr==GT_FALSE))
    {
        return rc;
    }

    /* build the reduce entry set into HW*/
    rc = prvCpssDxChExactMatchReducedToSWformat(devNum,*expandedActionIndexPtr,*actionTypePtr,reducedActionHwDataArr,exactMatchHwActionArray);
    if (rc  != GT_OK)
    {
        return rc;
    }

    cpssOsMemCpy(actionPtr, exactMatchHwActionArray, sizeof(exactMatchHwActionArray));
    cpssOsMemCpy(reducedActionPtr, reducedActionHwDataArr, sizeof(reducedActionHwDataArr));

    return rc;
}
/**
* @internal prvUtilCpssDxChExactMatchRuleDump function
* @endinternal
*
* @brief   Dump Exact Match rule
*
* @note     APPLICABLE DEVICES:  Falcon.
*
* @note     NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] exactMatchIndex   - index of rule
* @param[in] lookupNum         - lookup of rule
* @param[in] exactMatchPatternSize- key size of the rule pattern
* @param[in] patternArr[]      - key pattern of the rule
* @param[in] exactMatchKeySize - key size of the rule mask
* @param[in] keyStart          - Start of Exact Match Search
*                               (Byte offset from the TCAM key)
* @param[in] maskArr[]         - Mask the bits in the Exact
*                                Match Search Key
* @param[in] actionType        - action type
* @param[in] actionArr[]       - (pointer to) action words array
* @param[in] reducedActionArr[]- (pointer to) reduced action
*                                 bytes array
* @param[in] expandedActionIndex - index of the expanded action
*                                  configured on this rule
*
* @retval None
*/
static void prvUtilCpssDxChExactMatchRuleDump
(
    IN GT_U32                                   ruleIndex,
    IN CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT         lookupNum,
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT       exactMatchPatternSize,
    IN GT_U8                                    patternArr[],
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT       exactMatchKeySize,
    IN GT_U32                                   keyStart,
    IN GT_U8                                    maskArr[],
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN GT_U32                                   actionArr[],
    IN GT_U8                                    reducedActionArr[],
    IN GT_U32                                   expandedActionIndex
)
{
    static const GT_U32 nextLineBytes  = 24;
    GT_U32 bytesNum=0;
    GT_U32 maskBytesNum=0;
    GT_U32 i;

    /* strings with the same printed length */
    static char rule_ind_Hdr[]= "\n#Rule Index= 0x%X , #Rule lookup= %d, #Rule Key Size= %s";
    static char exp_act_ind_type_Hdr[]="\n#Expanded Action Index= %d, #Action Type= %s";
    static char key_params_Hdr[]="\n#Mask Key Size= %s, #Mask Key Start= %d";
    static char maskHdr[]   = "\n       MASK           ";
    static char pattHdr[]   = "\n       PATTERN        ";
    static char actnHdr[]   = "\n       ACTION         ";
    static char redActnHdr[]= "\n       REDUCED_ACTION ";
    static char lineBreak[] = "\n                      ";

    switch(exactMatchPatternSize)
    {
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        bytesNum=5;
        /* Rule Index, Rule lookup, Rule Key Size, keySize*/
        cpssOsPrintf(rule_ind_Hdr, ruleIndex,lookupNum, "5_BYTES");
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        bytesNum=19;
        cpssOsPrintf(rule_ind_Hdr, ruleIndex,lookupNum, "19_BYTES");
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        bytesNum=33;
        cpssOsPrintf(rule_ind_Hdr, ruleIndex,lookupNum, "33_BYTES");
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
        bytesNum=47;
        cpssOsPrintf(rule_ind_Hdr, ruleIndex,lookupNum, "47_BYTES");
        break;
    default:
        cpssOsPrintf(exp_act_ind_type_Hdr, expandedActionIndex,"ERROR");
        return;
    }
    switch (actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        /* Expanded Action Index and type */
        cpssOsPrintf(exp_act_ind_type_Hdr, expandedActionIndex,"TTI");
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        /* Expanded Action Index and type */
        cpssOsPrintf(exp_act_ind_type_Hdr, expandedActionIndex,"PCL");
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        /* Expanded Action Index and type */
        cpssOsPrintf(exp_act_ind_type_Hdr, expandedActionIndex,"EPCL");
        break;
    default:
        cpssOsPrintf(exp_act_ind_type_Hdr, expandedActionIndex,"ERROR");
        return;
    }

    switch(exactMatchKeySize)
    {
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        maskBytesNum=5;
        cpssOsPrintf(key_params_Hdr, "5_BYTES",keyStart);
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        maskBytesNum=19;
        cpssOsPrintf(key_params_Hdr, "19_BYTES",keyStart);
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        maskBytesNum=33;
        cpssOsPrintf(key_params_Hdr,"33_BYTES",keyStart);
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
        maskBytesNum=47;
        cpssOsPrintf(key_params_Hdr, "47_BYTES",keyStart);
        break;
    default:
        cpssOsPrintf(key_params_Hdr, "ERROR");
        return;
    }

    /* mask */
    cpssOsPrintf(maskHdr);
    for (i = 0; (i < maskBytesNum); i++)
    {
        if (((i % nextLineBytes) == 0)&&(i!=0))
        {
            cpssOsPrintf(lineBreak);
        }
        cpssOsPrintf("%02X ", maskArr[i]);
    }

   /* pattern */
    cpssOsPrintf(pattHdr);
    for (i = 0; (i < bytesNum); i++)
    {
        if (((i % nextLineBytes) == 0)&&(i!=0))
        {
            cpssOsPrintf(lineBreak);
        }
        cpssOsPrintf("%02X ", patternArr[i]);
    }

    /* action */
    cpssOsPrintf(actnHdr);
    for (i = 0; (i < PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS); i++)
    {
        cpssOsPrintf("%08X ", actionArr[i]);

    }
    /* Reduced Action */
    cpssOsPrintf(redActnHdr);
    for (i = 0; (i < PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS); i++)
    {
        /*cpssOsPrintf("%08X ", actionArr[i]);*/
        cpssOsPrintf("%X ", reducedActionArr[i]);
    }

    cpssOsPrintf("\n");
}


/**
* @internal internal_cpssDxChExactMatchRulesDump function
* @endinternal
*
* @brief    Debug API - Dump all valid Exact Match rules
*
* @note     APPLICABLE DEVICES:  Falcon.
*
* @note     NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum           - device number
* @param[in] portGroupsBmp    - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                          bitmap must be set with at least one bit representing
*                                          valid port group(s). If a bit of non valid port group
*                                          is set then function returns GT_BAD_PARAM.
*                                          value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] startIndex       - index of first rule
* @param[in] numOfRules       - number of rules to scan and dump
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChExactMatchRulesDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             startIndex,
    IN GT_U32                             numOfRules
)
{
    GT_STATUS                                       rc=GT_OK;
    GT_U32                                          exactMatchIndex;
    GT_BOOL                                         isValid;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    GT_U32                                          actionArr[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];/* the full Hw representation for actionPtr */
    GT_U8                                           reducedActionArr[PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS];/* the final 8 bytes to set the HW */
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC                 exactMatchEntry;
    GT_U32                                          expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;
    GT_U32                                          numberOfEntriesCounter=0;

      /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E|CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);
    PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC(devNum,startIndex);

    for (exactMatchIndex = startIndex; exactMatchIndex <= (startIndex + numOfRules); exactMatchIndex++)
    {
        /* get Exact Match Entry validity */
        rc = prvUtilCpssDxChExactMatchIsValid(devNum,portGroupsBmp,exactMatchIndex,&isValid);
        if (rc==GT_OUT_OF_RANGE)
        {
            cpssOsPrintf("prvUtilCpssDxChExactMatchIsValid error: GT_OUT_OF_RANGE index 0x%X rc %d\n", exactMatchIndex, rc);
            return rc;
        }
        if ((rc != GT_OK) || (isValid == GT_FALSE))
        {
            continue;
        }

        cpssOsMemSet(actionArr,0,sizeof(actionArr));
        cpssOsMemSet(reducedActionArr,0,sizeof(reducedActionArr));
        cpssOsMemSet(exactMatchEntry.key.pattern,0,sizeof(exactMatchEntry.key.pattern));

        rc = prvUtilCpssDxChExactMatchRuleGet(devNum,portGroupsBmp,exactMatchIndex,&isValid,
                                              &actionType,actionArr,reducedActionArr,
                                              &exactMatchEntry,&expandedActionIndex);

        if (rc != GT_OK)
        {
            cpssOsPrintf("prvUtilCpssDxChExactMatchRuleGet error: index 0x%X rc %d\n", exactMatchIndex, rc);
            return rc;
        }

        rc = cpssDxChExactMatchProfileKeyParamsGet(devNum,expandedActionIndex,&keyParams);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChExactMatchProfileKeyParamsGet error: index %d rc %d\n", expandedActionIndex, rc);
            return rc;
        }


        /* dump entry */
        numberOfEntriesCounter++;
        cpssOsPrintf("\n### Entry Number=%d ###",numberOfEntriesCounter);
        prvUtilCpssDxChExactMatchRuleDump(exactMatchIndex,
                                          exactMatchEntry.lookupNum,
                                          exactMatchEntry.key.keySize,
                                          exactMatchEntry.key.pattern,
                                          keyParams.keySize,
                                          keyParams.keyStart,
                                          keyParams.mask,
                                          actionType,
                                          actionArr,
                                          reducedActionArr,
                                          expandedActionIndex);
    }

    cpssOsPrintf("\n### Total Number of Exact Match Entries=%d ###\n",numberOfEntriesCounter);
    return GT_OK;
}

/**
* @internal cpssDxChExactMatchRulesDump function
* @endinternal
*
* @brief    Debug API - Dump all valid Exact Match rules
*
* @note     APPLICABLE DEVICES:  Falcon.
*
* @note     NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum           - device number
* @param[in] portGroupsBmp    - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                          bitmap must be set with at least one bit representing
*                                          valid port group(s). If a bit of non valid port group
*                                          is set then function returns GT_BAD_PARAM.
*                                          value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] startIndex       - index of first rule
* @param[in] numOfRules       - number of rules to scan and dump
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChExactMatchRulesDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             startIndex,
    IN GT_U32                             numOfRules
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchRulesDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, startIndex, numOfRules));

    rc = internal_cpssDxChExactMatchRulesDump(devNum, portGroupsBmp, startIndex, numOfRules);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, startIndex, numOfRules));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


