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
* @file cstTunnelStartMplsVpws.h
*
* @brief Basic VPWS test.
*
* @version   1
********************************************************************************
*/
#ifndef __cstTunnelStartMplsVpwsh
#define __cstTunnelStartMplsVpwsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY

/**
* @internal cstTunnelStartMplsVpwsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - One tagged packet and one untagged packet
*
* @param[in] isTaggedPacket           - GT_TRUE:send a tagged packet
*                                      GT_FALSE:send a NON tagged packet
*                                       None
*/
GT_VOID cstTunnelStartMplsVpwsTrafficGenerate
(
    GT_BOOL     isTaggedPacket
);

/**
* @internal cstTunnelStartMplsVpwsConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_STATUS cstTunnelStartMplsVpwsConfigRestore
(
    GT_VOID
);

/**
* @internal cstTunnelStartMplsVpwsConfiguration function
* @endinternal
*
* @brief   see file description.
*/
GT_STATUS cstTunnelStartMplsVpwsConfiguration
(
    GT_VOID
);

/**
* @internal csTunnelStartMplsVpwsRoutingTest function
* @endinternal
*
* @brief   see file description.
*/
GT_STATUS csTunnelStartMplsVpwsRoutingTest
(
    GT_VOID
);

#endif /*CHX_FAMILY*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cstTunnelStartMplsVpwsh */


