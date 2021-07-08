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
* @file gtPciDrv.h
*
* @brief Includes PCI functions wrappers, for PCI device discovering and
* configuration read/write operation.
*
* @version   4
********************************************************************************
*/
#ifndef __gtPciDrvh
#define __gtPciDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/os/extDrvOs.h>
#include <cpss/generic/cpssHwInfo.h>

/**
* @enum GT_PCI_INT
 *
 * @brief Enumeration For PCI interrupt lines.
*/
typedef enum{

    /** PCI INT# A */
    GT_PCI_INT_A = 1,

    /** PCI INT# B */
    GT_PCI_INT_B,

    /** PCI INT# C */
    GT_PCI_INT_C,

    /** PCI INT# D */
    GT_PCI_INT_D

} GT_PCI_INT;

/**
* @struct GT_EXT_DRV_PCI_MAP_STC
 *
 * @brief The structure reflect PCI mapping for device
*/
typedef struct {
    struct {
        GT_UINTPTR  base;
        GT_UINTPTR  size;
    } regs, config, dfx, sram;
} GT_EXT_DRV_PCI_MAP_STC;

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
);



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
);

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
);

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
);

#define MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E  1
#define MV_EXT_DRV_CFG_FLAG_ALLOW_NO_PP_E    2
#define MV_EXT_DRV_CFG_FLAG_DONT_MAP_E       4
#define MV_EXT_DRV_CFG_FLAG_EAGLE_E          8
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
);

/**
* @internal extDrvPexConfigureClear function
* @endinternal
*
* @brief   This routine unmaps PP registers and PCI registers into userspace
*         then fill CPSS_HW_INFO_STC
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPexConfigureClear
(
    IN  GT_U32                  busNo,
    IN  GT_U32                  devSel,
    IN  GT_U32                  funcNo
);

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
);



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
);

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
);

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
);

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
);

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
);

/**
* @internal extDrvPciDoubleRead function
* @endinternal
*
* @brief   This routine will read a 64-bit data from given address
*
* @param[in] address                  -  to read from
*
* @param[out] dataPtr                  -  pointer to the received data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvPciDoubleRead
(
    IN  GT_U32  address,
    OUT GT_U64  *dataPtr
);

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
);

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
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtPciDrvh */




