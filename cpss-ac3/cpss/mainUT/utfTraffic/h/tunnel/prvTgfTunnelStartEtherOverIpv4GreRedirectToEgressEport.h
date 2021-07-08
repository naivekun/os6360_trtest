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
* @file prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEport.h
*
* @brief Tunnel start Ethernet over Ipv4 GRE redirect to egress ePort
*
* @version   2
********************************************************************************
*/


/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*
* @param[in] useDscpRemapOnTs         - do we do egress port remap of DSCP-to-DSCP for the
*                                      IPv4 header of the TS
*                                       None
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportBridgeConfigSet
(
    IN GT_BOOL useDscpRemapOnTs
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTunnelConfigSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTunnelConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportPclConfigSet function
* @endinternal
*
* @brief   Set Pcl Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportPclConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportConfigurationRestore
(
    GT_VOID
);


