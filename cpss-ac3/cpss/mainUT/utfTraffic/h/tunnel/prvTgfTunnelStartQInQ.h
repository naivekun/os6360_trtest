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
* @file prvTgfTunnelStartQInQ.h
*
* @brief Tunnel Start: Q In Q - Basic
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTunnelStartQInQh
#define __prvTgfTunnelStartQInQh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTunnelStartQInQConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelStartQInQConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartQInQRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelStartQInQRouteConfigurationSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelStartQInQTunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartQInQTunnelConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartQInQTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartQInQTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartQInQConfigurationRestore function
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
GT_VOID prvTgfTunnelStartQInQConfigurationRestore
(
    GT_U32      prvUtfVrfId
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartQInQh */


