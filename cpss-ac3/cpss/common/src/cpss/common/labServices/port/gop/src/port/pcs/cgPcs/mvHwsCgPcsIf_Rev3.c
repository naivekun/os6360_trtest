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
* @file mvHwsCgPcsIf_Rev3.c
*
* @brief CGPCS interface API
*
* @version   2
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcsRev3If.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcsDb.h>


static char* mvHwsPcsTypeGetFunc(void)
{
  return "CGPCS Rev3";
}

/**
* @internal mvHwsCgPcsRev3IfInit function
* @endinternal
*
* @brief   Init GPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcsRev3IfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{

    funcPtrArray[CGPCS].pcsResetFunc = NULL; /* all functionality moved to CG MAC */
    funcPtrArray[CGPCS].pcsModeCfgFunc = mvHwsCgPcsMode;
    funcPtrArray[CGPCS].pcsLbCfgFunc = mvHwsCgPcsLoopBack;
    funcPtrArray[CGPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
    funcPtrArray[CGPCS].pcsFecCfgFunc =     (MV_HWS_PCS_FEC_CFG_FUNC_PTR)mvHwsCgPcsFecConfig;
    funcPtrArray[CGPCS].pcsFecCfgGetFunc =  (MV_HWS_PCS_FEC_CFG_GET_FUNC_PTR)mvHwsCgPcsFecConfigGet;

    funcPtrArray[CGPCS].pcsSignalDetectMaskEn = mvHwsCgPcsSignalDetectMaskRev3Set;

    return GT_OK;
}

/**
* @internal mvHwsCgPcsSignalDetectMaskRev3Set function
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
GT_STATUS mvHwsCgPcsSignalDetectMaskRev3Set
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 maskEn
)
{
    GT_U32 regAddr, data, mask;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    mvUnitInfoGet(devNum, CG_UNIT, &unitAddr, &unitIndex);

    regAddr = CG_CONTROL_0 + unitAddr + pcsNum*unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    if (((data >> 22) & 1) == 1)
    {
        /* supports 12 lanes */
        data = (maskEn == GT_TRUE) ? (0xFFF << 6) : 0;
        mask = 0xFFF << 6;
    }
    else
    {
        /* supports 10 lanes */
        data = (maskEn == GT_TRUE) ? (0x3FF << 6) : 0;
        mask = 0xFFF << 6;
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, mask));

    return GT_OK;
}
