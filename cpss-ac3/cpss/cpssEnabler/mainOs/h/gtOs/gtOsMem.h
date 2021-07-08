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
* @file gtOsMem.h
*
* @brief Operating System wrapper. Memory manipulation facility.
*
* @version   27
********************************************************************************
*/

#ifndef __gtOsMemh
#define __gtOsMemh

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* define 'osMalloc,osRealloc,osFree debug mode' to allow find memory leakage */
/* OS_MALLOC_MEMORY_LEAKAGE_DBG                                               */
/* logic:
    1. every allocation done by osMalloc,osRealloc will be registered with the
    file name + line number of the allocating source code.
    2. every free done by osFree will remove the registration of this info
    3. use osMallocMemoryLeakageDbgAction(start) to start the registration of the info
        every start clears the info about previous allocations
    4. use osMallocMemoryLeakageDbgAction(stop) to stop the registration of the info
    5. use osMallocMemoryLeakageDbgPrintStandingAllocated() to print the info about the
       allocated pointers that were not free in the time between 'start' and 'Stop'

    *********************************
    when work with CPSS must set OS_MALLOC_MEMORY_LEAKAGE_DBG also
    in file <../common/h/cpss/extServices/os/gtOs/cpssOsMem.h>
*/
/******************************************************************************/
/*#define OS_MALLOC_MEMORY_LEAKAGE_DBG*/

#ifndef CHX_FAMILY
    /* (currently) this mode supported only by the DXCH */
    #undef OS_MALLOC_MEMORY_LEAKAGE_DBG
#endif /*EXMXPM_FAMILY*/

/* currently allow the leakage detection only in WM */
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    #ifndef ASIC_SIMULATION
        #undef OS_MALLOC_MEMORY_LEAKAGE_DBG
    #endif /*!ASIC_SIMULATION*/
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/


/* check if flag to force ignoring the debug of leakage */
/* this flag MUST be used ONLY by the C files that IMPLEMENTS osMalloc,osRealloc,osFree */
/* and not by other parts of the system */
#ifdef FORCE_IGNORE_OS_MALLOC_MEMORY_LEAKAGE_DBG
    #ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
        #undef OS_MALLOC_MEMORY_LEAKAGE_DBG
        /* still need to give the internal file indication that
           OS_MALLOC_MEMORY_LEAKAGE_DBG was set !!! */
        #define INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
    #endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/
#endif /*FORCE_IGNORE_OS_MALLOC_MEMORY_LEAKAGE_DBG*/

#ifdef INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
    /* debug info of internal allocations */
    #define LOCAL_DEBUG_INFO    __FILE__,__LINE__
#else
    /* indication of no debug info */
    #define LOCAL_DEBUG_INFO    NULL,0
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************* Defines ********************************************************/
#if __WORDSIZE == 64
#  ifndef OS_MEM_64_BIT_ALIGNED
#    define OS_MEM_64_BIT_ALIGNED
#  endif
#endif

#ifdef OS_MEM_64_BIT_ALIGNED
#define OSMEM_DEFAULT_BLOCK_STEP_SIZE 8
#else
#define OSMEM_DEFAULT_BLOCK_STEP_SIZE 4
#endif
#define OSMEM_DEFAULT_STEPS_LIMIT     128
#define OSMEM_DEFAULT_POOLS_LIMIT     65536

/* Obsolete will be removed in the next version */
/* #define OSMEM_DEFAULT_MEM_INIT_SIZE   2048*1024 */
#ifdef EXMXPM_FAMILY
#if defined(CPU_78200) ||  (CPU == PPC85XX)
#define OSMEM_MAX_MEMORY_SIZE_USE 512*1024*1024 /* 512MB */
#else
#define OSMEM_MAX_MEMORY_SIZE_USE 64*1024*1024 /* 64MB */
#endif
#else
#if defined(SHARED_MEMORY) && defined(LINUX_SIM)
#define OSMEM_MAX_MEMORY_SIZE_USE 512*1024*1024 /* 512MB */
#else
#define OSMEM_MAX_MEMORY_SIZE_USE 128*1024*1024 /* 128MB */
#endif
#endif

/* don't disable debug if UTs included in the build! */
#define OSMEMPOOLS_DEBUG

/**
* @enum OSMEM_MEMORY_LEAKAGE_STATE_DBG_ENT
 *
 * @brief enumeration of debugging state of the memory leakage in the system.
 * values:
 * OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E - the memory manager stopped recording
 * all 'osMalloc,osRealloc,osFree' operations
 * OSMEM_MEMORY_LEAKAGE_STATE_DBG_ON_E - the memory manager recording all
 * 'osMalloc,osRealloc,osFree' operations
*/
typedef enum{

    OSMEM_MEMORY_LEAKAGE_STATE_DBG_OFF_E,

    OSMEM_MEMORY_LEAKAGE_STATE_DBG_ON_E

} OSMEM_MEMORY_LEAKAGE_STATE_DBG_ENT;

/*
 * Typedef: struct OSMEM_POOL_FREE_ELEM
 *
 * Description:
 *      This structure represent a node in the pool's free list.
 *
 * Fields:
 *      next_elem     - next node in the memory pool's list.
 *      size          - the size of the memory block (used only when the various
 *                      sizes pool).
 *
 */

typedef struct os_mem_pool_free_elem{
    struct os_mem_pool_free_elem    *next_elem;
    GT_U32                          size;
}OSMEM_POOL_FREE_ELEM,*OSMEM_POOL_FREE_ELEM_PTR;


/*
 * Typedef: struct OSMEM_POOL_ELEM
 *
 * Description:
 *      This structure represent a node followed by a memory block in the memory
 *      linked list managed by the memory pools.
 *
 * Fields:
 *        poolIndex -     real required for dynamic allocation size
 *                for example if the required size is 150 bytes but the allocated pool
 *                length is 256 bytes this  field will be set to 150
 *
 */

typedef struct os_mem_pool_elem{
#if (__WORDSIZE != 64) && defined(OS_MEM_64_BIT_ALIGNED)
    GT_UINTPTR                  padding;
#endif
    GT_UINTPTR                  poolIndex;
}OSMEM_POOL_ELEM,*OSMEM_POOL_ELEM_PTR;

/*
 * Typedef: struct OSMEM_VAR_POOL_ELEM
 *
 * Description:
 *      This structure represent a node followed by a memory block in the
 *      various sizes memory linked list managed by the memory pools.
 *
 * Fields:
 *      poolIndex     - the index of the pool which this element belongs to.
 *      size          - the size of the element.
 *
 */

typedef struct os_mem_var_pool_elem{
    GT_UINTPTR                  size;
    GT_UINTPTR                  poolIndex;
}OSMEM_VAR_POOL_ELEM,*OSMEM_VAR_POOL_ELEM_PTR;


/**
* @struct OSMEM_POOL_HDR,*OSMEM_POOL_HDR_PTR
 *
 * @brief This structure holds the header of a list of memory managed pool for
 * the osMalloc mechanism.
*/

typedef struct {
    GT_U32                      poolIndex;
    GT_U32                      elem_size;
    GT_U32                      free_size;
    GT_U32                      overAlloc;
    GT_U32                      alloc_times;
    GT_U32                      free_times;
    GT_U32                      malloc_used;
    GT_U32                      size;

    GT_U32                      chunkIndex;

    struct os_mem_pool_free_elem    *first_free_PTR;

}OSMEM_POOL_HDR,*OSMEM_POOL_HDR_PTR;


/*
 * Typedef: struct OSMEM_CHUNK_POINTER
 *
 * Description:
 *      This structure represent a chunk of memory allocated for the memory
 *      manager.
 *
 * Fields:
 *      startAddr   - a pointer to the beginning of the chunk.
 *      dynamicAddr - a pointer to the end of the dynamic memory allocated from
 *                    this chunk.
 *      staticAddr  - a pointer to the beginning of the static memory allocated
 *                    from this chunk.
 *      freeSize    - the current free size in the chunk.
 *
 */

typedef struct os_mem_chunk_pointer{
    GT_CHAR*                    startAddr;
    GT_CHAR*                    dynamicAddr;
    GT_CHAR*                    staticAddr;
    GT_U32                      freeSize;

}OSMEM_CHUNK_POINTER,*OSMEM_CHUNK_POINTER_PTR;


/*
 * Typedef: struct OSMEM_NOPOOLS_HDR_STC
 *
 * Description:
 *      This structure represent a header of memory which is not allocated from
 *      osMemLib pools.
 *
 * Fields:
 *      magic       - magic number, must be equal to pointer to this structure.
 *      size        - an allocated memory size.
 *
 */
typedef struct OSMEM_NOPOOLS_HDR_STCT {
    GT_UINTPTR size;
    GT_UINTPTR magic;
} OSMEM_NOPOOLS_HDR_STC;


/*******************************************************************************
* GT_MEMORY_ALLOC_FUNC
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
typedef GT_VOID* (*GT_MEMORY_ALLOC_FUNC)
(
    unsigned int size
);

/*******************************************************************************
* GT_MEMORY_FREE_FUNC
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
typedef GT_VOID (*GT_MEMORY_FREE_FUNC)
(
    GT_VOID *ptr
);


/**
* @enum GT_MEMORY_MANAGE_METHOD_ENT
 *
 * @brief This enumeration defines the memory allocation method
*/
typedef enum{

    /** @brief in the case of memory
     *  chunk depletion an out of memory will be reported
     */
    GT_MEMORY_MANAGE_METHOD_NO_REALLOC_ON_DEMAND_E,

    /** @brief in the case of memory
     *  chunk depletion a new chunk will be allocated
     */
    GT_MEMORY_MANAGE_METHOD_REALLOC_ON_DEMAND_E

} GT_MEMORY_MANAGE_METHOD_ENT;

/*
 * Typedef: struct OSMEM_CHUNK_ARRAY
 *
 * Description:
 *      This structure represent the array of chunks for the memory mangaer use
 *
 * Fields:
 *      chunkArray     - the array holding the pointer to the chunks.
 *      chunkSize      - the size of a chunk (one size for all).
 *      lastChunkIndex - a pointer to the beginning of the last allocated chunk.
 *      staticIndex    - the index from which to allocated the static memory.
 *      chunkArraySize - the size of the chunkArray
 *      memAllocFuncPtr - the function to be used in memory allocation.
 *      memAllocMethod - determines the method of the allocation
 *
 */

typedef struct os_mem_chunk_array{
    OSMEM_CHUNK_POINTER_PTR         chunkArray;
    GT_U32                          chunkSize;
    GT_U32                          lastChunkIndex;
    GT_U32                          staticIndex;
    GT_U32                          chunkArraySize;
    GT_MEMORY_ALLOC_FUNC            memAllocFuncPtr;
    GT_MEMORY_FREE_FUNC             memFreeFuncPtr;
    GT_MEMORY_MANAGE_METHOD_ENT     memAllocMethod;

}OSMEM_CHUNK_ARRAY,*OSMEM_CHUNK_ARRAY_PTR;





/************* Functions ******************************************************/

/**
* @internal osBzero function
* @endinternal
*
* @brief   Fills the first nbytes characters of the specified buffer with 0
*
* @param[in] start                    -  address of memory block to be zeroed
* @param[in] nbytes                   - size of block to be set
*                                       None
*/
GT_VOID osBzero
(
    IN GT_CHAR *start,
    IN GT_U32  nbytes
);

/*******************************************************************************
* osMemSet
*
* DESCRIPTION:
*       Stores 'simbol' converted to an unsigned char in each of the elements
*       of the array of unsigned char beginning at 'start', with size 'size'.
*
* INPUTS:
*       start  - start address of memory block for setting
*       simbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID * osMemSet
(
    IN GT_VOID * start,
    IN int       symbol,
    IN GT_U32    size
);

/*******************************************************************************
* osMemCpy
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID * osMemCpy
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32          size
);

/*******************************************************************************
* osMemMove
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. The memory areas may overlap:
*       copying takes place as though the bytes in src are first copied into
*       a temporary array that does  not  overlap src or dest, and the bytes
*       are then copied from the temporary array to dest.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID * osMemMove
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32          size
);


/**
* @internal osMemCmp function
* @endinternal
*
* @brief   Compare 'size' characters from the object pointed to by 'str1' to
*         the object pointed to by 'str2'.
* @param[in] str1                     - first memory area
* @param[in] str2                     - second memory area
* @param[in] size                     -  of memory to compare
*
* @retval > 0                      - if str1 is alfabetic bigger than str2
* @retval == 0                     - if str1 is equal to str2
* @retval < 0                      - if str1 is alfabetic smaller than str2
*/
GT_32 osMemCmp
(
    IN const GT_VOID  *str1,
    IN const GT_VOID  *str2,
    IN GT_U32          size
);


/**
* @internal osMemInit function
* @endinternal
*
* @brief   Initialize the memory management.
*
* @param[in] size                     - memory  to manage.
* @param[in] reallocOnDemand          - defines whether in the case of memory chunk depleation
*                                      a new chunk will be allocated or an out of memory will
*                                      be reported.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - not enough memory in the system.
*/
GT_STATUS osMemInit
(
    IN GT_U32 size,
    IN GT_BOOL reallocOnDemand
);


/**
* @internal osMemLibInit function
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
GT_STATUS osMemLibInit
(
    IN GT_MEMORY_ALLOC_FUNC memAllocFuncPtr,
    IN GT_MEMORY_FREE_FUNC memFreeFuncPtr,
    IN GT_U32 size,
    IN GT_MEMORY_MANAGE_METHOD_ENT memAllocMethod
);


/*******************************************************************************
* osStaticAlloc
*
* DESCRIPTION:
*       Allocates memory block of specified size, this memory will not be free.
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
*       Usage of this function is only on FIRST initialization.
*
*******************************************************************************/
GT_VOID *osStaticMalloc
(
    IN GT_U32 size
);

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
/* define osMalloc as macro to other function with debug info */
#define osMalloc(_size)         osMalloc_MemoryLeakageDbg(_size         ,__FILE__,__LINE__)
/* define osRealloc as macro to other function with debug info */
#define osRealloc(_ptr,_size)   osRealloc_MemoryLeakageDbg(_ptr,_size   ,__FILE__,__LINE__)
/* define osFree as macro to other function with debug info */
#define osFree(_memblock)       osFree_MemoryLeakageDbg(_memblock       ,__FILE__,__LINE__)
/* osMalloc_MemoryLeakageDbg replace osMalloc to get debug info */
GT_VOID *osMalloc_MemoryLeakageDbg
(
    IN GT_U32   size,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);
/* osRealloc_MemoryLeakageDbg replace osRealloc to get debug info */
GT_VOID *osRealloc_MemoryLeakageDbg
(
    IN GT_VOID * ptr ,
    IN GT_U32    size,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);
/* osFree_MemoryLeakageDbg replace osFree to get debug info */
GT_VOID osFree_MemoryLeakageDbg
(
    IN GT_VOID* const memblock,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);

#else   /* ! OS_MALLOC_MEMORY_LEAKAGE_DBG*/
/*******************************************************************************
* osMalloc
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
GT_VOID *osMalloc
(
    IN GT_U32 size
);

/*******************************************************************************
* osRealloc
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
GT_VOID *osRealloc
(
    IN GT_VOID * ptr ,
    IN GT_U32    size
);

/**
* @internal CPSS_osFree function
* @endinternal
*
* @brief   Deallocates or frees a specified memory block.
*
* @note Usage of this function is NEVER during initialization.
*
*/
#define osFree CPSS_osFree
GT_VOID CPSS_osFree
(
    IN GT_VOID* const memblock
);
#endif /*! OS_MALLOC_MEMORY_LEAKAGE_DBG*/


/**
* @internal osCacheDmaOverflowDetectionEnableSet function
* @endinternal
*
* @brief   enable the 'Cache DMA' manager to add 'overflow detection' region at
*         the end of each 'Cached DMA' (on top of the actual needed size) and to
*         initialize this region with 'predefined pattern' words.
*         this function allow the application to check via function
*         osCacheDmaOverflowDetectionErrorGet(...) to check if any of the 'Cached DMA'
*         caused 'overflow'
* @param[in] enable                   - GT_TRUE -  the detection.
*                                      GT_FALSE - disable the detection.
*
* @retval GT_OK                    - on success
*/
GT_STATUS osCacheDmaOverflowDetectionEnableSet
(
    IN GT_BOOL  enable
);

/**
* @internal osCacheDmaOverflowDetectionErrorGet function
* @endinternal
*
* @brief   the function checks ALL if any of the 'Cached DMA' memories caused 'overflow'
*         and the function will return the pointers to the 'Cached DMA' memories with errors.
* @param[in] maxNumErrorsToGet        - max number of errors that dmaMemoryWithErrorsArr[] can hold.
*                                      NOTE: value 0 means that application not care about which
*                                      memory caused the problem.
* @param[in,out] dmaMemoryWithErrorsArr[] - array of pointers to get the 'Cached DMA' memories with errors.
*                                      NOTE : 1. this array can be NULL when maxNumErrorsToGet == 0
*                                      2. this array must hold space for maxNumErrorsToGet 'Cached DMA' memories.
*                                      3. if maxNumErrorsToGet is smaller then actual number of memories with errors,
*                                      then the rest of the memories are not filled into this array.
*
* @param[out] actualNumErrorsPtr       - (pointer to) the actual number of 'Cached DMA' memories
*                                      that caused 'overflow'.
*                                      can be NULL if application not care about the number of errors.
* @param[in,out] dmaMemoryWithErrorsArr[] - array of pointers to the 'Cached DMA' memories with errors.
*
* @retval GT_OK                    - no memory caused error.
* @retval GT_ERROR                 - at least one of the 'Cached DMA' memories with errors.
*/
GT_STATUS osCacheDmaOverflowDetectionErrorGet
(
    IN    GT_U32  maxNumErrorsToGet,
    OUT   GT_U32  *actualNumErrorsPtr,
    INOUT   void*   dmaMemoryWithErrorsArr[]
);

/*******************************************************************************
* osCacheDmaMalloc
*
* DESCRIPTION:
*       Allocate a cache-safe buffer  of specified size for DMA devices
*       and drivers
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the cache-safe buffer, or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID *osCacheDmaMalloc
(
    IN GT_U32 size
);

/**
* @internal osCacheDmaFree function
* @endinternal
*
* @brief   Free the buffer acquired with osCacheDmaMalloc()
*
* @param[in] ptr                      - pointer to malloc/free buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osCacheDmaFree
(
    IN GT_VOID *ptr
);

/**
* @internal osCacheDmaFreeAll function
* @endinternal
*
* @brief   Free ALL DMA memory
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osCacheDmaFreeAll(GT_VOID);


/**
* @internal osPhy2Virt function
* @endinternal
*
* @brief   Converts physical address to virtual.
*
* @param[in] phyAddr                  - physical address
*
* @param[out] virtAddr                 - virtual address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osPhy2Virt
(
    IN  GT_UINTPTR phyAddr,
    OUT GT_UINTPTR *virtAddr
);

/**
* @internal osVirt2Phy function
* @endinternal
*
* @brief   Converts virtual address to physical.
*
* @param[in] virtAddr                 - virtual address
*
* @param[out] phyAddr                  - physical address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osVirt2Phy
(
    IN  GT_UINTPTR virtAddr,
    OUT GT_UINTPTR *phyAddr
);

/**
* @internal osMemPoolsClear function
* @endinternal
*
* @brief   returns all the allocated memory from all the pools to free.
*
* @retval GT_OK                    - if success.
*/
GT_STATUS osMemPoolsClear
(
    GT_VOID
);

/**
* @internal osMemGetHeapBytesAllocated function
* @endinternal
*
* @brief   returns the number of bytes allocated in heap
*/
GT_U32 osMemGetHeapBytesAllocated(GT_VOID);

/**
* @internal osMemStartHeapAllocationCounter function
* @endinternal
*
* @brief   set the current value of heap allocated bytes to the allocation counter
*/
GT_VOID osMemStartHeapAllocationCounter(GT_VOID);

/**
* @internal osMemGetHeapAllocationCounter function
* @endinternal
*
* @brief   returns the delta of current allocated bytes number and the value of allocation counter set by
*         preveous startHeapAllocationCounter() function
*/
GT_U32 osMemGetHeapAllocationCounter(GT_VOID);


#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG

/* define osDbgMalloc as macro to other function with debug info */
#define osDbgMalloc(_size) osDbgMalloc_MemoryLeakageDbg(_size,__FILE__,__LINE__)
/* osDbgMalloc_MemoryLeakageDbg replace osDbgMalloc to get debug info */
void *osDbgMalloc_MemoryLeakageDbg
(
    IN GT_U32 size,
    IN const char*    fileNamePtr,
    IN GT_U32   line
);

#else  /* ! OS_MALLOC_MEMORY_LEAKAGE_DBG */
/*******************************************************************************
* osDbgMalloc
*
* DESCRIPTION:
*       Allocates memory block of specified size or fail allocation
*       in accordance with checked flags
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
*       None
*
*******************************************************************************/
void *osDbgMalloc
(
    IN GT_U32 size
);

#endif /* ! OS_MALLOC_MEMORY_LEAKAGE_DBG */

/**
* @internal osDbgMallocFailSet function
* @endinternal
*
* @brief   Allocates memory block of specified size or fail allocation
*         in accordance with checked flags
*
* @retval GT_OK                    - on success
*/
GT_STATUS osDbgMallocFailSet
(
    IN GT_BOOL                  failEnable,
    IN GT_U32                   failCounter
);

/**
* @internal osMemGetMallocAllocationCounter function
* @endinternal
*
* @brief   Get malloc allocation counter
*
* @param[in] allocationNumberCounterPtr - pointer to number of malloc allocations
*
* @retval GT_OK                    - on success
*/
GT_STATUS osMemGetMallocAllocationCounter
(
    IN GT_U32 *allocationNumberCounterPtr
);

/**
* @internal osMallocMemoryLeakageDbgAction function
* @endinternal
*
* @brief   debug tool relate to 'Memory leakage detection'
*         relevant only when compilation with : OS_MALLOC_MEMORY_LEAKAGE_DBG
*         Set state of debug for memory leakage: start/stop.
* @param[in] memoryLeakageState       - state of debug for memory leakage:
*                                      start/stop.
*
* @retval GT_OK                    - on success
*
* @note use function osMallocMemoryLeakageDbgPrintStandingAllocated to dump all the
*       allocated pointer that are still allocated (not free) that were allocated
*       between last 'start' and the consecutive 'stop'
*
*/
GT_STATUS osMallocMemoryLeakageDbgAction
(
    IN OSMEM_MEMORY_LEAKAGE_STATE_DBG_ENT  memoryLeakageState
);

/**
* @internal osMallocMemoryLeakageDbgPrintStandingAllocated function
* @endinternal
*
* @brief   debug tool relate to 'Memory leakage detection'
*         relevant only when compilation with : OS_MALLOC_MEMORY_LEAKAGE_DBG
*         print the info about the allocated pointers that were not free in the
*         time between 'start' and 'Stop'
*
* @retval GT_OK                    - on success
*/
GT_STATUS osMallocMemoryLeakageDbgPrintStandingAllocated
(
    GT_VOID
);



#ifdef __cplusplus
}
#endif

#endif  /* __gtOsMemh */
/* Do Not Add Anything Below This Line */



