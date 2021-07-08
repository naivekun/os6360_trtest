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
* @file prvTgfIpv4Uc2MultipleVr.h
*
* @brief Create 1959 virtual routers. Add one prefix matching route per VR.
* A regular route of every VR routes packets with IP DA = 1.1.1.3 into
* vlan 3 (TX_VLAN_CNS). A default route of VR routes packets into vlan 4
* (DEFAULT_TX_VLAN_CNS). A packet MAC DA is set to 00:00:00:0f:xx:yy,
* where xx= vrId/100, yy = vrId%100 (i.e. MAC DA is unique for VR).
*
* To generate traffic we:
* - place an rx port (index 0) into 24 vlans (VLANS_COUNT_CNS).
* - place a tx port (index 1) into vlans 3,4 (TX_VLAN_CNS,
* DEFAULT_TX_VLAN_CNS).
* - in cycle through all vrId with step=24 assign unique vrId to
* every of 24 vlan.
* - generate traffic with burst = 24 twice(vlan id will be changed
* incrementally in every packet): with IP = 1.1.1.3 (to be routed by
* a regular route) and with IP 1.1.15.15 (routed by a default route).
* - check tx port counters and MAC DA, vlanId of captured packets.
* Test is not applicable for EXMXPM devices.
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4Uc2MultipleVrh
#define __prvTgfIpv4Uc2MultipleVrh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfIpv4Uc2MultipleVrBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv4Uc2MultipleVrBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4Uc2MultipleVrRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] createAllVrBeforeAddingPrefixes - whether to add the prefixes only after
*                                      all the virtual routers are created
*                                       None
*/
GT_VOID prvTgfIpv4Uc2MultipleVrRouteConfigurationSet
(
    GT_BOOL createAllVrBeforeAddingPrefixes
);

/**
* @internal prvTgfIpv4Uc2MultipleVrTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpv4Uc2MultipleVrTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpv4Uc2MultipleVrConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpv4Uc2MultipleVrConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4Uc2MultipleVrh */


