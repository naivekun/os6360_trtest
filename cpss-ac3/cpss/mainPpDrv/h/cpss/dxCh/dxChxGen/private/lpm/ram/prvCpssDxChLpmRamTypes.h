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
* @file prvCpssDxChLpmRamTypes.h
*
* @brief Private cpss DXCH LPM RAM library type definitions
*
* @version   8
********************************************************************************
*/
#ifndef __prvCpssDxChLpmRamTypesh
#define __prvCpssDxChLpmRamTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTrie.h>

/* Defines */
/* maximal prefix length and trie depth for level (octet) */
#define PRV_CPSS_DXCH_LPM_RAM_MAX_LEVEL_LENGTH_CNS  (8)

/* 3 unicast protocols: IPv4, IPv6, FCoE */
#define PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS   (3)

/* 2 multicast protocols: IPv4, IPv6 */
#define PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS   (2)

/* the lower possible value of IPv4 MC first octet */
#define PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS   (224)

/* the higher possible value of IPv4 MC first octet */
#define PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS   (239)

/* the lower possible value of IPv4 reserved space first octet */
#define PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS (240)

/* the higher possible value of IPv4 reserved space first octet */
#define PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS   (255)

/* the lower possible value of IPv6 MC first octet */
#define PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS   (255)

/* the higher possible value of IPv6 MC first octet */
#define PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS   (255)

#define PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS   (256)

/* the prefix that covers the IPv4 MC address space */
#define PRV_CPSS_DXCH_LPM_RAM_IPV4_MC_ADDRESS_SPACE_PREFIX_CNS (4)

/* the prefix that covers the IPv4 reserved space space */
#define PRV_CPSS_DXCH_LPM_RAM_IPV4_RESERVED_ADDRESS_SPACE_PREFIX_CNS (4)

/* the prefix that covers the IPv6 MC address space */
#define PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_ADDRESS_SPACE_PREFIX_CNS (8)

/* size of one LPM entry in bytes (in the address space) */
#define PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS  (4)

/* size of one LPM entry in words  */
#define PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS  \
                (PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS / 4)

/* number of ranges in LPM line */
#define PRV_CPSS_DXCH_LPM_RAM_NUM_OF_RANGES_IN_LPM_LINE_CNS   (4)

/* Number of entries in the NH and ECMP tables that are kept for default */
#define PRV_CPSS_DXCH_LPM_RAM_NUM_OF_DEFAULT_ENTRIES          (3)

/* Bit vector for one-line compressed bucket */
#define PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS      1

/* Bit vector for two-lines compressed bucket */
#define PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS      2

/* Bit vector for regular bucket */
#define PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS           11

/* Number of RAM memory blocks */
#define PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_CNS   20
#define PRV_CPSS_DXCH_LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS _16KB

/* maximum size of LPM bucket (256 ranges + 11 lines for bitmap) */
#define PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS       \
    (256 + PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS)
#define PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_BYTES_CNS           \
    (PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS *        \
     PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS)

#define MAX_NUMBER_OF_COMPRESSED_1_RANGES_CNS   5
#define MAX_NUMBER_OF_COMPRESSED_2_RANGES_CNS   10

/* size of LPM block */
#define PRV_CPSS_DXCH_LPM_RAM_BLOCK_SIZE_IN_LINES_CNS    (16384)
#define PRV_CPSS_DXCH_LPM_RAM_BLOCK_SIZE_IN_BYTES_CNS    (PRV_CPSS_DXCH_LPM_RAM_BLOCK_SIZE_IN_LINES_CNS * PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS)
/*////////////////////////////////////////////////////////////////////////////////////*/
/* SIP6 (Falcon) definitions */
/*////////////////////////////////////////////////////////////////////////////////////*/
/* Number of non empty child types possible: leaf, regular, compress */
#define PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS   3
#define PRV_CPSS_DXCH_LPM_MAX_RANGES_IN_COMPRESSED_NODE_E   10
#define PRV_CPSS_DXCH_LPM_MAX_RANGES_IN_EMBEDDED_1_NODE_E    7
#define PRV_CPSS_DXCH_LPM_MAX_RANGES_IN_EMBEDDED_2_NODE_E    5
#define PRV_CPSS_DXCH_LPM_MAX_RANGES_IN_EMBEDDED_3_NODE_E    3
#define PRV_CPSS_DXCH_LPM_MAX_LEAVES_IN_EMBEDDED_3_NODE_E    3
#define PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS   30
#define PRV_CPSS_DXCH_LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_FALCON_CNS _32K

#define MIN_NUMBER_OF_BIT_VECTOR_RANGES_CNS     11
#define MAX_NUMBER_OF_COMPRESSED_RANGES_CNS     10
#define MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS    5
#define START_ADDR_OF_SUBNODE_0_IN_GROUP_OF_NODES 0
#define END_ADDR_OF_SUBNODE_0_IN_GROUP_OF_NODES 43
#define START_ADDR_OF_SUBNODE_1_IN_GROUP_OF_NODES 44
#define END_ADDR_OF_SUBNODE_1_IN_GROUP_OF_NODES 87
#define START_ADDR_OF_SUBNODE_2_IN_GROUP_OF_NODES 88
#define END_ADDR_OF_SUBNODE_2_IN_GROUP_OF_NODES 131
#define START_ADDR_OF_SUBNODE_3_IN_GROUP_OF_NODES 132
#define END_ADDR_OF_SUBNODE_3_IN_GROUP_OF_NODES 175
#define START_ADDR_OF_SUBNODE_4_IN_GROUP_OF_NODES 176
#define END_ADDR_OF_SUBNODE_4_IN_GROUP_OF_NODES 219
#define START_ADDR_OF_SUBNODE_5_IN_GROUP_OF_NODES 220
#define END_ADDR_OF_SUBNODE_5_IN_GROUP_OF_NODES 255
#define NUMBER_OF_RANGES_IN_SUBNODE 44
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS 6
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS  16
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_PBR_SIZE_IN_LPM_LINES_CNS _32K
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_PBR_SIZE_MIN_LPM_MODE_IN_LPM_LINES_CNS _5K

/* size of FALCON LPM entry in bits */
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BITS_CNS  (115)
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS  (4)

/* max group of nodes size in LPM lines  */
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_LPM_LINES_CNS  \
                (NUMBER_OF_RANGES_IN_SUBNODE * PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS )

/* number of bit vector lines in bit vector */
#define PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS    (6)

/* Size of bit vector node in FALCON LPM */
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS      (6)
/* Size of compressed node in FALCON LPM */
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS      1

/* max group of nodes size in words  */
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_WORDS_CNS  \
                (PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_GROUP_OF_NODES_SIZE_IN_LPM_LINES_CNS*\
                PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS )
#define PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_BYTES_CNS           \
    (PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS *        \
     PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS)



/* offset from the start of the memory block in lines */
#define PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(handler)  (DMM_GET_OFFSET(handler)>> 2)
/* size of memory block in SIP5 lines */
#define PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(handler)    (DMM_GET_SIZE(handler) >> 2)

#define PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(handler)    (DMM_GET_SIZE(handler)>> 4)
#define PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(handler)  (DMM_GET_OFFSET(handler)>> 4)

/* Typedefs */
/*
 * Typedef: enum PRV_CPSS_DXCH_LPM_RAM_MERGE_METHOD_ENT
 *
 * Description: The different 4 types of range merging when inserting a new
 *              prefix to the LPM structure.
 *
 * Enumerations:
 *  PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E - overwrite merge
 *  PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E      - high merge
 *  PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E       - low merge
 *  PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E       - mid merge
 */
typedef enum _mergeMethod
{
    PRV_CPSS_DXCH_LPM_RAM_MERGE_OVERWRITE_E = 0,
    PRV_CPSS_DXCH_LPM_RAM_MERGE_HIGH_E      = 1,
    PRV_CPSS_DXCH_LPM_RAM_MERGE_LOW_E       = 2,
    PRV_CPSS_DXCH_LPM_RAM_MERGE_MID_E       = 3
}PRV_CPSS_DXCH_LPM_RAM_MERGE_METHOD_ENT;

/*
 * Typedef: enum PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT
 *
 * Description: The different 4 types of range splitting when deleting a
 *              prefix to the LPM structure.
 *
 * Enumerations:
 *  PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E       - overwrite split
 *  PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E      - high split
 *  PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E       - low split
 *  PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E       - mid split
 */
typedef enum _splitMethod
{
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_OVERWRITE_E   = 0,
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_HIGH_SPLIT_E  = 1,
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_LOW_SPLIT_E   = 2,
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_MID_SPLIT_E   = 3,
    PRV_CPSS_DXCH_LPM_RAM_SPLIT_LAST_E
}PRV_CPSS_DXCH_LPM_RAM_SPLIT_METHOD_ENT;

/**
* @union PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT
 *
 * @brief A pointer to the next LPM level.
 *
*/

typedef union
{
    struct PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STCT   *nextBucket;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC         *nextHopEntry;
} PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STCT
 *
 * Description: SW representation of a range in bucket.
 *
 * Fields:
 *      lowerLpmPtr     - The pointer to the lower LPM bucket / next hop / ECMP
 *                        entry, according to the value of pointerType.
 *      next            - A pointer to the next range in the ranges list.
 *      startAddr       - The lower address of the range.
 *      mask            - A bit mask indicating the length of the prefixes
 *                        covering this range.
 *      updateRangeInHw - indicates whether to update this range in the HW
 *      pointerType     - the type of pointer, See CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT.
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC
{
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT         lowerLpmPtr;
    struct PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *next;
    GT_U8                                            startAddr;
    GT_U8                                            mask;
    GT_BOOL                                          updateRangeInHw;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT              pointerType;
} PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT
 *
 * @brief Indicates what kind of insert to the lpm tree is being preformed.
*/
typedef enum{

    /** @brief a regular lpm insertion
     *  where the shadow, memory alloc, and hw are updated.
     */
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,

    /** @brief a hot sync styple lpm
     *  insertion, where only the shadow is updated.
     */
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E,

    /** @brief a bulk style lpm insertion.
     *  where the shadow and the memory allocation are the
     *  only ones updated , without the HW.
     */
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E

} PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_TYPE_ENT
 *
 * @brief the type of update the lpm trie needs
*/
typedef enum{

    /** @brief write the whole trie
     *  to the HW and allocate memory if needed.
     */
    PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E   = 0,

    /** @brief update the Trie without any
     *  memory allocation.(memory should already be allocated correctly)
     */
    PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_ONLY_E,

    /** @brief update the trie and
     *  allocate memory if needed.
     */
    PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E

} PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_UPDATE_STATUS_ENT
 *
 * @brief the status of the bucket in HW
*/
typedef enum{

    /** the bucket is fully updated in the HW */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_E = 0,

    /** the bucket needs update in HW of some of it's pointers */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_NEEDS_UPDATE_E,

    /** the bucket was resized and needs rewriting in the HW */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_WAS_RESIZED_AND_NEEDS_REWRITING_E,

    /** the bucket is fully updated in HW, but it's siblings needs HW update */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_OK_UPDATE_SIBLINGS_E

} PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_UPDATE_STATUS_ENT;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STCT
 *
 * Description: A struct representing an LPM level's bucket.
 *
 * Fields:
 *      pointingRangeMemAddr  - the parnet bucket's range memory address (required
 *                              mainly for bulk operations and swap mem usage,
 *                              0 indicates that this bucket is pointed from the
 *                              root bucke and may have several place it's pointed
 *                              from)
 *      trieRoot              - The root of the Trie to save all insertions history.
 *      rangeList             - A pointer to the first range in this bucket.
 *      hwBucketOffsetHandle  - The handle of HW Address of the bucket. The
 *                              address in this field is an offset from
 *                              structsBase.
 *      rangeCash             - cash used to make searching faster.
 *      numOfRanges           - Number of ranges in the list
 *      bucketHwUpdateStat    - Hw bucket update status
 *      bucketType            - this bucket type, can be
 *                              CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E or
 *                              CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E or
 *                              CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E
 *      fifthAddress          - the fifth address of the bucket in case of a
 *                              two-lines compressed bucket
 *      isImplicitGroupDefault- for MC group buckets:
 *                              GT_TRUE: group default was added implicitly
 *                              GT_FALSE: group default was added explicitly
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STCT
{
    GT_U32                                              pointingRangeMemAddr;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_NODE_STC                 trieRoot;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangeList;
    GT_UINTPTR                                          hwBucketOffsetHandle;
    GT_UINTPTR                                          hwGroupOffsetHandle[6];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC              *rangeCash;
    GT_U16                                              numOfRanges;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_HW_UPDATE_STATUS_ENT   bucketHwUpdateStat;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT                 bucketType;
    GT_U8                                               fifthAddress;
    GT_BOOL                                             isImplicitGroupDefault;
} PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STCT
 *
 * Description: LPM Engine memory configuration information
 *
 * Fields:
 *
 *      ramIndex         - the HW RAM index in the shadow memory configuration
 *      structBase       - the base addresses of the LPM block in lines
 *      structsMemPool   - The memory pool Id that manages the memory for search
 *                         structures allocation.
 *      nextMemInfoPtr   - (pointer to) the next memory info structure
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STCT
{
    GT_U32      ramIndex;
    GT_U32      structsBase;
    GT_UINTPTR  structsMemPool;
    struct PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STCT* nextMemInfoPtr;
} PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_TREE_ITERATOR_STC
 *
 * Description: A struct that holds an lpm tree iterator info
 *
 * Fields:
 *      rangePtrArray   - an array of pointers to ranges in the tree .
 *      currRangePtr - a pointer to the current range we are dealing with.
 *      currLpmEnginePtr - the pointer to the current place in the lpm engine
 *                         array.
 */
typedef struct _lpmTreeIterator
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    *rangePtrArray[17];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC    **currRangePtr;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC        **currLpmEnginePtr;
}PRV_CPSS_DXCH_LPM_RAM_TREE_ITERATOR_STC,*PRV_CPSS_DXCH_LPM_RAM_TREE_ITERATOR_STC_PTR;

/*******************************************************************************
* PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_PTR
*
* DESCRIPTION:
*       Writes a bucket parent ptr data
*
* INPUTS:
*       data - the data for the function.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK              - on success.
*       GT_FAIL            - on error
*
* COMMENTS:
*       Configures an entry in the prefix look up table
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_PTR)
(
    IN  GT_PTR                  data
);

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC
 *
 * Description: A struct that holds an lpm bucket parent date write function
 *              information
 *
 * Fields:
 *      bucketParentWriteFunc   - pointer to function .
 *      data - a pointer to data the function should get.
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STCT
{
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_PTR  bucketParentWriteFunc;
    GT_PTR                                              data;
}PRV_CPSS_DXCH_LPM_RAM_BUCKET_PARENT_WRITE_FUNC_INFO_STC;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_VR_TBL_UPDATE_TYPE_ENT
 *
 * @brief Indicates which the entries should be updated
 * and the way they should be updated in the VR router table.
*/
typedef enum{

    /** UC entries only. */
    PRV_CPSS_DXCH_LPM_RAM_VR_TBL_UPDATE_UC_E = 0,

    /** MC entries only. */
    PRV_CPSS_DXCH_LPM_RAM_VR_TBL_UPDATE_MC_E,

    /** UC & MC entries. */
    PRV_CPSS_DXCH_LPM_RAM_VR_TBL_UPDATE_UC_MC_E

} PRV_CPSS_DXCH_LPM_RAM_VR_TBL_UPDATE_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT
 *
 * @brief Defines the different operation done while traversing the ip
 * structures
*/
typedef enum{

    /** set the memory in the LPM structure */
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_SET_E,

    /** record the memory in the LPM structure */
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_REC_E,

    /** count the amount of memory entries in LPM structure. */
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_E,

    /** @brief set the memory in the LPM structure and LPM
     *  PCL structure (IPv6 MC)
     */
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_SET_WITH_PCL_E,

    /** @brief record the memory in the LPM structure and
     *  LPM PCL structure (IPv6 MC)
     */
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_REC_WITH_PCL_E,

    /** @brief count the amount of memory entries in
     *  LPM structure and LPM PCL structure (IPv6 MC)
     */
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_WITH_PCL_E

} PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STCT
 *
 * Description: IPv4 UC Prefix information for bulk operations
 *
 * Fields:
 *      vrId                    - the virtual router id
 *      ipAddr                  - the destination IPv4 address of this prefix
 *      prefixLen               - the prefix length of ipAddr
 *      nextHopInfo             - the route entry info accosiated with this UC
 *                                prefix
 *      override                - GT_TRUE:  override an existing prefix.
 *                                GT_FALSE: don't override an existing prefix,
 *                                and return an error
 *      returnStatus            - the return status for this prefix
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STCT
{
    GT_U32                                     vrId;
    GT_IPADDR                                  ipAddr;
    GT_U32                                     prefixLen;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  nextHopInfo;
    GT_BOOL                                    override;
    GT_STATUS                                  returnStatus;
} PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STCT
 *
 * Description: IPv6 UC Prefix information for bulk operations
 *
 * Fields:
 *      vrId                    - the virtual router id
 *      ipAddr                  - the destination IPv6 address of this prefix
 *      prefixLen               - the prefix length of ipAddr
 *      nextHopInfo             - the route entry info accosiated with this UC
 *                                prefix
 *      override                - GT_TRUE:  override an existing prefix.
 *                                GT_FALSE: don't override an existing prefix,
 *                                and return an error
 *      returnStatus            - the return status for this prefix
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STCT
{
    GT_U32                                     vrId;
    GT_IPV6ADDR                                ipAddr;
    GT_U32                                     prefixLen;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  nextHopInfo;
    GT_BOOL                                    override;
    GT_STATUS                                  returnStatus;
} PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STCT
 *
 * Description: IPv6 UC Prefix information for bulk operations
 *
 * Fields:
 *      supportIpv4Uc           - whether this VR supports UC Ipv4.
 *      defaultUcIpv4RouteEntry - the route entry info associated to the default
 *                                Ipv4 UC route entry.
 *      supportIpv4Mc           - whether this VR support MC Ipv4.
 *      defaultMcIpv4RouteEntry - the route entry info associated to the default
 *                                Ipv4 MC route entry.
 *      supportIpv6Uc           - whether this VR supports UC Ipv6.
 *      defaultUcIpv6RouteEntry - the route entry info associated to the default
 *                                Ipv6 UC route entry.
 *      supportIpv6Mc           - whether this VR supports MC Ipv6
 *      defaultMcIpv6RouteEntry - the route entry info associated to the default
 *                                Ipv6 MC route entry.
 *      supportFcoe             - whether this VR supports FCoE
 *      defaultFcoeForwardingEntry - the forwarding entry info associated to the
 *                                default FCoE forwarding entry.
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STCT
{
    GT_BOOL                                    supportUcIpv4;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  defaultUcIpv4RouteEntry;
    GT_BOOL                                    supportMcIpv4;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  defaultMcIpv4RouteEntry;
    GT_BOOL                                    supportUcIpv6;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  defaultUcIpv6RouteEntry;
    GT_BOOL                                    supportMcIpv6;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  defaultMcIpv6RouteEntry;
    GT_BOOL                                    supportFcoe;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  defaultFcoeForwardingEntry;
} PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STCT
 *
 * Description: LPM shadow shared device list
 *
 * Fields:
 *
 *      shareDevNum     - device numbers of devices sharing this shadow.
 *      shareDevs       - Device numbers of PPs sharing the shadow.
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STCT
{
    GT_U32   shareDevNum;
    GT_U8    *shareDevs;
} PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STCT
 *
 * Description:
 *      This structure holds a list entry of memory block to be freed after
 *      an operation on the UC/MC search structures.
 *
 * Fields:
 *      memAddr     - Pp Memory address to be freed.
 *      next        - the next entry in this free link list
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STCT
{
    GT_UINTPTR                                         memAddr;
    struct PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STCT   *next;
} PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC
 *
 * Description: Virtual Router information
 *
 * Fields:
 *
 *      valid                - is this vrId valid
 *      needsHwUpdate        - whether this vrId needs HW update
 *      isUnicastSupported   - whether unicast is supported in this VR
 *      isMulticastSupported - whether multicast is supported in this VR
 *      rootBucketType       - the root bucket type
 *      rootBucket           - the root bucket
 *      multicastDefault     - the default nexthop for (G,*), where G is 224/4
 *                             for IPv4 or 255/8 for IPv6
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STCT
{
    GT_BOOL         valid;
    GT_BOOL         needsHwUpdate;
    GT_BOOL         isUnicastSupported[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS];
    GT_BOOL         isMulticastSupported[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS];
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT
                    rootBucketType[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC
                    *rootBucket[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS];
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC
                    *multicastDefault[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS];
} PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT
 *
 * @brief Defines the method of LPM blocks allocation
*/
typedef enum{

    /** @brief the blocks are allocated dynamically, memory blocks are never shared
     *  among different octets of the same protocol
     */
    PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E  = 0,

    /** @brief the blocks are allocated dynamically, memory blocks are shared among
     *  different octets of the same protocol in case of missing free block.
     *  This can cause violation of full wire-speed.
     */
    PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E     = 1

} PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT
 *
 * @brief Defines the mode of LPM MEM module
*/
typedef enum{

    /** @brief each RAM pool will be fully utulized
     *  This can cause violation of full wire-speed.
     */
    PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,

    /** @brief each RAM pool will be half utulized in order to support full wirespeed.
     *  The second half of each RAM pool will be identical to first.
     */
    PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E

} PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT;


/**
* @enum PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT
 *
 * @brief shared tables configuration modes for following clients:
 * L3 (LPM), L2 (FDB), EM (Exact Match)
 * (APPLICABLE DEVICES : FALCON)
*/
typedef enum{

    /** @brief This mode :
     * 1. Maximum L3 table size (LPM)
     * 2. Minimum L2 table size (FDB)
     * 3. No use of EM (Exact Match)
     */
    PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E,
    /** @brief This mode :
     * 1. Minimum L3 table size (LPM)
     * 2. Maximum L2 table size (FDB)
     * 3. No use of EM (Exact Match)
     */
    PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E,
    /** @brief This mode :
     * 1. Medium L3 table size (LPM)
     * 2. Medium L2 table size (FDB)
     * 3. Minimum EM table size Exact Match)
     */
    PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E,
    /** @brief This mode :
     * 1. Medium L3 table size (LPM)
     * 2. Minimum L2 table size (FDB)
     * 3. Maximum EM table size (Exact Match)
     */
    PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E

} PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT;

/**
 * @enum PRV_CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_ENT
 *
 * @brief this enum is used in the process of (*,G) lookup.
 *        When an entry has this bit set, the (*, G) lookup
 *        terminates with a match at the current entry,
 *        and (S,* G) SIP based lookup is triggered.
 *        0x0 = Leaf
 *        0x1 = Trigger IP MC S+G Lookup
*/
typedef enum{

    PRV_CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_LEAF_E    = 0,
    PRV_CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_TRIGGER_E = 1

} PRV_CPSS_DXCH_LPM_LEAF_ENTRY_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_LEAF_TYPE_ENT
 *
 * @brief Defines the leaf type
 *        0x0 = Regular Leaf
 *        0x1 = Multipath Leaf
*/
typedef enum{

    PRV_CPSS_DXCH_LPM_LEAF_REGULAR_TYPE_ENT     = 0,
    PRV_CPSS_DXCH_LPM_LEAF_MULTIPATH_TYPE_E     = 1

} PRV_CPSS_DXCH_LPM_LEAF_TYPE_ENT;


/**
* @enum PRV_CPSS_DXCH_LPM_LEAF_NEXT_NODE_TYPE_ENT
 *
 * @brief Defines the next node entry type
 *        0x0 = Regular
 *        0x1 = Compressed
*/
typedef enum{

    PRV_CPSS_DXCH_LPM_LEAF_NEXT_NODE_REGULAR_TYPE_E    = 0,
    PRV_CPSS_DXCH_LPM_LEAF_NEXT_NODE_COMPRESSED_TYPE_E = 1

} PRV_CPSS_DXCH_LPM_LEAF_NEXT_NODE_TYPE_ENT;


/**
* @enum PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT
 *
 * @brief each range is associated with the child node type.
 *        0x0 = Empty; The current bit in the bit-vector
 *              does not open a new range
 *        0x1 = Leaf; The current bit in the bit-vector opens
 *               a new range which is associated with a leaf entry
 *        0x2 = Regular; The current bit in the bit-vector opens
 *              a new range which is associated with a regular node
 *        0x3 = Compressed; current bit in the bit-vector opens
 *              a new range which is associated with a compressed node
*/
typedef enum{

    PRV_CPSS_DXCH_LPM_CHILD_EMPTY_TYPE_E    = 0,
    PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E     = 1,
    PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E  = 2,
    PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E   = 3
} PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT;


/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_CONFIG_STCT
 *
 * Description: Memory configurations for RAM based LPM shadow
 *
 * Fields:
 *      numOfBlocks             - the number of RAM blocks that LPM uses.
 *                                (APPLICABLE VALUES : SIP5 devices: 1..20,
 *                                                     SIP6 devices: 1..30) 
 *      blocksSizeArray         - array that holds the sizes of the RAM blocks in lines
 *      blocksAllocationMethod  - the method of blocks allocation
 *      bigBanksNumber          - number of big banks (7K or 14K)
 *      bigBankSize             - big banks size (7K or 14K)
 *      numberOfBigPbrBanks     - number of big banks allocated for PBR
 *
 * Comments:
 *      None
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_CONFIG_STCT
{
    GT_U32                                              numOfBlocks;
    GT_U32                                              blocksSizeArray[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  blocksAllocationMethod;
    PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT                  lpmMemMode;
    GT_U32                                              bigBanksNumber;
    GT_U32                                              bigBankSize;
    GT_U32                                              numberOfBigPbrBanks;
} PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STCT
 *
 * Description: Octets to Block Mapping information.
 *
 * Fields:
 *      isBlockUsed                 - specify if the block is in use by any protocol ,
 *                                  meaning octets reside in the specific block
 *                                  GT_TRUE: block is used
 *                                  GT_FALSE: block is not used
 *
 *      octetsToBlockMappingBitmap - for each protocol we hold a bitmap that specify
 *                                   what octets are using this block.
 *                                   This bitmap is common for UC and MC,
 *                                   for example ipv4 UC/MC_Src and MC_Group will use
 *                                   bits 0-3 (octet0, octet1, octet2, octet3) to
 *                                   specify they are using this block.
 *                                   if the bitmap per protocol is equal to 0 then
 *                                   the protocol is not using the block.
 *
 * Comments:
 *      None
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STCT
{
    GT_BOOL                                             isBlockUsed;
    GT_U32                                              octetsToBlockMappingBitmap[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E];
} PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC;


/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC
 *
 * Description: pending information to be used for update blocks usage.
 *              The pending information will be used after data collection
 *              for all blocks memory allocations/de-allocation is done.
 *              Once collection was done the values in numOfIncUpdates and
 *              numOfDecUpdates will be used to update the counters per protocol.
 *              Those counters are used for managing block memory usage.
 *
 * Fields:
 *      updateInc   - used to decide if an incremental update of the counters usage is needed
 *                   GT_TRUE: need to increment block counters usage
 *                   GT_FALSE: no update is needed
 *      updateDec   - used to decide if an decrement update of the counters usage is needed
 *                   GT_TRUE: need to decrement block counters usage
 *                   GT_FALSE: no update is needed
 *      numOfIncUpdates - number of increments needed for the counters during insert/delete operation
 *      numOfIncUpdates - number of decrements needed for the counters during insert/delete operation
 *
 * Comments:
 *      None
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STCT
{
    GT_BOOL                                             updateInc;
    GT_BOOL                                             updateDec;
    GT_U32                                              numOfIncUpdates;
    GT_U32                                              numOfDecUpdates;
} PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STCT
 *
 * Description: Struct that specify for each protocol how many lines allocations
 *              or de-allocations were done for a specific block
 *
 * Fields:
 *      sumOfIpv4Counters - sum of all allocations done, if 0 then no allocation
 *                         was done for IPv4 for this block
 *      sumOfIpv6Counters - sum of all allocations done, if 0 the no allocation
 *                         was done for IPv6 for this block
 *      sumOfFcoeCounters - sum of all allocations done, if 0 then no allocation
 *                         was done for FCoE for this block
 * Comments:
 *      None
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STCT
{
    GT_U32   sumOfIpv4Counters;
    GT_U32   sumOfIpv6Counters;
    GT_U32   sumOfFcoeCounters;

} PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STC;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FROM_FIRST_SWAP_AREA_ENT
 *
 * @brief The direction of the copy operation, TO or FROM the swap area
*/
typedef enum{

    /** copy HW bucket from its bank location to the first swap area */
    PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FIRST_SWAP_AREA_E = 0,

    /** copy HW bucket from the first swap area to its new bank location */
    PRV_CPSS_DXCH_LPM_RAM_COPY_FROM_FIRST_SWAP_AREA_E

} PRV_CPSS_DXCH_LPM_RAM_COPY_TO_FROM_FIRST_SWAP_AREA_ENT;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE_ENT
 *
 * @brief HW and Pointers update phase
*/
typedef enum{

    /** Build data according to bucket shadow and write it to HW */
    PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE1_E = 0,

    /** @brief Update all HW and Shadow pointers according to the data
     *  written in phase 1
     */
    PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE2_E

} PRV_CPSS_DXCH_LPM_RAM_HW_AND_POINTERS_UPDATE_PHASE_ENT;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STCT
 *
 * Description: Memory configurations for RAM based LPM shadow
 *
 * Fields:
 *      neededMemoryBlocks      - array for memory handlers needed for octets allocation.
 *      neededMemoryBlocksSizes - array for needed memory sizes in lpm lines
 *      regularNode             - GT_TRUE:  the node is bit vector. It pointed for 6 group of node.
 *                                          So whole array of neededMemoryBlocks and neededMemoryBlocksSizes is used.
 *                              - GT_FALSE: the node is compressed type. So only fist element of arrays us used.
 *      octetId                 - octet number pointed to resized group of nodes
 *
 * Comments:
 *      None
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STCT
{
    GT_UINTPTR                                   neededMemoryBlocks[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
    GT_U32                                       neededMemoryBlocksSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
    GT_BOOL                                      regularNode;
    GT_U32                                       octetId;
} PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_SHADOW_STCT
 *
 * Description: LPM Shadow.
 *
 * Fields:
 *
 *      shadowType      - type of shadow
 *      lpmMemInfoArray - the memory info
 *      numOfLpmMemories- the number of memories used by LPM engines.
 *      bigBanksNumber  - number of LPM memories with banks size 7K or 14K (applicable for sip6 family) 
 *      smallBanksNumber - number of LPM memoris with small bank size :640 lines (applicable for sip6 family)
 *      bigBankSize      - big bank size (7K or 14K)(applicable for sip6 family)
 *      memoryOffsetValue - real memory start index on address lane (applicable for sip6 family)
 *      lpmRamBlocksSizeArrayPtr          - (pointer to) array that holds the number of entries in each RAM blocks in lines
 *      lpmRamTotalBlocksSizeIncludingGap - number of 'entries' between each blocks in lines
 *                                          (this size should be equal or biger then lpmRamBlocksSize)
 *      lpmRamBlocksAllocationMethod  - the method of blocks allocation
 *      lpmRamStructsMemPoolPtr       - (pointer to) array that holds the memPoolId for the allocated memory blocks in SW
 *      lpmRamOctetsToBlockMappingPtr - (pointer to) Holds the mapping of octets+protocol to block
 *                                      What are the octets binded to each block (per protocol)
 *      tempLpmRamOctetsToBlockMappingUsedForReconstractPtr - mapping used in case reconstruct is needed.
 *                                       this is the mapping after unbinding blocks that are not used anymore
 *      mcSearchMemArrayPtr - the MC search memory information for each
 *                        of the LPM levels. it combined 4 grp + 4 src (and one
 *                        extra for safety)
 *      ucSearchMemArrayPtr - the UC search memory information for each
 *                        of the LPM levels per protocol stack.
 *      swapMemoryAddr  - the address of the swap area used for delete and add prefixes
 *      secondSwapMemoryAddr - - the address of the second swap area, used for defrag
 *      vrRootBucketArray - The VR root bucket information
 *      vrfTblSize      - VRF table size (number of elements in vrRootBucketArray)
 *      workDevListPtr  - pointer to the device list to preform the actions on.
 *      shadowDevList   - the device list of the devices sharing this shadow.
 *      freeMemListEndOfUpdate - Free memory blocks at the end of update
 *      freeMemListDuringUpdate - Free memory blocks during update
 *      neededMemoryBlocksInfo - used for SIP6 memory needs
 *      neededMemoryBlocks - used for memory needs
 *      neededMemoryBlocksSizes - sizes of neededMemoryBlocks
 *      neededMemoryBlocksSwapUsedForAdd - specify if neededMemoryBlocks is a swap area memory or not
 *      neededMemoryListLen - number of elements in neededMemoryBlocks
 *      neededMemoryCurIdx - current index when running on neededMemoryBlocks
 *      isProtocolInitialized - is the protocol initialized
 *      allNewNextMemInfoAllocatedPerOctetArrayPtr - (pointer to) an array that holds for each octet
 *                                                   the allocated new element that need to be freed.
 *                                                   Size of the array is 16 for case of IPV6.
 *                                                   We use this array to hold the new allocated RAM space.
 *                                                   in case of an error we free all bound memory.
 *                                                   If an allocation of a new prefix succeed then we
 *                                                   reset this array for next prefix addition.
 *      pendingBlockToUpdateArr         - pending information to be used for update blocks usage.
 *                                        The pending information will be used after we finish collecting
 *                                        data regarding all the blocks that had memory allocations or
 *                                        de-allocation from them.
 *      protocolCountersPerBlockArr     - Once we are done collection data in the pendingBlockToUpdateArr we
 *                                        will use the value in numOfUpdates to update the counters per protocol.
 *                                        Those counters are used for managing block memory usage.
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_SHADOW_STCT
{
    PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT            shadowType;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E][PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_U32                                       numOfLpmMemories;
    GT_U32                                       bigBanksNumber;
    GT_U32                                       smallBanksNumber;
    GT_U32                                       bigBankSize;
    GT_U32                                       memoryOffsetValue;
    GT_U32                                              *lpmRamBlocksSizeArrayPtr;
    GT_U32                                              lpmRamTotalBlocksSizeIncludingGap;
    PRV_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  lpmRamBlocksAllocationMethod;
    GT_UINTPTR                                          *lpmRamStructsMemPoolPtr;
    PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC   *lpmRamOctetsToBlockMappingPtr;
    PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC   tempLpmRamOctetsToBlockMappingUsedForReconstractPtr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC*          mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS][PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_MC_PROTOCOL_CNS];
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC*          ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS][PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS];
    GT_UINTPTR                                   swapMemoryAddr;
    GT_UINTPTR                                   secondSwapMemoryAddr;
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC        *vrRootBucketArray;
    GT_U32                                       vrfTblSize;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC   *workDevListPtr;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC   shadowDevList;
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC     *freeMemListEndOfUpdate;
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC     *freeMemListDuringUpdate;
    PRV_CPSS_DXCH_LPM_RAM_NEEDED_MEM_DATA_STC    neededMemoryBlocksInfo[1024];
    GT_UINTPTR                                   neededMemoryBlocks[1024];
    GT_U32                                       neededMemoryBlocksSizes[1024];
    GT_BOOL                                      neededMemoryBlocksSwapUsedForAdd[1024];
    GT_U32                                       neededMemoryListLen;
    GT_U32                                       neededMemoryCurIdx;
    GT_BOOL                                      isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E];
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           *allNewNextMemInfoAllocatedPerOctetArrayPtr[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS];
    PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT           lpmMemMode;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   pendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STC         protocolCountersPerBlockArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
} PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC;

/*
 * Typedef: enum PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT
 *
 * Description: Defines the way a bucket is treated in the lpm insertion/ deletion
 *              process.
 *
 * Fields:
 *      PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITH_RAM_UPDATE_E - the bucket is a root
 *              bucket that needs ram update.
 *      PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_RAM_UPDATE_E - the bucket is a root
 *              bucket that needs no ram update.
 *      PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E - the bucket is root
 *              bucket that cannot use compressed 2 style (needs always a ram
 *              update).
 *      PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E -   the bucket is a not a root bucket.
 */
typedef enum PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENTT
{
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITH_RAM_UPDATE_E,
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_RAM_UPDATE_E,
    PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_WITHOUT_COMPRESSED_2_E,
    PRV_CPSS_DXCH_LPM_RAM_NOT_A_ROOT_BUCKET_E
}PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_FLAG_ENT;

/* Typedef: struct IP_MEM_ALLOC_INFO_STC
 *
 * Description: A structure that holds bucket memory allocation information
 *
 * Fields:
 *      memAddr            - the address of the memory allocated.
 *      memSize            - the size of the memory allocated.
 *      bucketHwUpdateStat - bucket hw status
 */
typedef struct IP_MEM_ALLOC_INFO_STCT
{
    GT_U32        memAddr;
    GT_U32        memSize;
    GT_U32        bucketHwUpdateStat;
} PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC
 *
 * Description: virtual router HSU/Hotsyc info
 *
 * Fields:
 *   vrId                - virtual router id
 *   vrIpUcSupport       - Boolean array stating UC support for every protocol stack
 *   vrIpMcSupport       - Boolean array stating MC support for every protocol stack
 *   defUcRouteEntrey    - the array shows default UC next hop ipv4/v6 information for
 *                         this virtual
 *   defMcRouteEntrey    - the array shows default MC ipv4/v6 route for this
 *                         virtual router.
 *   protocolStack       - types of IP stack used in this virtual router.
 *   defIpv6McRuleIndex  - in the case a Ipv6 VR is initialized, this is the
 *                         Rule index for the ipv6 Default Mc Group.
 *   vrIpv6McPclId       - in the case a Ipv6 VR is initialized, this is the
 *                         Pcl id for the ipv6 Mc Group entries.
 *   isLast              - is this the last
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STCT
{
    GT_U32                                  vrId;
    GT_BOOL     vrIpUcSupport[CPSS_IP_PROTOCOL_IPV4V6_E];
    GT_BOOL     vrIpMcSupport[CPSS_IP_PROTOCOL_IPV4V6_E];
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  defUcRouteEntrey[CPSS_IP_PROTOCOL_IPV4V6_E];
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  defMcRouteEntrey[CPSS_IP_PROTOCOL_IPV4V6_E];
    CPSS_IP_PROTOCOL_STACK_ENT              protocolStack;
    GT_U32                                  defIpv6McRuleIndex;
    GT_U32                                  vrIpv6McPclId;
    GT_BOOL                                 isLast;
} PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC;

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_OP_ENT
 *
 * @brief Defines the different operation that may be done be the
 * memory block list
*/
typedef enum{

    /** Reset the memory list. */
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E = 0,

    /** @brief Add a memory block to the
     *  to be freed memory list.
     */
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,

    /** @brief Free the memory form the
     *  collected memory list.
     *  Note:
     */
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E

} PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_OP_ENT;

/*
 * Typedef: enum PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_ENT
 *
 * Description: A next LPM Trie
 *
 * Fields:
 *      PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E - A next LPM Trie (used in
 *                                              MC src LPM structure)
 */
enum PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_ENT
{
    PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E = 255
};

/*
 * Typedef: struct PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC
 *
 * Description: LPM DB data
 *
 * Fields:
 *
 *      shadowArray           - the LPM DB array of shadow it holds
 *      numOfShadowCfg        - number of shadows in the shadowArray
 *      protocolBitmap        - the protocols this LPM DB supports
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STCT
{
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC   *shadowArray;
    GT_U32                             numOfShadowCfg;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap;
} PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC;




/**
 * @struct PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC
 *
 * @brief LPM leaf entry used for policy based routing
 *
 * @brief Fields:
 *      entryType                - Route entry type
 *                                 APPLICABLE VALUES:
 *                                  CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E
 *                                  CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E-(NOT APPLICABLE DEVICES:Falcon)
 *                                  CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E-(NOT APPLICABLE DEVICES:Falcon)
 *                                  CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E-(APPLICABLE DEVICES:Falcon)
 *  
 *      index                    - If entryType is
 *                                 CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E then
 *                                 this is the pointer to the route entry
 *                                 (APPLICABLE RANGES: 0..24575)
 *                                 Otherwise it is the pointer to ECMP/QoS entry
 *                                 (APPLICABLE RANGES: 0..12287)
 *  
 *      ucRPFCheckEnable         - Enable unicast RPF check for this entry
 *  
 *      sipSaCheckMismatchEnable - Enable unicast SIP MAC SA match check
 *  
 *      ipv6MCGroupScopeLevel    - the IPv6 Multicast group scope level
 *  
 *      priority                 - resolution priority between PBR/ExactMatch and FDB match results
 *                                 (APPLICABLE DEVICES: Falcon)
 *      pointToSip               - if points to SIP tree
 *      pointerToSip             - SIP tree offset in LPM memory
 *      nextNodeType             - SIP tree node type
 *
 * @brief Comments:
 *      index can point to either NH or ECMP entry. entryType determines the
 *      type of pointer: if entryType is CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E
 *      then it points to NH, else - to ECMP entry.
 */
typedef struct PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STCT
{
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT         entryType;
    GT_U32                                      index;
    GT_BOOL                                     ucRPFCheckEnable;
    GT_BOOL                                     sipSaCheckMismatchEnable;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  ipv6MCGroupScopeLevel;
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT  priority;
    GT_BOOL                                     pointToSip;
    GT_U32                                      pointerToSip;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT         nextNodeType;
} PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC;

/**
 * @struct struct PRV_CPSS_DXCH_LPM_COMPRESSED_STCT
 *
 * @brief LPM compressed node representation.
 *              (Applicable for SIP6 LPM)
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_COMPRESSED_STCT
{
    /** @brief - array of ranges that compressed node addresses.
     *           compressed node spreads up to 10 ranges with 9
     *           offset fields
    */
    GT_U8   ranges1_9[MAX_NUMBER_OF_COMPRESSED_RANGES_CNS];
    /** @brief - child nodes types associated with each range
    */
    GT_U8   childNodeTypes0_9[MAX_NUMBER_OF_COMPRESSED_RANGES_CNS];
    /** @brief -  lpm offset in lines
    */
    GT_UINTPTR  lpmOffset;
    /** @brief -
    */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT compressedType;
    /** @brief -
    */
    GT_U8  numberOfLeaves;
    /** @brief -
    */
    PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC embLeavesArray[PRV_CPSS_DXCH_LPM_MAX_LEAVES_IN_EMBEDDED_3_NODE_E];
} PRV_CPSS_DXCH_LPM_COMPRESSED_STC;

/**
 * @struct struct PRV_CPSS_DXCH_LPM_REGULAR_STC
 *
 * @brief LPM regular node representation.
 *              (Applicable for SIP6 LPM)
 *
 * Comments:
 */
typedef struct PRV_CPSS_DXCH_LPM_REGULAR_STCT
{
    /** @brief - child nodes types associated with each range.
    *          lpmOffsets
    */
    GT_U8       childNodeTypes0_255[PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS];

    /** @brief - lpm offsets (in lpm lines) array for regular node
    */
    GT_UINTPTR  lpmOffsets[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
} PRV_CPSS_DXCH_LPM_REGULAR_STC;


/**
 * @struct  PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STCT
 *
 *  @brief Representation of the lpm node range selection
 *      section. The section can be from one of the 2 following
 *      types:
 *      compressed   - the range selection section contains 10
 *      ranges
 *      regular - the range selection section contains till 256
 *      ranges
 */
typedef struct PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STCT
{
    /** @brief - representation of all regular nodes in current
       *  group of nodes
       */
    PRV_CPSS_DXCH_LPM_REGULAR_STC regularNodesArray[NUMBER_OF_RANGES_IN_SUBNODE];
     /** @brief - representation of all compressed nodes in current
      *  group of nodes
       */
    PRV_CPSS_DXCH_LPM_COMPRESSED_STC compressedNodesArray[NUMBER_OF_RANGES_IN_SUBNODE];
     /** @brief - representation of all leafArray nodes in current
       * group of nodes
       */
    PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC leafNodesArray[NUMBER_OF_RANGES_IN_SUBNODE];
} PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmRamTypesh */


