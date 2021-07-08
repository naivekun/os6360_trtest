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
* @file tgfPclGen.h
*
* @brief Generic API for PCL
*
* @version   67
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfPclGenh
#endif /*CHX_FAMILY*/


#ifndef __tgfPclGenh
#define __tgfPclGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
    #include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>

    extern GT_U32 appDemoDxChTcamIpclBaseIndexGet
    (
        IN     GT_U8                            devNum,
        IN     GT_U32                           lookupId
    );
    extern GT_U32 appDemoDxChTcamIpclNumOfIndexsGet
    (
        IN     GT_U8                            devNum,
        IN     GT_U32                           lookupId
    );
    extern GT_U32 appDemoDxChTcamEpclBaseIndexGet
    (
        IN     GT_U8                            devNum
    );
    extern GT_U32 appDemoDxChTcamEpclNumOfIndexsGet
    (
        IN     GT_U8                            devNum
    );
    /* restore TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
    extern GT_VOID appDemoDxChTcamSectionsRestore(GT_VOID);
    extern GT_STATUS appDemoDxChTcamClientSectionSet
    (
        IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
        IN     GT_U32                           hitNum,
        IN     GT_U32                           baseIndex,
        IN     GT_U32                           numOfIndexes
    );

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/pcl/cpssExMxPmPcl.h>
    #include <cpss/exMxPm/exMxPmGen/intTcam/cpssExMxPmIntTcam.h>
#endif /* EXMXPM_FAMILY */

#include <common/tgfCommon.h>

/* default PCL id for PCL-ID table configuration */
#define PRV_TGF_PCL_DEFAULT_ID_MAC(_direction, _lookup, _port) \
    ((_direction * 128) + (_lookup * 64) + _port)

/* index for base pcl-ids for virtual routers */
/* since the AppDemo define the default virtual router 0 , we currently must use it */
#define PRV_TGF_PCL_PBR_ID_BASE_CNS   0
/* macro to get pcl-id for relative virtual router */
#define PRV_TGF_PCL_PBR_ID_MAC(index) ((index) + PRV_TGF_PCL_PBR_ID_BASE_CNS)
/* relative index for virtual router to be used in PBR (single VR)*/
#define PRV_TGF_PCL_PBR_VIRTUAL_ROUTER_INDEX_CNS    0
/* pcl-id for the VR for tests */
#define PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS   (PRV_TGF_PCL_PBR_ID_MAC(PRV_TGF_PCL_PBR_VIRTUAL_ROUTER_INDEX_CNS))

/* maximal UDB index                   */
/* (APPLICABLE DEVICES: Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X */
#define PRV_TGF_PCL_UDB_MAX_NUMBER_CNS 70

/* maximal UDB index for the test device */
#define PRV_TGF_PCL_UDB_MAX_NUMBER_MAC(_dev) ((GT_U32)(PRV_CPSS_SIP_5_20_CHECK_MAC(_dev) ? 70 : 50))

/* amount of replaceable UDBs in Ingress UDB only keys */
/* (APPLICABLE DEVICES: Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X */
#define PRV_TGF_PCL_INGRESS_UDB_REPLACE_MAX_CNS  12

/* convert UDB packet types device specific format */
#define PRV_TGF_S2D_PCL_UDB_PACKET_TYPE_CONVERT_MAC(dstType, srcType)   \
    do                                                                  \
    {                                                                   \
        switch (srcType)                                                \
        {                                                               \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E,            \
                CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E);                  \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,            \
                CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E);                  \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_MPLS_E,                \
                CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E);                      \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E,       \
                CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E);             \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,          \
                CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E);                \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,      \
                CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E);            \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_UDE_E,                 \
                CPSS_DXCH_PCL_PACKET_TYPE_UDE_E);                       \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_IPV6_E,                \
                CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E);                      \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_UDE_1_E,               \
                CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E);                      \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_UDE_2_E,               \
                CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E);                      \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_UDE_3_E,               \
                CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E);                      \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_UDE_4_E,               \
                CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E);                      \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_IPV6_TCP_E,            \
                CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E);                  \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,            \
                CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E);                  \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,          \
                CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E);                \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_UDE5_E,                \
                CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E);                      \
            PRV_TGF_SWITCH_CASE_MAC(                                    \
                dstType, PRV_TGF_PCL_PACKET_TYPE_UDE6_E,                \
                CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E);                      \
            default:                                                    \
                return GT_BAD_PARAM;                                    \
        }                                                               \
    } while (0)



/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_PCL_LOOKUP_TYPE_ENT
 *
 * @brief Types of lookups 0 and 1
*/
typedef enum{

    /** single lookup */
    PRV_TGF_PCL_LOOKUP_TYPE_SINGLE_E,

    /** double lookup */
    PRV_TGF_PCL_LOOKUP_TYPE_DOUBLE_E

} PRV_TGF_PCL_LOOKUP_TYPE_ENT;

/**
* @enum PRV_TGF_PCL_ACTION_REDIRECT_CMD_ENT
 *
 * @brief enumerator for PCL redirection target
*/
typedef enum{

    /** no redirection */
    PRV_TGF_PCL_ACTION_REDIRECT_CMD_NONE_E,

    /** redirection to output interface */
    PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E,

    /** Ingress control pipe */
    PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_E,

    /** redirect to virtual router */
    PRV_TGF_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E,

    /** replace MAC source address */
    PRV_TGF_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E,

    PRV_TGF_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E

} PRV_TGF_PCL_ACTION_REDIRECT_CMD_ENT;

/**
* @enum PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT
 *
 * @brief enumerator for PCL redirection types of the passenger packet.
*/
typedef enum{

    /** the passenger packet is Ethernet */
    PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E,

    /** the passenger packet is IP */
    PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E

} PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT;

/**
* @enum PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT
 *
 * @brief enumerator Controls the index used for IPCL lookup
*/
typedef enum{

    /** retain */
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E,

    /** override */
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E

} PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT;

/**
* @enum PRV_TGF_PCL_POLICER_ENABLE_ENT
 *
 * @brief enumerator for policer options.
*/
typedef enum{

    /** Meter and Counter are disabled */
    PRV_TGF_PCL_POLICER_DISABLE_ALL_E              = GT_FALSE,

    /** Both Meter and Counter enabled */
    PRV_TGF_PCL_POLICER_ENABLE_METER_AND_COUNTER_E = GT_TRUE,

    /** Meter only enabled */
    PRV_TGF_PCL_POLICER_ENABLE_METER_ONLY_E,

    /** Counter only enabled */
    PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E

} PRV_TGF_PCL_POLICER_ENABLE_ENT;

/**
* @enum PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT
 *
 * @brief This enum describes possible formats of Policy rules
*/
typedef enum{

    /** Standard L2 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,

    /** Standard L2+IPv4/v6 QoS */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,

    /** Standard IPv4+L4 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,

    /** Standard IPV6 DIP */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E,

    /** Extended L2+IPv4 + L4 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,

    /** Extended L2+IPv6 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,

    /** Extended L4+IPv6 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,

    /** Standard L2 */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,

    /** Standard L2+IPv4/v6 QoS */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,

    /** Standard IPv4+L4 */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,

    /** Extended L2+IPv4 + L4 */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E,

    /** Extended L2+IPv6 */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,

    /** Extended L4+IPv6 */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,

    /** Standard UDB styled */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,

    /** Extended UDB styled */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,

    /** External 80-bytes */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_80B_E,

    /** Standard Routed ACL+QoS+IPv4 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,

    /** Extended Port/VLAN+QoS+IPv4 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,

    /** Ultra Port/VLAN+QoS+IPv6 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E ,

    /** Ultra Routed ACL+QoS+IPv6 */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E,

    /** Extended RACL/VACL IPv4 */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,

    /** Ultra RACL/VACL IPv6 */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E,

    /** Ingress UDB only 10 bytes */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,

    /** Ingress UDB only 20 bytes */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,

    /** Ingress UDB only 30 bytes */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,

    /** Ingress UDB only 40 bytes */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E,

    /** Ingress UDB only 50 bytes */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E,

    /** Ingress UDB only 50 + 10 bytes */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E,

    /** Ingress UDB only 60 bytes */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E,

    /** Egress UDB only 10 bytes */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,

    /** Egress UDB only 20 bytes */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E,

    /** Egress UDB only 30 bytes */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E,

    /** Egress UDB only 40 bytes */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E,

    /** Egress UDB only 50 bytes */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E,

    /** Egress UDB only 60 bytes */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E,

    /** last element */
    PRV_TGF_PCL_RULE_FORMAT_LAST_E

} PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT;

/**
* @enum PRV_TGF_PCL_PACKET_TYPE_ENT
 *
 * @brief Packet types
*/
typedef enum{

    /** IPV4 TCP */
    PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E,

    /** IPV4 UDP */
    PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,

    /** MPLS */
    PRV_TGF_PCL_PACKET_TYPE_MPLS_E,

    /** IPV4 Fragment */
    PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E,

    /** IPV4 Other */
    PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,

    /** Ethernet Other */
    PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,

    /** User Defined Ethernet Type (UDE 0) */
    PRV_TGF_PCL_PACKET_TYPE_UDE_E,

    /** IPV6 */
    PRV_TGF_PCL_PACKET_TYPE_IPV6_E,

    /** UDE 1 */
    PRV_TGF_PCL_PACKET_TYPE_UDE_1_E,

    /** UDE 2 */
    PRV_TGF_PCL_PACKET_TYPE_UDE_2_E,

    /** UDE 3 */
    PRV_TGF_PCL_PACKET_TYPE_UDE_3_E,

    /** UDE 4 */
    PRV_TGF_PCL_PACKET_TYPE_UDE_4_E,

    /** IPV6 TCP */
    PRV_TGF_PCL_PACKET_TYPE_IPV6_TCP_E,

    /** IPV6 UDP */
    PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,

    /** IPV6 not TCP and not UDP */
    PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,

    /** UDE 5 */
    PRV_TGF_PCL_PACKET_TYPE_UDE5_E,

    /** UDE 6 */
    PRV_TGF_PCL_PACKET_TYPE_UDE6_E,

    PRV_TGF_PCL_PACKET_TYPE_LAST_E

} PRV_TGF_PCL_PACKET_TYPE_ENT;

/**
* @enum PRV_TGF_PCL_OFFSET_TYPE_ENT
 *
 * @brief Offset types for packet headers parsing used for user defined
 * bytes configuration
*/
typedef enum{

    /** offset from start of mac(L2) header */
    PRV_TGF_PCL_OFFSET_L2_E,

    /** . */
    PRV_TGF_PCL_OFFSET_L3_E,

    /** offset from start of L4 header */
    PRV_TGF_PCL_OFFSET_L4_E,

    /** offset from start of IPV6 Extention Header */
    PRV_TGF_PCL_OFFSET_IPV6_EXT_HDR_E,

    /** @brief the user defined byte used
     *  for TCP or UDP comparator
     */
    PRV_TGF_PCL_OFFSET_TCP_UDP_COMPARATOR_E,

    /** offset from start of L3 header minus 2 */
    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,

    /** offset from ethernet type of MPLS minus 2 */
    PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E,

    /** offset from start of L2 tunnel header */
    PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,

    /** offset from start of L3 tunnel header */
    PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,

    /** offset from start of metadata */
    PRV_TGF_PCL_OFFSET_METADATA_E,

    /** invalid UDB, conains 0 always */
    PRV_TGF_PCL_OFFSET_INVALID_E

} PRV_TGF_PCL_OFFSET_TYPE_ENT;

/**
* @enum PRV_TGF_UDB_ERROR_CMD_ENT
 *
 * @brief This enum defines packet command taken in the UDB error case
*/
typedef enum{

    /** continue Policy Lookup */
    PRV_TGF_UDB_ERROR_CMD_LOOKUP_E,

    /** trap packet to CPU */
    PRV_TGF_UDB_ERROR_CMD_TRAP_TO_CPU_E,

    /** hard drop packet */
    PRV_TGF_UDB_ERROR_CMD_DROP_HARD_E,

    /** soft drop packet */
    PRV_TGF_UDB_ERROR_CMD_DROP_SOFT_E

} PRV_TGF_UDB_ERROR_CMD_ENT;

/**
* @enum PRV_TGF_PCL_RULE_OPTION_ENT
 *
 * @brief Enumerator for the TCAM rule write option flags.
*/
typedef enum{

    /** @brief write all fields
     *  of rule to TCAM but rule state is invalid
     *  (no match during lookups).
     *  The cpssDxChPclRuleValidStatusSet can turn the rule
     *  to valid state.
     *  (APPLICABLE DEVICES: DxCh3, xCat, Lion, xCat2, ExMxPm)
     */
    PRV_TGF_PCL_RULE_OPTION_WRITE_INVALID_E          = (0x1 << 0)

} PRV_TGF_PCL_RULE_OPTION_ENT;

/**
* @enum PRV_TGF_PCL_TCAM_SEGMENT_MODE_ENT
 *
 * @brief TCAM segment mode.
 * Defines whether the 4 TCAM segments assigned to the PCL are
 * regarded as one logical TCAM returning a single reply,
 * two separate TCAM segments returning two replies,
 * or four separate TCAM segments returning four replies.
 * The segment size measured in quarters of TCAM.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef enum{

    /** all TCAM segments are one logical TCAM */
    PRV_TGF_PCL_TCAM_SEGMENT_MODE_1_TCAM_E,

    /** @brief segmentation into 2 lists,
     *  logically distributed 50% and 50%
     */
    PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_2_AND_2_E,

    /** @brief segmentation into 2 lists,
     *  logically distributed 25% [first] and 75% [second];
     */
    PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_1_AND_3_E,

    /** @brief segmentation into 2 lists,
     *  logically distributed 75% [first] and 25% [second];
     */
    PRV_TGF_PCL_TCAM_SEGMENT_MODE_2_TCAMS_3_AND_1_E,

    /** segmentation into 4 lists, each - 25% */
    PRV_TGF_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E

} PRV_TGF_PCL_TCAM_SEGMENT_MODE_ENT;

/**
* @enum PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT
 *
 * @brief EPCL Key<SrcPort> field source selector for DSA tagged packets.
*/
typedef enum{

    /** For DSA tagged packet, <SrcPort> is taken from the DSA tag. */
    PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ORIGINAL_E,

    /** @brief For all packets (DSA tagged or not), <SrcPort> indicates the
     *  local device ingress physical port.
     */
    PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_LOCAL_E

} PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT;


/**
* @enum PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT
 *
 * @brief EPCL Key<Target port> field source selector.
*/
typedef enum{

    /** Local Device Physical Port. */
    PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E,

    /** Final Destination Port. */
    PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_FINAL_E

} PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT;

/**
* @struct PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC
 *
 * @brief This structure defines future lookups configuration
*/
typedef struct{

    /** index of Pcl Configuration table for next lookup */
    GT_U32 ipclConfigIndex;

    /** selection index of PCL Cfg Table for IPCL lookup0_1 */
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0_1OverrideConfigIndex;

    /** @brief selection index of PCL Cfg Table for IPCL lookup1
     *  Comments:
     */
    PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT pcl1OverrideConfigIndex;

} PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC;

/**
* @enum PRV_TGF_MIRROR_EGRESS_MODE_ENT
 *
 * @brief This enum defines mode of Egress Mirroring.
*/
typedef enum{

    /** @brief Egress Mirror packets
     *  that are not DROPPED
     */
    PRV_TGF_MIRROR_EGRESS_NOT_DROPPED_E,

    /** @brief Otherwise there are three options in case that packets are dropped:
     *  Egress Mirror packets
     *  on TAIL-DROP
     */
    PRV_TGF_MIRROR_EGRESS_TAIL_DROP_E,

    /** @brief Egress Mirror packets
     *  on CONGESTION
     */
    PRV_TGF_MIRROR_EGRESS_CONGESTION_E,

    /** @brief Egress Mirror packets
     *  on CONGESTION or TAIL-DROP
     */
    PRV_TGF_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E

}PRV_TGF_MIRROR_EGRESS_MODE_ENT;


/**
* @struct PRV_TGF_PCL_ACTION_MIRROR_STC
 *
 * @brief This structure defines the mirroring related Actions.
*/
typedef struct{

    /** The CPU code assigned to packets */
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;

    /** Enables mirroring to the ingress analyzer port */
    GT_BOOL mirrorToRxAnalyzerPort;

    /** @brief index of analyzer interface (0..6)
     *  (APPLICABLE DEVICES: Lion2)
     *  Relevant only when (mirrorToRxAnalyzerPort == GT_TRUE)
     */
    GT_U32 ingressMirrorToAnalyzerIndex;

    /** @brief TCP RST and FIN packets can be mirrored to CPU
     *  Comments:
     */
    GT_BOOL mirrorTcpRstAndFinPacketsToCpu;

    /** @brief Enables mirroring the packet to
     *  the egress analyzer interface.
     *  GT_FALSE - Packet is not mirrored to egress analyzer interface.
     *  GT_TRUE - Packet is mirrored to egress analyzer interface.
     *
     *  Relevant for egress actions , for next devices:
     *  APPLICABLE DEVICES : Falcon.
     */
    GT_BOOL mirrorToTxAnalyzerPortEn;

    /** @brief index of analyzer interface
     *  (APPLICABLE RANGES: 0..6)
     *  (APPLICABLE DEVICES: Falcon)
     *  Relevant only when (mirrorToTxAnalyzerPort == GT_TRUE)
     */
    GT_U32 egressMirrorToAnalyzerIndex;

    /** @brief egress mode
     *  Relevant only when (mirrorToTxAnalyzerPort == GT_TRUE)
     *
     *  Relevant for egress actions , for next devices:
     *  APPLICABLE DEVICES : Falcon.
     */
    PRV_TGF_MIRROR_EGRESS_MODE_ENT egressMirrorToAnalyzerMode;

} PRV_TGF_PCL_ACTION_MIRROR_STC;

/**
* @struct PRV_TGF_PCL_ACTION_MATCH_COUNTER_STC
 *
 * @brief This structure defines the using of rule match counter.
*/
typedef struct{

    /** enables the binding to the Policy Rule Match Counter */
    GT_BOOL enableMatchCount;

    /** @brief index one of the 32 Policy Rule Match Counter
     *  Comments:
     */
    GT_U32 matchCounterIndex;

} PRV_TGF_PCL_ACTION_MATCH_COUNTER_STC;

/**
* @enum PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_ENT
 *
 * @brief Enumerator for ingress Tag1 UP assignment command.
*/
typedef enum{

    /** @brief Do not modify the <UP1> assigned to the packet
     *  by previous engines.
     */
    PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E,

    /** @brief If packet does not contain Tag1
     *  assign according to action entry�s <UP1>,
     *  else retain previous engine <UP1> assignment
     */
    PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E,

    /** @brief If packet contains Tag0 use UP0,
     *  else use action entry�s <UP1> field.
     */
    PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E,

    /** Assign action entry�s <UP1> field to all packets. */
    PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_ALL_E

} PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_ENT;

/**
* @enum PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_ENT
 *
 * @brief Enumerator for egress Tag0 VID and UP assignment command.
*/
typedef enum{

    /** don't assign Tag0 value from entry. */
    PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E ,

    /** @brief assign Tag0 value from entry to outer Tag.
     *  It is Tag0 for not TS packets
     *  or Tunnel header Tag for TS packets.
     */
    PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E ,

    /** @brief assign Tag0 value from entry to Tag0
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E

} PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_ENT;

/**
* @enum PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT
 *
 * @brief Enumerator for egress DSCP assignment command.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef enum{

    /** Keep previous packet DSCP/EXP settings. */
    PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E = GT_FALSE,

    /** @brief Modify Outer
     *  If packet is MPLS, modify EXP of
     *  packet's outer label to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the tunnel header
     *  Otherwise, If packet is IP, modify the packet's DSCP to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the tunnel header.
     *  (Backward Compatible mode).
     */
    PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E = GT_TRUE,

    /** @brief Modify Inner DSCP;
     *  If packet is IP, modify the packet's DSCP to <DSCP EXP>.
     *  If the packet is tunneled, this refers to the passenger header.
     *  This mode is not applicable for MPLS packets;
     */
    PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E

} PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT;

/**
* @struct PRV_TGF_PCL_ACTION_QOS_STC
 *
 * @brief This structure defines the packet's QoS attributes mark Actions
*/
typedef struct{

    /** modify DSCP QoS attribute of the packet */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyDscp;

    /** modify UP QoS attribute of the packet */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyUp;

    /** modify TC QoS attribute of the packet */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyTc;

    /** modify DP QoS attribute of the packet */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyDp;

    /** modify EXP QoS attribute of the packet */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT modifyExp;

    /** TC field set to the transmitted packets */
    GT_U32 tc;

    /** DP field set to the transmitted packets */
    CPSS_DP_LEVEL_ENT dp;

    /** UP field set to the transmitted packets */
    GT_U32 up;

    /** DSCP field set to the transmitted packets */
    GT_U32 dscp;

    /** EXP field set to the transmitted packets */
    GT_U32 exp;

    /** The QoS Profile Attribute of the packet */
    GT_U32 profileIndex;

    /** Enable marking of QoS Profile Attribute */
    GT_BOOL profileAssignIndex;

    /** Marking of the QoSProfile Precedence */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT profilePrecedence;

    /** @brief egress Tag0 UP modify command
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_ENT egressUp0Cmd;

    /** @brief ingress Tag1 UP modify command
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_PCL_ACTION_INGRESS_UP1_CMD_ENT ingressUp1Cmd;

    /** @brief egress Tag1 UP modify enable
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL egressUp1ModifyEnable;

    /** @brief Tag1 UP field set to the transmitted packets
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 up1;

    /** @brief Egress Modify DSCP/EXP command
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Comments:
     */
    PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT egressDscpCmd;

} PRV_TGF_PCL_ACTION_QOS_STC;

/**
* @struct PRV_TGF_PCL_ACTION_REDIRECT_STC
 *
 * @brief This structure defines the redirection related Actions
*/
typedef struct
{
    PRV_TGF_PCL_ACTION_REDIRECT_CMD_ENT  redirectCmd;

    union
    {
        struct
        {
            GT_U32                      arpPtr;
            GT_BOOL                     modifyMacDa;
        } noRedirect;

        struct
        {
            CPSS_INTERFACE_INFO_STC     outInterface;
            PRV_TGF_OUTLIF_TYPE_ENT     outlifType;

            union
            {
                GT_U32  arpPtr;
                GT_U32  ditPtr;

                struct
                {
                    PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT tunnelType;
                    GT_U32                                      ptr;
                } tunnelStartPtr;

            } outlifPointer;

            GT_BOOL                      modifyMacDa;
            GT_BOOL                      modifyMacSa;
            GT_BOOL                      vntL2Echo;
        } outIf;

        GT_U32                   routerLttIndex;
        GT_U32                   vrfId;
        struct
        {
            GT_U32                   arpPtr;
            GT_ETHERADDR             macSa;
        } modifyMacSa;
        struct
        {
            CPSS_INTERFACE_INFO_STC  logicalInterface;
            GT_BOOL                  sourceMeshIdSetEnable;
            GT_U32                   sourceMeshId;
            GT_BOOL                  userTagAcEnable;
        } logicalSourceInterface;
    } data;

} PRV_TGF_PCL_ACTION_REDIRECT_STC;

/**
* @struct PRV_TGF_PCL_ACTION_POLICER_STC
 *
 * @brief This structure defines the policer related Actions
*/
typedef struct{

    /** policer enable */
    PRV_TGF_PCL_POLICER_ENABLE_ENT policerEnable;

    /** @brief policers table entry index
     *  Comments:
     */
    GT_U32 policerId;

} PRV_TGF_PCL_ACTION_POLICER_STC;

/**
* @enum PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT
 *
 * @brief Enumerator for ingress Tag1 VID assignment command.
*/
typedef enum{

    /** Do not modify Tag1 VID. */
    PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E,

    /** @brief The entry�s <Tag1 VID> is assigned as Tag1 VID for
     *  packets that do not have Tag1
     *  or Tag1-priority tagged packets.
     *  For packets received with Tag1 VID,
     *  retain previous engine assignment.
     */
    PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E,

    /** Assign action entry�s <VID1> field to all packets. */
    PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E

} PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT;

/**
* @struct PRV_TGF_PCL_ACTION_VLAN_STC
 *
 * @brief This structure defines the VLAN modification related Actions.
*/
typedef struct{

    /** VLAN id modification command */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT modifyVlan;

    /** nested VLAN */
    GT_BOOL nestedVlan;

    /** VLAN id */
    GT_U16 vlanId;

    /** the VLAN Assignment precedence */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT precedence;

    /** @brief VLAN id modification command
     *  (APPLICABLE DEVICES: Puma2)
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT vlanCmd;

    /** @brief VLAN id or InLif index
     *  (APPLICABLE DEVICES: Puma2)
     */
    GT_U32 vidOrInLif;

    /** @brief egress Tag0 UP modify command
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_ENT egressVlanId0Cmd;

    /** @brief Tag1 VID modify command for ingress
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT ingressVlanId1Cmd;

    /** @brief Tag1 VID modify enable for egress
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL egressVlanId1ModifyEnable;

    /** @brief Tag1 VID
     *  (APPLICABLE DEVICES: Lion2)
     *  Comments:
     */
    GT_U32 vlanId1;

} PRV_TGF_PCL_ACTION_VLAN_STC;

/**
* @struct PRV_TGF_PCL_ACTION_IP_UC_ROUTE_STC
 *
 * @brief This structure defines the IP unicast route parameters
*/
typedef struct{

    /** configure IP Unicast Routing Actions */
    GT_BOOL doIpUcRoute;

    /** route Entry ARP Index to the ARP Table (10 bit) */
    GT_U32 arpDaIndex;

    /** decrement IPv4 <TTL> or IPv6 <Hop Limit> enable */
    GT_BOOL decrementTTL;

    /** bypass Router engine TTL and Options Check */
    GT_BOOL bypassTTLCheck;

    /** @brief ICMP Redirect Check Enable
     *  Comments:
     */
    GT_BOOL icmpRedirectCheck;

} PRV_TGF_PCL_ACTION_IP_UC_ROUTE_STC;

/**
* @struct PRV_TGF_PCL_ACTION_SOURCE_ID_STC
 *
 * @brief This structure defines packet Source Id assignment
*/
typedef struct{

    /** assign Source Id enable */
    GT_BOOL assignSourceId;

    /** @brief the Source Id value to be assigned
     *  Comments:
     */
    GT_U32 sourceIdValue;

} PRV_TGF_PCL_ACTION_SOURCE_ID_STC;

/**
* @struct PRV_TGF_PCL_ACTION_FABRIC_STC
 *
 * @brief Fabric connectivity data
*/
typedef struct{

    /** Flow ID assigned here may be used by a 98FX950 fabric adapter */
    GT_U32 flowId;

} PRV_TGF_PCL_ACTION_FABRIC_STC;

/**
* @struct PRV_TGF_PCL_ACTION_OAM_STC
 *
 * @brief OAM relevant data
*/
typedef struct{

    /** @brief indicates that a timestamp should be inserted into the
     *  packet:
     *  - GT_TRUE - enable timestamp insertion.
     *  - GT_FALSE - disable timestamp insertion.
     */
    GT_BOOL timeStampEnable;

    /** @brief When <Timestamp Enable> is enabled, indicates
     *  the offset index for the timestamp offset table.
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 offsetIndex;

    /** @brief binds the packet to an entry in the OAM Table.
     *  - GT_TRUE - enable OAM packet processing.
     *  - GT_FALSE - disable OAM packet processing.
     */
    GT_BOOL oamProcessEnable;

    /** @brief determines the set of UDBs where the key attributes
     *  (opcode, MEG level, RDI, MEG level) of the OAM message
     *  are taken from.
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U32 oamProfile;

} PRV_TGF_PCL_ACTION_OAM_STC;

/**
* @struct PRV_TGF_PCL_ACTION_SOURCE_PORT_STC
 *
 * @brief This structure defines Source ePort Assignment.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief GT_FALSE
     *  - GT_TRUE - Assign source ePort from sourcePortValue.
     */
    GT_BOOL assignSourcePortEnable;

    /** @brief Source ePort value.
     *  Comments:
     */
    GT_U32 sourcePortValue;

} PRV_TGF_PCL_ACTION_SOURCE_PORT_STC;

/**
 * @struct PRV_TGF_PCL_ACTION_LATENCY_MONITORING_STC
 *
 * @brief A structure that describes latency PCL action.
 */
typedef struct
{
    /** @brief Enable latency monitoring */
    GT_BOOL monitoringEnable;

    /** @brief Assign latency monitoring profile */
    GT_U32  latencyProfile;
} PRV_TGF_PCL_ACTION_LATENCY_MONITORING_STC;

/**
* @struct PRV_TGF_PCL_ACTION_STC
 *
 * @brief Policy Engine Action
*/
typedef struct{

    /** @brief packet command (forward, mirror
     *  hard-drop, soft-drop, or trap-to-cpu)
     */
    CPSS_PACKET_CMD_ENT pktCmd;

    /** Action Stop */
    GT_BOOL actionStop;

    /** Bridge engine processed or bypassed */
    GT_BOOL bypassBridge;

    /** the ingress pipe bypassed or not */
    GT_BOOL bypassIngressPipe;

    /** @brief GT_TRUE
     *  GT_FALSE - Action is used for the Ingress Policy
     */
    GT_BOOL egressPolicy;

    /** configuration of IPCL lookups. */
    PRV_TGF_PCL_ACTION_LOOKUP_CONFIG_STC lookupConfig;

    /** packet mirroring configuration */
    PRV_TGF_PCL_ACTION_MIRROR_STC mirror;

    /** match counter configuration */
    PRV_TGF_PCL_ACTION_MATCH_COUNTER_STC matchCounter;

    /** packet QoS attributes modification configuration */
    PRV_TGF_PCL_ACTION_QOS_STC qos;

    /** packet Policy redirection configuration */
    PRV_TGF_PCL_ACTION_REDIRECT_STC redirect;

    /** packet Policing configuration */
    PRV_TGF_PCL_ACTION_POLICER_STC policer;

    /** packet VLAN modification configuration */
    PRV_TGF_PCL_ACTION_VLAN_STC vlan;

    /** @brief special DxCh (not relevant for DxCh2 and above) Ip Unicast Route
     *  action parameters configuration
     */
    PRV_TGF_PCL_ACTION_IP_UC_ROUTE_STC ipUcRoute;

    /** packet source Id assignment */
    PRV_TGF_PCL_ACTION_SOURCE_ID_STC sourceId;

    /** packet OAM configuration */
    PRV_TGF_PCL_ACTION_OAM_STC oam;

    /** fabric connectivity configuration */
    PRV_TGF_PCL_ACTION_FABRIC_STC fabric;

    /** flow Id (for DxCh devices) */
    GT_U32 flowId;

    /** @brief source ePort Assignment.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_PCL_ACTION_SOURCE_PORT_STC sourcePort;

    /** @brief GT_FALSE
     *  - GT_TRUE - Override the Mac2Me mechanism and set Mac2Me to 0x1.
     *  Ingress PCL Only.
     */
    GT_BOOL setMacToMe;

    /** @brief When enabled, the MPLS G
     *  to an OAM Opcode that is used by the OAM engine.
     *  - GT_FALSE - Disable.
     *  - GT_TRUE - Enable.
     *  Egress PCL Only.
     */
    GT_BOOL channelTypeToOpcodeMapEnable;

    /** @brief TM Queue ID assignment.
     *  This value is assigned to the outgoing descriptor.
     *  It may be subsequently modified by the TM Q-Mapper.
     *  Egress PCL Only.
     */
    GT_U32 tmQueueId;

    /** @brief GT_TRUE
     *  GT_FALSE configure action to preserve previous assignment.
     *  Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat, xCat3) xCat device: applicable starting from revision C0
     */
    GT_BOOL unknownSaCommandEnable;

    /** @brief Assigns the unknown
     *  Relevant when unknownSaCommandEnable == GT_TRUE.
     *  Relevant when VPLS mode init parameter is enabled.
     *  (APPLICABLE DEVICES: xCat, xCat3) xCat device: applicable starting from revision C0
     *  Comment:
     */
    CPSS_PACKET_CMD_ENT unknownSaCommand;

    /** @brief GT_TRUE - Terminate Cut Through packet mode (switch to Store and Forward).
     *         GT_FALSE - leave packet mode (CT or SF) unchanged.
     *   Terminate Cut Through packet mode (switch to Store and Forward).
     *   Relevant to Egress PCL only.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL terminateCutThroughMode;

        /** @brief Latency monitoring
     *  (APPLICABLE DEVICES: Falcon)
     */
    PRV_TGF_PCL_ACTION_LATENCY_MONITORING_STC latencyMonitor;

    /** @brief GT_TRUE - Skip FDB SA lookup.
     *   GT_FALSE - Don't modify the state of Skip FDB SA lookup.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL skipFdbSaLookup;

    /** @brief GT_TRUE - The Ingress PCL will trigger a dedicated
     *         interrupt towards the CPU.
     *   GT_FALSE - will not trigger.
     *   Relevant to Ingress PCL Only.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL triggerInterrupt;

     /** @brief GT_TRUE - The Exact Match action has a priority over the PCL action.
     *          GT_FALSE - The Exact Match action doesn't have a
     *          priority over the PCL action.
     *   (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL exactMatchOverPclEn;

} PRV_TGF_PCL_ACTION_STC;

/**
* @struct PRV_TGF_PCL_LOOKUP_CFG_STC
 *
 * @brief PCL Configuration Table Entry Per Lookup Attributes
*/
typedef struct
{
    GT_BOOL                                 enableLookup;
    GT_BOOL                                 externalLookup;
    GT_BOOL                                 dualLookup;
    GT_U32                                  pclIdL01;
    PRV_TGF_PCL_LOOKUP_TYPE_ENT             lookupType;
    GT_U32                                  pclId;

    struct
    {
        PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    nonIpKey;
        PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ipv4Key;
        PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ipv6Key;
    } groupKeyTypes;

    GT_BOOL                                 udbKeyBitmapEnable;
    PRV_TGF_PCL_TCAM_SEGMENT_MODE_ENT       tcamSegmentMode;

    /** @brief PCL Sub Profile Id for exact match
    *   APPLICABLE DEVICE: Falcon.
    *   APPLICABLE RANGES: 0..7.
    */
    GT_U32                                    exactMatchLookupSubProfileId;
} PRV_TGF_PCL_LOOKUP_CFG_STC;

/**
* @enum PRV_TGF_PCL_UDB_OVERRIDE_TYPE_ENT
 *
 * @brief User Defined Bytes override types.
*/
typedef enum{

    /** @brief VRF ID MSB
     *  (APPLICABLE DEVICES: xCat, Lion, Lion2).
     */
    PRV_TGF_PCL_UDB_OVERRIDE_TYPE_VRF_ID_MSB_E,

    /** @brief VRF ID LSB
     *  (APPLICABLE DEVICES: xCat, Lion, Lion2).
     */
    PRV_TGF_PCL_UDB_OVERRIDE_TYPE_VRF_ID_LSB_E,

    /** @brief Qos profile index
     *  (APPLICABLE DEVICES: xCat, Lion, xCat2, Lion2).
     */
    PRV_TGF_PCL_UDB_OVERRIDE_TYPE_QOS_PROFILE_E,

    /** @brief Trunk Hash
     *  (APPLICABLE DEVICES: xCat, Lion, xCat2, Lion2).
     */
    PRV_TGF_PCL_UDB_OVERRIDE_TYPE_TRUNK_HASH_E,

    /** @brief Tag1 info
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_PCL_UDB_OVERRIDE_TYPE_TAG1_INFO_E,

    /** @brief 20-bit flow Id.
     *  for extention only 3 UDBs overridden.
     *  (APPLICABLE DEVICES: Lion2)
     */
    PRV_TGF_PCL_UDB_OVERRIDE_TYPE_FLOW_ID_E

} PRV_TGF_PCL_UDB_OVERRIDE_TYPE_ENT;


/**
* @struct PRV_TGF_PCL_RULE_FORMAT_COMMON_STC
 *
 * @brief This structure describes the common segment of all key formats
*/
typedef struct{

    /** PCL */
    GT_U16 pclId;

    /** MAC To Me */
    GT_U8 macToMe;

    /** @brief port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     *  Field muxing description(APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourcePort is muxed with portListBmp[15:8]
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** @brief Port list bitmap.
     *  (APPLICABLE DEVICES: xCat2, Lion2).
     */
    CPSS_PORTS_BMP_STC portListBmp;

    /** Flag indicating the packet Tag state */
    GT_U8 isTagged;

    /** VLAN ID assigned to the packet */
    GT_U16 vid;

    /** packet's 802.1p User Priority field */
    GT_U8 up;

    /** QoS profile assigned to the packet */
    GT_U8 qosProfile;

    /** indication that the packet is IP */
    GT_U8 isIp;

    /** indication that the packet is IPv4 */
    GT_U8 isIpv4;

    /** indication that the packet is ARP */
    GT_U8 isArp;

    /** @brief indicates that L2 information in the
     *  search key is valid
     */
    GT_U8 isL2Valid;

    /** user */
    GT_U8 isUdbValid;

} PRV_TGF_PCL_RULE_FORMAT_COMMON_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC
 *
 * @brief This structure describes the common segment of all extended key formats
*/
typedef struct{

    /** indication that the packet is IPv6 */
    GT_U8 isIpv6;

    /** IP protocol/Next Header type */
    GT_U8 ipProtocol;

    /** DSCP field of the IPv4/6 header */
    GT_U8 dscp;

    /** L4 information is valid */
    GT_U8 isL4Valid;

    /** user */
    GT_U8 isUdbValid;

    /** L4 information is available for UDP and TCP */
    GT_U8 l4Byte0;

    /** L4 information is available for UDP and TCP */
    GT_U8 l4Byte1;

    /** L4 information is available for UDP and TCP */
    GT_U8 l4Byte2;

    /** L4 information is available for UDP and TCP */
    GT_U8 l4Byte3;

    /** L4 information is available for UDP and TCP */
    GT_U8 l4Byte13;

    /** indicates a valid IP header */
    GT_U8 ipHeaderOk;

    /** packet has VLAN tag 0 */
    GT_U8 isTagged;

    GT_U8 sipBits[4];

} PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_IP_STC
 *
 * @brief This structure describes common segment of all standard IP key formats
*/
typedef struct{

    /** indicates the Layer 4 IP protocol */
    GT_U8 ipProtocol;

    /** IP packets DSCP */
    GT_U8 dscp;

    /** 1 */
    GT_U8 isL4Valid;

    /** L4 header byte 2 */
    GT_U8 l4Byte2;

    /** L4 header byte 3 */
    GT_U8 l4Byte3;

    /** 0 */
    GT_U8 ipHeaderOk;

    /** indicates IPv4 fragment */
    GT_U8 ipv4Fragmented;

} PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_IP_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_UDB_STC
 *
 * @brief PCL Rule Key fields common to ingress "UDB" key formats
*/
typedef struct{

    /** PCL */
    GT_U16 pclId;

    /** MAC To Me */
    GT_U8 macToMe;

    /** @brief port number from which the packet
     *  ingressed the device
     *  isTagged    - Flag indicating the packet Tag state
     */
    GT_U8 sourcePort;

    /** VLAN ID assigned to the packet */
    GT_U16 vid;

    /** packet's 802.1p User Priority field */
    GT_U8 up;

    /** indication that the packet is IP */
    GT_U8 isIp;

    /** packet's VLAN Tag format */
    GT_U8 pktTagging;

    /** indicates that the L3 offset was found */
    GT_U8 l3OffsetInvalid;

    /** 0=Other/Ivalid; */
    GT_U8 l4ProtocolType;

    /** packet type */
    GT_U8 pktType;

    /** 0 */
    GT_U8 ipHeaderOk;

    /** known and Unknown Unicast */
    GT_U8 macDaType;

    /** indicates that the L4 offset was found */
    GT_U8 l4OffsetInvalid;

    /** L2 encapsulation of the packet */
    GT_U8 l2Encap;

    /** indicates that an IPv6 extension exists */
    GT_U8 isIpv6Eh;

    /** indicates that the IPv6 Header is hop */
    GT_U8 isIpv6HopByHop;

    /** user */
    GT_U8 isUdbValid;

    /** L2 fields */
    GT_U8 isL2Valid;

    /** IP DSCP or MPLS EXP */
    GT_U8 dscpOrExp;

} PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_UDB_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_STD_NOT_IP_STC
 *
 * @brief This structure describes the standard not-IP key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC common;

    /** EtherType */
    GT_U16 etherType;

    /** L2 encapsulation of the packet */
    GT_U8 l2Encap;

    /** ethernet Dst MAC address */
    GT_ETHERADDR macDa;

    /** ethernet Src MAC address */
    GT_ETHERADDR macSa;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** an additional positon of udb15 in the external key */
    GT_U8 udb15Dup;

    /** VRF Id (APPLICABLE DEVICES: xCat, Lion, Lion2) */
    GT_U32 vrfId;

    /** Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2) */
    GT_U32 trunkHash;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 cfi1;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb23_26  - User Defined Bytes 23-26
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_STD_NOT_IP_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
 *
 * @brief This structure describes the standard IPV4/V6 L2_QOS key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC common;

    /** common part for all stanard IP formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** IPv6 extension header exists */
    GT_U8 isIpv6ExtHdrExist;

    /** indicates that the IPv6 Original Extension Header */
    GT_U8 isIpv6HopByHop;

    /** ethernet Dst MAC address */
    GT_ETHERADDR macDa;

    /** ethernet Src MAC address */
    GT_ETHERADDR macSa;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** @brief VRF Id (APPLICABLE DEVICES: xCat, Lion, Lion2)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     */
    GT_U32 vrfId;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb27_30     - User Defined Bytes 27-30
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_STD_IPV4_L4_STC
 *
 * @brief This structure describes the standard IPV4_L4 key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC common;

    /** common part for all stanard IP formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** ethernet broadcast packet */
    GT_U8 isBc;

    /** IPv4 source IP address field */
    GT_IPADDR sip;

    /** IPv4 destination IP address field */
    GT_IPADDR dip;

    /** see l4Byte0 in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte0;

    /** see l4Byte0 in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte1;

    /** see l4Byte0 in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte13;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** VRF Id (APPLICABLE DEVICES: xCat, Lion, Lion2) */
    GT_U32 vrfId;

    /** Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2) */
    GT_U32 trunkHash;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb31_34  - User Defined Bytes 31-34
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_STD_IPV4_L4_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_STD_IPV6_DIP_STC
 *
 * @brief This structure describes the standard IPV6 DIP key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC common;

    /** common part for all standard IP formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_IP_STC commonStdIp;

    /** IPv6 extension header exists */
    GT_U8 isIpv6ExtHdrExist;

    /** indicates that the IPv6 Original Extension Header */
    GT_U8 isIpv6HopByHop;

    /** @brief 16 bytes IPV6 destination address.
     *  udb47_49     - User Defined Bytes 47-49
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     *  udb0       - User Defined Byte 0
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_IPV6ADDR dip;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_STD_IPV6_DIP_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
 *
 * @brief This structure describes the extended not-IPV6 key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC common;

    /** common part for all extended formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** IPv4 source IP address field */
    GT_IPADDR sip;

    /** IPv4 header destination IP address field */
    GT_IPADDR dip;

    /** ether type */
    GT_U16 etherType;

    /** indicates L2 encapsulation */
    GT_U8 l2Encap;

    /** ethernet Dst MAC address */
    GT_ETHERADDR macDa;

    /** ethernet Src MAC address */
    GT_ETHERADDR macSa;

    /** indicates IPv4 fragment */
    GT_U8 ipv4Fragmented;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 cfi1;

    /** VRF Id (APPLICABLE DEVICES: xCat, Lion, Lion2) */
    GT_U32 vrfId;

    /** Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2) */
    GT_U32 trunkHash;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb39_46    - User Defined Bytes 39-46
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
 *
 * @brief This structure describes the extended IPV6+L2 key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC common;

    /** common part for all extended formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field */
    GT_IPV6ADDR sip;

    /** IPV6 destination address highest 8 bits */
    GT_U8 dipBits;

    /** IPv6 extension header exists */
    GT_U8 isIpv6ExtHdrExist;

    /** indicates that the IPv6 Original Extension Header */
    GT_U8 isIpv6HopByHop;

    /** ethernet Dst MAC address */
    GT_ETHERADDR macDa;

    /** ethernet Src MAC address */
    GT_ETHERADDR macSa;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** the duplicated place of UDB11 in the key */
    GT_U8 udb11Dup;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 up1;

    /** VRF Id (APPLICABLE DEVICES: xCat, Lion, Lion2) */
    GT_U32 vrfId;

    /** Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2) */
    GT_U32 trunkHash;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb47_49     - User Defined Bytes 47-49
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     *  udb0_4      - User Defined Bytes 0-4
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_EXT_IPV6_L2_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
 *
 * @brief This structure describes the extended IPV6+L4 key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC common;

    /** common part for all extended formats */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field */
    GT_IPV6ADDR dip;

    /** IPv6 extension header exists */
    GT_U8 isIpv6ExtHdrExist;

    /** indicates that the IPv6 Original Extension Header */
    GT_U8 isIpv6HopByHop;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** duplicated place of UDB12 in the key */
    GT_U8 udb12Dup;

    /** duplicated place of UDB13 in the key */
    GT_U8 udb13Dup;

    /** duplicated place of UDB13 in the key */
    GT_U8 udb14Dup;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 up1;

    /** VRF Id (APPLICABLE DEVICES: xCat, Lion, Lion2) */
    GT_U32 vrfId;

    /** Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2) */
    GT_U32 trunkHash;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb15_22     - User Defined Bytes 15-22
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_EXT_IPV6_L4_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC
 *
 * @brief This structure describes the common segment of all egress key formats
*/
typedef struct
{
    GT_U16                 pclId;
    GT_U8                  isMpls;
    GT_U8                  isIpv4;
    GT_U8                  isArp;
    GT_U32                  srcPort;
    CPSS_PORTS_BMP_STC     portListBmp;
    GT_U8                  isTagged;
    GT_U16                 vid;
    GT_U8                  up;
    GT_U8                  isIp;
    GT_U8                  isL2Valid;
    GT_U8                  egrPacketType;
    struct
    {
        struct
        {
            GT_U8          cpuCode;
            GT_U8          srcTrg;
        }                  toCpu;
        struct
        {
            GT_U8          tc;
            GT_U8          dp;
            GT_U8          egrFilterEnable;
        }                  fromCpu;
        struct
        {
            GT_U8          rxSniff;
        }                  toAnalyzer;
        struct
        {
            GT_U8          qosProfile;
            GT_U8          srcTrunkId;
            GT_U8          srcIsTrunk;
            GT_U8          isUnknown;
            GT_U8          isRouted;
        }                  fwdData;
    }                      egrPktType;
    GT_U8                  srcDev;
    GT_U32                 sourceId;
    GT_U8                  isVidx;
    GT_U8                  tag1Exist;
} PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STD_IP_STC
 *
 * @brief This structure describes the common segment of all egress standard IP key formats
*/
typedef struct{

    GT_U8 isIpv4;

    /** indicates the Layer 4 IP protocol / Next Header type */
    GT_U8 ipProtocol;

    /** IP packets DSCP */
    GT_U8 dscp;

    /** 1 */
    GT_U8 isL4Valid;

    /** L4 header byte 2 */
    GT_U8 l4Byte2;

    /** L4 header byte 3 */
    GT_U8 l4Byte3;

    /** L4 header byte 13 */
    GT_U8 l4Byte13;

    /** indicates IPv4 fragment */
    GT_U8 ipv4Fragmented;

    /** Tcp Udp Port Comparators */
    GT_U8 egrTcpUdpPortComparator;

    GT_U8 dipBits[4];

    /** TOS bits 1:0 */
    GT_U8 tosBits;

} PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STD_IP_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_EXT_STC
 *
 * @brief This structure describes common segment of extended egress key formats
*/
typedef struct{

    /** indication that the packet is IPv6 */
    GT_U8 isIpv6;

    GT_U8 isArp;

    GT_U8 isL2Valid;

    GT_U8 packetCmd;

    /** IP protocol/Next Header type */
    GT_U8 ipProtocol;

    /** DSCP field of the IPv4/6 header */
    GT_U8 dscp;

    /** L4 information is valid */
    GT_U8 isL4Valid;

    /** L4 information is available */
    GT_U8 l4Byte0;

    /** L4 information is available */
    GT_U8 l4Byte1;

    /** L4 information is available */
    GT_U8 l4Byte2;

    /** L4 information is available */
    GT_U8 l4Byte3;

    /** L4 information is available */
    GT_U8 l4Byte13;

    /** TCP/UDP comparator result */
    GT_U8 egrTcpUdpPortComparator;

} PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_EXT_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_STD_NOT_IP_STC
 *
 * @brief This structure describes the standard egress not-IP key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC common;

    /** ether type */
    GT_U16 etherType;

    /** L2 encapsulation of the packet */
    GT_U8 l2Encap;

    /** ethernet Dst MAC address */
    GT_ETHERADDR macDa;

    /** ethernet Src MAC address */
    GT_ETHERADDR macSa;

    /** @brief ethernet broadcast packet
     *  (APPLICABLE DEVICES: EXMXPM)
     */
    GT_U8 isBc;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb0_3   - User Defined Bytes 0-3
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EGR_STD_NOT_IP_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_STD_IP_L2_QOS_STC
 *
 * @brief This structure describes the standard egress IPV4/V6 L2_QOS key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC common;

    /** common part for all standard IP formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STD_IP_STC commonStdIp;

    /** Ethernet Destination MAC address */
    GT_ETHERADDR macDa;

    GT_U8 sipBits[4];

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb4_7   - User Defined Bytes 4-7
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EGR_STD_IP_L2_QOS_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_STD_IPV4_L4_STC
 *
 * @brief This structure describes the standard egress IPV4_L4 key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC common;

    /** common part for all standard IP formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STD_IP_STC commonStdIp;

    /** ethernet broadcast packet */
    GT_U8 isBc;

    /** IPv4 source IP address field */
    GT_IPADDR sip;

    /** IPv4 destination IP address field */
    GT_IPADDR dip;

    /** L4 header byte 0 */
    GT_U8 l4Byte0;

    /** L4 header byte 1 */
    GT_U8 l4Byte1;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb8_11   - User Defined Bytes 8-11
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EGR_STD_IPV4_L4_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_NOT_IPV6_STC
 *
 * @brief This structure describes the egress extended not-IPV6 key.
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC common;

    /** common part for all extended formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_EXT_STC commonExt;

    /** IPv4 source IP address field. */
    GT_IPADDR sip;

    /** IPv4 header destination IP address field */
    GT_IPADDR dip;

    /** ether type */
    GT_U16 etherType;

    /** L2 encapsulation of the packet */
    GT_U8 l2Encap;

    /** ethernet Dst MAC address */
    GT_ETHERADDR macDa;

    /** ethernet Src MAC address */
    GT_ETHERADDR macSa;

    /** @brief indicates IPv4 fragment
     *  ipv4Options_Ext - indicates that the IPv4 header has an option field
     *  (APPLICABLE DEVICES: Puma2)
     */
    GT_U8 ipv4Fragmented;

    GT_U8 ipv4Options;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 cfi1;

    /** @brief (APPLICABLE DEVICES: Lion2)
     *  1 - MPLS, 0 - not MPLS.
     */
    GT_U8 isMpls;

    /** @brief (APPLICABLE DEVICES: Lion2)
     *  number of MPLS Labels (0..3).
     */
    GT_U32 numOfMplsLabels;

    /** @brief (APPLICABLE DEVICES: Lion2)
     *  inner protocol type (2-bit value)
     */
    GT_U32 protocolTypeAfterMpls;

    /** (APPLICABLE DEVICES: Lion2) MPLS Label0. */
    GT_U32 mplsLabel0;

    /** (APPLICABLE DEVICES: Lion2) MPLS Exp0. */
    GT_U32 mplsExp0;

    /** (APPLICABLE DEVICES: Lion2) MPLS Label1. */
    GT_U32 mplsLabel1;

    /** (APPLICABLE DEVICES: Lion2) MPLS Exp1. */
    GT_U32 mplsExp1;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     */
    GT_U8 qInQ;

    /** @brief Inner VID incase of QinQ packet. (only for external key)
     *  Valid when <QinQ> = 1
     *  (APPLICABLE DEVICES: ExMxPm)
     */
    GT_U16 innerVid_Extrn;

    /** @brief Inner VID incase of QinQ packet.
     *  Valid when <QinQ> = 1
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb12_19     - User Defined Bytes 12-19
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U16 innerVid;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_NOT_IPV6_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_IPV6_L2_STC
 *
 * @brief This structure describes the egress extended IPV6+L2 key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC common;

    /** common part for all extended formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field */
    GT_IPV6ADDR sip;

    /** IPV6 destination address highest 8 bits */
    GT_U8 dipBits;

    /** indicates that an IPv6 extension exists */
    GT_U8 isIpv6Eh;

    /** ethernet Dst MAC address */
    GT_ETHERADDR macDa;

    /** ethernet Src MAC address */
    GT_ETHERADDR macSa;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 cfi1;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     */
    GT_U8 qInQ;

    /** @brief Inner VID incase of QinQ packet. (only for external key)
     *  Valid when <QinQ> = 1
     *  (APPLICABLE DEVICES: ExMxPm)
     */
    GT_U16 innerVid_Extrn;

    /** @brief Inner VID incase of QinQ packet.
     *  Valid when <QinQ> = 1
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb20_27 - User Defined Bytes 20-27
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U16 innerVid;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_IPV6_L2_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_IPV6_L4_STC
 *
 * @brief This structure describes the egress extended IPV6+L4 key
*/
typedef struct{

    /** common part for all formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_STC common;

    /** common part for all extended formats */
    PRV_TGF_PCL_RULE_FORMAT_EGR_COMMON_EXT_STC commonExt;

    /** IPv6 source IP address field */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field */
    GT_IPV6ADDR dip;

    /** indicates that an IPv6 extension exists */
    GT_U8 isIpv6Eh;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     */
    GT_U8 qInQ;

    /** @brief Inner VID incase of QinQ packet. (only for external key)
     *  Valid when <QinQ> = 1
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb28_35 - User Defined Bytes 28-35
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U16 innerVid_Extrn;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_IPV6_L4_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_STD_UDB_STC
 *
 * @brief PCL Rule ingress standard "UDB" Key fields
*/
typedef struct{

    /** fields common for all ingress standard keys */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC commonStd;

    /** fields common for ingress "UDB" styled keys */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_UDB_STC commonUdb;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** VRF Id (APPLICABLE DEVICES: xCat, Lion, Lion2) */
    GT_U32 vrfId;

    /** QoS Profile(APPLICABLE DEVICES: xCat, Lion, xCat2, Lion2) */
    GT_U32 qosProfile;

    /** Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2) */
    GT_U32 trunkHash;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb35_38    - User Defined Bytes 35-38
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_STD_UDB_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EXT_UDB_STC
 *
 * @brief PCL Rule ingress extended "UDB" Key fields
*/
typedef struct{

    /** @brief fields common for all ingress standard keys
     *  commonIngrExt    - fields common for all ingress extended keys
     */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STC commonStd;

    PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** fields common for ingress "UDB" styled keys */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_UDB_STC commonIngrUdb;

    GT_U8 macSaOrSipBits79_32[6];

    GT_U8 macDaOrSipBits127_80[6];

    GT_U8 sipBits31_0[4];

    GT_U8 dipBits31_0[4];

    GT_U8 dipBits127_112[2];

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 cfi1;

    /** VRF Id (APPLICABLE DEVICES: xCat, Lion, Lion2) */
    GT_U32 vrfId;

    /** QoS Profile(APPLICABLE DEVICES: xCat, Lion, xCat2, Lion2) */
    GT_U32 qosProfile;

    /** Trunk Hash (APPLICABLE DEVICES: Lion, xCat2, Lion2) */
    GT_U32 trunkHash;

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     *  udb23_30       - User Defined Bytes 23-30
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_EXT_UDB_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EXT_80B_STC
 *
 * @brief PCL Rule ingress external-only 80-bytes Key fields
*/
typedef struct{

    /** fields common for all ingress extended keys */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC commonExt;

    /** fields common for ingress "UDB" styled keys */
    PRV_TGF_PCL_RULE_FORMAT_COMMON_STD_UDB_STC commonIngrUdb;

    /** ether type */
    GT_U16 ethType;

    /** ethernet DstMAC address */
    GT_ETHERADDR macDa;

    /** ethernet Src MAC address */
    GT_ETHERADDR macSa;

    /** src IP address of IPV6 packet */
    GT_IPV6ADDR sip;

    /** dst IP address of IPV6 packet */
    GT_IPV6ADDR dip;

    /** L4 header byte 0 */
    GT_U8 l4Byte0;

    /** L4 header byte 1 */
    GT_U8 l4Byte1;

    /** L4 header byte 2 */
    GT_U8 l4Byte2;

    /** L4 header byte 3 */
    GT_U8 l4Byte3;

    /** L4 header byte 13 */
    GT_U8 l4Byte13;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** @brief Is packet QInQ (802.1ad Provider Bridging)
     *  1 - packet has more then one VLAN tag
     *  0 - packet untagged or has only one VLAN tag
     *  (APPLICABLE DEVICES: ExMxPm)
     */
    GT_U8 qInQ;

} PRV_TGF_PCL_RULE_FORMAT_EXT_80B_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_IP_COMMON_STC
 *
 * @brief This structure describes the common fields of ingress keys.
 * The fields placed at the same bits of the keys.
*/
typedef struct{

    /** @brief The PCL
     *  Only 2 MSBs of pclId are used when portListBmp is used
     *  (see notes below about portListBmp).
     */
    GT_U32 pclId;

    /** @brief MAC To Me
     *  1 - packet matched by MAC-To-Me lookup, 0 - not matched
     *  The field relevant only for DxCh3 and above devices.
     *  DxCh1 and DxCh2 ignore the field
     */
    GT_U8 macToMe;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     *  Field muxing description(APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourcePort is muxed with portListBmp[15:8]
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** @brief Together with sourcePort indicates the network port at which the packet
     *  was received.
     *  Field muxing description(APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon):
     *  sourceDevice is muxed with portListBmp[27:16].
     */
    GT_U32 sourceDevice;

    /** @brief Port list bitmap. (APPLICABLE DEVICES: xCat2, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon).
     *  Relevant only when PCL lookup works in port-list mode.
     *  Field muxing description:
     *  portListBmp[7:0] is muxed with pclId[7:0],
     *  portListBmp[15:8] is muxed with sourcePort[7:0]
     *  portListBmp[27:16] is muxed with sourceTrunkId (APPLICABLE DEVICES: Lion2)
     *  or sourceDevice (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon).
     *  The portListBmp bits in rule's mask and pattern
     *  should be set to 0 (don't care) for matched ports.
     *  The portListBmp bits in rule's mask should be 1 and
     *  pattern should be set to 0 for not matched ports.
     *  Applying a rule for a packet received from the CPU requires
     *  the portListBmp pattern with all bits equal to zero. Hence
     *  rules cannot be shared between the CPU port and other ports
     *  in port-list mode.
     */
    CPSS_PORTS_BMP_STC portListBmp;

    /** @brief Flag indicating the packet Tag state
     *  For Ingress PCL Keys:
     *  For non-tunnel terminated packets:
     *  - If packet was received on DSA-tagged, this field is set
     *  to the DSA tag <SrcTagged> field.
     *  - If packet was received non-DSA-tagged, this field is set
     *  to 1 if the packet was received VLAN or Priority-tagged.
     *  For Ethernet-Over-xxx tunnel-terminated packets: Passenger
     *  packet VLAN tag format.
     */
    GT_U8 isTagged;

    /** VLAN ID assigned to the packet. */
    GT_U16 vid;

    /** The packet's 802.1p User Priority field. */
    GT_U32 up;

    /** 8 */
    GT_U32 tos;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** see in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 isL4Valid;

    /** see in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte0;

    /** see in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte1;

    /** see in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte2;

    /** see in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC */
    GT_U8 l4Byte3;

    /** @brief see in PRV_TGF_PCL_RULE_FORMAT_COMMON_EXT_STC
     *  Comments:
     */
    GT_U8 l4Byte13;

} PRV_TGF_PCL_RULE_FORMAT_IP_COMMON_STC;
/**
* @struct PRV_TGF_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
 *
 * @brief This structure describes the ingress Standard Routed Qos IPV4 Key.
 * CH3 : ingress standard IPV4 key format, used in first lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    PRV_TGF_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief packet type
     *  0 = Non of the following.
     *  1 = ARP.
     *  2 = IPv6 Over MPLS.
     *  3 = IPv4 Over MPLS.
     *  4 = MPLS.
     *  6 = IPv6.
     *  7 = IPv4.
     */
    GT_U32 pktType;

    /** @brief IP Fragmented
     *  0 = no fragment
     *  1 = first fragment
     *  2 = mid fragment
     *  3 = last fragment
     */
    GT_U8 ipFragmented;

    /** @brief IP Header information
     *  0 = Normal
     *  1 = IP options present
     *  2 = IP header validation fail
     *  3 = Is small offset
     */
    GT_U32 ipHeaderInfo;

    /** IP Packet length 14 */
    GT_U32 ipPacketLength;

    /** time to live */
    GT_U32 ttl;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[7:0] muxed with UDB 5
     */
    GT_U32 vrfId;

    /** @brief The transmitted packet TCP/UDP
     *  comparator result.
     *  Supported only for DxCh3 device and ignored for other.
     */
    GT_U32 tcpUdpPortComparators;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  udb5    - UDB5
     *  Supported only for DxChXCat and above devices
     *  and ignored for DxCh3 devices.
     *  udb41_44  - User Defined Bytes 41-44
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 isUdbValid;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
 *
 * @brief This structure describes the ingress Extended Port/VLAN Qos IPV4 Key.
 * CH3 : ingress extended IPV4 key format, used in second lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    PRV_TGF_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** @brief Ethernet Broadcast packet.
     *  Valid when <IsL2Valid> =1.
     *  Indicates an Ethernet Broadcast packet
     *  (<MAC_DA> == FF:FF:FF:FF:FF:FF).
     *  0 = MAC_DA is not Broadcast.
     *  1 = MAC_DA is Broadcast.
     */
    GT_U8 isBc;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief IP Fragmented
     *  0 = no fragment
     *  1 = first fragment
     *  2 = mid fragment
     *  3 = last fragment
     */
    GT_U8 ipFragmented;

    /** @brief IP Header information
     *  0 = Normal
     *  1 = IP options present
     *  2 = IP header validation fail
     *  3 = Is small offset
     */
    GT_U32 ipHeaderInfo;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[11:8] muxed with UDB 14
     *  vrfId[7:0] muxed with UDB 13
     */
    GT_U32 vrfId;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 15
     */
    GT_U32 trunkHash;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbStdIpL2Qos[2];

    GT_U8 UdbStdIpV4L4[3];

    GT_U8 UdbExtIpv6L2[4];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
 *
 * @brief This structure describes the ingress Ultra Port/VLAN Qos IPV6 Key.
 * CH3: ingress ultra IPV6 L2 key format, used in first lookup.
*/
typedef struct{

    /** The common fields of ingress IP keys. */
    PRV_TGF_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** IS ND */
    GT_U8 isNd;

    /** Ethernet Broadcast packet. */
    GT_U8 isBc;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief The Layer 2 encapsulation of the packet.
     *  0 = The L2 Encapsulation is LLC NON-SNAP.
     *  1 = The L2 Encapsulation is Ethernet V2 or LLC with SNAP.
     */
    GT_U32 l2Encap;

    /** @brief Valid when <IsL2Valid> =1.
     *  If <L2 Encap Type> = 0, this field contains the
     *  <Dsap-Ssap> of the LLC NON-SNAP packet.
     *  If <L2 Encap Type> = 1, this field contains the <EtherType/>
     *  of the Ethernet V2 or LLC with SNAP packet.
     */
    GT_U16 etherType;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** @brief IPv6 destination IP address field.
     *  For ARP packets this field holds the target IPv6 address.
     */
    GT_IPV6ADDR dip;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** @brief Indicates a valid IP header.
     *  0 = Packet IP header is invalid.
     *  1 = Packet IP header is valid.
     */
    GT_U8 ipHeaderOk;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[11:8] muxed with UDB 0
     *  vrfId[7:0] muxed with UDB 12
     */
    GT_U32 vrfId;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbExtNotIpv6[4];

    GT_U8 UdbExtIpv6L2[5];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** @brief (The source port or trunk assigned to the packet.
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     */
    GT_U32 srcPortOrTrunk;

    /** @brief Indicates packets received from a Trunk on a
     *  remote device or on the local device
     *  0 = Source is not a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is not a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD
     *  and DSA<SrcIsTrunk> =0
     *  1 = Source is a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD and
     *  DSA<SrcIsTrunk =1
     *  Relevant only when packet is of type is FORWARD.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 srcIsTrunk;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 1
     *  Comments:
     */
    GT_U32 trunkHash;

} PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
 *
 * @brief This structure describes the ingress Ultra Routed Qos IPV6 Key.
 * CH3 : ingress ultra IPV6 MPLS key format, used in second lookup.
*/
typedef struct{

    /** The common fields of all ingress keys. */
    PRV_TGF_PCL_RULE_FORMAT_IP_COMMON_STC ingressIpCommon;

    /** @brief packet type
     *  0 = Non of the following.
     *  1 = ARP.
     *  2 = IPv6 Over MPLS.
     *  3 = IPv4 Over MPLS.
     *  4 = MPLS.
     *  6 = IPv6.
     *  7 = IPv4.
     */
    GT_U32 pktType;

    /** IS ND */
    GT_U8 isNd;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tag1Exist;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** MPLS Outer Label */
    GT_U32 mplsOuterLabel;

    /** MPLS Outer Label EXP */
    GT_U32 mplsOuterLabExp;

    /** MPLS Outer Label S */
    GT_U8 mplsOuterLabSBit;

    /** IP Packet Length */
    GT_U32 ipPacketLength;

    /** IPv6 Header <Flow Label> */
    GT_U32 ipv6HdrFlowLabel;

    /** time to live */
    GT_U32 ttl;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** @brief IPv6 destination IP address field.
     *  For ARP packets this field holds the target IPv6 address.
     */
    GT_IPV6ADDR dip;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** Is IPv6 Link local */
    GT_U8 isIpv6LinkLocal;

    /** Is IPv6 MLD */
    GT_U8 isIpv6Mld;

    /** Indicates a valid IP header. */
    GT_U8 ipHeaderOk;

    /** @brief VRF Id (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  vrfId[11:8] muxed with UDB 19
     *  vrfId[7:0] muxed with UDB 18
     */
    GT_U32 vrfId;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     */
    GT_U8 isUdbValid;

    GT_U8 UdbStdNotIp[2];

    GT_U8 UdbStdIpL2Qos[3];

    GT_U8 UdbStdIpV4L4[3];

    GT_U8 UdbExtNotIpv6[4];

    GT_U8 UdbExtIpv6L2[3];

    GT_U8 UdbExtIpv6L4[3];

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** @brief (The source port or trunk assigned to the packet.
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     */
    GT_U32 srcPortOrTrunk;

    /** @brief Indicates packets received from a Trunk on a
     *  remote device or on the local device
     *  0 = Source is not a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is not a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD
     *  and DSA<SrcIsTrunk> =0
     *  1 = Source is a Trunk due to one of the following:
     *  - A non-DSA-tagged packet received from a network port
     *  which is a trunk member
     *  - A DSA-tagged packet with DSA<TagCmd> =FORWARD and
     *  DSA<SrcIsTrunk =1
     *  Relevant only when packet is of type is FORWARD.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 srcIsTrunk;

    /** @brief Trunk Hash (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  trunkHash muxed with UDB 1
     *  Comments:
     */
    GT_U32 trunkHash;

} PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC
 *
 * @brief This structure describes the common part of egress IP keys.
 * The fields placed at the same bits of the keys.
*/
typedef struct{

    /** The PCL */
    GT_U32 pclId;

    /** @brief The port number from which the packet ingressed the device.
     *  Port 63 is the CPU port.
     */
    GT_PHYSICAL_PORT_NUM sourcePort;

    /** The QoS Profile assigned to the packet until this lookup */
    GT_U32 qosProfile;

    /** @brief Indicates that Layer 2 information in the search key is valid.
     *  0 = Layer 2 fields in the Ingress PCL (IPCL) or
     *  Egress PCL (EPCL) are not valid
     *  1 = Layer 2 field in the Ingress PCL or Egress PCL key is valid.
     *  For Ingress PCL Keys: Layer 2 fields are not valid in the
     *  IPCL keys when IP-Over-xxx is tunnel-terminated and its key
     *  is based on the passenger IP packet which does not include
     *  a Layer 2 header.
     *  For Egress PCL Keys: Layer 2 fields are not valid in
     *  the EPCL keys when an IP-Over-xxx is forwarded to a Tunnel
     *  Start, and its PCL Key is based on the passenger IP packet
     *  which does not include Layer 2 data.
     */
    GT_U8 isL2Valid;

    /** @brief original VLAN ID assigned to the packet.
     *  For DxCh2, DxCh3, xCat, Lion, xCat2, Lion2
     *  VLAN Id is 12 bit.
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices support of 16-bit VLAN Id.
     */
    GT_U32 originalVid;

    /** @brief Is Source information (srcDevOrTrunkId) is Trunk ID
     *  0 = srcDevOrTrunkId is source device number
     *  1 = srcDevOrTrunkId is trunk ID
     */
    GT_U8 isSrcTrunk;

    /** @brief Source device number or trunk ID. See isSrcTrunk.
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices used only for TrunkId,
     *  for srcHwDev value see separate field srcHwDev.
     */
    GT_U32 srcDevOrTrunkId;

    /** @brief Together with <SrcPort> and <srcHwDev> indicates the
     *  network port at which the packet was received.
     *  Relevant for all packet types exclude the
     *  FWD_DATA packed entered to the PP from Trunk
     *  Fields srcTrunkId and srcHwDev are muxed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_HW_DEV_NUM srcHwDev;

    /** @brief An indication that the packet is IP
     *  0 = Non IP packet.
     *  1 = IPv4/6 packet.
     */
    GT_U8 isIp;

    /** @brief An indication that the packet is an ARP packet
     *  (identified by EtherType == 0x0806.)
     *  0 = Non ARP packet.
     *  1 = ARP packet.
     */
    GT_U8 isArp;

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief 0
     *  1 - Tag1 does not exist in packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 tag1Exist;

    /** @brief The Source ID assigned to the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 sourceId;

    /** 8 */
    GT_U32 tos;

    /** IP protocol/Next Header type. */
    GT_U32 ipProtocol;

    /** time to live */
    GT_U32 ttl;

    /** @brief Layer 4 information is valid.
     *  This field indicates that all the Layer 4
     *  information required for the search key is available
     *  and the IP header is valid.
     *  0 = Layer 4 information is not valid.
     *  1 = Layer 4 information is valid.
     *  Layer 4 information may not be available for any of the
     *  following reasons:
     *  - Layer 4 information is not included in the packet.
     *  For example, Layer 4 information
     *  isn't available in non-IP packets, or in IPv4
     *  non-initial-fragments.
     *  - Parsing failure: Layer 4 information is beyond
     *  the first 128B of the packet, or beyond
     *  IPv6 extension headers parsing capabilities.
     *  - IP header is invalid.
     */
    GT_U8 isL4Valid;

    /** @brief The following Layer 4 information is available for
     *  UDP and TCP packets - L4 Header Byte0 through Byte3,
     *  which contain the UDP/TCP destination and source ports.
     *  For TCP also L4 Header Byte13, which contains the TCP flags.
     *  For IGMP L4 Header Byte0, which contain the IGMP Type.
     *  For ICMP L4 Header Byte0 and Byte1,
     *  which contain the ICMP Type and Code fields
     *  and L4 Header Byte4<in l4Byte2> and Byte5 <in l4Byte3>,
     *  which contain the ICMP authentication field.
     *  The following Layer 4 information is available for packets
     *  that are not TCP, UDP, IGMP or ICMP:
     *  L4 Header Byte0 through Byte3 L4 Header Byte13
     */
    GT_U8 l4Byte0;

    /** see l4Byte0 */
    GT_U8 l4Byte1;

    /** see l4Byte0 */
    GT_U8 l4Byte2;

    /** see l4Byte0 */
    GT_U8 l4Byte3;

    /** see l4Byte0 */
    GT_U8 l4Byte13;

    /** The transmitted packet TCP/UDP comparator result */
    GT_U32 tcpUdpPortComparators;

    /** traffic class 3 */
    GT_U32 tc;

    /** drop priority 2 */
    GT_U32 dp;

    /** @brief two bits field:
     *  0 - packet to CPU
     *  1 - packet from CPU
     *  2 - packet to ANALYZER
     *  3 - forward DATA packet
     */
    GT_U32 egrPacketType;

    /** @brief Source/Target or Tx Mirror
     *  For TO CPU packets this field is <SrcTrg> of DSA tag.
     *  For TO_TARGET_SNIFFER packets this field is inverted
     *  <RxSniff> of DSA tag.
     *  For FORWARD and FROM_CPU packets this field is 0
     */
    GT_U8 srcTrgOrTxMirror;

    /** 3 */
    GT_U32 assignedUp;

    /** target physical port */
    GT_PHYSICAL_PORT_NUM trgPhysicalPort;

    /** @brief Indicates if this packet is Ingress or Egress
     *  mirrored to the Analyzer.
     *  0 = This packet is Egress mirrored to the analyzer.
     *  1 = This packet is Ingress mirrored to the analyzer.
     *  Relevant only when packet is of type is TO_ANALYZER.
     */
    GT_U8 rxSniff;

    /** @brief Indicates that the packet has been routed either by the
     *  local device or some previous device in a cascaded system.
     *  0 = The packet has not been routed.
     *  1 = The packet has been routed.
     *  Relevant only when packet is of type is FORWARD.
     */
    GT_U8 isRouted;

    /** @brief An indication that the packet is IPv6.
     *  0 = Non IPv6 packet.
     *  1 = IPv6 packet.
     */
    GT_U8 isIpv6;

    /** @brief The CPU Code forwarded to the CPU.
     *  Relevant only when packet is of type is TO_CPU.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cpuCode;

    /** @brief The <SrcTrg> field extracted from the TO_CPU DSA<SrcTrg>:
     *  0 = The packet was sent to the CPU by the ingress pipe and
     *  the DSA tag contain attributes related to the
     *  packet ingress.
     *  1 = The packet was sent to the CPU by the egress pipe
     *  and the DSA tag contains attributes related to the
     *  packet egress.
     *  Relevant only when packet is of type is TO_CPU.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    GT_U8 srcTrg;

    /** @brief The <EgressFilterEn> extracted from
     *  FROM_CPU DSA<EgressFilterEn>:
     *  0 = FROM_CPU packet is subject to egress filtering,
     *  e.g. data traffic from the CPU
     *  1 = FROM_CPU packet is not subject egress filtering,
     *  e.g. control traffic from the CPU
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  Comments:
     */
    GT_U8 egrFilterEnable;

} PRV_TGF_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC
 *
 * @brief This structure describes the Egress Extended (48B) RACL/VACL IPv4 key format.
 * CH3 : egress extended (48B) IPV4 key format.
*/
typedef struct{

    /** The common fields of egress IP keys. */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC egressIpCommon;

    /** @brief IPv4 source IP address field.
     *  For ARP packets this field holds the sender's IPv4 address.
     */
    GT_IPADDR sip;

    /** @brief IPv4 destination IP address field.
     *  For ARP packets this field holds the target IPv4 address.
     */
    GT_IPADDR dip;

    /** @brief IP V4 Options
     *  For non-tunnel start packets:
     *  0 = The transmitted packet is non-Ipv4 or an IPv4 options
     *  do not exist.
     *  1 = The transmitted packet is IPv4 and an IPv4 options
     *  exist.
     *  For tunnel-start packets:
     *  - always 0
     */
    GT_U8 ipv4Options;

    /** @brief Indicates that a packet is forwarded to a Multicast group.
     *  0 = The packet is a Unicast packet forwarded to a
     *  specific target port or trunk.
     *  1 = The packet is a multi-destination packet forwarded
     *  to a Multicast group.
     *  Relevant only for DxChXcat and above devices.
     */
    GT_U8 isVidx;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     *  udb36_49  - UDBs 36-49 (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     *  udb0    - UDB 0 (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  Comments:
     */
    GT_U8 isUdbValid;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC
 *
 * @brief This structure describes the Egress Ultra (72B) RACL/VACL IPv6 key format.
 * CH3 : egress Ultra (72B) IPV6 key format.
*/
typedef struct{

    /** The common fields of egress IP keys. */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC egressIpCommon;

    /** @brief IPv6 source IP address field.
     *  For ARP packets this field holds the sender's IPv6 address.
     */
    GT_IPV6ADDR sip;

    /** IPv6 destination IP address field. */
    GT_IPV6ADDR dip;

    /** Is ND */
    GT_U8 isNd;

    /** @brief IPv6 extension header exists.
     *  Indicates that an IPv6 extension exists.
     *  0 = Non-Ipv6 packet or IPv6 extension header does
     *  not exist.
     *  1 = Packet is IPv6 and extension header exists.
     */
    GT_U8 isIpv6ExtHdrExist;

    /** @brief Indicates that the IPv6 Original Extension Header
     *  is Hop-by-Hop.
     *  0 = Non-IPv6 packet or IPv6 extension header type is
     *  not Hop-by-Hop Option Header.
     *  1 = Packet is IPv6 and extension header type
     *  is Hop-by-Hop Option Header.
     */
    GT_U8 isIpv6HopByHop;

    /** @brief Indicates that a packet is forwarded to a Multicast group.
     *  0 = The packet is a Unicast packet forwarded to a
     *  specific target port or trunk.
     *  1 = The packet is a multi-destination packet forwarded
     *  to a Multicast group.
     *  Relevant only for DxChXcat and above devices.
     */
    GT_U8 isVidx;

    /** @brief VID from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 vid1;

    /** @brief UP from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 up1;

    /** @brief CFI from Tag1, valid only if Tag1 exist.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 cfi1;

    /** @brief Source port assign to the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 srcPort;

    /** @brief Egress port of the packet.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 trgPort;

    /** @brief User
     *  Indicates that all user-defined bytes used in that search
     *  key were successfully parsed.
     *  0 = At least 1 user-defined byte couldn't be parsed.
     *  1 = All user-defined bytes used in this search key
     *  were successfully parsed.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon)
     *  udb1_4   - UDBs 1-4 (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     *  Comments:
     */
    GT_U8 isUdbValid;

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

} PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC
 *
 * @brief This structure describes the Ingress UDB only keys fields
 * replacing the user defined bytes and being enabled/disabled
 * by cpssDxChPclUserDefinedBytesSelectSet.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief pclId UDB0 muxed with pclId[7:0],
     *  UDB1[1:0] muxed with pclId[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief UDB Valid: 1 when all UDBs in the key valid, otherwise 0,
     *  muxed with UDB1[7]
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief eVlan, UDB2 muxed with vid[7:0],
     *  UDB3 muxed with vid[12:8]
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort, UDB4 muxed with srcPort[7:0],
     *  UDB5 muxed with srcPort[12:8]
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 srcPort;

    /** @brief source Device Is Own
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 srcDevIsOwn;

    /** @brief tag1 VID, UDB6 muxed with vid1[7:0],
     *  UDB7[3:0] muxed with vid1[11:8]
     *  (APPLICABLE RANGES: 0..0xFFF)
     */
    GT_U32 vid1;

    /** @brief tag1 UP, UDB7[6:4] muxed with up1[2:0]
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 up1;

    /** @brief mac to me, UDB7[7] muxed with mac2me
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 macToMe;

    /** @brief qosProfile, UDB8 muxed with qosProfile[7:0],
     *  UDB9[1:0] muxed with qosProfile[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 qosProfile;

    /** @brief flow Id, UDB10 muxed with flowId[7:0]
     *  UDB11[3:0] muxed with flowId[11:8]
     *  (APPLICABLE RANGES: 0..0xFFF)
     *  Comments:
     */
    GT_U32 flowId;

} PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC
 *
 * @brief This structure describes the Ingress UDB60 keys fixed fields.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief UDB Valid
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief PCL
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief eVlan
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 srcPort;

    /** @brief source Device Is Own
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 srcDevIsOwn;

    /** @brief tag1 VID
     *  (APPLICABLE RANGES: 0..0xFFF)
     */
    GT_U32 vid1;

    /** @brief tag1 UP
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 up1;

    /** @brief mac to me
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 macToMe;

    /** @brief qosProfile
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 qosProfile;

    /** @brief flow Id
     *  (APPLICABLE RANGES: 0..0xFFF)
     *  Comments:
     */
    GT_U32 flowId;

} PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC
 *
 * @brief This structure describes the Ingress UDB only keys formats.
 * PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E
 * PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E
 * PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E
 * PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E
 * PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E
 * PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** replaced fixed fields. */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_REPLACED_STC replacedFld;

    /** @brief fixed fields,
     *  relevant only for PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E keys.
     *  Comments:
     */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB60_FIXED_STC udb60FixedFld;

} PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC
 *
 * @brief This structure describes the Egress UDB only keys fields
 * replacing the user defined bytes and being enabled/disabled
 * by cpssDxChPclUserDefinedBytesSelectSet.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief PCL
     *  UDB1[1:0] muxed with PCL-ID[9:8]
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief UDB Valid: 1 when all UDBs in the key valid, otherwise 0,
     *  muxed with UDB1[7]
     *  (APPLICABLE RANGES: 0..1)
     *  Comments:
     */
    GT_U8 isUdbValid;

} PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC
 *
 * @brief This structure describes the Egress UDB60 keys fixed fields.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    /** @brief PCL
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 pclId;

    /** @brief UDB Valid
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U8 isUdbValid;

    /** @brief eVlan
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 vid;

    /** @brief source ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 srcPort;

    /** @brief target ePort
     *  (APPLICABLE RANGES: 0..0x1FFF)
     */
    GT_U32 trgPort;

    /** @brief source Device
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 srcDev;

    /** @brief target Device
     *  (APPLICABLE RANGES: 0..0x3FF)
     */
    GT_U32 trgDev;

    /** @brief local device target physical port
     *  (APPLICABLE RANGES: 0..0xFF)
     *  Comments:
     */
    GT_PHYSICAL_PORT_NUM localDevTrgPhyPort;

} PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC;

/**
* @struct PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC
 *
 * @brief This structure describes the Egress UDB only keys formats.
 * PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E
 * PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E
 * PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E
 * PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E
 * PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E
 * PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
typedef struct{

    GT_U8 udb[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** replaced fixed fields. */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_REPLACED_STC replacedFld;

    /** @brief fixed fields,
     *  relevant only for PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E keys.
     *  Comments:
     */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB60_FIXED_STC udb60FixedFld;

} PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC;

/**
* @union PRV_TGF_PCL_RULE_FORMAT_UNT
 *
 * @brief This union describes the PCL key.
 *
*/

typedef union{
    /** Standard Not IP packet key */
    PRV_TGF_PCL_RULE_FORMAT_STD_NOT_IP_STC ruleStdNotIp;

    /** @brief Standard IPV4 and IPV6 packets L2 and QOS
     *  styled key
     */
    PRV_TGF_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC ruleStdIpL2Qos;

    /** Standard IPV4 packet L4 styled key */
    PRV_TGF_PCL_RULE_FORMAT_STD_IPV4_L4_STC ruleStdIpv4L4;

    /** Standard IPV6 packet DIP styled key */
    PRV_TGF_PCL_RULE_FORMAT_STD_IPV6_DIP_STC ruleStdIpv6Dip;

    /** Extended Not IP and IPV4 packet key */
    PRV_TGF_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC ruleExtNotIpv6;

    /** Extended IPV6 packet L2 styled key */
    PRV_TGF_PCL_RULE_FORMAT_EXT_IPV6_L2_STC ruleExtIpv6L2;

    /** Extended IPV6 packet L2 styled key */
    PRV_TGF_PCL_RULE_FORMAT_EXT_IPV6_L4_STC ruleExtIpv6L4;

    /** Egress Standard Not IP packet key */
    PRV_TGF_PCL_RULE_FORMAT_EGR_STD_NOT_IP_STC ruleEgrStdNotIp;

    /** @brief Egress Standard IPV4 and IPV6 packets L2 and
     *  QOS styled key
     */
    PRV_TGF_PCL_RULE_FORMAT_EGR_STD_IP_L2_QOS_STC ruleEgrStdIpL2Qos;

    /** Egress Standard IPV4 packet L4 styled key */
    PRV_TGF_PCL_RULE_FORMAT_EGR_STD_IPV4_L4_STC ruleEgrStdIpv4L4;

    /** Egress Extended Not IP and IPV4 packet key */
    PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_NOT_IPV6_STC ruleEgrExtNotIpv6;

    /** Egress Extended IPV6 packet L2 styled key */
    PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_IPV6_L2_STC ruleEgrExtIpv6L2;

    /** Egress Extended IPV6 packet L2 styled key */
    PRV_TGF_PCL_RULE_FORMAT_EGR_EXT_IPV6_L4_STC ruleEgrExtIpv6L4;

    /** Ingress Standard UDB styled key */
    PRV_TGF_PCL_RULE_FORMAT_STD_UDB_STC ruleStdUdb;

    /** Ingress Extended UDB styled key */
    PRV_TGF_PCL_RULE_FORMAT_EXT_UDB_STC ruleExtUdb;

    /** Ingress External only 80 */
    PRV_TGF_PCL_RULE_FORMAT_EXT_80B_STC ruleExtUdb80B;

    /** Ingress Standard Routed Qos IPV4 Key */
    PRV_TGF_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC ruleStdIpv4RoutedAclQos;

    /** Ingress Extended Port/VLAN Qos IPV4 Key */
    PRV_TGF_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC ruleExtIpv4PortVlanQos;

    /** Ingress Ultra Port/VLAN Qos IPV6 key */
    PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC ruleUltraIpv6PortVlanQos;

    /** Ingress Ultra Routed Qos IPV6 key */
    PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC ruleUltraIpv6RoutedAclQos;

    /** Egress Extended (48B) RACL/VACL IPv4 key */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC ruleEgrExtIpv4RaclVacl;

    /** Egress Ultra (72B) RACL/VACL IPv6 key */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC ruleEgrUltraIpv6RaclVacl;

    /** @brief Ingress UDB only formats.
     *  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E
     *  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E
     *  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E
     *  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E
     *  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E
     *  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E
     */
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC ruleIngrUdbOnly;

    /** @brief Egress UDB only formats.
     *  PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E
     *  PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E
     *  PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E
     *  PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E
     *  PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E
     *  PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E
     */
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC ruleEgrUdbOnly;

} PRV_TGF_PCL_RULE_FORMAT_UNT;


/**
* @struct PRV_TGF_PCL_UDB_SELECT_STC
 *
 * @brief This structure defines User Defined Bytes Selection
 * for UDB only keys.
 * (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
*/
typedef struct{
    /** @brief The UDB number in the global UDB pool that
     *  is selected as UDB number-n in this UDB only key.
     *  The index in this array is the number of UDB-position
     *  in appropriate UDB only key.
     */
    GT_U32 udbSelectArr[PRV_TGF_PCL_UDB_MAX_NUMBER_CNS];

    /** @brief enable replacing UDBs in the key by specific not-UDB data.
     * GT_TRUE - enable, GT_FALSE - disable.
     * Relevant for Ingress PCL only.
     * udbReplaceArr[0]  - UDB0  replaced with {PCL-ID[7:0]}
     * udbReplaceArr[1]  - UDB1  replaced with {UDB Valid,reserved,PCL-ID[9:8]}
     * udbReplaceArr[2]  - UDB2  replaced with eVLAN[7:0]
     * udbReplaceArr[3]  - UDB3  replaced with eVLAN[12:8]
     * udbReplaceArr[4]  - UDB4  replaced with Source-ePort[7:0]
     * udbReplaceArr[5]  - UDB5  replaced with Source-ePort[12:8]
     * udbReplaceArr[6]  - UDB6  replaced with Tag1-VID[7:0]
     * udbReplaceArr[7]  - UDB7  replaced with MAC2ME and Tag1-{UP[2:0],VID[11:8]}
     * udbReplaceArr[8]  - UDB8  replaced with QoS-Profile[7:0]
     * udbReplaceArr[9]  - UDB9  replaced with QoS-Profile[9:8]
     * udbReplaceArr[10] - UDB10 replaced with Flow-ID[7:0]
     * udbReplaceArr[11] - UDB11 replaced with Flow-ID[11:8]
     */
    GT_BOOL ingrUdbReplaceArr[PRV_TGF_PCL_INGRESS_UDB_REPLACE_MAX_CNS];

    /** @brief UDBs 0,1 are replaced with PCL-ID[9:0] padded with leading zeros.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  Relevant for Egress PCL only.
     */
    GT_BOOL egrUdb01Replace;

    /** @brief <UDB Valid> indication replaces bit 15 of the key,
     *  overriding any other configuration of bit 15 content,
     *  including the configuration in egrUdb01Replace.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  Relevant for Egress PCL only.
     *  Comments:
     */
    GT_BOOL egrUdbBit15Replace;

} PRV_TGF_PCL_UDB_SELECT_STC;

/**
* @struct PRV_TGF_PCL_OVERRIDE_UDB_STC
*
* @brief Defines content of some User Defined Bytes in the keys
*/
typedef struct{

    /** Standard Not Ip Key, VRF */
    GT_BOOL vrfIdLsbEnableStdNotIp;

    /** Standard Not Ip Key, VRF */
    GT_BOOL vrfIdMsbEnableStdNotIp;

    /** Standard Ip L2 Qos Key, VRF */
    GT_BOOL vrfIdLsbEnableStdIpL2Qos;

    /** Standard Ip L2 Qos Key, VRF */
    GT_BOOL vrfIdMsbEnableStdIpL2Qos;

    /** Standard Ipv4 L4 Key, VRF */
    GT_BOOL vrfIdLsbEnableStdIpv4L4;

    /** Standard Ipv4 L4 Key, VRF */
    GT_BOOL vrfIdMsbEnableStdIpv4L4;

    /** Standard UDB Key, VRF */
    GT_BOOL vrfIdLsbEnableStdUdb;

    /** Standard UDB Key, VRF */
    GT_BOOL vrfIdMsbEnableStdUdb;

    /** Extended Not Ipv6 Key, VRF */
    GT_BOOL vrfIdLsbEnableExtNotIpv6;

    /** Extended Not Ipv6 Key, VRF */
    GT_BOOL vrfIdMsbEnableExtNotIpv6;

    /** Extended Ipv6 L2 Key, VRF */
    GT_BOOL vrfIdLsbEnableExtIpv6L2;

    /** Extended Ipv6 L2 Key, VRF */
    GT_BOOL vrfIdMsbEnableExtIpv6L2;

    /** Extended Ipv6 L4 Key, VRF */
    GT_BOOL vrfIdLsbEnableExtIpv6L4;

    /** Extended Ipv6 L4 Key, VRF */
    GT_BOOL vrfIdMsbEnableExtIpv6L4;

    /** Extended UDB Key, VRF */
    GT_BOOL vrfIdLsbEnableExtUdb;

    /** Extended UDB Key, VRF */
    GT_BOOL vrfIdMsbEnableExtUdb;

    /** Standard UDB key, QoS profile in UDB2 */
    GT_BOOL qosProfileEnableStdUdb;

    /** Extended UDB key, QoS profile in UDB5 */
    GT_BOOL qosProfileEnableExtUdb;

} PRV_TGF_PCL_OVERRIDE_UDB_STC;


/**
* @struct PRV_TGF_PCL_OVERRIDE_UDB_TRUNK_HASH_STC
 *
 * @brief Defines content of some User Defined Bytes in the key
 * to be the packets trunk hash value.
 * Only for Lion and above devices.
*/
typedef struct{

    /** @brief Standard Not Ip Key,
     *  trunk hash in UDB17
     */
    GT_BOOL trunkHashEnableStdNotIp;

    /** @brief Standard Ipv4 L4 Key,
     *  trunk hash in UDB22
     */
    GT_BOOL trunkHashEnableStdIpv4L4;

    /** @brief Standard UDB Key,
     *  trunk hash in UDB3
     */
    GT_BOOL trunkHashEnableStdUdb;

    /** @brief Extended Not Ipv6 Key,
     *  trunk hash in UDB3
     */
    GT_BOOL trunkHashEnableExtNotIpv6;

    /** @brief Extended Ipv6 L2 Key,
     *  trunk hash in UDB7
     */
    GT_BOOL trunkHashEnableExtIpv6L2;

    /** @brief Extended Ipv6 L4 Key,
     *  trunk hash in UDB13
     */
    GT_BOOL trunkHashEnableExtIpv6L4;

    /** @brief Extended UDB Key,
     *  trunk hash in UDB3
     *  Comment:
     */
    GT_BOOL trunkHashEnableExtUdb;

} PRV_TGF_PCL_OVERRIDE_UDB_TRUNK_HASH_STC;

/**
* @enum PRV_TGF_PCL_EGRESS_KEY_VID_UP_MODE_ENT
 *
 * @brief enumerator for Egress PCL
 * VID and UP key fields content mode
*/
typedef enum{

    /** extract from packet Tag0 */
    PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E,

    /** extract from packet Tag1 */
    PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG1_E

} PRV_TGF_PCL_EGRESS_KEY_VID_UP_MODE_ENT;

/**
* @enum PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT
 *
 * @brief packet types that can be enabled/disabled for Egress PCL.
 * By default on all ports and on all packet types are disabled
 * Enumerators:
 * PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E - Enable/Disable Egress Policy
 * for Control packets FROM CPU.
 * PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E  - Enable/Disable Egress Policy
 * for data packets FROM CPU.
 * PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E      - Enable/Disable Egress Policy
 * on TO-CPU packets
 * PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E   - Enable/Disable Egress Policy
 * for TO ANALYZER packets.
 * PRV_TGF_PCL_EGRESS_PKT_TS_E        - Enable/Disable Egress Policy
 * for data packets that are tunneled in this device.
 * PRV_TGF_PCL_EGRESS_PKT_NON_TS_E      - Enable/Disable Egress Policy
 * for data packets that are not tunneled in this device.
 * Comment:
 * relevant only for DxCh2 and above devices
*/
typedef enum{

    PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,

    PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E,

    PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E,

    PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E,

    PRV_TGF_PCL_EGRESS_PKT_TS_E,

    PRV_TGF_PCL_EGRESS_PKT_NON_TS_E

} PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT;

/**
* @enum PRV_TGF_PCL_INGRESS_METADATA_FIELDS_ENT
 *
 * @brief IPCL Metadata fields
*/
typedef enum
{
    PRV_TGF_PCL_METADATA_FIELDS_PORT_LIST_SRC_E,
    PRV_TGF_PCL_METADATA_FIELDS_LOCAL_DEV_SRC_TRUNK_ID_E,
    PRV_TGF_PCL_METADATA_FIELDS_LOCAL_DEV_SRC_EPORT_E,
    PRV_TGF_PCL_METADATA_FIELDS_SRC_DEV_IS_OWN_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_1_E,
    PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_DEV_E,
    PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_IS_TRUNK_E,
    PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_EPORT_OR_TRUNK_ID_E,
    PRV_TGF_PCL_METADATA_FIELDS_LOCAL_DEV_SRC_PORT_E,
    PRV_TGF_PCL_METADATA_FIELDS_USE_VIDX_E,
    PRV_TGF_PCL_METADATA_FIELDS_EVIDX_E,
    PRV_TGF_PCL_METADATA_FIELDS_INGRESS_UDB_PACKET_TYPE_E,
    PRV_TGF_PCL_METADATA_FIELDS_IP_LEGAL_OR_FCOE_LEGAL_E,
    PRV_TGF_PCL_METADATA_FIELDS_L2_VALID_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_ARP_E,
    PRV_TGF_PCL_METADATA_FIELDS_TRG_DEV_E,
    PRV_TGF_PCL_METADATA_FIELDS_TRG_IS_TRUNK_E,
    PRV_TGF_PCL_METADATA_FIELDS_TRG_EPORT_OR_TRG_TRUNK_ID_E,
    PRV_TGF_PCL_METADATA_FIELDS_TRG_PHY_PORT_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_TRG_PHY_PORT_VALID_E,
    PRV_TGF_PCL_METADATA_FIELDS_EGRESS_UDB_PACKET_TYPE_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_2_E,
    PRV_TGF_PCL_METADATA_FIELDS_SRC_ID_E,
    PRV_TGF_PCL_METADATA_FIELDS_EGRESS_FILTER_REGISTERED_E,
    PRV_TGF_PCL_METADATA_FIELDS_PACKET_IS_LOOPED_E,
    PRV_TGF_PCL_METADATA_FIELDS_DROP_ON_SOURCE_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_3_E,
    PRV_TGF_PCL_METADATA_FIELDS_PHY_SRC_MC_FILTER_EN_E,
    PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_PHY_IS_TRUNK_E,
    PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_4_E,
    PRV_TGF_PCL_METADATA_FIELDS_L2_VALID_1_E,
    PRV_TGF_PCL_METADATA_FIELDS_L2_ENCAPSULATION_E,
    PRV_TGF_PCL_METADATA_FIELDS_BYPASS_BRIDGE_E,
    PRV_TGF_PCL_METADATA_FIELDS_MAC_DA_TYPE_E,
    PRV_TGF_PCL_METADATA_FIELDS_OUTER_IS_LLC_NON_SNAP_E,
    PRV_TGF_PCL_METADATA_FIELDS_INNER_IS_LLC_NON_SNAP_E,
    PRV_TGF_PCL_METADATA_FIELDS_ETHERTYPE_OR_DSAPSSAP_E,
    PRV_TGF_PCL_METADATA_FIELDS_TAG0_SRC_TAGGED_E,
    PRV_TGF_PCL_METADATA_FIELDS_TAG1_SRC_TAGGED_E,
    PRV_TGF_PCL_METADATA_FIELDS_SRC_TAG0_IS_OUTER_TAG_E,
    PRV_TGF_PCL_METADATA_FIELDS_OUTER_SRC_TAG_E,
    PRV_TGF_PCL_METADATA_FIELDS_TAG1_LOCAL_DEV_SRC_TAGGED_E,
    PRV_TGF_PCL_METADATA_FIELDS_NESTED_VLAN_EN_E,
    PRV_TGF_PCL_METADATA_FIELDS_TAG0_PRIO_TAGGED_E,
    PRV_TGF_PCL_METADATA_FIELDS_TAG1_PRIO_TAGGED_E,
    PRV_TGF_PCL_METADATA_FIELDS_ORIG_VID_E,
    PRV_TGF_PCL_METADATA_FIELDS_OVERRIDE_EVLAN_WITH_ORIGVID_E,
    PRV_TGF_PCL_METADATA_FIELDS_TRG_TAGGED_E,
    PRV_TGF_PCL_METADATA_FIELDS_EVLAN_PRECEDENCE_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_5_E,
    PRV_TGF_PCL_METADATA_FIELDS_TAG0_TPID_INDEX_E,
    PRV_TGF_PCL_METADATA_FIELDS_TAG1_TPID_INDEX_E,
    PRV_TGF_PCL_METADATA_FIELDS_NUM_OF_TAG_WORDS_TO_POP_E,
    PRV_TGF_PCL_METADATA_FIELDS_IPX_PROTOCOL_E,
    PRV_TGF_PCL_METADATA_FIELDS_IPV4_SIP_OR_ARP_SIP_OR_FCOE_S_ID_E,
    PRV_TGF_PCL_METADATA_FIELDS_IPV4_DIP_OR_ARP_DIP_OR_FCOE_D_ID_E,
    PRV_TGF_PCL_METADATA_FIELDS_POLICY_RTT_INDEX_E,
    PRV_TGF_PCL_METADATA_FIELDS_L3_OFFSET_INVALID_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IP_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IPV4_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_FCOE_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_ARP_1_E,
    PRV_TGF_PCL_METADATA_FIELDS_IP_LEGAL_OR_FCOE_LEGAL_1_E,
    PRV_TGF_PCL_METADATA_FIELDS_IPM_E,
    PRV_TGF_PCL_METADATA_FIELDS_IP_HEADER_INFO_E,
    PRV_TGF_PCL_METADATA_FIELDS_IP_FRAGMENTED_E,
    PRV_TGF_PCL_METADATA_FIELDS_FRAGMENTED_E,
    PRV_TGF_PCL_METADATA_FIELDS_ROUTED_E,
    PRV_TGF_PCL_METADATA_FIELDS_DO_ROUTE_HA_E,
    PRV_TGF_PCL_METADATA_FIELDS_MAC_SA_ARP_SA_MISMATCH_E,
    PRV_TGF_PCL_METADATA_FIELDS_IPX_HEADER_LENGTH_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_6_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IP_1_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_1_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_ND_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_LINK_LOCAL_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_MLD_E,
    PRV_TGF_PCL_METADATA_FIELDS_IPV6_HBH_EXT_E,
    PRV_TGF_PCL_METADATA_FIELDS_IPV6_EH_E,
    PRV_TGF_PCL_METADATA_FIELDS_SOLICITATION_MULTICAST_MESSAGE_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IP_2_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_2_E,
    PRV_TGF_PCL_METADATA_FIELDS_IPV6_FLOW_LABEL_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_7_E,
    PRV_TGF_PCL_METADATA_FIELDS_TCP_UDP_DEST_PORT_E,
    PRV_TGF_PCL_METADATA_FIELDS_TCP_OR_UDP_PORT_COMPARATORS_E,
    PRV_TGF_PCL_METADATA_FIELDS_L4_OFFSET_INVALID_E,
    PRV_TGF_PCL_METADATA_FIELDS_L4_VALID_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_SYN_E,
    PRV_TGF_PCL_METADATA_FIELDS_SYN_WITH_DATA_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_8_E,
    PRV_TGF_PCL_METADATA_FIELDS_L2_ECHO_E,
    PRV_TGF_PCL_METADATA_FIELDS_CFM_PACKET_E,
    PRV_TGF_PCL_METADATA_FIELDS_TIMESTAMP_EN_E,
    PRV_TGF_PCL_METADATA_FIELDS_TIMESTAMP_TAGGED_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_9_E,
    PRV_TGF_PCL_METADATA_FIELDS_TIMESTAMP_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_PTP_E,
    PRV_TGF_PCL_METADATA_FIELDS_PTP_DOMAIN_E,
    PRV_TGF_PCL_METADATA_FIELDS_PTP_U_FIELD_E,
    PRV_TGF_PCL_METADATA_FIELDS_PTP_TAI_SELECT_E,
    PRV_TGF_PCL_METADATA_FIELDS_PTP_TRIGGER_TYPE_E,
    PRV_TGF_PCL_METADATA_FIELDS_OAM_PROCESSING_EN_E,
    PRV_TGF_PCL_METADATA_FIELDS_OAM_PTP_OFFSET_INDEX_OR_PTP_OFFSET_E,
    PRV_TGF_PCL_METADATA_FIELDS_RX_SNIFF_OR_SRC_TRG_E,
    PRV_TGF_PCL_METADATA_FIELDS_SRC_TRG_EPORT_E,
    PRV_TGF_PCL_METADATA_FIELDS_SRC_TRG_DEV_E,
    PRV_TGF_PCL_METADATA_FIELDS_ANALYZER_INDEX_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_10_E,
    PRV_TGF_PCL_METADATA_FIELDS_SRC_TRG_PHY_PORT_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_MPLS_E,
    PRV_TGF_PCL_METADATA_FIELDS_MPLS_CMD_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_11_E,
    PRV_TGF_PCL_METADATA_FIELDS_ECN_CAPABLE_E,
    PRV_TGF_PCL_METADATA_FIELDS_QCN_RX_E,
    PRV_TGF_PCL_METADATA_FIELDS_ORIG_RX_QCN_PRIO_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_12_E,
    PRV_TGF_PCL_METADATA_FIELDS_TUNNEL_TERMINATED_E,
    PRV_TGF_PCL_METADATA_FIELDS_INNER_PACKET_TYPE_E,
    PRV_TGF_PCL_METADATA_FIELDS_TUNNEL_START_E,
    PRV_TGF_PCL_METADATA_FIELDS_TUNNEL_START_PASSENGER_TYPE_E,
    PRV_TGF_PCL_METADATA_FIELDS_ARP_PTR_OR_TUNNEL_PTR_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_13_E,
    PRV_TGF_PCL_METADATA_FIELDS_QOS_PROFILE_PRECEDENCE_E,
    PRV_TGF_PCL_METADATA_FIELDS_QOS_PROFILE_E,
    PRV_TGF_PCL_METADATA_FIELDS_MODIFY_UP_E,
    PRV_TGF_PCL_METADATA_FIELDS_MODIFY_DSCP_EXP_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_14_E,
    PRV_TGF_PCL_METADATA_FIELDS_BYTE_COUNT_E,
    PRV_TGF_PCL_METADATA_FIELDS_RECALC_CRC_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_15_E,
    PRV_TGF_PCL_METADATA_FIELDS_BYPASS_INGRESS_PIPE_E,
    PRV_TGF_PCL_METADATA_FIELDS_MARVELL_TAGGED_E,
    PRV_TGF_PCL_METADATA_FIELDS_MARVELL_TAGGED_EXTENDED_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_16_E,
    PRV_TGF_PCL_METADATA_FIELDS_RECEIVED_DSA_TAG_WORD_0_E,
    PRV_TGF_PCL_METADATA_FIELDS_RECEIVED_DSA_TAG_WORD_1_E,
    PRV_TGF_PCL_METADATA_FIELDS_RECEIVED_DSA_TAG_WORD_2_E,
    PRV_TGF_PCL_METADATA_FIELDS_RECEIVED_DSA_TAG_WORD_3_E,
    PRV_TGF_PCL_METADATA_FIELDS_IS_TRILL_E,
    PRV_TGF_PCL_METADATA_FIELDS_REP_E,
    PRV_TGF_PCL_METADATA_FIELDS_REP_LAST_E,
    PRV_TGF_PCL_METADATA_FIELDS_PACKET_TYPE_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_17_E,
    PRV_TGF_PCL_METADATA_FIELDS_APPLICABLE_FLOW_SUB_TEMPLATE_E,
    PRV_TGF_PCL_METADATA_FIELDS_METERING_EN_E,
    PRV_TGF_PCL_METADATA_FIELDS_BILLING_EN_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_18_E,
    PRV_TGF_PCL_METADATA_FIELDS_POLICER_PTR_E,
    PRV_TGF_PCL_METADATA_FIELDS_INGRESS_CORE_ID_E,
    PRV_TGF_PCL_METADATA_FIELDS_RX_IS_PROTECTION_PATH_E,
    PRV_TGF_PCL_METADATA_FIELDS_RX_PROTECTION_SWITCH_EN_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_19_E,
    PRV_TGF_PCL_METADATA_FIELDS_COPY_RESERVED_E,
    PRV_TGF_PCL_METADATA_FIELDS_RESERVED_20_E

} PRV_TGF_PCL_INGRESS_METADATA_FIELDS_ENT;

/**
* @struct PRV_TGF_PCL_METADATA_OFFSETS_STC
 *
 * @brief This structure defines metadata anchor offset and bit offsets
*/
typedef struct{

    /** metadata anchor offset */
    GT_U8 anchorOffset;

    /** offset in bits */
    GT_U8 bitOffset;

    /** @brief length of field in bits
     *  Comments:
     */
    GT_U8 fieldLength;

} PRV_TGF_PCL_METADATA_OFFSETS_STC;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChRuleAction function
* @endinternal
*
* @brief   Convert generic into device specific Policy Engine Action
*
* @param[in] ruleActionPtr            - (pointer to) Policy Engine Action
*
* @param[out] dxChRuleActionPtr        - (pointer to) DxCh Policy Engine Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChRuleAction
(
    IN  PRV_TGF_PCL_ACTION_STC      *ruleActionPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC    *dxChRuleActionPtr
);
/**
* @internal prvTgfConvertDxChToGenericRuleAction function
* @endinternal
*
* @brief   Convert device specific Policy Engine into generic
*          Action
*
* @param[in] dxChRuleActionPtr - (pointer to) DxCh Policy Engine
*                                 Action
*
* @param[out] ruleActionPtr  - (pointer to) Policy Engine Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertDxChToGenericRuleAction
(
    IN  CPSS_DXCH_PCL_ACTION_STC    *dxChRuleActionPtr,
    OUT PRV_TGF_PCL_ACTION_STC      *ruleActionPtr
);
#endif /* CHX_FAMILY */

/**
* @internal prvTgfPclInit function
* @endinternal
*
* @brief   The function initializes the Policy engine
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note NONE
*
*/
GT_STATUS prvTgfPclInit
(
    GT_VOID
);

/**
* @internal prvTgfPclIngressPolicyEnable function
* @endinternal
*
* @brief   Enables Ingress Policy
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclIngressPolicyEnable
(
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfPclEgressPolicyEnable function
* @endinternal
*
* @brief   Enables Egress Policy
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressPolicyEnable
(
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfPclPortIngressPolicyEnable function
* @endinternal
*
* @brief   Enables/disables ingress policy per port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclPortIngressPolicyEnable
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfPclEPortIngressPolicyEnable function
* @endinternal
*
* @brief   Enables/disables ingress policy per port.
*
* @param[in] devNum                   - device number
* @param[in] ePortNum                 - ePort number
* @param[in] enable                   - enable/disable Ingress Policy
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEPortIngressPolicyEnable
(
    IN GT_U8                          devNum,
    IN GT_U32                         ePortNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfPclEgressPclPacketTypesSet function
* @endinternal
*
* @brief   Enables/disables egress policy per packet type and port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pktType                  - packet type
* @param[in] enable                   - enable/disable Ingress Policy
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressPclPacketTypesSet
(
    IN GT_U8                             devNum,
    IN GT_U32                            portNum,
    IN PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT   pktType,
    IN GT_BOOL                           enable
);

/**
* @internal prvTgfPclEgressPclPacketTypesGet function
* @endinternal
*
* @brief   Get status of Egress PCL (EPCL) for set of packet types on port
*
* @param[in] devNum                   - device number
*                                      port          - port number
* @param[in] pktType                  - packet type to enable/disable EPCL for it
*
* @param[out] enablePtr                - enable EPCL for specific packet type
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPclEgressPclPacketTypesGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            portNum,
    IN  PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT   pktType,
    OUT GT_BOOL                           *enablePtr
);

/**
* @internal prvTgfPclPortLookupCfgTabAccessModeSet function
* @endinternal
*
* @brief   Configures VLAN/PORT access mode to Ingress or Egress PCL
*         configuration table perlookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclPortLookupCfgTabAccessModeSet
(
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_PCL_DIRECTION_ENT                       direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                   lookupNum,
    IN GT_U32                                       sublookup,
    IN PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT mode
);

/**
* @internal prvTgfPclPortLookupCfgTabAccessModeGet function
* @endinternal
*
* @brief   Gets VLAN/PORT/ePort access mode to Ingress or Egress PCL
*         configuration table per lookup.
*
* @param[out] modePtr                  - (pointer to)PCL Configuration Table access mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclPortLookupCfgTabAccessModeGet
(
    IN GT_PORT_NUM                                   portNum,
    IN CPSS_PCL_DIRECTION_ENT                        direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                    lookupNum,
    IN GT_U32                                        sublookup,
    OUT PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT *modePtr
);

/**
* @internal prvTgfPclCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*
* @param[in] interfaceInfoPtr         - (pointer to) interface data
* @param[in] direction                - Policy direction
* @param[in] lookupNum                - Lookup number
* @param[in] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclCfgTblSet
(
    IN CPSS_INTERFACE_INFO_STC       *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT         direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT     lookupNum,
    IN PRV_TGF_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
);

/**
* @internal prvTgfPclCfgTblGet function
* @endinternal
*
* @brief   Get PCL Configuration table entry's lookup configuration for interface.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - (pointer to) interface data
* @param[in] direction                - Policy direction
* @param[in] lookupNum                - Lookup number
*
* @param[out] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclCfgTblGet
(
    IN  GT_U8                          devNum,
    IN  CPSS_INTERFACE_INFO_STC        *interfaceInfoPtr,
    IN  CPSS_PCL_DIRECTION_ENT         direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT     lookupNum,
    OUT PRV_TGF_PCL_LOOKUP_CFG_STC     *lookupCfgPtr
);

/**
* @internal prvTgfPclRuleWithOptionsSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclRuleWithOptionsSet
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN GT_U32                             ruleIndex,
    IN PRV_TGF_PCL_RULE_OPTION_ENT        ruleOptionsBmp,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT       *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT       *patternPtr,
    IN PRV_TGF_PCL_ACTION_STC            *actionPtr
);

/**
* @internal prvTgfConvertGenericToDxChIngStdNotIp function
* @endinternal
*
* @brief   Convert generic into device specific standard not IP packet key
*
* @param[in] ruleStdNotIpPtr          - (pointer to) standard not IP packet key
*
* @param[out] dxChRuleStdNotIpPtr      - (pointer to) DxCh standard not IP packet key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChIngStdNotIp
(
    IN  PRV_TGF_PCL_RULE_FORMAT_STD_NOT_IP_STC      *ruleStdNotIpPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC    *dxChRuleStdNotIpPtr
);

/**
* @internal prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos function
* @endinternal
*
* @brief   Convert generic into device specific ingress Ultra Port/VLAN Qos IPV6 key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChIngRuleUltraIpv6PortVlanQos
(
    IN  PRV_TGF_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC   *ruleUltraIpv6PortVlanQosPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC *dxChRuleUltraIpv6PortVlanQosPtr
);

/**
* @internal prvTgfPclRuleSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclRuleSet
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN GT_U32                             ruleIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT       *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT       *patternPtr,
    IN PRV_TGF_PCL_ACTION_STC            *actionPtr
);

/**
* @internal prvTgfPclRuleActionUpdate function
* @endinternal
*
* @brief   The function updates the Rule Action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - on the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*
* @note NONE
*
*/
GT_STATUS prvTgfPclRuleActionUpdate
(
    IN CPSS_PCL_RULE_SIZE_ENT         ruleSize,
    IN GT_U32                         ruleIndex,
    IN PRV_TGF_PCL_ACTION_STC        *actionPtr
);

/**
* @internal prvTgfPclRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the Policy rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on TCAM found rule of different size
* @retval GT_TIMEOUT               - on max number of retries checking if PP ready
*/
GT_STATUS prvTgfPclRuleValidStatusSet
(
    IN CPSS_PCL_RULE_SIZE_ENT         ruleSize,
    IN GT_U32                         ruleIndex,
    IN GT_BOOL                        valid
);

/**
* @internal prvTgfPclRuleCopy function
* @endinternal
*
* @brief   The function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move Policy
*         Rule from old position to new one at first cpssDxChPclRuleCopy should be
*         called. And after this cpssDxChPclRuleInvalidate should be used to invalidate
*         Rule in old position
* @param[in] ruleSize                 - size of Rule.
* @param[in] ruleSrcIndex             - index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
* @param[in] ruleDstIndex             - index of the rule in the TCAM to which pattern,
*                                      mask and action are placed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclRuleCopy
(
    IN CPSS_PCL_RULE_SIZE_ENT             ruleSize,
    IN GT_U32                             ruleSrcIndex,
    IN GT_U32                             ruleDstIndex
);

/**
* @internal prvTgfPclUdbIndexConvert function
* @endinternal
*
* @brief   The function converts the UDB relative index (in rule format)
*         to UDB absolute index in UDB configuration table entry
*         Conversion needed for XCAT and Puma devices, not needed for CH1-3
* @param[in] ruleFormat               - rule format
*                                      packetType - packet Type
*                                      udbIndex   - relative index of UDB in the rule
*
* @param[out] udbAbsIndexPtr           - (pointer to) UDB absolute index in
*                                      UDB configuration table entry
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on parameter value more then HW bit field
*/
GT_STATUS prvTgfPclUdbIndexConvert
(
    IN  PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN  GT_U32                               udbRelIndex,
    OUT GT_U32                               *udbAbsIndexPtr
);

/**
* @internal prvTgfPclUdbIndexExtConvert function
* @endinternal
*
* @brief   The function converts the UDB relative index (in rule format)
*         to UDB absolute index in UDB configuration table entry
*         Conversion needed for XCAT and Puma devices, not needed for CH1-3
* @param[in] ruleFormat               - rule format
*                                      packetType - packet Type
*                                      udbIndex   - relative index of UDB in the rule
*
* @param[out] udbAbsIndexPtr           - (pointer to) UDB absolute index in
*                                      UDB configuration table entry
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on parameter value more then HW bit field
*/
GT_STATUS prvTgfPclUdbIndexExtConvert
(
    IN  PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN  GT_U32                               udbRelIndex,
    OUT GT_U32                               *udbAbsIndexPtr
);

/**
* @internal prvTgfPclUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @param[in] ruleFormat               - rule format
*                                      Relevant for DxCh1, DxCh2, DxCh3 devices
* @param[in] packetType               - packet Type
*                                      Relevant for DxChXCat and above devices
* @param[in] direction                -  (APPLICABLE DEVICES Lion2)
* @param[in] udbIndex                 - index of User Defined Byte to configure
* @param[in] offset                   - the type of offset
* @param[in] offset                   - The  of the user-defined byte
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on parameter value more then HW bit field
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclUserDefinedByteSet
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT          packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT          offsetType,
    IN GT_U8                                offset
);

/**
* @internal prvTgfPclUserDefinedBytesSelectSet function
* @endinternal
*
* @brief   Set the User Defined Byte (UDB) Selection Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] ruleFormat               - rule format
*                                      Valid only UDB only key formats.
*                                      The parameter used also to determinate Ingress or Egress.
*                                      Relevant values are:
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E
*                                      PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E
*                                      PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E
* @param[in] packetType               - packet Type
* @param[in] lookupNum                - Lookup number. Ignored for egress.
* @param[in] udbSelectPtr             - (pointer to) structure with UDB Selection configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclUserDefinedBytesSelectSet
(
    IN  PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN  PRV_TGF_PCL_PACKET_TYPE_ENT          packetType,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum,
    IN  PRV_TGF_PCL_UDB_SELECT_STC           *udbSelectPtr
);

/**
* @internal prvTgfPclOverrideUserDefinedBytesSet function
* @endinternal
*
* @brief   The function sets overriding of User Defined Bytes by predefined key fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclOverrideUserDefinedBytesSet
(
    IN  PRV_TGF_PCL_OVERRIDE_UDB_STC *udbOverridePtr
);

/**
* @internal prvTgfPclInvalidUdbCmdSet function
* @endinternal
*
* @brief   Set the command that is applied when the policy key <User-Defined>
*         field cannot be extracted from the packet due to lack of header depth
* @param[in] udbErrorCmd              - command applied to a packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on one of the input parameters is not valid
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclInvalidUdbCmdSet
(
    IN PRV_TGF_UDB_ERROR_CMD_ENT    udbErrorCmd
);


/******************************************************************************\
 *                       API for default settings                             *
\******************************************************************************/

/**
* @internal prvTgfPclDefPortInitExt2 function
* @endinternal
*
* @brief   Initialize PCL Engine
*
* @param[in] portNum                  - port number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfPclDefPortInitExt2
(
    IN GT_U32                            portNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_LOOKUP_CFG_STC       *lookupCfgPtr
);

/**
* @internal prvTgfPclDefPortInitExt1 function
* @endinternal
*
* @brief   Initialize PCL Engine
*
* @param[in] portNum                  - port number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] pclId                    - pcl Id
* @param[in] nonIpKey                 - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
* @param[in] ipv4Key                  - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
* @param[in] ipv6Key                  - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfPclDefPortInitExt1
(
    IN GT_U32                            portNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pclId,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT nonIpKey,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv4Key,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv6Key
);

/**
* @internal prvTgfPclDefPortInit function
* @endinternal
*
* @brief   Initialize PCL Engine
*
* @param[in] portNum                  - port number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] nonIpKey                 - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
* @param[in] ipv4Key                  - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
* @param[in] ipv6Key                  - PCL_RULE_FORMAT_TYPE for PCL_LOOKUP_CFG_TABLE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfPclDefPortInit
(
    IN GT_U32                            portNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT nonIpKey,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv4Key,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ipv6Key
);

/**
* @internal prvTgfPclUdeEtherTypeSet function
* @endinternal
*
* @brief   This function sets UDE Ethertype.
*
* @note   APPLICABLE DEVICES:      Lion and above.
* @param[in] index                    - UDE Ethertype index,
*                                      (APPLICABLE RANGES: Lion, xCat2, Lion2 0..4; Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..6)
* @param[in] ethType                  - Ethertype value (range 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclUdeEtherTypeSet
(
    IN  GT_U32          index,
    IN  GT_U32          ethType
);

/**
* @internal prvTgfPclOverrideUserDefinedBytesByTrunkHashSet function
* @endinternal
*
* @brief   The function sets overriding of User Defined Bytes
*         by packets Trunk Hash value.
* @param[in] udbOverTrunkHashPtr      - (pointer to) UDB override trunk hash structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclOverrideUserDefinedBytesByTrunkHashSet
(
    IN  PRV_TGF_PCL_OVERRIDE_UDB_TRUNK_HASH_STC *udbOverTrunkHashPtr
);

/**
* @internal prvTgfPclEgressKeyFieldsVidUpModeSet function
* @endinternal
*
* @brief   Sets Egress Policy VID and UP key fields content mode
*
* @param[in] vidUpMode                - VID and UP key fields content mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressKeyFieldsVidUpModeSet
(
    IN  PRV_TGF_PCL_EGRESS_KEY_VID_UP_MODE_ENT   vidUpMode
);

/**
* @internal prvTgfPclLookupCfgPortListEnableSet function
* @endinternal
*
* @brief   The function enables/disables using port-list in search keys.
*
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
* @param[in] enable                   - GT_TRUE  -  port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclLookupCfgPortListEnableSet
(
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    IN  GT_BOOL                       enable
);

/**
* @internal prvTgfPclLookupCfgPortListEnableGet function
* @endinternal
*
* @brief   The function gets enable/disable state of
*         using port-list in search keys.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] lookupNum                - lookup number: 0,1
* @param[in] subLookupNum             - Sub lookup Number - for ingress CPSS_PCL_LOOKUP_0_E
*                                      means 0: lookup0_0, 1: lookup0_1,
*                                      for other cases not relevant.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable port-list in search key
*                                      GT_FALSE - disable port-list in search key
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclLookupCfgPortListEnableGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    OUT GT_BOOL                       *enablePtr
);

/**
* @internal tgfPclOverrideUserDefinedBytesEnableSet function
* @endinternal
*
* @brief   The function enables or disables overriding of the
*         UDB content for specific UDB in specific key format
*         where it supported by HW.
* @param[in] ruleFormat               - rule format
* @param[in] udbOverrideType          - The type of the UDB configuration overriding.
* @param[in] enable                   - GT_TRUE -  overriding of UDB content
*                                      by udbOverrideType.
*                                      GT_FALSE - disable overriding of UDB content
*                                      by udbOverrideType, UDB configuration is used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS tgfPclOverrideUserDefinedBytesEnableSet
(
    IN  PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN  PRV_TGF_PCL_UDB_OVERRIDE_TYPE_ENT    udbOverrideType,
    IN  GT_BOOL                              enable
);

/**
* @internal prvTgfPclRestore function
* @endinternal
*
* @brief   Function clears ingress pcl settings.
*/
GT_VOID prvTgfPclRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclPortsRestore function
* @endinternal
*
* @brief   Function clears port related pcl settings.
*/
GT_VOID prvTgfPclPortsRestore
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum
);

/**
* @internal prvTgfPclPortsRestoreAll function
* @endinternal
*
* @brief   Function clears port related pcl settings - FOR ALL PORTS , EPCL,IPCL0,1,2
*/
GT_VOID prvTgfPclPortsRestoreAll
(
    GT_VOID
);


/**
* @internal prvTgfPclPolicerCheck function
* @endinternal
*
* @brief   check if the device supports the policer pointed from the PCL/TTI
*
* @retval GT_TRUE                  - the device supports the policer pointed from the PCL/TTI
* @retval GT_FALSE                 - the device not supports the policer pointed from the PCL/TTI
*/
GT_BOOL prvTgfPclPolicerCheck(
    void
);

/**
* @internal prvTgfPclTcpUdpPortComparatorSet function
* @endinternal
*
* @brief   Configure TCP or UDP Port Comparator entry
*
* @param[in] direction                - Policy Engine direction, Ingress or Egress
* @param[in] l4Protocol               - protocol, TCP or UDP
* @param[in] entryIndex               - entry index (0-7)
* @param[in] l4PortType               - TCP/UDP port type, source or destination port
* @param[in] compareOperator          - compare operator FALSE, LTE, GTE, NEQ
* @param[in] value                    - 16 bit  to compare with
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclTcpUdpPortComparatorSet
(
    IN CPSS_PCL_DIRECTION_ENT            direction,
    IN CPSS_L4_PROTOCOL_ENT              l4Protocol,
    IN GT_U32                            entryIndex,
    IN CPSS_L4_PROTOCOL_PORT_TYPE_ENT    l4PortType,
    IN CPSS_COMPARE_OPERATOR_ENT         compareOperator,
    IN GT_U16                            value
);
/**
* @internal prvTgfPclL3L4ParsingOverMplsEnableSet function
* @endinternal
*
* @brief   If enabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as IP packets for key selection and UDB usage.
*         If disabled, non-Tunnel-Terminated IPvx over MPLS packets are treated
*         by Ingress PCL as MPLS packets for key selection and UDB usage.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclL3L4ParsingOverMplsEnableSet
(
    IN  GT_BOOL                            enable
);

/**
* @internal prvTgfPclOamRdiSet function
* @endinternal
*
* @brief   This function sets OAM RDI configuration.
*
* @note   APPLICABLE DEVICES:      Lion 3 and above.
* @param[in] direction                - policy direction
* @param[in] profile                  - OAM RDI  index, range 0..1
* @param[in] rdiUdbValue              - RDI UDB value (range 0..0xFF)
* @param[in] rdiUdbMask               - RDI UDB mask  (range 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclOamRdiSet
(
    IN CPSS_PCL_DIRECTION_ENT       direction,
    IN  GT_U32                      profile,
    IN  GT_U32                      rdiUdbValue,
    IN  GT_U32                      rdiUdbMask
);

/**
* @internal prvTgfPclLookup0ForRoutedPacketsEnableSet function
* @endinternal
*
* @brief   Enables/disables PCL lookup0 for routed packets
*
* @note   APPLICABLE DEVICES:      Lion and above.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclLookup0ForRoutedPacketsEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  GT_BOOL                            enable
);

/**
* @internal prvTgfPclOamChannelTypeProfileToOpcodeMappingSet function
* @endinternal
*
* @brief   Set Mapping of MPLS Channel Type Profile to OAM opcode.
*
* @note   APPLICABLE DEVICES:      Bobcat2 and above.
* @param[in] channelTypeProfile       - MPLS G-ACh Channel Type Profile
*                                      (APPLICABLE RANGES: 1..15)
* @param[in] opcode                   - OAM  (APPLICABLE RANGES: 0..255)
* @param[in] RBitAssignmentEnable     - replace Bit0 of opcode by packet R-flag
*                                      GT_TRUE  - Bit0 of result opcode is packet R-flag
*                                      GT_FALSE - Bit0 of result opcode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - for out-of-range parameter values.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclOamChannelTypeProfileToOpcodeMappingSet
(
    IN  GT_U32                    channelTypeProfile,
    IN  GT_U32                    opcode,
    IN  GT_BOOL                   RBitAssignmentEnable
);

/**
* @internal prvTgfPclTunnelTermForceVlanModeEnableSet function
* @endinternal
*
* @brief   The function enables/disables forcing of the PCL ID configuration
*         for all TT packets according to the VLAN assignment.
*
* @note   APPLICABLE DEVICES:      DxCh3 and Above.
* @param[in] enable                   - force TT packets assigned to PCL
*                                      configuration table entry
*                                      GT_TRUE  - By VLAN
*                                      GT_FALSE - according to ingress port
*                                      per lookup settings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclTunnelTermForceVlanModeEnableSet
(
    IN  GT_BOOL                       enable
);

/**
* @internal prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet function
* @endinternal
*
* @brief   Sets Egress Policy Configuration Table Access Mode for
*         Tunnel Start packets
*
* @note   APPLICABLE DEVICES:      DxCh3 and Above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet
(
    IN  PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT     cfgTabAccMode
);

/**
* @internal prvTgfPclSourceIdMaskSet function
* @endinternal
*
* @brief   Set mask so only certain bits in the source ID will be modified due to PCL
*         action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lookupNum                - lookup number
* @param[in] mask                     - Source ID  (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclSourceIdMaskSet
(
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum,
    IN  GT_U32                      mask
);

/**
* @internal prvTgfPclSourceIdMaskGet function
* @endinternal
*
* @brief   Get mask used for source ID modify due to PCL action.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] lookupNum                - lookup number
*
* @param[out] maskPtr                  - (pointer to) Source ID mask (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclSourceIdMaskGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum,
    OUT GT_U32                      *maskPtr
);

/**
* @internal prvTgfPclEgressSourcePortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portSelectionMode        - Source port selection mode for DSA packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
GT_STATUS prvTgfPclEgressSourcePortSelectionModeSet
(
    IN  PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT   portSelectionMode
);

/**
* @internal prvTgfPclEgressSourcePortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL source port field selection mode for DSA tagged packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portSelectionModePtr     - (pointer to) Source port selection mode for
*                                      DSA packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <SrcPort> always reflects the local device source physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <SrcPort> is taken from the DSA tag
*       if packet was received DSA tagged.
*
*/
GT_STATUS prvTgfPclEgressSourcePortSelectionModeGet
(
    IN  GT_U8                                               devNum,
    OUT PRV_TGF_PCL_EGRESS_SOURCE_PORT_SELECTION_MODE_ENT   *portSelectionModePtr
);

/**
* @internal prvTgfPclEgressTargetPortSelectionModeSet function
* @endinternal
*
* @brief   Set EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portSelectionMode        - Target port selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <TrgPort> always reflects the local device target physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <TrgPort> is taken from the DSA tag
*       if packet is transmitted DSA tagged.
*
*/
GT_STATUS prvTgfPclEgressTargetPortSelectionModeSet
(
    IN  PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT portSelectionMode
);

/**
* @internal prvTgfPclEgressTargetPortSelectionModeGet function
* @endinternal
*
* @brief   Get EPCL target port field selection mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portSelectionModePtr     - (pointer to) Target port selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note EPCL key <TrgPort> always reflects the local device target physical
*       ingress port, even for DSA-tagged packets. To maintain backward
*       compatibility for DSA-tagged packets, a global mode allows working in
*       the backward compatible mode where <TrgPort> is taken from the DSA tag
*       if packet is transmitted DSA tagged.
*
*/
GT_STATUS prvTgfPclEgressTargetPortSelectionModeGet
(
    IN  GT_U8                                               devNum,
    OUT PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT   *portSelectionModePtr
);

/**
* @internal prvTgfPclDefEportInitExt function
* @endinternal
*
* @brief   Initialize PCL Engine on ePort-based binding mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ePortNum                 - source ePort number
* @param[in] portNum                  - physical port number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*
* @note Enable access to IPCL/EPCL configuration table on ePort-based binding mode
*
*/
GT_STATUS prvTgfPclDefEportInitExt
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      ePortNum,
    IN GT_U32                            portNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_LOOKUP_CFG_STC       *lookupCfgPtr
);

/**
* @internal prvTgfPclEportAccessModeInit function
* @endinternal
*
* @brief   Initialize ePort access on ePort
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - port number
* @param[in] ePortNum                 - ePort number
* @param[in] direction                - PCL_DIRECTION
* @param[in] lookupNum                - PCL_LOOKUP_NUMBER
* @param[in] lookupCfgPtr             - (pointer to) lookup configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*
* @note Enable PCL on source ePort
*       Enable access to IPCL/EPCL configuration table on ePort-based binding mode
*
*/
GT_STATUS prvTgfPclEportAccessModeInit
(
    IN GT_PORT_NUM                      portNum,
    IN GT_PORT_NUM                      ePortNum,
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_LOOKUP_CFG_STC       *lookupCfgPtr
);

/**
* @internal prvTgfPclPortListGroupingEnableSet function
* @endinternal
*
* @brief   Enable/disable port grouping mode per direction. When enabled, the 4 MSB
*         bits of the <Port List> field in the PCL keys are replaced by a 4-bit
*         <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - Policy direction
* @param[in] enable                   - enable/disable port grouping mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclPortListGroupingEnableSet
(
    IN CPSS_PCL_DIRECTION_ENT   direction,
    IN GT_BOOL                  enable
);

/**
* @internal prvTgfPclPortListGroupingEnableGet function
* @endinternal
*
* @brief   Get the port grouping mode enabling status per direction. When enabled,
*         the 4 MSB bits of the <Port List> field in the PCL keys are replaced by a
*         4-bit <Port Group>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
*
* @param[out] enablePtr                - (pointer to) the port grouping mode enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclPortListGroupingEnableGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal prvTgfPclPortListPortMappingSet function
* @endinternal
*
* @brief   Set port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - Policy direction
* @param[in] portNum                  - port number
* @param[in] enable                   - port participate in the port list vector.
* @param[in] group                    - PCL port group
*                                      Relevant only if <enable> == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] offset                   -  in the port list bit vector.
*                                      Relevant only if <enable> == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..27)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When working in PCL Port List mode with port grouping enabled, <offset>
*       value in the range 23..27 has no influence.
*
*/
GT_STATUS prvTgfPclPortListPortMappingSet
(
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  group,
    IN  GT_U32                  offset
);

/**
* @internal prvTgfPclPortListPortMappingGet function
* @endinternal
*
* @brief   Get port group mapping and offset in port list for PCL working in Physical
*         Port List.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Policy direction
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) port participate in the port list vector.
* @param[out] groupPtr                 - (pointer to) PCL port group
* @param[out] offsetPtr                - (pointer to) offset in the port list bit vector.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPclPortListPortMappingGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_U32                  *groupPtr,
    OUT GT_U32                  *offsetPtr
);


/**
* @internal prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet function
* @endinternal
*
* @brief   Function enables/disables for ingress analyzed packet the using of VID from
*         the incoming packet to access the EPCL Configuration table and for lookup
*         keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] enable                   - GT_TRUE  - use original VID
*                                      GT_FALSE - use VLAN assigned by the processing pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressRxAnalyzerUseOrigVidEnableSet
(
    IN  GT_BOOL      enable
);

/**
* @internal prvTgfPclEgressRxAnalyzerUseOrigVidEnableGet function
* @endinternal
*
* @brief   Get state of the flag in charge of the using of VID of the incoming packet
*         for ingress analyzed packet to access the EPCL Configuration table and for
*         lookup keys generation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable/disable the using of original VID.
*                                      GT_TRUE  - use original VID
*                                      GT_FALSE - use VLAN assigned by the processing pipe
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfPclEgressRxAnalyzerUseOrigVidEnableGet
(
    IN  GT_U8         devNum,
    OUT GT_BOOL       *enablePtr
);

/**
* @internal prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet function
* @endinternal
*
* @brief   Get absolute IPCL rules index bases in TCAM for IPCL0/1/2
*         the 'relative index' is modulo (%) by the function according to
*         appDemoDxChTcamIpclNumOfIndexsGet(lookupId)
*         relevant for earch devices, otherwise returns relativeTcamEntryIndex
* @param[in] lookupId                 -  IPCL - 0/1/2
* @param[in] relativeTcamEntryIndex
*                                       None
*/
GT_U32 prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet
(
    IN GT_U32      lookupId,
    IN  GT_U32     relativeTcamEntryIndex
);

/**
* @internal prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet function
* @endinternal
*
* @brief   Get absolute EPCL rules index bases in TCAM for EPCL
*         the 'relative index' is modulo (%) by the function according to
*         appDemoDxChTcamEpclNumOfIndexsGet()
*         relevant for earch devices, otherwise returns relativeTcamEntryIndex
* @param[in] relativeTcamEntryIndex
*/
GT_U32 prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet
(
    IN  GT_U32     relativeTcamEntryIndex
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPclGenh */
