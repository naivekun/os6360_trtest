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
* @file mvHwsHglPcsIf.c
*
* @brief HGL PCS interface API
*
* @version   12
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/hglPcs/mvHwsHglPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/hglPcs/mvHwsHglPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsPcsTypeGetFunc(void)
{
  return "HGLPCS";
}
/**
* @internal mvHwsHglPcsIfInit function
* @endinternal
*
* @brief   Init HGL PCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglPcsIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[HGLPCS].pcsResetFunc = mvHwsHglPcsReset;
    funcPtrArray[HGLPCS].pcsModeCfgFunc = mvHwsHglPcsMode;
    funcPtrArray[HGLPCS].pcsLbCfgFunc = mvHwsHglPcsLoopBack;
    funcPtrArray[HGLPCS].pcsTestGenStatusFunc = mvHwsHglPcsTestGenStatus;
    funcPtrArray[HGLPCS].pcsActiveStatusGetFunc = mvHwsHglPcsActiveStatusGet;

    funcPtrArray[HGLPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;

    return GT_OK;
}

/**
* @internal mvHwsHglPcsReset function
* @endinternal
*
* @brief   Set the selected PCS number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*                                      action    - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglPcsReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            resetMode
)
{
    MV_CFG_SEQ *seq;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    if ((RESET == resetMode) || (FULL_RESET == resetMode))
    {
        seq = &hwsXpcsPscSeqDb[XPCS_RESET_SEQ];
    }
    else if (UNRESET == resetMode)
    {
        seq = &hwsXpcsPscSeqDb[XPCS_UNRESET_SEQ];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", resetMode);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsHglPcsMode function
* @endinternal
*
* @brief   Set the internal mux's to the required PCS in the PI.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglPcsMode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  numOfLanes;
    MV_CFG_SEQ *seq;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numOfLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? 2 * curPortParams.numOfActLanes : curPortParams.numOfActLanes;

    seq = &hwsHglPscSeqDb[HGLPCS_MODE_MISC_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    switch(numOfLanes)
    {
        case 4:
            seq = &hwsHglPscSeqDb[HGLPCS_MODE_4_LANE_SEQ];
            break;
        case 6:
            seq = &hwsHglPscSeqDb[HGLPCS_MODE_6_LANE_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsHglPcsLoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*                                      pcsType   - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglPcsLoopBack
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(lbType)
    {
    case DISABLE_LB:
      seq = &hwsXpcsPscSeqDb[XPCS_LPBK_NORMAL_SEQ];
      break;
    case RX_2_TX_LB:
      seq = &hwsXpcsPscSeqDb[XPCS_LPBK_RX2TX_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsXpcsPscSeqDb[XPCS_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsHglPcsTestGenStatus function
* @endinternal
*
* @brief   Get PCS internal test generator mechanisms error counters and status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglPcsTestGenStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    MV_HWS_TEST_GEN_STATUS        *status
)
{
  GT_U32 data, tmp;
  GT_U32 accessAddr;
  GT_U32 baseAddr;
  GT_U32 unitIndex;

  if (status == 0)
  {
    return GT_BAD_PARAM;
  }

  mvUnitInfoGet(devNum, XPCS_UNIT, &baseAddr, &unitIndex);
  accessAddr = baseAddr + pcsNum * unitIndex;
  if ((pattern == TEST_GEN_CJPAT) || (pattern == TEST_GEN_CRPAT))
  {
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Tx_Packets_Cntr_LSB, &data, 0));
    status->goodFrames = data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Tx_Packets_Cntr_MSB, &data, 0));
    status->goodFrames += (data << 16);

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_LSB_Lane0, &data, 0));
    status->totalErrors = data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_MSB_Lane0, &data, 0));
    status->totalErrors += (data << 16);

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_LSB_Lane1, &data, 0));
    tmp = data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_MSB_Lane1, &data, 0));
    tmp += (data << 16);
    status->totalErrors += tmp;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_LSB_Lane2, &data, 0));
    tmp = data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_MSB_Lane2, &data, 0));
    tmp += (data << 16);
    status->totalErrors += tmp;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_LSB_Lane3, &data, 0));
    tmp = data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_MSB_Lane3, &data, 0));
    tmp += (data << 16);
    status->totalErrors += tmp;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_LSB_Lane4, &data, 0));
    tmp = data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_MSB_Lane4, &data, 0));
    tmp += (data << 16);
    status->totalErrors += tmp;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_LSB_Lane5, &data, 0));
    tmp = data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_Rx_Bad_Packets_Cntr_MSB_Lane5, &data, 0));
    tmp += (data << 16);
    status->totalErrors += tmp;

    status->checkerLock = 0;
  }
  else if ((pattern == TEST_GEN_PRBS7) || (pattern == TEST_GEN_PRBS23))
  {
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane0, &data, 0));
    status->totalErrors = data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane1, &data, 0));
    status->totalErrors += data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane2, &data, 0));

    status->totalErrors += data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane3, &data, 0));
    status->totalErrors += data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane4, &data, 0));
    status->totalErrors += data;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane5, &data, 0));
    status->totalErrors += data;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane0, &data, 0));
    status->checkerLock = data & 1;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane1, &data, 0));
    status->checkerLock += (data & 1) << 1;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane2, &data, 0));
    status->checkerLock += (data & 1) << 2;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane3, &data, 0));
    status->checkerLock += (data & 1) << 3;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane4, &data, 0));
    status->checkerLock += (data & 1) << 4;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+XPCS_PRBS_Error_Cntr_Lane5, &data, 0));
    status->checkerLock += (data & 1) << 5;
  }
  else
  {
    return GT_NOT_SUPPORTED;
  }
  return GT_OK;
}

/**
* @internal mvHwsHglPcsActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0, if current PCS isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHglPcsActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_U32                  *numOfLanes
)
{
    GT_U32 data;
    GT_U32 baseAddr;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    if (numOfLanes == NULL)
    {
      return GT_BAD_PARAM;
    }

    mvUnitInfoGet(devNum, XPCS_UNIT, &unitAddr, &unitIndex);
    baseAddr = unitAddr + pcsNum * unitIndex;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, baseAddr + XPCS_Global_Configuration_Reg0, &data, 0));

    *numOfLanes = 0;
    /* check unreset bit status */
    if (!(data & 1))
    {
        /* unit is under RESET */
        return GT_OK;
    }
    /* get number of active lanes */
    switch ((data >> 5) & 3)
    {
    case 0:
        *numOfLanes = 1;
        break;
    case 1:
        *numOfLanes = 2;
        break;
    case 2:
        *numOfLanes = 4;
        break;
    case 3:
        *numOfLanes = 6;
        break;
    default:
        *numOfLanes = 0;
        break;
    }

    return GT_OK;
}
