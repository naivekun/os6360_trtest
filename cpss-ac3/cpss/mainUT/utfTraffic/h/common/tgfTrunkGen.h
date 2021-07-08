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
* @file tgfTrunkGen.h
*
* @brief Generic API for Trunk
*
* @version   21
********************************************************************************
*/
#ifndef __tgfTrunkGenh
#define __tgfTrunkGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#include <common/tgfPclGen.h>
#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/trunk/cpssExMxPmTrunk.h>
#endif /* EXMXPM_FAMILY */

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT
 *
 * @brief Enumeration of general LBH (load balance hash) Mode.
 * the hashing will be according to use of bits from packet, or
 * according to the ingress port number.
*/
typedef enum{

    /** Hash is based on the packets data. */
    PRV_TGF_TRUNK_LBH_PACKETS_INFO_E,

    /** @brief Hash is based on the packet's
     *  Source Port# or Source Trunk#.
     *  On Cascading ports, the Source Port#
     *  or Source Trunk# is assigned from the
     *  DSA tag.
     */
    PRV_TGF_TRUNK_LBH_INGRESS_PORT_E,

    /** @brief Hash index is cyclic on the trunk members
     *  (per trunk).
     *  (Hash index is reset to 0 when number of
     *  members change)
     *  This assure a "perfect" load balance
     *  distribution between trunk members.
     *  NOTE:
     *  Meaning that if there are 3 members in
     *  the trunk ,
     *  -then first packet destined to this trunk
     *  will egress from first port in trunk.
     *  -the second packet destined to this trunk
     *  will egress from second port in trunk.
     *  -the third packet destined to this trunk
     *  will egress from third port in trunk.
     *  -the forth packet destined to this trunk
     *  will forth from first port in trunk.
     *  -the fifth packet destined to this trunk
     *  will forth from second port in trunk.
     *  ...
     *  -the (X) packet destined to this trunk
     *  will forth from (X%Y) port in trunk.
     *  where X - is the packet number
     *  Y - is number of ports in trunk
     *  relevant for EXMXPM devices only.
     */
    PRV_TGF_TRUNK_LBH_PERFECT_E,

    /** @brief CRC Hash based on the packet's data.
     *  Note :
     *  1. Applicable for Lion and above.
     *  2. Using this value will return GT_BAD_PARAM
     *  for devices that not support the feature.
     */
    PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E

} PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT;

/**
* @enum PRV_TGF_TRUNK_LBH_MASK_ENT
 *
 * @brief Enumeration of trunk LBH (load balance hash) relate fields to
 * apply mask .
*/
typedef enum{

    /** mask for MAC DA */
    PRV_TGF_TRUNK_LBH_MASK_MAC_DA_E,

    /** mask for MAC SA */
    PRV_TGF_TRUNK_LBH_MASK_MAC_SA_E,

    /** mask for MPLS label 0 */
    PRV_TGF_TRUNK_LBH_MASK_MPLS_LABEL0_E,

    /** mask for MPLS label 1 */
    PRV_TGF_TRUNK_LBH_MASK_MPLS_LABEL1_E,

    /** mask for MPLS label 2 */
    PRV_TGF_TRUNK_LBH_MASK_MPLS_LABEL2_E,

    /** mask for IPv4 DIP */
    PRV_TGF_TRUNK_LBH_MASK_IPV4_DIP_E,

    /** mask for IPv4 SIP */
    PRV_TGF_TRUNK_LBH_MASK_IPV4_SIP_E,

    /** mask for IPv6 DIP */
    PRV_TGF_TRUNK_LBH_MASK_IPV6_DIP_E,

    /** mask for IPv6 SIP */
    PRV_TGF_TRUNK_LBH_MASK_IPV6_SIP_E,

    /** mask for IPv6 flow */
    PRV_TGF_TRUNK_LBH_MASK_IPV6_FLOW_E,

    /** mask for L4 DST port */
    PRV_TGF_TRUNK_LBH_MASK_L4_DST_PORT_E,

    /** mask for L4 SRC port */
    PRV_TGF_TRUNK_LBH_MASK_L4_SRC_PORT_E

} PRV_TGF_TRUNK_LBH_MASK_ENT;

/**
* @enum PRV_TGF_TRUNK_LBH_CRC_MODE_ENT
 *
 * @brief enumerator for CRC LBH (load balance hash) mode.
*/
typedef enum{

    /** @brief CRC use 'CRC 6' ,
     *  polynomial: x^6+ x + 1 .
     */
    PRV_TGF_TRUNK_LBH_CRC_6_MODE_E = 6,

    /** @brief CRC use 'CRC 16' and Pearson hash ,
     *  polynomial: x^16 + x^15 + x^2 + 1 .
     */
    PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E,

    /** @brief CRC use 'CRC 32' ,
     *  polynomial: x3^2+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
     *  (NOTE: results 32 bits hash value)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_TRUNK_LBH_CRC_32_MODE_E

} PRV_TGF_TRUNK_LBH_CRC_MODE_ENT;

/*
 * typedef: structure PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC
 *
 * Description: structure of entry of 'CRC hash mask' table .
 *      === related to feature 'CRC hash mode' ===
 *
 *  in those fields each bit in the bitmap represents a BYTE to be added to hash.
 *
 *  for fields from the packet : the bitmap represent in bit 0 the MSB and in
 *      last bit the LSB (network order bytes)
 *  for UDBs : the bmp represent the UDBs (bit 14 --> UDB 14 , bit 22 UDB 22)
 *
 *  NOTE: the actual write to the HW will be in 'Little Endian' order (which is
 *         reverse to the 'Network order' for fields from the packet)
 *
 *  For L4 dst port the bmp is of 2 bits , because 2 bytes in L4 port
 *      using bit 0 to represent MSB , bit 1 the LSB (network order bytes)
 *  For mpls label 0 (and mpls label 1,mpls label 2) the bmp of 3 bits (for 3 bytes
 *      from label - actually 2.5 MSB)
 *      using bit 0 to represent MSB , bit 1 the second byte ,bit 2 the third byte (LSB) (network order bytes)
 *  For IP_SIP the bmp is of 16 bits , because 128 bits (16 bytes) in ipv6
 *      using bit 0 to represent IP[0] , bit 1 represent IP[1] ..., bit 15 represent IP[15] (network order bytes)
 *      the bits for ipv4 addresses (4 bits) located in bits 12..15 (MSB..LSB) (network order bytes)
 *  For UDBS the bmp is of 32 bits , because 32 UDBs
 *      using bit 0 to represent UDB 0 , bit 1 represent UDB 1 ..., bit 31 represent UDB 31
 *              Lion supports only UDBs 14..22 (bits 14..22 in the bitmap)
 *  For Local source port the bmp is of single bit (bit 0)
 *
 * fields:
 *    l4DstPortMaskBmp  - L4 DST port mask bmp  ( 2 bits for  2 bytes-'Network order')
 *    l4SrcPortMaskBmp  - L4 SRC port mask bmp  ( 2 bits for  2 bytes-'Network order')
 *    ipv6FlowMaskBmp   - IPv6 flow   mask bmp  ( 3 bits for  3 bytes-'Network order')
 *    ipDipMaskBmp      - IPv6/v4 DIP mask bmp  (16 bits for 16 bytes-'Network order')
 *    ipSipMaskBmp      - IPv6/v4 SIP mask bmp  (16 bits for 16 bytes-'Network order')
 *    macDaMaskBmp      - MAC DA      mask bmp  ( 6 bits for  6 bytes-'Network order')
 *    macSaMaskBmp      - MAC SA      mask bmp  ( 6 bits for  6 bytes-'Network order')
 *    mplsLabel0MaskBmp - MPLS label 0 mask bmp ( 3 bits for  3 bytes-'Network order')
 *    mplsLabel1MaskBmp - MPLS label 1 mask bmp ( 3 bits for  3 bytes-'Network order')
 *    mplsLabel2MaskBmp - MPLS label 2 mask bmp ( 3 bits for  3 bytes-'Network order')
 *    localSrcPortMaskBmp - local source port mask bmp (1 bit for 1 byte)
 *    udbsMaskBmp         - UDBs mask bmp (user defined bytes) (32 bits for 32 UDBs)
 *                          lion supports only UDBs 14..22
 *
 *      end of fields as 'bitmap of mask'
 *
 ************************
 *
 *     symmetricMacAddrEnable  - Enable/Disable 'symmetric mac addr' in hash calculations.
 *           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *     symmetricIpv4AddrEnable - Enable/Disable 'symmetric IPv4 addr' in hash calculations.(NOTE: 4  LSBytes in 16 bytes IP)
 *           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *     symmetricIpv6AddrEnable - Enable/Disable 'symmetric IPv6 addr' in hash calculations.(NOTE: 12 MSBytes in 16 bytes IP)
 *           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *     symmetricL4PortEnable   - Enable/Disable 'symmetric L4 port' in hash calculations.
 *           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *
 */
typedef struct {
    GT_U32      l4DstPortMaskBmp;
    GT_U32      l4SrcPortMaskBmp;
    GT_U32      ipv6FlowMaskBmp;
    GT_U32      ipDipMaskBmp;
    GT_U32      ipSipMaskBmp;
    GT_U32      macDaMaskBmp;
    GT_U32      macSaMaskBmp;
    GT_U32      mplsLabel0MaskBmp;
    GT_U32      mplsLabel1MaskBmp;
    GT_U32      mplsLabel2MaskBmp;
    GT_U32      localSrcPortMaskBmp;
    GT_U32      udbsMaskBmp;

    GT_BOOL     symmetricMacAddrEnable;
    GT_BOOL     symmetricIpv4AddrEnable;
    GT_BOOL     symmetricIpv6AddrEnable;
    GT_BOOL     symmetricL4PortEnable;

}PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC ;



/**
* @struct PRV_TGF_TRUNK_LBH_INPUT_DATA_STC
 *
 * @brief structure of HASH input fields .
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

} PRV_TGF_TRUNK_LBH_INPUT_DATA_STC;


/**
* @enum PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT
 *
 * @brief An enumeration for hash clients.
*/
typedef enum{

    /** trunk (trunk to physical port). */
    PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,

    /** @brief The ePort ECMP client uses the ingress hash value in order to select the egress
     *  interface. The ePort ECMP client uses the ingress hash to select one of the egress
     *  interfaces that can be an ePort or a trunk.
     */
    PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,

    /** @brief The L3 ECMP client uses the ingress hash value to select the egress interface
     *  when L3 ECMP is used. The router engine selects the next hop which can be an
     *  ECMP group. In such a case, the ingress hash is used to select a member from
     *  the ECMP group.
     */
    PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E,

    /** trunk member selection for multi-destination packets. */
    PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E,

    /** Ingress PCL. */
    PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E,

    /** Ingress OAM. */
    PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E,

    /** Egress OAM. */
    PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E,

    /** must be last */
    PRV_TGF_TRUNK_HASH_CLIENT_TYPE___LAST___E

} PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT;

/**
* @enum PRV_TGF_TRUNK_DESIGNATED_TABLE_MODE_ENT
 *
 * @brief enumeration for how to access the designated trunk table.
*/
typedef enum{

    /** @brief use the computed
     *  trunk hash that was calculated in ingress.
     */
    PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E = GT_TRUE,

    /** @brief use the computed
     *  trunk hash that was calculated in ingress , and
     *  XOR it with the vid.
     */
    PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E,

    /** @brief The index is based on
     *  the original source port/trunk, VID and VIDX.
     *  If the original source interface is a trunks,
     *  the 6 LSb of the trunk-id are used.
     */
    PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E

} PRV_TGF_TRUNK_DESIGNATED_TABLE_MODE_ENT;

/**
* @enum PRV_TGF_TRUNK_IPV6_HASH_MODE_ENT
 *
 * @brief Enumeration of IPv6 Hash Mode.
*/
typedef enum{

    /** Use LSB of SIP, DIP and Flow Label */
    PRV_TGF_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E = 0,

    /** Use MSB of SIP, DIP and Flow Label */
    PRV_TGF_TRUNK_IPV6_HASH_MSB_SIP_DIP_FLOW_E,

    /** @brief Use MSB and LSB of SIP,
     *  DIP and Flow Label
     */
    PRV_TGF_TRUNK_IPV6_HASH_MSB_LSB_SIP_DIP_FLOW_E,

    /** Use LSB of SIP, DIP */
    PRV_TGF_TRUNK_IPV6_HASH_LSB_SIP_DIP_E

} PRV_TGF_TRUNK_IPV6_HASH_MODE_ENT;

/**
* @enum PRV_TGF_TRUNK_L4_LBH_MODE_ENT
 *
 * @brief Enumeration of L4 LBH (load balance hash) Mode.
 * the use of bits from TCP/UDP ports (src,dst ports) to generate
 * trunk member index selection.
*/
typedef enum{

    /** L4 lbh disabled */
    PRV_TGF_TRUNK_L4_LBH_DISABLED_E,

    /** @brief use long L4 Hash Mode.
     *  L4 Trunk hash function is based on bits [5:0]
     *  and [13:8] of the UDP/TCP destination and
     *  source ports.
     */
    PRV_TGF_TRUNK_L4_LBH_LONG_E,

    /** @brief use short L4 Hash Mode .
     *  L4 Trunk hash function is based on bits [5:0]
     *  of the UDP/TCP destination and source ports.
     */
    PRV_TGF_TRUNK_L4_LBH_SHORT_E

} PRV_TGF_TRUNK_L4_LBH_MODE_ENT;




/* number of entries in the Pearson table */
#define PRV_TGF_TRUNK_PEARSON_TABLE_SIZE_CNS  64

/* indication to clear the table of mask hash */
#define PRV_TGF_TRUNK_HASH_MASK_TABLE_INDEX_CLEAR_ALL_CNS  0xFFFFFFFF

/* type of fields into trunk hash */
typedef enum{
    PRV_TGF_TRUNK_FIELD_L4_PORT_E,   /*src/dst*/
    PRV_TGF_TRUNK_FIELD_IPV6_FLOW_E,
    PRV_TGF_TRUNK_FIELD_IP_ADDR_E,   /*src/dst*/
    PRV_TGF_TRUNK_FIELD_MAC_ADDR_E,  /*src/dst*/
    PRV_TGF_TRUNK_FIELD_MPLS_LABEL_E,/*0/1/2*/
    PRV_TGF_TRUNK_FIELD_LOCAL_SRC_PORT_E,
    PRV_TGF_TRUNK_FIELD_UDBS_E,        /*0..31 (lion 14..22) */

    PRV_TGF_TRUNK_FIELD_LAST_E
}PRV_TGF_TRUNK_FIELD_TYPE_ENT;

/* direction : source/destination of fields in PRV_TGF_TRUNK_FIELD_TYPE_ENT */
typedef enum{
    PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E,
    PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E
}PRV_TGF_TRUNK_FIELD_DIRECTION_ENT;

/* get string for CRC mode */
#define TO_STRING_CRC_MODE_MAC(_crcMode)    \
        (                               \
        _crcMode ==    PRV_TGF_TRUNK_LBH_CRC_6_MODE_E               ? "CRC_6"                :\
        _crcMode ==    PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E  ? "CRC_16_AND_PEARSON"   :\
        _crcMode ==    PRV_TGF_TRUNK_LBH_CRC_32_MODE_E              ? "CRC_32"               :\
                                                                      "???")

/* get string for hash mask sub field type */
#define TO_STRING_HASH_MASK_FIELD_SUB_TYPE_MAC(_hashMaskFieldSubType)    \
        (                                                        \
        _hashMaskFieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E      ? "source"      : \
        _hashMaskFieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E ? "destination" : \
                                                                             "???")


/* get string for hash mask field type */
#define TO_STRING_HASH_MASK_FIELD_TYPE_MAC(_hashMaskFieldType)    \
        (                                                         \
        _hashMaskFieldType == PRV_TGF_TRUNK_FIELD_L4_PORT_E           ? "L4_PORT"         : \
        _hashMaskFieldType == PRV_TGF_TRUNK_FIELD_IPV6_FLOW_E          ? "IPV6_FLOW"      : \
        _hashMaskFieldType == PRV_TGF_TRUNK_FIELD_IP_ADDR_E            ? "IP_ADDR"        : \
        _hashMaskFieldType == PRV_TGF_TRUNK_FIELD_MAC_ADDR_E           ? "MAC_ADDR"       : \
        _hashMaskFieldType == PRV_TGF_TRUNK_FIELD_MPLS_LABEL_E         ? "MPLS_LABEL"     : \
        _hashMaskFieldType == PRV_TGF_TRUNK_FIELD_LOCAL_SRC_PORT_E     ? "LOCAL_SRC_PORT" : \
        _hashMaskFieldType == PRV_TGF_TRUNK_FIELD_UDBS_E               ? "UDBS"           : \
                                                                              "???")
/* get string for designated table mode */
#define TO_STRING_DESIGNATED_TABLE_MODE_MAC(_desigMode)    \
        (                                                         \
        _desigMode == PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E          ? "USE_INGRESS_HASH"         : \
        _desigMode == PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E  ? "USE_INGRESS_HASH_AND_VID" : \
        _desigMode == PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E           ? "USE_SOURCE_INFO"          : \
                                                                              "???")

/* get string for LBH mode */
#define TO_STRING_LBH_MODE_MAC(_lbhMode)    \
        (                                                         \
        _lbhMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E          ? "LOAD_BALANCE_MODE_SINGLE_PORT"           : \
        _lbhMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E                 ? "LOAD_BALANCE_MODE_EVEN"                  : \
        _lbhMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EMPTY_E                ? "LOAD_BALANCE_MODE_EMPTY"                 : \
        _lbhMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E? "LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS" : \
        _lbhMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E      ? "LOAD_BALANCE_MODE_LIMITED_MEMBERS"       : \
                                                                              "???")


/**
* @enum PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT
 *
 * @brief enumeration for cases that build of the CRC 70 bytes that used
 * to generate the CRC hash.
*/
typedef enum{

    /** the flow id can be used instead of UDB21 UDB22 */
    PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E,

    /** @brief the evlan can be used instead of mpls label 0.
     *  NOTE: for non-MPLS the evlan is used anyway.
     */
    PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL0_BY_EVLAN_E,

    /** @brief the original source eport can be used instead of mpls label 1.
     *  NOTE: for non-MPLS the original source eport is used anyway.
     */
    PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL1_BY_ORIGINAL_SOURCE_EPORT_E,

    /** @brief the local device source eport can be used instead of mpls label 2.
     *  NOTE: for non-MPLS the local device source eport is used anyway.
     */
    PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL2_BY_LOCAL_SOURCE_EPORT_E,

    /** @brief the UDB23 UDB34 (12 UDBs) can be used instead of 12 MSB of the SIP IPv6 address,
     *  the UDB0 - UDB11 (12 UDBs) can be used instead of 12 MSB of the DIP IPv6 address,
     *  meaning that only 4 LSB of the IPv6 address are still used by SIP,DIP.
     *  NOTE: for non-IPV6 the 24 UDBs always used
     */
    PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E

} PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfTrunkMembersSet function
* @endinternal
*
* @brief   This function set the trunk members
*
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) enabled members
* @param[in] numOfEnabledMembers      - number of enabled members in the array
* @param[in] disabledMembersArray[]   - (array of) disabled members
* @param[in] numOfDisabledMembers     - number of enabled members in the array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkMembersSet
(
    IN GT_TRUNK_ID                    trunkId,
    IN GT_U32                         numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC          enabledMembersArray[],
    IN GT_U32                         numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC          disabledMembersArray[]
);

/**
* @internal prvTgfTrunkHashGlobalModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the general hashing mode of trunk hash generation.
* @param[in] hashMode                 - hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS prvTgfTrunkHashGlobalModeSet
(
    IN PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode
);

/**
* @internal prvTgfTrunkHashGlobalModeGet function
* @endinternal
*
* @brief   Get the general hashing mode of trunk hash generation
*
* @param[in] devNum                   - the device number
*
* @param[out] hashModePtr              - (pointer to) hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS prvTgfTrunkHashGlobalModeGet
(
    IN  GT_U8                               devNum,
    OUT PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT  *hashModePtr
);

/**
* @internal prvTgfTrunkHashMplsModeEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the MPLS information,
*         when calculating the trunk hashing index for a packet.
*         Note:
*         1. Not relevant to NON-MPLS packets.
*         2. Relevant when cpssExMxPmTrunkHashGlobalModeSet(devNum,
*         CPSS_EXMXPM_TRUNK_LBH_PACKETS_INFO_E)
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
GT_STATUS prvTgfTrunkHashMplsModeEnableSet
(
    IN GT_BOOL enable
);

/**
* @internal prvTgfTrunkHashMaskSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the masks for the various packet fields being used at the Trunk
*         hash calculations
* @param[in] maskedField              - field to apply the mask on
* @param[in] maskValue                - The mask value to be used (0..0x3F)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or maskedField
* @retval GT_OUT_OF_RANGE          - maskValue > 0x3F
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashMaskSet
(
    IN PRV_TGF_TRUNK_LBH_MASK_ENT     maskedField,
    IN GT_U8                          maskValue
);

/**
* @internal prvTgfTrunkHashIpShiftSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the shift being done to IP addresses prior to hash calculations.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
* @param[in] shiftValue               - The shift to be done.
*                                      IPv4 valid shift: 0-3 bytes (Value = 0: no shift).
*                                      IPv6 valid shift: 0-15 bytes (Value = 0: no shift).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or protocolStack
* @retval GT_OUT_OF_RANGE          - shiftValue > 3 for IPv4 , shiftValue > 15 for IPv6
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpShiftSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    IN GT_U32                       shiftValue
);

/**
* @internal prvTgfTrunkHashIpModeSet function
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
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
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
GT_STATUS prvTgfTrunkHashIpModeSet
(
    IN GT_BOOL enable
);

/**
* @internal prvTgfTrunkDesignatedMemberSet function
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
* @note   APPLICABLE DEVICES:      All DxCh Devices
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDesignatedMemberSet
(
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvTgfTrunkDbDesignatedMemberGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function get Configuration per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDbDesignatedMemberGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_TRUNK_MEMBER_STC   *memberPtr
);

/**
* @internal prvTgfTrunkDbIsMemberOfTrunk function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Checks if a member (device,port) is a trunk member.
*         if it is trunk member the function retrieve the trunkId of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
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
GT_STATUS prvTgfTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
);


/**
* @internal prvTgfTrunkDesignatedPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
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
GT_STATUS prvTgfTrunkDesignatedPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    OUT  CPSS_PORTS_BMP_STC  *designatedPortsPtr
);

/**
* @internal prvTgfTrunkMemberAdd function
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
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to add to the trunk.
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
* @retval GT_FULL                  - trunk already contains maximum supported members
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkMemberAdd
(
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvTgfTrunkMemberRemove function
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
* @note   APPLICABLE DEVICES:      All DxCh Devices
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkMemberRemove
(
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);


/**
* @internal prvTgfTrunkMemberDisable function
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
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to disable in the trunk.
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
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkMemberDisable
(
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvTgfTrunkMemberEnable function
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
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to enable in the trunk.
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
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkMemberEnable
(
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvTgfTrunkCascadeTrunkPortsSet function
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
*         - members ports trunk ID is not set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets those ingresses in member ports are not associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
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
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. Application can manipulate the 'Per port' trunk-id , for those ports ,
*       using the 'Low level API' of : cpssDxChTrunkPortTrunkIdSet(...)
*       5. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       6. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
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
*       3. manipulate only 'Non-trunk' table and | 3. manipulate all trunk tables :
*       'Designated trunk' table       | 'Per port' trunk-id , 'Trunk members',
*       | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*       ----------------------------------------------------------------------------
*       5. not associated with trunk Id on    | 5. for cascade trunk : since 'Per port'
*       ingress                | the trunk-Id is set , then load balance
*       | according to 'Local port' for traffic
*       | that ingress cascade trunk and
*       | egress next cascade trunk , will
*       | egress only from one of the egress
*       | trunk ports. (because all ports associated
*       | with the trunk-id)
*       ----------------------------------------------------------------------------
*
*/
GT_STATUS prvTgfTrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
);


/**
* @internal prvTgfTrunkHashCrcParametersSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] crcMode                  - The CRC mode .
* @param[in] crcSeed                  - The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed is in range of 0..0x3f (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed is in range of 0..0xffff (16 bits value)
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
GT_STATUS prvTgfTrunkHashCrcParametersSet
(
    IN PRV_TGF_TRUNK_LBH_CRC_MODE_ENT     crcMode,
    IN GT_U32                               crcSeed
);


/**
* @internal prvTgfTrunkHashCrcParametersGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] devNum                   - device number.
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
* @retval GT_BAD_PARAM             - bad device number , or crcMode
* @retval GT_OUT_OF_RANGE          - crcSeed out of range.
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS prvTgfTrunkHashCrcParametersGet
(
    IN   GT_U8                               devNum,
    OUT  PRV_TGF_TRUNK_LBH_CRC_MODE_ENT      *crcModePtr,
    OUT  GT_U32                              *crcSeedPtr
);

/**
* @internal prvTgfTrunkPortHashMaskInfoSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set port-based hash mask info.
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] devNum                   - The device number.
* @param[in] portNum                  - The port number.
* @param[in] overrideEnable           - enable/disable the override
* @param[in] index                    - the  to use when 'Override enabled'.
*                                      (values 0..15) , relevant only when overrideEnable = GT_TRUE
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
GT_STATUS prvTgfTrunkPortHashMaskInfoSet
(
    IN GT_U8     devNum,
    IN GT_PORT_NUM portNum,
    IN GT_BOOL   overrideEnable,
    IN GT_U32    index
);

/**
* @internal prvTgfTrunkHashMaskCrcEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the entry of masks in the specified index in 'CRC hash mask table'.
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] useIndexOrPacketType     - GT_TRUE - use <index>
*                                      GT_FALSE - use <packetType>
* @param[in] index                    - the table  (0..15)
* @param[in] packetType               - packet type.
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
GT_STATUS prvTgfTrunkHashMaskCrcEntrySet
(
    IN GT_BOOL                        useIndexOrPacketType,
    IN GT_U32                         index,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT    packetType,
    IN PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
);

/**
* @internal prvTgfTrunkHashPearsonValueSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the Pearson hash value for the specific index.
*         NOTE: the Pearson hash used when CRC-16 mode is used.
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] index                    - the table  (0..63)
* @param[in] value                    - the Pearson hash  (0..63)
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
GT_STATUS prvTgfTrunkHashPearsonValueSet
(
    IN GT_U32                         index,
    IN GT_U32                         value
);

/**
* @internal prvTgfTrunkHashDesignatedTableModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash mode of the designated trunk table .
*
* @note   APPLICABLE DEVICES:      All Devices
* @param[in] mode                     - The designated table hashing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashDesignatedTableModeSet
(
    IN PRV_TGF_TRUNK_DESIGNATED_TABLE_MODE_ENT    mode
);


/**
* @internal prvTgfTrunkMcLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Enable/Disable sending multi-destination packets back to its source
*         trunk on the local device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
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
GT_STATUS prvTgfTrunkMcLocalSwitchingEnableSet
(
    IN GT_TRUNK_ID      trunkId,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfTrunkHashIpAddMacModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpAddMacModeSet
(
    IN GT_BOOL  enable
);

/**
* @internal prvTgfTrunkHashIpAddMacModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
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
GT_STATUS prvTgfTrunkHashIpAddMacModeGet
(
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfTrunkHashIpv6ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashMode                 - the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpv6ModeSet
(
    IN PRV_TGF_TRUNK_IPV6_HASH_MODE_ENT   hashMode
);

/**
* @internal prvTgfTrunkHashIpv6ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm.
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
GT_STATUS prvTgfTrunkHashIpv6ModeGet
(
    IN GT_U8                                devNum,
    OUT PRV_TGF_TRUNK_IPV6_HASH_MODE_ENT   *hashModePtr
);

/**
* @internal prvTgfTrunkHashL4ModeSet function
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
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashMode                 - L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashL4ModeSet
(
    IN PRV_TGF_TRUNK_L4_LBH_MODE_ENT hashMode
);

/**
* @internal prvTgfTrunkHashL4ModeGet function
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
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm.
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
GT_STATUS prvTgfTrunkHashL4ModeGet
(
    IN GT_U8   devNum,
    OUT PRV_TGF_TRUNK_L4_LBH_MODE_ENT *hashModePtr
);

/**
* @internal prvTgfTrunkDbMembersSortingEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable/disable 'sorting' of trunk members in the
*         'trunk members table' and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         'sorting enabled' : when the application will add/remove/set members in a trunk
*         cpss will make sure to set the trunk members into the
*         device in an order that is not effected by the 'history'
*         of 'add/remove' members to/from the trunk.
*         'sorting disabled' : (legacy mode / default mode) when the application
*         will add/remove/set members in a trunk cpss will set
*         the trunk members into the device in an order that is
*         effected by the 'history' of 'add/remove' members
*         to/from the trunk.
*         function uses the DB (no HW operations)
*         The API start to effect only on trunks that do operations (add/remove/set members)
*         after the API call.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  ExMxPm.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable the sorting
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDbMembersSortingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
);

/**
* @internal prvTgfTrunkDbMembersSortingEnableGet function
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
GT_STATUS prvTgfTrunkDbMembersSortingEnableGet
(
    IN GT_U8                    devNum,
    OUT GT_BOOL                 *enablePtr
);


/**
* @internal prvTgfTrunkCascadeTrunkWithWeightedPortsSet function
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
GT_STATUS prvTgfTrunkCascadeTrunkWithWeightedPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);

/**
* @internal prvTgfTrunkCascadeTrunkWithWeightedPortsGet function
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
GT_STATUS prvTgfTrunkCascadeTrunkWithWeightedPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);

/**
* @internal prvTgfTrunkTrunkDump function
* @endinternal
*
* @brief   debug function to dump the trunk info from DB and the HW.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*                                      OUTPUTS:
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkTrunkDump
(
    IN    GT_U8 devNum
);

/**
* @internal prvTgfTrunkHashCrcSaltByteSet function
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
GT_STATUS prvTgfTrunkHashCrcSaltByteSet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    IN GT_U8    saltValue
);

/**
* @internal prvTgfTrunkHashMaskCrcParamOverrideSet function
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
GT_STATUS prvTgfTrunkHashMaskCrcParamOverrideSet
(
    IN GT_U8                                            devNum,
    IN PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT   fieldType,
    IN GT_BOOL                                          override
);

/**
* @internal prvTgfTrunkPortTrunkIdSet function
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
GT_STATUS prvTgfTrunkPortTrunkIdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  memberOfTrunk,
    IN GT_TRUNK_ID              trunkId
);





/**
* @internal prvTgfTrunkHashIndexCalculate function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Calculate hash index of member load-balancing group for given hash client.
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
GT_STATUS prvTgfTrunkHashIndexCalculate
(
    IN GT_U8                                        devNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT                  packetType,
    IN PRV_TGF_TRUNK_LBH_INPUT_DATA_STC             *hashInputDataPtr,
    IN PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT           hashEntityType,
    IN GT_U32                                       numberOfMembers,
    OUT GT_U32                                      *hashIndexPtr
);

/**
* @internal prvTgfTrunkHashNumBitsSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Set the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT)
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
*/
GT_STATUS prvTgfTrunkHashNumBitsSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT   hashClient,
    IN GT_U32                               startBit,
    IN GT_U32                               numOfBits
);

/**
* @internal prvTgfTrunkHashNumBitsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Get the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT)
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
GT_STATUS prvTgfTrunkHashNumBitsGet
(
    IN  GT_U8                                devNum,
    IN  PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT   hashClient,
    OUT GT_U32                               *startBitPtr,
    OUT GT_U32                               *numOfBitsPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTrunkGenh */


