/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPhaLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChPhaLogh
#define __prvCpssDxChPhaLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_infoType;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_infoType;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_THREAD_TYPE_ENT_extType;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_packetCommand;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_packetOrderChangeEnable;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_phaThreadId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT_PTR_violationTypePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_THREAD_TYPE_ENT_PTR_extTypePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_packetCommandPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_capturedThreadIdPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_phaThreadIdPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChPhaInit_E = (CPSS_LOG_LIB_PHA_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChPhaThreadIdEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaThreadIdEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaPortThreadIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaPortThreadIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaSourcePortEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaSourcePortEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaTargetPortEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaTargetPortEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaHeaderModificationViolationInfoSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaHeaderModificationViolationInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaHeaderModificationViolationCapturedGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaStatisticalProcessingCounterThreadIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaStatisticalProcessingCounterThreadIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhaStatisticalProcessingCounterGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPhaLogh */
