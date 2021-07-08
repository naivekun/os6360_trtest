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
* @file noKmDrvPciDrv.c
*
* @brief Includes PCI functions wrappers implementation.
*
* @version   1
********************************************************************************
*/

#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#include <gtExtDrv/drivers/prvExtDrvLinuxMapping.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include "prvNoKmDrv.h"

CPSS_HW_INFO_STC noKmMappingsList[NOKM_DEVICES_MAX];
GT_U32 noKmMappingsNum = 0;
GT_U16 prvNoKmDevId = 0xFFFF;

#ifdef SHARED_MEMORY
GT_UINTPTR prvNoKmDrv_resource_virt_addr = LINUX_VMA_PP_REGS_BASE;
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
    return prvExtDrvPciSysfsConfigWriteReg(busNo>>8, busNo & 0xff, devSel, funcNo, regAddr, data);
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
    if (IS_MBUS_ADDR(busNo,devSel,funcNo))
    {
        return extDrvPciMbusConfigReadReg(regAddr, data);
    }

    return prvExtDrvPciSysfsConfigReadReg(busNo>>8, busNo & 0xff, devSel, funcNo, regAddr, data);
}



/**
* @internal extDrvInitDrv function
* @endinternal
*
* @brief   If first client initialize it
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
#ifdef SHARED_MEMORY
    FILE *f;
    char resname[128];
    void *vaddr, *vsize, *offset;
    static int initialized = 0;
    GT_UINTPTR dmaBase;

    if (initialized)
    {
        if (isFirstClient != NULL)
        {
            *isFirstClient = (GT_BOOL)(initialized-1);;
        }
        return GT_OK;
    }

    f = fopen(SHMEM_PP_MAPPINGS_FILENAME, "r");
    if (f == NULL)
    {
        if (isFirstClient != NULL)
        {
            *isFirstClient = GT_TRUE;
        }
        initialized = (int)GT_TRUE+1;
        return prvExtDrvSysfsInitDevices();
    }
    if (isFirstClient != NULL)
    {
        *isFirstClient = GT_FALSE;
    }
    initialized = (int)GT_FALSE+1;

    while (!feof(f))
    {
        int n, res;
        n = fscanf(f, "%s %p %p %p", resname, &vaddr, &vsize, &offset);
        if (n != 4)
            continue;
        if (strcmp(resname, "MBUS") == 0)
        {
            res = (int)((GT_UINTPTR)offset);
            prvNoKmMbusRemap(vaddr, vsize, res);
        }
        else
        {
            prvNoKmSysfsRemap(resname, vaddr, vsize, offset);
        }
    }
    fclose(f);
    extDrvGetDmaBase(&dmaBase);
    return GT_OK;
#else /* !defined(SHARED_MEMORY) */
    if (isFirstClient != NULL)
    {
        *isFirstClient = GT_TRUE;
    }
    return prvExtDrvSysfsInitDevices();
#endif
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
    GT_STATUS rc;
    CPSS_HW_INFO_STC hwInfo;

    rc = extDrvPexConfigure(busNo, devSel, funcNo,
        (regsSize == _4M) ? MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E : 0,
        &hwInfo);
    if (rc != GT_OK)
        return rc;

    mapPtr->config.base = hwInfo.resource.cnm.start;
    mapPtr->config.size = hwInfo.resource.cnm.size;
    mapPtr->regs.base   = hwInfo.resource.switching.start;
    mapPtr->regs.size   = hwInfo.resource.switching.size;
    mapPtr->dfx.base    = hwInfo.resource.resetAndInitController.start;
    mapPtr->dfx.size    = hwInfo.resource.resetAndInitController.size;
    mapPtr->sram.base   = hwInfo.resource.sram.start;
    mapPtr->sram.size   = hwInfo.resource.sram.size;
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
    GT_STATUS rc;
    GT_U32 i;

#define R hwInfoPtr->resource
    /* check if already mapped */
    for (i = 0; i < noKmMappingsNum; i++)
    {
        if (noKmMappingsList[i].hwAddr.busNo != busNo)
            continue;
        if (noKmMappingsList[i].hwAddr.devSel != devSel)
            continue;
        if (noKmMappingsList[i].hwAddr.funcNo != funcNo)
            continue;
        *hwInfoPtr = noKmMappingsList[i];
        if (!(flags & MV_EXT_DRV_CFG_FLAG_ALLOW_NO_PP_E))
        {
            if (R.switching.start == 0 || R.cnm.start == 0)
                return GT_FAIL;
        }
        return GT_OK;
    }


    if (noKmMappingsNum >= NOKM_DEVICES_MAX)
        return GT_NO_RESOURCE;

    if (IS_MBUS_ADDR(busNo,devSel,funcNo))
    {
        rc = prvExtDrvMbusConfigure(flags, hwInfoPtr);
    }
    else
    {
        rc = prvExtDrvPciSysfsConfigure(busNo>>8, busNo & 0xff, devSel, funcNo, flags, hwInfoPtr);
    }

    if (rc != GT_OK)
        return rc;

    if (!(flags & MV_EXT_DRV_CFG_FLAG_ALLOW_NO_PP_E))
    {
        if (R.switching.start == 0 || R.cnm.start == 0)
            return GT_FAIL;
    }
#undef R

    return GT_OK;
}


#ifdef NOKM_DRV_EMULATE_INTERRUPTS
/*******************************************************************************
* prvExtDrvNokmGetMappingByIntNum
*
* DESCRIPTION:
*       This routine returns userspace mapping for interrupt emulation
*
* INPUTS:
*       intNum   - a generated interrupt
*                  0x11ab0000 | (index_in_noKmMappingsList & 0xffff)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to structure or NULL
*
* COMMENTS:
*
*******************************************************************************/
CPSS_HW_INFO_STC* prvExtDrvNokmGetMappingByIntNum(GT_U32 intNum)
{
    GT_U32 i;
    for (i = 0; i < noKmMappingsNum; i++)
    {
        if (noKmMappingsList[i].irq.switching == intNum)
            return &(noKmMappingsList[i]);
    }
    return NULL;
}
#endif /* NOKM_DRV_EMULATE_INTERRUPTS */

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
    GT_STATUS rc;
    if (prvExtDrvIsMbusDevExists(devId) == GT_TRUE)
    {
        if (instance == 0)
        {
            *busNo = 0xff;
            *devSel = 0xff;
            *funcNo = 0xff;
            prvNoKmDevId = devId; /* Register 1st found devic */
            return GT_OK;
        }
        instance--;
    }

    rc =  prvExtDrvSysfsFindDev(vendorId, devId, instance,
            busNo, devSel, funcNo);
    if (rc == GT_OK && instance == 0)
            prvNoKmDevId = devId; /* Register 1st found devic */
    return rc;

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
    return GT_FAIL;
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
    return GT_FAIL;
}

/**
* @internal extDrvPcieGetInterruptNumber function
* @endinternal
*
* @brief   This routine returns interrupt vector/mask for PCIe device
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
    GT_STATUS rc;
    CPSS_HW_INFO_STC hwInfo;

    rc = extDrvPexConfigure(busNo, devSel, funcNo,
        MV_EXT_DRV_CFG_FLAG_ALLOW_NO_PP_E,
        &hwInfo);
    if (rc != GT_OK)
        return rc;
    *intNumPtr = hwInfo.irq.switching;
    return GT_OK;
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
    GT_U32 i;

    for (i = 0; i < noKmMappingsNum; i++)
    {
        if ((noKmMappingsList[i].hwAddr.busNo == busNo) &&
            (noKmMappingsList[i].hwAddr.devSel == devSel) &&
            (noKmMappingsList[i].hwAddr.funcNo == funcNo))
                break;
    }
    if(i == noKmMappingsNum)
        return GT_BAD_PARAM;

    /*unmap PCI BAR resources*/
#define R noKmMappingsList[i].resource
    prvNoKmSysfsUnmap((void*)R.cnm.start, R.cnm.size);
    prvNoKmSysfsUnmap((void*)R.switching.start, R.switching.size);
    prvNoKmSysfsUnmap((void*)R.resetAndInitController.start, R.resetAndInitController.size);
#undef R

    for (; i < noKmMappingsNum-1; i++)
        noKmMappingsList[i] = noKmMappingsList[i+1];

    memset(&noKmMappingsList[i], 0, sizeof(CPSS_HW_INFO_STC));
    noKmMappingsNum--;

    return GT_OK;
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
    return prvExtDrvSysfsConfigDev(pciBus,pciDev,pciFunc);
}
