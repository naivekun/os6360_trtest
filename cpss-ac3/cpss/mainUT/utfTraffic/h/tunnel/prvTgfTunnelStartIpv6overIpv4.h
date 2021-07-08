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
* @file prvTgfTunnelStartIpv6overIpv4.h
*
* @brief Tunnel Start: Ipv6 over Ipv4 - Basic
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTunnelStartIpv6overIpv4h
#define __prvTgfTunnelStartIpv6overIpv4h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTunnelStartIpv6overIpv4BaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelStartIpv6overIpv4BaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIpv6overIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelStartIpv6overIpv4RouteConfigurationSet
(
    GT_U32  prvUtfVrfId
);

/**
* @internal prvTgfTunnelStartIpv6overIpv4TunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartIpv6overIpv4TunnelConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIpv6overIpv4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartIpv6overIpv4TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIpv6overIpv4ConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelStartIpv6overIpv4ConfigurationRestore
(
    GT_U32  prvUtfVrfId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartIpv6overIpv4h */


