/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenPortLog.c
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
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/generic/port/private/prvCpssGenPortLog.h>


/********* enums *********/

char * prvCpssLogEnum_CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT[]  =
{
    "CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E",
    "CPSS_PORT_TX_DROP_MCAST_PRIORITY_HI_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_DROP_PROFILE_SET_ENT[]  =
{
    "CPSS_PORT_TX_DROP_PROFILE_1_E",
    "CPSS_PORT_TX_DROP_PROFILE_2_E",
    "CPSS_PORT_TX_DROP_PROFILE_3_E",
    "CPSS_PORT_TX_DROP_PROFILE_4_E",
    "CPSS_PORT_TX_DROP_PROFILE_5_E",
    "CPSS_PORT_TX_DROP_PROFILE_6_E",
    "CPSS_PORT_TX_DROP_PROFILE_7_E",
    "CPSS_PORT_TX_DROP_PROFILE_8_E",
    "CPSS_PORT_TX_DROP_PROFILE_9_E",
    "CPSS_PORT_TX_DROP_PROFILE_10_E",
    "CPSS_PORT_TX_DROP_PROFILE_11_E",
    "CPSS_PORT_TX_DROP_PROFILE_12_E",
    "CPSS_PORT_TX_DROP_PROFILE_13_E",
    "CPSS_PORT_TX_DROP_PROFILE_14_E",
    "CPSS_PORT_TX_DROP_PROFILE_15_E",
    "CPSS_PORT_TX_DROP_PROFILE_16_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_DROP_PROFILE_SET_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT[]  =
{
    "CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E",
    "CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_DROP_SHAPER_MODE_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_Q_ARB_GROUP_ENT[]  =
{
    "CPSS_PORT_TX_WRR_ARB_GROUP_0_E",
    "CPSS_PORT_TX_WRR_ARB_GROUP_1_E",
    "CPSS_PORT_TX_SP_ARB_GROUP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_Q_ARB_GROUP_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT[]  =
{
    "CPSS_PORT_TX_SCHEDULER_PROFILE_1_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_2_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_3_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_4_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_5_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_6_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_7_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_8_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_9_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_10_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_11_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_12_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_13_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_14_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_15_E",
    "CPSS_PORT_TX_SCHEDULER_PROFILE_16_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT[]  =
{
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E",
    "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT[]  =
{
    "CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E",
    "CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_WRR_MODE_ENT[]  =
{
    "CPSS_PORT_TX_WRR_BYTE_MODE_E",
    "CPSS_PORT_TX_WRR_PACKET_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_WRR_MODE_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_WRR_MTU_ENT[]  =
{
    "CPSS_PORT_TX_WRR_MTU_256_E",
    "CPSS_PORT_TX_WRR_MTU_2K_E",
    "CPSS_PORT_TX_WRR_MTU_8K_E",
    "CPSS_PORT_TX_WRR_MTU_64_E",
    "CPSS_PORT_TX_WRR_MTU_128_E",
    "CPSS_PORT_TX_WRR_MTU_512_E",
    "CPSS_PORT_TX_WRR_MTU_1K_E",
    "CPSS_PORT_TX_WRR_MTU_4K_E",
    "CPSS_PORT_TX_WRR_MTU_16K_E",
    "CPSS_PORT_TX_WRR_MTU_32K_E",
    "CPSS_PORT_TX_WRR_MTU_64K_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_WRR_MTU_ENT);
char * prvCpssLogEnum_CPSS_PORT_TX_TAIL_DROP_PROBABILITY_ENT[]  =
{
    "CPSS_PORT_TX_TAIL_DROP_PROBABILITY_100_E",
    "CPSS_PORT_TX_TAIL_DROP_PROBABILITY_89_E",
    "CPSS_PORT_TX_TAIL_DROP_PROBABILITY_73_E",
    "CPSS_PORT_TX_TAIL_DROP_PROBABILITY_67_E",
    "CPSS_PORT_TX_TAIL_DROP_PROBABILITY_50_E",
    "CPSS_PORT_TX_TAIL_DROP_PROBABILITY_25_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PORT_TX_TAIL_DROP_PROBABILITY_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, guaranteedLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, wredSize);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, probability, CPSS_PORT_TX_TAIL_DROP_PROBABILITY_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp0MaxBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp1MaxBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp2MaxBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp0MaxDescrNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp1MaxDescrNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp2MaxDescrNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tcMaxBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tcMaxDescrNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp0MaxMCBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp1MaxMCBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp2MaxMCBuffNum);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sharedUcAndMcCountersDisable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dp0QueueAlpha, CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dp1QueueAlpha, CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dp2QueueAlpha, CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dp0WredAttributes, CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dp1WredAttributes, CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dp2WredAttributes, CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_DROP_PROFILE_SET_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_DROP_PROFILE_SET_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_DROP_PROFILE_SET_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_DROP_PROFILE_SET_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_DROP_PROFILE_SET_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_DROP_SHAPER_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_DROP_SHAPER_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_DROP_SHAPER_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_DROP_SHAPER_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_Q_ARB_GROUP_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_Q_ARB_GROUP_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_Q_ARB_GROUP_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_Q_ARB_GROUP_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_Q_ARB_GROUP_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_Q_ARB_GROUP_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_WRR_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_WRR_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_WRR_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_WRR_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_WRR_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_WRR_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_WRR_MTU_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_WRR_MTU_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PORT_TX_WRR_MTU_ENT);
}
void prvCpssLogParamFunc_CPSS_PORT_TX_WRR_MTU_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PORT_TX_WRR_MTU_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PORT_TX_WRR_MTU_ENT);
}

