/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfTunnelStartGenericIpv4Template.h
*
* DESCRIPTION:
*       Tunnel start Ipv4 UDP Template
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/


/**
* @internal prvTgfTunnelStartGenericIpv4TemplateBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateBridgeConfigSet
(
     GT_VOID
);


/**
* @internal prvTgfTunnelStartGenericIpv4TemplateRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateRouteConfigurationSet
(
    GT_VOID
);



/**
* @internal prvTgfTunnelStartGenericIpv4TemplateTunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateTunnelConfigurationSet
(
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_ENT protocolType,
    GT_U32                                      protocolNum
);

/**
* @internal prvTgfTunnelStartGenericIpv4TemplateBuildPacket function
* @endinternal
*
* @brief   Build packet
*
* @param[in] ip_header_protocol       - ip header protocol (IP,GRE,UDP)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateBuildPacket
(
    GT_U32 ip_header_protocol
);


/**
* @internal prvTgfTunnelStartGenericIpv4TemplateCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*/
void prvTgfTunnelStartGenericIpv4TemplateCheckCaptureEgressTrafficOnPort
(
 GT_VOID
);

/**
* @internal prvTgfTunnelStartGenericIpv4TemplateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateTrafficGenerate
(
    GT_VOID
);


/**
* @internal prvTgfTunnelStartGenericIpv4TemplateConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartGenericIpv4TemplateTest function
* @endinternal
*
* @brief   Run generic ipv4 tunneling test
*
* @param[in] protocolType             - tunnel start ip header protocol type
* @param[in] protocolNum              - tunnel start ip header protocol number (relevant only in case that protocolType is  PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E)
*                                       None
*/

GT_VOID prvTgfTunnelStartGenericIpv4TemplateTest
(
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_ENT protocolType,
    GT_U32                                      protocolNum
);



