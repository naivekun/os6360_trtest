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
* @file prvCpssDxChLpmRamUc.c
*
* @brief This file includes functions declarations for controlling the LPM UC
* tables and structures, and structures definitions for shadow
* management.
*
* @version   8
********************************************************************************
*/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMng.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamUc.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMc.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>

/****************************************************************************
* Debug                                                                     *
****************************************************************************/

extern void * cpssOsLpmMalloc
(
    IN GT_U32 size
);

extern void cpssOsLpmFree
(
    IN void* const memblock
);

/**
* @internal prvCpssDxChLpmSip6RamUcEntryAdd function
* @endinternal
*
* @brief   creates a new or override an exist route entry in the routing
*         structures.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] destPtr                  - The destination address of this route.
* @param[in] prefix                   - The number of bits that are actual valid in,
*                                      the destPtr.
* @param[in] nextHopPointerPtr        - The next hop pointer to be associated with the given
* @param[in] prefix
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] override                 - whether this is an  or a new insert.
* @param[in] protocolStack            - type of protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given nextHopId is not valid, or
* @retval GT_ALREADY_EXIST         - If overide is false and entry already exist, or
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmSip6RamUcEntryAdd
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *destPtr,
    IN GT_U32                                       prefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopPointerPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      override,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *pBucket;/* the 1st level bucket   */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType; /* the 1st level bucket
                                                          type  */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT nextBucketType; /* next level bucket
                                                          type  */
    GT_BOOL alreadyExists = GT_FALSE; /* Indicates if the given prefix      */
                                      /* already exists in the LPM structure. */
    GT_PTR  dummy;

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *allocNextHopPointerPtr=NULL;
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC  **freeMemList;

    GT_STATUS retVal = GT_OK;
    GT_STATUS retVal2 = GT_OK;
    GT_U32 i;
    GT_U32 maxMemSize;
    GT_U32 shareDevListLen;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */

    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

    /* first check the protocol was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* Check if the given virtual router already exists */
    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
       (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* verify that unicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    for (i = 0; i < shareDevListLen; i++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        CPSS_API_LOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if (nextHopPointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E)
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[i])->fineTuning.tableSize.routerNextHop;
        }
        else /* PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E*/
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[i])->fineTuning.tableSize.ecmpQos;
        }
        if (nextHopPointerPtr->routeEntryBaseMemAddr >= maxMemSize)
        {
           /*Unlock the access to per device data base*/
           CPSS_API_UNLOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    /* if we're in bulk insertion we're allowed to free (reuse) only new
       memory which is not update in the HW */
    freeMemList = (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)?
                  &shadowPtr->freeMemListDuringUpdate:
                  &shadowPtr->freeMemListEndOfUpdate;

    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             freeMemList,shadowPtr);
    if (retVal != GT_OK)
        return (retVal);

    pBucket = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
    rootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack];

    /* Check if an entry with the same (address,prefix) already exists,  */
    /* if yes, check the override param.                                 */
    dummy = (GT_PTR)allocNextHopPointerPtr;
    retVal = prvCpssDxChLpmRamMngSearch(pBucket,destPtr,prefix, &dummy);
    allocNextHopPointerPtr = (PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *)dummy;
    if(retVal == GT_OK)
    {
        if(override == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }

        alreadyExists = GT_TRUE;
    }
    else
    {
        /* new entry allocate a new nexthop pointer */
        allocNextHopPointerPtr =
            cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

        if (allocNextHopPointerPtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* copy the next hop pointer fields */
    cpssOsMemCpy(allocNextHopPointerPtr,nextHopPointerPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];
    shadowPtr->neededMemoryListLen = 0;
    shadowPtr->neededMemoryCurIdx = 0;

    /* if we need to update the shadow only there is no need to check memory
       space */
    if (insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)
    {
        retVal = prvCpssDxChLpmSip6RamMngAllocAvailableMemCheck(pBucket,destPtr,prefix,
                                                                PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                                                lpmEngineMemPtr,
                                                                shadowPtr,
                                                                protocolStack);
        if (retVal != GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            if (alreadyExists == GT_FALSE)
                cpssOsLpmFree(allocNextHopPointerPtr);
            return (retVal);
        }
    }

    PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(allocNextHopPointerPtr->routeEntryMethod,nextBucketType);

    /* pointer is set to the first gon */
    lpmEngineMemPtr++;
    /* Insert the complete address to the lpm structure */
    retVal = prvCpssDxChLpmSip6RamMngInsert(pBucket, destPtr, prefix,
                                        allocNextHopPointerPtr,
                                        nextBucketType,
                                        PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                        lpmEngineMemPtr,&alreadyExists,
                                        &rootBucketType, shadowPtr, insertMode,GT_FALSE,protocolStack);
    if (retVal != GT_OK)
    {
        shadowPtr->neededMemoryListLen = 0;
        prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                        freeMemList,shadowPtr);

        /* insert was not successful. Activate rollback and free resources */
        prvCpssDxChLpmRamSip6UcEntryDel(vrId,
                                        destPtr,
                                        prefix,
                                        GT_TRUE,
                                        protocolStack,
                                        shadowPtr,
                                        GT_TRUE);
        if (alreadyExists == GT_FALSE)
            cpssOsLpmFree(allocNextHopPointerPtr);

        /* we fail in allocation, reset pending array */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

        if (insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)
        {
            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }
            /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
        }

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if (retVal == GT_OK)
    {
        /* Now update the UC first table according      */
        /* to the first level of the lpm structure.     */
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack] =
            rootBucketType;

        /* check that all the pre-allocated memory was used */
        if (shadowPtr->neededMemoryCurIdx != shadowPtr->neededMemoryListLen)
        {
            /* should never! - meaning we didn't use all preallcoated memory*/
            prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                            freeMemList,shadowPtr);

            /* reset pending array */
           cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            if (alreadyExists == GT_FALSE)
                cpssOsLpmFree(allocNextHopPointerPtr);
            shadowPtr->neededMemoryListLen = 0;

            if (insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)
            {
                /* free the allocated/bound RAM memory */
                retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
                if (retVal2!=GT_OK)
                {
                    return retVal2;
                }
                /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                             sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    shadowPtr->neededMemoryListLen = 0;

    if ((retVal != GT_OK) && (alreadyExists == GT_FALSE))
        cpssOsLpmFree(allocNextHopPointerPtr);

    retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                              freeMemList,shadowPtr);
    if (retVal == GT_OK)
    {
        if (retVal2==GT_OK)
        {
            /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
               next call to ADD will set this array with new values of allocated/bound blocks */
            cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr, 0, sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));
            cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

            if (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E)
            {
                /* update counters for UC allocation */
                retVal2 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                    shadowPtr->pendingBlockToUpdateArr,
                                                                    shadowPtr->protocolCountersPerBlockArr,
                                                                    shadowPtr->pendingBlockToUpdateArr,
                                                                    protocolStack,
                                                                    shadowPtr->numOfLpmMemories);
                if (retVal2!=GT_OK)
                {
                     /* reset pending array for future use */
                     cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                    return retVal2;

                }
            }
            else
            {
                /* in case of bulk operation first need to collect all updates needed
                   so do not reset the pendingBlockToUpdateArr and do not update the protocolCountersPerBlockArr
                   this will be done in prvCpssDxChLpmRamIpv4UcPrefixBulkAdd/prvCpssDxChLpmRamIpv6UcPrefixBulkAdd */
                if (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)
                {
                    /* keep values for recostruct */
                    cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                                 sizeof(shadowPtr->pendingBlockToUpdateArr));
                }
            }
        }
        return retVal2;
    }

    return retVal;

}


/**
* @internal prvCpssDxChLpmRamUcEntryAdd function
* @endinternal
*
* @brief   creates a new or override an exist route entry in the routing
*         structures.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] destPtr                  - The destination address of this route.
* @param[in] prefix                   - The number of bits that are actual valid in,
*                                      the destPtr.
* @param[in] nextHopPointerPtr        - The next hop pointer to be associated with the given
* @param[in] prefix
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] override                 - whether this is an  or a new insert.
* @param[in] protocolStack            - type of protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given nextHopId is not valid, or
* @retval GT_ALREADY_EXIST         - If overide is false and entry already exist, or
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntryAdd
(
    IN GT_U32                                       vrId,
    IN GT_U8                                        *destPtr,
    IN GT_U32                                       prefix,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopPointerPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode,
    IN GT_BOOL                                      override,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      defragmentationEnable
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *pBucket;/* the 1st level bucket   */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType; /* the 1st level bucket
                                                          type  */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT nextBucketType; /* next level bucket
                                                          type  */
    GT_BOOL alreadyExists = GT_FALSE; /* Indicates if the given prefix      */
                                      /* already exists in the LPM structure. */
    GT_PTR  dummy;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC writeFuncData;

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *allocNextHopPointerPtr=NULL;
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC  **freeMemList;

    GT_STATUS retVal = GT_OK;
    GT_STATUS retVal2 = GT_OK;
    GT_U32 i;
    GT_U32 maxMemSize;
    GT_U32 shareDevListLen;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */

    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

    /* first check the protocol was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* Check if the given virtual router already exists */
    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
       (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* verify that unicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* update the info for the parent write function */
    parentWriteFuncInfo.bucketParentWriteFunc =
        prvCpssDxChLpmRamUpdateVrTableFuncWrapper;

    writeFuncData.shadowPtr = shadowPtr;
    writeFuncData.protocol = protocolStack;
    writeFuncData.vrId = vrId;
    parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    for (i = 0; i < shareDevListLen; i++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        CPSS_API_LOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if (nextHopPointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E)
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[i])->fineTuning.tableSize.routerNextHop;
        }
        else /* PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E or PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E */
        {
            maxMemSize = PRV_CPSS_DXCH_PP_MAC(shareDevsList[i])->fineTuning.tableSize.ecmpQos;
        }
        if (nextHopPointerPtr->routeEntryBaseMemAddr >= maxMemSize)
        {
           /*Unlock the access to per device data base*/
           CPSS_API_UNLOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(shareDevsList[i],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    /* if we're in bulk insertion we're allowed to free (reuse) only new
       memory which is not update in the HW */
    freeMemList = (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)?
                  &shadowPtr->freeMemListDuringUpdate:
                  &shadowPtr->freeMemListEndOfUpdate;

    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             freeMemList,shadowPtr);
    if (retVal != GT_OK)
        return (retVal);

    pBucket = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
    rootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack];

    /* Check if an entry with the same (address,prefix) already exists,  */
    /* if yes, check the override param.                                 */
    dummy = (GT_PTR)allocNextHopPointerPtr;
    retVal = prvCpssDxChLpmRamMngSearch(pBucket,destPtr,prefix, &dummy);
    allocNextHopPointerPtr = (PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *)dummy;
    if(retVal == GT_OK)
    {
        if(override == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }

        alreadyExists = GT_TRUE;
    }
    else
    {
        /* new entry allocate a new nexthop pointer */
        allocNextHopPointerPtr =
            cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

        if (allocNextHopPointerPtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* copy the next hop pointer fields */
    cpssOsMemCpy(allocNextHopPointerPtr,nextHopPointerPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];
    shadowPtr->neededMemoryListLen = 0;
    shadowPtr->neededMemoryCurIdx = 0;

    /* if we need to update the shadow only there is no need to check memory
       space */
    if (insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)
    {
        /* Check memory availability.       */
         retVal = prvCpssDxChLpmRamMngAllocAvailableMemCheck(pBucket,destPtr,prefix,
                                                            PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                                            lpmEngineMemPtr,
                                                            shadowPtr,
                                                            protocolStack,
                                                            insertMode,
                                                            defragmentationEnable,
                                                            &parentWriteFuncInfo);
        if (retVal != GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            if (alreadyExists == GT_FALSE)
                cpssOsLpmFree(allocNextHopPointerPtr);
            return (retVal);
        }
    }

    PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(allocNextHopPointerPtr->routeEntryMethod,nextBucketType);

    /* Insert the complete address to the lpm structure */
    retVal = prvCpssDxChLpmRamMngInsert(pBucket, destPtr, prefix,
                                        allocNextHopPointerPtr,
                                        nextBucketType,
                                        PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                        lpmEngineMemPtr,&alreadyExists,
                                        &rootBucketType, shadowPtr, insertMode,
                                        GT_FALSE,&parentWriteFuncInfo,protocolStack);
    if (retVal != GT_OK)
    {
        shadowPtr->neededMemoryListLen = 0;
        prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                        freeMemList,shadowPtr);

        /* insert was not successful. Activate rollback and free resources */
        prvCpssDxChLpmRamUcEntryDel(vrId,
                                    destPtr,
                                    prefix,
                                    GT_TRUE,
                                    protocolStack,
                                    shadowPtr,
                                    GT_TRUE);
        if (alreadyExists == GT_FALSE)
            cpssOsLpmFree(allocNextHopPointerPtr);

        /* we fail in allocation, reset pending array */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

        if (insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)
        {
            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }
            /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
        }

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if (retVal == GT_OK)
    {
        /* Now update the UC first table according      */
        /* to the first level of the lpm structure.     */
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack] =
            rootBucketType;
        /* don't touch the hw in a hot sync process , it's done in the end */
        if (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E)
        {
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, protocolStack, shadowPtr);
            if (retVal != GT_OK)
            {
                shadowPtr->neededMemoryListLen = 0;
                prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                freeMemList,shadowPtr);

                /* insert was not successful. Activate rollback and free resources */
                prvCpssDxChLpmRamUcEntryDel(vrId,
                                            destPtr,
                                            prefix,
                                            GT_TRUE,
                                            protocolStack,
                                            shadowPtr,
                                            GT_TRUE);
                if (alreadyExists == GT_FALSE)
                    cpssOsLpmFree(allocNextHopPointerPtr);

                 /* we fail in allocation, reset pending array */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                /* free the allocated/bound RAM memory */
                retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
                if (retVal2!=GT_OK)
                {
                    return retVal2;
                }
                /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                             sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

                return retVal;
            }
        }
        else
        {
            /* indicate the VR HW wasn't updated and needs update */
            shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_TRUE;
        }
        /* check that all the pre-allocated memory was used */
        if (shadowPtr->neededMemoryCurIdx != shadowPtr->neededMemoryListLen)
        {
            /* should never! - meaning we didn't use all preallcoated memory*/
            prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                            freeMemList,shadowPtr);

            /* reset pending array */
           cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            if (alreadyExists == GT_FALSE)
                cpssOsLpmFree(allocNextHopPointerPtr);
            shadowPtr->neededMemoryListLen = 0;

            if (insertMode != PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E)
            {
                /* free the allocated/bound RAM memory */
                retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocolStack);
                if (retVal2!=GT_OK)
                {
                    return retVal2;
                }
                /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                             sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    shadowPtr->neededMemoryListLen = 0;

    if ((retVal != GT_OK) && (alreadyExists == GT_FALSE))
        cpssOsLpmFree(allocNextHopPointerPtr);

    retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                              freeMemList,shadowPtr);
    if (retVal == GT_OK)
    {
        if (retVal2==GT_OK)
        {
            /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
               next call to ADD will set this array with new values of allocated/bound blocks */
            cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr, 0, sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));
            cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

            if (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E)
            {
                /* update counters for UC allocation */
                retVal2 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                    shadowPtr->pendingBlockToUpdateArr,
                                                                    shadowPtr->protocolCountersPerBlockArr,
                                                                    shadowPtr->pendingBlockToUpdateArr,
                                                                    protocolStack,
                                                                    shadowPtr->numOfLpmMemories);
                if (retVal2!=GT_OK)
                {
                     /* reset pending array for future use */
                     cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                    return retVal2;
                }
            }
            else
            {
                /* in case of bulk operation first need to collect all updates needed
                   so do not reset the pendingBlockToUpdateArr and do not update the protocolCountersPerBlockArr
                   this will be done in prvCpssDxChLpmRamIpv4UcPrefixBulkAdd/prvCpssDxChLpmRamIpv6UcPrefixBulkAdd */
                if (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)
                {
                    /* keep values for recostruct */
                    cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                                 sizeof(tempPendingBlockToUpdateArr));
                }
            }
        }
        return retVal2;
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamSip6UcEntryDel function
* @endinternal
*
* @brief   Delete address/prefix unicast address from a practicular virtual router
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] destPtr                  - The destination address of this route.
* @param[in] prefix                   - The number of bits that are actual valid in destPtr
* @param[in] updateHwAndMem           - whether an HW update and memory alloc should take place
* @param[in] protocolStack            - type of protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] roolBack                 - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - if the required vrId is not valid, or
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamSip6UcEntryDel
(
    IN GT_U32                               vrId,
    IN GT_U8                                *destPtr,
    IN GT_U32                               prefix,
    IN GT_BOOL                              updateHwAndMem,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_BOOL                              roolBack
)
{
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT lpmPtr;/*the LPM structure to      */
                                                    /* delete from.             */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *delEntry = NULL;/* Next pointer struct.   */
    GT_PTR dummy;
    GT_STATUS retVal = GT_OK;
    GT_STATUS retVal2 = GT_OK;

    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType; /* The bucket type of the 1st level LPM */
                                    /* bucket.                              */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;

    /* first check that the protocol was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* can't delete the default route. only overwrite it */
    if(prefix == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check if the given virtual router already exists */
    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)||
       (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* verify that unicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if (retVal != GT_OK)
        return (retVal);

    lpmPtr.nextBucket = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

    dummy = (GT_PTR)delEntry;
    rootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack];
    retVal = prvCpssDxChLpmSip6RamMngEntryDelete(&lpmPtr,destPtr,prefix,
                                                 PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                                 updateHwAndMem,lpmEngineMemPtr,
                                                 &rootBucketType,&dummy,
                                                 shadowPtr,
                                                 GT_FALSE,roolBack,protocolStack);
    delEntry = (PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *)dummy;

    if (retVal == GT_OK)
    {
        /* ok first free the next hop pointer */
        if (roolBack == GT_FALSE)
        {
            cpssOsLpmFree(delEntry);
        }

        /* update the root bucket type */
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack] =
            rootBucketType;
    }

    retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                              &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if(retVal == GT_OK)
    /* in case of bulk operation first need to collect all updates needed
       so do not reset the pendingBlockToUpdateArr and do not update the protocolCountersPerBlockArr
       this will be done in prvCpssDxChLpmRamIpv4UcPrefixBulkDel/prvCpssDxChLpmRamIpv6UcPrefixBulkDel

       in case of rollBack no need to update counters in the "delete" operation
       since the counters were not updated in the "add" operation.
       counters are updated at the end of operation in case of success */
    if ((updateHwAndMem==GT_TRUE)&&(roolBack==GT_FALSE))
    {
        if (retVal2 == GT_OK)
        {
            retVal2 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                protocolStack,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal2!=GT_OK)
            {
                return retVal2;

            }
        }
        else
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
        }
    }

    if(retVal == GT_OK)
        return retVal2;

    return retVal;
}




/**
* @internal prvCpssDxChLpmRamUcEntryDel function
* @endinternal
*
* @brief   Delete address/prefix unicast address from a practicular virtual router
*
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] destPtr                  - The destination address of this route.
* @param[in] prefix                   - The number of bits that are actual valid in destPtr
* @param[in] updateHwAndMem           - whether an HW update and memory alloc should take place
* @param[in] protocolStack            - type of protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] roolBack                 - GT_TRUE: rollback is taking place.
*                                      GT_FALSE: otherwise.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - if the required vrId is not valid, or
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntryDel
(
    IN GT_U32                               vrId,
    IN GT_U8                                *destPtr,
    IN GT_U32                               prefix,
    IN GT_BOOL                              updateHwAndMem,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_BOOL                              roolBack
)
{
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT lpmPtr;/*the LPM structure to      */
                                                    /* delete from.             */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *delEntry = NULL;/* Next pointer struct.   */
    GT_PTR dummy;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;

    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC writeFuncData;

    GT_STATUS retVal = GT_OK;
    GT_STATUS retVal2 = GT_OK;

    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT rootBucketType; /* The bucket type of the 1st level LPM */
                                    /* bucket.                              */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmEngineMemPtr;

    /* first check that the protocol was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* can't delete the default route. only overwrite it */
    if(prefix == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check if the given virtual router already exists */
    if((vrId >= shadowPtr->vrfTblSize) ||
       (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)||
       (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* verify that unicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* update the info for the parent write function */
    parentWriteFuncInfo.bucketParentWriteFunc =
        prvCpssDxChLpmRamUpdateVrTableFuncWrapper;

    writeFuncData.shadowPtr = shadowPtr;
    writeFuncData.protocol = protocolStack;
    writeFuncData.vrId = vrId;
    parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

    retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    if (retVal != GT_OK)
        return (retVal);

    lpmPtr.nextBucket = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

    lpmEngineMemPtr = shadowPtr->ucSearchMemArrayPtr[protocolStack];

    dummy = (GT_PTR)delEntry;
    rootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack];
    retVal = prvCpssDxChLpmRamMngEntryDelete(&lpmPtr,destPtr,prefix,
                                             PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
                                             updateHwAndMem,lpmEngineMemPtr,
                                             &rootBucketType,&dummy,
                                             shadowPtr,&parentWriteFuncInfo,
                                             GT_FALSE,roolBack,protocolStack);
    delEntry = (PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *)dummy;

    if (retVal == GT_OK)
    {
        /* ok first free the next hop pointer */
        if (roolBack == GT_FALSE)
        {
            cpssOsLpmFree(delEntry);
        }

        /* update the root bucket type */
        shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack] =
            rootBucketType;

        if (updateHwAndMem == GT_TRUE)
        {
            /* Now update the uc first table according   */
            /* to the first level of the lpm structure.     */
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, protocolStack, shadowPtr);
        }
        else
        {
            /* indicate the VR HW wasn't updated and needs update */
            shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_TRUE;
        }
    }

    retVal2 = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                              &shadowPtr->freeMemListEndOfUpdate,shadowPtr);
    /* in case of bulk operation first need to collect all updates needed
       so do not reset the pendingBlockToUpdateArr and do not update the protocolCountersPerBlockArr
       this will be done in prvCpssDxChLpmRamIpv4UcPrefixBulkDel/prvCpssDxChLpmRamIpv6UcPrefixBulkDel

       in case of rollBack no need to update counters in the "delete" operation
       since the counters were not updated in the "add" operation.
       counters are updated at the end of operation in case of success */
    if ((updateHwAndMem==GT_TRUE)&&(roolBack==GT_FALSE))
    {
        if (retVal2 == GT_OK)
        {
            retVal2 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                protocolStack,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal2!=GT_OK)
            {
                return retVal2;

            }
        }
        else
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
        }
    }

    if(retVal == GT_OK)
        return retVal2;

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamUcEntriesFlush function
* @endinternal
*
* @brief   flushes the unicast Routing table and stays with the default entry
*         only.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] protocolStack            - the protocol to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntriesFlush
(
    IN GT_U32                               vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
)
{
    GT_STATUS retVal;
    GT_U8 addr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];   /* The next address & prefix to be */
    GT_U32 prefix = 0;                    /* deleted.                      */
    GT_U8 del[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];   /* addr in GT_U8[] format.   */
    GT_U32 i,bankIndex;

    /* first check that the protocol was initialized */
    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(addr, 0, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);

    i = 0;
    while(prvCpssDxChLpmRamUcEntryGet(vrId,addr,&prefix,NULL,
                                      protocolStack,shadowPtr) == GT_OK)
    {
        i++;

        cpssOsMemCpy(del, addr, sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);

        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E)
        {
            retVal = prvCpssDxChLpmRamUcEntryDel(vrId, del, prefix, GT_TRUE,
                                                 protocolStack,shadowPtr,
                                                GT_FALSE);
        }
        else
        {
            retVal = prvCpssDxChLpmRamSip6UcEntryDel(vrId, del, prefix, GT_TRUE,
                                                     protocolStack,shadowPtr,
                                                     GT_FALSE);
        }
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        cpssOsMemSet(addr, 0, sizeof(GT_U8)* PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS);

        prefix = 0;
    }

    /* release banks from its mapping */
    /* if the bank is marked as used we need to check if it is empty,
       if it is we need to unbind it from current "octet to bank mapping" and reuse it
       bank0 should never be released. For all the protocols octet 0 is mapped to bank 0.
       In Sip6 bank 1 is also should be never released. The GON is strted from bank 1 and
       octet 1 is mapped to bank 1 */
    if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        bankIndex = 2;
    }
    else
    {
        bankIndex = 1;
    }
    for (; bankIndex < shadowPtr->numOfLpmMemories; bankIndex++)
    {
        if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocolStack,bankIndex)==GT_TRUE)
        {
            retVal = prvCpssDxChLpmRamMngUnbindBlock(shadowPtr, bankIndex);
            if (retVal!=GT_OK)
            {
                return retVal;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamUcEntrySearch function
* @endinternal
*
* @brief   This function searches for a given uc address, and returns the next
*         hop pointer associated with it.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in] addrPtr                  - The address to search for.
* @param[in] prefix                   - Prefix length of the address.
* @param[in] protocolStack            - protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] nextHopPointerPtr        - The next hop pointer to which the given prefix is
*                                      bound.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_NOT_FOUND             - if the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntrySearch
(
    IN  GT_U32                                      vrId,
    IN  GT_U8                                       *addrPtr,
    IN  GT_U32                                      prefix,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopPointerPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocolStack,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr
)
{
    GT_STATUS retVal;
    GT_PTR dummy;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr;

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopEntry = NULL;

    if ((vrId >= shadowPtr->vrfTblSize) ||
        (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
        (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* verify that unicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

    if (nextHopPointerPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    dummy = (GT_PTR*)nextHopEntry;
    retVal = prvCpssDxChLpmRamMngSearch(bucketPtr,addrPtr,prefix,&dummy);
    nextHopEntry =(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *)dummy;
    if(retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    cpssOsMemCpy(nextHopPointerPtr,nextHopEntry,
                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamUcEntryLpmSearch function
* @endinternal
*
* @brief   This function searches for a given uc address, and returns the next
*         hop pointer associated with it. This is a LPM search - the exact prefix
*         doesn't have to exist in DB.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in] addrPtr                  - The address to search for.
* @param[in] protocolStack            - type of protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopPointerPtr        - The next hop pointer to which the given prefix is
*                                      bound.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_NOT_FOUND             - if the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntryLpmSearch
(
    IN  GT_U32                                      vrId,
    IN  GT_U8                                       *addrPtr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopPointerPtr,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocolStack,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr
)
{
    GT_STATUS retVal;
    GT_PTR dummy;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr;

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopEntry = NULL;

    if ((vrId >= shadowPtr->vrfTblSize) ||
        (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
        (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* verify that unicast is supported for this protocol in the VR */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

    if (nextHopPointerPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    dummy = (GT_PTR)nextHopEntry;
    retVal = prvCpssDxChLpmRamMngDoLpmSearch(bucketPtr,
                                             addrPtr,
                                             protocolStack,
                                             prefixLenPtr,
                                             &dummy);
    nextHopEntry = (PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *)dummy;
    if(retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    cpssOsMemCpy(nextHopPointerPtr,nextHopEntry,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamUcEntryGet function
* @endinternal
*
* @brief   This function returns an Unicast entry with larger (address,prefix) than
*         the given one.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] addrPtr                  - The address to start the search from.
* @param[in,out] prefixPtr                - Prefix length of addrPtr.
* @param[in] protocolStack            - type of protocol stack to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in,out] addrPtr                  - The address of the found entry.
* @param[in,out] prefixPtr                - The prefix length of the found entry.
*
* @param[out] nextHopPointerPtrPtr     - A pointer to the next hop pointer
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_NOT_FOUND             - if no more entries are left in the LPM shadow
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUcEntryGet
(
    IN    GT_U32                                    vrId,
    INOUT GT_U8                                     *addrPtr,
    INOUT GT_U32                                    *prefixPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC **nextHopPointerPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocolStack,
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr
)
{
    GT_STATUS retVal;
    GT_PTR dummy;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *tmpNextHopPointerPtr=NULL;

    if (shadowPtr->isProtocolInitialized[protocolStack] == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

    dummy = (GT_PTR)tmpNextHopPointerPtr;
    retVal = prvCpssDxChLpmRamMngEntryGet(bucketPtr,protocolStack,addrPtr,prefixPtr,(GT_PTR*)&dummy);
    if (retVal != GT_OK)
    {
        return /* do not log this error */ GT_NOT_FOUND;
    }
    /* Since UC and MC share the same tree, check that the entry is not MC */
    if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) &&
        (*addrPtr >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS) &&
        (*addrPtr <= PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS))
    {
        /* Skip the mulicast range - jump to the reserved range */
        *addrPtr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
        *prefixPtr = PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS;
        return prvCpssDxChLpmRamUcEntryGet(vrId, addrPtr, prefixPtr,
                                           nextHopPointerPtrPtr, protocolStack,
                                           shadowPtr);
    }
    if ((protocolStack == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) &&
        (*addrPtr >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS))
    {
        return /* do not log this error */ GT_NOT_FOUND;
    }

    tmpNextHopPointerPtr = (PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *)dummy;
    if (nextHopPointerPtrPtr != NULL)
    {
        *nextHopPointerPtrPtr = tmpNextHopPointerPtr;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIpLpmRamUcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific UC prefix
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] protocol                 - type of  stack to work on.
* @param[in] ipPtr                    - Pointer to the ip address to look for.
* @param[in] prefixLen                - ipAddr prefix length.
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
GT_STATUS prvCpssDxChIpLpmRamUcPrefixActivityStatusGet
(
    IN GT_U32                               vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocol,
    IN GT_U8                                *ipPtr,
    IN GT_U32                               prefixLen,
    IN GT_BOOL                              clearActivity,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    GT_STATUS  rc;
    GT_U32 entryIndex;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *rootBucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr;
    CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT rangeSelectSec;
    GT_U32 numOfRanges;
    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC nextPointerArray[256];
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *startRange;
    GT_U8  lastIpPtr;
    GT_U8   startAddr;
    GT_U8   endAddr;

    CPSS_NULL_PTR_CHECK_MAC(activityStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(ipPtr);

    rootBucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    /* Look for offset/index */
    rc = prvCpssDxChLpmRamMngSearchOffset(rootBucketPtr, ipPtr, prefixLen, &entryIndex, &bucketPtr);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    /* Read node from the hw by index. It's enough to read only first device  */
    rc = cpssDxChLpmNodeRead(shareDevsList[0], entryIndex, bucketPtr->bucketType, &rangeSelectSec, &numOfRanges, (CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC *)&nextPointerArray);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    /* From printLpm debugging:
     * 1.1.1.2 / 32
     * rangeStart[0] = 0 - always 0
     * rangeStart[1] = 0x2 rangeStart[2] = 0x3 rangeStart[3] =  0 rangeStart[4] =  0
     * rangeStart[3] = 0 - end of ranges if 0
     * So:
     * 0..1     range[i+1] - 1 == 2-1 == 1
     * 2..2     range[i+1] - 1 == 3-1 == 2
     * 3..255   range[i+1] - 1 == 0-1 == 255
     */

    /* Last address byte index depends on protocol */
    if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        lastIpPtr = 3;
    }
    else
    if(protocol == CPSS_IP_PROTOCOL_IPV6_E)
    {
        lastIpPtr = 15;
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* Look for appropriate range in the range list.
     * Indexes in the range list and in the nextPointerArray[] are same.
     */
    startRange = bucketPtr->rangeList;
    for(i = 0; i< numOfRanges; i++){
        startAddr = startRange->startAddr;
        endAddr = 0;
        if(startRange->next)
            endAddr = startRange->next->startAddr;
        /* check last address byte within range */
        if(startAddr <= ipPtr[lastIpPtr] && ipPtr[lastIpPtr]<= (GT_U8)(endAddr-1)) break;
        /* next range */
        startRange = startRange->next;
    }
    /* That's why we are here */
    *activityStatusPtr = nextPointerArray[i].pointerData.nextHopOrEcmpPointer.activityState;

    /* if status set and we need to clear it */
    if(*activityStatusPtr && clearActivity)
    {
        nextPointerArray[i].pointerData.nextHopOrEcmpPointer.activityState = GT_FALSE;
        /* Write to all devices in the dev list */
        for(i = 0; i < shareDevListLen; i++)
        {
            /* write node to the hw */
            rc = cpssDxChLpmNodeWrite(shareDevsList[i], entryIndex, bucketPtr->bucketType, &rangeSelectSec, numOfRanges, (CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC *)&nextPointerArray);
        }
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


