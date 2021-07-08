/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDriverManualLog.c
*       Manually implemented CPSS Log type wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpssDriver/log/private/prvCpssDriverLog.h>

/********* enums *********/

PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_ARRAY_addrArr = {
     "addrArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_HEX_ARRAY_dataArr = {
     "dataArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_ARRAY_dataArr = {
     "dataArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_HEX_ARRAY_data = {
     "data", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32_HEX)
};
PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_ARRAY_data = {
     "data", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32_HEX)
};

PRV_CPSS_LOG_FUNC_PARAM_STC * prvCpssDrvHwPpReadVec_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_ARRAY_addrArr,
    &OUT_GT_U32_HEX_ARRAY_dataArr,
    &IN_GT_U32_arrLen
};
PRV_CPSS_LOG_FUNC_PARAM_STC * prvCpssDrvHwPpWriteVec_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_ARRAY_addrArr,
    &IN_GT_U32_HEX_ARRAY_dataArr,
    &IN_GT_U32_arrLen
};
PRV_CPSS_LOG_FUNC_PARAM_STC * prvCpssDrvHwPpReadRam_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_addr,
    &IN_GT_U32_length,
    &OUT_GT_U32_HEX_ARRAY_data
};
PRV_CPSS_LOG_FUNC_PARAM_STC * prvCpssDrvHwPpWriteRam_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_addr,
    &IN_GT_U32_length,
    &IN_GT_U32_HEX_ARRAY_data
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDrvPpHwRamRead_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_addr,
    &IN_GT_U32_length,
    &OUT_GT_U32_HEX_ARRAY_data
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDrvPpHwRamWrite_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_addr,
    &IN_GT_U32_length,
    &IN_GT_U32_HEX_ARRAY_data
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDrvPpHwVectorRead_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_ARRAY_addrArr,
    &OUT_GT_U32_HEX_ARRAY_dataArr,
    &IN_GT_U32_arrLen
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssDrvPpHwVectorWrite_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_ARRAY_addrArr,
    &IN_GT_U32_HEX_ARRAY_dataArr,
    &IN_GT_U32_arrLen
};
/********* structure fields log functions *********/
/********* parameters log functions *********/

void prvCpssDrvHwPpReadVec_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* addrArr */
    va_arg(args, GT_U32*);
    /* dataArr */
    va_arg(args, GT_U32*);
    /* arrLen */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}
void prvCpssDrvHwPpReadRam_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* addr */
    va_arg(args, GT_U32);
    /* length */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}
void cpssDrvPpHwRamRead_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portGroupId */
    va_arg(args, GT_U32);
    /* addr */
    va_arg(args, GT_U32);
    /* length */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}
void cpssDrvPpHwVectorRead_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portGroupId */
    va_arg(args, GT_U32);
    /* addrArr */
    va_arg(args, GT_U32*);
    /* dataArr */
    va_arg(args, GT_U32*);
    /* arrLen */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}
