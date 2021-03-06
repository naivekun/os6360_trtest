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
* @file osVxMem.c
*
* @brief VxWorks Operating System wrapper. Memory manipulation facility.
*
* @version   23
********************************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cacheLib.h>
#include <vxWorks.h>

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsExc.h>

/*#include <../../mainDrv/h/prestera/common/gtMath.h>*/

#ifndef    RTOS_ON_SIM

#if (CPU == PPC604 || CPU == PPC603 || CPU == PPC405 || CPU == PPC860 || CPU == PPC85XX)
#define PHYS_TO_VIR_ADDR_CONVERT(addr) (addr)
#define VIR_TO_PHYS_ADDR_CONVERT(addr) (addr)
#endif

#if (CPU == R4000 || CPU == R3000)
#define PHYS_TO_VIR_ADDR_CONVERT(addr) (addr | 0xA0000000)
#define VIR_TO_PHYS_ADDR_CONVERT(addr) (addr & (~0xA0000000))
#endif

#if (CPU == ARMARCH4 || CPU == ARMARCH5 || CPU == ARMARCH7)
#define PHYS_TO_VIR_ADDR_CONVERT(addr) (addr)
#define VIR_TO_PHYS_ADDR_CONVERT(addr) (addr)
#endif

#else /* defined(RTOS_ON_SIM) */
#define PHYS_TO_VIR_ADDR_CONVERT(addr) (addr)
#define VIR_TO_PHYS_ADDR_CONVERT(addr) (addr)
#endif

/*
    Use the BSP functions to allocate non-cached (bspCachDmaMalloc) in an area totally
    outside of the VxWorks operating system. Especially for HSU usage.
*/

#if (CPU == PPC85XX)
#ifdef WORKBENCH_TOOL_ENV
#define USE_BSP_DMA_MALLOC
#endif
#endif

#ifdef XCAT_DRV
#define USE_BSP_DMA_MALLOC
#endif

#ifdef USE_BSP_DMA_MALLOC
extern int bspCacheDmaFree(void *ptr);
extern int bspCacheDmaMalloc(GT_U32 size);
#endif

/***** Global Vars ********************************************/

GT_BOOL  firstInitialization = GT_TRUE;

/***** External Functions **********************************************/
extern int osPrintf(const char* format, ...);

void *osMemPoolsAlloc(GT_U32 size);
void *osMemPoolsRealloc(void *ptr, GT_U32 size);
void osMemPoolFree(void *memBlock);
void *osStaticMallocFromChunk(GT_U32 size);

extern GT_VOID prvOsCacheDmaMallocOverflowDetectionSupportGet
(
    INOUT GT_U32    *sizePtr
);

extern GT_STATUS osCacheDmaRegisterPtr(
    IN GT_VOID *    allocPtr,
    IN GT_U32       numOfBytes
);
extern void osCacheDmaUnregisterPtr(
    IN GT_VOID *    oldPointer
);
extern GT_STATUS osCacheDmaSystemReset(void);
/***** Static functions ************************************************/

/***** Static variables ************************************************/

/***** Public Functions ************************************************/

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
)
{
    GT_MEMORY_MANAGE_METHOD_ENT memAllocMethod;
    GT_STATUS ret;

    if (reallocOnDemand == GT_TRUE)
    {
        memAllocMethod = GT_MEMORY_MANAGE_METHOD_REALLOC_ON_DEMAND_E;
    }
    else
    {
        memAllocMethod = GT_MEMORY_MANAGE_METHOD_NO_REALLOC_ON_DEMAND_E;
    }
    ret = osMemLibInit((GT_MEMORY_ALLOC_FUNC)malloc, (GT_MEMORY_FREE_FUNC)free, size, memAllocMethod);

    firstInitialization = GT_FALSE;
    return ret;
}



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
*       insufficient memory available or calling after first init.
*
* COMMENTS:
*       Usage of this function is only on FIRST initialization.
*
*******************************************************************************/
void *osStaticMalloc
(
    IN GT_U32 size
)
{
    if (firstInitialization == GT_TRUE)
    {
        return osMalloc(size);
    }
    return osStaticMallocFromChunk(size);
}

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
void *osMalloc
(
    IN GT_U32 size
)
{
    IS_WRAPPER_OPEN_PTR;

    if( size == 0) return NULL;

    if (firstInitialization == GT_TRUE)
    {
        /* during first init allocate from the OS */
        OSMEM_NOPOOLS_HDR_STC *hdrPtr;
        hdrPtr = (OSMEM_NOPOOLS_HDR_STC*)malloc(size+sizeof(OSMEM_NOPOOLS_HDR_STC));
        if (hdrPtr == NULL)
        {
            return NULL;
        }
        hdrPtr->magic = (GT_UINTPTR)hdrPtr;
        hdrPtr->size = size;
        hdrPtr++;
        return (GT_VOID*)hdrPtr;
    }
    else
    {
        return osMemPoolsAlloc(size);
    }

}

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
void *osRealloc
(
    IN void * ptr ,
    IN GT_U32 size
)
{
    OSMEM_NOPOOLS_HDR_STC *oldHdr;
    IS_WRAPPER_OPEN_PTR;

    if (ptr == NULL)
    {
        return osMalloc(size);
    }

    oldHdr = (OSMEM_NOPOOLS_HDR_STC*)((GT_UINTPTR)ptr - sizeof(OSMEM_NOPOOLS_HDR_STC));
    if (firstInitialization == GT_TRUE)
    {
        OSMEM_NOPOOLS_HDR_STC *allocPtr;
        if (oldHdr->magic != (GT_UINTPTR)oldHdr)
        {
            osFatalError(OS_FATAL_WARNING, "osRealloc(): bad magic");
        }
        allocPtr = realloc(oldHdr, size+sizeof(OSMEM_NOPOOLS_HDR_STC));
        if (allocPtr == NULL)
        {
            return NULL;
        }
        allocPtr->magic = (GT_UINTPTR)allocPtr;
        allocPtr->size = size;
        return (GT_VOID*)(allocPtr + 1);
    }
    else
    {
        if (oldHdr->magic == (GT_UINTPTR)oldHdr)
        {
            /* realloc memory allocated by malloc() */
            GT_VOID *newPtr = osMalloc(size);
            if (newPtr == NULL)
            {
                return NULL;
            }
            osMemCpy(newPtr, ptr, oldHdr->size < size ? oldHdr->size : size);
/*#ifdef OSMEMPOOLS_DEBUG
            osPrintf("Warning:Trying to Free a pointer allocated by malloc after osMemInit !\n");
#endif OSMEMPOOLS_DEBUG bothers CLI*/
            free(oldHdr);
            return newPtr;
        }
        return osMemPoolsRealloc(ptr, size);
    }
}

