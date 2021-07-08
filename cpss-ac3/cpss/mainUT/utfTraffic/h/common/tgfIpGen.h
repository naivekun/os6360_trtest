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
* @file tgfIpGen.h
*
* @brief Generic API for IP
*
* @version   63
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfIpGenh
#endif /*CHX_FAMILY*/

#ifndef __tgfIpGenh
#define __tgfIpGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfInlifGen.h>
#include <common/tgfMplsGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfTcamManagerGen.h>
#include <common/tgfTunnelGen.h>
#include <utf/private/prvUtfExtras.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
    #include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
    #include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
    #include <cpss/dxCh/dxChxGen/ip/cpssDxChIpNat.h>
    #include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>
    #include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
    #include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
    #include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/ip/cpssExMxPmIpStat.h>
    #include <cpss/exMxPm/exMxPmGen/ip/cpssExMxPmIpTables.h>
    #include <cpss/exMxPm/exMxPmGen/ip/cpssExMxPmIpTypes.h>
    #include <cpss/exMxPm/exMxPmGen/ip/cpssExMxPmIpCtrl.h>
    #include <cpss/exMxPm/exMxPmGen/ipLpmEngine/cpssExMxPmIpLpm.h>
    #include <cpss/exMxPm/exMxPmGen/inlif/cpssExMxPmInlif.h>
    #include <cpss/exMxPm/exMxPmGen/dit/cpssExMxPmDit.h>
    #include <cpss/exMxPm/exMxPmGen/ipLpmEngine/cpssExMxPmIpLpmDbg.h>
    #include <cpss/generic/port/cpssPortTx.h>
#endif /* EXMXPM_FAMILY */

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_IP_ROUTING_MODE_ENT
 *
 * @brief enumerator for IP_TCAM_ROUTE_ENTRY_INFO
 * see CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
*/
typedef enum{

    PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E,

    PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E,

    /** on the case of fail of detection */
    PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E

} PRV_TGF_IP_ROUTING_MODE_ENT;

/**
* @enum PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT
 *
 * @brief Each route entry block has a type.
*/
typedef enum{

    /** Equal-Cost-MultiPath block */
    PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E,

    /** Class-of-Service block */
    PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E,

    /** regular route entry */
    PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E,

    /** regular route entry */
    PRV_TGF_IP_MULTIPATH_ROUTE_ENTRY_GROUP_E


} PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT;

/* decide the enumeratiom type for regular route entry - Bobcat2 uses
   PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E while other devices use
   PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E */
#define PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC                                   \
 (((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))) ?    \
 PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E : PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E)

/**
* @enum PRV_TGF_RPF_IF_MODE_ENT
 *
 * @brief Different modes for unicast RPF check
*/
typedef enum{

    /** RPF check is bypassed */
    PRV_TGF_RPF_IF_MODE_DISABLED_E = 0,

    /** packet source port/trunk is compared to the entry <RPF interface> */
    PRV_TGF_RPF_IF_MODE_PORT_E     = 1,

    /** packet's VLAN is compared to the entry <RPF interface> */
    PRV_TGF_RPF_IF_MODE_L2_VLAN_E  = 2

} PRV_TGF_RPF_IF_MODE_ENT;

/**
* @enum PRV_TGF_IP_HOP_LIMIT_MODE_ENT
 *
 * @brief Enumeration of Hop Limit Manipulation mode
*/
typedef enum{

    /** @brief Hop Limit of routed packets
     *  associated with this Next Hop
     *  Entry is not decremented when
     *  the packet is transmitted from
     *  this device.
     */
    PRV_TGF_IP_HOP_LIMIT_NOT_DECREMENTED_E    = 0,

    /** @brief Hop Limit of routed packets
     *  associated with this Next Hop
     *  Entry is decremented by 1 when
     *  the packet is transmitted from
     *  this device.
     */
    PRV_TGF_IP_HOP_LIMIT_DECREMENTED_BY_ONE_E = 1

} PRV_TGF_IP_HOP_LIMIT_MODE_ENT;

/**
* @enum PRV_TGF_IP_TTL_MODE_ENT
 *
 * @brief Enumeration of TTL Manipulation mode.
*/
typedef enum{

    /** @brief = TTL of routed packets associated
     *  with this Next Hop Entry is not decremented when the packet is
     *  transmitted from this device.
     */
    PRV_TGF_IP_TTL_NOT_DECREMENTED_E      = 0,

    /** @brief = TTL of routed packets associated
     *  with this Next Hop Entry is decremented by 1 when the packet is
     *  transmitted from this device.
     */
    PRV_TGF_IP_TTL_DECREMENTED_BY_ONE_E   = 1,

    /** @brief = TTL of routed packets
     *  associated with this Next Hop Entry is decremented by <TTL> when
     *  the packet is transmitted from this device.
     */
    PRV_TGF_IP_TTL_DECREMENTED_BY_ENTRY_E = 2,

    /** @brief = TTL of routed packets associated with
     *  this Next Hop Entry is assigned by this entry to a new TTL
     *  value, when the packet is transmitted from this device.
     */
    PRV_TGF_IP_TTL_COPY_ENTRY_E           = 3

} PRV_TGF_IP_TTL_MODE_ENT;

/**
* @enum PRV_TGF_IP_MC_RPF_FAIL_COMMAND_MODE_ENT
 *
 * @brief decides on the way the RPF fail command is chosen when an RPF
 * fail occurs:
*/
typedef enum{

    /** use the MC route entry */
    PRV_TGF_IP_MC_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E,

    /** Use the MLL entry */
    PRV_TGF_IP_MC_MLL_RPF_FAIL_COMMAND_MODE_E

} PRV_TGF_IP_MC_RPF_FAIL_COMMAND_MODE_ENT;

/**
* @enum PRV_TGF_PAIR_READ_WRITE_FORM_ENT
 *
 * @brief the possible forms to write a pair of entries
*/
typedef enum{

    /** @brief reads/writes just the
     *  first entry of the pair
     */
    PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,

    /** @brief reads/writes
     *  just the second entry of the pair and the next pointer
     */
    PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E,

    /** reads/writes the whole pair */
    PRV_TGF_PAIR_READ_WRITE_WHOLE_E

} PRV_TGF_PAIR_READ_WRITE_FORM_ENT;

/* Old enumerators - remained for backward compatibility, unused in the code */
#define PRV_TGF_IP_MLL_PAIR_READ_WRITE_FIRST_MLL_ONLY_E                     PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E
#define PRV_TGF_IP_MLL_PAIR_READ_WRITE_SECOND_MLL_NEXT_POINTER_ONLY_E       PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E
#define PRV_TGF_IP_MLL_PAIR_READ_WRITE_WHOLE_E                              PRV_TGF_PAIR_READ_WRITE_WHOLE_E
#define PRV_TGF_IP_MLL_PAIR_READ_WRITE_FORM_ENT                             PRV_TGF_PAIR_READ_WRITE_FORM_ENT

/**
* @enum PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT
 *
 * @brief the types of IP shadows.
*/
typedef enum{

    PRV_TGF_IP_TCAM_ROUTER_BASED_SHADOW_E = 0,

    PRV_TGF_IP_TCAM_POLICY_BASED_ROUTING_SHADOW_E,

    /** routing is router RAM based */
    PRV_TGF_IP_RAM_SHADOW_E,

    /** last value in the enum */
    PRV_TGF_IP_TCAM_SHADOW_TYPE_LAST_E

} PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT;


/**
* @enum PRV_TGF_IP_EXCEPTION_TYPE_ENT
 *
 * @brief The Exception Types.
*/
typedef enum{

    /** Packet with an error in IP header */
    PRV_TGF_IP_EXCP_HDR_ERROR_E                 = 0,

    /** @brief Packet with illegal address
     *  PRV_TGF_IP_EXCP_DIP_DA_MISMATCH DIP to DA mismatch
     *  PRV_TGF_IP_EXCP_MTU_EXCEEDED_E- IP MTU EXCEEDED exception
     *  PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E- IP ALL ZERO exception
     *  PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E- IP Option/HopByHop Exception
     *  PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E- IP non HopByHop Extention
     *  supported only in CHX_FAMILY
     *  PRV_TGF_IP_EXCP_TTL_EXCEED_E- IP TTL Exceeded error
     *  PRV_TGF_IP_EXCP_RPF_FAIL_E- IP RPF FAIL
     *  PRV_TGF_IP_EXCP_SIP_SA_FAIL_E- IP SIP to SA match failure
     *  PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E- IP UC MTU EXCEEDED for non "don't fragment" (DF) packets exception supported only for Ch3 ipv4
     *  PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E- IP UC MTU EXCEEDED for "don't fragment" (DF) packets exception supported only for Ch3 ipv4
     *  supported only in EXMXPM_FAMILY
     *  PRV_TGF_IP_EXCP_SIP_SA_MISMATCH_E- Packet with mismatch between SIP and SA
     *  PRV_TGF_IP_EXCP_TTL0_EXCEED_E- Packet with TTL/Hop-Limit with value of 0.
     *  PRV_TGF_IP_EXCP_TTL1_EXCEED_E- Packets with TTL/Hop-Limit is 0 after the optional TTL/Hop-Limit decrease.
     *  PRV_TGF_IP_EXCP_URPF_FAIL_E- Packets for which Unicast RPF check has failed. This is for Unicast packets only.
     */
    PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E           = 1,

    PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E           = 2,

    PRV_TGF_IP_EXCP_MTU_EXCEEDED_E              = 3,

    PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E              = 4,

    PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E         = 5,

    PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E        = 6,

    PRV_TGF_IP_EXCP_TTL_EXCEED_E                = 7,

    PRV_TGF_IP_EXCP_RPF_FAIL_E                  = 8,

    PRV_TGF_IP_EXCP_SIP_SA_FAIL_E               = 9,

    PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E   = 10,

    PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E       = 11,

    PRV_TGF_IP_EXCP_SIP_SA_MISMATCH_E           = 12,

    PRV_TGF_IP_EXCP_TTL0_EXCEED_E               = 13,

    PRV_TGF_IP_EXCP_TTL1_EXCEED_E               = 14,

    PRV_TGF_IP_EXCP_URPF_FAIL_E                 = 15

} PRV_TGF_IP_EXCEPTION_TYPE_ENT;

/**
* @enum PRV_TGF_IP_EXC_CMD_ENT
 *
 * @brief This enum defines the exception command.
*/
typedef enum{

    /** trap packet to CPU */
    PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E         = 0,

    /** hard drop packet */
    PRV_TGF_IP_EXC_CMD_DROP_HARD_E           = 1,

    /** soft drop packet */
    PRV_TGF_IP_EXC_CMD_DROP_SOFT_E           = 2,

    /** IP Forward the packets */
    PRV_TGF_IP_EXC_CMD_ROUTE_E               = 3,

    /** @brief Packet is routed and mirrored to the CPU.
     *  PRV_TGF_IP_EXCT_CMD_BRIDGE_AND_MIRROR_E  Bridge and Mirror to CPU.
     *  DF marked packets are trapped to CPU
     *  DF marked packets are bridged and mirrored to CPU
     */
    PRV_TGF_IP_EXC_CMD_ROUTE_AND_MIRROR_E    = 4,

    PRV_TGF_IP_EXC_CMD_BRIDGE_AND_MIRROR_E   = 5,

    /** Bridge only */
    PRV_TGF_IP_EXC_CMD_BRIDGE_E              = 6,

    /** Do nothing. (disable) */
    PRV_TGF_IP_EXC_CMD_NONE_E                = 7,

    /** mirror packet to CPU */
    PRV_TGF_IP_EXC_CMD_MIRROR_TO_CPU_E       = 8,

    /** forward packet */
    PRV_TGF_IP_EXC_CMD_FORWARD_E             = 9,

    /** loopback packet is send back to originator */
    PRV_TGF_IP_EXC_CMD_LOOPBACK_E            = 10,

    /** same as PRV_TGF_IP_EXC_CMD_ROUTE_E but packet can be failed by loose uRPF */
    PRV_TGF_IP_EXC_CMD_DEFAULT_ROUTE_ENTRY_E = 11,

    /** Packet is forwarded unless it is marked with DF (Don't Fragment). */
    PRV_TGF_IP_EXC_CMD_ROUTE_ALL_TRAP_DF_E   = 12,

    /** Packet is routed unless it is marked with DF (Don't Fragment). */
    PRV_TGF_IP_EXC_CMD_DONT_ROUTE_DF_E       = 13

} PRV_TGF_IP_EXC_CMD_ENT;

/**
* @enum PRV_TGF_IP_HEADER_ERROR_ENT
 *
 * @brief types of IP header errors
*/
typedef enum{

    /** Incorrect checksum */
    PRV_TGF_IP_HEADER_ERROR_CHECKSUM_ENT = 0,

    /** Invalid version */
    PRV_TGF_IP_HEADER_ERROR_VERSION_ENT = 1,

    /** @brief For IPv4 packet:
     *  IPv4 header <total Length> + length of L2 header + 4 (CRC length) <=
     *  MAC layer packet byte count
     *  For IPv6 packet:
     *  IPv6 header <Payload Length> + 40 (IPv6 header length) + 4 (CRC length) <=
     *  MAC layer packet byte count
     */
    PRV_TGF_IP_HEADER_ERROR_LENGTH_ENT  = 2,

    /** SIP and DIP addresses are equal */
    PRV_TGF_IP_HEADER_ERROR_SIP_DIP_ENT = 3

} PRV_TGF_IP_HEADER_ERROR_ENT;

/**
* @enum PRV_TGF_LPM_NEXT_POINTER_TYPE_ENT
 *
 * @brief Defines the different types of LPM structures that may be pointed by a
 * next pointer in an lpm node.
*/
typedef enum{

    /** Regular node. */
    PRV_TGF_LPM_REGULAR_NODE_PTR_TYPE_E      = 0,

    /** one-line compressed node */
    PRV_TGF_LPM_COMPRESSED_1_NODE_PTR_TYPE_E = 1,

    /** two-lines compressed node */
    PRV_TGF_LPM_COMPRESSED_2_NODE_PTR_TYPE_E = 2,

    /** Next Hop entry. */
    PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E  = 3,

    /** ECMP entry */
    PRV_TGF_LPM_ECMP_ENTRY_PTR_TYPE_E        = 4,

    /** QoS entry */
    PRV_TGF_LPM_QOS_ENTRY_PTR_TYPE_E         = 5

} PRV_TGF_LPM_NEXT_POINTER_TYPE_ENT;

/**
* @enum PRV_TGF_IP_BRG_SERVICE_ENT
 *
 * @brief the router bridge services
*/
typedef enum{

    /** @brief IP Header Check for Bridged
     *  Unicast IPv4/6 Packets
     */
    PRV_TGF_IP_HEADER_CHECK_BRG_SERVICE_E = 0,

    /** @brief Unicast RPF Check for Bridged
     *  IPv4/6 and ARP Packets
     */
    PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E = 1,

    /** @brief SIP/SA Check for Bridged Unicast
     *  IPv4/6 and ARP Packets
     */
    PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E = 2,

    /** @brief SIP Filtering for Bridged Unicast
     *  IPv4/6 and ARP Packets
     */
    PRV_TGF_IP_SIP_FILTER_BRG_SERVICE_E   = 3,

    /** last element in enum */
    PRV_TGF_IP_BRG_SERVICE_LAST_E         = 4

} PRV_TGF_IP_BRG_SERVICE_ENT;

/**
* @enum PRV_TGF_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT
 *
 * @brief the router bridge services enable/disable mode
*/
typedef enum{

    /** @brief enable/disable the service
     *  for ipv4 packets
     */
    PRV_TGF_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E = 0,

    /** @brief enable/disable the service
     *  for ipv6 packets
     */
    PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E = 1,

    /** @brief enable/disable the service
     *  for arp packets
     */
    PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E  = 2

} PRV_TGF_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT;

/**
* @struct PRV_TGF_IP_UC_ROUTE_ENTRY_STC
 *
 * @brief Unicast Route Entry
*/
typedef struct{

    /** @brief Route entry command
     *  cpuCodeIdx     - the cpu code index
     *  appSpecificCpuCodeEnable- Enable CPU code overwritting
     */
    CPSS_PACKET_CMD_ENT cmd;

    GT_U32 cpuCodeIndex;

    GT_BOOL appSpecificCpuCodeEnable;

    /** @brief Enable: Assign the command HARD DROP if the
     *  SIP lookup matches this entry
     *  Disable: Do not assign the command HARD DROP due to
     *  the SIP lookup matching this entry
     */
    GT_BOOL unicastPacketSipFilterEnable;

    /** @brief mirror to ingress analyzer.
     *  qosProfileMarkingEnable- Enable Qos profile assignment.
     */
    GT_BOOL ingressMirror;

    /** The counter set this route entry is linked to */
    CPSS_IP_CNT_SET_ENT countSet;

    /** @brief enable Trap/Mirror ARP Broadcasts with DIP matching
     *  this entry
     */
    GT_BOOL trapMirrorArpBcEnable;

    /** The security level associated with the SIP. */
    GT_U32 sipAccessLevel;

    /** The security level associated with the DIP. */
    GT_U32 dipAccessLevel;

    /** @brief Enable performing ICMP Redirect Exception Mirroring.
     *  scopeCheckingEnable- Enable IPv6 Scope Checking.
     */
    GT_BOOL ICMPRedirectEnable;

    GT_BOOL scopeCheckingEnable;

    /** The site id of this route entry. */
    CPSS_IP_SITE_ID_ENT siteId;

    /** One of the eight global configurable MTU sizes (0..7). */
    GT_U32 mtuProfileIndex;

    /** the output vlan id */
    GT_U16 nextHopVlanId;

    /** @brief Enable TTL/Hop Limit Decrement
     *  ttlHopLimDecOptionsExtChkByPass- TTL/HopLimit Decrement and option/
     *  Extention check bypass.
     */
    GT_BOOL ttlHopLimitDecEnable;

    GT_BOOL ttlHopLimDecOptionsExtChkByPass;

    GT_BOOL qosProfileMarkingEnable;

    /** the qos profile */
    GT_U32 qosProfileIndex;

    /** @brief whether this packet Qos parameters can be overridden
     *  after this assigment.
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;

    /** whether to change the packets UP and how. */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyUp;

    /** whether to change the packets DSCP and how. */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyDscp;

    /** @brief whether this nexthop is tunnel start enrty
     *  isTunnelStart = GT_TRUE and isNat = GT_TRUE
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL isTunnelStart;

    /** @brief Indicate that the packet is subject to NAT,
     *  and nextHopNatPointer should be used as a pointer to a NAT entry.
     *  Relevant only if isTunnelStart = GT_TRUE
     *  GT_FALSE: use nextHopTunnelPointer as a Tunnel pointer
     *  GT_TRUE: use nextHopNatPointer as a NAT pointer
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL isNat;

    /** whether this nexthop outlif is DIT */
    GT_BOOL isOutlifDit;

    /** the output interface this next hop sends to */
    CPSS_INTERFACE_INFO_STC nextHopInterface;

    /** The ARP Pointer indicating the routed packet MAC DA */
    GT_U32 nextHopARPPointer;

    /** @brief The next hop DIT Pointer
     *  nextHopTunnelPointer- the tunnel pointer
     */
    GT_U32 nextHopDITPointer;

    GT_U32 nextHopTunnelPointer;

    /** the NAT pointer in case that */
    GT_U32 nextHopNatPointer;

    /** @brief This field is GT_TRUE when the Route entry is
     *  accessed for IP forwarding.
     */
    GT_BOOL ageRefresh;

    /** @brief Unicast RPF or ICMP redirect mode. Valid options:
     *  PRV_TGF_RPF_IF_MODE_DISABLED_E
     *  PRV_TGF_RPF_IF_MODE_PORT_E
     *  PRV_TGF_RPF_IF_MODE_L2_VLAN_E
     */
    PRV_TGF_RPF_IF_MODE_ENT ucRpfOrIcmpRedirectIfMode;

    /** Enables/disables SIP/SA check */
    GT_BOOL sipSaCheckEnable;

    /** @brief Whether to decrement/not decrement Hop Limit.
     *  Relevant for IPv6 packets that are not assigned
     *  an MPLS label only.
     */
    PRV_TGF_IP_HOP_LIMIT_MODE_ENT hopLimitMode;

    /** @brief TTL used for assignment of the packet's new TTL.
     *  Use of this field is controlled by ttlMode.
     */
    GT_U32 ttl;

    /** TTL manipulation mode */
    PRV_TGF_IP_TTL_MODE_ENT ttlMode;

    /** The label to use if mplsCommand = PRV_TGF_MPLS_PUSH_CMD_E */
    GT_U32 mplsLabel;

    /** MPLS command assigned to the packet. */
    PRV_TGF_MPLS_CMD_ENT mplsCommand;

    /** Enable/Disable modifing per QoS parameter */
    PRV_TGF_QOS_PARAM_MODIFY_STC qosParamsModify;

    /** @brief QoS Parameters in case <qosParamsModify> for the
     *  parameter is enabled
     */
    PRV_TGF_QOS_PARAM_STC qosParams;

} PRV_TGF_IP_UC_ROUTE_ENTRY_STC;

/**
* @struct PRV_TGF_IP_ROUTE_ENTRY_STC
 *
 * @brief LTT (lookup translation table) entry
*/
typedef struct{

    /** Route Entry Type */
    PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT routeType;

    /** The number of route paths bound to the lookup */
    GT_U32 numOfPaths;

    /** index to the single Route entry */
    GT_U32 routeEntryBaseIndex;

    /** the size of the route entries group */
    GT_U32 blockSize;

    /** Enable Unicast RPF check for this Entry */
    GT_BOOL ucRPFCheckEnable;

    /** Enable Unicast SIP MAC SA match check */
    GT_BOOL sipSaCheckMismatchEnable;

    /** This is the IPv6 Multicast group scope level */
    CPSS_IPV6_PREFIX_SCOPE_ENT ipv6MCGroupScopeLevel;

} PRV_TGF_IP_ROUTE_ENTRY_STC;

/*
 * Typedef: struct PRV_TGF_IP_COUNTER_SET_STC
 *
 * Description: IPvX Counter Set
 *
 * Fields:
 *      inUcPkts                 - number of ingress unicast packets
 *      inMcPkts                 - number of ingress multicast packets
 *      inUcNonRoutedExcpPkts    - number of ingress Unicast packets that were
 *                                 not routed due to one or more exceptions.
 *      inUcNonRoutedNonExcpPkts - number of ingress Unicast packets that were
 *                                 not routed but didn't recive any excption.
 *      inMcNonRoutedExcpPkts    - number of ingress multicast packets that were
 *                                 not routed due to one or more exceptions.
 *      inMcNonRoutedNonExcpPkts - number of ingress multicast packets that were
 *                                 not routed but didn't recive any excption.
 *      inUcTrappedMirrorPkts    - number of ingress unicast packets that were
 *                                 trapped or mirrored by the router engine.
 *      inMcTrappedMirrorPkts    - number of ingress multicast packets that were
 *                                 trapped or mirrored by the router engine.
 *      mcRfpFailPkts            - number of multicast packets with fail RPF.
 *      outUcRoutedPkts          - the number of egress unicast routed packets.
 */
typedef struct PRV_TGF_IP_COUNTER_SET_STCT
{
    GT_U32 inUcPkts;
    GT_U32 inMcPkts;
    GT_U32 inUcNonRoutedExcpPkts;
    GT_U32 inUcNonRoutedNonExcpPkts;
    GT_U32 inMcNonRoutedExcpPkts;
    GT_U32 inMcNonRoutedNonExcpPkts;
    GT_U32 inUcTrappedMirrorPkts;
    GT_U32 inMcTrappedMirrorPkts;
    GT_U32 mcRfpFailPkts;
    GT_U32 outUcRoutedPkts;
} PRV_TGF_IP_COUNTER_SET_STC;

/**
* @struct PRV_TGF_IP_LTT_ENTRY_STC
 *
 * @brief LTT (lookup translation table) entry
*/
typedef struct{

    /** route Entry Type. */
    PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT routeType;

    /** the number of route paths bound to the lookup */
    GT_U32 numOfPaths;

    /** index to Route entry */
    GT_U32 routeEntryBaseIndex;

    /** enable Unicast RPF check for this Entry */
    GT_BOOL ucRPFCheckEnable;

    /** enable Unicast SIP MAC SA match check */
    GT_BOOL sipSaCheckMismatchEnable;

    /** this is the IPv6 Multicast group scope level */
    CPSS_IPV6_PREFIX_SCOPE_ENT ipv6MCGroupScopeLevel;

    /** fdb or lpm lookup priority */
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT  priority;

} PRV_TGF_IP_LTT_ENTRY_STC;

/**
* @union PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT
 *
 * @brief This union holds the two possible accosiactions a prefix can have
 * for Cheetah+ devices it will be a PCL action.
 * for Cheetah2 devices it will be a LTT entry.
 *
*/

typedef union{
    PRV_TGF_PCL_ACTION_STC pclIpUcAction;

    PRV_TGF_IP_LTT_ENTRY_STC ipLttEntry;

} PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT;


/**
* @struct PRV_TGF_IPV4_PREFIX_STC
 *
 * @brief IPv4 prefix entry
*/
typedef struct{

    /** virtual router id */
    GT_U32 vrId;

    /** the IP address of this prefix */
    GT_IPADDR ipAddr;

    /** enable\disable writing MC source ip */
    GT_BOOL isMcSource;

    /** mc Group index row */
    GT_U32 mcGroupIndexRow;

    /** mc Group index column */
    GT_U32 mcGroupIndexColumn;

} PRV_TGF_IPV4_PREFIX_STC;

/**
* @struct PRV_TGF_IPV6_PREFIX_STC
 *
 * @brief IPv6 prefix entry
*/
typedef struct{

    /** virtual router id */
    GT_U32 vrId;

    /** the IP address of this prefix */
    GT_IPV6ADDR ipAddr;

    /** enable\disable writing MC source ip */
    GT_BOOL isMcSource;

    /** mc Group index row */
    GT_U32 mcGroupIndexRow;

} PRV_TGF_IPV6_PREFIX_STC;

/**
* @struct PRV_TGF_IP_MC_ROUTE_ENTRY_STC
 *
 * @brief Representation of the IP_MC_ROUTE_ENTRY in HW,
*/
typedef struct
{
    CPSS_PACKET_CMD_ENT                         cmd;
    GT_U32                                      cpuCodeIndex;
    GT_BOOL                                     appSpecificCpuCodeEnable;
    GT_BOOL                                     ttlHopLimitDecEnable;
    GT_BOOL                                     ttlHopLimDecOptionsExtChkByPass;
    GT_BOOL                                     ingressMirror;
    GT_BOOL                                     qosProfileMarkingEnable;
    GT_U32                                      qosProfileIndex;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT       modifyUp;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT       modifyDscp;
    CPSS_IP_CNT_SET_ENT                         countSet;
    GT_BOOL                                     multicastRPFCheckEnable;
    GT_U16                                      multicastRPFVlan;
    PRV_TGF_IP_MC_RPF_FAIL_COMMAND_MODE_ENT     mcRPFFailCmdMode;
    CPSS_PACKET_CMD_ENT                         RPFFailCommand;
    GT_BOOL                                     scopeCheckingEnable;
    CPSS_IP_SITE_ID_ENT                         siteId;
    GT_U32                                      mtuProfileIndex;
    GT_U32                                      ipv4MllPtr;
    GT_U32                                      ipv6InternalMllPtr;
    GT_U32                                      ipv6ExternalMllPtr;
    GT_BOOL                                     bypassTtlExceptionCheckEnable;
    GT_BOOL                                     ageRefresh;
    GT_U32                                      ipv6ScopeLevel;
    PRV_TGF_RPF_IF_MODE_ENT                     mcRpfIfMode;
    CPSS_INTERFACE_INFO_STC                     mcRpfIf;
    PRV_TGF_OUTLIF_INFO_STC                     nextHopIf;
    PRV_TGF_IP_HOP_LIMIT_MODE_ENT               hopLimitMode;
    PRV_TGF_QOS_PARAM_MODIFY_STC                qosParamsModify;
    PRV_TGF_QOS_PARAM_STC                       qosParams;
    GT_U32                                      ttl;
    PRV_TGF_IP_TTL_MODE_ENT                     ttlMode;
    GT_U32                                      mplsLabel;
    PRV_TGF_MPLS_CMD_ENT                        mplsCommand;
} PRV_TGF_IP_MC_ROUTE_ENTRY_STC;

/**
* @struct PRV_TGF_IP_MLL_STC
 *
 * @brief Representation of the CPSS_DXCH_IP_MLL_STC in HW,
*/
typedef struct{

    /** the MLL entry RPF fail command */
    CPSS_PACKET_CMD_ENT mllRPFFailCommand;

    /** weather this nexthop is tunnel start enrty */
    GT_BOOL isTunnelStart;

    /** the output interface this mll entry sends to */
    CPSS_INTERFACE_INFO_STC nextHopInterface;

    /** VLAN */
    GT_U16 nextHopVlanId;

    /** the tunnel pointer in case this is a tunnel start */
    GT_U32 nextHopTunnelPointer;

    /** the type of the tunneled, passenger packet */
    PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT tunnelStartPassengerType;

    /** minimal hop limit */
    GT_U16 ttlHopLimitThreshold;

    /** @brief if GT_TRUE then if the packet's vlan equals the mll
     *  entry vlan id then skip this entry.
     */
    GT_BOOL excludeSrcVlan;

    /** @brief if GT_TRUE then there are more mll entries in this
     *  list if GT_FALSE this is the end.
     */
    GT_BOOL last;

} PRV_TGF_IP_MLL_STC;

/**
* @struct PRV_TGF_IP_MLL_PAIR_STC
 *
 * @brief Representation of the IP muticast Link List pair in HW
*/
typedef struct{

    /** the first Mll of the pair */
    PRV_TGF_IP_MLL_STC firstMllNode;

    /** the second Mll of the pair */
    PRV_TGF_IP_MLL_STC secondMllNode;

    /** pointer to the next MLL entry */
    GT_U16 nextPointer;

} PRV_TGF_IP_MLL_PAIR_STC;

/**
* @enum PRV_TGF_IP_PORT_TRUNK_CNT_MODE_ENT
 *
 * @brief the counter Set Port/Trunk mode
*/
typedef enum{

    /** @brief This counter-set counts all
     *  packets regardless of their
     *  In / Out port or Trunk.
     */
    PRV_TGF_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E = 0,

    /** @brief This counter-set counts packets Received /
     *  Transmitted via binded Port + Dev.
     */
    PRV_TGF_IP_PORT_CNT_MODE_E                 = 1,

    /** @brief This counter-set counts packets Received /
     *  Transmitted via binded Trunk.
     */
    PRV_TGF_IP_TRUNK_CNT_MODE_E                = 2

} PRV_TGF_IP_PORT_TRUNK_CNT_MODE_ENT;

/**
* @enum PRV_TGF_IP_VLAN_CNT_MODE_ENT
 *
 * @brief the counter Set vlan mode
*/
typedef enum{

    /** @brief This counter-set counts all packets
     *  regardless of their In / Out vlan.
     */
    PRV_TGF_IP_DISREGARD_VLAN_CNT_MODE_E = 0,

    /** @brief This counter-set counts packets Received
     *  / Transmitted via binded vlan.
     */
    PRV_TGF_IP_USE_VLAN_CNT_MODE_E       = 1

} PRV_TGF_IP_VLAN_CNT_MODE_ENT;

/*
 * Typedef: struct PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC
 *
 * Description: IPvX Counter Set interface mode configuration
 *
 * Fields:
 *      portTrunkCntMode - the counter Set Port/Trunk mode
 *      ipMode           - the cnt set IP interface mode , which protocol stack
 *                         ipv4 ,ipv6 or both (which is actully disregarding the
 *                         ip protocol)
 *      vlanMode         - the counter Set vlan mode.
 *      devNum           - the binded devNum , relevant only if
 *                         portTrunkCntMode = PRV_TGF_IP_PORT_CNT_MODE_E
 *      port             - the binded por , relevant only if
 *                         portTrunkCntMode = PRV_TGF_IP_PORT_CNT_MODE_E
 *      trunk            - the binded trunk , relevant only if
 *                         portTrunkCntMode = PRV_TGF_IP_TRUNK_CNT_MODE_E
 *      portTrunk        - the above port/trunk
 *      vlanId           - the binded vlan , relevant only if
 *                         vlanMode = PRV_TGF_IP_USE_VLAN_CNT_MODE_E
 */
typedef struct PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STCT
{
    PRV_TGF_IP_PORT_TRUNK_CNT_MODE_ENT portTrunkCntMode;
    CPSS_IP_PROTOCOL_STACK_ENT           ipMode;
    PRV_TGF_IP_VLAN_CNT_MODE_ENT       vlanMode;
    GT_U8                                devNum;
    union
    {
        GT_U32                            port;
        GT_TRUNK_ID                      trunk;
    }portTrunk;
    GT_U16                               vlanId;
}PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC;

/*
 * typedef: struct PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC
 *
 * Description:
 *      This struct hold the capacity configuration required from the Tcam LPM
 *      manager. According to these requirements the TCAM entries will be
 *      fragmented.
 *
 * Fields:
 *  numOfIpv4Prefixes           - The total number of Ipv4 Uc prefixes and
 *                                ipv4 MC group prefixes required.
 *  numOfIpv4McSourcePrefixes   - The number of Ipv4 MC source prefixes required.
 *  numOfIpv6Prefixes           - The total number of Ipv6 Uc prefixes,
 *                                ipv6 MC group prefixes and ipv6 MC sources
 *                                prefixes required.
 */
typedef struct PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STCT
{
    GT_U32  numOfIpv4Prefixes;
    GT_U32  numOfIpv4McSourcePrefixes;
    GT_U32  numOfIpv6Prefixes;
}PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC;


/**
* @struct PRV_TGF_IP_ROUTE_CNFG_STC
 *
 * @brief This supplement structure is used to provide standard IP Roting configuration.
*/
typedef struct{

    /** lpm DB Id. */
    GT_U32 lpmDbId;

    /** device number. */
    GT_U8 devNum;

    /** vlan number to send traffic to. */
    GT_U16 sendVlanId;

    /** nextHop vlanId to receive traffic from. */
    GT_U16 nextHopVlanId;

    /** port number to send traffic to. */
    GT_U8 sendPortNum;

    /** nextHop port number to receive traffic from. */
    GT_U8 nextHopPortNum;

    /** @brief GT_TRUE: ARP MAC address entry is set.
     *  GT_FALSE: ARP MAC address entry is not set.
     */
    GT_BOOL isArpNeeded;

    /** router ARP index. */
    GT_U32 routerArpIndex;

    /** route entry base index. */
    GT_U32 routeEntryBaseIndex;

    /** @brief GT_TRUE: routed packet would be tunneled.
     *  GT_FALSE: routed packet wouldn't be tunneled.
     */
    GT_BOOL isTunnelStart;

    /** tunnel start index. */
    GT_U32 tunnelStartIndex;

    /** ARP MAC DA. */
    TGF_MAC_ADDR prvTgfArpMac;

    /** The counter set this route entry is linked to. */
    CPSS_IP_CNT_SET_ENT countSet;

    /** ipv4 address. */
    TGF_IPV4_ADDR dstIpAddr;

} PRV_TGF_IP_ROUTE_CNFG_STC;

/*
 * typedef: struct PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC
 *
 * Description:
 *      This struct hold the indexes available for the Tcam LPM manager.
 *      it defines the From-To indexes in the TCAM the lpm manager can use.
 *
 * Fields:
 *  firstIndex - this is the first index availble (from)
 *  lastIndex  - this is the last index availble (to - including)
 *
 */
typedef struct PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STCT
{
    GT_U32  firstIndex;
    GT_U32  lastIndex;
}PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC;


/*
 * Typedef: struct PRV_TGF_IP_ECMP_ENTRY_STC
 *
 * Description: ECMP table entry (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.)
 *
 * Fields:
 *      randomEnable           - Whether to do random selection of next hop.
 *      numOfPaths             - The number of ECMP or QoS paths.
 *                               (APPLICABLE RANGES: 1..4096)
 *      routeEntryBaseIndex    - The base address of the route entries block.
 *                               (APPLICABLE RANGES: 0..24575)
 *      multiPathMode          - multipath mode. Applicable values:
 *                                PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E,
 *                                PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E.
 *                                (APPLICABLE DEVICES: Falcon)

 */
typedef struct PRV_TGF_IP_ECMP_ENTRY_STCT
{
    GT_BOOL                              randomEnable;
    GT_U32                               numOfPaths;
    GT_U32                               routeEntryBaseIndex;
    PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT    multiPathMode;
} PRV_TGF_IP_ECMP_ENTRY_STC;

/*
 * Typedef: struct PRV_TGF_LPM_LEAF_ENTRY_STC
 *
 * Description: LPM leaf entry used for policy based routing
 *
 * Fields:
 *      entryType                - Route entry type
 *                                 (APPLICABLE VALUES:
 *                                  PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
 *                                  PRV_TGF_LPM_ECMP_ENTRY_PTR_TYPE_E,
 *                                  PRV_TGF_LPM_QOS_ENTRY_PTR_TYPE_E)
 *      index                    - If entryType is
 *                                 PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E then
 *                                 this is the pointer to the route entry
 *                                 (APPLICABLE RANGES: 0..24575)
 *                                 Otherwise it is the pointer to ECMP/QoS entry
 *                                 (APPLICABLE RANGES: 0..12287)
 *      ucRPFCheckEnable         - Enable unicast RPF check for this entry
 *      sipSaCheckMismatchEnable - Enable unicast SIP MAC SA match check
 *      ipv6MCGroupScopeLevel    - the IPv6 Multicast group scope level
 *      priority                 - LPM priority
 *
 * Comments:
 *      index can point to either NH or ECMP entry. entryType determines the
 *      type of pointer: if entryType is PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E
 *      then it points to NH, else - to ECMP entry.
 */
typedef struct PRV_TGF_LPM_LEAF_ENTRY_STC
{
    PRV_TGF_LPM_NEXT_POINTER_TYPE_ENT           entryType;
    GT_U32                                      index;
    GT_BOOL                                     ucRPFCheckEnable;
    GT_BOOL                                     sipSaCheckMismatchEnable;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  ipv6MCGroupScopeLevel;
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT  priority;
} PRV_TGF_LPM_LEAF_ENTRY_STC;

/**
* @enum PRV_TGF_IP_URPF_MODE_ENT
 *
 * @brief Defines the uRPF check modes
*/
typedef enum{

    /** uRPF check is disabled */
    PRV_TGF_IP_URPF_DISABLE_MODE_E,

    /** @brief If ECMP uRPF is globally enabled,
     *  then uRPF check is done by comparing the
     *  packet VID to the VID in the additional
     *  route entry, otherwise it is done using
     *  the SIP-Next Hop Entry VID.
     *  Note: VLAN-based uRPF mode is not
     *  supported for ECMP blocks
     *  greater than 8 entries
     */
    PRV_TGF_IP_URPF_VLAN_MODE_E,

    /** @brief uRPF check is done by comparing the
     *  packet source (device,port)/Trunk to
     *  the SIP-Next Hop Entry (device,port)/Trunk.
     *  Note: Port-based uRPF mode is not
     *  supported if the address is
     *  associated with an ECMP/QoS
     *  block of nexthop entries.
     */
    PRV_TGF_IP_URPF_PORT_TRUNK_MODE_E,

    /** @brief uRPF check is done by checking
     *  the SIPNext Hop Entry Route
     *  Command. uRPF check fails if Route
     *  command is not "Route" or
     *  "Route and Mirror".
     *  Note: Loose-based uRPF mode is not
     *  supported if the address is
     *  associated with an ECMP/QoS
     *  block of nexthop entries.
     */
    PRV_TGF_IP_URPF_LOOSE_MODE_E

} PRV_TGF_IP_URPF_MODE_ENT;

/**
* @enum PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT
 *
 * @brief This enum defines the loose mode type.
*/
typedef enum{

    /** @brief uRPF check fails if the SIP Next Hop <Packet Command> is HARD/SOFT DROP,
     *  TRAP, or DEFAULT_ROUTE_ENTRY (default)
     */
    PRV_TGF_URPF_LOOSE_MODE_TYPE_0_E = 0,

    /** @brief uRPF check fails if the SIP Next Hop <Packet Command> is HARD/SOFT DROP,
     *  or DEFAULT_ROUTE_ENTRY (default)
     */
    PRV_TGF_URPF_LOOSE_MODE_TYPE_1_E = 1

} PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT;

/**
* @enum PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_ENT
 *
 * @brief Enumeration of shaper granularity.
*/
typedef enum{

    /** Granularity is 64 core clock cycles. */
    PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E,

    /** Granularity is 1024 core clock cycles. */
    PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E

} PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_ENT;

/**
* @struct PRV_TGF_IP_NAT44_ENTRY_STC
 *
 * @brief Struct for NAT44 entry.
*/
typedef struct{

    /** The MAC DA of the passenger packet */
    GT_ETHERADDR macDa;

    /** @brief Selector for modifying the packet Destination IP address
     *  GT_FALSE: Do not modify the Destination IP address of the packet
     *  GT_TRUE: Modify the Destination IP address of the packet
     */
    GT_BOOL modifyDip;

    /** The new Destination IP address of the packet */
    GT_IPADDR newDip;

    /** @brief Selector for modifying the packet Source IP address
     *  GT_FALSE: Do not modify the Source IP address of the packet
     *  GT_TRUE: Modify the Source IP address of the packet
     */
    GT_BOOL modifySip;

    /** The new Source IP address of the packet */
    GT_IPADDR newSip;

    /** @brief Selector for modifying the packet TCP / UDP Destination port
     *  GT_FALSE: Do not modify the TCP / UDP Destination port of the packet
     *  GT_TRUE: Modify the TCP / UDP Destination port of the packet
     */
    GT_BOOL modifyTcpUdpDstPort;

    /** @brief The New TCP / UDP Destination Port of the packet
     *  (APPLICABLE RANGES: 0..65535)
     */
    GT_U32 newTcpUdpDstPort;

    /** @brief Selector for modifying the packet TCP / UDP Source port
     *  GT_FALSE: Do not modify the TCP / UDP Source port of the packet
     *  GT_TRUE: Modify the TCP / UDP Source port of the packet
     */
    GT_BOOL modifyTcpUdpSrcPort;

    /** @brief The New TCP / UDP Source Port of the packet
     *  (APPLICABLE RANGES: 0..65535)
     */
    GT_U32 newTcpUdpSrcPort;

} PRV_TGF_IP_NAT44_ENTRY_STC;

/**
* @enum PRV_TGF_IP_NAT66_MODIFY_COMMAND_ENT
 *
 * @brief Enumeration of NAT Modify Command.
*/
typedef enum{

    /** Replace the SIP prefix with the NAT<Address>. */
    PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E,

    /** Replace the DIP prefix with the NAT<Address>. */
    PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E,

    /** Replace the SIP address with NAT<Address>; */
    PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E,

    /** Replace the DIP address with NAT<Address>; */
    PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E

} PRV_TGF_IP_NAT66_MODIFY_COMMAND_ENT;

/**
* @struct PRV_TGF_IP_NAT66_ENTRY_STC
 *
 * @brief Struct for NAT66 entry.
*/
typedef struct{

    /** The MAC DA of the passenger packet */
    GT_ETHERADDR macDa;

    /** @brief Translation command
     *  if command is PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E, PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E,
     *  The prefix size is taken from the <Prefix size> field in this entry
     *  if command is PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E, PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E,
     *  The address is taken from the <address> field in this entry
     */
    PRV_TGF_IP_NAT66_MODIFY_COMMAND_ENT modifyCommand;

    /** @brief New SIP/DIP address used to replace a packet's SIP/DIP according to modifyCommand.
     *  if command is PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E, PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E,
     *  then this is the new SIP or DIP.
     *  if command is PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E, PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E,
     *  then replace the <Prefix size> MSB of the SIP or DIP with the prefix of this entry (the prefix is aligned to the left)
     *  This means that the MSBs of the NAT Address will be assigned as the new packet Addr (SIP/DIP).
     */
    GT_IPV6ADDR address;

    /** @brief Address prefix size for NAT modifyCommand == Modify SIP prefix / Modify DIP prefix (according to RFC6296).
     *  0   - Prefix size is 64bit
     *  1-63 - Prefix size is according to NAT<Prefix Size>
     */
    GT_U32 prefixSize;

} PRV_TGF_IP_NAT66_ENTRY_STC;

/**
* @enum PRV_TGF_IP_NAT_TYPE_ENT
 *
 * @brief Enumeration of NAT types.
*/
typedef enum{

    /** NAT44: translation of ipv4 address to ipv4 address */
    PRV_TGF_IP_NAT_TYPE_NAT44_E,

    /** NAT66: translation of ipv6 address to ipv6 address */
    PRV_TGF_IP_NAT_TYPE_NAT66_E

} PRV_TGF_IP_NAT_TYPE_ENT;

/**
* @union PRV_TGF_IP_NAT_ENTRY_UNT
 *
 * @brief Union for NAT entry
 *
*/

typedef union{
    /** entry for NAT44 */
    PRV_TGF_IP_NAT44_ENTRY_STC nat44Entry;

    PRV_TGF_IP_NAT66_ENTRY_STC nat66Entry;

} PRV_TGF_IP_NAT_ENTRY_UNT;


/**
* @enum PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT
 *
 * @brief Defines the method of LPM blocks allocation
*/
typedef enum{

    /** @brief the blocks are allocated dynamically, memory blocks are never shared
     *  among different octets of the same protocol
     */
    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E  = 0,

    /** @brief the blocks are allocated dynamically, memory blocks are shared among
     *  different octets of the same protocol in case of missing free block.
     *  This can cause violation of full wire-speed.
     */
    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E     = 1,

    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_LAST_E

} PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT;


/**
* @enum PRV_TGF_LPM_RAM_MEM_MODE_ENT
 *
 * @brief Defines the mode of LPM MEM module
*/
typedef enum{

    /** @brief each RAM pool will be fully utulized
     *  This can cause violation of full wire-speed.
     */
    PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,

    /** @brief each RAM pool will be half utulized in order to support full wirespeed.
     *  The second half of each RAM pool will be identical to first.
     */
    PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E,

    PRV_TGF_LPM_RAM_MEM_MODE_LAST_E

} PRV_TGF_LPM_RAM_MEM_MODE_ENT;


/**
* @enum PRV_TGF_CFG_SHARED_TABLE_MODE_ENT
 *
 * @brief shared tables configuration modes for following clients:
 * L3 (LPM), L2 (FDB), EM (Exact Match)
 * (APPLICABLE DEVICES : FALCON)
*/
typedef enum{

    /** @brief This mode :
     * 1. Maximum L3 table size (LPM)
     * 2. Minimum L2 table size (FDB)
     * 3. No use of EM (Exact Match)
     */
    PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E,
    /** @brief This mode :
     * 1. Minimum L3 table size (LPM)
     * 2. Maximum L2 table size (FDB)
     * 3. No use of EM (Exact Match)
     */
    PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E,
    /** @brief This mode :
     * 1. Medium L3 table size (LPM)
     * 2. Medium L2 table size (FDB)
     * 3. Minimum EM table size Exact Match)
     */
    PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E,
    /** @brief This mode :
     * 1. Medium L3 table size (LPM)
     * 2. Minimum L2 table size (FDB)
     * 3. Maximum EM table size (Exact Match)
     */
    PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E

} PRV_TGF_CFG_SHARED_TABLE_MODE_ENT;



/*
 * Typedef: struct CPSS_DXCH_LPM_RAM_CONFIG_STCT
 *
 * Description: Memory configurations for RAM based LPM shadow
 *
 * Fields:
 *      numOfBlocks             - the number of RAM blocks that LPM uses.
 *                                (APPLICABLE VALUES: 1..20)
 *      blocksSizeArray         - array that holds the sizes of the RAM blocks in bytes
 *      blocksAllocationMethod  - the method of blocks allocation
 *      lpmMemMode              - the mode of LPM MEM.(APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
 *      sharedMemCnfg           - shared memory configuration mode. (APPLICABLE DEVICES: Falcon)
 *      maxNumOfPbrEntries      - max number of LPM leafs that can be allocated in the RAM for policy
 *                                based routing (APPLICABLE DEVICES: Falcon)
 *
 * Comments:
 *      None
 *
 */
typedef struct PRV_TGF_LPM_RAM_CONFIG_STCT
{
    GT_U32                                          numOfBlocks;
    GT_U32                                          blocksSizeArray[CPSS_DXCH_SIP6_LPM_RAM_NUM_OF_MEMORIES_CNS];
    PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT    blocksAllocationMethod;
    PRV_TGF_LPM_RAM_MEM_MODE_ENT                    lpmMemMode;
    PRV_TGF_CFG_SHARED_TABLE_MODE_ENT               sharedMemCnfg;
    GT_U32                                          maxNumOfPbrEntries;
} PRV_TGF_LPM_RAM_CONFIG_STC;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfIpRoutingModeGet function
* @endinternal
*
* @brief   Return the current routing mode
*
* @param[out] routerModePtr            - the current routing mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpRoutingModeGet
(
    OUT PRV_TGF_IP_ROUTING_MODE_ENT *routerModePtr
);

/**
* @internal prvTgfIpUcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes an array of uc route entries to hw
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpUcRouteEntriesWrite
(
    IN GT_U32                         baseRouteEntryIndex,
    IN PRV_TGF_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                         numOfRouteEntries
);

/**
* @internal prvTgfIpUcRouteEntriesRead function
* @endinternal
*
* @brief   Reads an array of uc route entries from the hw
*
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to start reading
* @param[in,out] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array
* @param[in,out] routeEntriesArray        - the uc route entries array read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpUcRouteEntriesRead
(
    IN    GT_U8                          devNum,
    IN    GT_U32                         baseRouteEntryIndex,
    INOUT PRV_TGF_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN    GT_U32                         numOfRouteEntries
);

/**
* @internal prvTgfIpRouterArpAddrWrite function
* @endinternal
*
* @brief   Write a ARP MAC address to the router ARP / Tunnel start Table
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type
*/
GT_STATUS prvTgfIpRouterArpAddrWrite
(
    IN GT_U32                         routerArpIndex,
    IN GT_ETHERADDR                  *arpMacAddrPtr
);

/**
* @internal prvTgfIpRouterArpAddrRead function
* @endinternal
*
* @brief   Read a ARP MAC address from the router ARP / Tunnel start Table
*
* @param[in] devNum                   - the device number
* @param[in] routerArpIndex           - The Arp Address index
*
* @param[out] arpMacAddrPtr            - the ARP MAC address to read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type
*/
GT_STATUS prvTgfIpRouterArpAddrRead
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        routerArpIndex,
    OUT GT_ETHERADDR                 *arpMacAddrPtr
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv4 prefix
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                vrId,
    IN GT_IPADDR                             ipAddr,
    IN GT_U32                                prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    IN GT_BOOL                               override,
    IN GT_BOOL                               defragmentationEnable
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv4 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         vrId,
    IN GT_IPADDR                      ipAddr,
    IN GT_U32                         prefixLen
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv4 Routing table and stays with the default prefix
*         only for a specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv6 prefix
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - If prefix length is too big.
* @retval GT_ERROR                 - If the vrId was not created yet.
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate TCAM memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                vrId,
    IN GT_IPV6ADDR                           ipAddr,
    IN GT_U32                                prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    IN GT_BOOL                               override,
    IN GT_BOOL                               defragmentationEnable
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv6 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         vrId,
    IN GT_IPV6ADDR                    ipAddr,
    IN GT_U32                         prefixLen
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv6 Routing table and stays with the default prefix
*         only for a specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
);

/**
* @internal prvTgfCountersIpSet function
* @endinternal
*
* @brief   Set route entry mode and reset IP couters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvTgfCountersIpSet
(
    IN GT_U32                          portNum,
    IN GT_U32                         counterIndex
);

/**
* @internal prvTgfCountersIpVlanModeSet function
* @endinternal
*
* @brief   Set route entry VLAN interface mode and reset IP couters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvTgfCountersIpVlanModeSet
(
    IN GT_U32                         vlanId,
    IN GT_U32                         counterIndex
);

/**
* @internal prvTgfCountersIpGet function
* @endinternal
*
* @brief   This function gets the values of the various counters in the IP
*         Router Management Counter-Set.
* @param[in] devNum                   - device number
* @param[in] counterIndex             - counter index
* @param[in] enablePrint              - Enable/Disable output log
*
* @param[out] ipCountersPtr            - (pointer to) IP counters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfCountersIpGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterIndex,
    IN  GT_BOOL                       enablePrint,
    OUT PRV_TGF_IP_COUNTER_SET_STC   *ipCountersPtr
);

/**
* @internal prvTgfIpPortRoutingEnable function
* @endinternal
*
* @brief   Enable multicast/unicast IPv4/v6 routing on a port
*
* @param[in] portIndex                - index of port to enable
* @param[in] ucMcEnable               - routing type to enable Unicast/Multicast
* @param[in] protocol                 - what type of traffic to enable ipv4 or ipv6 or both
* @param[in] enable                   -  IP routing for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpPortRoutingEnable
(
    IN GT_U8                          portIndex,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfIpPortRoutingEnableGet function
* @endinternal
*
* @brief   Get status of multicast/unicast IPv4/v6 routing on a port.
*
* @param[out] enablePtr                - (pointer to)enable IP routing for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note CPSS_IP_PROTOCOL_IPV4V6_E is not supported in this get API.
*       Can not get both values for ipv4 and ipv6 in the same get.
*
*/
GT_STATUS prvTgfIpPortRoutingEnableGet
(
    IN GT_U8                          portIndex,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal prvTgfIpVlanRoutingEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 multicast/unicast Routing on Vlan
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfIpVlanRoutingEnable
(
    IN GT_U16                         vlanId,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfInlifEntrySet function
* @endinternal
*
* @brief   Set Inlif Entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfInlifEntrySet
(
    IN PRV_TGF_INLIF_TYPE_ENT         inlifType,
    IN GT_U32                         inlifIndex,
    IN PRV_TGF_INLIF_ENTRY_STC       *inlifEntryPtr

);

/**
* @internal prvTgfInlifEntryGet function
* @endinternal
*
* @brief   Get Inlif Entry
*
* @param[in] inlifType                - inlif type
* @param[in] inlifIndex               - inlif index
*
* @param[out] inlifEntryPtr            - (pointer to) the Inlif Fields
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfInlifEntryGet
(
    IN PRV_TGF_INLIF_TYPE_ENT         inlifType,
    IN GT_U32                         inlifIndex,
    OUT PRV_TGF_INLIF_ENTRY_STC       *inlifEntryPtr

);

/**
* @internal prvTgfIpLpmVirtualRouterAdd function
* @endinternal
*
* @brief   This function adds a virtual router in system for specific LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterAdd
(
    IN GT_U32                                lpmDbId,
    IN GT_U32                                vrId,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defFcoeNextHopInfoPtr
);

/**
* @internal prvTgfIpLpmVirtualRouterAddDefault function
* @endinternal
*
* @brief   This function adds the default virtual router in the default LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterAddDefault
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmVirtualRouterSharedAdd function
* @endinternal
*
* @brief   This function adds shared virtual router in system for specific LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterSharedAdd
(
    IN GT_U32                                lpmDbId,
    IN GT_U32                                vrId,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defFcoeNextHopInfoPtr
);

/**
* @internal prvTgfIpLpmVirtualRouterDel function
* @endinternal
*
* @brief   This function removes a virtual router in system for a specific LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
*/
GT_STATUS prvTgfIpLpmVirtualRouterDel
(
    IN GT_U32                         lpmDbId,
    IN GT_U32                         vrId
);

/**
* @internal prvTgfIpLpmVirtualRouterDelDefault function
* @endinternal
*
* @brief   This function deletes the default virtual router in the default LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
*/
GT_STATUS prvTgfIpLpmVirtualRouterDelDefault
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmVirtualRouterGet function
* @endinternal
*
* @brief   This function gets the virtual router in system for specific LPM DB
*
* @param[out] isSupportIpv4Uc          - (pointer to) whether VR support UC Ipv4
* @param[out] defIpv4UcNextHopInfoPtr  - (pointer to) ipv4 uc next hop info
* @param[out] isSupportIpv6Uc          - (pointer to) whether VR support UC Ipv6
* @param[out] defIpv6UcNextHopInfoPtr  - (pointer to) ipv6 uc next hop info
* @param[out] isSupportIpv4Mc          - (pointer to) whether VR support MC Ipv4
* @param[out] defIpv4McRouteLttEntryPtr - (pointer to) ipv4 mc LTT entry info
* @param[out] isSupportIpv6Mc          - (pointer to) whether VR support MC Ipv6
* @param[out] defIpv6McRouteLttEntryPtr - (pointer to) ipv6 mc LTT entry info
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterGet
(
    IN  GT_U32                                lpmDbId,
    IN  GT_U32                                vrId,
    OUT GT_BOOL                              *isSupportIpv4Uc,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    OUT GT_BOOL                              *isSupportIpv6Uc,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    OUT GT_BOOL                              *isSupportIpv4Mc,
    OUT PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    OUT GT_BOOL                              *isSupportIpv6Mc,
    OUT PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr
);

/**
* @internal prvTgfIpLttWrite function
* @endinternal
*
* @brief   Writes a LookUp Translation Table Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpLttWrite
(
    IN GT_U32                         lttTtiRow,
    IN GT_U32                         lttTtiColumn,
    IN PRV_TGF_IP_LTT_ENTRY_STC      *lttEntryPtr
);

/**
* @internal prvTgfIpv4PrefixSet function
* @endinternal
*
* @brief   Sets an ipv4 UC or MC prefix to the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv4PrefixSet
(
    IN GT_U32                         routerTtiTcamRow,
    IN GT_U32                         routerTtiTcamColumn,
    IN PRV_TGF_IPV4_PREFIX_STC       *prefixPtr,
    IN PRV_TGF_IPV4_PREFIX_STC       *maskPtr
);

/**
* @internal prvTgfIpv6PrefixSet function
* @endinternal
*
* @brief   Set an ipv6 UC or MC prefix to the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv6PrefixSet
(
    IN GT_U32                  routerTtiTcamRow,
    IN PRV_TGF_IPV6_PREFIX_STC *prefixPtr,
    IN PRV_TGF_IPV6_PREFIX_STC *maskPtr
);

/**
* @internal prvTgfIpv4PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv4 UC or MC prefix in the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv4PrefixInvalidate
(
    IN GT_U32                         routerTtiTcamRow,
    IN GT_U32                         routerTtiTcamColumn
);

/**
* @internal prvTgfIpv6PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv6 UC or MC prefix in the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv6PrefixInvalidate
(
    IN GT_U32 routerTtiTcamRow
);

/**
* @internal prvTgfIpMcRouteEntriesWrite function
* @endinternal
*
* @brief   Write an array of MC route entries to hw
*
* @param[in] ucMcBaseIndex            - base Index in the Route entries table
* @param[in] numOfEntries             - number of route entries to write
* @param[in] protocol                 - ip  type
* @param[in] entriesArrayPtr          - MC route entry array to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfIpMcRouteEntriesWrite
(
    IN GT_U32                         ucMcBaseIndex,
    IN GT_U32                         numOfEntries,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN PRV_TGF_IP_MC_ROUTE_ENTRY_STC *entriesArrayPtr
);

/**
* @internal prvTgfIpMllPairWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) pair entry to hw
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllPairWriteForm.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpMllPairWrite
(
    IN GT_U32                                   mllPairEntryIndex,
    IN PRV_TGF_PAIR_READ_WRITE_FORM_ENT         mllPairWriteForm,
    IN PRV_TGF_IP_MLL_PAIR_STC                  *mllPairEntryPtr
);

/**
* @internal prvTgfIpLpmDBDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing LPM DB
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] devList[]                - the array of device ids to add to the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfIpLpmDBDevListAdd
(
    IN GT_U32                         lpmDBId,
    IN GT_U8                          devList[],
    IN GT_U32                         numOfDevs
);

/**
* @internal prvTgfIpLpmDBDevListAddDefault function
* @endinternal
*
* @brief   This function adds a default device to an existing LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfIpLpmDBDevListAddDefault
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmIpv4McEntryAdd function
* @endinternal
*
* @brief   Add IP MC route for a particular/all source and group address
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual private network identifier
* @param[in] ipGroup                  - IP MC group address
* @param[in] ipGroupPrefixLen         - number of bits that are actual valid in ipGroup
* @param[in] ipSrc                    - root address for source base multi tree protocol
* @param[in] ipSrcPrefixLen           - number of bits that are actual valid in ipSrc
* @param[in] mcRouteLttEntryPtr       - LTT entry pointing to the MC route entry
* @param[in] override                 - whether to  an mc Route pointer
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if one of prefixes' lengths is too big
* @retval GT_ERROR                 - if the virtual router does not exist
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpLpmIpv4McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPADDR                    ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
);

/**
* @internal prvTgfIpLpmIpv4McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmIpv4McEntryDel
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPADDR                    ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixSearch function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the
*         information associated with it.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - if found, this is the route entry info.
*                                      accosiated with this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixSearch
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPADDR                            ipAddr,
    IN  GT_U32                               prefixLen,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixGet function
* @endinternal
*
* @brief   This function gets a given ip address, find LPM match in the trie and
*         returns the prefix length and pointer to the next hop information bound
*         to the longest prefix match.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - The prefix length of the found entry.
* @param[out] nextHopInfoPtr           - if found, this is the route entry info.
*                                      accosiated with this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this UC prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this UC prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixGet
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPADDR                            ipAddr,
    OUT GT_U32                               *prefixLenPtr,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
);

/**
* @internal prvTgfIpLpmIpv4McEntrySearch function
* @endinternal
*
* @brief   This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc,ipGroup TCAM indexes
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - The ip Group prefix len.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - the LTT entry pointer pointing to the MC route
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamGroupColumnIndexPtr  - pointer to TCAM group column  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
* @param[out] tcamSrcColumnIndexPtr    - pointer to TCAM source column  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
*/
GT_STATUS prvTgfIpLpmIpv4McEntrySearch
(
    IN  GT_U32                      lpmDBId,
    IN  GT_U32                      vrId,
    IN  GT_IPADDR                   ipGroup,
    IN  GT_U32                      ipGroupPrefixLen,
    IN  GT_IPADDR                   ipSrc,
    IN  GT_U32                      ipSrcPrefixLen,
    OUT PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    OUT GT_U32                      *tcamGroupRowIndexPtr,
    OUT GT_U32                      *tcamGroupColumnIndexPtr,
    OUT GT_U32                      *tcamSrcRowIndexPtr,
    OUT GT_U32                      *tcamSrcColumnIndexPtr
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixGet function
* @endinternal
*
* @brief   This function gets a given ip address, find LPM match in the trie and
*         returns the prefix length and pointer to the next hop information bound
*         to the longest prefix match.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - The prefix length of the found entry.
* @param[out] nextHopInfoPtr           - If  found, the route entry info accosiated with
*                                      this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixGet
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPV6ADDR                          ipAddr,
    OUT GT_U32                               *prefixLenPtr,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixSearch function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the next
*         hop pointer associated with it and TCAM prefix index.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If  found, the route entry info accosiated with
*                                      this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixSearch
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPV6ADDR                          ipAddr,
    IN  GT_U32                               prefixLen,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
);

/**
* @internal prvTgfIpLpmIpv6McEntrySearch function
* @endinternal
*
* @brief   This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc,ipGroup TCAM indexes
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - the LTT entry pointer pointing to the MC route
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
*/
GT_STATUS prvTgfIpLpmIpv6McEntrySearch
(
    IN  GT_U32                   lpmDBId,
    IN  GT_U32                   vrId,
    IN  GT_IPV6ADDR              ipGroup,
    IN  GT_U32                   ipGroupPrefixLen,
    IN  GT_IPV6ADDR              ipSrc,
    IN  GT_U32                   ipSrcPrefixLen,
    OUT PRV_TGF_IP_LTT_ENTRY_STC *mcRouteLttEntryPtr,
    OUT GT_U32                   *tcamGroupRowIndexPtr,
    OUT GT_U32                   *tcamSrcRowIndexPtr
);

/**
* @internal prvTgfIpLpmIpv6McEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from a particular
*         source and addressed to a particular IP multicast group address for a
*         specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteLttEntryPtr       - the LTT entry pointing to the MC route entry
*                                      associated with this MC route.
* @param[in] override                 - weather to  the mcRoutePointerPtr for the
*                                      given prefix
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big.
* @retval GT_ERROR                 - if the virtual router does not exist.
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented .
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note to override the default mc route use ipGroup = ipGroupPrefixLen = 0.
*
*/
GT_STATUS prvTgfIpLpmIpv6McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPV6ADDR                  ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                  ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
);

/**
* @internal prvTgfIpLpmIpv6McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmIpv6McEntryDel
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPV6ADDR                  ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                  ipSrc,
    IN GT_U32                       ipSrcPrefixLen
);

/**
* @internal prvTgfIpLpmIpv4McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv4McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
);

/**
* @internal prvTgfIpLpmIpv6McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv6McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
);

/**
* @internal prvTgfIpSetMllCntInterface function
* @endinternal
*
* @brief   Sets a mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpSetMllCntInterface
(
    IN GT_U32                                    mllCntSet,
    IN PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceCfgPtr
);

/**
* @internal prvTgfIpMllCntGet function
* @endinternal
*
* @brief   Get the mll counter.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
* @param[in] devNum                   - the device number.
* @param[in] mllCntSet                - the mll counter set out of the 2
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of routed IP Multicast packets Duplicated by the
*                                      MLL Engine and transmitted via this interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
);

/**
* @internal prvTgfIpMllCntSet function
* @endinternal
*
* @brief   set an mll counter.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - the mll counter set out of the 2
* @param[in] mllOutMCPkts             - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllCntSet
(
    IN GT_U8    devNum,
    IN GT_U32   mllCntSet,
    IN GT_U32   mllOutMCPkts
);

/**
* @internal prvTgfIpMllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the MLL priority queues.
*         A silent drop is a drop that is applied to a replica of the packet that
*         was previously replicated in the TTI.
* @param[in] devNum                   - the device number
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllSilentDropCntGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *silentDropPktsPtr
);

/**
* @internal prvTgfIpMtuProfileSet function
* @endinternal
*
* @brief   Sets the next hop interface MTU profile limit value.
*
* @param[in] devNum                   - the device number
* @param[in] mtu                      - the mtu profile index.  CHX_FAMILY (0..7)
*                                      EXMXPM_FAMILY (0..15)
* @param[in] mtu                      - the maximum transmission unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMtuProfileSet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    IN GT_U32 mtu
);

/**
* @internal prvTgfIpMtuProfileGet function
* @endinternal
*
* @brief   Gets the next hop interface MTU profile limit value.
*
* @param[in] devNum                   - device number
* @param[in] mtuProfileIndex          - the mtu profile index (0..15)
*
* @param[out] mtuPtr                   - points to the maximum transmission unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMtuProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mtuProfileIndex,
    OUT GT_U32  *mtuPtr
);

/**
* @internal prvTgfIpv6AddrPrefixScopeSet function
* @endinternal
*
* @brief   Defines a prefix of a scope type.
*
* @param[in] devNum                   - the device number
* @param[in] prefix                   - an IPv6 address prefix
* @param[in] prefixLen                - length of the prefix (APPLICABLE RANGES: 0..16)
* @param[in] addressScope             - type of the address scope spanned by the prefix
* @param[in] prefixScopeIndex         - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configures an entry in the prefix look up table
*
*/
GT_STATUS prvTgfIpv6AddrPrefixScopeSet
(
    IN  GT_U8                           devNum,
    IN  GT_IPV6ADDR                     prefix,
    IN  GT_U32                          prefixLen,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT      addressScope,
    IN  GT_U32                          prefixScopeIndex
);

/**
* @internal prvTgfIpv6AddrPrefixScopeGet function
* @endinternal
*
* @brief   Get a prefix of a scope type.
*
* @param[in] devNum                   - the device number
* @param[in] prefixScopeIndex         - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* @param[out] prefixPtr                - an IPv6 address prefix
* @param[out] prefixLenPtr             - length of the prefix
* @param[out] addressScopePtr          - type of the address scope spanned by the prefix
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvTgfIpv6AddrPrefixScopeGet
(
    IN  GT_U8                           devNum,
    OUT GT_IPV6ADDR                     *prefixPtr,
    OUT GT_U32                          *prefixLenPtr,
    OUT CPSS_IPV6_PREFIX_SCOPE_ENT      *addressScopePtr,
    IN  GT_U32                          prefixScopeIndex
);

/**
* @internal prvTgfIpExceptionCommandSet function
* @endinternal
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - Exception type
* @param[in] protocolStack            - whether to set for ipv4/v6 or both
* @param[in] packetType               - Packet type. Valid values:
*                                      CPSS_IP_UNICAST_E
*                                      CPSS_IP_MULTICAST_E
* @param[in] exceptionCmd             - Exception command
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpExceptionCommandSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT        exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_IP_UNICAST_MULTICAST_ENT        packetType,
    IN PRV_TGF_IP_EXC_CMD_ENT               exceptionCmd
);

/**
* @internal prvTgfIpExceptionCommandGet function
* @endinternal
*
* @brief   Get a specific exception command.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - Exception type
* @param[in] protocolStack            - IP protocol to set for
* @param[in] packetType               - Packet type. Valid values:
*                                      CPSS_IP_UNICAST_E
*                                      CPSS_IP_MULTICAST_E
*
* @param[out] exceptionCmdPtr          - points to exception command
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpExceptionCommandGet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT        exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_IP_UNICAST_MULTICAST_ENT        packetType,
    OUT PRV_TGF_IP_EXC_CMD_ENT              *exceptionCmdPtr
);

/**
* @internal prvTgfIpv6McScopeCommandSet function
* @endinternal
*
* @brief   sets the ipv6 Multicast scope commands.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
* @param[in] scopeCommand             - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*                                      CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_BRIDGE_E
* @param[in] mllSelectionRule         - rule for choosing MLL for IPv6 Multicast propogation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpv6McScopeCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN GT_BOOL                          borderCrossed,
    IN CPSS_PACKET_CMD_ENT              scopeCommand,
    IN CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule
);

/**
* @internal prvTgfIpv6McScopeCommandGet function
* @endinternal
*
* @brief   gets the ipv6 Multicast scope commands.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
*
* @param[out] scopeCommandPtr          - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*                                      CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_BRIDGE_E
* @param[out] mllSelectionRulePtr      - rule for choosing MLL for IPv6 Multicast propogation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvTgfIpv6McScopeCommandGet
(
    IN  GT_U8                            devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN  GT_BOOL                          borderCrossed,
    OUT CPSS_PACKET_CMD_ENT              *scopeCommandPtr,
    OUT CPSS_IPV6_MLL_SELECTION_RULE_ENT *mllSelectionRulePtr
);

/**
* @internal prvTgfIpRouterMacSaBaseSet function
* @endinternal
*
* @brief   Sets 40 MSBs of Router MAC SA Base address on specified device.
*
* @param[in] devNum                   - the device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
);

/**
* @internal prvTgfIpRouterMacSaBaseGet function
* @endinternal
*
* @brief   Gets 40 MSBs of Router MAC SA Base address on specified device.
*
* @param[in] devNum                   - the device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
);

/**
* @internal prvTgfIpRouterGlobalMacSaSet function
* @endinternal
*
* @brief   Sets full 48-bit Router MAC SA in Global MAC SA table.
*
* @param[in] devNum                   - the device number.
* @param[in] routerMacSaIndex         - The index into the global MAC SA table.
* @param[in] macSaAddrPtr             - The 48 bits MAC SA.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterGlobalMacSaSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerMacSaIndex,
    IN  GT_ETHERADDR                *macSaAddrPtr
);


/**
* @internal prvTgfIpRouterGlobalMacSaGet function
* @endinternal
*
* @brief   Gets full 48-bit Router MAC SA from Global MAC SA table.
*
* @param[in] devNum                   - the device number.
* @param[in] routerMacSaIndex         - The index into the global MAC SA table.
*
* @param[out] macSaAddrPtr             - The 48 bits MAC SA.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This API is applicable when per-egress-physical-port MAC SA assignment
*       mode is configured to global :CPSS_SA_LSB_FULL_48_BIT_GLOBAL by API
*       cpssDxChIpPortRouterMacSaLsbModeSet. The routerMacSaIndex is configured
*       by cpssDxChIpRouterGlobalMacSaIndexSet.
*
*/
GT_STATUS prvTgfIpRouterGlobalMacSaGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       routerMacSaIndex,
    OUT GT_ETHERADDR *macSaAddrPtr
);


/**
* @internal prvTgfIpRouterPortGlobalMacSaIndexSet function
* @endinternal
*
* @brief   Set router mac sa index refered to global MAC SA table.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] routerMacSaIndex         - global MAC SA table index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterPortGlobalMacSaIndexSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               routerMacSaIndex
);

/**
* @internal prvTgfIpRouterPortGlobalMacSaIndexGet function
* @endinternal
*
* @brief   Get router mac sa index refered to global MAC SA table.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] routerMacSaIndexPtr      - (pointer to) global MAC SA table index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterPortGlobalMacSaIndexGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *routerMacSaIndexPtr
);


/**
* @internal prvTgfIpRouterMacSaModifyEnable function
* @endinternal
*
* @brief   Per Egress port bit Enable Routed packets MAC SA Modification
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical or CPU port number.
* @param[in] enable                   - GT_FALSE: MAC SA Modification of routed packets is disabled
*                                      GT_TRUE: MAC SA Modification of routed packets is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaModifyEnable
(
    IN  GT_U8                      devNum,
    IN  GT_U32                      portNum,
    IN  GT_BOOL                    enable
);

/**
* @internal prvTgfIpRouterMacSaModifyEnableGet function
* @endinternal
*
* @brief   Per Egress port bit Get Routed packets MAC SA Modification State
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical or CPU port number.
*
* @param[out] enablePtr                - GT_FALSE: MAC SA Modification of routed packets is disabled
*                                      GT_TRUE: MAC SA Modification of routed packets is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaModifyEnableGet
(
   IN  GT_U8                       devNum,
   IN  GT_PHYSICAL_PORT_NUM        portNum,
   OUT GT_BOOL                     *enablePtr
);

/**
* @internal prvTgfIpPortRouterMacSaLsbModeSet function
* @endinternal
*
* @brief   Sets the mode, per port, in which the device sets the packet's MAC SA
*         least significant bytes.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
* @param[in] saLsbMode                - The MAC SA least-significant bit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device 5 most significant bytes are set by cpssDxChIpRouterMacSaBaseSet().
*       The least significant bytes are set by:
*       Port mode is set by cpssDxChIpRouterPortMacSaLsbSet().
*       Vlan mode is set by cpssDxChIpRouterVlanMacSaLsbSet().
*
*/
GT_STATUS prvTgfIpPortRouterMacSaLsbModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    IN  CPSS_MAC_SA_LSB_MODE_ENT    saLsbMode
);

/**
* @internal prvTgfIpPortRouterMacSaLsbModeGet function
* @endinternal
*
* @brief   Gets the mode, per port, in which the device sets the packet's MAC SA
*         least significant bytes.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
*
* @param[out] saLsbModePtr             - The MAC SA least-significant bit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device 40 most significant bytes are get by cpssDxChIpRouterMacSaBaseGet().
*       The least significant bytes are get by:
*       Port mode is get by cpssDxChIpRouterPortMacSaLsbGet().
*       Vlan mode is get by cpssDxChIpRouterVlanMacSaLsbGet().
*
*/
GT_STATUS prvTgfIpPortRouterMacSaLsbModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    OUT CPSS_MAC_SA_LSB_MODE_ENT    *saLsbModePtr
);

/**
* @internal prvTgfIpRouterPortMacSaLsbSet function
* @endinternal
*
* @brief   Sets the 8 LSB Router MAC SA for this EGGRESS PORT.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - Eggress Port number
* @param[in] saMac                    - The 8 bits SA mac value to be written to the SA bits of
*                                      routed packet if SA alteration mode is configured to
*                                      take LSB according to Eggress Port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterPortMacSaLsbSet
(
    IN GT_U8   devNum,
    IN GT_U32   portNum,
    IN GT_U8   saMac
);

/**
* @internal prvTgfIpRouterVlanMacSaLsbSet function
* @endinternal
*
* @brief   Sets the LSBs of Router MAC SA for this VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - the device number
* @param[in] vlan                     - VLAN Id
* @param[in] saMac                    - The Least Significant bits SA mac value to be written to the SA bits of
*                                      routed packet if SA alteration mode is configured to
*                                      take LSB according to VLAN.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterVlanMacSaLsbSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlan,
    IN GT_U32  saMac
);

/**
* @internal prvTgfIpRouterNextHopTableAgeBitsEntryRead function
* @endinternal
*
* @brief   read router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @param[out] activityBitPtr           - (pointer to) Age bit value of requested Next-hop entry.
*                                      field. Range 0..1.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvTgfIpRouterNextHopTableAgeBitsEntryRead
(
    GT_U32     *activityBitPtr
);

/**
* @internal prvTgfIpLpmDBCapacityGet function
* @endinternal
*
* @brief   This function gets the current LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[out] partitionEnablePtr       - GT_TRUE:  the TCAM is partitioned according
*                                      to the capacityCfgPtr, any unused TCAM entries
*                                      were allocated to IPv4 UC entries.
*                                      GT_FALSE: TCAM entries are allocated on demand
*                                      while entries are guaranteed as specified
*                                      in capacityCfgPtr.
* @param[out] tcamLpmManagerCapcityCfgPtr - the current capacity configuration. when
*                                      partitionEnable in cpssDxChIpLpmDBCreate
*                                      was set to GT_TRUE this means current
*                                      prefixes partition, when this was set to
*                                      GT_FALSE this means the current guaranteed
*                                      prefixes allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration.
*/
GT_STATUS prvTgfIpLpmDBCapacityGet
(
    IN  GT_U32                                     lpmDBId,
    OUT GT_BOOL                                    *partitionEnablePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
);

/**
* @internal prvTgfIpLpmDBCapacityUpdate function
* @endinternal
*
* @brief   This function updates the initial LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[in] tcamLpmManagerCapcityCfgPtr - the new capacity configuration. when
*                                      partitionEnable in cpssDxChIpLpmDBCreate
*                                      was set to GT_TRUE this means new prefixes
*                                      partition, when this was set to GT_FALSE
*                                      this means the new prefixes guaranteed
*                                      allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration
*
* @note This API is relevant only when using TCAM Manager. This API is used for
*       updating only the capacity configuration of the LPM. for updating the
*       lines reservation for the TCAM Manger use cpssDxChTcamManagerRangeUpdate.
*
*/
GT_STATUS prvTgfIpLpmDBCapacityUpdate
(
    IN GT_U32                                     lpmDBId,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
);

/**
* @internal prvTgfIpLpmDBConfigGet function
* @endinternal
*
* @brief   This function retrieves configuration of the LPM DB.
*
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] shadowTypePtr            - (pointer to) shadow type
* @param[out] protocolStackPtr         - (pointer to) protocol stack this LPM DB support
* @param[out] indexesRangePtr          - (pointer to) range of TCAM indexes availble for this LPM DB
* @param[out] partitionEnablePtr       - GT_TRUE:  TCAM is partitioned to different prefix types
*                                      GT_FALSE: TCAM entries are allocated on demand
* @param[out] tcamLpmManagerCapcityCfgPtr - (pointer to) capacity configuration
* @param[out] isTcamManagerInternalPtr - GT_TRUE:  TCAM manager is used and created
*                                      internally by the LPM DB
*                                      GT_FALSE: otherwise
* @param[out] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_PTR               - NULL pointer.
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmDBConfigGet
(
    IN  GT_U32                                         lpmDBId,
    OUT PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT               *shadowTypePtr,
    OUT CPSS_IP_PROTOCOL_STACK_ENT                    *protocolStackPtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    *indexesRangePtr,
    OUT GT_BOOL                                       *partitionEnablePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    *tcamLpmManagerCapcityCfgPtr,
    OUT GT_BOOL                                       *isTcamManagerInternalPtr,
    OUT GT_VOID                                       *tcamManagerHandlerPtr
);

/**
* @internal prvTgfIpLpmDBCreate function
* @endinternal
*
* @brief   This function creates an LPM DB for a shared LPM managment.
*
* @param[in] lpmDBId                  - LPM DB id.
* @param[in] shadowType               - type of shadow to maintain
* @param[in] protocolStack            - type of protocol stack this LPM DB support
* @param[in] indexesRangePtr          - (pointer to) range of TCAM indexes
* @param[in] partitionEnable          - GT_TRUE:  partition the TCAM range according to the
*                                      capacityCfgPtr, any unused TCAM entries will
*                                      be allocated to IPv4 UC entries
*                                      GT_FALSE: allocate TCAM entries on demand while
*                                      guarantee entries as specified in capacityCfgPtr
* @param[in] tcamLpmManagerCapcityCfgPtr - (pointer to) capacity configuration
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmDBCreate
(
    IN GT_U32                                      lpmDBId,
    IN PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN GT_BOOL                                     partitionEnable,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr,
    IN GT_VOID                                    *tcamManagerHandlerPtr
);

/**
* @internal prvTgfIpLpmRamDBCreate function
* @endinternal
*
* @brief   This function creates an RAM LPM DB for a shared LPM managment.
*
* @param[in] lpmDBId                  - LPM DB id.
* @param[in] protocolStack            - type of protocol stack this LPM DB support
* @param[in] ramDbCfgPtr              - (pointer to)the ram memory configuration for this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmRamDBCreate
(
    IN GT_U32                                      lpmDBId,
    IN CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack,
    PRV_TGF_LPM_RAM_CONFIG_STC                     *ramDbCfgPtr
);

/**
* @internal prvTgfIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculates the default RAM LPM DB configuration for a shared LPM managment.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - Null pointer
*/
GT_STATUS prvTgfIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                        devNum,
    OUT PRV_TGF_LPM_RAM_CONFIG_STC *ramDbCfgPtr
);

/**
* @internal prvTgfIpLpmRamDBCreateDefault function
* @endinternal
*
* @brief   This function creates a default RAM LPM DB for a shared LPM managment.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmRamDBCreateDefault
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmDBDelete function
* @endinternal
*
* @brief   This function deletes LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_STATE             - the LPM DB is not empty
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvTgfIpLpmDBDelete
(
    IN GT_U32           lpmDBId
);

/**
* @internal prvTgfIpLpmDBDeleteDefault function
* @endinternal
*
* @brief   This function deletes default LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_STATE             - the LPM DB is not empty
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvTgfIpLpmDBDeleteDefault
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmDBExtTcamManagerAdd function
* @endinternal
*
* @brief   This function add external TCAM manager into default LPM DB
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] tcamManagerRangePtr      - (pointer to) allocated TCAM range
*
* @param[out] tcamManagerHandlerPtrPtr - (pointer to) pointer of the created TCAM manager
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
* @retval GT_OUT_OF_CPU_MEM        - if no CPU memory for memory allocation
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmDBExtTcamManagerAdd
(
    IN  GT_U32                                lpmDBId,
    IN  PRV_TGF_TCAM_MANAGER_RANGE_STC       *tcamManagerRangePtr,
    OUT GT_VOID                             **tcamManagerHandlerPtrPtr
);

/**
* @internal prvTgfIpLpmDBExtTcamManagerRemove function
* @endinternal
*
* @brief   This function remove external TCAM manager from default LPM DB
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmDBExtTcamManagerRemove
(
    IN  GT_U32                                lpmDBId,
    IN  GT_VOID                              *tcamManagerHandlerPtr
);

/**
* @internal prvTgfIpLpmDBWithTcamCapacityUpdate function
* @endinternal
*
* @brief   This function updates LPM DB allocation and reserved TCAM lines.
*
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
* @param[in] lpmDBId                  - LPM DB id
* @param[in] indexesRangePtr          - (pointer to) range of TCAM indexes
* @param[in] tcamLpmManagerCapcityCfgPtr - (pointer to) new capacity configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NO_RESOURCE           - failed to allocate resources
*/
GT_STATUS prvTgfIpLpmDBWithTcamCapacityUpdate
(
    IN GT_VOID                                    *tcamManagerHandlerPtr,
    IN GT_U32                                      lpmDBId,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
);

