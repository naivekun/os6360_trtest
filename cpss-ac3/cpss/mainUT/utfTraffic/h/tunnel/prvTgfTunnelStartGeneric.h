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
* @file prvTgfTunnelStartGeneric.h
*
* @brief Tunnel Start: Generic tunnel-start entry functionality
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelStartGenerich
#define __prvTgfTunnelStartGenerich

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTunnelStartGenericBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelStartGenericBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartGenericRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelStartGenericRouteConfigurationSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelStartGenericTunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*
* @param[in] genType    - generic tunnel start type
*
*/
GT_VOID prvTgfTunnelStartGenericTunnelConfigurationSet
(
    PRV_TGF_TUNNEL_START_GENERIC_TYPE_ENT genType
);

/**
* @internal prvTgfTunnelStartGenericTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartGenericTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartGenericConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*
* @note 3. Restore Tunnel Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelStartGenericConfigurationRestore
(
    GT_U32      prvUtfVrfId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartGenerich */
