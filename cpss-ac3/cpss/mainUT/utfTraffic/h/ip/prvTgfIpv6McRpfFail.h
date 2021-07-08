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
* @file prvTgfIpv6McRpfFail.h
*
* @brief Basic IPV6 MC Routing
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfIpv6McRpfFailh
#define __prvTgfIpv6McRpfFailh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfIpv6McRpfFailBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv6McRpfFailBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv6McRpfFailRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpv6McRpfFailRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv6McRpfFailTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectNoTraffic          - whether to expect no traffic
*                                       None
*/
GT_VOID prvTgfIpv6McRpfFailTrafficGenerate
(
    GT_BOOL  expectNoTraffic
);

/**
* @internal prvTgfIpv6McRpfFailConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpv6McRpfFailConfigurationRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv6McRpfFailh */

