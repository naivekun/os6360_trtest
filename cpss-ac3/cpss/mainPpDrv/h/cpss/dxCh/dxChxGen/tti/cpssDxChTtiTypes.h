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
* @file cpssDxChTtiTypes.h
*
* @brief CPSS definitions for tunnel.
*
* @version   64
********************************************************************************
*/

#ifndef __cpssDxChTtiTypesh
#define __cpssDxChTtiTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/tunnel/cpssGenTunnelTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>

/* max number for UDB */
#define CPSS_DXCH_TTI_MAX_UDB_CNS   30

/**
* @enum CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT
 *
 * @brief TTI includes src interface fields in MAC2ME table type.
*/
typedef enum{

    /** @brief do not use the source interface
     *  fields as part of the MAC2ME lookup
     */
    CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E               = 0,

    /** @brief use the source interface fields
     *  as part of the MAC2ME lookup
     */
    CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E                    = 1,

    /** @brief use source
     *  interface fields as part of the MAC2ME lookup, exclude the source Device.
     */
    CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E = 2

} CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT;

/**
* @struct CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC
 *
 * @brief The MAC TO ME lookup is used to identify packets destined to:
 * 1. (MAC address, VLAN) if <includeSrcInterfaceType> = 0
 * 2. (MAC address, VLAN, SRC device, SRC is trunk, SRC trunk/port id)
 * if <includeSrcInterfaceType> = 1
 * 3. (MAC address, VLAN, SRC is trunk, SRC trunk/port id)
 * if <includeSrcInterfaceType> = 2.
 * This indication is used in subsequent pipeline units such as
 * the TTI lookup and Ingress Policy engine or for TRILL packets.
*/
typedef struct{

    CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT includeSrcInterface;

    /** local source HW device or DSA tag source HW device */
    GT_HW_DEV_NUM srcHwDevice;

    /** @brief whether source is port or trunk (APPLICABLE RANGES: 0..1)
     *  0: Source is not a trunk (i.e. it is port)
     *  1: Source is a trunk
     */
    GT_BOOL srcIsTrunk;

    /** @brief Source port or Source trunk
     *  if port, range (20 bits)
     *  if trunk, range (12 bits)
     */
    GT_U32 srcPortTrunk;

} CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC;

/**
* @struct CPSS_DXCH_TTI_MAC_VLAN_STC
 *
 * @brief Mac To Me and Vlan To Me.
*/
typedef struct{

    /** Mac to me address */
    GT_ETHERADDR mac;

    /** vlan */
    GT_U16 vlanId;

} CPSS_DXCH_TTI_MAC_VLAN_STC;

/**
* @struct CPSS_DXCH_TTI_TRILL_ADJACENCY_STC
 *
 * @brief The TRILL engine implements the following TRILL adjacency checks:
 * 1. TRILL IS-IS Adjacency check -
 * Checks that the single-destination TRILL frame arrives from a
 * (neighbor, port) for which an IS-IS adjacency exists.
 * 2. TRILL Tree Adjacency Check -
 * Checks that the multi-destination TRILL frame arrives from a
 * (neighbor, port) that is a branch on the given TRILL distribution tree.
*/
typedef struct{

    /** The outer MAC source address */
    GT_ETHERADDR outerMacSa;

    /** The TRILL header M */
    GT_U32 trillMBit;

    /** The TRILL header Egress RBridge Nickname (16 bits) */
    GT_U32 trillEgressRbid;

    /** local source HW device or DSA tag source HW device (10 bits) */
    GT_HW_DEV_NUM srcHwDevice;

    /** @brief whether source is port or trunk (APPLICABLE RANGES: 0..1)
     *  0: Source is not a trunk (i.e. it is port)
     *  1: Source is a trunk
     */
    GT_U32 srcIsTrunk;

    /** @brief Source port or Source trunk
     *  if port, range (15 bits)
     *  if trunk, range (12 bits)
     */
    GT_U32 srcPortTrunk;

} CPSS_DXCH_TTI_TRILL_ADJACENCY_STC;

/**
* @enum CPSS_DXCH_TTI_KEY_TYPE_ENT
 *
 * @brief TTI key type.
*/
typedef enum{

    /** IPv4 TTI key type */
    CPSS_DXCH_TTI_KEY_IPV4_E  = 0,

    /** MPLS TTI key type */
    CPSS_DXCH_TTI_KEY_MPLS_E  = 1,

    /** Ethernet TTI key type */
    CPSS_DXCH_TTI_KEY_ETH_E   = 2,

    /** Mac in Mac TTI key type (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_MIM_E   = 3,

    /** UDB IPv4 TCP key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E = 4,

    /** UDB IPv4 UDP key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E = 5,

    /** UDB MPLS key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_MPLS_E = 6,

    /** UDB IPv4 Fragment key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E = 7,

    /** UDB IPv4 key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E = 8,

    /** UDB Ethernet key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E = 9,

    /** UDB IPv6 key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_IPV6_E = 10,

    /** UDB IPv6 TCP key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E = 11,

    /** UDB IPv6 UDP key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E = 12,

    /** UDB UDE key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_UDE_E = 13,

    /** UDB UDE1 key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_UDE1_E = 14,

    /** UDB UDE2 key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_UDE2_E = 15,

    /** UDB UDE3 key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_UDE3_E = 16,

    /** UDB UDE4 key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_UDE4_E = 17,

    /** UDB UDE5 key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_UDE5_E = 18,

    /** UDB UDE6 key type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_KEY_UDB_UDE6_E = 19

} CPSS_DXCH_TTI_KEY_TYPE_ENT;

/**
* @enum CPSS_DXCH_TTI_RULE_TYPE_ENT
 *
 * @brief TTI rule type.
*/
typedef enum{

    /** IPv4 TTI rule size 30 bytes type */
    CPSS_DXCH_TTI_RULE_IPV4_E  = CPSS_DXCH_TTI_KEY_IPV4_E,

    /** MPLS TTI rule size 30 bytes type */
    CPSS_DXCH_TTI_RULE_MPLS_E  = CPSS_DXCH_TTI_KEY_MPLS_E,

    /** Ethernet TTI rule size 30 bytes type */
    CPSS_DXCH_TTI_RULE_ETH_E   = CPSS_DXCH_TTI_KEY_ETH_E,

    /** Mac in Mac TTI rule size 30 bytes type (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_RULE_MIM_E   = CPSS_DXCH_TTI_KEY_MIM_E,

    /** UDB rule size 10 bytes type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_RULE_UDB_10_E,

    /** UDB rule size 20 bytes type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_RULE_UDB_20_E,

    /** UDB rule size 30 bytes type (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_RULE_UDB_30_E

} CPSS_DXCH_TTI_RULE_TYPE_ENT;
/**
* @enum CPSS_DXCH_TTI_KEY_SIZE_ENT
 *
 * @brief TTI key size.
*/
typedef enum{

    /** TCAM key size to be used 10 Byets */
    CPSS_DXCH_TTI_KEY_SIZE_10_B_E  = 0,

    /** TCAM key size to be used 20 Byets */
    CPSS_DXCH_TTI_KEY_SIZE_20_B_E  = 1,

    /** TCAM key size to be used 30 Byets */
    CPSS_DXCH_TTI_KEY_SIZE_30_B_E  = 2

} CPSS_DXCH_TTI_KEY_SIZE_ENT;

/**
* @enum CPSS_DXCH_TTI_OFFSET_TYPE_ENT
 *
 * @brief Offset types for packet headers parsing used for user defined
 * bytes configuration
*/
typedef enum{

    /** offset from start of mac(L2) header */
    CPSS_DXCH_TTI_OFFSET_L2_E = 0,

    /** offset from start of L3 header minus 2. */
    CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E = 1,

    /** offset from start of L4 header */
    CPSS_DXCH_TTI_OFFSET_L4_E = 2,

    /** offset from Ethertype of MPLS (start of the MPLS payload minus 2) */
    CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E = 3,

    /** offset from TTI internal descriptor */
    CPSS_DXCH_TTI_OFFSET_METADATA_E = 4,

    /** invalid UDB - contains 0 always */
    CPSS_DXCH_TTI_OFFSET_INVALID_E = 5


} CPSS_DXCH_TTI_OFFSET_TYPE_ENT;


/**
* @enum CPSS_DXCH_TTI_MAC_MODE_ENT
 *
 * @brief Determines MAC that will be used to generate lookup TCAM key.
*/
typedef enum{

    /** use destination MAC */
    CPSS_DXCH_TTI_MAC_MODE_DA_E   = 0,

    /** use source MAC */
    CPSS_DXCH_TTI_MAC_MODE_SA_E   = 1

} CPSS_DXCH_TTI_MAC_MODE_ENT;

/**
* @struct CPSS_DXCH_TTI_TRILL_RBID_TABLE_ENTRY_STC
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

} CPSS_DXCH_TTI_TRILL_RBID_TABLE_ENTRY_STC;


/**
* @struct CPSS_DXCH_TTI_RULE_COMMON_STC
 *
 * @brief TTI Common fields in TCAM rule.
*/
typedef struct{

    /** PCL id */
    GT_U32 pclId;

    /** @brief whether source is port or trunk (APPLICABLE RANGES: 0..1)
     *  0: Source is not a trunk (i.e. it is port)
     *  1: Source is a trunk
     */
    GT_BOOL srcIsTrunk;

    /** @brief Source port or Source trunk
     *  if port, range (APPLICABLE RANGES: 0..63)
     *  if trunk, range (APPLICABLE RANGES: 0..127)
     */
    GT_U32 srcPortTrunk;

    /** MAC DA or MAC SA of the tunnel header */
    GT_ETHERADDR mac;

    /** packet VID assignment (APPLICABLE RANGES: 0..4095) */
    GT_U16 vid;

    /** whether the external mac is considered tagged on not */
    GT_BOOL isTagged;

    /** whether sourse is Trunk or not */
    GT_BOOL dsaSrcIsTrunk;

    /** @brief DSA tag source port or trunk; relevant only for DSA tagged packets.
     *  In multi port groups devices if this field is a criteria in rule,
     *  the field sourcePortGroupId is required to be filled as well :
     *  exact mach and equals to portGroupId to which dsaSrcPortTrunk
     *  is related.
     */
    GT_U32 dsaSrcPortTrunk;

    /** DSA tag source device; relevant only for DSA tagged packets */
    GT_U32 dsaSrcDevice;

    /** @brief Indicates the port group where the packet entered the device.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon. For multi port groups devices only)
     */
    GT_U32 sourcePortGroupId;

} CPSS_DXCH_TTI_RULE_COMMON_STC;

/**
* @struct CPSS_DXCH_TTI_IPV4_RULE_STC
 *
 * @brief TTI IPv4 TCAM rule.
*/
typedef struct{

    /** TTI Common fields in TCAM rule. */
    CPSS_DXCH_TTI_RULE_COMMON_STC common;

    /** @brief tunneling protocol of the packet: valid options:
     *  0: IPv4_OVER_IPv4
     *  1: IPv6_OVER_IPv4
     *  2: IPv4_OVER_GRE_IPv4; Tunnel IP protocol is 47 and
     *  GRE protocol is 0x0800.
     *  3: IPv6_OVER_GRE_IPv4; Tunnel IP protocol is 47 and
     *  GRE protocol is 0x86DD.
     *  4: Ethernet_OVER_GRE0_IPv4; Tunnel IP protocol is 47 and
     *  GRE protocol is set in function cpssDxChTtiEthernetTypeSet.
     *  5: Ethernet_OVER_GRE1_IPv4; Tunnel IP protocol is 47 and
     *  GRE protocol is set in function cpssDxChTtiEthernetTypeSet.
     *  6: Unknown - If the packet is identified as IPv4 GRE tunnel and
     *  there is no match on the configured GRE protocols
     *  7: Any Protocol - the packet is not one of the above protocols
     */
    GT_U32 tunneltype;

    /** tunnel source IP */
    GT_IPADDR srcIp;

    /** tunnel destination IP */
    GT_IPADDR destIp;

    /** whether the packet is ARP or not ARP */
    GT_BOOL isArp;

} CPSS_DXCH_TTI_IPV4_RULE_STC;


/**
* @struct CPSS_DXCH_TTI_MPLS_RULE_STC
 *
 * @brief TTI MPLS TCAM rule.
*/
typedef struct{

    /** TTI Common fields in TCAM rule. */
    CPSS_DXCH_TTI_RULE_COMMON_STC common;

    /** MPLS label 0; the outer most mpls label (APPLICABLE RANGES: 0..2^20 */
    GT_U32 label0;

    /** EXP of MPLS label 0 (APPLICABLE RANGES: 0..7) */
    GT_U32 exp0;

    /** @brief MPLS label 1; the inner MPLS label following Label0 (APPLICABLE RANGES: 0..2^20
     *  Note: must be set to 0 if this label does not exist
     */
    GT_U32 label1;

    /** @brief EXP of MPLS label 1 (APPLICABLE RANGES: 0..7)
     *  Note: must be set to 0 if this label does not exist
     */
    GT_U32 exp1;

    /** @brief MPLS label 2; the the inner MPLS label following Label1 (APPLICABLE RANGES: 0..2^20
     *  Note: must be set to 0 if this label does not exist
     */
    GT_U32 label2;

    /** @brief EXP of MPLS label 2 (APPLICABLE RANGES: 0..7)
     *  Note: must be set to 0 if this label does not exist
     */
    GT_U32 exp2;

    /** @brief The number of MPLS labels in the label stack. valid options:
     *  0: for One label
     *  1: for two labels
     *  2: for three labels
     *  3: for more than 3 labels
     */
    GT_U32 numOfLabels;

    /** @brief The protocol above MPLS. valid options:
     *  IPv4
     *  IPv6
     *  Note: feild not valid if more than 3 labels in the stack
     */
    GT_U32 protocolAboveMPLS;

    /** @brief GT_TRUE: if one of the MPLS stack labels is a reserved
     *  label (0<=label<=15)
     *  GT_FALSE: no reserved MPLS label is found in MPLS stack
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL reservedLabelExist;

    /** @brief If reservedLabelExist==GT_TRUE: The value of the 4 least
     *  significant bits of the reserved label
     *  If reservedLabelExist==GT_FALSE: The value of the 4 least
     *  significant bits of the MPLS label that is at the bottom
     *  of the MPLS stack
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 reservedLabelValue;

    /** @brief the index in the channel type profile table
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 channelTypeProfile;

    /** 5 bits following the last MPLS label (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    GT_U32 dataAfterInnerLabel;

    /** @brief first 4 bits immediately following MPLS end of stack label. If the first nibble
     *  of the control word is 0x1, this indicates the packet contains control information
     *  (APPLICABLE DEVICES: xCat3)
     */
    GT_U32 cwFirstNibble;

} CPSS_DXCH_TTI_MPLS_RULE_STC;


/**
* @struct CPSS_DXCH_TTI_ETH_RULE_STC
 *
 * @brief TTI Ethernet TCAM rule.
*/
typedef struct{

    /** TTI Common fields in TCAM rule. */
    CPSS_DXCH_TTI_RULE_COMMON_STC common;

    /** user priority as found in the outer VLAN tag header */
    GT_U32 up0;

    /** @brief CFI (Canonical Format Indicator) as found in the outer
     *  VLAN tag header
     */
    GT_U32 cfi0;

    /** @brief whether the fields: VLAN
     *  vlan1 is the vlan header matching the Ethertype that
     *  is different from the Ethertype selected for the port
     */
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

    /** @brief When packet has DSA tag this field reflects the source
     *  device as extracted from the DSA tag.
     *  When packet does not contain DSA tag this field is set to 0.
     *  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 srcId;

    /** @brief DSA tag QoS profile; relevant only for DSA tagged packets
     *  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 dsaQosProfile;

    /** @brief The index of the TPID used to identify this packet tag0
     *  this field is valid only when <VLAN0 Exists> is set.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag0TpidIndex;

    /** @brief The index of the TPID used to identify this packet tag1
     *  this field is valid only when <VLAN1 Exist> is set.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1TpidIndex;

    /** @brief The GRP[1:0] bits extracted from the IEEE 802.1BR E
     *  (APPLICABLE DEVICES: xCat3)
     */
    GT_U32 eTagGrp;

} CPSS_DXCH_TTI_ETH_RULE_STC;

/**
* @struct CPSS_DXCH_TTI_MIM_RULE_STC
 *
 * @brief TTI MIM TCAM rule.
*/
typedef struct{

    /** TTI Common fields in TCAM rule. */
    CPSS_DXCH_TTI_RULE_COMMON_STC common;

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

} CPSS_DXCH_TTI_MIM_RULE_STC;

/**
* @struct CPSS_DXCH_TTI_TRILL_RULE_STC
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
    CPSS_DXCH_TTI_RULE_COMMON_STC common;

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

} CPSS_DXCH_TTI_TRILL_RULE_STC;

/**
* @union CPSS_DXCH_TTI_RULE_UNT
 *
 * @brief Union for TTI rule.
 *
*/

typedef union{
    /** ipv4 rule */
    CPSS_DXCH_TTI_IPV4_RULE_STC ipv4;

    /** MPLS rule */
    CPSS_DXCH_TTI_MPLS_RULE_STC mpls;

    /** ethernet rule */
    CPSS_DXCH_TTI_ETH_RULE_STC eth;

    /** Mac in Mac rule (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_MIM_RULE_STC mim;

    /** TRILL rule (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_TRILL_RULE_STC trill;

    GT_U8 udbArray[CPSS_DXCH_TTI_MAX_UDB_CNS];

} CPSS_DXCH_TTI_RULE_UNT;


/**
* @enum CPSS_DXCH_TTI_PASSENGER_TYPE_ENT
 *
 * @brief TTI tunnel passenger protocol types.
*/
typedef enum{

    /** IPv4 passenger type */
    CPSS_DXCH_TTI_PASSENGER_IPV4_E              = 0,

    /** IPv6 passenger type */
    CPSS_DXCH_TTI_PASSENGER_IPV6_E              = 1,

    /** ethernet with crc passenger type */
    CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E      = 2,

    /** ethernet with no crc passenger type */
    CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E   = 3,

    /** IPv4/IPv6 passenger type */
    CPSS_DXCH_TTI_PASSENGER_IPV4V6_E            = 4,

    /** MPLS passenger type */
    CPSS_DXCH_TTI_PASSENGER_MPLS_E              = 5

} CPSS_DXCH_TTI_PASSENGER_TYPE_ENT;

/**
* @enum CPSS_DXCH_TTI_MPLS_CMD_ENT
 *
 * @brief Enumeration of MPLS command assigned to the packet
*/
typedef enum{

    /** do nothing */
    CPSS_DXCH_TTI_MPLS_NOP_CMD_E            = 0,

    /** swap the other label */
    CPSS_DXCH_TTI_MPLS_SWAP_CMD_E           = 1,

    /** push a new label */
    CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E          = 2,

    /** pop 1 label */
    CPSS_DXCH_TTI_MPLS_POP1_CMD_E           = 3,

    /** pop 2 labels */
    CPSS_DXCH_TTI_MPLS_POP2_CMD_E           = 4,

    /** pop one label and swap the second label */
    CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E   = 5,

    /** pop 3 labels */
    CPSS_DXCH_TTI_MPLS_POP3_CMD_E           = 6

} CPSS_DXCH_TTI_MPLS_CMD_ENT;

/**
* @enum CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT
 *
 * @brief TTI action redirect command.
*/
typedef enum{

    /** do not redirect this packet */
    CPSS_DXCH_TTI_NO_REDIRECT_E               = 0,

    /** @brief policy switching : Any non-TT can
     *  be redirect to egress port. But if
     *  TT, then only Ether-over-MPLS
     */
    CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E        = 1,

    /** policy routing  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E = 2,

    /** vrf id assignment (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_VRF_ID_ASSIGN_E             = 4,

    /** @brief logical port assignment
     *  (APPLICABLE DEVICES: xCat3; Lion2)
     */
    CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E       = 5

} CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT;

/**
* @enum CPSS_DXCH_TTI_VLAN_COMMAND_ENT
 *
 * @brief TTI action VLAN command.
*/
typedef enum{

    /** @brief do not modify vlan */
    CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E     = 0,

    /** @brief modify vlan only for untagged or Priority tagged
     *         packets
     *         If the packet is tunnel-terminated and the
     *         passenger is Ethernet, this applies to the
     *         passenger Ethernet packet
     * */
    CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E   = 1,

    /** @brief modify vlan only for tagged packets */
    CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E     = 2,

    /** @brief modify vlan to all packets */
    CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E        = 3

} CPSS_DXCH_TTI_VLAN_COMMAND_ENT;


/**
* @enum CPSS_DXCH_TTI_MODIFY_UP_ENT
 *
 * @brief TTI modify UP enable
*/
typedef enum{

    /** do not modify the previous UP enable setting */
    CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E   = 0,

    /** set modify UP enable flag to 1 */
    CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E        = 1,

    /** set modify UP enable flag to 0 */
    CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E       = 2,

    /** reserved */
    CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E      = 3

} CPSS_DXCH_TTI_MODIFY_UP_ENT;

/**
* @enum CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT
 *
 * @brief TTI tag1 UP command
*/
typedef enum{

    /** @brief If packet does not contain
     *  Tag1 assign according to action entry's <UP1>, else use Tag1<UP>
     */
    CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E      = 0,

    /** @brief If packet contains Tag0
     *  use Tag0<UP0>, else use action entry's <UP1> field
     */
    CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E      = 1,

    /** @brief Assign action entry's <UP1> field
     *  to all packets
     */
    CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E                 = 2

} CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT;

/**
* @enum CPSS_DXCH_TTI_MODIFY_DSCP_ENT
 *
 * @brief TTI modify DSCP enable
*/
typedef enum{

    /** do not modify the previous DSCP enable setting */
    CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E = 0,

    /** set modify DSCP enable flag to 1 */
    CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E      = 1,

    /** set modify DSCP enable flag to 0 */
    CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E     = 2,

    /** reserved */
    CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E    = 3

} CPSS_DXCH_TTI_MODIFY_DSCP_ENT;

/**
* @enum CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT
 *
 * @brief TTI qos trust table selection
*/
typedef enum{

    /** PTP trigger type PTP over L2 */
    CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E,

    /** PTP trigger type PTP over IPv4 UDP */
    CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E,

    /** PTP trigger type PTP over IPv6 UDP */
    CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E,

    /** reserved */
    CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_RESERVED_E

} CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT;

/**
* @enum CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT
 *
 * @brief TTI passenger parsing of transit MPLS tunnel mode
*/
typedef enum{

    /** parsing is based on the tunnel header. The passenger packet is not parsed. */
    CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E,

    /** Parse passenger packet as IPv4/6 */
    CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E,

    /** Parse passenger packet as Ethernet */
    CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E,

    /** Parse passenger packet as Control Word followed by Ethernet */
    CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E

} CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT;


/**
* @enum CPSS_DXCH_TTI_PW_TAG_MODE_ENT
 *
 * @brief Enumeration of TTI Pseudowire Tag Mode.
*/
typedef enum{

    /** NO Pseudowire VLAN manipulation */
    CPSS_DXCH_TTI_PW_TAG_DISABLED_MODE_E        = 0,

    /** @brief Inner packet outer tag (VID0)
     *  is the user tag, used for egress VLAN filtering.
     */
    CPSS_DXCH_TTI_PW_TAG_RAW_PW_MODE_E          = 1,

    /** @brief Inner packet outer tag (VID0)
     *  is the PTag, which should always exist. Inner packet inner tag
     *  (VID1) is the user tag, used for egress VLAN filtering.
     */
    CPSS_DXCH_TTI_PW_TAG_TAGGED_PW_MODE_E       = 2

} CPSS_DXCH_TTI_PW_TAG_MODE_ENT;

/**
* @struct CPSS_DXCH_TTI_ACTION_2_STC
 *
 * @brief TTI TCAM rule action parameters (standard action).
*/
typedef struct{

    /** @brief whether the packet's tunnel header is removed;
     *  must not be set for non-tunneled packets
     */
    GT_BOOL tunnelTerminate;

    /** @brief passenger packet type (refer to CPSS_DXCH_TTI_PASSENGER_TYPE_ENT).
     *  When <tunnelTerminate> is GT_FALSE but <MPLS Command> = POP<n>
     *  or PUSH, this field must be assigned a value of MPLS.
     *  Valid values:
     *  CPSS_DXCH_TTI_PASSENGER_IPV4V6_E
     *  CPSS_DXCH_TTI_PASSENGER_MPLS_E (APPLICABLE DEVICES: xCat3; xCat2; Lion2)
     *  CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E
     *  CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E
     */
    CPSS_DXCH_TTI_PASSENGER_TYPE_ENT ttPassengerPacketType;

    /** @brief Type of passenger packet for packet redirected
     *  to Tunnel-Start.
     *  Valid only when <Redirect Command>= Redirect to
     *  egress interface and <TunnelStart> = Enabled.
     *  When packet is not TT but <MPLS Command> = POP<n>
     *  or PUSH, the <Tunnel Type> bit must be set to OTHER.
     */
    CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT tsPassengerPacketType;

    /** @brief If '0' then the TT Header Len is default TT
     *  algorithm for each key type
     *  Else this is the Tunnel header length in units of Bytes.
     *  Granularity is in 2 Bytes.
     *  Note: Tunnel header begins at the start of the L3
     *  header, i.e. immediately after the EtherType field
     *  This field is relevant if TTI Action <Tunnel Terminate> = Enabled
     *  or TTI Action <Passenger Parsing of Non-MPLS Transit Tunnels Enable> = Enabled
     *  (APPLICABLE RANGES: 0..62) (APPLICABLE DEVICES: Bobcat2; Caelum)
     *  (APPLICABLE RANGES: 0..126) (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    GT_U32 ttHeaderLength;

    /** @brief Enables next TTI lookup (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  TTL
     *  ---
     */
    GT_BOOL continueToNextTtiLookup;

    /** @brief If <GT_TRUE> and the packet is MPLS
     *  (<MPLS Command> = POP and <MPLS TTL> = 0, or
     *  <Tunnel Terminate>=Enabled),
     *  the TTL (for Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X: also EXP) is
     *  copied from the last popped label.
     *  If <GT_TRUE> and the packet is X-over-IPv4
     *  tunnel-termination, and the passenger is IPv4/6,
     *  the TTL is copied from the IPv4 tunnel header
     *  rather than from passenger packet.
     *  MPLS
     *  ----
     */
    GT_BOOL copyTtlExpFromTunnelHeader;

    /** MPLS Action applied to the packet */
    CPSS_DXCH_TTI_MPLS_CMD_ENT mplsCommand;

    /** @brief 0:   TTL is taken from the tunnel or inner MPLS label
     *  1-255: TTL is set according to this field
     *  Relevant for MPLS packets that have a match in
     *  the TTI lookup, and with <MPLS Command>!=NOP
     */
    GT_U32 mplsTtl;

    /** @brief for MPLS packets that are not tunnel terminated
     *  <GT_TRUE>: TTL is decremented by one
     *  <GT_FALSE>: TTL is not decremented
     *  for TRILL packets:
     *  <GT_TRUE>: TRILL header hop count is decremented by one
     *  <GT_FALSE>: TRILL header hop count is not decremented
     *  Redirect Command and Data
     *  -------------------------
     */
    GT_BOOL enableDecrementTtl;

    /** @brief parsing is based on the tunnel header
     *  or The passenger packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon)
     *  (refer to CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT)
     */
    CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT passengerParsingOfTransitMplsTunnelMode;

    /** @brief generic mechanism to parse
     *  the passenger of transit tunnel packets other than MPLS,
     *  e.g. IP-based tunnels, MiM tunnels, etc.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon)
     *  (APPLICABLE RANGES: 0..255)
     *  Forwarding Attributes
     *  ---------------------
     */
    GT_BOOL passengerParsingOfTransitNonMplsTransitTunnelEnable;

    /** @brief forwarding command; valid values:
     *  CPSS_PACKET_CMD_FORWARD_E
     *  CPSS_PACKET_CMD_MIRROR_TO_CPU_E
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     *  CPSS_PACKET_CMD_DROP_HARD_E
     *  CPSS_PACKET_CMD_DROP_SOFT_E
     *  for TRILL packets: If Command is TRAP, HARD-DROP, or
     *  SOFT-DROP and the packet is TRILL Multi-Target instance 0,
     *  the second instance is implicitly assigned HARD-DROP
     */
    CPSS_PACKET_CMD_ENT command;

    /** @brief where to redirect the packet
     *  (refer to CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)
     */
    CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT redirectCommand;

    /** @brief egress interface to redirect packet. valid if <redirectCommand> is
     *  REDIRECT_TO_EGRESS. The packet is forwarded as Unicast to a port or a trunk
     *  For xCat3 when VPLS mode init parameter is enabled
     *  interface type = CPSS_INTERFACE_PORT_E
     *  For Lion2 B0 this is used as virtual source port when
     *  <redirectCommand> is LOGICAL_PORT.
     */
    CPSS_INTERFACE_INFO_STC egressInterface;

    /** @brief pointer to the ARP entry for MPLS routed packets
     *  Valid only when <Redirect Command> = Redirect to egress interface
     *  and <TunnelStart> = Disabled
     */
    GT_U32 arpPtr;

    /** @brief <GT_TRUE>: redirected to Tunnel Egress Interface
     *  <GT_FALSE>: redirected to non-Tunnel Egress Interface
     *  (supported for X-over-MPLS Tunnel Start only).
     *  Relevant only when <Redirect Command> = Redirect to egress interface
     */
    GT_BOOL tunnelStart;

    /** @brief pointer to the Tunnel Start entry. valid if <tunnelStart>
     *  is GT_TRUE and <redirectCommand> is REDIRECT_TO_EGRESS
     */
    GT_U32 tunnelStartPtr;

    /** @brief For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X this is a pointer to a leaf in
     *  the LPM RAM if <redirectCommand> is REDIRECT_TO_ROUTER_LOOKUP.
     *  For other devices, this is a pointer to the Router Lookup Translation
     *  Table entry if <redirectCommand> is REDIRECT_TO_ROUTER_LOOKUP.
     *  FEr#2018 - Router Lookup Translation Table (LTT) index can be written
     *  only to column 0 of the LTT row.
     *  NOTE:
     *  - xCat3 devices support index 0,4,8,12(max IP TCAM row 4) only
     *  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon)
     */
    GT_U32 routerLttPtr;

    /** @brief VRF
     *  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon)
     *  (APPLICABLE RANGES: 0..4095)
     *  Source ID assignment
     *  --------------------
     */
    GT_U32 vrfId;

    /** if set, then the <sourceId> is assign to the packet */
    GT_BOOL sourceIdSetEnable;

    /** @brief source ID assigned to the packet if <sourceIdSetEnable> is GT_TRUE
     *  (APPLICABLE RANGES: 0..31; Bobcat2; Caelum; Bobcat3, Aldrin2, Aldrin, AC3X 0..4095)
     *  VLAN Modification and Assignment
     *  --------------------------------
     */
    GT_U32 sourceId;

    /** @brief tag0 vlan command; valid options:
     *  CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E
     *  CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E
     *  CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E
     *  CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E
     */
    CPSS_DXCH_TTI_VLAN_COMMAND_ENT tag0VlanCmd;

    /** tag0 VLAN */
    GT_U16 tag0VlanId;

    /** @brief tag1 vlan command; valid options:
     *  CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E
     *  CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E
     *  Relevant if <mplsCommand> == CPSS_DXCH_TTI_MPLS_NOP_CMD_E
     */
    CPSS_DXCH_TTI_VLAN_COMMAND_ENT tag1VlanCmd;

    /** @brief tag1 VLAN
     *  Relevant if <mplsCommand> == CPSS_DXCH_TTI_MPLS_NOP_CMD_E
     */
    GT_U16 tag1VlanId;

    /** @brief whether the VID assignment can be overridden by
     *  subsequent VLAN assignment mechanism
     *  (refer to CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT tag0VlanPrecedence;

    /** @brief enable/disable nested vlan; relevant only for
     *  Ethernet packets
     *  Bind to Policer
     *  ---------------
     */
    GT_BOOL nestedVlanEnable;

    /** @brief If set, the packet is bound to the policer meter
     *  specified in <Policer Index>
     */
    GT_BOOL bindToPolicerMeter;

    /** @brief If set, the packet is bound to the policer/billing/IPFIX
     *  counter specified in <Policer Index>
     */
    GT_BOOL bindToPolicer;

    /** @brief Traffic profile to be used if the <bindToPolicer> is set
     *  Qos Fields
     *  ----------
     */
    GT_U32 policerIndex;

    /** @brief whether QoS profile can be overridden by subsequent pipeline
     *  engines (refer to CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;

    /** @brief <GT_FALSE>: Assign QoS profile based on QoS fields
     *  <GT_TRUE>: Keep prior QoS Profile assignment
     */
    GT_BOOL keepPreviousQoS;

    /** @brief <GT_FALSE>: Do not trust packets L2 QoS
     *  <GT_TRUE>: If packet is tunnel-terminated,
     *  trust passenger packet L2 QoS. If packet is
     *  not tunnel-terminated, trust packet L2 QoS
     *  Relevant only if <keepPreviousQoS>=GT_FALSE.
     */
    GT_BOOL trustUp;

    /** @brief <GT_FALSE>: Do not trust packets L3 QoS
     *  <GT_TRUE>: If packet is not tunnel-terminated,
     *  trust packets outer, remaining, MPLS labels EXP
     *  Relevant only if <keepPreviousQoS>=GT_FALSE.
     */
    GT_BOOL trustDscp;

    /** @brief <GT_FALSE>: Do not trust packets EXP
     *  <GT_TRUE>: If packet is tunnel-terminated, trust
     *  passenger packet L3 QoS. If packet is not
     *  tunnel-terminated, trust packet L3 QoS
     *  Relevant only if <keepPreviousQoS>=GT_FALSE.
     */
    GT_BOOL trustExp;

    /** @brief QoS profile to assign to the packet
     *  Relevant only if <keepPreviousQoS>=GT_FALSE.
     */
    GT_U32 qosProfile;

    /** modify tag0 UP (refer to CPSS_DXCH_TTI_MODIFY_UP_ENT) */
    CPSS_DXCH_TTI_MODIFY_UP_ENT modifyTag0Up;

    /** tag1 UP command (refer to CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT) */
    CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT tag1UpCommand;

    /** @brief modify DSCP mode (or EXP for MPLS)
     *  (refer to CPSS_DXCH_TTI_MODIFY_DSCP_ENT)
     */
    CPSS_DXCH_TTI_MODIFY_DSCP_ENT modifyDscp;

    /** tag0 UP assignment */
    GT_U32 tag0Up;

    /** tag1 UP assignment */
    GT_U32 tag1Up;

    /** @brief <GT_FALSE>: Packets DSCP is not remapped.
     *  <GT_TRUE>: Packets DSCP is remapped.
     *  If <Tunnel Terminate> = Enabled, the field is
     *  relevant only if passenger packet is IPv4/6.
     *  If <Tunnel Terminate> = Disabled, the field is
     *  relevant only if <Trust DSCP> = Enabled and the
     *  packet IPv4 or IPv6
     */
    GT_BOOL remapDSCP;

    /** @brief Indication if the 'Trust Qos Mapping Table Index' selected
     *  according to the UP (user priority - 0..7) of the packet or
     *  according the mappingTableIndex parameter (0..11).
     *  <GT_TRUE> - according to the UP (user priority) of the packet.
     *  <GT_FALSE> - according the mappingTableIndex parameter.
     *  Relevant only if <keepPreviousQoS>=GT_FALSE.
     *  Relevant only for L2 trust.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     */
    GT_BOOL qosUseUpAsIndexEnable;

    /** @brief the 'Trust Qos Mapping Table Index' (table selector).
     *  the value is ignored when useUpAsIndex == GT_TRUE
     *  Relevant only if <keepPreviousQoS>=GT_FALSE.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     *  (APPLICABLE RANGES : 0..11)
     */
    GT_U32 qosMappingTableIndex;

     /** @brief mplsLLspQoSProfileEnable-
     *  <GT_FALSE>: Do not support L-LSP Qos Profile
     *  <GT_TRUE>: the packet is assigned a QoSProfile
     *  that is the concatenation of the TTI Action<QoS Profile> and (QoSProfile 3 last bits)
     *  packet outer MPLS label EXP; this field is relevant if TTI
     *  Action <TRUST EXP> is unset, and TTI Action<Keep QoS> is unset
     *  TTI Action profile is the base index in the QOS table and
     *  EXP is the offset from this base.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL mplsLLspQoSProfileEnable;

    /** @brief IPCL Lookup Configurations
     *  ---------------------------------
     */
    /** @brief Controls the index used for IPCL0 lookup
     *  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0OverrideConfigIndex;

    /** @brief Controls the index used for IPCL0
     *  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0_1OverrideConfigIndex;

    /** @brief Controls the index used for IPCL1 lookup
     *  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     */
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT pcl1OverrideConfigIndex;

    /** @brief Pointer to IPCL configuration entry to be used
     *  when fetching IPCL parameter. This overrides
     *  the Port/VLAN based configuration entry selection.
     *  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     *  For xCat3 when VPLS mode init parameter is enabled
     *  (APPLICABLE RANGES: xCat3 0..2047)
     */
    GT_U32 iPclConfigIndex;

    /** @brief If set, <iPclUdbConfigTableIndex> holds direct index to one of 7 UDB configuration
     *  table user defined ethertype entries.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL iPclUdbConfigTableEnable;

    /** @brief Direct index to one of 7 UDB configuration table user defined ethertype entries.
     *  Relevant only when <redirectCommand> == CPSS_DXCH_TTI_NO_REDIRECT_E
     *  valid values;
     *  CPSS_DXCH_PCL_PACKET_TYPE_UDE_E
     *  CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E
     *  CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E
     *  CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E
     *  CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E
     *  CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E
     *  CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_PCL_PACKET_TYPE_ENT iPclUdbConfigTableIndex;

    /** @brief Management and Debug
     *  ---------------------------
     */
    /** @brief If set, the packet is mirrored to the Ingress
     *  Analyzer port. in eArch devices this enables writing mirrorToIngressAnalyzerIndex.
     */
    GT_BOOL mirrorToIngressAnalyzerEnable;

    /** @brief Enables mirroring the packet to an Ingress Analyzer interface.
     *  If the port configuration assigns a different analyzer.
     *  index, the higher index wins.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X),
     *  (APPLICABLE RANGES: 0..7).
     */
    GT_U32 mirrorToIngressAnalyzerIndex;

    /** @brief user defined cpu code. relavant only if command is
     *  trap or mirror
     */
    CPSS_NET_RX_CPU_CODE_ENT userDefinedCpuCode;

    /** @brief <GT_FALSE>: central counter binding is disabled.
     *  <GT_TRUE>: central counter binding is enabled,
     */
    GT_BOOL bindToCentralCounter;

    /** @brief central counter bound to this entry
     *  Relevant only when <bindToCentralCounter> == GT_TRUE
     *  For xCat3 when VPLS mode init parameter is enabled (APPLICABLE RANGES: xCat3 0..4095)
     */
    GT_U32 centralCounterIndex;

    /** @brief if set, the packet MAC SA/DA are switched on the transmitting
     *  Relevant only when <redirectCommand> == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     */
    GT_BOOL vntl2Echo;

    /** if set, the packet isn't subject to any bridge mechanisms */
    GT_BOOL bridgeBypass;

    /** If set, the ingress engines are all bypassed for this packet */
    GT_BOOL ingressPipeBypass;

    /** if set, the packet will not have any searches in the IPCL mechanism */
    GT_BOOL actionStop;

    /** @brief Determines which mask is used in the CRC based hash.
     *  0: do not override hash mask index.
     *  Other values: override the hash mask index value.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 hashMaskIndex;

    /** @brief If set, the packet is marked for MAC SA modification
     *  Relevant only when <redirectCommand> == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL modifyMacSa;

    /** @brief If set, the packet is marked for MAC DA modification,
     *  and the <ARP Index> specifies the new MAC DA.
     *  Relevant only when <redirectCommand> == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL modifyMacDa;

    /** @brief If set, the source port group ID of the packet is set to the current port group.
     *  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL ResetSrcPortGroupId;

    /** @brief If set, the packet is forwarded to a ring port for another TTI lookup.
     *  Relevant only when <redirectCommand> == CPSS_DXCH_TTI_NO_REDIRECT_E
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL multiPortGroupTtiEnable;

    /** @brief Enable assignment of Source ePort number from this entry
     *  GT_FALSE- Disabled
     *  GT_TRUE - Enabled - New source ePort number
     *  is taken from TTI Action Entry<Source ePort>.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL sourceEPortAssignmentEnable;

    /** @brief Source ePort number assigned by TTI entry
     *  when <Source ePort Assignment Enable> = Enabled
     *  NOTE: Overrides any previous assignment of
     *  source ePort number.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     *  (APPLICABLE RANGES: 0..8191)
     */
    GT_PORT_NUM sourceEPort;

    /** @brief The flow ID assigned to the packet.
     *  The value 0x0 represents do not assign Flow ID. The TTI overrides the
     *  existing Flow ID value if and only if this field in the TTI action is non-zero.
     *  NOTE: Valid only when redirect command == CPSS_DXCH_TTI_NO_REDIRECT_E
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X)
     *  (APPLICABLE RANGES: 0..65535)
     */
    GT_U32 flowId;

    /** @brief Override MAC2ME check and set to 1
     *  GT_FALSE - Do not override Mac2Me mechanism
     *  GT_TRUE - Override the Mac2Me mechanism and set Mac2Me to 0x1
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL setMacToMe;

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

    /** @brief Assigns the proper VID into VID0 for egress VLAN filtering,
     *  and sets VID1 to the User Tag if it exists
     *  Relevant when VPLS mode is enabled.
     *  (APPLICABLE DEVICES: xCat3)
     */
    CPSS_DXCH_TTI_PW_TAG_MODE_ENT pwTagMode;

    /** @brief OAM related fields
     *  --------------------------
     */
    /** @brief Indicates that a timestamp should be inserted into the packet:
     *  GT_TRUE - enable timestamp insertion.
     *  GT_FALSE - disable timestamp insertion.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL oamTimeStampEnable;

    /** @brief When <Timestamp Enable> is enabled, indicates the offset
     *  index for the timestamp offset table.
     *  (APPLICABLE RANGES: 0..127)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
     */
    GT_U32 oamOffsetIndex;

    /** @brief Binds the packet to an entry in the OAM Table.
     *  GT_TRUE - enable OAM packet processing.
     *  GT_FALSE - disable OAM packet processing.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
     */
    GT_BOOL oamProcessEnable;

    /** @brief Determines the set of UDBs where the key attributes
     *  (opcode, MEG level, RDI, MEG level) of the OAM message
     *  are taken from. (APPLICABLE RANGES: 0..1)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
     */
    GT_U32 oamProfile;

    /** @brief When enabled, the MPLS G-ACh Channel Type is mapped
     *  to an OAM Opcode that is used by the OAM engine
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  PTP related fields
     */
    GT_BOOL oamChannelTypeToOpcodeMappingEnable;

    /** @brief PTP related fields
     *  -------------------------
     */
    /** @brief When enabled, the TTI Action defines where the parser should start parsing
     *  the PTP header. The location is defined by TTI Action<PTP Offset>.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL isPtpPacket;

    /** @brief When <Is PTP Packet> is enabled, this field specifies the PTP Trigger Type
     *  (refer to CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_ENT ptpTriggerType;

    /** @brief When <Is PTP Packet> is enabled, this field defines the beginning of the PTP header,
     *  relative to the beginning of the L3 header. Note: <PTP Offset> is counted from <Inner L3 Offset>.
     *  (APPLICABLE RANGES: 0..127)(APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon).
     *  PWE3 related fields
     *  --------------------
     */
    GT_U32 ptpOffset;

    /** @brief PWE3 related fields
     *  --------------------------
     */
    /** @brief if set, indicates that this PseudoWire integrates a control word immediately following
     *  the bottom of the label stack
     *  For xCat3 Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL cwBasedPw;

    /** @brief if set, enables TTL expiration exception command assignment for Pseudo Wire
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL ttlExpiryVccvEnable;

    /** @brief if set, indicates that this PW supports flow hash label, and thus <PW_LABEL>
     *  is not at the bottom of the MPLS label stack, instead it is one label above
     *  the bottom of the label stack
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL pwe3FlowLabelExist;

    /** @brief if set, indicates that this PW
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL pwCwBasedETreeEnable;

    /** @brief determines whether to apply the non
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */

    /** @brief VPLS related fields
     *  --------------------------
     */
    GT_BOOL applyNonDataCwCommand;

    /** @brief if set, then the <unknownSaCommand> is assign to the packet.
     *  else configure action to preserve previous assignment.
     *  (APPLICABLE DEVICES: xCat3)
     */
    GT_BOOL unknownSaCommandEnable;

    /** @brief Assigns the unknown
     *  Relevant when unknownSaCommandEnable == GT_TRUE and when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat3)
     */
    CPSS_PACKET_CMD_ENT unknownSaCommand;

    /** @brief if set, then the <sourceMeshId> is assign to the packet.
     *  Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat3)
     */
    GT_BOOL sourceMeshIdSetEnable;

    /** @brief Source Mesh ID assigned to the packet if <sourceMeshIdSetEnable> is GT_TRUE
     *  (APPLICABLE RANGES: 1..3).
     *  Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat3)
     */
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


    /** @brief Priority resolution between ExactMatch action and TTI action
     *   GT_TRUE - The Exact Match action has a priority over the TTI action.
     *   GT_FALSE - The Exact Match action doesn't have a priority over the TTI action.
     *   (APPLICABLE DEVICES: Falcon)
     */
     GT_BOOL exactMatchOverTtiEn;

} CPSS_DXCH_TTI_ACTION_2_STC;

/**
* @union CPSS_DXCH_TTI_ACTION_UNT
 *
 * @brief Union for TTI action.
 *
*/

typedef union{
    /** type2 action (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_ACTION_2_STC type2;

} CPSS_DXCH_TTI_ACTION_UNT;


/**
* @enum CPSS_DXCH_TTI_ACTION_TYPE_ENT
 *
 * @brief TTI action type enum
*/
typedef enum{
    /** type2 action (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon) */
    CPSS_DXCH_TTI_ACTION_TYPE2_ENT   = 1

} CPSS_DXCH_TTI_ACTION_TYPE_ENT;


/**
* @enum CPSS_DXCH_TTI_EXCEPTION_ENT
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
    CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E       = 0,

    /** @brief IPv4 tunnel termination option/fragment error.
     *  An IPv4 Tunnel Termination Option/Fragment error occurs if the packet
     *  has either IPv4 Options or is fragmented.
     *  - IPv4 Options are detected if the IPv4 header <IP Header Len> > 5.
     *  - The packet is considered fragmented if either the IPv4 header flag
     *  <More Fragments> is set, or if the IPv4 header <Fragment offset> > 0.
     *  A tunnel-terminated fragmented packet must be reassembled prior
     *  to further processing of the passenger packet.
     */
    CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E  = 1,

    /** @brief IPv4 tunnel termination unsupported GRE header error.
     *  An IPv4 Tunnel Termination GRE Header error occurs if the IPv4 header
     *  <IP Protocol> = 47 (GRE) and the 16 most significant bits of the GRE
     *  header are not all 0.
     */
    CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E    = 2,

    /** @brief An MPLS TTL error is detected if the MPLS TTI rule has a match and
     *  any of the MPLS labels popped by the TTI have reached the value 0.
     *  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E  = 3,

    /** @brief MPLS unsupported error occurs if one of the following:
     *  - The <Tunnel Terminate> field is enabled for an MPLS packet, and the
     *  packet has more than three MPLS labels
     *  - The <MPLS Command> cannot be assigned to the packet, for one of
     *  the following reasons:
     *  - <MPLS Command> requires to Pop more labels than the packet has
     *  (this check is performed only when <Tunnel Terminate> is disabled).
     *  - <MPLS Command> != NOP/Push and packet is not MPLS.
     *  (APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E  = 4,

    /** @brief TRILL IS IS adjacency exception is detected if there is TCAM miss for
     *  unicast TRILL packet, indicating the packet was received from an invalid adjacency;
     *  the global configurable <TRILL IS IS Adjacency exception Cmd> is applied, and the
     *  respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 0.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E = 5,

    /** @brief TRILL tree adjacency exception is detected if there is TCAM miss for
     *  multi-target TRILL packet, indicating the packet was received from an invalid adjacency;
     *  the global configurable <TRILL Tree Adjacency exception Cmd> is applied, and the
     *  respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 1.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E = 6,

    /** @brief TRILL bad version exception is detected if the packet TRILL header version is greater
     *  than the global configurable <Max TRILL version>, the global configurable
     *  <TRILL bad Version exception Cmd> is applied, and the respective exception counter is icremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 2.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E = 7,

    /** @brief TRILL hop count exception is detected if the packet TRILL header Hop Count is ZERO,
     *  the global configurable <TRILL Hopcount is Zero exception Cmd> is assigned to the packet,
     *  and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 3.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E = 8,

    /** @brief TRILL options exception is detected if the packet TRILL header includes options
     *  (.i.e., the TRILL header Op-length field is greater than zero), the global configurable
     *  <TRILL Options exception Cmd> is assigned to the packet, and its respective exception
     *  counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 4.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E = 9,

    /** @brief TRILL critical hopbyhop exception is detected if the packet TRILL header
     *  includes options and the CHBH bit is set, the global configurable
     *  <TRILL CHBH exception Cmd> is assigned to the packet, and its respective exception
     *  counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 5.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E = 10,

    /** @brief The TRILL standard specifies that all TRILL packets on a link, if tagged, must use
     *  the VLAN-ID as specified by the designated RBridge.
     *  TRILL bad outer VID0 exception is detected if the packet TRILL outer tag VLAN-ID
     *  (Tag0 VLAN-ID) does not equal the global configurable <TRILL Outer Tag0 VID>,
     *  the global configurable <TRILL Bad Outer VID exception Cmd> is assigned to the packet,
     *  and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 6.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E = 11,

    /** @brief TRILL transit Unicast traffic, for which this RBridge is the nexthop, must have
     *  the MAC DA of this RBridge. Otherwise, the packet should be dropped by this RBridge.
     *  This is a normal occurrence if the link is attached to a bridged LAN and there are
     *  more than two peer RBridges on this link.
     *  It is required that the MAC DA's of this RBridge are configured in the MAC2ME table.
     *  TRILL UC not to me exception is detected if the packet is Unicast TRILL and its
     *  outer MAC DA does not match one of the of the MAC addresses configured in the MAC2ME table,
     *  the global configurable <TRILL UC Not MAC2ME exception Cmd> is assigned to the packet,
     *  and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 7.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E = 12,

    /** @brief TRILL MC with bad outer DA exception is detected if the TRILL outer MAC DA
     *  is Multicast, and the TRILL header M-bit is clear, the global configurable
     *  <TRILL MC M-bit exception Cmd> is assigned to the packet, and its respective
     *  exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 8.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E = 13,

    /** @brief TRILL UC with bad outer DA exception is detected if the TRILL outer MAC DA is UC,
     *  and the TRILL header M-bit is set, the global configurable <TRILL UC bad outer exception Cmd>
     *  is assigned to the packet, and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 9.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E = 14,

    /** @brief TRILL outer UC inner MC exception is detected if the TRILL outer MAC DA is UC,
     *  and the inner DA is MC, the global configurable <TRILL outer UC inner MC exception Cmd>
     *  is assigned to the packet, and its respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 10.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E = 15,

    /** @brief TRILL MC with bad outer DA is detected if the TRILL header Mbit is set, and the outer
     *  MAC DA is not the global configurable <All-RBridges Multicast address>, the global configurable
     *  <TRILL Bad MC DA exception Cmd> is assigned to the packet, and its respective exception
     *  counter is incremented.
     *  If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 11.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E = 16,

    /** @brief TRILL options too long exception is detected if If TRILL Header<Options Length> > 10;
     *  the global configurable <TRILL Options too long Cmd> is applied, and the
     *  respective exception counter is incremented.
     *  If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +16
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E = 21,

    /** @brief An IPv4 TTI Illegal Header SIP exception is detected if ANY of the following are not met:
     *  - IPv4 header <SIP> is loopback (127.0.0.1)
     *  - IPv4 header <SIP> is multicast address (224.x.x.x)
     *  - IPv4 header <SIP> is limited broadcast address (255.255.255.255)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E = 22,

    /** @brief The GRE header may optionally contain header extensions.
     *  If the IPv6 header <IP Next Header> = 47 (GRE) and the TTI action <Tunnel Terminate> = 1
     *  (packet is to be tunnel terminated) there is an optional check to verify
     *  that the 16 most significant bits of the GRE header are all 0. The check
     *  ensures that the GRE <version> is '0' and that the GRE header length is
     *  4 bytes, (i.e., there are no GRE extension headers, for example, checksum,
     *  key, sequence fields)
     *  If this check is enabled and the 16 most significant bits of the GRE header
     *  NOT all 0, the GRE Options exception command is applied.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E = 23,

    /** @brief An IPv6 TTI Header Error exception is detected if ANY of the following
     *  criteria are not met:
     *  - IPv6 header <version> = 6
     *  - IPv6 header <payload length> + 40 + packet L3 offset + 4 (CRC length) <= MAC layer packet byte count
     *  - IPv6 header <SIP> != IPv6 header <DIP>
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E = 24,

    /** @brief An IPv6 TTI HopbyHop exception occurs if the TTI action indicates this
     *  is a tunnel terminated packet, and the IPv6 header <IP Next Header> == 0
     *  is treated as an Hop by Hop exception
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E = 25,

    /** @brief An IPv6 TTI NonHopbyHop exception occurs if the TTI action indicates
     *  this is a tunnel terminated packet, and the IPv6 header <IP Next Header>
     *  contains any of recognized IPv6 extension headers other than hop-by-hop
     *  extension
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E = 26,

    /** @brief An IPv6 TTI SIP Address exception is triggered if ANY of the following is true:
     *  - IPv6 header <SIP> is ::1/128 (loopback address)
     *  - IPv6 header <SIP> is FF::/8 (multicast address)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E = 27

} CPSS_DXCH_TTI_EXCEPTION_ENT;

/**
* @enum CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT
 *
 * @brief TTI multicast duplication mode enum
*/
typedef enum{

    /** Disable descriptor duplication */
    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E = 0,

    /** Enable descriptor duplication */
    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E = 1,

    /** @brief Enable duplication for GRE protocol only, that matches
     *  global<GRE0_EtherType>/<GRE1_EtherType>
     */
    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_GRE_E = 2,

    /** Enable duplication for GRE protocol only, for any GRE Protocol */
    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_GRE_E = 3,

    /** @brief Enable duplication for UDP protocol only, for UDP DPort that matches
     *  the TTI Global <IPv4/6 MC Duplication UDP DPort>
     */
    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_SPECIFIC_UDP_PORT_E = 4,

    /** Enable duplication for UDP protocol only, for any UDP DPort */
    CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_FOR_ANY_UDP_PORT_E = 5

} CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT;

/**
* @enum CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT
 *
 * @brief TTI pseudowire exceptions enum
*/
typedef enum{

    /** @brief TTL expiry VCCV
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_EXPIRY_VCCV_E = 0,

    /** Nondata control word (i.e., control word first nibble = 1) */
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_NON_DATA_CONTROL_WORD_E = 1,

    /** @brief Applied when all the following conditions are met:
     *  1. Data control word (first nibble = 0)
     *  2. TTI action entry is matched and mark the packet for Tunnel Termination
     *  3. PW-CW<FRG> != 0
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_FRAGMENTED_E = 2,

    /** @brief Applied when all the following conditions are met:
     *  1. Data control word (first nibble = 0)
     *  2. TTI action entry is matched and mark the packet for Tunnel Termination
     *  3. ((TTI-AE<PW-CW Sequencing Enable> == Disable) AND (PW-CW<Sequence> != 0))
     *  OR
     *  ((TTI-AE<PW-CW Sequencing Enable> == enable) AND (PW-CW<Sequence> == 0))
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_SEQUENCE_ERROR_E = 3,

    /** @brief Packets with control word first nibble > 1
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E = 4,

    /** @brief PASSENGER Pseudowire Tag0 not found error is detected if <Redirect command>
     *  field is CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E and when <PW Tag Mode> =
     *  TAGGED PW Mode inner packet outer tag (VID0) is the P-Tag, which should
     *  always exist; however tag0 not found.
     *  (APPLICABLE DEVICES: xCat3)
     */
    CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E = 5

} CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT;

/**
* @enum CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT
 *
 * @brief TTI key TCAM segment modes
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
 * Note: Bobcat3; Aldrin2 does not support it ! feature was removed.
 * and behavior is as 'CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E'
 * APIs that get this parameter will ignore it.
 * (see PRV_CPSS_DXCH_BOBCAT3_RM_TCAM_SEGMENT_MODE_OBSOLETE_WA_E)
*/
typedef enum{

    /** @brief All the TTI TCAM block sets are treated as a single logical TCAM, and return a single
     *  action associated with the first match in the combined TCAM block sets.
     *  Relevant for devices that support dual lookup or quad lookup
     */
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ONE_TCAM_E = 0,

    /** @brief The TTI TCAM block set 0 is treated as a single logical TCAM, and returns an action
     *  associated with the first match in TCAM block set 0.
     *  - The TTI TCAM block set 1 is treated as a single logical TCAM, and returns an action
     *  associated with the first match in TCAM block set 1. This action is only applied if
     *  the previously assigned action <Continue to Next TTI Lookup> is set.
     *  Relevant only for devices that support dual lookup
     */
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E = 1,

    /** @brief The TTI TCAM block sets 0 and 1 are treated as a single logical TCAM and returns an
     *  action associated with the first match in the combined TCAM block sets 0 and 1.
     *  - The TTI TCAM block sets 2 and 3 are treated as a single logical TCAM and returns an
     *  action associated with the first match in the combined TCAM block sets 2 and 3. This
     *  action is only applied if the previously applied action <Continue to Next TTI Lookup>
     *  is set.
     *  Relevant only for devices that support quad lookup
     */
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_2_AND_2_E = 2,

    /** @brief The TTI TCAM block sets 0 is treated as a single logical TCAM and returns an action
     *  associated with the first match in the TCAM block set 0.
     *  - The TTI TCAM block sets 1, 2, and 3 are treated as a single logical TCAM and returns
     *  an action associated with the first match in the combined TCAM block sets 1, 2, and 3.
     *  This action is only applied if the previously applied action <Continue to Next TTI Lookup>
     *  is set.
     *  Relevant only for devices that support quad lookup
     */
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_1_AND_3_E = 3,

    /** @brief The TTI TCAM block sets 0, 1, and 2 are treated as a single logical TCAM and returns
     *  an action associated with the first match in the combined TCAM block sets 0, 1, and 2.
     *  - The TTI TCAM block sets 3 is treated as a single logical TCAM and returns an action
     *  associated with the first match in the TCAM block set 3. This action is only applied
     *  if the previously applied action <Continue to Next TTI Lookup> is set.
     *  Relevant only for devices that support quad lookup
     */
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_3_AND_1_E = 4,

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
    CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E = 5

} CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT;


/**
* @enum CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT
 *
 * @brief Enumeration for tunnel termination FCOE exceptions.
*/
typedef enum{

    /** @brief FCoE Ver Error exception is detected if the <Ver> field in the
     *  FCoE header is not equal to 0.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_VER_E              = 0,

    /** @brief FCoE SOF Error exception is detected if the SOF field is invalid.
     *  The valid values for the SOF field are (0x28, 0x2d, 0x35, 0x2E, 0x36).
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_SOF_E              = 1,

    /** @brief FCoE R_CTR Error exception is detected if the parser has found more than
     *  3 FC extended headers.
     *  The header parser can detect and skip up to 3 extended headers, allowing
     *  to parse the FC header which immediately follows the extended headers.
     *  In this context an extended header is either a VFT or an IFT.
     *  If more than 3 extended headers are present, the parser cannot parse
     *  the FC header, and this exception is triggered.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_R_CTL_E            = 2,

    /** @brief FCoE Header Length Error exception is detected if the packets D_ID and
     *  S_ID fields expand beyond the first 128B of the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_HEADER_LENGTH_E    = 3,

    /** @brief FCoE S_ID Error exception is detected if the S_ID is equal to 0xFFFFFF.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_S_ID_E             = 4,

    /** @brief FCoE MC Error exception is detected if bit 40 of the MAC DA is set,
     *  indicating a multi-destination packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_MC_E               = 5,

    /** @brief FCoE VFT Error exception is detected if the packet has a VFT and the
     *  <Ver> field of packets VFT is non-zero.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_VER_E          = 6,

    /** @brief FCoE VFT Type Error exception is detected if the packet has a VFT and the
     *  <Type> field of packets VFT is non-zero.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_TYPE_E         = 7,

    /** @brief FCoE VFT HOP CNT Error exception is detected if the packet has a VFT and
     *  the <HopCnt> field of packets VFT is equal to 1.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E -
     */
    CPSS_DXCH_TTI_FCOE_EXCEPTION_VFT_HOP_CNT_E      = 8

} CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTTiTypesh */

