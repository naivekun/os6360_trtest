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
* @file prvTgfIpv4UcRoutingHiddenPrefixes.h
*
* @brief LPM test for "hidden" prefixes
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4UcRoutingHiddenPrefixesh
#define __prvTgfIpv4UcRoutingHiddenPrefixesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfIpv4UcRoutingHiddenPrefixesBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv4UcRoutingHiddenPrefixesBaseConfigurationSet(GT_VOID);

/**
* @internal prvTgfIpv4UcRoutingHiddenPrefixesLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfIpv4UcRoutingHiddenPrefixesLttRouteConfigurationSet(GT_VOID);

/**
* @internal prvTgfIpv4UcRoutingHiddenPrefixesConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv4UcRoutingHiddenPrefixesConfigurationAndTrafficGenerate(GT_VOID);

/**
* @internal prvTgfIpv4UcRoutingHiddenPrefixesConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpv4UcRoutingHiddenPrefixesConfigurationRestore(GT_VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4UcRoutingHiddenPrefixesh */


