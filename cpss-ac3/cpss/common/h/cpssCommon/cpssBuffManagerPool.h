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
* @file cpssBuffManagerPool.h
*
* @brief Operating System wrapper for buffer management pool.
*
* @version   2
********************************************************************************
*/
#ifndef __cpssBuffManagerPoolh
#define __cpssBuffManagerPoolh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/************* Define *********************************************************/

#define CPSS_BM_POOL_NULL_ID  ((CPSS_BM_POOL_ID)NULL)



/************* Typedef ********************************************************/


/*
 * Typedef: CPSS_BM_POOL_ID
 *
 * Description:
 *  Define type for pool ID.
 *  The pool ID is allocated and returned to the user by the poolCreate function.
 *  In order to delet pool, allocate buffer or free buffer, the pool ID is given as
 *  parametr to those service functions.
 *
 */
typedef GT_VOID* CPSS_BM_POOL_ID;

/**
* @enum CPSS_BM_POOL_ALIGNMENT_ENT
 *
 * @brief Define the buffer alignment supported by the module.
*/
typedef enum{

    CPSS_BM_POOL_1_BYTE_ALIGNMENT_E  = 0x1,

    CPSS_BM_POOL_4_BYTE_ALIGNMENT_E  = 0x3,

    CPSS_BM_POOL_8_BYTE_ALIGNMENT_E  = 0x7,

    CPSS_BM_POOL_16_BYTE_ALIGNMENT_E = 0xF

} CPSS_BM_POOL_ALIGNMENT_ENT;



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
    IN   GT_U32                         reqBufferSize,
    IN   CPSS_BM_POOL_ALIGNMENT_ENT     alignment,
    IN   GT_U32                         numOfBuffers,
    OUT  CPSS_BM_POOL_ID                *pPoolId
);

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
);

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
);

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
*/
GT_STATUS cpssBmPoolDelete
(
    IN  CPSS_BM_POOL_ID  poolId
);

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
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *cpssBmPoolBufGet
(
    IN  CPSS_BM_POOL_ID poolId
);

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
);

/**
* @internal cpssBmPoolBufSizeGet function
* @endinternal
*
* @brief   Get the buffer size.
*
* @retval GT_U32                   - the buffer size.
*/
GT_U32 cpssBmPoolBufSizeGet
(
    IN        CPSS_BM_POOL_ID  poolId
);

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
);


/**
* @internal cpssBmPoolBufFreeCntGet function
* @endinternal
*
* @brief   Get the free buffer count.
*
* @retval GT_U32                   - the free buffer count.
*/
GT_U32 cpssBmPoolBufFreeCntGet
(
    IN        CPSS_BM_POOL_ID  poolId
);

/************ Private Functions ************************************************/

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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif

#endif  /* __cpssBuffManagerPoolh */



