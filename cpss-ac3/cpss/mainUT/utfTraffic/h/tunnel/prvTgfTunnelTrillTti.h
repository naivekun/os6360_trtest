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
* @file prvTgfTunnelTrillTti.h
*
* @brief Tunnel: TRILL tunnel termination
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelTrillTtih
#define __prvTgfTunnelTrillTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelTrillKnownUcDataToTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiAllTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelTrillKnownUcDataToTtiAllTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTrillKnownUcDataToTtiConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiConfigurationDsaTagEnableSet function
* @endinternal
*
* @brief   enable/disable running the test in DSA tag mode.
*
* @param[in] enable                   - enable/disable running the test in DSA tag mode.
* @param[in] dsaBypassBridge          - relevant when enable = TRUE
*                                      indicate that the traffic with 'forward' DSA tag do or not
*                                      'bypass' to the bridge.
*                                       None
*/
GT_VOID prvTgfTunnelTrillKnownUcDataToTtiConfigurationDsaTagEnableSet
(
    IN GT_BOOL      enable,
    IN GT_BOOL      dsaBypassBridge
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTrillTtih */

