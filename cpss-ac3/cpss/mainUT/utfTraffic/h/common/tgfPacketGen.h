/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfTunnelGen.h
*
* DESCRIPTION:
*       Generic API for Packets
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*
*******************************************************************************/
#ifndef __tgfPacketGenh
#define __tgfPacketGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
    #include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
    #include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/tti/cpssExMxPmTtiTypes.h>
    #include <cpss/exMxPm/exMxPmGen/tti/cpssExMxPmTti.h>
    #include <cpss/exMxPm/exMxPmGen/tunnelStart/cpssExMxPmTunnelStart.h>
#endif /* EXMXPM_FAMILY */

#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @internal prvTgfPacketHeaderPartGet function
* @endinternal
*
* @brief   Gets packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] packetInfoPtr            - (pointer to) packet's info
*
* @param[out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - Item not found
*/
GT_STATUS prvTgfPacketHeaderPartGet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    TGF_PACKET_STC      *packetInfoPtr,
    OUT   GT_VOID             **packetHeaderPartPtr
);


/**
* @internal prvTgfPacketNumberOfVlanTagsSet function
* @endinternal
*
* @brief   Sets number of vlan tags in tunnel or passenger header.
*
* @param[in] isPassenger              - whether this is passenger or tunnel
* @param[in] numberOfVlanTags         - number of vlan tags
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Default value for global parameters prvTgfPacketNumberOfVlanTagsPassenger
*       and prvTgfPacketNumberOfVlanTags is 1.
*
*/
GT_STATUS prvTgfPacketNumberOfVlanTagsSet
(
    IN    GT_BOOL    isPassenger,
    IN    GT_U32     numberOfVlanTags
);

/**
* @internal prvTgfPacketNumberOfMplsLabelsSet function
* @endinternal
*
* @brief   Sets number of MPLS labels in header.
*
* @param[in] numberOfMplsLabels       - number of MPLS labels
*                                       None.
*
* @note Default value for global parameter prvTgfPacketNumberOfMplsLabels is 1
*
*/
GT_STATUS prvTgfPacketNumberOfMplsLabelsSet
(
    IN    GT_BOOL  numberOfMplsLabels
);

/**
* @internal prvTgfPacketIsGreSet function
* @endinternal
*
* @brief   Sets whether the packet has GRE part in header.
*
* @param[in] isGre                    - Whether GRE part is included in packet header
*                                       None.
*
* @note Default value for global parameter prvTgfPacketIsGre is GT_TRUE
*
*/
GT_VOID prvTgfPacketIsGreSet
(
    IN    GT_BOOL  isGre
);

/**
* @internal prvTgfPacketIsCrcSet function
* @endinternal
*
* @brief   Sets whether the packet has CRC part in header.
*
* @param[in] isCrc                    - Whether CRC part is included in packet header
*                                       None.
*
* @note Default value for global parameter prvTgfPacketIsCrc is GT_FALSE
*
*/
GT_VOID prvTgfPacketIsCrcSet
(
    IN    GT_BOOL  isCrc
);

/**
* @internal prvTgfPacketIsWildcardSet function
* @endinternal
*
* @brief   Sets whether the packet has wildcard part in header.
*
* @param[in] isWildcard               - Whether wildcard part is included in packet header
*                                       None.
*
* @note Default value for global parameter prvTgfPacketIsWildcard is GT_FALSE
*
*/
GT_VOID prvTgfPacketIsWildcardSet
(
    IN    GT_BOOL  isWildcard
);

/**
* @internal prvTgfPacketTunnelHeaderForceTillPassengersSet function
* @endinternal
*
* @brief   Sets number of MPLS labels in header.
*
* @param[in] numberOfTunnelHeaderAdditionalBytes - number of additional bytes when
*                                      before passenger
*                                       None.
*
* @note Default value for global parameter prvTgfPacketTunnelHeaderForceTillPassenger is 0
*
*/
GT_VOID prvTgfPacketTunnelHeaderForceTillPassengersSet
(
    IN    GT_U32  numberOfTunnelHeaderAdditionalBytes
);

/**
* @internal prvTgfPacketRestoreDefaultParameters function
* @endinternal
*
* @brief   Sets default values for global parametsers.
*/
GT_VOID prvTgfPacketRestoreDefaultParameters
(
    GT_VOID
);

/**
* @internal prvTgfPAcketIpv4OverIpv4PacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Ipv4 over Ipv4 packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] isPassenger              - whether this is passenger or tunnel
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       passenger:
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*       TGF_PACKET_PART_CRC_E
*
*/
GT_STATUS prvTgfPAcketIpv4OverIpv4PacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_BOOL              isPassenger,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
);

/**
* @internal prvTgfPacketEthOverIpv4PacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Eth over Ipv4 packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] isPassenger              - whether this is passenger or tunnel
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_GRE_E
*       TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_PAYLOAD_E
*       TGF_PACKET_PART_CRC_E
*
*/
GT_STATUS prvTgfPacketEthOverIpv4PacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_BOOL              isPassenger,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
);



/**
* @internal prvTgfPacketEthernetPacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Ethernet default packet.
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketEthernetPacketDefaultPacketGet
(
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
);


/**
* @internal prvTgfPacketIpv4PacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Ipv4 default packet.
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketIpv4PacketDefaultPacketGet
(
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
);

/**
* @internal prvTgfPacketGenericIpv4TemplateDefaultPacketGet function
* @endinternal
*
* @brief   Gets generic IPV4 tunneled with UDP template data packet.
*
* @param[in] ip_header_protocol       - ip header protocol
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketGenericIpv4TemplateDefaultPacketGet
(
    IN GT_U32                     ip_header_protocol,
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
);

/**
* @internal prvTgfPacketGenericIpv6TemplateDefaultPacketGet function
* @endinternal
*
* @brief   Gets generic IPV6 tunneled with UDP/GRE, template data packet.
*
* @param[in] ip_header_protocol       - ip header protocol
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketGenericIpv6TemplateDefaultPacketGet
(
    IN GT_U32                     ip_header_protocol,
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
);

/**
* @internal prvTgfPacketGenericIpv6Template8DefaultPacketGet function
* @endinternal
*
* @brief   Gets generic IPV6 tunneled with UDP template data packet.
*
* @param[in] ip_header_protocol       - ip header protocol
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketGenericIpv6Template8DefaultPacketGet
(
    IN GT_U32                     ip_header_protocol,
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
);

/**
* @internal prvTgfPacketIpv4OverIpv4PacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Ipv4 over Ipv4 default packet.
*
* @param[in] isGre                    - whether this is GRE packet.
*
* @param[out] packetPtr                - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketIpv4OverIpv4PacketDefaultPacketGet
(
    IN  GT_BOOL                   isGre,
    OUT TGF_PACKET_PART_STC       *packetPtr
);


/**
* @internal prvTgfPacketEthOverIpv4PacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Eth over Ipv4 default packet.
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketEthOverIpv4PacketDefaultPacketGet
(
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
);


/**
* @internal prvTgfPacketIpv4PacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Ipv4 packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_STATUS prvTgfPacketIpv4PacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
);


/**
* @internal prvTgfPacketEthernetPacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Ethernet packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_STATUS prvTgfPacketEthernetPacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
);

/**
* @internal prvTgfPacketEthernetOverMplsPacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Mpls default packet.
*
* @param[in] passengerEthertype       - ethertype for the passenger of the MPLS tunnel.
*                                      value 0x0800 --> means IPv4 as L2 of the ethernet.
*                                      else --> means regular L2 ethernet (no L3).
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketEthernetOverMplsPacketDefaultPacketGet
(
    IN TGF_ETHER_TYPE               passengerEthertype,
    OUT GT_U32                      *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC         *packetPartArrayPtr[]
);

/**
* @internal prvTgfPacketEthOverMplsPacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Eth over Mpls packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] isPassenger              - whether this is passenger or tunnel
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_MPLS_E
*       TGF_PACKET_PART_MPLS_E
*       TGF_PACKET_PART_MPLS_E
*       TGF_PACKET_PART_MPLS_E (PW LABEL)
*       TGF_PACKET_PART_MPLS_E (FLOW LABEL)
*       TGF_PACKET_PART_WILDCARD_E (CW LABEL)
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_PAYLOAD_E
*       TGF_PACKET_PART_CRC_E
*
*/
GT_STATUS prvTgfPacketEthOverMplsPacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_BOOL              isPassenger,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
);


/**
* @internal prvTgfPacketGenericIpv4Template8DefaultPacketGet function
* @endinternal
*
* @brief   Gets generic IPV4 tunneled with UDP template data packet.
*
* @param[in] ip_header_protocol       - ip header protocol
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketGenericIpv4Template8DefaultPacketGet
(
    IN  GT_U32                     ip_header_protocol,
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
);

/**
* @internal prvTgfPacketGenericIpvTemplateSetGenericProtocol function
* @endinternal
*
* @brief   Set protocol number for generic ipv4/ipv6 tunneled packets
*
* @param[in] protocolNumber           - 8 bits protocol number
* @param[in] isIpv6                   - GT_TRUE if this is generic ipv6 tunnel ,else generic ipv4 tunnel
*
* @retval GT_OK                    - on success
*/

GT_STATUS prvTgfPacketGenericIpvTemplateSetGenericProtocol
(
    IN GT_U32                     protocolNumber,
    IN GT_BOOL                    isIpv6

);

/**
* @internal prvTgfPacketIpv6PacketDefaultPacketGet function
* @endinternal
*
* @brief   Gets Ipv6 default packet.
*
* @param[out] numOfPartsPtr            - (pointer to) number of parts in packet
* @param[out] packetPartArrayPtr[]     - (pointer to) default packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPacketIpv6PacketDefaultPacketGet
(
    OUT GT_U32                    *numOfPartsPtr,
    OUT TGF_PACKET_PART_STC       *packetPartArrayPtr[]
);
/**
* @internal prvTgfPacketIpv6PacketHeaderDataSet function
* @endinternal
*
* @brief   Sets Ipv6 packet header part.
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in] partIndex                - index part in section (start from 0)
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
* @param[in,out] packetHeaderPartPtr      - (pointer to) packet header part
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note Packet definision -
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV6_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_STATUS prvTgfPacketIpv6PacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    IN    GT_U32               partIndex,
    INOUT GT_VOID             *packetHeaderPartPtr
);


#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __tgfPacketGenh */


