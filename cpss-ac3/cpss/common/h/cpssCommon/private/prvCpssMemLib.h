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
* @file prvCpssMemLib.h
*
* @brief Implementation of a memory management unit, for managing all PP's inte-
* rnal memory for the use of tables and internal structures manipulation
* functions.
*
* This implementation provides the following functions for the user:
*
* -  prvCpssMemDevPoolCreate(void): Creates a new memory pool and returns
* it's Id, for further allocations / deallocations.
*
* -  prvCpssMemDevPoolAdd(poolId,memPtr,memSize): Adds a block to a
* previously created memory pool.
*
* -  prvCpssMemDevPoolMalloc(poolId,size,bitAllign): Allocates a block of
* memory from the given pool. The allocated block is alligned by the
* desired bit allignment.
*
* -  prvCpssMemDevPoolFree(poolId,ptr): Frees a previously allocated
* memory block, returns the number of freed bytes.
*
* -  prvCpssMemDevPoolStatus(poolId,allocatedSize,freeSize): This is a
* stat. function, which calculates the total free / allocated bytes
* remaining in / allocated from the given pool.
*
* In order that the user will be able to use the above functionality, the
* file "prvCpssMemLib.h" must be included.
*
* @version   2
********************************************************************************
*/

#ifndef __prvCpssMemLibh
#define __prvCpssMemLibh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>


typedef struct memData
{
    GT_U32  status;         /* FREEPTR==0 or ALLOCATED==1 (first search key)       */
    GT_UINTPTR  ptr;            /* pointer to the memory area (second search key)*/
    GT_U32  size;           /* the size of the memory area (in bytes)        */
}STRUCT_MEM_DATA;



/**
* @internal prvCpssMemDevPoolCreate function
* @endinternal
*
* @brief   Creates a new memory pool and returns its Id.
*/
GT_UINTPTR prvCpssMemDevPoolCreate(void);



/**
* @internal prvCpssMemDevPoolAdd function
* @endinternal
*
* @brief   Adds a new memory block to the given pool.
*
* @param[in] poolId                   - The Id of the memory pool to add to.
* @param[in] memPtr                   - a pointer to memory pool to be added.
* @param[in] memSize                  the size (in bytes) of the memory pool to be added.
*                                       GT_OK if the block adding succeeded, GT_FAIL otherwise.
*/
GT_STATUS prvCpssMemDevPoolAdd
(
    IN GT_UINTPTR poolId,
    IN GT_UINTPTR memPtr,
    IN GT_U32 memSize
);


/**
* @internal prvCpssMemDevPoolMallocLowest function
* @endinternal
*
* @brief   Allocates a lowest free memory block of given size from the pool.
*
* @param[in] poolId                   - the Id of the memory pool to allocae from.
* @param[in] size                     - the  (in bytes) of the memory pool to be allocated.
* @param[in] bitAllign                - the memory area allignment in bits.
*                                       The address of the allocated area, or NULL (0) if failed.
*/
GT_UINTPTR prvCpssMemDevPoolMallocLowest
(
    IN GT_UINTPTR poolId,
    IN GT_U32 size,
    IN GT_U32 bitAllign
);

/**
* @internal prvCpssMemDevPoolMallocHighest function
* @endinternal
*
* @brief   Allocates a highest free memory block of given size from the pool.
*
* @param[in] poolId                   - the Id of the memory pool to allocae from.
* @param[in] size                     - the  (in bytes) of the memory pool to be allocated.
* @param[in] bitAllign                - the memory area allignment in bits.
*                                       The address of the allocated area, or NULL (0) if failed.
*/
GT_UINTPTR prvCpssMemDevPoolMallocHighest
(
    IN GT_UINTPTR poolId,
    IN GT_U32 size,
    IN GT_U32 bitAllign
);

/**
* @internal prvCpssMemDevPoolMalloc function
* @endinternal
*
* @brief   Allocates a memory block from the pool.
*
* @param[in] poolId                   - the Id of the memory pool to allocae from.
* @param[in] size                     - the  (in bytes) of the memory pool to be allocated.
* @param[in] bitAllign                - the memory area allignment in bits.
*                                       The address of the allocated area, or NULL (0) if failed.
*/
GT_UINTPTR prvCpssMemDevPoolMalloc
(
    IN GT_UINTPTR poolId,
    IN GT_U32 size,
    IN GT_U32 bitAllign
);

/**
* @internal prvCpssMemDevPoolFree function
* @endinternal
*
* @brief   Frees a memory block that was previously allocated from the pool.
*
* @param[in] poolId                   - the Id of the memory pool.
* @param[in] ptr                      - pointer to the memory block to be freed.
*                                       The size of the freed area, or 0 if none.
*/
GT_U32 prvCpssMemDevPoolFree
(
    IN GT_UINTPTR poolId,
    IN GT_UINTPTR ptr
);

/**
* @internal prvCpssMemDevPoolClear function
* @endinternal
*
* @brief   Frees a all the memory blocks that were previously allocated from the
*         pool.
* @param[in] poolId                   - the Id of the memory pool.
*
* @retval GT_OK                    - if succeeded.
*/
GT_STATUS prvCpssMemDevPoolClear
(
    IN GT_UINTPTR poolId
);



/**
* @internal prvCpssMemDevPoolStatus function
* @endinternal
*
* @brief   This function returns the number of allocated & free bytes in a given
*         memory pool.
* @param[in] poolId                   - The  to check the status for.
*
* @param[out] allocatedSize            - Number of allocated bytes.
* @param[out] freeSize                 - Number of free bytes.
*                                       GT_OK
*/
GT_STATUS prvCpssMemDevPoolStatus
(
    IN GT_UINTPTR poolId,
    IN GT_U32 *allocatedSize,
    OUT GT_U32 *freeSize
);
/**
* @internal prvCpssMemDevPoolMove function
* @endinternal
*
* @brief   This function moves a block from ptr1 to ptr2.
*
* @param[in] poolId                   - The  to check the status for.
* @param[in] ptr1                     -    pointer to the src address
* @param[in] ptr2                     -    pointer to the dst address
*                                       GT_OK
*
* @note This function will not allocate new buffers and assumes both buffers
*       exist.It will only be possible to move to buffer in ptr2
*       if it has enough place or it will
*       overlap buffer in ptr1.
*
*/
GT_U32 prvCpssMemDevPoolMove
(
    IN GT_UINTPTR poolId,
    IN GT_U32 ptr1,  /*src*/
    IN GT_U32 ptr2  /*dst*/
);

/**
* @internal prvCpssMemDevPoolBlocksStatus function
* @endinternal
*
* @brief   This function return blocks status in the mwmory pool.
*
* @param[in] poolId                   - The  to check the status for.
*
* @param[out] numberOfBlocks           - number of allocated blocks.
* @param[out] largestConsecutiveBlock  - the largest consecutive block.
* @param[out] numberOfFreeMemBlocks    - number of free memory blocks.
*                                       GT_OK
*/
GT_STATUS prvCpssMemDevPoolBlocksStatus
(
    IN  GT_UINTPTR poolId,
    OUT GT_U32 *numberOfBlocks,
    OUT GT_U32 *largestConsecutiveBlock,
    OUT GT_U32 *numberOfFreeMemBlocks,
    OUT GT_U32 *freeSize
);

/**
* @internal prvCpssMemDevPoolForceAllocated function
* @endinternal
*
* @brief   Converts state of a specified free memory block from the pool
*         to "ALLOCATED".
* @param[in] poolId                   - the Id of the memory pool to allocae from.
* @param[in] ptr                      - pointer to the memory block to be allocated.
* @param[in] size                     - the  (in bytes) of the memory pool to be allocated.
*                                       GT_OK or GT_FAIL.
*/
GT_STATUS prvCpssMemDevPoolForceAllocated
(
    IN GT_UINTPTR poolId,
    IN GT_UINTPTR ptr,
    IN GT_U32 size
) ;

/**
* @internal prvCpssMemDevPoolGetSnapshot function
* @endinternal
*
* @brief   This function return memory contigous block infos,
*         sorted by base address.
* @param[in] poolId                   - The  to check the status for.
* @param[in] maxMemoBlocksNum         - size of array to get snaphot in blocks
*
* @param[out] memoBlocksNumPtr         actual amount of memory blocks
* @param[out] blokArarrayPtr           array of memory blocks infos
*                                       GT_OK on success
*/
GT_STATUS prvCpssMemDevPoolGetSnapshot
(
    IN  GT_UINTPTR       poolId,
    IN  GT_U32           maxMemoBlocksNum,
    OUT GT_U32           *memoBlocksNumPtr,
    OUT STRUCT_MEM_DATA  *blokArarrayPtr
) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssMemLibh */


