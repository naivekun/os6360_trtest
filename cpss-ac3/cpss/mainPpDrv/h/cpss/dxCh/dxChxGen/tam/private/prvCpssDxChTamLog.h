/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTamLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChTamLogh
#define __prvCpssDxChTamLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_TAM_COUNTER_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TAM_TRACKING_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_COUNTER_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_STATISTIC_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TAM_COUNTER_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TAM_STATISTIC_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_listSize;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfHistograms;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfStats;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfThresholds;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_counterPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_countersArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_tamCounterPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR_binsPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_STATISTIC_STC_PTR_statisticPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC_PTR_notifyFunc;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_highWatermarkLevel;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_histogramId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_listSize;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lowWatermarkLevel;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_microburstId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_snapshotId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_thresholdId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_thresholdLevel;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_counterPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_countersArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_COUNTER_ID_STC_PTR_tamCounterPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_COUNTER_STATISTICS_STC_PTR_statsDataArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_BIN_STC_PTR_binsPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR_histValueArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC_PTR_histValuePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC_PTR_statsDataPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_STATISTIC_STC_PTR_statisticPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC_PTR_thresholdsArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_fwCounter;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_fwStatCounter;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_highWatermarkLevel;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lowWatermarkLevel;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_thresholdLevel;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChTamInit_E = (CPSS_LOG_LIB_TAM_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChTamDebugControl_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamFwDebugCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamSnapshotSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamSnapshotGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamSnapshotStatGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamMicroburstSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamMicroburstGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamMicroburstStatGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamHistogramSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamHistogramGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamThresholdNotificationsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTamEventNotifyBind_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChTamLogh */
