/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssCommonInitLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/init/private/prvCpssCommonInitLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>


/********* API fields DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_BOOL_PTR_enablePtr = {
     "enablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};


/********* API prototypes DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC * cpssSystemDualDeviceIdModeEnableGet_PARAMS[] =  {
    &OUT_GT_BOOL_PTR_enablePtr
};

#ifdef PX_FAMILY
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssSystemDualDeviceIdModeEnableSet_PARAMS[] =  {
    &IN_GT_BOOL_enable
};
#endif


/********* lib API DB *********/


#ifdef CHX_FAMILY
static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonDXInitLogLibDb[] = {
    {"cpssPpDump", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssSystemDualDeviceIdModeEnableSet", 1, prvCpssLogGenEnable_PARAMS, NULL},
    {"cpssSystemDualDeviceIdModeEnableGet", 1, cpssSystemDualDeviceIdModeEnableGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_INIT(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonDXInitLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonDXInitLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

#ifdef PX_FAMILY
static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonPXInitLogLibDb[] = {
    {"cpssPpDump", 1, prvCpssLogGenDevNum3_PARAMS, NULL},
    {"cpssSystemDualDeviceIdModeEnableSet", 1, cpssSystemDualDeviceIdModeEnableSet_PARAMS, NULL},
    {"cpssSystemDualDeviceIdModeEnableGet", 1, cpssSystemDualDeviceIdModeEnableGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_INIT(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonPXInitLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonPXInitLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

