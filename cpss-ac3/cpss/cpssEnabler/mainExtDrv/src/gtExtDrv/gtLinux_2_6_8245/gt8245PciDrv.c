/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <private/8245/gtCore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include "kerneldrv/include/prestera_glob.h"

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

extern GT_32 gtPpFd;
unsigned int gtInternalRegBaseAddr = 0;

#define CONFIG_SIZE (4 * 1024)
#define REGS_SIZE (64 * 1024 * 1024)

/* PCI only, no quirks */
#if 0
/*
 * static struct prvPciDeviceQuirks prvPciDeviceQuirks[]
 *
 * Quirks can be added to GT_PCI_DEV_VENDOR_ID structure
 */
PRV_PCI_DEVICE_QUIRKS_ARRAY_MAC

static struct prvPciDeviceQuirks*
prvPciDeviceGetQuirks(unsigned vendorId, unsigned devId)
{
    int k;
    GT_U32  pciId;

    pciId = (devId << 16) | vendorId;

    for (k = 0; prvPciDeviceQuirks[k].pciId != 0xffffffff; k++)
    {
        if (prvPciDeviceQuirks[k].pciId == pciId)
            break;
    }
    return &(prvPciDeviceQuirks[k]);
}
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
    PciConfigReg_STC pciConfReg;
    
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
    PciConfigReg_STC pciConfReg;
    
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

    if (gtPpFd >= 0)
        return GT_ALREADY_EXIST;
    gtPpFd = open("/dev/mvPP", O_RDWR);
    if (gtPpFd < 0)
    {
        fprintf(stderr, "Cannot open /dev/mvPP for Read-Write, error=%d\n", errno);
        return GT_FAIL;
    }
    if (ioctl(gtPpFd, PRESTERA_IOC_ISFIRSTCLIENT, &first) == 0)
    {
        first = GT_TRUE;
    }
    if (isFirstClient != NULL)
        *isFirstClient = GT_TRUE;
    /*TODO: configure maping if first client */
    /*TODO: map all */
    return GT_OK;
}

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
    /* TODO */
    GT_U32 baseAddr;
    GT_U32 internalBase;
    /* PCI only, no quirks */
#if 0
    struct prvPciDeviceQuirks* quirks;
#endif
    static GT_U32 numOfDevices = 0;/* total number of devices in system */
#if defined(SHARED_MEMORY)
    GT_STATUS ret;
#endif

    if (mapPtr == NULL)
        return GT_BAD_PARAM;
    memset(mapPtr, 0, sizeof(mapPtr));

    numOfDevices++;

    /* PCI only, no quirks */
#if 0
    quirks = prvPciDeviceGetQuirks(vendorId, devId);
#endif

    /* map config space */
    internalBase = 0x60000000 + (numOfDevices - 1) * CONFIG_SIZE;
    internalBase = (GT_U32) mmap((void*)internalBase,
                            CONFIG_SIZE,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_FIXED,
                            gtPpFd, 0);

    if (MAP_FAILED == (void*)internalBase)
    {
        return GT_FAIL;
    }

    /* map register space */
    baseAddr = (GT_U32) mmap((void*)(0x70000000 + ((numOfDevices - 1) << 26)),
                            REGS_SIZE,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_FIXED,
                            gtPpFd,
                            0);

    if (MAP_FAILED == (void*)baseAddr)
    {
        return GT_FAIL;
    }

    /*TODO*/
    mapPtr->regs.base = (GT_UINTPTR)baseAddr;
    mapPtr->regs.size = REGS_SIZE;
    /*TODO*/
    mapPtr->config.base = (GT_UINTPTR)internalBase;
    mapPtr->config.size = PCI_CONFIG_SIZE;

#if defined(SHARED_MEMORY)
    /* We should note mapped section to restore mapping for all non-first clients */
    ret = shrMemRegisterPpMmapSection
    (
        (GT_VOID *)(mapPtr->regs.base),
        mapPtr->regs.size,
        0
    );
    if (GT_OK != ret) return ret;
    /* We should note mapped section to restore mapping for all non-first clients */
    ret = shrMemRegisterPpMmapSection
    (
        (GT_VOID *)(mapPtr->config.base),
        mapPtr->config.size,
        0
    );
    if (GT_OK != ret) return ret;
#endif

    return GT_OK;
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
    GT_PCI_Dev_STC dev;
    
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
    GT_Intr2Vec int2vec;
    
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
    *intVec = (void *) int2vec.vector;
    
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
#ifdef GT_PCI
    /* use /proc/bus/pci/devices */
    FILE *f;
    char s[512];
    unsigned pciAddr, devVend, irq;
    unsigned pciAddrQ;

    if (busNo == 0xff && devSel == 0xff && funcNo != 0xff)
    {
        /* MSYS, ask kernel module for interrupt */
        struct GT_Intr2Vec int2vec;

        /* get the PCI interrupt vector */
        /* call driver function */
        int2vec.intrLine = (GT_U32)PCI_INT_A;
        int2vec.bus = 0;
        int2vec.device = 0;
        int2vec.vector = 0;
        if (ioctl(gtPpFd, PRESTERA_IOC_GETINTVEC, &int2vec) < 0)
        {
            return GT_FAIL;
        }
        /* check whether a valid value */
        if (int2vec.vector == 0)
        {
            DBG_INFO(("Failed in gtPciIntrToVecNum\n"));
            return GT_FAIL;
        }
        *intNumPtr = int2vec.vector;

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
#else
    return GT_NOT_SUPPORTED;
#endif
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

