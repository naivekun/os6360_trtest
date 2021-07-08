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
* @file mvHwsMMPcsIf.c
*
* @brief MMPCS interface API
*
* @version   35
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsPcsTypeGetFunc(void)
{
  return "MMPCS";
}

GT_STATUS mvHwsMMPcsResetWa
(
 GT_U8                   devNum,
 GT_U32                  portGroup,
 GT_U32                  pcsNum
);

/**
* @internal mvHwsMMPcsIfInit function
* @endinternal
*
* @brief   Init MMPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[MMPCS].pcsResetFunc = mvHwsMMPcsReset;
    funcPtrArray[MMPCS].pcsModeCfgFunc = mvHwsMMPcsMode;
    funcPtrArray[MMPCS].pcsLbCfgFunc = mvHwsMMPcsLoopBack;
    funcPtrArray[MMPCS].pcsTestGenFunc = mvHwsMMPcsTestGenCfg;
    funcPtrArray[MMPCS].pcsTestGenStatusFunc = mvHwsMMPcsTestGenStatus;
    funcPtrArray[MMPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
    funcPtrArray[MMPCS].pcsRxResetFunc = mvHwsMMPcsRxReset;
    funcPtrArray[MMPCS].pcsSignalDetectMaskEn = mvHwsMMPcsSignalDetectMaskSet;
    funcPtrArray[MMPCS].pcsFecCfgFunc =     mvHwsMMPcsFecConfig;
    funcPtrArray[MMPCS].pcsFecCfgGetFunc =  mvHwsMMPcsFecConfigGet;
    funcPtrArray[MMPCS].pcsActiveStatusGetFunc = mvHwsMMPcsActiveStatusGet;

    return GT_OK;
}


static GT_STATUS mvHwsMMPcsUnreset
(
  GT_U8  devNum,
  GT_U32 portGroup,
  GT_U32 pcsNum,
  GT_U32 countVal
)
{
  GT_U32 regAddr;
  GT_U32 i;
  GT_U32 unitAddr;
  GT_U32 unitIndex;

  mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

  for (i = 0; i < countVal; i++)
  {
    regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 1, 1));
  }
  for (i = 0; i < countVal; i++)
  {
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x2, 0x2));
  }
  for (i = 0; i < countVal; i++)
  {
    regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    /* only MMPCS 0 are relevant; other lanes keep reset set 0 */
    if (i == 0)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x4, 0x4));
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x0, 0x4));
    }
  }
  for (i = 0; i < countVal; i++)
  {
    regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    /* only MMPCS 0 are relevant; other lanes keep reset set 0 */
    if (i == 0)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x20, 0x20));
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x0, 0x20));
    }
  }
  for (i = 0; i < countVal; i++)
  {
    regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    /* only MMPCS 0 are relevant; other lanes keep reset set 0 */
    if (i == 0)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x40, 0x40));
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x0, 0x40));
    }
  }
  for (i = 0; i < countVal; i++)
  {
    regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x10, 0x10));
  }
  for (i = 0; i < countVal; i++)
  {
    regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 8, 8));
  }

  return GT_OK;
}

