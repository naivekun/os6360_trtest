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
* @file gtLinux.h
*
* @brief This header file contains simple read/write macros for addressing
* the SDRAM, devices, GT`s registers and PCI (using the PCI`s
* address space). The macros take care of Big/Little endian conversions.
*
*/

#ifndef __INGtLinux
#define __INGtLinux

/**
* @internal extDrvGetDmaBase function
* @endinternal
*
* @brief   Get the base address of the DMA area need for Virt2Phys and Phys2Virt
*         translaton
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
extern GT_STATUS extDrvGetDmaBase
(
	OUT GT_UINTPTR * dmaBase
);


#endif /* __INGtLinux */



