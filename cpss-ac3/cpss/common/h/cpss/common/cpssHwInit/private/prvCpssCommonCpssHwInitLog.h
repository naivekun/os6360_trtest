/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssCommonCpssHwInitLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssCommonCpssHwInitLogh
#define __prvCpssCommonCpssHwInitLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_HW_PP_INIT_STAGE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_HW_PP_RESET_SKIP_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PP_INTERFACE_CHANNEL_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_LED_BLINK_DURATION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_LED_BLINK_DUTY_CYCLE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_LED_BLINK_SELECT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_LED_ORDER_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_LED_PULSE_STRETCH_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_RX_BUFF_ALLOC_METHOD_ENT);

#ifdef CHX_FAMILY
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_SYS_HA_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_TX_BUFF_ALLOC_METHOD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_TX_SDMA_QUEUE_MODE_ENT);
#endif


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_LED_GROUP_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_NET_IF_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_RX_BUF_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

#ifdef CHX_FAMILY
void prvCpssLogParamFuncStc_CPSS_AUQ_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_MULTI_NET_IF_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

#endif

#ifdef PX_FAMILY
void prvCpssLogParamFuncStc_CPSS_DMA_QUEUE_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

#endif


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_HW_PP_INIT_STAGE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_HW_PP_RESET_SKIP_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_LED_GROUP_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PP_INTERFACE_CHANNEL_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

#ifdef CHX_FAMILY
void prvCpssLogParamFunc_CPSS_AUQ_CFG_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_MULTI_NET_IF_CFG_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_NET_IF_CFG_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

#endif

#ifdef PX_FAMILY
void prvCpssLogParamFunc_CPSS_DMA_QUEUE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_NET_IF_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

#endif


/********* API fields DB *********/

extern PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_GT_U32_PTR_configDevDataBufferSizePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_void_PTR_configDevDataBufferPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_configDevDataBufferSize;
extern PRV_CPSS_LOG_FUNC_PARAM_STC IN_void_PTR_configDevDataBufferPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPpInterruptsDisable_E = (CPSS_LOG_LIB_HW_INIT_COMMON_E << 16),
    PRV_CPSS_LOG_FUNC_cpssPpConfigDevDataImport_E,
    PRV_CPSS_LOG_FUNC_cpssPpConfigDevDataExport_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssCommonCpssHwInitLogh */
