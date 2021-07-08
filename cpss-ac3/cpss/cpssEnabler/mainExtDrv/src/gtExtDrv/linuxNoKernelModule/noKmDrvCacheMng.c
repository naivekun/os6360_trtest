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
* @file noKmDrvCacheMng.c
*
* @brief Cache management functions
* Use mvDmaDrv or HUGETLB feature
* On Intel CPUs requires 'intel_iommu=off' kernel option
*
* @version   1
********************************************************************************
*/
#define _GNU_SOURCE
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/prvExtDrvLinuxMapping.h>
#include <stdlib.h>
#include "prvNoKmDrv.h"

/**
* @internal extDrvMgmtCacheFlush function
* @endinternal
*
* @brief   Flush to RAM content of cache
*
* @param[in] type                     -  of cache memory data/intraction
* @param[in] address_PTR              - starting address of memory block to flush
* @param[in] size                     -  of memory block
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvMgmtCacheFlush
(
    IN GT_MGMT_CACHE_TYPE_ENT   type, 
    IN void                     *address_PTR, 
    IN size_t                   size
)
{
    (void) type;
    (void) address_PTR;
    (void) size;
    return GT_OK;
}


/**
* @internal extDrvMgmtCacheInvalidate function
* @endinternal
*
* @brief   Invalidate current content of cache
*
* @param[in] type                     -  of cache memory data/intraction
* @param[in] address_PTR              - starting address of memory block to flush
* @param[in] size                     -  of memory block
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvMgmtCacheInvalidate 
(
    IN GT_MGMT_CACHE_TYPE_ENT   type, 
    IN void                     *address_PTR, 
    IN size_t                   size
)
{
    (void) type;
    (void) address_PTR;
    (void) size;
    return GT_OK;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

void*  prvExtDrvDmaPtr = NULL;
GT_UINTPTR prvExtDrvDmaPhys = 0;
uint64_t prvExtDrvDmaPhys64 = 0;
GT_U32 prvExtDrvDmaLen = 0;

#ifndef MAP_32BIT
#define MAP_32BIT 0x40
#endif

#if defined(MAP_HUGETLB) && !defined(SHARED_MEMORY)
static uint64_t linux_virt_to_phys(uintptr_t vaddr)
{
    int pagemap;
    int64_t paddr = 0;
    uintptr_t pagesize = getpagesize();
    uintptr_t offset;
    uint64_t e;

    offset = (vaddr / pagesize) * sizeof(uint64_t);
    /* https://www.kernel.org/doc/Documentation/vm/pagemap.txt */
    if ((pagemap = open("/proc/self/pagemap", O_RDONLY)) < 0)
        return 0;
    if (lseek(pagemap, offset, SEEK_SET) != offset)
    {
        close(pagemap);
        return 0;
    }
    if (read(pagemap, &e, sizeof(uint64_t)) != sizeof(uint64_t))
    {
        close(pagemap);
        return 0;
    }
    close(pagemap);
    if (e & (1ULL << 63)) { /* page present ? */
        paddr = e & ((1ULL << 54) - 1); /* pfn mask */
        paddr = paddr * getpagesize();
        /* add offset within page */
        paddr |= (vaddr & (pagesize - 1));
    }
    return paddr;
}

/* try HUGETLB feature */
static long hugePagesTotal = -1;
static long hugePagesFree = -1;
static long hugePageSize = -1;
static int scan_proc_meminfo(void)
{
    FILE *f;
    char buf[BUFSIZ];
    unsigned long num;

    f = fopen("/proc/meminfo","r");
    if (!f)
        return 1;
    while (!feof(f))
    {
        if (!fgets(buf,sizeof(buf)-1,f))
            break;
        buf[sizeof(buf)-1] = 0;
        if (sscanf(buf, "HugePages_Total: %lu", &num) == 1)
        {
            hugePagesTotal = num;
            NOKMDRV_IPRINTF(("hugePagesTotal=%lu\n",hugePagesTotal));
        }
        if (sscanf(buf, "HugePages_Free: %lu", &num) == 1)
        {
            hugePagesFree = num;
            NOKMDRV_IPRINTF(("hugePagesFree=%lu\n",hugePagesFree));
        }
        if (sscanf(buf, "Hugepagesize: %lu", &num) == 1)
        {
            hugePageSize = num * 1024;
            NOKMDRV_IPRINTF(("hugePageSize=%lu\n",hugePageSize));
        }
    }
    fclose(f);
    if (hugePagesTotal < 0 || hugePagesFree < 0 || hugePageSize < 0)
        return 1;
    return 0;
}

static int alloc_hugetlb(void)
{
    void *h;
    if (scan_proc_meminfo())
        return 1;
    if (hugePagesFree == 0)
    {
        FILE *f;
        f = fopen("/proc/sys/vm/nr_hugepages","w");
        fprintf(f,"%lu\n", hugePagesTotal + 1);
        fclose(f);
        if (scan_proc_meminfo())
            return 1;
        if (hugePagesFree == 0)
            return 1;
    }

    h = mmap(
#ifndef SHARED_MEMORY
            NULL,
#else /* SHARED_MEMORY */
            (void*)((GT_UINTPTR)LINUX_VMA_DMABASE),
#endif
            hugePageSize,
            PROT_READ | PROT_WRITE,
#ifdef SHARED_MEMORY
            MAP_FIXED |
#endif
            MAP_ANONYMOUS | MAP_SHARED | MAP_HUGETLB,
            0, 0);
    if (h == MAP_FAILED)
    {
        perror("mmap(MAP_HUGETLB)");
        return 1;
    }
    prvExtDrvDmaPtr = h;
    prvExtDrvDmaLen = hugePageSize;

    /* resolve phys address */
    mlock(prvExtDrvDmaPtr, prvExtDrvDmaLen);
    prvExtDrvDmaPhys64 = linux_virt_to_phys((uintptr_t)prvExtDrvDmaPtr);
    prvExtDrvDmaPhys = (GT_UINTPTR)prvExtDrvDmaPhys64;
    if ((prvExtDrvDmaPhys64 & 0xffffffff00000000L)
            && (getenv("MVPP_DENY64BITPHYS") != NULL))
    {
        /* Address above 32bit address space, not supported by Marvell PPs */
        munlock(prvExtDrvDmaPtr, prvExtDrvDmaLen);
        munmap(prvExtDrvDmaPtr, prvExtDrvDmaLen);
        prvExtDrvDmaPtr = NULL;
        prvExtDrvDmaLen = 0;
        prvExtDrvDmaPhys = (GT_UINTPTR)0L;
        prvExtDrvDmaPhys64 = (uint64_t)0L;
    }
    else
    {
        NOKMDRV_IPRINTF(("hugePage mapped to %p\n", prvExtDrvDmaPtr));
        /* reset high 32 bits */
        prvExtDrvDmaPhys = (GT_UINTPTR)(prvExtDrvDmaPhys64 & 0x00000000ffffffffL);
    }

    return 0;
}
#endif

/* The offset parameter of mmap for mvDmaDrv
 * if non-zero then map DMA for PCI device
 */
off_t mvDmaDrvOffset = 0;
#ifdef MTS_BUILD
#  define LINUX_VMA_DMABASE       0x19000000
#  define DMA_LEN                 (4*1024*1024)
#else
#  define DMA_LEN                 (2*1024*1024)
#endif
	
static void try_map_mvDmaDrv(void)
{
    int fd;

    /* to don't mix message with kernel messages
     * flush sdtout then wait till all output written */
    fflush(stdout);
    tcdrain(1);

    fd = open("/dev/mvDmaDrv", O_RDWR);
    if (fd >= 0)
    {
        lseek(fd, mvDmaDrvOffset, 0);
        prvExtDrvDmaLen = DMA_LEN;
        prvExtDrvDmaPtr = mmap(
#if !defined(SHARED_MEMORY) && !defined(MTS_BUILD)
            NULL,
#else /* SHARED_MEMORY */
            (void*)((GT_UINTPTR)LINUX_VMA_DMABASE),
#endif
            prvExtDrvDmaLen,
            PROT_READ | PROT_WRITE,
#if defined(SHARED_MEMORY) || defined(MTS_BUILD)
            MAP_FIXED |
#endif
            MAP_32BIT | MAP_SHARED,
            fd, 0);
        if (prvExtDrvDmaPtr == MAP_FAILED)
        {
            perror("mmap(mvDmaDrv)");
            prvExtDrvDmaPtr = NULL;
            close(fd);
            return;
        }
        else
        {
            NOKMDRV_IPRINTF(("mvDmaDrv mapped to %p\n",prvExtDrvDmaPtr));
        }
    }
    /* to don't mix message with kernel messages
     * flush sdtout then wait till all output written */
    fflush(stdout);
    tcdrain(1);


    prvExtDrvDmaPhys = 0;
    prvExtDrvDmaPhys64 = 0;
    if (read(fd, &prvExtDrvDmaPhys64, 8) == 8)
    {
        prvExtDrvDmaPhys = (GT_UINTPTR)prvExtDrvDmaPhys64;
    }
}

/**
* @internal check_dma function
* @endinternal
*
* @brief   Check if DMA block already allocated/mapped to userspace
*         If no then allocate/map
*/
static void check_dma(void)
{
    if (prvExtDrvDmaPtr != NULL)
        return;
    /* try to map from mvDmaDrv */
    try_map_mvDmaDrv();
#if defined(MAP_HUGETLB) && !defined(SHARED_MEMORY)
    if (prvExtDrvDmaPtr == NULL)
    {
        /* try to allocate hugetlb */
        alloc_hugetlb();
    }
#endif
NOKMDRV_IPRINTF(("dmaLen=%d\n", prvExtDrvDmaLen));
NOKMDRV_IPRINTF(("dmaPhys64=0x%llx\n", prvExtDrvDmaPhys64));
NOKMDRV_IPRINTF(("dmaPhys=%p\n", (void*)prvExtDrvDmaPhys));
}

/**
* @internal extDrvGetDmaBase function
* @endinternal
*
* @brief   Get the base address of the DMA area need for Virt2Phys and Phys2Virt
*         translaton
*
* @param[out] dmaBase                  - he base address of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDmaBase
(
	OUT GT_UINTPTR * dmaBase
)
{
    check_dma();
    *dmaBase = (GT_UINTPTR)prvExtDrvDmaPhys;
    if (!(*dmaBase))
        return GT_ERROR;

    return GT_OK;
}

/**
* @internal extDrvGetDmaVirtBase function
* @endinternal
*
* @brief   Get the base address of the DMA area in userspace
*
* @param[out] dmaVirtBase              - the base address of the DMA area in userspace.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDmaVirtBase
(
	OUT GT_UINTPTR *dmaVirtBase
)
{
    check_dma();
    *dmaVirtBase = (GT_UINTPTR)prvExtDrvDmaPtr;
    if (!(*dmaVirtBase))
        return GT_ERROR;

    return GT_OK;
}

/**
* @internal extDrvGetDmaSize function
* @endinternal
*
* @brief   Get the size of the DMA area
*
* @param[out] dmaSize                  - The size of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
extern GT_STATUS extDrvGetDmaSize
(
	OUT GT_U32 * dmaSize
)
{
    *dmaSize = prvExtDrvDmaLen;
    return GT_OK;
}


