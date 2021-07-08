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
* @file prvTgfFdbIpv4UcPointerRoute.h
*
* @brief Fdb ipv4 uc routing with pointer route type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbIpv4UcPointerRouteh
#define __prvTgfFdbIpv4UcPointerRouteh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfFdbIpv4UcPointerRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcPointerRouteConfigurationSet
(
    GT_BOOL routingByIndex
);

/**
* @internal prvTgfFdbIpv4UcPointerRouteConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv4UcPointerRouteConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv4UcPointerRouteEcmpRoutingConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcPointerRouteEcmpRoutingConfigurationSet
(
    GT_BOOL randomEnable,
    GT_BOOL indirectAcessEnable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbIpv4UcPointerRouteh */
