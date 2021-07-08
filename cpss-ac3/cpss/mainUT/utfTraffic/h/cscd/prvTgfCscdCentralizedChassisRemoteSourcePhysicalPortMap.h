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
* @file prvTgfCscdCentralizedChassisRemoteSourcePhysicalPortMap.h
*
* @brief Mapping of a Source physical port to a remote physical port that
* resides over a DSA-tagged interface
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdCentralizedChassisRemoteSourcePhysicalPortMap
#define __prvTgfCscdCentralizedChassisRemoteSourcePhysicalPortMap

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal 
*           prvTgfDSARemotePhysicalPortMapConfigurationSet
*           function
* @endinternal
*
* @brief   Set test configuration for remote port over DSA tag.
*/
GT_VOID prvTgfDSARemotePhysicalPortMapConfigurationSet
(
    GT_VOID
);


/**
* @internal 
*           prvTgfDSARemotePhysicalPortMapTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate and check traffic. 
*/
GT_VOID prvTgfDSARemotePhysicalPortMapTrafficGenerate
(
    GT_VOID
);


/**
* @internal 
*           prvTgfDSARemotePhysicalPortMapConfigurationRestore
*           function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfDSARemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
);


/**
* @internal 
*           prvTgfVlanRemotePhysicalPortMapConfigurationSet
*           function
* @endinternal
*
* @brief  Set test configuration for remote port over VLAN tag.
*/
GT_VOID prvTgfVlanRemotePhysicalPortMapConfigurationSet
(
    GT_VOID
);


/**
* @internal 
*           prvTgfVlanRemotePhysicalPortMapTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate and check traffic
*/
GT_VOID prvTgfVlanRemotePhysicalPortMapTrafficGenerate
(
    GT_VOID
);


/**
* @internal 
*           prvTgfVlanRemotePhysicalPortMapConfigurationRestore
*           function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfVlanRemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
);


/**
* @internal 
*           prvTgfEtagRemotePhysicalPortMapConfigurationSet
*           function
* @endinternal
*
* @brief  Set test configuration for remote port over E tag.
*/
GT_VOID prvTgfEtagRemotePhysicalPortMapConfigurationSet
(
    GT_VOID
);


/**
* @internal 
*           prvTgfEtagRemotePhysicalPortMapTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate and check traffic
*/
GT_VOID prvTgfEtagRemotePhysicalPortMapTrafficGenerate
(
    GT_VOID
);


/**
* @internal 
*           prvTgfEtagRemotePhysicalPortMapConfigurationRestore
*           function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfEtagRemotePhysicalPortMapConfigurationRestore
(
    GT_VOID
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdCentralizedChassisRemotePhysicalPortMap */


