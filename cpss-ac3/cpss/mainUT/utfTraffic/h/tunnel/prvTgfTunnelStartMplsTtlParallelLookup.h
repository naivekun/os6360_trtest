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
* @file prvTgfTunnelStartMplsTtlParallelLookup.h
*
* @brief Tunnel Start: Mpls functionality for Parallel Lookup
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelStartMplsTtlParallelLookuph
#define __prvTgfTunnelStartMplsTtlParallelLookuph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelMplsPopTtlParallelLookup function
* @endinternal
*
* @brief   MPLS LSR POP Functionality and TTL check for parallel Lookup
*
* @param[in] copyTtlExpFromTunnelHeaderArr[] - array of copyTtlExpFromTunnelHeader for lookups
* @param[in] enableDecrementTtlArr[]  - array of copyTtlExpFromTunnelHeader for lookups
* @param[in] lookup3Test              - testing forth lookup configuration
* @param[in] restoreConfig            - GT_TRUE:restore
*                                      GT_FALSE: do not restore
*                                       None
*/
GT_VOID prvTgfTunnelMplsPopTtlParallelLookup
(
    GT_BOOL copyTtlExpFromTunnelHeaderArr[],        
    GT_BOOL enableDecrementTtlArr[],        
    GT_BOOL lookup3Test,      
    GT_BOOL restoreConfig
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartMplsTtlParallelLookuph */


