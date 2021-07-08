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
* @file prvTgfTunnelStartTrillUnknownUcFlooding.h
*
* @brief Tunnel: TRILL tunnel start unknown uc flooding
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelStartTrillUnknownUcFloodingh
#define __prvTgfTunnelStartTrillUnknownUcFloodingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelStartTrillUnknownUcFloodingConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelStartTrillUnknownUcFloodingConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartTrillUnknownUcFloodingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelStartTrillUnknownUcFloodingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartTrillUnknownUcFloodingConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelStartTrillUnknownUcFloodingConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartTrillUnknownUcFloodingh */

