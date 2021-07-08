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
* @file prvTgfTunnelTermIpv4overIpv4Trap.h
*
* @brief Tunnel Term: Ipv4 over Ipv4 - Action Trap
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTunnelTermIpv4overIpv4Traph
#define __prvTgfTunnelTermIpv4overIpv4Traph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTunnelTermIpv4overIpv4TrapBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4TrapBaseConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4TrapIpConfigSet function
* @endinternal
*
* @brief   Set IP Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4TrapIpConfigSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4TrapTcamConfigSet function
* @endinternal
*
* @brief   Set TCAM Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4TrapTcamConfigSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4TrapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4TrapTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4TrapConfigRestore function
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
GT_VOID prvTgfTunnelTermIpv4overIpv4TrapConfigRestore
(
    GT_U32      prvUtfVrfId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermIpv4overIpv4Traph */