/**
* @internal mvHwsMMPcsReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ *seq;
    GT_U32 regAddr;
    GT_U32 data, i;
    GT_U32 countVal;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    if ((RESET == action) || (FULL_RESET == action))
    {
      seq = &hwsMmPscSeqDb[MMPCS_RESET_SEQ];
      for (i = 0; i < countVal; i++)
      {
        CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum + i, seq->cfgSeq, seq->cfgSeqSize));
      }
    }
    else if (UNRESET == action)
    {
      CHECK_STATUS(mvHwsMMPcsUnreset(devNum,portGroup,pcsNum,countVal));

      mvHwsMMPcsResetWa(devNum, portGroup, pcsNum);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcsMode function
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
GT_STATUS mvHwsMMPcsMode
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
    GT_U32 regAddr;
    GT_U32 data;
    GT_U32 mask;
    GT_U32 i;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numOfLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? 2 * curPortParams.numOfActLanes : curPortParams.numOfActLanes;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    /* config FEC and lanes number */
    for (i = 0; i < numOfLanes; i++)
    {
      regAddr = PCS40G_COMMON_CONTROL + unitAddr + (curPortParams.portPcsNumber+i) * unitIndex;
      data = (curPortParams.portFecMode == FC_FEC) ? (1 << 10) : 0;
      mask = (1 << 10) + (1 << 7); /* always set bit 7 to 0;  */
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, mask));
    }

    switch (numOfLanes)
    {
        case 1:
            seq = &hwsMmPscSeqDb[MMPCS_MODE_1_LANE_SEQ];
            break;
        case 2:
            seq = &hwsMmPscSeqDb[MMPCS_MODE_2_LANE_SEQ];
            break;
        case 4:
            seq = &hwsMmPscSeqDb[MMPCS_MODE_4_LANE_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMMPcsLoopBack function
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
GT_STATUS mvHwsMMPcsLoopBack
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
      seq = &hwsMmPscSeqDb[MMPCS_LPBK_NORMAL_SEQ];
      break;
    case RX_2_TX_LB:
      seq = &hwsMmPscSeqDb[MMPCS_LPBK_RX2TX_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsMmPscSeqDb[MMPCS_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
      break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMMPcsTestGenCfg function
* @endinternal
*
* @brief   Set PCS internal test generator mechanisms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsTestGenCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern
)
{
    MV_CFG_SEQ *seq;

    switch (pattern)
    {
    case TEST_GEN_KRPAT:
      seq = &hwsMmPscSeqDb[MMPCS_GEN_KRPAT_SEQ];
      break;
    case TEST_GEN_Normal:
      seq = &hwsMmPscSeqDb[MMPCS_GEN_NORMAL_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
      break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMMPcsTestGenStatus function
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
GT_STATUS mvHwsMMPcsTestGenStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    MV_HWS_TEST_GEN_STATUS        *status
)
{
  GT_U32 data;
  GT_U32 accessAddr;
  GT_U32 unitAddr;
  GT_U32 unitIndex;

  pattern = pattern;

  if (status == 0)
  {
    return GT_BAD_PARAM;
  }

  mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);
  accessAddr = FEC_DEC_STATUS_REG + unitAddr + pcsNum * unitIndex;;
  CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
  status->checkerLock = (data >> 3) & 1; /* bit 3 */

  accessAddr = TEST_PATTERN_ERROR_COUNT + unitAddr + pcsNum * unitIndex;;
  CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
  status->totalErrors = data;

  status->goodFrames = 0;

  return GT_OK;
}

static GT_STATUS pcsRxReset
(
  GT_U8  devNum,
  GT_U32 portGroup,
  GT_U32 pcsNum,
  GT_U32 countVal
)
{
  GT_U32 regAddr, data, i;
  GT_U32 tryNum;
  GT_U32 unitAddr;
  GT_U32 unitIndex;

  mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

  regAddr = unitAddr + pcsNum*unitIndex + 0x30;
  hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);
  tryNum = 0;
  /*hwsOsPrintf("\nBefore Run PCS WA on pcs #%d (data is 0x%x).",  pcsNum, data);*/
  while (((data & 1) == 0) && (tryNum < 10))
  {
    /* reset PCS */
    for (i = 0; i < countVal; i++)
    {
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, 0x38));
    }

    hwsOsTimerWkFuncPtr(1);
    /* unreset PCS */
    for (i = 0; i < countVal; i++)
    {
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, (1 << 5), (1 << 5)));
    }
    for (i = 0; i < countVal; i++)
    {
        regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, (1 << 4), (1 << 4)));
    }
    for (i = 0; i < countVal; i++)
    {
        regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, (1 << 3), (1 << 3)));
    }

    hwsOsTimerWkFuncPtr(1);
    /* Check AlignLock */
    regAddr = unitAddr + pcsNum*unitIndex + 0x30;
    hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);
    tryNum++;
  }
    /*hwsOsPrintf("\nAfter Run PCS WA on pcs #%d (data is 0x%x, loop number is %d).",
        pcsNum, data, tryNum);*/

  if ((data & 1) == 1)
  {
    /* align locked */
    return GT_OK;
  }
    return GT_FAIL;
}


GT_STATUS mvHwsMMPcsResetWa
(
 GT_U8                   devNum,
 GT_U32                  portGroup,
 GT_U32                  pcsNum
)
{
    GT_U32 regAddr, data, gearVal;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    hwsOsTimerWkFuncPtr(10);
    /* Check Gear Box Status */
    regAddr = unitAddr + pcsNum*unitIndex + 0xD0;
    hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);
    gearVal = (data >> 1) & 0xF;
    /*hwsOsPrintf("\nPCS reset WA on port %d. Gear Box is 0x%x.", pcsNum, gearVal);*/
    if (gearVal != 0xF)
    {
        if (gearVal == 0)
        {
            /* no connector in port */
            return GT_OK;
        }
        else
        {
          return GT_FAIL;
        }
    }
    else
    {
        GT_U32 countVal;

        regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum*unitIndex;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
        countVal = (data >> 8) & 3;
        countVal = (countVal == 0) ? 1 : countVal * 2;
        /* Lane is Locked -> Check the Align Lock Status  */
        CHECK_STATUS(pcsRxReset(devNum, portGroup, pcsNum, countVal));
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcsRxReset function
* @endinternal
*
* @brief   Set the selected RX PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsRxReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_RESET            action
)
{
    devNum = devNum;
    portGroup = portGroup;
    pcsNum = pcsNum;
    action = action;

    return GT_OK;
}

/**
* @internal mvHwsMMPcsSignalDetectMaskSet function
* @endinternal
*
* @brief   Set all related PCS with Signal Detect Mask value (1/0).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] maskEn                   - if true, enable signal detect mask bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 maskEn
)
{
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 countVal, i;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum*unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    for (i = 0; i < countVal; i++)
    {
        regAddr = CHANNEL_CONFIGURATION + unitAddr + (pcsNum + i)*unitIndex;
        data = (maskEn == GT_TRUE) ? (1 << 15) : 0;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, (1 << 15)));
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcsFecConfig function
* @endinternal
*
* @brief   Configure FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] portFecType              - FEC on/off
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsFecConfig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_FEC_MODE    portFecType
)
{
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 countVal, i;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum*unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    data = (FC_FEC == portFecType) ? (1 << 10) : 0;
    for (i = 0; i < countVal; i++)
    {
        regAddr = PCS40G_COMMON_CONTROL + unitAddr + (pcsNum + i)*unitIndex;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, (1 << 10)));
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcsFecConfigGet function
* @endinternal
*
* @brief   Return FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr           - FEC on/off
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsFecConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));

    *portFecTypePtr = ((data >> 10) & 1) ? FC_FEC : FEC_OFF;

    return GT_OK;
}

/**
* @internal mvHwsMMPcsActiveStatusGet function
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
GT_STATUS mvHwsMMPcsActiveStatusGet
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

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);
    baseAddr = unitAddr + pcsNum * unitIndex;

    *numOfLanes = 0;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, baseAddr + PCS_RESET_REG, &data, 0));
    /* check unreset bit status */
    if (!((data >> 6) & 1))
    {
        /* unit is under RESET */
        return GT_OK;
    }
    /* get number of active lanes */
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, baseAddr + PCS40G_COMMON_CONTROL, &data, 0));
    switch ((data >> 8) & 3)
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
    default:
        *numOfLanes = 0;
        break;
    }

    return GT_OK;
}


