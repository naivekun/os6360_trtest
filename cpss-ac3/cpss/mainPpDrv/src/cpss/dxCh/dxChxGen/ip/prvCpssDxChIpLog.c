/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpLog.c
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
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpNat.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIpLog.h>
#include <cpss/dxCh/dxChxGen/lpm/private/prvCpssDxChLpmLog.h>
#include <cpss/dxCh/dxChxGen/tunnel/private/prvCpssDxChTunnelLog.h>
#include <cpss/generic/cpssHwInit/private/prvCpssGenCpssHwInitLog.h>
#include <cpss/generic/ip/private/prvCpssGenIpLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/port/private/prvCpssGenPortLog.h>


/********* enums *********/

char * prvCpssLogEnum_CPSS_DXCH_IP_BRG_EXCP_CMD_ENT[]  =
{
    "CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_UC_IPV6_TRAP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_UC_IPV4_SOFT_DROP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_UC_IPV6_SOFT_DROP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_MC_IPV4_TRAP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_MC_IPV6_TRAP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_MC_IPV4_SOFT_DROP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_MC_IPV6_SOFT_DROP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E",
    "CPSS_DXCH_IP_BRG_ARP_SOFT_DROP_EXCP_CMD_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_BRG_EXCP_CMD_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT[]  =
{
    "CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E",
    "CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E",
    "CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_BRG_SERVICE_ENT[]  =
{
    "CPSS_DXCH_IP_HEADER_CHECK_BRG_SERVICE_E",
    "CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E",
    "CPSS_DXCH_IP_SIP_SA_CHECK_BRG_SERVICE_E",
    "CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E",
    "CPSS_DXCH_IP_BRG_SERVICE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_BRG_SERVICE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_CNT_SET_MODE_ENT[]  =
{
    "CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E",
    "CPSS_DXCH_IP_CNT_SET_ROUTE_ENTRY_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_CNT_SET_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_DROP_CNT_MODE_ENT[]  =
{
    "CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_IP_HEADER_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_DIP_DA_MISMATCH_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_SIP_SA_MISMATCH_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_ILLEGAL_ADDRESS_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_UC_RPF_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_MC_RPF_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_TTL_HOP_LIMIT_EXCEEDED_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_MTU_EXCEEDED_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_OPTION_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_IPV6_SCOPE_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_UC_SIP_FILTER_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_NH_CMD_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_ACCESS_MATRIX_MODE_E",
    "CPSS_DXCH_IP_DROP_CNT_SIP_ALL_ZEROS_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_DROP_CNT_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_EXCEPTION_TYPE_ENT[]  =
{
    "CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E",
    "CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E",
    "CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E",
    "CPSS_DXCH_IP_EXCP_MC_ILLEGAL_ADDRESS_E",
    "CPSS_DXCH_IP_EXCP_UC_DIP_DA_MISMATCH_E",
    "CPSS_DXCH_IP_EXCP_MC_DIP_DA_MISMATCH_E",
    "CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E",
    "CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_E",
    "CPSS_DXCH_IP_EXCP_UC_ALL_ZERO_SIP_E",
    "CPSS_DXCH_IP_EXCP_MC_ALL_ZERO_SIP_E",
    "CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E",
    "CPSS_DXCH_IP_EXCP_MC_OPTION_HOP_BY_HOP_E",
    "CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E",
    "CPSS_DXCH_IP_EXCP_MC_NON_HOP_BY_HOP_EXT_E",
    "CPSS_DXCH_IP_EXCP_UC_TTL_EXCEED_E",
    "CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E",
    "CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E",
    "CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_NON_DF_E",
    "CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_NON_DF_E",
    "CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_DF_E",
    "CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_DF_E",
    "CPSS_DXCH_IP_EXCEPTION_TYPE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_EXCEPTION_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_HEADER_ERROR_ENT[]  =
{
    "CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT",
    "CPSS_DXCH_IP_HEADER_ERROR_VERSION_ENT",
    "CPSS_DXCH_IP_HEADER_ERROR_LENGTH_ENT",
    "CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_HEADER_ERROR_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT[]  =
{
    "CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E",
    "CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT[]  =
{
    "CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E",
    "CPSS_DXCH_IP_MT_TC_QUEUE_SP_SCHED_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT[]  =
{
    "CPSS_DXCH_IP_MT_UC_SCHED_MTU_2K_E",
    "CPSS_DXCH_IP_MT_UC_SCHED_MTU_8K_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_URPF_MODE_ENT[]  =
{
    "CPSS_DXCH_IP_URPF_DISABLE_MODE_E",
    "CPSS_DXCH_IP_URPF_VLAN_MODE_E",
    "CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E",
    "CPSS_DXCH_IP_URPF_LOOSE_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_URPF_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT[]  =
{
    "CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E",
    "CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT);
char * prvCpssLogEnum_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT[]  =
{
    "CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E",
    "CPSS_DXCH_URPF_LOOSE_MODE_TYPE_1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT);
char * prvCpssLogEnum_CPSS_IP_NAT_TYPE_ENT[]  =
{
    "CPSS_IP_NAT_TYPE_NAT44_E",
    "CPSS_IP_NAT_TYPE_NAT66_E",
    "CPSS_IP_NAT_TYPE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_IP_NAT_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_CPU_CODE_INDEX_ENT[]  =
{
    "CPSS_DXCH_IP_CPU_CODE_IDX_0_E",
    "CPSS_DXCH_IP_CPU_CODE_IDX_1_E",
    "CPSS_DXCH_IP_CPU_CODE_IDX_2_E",
    "CPSS_DXCH_IP_CPU_CODE_IDX_3_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_CPU_CODE_INDEX_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_ENT[]  =
{
    "CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E",
    "CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_MULTICAST_RPF_FAIL_COMMAND_MODE_ENT[]  =
{
    "CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E",
    "CPSS_DXCH_IP_MULTICAST_MLL_RPF_FAIL_COMMAND_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_MULTICAST_RPF_FAIL_COMMAND_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_PORT_TRUNK_CNT_MODE_ENT[]  =
{
    "CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E",
    "CPSS_DXCH_IP_PORT_CNT_MODE_E",
    "CPSS_DXCH_IP_TRUNK_CNT_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_PORT_TRUNK_CNT_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT[]  =
{
    "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E",
    "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E",
    "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
    "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_UC_ROUTE_ENTRY_TYPE_ENT[]  =
{
    "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E",
    "CPSS_DXCH_IP_UC_ECMP_RPF_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_UC_ROUTE_ENTRY_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_VLAN_CNT_MODE_ENT[]  =
{
    "CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E",
    "CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_VLAN_CNT_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT[]  =
{
    "CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E",
    "CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E",
    "CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E",
    "CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_IPV4_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPV4_PREFIX_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vrId);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, ipAddr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isMcSource);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mcGroupIndexRow);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mcGroupIndexColumn);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPV6_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPV6_PREFIX_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vrId);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, ipAddr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isMcSource);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mcGroupIndexRow);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_COUNTER_SET_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_COUNTER_SET_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, inUcPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, inMcPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, inUcNonRoutedExcpPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, inUcNonRoutedNonExcpPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, inMcNonRoutedExcpPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, inMcNonRoutedNonExcpPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, inUcTrappedMirrorPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, inMcTrappedMirrorPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mcRfpFailPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, outUcRoutedPkts);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_ECMP_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, randomEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfPaths);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, routeEntryBaseIndex);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, multiPathMode, CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_LTT_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, routeType, CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfPaths);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, routeEntryBaseIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ucRPFCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sipSaCheckMismatchEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipv6MCGroupScopeLevel, CPSS_IPV6_PREFIX_SCOPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, priority, CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, cmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, cpuCodeIdx, CPSS_DXCH_IP_CPU_CODE_INDEX_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, appSpecificCpuCodeEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ttlHopLimitDecEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ingressMirror);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressMirrorToAnalyzerIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, qosProfileMarkingEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qosProfileIndex);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, qosPrecedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyUp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyDscp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, countSet, CPSS_IP_CNT_SET_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, multicastRPFCheckEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, multicastIngressVlanCheck, CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, multicastRPFVlan);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, multicastRPFRoutingSharedTreeIndex);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, multicastRPFFailCommandMode, CPSS_DXCH_IP_MULTICAST_RPF_FAIL_COMMAND_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, RPFFailCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, scopeCheckingEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, siteId, CPSS_IP_SITE_ID_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mtuProfileIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, internalMLLPointer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, externalMLLPointer);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_MLL_PAIR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_MLL_PAIR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, firstMllNode, CPSS_DXCH_IP_MLL_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, secondMllNode, CPSS_DXCH_IP_MLL_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextPointer);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_MLL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_MLL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mllRPFFailCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isTunnelStart);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, nextHopInterface, CPSS_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopVlanId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopTunnelPointer);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tunnelStartPassengerType, CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ttlHopLimitThreshold);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, excludeSrcVlan);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, last);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_NAT44_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_NAT44_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, modifyDip);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, newDip);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, modifySip);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, newSip);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, modifyTcpUdpDstPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, newTcpUdpDstPort);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, modifyTcpUdpSrcPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, newTcpUdpSrcPort);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_NAT66_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_NAT66_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyCommand, CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, address);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, prefixSize);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, vlanArray, 8, GT_U16);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, cmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, cpuCodeIdx, CPSS_DXCH_IP_CPU_CODE_INDEX_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, appSpecificCpuCodeEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, unicastPacketSipFilterEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ttlHopLimitDecEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ingressMirror);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressMirrorToAnalyzerIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, qosProfileMarkingEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qosProfileIndex);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, qosPrecedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyUp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyDscp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, countSet, CPSS_IP_CNT_SET_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, trapMirrorArpBcEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sipAccessLevel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dipAccessLevel);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ICMPRedirectEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, scopeCheckingEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, siteId, CPSS_IP_SITE_ID_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mtuProfileIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isTunnelStart);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isNat);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopVlanId);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, nextHopInterface, CPSS_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopARPPointer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopTunnelPointer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopNatPointer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopVlanId1);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_IPV4_PREFIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPV4_PREFIX_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPV4_PREFIX_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPV6_PREFIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPV6_PREFIX_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPV6_PREFIX_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_BRG_EXCP_CMD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_BRG_EXCP_CMD_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_BRG_EXCP_CMD_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_BRG_SERVICE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_BRG_SERVICE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_BRG_SERVICE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_CNT_SET_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_CNT_SET_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_CNT_SET_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_CNT_SET_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_CNT_SET_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_IP_CNT_SET_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_COUNTER_SET_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_COUNTER_SET_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_COUNTER_SET_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_DROP_CNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_DROP_CNT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_DROP_CNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_DROP_CNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_DROP_CNT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_IP_DROP_CNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_ECMP_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_EXCEPTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_EXCEPTION_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_EXCEPTION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_HEADER_ERROR_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_HEADER_ERROR_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_HEADER_ERROR_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_LTT_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_MLL_PAIR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_MLL_PAIR_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_MLL_PAIR_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_NAT_ENTRY_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_URPF_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_URPF_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IP_URPF_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_URPF_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_URPF_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_IP_URPF_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_IP_NAT_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_IP_NAT_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_IP_NAT_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_IP_NAT_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_IP_NAT_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_IP_NAT_TYPE_ENT);
}


/********* API fields DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC_PTR_routeEntriesArray = {
     "routeEntriesArray", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPV4_PREFIX_STC_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPV4_PREFIX_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPV4_PREFIX_STC_PTR_prefixPtr = {
     "prefixPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPV4_PREFIX_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPV6_PREFIX_STC_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPV6_PREFIX_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPV6_PREFIX_STC_PTR_prefixPtr = {
     "prefixPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPV6_PREFIX_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_BRG_EXCP_CMD_ENT_bridgeExceptionCmd = {
     "bridgeExceptionCmd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_BRG_EXCP_CMD_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT_enableDisableMode = {
     "enableDisableMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_BRG_SERVICE_ENT_bridgeService = {
     "bridgeService", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_BRG_SERVICE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_CNT_SET_MODE_ENT_cntSetMode = {
     "cntSetMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_CNT_SET_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceCfgPtr = {
     "interfaceCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceModeCfgPtr = {
     "interfaceModeCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_COUNTER_SET_STC_PTR_countersPtr = {
     "countersPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_COUNTER_SET_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_DROP_CNT_MODE_ENT_dropCntMode = {
     "dropCntMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_DROP_CNT_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR_ecmpEntryPtr = {
     "ecmpEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_ECMP_ENTRY_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_EXCEPTION_TYPE_ENT_exceptionType = {
     "exceptionType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_EXCEPTION_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_HEADER_ERROR_ENT_ipHeaderErrorType = {
     "ipHeaderErrorType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_HEADER_ERROR_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_lttEntryPtr = {
     "lttEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LTT_ENTRY_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR_routeEntryPtr = {
     "routeEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT_mllEntryPart = {
     "mllEntryPart", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MLL_PAIR_STC_PTR_mllPairEntryPtr = {
     "mllPairEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_MLL_PAIR_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT_schedulingMode = {
     "schedulingMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT_schedMtu = {
     "schedMtu", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_NAT_ENTRY_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC_PTR_routeEntriesArray = {
     "routeEntriesArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_URPF_MODE_ENT_uRpfMode = {
     "uRpfMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_URPF_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT_tokenBucketIntervalUpdateRatio = {
     "tokenBucketIntervalUpdateRatio", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT_looseModeType = {
     "looseModeType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IPV6_MLL_SELECTION_RULE_ENT_mllSelectionRule = {
     "mllSelectionRule", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IPV6_MLL_SELECTION_RULE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScope = {
     "addressScope", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IPV6_PREFIX_SCOPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeDest = {
     "addressScopeDest", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IPV6_PREFIX_SCOPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeSrc = {
     "addressScopeSrc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IPV6_PREFIX_SCOPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_CNT_SET_ENT_cntSet = {
     "cntSet", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IP_CNT_SET_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_NAT_TYPE_ENT_natType = {
     "natType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IP_NAT_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcEnable = {
     "ucMcEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IP_UNICAST_MULTICAST_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcSet = {
     "ucMcSet", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IP_UNICAST_MULTICAST_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_SA_LSB_MODE_ENT_saLsbMode = {
     "saLsbMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_MAC_SA_LSB_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_arpBcMode = {
     "arpBcMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PACKET_CMD_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_scopeCommand = {
     "scopeCommand", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PACKET_CMD_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_tokenBucketMode = {
     "tokenBucketMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_TX_DROP_SHAPER_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_borderCrossed = {
     "borderCrossed", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableRouterTrigger = {
     "enableRouterTrigger", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableRouting = {
     "enableRouting", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableService = {
     "enableService", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_lastBit = {
     "lastBit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_mask = {
     "mask", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_mllBridgeEnable = {
     "mllBridgeEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_multiTargetRateShaperEnable = {
     "multiTargetRateShaperEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_refreshEnable = {
     "refreshEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ucSPEnable = {
     "ucSPEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_arpMacAddrPtr = {
     "arpMacAddrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_macSaAddrPtr = {
     "macSaAddrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_prefixPtr = {
     "prefixPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPV6ADDR)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_sourceIdMask = {
     "sourceIdMask", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_baseRouteEntryIndex = {
     "baseRouteEntryIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ctrlMultiTargetTCQueue = {
     "ctrlMultiTargetTCQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dropPkts = {
     "dropPkts", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ecmpEntryIndex = {
     "ecmpEntryIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_failRpfMultiTargetTCQueue = {
     "failRpfMultiTargetTCQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_indirectIndex = {
     "indirectIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lttTtiColumn = {
     "lttTtiColumn", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lttTtiRow = {
     "lttTtiRow", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxBucketSize = {
     "maxBucketSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mcWeight = {
     "mcWeight", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mllOutMCPkts = {
     "mllOutMCPkts", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mtu = {
     "mtu", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mtuProfileIndex = {
     "mtuProfileIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_multiTargetTCQueue = {
     "multiTargetTCQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_multiTargetTcQueue = {
     "multiTargetTcQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_natIndex = {
     "natIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_nextHopIndex = {
     "nextHopIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfRouteEntries = {
     "numOfRouteEntries", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_prefixScopeIndex = {
     "prefixScopeIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_qosProfile = {
     "qosProfile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueWeight = {
     "queueWeight", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routeEntryIndex = {
     "routeEntryIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routeEntryOffset = {
     "routeEntryOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerArpIndex = {
     "routerArpIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerBridgedExceptionPkts = {
     "routerBridgedExceptionPkts", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerMacSaIndex = {
     "routerMacSaIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerNextHopTableAgeBitsEntry = {
     "routerNextHopTableAgeBitsEntry", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerNextHopTableAgeBitsEntryIndex = {
     "routerNextHopTableAgeBitsEntryIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerTtiTcamColumn = {
     "routerTtiTcamColumn", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerTtiTcamRow = {
     "routerTtiTcamRow", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_saMac = {
     "saMac", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_seed = {
     "seed", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tokenBucketIntervalSlowUpdateRatio = {
     "tokenBucketIntervalSlowUpdateRatio", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tokenBucketUpdateInterval = {
     "tokenBucketUpdateInterval", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ucWeight = {
     "ucWeight", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_PTR_mrstBmpPtr = {
     "mrstBmpPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_saMac = {
     "saMac", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPV4_PREFIX_STC_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPV4_PREFIX_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPV4_PREFIX_STC_PTR_prefixPtr = {
     "prefixPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPV4_PREFIX_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPV6_PREFIX_STC_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPV6_PREFIX_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPV6_PREFIX_STC_PTR_prefixPtr = {
     "prefixPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPV6_PREFIX_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_CNT_SET_MODE_ENT_PTR_cntSetModePtr = {
     "cntSetModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_CNT_SET_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceModeCfgPtr = {
     "interfaceModeCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_COUNTER_SET_STC_PTR_countersPtr = {
     "countersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_COUNTER_SET_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_DROP_CNT_MODE_ENT_PTR_dropCntModePtr = {
     "dropCntModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_DROP_CNT_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR_ecmpEntryPtr = {
     "ecmpEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_ECMP_ENTRY_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_lttEntryPtr = {
     "lttEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LTT_ENTRY_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR_routeEntryPtr = {
     "routeEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_MLL_PAIR_STC_PTR_mllPairEntryPtr = {
     "mllPairEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_MLL_PAIR_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT_PTR_schedulingModePtr = {
     "schedulingModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT_PTR_schedMtuPtr = {
     "schedMtuPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_NAT_ENTRY_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_URPF_MODE_ENT_PTR_uRpfModePtr = {
     "uRpfModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_URPF_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT_PTR_tokenBucketIntervalUpdateRatioPtr = {
     "tokenBucketIntervalUpdateRatioPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT_PTR_looseModeTypePtr = {
     "looseModeTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IPV6_MLL_SELECTION_RULE_ENT_PTR_mllSelectionRulePtr = {
     "mllSelectionRulePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_IPV6_MLL_SELECTION_RULE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IPV6_PREFIX_SCOPE_ENT_PTR_addressScopePtr = {
     "addressScopePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_IPV6_PREFIX_SCOPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IP_NAT_TYPE_ENT_PTR_natTypePtr = {
     "natTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_IP_NAT_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_SA_LSB_MODE_ENT_PTR_saLsbModePtr = {
     "saLsbModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_MAC_SA_LSB_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_arpBcModePtr = {
     "arpBcModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PACKET_CMD_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_exceptionCmdPtr = {
     "exceptionCmdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PACKET_CMD_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_scopeCommandPtr = {
     "scopeCommandPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PACKET_CMD_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_PTR_tokenBucketModePtr = {
     "tokenBucketModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_TX_DROP_SHAPER_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableRouterTriggerPtr = {
     "enableRouterTriggerPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableRoutingPtr = {
     "enableRoutingPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableServicePtr = {
     "enableServicePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_maskPtr = {
     "maskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_mllBridgeEnablePtr = {
     "mllBridgeEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_multiTargetRateShaperEnablePtr = {
     "multiTargetRateShaperEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_refreshEnablePtr = {
     "refreshEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ucSPEnablePtr = {
     "ucSPEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_arpMacAddrPtr = {
     "arpMacAddrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_macSaAddrPtr = {
     "macSaAddrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_prefixPtr = {
     "prefixPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPV6ADDR)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_sourceIdMaskPtr = {
     "sourceIdMaskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ctrlMultiTargetTCQueuePtr = {
     "ctrlMultiTargetTCQueuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dropPktsPtr = {
     "dropPktsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_failRpfMultiTargetTCQueuePtr = {
     "failRpfMultiTargetTCQueuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxBucketSizePtr = {
     "maxBucketSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mcWeightPtr = {
     "mcWeightPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mtuPtr = {
     "mtuPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_multiTargetTCQueuePtr = {
     "multiTargetTCQueuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_natDropPktsPtr = {
     "natDropPktsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nextHopIndexPtr = {
     "nextHopIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueWeightPtr = {
     "queueWeightPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_routeEntryOffsetPtr = {
     "routeEntryOffsetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_routerBridgedExceptionPktsPtr = {
     "routerBridgedExceptionPktsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_routerMacSaIndexPtr = {
     "routerMacSaIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_routerNextHopTableAgeBitsEntryPtr = {
     "routerNextHopTableAgeBitsEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_saMacPtr = {
     "saMacPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_seedPtr = {
     "seedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_skipCounterPtr = {
     "skipCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tokenBucketIntervalSlowUpdateRatioPtr = {
     "tokenBucketIntervalSlowUpdateRatioPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tokenBucketUpdateIntervalPtr = {
     "tokenBucketUpdateIntervalPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ucWeightPtr = {
     "ucWeightPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_mrstBmpPtr = {
     "mrstBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_saMacPtr = {
     "saMacPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpSpecialRouterTriggerEnable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_BRG_EXCP_CMD_ENT_bridgeExceptionCmd,
    &DX_IN_GT_BOOL_enableRouterTrigger
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpSpecialRouterTriggerEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_BRG_EXCP_CMD_ENT_bridgeExceptionCmd,
    &DX_OUT_GT_BOOL_PTR_enableRouterTriggerPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpBridgeServiceEnable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_BRG_SERVICE_ENT_bridgeService,
    &DX_IN_CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT_enableDisableMode,
    &DX_IN_GT_BOOL_enableService
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpBridgeServiceEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_BRG_SERVICE_ENT_bridgeService,
    &DX_IN_CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT_enableDisableMode,
    &DX_OUT_GT_BOOL_PTR_enableServicePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpSetDropCntMode_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_DROP_CNT_MODE_ENT_dropCntMode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpExceptionCommandSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_EXCEPTION_TYPE_ENT_exceptionType,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpExceptionCommandGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_EXCEPTION_TYPE_ENT_exceptionType,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_exceptionCmdPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpHeaderErrorMaskSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_HEADER_ERROR_ENT_ipHeaderErrorType,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_IN_CPSS_UNICAST_MULTICAST_ENT_prefixType,
    &DX_IN_GT_BOOL_mask
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpHeaderErrorMaskGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IP_HEADER_ERROR_ENT_ipHeaderErrorType,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_IN_CPSS_UNICAST_MULTICAST_ENT_prefixType,
    &DX_OUT_GT_BOOL_PTR_maskPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpUrpfLooseModeTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT_looseModeType
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6UcScopeCommandSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeSrc,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeDest,
    &DX_IN_GT_BOOL_borderCrossed,
    &DX_IN_CPSS_PACKET_CMD_ENT_scopeCommand
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6McScopeCommandSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeSrc,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeDest,
    &DX_IN_GT_BOOL_borderCrossed,
    &DX_IN_CPSS_PACKET_CMD_ENT_scopeCommand,
    &DX_IN_CPSS_IPV6_MLL_SELECTION_RULE_ENT_mllSelectionRule
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6UcScopeCommandGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeSrc,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeDest,
    &DX_IN_GT_BOOL_borderCrossed,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_scopeCommandPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6McScopeCommandGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeSrc,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeDest,
    &DX_IN_GT_BOOL_borderCrossed,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_scopeCommandPtr,
    &DX_OUT_CPSS_IPV6_MLL_SELECTION_RULE_ENT_PTR_mllSelectionRulePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpCntSetModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_CNT_SET_ENT_cntSet,
    &DX_IN_CPSS_DXCH_IP_CNT_SET_MODE_ENT_cntSetMode,
    &DX_IN_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceModeCfgPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_CNT_SET_ENT_cntSet,
    &DX_IN_CPSS_DXCH_IP_COUNTER_SET_STC_PTR_countersPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpCntSetModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_CNT_SET_ENT_cntSet,
    &DX_OUT_CPSS_DXCH_IP_CNT_SET_MODE_ENT_PTR_cntSetModePtr,
    &DX_OUT_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceModeCfgPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpCntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_CNT_SET_ENT_cntSet,
    &DX_OUT_CPSS_DXCH_IP_COUNTER_SET_STC_PTR_countersPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpFdbRoutePrefixLenSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_IN_GT_U32_prefixLen
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpFdbRoutePrefixLenGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_OUT_GT_U32_PTR_prefixLenPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterSourceIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcSet,
    &DX_IN_GT_U32_sourceId,
    &DX_IN_GT_U32_HEX_sourceIdMask
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterSourceIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcSet,
    &DX_OUT_GT_U32_PTR_sourceIdPtr,
    &DX_OUT_GT_U32_HEX_PTR_sourceIdMaskPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpArpBcModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PACKET_CMD_ENT_arpBcMode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllMultiTargetShaperTokenBucketModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_tokenBucketMode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRoutingEnable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_enableRouting
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllBridgeEnable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_mllBridgeEnable
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMultiTargetRateShaperSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_multiTargetRateShaperEnable,
    &DX_IN_GT_U32_windowSize
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpUcRouteAgingModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_refreshEnable
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMultiTargetUcSchedModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_ucSPEnable,
    &DX_IN_GT_U32_ucWeight,
    &DX_IN_GT_U32_mcWeight,
    &DX_IN_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT_schedMtu
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6AddrPrefixScopeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_IPV6ADDR_PTR_prefixPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScope,
    &DX_IN_GT_U32_prefixScopeIndex
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortRouterMacSaLsbModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_MAC_SA_LSB_MODE_ENT_saLsbMode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterPortMacSaLsbSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U8_saMac
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortRouterMacSaLsbModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_MAC_SA_LSB_MODE_ENT_PTR_saLsbModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterPortMacSaLsbGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U8_PTR_saMacPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_IP_CNT_SET_ENT_cntSet,
    &DX_IN_CPSS_DXCH_IP_COUNTER_SET_STC_PTR_countersPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupCntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_IP_CNT_SET_ENT_cntSet,
    &DX_OUT_CPSS_DXCH_IP_COUNTER_SET_STC_PTR_countersPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupRouterSourceIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcSet,
    &DX_IN_GT_U32_sourceId,
    &DX_IN_GT_U32_HEX_sourceIdMask
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupRouterSourceIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcSet,
    &DX_OUT_GT_U32_PTR_sourceIdPtr,
    &DX_OUT_GT_U32_HEX_PTR_sourceIdMaskPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupMultiTargetRateShaperSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_BOOL_multiTargetRateShaperEnable,
    &DX_IN_GT_U32_windowSize
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupDropCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_dropPkts
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupMllCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_mllCntSet,
    &DX_IN_GT_U32_mllOutMCPkts
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_routerBridgedExceptionPkts
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupMultiTargetRateShaperGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_BOOL_PTR_multiTargetRateShaperEnablePtr,
    &DX_OUT_GT_U32_PTR_windowSizePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupDropCntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_dropPktsPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_routerBridgedExceptionPktsPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllPortGroupSkippedEntriesCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_skipCounterPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortRoutingEnable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcEnable,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_IN_GT_BOOL_enableRouting
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpPortRoutingEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcEnable,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_OUT_GT_BOOL_PTR_enableRoutingPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterPortGlobalMacSaIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_routerMacSaIndex
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterPortGlobalMacSaIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_routerMacSaIndexPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpUcRpfModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vid,
    &DX_IN_CPSS_DXCH_IP_URPF_MODE_ENT_uRpfMode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpUcRpfModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vid,
    &DX_OUT_CPSS_DXCH_IP_URPF_MODE_ENT_PTR_uRpfModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterVlanMacSaLsbSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vlan,
    &DX_IN_GT_U32_saMac
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterVlanMacSaLsbGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vlan,
    &DX_OUT_GT_U32_PTR_saMacPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpVlanMrstBitmapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vlanId,
    &DX_IN_GT_U64_PTR_mrstBmpPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpVlanMrstBitmapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vlanId,
    &DX_OUT_GT_U64_PTR_mrstBmpPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpFcoeUcRpfAccessLevelSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_accessLevel
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpUcRouteEntriesWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_baseRouteEntryIndex,
    &DX_IN_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC_PTR_routeEntriesArray,
    &DX_IN_GT_U32_numOfRouteEntries
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpUcRouteEntriesRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_baseRouteEntryIndex,
    &DX_INOUT_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC_PTR_routeEntriesArray,
    &DX_IN_GT_U32_numOfRouteEntries
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_ctrlMultiTargetTCQueue,
    &DX_IN_GT_U32_failRpfMultiTargetTCQueue
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMultiTargetQueueFullDropCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_dropPkts
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpEcmpEntryWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_ecmpEntryIndex,
    &DX_IN_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR_ecmpEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpEcmpEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_ecmpEntryIndex,
    &DX_OUT_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR_ecmpEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpEcmpIndirectNextHopEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_indirectIndex,
    &DX_IN_GT_U32_nextHopIndex
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpEcmpIndirectNextHopEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_indirectIndex,
    &DX_OUT_GT_U32_PTR_nextHopIndexPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpLttWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lttTtiRow,
    &DX_IN_GT_U32_lttTtiColumn,
    &DX_IN_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_lttEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpLttRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lttTtiRow,
    &DX_IN_GT_U32_lttTtiColumn,
    &DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_lttEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllMultiTargetShaperConfigurationSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_maxBucketSize,
    &DX_INOUT_GT_U32_PTR_maxRatePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpSetMllCntInterface_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllCntSet,
    &DX_IN_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceCfgPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllCntSet,
    &DX_IN_GT_U32_mllOutMCPkts
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMLLPairRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllPairEntryIndex,
    &DX_IN_CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT_mllPairReadForm,
    &DX_OUT_CPSS_DXCH_IP_MLL_PAIR_STC_PTR_mllPairEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMLLPairWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllPairEntryIndex,
    &DX_IN_CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT_mllPairWriteForm,
    &DX_IN_CPSS_DXCH_IP_MLL_PAIR_STC_PTR_mllPairEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMLLLastBitWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllPairEntryIndex,
    &DX_IN_GT_BOOL_lastBit,
    &DX_IN_CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT_mllEntryPart
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllMultiTargetShaperMtuSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mtu
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMtuProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mtuProfileIndex,
    &DX_IN_GT_U32_mtu
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMtuProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mtuProfileIndex,
    &DX_OUT_GT_U32_PTR_mtuPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMultiTargetTCQueueSchedModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_multiTargetTcQueue,
    &DX_IN_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT_schedulingMode,
    &DX_IN_GT_U32_queueWeight
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMultiTargetTCQueueSchedModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_multiTargetTcQueue,
    &DX_OUT_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT_PTR_schedulingModePtr,
    &DX_OUT_GT_U32_PTR_queueWeightPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpNatEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_natIndex,
    &DX_IN_CPSS_IP_NAT_TYPE_ENT_natType,
    &DX_IN_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR_entryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpNatEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_natIndex,
    &DX_OUT_CPSS_IP_NAT_TYPE_ENT_PTR_natTypePtr,
    &DX_OUT_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR_entryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpQosProfileToMultiTargetTCQueueMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_qosProfile,
    &DX_IN_GT_U32_multiTargetTCQueue
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpQosProfileToRouteEntryMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_qosProfile,
    &DX_IN_GT_U32_routeEntryOffset
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpQosProfileToMultiTargetTCQueueMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_qosProfile,
    &DX_OUT_GT_U32_PTR_multiTargetTCQueuePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpQosProfileToRouteEntryMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_qosProfile,
    &DX_OUT_GT_U32_PTR_routeEntryOffsetPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMcRouteEntriesWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routeEntryIndex,
    &DX_IN_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR_routeEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMcRouteEntriesRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routeEntryIndex,
    &DX_OUT_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR_routeEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterArpAddrWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerArpIndex,
    &DX_IN_GT_ETHERADDR_PTR_arpMacAddrPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterArpAddrRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerArpIndex,
    &DX_OUT_GT_ETHERADDR_PTR_arpMacAddrPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterBridgedPacketsExceptionCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerBridgedExceptionPkts
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterGlobalMacSaSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerMacSaIndex,
    &DX_IN_GT_ETHERADDR_PTR_macSaAddrPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterGlobalMacSaGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerMacSaIndex,
    &DX_OUT_GT_ETHERADDR_PTR_macSaAddrPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterNextHopTableAgeBitsEntryWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerNextHopTableAgeBitsEntryIndex,
    &DX_IN_GT_U32_routerNextHopTableAgeBitsEntry
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterNextHopTableAgeBitsEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerNextHopTableAgeBitsEntryIndex,
    &DX_OUT_GT_U32_PTR_routerNextHopTableAgeBitsEntryPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6PrefixInvalidate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerTtiTcamRow
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6PrefixSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerTtiTcamRow,
    &DX_IN_CPSS_DXCH_IPV6_PREFIX_STC_PTR_prefixPtr,
    &DX_IN_CPSS_DXCH_IPV6_PREFIX_STC_PTR_maskPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv4PrefixInvalidate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerTtiTcamRow,
    &DX_IN_GT_U32_routerTtiTcamColumn
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv4PrefixSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerTtiTcamRow,
    &DX_IN_GT_U32_routerTtiTcamColumn,
    &DX_IN_CPSS_DXCH_IPV4_PREFIX_STC_PTR_prefixPtr,
    &DX_IN_CPSS_DXCH_IPV4_PREFIX_STC_PTR_maskPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv4PrefixGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerTtiTcamRow,
    &DX_IN_GT_U32_routerTtiTcamColumn,
    &DX_OUT_GT_BOOL_PTR_validPtr,
    &DX_OUT_CPSS_DXCH_IPV4_PREFIX_STC_PTR_prefixPtr,
    &DX_OUT_CPSS_DXCH_IPV4_PREFIX_STC_PTR_maskPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6PrefixGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_routerTtiTcamRow,
    &DX_OUT_GT_BOOL_PTR_validPtr,
    &DX_OUT_CPSS_DXCH_IPV6_PREFIX_STC_PTR_prefixPtr,
    &DX_OUT_CPSS_DXCH_IPV6_PREFIX_STC_PTR_maskPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpEcmpHashSeedValueSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_seed
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpEcmpHashNumBitsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_startBit,
    &DX_IN_GT_U32_numOfBits
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpTcDpToMultiTargetTcQueueMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tc,
    &DX_IN_CPSS_DP_LEVEL_ENT_dp,
    &DX_IN_GT_U32_multiTargetTCQueue
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpTcDpToMultiTargetTcQueueMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tc,
    &DX_IN_CPSS_DP_LEVEL_ENT_dp,
    &DX_OUT_GT_U32_PTR_multiTargetTCQueuePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tokenBucketIntervalSlowUpdateRatio,
    &DX_IN_GT_U32_tokenBucketUpdateInterval,
    &DX_IN_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT_tokenBucketIntervalUpdateRatio
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpGetDropCntMode_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_IP_DROP_CNT_MODE_ENT_PTR_dropCntModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpUrpfLooseModeTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT_PTR_looseModeTypePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpArpBcModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_arpBcModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllMultiTargetShaperTokenBucketModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_PTR_tokenBucketModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRoutingEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_enableRoutingPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllBridgeEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_mllBridgeEnablePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMultiTargetRateShaperGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_multiTargetRateShaperEnablePtr,
    &DX_OUT_GT_U32_PTR_windowSizePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpUcRouteAgingModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_refreshEnablePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMultiTargetUcSchedModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_ucSPEnablePtr,
    &DX_OUT_GT_U32_PTR_ucWeightPtr,
    &DX_OUT_GT_U32_PTR_mcWeightPtr,
    &DX_OUT_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT_PTR_schedMtuPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpv6AddrPrefixScopeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_IPV6ADDR_PTR_prefixPtr,
    &DX_OUT_GT_U32_PTR_prefixLenPtr,
    &DX_OUT_CPSS_IPV6_PREFIX_SCOPE_ENT_PTR_addressScopePtr,
    &DX_IN_GT_U32_prefixScopeIndex
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpFcoeUcRpfAccessLevelGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_accessLevelPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_ctrlMultiTargetTCQueuePtr,
    &DX_OUT_GT_U32_PTR_failRpfMultiTargetTCQueuePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMultiTargetQueueFullDropCntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_dropPktsPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllMultiTargetShaperConfigurationGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_maxBucketSizePtr,
    &DX_OUT_GT_U32_PTR_maxRatePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllMultiTargetShaperMtuGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_mtuPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpNatDroppedPacketsCntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_natDropPktsPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpRouterBridgedPacketsExceptionCntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_routerBridgedExceptionPktsPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpEcmpHashSeedValueGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_seedPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllSkippedEntriesCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_skipCounterPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpEcmpHashNumBitsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_startBitPtr,
    &DX_OUT_GT_U32_PTR_numOfBitsPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_tokenBucketIntervalSlowUpdateRatioPtr,
    &DX_OUT_GT_U32_PTR_tokenBucketUpdateIntervalPtr,
    &DX_OUT_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT_PTR_tokenBucketIntervalUpdateRatioPtr
};


/********* lib API DB *********/

extern void cpssDxChIpUcRouteEntriesWrite_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpUcRouteEntriesWrite_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpNatEntrySet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpNatEntryGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChIpLogLibDb[] = {
    {"cpssDxChIpv4PrefixSet", 5, cpssDxChIpv4PrefixSet_PARAMS, NULL},
    {"cpssDxChIpv4PrefixGet", 6, cpssDxChIpv4PrefixGet_PARAMS, NULL},
    {"cpssDxChIpv4PrefixInvalidate", 3, cpssDxChIpv4PrefixInvalidate_PARAMS, NULL},
    {"cpssDxChIpv6PrefixSet", 4, cpssDxChIpv6PrefixSet_PARAMS, NULL},
    {"cpssDxChIpv6PrefixGet", 5, cpssDxChIpv6PrefixGet_PARAMS, NULL},
    {"cpssDxChIpv6PrefixInvalidate", 2, cpssDxChIpv6PrefixInvalidate_PARAMS, NULL},
    {"cpssDxChIpLttWrite", 4, cpssDxChIpLttWrite_PARAMS, NULL},
    {"cpssDxChIpLttRead", 4, cpssDxChIpLttRead_PARAMS, NULL},
    {"cpssDxChIpUcRouteEntriesWrite", 4, cpssDxChIpUcRouteEntriesWrite_PARAMS, cpssDxChIpUcRouteEntriesWrite_preLogic},
    {"cpssDxChIpUcRouteEntriesRead", 4, cpssDxChIpUcRouteEntriesRead_PARAMS, cpssDxChIpUcRouteEntriesWrite_preLogic},
    {"cpssDxChIpMcRouteEntriesWrite", 3, cpssDxChIpMcRouteEntriesWrite_PARAMS, NULL},
    {"cpssDxChIpMcRouteEntriesRead", 3, cpssDxChIpMcRouteEntriesRead_PARAMS, NULL},
    {"cpssDxChIpRouterNextHopTableAgeBitsEntryWrite", 3, cpssDxChIpRouterNextHopTableAgeBitsEntryWrite_PARAMS, NULL},
    {"cpssDxChIpRouterNextHopTableAgeBitsEntryRead", 3, cpssDxChIpRouterNextHopTableAgeBitsEntryRead_PARAMS, NULL},
    {"cpssDxChIpMLLPairWrite", 4, cpssDxChIpMLLPairWrite_PARAMS, NULL},
    {"cpssDxChIpMLLPairRead", 4, cpssDxChIpMLLPairRead_PARAMS, NULL},
    {"cpssDxChIpMLLLastBitWrite", 4, cpssDxChIpMLLLastBitWrite_PARAMS, NULL},
    {"cpssDxChIpRouterArpAddrWrite", 3, cpssDxChIpRouterArpAddrWrite_PARAMS, NULL},
    {"cpssDxChIpRouterArpAddrRead", 3, cpssDxChIpRouterArpAddrRead_PARAMS, NULL},
    {"cpssDxChIpEcmpEntryWrite", 3, cpssDxChIpEcmpEntryWrite_PARAMS, NULL},
    {"cpssDxChIpEcmpEntryRead", 3, cpssDxChIpEcmpEntryRead_PARAMS, NULL},
    {"cpssDxChIpEcmpHashNumBitsSet", 3, cpssDxChIpEcmpHashNumBitsSet_PARAMS, NULL},
    {"cpssDxChIpEcmpHashNumBitsGet", 3, cpssDxChIpEcmpHashNumBitsGet_PARAMS, NULL},
    {"cpssDxChIpEcmpHashSeedValueSet", 2, cpssDxChIpEcmpHashSeedValueSet_PARAMS, NULL},
    {"cpssDxChIpEcmpHashSeedValueGet", 2, cpssDxChIpEcmpHashSeedValueGet_PARAMS, NULL},
    {"cpssDxChIpEcmpIndirectNextHopEntrySet", 3, cpssDxChIpEcmpIndirectNextHopEntrySet_PARAMS, NULL},
    {"cpssDxChIpEcmpIndirectNextHopEntryGet", 3, cpssDxChIpEcmpIndirectNextHopEntryGet_PARAMS, NULL},
    {"cpssDxChIpSpecialRouterTriggerEnable", 3, cpssDxChIpSpecialRouterTriggerEnable_PARAMS, NULL},
    {"cpssDxChIpSpecialRouterTriggerEnableGet", 3, cpssDxChIpSpecialRouterTriggerEnableGet_PARAMS, NULL},
    {"cpssDxChIpExceptionCommandSet", 4, cpssDxChIpExceptionCommandSet_PARAMS, NULL},
    {"cpssDxChIpExceptionCommandGet", 4, cpssDxChIpExceptionCommandGet_PARAMS, NULL},
    {"cpssDxChIpHeaderErrorMaskSet", 5, cpssDxChIpHeaderErrorMaskSet_PARAMS, NULL},
    {"cpssDxChIpHeaderErrorMaskGet", 5, cpssDxChIpHeaderErrorMaskGet_PARAMS, NULL},
    {"cpssDxChIpUcRouteAgingModeSet", 2, cpssDxChIpUcRouteAgingModeSet_PARAMS, NULL},
    {"cpssDxChIpUcRouteAgingModeGet", 2, cpssDxChIpUcRouteAgingModeGet_PARAMS, NULL},
    {"cpssDxChIpRouterSourceIdSet", 4, cpssDxChIpRouterSourceIdSet_PARAMS, NULL},
    {"cpssDxChIpRouterSourceIdGet", 4, cpssDxChIpRouterSourceIdGet_PARAMS, NULL},
    {"cpssDxChIpPortGroupRouterSourceIdSet", 5, cpssDxChIpPortGroupRouterSourceIdSet_PARAMS, NULL},
    {"cpssDxChIpPortGroupRouterSourceIdGet", 5, cpssDxChIpPortGroupRouterSourceIdGet_PARAMS, NULL},
    {"cpssDxChIpRouterSourceIdOverrideEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpRouterSourceIdOverrideEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet", 3, cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet_PARAMS, NULL},
    {"cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet", 3, cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet_PARAMS, NULL},
    {"cpssDxChIpQosProfileToMultiTargetTCQueueMapSet", 3, cpssDxChIpQosProfileToMultiTargetTCQueueMapSet_PARAMS, NULL},
    {"cpssDxChIpQosProfileToMultiTargetTCQueueMapGet", 3, cpssDxChIpQosProfileToMultiTargetTCQueueMapGet_PARAMS, NULL},
    {"cpssDxChIpTcDpToMultiTargetTcQueueMapSet", 4, cpssDxChIpTcDpToMultiTargetTcQueueMapSet_PARAMS, NULL},
    {"cpssDxChIpTcDpToMultiTargetTcQueueMapGet", 4, cpssDxChIpTcDpToMultiTargetTcQueueMapGet_PARAMS, NULL},
    {"cpssDxChIpMultiTargetQueueFullDropCntGet", 2, cpssDxChIpMultiTargetQueueFullDropCntGet_PARAMS, NULL},
    {"cpssDxChIpMultiTargetQueueFullDropCntSet", 2, cpssDxChIpMultiTargetQueueFullDropCntSet_PARAMS, NULL},
    {"cpssDxChIpMultiTargetTCQueueSchedModeSet", 4, cpssDxChIpMultiTargetTCQueueSchedModeSet_PARAMS, NULL},
    {"cpssDxChIpMultiTargetTCQueueSchedModeGet", 4, cpssDxChIpMultiTargetTCQueueSchedModeGet_PARAMS, NULL},
    {"cpssDxChIpBridgeServiceEnable", 4, cpssDxChIpBridgeServiceEnable_PARAMS, NULL},
    {"cpssDxChIpBridgeServiceEnableGet", 4, cpssDxChIpBridgeServiceEnableGet_PARAMS, NULL},
    {"cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet", 3, cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet_PARAMS, NULL},
    {"cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet", 3, cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet_PARAMS, NULL},
    {"cpssDxChIpRouterBridgedPacketsExceptionCntGet", 2, cpssDxChIpRouterBridgedPacketsExceptionCntGet_PARAMS, NULL},
    {"cpssDxChIpRouterBridgedPacketsExceptionCntSet", 2, cpssDxChIpRouterBridgedPacketsExceptionCntSet_PARAMS, NULL},
    {"cpssDxChIpMllBridgeEnable", 2, cpssDxChIpMllBridgeEnable_PARAMS, NULL},
    {"cpssDxChIpMllBridgeEnableGet", 2, cpssDxChIpMllBridgeEnableGet_PARAMS, NULL},
    {"cpssDxChIpMultiTargetRateShaperSet", 3, cpssDxChIpMultiTargetRateShaperSet_PARAMS, NULL},
    {"cpssDxChIpMultiTargetRateShaperGet", 3, cpssDxChIpMultiTargetRateShaperGet_PARAMS, NULL},
    {"cpssDxChIpMultiTargetUcSchedModeSet", 5, cpssDxChIpMultiTargetUcSchedModeSet_PARAMS, NULL},
    {"cpssDxChIpMultiTargetUcSchedModeGet", 5, cpssDxChIpMultiTargetUcSchedModeGet_PARAMS, NULL},
    {"cpssDxChIpArpBcModeSet", 2, cpssDxChIpArpBcModeSet_PARAMS, NULL},
    {"cpssDxChIpArpBcModeGet", 2, cpssDxChIpArpBcModeGet_PARAMS, NULL},
    {"cpssDxChIpPortRoutingEnable", 5, cpssDxChIpPortRoutingEnable_PARAMS, NULL},
    {"cpssDxChIpPortRoutingEnableGet", 5, cpssDxChIpPortRoutingEnableGet_PARAMS, NULL},
    {"cpssDxChIpPortFcoeForwardingEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChIpPortFcoeForwardingEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpQosProfileToRouteEntryMapSet", 3, cpssDxChIpQosProfileToRouteEntryMapSet_PARAMS, NULL},
    {"cpssDxChIpQosProfileToRouteEntryMapGet", 3, cpssDxChIpQosProfileToRouteEntryMapGet_PARAMS, NULL},
    {"cpssDxChIpRoutingEnable", 2, cpssDxChIpRoutingEnable_PARAMS, NULL},
    {"cpssDxChIpRoutingEnableGet", 2, cpssDxChIpRoutingEnableGet_PARAMS, NULL},
    {"cpssDxChIpCntGet", 3, cpssDxChIpCntGet_PARAMS, NULL},
    {"cpssDxChIpCntSetModeSet", 4, cpssDxChIpCntSetModeSet_PARAMS, NULL},
    {"cpssDxChIpCntSetModeGet", 4, cpssDxChIpCntSetModeGet_PARAMS, NULL},
    {"cpssDxChIpCntSet", 3, cpssDxChIpCntSet_PARAMS, NULL},
    {"cpssDxChIpMllSkippedEntriesCountersGet", 2, cpssDxChIpMllSkippedEntriesCountersGet_PARAMS, NULL},
    {"cpssDxChIpMllPortGroupSkippedEntriesCountersGet", 3, cpssDxChIpMllPortGroupSkippedEntriesCountersGet_PARAMS, NULL},
    {"cpssDxChIpSetMllCntInterface", 3, cpssDxChIpSetMllCntInterface_PARAMS, NULL},
    {"cpssDxChIpMllCntGet", 3, prvCpssLogGenDevNumMllCntSetMllOutMCPktsPtr_PARAMS, NULL},
    {"cpssDxChIpMllCntSet", 3, cpssDxChIpMllCntSet_PARAMS, NULL},
    {"cpssDxChIpMllSilentDropCntGet", 2, prvCpssLogGenDevNumSilentDropPktsPtr_PARAMS, NULL},
    {"cpssDxChIpMllPortGroupSilentDropCntGet", 3, prvCpssLogGenDevNumPortGroupsBmpSilentDropPktsPtr_PARAMS, NULL},
    {"cpssDxChIpDropCntSet", 2, cpssDxChIpMultiTargetQueueFullDropCntSet_PARAMS, NULL},
    {"cpssDxChIpSetDropCntMode", 2, cpssDxChIpSetDropCntMode_PARAMS, NULL},
    {"cpssDxChIpGetDropCntMode", 2, cpssDxChIpGetDropCntMode_PARAMS, NULL},
    {"cpssDxChIpDropCntGet", 2, cpssDxChIpMultiTargetQueueFullDropCntGet_PARAMS, NULL},
    {"cpssDxChIpMtuProfileSet", 3, cpssDxChIpMtuProfileSet_PARAMS, NULL},
    {"cpssDxChIpMtuProfileGet", 3, cpssDxChIpMtuProfileGet_PARAMS, NULL},
    {"cpssDxChIpv6AddrPrefixScopeSet", 5, cpssDxChIpv6AddrPrefixScopeSet_PARAMS, NULL},
    {"cpssDxChIpv6AddrPrefixScopeGet", 5, cpssDxChIpv6AddrPrefixScopeGet_PARAMS, NULL},
    {"cpssDxChIpv6UcScopeCommandSet", 5, cpssDxChIpv6UcScopeCommandSet_PARAMS, NULL},
    {"cpssDxChIpv6UcScopeCommandGet", 5, cpssDxChIpv6UcScopeCommandGet_PARAMS, NULL},
    {"cpssDxChIpv6McScopeCommandSet", 6, cpssDxChIpv6McScopeCommandSet_PARAMS, NULL},
    {"cpssDxChIpv6McScopeCommandGet", 6, cpssDxChIpv6McScopeCommandGet_PARAMS, NULL},
    {"cpssDxChIpRouterMacSaBaseSet", 2, prvCpssLogGenDevNumMacPtr_PARAMS, NULL},
    {"cpssDxChIpRouterMacSaBaseGet", 2, prvCpssLogGenDevNumMacPtr2_PARAMS, NULL},
    {"cpssDxChIpPortRouterMacSaLsbModeSet", 3, cpssDxChIpPortRouterMacSaLsbModeSet_PARAMS, NULL},
    {"cpssDxChIpPortRouterMacSaLsbModeGet", 3, cpssDxChIpPortRouterMacSaLsbModeGet_PARAMS, NULL},
    {"cpssDxChIpRouterPortMacSaLsbSet", 3, cpssDxChIpRouterPortMacSaLsbSet_PARAMS, NULL},
    {"cpssDxChIpRouterPortMacSaLsbGet", 3, cpssDxChIpRouterPortMacSaLsbGet_PARAMS, NULL},
    {"cpssDxChIpRouterVlanMacSaLsbSet", 3, cpssDxChIpRouterVlanMacSaLsbSet_PARAMS, NULL},
    {"cpssDxChIpRouterVlanMacSaLsbGet", 3, cpssDxChIpRouterVlanMacSaLsbGet_PARAMS, NULL},
    {"cpssDxChIpRouterGlobalMacSaSet", 3, cpssDxChIpRouterGlobalMacSaSet_PARAMS, NULL},
    {"cpssDxChIpRouterGlobalMacSaGet", 3, cpssDxChIpRouterGlobalMacSaGet_PARAMS, NULL},
    {"cpssDxChIpRouterPortGlobalMacSaIndexSet", 3, cpssDxChIpRouterPortGlobalMacSaIndexSet_PARAMS, NULL},
    {"cpssDxChIpRouterPortGlobalMacSaIndexGet", 3, cpssDxChIpRouterPortGlobalMacSaIndexGet_PARAMS, NULL},
    {"cpssDxChIpRouterMacSaModifyEnable", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChIpRouterMacSaModifyEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChIpEcmpUcRpfCheckEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpEcmpUcRpfCheckEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpPortGroupCntSet", 4, cpssDxChIpPortGroupCntSet_PARAMS, NULL},
    {"cpssDxChIpPortGroupCntGet", 4, cpssDxChIpPortGroupCntGet_PARAMS, NULL},
    {"cpssDxChIpPortGroupDropCntSet", 3, cpssDxChIpPortGroupDropCntSet_PARAMS, NULL},
    {"cpssDxChIpPortGroupDropCntGet", 3, cpssDxChIpPortGroupDropCntGet_PARAMS, NULL},
    {"cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet", 3, cpssDxChIpPortGroupDropCntGet_PARAMS, NULL},
    {"cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet", 3, cpssDxChIpPortGroupDropCntSet_PARAMS, NULL},
    {"cpssDxChIpPortGroupMllCntGet", 4, prvCpssLogGenDevNumPortGroupsBmpMllCntSetMllOutMCPktsPtr_PARAMS, NULL},
    {"cpssDxChIpPortGroupMllCntSet", 4, cpssDxChIpPortGroupMllCntSet_PARAMS, NULL},
    {"cpssDxChIpUcRpfModeSet", 3, cpssDxChIpUcRpfModeSet_PARAMS, NULL},
    {"cpssDxChIpUcRpfModeGet", 3, cpssDxChIpUcRpfModeGet_PARAMS, NULL},
    {"cpssDxChIpUrpfLooseModeTypeSet", 2, cpssDxChIpUrpfLooseModeTypeSet_PARAMS, NULL},
    {"cpssDxChIpUrpfLooseModeTypeGet", 2, cpssDxChIpUrpfLooseModeTypeGet_PARAMS, NULL},
    {"cpssDxChIpPortSipSaEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChIpPortSipSaEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpPortGroupMultiTargetRateShaperSet", 4, cpssDxChIpPortGroupMultiTargetRateShaperSet_PARAMS, NULL},
    {"cpssDxChIpPortGroupMultiTargetRateShaperGet", 4, cpssDxChIpPortGroupMultiTargetRateShaperGet_PARAMS, NULL},
    {"cpssDxChIpUcRoutingVid1AssignEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpUcRoutingVid1AssignEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperBaselineSet", 2, prvCpssLogGenDevNumBaseline_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperBaselineGet", 2, prvCpssLogGenDevNumBaselinePtr_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperMtuSet", 2, cpssDxChIpMllMultiTargetShaperMtuSet_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperMtuGet", 2, cpssDxChIpMllMultiTargetShaperMtuGet_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperTokenBucketModeSet", 2, cpssDxChIpMllMultiTargetShaperTokenBucketModeSet_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperTokenBucketModeGet", 2, cpssDxChIpMllMultiTargetShaperTokenBucketModeGet_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperConfigurationSet", 3, cpssDxChIpMllMultiTargetShaperConfigurationSet_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperConfigurationGet", 3, cpssDxChIpMllMultiTargetShaperConfigurationGet_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet", 4, cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet_PARAMS, NULL},
    {"cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet", 4, cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet_PARAMS, NULL},
    {"cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpVlanMrstBitmapSet", 3, cpssDxChIpVlanMrstBitmapSet_PARAMS, NULL},
    {"cpssDxChIpVlanMrstBitmapGet", 3, cpssDxChIpVlanMrstBitmapGet_PARAMS, NULL},
    {"cpssDxChIpFcoeExceptionPacketCommandSet", 2, prvCpssLogGenDevNumCommand_PARAMS, NULL},
    {"cpssDxChIpFcoeExceptionPacketCommandGet", 2, prvCpssLogGenDevNumCommandPtr_PARAMS, NULL},
    {"cpssDxChIpFcoeExceptionCpuCodeSet", 2, prvCpssLogGenDevNumCpuCode_PARAMS, NULL},
    {"cpssDxChIpFcoeExceptionCpuCodeGet", 2, prvCpssLogGenDevNumCpuCodePtr_PARAMS, NULL},
    {"cpssDxChIpFcoeSoftDropRouterEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpFcoeSoftDropRouterEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpFcoeTrapRouterEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpFcoeTrapRouterEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedUrpfCheckEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedUrpfCheckEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedUrpfCheckCommandSet", 2, prvCpssLogGenDevNumCommand_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedUrpfCheckCommandGet", 2, prvCpssLogGenDevNumCommandPtr_PARAMS, NULL},
    {"cpssDxChIpFcoeUcRpfAccessLevelSet", 2, cpssDxChIpFcoeUcRpfAccessLevelSet_PARAMS, NULL},
    {"cpssDxChIpFcoeUcRpfAccessLevelGet", 2, cpssDxChIpFcoeUcRpfAccessLevelGet_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCommandSet", 2, prvCpssLogGenDevNumCommand_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCommandGet", 2, prvCpssLogGenDevNumCommandPtr_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedSidFilterEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedSidFilterEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpFdbRoutePrefixLenSet", 3, cpssDxChIpFdbRoutePrefixLenSet_PARAMS, NULL},
    {"cpssDxChIpFdbRoutePrefixLenGet", 3, cpssDxChIpFdbRoutePrefixLenGet_PARAMS, NULL},
    {"cpssDxChIpFdbUnicastRouteForPbrEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpFdbUnicastRouteForPbrEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpNatEntrySet", 4, cpssDxChIpNatEntrySet_PARAMS, cpssDxChIpNatEntrySet_preLogic},
    {"cpssDxChIpNatEntryGet", 4, cpssDxChIpNatEntryGet_PARAMS, cpssDxChIpNatEntryGet_preLogic},
    {"cpssDxChIpNatDroppedPacketsCntGet", 2, cpssDxChIpNatDroppedPacketsCntGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_IP(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChIpLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChIpLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

