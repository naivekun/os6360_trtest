/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsHglMacIf.c
*
* @brief GE MAC interface
*
* @version   17
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "HGLMAC";
}

/**
* @internal hwsHglMacIfInit function
* @endinternal
*
* @brief   Init HGL MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsHglMacIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[HGLMAC].macRestFunc = mvHwsHglMacReset;
    funcPtrArray[HGLMAC].macModeCfgFunc = mvHwsHglMacModeCfg;
    funcPtrArray[HGLMAC].macLinkGetFunc = mvHwsHglMacLinkStatus;
    funcPtrArray[HGLMAC].macLbCfgFunc = mvHwsHglMacLoopbackCfg;
#ifndef CO_CPU_RUN
    funcPtrArray[HGLMAC].macLbStatusGetFunc = mvHwsHglMacLoopbackStatusGet;
    funcPtrArray[HGLMAC].macActiveStatusFunc = mvHwsHglMacActiveStatusGet;
#endif
    funcPtrArray[HGLMAC].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[HGLMAC].macFcStateCfgFunc = NULL;

    return GT_OK;
}

/**
* @internal mvHwsHglMacReset function
* @endinternal
*
* @brief   Set the HGL MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    devNum      = devNum;
    portGroup   = portGroup;
    portMacNum  = portMacNum;
    action      = action;
    portMode    = portMode;

    return GT_OK;
}

/**
* @internal mvHwsHglMacModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required MAC in the GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    seq = &hwsHglMacSeqDb[HGLMAC_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsHglMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacLinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    GT_U32 data;
    GT_U32 baseAddr;
    GT_U32 unitIndex;
    GT_U32 accessAddr;

    portMode = portMode;
    CHECK_STATUS(mvUnitInfoGet(devNum, XPCS_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + XPCS_Global_Status_Reg + (macNum / 2) * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    *linkStatus = data & 1;
    return GT_OK;
}

/**
* @internal mvHwsHglMacLoopbackCfg function
* @endinternal
*
* @brief   Loopback not supported.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacLoopbackCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    (GT_VOID)devNum;
    (GT_VOID)portGroup;
    (GT_VOID)macNum;
    (GT_VOID)lbType;
    (GT_VOID)portMode;

    return GT_NOT_SUPPORTED;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsHglMacLoopbackStatusGet function
* @endinternal
*
* @brief   Loopback not supported.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    (GT_VOID)devNum;
    (GT_VOID)portGroup;
    (GT_VOID)macNum;
    portMode = portMode;
    lbType = lbType;

    return GT_NOT_SUPPORTED;
}

/**
* @internal mvHwsHglMacActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] numOfLanes               - number of lanes agregated
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglMacActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
)
{
    GT_U32 data;
    GT_U32 baseAddr;
    GT_U32 unitIndex;
    GT_U32 accessAddr;

    portMode = portMode;
    if (numOfLanes == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    *numOfLanes = 0;
    /* get number of active lanes */
    accessAddr = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER3 + macNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    switch ((data >> 13) & 7)
    {
    case 7:
        *numOfLanes = 6;
        break;
    default:
        *numOfLanes = 0;
        break;
    }

    return GT_OK;
}

#endif
