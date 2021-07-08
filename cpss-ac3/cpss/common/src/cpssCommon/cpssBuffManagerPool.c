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
* @file cpssBuffManagerPool.c
*
* @brief Buffer management module.
*
* @version   7
********************************************************************************
*/

/********* include ************************************************************/

#include<cpss/extServices/os/gtOs/cpssOsSem.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/cpssBuffManagerPool.h>
#include <cpssCommon/cpssPresteraDefs.h>

/************ Internal Typedefs ***********************************************/

/*
 * Typedef: struct CPSS_BM_POOL_BUF_HEAD
 *
 * Description: Defines the header of a buffer in a pool.
 *
 * Fields:
 *      pNext - Pointer to the next free buffer in the pool.
 *
 */
typedef struct BufHead
{
    struct BufHead  *pNext;

}CPSS_BM_POOL_FREE_BUF_HEAD;


/*
 * Typedef: struct CPSS_BM_POOL_BLOCK_NODE
 *
 * Description: Defines the pool block node ptrs in the block's LL.
 *
 * Fields:
 *      pPoolBlock - Pointer to the allocated memory block
 *      pAlignedBlockStart - Pointer to the aligned start address
 *                           of the pool's memory block
 *      pAlignedBlockEnd   - Pointer to the aligned end address
 *                           of the pool's memory block
 *      pNext - Pointer to the next BLOCK node.
 *
 */
typedef struct _buf_block_node
{
    GT_VOID *pBlockPtr;
    GT_VOID *pPoolBlock;

    GT_VOID *pAlignedBlockStart;
    GT_VOID *pAlignedBlockEnd;
    struct _buf_block_node  *pNext;

}CPSS_BM_POOL_BLOCK_NODE;

/*
 * Typedef: struct pool_head
 *
 * Description: Defines the pool head
 *
 * Fields:
 *      pPoolBlocksHead
 *      poolAlignment    - Buffer alignment in the Pool.
 *                          GT_1_BYTE_ALIGNMENT,
 *                          GT_4_BYTE_ALIGNMENT,
 *                          GT_8_BYTE_ALIGNMENT,
 *                          GT_16_BYTE_ALIGNMENT
 *      pListHead
 *      memBlocSize      - Memory block size
 *      reqBufferSize    - requested buffer's size
 *      actBufferSize    - Actual buffer's size (after alignment)
 *      numOfBuffers     - Number of buffers in the pool
 *      numOfFreeBuffers - number of free buffers in the pool
 *      numOfRequest     - Total number of buffers requested from the pool
 *      numOfReturn      - Total number of buffers returned to the pool
 *      magic            - Protection
 *      forDma           - is this pool is used for dma (physically contigious)
 */
typedef struct pool_head
{
    CPSS_BM_POOL_BLOCK_NODE     *pPoolBlocksHead;
    GT_U32                      poolAlignment;
    CPSS_BM_POOL_FREE_BUF_HEAD  *pListHead;
    GT_U32                      memBlocSize;
    GT_U32                      reqBufferSize;
    GT_U32                      actBufferSize;
    GT_U32                      numOfBuffers;
    GT_U32                      numOfFreeBuffers;
    GT_U32                      numOfRequest;
    GT_U32                      numOfReturn;
    GT_U32                      magic;
    GT_BOOL                     forDma;

}CPSS_BM_POOL_HEAD;

#define  CPSS_BM_POOL_HEAD_MAGIC  0x5A5A5A00

/*************** Global ******************************************************/


/************ Private Functions ************************************************/

void cpssBmPoolPrintfInfo
(
    IN  CPSS_BM_POOL_ID poolId
);


/************ Public Functions ************************************************/

/**
* @internal internalPoolCreatePool function
* @endinternal
*
* @brief   Create pool of buffers. All buffers have the same size.
*
* @param[in] forDma                   - is physically contigious.
*                                      bufferSize   - Buffer's size
* @param[in] alignment                - Buffers' alignment
* @param[in] numOfBuffers             - number of buffer at the pool
*
* @param[out] pPoolId                  - Return the pool ID
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
* @note Usage of this function is only during FIRST initialization.
*
*/
static GT_STATUS internalPoolCreatePool
(
    IN   GT_U32                     reqBufferSize,
    IN   CPSS_BM_POOL_ALIGNMENT_ENT alignment,
    IN   GT_U32                     numOfBuffers,
    IN   GT_BOOL                    forDma,
    OUT  CPSS_BM_POOL_ID            *pPoolId
)
{
    GT_U32                      actBufferSize, poolSize;
    GT_U32                      indx;
    CPSS_BM_POOL_FREE_BUF_HEAD  *pBuf = NULL;
    GT_U8                       *pBytePtr;
    /* created pool */
    CPSS_BM_POOL_HEAD           *pHead;

    /* Zero buffer in the pool is not allowed.
     * Size smaller than the size of next pointer is not allowed.
     */
    if ( (numOfBuffers == 0) ||
         (reqBufferSize < sizeof(struct BufHead*)) )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    if (pPoolId == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    /* Buffer alignment in the Pool. */
    if( alignment != CPSS_BM_POOL_1_BYTE_ALIGNMENT_E &&
        alignment != CPSS_BM_POOL_4_BYTE_ALIGNMENT_E &&
        alignment != CPSS_BM_POOL_8_BYTE_ALIGNMENT_E &&
        alignment != CPSS_BM_POOL_16_BYTE_ALIGNMENT_E
        )
    {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }
    pHead = cpssOsMalloc(sizeof(CPSS_BM_POOL_HEAD));
    if (NULL == pHead)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);

    /* Calculate the real size of the buffer, depend on the user request,
     * alignment, and buffer header.
     */
    pHead->poolAlignment    = alignment;
    actBufferSize = (reqBufferSize + alignment) & (~(GT_UINTPTR)alignment);

    /* calculate the memory size needed for the pool */
    poolSize = numOfBuffers * actBufferSize;

    pHead->reqBufferSize    = reqBufferSize;
    pHead->actBufferSize    = actBufferSize;
    pHead->numOfBuffers     = numOfBuffers;
    pHead->numOfFreeBuffers = numOfBuffers;
    pHead->numOfRequest     = 0;
    pHead->numOfReturn      = 0;
    pHead->magic            = CPSS_BM_POOL_HEAD_MAGIC;
    pHead->forDma           = forDma;
    /* Allocate memory for the pool.
     * Allocated size is pool's size plus extra
     * bytes that might be needed for alignment.
     */
    if (forDma)
    {
        pBytePtr = cpssOsCacheDmaMalloc(poolSize + alignment);
    }
    else
    {
        pBytePtr = cpssOsMalloc(poolSize + alignment);
    }
    if (pBytePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* initialize the blocks link list */
    pHead->pPoolBlocksHead = cpssOsMalloc(sizeof(CPSS_BM_POOL_BLOCK_NODE));
    if (pHead->pPoolBlocksHead == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    pHead->pPoolBlocksHead->pNext = NULL;

    /* Save the block memory address and its size */
    pHead->pPoolBlocksHead->pPoolBlock = pBytePtr;

    pHead->memBlocSize = poolSize + alignment;

    /* Align the memory block */
    pBytePtr = (GT_U8*)((GT_UINTPTR)(pBytePtr + alignment) & (~((GT_UINTPTR)alignment)));

    /* Store the memory block boundaries*/
    pHead->pPoolBlocksHead->pAlignedBlockStart = pBytePtr;
    pHead->pPoolBlocksHead->pAlignedBlockEnd = pBytePtr + poolSize - 1;

    /* The first buffer is located in the beginning of the memory block */
    pHead->pListHead = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;

     /* Divide the memory block into buffers and make it as a link list */
    for (indx=0; indx < numOfBuffers; indx++)
    {
        pBuf = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;
        pBytePtr = pBytePtr + actBufferSize;
        pBuf->pNext = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;
    }
    /* Last buffer is pointing to NULL */
    pBuf->pNext = NULL;

    /* Set the return pool id. */
    *(CPSS_BM_POOL_HEAD **)pPoolId = pHead;

    return(GT_OK);
}

/**
* @internal cpssBmPoolCreate function
* @endinternal
*
* @brief   Create pool of buffers. All buffers have the same size.
*
* @param[out] pPoolId                  - Return the pool ID
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
* @note Usage of this function is only during FIRST initialization.
*
*/
GT_STATUS cpssBmPoolCreate
(
    IN   GT_U32                     reqBufferSize,
    IN   CPSS_BM_POOL_ALIGNMENT_ENT alignment,
    IN   GT_U32                     numOfBuffers,
    OUT  CPSS_BM_POOL_ID            *pPoolId
)
{
    return internalPoolCreatePool(reqBufferSize,alignment,numOfBuffers,GT_FALSE,pPoolId);
}
/**
* @internal cpssBmPoolCreateDma function
* @endinternal
*
* @brief   Create pool of buffers. All buffers have the same size.
*         The buffers are created in a phsycally contigious area.
*
* @param[out] pPoolId                  - Return the pool ID
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
* @note Usage of this function is only during FIRST initialization.
*
*/
GT_STATUS cpssBmPoolCreateDma
(
    IN   GT_U32                     reqBufferSize,
    IN   CPSS_BM_POOL_ALIGNMENT_ENT alignment,
    IN   GT_U32                     numOfBuffers,
    OUT  CPSS_BM_POOL_ID            *pPoolId
)
{
    return internalPoolCreatePool(reqBufferSize,alignment,numOfBuffers,GT_TRUE,pPoolId);
}
/**
* @internal cpssBmPoolReCreate function
* @endinternal
*
* @brief   Create pool of buffers. All buffers have the same size.
*
* @param[in] poolId                   - Id of the pool to be re-created.
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Bad poolId
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
* @note Usage of this function is only during FIRST initialization.
*
*/
GT_STATUS cpssBmPoolReCreate
(
    IN  CPSS_BM_POOL_ID  poolId
)
{
    GT_U32                          numOfBuffers;
    GT_U32                          indx;
    CPSS_BM_POOL_FREE_BUF_HEAD      *pBuf = NULL;
    GT_U8                           *pBytePtr;
    CPSS_BM_POOL_HEAD               *pHead;
    CPSS_BM_POOL_BLOCK_NODE         *pBlockNode;


    pHead = (CPSS_BM_POOL_HEAD *)poolId;
    if(pHead == NULL)
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    if(pHead->magic != CPSS_BM_POOL_HEAD_MAGIC)
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);


    numOfBuffers = pHead->numOfBuffers ;
    pHead->numOfFreeBuffers = numOfBuffers;
    pHead->numOfRequest     = 0;
    pHead->numOfReturn      = 0;

    pBlockNode = pHead->pPoolBlocksHead;
    /* Store the memory block boundaries*/
    pBytePtr = pBlockNode->pAlignedBlockStart;

    /* The first buffer is located in the beginning of the memory block */
    pHead->pListHead = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;

     /* Divide the memory block into buffers and make it as a link list */
    for (indx=0; indx < numOfBuffers; indx++)
    {
        pBuf = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;
        pBytePtr = pBytePtr + pHead->actBufferSize;
        if ((pBytePtr > (GT_U8*)pBlockNode->pAlignedBlockEnd)
            && (indx < (numOfBuffers-1)))
        {
            /* boundry check*/
            if (pBlockNode->pNext == NULL)
            {
                /*shouldn't happen*/
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            pBlockNode = pBlockNode->pNext;
            pBytePtr = pBlockNode->pAlignedBlockStart;
        }
        pBuf->pNext = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;
    }
    /* Last buffer is pointing to NULL */
    pBuf->pNext = NULL;

    return(GT_OK);
}

/**
* @internal cpssBmPoolDelete function
* @endinternal
*
* @brief   Delete a pool and free all the memory occupied by that pool.
*         The operation is failed if not all the buffers belong to that pool
*         have been freed.
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_STATE             - Not all buffers belong to that pool have
*                                       been freed.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*/
GT_STATUS cpssBmPoolDelete
(
    IN  CPSS_BM_POOL_ID  poolId
)
{
    CPSS_BM_POOL_BLOCK_NODE       *ptr;
    CPSS_BM_POOL_BLOCK_NODE       *freePtr;
    CPSS_BM_POOL_HEAD  *pHead = (CPSS_BM_POOL_HEAD *)poolId;
    if(pHead == NULL)
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    /* If not all the buffers have been freed, return with error */
    if (pHead->numOfFreeBuffers != pHead->numOfBuffers)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /* free pool's memory block */
    ptr = pHead->pPoolBlocksHead;
    while (ptr != NULL)
    {
        if (pHead->forDma)
        {

            cpssOsCacheDmaFree(ptr->pPoolBlock);
        }
        else
        {
            cpssOsFree(ptr->pPoolBlock);
        }
        freePtr = ptr;
        ptr = ptr->pNext;
        cpssOsFree(freePtr);
    }
    /* free pool ID */
    cpssOsFree(pHead);

    return(GT_OK);
}

/*******************************************************************************
* cpssBmPoolBufGet
*
* DESCRIPTION:
*           Return pointer to a buffer from the requested pool
*
* INPUTS:
*           PoolId - The pool ID as returned by the create function.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_VOID* - Pointer to the new allocated buffer. NULL is returned in case
*               no buffer is not available.
*       GT_NOT_INITIALIZED - In case of un-initialized package.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *cpssBmPoolBufGet
(
    IN  CPSS_BM_POOL_ID poolId
)
{

    CPSS_BM_POOL_FREE_BUF_HEAD *pBuf;
    CPSS_BM_POOL_HEAD          *pHead = (CPSS_BM_POOL_HEAD *)poolId;

    /* Take the buffer from the head of the list */
    pBuf = pHead->pListHead;
    if (pBuf != NULL) /* if the list is not empty, update the header */
    {
        pHead->pListHead = pBuf->pNext;

        /* Update number of free buffers */
        pHead->numOfFreeBuffers--;
    }

    return(pBuf);
}

/**
* @internal cpssBmPoolBufFree function
* @endinternal
*
* @brief   Free a buffer back to its pool.
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_PTR               - The returned buffer is not belongs to that pool
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*/
GT_STATUS cpssBmPoolBufFree
(
    IN  CPSS_BM_POOL_ID poolId,
    IN  GT_VOID   *pBuf
)
{
    CPSS_BM_POOL_FREE_BUF_HEAD   *pRetbuf;
    GT_U8                   *pBlocStart, *pBlockEnd;
    CPSS_BM_POOL_HEAD            *pHead = (CPSS_BM_POOL_HEAD *)poolId;
    CPSS_BM_POOL_BLOCK_NODE      *pBlockNode;

    if(pHead == NULL)
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    pBlockNode = pHead->pPoolBlocksHead;

    while (pBlockNode != NULL)
    {
        pBlocStart = pBlockNode->pAlignedBlockStart;
        pBlockEnd = pBlockNode->pAlignedBlockEnd;


        /* Test if the returned buffer belongs to that pool */
        if ( ( (GT_U8*)pBuf >= pBlocStart ) &&           /* in the pool boundaries */
             ( (GT_U8*)pBuf <= pBlockEnd )  &&
             ( ((GT_UINTPTR)pBuf & pHead->poolAlignment) == 0 )/* ...and aligned  */
           )
            break;

        pBlockNode = pBlockNode->pNext;
    }

    if (pBlockNode == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    /* Insert the returned buffer to the head of the list */
    pRetbuf = pBuf;
    pRetbuf->pNext = pHead->pListHead;
    pHead->pListHead = pRetbuf;

    /* Update number of free buffers */
    pHead->numOfFreeBuffers++;

    return(GT_OK);
}


/**
* @internal cpssBmPoolBufSizeGet function
* @endinternal
*
* @brief   Get the buffer size.
*
* @retval GT_U32                   - the buffer size.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*/
GT_U32 cpssBmPoolBufSizeGet
(
    IN        CPSS_BM_POOL_ID  poolId
)
{
    CPSS_BM_POOL_HEAD    *pHead = (CPSS_BM_POOL_HEAD *)poolId;
    if(pHead == NULL)
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    return pHead->reqBufferSize;
}

/**
* @internal cpssBmPoolExpand function
* @endinternal
*
* @brief   Expand a pool of buffers. All buffers have the same size.
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - Request for zero number of buffers or
*                                       buffer's size is smaller than the size of
*                                       pointer (usually 4 bytes)
* @retval GT_BAD_PTR               - Bad pointer for the pPoolId
* @retval GT_NO_RESOURCE           - No memory is available to create the pool.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*
* @note not used for Dma buffers!
*
*/
GT_STATUS cpssBmPoolExpand
(
    IN   CPSS_BM_POOL_ID        poolId,
    IN   GT_U32            numOfBuffers
)
{
    GT_U32                          actBufferSize, addedPoolSize;
    GT_U32                          indx;
    CPSS_BM_POOL_FREE_BUF_HEAD      *pBuf = NULL;
    GT_U8                           *pBytePtr;
    CPSS_BM_POOL_ALIGNMENT_ENT      alignment;
    CPSS_BM_POOL_BLOCK_NODE         *pBlockNode;
    CPSS_BM_POOL_FREE_BUF_HEAD      *lastFirstBuf;
    CPSS_BM_POOL_HEAD               *pHead = (CPSS_BM_POOL_HEAD *)poolId;

    if(pHead == NULL)
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    if (pHead->forDma == GT_TRUE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);


    /* Zero buffer in the pool is not allowed.
     * Size smaller than the size of next pointer is not allowed.
     */
    if (numOfBuffers == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* Calculate the real size of the buffer, depend on the user request,
     * alignment, and buffer header.
     */
    alignment = pHead->poolAlignment;
    actBufferSize = pHead->actBufferSize;

    /* calculate the memory size needed for the pool */
    addedPoolSize = numOfBuffers * pHead->actBufferSize;


    /* Allocate memory for the pool.
     * Allocated size is pool's size plus extra
     * bytes that might be needed for alignment.
     */
    pBytePtr = cpssOsMalloc(addedPoolSize + alignment);
    if (pBytePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    pHead->numOfBuffers     = numOfBuffers + pHead->numOfBuffers;
    pHead->numOfFreeBuffers = numOfBuffers + pHead->numOfFreeBuffers;

    /* add to the blocks link list */
    pBlockNode = pHead->pPoolBlocksHead;
    pHead->pPoolBlocksHead = cpssOsMalloc(sizeof(CPSS_BM_POOL_BLOCK_NODE));
    if (pHead->pPoolBlocksHead == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    pHead->pPoolBlocksHead->pNext = pBlockNode;

    /* Save the block memory address and its size */
    pHead->pPoolBlocksHead->pPoolBlock = pBytePtr;

    /* Align the memory block */
    pBytePtr = (GT_U8*)((GT_UINTPTR)(pBytePtr + alignment) & (~((GT_UINTPTR)alignment)));

    /* Store the memory block boundaries*/
    pHead->pPoolBlocksHead->pAlignedBlockStart = pBytePtr;
    pHead->pPoolBlocksHead->pAlignedBlockEnd = pBytePtr + addedPoolSize - 1;

    /* The first buffer is located in the beginning of the memory block */
    lastFirstBuf = pHead->pListHead;
    pHead->pListHead = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;

     /* Divide the memory block into buffers and make it as a link list */
    for (indx=0; indx < numOfBuffers; indx++)
    {
        pBuf = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;
        pBytePtr = pBytePtr + actBufferSize;
        pBuf->pNext = (CPSS_BM_POOL_FREE_BUF_HEAD*)pBytePtr;
    }
    /* Last buffer is pointing to last first buffer */
    pBuf->pNext = lastFirstBuf;

    return(GT_OK);
}


/**
* @internal cpssBmPoolBufFreeCntGet function
* @endinternal
*
* @brief   Get the free buffer count.
*
* @retval GT_U32                   - the free buffer count.
* @retval GT_NOT_INITIALIZED       - In case of un-initialized package.
*/
GT_U32 cpssBmPoolBufFreeCntGet
(
    IN        CPSS_BM_POOL_ID  poolId
)
{
    CPSS_BM_POOL_HEAD    *pHead = (CPSS_BM_POOL_HEAD *)poolId;
    if(pHead == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    return pHead->numOfFreeBuffers;
}

/************ Private Functions ************************************************/

/**
* @internal poolInfoPrintf function
* @endinternal
*
* @brief   Helper function for cpssBmPoolPrintStats
*/
static void poolInfoPrintf
(
    IN  CPSS_BM_POOL_ID poolId
)
{
    CPSS_BM_POOL_HEAD  *pHead = (CPSS_BM_POOL_HEAD *)poolId;
    CPSS_BM_POOL_BLOCK_NODE* pBlockNode;


    pBlockNode = pHead->pPoolBlocksHead;
    while (pBlockNode != NULL)
    {

        cpssOsPrintf("\nPool's ID          : 0x%08X\n", poolId);
        cpssOsPrintf("Pool's Boundaries  : %p - %p (%d Bytes)\n",
                 pBlockNode->pPoolBlock,
                 (void*)((GT_UINTPTR)(pBlockNode->pPoolBlock) + pHead->memBlocSize - 1),
                 pHead->memBlocSize);

        cpssOsPrintf("Aligned Boundaries : %p - %p (%d Bytes)\n",
                 pBlockNode->pAlignedBlockStart,
                 pBlockNode->pAlignedBlockEnd,
                 (pHead->numOfBuffers * pHead->actBufferSize));
        pBlockNode = pBlockNode->pNext;
    }

    cpssOsPrintf("Requested size     : %d\n", pHead->reqBufferSize);
    cpssOsPrintf("Aligned size       : %d\n", pHead->actBufferSize);
    cpssOsPrintf("Number of buffers  : %d (%d buffers free)\n",
              pHead->numOfBuffers, pHead->numOfFreeBuffers);
}

/**
* @internal cpssBmPoolStatsPrint function
* @endinternal
*
* @brief   Print pool's statistics for the requested pool, or for all
*         the pools in case poolID is CPSS_BM_POOL_NULL_ID.
*/
void cpssBmPoolStatsPrint
(
    IN  CPSS_BM_POOL_ID poolId
)
{

    if (poolId != CPSS_BM_POOL_NULL_ID)
    {
       poolInfoPrintf(poolId);
    }
}

/*******************************************************************************
* cpssBmPoolDebugBufRetrieve
*
* DESCRIPTION:
*           Retrieve buffer address by index.
*           Buffer can be busy or free
*
* INPUTS:
*           PoolId   - The pool ID as returned by the create function.
*           bufIndex - index of buffer
* OUTPUTS:
*       None
*
* RETURNS:
*       Address of required buffer or NULL on too big index.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID* cpssBmPoolDebugBufRetrieve
(
    IN  CPSS_BM_POOL_ID poolId,
    IN  GT_U32          bufIndex
)
{
    GT_U8                        *pBlocStart, *pBlockEnd;
    CPSS_BM_POOL_HEAD            *pHead = (CPSS_BM_POOL_HEAD *)poolId;
    CPSS_BM_POOL_BLOCK_NODE      *pBlockNode;
    GT_U32                       blockBaseIndex;
    GT_U32                       blockSize;

    if (bufIndex >= pHead->numOfBuffers) return NULL;
    if (pHead == NULL) return NULL;

    pBlockNode     = pHead->pPoolBlocksHead;
    blockBaseIndex = 0;

    while (pBlockNode != NULL)
    {
        pBlocStart = pBlockNode->pAlignedBlockStart;
        pBlockEnd = pBlockNode->pAlignedBlockEnd;
        blockSize = (GT_U32)((pBlockEnd - pBlocStart) / pHead->actBufferSize);
        if (bufIndex < (blockBaseIndex + blockSize))
        {
            return (GT_VOID*)(pBlocStart
                 + (pHead->actBufferSize * (bufIndex - blockBaseIndex)));
        }
        pBlockNode = pBlockNode->pNext;
        blockBaseIndex += blockSize;
    }

    return NULL;
}

/**
* @internal cpssBmPoolDebugBufIndexGet function
* @endinternal
*
* @brief   Get buffer index by address.
*         Buffer can be busy or free
*/
GT_U32 cpssBmPoolDebugBufIndexGet
(
    IN  CPSS_BM_POOL_ID poolId,
    IN  GT_VOID         *bufPtr
)
{
    GT_U8                        *pBlocStart, *pBlockEnd, *pBuf;
    CPSS_BM_POOL_HEAD            *pHead = (CPSS_BM_POOL_HEAD *)poolId;
    CPSS_BM_POOL_BLOCK_NODE      *pBlockNode;
    GT_U32                       blockBaseIndex;
    GT_U32                       blockSize;

    if (bufPtr == NULL) return 0xFFFFFFFF;
    if (pHead == NULL)  return 0xFFFFFFFF;

    pBlockNode     = pHead->pPoolBlocksHead;
    blockBaseIndex = 0;
    pBuf           = (GT_U8*)bufPtr;

    while (pBlockNode != NULL)
    {
        pBlocStart = pBlockNode->pAlignedBlockStart;
        pBlockEnd = pBlockNode->pAlignedBlockEnd;
        if ((pBuf >= pBlocStart) && (pBuf < pBlockEnd))
        {
            return (GT_U32)(blockBaseIndex + ((pBuf - pBlocStart) / pHead->actBufferSize));
        }
        blockSize = (GT_U32)((pBlockEnd - pBlocStart) / pHead->actBufferSize);
        pBlockNode = pBlockNode->pNext;
        blockBaseIndex += blockSize;
    }

    return 0xFFFFFFFF;
}

/**
* @internal cpssBmPoolDebugDumpBusyMemory function
* @endinternal
*
* @brief   Dump given amount of first busy records.
*         Typically used to investigate memory leak reason.
*/
void cpssBmPoolDebugDumpBusyMemory
(
    IN  CPSS_BM_POOL_ID poolId,
    IN  GT_U32          skipBufs,
    IN  GT_U32          dumpBufs
)
{
    CPSS_BM_POOL_HEAD            *pHead = (CPSS_BM_POOL_HEAD *)poolId;
    GT_U32                       *freeBufsBitmapArr;
    GT_U32                       wordsInBitmap;
    CPSS_BM_POOL_FREE_BUF_HEAD   *pBuf;
    GT_U32                       bufIndex;
    GT_U32                       wrongBufIndexCnt;
    GT_U32                       wordIdx, zeroBitIdx;
    GT_U32                       word, mask;
    GT_U32                       skippedBufs, dumpedBufs;
    GT_U32                       *bufPtr;
    GT_U32                       requiredWordsInBuf, bufWordIdx;

    if (poolId == NULL)
    {
       cpssOsPrintf("cpssBmPoolDebugDump: Null pointer of Pool\n");
       return;
    }

    requiredWordsInBuf = ((pHead->reqBufferSize + 3) / 4);
    wordsInBitmap = ((pHead->numOfBuffers + 31) / 32);
    freeBufsBitmapArr = cpssOsMalloc(sizeof(GT_U32) * wordsInBitmap);
    if (freeBufsBitmapArr == NULL)
    {
       cpssOsPrintf("cpssBmPoolDebugDump: Bitmap Allocation failed\n");
       return;
    }
    cpssOsMemSet(freeBufsBitmapArr, 0, (sizeof(GT_U32) * wordsInBitmap));

    wrongBufIndexCnt = 0;
    for (pBuf = pHead->pListHead; (pBuf != NULL); pBuf = pBuf->pNext)
    {
        bufIndex = cpssBmPoolDebugBufIndexGet(poolId, pBuf);
        if (bufIndex == 0xFFFFFFFF)
        {
            wrongBufIndexCnt ++;
            continue;
        }
        if (freeBufsBitmapArr[bufIndex / 32] & (1 << (bufIndex % 32)))
        {
            cpssOsFree(freeBufsBitmapArr);
            cpssOsPrintf("cpssBmPoolDebugDump: Found loop in free buffers chain\n");
            return;
        }
        freeBufsBitmapArr[bufIndex / 32] |= (1 << (bufIndex % 32));
    }
    if (wrongBufIndexCnt != 0)
    {
        cpssOsPrintf("cpssBmPoolDebugDump: out of bound buffer addresses %d\n", wrongBufIndexCnt);
    }
    skippedBufs  = 0;
    dumpedBufs   = 0;
    for (wordIdx = 0; (wordIdx < wordsInBitmap); wordIdx ++)
    {
        word = freeBufsBitmapArr[wordIdx];
        zeroBitIdx = 0;
        while (word != 0xFFFFFFFF)
        {
            for (/*none*/; (zeroBitIdx < 16); zeroBitIdx += 16)
            {
                mask = (0xFFFF << zeroBitIdx);
                if ((word & mask) != mask) break;
            }
            for (/*none*/; (zeroBitIdx < 24); zeroBitIdx += 8)
            {
                mask = (0xFF << zeroBitIdx);
                if ((word & mask) != mask) break;
            }
            for (/*none*/; (zeroBitIdx < 28); zeroBitIdx += 4)
            {
                mask = (0xF << zeroBitIdx);
                if ((word & mask) != mask) break;
            }
            for (/*none*/; (zeroBitIdx < 31); zeroBitIdx ++)
            {
                mask = (1 << zeroBitIdx);
                if ((word & mask) != mask) break;
            }
            /* set 1 to zero bit being treated */
            word |= (1 << zeroBitIdx);
            if (skippedBufs < skipBufs)
            {
                skippedBufs ++;
                continue;
            }
            if (dumpedBufs > dumpBufs) break;
            dumpedBufs ++;
            /* dump buffer */
            bufIndex = ((wordIdx * 32) + zeroBitIdx);
            bufPtr = (GT_U32*)cpssBmPoolDebugBufRetrieve(poolId, bufIndex);
            cpssOsPrintf(
                "cpssBmPoolDebugDump bufIndex %d bufPtr 0x%08X\n",
                bufIndex, bufPtr);
            if (bufPtr == NULL) continue;

            for (bufWordIdx = 0; (bufWordIdx < requiredWordsInBuf); bufWordIdx++)
            {
                if ((bufWordIdx != 0) && ((bufWordIdx % 4) == 0)) cpssOsPrintf("   ");
                    if ((bufWordIdx != 0) && ((bufWordIdx % 8) == 0)) cpssOsPrintf("\n");
                cpssOsPrintf("0x%08X  ", bufPtr[bufWordIdx]);
            }
            cpssOsPrintf("\n");
        }
    }
    cpssOsFree(freeBufsBitmapArr);
}






