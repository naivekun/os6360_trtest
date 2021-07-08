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
* @file prvTgfTunnelStartTrillKnownMc.h
*
* @brief Tunnel: TRILL Known Multicast
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelStartTrillKnownMch
#define __prvTgfTunnelStartTrillKnownMch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelStartTrillKnownMcConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelStartTrillKnownMcConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartTrillKnownMcTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelStartTrillKnownMcTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunneStartlTrillKnownMcRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunneStartlTrillKnownMcRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartTrillKnownMch */
