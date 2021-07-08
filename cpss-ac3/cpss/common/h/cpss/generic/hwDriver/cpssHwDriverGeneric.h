/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssHwDriverAPI.h
*
* @brief generic HW driver implementations
*
* @version   1
********************************************************************************
*/
#ifndef __cpssHwDriverGeneric_h__
#define __cpssHwDriverGeneric_h__
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/generic/cpssHwInfo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SSMI_FALCON_ADDRESS_SPACE   0xFFFFFFFD

/**
* @internal cpssHwDriverGenMmapCreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*
* @param[in] base   - resource virtual address
* @param[in] size   - mapped resource size
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapCreateDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size
);

/**
* @internal cpssHwDriverGenMmapAc8CreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*        with 8-region address completion
*
* @param[in] base           - resource virtual address
* @param[in] size           - mapped resource size
* @param[in] compRegionMask - the bitmap of address completion regions to use
*                             If the bit N is set to 1 then region N can be used
*                             by driver
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note  size must be >= 0x80000
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapAc8CreateDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size,
    IN  GT_U32      compRegionMask
);

/**
* @internal cpssHwDriverGenMmapAc4CreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*        with 4-region address completion
*
* @param[in] base           - resource virtual address
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note  size must be 64M
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapAc4CreateDrv(
    IN  GT_UINTPTR  base
);

/**
* @internal cpssHwDriverSip5SlaveSMICreateDrv function
* @endinternal
*
* @brief Create driver instance for Sip5 Slave SMI
*
* @param[in] smi        - pointer to SMI driver
* @param[in] phy        - PP phy address
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSip5SlaveSMICreateDrv(
    IN  CPSS_HW_DRIVER_STC *smi,
    IN  GT_U32              phy
);

/**
* @internal cpssHwDriverSlaveSmiConfigure function
* @endinternal
*
* @brief   Configure Slave SMI device driver
*
* @param[in] smiMasterPath            - driver pointer
* @param[in] slaveSmiPhyId            - SMI Id of slave device
*
* @param[out] hwInfoPtr               - The pointer to HW info, will be used
*                                       for cpssDxChHwPpPhase1Init()
*                                       NULL to not fill HW info
*
* @retval GT_OK                       - on success
* @retval GT_OUT_OF_CPU_MEM
*/
GT_STATUS cpssHwDriverSlaveSmiConfigure(
    IN  const char         *smiMasterPath,
    IN  GT_U32              slaveSmiPhyId,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(__cpssHwDriverGeneric_h__) */

