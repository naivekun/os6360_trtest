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
* @file prvTgfPclUdbOnlyKeys.h
*
* @brief UDB Only Keys specific PCL features testing
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfPclUdbOnlyKeysh
#define __prvTgfPclUdbOnlyKeysh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

/**
* @internal prvTgfPclUdbOnlyKeysPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclUdbOnlyKeysPortVlanFdbSet
(
    GT_VOID
);
/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSet function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pktType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *patternPtr
);

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet function
* @endinternal
*
* @brief   Set UDB Only PCL Key UDB Range selection
*         Replaced UDB positions ommited.
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
* @param[in] udbReplaceBitmap         - bitmap of replace UDBs (12 bits ingress, 2 bits egress)
* @param[in] udbAmount                - udb Amount
* @param[in] udbIndexBase             - udb Index Base
*                                      udbOffsetIncrement  - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           udbReplaceBitmap,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexBase,
    IN GT_32                            udbIndexIncrement
);

/**
* @internal prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet function
* @endinternal
*
* @brief   Set PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] offsetType               - offset Type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
* @param[in] udbOffsetBase            - udb Offset Base
* @param[in] udbOffsetIncrement       - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbOffsetBase,
    IN GT_32                            udbOffsetIncrement
);

/**
* @internal prvTgfPclUdbOnlyKeysPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclUdbOnlyKeysPortVlanFdbReset
(
    GT_VOID
);
/**
* @internal prvTgfPclUdbOnlyKeysVidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
GT_VOID prvTgfPclUdbOnlyKeysVidTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
);

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgReset function
* @endinternal
*
* @brief   Restore Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleSize                 - Rule Size
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_PCL_RULE_SIZE_ENT          ruleSize
);
/**
* @internal prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset function
* @endinternal
*
* @brief   Reset PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount
);
/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset function
* @endinternal
*
* @brief   Reset UDB Only PCL Key UDB selection
*
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat
);
/**
* @internal prvTgfPclUdbOnlyKeysIngressMetadataTest function
* @endinternal
*
* @brief   Test on Ingress PCL metadata
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressMetadataTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgressMetadataTest function
* @endinternal
*
* @brief   Test on Egress PCL metadata
*
* @note Metadata[207:196] - Original VID
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressMetadataTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngressTunnelL2Test function
* @endinternal
*
* @brief   Test on Ingress PCL Tunnel L2 offset type
*
* @note Tunnel Header MAC_SA
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressTunnelL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngressTunnelL3Test function
* @endinternal
*
* @brief   Test on Ingress PCL Tunnel L3 offset type
*
* @note Tunnel Header DIP[15:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressTunnelL3Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgressTunnelL2Test function
* @endinternal
*
* @brief   Test on Egress PCL Tunnel L2 offset type
*
* @note Tunnel Header MAC_DA[15:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressTunnelL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL Tunnel L3 Minus 2 offset type
*
* @note Tunnel Header SIP[31:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB10 L4 offset type on IPV4 TCP packet
*
* @note 10 first bytes from TCP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB10 L4 offset type on IPV4 UDP packet
*
* @note 10 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 20 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB30 MplsMinus2 offset type on Ethernet over MPLS packet
*
* @note 20 first bytes from MPLS Ethernet Type beginning.
*       MPLS offset supported by HW only up to 20 bytes
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet
*
* @note 40 first bytes from L2 header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 L3Minus2 offset type on IPV6 Other packet
*
* @note 50 first bytes from IPV6 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60 L3Minus2 offset type on IPV4 Other packet
*
* @note 50 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60_NO_FIXED L3Minus2 offset type on IPV4 Other
*         packet
*
* @note 60 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB10 L4 offset type on IPV4 TCP packet
*
* @note 10 first bytes from TCP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 20 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB30 MplsMinus2 offset type on ETH over MPLS packet.
*
* @note 20 first bytes from MPLS Ethernet Type beginning.
*       MPLS offset supported by HW only up to 20 bytes
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB40 L2 offset type on ETH Other packet.
*
* @note 40 first bytes from L2 Header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB50 L3Minus2 offset type on IPV6 Other packet.
*
* @note 50 first bytes from IPV6 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB60 L3Minus2 offset type on IPV4 Other packet.
*
* @note 50 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB30 L3Minus2 offset type on Ethernet over MPLS packet
*
* @note 30 first bytes from MPLS Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB30 L3Minus2 offset type on ETH over MPLS packet.
*
* @note 30 first bytes from MPLS Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest function
* @endinternal
*
* @brief   Test Egress PCL on LLC Non SNAP packet.
*
* @note Used 10 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclUdbOnlyKeysh */


