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
* @file tgfBridgeGen.h
*
* @brief Generic API for Bridge
*
* @version   81
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfBridgeGenh
#endif /*CHX_FAMILY*/

#ifndef __tgfBridgeGenh
#define __tgfBridgeGenh

#include <common/tgfTunnelGen.h>
#include <common/tgfInlifGen.h>

#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/generic/bridge/cpssGenBrgSrcId.h>
#include <cpss/generic/bridge/cpssGenBrgSecurityBreachTypes.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <extUtils/tgf/tgfBridgeGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* define ports : of type CPSS_INTERFACE_INFO_STC */
#define PRV_TGF_INTERFACE_PORT_MAC(device,port)   \
    {                           \
        CPSS_INTERFACE_PORT_E,  \
        {                       \
            device,             \
            port,               \
        },                      \
        0                       \
    }

/* define trunk : of type CPSS_INTERFACE_INFO_STC */
#define PRV_TGF_INTERFACE_TRUNK_MAC(trunkId)   \
    {                           \
        CPSS_INTERFACE_TRUNK_E, \
        {                       \
            0,                  \
            0,                  \
        },                      \
        trunkId                 \
    }

/* define trunk : of type PRV_TGF_OUTLIF_INFO_STC */
#define PRV_TGF_OUTLIF_LL_MAC(device,port)                        \
{                                                                 \
    PRV_TGF_OUTLIF_TYPE_LL_E,                                     \
    PRV_TGF_INTERFACE_PORT_MAC(device,port), /*interface*/        \
}

/* default (empty) mac entry : of type PRV_TGF_BRG_MAC_ENTRY_STC */
#define PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC           \
    {                                                   \
        {  /*PRV_TGF_MAC_ENTRY_KEY_STC*/               \
            PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E,          \
            0,                                          \
            {                                           \
                {                                       \
                    {{0x0,0x15,0x15,0x15,0x15,0x15}},/*mac*/\
                    1,                    /*vlan*/      \
                }                                       \
            }                                           \
        },                                              \
        PRV_TGF_INTERFACE_PORT_MAC(0,0), /*interface*/  \
        1,   /*age                        */            \
        PRV_TGF_OUTLIF_LL_MAC(0,0),/*outlif*/           \
        0,   /*isStatic;                  */            \
        0,   /*daCommand;                 */            \
        0,   /*saCommand;                 */            \
        0,   /*daRoute;                   */            \
        0,   /*mirrorToRxAnalyzerPortEn;  */            \
        0,   /*userDefined;              */             \
        0,   /*daQosIndex;               */             \
        0,   /*saQosIndex;               */             \
        0,   /*daSecurityLevel;          */             \
        0,   /*saSecurityLevel;          */             \
        0,   /*appSpecificCpuCode;       */             \
        0,   /*pwId;                     */             \
        0,   /*spUnknown;                */             \
        0,   /*sourceId;                 */             \
    }

/* number of words that the FDB device table is using */
#define PRV_TGF_FDB_DEV_TABLE_SIZE_CNS    4


/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/


/**
* @enum PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_ENT
 *
 * @brief This enum defines station movement modes
*/
typedef enum{

    /** @brief station moved from
     *  one port to another port
     */
    PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_PORT_E,

    /** @brief station moved from
     *  port to trunk
     */
    PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_PORT_TO_TRUNK_E,

    /** @brief station moved from
     *  trunk to port
     */
    PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_PORT_E,

    /** @brief station moved from
     *  one trunk to another trunk
     */
    PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_TRUNK_TO_TRUNK_E,

    /** @brief station moved from
     *  one port of the trunk to another port in the SAME trunk.
     */
    PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_IN_THE_TRUNK_E

} PRV_TGF_BRG_FDB_STATION_MOVEMENT_MODE_ENT;

/**
* @enum PRV_TGF_BRG_FDB_ACCESS_MODE_ENT
 *
 * @brief This enum defines access mode to the FDB table
*/
typedef enum{

    /** 'by message' */
    PRV_TGF_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E,

    /** 'by Index' */
    PRV_TGF_BRG_FDB_ACCESS_MODE_BY_INDEX_E

} PRV_TGF_BRG_FDB_ACCESS_MODE_ENT;

/**
* @enum PRV_TGF_BRG_IP_CTRL_TYPE_ENT
 *
 * @brief This enum defines IP Control to CPU Enable Mode
*/
typedef enum{

    /** Disable both IPv4 and Ipv6 Control traffic to CPU */
    PRV_TGF_BRG_IP_CTRL_NONE_E,

    /** Enable IPv4 Control traffic to CPU */
    PRV_TGF_BRG_IP_CTRL_IPV4_E,

    /** Enable IPv6 Control traffic to CPU */
    PRV_TGF_BRG_IP_CTRL_IPV6_E,

    /** Enable both IPv4 and IPv6 traffic to CPU */
    PRV_TGF_BRG_IP_CTRL_IPV4_IPV6_E

} PRV_TGF_BRG_IP_CTRL_TYPE_ENT;

/**
* @enum PRV_TGF_BRG_DROP_CNTR_MODE_ENT
 *
 * @brief Enumeration of Bridge Drop Counter reasons to be counted.
*/
typedef enum{

    /** Count All */
    PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E = 0,

    /** FDB Entry command Drop */
    PRV_TGF_BRG_DROP_CNTR_FDB_ENTRY_CMD_E,

    /** Unknown MAC SA Drop */
    PRV_TGF_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E,

    /** Invalid SA Drop */
    PRV_TGF_BRG_DROP_CNTR_INVALID_SA_E,

    /** VLAN not valid Drop */
    PRV_TGF_BRG_DROP_CNTR_INVALID_VLAN_E,

    /** Port not Member in VLAN Drop */
    PRV_TGF_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E,

    /** VLAN Range Drop */
    PRV_TGF_BRG_DROP_CNTR_VLAN_RANGE_E,

    /** Moved Static Address Drop */
    PRV_TGF_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E,

    /** ARP SA mismatch Drop */
    PRV_TGF_BRG_DROP_CNTR_ARP_SA_MISMATCH_E,

    /** SYN with data Drop */
    PRV_TGF_BRG_DROP_CNTR_SYN_WITH_DATA_E,

    /** TCP over MCAST or BCAST Drop */
    PRV_TGF_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E,

    /** Bridge Access Matrix Drop */
    PRV_TGF_BRG_DROP_CNTR_ACCESS_MATRIX_E,

    /** Secure Learning Drop */
    PRV_TGF_BRG_DROP_CNTR_SEC_LEARNING_E,

    /** Acceptable Frame Type Drop */
    PRV_TGF_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E,

    /** Fragmented ICMP Drop */
    PRV_TGF_BRG_DROP_CNTR_FRAG_ICMP_E,

    /** TCP Flags Zero Drop */
    PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E,

    /** TCP Flags FIN, URG and PSH are all set Drop */
    PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E,

    /** TCP Flags SYN and FIN are set Drop */
    PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E,

    /** TCP Flags SYN and RST are set Drop */
    PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E,

    /** TCP/UDP Source */
    PRV_TGF_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E,

    /** VLAN MRU Drop */
    PRV_TGF_BRG_DROP_CNTR_VLAN_MRU_E,

    /** Rate Limiting Drop */
    PRV_TGF_BRG_DROP_CNTR_RATE_LIMIT_E,

    /** Local Port Drop */
    PRV_TGF_BRG_DROP_CNTR_LOCAL_PORT_E,

    /** Spanning Tree port state Drop */
    PRV_TGF_BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E,

    /** IP Multicast Drop */
    PRV_TGF_BRG_DROP_CNTR_IP_MC_E,

    /** NON-IP Multicast Drop */
    PRV_TGF_BRG_DROP_CNTR_NON_IP_MC_E,

    /** DSATag due to Local dev Drop */
    PRV_TGF_BRG_DROP_CNTR_DSATAG_LOCAL_DEV_E,

    /** IEEE Reserved Drop */
    PRV_TGF_BRG_DROP_CNTR_IEEE_RESERVED_E,

    /** Unregistered L2 NON-IPM Multicast Drop */
    PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E,

    /** Unregistered L2 IPv6 Multicast Drop */
    PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E,

    /** Unregistered L2 IPv4 Multicast Drop */
    PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E,

    /** Unknown L2 Unicast Drop */
    PRV_TGF_BRG_DROP_CNTR_UNKNOWN_L2_UC_E,

    /** Unregistered L2 IPv4 Brodcast Drop */
    PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E,

    /** Unregistered L2 NON-IPv4 Brodcast Drop */
    PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E,

    PRV_TGF_BRG_DROP_CNTR_DATA_ERROR_E,

    PRV_TGF_BRG_DROP_CNTR_IPMC_IANA_RANGE_E,

    PRV_TGF_BRG_DROP_CNTR_IPMC_NOT_IANA_RANGE_E,

    PRV_TGF_BRG_DROP_CNTR_VALUE_CHASIS_E,

    PRV_TGF_BRG_DROP_CNTR_BLOCKING_LEARNING_E

} PRV_TGF_BRG_DROP_CNTR_MODE_ENT;

/**
* @enum PRV_TGF_BRG_VLAN_PORT_ISOLATION_CMD_ENT
 *
 * @brief Enumeration of port isolation commands
*/
typedef enum{

    /** port isolation is disabled */
    PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E,

    /** L2 packets forwaring enabled */
    PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_CMD_E,

    /** L3 packets forwaring enabled */
    PRV_TGF_BRG_VLAN_PORT_ISOLATION_L3_CMD_E,

    /** L2&L3 packets forwaring enabled */
    PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E

} PRV_TGF_BRG_VLAN_PORT_ISOLATION_CMD_ENT;

/**
* @enum PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ENT
 *
 * @brief Enumeration of assigning L2 multicast VIDX for Broadcast,
 * Unregistered Multicast, and unknown Unicast packets.
*/
typedef enum{

    /** unregistered MC mode */
    PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E,

    /** @brief apply <Flood VIDX> from VLAN entry for ALL
     *  flooded traffic
     */
    PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E

} PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ENT;

/**
* @enum PRV_TGF_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT
 *
 * @brief Enumeration of local switching traffic types.
*/
typedef enum{

    /** known UC */
    PRV_TGF_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E,

    /** unknown UC, BC, MC */
    PRV_TGF_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E

} PRV_TGF_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT;

/**
* @enum PRV_TGF_FDB_ACTION_MODE_ENT
 *
 * @brief Enum for FDB Action modes.
*/
typedef enum{

    /** @brief Automatic aging with removal
     *  of aged out entries.
     */
    PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E,

    /** @brief Automatic or triggered aging
     *  without removal of aged out
     *  entries or triggered FDB
     *  Upload.
     */
    PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E,

    /** Triggered Address deleting. */
    PRV_TGF_FDB_ACTION_DELETING_E,

    /** @brief Triggered Address
     *  Transplanting.
     */
    PRV_TGF_FDB_ACTION_TRANSPLANTING_E,

    /** @brief Scan the FDB table and send
     *  back to the CPU matching
     *  entries (FDB Upload).
     */
    PRV_TGF_FDB_ACTION_FDB_UPLOAD_E

} PRV_TGF_FDB_ACTION_MODE_ENT;

/**
* @enum PRV_TGF_MAC_ACTION_MODE_ENT
 *
 * @brief Enumeration of MAC address table action mode
*/
typedef enum{

    /** Action is done automatically */
    PRV_TGF_ACT_AUTO_E = 0,

    /** Action is done via trigger from CPU */
    PRV_TGF_ACT_TRIG_E

} PRV_TGF_MAC_ACTION_MODE_ENT;


/**
* @enum PRV_TGF_MAC_HASH_FUNC_MODE_ENT
 *
 * @brief Enum for FDB Hash function modes.
*/
typedef enum{

    /** @brief XOR based hash function mode,
     *  provides optimal hash index distribution
     *  for controlled testing scenarios, where
     *  sequential addresses and vlans are often used.
     */
    PRV_TGF_MAC_HASH_FUNC_XOR_E,

    /** @brief CRC based hash function mode,
     *  provides the best hash index distribution
     *  for random addresses and vlans.
     */
    PRV_TGF_MAC_HASH_FUNC_CRC_E,

    /** Different CRC hash functions per FDB bank. */
    PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E

} PRV_TGF_MAC_HASH_FUNC_MODE_ENT;

/**
* @enum PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_ENT
 *
 * @brief This enum defines the IEEE reserved multicast protocol type.
*/
typedef enum{

    /** @brief These standard protocols
     *  has a MAC range of:
     *  01-80-C2-00-00-00 to
     *  01-80-C2-00-00-0F
     */
    PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_STD_E,

    /** @brief These GARP protocols has
     *  a MAC range of:
     *  01-80-C2-00-00-20 to
     *  01-80-C2-00-00-2F
     */
    PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E

} PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_ENT;

/**
* @enum PRV_TGF_BRG_CNTR_SET_ID_ENT
 *
 * @brief Enumeration of counter set number.
*/
typedef enum{

    /** id for counter set 0 */
    PRV_TGF_BRG_CNTR_SET_ID_0_E = 0,

    /** id for counter set 1 */
    PRV_TGF_BRG_CNTR_SET_ID_1_E

} PRV_TGF_BRG_CNTR_SET_ID_ENT;

/**
* @struct PRV_TGF_BRG_VLAN_INFO_STC
 *
 * @brief A structure to hold generic VLAN entry info
*/
typedef struct{

    /** is CPU member */
    GT_BOOL cpuMember;

    /** Unknown Source Address is security breach event. */
    GT_BOOL unkSrcAddrSecBreach;

    /** The command of Unregistered non IP Multicast packets */
    CPSS_PACKET_CMD_ENT unregNonIpMcastCmd;

    /** The command of Unregistered IPv4 Multicast packets */
    CPSS_PACKET_CMD_ENT unregIpv4McastCmd;

    /** The command of Unregistered IPv6 Multicast packets */
    CPSS_PACKET_CMD_ENT unregIpv6McastCmd;

    /** The command of Unknown Unicast packets */
    CPSS_PACKET_CMD_ENT unkUcastCmd;

    /** The command of Unregistered IPv4 Broadcast packets */
    CPSS_PACKET_CMD_ENT unregIpv4BcastCmd;

    /** The command of Unregistered non */
    CPSS_PACKET_CMD_ENT unregNonIpv4BcastCmd;

    /** Enable IGMP Trapping or Mirroring to CPU */
    GT_BOOL ipv4IgmpToCpuEn;

    /** Mirror packets to Rx Analyzer port */
    GT_BOOL mirrToRxAnalyzerEn;

    /** Mirror packets to Tx Analyzer port */
    GT_BOOL mirrorToTxAnalyzerEn;

    /** Enable/Disable ICMPv6 trapping or mirroring */
    GT_BOOL ipv6IcmpToCpuEn;

    /** IPv4/6 control traffic trapping/mirroring to the CPU */
    PRV_TGF_BRG_IP_CTRL_TYPE_ENT ipCtrlToCpuEn;

    /** IPV4 Multicast Bridging mode */
    CPSS_BRG_IPM_MODE_ENT ipv4IpmBrgMode;

    /** IPV6 Multicast Bridging mode */
    CPSS_BRG_IPM_MODE_ENT ipv6IpmBrgMode;

    /** IPv4 Multicast Bridging Enable */
    GT_BOOL ipv4IpmBrgEn;

    /** IPv6 Multicast Bridging Enable */
    GT_BOOL ipv6IpmBrgEn;

    /** IPv6 Site Id Mode */
    CPSS_IP_SITE_ID_ENT ipv6SiteIdMode;

    /** Enable/Disable IPv4 Unicast Routing on the vlan */
    GT_BOOL ipv4UcastRouteEn;

    /** Enable/Disable IPv4 Multicast Routing on the vlan */
    GT_BOOL ipv4McastRouteEn;

    /** Enable/Disable IPv6 Unicast Routing on the vlan */
    GT_BOOL ipv6UcastRouteEn;

    /** Enable/Disable IPv6 Multicast Routing on the vlan */
    GT_BOOL ipv6McastRouteEn;

    /** stp group, range 0 */
    GT_U32 stgId;

    /** Enable/Disable automatic learning for this VLAN */
    GT_BOOL autoLearnDisable;

    /** Enable/Disable new address message sending to CPU */
    GT_BOOL naMsgToCpuEn;

    /** The index that this Vlan's MRU configuration packets */
    GT_U32 mruIdx;

    /** Enables trapping/mirroring of Broadcast UDP packets */
    GT_BOOL bcastUdpTrapMirrEn;

    /** Virtual Router ID */
    GT_U32 vrfId;

    /** The action applied on Multicast packets */
    CPSS_UNREG_MC_EGR_FILTER_CMD_ENT egressUnregMcFilterCmd;

    /** MC group to which the flooded packet is to be transmitted */
    GT_U16 floodVidx;

    /** @brief Mode for assigning L2 multicast VIDX for
     *  BC, Unregistered MC, and unknown UC packets
     */
    PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ENT floodVidxMode;

    /** Port isolation Mode */
    PRV_TGF_BRG_VLAN_PORT_ISOLATION_CMD_ENT portIsolationMode;

    /** Enable/disable local switching of known UC traffic */
    GT_BOOL ucastLocalSwitchingEn;

    /** @brief Enable/disable local switching of MC, unknown UC
     *  and BC traffic on this VLAN
     */
    GT_BOOL mcastLocalSwitchingEn;

    /** @brief Ingress Analyzer Mirror Index (0
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 mirrToRxAnalyzerIndex;

    /** @brief Enable to mirror packets to Tx Analyzer port
     *  GT_TRUE - Egress mirrored traffic assigned to
     *  this Vlan to the analyzer port
     *  GT_FALSE - Don't egress mirrored traffic assigned
     *  to this Vlan to the analyzer port
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL mirrToTxAnalyzerEn;

    /** @brief Egress Analyzer Mirror Index (0
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 mirrToTxAnalyzerIndex;

    /** @brief Forwarding ID value
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 fidValue;

    /** @brief bridge forwarding decision for packets with
     *  unknown Source Address. supported commands:
     *  CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
     *  CPSS_PACKET_CMD_DROP_SOFT_E
     *  (APPLICABLE DEVICES: Lion2)
     */
    CPSS_PACKET_CMD_ENT unknownMacSaCmd;

    /** @brief When enabled, IPv4 MC/BC packets in this VLAN
     *  are mirrored to the analyzer specified in
     */
    GT_BOOL ipv4McBcMirrToAnalyzerEn;

    /** @brief field.
     *  (APPLICABLE DEVICES: Lion2)
     *  ipv4McBcMirrToAnalyzerIndex - Mirror to Analyzer Index (0 - 6)
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 ipv4McBcMirrToAnalyzerIndex;

    /** @brief When enabled, IPv6 MC packets in this VLAN are
     *  mirrored to the analyzer specified in
     */
    GT_BOOL ipv6McMirrToAnalyzerEn;

    /** @brief field
     *  (APPLICABLE DEVICES: Lion2)
     *  ipv6McMirrToAnalyzerIndex - Mirror to Analyzer Index (0 - 6)
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 ipv6McMirrToAnalyzerIndex;

} PRV_TGF_BRG_VLAN_INFO_STC;


/**
* @struct PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC
 *
 * @brief structure that hold an array of ports' Tagging commands
*/
typedef struct{

    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT portsCmd[CPSS_MAX_PORTS_NUM_CNS];

} PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC;

/**
* @enum PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT
 *
 * @brief Enumeration of Tag1 removal mode when VID is zero.
*/
typedef enum{

    /** dont remove Tag1 */
    PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E,

    /** @brief enable only if
     *  ingress was NOT double tag (single tag or untagged)
     *  (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
     *  NOTE: the test can still call this value and the 'tgf glue' will convert it to
     *  CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E , for E_ARCH devices.
     */
    PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E,

    /** @brief enable regardless of ingress
     *  tagging
     *  CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E - enable only if
     *  ingress was without tag1
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E,

    PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E

} PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT;



/**
* @enum PRV_TGF_BRG_GEN_L1_RATE_LIMIT_ENT
 *
 * @brief Enumeration for L1 rate limit counting mode upon byte counting.
 * When rate limit counting mode is byte count this bit controls
 * the addition of 20 bytes for L1 (IPG + Preamble) for the packet
 * size.
*/
typedef enum{

    /** @brief Exclude L1; Rate limit byte
     *  count does not include the
     *  L1 value.
     */
    PRV_TGF_BRG_GEN_L1_RATE_LIMIT_EXCL_E = 0,

    /** @brief Include L1; Rate limit byte
     *  count includes the L1 value.
     */
    PRV_TGF_BRG_GEN_L1_RATE_LIMIT_INCL_E

} PRV_TGF_BRG_GEN_L1_RATE_LIMIT_ENT;

/**
* @struct PRV_TGF_BRG_GEN_RATE_LIMIT_STC
 *
 * @brief Describe rate limit parameters for DxCh devices.
*/
typedef struct{

    /** L1 rate limit counting mode upon byte counting */
    PRV_TGF_BRG_GEN_L1_RATE_LIMIT_ENT rateLimitModeL1;

    /** drop mode (soft or hard) */
    CPSS_DROP_MODE_TYPE_ENT dropMode;

    /** rate mode (bytes or packets) */
    CPSS_RATE_LIMIT_MODE_ENT rMode;

    /** size of time window for 10Mbps port speed */
    GT_U32 win10Mbps;

    /** size of time window for 100Mbps port speed */
    GT_U32 win100Mbps;

    /** size of time window for 1000Mbps port speed */
    GT_U32 win1000Mbps;

    /** @brief size of time window for 10Gbps port speed
     *  Comments:
     *  Window sizes specified in microseconds
     *  1000 Mbps: range - 256-16384 uSec  granularity - 256 uSec
     *  100 Mbps: range - 256-131072 uSec  granularity - 256 uSec
     *  10  Mbps: range - 256-1048570 uSec granularity - 256 uSec
     *  10  Gbps: range - 25.6-104857 uSec granularity - 25.6 uSec
     *  For DxCh3 and above: The granularity doesn't depend from PP core clock.
     *  For DxCh1, DxCh2:
     *  Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
     *  for they changed linearly with the clock value.
     */
    GT_U32 win10Gbps;

} PRV_TGF_BRG_GEN_RATE_LIMIT_STC;


/**
* @struct PRV_TGF_BRG_GEN_RATE_LIMIT_PORT_STC
 *
 * @brief Describe a port rate limit parameters.
*/
typedef struct{

    /** enable Bc packets rate limiting */
    GT_BOOL enableBc;

    /** @brief enable Mc packets rate limiting
     *  for xCat2 it controls unregistered Mc packets only
     */
    GT_BOOL enableMc;

    /** @brief enable Mc registered packets rate limiting
     *  (APPLICABLE DEVICES: xCat2)
     */
    GT_BOOL enableMcReg;

    /** enable Uc Unknown packets rate limiting */
    GT_BOOL enableUcUnk;

    /** enable Uc Known  packets rate limiting */
    GT_BOOL enableUcKnown;

    /** enable TCP SYN packets rate limiting */
    GT_BOOL enableTcpSyn;

    /** @brief rate limit specified in 64 bytes/packets
     *  depends on ratelimit mode
     *  range 0 - 65535
     *  Comments:
     *  None
     */
    GT_U32 rateLimit;

} PRV_TGF_BRG_GEN_RATE_LIMIT_PORT_STC;

/**
* @struct PRV_TGF_BRG_EGRESS_PORT_INFO_STC
 *
 * @brief Egress port info
*/
typedef struct{

    /** @brief GT_TRUE: redirected to Tunnel Egress Interface
     *  GT_FALSE: redirected to non-Tunnel Egress Interface
     */
    GT_BOOL tunnelStart;

    /** @brief pointer to the Tunnel Start entry. valid if <tunnelStart>
     *  is GT_TRUE
     */
    GT_U32 tunnelStartPtr;

    /** @brief Type of passenger packet for packet redirected
     *  to Tunnel-Start.
     */
    PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT tsPassengerPacketType;

    /** pointer to the ARP entry */
    GT_U32 arpPtr;

    /** @brief Indicates that the MAC SA of the packet is changed
     *  to the address of the current device.
     *  Relevant only when the <TS> flag is disabled.
     */
    GT_BOOL modifyMacSa;

    /** @brief Indicates that the MAC DA of the packet should
     *  be changed according to the <ARP pointer> field.
     *  Relevant only when the <TS> flag is disabled.
     */
    GT_BOOL modifyMacDa;

} PRV_TGF_BRG_EGRESS_PORT_INFO_STC;

/**
* @struct PRV_TGF_BRG_VLAN_PROT_CLASS_CFG_STC
 *
 * @brief Structure for configuring Vlan Protocol based classification
 * parameters.
*/
typedef struct{

    /** VLAN id */
    GT_U16 vlanId;

    /** VLAN id Assignment command. */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT vlanIdAssignCmd;

    /** @brief VLAN id Assignment precedence.
     *  Soft precedence - The VID assignment can be
     *  overridden by other subsequent
     *  mechanisms in the ingress pipeline.
     *  Hard precedence - The VID assignment cannot
     *  be overridden by other subsequent
     *  mechanisms in the ingress pipeline.
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT vlanIdAssignPrecedence;

} PRV_TGF_BRG_VLAN_PROT_CLASS_CFG_STC;

/**
* @struct PRV_TGF_BRG_QOS_PROT_CLASS_CFG_STC
 *
 * @brief Structure for configuring QOS Protocol based classification
 * parameters.
*/
typedef struct{

    /** Protocol VID assign QOS mode. */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT qosAssignCmd;

    /** @brief The port's QoSProfile. This is the port's
     *  default QoSProfile and may be assigned to the
     *  packet as part of the QoS assignment algorithm.
     */
    GT_U32 qosProfileId;

    /** @brief port marking of the QoS Precedence.
     *  Soft precedence - The QoS Profile assignment can
     *  be overridden by other subsequent mechanisms
     *  in the ingress pipeline.
     *  Hard precedence - The QoS Profile assignment
     *  cannot be overridden by other subsequent
     *  mechanisms in the ingress pipeline.
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;

    /** Enable/Disable modifing per QoS parameter */
    PRV_TGF_QOS_PARAM_MODIFY_STC qosParamsModify;

    /** QoS Parameters */
    PRV_TGF_QOS_PARAM_STC qosParams;

} PRV_TGF_BRG_QOS_PROT_CLASS_CFG_STC;

/**
* @enum PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT
 *
 * @brief This enum defines the egress ePort tag state.
*/
typedef enum{

    /** @brief Use tag state from
     *  the eVLAN entry, according to egress physical port
     */
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E,

    /** Use tag state from ePort entry */
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E

} PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT;


/**
* @enum PRV_TGF_BRG_FDB_QUEUE_TYPE_ENT
 *
 * @brief Enumeration of FDB message queues (AU/FU)
*/
typedef enum{

    /** type for AUQ (address update queue) */
    PRV_TGF_BRG_FDB_QUEUE_TYPE_AU_E,

    /** type for FUQ (FDB upload queue) */
    PRV_TGF_BRG_FDB_QUEUE_TYPE_FU_E

} PRV_TGF_BRG_FDB_QUEUE_TYPE_ENT;

/**
* @struct PRV_TGF_BRG_HOST_CNTR_STC
 *
 * @brief Structure of Host Group counters is maintained for a
 * CPU-configured MAC source and MAC destination addresses.
*/
typedef struct{

    /** @brief number of packets with a MAC DA matching the
     *  CPU-configured MAC DA.
     */
    GT_U32 gtHostInPkts;

    /** @brief number of packets with a MAC SA matching the
     *  CPU-configured MAC SA.
     */
    GT_U32 gtHostOutPkts;

    /** @brief number of Broadcast packets with a MAC SA
     *  matching the CPU-configured MAC SA.
     */
    GT_U32 gtHostOutBroadcastPkts;

    /** @brief number of Multicast packets with a MAC SA
     *  matching the CPU-configured MAC SA.
     */
    GT_U32 gtHostOutMulticastPkts;

} PRV_TGF_BRG_HOST_CNTR_STC;


/**
* @enum PRV_TGF_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT
 *
 * @brief Enumeration of TAG0/TAG1 TPID Select mode .
*/
typedef enum{

    /** @brief TPID of Tag0/Tag1 is
     *  selected according to <Egress Tag0/Tag1 TPID select>
     */
    PRV_TGF_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E,

    /** @brief TPID of Tag0/Tag1 is
     *  selected according to VID0/VID1 to TPID select table
     */
    PRV_TGF_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E

} PRV_TGF_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT;

/**
* @enum PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT
 *
 * @brief Enumeration of indexing mode to access VLAN related tables.
*/
typedef enum{

    /** indexing is based on VLAN value. */
    PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,

    /** indexing is based on Tag1 VID value. */
    PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E

} PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT;

/**
* @enum PRV_TGF_BRG_GEN_BYPASS_MODE_ENT
 *
 * @brief Enumeration of Bridge Bypass mode.
*/
typedef enum{

    /** bypass the whole bridge module except the SA learning. */
    PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E,

    /** bypass only the forwarding decision of the bridge. */
    PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E

} PRV_TGF_BRG_GEN_BYPASS_MODE_ENT;


/**
* @enum PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT
 *
 * @brief Enumeration of byte-count modes for MTU check feature.
*/
typedef enum{

    /** L3 bytecount is used for MTU check. */
    PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E,

    /** L2 bytecount is used for MTU check. */
    PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E

} PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT;


/**
* @enum PRV_TGF_BRG_VLAN_TRANSLATION_ENT
 *
 * @brief This enum defines kinds of ingress/egress VLAN translation
*/
typedef enum{

    /** @brief disable translation
     *  PRV_TGF_BRG_VLAN_TRANSLATION_TABLE_VID0_E    translate VID0, use
     *  Ingress/Egress Vlan Translation Table
     */
    PRV_TGF_BRG_VLAN_TRANSLATION_DISABLE_E = GT_FALSE,

    PRV_TGF_BRG_VLAN_TRANSLATION_VID0_E    = GT_TRUE,

    /** @brief translate to VID0,
     *  use eVLAN<SERVICEID>[23:12].
     *  Is relevant to egress translation only.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_BRG_VLAN_TRANSLATION_SID_VID0_E,

    /** @brief translate to
     *  VID0, VID1, use eVlan<SERVICEID>.
     *  Is relevant to egress translation only.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E

} PRV_TGF_BRG_VLAN_TRANSLATION_ENT;

/**
* @enum PRV_TGF_BRG_ARP_BCAST_CMD_MODE_ENT
 *
 * @brief Enumeration of ARP broadcast command mode.
*/
typedef enum{

    /** set ARP broadcast command for all ports */
    PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E,

    /** set ARP broadcast command for all VLANs */
    PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E

} PRV_TGF_BRG_ARP_BCAST_CMD_MODE_ENT;


/**
* @enum PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_ENT
 *
 * @brief Enumeration of number of bytes to pop
*/
typedef enum{

    /** pop none (do not pop any byte). */
    PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E    = 0,

    /** pop 4 bytes. */
    PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_4_E       = 1,

    /** pop 8 bytes. */
    PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_8_E       = 2,

    /** pop 6 bytes. */
    PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_6_E       = 3

} PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_ENT;

/**
* @struct PRV_TGF_BRG_VLAN_ENTRY_DATA_STC
 *
 * @brief Structure to hold all data of VLAN entry.
*/
typedef struct{

    /** ports members bitmap */
    CPSS_PORTS_BMP_STC portsMembers;

    /** ports tagging bitmap */
    CPSS_PORTS_BMP_STC portsTagging;

    /** is valid flag. */
    GT_BOOL isValid;

    /** VLAN info */
    PRV_TGF_BRG_VLAN_INFO_STC vlanInfo;

    /** tagging info */
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC taggingCmd;

} PRV_TGF_BRG_VLAN_ENTRY_DATA_STC;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/
/**
* @internal prvTgfBrgVlanVrfIdSet function
* @endinternal
*
* @brief   Updates VRF Id in HW vlan entry
*
* @param[in] vlanId                   - VLAN Id
* @param[in] vrfId                    - VRF Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgVlanVrfIdSet
(
    IN GT_U16                               vlanId,
    IN GT_U32                               vrfId
);

/**
* @internal prvTgfBrgVlanEntryWrite function
* @endinternal
*
* @brief   Builds and writes vlan entry to HW
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[in] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgVlanEntryWrite
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_PORTS_BMP_STC                  *portsMembersPtr,
    IN CPSS_PORTS_BMP_STC                  *portsTaggingPtr,
    IN PRV_TGF_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
);

/**
* @internal prvTgfBrgVlanEntryRead function
* @endinternal
*
* @brief   Read vlan entry.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
*
* @param[out] portsMembers             - (pointer to) bmp of ports members in vlan
*                                      CPU port supported
* @param[out] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      The parameter is relevant for DxCh1, DxCh2
*                                      and DxCh3 devices.
*                                      The parameter is relevant for xCat and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat and above devices with TR101 feature
*                                      support.
* @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
* @param[out] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*/
GT_STATUS prvTgfBrgVlanEntryRead
(
   IN GT_U8                                 devNum,
   IN GT_U16                                vlanId,
   OUT CPSS_PORTS_BMP_STC                  *portsMembers,
   OUT CPSS_PORTS_BMP_STC                  *portsTaggingPtr,
   OUT GT_BOOL                             *isValidPtr,
   OUT PRV_TGF_BRG_VLAN_INFO_STC           *vlanInfoPtr,
   OUT PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
);

/**
* @internal prvTgfBrgVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry
*
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong vid
*/
GT_STATUS prvTgfBrgVlanEntryInvalidate
(
    IN GT_U16               vlanId
);

/**
* @internal prvTgfBrgVlanMemberAdd function
* @endinternal
*
* @brief   Set specific member at VLAN entry
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
* @param[in] isTagged                 - GT_TRUE/GT_FALSE, to set the port as tagged/untagged
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgVlanMemberAdd
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32                portNum,
    IN GT_BOOL              isTagged
);

/**
* @internal prvTgfBrgVlanTagMemberAdd function
* @endinternal
*
* @brief   Set specific tagged member at VLAN entry
*
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
* @param[in] portTaggingCmd           - port tagging command
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgVlanTagMemberAdd
(
    IN GT_U16                            vlanId,
    IN GT_U32                             portNum,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT portTaggingCmd
);

/**
* @internal prvTgfBrgVlanMemberRemove function
* @endinternal
*
* @brief   Delete port member from vlan entry.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
*/
GT_STATUS prvTgfBrgVlanMemberRemove
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32                portNum
);

/**
* @internal prvTgfBrgVlanPortVidSet function
* @endinternal
*
* @brief   Set port's default VLAN Id.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanId                   - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
*/
GT_STATUS prvTgfBrgVlanPortVidSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    IN  GT_U16              vlanId
);

/**
* @internal prvTgfBrgVlanPortVidGet function
* @endinternal
*
* @brief   Get port's default VLAN Id
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vlanIdPtr                - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgVlanPortVidGet
(
    IN  GT_U8               devNum,
    IN  GT_U32               portNum,
    OUT GT_U16             *vlanIdPtr
);

/**
* @internal prvTgfBrgVlanPortVidPrecedenceSet function
* @endinternal
*
* @brief   Set Port VID Precedence
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] precedence               -  type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfBrgVlanPortVidPrecedenceSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                        portNum,
    IN CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence
);

/**
* @internal prvTgfBrgVlanPortForcePvidEnable function
* @endinternal
*
* @brief   Set Enable PVID Forcing
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   -  force PVid - GT_TRUE, disable - GT_FALSE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfBrgVlanPortForcePvidEnable
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgFdbMacEntryWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in specified index.
*
* @param[in] index                    - hw mac entry index
* @param[in] skip                     - entry  control
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryWrite
(
    IN GT_U32                        index,
    IN GT_BOOL                       skip,
    IN PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);

/**
* @internal prvTgfBrgFdbMacEntryRead function
* @endinternal
*
* @brief   Reads the new entry in Hardware MAC address table from specified index.
*
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
* @param[out] agedPtr                  - (pointer to) is entry aged
* @param[out] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parametres
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbMacEntryRead
(
    IN  GT_U32                        index,
    OUT GT_BOOL                      *validPtr,
    OUT GT_BOOL                      *skipPtr,
    OUT GT_BOOL                      *agedPtr,
    OUT GT_HW_DEV_NUM                *hwDevNumPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);

/**
* @internal prvTgfBrgFdbMacEntryInvalidatePerDev function
* @endinternal
*
* @brief   Invalidate an entry in Hardware MAC address table in specified index
*         of specified device.
*         the invalidation done by resetting to first word of the entry
* @param[in] devNum                   - a device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index                    - hw mac entry index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbMacEntryInvalidatePerDev
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               index
);

/**
* @internal prvTgfBrgFdbMacEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in Hardware MAC address table in specified index.
*         the invalidation done by resetting to first word of the entry
* @param[in] index                    - hw mac entry index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbMacEntryInvalidate
(
    IN GT_U32               index
);

/**
* @internal prvTgfBrgFdbEntryByMessageMustSucceedSet function
* @endinternal
*
* @brief   when add/delete FDB entry 'by message' operation can not succeed.
*         the operation MUST finish but is may not succeed.
*         An AU message sent by CPU can fail in the following cases:
*         1. The message type = CPSS_NA_E and the hash chain has reached it's
*         maximum length.
*         2. The message type = CPSS_QA_E or CPSS_AA_E and the FDB entry doesn't
*         exist.
* @param[in] mustSucceed              - GT_TRUE - operation must succeed
*                                      GT_FALSE - operation may fail (must finish but not succeed)
*                                       the previous state of the flag
*/
GT_BOOL prvTgfBrgFdbEntryByMessageMustSucceedSet
(
    IN GT_BOOL     mustSucceed
);

/**
* @internal prvTgfBrgFdbEntryByMessageStatusCheckSet function
* @endinternal
*
* @brief   The function enables/disables status of 'by message' API call.
*
* @param[in] mustCheck                - GT_TRUE - operation status must be checked
*                                      GT_FALSE - operation status must not be checked
*                                       the previous state of the flag
*/
GT_BOOL prvTgfBrgFdbEntryByMessageStatusCheckSet
(
    IN GT_BOOL     mustCheck
);

/**
* @internal prvTgfBrgFdbMacEntrySet function
* @endinternal
*
* @brief   Create new or update existing entry in Hardware MAC address table
*
* @param[in] macEntryPtr              - (pointer to) mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntrySet
(
    IN PRV_TGF_BRG_MAC_ENTRY_STC     *macEntryPtr
);

/**
* @internal prvTgfBrgFdbMacEntryGet function
* @endinternal
*
* @brief   Get existing entry in Hardware MAC address table
*
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] macEntryPtr              - (pointer to) mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryGet
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);

/**
* @internal prvTgfBrgFdbMacEntryDelete function
* @endinternal
*
* @brief   Delete an old entry in Hardware MAC address table
*
* @param[in] macEntryKeyPtr           - (pointer to) key parameters of the mac entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong vlanId
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryDelete
(
    IN PRV_TGF_MAC_ENTRY_KEY_STC     *macEntryKeyPtr
);

/**
* @internal prvTgfBrgFdbQaSend function
* @endinternal
*
* @brief   The function Send Query Address (QA) message to the hardware MAC address
*         table.
* @param[in] macEntryKeyPtr           - (pointer to) key parameters of the mac entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong vlanId
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbQaSend
(
    IN PRV_TGF_MAC_ENTRY_KEY_STC     *macEntryKeyPtr
);

/**
* @internal prvTgfBrgFdbFlush function
* @endinternal
*
* @brief   Flush FDB table (with/without static entries)
*
* @param[in] includeStatic            - include static entries
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
*/
GT_STATUS prvTgfBrgFdbFlush
(
    IN GT_BOOL                        includeStatic
);

/**
* @internal prvTgfBrgFdbActionStart function
* @endinternal
*
* @brief   Enable actions, sets action type, action mode to CPSS_ACT_TRIG_E and
*         starts triggered action by setting Aging Trigger.
*         This API may be used to start one of triggered actions: Aging, Deleting,
*         Transplanting and FDB Upload.
*         NOTE: caller should call prvTgfBrgFdbActionDoneWait(...) to check that
*         operation done before checking for the action outcomes...
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] mode                     - action mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous FDB triggered action is not completed yet
*                                       or CNC block upload not finished (or not all of it's
*                                       results retrieved from the common used FU and CNC
*                                       upload queue)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Deleting and Transplanting actions can be done only by CPU triggered
*       FDB action that activated by the function.
*       See below sequence of CPSS API calls for Deleting and
*       Transplanting actions.
*       Before making Delete or Transplant sequence the application need to
*       disable Actions to avoid wrong automatic aging.
*       It is Application responsibility to get and store all parameters
*       that are changed during those actions.
*       1. AA message to CPU status by cpssDxChBrgFdbAAandTAToCpuGet.
*       2. Action Active configuration by
*       cpssDxChBrgFdbActionActiveInterfaceGet,
*       cpssDxChBrgFdbActionActiveDevGet,
*       cpssDxChBrgFdbActionActiveVlanGet.
*       3. Action mode and trigger mode by cpssDxChBrgFdbActionModeGet
*       cpssDxChBrgFdbActionTriggerModeGet.
*       The AA and TA messages may be disabled before the FDB action
*       4. Disable AA and TA messages to CPU by cpssDxChBrgFdbAAandTAToCpuSet.
*       5. Set Active configuration by: cpssDxChBrgFdbActionActiveInterfaceSet,
*       cpssDxChBrgFdbActionActiveVlanSet and cpssDxChBrgFdbActionActiveDevSet.
*       6. Start triggered action by cpssDxChBrgFdbTrigActionStart
*       7. Wait that triggered action is completed by:
*       - Busy-wait poling of status - cpssDxChBrgFdbTrigActionStatusGet
*       - Wait of event CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E.
*       This wait may be done in context of dedicated task to restore
*       Active configuration and AA messages configuration.
*       for multi-port groups device :
*       the trigger is done on ALL port groups regardless to operation mode
*       (Trigger aging/transplant/delete/upload).
*       see also description of function cpssDxChBrgFdbActionModeSet about
*       'multi-port groups device':
*
*/
GT_STATUS prvTgfBrgFdbActionStart
(
    IN PRV_TGF_FDB_ACTION_MODE_ENT  mode
);

/**
* @internal prvTgfBrgFdbActionDoneWait function
* @endinternal
*
* @brief   wait for the FDB triggered action to end
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] aaTaClosed               - indication that the caller closed the AA,TA before the
*                                      action -->
*                                      GT_TRUE - the caller closed AA,TA before calling
*                                      prvTgfBrgFdbActionStart(...)
*                                      so action should be ended quickly.
*                                      timeout is set to 50 milliseconds
*                                      GT_FALSE - the caller NOT closed AA,TA before calling
*                                      prvTgfBrgFdbActionStart(...)
*                                      so action may take unpredictable time.
*                                      timeout is set to 50 seconds !!!
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_TIMEOUT               - after timed out. see timeout definition
*                                       according to aaTaClosed parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionDoneWait
(
    IN  GT_BOOL  aaTaClosed
);

/**
* @internal prvTgfBrgFdbActionModeSet function
* @endinternal
*
* @brief   Sets FDB action mode without setting Action Trigger
*
* @param[in] mode                     - FDB action mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionModeSet
(
    IN PRV_TGF_FDB_ACTION_MODE_ENT  mode
);

/**
* @internal prvTgfBrgFdbActionModeGet function
* @endinternal
*
* @brief   Gets FDB action mode.
*
* @param[out] modePtr                  - FDB action mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionModeGet
(
    OUT PRV_TGF_FDB_ACTION_MODE_ENT     *modePtr
);

/**
* @internal prvTgfBrgFdbAgingTimeoutSet function
* @endinternal
*
* @brief   Sets the timeout period in seconds for aging out dynamically learned
*         forwarding information. The standard recommends 300 sec.
* @param[in] timeout                  - aging time in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbAgingTimeoutSet
(
    IN GT_U32 timeout
);

/**
* @internal prvTgfBrgFdbAgingTimeoutGet function
* @endinternal
*
* @brief   Gets the timeout period in seconds for aging out dynamically learned
*         forwarding information. The standard recommends 300 sec.
*
* @param[out] timeoutPtr               - (pointer to)aging time in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbAgingTimeoutGet
(
    OUT GT_U32 *timeoutPtr
);


/**
* @internal prvTgfBrgFdbAgeBitDaRefreshEnableSet function
* @endinternal
*
* @brief   Enables/disables destination address-based aging. When this bit is
*         set, the aging process is done both on the source and the destination
*         address (i.e. the age bit will be refresh when MAC DA hit occurs, as
*         well as MAC SA hit).
* @param[in] enable                   - GT_TRUE -  refreshing
*                                      GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       COMMENTS
*/
GT_STATUS prvTgfBrgFdbAgeBitDaRefreshEnableSet
(
    IN GT_BOOL enable
);

/**
* @internal prvTgfBrgFdbMacTriggerModeSet function
* @endinternal
*
* @brief   Sets Mac address table Triggered\Automatic action mode.
*
* @param[in] mode                     - action mode:
*                                      PRV_TGF_ACT_AUTO_E - Action is done Automatically.
*                                      PRV_TGF_ACT_TRIG_E - Action is done via a trigger from CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacTriggerModeSet
(
    IN PRV_TGF_MAC_ACTION_MODE_ENT  mode
);

/**
* @internal prvTgfBrgFdbMacTriggerModeGet function
* @endinternal
*
* @brief   Get Mac address table Triggered\Automatic action mode.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to action mode:
*                                      PRV_TGF_ACT_AUTO_E - Action is done Automatically.
*                                      PRV_TGF_ACT_TRIG_E - Action is done via a trigger from CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacTriggerModeGet
(
    IN GT_U8                        devNum,
    IN PRV_TGF_MAC_ACTION_MODE_ENT  *modePtr
);

/**
* @internal prvTgfBrgFdbActionsEnableSet function
* @endinternal
*
* @brief   Enables/Disables FDB actions.
*
* @param[in] enable                   - GT_TRUE -  Actions are enabled
*                                      GT_FALSE -  Actions are disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionsEnableSet
(
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgFdbActionsEnableGet function
* @endinternal
*
* @brief   Get the status of FDB actions: flushing, deleting, uploading and
*         transplanting.
*
* @note   APPLICABLE DEVICES:      Puma2; Puma3.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] enablePtr                - GT_TRUE - actions are enabled
*                                      GT_FALSE - actions are disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvTgfBrgFdbActionsEnableGet
(
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgFdbMacEntryStatusGet function
* @endinternal
*
* @brief   Get the Valid and Skip Values of a FDB entry.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacEntryStatusGet
(
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr
);

/**
* @internal prvTgfBrgFdbMacEntryAgeBitSet function
* @endinternal
*
* @brief   Set age bit in specific FDB entry.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] index                    - hw mac entry index
* @param[in] age                      - Age flag that is used for the two-step Aging process.
*                                      GT_FALSE - The entry will be aged out in the next pass.
*                                      GT_TRUE - The entry will be aged-out in two age-passes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacEntryAgeBitSet
(
    IN GT_U32       index,
    IN GT_BOOL      age
);

/**
* @internal prvTgfBrgFdbDump function
* @endinternal
*
* @brief   This function dumps valid FDB entries.
*         in multi ports port groups device the function summarize the entries
*         from all the port groups (don't care about duplications)
*/
GT_STATUS prvTgfBrgFdbDump
(
    void
);

/**
* @internal prvTgfBrgFdbPortGroupCount function
* @endinternal
*
* @brief   This function count number of valid ,skip entries - for specific port group
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupId              - port group Id
*                                      when CPSS_PORT_GROUP_UNAWARE_MODE_CNS meaning read from first port group
*
* @param[out] numOfValidPtr            - (pointer to) number of entries with valid bit set
*                                      (number of valid entries)
* @param[out] numOfSkipPtr             - (pointer to) number of entries with skip bit set
*                                      (number of skipped entries)
*                                      entry not counted when valid = 0
* @param[out] numOfAgedPtr             - (pointer to) number of entries with age bit = 0 !!!
*                                      (number of aged out entries)
*                                      entry not counted when valid = 0 or skip = 1
* @param[out] numOfStormPreventionPtr  - (pointer to) number of entries with SP bit set
*                                      (number of SP entries)
*                                      entry not counted when valid = 0 or skip = 1
* @param[out] numOfErrorsPtr           - (pointer to) number of entries with read error
*                                       None.
*/
GT_STATUS prvTgfBrgFdbPortGroupCount
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,

    OUT GT_U32  *numOfValidPtr,
    OUT GT_U32  *numOfSkipPtr,
    OUT GT_U32  *numOfAgedPtr,
    OUT GT_U32  *numOfStormPreventionPtr,
    OUT GT_U32  *numOfErrorsPtr
);

/**
* @internal prvTgfBrgFdbCount function
* @endinternal
*
* @brief   This function count number of valid ,skip entries.
*         in multi ports port groups device the function summarize the entries
*         from all the port groups (don't care about duplications)
*
* @param[out] numOfValidPtr            - (pointer to) number of entries with valid bit set
*                                      (number of valid entries)
* @param[out] numOfSkipPtr             - (pointer to) number of entries with skip bit set
*                                      (number of skipped entries)
*                                      entry not counted when valid = 0
* @param[out] numOfAgedPtr             - (pointer to) number of entries with age bit = 0 !!!
*                                      (number of aged out entries)
*                                      entry not counted when valid = 0 or skip = 1
* @param[out] numOfStormPreventionPtr  - (pointer to) number of entries with SP bit set
*                                      (number of SP entries)
*                                      entry not counted when valid = 0 or skip = 1
* @param[out] numOfErrorsPtr           - (pointer to) number of entries with read error
*                                       None.
*/
GT_STATUS prvTgfBrgFdbCount
(
    OUT GT_U32  *numOfValidPtr,
    OUT GT_U32  *numOfSkipPtr,
    OUT GT_U32  *numOfAgedPtr,
    OUT GT_U32  *numOfStormPreventionPtr,
    OUT GT_U32  *numOfErrorsPtr
);

/**
* @internal prvTgfBrgFdbMacCompare function
* @endinternal
*
* @brief   compare 2 mac entries.
*
* @param[in] mac1Ptr                  - (pointer to) mac 1
* @param[in] mac2Ptr                  - (pointer to) mac 2
*                                      OUTPUTS:
*                                      > 0  - if mac1Ptr is  bigger than mac2Ptr
*                                      == 0 - if mac1Ptr is equal to mac2Ptr
*                                      < 0  - if mac1Ptr is smaller than mac2Ptr
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_FAIL                  - on error
*/
int prvTgfBrgFdbMacCompare
(
    IN  PRV_TGF_BRG_MAC_ENTRY_STC    *mac1Ptr,
    IN  PRV_TGF_BRG_MAC_ENTRY_STC    *mac2Ptr
);

/**
* @internal prvTgfBrgFdbMacKeyCompare function
* @endinternal
*
* @brief   compare 2 mac keys.
*
* @param[in] key1Ptr                  - (pointer to) key 1
* @param[in] key2Ptr                  - (pointer to) key 2
*                                      OUTPUTS:
*                                      > 0  - if key1Ptr is  bigger than key2Ptr
*                                      == 0 - if key1Ptr is equal to str2
*                                      < 0  - if key1Ptr is smaller than key2Ptr
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_FAIL                  - on error
*/
int prvTgfBrgFdbMacKeyCompare
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *key1Ptr,
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *key2Ptr
);

/**
* @internal prvTgfBrgFdbMacEntryHashCalc function
* @endinternal
*
* @brief   calculate the hash index for the key
*         NOTE:
*         see also function prvTgfBrgFdbMacEntryIndexFind(...) that find actual
*         place according to actual HW capacity
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] indexPtr                 - (pointer to) index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryHashCalc
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT GT_U32                       *indexPtr
);

/**
* @internal prvTgfBrgFdbMacEntryIndexFind function
* @endinternal
*
* @brief   function calculate hash index for this mac address , and then start to read
*         from HW the entries to find an existing entry that match the key.
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] indexPtr                 - (pointer to) :
*                                      when GT_OK (found) --> index of the entry
*                                      when GT_NOT_FOUND (not found) --> first index that can be used
*                                      when GT_FULL (not found) --> index od start of bucket
*                                      on other return value --> not relevant
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfBrgFdbMacEntryIndexFind
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT GT_U32                       *indexPtr
);

/**
* @internal prvTgfBrgFdbMacEntryFind function
* @endinternal
*
* @brief   scan the FDB table and find entry with same mac+vlan
*         entry MUST be valid and non-skipped
* @param[in] searchKeyPtr             - (pointer to) the search key (mac+vlan)
*
* @param[out] indexPtr                 - (pointer to) index
*                                      when NULL --> ignored
* @param[out] agedPtr                  - (pointer to) is entry aged
*                                      when NULL --> ignored
* @param[out] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
*                                      when NULL --> ignored
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*                                      when NULL --> ignored
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbMacEntryFind
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *searchKeyPtr,
    OUT GT_U32                       *indexPtr,
    OUT GT_BOOL                      *agedPtr,
    OUT GT_HW_DEV_NUM                *hwDevNumPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);


/**
* @internal prvTgfBrgFdbAccessModeSet function
* @endinternal
*
* @brief   Set access mode to the FDB : by message or by index
*
* @param[in] mode                     - access  : by message or by index
*                                       previous state
*/
PRV_TGF_BRG_FDB_ACCESS_MODE_ENT prvTgfBrgFdbAccessModeSet
(
    IN PRV_TGF_BRG_FDB_ACCESS_MODE_ENT  mode
);


/**
* @internal prvTgfBrgVlanLearningStateSet function
* @endinternal
*
* @brief   Sets state of VLAN based learning to specified VLAN on specified device
*
* @param[in] vlanId                   - vlan Id
* @param[in] status                   - GT_TRUE for enable  or GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong vid
*/
GT_STATUS prvTgfBrgVlanLearningStateSet
(
    IN GT_U16                         vlanId,
    IN GT_BOOL                        status
);

/**
* @internal prvTgfBrgVlanNaToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable New Address (NA) Message Sending to CPU per VLAN .
*         To send NA to CPU both VLAN and port must be set to send NA to CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanNaToCpuEnable
(
    IN GT_U16                         vlanId,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfBrgMcMemberAdd function
* @endinternal
*
* @brief   Add new port member to the Multicast Group entry
*
* @param[in] devNum                   - PP's device number
* @param[in] vidx                     - multicast group index
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value
*/
GT_STATUS prvTgfBrgMcMemberAdd
(
    IN GT_U8                          devNum,
    IN GT_U16                         vidx,
    IN GT_U32                          portNum
);

/**
* @internal prvTgfBrgMcEntryWrite function
* @endinternal
*
* @brief   Writes Multicast Group entry to the HW
*
* @param[in] devNum                   - PP's device number
* @param[in] vidx                     - multicast group index
* @param[in] portBitmapPtr            - pointer to the bitmap of ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or ports bitmap value.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
*/
GT_STATUS prvTgfBrgMcEntryWrite
(
    IN GT_U8                          devNum,
    IN GT_U16                         vidx,
    IN CPSS_PORTS_BMP_STC            *portBitmapPtr
);

/**
* @internal prvTgfBrgMcEntryRead function
* @endinternal
*
* @brief   Reads the Multicast Group entry from the HW
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index
*
* @param[out] portBitmapPtr            - pointer to the bitmap of ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfBrgMcEntryRead
(
    IN  GT_U8                         devNum,
    IN  GT_U16                        vidx,
    OUT CPSS_PORTS_BMP_STC           *portBitmapPtr
);

/**
* @internal prvTgfBrgVlanEgressFilteringEnable function
* @endinternal
*
* @brief   Enable/Disable VLAN Egress Filtering on specified device for Bridged
*         Known Unicast packets
* @param[in] enable                   - enable\disable VLAN egress filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
*/
GT_STATUS prvTgfBrgVlanEgressFilteringEnable
(
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfBrgVlanEgressFilteringEnableGet function
* @endinternal
*
* @brief   This function gets the VLAN Egress Filtering current state (enable/disable)
*         on specified device for Bridged Known Unicast packets.
*         If enabled the VLAN egress filter verifies that the egress port is a
*         member of the VID assigned to the packet.
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to (enable/disable) bridged known unicast packets filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgVlanEgressFilteringEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal prvTgfBrgCntDropCntrModeSet function
* @endinternal
*
* @brief   Sets Drop Counter Mode
*
* @param[in] dropMode                 - Drop Counter mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong dropMode
* @retval GT_NOT_SUPPORTED         - on non-supported dropMode
*/
GT_STATUS prvTgfBrgCntDropCntrModeSet
(
    IN  PRV_TGF_BRG_DROP_CNTR_MODE_ENT dropMode
);

/**
* @internal prvTgfBrgCntDropCntrModeGet function
* @endinternal
*
* @brief   Gets the Drop Counter Mode
*
* @param[out] dropModePtr              - (pointer to) the Drop Counter mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on non-supported dropMode
*/
GT_STATUS prvTgfBrgCntDropCntrModeGet
(
    OUT  PRV_TGF_BRG_DROP_CNTR_MODE_ENT *dropModePtr
);

/**
* @internal prvTgfBrgVlanPortIngFltEnable function
* @endinternal
*
* @brief   Enable/disable Ingress Filtering for specific port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable\disable ingress filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*/
GT_STATUS prvTgfBrgVlanPortIngFltEnable
(
    IN GT_U8                          devNum,
    IN GT_PORT_NUM                    portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfBrgVlanPortIngFltEnableGet function
* @endinternal
*
* @brief   Get status of Ingress Filtering for specific port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - Pointer to Ingress Filtering status
*                                      - GT_TRUE, ingress filtering is enabled
*                                      - GT_FALSE, ingress filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortIngFltEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal prvTgfBrgVlanL2VpnIngressFilterEnableSet function
* @endinternal
*
* @brief   Enable/disable STP and VLAN Ingress Filtering for L2 VPN traffic.
*
* @note   APPLICABLE DEVICES:      Puma3.
* @note   NOT APPLICABLE DEVICES:  Puma2.
*
* @param[in] enable                   - GT_TRUE  -  STP and VLAN Ingress Filtering
*                                      for L2 VPN traffic.
*                                      GT_FALSE - disable STP and VLAN Ingress Filtering
*                                      for L2 VPN traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanL2VpnIngressFilterEnableSet
(
    IN GT_BOOL enable
);

/**
* @internal prvTgfBrgCntDropCntrSet function
* @endinternal
*
* @brief   Sets the Bridge Ingress Drop Counter of specified device
*
* @param[in] dropCnt                  - the number of packets that were dropped
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfBrgCntDropCntrSet
(
    IN  GT_U32                        dropCnt
);

/**
* @internal prvTgfBrgCntDropCntrGet function
* @endinternal
*
* @brief   Gets the Bridge Ingress Drop Counter of specified device
*
* @param[out] dropCntPtr               - (pointer to) the number of packets that were dropped
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgCntDropCntrGet
(
    OUT GT_U32                       *dropCntPtr
);

/**
* @internal prvTgfBrgVlanLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Function sets local switching of Multicast, known and unknown Unicast,
*         and Broadcast traffic per VLAN.
* @param[in] vlanId                   - VLAN id
* @param[in] trafficType              - local switching traffic type
* @param[in] enable                   - enable/disable of local switching
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanLocalSwitchingEnableSet
(
    IN  GT_U16                                              vlanId,
    IN  PRV_TGF_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT   trafficType,
    IN  GT_BOOL                                             enable
);

/**
* @internal prvTgfBrgGenUcLocalSwitchingEnable function
* @endinternal
*
* @brief   Enable/disable local switching back through the ingress interface
*         for for known Unicast packets
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk
*                                      are assigned with a soft drop command.
*                                      GT_TRUE  = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk may be
*                                      forwarded back to their source.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenUcLocalSwitchingEnable
(
    IN GT_U8                          devNum,
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfBrgPortEgressMcLocalEnable function
* @endinternal
*
* @brief   Enable/Disable sending Multicast packets back to its source
*         port on the local device.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - Multicast packets may be sent back to
*                                      their source port on the local device.
*                                      GT_FALSE - Multicast packets are not sent back to
*                                      their source port on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgPortEgressMcLocalEnable
(
    IN GT_U8                          devNum,
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfBrgPortEgressMcLocalEnableGet function
* @endinternal
*
* @brief   Get the current status of Multicast packets local switching.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - points to (enable/disable) sending Multicast packets
*                                      back to its source
*                                      GT_TRUE  - Multicast packets may be sent back to
*                                      their source port on the local device.
*                                      GT_FALSE - Multicast packets are not sent back to
*                                      their source port on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgPortEgressMcLocalEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_U32               portNum,
    OUT GT_BOOL             *enablePtr
);

/**
* @internal prvTgfBrgEgrFltVlanPortFilteringEnableSet function
* @endinternal
*
* @brief   Enable/Disable ePort VLAN Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - disable ePort VLAN Egress Filtering.
*                                      GT_TRUE  - enable ePort VLAN Egress Filtering.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgEgrFltVlanPortFilteringEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal prvTgfBrgVlanRangeSet function
* @endinternal
*
* @brief   Function configures the valid VLAN Range
*
* @param[in] vidRange                 - VID range for VLAN filtering (0 - 4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - illegal vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanRangeSet
(
    IN  GT_U16                        vidRange
);

/**
* @internal prvTgfBrgVlanRangeGet function
* @endinternal
*
* @brief   Function gets the valid VLAN Range.
*
* @param[out] vidRangePtr              - (pointer to) VID range for VLAN filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - vidRangePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanRangeGet
(
    OUT GT_U16                       *vidRangePtr
);


/******************************************************************************\
 *                       API for default settings                             *
\******************************************************************************/

/**
* @internal prvTgfBrgDefVlanEntryWriteWithTaggingCmd function
* @endinternal
*
* @brief   Set vlan entry, with the all ports of the test as
*         in the vlan with specified tagging command
* @param[in] vlanId                   - the vlan Id
* @param[in] taggingCmd               - GT_TRUE - tagging command for all members
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefVlanEntryWriteWithTaggingCmd
(
    IN GT_U16                             vlanId,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT  taggingCmd
);

/**
* @internal prvTgfBrgDefVlanEntryWrite function
* @endinternal
*
* @brief   Set vlan entry, with the all ports of the test as untagged in the vlan
*
* @param[in] vlanId                   - the vlan Id
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefVlanEntryWrite
(
    IN GT_U16                         vlanId
);

/**
* @internal prvTgfBrgDefVlanEntryWithPortsSet function
* @endinternal
*
* @brief   Set vlan entry, with the requested ports, tagging
*
* @param[in] vlanId                   - the vlan id
* @param[in] portsArray[]             - array of ports
* @param[in] devArray[]               - array of devices (when NULL -> assume all ports belong to devNum)
* @param[in] tagArray[]               - array of tagging for the ports (when NULL -> assume all ports untagged)
* @param[in] numPorts                 - number of ports in the arrays
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefVlanEntryWithPortsSet
(
    IN GT_U16                         vlanId,
    IN GT_U32                          portsArray[],
    IN GT_U8                          devArray[],
    IN GT_U8                          tagArray[],
    IN GT_U32                         numPorts
);

/**
* @internal prvTgfBrgDefVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry and reset all it's content
*
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong vid
*/
GT_STATUS prvTgfBrgDefVlanEntryInvalidate
(
    IN GT_U16               vlanId
);

/**
* @internal prvTgfBrgVlanMruProfileIdxSet function
* @endinternal
*
* @brief   Set Maximum Receive Unit MRU profile index for a VLAN.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         Value of MRU for profile is set by cpssDxChBrgVlanMruProfileValueSet.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
* @param[in] vlanId                   - vlan id
* @param[in] mruIndex                 - MRU profile index [0..7]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMruProfileIdxSet
(
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
);


/**
* @internal prvTgfBrgVlanMruProfileIdxRestore function
* @endinternal
*
* @brief   Restore Maximum Receive Unit MRU profile index for a VLAN/Inlif.
*
* @param[in] vlanId                   - vlan id
* @param[in] mruIndex                 - MRU profile index [0..7]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMruProfileIdxRestore
(
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
);

/**
* @internal prvTgfBrgVlanMruProfileValueSet function
* @endinternal
*
* @brief   Set MRU value for a VLAN MRU profile.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         cpssDxChBrgVlanMruProfileIdxSet set index of profile for a specific VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
* @param[in] mruIndex                 - MRU profile index [0..7]
* @param[in] mruValue                 - MRU value in bytes [0..0xFFFF]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMruProfileValueSet
(
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
);

/**
* @internal prvTgfBrgDefFdbMacEntryOnPortSet function
* @endinternal
*
* @brief   Set FDB entry, as 'mac address + vlan' on port
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] dstDevNum                - destination device
* @param[in] dstPortNum               - destination port
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong vid
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryOnPortSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_HW_DEV_NUM                  dstDevNum,
    IN GT_PORT_NUM                    dstPortNum,
    IN GT_BOOL                        isStatic
);

/**
* @internal prvTgfBrgDefFdbMacEntryOnVidxSet function
* @endinternal
*
* @brief   Set FDB entry, as 'mac address + vlan' on Vidx
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] vidx                     -  number
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong vid
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryOnVidxSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_U16                         vidx,
    IN GT_BOOL                        isStatic
);

/**
* @internal prvTgfBrgDefFdbMacEntryOnTrunkSet function
* @endinternal
*
* @brief   Set FDB entry, as 'mac address + vlan' on Trunk
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] trunkId                  - trunk Id
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryOnTrunkSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_U16                         trunkId,
    IN GT_BOOL                        isStatic
);

/**
* @internal prvTgfBrgDefFdbMacEntryOnVidSet function
* @endinternal
*
* @brief   Set FDB entry, as 'mac address + vlan' on Vlan ID
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] vid                      - vlan Id
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong vid
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryOnVidSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_U16                         vid,
    IN GT_BOOL                        isStatic
);

/**
* @internal prvTgfBrgVidxEntrySet function
* @endinternal
*
* @brief   Set vidx entry with the requested ports
*
* @param[in] vidx                     -  number
* @param[in] portsArray[]             - array of ports
* @param[in] devicesArray[]           - array of devices
* @param[in] numPorts                 - number of ports in the arrays
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong vid
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note When devicesArray is NULL assume all ports belong to devNum
*
*/
GT_STATUS prvTgfBrgVidxEntrySet
(
    IN GT_U16                         vidx,
    IN GT_U32                          portsArray[],
    IN GT_U8                          devicesArray[],
    IN GT_U8                          numPorts
);

/**
* @internal prvTgfBrgVlanPvidSet function
* @endinternal
*
* @brief   Set port's Vlan Id to all tested ports
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong vid
*/
GT_STATUS prvTgfBrgVlanPvidSet
(
    IN GT_U16                         pvid
);

/**
* @internal prvTgfBrgVlanTpidEntrySet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) table entry.
*
* @param[in] direction                - ingress/egress direction
* @param[in] entryIndex               - entry index for TPID table
* @param[in] etherType                - Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong entryIndex, direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTpidEntrySet
(
    IN  CPSS_DIRECTION_ENT  direction,
    IN  GT_U32              entryIndex,
    IN  GT_U16              etherType
);

/**
* @internal prvTgfBrgVlanTpidEntryGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) table entry.
*
* @param[in] direction                - ingress/egress direction
* @param[in] entryIndex               - entry index for TPID table
*
* @param[out] etherTypePtr             - (pointer to) Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong entryIndex, direction
* @retval GT_BAD_PTR               - etherTypePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTpidEntryGet
(
    IN  CPSS_DIRECTION_ENT  direction,
    IN  GT_U32              entryIndex,
    OUT GT_U16              *etherTypePtr
);

/**
* @internal prvTgfBrgVlanPortEgressTpidSet function
* @endinternal
*
* @brief   Function sets index of TPID (Tag protocol ID) table per egress port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] tpidEntryIndex           - TPID table entry index (0-7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortEgressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidEntryIndex
);

/**
* @internal prvTgfBrgVlanPortEgressTpidGet function
* @endinternal
*
* @brief   Function gets index of TPID (Tag protocol ID) table per egress port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID table entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortEgressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidEntryIndexPtr
);

/**
* @internal prvTgfBrgVlanEgressTagTpidSelectModeSet function
* @endinternal
*
* @brief   Set Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] mode                     - 0: TPID of Tag0/1 is selected according to
*                                      <Egress Tag 0/1 TPID select>
*                                      1: TPID of Tag0/1 is selected according to
*                                      VID0/1 to TPID select table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressTagTpidSelectModeSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_ETHER_MODE_ENT                          ethMode,
    IN PRV_TGF_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT    mode
);

/**
* @internal prvTgfBrgVlanBridgingModeSet function
* @endinternal
*
* @brief   Set bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
*         The device supports a VLAN-unaware mode for 802.1D bridging.
*         When this mode is enabled:
*         - In VLAN-unaware mode, the device does not perform any packet
*         modifications. Packets are always transmitted as-received, without any
*         modification (i.e., packets received tagged are transmitted tagged;
*         packets received untagged are transmitted untagged).
*         - Packets are implicitly assigned with VLAN-ID 1, regardless of
*         VLAN-assignment mechanisms.
*         - Packets are implicitly assigned a VIDX Multicast group index 0xFFF,
*         indicating that the packet flood domain is according to the VLAN-in
*         this case VLAN 1. Registered Multicast is not supported in this mode.
*         All other features are operational in this mode, including internal
*         packet QoS, trapping, filtering, policy, etc.
* @param[in] brgMode                  - bridge mode: IEEE 802.1Q bridge or IEEE 802.1D bridge
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanBridgingModeSet
(
    IN CPSS_BRG_MODE_ENT    brgMode
);


/**
* @internal prvTgfBrgVlanPortTranslationEnableSet function
* @endinternal
*
* @brief   Enable/Disable Vlan Translation per ingress or egress port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - port number or CPU port number for egress direction.
* @param[in] direction                - ingress or egress
* @param[in] enable                   - enable/disable ingress/egress Vlan Translation.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortTranslationEnableSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_NUM                        portNum,
    IN CPSS_DIRECTION_ENT                 direction,
    IN PRV_TGF_BRG_VLAN_TRANSLATION_ENT   enable
);

/**
* @internal prvTgfBrgVlanPortTranslationEnableGet function
* @endinternal
*
* @brief   Get the status of Vlan Translation (Enable/Disable) per ingress or
*         egress port.
* @param[in] devNum                   - the device number
* @param[in] direction                - ingress or egress
* @param[in] portNum                  - port number or CPU port number for egress direction.
*
* @param[out] enablePtr                - (pointer to) ingress/egress Vlan Translation status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortTranslationEnableGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  CPSS_DIRECTION_ENT                 direction,
    OUT PRV_TGF_BRG_VLAN_TRANSLATION_ENT   *enablePtr
);

/**
* @internal prvTgfBrgVlanTranslationEntryWrite function
* @endinternal
*
* @brief   Write an entry into Vlan Translation Table Ingress or Egress
*         This mapping avoids collisions between a VLAN-ID
*         used for a logical interface and a VLAN-ID used by the local network.
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table
*                                      (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..8191)
* @param[in] direction                - ingress or egress
* @param[in] transVlanId              - Translated Vlan ID, use as value in the
*                                      Translation Table (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..8191)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when transVlanId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTranslationEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U16                       transVlanId
);

/**
* @internal prvTgfBrgVlanTranslationEntryRead function
* @endinternal
*
* @brief   Read an entry from Vlan Translation Table, Ingress or Egress.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table, 0..4095.
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..8191)
* @param[in] direction                - ingress or egress
*
* @param[out] transVlanIdPtr           - (pointer to) Translated Vlan ID, used as value in the
*                                      Translation Table.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTranslationEntryRead
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    OUT GT_U16                      *transVlanIdPtr
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastTrapEnable function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
* @param[in] dev                      - physical device number
* @param[in] enable                   - GT_FALSE -
*                                      Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE -
*                                      Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenIeeeReservedMcastTrapEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet function
* @endinternal
*
* @brief   Select the IEEE Reserved Multicast profile (table) associated with port
*
* @param[in] dev                      - device number
* @param[in] port                     - number (including CPU port)
* @param[in] profileIndex             - profile index (0..3). The parameter defines profile (table
*                                      number) for the 256 protocols.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev, port
* @retval GT_OUT_OF_RANGE          - for profileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet
(
    IN GT_U8                dev,
    IN GT_U32                port,
    IN GT_U32               profileIndex
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastProtCmdSet function
* @endinternal
*
* @brief   Enables forwarding, trapping, or mirroring to the CPU any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
* @param[in] dev                      - device number
* @param[in] profileIndex             - profile index (0..3). Parameter is relevant only for
*                                      DxChXcat and above. The parameter defines profile (table
*                                      number) for the 256 protocols. Profile bound per port by
*                                      <prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet>.
* @param[in] protocol                 - specifies the Multicast protocol
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, protocol or control packet command
*                                       of profileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note BPDU packets with MAC DA = 01-80-C2-00-00-00 are not affect
*       by this mechanism. BPDUs are implicitly trapped to the CPU
*       if the ingress port span tree is not disabled.
*
*/
GT_STATUS prvTgfBrgGenIeeeReservedMcastProtCmdSet
(
    IN GT_U8                dev,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal prvTgfBrgSrcIdPortSrcIdForceEnableSet function
* @endinternal
*
* @brief   Set Source ID Assignment force mode per Ingress Port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number, CPU port
* @param[in] enable                   - enable/disable SourceID force mode
*                                      GT_TRUE - enable Source ID force mode
*                                      GT_FALSE - disable Source ID force mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortSrcIdForceEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgSrcIdPortSrcIdForceEnableGet function
* @endinternal
*
* @brief   Set Source ID Assignment force mode per Ingress Port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] enablePtr                - (pointer to) Source ID Assignment force mode state
*                                      GT_TRUE - Source ID force mode is enabled
*                                      GT_FALSE - Source ID force mode is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortSrcIdForceEnableGet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgSrcIdGlobalSrcIdAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode.
*
* @param[in] mode                     - the assignment  of the packet Source ID.
*                                      CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E mode is
*                                      supported only for DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdGlobalSrcIdAssignModeSet
(
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
);

/**
* @internal prvTgfBrgSrcIdPortDefaultSrcIdSet function
* @endinternal
*
* @brief   Configure Port's Default Source ID.
*         The Source ID is used for source based egress filtering.
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number, CPU port
* @param[in] defaultSrcId             - Port's Default Source ID (0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortDefaultSrcIdSet
(
    IN GT_U8   devNum,
    IN GT_U32   portNum,
    IN GT_U32  defaultSrcId
);

/**
* @internal prvTgfBrgSrcIdPortDefaultSrcIdGet function
* @endinternal
*
* @brief   Get configuration of Port's Default Source ID.
*         The Source ID is used for Source based egress filtering.
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number, CPU port
*
* @param[out] defaultSrcIdPtr          - pointer to Port's Default Source ID (0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortDefaultSrcIdGet
(
    IN  GT_U8   devNum,
    IN  GT_U32   portNum,
    OUT GT_U32  *defaultSrcIdPtr
);

/**
* @internal prvTgfBrgSrcIdGroupPortDelete function
* @endinternal
*
* @brief   Delete a port from Source ID group. Packets assigned with this
*         Source ID will not be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is added to (0..31).
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdGroupPortDelete
(
    IN GT_U8   devNum,
    IN GT_U32  sourceId,
    IN GT_U32   portNum
);

/**
* @internal prvTgfBrgSrcIdPortUcastEgressFilterSet function
* @endinternal
*
* @brief   Per Egress Port enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number/all ports wild card, CPU port
* @param[in] enable                   - GT_TRUE -  Source ID filtering on unicast packets
*                                      forwarded to this port. Unicast packet is dropped
*                                      if egress port is not member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortUcastEgressFilterSet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgSrcIdGroupPortAdd function
* @endinternal
*
* @brief   Add a port to Source ID group. Packets assigned with this Source ID
*         may be forwarded to this port.
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is added to (0..31).
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdGroupPortAdd
(
    IN GT_U8   devNum,
    IN GT_U32  sourceId,
    IN GT_U32   portNum
);

/**
* @internal prvTgfBrgSrcIdPortAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode for non-DSA tagged packets
*         per Ingress Port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number, CPU port
* @param[in] mode                     - the assignment  of the packet Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortAssignModeSet
(
    IN GT_U8   devNum,
    IN GT_U32   portNum,
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
);

/**
* @internal prvTgfBrgSrcIdPortAssignModeGet function
* @endinternal
*
* @brief   Get Source ID Assignment mode for non-DSA tagged packets
*         per Ingress Port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - the assignment mode of the packet Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortAssignModeGet
(
    IN  GT_U8                            devNum,
    IN  GT_U8                            portNum,
    OUT CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  *modePtr
);

/**
* @internal prvTgfBrgVlanRemoveVlanTag1IfZeroModeSet function
* @endinternal
*
* @brief   Set Tag1 removal mode from the egress port Tag State if Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
* @param[in] mode                     - Vlan Tag1 Removal  when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanRemoveVlanTag1IfZeroModeSet
(
    IN  PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT   mode
);

/**
* @internal prvTgfBrgVlanRemoveVlanTag1IfZeroModeGet function
* @endinternal
*
* @brief   Get Tag1 removal mode from the egress port Tag State when Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @param[out] modePtr                  - (pointer to) Vlan Tag1 Removal mode when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanRemoveVlanTag1IfZeroModeGet
(
    OUT PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT   *modePtr
);

/**
* @internal prvTgfBrgFdbDeviceTableSet function
* @endinternal
*
* @brief   This function sets the device table of the PP.
*         the PP use this configuration in the FDB aging daemon .
*         once the aging daemon encounter an entry with non-exists devNum
*         associated with it , the daemon will DELETE the entry without sending
*         the CPU any notification.
*         a non-exists device determined by it's bit in the "device table"
*         So for proper work of PP the application must set the relevant bits of
*         all devices in the system prior to inserting FDB entries associated with
*         them
*
* @note   APPLICABLE DEVICES:      All Devices
* @param[in] devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS] - (array of) bmp of devices to set.
*                                      DxCh devices use only devTableBmp[0]
*                                      ExMxPm devices use devTableBmp[0..3]
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbDeviceTableSet
(
    IN  GT_U32  devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS]
);

/**
* @internal prvTgfBrgFdbDeviceTableGet function
* @endinternal
*
* @brief   This function gets the device table of the PP.
*         the PP use this configuration in the FDB aging daemon.
*         once the aging daemon encounter an entry with non-exists devNum
*         associated with it, the daemon will DELETE the entry without sending
*         the CPU any notification.
*         a non-exists device determined by it's bit in the "device table"
*
* @note   APPLICABLE DEVICES:      All devices
*
* @param[out] devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS] - pointer to (Array of) bmp (bitmap) of devices.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbDeviceTableGet
(
    OUT GT_U32  devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS]
);

/**
* @internal prvTgfBrgVlanKeepVlan1EnableSet function
* @endinternal
*
* @brief   Enable/Disable keeping VLAN1 in the packet, for packets received with VLAN1,
*         even-though the tag-state of this egress-port is configured in the
*         VLAN-table to "untagged" or "VLAN0".
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority
*                                      [0..7].
* @param[in] enable                   - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - illegal vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanKeepVlan1EnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U32       portNum,
    IN  GT_U8       up,
    IN  GT_BOOL     enable
);

/**
* @internal prvTgfBrgVlanKeepVlan1EnableGet function
* @endinternal
*
* @brief   Get status of keeping VLAN1 in the packet, for packets received with VLAN1.
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority
*                                      [0..7].
*
* @param[out] enablePtr                - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanKeepVlan1EnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_U8       up,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal prvTgfBrgVlanTr101Check function
* @endinternal
*
* @brief   check if the device supports the tr101
*
* @retval GT_TRUE                  - the device supports the tr101
* @retval GT_FALSE                 - the device not supports the tr101
*/
GT_BOOL prvTgfBrgVlanTr101Check(
    void
);

/**
* @internal prvTgfBrgGenPortIeeeReservedMcastLearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - learning is performed also for IEEE MC
*                                      trapped packets.
*                                      - GT_FALSE - No learning is performed for IEEE MC
*                                      trapped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenPortIeeeReservedMcastLearningEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgGenRateLimitGlobalCfgSet function
* @endinternal
*
* @brief   Configures global ingress rate limit parameters - rate limiting mode,
*         Four configurable global time window periods are assigned to ports as
*         a function of their operating speed:
*         10 Gbps, 1 Gbps, 100 Mbps, and 10 Mbps.
* @param[in] devNum                   - device number
* @param[in] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported windows for DxCh devices:
*       1000 Mbps: range - 256-16384 uSec  granularity - 256 uSec
*       100 Mbps: range - 256-131072 uSec  granularity - 256 uSec
*       10  Mbps: range - 256-1048570 uSec granularity - 256 uSec
*       10  Gbps: range - 25.6-104857 uSec granularity - 25.6 uSec
*       For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
GT_STATUS prvTgfBrgGenRateLimitGlobalCfgSet
(
    IN GT_U8                            devNum,
    IN PRV_TGF_BRG_GEN_RATE_LIMIT_STC   *brgRateLimitPtr
);

/**
* @internal prvTgfBrgGenRateLimitGlobalCfgGet function
* @endinternal
*
* @brief   Get global ingress rate limit parameters
*
* @param[in] devNum                   - device number
*
* @param[out] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
GT_STATUS prvTgfBrgGenRateLimitGlobalCfgGet
(
    IN  GT_U8                            devNum,
    OUT PRV_TGF_BRG_GEN_RATE_LIMIT_STC   *brgRateLimitPtr
);

/**
* @internal prvTgfBrgGenPortRateLimitSet function
* @endinternal
*
* @brief   Configures port ingress rate limit parameters
*         Each port maintains rate limits for unknown unicast packets,
*         known unicast packets, multicast packets and broadcast packets,
*         single configurable limit threshold value, and a single internal counter.
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] portGfgPtr               - pointer to rate limit configuration for a port
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_OUT_OF_RANGE          - on out of range rate limit values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenPortRateLimitSet
(
    IN  GT_U8                                   devNum,
    IN GT_U32                                    port,
    IN PRV_TGF_BRG_GEN_RATE_LIMIT_PORT_STC      *portGfgPtr
);


/**
* @internal prvTgfBrgFdbAuMsgBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of FDB Address Update (AU) messages,
*         the max number of elements defined by the caller
* @param[in] devNum                   - the device number from which AU are taken
* @param[in,out] numOfAuPtr               - (pointer to)max number of AU messages to get
* @param[in,out] numOfAuPtr               - (pointer to)actual number of AU messages that were received
*
* @param[out] auMessagesPtr            - array that holds received AU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note for multi-port groups device :
*       1. Unified FDB mode:
*       portGroup indication is required , for:
*       a. Aging calculations of Trunks
*       b. Aging calculations of regular entries when DA refresh is enabled.
*       In these 2 cases entries are considered as aged-out only if AA is
*       receives from all 4 portGroups.
*       2. In Unified-Linked FDB mode and Linked FDB mode:
*       portGroup indication is required for these reasons:
*       a. Aging calculations of Trunk and regular entries which have
*       been written to multiple portGroups.
*       b. New addresses (NA) source portGroup indication is required
*       so application can add new entry to the exact portGroup
*       which saw this station (especially on trunk entries).
*       c. Indication on which portGroup has removed this address when
*       AA of delete is fetched (relevant when same entry was written
*       to multiple portGroups).
*       d. Indication on which portGroup currently has this address
*       when QR (query reply) is being fetched (relevant when same
*       entry was written to multiple portGroups).
*
*/
GT_STATUS prvTgfBrgFdbAuMsgBlockGet
(
    IN     GT_U8                       devNum,
    INOUT  GT_U32                      *numOfAuPtr,
    OUT    CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagesPtr
);


/**
* @internal prvTgfBrgFdbNaMsgVid1EnableSet function
* @endinternal
*
* @brief   Enable/Disable Tag1 VLAN Id assignment in vid1 field of the NA AU
*         message CPSS_MAC_UPDATE_MSG_EXT_STC.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: vid1 field of the NA AU message is taken from
*                                      Tag1 VLAN.
*                                      GT_FALSE: vid1 field of the NA AU message is taken from
*                                      Tag0 VLAN.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbNaMsgVid1EnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgFdbTriggerAuQueueWa function
* @endinternal
*
* @brief   Trigger the Address Update (AU) queue workaround for case when learning
*         from CPU is stopped due to full Address Update (AU) fifo queue
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_STATE             - the secondary AUQ not reached the end of
*                                       queue , meaning no need to set the secondary
*                                       base address again.
*                                       this protect the SW from losing unread messages
*                                       in the secondary queue (otherwise the PP may
*                                       override them with new messages).
*                                       caller may use function
*                                       cpssDxChBrgFdbSecondaryAuMsgBlockGet(...)
*                                       to get messages from the secondary queue.
* @retval GT_NOT_SUPPORTED         - the device not need / not support the WA
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbTriggerAuQueueWa
(
    IN GT_U8    devNum
);

/**
* @internal prvTgfBrgFdbQueueRewindStatusGet function
* @endinternal
*
* @brief   function check if the specific AUQ was 'rewind' since the last time
*         this function was called for that AUQ
*         this information allow the application to know when to finish processing
*         of the WA relate to cpssDxChBrgFdbTriggerAuQueueWa(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] rewindPtr                - (pointer to) was the AUQ rewind since last call
*                                      GT_TRUE - AUQ was rewind since last call
*                                      GT_FALSE - AUQ wasn't rewind since last call
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbQueueRewindStatusGet
(
    IN  GT_U8                          devNum,
    OUT GT_BOOL                        *rewindPtr
);

/**
* @internal prvTgfBrgEportToPhysicalPortTargetMappingTableSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the E2PHY mapping table.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
GT_STATUS prvTgfBrgEportToPhysicalPortTargetMappingTableSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
);

/**
* @internal prvTgfBrgEportToPhysicalPortTargetMappingTableGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the E2PHY mapping table.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
GT_STATUS prvTgfBrgEportToPhysicalPortTargetMappingTableGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    OUT CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
);

/**
* @internal prvTgfBrgEportToPhysicalPortEgressPortInfoSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the HA Egress port table.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] egressInfoPtr            - (pointer to) physical information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgEportToPhysicalPortEgressPortInfoSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN PRV_TGF_BRG_EGRESS_PORT_INFO_STC     *egressInfoPtr
);

/**
* @internal prvTgfBrgEportToPhysicalPortEgressPortInfoGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the HA Egress port table.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] egressInfoPtr            - (pointer to) physical Information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgEportToPhysicalPortEgressPortInfoGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT PRV_TGF_BRG_EGRESS_PORT_INFO_STC    *egressInfoPtr
);

/**
* @internal prvTgfBrgVlanEgressPortTagStateModeSet function
* @endinternal
*
* @brief   Set the egress port tag state mode.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stateMode                - state mode: eVLAN or ePort
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressPortTagStateModeSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT        stateMode
);

/**
* @internal prvTgfBrgVlanEgressPortTagStateModeGet function
* @endinternal
*
* @brief   Get the egress port tag state mode.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] stateModePtr             - (pointer to) state mode: eVLAN or ePort
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressPortTagStateModeGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT           *stateModePtr
);

/**
* @internal prvTgfBrgVlanEgressPortTagStateSet function
* @endinternal
*
* @brief   Determines the egress port state.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tagCmd                   - Port tag state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressPortTagStateSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT                tagCmd
);

/**
* @internal prvTgfBrgVlanEgressPortTagStateGet function
* @endinternal
*
* @brief   Returns the egress port state.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tagCmdPtr                - (pointer to) Port tag state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressPortTagStateGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT                   *tagCmdPtr
);

/**
* @internal prvTgfBrgGenDropIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenDropIpMcEnable
(
    IN GT_BOOL  state
);

/**
* @internal prvTgfBrgVlanPortAccFrameTypeSet function
* @endinternal
*
* @brief   Set port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] frameType                -
*                                      CPSS_PORT_ACCEPT_FRAME_TAGGED_E    -
*                                      All Untagged/Priority Tagged packets received on
*                                      this port are discarded. Only Tagged accepted.
*                                      CPSS_PORT_ACCEPT_FRAME_ALL_E       -
*                                      Both Tagged and Untagged packets are accepted
*                                      on the port.
*                                      CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E  -
*                                      All Tagged packets received on this port are
*                                      discarded. Only Untagged/Priority Tagged accepted.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortAccFrameTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType
);

/**
* @internal prvTgfBrgVlanPortProtocolVidEnableSet function
* @endinternal
*
* @brief   Enable/Disable Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - Enable Port protocol VID assignment.
*                                      GT_FALSE - Disable Port protocol VID assignment
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortProtocolVidEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal prvTgfBrgVlanPortProtoClassQosEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortProtoClassQosEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);


/**
* @internal prvTgfBrgVlanProtocolEntrySet function
* @endinternal
*
* @brief   Write etherType and encapsulation of Protocol based classification
*         for specific device.
* @param[in] index                    - entry number
* @param[in] etherType                - Ether Type or DSAP/SSAP
* @param[in] encListPtr               - encapsulation.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanProtocolEntrySet
(
    IN GT_U32                           index,
    IN GT_U16                           etherType,
    IN CPSS_PROT_CLASS_ENCAP_STC        *encListPtr
);

/**
* @internal prvTgfBrgVlanPortProtoVlanQosSet function
* @endinternal
*
* @brief   Write Port Protocol VID and QOS Assignment values for specific device
*         and port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
* @param[in] vlanCfgPtr               - pointer to VID parameters
* @param[in] qosCfgPtr                - pointer to QoS parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortProtoVlanQosSet
(
    IN GT_U8                                   devNum,
    IN GT_PORT_NUM                             portNum,
    IN GT_U32                                  entryNum,
    IN PRV_TGF_BRG_VLAN_PROT_CLASS_CFG_STC    *vlanCfgPtr,
    IN PRV_TGF_BRG_QOS_PROT_CLASS_CFG_STC     *qosCfgPtr
);

/**
* @internal prvTgfBrgVlanPortProtoVlanQosInvalidate function
* @endinternal
*
* @brief   Invalidate protocol based QoS and VLAN assignment entry for
*         specific port
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortProtoVlanQosInvalidate
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   entryNum
);

/**
* @internal prvTgfBrgVlanProtocolEntryInvalidate function
* @endinternal
*
* @brief   Invalidates entry of Protocol based classification for specific device.
*
* @param[in] entryNum                 - entry number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanProtocolEntryInvalidate
(
    IN GT_U32                           entryNum
);

/**
* @internal prvTgfBrgFdbLsrEnableSet function
* @endinternal
*
* @brief   Enable/Disable the FDB Learning, Scanning and Refresh operations.
*         Halting of these operation allows the CPU un-interrupted access to
*         the FDB.
* @param[in] enable                   - GT_TRUE = Learning, Scanning and Refresh operations do
*                                      their regular work
*                                      GT_FALSE = Learning, Scanning and Refresh operations halt
*                                      to allow the CPU un-interrupted access to the FDB
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbLsrEnableSet
(
    IN GT_BOOL       enable
);

/**
* @internal prvTgfBrgFdbMacVlanLookupModeSet function
* @endinternal
*
* @brief   Sets the VLAN Lookup mode.
*
* @param[in] mode                     - lookup mode:
*                                      CPSS_IVL_E - Both MAC and Vlan are used for MAC table lookup
*                                      and learning.
*                                      CPSS_SVL_E - Only MAC is used for MAC table lookup and learning.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacVlanLookupModeSet
(
    IN CPSS_MAC_VL_ENT      mode
);

/**
* @internal prvTgfBrgFdbMacVlanLookupModeGet function
* @endinternal
*
* @brief   Get VLAN Lookup mode.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to lookup mode:
*                                      CPSS_IVL_E - Both MAC and Vlan are used for MAC table lookup
*                                      and learning.
*                                      CPSS_SVL_E - Only MAC is used for MAC table lookup and learning.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacVlanLookupModeGet
(
    IN  GT_U8            devNum,
    OUT CPSS_MAC_VL_ENT  *modePtr
);

/**
* @internal prvTgfBrgVlanEgressEtherTypeSet function
* @endinternal
*
* @brief   Sets egress VLAN Ether Type
*
* @param[in] etherType                - Mode of tagged vlan etherType.
*                                      May be either:CPSS_VLAN_ETHERTYPE0_E or
*                                      CPSS_VLAN_ETHERTYPE1_E By default only value 0 would
*                                      be used, unless nested VLANs are used.
*                                      See comment.
* @param[in] etherType                - the EtherType to be recognized tagged packets. See
*                                      comment.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressEtherTypeSet
(
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  etherType
);

/**
* @internal prvTgfBrgVlanEgressEtherTypeGet function
* @endinternal
*
* @brief   Gets egress VLAN Ether Type
*
* @param[in] devNum                   - device number
* @param[in] etherTypeMode            - Mode of tagged vlan etherType.
*                                      May be either:CPSS_VLAN_ETHERTYPE0_E or
*                                      CPSS_VLAN_ETHERTYPE1_E By default only value 0 would
*                                      be used, unless nested VLANs are used.
*                                      See comment.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressEtherTypeGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  *etherTypePtr
);

/**
* @internal prvTgfBrgNestVlanPortEgressEtherTypeSelectSet function
* @endinternal
*
* @brief   Sets selected VLAN EtherType for tagged packets transmitted
*         via this egress port.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, or CPU port
* @param[in] egressVlanSel            - selected port VLAN EtherType.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or ingressVlanSel values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgNestVlanPortEgressEtherTypeSelectSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                   portNum,
    IN  CPSS_ETHER_MODE_ENT     egressVlanSel
);

/**
* @internal prvTgfBrgNestVlanAccessPortEnableSet function
* @endinternal
*
* @brief   Configure given port as nested VLAN access port.
*         The VID of all the packets received on a nested VLAN access port is
*         discarded and they are assigned with the Port VID.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, or CPU port
* @param[in] enable                   -  GT_TRUE  - port is nested VLAN Access Port.
*                                      GT_FALSE - port is Core or Customer port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgNestVlanAccessPortEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U32       portNum,
    IN  GT_BOOL     enable
);


/**
* @internal prvTgfBrgFdbActionTransplantDataSet function
* @endinternal
*
* @brief   Prepares the entry for transplanting (old and new interface parameters).
*         VLAN and VLAN mask for transplanting is set by
*         cpssDxChBrgFdbActionActiveVlanSet. Execution of transplanting is done
*         by cpssDxChBrgFdbTrigActionStart.
* @param[in] oldInterfacePtr          - pointer to old Interface parameters
*                                      (it may be only port or trunk).
* @param[in] newInterfacePtr          - pointer to new Interface parameters
*                                      (it may be only port or trunk).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is no check done of current status of action's register
*
*/
GT_STATUS prvTgfBrgFdbActionTransplantDataSet
(
    IN CPSS_INTERFACE_INFO_STC  *oldInterfacePtr,
    IN CPSS_INTERFACE_INFO_STC  *newInterfacePtr
);

/**
* @internal prvTgfBrgFdbActionTransplantDataGet function
* @endinternal
*
* @brief   Get transplant data: old interface parameters and the new ones.
*
* @param[in] devNum                   - device number
*
* @param[out] oldInterfacePtr          - pointer to old Interface parameters.
*                                      (it may be only port or trunk).
* @param[out] newInterfacePtr          - pointer to new Interface parameters.
*                                      (it may be only port or trunk).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionTransplantDataGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_INTERFACE_INFO_STC  *oldInterfacePtr,
    OUT CPSS_INTERFACE_INFO_STC  *newInterfacePtr
);

/**
* @internal prvTgfBrgFdbActionActiveVlanSet function
* @endinternal
*
* @brief   Set action active vlan and vlan mask.
*         All actions will be taken on entries belonging to
*         a certain VLAN or a subset of VLANs.
* @param[in] vlanId                   - vlan Id
* @param[in] vlanMask                 - vlan mask filter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveVlanSet
(
    IN GT_U32   vlanId,
    IN GT_U32   vlanMask
);

/**
* @internal prvTgfBrgFdbActionActiveVlanGet function
* @endinternal
*
* @brief   Get action active vlan and vlan mask.
*         All actions are taken on entries belonging to a certain VLAN
*         or a subset of VLANs.
* @param[in] devNum                   - device number
*
* @param[out] vlanIdPtr                - pointer to vlan Id
* @param[out] vlanMaskPtr              - pointer to vlan mask filter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveVlanGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *vlanIdPtr,
    OUT GT_U32   *vlanMaskPtr
);

/**
* @internal prvTgfBrgFdbStaticTransEnableSet function
* @endinternal
*
* @brief   This routine determines whether the transplanting operate on static
*         entries.
*         When the PP will do the transplanting , it will/won't transplant
*         FDB static entries (entries that are not subject to aging).
*         When the PP will do the "port flush" (delete FDB entries associated
*         with to specific port), it will/won't Flush the FDB static entries
*         (entries that are not subject to aging).
* @param[in] enable                   - GT_TRUE transplanting is enabled on static entries,
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbStaticTransEnableSet
(
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgFdbStaticTransEnableGet function
* @endinternal
*
* @brief   Get whether the transplanting enabled to operate on static entries.
*         when the PP do the transplanting , it do/don't transplant
*         FDB static entries (entries that are not subject to aging).
*         When the PP will do the "port flush" (delete FDB entries associated
*         with to specific port), it do/don't Flush the FDB static entries
*         (entries that are not subject to aging).
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE transplanting is enabled on static entries,
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbStaticTransEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgFdbStaticDelEnableSet function
* @endinternal
*
* @brief   This routine determines whether flush delete operates on static entries.
*         When the PP will do the Flush , it will/won't Flush
*         FDB static entries (entries that are not subject to aging).
* @param[in] enable                   - GT_TRUE delete is enabled on static entries, GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbStaticDelEnableSet
(
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgFdbStaticDelEnableGet function
* @endinternal
*
* @brief   Get whether flush delete operates on static entries.
*         When the PP do the Flush , it do/don't Flush
*         FDB static entries (entries that are not subject to aging).
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE delete is enabled on static entries,
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbStaticDelEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgFdbQueueFullGet function
* @endinternal
*
* @brief   The function returns the state that the AUQ/FUQ - is full/not full
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] queueType                - queue type AU/FU queue
*                                      NOTE: when using shared queue for AU and FU messages, this
*                                      parameter is ignored (and the AUQ is queried)
*                                      meaning that this parameter always ignored for DxCh1
*                                      devices.
*
* @param[out] isFullPtr                - (pointer to) is the relevant queue full
*                                      GT_TRUE  - the queue is full
*                                      GT_FALSE - the queue is not full
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum , queueType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       'queue is full' when at least one of the port groups has queue full.
*
*/
GT_STATUS prvTgfBrgFdbQueueFullGet
(
    IN GT_U8    devNum,
    IN PRV_TGF_BRG_FDB_QUEUE_TYPE_ENT  queueType,
    OUT GT_BOOL *isFullPtr
);

/**
* @internal prvTgfBrgFdbNaStormPreventAllSet function
* @endinternal
*
* @brief   Enable/Disable New Address messages Storm Prevention.
*         Controlled learning relies on receiving NA messages for new source MAC
*         address. To prevent forwarding multiple NA messages to the CPU for the
*         same source MAC address, when NA Storm Prevention (SP) is enabled, the
*         device auto-learns this address with a special Storm Prevention flag set in
*         the entry. Subsequent packets from this source address do not generate
*         further NA messages to the CPU. Packets destined to this MAC address however
*         are treated as unknown packets. Upon receiving the single NA
*         message, the CPU can then overwrite the SP entry with a normal FDB
*         forwarding entry.
*         Only relevant in controlled address learning mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] enable                   - GT_TRUE -  NA Storm Prevention (SP),
*                                      GT_FALSE - disable NA Storm Prevention
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/portNum/auMsgType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       Enabling this mechanism in any of the ports, creates a situation where the
*       different portGroups populate different MACs at the same indexes,
*       causing FDBs to become unsynchronized, this leads to the situation that once
*       sending MAC update to different portGroups from CPU, it may succeed on some
*       of the portGroups, while failing on the others (due to SP entries already
*       occupying indexes)... So the only way to bypass these problems, assuming SP
*       is a must, is to add new MACs by exact index - this mode of operation
*       overwrites any SP entry currently there.
*       Application that enables the SP in any of the ports should not use the NA
*       sending mechanism (cpssDxChBrgFdbMacEntrySet / cpssDxChBrgFdbPortGroupMacEntrySet APIs)
*       to add new MAC addresses, especially to multiple portGroups -
*       as this may fail on some of the portGroups.
*       Removing or updating existing (non-SP) entries may still use the NA messages.
*
*/
GT_STATUS prvTgfBrgFdbNaStormPreventAllSet
(
    IN GT_BOOL          enable
);

/**
* @internal prvTgfBrgFdbNaStormPreventAllGet function
* @endinternal
*
* @brief   Get status of New Address messages Storm Prevention.
*         Controlled learning relies on receiving NA messages for new source MAC
*         address. To prevent forwarding multiple NA messages to the CPU for the
*         same source MAC address, when NA Storm Prevention (SP) is enabled, the
*         device auto-learns this address with a special Storm Prevention flag set in
*         the entry. Subsequent packets from this source address do not generate
*         further NA messages to the CPU. Packets destined to this MAC address however
*         are treated as unknown packets. Upon receiving the single NA
*         message, the CPU can then overwrite the SP entry with a normal FDB
*         forwarding entry.
*         Only relevant in controlled address learning mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
*
* @param[out] enablePtr                - pointer to the status of repeated NA CPU messages
*                                      GT_TRUE - NA Storm Prevention (SP) is enabled,
*                                      GT_FALSE - NA Storm Prevention is disabled
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/portNum/auMsgType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbNaStormPreventAllGet
(
    OUT GT_BOOL         *enablePtr
);

/**
* @internal prvTgfBrgFdbForceIgnoreNaStormPrevention function
* @endinternal
*
* @brief   DEBUG function to be called before running first enh-UT.
*         do we 'force' to ignore the SP settings by the tests ?
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] forceToIgnore            - do we 'force' to ignore the SP settings by the tests ?
*
* @retval GT_OK                    - on success,
*/
GT_STATUS prvTgfBrgFdbForceIgnoreNaStormPrevention
(
    IN GT_BOOL forceToIgnore
);

/**
* @internal prvTgfBrgFdbActionActiveInterfaceSet function
* @endinternal
*
* @brief   Set the Active trunkId/portNumber active trunk/port mask and
*         trunkID mask that control the FDB actions of :
*         Triggered aging with removal, Triggered aging without removal and
*         Triggered address deleting. A FDB entry will be treated by the
*         FDB action only if the following cases are true:
*         1. The interface is trunk and "active trunk mask" is set and
*         "associated trunkId" masked by the "active trunk/port mask"
*         equal to "Active trunkId".
*         2. The interface is port and "associated portNumber" masked by the
*         "active trunk/port mask" equal to "Active portNumber".
* @param[in] devNum                   - device number
* @param[in] actIsTrunk               - determines if the interface is port or trunk
* @param[in] actIsTrunkMask           - action active trunk mask.
* @param[in] actTrunkPort             - action active interface (port/trunk)
* @param[in] actTrunkPortMask         - action active port/trunk mask.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveInterfaceSet
(
    IN GT_U8   devNum,
    IN GT_U32  actIsTrunk,
    IN GT_U32  actIsTrunkMask,
    IN GT_U32  actTrunkPort,
    IN GT_U32  actTrunkPortMask
);

/**
* @internal prvTgfBrgFdbActionActiveInterfaceGet function
* @endinternal
*
* @brief   Get the Active trunkId/portNumber active trunk/port mask and
*         trunkID mask that control the FDB actions of: Triggered aging with
*         removal, Triggered aging without removal and Triggered address deleting.
* @param[in] devNum                   - device number
*
* @param[out] actIsTrunkPtr            - determines if the interface is port or trunk
* @param[out] actIsTrunkMaskPtr        - action active trunk mask.
* @param[out] actTrunkPortPtr          - action active interface (port/trunk)
* @param[out] actTrunkPortMaskPtr      - action active port/trunk mask.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveInterfaceGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *actIsTrunkPtr,
    OUT GT_U32  *actIsTrunkMaskPtr,
    OUT GT_U32  *actTrunkPortPtr,
    OUT GT_U32  *actTrunkPortMaskPtr
);

/**
* @internal prvTgfBrgFdbActionActiveDevSet function
* @endinternal
*
* @brief   Set the Active device number and active device number mask , that
*         control the FDB actions of : Trigger aging , transplant , flush ,
*         flush port.
*         A FDB entry will be treated by the FDB action only if the entry's
*         "associated devNum" masked by the "active device number mask" equal to
*         "Active device number"
* @param[in] devNum                   - device number
* @param[in] actDev                   - Action active device (APPLICABLE RANGES: 0..31)
* @param[in] actDevMask               - Action active mask (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*                                       in 'Dual deviceId system' , if the Pattern/mask
*                                       of the LSBit of the actDev,actDevMask are 'exact match = 1'
*                                       because all devices in this system with 'even numbers'
* @retval GT_OUT_OF_RANGE          - actDev > 31 or actDevMask > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is no check done of current status of action's register
*       For 'Dual deviceId system':
*       the LSBit on the actDev,acdDevMask are ignored and actually taken from
*       MSBit of actTrunkPort and actTrunkPortMask from prvTgfBrgFdbActionActiveInterfaceSet
*
*/
GT_STATUS prvTgfBrgFdbActionActiveDevSet
(
    IN  GT_U8   devNum,
    IN GT_U32   actDev,
    IN GT_U32   actDevMask
);

/**
* @internal prvTgfBrgFdbActionActiveDevGet function
* @endinternal
*
* @brief   Get the Active device number and active device number mask , that
*         control the FDB actions of : Trigger aging , transplant , flush ,
*         flush port.
*         A FDB entry will be treated by the FDB action only if the entry's
*         "associated devNum" masked by the "active device number mask" equal to
*         "Active device number"
* @param[in] devNum                   - device number
*
* @param[out] actDevPtr                - pointer to action active device (APPLICABLE RANGES: 0..31)
* @param[out] actDevMaskPtr            - pointer to action active mask (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - actDev > 31 or actDevMask > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveDevGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *actDevPtr,
    OUT GT_U32   *actDevMaskPtr
);

/**
* @internal prvTgfBrgSctIeeeReservedMcProtocolCpuCodeSet function
* @endinternal
*
* @brief   This function set the CPU code for specific IEEE reserved multicast
*         protocol at specific profile.
*         Each profile can be configured with 16 standard protocols
*         and 16 GARP protocols.
* @param[in] profileIndex             - index of profile. Can be either 0 or 1.
* @param[in] protocol                 - This param could be :
*                                      1. CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_STD_E -
*                                      standart protocols, that has MAC range
*                                      01-80-C2-00-00-00 to 01-80-C2-00-00-0F.
*                                      (note that the specific protocol expressed
*                                      by  last 4 bits are set by the next parameter)
*                                      2. CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E -
*                                      GARP protocols, that has a MAC range of:
*                                      01-80-C2-00-00-20 to 01-80-C2-00-00-2F.
*                                      (note that the specific protocol expressed
*                                      by  last 4 bits are set by the next parameter)
* @param[in] protocol                 -  specific  expressed by last 4 bits of
*                                      01-80-C2-00-00-0x or 01-80-C2-00-00-2x.
* @param[in] cpuCode                  -  CPU code for packets trapped or mirrored to CPU.
*                                      Can be either:
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramteres value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSctIeeeReservedMcProtocolCpuCodeSet
(
    IN GT_U32                                   profileIndex,
    IN PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_ENT     protocolType,
    IN GT_U32                                   protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
);

/**
* @internal prvTgfBrgSctIeeeReservedMcProtocolCmdSet function
* @endinternal
*
* @brief   This function set the command for specific IEEE reserved multicast
*         protocol at specific profile. Each profile can be configured with 16
*         standard protocols and 16 GARP protocols.
* @param[in] profileIndex             - index of profile. Can be either 0 or 1.
* @param[in] protocol                 - This param can be :
*                                      1. CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_STD_E -
*                                      standart protocols, that has MAC range
*                                      01-80-C2-00-00-00 to 01-80-C2-00-00-0F.
*                                      2. CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E -
*                                      GARP protocols, that has a MAC range of:
*                                      01-80-C2-00-00-20 to 01-80-C2-00-00-2F.
* @param[in] protocol                 -  specific  expressed by last 4 bits of
*                                      01-80-C2-00-00-0x or 01-80-C2-00-00-2x
* @param[in] cmd                      -  specify command to process packet with a MAC destination
*                                      Multicast address. In our case the cmd could take the
*                                      the following relevant values:
*                                      CPSS_PACKET_CMD_FORWARD_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramteres value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSctIeeeReservedMcProtocolCmdSet
(
    IN GT_U32                                   profileIndex,
    IN PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_ENT     protocolType,
    IN GT_U32                                   protocol,
    IN CPSS_PACKET_CMD_ENT                      cmd
);

/**
* @internal prvTgfBrgVlanIpmBridgingEnableSet function
* @endinternal
*
* @brief   IPv4/Ipv6 Multicast Bridging Enable
*
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] enable                   - GT_TRUE - IPv4 multicast packets are bridged
*                                      according to ipv4IpmBrgMode
*                                      GT_FALSE - IPv4 multicast packets are bridged
*                                      according to MAC DA
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by this API (per VLAN, per IP protocol).
*       'BrgMode' - set by prvTgfBrgVlanIpmBridgingModeSet (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
GT_STATUS prvTgfBrgVlanIpmBridgingEnableSet
(
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  GT_BOOL                     enable
);

/**
* @internal prvTgfBrgVlanIpmBridgingModeSet function
* @endinternal
*
* @brief   Sets the IPM bridging mode of Vlan. Relevant when IPM Bridging enabled.
*
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] ipmMode                  - IPM bridging mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by prvTgfBrgVlanIpmBridgingEnableSet (per VLAN, per IP protocol).
*       'BrgMode' - set by this API (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
GT_STATUS prvTgfBrgVlanIpmBridgingModeSet
(
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  CPSS_BRG_IPM_MODE_ENT       ipmMode
);

/**
* @internal prvTgfBrgGenDropNonIpMcEnable function
* @endinternal
*
* @brief   Sets an ability(Enable/Disable) to drop non-IPv4/6 Mc packets.
*         If it is enabled, the Drop Command assigned to all Ethernet
*         packets with MAC Multicast DA (MACDA[40] = 1), that are NOT in
*         the IPv4 Multicast IANA range (01-00-5E-00-00-00 to 01-00-5E-7F-FF-FF)
*         or in IPv6 Multicast IANA range (33-33-xx-xx-xx).
*         This does not include ARP Broadcast or Broadcast packets.
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenDropNonIpMcEnable
(
    IN GT_BOOL  state
);


/**
* @internal prvTgfBrgSecurityBreachEventDropModeSet function
* @endinternal
*
* @brief   Sets the Drop Mode for given Security Breach event.
*
* @param[in] eventType                - Security Breach event type.
* @param[in] dropMode                 - soft or hard drop mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum, eventType or dropMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The acceptable events are:
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E.
*
*/
GT_STATUS prvTgfBrgSecurityBreachEventDropModeSet
(
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    IN CPSS_DROP_MODE_TYPE_ENT              dropMode
);

/**
* @internal prvTgfBrgSecurityBreachEventDropModeGet function
* @endinternal
*
* @brief   Gets Drop Mode for given Security Breach event.
*
* @param[in] devNum                   - device number.
* @param[in] eventType                - security breach event type.
*
* @param[out] dropModePtr              - pointer to the drop mode: soft or hard.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum or eventType.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The acceptable events are:
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E.
*
*/
GT_STATUS prvTgfBrgSecurityBreachEventDropModeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropModePtr
);

/**
* @internal prvTgfBrgSecurityBreachPortVlanDropCntrModeSet function
* @endinternal
*
* @brief   Set Port/VLAN Security Breach Drop Counter to count security breach
*         dropped packets based on there ingress port or their assigned VID.
* @param[in] cntrCfgPtr               - (pointer to) security breach port/vlan counter
*                                      mode and it's configuration parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachPortVlanDropCntrModeSet
(
    IN CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC     *cntrCfgPtr
);

/**
* @internal prvTgfBrgSecurityBreachPortVlanDropCntrModeGet function
* @endinternal
*
* @brief   Get mode and it's configuration parameters for security breach
*         Port/VLAN drop counter.
* @param[in] dev                      - physical device number
*
* @param[out] cntrCfgPtr               - (pointer to) security breach port/vlan counter
*                                      mode and it's configuration parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachPortVlanDropCntrModeGet
(
    IN  GT_U8                                        dev,
    OUT CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC     *cntrCfgPtr
);

/**
* @internal prvTgfBrgEgressPortUnknownUcFilteringSet function
* @endinternal
*
* @brief   Enable/Disable egress Filtering for bridged Unknown Unicast packets
*         on the specified egress port.
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE: Unknown Unicast packets are filtered and are
*                                      not forwarded to this port.
*                                      GT_FALSE: Unknown Unicast packets are not filtered and may
*                                      be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnknownUcFilteringSet
(
    IN GT_U32            portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfBrgEgressPortUnknownUcFilteringGet function
* @endinternal
*
* @brief   Gets the current status of bridged Unknown Unicast packets filtering
*         on the specified egress port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - CPU port, virtual port number
*
* @param[out] enablePtr                - GT_TRUE: Unknown Unicast packets are filtered and are
*                                      not forwarded to this port.
*                                      GT_FALSE: Unknown Unicast packets are not filtered and may
*                                      be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnknownUcFilteringGet
(
    IN  GT_U8    devNum,
    IN  GT_U32    portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgEgressPortUnregMcFilteringSet function
* @endinternal
*
* @brief   Enable/Disable egress filtering for bridged unregistered multicast packets
*         on the specified egress port.
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE: Unregistered Multicast packets are filtered and
*                                      are not forwarded to this port.
*                                      GT_FALSE:Unregistered Multicast packets are not filtered
*                                      and may be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnregMcFilteringSet
(
    IN GT_U32            portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfBrgEgressPortUnregMcFilteringGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         for bridged Unregistered Multicast packets on the specified egress port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - CPU port, virtual port number
*
* @param[out] enablePtr                - points to:
*                                      GT_TRUE:  Unregistered Multicast packets are filtered and
*                                      are not forwarded to this port.
*                                      GT_FALSE: Unregistered Multicast packets are not filtered
*                                      and may be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnregMcFilteringGet
(
    IN  GT_U8    devNum,
    IN  GT_U32    portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgEgressPortUnregBcFilteringSet function
* @endinternal
*
* @brief   Enables or disables egress filtering of unregistered broadcast packets.
*         Unregistered broadcast packets are:
*         - packets with destination MAC ff-ff-ff-ff-ff-ff
*         - destination MAC address lookup not finds matching entry
*         - packets were not routed
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  -  filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are dropped
*                                      GT_FALSE - disable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are not dropped
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnregBcFilteringSet
(
    IN GT_U32            portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfBrgEgressPortUnregBcFilteringGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         of unregistered broadcast packets.
*         Unregistered broadcast packets are:
*         - packets with destination MAC ff-ff-ff-ff-ff-ff
*         - destination MAC address lookup not finds matching entry
*         - packets were not routed
* @param[in] devNum                   - device number
* @param[in] portNum                  - CPU port, virtual port number
*
* @param[out] enablePtr                - points to unregistered BC packets filtering
*                                      GT_TRUE  - enable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are dropped
*                                      GT_FALSE - disable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are not dropped
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnregBcFilteringGet
(
    IN  GT_U8    devNum,
    IN  GT_U32    portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgFdbAuqCbRegister function
* @endinternal
*
* @brief   register a CB (callback) function for a specific AUQ message type.
*         the registered function (cbFuncPtr) will be called for every AUQ message
*         that match the messageType.
* @param[in] messageType              - AUQ message type
* @param[in] bindOrUnbind             - bind/unbind   :
*                                      GT_TRUE - bind
*                                      GT_FALSE - unbind
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS   prvTgfBrgFdbAuqCbRegister
(
    IN CPSS_UPD_MSG_TYPE_ENT       messageType,
    IN GT_BOOL                     bindOrUnbind
);

/**
* @internal prvTgfBrgFdbAuqMessageAaCheck function
* @endinternal
*
* @brief   the function check AA (Aged out) messages.
*         the function allow to get AA only from the specific port groups bmp.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bmp of allowed port groups
*                                      OUTPUTS:
* @param[in] totalMessagesPtr         - (pointer to) total number of AA messages that received
*                                      from the port groups listed in portGroupsBmp
*                                      NOTE: can by NULL pointer --> ignored
* @param[in] totalErrorMessagesPtr    - (pointer to) total number of not expected messages that received.
*                                      NOTE: can by NULL pointer --> ignored
*
* @param[out] totalMessagesPtr         - (pointer to) total number of AA messages that received
*                                      from the port groups listed in portGroupsBmp
*                                      NOTE: can by NULL pointer --> ignored
* @param[out] totalErrorMessagesPtr    - (pointer to) total number of not expected messages that received.
*                                      NOTE: can by NULL pointer --> ignored
*                                      OUTPUTS:
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfBrgFdbAuqMessageAaCheck
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupsBmp,
    OUT GT_U32                        *totalMessagesPtr,
    OUT GT_U32                        *totalErrorMessagesPtr
);

/**
* @internal prvTgfBrgStpEntryRead function
* @endinternal
*
* @brief   Reads a raw STP group entry from HW.
*
* @param[in] dev                      - device number
*                                      stgId        - STG (Spanning Tree Group) index
*
* @param[out] stpEntryPtr              - (Pointer to) The raw stp entry to get.
*                                      memory allocated by the caller.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on stgId is larger than allowed
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - the ingress table and egress table values
*                                       do not match
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgStpEntryRead
(
    IN  GT_U8       dev,
    IN  GT_U16      stpId,
    OUT GT_U32      *stpEntryPtr
);

/**
* @internal prvTgfBrgStpEntryWrite function
* @endinternal
*
* @brief   Writes a raw STP entry to HW.
*
* @param[in] dev                      - device number
* @param[in] stpId                    - STG (Spanning Tree Group) index
* @param[in] stpEntryPtr              - (Pointer to) The raw stp entry to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or stpId
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgStpEntryWrite
(
    IN  GT_U8       dev,
    IN  GT_U16      stpId,
    IN  GT_U32      *stpEntryPtr
);

/**
* @internal prvTgfBrgVlanIngressEtherTypeSet function
* @endinternal
*
* @brief   Sets Ingress VLAN Ether Type and Range
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIngressEtherTypeSet
(
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  etherType,
    IN  GT_U16                  maxVidRange
);

/**
* @internal prvTgfBrgVlanIngressEtherTypeGet function
* @endinternal
*
* @brief   Gets Ingress VLAN Ether Type and Range
*
* @param[in] etherTypeMode            - mode of tagged vlan etherType. May be either
*                                      CPSS_VLAN_ETHERTYPE0_E or CPSS_VLAN_ETHERTYPE1_E.
*                                      See comment.
*
* @param[out] etherTypePtr             - the EtherType to be recognized tagged packets. See
*                                      comment.
* @param[out] maxVidRangePtr           - (pointer to)the maximum VID that may be assigned to
*                                      a packet received on a port with <Ingress VLANSel> set
*                                      to 1.
*                                      If the VID assigned to the packet is greater than this
*                                      value, a security breach occurs and the packet is
*                                      assigned with a hard drop or a soft drop command,
*                                      according to the setting of <VLANRange DropMode>
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the vid of the incoming packet is greater or equal than the vlan
*       range value, the packet is filtered.
*       Configuration of vid and range for CPSS_VLAN_ETHERTYPE1_E should be done
*       only when nested vlan is used. User can alter the etherType field of
*       the encapsulating vlan by setting a number other then 0x8100 in the
*       parameter <ethertype>.
*
*/
GT_STATUS prvTgfBrgVlanIngressEtherTypeGet
(
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  *etherTypePtr,
    IN  GT_U16                  *maxVidRangePtr
);

/**
* @internal prvTgfBrgCntMacDaSaSet function
* @endinternal
*
* @brief   Sets a specific MAC DA and SA to be monitored by Host
*         and Matrix counter groups on specified device.
* @param[in] macSaPtr                 - pointer to the source MAC address.
* @param[in] macDaPtr                 - pointer to the destination MAC address.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgCntMacDaSaSet
(
    IN  GT_ETHERADDR    *macSaPtr,
    IN  GT_ETHERADDR    *macDaPtr
);

/**
* @internal prvTgfBrgCntMacDaSaGet function
* @endinternal
*
* @brief   Gets a MAC DA and SA are monitored by Host
*         and Matrix counter groups on specified device.
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgCntMacDaSaGet
(
    IN   GT_U8           devNum,
    OUT  GT_ETHERADDR    *macSaPtr,
    OUT  GT_ETHERADDR    *macDaPtr
);

/**
* @internal prvTgfBrgCntHostGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Host group counters value of specified device.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] hostGroupCntrPtr         - pointer to structure with current counters value.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Host counters are clear-on-read.
*
*/
GT_STATUS prvTgfBrgCntHostGroupCntrsGet
(
    IN   GT_U8                        devNum,
    OUT  PRV_TGF_BRG_HOST_CNTR_STC   *hostGroupCntrPtr
);

/**
* @internal prvTgfBrgCntMatrixGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Matrix counter value of specified device.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] matrixCntSaDaPktsPtr     - number of packets (good only) with a MAC SA/DA
*                                      matching of the CPU-configured MAC SA/DA.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Matrix counter is clear-on-read.
*
*/
GT_STATUS prvTgfBrgCntMatrixGroupCntrsGet
(
    IN   GT_U8      devNum,
    OUT  GT_U32     *matrixCntSaDaPktsPtr
);

/**
* @internal prvTgfBrgCntBridgeIngressCntrsGet function
* @endinternal
*
* @brief   Gets a Bridge ingress Port/VLAN/Device counters from
*         specified counter set.
* @param[in] devNum                   - physical device number.
* @param[in] cntrSetId                - counter set number [0:1] (there are 2 Bridge Ingress
*                                      Counter Sets).
*
* @param[out] ingressCntrPtr           - pointer to the structure of Bridge Ingress Counters
*                                      current values.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrSetId.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Ingress counters are clear-on-read.
*
*/
GT_STATUS prvTgfBrgCntBridgeIngressCntrsGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_BRG_CNTR_SET_ID_ENT         cntrSetId,
    OUT CPSS_BRIDGE_INGRESS_CNTR_STC        *ingressCntrPtr
);

