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
* @file genDrvPciDrv.c
*
* @brief Includes PCI functions wrappers implementation.
*
* @version   11
********************************************************************************
*/
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <private/8245/gtCore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>

#include "kerneldrv/include/presteraGlob.h"

#ifdef SHARED_MEMORY
#   include <gtOs/gtOsSharedPp.h>
#endif

/*******************************************************************************
* internal definitions
*******************************************************************************/
#ifdef PRESTERA_DEBUG
#define PCI_DRV_DEBUG
#endif

#ifdef PCI_DRV_DEBUG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif

/* TODO: file descriptor type is int (POSIX) */
extern GT_32 gtPpFd;
#if __GNUC__ >= 4
unsigned int volatile gtInternalRegBaseAddr = 0;
#else
unsigned int gtInternalRegBaseAddr = 0;
#endif

/**
* @internal extDrvPciConfigWriteReg function
* @endinternal
*
* @brief   This routine write register to the PCI configuration space.
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] regAddr                  - Register offset in the configuration space.
* @param[in] data                     -  to write.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciConfigWriteReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    struct PciConfigReg_STC pciConfReg;
    
    pciConfReg.busNo = busNo;
    pciConfReg.devSel = devSel;
    pciConfReg.funcNo = funcNo;
    pciConfReg.regAddr = regAddr;
    pciConfReg.data = data;
    
    /* call driver function */
    if (ioctl(gtPpFd, PRESTERA_IOC_PCICONFIGWRITEREG, &pciConfReg) < 0)
    {
        return GT_FAIL;
    }
    
    return GT_OK;
}


/**
* @internal extDrvPciConfigReadReg function
* @endinternal
*
* @brief   This routine read register from the PCI configuration space.
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] regAddr                  - Register offset in the configuration space.
*
* @param[out] data                     - the read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciConfigReadReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    struct PciConfigReg_STC pciConfReg;
    
    pciConfReg.busNo = busNo;
    pciConfReg.devSel = devSel;
    pciConfReg.funcNo = funcNo;
    pciConfReg.regAddr = regAddr;
    
    /* call driver function */
    if (ioctl(gtPpFd, PRESTERA_IOC_PCICONFIGREADREG, &pciConfReg) < 0)
    {
        return GT_FAIL;
    }
    
    *data = pciConfReg.data;
    
    return GT_OK;
}

static int prvPciNumMappingsDone = 0;
static pthread_mutex_t prvPciMappingMtx = PTHREAD_MUTEX_INITIALIZER;

/**
* @internal prvExtDrvDoMapping function
* @endinternal
*
* @brief   Map all allocated to userspace
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note Linux only
*
*/
static GT_STATUS prvExtDrvDoMapping(GT_VOID)
{
    int                     rc;
    struct GT_PCI_MMAP_INFO_STC    mmapInfo;
    void*                   mapped;
    /* map all */
    pthread_mutex_lock(&prvPciMappingMtx);
    while (1)
    {
        mmapInfo.index = prvPciNumMappingsDone;
        rc = ioctl(gtPpFd, PRESTERA_IOC_GETMMAPINFO, &mmapInfo);
        if (rc < 0)
            break;

        mapped = mmap((void*)((uintptr_t)mmapInfo.addr + (size_t)mmapInfo.offset),
                            (size_t)mmapInfo.length,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_FIXED,
                            gtPpFd,
                            (size_t)mmapInfo.offset);

        if (MAP_FAILED == mapped)
        {
            pthread_mutex_unlock(&prvPciMappingMtx);
            return GT_FAIL;
        }
        prvPciNumMappingsDone++;
    }
    pthread_mutex_unlock(&prvPciMappingMtx);

    return GT_OK;
}

/**
* @internal extDrvInitDrv function
* @endinternal
*
* @brief   Open mvPP device,
*         If first client initialize it (set virtual addresses for userspace)
*         Map all to userspace
*
* @param[out] isFirstClient
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note Linux only
*
*/
GT_STATUS extDrvInitDrv
(
    OUT GT_BOOL  *isFirstClient
)
{
    GT_BOOL first = GT_FALSE;
    struct GT_PCI_VMA_ADDRESSES_STC vmConfig;
    int rc;

    if (gtPpFd >= 0)
        return GT_ALREADY_EXIST;
    gtPpFd = open("/dev/mvPP", O_RDWR);
    if (gtPpFd < 0)
    {
        fprintf(stderr, "Cannot open /dev/mvPP for Read-Write, error=%d\n", errno);
        fprintf(stderr, "Possible reasons:\n");
        fprintf(stderr, "  *** No driver loaded\n");
        fprintf(stderr, "  *** Driver not initialized because no Marvell PCI devices found\n");
        fprintf(stderr, "      Check /proc/bus/pci/devices\n");
        return GT_FAIL;
    }

    rc = ioctl(gtPpFd, PRESTERA_IOC_ISFIRSTCLIENT, &first);
    if (rc == 0)
        first = GT_TRUE;
    /*TODO: rc < 0  => ??? */
    if (isFirstClient != NULL)
    {
        *isFirstClient = first;
    }
    if (rc < 0)
    {
        return GT_OK;
    }

    if (ioctl(gtPpFd, PRESTERA_IOC_GETVMA, &vmConfig) == 0)
    {
        vmConfig.dmaBase = LINUX_VMA_DMABASE;
        vmConfig.ppConfigBase = LINUX_VMA_PP_CONF_BASE;
        vmConfig.ppRegsBase = LINUX_VMA_PP_REGS_BASE;
        vmConfig.ppDfxBase = LINUX_VMA_PP_DFX_BASE;
#ifdef LINUX_VMA_DRAGONITE
        vmConfig.xCatDraginiteBase = LINUX_VMA_DRAGONITE;
#endif
#ifdef LINUX_VMA_HSU
        vmConfig.hsuBaseAddr = LINUX_VMA_HSU;
#endif
        ioctl(gtPpFd, PRESTERA_IOC_SETVMA, &vmConfig);
    }

    return prvExtDrvDoMapping();
}

#ifdef PP_DYNAMIC_MAPPING
#ifdef SHARED_MEMORY
# error "PP_DYNAMIC_MAPPING can't be used with SHARED_MEMORY"
#endif
/**
* @internal prvExtDrvPciMapDyn function
* @endinternal
*
* @brief   This routine maps PP registers and PCI registers into userspace
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] barNo                    - 0 for config, 1 for regs, 2 for dfx
* @param[in] mapSize                  - map size (64M for PCI)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciMapDyn
(
    IN  GT_U32                  busNo,
    IN  GT_U32                  devSel,
    IN  GT_U32                  funcNo,
    IN  GT_U32                  barNo,
    IN  GT_UINTPTR              mapSize
)
{
    void*   mapped;
    off_t   mapId;
    char    fname[128];
    struct stat st;

#ifndef SYSTEM_PAGE_SHIFT
#define SYSTEM_PAGE_SHIFT 12
#endif

    sprintf(fname, "/sys/bus/pci/devices/0000:%02x:%02x.%x/resource%d",
            busNo, devSel, funcNo, barNo*2);
    if (stat(fname, &st) == 0)
    {
        if ((off_t)mapSize > st.st_size)
            mapSize = (GT_UINTPTR)st.st_size;
        if ((off_t)mapSize < st.st_size && barNo == 2)
        {
            mapSize = (GT_UINTPTR)st.st_size;
            if (mapSize > _8M)
                mapSize = _8M;
        }
    }

    mapId = ((busNo  & 0xff) << 10) | /* 8 bit */
            ((devSel & 0x1f) << 5) | /* 5 bit */
            ((funcNo & 0x07) << 2) | /* 3 bit */
             (barNo  & 0x03) | /* 2 bit */
             (1 << (31-SYSTEM_PAGE_SHIFT)); /* mark: bit31 == 1 */

    mapId <<= SYSTEM_PAGE_SHIFT;

    mapped = mmap(NULL, (size_t)mapSize,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            gtPpFd,
            mapId);

    if (MAP_FAILED == mapped)
    {
        return GT_FAIL;
    }
    return GT_OK;
}
#endif /* PP_DYNAMIC_MAPPING */

/**
* @internal extDrvPciMap function
* @endinternal
*
* @brief   This routine maps PP registers and PCI registers into userspace
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] regsSize                 - registers size (64M for PCI)
*
* @param[out] mapPtr                   - The pointer to all mappings
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciMap
(
    IN  GT_U32                  busNo,
    IN  GT_U32                  devSel,
    IN  GT_U32                  funcNo,
    IN  GT_UINTPTR              regsSize,
    OUT GT_EXT_DRV_PCI_MAP_STC  *mapPtr
)
{
    struct GT_PCI_Mapping_STC mapping;
    GT_STATUS rc;

    if (mapPtr == NULL)
        return GT_BAD_PARAM;
    memset(mapPtr, 0, sizeof(*mapPtr));

#ifdef PP_DYNAMIC_MAPPING
    rc = prvExtDrvPciMapDyn(busNo, devSel, funcNo, 0, _1M);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvExtDrvPciMapDyn(busNo, devSel, funcNo, 1, regsSize);
    if (rc != GT_OK)
    {
        return rc;
    }
    prvExtDrvPciMapDyn(busNo, devSel, funcNo, 2, _8M);
#endif

    mapping.busNo = busNo;
    mapping.devSel = devSel;
    mapping.funcNo = funcNo;
    mapping.regsSize = regsSize;

    /* call driver function to get mapping info*/
    if (ioctl(gtPpFd, PRESTERA_IOC_GETMAPPING, &mapping) < 0)
    {
        return GT_FAIL;
    }
    /* map all allocated (if not mapped yet) */
    rc = prvExtDrvDoMapping();
    if (rc != GT_OK)
    {
        return rc;
    }

    mapPtr->regs.size = (GT_UINTPTR)mapping.mapRegs.length;
    mapPtr->regs.base = (GT_UINTPTR) mapping.mapRegs.addr;

    mapPtr->config.size = (GT_UINTPTR)mapping.mapConfig.length;
    mapPtr->config.base = (GT_UINTPTR)mapping.mapConfig.addr;

    mapPtr->dfx.size = (GT_UINTPTR)mapping.mapDfx.length;
    mapPtr->dfx.base = (GT_UINTPTR)mapping.mapDfx.addr;

    if (mapPtr->dfx.size >= _4M)
    {
        mapPtr->sram.base = mapPtr->dfx.base + _2M;
        /* Special value for size: depends on PP */
        mapPtr->sram.size = 0xffffffff;
    }
    return GT_OK;
}

/**
* @internal extDrvPexConfigure function
* @endinternal
*
* @brief   This routine maps PP registers and PCI registers into userspace
*         then fill CPSS_HW_INFO_STC
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] flags                    - flags
*
* @param[out] hwInfoPtr                - The pointer to HW info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPexConfigure
(
    IN  GT_U32                  busNo,
    IN  GT_U32                  devSel,
    IN  GT_U32                  funcNo,
    IN  GT_U32                  flags,
    OUT CPSS_HW_INFO_STC        *hwInfoPtr
)
{
    GT_EXT_DRV_PCI_MAP_STC map;
    GT_STATUS rc;

    memset(hwInfoPtr, 0, sizeof(*hwInfoPtr));
    hwInfoPtr->busType = CPSS_HW_INFO_BUS_TYPE_PEX_E;
    hwInfoPtr->hwAddr.busNo = busNo;
    hwInfoPtr->hwAddr.devSel = devSel;
    hwInfoPtr->hwAddr.funcNo = funcNo;
    if (busNo == 0xff && devSel == 0xff && funcNo == 0xff)
        hwInfoPtr->busType = CPSS_HW_INFO_BUS_TYPE_MBUS_E;

    if ((flags & MV_EXT_DRV_CFG_FLAG_DONT_MAP_E) == 0)
    {
        rc = extDrvPciMap(busNo, devSel, funcNo,
                (flags & MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E) ? _4M : 64 * _1M,
                &map);
        if (rc != GT_OK)
        {
            printf("Failed to extDrvPciMap(): rc=%d\n", rc);
            return rc;
        }
        hwInfoPtr->resource.cnm.start                    = map.config.base;
        hwInfoPtr->resource.cnm.size                     = map.config.size;
        hwInfoPtr->resource.switching.start              = map.regs.base;
        hwInfoPtr->resource.switching.size               = map.regs.size;
        hwInfoPtr->resource.resetAndInitController.start = map.dfx.base;
        hwInfoPtr->resource.resetAndInitController.size  = map.dfx.size;
        hwInfoPtr->resource.sram.start                   = map.sram.base;
        hwInfoPtr->resource.sram.size                    = map.sram.size;
    }

    rc = extDrvPcieGetInterruptNumber(busNo, devSel, funcNo,
        &(hwInfoPtr->irq.switching));
    if (rc != GT_OK)
    {
        printf("Failed to extDrvPcieGetInterruptNumber(): rc=%d\n", rc);
    }
    hwInfoPtr->intMask.switching = (GT_UINTPTR)hwInfoPtr->irq.switching;
    return rc;
}


/**
* @internal extDrvPciFindDev function
* @endinternal
*
* @brief   This routine returns the next instance of the given device (defined by
*         vendorId & devId).
* @param[in] vendorId                 - The device vendor Id.
* @param[in] devId                    - The device Id.
* @param[in] instance                 - The requested device instance.
*
* @param[out] busNo                    - PCI bus number.
* @param[out] devSel                   - the device devSel.
* @param[out] funcNo                   - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciFindDev
(
    IN  GT_U16  vendorId,
    IN  GT_U16  devId,
    IN  GT_U32  instance,
    OUT GT_U32  *busNo,
    OUT GT_U32  *devSel,
    OUT GT_U32  *funcNo
)
{
    struct GT_PCI_Dev_STC dev;
    
    dev.vendorId = vendorId;
    dev.devId    = devId;
    dev.instance = instance;
    
    /* call driver function */
    if (ioctl(gtPpFd, PRESTERA_IOC_FIND_DEV, &dev) < 0)
    {
        return GT_FAIL;
    }
    
    *busNo  = dev.busNo;
    *devSel = dev.devSel;
    *funcNo = dev.funcNo;
    
    return GT_OK;
}


/**
* @internal extDrvGetPciIntVec function
* @endinternal
*
* @brief   This routine return the PCI interrupt vector.
*
* @param[in] pciInt                   - PCI interrupt number.
*
* @param[out] intVec                   - PCI interrupt vector.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetPciIntVec
(
    IN  GT_PCI_INT  pciInt,
    OUT void        **intVec
)
{
    struct GT_Intr2Vec int2vec;
    
    /* check parameters */
    if(intVec == NULL)
    {
        return GT_BAD_PARAM;
    }
    /* get the PCI interrupt vector */
    /* call driver function */
    int2vec.intrLine = (GT_U32)pciInt;
    int2vec.bus = 0;
    int2vec.device = 0;
    int2vec.vector = 0;
    if (ioctl(gtPpFd, PRESTERA_IOC_GETINTVEC, &int2vec) < 0)
    {
        return GT_FAIL;
    }
    *intVec = (void *)((GT_UINTPTR)int2vec.vector);
    
    /* check whether a valid value */
    if((*intVec) == NULL)
    {
        DBG_INFO(("Failed in gtPciIntrToVecNum\n"));
        return GT_FAIL;
    }
    return GT_OK;
}


/**
* @internal extDrvGetIntMask function
* @endinternal
*
* @brief   This routine return the PCI interrupt vector.
*
* @param[in] pciInt                   - PCI interrupt number.
*
* @param[out] intMask                  - PCI interrupt mask.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*
* @note PCI interrupt mask should be used for interrupt disable/enable.
*
*/
GT_STATUS extDrvGetIntMask
(
    IN  GT_PCI_INT  pciInt,
    OUT GT_UINTPTR  *intMask
)
{
    void        *intVec;

    /* check parameters */
    if(intMask == NULL)
    {
        return GT_BAD_PARAM;
    }
    /* get the PCI interrupt vector */
    extDrvGetPciIntVec(pciInt, &intVec);

    *intMask = (GT_UINTPTR)intVec;

    return GT_OK;
}

/**
* @internal extDrvPcieGetInterruptNumber function
* @endinternal
*
* @brief   This routine returns interrupt number for PCIe device
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
*
* @param[out] intNumPtr                - Interrupt number value
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPcieGetInterruptNumber
(
    IN  GT_U32      busNo,
    IN  GT_U32      devSel,
    IN  GT_U32      funcNo,
    OUT GT_U32     *intNumPtr
)
{
    /* use /proc/bus/pci/devices */
    FILE *f;
    char s[512];
    unsigned pciAddr, devVend, irq;
    unsigned pciAddrQ;

    if (busNo == 0xff && devSel == 0xff && funcNo == 0xff)
    {
        void *intVec;
        GT_STATUS rc;
        /* MSYS local connected PP */
        rc = extDrvGetPciIntVec(GT_PCI_INT_A, &intVec);
        if (rc != GT_OK)
        {
            printf("Failed to extDrvGetPciIntVec(GT_PCI_INT_A,&intVec): rc=%d\n", rc);
            return rc;
        }
        *intNumPtr = (GT_U32)((GT_UINTPTR)intVec);
        return GT_OK;
    }
    f = fopen("/proc/bus/pci/devices","r");
    if (f == NULL)
        return GT_BAD_PARAM;
    pciAddrQ = ((busNo & 0xff) << 8) |
               ((devSel & 0x1f) << 3) |
               (funcNo & 0x07);
    while (!feof(f))
    {
        if (fgets(s, sizeof(s), f) == NULL)
            break;
        if (sscanf(s, "%x %x %x", &pciAddr, &devVend, &irq) < 3)
            continue;
        if (pciAddr != pciAddrQ)
            continue;
        /* found */
        *intNumPtr = irq;
        fclose(f);
        return GT_OK;
    }
    fclose(f);
    return GT_FAIL;
}


/**
* @internal extDrvEnableCombinedPciAccess function
* @endinternal
*
* @brief   This function enables / disables the Pci writes / reads combining
*         feature.
*         Some system controllers support combining memory writes / reads. When a
*         long burst write / read is required and combining is enabled, the master
*         combines consecutive write / read transactions, if possible, and
*         performs one burst on the Pci instead of two. (see comments)
* @param[in] enWrCombine              - GT_TRUE enables write requests combining.
* @param[in] enRdCombine              - GT_TRUE enables read requests combining.
*
* @retval GT_OK                    - on sucess,
* @retval GT_NOT_SUPPORTED         - if the controller does not support this feature,
* @retval GT_FAIL                  - otherwise.
*
* @note 1. Example for combined write scenario:
*       The controller is required to write a 32-bit data to address 0x8000,
*       while this transaction is still in progress, a request for a write
*       operation to address 0x8004 arrives, in this case the two writes are
*       combined into a single burst of 8-bytes.
*
*/
GT_STATUS extDrvEnableCombinedPciAccess
(
    IN  GT_BOOL     enWrCombine,
    IN  GT_BOOL     enRdCombine
)
{
    return GT_OK;
}

/**
* @internal extDrvPciDoubleWrite function
* @endinternal
*
* @brief   This routine will write a 64-bit data to given address
*
* @param[in] address                  -  to write to
* @param[in] word1                    - the first half of double word to write (MSW)
* @param[in] word2                    - the second half of double word to write (LSW)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvPciDoubleWrite
(
    IN  GT_U32 address,
    IN  GT_U32 word1, 
    IN  GT_U32 word2
)
{
	return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvPciDoubleRead function
* @endinternal
*
* @brief   This routine will read a 64-bit data from given address
*
* @param[in] address                  -  to read from
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvPciDoubleRead
(
    IN  GT_U32  address,
    OUT GT_U64  *dataPtr
)
{
	return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvPexRemove function
* @endinternal
*
* @brief   This routine deletes the PCI HW Information
*
* @param[in] busNo        - PCI bus number.
* @param[in] devSel       - the device devSel.
* @param[in] funcNo       - function number.
*
* @retval GT_OK           - on success,
* @retval GT_FAIL         - othersise.
*/
GT_STATUS extDrvPexRemove
(
    IN  GT_U32 busNo,
     IN  GT_U32 devSel,
     IN  GT_U32 funcNo
 )
{
    return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvPciConfigDev function
* @endinternal
*
* @brief   This routine enables PCI device
*
* @param[in] pciBus                - PCI bus number.
* @param[in] pciDev                - the device devSel.
* @param[in] pciFunc               - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciConfigDev
(
    IN GT_U32  pciBus,
    IN GT_U32  pciDev,
    IN GT_U32  pciFunc
)
{
    return GT_OK;
}
