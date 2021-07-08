#error "Deprecated"
/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#ifndef __cliMemLibh
#define __cliMemLibh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>


/*******************************************************************************
* CLI_MEMORY_ALLOC_FUNC
*
* DESCRIPTION:
*       Allocates memory
*
* INPUTS:
*       size - size in bytes
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the allocated memory or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID* (*CLI_MEMORY_ALLOC_FUNC)
(
    unsigned int size
);

/*******************************************************************************
* CLI_MEMORY_FREE_FUNC
*
* DESCRIPTION:
*       Free allocated memory
*
* INPUTS:
*       ptr - A pointer to the allocated memory
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the allocated memory or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID (*CLI_MEMORY_FREE_FUNC)
(
    GT_VOID *ptr
);


/**
* @enum CLI_MEMORY_MANAGE_METHOD_ENT
 *
 * @brief This enumeration defines the memory allocation method
*/
typedef enum{

    /** @brief in the case of memory
     *  chunk depletion an out of memory will be reported
     */
    CLI_MEMORY_MANAGE_METHOD_NO_REALLOC_ON_DEMAND_E,

    /** @brief in the case of memory
     *  chunk depletion a new chunk will be allocated
     */
    CLI_MEMORY_MANAGE_METHOD_REALLOC_ON_DEMAND_E

} CLI_MEMORY_MANAGE_METHOD_ENT;




/************* Functions ******************************************************/

/**
* @internal cliMemLibInit function
* @endinternal
*
* @brief   Initialize the memory management with a user memory function.
*         Can be used to allocate from a dedicated memory pool that remains till
*         HW reset.
* @param[in] memAllocFuncPtr          - the function to be used in memory allocation.
* @param[in] memFreeFuncPtr           - the function to be used in memory allocation.
* @param[in] size                     - The memory  to manage
* @param[in] memAllocMethod           - determines the method of the allocation
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - not enough memory in the system.
*/
GT_STATUS cliMemLibInit
(    
    IN CLI_MEMORY_ALLOC_FUNC memAllocFuncPtr,
    IN CLI_MEMORY_FREE_FUNC memFreeFuncPtr,
    IN GT_U32 size,
    IN CLI_MEMORY_MANAGE_METHOD_ENT memAllocMethod
);

/**
* @internal cliMemLibReset function
* @endinternal
*
* @brief   Reset memory pools to empty state
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - not initialized yet
*/
GT_STATUS cliMemLibReset(void);

/*******************************************************************************
* cliMalloc
*
* DESCRIPTION:
*       Allocates memory block of specified size.
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
GT_VOID *cliMalloc
(
    IN GT_U32 size
);

/*******************************************************************************
* cliRealloc
*
* DESCRIPTION:
*       Reallocate memory block of specified size.
*
* INPUTS:
*       ptr  - pointer to previously allocated buffer
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*       Recommended to avoid usage of this function.
*
*******************************************************************************/
GT_VOID *cliRealloc
(
    IN GT_VOID * ptr ,
    IN GT_U32    size
);

/**
* @internal cliFree function
* @endinternal
*
* @brief   Deallocates or frees a specified memory block.
*
* @note Usage of this function is NEVER during initialization.
*
*/
GT_VOID cliFree
(
    IN GT_VOID* const memblock
);

#ifdef __cplusplus
}
#endif

#endif  /* __cliMemLibh */
/* Do Not Add Anything Below This Line */



