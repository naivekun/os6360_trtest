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
* @file cpssDxChBrgVlan.h
*
* @brief CPSS DxCh VLAN facility API
*
* @version   57
********************************************************************************
*/

#ifndef __cpssDxChBrgVlanh
#define __cpssDxChBrgVlanh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>

/**
* @struct CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC
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

} CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC;

/**
* @struct CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC
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
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosAssignPrecedence;

    /** @brief enable modification of the UP field
     *  GT_FALSE - If the packet was received tagged and
     *  transmitted tagged, the user priority
     *  is not modified. If the packet was
     *  received untagged, the user priority is
     *  set according to the default port
     *  user priority.
     *  GT_TRUE - If the packet is transmitted tagged,
     *  the user priority is set according to
     *  the QoS Profile table.
     */
    GT_BOOL enableModifyUp;

    /** @brief enable modification of the DSCP field (or EXP for MPLS)
     *  GT_FALSE - The IPv4/6 packet DSCP is not modified.
     *  GT_TRUE - The IPv4/6 packet DSCP is updated
     *  according to the QoS Profile assignment.
     */
    GT_BOOL enableModifyDscp;

} CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC;


/**
* @enum CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT
 *
 * @brief This enum defines IP Control to CPU Enable Mode
*/
typedef enum{

    /** @brief Disable both IPv4 and Ipv6 Control
     *  traffic to CPU
     */
    CPSS_DXCH_BRG_IP_CTRL_NONE_E,

    /** Enable IPv4 Control traffic to CPU */
    CPSS_DXCH_BRG_IP_CTRL_IPV4_E,

    /** Enable IPv6 Control traffic to CPU */
    CPSS_DXCH_BRG_IP_CTRL_IPV6_E,

    /** @brief Enable both IPv4 and IPv6 traffic
     *  to CPU
     */
    CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E

} CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT
 *
 * @brief This enum defines Vlan packet type
*/
typedef enum{

    /** Unknown unicast */
    CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E,

    /** @brief Unregistered Non IP
     *  Multicast
     */
    CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E,

    /** @brief Unregistered IPv4
     *  Multicast
     */
    CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E,

    /** @brief Unregistered IPv6
     *  Multicast
     */
    CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E,

    /** @brief Unregistered Non IPv4
     *  Broadcast
     */
    CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E,

    /** @brief Unregistered IPv4
     *  Broadcast
     */
    CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E

} CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT
 *
 * @brief Enumeration of assigning L2 multicast VIDX for Broadcast,
 * Unregistered Multicast and unknown Unicast packets.
*/
typedef enum{

    /** @brief Unregistered MC Mode Assign <Flood VIDX>
     *  from VLAN entry for Unregistered Multicast
     *  only. Broadcast and Unknown Unicast packets
     *  are assigned VIDX = 0xFFF which implies
     *  flooding the packet to all VLAN port members.
     */
    CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E,

    /** @brief Apply <Flood VIDX> from VLAN entry for ALL
     *  flooded traffic (I.e. Unreg MC, BC, Unk UC)
     */
    CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E

} CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT
 *
 * @brief Enumeration of port isolation commands.
*/
typedef enum{

    /** @brief Port Isolation feature
     *  is disabled
     */
    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E,

    /** @brief L2 packets forwaring
     *  enabled according to
     *  port isolation L2 table
     */
    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E,

    /** @brief L3 packets forwaring
     *  enabled according to
     *  port isolation L3 table
     */
    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E,

    /** @brief L2&L3 packets forwaring
     *  enabled according to
     *  port isolation L2&L3
     *  tables
     */
    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E

} CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT
 *
 * @brief Enumeration of Unregistered IP Multicast eVidx Mode.
 * The eVidx assigned to packet calculated using
 * VLAN entry fields unregIpmEVidx and floodVidx
*/
typedef enum{

    /** The eVIDX value assigned to unregistered IP MC packets is unregIpmEVidx */
    CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E,

    /** @brief The eVIDX value assigned to unregistered IPv4 MC packets is unregIpmEVidx.
     *  The eVIDX value assigned to unregistered IPv6 MC packets is floodVidx.
     */
    CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E,

    /** @brief The eVIDX value assigned to unregistered IPv6 MC packets is unregIpmEVidx.
     *  The eVIDX value that is assigned to unregistered IPv4 MC packets is floodVidx.
     *  IPV6 unregIpmEVidx
     */
    CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E,

    /** @brief The eVIDX value that is assigned to unregistered IPv4 MC packets is unregIpmEVidx.
     *  The eVIDX value assigned to unregistered IPv6 MC packets is unregIpmEVidx+1.
     */
    CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E

} CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT;

/**
* @struct CPSS_DXCH_BRG_VLAN_INFO_STC
 *
 * @brief A structure to hold generic VLAN entry info.
*/
typedef struct{

    /** @brief Unknown Source Address is security breach event.
     *  GT_TRUE - Packets with unknown MAC Source Addresses
     *  generate security breach events.
     *  GT_FALSE - Packets with unknown MAC Source Addresses
     *  assigned to this VLAN do not generate a
     *  security breach event and are processed
     *  according to the normal Source Address
     *  learning rules.
     */
    GT_BOOL unkSrcAddrSecBreach;

    /** @brief The command of Unregistered non IP Multicast packets
     *  (with a Multicast MAC_DA! = 01-00-5e-xx-xx-xx or
     *  33-33-xx-xx-xx-xx) assigned to this VLAN.
     */
    CPSS_PACKET_CMD_ENT unregNonIpMcastCmd;

    /** @brief The command of Unregistered IPv4 Multicast packets
     *  (with MAC_DA= 01-00-5e-xx-xx-xx) assigned to
     *  this VLAN.
     */
    CPSS_PACKET_CMD_ENT unregIpv4McastCmd;

    /** @brief The command of Unregistered IPv6 Multicast packets
     *  (with MAC_DA= 33-33-xx-xx-xx-xx) assigned to
     *  this VLAN.
     */
    CPSS_PACKET_CMD_ENT unregIpv6McastCmd;

    /** @brief The command of Unknown Unicast packets assigned
     *  to this VLAN.
     */
    CPSS_PACKET_CMD_ENT unkUcastCmd;

    /** @brief The command of Unregistered IPv4 Broadcast packets
     *  assigned to this VLAN.
     */
    CPSS_PACKET_CMD_ENT unregIpv4BcastCmd;

    /** @brief The command of Unregistered non
     *  packets assigned to this VLAN.
     */
    CPSS_PACKET_CMD_ENT unregNonIpv4BcastCmd;

    /** @brief Enable IGMP Trapping or Mirroring to CPU according
     *  to the global setting
     *  GT_TRUE - enable
     *  GT_FALSE - disable
     */
    GT_BOOL ipv4IgmpToCpuEn;

    /** @brief Enable to mirror packets to Rx Analyzer port
     *  GT_TRUE - Ingress mirrored traffic assigned to
     *  this Vlan to the analyzer port
     *  GT_FALSE - Don't ingress mirrored traffic assigned
     *  to this Vlan to the analyzer port
     */
    GT_BOOL mirrToRxAnalyzerEn;

    /** @brief Enable/Disable ICMPv6 trapping or mirroring to
     *  the CPU, according to global ICMPv6 message type
     *  GT_TRUE - enable
     *  GT_FALSE - disable
     */
    GT_BOOL ipv6IcmpToCpuEn;

    /** @brief IPv4/6 control traffic trapping/mirroring to the CPU
     *  If IPV4 set for this vlan and the corresponding
     *  mechanism specific global enable is set and the
     *  packet is IPV4,then the packet is subject to
     *  trapping/mirroring for the following packet types:
     *  1. IP Link local Multicast control check
     *  2. ARP BC
     *  3. RIPv1
     *  If IPv6 set for this vlan and the packet is IPv6,
     *  then the packet is subject to trapping/mirroring
     *  for the following packet types:
     *  1. IP Link local Multicast control check
     *  2. IPv6 Neighbor Solicitation Multicast
     */
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT ipCtrlToCpuEn;

    /** @brief when ipv4IpmBrgEn enabled:
     *  IPV4 Multicast Bridging mode,
     *  when ipv4IpmBrgEn disabled:
     *  In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices:
     *  IPV4 Multicast Routing Engine (IP MLL-based IGMPv3
     *  bridging) mode:
     *  - CPSS_BRG_IPM_SGV_E - Tag0 VLAN-ID mode;
     *  - CPSS_BRG_IPM_GV_E - VRF-ID mode.
     *  In other devices:
     *  Not relevant when ipv4IpmBrgEn disabled.
     */
    CPSS_BRG_IPM_MODE_ENT ipv4IpmBrgMode;

    /** @brief when ipv6IpmBrgEn enabled:
     *  IPV6 Multicast Bridging mode,
     *  when ipv6IpmBrgEn disabled:
     *  In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices:
     *  IPV6 Multicast Routing Engine (IP MLL-based IGMPv3
     *  bridging) mode:
     *  - CPSS_BRG_IPM_SGV_E - Tag0 VLAN-ID mode;
     *  - CPSS_BRG_IPM_GV_E - VRF-ID mode.
     *  In other devices:
     *  Not relevant when ipv6IpmBrgEn disabled.
     */
    CPSS_BRG_IPM_MODE_ENT ipv6IpmBrgMode;

    /** @brief IPv4 Multicast Bridging Enable
     *  GT_TRUE - IPv4 multicast packets are bridged
     *  according to ipv4IpmBrgMode
     *  GT_FALSE - IPv4 multicast packets are bridged
     *  according to MAC DA
     */
    GT_BOOL ipv4IpmBrgEn;

    /** @brief IPv6 Multicast Bridging Enable
     *  GT_TRUE - IPv6 multicast packets are bridged
     *  according to ipv6IpmBrgMode
     *  GT_FALSE - IPv4 multicast packets are bridged
     *  according to MAC DA
     */
    GT_BOOL ipv6IpmBrgEn;

    /** @brief IPv6 Site Id Mode
     *  Used by the router for IPv6 scope checking.
     *  Not supported and ignored for
     *  DxCh1, DxCh1_Diamond and xCat2 devices.
     *  Supported for other devices.
     */
    CPSS_IP_SITE_ID_ENT ipv6SiteIdMode;

    /** @brief Enable/Disable IPv4 Unicast Routing on the vlan
     *  GT_TRUE - enable,
     *  GT_FALSE - disable
     */
    GT_BOOL ipv4UcastRouteEn;

    /** @brief Enable/Disable IPv4 Multicast Routing on the vlan
     *  GT_TRUE - enable,
     *  GT_FALSE - disable
     *  Not supported and ignored for
     *  DxCh1, DxCh1_Diamond and xCat2 devices.
     *  Supported for other devices.
     */
    GT_BOOL ipv4McastRouteEn;

    /** @brief Enable/Disable IPv6 Unicast Routing on the vlan
     *  GT_TRUE - enable,
     *  GT_FALSE - disable
     */
    GT_BOOL ipv6UcastRouteEn;

    /** @brief Enable/Disable IPv6 Multicast Routing on the vlan
     *  GT_TRUE - enable,
     *  GT_FALSE - disable
     *  Not supported and ignored for
     *  DxCh1, DxCh1_Diamond and xCat2 devices.
     *  Supported for other devices.
     */
    GT_BOOL ipv6McastRouteEn;

    /** stp group (APPLICABLE RANGES: 0..255) */
    GT_U32 stgId;

    /** @brief Enable/Disable automatic learning for this VLAN
     *  GT_TRUE - Disable automatic learning
     *  GT_FALSE - Enable automatic learning
     *  Not supported and ignored for
     *  DxCh1 and DxCh1_Diamond devices.
     *  Supported for other devices.
     */
    GT_BOOL autoLearnDisable;

    /** @brief Enable/Disable new address message sending to CPU
     *  GT_TRUE - enable new address message to CPU
     *  GT_FALSE - disable new address message to CPU
     *  Not supported and ignored for
     *  DxCh1 and DxCh1_Diamond devices.
     *  Supported for other devices.
     */
    GT_BOOL naMsgToCpuEn;

    /** @brief The index that this Vlan's MRU configuration packets
     *  are associated with (APPLICABLE RANGES: 0..7)
     *  Not supported and ignored for
     *  DxCh1 and DxCh1_Diamond devices.
     *  Supported for other devices.
     */
    GT_U32 mruIdx;

    /** @brief Enables trapping/mirroring of Broadcast UDP packets
     *  based on their destination UDP port
     *  GT_TRUE - enable,
     *  GT_FALSE - disable
     *  Not supported and ignored for
     *  DxCh1 and DxCh1_Diamond devices.
     *  Supported for other devices.
     */
    GT_BOOL bcastUdpTrapMirrEn;

    /** @brief Virtual Router ID
     *  (APPLICABLE DEVICES: DxCh2; DxCh3; xCat; xCat3; Lion;Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  (APPLICABLE RANGES: 0..4095)
     */
    GT_U32 vrfId;

    /** @brief The multicast group to which the flooded packet
     *  is to be transmitted.
     *  Not supported and ignored for
     *  DxCh1, DxCh1_Diamond, DxCh2 and DxCh3 devices.
     *  Supported for other devices.
     *  valid ranges:
     *  (APPLICABLE RANGES: xCat2 0..1023, 4095)
     *  (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, Lion, Lion2 0..4095)
     *  (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..0xFFFF)
     */
    GT_U16 floodVidx;

    /** @brief Mode for assigning L2 multicast VIDX for
     *  Broadcast, Unregistered Multicast and
     *  unknown Unicast packets.
     *  Not supported and ignored for
     *  DxCh1, DxCh1_Diamond, DxCh2 and DxCh3 devices.
     */
    CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT floodVidxMode;

    /** @brief Port Isolation Mode.
     *  Not supported and ignored for
     *  DxCh1, DxCh1_Diamond, DxCh2 and DxCh3 devices.
     */
    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT portIsolationMode;

    /** @brief Enable/disable local switching of known Unicast
     *  traffic on this VLAN.
     *  GT_TRUE - enable
     *  GT_FALSE - disable
     *  In order to enable this feature the per port
     *  local switching should be enabled by the
     *  cpssDxChBrgGenUcLocalSwitchingEnable.
     *  Not supported and ignored for
     *  DxCh1, DxCh1_Diamond, DxCh2 and DxCh3 devices.
     */
    GT_BOOL ucastLocalSwitchingEn;

    /** @brief Enable/disable local switching of Multicast,
     *  unknown Unicast and Broadcast traffic on
     *  this VLAN.
     *  GT_TRUE - enable
     *  GT_FALSE - disable
     *  In order to enable this feature the per port
     *  local switching should be enabled by the
     *  cpssDxChBrgPortEgressMcastLocalEnable.
     *  Not supported and ignored for
     *  DxCh1, DxCh1_Diamond, DxCh2 and DxCh3 devices.
     */
    GT_BOOL mcastLocalSwitchingEn;

    /** @brief Ingress Analyzer Mirror Index (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 mirrToRxAnalyzerIndex;

    /** @brief Enable to mirror packets to Tx Analyzer port
     *  GT_TRUE - Egress mirrored traffic assigned to
     *  this Vlan to the analyzer port
     *  GT_FALSE - Don't egress mirrored traffic assigned
     *  to this Vlan to the analyzer port
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL mirrToTxAnalyzerEn;

    /** @brief Egress Analyzer Mirror Index (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 mirrToTxAnalyzerIndex;

    /** @brief Egress Analyzer Mirror Mode
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT mirrToTxAnalyzerMode;

    /** @brief Forwarding ID value
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 fidValue;

    /** @brief bridge forwarding decision for packets with
     *  unknown Source Address. supported commands:
     *  CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
     *  CPSS_PACKET_CMD_DROP_SOFT_E
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PACKET_CMD_ENT unknownMacSaCmd;

    /** @brief When enabled, IPv4 MC/BC packets in this VLAN
     *  are mirrored to the analyzer specified in
     */
    GT_BOOL ipv4McBcMirrToAnalyzerEn;

    /** @brief field.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  ipv4McBcMirrToAnalyzerIndex - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 ipv4McBcMirrToAnalyzerIndex;

    /** @brief When enabled, IPv6 MC packets in this VLAN are
     *  mirrored to the analyzer specified in
     */
    GT_BOOL ipv6McMirrToAnalyzerEn;

    /** @brief field
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  ipv6McMirrToAnalyzerIndex - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 ipv6McMirrToAnalyzerIndex;

    /** @brief When this field is enabled (GT_TRUE),
     *  and the ePort<FCoE Forwarding Enable> is enabled,
     *  the packet an be FCoE Forwarded.
     *  (depending on additional trigger requirements).
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL fcoeForwardingEn;

    /** @brief Unregistered IPM packet eVidx mode
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT unregIpmEVidxMode;

    /** @brief eVidx for assignment to Unregistered IPM packets
     *  If this field contains 0, eVidx will be assigned
     *  from floodVidx regardless unregIpmEVidxMode
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 unregIpmEVidx;

    /** @brief FDB Lookup Key Mode. Determine if VID1 will be used
     *  as part of a key to lookup an entry in FDB
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT fdbLookupKeyMode;

} CPSS_DXCH_BRG_VLAN_INFO_STC;

/**
* @enum CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT
 *
 * @brief Enumeration of tagging egress commands.
*/
typedef enum{

    /** @brief if Tag0 and/or Tag1
     *  were classified in
     *  the incoming packet,
     *  they are removed
     *  from the packet.
     */
    CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E,

    /** @brief packet egress with
     *  Tag0 (Tag0 as
     *  defined in ingress
     *  pipe).
     */
    CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E,

    /** @brief packet egress with
     *  Tag1 (Tag1 as
     *  defined in ingress
     *  pipe).
     */
    CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E,

    /** @brief Outer Tag0, Inner
     *  Tag1 (tag swap).
     */
    CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,

    /** @brief Outer Tag1, Inner
     *  Tag0 (tag swap).
     */
    CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E,

    /** @brief TAG0 is added to
     *  the packet
     *  regardless of
     *  whether Tag0 and
     *  TAG1 were
     *  classified in the
     *  incoming packet.
     *  (APPLICABLE DEVICES: xCat; xCat3; Lion; xCat2; Lion2)
     */
    CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E,

    /** @brief the incoming packet
     *  outer tag is
     *  removed, regardless
     *  of whether it is
     *  Tag0 or Tag1. This
     *  operation is a NOP
     *  if the packet
     *  arrived with neither
     *  Tag0 nor Tag1
     *  classified.
     */
    CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E,

    /** @brief Do not modify any tag,
     *  packet tags are sent
     *  as received (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Note:
     *  Relevant for devices with TR101 feature support.
     */
    CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E

} CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT;


/**
* @struct CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC
 *
 * @brief structure that hold an array of ports' Tagging commands
*/
typedef struct{

    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT portsCmd[CPSS_MAX_PORTS_NUM_CNS];

} CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC;

/**
* @enum CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT
 *
 * @brief Enumeration of local switching traffic types.
*/
typedef enum{

    /** known Unicast */
    CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E,

    /** @brief unknown Unicast,
     *  Broadcast, Multicast
     */
    CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E

} CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT;


/**
* @enum CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT
 *
 * @brief Enumeration of Tag1 removal mode when VID is zero.
*/
typedef enum{

    /** don't remove Tag1 */
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E,

    /** @brief enable only if
     *  ingress was NOT double tag (single tag or untagged)
     *  (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2)
     */
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E,

    /** @brief enable regardless of ingress
     *  tagging
     */
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E,

    /** @brief enable only if
     *  ingress was without tag1
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E

} CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT
 *
 * @brief This enum defines the egress ePort tag state.
*/
typedef enum{

    /** @brief Use tag state from
     *  the eVLAN entry, according to egress physical port
     */
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E,

    /** Use tag state from ePort entry */
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E

} CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_EVB_EGRESS_EPORT_TABLE_ACCESS_MODE_ENT
 *
 * @brief This enum defines the EVB Egress ePort table access mode.
*/
typedef enum{

    /** @brief The EVB Egress ePort attribute table is always accessed
     *  with target port / ePort of the original packet
     */
    CPSS_DXCH_BRG_VLAN_EVB_EGRESS_EPORT_TABLE_ACCESS_ORIGINAL_TARGET_MODE_E,

    /** @brief When the Desc<OutGoingMtagCmd> of a packet is
     *  ToTargetSniffer, the EVB Egress ePort Table is
     *  accessed using the <SniffTRGePort>.
     */
    CPSS_DXCH_BRG_VLAN_EVB_EGRESS_EPORT_TABLE_ACCESS_SNIFF_TARGET_MODE_E

} CPSS_DXCH_BRG_VLAN_EVB_EGRESS_EPORT_TABLE_ACCESS_MODE_ENT;


/**
* @enum CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT
 *
 * @brief This enum defines the Vlan Push command.
*/
typedef enum{

    /** none */
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E,

    /** Push single Tag on tunnel packet layer 2 header */
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E,

    /** @brief Push single Tag on passenger packet layer 2 header,
     *  prior to TS encapsulation
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E

} CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT
 *
 * @brief Enumeration of TAG0/TAG1 TPID Select mode .
*/
typedef enum{

    /** @brief TPID of Tag0/Tag1 is
     *  selected according to <Egress Tag0/Tag1 TPID select>
     */
    CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E,

    /** @brief TPID of Tag0/Tag1 is
     *  selected according to VID0/VID1 to TPID select table
     */
    CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E

} CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT
 *
 * @brief Enumeration of indexing mode to access VLAN related tables.
*/
typedef enum{

    /** indexing is based on VLAN value. */
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,

    /** indexing is based on Tag1 VID value. */
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E

} CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT
 *
 * @brief This enum defines kinds of ingress/egress VLAN translation
*/
typedef enum{

    /** disable translation */
    CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E = GT_FALSE,

    /** @brief translate VID0, use
     *  Ingress/Egress Vlan Translation Table
     */
    CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E    = GT_TRUE,

    /** @brief translate to VID0,
     *  use eVLAN<SERVICEID>[23:12].
     *  Is relevant to egress translation only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E,

    /** @brief translate to
     *  VID0, VID1, use eVlan<SERVICEID>.
     *  Is relevant to egress translation only.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E

} CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT;

/**
* @enum CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT
 *
 * @brief Enumeration of VLAN etherType modes
*/
typedef enum{

    /** @brief EtherType for Tag0. Ingress recognition of Tag0 existence;
     *  Egress building of Tag0 etherType.
     */
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E = CPSS_VLAN_ETHERTYPE0_E,

    /** @brief EtherType for Tag1. Ingress recognition of Tag1 existence;
     *  Egress building of Tag1 etherType.
     */
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E,

    /** @brief EtherType for Passenger Tag0. Ingress recognition of Passenger Tag0
     *  existence; Ingress building of Passenger Tag0 etherType.
     *  (APPLICABLE DEVICES: xCat, xCat3). xCat device: applicable starting from revision C0
     */
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E,

    /** @brief EtherType for Passenger Tag1. Ingress recognition of Passenger Tag0
     *  existence; Ingress building of Passenger Tag0 etherType.
     *  (APPLICABLE DEVICES: xCat, xCat3). xCat device: applicable starting from revision C0
     */
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E,

    /** @brief EtherType for Tunnel Start Tag. Egress recognition of Tunnel
     *  Start Tag existence; Egress building of Tunnel Start Tag etherType.
     *  (APPLICABLE DEVICES: xCat, xCat3). xCat device: applicable starting from revision C0
     */
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E

} CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT;

/**
* @enum GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT
 *
 * @brief Enumeration of number of bytes to pop
*/
typedef enum{

    /** pop none (do not pop any byte). */
    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E    = 0,

    /** pop 4 bytes. */
    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_4_E       = 1,

    /** pop 8 bytes. */
    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_8_E       = 2,

    /** pop 6 bytes. */
    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_6_E       = 3

} GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT;


/**
* @internal cpssDxChBrgVlanInit function
* @endinternal
*
* @brief   Initialize VLAN for specific device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on system init error.
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanInit
(
    IN GT_U8    devNum
);

/**
* @internal cpssDxChBrgVlanEntryWrite function
* @endinternal
*
* @brief   Builds and writes vlan entry to HW.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[in] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
*                                      The parameter is relevant for DxCh1, DxCh2
*                                      and DxCh3 devices.
*                                      The parameter is relevant for xCat and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat and above devices with TR101 feature
*                                      support.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
* @retval or vlanInfoPtr           ->stgId is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEntryWrite
(
    IN  GT_U8                                 devNum,
    IN  GT_U16                                vlanId,
    IN  CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    IN  CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    IN  CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN  CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
);

/**
* @internal cpssDxChBrgVlanEntriesRangeWrite function
* @endinternal
*
* @brief   Function set multiple vlans with the same configuration
*         function needed for performances , when the device use "indirect" access
*         to the vlan table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - first VLAN Id
* @param[in] numOfEntries             - number of entries (vlans) to set
* @param[in] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[in] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
*                                      The parameter is relevant for DxCh1, DxCh2
*                                      and DxCh3 devices.
*                                      The parameter is relevant for xCat and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat and above devices with TR101 feature
*                                      support.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or vlanId
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
* @retval or vlanInfoPtr           ->stgId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEntriesRangeWrite
(
    IN  GT_U8                                 devNum,
    IN  GT_U16                                vlanId,
    IN  GT_U32                                numOfEntries,
    IN  CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    IN  CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    IN  CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN  CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
);

/**
* @internal cpssDxChBrgVlanEntryRead function
* @endinternal
*
* @brief   Read vlan entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
*
* @param[out] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[out] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
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
* @param[out] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEntryRead
(
    IN  GT_U8                                devNum,
    IN  GT_U16                               vlanId,
    OUT CPSS_PORTS_BMP_STC                   *portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC                   *portsTaggingPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC          *vlanInfoPtr,
    OUT GT_BOOL                              *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr
);

/**
* @internal cpssDxChBrgVlanMemberAdd function
* @endinternal
*
* @brief   Add new port member to vlan entry. This function can be called only for
*         add port belongs to device that already member of the vlan.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
* @param[in] isTagged                 - GT_TRUE, to set the port as tagged member,
*                                      GT_FALSE, to set the port as untagged
*                                      The parameter is relevant for DxCh1, DxCh2
*                                      and DxCh3 devices.
*                                      The parameter is relevant for xCat and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat and above devices with TR101 feature
*                                      support.
* @param[in] portTaggingCmd           - port tagging command
*                                      The parameter is relevant only for xCat and above
*                                      with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vid or portTaggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case that added port belongs to device that is new device for vlan
*       other function is used - cpssDxChBrgVlanEntryWrite.
*
*/
GT_STATUS cpssDxChBrgVlanMemberAdd
(
    IN  GT_U8                   devNum,
    IN  GT_U16                  vlanId,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 isTagged,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT portTaggingCmd
);

/**
* @internal cpssDxChBrgVlanPortDelete function
* @endinternal
*
* @brief   Delete port member from vlan entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortDelete
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN GT_PHYSICAL_PORT_NUM         portNum
);

/**
* @internal cpssDxChBrgVlanMemberSet function
* @endinternal
*
* @brief   Set specific member at VLAN entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
* @param[in] isMember                 - GT_TRUE,  port would set as member
*                                      GT_FALSE, port would set as not-member
* @param[in] isTagged                 - GT_TRUE, to set the port as tagged member,
*                                      GT_FALSE, to set the port as untagged
*                                      The parameter is relevant for DxCh1, DxCh2
*                                      and DxCh3 devices.
*                                      The parameter is relevant for xCat and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat and above devices with TR101 feature
*                                      support.
* @param[in] taggingCmd               - port tagging command
*                                      The parameter is relevant only for xCat and above
*                                      with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or taggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMemberSet
(
    IN  GT_U8                   devNum,
    IN  GT_U16                  vlanId,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 isMember,
    IN  GT_BOOL                 isTagged,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCmd
);


/**
* @internal cpssDxChBrgVlanPortVidGet function
* @endinternal
*
* @brief   For ingress direction : Get port's default VID0.
*         For egress direction : Get port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[out] vidPtr                   - default VLAN ID.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVidGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN CPSS_DIRECTION_ENT          direction,
    OUT  GT_U16          *vidPtr
);

/**
* @internal cpssDxChBrgVlanPortVidSet function
* @endinternal
*
* @brief   For ingress direction : Set port's default VID0.
*         For egress direction : Set port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] vlanId                   - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVidSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN CPSS_DIRECTION_ENT          direction,
    IN  GT_U16          vlanId
);

/**
* @internal cpssDxChBrgVlanPortIngFltEnable function
* @endinternal
*
* @brief   Enable/disable Ingress Filtering for specific port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE,  ingress filtering
*                                      GT_FALSE, disable ingress filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortIngFltEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgVlanPortIngFltEnableGet function
* @endinternal
*
* @brief   Get status of Ingress Filtering for specific port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS cpssDxChBrgVlanPortIngFltEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortProtoClassVlanEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortProtoClassVlanEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgVlanPortProtoClassVlanEnableGet function
* @endinternal
*
* @brief   Get status of Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanPortProtoClassVlanEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortProtoClassQosEnable function
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
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortProtoClassQosEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgVlanPortProtoClassQosEnableGet function
* @endinternal
*
* @brief   Get status of Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanPortProtoClassQosEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChBrgVlanProtoClassSet function
* @endinternal
*
* @brief   Write etherType and encapsulation of Protocol based classification
*         for specific device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3 0..7;
*                                      xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
* @param[in] etherType                - Ether Type or DSAP/SSAP
* @param[in] encListPtr               - encapsulation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanProtoClassSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum,
    IN GT_U16                           etherType,
    IN CPSS_PROT_CLASS_ENCAP_STC        *encListPtr
);

/**
* @internal cpssDxChBrgVlanProtoClassGet function
* @endinternal
*
* @brief   Read etherType and encapsulation of Protocol based classification
*         for specific device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3 0..7;
*                                      xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @param[out] etherTypePtr             - (pointer to) Ether Type or DSAP/SSAP
* @param[out] encListPtr               - (pointer to) encapsulation.
* @param[out] validEntryPtr            - (pointer to) validity bit
*                                      GT_TRUE - the entry is valid
*                                      GT_FALSE - the entry is invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanProtoClassGet
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum,
    OUT GT_U16                          *etherTypePtr,
    OUT CPSS_PROT_CLASS_ENCAP_STC       *encListPtr,
    OUT GT_BOOL                         *validEntryPtr
);

/**
* @internal cpssDxChBrgVlanProtoClassInvalidate function
* @endinternal
*
* @brief   Invalidates entry of Protocol based classification for specific device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3 0..7;
*                                      xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanProtoClassInvalidate
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum
);

/**
* @internal cpssDxChBrgVlanPortProtoVlanQosSet function
* @endinternal
*
* @brief   Write Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3 0..7;
*                                      xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
* @param[in] vlanCfgPtr               - pointer to VID parameters
* @param[in] qosCfgPtr                - pointer to QoS parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is possible to update only VLAN or QOS parameters, when one
*       of those input pointers is NULL.
*
*/
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN GT_U32                                   entryNum,
    IN CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC    *vlanCfgPtr,
    IN CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC     *qosCfgPtr
);

/**
* @internal cpssDxChBrgVlanPortProtoVlanQosGet function
* @endinternal
*
* @brief   Get Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3 0..7;
*                                      xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @param[out] vlanCfgPtr               - pointer to VID parameters (can be NULL)
* @param[out] qosCfgPtr                - pointer to QoS parameters (can be NULL)
* @param[out] validEntryPtr            - pointer to validity bit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is possible to get only one of VLAN or QOS parameters by receiving
*       one of those pointers as NULL pointer or get only validity bit.
*
*/
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN GT_U32                                   entryNum,
    OUT CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC   *vlanCfgPtr,
    OUT CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC    *qosCfgPtr,
    OUT GT_BOOL                                 *validEntryPtr
);

/**
* @internal cpssDxChBrgVlanPortProtoVlanQosInvalidate function
* @endinternal
*
* @brief   Invalidate protocol based QoS and VLAN assignment entry for
*         specific port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3 0..7;
*                                      xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosInvalidate
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   entryNum
);
/**
* @internal cpssDxChBrgVlanLearningStateSet function
* @endinternal
*
* @brief   Sets state of VLAN based learning to specified VLAN on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan Id
* @param[in] status                   - GT_TRUE for enable  or GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanLearningStateSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_BOOL  status
);


/**
* @internal cpssDxChBrgVlanIsDevMember function
* @endinternal
*
* @brief   Checks if specified device has members belong to specified vlan
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
*
* @retval GT_TRUE                  - if port is member of vlan
* @retval GT_FALSE                 - if port does not member of vlan or other status error
*/
GT_BOOL cpssDxChBrgVlanIsDevMember
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId
);

/**
* @internal cpssDxChBrgVlanToStpIdBind function
* @endinternal
*
* @brief   Bind VLAN to STP Id.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Id
* @param[in] vlanId                   - vlan Id
* @param[in] stpId                    - STP Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_OUT_OF_RANGE          - out of range stpId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanToStpIdBind
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_U16   stpId
);

/**
* @internal cpssDxChBrgVlanStpIdGet function
* @endinternal
*
* @brief   Read STP Id that bind to specified VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Id
* @param[in] vlanId                   - vlan Id
*
* @param[out] stpIdPtr                 - pointer to STP Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanStpIdGet
(
    IN  GT_U8    devNum,
    IN  GT_U16   vlanId,
    OUT GT_U16   *stpIdPtr
);

/**
* @internal cpssDxChBrgVlanEtherTypeSet function
* @endinternal
*
* @brief   Sets vlan Ether Type mode and vlan range.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] etherType                - mode of tagged vlan etherType. May be either
*                                      CPSS_VLAN_ETHERTYPE0_E or CPSS_VLAN_ETHERTYPE1_E.
*                                      See comment.
* @param[in] etherType                - the EtherType to be recognized tagged packets. See
*                                      comment.
* @param[in] vidRange                 - the maximum vlan range to accept for the specified
*                                      etheType.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal vidRange value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the vid of the incoming packet is greater or equal (>=) than the vlan
*       range value, the packet is filtered.
*       Configuration of vid and range for CPSS_VLAN_ETHERTYPE1_E should be done
*       only when nested vlan is used. User can alter the etherType field of
*       the encapsulating vlan by setting a number other then 0x8100 in the
*       parameter <ethertype>.
*
*/
GT_STATUS cpssDxChBrgVlanEtherTypeSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  etherType,
    IN  GT_U16                  vidRange
);

/**
* @internal cpssDxChBrgVlanEtherTypeGet function
* @endinternal
*
* @brief   Gets vlan Ether Type mode and vlan range.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] etherTypeMode            - mode of tagged vlan etherType. May be either
*                                      CPSS_VLAN_ETHERTYPE0_E or CPSS_VLAN_ETHERTYPE1_E.
*                                      See comment.
*
* @param[out] etherTypePtr             - (pointer to)the EtherType to be recognized tagged
*                                      packets. See comment.
* @param[out] vidRangePtr              - (pointer to)the maximum vlan range to accept for
*                                      the specified etheType.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal vidRange value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanEtherTypeGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    OUT  GT_U16                 *etherTypePtr,
    OUT  GT_U16                 *vidRangePtr
);

/**
* @internal cpssDxChBrgVlanTableInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN Table entries by writing 0 to the
*         first word.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTableInvalidate
(
    IN GT_U8 devNum
);

/**
* @internal cpssDxChBrgVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEntryInvalidate
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId
);

/**
* @internal cpssDxChBrgVlanUnkUnregFilterSet function
* @endinternal
*
* @brief   Set per VLAN filtering command for specified Unknown or Unregistered
*         packet type
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN ID
* @param[in] packetType               - packet type
* @param[in] cmd                      - command for the specified packet type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUnkUnregFilterSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT   packetType,
    IN CPSS_PACKET_CMD_ENT                  cmd

);

/**
* @internal cpssDxChBrgVlanForcePvidEnable function
* @endinternal
*
* @brief   Set Port VID Assignment mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE  - PVID is assigned to all packets.
*                                      GT_FALSE - PVID is assigned to untagged or priority tagged
*                                      packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForcePvidEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgVlanForcePvidEnableGet function
* @endinternal
*
* @brief   Get Port VID Assignment mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE  - PVID is assigned to all packets.
*                                      GT_FALSE - PVID is assigned to untagged or priority tagged
*                                      packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForcePvidEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortVidPrecedenceSet function
* @endinternal
*
* @brief   Set Port VID Precedence.
*         Relevant for packets with assigned VLAN ID of Pvid.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] precedence               -  type - soft or hard
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVidPrecedenceSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence
);

/**
* @internal cpssDxChBrgVlanPortVidPrecedenceGet function
* @endinternal
*
* @brief   Get Port VID Precedence.
*         Relevant for packets with assigned VLAN ID of Pvid.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] precedencePtr            - (pointer to) precedence type - soft or hard
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanPortVidPrecedenceGet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    OUT CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  *precedencePtr
);

/**
* @internal cpssDxChBrgVlanIpUcRouteEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 Unicast Routing on Vlan
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
* @param[in] protocol                 - ipv4 or ipv6
* @param[in] enable                   - GT_TRUE -  ip unicast routing;
*                                      GT_FALSE -  disable ip unicast routing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpUcRouteEnable
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    IN GT_BOOL                      enable
);

/**
* @internal cpssDxChBrgVlanIpMcRouteEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 Multicast Routing on Vlan
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
* @param[in] protocol                 - ipv4 or ipv6
* @param[in] enable                   - GT_TRUE -  ip multicast routing;
*                                      GT_FALSE -  disable ip multicast routing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpMcRouteEnable
(
    IN GT_U8                            devNum,
    IN GT_U16                           vlanId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN GT_BOOL                          enable
);

/**
* @internal cpssDxChBrgVlanNASecurEnable function
* @endinternal
*
* @brief   This function enables/disables per VLAN the generation of
*         security breach event for packets with unknown Source MAC addresses.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] enable                   - GT_TRUE -  generation of security breach event
*                                      GT_FALSE- disable generation of security breach event
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanNASecurEnable
(
    IN GT_U8           devNum,
    IN GT_U16          vlanId,
    IN GT_BOOL         enable
);

/**
* @internal cpssDxChBrgVlanIgmpSnoopingEnable function
* @endinternal
*
* @brief   Enable/Disable IGMP trapping or mirroring to the CPU according to the
*         global setting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - the VLAN-ID in which IGMP trapping is enabled/disabled.
* @param[in] enable                   - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIgmpSnoopingEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChBrgVlanIgmpSnoopingEnableGet function
* @endinternal
*
* @brief   Gets the status of IGMP trapping or mirroring to the CPU according to the
*         global setting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - the VLAN-ID in which IGMP trapping is enabled/disabled.
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIgmpSnoopingEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgVlanIpCntlToCpuSet function
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
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] ipCntrlType              - IP control type
*                                      DXCH devices support : CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*                                      DXCH2 and above devices support: like DXCH device ,and also
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpCntlToCpuSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT       ipCntrlType
);

/**
* @internal cpssDxChBrgVlanIpCntlToCpuGet function
* @endinternal
*
* @brief   Gets IP control traffic trapping/mirroring to CPU status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] ipCntrlTypePtr           - IP control type
*                                      DXCH devices support : CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*                                      DXCH2 and above devices support: like DXCH device ,and also
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpCntlToCpuGet
(
    IN  GT_U8                               devNum,
    IN  GT_U16                              vlanId,
    OUT CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      *ipCntrlTypePtr
);

/**
* @internal cpssDxChBrgVlanIpV6IcmpToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable ICMPv6 trapping or mirroring to
*         the CPU, according to global ICMPv6 message type
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] enable                   - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpV6IcmpToCpuEnable
(
    IN GT_U8      devNum,
    IN GT_U16     vlanId,
    IN GT_BOOL    enable
);

/**
* @internal cpssDxChBrgVlanIpV6IcmpToCpuEnableGet function
* @endinternal
*
* @brief   Gets status of ICMPv6 trapping or mirroring to
*         the CPU, according to global ICMPv6 message type
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpV6IcmpToCpuEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U16     vlanId,
    OUT GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChBrgVlanUdpBcPktsToCpuEnable function
* @endinternal
*
* @brief   Enable or disable per Vlan the Broadcast UDP packets Mirror/Trap to the
*         CPU based on their destination UDP port. Destination UDP port, command
*         (trap or mirror) and CPU code configured by
*         cpssDxChBrgVlanUdpBcPktsToCpuEnable
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   -  vlan id
* @param[in] enable                   - Enable/disable UDP Trap Mirror.
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUdpBcPktsToCpuEnable
(
    IN GT_U8        devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChBrgVlanUdpBcPktsToCpuEnableGet function
* @endinternal
*
* @brief   Gets per Vlan if the Broadcast UDP packets are Mirror/Trap to the
*         CPU based on their destination UDP port.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   -  vlan id
*
* @param[out] enablePtr                - UDP Trap Mirror status
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUdpBcPktsToCpuEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgVlanIpv6SourceSiteIdSet function
* @endinternal
*
* @brief   Sets a vlan ipv6 site id
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - the vlan.
* @param[in] siteId                   - the site id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChBrgVlanIpv6SourceSiteIdSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_IP_SITE_ID_ENT                  siteId
);

/**
* @internal cpssDxChBrgVlanIpmBridgingEnable function
* @endinternal
*
* @brief   IPv4/Ipv6 Multicast Bridging Enable
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] enable                   - GT_TRUE - IPv4 multicast packets are bridged
*                                      according to ipv4IpmBrgMode
*                                      GT_FALSE - IPv4 multicast packets are bridged
*                                      according to MAC DA
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
*       'BrgEnable' - set by this API (per VLAN, per IP protocol).
*       'BrgMode' - set by cpssDxChBrgVlanIpmBridgingModeSet (per VLAN, per IP protocol).
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
GT_STATUS cpssDxChBrgVlanIpmBridgingEnable
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  GT_BOOL                     enable
);

/**
* @internal cpssDxChBrgVlanIpmBridgingModeSet function
* @endinternal
*
* @brief   Sets the IPM bridging mode of Vlan. Relevant when IPM Bridging enabled.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
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
*       'BrgEnable' - set by cpssDxChBrgVlanIpmBridgingEnable (per VLAN, per IP protocol).
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
GT_STATUS cpssDxChBrgVlanIpmBridgingModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  CPSS_BRG_IPM_MODE_ENT       ipmMode
);

/**
* @internal cpssDxChBrgVlanIngressMirrorEnable function
* @endinternal
*
* @brief   Enable/Disable ingress mirroring of packets to the Ingress Analyzer
*         port for packets assigned the given VLAN-ID.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - Ingress mirrored traffic assigned to
*                                      this Vlan to the analyzer port
*                                      GT_FALSE - Don't ingress mirrored traffic assigned
*                                      to this Vlan to the analyzer port
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIngressMirrorEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
);

/**
* @internal cpssDxChBrgVlanPortAccFrameTypeSet function
* @endinternal
*
* @brief   Set port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
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
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortAccFrameTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType
);

/**
* @internal cpssDxChBrgVlanPortAccFrameTypeGet function
* @endinternal
*
* @brief   Get port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] frameTypePtr             -
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
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortAccFrameTypeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     *frameTypePtr
);

/**
* @internal cpssDxChBrgVlanMruProfileIdxSet function
* @endinternal
*
* @brief   Set Maximum Receive Unit MRU profile index for a VLAN.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         Value of MRU for profile is set by cpssDxChBrgVlanMruProfileValueSet.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan id
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMruProfileIdxSet
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
);

/**
* @internal cpssDxChBrgVlanMruProfileValueSet function
* @endinternal
*
* @brief   Set MRU value for a VLAN MRU profile.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         cpssDxChBrgVlanMruProfileIdxSet set index of profile for a specific VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
* @param[in] mruValue                 - MRU value in bytes
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..0xFFFF;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X  0..0x3FFF)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMruProfileValueSet
(
    IN GT_U8     devNum,
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
);

/**
* @internal cpssDxChBrgVlanMruProfileValueGet function
* @endinternal
*
* @brief   Get MRU value for a VLAN MRU profile.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
*
* @param[out] mruValuePtr              - (pointer to) MRU value in bytes
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanMruProfileValueGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    mruIndex,
    OUT GT_U32    *mruValuePtr
);

/**
* @internal cpssDxChBrgVlanNaToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable New Address (NA) Message Sending to CPU per VLAN .
*         To send NA to CPU both VLAN and port must be set to send NA to CPU.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan id
* @param[in] enable                   - GT_TRUE  -  New Address Message Sending to CPU
*                                      GT_FALSE - disable New Address Message Sending to CPU
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanNaToCpuEnable
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_BOOL   enable
);

/**
* @internal cpssDxChBrgVlanVrfIdSet function
* @endinternal
*
* @brief   Sets vlan Virtual Router ID
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - the vlan id.
* @param[in] vrfId                    - the virtual router id (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when vrfId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChBrgVlanVrfIdSet
(
    IN GT_U8                   devNum,
    IN GT_U16                  vlanId,
    IN GT_U32                  vrfId
);

/**
* @internal cpssDxChBrgVlanBridgingModeSet function
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
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] brgMode                  - bridge mode: IEEE 802.1Q bridge or IEEE 802.1D bridge
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBridgingModeSet
(
    IN GT_U8                devNum,
    IN CPSS_BRG_MODE_ENT    brgMode
);

/**
* @internal cpssDxChBrgVlanBridgingModeGet function
* @endinternal
*
* @brief   Get bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
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
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] brgModePtr               - (pointer to) bridge mode: IEEE 802.1Q bridge or IEEE 802.1D bridge
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanBridgingModeGet
(
    IN  GT_U8                devNum,
    OUT CPSS_BRG_MODE_ENT    *brgModePtr
);

/**
* @internal cpssDxChBrgVlanPortTranslationEnableSet function
* @endinternal
*
* @brief   Enable/Disable Vlan Translation per ingress or egress port.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - port number or CPU port number for egress direction.
*                                      In eArch devices portNum is default ePort for ingress direction.
* @param[in] direction                - ingress or egress
* @param[in] enable                   - enable/disable ingress/egress Vlan Translation.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortTranslationEnableSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_NUM                        portNum,
    IN CPSS_DIRECTION_ENT                 direction,
    IN CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT enable
);

/**
* @internal cpssDxChBrgVlanPortTranslationEnableGet function
* @endinternal
*
* @brief   Get the status of Vlan Translation (Enable/Disable) per ingress or
*         egress port.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - the device number
* @param[in] direction                - ingress or egress
* @param[in] portNum                  - port number or CPU port number for egress direction.
*                                      In eArch devices portNum is default ePort for ingress direction.
*
* @param[out] enablePtr                - (pointer to) ingress/egress Vlan Translation status
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortTranslationEnableGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  CPSS_DIRECTION_ENT                 direction,
    OUT CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT *enablePtr
);

/**
* @internal cpssDxChBrgVlanTranslationEntryWrite function
* @endinternal
*
* @brief   Write an entry into Vlan Translation Table Ingress or Egress
*         This mapping avoids collisions between a VLAN-ID
*         used for a logical interface and a VLAN-ID used by the local network.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table
*                                      (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..8191)
* @param[in] direction                - ingress or egress
* @param[in] transVlanId              - Translated Vlan ID, use as value in the
*                                      Translation Table (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..8191)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when transVlanId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTranslationEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U16                       transVlanId
);

/**
* @internal cpssDxChBrgVlanTranslationEntryRead function
* @endinternal
*
* @brief   Read an entry from Vlan Translation Table, Ingress or Egress.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table, 0..4095.
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..8191)
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
GT_STATUS cpssDxChBrgVlanTranslationEntryRead
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    OUT GT_U16                      *transVlanIdPtr
);

/**
* @internal cpssDxChBrgVlanValidCheckEnableSet function
* @endinternal
*
* @brief   Enable/disable check of "Valid" field in the VLAN entry
*         When check enable and VLAN entry is not valid then packets are dropped.
*         When check disable and VLAN entry is not valid then packets are
*         not dropped and processed like in case of valid VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  - Enable check of "Valid" field in the VLAN entry
*                                      GT_FALSE - Disable check of "Valid" field in the VLAN entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanValidCheckEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgVlanValidCheckEnableGet function
* @endinternal
*
* @brief   Get check status of "Valid" field in the VLAN entry
*         When check enable and VLAN entry is not valid then packets are dropped.
*         When check disable and VLAN entry is not valid then packets are
*         not dropped and processed like in case of valid VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - pointer to check status  of "Valid" field in the VLAN entry.
*                                      - GT_TRUE  - Enable check of "Valid" field in the VLAN entry
*                                      GT_FALSE - Disable check of "Valid" field in the VLAN entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - failed to read from hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanValidCheckEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChBrgVlanTpidEntrySet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] tableType                - etherType table selection
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..7)
* @param[in] etherType                - Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex, tableType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E, TPID0 use entry indexes (0,2,4,6)
*       CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E, TPID1 use entry indexes (1,3,5,7)
*       Egress Logical Port TPID table each entry defines a Tag0 TPID and Tag1 TPID;
*       4 entries of (16b Tag0 TPID, 16b Tag1 TPID))
*
*/
GT_STATUS cpssDxChBrgVlanTpidEntrySet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType,
    IN  GT_U32                          entryIndex,
    IN  GT_U16                          etherType
);

/**
* @internal cpssDxChBrgVlanTpidEntryGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] tableType                - etherType table selection
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..7)
*
* @param[out] etherTypePtr             - (pointer to) Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex, tableType
* @retval GT_BAD_PTR               - etherTypePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTpidEntryGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType,
    IN  GT_U32                          entryIndex,
    OUT GT_U16                         *etherTypePtr
);

/**
* @internal cpssDxChBrgVlanIngressTpidProfileSet function
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
GT_STATUS cpssDxChBrgVlanIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
);

/**
* @internal cpssDxChBrgVlanIngressTpidProfileGet function
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
GT_STATUS cpssDxChBrgVlanIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
);

/**
* @internal cpssDxChBrgVlanTpidTagTypeSet function
* @endinternal
*
* @brief   Set TPID tag type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] index                    - TPID  (APPLICABLE RANGES: 0..7)
* @param[in] type                     - the TPID type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTpidTagTypeSet
(
    IN GT_U8                        devNum,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U32                       index,
    IN CPSS_BRG_TPID_SIZE_TYPE_ENT  type
);

/**
* @internal cpssDxChBrgVlanTpidTagTypeGet function
* @endinternal
*
* @brief   Get TPID tag type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] index                    - TPID  (APPLICABLE RANGES: 0..7)
*
* @param[out] typePtr                  - (pointer to) the TPID type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTpidTagTypeGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DIRECTION_ENT          direction,
    IN  GT_U32                      index,
    OUT CPSS_BRG_TPID_SIZE_TYPE_ENT *typePtr
);

/**
* @internal cpssDxChBrgVlanPortIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
*                                      In eArch devices portNum is default ePort for default profile.
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
GT_STATUS cpssDxChBrgVlanPortIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    IN  GT_U32               profile
);

/**
* @internal cpssDxChBrgVlanPortIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
*                                      In eArch devices portNum is default ePort for default profile.
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
GT_STATUS cpssDxChBrgVlanPortIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    OUT GT_U32               *profilePtr
);

/**
* @internal cpssDxChBrgVlanPortIngressTpidSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - etherType mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E
* @param[in] tpidBmp                  - bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*
*/
GT_STATUS cpssDxChBrgVlanPortIngressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
);

/**
* @internal cpssDxChBrgVlanPortIngressTpidGet function
* @endinternal
*
* @brief   Function gets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - etherType mode selector. valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidBmpPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*
*/
GT_STATUS cpssDxChBrgVlanPortIngressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
);

/**
* @internal cpssDxChBrgVlanPortEgressTpidSet function
* @endinternal
*
* @brief   Function sets index of TPID (Tag protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - ethertype mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E
* @param[in] tpidEntryIndex           - TPID table entry index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortEgressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    IN  GT_U32               tpidEntryIndex
);

/**
* @internal cpssDxChBrgVlanPortEgressTpidGet function
* @endinternal
*
* @brief   Function gets index of TPID (Tag Protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - ethertype mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID table entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortEgressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    OUT GT_U32               *tpidEntryIndexPtr
);

/**
* @internal cpssDxChBrgVlanEgressTagTpidSelectModeSet function
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
GT_STATUS cpssDxChBrgVlanEgressTagTpidSelectModeSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_ETHER_MODE_ENT                          ethMode,
    IN CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT  mode
);

/**
* @internal cpssDxChBrgVlanEgressTagTpidSelectModeGet function
* @endinternal
*
* @brief   Returns Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] modePtr                  - (pointer to)
*                                      0: TPID of Tag0/1 is selected according to
*                                      <Egress Tag 0/1 TPID select>
*                                      1: TPID of Tag0/1 is selected according to
*                                      VID0/1 to TPID select table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEgressTagTpidSelectModeGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    IN  CPSS_ETHER_MODE_ENT                                 ethMode,
    OUT CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT         *modePtr
);


/**
* @internal cpssDxChBrgVlanRangeSet function
* @endinternal
*
* @brief   Function configures the valid VLAN Range.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] vidRange                 - VID range for VLAN filtering (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - illegal vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanRangeSet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vidRange
);

/**
* @internal cpssDxChBrgVlanRangeGet function
* @endinternal
*
* @brief   Function gets the valid VLAN Range.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @param[out] vidRangePtr              - (pointer to) VID range for VLAN filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - vidRangePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanRangeGet
(
    IN  GT_U8   devNum,
    OUT GT_U16  *vidRangePtr
);

/**
* @internal cpssDxChBrgVlanPortIsolationCmdSet function
* @endinternal
*
* @brief   Function sets port isolation command per egress VLAN.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] cmd                      - port isolation command for given VID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or cmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortIsolationCmdSet
(
    IN  GT_U8                                      devNum,
    IN  GT_U16                                     vlanId,
    IN  CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgVlanLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Function sets local switching of Multicast, known and unknown Unicast,
*         and Broadcast traffic per VLAN.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] trafficType              - local switching traffic type
* @param[in] enable                   - Enable/disable of local switching
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To enable local switching of Multicast, unknown Unicast, and
*       Broadcast traffic, both this field in the VLAN entry and the
*       egress port configuration must be enabled for Multicast local switching
*       in function cpssDxChBrgPortEgressMcastLocalEnable.
*       2. To enable local switching of known Unicast traffic, both this
*       field in the VLAN entry and the ingress port configuration must
*       be enabled for Unicast local switching in function
*       cpssDxChBrgGenUcLocalSwitchingEnable.
*
*/
GT_STATUS cpssDxChBrgVlanLocalSwitchingEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    IN  CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT  trafficType,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChBrgVlanMirrorToTxAnalyzerSet function
* @endinternal
*
* @brief   Function sets Tx Analyzer port index to given VLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChBrgVlanMirrorToTxAnalyzerSet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable,
    IN  GT_U32      txAnalyzerIndex
);

/**
* @internal cpssDxChBrgVlanMirrorToTxAnalyzerModeSet function
* @endinternal
*
* @brief   Function sets Egress Mirroring mode to given VLAN.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] egressMirroringMode      - one of 4 options of the enum mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or egressMirroringMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMirrorToTxAnalyzerModeSet
(
    IN  GT_U8                            devNum,
    IN  GT_U16                           vlanId,
    IN  CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode
);

/**
* @internal cpssDxChBrgVlanFloodVidxModeSet function
* @endinternal
*
* @brief   Function sets Flood VIDX and Flood VIDX Mode per VLAN.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] floodVidx                - VIDX value (12bit) - VIDX value applied to Broadcast,
*                                      Unregistered Multicast and unknown Unicast packets,
*                                      depending on the VLAN entry field <Flood VIDX Mode>.
*                                      (APPLICABLE RANGES: xCat2 0..1023, 4095)
* @param[in] floodVidxMode            - Flood VIDX Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or floodVidxMode
* @retval GT_OUT_OF_RANGE          - illegal floodVidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanFloodVidxModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U16                                  vlanId,
    IN  GT_U16                                  floodVidx,
    IN  CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT  floodVidxMode
);

/**
* @internal cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet function
* @endinternal
*
* @brief   Set Tag1 removal mode from the egress port Tag State if Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Vlan Tag1 Removal  when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT     mode
);

/**
* @internal cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet function
* @endinternal
*
* @brief   Get Tag1 removal mode from the egress port Tag State when Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Vlan Tag1 Removal mode when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT     *modePtr
);

/**
* @internal cpssDxChBrgVlanForceNewDsaToCpuEnableSet function
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
* @param[in] devNum                   - device number.
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
GT_STATUS cpssDxChBrgVlanForceNewDsaToCpuEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChBrgVlanForceNewDsaToCpuEnableGet function
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
GT_STATUS cpssDxChBrgVlanForceNewDsaToCpuEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgVlanKeepVlan1EnableSet function
* @endinternal
*
* @brief   Enable/Disable keeping VLAN1 in the packet, for packets received with VLAN1,
*         even-though the tag-state of this egress-port is configured in the
*         VLAN-table to "untagged" or "VLAN0".
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanKeepVlan1EnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       up,
    IN  GT_BOOL                     enable
);

/**
* @internal cpssDxChBrgVlanKeepVlan1EnableGet function
* @endinternal
*
* @brief   Get status of keeping VLAN1 in the packet, for packets received with VLAN1.
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority (APPLICABLE RANGES: 0..7)
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
GT_STATUS cpssDxChBrgVlanKeepVlan1EnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       up,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet function
* @endinternal
*
* @brief   Enable Learning Of Original Tag1 VID Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  -  learning
*                                      GT_FALSE - disable learning
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet function
* @endinternal
*
* @brief   Get Learning mode Of Original Tag1 VID Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable learning
*                                      GT_FALSE - disable learning
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);


/**
* @internal cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet function
* @endinternal
*
* @brief   When enabled, IPv4 MC/BC packets in this eVLAN are mirrored to the
*         analyzer specified in this field. This is independent of the analyzer
*         configuration of other traffic in this eVLAN.
*         0x0 is a reserved value that means Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - IPv4 MC/BC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv4 MC/BC packets in this eVLAN are NOT mirrored
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
);

/**
* @internal cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet function
* @endinternal
*
* @brief   Get Ipv4 MC/BC Mirror To Analyzer Index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - IPv4 MC/BC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv4 MC/BC packets in this eVLAN are NOT mirrored
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
);


/**
* @internal cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet function
* @endinternal
*
* @brief   When enabled, IPv6 MC packets in this eVLAN are mirrored to the
*         analyzer. This is independent of the analyzer configuration of other
*         traffic in this eVLAN.
*         0x0 is a reserved value that means Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - IPv6 MC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv6 MC packets in this eVLAN are NOT mirrored to the
*                                      analyzer specified in this field
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
);

/**
* @internal cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet function
* @endinternal
*
* @brief   Get Ipv6 MC Mirror To Analyzer Index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - IPv6 MC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv6 MC packets in this eVLAN are NOT mirrored to the
*                                      analyzer specified in this field
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
);

/**
* @internal cpssDxChBrgVlanForwardingIdSet function
* @endinternal
*
* @brief   Set Forwarding ID value.
*         Used for virtual birdges per eVLAN.
*         Used by the bridge engine for entry lookup and entry match (replaces VID
*         in bridge entry)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] fidValue                 - Forwarding ID value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForwardingIdSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_U32  fidValue
);

/**
* @internal cpssDxChBrgVlanForwardingIdGet function
* @endinternal
*
* @brief   Get Forwarding ID value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] fidValuePtr              - (pointer to) Forwarding ID value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForwardingIdGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_U32  *fidValuePtr
);


/**
* @internal cpssDxChBrgVlanUnknownMacSaCommandSet function
* @endinternal
*
* @brief   Set bridge forwarding decision for packets with unknown Source Address.
*         Only relevant in controlled address learning mode, when <Auto-learning
*         enable> = Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUnknownMacSaCommandSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN CPSS_PACKET_CMD_ENT  cmd
);

/**
* @internal cpssDxChBrgVlanUnknownMacSaCommandGet function
* @endinternal
*
* @brief   Get bridge forwarding decision for packets with unknown Source Address.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] cmdPtr                   - (pointer to) supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUnknownMacSaCommandGet
(
    IN  GT_U8                devNum,
    IN  GT_U16               vlanId,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
);


/**
* @internal cpssDxChBrgVlanEgressPortTagStateModeSet function
* @endinternal
*
* @brief   Set the egress port tag state mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChBrgVlanEgressPortTagStateModeSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT      stateMode
);

/**
* @internal cpssDxChBrgVlanEgressPortTagStateModeGet function
* @endinternal
*
* @brief   Get the egress port tag state mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChBrgVlanEgressPortTagStateModeGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT         *stateModePtr
);

/**
* @internal cpssDxChBrgVlanEgressPortTagStateSet function
* @endinternal
*
* @brief   Determines the egress port state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChBrgVlanEgressPortTagStateSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT              tagCmd
);

/**
* @internal cpssDxChBrgVlanEgressPortTagStateGet function
* @endinternal
*
* @brief   Returns the egress port state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChBrgVlanEgressPortTagStateGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT                 *tagCmdPtr
);

/**
* @internal cpssDxChBrgVlanPortPushVlanCommandSet function
* @endinternal
*
* @brief   Set Push Vlan Command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanCmd                  - vlan command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssDxChBrgVlanPortPushVlanCommandSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT    vlanCmd
);

/**
* @internal cpssDxChBrgVlanPortPushVlanCommandGet function
* @endinternal
*
* @brief   Get Push Vlan Command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vlanCmdPtr               - (pointer to)  push vlan command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushVlanCommandGet
(
    IN  GT_U8                                      devNum,
    IN  GT_PORT_NUM                                portNum,
    OUT CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT   *vlanCmdPtr
);

/**
* @internal cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet function
* @endinternal
*
* @brief   Set TPID select table index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tpidEntryIndex           - TPID select table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           tpidEntryIndex
);

/**
* @internal cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet function
* @endinternal
*
* @brief   Get TPID select table index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID select table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *tpidEntryIndexPtr
);

/**
* @internal cpssDxChBrgVlanPortPushedTagValueSet function
* @endinternal
*
* @brief   Set the tag value to push
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tagValue                 - tag value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagValueSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U16           tagValue
);

/**
* @internal cpssDxChBrgVlanPortPushedTagValueGet function
* @endinternal
*
* @brief   Get the tag value to push
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tagValuePtr              - (pointer to) tag value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagValueGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *tagValuePtr
);


/**
* @internal cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet function
* @endinternal
*
* @brief   Determines if UP and CFI values will be assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Enable/Disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet function
* @endinternal
*
* @brief   Get if UP and CFI values will be assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) Enable/Disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortUpSet function
* @endinternal
*
* @brief   Set UP value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - UP value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUpSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up
);

/**
* @internal cpssDxChBrgVlanPortUpGet function
* @endinternal
*
* @brief   Get UP value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] upPtr                    - (pointer to)  UP value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUpGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *upPtr
);

/**
* @internal cpssDxChBrgVlanPortCfiEnableSet function
* @endinternal
*
* @brief   Set CFI value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - CFI value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortCfiEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanPortCfiEnableGet function
* @endinternal
*
* @brief   Get CFI value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) CFI value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortCfiEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortNumOfTagWordsToPopSet function
* @endinternal
*
* @brief   Set the number of bytes to pop for traffic that ingress from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] numBytesToPop            - enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortNumOfTagWordsToPopSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT numBytesToPop
);

/**
* @internal cpssDxChBrgVlanPortNumOfTagWordsToPopGet function
* @endinternal
*
* @brief   Get the number of bytes to pop for traffic that ingress from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] numBytesToPopPtr         - (pointer to) enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on not supported value read from the HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortNumOfTagWordsToPopGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT  GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT *numBytesToPopPtr
);

/**
* @internal cpssDxChBrgVlanPortVid0CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing VID0 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet VID0 is not changed.
*                                      GT_TRUE:  Assign new VID0 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid0CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanPortVid0CommandEnableGet function
* @endinternal
*
* @brief   Return if changing VID0 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet VID0 is not changed.
*                                      GT_TRUE:  Assign new VID0 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid0CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortVid1CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing VID1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet VID1 is not changed.
*                                      GT_TRUE:  Assign new VID1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid1CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanPortVid1CommandEnableGet function
* @endinternal
*
* @brief   Return if changing VID1 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet VID1 is not changed.
*                                      GT_TRUE:  Assign new VID1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid1CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortUp0CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing UP1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp0CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanPortUp0CommandEnableGet function
* @endinternal
*
* @brief   Return if changing UP0 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp0CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortUp1CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing UP1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp1CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanPortUp1CommandEnableGet function
* @endinternal
*
* @brief   Return if changing UP1 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp1CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgVlanPortVid1Set function
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
*                                      In eArch devices portNum is default ePort for ingress direction.
* @param[in] direction                - ingress/egress direction
* @param[in] vid1                     - VID1 value (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid1Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN CPSS_DIRECTION_ENT          direction,
    IN  GT_U16           vid1
);

/**
* @internal cpssDxChBrgVlanPortVid1Get function
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
*                                      In eArch devices portNum is default ePort for ingress direction.
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
GT_STATUS cpssDxChBrgVlanPortVid1Get
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN CPSS_DIRECTION_ENT          direction,
    OUT  GT_U16           *vid1Ptr
);

/**
* @internal cpssDxChBrgVlanPortUp0Set function
* @endinternal
*
* @brief   Set Up0 assigned to the egress packet if <UP0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up0                      - UP0 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp0Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up0
);

/**
* @internal cpssDxChBrgVlanPortUp0Get function
* @endinternal
*
* @brief   Return the UP0 assigned to the egress packet if <UP0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] up0Ptr                   - (pointer to) UP0 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *up0Ptr
);

/**
* @internal cpssDxChBrgVlanPortUp1Set function
* @endinternal
*
* @brief   Set Up1 assigned to the egress packet if <UP1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up1                      - UP1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp1Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up1
);

/**
* @internal cpssDxChBrgVlanPortUp1Get function
* @endinternal
*
* @brief   Return the UP1 assigned to the egress packet if <UP1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] up1Ptr                   - (pointer to) UP1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp1Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *up1Ptr
);

/**
* @internal cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN Tag1 removing from packets arrived without VLAN Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - according to global configuration
*                                      see cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
*                                      GT_TRUE  - Tag1 removed if original packet arrived without Tag1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of VLAN Tag1 removing from packets arrived without VLAN Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - according to global configuration
*                                      see cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
*                                      GT_TRUE  - Tag1 removed if original packet arrived without Tag1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal cpssDxChBrgVlanIndependentNonFloodVidxEnableSet function
* @endinternal
*
* @brief   Enable/Disable Independent non-flood VIDX
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: VIDX portlist is always AND'ed with VID portlist
*                                      GT_TRUE:  If packet has VIDX target and VIDX != 0xFFF
*                                      (i.e. not flooded to all ports in the VLAN),
*                                      then the VIDX portlist is the distribution portlist,
*                                      regardless of the VID portlist.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIndependentNonFloodVidxEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanIndependentNonFloodVidxEnableGet function
* @endinternal
*
* @brief   Get Independent non-flood VIDX status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: VIDX portlist is always AND'ed with VID portlist
*                                      GT_TRUE:  If packet has VIDX target and VIDX != 0xFFF
*                                      (i.e. not flooded to all ports in the VLAN),
*                                      then the VIDX portlist is the distribution portlist,
*                                      regardless of the VID portlist.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIndependentNonFloodVidxEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgVlanFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Set FCoE Forwarding Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which FCoE Forwarding enabled/disabled..
* @param[in] enable                   - FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanFcoeForwardingEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_U16           vlanId,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgVlanFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return FCoE Forwarding Enable status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which FCoE Forwarding enabled/disabled..
*
* @param[out] enablePtr                - (pointer to) FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanFcoeForwardingEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_U16           vlanId,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal cpssDxChBrgVlanUnregisteredIpmEVidxSet function
* @endinternal
*
* @brief   Set Unregistered IPM eVidx Assignment Mode and Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID.
* @param[in] unregIpmEVidx            - Unregistered IPM eVidx Assignment Mode.
* @param[in] unregIpmEVidx            - Unregistered IPM eVidx Assignment Value.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUnregisteredIpmEVidxSet
(
    IN GT_U8                                        devNum,
    IN GT_U16                                       vlanId,
    IN CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT  unregIpmEVidxMode,
    IN GT_U32                                       unregIpmEVidx
);

/**
* @internal cpssDxChBrgVlanMembersTableIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMembersTableIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
);

/**
* @internal cpssDxChBrgVlanMembersTableIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChBrgVlanMembersTableIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChBrgVlanStgIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanStgIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
);

/**
* @internal cpssDxChBrgVlanStgIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChBrgVlanStgIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChBrgVlanTagStateIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port supported)
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTagStateIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
);

/**
* @internal cpssDxChBrgVlanTagStateIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port supported)
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTagStateIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet function
* @endinternal
*
* @brief   Set the BPE E-Tag Ingress_E-CID_base calculation algorithm
*         for Looped Back Forwarded Bridged Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] forceSrcCidDefault       - GT_TRUE - Ingress_E-CID_base field of E-TAG forced
*                                      to get default value configured by
*                                      cpssDxChBrgVlanBpeTagMcCfgSet (defaultSrcECid).
*                                      GT_FALSE - Ingress_E-CID_base field of E-TAG is pushed VID.
*                                      configured by cpssDxChBrgVlanPortPushedTagValueSet
*                                      (tagValue) per source ePort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For regular multicast packets E-Tag Ingress_E-CID_base is always S_VID.
*
*/
GT_STATUS cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          forceSrcCidDefault
);

/**
* @internal cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet function
* @endinternal
*
* @brief   Get the BPE E-Tag Ingress_E-CID_base calculation algorithm
*         for Looped Back Forwarded Bridged Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] forceSrcCidDefaultPtr    - (pointer to)
*                                      GT_TRUE - Ingress_E-CID_base field of E-TAG forced
*                                      to get default value configured by
*                                      cpssDxChBrgVlanBpeTagMcCfgSet (defaultSrcECid).
*                                      GT_FALSE - Ingress_E-CID_base field of E-TAG is pushed VID.
*                                      configured by cpssDxChBrgVlanPortPushedTagValueSet
*                                      (tagValue) per source ePort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For regular multicast packets E-Tag Ingress_E-CID_base is always S_VID.
*
*/
GT_STATUS cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *forceSrcCidDefaultPtr
);

/**
* @internal cpssDxChBrgVlanBpeTagMcCfgSet function
* @endinternal
*
* @brief   Set the BPE E-Tag Configuration for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] eCidOffset               - GT_TRUE - negative, GT_FALSE - positive
* @param[in] eCidOffset               - E-CID Offset negative or positive
*                                      for Multicast packets, field
*                                      {GRP,E-CID_base} calculated as (eVIDX + eCidOffset)
*                                      (APPLICABLE RANGES: 0..0x3FFF)
* @param[in] defaultSrcECid           - default value for Ingress_E-CID_base field of E-TAG  -
*                                      the value that not filtered by source VM filtering.
*                                      Used for all Unicast and some looped back Multicast packets.
*                                      About looped back Multicast packets see
*                                      cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - when one of parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBpeTagMcCfgSet
(
    IN  GT_U8             devNum,
    IN  GT_BOOL           eCidOffsetNegative,
    IN  GT_U32            eCidOffset,
    IN  GT_U32            defaultSrcECid
);

/**
* @internal cpssDxChBrgVlanBpeTagMcCfgGet function
* @endinternal
*
* @brief   Get the BPE E-Tag Configuration for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] eCidOffsetNegativePtr    - (pointer to)GT_TRUE - negative, GT_FALSE - positive
* @param[out] eCidOffsetPtr            - (pointer to)E-CID Offset negative or positive
*                                      for Multicast packets, field
*                                      {GRP,E-CID_base} calculated as (eVIDX + eCidOffset)
* @param[out] defaultSrcECidPtr        - default value for Ingress_E-CID_base field of E-TAG  -
*                                      the value that not filtered by source VM filtering.
*                                      Used for all Unicast and some looped back Multicast packets.
*                                      About looped back Multicast packets see
*                                      cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBpeTagMcCfgGet
(
    IN  GT_U8             devNum,
    OUT GT_BOOL           *eCidOffsetNegativePtr,
    OUT GT_U32            *eCidOffsetPtr,
    OUT GT_U32            *defaultSrcECidPtr
);

/**
* @internal cpssDxChBrgVlanBpeTagReservedFieldsSet function
* @endinternal
*
* @brief   Set the values for BPE E-Tag Reserved Fields of all packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] reValue                  - value for reserved field called "re-"
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] ingressECidExtValue      - value for field "ingress_E-CID_ext"
*                                      (APPLICABLE RANGES: 0..0xFF)
* @param[in] eCidExtValue             - value for field "E-CID_ext"
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - when one of parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBpeTagReservedFieldsSet
(
    IN  GT_U8             devNum,
    IN  GT_U32            reValue,
    IN  GT_U32            ingressECidExtValue,
    IN  GT_U32            eCidExtValue
);

/**
* @internal cpssDxChBrgVlanBpeTagReservedFieldsGet function
* @endinternal
*
* @brief   Get the values for BPE E-Tag Reserved Fields of all packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] reValuePtr               - (pointer to)value for reserved field called "re-"
* @param[out] ingressECidExtValuePtr   - (pointer to)value for field "ingress_E-CID_ext"
* @param[out] eCidExtValuePtr          - (pointer to)value for field "E-CID_ext"
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBpeTagReservedFieldsGet
(
    IN  GT_U8             devNum,
    OUT GT_U32            *reValuePtr,
    OUT GT_U32            *ingressECidExtValuePtr,
    OUT GT_U32            *eCidExtValuePtr
);

/**
* @internal cpssDxChBrgVlanFdbLookupKeyModeSet function
* @endinternal
*
* @brief   Set an FDB Lookup key mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] mode                     - FDB Lookup key mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanFdbLookupKeyModeSet
(
    IN GT_U8                                 devNum,
    IN GT_U16                                vlanId,
    IN CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT mode
);

/**
* @internal cpssDxChBrgVlan6BytesTagConfigSet function
* @endinternal
*
* @brief   Set global configurations for 6 bytes v-tag.
*         NOTE: not relevant to standard 8 bytes BPE 802.1BR.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] isPortExtender           - is this 'port Extender' or 'Control bridge'
*                                      GT_TRUE - 'port Extender'
*                                      GT_FALSE - 'Control bridge'
* @param[in] lBitInSrcId              - the bit index of 'L bit' in the srcId field passed from ingress pipe to egress pipe.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] dBitInSrcId              - the bit index of 'D bit' in the srcId field passed from ingress pipe to egress pipe.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter devNum
* @retval GT_OUT_OF_RANGE          - out of range in value of lBitInSrcId or dBitInSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlan6BytesTagConfigSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  isPortExtender,
    IN GT_U32   lBitInSrcId,
    IN GT_U32   dBitInSrcId
);

/**
* @internal cpssDxChBrgVlan6BytesTagConfigGet function
* @endinternal
*
* @brief   Get global configurations for 6 bytes v-tag.
*         NOTE: not relevant to standard 8 bytes BPE 802.1BR.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] isPortExtenderPtr        - (pointer to) is this 'port Extender' or 'Control bridge'
*                                      GT_TRUE - 'port Extender'
*                                      GT_FALSE - 'Control bridge'
* @param[out] lBitInSrcIdPtr           - (pointer to) the bit index of 'L bit' in the srcId field passed from ingress pipe to egress pipe.
* @param[out] dBitInSrcIdPtr           - (pointer to) the bit index of 'D bit' in the srcId field passed from ingress pipe to egress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlan6BytesTagConfigGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *isPortExtenderPtr,
    OUT GT_U32   *lBitInSrcIdPtr,
    OUT GT_U32   *dBitInSrcIdPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgVlanh */

