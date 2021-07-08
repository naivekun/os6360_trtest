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
*       $Revision: 4 $
*
*******************************************************************************/
#ifndef __prvTgfpv4UcRoutingAddManyh
#define __prvTgfpv4UcRoutingAddManyh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerate();


/**
* @internal prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState function
* @endinternal
*
* @brief   Add Prefixes Configuration, generate traffic
*         and check LPM Activity State
*/
GT_VOID prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState();


/**
* @internal prvTgfIpv4UcRoutingAddManyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] ipAddr                   - ip adders of the packet
* @param[in] nextHopVlanId            - nextHop VLAN
*                                       None
*/
GT_VOID prvTgfIpv4UcRoutingAddManyTrafficGenerate
(
    GT_U32               sendPortNum,
    GT_U32               nextHopPortNum,
    GT_IPADDR           ipAddr,
    GT_U16              nextHopVlanId
);

/**
* @internal prvTgfIpv4UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpv4UcRoutingAddManyConfigurationRestore();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfpv4UcRoutingAddManyh */


