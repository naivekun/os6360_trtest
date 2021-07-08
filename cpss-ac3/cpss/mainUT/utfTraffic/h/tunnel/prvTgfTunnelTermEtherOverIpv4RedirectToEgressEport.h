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
* @file prvTgfTunnelTermEtherOverIpv4RedirectToEgressEport.h
*
* @brief Tunnel term Ethernet over Ipv4 redirect to egress ePort
*
* @version   1
********************************************************************************
*/


/**
* @internal prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportBridgeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportConfigurationRestore
(
    GT_VOID
);


