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
* @file prvCpssDriverCreate.h
*
* @brief Miscenaneuos CPSS HwPp drivers
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDriverCreate_h__
#define __prvCpssDriverCreate_h__

#include <cpss/generic/cpssHwInfo.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>

/**
* @internal cpssHwDriverPexMbusCreateDrv function
* @endinternal
*
* @brief Create driver instance for eArch PP
*        Resources are mapped to user-space
*
* @param[in] hwInfo             - HW info
* @param[in] compRegionMask     - the bitmap of address completion regions to use
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverPexMbusCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  GT_U32      compRegionMask
);


/**
* @internal cpssHwDriverPexCreateDrv function
* @endinternal
*
* @brief Create driver instance for PEX connected PP
*        (legacy 4-region address completion)
*        Resources are mapped to user-space
*
* @param[in] hwInfo             - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverPexCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
);

/**
* @internal cpssHwDriverEaglePexCreateDrv function
* @endinternal
*
* @brief Create driver instance for Eagle PEX - PP Resources are
*        mapped to user-space
*
* @param[in] hwInfo             - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverEaglePexCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
);

#ifdef ASIC_SIMULATION
/**
* @internal cpssHwDriverSimulationCreateDrv function
* @endinternal
*
* @brief Create driver instance for simulated PP
*
* @param[in] hwInfo         - HW info
* @param[in] compRegionMask - the bitmap of address completion regions to use
*                             If the bit N is set to 1 then region N can be used
*                             by driver
*                             The value 0xffffffff is used to indicate legacy
*                             4-region address completion
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSimulationCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  GT_U32      compRegionMask
);
#endif /* defined(ASIC_SIMULATION */

#endif /* __prvCpssDriverCreate_h__ */
