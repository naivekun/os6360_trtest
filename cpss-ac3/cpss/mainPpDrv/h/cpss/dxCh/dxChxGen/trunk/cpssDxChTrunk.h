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
* @file cpssDxChTrunk.h
*
* @brief API definitions for 802.3ad Link Aggregation (Trunk) facility
* CPSS - DxCh
*
* "max number of trunks" - see description in cpssDxChTrunkInit(...)
*
* States and Modes :
* A. Application can work in one of 2 modes:
* 1. use "high level" trunk manipulations set of APIs - Mode "HL"
* 2. use "low level" trunk HW tables/registers set of APIs - MODE "LL"
*
* B. Using "high level" trunk manipulations set of APIs - "High level"
* Mode.
* In this mode the CPSS synchronize all the relevant trunk
* tables/registers, implement some WA for trunks Errata.
* 1. Set trunk members (enabled,disabled)
* 2. Add/remove member to/from trunk
* 3. Enable/disable member in trunk
* 4. Add/Remove port to/from "non-trunk" entry
* 5. set/unset designated member for Multi-destination traffic and
* traffic sent to Cascade Trunks
* 6. Set cascade trunk local ports
* 7. Trunk ECMP LTT table (on applicable devices)
* C. Using "low level" trunk HW tables/registers set of APIs- "Low level"
* Mode.
* In this mode the CPSS allows direct access to trunk tables/registers.
* In this mode the Application required to implement the
* synchronization between the trunk tables/registers and to implement
* the WA to some of the trunk Errata.
*
* 1. Set per port the trunkId
* 2. Set per trunk the trunk members , number of members
* 3. Set per trunk the Non-trunk local ports bitmap
* 4. Set per Index the designated local ports bitmap
* 5. Set per trunk the Trunk ECMP LTT entry (on applicable devices)
*
* D. Application responsibility is not to use a mix of using API from
* those 2 modes
* The only time that the Application can shift between the 2 modes,
* is only when there are no trunks in the device.
*
* @version   32
********************************************************************************
*/

#ifndef __cpssDxChTrunkh
#define __cpssDxChTrunkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/trunk/cpssGenTrunkTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>

/**
* @enum CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT
 *
 * @brief enumerator that hold values for the type of how the CPSS
 * SW will fill the HW with trunk members .
*/
typedef enum{

    /** @brief the trunk members are filled
     *  according to the order given by application.
     *  Regular trunk may hold max of 8 members.
     *  Cascade trunk may hold :
     *  max of 8 members -> (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2)
     *  max of 64 members -> (APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E = 0,

    /** @brief the trunk members are
     *  filled in order to emulate the load balance that the
     *  SOHO devices use.
     *  The DX106/7 needs it to work with the SOHO devices.
     */
    CPSS_DXCH_TRUNK_MEMBERS_MODE_SOHO_EMULATION_E,

    /** @brief A mode to allows flexibility for
     *  each Regular trunk to state it's max number of members (before starting to add members).
     *  (this mode not effect 'cascade trunk' members)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Regular trunk may hold : max of 4K members. (each trunk set it's own limit)
     *  Cascade trunk may hold : max of 64 members.
     *  1. Choosing this mode allows the application to state per trunk the
     *  max number of members that will be used (each trunk can be with
     *  different max number of members)
     *  The application must call next API : cpssDxChTrunkFlexInfoSet(...)
     *  before calling the APIs that manage trunk members :
     *  cpssDxChTrunkTableEntryGet(...)
     *  cpssDxChTrunkMembersSet(...)
     *  cpssDxChTrunkMemberAdd(...)
     *  cpssDxChTrunkMemberRemove(...)
     *  cpssDxChTrunkMemberDisable(...)
     *  cpssDxChTrunkMemberEnable(...)
     *  2. Choosing this mode also impact next 2 APIs:
     *  a. cpssDxChBrgL2EcmpTableSet(...) :
     *  The CPSS will allows application full access to all 'L2 ECMP' table entries
     *  (in mode of CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E :
     *  the application is limited to 1/2 the table (lower half))
     *  b. cpssDxChBrgL2EcmpLttTableSet(...) :
     *  The CPSS will allows application full manage:
     *  to the 'ePort ECMP lookup translation' table
     *  (in mode of CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E :
     *  the application is limited to point to the lower half of the 'L2 ECMP' table)
     *  NOTE: for both APIs the CPSS will not check for 'collisions' with 'trunk settings'
     *  3. Choosing this mode also impact 'designated member table' :
     *  As this table is limited to 64 members (64 for UC , and separate 64 for MC),
     *  only the first 64 members of the 'flex' trunk will be represented
     *  in the 'designated member table'.
     *  So only those first 64 ports can 'flood' in vlan (single one per 'flood').
     *  Expectations from Application:
     *  1. Not to 'modify' entries in 'L2 ECMP' table those were 'given' to the
     *  CPSS for trunk management.
     *  2. Not to point from 'ePort ECMP lookup translation' to 'L2 ECMP'
     *  table to those were 'given' to the CPSS for trunk management.
     */
    CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E

} CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT;



/**
* @enum CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT
 *
 * @brief Enumeration of general LBH (load balance hash) Mode.
 * the hashing will be according to use of bits from packet, or
 * according to the ingress port number.
*/
typedef enum{

    /** Hash (XOR) is based on the packets data. */
    CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E,

    /** @brief Hash (XOR) is based on the packet's
     *  Source Port# or Source Trunk#.
     *  On Cascading ports, the Source Port#
     *  or Source Trunk# is assigned from the
     *  DSA tag.
     */
    CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E,

    /** @brief CRC Hash based on the packet's data.
     *  Note :
     *  1. Applicable for Lion and above.
     *  2. Using this value will return GT_BAD_PARAM
     *  for devices that not support the feature.
     */
    CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E

} CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT;



/**
* @enum CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT
 *
 * @brief Enumeration of IPv6 Hash Mode.
*/
typedef enum{

    /** Use LSB of SIP, DIP and Flow Label */
    CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E = 0,

    /** Use MSB of SIP, DIP and Flow Label */
    CPSS_DXCH_TRUNK_IPV6_HASH_MSB_SIP_DIP_FLOW_E,

    /** @brief Use MSB and LSB of SIP,
     *  DIP and Flow Label
     */
    CPSS_DXCH_TRUNK_IPV6_HASH_MSB_LSB_SIP_DIP_FLOW_E,

    /** Use LSB of SIP, DIP */
    CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_E

} CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT;

/**
* @enum CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT
 *
 * @brief Enumeration of L4 LBH (load balance hash) Mode.
 * the use of bits from TCP/UDP ports (src,dst ports) to generate
 * trunk member index selection.
*/
typedef enum{

    /** L4 lbh disabled */
    CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E,

    /** @brief use long L4 Hash Mode.
     *  L4 Trunk hash function is based on bits [5:0]
     *  and [13:8] of the UDP/TCP destination and
     *  source ports.
     */
    CPSS_DXCH_TRUNK_L4_LBH_LONG_E,

    /** @brief use short L4 Hash Mode .
     *  L4 Trunk hash function is based on bits [5:0]
     *  of the UDP/TCP destination and source ports.
     */
    CPSS_DXCH_TRUNK_L4_LBH_SHORT_E

} CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT;


/**
* @enum CPSS_DXCH_TRUNK_LBH_MASK_ENT
 *
 * @brief Enumeration of trunk LBH (load balance hash) relate fields to
 * apply mask .
*/
typedef enum{

    /** mask for MAC DA */
    CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E,

    /** mask for MAC SA */
    CPSS_DXCH_TRUNK_LBH_MASK_MAC_SA_E,

    /** mask for MPLS label 0 */
    CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E,

    /** mask for MPLS label 1 */
    CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL1_E,

    /** mask for MPLS label 2 */
    CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL2_E,

    /** mask for IPv4 DIP */
    CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E,

    /** mask for IPv4 SIP */
    CPSS_DXCH_TRUNK_LBH_MASK_IPV4_SIP_E,

    /** mask for IPv6 DIP */
    CPSS_DXCH_TRUNK_LBH_MASK_IPV6_DIP_E,

    /** mask for IPv6 SIP */
    CPSS_DXCH_TRUNK_LBH_MASK_IPV6_SIP_E,

    /** mask for IPv6 flow */
    CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E,

    /** mask for L4 DST port */
    CPSS_DXCH_TRUNK_LBH_MASK_L4_DST_PORT_E,

    /** mask for L4 SRC port */
    CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E

} CPSS_DXCH_TRUNK_LBH_MASK_ENT;


/**
* @enum CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT
 *
 * @brief enumerator for CRC LBH (load balance hash) mode.
*/
typedef enum{

    /** @brief CRC use 'CRC 6' ,
     *  polynomial: x^6+ x + 1 .
     *  (NOTE: results 6 bits hash value)
     */
    CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E,

    /** @brief CRC use 'CRC 16' and Pearson hash ,
     *  polynomial: x^16 + x^15 + x^2 + 1 .
     *  (NOTE: results 6 bits hash value)
     */
    CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E,

    /** @brief CRC use 'CRC 32' ,
     *  polynomial: x3^2+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
     *  (NOTE: results 32 bits hash value)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E

} CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT;



/**
* @enum CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT
 *
 * @brief An enumeration for the hash clients.
*/
typedef enum{

    /** @brief trunk member selection
     *  for singledestination packets (trunk to physical port).
     */
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,

    /** @brief The ePort ECMP client
     *  uses the ingress hash value in order to select the egress interface.
     *  The ePort ECMP client uses the ingress hash to select one of the
     *  egress interfaces that can be an ePort or a trunk.
     */
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,

    /** @brief The L3 ECMP client uses
     *  the ingress hash value to select the egress interface when L3 ECMP
     *  is used. The router engine selects the next hop which can be an ECMP
     *  group. In such a case, the ingress hash is used to select a member
     *  from the ECMP group.
     */
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E,

    /** @brief trunk member selection
     *  for multidestination packets.
     */
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E,

    /** Ingress PCL. */
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E,

    /** Ingress OAM. */
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E,

    /** Egress OAM. */
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E,

    /** must be last. */
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE___LAST___E

} CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT;


/**
* @struct CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC
 *
 * @brief structure of entry of 'CRC hash mask' table .
 * === related to feature 'CRC hash mode' ===
 * in those fields each bit in the bitmap represents a BYTE to be added to hash.
 * for fields from the packet : the bitmap represent in bit 0 the MSB and in
 * last bit the LSB (network order bytes)
 * for UDBs : the bmp represent the UDBs (bit 14 --> UDB 14 , bit 22 UDB 22)
 * NOTE: the actual write to the HW will be in 'Little Endian' order (which is
 * reverse to the 'Network order' for fields from the packet)
 * For L4 dst port the bmp is of 2 bits , because 2 bytes in L4 port
 * using bit 0 to represent MSB , bit 1 the LSB (network order bytes)
 * For mpls label 0 (and mpls label 1,mpls label 2) the bmp of 3 bits (for 3 bytes
 * from label - actually 2.5 MSB)
 * using bit 0 to represent MSB , bit 1 the second byte ,bit 2 the third byte (LSB) (network order bytes)
 * For IP_SIP the bmp is of 16 bits , because 128 bits (16 bytes) in ipv6
 * using bit 0 to represent IP[0] , bit 1 represent IP[1] ..., bit 15 represent IP[15] (network order bytes)
 * the bits for ipv4 addresses (4 bits) located in bits 12..15 (MSB..LSB) (network order bytes)
 * For UDBS the bmp is of 32 bits , because 32 UDBs
 * using bit 0 to represent UDB 0 , bit 1 represent UDB 1 ..., bit 31 represent UDB 31
 * Lion supports only UDBs 14..22 (bits 14..22 in the bitmap)
 * For Local source port the bmp is of single bit (bit 0)
*/
typedef struct{

    /** L4 DST port mask bmp ( 2 bits for 2 bytes */
    GT_U32 l4DstPortMaskBmp;

    /** L4 SRC port mask bmp ( 2 bits for 2 bytes */
    GT_U32 l4SrcPortMaskBmp;

    /** IPv6 flow  mask bmp ( 3 bits for 3 bytes */
    GT_U32 ipv6FlowMaskBmp;

    /** IPv6/v4 DIP mask bmp (16 bits for 16 bytes */
    GT_U32 ipDipMaskBmp;

    /** IPv6/v4 SIP mask bmp (16 bits for 16 bytes */
    GT_U32 ipSipMaskBmp;

    /** MAC DA   mask bmp ( 6 bits for 6 bytes */
    GT_U32 macDaMaskBmp;

    /** MAC SA   mask bmp ( 6 bits for 6 bytes */
    GT_U32 macSaMaskBmp;

    /** MPLS label 0 mask bmp ( 3 bits for 3 bytes */
    GT_U32 mplsLabel0MaskBmp;

    /** MPLS label 1 mask bmp ( 3 bits for 3 bytes */
    GT_U32 mplsLabel1MaskBmp;

    /** MPLS label 2 mask bmp ( 3 bits for 3 bytes */
    GT_U32 mplsLabel2MaskBmp;

    /** local source port mask bmp (1 bit for 1 byte) */
    GT_U32 localSrcPortMaskBmp;

    /** @brief UDBs mask bmp (user defined bytes) (32 bits for 32 UDBs)
     *  UDBs 14..22 are only supported.
     *  (Configure UD0-UDB11 mask in ipDipMaskBmp and UDB23-UDB34 in ipSipMaskBmp.)
     *  end of fields as 'bitmap of mask'
     */
    GT_U32 udbsMaskBmp;

    /** @brief Enable/Disable 'symmetric MAC address' in hash calculations.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL symmetricMacAddrEnable;

    /** @brief Enable/Disable 'symmetric IPv4 address' in hash calculations.(NOTE: 4 LSBytes in 16 bytes IP)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL symmetricIpv4AddrEnable;

    /** @brief Enable/Disable 'symmetric IPv6 address' in hash calculations.(NOTE: 12 MSBytes in 16 bytes IP)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL symmetricIpv6AddrEnable;

    /** @brief Enable/Disable 'symmetric L4 port' in hash calculations.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL symmetricL4PortEnable;

} CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC;

/**
* @struct CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC
 *
 * @brief structure of HASH input fields .
 * NOTE: 1. all BYTEs in field in this structure correlate to the 'bits' in
 * the similar fields in structure CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC
 * 2. all fields that represents 'packet bytes' must be in 'network order'
 * 3. field ipv6FlowArray[3] represents 20 bits :
 * ipv6FlowArray[0] hold 4 MS bits
 * ipv6FlowArray[1] hold next 8 bits
 * ipv6FlowArray[2] hold last LS 8 bits
 * 4. fields mplsLabel0Array[3] ,mplsLabel1Array[3] ,mplsLabel2Array[3] represents 20 bits (each):
 * mplsLabel0Array[0] hold 4 MS bits
 * mplsLabel0Array[1] hold next 8 bits
 * mplsLabel0Array[2] hold last LS 8 bits
 * 5. fields ipDipArray[16] , ipSipArray[16] supports:
 * IPv6 dip/sip in bytes 0..15
 * IPv4 dip/sip in bytes 12..15 (bytes 0..11 are zero)
 * FCoe dip/sip in bytes 13..15 (bytes 0..12 are zero)
*/
typedef struct{

    GT_U8 l4DstPortArray[2];

    GT_U8 l4SrcPortArray[2];

    GT_U8 ipv6FlowArray[3];

    GT_U8 ipDipArray[16];

    GT_U8 ipSipArray[16];

    GT_U8 macDaArray[6];

    GT_U8 macSaArray[6];

    GT_U8 mplsLabel0Array[3];

    GT_U8 mplsLabel1Array[3];

    GT_U8 mplsLabel2Array[3];

    GT_U8 localSrcPortArray[1];

    GT_U8 udbsArray[32];

} CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC;



/**
* @enum CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT
 *
 * @brief enumeration for how to access the designated trunk table.
*/
typedef enum{

    /** @brief use the computed
     *  trunk hash that was calculated in ingress.
     */
    CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E = GT_TRUE,

    /** @brief use the computed
     *  trunk hash that was calculated in ingress , and
     *  XOR it with the vid.
     */
    CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E,

    /** @brief The index is based on
     *  the original source port/trunk, VID and VIDX.
     *  If the original source interface is a trunks,
     *  the 6 LSb of the trunk-id are used.
     */
    CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E

} CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT;

/**
* @enum CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT
 *
 * @brief Enumeration of the 'Source port' hash mode
*/
typedef enum{

    /** @brief hash mode
     *  calculation using %8 (modulo).
     */
    CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_MODULO_8_E,

    /** @brief hash mode
     *  calculation using %64 (modulo).
     */
    CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_MODULO_64_E

} CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT;

/**
* @struct CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC
 *
 * @brief structure for the relation between source port and a port in trunk ,
 * to specify trunk designated table for 'source port' hashing.
 * the port in trunk (trunkPort) will receive the traffic destined to the trunk
 * when sent from the source port (srcPort).
*/
typedef struct{

    /** @brief source port , a port which traffic that ingress from it ,and should
     *  egress from the trunk(that trunkPort is member in), will choose
     *  srcPort sent to this trunk.
     */
    GT_PHYSICAL_PORT_NUM srcPort;

    /** @brief as the egress port.
     *  trunkPort - 'port in trunk' that need to represent the trunk when traffic from
     */
    GT_PHYSICAL_PORT_NUM trunkPort;

} CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC;


/**
* @enum CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT
 *
 * @brief enumeration for cases that build of the CRC 70 bytes that used
 * to generate the CRC hash.
*/
typedef enum{

    /** the flow id can be used instead of UDB21 UDB22 */
    CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E,

    /** @brief the eVLAN can be used instead of MPLS label 0.
     *  NOTE: for non-MPLS the eVLAN is used anyway.
     */
    CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL0_BY_EVLAN_E,

    /** @brief the original source ePort can be used instead of MPLS label 1.
     *  NOTE: for non-MPLS the original source ePort is used anyway.
     */
    CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL1_BY_ORIGINAL_SOURCE_EPORT_E,

    /** @brief the local device source ePort can be used instead of MPLS label 2.
     *  NOTE: for non-MPLS the local device source ePort is used anyway.
     */
    CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL2_BY_LOCAL_SOURCE_EPORT_E,

    /** @brief the UDB23 UDB34 (12 UDBs) can be used instead of 12 MSB of the SIP IPv6 address,
     *  the UDB0 - UDB11 (12 UDBs) can be used instead of 12 MSB of the DIP IPv6 address,
     *  meaning that only 4 LSB of the IPv6 address are still used by SIP,DIP.
     *  NOTE: for non-IPV6 the 24 UDBs always used
     */
    CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E

} CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT;


/**
* @struct CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC
 *
 * @brief TRUNK ECMP LTT Entry.
 * Relevant mode : Low level mode
*/
typedef struct{

    /** @brief Determines the start index of the L2 ECMP block.
     *  (APPLICABLE RANGES: 0..8191)
     */
    GT_U32 ecmpStartIndex;

    /** @brief The number of paths in the ECMP block:
     *  0x1 = 1 path, 0x2 = 2 paths and so on.
     *  (APPLICABLE RANGES: 1..4096)
     */
    GT_U32 ecmpNumOfPaths;

    /** @brief Determines whether this trunk is enabled for load balance.
     *  GT_FALSE - traffic destined to this trunk will be discarded.
     *  GT_TRUE - traffic destined to this trunk will load balance
     *  between it's members.
     */
    GT_BOOL ecmpEnable;

    /** allows random load balance */
    GT_BOOL ecmpRandomPathEnable;

} CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC;


/**
* @internal cpssDxChTrunkDbMembersSortingEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable/disable 'sorting' of trunk members in the
*         'trunk members table' and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         'sorting enabled' : when the application will add/remove/set members in a trunk
*         cpss will make sure to set the trunk members into the
*         device in an order that is not affected by the 'history'
*         of 'add/remove' members to/from the trunk.
*         'sorting disabled' : (legacy mode / default mode) when the application
*         will add/remove/set members in a trunk cpss will set
*         the trunk members into the device in an order that is
*         affected by the 'history' of 'add/remove' members
*         to/from the trunk.
*         function uses the DB (no HW operations)
*         The API start to effect only on trunks that do operations (add/remove/set members)
*         after the API call.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable the sorting
*                                      GT_TRUE : enabled , GT_FALSE - disabled
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbMembersSortingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChTrunkDbMembersSortingEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Gets the enable/disable 'sorting' of trunk members in the 'trunk members table'
*         and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) the sorting is enabled/disabled
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbMembersSortingEnableGet
(
    IN GT_U8                    devNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChTrunkPortTrunkIdSet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunkId field in the port's control register in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] memberOfTrunk            - is the port associated with the trunk
*                                      GT_FALSE - the port is set as "not member" in the trunk
*                                      GT_TRUE  - the port is set with the trunkId
* @param[in] trunkId                  - the trunk to which the port associate with
*                                      This field indicates the trunk group number (ID) to which the
*                                      port is a member.
*                                      1 through "max number of trunks" = The port is a member of the trunk
*                                      this value relevant only when memberOfTrunk = GT_TRUE
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or
*                                       bad trunkId number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The trunkId configuration should be done after the configuration of the
*       ePort to portNum in the translation table.
*       In case of a change in the translation table this API should be called
*       again, since the configuration done here will not be correct any more.
*
*/
GT_STATUS cpssDxChTrunkPortTrunkIdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  memberOfTrunk,
    IN GT_TRUNK_ID              trunkId
);

/**
* @internal cpssDxChTrunkPortTrunkIdGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Set the trunkId field in the port's control register in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] memberOfTrunkPtr         - (pointer to) is the port associated with the trunk
*                                      GT_FALSE - the port is set as "not member" in the trunk
*                                      GT_TRUE  - the port is set with the trunkId
* @param[out] trunkIdPtr               - (pointer to)the trunk to which the port associate with
*                                      This field indicates the trunk group number (ID) to which the
*                                      port is a member.
*                                      1 through "max number of trunks" = The port is a member of the trunk
*                                      this value relevant only when (memberOfTrunkPtr) = GT_TRUE
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunkId value is not synchronized in the 2 registers
*                                       that should hold the same value
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkPortTrunkIdGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *memberOfTrunkPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
);

/**
* @internal cpssDxChTrunkTableEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunk table entry , and set the number of members in it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
* @param[in] numMembers               - num of enabled members in the trunk
*                                      Note : value 0 is not recommended.
* @param[in] membersArray[]           - array of enabled members of the trunk
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - numMembers exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  GT_U32                  numMembers,
    IN  CPSS_TRUNK_MEMBER_STC   membersArray[]
);

/**
* @internal cpssDxChTrunkTableEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the trunk table entry , and get the number of members in it.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id
*
* @param[out] numMembersPtr            - (pointer to)num of members in the trunk
* @param[out] membersArray[]           - array of enabled members of the trunk
*                                      array is allocated by the caller , it is assumed that
*                                      the array can hold :
*                                      In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*                                      the max number of member that was set for this trunk.
*                                      Otherwise
*                                      CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS members
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_NOT_INITIALIZED       - 1. The trunk library was not initialized for the device
*                                       2. In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*                                       the 'maxNumOfMembers' for this trunk is 0
*                                       (cpssDxChTrunkFlexInfoSet(...) was not called ,
*                                       or was called with maxNumOfMembers = 0)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkTableEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT GT_U32                  *numMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   membersArray[]
);


/**
* @internal cpssDxChTrunkNonTrunkPortsEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunk's non-trunk ports specific bitmap entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkNonTrunkPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    IN  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
);


/**
* @internal cpssDxChTrunkNonTrunkPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the trunk's non-trunk ports bitmap specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - trunk id - in this API  can be ZERO !
*
* @param[out] nonTrunkPortsPtr         - (pointer to) non trunk port bitmap of the trunk.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkNonTrunkPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    OUT  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
);

/**
* @internal cpssDxChTrunkDesignatedPortsEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] entryIndex               - the index in the designated ports bitmap table
* @param[in] designatedPortsPtr       - (pointer to) designated ports bitmap
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDesignatedPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    IN  CPSS_PORTS_BMP_STC  *designatedPortsPtr
);


/**
* @internal cpssDxChTrunkDesignatedPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] entryIndex               - the index in the designated ports bitmap table
*
* @param[out] designatedPortsPtr       - (pointer to) designated ports bitmap
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
* @retval the index must be in range (0 - 7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDesignatedPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    OUT  CPSS_PORTS_BMP_STC  *designatedPortsPtr
);

/**
* @internal cpssDxChTrunkHashIpModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the IP SIP/DIP information,
*         when calculation the trunk hashing index for a packet.
*         Relevant when the is IPv4 or IPv6 and <TrunkHash Mode> = 0.
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - GT_FALSE - IP data is not added to the trunk load balancing
*                                      hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash, if the packet is IPv6.
*                                      IPTrunkHash = according to setting of API
*                                      cpssDxChTrunkHashIpv6ModeSet(...)
*                                      else packet is IPv4:
*                                      IPTrunkHash = SIP[5:0]^DIP[5:0]^SIP[21:16]^DIP[21:16].
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpModeSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChTrunkHashIpModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         get the enable/disable of device from considering the IP SIP/DIP
*         information, when calculation the trunk hashing index for a packet.
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                -(pointer to)
*                                      GT_FALSE - IP data is not added to the trunk load balancing
*                                      hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash, if the packet is IPv6.
*                                      IPTrunkHash = according to setting of API
*                                      cpssDxChTrunkHashIpv6ModeSet(...)
*                                      else packet is IPv4:
*                                      IPTrunkHash = SIP[5:0]^DIP[5:0]^SIP[21:16]^DIP[21:16].
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpModeGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChTrunkHashL4ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the L4 TCP/UDP
*         source/destination port information, when calculation the trunk hashing
*         index for a packet.
*         Note:
*         1. Not relevant to NON TCP/UDP packets.
*         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
*         setting not considered.
*         3. Not relevant to multi-destination packets (include routed IPM).
*         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashL4ModeSet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT hashMode
);

/**
* @internal cpssDxChTrunkHashL4ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         get the Enable/Disable of device from considering the L4 TCP/UDP
*         source/destination port information, when calculation the trunk hashing
*         index for a packet.
*         Note:
*         1. Not relevant to NON TCP/UDP packets.
*         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
*         setting not considered.
*         3. Not relevant to multi-destination packets (include routed IPM).
*         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashL4ModeGet
(
    IN GT_U8   devNum,
    OUT CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT *hashModePtr
);

/**
* @internal cpssDxChTrunkHashIpv6ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpv6ModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   hashMode
);

/**
* @internal cpssDxChTrunkHashIpv6ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpv6ModeGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   *hashModePtr
);

/**
* @internal cpssDxChTrunkHashIpAddMacModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - enable/disable feature
*                                      GT_FALSE - If the packet is an IP packet MAC data is not
*                                      added to the Trunk load balancing hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash:
*                                      MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0].
*                                      NOTE: When the packet is not an IP packet and
*                                      <TrunkLBH Mode> = 0, the trunk load balancing
*                                      hash = MACTrunkHash, regardless of this setting.
*                                      If the packet is IPv4/6-over-X tunnel-terminated,
*                                      the mode is always GT_FALSE (since there is no
*                                      passenger packet MAC header).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpAddMacModeSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChTrunkHashIpAddMacModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                - (pointer to)enable/disable feature
*                                      GT_FALSE - If the packet is an IP packet MAC data is not
*                                      added to the Trunk load balancing hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash:
*                                      MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0].
*                                      NOTE: When the packet is not an IP packet and
*                                      <TrunkLBH Mode> = 0, the trunk load balancing
*                                      hash = MACTrunkHash, regardless of this setting.
*                                      If the packet is IPv4/6-over-X tunnel-terminated,
*                                      the mode is always GT_FALSE (since there is no
*                                      passenger packet MAC header).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpAddMacModeGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChTrunkHashGlobalModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the general hashing mode of trunk hash generation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] hashMode                 - hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashGlobalModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode
);

/**
* @internal cpssDxChTrunkHashGlobalModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the general hashing mode of trunk hash generation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashGlobalModeGet
(
    IN GT_U8    devNum,
    OUT CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  *hashModePtr
);

/**
* @internal cpssDxChTrunkHashDesignatedTableModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash mode of the designated trunk table .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] mode                     - The designated table hashing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashDesignatedTableModeSet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT    mode
);

/**
* @internal cpssDxChTrunkHashDesignatedTableModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the hash mode of the designated trunk table .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] modePtr                  - (pointer to) The designated table hashing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashDesignatedTableModeGet
(
    IN GT_U8   devNum,
    OUT CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT *modePtr
);

/**
* @internal cpssDxChTrunkHashNumBitsSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Set the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBit                 - start bit (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - number of bits (APPLICABLE RANGES: 1..16)
*                                      NOTE: For TRUNK_DESIGNATED_TABLE, IPCL, IOAM, EOAM
* @param[in] numOfBits                must not exceed value 6.
*                                      For Ingress pipe clients (IPCL, IOAM, TRUNK,
*                                      L3ECMP, L2ECMP, TRUNK_DESIGNATED_TABLE)
* @param[in] startBit                 + numOfBits) must not exceed value 32.
*                                      For egress pipe clients (EOAM)
* @param[in] startBit                 + numOfBits) must not exceed value 12.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - startBit or numOfBits not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note startBit and numOfBits values may also be changed by:
*       - cpssDxChBrgL2EcmpMemberSelectionModeSet for L2 ECMP client to (0 6),
*       (6 6), (0 12) depending on member selection mode.
*       - cpssDxChIpEcmpHashNumBitsSet for L3 ECMP client to any value passed
*       to the function.
*       - cpssDxChTrunkMemberSelectionModeSet for Trunk ECMP client to (0 6),
*       (6 6), (0 12) depending on member selection mode.
*       - cpssDxChTrunkHashGlobalModeSet for Trunk ECMP and L2 ECMP clients to
*       (0, 6) or if CRC_32_MODE then use (0,12).
*       - cpssDxChTrunkInit for Trunk ECMP and L2 ECMP clients to (0, 6).
*       - cpssDxChTrunkHashCrcParametersSet for Trunk ECMP and L2 ECMP clients
*       to (0, 6) or if CRC_32_MODE then use (0,12).
*
*/
GT_STATUS cpssDxChTrunkHashNumBitsSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    IN GT_U32                               startBit,
    IN GT_U32                               numOfBits
);

/**
* @internal cpssDxChTrunkHashNumBitsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Get the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBitPtr              - (pointer to) start bit
* @param[in] numOfBitsPtr             - (pointer to) number of bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - NULL pointer to startBitPtr or numOfBitsPtr
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - startBit or numOfBits not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashNumBitsGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    OUT GT_U32                               *startBitPtr,
    OUT GT_U32                               *numOfBitsPtr
);

/**
* @internal cpssDxChTrunkDbEnabledMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         return the enabled members of the trunk
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in,out] numOfEnabledMembersPtr   - (pointer to) max num of enabled members to
*                                      retrieve - this value refer to the number of
*                                      members that the array of enabledMembersArray[]
*                                      can retrieve.
* @param[in,out] numOfEnabledMembersPtr   - (pointer to) the actual num of enabled members
*                                      in the trunk (up to CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS)
*
* @param[out] enabledMembersArray[]    - (array of) enabled members of the trunk
*                                      array was allocated by the caller
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbEnabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfEnabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   enabledMembersArray[]
);

/**
* @internal cpssDxChTrunkDbDisabledMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         return the disabled members of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in,out] numOfDisabledMembersPtr  - (pointer to) max num of disabled members to
*                                      retrieve - this value refer to the number of
*                                      members that the array of enabledMembersArray[]
*                                      can retrieve.
* @param[in,out] numOfDisabledMembersPtr  -(pointer to) the actual num of disabled members
*                                      in the trunk (up to CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS)
*
* @param[out] disabledMembersArray[]   - (array of) disabled members of the trunk
*                                      array was allocated by the caller
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbDisabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfDisabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   disabledMembersArray[]
);

/**
* @internal cpssDxChTrunkDbIsMemberOfTrunk function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Checks if a member (device,port) is a trunk member.
*         if it is trunk member the function retrieve the trunkId of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] memberPtr                - (pointer to) the member to check if is trunk member
*
* @param[out] trunkIdPtr               - (pointer to) trunk id of the port .
*                                      this pointer allocated by the caller.
*                                      this can be NULL pointer if the caller not require the
*                                      trunkId(only wanted to know that the member belongs to a
*                                      trunk)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_FOUND             - the pair (devNum,port) not a trunk member
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
);

/**
* @internal cpssDxChTrunkDbTrunkTypeGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Get the trunk type.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] trunkId                  - the trunk id.
*
* @param[out] typePtr                  - (pointer to) the trunk type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbTrunkTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT CPSS_TRUNK_TYPE_ENT     *typePtr
);

/**
* @internal cpssDxChTrunkDesignatedMemberSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function Configures per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         Setting value replace previously assigned designated member.
*         NOTE that:
*         under normal operation this should not be used with cascade Trunks,
*         due to the fact that in this case it affects all types of traffic -
*         not just Multi-destination as in regular Trunks.
*         Diagram 1 : Regular operation - Traffic distribution on all enabled
*         members (when no specific designated member defined)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Diagram 2: Traffic distribution once specific designated member defined
*         (M3 in this case - which is currently enabled member in trunk)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
* @param[in] enable                   - enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
* @param[in] enable                   at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[in] memberPtr                - (pointer to)the designated member we set to the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDesignatedMemberSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal cpssDxChTrunkDbDesignatedMemberGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function get Configuration per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
*                                      enabled-member at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[out] memberPtr                - (pointer to) the designated member of the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbDesignatedMemberGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_TRUNK_MEMBER_STC   *memberPtr
);

/**
* @internal cpssDxChTrunkMembersSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function set the trunk with the specified enable and disabled
*         members.
*         this setting override the previous setting of the trunk members.
*         the user can "invalidate/unset" trunk entry by setting :
*         numOfEnabledMembers = 0 and numOfDisabledMembers = 0
*         This function support next "set entry" options :
*         1. "reset" the entry
*         function will remove the previous settings
*         2. set entry after the entry was empty
*         function will set new settings
*         3. set entry with the same members that it is already hold
*         function will rewrite the HW entries as it was
*         4. set entry with different setting then previous setting
*         a. function will remove the previous settings
*         b. function will set new settings
*         Notes about designated trunk table:
*         If (no designated defined)
*         distribute MC/Cascade trunk traffic among the enabled members
*         else
*         1. Set all member ports bits with 0
*         2. If designated member is one of the enabled members, set its relevant
*         bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as disabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of disabled members in the array.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - 1. The trunk library was not initialized for the device
*                                       2. In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*                                       the 'maxNumOfMembers' for this trunk is 0
*                                       (cpssDxChTrunkFlexInfoSet(...) was not called ,
*                                       or was called with maxNumOfMembers = 0)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when the sum of number of enabled members + number of
*                                       disabled members exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
);

/**
* @internal cpssDxChTrunkCascadeTrunkPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the specified 'Local ports'
*         overriding any previous setting.
*         The cascade trunk may be invalidated/unset by portsMembersPtr = NULL.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] portsMembersPtr          - (pointer to) local ports bitmap to be members of the
*                                      cascade trunk.
*                                      NULL - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      not-NULL - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are ports in the bitmap that not supported by
*                                       the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or number
*                                       of ports (in the bitmap) larger then the number of
*                                       entries in the 'Designated trunk table'
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports are 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkPortsSet  |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id ,         | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
GT_STATUS cpssDxChTrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
);

/**
* @internal cpssDxChTrunkCascadeTrunkPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Gets the 'Local ports' of the 'cascade' trunk .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
*
* @param[out] portsMembersPtr          - (pointer to) local ports bitmap of the cascade trunk.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkCascadeTrunkPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT CPSS_PORTS_BMP_STC       *portsMembersPtr
);

/**
* @internal cpssDxChTrunkCascadeTrunkWithWeightedPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the Weighted specified 'Local ports'
*         overriding any previous setting.
*         the weights effect the number of times that each member will get representation
*         in the 'designated table' .
*         The cascade trunk may be invalidated by numOfMembers = 0.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] numOfMembers             - number of members in the array.
*                                      value 0 - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      value != 0 - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
* @param[in] weightedMembersArray[]   - (array of) members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are members that not supported by the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or bad total
*                                       weights (see restrictions below)
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_BAD_PTR               - when numOfMembers != 0 and weightedMembersArray = NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports as 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members according to their weight
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       6. the total weights of all the trunk members is restricted to :
*       a. must be equal to power of 2 (1,2,4,8,16,32,64...)
*       b. must not be larger then actual designated table size of the device.
*       (Lion ,Lion2 : 64 , other devices : 8)
*       c. not relevant when single member exists
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkWithWeightedPortsSet |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id          | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
GT_STATUS cpssDxChTrunkCascadeTrunkWithWeightedPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);



/**
* @internal cpssDxChTrunkCascadeTrunkWithWeightedPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         This function gets the Weighted 'Local ports' of 'cascade' trunk .
*         the weights reflect the number of times that each member is represented
*         in the 'designated table'
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in,out] numOfMembersPtr          - (pointer to) max num of members to retrieve - this value refer to the number of
*                                      members that the array of weightedMembersArray[] can retrieve.
*                                      OUTPUTS:
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
* @param[in] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
*
* @param[out] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*                                      OUTPUTS:
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkCascadeTrunkWithWeightedPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);


/**
* @internal cpssDxChTrunkMemberAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function add member to the trunk in the device.
*         If member is already in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members,
*         now taking into account also the added member
*         else
*         1. If added member is not the designated member - set its relevant bits to 0
*         2. If added member is the designated member & it's enabled,
*         set its relevant bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to add member to the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to add to the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - 1. The trunk library was not initialized for the device
*                                       2. In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*                                       the 'maxNumOfMembers' for this trunk is 0
*                                       (cpssDxChTrunkFlexInfoSet(...) was not called ,
*                                       or was called with maxNumOfMembers = 0)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_FULL                  - trunk already contains maximum supported members
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal cpssDxChTrunkMemberRemove function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function remove member from a trunk in the device.
*         If member not exists in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the removed member
*         else
*         1. If removed member is not the designated member - nothing to do
*         2. If removed member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to remove member from the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to remove from the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - 1. The trunk library was not initialized for the device
*                                       2. In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*                                       the 'maxNumOfMembers' for this trunk is 0
*                                       (cpssDxChTrunkFlexInfoSet(...) was not called ,
*                                       or was called with maxNumOfMembers = 0)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal cpssDxChTrunkMemberDisable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function disable (enabled)existing member of trunk in the device.
*         If member is already disabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the disabled member
*         else
*         1. If disabled member is not the designated member - set its relevant bits to 0
*         2. If disabled member is the designated member set its relevant bits
*         on all indexes to 0.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to disable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to disable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - 1. The trunk library was not initialized for the device
*                                       2. In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*                                       the 'maxNumOfMembers' for this trunk is 0
*                                       (cpssDxChTrunkFlexInfoSet(...) was not called ,
*                                       or was called with maxNumOfMembers = 0)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberDisable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal cpssDxChTrunkMemberEnable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable (disabled)existing member of trunk in the device.
*         If member is already enabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the enabled member
*         else
*         1. If enabled member is not the designated member - set its relevant bits to 0
*         2. If enabled member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to enable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to enable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - 1. The trunk library was not initialized for the device
*                                       2. In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*                                       the 'maxNumOfMembers' for this trunk is 0
*                                       (cpssDxChTrunkFlexInfoSet(...) was not called ,
*                                       or was called with maxNumOfMembers = 0)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberEnable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal cpssDxChTrunkNonTrunkPortsAdd function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         add the ports to the trunk's non-trunk entry .
*         NOTE : the ports are add to the "non trunk" table only and not effect
*         other trunk relate tables/registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsBmpPtr      - (pointer to)bitmap of ports to add to
*                                      "non-trunk members"
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkNonTrunkPortsAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
);

/**
* @internal cpssDxChTrunkNonTrunkPortsRemove function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         Removes the ports from the trunk's non-trunk entry .
*         NOTE : the ports are removed from the "non trunk" table only and not
*         effect other trunk relate tables/registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsBmpPtr      - (pointer to)bitmap of ports to remove from
*                                      "non-trunk members"
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkNonTrunkPortsRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
);

/**
* @internal cpssDxChTrunkInit function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         CPSS DxCh Trunk initialization of PP Tables/registers and
*         SW shadow data structures, all ports are set as non-trunk ports.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum       - device number
*         maxNumberOfTrunks - maximum number of trunk groups.
*         When this number is 0 , there will be no shadow used (API's that used shadow will FAIL.)
*         APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         APPLICABLE RANGES: 0..127
*         in mode : CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E :
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         APPLICABLE RANGES: 0..4095
*         else
*         APPLICABLE DEVICES: Bobcat2.
*         APPLICABLE RANGES: 0..511
*         APPLICABLE DEVICES: Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         APPLICABLE RANGES: 0..255
*         APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maxNumberOfTrunks        - maximum number of trunk groups.
*                                      When this number is 0 , there will be no shadow used (API's that used shadow will FAIL.)
*                                      APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*                                      APPLICABLE RANGES: 0..127
*                                      in mode : CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E :
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*                                      APPLICABLE RANGES: 0..4095
*                                      else
*                                      APPLICABLE DEVICES: Bobcat2.
*                                      APPLICABLE RANGES: 0..511
*                                      APPLICABLE DEVICES: Caelum; Aldrin; AC3X; Bobcat3.
*                                      APPLICABLE RANGES: 0..255
*                                      APPLICABLE DEVICES: Bobcat3.
*                                      APPLICABLE RANGES: 0..255
*                                      NOTE: 1. for exact number of trunks supported by specific device ,
*                                      check the addendum to the functional specification document for the device.
*                                      2. The API's that used shadow will support only trunkId in range 1..(maxNumberOfTrunks-1)
* @param[in] trunkMembersMode         - type of how the CPSS SW will fill the HW with
*                                      trunk members
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong devNum or bad trunkLbhMode
* @retval GT_OUT_OF_RANGE          - the numberOfTrunks > "max number of trunks"
* @retval GT_BAD_STATE             - if library already initialized with different
*                                       number of trunks than requested
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkInit
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxNumberOfTrunks,
    IN  CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT trunkMembersMode
);

/**
* @internal cpssDxChTrunkDbInitInfoGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the parameters that given during cpssDxChTrunkInit(...) .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maxNumberOfTrunksPtr     - (pointer to) maximum number of trunk groups.
*                                      if NULL - ignored
* @param[out] trunkMembersModePtr      - (pointer to) type of how the CPSS SW will fill the HW with
*                                      trunk members
*                                      if NULL - ignored
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbInitInfoGet
(
    IN   GT_U8       devNum,
    OUT  GT_U32      *maxNumberOfTrunksPtr,
    OUT  CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT *trunkMembersModePtr
);


/**
* @internal cpssDxChTrunkHashMplsModeEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the MPLS information,
*         when calculating the trunk hashing index for a packet.
*         Note:
*         1. Not relevant to NON-MPLS packets.
*         2. Relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E)
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] enable                   - GT_FALSE - MPLS parameter are not used in trunk hash index
*                                      GT_TRUE  - The following function is added to the trunk load
*                                      balancing hash:
*                                      MPLSTrunkHash = (mpls_label0[5:0] & mpls_trunk_lbl0_mask) ^
*                                      (mpls_label1[5:0] & mpls_trunk_lbl1_mask) ^
*                                      (mpls_label2[5:0] & mpls_trunk_lbl2_mask)
*                                      NOTE:
*                                      If any of MPLS Labels 0:2 do not exist in the packet,
*                                      the default value 0x0 is used for TrunkHash calculation
*                                      instead.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashMplsModeEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChTrunkHashMplsModeEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get trunk MPLS hash mode
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
*
* @param[out] enablePtr                - (pointer to)the MPLS hash mode.
*                                      GT_FALSE - MPLS parameter are not used in trunk hash index
*                                      GT_TRUE  - The following function is added to the trunk load
*                                      balancing hash:
*                                      MPLSTrunkHash = (mpls_label0[5:0] & mpls_trunk_lbl0_mask) ^
*                                      (mpls_label1[5:0] & mpls_trunk_lbl1_mask) ^
*                                      (mpls_label2[5:0] & mpls_trunk_lbl2_mask)
*                                      NOTE:
*                                      If any of MPLS Labels 0:2 do not exist in the packet,
*                                      the default value 0x0 is used for TrunkHash calculation
*                                      instead.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashMplsModeEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChTrunkHashMaskSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the masks for the various packet fields being used at the Trunk
*         hash calculations
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] maskedField              - field to apply the mask on
* @param[in] maskValue                - The mask value to be used (APPLICABLE RANGES: 0..0x3F)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or maskedField
* @retval GT_OUT_OF_RANGE          - maskValue > 0x3F
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashMaskSet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    IN GT_U8                          maskValue
);

/**
* @internal cpssDxChTrunkHashMaskGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the masks for the various packet fields being used at the Trunk
*         hash calculations
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] maskedField              - field to apply the mask on
*
* @param[out] maskValuePtr             - (pointer to)The mask value to be used (APPLICABLE RANGES: 0..0x3F)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or maskedField
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashMaskGet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    OUT GT_U8                         *maskValuePtr
);

/**
* @internal cpssDxChTrunkHashIpShiftSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the shift being done to IP addresses prior to hash calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
* @param[in] shiftValue               - The shift to be done.
*                                      IPv4 valid shift: (APPLICABLE RANGES: 0..3) bytes (Value = 0: no shift).
*                                      IPv6 valid shift: (APPLICABLE RANGES: 0..15) bytes (Value = 0: no shift).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or protocolStack
* @retval GT_OUT_OF_RANGE          - shiftValue > 3 for IPv4 , shiftValue > 15 for IPv6
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpShiftSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    IN GT_U32                       shiftValue
);

/**
* @internal cpssDxChTrunkHashIpShiftGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the shift being done to IP addresses prior to hash calculations.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
*
* @param[out] shiftValuePtr            - (pointer to) The shift to be done.
*                                      IPv4 valid shift: (APPLICABLE RANGES: 0..3) bytes (Value = 0: no shift).
*                                      IPv6 valid shift: (APPLICABLE RANGES: 0..15) bytes (Value = 0: no shift).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or protocolStack
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkHashIpShiftGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    OUT GT_U32                      *shiftValuePtr
);

/**
* @internal cpssDxChTrunkHashCrcParametersSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] crcMode                  - The CRC mode .
* @param[in] crcSeed                  - The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed (APPLICABLE RANGES: 0..0x3f) (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed (APPLICABLE RANGES: 0..0xffff) (16 bits value)
*                                      when crcMode is CRC_32 mode : crcSeed (32 bits value)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or crcMode
* @retval GT_OUT_OF_RANGE          - crcSeed out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashCrcParametersSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     crcMode,
    IN GT_U32                               crcSeed
);

/**
* @internal cpssDxChTrunkHashCrcParametersGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
*
* @param[out] crcModePtr               - (pointer to) The CRC mode .
* @param[out] crcSeedPtr               - (pointer to) The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed  (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed (16 bits value)
*                                      when crcMode is CRC_32 mode : crcSeed (32 bits value)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashCrcParametersGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     *crcModePtr,
    OUT GT_U32                               *crcSeedPtr
);


/**
* @internal cpssDxChTrunkPortHashMaskInfoSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set port-based hash mask info.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] portNum                  - The port number.
* @param[in] overrideEnable           - enable/disable the override
* @param[in] index                    - the  to use when 'Override enabled'.
*                                      (APPLICABLE RANGES: 0..15) , relevant only when overrideEnable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or portNum
* @retval GT_OUT_OF_RANGE          - when overrideEnable is enabled and index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkPortHashMaskInfoSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      overrideEnable,
    IN GT_U32       index
);

/**
* @internal cpssDxChTrunkPortHashMaskInfoGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get port-based hash mask info.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The device number.
* @param[in] portNum                  - The port number.
*
* @param[out] overrideEnablePtr        - (pointer to)enable/disable the override
* @param[out] indexPtr                 - (pointer to)the index to use when 'Override enabled'.
*                                      (APPLICABLE RANGES: 0..15) , relevant only when overrideEnable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or portNum
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkPortHashMaskInfoGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *overrideEnablePtr,
    OUT GT_U32      *indexPtr
);

/**
* @internal cpssDxChTrunkHashMaskCrcEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the entry of masks in the specified index in 'CRC hash mask table'.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table index.
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..27).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon 0..31).
* @param[in] entryPtr                 - (pointer to) The entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields in entryPtr are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashMaskCrcEntrySet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
);

/**
* @internal cpssDxChTrunkHashMaskCrcEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the entry of masks in the specified index in 'CRC hash mask table'.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table index.
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..27).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon 0..31).
*
* @param[out] entryPtr                 - (pointer to) The entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashMaskCrcEntryGet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    OUT CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
);



/**
* @internal cpssDxChTrunkHashIndexCalculate function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Calculate hash index of member load-balancing group for given hash client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] packetType               - packet type.
* @param[in] hashInputDataPtr         - (pointer to) hash input data.
*                                      hashClientType     - client that uses HASH.
*                                      (APPLICABLE VALUES: CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,
*                                      CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,
*                                      CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E)
* @param[in] numberOfMembers          - number of members in load-balancing group.
*
* @param[out] hashIndexPtr             - (pointer to) calculated by means of ingress
*                                      hash index of load-balancing group member.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is relevant when <Random Enable> flag = GT_FALSE.
*       It determines whether the load balancing is based on the
*       ingress hash or on a 16-bit pseudo-random.
*       The only supported hash mode is Enhanced CRC-based hash mode.
*
*/
GT_STATUS cpssDxChTrunkHashIndexCalculate
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT                packetType,
    IN CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC           *hashInputDataPtr,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT         hashEntityType,
    IN GT_U32                                       numberOfMembers,
    OUT GT_U32                                      *hashIndexPtr
);

/**
* @internal cpssDxChTrunkHashPearsonValueSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the Pearson hash value for the specific index.
*         NOTE: the Pearson hash used when CRC-16 mode is used.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table  (APPLICABLE RANGES: 0..63)
* @param[in] value                    - the Pearson hash  (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - value > 63
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashPearsonValueSet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN GT_U32                         value
);

/**
* @internal cpssDxChTrunkHashPearsonValueGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the Pearson hash value for the specific index.
*         NOTE: the Pearson hash used when CRC-16 mode is used.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - the device number.
* @param[in] index                    - the table  (APPLICABLE RANGES: 0..63)
*
* @param[out] valuePtr                 - (pointer to) the Pearson hash value (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashPearsonValueGet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    OUT GT_U32                        *valuePtr
);

/**
* @internal cpssDxChTrunkMcLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Enable/Disable sending multi-destination packets back to its source
*         trunk on the local device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - multi-destination packets may be sent back to
*                                      their source trunk on the local device.
*                                      GT_FALSE - multi-destination packets are not sent back to
*                                      their source trunk on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. the behavior of multi-destination traffic ingress from trunk is
*       not-affected by setting of cpssDxChBrgVlanLocalSwitchingEnableSet
*       and not-affected by setting of cpssDxChBrgPortEgressMcastLocalEnable
*       2. the functionality manipulates the 'non-trunk' table entry of the trunkId
*
*/
GT_STATUS cpssDxChTrunkMcLocalSwitchingEnableSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChTrunkDbMcLocalSwitchingEnableGet function
* @endinternal
*
* @brief   Get the current status of Enable/Disable sending multi-destination packets
*         back to its source trunk on the local device.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) Boolean value:
*                                      GT_TRUE  - multi-destination packets may be sent back to
*                                      their source trunk on the local device.
*                                      GT_FALSE - multi-destination packets are not sent back to
*                                      their source trunk on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbMcLocalSwitchingEnableGet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_BOOL          *enablePtr
);


/**
* @internal cpssDxChTrunkMemberSelectionModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Determines the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The selection mode configuration may be changed by:
*       - cpssDxChTrunkHashNumBitsSet.
*       - cpssDxChTrunkHashGlobalModeSet.
*       - cpssDxChTrunkInit.
*       - cpssDxChTrunkHashCrcParametersSet.
*
*/
GT_STATUS cpssDxChTrunkMemberSelectionModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_MEMBER_SELECTION_MODE_ENT  selectionMode
);

/**
* @internal cpssDxChTrunkMemberSelectionModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Return the number of bits used to calculate the Index of the trunk member.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to) member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkMemberSelectionModeGet
(
    IN  GT_U8                                 devNum,
    OUT CPSS_DXCH_MEMBER_SELECTION_MODE_ENT   *selectionModePtr
);


/**
* @internal cpssDxChTrunkLearnPrioritySet function
* @endinternal
*
* @brief   Set Learn priority per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] learnPriority            - Learn priority can be Low or High
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbLearnPrioritySet
*
*/
GT_STATUS cpssDxChTrunkLearnPrioritySet
(
    IN GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    IN CPSS_DXCH_FDB_LEARN_PRIORITY_ENT     learnPriority
);

/**
* @internal cpssDxChTrunkLearnPriorityGet function
* @endinternal
*
* @brief   Get Learn priority per trunk ID
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
*
* @param[out] learnPriorityPtr         - (pointer to)Learn priority can be Low or High
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbLearnPriorityGet
*
*/
GT_STATUS cpssDxChTrunkLearnPriorityGet
(
    IN  GT_U8                               devNum,
    IN  GT_TRUNK_ID                         trunkId,
    OUT CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    *learnPriorityPtr
);

/**
* @internal cpssDxChTrunkUserGroupSet function
* @endinternal
*
* @brief   Set User Group per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] userGroup                - user group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbUserGroupSet
*
*/
GT_STATUS cpssDxChTrunkUserGroupSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           userGroup
);

/**
* @internal cpssDxChTrunkUserGroupGet function
* @endinternal
*
* @brief   Get User Group per trunk ID, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
*
* @param[out] userGroupPtr             - (pointer to) user group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for port configuration: cpssDxChBrgFdbUserGroupGet
*
*/
GT_STATUS cpssDxChTrunkUserGroupGet
(
    IN  GT_U8           devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_U32          *userGroupPtr
);

/**
* @internal cpssDxChTrunkHashCrcSaltByteSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the CRC Salt value for specific byte .
*         relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E , CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] byteIndex                - the byte index one of the 70 bytes (APPLICABLE RANGES: 0..69)
* @param[in] saltValue                - the salt value of the byte (8 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or byteIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC 32 hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashCrcSaltByteSet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    IN GT_U8    saltValue
);

/**
* @internal cpssDxChTrunkHashCrcSaltByteGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC Salt value for specific byte .
*         relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E , CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] byteIndex                - the byte index one of the 70 bytes (APPLICABLE RANGES: 0..69)
*
* @param[out] saltValuePtr             - (pointer to)the salt value of the byte (8 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or byteIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC 32 hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashCrcSaltByteGet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    OUT GT_U8    *saltValuePtr
);


/**
* @internal cpssDxChTrunkHashMaskCrcParamOverrideSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set which fields of the CRC hash input bytes are override by other fields.
*         for example : option to use FlowID instead of UDB21,UDB22.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] fieldType                - the field type that used to override.
* @param[in] override                 -  the original field.
*                                      GT_TRUE  - override.
*                                      GT_FALSE - not override.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or fieldType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashMaskCrcParamOverrideSet
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT fieldType,
    IN GT_BOOL                                          override
);

/**
* @internal cpssDxChTrunkHashMaskCrcParamOverrideGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get which fields of the CRC hash input bytes are override by other fields.
*         for example : option to use FlowID instead of UDB21,UDB22.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] fieldType                - the field type that used to override.
*
* @param[out] overridePtr              - (pointer to)override the original field.
*                                      GT_TRUE  - override.
*                                      GT_FALSE - not override.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or fieldType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS cpssDxChTrunkHashMaskCrcParamOverrideGet
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT fieldType,
    OUT GT_BOOL                                          *overridePtr
);

/**
* @internal cpssDxChTrunkPortMcEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the mode how multicast destination traffic egress the
*         selected port while it is a trunk member.
*         see details in description of CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_E
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] mode                     - the 'designated mc table'  of the port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device or port number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If disabled multicast destination packets would not egress through the
*       configured port.
*       This configuration has no influence on unicast destination traffic or
*       when the port is not a trunk member.
*
*/
GT_STATUS cpssDxChTrunkPortMcEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT mode
);

/**
* @internal cpssDxChTrunkDbPortMcEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function gets the mode how multicast destination traffic egress the
*         selected port while it is a trunk member.
*         see details in description of CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] modePtr                  - (pointer to) the 'designated mc table' mode of the port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function output is retrieved from CPSS related trunk DB.
*       This configuration has no influence on unicast destination traffic or
*       when the port is not a trunk member.
*
*/
GT_STATUS cpssDxChTrunkDbPortMcEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChTrunkDbPortTrunkIdModeSet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function allows application to be responsible for the 'TrunkId of the port'.
*         in High level mode APIs the 'TrunkId of the port' is set internally be
*         the CPSS and may override the current value of the 'TrunkId of the port'.
*         This API allows application to notify the CPSS to not manage the 'TrunkId
*         of the port'.
*         function cpssDxChTrunkPortTrunkIdSet(...) allows application to manage
*         the trunkId of the port
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] manageMode               - the management mode.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or manageMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbPortTrunkIdModeSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT manageMode
);

/**
* @internal cpssDxChTrunkDbPortTrunkIdModeGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function get the 'management mode' of the port's trunkId.
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] manageModePtr            - (pointer to) the management mode.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or manageMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDbPortTrunkIdModeGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT *manageModePtr
);

/**
* @internal cpssDxChTrunkFlexInfoSet function
* @endinternal
*
* @brief   In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' state per 'regular' trunk
*         the 'start index' in the 'L2 ECMP' table , and the number of consecutive
*         entries that will be reserved for this trunk(in 'L2 ECMP').
*         NOTE:
*         1. See more details about this mode in description of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
*         2. This API is NOT intend to 'move' trunks from one range in the 'L2 ECMP'
*         to another range in this table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
*                                      NOTE: value 0xFFFF means that ALL trunks of this device need to use
*                                      the same 'max number' of members.
*                                      1. 'l2EcmpStartIndex' is ignored.
*                                      2. 'maxNumOfMembers' is applied to ALL trunks
*                                      3. the CPSS internally assign ALL trunks with consecutive
*                                      'L2 ECMP' indexes from end of the table towards
*                                      the start of the table.
* @param[in] l2EcmpStartIndex         - The 'start index' in the 'L2 ECMP' table.
*                                      (APPLICABLE RANGES: 0..('L2 ECMP' table size - 1))
*                                      parameter ignored when trunkId == 0xFFFF
* @param[in] maxNumOfMembers          - 1. The number of consecutive entries that will be reserved
*                                      for this trunk(in 'L2 ECMP')
*                                      2. The trunk will not be able to hold more members
*                                      than this value.
*                                      (APPLICABLE RANGES: 0..4096)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_OUT_OF_RANGE          - l2EcmpStartIndex is out of range , or maxNumOfMembers is out of range
* @retval GT_BAD_STATE             - 1. mode is not CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E
* @retval 2. the trunk             -type is CPSS_TRUNK_TYPE_REGULAR_E and
*                                       l2EcmpStartIndex is different from already exists or
*                                       maxNumOfMembers is less than current HW value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For 'better' load balance on 'small number' of flows , the CPSS will
*       set <Number Of L2 ECMP Paths> in the LTT with value in the range of
*       'actual number of members' ... maxNumOfMembers.
*       2. if the trunk already used as 'regular' trunk (hold members), the application
*       can increase 'maxNumOfMembers' or decrease if not less than current HW value.
*       (need to keep l2EcmpStartIndex as already exist)
*
*/
GT_STATUS cpssDxChTrunkFlexInfoSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           l2EcmpStartIndex,
    IN GT_U32           maxNumOfMembers
);

/**
* @internal cpssDxChTrunkDbFlexInfoGet function
* @endinternal
*
* @brief   Get per trunk the 'start index' in the 'L2 ECMP' table , and the number of consecutive
*         entries reserved for this trunk(in 'L2 ECMP').
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
*
* @param[out] l2EcmpStartIndexPtr      - (pointer to) The 'start index' in the 'L2 ECMP' table.
* @param[out] maxNumOfMembersPtr       - (pointer to) The number of consecutive entries reserved
*                                      for this trunk(in 'L2 ECMP')
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
*                                       or the trunk was not set with such info.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Function retrieve from DB (database) info that was set in cpssDxChTrunkFlexInfoSet()
*       2. function will return 'not initialized' for trunk that was not set via
*       cpssDxChTrunkFlexInfoSet()
*
*/
GT_STATUS cpssDxChTrunkDbFlexInfoGet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_U32          *l2EcmpStartIndexPtr,
    OUT GT_U32          *maxNumOfMembersPtr
);

/**
* @internal cpssDxChTrunkEcmpLttTableSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level mode
*         DESCRIPTION:
*         Set Trunk ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the  (LTT index)
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] ecmpLttInfoPtr           - (pointer to) ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range ecmpStartIndex or ecmpNumOfPaths
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkEcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    IN CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
);

/**
* @internal cpssDxChTrunkEcmpLttTableGet function
* @endinternal
*
* @brief   Get Trunk ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the  (LTT index)
*                                      (APPLICABLE RANGES: 0..4095)
*
* @param[out] ecmpLttInfoPtr           - (pointer to) ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkEcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    OUT CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
);

/**
* @internal cpssDxChTrunkDestroy function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         CPSS DxCh Trunk Destroy all relevant DB info.
*         No HW modifications are done.
*         Purpose :
*         Debug tool to allow to call cpssDxChTrunkInit(...) again with any new valid parameters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTrunkDestroy
(
    IN  GT_U8                                devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTrunkh */


