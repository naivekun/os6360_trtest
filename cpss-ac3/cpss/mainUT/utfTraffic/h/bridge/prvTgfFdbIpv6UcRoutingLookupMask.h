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
* @file prvTgfFdbIpv6UcRoutingLookupMask.h
*
* @brief Fdb ipv4 uc routing lookup mask
*
* @version   5
********************************************************************************
*/
#ifndef __prvTgfFdbIpv6UcRoutingLookupMaskh
#define __prvTgfFdbIpv6UcRoutingLookupMaskh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfBridgeGen.h>


/**
* @internal prvTgfFdbIpv6UcRoutingLookupMaskRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv6UcRoutingLookupMaskRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB lookup prefix length
*
* @param[in] prefixLen - FDB lookup prefix length
*
*/
GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskPrefixLengthSet
(
    GT_U32 prefixLen
);

/**
* @internal prvTgfFdbIpv6UcRoutingLookupMaskTest function
* @endinternal
*
* @brief   Test FDB IPv6 Lookup Mask
*/
GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskTest
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv6UcRoutingLookupMaskConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv6UcRoutingLookupMaskh */
