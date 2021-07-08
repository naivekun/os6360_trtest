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
* @file prvCpssDxChNetIf.h
* @version   24
********************************************************************************
*/

#ifndef __prvCpssDxChNetIfh
#define __prvCpssDxChNetIfh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfDsaTag.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfMii.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>

/* additional user packet header, this is used to indicate if the packet  */
/* was received tagged or not.                                            */
#define PRV_CPSS_DXCH_ETHPRT_TAGGED_PCKT_FLAG_LEN_CNS    4
#define PRV_CPSS_DXCH_ETHPRT_TAGGED_PCKT_CNS             1
#define PRV_CPSS_DXCH_ETHPRT_UNTAGGED_PCKT_CNS           2
#define PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS            12

/* num of bytes save for each rx packet --
   before the start of the buffer (before the offset of the application)

   used similar to PRV_CPSS_DXCH_ETHPRT_TAGGED_PCKT_FLAG_LEN_CNS
*/
#define PRV_CPSS_DXCH_NET_INTERNAL_RX_PACKET_INFO_SIZE_CNS   4

/* max number of destination devices for CPU destined packets */
#define PRV_CPSS_DXCH_NET_DSTDEVS_NUM_CNS 8 /* first not in use, so really it's 7 */

/* max number of statistical rate limit values for CPU destined packets */
#define PRV_CPSS_DXCH_NET_LIMITS_NUM_CNS 32

/* max number of CPU destined packets opcodes */
#define PRV_CPSS_DXCH_NET_CPUCODES_NUM_CNS 256

/* the offset with in every rx packet that the internal info about the packet
   can be save in */
#define PRV_CPSS_DXCH_RX_OFFSET_TO_INTERNAL_INFO_CNS_MAC(hdrOffset)         \
    (hdrOffset + PRV_CPSS_DXCH_NET_INTERNAL_RX_PACKET_INFO_SIZE_CNS)

/* INOUT GT_U8* buffPtr
   buffPtr -- the pointer to the buffer as it is going to be sent to the
              application

   offset set -- set offset of actual buffer in the reserved place.

   offset get -- get offset of actual buffer from the reserved place.

   --- comment : use those macros only when this is FIRST buffer . ---
*/
#define PRV_CPSS_DXCH_RX_ORIG_PCKT_OFFSET_SET_MAC(bufPtr, hdrOffset, offset)  \
                (*((bufPtr)-(PRV_CPSS_DXCH_RX_OFFSET_TO_INTERNAL_INFO_CNS_MAC(hdrOffset)))) = (GT_U8)offset

#define PRV_CPSS_DXCH_RX_ORIG_PCKT_OFFSET_GET_MAC(bufPtr, hdrOffset)   \
                (*((bufPtr)-(PRV_CPSS_DXCH_RX_OFFSET_TO_INTERNAL_INFO_CNS_MAC(hdrOffset))))

/* 2 byte offset for ip header to be aligned */
#define PRV_CPSS_DXCH_IP_ALIGN_OFFSET_CNS  2

/* check that the networkif of the device was initialized
    return GT_NOT_INITIALIZED if not initialized
*/
#define PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum)                   \
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.initDone == GT_FALSE)            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG)

/* Macro to check that Tx queue number in not out of range */
#define PRV_CPSS_DXCH_DEV_TX_SDMA_QUEUE_NUM_CHECK_MAC(_txQueue) \
    if (_txQueue >= NUM_OF_SDMA_QUEUES)                                      \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

/* Macro to check that Tx SDMA is configured to work as Generator */
#define PRV_CPSS_DXCH_DEV_TX_SDMA_GENERATOR_CHECK_MAC(_devNum,_txQueue)     \
    if(CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E !=                        \
            PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(_devNum)->                     \
               multiNetIfCfg.                                               \
                    txSdmaQueuesConfig[(_txQueue)>>3][(_txQueue) & 7].queueMode)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

