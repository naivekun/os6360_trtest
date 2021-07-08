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
* @file prvTgfFdbIpv6UcRoutingAgingEnable.h
*
* @brief Fdb ipv6 uc routing checking Aging enable/disable functionality
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbIpv6UcRoutingAgingEnableh
#define __prvTgfFdbIpv6UcRoutingAgingEnableh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <common/tgfBridgeGen.h>


/**
* @internal prvTgfFdbIpv6UcRoutingAgingEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingAgingEnableConfigurationSet
(
    GT_BOOL routingByIndex
);

/**
* @internal prvTgfFdbIpv6UcRoutingAgingEnableSet function
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
GT_VOID prvTgfFdbIpv6UcRoutingAgingEnableSet
(
    GT_BOOL     agingWithRemoval,
    GT_BOOL     enableFdbRoutingAging,
    GT_BOOL     enableFdbRoutingAAandTAToCpu

);

/**
* @internal prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues function
* @endinternal
*
* @brief   Check the Valid/Skip/Age values of the inserted FDB Entry is as expected
*
* @param[in] expectedValidAddress     -  GT_TRUE: valid is 1
*                                      GT_FALSE: valid is 0
* @param[in] expectedSkipAddress      -   GT_TRUE: skip is 1
*                                      GT_FALSE: skip is 0
* @param[in] expectedAgeAddress       -    GT_TRUE: age is 1
*                                      GT_FALSE: age is 0
* @param[in] expectedValidData        -     GT_TRUE: valid is 1
*                                      GT_FALSE: valid is 0
* @param[in] expectedSkipData         -      GT_TRUE: skip is 1
*                                      GT_FALSE: skip is 0
* @param[in] expectedAgeData          -       GT_TRUE: age is 1
*                                      GT_FALSE: age is 0
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues
(
    GT_BOOL expectedValidAddress,
    GT_BOOL expectedSkipAddress,
    GT_BOOL expectedAgeAddress,
    GT_BOOL expectedValidData,
    GT_BOOL expectedSkipData,
    GT_BOOL expectedAgeData
);

/**
* @internal prvTgfFdbIpv6UcRoutingAgingEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingAgingEnableConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv6UcRoutingAgingEnableh */


