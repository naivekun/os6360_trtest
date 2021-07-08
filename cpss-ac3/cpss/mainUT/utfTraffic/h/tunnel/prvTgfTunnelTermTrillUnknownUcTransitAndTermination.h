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
* @file prvTgfTunnelTermTrillUnknownUcTransitAndTermination.h
*
* @brief Tunnel: TRILL tunnel termination
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelTermTrillUnknownUcTransitAndTerminationh
#define __prvTgfTunnelTermTrillUnknownUcTransitAndTerminationh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrillUnknownUcTransitAndTerminationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelTermTrillUnknownUcTransitAndTerminationTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermTrillUnknownUcTransitAndTerminationh */

