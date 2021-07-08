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
* @file prvTgfTunnelIpEtherOverIpv4Gre.h
*
* @brief Packet received as IP-Ethernet-over-IPv4-GRE tunnel
* Packet is TT
* Passenger Ethernet packet MAC DA is Router MAC address
* Triggers IPv4 UC router, Nexthop entry has TS Pointer, and TS Type is IP
* TS entry is IPv4-GRE
* Packet is egressed as IPv4-over-GRE-IPv4
*
* @version   1
********************************************************************************
*/


/**
* @internal prvTgfTunnelIpEtherOverIpv4GreBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreBridgeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreTtiConfigSet function
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
GT_VOID prvTgfTunnelIpEtherOverIpv4GreTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreIpv4TunnelStartConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreIpv4TunnelStartConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreLttRouteConfigurationSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreBuildPacketSet function
* @endinternal
*
* @brief   Build packet
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
*       TGF_PACKET_PART_GRE_E
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreBuildPacketSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:01:02:03:34:02,
*         macSa = 00:04:05:06:07:11,
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelIpEtherOverIpv4GreConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelIpEtherOverIpv4GreConfigurationRestore
(
    GT_U32      prvUtfVrfId
);

