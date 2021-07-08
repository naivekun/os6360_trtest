/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpLpmEngineLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChIpLpmEngineLogh
#define __prvCpssDxChIpLpmEngineLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_TCAM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_FCID_PTR_fcoeAddrPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipAddrPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipGroupPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipSrcPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipAddrPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipGroupPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipSrcPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_defaultIpv4RuleIndexArrayLenPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_defaultIpv6RuleIndexArrayLenPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_ipGroupPrefixLenPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_ipSrcPrefixLenPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_lpmDbMemBlockSizePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfDevsPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_pclIdArrayLenPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_prefixLenPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_UINTPTR_PTR_iterPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR_fcoeAddrPrefixArrayPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR_ipv4PrefixArrayPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR_ipv6PrefixArrayPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR_memoryCfgPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_shadowType;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR_vrConfigPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR_tcamLpmManagerCapcityCfgPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR_indexesRangePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_activityBit;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_changeGrpAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_changeSrcAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_clearActivity;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_defragmentationEnable;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_dump;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isWholeFCoERandom;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isWholeIpRandom;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_returnOnFailure;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_returnOnFirstFail;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_FCID_PTR_fcoeAddrPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_FCID_startFcoeAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipAddrPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipGroupPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipSrcPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_grpStartIpAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_srcStartIpAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_startIpAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipAddrPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipGroupPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipSrcPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_grpStartIpAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_lastIpAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_srcStartIpAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_startIpAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_lpmDbMemBlockPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_pclIdArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_eighthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_elevenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fcoeAddrPrefixArrayLen;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fifteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fifthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_firstOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fourteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fourthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpEighthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpElevenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpFifteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpFifthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpFirstOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpFourteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpFourthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpNinthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpSecondOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpSeventhOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpSixteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpSixthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpTenthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpThirdOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpThirteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_grpTwelveOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipGroupPrefixLen;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipSrcPrefixLen;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipv4PrefixArrayLen;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipv6PrefixArrayLen;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmDBId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmDbId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ninthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfPrefixesToAdd;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfPrefixesToDel;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfPrefixesToGet;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pclIdArrayLen;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_prefixLength;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routeEntryBaseMemAddr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_secondOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_seventhOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sixteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sixthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcEighthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcElevenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcFifteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcFifthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcFirstOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcFourteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcFourthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcNinthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcPrefixLen;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcSecondOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcSeventhOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcSixteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcSixthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcTenthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcThirdOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcThirteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcTwelveOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_step;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tenthOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_thirdOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_thirteenOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_twelveOctetToChange;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vrId;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_srcStepArr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_stepArr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR_memoryCfgPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_PTR_shadowTypePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR_vrConfigPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR_tcamLpmManagerCapcityCfgPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR_indexesRangePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IP_PROTOCOL_STACK_ENT_PTR_protocolStackPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_activityBitPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_activityStatusPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_defaultMcUsedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_defragEnablePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_partitionEnablePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_FCID_PTR_lastFcoeAddrAddedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_FCID_PTR_lastFcoeAddrDeletedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPADDR_PTR_grpLastIpAddrAddedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPADDR_PTR_grpLastIpAddrDeletedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPADDR_PTR_lastIpAddrAddedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPADDR_PTR_lastIpAddrDeletedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPADDR_PTR_srcLastIpAddrAddedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPADDR_PTR_srcLastIpAddrDeletedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_grpLastIpAddrAddedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_grpLastIpAddrDeletedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_lastIpAddrAddedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_lastIpAddrDeletedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_srcLastIpAddrAddedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_srcLastIpAddrDeletedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_defaultIpv4RuleIndexArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_defaultIpv6RuleIndexArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lpmDbMemBlockPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lpmDbSizePtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfPrefixesAddedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfPrefixesDeletedPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfRetrievedPrefixesPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pclIdArray;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamColumnIndexPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamGroupColumnIndexPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamGroupRowIndexPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamRowIndexPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamSrcColumnIndexPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamSrcRowIndexPtr;
extern PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_devListArray;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBCreate_E = (CPSS_LOG_LIB_IP_LPM_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBCapacityUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBCapacityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBDevListAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBDevsListRemove_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBDevListGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmVirtualRouterAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmVirtualRouterSharedAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmVirtualRouterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmVirtualRouterDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixAddBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixDelBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixSearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntryAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntryDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntriesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntryGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntrySearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixAddBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixDelBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixSearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntryAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntryDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntriesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntryGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntrySearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixAddBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixDelBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixSearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBMemSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBExport_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBImport_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmActivityBitEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmActivityBitEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntryActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntryActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgRouteTcamDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgPatTriePrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgPatTrieValidityCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwMemPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwValidation_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgShadowValidityCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwShadowSyncValidityCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4UcPrefixAddMany_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv6UcPrefixGetManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv6UcPrefixAddRandomByRange_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4McPrefixGetManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv6McPrefixGetManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwOctetPerBlockPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwOctetPerProtocolPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv4SetStep_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgIpv6SetStep_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgFcoePrefixAddMany_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgFcoePrefixAddManyRandom_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgFcoePrefixAddManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgFcoePrefixDelManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgFcoePrefixGetManyByOctet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgDefragEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgDefragEnableGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChIpLpmEngineLogh */
