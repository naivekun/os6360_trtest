/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBasicIpNonExactMatchRouting.h
*
* DESCRIPTION:
*       Non-exact match IPv4 UC prefix routing.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfIpNonExactMatchRoutingIpv4Uc
#define __prvTgfIpNonExactMatchRoutingIpv4Uc

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfIpGen.h>

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4UcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv4UcBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4UcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv4UcRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4UcTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv4UcTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4UcConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore Route Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv4UcConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpNonExactMatchRoutingIpv4Uc */

