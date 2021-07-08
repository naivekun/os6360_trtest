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
* @file prvTgfBasicIpv4UcRouting.h
*
* @brief Basic IPV4 UC Routing
*
* @version   17
********************************************************************************
*/
#ifndef __prvTgfBasicIpv4UcRoutingPriorityh
#define __prvTgfBasicIpv4UcRoutingPriorityh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <common/tgfLpmGen.h>
#include <common/tgfIpGen.h>

/**
* @internal prvTgfIpv4UcRoutingPriorityBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] fdbPortNum               - the FDB port num to set in the macEntry destination Interface
*                                       None
*/
GT_VOID prvTgfIpv4UcRoutingPriorityBaseConfigurationSet
(
    GT_U32   fdbPortNum
);

/**
* @internal prvTgfIpv4UcRoutingPriorityBasicRoutingConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
* @param[in] enableIpv4McRouting      - whether to enable IPv4 MC routing in the virtual router,
*                                      not relevant for policy based routing.
* @param[in] enableIpv6McRouting      - whether to enable IPv6 MC routing in the virtual router,
*                                      not relevant for policy based routing.
*                                       None
*/
GT_VOID prvTgfIpv4UcRoutingPriorityBasicRoutingConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum
);

/**
* @internal prvTgfIpv4UcRoutingPriorityTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] expectFdbRouting         - expect FDB routing if value is GT_TRUE 
*
*/
GT_VOID prvTgfIpv4UcRoutingPriorityTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL   expectFdbRouting
);

/**
* @internal prvTgfIpv4UcRoutingPriorityFdbRoutingGenericConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfIpv4UcRoutingPriorityFdbRoutingGenericConfigurationSet
(
    GT_U8   sendPortIndex
);

/**
* @internal prvTgfIpv4UcRoutingPriorityFdbRoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv4UcRoutingPriorityFdbRoutingConfigurationSet
(
    GT_BOOL routingByIndex
);

/**
* @internal prvTgfIpv4UcRoutingPrioritySetLeafPriority function
* @endinternal
*
* @brief   Set routing priority between LPM and FDB loopup
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] priority                 - priority
*
*/
GT_VOID prvTgfIpv4UcRoutingPrioritySetLeafPriority
(
    GT_U32 prvUtfVrfId,
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT priority
);

/**
* @internal prvTgfIpv4UcRoutingPriorityConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortIndex         - port index of next hop port
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpv4UcRoutingPriorityConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv4UcRoutingPriorityh */


