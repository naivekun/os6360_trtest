/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChLpmMng.c
*
* DESCRIPTION:
*       Implementation of the LPM algorithm, for the use of UC and MC engines.
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 24 $
*
*******************************************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMng.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpssCommon/private/prvCpssMath.h>

extern void * cpssOsLpmMalloc
(
    IN GT_U32 size
);

extern void cpssOsLpmFree
(
    IN void* const memblock
);

GT_UINTPTR   dmmBlockNotFoundIndication = (GT_UINTPTR)(-1);

/***************************************************************************
* local defines
****************************************************************************/

/* This macro checks if the given range is the last range   */
/* in this LPM level.                                       */
#define LAST_RANGE_MAC(range)            (range->next == NULL)

/* This macro converts a prefix length to a mask            */
/* representation.                                          */
#define PREFIX_2_MASK_MAC(prefixLen,maxPrefix) \
        (((prefixLen > maxPrefix)|| (prefixLen == 0))? 0 : (1 << (prefixLen-1)))

/* The following macro returns the maximal prefix in the    */
/* next level bucket.                                       */
#define NEXT_BUCKET_PREFIX_MAC(prefix,levelPrefix)  \
        ((prefix <= levelPrefix) ? 0 : (prefix - levelPrefix))

/* The following macro returns the maximal prefix in the    */
/* given level bucket.                                       */
#define BUCKET_PREFIX_AT_LEVEL_MAC(prefix,levelPrefix,level)  \
        ((prefix <= (level * levelPrefix)) ? \
        0 : (prefix - (level * levelPrefix)))

/**
* @enum LPM_BUCKET_UPDATE_MODE_ENT
 *
 * @brief Indicates what kind of update the bucket will go.
*/
typedef enum{

    /** updates nothing. */
    LPM_BUCKET_UPDATE_NONE_E = 0,

    /** @brief updates the HW and mem alloc new
     *  memory if needed.
     */
    LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E,

    /** @brief updates the HW and resize
     *  bucket's memory if needed
     *  (bulk delete operation).
     */
    LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E,

    /** updates mem alloc only. */
    LPM_BUCKET_UPDATE_MEM_ONLY_E,

    /** @brief updates the bucket tree shadow only
     *  without memory alloc or HW.
     */
    LPM_BUCKET_UPDATE_SHADOW_ONLY_E,

    /** @brief this is an update for
     *  root bucket , it's a shodow update
     *  only, no touching of the HW.
     */
    LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E

} LPM_BUCKET_UPDATE_MODE_ENT;

/**
* @internal mask2PrefixLength function
* @endinternal
*
* @brief   This function returns the index of the most significant set bit (1-8),
*         in the given mask.
* @param[in] mask                     - The prefix  to operate on.
* @param[in] levelMaxPrefix           - The maximum prefix that can be hold by this mask.
*
* @retval 1                        - levelMaxPrefix if (mask != 0), 0 otherwise.
*/
static GT_U32 mask2PrefixLength
(
    IN GT_U32 mask,
    IN GT_U8 levelMaxPrefix
)
{
    GT_U8 i;

    for(i = 0; i < levelMaxPrefix; i++)
    {
        if((mask & (1 << (levelMaxPrefix - 1 - i))) != 0)
            return (levelMaxPrefix - i);
    }
    return 0;
}

/**
* @internal calcStartEndAddr function
* @endinternal
*
* @brief   This function calcules the start & end address of a prefix.
*
* @param[in] addr                     - the address octet.
* @param[in] prefixLen                - the address prefix length
*
* @param[out] startAddrPtr             - the calculated start address
* @param[out] endAddrPtr               - the calculated end address
*/
static GT_STATUS calcStartEndAddr
(
    IN  GT_U8  addr,
    IN  GT_U32 prefixLen,
    OUT GT_U8  *startAddrPtr,
    OUT GT_U8  *endAddrPtr
)
{
    GT_U8  prefixComp;          /* temp var. for calculating startAddr  */
                                /* and endAddr.                         */

    prefixComp = (GT_U8)(((prefixLen > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) ?
                  0 : (PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS - prefixLen)));

    *startAddrPtr = (GT_U8)(addr &
                 (BIT_MASK_MAC(PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) << prefixComp));

    if (endAddrPtr != NULL)
        *endAddrPtr = (GT_U8)(*startAddrPtr | ~(0xFF << prefixComp));

    return GT_OK;
}

/*******************************************************************************
* find1stOverlap
*
* DESCRIPTION:
*       This function traverses the ranges linked list from the low address and
*       stopping at the first overlapping range with the prefix.
*
* INPUTS:
*       bucketPtr   - pointer to bucket, in CPU's memory.
*       startAddr   - prefix start address, lower address covered by the prefix.
*       pPrevPtr    - A pointer to the node before the ovelapping node.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       A pointer to the first overlapping range.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *find1stOverlap
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     startAddr,
    OUT PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **pPrevPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *rangePtr;/* Current range pointer.   */

    if(pPrevPtr != NULL)
        *pPrevPtr = NULL;

    /* if the address we need is after the cashed range, start with the cashed
       range,
       or if the address we need is the same as the cashed ranged, use it only
       if we don't need to provide a prev range */
    if ((bucketPtr->rangeCash != NULL) &&
        ((startAddr > bucketPtr->rangeCash->startAddr) ||
         ((startAddr == bucketPtr->rangeCash->startAddr) &&
          (pPrevPtr == NULL))))
    {
        rangePtr = bucketPtr->rangeCash;
    }
    else
    {
        rangePtr = bucketPtr->rangeList;
    }

    while((rangePtr->next != NULL) &&
          (startAddr >= rangePtr->next->startAddr))
    {
        bucketPtr->rangeCash = rangePtr;/* the range cash always saves the prev*/
        if(pPrevPtr != NULL)
            *pPrevPtr = rangePtr;

        rangePtr = rangePtr->next;
    }

    return rangePtr;
}

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
*                                       GT_OK on success, GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChLpmRamMngRangeInNextPointerFormatSet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *rangePtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC        *lpmEngineMemPtr,
    OUT CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC       *nextPointerArrayPtr
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *lpmBucketPtr;

    if (rangePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if ((rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
        (rangePtr->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
        (rangePtr->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
    {
        nextHopPtr = rangePtr->lowerLpmPtr.nextHopEntry;
        if (nextHopPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        nextPointerArrayPtr->pointerType = rangePtr->pointerType;
        nextPointerArrayPtr->pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable =
            nextHopPtr->ucRpfCheckEnable;
        nextPointerArrayPtr->pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable =
            nextHopPtr->srcAddrCheckMismatchEnable;
        nextPointerArrayPtr->pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel =
            nextHopPtr->ipv6McGroupScopeLevel;
        nextPointerArrayPtr->pointerData.nextHopOrEcmpPointer.entryIndex =
            nextHopPtr->routeEntryBaseMemAddr;
    }
    else
    {
        lpmBucketPtr = rangePtr->lowerLpmPtr.nextBucket;
        if (lpmBucketPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        /* if the hwBucketOffsetHandle is 0 this is a new bucket that represents
           only a next hop -> thus take the next hop of the first range */
        if (lpmBucketPtr->hwBucketOffsetHandle == 0)
        {
            retVal =
                prvCpssDxChLpmRamMngRangeInNextPointerFormatSet(lpmBucketPtr->rangeList,
                                                                lpmEngineMemPtr,
                                                                nextPointerArrayPtr);
        }
        else
        {
            nextPointerArrayPtr->pointerType = lpmBucketPtr->bucketType;
            nextPointerArrayPtr->pointerData.nextNodePointer.nextPointer =
                PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(lpmBucketPtr->hwBucketOffsetHandle) +
                lpmEngineMemPtr->structsBase;
            nextPointerArrayPtr->pointerData.nextNodePointer.range5Index =
                lpmBucketPtr->fifthAddress;
            if ((GT_U32)rangePtr->pointerType == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
            {
                /* Set the pointToSipTree bit in case that the next bucket is
                   a root bucket (pointing to MC source tree) */
                nextPointerArrayPtr->pointerData.nextNodePointer.pointToSipTree = GT_TRUE;
            }
            else
            {
                nextPointerArrayPtr->pointerData.nextNodePointer.pointToSipTree = GT_FALSE;
            }
        }
    }
    return retVal;
}

/**
* @internal insert2Trie function
* @endinternal
*
* @brief   Inserts a next hop entry to the trie structure.
*
* @param[in] bucketPtr                - Bucket to which the given entry is associated.
* @param[in] startAddr                - Start Addr of the address associated with the given
*                                      entry.
* @param[in] prefix                   - The address prefix.
* @param[in] trieDepth                - The maximum depth of the trie.
* @param[in] nextPtr                  - A pointer to the next hop/next_lpm_trie entry to be
*                                      inserted to the trie.
*                                       GT_OK on success, GT_FAIL otherwise.
*/
static GT_STATUS insert2Trie
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN GT_U8                                    startAddr,
    IN GT_U32                                   prefix,
    IN GT_U8                                    trieDepth,
    IN GT_PTR                                   nextPtr
)
{
    GT_U8 addr[1];               /* prefix in GT_U8 representation   */

    addr[0] = (GT_U8)(startAddr & (BIT_MASK_MAC(trieDepth) << (trieDepth - prefix)));

    return prvCpssDxChLpmTrieInsert(&(bucketPtr->trieRoot),
                                    addr,prefix,trieDepth,nextPtr);
}

/*******************************************************************************
* createNewBucket
*
* DESCRIPTION:
*       This function creates a new bucket with a given default next hop route
*       entry.
*
* INPUTS:
*       nextHopPtr - The default route next hop data.
*       trieDepth   - The maximum depth of the trie.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       A pointer to the new created bucket if succeeded, NULL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC * createNewBucket
(
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopPtr,
    IN GT_U8 trieDepth
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *pBucket;     /* The bucket to be created.    */
    GT_STATUS                                rc;

    if((pBucket = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC)))
       == NULL)
        return NULL;

    if((pBucket->rangeList = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
       == NULL)
    {
        cpssOsLpmFree(pBucket);
        pBucket = NULL;

        return NULL;
    }

    /* Initialize the bucket's fields.          */

    pBucket->rangeList->pointerType              = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    pBucket->rangeList->lowerLpmPtr.nextHopEntry = nextHopPtr;

    pBucket->rangeList->startAddr   = 0;
    pBucket->rangeList->mask        = 0;
    pBucket->rangeList->next        = NULL;
    pBucket->bucketType             = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    pBucket->fifthAddress           = 0;
    pBucket->hwBucketOffsetHandle   = 0;
    cpssOsMemSet(pBucket->hwGroupOffsetHandle,0,sizeof(pBucket->hwGroupOffsetHandle));

    pBucket->pointingRangeMemAddr   = 0;

    pBucket->rangeCash = NULL;

    pBucket->numOfRanges = 1;
    pBucket->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;


    /* Insert the default prefix into the       */
    /* Trie of the newly create LPM bucket.     */
    pBucket->trieRoot.pData = NULL;
    rc = insert2Trie(pBucket,0,0,trieDepth,nextHopPtr);
    if (rc != GT_OK)
    {
        /* trie was not inserted successfully*/
        cpssOsLpmFree(pBucket->rangeList);
        cpssOsLpmFree(pBucket);
        return NULL;
    }

    pBucket->trieRoot.father = pBucket->trieRoot.leftSon = NULL;
    pBucket->trieRoot.rightSon = NULL;

    return pBucket;
}

/**
* @internal splitRange function
* @endinternal
*
* @brief   This function splits a range. According to one of the following possible
*         splits:
*         PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E, PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E,
*         PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E and PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E
* @param[in,out] rangePtrPtr              - Pointer to the range to be splitted.
* @param[in] startAddr                - The low address of the new range.
* @param[in] endAddr                  - The high address of the new range.
* @param[in] prefixLength             - The length of the dominant prefix of the new range.
* @param[in] rangeInfoPtr             - the range information to use for the new range
*                                      (or overwrite an old one with this new info).
* @param[in] levelPrefix              - The current lpm level prefix.
* @param[in] updateOldPtr             - (GT_TRUE) this is an update for an already existing entry.
* @param[in,out] rangePtrPtr              - A pointer to the next range to be checked for split.
*
* @param[out] numOfNewRangesPtr        - Number of new created ranges as a result of the split.
* @param[out] pointerTypePtrPtr        - the added/replaced range's pointerType field pointer
*                                       GT_OK on success, or
*                                       GT_OUT_OF_CPU_MEM on lack of cpu memory.
*/
static GT_STATUS splitRange
(
    INOUT PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **rangePtrPtr,
    IN GT_U8                                        startAddr,
    IN GT_U8                                        endAddr,
    IN GT_U32                                       prefixLength,
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       *rangeInfoPtr,
    IN GT_U8                                        levelPrefix,
    IN GT_BOOL                                      *updateOldPtr,
    OUT GT_U32                                      *numOfNewRangesPtr,
    OUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT         **pointerTypePtrPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *newRangePtr;  /* Points to the new create */
                            /* range, if such creation is needed.                      */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *curRangePtr = (*rangePtrPtr);    /* Points to rangePtrPtr, for  */
                                            /* easy access.             */
    GT_U8 mask;                /* The mask represented by prefixLength */
                                /* and levelPrefix.                     */
    GT_U8 endRange;            /* Index by which this range ends.      */
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT splitMethod;     /* The method by which to split the     */
                                                            /* rangePtrPtr, if needed.              */
    GT_BOOL splittedRangeNeedsHwUpdate = GT_FALSE;


    mask = (GT_U8)PREFIX_2_MASK_MAC(prefixLength,levelPrefix);

    endRange = (GT_U8)(LAST_RANGE_MAC(curRangePtr) ? ((1 << levelPrefix) - 1) :
        ((curRangePtr->next->startAddr) - 1));

    splitMethod = (((startAddr > curRangePtr->startAddr) ? 1 : 0) +
                   ((endAddr < endRange) ? 2 : 0));

    if((*rangePtrPtr)->updateRangeInHw == GT_TRUE)
    {
        /* the range is going to be splitted needs update in hw     */
        /* so all ranges created after splitting should be marked as*/
        /* as needed update in hw                                   */
        /* such situation could be occured during bulk operation    */
        splittedRangeNeedsHwUpdate = GT_TRUE;
    }

    switch (splitMethod)
    {
    case PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E:
        newRangePtr = curRangePtr;
        newRangePtr->mask |= mask;
        if(*updateOldPtr == GT_TRUE)       /*   Check This  */
        {
            /*osStatFree(newRangePtr->nextPointer.nextPtr.nextHopEntry);*/
            *updateOldPtr = GT_FALSE;
        }

        newRangePtr->pointerType = rangeInfoPtr->pointerType;
        newRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        newRangePtr->updateRangeInHw = GT_TRUE;

        *numOfNewRangesPtr = 0;

        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(newRangePtr->pointerType);
        break;

    case PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E:     /* A new node should be added before curRangePtr    */
        if((newRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
           == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

        /* Insert newRangePtr after curRangePtr     */
        newRangePtr->next = curRangePtr->next;
        curRangePtr->next = newRangePtr;

        /* newRangePtr represents the old Range.    */
        newRangePtr->startAddr  = (GT_U8)(endAddr + 1);
        newRangePtr->mask       = curRangePtr->mask;

        newRangePtr->pointerType = curRangePtr->pointerType;
        newRangePtr->lowerLpmPtr = curRangePtr->lowerLpmPtr;

        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            newRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* no new pointer in the old range */
            newRangePtr->updateRangeInHw = GT_FALSE;
        }


        /* curRangePtr represents the new Range.    */
        curRangePtr->mask       |= mask;
        curRangePtr->pointerType = rangeInfoPtr->pointerType;
        curRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        /* new pointer so we need to update in the hardware */
        curRangePtr->updateRangeInHw = GT_TRUE;

        *rangePtrPtr       = newRangePtr;
        *numOfNewRangesPtr = 1;

        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(curRangePtr->pointerType);
        break;

    case PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E:    /* A new node should be added after curRangePtr    */
        if((newRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
           == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

        /* Insert newRangePtr after curRangePtr    */
        newRangePtr->next = curRangePtr->next;
        curRangePtr->next = newRangePtr;

        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            curRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* no new pointer in the old range */
            curRangePtr->updateRangeInHw = GT_FALSE;
        }

        /* Update the fields of newRangePtr     */
        newRangePtr->startAddr  = startAddr;
        newRangePtr->mask       = (GT_U8)(curRangePtr->mask | mask);
        newRangePtr->pointerType = rangeInfoPtr->pointerType;
        newRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        /* new pointer so we need to update in the hardware */
        newRangePtr->updateRangeInHw = GT_TRUE;

        *rangePtrPtr       = newRangePtr;
        *numOfNewRangesPtr = 1;

        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(newRangePtr->pointerType);
        break;

    case PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E:   /* Two new node should be added, one for the    */
                                                    /* second part of curRangePtr, and one for the  */
                                                    /* range created by the new inserted prefix.    */
        if((newRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
           == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);


        if((newRangePtr->next = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC)))
           == NULL)
        {
            cpssOsLpmFree(newRangePtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

        }


        /* Insert the two new ranges after curRangePtr     */
        newRangePtr->next->next = curRangePtr->next;
        curRangePtr->next = newRangePtr;
        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            curRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* no new pointer in the old range */
            curRangePtr->updateRangeInHw = GT_FALSE;
        }


        /* Update the fields of the first added range   */
        newRangePtr->startAddr  = startAddr;
        newRangePtr->mask       = (GT_U8)(curRangePtr->mask | mask);

        newRangePtr->pointerType = rangeInfoPtr->pointerType;
        newRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        /* new pointer so we need to update in the hardware */
        newRangePtr->updateRangeInHw = GT_TRUE;

        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(newRangePtr->pointerType);

        /* Update the fields of the second added range  */
        newRangePtr             = newRangePtr->next;
        newRangePtr->startAddr  = (GT_U8)(endAddr + 1);
        newRangePtr->mask       = curRangePtr->mask;

        newRangePtr->pointerType = curRangePtr->pointerType;
        newRangePtr->lowerLpmPtr = curRangePtr->lowerLpmPtr;

        if (splittedRangeNeedsHwUpdate == GT_TRUE)
        {
            newRangePtr->updateRangeInHw = GT_TRUE;
        }
        else
        {
            /* the new pointer is the not new just copied, no need to update in Hw*/
            newRangePtr->updateRangeInHw = GT_FALSE;
        }

        *rangePtrPtr = newRangePtr;
        *numOfNewRangesPtr = 2;

        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal updateMirrorRange function
* @endinternal
*
* @brief   This function updates just one range in a bucket of the lpm Trie.
*         this function is used only in the case the range points to another
*         bucket.
* @param[in] bucketPtr                - Pointer to the bucket pointed by the range
* @param[in] lpmEngineMemPtrPtr       - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for
*                                      where and how to allocate search memory.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*                                       GT_OK on success,
*
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS updateMirrorRange
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC    *bucketPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC         **lpmEngineMemPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC           *shadowPtr
)
{
    GT_U32 i;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    GT_STATUS retVal = GT_OK;

    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC nextPointer;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   range;

    if (bucketPtr->pointingRangeMemAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    range.lowerLpmPtr.nextBucket = bucketPtr;
    range.pointerType = bucketPtr->bucketType;

    retVal = prvCpssDxChLpmRamMngRangeInNextPointerFormatSet(&range,
                            lpmEngineMemPtrPtr[0], &nextPointer);

    if (GT_OK != retVal )
    {
        return retVal;
    }

    for (i = 0; i < shareDevListLen; i++)
    {
        retVal =
            cpssDxChLpmNodeNextPointersWrite(shareDevsList[i],
                                             bucketPtr->pointingRangeMemAddr,
                                             1, &nextPointer);
        if (retVal != GT_OK)
        {
            break;
        }
    }

    return (retVal);
}

/**
* @internal getMirrorBucketInfo function
* @endinternal
*
* @brief   This function gets a mirrored bucket info
*
* @param[in] bucketPtr                - Pointer to the bucket.
* @param[in] lpmEngineMemPtrPtr       - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for
*                                      where and how to allocate search memory.
* @param[in] newBucketType            - the bucket's new type
*
* @param[out] memSizePtr               - the memory size this bucket needs (in LPM lines)
* @param[out] writeOffsetPtr           - the write offset in the bucket where the ranges start
* @param[out] fifthStartAddrPtr        - the calculated fifth range address.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS getMirrorBucketInfo
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC        **lpmEngineMemPtrPtr,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       newBucketType,
    OUT GT_U32                                    *memSizePtr,
    OUT GT_U32                                    *writeOffsetPtr,
    OUT GT_U8                                     *fifthStartAddrPtr
)
{
    /* warnings fix */
    lpmEngineMemPtrPtr = lpmEngineMemPtrPtr;

    switch (newBucketType)
    {
    case (CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E):
        *memSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS + bucketPtr->numOfRanges;
        /* One word for the bit-vector + place for next pointers.   */
        *writeOffsetPtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
        break;

    case (CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E):
        *memSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS + bucketPtr->numOfRanges;
        /* 2 words for the bit-vector + place for next pointers.   */
        *writeOffsetPtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;

        /* this is the only place where the fifth address is valid - so retrive it:
                             range idx <0>      <1>   <2>   <3>   <4>   <5> */
        *fifthStartAddrPtr = bucketPtr->rangeList->next->next->next->next->next->startAddr;
        break;

    case (CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E):
        *memSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS + bucketPtr->numOfRanges;
        /* 11 words for the bit-vector + place for next pointers.   */
        *writeOffsetPtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return (GT_OK);
}

/**
* @internal getMirrorBucketDataAndUpdateRangesAddress function
* @endinternal
*
* @brief   This function gets a the bucket's shadow data and formats accordingly
*         the bitvetor and the nexy pointer array.
*         it also updates the ranges hw address for the pointed buckets.
* @param[in] rangePtr                 - Pointer to the bucket's first range.
* @param[in] lpmEngineMemPtrPtr       - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed
*                                      for where and how to allocate search memory.
* @param[in] newBucketType            - the bucket's new type
* @param[in] bucketBaseAddress        - the bucket's base address
*
* @param[out] rangeSelectSecPtr        - range selection section (bitmap)
* @param[out] nextPointerArrayPtr      - the next pointer array.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS getMirrorBucketDataAndUpdateRangesAddress
(
    IN  PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *rangePtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC              **lpmEngineMemPtrPtr,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             newBucketType,
    IN  GT_U32                                          bucketBaseAddress,
    OUT CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT          *rangeSelectSecPtr,
    OUT CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC             *nextPointerArrayPtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U32 bitCount;        /* Will hold the number of sets bits when calcu-*/
                            /* lating the bit vector for regular-buckets.   */
    GT_U32 line;            /* Holds the bit vector line number, when prepa-*/
                            /* ring the bit vector for regular-buckets.     */
    GT_U32 j;
    GT_U32 maxNumOfRanges;

    GT_U8 *rangeStartAddrArray1_4,*rangeStartAddrArray6_9;
    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC *nextPointerPtr;

    CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC *bitVecEntryPtr;
    GT_U32 bitmapSize = 0;
    nextPointerPtr = nextPointerArrayPtr;
    rangeStartAddrArray6_9 = NULL;

    switch (newBucketType)
    {
    case (CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E):
    case (CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E):

        if (newBucketType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
        {
            rangeStartAddrArray1_4 = rangeSelectSecPtr->compressed1BitVector.ranges1_4;
            maxNumOfRanges = MAX_NUMBER_OF_COMPRESSED_1_RANGES_CNS;
        }
        else
        {
            rangeStartAddrArray1_4 = rangeSelectSecPtr->compressed2BitVector.ranges1_4;
            rangeStartAddrArray6_9 = rangeSelectSecPtr->compressed2BitVector.ranges6_9;
            maxNumOfRanges = MAX_NUMBER_OF_COMPRESSED_2_RANGES_CNS;
        }

        /* go over the 5 possible ranges and format them for hw writing */
        for (j = 0; j < maxNumOfRanges; j++)
        {
            if (rangePtr == NULL)
            {
                continue;
            }
            /* we don't need it now so remove the flag*/
            rangePtr->updateRangeInHw = GT_FALSE;

            /* update the range's bucket parent range memory address */
            if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                (rangePtr->pointerType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
                (rangePtr->pointerType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
            {
                if (newBucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                    bitmapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
                else if (newBucketType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
                    bitmapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
                else /* (newBucketType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E) */
                    bitmapSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;

                rangePtr->lowerLpmPtr.nextBucket->pointingRangeMemAddr =
                    bucketBaseAddress + (j + bitmapSize);
            }

            /* we overlook range 0 start address since it's always 0 and there
               is no place for it in the HW */
            if ((j > 0) && (j < 5))
            {
                rangeStartAddrArray1_4[j-1] = rangePtr->startAddr;
            }
            /* here we overlook range 5 start address since it's recorded in the
               pointing pointer */
            else if (j > 5)
            {
                if (rangeStartAddrArray6_9 == NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

                rangeStartAddrArray6_9[j-6] = rangePtr->startAddr;
            }

            /* format the range */
            retVal = prvCpssDxChLpmRamMngRangeInNextPointerFormatSet(rangePtr,
                                                                     lpmEngineMemPtrPtr[1],
                                                                     nextPointerPtr);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            /* advance the pointers */
            nextPointerPtr++;
            rangePtr = rangePtr->next;
        }
        break;
    case (CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E):

        bitVecEntryPtr = rangeSelectSecPtr->regularBitVector.bitVectorEntry;

        line = 1;
        j = 0;
        bitCount = 0;
        bitVecEntryPtr[0].rangeCounter = 10;/* First bitvectore counter contains
                                               the offset to the first next hop
                                               pointer.             */
        while (rangePtr != NULL)
        {
            /* we don't need it now so remove the flag*/
            rangePtr->updateRangeInHw = GT_FALSE;

            /* update the range's bucket parent range memory address */
            if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                (rangePtr->pointerType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
                (rangePtr->pointerType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
            {
                rangePtr->lowerLpmPtr.nextBucket->pointingRangeMemAddr =
                    bucketBaseAddress +
                    (j + PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS);
            }

            while (rangePtr->startAddr >= 24 * line)
            {
                bitVecEntryPtr[line].rangeCounter =
                    (GT_U8)(bitVecEntryPtr[line-1].rangeCounter + bitCount - 1);
                bitCount = 0;
                line++;
            }

            retVal = prvCpssDxChLpmRamMngRangeInNextPointerFormatSet(rangePtr,
                                                                     lpmEngineMemPtrPtr[1],
                                                                     nextPointerPtr);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            bitVecEntryPtr[line - 1].bitMap |= (1 << (rangePtr->startAddr % 24));
            j++;
            bitCount++;
            nextPointerPtr++;
            rangePtr = rangePtr->next;
        }

        while (line < PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS)
        {
            bitVecEntryPtr[line].rangeCounter =
                (GT_U8)(bitVecEntryPtr[line-1].rangeCounter + bitCount - 1);
            bitCount = 0;
            line++;
        }

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return (GT_OK);
}

/**
* @internal updateMirrorBucket function
* @endinternal
*
* @brief   This function creates a mirrored bucket in CPU's memory, and updates
*         the lpm structures in PP's memory.
* @param[in] bucketPtr                - Pointer to the bucket to create a mirror from.
* @param[in] bucketUpdateMode         - Indicates whether to write the table to the device's
*                                      Ram, update the memory alloc , or do nothing.
* @param[in] isDestTreeRootBucket     - Indicates whether the bucket is the root of the
*                                      destination address tree
* @param[in] resizeBucket             - Indicates whether the bucket was resized or not
*                                      during the insertion / deletion process.
* @param[in] forceWriteWholeBucket    - force writing of the whole bucket and not just
*                                      the ranges where the writeRangeInHw is set.
* @param[in] indicateSiblingUpdate    - whether to update siblings of buckets (used in
*                                      bulk mode)
* @param[in] useCompressed2           - whether to use compressed 2 type.
* @param[in] lpmEngineMemPtrPtr       - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed
*                                      for where and how to allocate search memory.
* @param[in,out] bucketTypePtr            - The bucket type of the mirrored bucket.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] parentWriteFuncPtr       - the bucket's parent write function in case there is
*                                      a need to update the packet's parent ptr data and
*                                      the parent is not a LPM trie range.
*                                      (relevant only in delete operations)
* @param[in,out] bucketTypePtr            - The bucket type of the mirrored bucket.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
*/
static GT_STATUS updateMirrorBucket
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN LPM_BUCKET_UPDATE_MODE_ENT               bucketUpdateMode,
    IN GT_BOOL                                  isDestTreeRootBucket,
    IN GT_BOOL                                  resizeBucket,
    IN GT_BOOL                                  forceWriteWholeBucket,
    IN GT_BOOL                                  indicateSiblingUpdate,
    IN GT_BOOL                                  useCompressed2,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC       **lpmEngineMemPtrPtr,
    INOUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT   *bucketTypePtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC *parentWriteFuncPtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U32 i;
    GT_U32 memSize = 0;/* Size of block to be written to RAM.          */

    GT_UINTPTR oldMemPool = 0;  /* The memory pool from which the old buckets   */
                            /* where allocated.                             */

    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;

    GT_UINTPTR tempHwAddrHandle;/* Will hold the allocated area in PP's RAM to  */
                                /* which the buckets will be written.           */
    GT_U32 memBlockBase = 0;/* the base address (offset) in the RAM for bucket*/

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    GT_UINTPTR                           tmpStructsMemPool;/* use for going over the list of blocks per octet */

    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */
    GT_BOOL     swapMemInUseForAdd=GT_FALSE; /* indicate that the memory allocated for ADD operation was taken from the swap memory */

    GT_BOOL     needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR  tempHwAddrHandleToBeFreed = 0;

    /* the fields for writing the bucket to the HW */
    static CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT rangeSelectionSec;
    static CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC nextPointersArray[256];

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;
    GT_U32 tempAddr = 0;     /* Temporary address                */
    GT_U8 fifthStartAddr = 0;/* The fifth start address, in case of LPM  */
                             /* Bucket type-2 compressed.                */

    GT_U32 writeOffset = 0; /* Holds the offset from the bucket pointer from*/
                            /* which we need to start writing in case there */
                            /* is no need to resize                         */
    GT_U32 inBucketWriteOffset;
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC     **freeMemListPtr;

    GT_BOOL swapMemInUse = GT_FALSE;
    GT_BOOL freeOldMem = GT_TRUE;

    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT newBucBucketType;

    GT_UINTPTR oldHwAddrHandle;

    indicateSiblingUpdate = indicateSiblingUpdate; /* prevent warning */

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    /* Determine the bucket type of the new bucket. */
    if(bucketPtr->numOfRanges == 1)
    {
        if (isDestTreeRootBucket)
        {
            newBucBucketType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
        }
        else
        {
            newBucBucketType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        }
    }
    else if(bucketPtr->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_1_RANGES_CNS)    /* 1-line compressed bucket */
    {
        newBucBucketType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
    }
    else if((bucketPtr->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_2_RANGES_CNS) &&
            (useCompressed2 == GT_TRUE))   /* 2-line compressed bucket */
    {
        newBucBucketType = CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E;
    }
    else                                    /* Regular bucket           */
    {
        newBucBucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
    }

    /* check the case of no hw update */
    if((bucketUpdateMode == LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E)  ||
       (bucketUpdateMode == LPM_BUCKET_UPDATE_NONE_E))
    {
        /* all buckets that are visited during the shadow update stage of
           the bulk operation should be scanned during the hardware update stage;
           therefore don't leave buckets with PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E
           mark */
        if (/* (indicateSiblingUpdate == GT_TRUE) && */
            (bucketPtr->bucketHwUpdateStat == PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E))
        {
            /*indicate we need to go through this bucket in bulk update in order
              to reach it's siblings for update. */
            bucketPtr->bucketHwUpdateStat =
                PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_UPDATE_SIBLINGS_E;
        }

        if (bucketUpdateMode == LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E)
        {
            /* for root bucket type we need to update the bucket type */
            *bucketTypePtr = newBucBucketType;
        }

        /* update the bucket type and exit */
        bucketPtr->bucketType = *bucketTypePtr;
        return GT_OK;
    }

    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) &&
        (bucketPtr->bucketHwUpdateStat ==
         PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E))
    {
        /* we are in a bulk operation update (delete) and this bucket
           was already found to be resized , so we refer to it as need to be
           resized since it's memory hasn't been resized yet*/

        resizeBucket = GT_TRUE;
    }

    if (bucketUpdateMode == LPM_BUCKET_UPDATE_SHADOW_ONLY_E)
    {
        /* this means we are in a stage where we just update the shadow
           the only released memory here is that of empty buckets */
        freeMemListPtr = &shadowPtr->freeMemListEndOfUpdate;
    }
    else
    {
        /* check what kind of memory this bucket occupies according to it HW update
           status */
        freeMemListPtr = (bucketPtr->bucketHwUpdateStat ==
                          PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E)?
            &shadowPtr->freeMemListDuringUpdate:
            &shadowPtr->freeMemListEndOfUpdate;
    }

    /* If there is only one range (next hop type), then delete the
       whole bucket */
    if(newBucBucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
        {
            /* swap memory is a pre-allocated memory that should never be freed */
            if ((bucketPtr->hwBucketOffsetHandle!=shadowPtr->swapMemoryAddr)&&
                (bucketPtr->hwBucketOffsetHandle!=shadowPtr->secondSwapMemoryAddr))

            {
                retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwBucketOffsetHandle,
                                                         PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                         freeMemListPtr,shadowPtr);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
            }
        }

        bucketPtr->bucketType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E pointer type
           shouldn't be updated because it represents that it points to a src
           trie (and not the actuall type of the bucket) */
        if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
            *bucketTypePtr = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        bucketPtr->fifthAddress = 0;
        bucketPtr->hwBucketOffsetHandle = 0;

        return GT_OK;
    }

    memBlockBase = lpmEngineMemPtrPtr[0]->structsBase;

    retVal = getMirrorBucketInfo(bucketPtr,lpmEngineMemPtrPtr,newBucBucketType,&memSize,
                                 &writeOffset,&fifthStartAddr);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    /* Allocate memory in PP's RAM if it's resized or never been allocated
       before (hotSync)*/
    if((bucketUpdateMode != LPM_BUCKET_UPDATE_SHADOW_ONLY_E) &&
       ((resizeBucket == GT_TRUE) || (bucketPtr->hwBucketOffsetHandle == 0)))
    {
        /* first check if memory was already allocated for these changes */
        /* if yes , use only that memory! */
        if (shadowPtr->neededMemoryListLen > 0)
        {
            i = shadowPtr->neededMemoryCurIdx;

            if ((i >= shadowPtr->neededMemoryListLen) ||
                (memSize != shadowPtr->neededMemoryBlocksSizes[i]))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* should not happen */

            tempHwAddrHandle = shadowPtr->neededMemoryBlocks[i];
            swapMemInUseForAdd = shadowPtr->neededMemoryBlocksSwapUsedForAdd[i];
            shadowPtr->neededMemoryCurIdx++;
        }
        else
        {
            tempNextMemInfoPtr = lpmEngineMemPtrPtr[0];
            tmpStructsMemPool = lpmEngineMemPtrPtr[0]->structsMemPool;
            /*  this can only happen in lpm Delete, since in an insert
                the memory is preallocated.
                this forces use to use the swap memory , which we will
                swap back after using */
            if (lpmEngineMemPtrPtr[0]->structsMemPool==0)
            {
                tempHwAddrHandle = DMM_BLOCK_NOT_FOUND;
            }
            else
            {
                do
                {
                    tempHwAddrHandle = prvCpssDmmAllocate(tmpStructsMemPool,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSize,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);

                    if (tempNextMemInfoPtr->nextMemInfoPtr == NULL)
                    {
                        break;
                    }
                    tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                    tmpStructsMemPool = tempNextMemInfoPtr->structsMemPool;

                } while (tempHwAddrHandle==DMM_BLOCK_NOT_FOUND);
            }

            if (tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)
            {
                /* we reached a max fragmentation in the PP's memory*/
                /* this can only happen in lpm Delete, since in an insert */
                /* the memory is preallocated. */
                /* this forces use to use the swap memory , which we will*/
                /* swap back after using */
                tempHwAddrHandle = shadowPtr->swapMemoryAddr;

                if (tempHwAddrHandle == 0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
                }

                swapMemInUse = GT_TRUE;
            }
            else
            {
                /* we had a CPU Memory allocation error */
                if(tempHwAddrHandle == DMM_MALLOC_FAIL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                /*  set pending flag for future need */
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);
            }
        }
    }
    else
    {
        tempHwAddrHandle = bucketPtr->hwBucketOffsetHandle;
    }

    /* update the bucket's hw status */
    if (resizeBucket == GT_TRUE)
    {
        bucketPtr->bucketHwUpdateStat =
            PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E;
    }
    else
    {
        if ((bucketPtr->bucketHwUpdateStat == PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E) ||
            (bucketPtr->bucketHwUpdateStat == PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_UPDATE_SIBLINGS_E))
        {
            /* the bucket wasn't resized , but we reached here so it needs
               re-writing in the HW (some pointers were changed in it) */
            bucketPtr->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_NEEDS_UPDATE_E;
        }
    }

    /* Write the tables to PP's RAM for each device that share this LPM table. */

    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E))
    {
        pRange = bucketPtr->rangeList;
        /* Preparation of HW words.     */

        if ((resizeBucket == GT_TRUE) ||
            (forceWriteWholeBucket == GT_TRUE) ||
            (bucketPtr->bucketHwUpdateStat ==
             PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E))
            /*it could be that this bucket was resized but haven't been written
              the the HW*/
        {
            /*
               In case SWAP area is in use:
               1. update pointers according to new pRange and
                  old memory as if this new range is going to be
                  located in the old/original memory.
                  call getMirrorBucketDataAndUpdateRangesAddress
               2. Update the swap memory with the new bucket data
                  call cpssDxChLpmNodeWrite with parameters we get from
                  getMirrorBucketDataAndUpdateRangesAddress
               3. update pointers using call to updateMirrorRange/bucketParentWriteFunc
               4. Delete the old memory
               5. allocate the new memory
               6. update pointers according to new pRange and
                  new memory allocated
                  call getMirrorBucketDataAndUpdateRangesAddress
               7. move the data from the swap to the allocated new space
                  by calling cpssDxChLpmNodeWrite with parameters we get from
                  getMirrorBucketDataAndUpdateRangesAddress and the new address
               */

            /* first get the bucket base addr for sibling buckets parent
               range mem address calculation */
            if ((swapMemInUse == GT_TRUE)|| (swapMemInUseForAdd == GT_TRUE))
            {
                /* if we're using the swap we will not record it but record the
                   bucket's old memory address */
                tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle)) +
                    memBlockBase;
            }
            else
            {
               tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)) + memBlockBase;
            }

            cpssOsMemSet(&rangeSelectionSec,0,
                         sizeof(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT));
            /* create the bucket data, and update the ranges hw address */
            getMirrorBucketDataAndUpdateRangesAddress(pRange,
                                                      lpmEngineMemPtrPtr,
                                                      newBucBucketType,
                                                      tempAddr,
                                                      &rangeSelectionSec,
                                                      nextPointersArray);

            /* Writing to RAM.*/
            do
            {
                tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle) + memBlockBase;
                for (i = 0; i < shareDevListLen; i++)
                {

                    retVal = cpssDxChLpmNodeWrite(shareDevsList[i],
                                                  tempAddr,
                                                  newBucBucketType,
                                                  &rangeSelectionSec,
                                                  bucketPtr->numOfRanges,
                                                  nextPointersArray);
                    if (retVal != GT_OK)
                    {
                        /* this is a fail that do not support any rollback operation,
                           because if we get here it is after we did all calculations
                           needed and we verified that the data can be written to the HW.
                           if we get a fail it means a fatal error that should not be
                           happen and the behavior is unpredicted */

                        /* just need to free any allocations done in previus stage */
                        if(needToFreeAllocationInCaseOfFail == GT_TRUE)
                        {
                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed);

                            prvCpssDmmFree(tempHwAddrHandleToBeFreed);
                        }

                       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                }

                if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) ||
                    (swapMemInUse == GT_TRUE) || (swapMemInUseForAdd==GT_TRUE))
                {
                    oldHwAddrHandle = bucketPtr->hwBucketOffsetHandle;
                     /* ok since the bucket is updated in the HW, we can now
                        update the shadow */
                    bucketPtr->fifthAddress = fifthStartAddr;
                    bucketPtr->bucketType = (GT_U8)newBucBucketType;

                    /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E we
                       don't update the pointer type since it represents that it
                       points to a src trie. (and not the actuall type of the
                       bucket it points to)*/
                    if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
                        *bucketTypePtr = (GT_U8)newBucBucketType;

                    bucketPtr->hwBucketOffsetHandle = tempHwAddrHandle;

                    /* in this mode we immidiatly update the parent range in
                       order to free the bucket's memory for further use .the
                       same goes to if we use the swap memory */
                    if (bucketPtr->pointingRangeMemAddr == 0)
                    {
                        /* if the parentRangeMemAddr = 0 then it means the parent
                           range is not an regular lpm range, thus order it's
                           rewriting using the function ptr*/
                        if (parentWriteFuncPtr->bucketParentWriteFunc(
                            parentWriteFuncPtr->data) != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                        }
                    }
                    else
                    {
                        /* write the bucket's parent range in order to be able to
                           free this bucket's old memory / swap memory
                          (resizing took place)*/
                        if (updateMirrorRange(bucketPtr,
                                              lpmEngineMemPtrPtr,
                                              shadowPtr)
                            != GT_OK)
                        {

                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                        }
                    }

                    /* now it's ok to free this bucket's old memory */
                    if ((freeOldMem == GT_TRUE) &&
                        (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                        (bucketPtr->bucketType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
                        (bucketPtr->bucketType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
                    {
                        oldMemPool = DMM_GET_PARTITION(oldHwAddrHandle);
                        /*  set pending flag for future need */
                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle);

                        prvCpssDmmFree(oldHwAddrHandle);

                        freeOldMem = GT_FALSE;

                    }
                }

                if ((swapMemInUse == GT_FALSE)&&(swapMemInUseForAdd==GT_FALSE))
                    break;

                /* if we reached here the swap memory is in use , and there is
                   a need to move and rewrite the bucket back where it was */

                if (oldMemPool == 0)
                {
                    /* should never happen since we just freed this mem*/
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                tempHwAddrHandle = prvCpssDmmAllocate(oldMemPool,
                                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSize,
                                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
                if ((tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle == DMM_MALLOC_FAIL))
                {
                    /* should never happen since we just freed this mem*/
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                else
                {
                    /* keep new tempHwAddrHandle -- in case of a fail should be freed */
                    needToFreeAllocationInCaseOfFail = GT_TRUE;
                    tempHwAddrHandleToBeFreed = tempHwAddrHandle;

                    /*  set pending flag for future need */
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);

                    /* need to update pointers again after reallocation of new memory
                       create the bucket data, and update the ranges hw address */
                    cpssOsMemSet(&rangeSelectionSec,0,
                         sizeof(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT));
                    tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)) + memBlockBase;
                    getMirrorBucketDataAndUpdateRangesAddress(pRange,
                                                      lpmEngineMemPtrPtr,
                                                      newBucBucketType,
                                                      tempAddr,
                                                      &rangeSelectionSec,
                                                      nextPointersArray);
                }

                /* indicate we're not using the swap memory anymore*/
                swapMemInUse = GT_FALSE;
                swapMemInUseForAdd = GT_FALSE;

            } while (GT_TRUE);

        }
        else
        {
            inBucketWriteOffset = 0;
            while (pRange != NULL)
            {
                /* only if the range needs updating , update it */
                if (pRange->updateRangeInHw == GT_TRUE)
                {
                    retVal =
                        prvCpssDxChLpmRamMngRangeInNextPointerFormatSet(pRange,
                                                                        lpmEngineMemPtrPtr[1],
                                                                        nextPointersArray);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }

                    tempAddr = (writeOffset + inBucketWriteOffset) +
                              PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle) + memBlockBase;

                    /* update the range's bucket parent range memory address */
                    if ((pRange->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                        (pRange->pointerType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
                        (pRange->pointerType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
                    {
                        pRange->lowerLpmPtr.nextBucket->pointingRangeMemAddr =
                            tempAddr;
                    }

                    for (i = 0; i < shareDevListLen; i++)
                    {
                        retVal =
                            cpssDxChLpmNodeNextPointersWrite(shareDevsList[i],
                                                             tempAddr,
                                                             1,
                                                             nextPointersArray);
                        if (retVal != GT_OK)
                        {
                            return (retVal);
                        }
                    }

                    /*  remove tha flag */
                    pRange->updateRangeInHw = GT_FALSE;
                }

                inBucketWriteOffset++;
                pRange = pRange->next;
            }
        }

        /* the bucket's HW update was done. no need to further update it */
        bucketPtr->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;
    }

    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_MEM_ONLY_E))
    {
        /* in cases we deal with memory allocation, check if we need to free the
           old bucket */
        freeOldMem = ((freeOldMem) &&
                      (resizeBucket == GT_TRUE) &&
                      (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                      (bucketPtr->bucketType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
                      (bucketPtr->bucketType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E)); /* not a new bucket */

        if (freeOldMem)
        {
            /* swap memory is a pre-allocated memory that should never be freed */
            if ((bucketPtr->hwBucketOffsetHandle!=shadowPtr->swapMemoryAddr)&&
                (bucketPtr->hwBucketOffsetHandle!=shadowPtr->secondSwapMemoryAddr))
            {
                /* Free the old bucket.  */
                retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwBucketOffsetHandle,
                                                         PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                         freeMemListPtr,shadowPtr);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
            }
        }

        bucketPtr->hwBucketOffsetHandle = tempHwAddrHandle;
    }

    /* in LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E mode , this shadow update part
       already been done */
    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_MEM_ONLY_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_SHADOW_ONLY_E))
    {
        /* update the bucket's info info */
        bucketPtr->fifthAddress = fifthStartAddr;
        bucketPtr->bucketType = newBucBucketType;

        /* update the range lower Level bucket type */
        /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E we don't update
           the lower Level bucket type (pointer type) since it represents that
           it points to a src trie. (and not the actuall type of the bucket it
           points to)*/
        if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
            *bucketTypePtr = newBucBucketType;
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *pBucket; /* The first level bucket */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;

    pBucket = createNewBucket(nextHopPtr,firstLevelPrefixLen);
    if(pBucket == NULL)
        return NULL;

    pRange = pBucket->rangeList;

    /* beacuse it's a new bucket there is a need to update it in the HW */
    pRange->updateRangeInHw = GT_TRUE;

    return pBucket;
}

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
)
{

    GT_U8  startAddr;           /* The startAddr and endAddr of the     */
    GT_U8  endAddr;             /* given prefix in the current level.   */

    GT_U32 rangePrefixLength;   /* The prefix length represented by the */
                                /* current range's mask field.          */
    GT_U32 newPrefixLength;     /* The new prefix length that the       */
                                /* current range represents.            */
    GT_U32 numOfNewRanges = 0;  /* Number of new created ranges as a    */
                                /* result of insertion.                 */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC newRange;/* the new created range from the insert */
    GT_STATUS retVal= GT_OK;           /* functions returned values.           */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pRange;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *parentRangeToUpdateDueToLowerLevelResizePtr = NULL;
    GT_U32  parentRangeLevel = 99; /* initializing with and invalid level*/
    GT_BOOL useCompressed2; /* indicates wheather to use a compressed 2 type
                               when allocting this bucket memory */
    GT_BOOL indicateSiblingUpdate = GT_FALSE;
    GT_U32  level = 0;
    GT_BOOL goDown;
    GT_BOOL isDestTreeRootBucket;

    LPM_BUCKET_UPDATE_MODE_ENT bucketUpdateMode;

    static GT_BOOL                                  resizeBucket[MAX_LPM_LEVELS_CNS];
    static GT_BOOL                                  doHwUpdate[MAX_LPM_LEVELS_CNS];
    static PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtrArry[MAX_LPM_LEVELS_CNS];
    static CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      *pointerTypePtrArray[MAX_LPM_LEVELS_CNS];
    static PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRangeArray[MAX_LPM_LEVELS_CNS];

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  **currBucketPtr = bucketPtrArry;
    GT_U8                                    *currAddrByte = addrCurBytePtr;
    GT_U32                                   currPrefix = prefix;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC       **currLpmEngineMemPtr = lpmEngineMemPtrPtr;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      **currBucketPtrType = pointerTypePtrArray;
    GT_BOOL                                  *resizeBucketPtr = resizeBucket;
    GT_BOOL                                  *doHwUpdatePtr = doHwUpdate;
    GT_U32                                   i = 0;
    GT_UINTPTR                               rollBackHwBucketOffsetHandle = 0;
    LPM_BUCKET_UPDATE_MODE_ENT               rollBackBucketUpdateMode = LPM_BUCKET_UPDATE_NONE_E;
    GT_BOOL                                  rollBackResizeBucket = GT_FALSE;
    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */

    cpssOsMemSet(&newRange,0,sizeof(newRange));

    /* zero the arrays */
    cpssOsMemSet(resizeBucket,GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(doHwUpdate,GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(bucketPtrArry,0,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(pRangeArray,0xFF, /*0xFFFFFFFF indicates untouched level*/
                 sizeof(pRangeArray));

    /* set the initial values.*/
    bucketPtrArry[0] = bucketPtr;
    pointerTypePtrArray[0] = bucketTypePtr;
    pRange = pRangeArray;

    /* first do the lpm tree shadow update part */
    while (currBucketPtr >= bucketPtrArry)
    {
        /* we start with the assumption we'll go up the bucket trie */
        goDown = GT_FALSE;

        /* if the range pointer reached NULL means that we finished with this level */
        if (*pRange != NULL)
        {
            calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);

            /* check if we need to continue this level or this a brand new level
               (0xFFFFFFFF) */
            if (*pRange == (PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *)(~((GT_UINTPTR)0)))
            {
                *pRange = find1stOverlap(*currBucketPtr,startAddr,NULL);
                /* If belongs to this level, insert it into trie.    */
                if (currPrefix <= PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                {
                    retVal = insert2Trie(*currBucketPtr,startAddr,
                                         currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,nextPtr);
                    if (retVal != GT_OK)
                    {
                        /* failed on last level*/
                        for (i=1; i <= level; i++)
                        {
                            if (*(pointerTypePtrArray[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                            {
                                *(pointerTypePtrArray[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                            }
                        }
                        if (shadowPtr->neededMemoryListLen > 0)
                        {
                            for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                            {
                                prvCpssDmmFree(shadowPtr->neededMemoryBlocks[i]);
                            }
                        }
                        /* create mask for current range */
                        (*pRange)->mask = (GT_U8)PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                        return retVal;
                    }

                }
            }

            do
            {
                rangePrefixLength = mask2PrefixLength((*pRange)->mask,
                                                      PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                if (currPrefix >= rangePrefixLength)
                {
                    if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* Insert the given addr. to the next level.    */
                        if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                        {
                            /* Create a new bucket with the dominant next_pointer*/
                            /* value of the upper level as the default route.    */
                            newRange.lowerLpmPtr.nextBucket =
                                createNewBucket((*pRange)->lowerLpmPtr.nextHopEntry,
                                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                            if (newRange.lowerLpmPtr.nextBucket == NULL)
                            {
                                for (i=1; i <= level; i++)
                                {
                                    if (*(pointerTypePtrArray[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                                    {
                                        *(pointerTypePtrArray[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                                    }
                                }
                                if (shadowPtr->neededMemoryListLen > 0)
                                {
                                    for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                                    {
                                        prvCpssDmmFree(shadowPtr->neededMemoryBlocks[i]);
                                    }
                                }
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                            }

                            /* the new range prefix len is as of what was the prefixlen*/
                            newPrefixLength = rangePrefixLength;

                            /* we set the new range pointer type to next hop
                               since we still don't know what kind of bucket it will
                               be (as in createNewBucket) */
                            newRange.pointerType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;

                            /* Insert the given entry to the next level.     */
                            currBucketPtr[1] = newRange.lowerLpmPtr.nextBucket;

                            /* indicate we need to proceed down the bucket trie*/
                            goDown = GT_TRUE;
                        }
                        else
                        {
                            /* this is the end level. record in the new range
                               the given next ptr */
                            newRange.pointerType = (GT_U8)nextPtrType;
                            if ((nextPtrType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                                (nextPtrType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                                (nextPtrType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
                            {
                                newRange.lowerLpmPtr.nextHopEntry = nextPtr;
                            }
                            else
                            {
                                /* nextPtrType ==
                                   PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E */
                                newRange.lowerLpmPtr.nextBucket = nextPtr;
                            }

                            /* the new range prefix len is as of what's left of
                               the prefix length */
                            newPrefixLength = currPrefix;
                        }
                        /* Create new ranges if needed, as a result of inserting    */
                        /* the new range on a pre-existing range.                   */
                        retVal = splitRange(pRange,startAddr,endAddr,
                                            newPrefixLength,&newRange,
                                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,updateOldPtr,
                                            &numOfNewRanges,
                                            &(currBucketPtrType[1]));
                        if (retVal != GT_OK)
                        {
                            if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                            {
                                /* delete new bucket that was created just before fail */
                                prvCpssDxChLpmRamMngBucketDelete(newRange.lowerLpmPtr.nextBucket,
                                                                 PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, NULL);
                            }
                            else
                            {
                                /* create mask for current range */
                                (*pRange)->mask = (GT_U8)PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                            }
                            for (i=1; i <= level; i++)
                            {
                                if (*(pointerTypePtrArray[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                                {
                                    *(pointerTypePtrArray[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                                }
                            }

                            if (shadowPtr->neededMemoryListLen > 0)
                            {
                                for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                                {
                                    prvCpssDmmFree(shadowPtr->neededMemoryBlocks[i]);
                                }
                            }
                            return retVal;
                        }


                        if (numOfNewRanges == 0)
                        {
                            /* An overwrite has been done, need to update the
                               HW.*/
                            *doHwUpdatePtr = GT_TRUE;
                        }
                        else
                        {
                            /* a change happend to the structure of the bucket
                               clear the cash*/
                            (*currBucketPtr)->rangeCash = NULL;

                            /* indicate that this bucket needs a resize*/
                            *resizeBucketPtr = GT_TRUE;

                            /* Add to the amount of ranges */
                            (*currBucketPtr)->numOfRanges = (GT_U16)((*currBucketPtr)->numOfRanges + numOfNewRanges);

                            /* and update the pointing range it needs to be
                               rewritten (if indeed this is the level after
                               that recorded pointing range) */
                            if ((parentRangeToUpdateDueToLowerLevelResizePtr != NULL) &&
                                ((parentRangeLevel +1) == level))
                            {
                                parentRangeToUpdateDueToLowerLevelResizePtr->updateRangeInHw =
                                    GT_TRUE;
                                /* indicate to the upper level that it needs
                                   a HW update*/
                                *(doHwUpdatePtr - 1) = GT_TRUE;
                            }
                        }
                    }
                    else
                    {
                        /* Insert the addr. to the existing next bucket. */
                        currBucketPtr[1] = (*pRange)->lowerLpmPtr.nextBucket;
                        currBucketPtrType[1] = &((*pRange)->pointerType);

                        /* indicate we need to proceed down the bucket trie*/
                        goDown = GT_TRUE;

                        if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                        {
                            /* in the case the prefix ends at lower level it
                               may cause the a resize in that lower level which
                               will cause this pointed bucket to change
                               location. so we record the range so the lower
                               level could indicate to this level that in needs
                               to rewrite only pointer and not the whole
                               bucket.
                               Note! if the prefix ends in this level
                               or it ended in higher level, there could be a
                               resize in lower leves! */
                            parentRangeToUpdateDueToLowerLevelResizePtr = *pRange;
                            parentRangeLevel = level;
                        }

                        (*pRange)->mask |=
                            PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                    }
                }
                else
                {
                    (*pRange)->mask |=
                        PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                }

                *pRange = (*pRange)->next;

            }while (((*pRange) != NULL) && (endAddr >= (*pRange)->startAddr) &&
                    (goDown == GT_FALSE));

            /* check if we finished with the ranges we need to go over */
            if (((*pRange) != NULL) && (endAddr < (*pRange)->startAddr))
                (*pRange) = NULL;
        }

        /* check which direction we go in the tree */
        if (goDown == GT_TRUE)
        {
            /* we're going down. so advance in the pointer arrays */
            currBucketPtr++;
            currBucketPtrType++;
            doHwUpdatePtr++;
            resizeBucketPtr++;
            pRange++;

            /* and advance in the prefix information */
            currAddrByte++;
            currLpmEngineMemPtr++;
            level++;
            currPrefix = BUCKET_PREFIX_AT_LEVEL_MAC(prefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                                  level);
        }
        else
        {
            /* we're going up, meaning we finished with the current level.
               write it to the HW and move back the pointers in the arrays*/

            /* if the bucket need resizing always update the HW */
            if (*resizeBucketPtr == GT_TRUE)
                *doHwUpdatePtr = GT_TRUE;

            if ((currBucketPtr == bucketPtrArry) && (isMcSrcTree == GT_FALSE))
            {
                isDestTreeRootBucket = GT_TRUE;
            }
            else
            {
                isDestTreeRootBucket = GT_FALSE;
            }

            /* during hot sync we shouldn't touch the Hw or the allocation
               it will be done in the end  */
            if (isDestTreeRootBucket &&
                (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_RAM_UPDATE_E))
            {
                bucketUpdateMode = LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E;
                indicateSiblingUpdate =
                    (insertMode == PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)?
                    GT_TRUE : GT_FALSE;
            }
            else switch (insertMode)
            {
                case PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E:
                    bucketUpdateMode = LPM_BUCKET_UPDATE_SHADOW_ONLY_E;
                    break;
                case PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E:
                    if (*doHwUpdatePtr == GT_TRUE)
                    {
                        bucketUpdateMode = LPM_BUCKET_UPDATE_MEM_ONLY_E;
                    }
                    else
                    {
                        bucketUpdateMode = LPM_BUCKET_UPDATE_NONE_E;
                        indicateSiblingUpdate = GT_FALSE;
                    }

                    break;
                case PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E:
                    bucketUpdateMode = (*doHwUpdatePtr == GT_TRUE) ?
                                       LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E:
                                       LPM_BUCKET_UPDATE_NONE_E;
                    break;
                 default:
                     for (i=1; i <= level; i++)
                     {
                         if (*(pointerTypePtrArray[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                         {
                             *(pointerTypePtrArray[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                         }
                     }

                     if (shadowPtr->neededMemoryListLen > 0)
                     {
                         for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                         {
                             prvCpssDmmFree(shadowPtr->neededMemoryBlocks[i]);
                         }
                     }
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            useCompressed2 =
                (isDestTreeRootBucket &&
                 (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E))?
                GT_FALSE : GT_TRUE;

            /* save rollback information */
            rollBackHwBucketOffsetHandle = (*currBucketPtr)->hwBucketOffsetHandle;
            rollBackBucketUpdateMode = bucketUpdateMode;
            rollBackResizeBucket = *resizeBucketPtr;

            /* The the shadow structures to the RAM.    */
            retVal = updateMirrorBucket(*currBucketPtr,bucketUpdateMode,
                                        isDestTreeRootBucket,*resizeBucketPtr,GT_FALSE,
                                        indicateSiblingUpdate,
                                        useCompressed2,currLpmEngineMemPtr,
                                        *currBucketPtrType,shadowPtr,parentWriteFuncPtr);
            if (retVal != GT_OK)
            {
                (*currBucketPtr)->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;
                for (i=1; i <= level; i++)
                {
                    if (*(pointerTypePtrArray[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                    {
                        *(pointerTypePtrArray[i]) = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
                    }
                }

                if((rollBackBucketUpdateMode != LPM_BUCKET_UPDATE_SHADOW_ONLY_E) &&
                   ((rollBackResizeBucket == GT_TRUE) || (rollBackHwBucketOffsetHandle == 0)))
                {
                    /* incase we have a fail in the updateMirrorBucket and we are using the
                       swap area inorder to do resize, this means that in the PP memory
                       estimation stage we didn't do any memory allocation  - so nothing to free */
                    if (((*currBucketPtr)->hwBucketOffsetHandle == shadowPtr->swapMemoryAddr) ||
                        /* hw update was not successful */
                        (rollBackHwBucketOffsetHandle == (*currBucketPtr)->hwBucketOffsetHandle))
                    {
                        /* remove memory allocation that was done for this bucket on */
                        /* PP memory estimation stage */
                        if (shadowPtr->neededMemoryListLen > 0)
                        {
                            if (shadowPtr->neededMemoryCurIdx > 0)
                            {
                                for (i = shadowPtr->neededMemoryCurIdx -1; i < shadowPtr->neededMemoryListLen; i++)
                                {
                                    if((shadowPtr->neededMemoryBlocks[i]!=shadowPtr->swapMemoryAddr)&&
                                       (shadowPtr->neededMemoryBlocks[i]!=shadowPtr->secondSwapMemoryAddr))
                                    {
                                        /*  set pending flag for future need */
                                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocks[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocks[i]);

                                        prvCpssDmmFree(shadowPtr->neededMemoryBlocks[i]);
                                    }
                                    else
                                    {
                                        /* nothing to free */
                                    }
                                }
                            }
                        }
                    }
                }
                else
                    if(rollBackResizeBucket == GT_FALSE)
                    {
                        /* it was not successfull update of existing bucket */
                        /* free all preallocated memory memory */
                        if (shadowPtr->neededMemoryListLen > 0)
                        {
                            for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                            {
                                if((shadowPtr->neededMemoryBlocks[i]!=shadowPtr->swapMemoryAddr)&&
                                   (shadowPtr->neededMemoryBlocks[i]!=shadowPtr->secondSwapMemoryAddr))
                                {
                                    /*  set pending flag for future need */
                                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocks[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                        PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocks[i]);

                                    prvCpssDmmFree(shadowPtr->neededMemoryBlocks[i]);
                                }
                            }
                        }
                    }
                return retVal;
            }

            /* update partition block */
            if ((*currBucketPtr)->hwBucketOffsetHandle!=0)
            {
                SET_DMM_BLOCK_PROTOCOL((*currBucketPtr)->hwBucketOffsetHandle, protocol);
                SET_DMM_BUCKET_SW_ADDRESS((*currBucketPtr)->hwBucketOffsetHandle, (*currBucketPtr));
            }
            /* move back in the arrays*/
            currBucketPtr--;
            currBucketPtrType--;
            doHwUpdatePtr--;
            resizeBucketPtr--;

            /* before we go up , indicate this level is finished, and ready
               for a fresh level if needed in the future */
            *pRange = (PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *)(~((GT_UINTPTR)0));
            pRange--;

            /* and move back in the prefix information */
            currAddrByte--;
            currLpmEngineMemPtr--;
            level--;
            currPrefix = BUCKET_PREFIX_AT_LEVEL_MAC(prefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                                    level);
        }
    }

    return GT_OK;
}

/**
* @internal lpmGetSplitMethod function
* @endinternal
*
* @brief   Return the split method of a given range in the lpm tree.
*
* @param[in,out] numOfBucketRangesPtr     - Number of bucket ranges before the split.
* @param[in] rangePtr                 - Pointer to the range to be splitted.
* @param[in] startAddr                - The low address of the new range.
* @param[in] endAddr                  - The high address of the new range.
*                                      prefixLength          - The length of the dominant prefix of the new range
* @param[in] levelPrefix              - The current lpm level prefix.
* @param[in,out] numOfBucketRangesPtr     - Number of bucket ranges after the split.
*
* @param[out] splitMethodPtr           - The mode according to which the range should be split.
*
* @retval GT_OK                    - on success.
*/
static GT_STATUS lpmGetSplitMethod
(
    INOUT   GT_U32                                 *numOfBucketRangesPtr,
    IN      PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *rangePtr,
    IN      GT_U8                                  startAddr,
    IN      GT_U8                                  endAddr,
    IN      GT_U8                                  levelPrefix,
    OUT     PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT *splitMethodPtr
)
{
    GT_U8  endRange;           /* Index by which this range ends.      */

    endRange = (GT_U8)(LAST_RANGE_MAC(rangePtr) ? ((1 << levelPrefix) - 1) :
        ((rangePtr->next->startAddr) - 1));

    *splitMethodPtr = (((startAddr > rangePtr->startAddr) ? 1 : 0) +
                    ((endAddr < endRange) ? 2 : 0));

    switch (*splitMethodPtr)
    {
        case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E):
        case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E):
            *numOfBucketRangesPtr += 1;
            break;

        case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E):
            *numOfBucketRangesPtr += 2;
            break;

        case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E):
            *numOfBucketRangesPtr += 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal lpmCalcNeededMemory function
* @endinternal
*
* @brief   Calc the ammount of needed Pp memory for unicast address insertion.
*
* @param[in] bucketPtr                - Pointer to the root bucket.
* @param[in] addrCurBytePtr           - The Byte Array represnting the Address associated
*                                      with the entry.
* @param[in] prefix                   - The address prefix.
* @param[in] rootBucketFlag           - Indicates the way to deal with a root bucket (if it is).
* @param[in] lpmEngineMemPtrPtr       points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in] insertMode               - how to insert the prefix
*                                      (see PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT)
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
* @param[in,out] neededMemList[]          - List holding the memory needed for inserting the given
*                                      address.
* @param[in,out] memPoolList[]            - List of memory pools from which the blocks should be
*                                      allocated from.
* @param[in,out] memListLenPtr            - Nuumber of memory blocks in neededMemList.
* @param[in,out] neededMemList[]          - List holding the memory needed for inserting the given
*                                      address (in LPM lines)
* @param[in,out] memPoolList[]            - List of memory pools from which the blocks should be
*                                      allocated from.
* @param[in,out] memListLenPtr            - Number of memory blocks in neededMemList.
* @param[in,out] memListOctetIndexList[]  - List holding the Octet Index of each memory needed for
*                                      inserting the given address
* @param[in,out] memListOldHandleAddr[]   - List holding the handle address of old bucket range
* @param[in,out] memListSwapCanBeUseful[] - List holding if swap area can be useful to minimize
*                                      memory usage in case of resize
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - Otherwise.
*
* @note - This function calculates the needed memory only in the tree layers
*       that are are needed to be written to HW (using rootBucketFlag)
*
*/
static GT_STATUS lpmCalcNeededMemory
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
    IN    GT_U8                                         *addrCurBytePtr,
    IN    GT_U32                                        prefix,
    IN    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT    rootBucketFlag,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC            **lpmEngineMemPtrPtr,
    IN    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT    insertMode,
    IN    GT_BOOL                                       defragmentationEnable,
    INOUT GT_U32                                        *neededMemList[],
    INOUT GT_UINTPTR                                    *memPoolList[],
    INOUT GT_U32                                        *memListLenPtr,
    INOUT GT_U32                                        *memListOctetIndexList[],
    INOUT GT_UINTPTR                                    *memListOldHandleAddr[],
    INOUT GT_BOOL                                       *memListSwapCanBeUseful[]
)
{
    GT_U8  startAddr;           /* The startAddr and endAddr of the     */
    GT_U8  endAddr;             /* given prefix in the current level.   */

    GT_U32 rangePrefixLength;   /* The prefix length represented by the */
                                /* current range's mask field.          */
    GT_STATUS   retVal = GT_OK; /* functions returned values.           */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;         /* The first range that overlaps with   */
                                /* the given address * prefix.               */
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT    splitMethod;  /* The method according to which the    */
                                                            /* range should be split.               */
    GT_U32          numOfBucketRanges;  /* Number of ranges in the      */
                                /* current's level's bucket.            */
    GT_U32          currNumOfBucketRanges;  /* Number of ranges in the      */
                                /* current's level's bucket.            */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *currBucketPtr = bucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *tempCurrBucketPtr = bucketPtr;/* to be used to check potential use of swap area */
    GT_U8 *currAddrByte = addrCurBytePtr;
    GT_U32 currPrefix = prefix;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC **currLpmEngineMemPtr = lpmEngineMemPtrPtr;
    GT_U32 octetIndex=0;

    GT_BOOL freeBlockCanBeFound;

    while(currPrefix > 0)
    {
        freeBlockCanBeFound=GT_FALSE;

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
        if (currBucketPtr == NULL)
        {
            /* This is a new tree level, a 1-line bucket should be created, */
            /* including 2 or 3 lines.                                      */
            if ((startAddr == 0) || (endAddr == 0xFF))
                **neededMemList = 3;
            else
                **neededMemList = 4;

            **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
            (*memListLenPtr)++;
            **memListOctetIndexList=octetIndex;
            (*memPoolList)--;
            (*neededMemList)--;
            (*memListOctetIndexList)--;
             **memListSwapCanBeUseful = GT_FALSE;
             (*memListSwapCanBeUseful)--;
             **memListOldHandleAddr = 0;
             (*memListOldHandleAddr)--;

            /* next level doesn't exists as well. so set it to null        */
            currBucketPtr = NULL;
        }
        else
        {
            pRange = find1stOverlap(currBucketPtr,startAddr,NULL);

            numOfBucketRanges = currNumOfBucketRanges = currBucketPtr->numOfRanges;

            /* Go over all levels until the first level to be written to HW.    */
            if (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_RAM_UPDATE_E)
            {
                /* Calculate the needed memory of the next level bucket.        */
                if ((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    (pRange->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                    (pRange->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                    ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                {
                    currBucketPtr = NULL;
                }
                else
                {
                    currBucketPtr = pRange->lowerLpmPtr.nextBucket;
                }
            }
            else do
            {
                rangePrefixLength = mask2PrefixLength(pRange->mask,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                if (currPrefix >= rangePrefixLength)
                {
                    /* the inserted prefix covers all or part of this range */
                    if ((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        (pRange->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                        (pRange->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* the range points to an end of tree (next-hop or
                           src bucket */

                        lpmGetSplitMethod(&numOfBucketRanges,pRange,startAddr,
                                          endAddr,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                          &splitMethod);

                        if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                        {
                            /* the prefix needs to be inserted deeper thus in
                            this case we Need to create new tree levels, all of
                            them of type "1-line bucket" including 2 or 3 lines.*/

                            /* (will reach here only once per do-while loop) */
                            currBucketPtr = NULL;
                        }
                    }
                    else
                    {
                        /* the next pointer points to a bucket  */
                        /* (will reach here only once per do-while loop, in
                            the case we need to go deeper. in other words: if
                            the insertion will cause lower levels to be
                            resized - this is the lower level bucket. if we
                            are on the last level to be resized then it can
                            reach this line several times, but it will have no
                            effect) */
                        currBucketPtr = pRange->lowerLpmPtr.nextBucket;
                    }
                }

                pRange = pRange->next;
            }while ((pRange != NULL) && (endAddr >= pRange->startAddr));

            /* check if a resize is needed, if not return */
            if (numOfBucketRanges != currNumOfBucketRanges)
            {
                if (numOfBucketRanges <= MAX_NUMBER_OF_COMPRESSED_1_RANGES_CNS)
                {
                    /* One line bucket: One word for the bit-vector +
                       next pointers. */
                    **neededMemList  = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS + numOfBucketRanges;
                }
                else if ((numOfBucketRanges <= MAX_NUMBER_OF_COMPRESSED_2_RANGES_CNS) &&
                         (rootBucketFlag != PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E))
                {
                    /* Two lines bucket: 2 words for the bit-vector +
                       next pointers. */
                    **neededMemList  = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS + numOfBucketRanges;
                }
                else
                {
                    /* Regular bucket: 11 words for the bit-vector +
                       next pointers.*/
                    **neededMemList  = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS + numOfBucketRanges;
                }


                /* Check if incase we use a swap area, the memory can be added.
                   this means that there is free block near the used block and
                   we can use them to create a new bigger block.
                   it presumes that current block is the same block that will
                   be freed as a result of add prefix operation.
                   in case of add-bulk operation we will not support defrag
                   and the use of swap area*/
                if ((defragmentationEnable==GT_TRUE)&&
                     (tempCurrBucketPtr != NULL)&&
                     (insertMode!=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E)&&
                     ((tempCurrBucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)||
                     (tempCurrBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)||
                     (tempCurrBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)))
                {
                    retVal = prvCpssDxChLpmRamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock(tempCurrBucketPtr,
                                                                                 **neededMemList,
                                                                                 &freeBlockCanBeFound);
                    if (retVal!=GT_OK)
                    {
                        return retVal;
                    }
                }

                if (freeBlockCanBeFound==GT_TRUE)
                {
                    **memListSwapCanBeUseful = GT_TRUE;
                    (*memListSwapCanBeUseful)--;
                    **memListOldHandleAddr = tempCurrBucketPtr->hwBucketOffsetHandle;
                    (*memListOldHandleAddr)--;
                }
                else
                {
                    **memListSwapCanBeUseful = GT_FALSE;
                    (*memListSwapCanBeUseful)--;
                    **memListOldHandleAddr = tempCurrBucketPtr->hwBucketOffsetHandle;
                    (*memListOldHandleAddr)--;
                }


                **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                (*memListLenPtr)++;
                (*memPoolList)--;
                (*neededMemList)--;
                **memListOctetIndexList=octetIndex;
                (*memListOctetIndexList)--;
            }
        }

        tempCurrBucketPtr = currBucketPtr;
        currAddrByte++;
        currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
        rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E;
        currLpmEngineMemPtr++;
        octetIndex++;
    }

    return retVal;
}

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
)
{
    GT_U32  rightStartAddr;      /* The start address of the right range */
                                 /* after taking away the unneeded LSBs  */
                                 /* according to the range's mask.       */
    GT_U32  rightPrefix;         /* Prefix length of the right range.    */
    GT_U32  leftPrefix;          /* Prefix length of the left range.     */

    if ((leftRangePtr == NULL) || (rightRangePtr == NULL))
        return 0;

    if (((leftRangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
         (leftRangePtr->pointerType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
         (leftRangePtr->pointerType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) &&
         ((GT_U32)(leftRangePtr->pointerType) != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)) ||
        ((rightRangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
         (rightRangePtr->pointerType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
         (rightRangePtr->pointerType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) &&
         ((GT_U32)(rightRangePtr->pointerType) != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)))
        return 0;

    rightPrefix = mask2PrefixLength(rightRangePtr->mask,levelPrefix);
    leftPrefix  = mask2PrefixLength(leftRangePtr->mask,levelPrefix);

    rightStartAddr = (rightRangePtr->startAddr & (0xFFFFFFFF <<
                                               (levelPrefix - rightPrefix)));

    if((leftRangePtr->startAddr >= rightStartAddr) && (leftPrefix == rightPrefix))
        return 1;

    return 0;
}

/**
* @internal mergeRange function
* @endinternal
*
* @brief   This function merges a range. According to one of the following possible
*         merges:
*         PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E, PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E,
*         PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E and PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E
* @param[in,out] rangePtr                 - Pointer to the range to be merged.
* @param[in] startAddr                - The low address of the range to be merged.
* @param[in] prefixLength             - The length of the dominant prefix of the range.
* @param[in] prevRangePtr             - A pointer to the lower address neighbor of the range to be
*                                      merged. NULL if rangePtr is the 1st in the list.
* @param[in] levelPrefix              - The current lpm level prefix.
* @param[in,out] rangePtr                 - A pointer to the next node in the list.
*
* @param[out] numOfNewRangesPtr        - Number of new created ranges as a result of the split.
*                                       GT_OK on success, GT_FAIL otherwise.
*/
static GT_STATUS mergeRange
(
    INOUT PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **rangePtr,
    IN    GT_U32                                    startAddr,
    IN    GT_U32                                    prefixLength,
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *prevRangePtr,
    IN    GT_U8                                     levelPrefix,
    OUT   GT_32                                     *numOfNewRangesPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *curRangePtr = (*rangePtr);/* Temp vars*/

    PRV_CPSS_DXCH_LPM_RAM_MERGE_METHOD_ENT mergeMethod;     /* The method by which too perform the  */
                                                            /* ranges merge.                        */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tmpPtr; /* Temporary range pointer to be used   */
                                /* in range operation.                  */

    /* warnings fix */
    startAddr = startAddr;
    prefixLength = prefixLength;

    mergeMethod = ((mergeCheck(curRangePtr,curRangePtr->next,
                               levelPrefix) ? 1 : 0) +
                   (mergeCheck(prevRangePtr,curRangePtr,levelPrefix) ? 2 : 0));

    switch (mergeMethod)
    {
    case PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E: /* The old range fell       */
                                                  /* exactly on a older range */
        *numOfNewRangesPtr = 0;

        curRangePtr->updateRangeInHw = GT_TRUE;
        break;

    case PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E:  /* Merge the upper part of the range */
        curRangePtr->mask = curRangePtr->next->mask;

        curRangePtr->lowerLpmPtr = curRangePtr->next->lowerLpmPtr;
        curRangePtr->pointerType = curRangePtr->next->pointerType;

        tmpPtr = curRangePtr->next;
        curRangePtr->next = curRangePtr->next->next;
        (*rangePtr) = curRangePtr;
        cpssOsLpmFree(tmpPtr);
        tmpPtr = NULL;
        *numOfNewRangesPtr = -1;

        curRangePtr->updateRangeInHw = GT_TRUE;

        break;

    case PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E: /* Merge the lower part of the range */

        prevRangePtr->next = curRangePtr->next;

        cpssOsLpmFree(curRangePtr);
        curRangePtr = NULL;
        (*rangePtr) = prevRangePtr;
        *numOfNewRangesPtr = -1;

        prevRangePtr->updateRangeInHw = GT_TRUE;
        break;

    case PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E: /* Merge the ranges from both */
                                            /* sides of the range.        */

        prevRangePtr->next = curRangePtr->next->next;

        cpssOsLpmFree(curRangePtr->next);
        curRangePtr->next = NULL;

        cpssOsLpmFree(curRangePtr);
        curRangePtr = NULL;

        (*rangePtr) = prevRangePtr;
        *numOfNewRangesPtr = -2;

        prevRangePtr->updateRangeInHw = GT_TRUE;
        break;

    default:
        break;
    }

    /* the range has expanded or an overwrite was done, need to update in HW */

    return GT_OK;
}

/**
* @internal getFromTrie function
* @endinternal
*
* @brief   Returns a next hop entry from the trie structure.
*
* @param[in] bucketPtr                - Bucket to which the given entry is associated.
* @param[in] startAddr                - Start Addr of the address to look for in the trie.
* @param[in] prefix                   - The address prefix.
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
)
{
    GT_PTR nextPointer = NULL; /* Trie search result.              */
    GT_U8 addr[1];             /* startAddr representation as      */
                               /* GT_U8.                           */

    addr[0] = (GT_U8)(startAddr & (BIT_MASK_MAC(levelPrefix) << (levelPrefix - prefix)));

    if(prvCpssDxChLpmTrieSearch(&(bucketPtr->trieRoot),addr,prefix,levelPrefix,
                                &nextPointer)== NULL)
        return NULL;

    return (nextPointer);
}

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
* @param[in] prefix                   - The address prefix.
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
)
{
    GT_STATUS rc;
    GT_PTR nextPointer;      /* Trie deleted node data pointer.          */
    GT_U8 addr[1];           /* startAddr & prefix representation        */

    addr[0] = (GT_U8)(startAddr & (BIT_MASK_MAC(trieDepth) << (trieDepth - prefix)));

    rc = prvCpssDxChLpmTrieDel(&(bucketPtr->trieRoot),addr,prefix,trieDepth,
                          &nextPointer);

    if(nextPointer == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    else
    {
        *delEntryPtr = nextPointer;
        return GT_OK;
    }
}

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
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr,
    IN  GT_U8                                   levelPrefix,
    OUT GT_PTR                                  *pNextPtr
)
{
    GT_PTR delEntry = 0;         /* Data stored in the Trie   */

    if (bucketPtr->numOfRanges != 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    delFromTrie(bucketPtr,0,0,levelPrefix,&delEntry);

    if(pNextPtr != NULL)
        *pNextPtr = delEntry;

    cpssOsLpmFree(bucketPtr->rangeList);
    bucketPtr->rangeList = NULL;

    cpssOsLpmFree(bucketPtr);
    bucketPtr = NULL;

    return GT_OK;
}

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
)
{
    GT_U8 startAddr;           /* The startAddr and endAddr of the     */
    GT_U8 endAddr;             /* given prefix in the current level.   */

    GT_U32 rangePrefixLength;   /* The prefix length represented by the */
                                /* current range's mask field.          */
    GT_U32 newPrefixLength;     /* The new prefix length that the       */
                                /* current range represents.            */

    GT_U8  newStartAddr;        /* The startAddr of the range that      */
                                /* should replace the deleted one.      */
    GT_BOOL resizeBucket;       /* Was the bucket resized as a  */
                                /* result of deletion.                  */
    GT_BOOL doHwUpdate;         /* is it needed to update the HW */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *newNextHopPtr;/* Next hop representing
                                                           the prefix that
                                                           replaces the deleted
                                                           one. */
    GT_32 numOfNewRanges;       /* Number of new ranges as a result of  */
                                /* range merging.                       */
    LPM_BUCKET_UPDATE_MODE_ENT bucketUpdateMode;/* indicates whether to write
                                                   the bucket to the device's
                                                   RAM.*/
    GT_BOOL useCompressed2; /* indicates wheather to use a compressed 2 type
                               when allocting this bucket memory */
    GT_BOOL indicateSiblingUpdate;
    GT_PTR  nextPtr;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT  insertMode;

    GT_U32 level = 0;
    GT_BOOL updateOld,updateUpperLevel = GT_FALSE;
    GT_STATUS retVal = GT_OK;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pPrevRange;/* The first range that overlaps with   */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pRange;    /* address & prefix, and a pointer to  */
                                                        /* the range that comes before it.      */

    static PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT       *lpmPtrArray[MAX_LPM_LEVELS_CNS];

    static PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *pRangeArray[MAX_LPM_LEVELS_CNS];
    static PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *pPrvRangeArray[MAX_LPM_LEVELS_CNS];
    static CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT            *bucketPtrTypeArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT              **currLpmPtr;
    GT_U8                                                 *currAddrByte;
    GT_U32                                                currPrefix;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                   **currBucketPtrType;
    GT_BOOL                                               isDestTreeRootBucket;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                   newNextHopPtrType;
    currAddrByte = addrCurBytePtr;
    currPrefix = prefix;

    cpssOsMemSet(lpmPtrArray,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(pRangeArray,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(pPrvRangeArray,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(bucketPtrTypeArray,0,sizeof(GT_U8*)*MAX_LPM_LEVELS_CNS);

    bucketPtrTypeArray[0] = bucketTypePtr;
    lpmPtrArray[0] = lpmPtr;
    currLpmPtr = lpmPtrArray;
    pPrevRange = pPrvRangeArray;
    pRange = pRangeArray;
    currBucketPtrType = bucketPtrTypeArray;

    /* first run and record all the neccesary info down the bucket tree */
    for(;;)
    {
        if (*currLpmPtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG); /* the prefix is not in the tree */

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);

        *pRange = find1stOverlap((*currLpmPtr)->nextBucket,startAddr,pPrevRange);
        /* The deleted prefix ends in a deeper level,   */
        /* call the delete function recursively.        */
        if ((currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) &&
            ((*pRange)->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            ((*pRange)->pointerType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
            ((*pRange)->pointerType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) &&
            ((GT_U32)((*pRange)->pointerType) != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            currLpmPtr[1] = &((*pRange)->lowerLpmPtr);
            currBucketPtrType[1] = &((*pRange)->pointerType);
            currAddrByte++;
            lpmEngineMemPtrPtr++;

            currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

            /* advance the array pointers */
            currLpmPtr++;
            pRange++;
            pPrevRange++;
            currBucketPtrType++;
            level++;
        }
        else
            break;
    }

    /* now were on the lowest level for this prefix, delete the from trie */
    retVal = delFromTrie((*currLpmPtr)->nextBucket,startAddr,currPrefix,
                         PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,delEntryPtr);
    if (retVal != GT_OK)
    {
        if (rollBack == GT_FALSE)
        {
            return retVal;
        }
    }

    while (currLpmPtr >= lpmPtrArray)
    {
        /* reset the flags */
        resizeBucket          = GT_FALSE;
        doHwUpdate            = GT_FALSE;
        indicateSiblingUpdate = GT_FALSE;

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);

        do
        {
            /* The deleted prefix ends in a deeper level,   */
            if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
            {
                /* check if the bucket was resized , thus moved in memory */
                if (updateUpperLevel == GT_TRUE)
                {
                    (*pRange)->updateRangeInHw = GT_TRUE;
                    doHwUpdate = GT_TRUE;

                    /* the upper level has been marked for update , remove
                       the signal */
                    updateUpperLevel = GT_FALSE;
                }

                /* Check the type of the next bucket after deletion */
                if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
                {
                    /* if it's type "route entry pointer" it means there is no
                       need for that pointed bucket (and it actually been
                       deleted in the lower level) so merge the range */
                    mergeRange(pRange,startAddr,currPrefix,*pPrevRange,
                               PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,&numOfNewRanges);

                    if (numOfNewRanges == 0)
                    {
                        /* An overwrite merge was done (no change in the amount
                           of ranges) extract the appropriate prefix from the
                           trie, and insert it in the same place.*/
                        rangePrefixLength =
                            mask2PrefixLength((*pRange)->mask,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                        nextPtr = getFromTrie((*currLpmPtr)->nextBucket,
                                              (*pRange)->startAddr,
                                              rangePrefixLength,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                        PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                    (*pRange)->pointerType);
                        (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;

                        doHwUpdate = GT_TRUE;
                    }
                    else
                    {
                        /* update the number of ranges */
                        (*currLpmPtr)->nextBucket->numOfRanges =
                            (GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges);

                        /* indicate a resize has happen */
                        resizeBucket = GT_TRUE;

                        /* a change happend to the structure of the bucket
                        (removed ranges) clear the cash, and signal the upper
                        level */
                        (*currLpmPtr)->nextBucket->rangeCash = NULL;

                        updateUpperLevel = GT_TRUE;
                    }
                }
            }
            else
            {
                /* The deleted prefix ends in this level.   */
                rangePrefixLength =
                    mask2PrefixLength((*pRange)->mask, PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                if (currPrefix > rangePrefixLength)
                    /* should never happen */
                    if (rollBack == GT_FALSE)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

                (*pRange)->mask &= ~(PREFIX_2_MASK_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS));

                if (currPrefix == rangePrefixLength)
                {
                    /* The current range represents the deleted prefix. */

                    if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* if the range's type "route entry pointer" it means
                           there is no need for this range, merge it */

                        mergeRange(pRange,startAddr,currPrefix,*pPrevRange,
                                   PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,&numOfNewRanges);

                        if (numOfNewRanges == 0)
                        {
                           /* An overwrite merge was done (no change in the
                              amount of ranges) extract the appropriate prefix
                              from the trie, and insert it in the same
                              place.*/
                            rangePrefixLength =
                                mask2PrefixLength((*pRange)->mask,
                                                  PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                            nextPtr =
                                getFromTrie((*currLpmPtr)->nextBucket,
                                            (*pRange)->startAddr,
                                            rangePrefixLength,
                                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                            PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                        (*pRange)->pointerType);
                            (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;

                            doHwUpdate = GT_TRUE;
                        }
                        else
                        {

                            /* update the amount of ranges */
                            (*currLpmPtr)->nextBucket->numOfRanges =
                                (GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges);

                            /* indicate a resize has happen */
                            resizeBucket = GT_TRUE;

                            /* a change happend to the structure of the bucket
                               (removed ranges) clear the cash, and signal the
                               upper level */
                            (*currLpmPtr)->nextBucket->rangeCash = NULL;

                            updateUpperLevel = GT_TRUE;
                        }
                    }
                    else
                    {
                        /* The current range was extended as a result of    */
                        /* previous insertions of larger prefixes.          */
                        newPrefixLength =
                            mask2PrefixLength((*pRange)->mask,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                        newStartAddr = (GT_U8)(startAddr &
                            (0xFF << (PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS - newPrefixLength)));

                        /* Get the prefix that should replace the   */
                        /* deleted prefix.                          */
                        newNextHopPtr = getFromTrie((*currLpmPtr)->nextBucket,
                                                    newStartAddr,
                                                    newPrefixLength,
                                                    PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                        /* in case of no HW update it's the same as in hot sync,
                           shadow update only */
                        insertMode = (updateHwAndMem == GT_TRUE)?
                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E:
                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;

                        /* Insert the replacing prefix to the next level. this
                           insertion cannot resize any lower level bucket , it
                           simply overwrites using the replacing prefix the
                           ranges the old prefix dominated. thus there is no
                           need to check if the lower levels have been resized
                           for a current range HW update */
                        updateOld = GT_FALSE;

                        PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(newNextHopPtr->routeEntryMethod,newNextHopPtrType);
                        retVal =
                            prvCpssDxChLpmRamMngInsert((*pRange)->lowerLpmPtr.nextBucket,
                                                       currAddrByte + 1,/* actually not relevant*/
                                                       0,newNextHopPtr,
                                                       newNextHopPtrType,
                                                       PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E,
                                                       lpmEngineMemPtrPtr + 1,
                                                       &updateOld,
                                                       &((*pRange)->pointerType),
                                                       shadowPtr,insertMode,
                                                       isMcSrcTree,
                                                       parentWriteFuncPtr,protocol);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }
                    }
                }
            }

            *pPrevRange = (*pRange);
            (*pRange) = (*pRange)->next;

        }while (((*pRange) != NULL) && (endAddr >= (*pRange)->startAddr));

        /* if the bucket need resizing always update the HW */
        if (resizeBucket == GT_TRUE)
            doHwUpdate = GT_TRUE;

        if ((currLpmPtr == lpmPtrArray) && (isMcSrcTree == GT_FALSE))
        {
            isDestTreeRootBucket = GT_TRUE;
        }
        else
        {
            isDestTreeRootBucket = GT_FALSE;
        }
        useCompressed2 = (isDestTreeRootBucket &&
                          (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E))?
                         GT_FALSE : GT_TRUE;

        if (isDestTreeRootBucket &&
            (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_RAM_UPDATE_E))
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E;
            indicateSiblingUpdate = (updateHwAndMem == GT_FALSE) ?
                                    GT_TRUE:GT_FALSE;
        }
        else if (doHwUpdate == GT_FALSE)
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_NONE_E;
            indicateSiblingUpdate =
            (updateHwAndMem == GT_FALSE)? GT_TRUE:GT_FALSE;
        }
        else if (updateHwAndMem == GT_FALSE)
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_SHADOW_ONLY_E;
        }
        else
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E;
        }

        /* Write the prepared shadow bucket to the device's RAM. */
        retVal = updateMirrorBucket((*currLpmPtr)->nextBucket,bucketUpdateMode,
                                    isDestTreeRootBucket,resizeBucket,GT_FALSE,
                                    indicateSiblingUpdate,useCompressed2,
                                    lpmEngineMemPtrPtr,*currBucketPtrType,
                                    shadowPtr,parentWriteFuncPtr);
        if (retVal != GT_OK)
        {
            return retVal;
        }

        /* update partition block */
        /* if hwBucketOffsetHandle==0 it means that the
           bucket was deleted in the updateMirrorBucket stage */
        if (((*currLpmPtr)->nextBucket)->hwBucketOffsetHandle!=0)
        {
            SET_DMM_BLOCK_PROTOCOL((*currLpmPtr)->nextBucket->hwBucketOffsetHandle, protocol);
            SET_DMM_BUCKET_SW_ADDRESS((*currLpmPtr)->nextBucket->hwBucketOffsetHandle, ((*currLpmPtr)->nextBucket));
        }


        /* If this level is not a root bucket then delete it if its empty.*/
        if (((currLpmPtr != lpmPtrArray) ||
             (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E)) &&
            ((**currBucketPtrType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
             (**currBucketPtrType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
             (**currBucketPtrType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E)))
        {
            newNextHopPtr =
                (*currLpmPtr)->nextBucket->rangeList->lowerLpmPtr.nextHopEntry;
            retVal = prvCpssDxChLpmRamMngBucketDelete((*currLpmPtr)->nextBucket,
                                                      PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, NULL);
            if (retVal != GT_OK)
            {
                return retVal;
            }
            (*currLpmPtr)->nextHopEntry = newNextHopPtr;
        }

        /* step back with the arrays */
        currLpmPtr--;
        pRange--;
        pPrevRange--;
        currBucketPtrType--;
        currAddrByte--;
        lpmEngineMemPtrPtr--;
        level--;
        currPrefix = BUCKET_PREFIX_AT_LEVEL_MAC(prefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,level);
    }

    return retVal;
}

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
)
{
    GT_U8                                       startAddr;

    GT_U32                                      currPrefix      = prefix;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *currBucketPtr  = bucketPtr;
    GT_U8                                       *currAddrByte   = addrCurBytePtr;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC      *pRange;

    if(bucketPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,NULL);

    while (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
    {
        pRange = find1stOverlap(currBucketPtr,startAddr,NULL);

        if((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
           (pRange->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
           (pRange->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
           ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            return /* do not log this error */ GT_NOT_FOUND;
        }
        /* search in the next level.    */
        currBucketPtr = pRange->lowerLpmPtr.nextBucket;
        currAddrByte += 1;
        currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,NULL);
    }

    *nextHopPtr = getFromTrie(currBucketPtr,startAddr,currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

    if(*nextHopPtr == NULL)
        return /* do not log this error */ GT_NOT_FOUND;
    else
        return GT_OK;
}

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
)
{
    GT_U8                                       startAddr;

    GT_U32                                      currPrefix      = prefix;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *currBucketPtr  = bucketPtr;
    GT_U8                                       *currAddrByte   = addrCurBytePtr;
    GT_PTR                                      *nextHopPtr;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC      *pRange;

    if(bucketPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,NULL);

    while (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
    {
        pRange = find1stOverlap(currBucketPtr,startAddr,NULL);

        if((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
           (pRange->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
           (pRange->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
           ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        /* search in the next level.    */
        currBucketPtr = pRange->lowerLpmPtr.nextBucket;
        currAddrByte += 1;
        currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,NULL);
    }

    nextHopPtr = getFromTrie(currBucketPtr,startAddr,currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
    *lastBucketPtr = currBucketPtr;

    if(nextHopPtr == NULL)
    {
        *lastHwBucketOffset = 0;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else
    {
        *lastHwBucketOffset = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(currBucketPtr->hwBucketOffsetHandle);
        return GT_OK;
    }
}

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
)
{
    GT_U8                                    startAddr;
    GT_U32                                   currPrefix;
    GT_U8                                    prefixLevel = 0;
    GT_U32                                   rangePrefixLength;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *currBucketPtr = bucketPtr;
    GT_U8                                    *currAddrByte = addrCurBytePtr;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;

    if(bucketPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    *nextHopPtr = NULL;
    switch (protocolStack)
    {
    case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
        currPrefix = 32;
        break;
    case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
        currPrefix = 128;
        break;
    case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
        currPrefix = 24;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    }

    do
    {
        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,NULL);

        pRange = find1stOverlap(currBucketPtr,startAddr,NULL);

        if ((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (pRange->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
            (pRange->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
        {
            *nextHopPtr = (GT_PTR*)pRange->lowerLpmPtr.nextHopEntry;
            rangePrefixLength = mask2PrefixLength(pRange->mask,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
            *prefixLenPtr = prefixLevel*PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS + rangePrefixLength;
            break;
        }

        /* search in the next level.    */
        currBucketPtr  = pRange->lowerLpmPtr.nextBucket;
        currAddrByte += 1;
        prefixLevel +=1;
        currPrefix   = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

    }while (currPrefix > 0);

    if (currPrefix == 0)
    {
        *nextHopPtr = (GT_PTR*)currBucketPtr->rangeList->lowerLpmPtr.nextHopEntry;
        *prefixLenPtr = 0;
    }

    if(*nextHopPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    else
        return GT_OK;

}

/**
* @internal getNextEntry function
* @endinternal
*
* @brief   This function is a recursive function that returns the first next hop
*         after the given (inAddr,Prefix).
* @param[in] bucketPtr                - A pointer to the current bucket to search in.
* @param[in,out] addrPtr                  - The Byte Array represnting the Address to search for
*                                      a consecutive for.
* @param[in,out] prefixPtr                - The inAddr prefix length.
* @param[in,out] addrPtr                  - The address search result.
* @param[in,out] prefixPtr                - the address prefix length.
*
* @param[out] nextPointerPtr           - A pointer to the data stored in the Trie, associated
*                                      with the found (address,prefix).
*                                       GT_OK if found, or
*                                       GT_NOT_FOUND on failure.
*/
static GT_STATUS getNextEntry
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC    *bucketPtr,
    INOUT GT_U8                                   *addrPtr,
    INOUT GT_U32                                  *prefixPtr,
    OUT GT_PTR                                    *nextPointerPtr
)
{
    GT_U32          prefixComp,*curLvlPrefix;
    GT_U8           tmpAddrArray[MAX_LPM_LEVELS_CNS];
    GT_U32          tmpLvlPrefixArray[MAX_LPM_LEVELS_CNS];
    GT_U8           *curAddr,*curInAddr;
    GT_32           tmpPrefix;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtrArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   **curBucketPtr;

    GT_U8  searchAddr[1]; /* inAddr in GT_U8 format */
    GT_U32  searchPrefix;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRangeArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   **pRange,*tmpRange;   /* Used for going over the current bucket's ranges. */
    GT_STATUS   retVal = GT_NOT_FOUND;
    GT_BOOL     moveUp = GT_FALSE;

    if(bucketPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    tmpPrefix = *prefixPtr;
    curAddr = tmpAddrArray;
    curInAddr = addrPtr;
    curLvlPrefix = tmpLvlPrefixArray;

    bucketPtrArray[0] = bucketPtr;
    curBucketPtr = bucketPtrArray;
    pRange = pRangeArray;
    while(tmpPrefix >= 0)
    {
        prefixComp = (tmpPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) ?
                          0 : (PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS - tmpPrefix);

        *curLvlPrefix = (tmpPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) ?
            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS : tmpPrefix;

        *curAddr = (GT_U8)(*curInAddr & (BIT_MASK_MAC(PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                                                   << prefixComp));

        if (tmpPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
        {
            *pRange = find1stOverlap(*curBucketPtr,*curAddr,NULL);

            if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                ((*pRange)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                ((*pRange)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
            {
                break;
            }

            curBucketPtr++;

            *curBucketPtr = (*pRange)->lowerLpmPtr.nextBucket;
            pRange++;

        }
        else
        {
            /* this is the level ,break */
            break;
        }

        curAddr++;
        curInAddr++;
        curLvlPrefix++;
        tmpPrefix -= PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS;
    }

    /* ok from here on we search for the next entry after the given found/not found
       one */

    while (curBucketPtr >= bucketPtrArray)
    {

        searchAddr[0] = *curAddr;
        searchPrefix = *curLvlPrefix;
        retVal = prvCpssDxChLpmTrieGetNext(&(*curBucketPtr)->trieRoot,
                                           PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                           GT_FALSE,searchAddr,&searchPrefix,nextPointerPtr);
        if (retVal == GT_FAIL)
        {
            /* this means the prefix we gave isn't in the trie, so to look for
               it we need to be more smart ,and work a bit */

            *nextPointerPtr = NULL;
            /* first we're gonna take the first node using the force == GT_TRUE*/
            retVal = prvCpssDxChLpmTrieGetNext(&(*curBucketPtr)->trieRoot,
                                               PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                               GT_TRUE,searchAddr,&searchPrefix,nextPointerPtr);
            /* now we're gonna advance till we have a bigger prefix */
            while (((searchAddr[0] < *curAddr) ||
                    ((searchAddr[0] == *curAddr) &&
                     (searchPrefix < *curLvlPrefix))) &&
                   (*nextPointerPtr != NULL))
            {
                *nextPointerPtr = NULL;
                retVal = prvCpssDxChLpmTrieGetNext(&(*curBucketPtr)->trieRoot,
                                                   PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                                   GT_TRUE,searchAddr,&searchPrefix,nextPointerPtr);
            }
            if (*nextPointerPtr == NULL)
            {
                /* this means we haven't found a next so the search ones should
                   be unchanged to indicate not found */
                searchAddr[0] = *curAddr;
                searchPrefix = *curLvlPrefix;
            }
        }
        if (moveUp == GT_FALSE)
        {

            /* ok we need to go down in the trie, see if the found next is down */
            /* find the next down accroding to the buckets */
            tmpRange = find1stOverlap(*curBucketPtr,*curAddr,NULL);
            while (tmpRange != NULL)
            {
                if ((tmpRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    (tmpRange->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                    (tmpRange->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                    (tmpRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                    ((GT_U32)(tmpRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                {
                    tmpRange = tmpRange->next;
                }
                else
                {
                    break;
                }
            }
            if ((tmpRange == NULL) || (searchAddr[0] <= tmpRange->startAddr))
            {
                if ((searchAddr[0] != (*curAddr)) ||
                    (searchPrefix != *curLvlPrefix))
                {
                    /* ok there is a next deeper one in this level ,record it and break */
                    *curAddr = searchAddr[0];
                    *curLvlPrefix = searchPrefix;
                    break;
                }
            }
        }
        else
        {
            /* ok this means we've been down and now we look for next up
               the found trie search could be the right one to go with unless
               there is down bucket between, lets check */
            tmpRange = (*pRange)->next;
            while (tmpRange != NULL)
            {
                if ((tmpRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    (tmpRange->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                    (tmpRange->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                    (tmpRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                    ((GT_U32)(tmpRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                {
                    tmpRange = tmpRange->next;
                }
                else
                {
                    break;
                }
            }

            if ((searchAddr[0] != (*curAddr)) ||
                (searchPrefix != *curLvlPrefix))
            {
                /* in this case the trie next is valid next */
                if ((tmpRange != NULL) &&
                    (tmpRange->startAddr < searchAddr[0]))
                {
                    /* the found range leads to a closer next , go down */
                    *curAddr = tmpRange->startAddr;
                    moveUp = GT_FALSE;
                }
                else
                {
                    /* ok the trie one is a closer next,record it and break */
                    *curAddr = searchAddr[0];
                    *curLvlPrefix = searchPrefix;
                    break;
                }
            }
            else
            {
                /*now only the bucket one could be the next , check if it's valid */
                if (tmpRange != NULL)
                {
                    *curAddr = tmpRange->startAddr;
                    moveUp = GT_FALSE;
                }
                /* the else part is that we continue to move up */
            }
        }

        if (moveUp == GT_FALSE)
        {
            /* not found try to move down */
            (*pRange) = find1stOverlap(*curBucketPtr,*curAddr,NULL);
            while ((*pRange) != NULL)
            {
                if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                    ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                {
                    (*pRange) = (*pRange)->next;
                }
                else
                {
                    break;
                }
            }
            if ((*pRange) == NULL)
            {
                moveUp = GT_TRUE;
            }
            else
            {
                *curAddr = (*pRange)->startAddr;
                *curLvlPrefix = PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS;

                curLvlPrefix++;
                curAddr++;
                curBucketPtr++;

                curInAddr++;

                *curBucketPtr = (*pRange)->lowerLpmPtr.nextBucket;

                pRange++;
                *curAddr = 0;
                *curLvlPrefix = 0;
            }
        }

        if (moveUp == GT_TRUE)
        {
            /* no next , move up */

            curLvlPrefix--;
            curAddr--;
            curBucketPtr--;
            pRange--;
            curInAddr--;
        }
    }

    /* if the array was breached that means we didn't found anything */
    if (curBucketPtr < bucketPtrArray)
    {
        return /* do not log this error */ GT_NOT_FOUND;
    }

    /* ok found , now build the address / prefix */
    *prefixPtr = 0;
    while (curInAddr >= addrPtr)
    {
        *curInAddr = *curAddr;
        *prefixPtr += *curLvlPrefix;

        curInAddr--;
        curAddr--;
        curLvlPrefix--;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS retVal = GT_OK;
    GT_U32 octets=0 ,bits=0 ,i=0, maxNumOfOctets=0;

    retVal = getNextEntry(bucketPtr,addrPtr,prefixPtr,nextPtr);
    if(retVal != GT_OK)
        return retVal;

    /* Reset unused octets and bits in addrPtr according to prefixPtr */

    /* octets --> number of full octets in addrPtr according to the prefix
       bits --> number of bits left in the addrPtr according to the prefix
       so if the prefix is 18 then we have 2 full octets data (octet 0 and 1 should not be masked)
       and 2 more bits of data (need to mask bits 0 to 5) taken from octet 3,
       octet 4 to maxNumOfOctets is all zero */

    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        /* in Ipv4 if the prefix is 32 there is nothing to reset */
        if (*prefixPtr == 32)
            return GT_OK;

        maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
    }
    else if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
    {
        /* in Ipv6 if the prefix is 128 there is nothing to reset */
        if (*prefixPtr == 128)
            return GT_OK;

        maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
    }
    else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E */
    {
        /* in FCoE if the prefix is 24 there is nothing to reset */
        if (*prefixPtr == 24)
            return GT_OK;

        maxNumOfOctets = 3;
    }

    octets = (*prefixPtr) / 8;
    bits = (*prefixPtr) % 8;

    addrPtr[octets] &= (BIT_MASK_MAC(bits) << (8 - bits));

    for (i = octets+1; i < maxNumOfOctets; i++)
    {
        addrPtr[i] = 0;
    }

    return GT_OK;
}

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
)
{
    GT_U32                                  octetIndex;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *headOfListToFreePtr; /* head of the list of memories we want to free */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *tempElemToFreePtr;   /* temp pointer used for free operation */
    GT_U32                                  maxNumOfOctets=0;

    switch (protocolStack)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* free the allocated/bound RAM memory */
    for (octetIndex=0; octetIndex<maxNumOfOctets; octetIndex++)
    {
        /* the first element in the list is the father of the first new allocation,
           we need to free all elements after the father */
        headOfListToFreePtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex];

        if (headOfListToFreePtr!=NULL)/* A new alloction was done for this octet */
        {
            if(headOfListToFreePtr->nextMemInfoPtr==NULL)/* this is an allocation of the root */
            {
                /* make sure this is the root - should always be true */
                if (headOfListToFreePtr==(&(shadowPtr->lpmMemInfoArray[protocolStack][octetIndex])))
                {
                    /* just reset the values */
                    headOfListToFreePtr->ramIndex=0;
                    headOfListToFreePtr->structsBase=0;
                    headOfListToFreePtr->structsMemPool=0;
                }
                else
                {
                    /* should never happen or it is a scenario we haven’t thought about */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                /* this is an allocation of a new element in the linked list
                   need to free the elemen and remove it from the linked list (update the father pointers) */
                while (headOfListToFreePtr->nextMemInfoPtr != NULL)
                {
                    /* in case of a merge we can have a case that a octet got a potential bank and
                       allocated form it prior to the merge, then the merge partially happened and
                       ended with an error. in case of an error the first allocation should be freed,
                       but the bank should get a permanent association with the octet due to the merge operation */
                    tempElemToFreePtr = headOfListToFreePtr->nextMemInfoPtr;

                    if (((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)&&
                            (shadowPtr->protocolCountersPerBlockArr[tempElemToFreePtr->ramIndex].sumOfIpv4Counters==0))||
                        ((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)&&
                            (shadowPtr->protocolCountersPerBlockArr[tempElemToFreePtr->ramIndex].sumOfIpv6Counters==0))||
                        ((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)&&
                            (shadowPtr->protocolCountersPerBlockArr[tempElemToFreePtr->ramIndex].sumOfFcoeCounters==0)))
                    {
                        headOfListToFreePtr->nextMemInfoPtr = headOfListToFreePtr->nextMemInfoPtr->nextMemInfoPtr;
                        cpssOsFree(tempElemToFreePtr);
                    }
                    else
                    {
                        /* the memory was passed by merge and we need to permanent update the assosiation to the bank
                           in the tempLpmRamOctetsToBlockMappingUsedForReconstractPtr used in case of a fail */
                         cpssOsMemCpy(&(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr[tempElemToFreePtr->ramIndex]),
                                      &(shadowPtr->lpmRamOctetsToBlockMappingPtr[tempElemToFreePtr->ramIndex]),
                                      sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC));

                         /* Set next element as head (will not be freed) */
                         headOfListToFreePtr = headOfListToFreePtr->nextMemInfoPtr;
                    }
                }
            }
        }
    }

    cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

    return GT_OK;
}

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
)
{
    GT_U32      neededMemoryBlocksSizes[MAX_LPM_LEVELS_CNS];  /* Holds memory allocation  */
    GT_UINTPTR  memoryPoolList[MAX_LPM_LEVELS_CNS];           /* needs.                   */
    GT_U32      neededMemoryBlocksOctetsIndexs[MAX_LPM_LEVELS_CNS];
    GT_UINTPTR  *neededMemoryBlocks;
    GT_U32      sizeOfOptionalUsedAndFreeBlock;
    GT_U32      i;
    GT_STATUS   retVal = GT_OK,retVal2=GT_OK;             /* Function return value.   */
    GT_U32      startIdx;
    GT_U32      *neededMemoryBlocksSizesPtr;
    GT_U32      *neededMemoryBlocksOctetsIndexsPtr;
    GT_UINTPTR  *memoryPoolListPtr;
    GT_U32      newFreeBlockIndex; /* index of a new free block */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoForSwapUsePtr;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *potentialNewBlockPtr=NULL;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                        the allocted new element that need to be freed.
                                                                                        Size of the array is 16 for case of IPV6 */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *firstMemInfoInListToFreePtr=NULL;/* use for going over the list of blocks per octet */

    GT_U32      octetIndex; /* the octet we are working on */
    GT_U32      blockStart=0; /* used as a start point for finding a free block */
    GT_BOOL     justBindAndDontAllocateFreePoolMem = GT_FALSE;
    GT_U32      blockIndex=0xFFFFFFFF; /* calculated according to the memory offset devided by block size including gap */

    /* parameters used for defrag */
    GT_U32      oldBlockIndex       = 0xFFFFFFFE; /* calculated according to the memory offset devided by block size including gap */
    GT_BOOL     *swapInUseForAdd;
    GT_UINTPTR  neededMemoryBlocksOldHandleAddr[MAX_LPM_LEVELS_CNS];  /* Holds handle address of old bucket
                                                                         in case of resize needed, for swap memory use */
    GT_BOOL     neededMemoryBlocksSwapCanBeUseful[MAX_LPM_LEVELS_CNS];/* in case of resize needed this indicate if swap
                                                                         memory can be useful to minimize memory usage */
    GT_UINTPTR  *neededMemoryBlocksOldHandleAddrPtr;
    GT_BOOL     *neededMemoryBlocksSwapCanBeUsefulPtr;

    GT_BOOL     freeBlockCanBeFound;
    GT_U32      bankIndexForShrink;
    GT_BOOL     useSwapArea;

    GT_BOOL     mergeBankCanBeFound;
    GT_U32      bankIndexForMerge;
    GT_U32      octetIndexForMerge;

    neededMemoryBlocks = shadowPtr->neededMemoryBlocks;
    swapInUseForAdd    = shadowPtr->neededMemoryBlocksSwapUsedForAdd;

    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    /* remember the current list len, to go over only the added ones*/
    startIdx = shadowPtr->neededMemoryListLen;

    neededMemoryBlocksSizesPtr = &neededMemoryBlocksSizes[MAX_LPM_LEVELS_CNS-1];
    neededMemoryBlocksOctetsIndexsPtr = &neededMemoryBlocksOctetsIndexs[MAX_LPM_LEVELS_CNS-1];
    memoryPoolListPtr = &memoryPoolList[MAX_LPM_LEVELS_CNS-1];
    neededMemoryBlocksOldHandleAddrPtr = &neededMemoryBlocksOldHandleAddr[MAX_LPM_LEVELS_CNS-1];
    neededMemoryBlocksSwapCanBeUsefulPtr = &neededMemoryBlocksSwapCanBeUseful[MAX_LPM_LEVELS_CNS-1];

    /* reset swapInUseForAdd before new allocations */
    cpssOsMemSet(swapInUseForAdd+startIdx, 0, (sizeof(GT_U32) * (shadowPtr->neededMemoryListLen-startIdx)));
    /* Get needed memory for LPM search insertion.  */
    retVal = lpmCalcNeededMemory(bucketPtr,destArr,prefix,
                                 rootBucketFlag,lpmEngineMemPtrPtr,insertMode,defragmentationEnable,
                                 &neededMemoryBlocksSizesPtr,
                                 &memoryPoolListPtr,
                                 &shadowPtr->neededMemoryListLen,
                                 &neededMemoryBlocksOctetsIndexsPtr,
                                 &neededMemoryBlocksOldHandleAddrPtr,
                                 &neededMemoryBlocksSwapCanBeUsefulPtr);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    /* keep values in case reconstruct is needed */
    cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

    /* reset neededMemoryBlocks before new allocations */
    cpssOsMemSet(neededMemoryBlocks+startIdx, (int)DMM_BLOCK_NOT_FOUND, (sizeof(GT_UINTPTR) * (shadowPtr->neededMemoryListLen-startIdx)));

    /* Try to allocate all needed memory.               */
    for(i = startIdx; i < shadowPtr->neededMemoryListLen; i++)
    {
        /* swap area that is used for add/delete prefixes is in use */
        swapInUseForAdd[i]=GT_FALSE;

        memoryPoolListPtr++;
        neededMemoryBlocksSizesPtr++;
        neededMemoryBlocksOctetsIndexsPtr++;
        neededMemoryBlocksOldHandleAddrPtr++;
        neededMemoryBlocksSwapCanBeUsefulPtr++;
        octetIndex = (*neededMemoryBlocksOctetsIndexsPtr);

        /* need to update the memoryPoolList - maybe it was changed by merge operation done in previous octet treatment */
        *memoryPoolListPtr = shadowPtr->lpmMemInfoArray[protocol][octetIndex].structsMemPool;

        if (*memoryPoolListPtr!=0)
        {
            neededMemoryBlocks[i] =
            prvCpssDmmAllocate(*memoryPoolListPtr,
                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * (*neededMemoryBlocksSizesPtr),
                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
        }
        else
        {
            /* no memory pool is bound to the root of the octet list,
               in this case no need to allocate a memory pool struct just
               to bind it to a free pool */
            justBindAndDontAllocateFreePoolMem = GT_TRUE;

        }
        shadowPtr->neededMemoryBlocksSizes[i] = *neededMemoryBlocksSizesPtr;
        /* if the neededMemoryBlocks is 0xFFFFFFFF --> then we did not succeed in allocating the needed memory  */
        if(neededMemoryBlocks[i] == DMM_BLOCK_NOT_FOUND)
        {
            /* check if the octet is bound to more blocks,
               if yes then try to allocte the memory again */

             /* shadowPtr->neededMemoryListLen is the number of blocks
                (just the first in the list of each octet) in neededMemoryBlocks
                --> meaninng number of octets we want to add in the currect prefix */

            /* the search is from the last octet to the first one since
               memoryPoolListPtr was initialized backwards in lpmCalcNeededMemory */
            tempNextMemInfoPtr = lpmEngineMemPtrPtr[octetIndex];

            while((tempNextMemInfoPtr->nextMemInfoPtr!= NULL)&&(neededMemoryBlocks[i]==DMM_BLOCK_NOT_FOUND))
            {
                neededMemoryBlocks[i] =
                            prvCpssDmmAllocate(tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool,
                                   DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * (*neededMemoryBlocksSizesPtr),
                                   DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);

                tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
            }

            /* could not find an empty space in the current bound blocks - continue looking */
            if ((tempNextMemInfoPtr->nextMemInfoPtr==NULL)&&(neededMemoryBlocks[i]==DMM_BLOCK_NOT_FOUND))
            {
                /* first we try to allocate the space using the swap area */
                if ((defragmentationEnable==GT_TRUE)&&(*neededMemoryBlocksSwapCanBeUsefulPtr==GT_TRUE))
                {
                    oldBlockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(*neededMemoryBlocksOldHandleAddrPtr)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    tempNextMemInfoForSwapUsePtr = lpmEngineMemPtrPtr[octetIndex];
                    /* go over again on the list of blocks bound to the octet and look
                       for the block with a potential to swap are use */
                    while((tempNextMemInfoForSwapUsePtr!= NULL)&&(neededMemoryBlocks[i]==DMM_BLOCK_NOT_FOUND))
                    {
                        /* if the current memory block is the memory block of the oldHwBucketOffsetHandle
                           we can achieve free space by using the swap area.*/
                        if (tempNextMemInfoForSwapUsePtr->ramIndex==oldBlockIndex)
                        {
                            /* in this stage we must check again that if we use
                            the swap area a reuse of the current memory can be done.
                            in some cases (in shared mode) we can have the memory near the used one
                            already been taken by another octet in case of sharing mode */
                            if(shadowPtr->lpmRamBlocksAllocationMethod==PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E)
                            {
                                sizeOfOptionalUsedAndFreeBlock =
                                   prvCpssDmmCheckResizeAvailableWithSameMemory(*neededMemoryBlocksOldHandleAddrPtr,
                                                                                *neededMemoryBlocksSizesPtr);
                                if (sizeOfOptionalUsedAndFreeBlock>0)
                                {
                                   neededMemoryBlocks[i] = shadowPtr->swapMemoryAddr;
                                   swapInUseForAdd[i] = GT_TRUE;
                                   break;
                                }
                            }
                            else
                            {
                                 neededMemoryBlocks[i] = shadowPtr->swapMemoryAddr;
                                 swapInUseForAdd[i] = GT_TRUE;
                                 break;
                            }
                        }

                        tempNextMemInfoForSwapUsePtr = tempNextMemInfoForSwapUsePtr->nextMemInfoPtr;
                    }
                }

                /* if using swap area do not help then find an available new free block that
                   could be bound to the octet */

                /* go over all blocks until the memory of the octet can be allocated in the block found */
                if (neededMemoryBlocks[i]==DMM_BLOCK_NOT_FOUND)
                {
                    while (blockStart < shadowPtr->numOfLpmMemories)
                    {
                        /* find a new free block and bind it to the octet and protocol */
                        retVal = prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol(shadowPtr,
                                                                                 protocol,
                                                                                 octetIndex,
                                                                                 blockStart,
                                                                                 &newFreeBlockIndex);


                        if (retVal != GT_OK)
                        {
                            break;
                        }

                        /* allocate the memory needed from the new structsMemPool bound */
                         neededMemoryBlocks[i] =
                            prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex],
                                   DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * (*neededMemoryBlocksSizesPtr),
                                   DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);

                        /* neededMemoryBlocks is not 0 nor 0xFFFFFFFF --> we did succeed in allocating the
                            needed memory from the valid block we found  */
                        if ((neededMemoryBlocks[i] != DMM_BLOCK_NOT_FOUND)&&(neededMemoryBlocks[i] != DMM_MALLOC_FAIL))
                        {
                            if(justBindAndDontAllocateFreePoolMem==GT_TRUE)
                            {
                                tempNextMemInfoPtr->ramIndex = newFreeBlockIndex;
                                tempNextMemInfoPtr->structsBase=0;
                                /* bind the new block */
                                tempNextMemInfoPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex];
                                tempNextMemInfoPtr->nextMemInfoPtr=NULL;
                                /* first element in linked list of the blocks that
                                   need to be freed in case of an error is the root */
                                firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                            }
                            else
                            {
                                /* allocate a new elemenet block to the list */
                                potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                                if (potentialNewBlockPtr == NULL)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                                }
                                potentialNewBlockPtr->ramIndex = newFreeBlockIndex;
                                potentialNewBlockPtr->structsBase = 0;
                                potentialNewBlockPtr->structsMemPool = 0;
                                potentialNewBlockPtr->nextMemInfoPtr = NULL;

                                /* bind the new block */
                                potentialNewBlockPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex];
                                tempNextMemInfoPtr->nextMemInfoPtr = potentialNewBlockPtr;

                                /* first element in linked list of the blocks that
                                   need to be freed in case of an error is the father
                                   of the new block added to the list
                                  (father of potentialNewBlockPtr is tempNextMemInfoPtr) */
                                firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                            }

                            /* mark the block as used */
                            shadowPtr->lpmRamOctetsToBlockMappingPtr[newFreeBlockIndex].isBlockUsed=GT_TRUE;
                            /*  set the block to be used by the specific octet and protocol*/
                            PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,protocol,octetIndex,newFreeBlockIndex);

                            /*  set pending flag for future need */
                            shadowPtr->pendingBlockToUpdateArr[newFreeBlockIndex].updateInc=GT_TRUE;
                            shadowPtr->pendingBlockToUpdateArr[newFreeBlockIndex].numOfIncUpdates +=
                                PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocks[i]);

                            /* We only save the first element allocated or bound per octet */
                            if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]==NULL)||
                                (allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]->structsMemPool==0))
                            {
                                /* keep the head of the list we need to free in case of an error -
                                first element is the father of the first element that should be freed */
                                allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex] = firstMemInfoInListToFreePtr;
                            }

                            break;
                        }
                        /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the
                            needed memory from the valid block we found, look for another valid block or
                            return an error incase of CPU memory allocation fail  */
                        else
                        {
                            if (neededMemoryBlocks[i] != DMM_MALLOC_FAIL)
                            {
                                blockStart = newFreeBlockIndex + 1;
                            }
                            else
                            {
                                retVal = GT_OUT_OF_CPU_MEM;
                                break;
                            }
                        }
                    }
                    if (neededMemoryBlocks[i]==DMM_BLOCK_NOT_FOUND)
                    {
                        if((retVal != GT_OK)&&
                           (shadowPtr->lpmRamBlocksAllocationMethod==PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E))
                        {
                            /* in sharing mode we do not support the next step - shrink*/
                            break;
                        }
                    }
                }

                /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the needed memory  */
                if ((defragmentationEnable==GT_TRUE)&&
                    ((neededMemoryBlocks[i] == DMM_BLOCK_NOT_FOUND)||(neededMemoryBlocks[i] == DMM_MALLOC_FAIL))&&
                    (shadowPtr->lpmRamBlocksAllocationMethod!=PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E))
                {
                    /* At the moment we do not support shrink in sharing mode  TBD .

                    check if shrink may help --> if YES then do shrink and try to allocate again */
                    retVal = prvCpssDxChLpmRamMngCheckIfShrinkOperationUsefulForDefrag(shadowPtr,
                                                                                      lpmEngineMemPtrPtr[octetIndex],
                                                                                      protocol,
                                                                                      *neededMemoryBlocksOldHandleAddrPtr,
                                                                                      *neededMemoryBlocksSizesPtr,
                                                                                      &freeBlockCanBeFound,
                                                                                      &bankIndexForShrink,
                                                                                      &useSwapArea);

                    if (retVal!=GT_OK)
                    {
                        break;
                    }

                    if (freeBlockCanBeFound == GT_TRUE)
                    {
                       /* shrink the Bank we found in previous phase */
                        retVal = prvCpssDxChLpmRamMngShrinkBank(shadowPtr,
                                                                lpmEngineMemPtrPtr,
                                                                octetIndex,
                                                                bankIndexForShrink,
                                                                *neededMemoryBlocksOldHandleAddrPtr,
                                                                *neededMemoryBlocksSizesPtr,
                                                                useSwapArea,
                                                                parentWriteFuncPtr);
                        if (retVal!=GT_OK)
                        {
                            break;
                        }

                        if (useSwapArea==GT_TRUE)
                        {
                             /* in case we used the swap_area_1 for the shrink operation
                               need to update pointers and free memory from the swap area
                               need to pass this information to the update mirror bucket */
                            neededMemoryBlocks[i] = shadowPtr->swapMemoryAddr;
                            swapInUseForAdd[i] = GT_TRUE;
                        }
                        else
                        {
                            /* try to allocate again the memory needed - operation should pass */
                            neededMemoryBlocks[i] =
                                        prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForShrink],
                                            DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * (*neededMemoryBlocksSizesPtr),
                                            DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
                        }

                        /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the needed memory  */
                        if ((neededMemoryBlocks[i] == DMM_BLOCK_NOT_FOUND)||(neededMemoryBlocks[i] == DMM_MALLOC_FAIL))
                        {
                            /* should not happen since we just shrinked the memory */
                            retVal = GT_OUT_OF_CPU_MEM;
                            break;
                        }
                        else
                        {
                            /* if we are using swap area then no allocation was done */
                            if (swapInUseForAdd[i] == GT_FALSE)
                            {
                                /* need to check if the block we shrink is an unbounded block,
                                   in this case we need to bound the block to the list */
                                if(PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr,protocol,octetIndex,bankIndexForShrink)==GT_FALSE)
                                {
                                    if(justBindAndDontAllocateFreePoolMem==GT_TRUE)
                                    {
                                        tempNextMemInfoPtr->ramIndex = bankIndexForShrink;
                                        tempNextMemInfoPtr->structsBase=0;
                                        /* bind the new block */
                                        tempNextMemInfoPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForShrink];
                                        tempNextMemInfoPtr->nextMemInfoPtr=NULL;
                                        /* first element in linked list of the blocks that
                                           need to be freed in case of an error is the root */
                                        firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                                    }
                                    else
                                    {
                                        /* allocate a new elemenet block to the list */
                                        potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                                        if (potentialNewBlockPtr == NULL)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                                        }
                                        potentialNewBlockPtr->ramIndex = bankIndexForShrink;
                                        potentialNewBlockPtr->structsBase = 0;
                                        potentialNewBlockPtr->structsMemPool = 0;
                                        potentialNewBlockPtr->nextMemInfoPtr = NULL;

                                        /* bind the new block */
                                        potentialNewBlockPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForShrink];
                                        tempNextMemInfoPtr->nextMemInfoPtr = potentialNewBlockPtr;

                                        /* first element in linked list of the blocks that
                                           need to be freed in case of an error is the father
                                           of the new block added to the list
                                          (father of potentialNewBlockPtr is tempNextMemInfoPtr) */
                                        firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                                    }

                                    /* mark the block as used */
                                    shadowPtr->lpmRamOctetsToBlockMappingPtr[bankIndexForShrink].isBlockUsed=GT_TRUE;
                                    /*  set the block to be used by the specific octet and protocol*/
                                    PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,protocol,octetIndex,bankIndexForShrink);

                                    /* We only save the first element allocated or bound per octet */
                                    if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]==NULL)||
                                        (allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]->structsMemPool==0))
                                    {
                                        /* keep the head of the list we need to free in case of an error -
                                        first element is the father of the first element that should be freed */
                                        allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex] = firstMemInfoInListToFreePtr;
                                    }
                                }

                                /* set pending flag for future need */
                                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocks[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                                    PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocks[i]);
                            }
                        }
                    }
                    else
                    {
                        /* check if merge between banks may help  */
                        retVal = prvCpssDxChLpmRamMngCheckIfMergeBankOperationUsefulForDefrag(shadowPtr,
                                                                      lpmEngineMemPtrPtr,
                                                                      protocol,
                                                                      octetIndex,
                                                                      *neededMemoryBlocksSizesPtr,
                                                                      &mergeBankCanBeFound,
                                                                      &bankIndexForMerge,
                                                                      &octetIndexForMerge);

                        if (retVal!=GT_OK)
                        {
                            break;
                        }
                        if (mergeBankCanBeFound == GT_TRUE)
                        {
                            /* Merge the Bank we found in previous phase */
                            retVal = prvCpssDxChLpmRamMngMergeBank(shadowPtr,
                                                                   lpmEngineMemPtrPtr,
                                                                   protocol,
                                                                   octetIndexForMerge,
                                                                   bankIndexForMerge,
                                                                   parentWriteFuncPtr);
                            if (retVal!=GT_OK)
                            {
                                break;
                            }
                            else
                            {
                                /* if we finish merged and with success it means we have space for new bucket.
                                   Next stage is to shrink the bank just released if needed,
                                   relocate the new / resized bucket & release its current location */

                                /* try to allocate again the memory needed - operation should pass */
                                neededMemoryBlocks[i] =
                                            prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge],
                                                DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * (*neededMemoryBlocksSizesPtr),
                                                DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);

                                /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the needed memory
                                   shrink the bank - operation must help since we just merge this bank to other banks and freed necessary
                                   space needed */
                                if ((neededMemoryBlocks[i] == DMM_BLOCK_NOT_FOUND)||(neededMemoryBlocks[i] == DMM_MALLOC_FAIL))
                                {
                                    /* shrink the Bank we found in previous phase */
                                    retVal = prvCpssDxChLpmRamMngShrinkBank(shadowPtr,
                                                                            lpmEngineMemPtrPtr,
                                                                            octetIndexForMerge,
                                                                            bankIndexForMerge,
                                                                            0, /* we need to do shrink for a new block, so no old pointers to update
                                                                                  and no old memory location to be moved */
                                                                            *neededMemoryBlocksSizesPtr,
                                                                            GT_FALSE,
                                                                            parentWriteFuncPtr);
                                    if (retVal!=GT_OK)
                                    {
                                        break;
                                    }
                                    /* try to allocate again the memory needed - operation should pass */
                                    neededMemoryBlocks[i] =
                                                prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge],
                                                    DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * (*neededMemoryBlocksSizesPtr),
                                                    DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);

                                    /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the needed memory  */
                                    if ((neededMemoryBlocks[i] == DMM_BLOCK_NOT_FOUND)||(neededMemoryBlocks[i] == DMM_MALLOC_FAIL))
                                    {
                                        /* should not happen since we just shrinked the memory */
                                        retVal = GT_OUT_OF_CPU_MEM;
                                        break;
                                    }
                                }

                                /* neededMemoryBlocks is not 0 nor 0xFFFFFFFF --> we did succeed in allocating the
                                    needed memory from the valid block we found  */
                                if ((neededMemoryBlocks[i] != DMM_BLOCK_NOT_FOUND)&&(neededMemoryBlocks[i] != DMM_MALLOC_FAIL))
                                {
                                    if(justBindAndDontAllocateFreePoolMem==GT_TRUE)
                                    {
                                        tempNextMemInfoPtr->ramIndex = bankIndexForMerge;
                                        tempNextMemInfoPtr->structsBase=0;
                                        /* bind the new block */
                                        tempNextMemInfoPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge];
                                        tempNextMemInfoPtr->nextMemInfoPtr=NULL;
                                        /* first element in linked list of the blocks that
                                           need to be freed in case of an error is the root */
                                        firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                                    }
                                    else
                                    {
                                        /* allocate a new elemenet block to the list */
                                        potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                                        if (potentialNewBlockPtr == NULL)
                                        {
                                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                                        }
                                        potentialNewBlockPtr->ramIndex = bankIndexForMerge;
                                        potentialNewBlockPtr->structsBase = 0;
                                        potentialNewBlockPtr->structsMemPool = 0;
                                        potentialNewBlockPtr->nextMemInfoPtr = NULL;

                                        /* bind the new block */
                                        potentialNewBlockPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge];
                                        tempNextMemInfoPtr->nextMemInfoPtr = potentialNewBlockPtr;

                                        /* first element in linked list of the blocks that
                                           need to be freed in case of an error is the father
                                           of the new block added to the list
                                          (father of potentialNewBlockPtr is tempNextMemInfoPtr) */
                                        firstMemInfoInListToFreePtr = tempNextMemInfoPtr;
                                    }

                                    /* mark the block as used */
                                    shadowPtr->lpmRamOctetsToBlockMappingPtr[bankIndexForMerge].isBlockUsed=GT_TRUE;
                                    /*  set the block to be used by the specific octet and protocol*/
                                    PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,protocol,octetIndex,bankIndexForMerge);

                                    /*  set pending flag for future need */
                                    shadowPtr->pendingBlockToUpdateArr[bankIndexForMerge].updateInc=GT_TRUE;
                                    shadowPtr->pendingBlockToUpdateArr[bankIndexForMerge].numOfIncUpdates +=
                                        PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocks[i]);

                                    /* We only save the first element allocated or bound per octet */
                                    if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]==NULL)||
                                        (allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]->structsMemPool==0))
                                    {
                                        /* keep the head of the list we need to free in case of an error -
                                        first element is the father of the first element that should be freed */
                                        allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex] = firstMemInfoInListToFreePtr;
                                    }
                                }
                                else
                                {
                                    /* should not happen since we just merged the memory */
                                    retVal = GT_OUT_OF_CPU_MEM;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            retVal = GT_OUT_OF_PP_MEM;
                            break;
                        }
                    }
                }
                /* neededMemoryBlocks is not 0 nor 0xFFFFFFFF --> then we did succeed in allocating the needed memory
                   --> save the allocated pool Id */
                else
                {
                    if ((neededMemoryBlocks[i] == DMM_BLOCK_NOT_FOUND)||(neededMemoryBlocks[i] == DMM_MALLOC_FAIL))
                    {
                       /* this case can happen when we did not find an empty new block to associate with
                          the octet and we do not do shrink - in sharing mode */
                        retVal = GT_OUT_OF_PP_MEM;
                        break;
                    }
                    else
                    {
                        /* We only save the first element allocated or bound per octet */
                        if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]==NULL)||
                            (allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]->structsMemPool==0))
                        {
                            /* keep the head of the list we need to free incase of an error -
                            first element is the father of the first element that should be freed */
                            allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex] = firstMemInfoInListToFreePtr;
                        }
                    }
                }
            }
            else
            {
                /* we had a CPU Memory allocation error */
                if(neededMemoryBlocks[i] == DMM_MALLOC_FAIL)
                {
                    retVal = GT_OUT_OF_CPU_MEM;
                    break;
                }
                /* if we are using swap area then no allocation was done */
                if (swapInUseForAdd[i] == GT_FALSE)
                {
                    /* mark the block as used */
                    /* set pending flag for future need */
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocks[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocks[i]);
                }
            }
        }
        else
        {
            /* we had a CPU Memory allocation error */
            if(neededMemoryBlocks[i] == DMM_MALLOC_FAIL)
            {
                retVal = GT_OUT_OF_CPU_MEM;
                break;
            }
            else
            {
                /* if we are using swap area then no allocation was done */
                if (swapInUseForAdd[i] == GT_FALSE)
                {
                    /*We succeed in allocating the memory*/
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocks[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    /* set pending flag for future need */
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;

                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocks[i]);
                }
            }
        }

        justBindAndDontAllocateFreePoolMem = GT_FALSE;
    }

    if(i != shadowPtr->neededMemoryListLen)
    {
        /* if we get to this point due to CPU error then leave the
           retVal we got in previous stage else return error due to PP */
        if (retVal!=GT_OUT_OF_CPU_MEM)
        {
            retVal = GT_OUT_OF_PP_MEM;
        }

        /* Allocation failed, free all allocated memory. */
        while(i > 0)
        {
            if (swapInUseForAdd[i-1]==GT_FALSE)
            {
                /*We fail in allocating the memory*/
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocks[i-1])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                /* set pending flag for future need */
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;

                prvCpssDmmFree(neededMemoryBlocks[i-1]);
                i--;
            }
            else
            {
                /* if we are using swap area then no allocation was done */
                neededMemoryBlocks[i-1]=0;
                i--;
            }
        }

        shadowPtr->neededMemoryListLen = 0;

        /* free the allocated/bound RAM memory */
        retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
        if (retVal2!=GT_OK)
        {
            return retVal2;
        }
        /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
        cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

        return retVal;
    }
    return retVal;
}

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
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       **currRange,*rangeMem[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC **currLpmEnginePtr;
    GT_BOOL forceWriteWholeBucket;
    LPM_BUCKET_UPDATE_MODE_ENT bucketUpdateMode;
    GT_BOOL isDestTreeRootBucket, useCompressed2;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC parentWriteFuncInfo;

    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC writeFuncData;

    /* update the info for the parent write function */
    parentWriteFuncInfo.bucketParentWriteFunc =
        prvCpssDxChLpmRamUpdateVrTableFuncWrapper;

    writeFuncData.shadowPtr = shadowPtr;
    writeFuncData.protocol = protocolStack;
    writeFuncData.vrId = vrId;

    parentWriteFuncInfo.data = (GT_PTR)(&writeFuncData);

    /* intialize the range memory */
    rangeMem[0] = rootRangePtr;

    /* start with the first level */
    currLpmEnginePtr = lpmEngineMemPtrPtr;
    currRange = rangeMem;

    forceWriteWholeBucket =
        (updateType == PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E)?
        GT_TRUE : GT_FALSE;

    bucketUpdateMode =
        (updateType == PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E)?
        LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E:
        LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E;

    while(currRange >= rangeMem)
    {
        if ((*currRange) == NULL)
        {
            /* this means that we finished with this level - move back up a level*/
            currRange--;
            currLpmEnginePtr--;
            if (currRange >= rangeMem)
            {
                if (((updateType == PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E) ||
                     /* in update mode, write only if this bucket needs update */
                     ((updateType != PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E) &&
                      (((*currRange)->lowerLpmPtr.nextBucket->bucketHwUpdateStat
                        != PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E) ||
                       ((*currRange)->lowerLpmPtr.nextBucket->bucketHwUpdateStat
                        != PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_UPDATE_SIBLINGS_E)))))
                {
                    isDestTreeRootBucket = (currLpmEnginePtr == lpmEngineMemPtrPtr) ? GT_TRUE : GT_FALSE;
                    useCompressed2 = (isDestTreeRootBucket == GT_TRUE) ? GT_FALSE : GT_TRUE;
                    /* now that we finished the lower levels , write this bucket */
                    retVal =
                        updateMirrorBucket((*currRange)->lowerLpmPtr.nextBucket,
                                           bucketUpdateMode,isDestTreeRootBucket,GT_FALSE,
                                           forceWriteWholeBucket,GT_FALSE,
                                           useCompressed2,currLpmEnginePtr,
                                           &(*currRange)->pointerType,
                                           shadowPtr,&parentWriteFuncInfo);
                    if (retVal != GT_OK)
                    {
                        return retVal;
                    }
                    else
                    {
                        /* update partition block */
                        if ((*currRange)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle!=0)
                        {
                            SET_DMM_BLOCK_PROTOCOL((*currRange)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle, protocolStack);
                            SET_DMM_BUCKET_SW_ADDRESS((*currRange)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle, (*currRange)->lowerLpmPtr.nextBucket);
                        }
                    }
                }
                else if ((updateType != PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E) ||
                         (currRange == rangeMem))
                {
                    /* it could be that it's BUCKET_HW_UPDATE_SIBLINGS we need
                       to reset it */
                    (*currRange)->lowerLpmPtr.nextBucket->bucketHwUpdateStat
                        = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E;
                }

                (*currRange) = (*currRange)->next;
            }
        }
        else if (((*currRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                 ((*currRange)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                 ((*currRange)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                 /* if we're in "update only" mode then check if this next bucket
                    or it's siblings need update */
                 ((updateType != PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E) &&
                  ((*currRange)->lowerLpmPtr.nextBucket->bucketHwUpdateStat ==
                   PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E)))
        {
            (*currRange) = (*currRange)->next;
        }
        else
        {
            /* this means this range has lower levels. go and explore */
            currRange[1] = (*currRange)->lowerLpmPtr.nextBucket->rangeList;

            currRange++;
            currLpmEnginePtr++;
        }
    }

    return retVal;
}

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
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_TREE_ITERATOR_STC *iterPtr;
    GT_U32 bucketSize;
    GT_UINTPTR hwBucketOffsetHandle = 0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */

    if ((memAllocArrayIndexPtr == NULL) ||
        (rootRangePtr == NULL) ||
        (stopPointIterPtr == NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (((traverseOp != PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_E) &&
         (traverseOp != PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_WITH_PCL_E)) &&
        (memAllocArrayPtr == NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* first allocate memory for the iterator if needed */
    if (*stopPointIterPtr == 0)
    {
        iterPtr = (PRV_CPSS_DXCH_LPM_RAM_TREE_ITERATOR_STC*)cpssOsLpmMalloc(sizeof (PRV_CPSS_DXCH_LPM_RAM_TREE_ITERATOR_STC));
        *stopPointIterPtr = (GT_UINTPTR)iterPtr;

        iterPtr->currRangePtr = iterPtr->rangePtrArray;
        iterPtr->currLpmEnginePtr = lpmEngineMemPtrPtr;

        /* intialize the range memory */
        iterPtr->rangePtrArray[0] = rootRangePtr;
    }
    else
    {
        iterPtr = (PRV_CPSS_DXCH_LPM_RAM_TREE_ITERATOR_STC*)*stopPointIterPtr;
    }

    while(iterPtr->currRangePtr >= iterPtr->rangePtrArray)
    {
        if ((*iterPtr->currRangePtr) == NULL)
        {
            /* this means that we finished with this level - move back up a level*/
            iterPtr->currRangePtr--;
            iterPtr->currLpmEnginePtr--;
            if (iterPtr->currRangePtr >= iterPtr->rangePtrArray)
            {
                /* first address the root bucket issue (set/record it or not )*/
                if ((iterPtr->currRangePtr != iterPtr->rangePtrArray) ||
                    (setRecoredRootBucketMem == GT_TRUE))
                {
                    switch(traverseOp)
                    {
                    case PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_SET_E:
                    case PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_SET_WITH_PCL_E:
                        /* first check we didn't by mistake have a next hop */
                        if (((*iterPtr->currRangePtr)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                            ((*iterPtr->currRangePtr)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                            ((*iterPtr->currRangePtr)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                        }

                        /* now that we finished the lower levels , use the memory , but
                        first check if the size fits */
                        switch ((*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->bucketType)
                        {
                        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
                            bucketSize =
                                (*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->numOfRanges + PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
                            break;
                        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
                            bucketSize =
                                (*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->numOfRanges + PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;
                            break;
                        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                            bucketSize =
                                (*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->numOfRanges + PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;

                            break;
                        default:
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                        }

                        /* check the size */
                        if (memAllocArrayPtr[*memAllocArrayIndexPtr].memSize != bucketSize)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        }

                        /* if already allocated , check for the same place */
                        if (((*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle != 0) &&
                            (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC((*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle) !=
                             memAllocArrayPtr[*memAllocArrayIndexPtr].memAddr))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        }

                        /* ok the size fits , alloc */
                        retVal = prvCpssDmmAllocateByPtr((*iterPtr->currLpmEnginePtr)->structsMemPool,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS*memAllocArrayPtr[*memAllocArrayIndexPtr].memAddr,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS*memAllocArrayPtr[*memAllocArrayIndexPtr].memSize,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS,
                                                         &hwBucketOffsetHandle);
                        if (retVal != GT_OK)
                        {
                            /* go over all blocks bound to the octet and try to allocate */
                            tempNextMemInfoPtr = (*iterPtr->currLpmEnginePtr);

                            while((retVal!=GT_OK)&&(tempNextMemInfoPtr->nextMemInfoPtr!= NULL)&&(hwBucketOffsetHandle==0))
                            {
                                retVal = prvCpssDmmAllocateByPtr(tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS*memAllocArrayPtr[*memAllocArrayIndexPtr].memAddr,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS*memAllocArrayPtr[*memAllocArrayIndexPtr].memSize,
                                                         DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS,
                                                         &hwBucketOffsetHandle);

                                tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                            }

                            if (retVal != GT_OK)
                                return retVal;
                        }

                        (*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle = hwBucketOffsetHandle;
                        /* set Hw bucket update status */
                        (*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->bucketHwUpdateStat =
                            memAllocArrayPtr[*memAllocArrayIndexPtr].bucketHwUpdateStat;

                        break;
                    case PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_REC_E:
                    case PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_REC_WITH_PCL_E:
                        /* now that we finished the lower levels , record this bucket
                           memory */
                        memAllocArrayPtr[*memAllocArrayIndexPtr].memAddr =
                            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC((*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle);

                        memAllocArrayPtr[*memAllocArrayIndexPtr].memSize =
                            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC((*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->hwBucketOffsetHandle);
                        /* Get Hw bucket update status */
                        memAllocArrayPtr[*memAllocArrayIndexPtr].bucketHwUpdateStat =
                            (*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->bucketHwUpdateStat;

                        break;
                    case PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_E:
                    case PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_WITH_PCL_E:
                    default:
                        break;
                    }

                    (*memAllocArrayIndexPtr)++;
                }

                (*iterPtr->currRangePtr) = (*iterPtr->currRangePtr)->next;

                /* check if we run out of memory (or quate)*/
                if (*memAllocArrayIndexPtr >= memAllocArraySize)
                {
                    /* this is where we stop now, we'll continue later*/
                    return (retVal);
                }
            }
        }
        else if (((*iterPtr->currRangePtr)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                 ((*iterPtr->currRangePtr)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                 ((*iterPtr->currRangePtr)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                 (((GT_U32)((*iterPtr->currRangePtr)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
                  (((*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->bucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                   ((*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->bucketType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                   ((*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->bucketType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))))
        {
            (*iterPtr->currRangePtr) = (*iterPtr->currRangePtr)->next;
        }
        else
        {
            /* this means this range has lower levels. go and explore */
            iterPtr->currRangePtr[1] = (*iterPtr->currRangePtr)->lowerLpmPtr.nextBucket->rangeList;

            iterPtr->currRangePtr++;
            iterPtr->currLpmEnginePtr++;

        }
    }

    /* if we reached here it means we finished the tree , so free the iterator
       and make it 0 */
    cpssOsLpmFree(iterPtr);
    *stopPointIterPtr = 0;

    return retVal;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *newRootBucket;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC              **lpmEngineMemPtrPtr;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *rangeListPtr;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *secondRangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *thirdRangePtr = NULL;
    CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT          rangeSelection;
    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC             nextPointerArray[3];
    GT_U8                                           *shareDevsList;
    GT_U32                                          shareDevListLen, devNum;
    GT_U32                                          memSize, lineOffset;
    GT_U8                                           prefixLength;
    GT_STATUS                                       retVal = GT_OK;
    GT_STATUS                                       retVal2 = GT_OK;
    GT_PTR                                          nextHopEntry = NULL;
    GT_U32                                          secondRangePrefixLength = 0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC              *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC              *potentialNewBlockPtr=NULL;
    GT_U32                                          blockStart = 0; /* used as a start point for finding a free block */
    GT_U32                                          newFreeBlockIndex; /* index of a new free block */

    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                        the allocted new element that need to be freed.
                                                                                        Size of the array is 16 for case of IPV6 */

    if ((protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E) && (defMcNextHopEntryPtr != NULL))
    {
        /* FCoE is unicast only */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ((defUcNextHopEntryPtr == NULL) && (defMcNextHopEntryPtr == NULL))
    {
        /* there must be at least one default */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        /* we need to add a nexthop to both UC and MC range. So if one range is
           invalid it will get a nexthop anyway. This is only a dummy nexthop
           that will never be hit. */
        if (defUcNextHopEntryPtr == NULL)
        {
            defUcNextHopEntryPtr = defMcNextHopEntryPtr;
            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                defReservedNextHopEntryPtr = defMcNextHopEntryPtr;
            }
        }
        if (defMcNextHopEntryPtr == NULL)
        {
            defMcNextHopEntryPtr = defUcNextHopEntryPtr;
        }
    }

    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    /* keep values in case reconstruct is needed */
    cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

    /* Create the root bucket. It's created with startAddr 0, which is the start
       of the UC address space */
    newRootBucket = prvCpssDxChLpmRamMngCreateNew(defUcNextHopEntryPtr,
                                                  PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
    if (newRootBucket == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    rangeListPtr = newRootBucket->rangeList;
    PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defUcNextHopEntryPtr->routeEntryMethod,rangeListPtr->pointerType);
    if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        /* Add a range for MC */
        secondRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC));
        if (secondRangePtr == NULL)
        {
            cpssOsLpmFree(rangeListPtr);
            cpssOsLpmFree(newRootBucket);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        rangeListPtr->next = secondRangePtr;
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            newRootBucket->numOfRanges = 3;
            secondRangePtr->startAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS;
            secondRangePrefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS;
        }
        else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
        {
            newRootBucket->numOfRanges = 2;
            secondRangePtr->startAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            secondRangePrefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;
        }
        retVal = prvCpssMathPowerOf2((GT_U8)(secondRangePrefixLength - 1), &secondRangePtr->mask);
        if (retVal != GT_OK)
        {
            cpssOsLpmFree(secondRangePtr);
            cpssOsLpmFree(rangeListPtr);
            cpssOsLpmFree(newRootBucket);
            return retVal;
        }
        PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defMcNextHopEntryPtr->routeEntryMethod,secondRangePtr->pointerType);
        secondRangePtr->lowerLpmPtr.nextHopEntry = defMcNextHopEntryPtr;
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            secondRangePtr->next = NULL;
        }
        secondRangePtr->updateRangeInHw = GT_TRUE;

        /* update the trie */
        retVal = insert2Trie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                             PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                             defMcNextHopEntryPtr);
        if (retVal != GT_OK)
        {
            cpssOsLpmFree(secondRangePtr);
            cpssOsLpmFree(rangeListPtr);
            cpssOsLpmFree(newRootBucket);
            return retVal;
        }
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            /* Add a range for the reserved address space */
            thirdRangePtr = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC));
            if (thirdRangePtr == NULL)
            {
                cpssOsLpmFree(secondRangePtr);
                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            secondRangePtr->next = thirdRangePtr;
            thirdRangePtr->startAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            prefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS;
            retVal = prvCpssMathPowerOf2((GT_U8)(prefixLength - 1), &thirdRangePtr->mask);
            if (retVal != GT_OK)
            {
                /* delete the second range (the one that represents MC) */
                delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);

                cpssOsLpmFree(thirdRangePtr);
                cpssOsLpmFree(secondRangePtr);
                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);
                return retVal;
            }
            PRV_CPSS_DXCH_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defReservedNextHopEntryPtr->routeEntryMethod,thirdRangePtr->pointerType);
            thirdRangePtr->lowerLpmPtr.nextHopEntry = defReservedNextHopEntryPtr;
            thirdRangePtr->next = NULL;
            thirdRangePtr->updateRangeInHw = GT_TRUE;

            /* update the trie */
            retVal = insert2Trie(newRootBucket, thirdRangePtr->startAddr, prefixLength,
                                 PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                 defReservedNextHopEntryPtr);
            if (retVal != GT_OK)
            {
                /* delete the second range (the one that represents MC) */
                delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);
                cpssOsLpmFree(thirdRangePtr);
                cpssOsLpmFree(secondRangePtr);
                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);
                return retVal;
            }

        }
    }

    newRootBucket->bucketType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;

    if (updateHw == GT_TRUE)
    {
        /* both unicast and multicast use ucSearchMemArrayPtr */
        lpmEngineMemPtrPtr = shadowPtr->ucSearchMemArrayPtr[protocol];

        /* set the new bucket's memory size: one line for the bit vector + line(s)
           for the range(s) */
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
        {
            /* one range (unicast) */
            memSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS + 1;
        }
        else if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            /* 3 ranges (unicast, multicast, reserved) */
            memSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS + 3;
        }
        else /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
        {
            /* 2 ranges (unicast, multicast) */
            memSize = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS + 2;
        }
        newRootBucket->hwBucketOffsetHandle =
            prvCpssDmmAllocate(lpmEngineMemPtrPtr[0]->structsMemPool,
                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSize,
                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);

        if (newRootBucket->hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND)
        {
            /* go over all blocks bound to the octet and try to allocate */
            tempNextMemInfoPtr = lpmEngineMemPtrPtr[0];

            while((tempNextMemInfoPtr->nextMemInfoPtr != NULL)&&(newRootBucket->hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND))
            {
                newRootBucket->hwBucketOffsetHandle =
                                            prvCpssDmmAllocate(tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool,
                                                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSize,
                                                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
                tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
            }

            /* could not find an empty space in the current bound blocks - continue looking */
            if ((tempNextMemInfoPtr->nextMemInfoPtr == NULL) && (newRootBucket->hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND))
            {
                /* go over all blocks until the memory of the octet can be allocated in the block found */
                while (blockStart < shadowPtr->numOfLpmMemories)
                {
                    /* find a new free block and bind it to the octet and protocol */
                    retVal = prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol(shadowPtr,
                                                                             protocol,
                                                                             0,
                                                                             blockStart,
                                                                             &newFreeBlockIndex);

                    if (retVal != GT_OK)
                    {
                        break;
                    }

                    /* allocate the memory needed from the new structsMemPool bound */
                     newRootBucket->hwBucketOffsetHandle =
                        prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex],
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSize,
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);

                    /* neededMemoryBlocks is not 0 nor 0xFFFFFFFF --> we did succeed in allocating the
                       needed memory from the valid block we found */
                    if ((newRootBucket->hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) && (newRootBucket->hwBucketOffsetHandle != DMM_MALLOC_FAIL))
                    {
                        /* allocate a new elemenet block to the list */
                        potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                        if (potentialNewBlockPtr == NULL)
                        {
                            retVal = GT_OUT_OF_CPU_MEM;
                            goto delete_ranges;
                        }
                        potentialNewBlockPtr->ramIndex = newFreeBlockIndex;
                        potentialNewBlockPtr->structsBase = 0;
                        potentialNewBlockPtr->structsMemPool = 0;
                        potentialNewBlockPtr->nextMemInfoPtr = NULL;

                        /* bind the new block */
                        potentialNewBlockPtr->structsMemPool=shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex];
                        tempNextMemInfoPtr->nextMemInfoPtr = potentialNewBlockPtr;

                        /* mark the block as used */
                        shadowPtr->lpmRamOctetsToBlockMappingPtr[newFreeBlockIndex].isBlockUsed = GT_TRUE;
                        /*  set the block to be used by the specific octet and protocol*/
                        PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,protocol,0,newFreeBlockIndex);

                         /*  set pending flag for future need */
                        shadowPtr->pendingBlockToUpdateArr[newFreeBlockIndex].updateInc=GT_TRUE;
                        shadowPtr->pendingBlockToUpdateArr[newFreeBlockIndex].numOfIncUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle);

                        /* We only save the first element allocated or bound per octet */
                        if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[0]==NULL)||
                            (allNewNextMemInfoAllocatedPerOctetArrayPtr[0]->structsMemPool==0))
                        {
                            /* keep the head of the list we need to free incase of an error */
                            allNewNextMemInfoAllocatedPerOctetArrayPtr[0] = tempNextMemInfoPtr;
                        }

                        break;
                    }
                    /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the
                        needed memory from the valid block we found, look for another valid block or
                        return an error incase of CPU memory allocation fail  */
                    else
                    {
                        if (newRootBucket->hwBucketOffsetHandle != DMM_MALLOC_FAIL)
                        {
                            blockStart = newFreeBlockIndex + 1;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the needed memory  */
                if ((newRootBucket->hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND) || (newRootBucket->hwBucketOffsetHandle == DMM_MALLOC_FAIL))
                {
                    if (newRootBucket->hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND)
                    {
                        retVal = GT_OUT_OF_PP_MEM;
                    }
                    else
                    {
                        retVal = GT_OUT_OF_CPU_MEM;
                    }
                }
            }
            else
            {
                /* we had a CPU Memory allocation error */
                if(newRootBucket->hwBucketOffsetHandle == DMM_MALLOC_FAIL)
                {
                    retVal = GT_OUT_OF_CPU_MEM;
                }
                else
                {
                    /*We succeed in allocating the memory*/
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    /* set pending flag for future need */
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle);
                }
            }

delete_ranges:
            if (retVal != GT_OK)
            {
                if ((newRootBucket->hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) && (newRootBucket->hwBucketOffsetHandle != DMM_MALLOC_FAIL))
                {
                    /*We failed in allocating the memory*/
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    /* set pending flag for future need */
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;

                    prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);
                }

                if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    /* delete the third range (the one that represents reserved) */
                    delFromTrie(newRootBucket, thirdRangePtr->startAddr, PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS,
                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);

                    cpssOsLpmFree(thirdRangePtr);
                }
                if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
                {
                    /* delete the second range (the one that represents MC) */
                    delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);
                    cpssOsLpmFree(secondRangePtr);
                }

                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);

                /* free the allocated/bound RAM memory */
                retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
                if (retVal2!=GT_OK)
                {
                    return retVal2;
                }
                /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
                return retVal;
            }
        }
        else
        {
             /*  set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[0].updateInc=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[0].numOfIncUpdates += memSize;
        }


        lineOffset = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(
            newRootBucket->hwBucketOffsetHandle) + lpmEngineMemPtrPtr[0]->structsBase;
        shareDevsList = shadowPtr->workDevListPtr->shareDevs;
        shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;
        cpssOsMemSet(&rangeSelection, 0,
                     sizeof(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT));
        cpssOsMemSet(&nextPointerArray, 0, sizeof(nextPointerArray));
        retVal = getMirrorBucketDataAndUpdateRangesAddress(rangeListPtr,
                                                           lpmEngineMemPtrPtr,
                                                           newRootBucket->bucketType,
                                                           lineOffset,
                                                           &rangeSelection,
                                                           nextPointerArray);
        if (retVal != GT_OK)
        {
            if ((newRootBucket->hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) && (newRootBucket->hwBucketOffsetHandle != DMM_MALLOC_FAIL))
            {
                /*We fail in allocating the memory*/
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                /* set pending flag for future need */
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;

                prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);
            }

            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                /* delete the third range (the one that represents reserved) */
                delFromTrie(newRootBucket, thirdRangePtr->startAddr, PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS,
                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);

                cpssOsLpmFree(thirdRangePtr);
            }
            if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
            {
                /* delete the second range (the one that represents MC) */
                delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);
                cpssOsLpmFree(secondRangePtr);
            }
            cpssOsLpmFree(rangeListPtr);
            cpssOsLpmFree(newRootBucket);

            /* free the allocated/bound RAM memory */
            retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }

            /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

            return retVal;
        }

        for (devNum = 0; devNum < shareDevListLen; devNum++)
        {
            retVal = cpssDxChLpmNodeWrite(shareDevsList[devNum],
                                          lineOffset,
                                          newRootBucket->bucketType,
                                          &rangeSelection,
                                          newRootBucket->numOfRanges,
                                          nextPointerArray);
            if (retVal != GT_OK)
            {
                if ((newRootBucket->hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) && (newRootBucket->hwBucketOffsetHandle != DMM_MALLOC_FAIL))
                {
                     /*We fail in allocating the memory*/
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    /* set pending flag for future need */
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;

                    prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);
                }

                if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    /* delete the third range (the one that represents reserved) */
                    delFromTrie(newRootBucket, thirdRangePtr->startAddr, PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS,
                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);

                    cpssOsLpmFree(thirdRangePtr);
                }
                if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
                {
                    /* delete the second range (the one that represents MC) */
                    delFromTrie(newRootBucket, secondRangePtr->startAddr, secondRangePrefixLength,
                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &nextHopEntry);
                    cpssOsLpmFree(secondRangePtr);
                }
                cpssOsLpmFree(rangeListPtr);
                cpssOsLpmFree(newRootBucket);
                /* free the allocated/bound RAM memory */
                retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
                if (retVal2!=GT_OK)
                {
                    return retVal2;
                }
                /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

                return retVal;
            }
        }
    }

    /* allocation passed - update the protocolCountersPerBlockArr according to the pending array */
    retVal = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                       shadowPtr->pendingBlockToUpdateArr,
                                                       shadowPtr->protocolCountersPerBlockArr,
                                                       shadowPtr->pendingBlockToUpdateArr,
                                                       protocol,
                                                       shadowPtr->numOfLpmMemories);
    if (retVal!=GT_OK)
    {
        /* reset pending array for future use */
        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

        return retVal;
    }

    /* update partition block */
    if (newRootBucket->hwBucketOffsetHandle!=0)
    {
        SET_DMM_BLOCK_PROTOCOL(newRootBucket->hwBucketOffsetHandle, protocol);
        SET_DMM_BUCKET_SW_ADDRESS(newRootBucket->hwBucketOffsetHandle, newRootBucket);
    }
    shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol] = newRootBucket;
    shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] = newRootBucket->bucketType;
    shadowPtr->vrRootBucketArray[vrId].valid = GT_TRUE;
    shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_FALSE;
    if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocol] =
            defMcNextHopEntryPtr;
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *rootBucketPtr;
    GT_U32                                          expectedNumOfRanges;
    GT_U8                                           mcStartAddr = 0;
    GT_U8                                           reservedStartAddr = 0;
    GT_U32                                          mcPrefixLength = 0;
    GT_U32                                          reservedPrefixLength = 0;
    GT_PTR                                          firstNextHopEntry = NULL;
    GT_PTR                                          secondNextHopEntry = NULL;
    GT_PTR                                          thirdNextHopEntry = NULL;

    if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            expectedNumOfRanges = 3;
            mcStartAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS;
            mcPrefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS;
            reservedStartAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            reservedPrefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS;
        }
        else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
        {
            expectedNumOfRanges = 2;
            mcStartAddr = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            mcPrefixLength = PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS;
        }
    }
    else
    {
        expectedNumOfRanges = 1;
    }

    rootBucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol];
    if (rootBucketPtr->numOfRanges != expectedNumOfRanges)
    {
        /* the root bucket is not empty */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if (rootBucketPtr->numOfRanges == 3)
    {
        /* delete the third range (the one that represents the reserved address space) */
        delFromTrie(rootBucketPtr, reservedStartAddr, reservedPrefixLength,
                    PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &thirdNextHopEntry);
        if (thirdNextHopEntry != NULL)
        {
            cpssOsLpmFree(thirdNextHopEntry);
        }
        cpssOsLpmFree(rootBucketPtr->rangeList->next->next);
        rootBucketPtr->numOfRanges--;
    }

    if (rootBucketPtr->numOfRanges == 2)
    {
        /* delete the second range (the one that represents MC) */
        delFromTrie(rootBucketPtr, mcStartAddr, mcPrefixLength,
                    PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &secondNextHopEntry);
        if ((secondNextHopEntry != NULL) && (secondNextHopEntry != thirdNextHopEntry))
        {
            cpssOsLpmFree(secondNextHopEntry);
        }
        cpssOsLpmFree(rootBucketPtr->rangeList->next);
    }

    /* Now delete the UC range */
    delFromTrie(rootBucketPtr, 0, 0, PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS, &firstNextHopEntry);
    if ((firstNextHopEntry != NULL) && (firstNextHopEntry != secondNextHopEntry) &&
        (firstNextHopEntry != thirdNextHopEntry))
    {
        cpssOsLpmFree(firstNextHopEntry);
    }

    cpssOsLpmFree(rootBucketPtr->rangeList);
    cpssOsLpmFree(rootBucketPtr);

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC   *vrRootBucket;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     rootBucketType;
    GT_U32                                  headOfTrie;
    GT_U32                                  headOfTrieLineOffset;
    GT_U8                                   *shareDevsList;
    GT_U32                                  numOfSharedDevs;
    GT_U32                                  dev;
    GT_STATUS                               retVal;
    GT_U8                                   rootLevelMemory;

    /* rootLevelMemory is 0 for now, need to be changed */
    CPSS_TBD_BOOKMARK
    rootLevelMemory = 0;

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    numOfSharedDevs = shadowPtr->workDevListPtr->shareDevNum;
    vrRootBucket    = &(shadowPtr->vrRootBucketArray[vrId]);
    if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        headOfTrieLineOffset =
            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(vrRootBucket->rootBucket[protocol]->hwBucketOffsetHandle);
    }
    else
    {
        headOfTrieLineOffset =
            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(vrRootBucket->rootBucket[protocol]->hwBucketOffsetHandle);
    }
    if (vrRootBucket->rootBucket[protocol] != NULL)
    {
        rootBucketType = vrRootBucket->rootBucketType[protocol];
        headOfTrie = headOfTrieLineOffset + shadowPtr->lpmMemInfoArray[protocol][rootLevelMemory].structsBase;

        for (dev = 0; dev < numOfSharedDevs; dev++)
        {
            retVal = prvCpssDxChLpmHwVrfEntryWrite(shareDevsList[dev],
                                                   shadowPtr->shadowType,
                                                   vrId,
                                                   protocol,
                                                   rootBucketType,
                                                   headOfTrie);
            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempCurrMemInfoPtr;    /* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempToFreeMemInfoPtr;  /* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;    /* use for going over the list of blocks per octet */
    GT_UINTPTR                           tmpStructsMemPool;     /* use for going over the list of blocks per octet */

    /* in case we have only one  element in the linked list */
    if(shadowPtr->lpmMemInfoArray[protocol][octetIndex].nextMemInfoPtr==NULL)
    {
        if((shadowPtr->lpmMemInfoArray[protocol][octetIndex].structsMemPool) == (shadowPtr->lpmRamStructsMemPoolPtr[blockIndex]))
        {
            shadowPtr->lpmMemInfoArray[protocol][octetIndex].ramIndex = 0;
            shadowPtr->lpmMemInfoArray[protocol][octetIndex].structsBase = 0;
            shadowPtr->lpmMemInfoArray[protocol][octetIndex].structsMemPool = 0;
            shadowPtr->lpmMemInfoArray[protocol][octetIndex].nextMemInfoPtr = NULL;
            PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadow, protocol, octetIndex, blockIndex);
            /* the blocks that was freed should stay free even if we get an error in the next phases so we update
               tempLpmRamOctetsToBlockMappingUsedForReconstractPtr to be used in case reconstruct is needed,*/
            PRV_CPSS_DXCH_TEMP_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadow, protocol, octetIndex, blockIndex);
        }
        return GT_OK;
    }
    else
    {
       tempCurrMemInfoPtr = &(shadowPtr->lpmMemInfoArray[protocol][octetIndex]);
       tempToFreeMemInfoPtr = shadowPtr->lpmMemInfoArray[protocol][octetIndex].nextMemInfoPtr;
       tempNextMemInfoPtr = shadowPtr->lpmMemInfoArray[protocol][octetIndex].nextMemInfoPtr;
       tmpStructsMemPool = shadowPtr->lpmMemInfoArray[protocol][octetIndex].structsMemPool;

       /* first option is that we have a linked list but the first element is empty (block was binded but no prefix is defined on it),
          so we need to copy the values of second element to the first one */
       if(tmpStructsMemPool == (shadowPtr->lpmRamStructsMemPoolPtr[blockIndex]))
       {
           shadowPtr->lpmMemInfoArray[protocol][octetIndex].ramIndex = tempNextMemInfoPtr->ramIndex;
           shadowPtr->lpmMemInfoArray[protocol][octetIndex].structsBase = tempNextMemInfoPtr->structsBase;
           shadowPtr->lpmMemInfoArray[protocol][octetIndex].structsMemPool = tempNextMemInfoPtr->structsMemPool;
           shadowPtr->lpmMemInfoArray[protocol][octetIndex].nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
           cpssOsFree(tempToFreeMemInfoPtr);
           PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadow, protocol, octetIndex, blockIndex);
           /* the blocks that was freed should stay free even if we get an error in the next phases so we update
               tempLpmRamOctetsToBlockMappingUsedForReconstractPtr to be used in case reconstruct is needed,*/
           PRV_CPSS_DXCH_TEMP_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadow, protocol, octetIndex, blockIndex);
           return GT_OK;
       }
       else
       {
           /* second option is that the first element is not empty --> then we go over
              the linked list looking for the block we need to free.
              Once found we remove the element from the linked list and free allocated struct */
           while (tempCurrMemInfoPtr->nextMemInfoPtr!=NULL)
           {
               tmpStructsMemPool = tempCurrMemInfoPtr->nextMemInfoPtr->structsMemPool;
                /* need to remove the structMemPool related to blockIndex from the list assosiated with this octetIndex */
                if(tmpStructsMemPool== (shadowPtr->lpmRamStructsMemPoolPtr[blockIndex]))
                {
                    /* we found an element that need to be removed */
                    tempCurrMemInfoPtr->nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                    cpssOsFree(tempToFreeMemInfoPtr);
                    PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadow, protocol, octetIndex, blockIndex);
                    /* the blocks that was freed should stay free even if we get an error in the next phases so we update
                       tempLpmRamOctetsToBlockMappingUsedForReconstractPtr to be used in case reconstruct is needed,*/
                    PRV_CPSS_DXCH_TEMP_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadow, protocol, octetIndex, blockIndex);
                    return GT_OK;

                }
                else
                {
                    tempCurrMemInfoPtr = tempCurrMemInfoPtr->nextMemInfoPtr;
                    tempToFreeMemInfoPtr = tempToFreeMemInfoPtr->nextMemInfoPtr;
                    tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                }
           }
       }
    }
    return GT_OK;
}


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
)
{
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol;
    GT_U32                                  maxNumOfOctets;
    GT_U32                                  sumOfCounters=0;
    GT_DMM_PARTITION                        *partition;
    GT_U32                                  octetIndex;
    GT_STATUS                               retVal;

    /* check if the block is empty and can be unbounded and reused */
    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[blockIndex];

    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* For each protocol - go over all octets bounded to the block  */
        for (protocol = 0; protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protocol++)
        {
            switch (protocol)
            {
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS;
                }
                else
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
                }
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS;
                }
                else
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
                }
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS;
                }
                else
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
                }
                break;
            default:
                maxNumOfOctets = PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS;
                break;
            }
            if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr, protocol, blockIndex) == GT_TRUE)
            {
                /* check what octet is using the block */
                for (octetIndex=0; octetIndex < maxNumOfOctets; octetIndex++)
                {
                    if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr, protocol, octetIndex, blockIndex) == GT_TRUE)
                    {
                        /* free the block from the linked list of the protocol and set the block as unused by the octet */
                        retVal = prvCpssDxChLpmRamMngUnbindBlockFromProtocolAndOctet(shadowPtr,
                                                                                    (PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT)protocol,
                                                                                     octetIndex,
                                                                                     blockIndex);
                        if (retVal!=GT_OK)
                        {
                            return retVal;
                        }
                    }
                }
            }
        }

        /* if the block is not used by any protocol set it as not used */
        if(((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,blockIndex)==GT_FALSE))&&
            ((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,blockIndex)==GT_FALSE))&&
            ((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,blockIndex)==GT_FALSE)))
        {
            shadowPtr->lpmRamOctetsToBlockMappingPtr[blockIndex].isBlockUsed = GT_FALSE;
            shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr[blockIndex].isBlockUsed = GT_FALSE;
            return GT_OK;
        }
    }
    else/* the partition is not empty --> the block is not empty */
    {
        /* For each protocol - go over all octets bounded to the block  */
        for (protocol = 0; protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protocol++)
        {
            switch (protocol)
            {
             case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                sumOfCounters = shadowPtr->protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters;
                if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS;
                }
                else
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
                }
                break;
             case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                sumOfCounters = shadowPtr->protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters;
                if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS;
                }
                else
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
                }
                break;
             case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                sumOfCounters = shadowPtr->protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters;
                if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS;
                }
                else
                {
                    maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"error in prvCpssDxChLpmRamMngUnbindBlock \n");
            }

            if ((sumOfCounters==0)&&(shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc==GT_FALSE))
            {
                /* the protocol is not using this block --> unbind it from the list */
                if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr, protocol, blockIndex) == GT_TRUE)
                {
                    /* check what octet is using the block */
                    for (octetIndex=0; octetIndex < maxNumOfOctets; octetIndex++)
                    {
                        if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr, protocol, octetIndex, blockIndex) == GT_TRUE)
                        {
                            /* free the block from the linked list of the protocol and set the block as unused by the octet */
                            retVal = prvCpssDxChLpmRamMngUnbindBlockFromProtocolAndOctet(shadowPtr,
                                                                                        (PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT)protocol,
                                                                                         octetIndex,
                                                                                         blockIndex);
                            if (retVal!=GT_OK)
                            {
                                return retVal;
                            }
                        }
                    }
                }
            }
        }
    }
    return GT_OK;
}

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
)
{
    GT_U32      i;
    GT_BOOL     potentialBlockToUseThatViolateWireSpeedFound=GT_FALSE;
    GT_U32      potentialBlockToUseThatViolateWireSpeedIndex=0;
    GT_U32      tempBlockIndex;
    GT_STATUS   retVal = GT_OK;


    tempBlockIndex = octetIndex;

   /* if the block we want to start looking from is smaller then the octetIndex then we
      try to allocate first from octetIndex and only then we run on blockStart */
    if (blockStart<octetIndex)
    {
         /* first we try to allocated to octetIndex the block with the same index */
        if (shadowPtr->lpmRamOctetsToBlockMappingPtr[tempBlockIndex].isBlockUsed == GT_FALSE)
        {
            *blockIndexPtr=tempBlockIndex;
            return GT_OK;
        }
        else
        {
            /* make sure that the block is not already in use by the specific octet */
            if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr,protocol,octetIndex,tempBlockIndex)==GT_FALSE)
            {
                /* if the blocked is marked as used we need to check if it is empty,
                   if it is we need to unbind it from current "octet to block mapping" and reuse it */
                 if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    retVal = prvCpssDxChLpmRamMngUnbindBlock(shadowPtr, tempBlockIndex);
                    if (retVal!=GT_OK)
                    {
                        return retVal;
                    }
                }
               /* check again if the block was unbounded and can be reused
                   if the block is used BUT not by the specific input protocol - not violate full wire speed */
                if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocol,tempBlockIndex)==GT_FALSE)
                {
                    *blockIndexPtr = tempBlockIndex;
                    return GT_OK;
                }
            }
        }
    }

    for (i = blockStart; i < shadowPtr->numOfLpmMemories; i++)
    {
        /* try to find an empty block */
        if (shadowPtr->lpmRamOctetsToBlockMappingPtr[i].isBlockUsed==GT_FALSE)
        {
            *blockIndexPtr=i;
            return GT_OK;
        }
        else
        {
            /* only if the octet is not already bind to the specific block then
               we try to find if this block if free and can be used by the octet */
            if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr,protocol,octetIndex,i)==GT_FALSE)
            {
                /* if the blocked is marked as used we need to check if it is empty,
                   if it is we need to unbind it from current "octet to block mapping" and reuse it */
                if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    retVal = prvCpssDxChLpmRamMngUnbindBlock(shadowPtr,i);
                    if (retVal!=GT_OK)
                    {
                        return retVal;
                    }
                }
                /* check again if the block was unbounded and can be reused
                   if the block is used BUT not by the specific input protocol - not violate full wire speed */
                if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocol,i)==GT_FALSE)
                {
                    *blockIndexPtr = i;
                    return GT_OK;
                }
            }
        }
    }

    for (i = blockStart; i < shadowPtr->numOfLpmMemories; i++)
    {
        /* if the block is used BUT not by the specific input protocol - not violate full wire speed */
        if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocol,i)==GT_FALSE)
        {
            *blockIndexPtr = i;
            return GT_OK;
        }

        if((shadowPtr->lpmRamBlocksAllocationMethod==PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E)&&
           (potentialBlockToUseThatViolateWireSpeedFound==GT_FALSE))
        {
            /* if the block is used by the specific input protocol but not by the specific input octet
               - violate full wire speed */
            if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_OCTET_IN_PROTOCOL_GET_MAC(shadowPtr,protocol,octetIndex,i)==GT_FALSE)
            {
                potentialBlockToUseThatViolateWireSpeedIndex = i;
                potentialBlockToUseThatViolateWireSpeedFound=GT_TRUE;
            }
        }
    }
    if(potentialBlockToUseThatViolateWireSpeedFound==GT_TRUE)
    {
         *blockIndexPtr = potentialBlockToUseThatViolateWireSpeedIndex;
          return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,"error in prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol - GT_NOT_FOUND \n");
}


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
)
{
    GT_DMM_BLOCK        *bucketBlock;
    GT_U32              sizeOfOptionalUsedAndFreeBlock;

    /* for a given oldBucketPtr we check if there is free memory near it
      (on top or below) that can be merged with the current oldBucketPtr
      memory space to create a bigger block */

    bucketBlock = (GT_DMM_BLOCK *)oldBucketPtr->hwBucketOffsetHandle;

    if( (oldBucketPtr->hwBucketOffsetHandle == 0) ||
        (DMM_BLOCK_STATUS(bucketBlock) != DMM_BLOCK_ALLOCATED) ||
        (DMM_BLOCK_STAMP(bucketBlock) != DMM_STAMP_USED) )
    {
        cpssOsPrintf("oldBucketPtr was not allocated\n");
        *freeBlockCanBeFoundPtr=GT_FALSE;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngFindIfSwapAreaCanBeUsefulToFindFreeBlock - GT_FAIL \n");
    }

    sizeOfOptionalUsedAndFreeBlock = prvCpssDmmCheckResizeAvailableWithSameMemory(oldBucketPtr->hwBucketOffsetHandle,newBucketSize);
    if (sizeOfOptionalUsedAndFreeBlock>0)
    {
        *freeBlockCanBeFoundPtr=GT_TRUE;
    }
    else
    {
        *freeBlockCanBeFoundPtr=GT_FALSE;
    }

    return GT_OK;
}

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
*                                      it means this is a new bucket
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
)
{
    GT_U32        oldBlockIndex=0;
    GT_U32        sumOfFreeBlockAfterShrink=0;
    GT_BOOL       useSwapArea=GT_FALSE;
    GT_DMM_BLOCK  *usedBlock;
    GT_U32        oldBlockSize=0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempMemInfoPtr;/* use for going over the list of blocks per octet */
    GT_U32        i=0;


    /* Phase 1 */
    /* Go over the partition were the old bucket is defined
       check if shrink can be useful when looking at the
       allocated block as optional to be freed using the swap_area_1 */
    if (oldHandleAddr!=0)
    {
        oldBlockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(oldHandleAddr) / (shadowPtr->lpmRamTotalBlocksSizeIncludingGap);

        /*first go over the partition were the old bucket is defined*/
        useSwapArea = GT_TRUE;
        usedBlock = (GT_DMM_BLOCK *)oldHandleAddr;
        oldBlockSize = SIZE_IN_WORDS(usedBlock);
        sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(shadowPtr->lpmRamStructsMemPoolPtr[oldBlockIndex],
                                                                                       newBucketSize,
                                                                                       oldBlockSize,
                                                                                       &useSwapArea);
        if (sumOfFreeBlockAfterShrink>0)
        {
            *freeBlockCanBeFoundPtr=GT_TRUE;
            *bankIndexForShrinkPtr  = oldBlockIndex;
            *useSwapAreaPtr = useSwapArea;
            return GT_OK;
        }
    }

    /* Phase 2 - continue looking */
    /* Go over all the list of partition related to the octet*/
    tempMemInfoPtr = lpmMemInfoPtr;

    while(tempMemInfoPtr!= NULL)
    {
        useSwapArea = GT_FALSE;
        sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(tempMemInfoPtr->structsMemPool,
                                                                                    newBucketSize,
                                                                                    oldBlockSize,
                                                                                    &useSwapArea);
        if (sumOfFreeBlockAfterShrink>0)
        {
            *bankIndexForShrinkPtr=tempMemInfoPtr->ramIndex;
            *freeBlockCanBeFoundPtr=GT_TRUE;
            *useSwapAreaPtr = useSwapArea;
            return GT_OK;
        }

        tempMemInfoPtr = tempMemInfoPtr->nextMemInfoPtr;
    }

    /* Phase 3 */
    /* if we get here it means we did not find place to shrink in the bounded memory,
       try to see if shrink memory can be useful in unbounded blocks */
    for (i = 0; i < shadowPtr->numOfLpmMemories; i++)
    {
        /* if the block is used BUT not by the specific input protocol - not violate full wire speed */
        if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocol,i)==GT_FALSE)
        {
            /* check if shrink is an option */
            useSwapArea = GT_FALSE;
            sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(shadowPtr->lpmRamStructsMemPoolPtr[i],
                                                                                           newBucketSize,
                                                                                           oldBlockSize,
                                                                                           &useSwapArea);
            if (sumOfFreeBlockAfterShrink>0)
            {
                *freeBlockCanBeFoundPtr=GT_TRUE;
                *bankIndexForShrinkPtr  = i;
                *useSwapAreaPtr = useSwapArea;
                return GT_OK;
            }
        }
    }

    /* if we reach here it means no block was found for shrink */
    *freeBlockCanBeFoundPtr=GT_FALSE;
    *bankIndexForShrinkPtr  = 0;
    *useSwapAreaPtr = GT_FALSE;

    return GT_OK;
}

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
)
{
    GT_STATUS                                retVal=GT_OK;

    /* the fields for writing the bucket to the HW */
    static CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT rangeSelectionSec;
    static CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC nextPointersArray[256];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;
    GT_U32                                   blockIndex=0;
    GT_U32                                   i=0;

    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;

    /* oldBucketShadowPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    pRange              = oldBucketShadowPtr->rangeList;

    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    if (phaseFlag == PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E)
    {

        cpssOsMemSet(&rangeSelectionSec, 0, sizeof(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT));
        /* create the bucket data, and update the ranges hw address */
        getMirrorBucketDataAndUpdateRangesAddress(pRange,
                                                  lpmMemInfoPtrPtr,
                                                  oldBucketShadowPtr->bucketType,
                                                  bucketBaseAddress,
                                                  &rangeSelectionSec,
                                                  nextPointersArray);

        for (i = 0; i < shareDevListLen; i++)
        {
            retVal = cpssDxChLpmNodeWrite(shareDevsList[i],
                                          bucketBaseAddress,
                                          oldBucketShadowPtr->bucketType,
                                          &rangeSelectionSec,
                                          oldBucketShadowPtr->numOfRanges,
                                          nextPointersArray);
            if (retVal != GT_OK)
            {
                /* this is a fail that do not support any rollback operation,
                   because if we get here it is after we did all calculations
                   needed and we verified that the data can be written to the HW.
                   if we get a fail it means a fatal error that should not be
                   happen and the behavior is unpredicted */

                /* just need to free any allocations done in previous stage */
                if(needToFreeAllocationInCaseOfFail == GT_TRUE)
                {
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed);

                    prvCpssDmmFree(tempHwAddrHandleToBeFreed);
                }

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkUpdatePointersPhase1Phase2 - GT_FAIL \n");
            }
        }
    }
    else /* PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E */
    {
         /* in this mode we immidiatly update the parent range in
           order to free the bucket's memory for further use */
        if (oldBucketShadowPtr->pointingRangeMemAddr == 0)
        {
            /* if the parentRangeMemAddr = 0 then it means the parent
               range is not an regular lpm range, thus order it's
               rewriting using the function ptr*/
            if (parentWriteFuncPtr->bucketParentWriteFunc(
                parentWriteFuncPtr->data) != GT_OK)
            {
                 /* just need to free any allocations done in previous stage */
                if(needToFreeAllocationInCaseOfFail == GT_TRUE)
                {
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed);

                    prvCpssDmmFree(tempHwAddrHandleToBeFreed);
                }
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkUpdatePointersPhase1Phase2 - GT_FAIL \n");
            }
        }
        else
        {
            /* write the bucket's parent range in order to be able to
               free this bucket's old memory */
            if (updateMirrorRange(oldBucketShadowPtr,
                                  lpmMemInfoPtrPtr,
                                  shadowPtr)
                != GT_OK)
            {

                 /* just need to free any allocations done in previous stage */
                if(needToFreeAllocationInCaseOfFail == GT_TRUE)
                {
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandleToBeFreed)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandleToBeFreed);

                    prvCpssDmmFree(tempHwAddrHandleToBeFreed);
                }

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkUpdatePointersPhase1Phase2 - GT_FAIL \n");
            }
        }
    }
    return retVal;
}


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
)
{
    GT_STATUS   retVal=GT_OK;
    GT_U32      blockIndex=0;

    /* free this bucket's old memory */
    if  ((oldBucketShadowPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
        (oldBucketShadowPtr->bucketType != CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) &&
        (oldBucketShadowPtr->bucketType != CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E))
    {
        /*  set pending flag for future need */
        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
        if (blockIndex!=bankIndex)
        {
            /* we should be working on the bankIndex we got as parameter to the function */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkFreeOldBucket - GT_FAIL \n");
        }
        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
            PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle);

        prvCpssDmmFree(oldHwAddrHandle);
    }
    return retVal;
}

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
)
{
    GT_STATUS                                retVal=GT_OK;
    GT_DMM_PARTITION                         *partition;
    GT_UINTPTR                                oldHwAddrHandle=0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      oldHwProtocol;/* the protocol of the bucket we are moving */
    GT_UINTPTR                                tempHwAddrHandle=0;

    GT_U32                                   memBlockBase = 0;/* the base address (offset) in the RAM for bucket*/
    GT_U32                                   tempAddr = 0;     /* Temporary address                */
    GT_U32                                   blockIndex=0;

    GT_BOOL     needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR  tempHwAddrHandleToBeFreed = 0;

    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndex];
    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1 - GT_FAIL \n");
    }

    /* oldBucketShadowPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    oldHwAddrHandle     = oldBucketShadowPtr->hwBucketOffsetHandle;
    memBlockBase        = lpmMemInfoPtrPtr[octetIndex]->structsBase;
    oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle);

    if(copyToFromSwapArea==PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E)
    {
        /* create the HW data to be writen to swap_area_1 according to the oldBucketPtr */
        tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->swapMemoryAddr) + memBlockBase;
    }
    else
    {
        /* Swap memory is in use , and there is a need to move and rewrite the bucket back to the partition it was */
        tempHwAddrHandle = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[bankIndex],
                                              DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * oldMemSize,
                                              DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
        if ((tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle == DMM_MALLOC_FAIL))
        {
            /* should never happen since we just freed this mem when calling
               prvCpssDxChLpmRamMngShrinkBankCopyToSwapArea1AndUpdatePointers*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1 - GT_FAIL \n");
        }
        else
        {
             /* keep new tempHwAddrHandle -- in case of a fail should be freed */
            needToFreeAllocationInCaseOfFail = GT_TRUE;
            tempHwAddrHandleToBeFreed = tempHwAddrHandle;

           /*  set pending flag for future need */
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            if (blockIndex!=bankIndex)
            {
                /* we should be working on the bankIndex we got as parameter to the function */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1 - GT_FAIL \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);
        }

        tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)) + memBlockBase;

   }

    retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                            lpmMemInfoPtrPtr,
                                                                            tempAddr,
                                                                            oldBucketShadowPtr,
                                                                            needToFreeAllocationInCaseOfFail,
                                                                            tempHwAddrHandleToBeFreed,
                                                                            parentWriteFuncPtr,
                                                                            PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E);
    if (retVal!=GT_OK)
    {
       /* in case of a fail any allocations done in previous stages was already
          free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1 - GT_FAIL \n");
    }
    if(copyToFromSwapArea==PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E)
    {
        /* ok since the bucket is updated in the HW, we can now update the shadow */
        oldBucketShadowPtr->hwBucketOffsetHandle = shadowPtr->swapMemoryAddr;
    }
    else
    {
       /* ok since the bucket is updated in the HW, we can now update the shadow */
        oldBucketShadowPtr->hwBucketOffsetHandle = tempHwAddrHandle;
        SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle, oldHwProtocol);
        SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwBucketOffsetHandle, oldBucketShadowPtr);
    }

    retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                            lpmMemInfoPtrPtr,
                                                                            tempAddr,
                                                                            oldBucketShadowPtr,
                                                                            needToFreeAllocationInCaseOfFail,
                                                                            tempHwAddrHandleToBeFreed,
                                                                            parentWriteFuncPtr,
                                                                            PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E);
    if (retVal!=GT_OK)
    {
       /* in case of a fail any allocations done in previous stages was already
          free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1 - GT_FAIL \n");
    }
    if(copyToFromSwapArea==PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E)
    {
       /* now it's ok to free this bucket's old memory */
        retVal = prvCpssDxChLpmRamMngShrinkFreeOldBucket(shadowPtr,
                                                         bankIndex,
                                                         oldHwAddrHandle,
                                                         oldBucketShadowPtr);
        if (retVal!=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1 - GT_FAIL \n");
        }
    }
    return GT_OK;
}

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
)
{
    GT_STATUS                                retVal=GT_OK;
    GT_DMM_PARTITION                         *partition;
    GT_UINTPTR                               oldHwAddrHandle=0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     oldHwProtocol;/* the protocol of the bucket we are moving */
    GT_UINTPTR                               tempHwAddrHandle=0;
    GT_BOOL                                  copyToSwapArea2=GT_TRUE;

    GT_U32                                   memBlockBase = 0;/* the base address (offset) in the RAM for bucket*/
    GT_U32                                   tempAddr = 0;     /* Temporary address                */
    GT_U32                                   blockIndex=0;

    GT_BOOL     needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR  tempHwAddrHandleToBeFreed = 0;

    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndex];
    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 - GT_FAIL \n");
    }

    /* oldBucketPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    oldHwAddrHandle     = oldBucketShadowPtr->hwBucketOffsetHandle;
    memBlockBase        = lpmMemInfoPtrPtr[octetIndex]->structsBase;
    oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle);

    /* we will loop the code twice
       one for copying the bucket to swap_area_2 and one for copying it back to the partition */
    while (GT_TRUE)
    {
        if (copyToSwapArea2 == GT_TRUE)
        {
            /* create the HW data to be writen to swap_area_1 according to the oldBucketPtr */
            tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->secondSwapMemoryAddr) + memBlockBase;
        }
        else
        {
            /* Swap memory is in use , and there is a need to move and rewrite the bucket back to the partition it was */
            tempHwAddrHandle = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[bankIndex],
                                                  DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * oldMemSize,
                                                  DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
            if ((tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle == DMM_MALLOC_FAIL))
            {
                /* should never happen since we just freed this mem when calling
                   prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 - GT_FAIL \n");
            }
            else
            {
                /* keep new tempHwAddrHandle -- in case of a fail should be freed */
                needToFreeAllocationInCaseOfFail = GT_TRUE;
                tempHwAddrHandleToBeFreed = tempHwAddrHandle;


               /*  set pending flag for future need */
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                if (blockIndex!=bankIndex)
                {
                    /* we should be working on the bankIndex we got as parameter to the function */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 - GT_FAIL \n");
                }
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);
            }

            tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)) + memBlockBase;

       }

        retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                lpmMemInfoPtrPtr,
                                                                                tempAddr,
                                                                                oldBucketShadowPtr,
                                                                                needToFreeAllocationInCaseOfFail,
                                                                                tempHwAddrHandleToBeFreed,
                                                                                parentWriteFuncPtr,
                                                                                PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E);
        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 - GT_FAIL \n");
        }
        if(copyToSwapArea2==GT_TRUE)
        {
            /* ok since the bucket is updated in the HW, we can now update the shadow */
            oldBucketShadowPtr->hwBucketOffsetHandle = shadowPtr->secondSwapMemoryAddr;
        }
        else
        {
           /* ok since the bucket is updated in the HW, we can now update the shadow */
            oldBucketShadowPtr->hwBucketOffsetHandle = tempHwAddrHandle;

            SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle, oldHwProtocol);
            SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwBucketOffsetHandle, oldBucketShadowPtr);
        }

        retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                lpmMemInfoPtrPtr,
                                                                                tempAddr,
                                                                                oldBucketShadowPtr,
                                                                                needToFreeAllocationInCaseOfFail,
                                                                                tempHwAddrHandleToBeFreed,
                                                                                parentWriteFuncPtr,
                                                                                PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E);
        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 - GT_FAIL \n");
        }
        if(copyToSwapArea2==GT_TRUE)
        {
            /* now it's ok to free this bucket's old memory */
            retVal = prvCpssDxChLpmRamMngShrinkFreeOldBucket(shadowPtr,
                                                             bankIndex,
                                                             oldHwAddrHandle,
                                                             oldBucketShadowPtr);
            if (retVal!=GT_OK)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2 - GT_FAIL \n");
            }
        }

        if (copyToSwapArea2==GT_TRUE)
        {
            copyToSwapArea2 = GT_FALSE;
        }
        else
        {
            /* finish to shrink one block element */
            break;
        }
    }
    return GT_OK;
}
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
)
{
    GT_STATUS                                retVal=GT_OK;
    GT_DMM_PARTITION                         *partition;
    GT_UINTPTR                               oldHwAddrHandle=0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT     oldHwProtocol;/* the protocol of the bucket we are moving */
    GT_UINTPTR                               tempHwAddrHandle=0;

    GT_U32                                   memBlockBase = 0;/* the base address (offset) in the RAM for bucket*/
    GT_U32                                   tempAddr = 0;     /* Temporary address                */
    GT_U32                                   blockIndex=0;
    GT_U32                                   newLocationBlockSize;

    GT_BOOL     needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR  tempHwAddrHandleToBeFreed = 0;

    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndex];
    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlock - GT_FAIL \n");
    }

    /* oldBucketPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    oldHwAddrHandle     = oldBucketShadowPtr->hwBucketOffsetHandle;
    memBlockBase        = lpmMemInfoPtrPtr[octetIndex]->structsBase;
    oldHwProtocol       = GET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle);

    /* double check that the element we want to move have a free space it will fit
       before its current location */
    if (((GT_DMM_BLOCK *)oldBucketShadowPtr->hwBucketOffsetHandle)->prevByAddr != NULL)
    {
        newLocationBlockSize =  SIZE_IN_WORDS(((GT_DMM_BLOCK *)oldBucketShadowPtr->hwBucketOffsetHandle)->prevByAddr);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlock - GT_FAIL \n");
    }

    if((DMM_BLOCK_STATUS(((GT_DMM_BLOCK *)oldBucketShadowPtr->hwBucketOffsetHandle)->prevByAddr) == DMM_BLOCK_FREE) &&
       ((OFFSET_IN_WORDS(((GT_DMM_BLOCK *)oldBucketShadowPtr->hwBucketOffsetHandle)->prevByAddr) + newLocationBlockSize) <=
         OFFSET_IN_WORDS(oldBucketShadowPtr->hwBucketOffsetHandle)) &&
       (oldMemSize<=newLocationBlockSize))
    {

        retVal = prvCpssDmmAllocateByPtr(shadowPtr->lpmRamStructsMemPoolPtr[bankIndex],
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS*(OFFSET_IN_WORDS(((GT_DMM_BLOCK *)oldBucketShadowPtr->hwBucketOffsetHandle)->prevByAddr)),
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS* oldMemSize,
                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS,
                                 &tempHwAddrHandle);

        if ((tempHwAddrHandle == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle == DMM_MALLOC_FAIL))
        {
            /* should never happen since we just freed this mem when calling
               prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlock - GT_FAIL \n");
        }
        else
        {
            /* keep new tempHwAddrHandle -- in case of a fail should be freed */
            needToFreeAllocationInCaseOfFail = GT_TRUE;
            tempHwAddrHandleToBeFreed = tempHwAddrHandle;

           /*  set pending flag for future need */
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            if (blockIndex!=bankIndex)
            {
                /* we should be working on the bankIndex we got as parameter to the function */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlock - GT_FAIL \n");
            }
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle);
        }

        tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle)) + memBlockBase;

        retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                lpmMemInfoPtrPtr,
                                                                                tempAddr,
                                                                                oldBucketShadowPtr,
                                                                                needToFreeAllocationInCaseOfFail,
                                                                                tempHwAddrHandleToBeFreed,
                                                                                parentWriteFuncPtr,
                                                                                PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E);
        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlock - GT_FAIL \n");
        }

        /* ok since the bucket is updated in the HW, we can now update the shadow */
        oldBucketShadowPtr->hwBucketOffsetHandle = tempHwAddrHandle;

        SET_DMM_BLOCK_PROTOCOL(oldBucketShadowPtr->hwBucketOffsetHandle, oldHwProtocol);
        SET_DMM_BUCKET_SW_ADDRESS(oldBucketShadowPtr->hwBucketOffsetHandle, oldBucketShadowPtr);

        retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                lpmMemInfoPtrPtr,
                                                                                tempAddr,
                                                                                oldBucketShadowPtr,
                                                                                needToFreeAllocationInCaseOfFail,
                                                                                tempHwAddrHandleToBeFreed,
                                                                                parentWriteFuncPtr,
                                                                                PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E);
        if (retVal!=GT_OK)
        {
           /* in case of a fail any allocations done in previous stages was already
              free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlock - GT_FAIL \n");
        }

        /* now it's ok to free this bucket's old memory */
        retVal = prvCpssDxChLpmRamMngShrinkFreeOldBucket(shadowPtr,
                                                         bankIndex,
                                                         oldHwAddrHandle,
                                                         oldBucketShadowPtr);
        if (retVal!=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlock - GT_FAIL \n");
        }
    }
    else
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkOneBlock - GT_FAIL \n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamMngShrinkBank function
* @endinternal
*
* @brief   Shrink the partition
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
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
)
{
    GT_STATUS           retVal = GT_OK;
    GT_DMM_PARTITION    *partition;

    GT_DMM_BLOCK        *firstFreeBlock;
    GT_DMM_BLOCK        *firstOccupiedBlockAfterFirstFreeBlock;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *oldBucketShadowPtr=NULL;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *oldBucketShadowToMovePtr=NULL;
    GT_U32                                   oldMemSize=0,oldMemSizeToMove=0;
    GT_BOOL                                  oldBucketShouldBeMoved=GT_FALSE;/* flag to specify if we need to move the old bucket */

    /* oldBucketShadowPtr will be used to build the HW from SW shadow and for
       updated pointers when we move the HW bucket location */
    if (oldHwHandleAddr!=0)
    {
        oldBucketShadowPtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(oldHwHandleAddr);/* old bucket representation*/
        oldMemSize         = SIZE_IN_WORDS(oldBucketShadowPtr->hwBucketOffsetHandle);
        oldBucketShouldBeMoved=GT_TRUE;
    }
    else
    {
        /* we need to do shrink for a new block, so no old pointers to update
           and no old memory location to be moved */
        oldBucketShouldBeMoved=GT_FALSE;
    }

    partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndex];
    /* if the partition is empty --> the block is empty */
    if(partition->allocatedBytes == 0)
    {
        /* nothing to shrink
           should never happen since this function should be called after
           checking shrink operation may be useful for defrag */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkBank - GT_FAIL \n");
    }

    /* if useSwapArea is GT_TRUE this means we need to copy old bucket
       to swap_area_1, since we need its space for the shrink operation */
    if ((useSwapArea==GT_TRUE)&&(oldBucketShouldBeMoved==GT_TRUE))
    {
        /* 1. copy oldHwHandleAddr to the swap_area_1 */
        /* 2. update pointers */
        /* 3. free space of oldHwHandleAddr */
        retVal = prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1(shadowPtr,
                                                              lpmMemInfoPtrPtr,
                                                              octetIndex,
                                                              bankIndex,
                                                              oldBucketShadowPtr,
                                                              oldMemSize,
                                                              PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E,
                                                              parentWriteFuncPtr);
        if (retVal!=GT_OK)
        {
            /* should never happen since swap area is a free memory that can be used */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkBank - GT_FAIL \n");
        }

        /* 4. continue to next steps of shrink */
    }

    /* find the first free block in the partition */
    firstFreeBlock = partition->pointedFirstFreeBlock;
    if((firstFreeBlock == NULL)||(DMM_BLOCK_STATUS(firstFreeBlock)!=DMM_BLOCK_FREE))
    {
         /* nothing to shrink
            should never happen since this function should be called after
            checking shrink operation may be useful for defrag */

        /* revert what was done above

           if useSwapArea is GT_TRUE this means we need to copy old bucket
           from swap_area_1 to its original partition */
        if ((useSwapArea==GT_TRUE)&&(oldBucketShouldBeMoved==GT_TRUE))
        {
            retVal = prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1(shadowPtr,
                                                                    lpmMemInfoPtrPtr,
                                                                    octetIndex,
                                                                    bankIndex,
                                                                    oldBucketShadowPtr,
                                                                    oldMemSize,
                                                                    PRV_CPSS_DXCH_LPM_RAM_COPY_FROM_FIRST_SWAP_AREA_E,
                                                                    parentWriteFuncPtr);
            if (retVal!=GT_OK)
            {
                /* should never happen since swap area is a free memory that can be used */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkBank - GT_FAIL \n");
            }
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkBank - GT_FAIL \n");
    }

    /* go over the list and copy allocated block to the free block
       this will shrink all occupied blocks down and all free blocks up */
    while(GT_TRUE)
    {
        /* check if now we have big enough free space to add new bucket */
        if (newBucketSize<=SIZE_IN_WORDS(firstFreeBlock))
        {
            /* we found a free space to fit newBucketSize -- stop the shrink operation */

            /* we shrink the bank but the new allocation of space should be done
               when getting out of the function
               1. write new bucket to head of free space
               2. update pointers
               3. update free list */
            break;
        }

        /* after a free block we will always have an occupied block,
           otherwise there is a merged between 2 free blocks */
        firstOccupiedBlockAfterFirstFreeBlock = firstFreeBlock->nextByAddr;
        if (firstOccupiedBlockAfterFirstFreeBlock==NULL)
        {
            /* if no next bucket, bank is fully shrinked and failed to add a bucket.
               this is considered error as we previously verified bank has enough space */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkBank - GT_FAIL \n");
        }

        /* if the size of the block we want to move is smaller or equal to the free block we have,
           then there is no problem to move the block and update the pointers */
        if (SIZE_IN_WORDS(firstOccupiedBlockAfterFirstFreeBlock)<=SIZE_IN_WORDS(firstFreeBlock))
        {
            oldBucketShadowToMovePtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(firstOccupiedBlockAfterFirstFreeBlock);/* old bucket representation*/
            if (oldBucketShadowToMovePtr!=0)
            {
                oldMemSizeToMove         = SIZE_IN_WORDS(firstOccupiedBlockAfterFirstFreeBlock);
                retVal = prvCpssDxChLpmRamMngShrinkOneBlock(shadowPtr,
                                                          lpmMemInfoPtrPtr,
                                                          octetIndex,
                                                          bankIndex,
                                                          oldBucketShadowToMovePtr,
                                                          oldMemSizeToMove,
                                                          parentWriteFuncPtr);
                if (retVal!=GT_OK)
                {
                    /* should never happen since swap area is a free memory that can be used */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkBank - GT_FAIL \n");
                }
            }
            else
            {
                /* this case can happen in case we are working on group prefix
                   stage 1 - source prefix memory allocation was done but not updated in HW yet
                   stage 2 - group prefix memory allocation need to move the source prefix from stage 1,
                   this case should be prevented - no move should be done */
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"error in prvCpssDxChLpmRamMngShrinkBank - GT_OUT_OF_CPU_MEM \n");

            }
        }
        else
        {
            /* we need to copy the firstOccupiedBlock to swap_area_2
               to prevent the bucket from overwrite itself  */

            /* 1. copy firstOccupiedBlockAfterFirstFreeBlock to swap_area_2 */
            /* 2. update pointers to swap_area_2 */
            /* 3. free firstOccupiedBlockAfterFirstFreeBlock original location */
            /* 3. write firstOccupiedBlockAfterFirstFreeBlock to firstFreeBlock */
            /* 4. update the pointers to the new location */
            oldBucketShadowToMovePtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(firstOccupiedBlockAfterFirstFreeBlock);/* old bucket representation*/
            if (oldBucketShadowToMovePtr!=0)
            {
                oldMemSizeToMove         = SIZE_IN_WORDS(firstOccupiedBlockAfterFirstFreeBlock);
                retVal = prvCpssDxChLpmRamMngShrinkOneBlockUsingSwapArea2(shadowPtr,
                                                                          lpmMemInfoPtrPtr,
                                                                          octetIndex,
                                                                          bankIndex,
                                                                          oldBucketShadowToMovePtr,
                                                                          oldMemSizeToMove,
                                                                          parentWriteFuncPtr);
                if (retVal!=GT_OK)
                {
                    /* should never happen since swap area is a free memory that can be used */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkBank - GT_FAIL \n");
                }
            }
            else
            {
                /* this case can happen in case we are working on group prefix
                   stage 1 - source prefix memory allocation was done but not updated in HW yet
                   stage 2 - group prefix memory allocation need to move the source prefix from stage 1,
                   this case should be prevented - no move should be done */
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"error in prvCpssDxChLpmRamMngShrinkBank - GT_OUT_OF_CPU_MEM \n");

            }
        }

         /* find the new first free block in the partition */
         firstFreeBlock = partition->pointedFirstFreeBlock;
    }


    /* if useSwapArea is GT_TRUE this means we need to copy old bucket
       from swap_area_1 to its original partition */
    if ((useSwapArea==GT_TRUE)&&(oldBucketShouldBeMoved==GT_TRUE))
    {
        retVal = prvCpssDxChLpmRamMngShrinkCopyToOrFromSwapArea1(shadowPtr,
                                                                lpmMemInfoPtrPtr,
                                                                octetIndex,
                                                                bankIndex,
                                                                oldBucketShadowPtr,
                                                                oldMemSize,
                                                                PRV_CPSS_DXCH_LPM_RAM_COPY_FROM_FIRST_SWAP_AREA_E,
                                                                parentWriteFuncPtr);
        if (retVal!=GT_OK)
        {
            /* should never happen since swap area is a free memory that can be used */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngShrinkBank - GT_FAIL \n");
        }
    }

    return GT_OK;
}

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
)
{
    GT_STATUS retVal=GT_OK;
    /* Merge source bank info */
    GT_BOOL mergeBank=GT_FALSE;  /* did we found a bank to merge */
    GT_U32  mergeSourceOctet = 0;/* the octet mapped to the bank we are going to merge */
    GT_U32  mergeSourceBank = 0; /* the bank index we are going to merge */
    GT_U32  extraLines = 0;      /* number of extra lines not used by any
                                    of the banks related to mergeSourceOctet */
    GT_U32 maxNumOfOctets = 0;   /* max number of octets according to the protocol */
    GT_U32 octetInd;             /* index to run over all octets */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *tempLpmMemInfoPtr; /* use for going over the list of
                                                                   blocks mapped to a specific octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *secondTempLpmMemInfoPtr;
    GT_U32 sumOfSourceLines; /* calculation of total lines that will have relocated -
                                only for the specific protocol buckets */
    GT_U32 bestSumOfSourceLines=0; /* best result calculation of total lines that will have relocated -
                                      only for the specific protocol buckets */

    GT_U32 sumOfFreeLinesAtTargetBanks=0; /* calculation of total free lines in the bank we want to merge */
    GT_U32 temp;

    GT_U32 blockIndex=0;      /* the block index of a given bank we are working on */
    GT_BOOL allUpdatedInHwFlag;

    /* Scan all other octets that are not the octetIndex we got as parameter to this function
       Try to find the best single bank to release.
       We scan all banks from all octets no matter what - we want to verify we take the best.
       The "best" is considered the one that has most extra free lines at its target banks
       - on top the ones required to hold its source lines.
       Note that doesn't necessarily the one with the least lines to move. */

    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* scan all other octets of the protocol */
    for (octetInd=0; octetInd < maxNumOfOctets; octetInd++)
    {
        if (octetInd==octetIndex)
        {
            /* no need to check the octetIndex we got as parameter to this function */
            continue;
        }
        /* skip octets with less than 2 banks - no potential to merge */
        if (lpmMemInfoPtrPtr[octetInd]->nextMemInfoPtr==NULL)
        {
            continue;
        }
        tempLpmMemInfoPtr = lpmMemInfoPtrPtr[octetInd];
        /* try to find the bank with the best chance to succeed in releasing
           bank 0 can not be merged to other banks since it is a special bank holding the swap areas */
        while ((tempLpmMemInfoPtr!=NULL)&&(tempLpmMemInfoPtr->ramIndex!=0))
        {
            /* reset values each new loop */
            sumOfFreeLinesAtTargetBanks=0;
            blockIndex = tempLpmMemInfoPtr->ramIndex;

            /* First check that the potential bank to merge do not hold blocks that were not
               yet updated in HW, such in case that we are looking for a place for MC
               group but the source was not yet updated in HW */
            retVal = prvCpssDmmAllPartitionIsUpdatedInHw(tempLpmMemInfoPtr->structsMemPool,&allUpdatedInHwFlag);
            if (retVal!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDmmAllPartitionIsUpdatedInHw - GT_FAIL \n");
            }

            if (allUpdatedInHwFlag == GT_FALSE)
            {
                tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                continue;
            }

            /* total lines that will have relocated - only specific protocol buckets

               at this point the counters we have in protocolCountersPerBlockArr do
               not show the correct picture.
               In some cases prvCpssDxChLpmRamUpdateBlockUsageCounters was not
               called yet so we need to take into consideration also the pending counters */
            switch (protocol)
            {
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                sumOfSourceLines = (shadowPtr->protocolCountersPerBlockArr[tempLpmMemInfoPtr->ramIndex].sumOfIpv4Counters+
                                    shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfIncUpdates-
                                    shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfDecUpdates);
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                sumOfSourceLines = (shadowPtr->protocolCountersPerBlockArr[tempLpmMemInfoPtr->ramIndex].sumOfIpv6Counters+
                                 shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfIncUpdates-
                                 shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfDecUpdates);
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                sumOfSourceLines = (shadowPtr->protocolCountersPerBlockArr[tempLpmMemInfoPtr->ramIndex].sumOfFcoeCounters+
                                 shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfIncUpdates-
                                 shadowPtr->pendingBlockToUpdateArr[tempLpmMemInfoPtr->ramIndex].numOfDecUpdates);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"error in illegal protocol - GT_BAD_PARAM \n");

            }

            /* check that relocating the buckets will allow to add the new one instead */
            if (sumOfSourceLines<newBucketSize)
            {
                tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                continue;
            }

            if(mergeBank==GT_FALSE)/* we still did not find a potential bank to merge */
            {
                bestSumOfSourceLines = sumOfSourceLines;
            }
            /* after getting a potential bank to be merged, we check that there is enough
               free space in the rest of the banks to merge it to.

               total free lines at target banks */
            secondTempLpmMemInfoPtr = lpmMemInfoPtrPtr[octetInd];
            while (secondTempLpmMemInfoPtr!=NULL)
            {
                /* skip the bank we found as potential for merge */
                if (secondTempLpmMemInfoPtr->structsMemPool==tempLpmMemInfoPtr->structsMemPool)
                {
                    secondTempLpmMemInfoPtr=secondTempLpmMemInfoPtr->nextMemInfoPtr;
                    continue;
                }

                /* total free lines in the target banks

                   at this point the counters we have in protocolCountersPerBlockArr do
                   not show the correct picture.
                   In some cases prvCpssDxChLpmRamUpdateBlockUsageCounters was not
                   called yet so we need to take into consideration also the pending counters
                   example: add new prefix -
                   octet 0 was taken from bank3 -> numOfIncUpdates=4
                   octet 1 has no place so we go and check if a merge can be done
                   the merge checks bank3 for allocated and free space.
                   if we use the counters we will get a wrong value since numOfIncUpdates=4 was
                   not update in protocolCountersPerBlockArr.
                   the update is done in the end of the add operation only after all went well */
                temp = shadowPtr->lpmRamBlocksSizeArrayPtr[secondTempLpmMemInfoPtr->ramIndex]-
                                (shadowPtr->protocolCountersPerBlockArr[secondTempLpmMemInfoPtr->ramIndex].sumOfIpv4Counters+
                                 shadowPtr->protocolCountersPerBlockArr[secondTempLpmMemInfoPtr->ramIndex].sumOfIpv6Counters+
                                 shadowPtr->protocolCountersPerBlockArr[secondTempLpmMemInfoPtr->ramIndex].sumOfFcoeCounters+
                                 shadowPtr->pendingBlockToUpdateArr[secondTempLpmMemInfoPtr->ramIndex].numOfIncUpdates-
                                 shadowPtr->pendingBlockToUpdateArr[secondTempLpmMemInfoPtr->ramIndex].numOfDecUpdates);

                sumOfFreeLinesAtTargetBanks += temp;

                secondTempLpmMemInfoPtr=secondTempLpmMemInfoPtr->nextMemInfoPtr;
            }
            /* check enough free space */
            if (sumOfFreeLinesAtTargetBanks<sumOfSourceLines)
            {
                tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
                continue;
            }

            /* check if this is the best bank to relocate so far or first one we found */
            if (((sumOfFreeLinesAtTargetBanks-sumOfSourceLines)>extraLines)||
                (mergeBank==GT_FALSE)/* first potential bank found */ ||
                (((sumOfFreeLinesAtTargetBanks-sumOfSourceLines)==extraLines) &&
                 (sumOfSourceLines < bestSumOfSourceLines)))
            {
                mergeBank=GT_TRUE;           /* we found a bank to merge */
                mergeSourceOctet = octetInd; /* the octet using the bank we are going to merge is the current octet */
                mergeSourceBank = blockIndex;/* the bank index we are going to merge is the current bank */
                /* number of extra lines not used by any of the banks related to mergeSourceOctet */
                extraLines = sumOfFreeLinesAtTargetBanks-sumOfSourceLines;
                bestSumOfSourceLines = sumOfSourceLines;
            }
            tempLpmMemInfoPtr = tempLpmMemInfoPtr->nextMemInfoPtr;
        }
    }

    *mergeBankCanBeFoundPtr = mergeBank;
    *bankIndexForMergePtr   = mergeSourceBank;
    *octetIndexForMergePtr  = mergeSourceOctet;

    return GT_OK;
}

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
)
{
    GT_STATUS                               retVal=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *tempLpmMemInfoPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketShadowToMergePtr;/* bucket to be merged in SW representation*/
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    bucketShadowToMergeProtocol;/* the protocol of the bucket we are moving */
    GT_U32                                  memSizeToMerge;         /* bucket size to be merged*/
    GT_U32                                  memBlockBase=0;
    GT_UINTPTR                              newHwBucketOffsetHandle = 0;
    GT_BOOL                                 needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR                              tempHwAddrHandleToBeFreed = 0;
    GT_U32                                  blockIndex;
    GT_U32                                  tempAddr = 0;     /* Temporary address */


    bucketShadowToMergePtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(blockHandleAddrForMerge);
    memSizeToMerge         = SIZE_IN_WORDS(blockHandleAddrForMerge);
    bucketShadowToMergeProtocol   = GET_DMM_BLOCK_PROTOCOL(bucketShadowToMergePtr->hwBucketOffsetHandle);

    tempLpmMemInfoPtr = lpmMemInfoPtrPtr[octetIndexForMerge];
    *bucketRelocatedPtr=GT_FALSE;

    /* try to relocate the naive way - scan the list by the order it appears */
    while ((tempLpmMemInfoPtr!=NULL)&&(newHwBucketOffsetHandle==0))
    {
        /* skip the bank that we are going to release */
        if (tempLpmMemInfoPtr->structsMemPool == shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge])
        {
            tempLpmMemInfoPtr=tempLpmMemInfoPtr->nextMemInfoPtr;
            continue;/* try next bank */
        }
        /* look for the smallest (which is big enough) free space
           in bank to hold relocated bucket - it is done by the DMM allocation algorithm */
         memBlockBase = tempLpmMemInfoPtr->structsBase;
         newHwBucketOffsetHandle = prvCpssDmmAllocate(tempLpmMemInfoPtr->structsMemPool,
                                                DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSizeToMerge,
                                                DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
         if ((newHwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND)||(newHwBucketOffsetHandle == DMM_MALLOC_FAIL))
         {
             /* continue looking in next bank - we did not find what we needed in current bank */
             newHwBucketOffsetHandle = 0;
         }
         tempLpmMemInfoPtr=tempLpmMemInfoPtr->nextMemInfoPtr;
    }

    /* if free space was found then we move the bucket to
       its new location and update all related pointers */
    if(newHwBucketOffsetHandle!=0)
    {
         /* keep new tempHwAddrHandle -- in case of a fail should be freed */
         needToFreeAllocationInCaseOfFail = GT_TRUE;
         tempHwAddrHandleToBeFreed = newHwBucketOffsetHandle;

        /*  set pending flag for future need */
         blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(newHwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
         shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
         shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                 PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(newHwBucketOffsetHandle);

         tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(newHwBucketOffsetHandle)) + memBlockBase;

         retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                 lpmMemInfoPtrPtr,
                                                                                 tempAddr,
                                                                                 bucketShadowToMergePtr,
                                                                                 needToFreeAllocationInCaseOfFail,
                                                                                 tempHwAddrHandleToBeFreed,
                                                                                 parentWriteFuncPtr,
                                                                                 PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E);
         if (retVal!=GT_OK)
         {
            /* in case of a fail any allocations done in previous stages was already
               free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocation - GT_FAIL \n");
         }

         /* ok since the bucket is updated in the HW, we can now update the shadow */
         bucketShadowToMergePtr->hwBucketOffsetHandle = newHwBucketOffsetHandle;

         SET_DMM_BLOCK_PROTOCOL(bucketShadowToMergePtr->hwBucketOffsetHandle, bucketShadowToMergeProtocol);
         SET_DMM_BUCKET_SW_ADDRESS(bucketShadowToMergePtr->hwBucketOffsetHandle, bucketShadowToMergePtr);

         retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                 lpmMemInfoPtrPtr,
                                                                                 tempAddr,
                                                                                 bucketShadowToMergePtr,
                                                                                 needToFreeAllocationInCaseOfFail,
                                                                                 tempHwAddrHandleToBeFreed,
                                                                                 parentWriteFuncPtr,
                                                                                 PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E);
         if (retVal!=GT_OK)
         {
            /* in case of a fail any allocations done in previous stages was already
               free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocation - GT_FAIL \n");
         }

         /* now it's ok to free this bucket's old memory */
         retVal = prvCpssDxChLpmRamMngShrinkFreeOldBucket(shadowPtr,
                                                          bankIndexForMerge,
                                                          blockHandleAddrForMerge,
                                                          bucketShadowToMergePtr);
         if (retVal!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocation - GT_FAIL \n");
         }

         *bucketRelocatedPtr=GT_TRUE;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS                               retVal=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *tempLpmMemInfoPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketShadowToMergePtr;/* bucket to be merged in SW representation*/
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    bucketShadowToMergeProtocol;/* the protocol of the bucket we are moving */
    GT_U32                                  memSizeToMerge;         /* bucket size to be merged*/
    GT_U32                                  memBlockBase=0;
    GT_UINTPTR                              newHwBucketOffsetHandle = 0;
    GT_U32                                  bankToShrink=0;
    GT_U32                                  octetToShrink=0;
    GT_BOOL                                 useSwapArea = GT_FALSE;
    GT_U32                                  sumOfFreeBlockAfterShrink=0;
    GT_BOOL                                 needToFreeAllocationInCaseOfFail = GT_FALSE;
    GT_UINTPTR                              tempHwAddrHandleToBeFreed = 0;
    GT_U32                                  tempAddr = 0;     /* Temporary address                */
    GT_U32                                  blockIndex=0;


    bucketShadowToMergePtr = (PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *)GET_DMM_BUCKET_SW_ADDRESS(blockHandleAddrForMerge);
    memSizeToMerge         = SIZE_IN_WORDS(blockHandleAddrForMerge);
    bucketShadowToMergeProtocol   = GET_DMM_BLOCK_PROTOCOL(bucketShadowToMergePtr->hwBucketOffsetHandle);

    tempLpmMemInfoPtr = lpmMemInfoPtrPtr[octetIndexForMerge];
    *bucketRelocatedPtr=GT_FALSE;

    /* try to relocate the naive way - scan the list by the order it appears */
    while ((tempLpmMemInfoPtr!=NULL)&&(newHwBucketOffsetHandle==0))
    {
        /* skip the bank that we are going to release */
        if (tempLpmMemInfoPtr->structsMemPool == shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge])
        {
            tempLpmMemInfoPtr=tempLpmMemInfoPtr->nextMemInfoPtr;
            continue;/* try next bank */
        }
        /* look for the smallest (which is big enough) free space
           in bank to hold relocated bucket - it is done by the DMM allocation algorithm
           in the call to prvCpssDxChLpmRamMngMoveBlockToNewLocation we already found that a simple allocation
           will not succeed so now we need to shrink the bank before allocation */

        /* check if shrink can be usefull */
        sumOfFreeBlockAfterShrink = prvCpssDmmCheckIfShrinkOperationUsefulForDefrag(tempLpmMemInfoPtr->structsMemPool,
                                                                                    memSizeToMerge,
                                                                                    0, /* oldBlockSize = 0, we do not have any old block to move */
                                                                                    &useSwapArea);
        if (sumOfFreeBlockAfterShrink>0)
        {
            /* shrink the Bank we found in previous phase */
            bankToShrink = tempLpmMemInfoPtr->ramIndex;
            octetToShrink = octetIndexForMerge;
            retVal = prvCpssDxChLpmRamMngShrinkBank(shadowPtr,
                                                    lpmMemInfoPtrPtr,
                                                    octetToShrink,
                                                    bankToShrink,
                                                    0, /* we need to do shrink for a new block, so no old pointers to update
                                                          and no old memory location to be moved */
                                                    memSizeToMerge,
                                                    GT_FALSE,
                                                    parentWriteFuncPtr);
            if (retVal!=GT_OK)
            {
                break;
            }

            /* try to alloctae the memory */
             memBlockBase = tempLpmMemInfoPtr->structsBase;
             newHwBucketOffsetHandle = prvCpssDmmAllocate(tempLpmMemInfoPtr->structsMemPool,
                                                    DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSizeToMerge,
                                                    DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS);
             if ((newHwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND)||(newHwBucketOffsetHandle == DMM_MALLOC_FAIL))
             {
                 /* illegal case, since we checked that shrink should work for moving the block with size = memSizeToMerge */
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocationWithShrink - GT_FAIL \n");
             }
        }
        /* continue looking for potential bank to shrink */
        tempLpmMemInfoPtr=tempLpmMemInfoPtr->nextMemInfoPtr;
    }

    /* if free space was found then we move the bucket to
       its new location and update all related pointers */
    if(newHwBucketOffsetHandle!=0)
    {
         /* keep new tempHwAddrHandle -- in case of a fail should be freed */
         needToFreeAllocationInCaseOfFail = GT_TRUE;
         tempHwAddrHandleToBeFreed = newHwBucketOffsetHandle;

        /*  set pending flag for future need */
         blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(newHwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
         shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc = GT_TRUE;
         shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                 PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(newHwBucketOffsetHandle);

         tempAddr = (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(newHwBucketOffsetHandle)) + memBlockBase;

         retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                 lpmMemInfoPtrPtr,
                                                                                 tempAddr,
                                                                                 bucketShadowToMergePtr,
                                                                                 needToFreeAllocationInCaseOfFail,
                                                                                 tempHwAddrHandleToBeFreed,
                                                                                 parentWriteFuncPtr,
                                                                                 PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E);
         if (retVal!=GT_OK)
         {
            /* in case of a fail any allocations done in previous stages was already
               free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocation - GT_FAIL \n");
         }

         /* ok since the bucket is updated in the HW, we can now update the shadow */
         bucketShadowToMergePtr->hwBucketOffsetHandle = newHwBucketOffsetHandle;

         SET_DMM_BLOCK_PROTOCOL(bucketShadowToMergePtr->hwBucketOffsetHandle, bucketShadowToMergeProtocol);
         SET_DMM_BUCKET_SW_ADDRESS(bucketShadowToMergePtr->hwBucketOffsetHandle, bucketShadowToMergePtr);

         retVal = prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2(shadowPtr,
                                                                                 lpmMemInfoPtrPtr,
                                                                                 tempAddr,
                                                                                 bucketShadowToMergePtr,
                                                                                 needToFreeAllocationInCaseOfFail,
                                                                                 tempHwAddrHandleToBeFreed,
                                                                                 parentWriteFuncPtr,
                                                                                 PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E);
         if (retVal!=GT_OK)
         {
            /* in case of a fail any allocations done in previous stages was already
               free in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocation - GT_FAIL \n");
         }

         /* now it's ok to free this bucket's old memory */
         retVal = prvCpssDxChLpmRamMngShrinkFreeOldBucket(shadowPtr,
                                                          bankIndexForMerge,
                                                          blockHandleAddrForMerge,
                                                          bucketShadowToMergePtr);
         if (retVal!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMoveBlockToNewLocation - GT_FAIL \n");
         }

         *bucketRelocatedPtr=GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamMngMergeBank function
* @endinternal
*
* @brief   Merge one bank to the rest of the banks mapped to a given octet
*
* @param[in] shadowPtr                - (pointer to) the shadow information
* @param[in] lpmMemInfoPtrPtr         (pointer to) the lpm memory linked list holding
*                                      all blocked mapped to this octetIndex
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
)
{
     GT_STATUS                          retVal = GT_OK;
     GT_STATUS                          retVal1 = GT_OK;
     GT_DMM_PARTITION                   *partition;
     GT_UINTPTR                         partitionId;
     GT_U32                             relocatedBucketSize=0;  /* the size of the buket we are going to move */
     GT_DMM_SORTED_PARTITION_ELEMENT    *tempElemList;          /* temporary list to be used when running over
                                                                   a list of buckets to be moved */
     GT_BOOL                            bucketRelocated = GT_FALSE;/* flag specify success of relocate operation */
     PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

     PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
     PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempElemToFreePtr;   /* temp pointer used for free operation */

    /* Create a sorted list of all buckets at the bank we need to release (only of relevant protocol),
       motivation is that later on when we will try to add them to the other banks one by one starting
       from the biggest to smallest - this will increase chances to success.*/
     partitionId = shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge];
     partition = (GT_DMM_PARTITION *)shadowPtr->lpmRamStructsMemPoolPtr[bankIndexForMerge];

     retVal = prvCpssDmmSortPartition(partitionId,(GT_U32)protocol);
     if (retVal!=GT_OK)
     {
         prvCpssDmmFreeSortedPartitionArray(partitionId);
         return retVal;
     }

     /* save values pendingBlockToUpdateArr before the merge */
     cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
     cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
     /* reset shadowPtr->pendingBlockToUpdateArr */
     cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

     /* Run all over the buckets that need to be relocated from biggest to smallest */
     for (relocatedBucketSize=DMM_MAXIMAL_BLOCK_SIZE_ALLOWED;relocatedBucketSize>0;relocatedBucketSize--)
     {
         /*check if we have a bank of relocatedBucketSize to be moved */
         tempElemList = partition->tableOfSortedUsedBlocksPointers[relocatedBucketSize];
         if (tempElemList!=NULL)
         {
             /* go over all element of relocatedBucketSize and move them */
             while (tempElemList!=NULL)
             {
                 /* move the element to its new location */
                 retVal = prvCpssDxChLpmRamMngMoveBlockToNewLocation(shadowPtr,
                                                                    lpmMemInfoPtrPtr,
                                                                    octetIndexForMerge,
                                                                    bankIndexForMerge,
                                                                    (GT_UINTPTR)tempElemList->blockElementPtr,/* the block to be moved */
                                                                    parentWriteFuncPtr,
                                                                    &bucketRelocated);
                 if (retVal!=GT_OK)
                 {
                     prvCpssDmmFreeSortedPartitionArray(partitionId);
                     /* in case of merge we should instantly update the block usage counters
                       since it is a permanent operation that will not be reverted in case
                       of a fail.
                       For example in MC case we need the counters to be updated corectly after
                       the Src addition and not after Src+Grp finish succesfuly */
                     prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                        shadowPtr->pendingBlockToUpdateArr,
                                                                        shadowPtr->protocolCountersPerBlockArr,
                                                                        shadowPtr->pendingBlockToUpdateArr,
                                                                        protocol,
                                                                        shadowPtr->numOfLpmMemories);

                     /* put back the values kept in pendingBlockToUpdateArr before the merge */
                     cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
                     CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
                 }

                 if (bucketRelocated==GT_TRUE)
                 {
                     /* in case of merge we should instantly update the block usage counters
                       since it is a permanent operation that will not be reverted in case
                       of a fail.*/
                     retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                        shadowPtr->pendingBlockToUpdateArr,
                                                                        shadowPtr->protocolCountersPerBlockArr,
                                                                        shadowPtr->pendingBlockToUpdateArr,
                                                                        protocol,
                                                                        shadowPtr->numOfLpmMemories);
                     if (retVal1!=GT_OK)
                     {
                         /* put back the values kept in pendingBlockToUpdateArr before the merge */
                         cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
                         CPSS_LOG_ERROR_AND_RETURN_MAC(retVal1,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
                     }

                     /* go to next element with (size = relocatedBucketSize) to be moved */
                     tempElemList = tempElemList->nextSortedElemPtr;
                 }
                 else/* bucketRelocated==GT_FALSE*/
                 {
                     /* if no free space try shrinking one of the banks.
                        we do it in second loop as we want to give naive procedure
                        the chance to scan all banks before we try shrinking */
                     retVal = prvCpssDxChLpmRamMngMoveBlockToNewLocationWithShrink(
                                                                    shadowPtr,
                                                                    lpmMemInfoPtrPtr,
                                                                    octetIndexForMerge,
                                                                    bankIndexForMerge,
                                                                    (GT_UINTPTR)tempElemList->blockElementPtr,/* the block to be moved */
                                                                    parentWriteFuncPtr,
                                                                    &bucketRelocated);
                     if (retVal!=GT_OK)
                     {
                         /* if cannot relocate bucket even with shrink it means the merge failed */
                         prvCpssDmmFreeSortedPartitionArray(partitionId);

                         /* in case of merge we should instantly update the block usage counters
                           since it is a permanent operation that will not be reverted in case
                           of a fail.
                           For example in MC case we need the counters to be updated corectly after
                           the Src addition and not after Src+Grp finish succesfuly */
                         prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                            shadowPtr->pendingBlockToUpdateArr,
                                                                            shadowPtr->protocolCountersPerBlockArr,
                                                                            shadowPtr->pendingBlockToUpdateArr,
                                                                            protocol,
                                                                            shadowPtr->numOfLpmMemories);

                         /* put back the values kept in pendingBlockToUpdateArr before the merge */
                         cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
                         CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
                     }

                     if (bucketRelocated==GT_TRUE)
                     {
                         /* in case of merge we should instantly update the block usage counters
                           since it is a permanent operation that will not be reverted in case
                           of a fail.*/
                         retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                            shadowPtr->pendingBlockToUpdateArr,
                                                                            shadowPtr->protocolCountersPerBlockArr,
                                                                            shadowPtr->pendingBlockToUpdateArr,
                                                                            protocol,
                                                                            shadowPtr->numOfLpmMemories);
                         if (retVal1!=GT_OK)
                         {
                             /* put back the values kept in pendingBlockToUpdateArr before the merge */
                             cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
                             CPSS_LOG_ERROR_AND_RETURN_MAC(retVal1,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
                         }

                         /* go to next element with (size = relocatedBucketSize) to be moved */
                         tempElemList = tempElemList->nextSortedElemPtr;
                     }
                     else/* bucketRelocated==GT_FALSE */
                     {
                         /* even with shrink we do not succeed in relocating the block - this is a fail */
                         /* if cannot relocate bucket even with shrink it means the merge failed */
                         prvCpssDmmFreeSortedPartitionArray(partitionId);

                         /* put back the values kept in pendingBlockToUpdateArr before the merge */
                         cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
                         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
                     }
                 }
             }
         }
     }

     /* if we reached here it means the selected bank was merged to other
        banks related to the selected octet.
        Now the released bank can be used by another octet
        We should mark this bank as not used anymore by the given protocol */

     /*  clear block mapping for the given protocol */
     PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadowPtr, protocol, octetIndexForMerge, bankIndexForMerge);
     /* the blocks that was freed should stay free even if we get an error in the next phases so we update
        tempLpmRamOctetsToBlockMappingUsedForReconstractPtr to be used in case reconstruct is needed,*/
     PRV_CPSS_DXCH_TEMP_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadowPtr, protocol, octetIndexForMerge, bankIndexForMerge);

    /* if the block is not used by any protocol set it as not used */
    if(((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,bankIndexForMerge)==GT_FALSE))&&
        ((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,bankIndexForMerge)==GT_FALSE))&&
        ((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,bankIndexForMerge)==GT_FALSE)))
    {
        shadowPtr->lpmRamOctetsToBlockMappingPtr[bankIndexForMerge].isBlockUsed = GT_FALSE;
        shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr[bankIndexForMerge].isBlockUsed = GT_FALSE;
    }


    /* need to remove the bankIndexForMerge from its current location in the linked list */
    tempNextMemInfoPtr = lpmMemInfoPtrPtr[octetIndexForMerge];

    if (tempNextMemInfoPtr!=NULL)/* the list is not empty */
    {
        if(tempNextMemInfoPtr->nextMemInfoPtr==NULL)/* this is an allocation of the root */
        {
            if (tempNextMemInfoPtr->ramIndex==bankIndexForMerge)
            {
                /* just reset the values */
                tempNextMemInfoPtr->ramIndex=0;
                tempNextMemInfoPtr->structsBase=0;
                tempNextMemInfoPtr->structsMemPool=0;
            }
            else
            {
                /* should never happen or it is a scenario we haven’t thought about */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            /* the bankIndexForMerge is located as the first element in the linked list */
            if (tempNextMemInfoPtr->ramIndex==bankIndexForMerge)
            {
                /* copy the values from the next element to the root  */
                tempElemToFreePtr = tempNextMemInfoPtr->nextMemInfoPtr;

                tempNextMemInfoPtr->ramIndex=tempNextMemInfoPtr->nextMemInfoPtr->ramIndex;
                tempNextMemInfoPtr->structsBase=tempNextMemInfoPtr->nextMemInfoPtr->structsBase;
                tempNextMemInfoPtr->structsMemPool=tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool;

                tempNextMemInfoPtr->nextMemInfoPtr=tempNextMemInfoPtr->nextMemInfoPtr->nextMemInfoPtr;
                cpssOsFree(tempElemToFreePtr);
            }
            else
            {
                /* this is an allocation of an element in the linked list
                   need to free the elemen and remove it from the linked list
                   (update the father pointers) */
                while(tempNextMemInfoPtr->nextMemInfoPtr != NULL)
                {
                    if (tempNextMemInfoPtr->nextMemInfoPtr->ramIndex==bankIndexForMerge)
                    {
                        /* free the element */
                         tempElemToFreePtr = tempNextMemInfoPtr->nextMemInfoPtr;

                         tempNextMemInfoPtr->nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr->nextMemInfoPtr;
                         cpssOsFree(tempElemToFreePtr);
                         break;
                    }
                    tempNextMemInfoPtr=tempNextMemInfoPtr->nextMemInfoPtr;
                }
            }
        }
    }
    else
    {
        /* should never happen or it is a scenario we haven’t thought about */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* reset sorted partition list */
    prvCpssDmmFreeSortedPartitionArray(partitionId);

    /* in case of merge we should instantly update the block usage counters
       since it is a permanent operation that will not be reverted in case
       of a fail.
       For example in MC case we need the counters to be updated corectly after
       the Src addition and not after Src+Grp finish succesfuly */
     retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                        shadowPtr->pendingBlockToUpdateArr,
                                                        shadowPtr->protocolCountersPerBlockArr,
                                                        shadowPtr->pendingBlockToUpdateArr,
                                                        protocol,
                                                        shadowPtr->numOfLpmMemories);
     if (retVal1!=GT_OK)
     {
         /* put back the values kept in pendingBlockToUpdateArr before the merge */
         cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
         CPSS_LOG_ERROR_AND_RETURN_MAC(retVal1,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
     }

     if (retVal1!=GT_OK)
     {
         /* put back the values kept in pendingBlockToUpdateArr before the merge */
         cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
         CPSS_LOG_ERROR_AND_RETURN_MAC(retVal1,"error in prvCpssDxChLpmRamMngMergeBank - GT_FAIL \n");
     }

     /* put back the values kept in pendingBlockToUpdateArr before the merge */
    cpssOsMemCpy(shadowPtr->pendingBlockToUpdateArr,tempPendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));

    return GT_OK;
}
