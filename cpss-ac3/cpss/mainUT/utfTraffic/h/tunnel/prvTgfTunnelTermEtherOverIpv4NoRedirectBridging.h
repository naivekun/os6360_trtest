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
* @file prvTgfTunnelTermEtherOverIpv4NoRedirectBridging.h
*
* @brief Tunnel term Ethernet over Ipv4 No redirect + Bridging
*
* @version   2
********************************************************************************
*/


/**
* @internal prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingBridgeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingInilifConfigSet function
* @endinternal
*
* @brief   Set inlif configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingInilifConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingTtiConfigSet function
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
GT_VOID prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingConfigurationRestore
(
    GT_VOID
);


