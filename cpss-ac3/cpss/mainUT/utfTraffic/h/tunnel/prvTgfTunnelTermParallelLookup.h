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
* @file prvTgfTunnelTermParallelLookup.h
*
* @brief Tunnel Term: Ipv4 over Ipv4 - check parallel lookup
*
* @version   5
********************************************************************************
*/
#ifndef __prvTgfTunnelTermParallelLookuph
#define __prvTgfTunnelTermParallelLookuph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfTunnelGen.h>

/**
* @internal prvTgfTunnelTermParallelLookupBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupBaseConfigurationSet
(
    GT_U32     vrfId
);

/**
* @internal prvTgfTunnelTermParallelLookupRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermParallelLookupRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermParallelLookupTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
GT_VOID prvTgfTunnelTermParallelLookupTtiConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermParallelLookupRedirectRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermParallelLookupRedirectTtiRule0ConfigurationSet function
* @endinternal
*
* @brief   Set TTI Dual lookup redirect Configuration
*
* @param[in] redirectCommand          - redirect command
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectTtiRule0ConfigurationSet
(
    PRV_TGF_TTI_REDIRECT_COMMAND_ENT    redirectCommand
);

/**
* @internal prvTgfTunnelTermParallelLookupRedirectTtiRule1ConfigurationSet function
* @endinternal
*
* @brief   Set TTI Dual lookup redirect Configuration
*
* @param[in] redirectCommand          - redirect command
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectTtiRule1ConfigurationSet
(
    PRV_TGF_TTI_REDIRECT_COMMAND_ENT    redirectCommand
);

/**
* @internal prvTgfTunnelTermParallelLookupRedirectTtiRule2ConfigurationSet function
* @endinternal
*
* @brief   Set TTI Dual lookup redirect Configuration
*
* @param[in] redirectCommand          - redirect command
* @param[in] changeVlan               - whether to change VlanId in TTI action
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectTtiRule2ConfigurationSet
(
    PRV_TGF_TTI_REDIRECT_COMMAND_ENT    redirectCommand,
    GT_BOOL                             changeVlan
);

/**
* @internal prvTgfTunnelTermParallelLookupRedirectTtiRule3ConfigurationSet function
* @endinternal
*
* @brief   Set TTI Dual lookup redirect Configuration
*
* @param[in] command                  - packet command
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupRedirectTtiRule3ConfigurationSet
(
    CPSS_PACKET_CMD_ENT    command
);

/**
* @internal prvTgfTunnelTermParallelLookupEnableLookup function
* @endinternal
*
* @brief   Enable/disable continuing to next TTI lookup
*
* @param[in] lookup                   -  number
* @param[in] nextLookupEnable         - enable/disable TTI lookup
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupEnableLookup
(
    GT_U32  lookup,
    GT_BOOL nextLookupEnable
);

/**
* @internal prvTgfTunnelTermParallelLookupEnable function
* @endinternal
*
* @brief   Set TCAM segment mode for IPv4 key
*
* @param[in] enable                   - parallel lookup is enabled/disabled
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupEnable
(
    GT_BOOL enable
);

/**
* @internal prvTgfTunnelTermParallelLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] isPacketExpectedOnCpu    - is packet mirrored  to CPU
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupTrafficGenerate
(
    GT_BOOL    isPacketExpectedOnCpu
);

/**
* @internal prvTgfTunnelTermParallelLookupTrafficGenerateExpectNoTraffic function
* @endinternal
*
* @brief   Generate traffic - expect no traffic
*
* @param[in] isPacketExpectedOnCpu    - is packet trapped to CPU
*                                       None
*/
GT_VOID prvTgfTunnelTermParallelLookupTrafficGenerateExpectNoTraffic
(
    GT_BOOL    isPacketExpectedOnCpu
);

/**
* @internal prvTgfTunnelTermParallelLookupConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTermParallelLookupConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermParallelLookupCncRestore function
* @endinternal
*
* @brief   Restore CNC Configuration per lookup.
*/
GT_VOID prvTgfTunnelTermParallelLookupCncRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermParallelLookuph */


