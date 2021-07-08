/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChLatencyMonitoringLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChLatencyMonitoringLogh
#define __prvCpssDxChLatencyMonitoringLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LATENCY_MONITORING_STAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_MONITORING_STAT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_GT_LATENCY_MONITORING_UNIT_BMP(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIRECTION_ENT_portType;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR_lmuConfigPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT_samplingMode;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enabled;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_LATENCY_MONITORING_UNIT_BMP_lmuBmp;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_srcPortNum;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_trgPortNum;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_latencyProfile;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_samplingProfile;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_samplingThreshold;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR_lmuConfigPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT_PTR_samplingModePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LATENCY_MONITORING_STAT_STC_PTR_statisticsPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_samplingProfilePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_samplingThresholdPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringPortCfgSet_E = (CPSS_LOG_LIB_LATENCY_MONITORING_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringPortCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringSamplingProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringSamplingProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringSamplingConfigurationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringSamplingConfigurationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringStatGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringPortEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLatencyMonitoringPortEnableGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChLatencyMonitoringLogh */
