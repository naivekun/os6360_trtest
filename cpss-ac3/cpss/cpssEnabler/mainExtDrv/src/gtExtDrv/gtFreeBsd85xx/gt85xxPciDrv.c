/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <gtExtDrv/drivers/gtPciDrv.h>
#include <private/85xx/gtCore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>

#include "kerneldrv/include/prestera_glob.h"
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

#define PCIR_BARS        0x10
#define PCIR_BAR(x)     (PCIR_BARS + (x) * 4)

extern GT_32 gtPpFd;
unsigned int gtInternalRegBaseAddr = 0;
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
    GT_PCI_VMA_ADDRESSES_STC vmConfig;
    int rc;

    if (gtPpFd >= 0)
        return GT_ALREADY_EXIST;
    gtPpFd = open("/dev/mvPP", O_RDWR);
    if (gtPpFd < 0)
    {
        fprintf(stderr, "Cannot open /dev/mvPP for Read-Write, error=%d\n", errno);
        return GT_FAIL;
    }

    /* TODO */
    rc = ioctl(gtPpFd, PRESTERA_IOC_ISFIRSTCLIENT, &mapping);
    if (rc == 0)
        first = GT_TRUE;
    /*TODO: rc < 0  => ??? */
    if (isFirstClient != NULL)
    {
        *isFirstClient = first;
    }

    /* nothing to do: configure maping if first client */
    /* nothing to do: map all */

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
    GT_U32 baseAddr;
    GT_U32 internalBase;
    GT_U32 dfxBase;

    if (mapPtr == NULL)
        return GT_BAD_PARAM;
    mapPtr->regs.base = 0;
    mapPtr->regs.size = 0;
    mapPtr->config = mapPtr->dfx = mapPtr->regs;

    /* config address BAR0 */
    extDrvPciConfigReadReg(busNo, deviceNo, funcNo, PCIR_BAR(0), &internalBase);
    internalBase &= ~((GT_U32)0x3f);
    extDrvPhy2Virt(internalBase, &(mapPtr->config.base));
    mapPtr->config.size = 1*1024*1024;

    /* registers address BAR2 (there is no bar1) */
    extDrvPciConfigReadReg(busNo,deviceNo,funcNo, PCIR_BAR(2), &baseAddr);
    baseAddr &= ~((GT_U32)0x3f);
    extDrvPhy2Virt(baseAddr, &(mapPtr->regs.base));
    mapPtr->regs.size = 64*1024*1024;

    /* dfx */
    extDrvPciConfigReadReg(busNo,deviceNo,funcNo, PCIR_BAR(4), &dfxBase);
    dfxBase &= ~((GT_U32)0x3f);
    if (dfxBase != 0)
    {
        mapPtr->dfx.size = 1*1024*1024;
        extDrvPhy2Virt(baseAddr+64*1024*1024, &(mapPtr->dfx.base));
    }

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
    return GT_NOT_IMPLEMENTED;
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

