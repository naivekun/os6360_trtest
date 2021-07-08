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
* mvHwsXglMacRev2If.c
*
* DESCRIPTION:
*       XGL MAC revision 2 interface (for puma3)
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacRev2If.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "XLGMAC_Rev2";
}
/**
* @internal hwsXlgMacRev2IfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXlgMacRev2IfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{

    funcPtrArray[XLGMAC].macRestFunc = mvHwsXlgMacReset;
    funcPtrArray[XLGMAC].macLinkGetFunc = mvHwsXlgMacLinkStatus;
    funcPtrArray[XLGMAC].macLbCfgFunc = mvHwsXlgMacLoopbackCfg;
    funcPtrArray[XLGMAC].macTypeGetFunc = mvHwsMacTypeGet;
#ifndef CO_CPU_RUN
    funcPtrArray[XLGMAC].macLbStatusGetFunc = mvHwsXlgMacLoopbackStatusGet;
    funcPtrArray[XLGMAC].macActiveStatusFunc = mvHwsXlgMacActiveStatusGet;
#endif
    funcPtrArray[XLGMAC].macFcStateCfgFunc = NULL;

    /* specific function */
    funcPtrArray[XLGMAC].macModeCfgFunc = mvHwsXlgMacRev2ModeCfg;

    return GT_OK;
}


/**
* @internal mvHwsXlgMacRev2ModeCfg function
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
GT_STATUS mvHwsXlgMacRev2ModeCfg
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

    switch(curPortParams.numOfActLanes)
    {
    case 1:
      seq = &hwsXlgMacSeqDb[XLGMAC_MODE_1_Lane_SEQ];
      accessAddr  = baseAddr + MACRO_CONTROL + 8 * unitIndex;
      if (curPortParams.portMacNumber == 12)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 5), (1 << 5)));
      }
      if (curPortParams.portMacNumber == 14)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 7), (1 << 7)));
      }

      /*Rx XLG to RxDMA interface is 64 bits wide.*/
      accessAddr  = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER4 + curPortParams.portMacNumber * unitIndex;
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 9), (1 << 9)));
      break;
    case 2:
      seq = &hwsXlgMacSeqDb[XLGMAC_MODE_2_Lane_SEQ];
      accessAddr  = baseAddr + MACRO_CONTROL + 8 * unitIndex;
      if (curPortParams.portMacNumber == 12)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 5), (1 << 5)));
      }
      if (curPortParams.portMacNumber == 14)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 7), (1 << 7)));
      }

      /*Rx XLG to RxDMA interface is 64 bits wide.*/
      accessAddr  = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER4 + curPortParams.portMacNumber * unitIndex;
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 9), (1 << 9)));
      break;
    case 4:
      seq = &hwsXlgMacSeqDb[XLGMAC_MODE_4_Lane_SEQ];
      accessAddr  = baseAddr + MACRO_CONTROL + 8 * unitIndex;
      if (curPortParams.portMacNumber == 12)
      {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 5), (1 << 5)));
      }

      /*Rx XLG to RxDMA interface is 256 bits wide.*/
      accessAddr  = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER4 + curPortParams.portMacNumber * unitIndex;
      CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, 0, (1 << 9)));
      break;
    default:
        return GT_BAD_PARAM;
        break;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    /* keep in reset all unused MACs */
    for (i = 1; (i < curPortParams.numOfActLanes); i++)
    {
        if ((curPortParams.portMacNumber+i) < 12)
        {
            /* 0x010C0000 + 0x1000*p: where p (16-19) represents fabric port
               0x010C0000 + 0x1000*p: where p (0-11) represents network port */
            accessAddr = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER0 + (curPortParams.portMacNumber+i) * unitIndex;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, 0, (1 << 1)));
        }
    }
    return GT_OK;
}


