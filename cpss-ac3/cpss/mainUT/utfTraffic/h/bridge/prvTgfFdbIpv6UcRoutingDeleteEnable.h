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
* @file prvTgfFdbIpv6UcRoutingDeleteEnable.h
*
* @brief Fdb ipv4 uc routing checking delete enable/disable functionality
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbIpv6UcRoutingDeleteEnableh
#define __prvTgfFdbIpv6UcRoutingDeleteEnableh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfFdbIpv6UcRoutingDeleteEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingDeleteEnableConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv6UcRoutingDeleteEnableSet function
* @endinternal
*
* @brief   Enable/Disable delete of UC entries
*
* @param[in] enable                   - GT_TRUE:  Enable delete of UC entries
*                                      GT_FALSE: Disable delete of UC entries
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingDeleteEnableSet
(
    GT_BOOL enable
);

/**
* @internal prvTgfFdbIpv6UcRoutingDeleteEnableTrafficGenerate function
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
GT_VOID prvTgfFdbIpv6UcRoutingDeleteEnableTrafficGenerate
(
    GT_BOOL  expectTraffic
);

/**
* @internal prvTgfFdbIpv6UcRoutingDeleteEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingDeleteEnableConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv6UcRoutingDeleteEnableh */


