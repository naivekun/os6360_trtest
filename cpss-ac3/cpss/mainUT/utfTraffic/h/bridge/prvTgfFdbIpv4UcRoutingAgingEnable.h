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
* @file prvTgfFdbIpv4UcRoutingAgingEnable.h
*
* @brief Fdb ipv4 uc routing checking Aging enable/disable functionality
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbIpv4UcRoutingAgingEnableh
#define __prvTgfFdbIpv4UcRoutingAgingEnableh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <common/tgfBridgeGen.h>


/**
* @internal prvTgfFdbIpv4UcRoutingAgingEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingEnableConfigurationSet
(
    GT_BOOL routingByIndex
);

/**
* @internal prvTgfFdbIpv4UcRoutingAgingEnableSet function
* @endinternal
*
* @brief   Enable/Disable Aging of UC entries
*
* @param[in] agingWithRemoval         - GT_TRUE: age with removal
*                                      GT_FALSE: age without removal
* @param[in] enableFdbRoutingAging    - GT_TRUE:  Enable Aging of UC entries
*                                      GT_FALSE: Disable Aging of UC entries
* @param[in] enableFdbRoutingAAandTAToCpu - GT_TRUE  - AA and TA messages are not
*                                      forwarded to the CPU for UC route entries.
*                                      GT_FALSE - AA and TA messages are not
*                                      forwarded to the CPU for
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingEnableSet
(
    GT_BOOL     agingWithRemoval,
    GT_BOOL     enableFdbRoutingAging,
    GT_BOOL     enableFdbRoutingAAandTAToCpu

);

/**
* @internal prvTgfFdbIpv4UcRoutingAgingEnableTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's sendPortNum packet:
*         Success Criteria:
*         if expectTraffic== GT_TRUE --> IPv4 Uc Packet is captured on port 2
*         if expectTraffic== GT_FALSE --> IPv4 Uc Packet is not captured on port 2
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingEnableTrafficGenerate
(
    GT_U8    sendPortNum,
    GT_U8    nextHopPortNum,
    GT_BOOL  expectTraffic
);

/**
* @internal prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues function
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
GT_VOID prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues
(
    GT_BOOL expectedValid,
    GT_BOOL expectedSkip,
    GT_BOOL expectedAge
);

/**
* @internal prvTgfFdbIpv4UcRoutingAgingEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingEnableConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv4UcRoutingAgingEnableh */


