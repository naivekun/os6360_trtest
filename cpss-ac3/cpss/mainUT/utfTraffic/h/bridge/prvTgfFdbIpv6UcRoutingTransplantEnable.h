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
* @file prvTgfFdbIpv6UcRoutingTransplantEnable.h
*
* @brief Fdb ipv6 uc routing checking Transplant enable/disable functionality
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbIpv6UcRoutingTransplantEnableh
#define __prvTgfFdbIpv6UcRoutingTransplantEnableh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <common/tgfBridgeGen.h>


/**
* @internal prvTgfFdbIpv6UcRoutingTransplantEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingTransplantEnableConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv6UcRoutingTransplantEnableSet function
* @endinternal
*
* @brief   Enable/Disable Transplant of UC entries
*
* @param[in] enableFdbRoutingTransplant - GT_TRUE:  Enable Transplant of UC entries
*                                      GT_FALSE: Disable Transplant of UC entries
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingTransplantEnableSet
(
    GT_U8   oldPort,
    GT_U8   newPort,
    GT_BOOL enableFdbRoutingTransplant
);

/**
* @internal prvTgfFdbIpv6UcRoutingTransplantEnableTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's sendPortNum packet:
*         Success Criteria:
*         if expectTraffic== GT_TRUE --> Ipv6 Uc Packet is captured on port 2,3
*         if expectTraffic== GT_FALSE --> Ipv6 Uc Packet is not captured on port 2,3
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingTransplantEnableTrafficGenerate
(
    GT_U8    sendPortNum,
    GT_U8    nextHopPortNum,
    GT_BOOL  expectTraffic
);

/**
* @internal prvTgfFdbIpv6UcRoutingTransplantEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingTransplantEnableConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv6UcRoutingTransplantEnableh */


