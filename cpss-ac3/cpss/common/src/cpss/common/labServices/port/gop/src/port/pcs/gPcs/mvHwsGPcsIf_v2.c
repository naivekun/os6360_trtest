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
* mvHwsGPcsIf.c
*
* DESCRIPTION:
*       GPCS interface API
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf_v2.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsDb.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsPcsTypeGetFunc(void)
{
  return "GPCS_Rev2";
}

/**
* @internal mvHwsGPcsRev2IfInit function
* @endinternal
*
* @brief   Init GPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGPcsRev2IfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{

    funcPtrArray[GPCS].pcsResetFunc = NULL;
    funcPtrArray[GPCS].pcsLbCfgFunc = mvHwsGPcsLoopBack;
    funcPtrArray[GPCS].pcsTestGenFunc = mvHwsGPcsTestGenCfg;
    funcPtrArray[GPCS].pcsTestGenStatusFunc = mvHwsGPcsTestGenStatus;
    funcPtrArray[GPCS].pcsActiveStatusGetFunc = mvHwsGPcsActiveStatusGet;
    funcPtrArray[GPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;

    funcPtrArray[GPCS].pcsModeCfgFunc = mvHwsGPcsRev2ModeCfg;

    return GT_OK;
}

/**
* @internal mvHwsGPcsRev2ModeCfg function
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
GT_STATUS mvHwsGPcsRev2ModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 accessAddr;
    GT_U32 baseAddr;
    GT_U32 unitIndex;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* EXTERNAL_CONTROL - Select recovered clock 0 per pair of cores */
    mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex);
    accessAddr = baseAddr + unitIndex * curPortParams.portPcsNumber + EXTERNAL_CONTROL;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, 0, 3));

    return GT_OK;
}
