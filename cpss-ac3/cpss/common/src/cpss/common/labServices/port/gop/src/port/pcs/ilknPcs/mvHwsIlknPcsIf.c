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
* mvHwsIlknPcsIf.h
*
* DESCRIPTION:
*       InterLaken unit PCS interface API
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/ilknPcs/mvHwsIlknPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/ilknPcs/mvHwsIlknPcsDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

#ifndef CO_CPU_RUN
static char* mvHwsPcsTypeGetFunc(void)
{
  return "ILKN_PCS";
}
#endif

/**
* @internal mvHwsIlknPcsIfInit function
* @endinternal
*
* @brief   Init GPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIlknPcsIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[INTLKN_PCS].pcsResetFunc = mvHwsIlknPcsReset;
    funcPtrArray[INTLKN_PCS].pcsModeCfgFunc = mvHwsIlknPcsMode;
#ifndef CO_CPU_RUN
    funcPtrArray[INTLKN_PCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
#endif

    return GT_OK;
}

/**
* @internal mvHwsIlknPcsReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIlknPcsReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ *seq;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    if ((RESET == action) || (FULL_RESET == action))
    {
        seq = &hwsIlknPcsPscSeqDb[ILKNPCS_RESET_SEQ];
    }
    else if (UNRESET == action)
    {
        seq = &hwsIlknPcsPscSeqDb[ILKNPCS_UNRESET_SEQ];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;

}

/**
* @internal mvHwsIlknPcsMode function
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
GT_STATUS mvHwsIlknPcsMode
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  numOfLanes;
    GT_UREG_DATA regData;
    GT_UOPT xlgNum;
    MV_CFG_SEQ *seq;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numOfLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? 2 * curPortParams.numOfActLanes : curPortParams.numOfActLanes;

    if(numOfLanes == 4 || numOfLanes == 8)
    {
        /* for BC2, the mux value should be as follows
           TBD - should this function be separated to a diffeerent component */
        regData = 1;
        xlgNum = 64;
    }
    else
    {
        regData = 2;
        xlgNum = 16;
    }

    /* Set MUX to ILKN */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, xlgNum, EXTERNAL_CONTROL, regData, 3));

    seq = &hwsIlknPcsPscSeqDb[ILKNPCS_MODE_MISC_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));
    switch (numOfLanes)
    {
        case 4:
            seq = &hwsIlknPcsPscSeqDb[ILKNPCS_MODE_4_LANE_SEQ];
            break;
        case 8:
            seq = &hwsIlknPcsPscSeqDb[ILKNPCS_MODE_8_LANE_SEQ];
            break;
        case 12:
            seq = &hwsIlknPcsPscSeqDb[ILKNPCS_MODE_12_LANE_SEQ];
            break;
        case 16:
            seq = &hwsIlknPcsPscSeqDb[ILKNPCS_MODE_16_LANE_SEQ];
            break;
        case 24:
            seq = &hwsIlknPcsPscSeqDb[ILKNPCS_MODE_24_LANE_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;

}

