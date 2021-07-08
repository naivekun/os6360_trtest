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
* @file mvHwsGeMacSgIf.c
*
* @brief GE MAC interface
*
* @version   11
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "GEMAC_SG";
}
static GT_BOOL hwsInitDone = GT_FALSE;

extern GT_STATUS mvHwsGeMacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
);

GT_STATUS mvHwsGeNetMacSgReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

/**
* @internal hwsGeMacSgIfInit function
* @endinternal
*
* @brief   Init GE MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsGeMacSgIfInit(GT_U8 devNum, MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    if (hwsInitDone == GT_FALSE)
    {
        /* init sequences DB */
        CHECK_STATUS(hwsGeMacSgSeqInit(devNum));
    }

    funcPtrArray[GEMAC_SG].macRestFunc = mvHwsGeMacSgReset;
    funcPtrArray[GEMAC_SG].macModeCfgFunc = mvHwsGeMacSgModeCfg;
    funcPtrArray[GEMAC_SG].macLinkGetFunc = mvHwsGeMacSgLinkStatus;
    funcPtrArray[GEMAC_SG].macLbCfgFunc = mvHwsGeMacSgLoopbackCfg;
    funcPtrArray[GEMAC_SG].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[GEMAC_SG].macFcStateCfgFunc = NULL;

    funcPtrArray[GEMAC_NET_SG].macRestFunc = mvHwsGeNetMacSgReset;       /* Network port implementation */
    funcPtrArray[GEMAC_NET_SG].macModeCfgFunc = mvHwsGeNetMacSgModeCfg;  /* Network port implementation */
    funcPtrArray[GEMAC_NET_SG].macLinkGetFunc = mvHwsGeMacSgLinkStatus;
    funcPtrArray[GEMAC_NET_SG].macLbCfgFunc = mvHwsGeMacSgLoopbackCfg;
    funcPtrArray[GEMAC_NET_SG].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[GEMAC_NET_SG].macFcStateCfgFunc = NULL;
#ifndef CO_CPU_RUN
    funcPtrArray[GEMAC_SG].macActiveStatusFunc = mvHwsGeMacSgActiveStatusGet;
    funcPtrArray[GEMAC_SG].macLbStatusGetFunc = mvHwsGeMacLoopbackStatusGet;

    funcPtrArray[GEMAC_NET_SG].macActiveStatusFunc = mvHwsGeMacSgActiveStatusGet;
    funcPtrArray[GEMAC_NET_SG].macLbStatusGetFunc = mvHwsGeMacLoopbackStatusGet;
#endif
    hwsInitDone = GT_TRUE;

    return GT_OK;
}

/**
* @internal mvHwsGeMacSgReset function
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
GT_STATUS mvHwsGeMacSgReset
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

    if((HWS_DEV_SILICON_TYPE(devNum) != BobcatA0) &&
                                (HWS_DEV_SILICON_TYPE(devNum) != Alleycat3A0))
    {
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
    }

    return GT_OK;
}

/**
* @internal mvHwsGeMacSgModeCfg function
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
GT_STATUS mvHwsGeMacSgModeCfg
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

    if((HWS_DEV_SILICON_TYPE(devNum) != BobcatA0) &&
                                (HWS_DEV_SILICON_TYPE(devNum) != Alleycat3A0))
    {
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
    }

    seq = &hwsGeMacSeqDb[GEMAC_SG_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsGeMacSgLoopbackCfg function
* @endinternal
*
* @brief   .
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMacSgLoopbackCfg
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
    case RX_2_TX_LB:
      seq = &hwsGeMacSeqDb[GEMAC_LPBK_RX2TX_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsGeMacSeqDb[GEMAC_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsGeMacSgLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMacSgLinkStatus
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
    CHECK_STATUS(mvUnitExtInfoGet(devNum, GEMAC_UNIT, macNum, &baseAddr, &unitIndex, &macNum));

    accessAddr = baseAddr + GIG_Port_Status_Reg + macNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    *linkStatus = data & 1;

    return GT_OK;
}

/**
* @internal mvHwsGeMacSgActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*                                      macType   - MAC type according to port configuration
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMacSgActiveStatusGet
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

    CHECK_STATUS(mvUnitExtInfoGet(devNum, GEMAC_UNIT, macNum, &baseAddr, &unitIndex, &macNum));

    *numOfLanes = 0;
    accessAddr = baseAddr + GIG_PORT_MAC_CONTROL_REGISTER0 + macNum * unitIndex;
    /* port MAC type bit 1 */
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    if (((data >> 1) & 1) != 0)
    {
        /* 0x0 = SGMII */
        return GT_OK;
    }

    accessAddr = baseAddr + GIG_PORT_MAC_CONTROL_REGISTER2 + macNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    if ((data >> 6) & 1)
    {
        /* unit is under RESET */
        return GT_OK;
    }
    *numOfLanes = 1;

    return GT_OK;
}

/**
* @internal mvHwsGeNetMacSgReset function
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
GT_STATUS mvHwsGeNetMacSgReset
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
        seq = &hwsGeMacSeqDb[GEMAC_RESET_SEQ];
    }
    else if (UNRESET == action)
    {
        seq = &hwsGeMacSeqDb[GEMAC_UNRESET_SEQ];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsGeNetMacSgModeCfg function
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
GT_STATUS mvHwsGeNetMacSgModeCfg
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

    seq = &hwsGeMacSeqDb[GEMAC_NET_SG_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

