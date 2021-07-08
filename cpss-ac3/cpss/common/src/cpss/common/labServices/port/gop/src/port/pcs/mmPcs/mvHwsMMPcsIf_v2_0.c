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
* @file mvHwsMMPcsIf_v2_0.c
*
* @brief MMPCS V2.0 interface API
*
* @version   31
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsV2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>


#define SYNC_CHANGED_INTERRUPT     0x6


static char* mvHwsPcsTypeGetFunc(void)
{
    return "MMPCS V2.0";
}

static  GT_U32 indArr[4] = {0, 0x400, 0x600, 0x800};

/**
* @internal mvHwsMMPcsV2IfInit function
* @endinternal
*
* @brief   Init MMPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsV2IfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[MMPCS].pcsResetFunc = mvHwsMMPcsV2Reset;
    funcPtrArray[MMPCS].pcsModeCfgFunc = mvHwsMMPcsV2Mode;
    funcPtrArray[MMPCS].pcsRxResetFunc = mvHwsMMPcsV2RxReset;
    funcPtrArray[MMPCS].pcsExtPllCfgFunc = mvMmPcsV2ExtPllCfg;
    funcPtrArray[MMPCS].pcsSignalDetectMaskEn = mvHwsMMPcsV2SignalDetectMaskSet;
    funcPtrArray[MMPCS].pcsCheckGearBoxFunc = mvHwsMMPcsV2CheckGearBox;

    funcPtrArray[MMPCS].pcsLbCfgFunc = mvHwsMMPcsLoopBack;
    funcPtrArray[MMPCS].pcsTestGenFunc = mvHwsMMPcsTestGenCfg;
    funcPtrArray[MMPCS].pcsTestGenStatusFunc = mvHwsMMPcsTestGenStatus;
    funcPtrArray[MMPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
    funcPtrArray[MMPCS].pcsFecCfgFunc =     mvHwsMMPcsFecConfig;
    funcPtrArray[MMPCS].pcsFecCfgGetFunc =  mvHwsMMPcsFecConfigGet;
    funcPtrArray[MMPCS].pcsActiveStatusGetFunc = mvHwsMMPcsActiveStatusGet;

    return GT_OK;
}

static GT_STATUS mvHwsMMPcsV2Unreset
(
  GT_U8  devNum,
  GT_U32 portGroup,
  GT_U32 pcsNum,
  GT_U32 countVal
)
{
  GT_U32 regAddr;
  GT_U32 i;
  GT_U32 unitAddr;
  GT_U32 unitIndex;

  mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

  for (i = 0; i < countVal; i++)
  {
    if (pcsNum < 16)
    {
      /* access to network PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    }
    else
    {
      /* access to fabric PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 1, 1));
  }
  for (i = 0; i < countVal; i++)
  {
    if (pcsNum < 16)
    {
      /* access to network PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    }
    else
    {
      /* access to fabric PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x2, 0x2));
  }
  for (i = 0; i < countVal; i++)
  {
    if (pcsNum < 16)
    {
      /* access to network PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    }
    else
    {
      /* access to fabric PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
    }
    /* only MMPCS 0 are relevant; other lanes keep reset set 0 */
    if (i == 0)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x4, 0x4));
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x0, 0x4));
    }
  }
  for (i = 0; i < countVal; i++)
  {
    if (pcsNum < 16)
    {
      /* access to network PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    }
    else
    {
      /* access to fabric PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
    }
    /* only MMPCS 0 are relevant; other lanes keep reset set 0 */
    if (i == 0)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x20, 0x20));
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x0, 0x20));
    }
  }
  for (i = 0; i < countVal; i++)
  {
    if (pcsNum < 16)
    {
      /* access to network PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    }
    else
    {
      /* access to fabric PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
    }
    /* only MMPCS 0 are relevant; other lanes keep reset set 0 */
    if (i == 0)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x40, 0x40));
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x0, 0x40));
    }
  }
  for (i = 0; i < countVal; i++)
  {
    if (pcsNum < 16)
    {
      /* access to network PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    }
    else
    {
      /* access to fabric PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0x10, 0x10));
  }
  for (i = 0; i < countVal; i++)
  {
    if (pcsNum < 16)
    {
      /* access to network PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
    }
    else
    {
      /* access to fabric PCS */
      regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 8, 8));
  }

  return GT_OK;
}

/**
* @internal mvHwsMMPcsV2Reset function
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
GT_STATUS mvHwsMMPcsV2Reset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    GT_U32 regAddr;
    GT_U32 data, i;
    GT_U32 countVal;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;

    /* Check if the port is in AP mode.
       If true don't perform any PCS operation */
    if (countVal == 3)
    {
        return GT_BAD_STATE;
    }

    countVal = (countVal == 0) ? 1 : countVal * 2;

    if ((RESET == action) || (FULL_RESET == action))
    {
        for (i = 0; i < countVal; i++)
        {
            if (pcsNum < 16)
            {
              /* access to network PCS */
              regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
            }
            else
            {
              /* access to fabric PCS */
              regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
            }
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, 0x7F));
        }
    }
    else if (UNRESET == action)
    {
        CHECK_STATUS(mvHwsMMPcsV2Unreset(devNum,portGroup,pcsNum,countVal));
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}


/**
* @internal mvHwsMMPcsV2RxReset function
* @endinternal
*
* @brief   Set the selected RX PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsV2RxReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_RESET            action
)
{
    GT_U32 regAddr;
    GT_U32 data, i;
    GT_U32 countVal;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;

    /* Check if the port is in AP mode.
       If true don't perform any PCS operation */
    if (countVal == 3)
    {
        return GT_BAD_STATE;
    }

    countVal = (countVal == 0) ? 1 : countVal * 2;

    if (action == RESET)
    {
        /* reset PCS Rx only */
        /* on first lane reset bits 5,4,3 */
        regAddr = PCS_RESET_REG + unitAddr + pcsNum * unitIndex;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, 0x38));
        for (i = 1; i < countVal; i++)
        {
            /* on other lanes reset bits 4,3 only */
            if (pcsNum < 16)
            {
              /* access to network PCS */
              regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
            }
            else
            {
              /* access to fabric PCS */
              regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
            }
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, 0x18));
        }
    }
    else
    {
        /* un reset PCS Rx only */
        regAddr = PCS_RESET_REG + unitAddr + pcsNum * unitIndex;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, (1 << 5), (1 << 5)));
        for (i = 0; i < countVal; i++)
        {
            if (pcsNum < 16)
            {
              /* access to network PCS */
              regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
            }
            else
            {
              /* access to fabric PCS */
              regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
            }
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, (1 << 4), (1 << 4)));
        }
        for (i = 0; i < countVal; i++)
        {
            if (pcsNum < 16)
            {
              /* access to network PCS */
              regAddr = PCS_RESET_REG + unitAddr + (pcsNum+i) * unitIndex;
            }
            else
            {
              /* access to fabric PCS */
              regAddr = PCS_RESET_REG + unitAddr + (pcsNum * unitIndex) + indArr[i];
            }
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, (1 << 3), (1 << 3)));
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsMMPcsV2Mode function
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
GT_STATUS mvHwsMMPcsV2Mode
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
    /* clear register used for WA disable */
    regAddr = FEC_DEC_DMA_WR_DATA + unitAddr + curPortParams.portPcsNumber * unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, 0));

    /* by default don't use external PLL */
    regAddr = PCS40G_COMMON_CONTROL + unitAddr + curPortParams.portPcsNumber * unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, (1 << 14)));

    /* config FEC and lanes number; set MAC clock disable */
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

    for (i = 0; i < numOfLanes; i++)
    {
        if (curPortParams.portPcsNumber < 16)
        {
            regAddr = CHANNEL0_RX_GB_FIFO_CONFIGURATION + unitAddr + (curPortParams.portPcsNumber + i) * unitIndex;
        }
        else
        {
            /* access to fabric PCS */
            regAddr = CHANNEL0_RX_GB_FIFO_CONFIGURATION + unitAddr + (curPortParams.portPcsNumber * unitIndex) + indArr[i];
        }
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 26, 0));
    }

    if (curPortParams.portPcsNumber < 16)
    {
        switch (numOfLanes)
        {
        case 1:
            seq = &hwsMmPscSeqDb[MMPCS_MODE_1_LANE_SEQ];
            break;
        case 2:
            seq = &hwsMmPscSeqDb[MMPCS_MODE_2_LANE_SEQ];
            break;
        case 4:
            seq = &hwsMmPscSeqDb[MMPCS_MODE_4_LANE_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
        }

        /* Set interrupt signal detect change interrupt to be SYNC;
           set then FEC DEC status bit 1 to be rx_decoder error */
        for (i = 0; i < numOfLanes; i++)
        {
            regAddr = FEC_DEC_DMA_CONTROL + unitAddr + (curPortParams.portPcsNumber + i) * unitIndex;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, SYNC_CHANGED_INTERRUPT, SYNC_CHANGED_INTERRUPT));
        }

        if ((Lion2B0 == HWS_DEV_SILICON_TYPE(devNum)) || (HooperA0 == HWS_DEV_SILICON_TYPE(devNum)))
        {
             /* JIRA MSPEEDPCS-553 - mux of signal detect or sync_change interrupt
                    controlled by fdec_dma_read_s of MSM0 affecting all ports */
            if (curPortParams.portPcsNumber % 4 != 0)
            {
                regAddr = FEC_DEC_DMA_CONTROL + unitAddr + (curPortParams.portPcsNumber & 0xFFFFFFFC) * unitIndex;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, SYNC_CHANGED_INTERRUPT, SYNC_CHANGED_INTERRUPT));
            }

            /* for Lion change interrupt for extended/regular PCS of ports 9 and 11 */
            switch(curPortParams.portPcsNumber)
            {
                case 9:
                    for (i = 12; i < 12 + numOfLanes; i++)
                    {
                        regAddr = FEC_DEC_DMA_CONTROL + unitAddr + i * unitIndex;
                        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, SYNC_CHANGED_INTERRUPT, SYNC_CHANGED_INTERRUPT));
                    }
                    break;

                case 12:
                    regAddr = FEC_DEC_DMA_CONTROL + unitAddr + 9 * unitIndex;
                    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, SYNC_CHANGED_INTERRUPT, SYNC_CHANGED_INTERRUPT));
                    break;

                case 11:
                    for (i = 14; i < 14 + numOfLanes; i++)
                    {
                        regAddr = FEC_DEC_DMA_CONTROL + unitAddr + i * unitIndex;
                        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, SYNC_CHANGED_INTERRUPT, SYNC_CHANGED_INTERRUPT));
                    }
                    break;

                case 14:
                    regAddr = FEC_DEC_DMA_CONTROL + unitAddr + 11 * unitIndex;
                    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, SYNC_CHANGED_INTERRUPT, SYNC_CHANGED_INTERRUPT));
                    break;

                default:
                    break;
            }
        }
    }
    else
    {
        switch (numOfLanes)
        {
        case 1:
            seq = &hwsMmPscSeqDb[MMPCS_FABRIC_MODE_1_LANE_SEQ];
            break;
        case 2:
            seq = &hwsMmPscSeqDb[MMPCS_FABRIC_MODE_2_LANE_SEQ];
            break;
        case 4:
            seq = &hwsMmPscSeqDb[MMPCS_FABRIC_MODE_4_LANE_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
        }
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMMPcsV2CheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] numOfLanes               - number of lanes agregated in PCS
*
* @param[out] laneLock                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcsV2CheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
)
{
    GT_U32 regAddr;
    GT_U32 data;
    GT_U32 mask;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 pcsNum, numOfLanes;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;
    numOfLanes = curPortParams.numOfActLanes;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    /* Check Gear Box Status */
    regAddr = unitAddr + pcsNum*unitIndex + GEARBOX_STATUS;
    /* gear box status can show lock in not used bits, so read only
       required bits */
    mask = ((1 << numOfLanes) - 1) << 1;
    hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, mask);

    *laneLock = (data == mask) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}


#ifndef ASIC_SIMULATION
static GT_U32 delay10GConnectWa = 10;
#endif
static GT_U32 readSyncStatus = 1000;
static GT_U32 readSyncCount = 30;

static GT_STATUS mvHwsMmPcs10GConnectWa
(
 GT_U8                   devNum,
 GT_U32                  portGroup,
 GT_U32                  pcsNum
)
{
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex, i;
    GT_BOOL errorDetect;
    GT_U32 syncDataPrev, syncDataCurr;
    GT_U32 runCount;
    GT_BOOL noSyncData;
    GT_U32 rx_decoder_error_bitNum;
    GT_U32 baseAddr;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);
    errorDetect = GT_FALSE;

    if(BobcatA0 == HWS_DEV_SILICON_TYPE(devNum))
    {
        if(pcsNum < 56)
        {
            baseAddr = unitAddr + pcsNum * unitIndex;
        }
        else
        {
            baseAddr = unitAddr + 0x200000 + (pcsNum  - 56) * unitIndex;
        }
    }
    else
    {
        baseAddr = unitAddr + pcsNum * unitIndex;
    }

    /* Check Sync Down/Up  bit[3]*/

    runCount = 0;
    do
    {
#ifndef ASIC_SIMULATION
        hwsOsTimerWkFuncPtr(delay10GConnectWa);
#endif /*ASIC_SIMULATION*/
        noSyncData = GT_FALSE;
        regAddr = baseAddr + FEC_DEC_STATUS_REG;
        hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &syncDataPrev, 0);
        syncDataPrev &= (1 << 3);
        for (i = 1; i < readSyncStatus; i++)
        {
            hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &syncDataCurr, 0);
            syncDataCurr &= (1 << 3);

            /* The Sync-Detection indication is stable only if there is a pattern of 111... */
            if (syncDataCurr == 0)
            {
                noSyncData = GT_TRUE;
                break;
            }
            syncDataPrev = syncDataCurr;
        }
        runCount++;
    }while ((runCount < readSyncCount) && (noSyncData));

#ifdef ASIC_SIMULATION
    noSyncData = GT_FALSE;
#endif

    /* Check if there Serdes is Synced according to the Data pattern (only when the pattern is 111...) */
    if (!noSyncData)
    {
        if(BobcatA0 == HWS_DEV_SILICON_TYPE(devNum))
        {
            regAddr = PCS40G_COMMON_STATUS + baseAddr;
            rx_decoder_error_bitNum = 14;
        }
        else
        {
            regAddr = baseAddr + FEC_DEC_STATUS_REG;
            rx_decoder_error_bitNum = 1;
        }
        /* sync UP, checks if an RX Decoder Error is detected */
        for (i = 0; (i < 20); i++)
        {
            errorDetect = GT_FALSE;
            hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);


            if (((data >> rx_decoder_error_bitNum) & 1) == 1)
            {
                errorDetect = GT_TRUE;
            }
        }

        if (!errorDetect)
        {
            /* disable force link down on port */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, pcsNum, MSM_PORT_MAC_CONTROL_REGISTER0, 0, (1 << 2)));
        }
        else /* in case of Link up state: if the Sync-Detection interrupt raised and there are errors on the line --> Link down */
        {
            /* enable force link down on port */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, pcsNum, MSM_PORT_MAC_CONTROL_REGISTER0, (1 << 2), (1 << 2)));
        }
    }
    else
    {
        /* enable force link down on port */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, pcsNum, MSM_PORT_MAC_CONTROL_REGISTER0, (1 << 2), (1 << 2)));
    }

    return GT_OK;
}


/*
Upon interrupt of the above call to HW services function that implements the bellow WA:
1.  Check GB lock indication  status  (/Cider/EBU/Lion2/Lion2_B0 {Current}/Lion2_B0 Units/GOP/<MMPCS> MMPCS MMPCS MPCS_IP %p Units/Gear Box Status):
a.  If at least one GB lock indication is in-active do nothing.
b.  Else check all GB lock is active and align done is  in-active set PCS rx reset and then un-reset (may try several times)
c.  If align done is active do nothing.
CPSS/VIPS On port delete return mask to this interrupt.

*/
GT_STATUS mvHwsMmPcs40GConnectWa
(
 GT_U8                   devNum,
 GT_U32                  portGroup,
 GT_U32                  pcsNum
)
{
    GT_U32 regAddr, data, gearVal;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 countVal;
    GT_U32 baseAddr;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    if(BobcatA0 == HWS_DEV_SILICON_TYPE(devNum))
    {
        if(pcsNum < 56)
        {
            baseAddr = unitAddr + pcsNum * unitIndex;
        }
        else
        {
            baseAddr = unitAddr + 0x200000 + (pcsNum  - 56) * unitIndex;
        }

    }
    else
    {
        baseAddr = unitAddr + pcsNum * unitIndex;
    }
    regAddr = FEC_DEC_DMA_WR_DATA + baseAddr;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    if ((data != 0x8000) && (data != 0xffffffff))
    {
        /* unit not initalize or under TRx training, delete */
        return GT_OK;
    }

    regAddr = PCS40G_COMMON_CONTROL + baseAddr;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    if (countVal == 1)
    {
        /* 10G WA */
        return mvHwsMmPcs10GConnectWa(devNum,portGroup,pcsNum);
    }

    /* 40G WA */
    hwsOsTimerWkFuncPtr(10);
    /* Check Gear Box Status */
    regAddr = GEARBOX_STATUS + baseAddr;
    hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);
    gearVal = (data >> 1) & 0xF;
    /*hwsOsPrintf("\nPCS reset WA on port %d(device %d). Gear Box is 0x%x.", pcsNum, devNum, gearVal);*/
    if (gearVal != 0xF)
    {
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
        regAddr = PCS40G_COMMON_CONTROL + baseAddr;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
        if (!(data & 1))
        {
            /* run RX Reset */
            mvHwsMMPcsV2RxReset(devNum,portGroup,pcsNum,RESET);
            mvHwsMMPcsV2RxReset(devNum,portGroup,pcsNum,UNRESET);
        }
    }

    return GT_OK;
}

GT_STATUS mvMmPcsV2ExtPllCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum
)
{
    GT_U32 regAddr;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    mvUnitInfoGet(devNum, MMPCS_UNIT, &unitAddr, &unitIndex);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum*unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, (1 << 14), (1 << 14)));

    return GT_OK;
}

/**
* @internal mvHwsMMPcsV2SignalDetectMaskSet function
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
GT_STATUS mvHwsMMPcsV2SignalDetectMaskSet
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

    /* Check if the port is in AP mode.
       If true don't perform any PCS operation */
    if (countVal == 3)
    {
        return GT_BAD_STATE;
    }

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
        data = (maskEn == GT_TRUE) ? (1 << 15) : 0;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, (1 << 15)));
    }

    return GT_OK;
}
