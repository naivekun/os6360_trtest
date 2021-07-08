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
*/
/**
********************************************************************************
* @file mvHwsGeneralPortCfgIf.c
*
* @brief This file contains API for port configuartion and tuning parameters
*
* @version   48
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#include <mv_hws_avago_if.h>


/**************************** Definition ***************************************/
#define MV_HWS_AVAGO_GUI_ENABLED (0x1)
#define MV_HWS_AVAGO_GUI_MASK    (0x1)

/**************************** Pre-Declarations *********************************/
extern int mvHwsAvagoSerdesTemperatureGet
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    int             *temperature
);

#ifndef ASIC_SIMULATION
extern int avagoSerdesAacsServerExec
(
    unsigned char devNum
);
#endif /* ASIC_SIMULATION */

GT_STATUS mvHwsPortAvagoSerdesTunePreConfigSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 resetEnable
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32                      i, regData, counter;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortEnhanceTuneSet null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* 50/40G KR2 NO FEC workaround */
    if((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        if((curPortParams.portFecMode == FEC_OFF) &&
          ((portMode == _50GBase_KR2) || (portMode == _40GBase_KR2) || (portMode == _50GBase_SR2) || (portMode == _50GBase_CR2) || (portMode == _52_5GBase_KR2)))
        {
            if(resetEnable == GT_FALSE)
            {
                for (i = 0; i < curPortParams.numOfActLanes; i++)
                {
                    counter = 0;

                    /* Poll for o_core_status[4] == 1 */
                    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, SERDES_UNIT, (curLanesList[i] & 0xFFFF), AVAGO_CORE_STATUS, &regData, 0));
                    while(((regData >> 4) & 1) != 1)
                    {
                        counter++;
                        hwsOsTimerWkFuncPtr(1);
                        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, SERDES_UNIT, (curLanesList[i] & 0xFFFF), AVAGO_CORE_STATUS, &regData, 0));
                        if(counter == 50)
                        {
                            HWS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ARG_STRING_MAC("SD Clock to Gearbox is not ready\n"));
                        }
                    }
                }
            }

            regData = (resetEnable == GT_TRUE) ? 0 : 1;
            /* Disable rx_tlat - stop clock from SERDES to GearBox */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, (curLanesList[i] & 0xFFFF), SERDES_EXTERNAL_CONFIGURATION_0, (regData << 13), (1 << 13)));
            }

            /* Reset sd_rx_reset_ - GearBox RX reset*/
            if(phyPortNum % 4 == 0)
            {
                regData = (resetEnable == GT_TRUE) ? 0x0 : 0x3;
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_RESETS, regData, 0x3));
            }
            else if(phyPortNum % 2 == 0)
            {
                regData = (resetEnable == GT_TRUE) ? 0x0 : 0xC;
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_RESETS, regData, 0xC));
            }
            else
            {
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal port num for 50G/40G WA"));
            }
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsGeneralPortSignalDetectMask function
* @endinternal
*
* @brief   mask/unmask-signal detect interrupt
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   -  / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralPortSignalDetectMask
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsGeneralPortSignalDetectMask null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, (curLanesList[i] & 0xFFFF), SERDES_EXTERNAL_CONFIGURATION_0, (enable << 7), (1 << 7)));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortManualCtleConfig function
* @endinternal
*
* @brief   configures SERDES Ctle parameters for specific one SERDES lane or
*         for all SERDES lanes on port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] laneNum                  - Serdes number on port or
*                                      0xFF for all Serdes numbers on port
* @param[in] configParams             pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualCtleConfig
(
    GT_U8                               devNum,
    GT_U32                              portGroup,
    GT_U32                              phyPortNum,
    MV_HWS_PORT_STANDARD                portMode,
    GT_U8                               laneNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *configParams
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  startLaneNum;
    GT_U32  endLaneNum;
    GT_U32  i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
        || (HWS_DEV_SERDES_TYPE(devNum) != AVAGO))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortManualCtleConfig null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* for all lanes on port */
    if (laneNum == 0xFF)
    {
        startLaneNum = 0;
        endLaneNum = (curPortParams.numOfActLanes) - 1;
    }
    else /* for specific one lane on port */
    {
        startLaneNum = endLaneNum = laneNum;
    }

    for (i=startLaneNum; i <= endLaneNum; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesManualCtleConfig(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), configParams->dcGain,
                                                      configParams->lowFrequency, configParams->highFrequency, configParams->bandWidth,
                                                      configParams->loopBandwidth, configParams->squelch));
    }

    return GT_OK;
}


/**
* @internal mvHwsPortManualCtleConfigGet function
* @endinternal
*
* @brief   Get SERDES Ctle parameters of each SERDES lane.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] laneNum                  - Serdes number on port or
*                                      0xFF for all Serdes numbers on port
*
* @param[out] configParams[]           - array of Ctle params structures, Ctle parameters of each SERDES lane.
*                                      Ctle params structures:
*                                      dcGain         DC-Gain value        (rang: 0-255)
*                                      lowFrequency   CTLE Low-Frequency   (rang: 0-15)
*                                      highFrequency  CTLE High-Frequency  (rang: 0-15)
*                                      bandWidth      CTLE Band-width      (rang: 0-15)
*                                      loopBandwidth  CTLE Loop Band-width (rang: 0-15)
*                                      squelch        Signal OK threshold  (rang: 0-310)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualCtleConfigGet
(
    GT_U8                               devNum,
    GT_U32                              portGroup,
    GT_U32                              phyPortNum,
    MV_HWS_PORT_STANDARD                portMode,
    GT_U8                               laneNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    configParams[]
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  startLaneNum;
    GT_U32  endLaneNum;
    GT_U32  i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
        || (HWS_DEV_SERDES_TYPE(devNum) != AVAGO))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortManualCtleConfigGet null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* for all lanes on port */
    if (laneNum == 0xFF)
    {
        startLaneNum = 0;
        endLaneNum = (curPortParams.numOfActLanes) - 1;
    }
    else /* for specific one lane on port */
    {
        startLaneNum = endLaneNum = laneNum;
    }

    for (i=startLaneNum; i <= endLaneNum; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesManualCtleConfigGet(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &configParams[i]));
    }

    return GT_OK;
}
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)

/**
* @internal mvHwsGeneralPortExtendedModeCfgGet function
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
GT_STATUS mvHwsGeneralPortExtendedModeCfgGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *extendedMode
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
* @internal mvHwsPortEnhanceTuneSet function
* @endinternal
*
* @brief   Perform Enhance Tuning for finding the best peak of the eye
*         on specific port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneSet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_LF,
    GT_U8       max_LF
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      curLanesList[HWS_MAX_SERDES_NUM];

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    if((portMode == _100GBase_MLG) && ((phyPortNum % 4) != 0))
    {
        phyPortNum &= 0xFFFFFFFC;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortEnhanceTuneSet null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
    CHECK_STATUS(mvHwsPortAvagoSerdesTunePreConfigSet(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsAvagoSerdesEnhanceTune(devNum, portGroup, curLanesList, curPortParams.numOfActLanes, min_LF, max_LF));
    CHECK_STATUS(mvHwsPortAvagoSerdesTunePreConfigSet(devNum, portGroup, phyPortNum, portMode, GT_FALSE));

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhaseAllocation function
* @endinternal
*
* @brief   Allocate memory (if needed) for enhance-tune-by-phase algorithm which
*         is used by port manager.
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhaseAllocation
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
#else
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] =
            (MV_HWS_AVAGO_PORT_MANAGER_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of portManagerPerPortDbPtr[phyPortNum] failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum],
                           0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
    }

    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr =
            (MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of enhanceTuneStcPtr failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr,
                       0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC));

        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->parametersInitialized = GT_FALSE;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesOneShotTunePhaseAllocation function
* @endinternal
*
* @brief   Allocate memory (if needed) for
*         one-shot-tune-by-phase algorithm which is used by port
*         manager.
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTunePhaseAllocation
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
#else
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] =
            (MV_HWS_AVAGO_PORT_MANAGER_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of portManagerPerPortDbPtr[phyPortNum] failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum],
                           0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
    }

    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr =
            (MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of mvHwsAvagoSerdesOneShotTunePhaseAllocation failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr,
                       0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC));

        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr->parametersInitialized = GT_FALSE;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEdgeDetectPhaseAllocation function
* @endinternal
*
* @brief   Allocate memory (if needed) for edge-detect-by-phase algorithm which
*         is used by port manager.
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEdgeDetectPhaseAllocation
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
#else
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] =
            (MV_HWS_AVAGO_PORT_MANAGER_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum] == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of portManagerPerPortDbPtr[phyPortNum] failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum],
                           0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC));
    }

    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr == NULL)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr =
            (MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC *)
            hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC));
        if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr == NULL)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of edgeDetectStcPtr failed"))
        }
        hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr,
                       0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC));
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr->initFlag
                           = GT_FALSE;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsPortManagerClose function
* @endinternal
*
* @brief   Free all port manager allocated memory.
*
* @param[in] devNum                   - system device number
*                                      phyPortNum - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManagerClose
(
    GT_U8   devNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
#else
    GT_U32 portMacNum;

    /* port manager close */
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr != NULL)
    {
        for (portMacNum=0; portMacNum<hwsDeviceSpecInfo[devNum].portsNum; portMacNum++)
        {
            if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum] != NULL)
            {
                if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]->edgeDetectStcPtr != NULL)
                {
                    hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]->edgeDetectStcPtr);
                }
                if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]->enhanceTuneStcPtr != NULL)
                {
                    hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]->enhanceTuneStcPtr);
                }
                hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portMacNum]);
            }
        }
        hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr);
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsPortManagerInit function
* @endinternal
*
* @brief   Allocate inital memory for Hws port manager DB.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManagerInit
(
    GT_U8   devNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
#else
    /* db not needed in FW, and also currently not needed in simulation
       as all APIs using this db return GT_OK in simulation, therefore currently
       we do not need this memory space in simulation also */
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr = (MV_HWS_AVAGO_PORT_MANAGER_STC_PTR*)
        hwsOsMallocFuncPtr(sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC_PTR)*hwsDeviceSpecInfo[devNum].portsNum);
    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr == NULL)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    LOG_ARG_STRING_MAC("allocation of portManagerPerPortDbPtrPtr failed"));
    }
    hwsOsMemSetFuncPtr(hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr,
                           0, sizeof(MV_HWS_AVAGO_PORT_MANAGER_STC_PTR)*hwsDeviceSpecInfo[devNum].portsNum);
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEdgeDetectByPhase function
* @endinternal
*
* @brief   This function calls Edge-Detect algorithm phases, in a way that it returns
*         after triggering each RX tune performed inside the Edge-Detect, without
*         affecting the sequence of the Edge-Detect algorithm. This mode of operation
*         allows the caller to utilize the idle time after RX tune trigger.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase                    - current Edge-Detect  to run
*
* @param[out] phaseFinishedPtr         - whether or not phase is finished
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEdgeDetectByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
#else
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_STATUS rc;
    GT_BOOL continueExecute,isInNonBlockWait;
    GT_U32 timeOut;

    if (!( HWS_DEV_SILICON_TYPE(devNum) == Alleycat3A0 ||
           HWS_DEV_SILICON_TYPE(devNum) == BobK ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin ||
           HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 ||
           HWS_DEV_SILICON_TYPE(devNum) == Pipe ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) )
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                     LOG_ARG_STRING_MAC("edge-detect by phase not supported for device"),
                                     LOG_ARG_GEN_PARAM_MAC((HWS_DEV_SILICON_TYPE(devNum))));
    }

    phaseFinishedPtr=phaseFinishedPtr;

    *phaseFinishedPtr = GT_FALSE;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsAvagoSerdesEdgeDetectByPhase null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* check that edge detect phase db is allocated */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesEdgeDetectPhaseAllocation(devNum, phyPortNum),
                     LOG_ARG_STRING_MAC("allocation of edge detect phase failed"));

    timeOut = 300; /* milisec */

    if (hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr->initFlag == GT_FALSE)
    {
        mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb(devNum, phyPortNum, timeOut);
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr->initFlag = GT_TRUE;
    }

    if (phase==1)
    {
        rc = mvHwsAvagoSerdesWaitForEdgeDetectPhase1(devNum, portGroup, curPortParams.numOfActLanes,
                                                  &(curLanesList[0]),50,10);
        if (rc!=GT_OK)
        {
            return rc;
        }

        rc = mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1(devNum,phyPortNum,portGroup,curPortParams.numOfActLanes,
                                          &(curLanesList[0]),50,10);
        if (rc!=GT_OK)
        {
            return rc;
        }
        /* phase1 finished */
        *phaseFinishedPtr = GT_TRUE;
    }
    else
    {
        rc = mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2(devNum,phyPortNum,portGroup,curPortParams.numOfActLanes,
                                                  &(curLanesList[0]),50,10, &continueExecute, &isInNonBlockWait);
        if (rc!=GT_OK)
        {
            return rc;
        }

        if (continueExecute==GT_TRUE && isInNonBlockWait==GT_FALSE)
        {
            rc = mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1(devNum, phyPortNum, portGroup, curPortParams.numOfActLanes,
                                                  &(curLanesList[0]),50,10);
            if (rc!=GT_OK)
            {
                return rc;
            }
        }

    }

    if (phase == 2 && continueExecute == GT_FALSE)
    {
        *phaseFinishedPtr = GT_TRUE;
        mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb(devNum, phyPortNum, timeOut);
    }
#endif

    return GT_OK;
}
/**
* @internal mvHwsAvagoSerdesOneShotTuneByPhase function
* @endinternal
*
* @brief   running one shot tune (iCal) by phases, it take care
* of KR2/CR2 cases with Fec mode disabled, to unharm the
* sequence of port manager.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase....................- current phase
*
* @param[out] phaseFinishedPtr                 - (pointer to)
*       whether or not we finished the last phase.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTuneByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
#else
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_PORT_FEC_MODE fecCorrect;
    GT_STATUS rc;
    GT_U32 ii;
    MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *oneShotTuneStcPtr;

    if (!( HWS_DEV_SILICON_TYPE(devNum) == Alleycat3A0 ||
           HWS_DEV_SILICON_TYPE(devNum) == BobK ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin ||
           HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 ||
           HWS_DEV_SILICON_TYPE(devNum) == Pipe ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) )
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                     LOG_ARG_STRING_MAC("one-shot-tune by phase not supported for device"),
                                     LOG_ARG_GEN_PARAM_MAC((HWS_DEV_SILICON_TYPE(devNum))));
    }

    rc = GT_OK;
        /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsAvagoSerdesOneShotTuneByPhase null hws param ptr"));
    }
    fecCorrect = curPortParams.portFecMode;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* checking that port manager one shot tune db is allocated for the port */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesOneShotTunePhaseAllocation(devNum ,phyPortNum),
                     LOG_ARG_STRING_MAC("one shot tune phase allocation failed"));

    oneShotTuneStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr;

    if (oneShotTuneStcPtr->parametersInitialized==GT_FALSE)
    {
        mvHwsAvagoSerdesOneShotTuneByPhaseInitDb(devNum, phyPortNum);
        oneShotTuneStcPtr->parametersInitialized=GT_TRUE;
    }

    switch (phase)
    {
        case 1:
            rc = mvHwsAvagoSerdesPrePostTuningByPhase(
            devNum,
            portGroup,
            phyPortNum,
            &(curLanesList[0]),
            curPortParams.numOfActLanes,
            GT_TRUE,
            &(oneShotTuneStcPtr->coreStatusReady[0]),
            fecCorrect,
            portMode);
            if (rc != GT_OK)
            {
                return rc;
            }
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL));
            *phaseFinishedPtr = GT_TRUE;
            break;

        case 2:
            rc = mvHwsAvagoSerdesPrePostTuningByPhase(
            devNum,
            portGroup,
            phyPortNum,
            &(curLanesList[0]),
            curPortParams.numOfActLanes,
            GT_FALSE,
            &(oneShotTuneStcPtr->coreStatusReady[0]),
            fecCorrect,
            portMode
            );

            *phaseFinishedPtr = GT_TRUE;
            for (ii=0; ii<curPortParams.numOfActLanes; ii++)
            {
                if (oneShotTuneStcPtr->coreStatusReady[ii] == GT_FALSE)
                {
                    *phaseFinishedPtr = GT_FALSE;
                }
            }
            break;

        default:
            hwsOsPrintf("\n** One Shot Tune: Default switch-case. N/A **\n");
    }

    if (phase==2 && *phaseFinishedPtr == GT_TRUE)
    {
       mvHwsAvagoSerdesOneShotTuneByPhaseInitDb(devNum, phyPortNum);
    }
#endif
    return GT_OK;

}

/**
* @internal mvHwsAvagoSerdesEnhanceTuneSetByPhase function
* @endinternal
*
* @brief   This function calls Enhance-Tune algorithm phases, in a way that it returns
*         after triggering each RX tune performed inside the Enhance-Tune, without
*         affecting the sequence of the Enhance-Tune algorithm. This mode of operation
*         allows the caller to utilize the idle time after RX tune trigger.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @param[out] phaseFinishedPtr         - whether or not phase is finished
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneSetByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_LF,
    GT_U8       max_LF,
    GT_U32      phase,
    GT_BOOL     *phaseFinishedPtr
)
{
#ifdef ASIC_SIMULATION
    GT_STATUS rc = GT_OK;
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    min_LF = min_LF;
    max_LF = max_LF;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
#else
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_FEC_MODE  fecCorrect;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_STATUS rc;
    GT_BOOL prePostConfigTune = GT_FALSE;
    GT_U32 ii;
    MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC *enhanceTuneStcPtr;
    MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *oneShotTuneStcPtr = NULL;


    if (!( HWS_DEV_SILICON_TYPE(devNum) == Alleycat3A0 ||
           HWS_DEV_SILICON_TYPE(devNum) == BobK ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin ||
           HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 ||
           HWS_DEV_SILICON_TYPE(devNum) == Pipe ||
           HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) )
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                                     LOG_ARG_STRING_MAC("enhance-tune by phase not supported for device"),
                                     LOG_ARG_GEN_PARAM_MAC((HWS_DEV_SILICON_TYPE(devNum))));
    }

    rc = GT_OK;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsAvagoSerdesEnhanceTuneSetByPhase null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* checking that port manager enhace tune db is allocated for the port */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesEnhanceTunePhaseAllocation(devNum ,phyPortNum),
                     LOG_ARG_STRING_MAC("enhance tune phase allocation failed"));

    enhanceTuneStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr;


    if ((enhanceTuneStcPtr->parametersInitialized==GT_FALSE) || (phase == 1))
    {
        mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb(devNum, phyPortNum);
        enhanceTuneStcPtr->parametersInitialized=GT_TRUE;
    }
    fecCorrect = curPortParams.portFecMode;
    /* Align CG resets workaround (KR2/CR2 with FEC OFF) before Rx Training for Port Manager EnhanceTune Algorithm */
    if((fecCorrect == FEC_OFF) &&
          ((portMode == _50GBase_KR2) || (portMode == _40GBase_KR2) || (portMode == _50GBase_SR2) || (portMode == _50GBase_CR2) || (portMode == _52_5GBase_KR2)))
    {
            prePostConfigTune = GT_TRUE;
            /* checking that port manager one shot tune db is allocated for the port */
            CHECK_STATUS_EXT(mvHwsAvagoSerdesOneShotTunePhaseAllocation(devNum ,phyPortNum),
                     LOG_ARG_STRING_MAC("one shot tune phase allocation failed"));

            oneShotTuneStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr;

            if (oneShotTuneStcPtr->parametersInitialized==GT_FALSE)
            {
                mvHwsAvagoSerdesOneShotTuneByPhaseInitDb(devNum, phyPortNum);
                oneShotTuneStcPtr->parametersInitialized=GT_TRUE;
            }
    }

    switch (phase)
    {
        case 1:
            if (prePostConfigTune)
            {
                rc = mvHwsAvagoSerdesPrePostTuningByPhase(
                devNum,
                portGroup,
                phyPortNum,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                GT_TRUE,
                &(oneShotTuneStcPtr->coreStatusReady[0]),
                fecCorrect,
                portMode);
            }
            /*hwsOsPrintf("\n** Enhance tune: phase 1 **\n");*/
            rc = mvHwsAvagoSerdesEnhanceTunePhase1(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &enhanceTuneStcPtr->phase_CurrentDelayPtr,
                &enhanceTuneStcPtr->phase_CurrentDelaySize,
                &(enhanceTuneStcPtr->phase_Best_eye[0]));
        break;

        case 2:
            rc =  mvHwsAvagoSerdesEnhanceTunePhase2(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &(enhanceTuneStcPtr->phase_Best_LF[0]));
            *phaseFinishedPtr = GT_TRUE;
            break;

        case 3:
            /*hwsOsPrintf("\n** Enhance tune: phase 3 **\n");*/
            rc = mvHwsAvagoSerdesEnhanceTunePhase3(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                enhanceTuneStcPtr->phase_CurrentDelaySize,
                enhanceTuneStcPtr->phase_CurrentDelayPtr,
                &enhanceTuneStcPtr->phase_InOutI,
                &(enhanceTuneStcPtr->phase_Best_LF[0]),
                &(enhanceTuneStcPtr->phase_Best_eye[0]),
                &(enhanceTuneStcPtr->phase_Best_dly[0]),
                &(enhanceTuneStcPtr->subPhase));
            if (rc==GT_OK)
            {
                if (enhanceTuneStcPtr->phase_InOutI == 0xFFFFFFFF)
                {
                    *phaseFinishedPtr = GT_TRUE;
                }
                else
                {
                    *phaseFinishedPtr = GT_FALSE;
                }
            }
            break;

        case 4:
            /*hwsOsPrintf("\n** Enhance tune: phase 4 **\n");*/
            rc = mvHwsAvagoSerdesEnhanceTunePhase4(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &(enhanceTuneStcPtr->phase_Best_dly[0]));
        break;

        case 5:
            /*hwsOsPrintf("\n** Enhance tune: phase 5 **\n");*/
            rc = mvHwsAvagoSerdesEnhanceTunePhase5(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &enhanceTuneStcPtr->phase_LF1_Arr[0]);
        break;

        case 6:
            if (enhanceTuneStcPtr->waitForCoreReady == GT_FALSE)
            {
                /*hwsOsPrintf("\n** Enhance tune: phase 6 **\n");*/
                    rc = mvHwsAvagoSerdesEnhanceTunePhase6(
                        devNum,
                        portGroup,
                        &(curLanesList[0]),
                        curPortParams.numOfActLanes,
                        min_LF,
                        max_LF,
                        /* phase dependant args */
                        &enhanceTuneStcPtr->phase_LF1_Arr[0],
                        &(enhanceTuneStcPtr->phase_Best_eye[0]),
                        &(enhanceTuneStcPtr->phase_Best_dly[0]),
                        &enhanceTuneStcPtr->phase_InOutKk,
                        &enhanceTuneStcPtr->phase_continueExecute);
            }
            if (rc==GT_OK)
            {
                if (enhanceTuneStcPtr->phase_continueExecute==GT_FALSE)
                {
                    if (prePostConfigTune)
                    {
                        rc = mvHwsAvagoSerdesPrePostTuningByPhase(
                        devNum,
                        portGroup,
                        phyPortNum,
                        &(curLanesList[0]),
                        curPortParams.numOfActLanes,
                        GT_FALSE,
                        &(oneShotTuneStcPtr->coreStatusReady[0]),
                        fecCorrect,
                        portMode);
                        *phaseFinishedPtr = GT_TRUE;
                        for (ii=0; ii<curPortParams.numOfActLanes; ii++)
                        {
                            if (oneShotTuneStcPtr->coreStatusReady[ii] == GT_FALSE)
                            {
                                enhanceTuneStcPtr->waitForCoreReady = GT_TRUE;
                                *phaseFinishedPtr = GT_FALSE;
                            }
                        }
                    }
                    else
                    {
                        *phaseFinishedPtr = GT_TRUE;
                    }
                }
                else
                {
                    *phaseFinishedPtr = GT_FALSE;
                }
            }
        break;

        default:
            hwsOsPrintf("\n** Enhance tune: Default switch-case. N/A **\n");
        break;
    }

    if ( (phase==6 && *phaseFinishedPtr == GT_TRUE) || rc != GT_OK)
    {
        mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb(devNum, phyPortNum);
    }
#endif

    return rc;
}

/**
* @internal mvHwsPortAvagoSerdesRxSignalOkStateGet function
* @endinternal
*
* @brief   Get the rx_signal_ok indication from the port lanes. If all lanes
*         have rx_signal_ok indication, then output value is true. Else,
*         output value is false.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoSerdesRxSignalOkStateGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL *signalOk
)
{
    GT_BOOL tmpSignalOk;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 i;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];

    HWS_NULL_PTR_CHECK_MAC(signalOk);

    *signalOk = GT_TRUE;
    tmpSignalOk = GT_FALSE;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS_EXT(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList),
                 LOG_ARG_GEN_PARAM_MAC(portMode), LOG_ARG_STRING_MAC("rebuilding active lanes list"));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "RxSignalOkStateGet- curPortParams is NULL ");
    }

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS_EXT(mvHwsAvagoSerdesRxSignalOkGet(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &tmpSignalOk),
                         LOG_ARG_SERDES_IDX_MAC(MV_HWS_SERDES_NUM(curLanesList[i])), LOG_ARG_STRING_MAC("RxSignalOK"));
        /* if at lease one of the serdeses is ont rx_signal_ok, then no rx_signal_ok */
        if (tmpSignalOk == GT_FALSE)
        {
            *signalOk = GT_FALSE;
            return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoSerdesRxSignalOkChangeGet function
* @endinternal
*
* @brief   Get the rx_signal_ok change indication from the port
*         lanes. If all lanes have rx_signal_ok indication, then
*         output value is true. Else, output value is false.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoSerdesRxSignalOkChangeGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL *signalOkChange
)
{
    GT_BOOL tmpSignalOkChange;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 i;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];

    HWS_NULL_PTR_CHECK_MAC(signalOkChange);

    *signalOkChange = GT_FALSE;
    tmpSignalOkChange = GT_FALSE;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS_EXT(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList),
                 LOG_ARG_GEN_PARAM_MAC(portMode), LOG_ARG_STRING_MAC("rebuilding active lanes list"));
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "RxSignalOkChangeGet- curPortParams is NULL ");
    }

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS_EXT(mvHwsAvagoSerdesSignalOkChange(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &tmpSignalOkChange),
                         LOG_ARG_SERDES_IDX_MAC(MV_HWS_SERDES_NUM(curLanesList[i])), LOG_ARG_STRING_MAC("RxSignalOKChange"));
        /* if at lease one of the serdeses is ont rx_signal_ok, then no rx_signal_ok */
        if (tmpSignalOkChange == GT_TRUE)
        {
            *signalOkChange = GT_TRUE;
        }
    }

    return GT_OK;
}

#endif /*MV_HWS_REDUCED_BUILD_EXT_CM3*/
#endif /* #if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) */

/*******************************************************************************
* mvHwsPortEnhanceTuneLitePhaseAllocation
*
* DESCRIPTION:
*       Allocate memory for EnhanceTuneLite by phase algorithm which
*       is used for AP port
*
* INPUTS:
*       devNum     - system device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
GT_STATUS mvHwsPortEnhanceTuneLitePhaseDeviceInit
(
    GT_U8   devNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
#else
    GT_U32 phyPortIndex;

    for (phyPortIndex=0; phyPortIndex < (MV_PORT_CTRL_MAX_AP_PORT_NUM); phyPortIndex++)
    {
        CHECK_STATUS(mvHwsPortEnhanceTuneLiteByPhaseInitDb(devNum, phyPortIndex));
    }
#endif

    return GT_OK;
}
#endif /* defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)*/

/*******************************************************************************
* mvHwsPortEnhanceTuneLiteByPhaseInitDb
*
* DESCRIPTION:
*       Init HWS DB of EnhanceTuneLite by phase algorithm used for AP port
*
* INPUTS:
*       devNum       - system device number
*       apPortIndex  - ap port number index
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
GT_STATUS mvHwsPortEnhanceTuneLiteByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  apPortIndex
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    apPortIndex = apPortIndex;
#else
    GT_U32 i;

    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_CurrentDelaySize = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_CurrentDelayPtr = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_InOutI = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].subPhase = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phaseEnhanceTuneLiteParams.phase = 0;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phaseEnhanceTuneLiteParams.phaseFinished = GT_FALSE;
    hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].parametersInitialized = GT_TRUE;

    for (i=0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_Best_eye[i] = 4;
        hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr[apPortIndex].phase_Best_dly[i] = 0;
    }
#endif

    return GT_OK;
}
#endif /* defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)*/

/**
* @internal mvHwsAvagoSerdesDefaultMinMaxDlyGet function
* @endinternal
*
* @brief   This function returns the default minimum and maximum delay
*         values according to the given port mode
* @param[in] portMode                 - mode type of port
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDefaultMinMaxDlyGet
(
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8                   *min_dly_ptr,
    GT_U8                   *max_dly_ptr
)
{
    HWS_NULL_PTR_CHECK_MAC(min_dly_ptr);
    HWS_NULL_PTR_CHECK_MAC(max_dly_ptr);

    if ((portMode == _10GBase_KR) ||
        (portMode == _20GBase_KR) ||
        (portMode == _40GBase_KR) ||
        (portMode == _40GBase_KR4)||
        (portMode == _40GBase_CR4))
    {
        *min_dly_ptr = 25;
        *max_dly_ptr = 30;
    }
    else if ((portMode == _25GBase_KR)    ||
             (portMode == _50GBase_KR2)   ||
             (portMode == _100GBase_KR4)  ||
             (portMode == _25GBase_KR_C)  ||
             (portMode == _50GBase_KR2_C) ||
             (portMode == _25GBase_CR)    ||
             (portMode == _50GBase_CR2)   ||
             (portMode == _100GBase_CR4)  ||
             (portMode == _25GBase_KR_S)  ||
             (portMode == _25GBase_CR_S)  ||
             (portMode == _25GBase_CR_C)  ||
             (portMode == _50GBase_CR2_C) ||
             (portMode == _50GBase_KR)    ||
             (portMode == _100GBase_KR2)  ||
             (portMode == _200GBase_KR4)  ||
             (portMode == _200GBase_KR8)  ||
             (portMode == _400GBase_KR8))
    {
        *min_dly_ptr = 15;
        *max_dly_ptr = 21;
    }
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("portMode is not supported"));
    }

    return  GT_OK;
}

#if defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
/**
* @internal mvHwsPortEnhanceTuneLiteSetByPhase function
* @endinternal
*
* @brief   This function calls Enhance-Tune Lite algorithm phases.
*         The operation is relevant only for AP port with Avago Serdes
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
* @param[in] phase                    -  to perform
* @param[in] min_dly                  - Minimum delay_cal value: (rang: 0-31)
* @param[in] max_dly                  - Maximum delay_cal value: (rang: 0-31)
*
* @param[out] phaseFinishedPtr         - whether or not phase is finished
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLiteSetByPhase
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       phase,
    GT_U8       *phaseFinishedPtr,
    GT_U8       min_dly,
    GT_U8       max_dly
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    phase = phase;
    phaseFinishedPtr = phaseFinishedPtr;
    etlMinDelay = etlMinDelay;
    etlMaxDelay = etlMaxDelay;
#else
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_PORT_AVAGO_SERDES_NUM];
    GT_STATUS rc = GT_OK;
    MV_HWS_PORT_ENHANCE_TUNE_LITE_STC   *enhanceTuneLiteStcPtr;
    GT_U8 default_min_dly, default_max_dly;

    /* For AP port: delay values for EnhanceTuneLite configuration */
    GT_U8  static_delay_10G[] = {25, 26, 28, 30}; /* best delay values for 10G speed */
    GT_U8  static_delay_25G[] = {15, 17, 19, 21}; /* best delay values for 25G speed */

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("EnhancedTuneLite null HWS param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    enhanceTuneLiteStcPtr = &(hwsDeviceSpecInfo[devNum].portEnhanceTuneLiteDbPtr
                            [MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(phyPortNum, MV_PORT_CTRL_AP_PORT_NUM_BASE)]);
    if (enhanceTuneLiteStcPtr->parametersInitialized == GT_FALSE)
    {
        mvHwsPortEnhanceTuneLiteByPhaseInitDb(devNum,
                            MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(phyPortNum, MV_PORT_CTRL_AP_PORT_NUM_BASE));
        enhanceTuneLiteStcPtr->parametersInitialized = GT_TRUE;
    }

    if(HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3)
    {
        if(curPortParams.serdesSpeed >= _20_625G)
        {
            enhanceTuneLiteStcPtr->phase_CurrentDelayPtr = static_delay_25G;
            enhanceTuneLiteStcPtr->phase_CurrentDelaySize = sizeof(static_delay_25G)/sizeof(GT_U8);
        }
        else
        {
            enhanceTuneLiteStcPtr->phase_CurrentDelayPtr = static_delay_10G;
            enhanceTuneLiteStcPtr->phase_CurrentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U8);
        }
    }
    else
    {
        enhanceTuneLiteStcPtr->phase_CurrentDelayPtr = static_delay_10G;
        enhanceTuneLiteStcPtr->phase_CurrentDelaySize = sizeof(static_delay_10G) / sizeof(GT_U8);
    }

    CHECK_STATUS(mvHwsAvagoSerdesDefaultMinMaxDlyGet(portMode, &default_min_dly, &default_max_dly));

    if ( (min_dly>>7) & 1 )
    {
        min_dly = min_dly & 0x7F;
    }
    else
    {
        min_dly = default_min_dly;
    }

    if ( (max_dly>>7) & 1 )
    {
        max_dly = max_dly & 0x7F;
    }
    else
    {
        max_dly = default_max_dly;
    }

    switch (phase)
    {
        case 1:
            rc = mvHwsAvagoSerdesEnhanceTuneLitePhase1(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                curPortParams.serdesSpeed,
                enhanceTuneLiteStcPtr->phase_CurrentDelaySize,
                enhanceTuneLiteStcPtr->phase_CurrentDelayPtr,
                &enhanceTuneLiteStcPtr->phase_InOutI,
                &(enhanceTuneLiteStcPtr->phase_Best_eye[0]),
                &(enhanceTuneLiteStcPtr->phase_Best_dly[0]),
                &(enhanceTuneLiteStcPtr->subPhase),
                min_dly,
                max_dly);
            if (rc != GT_OK)
                HWS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsAvagoSerdesEnhanceTuneLitePhase1 Fail");
            *phaseFinishedPtr = (enhanceTuneLiteStcPtr->phase_InOutI == 0xFF) ? GT_TRUE : GT_FALSE;
            break;

        case 2:
            rc = mvHwsAvagoSerdesEnhanceTuneLitePhase2(
                devNum,
                portGroup,
                &(curLanesList[0]),
                curPortParams.numOfActLanes,
                /* phase dependant args */
                &(enhanceTuneLiteStcPtr->phase_Best_dly[0]));
            if (rc != GT_OK)
                HWS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsAvagoSerdesEnhanceTuneLitePhase2 Fail");
            *phaseFinishedPtr = GT_TRUE;
            break;

        default:
            hwsOsPrintf("\n EnhancedTuneLite: phase number is not correct \n");
        break;
    }
#endif

    return GT_OK;
}
#endif /* defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)*/
/*******************************************************************************
* mvHwsPortAvagoRxTuneStartStausCheck
*
* DESCRIPTION:
*       Start the port Rx Tunning and get the status of port Rx Tunning
*       If the Rx Tunning pass, TAP1 mode will set to Enable
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
GT_STATUS mvHwsPortAvagoRxTuneStartStausCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_AUTO_TUNE_STATUS rxStatus = TUNE_NOT_COMPLITED;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i, j;
    GT_U16  rxStatusBitmap = 0; /* bit per successful rx training result */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoRxTuneStartStausCheck null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                HWS_DEV_SERDES_TYPE(devNum), GT_TRUE));
    }

    for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
    {
        /* Delay in 1ms */
        hwsOsExactDelayPtr(devNum, portGroup, 1);

        /* check the status of Rx-Training on each related serdes */
        for (j = 0; (j < HWS_MAX_SERDES_NUM) && (j < curPortParams.numOfActLanes); j++)
        {
            /* if lane tune not finished */
            if ( !(rxStatusBitmap & (1<<j)) )
            {
                /* run the Rx-Training status to enable the TAP1 if it passed */
                CHECK_STATUS(mvHwsSerdesAutoTuneStatus(devNum, (portGroup + ((curLanesList[j] >> 16) & 0xFF)),
                                                       (curLanesList[j] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), &rxStatus, NULL));
                if (rxStatus == TUNE_PASS)
                {
                    /* mark lane as finished */
                    rxStatusBitmap |= (1<<j);
                }
            }
        }
        /* if all lanes finished training */
        if ( (rxStatusBitmap & (0xFFFF>>(16-curPortParams.numOfActLanes))) ==
            (0xFFFF>>(16-curPortParams.numOfActLanes)) )
        {
            break;
        }
    }

    /* timeout handling */
    if (i == MV_AVAGO_TRAINING_TIMEOUT)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ARG_STRING_MAC("blocking rx-training reached timeout"));
    }

    return GT_OK;
}
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT) */

/**
* @internal mvHwsPortAvagoTxAutoTuneStatus function
* @endinternal
*
* @brief   Get the status of port Tx Tunning
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if SerdesAutoTuneStatus returns with TUNE_FAIL
*/
GT_STATUS mvHwsPortAvagoTxAutoTuneStatus
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_AUTO_TUNE_STATUS txStatus[HWS_MAX_SERDES_NUM];
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        return GT_BAD_PTR;
    }
    if (curPortParams.numOfActLanes > HWS_MAX_SERDES_NUM)
    {
        return GT_BAD_PARAM;
    }

    hwsOsMemSetFuncPtr(txStatus, 0, sizeof(txStatus));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesAutoTuneStatus(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum),
                                               NULL, &txStatus[i]));

        if (txStatus[i] == TUNE_FAIL)
        {
            return GT_FAIL;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoTxAutoTuneStart function
* @endinternal
*
* @brief   Start the port Tx Tunning
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoTxAutoTuneStart
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoTxAutoTuneStart null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
#ifndef ASIC_SIMULATION
        /* Set PMD PRBS Sequence (PMD Lane ID #) based on Value */
        if((_100GBase_KR4 == portMode) || (_25GBase_KR == portMode) || (_50GBase_KR2 == portMode) || (_40GBase_KR2 == portMode)
           || (_25GBase_KR_C == portMode) || (_50GBase_KR2_C == portMode) || (_25GBase_KR_S == portMode)
           || (_100GBase_CR4 == portMode) || (_25GBase_CR == portMode) || (_25GBase_CR_S == portMode) || (_25GBase_CR_C == portMode)
           || (_50GBase_CR2 == portMode)|| (_50GBase_CR2_C == portMode))
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                    0x3d, ((3<<12) | i) /* ((Select<<12) | Value) - Value here is laneNum = 0..3 */,
                    NULL));
        }
        else
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                    0x3d, ((3<<12) | 0x4) /* restore default - 0x4 here means - (Clause 72) (default) */,
                    NULL));
        }
#endif
        /* enable TX Tunning */
        CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStartExt(devNum, portGroup,
                                                        MV_HWS_SERDES_NUM(curLanesList[i]),
                                                        IGNORE_RX_TRAINING, START_TRAINING));
    }

    return GT_OK;
}

#ifndef MV_HWS_FREE_RTOS
/**
* @internal mvHwsPortEnhanceTuneLite function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] min_dly                  - Minimum delay_cal value:
*                                      - for Serdes speed 10G (25...28)
*                                      - for Serdes speed 25G (15...19)
* @param[in] max_dly                  - Maximum delay_cal value:
*                                      - for Serdes speed 10G (30)
*                                      - for Serdes speed 25G (21)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLite
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_dly,
    GT_U8       max_dly
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("light enhanced tune null hws param ptr"));
    }

    CHECK_STATUS(mvHwsAvagoSerdesDefaultMinMaxDlyGet(portMode, &min_dly, &max_dly));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    CHECK_STATUS(mvHwsAvagoSerdesEnhanceTuneLite(devNum, portGroup, curLanesList, curPortParams.numOfActLanes, curPortParams.serdesSpeed, min_dly, max_dly));

    return GT_OK;
}
#endif /* MV_HWS_FREE_RTOS */

/**
* @internal mvHwsPortAvagoDfeCfgSet function
* @endinternal
*
* @brief   Set the DFE (Rx-Traning) configuration mode on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
* @param[in] dfeMode                  - mode of DFE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if SerdesAutoTuneStatus returns with TUNE_FAIL
*/
GT_STATUS mvHwsPortAvagoDfeCfgSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_DFE_MODE         dfeMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoDfeCfgSet null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesDfeConfig(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum),
                                          dfeMode, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoRxSignalCheck function
* @endinternal
*
* @brief   Check for all Serdeses on port the Rx Signal.
*         This function need to be run before executing Rx or Tx Training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsPortAvagoRxSignalCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *txTune
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL                 signalDet;
    GT_U32                  i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoRxSignalCheck null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    *txTune = TUNE_NOT_READY;

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSignalDetectGet(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &signalDet));
        if (!signalDet)
        {
            return GT_OK;
        }
    }

    *txTune = TUNE_READY;

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoTxAutoTuneStateCheck function
* @endinternal
*
* @brief   Get the status of port Tx Tunning for Avago Serdes
*         For AP process: This function doesn't contains a polling checking on
*         Tx status
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsPortAvagoTxAutoTuneStateCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *txTune
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_AUTO_TUNE_STATUS txStatus[HWS_MAX_SERDES_NUM] = {0};
    GT_U32 i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoTxAutoTuneStateCheck null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    *txTune = TUNE_PASS;

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesTxAutoTuneStatusShort(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), &txStatus[i]));

        if (txStatus[i] != TUNE_PASS)
        {
            *txTune = txStatus[i];
            return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoTRxTuneStop function
* @endinternal
*
* @brief   Stop the TX training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsPortAvagoTRxTuneStop
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPortAvagoTRxTuneStop null hws param ptr"));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesTxAutoTuneStop(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i])));
    }

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsPortAvagoIsAPGet function
* @endinternal
*
* @brief   This function gets flag is device works in AP mode
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsPortAvagoIsAPGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroup,
    IN  GT_U32                      phyPortNum,
    IN  MV_HWS_PORT_STANDARD        portMode,
    IN  GT_BOOL                    *isAPPtr
)
{
    GT_UREG_DATA     data;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(NULL == isAPPtr)
    {
        return GT_FAIL;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_FAIL;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT,
                                    curPortParams.portPcsNumber,
                                    AP_STATUS_REG_0, &data, 0));

    *isAPPtr = (data > 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}
#endif /*# MV_HWS_REDUCED_BUILD*/

/**
* @internal mvHwsPortAvagoAutoTuneSet function
* @endinternal
*
* @brief   Calling to port Rx or Tx Tunning function
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - RxTunning or TxTunning
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoAutoTuneSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    void                        *results
)
{
    MV_HWS_AUTO_TUNE_STATUS_RES *tuneRes;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    switch (portTuningMode)
    {
        case TRxTuneCfg:
            /* TRxTuneCfg mode is not relevant for Avago Serdes */
            return GT_OK;
        case RxTrainingOnly:

            CHECK_STATUS(mvHwsPortAvagoSerdesTunePreConfigSet(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL));
            CHECK_STATUS(mvHwsPortAvagoSerdesTunePreConfigSet(devNum, portGroup, phyPortNum, portMode, GT_FALSE));

            break;
            /* blocking rx training not needed in AP FW */
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
        case RxTrainingWaitForFinish:
            /* if Rx-Training pass, we need to enable the TAP1 mode */
            CHECK_STATUS(mvHwsPortAvagoRxTuneStartStausCheck(devNum, portGroup, phyPortNum, portMode));
            break;
#endif /* (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)) */
        case TRxTuneStart:
            tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
            tuneRes->rxTune = TUNE_PASS; /* rxTune is not relevant for TxTune mode */
            CHECK_STATUS(mvHwsPortAvagoRxSignalCheck(devNum, portGroup, phyPortNum, portMode, &tuneRes->txTune));
            /* this check needed otherwise TRX training frequently fails */
            if(tuneRes->txTune == TUNE_READY)
            {
                CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStart(devNum, portGroup, phyPortNum, portMode));
            }
            if (tuneRes->txTune == TUNE_NOT_READY)
            {
                return GT_OK;
            }
            break;
        case TRxTuneStatus:
            CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStatus(devNum, portGroup, phyPortNum, portMode));
            break;
        case TRxTuneStatusNonBlocking: /* for AP mode */
            tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
            tuneRes->rxTune = TUNE_PASS; /* rxTune is not relevant for TxTune mode */
            CHECK_STATUS(mvHwsPortAvagoTxAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode, &tuneRes->txTune));
            break;
        case TRxTuneStop:
            CHECK_STATUS(mvHwsPortAvagoTRxTuneStop(devNum, portGroup, phyPortNum, portMode));
            break;
        case RxTrainingAdative: /* running Adaptive pCal */
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_START_ADAPTIVE));
            break;
        case RxTrainingStopAdative:
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_STOP_ADAPTIVE));
            break;
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
        case RxTrainingVsr:
            CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL_VSR));
            break;
#endif /* (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)) */
        default:
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("training mode not supported"));
    }

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesLunchOneShotiCal function
* @endinternal
*
* @brief   run one shot dfe tune without checking the tune
*          status
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - mode type of port
*
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesLunchOneShotiCal
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsPortAvagoDfeCfgSet(devNum, portGroup, phyPortNum, portMode, DFE_ICAL));

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoAutoTuneSetExt function
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
GT_STATUS mvHwsPortAvagoAutoTuneSetExt
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    GT_U32                      optAlgoMask,
    void                        *results
)
{
    /* avoid warnings */
    optAlgoMask = optAlgoMask;

    if ((portTuningMode == RxTrainingOnly) || (portTuningMode == TRxTuneStart) || (portTuningMode == TRxTuneStatusNonBlocking))
    {
        if (results == NULL)
        {
            return GT_BAD_PARAM;
        }
    }

    return mvHwsPortAvagoAutoTuneSet(devNum, portGroup, phyPortNum, portMode, portTuningMode, results);
}

#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)

/**
* @internal mvHwsPortVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) of BobK device
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] voltage                  - device  value (in mV)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVoltageGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      *voltage
)
{
    if (HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    /* Get the voltage (in mV) from Avago Serdes, use Serdes #20 as voltage reference of device */
    CHECK_STATUS(mvHwsAvagoSerdesVoltageGet(devNum, portGroup, 20, voltage));

    return GT_OK;
}

/**
* @internal mvHwsPortTemperatureGet function
* @endinternal
*
* @brief   Get the Temperature (in C) of BobK device
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] temperature              - device  (in C)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTemperatureGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_32       *temperature
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    temperature = temperature;
#else
    int temperature_tmp;

    if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    /* Get the Temperature (in C) from Avago Serdes, use Serdes #20 as temperature reference of device */
    CHECK_STATUS(mvHwsAvagoSerdesTemperatureGet(devNum, portGroup, 20, &temperature_tmp));
    *temperature = (GT_32)temperature_tmp; /* because GT_32 can be sometimes 'int' sometimes 'long' */

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsPortAvagoGuiExec function
* @endinternal
*
* @brief   Enable / Disable Avago GUI
*
* @param[in] devNum                   - system device number
* @param[in] enable                   - 0 = disable
*                                      1 = enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoGuiExec
(
    GT_U8 devNum,
    GT_U8 enable
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    enable = enable;
#else
#ifndef CPSS_BLOB
    static unsigned int aacsServerActiveFlag = 0;
#endif

    if (HWS_DEV_SERDES_TYPE(devNum) != AVAGO)
    {
        return GT_NOT_SUPPORTED;
    }

    enable &= MV_HWS_AVAGO_GUI_MASK;

#ifndef CPSS_BLOB
    /* Avago GUI Enable
    ** Enabling Avago GUI includes two parts
    ** 1. Activate AACS Server process
    **    Once activated this process is not removed and execute during
    **    system life-time, therefore it should only be open once.
    ** 2. Disable periodic execution of AP process
    **    This action is required only if Service CPU firmware was loaded
    **    to Service CPU (retrieved by mvHwsServiceCpuEnableGet API)
    */
    if (enable == MV_HWS_AVAGO_GUI_ENABLED)
    {
        if (aacsServerActiveFlag == 0)
        {
            aacsServerActiveFlag = 1;
            avagoSerdesAacsServerExec(devNum);
        }

        if (mvHwsServiceCpuEnableGet(devNum))
        {
            mvHwsApPortCtrlAvagoGuiSet(devNum, enable);
        }
    }
    /* Avago GUI Enable
    ** Re-Enable periodic execution of AP process
    */
    else /* enable != MV_HWS_AVAGO_GUI_ENABLED */
#endif
    {
        if (mvHwsServiceCpuEnableGet(devNum))
        {
            mvHwsApPortCtrlAvagoGuiSet(devNum, enable);
        }
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsMtipExtFecTypeGet function
* @endinternal
*
* @brief  Get port fec type
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] width           - lane width
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtFecTypeGet
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_FEC_MODE        *fecTypePtr
)
{
    GT_U32 baseAddr, unitIndex, unitNum, i;
    GT_U32 regAddr, regData, regMask, localPortIndex;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    MV_HWS_PORT_FEC_MODE        fecType;
    regData = 0;
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_FEC_CONTROL;
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(portNum);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    regMask = 0;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
    *fecTypePtr = FEC_OFF;
    fecType = FEC_OFF;
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch ((regData>>(localPortIndex+i)) & 0x1010001){
            case 1: /*fc_fec */
                if ((fecType !=FEC_OFF) && (fecType !=FC_FEC)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeGet fec invalid"));
                }
                fecType = FC_FEC;
                break;
            case 0x0010000: /*rs_fec_91*/
                if ((fecType !=FEC_OFF) && (fecType !=RS_FEC)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeGet fec invalid"));
                }
                fecType = RS_FEC;
                break;
            case 0x1010000: /* rs_fec_kp */
                if ((fecType !=FEC_OFF) && (fecType !=RS_FEC_544_514)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeGet fec invalid"));
                }
                fecType = RS_FEC_544_514;
                break;
            default: /* no fec*/
                if (fecType !=FEC_OFF) {
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtFecTypeGet fec invalid"));
                }
                break;
        }
    }

        switch (fecType) {
            case RS_FEC_544_514:
                if ((portMode != _50GBase_KR ) && (portMode != _100GBase_KR2) &&
                   (portMode != _200GBase_KR4) && (portMode != _200GBase_KR8) &&
                   (portMode != _400GBase_KR8)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtGetFecType hws bad fec type param"));
                }
                break;
            case RS_FEC:
                if ((portMode != _25GBase_KR ) && (portMode != _50GBase_KR2) &&
                   (portMode != _100GBase_KR4)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtGetFecType hws bad fec type param"));
                }
                break;
            case FC_FEC:
                if ((portMode != _5GBaseR ) && (portMode != _10GBase_KR) &&
                    (portMode != _25GBase_KR) &&(portMode != _40GBase_KR) && (portMode != _40GBase_KR4) &&
                    (portMode != _50GBase_KR2)){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtGetFecType hws bad fec type param"));
                }
                break;
            default:
                break;
         }

        *fecTypePtr = fecType;
    return GT_OK;
}
#endif /* #if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) */



