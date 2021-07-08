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
* @file prvTgfFdbIpv6UcRoutingRefreshEnable.h
*
* @brief Fdb ipv6 uc routing checking refresh enable/disable functionality
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfFdbIpv6UcRoutingRefreshEnableh
#define __prvTgfFdbIpv6UcRoutingRefreshEnableh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfFdbIpv6UcRoutingRefreshEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingRefreshEnableConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv6UcRoutingRefreshEnableSet function
* @endinternal
*
* @brief   Enable/Disable delete of UC entries
*
* @param[in] enable                   - GT_TRUE:  Enable delete of UC entries
*                                      GT_FALSE: Disable delete of UC entries
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingRefreshEnableSet
(
    GT_BOOL enable
);

/**
* @internal prvTgfFdbIpv6UcRoutingRefreshEnableTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's sendPortNum packet:
*         Success Criteria:
*         if expectTraffic== GT_TRUE --> IPv6 Uc Packet is captured on port 2,3
*         if expectTraffic== GT_FALSE --> IPv6 Uc Packet is not captured on port 2,3
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingRefreshEnableTrafficGenerate
(
    GT_BOOL  expectTraffic
);

/**
* @internal prvTgfFdbIpv6UcRoutingRefreshCheckValidSkipAgeValues function
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
GT_VOID prvTgfFdbIpv6UcRoutingRefreshCheckValidSkipAgeValues
(
    GT_BOOL expectedValidAddress,
    GT_BOOL expectedSkipAddress,
    GT_BOOL expectedAgeAddress,
    GT_BOOL expectedValidData,
    GT_BOOL expectedSkipData,
    GT_BOOL expectedAgeData
);

/**
* @internal prvTgfFdbIpv6UcRoutingRefreshEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingRefreshEnableConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv6UcRoutingRefreshEnableh */


