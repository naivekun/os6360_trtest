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
* @file mvHwsHglMacRev2If.c
*
* @brief HGL MAC interface revision 2 (for puma3)
*
* @version   3
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacRev2If.h>
#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacDb.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "HGLMAC_Rev2";
}

/**
* @internal hwsHglMacRev2IfInit function
* @endinternal
*
* @brief   Init HGL MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsHglMacRev2IfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[HGLMAC].macRestFunc = mvHwsHglMacReset;
    funcPtrArray[HGLMAC].macLinkGetFunc = mvHwsHglMacLinkStatus;
    funcPtrArray[HGLMAC].macLbCfgFunc = mvHwsHglMacLoopbackCfg;
    funcPtrArray[HGLMAC].macTypeGetFunc = mvHwsMacTypeGet;
#ifndef CO_CPU_RUN
    funcPtrArray[HGLMAC].macLbStatusGetFunc = mvHwsHglMacLoopbackStatusGet;
    funcPtrArray[HGLMAC].macActiveStatusFunc = mvHwsHglMacActiveStatusGet;
#endif
    funcPtrArray[HGLMAC].macFcStateCfgFunc = NULL;

    /* specific function */
    funcPtrArray[HGLMAC].macModeCfgFunc = mvHwsHglMacRev2ModeCfg;

    return GT_OK;
}

/**
* @internal mvHwsHglMacRev2ModeCfg function
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
GT_STATUS mvHwsHglMacRev2ModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams ;
    MV_CFG_SEQ *seq;
    GT_U32 baseAddr;
    GT_U32 unitIndex;
    GT_U32 accessAddr;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    seq = &hwsHglMacSeqDb[HGLMAC_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    /* Rx XLG to RxDMA interface is 64 bits wide.*/
    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER4 + curPortParams.portMacNumber * unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 9), (1 << 9)));

    return GT_OK;
}

