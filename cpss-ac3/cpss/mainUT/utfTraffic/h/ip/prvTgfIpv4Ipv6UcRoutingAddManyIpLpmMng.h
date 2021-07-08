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
* @file prvTgfIpv4Ipv6UcRoutingAddManyIpLpmMng.h
*
* @brief Ipv4 & IPV6 UC Routing when filling the Lpm using cpssDxChIpLpmIpv4UcPrefixAddManyByOctet & cpssDxChIpLpmIpv6UcPrefixAddManyByOctet.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4Ipv6UcRoutingAddManyIpLpmMngh
#define __prvTgfIpv4Ipv6UcRoutingAddManyIpLpmMngh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationAndTrafficGenerate(GT_VOID);

/**
* @internal prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpLpmMngIpv4Ipv6UcRoutingAddManyConfigurationRestore(GT_VOID);

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __prvTgfIpv4Ipv6UcRoutingAddManyIpLpmMngh */


