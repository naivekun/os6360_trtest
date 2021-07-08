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
* @file prvTgfIpv4UcRoutingAddDeleteAddRealPrefixes.h
*
* @brief IPV4 UC Routing adding, deleting and the adding again prefixes into the Lpm
* using real costumer prefixes defined in a file.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4UcRoutingAddDeleteAddRealPrefixesh
#define __prvTgfIpv4UcRoutingAddDeleteAddRealPrefixesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationAndTrafficGenerate(GT_VOID);

/**
* @internal prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] deleteFromTop            - delete the prefixes from bottom/top
*                                       None
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv4UcRoutingAddDeleteAddRealPrefixesConfigurationRestore
(
    IN GT_BOOL deleteFromTop
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4UcRoutingAddDeleteAddRealPrefixesh */

