/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsPortCfgIf.c
*
* DESCRIPTION:
*           This file contains API for port configuartion and tuning parameters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 48 $
******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <silicon/bobcat2/mvHwsBobcat2PortIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3If.h>

/**
* @internal mvHwsBobcat2PCSMarkModeSet function
* @endinternal
*
* @brief   Mark/Un-mark PCS unit
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - GT_TRUE  for mark the PCS,
*                                      GT_FALSE for un-mark the PCS
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS mvHwsBobcat2PCSMarkModeSet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL enable
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    switch (portMode)
    {
        case _10GBase_KR:
        case _40GBase_KR:
        case _10GBase_SR_LR:
        case _40GBase_SR_LR:
        case _12GBaseR:
        case _24GBase_KR2:
        case _48GBaseR:
        case _12GBase_SR:
        case _48GBase_SR:
        case _40GBase_CR4:
            if (HWS_DEV_IS_BOBCAT2_A0(devNum))
            {
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, curPortParams.portPcsNumber,
                                                FEC_DEC_DMA_WR_DATA, enable, 1));
            }
            break;
        default:
            hwsOsPrintf("mvHwsPCSMarkModeSet: portMode %d is not supported for Mark/Un-mark PCS unit\n", portMode);
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsBobcat2PortFixAlign90Ext function
* @endinternal
*
* @brief   Fix Align90 parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      optAlgoMask - bit mask for optimization algorithms
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobcat2PortFixAlign90Ext
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    switch (portMode)
    {
    case _10GBase_KR:
    case _20GBase_KR:
    case _40GBase_KR:
    case _40GBase_KR4:
    case _100GBase_KR10:
    case _10GBase_SR_LR:
    case _20GBase_SR_LR:
    case _40GBase_SR_LR:
    case _12GBaseR:
    case _24GBase_KR2:
    case _48GBaseR:
    case _12GBase_SR:
    case _48GBase_SR:
    case _12_1GBase_KR:  /* XLHGL_KR */
    case XLHGL_KR4:
    case INTLKN_4Lanes_3_125G:
    case INTLKN_8Lanes_3_125G:
    case INTLKN_4Lanes_6_25G:
    case INTLKN_8Lanes_6_25G:
    case _40GBase_CR4:
        if (HWS_DEV_IS_BOBCAT2_A0(devNum))
        {
            CHECK_STATUS(mvHwsPortFixAlign90(devNum, portGroup, phyPortNum, portMode, 0));
        }
        CHECK_STATUS(mvHwsBobcat2PortFineTune(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
        break;
    default:
        break;
    }

    return GT_OK;
}

/**
* @internal mvHwsBobcat2PortRxAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobcat2PortRxAutoTuneSetExt
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32  optAlgoMask,
    void *  results
)
{
    MV_HWS_AUTO_TUNE_RESULTS res[HWS_MAX_SERDES_NUM];
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_AUTO_TUNE_STATUS rxStatus[HWS_MAX_SERDES_NUM];
    MV_HWS_AUTO_TUNE_STATUS txStatus[HWS_MAX_SERDES_NUM];
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL autoTunePass[HWS_MAX_SERDES_NUM];
    GT_BOOL laneFail = GT_FALSE;
    GT_U32  sqleuch, ffeR, ffeC, align90;
    GT_U32  txAmp, txEmph0, txEmph1;
    GT_U32  i;
    GT_U32 unitAddr, regAddr;
    GT_U32 unitIndex;
    GT_U32 numOfActLanes;

    /* avoid warnings */
    portTuningMode = portTuningMode;
    optAlgoMask = optAlgoMask;

    (void)results; /* avoid warnings */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* for RX Training Only */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    hwsOsMemSetFuncPtr(autoTunePass, 0, sizeof(autoTunePass));
    hwsOsMemSetFuncPtr(res, 0, sizeof(res));
    hwsOsMemSetFuncPtr(rxStatus, 0, sizeof(rxStatus));
    hwsOsMemSetFuncPtr(txStatus, 0, sizeof(txStatus));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* mark PCS to be under Rx training for Puma3B0 and BobcatA0 only */
    if (HWS_DEV_IS_BOBCAT2_A0(devNum))
    {
        switch (portMode)
        {
        case _40GBase_KR:
        case _40GBase_CR4:
        case _40GBase_SR_LR:
            CHECK_STATUS(mvUnitExtInfoGet(devNum, MMPCS_UNIT, curPortParams.portPcsNumber, &unitAddr, &unitIndex,NULL));
            regAddr = FEC_DEC_DMA_WR_DATA + unitAddr + (curPortParams.portPcsNumber * unitIndex);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 1, 1));
            break;
        default:
            break;
        }
    }

    numOfActLanes = curPortParams.numOfActLanes;

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        if (autoTunePass[i] == GT_FALSE)
        {
            CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), GT_TRUE));
        }
    }

    /* check status and start auto tune again if not passed */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        if (autoTunePass[i] == GT_FALSE)
        {
            mvHwsSerdesAutoTuneStatus(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &rxStatus[i], NULL);
            if (rxStatus[i] == TUNE_PASS)
            {
                autoTunePass[i] = GT_TRUE;
            }
            else
            {
                laneFail = GT_TRUE;
            }
        }
    }

    hwsOsTimerWkFuncPtr(1);

    if (laneFail == GT_TRUE)
    {
        return GT_FAIL;
    }

    /* get results */
    sqleuch = ffeR = ffeC = align90 = 0;
    txAmp = txEmph0 = txEmph1 = 0;
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesAutoTuneResult(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &res[i]));
        sqleuch += res[i].sqleuch;
        ffeR += res[i].ffeR;
        ffeC += res[i].ffeC;
        txAmp += res[i].txAmp;
        txEmph0 += res[i].txEmph0;
        txEmph1 += res[i].txEmph1;
    }

    /* disable training on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), GT_FALSE));
    }

    CHECK_STATUS(mvHwsBobcat2PortFixAlign90Ext(devNum, portGroup, phyPortNum, portMode));

    /* unreset PCS Rx only */
    hwsOsTimerWkFuncPtr(1);

    /* unmark PCS to be under Rx training for Puma3B0 and BobcatA0 only */
    if (HWS_DEV_IS_BOBCAT2_A0(devNum))
    {
        switch (portMode)
        {
        case _40GBase_KR:
        case _40GBase_SR_LR:
        case _40GBase_CR4:

            CHECK_STATUS(mvUnitExtInfoGet(devNum, MMPCS_UNIT, (GT_U8)curPortParams.portPcsNumber, &unitAddr, &unitIndex,NULL));

            regAddr = FEC_DEC_DMA_WR_DATA + unitAddr + (curPortParams.portPcsNumber * unitIndex);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, 0, 1));

            /* call to WA */
            mvHwsMmPcs28nm40GBackWa(devNum, portGroup, curPortParams.portMacNumber, portMode);
            break;
        default:
            break;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsBobcat2PortTxAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx only parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobcat2PortTxAutoTuneSetExt
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    void    *results
)
{
    MV_HWS_AUTO_TUNE_STATUS_RES *tuneRes;

    if (portTuningMode <= TRxTuneStatus)
    {
        if (portTuningMode == TRxTuneStart)
        {
            if (results == NULL)
            {
                hwsOsPrintf("Error: results cannot be NULL when using TRxTuneStart param\n");
                return GT_BAD_PARAM;
            }
            else
            {
                tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
                tuneRes->txTune = TUNE_PASS;
            }
        }

        CHECK_STATUS(hwsPortTxAutoTuneStartSet(devNum, portGroup, phyPortNum, portMode, portTuningMode, 0/*optAlgoMask*/));
    }

    if (portTuningMode == TRxTuneStatus)
    {
        CHECK_STATUS(mvHwsBobcat2PortFixAlign90Ext(devNum, portGroup, phyPortNum, portMode));

        /* during Tx config both MAC and PCS are in Reset. Status part only Un-Reset the PCS
           so MAC needs to be Un-Reset as well */
        CHECK_STATUS(hwsPortTxAutoTuneActivateSet(devNum, portGroup, phyPortNum, portMode, portTuningMode));
    }
    else if (portTuningMode == TRxTuneStatusNonBlocking)
    {
        if (results == NULL)
        {
            hwsOsPrintf("Error: results cannot be NULL when using TRxTuneStatusNonBlocking param\n");
            return GT_BAD_PARAM;
        }
        tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;

        CHECK_STATUS(mvHwsPortAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode,
                                                 &tuneRes->rxTune, &tuneRes->txTune));
    }
    else if (portTuningMode == TRxTuneStop)
    {
        /* Fix align90 wll be executed during AP stop )and after that the MAC will be un-reset */
        CHECK_STATUS(mvHwsPortAutoTuneStop(devNum, portGroup, phyPortNum, portMode, GT_FALSE /*Rx*/, GT_TRUE /*Tx*/));
    }

    return GT_OK;
}


/**
* @internal mvHwsBobcat2PortAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobcat2PortAutoTuneSetExt
(
    GT_U8    devNum,
    GT_U32   portGroup,
    GT_U32   phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32   optAlgoMask,
    void     *results
)
{
    /* avoid warnings */
    optAlgoMask = optAlgoMask;

    if (portTuningMode == RxTrainingAdative || portTuningMode == RxTrainingStopAdative)
    {
        return GT_NOT_SUPPORTED;
    }

    if (portTuningMode == RxTrainingOnly)
    {
        return mvHwsBobcat2PortRxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portMode, portTuningMode, results);
    }
    else
    {
        return mvHwsBobcat2PortTxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portTuningMode, results);
    }
}

/**
* @internal mvHwsBobcat2PortFineTune function
* @endinternal
*
* @brief   - Set the PPM compensation for Rx/TRX training in 10G and above.
*         Relevant to GOP_28NM_REV1 and above.
*         - Set FixAlign90 for Serdes 28nmRev3 process on current SERDES
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] configPpm                - enable/disable the PPM configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobcat2PortFineTune
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL configPpm
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        if (curPortParams.serdesSpeed >= _10_3125G)
        {
            /* Set the PPM compensation on Serdes after Rx/TRX training in 10G and above */
            CHECK_STATUS(mvHwsComH28nmSerdesAdaptPpm(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                        (curLanesList[i] & 0xFFFF), configPpm));
        }

         if (HWS_DEV_IS_BOBCAT2_B0(devNum))
        {
             /* Run fix Align90 process on SERDES */
             CHECK_STATUS(mvHwsComH28nmRev3SerdesFixAlign90(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                            (curLanesList[i] & 0xFFFF)));
         }
    }

    return GT_OK;
}

#if 0
/**
* @internal mvHwsBobcat2PortBetterAdaptationSet function
* @endinternal
*
* @brief   Run the better Align90 adaptation algorithm on specific port number.
*         The function returns indication if the Better adaptation algorithm
*         succeeded to run on port or not.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*                                      portMode   - port standard metric
*
* @param[out] betterAlign90            - indicates if the Better adaptation algorithm run on
*                                      specific port or not
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsBobcat2PortBetterAdaptationSet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *betterAlign90
)
{
    if (HWS_DEV_IS_BOBCAT2_B0(devNum))
    {
        CHECK_STATUS(mvHwsComH28nmRev3FindBetterAdaptation(devNum, portGroup, serdesNum, 300, 10, betterAlign90));
    }

    return GT_OK;
}
#endif

/**
* @internal mvHwsBobcat2PortExtendedModeCfg function
* @endinternal
*
* @brief   Enable / disable extended mode on port specified.
*         Extended ports supported only in Lion2 and Alleycat3 devices.
*         For Lion2:   1G, 10GBase-R, 20GBase-R2, RXAUI - can be normal or extended
*         XAUI, DXAUI, 40GBase-R - only extended
*         For Alleycat3: ports 25 and 27 can be 10GBase_KR, 10GBase_SR_LR - normal or extended modes
*         port 27 can be 20GBase_KR, 20GBase_SR_LR - only in extended mode
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] extendedMode             - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobcat2PortExtendedModeCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL extendedMode
)
{
    /* avoid warnings */
    devNum       = devNum;
    portGroup    = portGroup;
    phyPortNum   = phyPortNum;
    portMode     = portMode;
    extendedMode = extendedMode;

    return GT_NOT_SUPPORTED;
}

/**
* @internal mvHwsBobcat2PortExtendedModeCfgGet function
* @endinternal
*
* @brief   Returns the extended mode status on port specified.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] extendedMode             - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobcat2PortExtendedModeCfgGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL *extendedMode
)
{
    /* avoid warnings */
    devNum       = devNum;
    portGroup    = portGroup;
    phyPortNum   = phyPortNum;
    portMode     = portMode;
    extendedMode = extendedMode;

    return GT_NOT_SUPPORTED;
}

/**
* @internal mvHwsBobcat2PortPsyncBypassCfg function
* @endinternal
*
* @brief   Configures the bypass of synchronization module.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobcat2PortPsyncBypassCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_UOPT macNum;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    macNum = curPortParams.portMacNumber;

    if (portMode == SGMII2_5)
    {
        /* set P Sync Bypass */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
                            GIG_PORT_MAC_CONTROL_REGISTER4, (1 << 6), (1 << 6)));
        return GT_OK;
    }

    /* unset P Sync Bypass */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                        GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));

    return GT_OK;
}