/**
* @internal osFree function
* @endinternal
*
* @brief   Deallocates or frees a specified memory block.
*
* @note Usage of this function is NEVER during initialization.
*
*/
void osFree
(
    IN void* const memblock
)
{
    OSMEM_NOPOOLS_HDR_STC *oldHdr = NULL;
    assert(memblock != NULL);
    oldHdr = (OSMEM_NOPOOLS_HDR_STC*)((GT_UINTPTR)memblock - sizeof(OSMEM_NOPOOLS_HDR_STC));
    if (firstInitialization == GT_TRUE)
    {
        assert(oldHdr->magic == (GT_UINTPTR)oldHdr);
        free(oldHdr);
    }
    else
    {
        /* check if this free is trying to free something that wasn't
           allocated by owr pools manager */
        if (oldHdr->magic == (GT_UINTPTR)oldHdr)
        {
/*#ifdef OSMEMPOOLS_DEBUG
            osPrintf("Warning:Trying to Free a pointer allocated by malloc after osMemInit !\n");
#endif OSMEMPOOLS_DEBUG bothers CLI*/
            free(oldHdr);
        }
        else
        {
            osMemPoolFree(memblock);
        }
    }
    return;
}

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
void *osCacheDmaMalloc
(
    IN GT_U32 size
)
{
    void * ptr;

    IS_WRAPPER_OPEN_PTR;

    /* check if need to get modified size to hold support for 'overflow detection' */
    prvOsCacheDmaMallocOverflowDetectionSupportGet(&size);

#ifdef USE_BSP_DMA_MALLOC
    ptr =  (void*)bspCacheDmaMalloc(size);
#else
    ptr =  (void*)cacheDmaMalloc(size);
#endif

    /* register the dma pointer for system reset */
    osCacheDmaRegisterPtr(ptr,size);

    return ptr;

}

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
    IN void *ptr
)
{
    IS_WRAPPER_OPEN_STATUS;

    /* unregister the ptr */
    (void)osCacheDmaUnregisterPtr(ptr);

#ifdef USE_BSP_DMA_MALLOC
    if (bspCacheDmaFree(ptr) == OK)
    {
        return GT_OK;
    }
    else
        return GT_FAIL;
#else
    if (cacheDmaFree(ptr) == OK)
    {
        return GT_OK;
    }
    else
        return GT_FAIL;
#endif

}

/**
* @internal osCacheDmaFreeAll function
* @endinternal
*
* @brief   Free ALL DMA memory (BM only)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osCacheDmaFreeAll(GT_VOID)
{
    /* let the manager clear the DB one by one (will be calling osCacheDmaFree(...))*/
    return osCacheDmaSystemReset();
}


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
)
{
    IS_WRAPPER_OPEN_STATUS;
    *virtAddr = PHYS_TO_VIR_ADDR_CONVERT(phyAddr);
    return GT_OK;
}

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
)
{
    IS_WRAPPER_OPEN_STATUS;
    *phyAddr = VIR_TO_PHYS_ADDR_CONVERT(virtAddr);

    return GT_OK;
}


