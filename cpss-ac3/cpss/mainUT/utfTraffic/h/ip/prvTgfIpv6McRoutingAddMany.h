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
* @file prvTgfIpv6McRoutingAddMany.h
*
* @brief IPV6 MC Routing with prefix manipulations to fit compress_1,
* compress_2 and regular buckets
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv6McRoutingAddManygh
#define __prvTgfIpv6McRoutingAddManyh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerate();


/**
* @internal prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*         and check LPM Activity State
*/
GT_VOID prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState();


/**
* @internal prvTgfIpv6McRoutingAddManyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] srcIpAddr                - source IP Address of the packet sent
*                                       None
*/
GT_VOID prvTgfIpv6McRoutingAddManyTrafficGenerate
(
    GT_IPV6ADDR           srcIpAddr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv6McRoutingAddManyh */


