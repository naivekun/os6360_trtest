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
* @file mvHwsGeMacIf.c
*
* @brief GE MAC interface
*
* @version   17
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "GEMAC_X";
}

/**
* @internal hwsGeMacIfInit function
* @endinternal
*
* @brief   Init GE MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsGeMacIfInit(GT_U8 devNum, MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    CHECK_STATUS(hwsGeMacSeqInit(devNum));

    funcPtrArray[GEMAC_X].macRestFunc = mvHwsGeMacReset;
    funcPtrArray[GEMAC_X].macModeCfgFunc = mvHwsGeMacModeCfg;
    funcPtrArray[GEMAC_X].macLinkGetFunc = mvHwsGeMacLinkStatus;
    funcPtrArray[GEMAC_X].macLbCfgFunc = mvHwsGeMacLoopbackCfg;
#ifndef CO_CPU_RUN
    funcPtrArray[GEMAC_X].macLbStatusGetFunc = mvHwsGeMacLoopbackStatusGet;
    funcPtrArray[GEMAC_X].macActiveStatusFunc = mvHwsGeMacActiveStatusGet;
#endif
    funcPtrArray[GEMAC_X].macTypeGetFunc = mvHwsMacTypeGet;

    return GT_OK;
}

/**
* @internal mvHwsGeMacReset function
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
GT_STATUS mvHwsGeMacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ *seq;
    GT_U32 accessAddr;
    GT_U32 baseAddr;
    GT_U32 unitIndex;
    GT_U32 data;
    portMode = portMode;

    if ((RESET == action) || (FULL_RESET == action))
    {
        seq = &hwsGeMacSeqDb[GEMAC_RESET_SEQ];
        data = 0;
    }
    else if (UNRESET == action)
    {
        seq = &hwsGeMacSeqDb[GEMAC_UNRESET_SEQ];
        data = 1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + MACRO_CONTROL + 8 * unitIndex;

    if (portMacNum == 12)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (data << 5), (1 << 5)));
    }
    if (portMacNum == 14)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (data << 7), (1 << 7)));
    }

    return GT_OK;
}

/**
* @internal mvHwsGeMacModeCfg function
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
GT_STATUS mvHwsGeMacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;
    GT_U32 accessAddr;
    GT_U32 baseAddr;
    GT_U32 unitIndex;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + MACRO_CONTROL + 8 * unitIndex;
    if (curPortParams.portMacNumber == 12)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 5), (1 << 5)));
    }
    if (curPortParams.portMacNumber == 14)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 7), (1 << 7)));
    }

    seq = &hwsGeMacSeqDb[GEMAC_X_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsGeMacLoopbackCfg function
* @endinternal
*
* @brief   Configure MAC loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMacLoopbackCfg
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
      seq = &hwsGeMacSeqDb[GEMAC_LPBK_NORMAL_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsGeMacSeqDb[GEMAC_LPBK_TX2RX_SEQ];
      break;
    case RX_2_TX_LB:
    default:
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsGeMacLoopbackStatusGet function
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
GT_STATUS mvHwsGeMacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 data;

    portMode = portMode;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_PORT_MAC_CONTROL_REGISTER1, &data, 0));

    *lbType = ((data >> 5) & 1) ? TX_2_RX_LB : DISABLE_LB;

    return GT_OK;
}
#endif

/**
* @internal mvHwsGeMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMacLinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    GT_U32 data;

    portMode = portMode;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_Port_Status_Reg, &data, 0));

    *linkStatus = data & 1;

    return GT_OK;
}

/**
* @internal mvHwsGeMacActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMacActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
)
{
    GT_U32 data;

    portMode = portMode;
    if (numOfLanes == NULL)
    {
        return GT_BAD_PARAM;
    }

    *numOfLanes = 0;
    /* port MAC type bit 1 */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_PORT_MAC_CONTROL_REGISTER0, &data, 0));
    if (((data >> 1) & 1) != 1)
    {
        /* 0x1 = 1000Base-X_mode */
        return GT_OK;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_PORT_MAC_CONTROL_REGISTER2, &data, 0));
    if ((data >> 6) & 1)
    {
        /* unit is under RESET */
        return GT_OK;
    }
    *numOfLanes = 1;

    return GT_OK;
}
