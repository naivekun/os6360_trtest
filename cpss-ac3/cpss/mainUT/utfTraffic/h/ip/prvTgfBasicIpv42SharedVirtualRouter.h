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
* @file prvTgfBasicIpv42SharedVirtualRouter.h
*
* @brief Basic IPV4 UC 2 shared virtual Router
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBasicIpv42SharedVirtualRouterh
#define __prvTgfBasicIpv42SharedVirtualRouterh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBasicIpv42SharedVirtualRouterBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv42SharedVirtualRouterBaseConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv42SharedVirtualRouterRouteConfigSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfBasicIpv42SharedVirtualRouterRouteConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv42SharedVirtualRouterUcIpConfigSet function
* @endinternal
*
* @brief   Set UC IP Configuration
*/
GT_VOID prvTgfBasicIpv42SharedVirtualRouterUcIpConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv42NonSharedVirtualRouterMcIpConfigSet function
* @endinternal
*
* @brief   Set MC IP Configuration
*/
GT_VOID prvTgfBasicIpv42NonSharedVirtualRouterMcIpConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv42SharedVirtualRouterEnable function
* @endinternal
*
* @brief   Enable routing mechanism
*/
GT_VOID prvTgfBasicIpv42SharedVirtualRouterEnable
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv42SharedVirtualRouterTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBasicIpv42SharedVirtualRouterTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4Uc2SharedVRConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4Uc2SharedVRConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv42SharedVirtualRouterh */


