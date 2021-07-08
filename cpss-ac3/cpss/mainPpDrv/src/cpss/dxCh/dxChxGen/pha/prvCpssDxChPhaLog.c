/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPhaLog.c
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
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPhaLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

char * prvCpssLogEnum_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT[]  =
{
    "CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_INC_OVERSIZE_E",
    "CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_OVERSIZE_E",
    "CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_DEC_VIOLATION_E",
    "CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_UNDERSIZE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT[]  =
{
    "CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E",
    "CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_A_E",
    "CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_B_E",
    "CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_C_E",
    "CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_Z_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT[]  =
{
    "CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E",
    "CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_A_E",
    "CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_B_E",
    "CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_C_E",
    "CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_Z_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_PHA_THREAD_TYPE_ENT[]  =
{
    "CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE___LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT[]  =
{
    "CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E",
    "CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E",
    "CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, statisticalProcessingFactor);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, busyStallMode, CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, stallDropCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, notNeeded);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamIngressSwitchIpv4, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamIngressSwitchIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamTransitSwitchIpv4, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamTransitSwitchIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_Trace_Type);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Maximum_Length);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Flags);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Hop_Lim);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Type1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_HDR_len1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Reserved1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Next_Protocol1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Type2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_HDR_len2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Reserved2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Next_Protocol2);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_Trace_Type);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Maximum_Length);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Flags);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Hop_Lim);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Type1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_HDR_len1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Reserved1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Next_Protocol1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Type2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_HDR_len2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Reserved2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Next_Protocol2);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, *paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, *paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PHA_THREAD_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_THREAD_TYPE_ENT);
}


/********* API fields DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr = {
     "commonInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_infoType = {
     "infoType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_infoType = {
     "infoType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr = {
     "extInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_THREAD_INFO_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_THREAD_TYPE_ENT_extType = {
     "extType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_packetCommand = {
     "packetCommand", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PACKET_CMD_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_packetOrderChangeEnable = {
     "packetOrderChangeEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_phaThreadId = {
     "phaThreadId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr = {
     "commonInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT_PTR_violationTypePtr = {
     "violationTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr = {
     "infoTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr = {
     "infoTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr = {
     "extInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_THREAD_INFO_UNT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_THREAD_TYPE_ENT_PTR_extTypePtr = {
     "extTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_packetCommandPtr = {
     "packetCommandPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PACKET_CMD_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_capturedThreadIdPtr = {
     "capturedThreadIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_phaThreadIdPtr = {
     "phaThreadIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaHeaderModificationViolationInfoSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_dropCode,
    &DX_IN_CPSS_PACKET_CMD_ENT_packetCommand
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_packetOrderChangeEnable
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaSourcePortEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_infoType,
    &DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaTargetPortEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_infoType,
    &DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaSourcePortEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr,
    &DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaTargetPortEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr,
    &DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaPortThreadIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_GT_U32_phaThreadId
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaPortThreadIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_GT_U32_PTR_phaThreadIdPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaStatisticalProcessingCounterThreadIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_phaThreadId
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaThreadIdEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_phaThreadId,
    &DX_IN_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr,
    &DX_IN_CPSS_DXCH_PHA_THREAD_TYPE_ENT_extType,
    &DX_IN_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaThreadIdEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_phaThreadId,
    &DX_OUT_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr,
    &DX_OUT_CPSS_DXCH_PHA_THREAD_TYPE_ENT_PTR_extTypePtr,
    &DX_OUT_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaHeaderModificationViolationInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_dropCodePtr,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_packetCommandPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaHeaderModificationViolationCapturedGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_capturedThreadIdPtr,
    &DX_OUT_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT_PTR_violationTypePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaStatisticalProcessingCounterThreadIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_phaThreadIdPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChPhaStatisticalProcessingCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U64_PTR_counterPtr
};


/********* lib API DB *********/

static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChPhaLogLibDb[] = {
    {"cpssDxChPhaInit", 2, cpssDxChPhaInit_PARAMS, NULL},
    {"cpssDxChPhaThreadIdEntrySet", 5, cpssDxChPhaThreadIdEntrySet_PARAMS, NULL},
    {"cpssDxChPhaThreadIdEntryGet", 5, cpssDxChPhaThreadIdEntryGet_PARAMS, NULL},
    {"cpssDxChPhaPortThreadIdSet", 4, cpssDxChPhaPortThreadIdSet_PARAMS, NULL},
    {"cpssDxChPhaPortThreadIdGet", 4, cpssDxChPhaPortThreadIdGet_PARAMS, NULL},
    {"cpssDxChPhaSourcePortEntrySet", 4, cpssDxChPhaSourcePortEntrySet_PARAMS, NULL},
    {"cpssDxChPhaSourcePortEntryGet", 4, cpssDxChPhaSourcePortEntryGet_PARAMS, NULL},
    {"cpssDxChPhaTargetPortEntrySet", 4, cpssDxChPhaTargetPortEntrySet_PARAMS, NULL},
    {"cpssDxChPhaTargetPortEntryGet", 4, cpssDxChPhaTargetPortEntryGet_PARAMS, NULL},
    {"cpssDxChPhaHeaderModificationViolationInfoSet", 3, cpssDxChPhaHeaderModificationViolationInfoSet_PARAMS, NULL},
    {"cpssDxChPhaHeaderModificationViolationInfoGet", 3, cpssDxChPhaHeaderModificationViolationInfoGet_PARAMS, NULL},
    {"cpssDxChPhaHeaderModificationViolationCapturedGet", 3, cpssDxChPhaHeaderModificationViolationCapturedGet_PARAMS, NULL},
    {"cpssDxChPhaStatisticalProcessingCounterThreadIdSet", 2, cpssDxChPhaStatisticalProcessingCounterThreadIdSet_PARAMS, NULL},
    {"cpssDxChPhaStatisticalProcessingCounterThreadIdGet", 2, cpssDxChPhaStatisticalProcessingCounterThreadIdGet_PARAMS, NULL},
    {"cpssDxChPhaStatisticalProcessingCounterGet", 2, cpssDxChPhaStatisticalProcessingCounterGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_PHA(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChPhaLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChPhaLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

