/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfCscdCentralizedChassisRemotePhysicalPortMap.h
*
* @brief Mapping of a target physical port to a remote physical port that
* resides over a DSA-tagged interface
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdCentralizedChassisRemotePhysicalPortMap
#define __prvTgfCscdCentralizedChassisRemotePhysicalPortMap

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         1. Set VLAN and FDB entry
*         2. Set E2PHY entry
*         3. Enable and Set remote physical port mapping
*/
GT_VOID prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfCscdCentralizedChassisRemotePhysicalPortMapTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfCscdCentralizedChassisRemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdCentralizedChassisRemotePhysicalPortMap */


