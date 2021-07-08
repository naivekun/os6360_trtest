/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenPpTypesLog.c
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
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/log/prvCpssGenPpTypesLog.h>


/********* enums *********/

char * prvCpssLogEnum_CPSS_COMPARE_OPERATOR_ENT[]  =
{
    "CPSS_COMPARE_OPERATOR_INVALID_E",
    "CPSS_COMPARE_OPERATOR_LTE",
    "CPSS_COMPARE_OPERATOR_GTE",
    "CPSS_COMPARE_OPERATOR_NEQ"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_COMPARE_OPERATOR_ENT);
char * prvCpssLogEnum_CPSS_DROP_MODE_TYPE_ENT[]  =
{
    "CPSS_DROP_MODE_SOFT_E",
    "CPSS_DROP_MODE_HARD_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DROP_MODE_TYPE_ENT);
PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_L4_PROTOCOL_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_L4_PROTOCOL_TCP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_L4_PROTOCOL_UDP_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_L4_PROTOCOL_ENT);
char * prvCpssLogEnum_CPSS_L4_PROTOCOL_PORT_TYPE_ENT[]  =
{
    "CPSS_L4_PROTOCOL_PORT_SRC_E",
    "CPSS_L4_PROTOCOL_PORT_DST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_L4_PROTOCOL_PORT_TYPE_ENT);
char * prvCpssLogEnum_CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT[]  =
{
    "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E",
    "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
char * prvCpssLogEnum_CPSS_PACKET_CMD_ENT[]  =
{
    "CPSS_PACKET_CMD_FORWARD_E",
    "CPSS_PACKET_CMD_MIRROR_TO_CPU_E",
    "CPSS_PACKET_CMD_TRAP_TO_CPU_E",
    "CPSS_PACKET_CMD_DROP_HARD_E",
    "CPSS_PACKET_CMD_DROP_SOFT_E",
    "CPSS_PACKET_CMD_ROUTE_E",
    "CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E",
    "CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E",
    "CPSS_PACKET_CMD_BRIDGE_E",
    "CPSS_PACKET_CMD_NONE_E",
    "CPSS_PACKET_CMD_LOOPBACK_E",
    "CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PACKET_CMD_ENT);
char * prvCpssLogEnum_CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT[]  =
{
    "CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E",
    "CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E",
    "CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E",
    "CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT);
char * prvCpssLogEnum_CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT[]  =
{
    "CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E",
    "CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E",
    "CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_COMPARE_OPERATOR_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_COMPARE_OPERATOR_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_COMPARE_OPERATOR_ENT);
}
void prvCpssLogParamFunc_CPSS_COMPARE_OPERATOR_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_COMPARE_OPERATOR_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_COMPARE_OPERATOR_ENT);
}
void prvCpssLogParamFunc_CPSS_DROP_MODE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DROP_MODE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DROP_MODE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DROP_MODE_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DROP_MODE_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DROP_MODE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_L4_PROTOCOL_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_L4_PROTOCOL_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_L4_PROTOCOL_ENT);
}
void prvCpssLogParamFunc_CPSS_L4_PROTOCOL_PORT_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_L4_PROTOCOL_PORT_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_L4_PROTOCOL_PORT_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_L4_PROTOCOL_PORT_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_L4_PROTOCOL_PORT_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_L4_PROTOCOL_PORT_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
}
void prvCpssLogParamFunc_CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
}
void prvCpssLogParamFunc_CPSS_PACKET_CMD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PACKET_CMD_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PACKET_CMD_ENT);
}
void prvCpssLogParamFunc_CPSS_PACKET_CMD_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PACKET_CMD_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PACKET_CMD_ENT);
}

