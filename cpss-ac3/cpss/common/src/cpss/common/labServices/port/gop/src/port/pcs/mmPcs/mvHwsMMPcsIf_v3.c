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
* @file mvHwsMMPcsIf_v3.c
*
* @brief MMPCS V3.0 interface API for puma3B0
*
* @version   10
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsV2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsV3If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDbRev3.h>

#ifdef RUN_PCS_DEBUG
#include <common/os/gtOs.h>
#define DEBUG_PCS(s)              hwsOsPrintf s
#else
#define DEBUG_PCS(s)
#endif



static char* mvHwsPcsTypeGetFunc(void)
{
    return "MMPCS V3.0";
}

static  GT_U32 indArr[4] = {0, 0x400, 0x600, 0x800};

static GT_U32 mmPcsFabWa = 10;

/**
* @internal mvHwsMMPcsV3IfInit function
* @endinternal
*
* @brief   Init MMPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsV3IfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[MMPCS].pcsLbCfgFunc = mvHwsMMPcsLoopBack;
    funcPtrArray[MMPCS].pcsTestGenFunc = mvHwsMMPcsTestGenCfg;
    funcPtrArray[MMPCS].pcsTestGenStatusFunc = mvHwsMMPcsTestGenStatus;
    funcPtrArray[MMPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
    funcPtrArray[MMPCS].pcsFecCfgFunc =     mvHwsMMPcsFecConfig;
    funcPtrArray[MMPCS].pcsFecCfgGetFunc =  mvHwsMMPcsFecConfigGet;
    funcPtrArray[MMPCS].pcsActiveStatusGetFunc = mvHwsMMPcsActiveStatusGet;

    funcPtrArray[MMPCS].pcsRxResetFunc = mvHwsMMPcsV2RxReset;
    funcPtrArray[MMPCS].pcsExtPllCfgFunc = mvMmPcsV2ExtPllCfg;
    funcPtrArray[MMPCS].pcsCheckGearBoxFunc = mvHwsMMPcsV2CheckGearBox;

    funcPtrArray[MMPCS].pcsResetFunc = mvHwsMMPcsV3Reset;
    funcPtrArray[MMPCS].pcsModeCfgFunc = mvHwsMMPcsV3Mode;
    funcPtrArray[MMPCS].pcsSignalDetectMaskEn = mvHwsMMPcsV3SignalDetectMaskSet;

    return GT_OK;
}

/**
* @internal mvHwsMMPcsV3Reset function
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
GT_STATUS mvHwsMMPcsV3Reset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    GT_U32 regAddr;
    GT_U32 data;
    GT_U32 countVal;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    MV_CFG_SEQ *seq;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    /* read PCS mode */
    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    if ((RESET == action) || (FULL_RESET == action))
    {
        if (pcsNum < 16)
        {
            switch(countVal)
            {
            case 1:
                seq = &hwsMmPscSeqDbRev3[MMPCS_RESET_1_LANE_SEQ];
                break;
            case 2:
                seq = &hwsMmPscSeqDbRev3[MMPCS_RESET_2_LANE_SEQ];
                break;
            case 4:
                seq = &hwsMmPscSeqDbRev3[MMPCS_RESET_4_LANE_SEQ];
                break;
            default:
                return GT_BAD_PARAM;
            }
        }
        else
        {
            switch(countVal)
            {
            case 1:
                seq = &hwsMmPscSeqDbRev3[MMPCS_RESET_1_LANE_SEQ];
                break;
            case 2:
                seq = &hwsMmPscSeqDbRev3[MMPCS_FABRIC_RESET_2_LANE_SEQ];
                break;
            case 4:
                seq = &hwsMmPscSeqDbRev3[MMPCS_FABRIC_RESET_4_LANE_SEQ];
                break;
            default:
                return GT_BAD_PARAM;
            }
        }
    }/* RESET action */
    else if (UNRESET == action)
    {
        if (pcsNum < 16)
        {
            switch(countVal)
            {
            case 1:
                seq = &hwsMmPscSeqDbRev3[MMPCS_UNRESET_1_LANE_SEQ];
                break;
            case 2:
                seq = &hwsMmPscSeqDbRev3[MMPCS_UNRESET_2_LANE_SEQ];
                break;
            case 4:
                seq = &hwsMmPscSeqDbRev3[MMPCS_UNRESET_4_LANE_SEQ];
                break;
            default:
                return GT_BAD_PARAM;
            }
        }
        else
        {
            switch(countVal)
            {
            case 1:
                seq = &hwsMmPscSeqDbRev3[MMPCS_UNRESET_1_LANE_SEQ];
                break;
            case 2:
                seq = &hwsMmPscSeqDbRev3[MMPCS_FABRIC_UNRESET_2_LANE_SEQ];
                break;
            case 4:
                seq = &hwsMmPscSeqDbRev3[MMPCS_FABRIC_UNRESET_4_LANE_SEQ];
                break;
            default:
                return GT_BAD_PARAM;
            }
        }
    }/* UNRESET action */
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize));

    if ((UNRESET == action) && (pcsNum >= 16) && (countVal == 4))
    {
        GT_U32 i;

        /* read interrupts */
        regAddr = PCS40G_COMMON_INTERRUPT_CAUSE + unitAddr + pcsNum * unitIndex;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
        for (i = 0; i < mmPcsFabWa; i++)
        {
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
            if (data & (1 << 5))
            {
                DEBUG_PCS(("\nRun MMPCS WA on device %d, pcs %d (i = %d).", devNum, pcsNum, i));
                /* reset / unreset */
                CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, hwsMmPscSeqDbRev3[MMPCS_FABRIC_RESET_4_LANE_SEQ].cfgSeq,
                                          hwsMmPscSeqDbRev3[MMPCS_FABRIC_RESET_4_LANE_SEQ].cfgSeqSize));
                hwsOsExactDelayPtr(devNum, portGroup, 1);
                CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, hwsMmPscSeqDbRev3[MMPCS_FABRIC_UNRESET_4_LANE_SEQ].cfgSeq,
                                          hwsMmPscSeqDbRev3[MMPCS_FABRIC_UNRESET_4_LANE_SEQ].cfgSeqSize));
                hwsOsExactDelayPtr(devNum, portGroup, 1);
                continue;
            }
            else
            {
                break;
            }
        }

        /* unmask tx_gb_fifo_full interrupt on lane 0 only */
        regAddr = PCS40G_COMMON_INTERRUPT_MASK + unitAddr + pcsNum * unitIndex;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, (1 << 5), (1 << 5)));
        if (data & (1 << 5))
        {
            return GT_FAIL;
        }
    }

    return GT_OK;
}


/**
* @internal mvHwsMMPcsV3Mode function
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
GT_STATUS mvHwsMMPcsV3Mode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  numOfLanes;
    MV_CFG_SEQ *seq;
    GT_U32 regAddr;
    GT_U32 data;
    GT_U32 mask;
    GT_U32 i;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numOfLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? 2 * curPortParams.numOfActLanes : curPortParams.numOfActLanes;

    /* EXTERNAL_CONTROL - Select recovered clock 0 per pair of cores */
    mvUnitInfoGet(devNum, XLGMAC_UNIT, &unitAddr, &unitIndex);
    regAddr = unitAddr + unitIndex * curPortParams.portPcsNumber + EXTERNAL_CONTROL;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, 3));

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    /* clear register used to prevent WA function execution */
    regAddr = FEC_DEC_DMA_WR_DATA + unitAddr + curPortParams.portPcsNumber * unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, 0));

    if ((curPortParams.portPcsNumber >= 16) && (numOfLanes == 4))
    {
        /* mask tx_gb_fifo_full interrupt on lane 0 only */
        regAddr = PCS40G_COMMON_INTERRUPT_MASK + unitAddr + curPortParams.portPcsNumber * unitIndex;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, (1 << 5)));
        /* clear interrupts */
        regAddr = PCS40G_COMMON_INTERRUPT_CAUSE + unitAddr + curPortParams.portPcsNumber * unitIndex;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    }
    /* config FEC */
    for (i = 0; i < numOfLanes; i++)
    {
        if (curPortParams.portPcsNumber < 16)
        {
            /* access to network PCS */
            regAddr = PCS40G_COMMON_CONTROL + unitAddr + (curPortParams.portPcsNumber+i) * unitIndex;
        }
        else
        {
            /* access to fabric PCS */
            regAddr = PCS40G_COMMON_CONTROL + unitAddr + (curPortParams.portPcsNumber * unitIndex) + indArr[i];
        }
        data = (curPortParams.portFecMode == FC_FEC) ? (1 << 10) : 0;
        mask = (1 << 10) + (1 << 7); /* always set bit 7 to 0; */
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, mask));
    }

    /* reduce the probability of the link loss to about once in ~1000 times */
    regAddr = CHANNEL0_RX_GB_FIFO_CONFIGURATION + unitAddr + curPortParams.portPcsNumber * unitIndex;
    for (i = 0; i < 4; i++)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr + i*4, 0x18, 0));
    }

    if (curPortParams.portPcsNumber < 16)
    {
        switch (numOfLanes)
        {
        case 1:
            seq = &hwsMmPscSeqDbRev3[MMPCS_MODE_1_LANE_SEQ];
            break;
        case 2:
            seq = &hwsMmPscSeqDbRev3[MMPCS_MODE_2_LANE_SEQ];
            break;
        case 4:
            seq = &hwsMmPscSeqDbRev3[MMPCS_MODE_4_LANE_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
        }
    }
    else
    {
        switch (numOfLanes)
        {
        case 1:
            seq = &hwsMmPscSeqDbRev3[MMPCS_FABRIC_MODE_1_LANE_SEQ];
            break;
        case 2:
            seq = &hwsMmPscSeqDbRev3[MMPCS_FABRIC_MODE_2_LANE_SEQ];
            break;
        case 4:
            seq = &hwsMmPscSeqDbRev3[MMPCS_FABRIC_MODE_4_LANE_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
        }
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}


/**
* @internal mvHwsMMPcsV3SignalDetectMaskSet function
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
GT_STATUS mvHwsMMPcsV3SignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 maskEn
)
{
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 countVal, i;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum*unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    for (i = 0; i < countVal; i++)
    {
        if (pcsNum < 16)
        {
            /* access to network PCS */
            regAddr = CHANNEL_CONFIGURATION + unitAddr + (pcsNum + i)*unitIndex;
        }
        else
        {
            /* access to fabric PCS */
            regAddr = CHANNEL_CONFIGURATION + unitAddr + (pcsNum * unitIndex) + indArr[i];
        }
        data = (maskEn == GT_TRUE) ? (1 << 14) : 0;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, (1 << 14)));
    }

    return GT_OK;
}

GT_STATUS mvHwsMmPcs40GBackWa
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 countVal;
    GT_U32 gearVal;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = FEC_DEC_DMA_WR_DATA + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    if (data != 0x8000)
    {
        /* unit not initalize or under TRx training, delete */
        DEBUG_PCS(("\nRun mvHwsMmPcs40GBackWa PORT NOT CREATED on device %d, pcs %d", devNum, pcsNum));
        return GT_OK;
    }
    hwsOsTimerWkFuncPtr(2);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    if (countVal != 4)
    {
        /* not 40G */
        DEBUG_PCS(("\nRun mvHwsMmPcs40GBackWa PORT NOT CREATED (4) on device %d, pcs %d", devNum, pcsNum));
        return GT_OK;
    }

    DEBUG_PCS(("\nRun mvHwsMmPcs40GBackWa on device %d, pcs %d", devNum, pcsNum));
    /* Check Gear Box Status */
    regAddr = unitAddr + pcsNum*unitIndex + GEARBOX_STATUS;
    hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);
    gearVal = (data >> 1) & 0xF;
    if (gearVal != 0xF)
    {
        DEBUG_PCS(("\nCheck Gear Box Status on device %d, pcs %d (data 0x%x)", devNum, pcsNum, data));
        if (gearVal == 0)
        {
            /* no connector in port */
            return GT_OK;
        }
        else
        {
          return GT_FAIL;
        }
    }
    else
    {
        regAddr = PCS40G_COMMON_STATUS + unitAddr + pcsNum*unitIndex;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
        if (!(data & 1))
        {
            DEBUG_PCS(("\nRun RESET on device %d, pcs %d", devNum, pcsNum));
            /* one SD sq_detect = 1 perform pcs reset sequence */
            mvHwsMMPcsV3Reset(devNum,portGroup,portMacNum,portMode,RESET);
            hwsOsTimerWkFuncPtr(1);
            mvHwsMMPcsV3Reset(devNum,portGroup,portMacNum,portMode,UNRESET);
        }
    }

    return GT_OK;
}


