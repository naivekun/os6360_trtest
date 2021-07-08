/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtBmPool.h
*
* @brief Operating System wrapper for buffer management pool.
*
* @version   9
********************************************************************************
*/

#ifndef __gtBmPoolh
#define __gtBmPoolh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>


#if (!defined __cmdExtServices_h_) || (defined PSS_PRODUCT)

/************* Define *********************************************************/

#define GT_POOL_NULL_ID  ((GT_POOL_ID)NULL)



/************* Typedef ********************************************************/


/*
 * Typedef: GT_POOL_ID
 *
 * Description:
 *  Define type for pool ID.
 *  The pool ID is allocated and returned to the user by the poolCreate function.
 *  In order to delet pool, allocate buffer or free buffer, the pool ID is given as
 *  parametr to those service functions.
 *
 */
typedef GT_VOID* GT_POOL_ID;

/**
* @enum GT_POOL_ALIGNMENT
 *
 * @brief Define the buffer alignment supported by the module.
*/
typedef enum{

    GT_1_BYTE_ALIGNMENT  = 0x1,

    GT_4_BYTE_ALIGNMENT  = 0x3,

    GT_8_BYTE_ALIGNMENT  = 0x7,

    GT_16_BYTE_ALIGNMENT = 0xF

} GT_POOL_ALIGNMENT;

#endif /* __cmdExtServices_h_ */

/************* Functions ******************************************************/


/**
* @internal gtPoolInit function
* @endinternal
*
* @brief   Used to initialize the pool module
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_VALUE             - In case of bad alignment value
* @retval GT_INIT_ERROR            - In case re-initialization.
* @retval GT_NO_RESOURCE           - Not enough memory or semaphores.
*
* @note Usage of this function is only during FIRST initialization.
*
*/
GT_STATUS  gtPoolInit
(
   IN   GT_U32             maxPools
);



/**
* @internal gtPoolCreatePool function
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
GT_STATUS gtPoolCreatePool
(
    IN   GT_U32            reqBufferSize,
    IN   GT_POOL_ALIGNMENT alignment,
    IN   GT_U32            numOfBuffers,
    IN   GT_BOOL           useSem,
    OUT  GT_POOL_ID       *pPoolId
);

/**
* @internal gtPoolCreateDmaPool function
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
GT_STATUS gtPoolCreateDmaPool
(
    IN   GT_U32            reqBufferSize,
    IN   GT_POOL_ALIGNMENT alignment,
    IN   GT_U32            numOfBuffers,
    IN   GT_BOOL           useSem,
    OUT  GT_POOL_ID       *pPoolId
);

/**
* @internal gtPoolReCreatePool function
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
GT_STATUS gtPoolReCreatePool
(
    IN  GT_POOL_ID  poolId
);

/**
* @internal gtPoolDeletePool function
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
GT_STATUS gtPoolDeletePool
(
    IN  GT_POOL_ID  poolId
);

/*******************************************************************************
* gtPoolGetBuf
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
GT_VOID* gtPoolGetBuf
(
    IN        GT_POOL_ID  poolId
);


/**
* @internal gtPoolFreeBuf function
* @endinternal
*
* @brief   Free a buffer back to its pool.
*
* @retval GT_STATUS  //  GT_OK     - For successful operation.
* @retval GT_BAD_PTR               - The returned buffer is not belongs to that pool
*/
GT_STATUS gtPoolFreeBuf
(
    IN        GT_POOL_ID  poolId,
    IN        GT_VOID*    pBuf
);


/**
* @internal gtPoolGetBufSize function
* @endinternal
*
* @brief   Get the buffer size.
*
* @retval GT_U32                   - the buffer size.
*/
GT_U32 gtPoolGetBufSize
(
    IN        GT_POOL_ID  poolId
);

/**
* @internal gtPoolExpandPool function
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
GT_STATUS gtPoolExpandPool
(
    IN   GT_POOL_ID        pPoolId,
    IN   GT_U32            numOfBuffers
);


/**
* @internal gtPoolGetBufFreeCnt function
* @endinternal
*
* @brief   Get the free buffer count.
*
* @retval GT_U32                   - the free buffer count.
*/
GT_U32 gtPoolGetBufFreeCnt
(
    IN        GT_POOL_ID  poolId
);


/**
* @internal gtPoolPrintStats function
* @endinternal
*
* @brief   Print pool's statistics for the requested pool, or for all
*         the pols in case poolID is GT_POOL_NULL_ID.
*/
void gtPoolPrintStats
(
    IN  GT_POOL_ID poolId
);


#ifdef __cplusplus
}
#endif

#endif  /* __gtBmPoolh */
/* Do Not Add Anything Below This Line */



