/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBasicIpv6UcRouting.h
*
* DESCRIPTION:
*       Basic IPV6 UC Routing
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfpv6UcRoutingAddManyh
#define __prvTgfpv6UcRoutingAddManyh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerate();


/**
* @internal prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*         and check LPM Activity State
*/
GT_VOID prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState();


/**
* @internal prvTgfIpv6UcRoutingAddManyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] ipAddr                   - ip address
*                                       None
*/
GT_VOID prvTgfIpv6UcRoutingAddManyTrafficGenerate
(
    GT_IPV6ADDR           ipAddr
);

/**
* @internal prvTgfIpv6UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpv6UcRoutingAddManyConfigurationRestore();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfpv6UcRoutingAddManyh */


