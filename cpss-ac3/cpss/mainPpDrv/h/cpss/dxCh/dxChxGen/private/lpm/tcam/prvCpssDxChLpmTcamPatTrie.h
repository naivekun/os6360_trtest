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
* @file prvCpssDxChLpmTcamPatTrie.h
*
* @brief This file includes implementation of a patricia trie for the purpose
* of hierarchy managing to the Ip tree structure
*
* @version   20
********************************************************************************
*/

#ifndef __prvCpssDxChLpmTcamPatTrieh
#define __prvCpssDxChLpmTcamPatTrieh

#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamEntriesHandlerTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamMgm.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS 16 /*ecmp - 65*/

struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STCT;
struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT;

/**
* @enum PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_PREFIX_FLAG_TYPE_ENT
 *
 * @brief type of PREFIX FLAG in the PAT TRIE node
*/
typedef enum{

    /** @brief the PAT TRIE node holds
     *  a prefix which is part
     *  ip address
     */
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_PREFIX_FLAG_ADDR_E   = 0,

    /** @brief the PAT TRIE node holds
     *  a prefix which is part
     *  "ECMP prefix"
     */
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_PREFIX_FLAG_ECMP_E   = 1

} PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_PREFIX_FLAG_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_UPDATE_STATUS_ENT
 *
 * @brief the update status of a node in the pat trie.
*/
typedef enum{

    /** there is no action needed for this node, or it's children. */
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_UPDATE_STATUS_OK_E = 0,

    /** some of this node's children need a hw update. */
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_UPDATE_CHILDREN_E  = 1,

    /** this node needs a HW update. */
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_UPDATE_HW_E        = 2,

    /** @brief this means that there is a need of a hw update which will be over an
     *  existing one, which means we need to invlaidate first.
     */
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_INVALIDATE_FIRST_E = 4,

    /** @brief this is new node create by the insert bulk it should be writen only
     *  all the children are in there place.
     */
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_NEW_INSERT_E       = 8

} PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_UPDATE_STATUS_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_MODE_ENT
 *
 * @brief TCAM management mode
*/
typedef enum{

    /** @brief in this
     *  mode TCAM management is done by TCAM Entries Handler.
     */
    PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_ENTRIES_HANDLER_MODE_E,

    /** @brief in this
     *  mode TCAM management is done by TCAM Manager
     */
    PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_TCAM_MANAGER_MODE_E

} PRV_CPSS_DXCH_LPM_TCAM_MANAGEMENT_MODE_ENT;

/****************************************************************************
 * Ip-Patricia Trie related Data structures                                 *
 ****************************************************************************/

/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC
 *
 * Description: the LPM patricia trie node structure.
 *
 *
 * Fields:
 *      father          - the parent node pointer
 *      leftSon         - the left son (starts with 0) node pointer
 *      rightSon        - the right son (starts with 1) node pointer
 *      ipAddr          - the partial ip address this node holds
 *      updateStatus    - this the update status of this node.
 *                        in the case this is an invalid node then it will
 *                        just indicate whether some of it's sons need update.
 *      size            - the size of the trie this node is a root for.
 *      prefixFlagType  - the type of node this is (ecmp extention or regular
 *                        address)
 *      prefixLen       - out of the ipAddr the relevant prefix length
 *      tcamAllocInfo   - the rule info that this node is in charge of.
 *      pData           - A pointer to the node's data, also indicates validity
 *                        if != NULL
 *      vrId            - virtual router associated with this entry
 *      entryType       - type of the entry
 *      ipTcamFuncsPtrs - set of TCAM functions used to add this prefix
 *      ipTcamShadowPtr - pointer to ip shadow we are working on.
 */
typedef struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STCT
{
   struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STCT      *father;
   struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STCT      *leftSon;
   struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STCT      *rightSon;
   GT_U8                                                 ipAddr[PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS];
   GT_U8                                                 updateStatus;
   GT_U16                                                size;
   GT_U8                                                 prefixFlagType;
   GT_U32                                                prefixLen;
   union
   {
       struct PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STCT      *ruleNode;
       struct PRV_CPSS_DXCH_LPM_TCAM_TOKEN_NODE_STCT     *tokenNode;
   }
   tcamAllocInfo;
   GT_PTR                                                pData;
   GT_U32                                                vrId;
   PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT entryType;
   struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_FUNCS_OBJ_STCT *ipTcamFuncsPtr;
   struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT             *ipTcamShadowPtr;
}PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC;

/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_OLD_NODE_REC_STC
 *
 * Description: the LPM patricia trie old node record for bulk insertion
 *
 *
 * Fields:
 *      oldNode - the pointer for the old node that used to sit here
 *      pData   - the real data of this node.
 */
typedef struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_OLD_NODE_REC_STCT
{
   PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC  *oldNode;
   GT_PTR                                    pData;
}PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_OLD_NODE_REC_STC;


/*******************************************************************************
* PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_TRAVERSE_FUN
*
* DESCRIPTION:
*       function protoType to use when traversing.
*
* INPUTS:
*       ipTcamShadowPtr     - ip shadow we are working on.
*       vrId                 - the virtual router id.
*       ipAddr               - IP Uc address
*       prefixLen            - Ip Uc prefix length
*       trieNodePtr          - the trie node that we are currently at.
*       extraData            - extra passed data
*       devListPtr           - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       If we want to add a default IP group, then prefixLen should be 0.
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_TRAVERSE_FUN)
(
    IN struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT      *ipTcamShadowPtr,
    IN GT_U32                                         vrId,
    IN GT_U8                                          *ipAddr,
    IN GT_U32                                         prefixLen,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC       *trieNodePtr,
    IN GT_PTR                                         extraData,
    IN struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STCT *devListPtr
);

/*******************************************************************************
* PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_SET_PREFIX_FUN
*
* DESCRIPTION:
*       utility function to Add IP UC prefix.
*
* INPUTS:
*       vrId                - the virtual router id.
*       ipAddr              - IP Uc address
*       prefixLen           - Ip Uc prefix length
*       ecmpRouteNum        - the ecmp route number to write
*       tcamRulePtr         - the tcam rule to set it in
*       ipTcamShadowPtr     - ip shadow we are working on.
*       pData               - the data to attach to this prefix.
*       devListPtr          - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       If we want to add a default IP group, then prefixLen should be 0.
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_SET_PREFIX_FUN)
(
    IN  GT_U32                                         vrId,
    IN  GT_U8                                          *ipAddr,
    IN  GT_U32                                         prefixLen,
    IN  GT_U8                                          ecmpRouteNum,
    IN  GT_PTR                                         tcamRulePtr,
    IN  struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT      *ipTcamShadowPtr,
    IN  GT_PTR                                         pData,
    IN  struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STCT *devListPtr
);

/*******************************************************************************
* PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_DEL_PREFIX_FUN
*
* DESCRIPTION:
*       utility function to Delete IP prefix PCE from IP table.
*
* INPUTS:
*       vrId                - the virtual router id.
*       tcamRulePtr         - the tcam rule to delete it from.
*       ipTcamShadowPtr     - ip shadow we are working on.
*       pData               - the data entry that was attached to this prefix.
*       devListPtr          - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_DEL_PREFIX_FUN)
(
    IN  GT_U32                                         vrId,
    IN  GT_PTR                                         tcamRulePtr,
    IN  struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT      *ipTcamShadowPtr,
    IN  GT_PTR                                         pData,
    IN  struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STCT *devListPtr
);




/*******************************************************************************
* PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MOVE_PREFIX_FUN
*
* DESCRIPTION:
*       utility function to Update move PCE entry action in IP pCL table.
*
* INPUTS:
*       vrId                - the virtual router id.
*       moveFromTcamRulePtr - the tcam rule to move from.
*       moveToTcamRulePtr   - the tcam rule to move to .
*       ipTcamShadowPtr     - ip shadow we are working on.
*       trieNodePtr         - trie node associated with moved rule
*       devListPtr          - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MOVE_PREFIX_FUN)
(
    IN  GT_U32                                         vrId,
    IN  GT_PTR                                         moveFromTcamRulePtr,
    IN  GT_PTR                                         moveToTcamRulePtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC       *trieNodePtr,
    IN  struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STCT *devListPtr
);

/*******************************************************************************
* PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_UPDATE_DATA_IDX_FUN
*
* DESCRIPTION:
*       utility function to update the tcam rule with the data entry given.
*
* INPUTS:
*       oldDataEntry        - the data entry currently in.
*       dataEntry           - the data entry to set.
*       tcamRulePtr         - the tcam rule needs updating
*       isToken             - GT_TRUE  - tcamRulePtr means token (tcam manager mode)
*                           - GT_FALSE - tcamRulePtr means tcam location (tcam manager mode)
*       ipTcamShadowPtr     - ip shadow we are working on.
*       devListPtr          - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_UPDATE_DATA_IDX_FUN)
(
    IN  GT_PTR                                         oldDataEntry,
    IN  GT_PTR                                         dataEntry,
    IN  GT_PTR                                         tcamRulePtr,
    IN  GT_BOOL                                        isToken,
    IN  struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT      *ipTcamShadowPtr,
    IN  struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STCT *devListPtr
);


/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_FUNCS_OBJ_STC
 *
 * Description:
 *      TCAM utility functions Object.
 *
 * Fields:
 *      workDevListPtr   - pointer to the device list to preform the actions on.
 *      ipTcamSetPrefix  - utility function to Add IP UC prefix.
 *      ipTcamDelPrefix  - utility function to Delete IP prefix PCE from IP
 *                         table.
 *      ipTcamMovePrefix - utility function to Update move PCE entry action in
 *                         IP pCL table..
 *      ipTcamUpdateDataIdx - utility function to update the offset/pceNum (idx)
 *                            in the data entry given.
 *
 * Comments:
 *
 */

typedef struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_FUNCS_OBJ_STCT
{

    struct PRV_CPSS_DXCH_LPM_SHADOW_DEVS_LIST_STCT       **workDevListPtr;

    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_SET_PREFIX_FUN       ipTcamSetPrefix;
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_DEL_PREFIX_FUN       ipTcamDelPrefix;
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MOVE_PREFIX_FUN      ipTcamMovePrefix;
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_UPDATE_DATA_IDX_FUN  ipTcamUpdateDataIdx;

}PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_FUNCS_OBJ_STC;

/**
* @internal prvCpssDxChLpmTcamPatTrieGetBoundingSon function
* @endinternal
*
* @brief   retrive the bounding son from below the given trie node (bounding it
*         as the bounding place it can move down the trie , rule position wise)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] trieNodePtr              - the trie node .
*                                      ipTcamShadowPtr        - pointer to ip shadow we are working on.
*
* @param[out] boundingSonTrieNodePtrPtr - pointer to boundingSonTrieNode.
*                                       the bounding son.
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieGetBoundingSon
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC    *trieNodePtr,
    IN  struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT   *ipTcamShadowPtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC    **boundingSonTrieNodePtrPtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieInsert function
* @endinternal
*
* @brief   Insert a new NEXT_POINTER_PTR to the ip trie structure.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vrId                     - the virtual router id.
* @param[in] rootPtr                  - Root of the trie to insert the entry.
*                                      ipTcamShadowPtr         - the ip shadow we are working on.
* @param[in] entry                    - entry type
* @param[in] ipAddrPtr                - IP address associated with the given entry.
* @param[in] prefixLen                - prefix of ipAddr.
* @param[in] ecmpPrefixLen            - the "ECMP prefix" length (the route num of the ecmp next
*                                      hop)
* @param[in] entry                    - The  to be inserted.
* @param[in] isMaxPrefix              - is this prefix is at max prefix legnth for this protocol
*                                      stack.
* @param[in] isBulkInsert             - the type of insert this is: Bulk or regular.
* @param[in] defragEnable             - whether to defrag entries in case there is no
*                                      free place for this entry cause of entries
*                                      fragmentation; relevant only if the TCAM entries
*                                      handler was created with partitionEnable = GT_FALSE
* @param[in] ruleNodePtr              - if this isn't NULL then this is the rule node which this
*                                      prefix should occupy (HSU use)
* @param[in] ipTcamFuncsPtr           - the utility functions to use for tcam access
*                                       GT_OK if succeeded,
*                                       GT_OUT_OF_CPU_MEM if failed to allocate memory,
*                                       GT_FAIL otherwise.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieInsert
(
    IN GT_U32                                                   vrId,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC                 *rootPtr,
    IN struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT                *ipTcamShadowPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT    entryType,
    IN GT_U8                                                    *ipAddrPtr,
    IN GT_U32                                                   prefixLen,
    IN GT_U8                                                    ecmpPrefixLen,
    IN GT_PTR                                                   entry,
    IN GT_BOOL                                                  isMaxPrefix,
    IN GT_BOOL                                                  isBulkInsert,
    IN GT_BOOL                                                  defragEnable,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                     *ruleNodePtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_FUNCS_OBJ_STC            *ipTcamFuncsPtr
);

/*******************************************************************************
* prvCpssDxChLpmTcamPatTrieSearch
*
* DESCRIPTION:
*       Search for the entry which is associated with ipAddr & ipPrefix .
*
* INPUTS:
*       rootPtr     - Root of the trie to search for the entry.
*       ipAddr      - IP address associated with the given entry.
*       ipPrefix    - prefix of ipAddr.
*       entryPtr    - Indicates whether to return the entry in the searched node
*                   or not.
*
* OUTPUTS:
*       entry   - A pointer to the searched entry if found, NULL otherwise.
*       numOfRules - the number of rules this prefix takes in the TCAM (for ecmp
*                    connected to prefixes)
*
* RETURNS:
*       A pointer to the node in the trie including the searched entry.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC* prvCpssDxChLpmTcamPatTrieSearch
(
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *rootPtr,
    IN GT_U8                                    *ipAddr,
    IN GT_U32                                   prefixLen,
    OUT GT_U32                                  *numOfRules,
    INOUT GT_PTR                                *entryPtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieGetAddrFromNode function
* @endinternal
*
* @brief   construct an ip address from a given node
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] nodePtr                  - the node form which to start constructing
*
* @param[out] ipAddrPtr                - IP address associated with the given node.
* @param[out] prefixLenPtr             - prefix of ipAddr.
*                                       GT_OK
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieGetAddrFromNode
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC  *nodePtr,
    OUT GT_U8                                     *ipAddrPtr,
    OUT GT_U32                                    *prefixLenPtr
);

/*******************************************************************************
* prvCpssDxChLpmTcamPatTrieGetNext
*
* DESCRIPTION:
*       Search for the entry which is associated with larger ipAddr & ipPrefix .
*
* INPUTS:
*       rootPtr         - Root of the trie to search for the entry.
*       ipAddr          - IP address associated with the given entry.
*       prefixLenPtr    - prefix of ipAddr.
*       entryPtr        - Indicates whether to return the entry in the searched node
*                       or not.
*
* OUTPUTS:
*       ipAddr    - the address of the next prefix
*       prefixLen - the prefix length of the next prefix
*       entryPtr  - A pointer to the searched entry if found, NULL otherwise.
*
* RETURNS:
*       A pointer to the node in the trie including the next entry.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC* prvCpssDxChLpmTcamPatTrieGetNext
(
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC      *rootPtr,
    INOUT GT_U8                                      *ipAddr,
    INOUT GT_U32                                     *prefixLenPtr,
    INOUT GT_PTR                                     *entryPtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieWriteToHw function
* @endinternal
*
* @brief   write the whole patricai tree to the HW.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       GT_OK
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieWriteToHw
(
    IN struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT     *ipTcamShadowPtr,
    IN GT_U32                                        vrId,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC      *rootPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_FUNCS_OBJ_STC *ipTcamFuncsPtr,
    IN GT_BOOL                                       isUpdateOnly
);

/**
* @internal prvCpssDxChLpmTcamPatTrieDel function
* @endinternal
*
* @brief   Deletes a previously inserted entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vrId                     - the virtual router id.
* @param[in] rootPtr                  - Root of the trie to delete from.
*                                      ipTcamShadowPtr - the ip shadow we are working on.
* @param[in] entryType                - entry type
* @param[in] ipAddrPtr                - IP address associated with the entry to be deleted.
* @param[in] prefixLen                - prefix of ipAddr.
* @param[in] ecmpPrefixLen            - the "ECMP prefix" length (the route num of the ecmp next
*                                      hop)
* @param[in] ipTcamFuncsPtr           the utility functions to use for tcam access
*
* @param[out] entryPtr                 - A pointer to the entry of the deleted node if found, NULL
*                                      otherwise.
*                                       GT_OK if succeeded, GT_FAIL otherwise.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function does not free the pointers to the data stored in the
*       trie node, it's the responsibility of the caller to do that.
*
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieDel
(
    IN GT_U32                                                  vrId,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC                *rootPtr,
    IN struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT               *ipTcamShadowPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT   entryType,
    IN GT_U8                                                   *ipAddrPtr,
    IN GT_U32                                                  prefixLen,
    IN GT_U8                                                   ecmpPrefixLen,
    OUT GT_PTR                                                 *entryPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_FUNCS_OBJ_STC           *ipTcamFuncsPtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieInit function
* @endinternal
*
* @brief   Initialize a new Pat-Trie for the prefixes
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] rootPtrPtr               - the root of the new trie.
*                                       GT_OK if succeeded, GT_FAIL otherwise.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieInit
(
    IN struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT            *ipTcamShadowPtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC            **rootPtrPtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieDestroy function
* @endinternal
*
* @brief   Delete a Pat-Trie , only if it is an empty one
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       GT_OK if succeeded, GT_FAIL otherwise.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieDestroy
(
    IN struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT            *ipTcamShadowPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC             *rootPtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieFlush function
* @endinternal
*
* @brief   flushes all trie (along with deleting all data nodes).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] rootPtr                  - Root of the trie to delete from.
*                                      ipTcamShadowPtr - the ip shadow we are working on
* @param[in] entryType                - entry type
* @param[in] ipTcamFuncsPtr           - the utility functions to use for tcam access
* @param[in] dataFreeFunc             - A pointer to a function to be called for deleteing the
*                                      user data, or NULL if no free operation should be done
*                                      on the user's data.
* @param[in] flushDefault             - whether to flush the default rule as well
*                                       None.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieFlush
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC               *rootPtr,
    IN  struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT              *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_FUNCS_OBJ_STC          *ipTcamFuncsPtr,
    IN  GT_MEMFREEFUNCPTR                                      dataFreeFunc,
    IN  GT_BOOL                                                flushDefault
);

/**
* @internal prvCpssDxChLpmTcamPatTrieIsEmpty function
* @endinternal
*
* @brief   check whether the trie is empty
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] rootPtr                  - the root of the trie.
*                                       whether the trie is empty
*/
GT_BOOL prvCpssDxChLpmTcamPatTrieIsEmpty
(
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *rootPtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieGetNextPostOrderTrieNode function
* @endinternal
*
* @brief   Retrieves the next node to process in post order DFS scan.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] nodePtrPtr               - points to the last node retrieved by the function;
*                                      on first call, use the trie root
* @param[in,out] nodePtrPtr               - points to the next trie node to process
*
* @param[out] lastNodePtr              - GT_TRUE:  nodePtr is the last node to process in the trie
*                                      GT_FALSE: there are additional nodes to process in the trie
*
* @retval GT_OK                    - on success
*
* @note 1. The trie root is always the last node to be processed in post order DFS
*       scan. Therefore calling the function with the trie root is used as an
*       indication that this is the first call.
*       2. Using NULL as one of the pointers is not valid.
*       3. Use the following code skeleton to scan trie:
*       nodePtr = trieRootPtr;
*       isLastNode = GT_FALSE;
*       while (isLastNode == GT_FALSE)
*       (
*       prvCpssDxChLpmTcamPatTrieGetNextPostOrderTrieNode(&nodePtr,&isLastNode);
*       process node...
*       )
*
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieGetNextPostOrderTrieNode
(
    INOUT PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC    **nodePtrPtr,
    OUT   GT_BOOL                                     *lastNodePtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieRelocatePrefix function
* @endinternal
*
* @brief   Move a prefix from a specific location in the TCAM to another location
*         in the TCAM.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - if success
*/
GT_STATUS prvCpssDxChLpmTcamPatTrieRelocatePrefix
(
    IN struct PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STCT       *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC       *prefixPtr
);

/**
* @internal prvCpssDxChLpmTcamPatTrieCopyIpAddr function
* @endinternal
*
* @brief   copies one Ip GT_U8 array address to another
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] destIpAddrPtr            - (pointer to)destination ip address.
* @param[in,out] sourceIpAddrPtr          - (pointer to)source ip address.
* @param[in] startBitSource           - start bit index to copy from.
* @param[in] endBitSource             - end bit index to copy from.
* @param[in] startBitDest             - start bit to copy to in the destination.
* @param[in,out] destIpAddrPtr            - (pointer to)destination ip address.
* @param[in,out] sourceIpAddrPtr          - (pointer to)source ip address.
*                                       None.
*/
void prvCpssDxChLpmTcamPatTrieCopyIpAddr
(
    INOUT GT_U8 *destIpAddrPtr,
    INOUT GT_U8 *sourceIpAddrPtr,
    IN GT_U32   startBitSource,
    IN GT_U32   endBitSource,
    IN GT_U32   startBitDest
);

/*******************************************************************************
* prvCpssDxChLpmTcamPatTrieGetValidFather
*
* DESCRIPTION:
*       get the first valid father node of a give trie node.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       trieNodePtr    - the trie node .
*
* OUTPUTS:
*       None.
*
* RETURNS:
*      GT_OK.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *prvCpssDxChLpmTcamPatTrieGetValidFather
(
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *trieNodePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTcamPatTrieh */


