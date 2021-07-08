/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenCommonTypesManualLog.c
*       Manually implemented CPSS Log type  wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtEnvDep.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>


void prvCpssLogParamFuncStc_GT_U64_PTR
(
    IN    CPSS_LOG_LIB_ENT                        contextLib,
    IN    CPSS_LOG_TYPE_ENT                       logType,
    IN    GT_CHAR_PTR                             namePtr,
    IN    void                                  * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    GT_U64 *valPtr = (GT_U64*) fieldPtr;
    if (valPtr == NULL) 
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogU64Number(contextLib,logType,namePtr,valPtr,GT_FALSE);
}

void prvCpssLogParamFunc_GT_PORT_GROUPS_BMP(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    inOutParamInfoPtr->formatPtr = "%s = 0x%04x\n";
    prvCpssLogParamFunc_GT_U32(contextLib, logType, namePtr,argsPtr,skipLog,inOutParamInfoPtr);
}

