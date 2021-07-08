/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip6LpmRamMng.h
*
* DESCRIPTION:
*       Definitions of the SIP6 LPM algorithm
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvCpssDxChSip6LpmMngh
#define __prvCpssDxChSip6LpmMngh

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*#define LPM_DEBUG*/
#ifdef PRESTERA_DEBUG
#define LPM_DEBUG
#endif /* PRESTERA_DEBUG */

/*******************************************************************************
* prvCpssDxChLpmSip6RamMngInsert
*
* DESCRIPTION:
*       Insert a new entry to the LPM tables.
*
* INPUTS:
*       bucketPtr         - Pointer to the root bucket.
*       addrCurBytePtr    - The Byte Array represnting the Address associated
*                           with the entry.
*       prefix            - The address prefix.
*       nextPtr           - A pointer to a next hop/next bucket entry.
*       nextPtrType       - The type of nextPtr (can be next hop, ECMP/QoS entry
*                           or bucket)
*       rootBucketFlag    - Indicates the way to deal with a root bucket (if it is).
*       lpmEngineMemPtrPtr- points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                           which holds all the memory information needed for where and
*                           how to allocate search memory for each of the lpm levels
*       updateOldPtr      - (GT_TRUE) this is an update for an already existing entry.
*       bucketTypePtr     - The bucket type of the bucket inserted on the root.
*                           Possible values:
*                           REGULAR_BUCKET, COMPRESSED_1, or COMPRESSED_2.
*       shadowPtr         - the shadow relevant for the devices asked to act on.
*       insertMode        - is the function called in the course of Hot Sync,bulk
*                           operation or regular
*       isMcSrcTree       - indicates whether bucketPtr is the toor bucket of a
*                           multicast source tree
*       protocol          - the protocol
*
* OUTPUTS:
*       updateOldPtr      - (GT_TRUE) this is an update for an already existing entry.
*       bucketTypePtr     - The bucket type of the bucket inserted on the root.
*                           Possible values:
*                           REGULAR_BUCKET, COMPRESSED_1, or COMPRESSED_2.
*
* RETURNS:
*       GT_OK on success, or
*       GT_OUT_OF_CPU_MEM - if failed to allocate CPU memory, or
*       GT_OUT_OF_PP_MEM - if failed to allocate PP memory.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChLpmSip6RamMngInsert
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
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT          protocol
);

/*******************************************************************************
* prvCpssDxChLpmSip6RamMngEntryDelete
*
* DESCRIPTION:
*       Delete an entry from the LPM tables.
*
* INPUTS:
*       lpmPtr            - Pointer to the root bucket.
*       addrCurBytePtr    - The Byte Array represnting the Address associated
*                           with the entry.
*       prefix            - The address prefix.
*       rootBucketFlag    - Indicates the way to deal with a root bucket (if it is)
*       updateHwAndMem    - whether an HW update and memory alloc should take place
*       lpmEngineMemPtrPtr- points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                           which holds all the memory information needed for where and
*                           how to allocate search memory for each of the lpm levels
*       bucketTypePtr     - The bucket type of the root bucket after deletion.
*                           Possible values:
*                           REGULAR_BUCKET, COMPRESSED_1, COMPRESSED_2, or NEXT_HOP
*       shadowPtr         - the shadow relevant for the devices asked to act on.
*       isMcSrcTree       - indicates whether bucketPtr is the toor bucket of a
*                           multicast source tree
*       rollBack          - GT_TRUE: rollback is taking place
*                           GT_FALSE: otherwise
*       protocol          - the protocol
*
* OUTPUTS:
*       lpmPtr            - Pointer to the root bucket.
*       bucketTypePtr     - The bucket type of the root bucket after deletion.
*                           Possible values:
*                           REGULAR_BUCKET, COMPRESSED_1, COMPRESSED_2, or NEXT_HOP
*       delEntryPtr       - The next_pointer structure of the entry associated with
*                           the deleted (address,prefix).
*
* RETURNS:
*       GT_OK on success, or
*       GT_NOT_FOUND             - If (address,prefix) not found.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChLpmSip6RamMngEntryDelete
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
    IN    GT_BOOL                                                   isMcSrcTree,
    IN    GT_BOOL                                                   rollBack,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT                      protocol
);


/*******************************************************************************
* prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck
*
* DESCRIPTION:
*       Check if there is enough available memory to insert a new
*       Unicast or Multicast address and if there is allocate it
*       for further use in the insertion.
*
* INPUTS:
*       bucketPtr       - The LPM bucket to check on the LPM insert.
*       destArr         - If holds the unicast address to be inserted.
*       prefix          - Holds the prefix length of destArr.
*       rootBucketFlag  - Indicates the way to deal with a root bucket (if it is).
*       lpmEngineMemPtrPtr - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                         which holds all the memory information needed for where and
*                         how to allocate search memory for each of the lpm levels
*       shadowPtr       - the shadow relevant for the devices asked to act on.
*       protocol        - type of ip protocol stack to work on.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - If there is enough memory for the insertion.
*       GT_OUT_OF_PP_MEM         - otherwise.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*        Check available memory is done for group of nodes per octet
*
*******************************************************************************/
GT_STATUS prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC      *bucketPtr,
    IN GT_U8                                        destArr[],
    IN GT_U32                                       prefix,
    IN PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT   rootBucketFlag,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol
);


/*******************************************************************************
* prvCpssDxChLpmSip6RamMngRootBucketCreate
*
* DESCRIPTION:
*       Create a shadow root bucket for a specific virtual router/forwarder Id
*       and protocol, and write it to the HW.
*
* INPUTS:
*       shadowPtr                   - the shadow to work on
*       vrId                        - The virtual router/forwarder ID
*       protocol                    - the protocol
*       defUcNextHopEntryPtr        - the default unicast nexthop
*       defReservedNextHopEntryPtr  - the default reserved range nexthop
*       defMcNextHopEntryPtr        - the default multicast nexthop
*       updateHw                    - whether to update the HW
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                   - on success
*       GT_BAD_PARAM            - on bad input parameters
*       GT_OUT_OF_CPU_MEM       - no memory
*       GT_FAIL                 - on other failure
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS prvCpssDxChLpmSip6RamMngRootBucketCreate
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocol,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defUcNextHopEntryPtr,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defReservedNextHopEntryPtr,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *defMcNextHopEntryPtr,
    IN GT_BOOL                                      updateHw
);

/*******************************************************************************
* prvCpssDxChLpmSip6RamMngCreateNew
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
PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC* prvCpssDxChLpmSip6RamMngCreateNew
(
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopPtr,
    IN GT_U8 firstLevelPrefixLen
);


/**
* @internal prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree
*           function
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
GT_STATUS prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack
);

/*******************************************************************************
* prvCpssDxChLpmGroupOfNodesWrite
*
* DESCRIPTION:
*       Write an LPM Group Of nodes to the HW.
*
* APPLICABLE DEVICES:
*        Falcon.
*
* NOT APPLICABLE DEVICES:
*         xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2
*
* INPUTS:
*       devNum              - the device number
*       lpmGroupLineOffset  - group of nodes offset from the LPM base in LPM lines
*       groupOfNodesPtr     - group of nodes content
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - one of the parameters with bad value
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChLpmGroupOfNodesWrite
(
    IN GT_U8                                               devNum,
    IN GT_U32                                              lpmGroupLineOffset,
    IN OUT PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC      *groupOfNodesPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChSip6LpmMngh */