/**
* @enum PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT
 *
 * @brief Defines the different CPU codes that indicate the reason for
 * sending a packet to the CPU.
*/
typedef enum{

    /** regular forwarding packet */
    PRV_CPSS_DXCH_NET_DSA_TAG_REGULAR_FORWARD_E                         = 0,

    /** reserved */
    PRV_CPSS_DXCH_NET_DSA_TAG_CONTROL_TO_CPU_E                          = 1,

    /** Control BPDU packet. */
    PRV_CPSS_DXCH_NET_DSA_TAG_BPDU_E                                    = 2,

    /** @brief Source & Destination MAC
     *  trapped packet.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_MAC_ADDR_TRAP_E                           = 3,

    /** @brief MAC Range Trapped packet.
     *  INTERVENTION CODES:
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_MAC_RANGE_TRAP_E                          = 4,

    /** Intervention ARP */
    PRV_CPSS_DXCH_NET_DSA_TAG_ARP_BROADCAST_E                           = 5,

    /** @brief Intervention IGMP
     *  can also be trapped according to VLAN)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IGMP_PACKET_E                             = 6,

    /** @brief Intervention Source
     *  and Destination addresses
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_INTERVENTION_MAC_ADDR_E                   = 7,

    /** @brief Unknown MAC SA Command
     *  MIRRORING CODES:
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_UNK_SRC_MAC_ADDR_E                        = 8,

    /** @brief Packet Mirrored to
     *  CPU because MAC range
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_MIRROR_MAC_RANGE_TO_CPU_E                 = 9,

    /** @brief Packet Mirrored to
     *  CPU because port lock
     *  MISC CODES:
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_MIRROR_LOCK_TO_CPU_E                      = 10,

    /** Unknown unicast packets. */
    PRV_CPSS_DXCH_NET_DSA_TAG_UNKNOWN_UNICAST_E                         = 11,

    /** @brief Unregistered multicast packet.
     *  CHEETAH used additional Codes:
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_UNREGISTERED_MULTICAST_E                  = 12,

    /** @brief Packet with MAC
     *  DA in the IEEE Reserved Multicast range trapped or
     *  mirrored to the host CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IEEE_RSRVD_MULTICAST_ADDR_E               = 13,

    /** @brief IPv6 ICMP packet
     *  trapped or mirrored to the host CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_ICMP_PACKET_E                        = 14,

    /** @brief Packet with extended
     *  tag trapped or mirrored to the host CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_DSA_EXTENDED_TAG_TO_CPU_E                 = 15,

    /** @brief IPv4/ Pv6
     *  Link Local MC DIP packet trapped or mirrored to the host CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E     = 16,

    /** @brief IPv4 RIPv1
     *  packet mirrored to the host CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_MIRROR_IPV4_RIPV1_TO_CPU_E                = 17,

    /** @brief IPv6 neighbor
     *  solicitation packet trapped or mirrored to the host CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_NEIGHBOR_SOLICITATION_E              = 18,

    /** @brief IPv4 Broadcast packet assigned to a VLAN with
     *  IPv4 Broadcast Command set to Trap or Mirror.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_BROADCAST_PACKET_E                   = 19,

    /** @brief Non IPv4 Broadcast packet assigned to a VLAN
     *  with non IPv4 Broadcast Command set to Trap
     *  or Mirror.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_NON_IPV4_BROADCAST_PACKET_E               = 20,

    /** @brief A Multicast packet with a MAC DA in the CISCO
     *  AUI range trapped or mirrored to the host CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_CISCO_MCAST_MAC_RANGE_E                   = 21,

    /** @brief Unregistered Multicast packet assigned
     *  to a VLAN with Unregistered Multicast Command set to Trap or Mirror.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_BRG_UNREGD_MCAST_E                        = 22,

    /** @brief IPv4 Unregistered Multicast packet assigned
     *  to a VLAN with IPv4 Unregistered Multicast
     *  Command set to Trap or Mirror.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_BRG_IPV4_UNREG_MCAST_E                    = 23,

    /** @brief IPv6 Unregistered Multicast packet assigned
     *  to a VLAN with IPv6 Unregistered Multicast
     *  Command set to Trap or Mirror.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_BRG_IPV6_UNREG_MCAST_E                    = 24,

    /** @brief unknown unicast that where trapped/mirrored
     *  to cpu due to vlan command
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_UNKNOWN_UNICAST_EXT_E                     = 25,

    /** CPU Code=1 for Ieee Reserved Multicast Addrs. */
    PRV_CPSS_DXCH_NET_DSA_TAG_IEEE_RSRVD_MULTICAST_ADDR_1_E             = 26,

    /** CPU Code=2 for Ieee Reserved Multicast Addrs. */
    PRV_CPSS_DXCH_NET_DSA_TAG_IEEE_RSRVD_MULTICAST_ADDR_2_E             = 27,

    /** CPU Code=3 for Ieee Reserved Multicast Addrs. */
    PRV_CPSS_DXCH_NET_DSA_TAG_IEEE_RSRVD_MULTICAST_ADDR_3_E             = 28,

    /** Ipv4/Ipv6 link local multicast CPU Code index=1 */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E   = 29,

    /** Ipv4/Ipv6 link local multicast CPU Code index=2 */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E   = 30,

    /** Ipv4/Ipv6 link local multicast CPU Code index=3 */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E   = 31,

    /** @brief The UDP BC packet matches one of entries
     *  in UDP relay table and its CPU code index is 0
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_UDP_BC_MIRROR_TRAP0_E                     = 32,

    /** @brief The UDP BC packet matches one of entries
     *  in UDP relay table and its CPU code index is 1
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_UDP_BC_MIRROR_TRAP1_E                     = 33,

    /** @brief The UDP BC packet matches one of entries
     *  in UDP relay table and its CPU code index is 2
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_UDP_BC_MIRROR_TRAP2_E                     = 34,

    /** @brief The UDP BC packet matches one of entries
     *  in UDP relay table and its CPU code index is 3
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_UDP_BC_MIRROR_TRAP3_E                     = 35,

    /** @brief In Secure Automatic learning,an uknown source MAC
     *  is trapped to CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_SEC_AUTO_LEARN_UNK_SRC_TRAP_E             = 36,

    /** @brief The drop code for a packet that
     *  was dropped by the RxDMA and forwarded to the ingress pipe.(RXDMA)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_RXDMA_DROP_E                              = 37,

    /** VLAN Not Valid Drop (a security breach) */
    PRV_CPSS_DXCH_NET_DSA_TAG_VLAN_NOT_VALID_E                          = 38,

    /** Port Not Member in VLAN Drop (a security breach) */
    PRV_CPSS_DXCH_NET_DSA_TAG_PORT_NOT_VLAN_MEM_E                       = 39,

    /** VLAN Range Drop (a security breach) */
    PRV_CPSS_DXCH_NET_DSA_TAG_VLAN_RANGE_E                              = 40,

    /** Moved Static Address Drop (a security breach) */
    PRV_CPSS_DXCH_NET_DSA_TAG_STATIC_ADDR_MOVED_E                       = 41,

    /** Mac Spoof Protection Event */
    PRV_CPSS_DXCH_NET_DSA_TAG_MAC_SPOOF_E                               = 42,

    /** ARP SA mismatch Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_ARP_SA_MISMATCH_E                         = 43,

    /** SYN with Data Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_SYN_WITH_DATA_E                           = 44,

    /** TCP over Multicast or Broadcast Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_TCP_OVER_MC_BC_E                          = 45,

    /** Fragmented ICMP Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_FRAGMENT_ICMP_E                           = 46,

    /** TCP Flags Zero Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_TCP_FLAGS_ZERO_E                          = 47,

    /** TCP Flags FIN, URG and PSH are all set Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_TCP_FLAGS_FIN_URG_PSH_E                   = 48,

    /** TCP Flags SYN and FIN are set Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_TCP_FLAGS_SYN_FIN_E                       = 49,

    /** TCP Flags SYN and RST are set Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_TCP_FLAGS_SYN_RST_E                       = 50,

    /** TCP/UDP Source or Destination Port is Zero Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_TCP_UDP_SRC_DEST_ZERO_E                   = 51,

    /** Bridge Access Matrix Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_BRIDGE_ACCESS_MATRIX_E                    = 52,

    /** Acceptable Frame Type Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_ACCEPT_FRAME_TYPE_E                       = 53,

    /** VLAN MRU Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_VLAN_MRU_E                                = 54,

    /** Rate Limiting Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_RATE_LIMITING_E                           = 55,

    /** Local Port Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_LOCAL_PORT_E                              = 56,

    /** IP Multicast In Iana Range Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_IP_MC_E                                   = 57,

    /** IP Multicast Not In Iana Range Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_NON_IP_MC_E                               = 58,

    /** DSA Tag source device equals Local device Drop */
    PRV_CPSS_DXCH_NET_DSA_TAG_DSA_TAG_LOCAL_DEV_E                       = 59,

    /** Invalid SA Drop (a security breach) */
    PRV_CPSS_DXCH_NET_DSA_TAG_INVALID_SA_E                              = 60,

    /** @brief Indicates that no
     *  engine has assigned a target ePort to the current packet,
     *  and thus the packet is dropped.(EQ)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_TARGET_NULL_PORT_E                        = 62,

    /** @brief Used when the packet is trapped or dropped due to an MTU check based on the target ePort in the E2PHY.(EQ)
     *  Or when the packet is dropped due to a protection switching filter.(EQ)
     *  Or when a packet is filtered by the ingress forwarding restrictions feature. (EQ)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_TARGET_EPORT_MTU_EXCEEDED_OR_RX_PROTECTION_SWITCHING_OR_INGRESS_FORWARDING_RESTRICTIONS_E = 63,

    /** @brief packet forwarded to the host CPU by the
     *  router ("IP to me")
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_ROUTED_PACKET_FORWARD_E                   = 64,

    /** @brief Packet forwarded to the host CPU by one of
     *  the following engines in the device:
     *  1. Redirected by the PCL engine to the CPU
     *  port number.
     *  2. MAC DA is associated with the CPU port
     *  number.
     *  3. Private VLAN edge target set to CPU port
     *  number.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_BRIDGED_PACKET_FRW_E                      = 65,

    /** @brief Ingress mirrored packets to the analyzer port,
     *  when the ingress analyzer port number is set
     *  to the CPU port number.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_INGRESS_MIRRORED_TO_ANALYZER_E            = 66,

    /** @brief Egress mirrored packets to the analyzer port,
     *  when the egress analyzer port number is set
     *  to the CPU port number.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_EGRESS_MIRRORED_TO_ANALYZER_E             = 67,

    /** @brief A packet sent to the local host CPU as a mail
     *  from the neighboring CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_MAIL_FROM_NEIGHBOR_CPU_E                  = 68,

    /** @brief A packet sent to the local host CPU, from one
     *  of the other host CPUs in the system.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_TO_CPU_E                              = 69,

    /** @brief Packet mirrored to the host CPU by the egress
     *  STC mechanism.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_EGRESS_SAMPLED_E                          = 70,

    /** @brief Packet mirrored to the host CPU by the ingress
     *  STC mechanism.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_INGRESS_SAMPLED_E                         = 71,

    /** @brief Packet trapped/mirrored to the host CPU by the
     *  Policy Engine due to the following:
     *  User-defined bytes in the key could not be
     *  parsed due to packet header length or its
     *  format.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_INVALID_PCL_KEY_TRAP_E                    = 74,

    /** @brief Tunnel-terminated packet is trapped to the CPU
     *  due to an IPV4 tunnel header error.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_TT_HEADER_ERROR_E                    = 75,

    /** @brief Tunnel-terminated packet is trapped to the CPU
     *  due to the IPV4 tunnel header, which is
     *  fragmented or has options.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_TT_OPTION_FRAG_ERROR_E               = 76,

    /** @brief Tunnel-terminated packet is trapped to the CPU
     *  due to an unsupported GRE tunnel header format.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_TT_UNSUP_GRE_ERROR_E                 = 77,

    /** @brief Used in Hop-By-Hop mirroring mode,
     *  when a packet is duplicated to a target analyzer, and the
     *  global Ingress Analyzer Index or Egress Analyzer Index disabled.
     *  In this case the packet is hard dropped with this drop code.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_BAD_ANALYZER_INDEX_DROP_ERROR_E           = 78,

    /** @brief Tunnel-terminated MPLS packet is trapped to the CPU
     *  due to an illegal MPLS header or MPLS command in action entry.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_TT_MPLS_HEADER_CHECK_E                    = 79,

    /** @brief Tunnel-terminated MPLS packet is trapped to the CPU
     *  due to an assigned TTL of ZERO.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_TT_MPLS_TTL_EXCEED_E                      = 80,

    /** @brief Oversize drop -
     *  used only for drop command. (TTI)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_UNTAGGED_OVERSIZE_DROP_CODE_E             = 81,

    /** @brief Used by the TTI engine
     *  to indicate an error with the GRE tunnel header on tunneltermination.(TTI)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_TT_UNSUP_GRE_ERROR_E                 = 82,

    /** An OAM-PDU packet is trappedto the CPU */
    PRV_CPSS_DXCH_NET_DSA_TAG_OAM_PDU_TRAP_E                            = 83,

    /** CCFC packet dropped. */
    PRV_CPSS_DXCH_NET_DSA_TAG_CCFC_PKT_DROP_E                           = 86,

    /** @brief PTP packet received by TTI, and the header is too short,
     *  i.e., it does not contain some of the necessary fields for PTP parsing.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_PTP_HEADER_ERROR_E                        = 87,

    /** @brief Used for bad IEEE 802.3x Flow
     *  Control packets.(TTI)
     *  PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UDP_E
     *  PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_BAD_DIP_SIP_E
     *  PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_MC_BAD_SIP_DIP_E
     *  PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_BAD_DIP_SIP_E
     *  PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_MC_BAD_SIP_DIP_E
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_BAD_FC_PKT_DROP_E                         = 88,

    /** @brief IPv4 packet, triggered for routing Trapped/Mirrored
     *  to the CPU due to TTL exceeded.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_TTL_EXCEEDED_E                    = 133,

    /** @brief IPv4/6 packet, triggered for routing Trapped/Mirrored
     *  to the CPU due packet’s length longer than Route entry <MTU Index>.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_MTU_EXCEEDED_E                     = 134,

    /** @brief IPv6 packet, triggered for routing Trapped/Mirrored
     *  to the CPU due to Hop Limit exceeded.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_HOP_LIMIT_EXCEEDED_E              = 135,

    /** @brief IPv4/6 packet, triggered for routing, is Trapped/Mirrored
     *  due to an illegal IPv4/6 destination or source address.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_ILLEGAL_ADDR_ERROR_E               = 136,

    /** @brief IPv4 packet, triggered for routing Trapped/Mirrored
     *  to the CPU due to IPv4 header Error
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_HEADER_ERROR_E                    = 137,

    /** @brief IPv4/6 packet, triggered for routing, is Trapped/Mirrored
     *  due to a mismatch between the destination IP address
     *  and the destination MAC address.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_DIP_DA_MISMATCH_E                  = 138,

    /** @brief IPv6 packet, triggered for routing Trapped/Mirrored
     *  to the CPU due to IPv6 header Error.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_HEADER_ERROR_E                    = 139,

    /** @brief IPv4/6 Unicast packet, triggered for routing,
     *  is Trapped/Mirrored due to a mismatch between
     *  the source IP address and its configured ARP MAC address.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_UC_SIP_SA_MISMATCH_E               = 140,

    /** @brief IPv4 packet, triggered for routing Trapped/Mirrored
     *  to the CPU due to IPv4 header Options.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_OPTIONS_E                         = 141,

    /** @brief IPv6 packet, triggered for routing, Trapped/Mirrored
     *  due to one of the following non hopbyhop header
     *  extension options: Routing, Fragmentation,
     *  Encapsulating Security Payload, Authentication, Destination.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_NON_HBH_OPTION_E                     = 142,

    /** @brief IPv6 packet, triggered for routing Trapped/Mirrored
     *  to the CPU due to IPv6 header HopByHop Options.
     *  PRV_CPSS_DXCH_NET_DSA_TAG_IPv4_6_SIP_FILTERING_E - Indicates that the
     *  source IP address was filtered by the SIP filtering mechanism.(Router)
     *  PRV_CPSS_DXCH_NET_DSA_TAG_IPv4_6_SIP_IS_ZERO_E  - Indicates that the
     *  source IP address is 0. (Router)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_OPTIONS_E                         = 143,

    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_SIP_FILTERING_E                    = 144,

    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_SIP_IS_ZERO_E                      = 145,

    /** @brief Indicates that the
     *  packet was dropped/mirrored/trapped. (Router)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_ACCESS_MATRIX_E                           = 146,

    /** @brief Indicates an invalid field in the FCoE header Or FCoE DIP was not
     *  found. (Router)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_FCOE_DIP_LOOKUP_NOT_FOUND_OR_FCOE_EXCEPTION_E = 147,

    /** @brief Indicates that the
     *  FCoE SIP was not found. (Router)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_FCOE_SIP_NOT_FOUND_E                      = 148,

    /** @brief IPv6 packet, triggered for routing, Trapped/Mirrored
     *  to CPU due to IPv6 Scope checking.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_SCOPE_E                              = 159,

    /** @brief IPv4 Unicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 0.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_ROUTE0_E                          = 160,

    /** @brief IPv4 Unicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 1.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_ROUTE1_E                          = 161,

    /** @brief IPv4 Unicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 2.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_ROUTE2_E                          = 162,

    /** @brief IPv4 Unicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 3.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_ROUTE3_E                          = 163,

    /** @brief IPv4 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 0.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_MC_ROUTE0_E                          = 164,

    /** @brief IPv4 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 1
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_MC_ROUTE1_E                          = 165,

    /** @brief IPv4 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 2.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_MC_ROUTE2_E                          = 166,

    /** @brief IPv4 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 3.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_MC_ROUTE3_E                          = 167,

    /** @brief IPv6 Unicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 0.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_ROUTE0_E                          = 168,

    /** @brief IPv6 Unicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 1.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_ROUTE1_E                          = 169,

    /** @brief IPv6 Unicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 2.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_ROUTE2_E                          = 170,

    /** @brief IPv6 Unicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 3.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_ROUTE3_E                          = 171,

    /** @brief IPv6 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 0.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_MC_ROUTE0_E                          = 172,

    /** @brief IPv6 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 1.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_MC_ROUTE1_E                          = 173,

    /** @brief IPv6 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route entry with <CPUCodeIndex> = 2.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_MC_ROUTE2_E                          = 174,

    /** @brief IPv6 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU from Route Entry with <CPUCodeIndex> = 3.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_MC_ROUTE3_E                          = 175,

    /** @brief IPv4/6 Unicast packet, triggered for routing, is Trapped/Mirrored
     *  due to Unicast RPF Check failure.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_UC_RPF_FAIL_E                      = 176,

    /** @brief IPv4/6 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU Due to Route entry Multicast RPF Fail command.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_MC_ROUTE_RPF_FAIL_E                = 177,

    /** @brief IPv4/6 Multicast packet, triggered for routing, Trapped/Mirrored
     *  to CPU Due to MLL entry Multicast RPF Fail command.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_6_MC_MLL_RPF_FAIL_E                  = 178,

    /** @brief ARP Broadcast packet, whose destination IP address is the host
     *  address of the router, as indicated in the
     *  Route entry <Trap/Mirror ARP BC to CPU>.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_ARP_BC_TO_ME_E                            = 179,

    /** @brief IPv4 Unicast packet, triggered for routing, Mirrored
     *  to CPU due to ICMP redirect.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV4_UC_ICMP_REDIRECT_E                   = 180,

    /** @brief IPv6 Unicast packet, triggered for routing, Mirrored
     *  to CPU due to ICMP redirect.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IPV6_UC_ICMP_REDIRECT_E                   = 181,

    /** @brief Indication that
     *  the packet was trapped/mirrored to the CPU due to the bidirectional
     *  tree RPF check failure.(Router)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_IP_MC_ROUTE_BIDIR_RPF_FAIL_E              = 182,

    /** @brief Used when a non-conforming
     *  packet is dropped by the Ingress Policer engine.(PLR)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_MET_RED_DROP_CODE_E                       = 185,

    /** @brief Used when a packet is dropped
     *  by the Ingress Policer engine due to an ECC error.(IPLR)
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_ECC_DROP_CODE_E                           = 186,

    /** @brief The packet is an ARP reply, and forwarded by the FDB
     *  entry with <destination port> = CPU port 63, and <application specific CPU code> = 1.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_ARP_REPLY_TO_ME_E                         = 188,

    /** @brief The packet is sent with a FROM_CPU DSA tag with
     *  the <mirror2CPU> bit set and this mirrored to the local CPU.
     *  PRV_CPSS_DXCH_NET_DSA_TAG_CPU_TO_ALL_CPUS_E       - The packet is sent witha FROM_CPU DSA Tag with
     *  the <mirror2CPU> bit set and thus mirrored to a
     *  local CPU.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_TO_ALL_CPUS_E                         = 189,

    /** @brief The packet is Trapped, Mirrored, forwarded to the
     *  CPU port 63, or forwarded to the Virtual Router port 63,
     *  and the packet is a TCP SYN packet.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_TCP_SYN_TO_CPU_E                          = 190,

    /** @brief The packet is
     *  sent to virtual router port
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_PACKET_TO_VIRTUAL_ROUTER_PORT_E           = 191,

    /** @brief User-Defined CPU codes. The user may use this
     *  range for any application specific purpose.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_FIRST_USER_DEFINED_E                      = 192,

    /** @brief User-Defined CPU codes. The user may use this
     *  range for any application specific purpose.
     */
    PRV_CPSS_DXCH_NET_DSA_TAG_LAST_USER_DEFINED_E               = PRV_CPSS_DXCH_NET_DSA_TAG_FIRST_USER_DEFINED_E + 63,

    /** one more than the last */
    PRV_CPSS_DXCH_NET_DSA_TAG_MAX_E

} PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT;

/**
* @struct PRV_CPSS_DXCH_NET_INFO_STC
 *
 * @brief A structure to hold all PP data needed on networkIf DXCH layer
*/
typedef struct{

    /** @brief was the initalization of the networkIf done.
     *  it is needed for 'restart after shutdown' with out HW reset.
     *  because we need to synchronize the PP with the SW regarding
     *  the Rx,Tx descriptors , buffers.
     */
    GT_BOOL initDone;

    /** @brief Enables/Disable pre
     *  header to all packets forwarded to the CPU
     *  (via the CPU port or the PCI interface).
     *  This two-byte header is used to align the
     *  IPv4 header to 32 bits.
     *  GT_TRUE - Two-byte header is pre-pended to
     *  packets forwarded to the CPU.
     *  GT_FALSE - Two-byte header is NOT pre-pended
     *  to packets forwarded to the CPU.
     */
    GT_BOOL prePendTwoBytesHeader;

    CPSS_RX_BUFF_ALLOC_METHOD_ENT allocMethod[NUM_OF_SDMA_QUEUES];

    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT rateMode[CPSS_MAX_SDMA_CPU_PORTS_CNS][NUM_OF_TX_QUEUES];

    GT_U64 rateValue[CPSS_MAX_SDMA_CPU_PORTS_CNS][NUM_OF_TX_QUEUES];

} PRV_CPSS_DXCH_NET_INFO_STC;

/* the MACRO convert the netIfNum to mgUnitId                    */
/* on error : the MACRO will return error (and exit the function)*/
#define PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,_mgUnitId)\
    {                                                                          \
        GT_STATUS _rc;                                                         \
        _mgUnitId = 0;                                                         \
        _rc = prvCpssDxChNetifCheckNetifNumAndConvertToMgUnitId(devNum,netIfNum,&_mgUnitId); \
        if (_rc != GT_OK)                                                      \
        {                                                                      \
            return _rc;                                                        \
        }                                                                      \
    }

/* MACRO to allow loops on CPSS_MAX_SDMA_CPU_PORTS_CNS and to access only valid interfaces */
/* initialization is during : setSip5FlexInfo from hwPpPhase1Part1 */
/* and this info is regardless to bind 'CPU SDMA' of 'port mapping'*/
#define PRV_CPSS_DXCH_NETIF_SKIP_NON_EXISTS_NETIF_NUM_MAC(devNum,cpuIndex)     \
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[cpuIndex].valid == GT_FALSE)   \
        continue

/* MACRO to allow loops on CPSS_MAX_SDMA_CPU_PORTS_CNS and to access only used interfaces (valid and used !!!)*/
#define PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum,cpuIndex)     \
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[cpuIndex].valid == GT_FALSE ||         \
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[cpuIndex].usedAsCpuPort == GT_FALSE)   \
        continue



/**
* @internal prvCpssDxChNetIfCpuToDsaCode function
* @endinternal
*
* @brief   Convert the CPU code to the DSA CPU code.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cpuCode                  - CPU code.
*
* @param[out] dsaCpuCodePtr            - DSA CPU code
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - None DSA code for this CPU code.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not all cpu codes translation implemented
*
*/
GT_STATUS prvCpssDxChNetIfCpuToDsaCode
(
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode,
    OUT PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT *dsaCpuCodePtr
);

/**
* @internal prvCpssDxChNetIfDsaToCpuCode function
* @endinternal
*
* @brief   Convert the DSA CPU code to the CPU code.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaCpuCode               - DSA CPU code
*
* @param[out] cpuCodePtr               - CPU code.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - None DSA code for this CPU code.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not all cpu codes translation implemented
*
*/
GT_STATUS prvCpssDxChNetIfDsaToCpuCode
(
    IN  PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT   dsaCpuCode,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
);


/**
* @internal prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle function
* @endinternal
*
* @brief   This routine frees all transmitted packets descriptors. In addition, all
*         user relevant data in Tx End FIFO. -- SDMA relate.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - The device number the packet was transmitted from
* @param[in] rawQueueIdx              - The (raw) queue index encapsulate the MG unit index that
*                                       the packet was transmitted upon.
*                                       GT_OK on success, or
*
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - the CPSS does not handle the ISR for the device
*                                       so function must not be called
*                                       GT_FAIL otherwise.
*
* @note Invoked by ISR routine !!
*       the function is called internally in the CPSS from the driver as a call
*       back , see bind in function hwPpPhase1Part1(...)
*
*/
GT_STATUS prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle
(
    IN GT_U8                devNum,
    IN GT_U8                rawQueueIdx
);

/**
* @internal prvCpssDxChNetIfMiiInitNetIfDev function
* @endinternal
*
* @brief   Initialize the network interface structures, Rx buffers and Tx header
*         buffers (For a single device).
* @param[in] devNum                   - device number
* @param[in] miiInitPtr               - MII Init parameters
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChNetIfMiiInitNetIfDev
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_NETIF_MII_INIT_STC *miiInitPtr
);

/**
* @internal prvCpssDxChNetIfMiiPacketTx function
* @endinternal
*
* @brief   This function receives packet buffers & parameters from Application .
*         Prepares them for the transmit operation, and transmits a packet
*         through the ethernet port.
* @param[in] devNum                   - dev num
* @param[in] pcktParamsPtr            - (pointer to)The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The packet data buffers list.
* @param[in] buffLenArr[]             - A list of the buffers len in packetBuffsArrPtr.
* @param[in] numOfBufs                - Length of packetBuffsArrPtr.
*                                       GT_OK on success, or
*                                       GT_NO_RESOURCE if there is not enough desc. for enqueuing the packet, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChNetIfMiiPacketTx
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC      *pcktParamsPtr,
    IN GT_U8                            *packetBuffsArrPtr[],
    IN GT_U32                           buffLenArr[],
    IN GT_U32                           numOfBufs
);

/**
* @internal prvCpssDxChNetIfMiiRxGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
* @param[in,out] numOfBuffPtr             - Num of used buffs in packetBuffsArrPtr.
*
* @param[out] packetBuffsArrPtr[]      - The received packet buffers list.
* @param[out] buffLenArr[]             - List of buffer lengths for packetBuffs.
* @param[out] rxParamsPtr              - (Pointer to)information parameters of received packets
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,queueIdx.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
*
* @note It is recommended to call cpssDxChNetIfRxBufFree for this queue
*       i.e. return the buffer to their original queue.
*
*/
GT_STATUS prvCpssDxChNetIfMiiRxGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[],
    OUT CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
);

/**
* @internal prvCpssDxChNetifCheckNetifNumAndConvertToMgUnitId function
* @endinternal
*
* @brief   check netIfNum and convert to mgUnitId
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp device number.
* @param[in] netIfNum                 - The network interface number.
* @param[out] mgUnitIdPtr             - MG unitId (MG0 .. MG15).
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChNetifCheckNetifNumAndConvertToMgUnitId(
    IN GT_U8    devNum,
    IN GT_U32   netIfNum,
    IN GT_U32   *mgUnitIdPtr
);
/**
* @internal prvCpssDxChNetifEventSdmaCpuPortConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping for port per event type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] evConvertType            - event convert type
* @param[in] IN_queue                 - queue to convert
*
* @param[out] OUT_queuePtr            - (pointer to) converted queue, according to convert event type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNumFrom
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChNetifEventSdmaCpuPortConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  PRV_CPSS_EV_CONVERT_DIRECTION_ENT    evConvertType,
    IN  GT_U32                          IN_queue,
    OUT GT_U32                          *OUT_queuePtr
);

/**
* @internal prvCpssDxChNetIfInitAfterPortMapping function
* @endinternal
*
* @brief   build the information about all the network interfaces according to 'port mapping' params
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssDxChNetIfInitAfterPortMapping
(
    IN GT_U8                        devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssDxChNetIfh */


