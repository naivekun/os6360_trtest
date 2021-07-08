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
* @file prvTgfTunnelStartEtherOverIpv4GreRedirectToEgress.h
*
* @brief Tunnel start Ethernet over Ipv4 GRE redirect to egress
*
* @version   4
********************************************************************************
*/

#include <trafficEngine/tgfTrafficEngine.h>

/**
* @internal tgfTunnelStartTestAsDsaTagged function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*
* @param[in] egressDsaType            - type of DSA tag
*                                       None
*/
GT_STATUS tgfTunnelStartTestAsDsaTagged
(
    TGF_DSA_TYPE_ENT    egressDsaType
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressBridgeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTunnelConfigSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*
* @param[in] retainCrc                - determines whether the original passenger Ethernet CRC is
*                                      retain or stripped
*                                       None
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTunnelConfigSet
(
    IN GT_BOOL      retainCrc
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressPclConfigSet function
* @endinternal
*
* @brief   Set Pcl Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressPclConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] retainCrc                - determines whether the original passenger Ethernet CRC is
*                                      retain or stripped
*                                       None
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTrafficGenerate
(
    IN GT_BOOL  retainCrc
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressConfigurationRestore
(
    GT_VOID
);


