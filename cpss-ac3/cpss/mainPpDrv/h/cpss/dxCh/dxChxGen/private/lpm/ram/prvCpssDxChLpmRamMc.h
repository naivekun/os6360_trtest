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
* @file prvCpssDxChLpmRamMc.h
*
* @brief This file includes functions declarations for controlling the LPM MC
* tables and structures, and structures definitions for shadow management.
*
* @version   2
********************************************************************************
*/
#ifndef __prvCpssDxChLpmRamMch
#define __prvCpssDxChLpmRamMch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>

/**
* @internal prvCpssDxChLpmRamMcEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrcArr. (CIDR like)
* @param[in] mcRoutePointerPtr        - the mc route entry pointer.
* @param[in] override                 - update or  an existing entry, or create a new one.
* @param[in] insertMode               - how to insert the new entry.
* @param[in] protocolStack            - protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - if inserted group is already exist and new ipGroupRuleIndex
*                                       is not equal to current ipGroupRuleIndex, or
* @retval GT_ERROR                 - if the virtual router does not exist,
*                                       or ipGroupPrefix == 0,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMcEntryAdd
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipGroupPtr,
    IN GT_U32                                       ipGroupPrefix,
    IN GT_U8                                        ipSrcArr[],
    IN GT_U32                                       ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRoutePointerPtr,
    IN GT_BOOL                                      override,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      defragmentationEnable
);


/**
* @internal prvCpssDxChLpmSip6RamMcEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrcArr. (CIDR like)
* @param[in] mcRoutePointerPtr        - the mc route entry pointer.
* @param[in] override                 - update or  an existing entry, or create a new one.
* @param[in] insertMode               - how to insert the new entry.
* @param[in] protocolStack            - protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - if inserted group is already exist and new ipGroupRuleIndex
*                                       is not equal to current ipGroupRuleIndex, or
* @retval GT_ERROR                 - if the virtual router does not exist,
*                                       or ipGroupPrefix == 0,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmSip6RamMcEntryAdd
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipGroupPtr,
    IN GT_U32                                       ipGroupPrefix,
    IN GT_U8                                        ipSrcArr[],
    IN GT_U32                                       ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRoutePointerPtr,
    IN GT_BOOL                                      override,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      defragmentationEnable
);

/**
* @internal prvCpssDxChLpmRamMcEntryDelete function
* @endinternal
*
* @brief   To delete a particular mc route
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrcArr. (CIDR like)
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*                                      roollBack                - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
* @param[in] rollBackSrcBucketPtr     - pointer to src bucket.(used only for rollback)
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to delete the all the multicast entries, call this function
*       with ipSrcArr[0] = ipSrcPrefix = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamMcEntryDelete
(
    IN GT_U32                                   vrId,
    IN GT_U8                                    *ipGroupPtr,
    IN GT_U32                                   ipGroupPrefix,
    IN GT_U8                                    ipSrcArr[],
    IN GT_U32                                   ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr,
    IN GT_BOOL                                  rollBack,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *rollBackSrcBucketPtr
);

/**
* @internal prvCpssDxChLpmSip6RamMcEntryDelete function
* @endinternal
*
* @brief   To delete a particular mc route
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroupPtr               - The IP multicast group address.
* @param[in] ipGroupPrefix            - The number of bits that are actual valid in,
*                                      the ipGroup. (CIDR like)
* @param[in] ipSrcArr[]               - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefix              - The number of bits that are actual valid in,
*                                      the ipSrcArr. (CIDR like)
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*                                      roollBack                - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
* @param[in] rollBackSrcBucketPtr     - pointer to src bucket.(used only for rollback)
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to delete the all the multicast entries, call this function
*       with ipSrcArr[0] = ipSrcPrefix = 0.
*
*/
GT_STATUS prvCpssDxChLpmSip6RamMcEntryDelete
(
    IN GT_U32                                   vrId,
    IN GT_U8                                    *ipGroupPtr,
    IN GT_U32                                   ipGroupPrefix,
    IN GT_U8                                    ipSrcArr[],
    IN GT_U32                                   ipSrcPrefix,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr,
    IN GT_BOOL                                  rollBack,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *rollBackSrcBucketPtr
);

/**
* @internal prvCpssDxChLpmRamMcEntriesFlush function
* @endinternal
*
* @brief   flushes the multicast IP Routing table and stays with the default entry
*         only.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] protocolStack            - protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMcEntriesFlush
(
    IN GT_U32                               vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
);

/**
* @internal prvCpssDxChLpmRamMcEntryGetNext function
* @endinternal
*
* @brief   This function returns the next muticast (ipSrc,ipGroup) entry.
*
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixPtr         - ipGroup prefix length.
* @param[in,out] ipSrcPtr                 - The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixPtr           - ipSrc prefix length.
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in,out] ipGroupPtr               - The next ip Group address.
* @param[in,out] ipGroupPrefixPtr         - ipGroup prefix length.
* @param[in,out] ipSrcPtr                 - The next ip Source address. (must be 16 x GT_U8!)
* @param[in,out] ipSrcPrefixPtr           - ipSrc prefix length.
*
* @param[out] mcRoutePointerPtr        the mc route entry (NH) pointer.
*                                       GT_OK if found, or
*
* @retval GT_NOT_FOUND             - If the given address is the last one on the IP-Mc table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (ipGroup,ipGroupPrefix) must be a valid values, it
*       means that they exist in the IP-Mc Table, unless this is the first
*       call to this function, then the value of (ipGroupPrefix) is (0).
*
*/
GT_STATUS prvCpssDxChLpmRamMcEntryGetNext
(
    IN    GT_U32                                    vrId,
    INOUT GT_U8                                     *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixPtr,
    INOUT GT_U8                                     *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRoutePointerPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocolStack,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr
);

/**
* @internal prvCpssDxChLpmRamMcEntrySearch function
* @endinternal
*
* @brief   This function searches for the route entries associates with a given
*         (S,G), prefixes.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefix            - ipGroupPtr prefix length.
* @param[in] ipSrcArr[]               - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefix              - ipSrcArr prefix length.
* @param[in] protocolStack            - the protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] ipNextHopEntryPtr        - A pointer to a next hop struct including the HW
*                                      pointer
*                                       GT_OK if successful, or
*                                       GT_NOT_FOUND if the given prefix was not found.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Calling this function with ipGroupPrefix = 0, will return the default
*       Mc route entries.
*
*/
GT_STATUS prvCpssDxChLpmRamMcEntrySearch
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *ipGroupPtr,
    IN GT_U32                                       ipGroupPrefix,
    IN GT_U8                                        ipSrcArr[],
    IN GT_U32                                       ipSrcPrefix,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *ipNextHopEntryPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr
);

/**
* @internal prvCpssDxChLpmRamMcDefRouteGet function
* @endinternal
*
* @brief   retrieves the default MC route for the given VR router.
*
* @param[in] vrId                     - The  to which this tree belongs to.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol
*
* @param[out] defMcRoutePointerPtr     - A pointer to the default mc route
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDxChLpmRamMcDefRouteGet
(
    IN  GT_U32                                    vrId,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC **defMcRoutePointerPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
);

/**
* @internal prvCpssDxChLpmRamMcTraverse function
* @endinternal
*
* @brief   a tree traverse function to set/retrive/count all the memory allocations
*         in this mc , and it's lpm src tree. it is done in DFS fashion.
* @param[in] traverseOp               - the traverse operation needed.
* @param[in] memAllocArrayPtr         - the array that holds the information of memory
*                                      allocations.
* @param[in] memAllocArrayIndexPtr    - the index in the array this function should start
*                                      using.
* @param[in] memAllocArraySize        - the size of the above array.
* @param[in] vrId                     - The  to which this table belongs to.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocol                 - the protocol
*
* @param[out] stopPointIterPtr         - an iterator that is returned to point where we stopped
*                                      in the lpm tree, so we can continue from there in the
*                                      next call.
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
*
* @note if stopPointIter ==0 we start at the begining.
*       and if the returned stopPointIter==0 the we finished with this tree.
*
*/
GT_STATUS prvCpssDxChLpmRamMcTraverse
(
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT    traverseOp,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC     *memAllocArrayPtr,
    IN GT_U32                                       *memAllocArrayIndexPtr,
    IN GT_U32                                       memAllocArraySize,
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    OUT GT_UINTPTR                                  *stopPointIterPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol
);

/**
* @internal prvCpssDxChIpLpmRamMcEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific MC entry
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] protocol                 - type of  stack to work on.
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamMcEntryActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT          protocol,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmRamMch */


