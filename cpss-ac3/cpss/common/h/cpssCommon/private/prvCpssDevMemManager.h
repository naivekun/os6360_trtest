/*******************************************************************************
*         Copyright 2003, MARVELL SEMICONDUCTOR ISRAEL, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL SEMICONDUCTOR ISRAEL. (MSIL),  MARVELL TAIWAN, LTD. AND      *
* SYSKONNECT GMBH.                                                             *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDevMemManager.h
*
* @brief Device memory manager.
*
* @version   10
********************************************************************************
*/
#ifndef __prvCpssDevMemManager
#define __prvCpssDevMemManager

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

#define DMM_BLOCK_FREE                      0
#define DMM_BLOCK_ALLOCATED                 1
#define DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS   4
#define DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS   16

/* Illegal values constant to distinguish between an error
   due to CPU memory allocation or DMM block allocation */
#define DMM_MALLOC_FAIL                     0
extern GT_UINTPTR dmmBlockNotFoundIndication;
#define DMM_BLOCK_NOT_FOUND                dmmBlockNotFoundIndication

#define SIZE_IN_WORDS(handler)      (((GT_DMM_BLOCK*)handler)->word1 & 0x1FFFFFF)
#define SET_SIZE_IN_WORDS(handler, size)    \
            if((size) > 0x1FFFFFF)            \
            {                               \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);     \
            }                               \
            else                            \
            {                               \
                handler->word1 = (handler->word1 & 0xFE000000) | (size);     \
            }

#define DMM_BLOCK_STATUS(handler)   ((((GT_DMM_BLOCK*)handler)->word1>>25) & 0x1)
/* following two macro's instead of old one implemented to avoid warning
  "statement not reached" in DIAB WB 2.6
*/
#define SET_DMM_BLOCK_STATUS_FREE(handler)  \
    ((GT_DMM_BLOCK*)handler)->word1 &= ~(1<<25)

#define SET_DMM_BLOCK_STATUS_ALLOCATED(handler) \
    ((GT_DMM_BLOCK*)handler)->word1 |= (1<<25)

#define OFFSET_IN_WORDS(handler)    (((GT_DMM_BLOCK*)handler)->word2 & 0xFFFFFF)
#define SET_OFFSET_IN_WORDS(handler, offset)    \
            if((offset) > 0xFFFFFF)           \
            {                               \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);     \
            }                               \
            else                            \
            {                               \
                handler->word2 = (handler->word2 & 0xFF000000) | (offset);   \
            }

#define DMM_BLOCK_STAMP(handler)    ((((GT_DMM_BLOCK*)handler)->word2>>24) & 0xFF)
/* validity check removed to avoid warning "statement not reached" in DIAB WB 2.6 and
 * because it's internal macro which is used with valid internal pre-defined constants only
 */
#define SET_DMM_BLOCK_STAMP(handler, stamp) \
    handler->word2 = (handler->word2 & 0xFFFFFF) | ((stamp) << 24)

#define DMM_GET_SIZE(handler)  (SIZE_IN_WORDS(handler) << 2)
#define DMM_GET_OFFSET(handler)  (OFFSET_IN_WORDS(handler) << 2)
#define DMM_GET_PARTITION(handler)  ((GT_UINTPTR)(((GT_DMM_BLOCK*)handler)->nextBySizeOrPartitionPtr.partitionPtr))


#define GET_DMM_BUCKET_SW_ADDRESS(handler)      (((GT_DMM_BLOCK*)handler)->word3)
#define SET_DMM_BUCKET_SW_ADDRESS(handler, address)                                \
            {                                                                      \
                ((GT_DMM_BLOCK*)handler)->word3 = ((GT_UINTPTR)address);           \
            }
#define RESET_BUCKET_SW_ADDRESS(handler)  \
    ((GT_DMM_BLOCK*)handler)->word3 = 0

/* Protocol: ipv4=0, ipv6=1, FCoE=2 */
#define GET_DMM_BLOCK_PROTOCOL(handler)     (((GT_DMM_BLOCK*)handler)->word4 & 0x3)
#define SET_DMM_BLOCK_PROTOCOL(handler, protocol)                                   \
        if((protocol) > 0x2)                                                        \
        {                                                                           \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);       \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            ((GT_DMM_BLOCK*)handler)->word4 =                                       \
            (((GT_DMM_BLOCK*)handler)->word4 & 0xFFFFFFFC) | (protocol);            \
        }
#define RESET_BLOCK_PROTOCOL(handler)                                               \
    ((GT_DMM_BLOCK*)handler)->word4 =                                               \
            (((GT_DMM_BLOCK*)handler)->word4 & 0xFFFFFFFC)      ;                   \

/*Note : The number must be multiple of 32*/
#define DMM_MAXIMAL_BLOCK_SIZE_ALLOWED  2112
/*This is DMM_MAXIMAL_BLOCK_SIZE_ALLOWED / 32*/
#define DMM_EMPTINESS_ARRAY_SIZE        ((DMM_MAXIMAL_BLOCK_SIZE_ALLOWED/32)+1)   /*33*/

#define DMM_STAMP_FREE                  0x59
#define DMM_STAMP_USED                  0xC8
#define DMM_STAMP_USED_STATIC           0xC9

struct _gtDmmPartition;
/*
 * Typedef: GT_DMM_BLOCK
 *
 * Description: Device memory block handler.
 *
 * Fields:
 *          partitionPtr - the partiotion pointer that this DMM block was taken
 *                         from.
 *          nextBySize, prevBySize- Pointers to double linked list of all free
 *                                  memory blocks in the same size.
 *          nextByAddr, prevByAdd - Pointers to sorted by address double
 *                                  linked list of all memory blocks.
 *  Following fields packed into fields of 32bit length to save in
 *  memory and to not mess with bit-fields:
 *          word1:
 *              - sizeInWords     - Size of the block in words.
 *              - status          - Status of the block, free or allocated.
 *          word2:
 *              - stamp           - used for check handler validity,
 *                              different for free and allocated blocks
 *              - offsetInWords   - Address of the memory handled
 *          word3:
 *              - bucketSwAddress - address of the SW bucket pointing
 *                                  to this block memory
 *          word4:
 *              - protocolValue   - ipv4=0 ipv6=1, FCoE=2:
                                    what is the prefix LPM protocol held by this bucket
 *
 */
typedef struct dmmBlock
{
    union
    {
        struct _gtDmmPartition *partitionPtr;
        struct  dmmBlock *nextBySize;
    }nextBySizeOrPartitionPtr;
    struct  dmmBlock *prevBySize;
    struct  dmmBlock *nextByAddr, *prevByAddr;
    GT_U32      word1; /* sizeInWords:25(bits 0-24), status:1(bit #25)      */
    GT_U32      word2; /* offsetInWords:24(bits 0-23), stamp:8(bits 24-31)  */
    GT_UINTPTR  word3; /* bucketSwAddress:32(bits 0-31)                     */
    GT_U32      word4; /* protocolValue:2(bits 0-1)                         */
}GT_DMM_BLOCK;

/*
 * Typedef: _gtDmmSortedPartitionElement
 *
 * Description: Device memory block pointer,
 *              used as an element in the sorted partition.
 *
 * Fields:
 *          blockElementPtr - (pointer to) block in the partition
 *                            always point on a single block in the partion
 *          nextSortedElemPtr - (pointer to) the next element in the list
 *
 */
typedef struct _gtDmmSortedPartitionElement
{
     GT_DMM_BLOCK                            *blockElementPtr;
     struct _gtDmmSortedPartitionElement     *nextSortedElemPtr;

}GT_DMM_SORTED_PARTITION_ELEMENT;

/*
 * Typedef: GT_DMM_PARTITION
 *
 * Description: Device memory block handler.
 *
 * Fields:
 *          tableOfSizePointers - Array of pointers to linked lists of all free
 *                                  memory blocks in the same size.
 *          bigBlocksList       - Pointer to linked lists of all free
 *                                  memory blocks in size bigger that.
 *          emptinessBitArray   - bit array of DMM_MAXIMAL_BLOCK_SIZE_ALLOWED
 *                                  bits, used for fast search of non empty
 *                                  indexes in tableOfSizePointers.
 *          minBlockSize      - Minimal size can be allocated from the partition
 *          maxBlockSize      - Maximal size can be allocated from the partition
 *          pointedBlock      - block in the partition - for statistics only.
 *                              always point on a single block in the partion
 *                              needed for statistics when no free block in the
 *                              partition, the tableOfSizePointers and the
 *                              bigBlocksList points to no blocks.
 *          pointedFirstFreeBlock -  pointer to the first free block in the partition
 *          pointedFirstBlock - pointer to the first block in the partition (free or used)
 *                              Used for defragmentation.
 *          tableOfSortedUsedBlocksPointers - Array of pointers to linked lists of
 *                              all used memory blocks in the same size.
 *                              Used for defragmentation.
 *
 */
typedef struct _gtDmmPartition
{
    GT_DMM_BLOCK    *tableOfSizePointers[DMM_MAXIMAL_BLOCK_SIZE_ALLOWED+1];
    GT_DMM_BLOCK    *bigBlocksList;
    GT_U32          emptinessBitArray[DMM_EMPTINESS_ARRAY_SIZE];
    GT_U32          minBlockSizeInWords;
    GT_U32          maxBlockSizeInWords;
    GT_U32          pickAllocatedBytes;
    GT_U32          allocatedBytes;
    GT_U32          pickBlocks;
    GT_U32          currentBlocks;
    GT_DMM_BLOCK    *pointedBlock;
    GT_DMM_BLOCK    *pointedFirstFreeBlock;
    GT_DMM_BLOCK    *pointedFirstBlock;
    GT_DMM_SORTED_PARTITION_ELEMENT  *tableOfSortedUsedBlocksPointers[DMM_MAXIMAL_BLOCK_SIZE_ALLOWED+1];
}GT_DMM_PARTITION;

/**
* @internal prvCpssDmmCreatePartition function
* @endinternal
*
* @brief   Create new device memory partition
*
* @param[in] sizeOfPartition          - Ammount of memory added.
* @param[in] startAddress             - the offset that memory block will be added with.
* @param[in] minBlockSize             - Minimal block size can be allocated from this partition.
* @param[in] maxBlockSize             - Maximal block size can be allocated from this partition.
*
* @param[out] partitionId              - ID of the created partition.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid parameters
* @retval GT_NO_RESOURCE           - No memory is available to create the partition.
*/
GT_STATUS prvCpssDmmCreatePartition
(
    IN GT_U32   sizeOfPartition,
    IN GT_U32   startAddress,
    IN GT_U32   minBlockSize,
    IN GT_U32   maxBlockSize,
    OUT GT_UINTPTR  *partitionId
);

/**
* @internal prvCpssDmmAllocate function
* @endinternal
*
* @brief   Allocate new device memory block to existing partition.
*
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
* @param[in] size                     - Ammount of memory requested
* @param[in] alignment                - Alignment of memory requested
*                                      ( Only word alingment supported )
*
* @retval handler                  - handler/id of the allocated adress in device memory.
*                                       Offset and size of the block can be accessed by macros :
*                                       DMM_GET_SIZE(handler)
*                                       DMM_GET_OFFSET(handler)
*
* @note If handler == 0, the operation failed.
*
*/
GT_UINTPTR prvCpssDmmAllocate
(
    IN GT_UINTPTR   partitionId,
    IN GT_U32       size,
    IN GT_U32       alignment
);

/**
* @internal prvCpssDmmStaticAllocate function
* @endinternal
*
* @brief   Allocate new device static (not to be freed) memory block to existing
*         partition.
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
* @param[in] size                     - Ammount of memory requested
* @param[in] alignment                - Alignment of memory requested
*
* @retval handler                  - handler/id of the allocated adress in device memory.
*                                       Offset and size of the block can be accessed by macros :
*                                       DMM_GET_SIZE(handler)
*                                       DMM_GET_OFFSET(handler)
*
* @note If handler == 0, the operation failed.
*
*/
GT_UINTPTR prvCpssDmmStaticAllocate
(
    IN GT_UINTPTR   partitionId,
    IN GT_U32       size,
    IN GT_U32       alignment
);

/**
* @internal prvCpssDmmAllocateByPtr function
* @endinternal
*
* @brief   Allocate new device memory block to existing partition by an already
*         given HW ptr.
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
* @param[in] hwOffset                 - The given hw offset.
* @param[in] size                     - Ammount of memory requested
* @param[in] alignment                - Alignment of memory requested
*
* @param[out] hwOffsetHandlePtr        - handler of the allocated adress in device memory.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong blockSize
* @retval GT_OUT_OF_RANGE          - on value out of range
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
*
* @note Offset and size of the block can be accessed by macros :
*       DMM_GET_SIZE(handler)
*       DMM_GET_OFFSET(handler)
*
*/
GT_STATUS prvCpssDmmAllocateByPtr
(
    IN  GT_UINTPTR   partitionId,
    IN  GT_U32       hwOffset,
    IN  GT_U32       size,
    IN  GT_U32       alignment,
    OUT GT_UINTPTR   *hwOffsetHandlePtr
);

/**
* @internal prvCpssDmmFree function
* @endinternal
*
* @brief   Free device memory block to existing partition.
*
* @retval size                     - of the block freed if the status is GT_OK.
*
* @note If size == 0, the operation failed.
*
*/
GT_U32 prvCpssDmmFree
(
    IN GT_UINTPTR   handler
);

/**
* @internal prvCpssDmmClosePartition function
* @endinternal
*
* @brief   Close device memory partition
*
* @param[in] partitionId              - ID of the created partition.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid parameters
*/
GT_STATUS prvCpssDmmClosePartition
(
    IN GT_UINTPTR  partitionId
);


/**
* @internal prvCpssDmmStatistic function
* @endinternal
*
* @brief   Get the partition statistic
*
* @param[in] partitionId              - ID of the created partition.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail.
* @retval GT_NO_RESOURCE           - No memory is available to create the memory map or
*                                       allocated table.
*/
GT_STATUS prvCpssDmmStatistic
(
    IN GT_UINTPTR  partitionId,
    OUT GT_U32 *numOfUsedBlocks,
    OUT GT_U32 *numOfAllocatedBytes,
    OUT GT_U32 *numOfFreeBytes,
    OUT GT_U32 *numOfPickAllocatedBytes,
    OUT GT_U32 *numOfPickAllBlocks
);

/**
* @internal prvCpssDmmPartitionPrint function
* @endinternal
*
* @brief   Print partition info
*
* @param[in] partitionId              - ID of the created partition.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail.
* @retval GT_NO_RESOURCE           - No memory is available to create the memory map or
*                                       allocated table.
*/
GT_STATUS prvCpssDmmPartitionPrint
(
    IN GT_UINTPTR  partitionId
);

/**
* @internal prvCpssDmmCheckResizeAvailableWithSameMemory function
* @endinternal
*
* @brief   Check if resize is available using same memory block
*
* @param[in] handler                  - handler/id of the allocated by dmmAllocate() adress.
* @param[in] newBlockSize             - size (in lines) of the new block after resize
*
* @retval sizeOfOptionalUsedAndFreeBlock - size of the block created in case we merge the
*                                       used block and one of its nearest free block
*                                       (next block or previous block)
*
* @note If sizeOfOptionalUsedAndFreeBlock == 0, the operation failed.
*
*/
GT_U32 prvCpssDmmCheckResizeAvailableWithSameMemory
(
    IN  GT_UINTPTR   handler,
    IN  GT_U32       newBlockSize
);

/**
* @internal prvCpssDmmCheckIfShrinkOperationUsefulForDefrag function
* @endinternal
*
* @brief   Check if shrink operation may help for defrag
*
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
* @param[in] newBlockSize             - size (in lines) of the new block
* @param[in] oldBlockSize             - size (in lines) of the old block
* @param[in,out] useSwapAreaPtr           - (pointer to)
*                                      GT_TRUE  - swapArea will be used when doing shrink
*                                      GT_FALSE - swapArea will NOT be used when doing shrink
* @param[in,out] useSwapAreaPtr           - (pointer to)
*                                      GT_TRUE  - swapArea will be used when doing shrink
*                                      GT_FALSE - swapArea will NOT be used when doing shrink
*
* @retval sumOfFreeBlocksInPartition - size of the block created in case we merge the
*                                       free blocks using shrink
*
* @note If sumOfFreeBlocksInPartition == 0, the operation failed, shrink is not an option.
*       This API should be called in cases that a regular prvCpssDmmAllocate
*       return DMM_BLOCK_NOT_FOUND or DMM_MALLOC_FAIL
*       oldBlockSize may be 0 in cases we do not have a resize operation but a new bucket.
*       in this case no real use of the oldBlockSize parameter
*
*/
GT_U32 prvCpssDmmCheckIfShrinkOperationUsefulForDefrag
(
    IN    GT_UINTPTR   partitionId,
    IN    GT_U32       newBlockSize,
    IN    GT_U32       oldBlockSize,
    INOUT GT_BOOL      *useSwapAreaPtr
);

/**
* @internal prvCpssDmmGetAllocatedLinesForGivenPartitionAndProtocol function
* @endinternal
*
* @brief   Get allocated lines for given partition and Protocol
*
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
* @param[in] protocol                 - the  we want to sum its lines
*
* @retval sumOfAllocatedLines      - size of the allocated lines for this protocol
*/
GT_U32 prvCpssDmmGetAllocatedLinesForGivenPartitionAndProtocol
(
    IN    GT_UINTPTR   partitionId,
    IN    GT_U32       protocol
);

/**
* @internal prvCpssDmmGetFreeLinesForGivenPartition function
* @endinternal
*
* @brief   Get free lines for given partition
*
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
*
* @retval sumOfFreeLines           - size of the free lines
*/
GT_U32 prvCpssDmmGetFreeLinesForGivenPartition
(
    IN    GT_UINTPTR     partitionId
);

/**
* @internal prvCpssDmmAllPartitionIsUpdatedInHw function
* @endinternal
*
* @brief   Check if all blocks in the partition are updated in HW
*
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
*
* @param[out] allUpdatedInHwFlagPtr    - (pointer to)
*                                      GT_TRUE: all blocks in the partition are updated in HW
*                                      GT_FALSE: NOT all blocks in the partition are updated in HW
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssDmmAllPartitionIsUpdatedInHw
(
    IN    GT_UINTPTR     partitionId,
    OUT   GT_BOOL        *allUpdatedInHwFlagPtr
);

/**
* @internal prvCpssDmmFreeSortedPartitionArray function
* @endinternal
*
* @brief   For a given partitionId free the sorted partition array elements
*
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
*
* @retval GT_OK                    - free operation success
* @retval GT_FAIL                  - free operation fail
*/
GT_U32 prvCpssDmmFreeSortedPartitionArray
(
    IN    GT_UINTPTR   partitionId
);

/**
* @internal prvCpssDmmSortPartition function
* @endinternal
*
* @brief   For a given partitionId, sort the allocated memory according to the
*         blocks size. Each element in the sorted array, will hold a linked
*         list of all blocks with the same size.
* @param[in] partitionId              - ID of the partition, created by dmmCreatePartition().
* @param[in] protocol                 - the  we want to sort it's blocks
*
* @retval GT_OK                    - sort operation success
* @retval GT_FAIL                  - sort operation fail
*/
GT_U32 prvCpssDmmSortPartition
(
    IN GT_UINTPTR   partitionId,
    IN GT_U32       protocol
);

#endif /*__prvCpssDevMemManager*/


