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
* @file prvCpssDxChLpmRamMng.h
*
* @brief Implementation of the LPM algorithm
*
* @version   5
********************************************************************************
*/
#ifndef __prvCpssDxChLpmMngh
#define __prvCpssDxChLpmMngh

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*#define LPM_DEBUG*/
#ifdef PRESTERA_DEBUG
#define LPM_DEBUG
#endif /* PRESTERA_DEBUG */

#define MAX_LPM_LEVELS_CNS 17
/**
* @internal prvCpssDxChLpmRamMngRangeInNextPointerFormatSet function
* @endinternal
*
* @brief   This sets the given range in next pointer format
*
* @param[in] rangePtr                 - the range to get from.
* @param[in] lpmEngineMemPtr          - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC which
*                                      holds all the memory information needed for where
*                                      and how to treat the search memory.
*
* @param[out] nextPointerArrayPtr      - the next pointer
*                                       The range Hw value.
*                                       0 for error in the retrival.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMngRangeInNextPointerFormatSet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *rangePtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC        *lpmEngineMemPtr,
    OUT CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC       *nextPointerArrayPtr
);

/*******************************************************************************
* prvCpssDxChLpmRamMngCreateNew
*
* DESCRIPTION:
*       This function creates a new LPM structure, with the pair (0,0) as the
*       default route entry.
*
* INPUTS:
*       nextHopPtr          - A pointer to the next hop entry to be set in the
*                             nextHopEntry field.
*       firstLevelPrefixLen - The first lpm level prefix.
*
*
* RETURNS:
*       A pointer to the new created Bucket, or NULL if allocation failed.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC* prvCpssDxChLpmRamMngCreateNew
(
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopPtr,
    IN GT_U8 firstLevelPrefixLen
);

/**
* @internal prvCpssDxChLpmRamMngInsert function
* @endinternal
*
* @brief   Insert a new entry to the LPM tables.
*
* @param[in] bucketPtr                - Pointer to the root bucket.
* @param[in] addrCurBytePtr           - The Byte Array represnting the Address associated
*                                      with the entry.
* @param[in] prefix                   - The address prefix.
* @param[in] nextPtr                  - A pointer to a next hop/next bucket entry.
* @param[in] nextPtrType              - The type of nextPtr (can be next hop, ECMP/QoS entry
*                                      or bucket)
* @param[in] rootBucketFlag           - Indicates the way to deal with a root bucket (if it is).
* @param[in] lpmEngineMemPtrPtr       points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in,out] updateOldPtr             - (GT_TRUE) this is an update for an already existing entry.
* @param[in,out] bucketTypePtr            - The bucket type of the bucket inserted on the root.
*                                      Possible values:
*                                      REGULAR_BUCKET, COMPRESSED_1, or COMPRESSED_2.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] insertMode               - is the function called in the course of Hot Sync,bulk
*                                      operation or regular
* @param[in] isMcSrcTree              - indicates whether bucketPtr is the toor bucket of a
*                                      multicast source tree
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
* @param[in] protocol                 - the protocol
* @param[in,out] updateOldPtr             - (GT_TRUE) this is an update for an already existing entry.
* @param[in,out] bucketTypePtr            - The bucket type of the bucket inserted on the root.
*                                      Possible values:
*                                      REGULAR_BUCKET, COMPRESSED_1, or COMPRESSED_2.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
*/
GT_STATUS prvCpssDxChLpmRamMngInsert
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
    IN    GT_U8                                         *addrCurBytePtr,
    IN    GT_U32                                        prefix,
    IN    GT_PTR                                        nextPtr,
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT           nextPtrType,
    IN    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT    rootBucketFlag,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC            **lpmEngineMemPtrPtr,
    INOUT GT_BOOL                                       *updateOldPtr,
    INOUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT           *bucketTypePtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC              *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT    insertMode,
    IN    GT_BOOL                                       isMcSrcTree,
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT          protocol
);

/**
* @internal prvCpssDxChLpmRamMngSearch function
* @endinternal
*
* @brief   Search for the existence of a given (address,Prefix) in the given LPM
*         structure.
* @param[in] bucketPtr                - A pointer to the bucket to search in.
* @param[in] addrCurBytePtr           - The Byte Array represnting the Address to search for.
* @param[in] prefix                   - address  length.
*
* @param[out] nextHopPtr               - A pointer to the found next hop entry, or NULL if not
*                                      found.
*                                       GT_OK if found, or
*                                       GT_NOT_FOUND if not found.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMngSearch
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U8                                    *addrCurBytePtr,
    IN  GT_U32                                   prefix,
    OUT GT_PTR                                   *nextHopPtr
);

/**
* @internal prvCpssDxChLpmRamMngSearchOffset function
* @endinternal
*
* @brief   Search for the existence of a given (address,Prefix) in the given LPM
*         structure and return associated hw bucket offset and pointer to the bucket.
* @param[in] bucketPtr                - A pointer to the bucket to search in.
* @param[in] addrCurBytePtr           - The Byte Array represnting the Address to search for.
* @param[in] prefix                   - address  length.
*
* @param[out] lastHwBucketOffset       -
*                                      hw bucket offset
* @param[out] lastBucketPtr            - pointer to the bucket
*                                       GT_OK if found, or
*                                       GT_NOT_FOUND if not found.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMngSearchOffset
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U8                                    *addrCurBytePtr,
    IN  GT_U32                                   prefix,
    OUT GT_U32                                   *lastHwBucketOffset,
    OUT PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  **lastBucketPtr
);

/**
* @internal prvCpssDxChLpmRamMngDoLpmSearch function
* @endinternal
*
* @brief   Search for the existence of a given (address,Prefix) in the given LPM
*         structure. This is a LPM search - meaning if exact search hasn't found
*         data - it is taken from range.
* @param[in] bucketPtr                - A pointer to the bucket to search in.
* @param[in] addrCurBytePtr           - The Byte Array represnting the address to search for.
* @param[in] protocolStack            - protocol stack to work on.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopPtr               - A pointer to the found next hop entry, or NULL if not
*                                      found.
*
* @retval GT_OK                    - if found.
* @retval GT_NOT_FOUND             - if not found.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMngDoLpmSearch
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U8                                    *addrCurBytePtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocolStack,
    OUT GT_U32                                   *prefixLenPtr,
    OUT GT_PTR                                   *nextHopPtr
);

/**
* @internal prvCpssDxChLpmRamMngEntryGet function
* @endinternal
*
* @brief   This function returns the entry indexed 'index', entries are sorted by
*         (address,prefix) key.
* @param[in] bucketPtr                - A pointer to the bucket to search in.
* @param[in] protocol                 - the protocol
* @param[in,out] addrPtr                  - The address associated with the returned next hop pointer.
* @param[in,out] prefixPtr                - The address prefix length.
* @param[in,out] addrPtr                  - The address associated with the returned next hop pointer.
* @param[in,out] prefixPtr                - The address prefix length.
*
* @param[out] nextPtr                  - A pointer to the found next hop entry, or NULL if not
*                                      found.
*                                       GT_OK if the required entry was found, or
*
* @retval GT_NOT_FOUND             - if no more entries where found in the structure.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMngEntryGet
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocol,
    INOUT GT_U8                                    *addrPtr,
    INOUT GT_U32                                   *prefixPtr,
    OUT   GT_PTR                                   *nextPtr
);

/**
* @internal prvCpssDxChLpmRamMngBucketDelete function
* @endinternal
*
* @brief   This function deletes an empty bucket structure from memory.
*
* @param[in] bucketPtr                - A pointer to the bucket to be deleted.
* @param[in] levelPrefix              - The current lpm level prefix.
*
* @param[out] pNextPtr                 - A pointer to the data stored in the Trie.
*                                       GT_OK
*
* @retval GT_BAD_STATE             - if the bucket is not empty (has more then 1 range)
*
* @note This function is called only for non-root buckets, which are fully empty
*
*/
GT_STATUS prvCpssDxChLpmRamMngBucketDelete
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN GT_U8                                    levelPrefix,
    OUT GT_PTR                                  *pNextPtr
);

/**
* @internal prvCpssDxChLpmRamMngEntryDelete function
* @endinternal
*
* @brief   Delete an entry from the LPM tables.
*
* @param[in,out] lpmPtr                   - Pointer to the root bucket.
* @param[in] addrCurBytePtr           - The Byte Array represnting the Address associated
*                                      with the entry.
* @param[in] prefix                   - The address prefix.
* @param[in] rootBucketFlag           - Indicates the way to deal with a root bucket (if it is)
* @param[in] updateHwAndMem           - whether an HW update and memory alloc should take place
* @param[in] lpmEngineMemPtrPtr       points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in,out] bucketTypePtr            - The bucket type of the root bucket after deletion.
*                                      Possible values:
*                                      REGULAR_BUCKET, COMPRESSED_1, COMPRESSED_2, or NEXT_HOP
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
* @param[in] isMcSrcTree              - indicates whether bucketPtr is the toor bucket of a
*                                      multicast source tree
* @param[in] rollBack                 - GT_TRUE: rollback is taking place
*                                      GT_FALSE: otherwise
* @param[in] protocol                 - the protocol
* @param[in,out] lpmPtr                   - Pointer to the root bucket.
* @param[in,out] bucketTypePtr            - The bucket type of the root bucket after deletion.
*                                      Possible values:
*                                      REGULAR_BUCKET, COMPRESSED_1, COMPRESSED_2, or NEXT_HOP
*
* @param[out] delEntryPtr              - The next_pointer structure of the entry associated with
*                                      the deleted (address,prefix).
*                                       GT_OK on success, or
*
* @retval GT_NOT_FOUND             - If (address,prefix) not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMngEntryDelete
(
    INOUT PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT                  *lpmPtr,
    IN    GT_U8                                                     *addrCurBytePtr,
    IN    GT_U32                                                    prefix,
    IN    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT                rootBucketFlag,
    IN    GT_BOOL                                                   updateHwAndMem,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC                        **lpmEngineMemPtrPtr,
    INOUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                       *bucketTypePtr,
    OUT   GT_PTR                                                    *delEntryPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                          *shadowPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr,
    IN    GT_BOOL                                                   isMcSrcTree,
    IN    GT_BOOL                                                   rollBack,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT                      protocol
);

/**
* @internal prvCpssDxChLpmRamMngAllocAvailableMemCheck function
* @endinternal
*
* @brief   Check if there is enough available memory to insert a new
*         Unicast or Multicast address. and if there is allocate it
*         for further use in the insertion.
* @param[in] bucketPtr                - The LPM bucket to check on the LPM insert.
* @param[in] destArr[]                - If holds the unicast address to be inserted.
* @param[in] prefix                   - Holds the  length of destArr.
* @param[in] rootBucketFlag           - Indicates the way to deal with a root bucket (if it is).
* @param[in] lpmEngineMemPtrPtr       - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - type of ip  stack to work on.
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMngAllocAvailableMemCheck
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *bucketPtr,
    IN GT_U8                                        destArr[],
    IN GT_U32                                       prefix,
    IN PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT   rootBucketFlag,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      defragmentationEnable,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr
);

/**
* @internal prvCpssDxChLpmRamMngAllocatedAndBoundMemFree function
* @endinternal
*
* @brief   Free memory that was pre allocated or bound in prvCpssDxChLpmRamMngAllocAvailableMemCheck
*         function. used in case of error in the insertion.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocolStack            - protocol stack to work on.
*
* @retval GT_OK                    - operation finish successfully
* @retval GT_BAD_PARAM             - Bad protocolStack input parameter
*/
GT_STATUS prvCpssDxChLpmRamMngAllocatedAndBoundMemFree
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack
);

/**
* @internal prvCpssDxChLpmRamMngBucketTreeWrite function
* @endinternal
*
* @brief   write an lpm bucket tree to the HW, and if neccessary allocate memory
*         for it - assuming there is enough memory
* @param[in] rootRangePtr             - the range which holds the root bucket of the lpm.
* @param[in] lpmEngineMemPtrPtr       - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] updateType               - is this an update only for the trie, overwrite of
*                                      the whole tree , or update and allocate memory.
* @param[in] protocolStack            - the protocol Stack (relvant only if updateType ==
*                                      PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E)
* @param[in] vrId                     - VR Id (relvant only if updateType ==
*                                      PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E)
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMngBucketTreeWrite
(
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       *rootRangePtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_TYPE_ENT   updateType,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN GT_U32                                       vrId
);

/**
* @internal prvCpssDxChLpmRamMngMemTraverse function
* @endinternal
*
* @brief   a tree traverse function to set/retrive all the memory allocations in this
*         lpm bucket tree. it is done in DFS fashion.
* @param[in] traverseOp               - the traverse operation done on the lpm trie.
* @param[in] memAllocArrayPtr         - the array that holds the information of memory
*                                      allocations.
* @param[in] memAllocArrayIndexPtr    - the index in the array this function should start
*                                      using.
* @param[in] memAllocArraySize        - the size of the above array.
* @param[in] rootRangePtr             - the range which holds the root bucket of the lpm.
* @param[in] setRecoredRootBucketMem  - whether to set/record the root bucket's memory.
* @param[in] lpmEngineMemPtrPtr       - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
*
* @param[out] stopPointIterPtr         - an iterator that is returned to point where we stopped
*                                      in the lpm tree, so we can continue from there in the
*                                      next call.
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note if stopPointIter ==0 we start at the begining.
*       and if the returned stopPointIter==0 the we finished with this tree.
*
*/
GT_STATUS prvCpssDxChLpmRamMngMemTraverse
(
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT    traverseOp,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC     *memAllocArrayPtr,
    IN GT_U32                                       *memAllocArrayIndexPtr,
    IN GT_U32                                       memAllocArraySize,
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       *rootRangePtr,
    IN GT_BOOL                                      setRecoredRootBucketMem,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    OUT GT_UINTPTR                                  *stopPointIterPtr
);

/**
* @internal prvCpssDxChLpmRamMngRootBucketCreate function
* @endinternal
*
* @brief   Create a shadow root bucket for a specific virtual router/forwarder Id
*         and protocol, and write it to the HW.
* @param[in] shadowPtr                - the shadow to work on
* @param[in] vrId                     - The virtual router/forwarder ID
* @param[in] protocol                 - the protocol
* @param[in] defUcNextHopEntryPtr     - the default unicast nexthop
* @param[in] defReservedNextHopEntryPtr - the default reserved range nexthop
* @param[in] defMcNextHopEntryPtr     - the default multicast nexthop
* @param[in] updateHw                 - whether to update the HW
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad input parameters
* @retval GT_OUT_OF_CPU_MEM        - no memory
* @retval GT_FAIL                  - on other failure
*/
GT_STATUS prvCpssDxChLpmRamMngRootBucketCreate
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defUcNextHopEntryPtr,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defReservedNextHopEntryPtr,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defMcNextHopEntryPtr,
    IN GT_BOOL                                      updateHw
);

/**
* @internal prvCpssDxChLpmRamMngRootBucketDelete function
* @endinternal
*
* @brief   This function deletes the root bucket structure from memory.
*
* @param[in] shadowPtr                - the shadow to work on
* @param[in] vrId                     - The virtual router/forwarder ID
* @param[in] protocol                 - the protocol
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - if the bucket is not empty
*/
GT_STATUS prvCpssDxChLpmRamMngRootBucketDelete
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol
);

/**
* @internal prvCpssDxChLpmRamMngVrfEntryUpdate function
* @endinternal
*
* @brief   perform an update of the VRF table
*
* @param[in] vrId                     - the  of the updated VR
* @param[in] protocol                 - the protocol
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDxChLpmRamMngVrfEntryUpdate
(
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr
);

/**
* @internal prvCpssDxChLpmRamMngUnbindBlockFromProtocolAndOctet function
* @endinternal
*
* @brief   Release a memory block from being bound to a specific protocol and octet.
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] protocol                 - the protocol
* @param[in] octetIndex               - the  to whom we want to unbind the found block
* @param[in] blockIndex               - block to release
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamMngUnbindBlockFromProtocolAndOctet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  GT_U32                                  octetIndex,
    IN  GT_U32                                  blockIndex
);

/**
* @internal prvCpssDxChLpmRamMngUnbindBlock function
* @endinternal
*
* @brief   Release a memory block from being bound
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] blockIndex               - start searching from this block
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamMngUnbindBlock
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN  GT_U32                                  blockIndex
);

/**
* @internal prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol function
* @endinternal
*
* @brief   Search for a memory block.
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] protocol                 - the protocol
* @param[in] octetIndex               - the  to whom we want to bind the found block
* @param[in] blockStart               - start searching from this block
*
* @param[out] blockIndexPtr            - (pointer to) the index of the first empty block
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*
* @note We try to associate to a given protocol a block that will not
*       violate full wire speed.
*       If we did not find a block and the allocation mode is
*       PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E
*       we do allocate a block that may cause full wire speed violation.
*
*/
GT_STATUS prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  GT_U32                                  octetIndex,
    IN  GT_U32                                  blockStart,
    OUT GT_U32                                  *blockIndexPtr
);


/**
* @internal prvCpssDxChLpmRamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock function
* @endinternal
*
* @brief   Check if incase we use a swap area, new memory can be added.
*         this means that there is free block near the used block and
*         we can use them to create a new bigger block
* @param[in] oldBucketPtr             - (pointer to) the old bucket we want to extend
* @param[in] newBucketSize            - the size (in lines) of the new bucket
*
* @param[out] freeBlockCanBeFoundPtr   - (pointer to)
*                                      GT_TRUE: empty block can be found if we use swap area
*                                      GT_FALSE: empty block can NOT be found even if we use swap area
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmRamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *oldBucketPtr,
    IN  GT_U32                                  newBucketSize,
    OUT GT_BOOL                                 *freeBlockCanBeFoundPtr
);

/**
* @internal prvCpssDxChLpmRamMngCheckIfShrinkOperationUsefulForDefrag function
* @endinternal
*
* @brief   Check if shrink operation may help for defrag
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtr            - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] protocol                 - the protocol
* @param[in] oldHandleAddr            - the old bucket handle, if the value is 0,
*                                      it means this is a new bucket, no resize
* @param[in] newBucketSize            - the size (in lines) of the new bucket
*
* @param[out] freeBlockCanBeFoundPtr   - (pointer to)
*                                      GT_TRUE: empty block can be found if we shrink memory
*                                      GT_FALSE: empty block can NOT be found even if we shrink memory
* @param[out] bankIndexForShrinkPtr    - (pointer to) the index of the bank to shrink
* @param[out] useSwapAreaPtr           - (pointer to)
*                                      GT_TRUE  - swapArea will be used when doing shrink
*                                      GT_FALSE - swapArea will NOT be used when doing shrink
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmRamMngCheckIfShrinkOperationUsefulForDefrag
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *lpmMemInfoPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  GT_UINTPTR                              oldHandleAddr,
    IN  GT_U32                                  newBucketSize,
    OUT GT_BOOL                                 *freeBlockCanBeFoundPtr,
    OUT GT_U32                                  *bankIndexForShrinkPtr,
    OUT GT_BOOL                                 *useSwapAreaPtr
);

/**
* @internal prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2 function
* @endinternal
*
* @brief   Build the HW representation of the block according to the Shadow,
*         write the data to the HW and update the pointers.
* @param[in] shadowPtr                - (pointer to) the shadow information
*                                      lpmMemInfoPtr       - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] bucketBaseAddress        - the bucket's base address
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] needToFreeAllocationInCaseOfFail - GT_TRUE: need to free memory
*                                      GT_FALSE: NO need to free memory
* @param[in] tempHwAddrHandleToBeFreed - Hw handle to free in case of fail
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
* @param[in] phaseFlag                - GT_TRUE: phase1 update
*                                      GT_FALSE: phase2 update
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      **lpmMemInfoPtrPtr,
    IN GT_U32                                  bucketBaseAddress,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *oldBucketShadowPtr,
    IN GT_BOOL                                 needToFreeAllocationInCaseOfFail,
    IN GT_UINTPTR                              tempHwAddrHandleToBeFreed,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE_ENT     phaseFlag
);
/**
* @internal prvCpssDxChLpmRamMngShrinkFreeOldBucket function
* @endinternal
*
* @brief   Free old bucket HW data.
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldHwAddrHandle          - the bucket's old HW address
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDxChLpmRamMngShrinkFreeOldBucket
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN GT_U32                                  bankIndex,
    IN GT_UINTPTR                              oldHwAddrHandle,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *oldBucketShadowPtr
);

/**
* @internal prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1 function
* @endinternal
*
* @brief   Write old bucket to and from swap area 1
*
* @param[in] shadowPtr                - (pointer to) the shadow information
*                                      lpmMemInfoPtr   - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] oldMemSize               - size of the old bucket in lines
* @param[in] copyToFromSwapArea       - The direction of the copy operation,
*                                      TO or FROM the swap area
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      **lpmMemInfoPtrPtr,
    IN GT_U32                                  octetIndex,
    IN GT_U32                                  bankIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *oldBucketShadowPtr,
    IN GT_U32                                  oldMemSize,
    IN PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FROM_FIRST_SWAP_AREA_ENT    copyToFromSwapArea,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr
);

/**
* @internal prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 function
* @endinternal
*
* @brief   Shrink one block using swap area 2
*
* @param[in] shadowPtr                - (pointer to) the shadow information
*                                      lpmMemInfoPtr   - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] oldMemSize               - size of the old bucket in lines
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*
* @note oldMemSize may be 0 in cases we do not have a resize
*       operation but a new bucket in this case no real use of the
*       oldMemSize parameter
*
*/
GT_STATUS prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      **lpmMemInfoPtrPtr,
    IN GT_U32                                  octetIndex,
    IN GT_U32                                  bankIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *oldBucketShadowPtr,
    IN GT_U32                                  oldMemSize,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr
);

/**
* @internal prvCpssDxChLpmRamMngShrinkOneBlock function
* @endinternal
*
* @brief   Shrink one block element in the bank
*
* @param[in] shadowPtr                - (pointer to) the shadow information
*                                      lpmMemInfoPtr   - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldBucketShadowPtr       - the old SW bucket handle
* @param[in] oldMemSize               - size of the old bucket in lines
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmRamMngShrinkOneBlock
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      **lpmMemInfoPtrPtr,
    IN GT_U32                                  octetIndex,
    IN GT_U32                                  bankIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *oldBucketShadowPtr,
    IN GT_U32                                  oldMemSize,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr
);

/**
* @internal prvCpssDxChLpmRamMngShrinkBank function
* @endinternal
*
* @brief   Shrink the partition
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         (pointer to) the lpm memory linked list holding
*                                      all blocked mapped
* @param[in] octetIndex               - octet index of the bucket we are working on
* @param[in] bankIndex                - the index of the bank to shrink
* @param[in] oldHwHandleAddr          - the old HW bucket handle, if the value is 0,
*                                      it is a new bucket, no resize
* @param[in] newBucketSize            - the size (in lines) of the new bucket
* @param[in] useSwapArea              - GT_TRUE  - swapArea will be used when doing shrink
*                                      GT_FALSE - swapArea will NOT be used when doing shrink
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmRamMngShrinkBank
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmMemInfoPtrPtr,
    IN GT_U32                                  octetIndex,
    IN GT_U32                                  bankIndex,
    IN GT_UINTPTR                              oldHwHandleAddr,
    IN GT_U32                                  newBucketSize,
    IN GT_BOOL                                 useSwapArea,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr
);

/**
* @internal prvCpssDxChLpmRamMngCheckIfMergeBankOperationUsefulForDefrag function
* @endinternal
*
* @brief   Check if merge bank operation may help for defrag
*
* @param[in] shadowPtr                - (pointer to) the shadow information
*                                      lpmEngineMemPtrPtr - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in] protocol                 - the protocol
* @param[in] octetIndex               - octet index of the bucket we are working on
* @param[in] newBucketSize            - the size (in lines) of the new bucket
*
* @param[out] mergeBankCanBeFoundPtr   - (pointer to)
*                                      GT_TRUE: empty block can be found if we merge memory
*                                      GT_FALSE: empty block can NOT be found even if we merge memory
* @param[out] bankIndexForMergePtr     - (pointer to) the index of the bank to merge
* @param[out] octetIndexForMergePtr    - (pointer to) the octet using the bank we are going to merge
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - a potential bank for merge was not found
*/
GT_STATUS prvCpssDxChLpmRamMngCheckIfMergeBankOperationUsefulForDefrag
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      **lpmMemInfoPtrPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  GT_U32                                  octetIndex,
    IN  GT_U32                                  newBucketSize,
    OUT GT_BOOL                                 *mergeBankCanBeFoundPtr,
    OUT GT_U32                                  *bankIndexForMergePtr,
    OUT GT_U32                                  *octetIndexForMergePtr
);

/**
* @internal prvCpssDxChLpmRamMngMoveBlockToNewLocation function
* @endinternal
*
* @brief   Move one block from the bank we need to release to one of the other
*         banks mapped to a given octet
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         - (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] octetIndexForMerge       - octet index of the bucket we are working on
* @param[in] bankIndexForMerge        - the index of the bank to merge
* @param[in] blockHandleAddrForMerge  - the block address to be moved
* @param[in] parentWriteFuncPtr       - a bucket's parent write function in case there is
*                                      a need to update the bucket's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @param[out] bucketRelocatedPtr       - (pointer to) GT_TRUE: the bucket was relocated
*                                      GT_FALSE: the bucket was NOT relocated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDxChLpmRamMngMoveBlockToNewLocation
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC       *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmMemInfoPtrPtr,
    IN GT_U32                                  octetIndexForMerge,
    IN GT_U32                                  bankIndexForMerge,
    IN GT_UINTPTR                              blockHandleAddrForMerge,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr,
    OUT GT_BOOL                                *bucketRelocatedPtr
);

/**
* @internal prvCpssDxChLpmRamMngMoveBlockToNewLocationWithShrink function
* @endinternal
*
* @brief   Move one block from the bank we need to release to one of the other
*         banks mapped to a given octet using shrink
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] octetIndexForMerge       - octet index of the bucket we are working on
* @param[in] bankIndexForMerge        - the index of the bank to merge
* @param[in] blockHandleAddrForMerge  - the block address to be moved
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the bucket's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @param[out] bucketRelocatedPtr       - (pointer to) GT_TRUE: the bucket was relocated
*                                      GT_FALSE: the bucket was NOT relocated
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - an available block was not found
*/
GT_STATUS prvCpssDxChLpmRamMngMoveBlockToNewLocationWithShrink
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmMemInfoPtrPtr,
    IN GT_U32                                  octetIndexForMerge,
    IN GT_U32                                  bankIndexForMerge,
    IN GT_UINTPTR                              blockHandleAddrForMerge,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC   *parentWriteFuncPtr,
    OUT GT_BOOL                                *bucketRelocatedPtr
);

/**
* @internal prvCpssDxChLpmRamMngMergeBank function
* @endinternal
*
* @brief   Merge one bank to the rest of the banks mapped to a given octet
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
* @param[in] protocol                 - the protocol
* @param[in] octetIndexForMerge       - octet index of the bucket we are working on
* @param[in] bankIndexForMerge        - the index of the bank to merge
* @param[in] parentWriteFuncPtr       a bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - merge operation fail
*/
GT_STATUS prvCpssDxChLpmRamMngMergeBank
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                         *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC                       **lpmMemInfoPtrPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT                     protocol,
    IN GT_U32                                                   octetIndexForMerge,
    IN GT_U32                                                   bankIndexForMerge,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC  *parentWriteFuncPtr
);


/**
* @internal mergeCheck function
* @endinternal
*
* @brief   Returns 1 if the two input ranges can be merged to one range.
*
* @param[in] leftRangePtr             - The first range to check for merging.
* @param[in] rightRangePtr            - The second range to check for merging.
* @param[in] levelPrefix              - The current lpm level prefix.
*                                       1 if the two input ranges can be merged to one range.
*/
GT_U8 mergeCheck
(
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *leftRangePtr,
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *rightRangePtr,
    IN GT_U8                                    levelPrefix
);

/**
* @internal delFromTrie function
* @endinternal
*
* @brief   This function deletes an entry from the trie structure.
*
* @param[in] bucketPtr                - A pointer to the bucket to which the deleted address is
*                                      is associated.
* @param[in] startAddr                - The start address associated with the address to be
*                                      deleted.
* @param[in] prefix                   - The address .
* @param[in] trieDepth                - The maximum depth of the trie.
*
* @param[out] delEntryPtr              - A pointer to the next hop entry stored in the trie.
*                                       GT_OK on success, or GT_FAIL if the given (address,prefix) are not found.
*/
GT_STATUS delFromTrie
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U8                                       startAddr,
    IN  GT_U32                                      prefix,
    IN  GT_U8                                       trieDepth,
    OUT GT_PTR                                      *delEntryPtr
);

/**
* @internal getFromTrie function
* @endinternal
*
* @brief   Returns a next hop entry from the trie structure.
*
* @param[in] bucketPtr                - Bucket to which the given entry is associated.
* @param[in] startAddr                - Start Addr of the address to look for in the trie.
* @param[in] prefix                   - The address .
* @param[in] levelPrefix              - The current lpm level prefix.
*
* @retval nextHopPtr               - A pointer to the next hop entry, if found, NULL otherwise.
*/
GT_PTR getFromTrie
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC    *bucketPtr,
    IN GT_U8                                      startAddr,
    IN GT_U32                                     prefix,
    IN GT_U8                                      levelPrefix
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmMngh */


