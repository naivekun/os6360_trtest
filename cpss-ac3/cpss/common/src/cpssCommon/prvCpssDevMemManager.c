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
* @file prvCpssDevMemManager.c
*
* @brief Device memory manager.
*
* @version   15
********************************************************************************
*/

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpssCommon/cpssBuffManagerPool.h>
#include <cpssCommon/cpssPresteraDefs.h>

#ifdef CPSS_EXMX_IP_LPM_DEBUG_MEM

extern void * cpssOsIpMalloc
(
    IN GT_U32 size
);

extern void cpssOsIpFree
(
    IN void* const memblock
);
#else
#define cpssOsIpMalloc cpssOsMalloc
#define cpssOsIpFree cpssOsFree
#endif

#undef DMM_DEBUG_ENABLE

#define DMM_SET_EPTINESS(partition, index) \
                { partition->emptinessBitArray[index>>5] |=(1<<(index & 0x1F));}

#define DMM_CLEAN_EPTINESS(partition, index) \
              {partition->emptinessBitArray[index>>5] &= ~(1<<(index & 0x1F));}

#define DMM_EXIST_EXACT_FIT(partition, size) \
                   ((partition->tableOfSizePointers[size] != NULL)?\
                            (GT_TRUE):(GT_FALSE))

static GT_U32 maskArray[32] =
   {0x1,0x2,0x4,0x8,0x10,
    0x20,0x40,0x80,0x100,0x200,0x400,0x800,
    0x1000,0x2000,0x4000,0x8000,0x10000,0x20000,
    0x40000,0x80000,0x100000,0x200000,0x400000,
    0x800000,0x1000000,0x2000000,0x4000000,0x8000000,
    0x10000000,0x20000000,0x40000000,0x80000000};


static GT_U32 revMaskArray[32] =
   {0xFFFFFFFF ,0xFFFFFFFE ,0xFFFFFFFC ,0xFFFFFFF8 ,
    0xFFFFFFF0 ,0xFFFFFFE0 ,0xFFFFFFC0 ,0xFFFFFF80 ,
    0xFFFFFF00 ,0xFFFFFE00 ,0xFFFFFC00 ,0xFFFFF800 ,
    0xFFFFF000 ,0xFFFFE000 ,0xFFFFC000 ,0xFFFF8000 ,
    0xFFFF0000 ,0xFFFE0000 ,0xFFFC0000 ,0xFFF80000 ,
    0xFFF00000 ,0xFFE00000 ,0xFFC00000 ,0xFF800000 ,
    0xFF000000 ,0xFE000000 ,0xFC000000 ,0xF8000000 ,
    0xF0000000 ,0xE0000000 ,0xC0000000 ,0x80000000};



static GT_U32 dmmNextNotEmptyAfter
(
    GT_DMM_PARTITION    *partition,
    GT_U32              size
)
{
    GT_U32  theWord,theBit,retVal=0;

    /*Finish current word*/
    theWord = size >> 5;
    theBit = size & 0x1F;
    /*If the rest of the word are zero - skip the next while*/
    if( (partition->emptinessBitArray[theWord] & revMaskArray[theBit] ) !=0 )
    {
        while(GT_TRUE)
        {
            if(theBit == 32)
            {
                break;
            }

            if( (partition->emptinessBitArray[theWord] & maskArray[theBit]) ==0)
            {
                theBit++;
                continue;
            }
            else
            {
                return (theWord << 5) | theBit;
            }
        }
    }

    theWord++;

    /*Find next non zero word*/
    while(GT_TRUE)
    {
        if(theWord == DMM_EMPTINESS_ARRAY_SIZE)
            return 0;

        if( partition->emptinessBitArray[theWord] != 0 )
        {
            break;
        }
        else
        {
            theWord++;
        }
    }

    /*Find in non zero word*/
    theBit = 0;
    while(GT_TRUE)
    {
        if( (partition->emptinessBitArray[theWord] & maskArray[theBit]) == 0)
        {
            theBit++;
            continue;
        }
        else
        {
            retVal = (theWord << 5) | theBit;
            break;
        }
    }

    return retVal;
}


static GT_U32 removeFromSizedDll
(
    GT_DMM_PARTITION    *partition,
    GT_DMM_BLOCK        *node
)
{
    if((node->nextBySizeOrPartitionPtr.nextBySize != NULL) && (node->prevBySize != NULL) )
    { /*Middle of the dll*/
        node->prevBySize->nextBySizeOrPartitionPtr.nextBySize =
            node->nextBySizeOrPartitionPtr.nextBySize;
        node->nextBySizeOrPartitionPtr.nextBySize->prevBySize = node->prevBySize;
        node->prevBySize = node->nextBySizeOrPartitionPtr.nextBySize = NULL;
    }

    else if((node->nextBySizeOrPartitionPtr.nextBySize == NULL) && (node->prevBySize != NULL))
    { /*End of the dll*/
        node->prevBySize->nextBySizeOrPartitionPtr.nextBySize = NULL;
        node->prevBySize = NULL;
    }

    else if((node->nextBySizeOrPartitionPtr.nextBySize != NULL) && (node->prevBySize == NULL))
    { /*Head of the dll*/
        if(SIZE_IN_WORDS(node) <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED)
        {
            partition->tableOfSizePointers[SIZE_IN_WORDS(node)] =
                node->nextBySizeOrPartitionPtr.nextBySize;
        }
        else
        {
            partition->bigBlocksList = node->nextBySizeOrPartitionPtr.nextBySize;
        }
        node->nextBySizeOrPartitionPtr.nextBySize->prevBySize = NULL;
        node->nextBySizeOrPartitionPtr.nextBySize = NULL;
    }

    else if((node->nextBySizeOrPartitionPtr.nextBySize == NULL) && (node->prevBySize == NULL))
    { /*Single node in the dll*/

        if(SIZE_IN_WORDS(node) <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED)
        {
            partition->tableOfSizePointers[SIZE_IN_WORDS(node)] = NULL;
            DMM_CLEAN_EPTINESS(partition,SIZE_IN_WORDS(node));
        }
        else
        {
            partition->bigBlocksList = NULL;
        }
    }

    return 0;
}

static GT_U32 addToSizedDll
(
    GT_DMM_PARTITION    *partition,
    GT_DMM_BLOCK        *node
)
{
    if(SIZE_IN_WORDS(node) <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED)
    {
        node->nextBySizeOrPartitionPtr.nextBySize = partition->tableOfSizePointers[SIZE_IN_WORDS(node)];

        if(partition->tableOfSizePointers[SIZE_IN_WORDS(node)] != NULL)
        {
            partition->tableOfSizePointers[SIZE_IN_WORDS(node)]->prevBySize =node;
        }
        else
        {
            DMM_SET_EPTINESS(partition,SIZE_IN_WORDS(node));
        }

        partition->tableOfSizePointers[SIZE_IN_WORDS(node)] = node;
        node->prevBySize = NULL;
    }
    else
    {
        node->nextBySizeOrPartitionPtr.nextBySize = partition->bigBlocksList;
        if(partition->bigBlocksList != NULL)
            partition->bigBlocksList->prevBySize = node;
        partition->bigBlocksList = node;
        node->prevBySize = NULL;
    }

    return 0;
}

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
)
{
    GT_DMM_PARTITION    *partition;
    GT_DMM_BLOCK        *newBlock;

    if( ( (sizeOfPartition & 0x3) != 0 ) ||
        ( (startAddress & 0x3) != 0 ) || ( sizeOfPartition == 0 ) ||
        ( (minBlockSize & 0x3) != 0 ) || ( minBlockSize == 0 ) ||
        ( (maxBlockSize >> 2) > DMM_MAXIMAL_BLOCK_SIZE_ALLOWED ))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    partition = (GT_DMM_PARTITION*)cpssOsIpMalloc(sizeof(GT_DMM_PARTITION));
    if (partition == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);

    /*Arrays must be 0*/
    cpssOsMemSet(partition,0,sizeof(GT_DMM_PARTITION));
    cpssOsMemSet(partition->tableOfSizePointers,0,
             sizeof(partition->tableOfSizePointers));
    cpssOsMemSet(partition->emptinessBitArray,0,
             sizeof(partition->emptinessBitArray));
    cpssOsMemSet(partition->tableOfSortedUsedBlocksPointers,0,
             sizeof(partition->tableOfSortedUsedBlocksPointers));

    /*No big blocks*/
    partition->bigBlocksList = NULL;

    partition->minBlockSizeInWords = minBlockSize >> 2;
    partition->maxBlockSizeInWords = maxBlockSize >> 2;

    sizeOfPartition = sizeOfPartition >> 2;

    /* Add new block to database */
    newBlock = (GT_DMM_BLOCK*)cpssOsIpMalloc(sizeof (GT_DMM_BLOCK));
    if(newBlock == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(newBlock,0,sizeof(GT_DMM_BLOCK));
    partition->currentBlocks = partition->pickBlocks = 1;
    partition->pickAllocatedBytes = partition->allocatedBytes = 0;

    SET_OFFSET_IN_WORDS(newBlock, (startAddress >> 2));
    SET_DMM_BLOCK_STATUS_FREE(newBlock);
    SET_DMM_BLOCK_STAMP(newBlock, DMM_STAMP_FREE);
    RESET_BUCKET_SW_ADDRESS(newBlock);
    RESET_BLOCK_PROTOCOL(newBlock);
    SET_SIZE_IN_WORDS(newBlock, sizeOfPartition);

    /*Dlls with one node*/
    newBlock->prevBySize = newBlock->nextBySizeOrPartitionPtr.nextBySize = NULL;
    newBlock->nextByAddr = newBlock->prevByAddr = NULL;

    addToSizedDll(partition,newBlock);

    *partitionId = (GT_UINTPTR)partition;

    /* pointedBlock points on the only block in the partition */
    partition->pointedBlock = newBlock;

    /* pointedFirstFreeBlock points on the first free block in the partition */
    partition->pointedFirstFreeBlock = newBlock;

    /* pointedFirstBlock points on the first block in the partition (free or allocated) */
    partition->pointedFirstBlock = newBlock;

    return GT_OK;
}

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
)
{
    GT_DMM_BLOCK        *allocatedBlock = NULL;
    GT_DMM_BLOCK        *foundBlock = NULL;
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_U32              blockSize = size >> 2;
    GT_DMM_BLOCK        *tempBlock;/* use for going over the list of all blocks (free and allocated) */

    alignment = alignment; /* warning fix */

    foundBlock = partition->bigBlocksList;

    while ((foundBlock != NULL) && (SIZE_IN_WORDS(foundBlock) < blockSize))
    {
        foundBlock = foundBlock->nextBySizeOrPartitionPtr.nextBySize;
    }

    if (foundBlock == NULL)
    {
        return 0;
    }

    /*Update addressed dll*/
    if ( NULL ==
         ( allocatedBlock = (GT_DMM_BLOCK*)cpssOsIpMalloc(sizeof(GT_DMM_BLOCK))))
    {
        return (0);
    }

    cpssOsMemSet(allocatedBlock,0,sizeof(GT_DMM_BLOCK));

    partition->currentBlocks++;
    if (  partition->currentBlocks > partition->pickBlocks )
    {
        partition->pickBlocks = partition->currentBlocks;
    }

    /*Update addressed dll*/
    if (foundBlock->prevByAddr != NULL)
        foundBlock->prevByAddr->nextByAddr = allocatedBlock;
    allocatedBlock->nextByAddr = foundBlock;
    allocatedBlock->prevByAddr = foundBlock->prevByAddr;
    foundBlock->prevByAddr = allocatedBlock;

    SET_OFFSET_IN_WORDS(allocatedBlock, OFFSET_IN_WORDS(foundBlock));
    SET_SIZE_IN_WORDS(allocatedBlock, blockSize);
    SET_DMM_BLOCK_STATUS_ALLOCATED(allocatedBlock);
    SET_DMM_BLOCK_STAMP(allocatedBlock, DMM_STAMP_USED_STATIC);
    allocatedBlock->prevBySize = NULL;

    SET_OFFSET_IN_WORDS(foundBlock, (OFFSET_IN_WORDS(foundBlock)+blockSize));

    /*Remove from old sized dll*/
    removeFromSizedDll(partition,foundBlock);

    SET_SIZE_IN_WORDS(foundBlock, (SIZE_IN_WORDS(foundBlock)-blockSize));

    /*Add to new sized dll*/
    addToSizedDll(partition,foundBlock);


    partition->allocatedBytes += blockSize << 2;
    if (partition->allocatedBytes > partition->pickAllocatedBytes )
        partition->pickAllocatedBytes = partition->allocatedBytes;

    /* pointedBlock points on the allocated block */
    partition->pointedBlock = allocatedBlock;

    /* if the new allocated block has a smaller offset then the current pointedFirstBlock,
       then we need to update the pointedFirstBlock to be the new allocatedBlock */
    if (partition->pointedFirstBlock==NULL)
    {
        cpssOsPrintf("\n partition->pointedFirstBlock should not be NULL since it is always allocated in prvCpssDmmCreatePartition\n");
        return 0;
    }
    if (OFFSET_IN_WORDS(allocatedBlock) < OFFSET_IN_WORDS(partition->pointedFirstBlock))
    {
        partition->pointedFirstBlock = allocatedBlock;
    }

    allocatedBlock->nextBySizeOrPartitionPtr.partitionPtr = partition;

    /* find next free block incase pointedFirstFreeBlock is not free anymore
       look for a free block located after the pointedFirstFreeBlock (next by address) */
    if(DMM_BLOCK_STATUS(partition->pointedFirstFreeBlock) != DMM_BLOCK_FREE)
    {
        tempBlock = partition->pointedFirstFreeBlock;
        while (tempBlock->nextByAddr!=NULL)
        {
            /* look for the next free block */
            if (DMM_BLOCK_STATUS(tempBlock->nextByAddr)==DMM_BLOCK_FREE)
            {
                /* found a free block */
                partition->pointedFirstFreeBlock = tempBlock->nextByAddr;
                break;
            }
            tempBlock=tempBlock->nextByAddr;
        }
        /* verify if we found a new free block */
        if(DMM_BLOCK_STATUS(partition->pointedFirstFreeBlock) != DMM_BLOCK_FREE)
        {
            /* there are no free blocks */
            partition->pointedFirstFreeBlock = NULL;
        }
    }
    return (GT_UINTPTR)allocatedBlock;
}

/**
* @internal prvCpssDmmAllocate function
* @endinternal
*
* @brief   Allocate new device memory block to existing partition.
*
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
GT_UINTPTR prvCpssDmmAllocate
(
    IN GT_UINTPTR   partitionId,
    IN GT_U32       size,
    IN GT_U32       alignment
)
{
    GT_DMM_BLOCK        *allocatedBlock = NULL;
    GT_DMM_BLOCK        *foundBlock = NULL;
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_U32              blockSize = size >> 2;
    GT_U32              foundBlockSize = 0, tempSize;
    GT_DMM_BLOCK        *tempBlock;/* use for going over the list of all blocks (free and allocated) */

    alignment = alignment; /* warning fix */

    if( (blockSize > partition->maxBlockSizeInWords) ||
        (blockSize < partition->minBlockSizeInWords) /*||
        (alignment != 2 )*/ )
    {
        cpssOsPrintf("prvCpssDmmAllocate : wrong size calculation!!!!");
        return DMM_BLOCK_NOT_FOUND;
    }

    if(DMM_EXIST_EXACT_FIT(partition,blockSize))
    {
        allocatedBlock = partition->tableOfSizePointers[blockSize];

        removeFromSizedDll(partition,allocatedBlock);

        /*Change status of the allocated block*/
        SET_DMM_BLOCK_STATUS_ALLOCATED(allocatedBlock);
        SET_DMM_BLOCK_STAMP(allocatedBlock, DMM_STAMP_USED);

        partition->allocatedBytes += blockSize << 2;
        if(partition->allocatedBytes > partition->pickAllocatedBytes )
            partition->pickAllocatedBytes = partition->allocatedBytes;

        /* pointedBlock points on the allocated block */
        partition->pointedBlock = allocatedBlock;

        /* if the new allocated block has a smaller offset then the current pointedFirstBlock,
           then we need to update the pointedFirstBlock to be the new allocatedBlock */
        if (partition->pointedFirstBlock==NULL)
        {
            cpssOsPrintf("\n partition->pointedFirstBlock should not be NULL since it is always allocated in prvCpssDmmCreatePartition\n");
            return 0;
        }
        if (OFFSET_IN_WORDS(allocatedBlock) < OFFSET_IN_WORDS(partition->pointedFirstBlock))
        {
            partition->pointedFirstBlock = allocatedBlock;
        }

        /*  - store partition id in the allocated DMM block.*/
        allocatedBlock->nextBySizeOrPartitionPtr.partitionPtr = partition;

    }
    else
    {
        /*Look for block*/
        tempSize = blockSize + partition->minBlockSizeInWords;
        if( tempSize <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED )
        {
            if(DMM_EXIST_EXACT_FIT(partition,tempSize))
            {
                foundBlockSize = tempSize;
            }
        }

        if( foundBlockSize == 0)
        {
            tempSize = blockSize << 1;
            if( tempSize <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED )
            {
                if(DMM_EXIST_EXACT_FIT(partition,tempSize))
                {
                    foundBlockSize = tempSize;
                }
            }
        }

        if( foundBlockSize == 0)
        {
            if( 0 == (foundBlockSize = dmmNextNotEmptyAfter(partition,
                            blockSize + partition->minBlockSizeInWords - 1) ))
            {
                if( partition->bigBlocksList == NULL )
                {
                    foundBlockSize = dmmNextNotEmptyAfter(partition,blockSize);
                }
            }
        }

        if( (foundBlockSize == 0) && (partition->bigBlocksList ==  NULL))
        {   /*If sized block not found and list of big blocks empry -
                out of memory */
            return DMM_BLOCK_NOT_FOUND;
        }

        foundBlock = ( foundBlockSize == 0 )?(partition->bigBlocksList):
                            (partition->tableOfSizePointers[foundBlockSize]);

        /*Update addressed dll*/
        if( NULL ==
            ( allocatedBlock = (GT_DMM_BLOCK*)cpssOsIpMalloc(sizeof(GT_DMM_BLOCK))))
        {
            return DMM_MALLOC_FAIL;
        }

        cpssOsMemSet(allocatedBlock,0,sizeof(GT_DMM_BLOCK));

        partition->currentBlocks++;
        if(  partition->currentBlocks > partition->pickBlocks )
        {
            partition->pickBlocks = partition->currentBlocks;
        }

        /*Update addressed dll*/
        if(foundBlock->prevByAddr != NULL)
            foundBlock->prevByAddr->nextByAddr = allocatedBlock;
        allocatedBlock->nextByAddr = foundBlock;
        allocatedBlock->prevByAddr = foundBlock->prevByAddr;
        foundBlock->prevByAddr = allocatedBlock;

        SET_OFFSET_IN_WORDS(allocatedBlock, OFFSET_IN_WORDS(foundBlock));
        SET_SIZE_IN_WORDS(allocatedBlock, blockSize);
        SET_DMM_BLOCK_STATUS_ALLOCATED(allocatedBlock);
        SET_DMM_BLOCK_STAMP(allocatedBlock, DMM_STAMP_USED);

        allocatedBlock->nextBySizeOrPartitionPtr.nextBySize = allocatedBlock->prevBySize = NULL;

        SET_OFFSET_IN_WORDS(foundBlock, (OFFSET_IN_WORDS(foundBlock)+blockSize));

        /*Update sized dll's*/
        if( foundBlockSize != 0 )
        { /*Small block to smal block*/

            /*Remove from old sized dll*/
            removeFromSizedDll(partition,foundBlock);

            SET_SIZE_IN_WORDS(foundBlock, (SIZE_IN_WORDS(foundBlock)-blockSize));

            /*Add to new sized dll*/
            addToSizedDll(partition,foundBlock);
        }
        else if ( (SIZE_IN_WORDS(foundBlock)-blockSize) >
                  DMM_MAXIMAL_BLOCK_SIZE_ALLOWED)
        { /* Big block to big block*/

            /*Just update the size*/
            SET_SIZE_IN_WORDS(foundBlock, (SIZE_IN_WORDS(foundBlock)-blockSize));
        }
        else
        { /* Big block to small block*/

            /*Remove from big blocks dll ( remove the head of the dll)*/
            removeFromSizedDll(partition, partition->bigBlocksList);

            SET_SIZE_IN_WORDS(foundBlock, (SIZE_IN_WORDS(foundBlock)-blockSize));

            /*Add to new sized dll*/
            addToSizedDll(partition,foundBlock);
        }

        partition->allocatedBytes += blockSize << 2;
        if(partition->allocatedBytes > partition->pickAllocatedBytes )
            partition->pickAllocatedBytes = partition->allocatedBytes;

        /* pointedBlock points on the allocated block */
        partition->pointedBlock = allocatedBlock;

        /* if the new allocated block has a smaller offset then the current pointedFirstBlock,
        then we need to update the pointedFirstBlock to be the new allocatedBlock */
        if (partition->pointedFirstBlock==NULL)
        {
            cpssOsPrintf("\n partition->pointedFirstBlock should not be NULL since it is always allocated in prvCpssDmmCreatePartition\n");
            return 0;
        }
        if (OFFSET_IN_WORDS(allocatedBlock) < OFFSET_IN_WORDS(partition->pointedFirstBlock))
        {
            partition->pointedFirstBlock = allocatedBlock;
        }

        /*  - store partition id in the allocated DMM block.*/
        allocatedBlock->nextBySizeOrPartitionPtr.partitionPtr = partition;

    }

    /* find next free block incase pointedFirstFreeBlock is not free anymore
       look for a free block located after the pointedFirstFreeBlock (next by address) */
    if(DMM_BLOCK_STATUS(partition->pointedFirstFreeBlock) != DMM_BLOCK_FREE)
    {
        tempBlock = partition->pointedFirstFreeBlock;
        while (tempBlock->nextByAddr!=NULL)
        {
            /* look for the next free block */
            if (DMM_BLOCK_STATUS(tempBlock->nextByAddr)==DMM_BLOCK_FREE)
            {
                /* found a free block */
                partition->pointedFirstFreeBlock = tempBlock->nextByAddr;
                break;
            }
            tempBlock=tempBlock->nextByAddr;
        }
        /* verify if we found a new free block */
        if(DMM_BLOCK_STATUS(partition->pointedFirstFreeBlock) != DMM_BLOCK_FREE)
        {
            /* there are no free blocks */
            partition->pointedFirstFreeBlock = NULL;
        }
    }
    return (GT_UINTPTR)allocatedBlock;
}


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
)
{
    GT_DMM_BLOCK        *allocatedBlock = NULL;
    GT_DMM_BLOCK        *freeBlock = NULL;
    GT_DMM_BLOCK        *foundBlock = NULL ;
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_U32              blockSize = size >> 2;
    GT_U32              foundBlockSize = 0,  givenHwOffsetInWords;
    GT_BOOL             gotoNext = GT_TRUE;
    GT_DMM_BLOCK        *tempBlock;/* use for going over the list of all blocks (free and allocated) */

    alignment = alignment; /* warning fix */

    if( (blockSize > partition->maxBlockSizeInWords) ||
        (blockSize < partition->minBlockSizeInWords) /*||
        (alignment != 2 )*/ )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    givenHwOffsetInWords = hwOffset >> 2;
    foundBlock = partition->pointedBlock;

    /* check which direction to search */
    if (givenHwOffsetInWords < OFFSET_IN_WORDS(foundBlock) )
    {
        gotoNext = GT_FALSE;
    }

    /* start by locating the block which includes this address in two directions
       search fashion */
    while ((foundBlock != NULL) &&
           ((givenHwOffsetInWords < OFFSET_IN_WORDS(foundBlock) ) ||
           (givenHwOffsetInWords >= (OFFSET_IN_WORDS(foundBlock) + SIZE_IN_WORDS(foundBlock)))))
    {
        foundBlock = (gotoNext == GT_TRUE)? foundBlock->nextByAddr:
            foundBlock->prevByAddr;
    }

    /* check if we reached the end of the memory , or this is an already
       allocated space, or the found block is smaller then the needed memory*/
    if ((foundBlock == NULL) || (DMM_BLOCK_STATUS(foundBlock) == DMM_BLOCK_ALLOCATED) ||
        (SIZE_IN_WORDS(foundBlock) < blockSize))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }


    /* now treat each one of the spilts */

    /* first case , the wanted offset is the same as the start of this block */
    if (OFFSET_IN_WORDS(foundBlock) == givenHwOffsetInWords)
    {
        /* first check if we have and exact match in size for a block */
        if (SIZE_IN_WORDS(foundBlock) == blockSize)
        {
            allocatedBlock = foundBlock;

            removeFromSizedDll(partition,allocatedBlock);

            /*Change status of the allocated block*/
            SET_DMM_BLOCK_STATUS_ALLOCATED(allocatedBlock);
            SET_DMM_BLOCK_STAMP(allocatedBlock, DMM_STAMP_USED);

            partition->allocatedBytes += blockSize << 2;
            if (partition->allocatedBytes > partition->pickAllocatedBytes )
                partition->pickAllocatedBytes = partition->allocatedBytes;

            /* pointedBlock points on the allocated block */
            partition->pointedBlock = allocatedBlock;

            /* if the new allocated block has a smaller offset then the current pointedFirstBlock,
               then we need to update the pointedFirstBlock to be the new allocatedBlock */
            if (partition->pointedFirstBlock==NULL)
            {
                cpssOsPrintf("\n partition->pointedFirstBlock should not be NULL since it is always allocated in prvCpssDmmCreatePartition\n");
                return 0;
            }
            if (OFFSET_IN_WORDS(allocatedBlock) < OFFSET_IN_WORDS(partition->pointedFirstBlock))
            {
                partition->pointedFirstBlock = allocatedBlock;
            }

            allocatedBlock->nextBySizeOrPartitionPtr.partitionPtr = partition;
            *hwOffsetHandlePtr = (GT_UINTPTR)allocatedBlock;
            return GT_OK;
        }
        else
        {
            /* we need to split in the middle of the block , the end will be a
               new free block */

            if ( NULL ==
                 ( allocatedBlock = (GT_DMM_BLOCK*)cpssOsIpMalloc(sizeof(GT_DMM_BLOCK))))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            cpssOsMemSet(allocatedBlock,0,sizeof(GT_DMM_BLOCK));

            partition->currentBlocks++;

            allocatedBlock->nextByAddr = foundBlock;
            allocatedBlock->prevByAddr = foundBlock->prevByAddr;

            if (foundBlock->prevByAddr != NULL)
                foundBlock->prevByAddr->nextByAddr = allocatedBlock;
            foundBlock->prevByAddr = allocatedBlock;

            SET_OFFSET_IN_WORDS(allocatedBlock, OFFSET_IN_WORDS(foundBlock));
            SET_SIZE_IN_WORDS(allocatedBlock, blockSize);
            SET_DMM_BLOCK_STATUS_ALLOCATED(allocatedBlock);
            SET_DMM_BLOCK_STAMP(allocatedBlock, DMM_STAMP_USED);
            allocatedBlock->nextBySizeOrPartitionPtr.nextBySize = allocatedBlock->prevBySize = NULL;

            SET_OFFSET_IN_WORDS(foundBlock, (OFFSET_IN_WORDS(foundBlock)+blockSize));

            /* calculate the new second free block size */
            foundBlockSize = SIZE_IN_WORDS(foundBlock) - blockSize;
        }
    }
    else
    {
        /* the wanted offset is in the middle of the block */
        if (OFFSET_IN_WORDS(foundBlock) + SIZE_IN_WORDS(foundBlock) ==
            givenHwOffsetInWords + blockSize)
        {
            /* the found block ends in the same place the found block ends */
            /* we need to split in middle of the block and the start block will
               be a new free block */

            if ( NULL ==
                 ( allocatedBlock = (GT_DMM_BLOCK*)cpssOsIpMalloc(sizeof(GT_DMM_BLOCK))))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            cpssOsMemSet(allocatedBlock,0,sizeof(GT_DMM_BLOCK));

            partition->currentBlocks++;

            allocatedBlock->nextByAddr = foundBlock->nextByAddr;
            allocatedBlock->prevByAddr = foundBlock;

            if (foundBlock->nextByAddr != NULL)
                foundBlock->nextByAddr->prevByAddr = allocatedBlock;
            foundBlock->nextByAddr = allocatedBlock;

            SET_OFFSET_IN_WORDS(allocatedBlock, givenHwOffsetInWords);
            SET_SIZE_IN_WORDS(allocatedBlock, blockSize);
            SET_DMM_BLOCK_STATUS_ALLOCATED(allocatedBlock);
            SET_DMM_BLOCK_STAMP(allocatedBlock, DMM_STAMP_USED);
            allocatedBlock->nextBySizeOrPartitionPtr.nextBySize = allocatedBlock->prevBySize = NULL;

            /* calculate the new first free block size */
            foundBlockSize = SIZE_IN_WORDS(foundBlock) - blockSize;
        }
        else
        {
            /* the needed block falls in the middle of the found block, we need
               to split the found block to two free blocks and a new allocated
               block */

            if ( NULL ==
                 ( allocatedBlock = (GT_DMM_BLOCK*)cpssOsIpMalloc(sizeof(GT_DMM_BLOCK))))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            cpssOsMemSet(allocatedBlock,0,sizeof(GT_DMM_BLOCK));
            if ( NULL ==
                 ( freeBlock = (GT_DMM_BLOCK*)cpssOsIpMalloc(sizeof(GT_DMM_BLOCK))))
            {
                cpssOsIpFree(allocatedBlock);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            cpssOsMemSet(freeBlock,0,sizeof(GT_DMM_BLOCK));
            partition->currentBlocks += 2;

            allocatedBlock->nextByAddr = freeBlock;
            allocatedBlock->prevByAddr = foundBlock;

            freeBlock->nextByAddr = foundBlock->nextByAddr;
            freeBlock->prevByAddr = allocatedBlock;

            if (foundBlock->nextByAddr != NULL)
                foundBlock->nextByAddr->prevByAddr = freeBlock;
            foundBlock->nextByAddr = allocatedBlock;

            /* calculate the new first free block size */

            foundBlockSize = givenHwOffsetInWords - OFFSET_IN_WORDS(foundBlock);

            SET_OFFSET_IN_WORDS(allocatedBlock, givenHwOffsetInWords);
            SET_SIZE_IN_WORDS(allocatedBlock, blockSize);
            SET_DMM_BLOCK_STATUS_ALLOCATED(allocatedBlock);
            SET_DMM_BLOCK_STAMP(allocatedBlock, DMM_STAMP_USED);
            allocatedBlock->nextBySizeOrPartitionPtr.nextBySize = allocatedBlock->prevBySize = NULL;

            SET_OFFSET_IN_WORDS(freeBlock, givenHwOffsetInWords + blockSize);
            SET_SIZE_IN_WORDS(freeBlock,
                              SIZE_IN_WORDS(foundBlock) - blockSize - foundBlockSize);
            SET_DMM_BLOCK_STATUS_FREE(freeBlock);
            SET_DMM_BLOCK_STAMP(freeBlock, DMM_STAMP_FREE);
            RESET_BUCKET_SW_ADDRESS(freeBlock);
            RESET_BLOCK_PROTOCOL(freeBlock);
            freeBlock->nextBySizeOrPartitionPtr.nextBySize = allocatedBlock->prevBySize = NULL;

            /*Add the new free to sized dll*/
            addToSizedDll(partition,freeBlock);
        }
    }

    /* update the foundblock in the DLL */

    /*Remove from old sized dll*/
    removeFromSizedDll(partition,foundBlock);

    /* update it's size */
    SET_SIZE_IN_WORDS(foundBlock, foundBlockSize);

    /*Add to new sized dll*/
    addToSizedDll(partition,foundBlock);



    partition->allocatedBytes += blockSize << 2;
    if (partition->allocatedBytes > partition->pickAllocatedBytes )
        partition->pickAllocatedBytes = partition->allocatedBytes;

    /* pointedBlock points on the allocated block */
    partition->pointedBlock = allocatedBlock;

    /* if the new allocated block has a smaller offset then the current pointedFirstBlock,
    then we need to update the pointedFirstBlock to be the new allocatedBlock */
    if (partition->pointedFirstBlock==NULL)
    {
        cpssOsPrintf("\n partition->pointedFirstBlock should not be NULL since it is always allocated in prvCpssDmmCreatePartition\n");
        return 0;
    }
    if (OFFSET_IN_WORDS(allocatedBlock) < OFFSET_IN_WORDS(partition->pointedFirstBlock))
    {
        partition->pointedFirstBlock = allocatedBlock;
    }

    allocatedBlock->nextBySizeOrPartitionPtr.partitionPtr = partition;

    /* find next free block incase pointedFirstFreeBlock is not free anymore
       look for a free block located after the pointedFirstFreeBlock (next by address) */
    if(DMM_BLOCK_STATUS(partition->pointedFirstFreeBlock) != DMM_BLOCK_FREE)
    {
        tempBlock = partition->pointedFirstFreeBlock;
        while (tempBlock->nextByAddr!=NULL)
        {
            /* look for the next free block */
            if (DMM_BLOCK_STATUS(tempBlock->nextByAddr)==DMM_BLOCK_FREE)
            {
                /* found a free block */
                partition->pointedFirstFreeBlock = tempBlock->nextByAddr;
                break;
            }
            tempBlock=tempBlock->nextByAddr;
        }
        /* verify if we found a new free block */
        if(DMM_BLOCK_STATUS(partition->pointedFirstFreeBlock) != DMM_BLOCK_FREE)
        {
            /* there are no free blocks */
            partition->pointedFirstFreeBlock = NULL;
        }
    }

    *hwOffsetHandlePtr = (GT_UINTPTR)allocatedBlock;
    return GT_OK;
}

