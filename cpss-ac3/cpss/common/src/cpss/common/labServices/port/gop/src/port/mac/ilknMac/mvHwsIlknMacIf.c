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
* @file mvHwsIlknMacIf.c
*
* @brief CG MAC reset interface
*
* @version   18
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/ilknMac/mvHwsIlknMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/ilknMac/mvHwsIlknMacDb.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

#ifndef CO_CPU_RUN
static char* mvHwsMacTypeGet(void)
{
  return "ILKN_MAC";
}
#endif

/**
* @internal hwsIlknMacIfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsIlknMacIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[INTLKN_MAC].macRestFunc = mvHwsIlknMacReset;
    funcPtrArray[INTLKN_MAC].macModeCfgFunc = mvHwsIlknMacModeCfg;
    funcPtrArray[INTLKN_MAC].macLinkGetFunc = mvHwsIlknMacLinkStatus;
#ifndef CO_CPU_RUN
    funcPtrArray[INTLKN_MAC].macLbCfgFunc = mvHwsIlknMacLoopbackCfg;
    funcPtrArray[INTLKN_MAC].macLbStatusGetFunc = mvHwsIlknMacLoopbackStatusGet;
    funcPtrArray[INTLKN_MAC].macTypeGetFunc = mvHwsMacTypeGet;
#endif
    funcPtrArray[INTLKN_MAC].macFcStateCfgFunc = NULL;

    return GT_OK;
}


/**
* @internal mvHwsIlknMacReset function
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
GT_STATUS mvHwsIlknMacReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ *seq;
    portMode = portMode;

    if ((RESET == action) || (FULL_RESET == action))
    {
        seq = &hwsIlknMacSeqDb[INTLKN_MAC_RESET_SEQ];
    }
    else if (UNRESET == action)
    {
        seq = &hwsIlknMacSeqDb[INTLKN_MAC_UNRESET_SEQ];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsIlknMacModeCfg function
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
GT_STATUS mvHwsIlknMacModeCfg
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

    switch(curPortParams.numOfActLanes)
    {
    case 4:
        seq = &hwsIlknMacSeqDb[INTLKN_MAC_MODE_4_LANES_SEQ];
        break;
    case 8:
        seq = &hwsIlknMacSeqDb[INTLKN_MAC_MODE_8_LANES_SEQ];
        break;
    case 12:
        seq = &hwsIlknMacSeqDb[INTLKN_MAC_MODE_12_LANES_SEQ];
        break;
    case 16:
        seq = &hwsIlknMacSeqDb[INTLKN_MAC_MODE_16_LANES_SEQ];
        break;
    case 24:
        seq = &hwsIlknMacSeqDb[INTLKN_MAC_MODE_24_LANES_SEQ];
        break;
    default:
        return GT_BAD_PARAM;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    if (HWS_DEV_SILICON_TYPE(devNum) == Puma3B0)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, INTLKN_UNIT, curPortParams.portMacNumber, TXDMA_converter_control_0, (1 << 15), (1 << 15)));

        /* set the field in the RxDMA converter packet size restriction register
          (WA to avoid packets corruption Puma3  FE-820601) */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, INTLKN_UNIT, curPortParams.portMacNumber, 0x4C, (0x3FFF << 14), (0x3FFF << 14)));
    }

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsIlknMacLoopbackCfg function
* @endinternal
*
* @brief   .
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIlknMacLoopbackCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_CFG_SEQ *seq;

    portMode = portMode;

    switch (lbType)
    {
    case DISABLE_LB:
      seq = &hwsIlknMacSeqDb[INTLKN_MAC_LPBK_NORMAL_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsIlknMacSeqDb[INTLKN_MAC_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsIlknMacLoopbackStatusGet function
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
GT_STATUS mvHwsIlknMacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_UREG_DATA data;

    portMode = portMode;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, INTLKN_UNIT, macNum, RXDMA_converter_control_0, &data, 0));

    switch ((data >> 5) & 3)
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
#endif

/**
* @internal mvHwsIlknMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIlknMacLinkStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    GT_UREG_DATA data;
    MV_HWS_UNITS_ID unitId;

    portMode = portMode;
    /* check if unit not in RESET */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, INTLKN_UNIT, macNum, ILKN_RESETS, &data, 0));

    if (((data >> 1) & 1) == 0)
    {
        /* unit under reset */
        *linkStatus = GT_FALSE;
        return GT_OK;
    }

    /* get link status */
    if ((HWS_DEV_SILICON_TYPE(devNum) == Puma3A0) || (HWS_DEV_SILICON_TYPE(devNum) == Puma3B0))
    {
        unitId = INTLKN_UNIT;
    }
    else
    {
        unitId = INTLKN_RF_UNIT;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, unitId, macNum, ILKN_0_ALIGN_STATUS, &data, 0));

    *linkStatus = (((data >> 1) & 3) == 3) ? GT_TRUE : GT_FALSE;

    if (*linkStatus)
    {
        return GT_OK;
    }
    return GT_OK;
}

