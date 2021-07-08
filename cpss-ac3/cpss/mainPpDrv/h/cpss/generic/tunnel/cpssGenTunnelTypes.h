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
* @file cpssGenTunnelTypes.h
*
* @brief Generic definitions for tunnel.
*
* @version   10
********************************************************************************
*/

#ifndef __cpssGenTunnelTypesh
#define __cpssGenTunnelTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>


/**
* @enum CPSS_TUNNEL_GRE_ETHER_TYPE_ENT
 *
 * @brief GRE ether type.
*/
typedef enum{

    /** GRE type 0 - control ether type. */
    CPSS_TUNNEL_GRE0_ETHER_TYPE_E     = 0,

    /** GRE type 1 - data ether type. */
    CPSS_TUNNEL_GRE1_ETHER_TYPE_E     = 1

} CPSS_TUNNEL_GRE_ETHER_TYPE_ENT;


/**
* @enum CPSS_TUNNEL_TYPE_ENT
 *
 * @brief Enumeration of tunnel types.
*/
typedef enum{

    /** @brief X tunneled over ipv4
     *  passenger protocol is not relevant
     *  tunnel protocol is ipv4.
     *  Applicable devices: xCat3; Lion2
     */
    CPSS_TUNNEL_X_OVER_IPV4_E     = 6,

    /** @brief X tunneled over GRE ipv4
     *  passenger protocol is not relevant
     *  tunnel protocol is GRE ipv4.
     *  Applicable devices: xCat3; Lion2
     */
    CPSS_TUNNEL_X_OVER_GRE_IPV4_E = 7,

    /** @brief X tunneled over MPLS
     *  passenger protocol is not relevant
     *  tunnel protocol is MPLS
     */
    CPSS_TUNNEL_X_OVER_MPLS_E     = 8,

    /** @brief MAC in MAC
     *  based on IEEE 802.1ah
     */
    CPSS_TUNNEL_MAC_IN_MAC_E      = 10,

    /** @brief TRILL
     *  based on transparent interconnections
     *  of lots of links control protocol. Applicable only for eArch devices
     *  Applicable devices: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
     */
    CPSS_TUNNEL_TRILL_E           = 11,

    /** @brief Generic IPv4 Tunnel-start, provide flexible
     *  support for tunneling protocols such as:
     *  IP4/6-over-IPv4 tunnels,
     *  IP4/6-over-IPv4-GRE tunnels,
     *  where the GRE header may include configurable extensions,
     *  Ethernet-over-IPv4-GRE tunnels, where the GRE header
     *  may include configurable extensions.
     *  Ethernet-over-IPv4-UDP tunnels, where the UDP header may
     *  be followed by a configurable shim extension.
     *  Applicable devices: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2
     */
    CPSS_TUNNEL_GENERIC_IPV4_E    = 13,

    /** @brief Generic IPv6 Tunnel-start, provide flexible
     *  support for tunneling protocols such as:
     *  IP4/6-over-IPv6 tunnels,
     *  IP4/6-over-IPv6-GRE tunnels,
     *  where the GRE header may include configurable extensions,
     *  Ethernet-over-IPv6-GRE tunnels, where the GRE header
     *  may include configurable extensions.
     *  Ethernet-over-IPv6-UDP tunnels, where the UDP header may
     *  be followed by a configurable shim extension.
     *  Applicable devices: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2
     */
    CPSS_TUNNEL_GENERIC_IPV6_E    = 14,

    /** @brief Generic
     * Applicable devices: Falcon
     */
    CPSS_TUNNEL_GENERIC_E         = 15

} CPSS_TUNNEL_TYPE_ENT;

/**
* @enum CPSS_TUNNEL_PASSENGER_PACKET_ENT
 *
 * @brief Enumeration of tunnel passenger packet type.
*/
typedef enum{

    /** IPv4 passenger packet */
    CPSS_TUNNEL_PASSENGER_PACKET_IPV4_E            = 0,

    /** IPv6 passenger packet */
    CPSS_TUNNEL_PASSENGER_PACKET_IPV6_E            = 1,

    /** ethernet without CRC passenger packet */
    CPSS_TUNNEL_PASSENGER_PACKET_ETHERNET_NO_CRC_E = 2,

    /** ethernet with CRC passenger packet */
    CPSS_TUNNEL_PASSENGER_PACKET_ETHERNET_CRC_E    = 3

} CPSS_TUNNEL_PASSENGER_PACKET_ENT;

/**
* @enum CPSS_TUNNEL_ETHERTYPE_TYPE_ENT
 *
 * @brief Ethertype type.
*/
typedef enum{

    /** IPv4 GRE 0 Ethernet type */
    CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E = 0,

    /** IPv4 GRE 1 Ethernet type */
    CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E = 1,

    /** MPLS Ethernet type for Unicast packets */
    CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E = 2,

    /** MPLS Ethernet type for Multicast packets */
    CPSS_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E = 3,

    /** Mac in Mac Ethernet type */
    CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E = 4,

    /** @brief TRILL Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_TRILL_E = 5,

    /** @brief IPv6 GRE 0 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE0_E = 6,

    /** @brief IPv6 GRE 1 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE1_E = 7,

    /** @brief User defined 0 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E = 8,

    /** @brief User defined 1 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E = 9,

    /** @brief User defined 2 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E = 10,

    /** @brief User defined 3 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E = 11,

    /** @brief User defined 4 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E = 12,

    /** @brief User defined 5 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E = 13,

    /** @brief User defined 6 Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3;
     *  Caelum; Aldrin; AC3X; Aldrin2; Falcon)
     */
    CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E = 14

} CPSS_TUNNEL_ETHERTYPE_TYPE_ENT;

/**
* @enum CPSS_TUNNEL_MULTICAST_TYPE_ENT
 *
 * @brief Enumeration of multicast tunnel type.
*/
typedef enum{

    /** IPv4 multicast tunnel */
    CPSS_TUNNEL_MULTICAST_IPV4_E,

    /** IPv6 multicast tunnel */
    CPSS_TUNNEL_MULTICAST_IPV6_E,

    /** MPLS multicast tunnel */
    CPSS_TUNNEL_MULTICAST_MPLS_E,

    /** TRILL multicast tunnel */
    CPSS_TUNNEL_MULTICAST_TRILL_E,

    /** PBB multicast tunnel */
    CPSS_TUNNEL_MULTICAST_PBB_E

} CPSS_TUNNEL_MULTICAST_TYPE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenTunnelTypesh */


