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
* @file prvTgfBasicIpv4UcEcmpRouting.h
*
* @brief Basic IPV4 UC Routing
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfBasicIpv4UcEcmpRoutingh
#define __prvTgfBasicIpv4UcEcmpRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBasicIpv4UcEcmpRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route and hashing Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingRouteConfigurationSet
(
    GT_BOOL randomEnable,
    GT_BOOL indirectAcessEnable
);

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingHashCheckRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route and hashing Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingHashCheckRouteConfigurationSet
(
    GT_BOOL randomEnable
);

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate
(
    GT_BOOL randomEnable
);

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingHashCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingHashCheckTrafficGenerate
(
    GT_BOOL randomEnable
);

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] enhancedHashCrcMode      - hash mode
*                                      GT_FALSE - default legacy mode
*                                      GT_TRUE  - enhanced CRC hash mode
*                                       None
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingConfigurationRestore
(
    GT_BOOL enhancedHashCrcMode
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv4UcEcmpRoutingh */