/**
* @internal prvCpssDmmFree function
* @endinternal
*
* @brief   Free device memory block to existing partition.
*
* @param[in] handler                  - handler/id of the allocated by dmmAllocate() address
*
* @retval size                     - of the block freed if the status is GT_OK.
*
* @note If size == 0, the operation failed.
*
*/
GT_U32 prvCpssDmmFree
(
    IN GT_UINTPTR   handler
)
{
    GT_DMM_PARTITION    *partition;
    GT_DMM_BLOCK        *freedBlock, *unusedBlock;
    GT_U32              freedSize;
    GT_U8               leftRigthJoin = 0;
    GT_U32              pointedFirstFreeBlockOffset=0;

    freedBlock = (GT_DMM_BLOCK *)handler;
    freedSize = SIZE_IN_WORDS(freedBlock) << 2;

    partition = freedBlock->nextBySizeOrPartitionPtr.partitionPtr ;

    if( (handler == 0)  || (partition == 0) ||
        (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)) ||
        (DMM_BLOCK_STATUS(freedBlock) != DMM_BLOCK_ALLOCATED) ||
        (DMM_BLOCK_STAMP(freedBlock) != DMM_STAMP_USED) )
    {
        return 0;
    }

    /* keep the size before any change is done in the free blocks pointers

       if there is no pointedFirstFreeBlock it means this will be
       the first free block allocation */
    if (partition->pointedFirstFreeBlock!=NULL)
    {
        pointedFirstFreeBlockOffset = OFFSET_IN_WORDS(partition->pointedFirstFreeBlock);
    }

    SET_DMM_BLOCK_STATUS_FREE(freedBlock);
    SET_DMM_BLOCK_STAMP(freedBlock, DMM_STAMP_FREE);
    RESET_BUCKET_SW_ADDRESS(freedBlock);
    RESET_BLOCK_PROTOCOL(freedBlock);

    /*Try to join from rigth */
    if( (freedBlock->nextByAddr != NULL) &&
        (DMM_BLOCK_STATUS(freedBlock->nextByAddr) == DMM_BLOCK_FREE) &&
        ((OFFSET_IN_WORDS(freedBlock) + SIZE_IN_WORDS(freedBlock)) ==
         OFFSET_IN_WORDS(freedBlock->nextByAddr)) )
    {
        /*remove joined block from sized dll */
        removeFromSizedDll(partition,freedBlock->nextByAddr);

        SET_SIZE_IN_WORDS(freedBlock,
            SIZE_IN_WORDS(freedBlock) + SIZE_IN_WORDS(freedBlock->nextByAddr));

        leftRigthJoin++;
    }

    /*Try to join from left */
    if( (freedBlock->prevByAddr != NULL) &&
        (DMM_BLOCK_STATUS(freedBlock->prevByAddr) == DMM_BLOCK_FREE) &&
        ((OFFSET_IN_WORDS(freedBlock->prevByAddr) +
          SIZE_IN_WORDS(freedBlock->prevByAddr)) == OFFSET_IN_WORDS(freedBlock)) )
    {
        /*remove joined block from sized dll */
        removeFromSizedDll(partition, freedBlock->prevByAddr);

        SET_SIZE_IN_WORDS(freedBlock->prevByAddr,
            SIZE_IN_WORDS(freedBlock->prevByAddr) + SIZE_IN_WORDS(freedBlock));

        freedBlock = freedBlock->prevByAddr;
        leftRigthJoin++;
    }

    /*Arrange sized DLL*/
    if(leftRigthJoin == 1)
    {
        unusedBlock = freedBlock->nextByAddr;
        if(freedBlock->nextByAddr->nextByAddr != NULL)
            freedBlock->nextByAddr->nextByAddr->prevByAddr = freedBlock;
        freedBlock->nextByAddr = freedBlock->nextByAddr->nextByAddr;

        cpssOsIpFree(unusedBlock);

        partition->currentBlocks--;
    }
    else if (leftRigthJoin == 2)
    {
        unusedBlock = freedBlock->nextByAddr;

        if(freedBlock->nextByAddr->nextByAddr->nextByAddr != NULL)
            freedBlock->nextByAddr->nextByAddr->nextByAddr->prevByAddr =
                freedBlock;
        freedBlock->nextByAddr = freedBlock->nextByAddr->nextByAddr->nextByAddr;

        cpssOsIpFree(unusedBlock->nextByAddr);

        partition->currentBlocks--;

        cpssOsIpFree(unusedBlock);

        partition->currentBlocks--;
    }

    /*Add to new sized dll*/
    addToSizedDll(partition,freedBlock);


    /* pointedBlock points on free block */
    partition->pointedBlock = freedBlock;

    /* if the new allocated block has a smaller offset then the current pointedFirstBlock,
    then we need to update the pointedFirstBlock to be the new allocatedBlock */
    if (partition->pointedFirstBlock==NULL)
    {
        cpssOsPrintf("\n partition->pointedFirstBlock should not be NULL since it is always allocated in prvCpssDmmCreatePartition\n");
        return 0;
    }
    if (OFFSET_IN_WORDS(freedBlock) < OFFSET_IN_WORDS(partition->pointedFirstBlock))
    {
        partition->pointedFirstBlock = freedBlock;
    }

    partition->allocatedBytes -= freedSize;

    /* check is pointedFirstFreeBlock is still the first free
       block or it should be updated to be freedBlock
       if freedBlock_size <  :then it means it is located before pointedFirstFreeBlockOffset
       if freedBlock_size =  :then it means it was merged with pointedFirstFreeBlock and
       now it is still the first free */
    if (partition->pointedFirstFreeBlock==NULL)
    {
         partition->pointedFirstFreeBlock = freedBlock;
    }
    else
    {
        if (DMM_BLOCK_STATUS(partition->pointedFirstFreeBlock) == DMM_BLOCK_FREE)
        {
            if(OFFSET_IN_WORDS(freedBlock) <= pointedFirstFreeBlockOffset)
            {
                partition->pointedFirstFreeBlock = freedBlock;
            }
            else
            {
                /*pointedFirstFreeBlockOffset is still the first free block in the list*/
            }
        }
        else
        {
             partition->pointedFirstFreeBlock = freedBlock;
        }
    }
    return freedSize;
}

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
)
{
    GT_DMM_PARTITION *partition = (GT_DMM_PARTITION*)partitionId;

    if(partition->pointedBlock)
    {
        /* was allocated in
            newBlock = (GT_DMM_BLOCK*)cpssOsIpMalloc(sizeof (GT_DMM_BLOCK)); */
        cpssOsIpFree(partition->pointedBlock);
    }

    cpssOsIpFree(partition);

    return GT_OK;
}

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
    IN GT_UINTPTR partitionId,
    OUT GT_U32 *numOfUsedBlocks,
    OUT GT_U32 *numOfAllocatedBytes,
    OUT GT_U32 *numOfFreeBytes,
    OUT GT_U32 *numOfPickAllocatedBytes,
    OUT GT_U32 *numOfPickAllBlocks
)
{
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_DMM_BLOCK        *tempBlock;
    GT_U32              numOfAllocatedBlocks = 0;
    GT_U32              numOfFreeBlocks = 0;
    GT_U32              freeMem = 0, allocMem = 0;
    static GT_U32       sizes[DMM_MAXIMAL_BLOCK_SIZE_ALLOWED];
    GT_U32              addrCheck = 0;

    cpssOsMemSet(sizes,0,DMM_MAXIMAL_BLOCK_SIZE_ALLOWED*4);

    /* scan the partition block - start from pointedBlock */


    /*Go left from pointed block */
    tempBlock = partition->pointedBlock;
    addrCheck = OFFSET_IN_WORDS(tempBlock);
    do
    {
        if (addrCheck != OFFSET_IN_WORDS(tempBlock))
        {
            cpssOsPrintf("inconsistency found \n");
        }

        if(DMM_BLOCK_STATUS(tempBlock) == DMM_BLOCK_FREE )
        {
            numOfFreeBlocks++;
            freeMem += (SIZE_IN_WORDS(tempBlock)<<2);
            if(SIZE_IN_WORDS(tempBlock) < DMM_MAXIMAL_BLOCK_SIZE_ALLOWED )
            {
                sizes[SIZE_IN_WORDS(tempBlock)]++;
            }
        }
        else
        {
            numOfAllocatedBlocks++;
            allocMem += (SIZE_IN_WORDS(tempBlock)<<2);
        }

        tempBlock = tempBlock->prevByAddr;
        if (tempBlock != NULL)
        {
            addrCheck -= SIZE_IN_WORDS(tempBlock);
        }
    }while(tempBlock != NULL);

    /*Go rigth from pointed block*/
    tempBlock = partition->pointedBlock->nextByAddr;
    if (tempBlock != NULL)
    {
        addrCheck = OFFSET_IN_WORDS(tempBlock);
    }
    while(tempBlock != NULL)
    {
        if (addrCheck != OFFSET_IN_WORDS(tempBlock))
        {
            cpssOsPrintf("inconsistency found \n");
        }
        if(DMM_BLOCK_STATUS(tempBlock) == DMM_BLOCK_FREE )
        {
            numOfFreeBlocks++;
            freeMem += (SIZE_IN_WORDS(tempBlock)<<2);
            if(SIZE_IN_WORDS(tempBlock) < DMM_MAXIMAL_BLOCK_SIZE_ALLOWED )
            {
                sizes[SIZE_IN_WORDS(tempBlock)]++;
            }
        }
        else
        {
            numOfAllocatedBlocks++;
            allocMem += (SIZE_IN_WORDS(tempBlock)<<2);
        }

        addrCheck += SIZE_IN_WORDS(tempBlock);
        tempBlock = tempBlock->nextByAddr;
    }

    cpssOsPrintf("\n\n");

    *numOfUsedBlocks     = numOfAllocatedBlocks;
    *numOfAllocatedBytes = allocMem;
    *numOfFreeBytes      = freeMem;

    *numOfPickAllocatedBytes = partition->pickAllocatedBytes;
    *numOfPickAllBlocks      = partition->pickBlocks;

    /*
    for( i = 0; i < DMM_MAXIMAL_BLOCK_SIZE_ALLOWED; i++)
    {
        if( sizes[i] == 0 )
            continue;
        osPrintf("\n In Size %d \t %d",i,sizes[i]);

    }
    osPrintf("\n\n Allocated blocks : %d \n Free blocks : %d \
             \n Allocated memory : 0x%X\n Free memory      : 0x%X\n All memory : 0x%X",
             numOfAllocatedBlocks, numOfFreeBlocks, freeMem, allocMem, (freeMem+allocMem));
*/
    return GT_OK;
}


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
)
{
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_DMM_BLOCK        *tempBlock;
    GT_U32              numOfAllocatedBlocks = 0;
    GT_U32              numOfFreeBlocks = 0;
    GT_U32              freeMem = 0, allocMem = 0;
    static GT_U32       sizes[DMM_MAXIMAL_BLOCK_SIZE_ALLOWED];
    GT_U32              addrCheck = 0;
    GT_U32              i = 0;
    GT_DMM_BLOCK *tempPtr;

    cpssOsMemSet(sizes,0,DMM_MAXIMAL_BLOCK_SIZE_ALLOWED*4);
    cpssOsPrintf("\n\n\nPrint partition 0X%p memory statistic:\n\n",partitionId);
    /* print free list for this partition */
    cpssOsPrintf(" \nfree lists for small blocks till %d bytes:\n",
                 DMM_MAXIMAL_BLOCK_SIZE_ALLOWED*4);
    for (i= 0; i <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED; i++)
    {
        if (partition->tableOfSizePointers[i] == NULL)
        {
            continue;
        }
        tempPtr =  partition->tableOfSizePointers[i];
        cpssOsPrintf("%d :",i*4);
        while(tempPtr != NULL)
        {
            cpssOsPrintf("%x ->",OFFSET_IN_WORDS(tempPtr));
            tempPtr = tempPtr->nextBySizeOrPartitionPtr.nextBySize;
        }
        cpssOsPrintf("EOSL\n");
        cpssOsPrintf("----------------------------------------------------\n");
    }
    /* check big free blocks */
    if (partition->bigBlocksList != NULL)
    {
        if(DMM_BLOCK_STATUS(partition->bigBlocksList) != DMM_BLOCK_FREE )
        {
            cpssOsPrintf("!!!!!!!! not free big block\n");
        }
         tempPtr = partition->bigBlocksList;
         cpssOsPrintf("free list for big block\n");
         cpssOsPrintf("%d:",SIZE_IN_WORDS(tempPtr));

         while(tempPtr != NULL)
         {
             cpssOsPrintf("%x ->",OFFSET_IN_WORDS(tempPtr));
             tempPtr = tempPtr->nextBySizeOrPartitionPtr.nextBySize;
         }
         cpssOsPrintf("EOBL\n");
         cpssOsPrintf("-----------------------------------------------------------------\n");

    }

    /* scan the partition block - start from pointedBlock */

    /*Go left from pointed block */
    tempBlock = partition->pointedBlock;
    addrCheck = OFFSET_IN_WORDS(tempBlock);
    while(tempBlock->nextByAddr != NULL)
        tempBlock = tempBlock->nextByAddr;
    addrCheck = OFFSET_IN_WORDS(tempBlock);
    do
    {
        if (addrCheck != OFFSET_IN_WORDS(tempBlock))
        {
            cpssOsPrintf("inconsistency found \n");
        }

        if(DMM_BLOCK_STATUS(tempBlock) == DMM_BLOCK_FREE )
        {
            numOfFreeBlocks++;
            freeMem += (SIZE_IN_WORDS(tempBlock));
            if(SIZE_IN_WORDS(tempBlock) < DMM_MAXIMAL_BLOCK_SIZE_ALLOWED )
            {
                sizes[SIZE_IN_WORDS(tempBlock)]++;
            }
            cpssOsPrintf("Free Block:      addr = 0x%X, size = %d\n",
                         OFFSET_IN_WORDS(tempBlock),
                         SIZE_IN_WORDS(tempBlock));
        }
        else
        {
            numOfAllocatedBlocks++;
            allocMem += (SIZE_IN_WORDS(tempBlock));
            cpssOsPrintf("Allocated Block: addr = 0x%X, size = %d\n",
                         OFFSET_IN_WORDS(tempBlock),
                         SIZE_IN_WORDS(tempBlock));
        }

        tempBlock = tempBlock->prevByAddr;
        if (tempBlock != NULL)
        {
            addrCheck -= SIZE_IN_WORDS(tempBlock);
        }
    }while(tempBlock != NULL);

    /*Go rigth from pointed block*/
   /* tempBlock = partition->pointedBlock->nextByAddr;
    if (tempBlock != NULL)
    {
        addrCheck = OFFSET_IN_WORDS(tempBlock);
    }
    while(tempBlock != NULL)
    {
        if (addrCheck != OFFSET_IN_WORDS(tempBlock))
        {
            cpssOsPrintf("inconsistency found \n");
        }
        if(DMM_BLOCK_STATUS(tempBlock) == DMM_BLOCK_FREE )
        {
            numOfFreeBlocks++;
            freeMem += (SIZE_IN_WORDS(tempBlock)<<2);
            if(SIZE_IN_WORDS(tempBlock) < DMM_MAXIMAL_BLOCK_SIZE_ALLOWED )
            {
                sizes[SIZE_IN_WORDS(tempBlock)]++;
            }
            cpssOsPrintf("Free Block:      addr = 0x%X, size = %d\n",
                         OFFSET_IN_WORDS(tempBlock),
                         SIZE_IN_WORDS(tempBlock)<<2);
        }
        else
        {
            numOfAllocatedBlocks++;
            allocMem += (SIZE_IN_WORDS(tempBlock)<<2);
            cpssOsPrintf("Allocated Block: addr = 0x%X, size = %d\n",
                         OFFSET_IN_WORDS(tempBlock),
                         SIZE_IN_WORDS(tempBlock)<<2);
        }

        addrCheck += SIZE_IN_WORDS(tempBlock);
        tempBlock = tempBlock->nextByAddr;
    }*/

    cpssOsPrintf("\n\n Allocated blocks : %d \n Free blocks : %d \
             \n Allocated memory : 0x%X\n Free memory      : 0x%X\n All memory : 0x%X",
             numOfAllocatedBlocks, numOfFreeBlocks, allocMem, freeMem, (freeMem+allocMem));
    cpssOsPrintf("\n\n Real allocated bytes : %d, falcon lines : %d\n",
                 partition->allocatedBytes,partition->allocatedBytes/16);
    return GT_OK;
}

/**
* @internal prvCpssDmmCheckResizeAvailableWithSameMemory function
* @endinternal
*
* @brief   Check if resize is available using same memory block
*
* @param[in] handler                  - handler/id of the allocated by dmmAllocate() address.
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
)
{
    GT_DMM_PARTITION    *partition;
    GT_DMM_BLOCK        *usedBlock;
    GT_U32              sizeOfOptionalUsedAndFreeBlock=0;

    usedBlock = (GT_DMM_BLOCK *)handler;
    partition = usedBlock->nextBySizeOrPartitionPtr.partitionPtr;/* each handler points to the partition from were it was allocated */

    if((newBlockSize > partition->maxBlockSizeInWords)||
       (newBlockSize < partition->minBlockSizeInWords))
    {
        return DMM_BLOCK_NOT_FOUND;
    }

    /*Try to join from rigth */
    if( (usedBlock->nextByAddr != NULL) &&
        (DMM_BLOCK_STATUS(usedBlock->nextByAddr) == DMM_BLOCK_FREE) &&
        ((SIZE_IN_WORDS(usedBlock)+SIZE_IN_WORDS(usedBlock->nextByAddr)) >= newBlockSize)&&
        ((OFFSET_IN_WORDS(usedBlock) + SIZE_IN_WORDS(usedBlock)) ==
         OFFSET_IN_WORDS(usedBlock->nextByAddr)) )
    {
        sizeOfOptionalUsedAndFreeBlock += SIZE_IN_WORDS(usedBlock->nextByAddr);

    }

    /*Try to join from left */
     if( (usedBlock->prevByAddr != NULL) &&
        (DMM_BLOCK_STATUS(usedBlock->prevByAddr) == DMM_BLOCK_FREE) &&
        ((SIZE_IN_WORDS(usedBlock)+SIZE_IN_WORDS(usedBlock->prevByAddr)) >= newBlockSize)&&
        ((OFFSET_IN_WORDS(usedBlock->prevByAddr) +
          SIZE_IN_WORDS(usedBlock->prevByAddr)) == OFFSET_IN_WORDS(usedBlock)))
    {
         sizeOfOptionalUsedAndFreeBlock += SIZE_IN_WORDS(usedBlock->prevByAddr);
    }

     /* if there is a free block near the usedBlock then add the size of the
        usedBlock to the sum of free blocks*/
     if (sizeOfOptionalUsedAndFreeBlock>0)
     {
         sizeOfOptionalUsedAndFreeBlock += SIZE_IN_WORDS(usedBlock);
     }
     return sizeOfOptionalUsedAndFreeBlock;
}


/**
* @internal dmmGetEmptyMemoryLeft function
* @endinternal
*
* @brief   Return the total size of empty memory blocks in the partition
*
* @param[in] partition                -  created by dmmCreatePartition().
*
* @retval sumOfFreeBlocksInPartition - size of the block created in case we merge the
*                                       free blocks using shrink
*/
static GT_U32 dmmGetEmptyMemoryLeft
(
    GT_DMM_PARTITION    *partition
)
{
    GT_U32  theWord=0;
    GT_U32  theBit=0;
    GT_U32  sumOfFreeBlocksInPartition=0;
    GT_U32  sizeOfFreeElement=0;
    GT_DMM_BLOCK    *foundBlock = NULL;

    /* go over all word and sum the empty blocks sizes */
    for (theWord=0;theWord<DMM_EMPTINESS_ARRAY_SIZE;theWord++)
    {
        /*Find non zero word*/
        if( partition->emptinessBitArray[theWord] != 0 )
        {
            /* we found an element in the emptinessBitArray that have an empty block flag on */
            /* find the size of the first empty block */
            for (theBit=0;theBit<32;theBit++) /* 32 bits in an word */
            {
                if( (partition->emptinessBitArray[theWord] & maskArray[theBit]) != 0)
                {
                    sizeOfFreeElement = (theWord << 5) | theBit;
                    if (sizeOfFreeElement!=0)
                    {
                        foundBlock = (partition->tableOfSizePointers[sizeOfFreeElement]);
                        /* double check that the block is free, then go over all the list
                           of same size of free blocks and sum the sizes */
                         if((DMM_BLOCK_STATUS(foundBlock) != DMM_BLOCK_FREE) ||
                            (DMM_BLOCK_STAMP(foundBlock) != DMM_STAMP_FREE))
                         {
                             /* continue to next bit */
                             continue;
                         }
                         else
                         {
                             /* go over the list of all free blocks with this size and sum it*/
                             sumOfFreeBlocksInPartition += SIZE_IN_WORDS(foundBlock);
                             foundBlock = foundBlock->nextBySizeOrPartitionPtr.nextBySize;

                             while (foundBlock != NULL)
                             {
                                  sumOfFreeBlocksInPartition += SIZE_IN_WORDS(foundBlock);
                                  foundBlock = foundBlock->nextBySizeOrPartitionPtr.nextBySize;
                             }
                         }
                    }
                }
            }
        }
    }

    return sumOfFreeBlocksInPartition;
}

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
)
{
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_U32              sumOfFreeBlocksInPartition;


    if( (partition == 0) ||
        (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
    {
        return 0;
    }

    if((newBlockSize > partition->maxBlockSizeInWords)||
       (newBlockSize < partition->minBlockSizeInWords))
    {
        return 0;
    }

    /* find if the partition have free space to fit the newBlockSize.
       space can be found as free block or as a sum of small free blocks */
    if( newBlockSize <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED )
    {
        if(DMM_EXIST_EXACT_FIT(partition,newBlockSize))
        {
            /* this should never happen since this function should not be called
               in case of exact fit exist, but in case it was called we
               will return GT_TRUE */
            return newBlockSize;
        }

        /* there is no exact fit --> check if shrink will help to find space */
        sumOfFreeBlocksInPartition = dmmGetEmptyMemoryLeft(partition);
        if (*useSwapAreaPtr==GT_TRUE)
        {
             if (sumOfFreeBlocksInPartition>=newBlockSize)
             {
                 /* specify that swap area is not a must for shrink operation */
                 *useSwapAreaPtr=GT_FALSE;
                 return sumOfFreeBlocksInPartition;
             }
             else
             {
                 /* add oldBlockSize to the amount we found */
                 sumOfFreeBlocksInPartition += oldBlockSize;
             }
        }
        if (sumOfFreeBlocksInPartition>=newBlockSize)
        {
            return sumOfFreeBlocksInPartition;
        }
    }
    return 0;
}

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
    IN    GT_UINTPTR     partitionId,
    IN    GT_U32         protocol
)
{
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_U32              sumOfAllocatedLines=0; /* sum of all allocated memory in
                                                  the partition in lines representation */
    GT_DMM_BLOCK        *tempPointedFirstBlock;/* use for going over the list of all
                                                  occupied and free memories starting
                                                  from the first block in the list.*/
    if( (partition == 0) ||
        (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
    {
        return 0;
    }
    /* ipv4=0, ipv6=1, fcoe=2 */
    if (protocol>2)
    {
        cpssOsPrintf("\n protocol=%d not valid in prvCpssDmmGetAllocatedLinesForGivenPartitionAndProtocol\n",protocol);
        return 0;
    }
    tempPointedFirstBlock = partition->pointedFirstBlock;
    while (tempPointedFirstBlock!=NULL)
    {
        if ((DMM_BLOCK_STATUS(tempPointedFirstBlock) == DMM_BLOCK_ALLOCATED)&&
             (GET_DMM_BLOCK_PROTOCOL(tempPointedFirstBlock)== protocol))
        {
            sumOfAllocatedLines+=SIZE_IN_WORDS(tempPointedFirstBlock);
        }
        tempPointedFirstBlock = tempPointedFirstBlock->nextByAddr;
    }
    return sumOfAllocatedLines;
}

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
)
{
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_U32              sumOfFreeLines=0;      /* sum of all free memory in
                                                  the partition in lines representation */
    GT_DMM_BLOCK        *tempPointedFirstBlock;/* use for going over the list of all
                                                  occupied and free memories starting
                                                  from the first block in the list.*/
    if( (partition == 0) ||
        (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
    {
        return 0;
    }

    tempPointedFirstBlock = partition->pointedFirstBlock;
    while (tempPointedFirstBlock!=NULL)
    {
        if (DMM_BLOCK_STATUS(tempPointedFirstBlock) == DMM_BLOCK_FREE)
        {
            sumOfFreeLines+=SIZE_IN_WORDS(tempPointedFirstBlock);
        }
        tempPointedFirstBlock = tempPointedFirstBlock->nextByAddr;
    }
    return sumOfFreeLines;
}

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
)
{
    GT_DMM_PARTITION    *partition = (GT_DMM_PARTITION *)partitionId;
    GT_DMM_BLOCK        *tempPointedFirstBlock;/* use for going over the list of all
                                                  occupied and free memories starting
                                                  from the first block in the list.*/
    if( (partition == 0) ||
        (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    *allUpdatedInHwFlagPtr=GT_TRUE;

    tempPointedFirstBlock = partition->pointedFirstBlock;
    while (tempPointedFirstBlock!=NULL)
    {
        if ((DMM_BLOCK_STATUS(tempPointedFirstBlock) == DMM_BLOCK_ALLOCATED)&&
            (GET_DMM_BUCKET_SW_ADDRESS(tempPointedFirstBlock)==0)) /* SW address is different than 0 only after HW set */
        {
            *allUpdatedInHwFlagPtr=GT_FALSE;
            break;
        }
        tempPointedFirstBlock = tempPointedFirstBlock->nextByAddr;
    }
    return GT_OK;
}
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
)
{
    GT_DMM_PARTITION                *partition = (GT_DMM_PARTITION *)partitionId;
    GT_DMM_SORTED_PARTITION_ELEMENT *sortedPartitionElemPtr=NULL;
    GT_U32                          i=0;


    if( (partition == 0) ||
        (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
    {
        return 0;
    }

    /* go over the array of linked list according to the blocks size */
    for (i=1;i<(DMM_MAXIMAL_BLOCK_SIZE_ALLOWED+1);i++)
    {
        while (partition->tableOfSortedUsedBlocksPointers[i] != NULL)
        {
            sortedPartitionElemPtr = partition->tableOfSortedUsedBlocksPointers[i];
            partition->tableOfSortedUsedBlocksPointers[i] = partition->tableOfSortedUsedBlocksPointers[i]->nextSortedElemPtr;
            cpssOsFree(sortedPartitionElemPtr);
        }
    }
    return GT_OK;
}

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
)
{
    GT_DMM_PARTITION                *partition = (GT_DMM_PARTITION *)partitionId;
    GT_DMM_BLOCK                    *pointedBlockPtr;
    GT_DMM_SORTED_PARTITION_ELEMENT *sortedPartitionElemPtr=NULL;
    GT_U32                          sizeOfBlock=0;


    if( (partition == 0) ||
        (partition == (GT_DMM_PARTITION*)((GT_UINTPTR)0xFFFFFFFE)))
    {
        return 0;
    }
    /* ipv4=0, ipv6=1, fcoe=2 */
    if (protocol>2)
    {
        cpssOsPrintf("\n protocol=%d not valid in prvCpssDmmSortPartition\n",protocol);
        return 0;
    }

    /* go over all used block in the partition and add them to an
       array of linked list according to the blocks size */
    pointedBlockPtr = partition->pointedFirstBlock;
    while (pointedBlockPtr!=NULL)
    {
        if((DMM_BLOCK_STATUS(pointedBlockPtr) == DMM_BLOCK_ALLOCATED)&&
           (GET_DMM_BLOCK_PROTOCOL(pointedBlockPtr)==protocol))
        {
            sizeOfBlock = SIZE_IN_WORDS(pointedBlockPtr);

            sortedPartitionElemPtr =  (GT_DMM_SORTED_PARTITION_ELEMENT *)cpssOsMalloc(sizeof(GT_DMM_SORTED_PARTITION_ELEMENT));
            if (sortedPartitionElemPtr == NULL)
            {
                /* free all allocated elements */
                prvCpssDmmFreeSortedPartitionArray(partitionId);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            cpssOsMemSet(sortedPartitionElemPtr,0,sizeof(sortedPartitionElemPtr));

            sortedPartitionElemPtr->blockElementPtr = pointedBlockPtr;
            sortedPartitionElemPtr->nextSortedElemPtr = partition->tableOfSortedUsedBlocksPointers[sizeOfBlock];
            partition->tableOfSortedUsedBlocksPointers[sizeOfBlock] = sortedPartitionElemPtr;
        }
        pointedBlockPtr = pointedBlockPtr->nextByAddr;
    }

    return GT_OK;
}


#ifdef  DMM_DEBUG_ENABLE
GT_STATUS dmmDebugStress
(
    GT_U32  sizeOfPartition,
    GT_U32  minBlock,
    GT_U32  maxBlock,
    GT_U8   strategy
)
{
    GT_UINTPTR pId;
    GT_UINTPTR allocatedTable[1024], i, j, stam, mm;
    GT_U32 loopId;
    GT_U32 allocatedBlocks = 1;
    GT_DMM_BLOCK *block;
    GT_U32       *memoryMap;
    GT_U32          existAllocBlocks,existBlocks,prevExistBlocks,existFreeBlocks;

    if( GT_OK != prvCpssDmmCreatePartition(sizeOfPartition,0,minBlock,maxBlock,
                                           &pId))
      return 1;

    memoryMap = cpssOsIpMalloc(sizeOfPartition);
    if (memoryMap == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    cpssOsMemSet(memoryMap,0,sizeOfPartition);

    for(loopId = 0; loopId < 20; loopId++)
    {
        for ( i = 0; i < 1024; i++ )
        {
            allocatedTable[i] = prvCpssDmmAllocate(pId,((osRand()%900)+3)*4 , 2);
            if((allocatedTable[i]==DMM_BLOCK_NOT_FOUND)||(allocatedTable[i]==DMM_MALLOC_FAIL))
            {
                cpssOsPrintf("Alloc Fail");
                return 26;
            }


            allocatedBlocks++;
            /*fill all allocated area*/
            for( mm = DMM_GET_OFFSET(allocatedTable[i]) / 4;
                 mm < DMM_GET_OFFSET(allocatedTable[i]) / 4 + DMM_GET_SIZE(allocatedTable[i]) / 4;
                 mm += 4)
            {
                if( memoryMap[mm] != 0 )
                {
                    cpssOsPrintf("PP mem corruption !!!");
                    return 13;
                }

                memoryMap[mm] = DMM_GET_OFFSET(allocatedTable[i]);
            }

            existBlocks = 0;
            block = (GT_DMM_BLOCK*)allocatedTable[i];

            do
            {
                existBlocks++;
                block = block->nextByAddr;
            }while( block != NULL);

            block = (GT_DMM_BLOCK*)allocatedTable[i];
            existBlocks--;
            do
            {
                existBlocks++;
                block = block->prevByAddr;
            }while( block != NULL);
            stam++;

            if((osRand()%2) == 0)
            {
                j = osRand()%(i+1);

                /*Clean and Compare Free area*/
                for( mm = DMM_GET_OFFSET(allocatedTable[j]) / 4;
                     mm < DMM_GET_OFFSET(allocatedTable[j]) / 4+
                                DMM_GET_SIZE(allocatedTable[j]) / 4;
                     mm += 4)
                {
                    if( memoryMap[mm] != DMM_GET_OFFSET(allocatedTable[j]) )
                    {
                        cpssOsPrintf("PP mem corruption !!!");
                        return 13;
                    }

                    memoryMap[mm] = 0;
                }


                if( 0 == prvCpssDmmFree(allocatedTable[j]))
                {
                    cpssOsPrintf("Free Fail");
                    return 26;
                }

                allocatedBlocks--;

            if( (loopId == 0) && ( i == 550 ) )
                stam++;

                allocatedTable[j] = prvCpssDmmAllocate(pId,((osRand()%900)+3)*4,2);
                if((allocatedTable[i]==DMM_BLOCK_NOT_FOUND)||(allocatedTable[i]==DMM_MALLOC_FAIL))
                {
                    cpssOsPrintf("Alloc Fail");
                    return 26;
                }

                allocatedBlocks++;
                /*fill all allocated area*/
                for( mm = DMM_GET_OFFSET(allocatedTable[j]) / 4;
                     mm < DMM_GET_OFFSET(allocatedTable[j]) / 4 + DMM_GET_SIZE(allocatedTable[j]) / 4;
                     mm += 4)
                {
                    if( memoryMap[mm] != 0 )
                    {
                        cpssOsPrintf("PP mem corruption !!!");
                        return 13;
                    }

                    memoryMap[mm] = DMM_GET_OFFSET(allocatedTable[j]);
                }

                existBlocks = 0;
                block = (GT_DMM_BLOCK*)allocatedTable[i];
                do
                {
                    existBlocks++;
                    block = block->nextByAddr;
                }while( block != NULL);

                    block = (GT_DMM_BLOCK*)allocatedTable[i];
                existBlocks--;
                do
                {
                    existBlocks++;
                    block = block->prevByAddr;
                }while( block != NULL);
                stam++;
            }
        }
        i--;

        for ( ; ; i-- )
        {
            /*Clean and Compare Free area*/
            for( mm = DMM_GET_OFFSET(allocatedTable[i]) / 4;
                 mm < DMM_GET_OFFSET(allocatedTable[i]) / 4 + DMM_GET_SIZE(allocatedTable[i]) / 4;
                 mm += 4)
            {
                if( memoryMap[mm] != DMM_GET_OFFSET(allocatedTable[i]) )
                {
                    cpssOsPrintf("PP mem corruption !!!");
                    return 13;
                }

                memoryMap[mm] = 0;
            }

            if( 0 == prvCpssDmmFree(allocatedTable[i]))
            {
                cpssOsPrintf("Free Fail");
                return 26;
            }
            allocatedBlocks--;

                existBlocks = existAllocBlocks = existFreeBlocks = 0;
                block = (GT_DMM_BLOCK*)allocatedTable[i];
                do
                {
                    existBlocks++;
                    if(DMM_BLOCK_STATUS(block) == DMM_BLOCK_FREE)
                    {
                        existFreeBlocks++;
                    }
                    else
                    {
                        existAllocBlocks++;
                    }
                    block = block->nextByAddr;
                }while( block != NULL);

                block = (GT_DMM_BLOCK*)allocatedTable[i];
                existBlocks--;
                if(DMM_BLOCK_STATUS(block) == DMM_BLOCK_FREE)
                {
                    existFreeBlocks--;
                }
                else
                {
                    existAllocBlocks--;
                }
                do
                {
                    existBlocks++;
                    if(DMM_BLOCK_STATUS(block) == DMM_BLOCK_FREE)
                    {
                        existFreeBlocks++;
                    }
                    else
                    {
                        existAllocBlocks++;
                    }
                    block = block->prevByAddr;
                }while( block != NULL);

                if( prevExistBlocks - existBlocks == 2 )
                    stam++;

                prevExistBlocks = existBlocks;

            if(i == 0 )
            {
                /*dmmClosePartition(pId);*/
                break;
            }
        }
    }

    prvCpssDmmClosePartition(pId);
    return 0;
}

GT_STATUS dmmOldDebugStress
(
    GT_U32  sizeOfPartition,
    GT_U32  minBlock,
    GT_U32  maxBlock,
    GT_U8   strategy
)
{
    GT_UINTPTR pId;
    GT_UINTPTR *allocatedTable;
    GT_U32 i, j;
    GT_U32 loopId;
    GT_U32       *memoryMap;

    allocatedTable = cpssOsIpMalloc(sizeof(*allocatedTable)*1000*1024);
    if (allocatedTable == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    pId = prvCpssMemDevPoolCreate();
    if( GT_OK != prvCpssMemDevPoolAdd(pId,4,sizeOfPartition ))
      return 1;

    memoryMap = cpssOsIpMalloc(sizeOfPartition);
    if (memoryMap == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    cpssOsMemSet(memoryMap,0,sizeOfPartition);

    for(loopId = 0; loopId < 1; loopId++)
    {
        for ( i = 0; i < 1000*1024; i++ )
        {
            if( 0 == (allocatedTable[i] =
                      prvCpssMemDevPoolMalloc(pId,((osRand()%13)+3)*4 , 2)))
            {
                cpssOsPrintf("Alloc Fail");
                return 26;
            }

            if((osRand()%2) == 0)
            {
                j = osRand()%(i+1);

                if( 0 == prvCpssMemDevPoolFree(pId,allocatedTable[j]))
                {
                    cpssOsPrintf("Free Fail");
                    return 26;
                }

                if( 0 == (allocatedTable[j] =
                          prvCpssMemDevPoolMalloc(pId,((osRand()%13)+3)*4,2)))
                {
                    cpssOsPrintf("Alloc Fail");
                    return 26;
                }
            }
        }
        i--;

        for ( ; ; i-- )
        {
            if( 0 == prvCpssMemDevPoolFree(pId,allocatedTable[i]))
            {
                cpssOsPrintf("Free Fail");
                return 26;
            }

            if(i == 0 )
            {
                /*dmmClosePartition(pId); */
                break;
            }
        }
    }

    cpssOsIpFree(allocatedTable);
    cpssOsIpFree(memoryMap);
    return 0;
}

GT_STATUS dmmNewDebugStress
(
    GT_U32  sizeOfPartition,
    GT_U32  minBlock,
    GT_U32  maxBlock,
    GT_U8   strategy
)
{
    GT_UINTPTR pId;
    GT_UINTPTR *allocatedTable;
    GT_U32 i, j, stam;
    GT_U32 loopId;
    GT_U32 *memoryMap;

    allocatedTable = cpssOsIpMalloc(sizeof(*allocatedTable)*1000*1024);
    if (allocatedTable == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    if( GT_OK != prvCpssDmmCreatePartition(sizeOfPartition,0,minBlock,maxBlock,
                                           &pId))
      return 1;

    memoryMap = cpssOsIpMalloc(sizeOfPartition);
    if (memoryMap == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    cpssOsMemSet(memoryMap,0,sizeOfPartition);

    for(loopId = 0; loopId < 1; loopId++)
    {
        for ( i = 0; i < 1000*1024; i++ )
        {
            allocatedTable[i] = prvCpssDmmAllocate(pId,((osRand()%13)+3)*4 , 2);
            if((allocatedTable[i]==DMM_BLOCK_NOT_FOUND)||(allocatedTable[i]==DMM_MALLOC_FAIL))
            {
                cpssOsPrintf("Alloc Fail");
                return 26;
            }

            if((osRand()%2) == 0)
            {
                j = osRand()%(i+1);

                if( 0 == prvCpssDmmFree(allocatedTable[j]))
                {
                    cpssOsPrintf("Free Fail");
                    return 26;
                }

                if( i == 537334 )
                    stam++;

                allocatedTable[j] = prvCpssDmmAllocate(pId,((osRand()%13)+3)*4,2);
                if((allocatedTable[i]==DMM_BLOCK_NOT_FOUND)||(allocatedTable[i]==DMM_MALLOC_FAIL))
                {
                    cpssOsPrintf("Alloc Fail");
                    return 26;
                }
            }
        }
        i--;

        for ( ; ; i-- )
        {
            if( 0 == prvCpssDmmFree(allocatedTable[i]))
            {
                cpssOsPrintf("Free Fail");
                return 26;
            }
            if(i == 0 )
            {
                /*prvCpssDmmClosePartition(pId); */
                break;
            }
        }
    }

    prvCpssDmmClosePartition(pId);
    cpssOsIpFree(allocatedTable);
    cpssOsIpFree(memoryMap);
    return 0;
}

#endif /*DMM_DEBUG_ENABLE*/

