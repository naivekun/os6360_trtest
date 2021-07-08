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
* @file noKmDrvPciDrvMbus.c
*
* @brief MBUS driver
*
* @version   1
********************************************************************************
*/
#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#include "prvNoKmDrv.h"

#include "drivers/mvResources.h"

#define RES_OFFSET(_nm) offsetof(CPSS_HW_INFO_STC,resource._nm)

static struct {
    int cpss_resource_offset;
    int mbus_resource;
    GT_UINTPTR  maxSize;
} mbusResources[] = {
    { RES_OFFSET(cnm),                    MV_RESOURCE_MBUS_RUNIT,  _1M },
    { RES_OFFSET(switching),              MV_RESOURCE_MBUS_SWITCH, _64M },
    { RES_OFFSET(resetAndInitController), MV_RESOURCE_MBUS_DFX,    0 },
    { RES_OFFSET(dragonite.itcm),         MV_RESOURCE_MBUS_DRAGONITE_ITCM, 0 },
    { RES_OFFSET(dragonite.dtcm),         MV_RESOURCE_MBUS_DRAGONITE_DTCM, 0 },
    { -1, -1, 0 }
};

static int mvMbusDrvFd = -1;

static int mvMbusDrvOpen(void)
{
    if (mvMbusDrvFd < 0)
        mvMbusDrvFd = open("/dev/mvMbusDrv", O_RDWR);
    return mvMbusDrvFd < 0 ? 0 : 1;
}

static int mvMbusDrvReadConfig(int resource, unsigned long long *val)
{
    if (!mvMbusDrvOpen())
        return -1;
    lseek(mvMbusDrvFd, resource, 0);
    return read(mvMbusDrvFd, val, sizeof(*val));
}

/**
* @internal extDrvPciMbusConfigReadReg function
* @endinternal
*
* @brief   This routine read register from the PCI configuration space.
*
* @param[in] regAddr                  - Register offset in the configuration space.
*
* @param[out] data                     - the read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciMbusConfigReadReg
(
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    unsigned long long res;
    switch (regAddr)
    {
        case 0x00:
            if (mvMbusDrvReadConfig(MV_RESOURCE_DEV_ID | MV_RESOURCE_START, &res) < 0)
            {
                *data = 0;
                return GT_OK;
            }
            *data = (((GT_U32)res) << 16) | 0x11ab;
            return GT_OK;
        case 0x10:
        case 0x18:
        case 0x20:
            *data = 0xfff0000c;
            return GT_OK;
#if 0
        /* revision */
        case (0x8):
#endif
    }
    *data = 0;
    return GT_OK;
}




#ifndef SHARED_MEMORY
GT_STATUS mbus_map_resource(
    IN  int                         mbusResource,
    IN  GT_UINTPTR                  maxSize,
    OUT CPSS_HW_INFO_RESOURCE_MAPPING_STC *mappingPtr
)
{
    unsigned long long res;
    void *vaddr;

    if (mvMbusDrvReadConfig(mbusResource | MV_RESOURCE_SIZE, &res) < 0)
        return GT_NOT_FOUND;
    if (maxSize > (GT_UINTPTR)res || maxSize == 0)
        maxSize = (GT_UINTPTR)res;
    vaddr = mmap(NULL,
                maxSize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                mvMbusDrvFd,
                (off_t)(mbusResource << SYSTEM_PAGE_SHIFT));
    if (MAP_FAILED == vaddr)
    {
        return GT_FAIL;
    }
    mappingPtr->start = (GT_UINTPTR)vaddr;
    mappingPtr->size  = (GT_UINTPTR)maxSize;
    if (mvMbusDrvReadConfig(mbusResource | MV_RESOURCE_START, &res) >= 0)
        mappingPtr->phys = (GT_PHYSICAL_ADDR)res;

    return GT_OK;
}

#else /* defined(SHARED_MEMORY) */
GT_STATUS mbus_map_resource(
    IN  int                                mbusResource,
    IN  GT_UINTPTR                         maxSize,
    OUT CPSS_HW_INFO_RESOURCE_MAPPING_STC *mappingPtr
)
{
    void *vaddr;
    unsigned long long res;
    FILE *f;

    if (mvMbusDrvReadConfig(mbusResource | MV_RESOURCE_SIZE, &res) < 0)
        return GT_NOT_FOUND;
    if (maxSize > (GT_UINTPTR)res || maxSize == 0)
        maxSize = (GT_UINTPTR)res;

    vaddr = (void*)prvNoKmDrv_resource_virt_addr;
    prvNoKmDrv_resource_virt_addr += maxSize;
    vaddr = mmap(vaddr,
                maxSize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_FIXED,
                mvMbusDrvFd,
                (off_t)(mbusResource << SYSTEM_PAGE_SHIFT));
    if (MAP_FAILED == vaddr)
    {
        return GT_FAIL;
    }

    mappingPtr->start = (GT_UINTPTR)vaddr;
    mappingPtr->size  = (GT_UINTPTR)maxSize;
    if (mvMbusDrvReadConfig(mbusResource | MV_RESOURCE_START, &res) >= 0)
        mappingPtr->phys = (GT_PHYSICAL_ADDR)res;

    f = fopen(SHMEM_PP_MAPPINGS_FILENAME, "a");
    if (f == NULL)
        return GT_FAIL;
    fprintf(f, "MBUS %p %p %p\n",
            vaddr,
            (void*)((GT_UINTPTR)maxSize),
            (void*)((GT_UINTPTR)mbusResource));
    fclose(f);
    return GT_OK;
}
#endif /* defined(SHARED_MEMORY) */

/**
* @internal prvExtDrvMbusConfigure function
* @endinternal
*
* @brief   This routine maps all PP resources to userspace and detects IRQ
*
* @param[out] hwInfoPtr                - pointer to HW info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvMbusConfigure
(
    IN  GT_U32              flags,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_U32 i;
    unsigned long long r;

    /* TODO: get devVend for quirks */

    memset(hwInfoPtr, 0, sizeof(*hwInfoPtr));
    hwInfoPtr->busType = CPSS_HW_INFO_BUS_TYPE_MBUS_E;
    hwInfoPtr->hwAddr.busNo = 0xff;
    hwInfoPtr->hwAddr.devSel = 0xff;
    hwInfoPtr->hwAddr.funcNo = 0xff;

    for (i = 0; mbusResources[i].cpss_resource_offset >= 0; i++)
    {
        GT_UINTPTR maxSize = mbusResources[i].maxSize;
        if ((mbusResources[i].mbus_resource == MV_RESOURCE_MBUS_SWITCH)
            && (flags & MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E))
        {
            maxSize = _4M;
        }
        mbus_map_resource(
                mbusResources[i].mbus_resource, maxSize,
                (CPSS_HW_INFO_RESOURCE_MAPPING_STC*)
                    (((GT_UINTPTR)hwInfoPtr)+mbusResources[i].cpss_resource_offset));
        }

    prvNoKmDrv_configure_dma(hwInfoPtr->resource.switching.start, 1);

    hwInfoPtr->irq.switching = 0x11ab0000 | (noKmMappingsNum & 0xffff);
    if (mvMbusDrvReadConfig(MV_RESOURCE_MBUS_SWITCH_IRQ | MV_RESOURCE_START, &r) > 0)
    {
        hwInfoPtr->irq.switching = (GT_U32)r;
    }
#ifdef NOKM_DRV_EMULATE_INTERRUPTS
    if (prvExtDrvMvIntDrvConnected() == GT_FALSE)
    {
        hwInfoPtr->irq.switching = 0x11ab0000 | (noKmMappingsNum & 0xffff);
    }
#endif
    hwInfoPtr->intMask.switching = (GT_UINTPTR)hwInfoPtr->irq.switching;

    noKmMappingsList[noKmMappingsNum] = *hwInfoPtr;
    noKmMappingsNum++;

    return GT_OK;
}

void prvNoKmMbusRemap(
    void *vaddr,
    void *vsize,
    int res
)
{
    void *vaddrm;
    if (!mvMbusDrvOpen())
        return;
    vaddrm = mmap(vaddr,
                (size_t)((GT_UINTPTR)vsize),
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_FIXED,
                mvMbusDrvFd,
                (off_t)(res << SYSTEM_PAGE_SHIFT));
    if (MAP_FAILED == vaddrm)
    {
        perror("can't mmap MBUS resource");
    }
}

/**
* @internal prvExtDrvIsMbusDevExists function
* @endinternal
*
* @brief   This routine returns GT_TRUE if MBus device found
*
* @param[in] devId                    - The device Id.
*                                       GT_TRUE, GT_FALSE
*/
GT_BOOL prvExtDrvIsMbusDevExists
(
    IN  GT_U16  devId
)
{
    unsigned long long r;
    if (mvMbusDrvReadConfig(MV_RESOURCE_DEV_ID | MV_RESOURCE_START, &r) <= 0)
        return GT_FALSE;

    return ((GT_U16)r == devId) ? GT_TRUE : GT_FALSE;
}



