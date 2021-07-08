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
* @file mvHwsGPcsIf.c
*
* @brief GPCS interface API
*
* @version   12
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsDb.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>


static char* mvHwsPcsTypeGetFunc(void)
{
  return "GPCS";
}
/**
* @internal mvHwsGPcsIfInit function
* @endinternal
*
* @brief   Init GPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcsIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{

    funcPtrArray[GPCS].pcsResetFunc = NULL;
    funcPtrArray[GPCS].pcsModeCfgFunc = NULL;
    funcPtrArray[GPCS].pcsLbCfgFunc = mvHwsGPcsLoopBack;
    funcPtrArray[GPCS].pcsTestGenFunc = mvHwsGPcsTestGenCfg;
    funcPtrArray[GPCS].pcsTestGenStatusFunc = mvHwsGPcsTestGenStatus;
    funcPtrArray[GPCS].pcsActiveStatusGetFunc = mvHwsGPcsActiveStatusGet;

    funcPtrArray[GPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;

    return GT_OK;
}

/**
* @internal mvHwsGPcsLoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcsLoopBack
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
      seq = &hwsGPscSeqDb[GPCS_LPBK_NORMAL_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsGPscSeqDb[GPCS_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
      break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsGPcsTestGenCfg function
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
GT_STATUS mvHwsGPcsTestGenCfg
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
    case TEST_GEN_PRBS7:
      seq = &hwsGPscSeqDb[GPCS_GEN_PRBS7_SEQ];
      break;
    case TEST_GEN_Normal:
      seq = &hwsGPscSeqDb[GPCS_GEN_NORMAL_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
      break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsGPcsTestGenStatus function
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
GT_STATUS mvHwsGPcsTestGenStatus
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
  GT_U32 baseAddr;
  GT_U32 unitIndex;

  pattern = pattern;

  if (status == 0)
  {
    return GT_BAD_PARAM;
  }

  mvUnitInfoGet(devNum, GEMAC_UNIT, &baseAddr, &unitIndex);
  accessAddr  = baseAddr + pcsNum * unitIndex;
  CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+GIG_Port_PRBS_Status_Reg, &data, 0));
  status->checkerLock = data & 1; /* bit 0 */

  CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr+GIG_Port_PRBS_Error_Cntr_Reg, &data, 0));
  status->totalErrors = data;

  status->goodFrames = 0;

  return GT_OK;
}

/**
* @internal mvHwsGPcsActiveStatusGet function
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
GT_STATUS mvHwsGPcsActiveStatusGet
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

    mvUnitInfoGet(devNum, GEMAC_UNIT, &unitAddr, &unitIndex);
    baseAddr = unitAddr + pcsNum * unitIndex;

    *numOfLanes = 0;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, baseAddr + GIG_PORT_MAC_CONTROL_REGISTER2, &data, 0));
    /* check unreset bit status */
    if (!((data >> 3) & 1))
    {
        /* unit is under RESET */
        return GT_OK;
    }
    /* one active lane always */
    *numOfLanes = 1;

    return GT_OK;
}
