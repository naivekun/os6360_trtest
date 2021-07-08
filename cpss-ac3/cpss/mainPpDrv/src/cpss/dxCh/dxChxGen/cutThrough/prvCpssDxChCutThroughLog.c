/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChCutThroughLog.c
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
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/cutThrough/private/prvCpssDxChCutThroughLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

char * prvCpssLogEnum_CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT[]  =
{
    "CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_ALL_DATA_E",
    "CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_BYTE_COUNT_ONLY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT[]  =
{
    "CPSS_DXCH_EARLY_PROCESSING_CUT_THROUGH_E",
    "CPSS_DXCH_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_EARLY_PROCESSING_MODE_ENT);
char * prvCpssLogEnum_CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT[]  =
{
    "CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E",
    "CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E",
    "CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E",
    "CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CUT_THROUGH_BYPASS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassRouter);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassIngressPolicerStage0);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassIngressPolicerStage1);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassEgressPolicer);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassEgressPcl);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, bypassIngressPcl, CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassIngressOam);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassEgressOam);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bypassMll);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minByteCountBoundary);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxByteCountBoundary);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enableByteCountBoundariesCheck);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enableIpv4HdrCheckByChecksum);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, hdrIntergrityExceptionPktCmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, hdrIntergrityExceptionCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CUT_THROUGH_BYPASS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EARLY_PROCESSING_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_EARLY_PROCESSING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EARLY_PROCESSING_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_EARLY_PROCESSING_MODE_ENT);
}


/********* API fields DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR_bypassModePtr = {
     "bypassModePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CUT_THROUGH_BYPASS_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR_cfgPtr = {
     "cfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT_parsingMode = {
     "parsingMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_EARLY_PROCESSING_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_buffersLimitEnable = {
     "buffersLimitEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_toSubtractOrToAdd = {
     "toSubtractOrToAdd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_udeCutThroughEnable = {
     "udeCutThroughEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_untaggedEnable = {
     "untaggedEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_etherType0 = {
     "etherType0", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_etherType1 = {
     "etherType1", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_buffersLimit = {
     "buffersLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_subtractedOrAddedValue = {
     "subtractedOrAddedValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_udeByteCount = {
     "udeByteCount", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_udeIndex = {
     "udeIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR_bypassModePtr = {
     "bypassModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CUT_THROUGH_BYPASS_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR_cfgPtr = {
     "cfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT_PTR_parsingModePtr = {
     "parsingModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EARLY_PROCESSING_MODE_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SPEED_ENT_PTR_portSpeedPtr = {
     "portSpeedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SPEED_ENT)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_buffersLimitEnablePtr = {
     "buffersLimitEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_toSubtractOrToAddPtr = {
     "toSubtractOrToAddPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_udeCutThroughEnablePtr = {
     "udeCutThroughEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_untaggedEnablePtr = {
     "untaggedEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_etherType0Ptr = {
     "etherType0Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_etherType1Ptr = {
     "etherType1Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_buffersLimitPtr = {
     "buffersLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_countPtr = {
     "countPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_subtractedOrAddedValuePtr = {
     "subtractedOrAddedValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_udeByteCountPtr = {
     "udeByteCountPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughBypassModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR_bypassModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR_cfgPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPassengerParsingModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT_parsingMode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughEarlyProcessingModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT_mode
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughMemoryRateLimitSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_CPSS_PORT_SPEED_ENT_portSpeed
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPortEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_GT_BOOL_untaggedEnable
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPortByteCountUpdateSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_toSubtractOrToAdd,
    &DX_IN_GT_U32_subtractedOrAddedValue
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughMemoryRateLimitGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_CPSS_PORT_SPEED_ENT_PTR_portSpeedPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPortEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_GT_BOOL_PTR_untaggedEnablePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPortByteCountUpdateGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_toSubtractOrToAddPtr,
    &DX_OUT_GT_U32_PTR_subtractedOrAddedValuePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPortGroupMaxBuffersLimitSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_BOOL_buffersLimitEnable,
    &DX_IN_GT_U32_buffersLimit
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPortGroupMaxBuffersLimitGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_BOOL_PTR_buffersLimitEnablePtr,
    &DX_OUT_GT_U32_PTR_buffersLimitPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughMinimalPacketSizeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_size
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughUdeCfgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_udeIndex,
    &DX_IN_GT_BOOL_udeCutThroughEnable,
    &DX_IN_GT_U32_udeByteCount
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughUdeCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_udeIndex,
    &DX_OUT_GT_BOOL_PTR_udeCutThroughEnablePtr,
    &DX_OUT_GT_U32_PTR_udeByteCountPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughVlanEthertypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_HEX_etherType0,
    &DX_IN_GT_U32_HEX_etherType1
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughUpEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_up,
    &DX_IN_GT_BOOL_enable
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughUpEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_up,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughBypassModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR_bypassModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR_cfgPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughPassengerParsingModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT_PTR_parsingModePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughEarlyProcessingModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT_PTR_modePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughByteCountExtractFailsCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_countPtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughMinimalPacketSizeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_sizePtr
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDxChCutThroughVlanEthertypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_HEX_PTR_etherType0Ptr,
    &DX_OUT_GT_U32_HEX_PTR_etherType1Ptr
};


/********* lib API DB *********/

static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChCutThroughLogLibDb[] = {
    {"cpssDxChCutThroughPortEnableSet", 4, cpssDxChCutThroughPortEnableSet_PARAMS, NULL},
    {"cpssDxChCutThroughPortEnableGet", 4, cpssDxChCutThroughPortEnableGet_PARAMS, NULL},
    {"cpssDxChCutThroughUpEnableSet", 3, cpssDxChCutThroughUpEnableSet_PARAMS, NULL},
    {"cpssDxChCutThroughUpEnableGet", 3, cpssDxChCutThroughUpEnableGet_PARAMS, NULL},
    {"cpssDxChCutThroughVlanEthertypeSet", 3, cpssDxChCutThroughVlanEthertypeSet_PARAMS, NULL},
    {"cpssDxChCutThroughVlanEthertypeGet", 3, cpssDxChCutThroughVlanEthertypeGet_PARAMS, NULL},
    {"cpssDxChCutThroughMinimalPacketSizeSet", 2, cpssDxChCutThroughMinimalPacketSizeSet_PARAMS, NULL},
    {"cpssDxChCutThroughMinimalPacketSizeGet", 2, cpssDxChCutThroughMinimalPacketSizeGet_PARAMS, NULL},
    {"cpssDxChCutThroughMemoryRateLimitSet", 4, cpssDxChCutThroughMemoryRateLimitSet_PARAMS, NULL},
    {"cpssDxChCutThroughMemoryRateLimitGet", 4, cpssDxChCutThroughMemoryRateLimitGet_PARAMS, NULL},
    {"cpssDxChCutThroughBypassRouterAndPolicerEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCutThroughBypassRouterAndPolicerEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCutThroughBypassModeSet", 2, cpssDxChCutThroughBypassModeSet_PARAMS, NULL},
    {"cpssDxChCutThroughBypassModeGet", 2, cpssDxChCutThroughBypassModeGet_PARAMS, NULL},
    {"cpssDxChCutThroughPortGroupMaxBuffersLimitSet", 4, cpssDxChCutThroughPortGroupMaxBuffersLimitSet_PARAMS, NULL},
    {"cpssDxChCutThroughPortGroupMaxBuffersLimitGet", 4, cpssDxChCutThroughPortGroupMaxBuffersLimitGet_PARAMS, NULL},
    {"cpssDxChCutThroughEarlyProcessingModeSet", 2, cpssDxChCutThroughEarlyProcessingModeSet_PARAMS, NULL},
    {"cpssDxChCutThroughEarlyProcessingModeGet", 2, cpssDxChCutThroughEarlyProcessingModeGet_PARAMS, NULL},
    {"cpssDxChCutThroughUdeCfgSet", 4, cpssDxChCutThroughUdeCfgSet_PARAMS, NULL},
    {"cpssDxChCutThroughUdeCfgGet", 4, cpssDxChCutThroughUdeCfgGet_PARAMS, NULL},
    {"cpssDxChCutThroughPassengerParsingModeSet", 2, cpssDxChCutThroughPassengerParsingModeSet_PARAMS, NULL},
    {"cpssDxChCutThroughPassengerParsingModeGet", 2, cpssDxChCutThroughPassengerParsingModeGet_PARAMS, NULL},
    {"cpssDxChCutThroughByteCountExtractFailsCounterGet", 2, cpssDxChCutThroughByteCountExtractFailsCounterGet_PARAMS, NULL},
    {"cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet", 2, cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet_PARAMS, NULL},
    {"cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet", 2, cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet_PARAMS, NULL},
    {"cpssDxChCutThroughPortByteCountUpdateSet", 4, cpssDxChCutThroughPortByteCountUpdateSet_PARAMS, NULL},
    {"cpssDxChCutThroughPortByteCountUpdateGet", 4, cpssDxChCutThroughPortByteCountUpdateGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_CUT_THROUGH(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChCutThroughLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChCutThroughLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