/**
* @internal prvTgfBrgCntBridgeIngressCntrModeSet function
* @endinternal
*
* @brief   Configures a specified Set of Bridge Ingress
*         counters to work in requested mode.
* @param[in] cntrSetId                - Counter Set ID
* @param[in] setMode                  - count mode of specified Set of Bridge Ingress Counters.
* @param[in] port                     -  number monitored by Set of Counters.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_1_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
* @param[in] vlan                     - VLAN ID monitored by Counters Set.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_2_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum, setMode or cntrSetId.
* @retval GT_OUT_OF_RANGE          - on port number or vlan out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgCntBridgeIngressCntrModeSet
(
    IN  PRV_TGF_BRG_CNTR_SET_ID_ENT         cntrSetId,
    IN  CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode,
    IN  GT_PORT_NUM                         port,
    IN  GT_U16                              vlan
);

/**
* @internal prvTgfBrgCntBridgeIngressCntrModeGet function
* @endinternal
*
* @brief   Gets the mode (port number and VLAN Id as well) of specified
*         Bridge Ingress counters Set.
* @param[in] devNum                   - physical device number
* @param[in] cntrSetId                - Counter Set ID
*
* @param[out] setModePtr               - pointer to the count mode of specified Set of Bridge
*                                      Ingress Counters.
* @param[out] portPtr                  - pointer to the port number monitored by Set of Counters.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_1_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*                                      This parameter can be NULL;
* @param[out] vlanPtr                  - pointer to the VLAN ID monitored by Counters Set.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_2_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*                                      This parameter can be NULL;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or counter set number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgCntBridgeIngressCntrModeGet
(
    IN   GT_U8                               devNum,
    IN   PRV_TGF_BRG_CNTR_SET_ID_ENT         cntrSetId,
    OUT  CPSS_BRIDGE_INGR_CNTR_MODES_ENT     *setModePtr,
    OUT  GT_PORT_NUM                         *portPtr,
    OUT  GT_U16                              *vlanPtr
);

/**
* @internal prvTgfBrgVlanIngressMirrorEnable function
* @endinternal
*
* @brief   Enable/Disable ingress mirroring of packets to the Ingress Analyzer port
*         for packets assigned the given VLAN-ID
* @param[in] devNum                   - device id
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled
* @param[in] enable                   - don't ingress mirrored traffic assigned to this VLAN to the
*                                      analyzer port.
* @param[in] index                    - mirror to analyzer  (applicable to: Lion3, Bobcat2)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIngressMirrorEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
);

/**
* @internal prvTgfBrgVlanMirrorToTxAnalyzerEnableSet function
* @endinternal
*
* @brief   Enable/Disable egress mirroring on this Vlan.
*         When enabled, egress traffic on this Vlan is mirrored
*         to the Tx Analyzer port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMirrorToTxAnalyzerEnableSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanStgIdBind function
* @endinternal
*
* @brief   Bind VLAN to STP Id.
*
* @param[in] vlanId                   - vlan Id
* @param[in] stgId                    - STG Id
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanStgIdBind
(
    IN GT_U16   vlanId,
    IN GT_U16   stgId
);

/**
* @internal prvTgfBrgStpPortStateSet function
* @endinternal
*
* @brief   Sets STP state of port belonging within an STP group.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stgId                    - STG (Spanning Tree Group) index
* @param[in] state                    - STP port state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or port or stgId or state
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgStpPortStateSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               stgId,
    IN CPSS_STP_STATE_ENT   state
);

/**
* @internal prvTgfBrgStpPortStateGet function
* @endinternal
*
* @brief   Get STP state of port belonging within an STP group.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stgId                    - STG (Spanning Tree Group) index  (0..4095)
*
* @param[out] statePtr                 - (pointer ot) STP port state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or port or stgId
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_BAD_VALUE             - on bad stp state value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - the ingress table and egress table values
*                                       do not match
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgStpPortStateGet
(
    IN  GT_U8               devNum,
    IN  GT_U32               portNum,
    IN  GT_U32              stgId,
    OUT CPSS_STP_STATE_ENT  *statePtr
);

/**
* @internal prvTgfBrgGenDropInvalidSaEnableSet function
* @endinternal
*
* @brief   Sets an ability (Enable/Disable) to drop Ethernet packets
*         with Multicast MAC SA (MACSA[40] = 1).
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - packets with the Mc MAC SA are dropped.
*                                      These packets are assigned with a soft
*                                      drop or a hard drop Command, according to
*                                      the Drop Mode of packets with invalid SA.
*                                      GT_FALSE - packets with the Mc MAC SA aren't dropped.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenDropInvalidSaEnableSet
(
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet function
* @endinternal
*
* @brief   Enables\Disables Security Breach for Moved Static addresses.
*         When the FDB entry <Static> bit is set to 1, it is not subject to Aging.
*         In addition, there may be cases in which the Port#/Trunk# in this entry
*         doesn't match the Port#/Trunk# from which this packet was received.
*         In this case the moved static address could (or couldn't) to be
*         considered as Security Breach at the specified port.
* @param[in] port                     -  number.
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - moved static address is considered as
*                                      Security Breach at the specified port
*                                      and are dropped according to the
*                                      Moved Static Drop Mode (hard or soft).
*                                      GT_FALSE - moved static address isn't considered
*                                      as Security Breach at the specified port
*                                      and are forwarded regularly.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet
(
    IN GT_U32    port,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfBrgSecurityBreachPortMovedStaticAddrEnableGet function
* @endinternal
*
* @brief   Get if static addresses that are moved, are regarded as Security Breach
*         or not (this is only in case that the fdb entry is static or the
*         interface does't match the interface from which this packet was received).
*
* @param[out] enablePtr                - GT_TRUE - moved static addresses are regarded as
*                                      security breach event and are dropped.
*                                      GT_FALSE - moved static addresses are not regarded
*                                      as security breach event and are forwarded.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachPortMovedStaticAddrEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U32    port,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgPortUnkSrcMacDropSet function
* @endinternal
*
* @brief   Enable dropping unknown source mac address.
*
* @param[in] portNum                  port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on unknown command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgPortUnkSrcMacDropSet
(
    IN GT_PORT_NUM              portNum
);

/**
* @internal prvTgfBrgPortUnkSrcMacDropInvalidate function
* @endinternal
*
* @brief   Invalidate config after prvTgfBrgPortUnkSrcMacDropSet.
*
* @param[in] portNum                  port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on unknown command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgPortUnkSrcMacDropInvalidate
(
    IN GT_PORT_NUM              portNum
);

/**
* @internal prvTgfBrgFdbSecondaryAuMsgBlockGet function
* @endinternal
*
* @brief   The function returns a block (array) of FDB Address Update (AU) messages,
*         the max number of elements defined by the caller --> from the secondary AUQ.
*         The function cpssDxChHwPpPhase2Init configures queue for secondary AU
*         messages. If common FU queue is used then function returns all AU
*         messages in the queue including FU ones.
*         If separate AU queue is used then function returns only AU messages.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which AU are taken
* @param[in,out] numOfAuPtr               - (pointer to)max number of AU messages to get
* @param[in,out] numOfAuPtr               - (pointer to)actual number of AU messages that were received
*
* @param[out] auMessagesPtr            - array that holds received AU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbSecondaryAuMsgBlockGet
(
    IN      GT_U8                                    devNum,
    INOUT   GT_U32                                   *numOfAuPtr,
    OUT     CPSS_MAC_UPDATE_MSG_EXT_STC              *auMessagesPtr
);

/**
* @internal prvTgfBrgFdbAAandTAToCpuSet function
* @endinternal
*
* @brief   Enable/Disable the PP to/from sending an AA and TA address
*         update messages to the CPU.
* @param[in] enable                   -  or disable the message
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or auMsgType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       Whenever applicable it's highly recommended disabling these messages,
*       as enabling this would stress the AUQ with excessive messages
*
*/
GT_STATUS prvTgfBrgFdbAAandTAToCpuSet
(
    IN GT_BOOL                      enable
);

/**
* @internal prvTgfBrgFdbMessagesQueueManagerInfoGet function
* @endinternal
*
* @brief   debug tool - print info that the CPSS hold about the state of the :
*         primary AUQ,FUQ,secondary AU queue
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note relate to CQ#109715
*
*/
GT_STATUS prvTgfBrgFdbMessagesQueueManagerInfoGet
(
    IN  GT_U8   devNum
);

/**
* @internal prvTgfFdbAuWaCheck function
* @endinternal
*
* @brief   Check if the device supports the AU queue WA
*
* @param[in] devNum                   - device number
* @param[in] useDoubleAuq             - double AUQ is in use
*
* @retval GT_TRUE                  - the device supports the AU queue WA
* @retval GT_FALSE                 - the device not supports the AU queue WA
*/
GT_BOOL prvTgfFdbAuWaCheck
(
    IN  GT_U8   devNum,
    IN  GT_BOOL useDoubleAuq
);

/**
* @internal prvTgfBrgLoopbackCt10And40Check function
* @endinternal
*
* @brief   Check if the device supports the Source ID, cascading, Vlan Table Write
*         Workaround tests.
*
* @retval GT_TRUE                  - the device supports the Source ID, cascading,
*                                       Vlan Table Write Workaround tests.
* @retval GT_FALSE                 - the device not supports the Source ID, cascading,
*                                       Vlan Table Write Workaround tests.
*/
GT_BOOL prvTgfBrgLoopbackCt10And40Check(
    void
);


/**
* @internal prvTgfBrgFdbSecureAutoLearnSet function
* @endinternal
*
* @brief   Configure secure auto learning mode.
*         If Secure Automatic Learning is enabled, and a packet enabled for
*         automatic learning, then:
*         - If the source MAC address is found in the FDB, and the associated
*         location of the address has changed, then the FDB entry is updated
*         with the new location of the address.
*         - If the FDB entry is modified with the new location of the address,
*         a New Address Update message is sent to the CPU,
*         if enabled to do so on the port and the packet VLAN.
*         - If the source MAC address is NOT found in the FDB, then
*         Secure Automatic Learning Unknown Source command (Trap to CPU,
*         Soft Drop or Hard Drop) is assigned to the packet according to mode
*         input parameter and packet treated as security breach event.
* @param[in] mode                     - Enable/Disable secure automatic learning.
*                                      The command assigned to packet with new SA not found in FDB
*                                      when secure automatic learning enable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbSecureAutoLearnSet
(
    IN  CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT   mode
);


/**
* @internal prvTgfBrgFdbSecureAutoLearnGet function
* @endinternal
*
* @brief   Get secure auto learning mode.
*         If Secure Automatic Learning is enabled, and a packet enabled for
*         automatic learning, then:
*         - If the source MAC address is found in the FDB, and the associated
*         location of the address has changed, then the FDB entry is updated
*         with the new location of the address.
*         - If the FDB entry is modified with the new location of the address,
*         a New Address Update message is sent to the CPU,
*         if enabled to do so on the port and the packet VLAN.
*         - If the source MAC address is NOT found in the FDB, then
*         Secure Automatic Learning Unknown Source command (Trap to CPU,
*         Soft Drop or Hard Drop) is assigned to the packet according to mode
*         input parameter and packet treated as security breach event.
*
* @param[out] modePtr                  - Enabled/Disabled secure automatic learning.
*                                      The command assigned to packet with new SA not found in FDB
*                                      when secure automatic learning enabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbSecureAutoLearnGet
(
    OUT CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT  *modePtr
);


/**
* @internal prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet function
* @endinternal
*
* @brief   Function prevents learning of moved SA enries. When enabled moved SA
*         address are considered as security breach, and are not learned. Relevant
*         when SecureAutoLearning is enabled (cpssExMxPmBrgFdbSecureAutoLearnSet).
* @param[in] enable                   - GT_TRUE - moved SA addresses are considered as security breach
*                                      GT_FALSE - moved SA addresses learned as usual
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: Puma3 device applicable starting from revision B0
*
*/
GT_STATUS prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet
(
    IN  GT_BOOL enable
);


/**
* @internal prvTgfBrgFdbSecureAutoLearnMovedSaEnableGet function
* @endinternal
*
* @brief   The function return learning status of moved SA enries.
*
* @param[out] enablePtr                - (pointer to) status of moved SA addresses
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: Puma3 device applicable starting from revision B0
*
*/
GT_STATUS prvTgfBrgFdbSecureAutoLearnMovedSaEnableGet
(
    OUT GT_BOOL *enablePtr
);


/**
* @internal prvTgfBrgFdbPortAutoLearnEnableSet function
* @endinternal
*
* @brief   Enable/Disable auto-learning per port.
*
* @param[in] devNum                   - device number
*                                      port    - port number
* @param[in] enable                   - GT_TRUE  -  auto-learning
*                                      GT_FALSE - disable auto-learning
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbPortAutoLearnEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PORT_NUM   portNum,
    IN  GT_BOOL enable
);

/**
* @internal prvTgfBrgFdbPortAutoLearnEnableGet function
* @endinternal
*
* @brief   Get auto-learning status per port.
*         NOT APPLICABLE DEVICES:
*         None.
*
* @note   APPLICABLE DEVICES:      None.
* @param[in] devNum                   - device number
*                                      port    - port number
*
* @param[out] enablePtr                - pointer to auto-learning status
*                                      GT_TRUE  - auto-learning enabled
*                                      GT_FALSE - auto-learning disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbPortAutoLearnEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PORT_NUM   portNum,
    IN  GT_BOOL *enablePtr
);


/**
* @internal prvTgfBrgFdbNaToCpuPerPortSet function
* @endinternal
*
* @brief   Enable/disable forwarding a new mac address message to CPU --per port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] enable                   - If GT_TRUE, forward NA message to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function also called from cascade management
*
*/
GT_STATUS prvTgfBrgFdbNaToCpuPerPortSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal prvTgfBrgFdbNaToCpuPerPortGet function
* @endinternal
*
* @brief   Get Enable/disable forwarding a new mac address message to CPU --
*         per port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] enablePtr                - If GT_TRUE, NA message is forwarded to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function also called from cascade management
*
*/
GT_STATUS prvTgfBrgFdbNaToCpuPerPortGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);


/**
* @internal prvTgfBrgVlanIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - TPID profile. (APPLICABLE RANGES: 0..7)
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] tpidBmp                  - bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For the ingress direction multiple TPID may be used to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*       2. bind 'port' to 'profile' using function cpssDxChBrgVlanPortIngressTpidProfileSet
*
*/
GT_STATUS prvTgfBrgVlanIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
);

/**
* @internal prvTgfBrgVlanIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gts bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - TPID profile. (APPLICABLE RANGES: 0..7)
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
);

/**
* @internal prvTgfBrgVlanPortIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; .
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] isDefaultProfile         - indication that the profile is the default profile or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet
*                                      GT_TRUE  - used as default profile
*                                      GT_FALSE - used for re-parse after TTI lookup.
* @param[in] profile                  - TPID profile. (APPLICABLE RANGE: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - profile > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For setting TPID bmp per profile use function cpssDxChBrgVlanIngressTpidProfileSet
*
*/
GT_STATUS prvTgfBrgVlanPortIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    IN  GT_U32               profile
);

/**
* @internal prvTgfBrgVlanPortIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; .
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] isDefaultProfile         - indication that the profile is the default profile or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet
*                                      GT_TRUE  - used as default profile
*                                      GT_FALSE - used for re-parse after TTI lookup.
*
* @param[out] profilePtr               - (pointer to)TPID profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    OUT GT_U32               *profilePtr
);

/**
* @internal prvTgfBrgVlanPortVid1Set function
* @endinternal
*
* @brief   For ingress direction : Set port's default Vid1.
*         For egress direction : Set port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
* @param[in] vid1                     - VID1 value, range 0..4095
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortVid1Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN CPSS_DIRECTION_ENT          direction,
    IN  GT_U16           vid1
);

/**
* @internal prvTgfBrgVlanPortVid1Get function
* @endinternal
*
* @brief   For ingress direction : Get port's default Vid1.
*         For egress direction : Get port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
*
* @param[out] vid1Ptr                  - (pointer to)VID1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortVid1Get
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN CPSS_DIRECTION_ENT          direction,
    OUT  GT_U16           *vid1Ptr
);

/**
* @internal prvTgfBrgVlanMembersTableIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMembersTableIndexingModeSet
(
    IN  CPSS_DIRECTION_ENT                  direction,
    IN  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT mode
);

/**
* @internal prvTgfBrgVlanMembersTableIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - ingress/egress direction
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMembersTableIndexingModeGet
(
    IN  CPSS_DIRECTION_ENT                  direction,
    OUT PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT *modePtr
);

/**
* @internal prvTgfBrgVlanStgIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanStgIndexingModeSet
(
    IN  CPSS_DIRECTION_ENT                  direction,
    IN  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT mode
);

/**
* @internal prvTgfBrgVlanStgIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - ingress/egress direction
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanStgIndexingModeGet
(
    IN  CPSS_DIRECTION_ENT                  direction,
    OUT PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT *modePtr
);

/**
* @internal prvTgfBrgVlanTagStateIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTagStateIndexingModeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT mode
);

/**
* @internal prvTgfBrgVlanTagStateIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTagStateIndexingModeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT *modePtr
);

/**
* @internal prvTgfBrgGenBypassModeSet function
* @endinternal
*
* @brief   Set Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] mode                     - bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
GT_STATUS prvTgfBrgGenBypassModeSet
(
    IN  PRV_TGF_BRG_GEN_BYPASS_MODE_ENT mode
);

/**
* @internal prvTgfBrgGenBypassModeGet function
* @endinternal
*
* @brief   Get Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
GT_STATUS prvTgfBrgGenBypassModeGet
(
    IN  GT_U8                           devNum,
    OUT PRV_TGF_BRG_GEN_BYPASS_MODE_ENT *modePtr
);

/**
* @internal prvTgfBrgSecurBreachEventPacketCommandSet function
* @endinternal
*
* @brief   Set for given security breach event it's packet command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this function similar to cpssDxChBrgSecurBreachEventDropModeSet , but
*       allow additional packet commands.
*
*/
GT_STATUS prvTgfBrgSecurBreachEventPacketCommandSet
(
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    IN CPSS_PACKET_CMD_ENT                  command
);

/**
* @internal prvTgfBrgGenCfiRelayEnableSet function
* @endinternal
*
* @brief   Enable / Disable CFI bit relay.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  Tagged packets transmitted from the device have
*                                      have their CFI bit set according to received
*                                      packet CFI bit if the received packet was tagged
*                                      or set to 0 if the received packet is untagged.
*                                      - GT_FALSE - Tagged packets transmitted from the device
*                                      have their CFI bit set to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenCfiRelayEnableSet
(
    IN GT_U8                          devNum,
    IN GT_BOOL                        enable
);


/**
* @internal prvTgfBrgGenMtuConfigSet function
* @endinternal
*
* @brief   Enable/disable Target ePort MTU check feature.
*         and configure the following parameters:
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] mtuCheckEnable           - target ePort MTU check feature enabled/disabled
*                                      GT_TRUE  MTU check enabled, GT_FALSE - MTU check disabled
* @param[in] byteCountMode            - byte-count mode for MTU
* @param[in] exceptionCommand         command assigned to frames that exceed the
*                                      default ePort MTU size
* @param[in] exceptionCpuCode         CPU/drop code assigned to frame which fails MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, byteCountMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenMtuConfigSet
(
    IN GT_BOOL                                      mtuCheckEnable,
    IN PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT      byteCountMode,
    IN CPSS_PACKET_CMD_ENT                          exceptionCommand,
    IN CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode
);

/**
* @internal prvTgfBrgGenMtuConfigGet function
* @endinternal
*
* @brief   Get the target ePort MTU feature parameters:
*         - the enable/disable target ePort MTU check feature.
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] mtuCheckEnablePtr        - (pointer to) enable/disable target ePort MTU feature.
* @param[out] byteCountModePtr         - (pointer to) MTU byte-count mode
* @param[out] exceptionCommandPtr      - (pointer to) exception cmd for frames which
*                                      fails MTU check
* @param[out] exceptionCpuCodePtr      - (pointer to) CPU/drop code assigned to frame
*                                      which fails the MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong HW value
*/
GT_STATUS prvTgfBrgGenMtuConfigGet
(
    OUT GT_BOOL                                      *mtuCheckEnablePtr,
    OUT PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT      *byteCountModePtr,
    OUT CPSS_PACKET_CMD_ENT                          *exceptionCommandPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT                     *exceptionCpuCodePtr
);

/**
* @internal prvTgfBrgGenMtuPortProfileIdxSet function
* @endinternal
*
* @brief   Set MTU profile index for the given target ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mtuProfileIdx            - index to global MTU entry
*                                      (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_OUT_OF_RANGE      mtuProfileIdx out of range
*/
GT_STATUS prvTgfBrgGenMtuPortProfileIdxSet
(
    IN GT_U8         devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_U32        mtuProfileIdx
);


/**
* @internal prvTgfBrgGenMtuPortProfileIdxGet function
* @endinternal
*
* @brief   Get MTU profile index for the given ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] mtuProfileIdxPtr         - (pointer to)index to the global MTU entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenMtuPortProfileIdxGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    portNum,
    OUT GT_U32         *mtuProfileIdxPtr
);

/**
* @internal prvTgfBrgGenMtuExceedCntrGet function
* @endinternal
*
* @brief   Get target ePort MTU exceeded counter.
*         The number of packets that triggered the Target ePort MTU
*         exceeded exception check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] valuePtr                 - (pointer to) the value of target ePort MTU exceeded counter.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenMtuExceedCntrGet
(
    OUT GT_U32       *valuePtr
);

/**
* @internal prvTgfBrgGenMtuProfileSet function
* @endinternal
*
* @brief   Set an MTU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
* @param[in] mtuSize                  - max transmit packet size in bytes
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range mtuSize
*/
GT_STATUS prvTgfBrgGenMtuProfileSet
(
    IN GT_U32   profile,
    IN GT_U32   mtuSize
);

/**
* @internal prvTgfBrgGenMtuProfileGet function
* @endinternal
*
* @brief   Get an MTU size for ePort profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
*
* @param[out] mtuSizePtr               - (pointer to) max transmit packet size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgGenMtuProfileGet
(
    IN  GT_U32  profile,
    OUT GT_U32  *mtuSizePtr
);

/**
* @internal prvTgfBrgVlanIpCntlToCpuSet function
* @endinternal
*
* @brief   Enable/disable IP control traffic trapping/mirroring to CPU.
*         When set, this enables the following control traffic to be eligible
*         for mirroring/trapping to the CPU:
*         - ARP
*         - IPv6 Neighbor Solicitation
*         - IPv4/v6 Control Protocols Running Over Link-Local Multicast
*         - RIPv1 MAC Broadcast
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIpCntlToCpuSet
(
    IN GT_U16                               vlanId,
    IN PRV_TGF_BRG_IP_CTRL_TYPE_ENT         ipCntrlType
);

/**
* @internal prvTgfBrgGenArpBcastToCpuCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU ARP Broadcast packets for all VLANs,
*         with with IPv4/IPv6 Control Traffic To CPU Enable set by function
*         prvTgfBrgVlanIpCntlToCpuSet
*         INPUTS:
*         devNum - device number
*         cmdMode  command interface mode:
*         PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E  will set ARP broadcast command for all ports,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenArpBcastToCpuCmdSet
(
    IN PRV_TGF_BRG_ARP_BCAST_CMD_MODE_ENT cmdMode,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal prvTgfBrgGenArpBcastToCpuCmdGet function
* @endinternal
*
* @brief   Get status of CPU ARP Broadcast packets for all VLANs
*         INPUTS:
*         devNum - device number
*         cmdMode  command interface mode:
*         PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E  will set ARP broadcast command for all ports,
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfBrgGenArpBcastToCpuCmdGet
(
    IN PRV_TGF_BRG_ARP_BCAST_CMD_MODE_ENT cmdMode,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
);

/**
* @internal prvTgfBrgVlanPortNumOfTagWordsToPopSet function
* @endinternal
*
* @brief   Set the number of bytes to pop for traffic that ingress from the port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] numBytesToPop            - enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when parameter numberOfTagWords is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortNumOfTagWordsToPopSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_ENT           numBytesToPop
);

/**
* @internal prvTgfBrgGenCiscoL2ProtCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU Proprietary Layer 2 Control MC packets.
*
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenCiscoL2ProtCmdSet
(
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal prvTgfBrgGenCiscoL2ProtCmdGet function
* @endinternal
*
* @brief   Get current command referring Proprietary Layer 2 Control MC packets.
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfBrgGenCiscoL2ProtCmdGet
(
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
);


/**
* @internal prvTgfBrgGenIcmpv6MsgTypeSet function
* @endinternal
*
* @brief   set trapping or mirroring to CPU command for IPv6 ICMP packets,
*         accordong to their ICMP message type.
* @param[in] index                    - the  it the table (0 to 7)
* @param[in] msgType                  - ICMP message type
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenIcmpv6MsgTypeSet
(
    IN GT_U32               index,
    IN GT_U8                msgType,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal prvTgfBrgGenIcmpv6MsgTypeGet function
* @endinternal
*
* @brief   Get current command referring IPv6 ICMP packets.
*
* @param[in] index                    - the entry  in the table (0 to 7)
*
* @param[out] msgTypePtr               - (pointer to) ICMP message type
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfBrgGenIcmpv6MsgTypeGet
(
    IN  GT_U32               index,
    OUT GT_U8                *msgTypePtr,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
);




/**
* @internal prvTgfBrgVlanIpV6IcmpToCpuEnable function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU IPv6 ICMP packets.
*
* @param[in] vlanId                   - the vlan ID
* @param[in] enable                   - GT_TRUE / GT_FALSE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIpV6IcmpToCpuEnable
(
    IN  GT_U16    vlanId,
    IN  GT_BOOL   enable
);


/**
* @internal prvTgfBrgVlanMirrorToTxAnalyzerSet function
* @endinternal
*
* @brief   Function sets Tx Analyzer port index to given VLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] vlanId                   - VLAN id
* @param[in] enable                   - Enable/disable Tx Mirroring
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] txAnalyzerIndex          - Tx Analyzer index
*                                      (APPLICABLE RANGES: 0..6)
*                                      relevant only if mirrToTxAnalyzerEn == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMirrorToTxAnalyzerSet
(
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable,
    IN  GT_U32      txAnalyzerIndex
);

/**
* @internal prvTgfBrgFdbDaLookupAnalyzerIndexSet function
* @endinternal
*
* @brief   Set analyzer index used when the FDB DA lookup has
*         <DA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NOTE: If a previous mechanism assigns a different analyzer index,
*       the higher index wins.
*
*/
GT_STATUS prvTgfBrgFdbDaLookupAnalyzerIndexSet
(
    IN GT_BOOL enable,
    IN GT_U32  index
);

/**
* @internal prvTgfBrgFdbDaLookupAnalyzerIndexGet function
* @endinternal
*
* @brief   Get analyzer index used when the FDB DA lookup has
*         <DA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - analyzer index field is NOT ZERO
*                                      GT_FALSE - analyzer index field is set to ZERO
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbDaLookupAnalyzerIndexGet
(
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
);

/**
* @internal prvTgfBrgFdbSaLookupAnalyzerIndexSet function
* @endinternal
*
* @brief   Set analyzer index used when the FDB SA lookup has
*         <SA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NOTE: If a previous mechanism assigns a different analyzer index,
*       the higher index wins.
*
*/
GT_STATUS prvTgfBrgFdbSaLookupAnalyzerIndexSet
(
    IN GT_BOOL enable,
    IN GT_U32  index
);

/**
* @internal prvTgfBrgFdbSaLookupAnalyzerIndexGet function
* @endinternal
*
* @brief   Get analyzer index used when the FDB SA lookup has
*         <SA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - analyzer index field is NOT ZERO
*                                      GT_FALSE - analyzer index field is set to ZERO
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbSaLookupAnalyzerIndexGet
(
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
);
/**
* @internal prvTgfBrgFdbVid1AssignmentEnableSet function
* @endinternal
*
* @brief   Enable/Disable using of VID1 in FDB entry.
*         The Source ID bits [8:6], SA Security Level and DA Security Level are read as 0
*         from the FDB entry when VID1 enabled.
* @param[in] enable                   - GT_FALSE: <Tag1 VID> is not written in the FDB and is not read from the FDB.
*                                      <SrcID>[8:6] can be used for src-id filtering and <SA Security Level>
*                                      and <DA Security Level> are written/read from the FDB.
*                                      GT_TRUE:  Tag1 VID> is written in the FDB and read from the FDB
*                                      as described in Mac Based VLAN FS section.
*                                      <SrcID>[8:6], <SA Security Level> and <DA Security Level>
*                                      are read as 0 from the FDB entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbVid1AssignmentEnableSet
(
    IN GT_BOOL          enable
);

/**
* @internal prvTgfBrgFdbPortVid1LearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable copying Tag1 VID from packet to FDB entry
*         at automatic learning.
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  copying Tag1 VID from packet to FDB entry,
*                                      GT_FALSE - disable, the Tag1Vid in FDB will be 0
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbPortVid1LearningEnableSet
(
        IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal prvTgfBrgFdbMacEntryMuxingModeSet function
* @endinternal
*
* @brief   Set FDB mac entry muxing modes.
*         how the 12 bits that are muxed in the FDB mac entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] muxingMode               - one of the muxing modes according to
*                                      CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or muxingMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacEntryMuxingModeSet
(
    IN CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT          muxingMode
);

/**
* @internal prvTgfBrgFdbVid1AssignmentEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status of using of VID1 in FDB entry.
*         The Source ID bits [8:6], SA Security Level and DA Security Level are read as 0
*         from the FDB entry when VID1 enabled.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: <Tag1 VID> is not written in the FDB and is not read from the FDB.
*                                      <SrcID>[8:6] can be used for src-id filtering and <SA Security Level>
*                                      and <DA Security Level> are written/read from the FDB.
*                                      GT_TRUE:  Tag1 VID> is written in the FDB and read from the FDB
*                                      as described in Mac Based VLAN FS section.
*                                      <SrcID>[8:6], <SA Security Level> and <DA Security Level>
*                                      are read as 0 from the FDB entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbVid1AssignmentEnableGet
(
    OUT GT_BOOL         *enablePtr
);
/**
* @internal prvTgfBrgFdbPortVid1LearningEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status of copying Tag1 VID from packet to FDB entry
*         at automatic learning.
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - enable copying Tag1 VID from packet to FDB entry,
*                                      GT_FALSE - disable, the Tag1Vid in FDB will be 0
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbPortVid1LearningEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal prvTgfBrgFdbMacEntryMuxingModeGet function
* @endinternal
*
* @brief   Get FDB mac entry muxing modes.
*         how the 12 bits that are muxed in the FDB mac entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
*
* @param[out] muxingModePtr            - (pointer to) one of the muxing modes according to
*                                      CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or muxingMode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacEntryMuxingModeGet
(
    OUT CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT         *muxingModePtr
);

/**
* @internal prvTgfBrgFdbHashModeSet function
* @endinternal
*
* @brief   Sets the FDB hash function mode.
*         The CRC based hash function provides the best hash index distribution
*         for random addresses and vlans.
*         The XOR based hash function provides optimal hash index distribution
*         for controlled testing scenarios, where sequential addresses and vlans
*         are often used.
* @param[in] devNum                   - device number
* @param[in] mode                     - hash function based mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbHashModeSet
(
    IN GT_U8                           devNum,
    IN PRV_TGF_MAC_HASH_FUNC_MODE_ENT  mode
);

/**
* @internal prvTgfBrgFdbHashModeGet function
* @endinternal
*
* @brief   Gets the FDB hash function mode.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - hash function based mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbHashModeGet
(
    IN    GT_U8                            devNum,
    OUT   PRV_TGF_MAC_HASH_FUNC_MODE_ENT   *modePtr
);

/**
* @internal prvTgfBrgFdbRoutingNextHopPacketCmdSet function
* @endinternal
*
* @brief   Set Packet Commands for FDB routed packets.
*
* @param[in] devNum                   - device number
* @param[in] nhPacketCmd              - Route entry command.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingNextHopPacketCmdSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PACKET_CMD_ENT         nhPacketCmd
);

/**
* @internal prvTgfBrgFdbRoutingNextHopPacketCmdGet function
* @endinternal
*
* @brief   Get Packet Commands for FDB routed packets.
*
* @param[in] devNum                   - device number
*
* @param[out] nhPacketCmdPtr           - (pointer to) Route entry command.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_VALUE             - wrong hw value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingNextHopPacketCmdGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_PACKET_CMD_ENT         *nhPacketCmdPtr
);


/**
* @internal prvTgfBrgFdbRoutingPortIpUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable using of FDB for Destination IP lookup for IP UC routing.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] protocol                 - ipv4 or ipv6 or both
* @param[in] enable                   - GT_TRUE  -  using of FDB for DIP lookup for IP UC routing,
*                                      GT_FALSE - disable using of FDB for DIP lookup for IP UC routing
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_STATE             - the setting not allowed in current state of
*                                       configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingPortIpUcEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    IN  GT_BOOL                      enable
);


/**
* @internal prvTgfBrgFdbRoutingPortIpUcEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of using of FDB for Destination IP lookup for IP UC routing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] protocol                 - ipv4 or ipv6
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable using of FDB for DIP lookup for IP UC routing,
*                                      GT_FALSE - disable using of FDB for DIP lookup for IP UC routing
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingPortIpUcEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal prvTgfBrgFdbRoutingUcRefreshEnableSet function
* @endinternal
*
* @brief   Enable/Disable refresh for UC Routing entries
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - Perform refresh for UC Routing entries
*                                      GT_FALSE - Do not perform refresh for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcRefreshEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
);

/**
* @internal prvTgfBrgFdbRoutingUcRefreshEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable refresh status for UC Routing entries
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform refresh for UC Routing entries
*                                      GT_FALSE - Do not perform refresh for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcRefreshEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal prvTgfBrgFdbRoutingUcAgingEnableSet function
* @endinternal
*
* @brief   Enable/Disable aging for UC Routing entries
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - UC routing entries are examined for aging and
*                                      an AA message is sent out for an entry that is
*                                      aged out. Auto delete is not performed
*                                      GT_FALSE - UC routing entries are not examined for aging
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcAgingEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
);

/**
* @internal prvTgfBrgFdbRoutingUcAgingEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable aging status for UC Routing entries
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - UC routing entries are examined for aging and
*                                      an AA message is sent out for an entry that is
*                                      aged out. Auto delete is not performed
*                                      GT_FALSE - UC routing entries are not examined for aging
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcAgingEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal prvTgfBrgFdbRoutingUcTransplantEnableSet function
* @endinternal
*
* @brief   Enable/Disable transplanting for UC Routing entries
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Perform address transplanting for UC Routing entries
*                                      GT_FALSE - Do not perform address transplanting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcTransplantEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
);

/**
* @internal prvTgfBrgFdbRoutingUcTransplantEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable transplanting status for UC Routing entries
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform address transplanting for UC Routing entries
*                                      GT_FALSE - Do not perform address transplanting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcTransplantEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal prvTgfBrgFdbRoutingUcDeleteEnableSet function
* @endinternal
*
* @brief   Enable/Disable delete for UC Routing entries
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Perform address deleting for UC Routing entries
*                                      GT_FALSE - Do not perform address deleting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcDeleteEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
);


/**
* @internal prvTgfBrgFdbRoutingUcDeleteEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable delete status for UC Routing entries
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform address deleting for UC Routing entries
*                                      GT_FALSE - Do not perform address deleting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcDeleteEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal prvTgfBrgFdbRoutingUcAAandTAToCpuSet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting
*         mechanism. When an entry is aged out or is transplanted, a message can
*         be forwarded to the CPU.
*         This field enables/disables forwarding address aged out messages and
*         transplanted address messages to the Host CPU for UC route FDB entries.
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*                                      GT_FALSE - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcAAandTAToCpuSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
);

/**
* @internal prvTgfBrgFdbRoutingUcAAandTAToCpuGet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting mechanism
*         When an entry is aged out or is transplanted, a message can be forwarded
*         to the CPU.
*         This field get the enables/disable status of forwarding address aged out
*         messages and transplanted address messages to the Host CPU for UC route FDB entries.
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*                                      GT_FALSE - AA and TA messages are not forwarded to the CPU for
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcAAandTAToCpuGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal prvTgfBrgFdbBankCounterUpdate function
* @endinternal
*
* @brief   Update (increment/decrement) the counter of the specific bank
*         The function will fail (GT_BAD_STATE) if the PP is busy with previous
*         update.
*         the application can determine if PP is ready by calling
*         cpssDxChBrgFdbBankCounterUpdateStatusGet.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*         NOTE: this function is needed to be called after calling one of the next:
*         'write by index' - cpssDxChBrgFdbMacEntryWrite ,
*         cpssDxChBrgFdbPortGroupMacEntryWrite
*         'invalidate by index' - cpssDxChBrgFdbMacEntryInvalidate ,
*         cpssDxChBrgFdbPortGroupMacEntryInvalidate
*         The application logic should be:
*         if last action was 'write by index' then :
*         if the previous entry (in the index) was valid --
*         do no call this function.
*         if the previous entry (in the index) was not valid --
*         do 'increment'.
*         if last action was 'invalidate by index' then :
*         if the previous entry (in the index) was valid --
*         do 'decrement'.
*         if the previous entry (in the index) was not valid --
*         do no call this function.
* @param[in] devNum                   - device number
* @param[in] bankIndex                - the index of the bank
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] incOrDec                 - increment or decrement the counter by one.
*                                      GT_TRUE - increment the counter
*                                      GT_FALSE - decrement the counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - bankIndex > 15
* @retval GT_BAD_STATE             - the PP is not ready to get a new update from CPU.
*                                       (PP still busy with previous update)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbBankCounterUpdate
(
    IN GT_U8                        devNum,
    IN GT_U32                       bankIndex,
    IN GT_BOOL                      incOrDec
);

/**
* @internal prvTgfBrgFdbBankCounterValueGet function
* @endinternal
*
* @brief   Get the value of counter of the specific FDB table bank.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] bankIndex                - the index of the bank
*                                      (APPLICABLE RANGES: 0..15)
*
* @param[out] valuePtr                 - (pointer to)the value of the counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbBankCounterValueGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          bankIndex,
    OUT GT_U32          *valuePtr
);

/**
* @internal prvTgfBrgMcPhysicalSourceFilteringEnableSet function
* @endinternal
*
* @brief   Enable/disable source physical port/trunk filtering for packets that are
*         forwarded to a VIDX target.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable filtering
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgMcPhysicalSourceFilteringEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
);


/**
* @internal prvTgfBrgMcPhysicalSourceFilteringEnableGet function
* @endinternal
*
* @brief   Get the enabling status of source physical port/trunk filtering for
*         packets that are forwarded to a VIDX target.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - filtering enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgMcPhysicalSourceFilteringEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
);


/**
* @internal prvTgfBrgVlanForceNewDsaToCpuEnableSet function
* @endinternal
*
* @brief   Enable / Disable preserving the original VLAN tag
*         and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] enable                   - GT_TRUE - Double Tag to CPU: Packets sent to the
*                                      CPU whose Vlan Id was modified by the device,
*                                      with a DSA tag that contains the newly
*                                      assigned Vlan Id and the original Vlan tag
*                                      (or tags) that is preserved after the DSA tag.
*                                      - GT_FALSE - If the original packet contained a Vlan tag,
*                                      it is removed and the DSA tag contains
*                                      the newly assigned Vlan Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanForceNewDsaToCpuEnableSet
(
    IN GT_BOOL enable
);


/**
* @internal prvTgfBrgVlanForceNewDsaToCpuEnableGet function
* @endinternal
*
* @brief   Get Force New DSA To Cpu mode for enabling / disabling
*         preserving the original VLAN tag and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to Force New DSA To Cpu mode.
*                                      - GT_TRUE - Double Tag to CPU: Packets sent to the
*                                      CPU whose Vlan Id was modified by the device,
*                                      with a DSA tag that contains the newly
*                                      assigned Vlan Id and the original Vlan tag
*                                      (or tags) that is preserved after the DSA tag.
*                                      - GT_FALSE - If the original packet contained a Vlan tag,
*                                      it is removed and the DSA tag contains
*                                      the newly assigned Vlan Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanForceNewDsaToCpuEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal prvTgfBrgFdbMacBankArrayGet function
* @endinternal
*
* @brief   Get FDB MAC addresses for different banks according to FDB hash mode
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacBankArrayGet
(
    IN      GT_U8               devNum,
    OUT     TGF_MAC_ADDR        prvTgfSaMacArr[],
    OUT     GT_U32              *numOfBanksPtr
);

/**
* @internal prvTgfBrgVlanEntryStore function
* @endinternal
*
* @brief   Store content of VLAN entry in static buffer
*
* @param[in] vid                      - VLAN id
*                                       None
*/
GT_VOID prvTgfBrgVlanEntryStore
(
    GT_U16 vid
);

/**
* @internal prvTgfBrgVlanEntryRestore function
* @endinternal
*
* @brief   Restore content of VLAN entry in device from static buffer
*
* @param[in] vid                      - VLAN id
*                                       None
*/
GT_VOID prvTgfBrgVlanEntryRestore
(
    GT_U16 vid
);

/**
* @internal prvTgfBrgVlanFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Set FCoE Forwarding Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] vlanId                   - VLAN ID which FCoE Forwarding enabled/disabled..
* @param[in] enable                   - FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanFcoeForwardingEnableSet
(
    IN  GT_U16           vlanId,
    IN  GT_BOOL          enable
);

/**
* @internal prvTgfBrgVlanFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return FCoE Forwarding Enable status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] enablePtr                - (pointer to) FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanFcoeForwardingEnableGet
(
    IN  GT_U16           vlanId,
    OUT GT_BOOL          *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfBridgeGenh */

