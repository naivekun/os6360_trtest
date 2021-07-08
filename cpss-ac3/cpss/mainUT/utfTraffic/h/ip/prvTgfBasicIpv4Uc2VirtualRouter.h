/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBasicIpv4UcRouting.h
*
* DESCRIPTION:
*       Basic IPV4 UC Routing
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#ifndef __prvTgfBasicIpv4Uc2VirtualRouterh
#define __prvTgfBasicIpv4Uc2VirtualRouterh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBasicIpv4Uc2VirtualRouterBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] prvUtfVrfId              - base value for vrfId
*                                       None
*/
GT_VOID prvTgfBasicIpv4Uc2VirtualRouterBaseConfigurationSet
(
    IN GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfBasicIpv4Uc2VirtualRouterRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfBasicIpv4Uc2VirtualRouterRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4Uc2VirtualRouterTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBasicIpv4Uc2VirtualRouterTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4Uc2VirtualRouterConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4Uc2VirtualRouterConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv4Uc2VirtualRouterh */


