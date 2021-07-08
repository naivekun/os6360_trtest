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
* mvHwsXglMacIf.c
*
* DESCRIPTION:
*       XGL MAC reset interface
*
* FILE REVISION NUMBER:
*       $Revision: 30 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "XLGMAC";
}

/**
* @internal hwsXlgMacIfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXlgMacIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{

    funcPtrArray[XLGMAC].macRestFunc = mvHwsXlgMacReset;
    funcPtrArray[XLGMAC].macModeCfgFunc = mvHwsXlgMacModeCfg;
    funcPtrArray[XLGMAC].macLinkGetFunc = mvHwsXlgMacLinkStatus;
    funcPtrArray[XLGMAC].macLbCfgFunc = mvHwsXlgMacLoopbackCfg;
#ifndef CO_CPU_RUN
    funcPtrArray[XLGMAC].macLbStatusGetFunc = mvHwsXlgMacLoopbackStatusGet;
    funcPtrArray[XLGMAC].macActiveStatusFunc = mvHwsXlgMacActiveStatusGet;
#endif
    funcPtrArray[XLGMAC].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[XLGMAC].macFcStateCfgFunc = NULL;

    return GT_OK;
}


/**
* @internal mvHwsXlgMacReset function
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
GT_STATUS mvHwsXlgMacReset
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
        seq = &hwsXlgMacSeqDb[XLGMAC_RESET_SEQ];
        data = 0;
    }
    else if (UNRESET == action)
    {
        seq = &hwsXlgMacSeqDb[XLGMAC_UNRESET_SEQ];
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
* @internal mvHwsXlgMacModeCfg function
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
GT_STATUS mvHwsXlgMacModeCfg
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
    GT_U32 unitIndex, i;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));

    accessAddr  = baseAddr + MACRO_CONTROL + 8 * unitIndex;

    switch(curPortParams.numOfActLanes)
    {
    case 1:
      seq = &hwsXlgMacSeqDb[XLGMAC_MODE_1_Lane_SEQ];
      if (curPortParams.portMacNumber == 12)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 5), (1 << 5)));
      }
      if (curPortParams.portMacNumber == 14)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 7), (1 << 7)));
      }
      break;
    case 2:
      seq = &hwsXlgMacSeqDb[XLGMAC_MODE_2_Lane_SEQ];
      if (curPortParams.portMacNumber == 12)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 5), (1 << 5)));
      }
      if (curPortParams.portMacNumber == 14)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 7), (1 << 7)));
      }
      break;
    case 4:
      seq = &hwsXlgMacSeqDb[XLGMAC_MODE_4_Lane_SEQ];
      if (curPortParams.portMacNumber == 12)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 5), (1 << 5)));
      }
      break;
    default:
        return GT_BAD_PARAM;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    /* keep in reset all unused MACs */
    for (i = 1; (i < curPortParams.numOfActLanes); i++)
    {
        /*0x088C0000 + p * 0x1000: where p (14-14) represents Port Num
          0x088C0000 + p * 0x1000: where p (0-12) represents Port Num*/
        if (((curPortParams.portMacNumber+i) < 15) && ((curPortParams.portMacNumber+i) != 13))
        {
            accessAddr = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER0 + (curPortParams.portMacNumber+i) * unitIndex;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, 0, (1 << 1)));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsXlgMacLoopbackCfg function
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
GT_STATUS mvHwsXlgMacLoopbackCfg
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
      seq = &hwsXlgMacSeqDb[XLGMAC_LPBK_NORMAL_SEQ];
      break;
    case RX_2_TX_LB:
      seq = &hwsXlgMacSeqDb[XLGMAC_LPBK_RX2TX_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsXlgMacSeqDb[XLGMAC_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsXlgMacLoopbackStatusGet function
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
GT_STATUS mvHwsXlgMacLoopbackStatusGet
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
    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER1 + macNum * unitIndex;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    switch ((data >> 13) & 3)
    {
    case 0:
        *lbType = DISABLE_LB;
      break;
    case 1:
        *lbType = TX_2_RX_LB;
      break;
    case 2:
        *lbType = RX_2_TX_LB;
      break;
    default:
      return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}
#endif
/**
* @internal mvHwsXlgMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXlgMacLinkStatus
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
    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + MSM_PORT_STATUS + macNum * unitIndex;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    *linkStatus = data & 1;
    return GT_OK;
}

/**
* @internal mvHwsXlgMacModeSelectDefaultCfg function
* @endinternal
*
* @brief   MAC mode select default value configuration (needed for AP mechanism).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXlgMacModeSelectDefaultCfg
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                macNum
)
{
    /* change to genUnitRegisterSet since this functionality is also used for 28nm devices */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, macNum, MSM_PORT_MAC_CONTROL_REGISTER3, (1 << 13),(7 << 13)));

    return GT_OK;
}

/**
* @internal mvHwsXlgMacActiveStatusGet function
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
GT_STATUS mvHwsXlgMacActiveStatusGet
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

    accessAddr = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER0 + macNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    if (!((data >> 1) & 1))
    {
        /* unit is under RESET */
        return GT_OK;
    }

    *numOfLanes = 0;
    /* get number of active lanes */
    accessAddr = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER3 + macNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    switch ((data >> 13) & 7)
    {
    case 1:
        *numOfLanes = 1;
        break;
    case 2:
        *numOfLanes = 2;
        break;
    case 3:
        *numOfLanes = 4;
        break;
    case 7:
    case 0:
    default:
        *numOfLanes = 0;
        break;
    }

    return GT_OK;
}

