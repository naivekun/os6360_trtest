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
* @file prvCpssDxChLpmTcamEntriesHandler.h
*
* @brief This file contains object to handle TCAM entries.
* The TCAM entries handler is responsible for handling the TCAM entries
* and deciding which entries to allocate to each prefix type.
* The TCAM entries handler has two modes of operation:
* - Partition mode. In this mode the TCAM range allocated for the TCAM
* entries handler in partitioned so each entry type has fixed section
* of the TCAM.
* - No partition mode. In this mode the TCAM range is not partitioned.
* All entry types share the TCAM entries and TCAM entries are
* allocated on demand.
* In partition mode, the TCAM entries handler is used as a glue layer to
* the common tcam pool manager code which implements a partitioned TCAM.
* In no partition mode, the TCAM entries handler implements a dynamic
* method for a non partitioned TCAM.
*
* @version   6
********************************************************************************
*/

#ifndef __prvCpssDxChLpmTcamEntriesHandlerh
#define __prvCpssDxChLpmTcamEntriesHandlerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamMgm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamEntriesHandlerTypes.h>

#define PRV_CPSS_DXCH_LPM_TCAM_CHEETAH2_NUM_OF_RULES_IN_COLUMN_CNS 1024
#define PRV_CPSS_DXCH_LPM_TCAM_CHEETAH_NUM_OF_RULES_IN_COLUMN_CNS  512
#define PRV_CPSS_DXCH_LPM_TCAM_CHEETAH3_TCAM_NUM_ROWS_CEILING_CNS 10240
#define PRV_CPSS_DXCH_LPM_TCAM_XCAT_POLICY_TCAM_NUM_OF_RULES_CEILING_CNS 10240

/**
* @internal prvCpssDxChLpmTcamEntriesHandlerCreate function
* @endinternal
*
* @brief   Creates TCAM entries handler object.
*         The TCAM entries handler is responsible for handling the TCAM entries
*         and deciding which entries to allocate to each prefix type.
*         The TCAM entries handler has two modes of operation:
*         - Partition mode. In this mode the TCAM range allocated for the TCAM
*         entries handler in partitioned so each entry type has fixed section
*         of the TCAM.
*         - No partition mode. In this mode the TCAM range is not partitioned.
*         All entry types share the TCAM entries and TCAM entries are
*         allocated on demand.
*         In partition mode, the TCAM entries handler is used as a glue layer to
*         the common tcam pool manager code which implements a partitioned TCAM.
*         In no partition mode, the TCAM entries handler implements a dynamic
*         method for a non partitioned TCAM.
* @param[in,out] ipTcamShadowPtr          - the shadow type to create the TCAM entries handler for
* @param[in] indexesRangePtr          - the range of TCAM indexes allocated for this TCAM entries
*                                      handler
* @param[in] partitionEnable          - GT_TRUE:  partition the TCAM range according to the
* @param[in] capacityCfgPtr           any unused TCAM entries will
*                                      be allocated to IPv4 UC entries
*                                      GT_FALSE: allocate TCAM entries on demand while
*                                      guarantee entries as specified in capacityCfgPtr
* @param[in] capacityCfgPtr           - capacity configuration
* @param[in] numOfReservedPrefixes    - number of entries to reserve for non-ip entries
*                                      used for Cheetah2 Multi Virtual Router support only
* @param[in] protocolStack            - the protocol stack to support
* @param[in] tcamRows                 - total number of rows in the TCAM
* @param[in,out] ipTcamShadowPtr          - the shadow type to create the TCAM entries handler for
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerCreate
(
    INOUT PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC             *ipTcamShadowPtr,
    IN    PRV_CPSS_DXCH_LPM_TCAM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN    GT_BOOL                                       partitionEnable,
    IN    PRV_CPSS_DXCH_LPM_TCAM_MANGER_CAPCITY_CFG_STC *capacityCfgPtr,
    IN    GT_U32                                        numOfReservedPrefixes,
    IN    CPSS_IP_PROTOCOL_STACK_ENT                    protocolStack,
    IN    GT_U32                                        tcamRows
);

/**
* @internal prvCpssDxChLpmTcamEntriesHandlerDelete function
* @endinternal
*
* @brief   Deletes TCAM entries handler object.
*
* @param[in,out] ipTcamShadowPtr          - the shadow type to delete the TCAM entries handler from
* @param[in,out] ipTcamShadowPtr          - the shadow type to delete the TCAM entries handler from
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerDelete
(
    INOUT PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC             *ipTcamShadowPtr
);


/**
* @internal prvCpssDxChLpmTcamEntriesHandlerAllocate function
* @endinternal
*
* @brief   Allocates TCAM entry for specific type of prefix. The entry is allocated
*         in a range that is bounded by two other entries (bounded in the meaning
*         of TCAM search order). The entry is allocated according to specific
*         allocation method.
* @param[in] ipTcamShadowPtr          - the ip shadow pointer we are working on
* @param[in] entryType                - entry type to allocate
* @param[in] lowerBoundPtr            - the lower bound; the allocated rule reside after
*                                      this rule; if NULL then no lower bound
* @param[in] upperBoundPtr            - the upper bound; the allocated rule reside before
*                                      this rule; if NULL then no upper bound
* @param[in] allocMethod              - the allocation method
* @param[in] defragEnable             - whether to defrag entries in case there is no
*                                      free place for this entry cause of entries
*                                      fragmentation; relevant only if the TCAM entries
*                                      handler was created with partitionEnable = GT_FALSE
*
* @param[out] allocNodePtrPtr          - the allocated rule node
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper
*                                       and lower bounds
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerAllocate
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC                      *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                   *lowerBoundPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                   *upperBoundPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ALLOC_METHOD_ENT allocMethod,
    IN  GT_BOOL                                                defragEnable,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                   **allocNodePtrPtr
);

/**
* @internal prvCpssDxChLpmTcamEntriesHandlerAllocByGivenRule function
* @endinternal
*
* @brief   Allocates a rule by a another given rule.
*
* @param[in] tcamHandlerPtr           - points to TCAM entries handler
* @param[in] entryType                - entry type
* @param[in] allocRulePtr             - given rule to allocate the new rule from it
*
* @param[out] allocNodePtrPtr          - the allocated rule node
*                                       GT_OK if allocation succedded.
*
* @retval GT_ALREADY_EXIST         - if the rule is already allocated.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerAllocByGivenRule
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_STC             *tcamHandlerPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                   *allocRulePtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                   **allocNodePtrPtr
);


/**
* @internal prvCpssDxChLpmTcamEntriesHandlerFree function
* @endinternal
*
* @brief   Frees a TCAM entry.
*
* @param[in] tcamHandlerPtr           - points to TCAM entries handler
* @param[in] entryType                - entry type
* @param[in] allocNodePtr             - the entry to free.
*                                       GT_OK if allocation succedded.
*
* @retval GT_NOT_FOUND             - if the rule wasn't found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerFree
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_STC              *tcamHandlerPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT   entryType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                    *allocNodePtr
);

/**
* @internal prvCpssDxChLpmTcamEntriesHandlerCheckAvailableMem function
* @endinternal
*
* @brief   Checks whether the TCAM has enough available space to add additional
*         entries of specific entry type.
* @param[in] tcamHandlerPtr           - points to TCAM entries handler
* @param[in] entryType                - entry type
* @param[in] numOfRulesNeeded         - number of entries needed
*                                       GT_OK if there is room
*                                       GT_OUT_OF_PP_MEM otherwise
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note numOfIpTcamRules != 0!
*
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerCheckAvailableMem
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_STC              *tcamHandlerPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT   entryType,
    IN  GT_U32                                                  numOfRulesNeeded
);

/**
* @internal prvCpssDxChLpmTcamEntriesHandlerGetRuleIdx function
* @endinternal
*
* @brief   The TCAM entries handler maintains internal array that holds rules (those
*         rules are also associated with the prefixes attached to them).
*         This funtion returns the index of the rule in that array.
* @param[in] tcamHandlerPtr           - points to TCAM entries handler
* @param[in] entryType                - entry type
* @param[in] allocNodePtr             - the rule to get its index
*
* @param[out] ruleIdxPtr               - the rule idx.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerGetRuleIdx
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_STC              *tcamHandlerPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT   entryType,
    IN  PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                    *allocNodePtr,
    OUT GT_32                                                   *ruleIdxPtr
);

/**
* @internal prvCpssDxChLpmTcamEntriesHandlerGetRuleByRuleIdx function
* @endinternal
*
* @brief   The TCAM entries handler maintains internal array that holds rules (those
*         rules are also associated with the prefixes attached to them).
*         This function returns the rule located at the internal rule index in
*         that array.
* @param[in] tcamHandlerPtr           - points to TCAM entries handler
* @param[in] entryType                - entry type
* @param[in] ruleIdx                  - the rule idx.
*
* @param[out] ruleNodePtrPtr           - the rule node of this index
*
* @retval GT_OK                    - if success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerGetRuleByRuleIdx
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_STC              *tcamHandlerPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT   entryType,
    IN  GT_32                                                   ruleIdx,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                    **ruleNodePtrPtr
);

/**
* @internal prvCpssDxChLpmTcamEntriesHandlerSetRealIdxAsAlloc function
* @endinternal
*
* @brief   Marks specific TCAM entry as used and returns the rule associated with
*         this TCAM entry.
* @param[in] tcamHandlerPtr           - points to TCAM entries handler
* @param[in] entryType                - entry type
* @param[in] hwIndex                  - the hwRule index.
*
* @param[out] allocNodePtrPtr          - points to allocated node
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if Tcam handler new mode is used
* @retval GT_NOT_IMPLEMENTED       - others
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerSetRealIdxAsAlloc
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_STC             *tcamHandlerPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT  entryType,
    IN  GT_U32                                                 hwIndex,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                   **allocNodePtrPtr
);

/**
* @internal prvCpssDxChLpmTcamEntriesHandlerGetRuleInfoByRealIdx function
* @endinternal
*
* @brief   The TCAM entries handler maintains internal array that holds rules (those
*         rules are also associated with the prefixes attached to them).
*         This function gets hardware TCAM index and returns the rule associated
*         with this entry.
*         In static partition, the function also returns the type of the entry,
*         according to the pool that the TCAM index resides in.
*         In dynamic partition, the entry type is not returned because in dynamic
*         partitioning, same entry can be used to more than one entry type.
* @param[in] tcamHandlerPtr           - points to TCAM entries handler
* @param[in] hwIndex                  - the rule index.
* @param[in,out] entryTypePtr             - entry type, relevant as input only in case of
*                                      dynamic partition
* @param[in,out] entryTypePtr             - entry type, relevant as output only in case of
*                                      static partition
*
* @param[out] ruleNodePtrPtr           - the rule node of this index
*
* @retval GT_OK                    - if success
* @retval GT_OUT_OF_RANGE          - if the hw index is not in any of the pools
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In dynamic partition, the rule accosiated with the hwIndex is returned.
*       This rule should be deal with care. It must not be used as IPv6 entry
*       if it is not in the first TCAM column.
*
*/
GT_STATUS prvCpssDxChLpmTcamEntriesHandlerGetRuleInfoByRealIdx
(
    IN    PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_STC            *tcamHandlerPtr,
    IN    GT_U32                                                hwIndex,
    INOUT PRV_CPSS_DXCH_LPM_TCAM_ENTRIES_HANDLER_ENTRY_TYPE_ENT *entryTypePtr,
    OUT   PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC                  **ruleNodePtrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTcamEntriesHandlerh */


