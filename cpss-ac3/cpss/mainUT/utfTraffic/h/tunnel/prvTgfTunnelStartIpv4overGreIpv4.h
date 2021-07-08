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
* @file prvTgfTunnelStartIpv4overGreIpv4.h
*
* @brief Tunnel Start: Ipv4 over GRE Ipv4 - Basic
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTunnelStartIpv4overGreIpv4h
#define __prvTgfTunnelStartIpv4overGreIpv4h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTunnelStartIpv4overGreIpv4BaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4BaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4RouteConfigurationSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4TunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4TunnelConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4ConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*
* @note 3. Restore Tunnel Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4ConfigurationRestore
(
    GT_U32      prvUtfVrfId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartIpv4overGreIpv4h */


