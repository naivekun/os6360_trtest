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
* @file prvTgfTunnelStartTrillKnownUcEcmp.h
*
* @brief Tunnel: TRILL Known Unicast Multipath (ECMP)
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelStartTrillKnownUcEcmph
#define __prvTgfTunnelStartTrillKnownUcEcmph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpConfig2Set function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpConfig2Set
(
    GT_VOID
);

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpTraffic2Generate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpTraffic2Generate
(
    GT_VOID
);

/**
* @internal prvTgfTunneStartlTrillKnownUcEcmpRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunneStartlTrillKnownUcEcmpRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartTrillKnownUcEcmph */

