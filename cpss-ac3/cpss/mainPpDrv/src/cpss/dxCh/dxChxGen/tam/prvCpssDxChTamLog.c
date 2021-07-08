/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTamLog.c
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
#include <cpss/dxCh/dxChxGen/tam/cpssDxChTam.h>
#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTamLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TAM_COUNTER_TYPE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_INGRESS_GLOBAL_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_INGRESS_GLOBAL_MIRROR_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_INGRESS_PORT_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_INGRESS_PG_TC_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_EGRESS_GLOBAL_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_EGRESS_GLOBAL_MULTI_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_EGRESS_GLOBAL_MIRROR_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_EGRESS_PORT_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_EGRESS_QUEUE_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_EGRESS_SHARED_POOL_BUFFER_COUNTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_TAM_COUNTER_TYPE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_TAM_TRACKING_MODE_ENT[]  =
{
    "CPSS_DXCH_TAM_TRACKING_MODE_CURRENT_E",
    "CPSS_DXCH_TAM_TRACKING_MODE_PEAK_E",
    "CPSS_DXCH_TAM_TRACKING_MODE_MINIMUM_E",
    "CPSS_DXCH_TAM_TRACKING_MODE_AVERAGE_E",
    "CPSS_DXCH_TAM_TRACKING_MODE_HISTOGRAM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TAM_TRACKING_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TAM_COUNTER_ID_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, type, CPSS_DXCH_TAM_COUNTER_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, id);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, port);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, queue);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_COUNTER_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TAM_COUNTER_STATISTICS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, current, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, minimum, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, peak, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, average, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TAM_HISTOGRAM_BIN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, bin, CPSS_DXCH_TAM_HISTOGRAM_BIN_LEVELS, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, histogramId);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, value, CPSS_DXCH_TAM_HISTOGRAM_BIN_VALUES, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lastDuration);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, shortestDuration);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, longestDuration);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, averageDuration);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numMicrobursts);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_STATISTIC_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TAM_STATISTIC_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mode, CPSS_DXCH_TAM_TRACKING_MODE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, counter, CPSS_DXCH_TAM_COUNTER_ID_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, thresholdId);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, value, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TAM_COUNTER_ID_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TAM_COUNTER_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TAM_COUNTER_STATISTICS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TAM_COUNTER_STATISTICS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TAM_HISTOGRAM_BIN_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TAM_STATISTIC_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TAM_STATISTIC_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TAM_STATISTIC_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char *formatPtr;
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC*, paramVal);
    PRV_CPSS_LOG_PTR_FORMAT_MAC(formatPtr);
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


/********* API fields DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_listSize = {
     "listSize", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfHistograms = {
     "numOfHistograms", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfStats = {
     "numOfStats", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfThresholds = {
     "numOfThresholds", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_counterPtr = {
     "counterPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_COUNTER_ID_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_countersArray = {
     "countersArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_COUNTER_ID_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_tamCounterPtr = {
     "tamCounterPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_COUNTER_ID_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR_binsPtr = {
     "binsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_HISTOGRAM_BIN_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_STATISTIC_STC_PTR_statisticPtr = {
     "statisticPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_STATISTIC_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC_PTR_notifyFunc = {
     "notifyFunc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_highWatermarkLevel = {
     "highWatermarkLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_histogramId = {
     "histogramId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_listSize = {
     "listSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lowWatermarkLevel = {
     "lowWatermarkLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_microburstId = {
     "microburstId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_snapshotId = {
     "snapshotId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_thresholdId = {
     "thresholdId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_thresholdLevel = {
     "thresholdLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U64)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_counterPtr = {
     "counterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_COUNTER_ID_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_countersArray = {
     "countersArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_COUNTER_ID_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_tamCounterPtr = {
     "tamCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_COUNTER_ID_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_COUNTER_STATISTICS_STC_PTR_statsDataArray = {
     "statsDataArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_COUNTER_STATISTICS_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR_binsPtr = {
     "binsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_HISTOGRAM_BIN_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR_histValueArray = {
     "histValueArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR_histValuePtr = {
     "histValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC_PTR_statsDataPtr = {
     "statsDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_STATISTIC_STC_PTR_statisticPtr = {
     "statisticPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_STATISTIC_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC_PTR_thresholdsArray = {
     "thresholdsArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_fwCounter = {
     "fwCounter", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_fwStatCounter = {
     "fwStatCounter", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_highWatermarkLevel = {
     "highWatermarkLevel", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lowWatermarkLevel = {
     "lowWatermarkLevel", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_thresholdLevel = {
     "thresholdLevel", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};


/********* API prototypes DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamEventNotifyBind_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC_PTR_notifyFunc
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamDebugControl_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_startStopStatistics
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamHistogramSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_histogramId,
    &DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_counterPtr,
    &DX_IN_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR_binsPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamHistogramGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_histogramId,
    &DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_counterPtr,
    &DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR_binsPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamMicroburstSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_microburstId,
    &DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_tamCounterPtr,
    &DX_IN_GT_U32_highWatermarkLevel,
    &DX_IN_GT_U32_lowWatermarkLevel
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamMicroburstGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_microburstId,
    &DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_tamCounterPtr,
    &DX_OUT_GT_U32_PTR_highWatermarkLevel,
    &DX_OUT_GT_U32_PTR_lowWatermarkLevel
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamMicroburstStatGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_microburstId,
    &DX_OUT_CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC_PTR_statsDataPtr,
    &DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR_histValuePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamSnapshotSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_snapshotId,
    &DX_IN_GT_U32_listSize,
    &DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_countersArray
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamSnapshotGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_snapshotId,
    &DX_INOUT_GT_U32_PTR_listSize,
    &DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_countersArray
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamSnapshotStatGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_snapshotId,
    &DX_INOUT_GT_U32_PTR_numOfStats,
    &DX_OUT_CPSS_DXCH_TAM_COUNTER_STATISTICS_STC_PTR_statsDataArray,
    &DX_INOUT_GT_U32_PTR_numOfHistograms,
    &DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR_histValueArray
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamThresholdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_thresholdId,
    &DX_IN_CPSS_DXCH_TAM_STATISTIC_STC_PTR_statisticPtr,
    &DX_IN_GT_U64_thresholdLevel
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamThresholdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_thresholdId,
    &DX_OUT_CPSS_DXCH_TAM_STATISTIC_STC_PTR_statisticPtr,
    &DX_OUT_GT_U64_PTR_thresholdLevel
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamThresholdNotificationsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_INOUT_GT_U32_PTR_numOfThresholds,
    &DX_OUT_CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC_PTR_thresholdsArray
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChTamFwDebugCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_fwCounter,
    &DX_OUT_GT_U32_PTR_fwStatCounter
};


/********* lib API DB *********/

static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChTamLogLibDb[] = {
    {"cpssDxChTamInit", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChTamDebugControl", 2, cpssDxChTamDebugControl_PARAMS, NULL},
    {"cpssDxChTamFwDebugCountersGet", 3, cpssDxChTamFwDebugCountersGet_PARAMS, NULL},
    {"cpssDxChTamSnapshotSet", 4, cpssDxChTamSnapshotSet_PARAMS, NULL},
    {"cpssDxChTamSnapshotGet", 4, cpssDxChTamSnapshotGet_PARAMS, NULL},
    {"cpssDxChTamSnapshotStatGet", 6, cpssDxChTamSnapshotStatGet_PARAMS, NULL},
    {"cpssDxChTamMicroburstSet", 5, cpssDxChTamMicroburstSet_PARAMS, NULL},
    {"cpssDxChTamMicroburstGet", 5, cpssDxChTamMicroburstGet_PARAMS, NULL},
    {"cpssDxChTamMicroburstStatGet", 4, cpssDxChTamMicroburstStatGet_PARAMS, NULL},
    {"cpssDxChTamHistogramSet", 4, cpssDxChTamHistogramSet_PARAMS, NULL},
    {"cpssDxChTamHistogramGet", 4, cpssDxChTamHistogramGet_PARAMS, NULL},
    {"cpssDxChTamThresholdSet", 4, cpssDxChTamThresholdSet_PARAMS, NULL},
    {"cpssDxChTamThresholdGet", 4, cpssDxChTamThresholdGet_PARAMS, NULL},
    {"cpssDxChTamThresholdNotificationsGet", 3, cpssDxChTamThresholdNotificationsGet_PARAMS, NULL},
    {"cpssDxChTamEventNotifyBind", 2, cpssDxChTamEventNotifyBind_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_TAM(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChTamLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChTamLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

