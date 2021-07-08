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
* @file cstIpBasicIpv4UcRouting.h
*
* @brief Basic IPV4 UC Routing
*
* @version   17
********************************************************************************
*/
#ifndef __cstIpBasicIpv4UcRoutingh
#define __cstIpBasicIpv4UcRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#ifdef CHX_FAMILY
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <common/tgfLpmGen.h>

/**
* @internal cstIpBasicIpv4UcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_STATUS cstIpBasicIpv4UcRoutingConfigurationRestore
(
    GT_VOID
);


/**
* @internal cstIpBasicIpv4UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID cstIpBasicIpv4UcRoutingTrafficGenerate
(
    GT_VOID
);


/**
* @internal cstIpBasicIpv4UcRoutingTest function
* @endinternal
*
* @brief   see file description.
*/
GT_STATUS cstIpBasicIpv4UcRoutingTest
(
    GT_VOID
);

/**
* @internal cstIpBasicIpv4UcRoutingConfiguration function
* @endinternal
*
* @brief   see file description.
*/
GT_STATUS cstIpBasicIpv4UcRoutingConfiguration
(
    GT_VOID
);

#endif /* CHX_FAMILY */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cstIpBasicIpv4UcRoutingh */


