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
* @file prvTgfFdbIpv4UcRoutingTransplantEnable.h
*
* @brief Fdb ipv4 uc routing checking Transplant enable/disable functionality
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbIpv4UcRoutingTransplantEnableh
#define __prvTgfFdbIpv4UcRoutingTransplantEnableh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <common/tgfBridgeGen.h>


/**
* @internal prvTgfFdbIpv4UcRoutingTransplantEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingTransplantEnableConfigurationSet
(
    GT_BOOL routingByIndex
);

/**
* @internal prvTgfFdbIpv4UcRoutingTransplantEnableSet function
* @endinternal
*
* @brief   Enable/Disable Transplant of UC entries
*
* @param[in] enableFdbRoutingTransplant - GT_TRUE:  Enable Transplant of UC entries
*                                      GT_FALSE: Disable Transplant of UC entries
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingTransplantEnableSet
(
    GT_U32   oldPort,
    GT_U32   newPort,
    GT_BOOL enableFdbRoutingTransplant
);

/**
* @internal prvTgfFdbIpv4UcRoutingTransplantEnableTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's sendPortNum packet:
*         Success Criteria:
*         if expectTraffic== GT_TRUE --> IPv4 Uc Packet is captured on port 2,3
*         if expectTraffic== GT_FALSE --> IPv4 Uc Packet is not captured on port 2,3
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingTransplantEnableTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL  expectTraffic
);

/**
* @internal prvTgfFdbIpv4UcRoutingTransplantEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv4UcRoutingTransplantEnableConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv4UcRoutingTransplantEnableh */


