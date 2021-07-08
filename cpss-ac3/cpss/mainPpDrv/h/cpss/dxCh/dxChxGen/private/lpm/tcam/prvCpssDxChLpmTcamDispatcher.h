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
* @file prvCpssDxChLpmTcamDispatcher.h
*
* @brief This file defines which TCAM engine is used : TCAM entry handler or
* TCAM Manager
*
* @version   10
********************************************************************************
*/

#ifndef __prvCpssDxChLpmTcamDispatcherh
#define __prvCpssDxChLpmTcamDispatcherh

#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamEntriesHandler.h>
#include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManager.h>


#define PRV_CPSS_DXCH_LPM_TCAM_NON_INITIALIZED_TOKEN_CNS 0xFFFFFFFF

/**
* @enum PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_RULE_UPDATE_ENT
 *
 * @brief define update tcam rule cases
*/
typedef enum{

    /** update trie node of given rule */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_RULE_UPDATE_TRIE_E,

    /** update rule by new allocated rule. */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_ALLOCATED_RULE_UPDATE_E,

    /** @brief update rule by new allocated rule
     *  and update its trie node.
     */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_ALLOCATED_RULE_AND_TRIE_UPDATE_E,

    /** copy rule from one trie node to another. */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_RULE_COPY_E,

    /** invalid rule node. */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_INVALID_RULE_E,

    /** invalid legacy rule node only. */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_INVALID_OLD_RULE_E,

    /** invalid token rule node only. */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_INIT_TOKEN_RULE_INFO_E,

    /** initilize rule parameters. */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_INIT_RULE_INFO_E,

    /** check rule validity. */
    PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_CHECK_RULE_VALIDITY_E

} PRV_CPSS_DXCH_LPM_TCAM_DISPATCHER_RULE_UPDATE_ENT;



#define PRV_CPSS_DXCH_LPM_TCAM_SET_BOUNDING_SON_MAC(tcamManagementMode,trieNodePtr,boundingSonTrieNodePtr)       \
        if((tcamManagementMode) == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E)        \
            boundingSonTrieNodePtr = trieNodePtr->tcamAllocInfo.ruleNode->trieNode ;                   \
        else                                                                                            \
            boundingSonTrieNodePtr = trieNodePtr->tcamAllocInfo.tokenNode->trieNode;

#define  PRV_CPSS_DXCH_LPM_TCAM_ALLOCATE_TOKEN_NODE_MAC(tcamManagementMode,trieNodePtr)\
        if((tcamManagementMode) == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)\
          {\
           (trieNodePtr)->tcamAllocInfo.tokenNode = (PRV_CPSS_DXCH_LPM_TCAM_TOKEN_NODE_STC *)cpssOsMalloc(\
                                                    sizeof(PRV_CPSS_DXCH_LPM_TCAM_TOKEN_NODE_STC));\
           if((trieNodePtr)->tcamAllocInfo.tokenNode == NULL)\
             {\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);\
             }\
          }

#define  PRV_CPSS_DXCH_LPM_TCAM_FREE_TOKEN_NODE_MAC(tcamManagementMode,trieNodePtr)\
        if((tcamManagementMode) == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)\
          {\
           if((trieNodePtr)->tcamAllocInfo.tokenNode != NULL)\
             cpssOsFree((trieNodePtr)->tcamAllocInfo.tokenNode);\
          }

#define  PRV_CPSS_DXCH_LPM_TCAM_FREE_TOKEN_NODE_FOR_VALID_TRIE_MAC(tcamManagementMode,trieNodePtr)\
        if((tcamManagementMode) == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)\
          {\
          if(trieNodePtr->tcamAllocInfo.tokenNode != NULL)\
           if(!( ((trieNodePtr->father != NULL) && \
                 (trieNodePtr->tcamAllocInfo.tokenNode == trieNodePtr->father->tcamAllocInfo.tokenNode))||\
                ((trieNodePtr->leftSon != NULL) && \
                 (trieNodePtr->tcamAllocInfo.tokenNode == trieNodePtr->leftSon->tcamAllocInfo.tokenNode))||\
                ((trieNodePtr->rightSon != NULL) && \
                 (trieNodePtr->tcamAllocInfo.tokenNode == trieNodePtr->rightSon->tcamAllocInfo.tokenNode)) ) )\
           {\
               cpssOsFree(trieNodePtr->tcamAllocInfo.tokenNode);\
           }\
          }
#define  PRV_CPSS_DXCH_LPM_TCAM_FREE_ROOT_TOKEN_NODE_MAC(tcamManagementMode,trieNodePtr)\
        if((tcamManagementMode) == PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E)\
          {\
           if(trieNodePtr->tcamAllocInfo.tokenNode != NULL)\
               cpssOsFree(trieNodePtr->tcamAllocInfo.tokenNode);\
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
);


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
);


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
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                      *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    IN  GT_PTR                                                 lowerBound,
    IN  GT_PTR                                                 upperBound,
    IN  PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_METHOD_ENT            tcamHandlerAllocMethod,
    IN  GT_BOOL                                                defragEnable,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC               *trieNodePtr,
    OUT GT_PTR                                                 *tcamRulePtr
);



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
);




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
);

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
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                     *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT entryType,
    IN  GT_U32                                                numOfRulesNeeded,
    IN  CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC               *requestedEntriesArray,
    IN  GT_U32                                                sizeOfArray
);

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
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                      *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC               *trieNodePtr,
    OUT GT_32                                                  *hsuInfoPtr
);

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
);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTcamDispatcherh */


