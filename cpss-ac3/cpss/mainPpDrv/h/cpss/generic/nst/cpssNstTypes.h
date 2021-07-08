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
* @file cpssNstTypes.h
*
* @brief Includes structures definition of Network Shield Technology (NST) module.
*
*
* @version   4
********************************************************************************
*/

#ifndef __cpssNstTypesh
#define __cpssNstTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

/**
* @enum CPSS_NST_AM_PARAM_ENT
 *
 * @brief specifies type of configuration parameter
*/
typedef enum{

    /** @brief To configure the security level of
     *  source MAC address for FDB entry which
     *  is learned in the FDB by automatic
     *  learning
     */
    CPSS_NST_AM_SA_AUTO_LEARNED_E,

    /** @brief To configure the security level of
     *  destination MAC address for FDB
     *  entry which is learned in the FDB by
     *  automatic learning
     */
    CPSS_NST_AM_DA_AUTO_LEARNED_E,

    /** @brief To configure security level for
     *  unknown source MAC address
     */
    CPSS_NST_AM_SA_UNKNOWN_E,

    /** @brief To configure security level for
     *  unknown destination MAC address
     */
    CPSS_NST_AM_DA_UNKNOWN_E

} CPSS_NST_AM_PARAM_ENT;

/**
* @enum CPSS_NST_CHECK_ENT
 *
 * @brief Specifies sanity checks.
*/
typedef enum{

    /** TCP SYN packets with data check */
    CPSS_NST_CHECK_TCP_SYN_DATA_E,

    /** TCP over MC/BC packets check */
    CPSS_NST_CHECK_TCP_OVER_MAC_MC_BC_E,

    /** @brief TCP packets with all flags zero
     *  check
     */
    CPSS_NST_CHECK_TCP_FLAG_ZERO_E,

    /** @brief TCP packets with the TCP FIN,
     *  URG, and PSH flags check
     */
    CPSS_NST_CHECK_TCP_FLAGS_FIN_URG_PSH_E,

    /** @brief TCP packet with the TCP SYN
     *  and FIN flags
     */
    CPSS_NST_CHECK_TCP_FLAGS_SYN_FIN_E,

    /** @brief TCP packet with the TCP SYN
     *  and RST flags check
     */
    CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E,

    /** @brief TCP/UDP packet with a zero
     *  source or destination port
     */
    CPSS_NST_CHECK_TCP_UDP_PORT_ZERO_E,

    /** all TCP checks */
    CPSS_NST_CHECK_TCP_ALL_E,

    /** IPv4 fragmented packet check */
    CPSS_NST_CHECK_FRAG_IPV4_ICMP_E,

    /** ARP MAC SA Mismatch check */
    CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E,

    /** @brief IPv4/IPv6 packets with SIP address equal to DIP
     *  address check
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_NST_CHECK_SIP_IS_DIP_E,

    /** @brief TCP first fragment (or non-fragmented) packets
     *  without full TCP header check
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E,

    /** @brief TCP packets with the TCP FIN flag set
     *  and the TCP ACK not set check
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E,

    /** @brief TCP packets with source TCP/UDP equal to destination
     *  TCP/UDP port check.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E

} CPSS_NST_CHECK_ENT;


/**
* @enum CPSS_NST_INGRESS_FRW_FILTER_ENT
 *
 * @brief ingress forwarding restrictions types.
*/
typedef enum{

    /** to another network port */
    CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E,

    /** to a CPU port */
    CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E,

    /** @brief to an analyzer port
     *  (isn't supported for ExMxPm devices)
     */
    CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E

} CPSS_NST_INGRESS_FRW_FILTER_ENT;

/**
* @enum CPSS_NST_EGRESS_FRW_FILTER_ENT
 *
 * @brief egress forwarding restrictions types.
*/
typedef enum{

    /** from the CPU */
    CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E,

    /** from the bridging engine */
    CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E,

    /** from the routing engine */
    CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E,

    /** @brief from MPLS engine
     *  APPLICABLE DEVICES: Puma2
     */
    CPSS_NST_EGRESS_FRW_FILTER_MPLS_E,

    /** @brief from L2 VPN engine
     *  APPLICABLE DEVICES: Puma2
     */
    CPSS_NST_EGRESS_FRW_FILTER_L2VPN_E

} CPSS_NST_EGRESS_FRW_FILTER_ENT;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssNstTypesh */


