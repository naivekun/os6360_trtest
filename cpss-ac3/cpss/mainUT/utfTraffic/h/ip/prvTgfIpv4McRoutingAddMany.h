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
* @file prvTgfIpv4McRoutingAddMany.h
*
* @brief IPV4 MC Routing with prefix manipulations to fit compress_1,
* compress_2 and regular buckets
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4McRoutingAddManyh
#define __prvTgfIpv4McRoutingAddManyh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerate();


/**
* @internal prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*         and check LPM Activity State
*/
GT_VOID prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState();


/**
* @internal prvTgfIpv4McRoutingAddManyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] dstIpAddr                - destination IP Address of the packet sent
* @param[in] srcIpAddr                - source IP Address of the packet sent
* @param[in] daMac                    - destination MAC address
* @param[in] saMac                    - source MAC address
*                                       None
*/
GT_VOID prvTgfIpv4McRoutingAddManyTrafficGenerate
(
    GT_IPADDR           dstIpAddr,
    GT_IPADDR           srcIpAddr,
    TGF_MAC_ADDR        daMac,
    TGF_MAC_ADDR        saMac
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4McRoutingAddManyh */