/**
* @internal prvTgfIpValidityCheck function
* @endinternal
*
* @brief   Check Patricia trie validity for specific LPM DB Id and virtual router Id
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
*                                      entryType   - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
*/
GT_STATUS prvTgfIpValidityCheck
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType

);


/**
* @internal prvTgfIpValidityCheckEnable function
* @endinternal
*
* @brief   This function sets value of validity flag.
*
* @param[in] validityFlag             - flag whether to check (GT_TRUE) Patricia trie validity
*                                       None.
*/
GT_VOID prvTgfIpValidityCheckEnable
(
    IN GT_BOOL validityFlag
);

/**
* @internal prvTgfIpLpmValidityCheck function
* @endinternal
*
* @brief   Check IP LPM Shadow validity for specific LPM DB Id and virtual router Id
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
*/
GT_STATUS prvTgfIpLpmValidityCheck
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType,
    IN GT_BOOL                    returnOnFailure

);

/**
* @internal prvTgfIpLpmDbgIpv4UcPrefixAddManyRandom function
* @endinternal
*
* @brief   This function tries to add many random IPv4 Unicast prefixes and
*         returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] isWholeIpRandom          - GT_TRUE: all IP octets calculated by cpssOsRand
*                                      GT_FALSE: only 2 LSB octets calculated by cpssOsRand
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmDbgIpv4UcPrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeIpRandom,
    IN  GT_BOOL     defragmentationEnable,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal prvTgfIpLpmDbgPrefixLengthSet function
* @endinternal
*
* @brief   set prefix length used in addManyByOctet APIs
*
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] prefixLength             - prefix length
* @param[in] srcPrefixLength          - src prefix length
*/
GT_U32 prvTgfIpLpmDbgPrefixLengthSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType,
    IN GT_U32                     prefixLength,
    IN GT_U32                     srcPrefixLength
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of the prefixes
*       that was added by cpssDxChIpLpmIpv4UcPrefixAddManyByOctet.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential IPv4 Unicast prefixes (that were
*         added by cpssDxChIpLpmIpv4UcPrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to deleted
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of prefixes
*       that was added by cpssDxChIpLpmIpv6UcPrefixAddManyByOctet.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixBulkAdd function
* @endinternal
*
* @brief   Add bulk of IPv4 UC prefixes
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixBulkAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                *vrId,
    IN GT_IPADDR                             *ipAddr,
    IN GT_U32                                *prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    IN GT_BOOL                               *override,
    IN GT_U32                                sizeOfBulk
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixBulkDel function
* @endinternal
*
* @brief   Deletes an existing IPv4 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixBulkDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         *vrId,
    IN GT_IPADDR                      *ipAddr,
    IN GT_U32                         *prefixLen,
    IN GT_U32                         sizeOfBulk
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixBulkAdd function
* @endinternal
*
* @brief   Add bulk of IPv6 UC prefixes
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixBulkAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                *vrId,
    IN GT_IPV6ADDR                           *ipAddr,
    IN GT_U32                                *prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    IN GT_BOOL                               *override,
    IN GT_BOOL                               defragmentationEnable,
    IN GT_U32                                sizeOfBulk
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixBulkDel function
* @endinternal
*
* @brief   Deletes an existing IPv6 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixBulkDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         *vrId,
    IN GT_IPV6ADDR                    *ipAddr,
    IN GT_U32                         *prefixLen,
    IN GT_U32                         sizeOfBulk
);

/**
* @internal prvTgfIpv4PbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] pbrConfigPtr             - pbr configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfIpv4PbrConfigurationSet
(
    IN PRV_TGF_IP_ROUTE_CNFG_STC *pbrConfigPtr
);

/**
* @internal prvTgfIpv4LttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] lttConfigPtr             - ltt routing configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfIpv4LttRouteConfigurationSet
(
    IN PRV_TGF_IP_ROUTE_CNFG_STC *lttConfigPtr
);

/**
* @internal prvTgfIpHeaderErrorMaskSet function
* @endinternal
*
* @brief   Mask or unmask an IP header error.
*
* @param[in] devNum                   - the device number
* @param[in] ipHeaderErrorType        - the IP header error type
* @param[in] protocolStack            - whether to mask/unmask the error for IPv4 or IPv6
* @param[in] prefixType               - whether to mask/unmask the error for unicast or
*                                      multicast
* @param[in] mask                     - GT_TRUE:  the error
*                                      GT_FALSE: unmask the error
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
GT_STATUS prvTgfIpHeaderErrorMaskSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_HEADER_ERROR_ENT          ipHeaderErrorType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT           prefixType,
    IN GT_BOOL                              mask
);

/**
* @internal prvTgfIpHeaderErrorMaskGet function
* @endinternal
*
* @brief   Mask or unmask an IP header error.
*
* @param[in] devNum                   - the device number
* @param[in] ipHeaderErrorType        - the IP header error type
* @param[in] protocolStack            - whether to mask/unmask the error for IPv4 or IPv6
* @param[in] prefixType               - whether to mask/unmask the error for unicast or
*                                      multicast
*
* @param[out] maskPtr                  - GT_TRUE: mask the error
*                                      GT_FALSE: unmask the error
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
GT_STATUS prvTgfIpHeaderErrorMaskGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_IP_HEADER_ERROR_ENT         ipHeaderErrorType,
    IN  CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN  CPSS_UNICAST_MULTICAST_ENT          prefixType,
    OUT GT_BOOL                             *maskPtr
);

/**
* @internal prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet function
* @endinternal
*
* @brief   Sets device ID modification for Routed packets.
*         Enables/Disables FORWARD DSA tag modification of the <source device>
*         and <source port> fields of packets routed by the local device.
*         The <source device> is set to the local device ID and the <source port>
*         is set to 61 (the virtual router port).
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
* @param[in] modifyEnable             - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 modifyEnable
);

/**
* @internal prvTgfIpArpBcModeSet function
* @endinternal
*
* @brief   set a arp broadcast mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpArpBcModeSet
(
    IN CPSS_PACKET_CMD_ENT arpBcMode
);

/**
* @internal prvTgfIpArpBcModeGet function
* @endinternal
*
* @brief   get a arp broadcast mode.
*
* @param[out] arpBcModePtr             - the arp broadcast command. Possible Commands:
*                                      CPSS_PACKET_CMD_NONE_E,CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpArpBcModeGet
(
    OUT CPSS_PACKET_CMD_ENT *arpBcModePtr
);

/**
* @internal prvTgfIpEcmpEntryWrite function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*                                      (APPLICABLE RANGES: 0..12287)
* @param[in] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpEcmpEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       ecmpEntryIndex,
    IN PRV_TGF_IP_ECMP_ENTRY_STC    *ecmpEntryPtr
);

/**
* @internal prvTgfIpEcmpEntryRead function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*                                      (APPLICABLE RANGES: 0..12287)
*
* @param[out] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpEcmpEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ecmpEntryIndex,
    OUT PRV_TGF_IP_ECMP_ENTRY_STC    *ecmpEntryPtr
);

/**
* @internal prvTgfIpRouterSourceIdSet function
* @endinternal
*
* @brief   set the router source id assignment
*
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
* @param[in] sourceId                 - the assigned source id.
* @param[in] sourceIdMask             - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterSourceIdSet
(
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId,
    IN  GT_U32                          sourceIdMask
);

/**
* @internal prvTgfIpRouterSourceIdGet function
* @endinternal
*
* @brief   get the router source id assignment
*
* @param[in] ucMcSet                  - whether to get it for unicast packets or multicast.
*
* @param[out] sourceIdPtr              - the assigned source id.
* @param[out] sourceIdMaskPtr          - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterSourceIdGet
(
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr,
    OUT GT_U32                          *sourceIdMaskPtr
);



/**
* @internal prvTgfLpmLeafEntryWrite function
* @endinternal
*
* @brief   Write an LPM leaf entry, which is used for policy based routing, to the
*         HW
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
* @param[in] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS prvTgfLpmLeafEntryWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    IN PRV_TGF_LPM_LEAF_ENTRY_STC               *leafPtr
);

/**
* @internal prvTgfLpmLeafEntryRead function
* @endinternal
*
* @brief   Read an LPM leaf entry, which is used for policy based routing, from the
*         HW
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS prvTgfLpmLeafEntryRead
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    OUT PRV_TGF_LPM_LEAF_ENTRY_STC              *leafPtr
);

/**
* @internal prvTgfIpMllMultiTargetShaperBaselineSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline,
*         the respective packet is not served.
* @param[in] devNum                   - device number.
* @param[in] baseline                 - Token Bucket Baseline value in bytes
*                                      (APPLICABLE RANGES: 0..0xFFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU (the maximum expected packet size in the system).
*       2. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       cpssDxChIpMllMultiTargetShaperMtuSet.
*
*/
GT_STATUS   prvTgfIpMllMultiTargetShaperBaselineSet
(
    IN GT_U8    devNum,
    IN GT_U32   baseline
);

/**
* @internal prvTgfIpMllMultiTargetShaperBaselineGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective packet
*         is not served.
* @param[in] devNum                   - device number.
*
* @param[out] baselinePtr              - (pointer to) Token Bucket Baseline value in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperBaselineGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *baselinePtr
);

/**
* @internal prvTgfIpMllMultiTargetShaperMtuSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
* @param[in] devNum                   - physical device number
* @param[in] mtu                      - MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mtu
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperMtuSet
(
    IN GT_U8   devNum,
    IN GT_U32  mtu
);

/**
* @internal prvTgfIpMllMultiTargetShaperMtuGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
* @param[in] devNum                   - physical device number
*
* @param[out] mtuPtr                   -  pointer to MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperMtuGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *mtuPtr
);

/**
* @internal prvTgfIpMllMultiTargetShaperTokenBucketModeSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Mode Mll shaper.
* @param[in] devNum                   - device number.
* @param[in] tokenBucketMode          -   Defines the packet length used to decrease the shaper token bucket.
*                                      In Byte mode, the token bucket rate shaper is decreased according to the
*                                      packet length as received by the switch.
*                                      In Packet mode, the token bucket rate shaper is decreased by the packet
*                                      length specified in cpssDxChIpMllMultiTargetShaperMtuSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperTokenBucketModeSet
(
    IN  GT_U8                                        devNum,
    IN  CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            tokenBucketMode
);

/**
* @internal prvTgfIpMllMultiTargetShaperTokenBucketModeGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Relevant for L2 and L3 MLL.
*         Get Token Bucket Mode Mll shaper.
* @param[in] devNum                   - device number.
*
* @param[out] tokenBucketModePtr       - (pointer to)Defines the packet length used to decrease the shaper token bucket.
*                                      In Byte mode, the token bucket rate shaper is decreased according to the
*                                      packet length as received by the switch.
*                                      In Packet mode, the token bucket rate shaper is decreased by the packet
*                                      length specified in cpssDxChIpMllMultiTargetShaperMtuSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperTokenBucketModeGet
(
    IN  GT_U8                                        devNum,
    OUT CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            *tokenBucketModePtr
);

/**
* @internal prvTgfIpUcRpfModeSet function
* @endinternal
*
* @brief   Defines the uRPF check mode for a given VID.
*
* @param[in] devNum                   - the device number
* @param[in] vid                      - Vlan Id
* @param[in] uRpfMode                 - unicast RPF mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, vid or uRpfMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
GT_STATUS prvTgfIpUcRpfModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U16                          vid,
    IN  PRV_TGF_IP_URPF_MODE_ENT        uRpfMode
);

/**
* @internal prvTgfIpUcRpfModeGet function
* @endinternal
*
* @brief   Read uRPF check mode for a given VID.
*
* @param[in] devNum                   - the device number
* @param[in] vid                      - Vlan Id
*
* @param[out] uRpfModePtr              -  (pointer to) unicast RPF mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
GT_STATUS prvTgfIpUcRpfModeGet
(
    IN  GT_U8                           devNum,
    IN  GT_U16                          vid,
    OUT PRV_TGF_IP_URPF_MODE_ENT        *uRpfModePtr
);

/**
* @internal prvTgfIpUrpfLooseModeTypeSet function
* @endinternal
*
* @brief   This function set type of Urpf loose mode
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3x ; Lion2; Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   -  the device number
* @param[in] looseModeType            - value of urpf loose mode
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpUrpfLooseModeTypeSet
(
    IN  GT_U8                            devNum,
    IN  PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT looseModeType
);

/**
* @internal prvTgfIpMllMultiTargetShaperEnableSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Enable/Disable Token Bucket rate shaping.
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvTgfIpMllMultiTargetShaperEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal prvTgfIpMllMultiTargetShaperEnableGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Enable/Disable Token Bucket rate shaping status
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - Pointer to Token Bucket rate shaping status.
*                                      - GT_TRUE, enable Shaping
*                                      - GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfIpMllMultiTargetShaperConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper.
* @param[in] devNum                   - device number.
* @param[in] maxBucketSize            - Maximum bucket size in bytes. The field resolution is 4096 bytes.
*                                      The actual burst size is approximately
*                                      <MaxBucketSize>4096 - <Token Bucket Base Line>
*                                      When the CPU writes to this field, the value is also written
*                                      to the Token Bucket counter.
*                                      <MaxBucketSize>4096 must be set to be greater than both <Tokens>
*                                      and <Token Bucket Base Line>.
*                                      0 means 4k and 0xFFF means 16M
*                                      The bucket size ranges from 4 KB to 16 MB, in steps of 4K.
*                                      (APPLICABLE RANGES: 0...0xFFF)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second
*                                      according to shaper mode.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       see:
*       cpssDxChIpMllMultiTargetShaperMtuSet
*       cpssDxChIpMllMultiTargetShaperBaselineSet.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperConfigurationSet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       maxBucketSize,
    INOUT GT_U32                                     *maxRatePtr
);


/**
* @internal prvTgfIpMllMultiTargetShaperConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper.
* @param[in] devNum                   - device number.
*
* @param[out] maxBucketSizePtr         - (pointer to) Maximum bucket size in bytes. The field resolution is 4096 bytes.
*                                      The actual burst size is approximately
*                                      <MaxBucketSize>4096 - <Token Bucket Base Line>
*                                      When the CPU writes to this field, the value is also written
*                                      to the Token Bucket counter.
*                                      <MaxBucketSize>4096 must be set to be greater than both <Tokens>
*                                      and <Token Bucket Base Line>.
*                                      0 equal 4k all 12'bFFF equal 16M
*                                      The bucket size ranges from 4 KB to 16 MB, in steps of 4K.
*                                      (APPLICABLE RANGES: 0...0xFFF)
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperConfigurationGet
(
    IN  GT_U8                                        devNum,
    OUT GT_U32                                       *maxBucketSizePtr,
    OUT GT_U32                                       *maxRatePtr
);


/**
* @internal prvTgfIpMllMultiTargetShaperIntervalConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper Interval.
* @param[in] devNum                   - device number.
* @param[in] tokenBucketIntervalSlowUpdateRatio - Increases the token bucket update interval to
*                                      <TokenBucketIntervalSlowUpdateRatio>+1>  <TokenBucketUpdateInterval>
*                                      (64 or 1024 depends on <TokenBucketIntervalUpdateRatio>) core clock cycles.
*                                      Enabled by setting SlowRateEn = GT_TRUE.
*                                      (APPLICABLE RANGES: 1...16)
* @param[in] tokenBucketUpdateInterval -   Defines the token update interval for the egress rate shapers.
*                                      The update interval = <TokenBucketIntervalSlowUpdateRatio>+1>
*                                      <TokenBucketUpdateInterval>  (64 or 1024 depends on
*                                      <TokenBucketIntervalUpdateRatio>)2^(Port/PriorityTokenBucketEntry<TBIntervalUpdateRatio>) core clock cycles.
*                                      Upon every interval expiration, a configured amount of tokens is added to the
*                                      token bucket. The amount of tokens is configured in Port/PriorityTokenBucketEntry<Tokens>.
*                                      NOTE:
*                                      - This field must not be 0.
*                                      (APPLICABLE RANGES: 1...15)
* @param[in] tokenBucketIntervalUpdateRatio - Token Bucket Interval Update Ratio : 64 or 1024
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperIntervalConfigurationSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          tokenBucketIntervalSlowUpdateRatio,
    IN  GT_U32                                          tokenBucketUpdateInterval,
    IN  PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_ENT     tokenBucketIntervalUpdateRatio
);

/**
* @internal prvTgfIpMllMultiTargetShaperIntervalConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper Interval.
* @param[in] devNum                   - device number.
*
* @param[out] tokenBucketIntervalSlowUpdateRatioPtr - (pointer to) Increases the token bucket update interval to
*                                      <TokenBucketIntervalSlowUpdateRatio>+1>  <TokenBucketUpdateInterval>
*                                      (64 or 1024 depends on <TokenBucketIntervalUpdateRatio>) core clock cycles.
*                                      Enabled by setting SlowRateEn = GT_TRUE.
*                                      (APPLICABLE RANGES: 1...16)
* @param[out] tokenBucketUpdateIntervalPtr -   (pointer to) the token update interval for the egress rate shapers.
*                                      The update interval = <TokenBucketIntervalSlowUpdateRatio>+1>
*                                      <TokenBucketUpdateInterval>  (64 or 1024 depends on
*                                      <TokenBucketIntervalUpdateRatio>)2^(Port/PriorityTokenBucketEntry<TBIntervalUpdateRatio>) core clock cycles.
*                                      Upon every interval expiration, a configured amount of tokens is added to the
*                                      token bucket. The amount of tokens is configured in Port/PriorityTokenBucketEntry<Tokens>.
*                                      NOTE:
*                                      - This field must not be 0.
*                                      (APPLICABLE RANGES: 1...15)
* @param[out] tokenBucketIntervalUpdateRatioPtr - (pointer to) Token Bucket Interval Update Ratio : 64 or 1024
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperIntervalConfigurationGet
(
    IN   GT_U8                                           devNum,
    OUT  GT_U32                                          *tokenBucketIntervalSlowUpdateRatioPtr,
    OUT  GT_U32                                          *tokenBucketUpdateIntervalPtr,
    OUT  PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_ENT     *tokenBucketIntervalUpdateRatioPtr
);

/**
* @internal prvTgfIpPortSipSaEnableSet function
* @endinternal
*
* @brief   Enable SIP/SA check for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (including CPU port)
* @param[in] enable                   - GT_FALSE: disable SIP/SA check on the port
*                                      GT_TRUE:  enable SIP/SA check on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
GT_STATUS prvTgfIpPortSipSaEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
);

/**
* @internal prvTgfIpPortSipSaEnableGet function
* @endinternal
*
* @brief   Return the SIP/SA check status for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (including CPU port)
*
* @param[out] enablePtr                - GT_FALSE: SIP/SA check on the port is disabled
*                                      GT_TRUE:  SIP/SA check on the port is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
GT_STATUS prvTgfIpPortSipSaEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal prvTgfIpEcmpHashNumBitsSet function
* @endinternal
*
* @brief   Set the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism
* @param[in] devNum                   - the device number
* @param[in] startBit                 - the index of the first bit that is needed by the L3
*                                      ECMP hash mechanism (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - the number of bits that is needed by the L3 ECMP hash
*                                      mechanism (APPLICABLE RANGES: 1..16)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong startBit or numOfBits
*
* @note startBit is relevant only when randomEnable field in the entry is set to
*       GT_FALSE.
*       startBit + numOfBits must not exceed 32.
*
*/
GT_STATUS prvTgfIpEcmpHashNumBitsSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       startBit,
    IN GT_U32                       numOfBits
);


/**
* @internal prvTgfIpEcmpHashNumBitsGet function
* @endinternal
*
* @brief   Get the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism
* @param[in] devNum                   - the device number
*
* @param[out] startBitPtr              - (pointer to) the index of the first bit that is needed
*                                      by the L3 ECMP hash mechanism
* @param[out] numOfBitsPtr             - (pointer to) the number of bits that is needed by the
*                                      L3 ECMP hash mechanism
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note startBit is relevant only when randomEnable field in the entry is set to
*       GT_FALSE.
*
*/
GT_STATUS prvTgfIpEcmpHashNumBitsGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *startBitPtr,
    OUT GT_U32                      *numOfBitsPtr
);

/**
* @internal prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet function
* @endinternal
*
* @brief   Enable/disable bypassing the router triggering requirements for policy
*         based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  bypassing the router triggering requirements for PBR
*                                      packets:
*                                      GT_FALSE: Policy based routing uses the normal router
*                                      triggering requirements
*                                      GT_TRUE:  Policy based routing bypasses the router triggering
*                                      requirement for the FDB DA entry <DA Route> to be
*                                      set for UC routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet function
* @endinternal
*
* @brief   Get the enabling status of bypassing the router triggering requirements
*         for policy based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - the router triggering requirements enabling status for PBR
*                                      packets:
*                                      GT_FALSE: Policy based routing uses the normal router
*                                      triggering requirements
*                                      GT_TRUE:  Policy based routing bypasses the router triggering
*                                      requirement for the FDB DA entry <DA Route> to be
*                                      set for UC routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfIpNatEntrySet function
* @endinternal
*
* @brief   Set a NAT entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] natIndex                 - index for the NAT entry
*                                      in the router ARP / tunnel start / NAT table
* @param[in] natType                  - type of the NAT
* @param[in] entryPtr                 - (pointer to) NAT entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table, Tunnel start entries table and router ARP addresses
*       table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS prvTgfIpNatEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              natIndex,
    IN  PRV_TGF_IP_NAT_TYPE_ENT             natType,
    IN  PRV_TGF_IP_NAT_ENTRY_UNT           *entryPtr
);

/**
* @internal prvTgfIpNatEntryGet function
* @endinternal
*
* @brief   Get a NAT entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] natIndex                 - index for the NAT entry
*                                      in the router ARP / tunnel start / NAT table
*
* @param[out] natTypePtr               - (pointer to) type of the NAT
* @param[out] entryPtr                 - (pointer to) NAT entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table, Tunnel start entries table and router ARP addresses
*       table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS prvTgfIpNatEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             natIndex,
    OUT  PRV_TGF_IP_NAT_TYPE_ENT            *natTypePtr,
    OUT  PRV_TGF_IP_NAT_ENTRY_UNT           *entryPtr
);

/**
* @internal prvTgfIpLpmDbgHwOctetPerBlockPrint function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerBlockPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrint
(
    IN GT_U32                           lpmDbId
);


/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters
(
    IN GT_U32                           lpmDbId
);


/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines and Banks memory debug information for a
*         specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters
(
    IN GT_U32                           lpmDbId
);

/**
* @internal prvTgfIpMllBridgeEnable function
* @endinternal
*
* @brief   enable/disable MLL based bridging.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] mllBridgeEnable          - enable /disable MLL based bridging.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllBridgeEnable
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    mllBridgeEnable
);

/**
* @internal prvTgfIpBridgeServiceEnable function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] bridgeService            - the router bridge service
* @param[in] enableDisableMode        - Enable/Disable mode of this function (weather
*                                      to enable/disable for ipv4/ipv6/arp)
* @param[in] enableService            - weather to enable the service for the above more.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpBridgeServiceEnable
(
    IN  GT_U8                                           devNum,
    IN  PRV_TGF_IP_BRG_SERVICE_ENT                      bridgeService,
    IN  PRV_TGF_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT  enableDisableMode,
    IN  GT_BOOL                                         enableService
);

/**
* @internal prvTgfIpBridgeServiceEnableGet function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] bridgeService            - the router bridge service
* @param[in] enableDisableMode        - Enable/Disable mode of this function (weather
*                                      to enable/disable for ipv4/ipv6/arp)
*
* @param[out] enableServicePtr         - weather to enable the service for the above more.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpBridgeServiceEnableGet
(
    IN  GT_U8                                           devNum,
    IN  PRV_TGF_IP_BRG_SERVICE_ENT                      bridgeService,
    IN  PRV_TGF_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT  enableDisableMode,
    OUT GT_BOOL                                         *enableServicePtr
);

/**
* @internal prvTgfIpLpmActivityBitEnableGet function
* @endinternal
*
* @brief   Get status of the aging/refresh mechanism of trie leaf entries in the
*         LPM memory.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmActivityBitEnableGet
(
    OUT GT_BOOL     *activityBitPtr
);

/**
* @internal prvTgfIpLpmActivityBitEnableSet function
* @endinternal
*
* @brief   Set status of the aging/refresh mechanism of trie leaf entries in the
*         LPM memory.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmActivityBitEnableSet
(
    IN  GT_BOOL     activityBit
);

/**
* @internal prvTgfIpLpmIpv4UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 UC prefix for a specific LPM DB
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixActivityStatusGet
(
    IN GT_U32       lpmDbId,
    IN GT_U32       vrId,
    IN GT_IPADDR    ipAddr,
    IN GT_U32       prefixLen,
    IN GT_BOOL      clearActivity,
    OUT GT_BOOL     *activityStatusPtr
);

/**
* @internal prvTgfIpLpmIpv4McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2.
*
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv4McEntryActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   ipGroup,
    IN  GT_U32      ipGroupPrefixLen,
    IN  GT_IPADDR   ipSrc,
    IN  GT_U32      ipSrcPrefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
);

/**
* @internal prvTgfIpLpmIpv6McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2.
*
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv6McEntryActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR ipGroup,
    IN  GT_U32      ipGroupPrefixLen,
    IN  GT_IPV6ADDR ipSrc,
    IN  GT_U32      ipSrcPrefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
);

/**
* @internal prvTgfIpLpmIpv6UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixActivityStatusGet
(
    IN GT_U32       lpmDbId,
    IN GT_U32       vrId,
    IN GT_IPV6ADDR  ipAddr,
    IN GT_U32       prefixLen,
    IN GT_BOOL      clearActivity,
    OUT GT_BOOL     *activityStatusPtr
);

/**
* @internal prvTgfIpPortFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Enable FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - the port to enable on
* @param[in] enable                   -  FCoE Forwarding for this port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpPortFcoeForwardingEnableSet
(
    IN GT_PORT_NUM                      portNum,
    IN GT_BOOL                          enable
);

/**
* @internal prvTgfIpPortFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Get status of FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] enablePtr                - (pointer to)enable FCoE Forwarding for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpPortFcoeForwardingEnableGet
(
    IN  GT_PORT_NUM                      portNum,
    OUT GT_BOOL                          *enablePtr
);

/**
* @internal prvTgfIpLpmFcoePrefixAdd function
* @endinternal
*
* @brief   This function adds a new FCoE prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing FCoE prefix.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvTgfIpLpmFcoePrefixAdd
(
    IN  GT_U32                                      lpmDBId,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    IN  CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT      *nextHopInfoPtr,
    IN  GT_BOOL                                     override,
    IN GT_BOOL                                      defragmentationEnable
);

/**
* @internal prvTgfIpLpmFcoePrefixDel function
* @endinternal
*
* @brief   Deletes an existing FCoE prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NO_SUCH               - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvTgfIpLpmFcoePrefixDel
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 fcoeAddr,
    IN  GT_U32                                  prefixLen
);

/**
* @internal prvTgfIpFdbRoutePrefixLenSet function
* @endinternal
*
* @brief   set the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
* @param[in] prefixLen                - The number of bits that
*                                       are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpFdbRoutePrefixLenSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_U32                       prefixLen
);

/**
* @internal prvTgfIpFdbRoutePrefixLenGet function
* @endinternal
*
* @brief   get the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
*
* @param[out] prefixLen                - The number of bits that
*                                       are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_VALUE             - on bad output value
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpFdbRoutePrefixLenGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_U32                      *prefixLenPtr
);

/**
* @internal prvTgfIpFdbUnicastRouteForPbrEnableSet function
* @endinternal
*
* @brief   Enable/Disable FDB Unicast routing for PBR (Policy Based
*          Routed) packets
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable FDB
*                                       routing for PBR packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
GT_STATUS prvTgfIpFdbUnicastRouteForPbrEnableSet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      enable
);

/**
* @internal prvTgfIpFdbUnicastRouteForPbrEnableGet function
* @endinternal
*
* @brief   Return if FDB Unicast routing for PBR value (Policy Based
*          Routed) packets is enabled
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr               - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
GT_STATUS prvTgfIpFdbUnicastRouteForPbrEnableGet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfIpGenh */
