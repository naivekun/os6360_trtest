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
* @file prvCpssDxChLpmTcamMgm.c
*
* @brief This file includes Rules alloction manegment for TCAM IP routing
*
* @version   24
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamMgm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamPatTrie.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamTypes.h>

#define DEBUG_IP_MGM_TRIE

#undef DEBUG_IP_MGM_TRIE

#ifdef DEBUG_IP_MGM_TRIE
#define DEBUG_IP_MGM_TRIE(x) cpssOsPrintf x
#else
#define DEBUG_IP_MGM_TRIE(x)
#endif

/**
* @internal prvCpssDxChLpmTcamMgmGetRuleIdx function
* @endinternal
*
* @brief   returns the rule index in the rules array
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] allocNodePtr             - the rule to get the idx for.
*
* @param[out] ruleIdxPtr               - the rule idx.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmGetRuleIdx
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocNodePtr,
    OUT GT_32                                  *ruleIdxPtr
)
{

    GT_UINTPTR id;
    if (allocNodePtr >= pIpTcamPoolPtr->coreIpTcamRulesAllocArray)
    {
        id = ((GT_UINTPTR)(allocNodePtr) -
              (GT_UINTPTR)(pIpTcamPoolPtr->coreIpTcamRulesAllocArray)) /
              sizeof(PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC);
        if (id < pIpTcamPoolPtr->coreIpNumOfIpTcamRules)
        {
            *ruleIdxPtr = (GT_32)id;
            DEBUG_IP_MGM_TRIE(("Found rule = %d at index = %d\n", allocNodePtr->offset, *ruleIdxPtr));
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChLpmTcamMgmGetRuleByRuleIdx function
* @endinternal
*
* @brief   returns the rule node according to the internal rule index
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] ruleIdx                  - the rule idx.
*
* @param[out] ruleNodePtrPtr           - the rule node of this idx.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmGetRuleByRuleIdx
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN  GT_32                                  ruleIdx,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   **ruleNodePtrPtr
)
{
    if (pIpTcamPoolPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* check boundries */
    if ((ruleIdx < 0) ||
        ((GT_U32)ruleIdx >= pIpTcamPoolPtr->coreIpNumOfIpTcamRules))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* get the rule */
    *ruleNodePtrPtr = &pIpTcamPoolPtr->coreIpTcamRulesAllocArray[ruleIdx];
    DEBUG_IP_MGM_TRIE(("Found index = %d -> rule = %d\n", ruleIdx, (*ruleNodePtrPtr)->offset));
    return GT_OK;
}

/*******************************************************************************
* prvCpssDxChLpmTcamMgmCreate
*
* DESCRIPTION:
*       Creates a new IP TCAM rules pool manager
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       numOfIpTcamRules            - the number of ip Tcam rules.
*       segmentsArrayPtr            - the array of index segments that
*                                     represents the physical indexes of the
*                                     rules in this pool.
*       numOfSegments               - the number of segments
*       redirectSegmentsArrayPtr    - If != NULL, redirect info for policy-based
*                                     routing. Must be the same size as segmentsArray.
*       numOfRedirectSegments       - the number of redirect segments.
*       interConnectedPoolPtrPtr    - another interconnected pool pointer that
*                                     shared the rules with this pool in some
*                                     way, the way to convert the indexes is
*                                     using the interConnectedPoolIndexConFun
*                                     if = NULL then there is no inter connected
*                                     pool.
*       interConnectedPoolIndexConFun - the inter connected pool index convert
*                                       function.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       the pool pointer if succeeded,
*       NULL otherwise.
*
* COMMENTS:
*       numOfIpTcamRules != 0!
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC* prvCpssDxChLpmTcamMgmCreate
(
    IN GT_U32                                               numOfIpTcamRules,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC      *segmentsArrayPtr,
    IN GT_U32                                               numOfSegments,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC      *redirectSegmentsArrayPtr,
    IN GT_U32                                               numOfRedirectSegments,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC                  **interConnectedPoolPtrPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_INDEX_CONVERT_FUN    interConnectedPoolIndexConFun
)
{
    GT_32                                           i;
    GT_U32                                          currIdx,segIdx;
    PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC             *pPool;
    PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC *currSeg;
    PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC *currRedirectSeg;
    GT_U32                                          currRedirectIdx = 0,redirectSegIdx;
    if (numOfIpTcamRules == 0)
    {
        return NULL;
    }

    pPool = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC));
    if (pPool == NULL)
    {
        return NULL;
    }
    pPool->coreIpTcamRulesAllocArray = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC)*
                                        numOfIpTcamRules);
    pPool->coreIpNumOfIpTcamRules = numOfIpTcamRules;
    pPool->coreIpTcamBitMapSize = ((numOfIpTcamRules-1) / 32)+1;
    pPool->coreIpTcamRulesAllocBitMap = cpssOsMalloc(sizeof(GT_U32)*(pPool->coreIpTcamBitMapSize +1));

    cpssOsMemSet(pPool->coreIpTcamRulesAllocBitMap,0,sizeof(GT_U32)*(pPool->coreIpTcamBitMapSize));

    /* set as allocated all the non valid ones in the last GT_U32 of the bitmap*/
    for (i = 31; i >= (GT_32)(numOfIpTcamRules%32); i--)
    {
       pPool->coreIpTcamRulesAllocBitMap[numOfIpTcamRules/32] |= 1<<i;
    }

    /* now set the indexes according the given segments */
    currSeg         = segmentsArrayPtr;
    currRedirectSeg = redirectSegmentsArrayPtr;
    currIdx         = currSeg->startIndex;
    segIdx          = 0;
    redirectSegIdx  = 0;
    if (currRedirectSeg != NULL)
    {
        currRedirectIdx = currRedirectSeg->startIndex;
    }

    /* find first non-empty segment */
    while (currSeg->numOfIndexes == 0)
    {
        currSeg++;
        currIdx = currSeg->startIndex;
        segIdx++;
    }
    /* find first non-empty redirect segment */
    if (currRedirectSeg != NULL)
    {
        while (currRedirectSeg->numOfIndexes == 0)
        {
            redirectSegIdx++;
            currRedirectSeg++;
            currRedirectIdx = currRedirectSeg->startIndex;
        }
    }

    DEBUG_IP_MGM_TRIE(("Creating pool: numOFrules = %d\n",numOfIpTcamRules));
    DEBUG_IP_MGM_TRIE(("Creating pool: segment [start = %d end = %d]\n",
               currSeg->startIndex, currSeg->endIndex));
    if (currRedirectSeg != NULL)
    {
        DEBUG_IP_MGM_TRIE(("Using redirect pool: segment [start = %d end = %d]\n",
                   currRedirectSeg->startIndex, currRedirectSeg->endIndex));
    }
    else
    {
        DEBUG_IP_MGM_TRIE(("No redirect pool for this segment\n"));
    }

    for (i = 0; i < (GT_32)(numOfIpTcamRules) ; i++)
    {
        /* save index in array */
        pPool->coreIpTcamRulesAllocArray[i].offset          = currIdx;
        pPool->coreIpTcamRulesAllocArray[i].valid           = GT_FALSE;
        pPool->coreIpTcamRulesAllocArray[i].trieNode        = NULL;
        pPool->coreIpTcamRulesAllocArray[i].redirectOffset  = currIdx;
        if (currRedirectSeg != NULL)
        {
            pPool->coreIpTcamRulesAllocArray[i].redirectOffset = currRedirectIdx;
        }

        currIdx++;
        if (currRedirectSeg != NULL)
        {
            currRedirectIdx++;
        }

        if (currIdx > currSeg->endIndex)
        {
            /* find next non-empty segment */
            do {
                segIdx++;
                /* check if we got segments left */
                if ((segIdx >= numOfSegments) && (i < (GT_32)(numOfIpTcamRules - 1)))
                {
                    return (NULL);
                }
                currSeg++;
                currIdx = currSeg->startIndex;

            } while ((currSeg->numOfIndexes == 0) && (segIdx < numOfSegments));

            if ((segIdx < numOfSegments) && (currSeg->numOfIndexes > 0))
            {
                DEBUG_IP_MGM_TRIE(("Creating pool: segment [start = %d end = %d]\n",
                           currSeg->startIndex, currSeg->endIndex));
                if (currRedirectSeg == NULL)
                {
                    DEBUG_IP_MGM_TRIE(("No redirect pool for this segment\n"));
                }
            }
        }

        if ((currRedirectSeg != NULL) && (currRedirectIdx > currRedirectSeg->endIndex))
        {
            /* find next non-empty redirect segment */
            do {
                redirectSegIdx++;
                /* check if we got redirect segments left */
                if ((redirectSegIdx >= numOfRedirectSegments) && (i < (GT_32)(numOfIpTcamRules - 1)))
                {
                    return (NULL);
                }
                currRedirectSeg++;
                currRedirectIdx = currRedirectSeg->startIndex;

            } while ((currRedirectSeg->numOfIndexes == 0) && (redirectSegIdx < numOfRedirectSegments));

            if ((redirectSegIdx < numOfRedirectSegments) && (currRedirectSeg->numOfIndexes > 0))
            {

                DEBUG_IP_MGM_TRIE(("Using redirect pool: segment [start = %d end = %d]\n",
                            currRedirectSeg->startIndex, currRedirectSeg->endIndex));
            }
        }
    }

    pPool->coreIpTcamNumOfAlloc = 0;

    pPool->interConnectedPoolIndexConFun = interConnectedPoolIndexConFun;
    pPool->interConnectedPoolPtr = interConnectedPoolPtrPtr;

    return pPool;
}

/**
* @internal prvCpssDxChLpmTcamMgmDestroy function
* @endinternal
*
* @brief   Destroys a IP Tcam rules pool manager
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
*
* @retval GT_OK                    - if susccessed
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmDestroy
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC     *pIpTcamPoolPtr
)
{
    if (pIpTcamPoolPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    cpssOsFree(pIpTcamPoolPtr->coreIpTcamRulesAllocArray);
    pIpTcamPoolPtr->coreIpTcamRulesAllocArray = NULL;

    cpssOsFree(pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap);
    pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap = NULL;


    pIpTcamPoolPtr->coreIpTcamBitMapSize = 0;
    pIpTcamPoolPtr->coreIpNumOfIpTcamRules = 0;
    pIpTcamPoolPtr->coreIpTcamNumOfAlloc = 0;

    cpssOsFree(pIpTcamPoolPtr);

    return GT_OK;
}

/*******************************************************************************
* gtCoreIpWrapAllocRule
*
* DESCRIPTION:
*       Wraps a alloction procedure and returns the allocated rule
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr  - the pool pointer
*       allocWordPtr    - the allocated from bitmap word.
*       allocBit        - the bit in the bitmap word which incateds the allocation
*                       index.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       the allocated node pointer.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC* gtCoreIpWrapAllocRule
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC  *pIpTcamPoolPtr,
    IN GT_U32                               *allocWordPtr,
    IN GT_U32                               allocBit
)
{
    GT_U32 offset;

    offset = (GT_U32)(allocWordPtr - pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap);
    offset *= 32;

    offset += allocBit;

    /* set the bit in the bit vector */
    *allocWordPtr |=  1 << allocBit;

    pIpTcamPoolPtr->coreIpTcamRulesAllocArray[offset].valid = GT_TRUE;

    pIpTcamPoolPtr->coreIpTcamNumOfAlloc++;

    return &(pIpTcamPoolPtr->coreIpTcamRulesAllocArray[offset]);


}

/**
* @internal ipTcamMgmAllocMid function
* @endinternal
*
* @brief   allocates a rule of the pool by near middle index allocation
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] lowerBoundPtr            - the lower bound rule to allocate between.
*                                      if = NULL the use the lowest index.
* @param[in] upperBoundPtr            - the upper bound rule to allocate between.
*                                      if = NULL the use the max index.
*
* @param[out] allocNodePtrPtr          - the allocated rule node.
*                                       GT_OK if allocation succedded.
*
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper and lower
*                                       bounds.
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range of the pool.
*/
static GT_STATUS ipTcamMgmAllocMid
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *lowerBoundPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *upperBoundPtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC  **allocNodePtrPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 upperWordMask,lowerWordMask;
    GT_32 i,realLowerBound,step,top,realUpperBound;
    GT_U32 *lowerWord, *upperWord,*startUpperWord,*startLowerWord,*chosenWord;

    GT_U32 conStartUpperWord,conStartLowerWord,conChosenWord;

    if (pIpTcamPoolPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* first convert to real indexes*/
    if (upperBoundPtr == NULL)
        realUpperBound = pIpTcamPoolPtr->coreIpNumOfIpTcamRules;
    else
    {
        rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,upperBoundPtr,&realUpperBound);
        if(rc != GT_OK)
            return rc;
    }

    if (lowerBoundPtr == NULL)
        realLowerBound = -1;
    else
    {
        rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,lowerBoundPtr,&realLowerBound);
        if(rc != GT_OK)
            return rc;
    }

    /* first check for validty */
    if (realLowerBound >= (GT_32)realUpperBound)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
    }
    if (realLowerBound >= (GT_32)pIpTcamPoolPtr->coreIpNumOfIpTcamRules)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (realLowerBound == (GT_32)(pIpTcamPoolPtr->coreIpNumOfIpTcamRules -1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
    }

    /* create the masks for the upper and lower words, to restrict for the bounds
       only */
    upperWordMask = 0xFFFFFFFF << (realUpperBound % 32);
    lowerWordMask = 0xFFFFFFFF >> (32 - ((realLowerBound+1) % 32));

    lowerWord = &(pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[realLowerBound /32]);
    upperWord = &(pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[realUpperBound /32]);
    startUpperWord = startLowerWord = lowerWord + ((upperWord - lowerWord) /2);

    /* convert the words using the mask if neccessary */
    conStartUpperWord = *startUpperWord;
    conStartUpperWord |= (startUpperWord == upperWord)? upperWordMask:0;
    conStartUpperWord |= ((startUpperWord == lowerWord)&& (realLowerBound != -1))?
        lowerWordMask:0;


    conStartLowerWord = *startLowerWord;
    conStartLowerWord |= (startLowerWord == upperWord)? upperWordMask :0;
    conStartLowerWord |= ((startLowerWord == lowerWord)&& (realLowerBound != -1))?
        lowerWordMask :0;

    /* start the search in the middle and move both ways */
    while ((conStartUpperWord == 0xFFFFFFFF) &&
           (conStartLowerWord == 0xFFFFFFFF)
           && ((startLowerWord != lowerWord) ||
               (startUpperWord != upperWord)))
    {

        /* if it's possible to move down , move */
        if (startLowerWord != lowerWord)
            startLowerWord--;

        /* convert the words using the mask if neccessary */
        conStartLowerWord = *startLowerWord;
        conStartLowerWord |= (startLowerWord == upperWord)? upperWordMask :0;
        conStartLowerWord |= ((startLowerWord == lowerWord)&& (realLowerBound != -1))?
                             lowerWordMask :0;

        /* if it's possible to move up , move */
        if (startUpperWord != upperWord)
            startUpperWord++;

        /* convert the words using the mask if neccessary */
        conStartUpperWord = *startUpperWord;
        conStartUpperWord |= (startUpperWord == upperWord)? upperWordMask:0;
        conStartUpperWord |= ((startUpperWord == lowerWord)&& (realLowerBound != -1))?
                             lowerWordMask:0;


    }

    /* if we found a lower word with a place, start with it */
    if (conStartLowerWord != 0xFFFFFFFF)
    {
        /* set up the choosing order - top buttom*/
        step = -1;
        i = 31;
        top = -1;
        /* set up the chosen word */
        chosenWord = startLowerWord;
        conChosenWord = conStartLowerWord;
    }
    else
    {
        /* set up the choosing order - buttom up*/
        step = 1;
        i = 0;
        top = 32;
        /* set up the chosen word */
        chosenWord = startUpperWord;
        conChosenWord = conStartUpperWord;
    }

    /* now decide on the bit/ empty place */
    if (conChosenWord != 0xFFFFFFFF)
    {
        while (i != top)
        {

            if ((((conChosenWord) >> i) & 0x1) == 0)
            {
                /* found a place, wrap it up and send it home */
                *allocNodePtrPtr = gtCoreIpWrapAllocRule(pIpTcamPoolPtr,chosenWord,i);
                return GT_OK;
            }
            i = i+step;

        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);

}

/**
* @internal ipTcamMgmAllocMax function
* @endinternal
*
* @brief   allocates a rule of the pool by maximum index allocation
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] lowerBoundPtr            - the lower bound rule to allocate between.
*                                      if = NULL the use the lowest index.
* @param[in] upperBoundPtr            - the upper bound rule to allocate between.
*                                      if = NULL the use the max index.
*
* @param[out] allocNodePtrPtr          - the allocated rule node.
*                                       GT_OK if allocation succedded.
*
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper and lower
*                                       bounds.
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range of the pool.
*/
static GT_STATUS ipTcamMgmAllocMax
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *lowerBoundPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *upperBoundPtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC  **allocNodePtrPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 upperWordMask,lowerWordMask;
    GT_32 realLowerBound,realUpperBound;
    GT_32 i;
    GT_U32 *lowerWord, *upperWord;

    GT_U32 conUpperWord;

    if (pIpTcamPoolPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* first convert to real indexes*/
    if (upperBoundPtr == NULL)
        realUpperBound = pIpTcamPoolPtr->coreIpNumOfIpTcamRules;
    else
    {
        rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,upperBoundPtr,&realUpperBound);
        if(rc != GT_OK)
            return rc;
    }

    if (lowerBoundPtr == NULL)
        realLowerBound = -1;
    else
    {
        rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,lowerBoundPtr,&realLowerBound);
        if(rc != GT_OK)
            return rc;
    }

    /* first check for validty */
    if (realLowerBound >= (GT_32)realUpperBound)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
    }
    if (realLowerBound >= (GT_32)pIpTcamPoolPtr->coreIpNumOfIpTcamRules)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (realLowerBound == (GT_32)(pIpTcamPoolPtr->coreIpNumOfIpTcamRules -1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
    }

    /* create the masks for the upper and lower words, to restrict for the bounds
       only */
    upperWordMask = 0xFFFFFFFF << (realUpperBound % 32);
    lowerWordMask = 0xFFFFFFFF >> (32 - ((realLowerBound+1) % 32));

    lowerWord = &(pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[realLowerBound /32]);
    upperWord = &(pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[realUpperBound /32]);

    /* convert the words using the mask if neccessary */
    conUpperWord = *upperWord | upperWordMask;
    conUpperWord |= ((lowerWord == upperWord) && (realLowerBound != -1))?
        lowerWordMask:0;

    /* start looking for the top and down */
    while ((conUpperWord == 0xFFFFFFFF) && (upperWord != lowerWord))
    {
        if (upperWord != lowerWord)
        {
            upperWord--;
        }

        /* convert the words using the mask if neccessary */
        conUpperWord = *upperWord;
        conUpperWord |= ((lowerWord == upperWord) && (realLowerBound != -1))?
                        lowerWordMask :0;
    }


    /* check if we have an empty place */
    if (conUpperWord != 0xFFFFFFFF)
    {
        for (i = 31; i >= 0; i--)
        {
            if ((((conUpperWord) >> i) & 0x1) == 0)
            {
                /* found a place, wrap it up and send it home */
                *allocNodePtrPtr = gtCoreIpWrapAllocRule(pIpTcamPoolPtr,upperWord,i);
                return GT_OK;
            }

        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
}

/**
* @internal ipTcamMgmAllocMin function
* @endinternal
*
* @brief   allocates a rule of the pool by minimum index allocation
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] lowerBoundPtr            - the lower bound rule to allocate between.
*                                      if = NULL the use the lowest index.
* @param[in] upperBoundPtr            - the upper bound rule to allocate between.
*                                      if = NULL the use the max index.
*
* @param[out] allocNodePtrPtr          - the allocated rule node.
*                                       GT_OK if allocation succedded.
*
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper and lower
*                                       bounds.
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range of the pool.
*/
static GT_STATUS ipTcamMgmAllocMin
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *lowerBoundPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *upperBoundPtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC  **allocNodePtrPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 upperWordMask,lowerWordMask;
    GT_32 i,realLowerBound,realUpperBound;
    GT_U32 *lowerWord, *upperWord;

    GT_U32 conLowerWord;

    if (pIpTcamPoolPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* first convert to real indexes*/
    if (upperBoundPtr == NULL)
        realUpperBound = pIpTcamPoolPtr->coreIpNumOfIpTcamRules;
    else
    {
        rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,upperBoundPtr,&realUpperBound);
        if(rc != GT_OK)
            return rc;
    }

    if (lowerBoundPtr == NULL)
        realLowerBound = -1;
    else
    {
        rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,lowerBoundPtr,&realLowerBound);
        if(rc != GT_OK)
            return rc;
    }

    /* first check for validty */
    if (realLowerBound >= (GT_32)realUpperBound)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
    }
    if (realLowerBound >= (GT_32)pIpTcamPoolPtr->coreIpNumOfIpTcamRules)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (realLowerBound == (GT_32)(pIpTcamPoolPtr->coreIpNumOfIpTcamRules -1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
    }

    /* create the masks for the upper and lower words, to restrict for the bounds
       only */
    upperWordMask = 0xFFFFFFFF << (realUpperBound % 32);
    lowerWordMask = (realLowerBound != -1)?
        0xFFFFFFFF >> (32 - ((realLowerBound+1) % 32)) :0;

    lowerWord = &(pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[realLowerBound /32]);
    upperWord = &(pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[realUpperBound /32]);

    /* convert the words using the mask if neccessary */
    conLowerWord = *lowerWord | lowerWordMask;
    conLowerWord |= (lowerWord == upperWord) ?
        upperWordMask:0;

    /* start looking for the top and down */
    while ((conLowerWord == 0xFFFFFFFF) && (upperWord != lowerWord))
    {
        if (upperWord != lowerWord)
        {
            lowerWord++;
        }

        /* convert the words using the mask if neccessary */
        conLowerWord = *lowerWord ;
        conLowerWord |= (lowerWord == upperWord) ? upperWordMask :0;
    }

    /* check if we have an empty place */
    if (conLowerWord != 0xFFFFFFFF)
    {
        for (i = 0; i < 32; i++)
        {
            if ((((conLowerWord) >> i) & 0x1) == 0)
            {
                /* found a place, wrap it up and send it home */
                *allocNodePtrPtr = gtCoreIpWrapAllocRule(pIpTcamPoolPtr,lowerWord,i);
                return GT_OK;
            }

        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChLpmTcamMgmAlloc function
* @endinternal
*
* @brief   allocates a rule of the pool by the method given
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] lowerBoundPtr            - the lower bound rule to allocate between.
*                                      if = NULL the use the lowest index.
* @param[in] upperBoundPtr            - the upper bound rule to allocate between.
*                                      if = NULL the use the max index.
* @param[in] allocMethod              - the allocation method in which to allocate
*
* @param[out] allocNodePtrPtr          - the allocated rule node.
*                                       GT_OK if allocation succedded.
*
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper and lower
*                                       bounds.
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range of the pool.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmAlloc
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC         *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC        *lowerBoundPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC        *upperBoundPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_METHOD_ENT allocMethod,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC       **allocNodePtrPtr
)
{
    GT_STATUS ret;

    DEBUG_IP_MGM_TRIE(("Allocating in [start = %d end = %d] ",
               (lowerBoundPtr == NULL)?-1:lowerBoundPtr->offset,
               (upperBoundPtr == NULL)?-1:upperBoundPtr->offset));

    if (pIpTcamPoolPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    switch (allocMethod)
    {
        case PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MIN_E:
            DEBUG_IP_MGM_TRIE((" according to MIN method\n"));
            ret = ipTcamMgmAllocMin(pIpTcamPoolPtr,lowerBoundPtr,upperBoundPtr,allocNodePtrPtr);
            break;
        case PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MID_E:
            DEBUG_IP_MGM_TRIE((" according to MID method\n"));
            ret = ipTcamMgmAllocMid(pIpTcamPoolPtr,lowerBoundPtr,upperBoundPtr,allocNodePtrPtr);
            break;
        case PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MAX_E:
            DEBUG_IP_MGM_TRIE((" according to MAX method\n"));
            ret = ipTcamMgmAllocMax(pIpTcamPoolPtr,lowerBoundPtr,upperBoundPtr,allocNodePtrPtr);
            break;
        default:
            ret = GT_FAIL;
    }
    return ret;
}

/**
* @internal prvCpssDxChLpmTcamMgmAllocByGivenRule function
* @endinternal
*
* @brief   allocates a given rule.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] allocRulePtr             - the rule to allocate .
*
* @param[out] allocNodePtrPtr          - the allocated rule node.
*                                       GT_OK if allocation succedded.
*
* @retval GT_ALREADY_EXIST         - if the rule is already allocated.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmAllocByGivenRule
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocRulePtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC  **allocNodePtrPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 vecByte,vecBit;
    GT_32 index;

    rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,allocRulePtr,&index);
    if(rc != GT_OK)
        return rc;

    vecByte = (GT_U32)(index /32);
    vecBit = (GT_U32)(index % 32);

    /* first check if it's really allocated */
    if (((pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[vecByte] >> vecBit) & 0x1) == 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    *allocNodePtrPtr = gtCoreIpWrapAllocRule(pIpTcamPoolPtr,
                                       &(pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[vecByte]),
                                       index%32);
    return GT_OK;

}

/**
* @internal prvCpssDxChLpmTcamMgmFree function
* @endinternal
*
* @brief   frees a rule.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] allocNodePtr             - the rule to free.
*                                       GT_OK if allocation succedded.
*
* @retval GT_NOT_FOUND             - if the rule wasn't found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmFree
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocNodePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 vecByte,vecBit;
    GT_32 ruleIdx;

    DEBUG_IP_MGM_TRIE(("Freeing rule = %d\n",allocNodePtr->offset));

    rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,allocNodePtr,&ruleIdx);
    if(rc != GT_OK)
        return rc;

    vecByte = (GT_U32)(ruleIdx / 32);
    vecBit = (GT_U32)(ruleIdx % 32);

    /* first check if it's really allocated */
    if (((pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[vecByte] >> vecBit) & 0x1) == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[vecByte] &= ~( 1 << vecBit);

    allocNodePtr->trieNode = NULL;
    allocNodePtr->valid = GT_FALSE;

    pIpTcamPoolPtr->coreIpTcamNumOfAlloc--;

    return GT_OK;
}

/*******************************************************************************
* prvCpssDxChLpmTcamMgmSetRealIdxAsAlloc
*
* DESCRIPTION:
*       sets a real hw index as allocated.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       hwIndex          - the hwRule index.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       NULL - if the rule is already allocated or it's not in the pool
*       or the rule pointer, if we managed to allocate it.
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC *prvCpssDxChLpmTcamMgmSetRealIdxAsAlloc
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN GT_U32                                 hwIndex
)
{
    GT_U32 vecByte,vecBit;
    GT_32 ruleIdx;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC *allocNode;
    allocNode = &pIpTcamPoolPtr->coreIpTcamRulesAllocArray[0];

    /* search for the rule */
    while ((allocNode->offset < hwIndex) &&
           ((GT_UINTPTR)allocNode < (GT_UINTPTR)&pIpTcamPoolPtr->coreIpTcamRulesAllocArray[pIpTcamPoolPtr->coreIpTcamNumOfAlloc]))
    {
        allocNode++;
    }

    if (allocNode->offset != hwIndex)
    {
        return NULL;
    }

    DEBUG_IP_MGM_TRIE(("Virtual allocation of rule index = %d\n",hwIndex));

    rc = prvCpssDxChLpmTcamMgmGetRuleIdx(pIpTcamPoolPtr,allocNode,&ruleIdx);
    if (rc != GT_OK)
        return NULL;

    vecByte = (GT_U32)(ruleIdx / 32);
    vecBit = (GT_U32)(ruleIdx % 32);

    /* first check if it's free */
    if (((pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[vecByte] >> vecBit) & 0x1) == 1)
    {
        return NULL;
    }

    /* set it as allocated */
    pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[vecByte] |= ( 1 << vecBit);

    allocNode->trieNode = NULL;
    allocNode->valid = GT_TRUE;

    pIpTcamPoolPtr->coreIpTcamNumOfAlloc++;

    return allocNode;
}


/**
* @internal prvCpssDxChLpmTcamMgmCheckAvailbleMem function
* @endinternal
*
* @brief   Checks whether this poll has the availble memory
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] numOfRulesNeeded         - number of tcam rules needed
*                                       GT_OK if there is room
*                                       GT_OUT_OF_PP_MEM otherwise
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note numOfIpTcamRules != 0!
*
*/
GT_STATUS prvCpssDxChLpmTcamMgmCheckAvailbleMem
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC  *pIpTcamPoolPtr,
    IN GT_U32                               numOfRulesNeeded
)
{
    if (pIpTcamPoolPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    DEBUG_IP_MGM_TRIE(("Available mem. : [Total = %d] [Allocated = %d] [Needed = %d] \n",
               pIpTcamPoolPtr->coreIpNumOfIpTcamRules,
               pIpTcamPoolPtr->coreIpTcamNumOfAlloc,
               numOfRulesNeeded));

    if ((pIpTcamPoolPtr->coreIpTcamNumOfAlloc + numOfRulesNeeded) <= pIpTcamPoolPtr->coreIpNumOfIpTcamRules)
    {
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChLpmTcamSetRuleData function
* @endinternal
*
* @brief   set a rule data by rule index, only for valid rules!
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] ruleIdx                  - the rule index (pce num)
* @param[in] ruleData                 - the data to set for that rule
*                                       GT_OK if allocation succedded.
*
* @retval GT_ERROR                 - for trying to set an invalid rule.
* @retval GT_OUT_OF_RANGE          - if the index is out of range of the pool.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamSetRuleData
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC       *pIpTcamPoolPtr,
    IN GT_U32                                    ruleIdx,
    IN GT_PTR                                    ruleData
)
{

    GT_U32 vecByte,vecBit;

    vecByte = ruleIdx /32;
    vecBit = ruleIdx % 32;

    if (pIpTcamPoolPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* first check if it's really allocated */
    if (((pIpTcamPoolPtr->coreIpTcamRulesAllocBitMap[vecByte] >> vecBit) & 0x1) == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
    }

    if (pIpTcamPoolPtr->coreIpTcamRulesAllocArray[ruleIdx].trieNode->pData == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    pIpTcamPoolPtr->coreIpTcamRulesAllocArray[ruleIdx].trieNode->pData = ruleData;

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmTcamCompRulesPos function
* @endinternal
*
* @brief   compares two rules in terms of position
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] allocNode1Ptr            - the first rule to compare.
* @param[in] allocNode2Ptr            - the secone rule to compare.
*                                       comapre result GT_EQUAL , GT_GREATER , or GT_SMALLER.
*/
GT_COMP_RES prvCpssDxChLpmTcamCompRulesPos
(
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocNode1Ptr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocNode2Ptr
)
{
    /* Check the null cases     */
    if ((allocNode1Ptr == NULL) && (allocNode2Ptr == NULL))
        return GT_EQUAL;
    if (allocNode1Ptr == NULL)
        return GT_SMALLER;
    if (allocNode2Ptr == NULL)
        return GT_GREATER;

    if (allocNode1Ptr->offset == allocNode2Ptr->offset)
    {
        return GT_EQUAL;
    }
    else if (allocNode1Ptr->offset > allocNode2Ptr->offset)
    {
        return GT_GREATER;
    }

    return GT_SMALLER;
}


