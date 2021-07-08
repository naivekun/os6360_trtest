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
* @file tgfTunnelGen.h
*
* @brief Generic API for Tunnel
*
* @version   74
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfTunnelGenh
#endif /*CHX_FAMILY*/

#ifndef __tgfTunnelGenh
#define __tgfTunnelGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>
#include <common/tgfInlifGen.h>

#ifdef CHX_FAMILY
    extern GT_U32 appDemoDxChTcamTtiBaseIndexGet
    (
        IN     GT_U8                            devNum,
        IN     GT_U32                           hitNum
    );
    extern GT_U32 appDemoDxChTcamTtiNumOfIndexsGet
    (
        IN     GT_U8                            devNum,
        IN     GT_U32                           hitNum
    );
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

/* default PCL ID for IPv4 */
#define PRV_TGF_TTI_IPV4_DEFAULT_PCL_ID_CNS     1

/* max number for UDB */
#define PRV_TGF_TTI_MAX_UDB_CNS 30

/* max size of template data in bits that generates by Profile table entry */
#define PRV_TGF_TUNNEL_START_TEMPLATE_DATA_MAX_SIZE_CNS 128

/* max num of tti lookups (general limit) */
#define PRV_TGF_MAX_LOOKUPS_NUM_CNS 4

/* max num of tti lookups, supported by device, eArch only */
#define PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS  \
    ( PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) ? \
     PRV_TGF_MAX_LOOKUPS_NUM_CNS : (UTF_CPSS_PP_EXMXPM_CHECK_MAC(prvTgfDevNum) ? 1 : 2) )

#define PRV_TTI_LOOKUP_0    0
#define PRV_TTI_LOOKUP_1    1
#define PRV_TTI_LOOKUP_2    2
#define PRV_TTI_LOOKUP_3    3

#ifdef CHX_FAMILY
#define PRV_TGF_FIRST_TTI_INDEX_MAC(_devNum,_hitNum)    \
        appDemoDxChTcamTtiBaseIndexGet(_devNum,_hitNum)

#else
#define PRV_TGF_FIRST_TTI_INDEX_MAC(_devNum,_hitNum)    0
#endif
/**
* @enum PRV_TGF_TTI_REDIRECT_COMMAND_ENT
 *
 * @brief TTI action redirect command
*/
typedef enum{

    /** do not redirect this packet */
    PRV_TGF_TTI_NO_REDIRECT_E,

    /** policy switching */
    PRV_TGF_TTI_REDIRECT_TO_EGRESS_E,

    /** policy routing */
    PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E,

    /** vrf id assignment */
    PRV_TGF_TTI_VRF_ID_ASSIGN_E,

    /** redirect to a configured OutLIF */
    PRV_TGF_TTI_REDIRECT_TO_OUTLIF_E,

    /** redirect to an IP Next Hop Entry */
    PRV_TGF_TTI_REDIRECT_TO_NEXT_HOP_E,

    /** trigger VPLS Lookup */
    PRV_TGF_TTI_TRIGGER_VPLS_LOOKUP_E,

    /** apply logical port assignment */
    PRV_TGF_TTI_LOGICAL_PORT_ASSIGN_E

} PRV_TGF_TTI_REDIRECT_COMMAND_ENT;

/**
* @enum PRV_TGF_TTI_PASSENGER_TYPE_ENT
 *
 * @brief TTI tunnel passenger protocol types
*/
typedef enum{

    /** IPv4 passenger type */
    PRV_TGF_TTI_PASSENGER_IPV4_E,

    /** IPv6 passenger type */
    PRV_TGF_TTI_PASSENGER_IPV6_E,

    /** ethernet with crc passenger type */
    PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E,

    /** ethernet with no crc passenger type */
    PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E,

    /** IPv4/IPv6 passenger type */
    PRV_TGF_TTI_PASSENGER_IPV4V6_E,

    /** MPLS passenger type */
    PRV_TGF_TTI_PASSENGER_MPLS_E

} PRV_TGF_TTI_PASSENGER_TYPE_ENT;

/**
* @enum PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT
 *
 * @brief Tunnel passenger protocol types
*/
typedef enum{

    /** ethernet passenger type */
    PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E,

    /** other passenger type */
    PRV_TGF_TUNNEL_PASSENGER_OTHER_E

} PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT;

/**
* @enum PRV_TGF_TTI_VLAN_COMMAND_ENT
 *
 * @brief TTI action VLAN command.
*/
typedef enum{

    /** do not modify vlan (not applicable to tag1) */
    PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E,

    /** modify vlan only for untagged packets */
    PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E,

    /** modify vlan only for tagged packets (not applicable to tag1) */
    PRV_TGF_TTI_VLAN_MODIFY_TAGGED_E,

    /** modify vlan to all packets */
    PRV_TGF_TTI_VLAN_MODIFY_ALL_E

} PRV_TGF_TTI_VLAN_COMMAND_ENT;


/**
* @enum PRV_TGF_TTI_VLAN1_COMMAND_ENT
 *
 * @brief TTI action tag1 VLAN command.
*/
typedef enum{

    /** modify vlan only for untagged packets */
    PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E,

    /** modify vlan to all packets */
    PRV_TGF_TTI_VLAN1_MODIFY_ALL_E,

    /** do not modify vlan (not applicable to tag1) */
    PRV_TGF_TTI_VLAN1_DO_NOT_MODIFY_E,

    /** modify vlan only for tagged packets (not applicable to tag1) */
    PRV_TGF_TTI_VLAN1_MODIFY_TAGGED_E

} PRV_TGF_TTI_VLAN1_COMMAND_ENT;
/**
* @enum PRV_TGF_TTI_KEY_TYPE_ENT
 *
 * @brief TTI key type.
*/
typedef enum
{
    PRV_TGF_TTI_KEY_IPV4_E,
    PRV_TGF_TTI_KEY_MPLS_E,
    PRV_TGF_TTI_KEY_ETH_E,
    PRV_TGF_TTI_KEY_MIM_E,
    PRV_TGF_TTI_KEY_UDB_IPV4_TCP_E,
    PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E,
    PRV_TGF_TTI_KEY_UDB_MPLS_E,
    PRV_TGF_TTI_KEY_UDB_IPV4_FRAGMENT_E,
    PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E,
    PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
    PRV_TGF_TTI_KEY_UDB_IPV6_E,
    PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E,
    PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
    PRV_TGF_TTI_KEY_UDB_UDE_E,
    PRV_TGF_TTI_KEY_UDB_UDE1_E,
    PRV_TGF_TTI_KEY_UDB_UDE2_E,
    PRV_TGF_TTI_KEY_UDB_UDE3_E,
    PRV_TGF_TTI_KEY_UDB_UDE4_E,
    PRV_TGF_TTI_KEY_UDB_UDE5_E,
    PRV_TGF_TTI_KEY_UDB_UDE6_E,
    PRV_TGF_TTI_KEY_TRILL_E /* to be remove later */
} PRV_TGF_TTI_KEY_TYPE_ENT;

/**
* @enum PRV_TGF_TTI_KEY_SIZE_ENT
 *
 * @brief TTI key size.
*/
typedef enum{

    /** TCAM key size to be used 10 Byets */
    PRV_TGF_TTI_KEY_SIZE_10_B_E  = 0,

    /** TCAM key size to be used 20 Byets */
    PRV_TGF_TTI_KEY_SIZE_20_B_E  = 1,

    /** TCAM key size to be used 30 Byets */
    PRV_TGF_TTI_KEY_SIZE_30_B_E  = 2

} PRV_TGF_TTI_KEY_SIZE_ENT;

/**
* @enum PRV_TGF_TTI_RULE_TYPE_ENT
 *
 * @brief TTI rule type.
*/
typedef enum{

    /** IPv4 TTI rule size 30 bytes type */
    PRV_TGF_TTI_RULE_IPV4_E  = PRV_TGF_TTI_KEY_IPV4_E,

    /** MPLS TTI rule size 30 bytes type */
    PRV_TGF_TTI_RULE_MPLS_E  = PRV_TGF_TTI_KEY_MPLS_E,

    /** Ethernet TTI rule size 30 bytes type */
    PRV_TGF_TTI_RULE_ETH_E   = PRV_TGF_TTI_KEY_ETH_E,

    /** Mac in Mac TTI rule size 30 bytes type (APPLICABLE DEVICES: xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    PRV_TGF_TTI_RULE_MIM_E   = PRV_TGF_TTI_KEY_MIM_E,

    /** UDB rule size 10 bytes type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    PRV_TGF_TTI_RULE_UDB_10_E,

    /** UDB rule size 20 bytes type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    PRV_TGF_TTI_RULE_UDB_20_E,

    /** UDB rule size 30 bytes type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    PRV_TGF_TTI_RULE_UDB_30_E

} PRV_TGF_TTI_RULE_TYPE_ENT;

/**
* @enum PRV_TGF_TTI_MAC_MODE_ENT
 *
 * @brief Determines MAC that will be used to generate lookup TCAM key
*/
typedef enum{

    /** use destination MAC */
    PRV_TGF_TTI_MAC_MODE_DA_E,

    /** use source MAC */
    PRV_TGF_TTI_MAC_MODE_SA_E

} PRV_TGF_TTI_MAC_MODE_ENT;

/**
* @enum PRV_TGF_TTI_QOS_MODE_TYPE_ENT
 *
 * @brief QoS trust mode type
*/
typedef enum{

    /** keep prior QoS profile assignment */
    PRV_TGF_TTI_QOS_KEEP_PREVIOUS_E    = 0,

    /** Trust passenger packet L2 QoS */
    PRV_TGF_TTI_QOS_TRUST_PASS_L2_E    = 1,

    /** Trust passenger packet L3 QoS */
    PRV_TGF_TTI_QOS_TRUST_PASS_L3_E    = 2,

    /** Trust passenger packet L2 and L3 QoS */
    PRV_TGF_TTI_QOS_TRUST_PASS_L2_L3_E = 3,

    /** Untrust packet QoS */
    PRV_TGF_TTI_QOS_UNTRUST_E          = 4,

    /** Trust outer MPLS label EXP */
    PRV_TGF_TTI_QOS_TRUST_MPLS_EXP_E   = 5

} PRV_TGF_TTI_QOS_MODE_TYPE_ENT;

/**
* @enum PRV_TGF_TTI_MODIFY_UP_ENT
 *
 * @brief TTI modify UP enable
*/
typedef enum{

    /** do not modify the previous UP enable setting */
    PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E = 0,

    /** set modify UP enable flag to 1 */
    PRV_TGF_TTI_MODIFY_UP_ENABLE_E      = 1,

    /** set modify UP enable flag to 0 */
    PRV_TGF_TTI_MODIFY_UP_DISABLE_E     = 2,

    /** reserved */
    PRV_TGF_TTI_MODIFY_UP_RESERVED_E    = 3

} PRV_TGF_TTI_MODIFY_UP_ENT;

/**
* @enum PRV_TGF_TTI_TAG1_UP_COMMAND_ENT
 *
 * @brief TTI tag1 UP command
*/
typedef enum{

    /** @brief If packet does not contain Tag1 assign according
     *  to action entry's <UP1>, else use Tag1<UP>
     */
    PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E = 0,

    /** @brief If packet contains Tag0 use Tag0<UP0>, else
     *  use action entry's <UP1> field
     */
    PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E = 1,

    /** Assign action entry's <UP1> field to all packets */
    PRV_TGF_TTI_TAG1_UP_ASSIGN_ALL_E            = 2

} PRV_TGF_TTI_TAG1_UP_COMMAND_ENT;

/**
* @enum PRV_TGF_TTI_MODIFY_DSCP_ENT
 *
 * @brief TTI modify DSCP enable
*/
typedef enum{

    /** do not modify the previous DSCP enable setting */
    PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E = 0,

    /** set modify DSCP enable flag to 1 */
    PRV_TGF_TTI_MODIFY_DSCP_ENABLE_E      = 1,

    /** set modify DSCP enable flag to 0 */
    PRV_TGF_TTI_MODIFY_DSCP_DISABLE_E     = 2,

    /** reserved */
    PRV_TGF_TTI_MODIFY_DSCP_RESERVED_E    = 3

} PRV_TGF_TTI_MODIFY_DSCP_ENT;

/**
* @enum PRV_TGF_TTI_MPLS_COMMAND_ENT
 *
 * @brief TTI action MPLS command.
*/
typedef enum{

    /** self explainatory */
    PRV_TGF_TTI_MPLS_NOP_COMMAND_E          = 0,

    /** self explainatory */
    PRV_TGF_TTI_MPLS_SWAP_COMMAND_E         = 1,

    /** self explainatory */
    PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E  = 2,

    /** self explainatory */
    PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E   = 3,

    /** self explainatory */
    PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E  = 4,

    /** self explainatory */
    PRV_TGF_TTI_MPLS_POP3_LABELS_COMMAND_E  = 5,

    /** self explainatory */
    PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E = 6

} PRV_TGF_TTI_MPLS_COMMAND_ENT;

/**
* @enum PRV_TGF_TTI_PW_ID_MODE_ENT
 *
 * @brief TTI pw assignment mode
*/
typedef enum{

    /** pseudo wire id is defined in TTI entry */
    PRV_TGF_TTI_PW_ID_MODE_ENTRY_E  = 0,

    /** @brief pseudo wire id is derived according to
     *  packet's (VC Label)
     */
    PRV_TGF_TTI_PW_ID_MODE_BASE_E   = 1

} PRV_TGF_TTI_PW_ID_MODE_ENT;

/**
* @enum PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT
 *
 * @brief TTI passenger parsing of transit MPLS tunnel mode
*/
typedef enum{

    /** parsing is based on the tunnel header. The passenger packet is not parsed. */
    PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E,

    /** Parse passenger packet as IPv4/6 */
    PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E,

    /** Parse passenger packet as Ethernet */
    PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E,

    /** Parse passenger packet as Control Word followed by Ethernet */
    PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E

} PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT;

/**
* @enum PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of tunnel start MIM B-DA (Backbone
 * destination address) assign mode
*/
typedef enum{

    /** @brief BDA is fully assigned from Tunnelstart entry: <Tunnel MAC DA>. This
     *  mode is used for known unicast packets.
     *  PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E -
     *  The most-significant bits of the B-DA(47:24) are assigned from the entry
     *  <Tunnel MAC DA> and the 24 least-significant bits(23:0) are assigned from
     *  the eVLAN <Service-ID> field. This mode is used for unknown unicast,
     *  multicast, and broadcast packets.
     */
    PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E = 0,

    PRV_TGF_CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E = 1

} PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT;

/**
* @enum PRV_TGF_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of tunnel start MIM B-SA (Backbone
 * source address) assign mode
*/
typedef enum{

    /** @brief The MAC SA is selected from a dedicated 256 global MAC SA table
     *  based on the target ePort <Router MAC SA Index>.
     */
    PRV_TGF_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E = 0,

    /** @brief The MAC SA is selected from a dedicated 256 global MAC SA table
     *  based on the source ePort <Router MAC SA Index>.
     */
    PRV_TGF_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E = 1

} PRV_TGF_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT;


/**
* @enum PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of tunnel start MIM I-SID assign mode
*/
typedef enum{

    /** ISID assigned from the tunnel start entry */
    PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E = 0,

    /** ISID assigned from the VLAN table to VLAN ServiceID */
    PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E = 1

} PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT;

/**
* @enum PRV_TGF_TUNNEL_START_MARK_MODE_ENT
 *
 * @brief Enumeration of tunnel start entry mark modes
*/
typedef enum{

    /** based on Tunnel Start entry */
    PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E              = 0,

    /** based on t QoS Profile */
    PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E = 1,

    /** based on ingress pipe */
    PRV_TGF_TUNNEL_START_MARK_FROM_INGRESS_PIPE_E       = 2

} PRV_TGF_TUNNEL_START_MARK_MODE_ENT;

/**
* @enum PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_ENT
 *
 * @brief Enumeration of tunnel start entry mpls ethertypes
*/
typedef enum{

    /** MPLS ethertype Unicast */
    PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_UC_E = 0,

    /** MPLS ethertype Multicast */
    PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_MC_E = 1

} PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_ENT;

/**
* @enum PRV_TGF_TTI_ACTION_TYPE_ENT
 *
 * @brief TTI action type enum
*/
typedef enum{
    /** type2 action */
    PRV_TGF_TTI_ACTION_TYPE2_ENT = 1

} PRV_TGF_TTI_ACTION_TYPE_ENT;

/**
* @enum PRV_TGF_TUNNEL_QOS_MODE_ENT
 *
 * @brief Enumeration of QoS modes
*/
typedef enum{

    /** @brief keep QoS profile assignment from
     *  previous mechanisms
     */
    PRV_TGF_TUNNEL_QOS_KEEP_PREVIOUS_E      = 0,

    /** @brief trust passenger protocol 802.1p user
     *  priority
     */
    PRV_TGF_TUNNEL_QOS_TRUST_PASS_UP_E      = 1,

    /** trust passenger protocol IP DSCP */
    PRV_TGF_TUNNEL_QOS_TRUST_PASS_DSCP_E    = 2,

    /** @brief trust passenger protocol 802.1p user
     *  priority and IP DSCP
     */
    PRV_TGF_TUNNEL_QOS_TRUST_PASS_UP_DSCP_E = 3,

    /** trust outer label MPLS EXP */
    PRV_TGF_TUNNEL_QOS_TRUST_MPLS_EXP_E     = 4,

    /** untrust packet QoS */
    PRV_TGF_TUNNEL_QOS_UNTRUST_PKT_E        = 5

} PRV_TGF_TUNNEL_QOS_MODE_ENT;

/**
* @enum PRV_TGF_TUNNEL_START_TTL_MODE_ENT
 *
 * @brief Enumeration of tunnel start MPLS LSR TTL operation.
*/
typedef enum{

    /** set TTL to incoming TTL Assignment */
    PRV_TGF_TUNNEL_START_TTL_TO_INCOMING_TTL_E         = 0,

    /** set TTL to the TTL of the swapped label */
    PRV_TGF_TUNNEL_START_TTL_TO_SWAP_LABEL_E           = 1,

    /** set TTL to the TTL of the swapped label - 1 */
    PRV_TGF_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E = 2,

    PRV_TGF_TUNNEL_START_TTL_TO_POP_OUTER_E     = 3

} PRV_TGF_TUNNEL_START_TTL_MODE_ENT;

/**
* @enum PRV_TGF_TUNNEL_GRE_ETHER_TYPE_ENT
 *
 * @brief GRE ether type.
*/
typedef enum{

    /** GRE type 0 - control ether type. */
    PRV_TGF_TUNNEL_GRE0_ETHER_TYPE_E = 0,

    /** GRE type 1 - data ether type. */
    PRV_TGF_TUNNEL_GRE1_ETHER_TYPE_E = 1

} PRV_TGF_TUNNEL_GRE_ETHER_TYPE_ENT;

/**
* @enum PRV_TGF_TUNNEL_ETHERTYPE_TYPE_ENT
 *
 * @brief Ethertype type.
*/
typedef enum{

    /** IPv4 GRE 0 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E = 0,

    /** IPv4 GRE 1 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE1_E = 1,

    /** MPLS Ethernet type for Unicast packets */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MPLS_UNICAST_E = 2,

    /** MPLS Ethernet type for Multicast packets */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MPLS_MULTICAST_E = 3,

    /** @brief Mac in Mac Ethernet type
     *  (APPLICABLE DEVICES: xCat; xCat3; Lion; xCat2; Lion2)
     */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_MIM_E = 4,

    /** TRILL Ethernet type (APPLICABLE DEVICES: Bobcat2; Bobcat3; Aldrin2; Falcon; Caelum; Aldrin; AC3X) */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E = 5,

    /** IPv6 GRE 0 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE0_E = 6,

    /** IPv6 GRE 1 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_IPV6_GRE1_E = 7,

    /** User defined 0 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED0_E = 8,

    /** User defined 1 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED1_E = 9,

    /** User defined 2 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED2_E = 10,

    /** User defined 3 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED3_E = 11,

    /** User defined 4 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED4_E = 12,

    /** User defined 5 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED5_E = 13,

    /** User defined 6 Ethernet type */
    PRV_TGF_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E = 14

} PRV_TGF_TUNNEL_ETHERTYPE_TYPE_ENT;

/**
* @enum PRV_TGF_TTI_EXCEPTION_ENT
 *
 * @brief Enumeration for tunnel termination exceptions.
*/
typedef enum{

    /** @brief IPv4 tunnel termination header error.
     *  IPv4 Tunnel Termination Header Error exception is detected if ANY of
     *  the following criteria are NOT met:
     *  - IPv4 header <Checksum> is correct
     *  - IPv4 header <Version> = 4
     *  - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
     *  - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
     *  - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
     *  - IPv4 header <SIP> != IPv4 header <DIP>
     */
    PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E       = 0,

    /** @brief IPv4 tunnel termination option/fragment error.
     *  An IPv4 Tunnel Termination Option/Fragment error occurs if the packet
     *  has either IPv4 Options or is fragmented.
     *  - IPv4 Options are detected if the IPv4 header <IP Header Len> > 5.
     *  - The packet is considered fragmented if either the IPv4 header flag
     *  <More Fragments> is set, or if the IPv4 header <Fragment offset> > 0.
     *  A tunnel-terminated fragmented packet must be reassembled prior
     *  to further processing of the passenger packet.
     */
    PRV_TGF_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E  = 1,

    /** @brief IPv4 tunnel termination unsupported GRE header error.
     *  An IPv4 Tunnel Termination GRE Header error occurs if the IPv4 header
     *  <IP Protocol> = 47 (GRE) and the 16 most significant bits of the GRE
     *  header are not all 0.
     */
    PRV_TGF_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E    = 2,

    /** @brief An MPLS TTL error is detected if the MPLS TTI rule has a match and
     *  any of the MPLS labels popped by the TTI have reached the value 0.
     *  (APPLICABLE DEVICES: xCat; xCat3; Lion; xCat2; Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E  = 3,

    /** @brief MPLS unsupported error occurs if one of the following:
     *  - The <Tunnel Terminate> field is enabled for an MPLS packet, and the
     *  packet has more than three MPLS labels
     *  - The <MPLS Command> cannot be assigned to the packet, for one of
     *  the following reasons:
     *  - <MPLS Command> requires to Pop more labels than the packet has
     *  (this check is performed only when <Tunnel Terminate> is disabled).
     *  - <MPLS Command> != NOP/Push and packet is not MPLS.
     *  (APPLICABLE DEVICES: xCat; xCat3; Lion; xCat2; Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E  = 4,

    /** @brief TRILL IS IS adjacency exception is detected if there is TCAM miss for
     *  unicast TRILL packet, indicating the packet was received from an invalid adjacency;
     *  the global configurable <TRILL IS IS Adjacency exception Cmd> is applied, and the
     *  respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 0.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E = 5,

    /** @brief TRILL tree adjacency exception is detected if there is TCAM miss for
     *  multi-target TRILL packet, indicating the packet was received from an invalid adjacency;
     *  the global configurable <TRILL Tree Adjacency exception Cmd> is applied, and the
     *  respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 1.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E = 6,

    /** @brief TRILL bad version exception is detected if the packet TRILL header version is greater
     *  than the global configurable <Max TRILL version>, the global configurable
     *  <TRILL bad Version exception Cmd> is applied, and the respective exception counter is icremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 2.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_BAD_VERSION_E = 7,

    /** @brief TRILL hop count exception is detected if the packet TRILL header Hop Count is ZERO,
     *  the global configurable <TRILL Hopcount is Zero exception Cmd> is assigned to the packet,
     *  and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 3.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E = 8,

    /** @brief TRILL options exception is detected if the packet TRILL header includes options
     *  (.i.e., the TRILL header Op-length field is greater than zero), the global configurable
     *  <TRILL Options exception Cmd> is assigned to the packet, and its respective exception
     *  counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 4.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_OPTIONS_E = 9,

    /** @brief TRILL critical hopbyhop exception is detected if the packet TRILL header
     *  includes options and the CHBH bit is set, the global configurable
     *  <TRILL CHBH exception Cmd> is assigned to the packet, and its respective exception
     *  counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 5.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_CHBH_E = 10,

    /** @brief The TRILL standard specifies that all TRILL packets on a link, if tagged, must use
     *  the VLAN-ID as specified by the designated RBridge.
     *  TRILL bad outer VID0 exception is detected if the packet TRILL outer tag VLAN-ID
     *  (Tag0 VLAN-ID) does not equal the global configurable <TRILL Outer Tag0 VID>,
     *  the global configurable <TRILL Bad Outer VID exception Cmd> is assigned to the packet,
     *  and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 6.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E = 11,

    /** @brief TRILL transit Unicast traffic, for which this RBridge is the nexthop, must have
     *  the MAC DA of this RBridge. Otherwise, the packet should be dropped by this RBridge.
     *  This is a normal occurrence if the link is attached to a bridged LAN and there are
     *  more than two peer RBridges on this link.
     *  It is required that the MAC DA�s of this RBridge are configured in the MAC2ME table.
     *  TRILL UC not to me exception is detected if the packet is Unicast TRILL and its
     *  outer MAC DA does not match one of the of the MAC addresses configured in the MAC2ME table,
     *  the global configurable <TRILL UC Not MAC2ME exception Cmd> is assigned to the packet,
     *  and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 7.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E = 12,

    PRV_TGF_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E = 13,

    /** @brief TRILL UC with bad outer DA exception is detected if the TRILL outer MAC DA is UC,
     *  and the TRILL header M-bit is set, the global configurable <TRILL UC bad outer exception Cmd>
     *  is assigned to the packet, and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 9.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E = 14,

    /** @brief TRILL outer UC inner MC exception is detected if the TRILL outer MAC DA is UC,
     *  and the inner DA is MC, the global configurable <TRILL outer UC inner MC exception Cmd>
     *  is assigned to the packet, and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 10.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E = 15,

    /** @brief TRILL MC with bad outer DA exception is detected if the TRILL outer MAC DA
     *  is Multicast, and the TRILL header M-bit is clear, the global configurable
     *  <TRILL MC M-bit exception Cmd> is assigned to the packet, and its respective
     *  exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 8.
     *  (APPLICABLE DEVICES: Lion2)
     *  PRV_TGF_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E -
     *  TRILL MC with bad outer DA is detected if the TRILL header M-bit is set, and the outer
     *  MAC DA is not the global configurable <All-RBridges Multicast address>, the global configurable
     *  <TRILL Bad MC DA exception Cmd> is assigned to the packet, and its respective exception
     *  counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 11.
     *  (APPLICABLE DEVICES: Lion2)
     *  PRV_TGF_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E -
     *  TRILL invalid I-RBID exception is detected if the I-RBID lookup results in
     *  an entry <port> == 0xFFFFF; the global configurable <Trill Invalid I-RBID exception Cmd>
     *  is applied, and the respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +12
     *  (APPLICABLE DEVICES: Lion2)
     *  PRV_TGF_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E -
     *  TRILL invalid E-RBID exception is detected if the E-RBID lookup results in
     *  an entry <port> == 0xFFFFF; the global configurable <Trill Invalid E-RBID exception Cmd>
     *  is applied, and the respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +13
     *  (APPLICABLE DEVICES: Lion2)
     *  PRV_TGF_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E -
     *  TRILL UC Critical Ingress to Egress exception is detected if the E-RBID lookup
     *  results in an entry <port> == 0xFFFFE and TRILL.M == 0 (i.e. the packet is
     *  TRILL Unicast-to-ME), and the TRILL.OptionData.CItE == 1 (i.e. packet has the Critical
     *  Ingress-to-Egress bit set); the global configurable <TRILL UC CItE Option exception Cmd>
     *  is applied, and the respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +14
     *  (APPLICABLE DEVICES: Lion2)
     *  PRV_TGF_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E -
     *  TRILL MC Critical Ingress to Egress exception is detected if the TRILL.M == 0
     *  (i.e. the packet is multi-destination TRILL), and the TRILL.OptionData.CItE == 1
     *  (i.e. packet has the Critical Ingress-to-Egress bit set); the global configurable
     *  <TRILL Multi-target CItE Option exception Cmd> is applied, and the respective exception
     *  counter is icremented.
     *  If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +15.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E = 16,

    PRV_TGF_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E = 21,

    PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E = 22,

    PRV_TGF_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E = 23,

    PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E = 24,

    PRV_TGF_TTI_EXCEPTION_IPV6_HBH_ERROR_E = 25,

    PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E = 26,

    PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E = 27

} PRV_TGF_TTI_EXCEPTION_ENT;

/**
* @enum PRV_TGF_TTI_OFFSET_TYPE_ENT
 *
 * @brief Offset types for packet headers parsing used for user defined
 * bytes configuration
*/
typedef enum{

    /** offset from start of mac(L2) header */
    PRV_TGF_TTI_OFFSET_L2_E = 0,

    /** offset from start of L3 header minus 2. */
    PRV_TGF_TTI_OFFSET_L3_MINUS_2_E = 1,

    /** offset from start of L4 header */
    PRV_TGF_TTI_OFFSET_L4_E = 2,

    /** offset from Ethertype of MPLS minus 2. */
    PRV_TGF_TTI_OFFSET_MPLS_MINUS_2_E = 3,

    /** @brief offset from TTI internal descriptor
     *  PRV_TGF_TTI_OFFSET_INVALID_E     invalid UDB contains 0 always
     */
    PRV_TGF_TTI_OFFSET_METADATA_E = 4,

    PRV_TGFH_TTI_OFFSET_INVALID_E = 5

} PRV_TGF_TTI_OFFSET_TYPE_ENT;

/**
* @enum PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT
 *
 * @brief TTI key TCAM segment modes
*/
typedef enum{

    /** @brief All the TTI TCAM block sets are treated as a single logical TCAM, and return a single
     *  action associated with the first match in the combined TCAM block sets.
     *  Relevant for devices that support dual lookup or quad lookup
     */
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E = 0,

    /** @brief The TTI TCAM block set 0 is treated as a single logical TCAM, and returns an action
     *  associated with the first match in TCAM block set 0.
     *  - The TTI TCAM block set 1 is treated as a single logical TCAM, and returns an action
     *  associated with the first match in TCAM block set 1. This action is only applied if
     *  the previously assigned action <Continue to Next TTI Lookup> is set.
     *  Relevant only for devices that support dual lookup
     */
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E = 1,

    /** @brief The TTI TCAM block sets 0 and 1 are treated as a single logical TCAM and returns an
     *  action associated with the first match in the combined TCAM block sets 0 and 1.
     *  - The TTI TCAM block sets 2 and 3 are treated as a single logical TCAM and returns an
     *  action associated with the first match in the combined TCAM block sets 2 and 3. This
     *  action is only applied if the previously applied action <Continue to Next TTI Lookup>
     *  is set.
     *  Relevant only for devices that support quad lookup
     */
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E = 2,

    /** @brief The TTI TCAM block sets 0 is treated as a single logical TCAM and returns an action
     *  associated with the first match in the TCAM block set 0.
     *  - The TTI TCAM block sets 1, 2, and 3 are treated as a single logical TCAM and returns
     *  an action associated with the first match in the combined TCAM block sets 1, 2, and 3.
     *  This action is only applied if the previously applied action <Continue to Next TTI Lookup>
     *  is set.
     *  Relevant only for devices that support quad lookup
     */
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E = 3,

    /** @brief The TTI TCAM block sets 0, 1, and 2 are treated as a single logical TCAM and returns
     *  an action associated with the first match in the combined TCAM block sets 0, 1, and 2.
     *  - The TTI TCAM block sets 3 is treated as a single logical TCAM and returns an action
     *  associated with the first match in the TCAM block set 3. This action is only applied
     *  if the previously applied action <Continue to Next TTI Lookup> is set.
     *  Relevant only for devices that support quad lookup
     */
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E = 4,

    /** @brief The TTI TCAM block set 0 is treated as a single logical TCAM and returns an action
     *  associated with the first match in the TCAM block set 0.
     *  - The TTI TCAM block set 1 is treated as a single logical TCAM and returns an action
     *  associated with the first match in the TCAM block set 1. This action is only applied
     *  if the previously applied action <Continue to Next TTI Lookup> is set.
     *  - The TTI TCAM block set 2 is treated as a single logical TCAM and returns an action
     *  associated with the first match in the TCAM block set 2. This action is only applied
     *  if the previously applied action <Continue to Next TTI Lookup> is set.
     *  - The TTI TCAM block set 3 is treated as a single logical TCAM and returns an action
     *  associated with the first match in the TCAM block set 3. This action is only applied
     *  if the previously applied action <Continue to Next TTI Lookup> is set.
     *  Relevant only for devices that support quad lookup
     */
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E = 5

} PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT;

/**
* @struct PRV_TGF_TTI_TRILL_ADJACENCY_STC
 *
 * @brief The TRILL engine uses a single TCAM lookup to implement
 * the following TRILL adjacency checks:
 * 1. TRILL IS-IS Adjacency check -
 * Checks that the single-destination TRILL frame arrives from a
 * (neighbor, port) for which an IS-IS adjacency exists.
 * 2. TRILL Tree Adjacency Check -
 * Checks that the multi-destination TRILL frame arrives from a
 * (neighbor, port) that is a branch on the given TRILL distribution tree.
 * A TCAM lookup is performed for every TRILL packet processed by the
 * TRILL engine.
*/
typedef struct{

    /** The outer MAC source address */
    GT_ETHERADDR outerMacSa;

    /** The TRILL header M */
    GT_U32 trillMBit;

    /** The TRILL header Egress RBridge Nickname (16 bits) */
    GT_U32 trillEgressRbid;

    /** local source HW device or DSA tag source device (12 bits) */
    GT_HW_DEV_NUM srcHwDevice;

    /** @brief whether source is port or trunk (0..1)
     *  0: Source is not a trunk (i.e. it is port)
     *  1: Source is a trunk
     */
    GT_U32 srcIsTrunk;

    /** @brief Source port or Source trunk
     *  if port, range (20 bits)
     *  if trunk, range (12 bits)
     */
    GT_U32 srcPortTrunk;

} PRV_TGF_TTI_TRILL_ADJACENCY_STC;

/**
* @struct PRV_TGF_TTI_TRILL_RBID_TABLE_ENTRY_STC
 *
 * @brief The RBID engine consists of the RBID Lookup Translation Table and RBID Table.
 * For each TRILL packet, the RBID table is accessed twice: once with the index
 * set to the TRILL.I-RBID, and once with the index set to the TRILL.E-RBID.
*/
typedef struct{

    /** @brief Ingress RBridge lookup: Source port associated with Ingress RBridge
     *  Egress RBridge lookup: Target port associated with Egress RBridge
     */
    GT_PORT_NUM srcTrgPort;

    /** Target HW device associated with Egress RBridge */
    GT_HW_DEV_NUM trgHwDevice;

} PRV_TGF_TTI_TRILL_RBID_TABLE_ENTRY_STC;

/**
* @struct PRV_TGF_TTI_RULE_COMMON_STC
 *
 * @brief TTI Common fields in TCAM rule
*/
typedef struct{

    /** PCL id */
    GT_U32 pclId;

    /** whether source is port or trunk */
    GT_BOOL srcIsTrunk;

    /** Source port or Source trunk */
    GT_U32 srcPortTrunk;

    /** MAC DA or MAC SA of the tunnel header */
    GT_ETHERADDR mac;

    /** packet VID assignment */
    GT_U16 vid;

    /** whether the external mac is considered tagged on not */
    GT_BOOL isTagged;

    /** whether sourse is Trunk or not */
    GT_BOOL dsaSrcIsTrunk;

    /** DSA tag source port or trunk */
    GT_U32 dsaSrcPortTrunk;

    /** DSA tag source device */
    GT_U32 dsaSrcDevice;

    /** @brief Indicates the port group where the packet entered the device.
     *  Defined only for MPLS lookup.
     *  (Lion and above for multi port groups devices only)
     */
    GT_U32 sourcePortGroupId;

} PRV_TGF_TTI_RULE_COMMON_STC;

/*
 * Typedef: struct PRV_TGF_TTI_RULE_COMMON_DEFAULT_INFO_STC
 *
 * Description:
 *          TTI default info for Common fields in TCAM rule - subset of PRV_TGF_TTI_RULE_COMMON_STC
 */
typedef struct
{
    GT_U32                  srcPort;
    GT_ETHERADDR            mac;
    GT_U16                  vid;
} PRV_TGF_TTI_RULE_COMMON_DEFAULT_INFO_STC;

/**
* @struct PRV_TGF_TTI_IPV4_RULE_STC
 *
 * @brief TTI IPv4 TCAM rule
*/
typedef struct{

    PRV_TGF_TTI_RULE_COMMON_STC common;

    /** tunneling protocol of the packet */
    GT_U32 tunneltype;

    /** tunnel source IP */
    GT_IPADDR srcIp;

    /** tunnel destination IP */
    GT_IPADDR destIp;

    /** whether the packet is ARP or not ARP */
    GT_BOOL isArp;

} PRV_TGF_TTI_IPV4_RULE_STC;

/*
 * Typedef: struct PRV_TGF_TTI_IPV4_RULE_DEFAULT_INFO_STC
 *
 * Description:
 *          TTI IPv4 default info for TCAM rule - subset of PRV_TGF_TTI_IPV4_RULE_STC
 */
typedef struct
{
    PRV_TGF_TTI_RULE_COMMON_DEFAULT_INFO_STC common;
    GT_IPADDR                                srcIp;
    GT_IPADDR                                destIp;
} PRV_TGF_TTI_IPV4_RULE_DEFAULT_INFO_STC;

/**
* @struct PRV_TGF_TTI_MPLS_RULE_STC
 *
 * @brief TTI MPLS TCAM rule
*/
typedef struct{

    PRV_TGF_TTI_RULE_COMMON_STC common;

    /** MPLS label 0 */
    GT_U32 label0;

    /** EXP of MPLS label 0 */
    GT_U32 exp0;

    /** MPLS label 1 */
    GT_U32 label1;

    /** EXP of MPLS label 1 */
    GT_U32 exp1;

    /** MPLS label 2 */
    GT_U32 label2;

    /** EXP of MPLS label 2 */
    GT_U32 exp2;

    /** The number of MPLS labels in the label stack */
    GT_U32 numOfLabels;

    /** The protocol above MPLS */
    GT_U32 protocolAboveMPLS;

} PRV_TGF_TTI_MPLS_RULE_STC;

/**
* @struct PRV_TGF_TTI_ETH_RULE_STC
 *
 * @brief TTI Ethernet TCAM rule
*/
typedef struct{

    PRV_TGF_TTI_RULE_COMMON_STC common;

    /** user priority as found in the outer VLAN tag header */
    GT_U32 up0;

    /** CFI (Canonical Format Indicator) */
    GT_U32 cfi0;

    /** whether the fields: VLAN */
    GT_BOOL isVlan1Exists;

    /** VLAN ID of vlan1 */
    GT_U16 vid1;

    /** user priority of vlan1 */
    GT_U32 up1;

    /** CFI (Canonical Format Indicator) of vlan1 */
    GT_U32 cfi1;

    /** ether type */
    GT_U32 etherType;

    /** if there was a match in MAC2ME table */
    GT_BOOL macToMe;

} PRV_TGF_TTI_ETH_RULE_STC;

/**
* @struct PRV_TGF_TTI_MIM_RULE_STC
 *
 * @brief TTI MIM TCAM rule
*/
typedef struct{

    PRV_TGF_TTI_RULE_COMMON_STC common;

    /** backbone user priority assigned from vlan header */
    GT_U32 bUp;

    /** backbone drop precedence assigned from vlan header */
    GT_U32 bDp;

    /** inner service instance assigned from tag information */
    GT_U32 iSid;

    /** inner user priority assigned from tag information */
    GT_U32 iUp;

    /** inner drop precedence assigned from tag information */
    GT_U32 iDp;

    /** 2 reserved bits */
    GT_U32 iRes1;

    /** 2 reserved bits */
    GT_U32 iRes2;

    /** Packet is identified as MACtoME. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    GT_BOOL macToMe;

    /** @brief GT_TRUE: Passenger Ethernet packet contains a VLAN Tag
     *  GT_FALSE:Passenger Ethernet packet does not contains a VLAN Tag
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL passengerPacketOuterTagExists;

    /** @brief if passengerPacketOuterTagExists=GT_TRUE this
     *  field contain the passenger packet outer tag VLAN_ID
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 passengerPacketOuterTagVid;

    /** @brief if passengerPacketOuterTagExists=GT_TRUE this
     *  field contain the passenger packet outer tag UP
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 passengerPacketOuterTagUp;

    /** @brief if passengerPacketOuterTagExists=GT_TRUE this
     *  field contain the passenger packet outer tag DEI
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 passengerPacketOuterTagDei;

} PRV_TGF_TTI_MIM_RULE_STC;

/**
* @struct PRV_TGF_TTI_TRILL_RULE_STC
 *
 * @brief TTI TRILL TCAM rule
*/
typedef struct{

    /** @brief TTI Common fields in TCAM rule.
     *  common.srcIsTrunk      - If packet arrived DSA-tagged && physical port <Centralized Chassis> ==1
     *  this field is always 0.
     *  If packet arrived DSA-tagged && physical port <Centralized Chassis> ==0
     *  this is the DSA tag <Src Is Trunk>.
     *  If the packet arrived non-DSA, this bit is set if the default port
     *  <Trunk-ID> != 0; otherwise it is clear.
     *  common.srcPortTrunk     - If the packet arrived DSA-tagged && physical port <Centralized Chassis> == 0
     *  this is the DSA tag <source Trunk-ID/Port>.
     *  If the packet arrived DSA-tagged && physical port <Centralized Chassis> ==1
     *  this is the Centralized Chassis default port mapped from the DSA <SrcTrunk-ID/Port>.
     *  If packet arrived non-DSA tagged:
     *  If default port <Trunk-ID> != 0, this is the default port <trunk-ID>.
     *  If default port <Trunk-ID> == 0, this is the physical port <Default port>.
     *  common.dsaSrcDevice     - If packet arrived DSA-tagged && physical port <Centralized Chassis> ==1
     *  this is the local Device.
     *  If packet arrived DSA-tagged && physical port <Centralized Chassis> ==0
     *  this is the DSA tag <SrcDev>.
     *  If not-DSA, this is the local Device.
     *  common.isTagged       - Not Relevant
     *  common.dsaSrcIsTrunk    - Not Relevant
     *  common.dsaSrcPortTrunk   - Not Relevant
     */
    PRV_TGF_TTI_RULE_COMMON_STC common;

    /** TRILL header Multi */
    GT_BOOL trillMBit;

    /** TRILL Egress RBID (16 bits) */
    GT_U32 trillEgressRbid;

    /** TRILL Ingress RBID (16 bits) */
    GT_U32 trillIngressRbid;

    /** The inner packet MAC DA */
    GT_ETHERADDR innerPacketMacDa;

    /** @brief 1: Tag0 found in inner packet
     *  0: Tag0 not found in inner packet
     */
    GT_BOOL innerPacketTag0Exists;

    /** The inner packet Tag0 VID classification (12 bits) */
    GT_U32 innerPacketTag0Vid;

    /** @brief innerPacketMacDa, innerPacketTag0Exists and innerPacketTag0Vid
     *  fields are valid.
     *  NOTE: the inner packet fields are not valid when
     *  the option data exceed 40B
     */
    GT_BOOL innerPacketFieldsAreValid;

    /** @brief All multi
     *  The 1st instance has key field assignment
     *  <TRILL MC Desc Instance> = 0
     *  The 2nd instance has key field assignment
     *  <TRILL MC Desc Instance> = 1
     *  NOTE: this indication is not from the packet
     */
    GT_BOOL trillMcDescriptorInstance;

} PRV_TGF_TTI_TRILL_RULE_STC;

/**
* @union PRV_TGF_TTI_RULE_UNT
 *
 * @brief Union for TTI rule
 *
*/

typedef union{
    /** ipv4 rule */
    PRV_TGF_TTI_IPV4_RULE_STC ipv4;

    /** MPLS rule */
    PRV_TGF_TTI_MPLS_RULE_STC mpls;

    /** ethernet rule */
    PRV_TGF_TTI_ETH_RULE_STC eth;

    /** Mac in Mac rule */
    PRV_TGF_TTI_MIM_RULE_STC mim;

    /** trill rule */
    PRV_TGF_TTI_TRILL_RULE_STC trill;

    GT_U8 udbArray[PRV_TGF_TTI_MAX_UDB_CNS];

} PRV_TGF_TTI_RULE_UNT;


/**
* @struct PRV_TGF_TTI_ACTION_STC
 *
 * @brief TTI TCAM rule action parameters
*/
typedef struct
{
    CPSS_PACKET_CMD_ENT                         command;
    PRV_TGF_TTI_REDIRECT_COMMAND_ENT            redirectCommand;
    CPSS_NET_RX_CPU_CODE_ENT                    userDefinedCpuCode;
    GT_BOOL                                     tunnelTerminate;
    PRV_TGF_TTI_PASSENGER_TYPE_ENT              passengerPacketType;
    GT_BOOL                                     copyTtlFromTunnelHeader;
    GT_BOOL                                     mirrorToIngressAnalyzerEnable;
    GT_U32                                      policerIndex;
    CPSS_INTERFACE_INFO_STC                     interfaceInfo;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT vlanPrecedence;
    PRV_TGF_TTI_VLAN_COMMAND_ENT                vlanCmd;
    GT_U16                                      vlanId;
    GT_BOOL                                     nestedVlanEnable;

    GT_BOOL                                     tunnelStart;
    GT_U32                                      tunnelStartPtr;
    GT_U32                                      routerLookupPtr;
    GT_U32                                      vrfId;
    GT_BOOL                                     sourceIdSetEnable;
    GT_U32                                      sourceId;
    GT_BOOL                                     bindToPolicer;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;
    PRV_TGF_TTI_QOS_MODE_TYPE_ENT               qosTrustMode;
    GT_U32                                      qosProfile;
    PRV_TGF_TTI_MODIFY_UP_ENT                   modifyUpEnable;
    PRV_TGF_TTI_MODIFY_DSCP_ENT                 modifyDscpEnable;
    GT_U32                                      up;
    GT_BOOL                                     remapDSCP;
    GT_BOOL                                     vntl2Echo;
    GT_BOOL                                     bridgeBypass;
    GT_BOOL                                     actionStop;
    GT_BOOL                                     activateCounter;
    GT_U32                                      counterIndex;

    GT_U32                                      arpPtr;
    GT_U32                                      ipNextHopIndex;
    PRV_TGF_TTI_MPLS_COMMAND_ENT                mplsCmd;
    GT_U32                                      exp;
    GT_BOOL                                     setExp;
    GT_BOOL                                     enableDecTtl;
    GT_U32                                      mplsLabel;
    GT_U32                                      ttl;
    GT_U32                                      pwId;
    GT_BOOL                                     sourceIsPE;
    GT_BOOL                                     enableSourceLocalFiltering;
    GT_U32                                      floodDitPointer;
    GT_U32                                      baseMplsLabel;
    PRV_TGF_TTI_PW_ID_MODE_ENT                  pwIdMode;
    GT_BOOL                                     counterEnable;
    GT_BOOL                                     meterEnable;
    GT_U32                                      flowId;
    GT_BOOL                                     sstIdEnable;
    GT_U32                                      sstId;

    PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT           tsPassengerPacketType;
    PRV_TGF_TTI_VLAN_COMMAND_ENT                tag0VlanCmd;
    GT_U16                                      tag0VlanId;
    PRV_TGF_TTI_VLAN1_COMMAND_ENT               tag1VlanCmd;
    GT_U16                                      tag1VlanId;
    GT_BOOL                                     keepPreviousQoS;
    GT_BOOL                                     trustUp;
    GT_BOOL                                     trustDscp;
    GT_BOOL                                     trustExp;
    PRV_TGF_TTI_TAG1_UP_COMMAND_ENT             tag1UpCommand;
    GT_U32                                      tag0Up;
    GT_U32                                      tag1Up;
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT         pcl0OverrideConfigIndex;
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT         pcl0_1OverrideConfigIndex;
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT         pcl1OverrideConfigIndex;
    GT_U32                                      iPclConfigIndex;
    GT_BOOL                                     bindToCentralCounter;
    GT_U32                                      centralCounterIndex;
    GT_BOOL                                     ingressPipeBypass;

} PRV_TGF_TTI_ACTION_STC;

/**
* @struct PRV_TGF_TTI_ACTION_2_STC
 *
 * @brief TTI TCAM rule action parameters (standard action)
*/
typedef struct{

    /** whether the packet's tunnel header is removed */
    GT_BOOL tunnelTerminate;

    /** passenger packet type */
    PRV_TGF_TTI_PASSENGER_TYPE_ENT ttPassengerPacketType;

    /** type of passenger packet */
    PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT tsPassengerPacketType;

    /** @brief If <GT_TRUE> and the packet is MPLS
     *  (<MPLS Command> = POP and <MPLS TTL> = 0, or
     *  <Tunnel Terminate>=Enabled),
     *  the TTL (for Bobcat2, Caelum, Bobcat3, Aldrin, AC3X: also EXP) is
     *  copied from the last popped label.
     *  If <GT_TRUE> and the packet is X-over-IPv4
     *  tunnel-termination, and the passenger is IPv4/6,
     *  the TTL is copied from the IPv4 tunnel header
     *  rather than from passenger packet.
     */
    GT_BOOL copyTtlExpFromTunnelHeader;

    /** MPLS Action applied to the packet */
    PRV_TGF_TTI_MPLS_COMMAND_ENT mplsCommand;

    /** TTL is taken from the tunnel or inner MPLS label */
    GT_U32 mplsTtl;

    /** exp */
    GT_U32 exp;

    /** @brief whether to set exp
     *  enableDecTtl     - enable/disable decrement TTL
     */
    GT_BOOL setExp;

    /** MPLS label (20 bits) */
    GT_U32 mplsLabel;

    /** TTL (8 bits) */
    GT_U32 ttl;

    /** enable/disable TTL decremented */
    GT_BOOL enableDecrementTtl;

    /** forwarding command */
    CPSS_PACKET_CMD_ENT command;

    /** where to redirect the packet */
    PRV_TGF_TTI_REDIRECT_COMMAND_ENT redirectCommand;

    /** egress interface to redirect packet */
    CPSS_INTERFACE_INFO_STC egressInterface;

    /** @brief next hop to redirect; relevant if redirect command
     *  is to redirect to next hop (16 bits)
     */
    GT_U32 ipNextHopIndex;

    /** pointer to the ARP entry for MPLS routed packets */
    GT_U32 arpPtr;

    /** redirected to Tunnel/non */
    GT_BOOL tunnelStart;

    /** pointer to the Tunnel Start entry */
    GT_U32 tunnelStartPtr;

    /** pointer to the Router Lookup Translation Table entry */
    GT_U32 routerLttPtr;

    /** VRF */
    GT_U32 vrfId;

    /** if set, then the <sourceId> is assign to the packet */
    GT_BOOL sourceIdSetEnable;

    /** source ID assigned to the packet */
    GT_U32 sourceId;

    /** tag0 vlan command */
    PRV_TGF_TTI_VLAN_COMMAND_ENT tag0VlanCmd;

    /** tag0 VLAN */
    GT_U16 tag0VlanId;

    /** tag1 vlan command */
    PRV_TGF_TTI_VLAN1_COMMAND_ENT tag1VlanCmd;

    /** tag1 VLAN */
    GT_U16 tag1VlanId;

    /** @brief whether the VID assignment can be overridden by
     *  subsequent VLAN assignment mechanism
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT tag0VlanPrecedence;

    /** enable/disable nested vlan */
    GT_BOOL nestedVlanEnable;

    /** If set, the packet is bound to the policer meter */
    GT_BOOL bindToPolicerMeter;

    /** If set, the packet is bound to the policer/billing/IPFIX */
    GT_BOOL bindToPolicer;

    /** Traffic profile to be used if the <bindToPolicer> is set */
    GT_U32 policerIndex;

    /** whether QoS profile can be overridden by subsequent pipeline */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;

    /** @brief assign QoS profile based on QoS fields
     *  keepPreviousQoS disabled
     *  QoS Profile trustUp enabled
     */
    GT_BOOL keepPreviousQoS;

    /** do not trust packet�s L2 QoS */
    GT_BOOL trustUp;

    /** do not trust packet�s L3 QoS */
    GT_BOOL trustDscp;

    /** do not trust packet�s EXP */
    GT_BOOL trustExp;

    /** QoS profile to assign to the packet */
    GT_U32 qosProfile;

    /** modify tag0 UP */
    PRV_TGF_TTI_MODIFY_UP_ENT modifyTag0Up;

    /** tag1 UP command */
    PRV_TGF_TTI_TAG1_UP_COMMAND_ENT tag1UpCommand;

    /** modify DSCP mode */
    PRV_TGF_TTI_MODIFY_DSCP_ENT modifyDscp;

    /** tag0 UP assignment */
    GT_U32 tag0Up;

    /** tag1 UP assignment */
    GT_U32 tag1Up;

    /** packet�s DSCP is not remapped */
    GT_BOOL remapDSCP;

    /** @brief controls the index used for IPCL0 lookup
     *  pcl0O1verrideConfigIndex - controls the index used for IPCL0-1 lookup
     */
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0OverrideConfigIndex;

    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0_1OverrideConfigIndex;

    /** controls the index used for IPCL1 lookup */
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT pcl1OverrideConfigIndex;

    /** pointer to IPCL configuration entry to be used */
    GT_U32 iPclConfigIndex;

    /** packet is mirrored to the Ingress Analyzer port */
    GT_BOOL mirrorToIngressAnalyzerEnable;

    /** user defined cpu code */
    CPSS_NET_RX_CPU_CODE_ENT userDefinedCpuCode;

    /** enable/disable central counter binding */
    GT_BOOL bindToCentralCounter;

    /** central counter bound to this entry */
    GT_U32 centralCounterIndex;

    /** if set, the packet MAC SA/DA are switched on the transmitting */
    GT_BOOL vntl2Echo;

    /** if set, the packet isn't subject to any bridge mechanisms */
    GT_BOOL bridgeBypass;

    /** if set, the ingress engines are all bypassed for this packet */
    GT_BOOL ingressPipeBypass;

    /** if set, the packet will not have any searches in the IPCL mechanism */
    GT_BOOL actionStop;

    /** @brief Determines which mask is used in the CRC based hash.
     *  0: do not override hash mask index.
     *  Other values: override the hash mask index value.
     *  (Lion and above devices only)
     */
    GT_U32 hashMaskIndex;

    /** @brief If set, the packet is marked for MAC SA modification
     *  Relevant only when <redirectCommand> == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     *  (Lion and above devices only)
     */
    GT_BOOL modifyMacSa;

    /** @brief If set, the packet is marked for MAC DA modification,
     *  and the <ARP Index> specifies the new MAC DA.
     *  Relevant only when <redirectCommand> == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     *  (Lion and above devices only)
     */
    GT_BOOL modifyMacDa;

    /** @brief If set, the source port group ID of the packet is set to the current port group.
     *  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     *  (Lion and above for multi port groups devices only)
     */
    GT_BOOL ResetSrcPortGroupId;

    /** @brief If set, the packet is forwarded to a ring port for another TTI lookup.
     *  Relevant only when <redirectCommand> == CPSS_DXCH_TTI_NO_REDIRECT_E
     *  (Lion and above for multi port groups devices only)
     */
    GT_BOOL multiPortGroupTtiEnable;

    /** @brief Enable assignment of Source ePort number from this entry
     *  GT_FALSE- Disabled
     *  GT_TRUE - Enabled - New source ePort number
     *  is taken from TTI Action Entry<Source ePort>.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL sourceEPortAssignmentEnable;

    /** @brief Source ePort number assigned by TTI entry
     *  when <Source ePort Assignment Enable> = Enabled
     *  NOTE: Overrides any previous assignment of
     *  source ePort number.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_PORT_NUM sourceEPort;

    /** PW ID (7 bits) */
    GT_U32 pwId;

    /** whether source is PE (provider edge) or customer edge */
    GT_BOOL sourceIsPE;

    /** @brief GT_TRUE: filter packets whose assigned inlif
     *  id is the same as inlif id from VPLS forwarding
     *  lookup action
     *  GT_FALSE: disable the filtering
     */
    GT_BOOL enableSourceLocalFiltering;

    /** @brief Dit Pointer assigned to packet by VPLS Bridge
     *  when packet's DA is unknown
     */
    GT_U32 floodDitPointer;

    /** base MPLS label bits (20 bits) */
    GT_U32 baseMplsLabel;

    /** PW */
    PRV_TGF_TTI_PW_ID_MODE_ENT pwIdMode;

    /** enable/disable packet countering according to the traffic profile */
    GT_BOOL counterEnable;

    /** enable/disable metering traffic according to policer profile */
    GT_BOOL meterEnable;

    /** determines the Flow ID of the packet (14 bits) */
    GT_U32 flowId;

    /** If set, the Source Id is taken from the <sstId> field */
    GT_BOOL sstIdEnable;

    /** Source ID (5 bits) */
    GT_U32 sstId;

    /** @brief If '0' then the TT Header Len is default TT
     *  algorithm for each key type
     *  Else this is the Tunnel header length in units of Bytes.
     *  Granularity is in 2 Bytes.
     *  Note: Tunnel header begins at the start of the L3
     *  header, i.e. immediately after the EtherType field
     *  This field is relevant if TTI Action <Tunnel Terminate> = Enabled
     *  or TTI Action <Passenger Parsing of Non-MPLS Transit Tunnels Enable> = Enabled
     *  (APPLICABLE RANGES: 0..31) (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    GT_U32 ttHeaderLength;

    /** @brief generic mechanism to parse
     *  the passenger of transit tunnel packets over MPLS.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT passengerParsingOfTransitMplsTunnelMode;

    /** @brief generic mechanism to parse
     *  the passenger of transit tunnel packets other than MPLS,
     *  e.g. IP-based tunnels, MiM tunnels, etc.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL passengerParsingOfTransitNonMplsTransitTunnelEnable;

    GT_BOOL cwBasedPw;

    GT_BOOL applyNonDataCwCommand;

    /** Enables next TTI lookup (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    GT_BOOL continueToNextTtiLookup;

    /** @brief ingress analyzer index
     *  (APPLICABLE RANGES: 0..6) lookup (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     *  Note: relevant when < mirrorToIngressAnalyzerEnable > = Enabled
     */
    GT_U32 mirrorToIngressAnalyzerIndex;

    /** @brief Enable Rx Protection Switching
     *  GT_FALSE- Disable Rx Protection Switching
     *  GT_TRUE - Enable Rx Protection Switching
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL rxProtectionSwitchEnable;

    /** @brief whether rx is protection path
     *  GT_FALSE- Packet arrived on Working Path
     *  GT_TRUE - Packet arrived on Protection Path
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL rxIsProtectionPath;

    GT_BOOL setMacToMe;

    /** @brief enables use of user priority
     *  (APPLICABLE RANGES: 0..7) to select
     *  a mapping table when set to GT_TRUE
     *  Note: relevant when
     */
    GT_BOOL qosUseUpAsIndexEnable;

    /** @brief mapping table index
     *  (APPLICABLE RANGES: 0..11)
     */
    GT_U32 qosMappingTableIndex;

    /** indicates whether iPclUdbConfigTableIndex relevant or not */
    GT_BOOL iPclUdbConfigTableEnable;

    /** @brief sip5 only: value of UDE index for iPcl
     *  (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_PCL_PACKET_TYPE_ENT iPclUdbConfigTableIndex;

    /** @brief enables TTL expiration exception command assignment for Pseudo Wire
     *  0 = Disable: No PW label TTL check is performed
     *  1 = Enable: The PW label TTL is checked if it is 0 or 1, and if so
     *  an exception is triggered
     */
    GT_BOOL ttlExpiryVccvEnable;

    /** @brief if set, indicates that this PW supports flow hash label, and thus <PW_LABEL>
     *  is not at the bottom of the MPLS label stack, instead it is one label above
     *  the bottom of the label stack (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  pwCwBasedETReeEnable  - when set, indicates that this PW-CW supports E-Tree Root/Leaf indication
     *  via Control Word <L> bit
     *  0 = Disable
     *  1 = Enable
     */
    GT_BOOL pwe3FlowLabelExist;

    GT_BOOL pwCwBasedETreeEnable;

    GT_BOOL oamProcessEnable;

    GT_U32 oamProfile;

    GT_BOOL oamChannelTypeToOpcodeMappingEnable;

    GT_BOOL unknownSaCommandEnable;

    CPSS_PACKET_CMD_ENT unknownSaCommand;

    GT_BOOL sourceMeshIdSetEnable;

    GT_U32 sourceMeshId;

    /** @brief GT_TRUE - tunnel Header Starts at L4.
    *  GT_FALSE - tunnel Header starts at L3 ,
    *  TTI Action<Tunnel Header Length> is relative to the start of
    *  the L3 or L4 by this value .
    *  (APPLICABLE DEVICES: Falcon)
    */
    GT_BOOL tunnelHeaderStartL4Enable;

    /** @brief GT_TRUE - Skip FDB SA lookup.
    *   GT_FALSE - Don't modify the state of Skip FDB SA lookup.
    *  (APPLICABLE DEVICES: Falcon)
    */
    GT_BOOL skipFdbSaLookupEnable;

    /** @brief Sets that this Device is an End Node of IPv6 Segment .
    *  (APPLICABLE DEVICES: Falcon)
    */
    GT_BOOL ipv6SegmentRoutingEndNodeEnable;


    /** @brief GT_TRUE - The Exact Match action has a priority over
    *          the TTI action.
    *  GT_FALSE - The Exact Match action doesn't have a priority
    *  over the TTI action.
    *  (APPLICABLE DEVICES: Falcon)
    */
    GT_BOOL exactMatchOverTtiEn;


} PRV_TGF_TTI_ACTION_2_STC;

/**
* @union PRV_TGF_TTI_ACTION_UNT
 *
 * @brief Union for TTI action.
 *
*/

typedef union{
    /** type1 action */
    PRV_TGF_TTI_ACTION_STC type1;

    /** type2 action */
    PRV_TGF_TTI_ACTION_2_STC type2;

} PRV_TGF_TTI_ACTION_UNT;





/**
* @enum PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT
 *
 * @brief Enumeration of tunnel start template data size.
 * This enumerator is determined value of template data size
 * in Generic IP Tunnel start Profile table entry.
 * Based on the template data size, the Profile table entry template generates up to 16
 * bytes of data after the 4B GRE header or after the 8B UDP header.
*/
typedef enum{

    /** No data is generated by this profile template; */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E,

    /** 4 byte data is generated by this profile template. */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E,

    /** 8 byte data is generated by this profile template. */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E,

    /** 12 byte data is generated by this profile template. */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E,

    /** 16 byte data is generated by this profile template. */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E

} PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT;



/**
* @enum PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_ENT
 *
 * @brief Enumeration of tunnel start template data configuration.
 * It describes configuration options for each bit of template data.
 * The template is used to generate up to 16B (128 bits) of GRE extension
 * data or UPD shim header.
 * So there are 128 instances of this field, one for each bit generated. It is
 * repeated for bytes y = 0 to y = 15, and for each byte y it is repeated for bits
 * x = 0 to x = 7 (bit 0 is the least significant bit of the byte).
*/
typedef enum{

    /** the relevant bit value should be constant 0. */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E,

    /** the relevant bit value should be constant 1. */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 0 of ePort <TS Extension> .
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_0_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 1 of ePort <TS Extension> .
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_1_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 2 of ePort <TS Extension> .
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_2_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 3 of ePort <TS Extension> .
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_3_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 0 of eVLAN <ServiceID>.
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 1 of eVLAN <ServiceID>.
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 2 of eVLAN <ServiceID>.
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 0 of the ShiftedHash value.
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_0_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 1 of the ShiftedHash value.
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_1_E

} PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_ENT;



/**
* @enum PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of udp source port mode .
 * This enumerator describes Tunnel-start UDP source port
 * assignment options.
*/
typedef enum{

    /** @brief UDP header
     *  source port is assigned an explicit value from the IP Generic TS entry <UDP Source Port>.
     */
    PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E,

    /** @brief UDP header
     *  source port is assigned with the packet hash value.
     */
    PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E

} PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT;


/**
* @struct PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC
 *
 * @brief Struct of configuration for Generic IP Tunnel start Profile table entry.
 * serves as logical extension to the Generic IPv4 Tunnel-Start and
 * Generic IPv6 Tunnel-Start when the IPv4/6 tunnel header includes
 * either a GRE or UDP header.
*/
typedef struct{

    /** @brief template data size. Template data is used to generate up to 16Byte.
     *  Template bytes are concatenated in the following order: Byte0,
     *  Byte1,..., Byte15, while each byte format is: (Bit7, Bit6,..., Bit1, Bit0).
     */
    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT templateDataSize;

    PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_ENT templateDataBitsCfg[PRV_TGF_TUNNEL_START_TEMPLATE_DATA_MAX_SIZE_CNS];

    /** @brief The number of bits to circular shift left the internal 12
     *  value. (APPLICABLE RANGES: 0...7). The result is a 16-bit Shifted-Hash value.
     */
    GT_U32 hashShiftLeftBitsNumber;

    /** The Tunnel */
    PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT udpSrcPortMode;

    /** @brief The number of bits to circularly shift the eVLAN attribute <Service
     *  The result is the Shifted Service-ID value.
     *  (APPLICABLE RANGES: 0...23),Values 24-31 are reserved
     *  This value is used for the Profile entry <MAC DA mode> and/or
     *  Profile entry <Destination IP mode> if the respective field is
     *  configured to use the shifted Service-ID in its LS bits.
     *  This shifted Service-ID is not used by the Profile template header generation.
     */
    GT_U32 serviceIdCircularShiftSize;

    /** @brief 0 = Set MAC DA according to TS entry <MAC DA>.
     *  x (1-24) = The x LS bits are taken from the Shifted eVLAN attribute <Service-ID>.
     *  The MS bits are taken from the MS bits of the TS entry <MAC DA>
     *  (APPLICABLE RANGES: 0...24), 25-31 = Reserved
     */
    GT_U32 macDaMode;

    /** @brief 0 = Set IP address according to TS entry <DIP>
     *  x (1-24) = The x LS bits are taken from the Shifted eVLAN <Service-ID> attribute.
     *  The MS bits are taken from the MS bits of the TS entry's <DIP>.
     *  (APPLICABLE RANGES: 0...24), 25-31 = Reserved
     */
    GT_U32 dipMode;

    /** @brief 0
     *  1-7 - Insert control word (indexed by this field) after the MPLS labels
     *  Used for MPLS over IP-GRE
     *  (APPLICABLE RANGES: 0...7)
     */
    GT_U32 controlWordIndex;

} PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC;

/**
* @enum PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_ENT
 *
 * @brief Enumeration of tunnel start ip header protocol.
 * This enumerator is determined value in protocol field of
 * generated IP Tunnel Header.
*/
typedef enum{

    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E,

    /** @brief GRE protocol.
     *  The protocol value in IP Tunnel Header is 47.
     */
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E,

    /** @brief UDP protocol.
     *  The protocol value in IP Tunnel Header is 17.
     */
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E,

    /** Protocol is according to <IP Protocol> field */
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E

} PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_ENT;

/**
* @enum PRV_TGF_TUNNEL_START_GENERIC_TYPE_ENT
 *
 * @brief Enumeration of generic tunnel type.
 * This enumerator describes generic tunenl types entries.
*/
typedef enum{

    /** @brief short generic tunnel start [96 bits] */
    PRV_TGF_TUNNEL_START_GENERIC_SHORT_TYPE_E,

    /** @brief medium generic tunnel start [192 bits] */
    PRV_TGF_TUNNEL_START_GENERIC_MEDIUM_TYPE_E,

    /** @brief long generic tunnel start [384 bits] */
    PRV_TGF_TUNNEL_START_GENERIC_LONG_TYPE_E

} PRV_TGF_TUNNEL_START_GENERIC_TYPE_ENT;

/**
* @enum PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of flow label assign mode .
 * This enumerator is describes Tunnel-start flow label
 * assignment options.
*/
typedef enum{

    /** @brief Set the Flow Label to 0.
     *  PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E Set the Flow Label to packet hash value.
     */
    PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E,

    PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E

} PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT;

/**
* @struct PRV_TGF_TUNNEL_START_MPLS_ENTRY_STC
 *
 * @brief Struct of configuration for X-over-MPLS tunnel start
*/
typedef struct{

    /** If set, the packet is sent with VLAN tag */
    GT_BOOL tagEnable;

    /** VID in the Tunnel VLAN tag */
    GT_U16 vlanId;

    /** user priority mode */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT upMarkMode;

    /** user priority */
    GT_U32 up;

    /** Tunnel next hop MAC DA */
    GT_ETHERADDR macDa;

    /** number of MPLS labels */
    GT_U32 numLabels;

    /** tunnel IPv4 header TTL */
    GT_U32 ttl;

    /** MPLS LSR TTL operations */
    PRV_TGF_TUNNEL_START_TTL_MODE_ENT ttlMode;

    PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_ENT ethType;

    /** MPLS label 1 */
    GT_U32 label1;

    /** EXP1 mode */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT exp1MarkMode;

    /** The EXP1 value */
    GT_U32 exp1;

    /** MPLS label 2 */
    GT_U32 label2;

    /** @brief EXP2 mode
     *  exp2MarkMode   - EXP3 mode
     */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT exp2MarkMode;

    /** @brief The EXP2 value
     *  exp2       - The EXP3 value
     */
    GT_U32 exp2;

    GT_U32 label3;

    PRV_TGF_TUNNEL_START_MARK_MODE_ENT exp3MarkMode;

    GT_U32 exp3;

    /** retain the passenger packet CRC */
    GT_BOOL retainCRC;

    /** S */
    GT_BOOL setSBit;

    /** CFI bit assigned for TS packets */
    GT_U32 cfi;

    /** whether to insert an MPLS control word to the MPLS tunnel */
    GT_BOOL controlWordEnable;

    /** @brief the MPLS Control word to insert to the MPLS tunnel.
     *  Applicable when controlWordEnable is set to GT_TRUE.
     *  mplsEthertypeSelect- Select which of the 2 global MPLS EtherTypes to use in
     *  the outgoing packet. Per RFC 5332, there are separate
     *  MPLS EtherTypes for downstream assigned labels and
     *  multicast upstream assigned labels.
     */
    GT_U32 controlWordIndex;

    PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_ENT mplsEthertypeSelect;

} PRV_TGF_TUNNEL_START_MPLS_ENTRY_STC;

/**
* @struct PRV_TGF_TUNNEL_START_IPV4_ENTRY_STC
 *
 * @brief Struct of configuration for X-over-IPv4 tunnel start
*/
typedef struct{

    /** If set, the packet is sent with VLAN tag */
    GT_BOOL tagEnable;

    /** VID in the Tunnel VLAN tag */
    GT_U16 vlanId;

    /** user priority */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT upMarkMode;

    /** The tag 802.1p user priority */
    GT_U32 up;

    /** DSCP mode */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT dscpMarkMode;

    /** The DSCP in the IPv4 tunnel header */
    GT_U32 dscp;

    /** Tunnel next hop MAC DA */
    GT_ETHERADDR macDa;

    /** Don't fragment flag in the tunnel IPv4 header */
    GT_BOOL dontFragmentFlag;

    /** tunnel IPv4 header TTL */
    GT_U32 ttl;

    /** @brief Relevant for IPv6
     *  if GT_TRUE, the IPv4 header DIP is derived from IPv6
     *  passenger packet and destIp field is ignored
     */
    GT_BOOL autoTunnel;

    /** @brief Relevant for IPv6
     *  If <autoTunnel> is GT_TRUE, this field is the offset
     *  of IPv4 destination address inside IPv6 destination
     *  address
     */
    GT_U32 autoTunnelOffset;

    /** Ethertype to put in GRE protocol */
    CPSS_TUNNEL_GRE_ETHER_TYPE_ENT ethType;

    /** Tunnel destination IP */
    GT_IPADDR destIp;

    /** Tunnel source IP */
    GT_IPADDR srcIp;

    /** CFI bit assigned for TS packets */
    GT_U32 cfi;

    /** retain passenger packet CRC */
    GT_BOOL retainCrc;

    /** IP Header protocol field in the tunnel header.(APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_ENT ipHeaderProtocol;

    GT_U32 ipProtocol;

    /** @brief The profile table index of this Generic IPv4 TS entry.(APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  (APPLICABLE RANGES: 0...7).
     *  The Generic Tunnel-start Profile table serves as logical extension to the
     *  Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes either
     *  GRE or UDP header.
     *  There are 8 profiles available to use. But if application is
     *  going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
     *  (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
     *  tunnel types take place , and these tunnels are not required profile data generation - empty
     *  profile should be reserved. In this case, in order to save empty profiles number, it is
     *  recommended to utilize profile 7 as well.
     */
    GT_U32 profileIndex;

    /** @brief GRE Protocol for Ethernet Passenger. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E. If the passenger packet is Ethernet,
     *  this field defines the GRE protocol.(The least-significant 16 bits of
     *  the first word in the GRE header are generated in accordance with this value)
     *  NOTE: If the passenger is IPv4, the GRE protocol is fixed: 0x0800.
     *  If the passenger is IPv6, the GRE protocol is fixed: 0x86DD.
     */
    GT_U32 greProtocolForEthernet;

    /** @brief GRE flags and version. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E.
     *  The most significant 16-bits of the GRE header that includes the
     */
    GT_U32 greFlagsAndVersion;

    /** @brief UDP destination port. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E.
     *  GRE flag bits and the version number.
     */
    GT_U32 udpDstPort;

    /** @brief UDP source port. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E and the Profile <UDP
     *  source port mode> is not set to Hash Mode.
     */
    GT_U32 udpSrcPort;

} PRV_TGF_TUNNEL_START_IPV4_ENTRY_STC;


/**
* @struct PRV_TGF_TUNNEL_START_IPV6_ENTRY_STC
 *
 * @brief Struct of configuration for Generic IPv6 tunnel start
*/
typedef struct{

    /** If set, the packet is sent with VLAN tag */
    GT_BOOL tagEnable;

    /** VID in the Tunnel VLAN tag */
    GT_U16 vlanId;

    /** user priority */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT upMarkMode;

    /** The tag 802.1p user priority */
    GT_U32 up;

    /** DSCP mode */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT dscpMarkMode;

    /** The DSCP in the IPv4 tunnel header */
    GT_U32 dscp;

    /** Tunnel next hop MAC DA */
    GT_ETHERADDR macDa;

    /** tunnel IPv4 header TTL */
    GT_U32 ttl;

    /** Tunnel destination IP */
    GT_IPV6ADDR destIp;

    /** Tunnel source IP */
    GT_IPV6ADDR srcIp;

    /** retain passenger packet CRC */
    GT_BOOL retainCrc;

    /** IP Header protocol field in the tunnel header.(APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_ENT ipHeaderProtocol;

    GT_U32 ipProtocol;

    /** @brief The profile table index of this Generic IPv4 TS entry.(APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  (APPLICABLE RANGES: 0...7).
     *  The Generic Tunnel-start Profile table serves as logical extension to the
     *  Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes either
     *  GRE or UDP header.
     *  There are 8 profiles available to use. But if application is
     *  going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
     *  (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
     *  tunnel types take place , and these tunnels are not required profile data generation - empty
     *  profile should be reserved. In this case, in order to save empty profiles number, it is
     *  recommended to utilize profile 7 as well.
     */
    GT_U32 profileIndex;

    /** @brief GRE Protocol for Ethernet Passenger. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E. If the passenger packet is Ethernet,
     *  this field defines the GRE protocol.(The least-significant 16 bits of
     *  the first word in the GRE header are generated in accordance with this value)
     *  NOTE: If the passenger is IPv4, the GRE protocol is fixed: 0x0800.
     *  If the passenger is IPv6, the GRE protocol is fixed: 0x86DD.
     */
    GT_U32 greProtocolForEthernet;

    /** @brief GRE flags and version. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E.
     *  The most significant 16-bits of the GRE header that includes the
     */
    GT_U32 greFlagsAndVersion;

    /** @brief UDP destination port. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E.
     *  GRE flag bits and the version number.
     */
    GT_U32 udpDstPort;

    /** @brief UDP source port. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E and the Profile <UDP
     *  source port mode> is not set to Hash Mode.
     */
    GT_U32 udpSrcPort;

    /** ipv6 flow label mode. */
    PRV_TGF_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT flowLabelMode;

} PRV_TGF_TUNNEL_START_IPV6_ENTRY_STC;


/**
* @struct PRV_TGF_TUNNEL_START_MIM_ENTRY_STC
 *
 * @brief Struct of configuration for MacInMac tunnel start entry
*/
typedef struct{

    /** the packet is sent with VLAN tag */
    GT_BOOL tagEnable;

    /** VID in the Tunnel VLAN tag */
    GT_U16 vlanId;

    /** user priority mode */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT upMarkMode;

    /** user priority */
    GT_U32 up;

    /** tunnel IPv4 header TTL */
    GT_U32 ttl;

    /** Tunnel header MAC DA */
    GT_ETHERADDR macDa;

    /** retain passenger packet CRC */
    GT_BOOL retainCrc;

    /** MacInMac inner service identifier */
    GT_U32 iSid;

    /** iSid Assignment mode */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT iSidAssignMode;

    /** inner user priority field */
    GT_U32 iUp;

    /** Marking mode to determine assignment of iUp */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT iUpMarkMode;

    /** inner drop precedence in the iTag */
    GT_U32 iDp;

    /** DP mode */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT iDpMarkMode;

    /** The reserved bits to be set in the iTag */
    GT_U32 iTagReserved;

    /** tunnel start MIM I */
    PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT mimISidAssignMode;

    /** B */
    PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT mimBDaAssignMode;

    PRV_TGF_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT mimBSaAssignMode;

} PRV_TGF_TUNNEL_START_MIM_ENTRY_STC;

/**
* @struct PRV_TGF_TUNNEL_START_TRILL_HEADER_STC
 *
 * @brief configuration struct for TRILL header
*/
typedef struct{

    /** version (2 bits) */
    GT_U32 version;

    /** @brief Multi
     *  0: unicat packet
     *  1: multicast packet
     */
    GT_BOOL mBit;

    /** option length (5 bits) */
    GT_U32 opLength;

    /** hop count (6 bits) */
    GT_U32 hopCount;

    /** @brief when mBit=0, this is the Egress RBridge
     *  when mBit=1, this is the TREE-ID (16 bits)
     */
    GT_U32 eRbid;

    /** Ingress RBridge (16 bits) */
    GT_U32 iRbid;

} PRV_TGF_TUNNEL_START_TRILL_HEADER_STC;

/**
* @struct PRV_TGF_TUNNEL_START_TRILL_ENTRY_STC
 *
 * @brief Struct of configuration for TRILL tunnel start
*/
typedef struct{

    /** @brief Relevant only if entry <tagEnable> is set.
     *  Options:
     *  - Set user priority according to entry <802.1p UP>.
     *  - Set user priority according to packet assigned QoS attributes
     */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT upMarkMode;

    /** @brief The tag 802.1p user priority (0..7)
     *  Relevant only if entry <tagEnable> is set and entry
     *  <upMarkMode> is set according to entry <802.1p UP>.
     */
    GT_U32 up;

    /** If set, the packet is sent with VLAN tag */
    GT_BOOL tagEnable;

    /** @brief Relevant only if entry <tagEnable> is set. This
     *  field is the VID in the Tunnel VLAN tag (0..4095)
     */
    GT_U16 vlanId;

    /** Tunnel header next hop MAC DA */
    GT_ETHERADDR nextHopMacDa;

    /** TRILL header, relevant only if incoming packet is native Ethernet */
    PRV_TGF_TUNNEL_START_TRILL_HEADER_STC trillHeader;

} PRV_TGF_TUNNEL_START_TRILL_ENTRY_STC;

/**
* @struct PRV_TGF_TUNNEL_START_QINQ_ENTRY_STC
 *
 * @brief Struct of configuration for QinQ tunnel start entry.
*/
typedef struct{

    /** @brief GT_TRUE: the packet is sent with VLAN tag
     *  GT_FALSE: the packet is send without VLAN tag
     */
    GT_BOOL tagEnable;

    /** @brief Relevant only if entry <tagEnable> is set. This
     *  field is the VID in the Tunnel VLAN tag (0..4095)
     */
    GT_U16 vlanId;

    /** @brief Relevant only if entry <tagEnable> is set.
     *  Options:
     *  - Set user priority according to entry <802.1p UP>.
     *  - Set user priority according to up assigned by previous
     *  ingress engines.
     */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT upMarkMode;

    /** @brief The tag 802.1p user priority (0..7)
     *  Relevant only if entry <tagEnable> is set and entry
     *  <upMarkMode> is set according to entry <802.1p UP>.
     */
    GT_U32 up;

    /** Tunnel next hop MAC DA */
    GT_ETHERADDR macDa;

    /** @brief tunnel IPv4 header TTL (0..255)
     *  0:   use passenger packet TTL
     *  1-255: use this field for header TTL
     */
    GT_U32 ttl;

    /** @brief Relevant for Ethernet passenger packet.
     *  GT_TRUE: retain passenger packet CRC and add
     *  additional 4-byte CRC based on the tunneling header
     *  packet.
     *  GT_FALSE: remove passenger packet CRC
     */
    GT_BOOL retainCrc;

    /** @brief encapsulated MPLS packet ether type (mpls packet comes
     *  after the inner c-tag).
     */
    PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_ENT mplsEthType;

    /** Outer Vlan tag Ether Type (16 bit). */
    GT_32 sTagEthType;

    /** Outer Vlan tag CFI (Canonical format indicator) (0..1). */
    GT_BOOL sTagCfi;

    /** @brief Options:
     *  - Set sTag according to entry.
     *  - Set sTag according to sTag assigned by previous
     *  ingress engines.
     */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT sTagCommand;

    /** Inner Vlan tag Ether Type (16 bit). */
    GT_32 cTagEthType;

    /** Inner tag CFI (0..1). */
    GT_BOOL cTagCfi;

    /** @brief Options:
     *  - Set cTag according to entry.
     *  - Set cTag according to cTag assigned by previous
     *  ingress engines.
     */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT cTagCommand;

    /** Inner tag vid (vlan id) (0..4095). */
    GT_U16 cTagVid;

    /** @brief Options:
     *  - Set cTagUp according to entry.
     *  - Set cTagUp according to cTagUp assigned by previous
     *  ingress engines.
     */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT cTagUpMarkMode;

    /** Inner tag user priority (0..7). */
    GT_U32 cTagUp;

} PRV_TGF_TUNNEL_START_QINQ_ENTRY_STC;

/**
* @struct PRV_TGF_TUNNEL_START_GENERIC_CONFIG_STC
 *
 * @brief Struct of configuration for generic tunnel start
 * APPLICABLE DEVICES: Falcon.
 * NOT APPLICABLE DEVICES:
 * xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*/
typedef struct
{
    /** @brief Relevant only if entry <tagEnable> is set.
     *  Options:
     *  - Set user priority according to entry <802.1p UP>.
     *  - Set user priority according to up assigned by previous
     *  ingress engines.
     */
    PRV_TGF_TUNNEL_START_MARK_MODE_ENT upMarkMode;

    /** @brief The tag 802.1p user priority (APPLICABLE RANGES: 0..7)
     *  Relevant only if entry <tagEnable> is set and entry
     *  <upMarkMode> is set according to entry <802.1p UP>.
     */
    GT_U32 up;

    /** @brief If set, the packet is sent with VLAN tag. */
    GT_BOOL tagEnable;

    /** @brief Relevant only if entry <tagEnable> is set. This
     *  field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
     */
    GT_U16 vlanId;

    /** @brief Defines generic tunnel start type. */
    PRV_TGF_TUNNEL_START_GENERIC_TYPE_ENT genericType;

    /** @brief Relevant for Ethernet passenger packet.
     *  GT_TRUE: retain passenger packet CRC and add
     *  additional 4-byte CRC based on the tunneling header
     *  packet.
     *  GT_FALSE: remove passenger packet CRC.
     */
    GT_BOOL retainCrc;

    /** @brief Tunnel header MAC DA. */
    GT_ETHERADDR macDa;

    /** @brief Generic tunnel start header ether type. */
    GT_U32 etherType;

    /** @brief Bytes that are added after L2 header.
     *  Relevant for <Generic TS Type>=Medium or Long.
     *  If <Generic TS Type>=Medium only 12 first bytes are relevant.
    */
    GT_U8 data[36];
}
PRV_TGF_TUNNEL_START_GENERIC_ENTRY_STC;

/**
* @union PRV_TGF_TUNNEL_START_ENTRY_UNT
 *
 * @brief Union for configuration for tunnel start
 *
*/
typedef union{
    /** configuration for generic IPv4 tunnel start entry */
    PRV_TGF_TUNNEL_START_IPV4_ENTRY_STC ipv4Cfg;

    /** configuration for generic IPv6 tunnel start entry */
    PRV_TGF_TUNNEL_START_IPV6_ENTRY_STC ipv6Cfg;

    /** configuration for X */
    PRV_TGF_TUNNEL_START_MPLS_ENTRY_STC mplsCfg;

    /** configuration for MacInMac tunnel start entry */
    PRV_TGF_TUNNEL_START_MIM_ENTRY_STC mimCfg;

    /** configuration for TRILL tunnel start entry */
    PRV_TGF_TUNNEL_START_TRILL_ENTRY_STC trillCfg;

    PRV_TGF_TUNNEL_START_QINQ_ENTRY_STC qinqCfg;

    /** generic tunnel start configuration */
    PRV_TGF_TUNNEL_START_GENERIC_ENTRY_STC genCfg;

} PRV_TGF_TUNNEL_START_ENTRY_UNT;



/**
* @struct PRV_TGF_TTI_MAC_VLAN_STC
 *
 * @brief Mac To Me and Vlan To Me.
*/
typedef struct{

    /** Mac to me address */
    GT_ETHERADDR mac;

    /** vlan */
    GT_U16 vlanId;

} PRV_TGF_TTI_MAC_VLAN_STC;

/**
* @struct PRV_TGF_TTI_QOS_STC
 *
 * @brief TTI TCAM action qos parameters.
*/
typedef struct{

    /** enable/disable keep previous QoS */
    GT_BOOL keepPreviousQoS;

    /** whether to trust UP */
    GT_BOOL trustUp;

    /** whether to trust DSCP */
    GT_BOOL trustDscp;

    /** whether to trust EXP */
    GT_BOOL trustExp;

    /** enable/disable UP to UP remapping */
    GT_BOOL enableUpToUpRemapping;

    /** enable/disable DSCP to DSCP remapping */
    GT_BOOL enableDscpToDscpRemapping;

    /** enable/disable CFI to DP mapping */
    GT_BOOL enableCfiToDpMapping;

    /** user priority (0..7) */
    GT_U32 up;

    /** DSCP (6 bits) */
    GT_U32 dscp;

    /** traffic class (0..7) */
    GT_U32 tc;

    /** Drop precedence (color) */
    CPSS_DP_LEVEL_ENT dp;

    /** enable/disable modify DSCP */
    GT_BOOL modifyDscp;

    /** enable/disable modify UP */
    GT_BOOL modifyUp;

    /** enable/disable modify EXP */
    GT_BOOL modifyExp;

    /** QoS Precedence */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;

} PRV_TGF_TTI_QOS_STC;

/**
* @struct PRV_TGF_TTI_ACTION_QOS_STC
 *
 * @brief TTI TCAM default action qos parameters.
*/
typedef struct{

    /** TTI TCAM action qos parameters */
    PRV_TGF_TTI_QOS_STC qosParams;

    /** @brief exp
     *  Note: the exp is not part of the qos parameters
     *  because it belong also to mpls parameters
     */
    GT_U32 exp;

} PRV_TGF_TTI_ACTION_QOS_STC;

/**
* @enum PRV_TGF_TTI_PW_CW_EXCEPTION_ENT
 *
 * @brief TTI pseudowire exceptions enum
*/
typedef enum{

    /** TTL expiry VCCV */
    PRV_TGF_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E = 0,

    /** Nondata control word (i.e., control word first nibble = 1) */
    PRV_TGF_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E = 1,

    /** @brief Applied when all the following conditions are met:
     *  1. Data control word (first nibble = 0)
     *  2. TTI action entry is matched and mark the packet for Tunnel Termination
     *  3. PW-CW<FRG> != 0
     */
    PRV_TGF_TTI_PW_CW_EXCEPTION_FRAGMENTED_E = 2,

    /** @brief Applied when all the following conditions are met:
     *  1. Data control word (first nibble = 0)
     *  2. TTI action entry is matched and mark the packet for Tunnel Termination
     *  3. ((TTI-AE<PW-CW Sequencing Enable> == Disable) AND (PW-CW<Sequence> != 0))
     *  OR
     *  ((TTI-AE<PW-CW Sequencing Enable> == enable) AND (PW-CW<Sequence> == 0))
     */
    PRV_TGF_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E = 3,

    /** Packets with control word first nibble > 1 */
    PRV_TGF_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E = 4

} PRV_TGF_TTI_PW_CW_EXCEPTION_ENT;

/**
* @enum PRV_TGF_TUNNEL_START_ECN_MODE_ENT
 *
 * @brief Enumeration of ECN mode.
 * This enumerator describes Tunnel-start ECN modes (according to rfc 6040).
*/
typedef enum{

    /** copatibility mode */
    PRV_TGF_TUNNEL_START_ECN_COMPATIBILITY_MODE_E,

    /** normal mode */
    PRV_TGF_TUNNEL_START_ECN_NORMAL_MODE_E

} PRV_TGF_TUNNEL_START_ECN_MODE_ENT;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfTtiTcamIndexIsRelativeSet function
* @endinternal
*
* @brief   Set whether the TTI indexes are relative to the start of the TTI section
*         in TCAM
* @param[in] isRelative               - GT_TRUE: TTI indexes are relative to the start of the TTI section
*                                      GT_FALSE: TTI indexes are absolute
*                                       None
*/
GT_VOID prvTgfTtiTcamIndexIsRelativeSet
(
    IN  GT_BOOL     isRelative
);

/**
* @internal prvTgfTtiTcamAbsoluteBaseGet function
* @endinternal
*
* @brief   Get absolute TTI rules index bases in TCAM for Hit0 and for Hit1
*         relevant for earch devices, otherwise returns 0
* @param[in] hitIndex                 -  0 or 1
*                                       None
*/
GT_U32 prvTgfTtiTcamAbsoluteBaseGet
(
    IN  GT_U32     hitIndex
);

/**
* @internal prvTgfTcamTtiNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for TTI rules
*
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       TCAM number of indexes for TTI rules.
*/
GT_U32 prvTgfTcamTtiNumOfIndexsGet
(
    IN     GT_U32                           hitNum
);

/**
* @internal prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet function
* @endinternal
*
* @brief   Get absolute TTI rules index bases in TCAM for Hit0/1/2/3
*         the 'relative index' is modulo (%) by the function according to
*         prvTgfTcamTtiNumOfIndexsGet(hitNum)
*         relevant for earch devices, otherwise returns 0
* @param[in] hitIndex                 -  0 .. 3
* @param[in] relativeTcamEntryIndex
*                                       None
*/
GT_U32 prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet
(
    IN  GT_U32     hitIndex,
    IN  GT_U32     relativeTcamEntryIndex
);


/**
* @internal prvTgfTtiRuleActionSet function
* @endinternal
*
* @brief   This function sets rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleActionSet
(
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_ACTION_STC       *actionPtr
);

/**
* @internal prvTgfTtiRuleAction2Set function
* @endinternal
*
* @brief   This function sets rule action
*
* @param[in] ruleIndex                - index of the rule in the TCAM
*                                      actionPtr - (pointer to) the TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleAction2Set
(
    IN  GT_U32                          ruleIndex,
    IN  PRV_TGF_TTI_ACTION_2_STC       *action2Ptr
);


/**
* @internal prvTgfTtiRuleDefaultActionSet function
* @endinternal
*
* @brief   This function sets the global default action (used by standard
*         actions).
* @param[in] defaultQosParamsPtr      - points to the default TTI rule action qos
*                                      parameters that applied on packet if packet's
*                                      search key matched with masked pattern
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiRuleDefaultActionSet
(
    IN  PRV_TGF_TTI_ACTION_QOS_STC  *defaultQosParamsPtr
);

/**
* @internal prvTgfTtiRuleDefaultActionGet function
* @endinternal
*
* @brief   This function gets the global default action qos parameters.
*
* @param[in] devNum                   - device number
*
* @param[out] defaultQosParamsPtr      - points to the default TTI rule action qos
*                                      parameters that applied on packet if packet's
*                                      search key matched with masked pattern
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiRuleDefaultActionGet
(
    IN  GT_U8                       devNum,
    OUT PRV_TGF_TTI_ACTION_QOS_STC  *defaultQosParamsPtr
);

/**
* @internal prvTgfTtiRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleSet
(
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT     ruleType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    IN  PRV_TGF_TTI_ACTION_STC       *actionPtr
);

/**
* @internal prvTgfTtiRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action
*
* @param[in] devNum                   - device number
* @param[in] ruleType                 - TTI rule type
* @param[in] ruleIndex                - index of the tunnel termination entry
*
* @param[out] patternPtr               - (pointer to) the rule's pattern
* @param[out] maskPtr                  - (pointer to) the rule's mask
* @param[out] actionPtr                - (pointer to) the TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT     ruleType,
    OUT PRV_TGF_TTI_RULE_UNT         *patternPtr,
    OUT PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT PRV_TGF_TTI_ACTION_STC       *actionPtr
);

/**
* @internal prvTgfTtiRule2Set function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRule2Set
(
    IN  GT_U32                       ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT    keyType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    IN  PRV_TGF_TTI_ACTION_2_STC     *actionPtr
);

/**
* @internal prvTgfTtiRule2Get function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
* @param[in] ruleIndex                - index of the tunnel termination entry
*
* @param[out] patternPtr               - (pointer to) the rule's pattern
* @param[out] maskPtr                  - (pointer to) the rule's mask
* @param[out] actionPtr                - (pointer to) the TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRule2Get
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT    keyType,
    OUT PRV_TGF_TTI_RULE_UNT         *patternPtr,
    OUT PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT PRV_TGF_TTI_ACTION_2_STC     *actionPtr
);
/**
* @internal prvTgfTtiRuleUdbSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and
*          Action Acording to UDB rule type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleUdbSet
(
    IN  GT_U32                       ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT    ruleType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    IN  PRV_TGF_TTI_ACTION_2_STC     *actionPtr
);
/**
* @internal prvTgfTtiRuleUdbGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and
*          Action Acording to UDB rule type
*
* @param[in] devNum                   - device number
* @param[in] ruleType                 - TTI UDB rule type
* @param[in] ruleIndex                - index of the tunnel termination entry
*
* @param[out] patternPtr               - (pointer to) the rule's pattern
* @param[out] maskPtr                  - (pointer to) the rule's mask
* @param[out] actionPtr                - (pointer to) the TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiRuleUdbGet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ruleIndex,
    IN  PRV_TGF_TTI_RULE_TYPE_ENT    ruleType,
    OUT PRV_TGF_TTI_RULE_UNT         *patternPtr,
    OUT PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT PRV_TGF_TTI_ACTION_2_STC     *actionPtr
);

/**
* @internal prvTgfTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
GT_STATUS prvTgfTtiRuleValidStatusSet
(
    IN  GT_U32                        routerTtiTcamRow,
    IN  GT_BOOL                       valid
);

/**
* @internal prvTgfTunnelStartEntrySet function
* @endinternal
*
* @brief   Set a tunnel start entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
*/
GT_STATUS prvTgfTunnelStartEntrySet
(
    IN  GT_U32                          index,
    IN  CPSS_TUNNEL_TYPE_ENT            tunnelType,
    IN  PRV_TGF_TUNNEL_START_ENTRY_UNT *configPtr
);

/**
* @internal prvTgfTunnelStartEntryGet function
* @endinternal
*
* @brief   Get a tunnel start entry
*
* @param[in] devNum                   - device number
* @param[in] index                    - line  for the tunnel start entry
* @param[in,out] tunnelTypePtr            - (pointer to) the type of the tunnel
*
* @param[out] configPtr                - (pointer to) tunnel start configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfTunnelStartEntryGet
(
    IN    GT_U8                           devNum,
    IN    GT_U32                          index,
    INOUT CPSS_TUNNEL_TYPE_ENT           *tunnelTypePtr,
    OUT   PRV_TGF_TUNNEL_START_ENTRY_UNT *configPtr
);

/**
* @internal prvTgfTunnelStartMimITagEtherTypeSet function
* @endinternal
*
* @brief   Sets globally the MAC in MAC I-Tag Ethertype (802.1ah).
*         This is the Backbone Service Instance TAG identified by
*         a 802.1ah Ethertype
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartMimITagEtherTypeSet
(
    IN  GT_U32  iTagEthType
);

/**
* @internal prvTgfTunnelStartMimITagEtherTypeGet function
* @endinternal
*
* @brief   Gets the MAC in MAC I-Tag Ethertype (802.1ah).
*         This is the Backbone Service Instance TAG identified by
*         a 802.1ah Ethertype.
* @param[in] devNum                   - device number
*
* @param[out] iTagEthTypePtr           - i-tag (instance tag) ethertype
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartMimITagEtherTypeGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *iTagEthTypePtr
);

/**
* @internal prvTgfTtiPortLookupEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for the specified key
*         type at the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortLookupEnableSet
(
    IN  GT_U32                         portNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_BOOL                       enable
);

/**
* @internal prvTgfTtiPortLookupEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         TTI lookup for the specified key type.
*
* @param[out] enablePtr                - points to enable/disable TTI lookup
*                                      GT_TRUE: TTI lookup is enabled
*                                      GT_FALSE: TTI lookup is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortLookupEnableGet
(
    IN  GT_U32                         portNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal prvTgfTtiMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiMacModeSet
(
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  PRV_TGF_TTI_MAC_MODE_ENT      macMode
);

/**
* @internal prvTgfTtiMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
*
* @param[out] macModePtr               - (poiner to) MAC mode to use
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiMacModeGet
(
    IN  GT_U8                         devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT PRV_TGF_TTI_MAC_MODE_ENT     *macModePtr
);

/**
* @internal prvTgfTtiPortIpv4OnlyTunneledEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only tunneled
*         packets received on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_U32                         portNum,
    IN  GT_BOOL                       enable
);

/**
* @internal prvTgfTtiMimEthTypeSet function
* @endinternal
*
* @brief   This function sets the MIM Ethernet type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiMimEthTypeSet
(
    IN  GT_U32                        ethType
);

/**
* @internal prvTgfTtiMacToMeSet function
* @endinternal
*
* @brief   This function sets the TTI MacToMe relevant Mac address and Vlan.
*         if a match is found, an internal flag is set. The MACTOME flag
*         is optionally used as part of the TT lookup trigger for IPV4 or MPLS traffic.
*         Note: if the packet is TT and the pasenger is Ethernet, another MACTOME
*         lookup is performed and the internal flag is set accordingly.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiMacToMeSet
(
    IN  GT_U32                            entryIndex,
    IN  PRV_TGF_TTI_MAC_VLAN_STC         *valuePtr,
    IN  PRV_TGF_TTI_MAC_VLAN_STC         *maskPtr
);

/**
* @internal prvTgfTtiPortGroupRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortGroupRuleSet
(
    IN  GT_U32                        ruleIndex,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  PRV_TGF_TTI_RULE_UNT         *patternPtr,
    IN  PRV_TGF_TTI_RULE_UNT         *maskPtr,
    IN  PRV_TGF_TTI_ACTION_STC       *actionPtr
);

/**
* @internal prvTgfTtiPortGroupRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType                  - TTI key type
* @param[in] ruleIndex                - index of the tunnel termination entry
*
* @param[out] patternPtr               - (pointer to) the rule's pattern
* @param[out] maskPtr                  - (pointer to) the rule's mask
* @param[out] actionPtr                - (pointer to) the TTI rule action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortGroupRuleGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  GT_U32                        ruleIndex,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT PRV_TGF_TTI_RULE_UNT         *patternPtr,
    OUT PRV_TGF_TTI_RULE_UNT         *maskPtr,
    OUT PRV_TGF_TTI_ACTION_STC       *actionPtr
);

/**
* @internal prvTgfTtiPortGroupMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortGroupMacModeSet
(
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  PRV_TGF_TTI_MAC_MODE_ENT      macMode
);

/**
* @internal prvTgfTtiPortGroupMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType                  - TTI key type
*
* @param[out] macModePtr               - (pointer to) MAC mode to use
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPortGroupMacModeGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT PRV_TGF_TTI_MAC_MODE_ENT     *macModePtr
);

/**
* @internal prvTgfTtiPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
GT_STATUS prvTgfTtiPortGroupRuleValidStatusSet
(
    IN  GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN  GT_U32                        routerTtiTcamRow,
    IN  GT_BOOL                       valid
);

/**
* @internal prvTgfBrgTtiCheck function
* @endinternal
*
* @brief   check if the device supports the tti
*
* @retval GT_TRUE                  - the device supports the tti
* @retval GT_FALSE                 - the device not supports the tti
*/
GT_BOOL prvTgfBrgTtiCheck(
    void
);

/**
* @internal prvTgfTtiEthernetTypeSet function
* @endinternal
*
* @brief   This function sets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
* @param[in] devNum                   - device number
* @param[in] ethertype                - Ethernet type
* @param[in] ethertype                - Ethernet type value (range 16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If there are 2 registers used for ethertype configuration,
*       one for ethertype identification of incoming tunneled packets in TTI
*       and one for setting the ethertype for outgoing packets in tunnel start
*       header alteration, these registers are configured to have the same value.
*
*/
GT_STATUS prvTgfTtiEthernetTypeSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TUNNEL_ETHERTYPE_TYPE_ENT   ethertypeType,
    IN  GT_U32                              ethertype
);

/**
* @internal prvTgfTtiEthernetTypeGet function
* @endinternal
*
* @brief   This function gets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
* @param[in] devNum                   - device number
* @param[in] ethertypeType            - Ethernet type
*
* @param[out] ethertypePtr             - Points to Ethernet type value (range 16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note If there are 2 registers used for ethertype configuration,
*       one for ethertype identification of incoming tunneled packets in TTI
*       and one for setting the ethertype for outgoing packets in tunnel start
*       header alteration, these registers are configured to have the same value.
*
*/
GT_STATUS prvTgfTtiEthernetTypeGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TUNNEL_ETHERTYPE_TYPE_ENT   ethertypeType,
    OUT GT_U32                              *ethertypePtr
);


/**
* @internal prvTgfTtiIpv4GreEthTypeSet function
* @endinternal
*
* @brief   This function sets the IPv4 GRE protocol Ethernet type.
*
* @param[in] devNum                   - device number
* @param[in] greTunnelType            - GRE tunnel type
* @param[in] ethType                  - Ethernet type value (range 16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiIpv4GreEthTypeSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TUNNEL_GRE_ETHER_TYPE_ENT   greTunnelType,
    IN  GT_U32                              ethType
);

/**
* @internal prvTgfTtiIpv4GreEthTypeGet function
* @endinternal
*
* @brief   This function gets the IPv4 GRE protocol Ethernet type.
*
* @param[in] devNum                   - device number
* @param[in] greTunnelType            - GRE tunnel type
*
* @param[out] ethTypePtr               - (pointer to) Ethernet type value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiIpv4GreEthTypeGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TUNNEL_GRE_ETHER_TYPE_ENT   greTunnelType,
    OUT GT_U32                              *ethTypePtr
);

/**
* @internal prvTgfTunnelStartPortIpTunnelTotalLengthOffsetEnableSet function
* @endinternal
*
* @brief   This feature allows overriding the <total length> in the IP header.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         This API enables this feature per port.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] enable                   - GT_TRUE: Add offset to tunnel total length
*                                      GT_FALSE: Don't add offset to tunnel total length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTunnelStartPortIpTunnelTotalLengthOffsetEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32   port,
    IN  GT_BOOL enable
);

/**
* @internal prvTgfTunnelStartIpTunnelTotalLengthOffsetSet function
* @endinternal
*
* @brief   This API sets the value for <IP Tunnel Total Length Offset>.
*         When the egress port is enabled for this feature, then the new
*         <Total Length> is Old<Total Length> + <IP Tunnel Total Length Offset>.
*         For example: when sending Eth-Over-IPv4 to a port connected to MacSec Phy,
*         then total length of the tunnel header need to be increased by 4 bytes
*         because the MacSec adds additional 4 bytes to the passenger packet but
*         is unable to update the tunnel header alone.
* @param[in] devNum                   - device number
* @param[in] additionToLength         - Ip tunnel total length offset (6 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range values
*/
GT_STATUS prvTgfTunnelStartIpTunnelTotalLengthOffsetSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 additionToLength
);

/**
* @internal prvTgfTtiPortTrillEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TRILL engine on the port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  TRILL engine on port
*                                      GT_FALSE: disable TRILL engine on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortTrillEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal prvTgfTtiPortTrillEnableGet function
* @endinternal
*
* @brief   This function gets the current state enables/disables of TRILL engine
*         on the port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - points to enable/disable TRILL
*                                      GT_TRUE:  TRILL engine enabled on port
*                                      GT_FALSE: TRILL engine disabled on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortTrillEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT GT_BOOL             *enablePtr
);

/**
* @internal prvTgfTtiTrillAdjacencyCheckEntrySet function
* @endinternal
*
* @brief   This function sets entry in the TRILL Adjacency Check dedicated TCAM.
*         A TCAM lookup is performed for every TRILL packet processed by the TRILL engine.
*         The TRILL engine uses a single TCAM lookup to implement
*         the following TRILL adjacency checks:
*         1. TRILL IS-IS Adjacency check -
*         Checks that the single-destination TRILL frame arrives from a
*         (neighbor, port) for which an IS-IS adjacency exists.
*         2. TRILL Tree Adjacency Check -
*         Checks that the multi-destination TRILL frame arrives from a
*         (neighbor, port) that is a branch on the given TRILL distribution tree.
*         If there is TCAM MISS, invoke the respective UC or Multi-Target exception command.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM (0..255)
*                                      ttiTrillAdjacencyPtr - points to TRILL Adjacency STC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillAdjacencyCheckEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  PRV_TGF_TTI_TRILL_ADJACENCY_STC    *valuePtr
);

/**
* @internal prvTgfTtiTrillAdjacencyCheckEntryGet function
* @endinternal
*
* @brief   This function gets entry in the TRILL Adjacency Check dedicated TCAM.
*         A TCAM lookup is performed for every TRILL packet processed by the TRILL engine.
*         The TRILL engine uses a single TCAM lookup to implement
*         the following TRILL adjacency checks:
*         1. TRILL IS-IS Adjacency check -
*         Checks that the single-destination TRILL frame arrives from a
*         (neighbor, port) for which an IS-IS adjacency exists.
*         2. TRILL Tree Adjacency Check -
*         Checks that the multi-destination TRILL frame arrives from a
*         (neighbor, port) that is a branch on the given TRILL distribution tree.
*         If there is TCAM MISS, invoke the respective UC or Multi-Target exception command.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM (0..255)
*
* @param[out] valuePtr                 - points to TRILL Adjacency STC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillAdjacencyCheckEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT PRV_TGF_TTI_TRILL_ADJACENCY_STC    *valuePtr
);

/**
* @internal prvTgfTtiPortTrillOuterVid0Set function
* @endinternal
*
* @brief   This function sets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanId                   - TRILL Outer Tag0 VID (0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortTrillOuterVid0Set
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U16          vlanId
);

/**
* @internal prvTgfTtiPortTrillOuterVid0Get function
* @endinternal
*
* @brief   This function gets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vlanIdPtr                - (pointer to) TRILL Outer Tag0 VID (0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortTrillOuterVid0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *vlanIdPtr
);

/**
* @internal prvTgfTtiTrillCpuCodeBaseSet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - The base CPU code value for the TRILL engine (192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTtiTrillCpuCodeBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          cpuCodeBase
);

/**
* @internal prvTgfTtiTrillCpuCodeBaseGet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - Points to the base CPU code value for the TRILL engine (192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTtiTrillCpuCodeBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *cpuCodeBasePtr
);

/**
* @internal prvTgfTtiExceptionCmdSet function
* @endinternal
*
* @brief   Set tunnel termination exception command.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are:
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS prvTgfTtiExceptionCmdSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    IN  CPSS_PACKET_CMD_ENT                 command
);

/**
* @internal prvTgfTtiExceptionCmdGet function
* @endinternal
*
* @brief   Get tunnel termination exception command.
*
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
*
* @param[out] commandPtr               - points to the command for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are:
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS prvTgfTtiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

/**
* @internal prvTgfTtiTrillDropCounterSet function
* @endinternal
*
* @brief   Sets the TRILL drop packet counter.
*
* @param[in] devNum                   - the device number
* @param[in] exceptionType            - tunnel termination exception type to set counter for; valid options:
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
* @param[in] counter                  - value to set counter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillDropCounterSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    IN  GT_U32                              counter
);

/**
* @internal prvTgfTtiTrillDropCounterGet function
* @endinternal
*
* @brief   Gets the TRILL drop packet counter.
*
* @param[in] devNum                   - the device number
* @param[in] exceptionType            - tunnel termination exception type to get counter for; valid options:
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E
*                                      CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E
*
* @param[out] counterPtr               - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillDropCounterGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    OUT GT_U32                              *counterPtr
);

/**
* @internal prvTgfTtiTrillMaxVersionSet function
* @endinternal
*
* @brief   This function sets the max Trill version.
*
* @param[in] devNum                   - device number
* @param[in] maxTrillVersion          - max TRILL version value (range 2 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillMaxVersionSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          maxTrillVersion
);

/**
* @internal prvTgfTtiTrillMaxVersionGet function
* @endinternal
*
* @brief   This function gets the max Trill version.
*
* @param[in] devNum                   - device number
*
* @param[out] maxTrillVersionPtr       - Points to max TRILL version value (range 2 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiTrillMaxVersionGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *maxTrillVersionPtr
);

/**
* @internal prvTgfTunnelTermInilifConfigurationSet function
* @endinternal
*
* @brief   Set Inlif Configuration
*/
GT_VOID prvTgfTunnelTermInilifConfigurationSet
(
    IN  GT_U8                               portIndex,
    OUT PRV_TGF_INLIF_PORT_MODE_ENT         *defaultInlifModePtr
);

/**
* @internal prvTgfTtiPclIdSet function
* @endinternal
*
* @brief   This function sets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
* @param[in] pclId                    - PCL ID value (10 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*/
GT_STATUS prvTgfTtiPclIdSet
(
    IN  GT_U8                           devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_U32                          pclId
);

/**
* @internal prvTgfTtiPclIdGet function
* @endinternal
*
* @brief   This function gets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
*
* @param[out] pclIdPtr                 - (points to) PCL ID value (10 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*/
GT_STATUS prvTgfTtiPclIdGet
(
    IN  GT_U8                           devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT      keyType,
    OUT GT_U32                          *pclIdPtr
);

/**
* @internal prvTgfTtiPacketTypeKeySizeSet function
* @endinternal
*
* @brief   function sets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] size                     - key  in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPacketTypeKeySizeSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_TTI_KEY_TYPE_ENT             keyType,
    IN PRV_TGF_TTI_KEY_SIZE_ENT             size
);

/**
* @internal prvTgfTtiPacketTypeKeySizeGet function
* @endinternal
*
* @brief   function gets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
*
* @param[out] sizePtr                  - points to key size in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPacketTypeKeySizeGet
(
    IN  GT_U8                                devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT             keyType,
    OUT PRV_TGF_TTI_KEY_SIZE_ENT            *sizePtr
);

/**
* @internal prvTgfTtiPortPassengerOuterIsTag0Or1Set function
* @endinternal
*
* @brief   Set per port TTI passenger Outer Tag is Tag0 or Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - recognize Outer Tag existence according to TPID1 or TPID0
*                                      CPSS_VLAN_ETHERTYPE0_E: Outer Tag exists if passenger outer VLAN Tag TPID = Tag0 TPID
*                                      CPSS_VLAN_ETHERTYPE1_E: Outer Tag exists if passenger outer VLAN tag TPID = Tag1 TPID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port , mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortPassengerOuterIsTag0Or1Set
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_ETHER_MODE_ENT          mode
);

/**
* @internal prvTgfTtiPortIpTotalLengthDeductionEnableSet function
* @endinternal
*
* @brief   For MACSEC packets over IPv4/6 tunnel, that are to be tunnel terminated,
*         this configuration enables aligning the IPv4/6 total header length to the
*         correct offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Enable/disable Ipv4/6 Total Length Deduction.
*                                      GT_TRUE: When enabled, and ePort default attribute
*                                      <IPv4/6 Total Length Deduction Enable> == Enabled:
*                                      - for IPv4:
*                                      <IPv4 Total Length> = Packet IPv4 header <Total Length> -
*                                      Global configuration < IPv4 Total Length Deduction Value>
*                                      - for IPv6:
*                                      <IPv6 Total Length> = Packet IPv6 header <Total Length> -
*                                      Global configuration < IPv6 Total Length Deduction Value>
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS prvTgfTtiPortIpTotalLengthDeductionEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfTtiPortIpTotalLengthDeductionEnableGet function
* @endinternal
*
* @brief   Get if IPv4/6 total header length is aligned to the correct offset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                -  Enable/disable Ipv4/6 Total Length Deduction.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS prvTgfTtiPortIpTotalLengthDeductionEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal prvTgfTtiIpTotalLengthDeductionValueSet function
* @endinternal
*
* @brief   Set Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipType                   - IPv4 or Ipv6; valid values:
*                                      CPSS_IP_PROTOCOL_IPV4_E
*                                      CPSS_IP_PROTOCOL_IPV6_E
* @param[in] value                    - IPv4 or IPv6 Total Length Deduction Value (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS prvTgfTtiIpTotalLengthDeductionValueSet
(
    IN GT_U8                                devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    IN GT_U32                               value
);

/**
* @internal prvTgfTtiIpTotalLengthDeductionValueGet function
* @endinternal
*
* @brief   Get Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipType                   - IPv4 or Ipv6; valid values:
*                                      CPSS_IP_PROTOCOL_IPV4_E
*                                      CPSS_IP_PROTOCOL_IPV6_E
*
* @param[out] valuePtr                 -  (pointer to) IPv4 or IPv6 Total Length Deduction Value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS prvTgfTtiIpTotalLengthDeductionValueGet
(
    IN  GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    OUT GT_U32                              *valuePtr
);

/**
* @internal prvTgfTtiUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] udbIndex                 - index of User Defined Byte to configure.
* @param[in] offset                   - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfTtiUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_TTI_KEY_TYPE_ENT             keyType,
    IN GT_U32                               udbIndex,
    IN PRV_TGF_TTI_OFFSET_TYPE_ENT          offsetType,
    IN GT_U8                                offset
);

/**
* @internal prvTgfTtiUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] udbIndex                 - index of User Defined Byte to configure.
*
* @param[out] offsetTypePtr            - (pointer to) TTI offset type.
* @param[out] offsetPtr                - (pointer to) The offset of the user-defined byte,
*                                      in bytes,from the place in the packet
*                                      indicated by the offset type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfTtiUserDefinedByteGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT            keyType,
    IN  GT_U32                              udbIndex,
    OUT PRV_TGF_TTI_OFFSET_TYPE_ENT         *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
);
/**
* @internal prvTgfTunnelStartEgessVlanTableServiceIdSet function
* @endinternal
*
* @brief   Set egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id, used as index in the Egress Vlan Translation Table.
*                                      (APPLICABLE RANGES:0..8191).
* @param[in] vlanServiceId            - tunnel start entry extension value (APPLICABLE RANGES: 0...FFFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartEgessVlanTableServiceIdSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32               vlanServiceId
);

/**
* @internal prvTgfTunnelStartEgessVlanTableServiceIdGet function
* @endinternal
*
* @brief   Get egress vlan table service Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id, used as index in the Egress Vlan Translation Table.
*                                      (APPLICABLE RANGES:0..8191).
*
* @param[out] vlanServiceIdPtr         - (pointer to) vlan service Id.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartEgessVlanTableServiceIdGet
(
    IN  GT_U8                devNum,
    IN  GT_U16               vlanId,
    OUT GT_U32               *vlanServiceIdPtr
);

/**
* @internal prvTgfTtiDefaultAction2Get function
* @endinternal
*
* @brief   This function gets default action values.
*
* @param[out] actionPtr                - (pointer to) the TTI rule action 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfTtiDefaultAction2Get
(
    OUT  PRV_TGF_TTI_ACTION_2_STC         *actionPtr
);

/**
* @internal prvTgfTtiDefaultIpv4PatternMaskGet function
* @endinternal
*
* @brief   This function gets default rule's pattern and mask values for IPv4 key.
*
* @param[in] defaultInfo              - default information to be set
*
* @param[out] patternPtr               - (pointer to) the TTI rule's pattern for IPv4 key
* @param[out] maskPtr                  - (pointer to) the TTI rule's mask for IPv4 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note default values:
*       pclId = 1;
*       srcIsTrunk = GT_FALSE;
*       isTagged = GT_TRUE;
*       tunneltype = IPv4-Over-IPv4;
*       All the other fields that are not input parameter are masked.
*       Input parameters are exect match.
*
*/
GT_STATUS prvTgfTtiDefaultIpv4PatternMaskGet
(
    IN   PRV_TGF_TTI_IPV4_RULE_DEFAULT_INFO_STC defaultInfo,
    OUT  PRV_TGF_TTI_IPV4_RULE_STC         *patternPtr,
    OUT  PRV_TGF_TTI_IPV4_RULE_STC         *maskPtr
);

/**
* @internal prvTgfTunnelStartGenProfileTableEntrySet function
* @endinternal
*
* @brief   Set Generic Tunnel-start Profile table entry .
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
* @param[in] profileDataPtr           - pointer to Generic Tunnel-start Profile data.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/

GT_STATUS prvTgfTunnelStartGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    IN  PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC         *profileDataPtr
);

/**
* @internal prvTgfTunnelStartGenProfileTableEntryGet function
* @endinternal
*
* @brief   Get Generic Tunnel-start Profile table entry .
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - entry index in Generic Tunnel-start Profile table.
*
* @param[out] profileDataPtr           - (pointer to) tunnel start profile configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfTunnelStartGenProfileTableEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  profileIndex,
    OUT PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC *profileDataPtr
);

/**
* @internal prvTgfTtiGreExtensionsSet function
* @endinternal
*
* @brief   Enable/Disable GRE extensions
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable the GRE extensions
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiGreExtensionsSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal prvTgfTtiIpv4McEnableSet function
* @endinternal
*
* @brief   Enable/disable the TTI lookup for IPv4 multicast
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:   the TTI lookup for IPv4 multicast
*                                      - GT_FALSE: disable the TTI lookup for IPv4 multicast
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiIpv4McEnableSet
(
    IN GT_U8       devNum,
    IN GT_BOOL     enable
);

/**
* @internal prvTgfTunnelStartMplsPwControlWordSet function
* @endinternal
*
* @brief   Sets a Pseudo Wire control word
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
* @param[in] value                    - the PW word to write
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartMplsPwControlWordSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    IN  GT_U32          value
);

/**
* @internal prvTgfTunnelStartMplsPwControlWordGet function
* @endinternal
*
* @brief   Gets a Pseudo Wire control word
*
* @param[in] devNum                   - device number
* @param[in] index                    - the PW word  (APPLICABLE RANGES: 0...14)
*
* @param[out] valuePtr                 - (pointer to) the read PW word
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or illegal PW word index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartMplsPwControlWordGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_U32          *valuePtr
);

/**
* @internal prvTgfTtiPortIpv4OnlyMacToMeEnableSet function
* @endinternal
*
* @brief   Enable/disable the IPv4 TTI lookup for only mac to me packets received on port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortIpv4OnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal prvTgfTtiPortMplsOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   Enable/disable the MPLS TTI lookup for only mac to me packets received on port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:  MPLS TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MPLS TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortMplsOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal prvTgfTtiPortMimOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   Enable/disable the MIM TTI lookup for only mac to me packets received on port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE:   MIM TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MIM TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiPortMimOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal prvTgfTtiPwCwExceptionCmdSet function
* @endinternal
*
* @brief   Set a PW CW exception command
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
* @param[in] command                  - the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPwCwExceptionCmdSet
(
    IN GT_U8                            devNum,
    IN PRV_TGF_TTI_PW_CW_EXCEPTION_ENT  exceptionType,
    IN CPSS_PACKET_CMD_ENT              command
);

/**
* @internal prvTgfTtiPwCwExceptionCmdGet function
* @endinternal
*
* @brief   Get a PW CW exception command
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
*
* @param[out] commandPtr               - (pointer to) the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPwCwExceptionCmdGet
(
    IN  GT_U8                           devNum,
    IN  PRV_TGF_TTI_PW_CW_EXCEPTION_ENT exceptionType,
    OUT CPSS_PACKET_CMD_ENT             *commandPtr
);

/**
* @internal prvTgfTtiPwCwCpuCodeBaseSet function
* @endinternal
*
* @brief   Set the base CPU code value for PWE3
*
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPwCwCpuCodeBaseSet
(
    IN GT_U8    devNum,
    IN GT_U32   cpuCodeBase
);

/**
* @internal prvTgfTtiPwCwCpuCodeBaseGet function
* @endinternal
*
* @brief   Get the base CPU code value for PWE3
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - (pointer to) the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiPwCwCpuCodeBaseGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *cpuCodeBasePtr
);

/**
* @internal prvTgfTtiTcamSegmentModeSet function
* @endinternal
*
* @brief   Sets a TTI TCAM segment mode for a specific key type
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
* @param[in] segmentMode              - TTI TCAM segment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiTcamSegmentModeSet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_TTI_KEY_TYPE_ENT                 keyType,
    IN PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT    segmentMode
);

/**
* @internal prvTgfTtiTcamSegmentModeGet function
* @endinternal
*
* @brief   Gets a TTI TCAM segment mode for a specific key type
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
*                                      OUTPUTS:
* @param[in] segmentModePtr           - (pointer to) TTI TCAM segment mode
*
* @param[out] segmentModePtr           - (pointer to) TTI TCAM segment mode
*                                      OUTPUTS:
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid HW value
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiTcamSegmentModeGet
(
    IN  GT_U8                                   devNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT                keyType,
    OUT PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT   *segmentModePtr
);

/**
* @internal prvTgfTtiExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set tunnel termination exception CPU code.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set CPU code for.
*                                      valid values:
*                                      PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
* @param[in] code                     - the  for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiExceptionCpuCodeSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    IN  CPSS_NET_RX_CPU_CODE_ENT            code
);

/**
* @internal prvTgfTtiExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get tunnel termination exception CPU code.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set CPU code for.
*                                      valid values:
*                                      PRV_TGF_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*
* @param[out] codePtr                  - (points to) the code for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_TTI_EXCEPTION_ENT           exceptionType,
    IN  CPSS_NET_RX_CPU_CODE_ENT            *codePtr
);

/**
* @internal prvTgfTtiSourceIdBitsOverrideSet function
* @endinternal
*
* @brief   Set the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
* @param[in] devNum                   - device number
* @param[in] lookup                   - the  number (APPLICABLE RANGES: 0..3)
* @param[in] overrideBitmap           - 12 bits bitmap. For each bit:
*                                      0 - do not override
*                                      1 - override
*                                      (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiSourceIdBitsOverrideSet
(
    IN GT_U8    devNum,
    IN GT_U32   lookup,
    IN GT_U32   overrideBitmap
);

/**
* @internal prvTgfTtiSourceIdBitsOverrideGet function
* @endinternal
*
* @brief   Get the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
* @param[in] devNum                   - device number
* @param[in] lookup                   - the  number (APPLICABLE RANGES: 0..3)
*
* @param[out] overrideBitmapPtr        - 12 bits bitmap. For each bit:
*                                      0 - do not override
*                                      1 - override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiSourceIdBitsOverrideGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   lookup,
    OUT GT_U32  *overrideBitmapPtr
);

/**
* @internal prvTgfTtiIPv6ExtensionHeaderSet function
* @endinternal
*
* @brief   Set one of the 2 configurable IPv6 extension headers.
*
* @param[in] devNum                   - device number
* @param[in] extensionHeaderId        - the configurable extension header index
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] extensionHeaderValue     - the configurable extension header value
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiIPv6ExtensionHeaderSet
(
    IN GT_U8    devNum,
    IN GT_U32   extensionHeaderId,
    IN GT_U32   extensionHeaderValue
);

/**
* @internal prvTgfTtiIPv6ExtensionHeaderGet function
* @endinternal
*
* @brief   Get one of the 2 configurable IPv6 extension headers.
*
* @param[in] devNum                   - device number
* @param[in] extensionHeaderId        - the configurable extension header index
*                                      (APPLICABLE RANGES: 0..1)
*                                      extensionHeaderValue - the configurable extension header value
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfTtiIPv6ExtensionHeaderGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   extensionHeaderId,
    OUT GT_U32  *extensionHeaderValuePtr
);

/**
* @internal prvTgfTtiFcoeForwardingEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet matches the "FCoE Ethertype", it is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] enable                   - enable/disable FCoE Forwarding
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeForwardingEnableSet
(
    IN GT_BOOL      enable
);

/**
* @internal prvTgfTtiFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return if a packet that matches the "FCoE Ethertype", is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeForwardingEnableGet
(
    OUT GT_BOOL     *enablePtr
);

/**
* @internal prvTgfTtiFcoeEtherTypeSet function
* @endinternal
*
* @brief   Set the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] etherType                - EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeEtherTypeSet
(
    IN GT_U16       etherType
);

/**
* @internal prvTgfTtiFcoeEtherTypeGet function
* @endinternal
*
* @brief   Return the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] etherTypePtr             - (pointer to) EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeEtherTypeGet
(
    OUT GT_U16      *etherTypePtr
);

/**
* @internal prvTgfTtiFcoeAssignVfIdEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet is FCoE and it contains a VF Tag, then the
*         VRF-ID is assigned the value VF_ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - enable/disable FCoE Forwarding
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeAssignVfIdEnableSet
(
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal prvTgfTtiFcoeAssignVfIdEnableGet function
* @endinternal
*
* @brief   Return if FCoE packet that contains a VF Tag is assigned the value VF_ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTtiFcoeAssignVfIdEnableGet
(
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal prvTgfTunnelStartEcnModeSet function
* @endinternal
*
* @brief   Set Tunnel-start ECN mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] mode                  - ECN mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ECN mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelStartEcnModeSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_TUNNEL_START_ECN_MODE_ENT    mode
);

/**
* @internal prvTgfTunnelEcnPacketCommandSet function
* @endinternal
*
* @brief   Set packet command assigned to tunnel terminated packets that have
*           an ECN indication (Congestion Experienced) in tunnel header, and
*           Not-ECT in the passenger header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] command               - packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or packet command.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelEcnPacketCommandSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT   command
);

/**
* @internal prvTgfTunnelEcnCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/Drop Code assigned to the packet if the packet
*          command is non-zero.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] cpuCode               - CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or packet command.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTunnelEcnCpuCodeSet
(
    IN GT_U8                     devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode
);

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __tgfTunnelGenh */

