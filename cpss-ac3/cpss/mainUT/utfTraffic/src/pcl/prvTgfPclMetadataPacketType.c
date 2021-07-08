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
* @file prvTgfPclMetadataPacketType.c
*
* @brief IPCL Metadata Packet Type/Applicable Flow Sub-template advanced UT.
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTcamGen.h>
#include <common/tgfCosGen.h>

#include <pcl/prvTgfPclMetadataPacketType.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS    0

/* target port */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS 1

/* VLAN 0 Id */
#define PRV_TGF_VLANID_CNS           5
#define PRV_TGF_VLANID_MODIFY_CNS    6

/* user defined ethertype */
#define PRV_TGF_UDE_TAG_CNS         0x8888

/* IPCL rule & action index */
#define IPCL_MATCH_INDEX        10

/* TTI & IPCL rule & action indexes */
#define PRV_TGF_TTI_IPV4_O_MPLS_IDX_CNS   1
#define PRV_TGF_TTI_IPV6_O_MPLS_IDX_CNS   2

/* MPLS labels */
#define PRV_TGF_IPV4_O_MPLS_LABEL_CNS   49
#define PRV_TGF_IPV6_O_MPLS_LABEL_CNS   51

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* Ingress UDB Packet Type values in IPCL Metadata
0 = IPv4 TCP
1 = IPv4 UDP
2 = MPLS
3 = IPv4 Fragment
4 = IPv4
5 = Ethernet
6 = IPv6
7 = IPv6-TCP
8 = IPv6-UDP
9..15 = User Defined EtherType (UDE0-6)
Passenger header is used in case of tunnel terminated packet.
*/
static GT_U8 prvTgfMetadataIngressUdbPacketType[] = {0, 5, 6, 1, 2, 3, 5, 4, 9, 7, 2, 8, 2};

/* Ingress UDB Packet Type values in IPCL Metadata */
static GT_U8 prvTgfMetadataIngressUdbIpOverMplsPacketType[] = {4, 6};

/* Applicable Flow Sub-template field values in IPCL Metadata
1 = IPv4_TCP
2 = IPv4_UDP
4 = MPLS
8 = IPv4_FRAGMENT
16 = IPv4_OTHER
32 = ETHERNET_OTHER
64 = IPv6
128 = UDE0-4
Passenger header is used in case of tunnel terminated packet.
 */
static GT_U8 prvTgfMetadataApplicableFlowSubTemplate[] = {1, 32, 64, 2, 4, 8, 32, 16, 128, 64, 4, 64, 4};

/* Applicable Flow Sub-template field values in IPCL Metadata */
static GT_U8 prvTgfMetadataApplicableFlowSubTemplateIpOverMpls[] = {4, 4};

/* Packet Type field values in IPCL Metadata
Possible values:
0=Other
1=ARP
2=IPv6overMPLS
3=IPv4overMPLS
4=MPLS
5=Reserved
6=IPv6
7=IPv4
Passenger header is used in case of tunnel terminated packet.
IPv6overMPLS and IPv4overMPLS are assigned for non tunnel terminated packet
and only if L3 L4 Parsing Over Mpls enabled by cpssDxChPclL3L4ParsingOverMplsEnableSet.
 */
static GT_U8 prvTgfMetadataPacketType[] = {7, 1, 6, 7, 4, 7, 0, 7, 0, 6, 7, 6, 6};

/* Packet Type field values in IPCL Metadata */
static GT_U8 prvTgfMetadataPacketTypeIpOverMpls[] = { 3, 2};


/* IPCL Metadata fields information */
static PRV_TGF_PCL_METADATA_OFFSETS_STC prvTgfPclIngressMetadataInfo[] =
{
    {0, 0, 28 },   /* PRV_TGF_PCL_METADATA_FIELDS_PORT_LIST_SRC_E */
    {3, 4, 12 },   /* PRV_TGF_PCL_METADATA_FIELDS_LOCAL_DEV_SRC_TRUNK_ID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_LOCAL_DEV_SRC_EPORT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_SRC_DEV_IS_OWN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_1_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_DEV_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_IS_TRUNK_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_EPORT_OR_TRUNK_ID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_LOCAL_DEV_SRC_PORT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_USE_VIDX_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_EVIDX_E */
    {13, 1, 4 },   /* PRV_TGF_PCL_METADATA_FIELDS_INGRESS_UDB_PACKET_TYPE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IP_LEGAL_OR_FCOE_LEGAL_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_L2_VALID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_ARP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TRG_DEV_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TRG_IS_TRUNK_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TRG_EPORT_OR_TRG_TRUNK_ID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TRG_PHY_PORT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_TRG_PHY_PORT_VALID_E */
    {18, 1, 4 },   /* PRV_TGF_PCL_METADATA_FIELDS_EGRESS_UDB_PACKET_TYPE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_2_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_SRC_ID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_EGRESS_FILTER_REGISTERED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_PACKET_IS_LOOPED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_DROP_ON_SOURCE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_3_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_PHY_SRC_MC_FILTER_EN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_PHY_IS_TRUNK_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_4_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_L2_VALID_1_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_L2_ENCAPSULATION_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_BYPASS_BRIDGE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_MAC_DA_TYPE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_OUTER_IS_LLC_NON_SNAP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_INNER_IS_LLC_NON_SNAP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ETHERTYPE_OR_DSAPSSAP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TAG0_SRC_TAGGED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TAG1_SRC_TAGGED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_SRC_TAG0_IS_OUTER_TAG_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_OUTER_SRC_TAG_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TAG1_LOCAL_DEV_SRC_TAGGED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_NESTED_VLAN_EN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TAG0_PRIO_TAGGED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TAG1_PRIO_TAGGED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ORIG_VID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_OVERRIDE_EVLAN_WITH_ORIGVID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TRG_TAGGED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_EVLAN_PRECEDENCE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_5_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TAG0_TPID_INDEX_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TAG1_TPID_INDEX_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_NUM_OF_TAG_WORDS_TO_POP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IPX_PROTOCOL_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IPV4_SIP_OR_ARP_SIP_OR_FCOE_S_ID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IPV4_DIP_OR_ARP_DIP_OR_FCOE_D_ID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_POLICY_RTT_INDEX_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_L3_OFFSET_INVALID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IPV4_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_FCOE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_ARP_1_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IP_LEGAL_OR_FCOE_LEGAL_1_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IPM_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IP_HEADER_INFO_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IP_FRAGMENTED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_FRAGMENTED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ROUTED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_DO_ROUTE_HA_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_MAC_SA_ARP_SA_MISMATCH_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IPX_HEADER_LENGTH_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_6_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IP_1_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_1_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_ND_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_LINK_LOCAL_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_MLD_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IPV6_HBH_EXT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IPV6_EH_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_SOLICITATION_MULTICAST_MESSAGE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IP_2_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_IPV6_2_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IPV6_FLOW_LABEL_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_7_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TCP_UDP_DEST_PORT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TCP_OR_UDP_PORT_COMPARATORS_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_L4_OFFSET_INVALID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_L4_VALID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_SYN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_SYN_WITH_DATA_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_8_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_L2_ECHO_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_CFM_PACKET_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TIMESTAMP_EN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TIMESTAMP_TAGGED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_9_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TIMESTAMP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_PTP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_PTP_DOMAIN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_PTP_U_FIELD_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_PTP_TAI_SELECT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_PTP_TRIGGER_TYPE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_OAM_PROCESSING_EN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_OAM_PTP_OFFSET_INDEX_OR_PTP_OFFSET_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RX_SNIFF_OR_SRC_TRG_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_SRC_TRG_EPORT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_SRC_TRG_DEV_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ANALYZER_INDEX_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_10_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_SRC_TRG_PHY_PORT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_MPLS_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_MPLS_CMD_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_11_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ECN_CAPABLE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_QCN_RX_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ORIG_RX_QCN_PRIO_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_12_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TUNNEL_TERMINATED_E */
    {66, 1, 3 },   /* PRV_TGF_PCL_METADATA_FIELDS_INNER_PACKET_TYPE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TUNNEL_START_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_TUNNEL_START_PASSENGER_TYPE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_ARP_PTR_OR_TUNNEL_PTR_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_13_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_QOS_PROFILE_PRECEDENCE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_QOS_PROFILE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_MODIFY_UP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_MODIFY_DSCP_EXP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_14_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_BYTE_COUNT_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RECALC_CRC_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_15_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_BYPASS_INGRESS_PIPE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_MARVELL_TAGGED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_MARVELL_TAGGED_EXTENDED_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_16_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RECEIVED_DSA_TAG_WORD_0_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RECEIVED_DSA_TAG_WORD_1_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RECEIVED_DSA_TAG_WORD_2_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RECEIVED_DSA_TAG_WORD_3_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_IS_TRILL_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_REP_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_REP_LAST_E */
    {90, 3, 3 },   /* PRV_TGF_PCL_METADATA_FIELDS_PACKET_TYPE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_17_E */
    {91, 0, 8 },   /* PRV_TGF_PCL_METADATA_FIELDS_APPLICABLE_FLOW_SUB_TEMPLATE_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_METERING_EN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_BILLING_EN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_18_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_POLICER_PTR_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_INGRESS_CORE_ID_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RX_IS_PROTECTION_PATH_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RX_PROTECTION_SWITCH_EN_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_19_E */
    {0, 0, 0 },    /* PRV_TGF_PCL_METADATA_FIELDS_COPY_RESERVED_E */
    {0, 0, 0 }     /* PRV_TGF_PCL_METADATA_FIELDS_RESERVED_20_E */

};

/* Test Rules array */
static GT_U8 prvTgfMetadataTestRulesArr[] = {
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
};


/* Test Rules array */
static GT_U8 prvTgfMetadataTestIpOverMplsRulesArr[] = {
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E
};

/* Test Packet Types array */
static GT_U8 prvTgfMetadataTestPacketTypeArr[] = {
    PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E,
    PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
    PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
    PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
    PRV_TGF_PCL_PACKET_TYPE_MPLS_E,
    PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E,
    PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
    PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
    PRV_TGF_PCL_PACKET_TYPE_IPV6_TCP_E,
    PRV_TGF_PCL_PACKET_TYPE_MPLS_E,
    PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,
    PRV_TGF_PCL_PACKET_TYPE_MPLS_E
};

/* Test Packet Types array */
static GT_U8 prvTgfMetadataTestIpOverMplsPacketTypeArr[] = {
    PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
    PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E
};

/************************* General packet's parts *****************************/

/* L2 part of packet.
   Please don't set MAC-DA[1] to zero. UDB-only PCL rules use it guarantee
   the TCAM rule pattern is non-zero. */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x11}                 /* saMac */
};

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* ethertype part of IPV6 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* ethertype part of ETH_OTHER packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherOtherTypePart = {0x2222};

/* Data of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};
/******************************************************************************/

/******************************* TCP packet **********************************/

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketTcpIpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x42,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    6,                  /* protocol */
    0x4C87,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    123456,             /* sequence number */
    234567,             /* acknowledgment number */
    5,                  /* data offset */
    0x50,               /* reserved */
    0x10,               /* flags */
    4096,               /* window */
    0xFAF6,             /* csum */
    0                   /* urgent pointer */
};

/* PARTS of packet TCP */
static TGF_PACKET_PART_STC prvTgfTcpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTcpIpPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of TCP packet */
#define PRV_TGF_TCP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS \
    + TGF_TCP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of TCP packet with CRC */
#define PRV_TGF_TCP_PACKET_CRC_LEN_CNS  PRV_TGF_TCP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* TCP packet to send */
static TGF_PACKET_STC prvTgfTcpPacketInfo =
{
    PRV_TGF_TCP_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfTcpPacketPartArray) / sizeof(prvTgfTcpPacketPartArray[0]), /* numOfParts */
    prvTgfTcpPacketPartArray                                                /* partsArray */
};
/******************************************************************************/

/******************************* UDP packet **********************************/

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacketUdpIpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x36,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    17,                 /* protocol */
    0x4C88,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    8,                                             /* src port */
    0,                                             /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS              /* csum */
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfUdpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketUdpIpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of UDP packet */
#define PRV_TGF_UDP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS \
    + TGF_UDP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of UDP packet with CRC */
#define PRV_TGF_UDP_PACKET_CRC_LEN_CNS  PRV_TGF_UDP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* UDP packet to send */
static TGF_PACKET_STC prvTgfUdpPacketInfo =
{
    PRV_TGF_UDP_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfUdpPacketPartArray) / sizeof(prvTgfUdpPacketPartArray[0]), /* numOfParts */
    prvTgfUdpPacketPartArray                                                /* partsArray */
};
/******************************************************************************/

/******************************* IPv4 packet **********************************/

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OtherPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    4,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C99,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4FragmentPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    1,                  /* flags */    /* more fragments     */
    0x200,              /* offset */   /* not first fragment */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    0x4C99,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet IPv4 - IPV4 OTHER */
static TGF_PACKET_PART_STC prvTgfIpv4OtherPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet IPv4 - IPV4 FRAGMENT */
static TGF_PACKET_PART_STC prvTgfIpv4FragmentPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4FragmentPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of IPv4 packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadDataArr)

/* Length of IPv4 packet with CRC */
#define PRV_TGF_IPV4_PACKET_CRC_LEN_CNS  PRV_TGF_IPV4_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* IPv4 Other packet to send */
static TGF_PACKET_STC prvTgfIpv4OtherPacketInfo =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                      /* totalLen */
    (sizeof(prvTgfIpv4OtherPacketPartArray)
        / sizeof(prvTgfIpv4OtherPacketPartArray[0])), /* numOfParts */
    prvTgfIpv4OtherPacketPartArray                    /* partsArray */
};

/* IPv4 Fragment packet to send */
static TGF_PACKET_STC prvTgfIpv4FragmentPacketInfo =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                         /* totalLen */
    (sizeof(prvTgfIpv4FragmentPacketPartArray)
        / sizeof(prvTgfIpv4FragmentPacketPartArray[0])), /* numOfParts */
    prvTgfIpv4FragmentPacketPartArray                    /* partsArray */
};

/******************************************************************************/

/***************************** Ethernet packet ********************************/

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfEthernetPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherOtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of Ethernet packet */
#define PRV_TGF_ETHERNET_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Length of Ethernet packet with CRC */
#define PRV_TGF_ETHERNET_PACKET_CRC_LEN_CNS  \
    PRV_TGF_ETHERNET_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* Ethernet packet to send */
static TGF_PACKET_STC prvTgfEthernetPacketInfo =
{
    PRV_TGF_ETHERNET_PACKET_LEN_CNS,                 /* totalLen */
    (sizeof(prvTgfEthernetPacketPartArray)
        / sizeof(prvTgfEthernetPacketPartArray[0])), /* numOfParts */
    prvTgfEthernetPacketPartArray                    /* partsArray */
};
/******************************************************************************/

/******************************* MPLS packet **********************************/

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart = {TGF_ETHERTYPE_8847_MPLS_TAG_CNS};

/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsPart =
{
    1,                  /* label */
    0,                  /* experimental use */
    1,                  /* stack */
    0x40                /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of MPLS packet */
#define PRV_TGF_MPLS_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_MPLS_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadLongDataArr)

/* Length of MPLS packet with CRC */
#define PRV_TGF_MPLS_PACKET_CRC_LEN_CNS  \
    PRV_TGF_MPLS_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* MPLS packet to send */
static TGF_PACKET_STC prvTgfMplsPacketInfo =
{
    PRV_TGF_MPLS_PACKET_LEN_CNS,                 /* totalLen */
    (sizeof(prvTgfMplsPacketPartArray)
        / sizeof(prvTgfMplsPacketPartArray[0])), /* numOfParts */
    prvTgfMplsPacketPartArray                    /* partsArray */
};
/******************************* IPv6 packet **********************************/

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part =
{
    6,                                  /* version */
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    0x2E,                               /* payloadLen */
    41,                                 /* nextHeader */
    0x40,                               /* hopLimit */
    {22, 22, 22, 22, 22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  1,  1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet IPv6 */
static TGF_PACKET_PART_STC prvTgfIpv6PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of IPv6 packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadDataArr)

/* Length of IPv6 packet with CRC */
#define PRV_TGF_IPV6_PACKET_CRC_LEN_CNS  PRV_TGF_IPV6_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* IPv6 packet to send */
static TGF_PACKET_STC prvTgfIpv6PacketInfo =
{
    PRV_TGF_IPV6_PACKET_LEN_CNS,                                              /* totalLen */
    sizeof(prvTgfIpv6PacketPartArray) / sizeof(prvTgfIpv6PacketPartArray[0]), /* numOfParts */
    prvTgfIpv6PacketPartArray                                                 /* partsArray */
};
/******************************************************************************/

/***************************** IPv6 TCP packet ********************************/

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6IpPartForTcp =
{
    6,                                  /* version */
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    0x2E,                               /* payloadLen */
    6,                                  /* TCP protocol number */
    0x40,                               /* hopLimit */
    {22, 22, 22, 22, 22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  1,  1,  1,  1,  3}    /* dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketIpv6TcpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    123456,             /* sequence number */
    234567,             /* acknowledgment number */
    5,                  /* data offset */
    0x50,               /* reserved */
    0x10,               /* flags */
    4096,               /* window */
    0xFAF6,             /* csum */
    0                   /* urgent pointer */
};

/* PARTS of packet IPv6 */
static TGF_PACKET_PART_STC prvTgfIpv6TcpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6IpPartForTcp},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketIpv6TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of IPv6 packet */
#define PRV_TGF_IPV6_TCP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS \
    + TGF_TCP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of IPv6 packet with CRC */
#define PRV_TGF_IPV6_TCP_PACKET_CRC_LEN_CNS  PRV_TGF_IPV6_TCP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* IPv6 packet to send */
static TGF_PACKET_STC prvTgfIpv6TcpPacketInfo =
{
    PRV_TGF_IPV6_TCP_PACKET_LEN_CNS,                                                /* totalLen */
    sizeof(prvTgfIpv6TcpPacketPartArray) / sizeof(prvTgfIpv6TcpPacketPartArray[0]), /* numOfParts */
    prvTgfIpv6TcpPacketPartArray                                                    /* partsArray */
};

/******************************************************************************/

/***************************** IPv6 UDP packet ********************************/

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6IpPartForUdp =
{
    6,                                  /* version */
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    0x2E,                               /* payloadLen */
    17,                                  /* protocol number */
    0x40,                               /* hopLimit */
    {22, 22, 22, 22, 22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  1,  1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet IPv6 */
static TGF_PACKET_PART_STC prvTgfIpv6UdpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6IpPartForUdp},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of IPv6 packet */
#define PRV_TGF_IPV6_UDP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS \
    + TGF_UDP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* Length of IPv6 packet with CRC */
#define PRV_TGF_IPV6_UDP_PACKET_CRC_LEN_CNS  PRV_TGF_IPV6_UDP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* IPv6 packet to send */
static TGF_PACKET_STC prvTgfIpv6UdpPacketInfo =
{
    PRV_TGF_IPV6_UDP_PACKET_LEN_CNS,                                                /* totalLen */
    sizeof(prvTgfIpv6UdpPacketPartArray) / sizeof(prvTgfIpv6UdpPacketPartArray[0]), /* numOfParts */
    prvTgfIpv6UdpPacketPartArray                                                    /* partsArray */
};

/******************************************************************************/

/******************************* UDE packet ***********************************/

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketUsedDefinedEtherTypePart = {PRV_TGF_UDE_TAG_CNS};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfUdePacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketUsedDefinedEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of UDE packet */
#define PRV_TGF_UDE_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Length of UDE packet with CRC */
#define PRV_TGF_UDE_PACKET_CRC_LEN_CNS  PRV_TGF_UDE_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* UDE packet to send */
static TGF_PACKET_STC prvTgfUdePacketInfo =
{
    PRV_TGF_UDE_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfUdePacketPartArray) / sizeof(prvTgfUdePacketPartArray[0]), /* numOfParts */
    prvTgfUdePacketPartArray                                                /* partsArray */
};

/******************************************************************************/

/****************************** packet ARP ************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketArpL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                 /* saMac */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketArpEtherTypePart = {
    TGF_ETHERTYPE_0806_ARP_TAG_CNS
};
/* packet's ARP header */
static TGF_PACKET_ARP_STC prvTgfPacketArpPart = {
    0x01, 0x800, 0x06, 0x04, 0x01,          /* hwType, protType, hwLen, protLen, opCode */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* srcMac */
    {0x01, 0x01, 0x01, 0x01},               /* srcIp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   /* dstMac */
    {0x00, 0x00, 0x00, 0x00}                /* dstIp */
};
/* packet's payload part */
static GT_U8 prvTgfPacketArpPayloadDataArr[] = {
    0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketArpPayloadPart = {
    sizeof(prvTgfPacketArpPayloadDataArr),               /* dataLength */
    prvTgfPacketArpPayloadDataArr                        /* dataPtr */
};

/* PARTS of packet ARP */
static TGF_PACKET_PART_STC prvTgfPacketArpArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketArpL2Part},     /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketArpEtherTypePart},
    {TGF_PACKET_PART_ARP_E,       &prvTgfPacketArpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketArpPayloadPart}
};

/* PACKET ARP to send */
static TGF_PACKET_STC prvTgfPacketArpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_ARP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketArpPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketArpArray) / sizeof(TGF_PACKET_PART_STC),     /* numOfParts */
    prvTgfPacketArpArray                                            /* partsArray */
};

/******************************************************************************/

/************************* Ipv4 Over MPLS packet ******************************/
/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls1Part =
{
    PRV_TGF_IPV4_O_MPLS_LABEL_CNS,    /* label */
    1,                                /* experimental use */
    1,                                /* stack */
    0xFF                              /* timeToLive */
};

/* PARTS of the packet */
static TGF_PACKET_PART_STC prvTgfIpv4OverMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls1Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of packet */
#define PRV_TGF_IPV4_OVER_MPLS_PACKET_LEN_CNS                                \
    TGF_L2_HEADER_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS +sizeof(prvTgfPacketPayloadLongDataArr)

/* Packet to send */
static TGF_PACKET_STC prvTgfIpv4OverMplsPacketInfo =
{
    PRV_TGF_IPV4_OVER_MPLS_PACKET_LEN_CNS,                    /* totalLen */
    sizeof(prvTgfIpv4OverMplsPacketPartArray)
        / sizeof(prvTgfIpv4OverMplsPacketPartArray[0]),        /* numOfParts */
    prvTgfIpv4OverMplsPacketPartArray                          /* partsArray */
};

/******************************************************************************/

/************************* Ipv6 Over MPLS packet ******************************/
/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls2Part =
{
    PRV_TGF_IPV6_O_MPLS_LABEL_CNS,    /* label */
    1,                                /* experimental use */
    1,                                /* stack */
    0xFF                              /* timeToLive */
};

/* PARTS of the packet */
static TGF_PACKET_PART_STC prvTgfIpv6OverMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls2Part},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of packet */
#define PRV_TGF_IPV6_OVER_MPLS_PACKET_LEN_CNS                                \
    TGF_L2_HEADER_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV6_HEADER_SIZE_CNS +sizeof(prvTgfPacketPayloadLongDataArr)

/* Packet to send */
static TGF_PACKET_STC prvTgfIpv6OverMplsPacketInfo =
{
    PRV_TGF_IPV6_OVER_MPLS_PACKET_LEN_CNS,                    /* totalLen */
    sizeof(prvTgfIpv6OverMplsPacketPartArray)
        / sizeof(prvTgfIpv6OverMplsPacketPartArray[0]),        /* numOfParts */
    prvTgfIpv6OverMplsPacketPartArray                          /* partsArray */
};

/******************************************************************************/

/* Test Packets array */
static TGF_PACKET_STC* prvTgfMetadataTestPackets[] = {
    &prvTgfTcpPacketInfo,
    &prvTgfPacketArpInfo,
    &prvTgfIpv6PacketInfo,
    &prvTgfUdpPacketInfo,
    &prvTgfMplsPacketInfo,
    &prvTgfIpv4FragmentPacketInfo,
    &prvTgfEthernetPacketInfo,
    &prvTgfIpv4OtherPacketInfo,
    &prvTgfUdePacketInfo,
    &prvTgfIpv6TcpPacketInfo,
    &prvTgfIpv4OverMplsPacketInfo,
    &prvTgfIpv6UdpPacketInfo,
    &prvTgfIpv6OverMplsPacketInfo,
    &prvTgfTcpPacketInfo
};

static TGF_PACKET_STC* prvTgfMetadataTestIpOverMplsPackets[] = {
    &prvTgfIpv4OverMplsPacketInfo,
    &prvTgfIpv6OverMplsPacketInfo,
    &prvTgfTcpPacketInfo
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfPclMetadataTtiRuleAndActionBuild function
* @endinternal
*
* @brief   Build TTI rule and action
*
* @param[in] index                    - rule and action indexes
* @param[in] mplsLabel                - MPLS label to match
*                                       None
*/
static GT_VOID prvTgfPclMetadataTtiRuleAndActionBuild
(
    IN GT_U32    index,
    IN GT_U32    mplsLabel
)
{
    GT_STATUS   rc;

    PRV_TGF_TTI_ACTION_2_STC    ttiAction;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    ttiPattern.mpls.label0 = mplsLabel;
    ttiMask.mpls.label0 = 0xFFFFF;

    ttiAction.tunnelTerminate = GT_TRUE;
    ttiAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    rc = prvTgfTtiRule2Set(index, PRV_TGF_TTI_KEY_MPLS_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d, %d",
                                 index, PRV_TGF_TTI_KEY_MPLS_E);
}


/**
* @internal prvTgfPclMetadataTestTtiConfiguration function
* @endinternal
*
* @brief   Set TTI configurations - enabling, rules & actions
*/
static GT_VOID prvTgfPclMetadataTestTtiConfiguration
(
    GT_VOID
)
{
    GT_STATUS   rc;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable the TTI lookup for MPLS on port */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);

    /* AUTODOC: create rule & action for IPv4-over-MPLS */
    prvTgfPclMetadataTtiRuleAndActionBuild( PRV_TGF_TTI_IPV4_O_MPLS_IDX_CNS,
                                            PRV_TGF_IPV4_O_MPLS_LABEL_CNS);

    /* AUTODOC: create rule & action for IPv6-over-MPLS */
    prvTgfPclMetadataTtiRuleAndActionBuild( PRV_TGF_TTI_IPV6_O_MPLS_IDX_CNS,
                                            PRV_TGF_IPV6_O_MPLS_LABEL_CNS);

    prvTgfPclTunnelTermForceVlanModeEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc,
                 "prvTgfPclTunnelTermForceVlanModeEnableSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfPclMetadataTestTtiConfigurationRestore function
* @endinternal
*
* @brief   Restore TTI configurations - enabling, rules & actions
*/
static GT_VOID prvTgfPclMetadataTestTtiConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_IPV4_O_MPLS_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d",
                                 PRV_TGF_TTI_IPV4_O_MPLS_IDX_CNS, GT_FALSE);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_IPV6_O_MPLS_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d",
                                 PRV_TGF_TTI_IPV6_O_MPLS_IDX_CNS, GT_FALSE);

    /* AUTODOC: Disable the TTI lookup for MPLS on port */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    prvTgfPclTunnelTermForceVlanModeEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
      GT_OK, rc, "prvTgfPclTunnelTermForceVlanModeEnableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclMetadataTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*
* @param[in] ruleFormat               - format of the Rule
* @param[in] metaDataFieldBitOffset   - field offset in metadata in bits
* @param[in] metaDataFieldBitLength   - lengh of field in metadata in bits
* @param[in] value                    -  to match
*                                       None
*/
static GT_VOID prvTgfPclMetadataTestConfigurationSet
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN GT_U32                               metaDataFieldBitOffset,
    IN GT_U32                               metaDataFieldBitLength,
    IN GT_U32                               value
)
{
    GT_STATUS                         rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_ACTION_STC            action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ethOthKey;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ipv4Key;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ipv6Key;
    GT_BOOL                           isUdbOnly   = GT_FALSE;
    GT_U32                            udbAbsIndex = 31;
    PRV_TGF_PCL_UDB_SELECT_STC        udbSelect;
    GT_U32                            metaDataFieldByteOffset;
    GT_U32                            metaDataFieldByteLength;
    GT_U32                            metaDataFieldBitShift;
    GT_U32                            byteIdx;
    GT_U32                            byteBitShift;
    GT_U32                            byteBitLength;
    GT_U32                            w;
    GT_U8                             udbMask;
    GT_U8                             udbPattern;
    PRV_TGF_PCL_PACKET_TYPE_ENT       packetTypeIter;

    /* AUTODOC: SETUP CONFIGURATION: */

    ethOthKey = ruleFormat;
    ipv4Key   = ruleFormat;
    ipv6Key   = ruleFormat;

    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E:
            isUdbOnly   = GT_TRUE;
            udbAbsIndex = 30;
            break ;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            /* ipv6Key doesn't support this Ext format. Use another one */
            ipv6Key   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;
            break ;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            /* ethOthKey, ipv6Key don't support this Std format. Use another one */
            ethOthKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
            ipv6Key   = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
            break ;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            udbAbsIndex = 35;
            break ;
        default:
            UTF_VERIFY_EQUAL0_STRING_MAC(1,0, "Unhandled rule format");
    }

    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        ethOthKey /*nonIpKey*/,
        ipv4Key /*ipv4Key*/,
        ipv6Key /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* AUTODOC: Metadata UDBs configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    metaDataFieldByteOffset = (metaDataFieldBitOffset / 8);
    metaDataFieldBitShift   = (metaDataFieldBitOffset % 8);
    metaDataFieldByteLength = ((metaDataFieldBitShift + metaDataFieldBitLength + 7) / 8);

    if (isUdbOnly != GT_FALSE)
    {
        cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));
        /* except UDBs requred to store metadata, add one more UDB for MAC DA[1].*/
        for (byteIdx = 0; (byteIdx <= metaDataFieldByteLength); byteIdx++)
        {
            udbSelect.udbSelectArr[byteIdx] = (udbAbsIndex + byteIdx);
        }

        for (packetTypeIter = PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E;
             packetTypeIter < PRV_TGF_PCL_PACKET_TYPE_LAST_E; packetTypeIter++)
        {
            if (packetTypeIter == PRV_TGF_PCL_PACKET_TYPE_IPV6_E)
            {
                /* is not relevant for sip5 */
                continue;
            }
            rc = prvTgfPclUserDefinedBytesSelectSet(ruleFormat, packetTypeIter,
                                                    CPSS_PCL_LOOKUP_0_0_E,
                                                    &udbSelect);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }

    for (byteIdx = 0; (byteIdx < metaDataFieldByteLength); byteIdx++)
    {
        for (packetTypeIter = PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E;
             packetTypeIter < PRV_TGF_PCL_PACKET_TYPE_LAST_E; packetTypeIter++)
        {
            rc = prvTgfPclUserDefinedByteSet(
                ruleFormat, packetTypeIter, CPSS_PCL_DIRECTION_INGRESS_E, (udbAbsIndex + byteIdx),
                PRV_TGF_PCL_OFFSET_METADATA_E, (GT_U8)(metaDataFieldByteOffset + byteIdx));
            UTF_VERIFY_EQUAL4_STRING_MAC( GT_OK, rc,
                "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d, OffsetType %d, Offset %d\n",
                packetTypeIter, (udbAbsIndex + byteIdx),
                PRV_TGF_PCL_OFFSET_METADATA_E, (metaDataFieldByteOffset + byteIdx));
        }
        byteBitShift   = ((byteIdx == 0) ? metaDataFieldBitShift : 0);
        if ((byteIdx + 1) < metaDataFieldByteLength)
        {
            /* part in not last byte */
            byteBitLength  = (8 - byteBitShift);
        }
        else
        {
            /* part in last byte */
            if (byteIdx == 0)
            {
                byteBitLength  = metaDataFieldBitLength;
            }
            else
            {
                w = ((metaDataFieldBitShift + metaDataFieldBitLength) % 8);
                byteBitLength  = ((w == 0) ? 8 : w);
            }
        }

        udbMask     = (GT_U8)(((1 << byteBitLength) - 1) << byteBitShift);
        udbPattern  = (GT_U8)(udbMask & ((value << metaDataFieldBitShift) >> (byteIdx * 8)));

        /* AUTODOC: mask and pattern for Packet Type */
        switch (ruleFormat)
        {
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
                  mask.ruleStdIpv4L4.udb[udbAbsIndex + byteIdx]    = udbMask;
                  pattern.ruleStdIpv4L4.udb[udbAbsIndex + byteIdx] = udbPattern;
                break ;
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
                  mask.ruleExtIpv4PortVlanQos.udb[udbAbsIndex + byteIdx]    = udbMask;
                  pattern.ruleExtIpv4PortVlanQos.udb[udbAbsIndex + byteIdx] = udbPattern;
                break ;
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
                  mask.ruleStdUdb.udb[udbAbsIndex + byteIdx]    = udbMask;
                  pattern.ruleStdUdb.udb[udbAbsIndex + byteIdx] = udbPattern;
                break ;
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E:
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E:
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E:
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E:
            case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E:
                  mask.ruleIngrUdbOnly.udb[byteIdx]    = udbMask;
                  pattern.ruleIngrUdbOnly.udb[byteIdx] = udbPattern;
                break ;
            default:
                break ;
        }

    }

    /* AUTODOC: add UDB pointing to MAC DA[1] for UDB-only rules to ensure the rule is not zero. */
    if (isUdbOnly != GT_FALSE)
    {
        /* byteIdx == metaDataFieldByteLength */
        for (packetTypeIter = PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E;
             packetTypeIter < PRV_TGF_PCL_PACKET_TYPE_LAST_E; packetTypeIter++)
        {
            rc = prvTgfPclUserDefinedByteSet(ruleFormat, packetTypeIter,
                                             CPSS_PCL_DIRECTION_INGRESS_E,
                                             udbAbsIndex + byteIdx,
                                             PRV_TGF_PCL_OFFSET_L2_E,
                                             1);
            UTF_VERIFY_EQUAL3_STRING_MAC( GT_OK, rc,
                "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d, OffsetType %d\n",
                 packetTypeIter, udbAbsIndex + byteIdx, PRV_TGF_PCL_OFFSET_METADATA_E);
        }

        pattern.ruleIngrUdbOnly.udb[byteIdx] = prvTgfPacketL2Part.daMac[1];
        mask.ruleIngrUdbOnly.udb[byteIdx]    = 0xff;
    }

    /* AUTODOC: action - tag0 vlan value change */
    action.vlan.modifyVlan =  CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId = PRV_TGF_VLANID_MODIFY_CNS;

    rc = prvTgfPclRuleSet(ruleFormat, IPCL_MATCH_INDEX, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d", IPCL_MATCH_INDEX);
}

/**
* @internal prvTgfPclMetadataTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*
* @param[in] ruleFormat               - format of the Rule
* @param[in] ruleIndex                - rule Index
*                                       None
*/
static GT_VOID prvTgfPclMetadataTestConfigurationRestore
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat,
    IN GT_U32                               ruleIndex
)
{
    GT_STATUS                       rc;
    CPSS_PCL_RULE_SIZE_ENT          ruleSize;
    GT_U32                          udbIndex;
    PRV_TGF_PCL_PACKET_TYPE_ENT     packetTypeIter;

    /* AUTODOC: RESTORE CONFIGURATION: */

    for (udbIndex = 30; (udbIndex < 50); udbIndex++)
    {
        for (packetTypeIter = PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E;
             packetTypeIter < PRV_TGF_PCL_PACKET_TYPE_LAST_E; packetTypeIter++)
        {
            rc = prvTgfPclUserDefinedByteSet(
                ruleFormat, packetTypeIter, CPSS_PCL_DIRECTION_INGRESS_E, udbIndex,
                PRV_TGF_PCL_OFFSET_INVALID_E, 0/*offset*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d",
                                         udbIndex);
        }
    }

    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E:
            ruleSize = CPSS_PCL_RULE_SIZE_10_BYTES_E;
            break ;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E:
            ruleSize = CPSS_PCL_RULE_SIZE_20_BYTES_E;
            break ;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E:
            ruleSize = CPSS_PCL_RULE_SIZE_30_BYTES_E;
            break ;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E:
            ruleSize = CPSS_PCL_RULE_SIZE_40_BYTES_E;
            break ;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E:
            ruleSize = CPSS_PCL_RULE_SIZE_50_BYTES_E;
            break ;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            ruleSize = CPSS_PCL_RULE_SIZE_EXT_E;
            break ;
        default:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
            break ;
    }
    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(ruleSize, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 ruleSize, ruleIndex, GT_FALSE);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);
}

/**
* @internal prvTgfPclMetadataPacketTypeTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
static GT_VOID prvTgfPclMetadataPacketTypeTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS     rc;
    GT_U32         portsArray[1];
    GT_U8         tagArray[] = {0};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: create VLAN 5 with untagged port 0*/

    portsArray[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    rc = prvTgfBrgDefVlanEntryWithPortsSet( PRV_TGF_VLANID_CNS, portsArray, NULL,
                                            tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: create VLAN 6 with untagged port 1*/
    portsArray[0] = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    rc = prvTgfBrgDefVlanEntryWithPortsSet( PRV_TGF_VLANID_MODIFY_CNS, portsArray,
                                            NULL, tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_VLANID_MODIFY_CNS);

    /* AUTODOC: add FDB entry with daMac, vlanId, ingress port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet( prvTgfPacketL2Part.daMac,
        PRV_TGF_VLANID_CNS, prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with daMac, vlanId, egress port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet( prvTgfPacketL2Part.daMac,
        PRV_TGF_VLANID_MODIFY_CNS, prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* set CFM ethertype UDE0 */
    rc = prvTgfPclUdeEtherTypeSet( 0,/* UDE0 */ PRV_TGF_UDE_TAG_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}

/**
* @internal prvTgfPclMetadataPacketTypeTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
static GT_VOID prvTgfPclMetadataPacketTypeTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(  GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                   prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: Invalidate vlan entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: Invalidate vlan entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_MODIFY_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_MODIFY_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclMetadataPacketTypeTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclMetadataPacketTypeTestPacketSend
(
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      portIter;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}


/**
* @internal prvTgfPclMetadataPacketTypeTestCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPclMetadataPacketTypeTestCheckCounters
(
    IN GT_U32           expectedValue,
    IN GT_U32           callIdentifier
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                       prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL4_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                        "Rx another goodPktsRcv counter %d, than expected %d, on port %d, %d",
                                         portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
        else if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(expectedValue, portCntrs.goodPktsSent.l[0],
                        "Tx another goodPktsSent counter %d, than expected %d, on port %d, %d",
                                         portCntrs.goodPktsSent.l[0], expectedValue,
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
    }
}

/**
* @internal prvTgfPclMetadataPacketTypeTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
static GT_VOID prvTgfPclMetadataPacketTypeTestGenerateTraffic
(
    IN TGF_PACKET_STC   *firstPacketPtr,
    IN TGF_PACKET_STC   *secondPacketPtr
)
{
    /* AUTODOC: Send matched packet */
    prvTgfPclMetadataPacketTypeTestPacketSend(firstPacketPtr);

    /* AUTODOC: Verify packet drop */
    prvTgfPclMetadataPacketTypeTestCheckCounters(prvTgfBurstCount, 1);

    /* AUTODOC: Send unmatch packet */
    prvTgfPclMetadataPacketTypeTestPacketSend(secondPacketPtr);

    /* AUTODOC: Verify packet flooded in vlan */
    prvTgfPclMetadataPacketTypeTestCheckCounters(0, 2);
}

/**
* @internal prvTgfPclMetadataPacketTypeTest function
* @endinternal
*
* @brief   IPCL Metadata Ingress UDB Packet Type/Applicable Flow Sub-template test
*/
GT_VOID prvTgfPclMetadataPacketTypeTest
(
    GT_VOID
)
{
    GT_U32        caseIdx;

    /* AUTODOC: Set common test configuration */
    prvTgfPclMetadataPacketTypeTestConfigurationSet();

    /* AUTODOC: Iterate for all test Packet Types */
    for(caseIdx = 0; caseIdx < sizeof(prvTgfMetadataTestPacketTypeArr); caseIdx++)
    {

        PRV_UTF_LOG3_MAC(
            "\n*** Iteration %02d;  Rule: %02d; PacketType: %02d; ***\n\n",
            caseIdx,
            prvTgfMetadataTestRulesArr[caseIdx],
            prvTgfMetadataTestPacketTypeArr[caseIdx]);

        /* AUTODOC: Test Ingress UDB Packet Type field */

        prvTgfPclMetadataTestConfigurationSet(
            prvTgfMetadataTestRulesArr[caseIdx],
            ((prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_INGRESS_UDB_PACKET_TYPE_E].anchorOffset * 8)
              + prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_INGRESS_UDB_PACKET_TYPE_E].bitOffset),
            prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_INGRESS_UDB_PACKET_TYPE_E].fieldLength,
            prvTgfMetadataIngressUdbPacketType[caseIdx]);

        prvTgfPclMetadataPacketTypeTestGenerateTraffic(
            prvTgfMetadataTestPackets[caseIdx],
            prvTgfMetadataTestPackets[caseIdx + 1]);

        prvTgfPclMetadataTestConfigurationRestore(
            prvTgfMetadataTestRulesArr[caseIdx], IPCL_MATCH_INDEX);

        /* AUTODOC: Test Applicable Flow Sub-template field */

        prvTgfPclMetadataTestConfigurationSet(
            prvTgfMetadataTestRulesArr[caseIdx],
            ((prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_APPLICABLE_FLOW_SUB_TEMPLATE_E].anchorOffset * 8)
              + prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_APPLICABLE_FLOW_SUB_TEMPLATE_E].bitOffset),
            prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_APPLICABLE_FLOW_SUB_TEMPLATE_E].fieldLength,
            prvTgfMetadataApplicableFlowSubTemplate[caseIdx]);

        prvTgfPclMetadataPacketTypeTestGenerateTraffic(
            prvTgfMetadataTestPackets[caseIdx],
            prvTgfMetadataTestPackets[caseIdx + 1]);

        prvTgfPclMetadataTestConfigurationRestore(
            prvTgfMetadataTestRulesArr[caseIdx], IPCL_MATCH_INDEX);

        /* AUTODOC: Test Packet Type field */

        /* AUTODOC: configure TTI for IP over MPLS packets */
        prvTgfPclMetadataTestTtiConfiguration();

        prvTgfPclMetadataTestConfigurationSet(
            prvTgfMetadataTestRulesArr[caseIdx],
            ((prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_PACKET_TYPE_E].anchorOffset * 8)
              + prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_PACKET_TYPE_E].bitOffset),
            prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_PACKET_TYPE_E].fieldLength,
            prvTgfMetadataPacketType[caseIdx]);

        prvTgfPclMetadataPacketTypeTestGenerateTraffic(
            prvTgfMetadataTestPackets[caseIdx],
            prvTgfMetadataTestPackets[caseIdx + 1]);

        prvTgfPclMetadataTestConfigurationRestore(
            prvTgfMetadataTestRulesArr[caseIdx], IPCL_MATCH_INDEX);

        /* AUTODOC: restore configuration TTI for IP over MPLS packets */
        prvTgfPclMetadataTestTtiConfigurationRestore();
    }

    /* AUTODOC: Restore common test configuration */
    prvTgfPclMetadataPacketTypeTestConfigurationRestore();
}


/**
* @internal prvTgfPclMetadataPacketTypeIpOverMplsTest function
* @endinternal
*
* @brief   IPCL Metadata Ingress UDB Packet Type/Applicable Flow Sub-template test
*/
GT_VOID prvTgfPclMetadataPacketTypeIpOverMplsTest
(
    GT_VOID
)
{
    GT_U32        caseIdx;
    GT_STATUS     rc;

    /* AUTODOC: Set common test configuration */
    prvTgfPclMetadataPacketTypeTestConfigurationSet();

    /* set <L3L4ParsingOverMplsEnable> = GT_TRUE */
    rc = prvTgfPclL3L4ParsingOverMplsEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclL3L4ParsingOverMplsEnableSet: failed \n");

    /* AUTODOC: Iterate for all test Packet Types */
    for(caseIdx = 0; caseIdx < sizeof(prvTgfMetadataTestIpOverMplsPacketTypeArr); caseIdx++)
    {

        PRV_UTF_LOG3_MAC(
            "\n*** Iteration %02d;  Rule: %02d; PacketType: %02d; ***\n\n",
            caseIdx,
            prvTgfMetadataTestIpOverMplsRulesArr[caseIdx],
            prvTgfMetadataTestIpOverMplsPacketTypeArr[caseIdx]);

        /* AUTODOC: Test Ingress UDB Packet Type field */

        prvTgfPclMetadataTestConfigurationSet(
            prvTgfMetadataTestIpOverMplsRulesArr[caseIdx],
            ((prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_INGRESS_UDB_PACKET_TYPE_E].anchorOffset * 8)
              + prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_INGRESS_UDB_PACKET_TYPE_E].bitOffset),
            prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_INGRESS_UDB_PACKET_TYPE_E].fieldLength,
            prvTgfMetadataIngressUdbIpOverMplsPacketType[caseIdx]);

        prvTgfPclMetadataPacketTypeTestGenerateTraffic(
            prvTgfMetadataTestIpOverMplsPackets[caseIdx],
            prvTgfMetadataTestIpOverMplsPackets[caseIdx + 1]);

        prvTgfPclMetadataTestConfigurationRestore(
            prvTgfMetadataTestIpOverMplsRulesArr[caseIdx], IPCL_MATCH_INDEX);

        /* AUTODOC: Test Applicable Flow Sub-template field */

        prvTgfPclMetadataTestConfigurationSet(
            prvTgfMetadataTestIpOverMplsRulesArr[caseIdx],
            ((prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_APPLICABLE_FLOW_SUB_TEMPLATE_E].anchorOffset * 8)
              + prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_APPLICABLE_FLOW_SUB_TEMPLATE_E].bitOffset),
            prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_APPLICABLE_FLOW_SUB_TEMPLATE_E].fieldLength,
            prvTgfMetadataApplicableFlowSubTemplateIpOverMpls[caseIdx]);

        prvTgfPclMetadataPacketTypeTestGenerateTraffic(
            prvTgfMetadataTestIpOverMplsPackets[caseIdx],
            prvTgfMetadataTestIpOverMplsPackets[(caseIdx ? caseIdx + 1 : 2)]);

        prvTgfPclMetadataTestConfigurationRestore(
            prvTgfMetadataTestIpOverMplsRulesArr[caseIdx], IPCL_MATCH_INDEX);

        /* AUTODOC: Test Packet Type field */
        prvTgfPclMetadataTestConfigurationSet(
            prvTgfMetadataTestIpOverMplsRulesArr[caseIdx],
            ((prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_PACKET_TYPE_E].anchorOffset * 8)
              + prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_PACKET_TYPE_E].bitOffset),
            prvTgfPclIngressMetadataInfo[PRV_TGF_PCL_METADATA_FIELDS_PACKET_TYPE_E].fieldLength,
            prvTgfMetadataPacketTypeIpOverMpls[caseIdx]);

        prvTgfPclMetadataPacketTypeTestGenerateTraffic(
            prvTgfMetadataTestIpOverMplsPackets[caseIdx],
            prvTgfMetadataTestIpOverMplsPackets[caseIdx + 1]);

        prvTgfPclMetadataTestConfigurationRestore(
            prvTgfMetadataTestIpOverMplsRulesArr[caseIdx], IPCL_MATCH_INDEX);

    }

    /* set <L3L4ParsingOverMplsEnable> = GT_FALSE */
    rc = prvTgfPclL3L4ParsingOverMplsEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclL3L4ParsingOverMplsEnableSet: failed \n");

    /* AUTODOC: Restore common test configuration */
    prvTgfPclMetadataPacketTypeTestConfigurationRestore();
}

