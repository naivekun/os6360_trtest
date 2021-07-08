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
* @file prvTgfTunnelTermEtherOverIpv4GreRedirectToEgress.h
*
* @brief Tunnel term Ethernet over Ipv4 GRE redirect to egress
*
* @version   7
********************************************************************************
*/


/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*
* @param[in] isTaggedVlansUsed        - to mark vlans tagged or not
*                                       None
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBridgeConfigSet
(
    IN GT_BOOL isTaggedVlansUsed
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressInilifConfigurationSet function
* @endinternal
*
* @brief   Set inlif configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressInilifConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
* @param[in] ttHeaderLength           - Tunnel header length in units of Bytes.
*                                      Qranularity is in 2 Bytes.
* @param[in] passengerParsingOfTransitNonMplsTransitTunnelEnable -
*                                      Whether to parse the passenger of transit tunnel
*                                      packets other than MPLS.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet
(
    GT_U32      ttHeaderLength,
    GT_BOOL     passengerParsingOfTransitNonMplsTransitTunnelEnable
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] testNum                  - test number
*                                       None
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate
(
    GT_U32 testNum
);


/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressMacSecConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
* @param[in] includeMacSecConfig      - whether to include MACSec configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressMacSecConfigSet
(
    GT_BOOL includeMacSecConfig
);

/**
* @internal prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet function
* @endinternal
*
* @brief   Build packet
*
* @param[in] testNum                  - special parameters for the test
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E (GRE)
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet
(
    GT_U32  testNum
);


