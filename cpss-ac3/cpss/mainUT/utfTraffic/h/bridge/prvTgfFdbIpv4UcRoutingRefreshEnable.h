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
* @file prvTgfFdbIpv4UcRoutingRefreshEnable.h
*
* @brief Fdb ipv4 uc routing checking refresh enable/disable functionality
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbIpv4UcRoutingRefreshEnableh
#define __prvTgfFdbIpv4UcRoutingRefreshEnableh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfFdbIpv4UcRoutingRefreshEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingRefreshEnableConfigurationSet
(
    IN GT_BOOL routingByIndex,
    IN GT_U32   nexthop_2_port_idx
);

/**
* @internal prvTgfFdbIpv4UcRoutingRefreshEnableSet function
* @endinternal
*
* @brief   Enable/Disable delete of UC entries
*
* @param[in] enable                   - GT_TRUE:  Enable delete of UC entries
*                                      GT_FALSE: Disable delete of UC entries
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingRefreshEnableSet
(
    GT_BOOL enable
);

/**
* @internal prvTgfFdbIpv4UcRoutingRefreshEnableTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's sendPortNum packet:
*         Success Criteria:
*         if expectTraffic== GT_TRUE --> IPv4 Uc Packet is captured on port 2,3
*         if expectTraffic== GT_FALSE --> IPv4 Uc Packet is not captured on port 2,3
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*                                      expectNoTraffic - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingRefreshEnableTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL  expectTraffic
);

/**
* @internal prvTgfFdbIpv4UcRoutingRefreshCheckValidSkipAgeValues function
* @endinternal
*
* @brief   Check the Valid/Skip/Age values of the inserted FDB Entry is as expected
*
* @param[in] expectedValid            - GT_TRUE: valid is 1
*                                      GT_FALSE: valid is 0
* @param[in] expectedSkip             - GT_TRUE: skip is 1
*                                      GT_FALSE: skip is 0
* @param[in] expectedAge              - GT_TRUE: age is 1
*                                      GT_FALSE: age is 0
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingRefreshCheckValidSkipAgeValues
(
    GT_BOOL expectedValid,
    GT_BOOL expectedSkip,
    GT_BOOL expectedAge
);

/**
* @internal prvTgfFdbIpv4UcRoutingRefreshEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv4UcRoutingRefreshEnableConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv4UcRoutingRefreshEnableh */


