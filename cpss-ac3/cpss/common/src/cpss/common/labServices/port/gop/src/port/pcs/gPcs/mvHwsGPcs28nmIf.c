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
* @file mvHwsGPcs28nmIf.c
*
* @brief GPCS interface API (for bobcat2)
*
* @version   7
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsDb.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>

#ifndef CO_CPU_RUN
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf.h>
#endif

#ifndef CO_CPU_RUN
static char* mvHwsPcsTypeGetFunc(void)
{
  return "GPCS_28nm";
}
#endif

/**
* @internal mvHwsGPcs28nmIfInit function
* @endinternal
*
* @brief   Init GPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcs28nmIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{

    funcPtrArray[GPCS].pcsResetFunc = NULL;
    funcPtrArray[GPCS].pcsModeCfgFunc = NULL;
    funcPtrArray[GPCS_NET].pcsResetFunc = NULL;
    funcPtrArray[GPCS_NET].pcsModeCfgFunc = NULL;
    funcPtrArray[QSGMII_PCS].pcsResetFunc = NULL;
    funcPtrArray[QSGMII_PCS].pcsModeCfgFunc = NULL;

#ifndef CO_CPU_RUN
    funcPtrArray[GPCS].pcsLbCfgFunc = mvHwsGPcs28nmLoopBack;
    funcPtrArray[GPCS].pcsLbCfgGetFunc = mvHwsGPcs28nmLoopBackGet;
    funcPtrArray[GPCS].pcsSignalDetectMaskEn = mvHwsGPcs28nmSignalDetectMaskSet;
    funcPtrArray[GPCS].pcsTestGenStatusFunc = mvHwsGPcs28nmTestGenStatus;
    funcPtrArray[GPCS].pcsTestGenFunc = mvHwsGPcs28nmTestGenCfg;
    funcPtrArray[GPCS].pcsActiveStatusGetFunc = mvHwsGPcs28nmActiveStatusGet;
    funcPtrArray[GPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
    funcPtrArray[GPCS_NET].pcsLbCfgFunc = mvHwsGPcs28nmLoopBack;
    funcPtrArray[GPCS_NET].pcsLbCfgGetFunc = mvHwsGPcs28nmLoopBackGet;
    funcPtrArray[GPCS_NET].pcsSignalDetectMaskEn = mvHwsGPcs28nmSignalDetectMaskSet;
    funcPtrArray[GPCS_NET].pcsTestGenStatusFunc = mvHwsGPcs28nmTestGenStatus;
    funcPtrArray[GPCS_NET].pcsTestGenFunc = mvHwsGPcs28nmTestGenCfg;
    funcPtrArray[GPCS_NET].pcsActiveStatusGetFunc = mvHwsGPcs28nmActiveStatusGet;
    funcPtrArray[GPCS_NET].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
    funcPtrArray[QSGMII_PCS].pcsLbCfgFunc = mvHwsGPcs28nmLoopBack;
    funcPtrArray[QSGMII_PCS].pcsLbCfgGetFunc = mvHwsGPcs28nmLoopBackGet;
    funcPtrArray[QSGMII_PCS].pcsSignalDetectMaskEn = mvHwsQsgmiiPcs28nmSignalDetectMaskSet;
    funcPtrArray[QSGMII_PCS].pcsTestGenStatusFunc = mvHwsGPcs28nmTestGenStatus;
    funcPtrArray[QSGMII_PCS].pcsTestGenFunc = mvHwsGPcs28nmTestGenCfg;
    funcPtrArray[QSGMII_PCS].pcsActiveStatusGetFunc = mvHwsGPcs28nmActiveStatusGet;
    funcPtrArray[QSGMII_PCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
#endif

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsGPcs28nmLoopBack function
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
GT_STATUS mvHwsGPcs28nmLoopBack
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
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
* @internal mvHwsGPcs28nmLoopBackGet function
* @endinternal
*
* @brief   Get the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcs28nmLoopBackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, curPortParams.portPcsNumber, GIG_PORT_MAC_CONTROL_REGISTER1, &data, (0x1 << 6)));

    switch(data>>6)
    {
    case 0:
        *lbType = DISABLE_LB;
        break;
    case 1:
        *lbType = TX_2_RX_LB;
        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal mvHwsGPcs28nmSerdesSignalDetectMaskSet function
* @endinternal
*
* @brief   Set all related PCS with Signal Detect Mask value (1/0).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - port number
*                                      PortMode   - port mode
* @param[in] maskEn                   - if true, enable signal detect mask bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcs28nmSerdesSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 maskEn
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[1];

    /* signal detect is done in the Serdes */

    /* we assume that PCS num is the correlated to port num*/
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        return GT_BAD_PARAM;
    }

    if(curPortParams.numOfActLanes != 1)
    {
        return GT_BAD_SIZE;
    }

    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));


    /* set bit 0 */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, curLanesList[0] & 0xFFFF,
                                        SERDES_EXTERNAL_CONFIGURATION_2, (maskEn << 8), (1 << 8)));

    return GT_OK;
}

/**
* @internal mvHwsGPcs28nmSignalDetectMaskSet function
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
GT_STATUS mvHwsGPcs28nmSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 maskEn
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    pcsNum = pcsNum;
    maskEn = maskEn;

    /* we assume that PCS num is the correlated to port num*/
    return GT_NOT_SUPPORTED;
}

/**
* @internal mvHwsQsgmiiPcs28nmSignalDetectMaskSet function
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
GT_STATUS mvHwsQsgmiiPcs28nmSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 maskEn
)
{
    /* we assume that PCS num is the correlated to port num*/
    return mvHwsGPcs28nmSerdesSignalDetectMaskSet(devNum, portGroup, pcsNum, QSGMII, maskEn);
}

/**
* @internal mvHwsGPcs28nmTestGenCfg function
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
GT_STATUS mvHwsGPcs28nmTestGenCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
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
* @internal mvHwsGPcs28nmTestGenStatus function
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
GT_STATUS mvHwsGPcs28nmTestGenStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    MV_HWS_TEST_GEN_STATUS        *status
)
{
  GT_UREG_DATA data;

  pattern = pattern;

  if (status == 0)
  {
    return GT_BAD_PARAM;
  }

  CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, pcsNum, GIG_Port_PRBS_Status_Reg, &data, 0));
  status->checkerLock = data & 1; /* bit 0 */

  CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, pcsNum, GIG_Port_PRBS_Error_Cntr_Reg, &data, 0));
  status->totalErrors = data;

  status->goodFrames = 0;

  return GT_OK;
}

/**
* @internal mvHwsGPcs28nmActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0, if current PCS is disabled.
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
GT_STATUS mvHwsGPcs28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_UOPT                 *numOfLanes
)
{
    GT_UREG_DATA data;

    if (numOfLanes == NULL)
    {
      return GT_BAD_PARAM;
    }

    *numOfLanes = 0;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, pcsNum, GIG_PORT_MAC_CONTROL_REGISTER2, &data, 0));
    /* check unreset bit status */
    if (!((data >> 3) & 1))
    {
        /* GPCS is disabled */
        return GT_OK;
    }
    /* one active lane always */
    *numOfLanes = 1;

    return GT_OK;
}
#endif
