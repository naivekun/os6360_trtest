/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip6LpmRamMng.c
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
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMng.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpssCommon/private/prvCpssMath.h>



static GT_STATUS updateHwRangeDataAndGonPtr
(
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC    *parentNodeDataPtr
);


extern void * cpssOsLpmMalloc
(
    IN GT_U32 size
);

extern void cpssOsLpmFree
(
    IN void* const memblock
);

GT_U8                                          startSubNodeAddress[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] =
    {START_ADDR_OF_SUBNODE_0_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_1_IN_GROUP_OF_NODES,
     START_ADDR_OF_SUBNODE_2_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_3_IN_GROUP_OF_NODES,
     START_ADDR_OF_SUBNODE_4_IN_GROUP_OF_NODES,START_ADDR_OF_SUBNODE_5_IN_GROUP_OF_NODES};
GT_U8                                           endSubNodeAddress[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] =
    {END_ADDR_OF_SUBNODE_0_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_1_IN_GROUP_OF_NODES,
     END_ADDR_OF_SUBNODE_2_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_3_IN_GROUP_OF_NODES,
     END_ADDR_OF_SUBNODE_4_IN_GROUP_OF_NODES,END_ADDR_OF_SUBNODE_5_IN_GROUP_OF_NODES};


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


/*
 * Typedef: enum LPM_BUCKET_UPDATE_MODE_ENT
 *
 * Description:
 *      Indicates what kind of update the bucket will go.
 *
 *
 * Fields:
 *      LPM_BUCKET_UPDATE_NONE_E  - updates nothing.
 *      LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E - updates the HW and mem alloc new
 *                                            memory if needed.
 *      LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E - updates the HW and resize
 *                                             bucket's memory if needed
 *                                            (bulk delete operation).
 *      LPM_BUCKET_UPDATE_MEM_ONLY_E - updates mem alloc only.
 *      LPM_BUCKET_UPDATE_SHADOW_ONLY_E - updates the bucket tree shadow only
 *                                       without memory alloc or HW.
 *      LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E - this is an update for
 *                                     root bucket , it's a shodow update
 *                                     only, no touching of the HW.
 *
 */
typedef enum
{
    LPM_BUCKET_UPDATE_NONE_E = 0,
    LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E,
    LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E,
    LPM_BUCKET_UPDATE_MEM_ONLY_E,
    LPM_BUCKET_UPDATE_SHADOW_ONLY_E,
    LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E,
    LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E,

}LPM_BUCKET_UPDATE_MODE_ENT;


/*
 * Typedef: enum LPM_ROOT_BUCKET_UPDATE_ENT
 *
 * Description:
 *      Indicates what kind of update the root bucket will pass.
 *
 *
 * Fields:
 *      LPM_ROOT_BUCKET_UPDATE_NONE_E  - updates nothing.
 *      LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E     - updates the HW in new mem alloc.
 *      LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E - updates the src HW in new mem alloc.
 *      LPM_ROOT_BUCKET_UPDATE_HW_E                    - updates the HW.
 */
typedef enum
{
    LPM_ROOT_BUCKET_UPDATE_NONE_E = 0,
    LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E,
    LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E,
    LPM_ROOT_BUCKET_UPDATE_HW_E
}LPM_ROOT_BUCKET_UPDATE_ENT;

/*******************************************************************************
* mask2PrefixLength
*
* DESCRIPTION:
*       This function returns the index of the most significant set bit (1-8),
*       in the given mask.
*
* INPUTS:
*       mask            - The prefix mask to operate on.
*       levelMaxPrefix  - The maximum prefix that can be hold by this mask.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       1 - levelMaxPrefix if (mask != 0), 0 otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_U32 mask2PrefixLength
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

/*******************************************************************************
* calcStartEndAddr
*
* DESCRIPTION:
*       This function calcules the start & end address of a prefix.
*
* INPUTS:
*       addr          - the address octet.
*       prefixLen     - the address prefix length
* OUTPUTS:
*       startAddrPtr  - the calculated start address
*       endAddrPtr    - the calculated end address
*
*
* RETURNS:
*
* COMMENTS:
*       None.
*
*******************************************************************************/
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


/*******************************************************************************
* insert2Trie
*
* DESCRIPTION:
*       Inserts a next hop entry to the trie structure.
*
* INPUTS:
*       bucketPtr   - Bucket to which the given entry is associated.
*       startAddr   - Start Addr of the address associated with the given
*                     entry.
*       prefix      - The address prefix.
*       trieDepth   - The maximum depth of the trie.
*       nextPtr     - A pointer to the next hop/next_lpm_trie entry to be
*                     inserted to the trie.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
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
* createNewSip6Bucket
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
static PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC * createNewSip6Bucket
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

/*******************************************************************************
* sip6SplitRange
*
* DESCRIPTION:
*       This function splits a range. According to one of the following possible
*       splits:
*       PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E, PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E,
*       PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E and PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E
*
* INPUTS:
*       rangePtrPtr     - Pointer to the range to be splitted.
*       startAddr       - The low address of the new range.
*       endAddr         - The high address of the new range.
*       prefixLength    - The length of the dominant prefix of the new range.
*       rangeInfoPtr    - the range information to use for the new range
*                         (or overwrite an old one with this new info).
*       levelPrefix     - The current lpm level prefix.
*       updateOldPtr    - (GT_TRUE) this is an update for an already existing entry.
*
* OUTPUTS:
*       rangePtrPtr         - A pointer to the next range to be checked for split.
*       numOfNewRangesPtr   - Number of new created ranges as a result of the split.
*       pointerTypePtrPtr   - the added/replaced range's pointerType field pointer
*
* RETURNS:
*       GT_OK on success, or
*       GT_OUT_OF_CPU_MEM on lack of cpu memory.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS sip6SplitRange
(
    INOUT PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **rangePtrPtr,
    IN GT_U8                                        startAddr,
    IN GT_U8                                        endAddr,
    IN GT_U32                                       prefixLength,
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC       *rangeInfoPtr,
    IN GT_U8                                        levelPrefix,
    IN GT_BOOL                                      *updateOldPtr,
    IN CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT          *bucketTypePtr,
    OUT GT_U32                                      *numOfNewRangesPtr,
    OUT GT_U32                                      *numOfNewHwRangesPtr,
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
   /* GT_BOOL splittedRangeNeedsHwUpdate = GT_FALSE;*/
    GT_U8 groupSubNodeId;

    mask = (GT_U8)PREFIX_2_MASK_MAC(prefixLength,levelPrefix);

    endRange = (GT_U8)(LAST_RANGE_MAC(curRangePtr) ? ((1 << levelPrefix) - 1) :
        ((curRangePtr->next->startAddr) - 1));

    splitMethod = (((startAddr > curRangePtr->startAddr) ? 1 : 0) +
                   ((endAddr < endRange) ? 2 : 0));
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
       /* newRangePtr->updateRangeInHw = GT_TRUE;*/

        *numOfNewRangesPtr = 0;
        *numOfNewHwRangesPtr = 0;
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

        /* curRangePtr represents the new Range.    */
        curRangePtr->mask       |= mask;
        curRangePtr->pointerType = rangeInfoPtr->pointerType;
        curRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        /* new pointer so we need to update in the hardware */
       /* curRangePtr->updateRangeInHw = GT_TRUE;*/

        *rangePtrPtr       = newRangePtr;
        *numOfNewRangesPtr = 1;
        *numOfNewHwRangesPtr = 1;
        if (*bucketTypePtr == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (startAddr != 0)
            {
               groupSubNodeId = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
               if (endAddr == endSubNodeAddress[groupSubNodeId])
               {
                   *numOfNewHwRangesPtr = 0;
               }
            }
        }

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
        /* Update the fields of newRangePtr     */
        newRangePtr->startAddr  = startAddr;
        newRangePtr->mask       = (GT_U8)(curRangePtr->mask | mask);
        newRangePtr->pointerType = rangeInfoPtr->pointerType;
        newRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        /* new pointer so we need to update in the hardware */
       /* newRangePtr->updateRangeInHw = GT_TRUE;*/

        *rangePtrPtr       = newRangePtr;
        *numOfNewRangesPtr = 1;
        *numOfNewHwRangesPtr = 1;
        if (*bucketTypePtr == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (endAddr != 255)
            {
               groupSubNodeId = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
               if (startAddr == startSubNodeAddress[groupSubNodeId])
               {
                   *numOfNewHwRangesPtr = 0;
               }
            }
        }
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

        /* Update the fields of the first added range   */
        newRangePtr->startAddr  = startAddr;
        newRangePtr->mask       = (GT_U8)(curRangePtr->mask | mask);

        newRangePtr->pointerType = rangeInfoPtr->pointerType;
        newRangePtr->lowerLpmPtr = rangeInfoPtr->lowerLpmPtr;
        /* new pointer so we need to update in the hardware */
       /* newRangePtr->updateRangeInHw = GT_TRUE;*/

        /* record the pointer type ptr */
        *pointerTypePtrPtr = &(newRangePtr->pointerType);

        /* Update the fields of the second added range  */
        newRangePtr             = newRangePtr->next;
        newRangePtr->startAddr  = (GT_U8)(endAddr + 1);
        newRangePtr->mask       = curRangePtr->mask;

        newRangePtr->pointerType = curRangePtr->pointerType;
        newRangePtr->lowerLpmPtr = curRangePtr->lowerLpmPtr;
        *rangePtrPtr = newRangePtr;
        *numOfNewRangesPtr = 2;
        *numOfNewHwRangesPtr = 2;
        if (*bucketTypePtr == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
           groupSubNodeId = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
           if (startAddr == startSubNodeAddress[groupSubNodeId])
           {
               (*numOfNewHwRangesPtr)--;
           }
           groupSubNodeId = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
           if (endAddr == endSubNodeAddress[groupSubNodeId])
           {
               (*numOfNewHwRangesPtr)--;
           }
        }

        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/*******************************************************************************
* lpmFalconFillLeafData
*
* DESCRIPTION:
*       This function fill leafsNodesArray struct.
*
* INPUTS:
*       nextHopPtr             - pointer to nexthop data
*       entryType              - next hop pointer type
*
* OUTPUTS:
*       leafNodesArrayPtr  - array saving leaf nodes info
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS lpmFalconFillLeafData
(
    IN  PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *rangePtr,
    OUT PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC               *leafNodesArrayPtr
)
{
    GT_UINTPTR hwBucketOffsetHandle = 0;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT            entryType;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC      *nextHopPtr = NULL;

    entryType = rangePtr->pointerType;
    if ( (entryType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
         (entryType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  &&
         (entryType != 0xff) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong node type");
    }
    if (entryType == 0xff) /*leaf points to src trie*/
    {
        hwBucketOffsetHandle = rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle;
        /* It points on src tree */
        if (hwBucketOffsetHandle != 0) 
        {
            leafNodesArrayPtr->pointToSip = GT_TRUE;
            leafNodesArrayPtr->pointerToSip = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(hwBucketOffsetHandle);
            leafNodesArrayPtr->nextNodeType = rangePtr->lowerLpmPtr.nextBucket->bucketType;
            leafNodesArrayPtr->priority = 0;
            if ((leafNodesArrayPtr->nextNodeType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                (leafNodesArrayPtr->nextNodeType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) )
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong node type");
            }
            leafNodesArrayPtr->entryType = PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E;
            return GT_OK;
        }
        else
        {
            /* we have only shadow src tree */
            entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        }
    }
    if (rangePtr->pointerType == 0xff) 
    {
        nextHopPtr = rangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry;
    }
    else
    {
        nextHopPtr = rangePtr->lowerLpmPtr.nextHopEntry;
    }
     
    leafNodesArrayPtr->priority = nextHopPtr->priority;
    leafNodesArrayPtr->entryType = entryType; 
    leafNodesArrayPtr->index = nextHopPtr->routeEntryBaseMemAddr;
    leafNodesArrayPtr->ipv6MCGroupScopeLevel = nextHopPtr->ipv6McGroupScopeLevel;
    leafNodesArrayPtr->ucRPFCheckEnable = nextHopPtr->ucRpfCheckEnable;
    leafNodesArrayPtr->sipSaCheckMismatchEnable = nextHopPtr->srcAddrCheckMismatchEnable;
    return GT_OK;
}


/*******************************************************************************
* lpmFalconConvertLpmOffset
*
* DESCRIPTION:
*       This function convert absolute lpm offset to bunk number and offset within the bank.
*
* INPUTS:
*       bucketType                    - type of bucket
*       groupOffseHandletPtr          - pointer to  group of nodes offsets handle array
*       startSubnode                  - first affected LPM line in regular node
*       endSubnode                    - last affected LPM line in regular node
*       allSubnodes                   - all lines in regular node
*
* OUTPUTS:
*       relativeOffsetPtr             - pointer to  group of nodes offsets array {bank,offset_in_bank}
*
* RETURNS:
*       GT_OK on success, or
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS lpmFalconConvertLpmOffset
(
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT bucketType,
    IN  GT_UINTPTR                          *groupOffseHandlePtr,
    IN  GT_U32                              lpmRamTotalBlocksSizeIncludingGap,
    IN  GT_U32                              startSubnode,
    IN  GT_U32                              endSubnode,
    IN  GT_BOOL                             allSubnodes,
    OUT GT_UINTPTR                          *relativeOffsetPtr
)
{
    GT_U32 bankNumber;
    GT_U32 offsetInBank;
    GT_U32 groupOffset;
    GT_U32 i;
    GT_U32 maxNumOfSubnodes, startNumOfSubnodes;
    if ( (bucketType != CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         &&
         (bucketType != CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   &&
         (bucketType != CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) &&
         (bucketType != CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)       &&
         (bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        if (allSubnodes == GT_TRUE)
        {
            startNumOfSubnodes = 0;
            maxNumOfSubnodes = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;
        }
        else
        {
            startNumOfSubnodes = startSubnode;
            maxNumOfSubnodes = endSubnode;
        }
    }
    else
    {
        startNumOfSubnodes = 0;
        maxNumOfSubnodes = 1;
    }
    for (i = startNumOfSubnodes; i < maxNumOfSubnodes; i++)
    {
        if (groupOffseHandlePtr[i] != 0)
        {
            groupOffset = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(groupOffseHandlePtr[i]));
            bankNumber = groupOffset/lpmRamTotalBlocksSizeIncludingGap;
            offsetInBank = groupOffset%lpmRamTotalBlocksSizeIncludingGap;
            *relativeOffsetPtr = (bankNumber << 15 | offsetInBank)&0xfffff;
        }
        else
        {
            *relativeOffsetPtr = 0;
        }
        relativeOffsetPtr++;
    }
   return GT_OK;
}


/*******************************************************************************
* lpmFalconFillCompressedData
*
* DESCRIPTION:
*       This function scan bucket shadow and fill compressedNodesArray struct.
*
* INPUTS:
*       bucketPtr                         - pointer to  bucket
*       lpmRamTotalBlocksSizeIncludingGap - lpm total block size
*
* OUTPUTS:
*       compressedNodesArrayPtr  - array saving compressed nodes info
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS lpmFalconFillCompressedData
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      lpmRamTotalBlocksSizeIncludingGap,
    OUT PRV_CPSS_DXCH_LPM_COMPRESSED_STC            *compressedNodesArrayPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i = 0;
    GT_U8 numOfLeaves = 0;
    GT_U8 maxNumOfLeaves = 0;
    GT_U8 maxOffsetsNum = 0;
    GT_U8 lastOffset = 0;
    GT_U8 lastChildNodeTypes = 0;
    GT_U8 offsetBeforeLast = 0;
    GT_U8 childTypeBeforeLast;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    switch (bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        maxNumOfLeaves = 0;
        maxOffsetsNum = 9;
        break;
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        maxNumOfLeaves = 1;
        maxOffsetsNum = 6;
        break;
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        maxNumOfLeaves = 2;
        maxOffsetsNum = 4;
        break;
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
        maxNumOfLeaves = 3;
        maxOffsetsNum = 2;
        break;
   /* case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
    case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
    case PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E:
        break;*/
    default:
        cpssOsPrintf(" BAD STATE: wrong bucket type\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    compressedNodesArrayPtr->compressedType = bucketPtr->bucketType;
    rangePtr = bucketPtr->rangeList;

    while (rangePtr != NULL)
    {
        if (i == 0)
        {
            /* first range : its start is 0*/
            compressedNodesArrayPtr->ranges1_9[i]=0;
        }
        else
            compressedNodesArrayPtr->ranges1_9[i]= rangePtr->startAddr;
        /* check subnodes ths range belongs to */
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            compressedNodesArrayPtr->childNodeTypes0_9[i] = 0x2;
        }

        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            compressedNodesArrayPtr->childNodeTypes0_9[i] = 0x3;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            compressedNodesArrayPtr->childNodeTypes0_9[i] = 0x1;
            if( (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
                (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
                (bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
            {
                /* embedded leaves bucket */
                if (numOfLeaves < maxNumOfLeaves)
                {
                    rc =  lpmFalconFillLeafData(rangePtr,
                                                &compressedNodesArrayPtr->embLeavesArray[numOfLeaves]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                else
                    cpssOsPrintf(" BAD STATE: number of leaves doesn't match compress bucket type\n");
            }
            numOfLeaves++;
        }
        rangePtr = rangePtr->next;
        i++;
    }
    if (i - 1 < maxOffsetsNum)
    {
        lastOffset = compressedNodesArrayPtr->ranges1_9[i - 1];
        lastChildNodeTypes = compressedNodesArrayPtr->childNodeTypes0_9[i-1];
        offsetBeforeLast = (i-2 > 0)? compressedNodesArrayPtr->ranges1_9[i-2] : lastOffset;
        childTypeBeforeLast = (i-2 > 0)? compressedNodesArrayPtr->childNodeTypes0_9[i-2] :
                               i==2    ? 0 : lastChildNodeTypes;
        if((i-2) > 0)
        {
            compressedNodesArrayPtr->childNodeTypes0_9[i-2] = 0;
        }

        for (i = i-1; i <= maxOffsetsNum; i++)
        {
            if (i == maxOffsetsNum)
            {
                compressedNodesArrayPtr->ranges1_9[i] = lastOffset;
                compressedNodesArrayPtr->childNodeTypes0_9[i] = lastChildNodeTypes;
            }
            else
            if ((i+1) == maxOffsetsNum)
            {
                compressedNodesArrayPtr->ranges1_9[i] = offsetBeforeLast;
                compressedNodesArrayPtr->childNodeTypes0_9[i] = childTypeBeforeLast;
            }
            else
            {
                compressedNodesArrayPtr->ranges1_9[i] = offsetBeforeLast;
                compressedNodesArrayPtr->childNodeTypes0_9[i] = 0;
            }
        }
    }

    /* now add ofsets to hw data: */
    if (bucketPtr->hwGroupOffsetHandle[0]!= 0)
    {
         rc =  lpmFalconConvertLpmOffset(bucketPtr->bucketType,
                                    bucketPtr->hwGroupOffsetHandle,
                                    lpmRamTotalBlocksSizeIncludingGap,
                                    0,0,0,
                                    &compressedNodesArrayPtr->lpmOffset);
    }
    else
    {
        /* possible to check for node with embedded leaves. Otherwisw - error*/
        /* it may be last node with embedded leaves */
        compressedNodesArrayPtr->lpmOffset = 0;
    }
    compressedNodesArrayPtr->numberOfLeaves = numOfLeaves;
    return rc;
}



/*******************************************************************************
* lpmFalconFillRegularData
*
* DESCRIPTION:
*       This function scan bucket shadow and fill regularNodesArray struct.
*
* INPUTS:
*       bucketPtr                         - pointer to  bucket
*       lpmRamTotalBlocksSizeIncludingGap - lpm total block size
*
* OUTPUTS:
*       regularNodesArrayPtr  - array saving regular nodes info
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS lpmFalconFillRegularData
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      lpmRamTotalBlocksSizeIncludingGap,
    OUT PRV_CPSS_DXCH_LPM_REGULAR_STC               *regularNodesArrayPtr
)
{
    GT_U32 startNodeIndex, endNodeIndex,i;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    GT_U8 endRangeAddress = 0;
    rangePtr = bucketPtr->rangeList;
    /* In regular node we have 6 different subnodes */
    while (rangePtr != NULL)
    {
        startNodeIndex = rangePtr->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        if (rangePtr->next == NULL)
        {
            endRangeAddress = 255; /* last range end addr */
        }
        else
        {
            endRangeAddress = rangePtr->next->startAddr - 1;
        }
        endNodeIndex =  endRangeAddress / NUMBER_OF_RANGES_IN_SUBNODE;
        /* check subnodes ths range belongs to */
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x2;
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                if (i > startNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 0x2;
                }
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x3;
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                if (i > startNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 0x3;
                }
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x1;

            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                if (i > startNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 1;
                }
            }
        }
        rangePtr = rangePtr->next;
    }
    /* now add ofsets to hw data: */
    lpmFalconConvertLpmOffset(bucketPtr->bucketType,
                              bucketPtr->hwGroupOffsetHandle,
                              lpmRamTotalBlocksSizeIncludingGap,
                              0,0,GT_TRUE,
                              regularNodesArrayPtr->lpmOffsets);
    return GT_OK;
}


#if 0
/*******************************************************************************
* lpmFalconUpdateRegularDataForGivenSubnodes
*
* DESCRIPTION:
*       This function scan bucket shadow and fill regularNodesArray struct for
*       given subnode.
*
* INPUTS:
*       bucketPtr                         - pointer to  bucket
*       lpmRamTotalBlocksSizeIncludingGap - lpm total block size
*       startSubNodeAddress               - start subnode address
*       endSubNodeAddress                 - end subnode address
*
* OUTPUTS:
*       regularNodesArrayPtr  - array saving regular nodes info
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS lpmFalconUpdateRegularDataForGivenSubnodes
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      lpmRamTotalBlocksSizeIncludingGap,
    IN  GT_U32                                      startSubNodeAddr,
    IN  GT_U32                                      endSubNodeAddr,
    OUT CPSS_DXCH_LPM_REGULAR_STC                   *regularNodesArrayPtr
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    GT_U32 endRangeAddr = 0;
    GT_U32 startSubNodeIndex,endSubNodeIndex;
    startSubNodeIndex = startSubNodeAddr/NUMBER_OF_RANGES_IN_SUBNODE;
    endSubNodeIndex = endSubNodeAddr/NUMBER_OF_RANGES_IN_SUBNODE;
    rangePtr = bucketPtr->rangeList;

    /* choose needed range for subnode */
    while (rangePtr !=NULL)
    {
        if (rangePtr->startAddr >= startSubNodeAddr)
        {
            break;
        }
        else
        {
            /* check if the range incorporate needed range */
            if (rangePtr->next != NULL)
            {
                endRangeAddr = rangePtr->next->startAddr - 1;
            }
            else
            {
                endRangeAddr = 255; /* last range end address must be 255*/
            }
            if (endRangeAddr > startSubNodeAddr)
            {
                /* It must be start from this range */
                break;
            }
        }
        rangePtr = rangePtr->next;
    }

    while (rangePtr != NULL)
    {
        if (rangePtr->next == NULL)
        {
            endRangeAddr = 255; /* last range end addr */
        }
        else
        {
            endRangeAddr = rangePtr->next->startAddr - 1;
        }
        if (rangePtr->startAddr > endSubNodeAddr)
        {
            break;
        }

        /* check subnodes ths range belongs to */
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x2;
            for (i = startSubNodeIndex; i <= endSubNodeIndex; i++ )
            {
                if (i > startSubNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 0x2;
                }
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 0x3;
            for (i = startSubNodeIndex; i <= endSubNodeIndex; i++ )
            {
                if (i > startSubNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 0x3;

                }
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) )
        {
            if (rangePtr->startAddr < startSubNodeAddr)
            {
                regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddr] = 1;
            }
            else
            {
                regularNodesArrayPtr->childNodeTypes0_255[rangePtr->startAddr] = 1;
            }

            for (i = startSubNodeIndex; i <= endSubNodeIndex; i++ )
            {
                if (i > startSubNodeIndex)
                {
                    /* catch all subnodes crosses in spectrum */
                    regularNodesArrayPtr->childNodeTypes0_255[startSubNodeAddress[i]] = 1;
                }
            }
        }
        rangePtr = rangePtr->next;
    }
    /* now add ofsets to hw data: */
    lpmFalconConvertLpmOffset(bucketPtr->bucketType,
                              bucketPtr->hwGroupOffsetHandle,
                              lpmRamTotalBlocksSizeIncludingGap,
                              startSubNodeIndex,endSubNodeIndex,GT_TRUE,
                              regularNodesArrayPtr->lpmOffsets);
    return GT_OK;
}

#endif
/*******************************************************************************
* getMirrorGroupOfNodesDataAndUpdateRangesAddress
*
* DESCRIPTION:
*       This function gets a the bucket's shadow data and formats accordingly
*       the bitvector,compressed and the next pointer array.
*
*
* INPUTS:
*       bucketType            - bucket type
*       startAddr             - start subnode address of needed range
*       endAddr               - end subnode address of needed range
*       rangePtr              - Pointer to the current bucket's first range.
*       lpmEngineMemPtrPtr    - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                               which holds all the memory information needed
*                               for where and how to allocate search memory.
*       newBucketType         - the bucket's new type
*       bucketBaseAddress     - the bucket's base address
*
* OUTPUTS:
*       groupOfNodesPtr       - group of nodes contents
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS getMirrorGroupOfNodesDataAndUpdateRangesAddress
(
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             bucketType,
    IN  GT_U8                                           startAddress,
    IN  GT_U8                                           endAddress,
    IN  PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *rangePtr,
    IN  GT_U32                                          lpmRamTotalBlocksSizeIncludingGap,
    OUT PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC      *groupOfNodesPtr
)
{
    GT_STATUS rc;
    GT_U8 regularCounter = 0;
    GT_U8 compressedCounter = 0;
    GT_U8 leafCounter = 0;
    GT_U8 endRangeAddr = 0;

    if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        /* choose needed range for subnode */
        while (rangePtr !=NULL)
        {
            if (rangePtr->startAddr >= startAddress)
            {
                break;
            }
            else
            {
                /* check if the range incorporate needed range */
                if (rangePtr->next != NULL)
                {
                    endRangeAddr = rangePtr->next->startAddr - 1;
                }
                else
                {
                    endRangeAddr = 255; /* last range end address must be 255*/
                }
                if (endRangeAddr >= startAddress)
                {
                    /* It must be start from this range */
                    break;
                }
            }
            rangePtr = rangePtr->next;
        }
    }
    while (rangePtr != NULL)
    {
        if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (rangePtr->startAddr > endAddress)
            {
                break;
            }
        }
        if(rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            rc =  lpmFalconFillRegularData(rangePtr->lowerLpmPtr.nextBucket,
                                           lpmRamTotalBlocksSizeIncludingGap,
                                           &groupOfNodesPtr->regularNodesArray[regularCounter]);
            if (rc != GT_OK)
            {
                return rc;
            }
            regularCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            rc =  lpmFalconFillCompressedData(rangePtr->lowerLpmPtr.nextBucket,
                                              lpmRamTotalBlocksSizeIncludingGap,
                                              &groupOfNodesPtr->compressedNodesArray[compressedCounter]);
            if (rc != GT_OK)
            {
                return rc;
            }
            compressedCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            /* if bucket type is compressed_with_embedded leaves then all leaves are inside*/
            /* compressed node itself. It is not included into group of nodes current bucket pointed on*/
            /* These leaves relate to the next group of nodes together with embedded compressed: 1 level up*/
            if( (bucketType != CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   &&
                (bucketType != CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) &&
                (bucketType != CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
            {
                rc =  lpmFalconFillLeafData(rangePtr,
                                            &groupOfNodesPtr->leafNodesArray[leafCounter]);
                if (rc != GT_OK)
                {
                    return rc;
                }
                leafCounter++;
            }
        }
        rangePtr->updateRangeInHw = GT_FALSE;
        rangePtr = rangePtr->next;
    }


    return GT_OK;
}


/*******************************************************************************
* lpmFalconGetHwNodeOffsetInsideGroupOfNodes
*
* DESCRIPTION:
*       This function
*       the lpm structures in PP's memory.
*
* INPUTS:
*       bucketPtr             - Pointer to the bucket on which needed range exist.
*       bucketType            - bucket type updated range pointed on
*       rangeAddr             - start address of needed range.
*                               (the end address the same)
*       startSubNodeAddress   - start subnode address. (relevant for regular bucket)
*       endSubNodeAddress     - end subnode address. (relevant for regular bucket)
*       hwGroupOffset         - hw group offset
*                               the ranges where the writeRangeInHw is set.
*
* OUTPUTS:
*       nodeHwAddrPtr         - The hw offset of needed node inside group.
*
* RETURNS:
*       GT_OK on success, or
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS lpmFalconGetHwNodeOffsetInsideGroupOfNodes
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN  GT_U32                                    rangeStartAddr,
    IN  GT_U8                                     startSubNodeAddress,
    IN  GT_U8                                     endSubNodeAddress,
    IN  GT_U32                                    hwGroupOffset,
    OUT GT_U32                                    *nodeHwAddrPtr
)
{
    GT_U32 leafCounter = 0;
    GT_U32 compressedCounter = 0;
    GT_U32 regularCounter = 0;
    GT_U32 numberOfCompressedNodesBeforeRange = 0;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *rangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *updatedRangePtr = NULL;
    rangePtr = bucketPtr->rangeList;
    switch (bucketPtr->bucketType)
    {
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        break;
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        /* choose needed range for subnode */
        while (rangePtr !=NULL)
        {
            if (rangePtr->startAddr >= startSubNodeAddress)
            {
                break;
            }
            else
            {
                /* check if the range incorporate needed range */
                if (rangePtr->next != NULL)
                {
                    if ((rangePtr->next->startAddr - 1) >= startSubNodeAddress)
                    {
                        /* It must be start from this range */
                        break;
                    }
                }
            }
            rangePtr = rangePtr->next;
        }
    }
    while (rangePtr != NULL)
    {
        if (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (rangePtr->startAddr > endSubNodeAddress)
            {
                break;
            }
        }
        if (rangePtr->startAddr == rangeStartAddr)
        {
            updatedRangePtr = rangePtr;

            if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                /* for this case the only number of regular before this range is interesting*/
                break;
            }
            else
                if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
                    (rangePtr->pointerType == 0xff) )
                {
                    /* this is illegal case */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "rangePtr->pointerType : Range poining to leaf is not relevant object! ");
                }
                else
                {
                    /* in case of compressed all number of regular and number of compressed before the range*/
                    numberOfCompressedNodesBeforeRange = compressedCounter;
                }
        }
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
           regularCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            compressedCounter++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            leafCounter++;
        }
        rangePtr = rangePtr->next;
    }
    if (updatedRangePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "updatedRangePtr : Can't find updated range! ");

    }
    if (updatedRangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        *nodeHwAddrPtr = hwGroupOffset + 6*regularCounter;
    }
    else
    {
        *nodeHwAddrPtr = hwGroupOffset+6*regularCounter+numberOfCompressedNodesBeforeRange;
    }
    return GT_OK;
}

/*******************************************************************************
* lpmFalconUpdateRangeInHw
*
* DESCRIPTION:
*       This function creates a mirrored group of nodes in CPU's memory, and updates
*       the lpm structures in PP's memory.
*
* INPUTS:
*       bucketPtr             - pointer to the bucket to create a mirror from.
*       pRange                - pointer to range list
*       shadowPtr             - lpm shadow ptr
*
* OUTPUTS:
*       None
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
static GT_STATUS lpmFalconUpdateRangeInHw
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *bucketPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *pRange,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                *shadowPtr
)
{
    GT_U32 tempAddr;
    GT_U32 j = 0;
    GT_STATUS retVal;
    static PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   groupOfNodes;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    /*GT_U8 startSubnodeId = 0;
     GT_U8 endSubnodeId = 0;*/
    GT_U8 subNodeId = 0;
    GT_UINTPTR groupStartAddrHandle = 0;

    cpssOsMemSet(&groupOfNodes, 0, sizeof(groupOfNodes));
    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;


    while (pRange != NULL)
    {
        /* only if the range needs updating , update it */
        if (pRange->updateRangeInHw == GT_TRUE)
        {
            /* calculate group of nodes start address*/
            switch (bucketPtr->bucketType)
            {
            case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                groupStartAddrHandle = bucketPtr->hwGroupOffsetHandle[0];
                break;
            case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                subNodeId = pRange->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                groupStartAddrHandle = bucketPtr->hwGroupOffsetHandle[subNodeId];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            }

            switch (pRange->lowerLpmPtr.nextBucket->bucketType)
            {
            case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
            case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
                retVal =  lpmFalconFillCompressedData(pRange->lowerLpmPtr.nextBucket,
                                                      shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                      &groupOfNodes.compressedNodesArray[0]);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
                break;
            case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
                retVal =  lpmFalconFillRegularData(pRange->lowerLpmPtr.nextBucket,
                                                   shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                   &groupOfNodes.regularNodesArray[0]);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            }

            /* now calculate offset inside of group of nodes for node this range pointed on */
            retVal = lpmFalconGetHwNodeOffsetInsideGroupOfNodes(bucketPtr,
                                                                pRange->startAddr,
                                                                startSubNodeAddress[subNodeId],
                                                                endSubNodeAddress[subNodeId],
                                                                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(groupStartAddrHandle),
                                                                &tempAddr);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            /* tempAddr shows where this node starts in group of nodes*/
            for (j = 0; j < shareDevListLen; j++)
            {
                retVal =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[j],
                                                          tempAddr,
                                                          &groupOfNodes);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            /*  remove the flag */
            pRange->updateRangeInHw = GT_FALSE;
        }
        pRange = pRange->next;
    }
    return GT_OK;
}

/*******************************************************************************
* lpmFalconGetCompressedBucketType
*
* DESCRIPTION:
*       Get exact type of compressed bucket
*
* INPUTS:
*       bucketPtr                     - Pointer to the bucket to create a mirror from.
*
* OUTPUTS:
*       newCompressedBucBucketTypePtr - The new compressed type of the mirrored bucket.
*
* RETURNS:
*       None.
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_VOID lpmFalconGetCompressedBucketType
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    OUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     *newCompressedBucBucketTypePtr
)
{
    GT_U32 numOfLeaves = 0;
    GT_U32 numOfRanges = bucketPtr->numOfRanges;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC      *rangePtr = NULL;
    /* calculate leaves number*/

    rangePtr = bucketPtr->rangeList;
    while (rangePtr != NULL)
    {
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            numOfLeaves++;
        }
        rangePtr = rangePtr->next;
    }
    if ((numOfRanges <=7) && (numOfLeaves == 1))
    {
        *newCompressedBucBucketTypePtr = CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E;
    }else
        if ((numOfRanges <=5) && (numOfLeaves == 2))
        {
            *newCompressedBucBucketTypePtr = CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E;
        }else
            if ((numOfRanges ==3) && (numOfLeaves == 3))
            {
                *newCompressedBucBucketTypePtr = CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E;
            }else
            {
                *newCompressedBucBucketTypePtr = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
            }
}
/*******************************************************************************
* updateMirrorGroupOfNodes
*
* DESCRIPTION:
*       This function creates a mirrored group of nodes in CPU's memory, and updates
*       the lpm structures in PP's memory.
*
* INPUTS:
*       bucketPtr             - Pointer to the bucket to create a mirror from.
*       bucketUpdateMode      - Indicates whether to write the table to the device's
*                               Ram, update the memory alloc , or do nothing.
*       isDestTreeRootBucket  - Indicates whether the bucket is the root of the
*                               destination address tree
*       resizeBucket          - Indicates whether the bucket was resized or not
*                               during the insertion / deletion process.
*       forceWriteWholeBucket - force writing of the whole bucket and not just
*                               the ranges where the writeRangeInHw is set.
*       indicateSiblingUpdate - whether to update siblings of buckets (used in
*                               bulk mode)
*       lpmEngineMemPtrPtr    - points to a an PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                               which holds all the memory information needed
*                               for where and how to allocate search memory.
*       bucketTypePtr         - The bucket type of the mirrored bucket.
*       shadowPtr             - the shadow relevant for the devices asked to act on.
*       parentWriteFuncPtr    - the bucket's parent write function in case there is
*                               a need to update the packet's parent ptr data and
*                               the parent is not a LPM trie range.
*                               (relevant only in delete operations)
*       subnodesSizesPtr       - pointer to subnodes gon sizes
*       subnodesIndexesPtr     - pointer to subnodes array showing subnodes going to change.
*
* OUTPUTS:
*       bucketTypePtr         - The bucket type of the mirrored bucket.
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
static GT_STATUS updateMirrorGroupOfNodes
(
    IN PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr,
    IN LPM_BUCKET_UPDATE_MODE_ENT               bucketUpdateMode,
    IN GT_BOOL                                  isDestTreeRootBucket,
    IN GT_BOOL                                  isSrctTreeRootBucket,
    IN LPM_ROOT_BUCKET_UPDATE_ENT               rootBucketState,
    IN GT_BOOL                                  resizeGroup,
    IN GT_BOOL                                  numOfRangesIsChanged,
    IN GT_BOOL                                  overwriteGroup,
    IN GT_BOOL                                  forceWriteWholeBucket,
    IN GT_BOOL                                  indicateSiblingUpdate,
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC       **lpmEngineMemPtrPtr,
    INOUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT   *bucketTypePtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC    *parentNodeData,
    IN GT_BOOL                                  *subnodesIndexesPtr,
    IN GT_U32                                   *subnodesSizesPtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U32 i,j;

    GT_UINTPTR oldMemPool = 0;  /* The memory pool from which the old buckets   */
                            /* where allocated.                             */

    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    GT_UINTPTR tempHwRootAddrHandle = 0;
    GT_UINTPTR tempHwAddrHandle[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};/* Will hold the allocated area in PP's RAM to  */
                                /* which the buckets will be written.           */

    GT_U32 memBlockBase = 0;/* the base address (offset) in the RAM for bucket*/

    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr = NULL;/* use for going over the list of blocks per octet */
    GT_UINTPTR                           tmpStructsMemPool = 0;/* use for going over the list of blocks per octet */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange = NULL;
    GT_U32 tempAddr[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};     /* Temporary address                */

    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC     **freeMemListPtr;
    GT_BOOL swapMemInUse = GT_FALSE;
    GT_BOOL secondSwapMemInUse = GT_FALSE;

    GT_BOOL freeOldMem = GT_TRUE;
    GT_BOOL groopsAllocWasDone = GT_FALSE;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT newBucBucketType;
    GT_UINTPTR oldHwAddrHandle[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
    GT_U32 groupOfNodesNum = 1;
    GT_BOOL updatePtrToNewAllocatedAfterSwap = GT_FALSE;
    static PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   groupOfNodes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];

    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */

    indicateSiblingUpdate = indicateSiblingUpdate; /* prevent warning */
    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    /* Determine the bucket type of the new bucket. */
    if(bucketPtr->numOfRanges == 1)
    {
        if (isDestTreeRootBucket == GT_TRUE)
        {
            newBucBucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
            groupOfNodesNum = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;
        }
        else
        {
            newBucBucketType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        }
    }
    else
        if((bucketPtr->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS)&&    /*  compressed bucket */
           (isDestTreeRootBucket == GT_FALSE) )
    {
        /* check exact compressed type */
        lpmFalconGetCompressedBucketType(bucketPtr,&newBucBucketType);
    }
    else                                    /* Regular bucket           */
    {
        newBucBucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
        groupOfNodesNum = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;
    }

    if ( (bucketUpdateMode == LPM_BUCKET_UPDATE_SHADOW_ONLY_E) ||
         (bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E) )
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
    if ( bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E )
    {
        /* This is delete operation and bucket going to be compressed with embedded leaves:
                   3 ranges 3 leaves (last level). It means we need to free memory and updates shadow */
        if (resizeGroup == GT_TRUE)
        {
            if (bucketPtr->hwGroupOffsetHandle[0] == 0)
            {
                cpssOsPrintf("BAD STATE:Last level, resize = TRUE,bucketPtr->hwGroupOffsetHandle[0] =0\n");
            }
            retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwGroupOffsetHandle[0],
                                                     PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                     freeMemListPtr,shadowPtr);
            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
    }

    /* If there is only one range (next hop type), then delete the
       whole bucket */
    if(newBucBucketType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
        {
            if ((resizeGroup == GT_TRUE)&& (bucketPtr->hwGroupOffsetHandle[0] == 0) )
            {
                cpssOsPrintf("BAD STATE: resize == TRUE, GON =0");
            }
            if ((bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E) ||
                (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E))
            {
                cpssOsPrintf("BAD STATE: wrong bucket type\n");
            }
            if ((bucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E) &&
                (resizeGroup == GT_FALSE))
            {
                cpssOsPrintf("BAD STATE: wrong bucket type\n");
            }
            /* in some cases bucket is not pointing on GON, so there is no need to free */
            if (bucketPtr->hwGroupOffsetHandle[0] != 0)
            {
                retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwGroupOffsetHandle[0],
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
        if (isSrctTreeRootBucket == GT_TRUE) 
        {
            /* src tree root bucket*/
            if (bucketPtr->hwBucketOffsetHandle != 0)
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
        bucketPtr->hwBucketOffsetHandle = 0; 
        cpssOsMemSet(bucketPtr->hwGroupOffsetHandle,0,sizeof(bucketPtr->hwGroupOffsetHandle));
        return GT_OK;
    }


    /* check the case of no hw update */
    if((bucketUpdateMode == LPM_BUCKET_UPDATE_ROOT_BUCKET_SHADOW_ONLY_E)  ||
       (bucketUpdateMode == LPM_BUCKET_UPDATE_NONE_E))
    {
        /* all buckets that are visited during the shadow update stage of
           the bulk operation should be scanned during the hardware update stage;
           therefore don't leave buckets with PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E
           mark */
        if ((indicateSiblingUpdate == GT_TRUE) &&
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

        rootBucketState = LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E;
    }


    /* check if any allocation was done for group of nodes */
    for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
    {
        if ((bucketPtr->hwGroupOffsetHandle[i] != 0) && (bucketPtr->hwGroupOffsetHandle[i] != DMM_BLOCK_NOT_FOUND))
        {
            groopsAllocWasDone = GT_TRUE;
            break;
        }
        if (bucketPtr->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            break;
        }
    }
    if(((bucketUpdateMode != LPM_BUCKET_UPDATE_SHADOW_ONLY_E) && (bucketUpdateMode != LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)) &&
       (( resizeGroup == GT_TRUE) || (groopsAllocWasDone == GT_FALSE)))
    {
        /* first check if memory was already allocated for these changes */
        /* if yes , use only that memory! */
        if (shadowPtr->neededMemoryListLen > 0)
        {
            i = shadowPtr->neededMemoryCurIdx;

            if (i >= shadowPtr->neededMemoryListLen)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* should not happen */
            /* Add another check for size : new regular, new compressed,*/
            cpssOsMemCpy(tempHwAddrHandle,shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks,
                         sizeof(tempHwAddrHandle));
            shadowPtr->neededMemoryCurIdx++;
        }
        else
        {
            tempNextMemInfoPtr = lpmEngineMemPtrPtr[0];
            tmpStructsMemPool = lpmEngineMemPtrPtr[0]->structsMemPool;
            /*  this can only happen in lpm Delete, since in an insert the memory is preallocated.
               if we here gon must be reallocated */
            /* calculate new gon size : take relevant from subnodesIndexesPtr, subnodesSizesPtr*/

            for (i = 0; i < groupOfNodesNum; i++)
            {
                if (newBucBucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    if (subnodesIndexesPtr[i] == GT_FALSE)
                    {
                        continue;
                    }
                }
                if (lpmEngineMemPtrPtr[0]->structsMemPool == 0)
                {
                    tempHwAddrHandle[i] = DMM_BLOCK_NOT_FOUND;
                    cpssOsPrintf(" \n !!!!!!!There is no bank list for given octet\n");
                }
                else
                {
                    do
                    {

                        tempHwAddrHandle[i] = prvCpssDmmAllocate(tmpStructsMemPool,
                                                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * subnodesSizesPtr[i],
                                                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                        if (tempNextMemInfoPtr->nextMemInfoPtr == NULL)
                        {
                            break;
                        }
                        tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                        tmpStructsMemPool = tempNextMemInfoPtr->structsMemPool;

                    } while (tempHwAddrHandle[i]==DMM_BLOCK_NOT_FOUND);
                }
                if (tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND)
                {
                    /* we reached a max fragmentation in the PP's memory*/
                    /* this can only happen in lpm Delete, since in an insert */
                    /* the memory is preallocated. */
                    /* this forces use to use the swap memory , which we will*/
                    /* swap back after using */
                    if ( swapMemInUse == GT_TRUE)
                    {
                        if (secondSwapMemInUse == GT_FALSE) 
                        {
                            /* first swap is already occupied */
                            tempHwAddrHandle[i] = shadowPtr->secondSwapMemoryAddr;
                            secondSwapMemInUse = GT_TRUE;
                        }
                        else
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                        }
                    }
                    else
                    {
                        tempHwAddrHandle[i] = shadowPtr->swapMemoryAddr; 
                        swapMemInUse = GT_TRUE;

                    }
                    if (tempHwAddrHandle[i] == 0)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    /* we had a CPU Memory allocation error */
                    if(tempHwAddrHandle[i] == DMM_MALLOC_FAIL)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                    /*  set pending flag for future need */
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle[i]);
                }
            }
        }
    }
    else
    {
        cpssOsMemCpy(tempHwAddrHandle,bucketPtr->hwGroupOffsetHandle,sizeof(tempHwAddrHandle));
    }
    if ((isDestTreeRootBucket == GT_TRUE)||(isSrctTreeRootBucket == GT_TRUE))
    {
        /* root bucket */
        tempHwRootAddrHandle = bucketPtr->hwBucketOffsetHandle;
    }
    /* update the bucket's hw status */
    if (resizeGroup /*resizeBucket*/ == GT_TRUE)
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
            if (bucketUpdateMode != LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)
            {
                bucketPtr->bucketHwUpdateStat = PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_NEEDS_UPDATE_E;
            }
        }
    }

    /* Write the tables to PP's RAM for each device that share this LPM table. */

    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E))
    {
        pRange = bucketPtr->rangeList;
        /* Preparation of HW words.     */

        if ((resizeGroup /*resizeBucket*/ == GT_TRUE) ||
            (forceWriteWholeBucket == GT_TRUE) ||
            (bucketPtr->bucketHwUpdateStat ==
             PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E) ||
            (numOfRangesIsChanged == GT_TRUE) || (overwriteGroup == GT_TRUE))
            /*it could be that this bucket was resized but haven't been written
              the the HW*/
        {
            /* first get the bucket base addr for sibling buckets parent
               range mem address calculation */
            /* how to undersatnd in case of regular in which line change occured ????*/
            if (swapMemInUse == GT_TRUE)
            {
                /* if we're using the swap we will not record it but record the
                   bucket's old memory address */
                for (i=0; i<PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;i++ )
                {
                    if ((bucketPtr->hwGroupOffsetHandle[i] == 0) || (bucketPtr->hwGroupOffsetHandle[i] == DMM_BLOCK_NOT_FOUND))
                    {
                        continue;
                    }
                    tempAddr[i] = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[i])) +
                        memBlockBase;
                }
            }
            else
            {

                for (i=0; i<groupOfNodesNum;i++ )
                {
                    if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] ==0))
                    {
                        continue;
                    }
                    tempAddr[i] = (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[i])) + memBlockBase;
                }
            }
            cpssOsMemSet(groupOfNodes, 0, sizeof(groupOfNodes));
            for (i = 0; i < groupOfNodesNum; i++)
            {
                /* build hw data for affected group subnodes */
                if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] ==0))
                {
                    continue;
                }
                retVal = getMirrorGroupOfNodesDataAndUpdateRangesAddress(newBucBucketType,
                                                                         startSubNodeAddress[i],
                                                                         endSubNodeAddress[i],
                                                                         pRange,
                                                                         shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                                         &groupOfNodes[i]);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
            }
            /* Writing to RAM.*/
            do
            {
                for (j = 0; j < groupOfNodesNum; j++)
                {
                    /* build hw data for affected group subnodes */
                    if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                    {
                        continue;
                    }

                    tempAddr[j] = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j]) + memBlockBase;
                    for (i = 0; i < shareDevListLen; i++)
                    {
                        retVal =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                                  tempAddr[j],
                                                                  &groupOfNodes[j]);
                        if (retVal != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                        }
                    }
                }
                if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) ||
                    (swapMemInUse == GT_TRUE))
                {
                    cpssOsMemCpy(oldHwAddrHandle,bucketPtr->hwGroupOffsetHandle,sizeof(oldHwAddrHandle));
                    if (isDestTreeRootBucket == GT_TRUE)
                    {
                       /* oldRootHwAddrHandle = bucketPtr->hwBucketOffsetHandle; */
                    }
                     /* ok since the group of nodes bucket pointed on is updated in the HW, we can now
                        update the shadow for given bucket: group of nodes current bucket belongs will
                        be updated on higher level */

                    bucketPtr->bucketType = (GT_U8)newBucBucketType;

                    /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E we
                       don't update the pointer type since it represents that it
                       points to a src trie. (and not the actuall type of the
                       bucket it points to)*/
                    if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
                        *bucketTypePtr = (GT_U8)newBucBucketType;
                    for (j = 0; j < groupOfNodesNum; j++)
                    {
                        /* build hw data for affected group subnodes */
                        if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                       {
                            continue;
                        }
                        bucketPtr->hwGroupOffsetHandle[j] = tempHwAddrHandle[j];
                    }
					/* gon was written in swap area. The old connection is still working. In order to activate new connection, 
					the previous gon must be updated with swap pointer */ 
                    if((isDestTreeRootBucket == GT_TRUE) || (isSrctTreeRootBucket == GT_TRUE) )
                        bucketPtr->hwBucketOffsetHandle = tempHwRootAddrHandle;
                    
                    retVal = updateHwRangeDataAndGonPtr(parentNodeData);
                    if (retVal != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bad swap update");
                    }

                    /* now it's ok to free this bucket's old memory */
                    /* in multicast may be additional type is compareded like leaf_triger */
                    if ((freeOldMem == GT_TRUE) &&
                        (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                        (bucketPtr->bucketType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) )
                    {
                        for (j = 0; j < groupOfNodesNum; j++)
                        {
                            /* build hw data for affected group subnodes */
                            if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                            {
                                continue;
                            }
                            oldMemPool = DMM_GET_PARTITION(oldHwAddrHandle[j]);

                            /*  set pending flag for future need */
                            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(oldHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                            blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(oldHwAddrHandle[j]);

                            prvCpssDmmFree(oldHwAddrHandle[j]);
                        }
                        freeOldMem = GT_FALSE;
                    }
                }

                if (swapMemInUse == GT_FALSE)
                    break;

                /* if we reached here the swap memory is in use , and there is
                   a need to move and rewrite the bucket back where it was */

                if (oldMemPool == 0)
                {
                    /* should never happen since we just freed this mem*/
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                for (j = 0; j < groupOfNodesNum; j++)
                {
                    /* build hw data for affected group subnodes */
                    if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[j] ==0))
                    {
                        continue;
                    }
                    tempHwAddrHandle[j] = prvCpssDmmAllocate(oldMemPool,
                                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * subnodesSizesPtr[j],
                                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                    if ((tempHwAddrHandle[j] == DMM_BLOCK_NOT_FOUND)||(tempHwAddrHandle[j] == DMM_MALLOC_FAIL))
                    {
                        /* should never happen since we just freed this mem*/
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                    else
                    {
                        /* keep new tempHwAddrHandle -- in case of a fail should be freed
                        needToFreeAllocationInCaseOfFail = GT_TRUE;
                        tempHwAddrHandleToBeFreed = tempHwAddrHandle;*/

                        /*  set pending flag for future need */
                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwAddrHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(tempHwAddrHandle[j]);

                    }
                }
                /* indicate we're not using the swap memory anymore*/
                swapMemInUse = GT_FALSE;
                secondSwapMemInUse = GT_FALSE;
                /*we have new allocation and must rewrite pointer to current GON*/
                updatePtrToNewAllocatedAfterSwap = GT_TRUE;

            } while (GT_TRUE);
            /* check if it is root bucket we should update root node itself*/
        }
        else
        {
            /* there was no resize in octet group of nodes, but may be group of nodes content
               was changed and update is needed */
            retVal = lpmFalconUpdateRangeInHw(bucketPtr,pRange,
                                              shadowPtr);
            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
        if ( (isDestTreeRootBucket == GT_TRUE) || (isSrctTreeRootBucket == GT_TRUE) )
        {
            /* destination root bucket or src root bucket is going to be updated*/
            if ((rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) ||
                (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E) ||
                (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_E) )
            {
                 GT_UINTPTR oldHwGroupOffsetHandle[6];
                 cpssOsMemCpy(oldHwGroupOffsetHandle,bucketPtr->hwGroupOffsetHandle,sizeof(oldHwGroupOffsetHandle));
                    /* it means new ranges were added into root node*/
                cpssOsMemSet(&groupOfNodes,0,sizeof(groupOfNodes));
                
                if((isSrctTreeRootBucket == GT_TRUE)&&(rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E))
                {
                    if (shadowPtr->neededMemoryListLen > 0)
                    {
                        i = shadowPtr->neededMemoryCurIdx;
                        if (i >= shadowPtr->neededMemoryListLen)
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* should not happen */

                        tempHwRootAddrHandle = shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0];
                        shadowPtr->neededMemoryCurIdx++;
                    }
                }
                else
                    if((isSrctTreeRootBucket == GT_TRUE)&&(rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E))
                    {
                        /* mc src delete : from regular to compressed*/
                        /* new alloc for compressed in octet 0 memory and free old memory*/
                        if ( (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) &&
                             (newBucBucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E) )
                        {
                            PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC     **lpmOctet0MemPtr;
                            lpmOctet0MemPtr = lpmEngineMemPtrPtr-1;
                            /* allocate memory */
                            tempHwRootAddrHandle = prvCpssDmmAllocate(lpmOctet0MemPtr[0]->structsMemPool,
                                                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * 1,
                                                                 DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                            if (tempHwRootAddrHandle == DMM_BLOCK_NOT_FOUND)
                            {
                                /* there is no free place */
                                /* in this case swap area must be used TBD */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_PP_MEM, LOG_ERROR_NO_MSG);
                            }

                        }
                        else
                        {
                            cpssOsPrintf(" Bad data state: contradiction between backet data and delete logic\n");
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Bad data state: contradiction between backet data and delete logic"); /* should not happen */
                        }
                    }
                    else
                    {
                        tempHwRootAddrHandle = bucketPtr->hwBucketOffsetHandle;
                    }
                
                /* We are doing update of root node due to some of its GONs were changed.
                   Update GON's offset information */
                for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                {
                    if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] ==0))
                    {
                      continue;
                    }
                    bucketPtr->hwGroupOffsetHandle[i] = tempHwAddrHandle[i];
                }
                if (isDestTreeRootBucket == GT_TRUE)
                {
                    retVal = lpmFalconFillRegularData(bucketPtr, 
                                                      shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                      &groupOfNodes[0].regularNodesArray[0]);
                }
                else
                {
                    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT oldBucBucketType = bucketPtr->bucketType;
                    bucketPtr->bucketType = newBucBucketType;
                    if(newBucBucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                    {
                        retVal = lpmFalconFillRegularData(bucketPtr, 
                                                          shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                          &groupOfNodes[0].regularNodesArray[0]);
                    }
                    else
                    {
                        retVal = lpmFalconFillCompressedData(bucketPtr, 
                                                             shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                             &groupOfNodes[0].compressedNodesArray[0]);
                    }
                    bucketPtr->bucketType = oldBucBucketType;
                }
                
                if (retVal != GT_OK)
                {
                    return retVal;
                }
                for (i = 0; i < shareDevListLen; i++)
                {
                    /* write regular root node */
                    retVal = prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[i],
                                                             PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(tempHwRootAddrHandle),
                                                             &groupOfNodes[0]);
                    if (retVal != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                }
                cpssOsMemCpy(bucketPtr->hwGroupOffsetHandle,oldHwGroupOffsetHandle,sizeof(oldHwGroupOffsetHandle));
                if(isSrctTreeRootBucket == GT_TRUE)
                {
                    /* bucket was resized */
                    if ( ( (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) || 
                           (rootBucketState == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E))&&
                         (bucketPtr->hwBucketOffsetHandle != 0) )
                    {
                        /* Free the old group of nodes */
                        retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwBucketOffsetHandle,
                                                                 PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                                 freeMemListPtr,shadowPtr);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }
                    }
                    bucketPtr->hwBucketOffsetHandle = tempHwRootAddrHandle;

                }

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
        /* in multicast we have new leaf type leaf_triger */
        freeOldMem = ((freeOldMem) &&
                      (/*resizeBucket*/resizeGroup == GT_TRUE) &&
                      (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
                      (bucketPtr->bucketType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ); /* not a new bucket */

        if (freeOldMem)
        {
         /* if we are in delete and regular turned to compressed all 6 group of nodes must be freed*/
            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
            {
                if ( (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) &&
                     (newBucBucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E) )
                {
                    if (bucketPtr->hwGroupOffsetHandle[i] == 0)
                    {
                        /* must not happend !!!!*/
                        cpssOsPrintf(" FAIL!!! regular node with no group offsets!!!!\n");
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] ==0) ||
                        (bucketPtr->hwGroupOffsetHandle[i] == 0))
                    {
                        continue;
                    }
                }
                /* Free the old group of nodes */
                retVal = prvCpssDxChLpmRamMemFreeListMng(bucketPtr->hwGroupOffsetHandle[i],
                                                         PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
                                                         freeMemListPtr,shadowPtr);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
                if (bucketPtr->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    break;
                }
            }
        }
        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
        {
            if ( (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) &&
                 (newBucBucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E) )
            {
                bucketPtr->hwGroupOffsetHandle[i] = 0;
            }

            if ((tempHwAddrHandle[i] == DMM_BLOCK_NOT_FOUND) || (tempHwAddrHandle[i] ==0))
            {
              continue;
            }
            bucketPtr->hwGroupOffsetHandle[i] = tempHwAddrHandle[i];
        }
        if (updatePtrToNewAllocatedAfterSwap == GT_TRUE) 
        {
            retVal = updateHwRangeDataAndGonPtr(parentNodeData);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "bad after swap update");
            }
            updatePtrToNewAllocatedAfterSwap = GT_FALSE;
        }
    }

    /* in LPM_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E mode , this shadow update part
       already been done */
    if ((bucketUpdateMode == LPM_BUCKET_UPDATE_HW_AND_MEM_ALLOC_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_MEM_ONLY_E) ||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_SHADOW_ONLY_E)||
        (bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E))
    {
        /* update the bucket's info info */
        bucketPtr->bucketType = newBucBucketType;

        /* update the range lower Level bucket type */
        /* in case of PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E we don't update
           the lower Level bucket type (pointer type) since it represents that
           it points to a src trie. (and not the actuall type of the bucket it
           points to)*/
        if ((GT_U32)*bucketTypePtr != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
            *bucketTypePtr = newBucBucketType;
        if (bucketUpdateMode == LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E)
        {
            cpssOsMemSet(bucketPtr->hwGroupOffsetHandle, 0, sizeof(bucketPtr->hwGroupOffsetHandle));
        }
    }

    return GT_OK;
}

/*******************************************************************************
* checkGroupOfNodesAllocationForOctet
*
* DESCRIPTION:
*       Check if allocation was done for given octet
*
* INPUTS:
*       octetIndex          - octet number
*       shadowPtr           - lpm shadow
*
* OUTPUTS:
*       isAllocatedPtr      - GT_TRUE: allocation is done
*                             GT_FALSE: otherwise
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS checkGroupOfNodesAllocationForOctet
(
    IN  GT_U32                               octetIndex,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    OUT GT_BOOL                              *isAllocatedPtr
)
{

    GT_U32 i,j;
    *isAllocatedPtr = GT_FALSE;
    for (i = 0; i <shadowPtr->neededMemoryListLen; i++ )
    {
        if (shadowPtr->neededMemoryBlocksInfo[i].octetId == octetIndex)
        {
            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
            {
               if( shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j] != DMM_BLOCK_NOT_FOUND)
               {
                   *isAllocatedPtr = GT_TRUE;
                   return GT_OK;
               }
            }
        }
    }
    return GT_OK;
}


/*******************************************************************************
* updateHwRangeDataAndGonPtr
*
* DESCRIPTION:
*       This function updates lpm node 
*
* INPUTS:
*       parentNodeDataPtr     - pointer to the bucket to parent node parameters.
*
* OUTPUTS:
*       None
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
static GT_STATUS updateHwRangeDataAndGonPtr
(
    IN PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC    *parentNodeDataPtr
)
{
    GT_STATUS   retVal;
    GT_U8 subNodeId = 0;
    GT_UINTPTR groupStartAddrHandle = 0;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *pRange = NULL;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *bucketPtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *nextBucketPtr = NULL;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      nextBucketBucketType;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr = NULL;
    static PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   groupOfNodes;
    GT_U8  *shareDevsList;  /* List of devices sharing this LPM structure   */
    GT_U32 shareDevListLen;
    GT_U32 tempAddr;
    GT_U32 j = 0;

    cpssOsMemSet(&groupOfNodes, 0, sizeof(groupOfNodes));
    bucketPtr = parentNodeDataPtr->bucketPtr;
    pRange = parentNodeDataPtr->pRange; 
    shadowPtr = parentNodeDataPtr->shadowPtr;
    shareDevsList   = shadowPtr->workDevListPtr->shareDevs;
    shareDevListLen = shadowPtr->workDevListPtr->shareDevNum;

    /* there are 2 cases update range in root node and other nodes */
    /* in case of regular no more than 2 nodes can be updated simultaneously */
   
    /* calculate group of nodes start address*/
    if (parentNodeDataPtr->isRootNode == GT_TRUE) 
    {
        nextBucketPtr = parentNodeDataPtr->bucketPtr;
        nextBucketBucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
    }
    else
    {
         nextBucketPtr = pRange->lowerLpmPtr.nextBucket;
         nextBucketBucketType = pRange->lowerLpmPtr.nextBucket->bucketType;
    }
    if (parentNodeDataPtr->isRootNode == GT_FALSE)
    {
        switch (bucketPtr->bucketType) 
        {
        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            groupStartAddrHandle = bucketPtr->hwGroupOffsetHandle[0];
            break;
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            subNodeId = pRange->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
            groupStartAddrHandle = bucketPtr->hwGroupOffsetHandle[subNodeId];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }
    }

    switch (nextBucketBucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
        retVal =  lpmFalconFillCompressedData(nextBucketPtr,
                                              shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                              &groupOfNodes.compressedNodesArray[0]);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        break;
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        retVal =  lpmFalconFillRegularData(nextBucketPtr,
                                           shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                           &groupOfNodes.regularNodesArray[0]);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }
    if (parentNodeDataPtr->isRootNode == GT_FALSE)
    {
        /* now calculate offset inside of group of nodes for node this range pointed on */
        retVal = lpmFalconGetHwNodeOffsetInsideGroupOfNodes(bucketPtr,
                                                            pRange->startAddr,
                                                            startSubNodeAddress[subNodeId],
                                                            endSubNodeAddress[subNodeId],
                                                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(groupStartAddrHandle),
                                                            &tempAddr);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        tempAddr = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);
    }
    /* tempAddr shows where this node starts in group of nodes*/
    for (j = 0; j < shareDevListLen; j++)
    {
        retVal =  prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[j],
                                                  tempAddr,
                                                  &groupOfNodes);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }


    return GT_OK;
}

/*******************************************************************************
* lpmFalconCheckIfLastLevelBucketEmbdLeaves
*
* DESCRIPTION:
*       Check if this embedded leaves bucket from the last level .
*
* INPUTS:
*       bucketPtr         - Pointer to the  bucket.
*
* OUTPUTS:
*       lastLevelBacketEmbLeavesPtr      - (GT_TRUE) this is last level embeded leaves bucket.
*
* RETURNS:
*       None.
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_VOID lpmFalconCheckIfLastLevelBucketEmbdLeaves
(
  IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
  OUT   GT_BOOL                                       *lastLevelBacketEmbLeavesPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *rangePtr = NULL;
    GT_U32  i = 0;
    *lastLevelBacketEmbLeavesPtr = GT_FALSE;
    rangePtr = bucketPtr->rangeList;

    while (rangePtr != NULL)
    {
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )
        {
            i++;
        }
        else
        {
            /* it is not last level */
            *lastLevelBacketEmbLeavesPtr = GT_FALSE;
            return;
        }
        rangePtr = rangePtr->next;
    }
    if (i <= 3)
    {
        *lastLevelBacketEmbLeavesPtr = GT_TRUE;
    }
    return;
}
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
    GT_U32 numOfNewHwRanges = 0;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC newRange;/* the new created range from the insert */
    GT_STATUS retVal= GT_OK;           /* functions returned values.           */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pRange;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *parentRangeToUpdateDueToLowerLevelResizePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *parentRangeToUpdateDueToSecondLowerLevelResizePtr = NULL;
    GT_U32  parentRangeLevel = 99; /* initializing with and invalid level*/
    GT_U32  parentSecondRangeLevel = 99; /* initializing with and invalid level*/

    GT_BOOL indicateSiblingUpdate = GT_FALSE;
    GT_U32  level = 0;
    GT_BOOL goDown;
    GT_BOOL isDestTreeRootBucket = GT_FALSE;
    GT_BOOL isSrctTreeRootBucket = GT_FALSE;

    LPM_BUCKET_UPDATE_MODE_ENT bucketUpdateMode;

    static LPM_ROOT_BUCKET_UPDATE_ENT               rootBucketUpdateState[MAX_LPM_LEVELS_CNS];
    static GT_BOOL                                  resizeGroup[MAX_LPM_LEVELS_CNS];
    static GT_BOOL                                  doHwUpdate[MAX_LPM_LEVELS_CNS];
    static GT_BOOL                                  newRangeIsAdded[MAX_LPM_LEVELS_CNS];
    static GT_BOOL                                  overriteIsDone[MAX_LPM_LEVELS_CNS];
    static PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtrArry[MAX_LPM_LEVELS_CNS];
    static CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      *pointerTypePtrArray[MAX_LPM_LEVELS_CNS];
    static PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRangeArray[MAX_LPM_LEVELS_CNS];

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  **currBucketPtr = bucketPtrArry;
    GT_U8                                    *currAddrByte = addrCurBytePtr;
    GT_U32                                   currPrefix = prefix;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC       **currLpmEngineMemPtr = lpmEngineMemPtrPtr;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      **currBucketPtrType = pointerTypePtrArray;
    LPM_ROOT_BUCKET_UPDATE_ENT               *rootBucketUpdateStatePtr = rootBucketUpdateState;
    GT_BOOL                                  *resizeGroupPtr = resizeGroup;
    GT_BOOL                                  *newRangeIsAddedPtr = newRangeIsAdded;
    GT_BOOL                                  *overriteDonePtr = overriteIsDone;
    GT_BOOL                                  *doHwUpdatePtr = doHwUpdate;
    GT_BOOL                                  isAllocated = GT_FALSE;
    GT_U32                                   i = 0;
    GT_U32                                   j = 0;
    #if 0
    GT_UINTPTR                               rollBackHwBucketOffsetHandle = 0;
    LPM_BUCKET_UPDATE_MODE_ENT               rollBackBucketUpdateMode = LPM_BUCKET_UPDATE_NONE_E;
    GT_BOOL                                  rollBackResizeBucket = GT_FALSE;
    #endif
    GT_BOOL                                  lastLevelBacketEmbLeaves = GT_FALSE;
    GT_U32 subnodesSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_BOOL subnodesIndexes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};

    cpssOsMemSet(&newRange,0,sizeof(newRange));

    /* zero the arrays */
    cpssOsMemSet(rootBucketUpdateState,LPM_ROOT_BUCKET_UPDATE_NONE_E,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(resizeGroup,GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(doHwUpdate,GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(bucketPtrArry,0,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(newRangeIsAdded,GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(overriteIsDone,GT_FALSE,sizeof(GT_BOOL)*MAX_LPM_LEVELS_CNS);


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
        lastLevelBacketEmbLeaves = GT_FALSE;
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
                    if (!(((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)))
                    {
                        /* it is not the end level. So nextHop overwrite should be done on the next level */
                         *doHwUpdatePtr = GT_TRUE;
                         *overriteDonePtr = GT_TRUE;
                    }
                    retVal = insert2Trie(*currBucketPtr, startAddr, 
                                         currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,nextPtr);
                    if (retVal != GT_OK)
                    {
                        /* failed on last level*/
                        for (i=1; i <= level; i++)
                        {
                            if (*(pointerTypePtrArray[i]) == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
                            {
                                *(pointerTypePtrArray[i]) = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
                            }
                        }
                        if (shadowPtr->neededMemoryListLen > 0)
                        {
                            for (i = shadowPtr->neededMemoryCurIdx; i < shadowPtr->neededMemoryListLen; i++)
                            {
                                if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                {
                                    for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                    {
                                        prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                    }
                                }
                                else
                                    prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
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
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* Insert the given addr. to the next level.    */
                        if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                        {
                            /* Create a new bucket with the dominant next_pointer*/
                            /* value of the upper level as the default route.    */
                            newRange.lowerLpmPtr.nextBucket =
                                createNewSip6Bucket((*pRange)->lowerLpmPtr.nextHopEntry,
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
                                        if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                        {
                                            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                            {
                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                            }
                                        }
                                        else
                                            prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
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
                                (nextPtrType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) )
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
                        retVal = sip6SplitRange(pRange,startAddr,endAddr,
                                                newPrefixLength,&newRange,
                                                PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,updateOldPtr,
                                                pointerTypePtrArray[level],&numOfNewRanges,&numOfNewHwRanges,
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
                                    if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                    {
                                        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                        {
                                            prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                        }
                                    }
                                    else
                                        prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                                }
                            }
                            return retVal;
                        }

                        /* a change happend to the structure of the bucket
                           clear the cash*/
                        (*currBucketPtr)->rangeCash = NULL;
                        /* check if group is also should be resized */
                        if ((currBucketPtr == bucketPtrArry)     &&
                            (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) )
                        {
                            if (isMcSrcTree == GT_FALSE) 
                            {
                                /* for root bucket */
                                *rootBucketUpdateStatePtr = LPM_ROOT_BUCKET_UPDATE_HW_E;
                            }
                            else
                            {
                                /* mc source root */
                                /* check if resized*/
                                retVal = checkGroupOfNodesAllocationForOctet(level,shadowPtr,&isAllocated);
                                if (retVal != GT_OK)
                                {
                                    return retVal;
                                }
                                if (isAllocated == GT_TRUE)
                                {
                                    *rootBucketUpdateStatePtr = LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E;
                                }
                                else
                                {
                                    *rootBucketUpdateStatePtr = LPM_ROOT_BUCKET_UPDATE_HW_E;
                                }
                            }
                        }

                        if (/*numOfNewRanges*/numOfNewHwRanges != 0)
                        {
                            *newRangeIsAddedPtr = GT_TRUE;
                        }
                        if ( ((*currBucketPtr)->numOfRanges == 1) ||
                             ( ((*currBucketPtr)->numOfRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) &&
                                (((*currBucketPtr)->numOfRanges + numOfNewRanges) > MAX_NUMBER_OF_COMPRESSED_RANGES_CNS)) )
                        {
                            /* new bucket on this level or bucket turn to regular from compressed.
                               This is clear that  groop of nodes new/turned bucket pointed on
                               must be resized.
                               Also the group of nodes size this bucket belongs to is changed.
                               The previous level pointed on this group of nodes must be aware
                               that size of its group are changed and groop needs a resize*/
                             if (((*currBucketPtr)->numOfRanges == 1) &&
                                (currPrefix <= PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS))
                             {
                                /* new bucket, last level --> compressed with embedded leaves*/
                                 *resizeGroupPtr = GT_FALSE;
                             }
                             else
                             {
                                 *resizeGroupPtr = GT_TRUE;
                             }
                             if (!((currBucketPtr == bucketPtrArry)     &&
                                 (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) ))
                             {
                                /* we are not in root */
                                /* must to check if previous level really need resize */
                                retVal = checkGroupOfNodesAllocationForOctet(level,shadowPtr,&isAllocated);
                                if (retVal != GT_OK)
                                {
                                    return retVal;
                                }
                                if (isAllocated == GT_TRUE)
                                {
                                    *(resizeGroupPtr - 1) = GT_TRUE;
                                    /* if previous level is root*/
                                    if ( (level-1 == 0) && 
                                         (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) )
                                    {
                                        /* for root bucket */
                                        if (isMcSrcTree == GT_FALSE)
                                        {
                                            *(rootBucketUpdateStatePtr - 1) = LPM_ROOT_BUCKET_UPDATE_HW_E; 
                                        }
                                        else
                                        {
                                            if ( *(rootBucketUpdateStatePtr - 1) == LPM_ROOT_BUCKET_UPDATE_NONE_E) 
                                            {
                                                *(rootBucketUpdateStatePtr - 1) = LPM_ROOT_BUCKET_UPDATE_HW_E; 
                                            }
                                        }
                                        
                                    }
                                }
                            }
                        }
                        else
                        {
                            /* in order to know whether group of nodes this bucket pointed on will be resized
                               check allocation info for given level */
                            retVal = checkGroupOfNodesAllocationForOctet(level+1,shadowPtr,&isAllocated);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }
                            if (isAllocated == GT_TRUE)
                            {
                                *resizeGroupPtr = GT_TRUE;
                            }
                            else
                            {
                                if (numOfNewHwRanges == 0)
                                {
                                    /* owerrite is done: group of nodes must be updated */
                                    *overriteDonePtr = GT_TRUE;
                                }
                            }
                        }
                        *doHwUpdatePtr = GT_TRUE;
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
                        if ((parentRangeToUpdateDueToSecondLowerLevelResizePtr != NULL) &&
                            ((parentSecondRangeLevel +2) == level))
                        {
                            if ( *(resizeGroupPtr - 1) == GT_TRUE)
                            {
                                parentRangeToUpdateDueToSecondLowerLevelResizePtr->updateRangeInHw =
                                    GT_TRUE;
                                /* indicate to the upper level that it needs
                                   a HW update*/
                                *(doHwUpdatePtr - 2) = GT_TRUE;
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
                            parentSecondRangeLevel = parentRangeLevel;
                            parentRangeToUpdateDueToSecondLowerLevelResizePtr =
                                parentRangeToUpdateDueToLowerLevelResizePtr;
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
            rootBucketUpdateStatePtr++;
            resizeGroupPtr++;
            newRangeIsAddedPtr++;
            overriteDonePtr++;
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

            /* here additional check for last level node must be done : if this node is compressed */
            /* and number of ranges no more than 3*/



            if ( currPrefix <= PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
            {
                /* it is last level but it can't be root bucket */
                if( !((level == 0) && (currBucketPtr == bucketPtrArry) && (isMcSrcTree == GT_FALSE) &&
                ((*currBucketPtr)->hwBucketOffsetHandle != 0)                 &&
                (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) ) )
                {
                    if ((*currBucketPtr)->numOfRanges <= 3)
                    {
                        lpmFalconCheckIfLastLevelBucketEmbdLeaves(*currBucketPtr,
                                                                  &lastLevelBacketEmbLeaves);
                        /* so there must be comressed with 2 or 3  embedded leaves leaves */
                        if (lastLevelBacketEmbLeaves == GT_TRUE)
                        {
                            /* such node is not pointing on GON */
                            *resizeGroupPtr = GT_FALSE;
                            *doHwUpdatePtr = GT_FALSE;
                            if ((*currBucketPtr)->hwGroupOffsetHandle[0] != 0)
                            {
                                cpssOsPrintf(" BAD STATE !!!!, hwGroupOffsetHandle[0] != 0 on last level\n ");
                            }
                        }
                    }
                }
            }


            /* if the bucket need resizing always update the HW */
            if ((*rootBucketUpdateStatePtr == LPM_ROOT_BUCKET_UPDATE_HW_E) || 
                (*rootBucketUpdateStatePtr == LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_RESIZE_E) )
            {
                *doHwUpdatePtr = GT_TRUE;
            }
            if ((currBucketPtr == bucketPtrArry)     &&
                (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) )
            {
                if (isMcSrcTree == GT_FALSE) 
                {
                    isDestTreeRootBucket = GT_TRUE;
                    isSrctTreeRootBucket = GT_FALSE;
                }
                else
                {
                    isSrctTreeRootBucket = GT_TRUE;
                    isDestTreeRootBucket = GT_FALSE;
                }
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
                    if (lastLevelBacketEmbLeaves == GT_TRUE)
                    {
                        bucketUpdateMode = LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E;
                    }
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
                             if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                             {
                                 for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                 {
                                     prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                 }
                             }
                             else
                                 prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                         }
                     }
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* save rollback information */
            #if 0
            rollBackHwBucketOffsetHandle = (*currBucketPtr)->hwBucketOffsetHandle;
            rollBackBucketUpdateMode = bucketUpdateMode;
            rollBackResizeBucket = *resizeBucketPtr;
           #endif
             /* The the shadow structures to the RAM.    */
            retVal = updateMirrorGroupOfNodes(*currBucketPtr,bucketUpdateMode,
                                              isDestTreeRootBucket,isSrctTreeRootBucket,
                                              *rootBucketUpdateStatePtr,*resizeGroupPtr,*newRangeIsAddedPtr,
                                              *overriteDonePtr,GT_FALSE,
                                              indicateSiblingUpdate,
                                              currLpmEngineMemPtr,
                                              *currBucketPtrType,shadowPtr,NULL,
                                              &subnodesIndexes[0],&subnodesSizes[0]);
            if (retVal != GT_OK)
            {
                /* TBD Roolback is not implemented yet for SIP6
                   When we implement it, the following code should be
                   investigated and counters should be updtaed properly */
#if 0
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
                                for (i = shadowPtr->neededMemoryCurIdx-1; i < shadowPtr->neededMemoryListLen; i++)
                                {
                                    if((shadowPtr->neededMemoryBlocks[i]!=shadowPtr->swapMemoryAddr)&&
                                       (shadowPtr->neededMemoryBlocks[i]!=shadowPtr->secondSwapMemoryAddr))
                                    {
                                        if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                        {
                                            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                            {
                                                /*  set pending flag for future need */
                                                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);

                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                            }
                                         }
                                         else
                                         {
                                           /*  set pending flag for future need */
                                                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);

                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                                         }
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
                {
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
                                        if (shadowPtr->neededMemoryBlocksInfo[i].regularNode == GT_TRUE)
                                        {
                                            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
                                            {
                                                /*  set pending flag for future need */
                                                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);

                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[j]);
                                            }
                                         }
                                         else
                                         {
                                           /*  set pending flag for future need */
                                                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                                                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                                                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);

                                                prvCpssDmmFree(shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocks[0]);
                                         }
                                    }
                                }
                            }
                        }
                    }
#endif
                return retVal;
            }

            /* update partition block */
            if ((*currBucketPtr)->hwBucketOffsetHandle!=0)
            {
                SET_DMM_BLOCK_PROTOCOL((*currBucketPtr)->hwBucketOffsetHandle, protocol);
                SET_DMM_BUCKET_SW_ADDRESS((*currBucketPtr)->hwBucketOffsetHandle, (*currBucketPtr));
            }
            for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
            {
                if ((*currBucketPtr)->hwGroupOffsetHandle[j]!=0)
                {
                    SET_DMM_BLOCK_PROTOCOL((*currBucketPtr)->hwGroupOffsetHandle[j], protocol);
                    SET_DMM_BUCKET_SW_ADDRESS((*currBucketPtr)->hwGroupOffsetHandle[j], (*currBucketPtr));
                }
            }

            /* move back in the arrays*/
            currBucketPtr--;
            currBucketPtrType--;
            doHwUpdatePtr--;
            rootBucketUpdateStatePtr--;
            resizeGroupPtr--;
            newRangeIsAddedPtr--;
            overriteDonePtr--;

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


/*******************************************************************************
* lpmFalconGetSplitMethod
*
* DESCRIPTION:
*     Return the split method of a given range in the lpm tree.
*
* INPUTS:
*       bucketType            - type of bucket
*       numOfBucketRangesPtr  - Number of bucket ranges before the split.
*       rangePtr              - Pointer to the range to be splitted.
*       startAddr             - The low address of the new range.
*       endAddr               - The high address of the new range.
*       prefixLength          - The length of the dominant prefix of the new range
*       levelPrefix           - The current lpm level prefix.
*
* OUTPUTS:
*       numOfBucketRangesPtr   - Number of bucket ranges after the split.
*       numOfBitVectorRangesPtr -
*       splitMethodPtr         - The mode according to which the range should be split.
*
* RETURNS:
*     GT_OK     - on success.
*
* COMMENTS:
*     None.
*
*******************************************************************************/
static GT_STATUS lpmFalconGetSplitMethod
(
    IN      CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT    bucketType,
    INOUT   GT_U32                                 *numOfBucketRangesPtr,
    IN      PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *rangePtr,
    IN      GT_U8                                  startAddr,
    IN      GT_U8                                  endAddr,
    IN      GT_U8                                  levelPrefix,
    OUT     PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT *splitMethodPtr,
    OUT     GT_U32                                 *numOfBitVectorLineRangesPtr
)
{
    GT_U8  endRange, realEndRange;           /* Index by which this range ends.      */
    GT_U8  rangeStatrtAddr;
    GT_U8 groupSubNodeId;
    GT_U8 startSubNodeId;
    GT_U8 endSubNodeId;
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT bvSplitMethod;
    GT_U32 oldNumOfRanges = *numOfBucketRangesPtr;
    rangeStatrtAddr = rangePtr->startAddr;
    endRange = (GT_U8)(LAST_RANGE_MAC(rangePtr)? ((1 << levelPrefix) - 1) :
    ((rangePtr->next->startAddr) - 1));
    realEndRange = endRange;

    *splitMethodPtr = (((startAddr > rangeStatrtAddr) ? 1 : 0) +
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
    if ((bucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E) ||
        (bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E) ||
        (bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
        (bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
    {
        if (((oldNumOfRanges < MIN_NUMBER_OF_BIT_VECTOR_RANGES_CNS)&& (*numOfBucketRangesPtr >= MIN_NUMBER_OF_BIT_VECTOR_RANGES_CNS)) ||
            ((startAddr != endAddr)&& (oldNumOfRanges >= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS - 1 ))) 
        {
            /* node is going to be regular */
            bucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
        }
        else
        {
            return GT_OK; 
        }
    }

    if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        startSubNodeId = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        endSubNodeId = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        for (groupSubNodeId = startSubNodeId; groupSubNodeId <= endSubNodeId; groupSubNodeId++ ) 
        {
            if (startSubNodeAddress[groupSubNodeId] > realEndRange) 
            {
                /* we are checking how giving leaf range impact on current range
                   and moving on subnodes that leaf range includes*/
                /* In case when start subnode address bigger then end of current range zone of impact is over*/
                break;
            }
            if (endSubNodeAddress[groupSubNodeId] < rangePtr->startAddr) 
            {
                continue;
            }
            if (startAddr <= startSubNodeAddress[groupSubNodeId]) 
            {
                rangeStatrtAddr =startSubNodeAddress[groupSubNodeId];
            }
            if ( (LAST_RANGE_MAC(rangePtr) == GT_TRUE)||
                 (((rangePtr->next->startAddr) - 1) > (GT_U8)endSubNodeAddress[groupSubNodeId]) )
            {
                endRange = (GT_U8)endSubNodeAddress[groupSubNodeId];
            }
            else
            {
                endRange = (rangePtr->next->startAddr) - 1;
            }
            bvSplitMethod = (((startAddr > rangeStatrtAddr) ? 1 : 0) +
                          ((endAddr < endRange) ? 2 : 0));
            switch (bvSplitMethod)
            {
                case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E):
                case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E):
                    numOfBitVectorLineRangesPtr[groupSubNodeId] += 1;
                    break;

                case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E):
                    numOfBitVectorLineRangesPtr[groupSubNodeId] += 2;
                    break;

                case (PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E):
                    numOfBitVectorLineRangesPtr[groupSubNodeId] += 0;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        
    }

    return GT_OK;
}


/*******************************************************************************
* lpmFalconGetGroupOfNodesItemsForCompressed
*
* DESCRIPTION:
*       Gets number group of nodes components for compressed node.
*
* INPUTS:
*       rangeListPtr      - Pointer to the bucket ranges .
*
* OUTPUTS:
*       regularNodesNumberPtr     - pointer to regular nodes number
*       compressedNodesNumberPtr  - pointer to compressed nodes number
*       leafNodesNumberPtr        - pointer to leaf nodes number
*
* RETURNS:
*       GT_OK               - on success,
*       GT_FAIL             - Otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS lpmFalconGetGroupOfNodesItemsForCompressed
(
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC        *rangeListPtr,
    OUT   GT_U32                                        *regularNodesNumberPtr,
    OUT   GT_U32                                        *compressedNodesNumberPtr,
    OUT   GT_U32                                        *leafNodesNumberPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    rangePtr = rangeListPtr;
    while (rangePtr != NULL)
    {
        if(rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            regularNodesNumberPtr[0]++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            compressedNodesNumberPtr[0]++;
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )        
        {
            leafNodesNumberPtr[0]++;
        }
        rangePtr = rangePtr->next;
    }

    return GT_OK;
}

/*******************************************************************************
* lpmFalconGetGroupOfNodesItemsForRegular
*
* DESCRIPTION:
*       Gets number group of nodes components for regular node.
*
* INPUTS:
*       rangeListPtr      - Pointer to the bucket ranges .
*       startAddr         - new range start address
*       endAddr           - new range end address
*
* OUTPUTS:
*       isStartAddrMatchRegionPtr - if start of new range already points on lpm node
*       isEndAddrMatchRegionPtr   - if end of new range already points on lpm node
*       regularNodesNumberPtr     - pointer to regular nodes number
*       compressedNodesNumberPtr  - pointer to compressed nodes number
*       leafNodesNumberPtr        - pointer to leaf nodes number
*
* RETURNS:
*       GT_OK               - on success,
*       GT_FAIL             - Otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS lpmFalconGetGroupOfNodesItemsForRegular
(
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC        *rangeListPtr,
    IN    GT_U32                                        startAddr,
    IN    GT_U32                                        endAddr,
    OUT   GT_BOOL                                       *isStartAddrMatchRegionPtr,
    OUT   GT_BOOL                                       *isEndAddrMatchRegionPtr,
    OUT   GT_U32                                        *regularNodesNumberPtr,
    OUT   GT_U32                                        *compressedNodesNumberPtr,
    OUT   GT_U32                                        *leafNodesNumberPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangePtr = NULL;
    GT_U32 startNodeIndex, endNodeIndex,endRangeAddr,i;
    rangePtr = rangeListPtr;
    /* In regular node we have 6 different subnodes */
    while (rangePtr != NULL)
    {
        if (rangePtr->startAddr == startAddr)
        {
            *isStartAddrMatchRegionPtr = GT_TRUE;
        }
        startNodeIndex = rangePtr->startAddr / NUMBER_OF_RANGES_IN_SUBNODE;
        if (rangePtr->next == NULL)
        {
            endRangeAddr = 255;

        }
        else
        {
            endRangeAddr = rangePtr->next->startAddr - 1;
        }
        endNodeIndex = endRangeAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        if (endRangeAddr == endAddr)
        {
            *isEndAddrMatchRegionPtr = GT_TRUE;
        }
        /* check subnodes ths range belongs to */
        if (rangePtr->pointerType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                regularNodesNumberPtr[i]++;
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)                         ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)   ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
        {
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                compressedNodesNumberPtr[i]++;
            }
        }
        if( (rangePtr->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
            (rangePtr->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  ||
            (rangePtr->pointerType == 0xff) )        
        {
            for (i = startNodeIndex; i <= endNodeIndex; i++ )
            {
                leafNodesNumberPtr[i]++;
            }
        }
        rangePtr = rangePtr->next;
    }

    return GT_OK;
}

/*******************************************************************************
* lpmFalconCheckIfCompressedWithEmbeddedLeaves
*
* DESCRIPTION:
*       Check if compressed node with embedded leaves.
*
* INPUTS:
*       numOfRanges      - number of ranges in current compressed node
*       numOfLeaves      - number of leaves in current compressed node
*
* OUTPUTS:
*       isCompressedWithEmbLeavesPtr     - pointer to type of compressed node
*
* RETURNS:
*       None.
*
* COMMENTS:
*
*
*******************************************************************************/
GT_VOID lpmFalconCheckIfCompressedWithEmbeddedLeaves
(
    IN    GT_U32 numOfRanges,
    IN    GT_U32 numOfLeaves,
    OUT   GT_BOOL *isCompressedWithEmbLeavesPtr,
    OUT   CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT *compressNodeTypePtr
)
{
    *isCompressedWithEmbLeavesPtr = GT_FALSE;
    if ((numOfRanges <=7) && (numOfLeaves == 1))
    {
        *isCompressedWithEmbLeavesPtr = GT_TRUE;
        *compressNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E;
    }else
        if ((numOfRanges <=5) && (numOfLeaves == 2))
        {
            *isCompressedWithEmbLeavesPtr = GT_TRUE;
            *compressNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E;
        }else
            if ((numOfRanges ==3) && (numOfLeaves == 3))
            {
                *isCompressedWithEmbLeavesPtr = GT_TRUE;
                *compressNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E;
            }else
            {
                *compressNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
            }
}

/*******************************************************************************
* lpmFalconGetGroupOfNodesItems
*
* DESCRIPTION:
*       Gets number of components assembled group of nodes.
*
* INPUTS:
*       rangeListPtr      - Pointer to the bucket ranges .
*       bucketType        - bucket type
*       startAddr         - new range start address
*       endAddr           - new range end address
*
* OUTPUTS:
*       isStartAddrMatchRegionPtr - if start of new range already points on lpm node
*       isEndAddrMatchRegionPtr   - if end of new range already points on lpm node
*       regularNodesNumberPtr     - pointer to regular nodes number
*       compressedNodesNumberPtr  - pointer to compressed nodes number
*       leafNodesNumberPtr        - pointer to leaf nodes number
*
* RETURNS:
*       GT_OK               - on success,
*       GT_FAIL             - Otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS lpmFalconGetGroupOfNodesItems
(
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC        *rangeListPtr,
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT           bucketType,
    IN    GT_U32                                        startAddr,
    IN    GT_U32                                        endAddr,
    OUT   GT_BOOL                                       *isStartAddrMatchRegionPtr,
    OUT   GT_BOOL                                       *isEndAddrMatchRegionPtr,
    OUT   GT_U32                                        *regularNodesNumberPtr,
    OUT   GT_U32                                        *compressedNodesNumberPtr,
    OUT   GT_U32                                        *leafNodesNumberPtr
)
{

    switch (bucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        lpmFalconGetGroupOfNodesItemsForCompressed(rangeListPtr, regularNodesNumberPtr,
                                                   compressedNodesNumberPtr,leafNodesNumberPtr);
        break;
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        lpmFalconGetGroupOfNodesItemsForRegular(rangeListPtr,startAddr,endAddr,isStartAddrMatchRegionPtr,
                                                isEndAddrMatchRegionPtr,regularNodesNumberPtr,
                                                compressedNodesNumberPtr,leafNodesNumberPtr);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/*******************************************************************************
* lpmFalconGetGroupOfNodesSizes
*
* DESCRIPTION:
*       Calc the ammount of needed Pp memory for given group of nodes.
*
* INPUTS:
*       bucketPtr         - Pointer to the given bucket.
*       bucketType        - given bucket type
*       curAddrByte       - The current address byte
*       prefix            - The address prefix.
*       numberOfNewRanges - number of new ranges after insertion
*       newTurnedRegular  - GT_TRUE: bucket going to turn into regular under current insertion
*                           GT_FALSE: bucket was regular before current insertion
*       newLevelPlaned    - GT_TRUE: new level will be opened under current insertion
*                           GT_FALSE: new level will not be opened under current insertion
*
* OUTPUTS:
*       groupOfNodesIndex - group of node index size calculated for (if 0xf - fro all 6 indexes)
*       memPoolList       - List of memory pools from which the blocks should be
*                           allocated from.
*       memListLenPtr     - Number of memory blocks in neededMemList.
*       groupOfNodesSizes - group of nodes sizes array
*
* RETURNS:
*       GT_OK               - on success,
*       GT_FAIL             - Otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS lpmFalconGetGroupOfNodesSizes
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT           bucketType,
    IN    GT_U8                                         curAddrByte,
    IN    GT_U32                                        prefix,
    IN    GT_U32                                        numberOfNewRanges,
    IN    GT_BOOL                                       newTurnedRegular,
    IN    GT_BOOL                                       newLevelPlaned,
    IN    GT_U32                                        *numberOfBvLineRanges,
    OUT   GT_BOOL                                       *groupOfNodesIndexes,
    OUT   GT_U32                                        *groupOfNodesSizes
)
{
    GT_STATUS                                        rc = GT_OK;
    GT_U32                                           regularNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           compressedNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           leafNodesNumber[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32                                           leafNodesSize[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U8                                            startAddr = 0;
    GT_U8                                            endAddr = 0;
    GT_U8                                            startSubNodeIndex = 0;
    GT_U8                                            endSubNodeIndex = 0;
    GT_U8                                            gonIndex = 0;
    GT_U32                                           i = 0;
    GT_U32                                           maxNodesNumber = 0;
    GT_BOOL isCompressedWithEmbLeaves = GT_FALSE;
    GT_BOOL isStartAddrMatchRegion = GT_FALSE;
    GT_BOOL isEndAddrMatchRegion = GT_FALSE;
    GT_BOOL isOverwrite = GT_FALSE;
    GT_U8   counter = 0xff;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT compressedNodeType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
    switch (bucketType)
    {
    case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:

        maxNodesNumber = 1;
        rc = lpmFalconGetGroupOfNodesItems(bucketPtr->rangeList,CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E,
                                           startAddr,endAddr,&isStartAddrMatchRegion,&isEndAddrMatchRegion,
                                           &regularNodesNumber[0],&compressedNodesNumber[0],
                                           &leafNodesNumber[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (newLevelPlaned == GT_TRUE)
        {
            /* in this bucket new range is opened. It points on new bucket*/
            compressedNodesNumber[0]++;
            leafNodesNumber[0] = leafNodesNumber[0] + numberOfNewRanges - bucketPtr->numOfRanges - 1;
        }
        else
        {
            leafNodesNumber[0] = leafNodesNumber[0] + numberOfNewRanges - bucketPtr->numOfRanges;
        }
        /* verify do we need special place for leaves or it is embedded leaves*/
        /* in order to do that check if current node after adding new regions is node with embedded leaves*/
        lpmFalconCheckIfCompressedWithEmbeddedLeaves(numberOfNewRanges,leafNodesNumber[0],
                                                     &isCompressedWithEmbLeaves, &compressedNodeType);
        if (isCompressedWithEmbLeaves == GT_TRUE)
        {
            /* all leaves are embedded.*/
            leafNodesNumber[0] = 0;
        }
        break;
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        maxNodesNumber = PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS;
        /* Some ranges are added: need to know in which subnodes */
        calcStartEndAddr(curAddrByte,prefix,&startAddr,&endAddr);
        startSubNodeIndex = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        endSubNodeIndex = endAddr/NUMBER_OF_RANGES_IN_SUBNODE;

        rc = lpmFalconGetGroupOfNodesItems(bucketPtr->rangeList,bucketType,startAddr,endAddr,
                                           &isStartAddrMatchRegion,&isEndAddrMatchRegion, &regularNodesNumber[0],
                                           &compressedNodesNumber[0],&leafNodesNumber[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
        
        /* the number of hw bit vector ranges can be differ from shadow  */
        /* it is possible that "changed" bit vector has no additionals ranges */
        if (numberOfBvLineRanges != NULL) 
        {
            if (numberOfNewRanges > bucketPtr->numOfRanges)
            {
                counter = 0;
                for (gonIndex = startSubNodeIndex; gonIndex <= endSubNodeIndex; gonIndex++) 
                {
                    if (numberOfBvLineRanges[gonIndex] > 0) 
                    {
                        counter++;
                    }
                }
            }
        }
        if ((numberOfNewRanges == bucketPtr->numOfRanges) || (counter == 0) ) 
        {
            isOverwrite = GT_TRUE;
        }
        /* check overwrite */
        if (isOverwrite == GT_TRUE)
        {
            /* overwrite take place */
            /* overwrite for single range and for leaf spectrum lead to same result */
            /* the new level doesn't open in case of leaf stream */
             if (newLevelPlaned == GT_TRUE)
             {
                 /* leaf is going to be changed to compressed */
                 compressedNodesNumber[startSubNodeIndex]++;
                 leafNodesNumber[startSubNodeIndex]--;
                     
             }
        }
        else /* no overwrite*/
            if (startAddr == endAddr)
            {
                /* startSubNodeIndex = endSubNodeIndex*/
                /* it is not leaf spectrum*/

                if (newLevelPlaned == GT_TRUE)
                {
                    compressedNodesNumber[startSubNodeIndex]++;
                    leafNodesNumber[startSubNodeIndex] =
                        leafNodesNumber[startSubNodeIndex] + numberOfBvLineRanges[startSubNodeIndex] - 1;
                }
                else
                {
                    leafNodesNumber[startSubNodeIndex] =
                        leafNodesNumber[startSubNodeIndex] + numberOfBvLineRanges[startSubNodeIndex];
                }
            }
            else
            {
                /* leaf spectrum */
                /* max number of new ranges is 2 */
                for (gonIndex = startSubNodeIndex; gonIndex <= endSubNodeIndex; gonIndex++) 
                {
                   leafNodesNumber[gonIndex]+= numberOfBvLineRanges[gonIndex];  
                }
            }
       break;
    default:
        break;
    }
    if (bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
         groupOfNodesIndexes[0] = GT_TRUE;
    }
    for (i = 0; i < maxNodesNumber; i++)
    {
        if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
        {
            if (newTurnedRegular == GT_FALSE)
            {
                if ( (i != startSubNodeIndex) && (i != endSubNodeIndex))
                {
                    continue;
                }
                groupOfNodesIndexes[i] = GT_TRUE;
            }
            else
            {
                groupOfNodesIndexes[i] = GT_TRUE;
            }
        }

        /* calculate place for leaves */
        leafNodesSize[i] = leafNodesNumber[i]/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
        if ( (leafNodesNumber[i] % MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS) != 0 )
            leafNodesSize[i]++;
        groupOfNodesSizes[i] =
            regularNodesNumber[i]*PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS +
             compressedNodesNumber[i]*PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS + leafNodesSize[i];
    }
    return GT_OK;
}



/*******************************************************************************
* lpmFalconCalcNeededMemory
*
* DESCRIPTION:
*       Calc the ammount of needed Pp memory for unicast address insertion.
*
* INPUTS:
*       bucketPtr         - Pointer to the root bucket.
*       addrCurBytePtr    - The Byte Array represnting the Address associated
*                           with the entry.
*       prefix            - The address prefix.
*       rootBucketFlag    - Indicates the way to deal with a root bucket (if it is).
*       lpmEngineMemPtrPtr- points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                           which holds all the memory information needed for where and
*                           how to allocate search memory for each of the lpm levels
*       neededMemList     - List holding the memory needed for inserting the given
*                           address.
*       memPoolList       - List of memory pools from which the blocks should be
*                           allocated from.
*       memListLenPtr     - Nuumber of memory blocks in neededMemList.
*
* OUTPUTS:
*       neededMemList     - List holding the memory needed for inserting the given
*                           address (in LPM lines)
*       memPoolList       - List of memory pools from which the blocks should be
*                           allocated from.
*       memListLenPtr     - Number of memory blocks in neededMemList.
*       memListOctetIndexList - List holding the Octet Index of each memory needed for
*                           inserting the given address
*
* RETURNS:
*       GT_OK               - on success,
*       GT_FAIL             - Otherwise.
*
* COMMENTS:
*       - This function calculates the needed memory only in the tree layers
*         that are are needed to be written to HW (using rootBucketFlag)
*
*******************************************************************************/
static GT_STATUS lpmFalconCalcNeededMemory
(
    IN    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC       *bucketPtr,
    IN    GT_U8                                         *addrCurBytePtr,
    IN    GT_U32                                        prefix,
    IN    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT    rootBucketFlag,
    IN    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC            **lpmEngineMemPtrPtr,
    INOUT PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC     *neededMemList[],
    INOUT GT_UINTPTR                                    *memPoolList[],
    INOUT GT_U32                                        *memListLenPtr,
    INOUT GT_U32                                        *memListOctetIndexList[]
)
{
    GT_U8  startAddr;           /* The startAddr and endAddr of the     */
    GT_U8  endAddr;             /* given prefix in the current level.   */

    GT_U32 rangePrefixLength;   /* The prefix length represented by the */
                                /* current range's mask field.          */


    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;         /* The first range that overlaps with   */
                                /* the given address * prefix.               */
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT    splitMethod;  /* The method according to which the    */
                                                              /* range should be split.               */
    GT_U32          numOfBucketRanges;  /* Number of ranges in the      */
    GT_U32          numberOfBvLineRanges[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0}; /* number of ranges in bv line */
                                /* current's level's bucket.            */
    GT_U32          currNumOfBucketRanges;  /* Number of ranges in the      */
                                /* current's level's bucket.            */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *currBucketPtr = bucketPtr;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *copyOfCurrentBucketPtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *prevLevelBucketPtr = NULL;
    GT_U8 *currAddrByte = addrCurBytePtr;
    GT_U32 currPrefix = prefix;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC **currLpmEngineMemPtr = lpmEngineMemPtrPtr;
    GT_U32 octetIndex;
    GT_BOOL planedNewBucket[16];
    GT_U32 grOfNodesSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_BOOL grOfNodesIndexes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32 prevLevelGroupOfNodesSize = 0;
    GT_U32 prevLevelGroupOfNodesIndex = 0;
    GT_U32 i = 0;
    GT_U32 tempPrefixLength = 0;
    GT_U32 prevStartAddr = 0;
    GT_STATUS rc = GT_OK;
    GT_BOOL allocIsNeeded = GT_FALSE;
    GT_BOOL allocIsNeededPerGroup[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS]= {0};
    GT_UINTPTR  prevOctetstructsMemPool = 0;
    GT_U32 level = 0;

    /* All allocations are calculated for group of nodes */
    /* First group is starting from octet 1*/
    octetIndex=1;
    currLpmEngineMemPtr++;
    
    cpssOsMemSet(planedNewBucket, 0, sizeof(planedNewBucket));
    while(currPrefix > 0)
    {
        copyOfCurrentBucketPtr = currBucketPtr;
        splitMethod = PRV_CPSS_DXCH_LPM_RAM_SPLIT_LAST_E;
        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);
        if (currBucketPtr == NULL)
        {
            /* This is a new tree level, a new bucket should be created, */
            /* This new bucket will point to new group of nodes*/
            tempPrefixLength = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
            if (tempPrefixLength > 0)
            {
                /* The new groups of nodes can be 1 compressed + 1 leaf  */
                /* or 1 compressed +2 leaves*/
                /* new compressed nodes types with 1,2,3 leaves takes 1 line*/
                /* it consists leaves inside of compressed node itself, so new group of nodes */
                /* would be only 1 compressed without leaves */
                (*neededMemList)->neededMemoryBlocksSizes[0] = 1;
            }
            else
            {
                /* This is last level . The new group of nodes is 2 or 3 leaves */
                /* In this case new compressed type with 2 or 3 leaves is used */
                /* This compressed incorporates leaves inside and point on them*/
                /* So there is no new group of nodes */
                (*neededMemList)->neededMemoryBlocksSizes[0] = 0;

            }
            if((*neededMemList)->neededMemoryBlocksSizes[0]>0)
            {
                (*neededMemList)->octetId = octetIndex;
                **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                (*memListLenPtr)++;
                **memListOctetIndexList=octetIndex;
                (*memPoolList)--;
                (*neededMemList)--;
                (*memListOctetIndexList)--;
            }

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
                /* in multicast may be will use leaf_triger type*/
                if ((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    (pRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
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
                    /* in multicast may be will use leaf_triger type */
                    if ((pRange->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        (pRange->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)(pRange->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* the range points to an end of tree (next-hop or
                           src bucket */

                        lpmFalconGetSplitMethod(copyOfCurrentBucketPtr->bucketType,
                                                &numOfBucketRanges,
                                                pRange,startAddr,
                                                endAddr,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,
                                                &splitMethod, &numberOfBvLineRanges[0]);

                        if (currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS)
                        {
                            /* the prefix needs to be inserted deeper thus in
                            this case we Need to create new tree levels, all of
                            them of type "1-line bucket" including 2 or 3 lines.*/

                            /* (will reach here only once per do-while loop) */
                            currBucketPtr = NULL;
                            planedNewBucket[octetIndex] = GT_TRUE;
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
                       /* prevLevelBucketPtr = pRange->lowerLpmPtr.nextBucket;*/
                        currBucketPtr = pRange->lowerLpmPtr.nextBucket;
                    }
                }

                pRange = pRange->next;
            }while ((pRange != NULL) && (endAddr >= pRange->startAddr));

            /* check if a resize is needed, if not return */
            if ( (numOfBucketRanges != currNumOfBucketRanges) ||
                 (splitMethod == PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E) )
            {
                /* The bucket size is changed, group of nodes this bucket pointed on */
                /* also changed */
                
                if ((numOfBucketRanges <= MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) && 
                    (copyOfCurrentBucketPtr->bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E))
                {
                    /* check if we have new mc src bucket  */
                    if ( (copyOfCurrentBucketPtr->hwBucketOffsetHandle == 0) &&
                         (currNumOfBucketRanges == 1) && (level == 0) )
                    {
                        /* this is new mc source root which has no hw allocation yet */
                        /* This node at this moment can be compressed : 1 line */
                        (*neededMemList)->neededMemoryBlocksSizes[0] = 1;
                        (*memListLenPtr)++;
                        (*neededMemList)->octetId = octetIndex-1;
                        (*neededMemList)--;
                        currLpmEngineMemPtr--;
                        **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                        /* return pool ptr on correct octet */
                        currLpmEngineMemPtr++;
                        (*memPoolList)--;
                        **memListOctetIndexList=octetIndex-1;
                        (*memListOctetIndexList)--;
                    }
                    /* there is one case when bucket is regular but number of ranges like in compressed:*/
                    /* root bucket is always regular */
                    /* the bucket is changed. It means that whole group of nodes this bucket belongs to */
                    /* is also changed. So it is needed recalculate memory for this group of nodes */
                    /* calculate group of nodes size in lpm lines this bucket pointed on */
                    cpssOsMemSet(grOfNodesIndexes,0,sizeof(grOfNodesIndexes));
                    cpssOsMemSet(grOfNodesSizes,0,sizeof(grOfNodesSizes));

                    rc = lpmFalconGetGroupOfNodesSizes(copyOfCurrentBucketPtr,
                                                       CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E,
                                                       *currAddrByte,currPrefix,numOfBucketRanges,
                                                       GT_FALSE,planedNewBucket[octetIndex], &numberOfBvLineRanges[0],
                                                       &grOfNodesIndexes[0], &grOfNodesSizes[0]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    cpssOsMemSet(numberOfBvLineRanges,0,sizeof(numberOfBvLineRanges));
                    /* check if previous group size is differ  */
                    if (copyOfCurrentBucketPtr->hwGroupOffsetHandle[0] != 0)
                    {
                        if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(copyOfCurrentBucketPtr->hwGroupOffsetHandle[0]) != grOfNodesSizes[0])
                        {
                            allocIsNeeded = GT_TRUE;
                            (*neededMemList)->neededMemoryBlocksSizes[0] = grOfNodesSizes[0];
                            (*neededMemList)->octetId = octetIndex;
                            (*neededMemList)->regularNode = GT_FALSE;
                        }
                    }
                    else
                    {
                        /* it can be when on last level is compressed node with 3 ranges and 3 leaves or
                           compressed with 2 ranges and 2 leaves*/
                        if (grOfNodesSizes[0]>0)
                        {
                            allocIsNeeded = GT_TRUE;
                            (*neededMemList)->neededMemoryBlocksSizes[0] = grOfNodesSizes[0];
                            (*neededMemList)->octetId = octetIndex;
                            (*neededMemList)->regularNode = GT_FALSE;
                        }
                    }
                }
                else
                {
                    /* Regular bucket: 6 groups of nodes*/
                        /*  Check if node was regular before or it is result of new
                           range adding under current operation */
                    if ((currNumOfBucketRanges > MAX_NUMBER_OF_COMPRESSED_RANGES_CNS) ||
                        ((level == 0) && (copyOfCurrentBucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)) )
                    {
                        /* node was regular before current adding or destination root  */
                        /* need find index of changed group of nodes and calculate new size */
                        cpssOsMemSet(grOfNodesIndexes,0,sizeof(grOfNodesIndexes));
                        cpssOsMemSet(grOfNodesSizes,0,sizeof(grOfNodesSizes));

                        rc = lpmFalconGetGroupOfNodesSizes(copyOfCurrentBucketPtr,
                                                           CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,
                                                           *currAddrByte,currPrefix,numOfBucketRanges,
                                                           GT_FALSE,planedNewBucket[octetIndex],&numberOfBvLineRanges[0],
                                                           &grOfNodesIndexes[0],&grOfNodesSizes[0]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        cpssOsMemSet(numberOfBvLineRanges,0,sizeof(numberOfBvLineRanges));
                        /* check if we really need new allocation */
                        for (i =0; i <PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            if (grOfNodesIndexes[i] == GT_TRUE)
                            {
                                if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(copyOfCurrentBucketPtr->hwGroupOffsetHandle[i]) != grOfNodesSizes[i])
                                {
                                    allocIsNeeded = GT_TRUE;
                                    allocIsNeededPerGroup[i] = GT_TRUE;
                                }
                            }
                        }
                    }
                    else
                    {
                        /* As a result of range adding bucket turned into regular */
                        /* there is possibility that node is mc source root*/
                        /* if it is so, new allocation for regular node is done*/

                        if ((level == 0) && (copyOfCurrentBucketPtr->bucketType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)) 
                        {
                            /* it is mc source root tree going to be regular*/
                            (*neededMemList)->neededMemoryBlocksSizes[0] = 6;
                            (*memListLenPtr)++;
                            (*neededMemList)->octetId = octetIndex-1;
                            (*neededMemList)->regularNode = GT_TRUE;
                            (*neededMemList)--;
                            currLpmEngineMemPtr--;
                            **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                            /* return pool ptr on correct octet */
                            currLpmEngineMemPtr++;
                            (*memPoolList)--;
                            **memListOctetIndexList=octetIndex-1;
                            (*memListOctetIndexList)--;
                        }
                        else
                        {
                            /* As a result of range adding bucket turned into regular. So its group of nodes */
                            /* size was changed. It is needed to go back and update group of nodes size in */
                            /* previous octet*/
                            prevLevelGroupOfNodesIndex = 0; /* for compressed */
                            if (prevLevelBucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                            {
                                prevLevelGroupOfNodesIndex = prevStartAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                                (*neededMemList)->regularNode = GT_TRUE;
                            }
                            else
                            {
                                (*neededMemList)->regularNode = GT_FALSE;
                            }
                            prevLevelGroupOfNodesSize =
                                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(prevLevelBucketPtr->hwGroupOffsetHandle[prevLevelGroupOfNodesIndex]);
                            if (prevLevelGroupOfNodesSize > 264)
                            {
                                cpssOsPrintf(" The GON size too big!!!!!! ERROR!!!!\n");
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                            }
                            prevLevelGroupOfNodesSize = prevLevelGroupOfNodesSize -
                                PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS + PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
                            if (prevLevelGroupOfNodesSize > 264)
                            {
                                cpssOsPrintf(" The GON size too big!!!!!! ERROR!!!!\n");
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                            }

                            /* here we have clear allocation case for previous level */
                           /* (*neededMemList)++;*/
                            (*neededMemList)->neededMemoryBlocksSizes[prevLevelGroupOfNodesIndex] =
                                prevLevelGroupOfNodesSize;
                            (*memListLenPtr)++;
                            (*neededMemList)->octetId = octetIndex-1;
                            (*neededMemList)--;
                            **memPoolList = prevOctetstructsMemPool;
                            (*memPoolList)--;
                            **memListOctetIndexList=octetIndex-1;
                            (*memListOctetIndexList)--;

                        }
                        /* new 6 group of nodes would be created */
                        cpssOsMemSet(grOfNodesIndexes,0,sizeof(grOfNodesIndexes));
                        cpssOsMemSet(grOfNodesSizes,0,sizeof(grOfNodesSizes));

                        rc = lpmFalconGetGroupOfNodesSizes(copyOfCurrentBucketPtr,
                                                           CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,
                                                           *currAddrByte,currPrefix,numOfBucketRanges,
                                                           GT_TRUE,planedNewBucket[octetIndex],&numberOfBvLineRanges[0],
                                                           &grOfNodesIndexes[0],&grOfNodesSizes[0]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        cpssOsMemSet(numberOfBvLineRanges,0,sizeof(numberOfBvLineRanges));
                        allocIsNeeded = GT_TRUE;
                        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            allocIsNeededPerGroup[i] = GT_TRUE;
                        }
                    }
                    if (allocIsNeeded == GT_TRUE)
                    {
                        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            if (grOfNodesIndexes[i] == GT_TRUE)
                            {
                                if (allocIsNeededPerGroup[i] == GT_TRUE)
                                {
                                    (*neededMemList)->neededMemoryBlocksSizes[i] = grOfNodesSizes[i];
                                    (*neededMemList)->regularNode = GT_TRUE;
                                    (*neededMemList)->octetId = octetIndex;
                                }
                            }
                        }
                    }
                }
                if (allocIsNeeded == GT_TRUE)
                {
                    **memPoolList = currLpmEngineMemPtr[0]->structsMemPool;
                    (*memListLenPtr)++;
                    (*memPoolList)--;
                    (*neededMemList)--;
                    **memListOctetIndexList=octetIndex;
                    (*memListOctetIndexList)--;
                    allocIsNeeded = GT_FALSE;
                    cpssOsMemSet(allocIsNeededPerGroup,GT_FALSE,sizeof(allocIsNeededPerGroup));
                }
            }
        }
        prevStartAddr = startAddr;
        prevLevelBucketPtr = copyOfCurrentBucketPtr;
        currAddrByte++;
        currPrefix = NEXT_BUCKET_PREFIX_MAC(currPrefix,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
        rootBucketFlag = PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E;
        prevOctetstructsMemPool = currLpmEngineMemPtr[0]->structsMemPool;
        currLpmEngineMemPtr++;
        octetIndex++;
        level++;
    }

    return rc;
}



/**
* @internal mergeCheckSip6 function
* @endinternal
*
* @brief   Returns 1 if the two input ranges can be merged to one range.
*
* @param[in] leftRangePtr             - The first range to check for merging.
* @param[in] rightRangePtr            - The second range to check for merging.
* @param[in] levelPrefix              - The current lpm level prefix.
*                                       1 if the two input ranges can be merged to one range.
*/
GT_U8 mergeCheckSip6
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
         (leftRangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) &&
         ((GT_U32)(leftRangePtr->pointerType) != (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)) ||
        ((rightRangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
         (rightRangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) &&
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







/*******************************************************************************
* mergeRangeSip6
*
* DESCRIPTION:
*       This function merges a range. According to one of the following possible
*       merges:
*       PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E, PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E,
*       PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E and PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E
*
* INPUTS:
*       rangePtr          - Pointer to the range to be merged.
*       startAddr         - The low address of the range to be merged.
*       prefixLength      - The length of the dominant prefix of the range.
*       prevRangePtr      - A pointer to the lower address neighbor of the range to be
*                           merged. NULL if rangePtr is the 1st in the list.
*       levelPrefix       - The current lpm level prefix.
*
* OUTPUTS:
*       rangePtr          - A pointer to the next node in the list.
*       numOfNewRangesPtr - Number of new created ranges as a result of the split.
*
* RETURNS:
*       GT_OK on success, GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS mergeRangeSip6
(
    IN    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       bucketType,
    INOUT PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **rangePtr,
    IN    GT_U32                                    startAddr,
    IN    GT_U32                                    endAddr,
    IN    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *prevRangePtr,
    IN    GT_U8                                     levelPrefix,
    OUT   GT_32                                     *numOfNewRangesPtr,
    OUT   GT_32                                     *numOfNewHwRangesPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *curRangePtr = (*rangePtr);/* Temp vars*/
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempNextRangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempPrevRangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_MERGE_METHOD_ENT mergeMethod;     /* The method by which too perform the  */
                                                            /* ranges merge.                        */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tmpPtr; /* Temporary range pointer to be used   */
                                /* in range operation.                  */
    GT_U32                                           startSubNodeIndex = 0;
    GT_U32                                           endSubNodeIndex = 0;

    /* warnings fix */
    *numOfNewHwRangesPtr = 0;
    /* calculate number of real ranges in bit vector */
    if (bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        startSubNodeIndex = startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        endSubNodeIndex =   endAddr/NUMBER_OF_RANGES_IN_SUBNODE;
        if (startSubNodeIndex == endSubNodeIndex)
        {
            if (endAddr == endSubNodeAddress[endSubNodeIndex])
            {
                tempNextRangePtr = NULL;
            }
            else
            {
                tempNextRangePtr = curRangePtr->next;
            }
            if (startAddr == startSubNodeAddress[startSubNodeIndex])
            {
                tempPrevRangePtr = NULL;
            }
            else
            {
                tempPrevRangePtr = prevRangePtr;
            }
            mergeMethod = ((mergeCheckSip6(curRangePtr,tempNextRangePtr,
                                       levelPrefix) ? 1 : 0) +
                           (mergeCheckSip6(tempPrevRangePtr,curRangePtr,levelPrefix) ? 2 : 0));
            switch (mergeMethod)
            {
            case PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E: /* The old range fell       */
                *numOfNewHwRangesPtr = 0;
                break;
            case PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E:  /* Merge the upper part of the range */
                *numOfNewHwRangesPtr = -1;
                break;
            case PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E: /* Merge the lower part of the range */
                *numOfNewHwRangesPtr = -1;
                break;
            case PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E: /* Merge the ranges from both sides of the range.*/
                *numOfNewHwRangesPtr = -2;
                break;
            default:
                break;
            }
        }
    }
    mergeMethod = ((mergeCheckSip6(curRangePtr,curRangePtr->next,
                               levelPrefix) ? 1 : 0) +
                   (mergeCheckSip6(prevRangePtr,curRangePtr,levelPrefix) ? 2 : 0));

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

    if (bucketType != CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
    {
        *numOfNewHwRangesPtr = *numOfNewRangesPtr;
    }
    else
        if (startSubNodeIndex != endSubNodeIndex)
        {
            *numOfNewHwRangesPtr = *numOfNewRangesPtr;
        }
    return GT_OK;
}

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

   /* Was the bucket resized as a result of deletion.                  */
    LPM_ROOT_BUCKET_UPDATE_ENT               rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_NONE_E;
    GT_BOOL resizeGroup;       /* Was the group resized as a  */
                                /* result of deletion.                  */

    GT_BOOL doHwUpdate;         /* is it needed to update the HW */
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *newNextHopPtr;/* Next hop representing
                                                           the prefix that
                                                           replaces the deleted
                                                           one. */
    GT_32 numOfNewRanges;       /* Number of new ranges as a result of  */
                                /* range merging.                       */
    GT_32 numberOfNewHwRanges;   /* Number of new hw ranges as a result of  */
                                /* range merging.(relates to bitvector)     */
    LPM_BUCKET_UPDATE_MODE_ENT bucketUpdateMode;/* indicates whether to write
                                                   the bucket to the device's
                                                   RAM.*/
    GT_BOOL indicateSiblingUpdate;
    GT_PTR  nextPtr;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT  insertMode;

    GT_U32 level = 0;
    GT_U32 j=0;
    GT_U32 oldNumOfRanges = 0;
    GT_BOOL updateOld,updateUpperLevel = GT_FALSE;
    GT_STATUS retVal = GT_OK;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pPrevRange;/* The first range that overlaps with   */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC **pRange;    /* address & prefix, and a pointer to  */
                                                        /* the range that comes before it.      */
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_NODE_STC  updateParams;

    static PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT       *lpmPtrArray[MAX_LPM_LEVELS_CNS];

    static PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *pRangeArray[MAX_LPM_LEVELS_CNS];
    static PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *pPrvRangeArray[MAX_LPM_LEVELS_CNS];
    static CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT            *bucketPtrTypeArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT              **currLpmPtr;
    GT_U8                                                 *currAddrByte;
    GT_U32                                                currPrefix;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                   **currBucketPtrType;
    GT_BOOL                                               isDestTreeRootBucket = GT_FALSE;
    GT_BOOL                                               isSrcTreeRootBucket = GT_FALSE;
    GT_BOOL                                               regularCompressedConversion = GT_FALSE;
    GT_BOOL                                               rangeIsRemoved = GT_FALSE;
    GT_BOOL                                               overwriteIsDone = GT_FALSE;
    GT_BOOL                                               lastLevelBacketEmbLeaves = GT_FALSE;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                   newNextHopPtrType;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                   updatedBucketType;
    GT_U32 subnodesSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_BOOL subnodesIndexes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    GT_U32 i;
    currAddrByte = addrCurBytePtr;
    currPrefix = prefix;

    cpssOsMemSet(lpmPtrArray,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(pRangeArray,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(pPrvRangeArray,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(bucketPtrTypeArray,0,sizeof(GT_U8*)*MAX_LPM_LEVELS_CNS);
    cpssOsMemSet(&updateParams,0,sizeof(updateParams));

    bucketPtrTypeArray[0] = bucketTypePtr;
    lpmPtrArray[0] = lpmPtr;
    currLpmPtr = lpmPtrArray;
    pPrevRange = pPrvRangeArray;
    pRange = pRangeArray;
    currBucketPtrType = bucketPtrTypeArray;
    /* lpmEngineMemPtrPtr points on memory pool for octet 0*/
    /* gon for bucket related to octet 0 can't be in banks associated with octet 0 */
    lpmEngineMemPtrPtr++;

    /* first run and record all the neccesary info down the bucket tree */
    for(;;)
    {
        if (*currLpmPtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG); /* the prefix is not in the tree */

        calcStartEndAddr(*currAddrByte,currPrefix,&startAddr,&endAddr);

        *pRange = find1stOverlap((*currLpmPtr)->nextBucket,startAddr,pPrevRange);
        /* The deleted prefix ends in a deeper level,   */
        /* call the delete function recursively.        */
        /* in multicast may be use leaf_tigger type */
        if ((currPrefix > PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS) &&
            ((*pRange)->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            ((*pRange)->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)  &&
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
        rootBucketUpdateState    = LPM_ROOT_BUCKET_UPDATE_NONE_E;
        resizeGroup              = GT_FALSE;
        doHwUpdate               = GT_FALSE;
        indicateSiblingUpdate    = GT_FALSE;
        rangeIsRemoved           = GT_FALSE;
        overwriteIsDone          = GT_FALSE;
        lastLevelBacketEmbLeaves = GT_FALSE;
        updatedBucketType = (*currLpmPtr)->nextBucket->bucketType;

        if ((currLpmPtr == lpmPtrArray)     &&
            (rootBucketFlag !=PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E) )
        {
            if (isMcSrcTree == GT_FALSE) 
            {
                isDestTreeRootBucket = GT_TRUE;
                isSrcTreeRootBucket = GT_FALSE;
            }
            else
            {
                isSrcTreeRootBucket = GT_TRUE;
                isDestTreeRootBucket = GT_FALSE;
            }
        }

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
                    if (regularCompressedConversion == GT_TRUE)
                    {
                        /* The bucket on next deeper level turn to compressed */
                        /* The size of current GON is changed */
                        cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                        cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
                        retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                               updatedBucketType,
                                                                *currAddrByte,currPrefix,
                                                               (*currLpmPtr)->nextBucket->numOfRanges,
                                                               GT_FALSE,GT_FALSE,NULL,
                                                               &subnodesIndexes[0],&subnodesSizes[0]);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }

                        resizeGroup = GT_TRUE;
                        regularCompressedConversion = GT_FALSE;
                    }
                    /* the upper level has been marked for update , remove
                       the signal */
                    if (resizeGroup == GT_FALSE)
                        updateUpperLevel = GT_FALSE;
                }

                /* Check the type of the next bucket after deletion */
                if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E))
                {
                    /* if it's type "route entry pointer" it means there is no
                       need for that pointed bucket (and it actually been
                       deleted in the lower level) so merge the range */
                    mergeRangeSip6((*currLpmPtr)->nextBucket->bucketType,pRange,startAddr,endAddr,
                                   *pPrevRange,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,&numOfNewRanges,
                                   &numberOfNewHwRanges);
                    if ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                    {
                        if ((numberOfNewHwRanges == 0) && (numOfNewRanges == -1))
                        {
                            if ((GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges) >= 11)
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

                                PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                                 (*pRange)->pointerType);
                                (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;

                                doHwUpdate = GT_TRUE;

                            }
                        }
                    }
                    if (numOfNewRanges == 0)
                    {
                        /* An overwrite merge was done (no change in the amount
                           of ranges) extract the appropriate prefix from the
                           trie, and insert it in the same place.*/
                        /* The override can impact group of nodes size. The GON size must be
                           recalculated to understand if resize is needed */
                        cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                        cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
                        retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                               updatedBucketType,
                                                                *currAddrByte,currPrefix,
                                                               (*currLpmPtr)->nextBucket->numOfRanges,
                                                               GT_FALSE,GT_FALSE,NULL,
                                                               &subnodesIndexes[0],&subnodesSizes[0]);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }
                        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                        {
                            if (subnodesIndexes[i] == GT_TRUE)
                            {
                                if (subnodesSizes[i] == 0)
                                {
                                    /* it is possible only if it become  Last level  with bucket became 3-3,2-2 */
                                    /* as a result of delettion deeper level */
                                    lastLevelBacketEmbLeaves = GT_TRUE;

                                    if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[0] != 0)
                                    {
                                        resizeGroup = GT_TRUE;
                                        /* the memory must be freed */
                                    }
                                    else
                                    {
                                        resizeGroup = GT_FALSE;
                                    }
                                    break;
                                }
                                if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]) != subnodesSizes[i])
                                {
                                    resizeGroup = GT_TRUE;
                                    break;
                                }
                            }

                        }
                        rangePrefixLength =
                            mask2PrefixLength((*pRange)->mask,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                        nextPtr = getFromTrie((*currLpmPtr)->nextBucket,
                                              (*pRange)->startAddr,
                                              rangePrefixLength,
                                              PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                        PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                         (*pRange)->pointerType);
                        (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;

                        doHwUpdate = GT_TRUE;
                        overwriteIsDone = GT_TRUE;
                    }
                    else
                    {
                        /* update the number of ranges */
                        oldNumOfRanges = (*currLpmPtr)->nextBucket->numOfRanges;
                        (*currLpmPtr)->nextBucket->numOfRanges =
                            (GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges);

                        /* check if range deletion cause to group resize */
                        /* indicate a resize has happen */
                        if ( ((*currLpmPtr)->nextBucket->numOfRanges == 1) &&
                             ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) )
                        {
                            resizeGroup = GT_FALSE;
                        }
                        else
                            if ((*currLpmPtr)->nextBucket->numOfRanges == 1)
                            {
                                /* this bucket is going to be deleted fully*/
                                resizeGroup = GT_TRUE;
                            }

                        if ( (oldNumOfRanges >= 11) && ((*currLpmPtr)->nextBucket->numOfRanges < 11))
                        {
                            if (isDestTreeRootBucket == GT_FALSE)
                            {
                                resizeGroup = GT_TRUE;
                                regularCompressedConversion = GT_TRUE;
                                updatedBucketType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
                            }
                        }
                        if ((*currLpmPtr)->nextBucket->numOfRanges > 1)
                        {
                            /* in this case 1 or 2 leaves removed.*/
                            /* calculate group of nodes size */
                            cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                            cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));
                            retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                                   updatedBucketType,
                                                                   *currAddrByte,currPrefix,
                                                                   (*currLpmPtr)->nextBucket->numOfRanges,
                                                                   GT_FALSE,GT_FALSE,NULL,
                                                                   &subnodesIndexes[0],&subnodesSizes[0]);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }
                            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                            {
                                if ( subnodesIndexes[i] == GT_TRUE)
                                {
                                    if (subnodesSizes[i] == 0)
                                    {
                                        /* it is possible only if it become  Last level  with bucket became 3-3,2-2 */
                                        /* as a result of delettion deeper level */
                                        lastLevelBacketEmbLeaves = GT_TRUE;

                                        if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[0] != 0)
                                        {
                                            resizeGroup = GT_TRUE;
                                            /* the memory must be freed */
                                        }
                                        else
                                        {
                                            resizeGroup = GT_FALSE;
                                        }
                                        break;
                                    }
                                    if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]) != subnodesSizes[i])
                                    {
                                        resizeGroup = GT_TRUE;
                                        break;
                                    }
                                }
                            }
                            if (resizeGroup == GT_FALSE)
                            {
                                /* There is no resize, but number of ranges was removed. The correspondent
                                   GON must be updated */
                                rangeIsRemoved = GT_TRUE;
                                doHwUpdate = GT_TRUE;
                            }
                        }
                        /* a change happend to the structure of the bucket
                        (removed ranges) clear the cash, and signal the upper
                        level */
                        (*currLpmPtr)->nextBucket->rangeCash = NULL;
                    }
                    updateUpperLevel = GT_TRUE;
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
                    /* in multicast may be use leaf_trigger type */
                    if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* if the range's type "route entry pointer" it means
                           there is no need for this range, merge it */

                        mergeRangeSip6((*currLpmPtr)->nextBucket->bucketType, pRange,startAddr,endAddr,
                                       *pPrevRange,PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS,&numOfNewRanges,
                                       &numberOfNewHwRanges);
                        if ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                        {
                            if ((numberOfNewHwRanges == 0) && (numOfNewRanges == -1))
                            {
                                if ((GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges) >= 11)
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

                                    PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                                     (*pRange)->pointerType);
                                    (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;

                                    doHwUpdate = GT_TRUE;

                                }
                            }
                        }
                        if (numOfNewRanges == 0)
                        {
                           /* An overwrite merge was done (no change in the
                              amount of ranges) extract the appropriate prefix
                              from the trie, and insert it in the same
                              place.*/
                            cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                            cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));

                            retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                                   updatedBucketType,
                                                                    *currAddrByte,currPrefix,
                                                                   (*currLpmPtr)->nextBucket->numOfRanges,
                                                                   GT_FALSE,GT_FALSE,NULL,
                                                                   &subnodesIndexes[0],&subnodesSizes[0]);
                            if (retVal != GT_OK)
                            {
                                return retVal;
                            }
                            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                            {
                                if (subnodesIndexes[i] == GT_TRUE)
                                {
                                    if (subnodesSizes[i] == 0)
                                    {
                                        /* it is possible only if it become  Last level  with bucket became 3-3,2-2 */
                                        /* as a result of delettion deeper level */
                                        lastLevelBacketEmbLeaves = GT_TRUE;

                                        if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[0] != 0)
                                        {
                                            resizeGroup = GT_TRUE;
                                            /* the memory must be freed */
                                        }
                                        else
                                        {
                                            resizeGroup = GT_FALSE;
                                        }
                                        break;
                                    }
                                    if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]) != subnodesSizes[i])
                                    {
                                        resizeGroup = GT_TRUE;
                                        break;
                                    }
                                }
                            }

                            rangePrefixLength =
                                mask2PrefixLength((*pRange)->mask,
                                                  PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
                            nextPtr =
                                getFromTrie((*currLpmPtr)->nextBucket,
                                            (*pRange)->startAddr,
                                            rangePrefixLength,
                                            PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);

                            PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)nextPtr)->routeEntryMethod,
                                                                             (*pRange)->pointerType);
                            (*pRange)->lowerLpmPtr.nextHopEntry = nextPtr;
                            if ( (resizeGroup == GT_FALSE) && (lastLevelBacketEmbLeaves == GT_FALSE) )
                            {
                                /* There is no resize, but range was overwrited. The correspondent
                                   GON must be updated */
                                overwriteIsDone = GT_TRUE;
                                doHwUpdate = GT_TRUE;
                            }
                        }
                        else
                        {

                            /* update the amount of ranges */
                            oldNumOfRanges = (*currLpmPtr)->nextBucket->numOfRanges;
                            (*currLpmPtr)->nextBucket->numOfRanges =
                                (GT_U16)((*currLpmPtr)->nextBucket->numOfRanges + numOfNewRanges);
                            /* indicate a resize has happen */

                            if ( ((*currLpmPtr)->nextBucket->numOfRanges == 1) &&
                                 ( ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E) ||
                                    ((*currLpmPtr)->nextBucket->bucketType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)) )
                            {
                                /* this is the deepest level: if node was 2 ranges 2 leaves or 3 ranges 3 leaves
                                   it doesn't points on GON: leaves are inside of node structure */
                              /*  lastLevelBacketEmbLeaves = GT_TRUE;*/
                                resizeGroup = GT_FALSE;
                            }
                            else
                                if ((*currLpmPtr)->nextBucket->numOfRanges == 1)
                                {
                                    /* this bucket is going to be deleted fully*/
                                    resizeGroup = GT_TRUE;
                                }


                            if ( (oldNumOfRanges >= 11) && ((*currLpmPtr)->nextBucket->numOfRanges < 11))
                            {
                                if (isDestTreeRootBucket == GT_FALSE)
                                {
                                    resizeGroup = GT_TRUE;
                                    regularCompressedConversion = GT_TRUE;
                                    updatedBucketType = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
                                }

                            }
                            if ((*currLpmPtr)->nextBucket->numOfRanges > 1)
                            {
                                /* in this case 1 or 2 leaves removed.*/
                                /* calculate group of nodes size */
                                cpssOsMemSet(subnodesIndexes,0,sizeof(subnodesIndexes));
                                cpssOsMemSet(subnodesSizes,0,sizeof(subnodesSizes));

                                retVal = lpmFalconGetGroupOfNodesSizes((*currLpmPtr)->nextBucket,
                                                                       updatedBucketType,
                                                                       *currAddrByte,currPrefix,
                                                                       (*currLpmPtr)->nextBucket->numOfRanges,
                                                                       GT_FALSE,GT_FALSE,NULL,
                                                                       &subnodesIndexes[0],&subnodesSizes[0]);
                                if (retVal != GT_OK)
                                {
                                    return retVal;
                                }
                                for (i =0; i <PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
                                {
                                    if (subnodesIndexes[i] == GT_TRUE)
                                    {
                                        if (subnodesSizes[i] == 0)
                                        {
                                            /* it is possible only if it become  Last level  with bucket became 3-3,2-2 */
                                            /* as a result of delettion deeper level */
                                            lastLevelBacketEmbLeaves = GT_TRUE;
                                            if ((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[0] != 0)
                                            {
                                                resizeGroup = GT_TRUE;
                                                /* the memory must be freed */
                                            }
                                            else
                                            {
                                                resizeGroup = GT_FALSE;
                                            }
                                            break;
                                        }
                                        if (PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[i]) != subnodesSizes[i])
                                        {
                                            resizeGroup = GT_TRUE;
                                            break;
                                        }
                                    }
                                }
                                if ( (resizeGroup == GT_FALSE) && (lastLevelBacketEmbLeaves == GT_FALSE) )
                                {
                                    /* There is no resize, but number of ranges was removed. The correspondent
                                       GON must be updated */
                                    rangeIsRemoved = GT_TRUE;
                                    doHwUpdate = GT_TRUE;
                                }
                            }

                            /* a change happend to the structure of the bucket
                               (removed ranges) clear the cash, and signal the
                               upper level */
                            (*currLpmPtr)->nextBucket->rangeCash = NULL;
                        }
                        updateUpperLevel = GT_TRUE;
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

                        PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(newNextHopPtr->routeEntryMethod,newNextHopPtrType);
                        retVal =
                            prvCpssDxChLpmSip6RamMngInsert((*pRange)->lowerLpmPtr.nextBucket,
                                                           currAddrByte + 1,/* actually not relevant*/
                                                           0,newNextHopPtr,
                                                           newNextHopPtrType,
                                                           PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E,
                                                           lpmEngineMemPtrPtr + 1,
                                                           &updateOld,
                                                           &((*pRange)->pointerType),
                                                           shadowPtr,insertMode,
                                                           isMcSrcTree,
                                                           protocol);
                        if (retVal != GT_OK)
                        {
                            return retVal;
                        }
                        /* the GON on which pointed (*currLpmPtr)->nextBucket must be updated*/
                        doHwUpdate = GT_TRUE;
                        overwriteIsDone = GT_TRUE;
                    }
                }
            }

            *pPrevRange = (*pRange);
            (*pRange) = (*pRange)->next;

        }while (((*pRange) != NULL) && (endAddr >= (*pRange)->startAddr));

        /* if the bucket need resizing always update the HW */
        if (resizeGroup == GT_TRUE)
            doHwUpdate = GT_TRUE;

        if (isDestTreeRootBucket == GT_TRUE) 
        {
            if ( (resizeGroup == GT_TRUE) || (rangeIsRemoved == GT_TRUE) )
            {
                /* some ranges were deleted from root, need update */
                rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_HW_E;
            }
        }
        if (isSrcTreeRootBucket == GT_TRUE) 
        {
            if (regularCompressedConversion == GT_TRUE) 
            {
                rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_HW_AND_MEM_SRC_RESIZE_E;
            }
            else
                if ( (resizeGroup == GT_TRUE) || (rangeIsRemoved == GT_TRUE) )
                {
                    /* some ranges were deleted from root, need update */
                    /* here some problem may be seen: i need know resize or not*/
                    rootBucketUpdateState = LPM_ROOT_BUCKET_UPDATE_HW_E;
                }
        }

       
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
        if (lastLevelBacketEmbLeaves == GT_TRUE)
        {
            bucketUpdateMode = LPM_BUCKET_UPDATE_LAST_LEVEL_SHADOW_ONLY_E;
        }
        if ( (isDestTreeRootBucket == GT_FALSE) && (isSrcTreeRootBucket == GT_FALSE) )
        {
            updateParams.bucketPtr = lpmPtrArray[level - 1]->nextBucket; 
            updateParams.pRange = pRangeArray[level-1];
        }
        else
        {
            updateParams.bucketPtr = (*currLpmPtr)->nextBucket;
            updateParams.pRange = NULL;
        }
        updateParams.isRootNode = isDestTreeRootBucket;
        updateParams.shadowPtr = shadowPtr;
       
        /* Write the prepared shadow bucket to the device's RAM. */
        retVal = updateMirrorGroupOfNodes((*currLpmPtr)->nextBucket,bucketUpdateMode,
                                          isDestTreeRootBucket,isSrcTreeRootBucket,rootBucketUpdateState,
                                          resizeGroup,rangeIsRemoved,overwriteIsDone,GT_FALSE,
                                          indicateSiblingUpdate,lpmEngineMemPtrPtr,
                                          *currBucketPtrType,shadowPtr,&updateParams,
                                          &subnodesIndexes[0],&subnodesSizes[0]);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        pPrvRangeArray[level]->updateRangeInHw = GT_FALSE;

        /* update partition block */
        /* if hwBucketOffsetHandle==0 it means that the
           bucket was deleted in the updateMirrorGroupOfNodes stage */
        if (((*currLpmPtr)->nextBucket)->hwBucketOffsetHandle!=0)
        {
            SET_DMM_BLOCK_PROTOCOL((*currLpmPtr)->nextBucket->hwBucketOffsetHandle, protocol);
            SET_DMM_BUCKET_SW_ADDRESS((*currLpmPtr)->nextBucket->hwBucketOffsetHandle, ((*currLpmPtr)->nextBucket));
        }
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            if (((*currLpmPtr)->nextBucket)->hwGroupOffsetHandle[j]!=0)
            {
                SET_DMM_BLOCK_PROTOCOL((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[j], protocol);
                SET_DMM_BUCKET_SW_ADDRESS((*currLpmPtr)->nextBucket->hwGroupOffsetHandle[j], ((*currLpmPtr)->nextBucket));
            }
        }

        /* If this level is not a root bucket then delete it if its empty.*/
        /* in multicast may be use another type leaf_trigger */
        if (((currLpmPtr != lpmPtrArray) ||
             (rootBucketFlag == PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E)) &&
            ((**currBucketPtrType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
             (**currBucketPtrType == CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E)))
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
)
{
    GT_U32                                  octetIndex;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *headOfListToFreePtr; /* head of the list of memories we want to free */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC      *tempElemToFreePtr;   /* temp pointer used for free operation */
    GT_U32                                  maxNumOfOctets=0;

    switch (protocolStack)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            maxNumOfOctets = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS;
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
                    tempElemToFreePtr = headOfListToFreePtr->nextMemInfoPtr;
                    headOfListToFreePtr->nextMemInfoPtr = headOfListToFreePtr->nextMemInfoPtr->nextMemInfoPtr;
                    cpssOsFree(tempElemToFreePtr);
                }
            }
        }
    }

    cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC  neededMemoryBlocksSizes[MAX_LPM_LEVELS_CNS];  /* Holds memory allocation  */
    GT_UINTPTR  memoryPoolList[MAX_LPM_LEVELS_CNS];            /* needs.                   */
    GT_U32      neededMemoryBlocksOctetsIndexs[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC  *neededMemoryBlocksPtr;
    GT_U32      i,j,k;
    GT_STATUS   retVal = GT_OK,retVal2=GT_OK;             /* Function return value.   */
    GT_U32      startIdx;
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC *neededMemoryBlocksSizesPtr;
    GT_U32      *neededMemoryBlocksOctetsIndexsPtr;
    GT_UINTPTR  *memoryPoolListPtr;
    GT_U32      newFreeBlockIndex; /* index of a new free block */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *tempNextMemInfoPtr;/* use for going over the list of blocks per octet */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *potentialNewBlockPtr=NULL;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                      the allocted new element that need to be freed.
                                                                                      Size of the array is 16 for case of IPV6 */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *firstMemInfoInListToFreePtr=NULL;/* use for going over the list of blocks per octet */

    GT_U32      octetIndex; /* the octet we are working on */
    GT_U32      blockStart=0; /* used as a start point for finding a free block */
    GT_BOOL     justBindAndDontAllocateFreePoolMem = GT_FALSE;
    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */

    cpssOsMemSet(neededMemoryBlocksSizes, 0, sizeof(neededMemoryBlocksSizes));
    neededMemoryBlocksPtr  = shadowPtr->neededMemoryBlocksInfo;

    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    /* remember the current list len, to go over only the added ones*/
    startIdx = shadowPtr->neededMemoryListLen;

    neededMemoryBlocksSizesPtr = &neededMemoryBlocksSizes[MAX_LPM_LEVELS_CNS-1];
    neededMemoryBlocksOctetsIndexsPtr = &neededMemoryBlocksOctetsIndexs[MAX_LPM_LEVELS_CNS-1];
    memoryPoolListPtr = &memoryPoolList[MAX_LPM_LEVELS_CNS-1];

    /* Get needed memory for LPM search insertion.  */
    retVal = lpmFalconCalcNeededMemory(bucketPtr,destArr,prefix,
                                       rootBucketFlag,lpmEngineMemPtrPtr,
                                       &neededMemoryBlocksSizesPtr,
                                       &memoryPoolListPtr,
                                       &shadowPtr->neededMemoryListLen,
                                       &neededMemoryBlocksOctetsIndexsPtr);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    /* keep values in case reconstruct is needed */
    cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

   /* reset neededMemoryBlocks before new allocations */
   /* cpssOsMemSet(neededMemoryBlocks+startIdx, (int)DMM_BLOCK_NOT_FOUND, (sizeof(GT_UINTPTR) * (shadowPtr->neededMemoryListLen-startIdx)));*/
    for (i = startIdx; i <shadowPtr->neededMemoryListLen; i++ )
    {
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            neededMemoryBlocksPtr[i].neededMemoryBlocks[j] = DMM_BLOCK_NOT_FOUND;
            neededMemoryBlocksPtr[i].neededMemoryBlocksSizes[j] = 0;
        }
        neededMemoryBlocksPtr[i].regularNode = GT_FALSE;
        neededMemoryBlocksPtr[i].octetId = 0xff;
    }
    /* Try to allocate all needed memory.               */
    for(i = startIdx; i < shadowPtr->neededMemoryListLen; i++)
    {
        memoryPoolListPtr++;
        neededMemoryBlocksSizesPtr++;
        neededMemoryBlocksOctetsIndexsPtr++;
        octetIndex = (*neededMemoryBlocksOctetsIndexsPtr);
        if (*memoryPoolListPtr!=0)
        {
            for (k = 0; k < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; k++)
            {
                if (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[k] > 0)
                {
                    neededMemoryBlocksPtr[i].neededMemoryBlocks[k] =
                    prvCpssDmmAllocate(*memoryPoolListPtr,
                                       DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[k]),
                                       DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                }
                if (neededMemoryBlocksSizesPtr->regularNode == GT_FALSE)
                {
                    break;
                }
            }
        }
        else
        {
            /* no memory pool is bound to the root of the octet list,
               in this case no need to allocate a memory pool struct just
               to bind it to a free pool */
            justBindAndDontAllocateFreePoolMem = GT_TRUE;

        }
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[j] =
            neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j];
        }
        shadowPtr->neededMemoryBlocksInfo[i].octetId = neededMemoryBlocksSizesPtr->octetId;
        shadowPtr->neededMemoryBlocksInfo[i].regularNode = neededMemoryBlocksSizesPtr->regularNode;
       /* shadowPtr->neededMemoryBlocksInfo[i].neededMemoryBlocksSizes[0] =
            neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[0];*/
        /* ??????????????????????????????????????????????????????????????????????*/
        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
        {
            /* in falcon we are checking for all group of nodes */
            if (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j] == 0)
            {
                continue;
            }
            if(neededMemoryBlocksPtr[i].neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND)
            {
                /* check if the octet is bound to more blocks,
                  if yes then try to allocte the memory again */

                 /* shadowPtr->neededMemoryListLen is the number of blocks
                    (just the first in the list of each octet) in neededMemoryBlocks
                    --> meaninng number of octets we want to add in the currect prefix */

                /* the search is from the last octet to the first one since
                   memoryPoolListPtr was initialized backwards in lpmCalcNeededMemory */
                tempNextMemInfoPtr = lpmEngineMemPtrPtr[octetIndex];

                while((tempNextMemInfoPtr->nextMemInfoPtr!= NULL)&&
                      (neededMemoryBlocksPtr[i].neededMemoryBlocks[j]==DMM_BLOCK_NOT_FOUND))
                {
                    neededMemoryBlocksPtr[i].neededMemoryBlocks[j] =
                        prvCpssDmmAllocate(tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool,
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j]),
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);
                    tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                }

                /* could not find an empty space in the current bound blocks - continue looking */
                if ((tempNextMemInfoPtr->nextMemInfoPtr==NULL)&&
                    (neededMemoryBlocksPtr[i].neededMemoryBlocks[j]==DMM_BLOCK_NOT_FOUND))
                {
                    /* go over all blocks until the memory of the octet can be allocated in the block found */
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
                        neededMemoryBlocksPtr[i].neededMemoryBlocks[j] =
                            prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex],
                                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS * (neededMemoryBlocksSizesPtr->neededMemoryBlocksSizes[j]),
                                               DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS);

                        /* neededMemoryBlocks is not 0 nor 0xFFFFFFFF --> we did succeed in allocating the
                        needed memory from the valid block we found  */
                        if ((neededMemoryBlocksPtr[i].neededMemoryBlocks[j] != DMM_BLOCK_NOT_FOUND)&&
                            (neededMemoryBlocksPtr[i].neededMemoryBlocks[j] != DMM_MALLOC_FAIL))
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
                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j]);

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
                            if (neededMemoryBlocksPtr[i].neededMemoryBlocks[j] != DMM_MALLOC_FAIL)
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
                    /* neededMemoryBlocks is not 0 nor 0xFFFFFFFF --> then we did succeed in allocating the needed memory
                       --> save the allocated pool Id */
                    if ((neededMemoryBlocksPtr[i].neededMemoryBlocks[j] == DMM_BLOCK_NOT_FOUND)||
                        (neededMemoryBlocksPtr[i].neededMemoryBlocks[j]== DMM_MALLOC_FAIL))
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
                else
                {
                    /* we had a CPU Memory allocation error */
                    if(neededMemoryBlocksPtr[i].neededMemoryBlocks[j] == DMM_MALLOC_FAIL)
                    {
                        retVal = GT_OUT_OF_CPU_MEM;
                        break;
                    }
                    else
                    {
                        /* mark the block as used */
                        /* set pending flag for future need */
                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j]);
                    }
                }
            }
            else
            {
            /* we had a CPU Memory allocation error */
                if(neededMemoryBlocksPtr[i].neededMemoryBlocks[j] == DMM_MALLOC_FAIL)
                {
                    retVal = GT_OUT_OF_CPU_MEM;
                    break;
                }
                else
                {
                    /*We succeed in allocating the memory*/
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                    /* set pending flag for future need */
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(neededMemoryBlocksPtr[i].neededMemoryBlocks[j]);
                }
            }
        }

        justBindAndDontAllocateFreePoolMem = GT_FALSE;

        if (retVal != GT_OK)
        {
            break;
        }
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
            for (j= 0; j < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; j++)
            {
                if ((neededMemoryBlocksPtr[i - 1].neededMemoryBlocks[j] != DMM_BLOCK_NOT_FOUND) ||
					(neededMemoryBlocksPtr[i - 1].neededMemoryBlocks[j] != DMM_MALLOC_FAIL) )
                {
                    /*We fail in allocating the memory*/
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(neededMemoryBlocksPtr[i - 1].neededMemoryBlocks[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                    /* set pending flag for future need */
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;

                    prvCpssDmmFree(neededMemoryBlocksPtr[i - 1].neededMemoryBlocks[j]);
                }
				if(neededMemoryBlocksPtr[i - 1].regularNode == GT_FALSE)
                {
                    break;
                }
            }
            i--;
        }

        shadowPtr->neededMemoryListLen = 0;

        /* free the allocated/bound RAM memory */
        retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
        if (retVal2!=GT_OK)
        {
            return retVal2;
        }
        /* in case of fail we will need to reconstruct to the status we had after the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
        cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
               sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

        return retVal;
    }
    return retVal;
}






/*******************************************************************************
* prvCpssDxChLpmRamMngHandleRootNodeAllocation
*
* DESCRIPTION:
*       allocate lpm hw memory for root node
*
* INPUTS:
*       tempNextMemInfoPtr          - pointer used for going over the list of blocks per octet
*       memSize                     - memory size in lines
*       minAllocationSizeInBytes    - minimum allocation size in bytes
*       protocol                    - the protocol
*       octetIndex                  - can be 0 for the root regular bit vector or
*                                     1 for the GON of the root
*       shadowPtr                   - pointer to LPM shadow
*
* OUTPUTS:
*       hwBucketOffsetHandlePtr     - pointer to hw bucket offset handle.
*
* RETURNS:
*       GT_OK                    - If there is enough memory for the insertion.
*       GT_OUT_OF_PP_MEM         - otherwise.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_STATUS prvCpssDxChLpmRamMngHandleRootNodeAllocation
(
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC          *tempNextMemInfoPtr,
    IN  GT_U32                                      memSize,
    IN  GT_U32                                      minAllocationSizeInBytes,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocol,
    IN  GT_U32                                      octetIndex,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    OUT GT_UINTPTR                                  *hwBucketOffsetHandlePtr
)
{
    GT_UINTPTR hwBucketOffsetHandle = DMM_BLOCK_NOT_FOUND;
    GT_U32 blockStart = 0;
    GT_U32 newFreeBlockIndex; /* index of a new free block */
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *potentialNewBlockPtr=NULL;
    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                        the allocted new element that need to be freed.
                                                                                        Size of the array is 16 for case of IPV6 */

    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    hwBucketOffsetHandle = prvCpssDmmAllocate(tempNextMemInfoPtr->structsMemPool,
                                              minAllocationSizeInBytes * memSize,
                                              minAllocationSizeInBytes);
    if ( (hwBucketOffsetHandle != DMM_MALLOC_FAIL) && (hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) )
    {
        *hwBucketOffsetHandlePtr = hwBucketOffsetHandle;

        /*  set pending flag for future need */
        shadowPtr->pendingBlockToUpdateArr[tempNextMemInfoPtr->ramIndex].updateInc=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[tempNextMemInfoPtr->ramIndex].numOfIncUpdates += memSize;

        return GT_OK;
    }
    if (hwBucketOffsetHandle == DMM_MALLOC_FAIL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* go over all blocks bound to the octet and try to allocate */
    while((tempNextMemInfoPtr->nextMemInfoPtr != NULL) && (hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND))
    {
        hwBucketOffsetHandle = prvCpssDmmAllocate(tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool,
                                                       minAllocationSizeInBytes * memSize,
                                                       minAllocationSizeInBytes);
        blockIndex = tempNextMemInfoPtr->nextMemInfoPtr->ramIndex;
        tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
    }

    /* could not find an empty space in the current bound blocks - continue looking */
    if ((tempNextMemInfoPtr->nextMemInfoPtr == NULL) && (hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND))
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
             hwBucketOffsetHandle = prvCpssDmmAllocate(shadowPtr->lpmRamStructsMemPoolPtr[newFreeBlockIndex],
                                   minAllocationSizeInBytes * memSize,
                                   minAllocationSizeInBytes);

            /* neededMemoryBlocks is not 0 nor 0xFFFFFFFF --> we did succeed in allocating the
               needed memory from the valid block we found */
            if ((hwBucketOffsetHandle != DMM_BLOCK_NOT_FOUND) && (hwBucketOffsetHandle != DMM_MALLOC_FAIL))
            {
                /* allocate a new elemenet block to the list */
                potentialNewBlockPtr =  (PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC));
                if (potentialNewBlockPtr == NULL)
                {
                    /*goto delete_ranges*/
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
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
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(hwBucketOffsetHandle);

                /* We only save the first element allocated or bound per octet
                   we are dealing now with the root so we will get here twice
                   first time for the regular but vector located at bank0 (octet 0)
                   second time for the GONs of this bit vector located at bank 1 (octet 1)
                   for SIP 6 we look at the octets like we have 5 octets for ipv4 and 17 octets for ipv6 */
                if ((allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]==NULL)||
                    (allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex]->structsMemPool==0))
                {
                    /* keep the head of the list we need to free incase of an error */
                    allNewNextMemInfoAllocatedPerOctetArrayPtr[octetIndex] = tempNextMemInfoPtr;
                }

                break;
            }
            /* if the neededMemoryBlocks is 0 or 0xFFFFFFFF --> then we did not succeed in allocating the
                needed memory from the valid block we found, look for another valid block or
                return an error incase of CPU memory allocation fail  */
            else
            {
                if (hwBucketOffsetHandle != DMM_MALLOC_FAIL)
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
        if ((hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND) || (hwBucketOffsetHandle == DMM_MALLOC_FAIL))
        {
            if (hwBucketOffsetHandle == DMM_BLOCK_NOT_FOUND)
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
        if(hwBucketOffsetHandle == DMM_MALLOC_FAIL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        else
        {
            /*We succeed in allocating the memory*/
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
            /*  set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates += memSize;

        }
    }
    *hwBucketOffsetHandlePtr = hwBucketOffsetHandle;
    return retVal;
}

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
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *newRootBucket;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC              **lpmEngineMemPtrPtr;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *rangeListPtr;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *secondRangePtr = NULL;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *thirdRangePtr = NULL;
    GT_U8                                           *shareDevsList;
    GT_U8                                           shareDevListLen, devNum;
    GT_U32                                          memSize;
    GT_U8                                           prefixLength;
    GT_STATUS                                       retVal = GT_OK,retVal2 = GT_OK;
    GT_PTR                                          nextHopEntry = NULL;
    GT_U32                                          secondRangePrefixLength = 0;
    GT_U32                                          minAllocationSizeInBytes = DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS;
    GT_U32                                          i = 0;

    GT_U32      blockIndex=0; /* calculated according to the memory offset devided by block size including gap */

    static PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   groupOfNodes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
    static PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   rootGroupOfNodes;

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

    /* keep values in case reconstruct is needed */
    cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

    /* Create the root bucket. It's created with startAddr 0, which is the start
       of the UC address space */
    newRootBucket = prvCpssDxChLpmSip6RamMngCreateNew(defUcNextHopEntryPtr,
                                                      PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS);
    if (newRootBucket == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    rangeListPtr = newRootBucket->rangeList;
    rangeListPtr->updateRangeInHw = GT_FALSE;

    PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defUcNextHopEntryPtr->routeEntryMethod,rangeListPtr->pointerType);

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
        PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defMcNextHopEntryPtr->routeEntryMethod,secondRangePtr->pointerType);
        secondRangePtr->lowerLpmPtr.nextHopEntry = defMcNextHopEntryPtr;
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            secondRangePtr->next = NULL;
        }
        secondRangePtr->updateRangeInHw = GT_FALSE;

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
            PRV_CPSS_DXCH_SIP6_LPM_NEXT_PTR_TYPE_CONVERT_MAC(defReservedNextHopEntryPtr->routeEntryMethod,thirdRangePtr->pointerType);
            thirdRangePtr->lowerLpmPtr.nextHopEntry = defReservedNextHopEntryPtr;
            thirdRangePtr->next = NULL;
            thirdRangePtr->updateRangeInHw = GT_FALSE;

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
    /* new node type for SIP6 architecture */
    newRootBucket->bucketType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;

    if (updateHw == GT_TRUE)
    {
        /* both unicast and multicast use ucSearchMemArrayPtr */
        lpmEngineMemPtrPtr = shadowPtr->ucSearchMemArrayPtr[protocol];
        memSize = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
        shareDevsList = shadowPtr->workDevListPtr->shareDevs;
        shareDevListLen = (GT_U8)shadowPtr->workDevListPtr->shareDevNum;

        /* allocate memory for root bucket */
        retVal = prvCpssDxChLpmRamMngHandleRootNodeAllocation(lpmEngineMemPtrPtr[0], memSize,
                                                              minAllocationSizeInBytes,protocol,0,shadowPtr,
                                                              &newRootBucket->hwBucketOffsetHandle);
        if (retVal != GT_OK)
        {
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
            retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
            if (retVal2!=GT_OK)
            {
                return retVal2;
            }
            /* in case of fail we will need to reconstruct to the status we had before the call to prvCpssDxChLpmRamMngFindValidMemoryBlockForProtocol */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
            return retVal;
        }
        /* allocate memory for 6 group of nodes root node pointed on */
        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
        {
            memSize = 1; /* max group of nodes is 3 leaves - 1 line */
            /* group of nodes must be in bank associated with the next octet */
            retVal = prvCpssDxChLpmRamMngHandleRootNodeAllocation(lpmEngineMemPtrPtr[1], memSize,
                                                                  minAllocationSizeInBytes,protocol,1,shadowPtr,
                                                                  &newRootBucket->hwGroupOffsetHandle[i]);
            if (retVal != GT_OK)
            {
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
                retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
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
        /* build group of nodes data */
        cpssOsMemSet(groupOfNodes, 0, sizeof(groupOfNodes));
        for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
        {

            retVal = getMirrorGroupOfNodesDataAndUpdateRangesAddress(CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,
                                                                     startSubNodeAddress[i],
                                                                     endSubNodeAddress[i],
                                                                     newRootBucket->rangeList,
                                                                     shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                                                     &groupOfNodes[i]);
            if (retVal != GT_OK)
            {
                 /* just need to free any allocations done in previous stage */

                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle);

                prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);
                for (i = 0; i < 6; i++)
                {
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwGroupOffsetHandle[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwGroupOffsetHandle[i]);

                    prvCpssDmmFree(newRootBucket->hwGroupOffsetHandle[i]);
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
            }
        }
        /* prepare the same for root node */
        cpssOsMemSet(&rootGroupOfNodes,0,sizeof(rootGroupOfNodes));
        retVal = lpmFalconFillRegularData(newRootBucket,
                                          shadowPtr->lpmRamTotalBlocksSizeIncludingGap,
                                          &rootGroupOfNodes.regularNodesArray[0]);
        if (retVal != GT_OK)
        {
            /* just need to free any allocations done in previous stage */


            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle);

            prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);
            for (i = 0; i < 6; i++)
            {
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwGroupOffsetHandle[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwGroupOffsetHandle[i]);

                prvCpssDmmFree(newRootBucket->hwGroupOffsetHandle[i]);
            }

            return retVal;
        }

        for (devNum = 0; devNum < shareDevListLen; devNum++)
        {

            /* write root group of nodes */
            for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
            {

                retVal = prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[devNum],
                                                         PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwGroupOffsetHandle[i]),
                                                         &groupOfNodes[i]);
                if (retVal != GT_OK)
                {
                    /* just need to free any allocations done in previous stage */
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle);

                    prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);
                    for (i = 0; i < 6; i++)
                    {
                        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwGroupOffsetHandle[i])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                        blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                            PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwGroupOffsetHandle[i]);

                        prvCpssDmmFree(newRootBucket->hwGroupOffsetHandle[i]);
                    }
                    return retVal;
                }
            }
            /* write regular root node */
            retVal = prvCpssDxChLpmGroupOfNodesWrite(shareDevsList[devNum],
                                                     PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle),
                                                     &rootGroupOfNodes);
            if (retVal != GT_OK)
            {
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                blockIndex = blockIndex - shadowPtr->memoryOffsetValue;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                    PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(newRootBucket->hwBucketOffsetHandle);

                prvCpssDmmFree(newRootBucket->hwBucketOffsetHandle);

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
                retVal2 = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
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

    for (i = 0; i < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; i++)
    {
         if (newRootBucket->hwGroupOffsetHandle[i]!=0)
         {
             SET_DMM_BLOCK_PROTOCOL(newRootBucket->hwGroupOffsetHandle[i], protocol);
             SET_DMM_BUCKET_SW_ADDRESS(newRootBucket->hwGroupOffsetHandle[i], newRootBucket);
         }
    }

    shadowPtr->vrRootBucketArray[vrId].rootBucket[protocol] = newRootBucket;
    shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol] =
                                newRootBucket->bucketType;
    shadowPtr->vrRootBucketArray[vrId].valid = GT_TRUE;
    shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_FALSE;
    if (protocol != PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        shadowPtr->vrRootBucketArray[vrId].multicastDefault[protocol] =
            defMcNextHopEntryPtr;
    }

    return GT_OK;
}




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
)
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC *pBucket; /* The first level bucket */

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRange;

    pBucket = createNewSip6Bucket(nextHopPtr,firstLevelPrefixLen);
    if(pBucket == NULL)
        return NULL;

    pRange = pBucket->rangeList;

    /* beacuse it's a new bucket there is a need to update it in the HW */
    pRange->updateRangeInHw = GT_TRUE;

    return pBucket;
}



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
)
{
    GT_STATUS   rc;
    GT_U32      i,j,l,leavesCounter;
    int k;
    GT_U32      *groupOfNodesArrayPtr;
    GT_U32      *tempGonArrayPtr;
    GT_U32      bankNumber = 0;
    GT_U32      offsetInBank = 0;
    GT_U32      lpmLinesNumber = 0;
    GT_U32      value = 0;
    GT_BOOL     startNewGonLeaves = GT_FALSE;
    GT_U32      groupOfNodesArray[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_WORDS_CNS] = {0};
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
       CPSS_XCAT3_E |  CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E |
       CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(groupOfNodesPtr);

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,CPSS_DXCH_SIP5_TABLE_LPM_MEM_E);
    tableInfoPtr->entrySize = 4;
    groupOfNodesArrayPtr = groupOfNodesArray;
    /* start processing from regular*/
    for (j=0; j<NUMBER_OF_RANGES_IN_SUBNODE; j++)
    {
        if (groupOfNodesPtr->regularNodesArray[j].childNodeTypes0_255[0] == 0)
        {
            break;
        }

        /* take in account 4 reserved, for bit vector start k =3 */
        /* process regular line */
        for (l = 0; l < PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS; l++)
        {
            tempGonArrayPtr = groupOfNodesArrayPtr;
            groupOfNodesArrayPtr = groupOfNodesArrayPtr +3; /* starting from the last word of BV line */
            /* write number of leaves in first 2 bits: for implemented case it is alwase 0*/
            U32_SET_FIELD_MASKED_MAC(*groupOfNodesArrayPtr, 16, 2, 0);
            /* bits from 111 .... 108 are reserved, so in each bit vector line range info is started */
            /* from bit number 106 ->10 bit in word -> it means k = 5 */
            k = 5;
            for (i = startSubNodeAddress[l]; i <= endSubNodeAddress[l]; i++,k--)
            {
                U32_SET_FIELD_MASKED_MAC(*groupOfNodesArrayPtr, k *2, 2,
                                          groupOfNodesPtr->regularNodesArray[j].childNodeTypes0_255[i]);
                if (k==0)
                {
                    k = 16;
                    groupOfNodesArrayPtr--;
                }
                if (i == 255)
                {
                    /* This is last range, go bank and offset word */
                    groupOfNodesArrayPtr--;
                }
            }

            /* add pointer bits 19 -0: 19-15 bank number, 14-0 offset in bank */
            offsetInBank = groupOfNodesPtr->regularNodesArray[j].lpmOffsets[l] & 0x7fff;
            bankNumber = (groupOfNodesPtr->regularNodesArray[j].lpmOffsets[l] >> 15) & 0x1f;
            U32_SET_FIELD_MASKED_MAC(*groupOfNodesArrayPtr,15,5,bankNumber & 0x1f);
            U32_SET_FIELD_MASKED_MAC(*groupOfNodesArrayPtr,0,15,offsetInBank & 0x7fff);
            if (groupOfNodesArrayPtr != tempGonArrayPtr)
            {
                cpssOsPrintf("prvCpssDxChLpmGroupOfNodesWrite: wrong GON ptr\n ");
            }
            /* Now bit vector line is ready. Move pointer to next bv line */
            groupOfNodesArrayPtr = groupOfNodesArrayPtr + 4;
            lpmLinesNumber++;
        }
    }

    /* start processing compressed */
    for (j=0; j<NUMBER_OF_RANGES_IN_SUBNODE; j++)
    {
        if (groupOfNodesPtr->compressedNodesArray[j].childNodeTypes0_9[0] == 0)
        {
            break;
        }
        /*check compress type*/
        rc = prvCpssDxChLpmFalconCompressedNodeDataBuild(&groupOfNodesPtr->compressedNodesArray[j],
                                                         &groupOfNodesArrayPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        lpmLinesNumber++;
    }
    /* start processing leaves*/
    /* There is 2 types of leaves unicast and multicast*/
    /* in meantime only unicast leaf is implemented    */
    leavesCounter = 0; /* leaves counter */
    k = 0; /* offset inside word */
    startNewGonLeaves = GT_TRUE;
    for (j=0; j<NUMBER_OF_RANGES_IN_SUBNODE; j++)
    {
        value = 0;
        if (groupOfNodesPtr->leafNodesArray[j].entryType == 0)
        {
            break;
        }
        /* in one lpm entry- 4 words-  we have 5 leaves*/
        rc = prvCpssDxChLpmFalconLeafNodeDataBuild(&groupOfNodesPtr->leafNodesArray[j],
                                                   leavesCounter,
                                                   startNewGonLeaves,
                                                   &groupOfNodesArrayPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        startNewGonLeaves = GT_FALSE;
        leavesCounter++;
    }
    value = leavesCounter /MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
    if ((leavesCounter%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS) != 0)
    {
        value++;
    }
    lpmLinesNumber = lpmLinesNumber + value;

    /* Write the node */
    rc = prvCpssDxChWriteTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                         lpmGroupLineOffset,
                                         lpmLinesNumber,
                                         &groupOfNodesArray[0]);

    return rc;

}
