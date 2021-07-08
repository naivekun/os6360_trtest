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
* @file prvCpssDxChHwTablesSip6.h
*
* @brief Private API definition for tables access of the Sip6 devices.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwTablesSip6h
#define __prvCpssDxChHwTablesSip6h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum SIP6_FDB_AU_MSG_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of (FDB) AU messages table in SIP6 devices.(FDB unit)
*/
typedef enum {
    /* fields from 'MAC NA moved' */
     SIP6_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_VALID_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SKIP_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_AGE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_CHAIN_TOO_LONG_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_OFFSET_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_UP0_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_DEV_ID_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_FID_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_10_7_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_0_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_4_1_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_6_5_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_11_7_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_6_3_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_2_1_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_0_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_11_7_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_6_1_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_0_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SP_UNKNOWN_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_MOVED_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_IS_TRUNK_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_EPORT_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_TRUNK_NUM_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_DEVICE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_SRC_ID_E

    /* start : The NA format from the device hold the fields in different place than
               those in the format from the CPU */
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_TRUNK_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_EPORT_NUM_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_TRUNK_NUM_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_10_7_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_0_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_4_1_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_6_5_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_11_7_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_6_3_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_2_1_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_0_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_11_7_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_6_1_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_0_E
    /* end : The NA format from the device hold the fields in different place than
               those in the format from the CPU */
    /******************************/
    /* extra fields from 'MAC NA' */
    /******************************/

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_VIDX_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_8_0_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_20_9_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_ENTRY_FOUND_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_DIP_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_26_0_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_30_27_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_31_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SEARCH_TYPE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_MULTIPLE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_CMD_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_SA_CMD_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E


    /****************************************************************/
    /* extra fields from 'IPv4 of FCOE or IPv6-Data Update Message' */
    /****************************************************************/
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MAC_ADDR_INDEX_20_9_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ECMP_POINTER_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E
    ,SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E


    /********************************/
    /* ipv6 key routing fields      */
    /********************************/
    ,SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0_E
    ,SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32_E
    ,SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64_E
    ,SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96_E

    ,SIP6_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E
}SIP6_FDB_AU_MSG_TABLE_FIELDS_ENT;


/*
 * Typedef: enum SIP6_IPCL_ACTION_TABLE_FIELDS_ENT
 *
 * Description:
 *      enumeration to hold fields of IPCL Action table.
 *
 */
/* the fields of the (IPCL) action table sip6 (same order as sip5_20,reordered from sip5 !!! ) */
typedef enum {
     SIP6_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_COMMAND_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_VIDX_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_PBR_MODE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_MAC_SA_27_0_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_28_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_RESERVED_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_SET_SST_ID_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_SST_ID_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_COUNTER_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_COUNTER_INDEX_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_VID0_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_VID1_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_UP1_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_RESERVED_1_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_EN_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_TRIGGER_INTERRUPT_EN_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_RESERVED_3_BITS_E
    ,SIP6_IPCL_ACTION_TABLE_FIELDS_EXACT_MATCH_OVER_PCL_ENABLE_E

    ,SIP6_IPCL_ACTION_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP6_IPCL_ACTION_TABLE_FIELDS_ENT;


/* macro to set value to field of (FDB) AU message format in buffer */
#define SIP6_FDB_AU_MSG_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_SIP6_DIFF_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)


/* macro to get field value of (FDB) AU message format in buffer */
#define SIP6_FDB_AU_MSG_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_SIP6_DIFF_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)


/**
* @enum SIP6_FDB_FDB_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of FDB table in SIP6 devices.(FDB unit)
 * NOTE: the FDB entry reduced to 115 bits from 140 in SIP5 devices.
*/
typedef enum {
     SIP6_FDB_FDB_TABLE_FIELDS_VALID_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_AGE_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_FID_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_DIP_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SIP_26_0_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_7_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_0_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SIP_30_27_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_VIDX_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SIP_31_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_MULTIPLE_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SA_CMD_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E

    /**********************************/
    /* ipv4/6 and fcoe routing fields */
    /**********************************/
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_EVIDX_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_POINTER_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E

    /********************************/
    /* ipv6 key routing fields      */
    /********************************/
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64_E
    ,SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96_E

    ,SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E

}SIP6_FDB_FDB_TABLE_FIELDS_ENT;

/* macro to set value to field of (FDB) FDB entry format in buffer */
#define SIP6_FDB_ENTRY_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E].fieldsInfoPtr,                  \
        _fieldName,                                                 \
        _value)


/* macro to get field value of (FDB) FDB entry format in buffer */
#define SIP6_FDB_ENTRY_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E].fieldsInfoPtr,                  \
        _fieldName,                                                 \
        _value)


/* macro to set value to MAC ADDR field of (FDB) FDB entry format in buffer */
#define SIP6_FDB_ENTRY_FIELD_MAC_ADDR_SET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E].fieldsInfoPtr,                  \
        SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E,                   \
        _macAddrBytes)

/* macro to get value to MAC ADDR field of (FDB) FDB entry format in buffer */
#define SIP6_FDB_ENTRY_FIELD_MAC_ADDR_GET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E].fieldsInfoPtr,                  \
        SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E,                   \
        _macAddrBytes)

/* SA command : SIP6 different SA command macro from the DA command */
/* we force the application on 'drop' to stick to global value of 'SA drop command'*/
#define SIP6_CONVERT_SW_SA_CMD_TO_HW_VAL_MAC(_devNum,_val, _cmd)        \
    switch (_cmd)                                                       \
    {                                                                   \
        case CPSS_MAC_TABLE_FRWRD_E:                                    \
            _val = 0;                                                   \
            break;                                                      \
        case CPSS_MAC_TABLE_DROP_E:                                     \
            if((PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.sip6FdbSaDropCommand) != (_cmd))                             \
            {                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,             \
                "[%s] = CPSS_MAC_TABLE_DROP_E in FDB entry not supported since global mode is CPSS_MAC_TABLE_SOFT_DROP_E",#_cmd); \
            }                                                           \
            _val = 1;/*same HW value for soft and hard drop*/           \
            break;                                                      \
        case CPSS_MAC_TABLE_SOFT_DROP_E:                                \
            if((PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.sip6FdbSaDropCommand) != (_cmd))                             \
            {                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,             \
                "[%s] = CPSS_MAC_TABLE_SOFT_DROP_E in FDB entry not supported since global mode is CPSS_MAC_TABLE_DROP_E",#_cmd); \
            }                                                           \
            _val = 1;/*same HW value for soft and hard drop*/           \
            break;                                                      \
        default:                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(_cmd)); \
    }

/* DA command : SIP6 different SA command macro from the DA command */
/* we force the application on 'drop' to stick to global value of 'DA drop command'*/
#define SIP6_CONVERT_SW_DA_CMD_TO_HW_VAL_MAC(_devNum,_val, _cmd)        \
    switch (_cmd)                                                       \
    {                                                                   \
        case CPSS_MAC_TABLE_FRWRD_E:                                    \
            _val = 0;                                                   \
            break;                                                      \
        case CPSS_MAC_TABLE_MIRROR_TO_CPU_E:                            \
            _val = 1;                                                   \
            break;                                                      \
        case CPSS_MAC_TABLE_CNTL_E:                                     \
            _val = 2;                                                   \
            break;                                                      \
        case CPSS_MAC_TABLE_DROP_E:                                     \
            if((PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.sip6FdbDaDropCommand) != (_cmd))                             \
            {                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,             \
                "[%s] = CPSS_MAC_TABLE_DROP_E in FDB entry not supported since global mode is CPSS_MAC_TABLE_SOFT_DROP_E",#_cmd); \
            }                                                           \
            _val = 3;/*same HW value for soft and hard drop*/           \
            break;                                                      \
        case CPSS_MAC_TABLE_SOFT_DROP_E:                                \
            if((PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.sip6FdbDaDropCommand) != (_cmd))                             \
            {                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,             \
                "[%s] = CPSS_MAC_TABLE_SOFT_DROP_E in FDB entry not supported since global mode is CPSS_MAC_TABLE_DROP_E",#_cmd); \
            }                                                           \
            _val = 3;/*same HW value for soft and hard drop*/           \
            break;                                                      \
        default:                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(_cmd)); \
    }

/* SA command : SIP6 different SA command macro from the DA command */
/* we return the application the 'drop' according to global value of 'SA drop command'*/
#define SIP6_CONVERT_HW_SA_CMD_TO_SW_VAL_MAC(_devNum, _cmd, _val)           \
    switch (_val)                                                       \
    {                                                                   \
        default:/*can't get here on 1 bit field*/                       \
        case 0:                                                         \
            _cmd = CPSS_MAC_TABLE_FRWRD_E;                              \
            break;                                                      \
        case 1:                                                         \
            _cmd = (PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.sip6FdbSaDropCommand); \
            break;                                                      \
    }

/* DA command : SIP6 different SA command macro from the DA command */
/* we return the application the 'drop' according to global value of 'DA drop command'*/
#define SIP6_CONVERT_HW_DA_CMD_TO_SW_VAL_MAC(_devNum, _cmd, _val)           \
    switch (_val)                                                       \
    {                                                                   \
        default:/*can't get here on 2 bits field*/                       \
        case 0:                                                         \
            _cmd = CPSS_MAC_TABLE_FRWRD_E;                              \
            break;                                                      \
        case 1:                                                         \
            _cmd = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;                      \
            break;                                                      \
        case 2:                                                         \
            _cmd = CPSS_MAC_TABLE_CNTL_E;                               \
            break;                                                      \
        case 3:                                                         \
            _cmd = (PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.sip6FdbDaDropCommand); \
            break;                                                      \
    }


/**
* @enum SIP6_EXACT_MATCH_TABLE_FIELDS_ENT
 *
*  @brief enumeration to hold fields of Exact Match table.
*       (Exact Match unit)
*/
typedef enum {

     SIP6_EXACT_MATCH_TABLE_FIELDS_VALID_E
    ,SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE_E
    ,SIP6_EXACT_MATCH_TABLE_FIELDS_AGE_E
    ,SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_SIZE_E
    ,SIP6_EXACT_MATCH_TABLE_FIELDS_LOOKUP_NUM_E
    ,SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_E
    ,SIP5_EXACT_MATCH_TABLE_FIELDS_REDUCE_ACTION_E
    ,SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_ONLY_E

    ,SIP6_EXACT_MATCH_TABLE_FIELDS___LAST_VALUE___E/* used for array size */

}SIP6_EXACT_MATCH_TABLE_FIELDS_ENT;

/* macro to set value to field of Exact Match entry format in buffer */
#define SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)                                 \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                                         \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E].fieldsInfoPtr,    \
        _fieldName,                                                                                             \
        _value)

/* macro to get field value of Exact Match entry format in buffer */
#define SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)                                 \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                                         \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E].fieldsInfoPtr,    \
        _fieldName,                                                                                             \
        _value)

/* macro to set value to key field of Exact Match entry format in buffer */
#define SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_SET_MAC(_dev,_hwDataArr,_keyPatternBytes)                            \
    KEY_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                                       \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E].fieldsInfoPtr,  \
        SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_E,                                                                  \
        _keyPatternBytes)

/* macro to get value of key field of Exact Match entry format in buffer */
#define SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_GET_MAC(_dev,_hwDataArr,_keyPatternBytes)                           \
    KEY_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                                       \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E].fieldsInfoPtr,  \
        SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_E,                                                                  \
        _keyPatternBytes)

/* macro to set value to Reduced Action field of Exact Match entry format in buffer */
#define SIP6_EXACT_MATCH_ENTRY_FIELD_REDUCED_ACTION_SET_MAC(_dev,_hwDataArr,_reduceActionBytes,_expandedActionIndex)\
    REDUCED_ACTION_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                              \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E].fieldsInfoPtr,    \
        SIP5_EXACT_MATCH_TABLE_FIELDS_REDUCE_ACTION_E,                                                          \
        _reduceActionBytes,_expandedActionIndex)

/* macro to get value from Reduced Action field of Exact Match entry format in buffer */
#define SIP6_EXACT_MATCH_ENTRY_FIELD_REDUCED_ACTION_GET_MAC(_dev,_hwDataArr,_reduceActionBytes,_expandedActionIndex)\
    REDUCED_ACTION_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                              \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E].fieldsInfoPtr,    \
        SIP5_EXACT_MATCH_TABLE_FIELDS_REDUCE_ACTION_E,                                                          \
        _reduceActionBytes,_expandedActionIndex)

/* macro to set value to key only field of Exact Match entry format in buffer */
#define SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_ONLY_SET_MAC(_dev,_hwDataArr,_keyPatternBytes)                         \
    KEY_ONLY_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                                    \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E].fieldsInfoPtr,    \
        SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_ONLY_E,                                                               \
        _keyPatternBytes)

/* macro to get value from key only field of Exact Match entry format in buffer */
#define SIP6_EXACT_MATCH_ENTRY_FIELD_KEY_ONLY_GET_MAC(_dev,_hwDataArr,_keyPatternBytes)                         \
    KEY_ONLY_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                                    \
        PRV_CPSS_DXCH_PP_MAC(_dev)->tableFormatInfo[PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E].fieldsInfoPtr,    \
        SIP6_EXACT_MATCH_TABLE_FIELDS_KEY_ONLY_E,                                                               \
        _keyPatternBytes)

/**
* @internal initTablesDbSip6 function
* @endinternal
*
* @brief   init the Sip6 tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
GT_STATUS initTablesDbSip6
(
    IN GT_U8                devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwTablesSip6h */

