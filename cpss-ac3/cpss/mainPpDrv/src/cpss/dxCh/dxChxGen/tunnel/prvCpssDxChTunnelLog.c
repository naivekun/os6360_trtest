/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTunnelLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/private/prvCpssDxChTunnelLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/tunnel/private/prvCpssGenTunnelLog.h>


/********* enums *********/

char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E",
    "CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E",
    "CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E",
    "CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_0_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_1_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_2_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_3_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_0_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E",
    "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E",
    "CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E",
    "CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_GENERIC_TYPE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_GENERIC_SHORT_TYPE_E",
    "CPSS_DXCH_TUNNEL_START_GENERIC_MEDIUM_TYPE_E",
    "CPSS_DXCH_TUNNEL_START_GENERIC_LONG_TYPE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_GENERIC_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E",
    "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E",
    "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E",
    "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E",
    "CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E",
    "CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E",
    "CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E",
    "CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_EPORT_E",
    "CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_PACKET_QOS_PROFILE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E",
    "CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT[]  =
{
    "CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E",
    "CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E",
    "CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E",
    "CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, templateDataSize, CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, templateDataBitsCfg, CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_MAX_SIZE_CNS, CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hashShiftLeftBitsNumber);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, udpSrcPortMode, CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, serviceIdCircularShiftSize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, macDaMode);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dipMode);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, controlWordIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_GENERIC_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TUNNEL_START_GENERIC_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, upMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tagEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, genericType, CPSS_DXCH_TUNNEL_START_GENERIC_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, retainCrc);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, data, 36, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tagEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, upMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dscpMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dscp);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dontFragmentFlag);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ttl);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, autoTunnel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, autoTunnelOffset);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, destIp);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, srcIp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfi);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, retainCRC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipHeaderProtocol, CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipProtocol);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, profileIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, greProtocolForEthernet);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, greFlagsAndVersion);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udpDstPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udpSrcPort);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_IPV6_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TUNNEL_START_IPV6_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tagEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, upMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dscpMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dscp);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ttl);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, destIp);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, srcIp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, retainCRC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipHeaderProtocol, CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipProtocol);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, profileIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, greProtocolForEthernet);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udpDstPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, greFlagsAndVersion);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udpSrcPort);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, flowLabelMode, CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tagEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, upMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, retainCrc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iSid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iUp);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, iUpMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iDp);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, iDpMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iTagReserved);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, iSidAssignMode, CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, bDaAssignMode, CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, bSaAssignMode, CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tagEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, upMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numLabels);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ttl);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ttlMode, CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, label1);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, exp1MarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, exp1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, label2);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, exp2MarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, exp2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, label3);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, exp3MarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, exp3);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, retainCRC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, setSBit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfi);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, controlWordEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, controlWordIndex);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mplsEthertypeSelect, CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pushEliAndElAfterLabel1);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pushEliAndElAfterLabel2);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pushEliAndElAfterLabel3);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, pwExpMarkMode, CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *valPtr, CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_TRILL_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TUNNEL_START_TRILL_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, upMarkMode, CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tagEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanId);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, nextHopMacDa);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, trillHeader, CPSS_DXCH_TUNNEL_START_TRILL_HEADER_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_TRILL_HEADER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TUNNEL_START_TRILL_HEADER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, version);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, mBit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, opLength);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hopCount);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, eRbid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, iRbid);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TUNNEL_START_CONFIG_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TUNNEL_START_CONFIG_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_CONFIG_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TUNNEL_START_CONFIG_UNT_PTR_configPtr = {
     "configPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TUNNEL_START_CONFIG_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT_tagMode = {
     "tagMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR_profileDataPtr = {
     "profileDataPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TUNNEL_TYPE_ENT_tunnelType = {
     "tunnelType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TUNNEL_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_additionToLength = {
     "additionToLength", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_label = {
     "label", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerArpTunnelStartLineIndex = {
     "routerArpTunnelStartLineIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tsExtension = {
     "tsExtension", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ttl = {
     "ttl", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vlanServiceId = {
     "vlanServiceId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_ttl = {
     "ttl", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TUNNEL_START_CONFIG_UNT_PTR_configPtr = {
     "configPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TUNNEL_START_CONFIG_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT_PTR_tagModePtr = {
     "tagModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR_profileDataPtr = {
     "profileDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TUNNEL_TYPE_ENT_PTR_tunnelTypePtr = {
     "tunnelTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TUNNEL_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_additionToLengthPtr = {
     "additionToLengthPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_labelPtr = {
     "labelPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tsExtensionPtr = {
     "tsExtensionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ttlPtr = {
     "ttlPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vlanServiceIdPtr = {
     "vlanServiceIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_ttlPtr = {
     "ttlPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartEcnModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT_mode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChEthernetOverMplsTunnelStartTagModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT_tagMode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartPortGroupGenProfileTableEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_profileIndex,
    &DX_IN_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR_profileDataPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartPortGroupGenProfileTableEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_profileIndex,
    &DX_OUT_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR_profileDataPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartPortGroupEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_routerArpTunnelStartLineIndex,
    &DX_IN_CPSS_TUNNEL_TYPE_ENT_tunnelType,
    &DX_IN_CPSS_DXCH_TUNNEL_START_CONFIG_UNT_PTR_configPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartPortGroupEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_routerArpTunnelStartLineIndex,
    &DX_OUT_CPSS_TUNNEL_TYPE_ENT_PTR_tunnelTypePtr,
    &DX_OUT_CPSS_DXCH_TUNNEL_START_CONFIG_UNT_PTR_configPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartMplsPwLabelExpSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_exp
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartMplsPwLabelSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_label
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartEntryExtensionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_tsExtension
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartMplsPwLabelTtlSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_ttl
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartMplsPwLabelExpGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_expPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartMplsPwLabelGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_labelPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartEntryExtensionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_tsExtensionPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartMplsPwLabelTtlGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_ttlPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartEgessVlanTableServiceIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vlanId,
    &DX_IN_GT_U32_vlanServiceId
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartEgessVlanTableServiceIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vlanId,
    &DX_OUT_GT_U32_PTR_vlanServiceIdPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_additionToLength
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartGenProfileTableEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIndex,
    &DX_IN_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR_profileDataPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartGenProfileTableEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIndex,
    &DX_OUT_CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC_PTR_profileDataPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerArpTunnelStartLineIndex,
    &DX_IN_CPSS_TUNNEL_TYPE_ENT_tunnelType,
    &DX_IN_CPSS_DXCH_TUNNEL_START_CONFIG_UNT_PTR_configPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerArpTunnelStartLineIndex,
    &DX_OUT_CPSS_TUNNEL_TYPE_ENT_PTR_tunnelTypePtr,
    &DX_OUT_CPSS_DXCH_TUNNEL_START_CONFIG_UNT_PTR_configPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartMplsFlowLabelTtlSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_ttl
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartEcnModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT_PTR_modePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChEthernetOverMplsTunnelStartTagModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT_PTR_tagModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_additionToLengthPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTunnelStartMplsFlowLabelTtlGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U8_PTR_ttlPtr
};


/********* lib API DB *********/

extern void cpssDxChTunnelStartEntrySet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTunnelStartEntryGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTunnelStartPortGroupEntrySet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTunnelStartPortGroupEntryGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChTunnelLogLibDb[] = {
    {"cpssDxChTunnelStartEntrySet", 4, cpssDxChTunnelStartEntrySet_PARAMS, cpssDxChTunnelStartEntrySet_preLogic},
    {"cpssDxChTunnelStartPortGroupGenProfileTableEntrySet", 4, cpssDxChTunnelStartPortGroupGenProfileTableEntrySet_PARAMS, NULL},
    {"cpssDxChTunnelStartGenProfileTableEntrySet", 3, cpssDxChTunnelStartGenProfileTableEntrySet_PARAMS, NULL},
    {"cpssDxChTunnelStartPortGroupGenProfileTableEntryGet", 4, cpssDxChTunnelStartPortGroupGenProfileTableEntryGet_PARAMS, NULL},
    {"cpssDxChTunnelStartGenProfileTableEntryGet", 3, cpssDxChTunnelStartGenProfileTableEntryGet_PARAMS, NULL},
    {"cpssDxChTunnelStartEntryGet", 4, cpssDxChTunnelStartEntryGet_PARAMS, cpssDxChTunnelStartEntryGet_preLogic},
    {"cpssDxChTunnelStartPortGroupEntrySet", 5, cpssDxChTunnelStartPortGroupEntrySet_PARAMS, cpssDxChTunnelStartPortGroupEntrySet_preLogic},
    {"cpssDxChTunnelStartPortGroupEntryGet", 5, cpssDxChTunnelStartPortGroupEntryGet_PARAMS, cpssDxChTunnelStartPortGroupEntryGet_preLogic},
    {"cpssDxChEthernetOverMplsTunnelStartTaggingSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChEthernetOverMplsTunnelStartTaggingGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChEthernetOverMplsTunnelStartTagModeSet", 2, cpssDxChEthernetOverMplsTunnelStartTagModeSet_PARAMS, NULL},
    {"cpssDxChEthernetOverMplsTunnelStartTagModeGet", 2, cpssDxChEthernetOverMplsTunnelStartTagModeGet_PARAMS, NULL},
    {"cpssDxChTunnelStartPassengerVlanTranslationEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTunnelStartPassengerVlanTranslationEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet", 3, prvCpssLogGenDevNumPortEnable_PARAMS, NULL},
    {"cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet", 3, prvCpssLogGenDevNumPortEnablePtr_PARAMS, NULL},
    {"cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet", 2, cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet_PARAMS, NULL},
    {"cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet", 2, cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwLabelPushEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwLabelPushEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwLabelSet", 3, cpssDxChTunnelStartMplsPwLabelSet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwLabelGet", 3, cpssDxChTunnelStartMplsPwLabelGet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsFlowLabelEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsFlowLabelEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsFlowLabelTtlSet", 2, cpssDxChTunnelStartMplsFlowLabelTtlSet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsFlowLabelTtlGet", 2, cpssDxChTunnelStartMplsFlowLabelTtlGet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwLabelExpSet", 3, cpssDxChTunnelStartMplsPwLabelExpSet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwLabelExpGet", 3, cpssDxChTunnelStartMplsPwLabelExpGet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwLabelTtlSet", 3, cpssDxChTunnelStartMplsPwLabelTtlSet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwLabelTtlGet", 3, cpssDxChTunnelStartMplsPwLabelTtlGet_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwControlWordSet", 3, prvCpssLogGenDevNumIndexValue_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwControlWordGet", 3, prvCpssLogGenDevNumIndexValuePtr_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwETreeEnableSet", 3, prvCpssLogGenDevNumIndexEnable_PARAMS, NULL},
    {"cpssDxChTunnelStartMplsPwETreeEnableGet", 3, prvCpssLogGenDevNumIndexEnablePtr_PARAMS, NULL},
    {"cpssDxChTunnelStartHeaderTpidSelectSet", 3, prvCpssLogGenDevNumPortNumTpidEntryIndex_PARAMS, NULL},
    {"cpssDxChTunnelStartHeaderTpidSelectGet", 3, prvCpssLogGenDevNumPortNumTpidEntryIndexPtr_PARAMS, NULL},
    {"cpssDxChTunnelStartEntryExtensionSet", 3, cpssDxChTunnelStartEntryExtensionSet_PARAMS, NULL},
    {"cpssDxChTunnelStartEntryExtensionGet", 3, cpssDxChTunnelStartEntryExtensionGet_PARAMS, NULL},
    {"cpssDxChTunnelStartEgessVlanTableServiceIdSet", 3, cpssDxChTunnelStartEgessVlanTableServiceIdSet_PARAMS, NULL},
    {"cpssDxChTunnelStartEgessVlanTableServiceIdGet", 3, cpssDxChTunnelStartEgessVlanTableServiceIdGet_PARAMS, NULL},
    {"cpssDxChTunnelStartEcnModeSet", 2, cpssDxChTunnelStartEcnModeSet_PARAMS, NULL},
    {"cpssDxChTunnelStartEcnModeGet", 2, cpssDxChTunnelStartEcnModeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_TUNNEL(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChTunnelLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChTunnelLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

