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
* @file prvTgfTunnelTermIpv6overIpv4.h
*
* @brief Tunnel Term: Ipv6 over Ipv4 - Basic
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfTunnelTermIpv6overIpv4h
#define __prvTgfTunnelTermIpv6overIpv4h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTunnelTermIpv6overIpv4BaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelTermIpv6overIpv4BaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv6overIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv6overIpv4RouteConfigurationSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelTermIpv6overIpv4TtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] useUdbConf               - tti rule is udb
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv6overIpv4TtiConfigurationSet
(
    GT_U32      prvUtfVrfId,
    GT_BOOL     useUdbConf
);

/**
* @internal prvTgfTunnelTermIpv6overIpv4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermIpv6overIpv4TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv6overIpv4ConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] useUdbConf               - tti rule is udb
*                                       None
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermIpv6overIpv4ConfigurationRestore
(
    GT_U32      prvUtfVrfId,
    GT_BOOL     useUdbConf
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermIpv6overIpv4h */


