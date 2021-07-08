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
* @file prvTgfFdbIpv4UcRoutingLookupMask.h
*
* @brief Fdb ipv4 uc routing lookup mask
*
* @version   5
********************************************************************************
*/
#ifndef __prvTgfFdbIpv4UcRoutingLookupMaskh
#define __prvTgfFdbIpv4UcRoutingLookupMaskh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskRouteConfigurationSet
(
    GT_BOOL routingByIndex
);

/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB lookup prefix length
*
* @param[in] prefixLen  - FDB lookup prefix length
*
*/
GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskPrefixLengthSet
(
    GT_U32 prefixLen
);

/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskTest function
* @endinternal
*
* @brief   Test FDB IPv4 Lookup Mask
*/
GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskTest
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv4UcRoutingLookupMaskh */
