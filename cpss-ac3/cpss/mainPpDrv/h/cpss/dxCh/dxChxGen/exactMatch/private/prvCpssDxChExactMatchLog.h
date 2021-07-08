/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChExactMatchLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChExactMatchLogh
#define __prvCpssDxChExactMatchLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_clientType;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_entryPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR_expandedActionOriginPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR_entryKeyPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_exactMatchLookupNum;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_pclLookupNum;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR_keyParamsPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_defActionEn;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableExactMatchLookup;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_exactMatchClearActivity;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_exactMatchEntryIndex;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_exactMatchProfileIndex;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_expandedActionIndex;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfRules;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_subProfileId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_PTR_actionTypePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_PTR_clientTypePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_entryPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR_expandedActionOriginPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR_keyParamsPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_defActionEnPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableExactMatchLookupPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_exactMatchActivityStatusPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_exactMatchCrcMultiHashArr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_expandedActionIndexPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfElemInCrcMultiHashArrPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiProfileIdSet_E = (CPSS_LOG_LIB_EXACT_MATCH_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiProfileIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPclProfileIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPclProfileIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchClientLookupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchClientLookupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchActivityBitEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchActivityBitEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchProfileKeyParamsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchProfileKeyParamsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchProfileDefaultActionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchProfileDefaultActionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchExpandedActionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchExpandedActionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupEntryInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchHashCrcMultiResultsByParamsCalc_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchRulesDump_E,
    PRV_CPSS_LOG_FUNC_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChExactMatchLogh */
