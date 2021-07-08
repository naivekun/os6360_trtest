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
* @brief PBR IPV4 UC Routing priority test
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4UcPbrRoutingPriotiryh
#define __prvTgfIpv4UcPbrRoutingPriotiryh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <common/tgfLpmGen.h>
#include <common/tgfIpGen.h>

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] prvUtfVrfId    - virtual router ID
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityBaseConfigurationSet
(
    GT_U32  prvUtfVrfId
);

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityBasicRoutingConfigurationSet function
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
GT_VOID prvTgfIpv4UcPbrRoutingPriorityBasicRoutingConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum
);

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] matchFdb              - match FDB route entry if value is GT_TRUE
* @param[in] expectFdbRouting      - expect FDB routing if value is GT_TRUE
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate
(
    GT_BOOL   matchFdb,
    GT_BOOL   expectFdbRouting
);

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityFdbRoutingGenericConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityFdbRoutingGenericConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityFdbRoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityFdbRoutingConfigurationSet
(
    GT_BOOL routingByIndex
);

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityPclConfigurationSet function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityPclConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet function
* @endinternal
*
* @brief   Set LPM leaf entry
*
* @param[in] priority              - LPM leaf entry priority
*
*/
GT_STATUS prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet
(
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT priority
);

/**
* @internal prvTgfIpv4UcPbrRoutingUnicastRouteForPbrEnable function
* @endinternal
*
* @brief   Enable FDB unicast routing for PBR
*
* @param[in] enable - enable value
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingUnicastRouteForPbrEnable
(
    GT_BOOL enable
);

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityConfigurationRestore function
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
GT_VOID prvTgfIpv4UcPbrRoutingPriorityConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4UcPbrRoutingPriotiryh */
