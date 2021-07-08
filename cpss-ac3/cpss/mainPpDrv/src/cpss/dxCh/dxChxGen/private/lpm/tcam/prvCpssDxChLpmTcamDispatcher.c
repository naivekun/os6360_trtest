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
* @file prvCpssDxChLpmTcamDispatcher.c
*
* @brief This file defines which TCAM engine is used : TCAM entry handler or
* TCAM Manager
*
* @version   9
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamDispatcher.h>

/**
* @internal prvCpssDxChGetTcamManagerAllocMethod function
* @endinternal
*
* @brief   Get TCAM Manager allocation method
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamHandlerAllocMethod   - allocation method used by TCAM Entries Handler
*
* @param[out] tcamManagerAllocMethodPtr - pointer to allocation method used by TCAM Manager.
*
* @retval GT_OK                    - if OK
* @retval GT_BAD_VALUE             - if bad value
*/
static GT_STATUS prvCpssDxChGetTcamManagerAllocMethod
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_METHOD_ENT tcamHandlerAllocMethod,
    OUT CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT     *tcamManagerAllocMethodPtr
)
{
    switch(tcamHandlerAllocMethod)
    {
    case PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MIN_E:
        *tcamManagerAllocMethodPtr = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MID_E:
        *tcamManagerAllocMethodPtr = CPSS_DXCH_TCAM_MANAGER_ALLOC_MID_E;
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MAX_E:
        *tcamManagerAllocMethodPtr = CPSS_DXCH_TCAM_MANAGER_ALLOC_MAX_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}



/**
* @internal prvCpssDxChGetTcamManagerEntryType function
* @endinternal
*
* @brief   Get TCAM Manager entry type
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] shadowType               - DXCH ip shadow type
* @param[in] entryType                - TCAM handler entry type
*
* @param[out] tcamManagerEntryTypePtr  - pointer to TCAM Manager entry type.
*
* @retval GT_OK                    - if OK
* @retval GT_BAD_VALUE             - if bad value
*/
static GT_STATUS prvCpssDxChGetTcamManagerEntryType
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT                      shadowType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    OUT CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT                  *tcamManagerEntryTypePtr
)
{
    GT_STATUS rc = GT_OK;
    if (shadowType == PRV_CPSS_DXCH_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E)
    {
        *tcamManagerEntryTypePtr = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
    }
    else
    {
        switch(entryType)
        {
        case PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_IPV4_UC_ENTRY_E:
        case PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_IPV4_MC_GROUP_ENTRY_E:
        case PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_IPV4_MC_SOURCE_ENTRY_E:
            *tcamManagerEntryTypePtr = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;
            break;
        case PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_IPV6_UC_ENTRY_E:
        case PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_IPV6_MC_GROUP_ENTRY_E:
        case PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_IPV6_MC_SOURCE_ENTRY_E:
            *tcamManagerEntryTypePtr = CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChLpmTcamDispatcherRuleSet function
* @endinternal
*
* @brief   Set TCAM allocate rule
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] trieNodePtr              - trie node associated with tcam allocated rule
* @param[in] ipTcamShadowPtr          - pointer to ip shadow we are working on
*
* @param[out] tcamRulePtr              - pointer to TCAM rule allocation info.
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on fail
*/
 GT_STATUS prvCpssDxChLpmTcamDispatcherRuleSet
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC    *trieNodePtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC           *ipTcamShadowPtr,
    OUT GT_PTR                                      *tcamRulePtr
)
{
    if(ipTcamShadowPtr->tcamManagementMode ==
                        PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)
    {
        *tcamRulePtr = (GT_PTR)trieNodePtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken;
    }
    else
    {
        *tcamRulePtr = (GT_PTR)trieNodePtr->tcamAllocInfo.ruleNode;
    }
    return GT_OK;
}



/**
* @internal prvCpssDxChLpmTcamDispatchAllocate function
* @endinternal
*
* @brief   It decides which allocation mechanism is involved: TCAM manager or
*         TCAM handler.
*         Allocates TCAM entry for specific type of prefix. The entry is allocated
*         in a range that is bounded by two other entries (bounded in the meaning
*         of TCAM search order). The entry is allocated according to specific
*         allocation method.
* @param[in] ipTcamShadowPtr          - pointer to ip shadow we are working on.
* @param[in] entryType                - entry type to allocate
* @param[in] lowerBound               - lower bound; the allocated rule reside after
*                                      this rule; if NULL then no lower bound
* @param[in] upperBound               - upper bound; the allocated rule reside before
*                                      this rule; if NULL then no upper bound
* @param[in] tcamHandlerAllocMethod   - TCAM handler allocation method
* @param[in] defragEnable             - whether to defrag entries in case there is no
*                                      free place for this entry cause of entries
*                                      fragmentation; relevant only if the TCAM entries
*                                      handler was created with partitionEnable = GT_FALSE
* @param[in] trieNodePtr              - trieNode associated with new TCAM rule
*
* @param[out] tcamRulePtr              - the allocated rule node
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDispatchAllocate
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                       *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT   entryType,
    IN  GT_PTR                                                  lowerBound,
    IN  GT_PTR                                                  upperBound,
    IN  PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_METHOD_ENT             tcamHandlerAllocMethod,
    IN  GT_BOOL                                                 defragEnable,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC                 *trieNodePtr,
    OUT GT_PTR                                                  *tcamRulePtr
)
{
    CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT    tcamManagerAllocMethod = CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E;
    GT_U32                                     allocEntryToken = 0;
    CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT      tcamManagerEntryType = CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E;

    GT_STATUS ret = GT_OK;

    if(ipTcamShadowPtr->tcamManagementMode ==
                        PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
    {
        ret = prvCpssDxChLpmTcamEntriesHandlerAllocate(ipTcamShadowPtr,
                                                       entryType,
                                                       (PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC*)lowerBound,
                                                       (PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC*)upperBound,
                                                       tcamHandlerAllocMethod,
                                                       defragEnable,
                                                       (PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC**)tcamRulePtr);
        if (ret != GT_OK)
        {
            return ret;
        }
    }
    else
    {
        ret = prvCpssDxChGetTcamManagerEntryType(ipTcamShadowPtr->shadowType,
                                                 entryType,
                                                 &tcamManagerEntryType );
        if (ret != GT_OK)
        {
            return ret;
        }

        ret = prvCpssDxChGetTcamManagerAllocMethod(tcamHandlerAllocMethod,
                                                   &tcamManagerAllocMethod);
        if (ret != GT_OK)
        {
            return ret;
        }


        ret = cpssDxChTcamManagerEntryAllocate(ipTcamShadowPtr->tcamManagerHandlerPtr,
                                               ipTcamShadowPtr->clientId,
                                               tcamManagerEntryType,
                                               (GT_U32)((GT_UINTPTR)lowerBound),
                                               (GT_U32)((GT_UINTPTR)upperBound),
                                               tcamManagerAllocMethod,
                                               defragEnable,
                                               (GT_PTR)trieNodePtr,
                                               &allocEntryToken);
        if (ret != GT_OK)
        {
            return ret;
        }
        *tcamRulePtr = (GT_PTR)((GT_UINTPTR)allocEntryToken);

    }

    return ret;
}


/**
* @internal prvCpssDxChLpmTcamDispatchFree function
* @endinternal
*
* @brief   Frees a TCAM entry.
*
* @param[in] ipTcamShadowPtr          - pointer to ip shadow we are working on.
* @param[in] entryType                - entry type
* @param[in] trieNodePtr              - trie node associated with the entry to free.
*
* @retval GT_OK                    - if allocation succedded.
* @retval GT_NOT_FOUND             - if the rule wasn't found.
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDispatchFree
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                      *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC               *trieNodePtr
)
{
    GT_STATUS ret = GT_OK;
    if(ipTcamShadowPtr->tcamManagementMode ==
                        PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
    {
        ret = prvCpssDxChLpmTcamEntriesHandlerFree(ipTcamShadowPtr->tcamEntriesHandler,
                                                   entryType,
                                                   trieNodePtr->tcamAllocInfo.ruleNode);

    }
    else
    {
        ret = cpssDxChTcamManagerEntryFree(ipTcamShadowPtr->tcamManagerHandlerPtr,
                                           ipTcamShadowPtr->clientId,
                                           (GT_U32)(trieNodePtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken));
    }
    return ret;

}


/**
* @internal prvCpssDxChLpmTcamDispatcherCompRulesPos function
* @endinternal
*
* @brief   compares two rules in terms of position.
*
* @param[in] ipTcamShadowPtr          - pointer to ip shadow we are working on.
* @param[in] firstTcamRulePtr         - pointer to tcam rule.
* @param[in] secondTcamRulePtr        - pointer to tcam rule.
*
* @param[out] resultPtr                - compare result GT_EQUAL , GT_GREATER , or GT_SMALLER.
*
* @retval GT_OK                    - if compare succedded
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
*/
GT_STATUS prvCpssDxChLpmTcamDispatcherCompRulesPos
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                    *ipTcamShadowPtr,
    IN  GT_PTR                                               *firstTcamRulePtr,
    IN  GT_PTR                                               *secondTcamRulePtr,
    OUT GT_COMP_RES                                          *resultPtr
)
{
    GT_STATUS ret = GT_OK;

    if(ipTcamShadowPtr->tcamManagementMode ==
       PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
    {
        *resultPtr =  prvCpssDxChLpmTcamCompRulesPos((PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC *)firstTcamRulePtr,
                                                     (PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC *)secondTcamRulePtr);
    }
    else
    {
        if (((GT_UINTPTR)firstTcamRulePtr == PRV_CPSS_DXCH_LPM_TCAM_NON_INITIALIZED_TOKEN_CNS) &&
            ((GT_UINTPTR)secondTcamRulePtr == PRV_CPSS_DXCH_LPM_TCAM_NON_INITIALIZED_TOKEN_CNS))
            *resultPtr = GT_EQUAL;
        else if ((GT_UINTPTR)firstTcamRulePtr == PRV_CPSS_DXCH_LPM_TCAM_NON_INITIALIZED_TOKEN_CNS)
            *resultPtr = GT_SMALLER;
        else if ((GT_UINTPTR)secondTcamRulePtr == PRV_CPSS_DXCH_LPM_TCAM_NON_INITIALIZED_TOKEN_CNS)
            *resultPtr = GT_GREATER;
        else
            ret =  cpssDxChTcamManagerTokenCompare(ipTcamShadowPtr->tcamManagerHandlerPtr,
                                               ipTcamShadowPtr->clientId,
                                               (GT_U32)((GT_UINTPTR)firstTcamRulePtr),
                                               (GT_U32)((GT_UINTPTR)secondTcamRulePtr),
                                               resultPtr);
    }
    return ret;
}


/**
* @internal prvCpssDxChLpmTcamDispatchCheckAvailableMem function
* @endinternal
*
* @brief   compares two rules in terms of position.
*
* @param[in] ipTcamShadowPtr          - pointer to ip shadow we are working on.
* @param[in] entryType                - entry type
* @param[in] numOfRulesNeeded         - number of entries needed
* @param[in] requestedEntriesArray    - array of amount of entries requested by client
* @param[in] sizeOfArray              - number of elemenets in the array
*
* @retval GT_OK                    - if OK
* @retval GT_BAD_PARAM             - if wrong parameters
*/
GT_STATUS prvCpssDxChLpmTcamDispatchCheckAvailableMem
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                         *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT     entryType,
    IN  GT_U32                                                    numOfRulesNeeded,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC                   *requestedEntriesArray,
    IN  GT_U32                                                    sizeOfArray
)
{
    GT_STATUS ret = GT_OK;
    if(ipTcamShadowPtr->tcamManagementMode ==
       PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
    {
        ret = prvCpssDxChLpmTcamEntriesHandlerCheckAvailableMem(ipTcamShadowPtr->tcamEntriesHandler,
                                                                entryType,
                                                                numOfRulesNeeded);
    }
    else
    {
        ret =  cpssDxChTcamManagerAvailableEntriesCheck(ipTcamShadowPtr->tcamManagerHandlerPtr,
                                                        ipTcamShadowPtr->clientId,
                                                        requestedEntriesArray,
                                                        sizeOfArray);

    }
    return ret;
}

/**
* @internal prvCpssDxChLpmTcamDispatcherRuleInfoUpdate function
* @endinternal
*
* @brief   Set TCAM allocate rule
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] ruleUpdateAction         - rule update action
* @param[in] tcamManagementMode       - tcam management mode
* @param[in] srcTrieNodePtr           - trieNode which rule is taking from
* @param[in] dstTrieNodePtr           - trieNode which rule is updated
* @param[in] allocatedRulePtr         - allocated rule
*
* @retval GT_OK                    - on OK.
* @retval GT_BAD_PARAM             - on bad ruleUpdateAction.
* @retval GT_BAD_VALUE             - on bad value.
*/
 GT_STATUS prvCpssDxChLpmTcamDispatcherRuleInfoUpdate
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_RULE_UPDATE_ENT ruleUpdateAction,
    IN  PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_MODE_ENT        tcamManagementMode,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC          *srcTrieNodePtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC          *dstTrieNodePtr,
    IN  GT_PTR                                            allocatedRulePtr
)
{
    switch (ruleUpdateAction)
    {
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_RULE_UPDATE_TRIE_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
            dstTrieNodePtr->tcamAllocInfo.ruleNode->trieNode = srcTrieNodePtr;
        else
            dstTrieNodePtr->tcamAllocInfo.tokenNode->trieNode = srcTrieNodePtr;
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_ALLOCATED_RULE_UPDATE_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)
            dstTrieNodePtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken = (GT_UINTPTR)allocatedRulePtr;
        else
            dstTrieNodePtr->tcamAllocInfo.ruleNode = (PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC*)allocatedRulePtr;
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_ALLOCATED_RULE_AND_TRIE_UPDATE_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)
        {
            dstTrieNodePtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken = (GT_UINTPTR)allocatedRulePtr;
            dstTrieNodePtr->tcamAllocInfo.tokenNode->trieNode = dstTrieNodePtr;
        }
        else
        {
            dstTrieNodePtr->tcamAllocInfo.ruleNode = (PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC*)allocatedRulePtr;
            dstTrieNodePtr->tcamAllocInfo.ruleNode->trieNode = dstTrieNodePtr;
        }
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_RULE_COPY_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
            dstTrieNodePtr->tcamAllocInfo.ruleNode = srcTrieNodePtr->tcamAllocInfo.ruleNode ;
        else
        {
            /* update node for all tir nodes except of dummy mc src root trie node */
            if( !((dstTrieNodePtr->father == NULL) && (dstTrieNodePtr->pData == NULL)) )
            {
                dstTrieNodePtr->tcamAllocInfo.tokenNode = srcTrieNodePtr->tcamAllocInfo.tokenNode;
            }
        }
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_INVALID_RULE_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
            dstTrieNodePtr->tcamAllocInfo.ruleNode = NULL;
        else
            dstTrieNodePtr->tcamAllocInfo.tokenNode = NULL;
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_INVALID_OLD_RULE_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
            dstTrieNodePtr->tcamAllocInfo.ruleNode = NULL;
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_INIT_TOKEN_RULE_INFO_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)
        {
            dstTrieNodePtr->tcamAllocInfo.tokenNode->trieNode = NULL;
            dstTrieNodePtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken = PRV_CPSS_DXCH_LPM_TCAM_NON_INITIALIZED_TOKEN_CNS;
        }
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_INIT_RULE_INFO_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
        {
            dstTrieNodePtr->tcamAllocInfo.ruleNode->valid = GT_FALSE;
            dstTrieNodePtr->tcamAllocInfo.ruleNode->trieNode = NULL;
        }
        else
        {
            dstTrieNodePtr->tcamAllocInfo.tokenNode->trieNode = NULL;
            dstTrieNodePtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken = PRV_CPSS_DXCH_LPM_TCAM_NON_INITIALIZED_TOKEN_CNS;
        }
        break;
    case PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_CHECK_RULE_VALIDITY_E:
        if(tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
        {
            if( dstTrieNodePtr->tcamAllocInfo.ruleNode->valid != GT_TRUE)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }
        else
        {
            if( dstTrieNodePtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken == PRV_CPSS_DXCH_LPM_TCAM_NON_INITIALIZED_TOKEN_CNS)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmTcamDispatchGetEntryHsuInfo function
* @endinternal
*
* @brief   Gets HSU information for entry.
*
* @param[in] ipTcamShadowPtr          - pointer to ip shadow we are working on
* @param[in] entryType                - entry type
* @param[in] trieNodePtr              - trie node associated with the entry
*
* @param[out] hsuInfoPtr               - points to hsu information
*
* @retval GT_OK                    - if allocation succedded.
* @retval GT_NOT_FOUND             - if the rule wasn't found.
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDispatchGetEntryHsuInfo
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                       *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT   entryType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC                *trieNodePtr,
    OUT GT_32                                                   *hsuInfoPtr
)
{
    GT_STATUS ret = GT_OK;

    if(ipTcamShadowPtr->tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
    {
        ret = prvCpssDxChLpmTcamEntriesHandlerGetRuleIdx(ipTcamShadowPtr->tcamEntriesHandler,
                                                         entryType,
                                                         trieNodePtr->tcamAllocInfo.ruleNode,
                                                         hsuInfoPtr);
    }
    else
    {
        *hsuInfoPtr = (GT_U32)(trieNodePtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken);
    }

    return ret;
}

/**
* @internal prvCpssDxChLpmTcamDispatchSetEntryHsuInfo function
* @endinternal
*
* @brief   Sets HSU information for entry.
*
* @param[in] ipTcamShadowPtr          - pointer to ip shadow we are working on
* @param[in] entryType                - entry type
* @param[in] hsuInfo                  - hsu information
*
* @param[out] tcamRulePtr              - the allocated rule node
*
* @retval GT_OK                    - if allocation succedded.
* @retval GT_NOT_FOUND             - if the rule wasn't found.
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDispatchSetEntryHsuInfo
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                      *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    IN  GT_U32                                                 hsuInfo,
    OUT GT_PTR                                                 *tcamRulePtr
)
{
    GT_STATUS ret = GT_OK;

    if(ipTcamShadowPtr->tcamManagementMode == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)
    {
        ret = prvCpssDxChLpmTcamEntriesHandlerGetRuleByRuleIdx(ipTcamShadowPtr->tcamEntriesHandler,
                                                               entryType,
                                                               hsuInfo,
                                                               (PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC**)tcamRulePtr);
    }
    else
    {
        *tcamRulePtr = (GT_PTR)((GT_UINTPTR)hsuInfo);
    }

    return ret;
}


