/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChConfigManualLog.c
*       Manually implemented CPSS Log type wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/cpssHwInit/private/prvCpssCommonCpssHwInitLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChConfigLog.h>
#include <cpss/generic/cpssHwInit/private/prvCpssGenCpssHwInitLog.h>
#include <cpss/generic/extMac/private/prvCpssGenExtMacLog.h>
#include <cpss/generic/phy/private/prvCpssGenPhyLog.h>

/********* enums *********/

/********* structure fields log functions *********/
void prvCpssLogParamFuncStc_CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC *, valPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, phyMacType, CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mngInterfaceType, CPSS_PP_INTERFACE_CHANNEL_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mngBusNumber);
    PRV_CPSS_LOG_STC_UINTPTR_MAC(valPtr,busBaseAddr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, interruptMultiPurposePinIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, internalFcTxqPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfRemotePorts);
    inOutParamInfoPtr->paramKey.paramKeyArr[2] = valPtr->numOfRemotePorts;
    prvCpssLogArrayOfParamsHandle(contextLib,logType,"remotePortInfoArr",(void*) valPtr->remotePortInfoArr, sizeof(CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC), inOutParamInfoPtr,
                                  prvCpssLogParamFuncStc_CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC_PTR);
    prvCpssLogStcLogEnd(contextLib, logType);
}

/********* parameters log functions *********/


