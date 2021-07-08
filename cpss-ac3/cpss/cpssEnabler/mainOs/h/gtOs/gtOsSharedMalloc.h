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
* @file gtOsSharedMalloc.h
*
* @brief This file contains declarations for functions which allocate memory
* for run-time variables in shared memory.
*
* @version   4
********************************************************************************
*/
#ifndef __gtOsSharedMalloch
#define __gtOsSharedMalloch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************* Includes *******************************************************/

#include <pthread.h>

#include <gtOs/gtGenTypes.h>

/************* Defines ********************************************************/

#ifdef SHARED_MEMORY
/* Use shared malloc for Shared Memory approach */
#   define SHARED_MALLOC_MAC shrMemSharedMalloc
#   define SHARED_FREE_MAC  shrMemSharedFree
#else
/* Use regular malloc/free without shared memory */
#   define SHARED_MALLOC_MAC malloc
#   define SHARED_FREE_MAC free
#endif

/************* Typedefs ********************************************************/
/**
* @struct SHARED_MEM_STC
 *
 * @brief This structure presents addresses begin and end of section.
*/
typedef struct{

    pthread_mutex_t mutex;

    char *name;

    GT_UINTPTR base;

    GT_UINTPTR curr;

    GT_UINTPTR last;

    GT_UINTPTR end;

} SHARED_MEM_STC;

/************* Functions ******************************************************/
/**
* @internal shrMemMmapMemory function
* @endinternal
*
* @brief   Function maps file into specified address space.
*
* @param[in] base                     -  address to be mapped to
* @param[in] size                     -  of memory block to be mapped
* @param[in] offset                   -  into file to map from
* @param[in] fd                       - file descriptor of file to be mapped
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*
* @note Performs mapping with FIXED and SHARED flag, RW access rights.
*       Checks for validity (FIXED region should be mapped without address
*       changes).
*
*/
GT_STATUS shrMemMmapMemory
(
    IN      GT_UINTPTR      base,
    IN      GT_U32          size,
    IN      GT_U32          offset,
    IN      GT_U32          fd
);

/**
* @internal shrMemSharedBufferInit function
* @endinternal
*
* @brief   Performes sharing of file as shared buffer.
*
* @param[in] namePtr                  - name of shared memory
* @param[in] base                     -  adress of shred memory
* @param[in] size                     -  of shared memory
* @param[in] fd                       - file descriptor of buffer
* @param[in] doWrite                  - protected to write(not used yet)
*
* @param[out] pmemPtr                  - pointer to shared memory stucure to be updated
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*
* @note pmem CAN refer into head of buffer to be shared. This case
*       should be habled correctly.
*
*/
GT_STATUS shrMemSharedBufferInit
(
    IN      char            *namePtr,
    IN      GT_UINTPTR      base,
    IN      GT_U32          size,
    IN      GT_32           fd,
    IN      GT_BOOL         doWrite,
    OUT     SHARED_MEM_STC  *pmemPtr
);

/*******************************************************************************
* shrMemSharedMallocIntoBlock
*
* DESCRIPTION:
*       Allocate memory into block
*
* INPUTS:
*       pmem    - shared memory stucture
*       fd      - descriptor of file to be extended if necessary 
*       size    - size of shared memory
*
*
* OUTPUTS:
*        None.
*
* RETURNS:
*       void*   - Operation succeeded. Adress of allocated memory
*       NULL    - Operaton failed
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *shrMemSharedMallocIntoBlock
(
    INOUT SHARED_MEM_STC    *pmem,
    IN    GT_U32            fd,
    IN    GT_U32            size
);

/*******************************************************************************
* shrMemSharedMalloc
*
* DESCRIPTION:
*       Allocate memory in size
*
* INPUTS:
*	size	- size of memory which needs to be allocated
*
* OUTPUTS:
*       None
*
* RETURNS:
*	GT_VOID *   - Operation succeeded. Adress of allocated memory
*   NULL        - Operaton failed
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *shrMemSharedMalloc
(
    IN unsigned int size
);

/**
* @internal shrMemSharedFree function
* @endinternal
*
* @brief   Free memory allocated by shrMemSharedMalloc()
*
* @param[in] ptr                      - memory which needs to be freed
*                                       None
*/
GT_VOID shrMemSharedFree
(
    IN GT_VOID *ptr
);

/**
* @internal shrMemSharedFreeSize function
* @endinternal
*
* @brief   Free memory allocated by shrMemSharedMalloc()
*
* @param[in] ptr                      - memory which needs to be freed
*                                      size - memory block size
*                                       None
*/
GT_VOID shrMemSharedFreeSize
(
    IN GT_VOID *ptr,
    IN unsigned int size
);

/**
* @internal shrMemSharedMallocInit function
* @endinternal
*
* @brief   Initialize shared library for processes
*
* @param[in] isFirstClient            - specify if client is the first client (CPSS enabler)
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operation failed
* @retval GT_NO_RESOURCE           - Operaton failed. No resource.
*
* @note Inits shared_malloc_fd descriptor (private for each process).
*
*/
GT_STATUS shrMemSharedMallocInit
(
    IN GT_BOOL isFirstClient
);

/*******************************************************************************
* shrMemAllocStatic
*
* DESCRIPTION:
*       Allocates memory into shared buffer with a given name
*       If memory with such name already allocated then return
*       pointer to such object
*
* INPUTS:
*       memBlockName - the name for memory block
*       size         - size of memory which needs to be allocated
*
* OUTPUTS:
*       None
*
* RETURNS:
*       void*   - Operation succeeded. Adress of allocated memory
*       NULL    - Operaton failed
*
* COMMENTS:
*       Uses shrMemSharedMalloc()
*
*******************************************************************************/
GT_VOID *shrMemAllocStatic(
  IN const char*    memBlockName,
  IN unsigned int   size
);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __gtOsSharedMalloch */


