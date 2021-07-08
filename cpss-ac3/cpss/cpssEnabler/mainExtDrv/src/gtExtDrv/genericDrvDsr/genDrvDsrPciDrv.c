/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/


#include <prestera/os/gtTypes.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h> /* for extDrvSmiDevVendorIdGet */
#include <gtExtDrv/drivers/pssBspApis.h>


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
#ifdef GT_PCI
    return bspPciConfigWriteReg(busNo, devSel, funcNo, regAddr, data);
#else
    return GT_NOT_SUPPORTED;
#endif
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
#ifdef GT_PCI
    return bspPciConfigReadReg(busNo, devSel, funcNo, regAddr, data);
#else
    return GT_NOT_SUPPORTED;
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
#ifdef GT_PCI
    GT_U32 baseAddr;
    GT_U32 internalBase;
    GT_STATUS ret;

    if (mapPtr == NULL)
        return GT_BAD_PARAM;
    mapPtr->regs.base = 0;
    mapPtr->regs.size = 0;
    mapPtr->config = mapPtr->dfx = mapPtr->regs;

    /* get device BAR 0 */
    ret = extDrvPciConfigReadReg(busNo,devSel,funcNo,0x14,&baseAddr);
    if(ret != GT_OK)
    {
        return GT_FAIL;
    }

    /* Get internal base address    */
    ret = extDrvPciConfigReadReg(busNo,devSel,funcNo,0x10,&internalBase);
    if(ret != GT_OK)
    {
        return GT_FAIL;
    }

    mapPtr->regs.size = 64*1024*1024;
    osPhy2Virt((GT_UINTPTR)baseAddr, &(mapPtr->regs.base));
    mapPtr->config.size = 1*1024*1024;
    osPhy2Virt((GT_UINTPTR)internalBase, &(mapPtr->config.base));

    return GT_OK;
#else
    return GT_NOT_SUPPORTED;
#endif
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
#if defined(GT_PCI)
    return bspPciFindDev(vendorId, devId, instance, busNo, devSel, funcNo);
#elif defined(GT_SMI)
    return extDrvSmiDevVendorIdGet(&vendorId,&devId,instance);
#else
    return GT_NOT_SUPPORTED;
#endif
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
#ifdef GT_PCI
    /* assumption: The enums bspPciInt_PCI_INT and GT_PCI_INT are identical. */
    return bspPciGetIntVec(pciInt, intVec);
#else
    return GT_NOT_SUPPORTED;
#endif
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
#ifdef GT_PCI
    GT_U32 intMaskTmp;
    GT_STATUS ret;
    ret = bspPciGetIntMask(pciInt, &intMaskTmp);
    *intMask = (GT_UINTPTR)intMaskTmp;
    return ret;
#else
    return GT_NOT_SUPPORTED;
#endif
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
#ifdef GT_PCI
    return bspPciEnableCombinedAccess(enWrCombine, enRdCombine);
#else
    return GT_OK;
#endif
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

