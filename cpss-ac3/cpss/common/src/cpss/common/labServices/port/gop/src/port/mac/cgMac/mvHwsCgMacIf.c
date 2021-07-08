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
* @file mvHwsCgMacIf.c
*
* @brief CG MAC reset interface
*
* @version   5
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacDb.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "CGMAC";
}

/**
* @internal hwsCgMacIfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsCgMacIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{

    funcPtrArray[CGMAC].macRestFunc = mvHwsCgMacReset;
    funcPtrArray[CGMAC].macModeCfgFunc = mvHwsCgMacModeCfg;
    funcPtrArray[CGMAC].macLinkGetFunc = mvHwsCgMacLinkStatus;
    funcPtrArray[CGMAC].macLbCfgFunc = mvHwsCgMacLoopbackCfg;
#ifndef CO_CPU_RUN
    funcPtrArray[CGMAC].macLbStatusGetFunc = mvHwsCgMacLoopbackStatusGet;
#endif
    funcPtrArray[CGMAC].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[CGMAC].macFcStateCfgFunc = NULL;

    return GT_OK;
}


/**
* @internal mvHwsCgMacReset function
* @endinternal
*
* @brief   Set the GE MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ *seq;
    portMode = portMode;

    if ((RESET == action) || (FULL_RESET == action))
    {
        seq = &hwsCgMacSeqDb[CGMAC_RESET_SEQ];
    }
    else if (UNRESET == action)
    {
        seq = &hwsCgMacSeqDb[CGMAC_UNRESET_SEQ];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsCgMacModeCfg function
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
GT_STATUS mvHwsCgMacModeCfg
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

    seq = &hwsCgMacSeqDb[CGMAC_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsCgMacLoopbackCfg function
* @endinternal
*
* @brief   .
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMacLoopbackCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_CFG_SEQ *seq;

    portMode = portMode;

    switch (lbType)
    {
    case DISABLE_LB:
      seq = &hwsCgMacSeqDb[CGMAC_LPBK_NORMAL_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsCgMacSeqDb[CGMAC_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsCgMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMacLinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    GT_U32 data;
    GT_U32 accessAddr;
    GT_U32 unitIndex;
    GT_U32 baseAddr;

    portMode = portMode;
    CHECK_STATUS(mvUnitInfoGet(devNum, CG_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + CG_IP_STATUS + macNum * unitIndex;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    *linkStatus = ((data >> 29) & 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsCgMacRedundencyVectorGet function
* @endinternal
*
* @brief   Get SD vector.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMacRedundencyVectorGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                 *sdVector
)
{
    devNum = devNum;
    portGroup = portGroup;
    sdVector = sdVector;

    return GT_NOT_SUPPORTED;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsCgMacLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 data;
    GT_U32 accessAddr;
    GT_U32 unitIndex;
    GT_U32 baseAddr;

    portMode = portMode;
    CHECK_STATUS(mvUnitInfoGet(devNum, CG_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + CG_COMMAND_CONFIG + macNum * unitIndex;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    *lbType = DISABLE_LB;
    if ((data >> 10) & 1)
    {
        /* loopback enabled (only Tx2Rx supported) */
        *lbType = TX_2_RX_LB;
    }
    return GT_OK;
}
#endif
