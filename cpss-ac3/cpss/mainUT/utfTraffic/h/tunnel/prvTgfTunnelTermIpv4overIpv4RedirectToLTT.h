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
* @file prvTgfTunnelTermIpv4overIpv4RedirectToLTT.h
*
* @brief Tunnel Term: Ipv4 over Ipv4 - Redirect to LTT
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTunnelTermIpv4overIpv4RedirectToLTTh
#define __prvTgfTunnelTermIpv4overIpv4RedirectToLTTh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTunnelTermIpv4overIpv4RedirectToLttBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4RedirectToLttBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4RedirectToLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4RedirectToLttRouteConfigurationSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4RedirectToLttTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4RedirectToLttTtiConfigurationSet
(
    GT_U32      prvUtfVrfId
);


/**
* @internal prvTgfTunnelTermIpv4overIpv4RedirectToLttTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4RedirectToLttTrafficGenerate
(
    GT_VOID
);


/**
* @internal prvTgfTunnelTermIpv4overIpv4RedirectToLttConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4RedirectToLttConfigurationRestore
(
    GT_U32      prvUtfVrfId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ____prvTgfTunnelTermIpv4overIpv4RedirectToLTTh */


