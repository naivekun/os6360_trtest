/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
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
* @file mvHwsPortInitIf.c
*
* @brief
*
* @version   55
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#endif
GT_BOOL  multiLaneConfig = GT_TRUE;
#ifdef BV_DEV_SUPPORT
GT_STATUS mvHwsBearValleySetLinePcsMode
(
     GT_U8                devNum,
     GT_U16               mdioPort,
     MV_HWS_PORT_STANDARD portMode,
     GT_U32 fecCorrect
);
#endif

extern  GT_BOOL hwsPpHwTraceFlag;

#define NEW_PORT_INIT_SEQ
/*****************************************************************************************/

GT_BOOL multiLaneConfigSupport(GT_BOOL enable)
{
    GT_BOOL oldState = multiLaneConfig;
    multiLaneConfig = enable;
    return oldState;
}

/**
* @internal mvHwsPortInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInit
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_DEV_FUNC_PTRS            *hwsDevFunc;

#ifndef MV_HWS_REDUCED_BUILD
    GT_BOOL                   lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return GT_BAD_PTR;
    }
    lbPort = portInitInParamPtr->lbPort;
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortInitIpc(devNum,portGroup, phyPortNum,portMode, lbPort, refClock, refClockSource));
#endif
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc == NULL)
    {
        return GT_BAD_PTR;
    }
NEW_PORT_INIT_SEQ
    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc(devNum, portGroup, phyPortNum, portMode, portInitInParamPtr);
}

/**
* @internal mvHwsApPortInit function
* @endinternal
*
* @brief   Init physical port for 802.3AP protocol. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortInit
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD       portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

#ifndef MV_HWS_REDUCED_BUILD
    GT_BOOL                   lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return GT_BAD_PTR;
    }
    lbPort = portInitInParamPtr->lbPort;
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION) return (mvHwsPortInitIpc(devNum, portGroup, phyPortNum, portMode, lbPort, refClock, refClockSource));
#endif
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc == NULL)
    {
        return GT_BAD_PTR;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc(devNum, portGroup, phyPortNum, portMode, portInitInParamPtr);
}

/**
* @internal mvHwsPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortReset
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION  action
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortResetIpc(devNum,portGroup,phyPortNum,portMode,action));
#endif

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc == NULL)
    {
        return GT_BAD_PTR;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc(devNum, portGroup, phyPortNum, portMode, action);
}

/**
* @internal mvHwsPortFlavorInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly to specified flavor.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFlavorInit
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{

    return mvHwsPortInit(devNum,portGroup,phyPortNum,portMode, portInitInParamPtr);
}

/**
* @internal mvHwsPortValidate function
* @endinternal
*
* @brief   Validate port API's input parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortValidate
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    /* validation */
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        (portMode >= (MV_HWS_PORT_STANDARD)HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* In MLG mode only first port number in the GOP has 4 active lanes,
        all other three ports have 0 active lane */
    if (((GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) || (curPortParams.numOfActLanes == 0)) &&
        (portMode != _100GBase_MLG))
    {
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal mvHwsPortLoopbackValidate function
* @endinternal
*
* @brief   Validate loopback port input parameters.
*         In MMPCS mode: the ref_clk comes from ExtPLL, thus the Serdes can be in power-down.
*         In all other PCS modes: there is no ExtPLL, thus the ref_clk comes is taken
*         from Serdes, so the Serdes should be in power-up.
* @param[in] portPcsType              - port pcs type
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] serdesInit               - if true, init port serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackValidate
(
    GT_U32  portPcsType,
    GT_BOOL lbPort,
    GT_BOOL *serdesInit
)
{
    if (lbPort == GT_TRUE)
    {
        /* in MMPCS PCS mode for Loopback mode, the ref_clk comes from ExtPLL,
           so the Serdes can be in power-down */
        if ((MV_HWS_PORT_PCS_TYPE)portPcsType == MMPCS)
        {
            *serdesInit = GT_FALSE;
        }
        else /* for all other PCS modes in Loopback mode, there is no ExtPLL,
                so the Serdes should be in power-up */
        {
            *serdesInit = GT_TRUE;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/


GT_STATUS mvHwsPortSerdesPowerUp
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_REF_CLOCK_SUP_VAL    refClock,
    MV_HWS_REF_CLOCK_SOURCE     refClockSource,
    GT_U32                      *curLanesList
)
{
    GT_U32 i;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    MV_HWS_SERDES_CONFIG_STR        serdesConfig;
    MV_HWS_SERDES_ENCODING_TYPE     serdesEncoding;
    GT_U32                          numOfActLanes;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }
    numOfActLanes = curPortParams.numOfActLanes;

    serdesConfig.baudRate       = curPortParams.serdesSpeed;
    serdesConfig.media          = curPortParams.serdesMediaType;
    serdesConfig.busWidth       = curPortParams.serdes10BitStatus;
    serdesConfig.refClock       = refClock;
    serdesConfig.refClockSource = refClockSource;
    serdesConfig.serdesType     = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum));
    switch (serdesConfig.baudRate)
    {
        case _26_5625G_PAM4:      /* for NRZ mode at 26.5625G, for PAM4 mode at 53.125G */
        case _28_28125G_PAM4:     /* for NRZ mode at 28.28125G, for PAM4 mode at 56.5625G */
            serdesEncoding = SERDES_ENCODING_AVAGO_PAM4;
            break;
        default:
            serdesEncoding = SERDES_ENCODING_NA;
            break;
    }
    serdesConfig.rxEncoding = serdesEncoding;
    serdesConfig.txEncoding = serdesEncoding;

    /* power up the serdes */
    if (multiLaneConfig == GT_TRUE)
    {
        CHECK_STATUS(mvHwsSerdesArrayPowerCtrl(devNum, portGroup, numOfActLanes,
                                              curLanesList, GT_TRUE, &serdesConfig));
    }
    else
    { /* on each related serdes */
        for (i = 0; i < numOfActLanes; i++)
        {
            CHECK_STATUS(mvHwsSerdesPowerCtrl(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), GT_TRUE, &serdesConfig));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortApSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes for 802.3AP protocol.
*         assumes: parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesPowerUp
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_REF_CLOCK_SUP_VAL    refClock,
    MV_HWS_REF_CLOCK_SOURCE refClockSource,
    GT_U32 *curLanesList
)
{
    GT_U32 i;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_SERDES_CONFIG_STR    serdesConfig;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }


    serdesConfig.baudRate = curPortParams.serdesSpeed;
    serdesConfig.media = curPortParams.serdesMediaType;
    serdesConfig.busWidth = curPortParams.serdes10BitStatus;
    serdesConfig.refClock = refClock;
    serdesConfig.refClockSource = refClockSource;
    serdesConfig.rxEncoding = SERDES_ENCODING_NA;
    serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum));
    serdesConfig.txEncoding = SERDES_ENCODING_NA;

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesApPowerCtrl(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                            (curLanesList[i] & 0xFFFF), GT_TRUE, &serdesConfig));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortModeSquelchCfg function
* @endinternal
*
* @brief   reconfigure default squelch threshold value only for KR (CR) modes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] curLanesList             - active Serdes lanes list according to configuration
* @param[in] txAmp                    - Reference clock frequency
* @param[in] emph0                    - Reference clock source line
* @param[in] emph1                    - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortModeSquelchCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32  *curLanesList,
    GT_U32  txAmp,
    GT_U32  emph0,
    GT_U32  emph1
)
{
    GT_U32 i;
    GT_U32 squelchVal;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    switch (portMode)
    {
        case _10GBase_KR:
        case _20GBase_KR:
        case _40GBase_KR:
        case _40GBase_KR4:
        case _40GBase_CR4:
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                mvHwsSerdesSqlchCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), 2);
            }
            break;
        case _100GBase_KR10:
        case _12_1GBase_KR:
        case _12_5GBase_KR:
        case XLHGL_KR4:
        case RXAUI:
        case _12GBaseR:
        case _5_625GBaseR:
        case _48GBaseR:
        case _12GBase_SR:
        case _48GBase_SR:
        case _5GBaseR:
            /* on each related serdes */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                mvHwsSerdesSqlchCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), 2);
                mvHwsSerdesManualTxConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                          (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), 26, GT_TRUE, 3, 3, GT_TRUE, GT_TRUE);
            }
            break;
        case _10GBase_SR_LR:
        case _20GBase_SR_LR:
        case _40GBase_SR_LR:
            squelchVal = (HWS_DEV_SERDES_TYPE(devNum) <= COM_PHY_H_REV2) ? 6 : 2;

            /* on each related serdes */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                mvHwsSerdesSqlchCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum), squelchVal);
            }
        case _100GBase_SR10:
            /* on each related serdes transmitter setting needs to be tuned to compliance XLPPI eye mask on a compliance board such as 32QSFP */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                mvHwsSerdesManualTxConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF),
                                          HWS_DEV_SERDES_TYPE(devNum), txAmp, GT_TRUE, emph0, emph1, GT_FALSE, GT_TRUE);
            }
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal mvHwsPortModeCfg function
* @endinternal
*
* @brief   configures MAC and PCS components
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortModeCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsPortModeCfg ******\n");
    }
#endif

#ifndef BV_DEV_SUPPORT
    /* set PCS mode */
    if ((curPortParams.portPcsType != GPCS) && (curPortParams.portPcsType != GPCS_NET)
             && (curPortParams.portPcsType != QSGMII_PCS))
    {
        CHECK_STATUS(mvHwsPcsModeCfg(devNum, portGroup, phyPortNum, portMode));
    }

    /* set MAC mode */
    CHECK_STATUS(mvHwsMacModeCfg(devNum, portGroup, phyPortNum, portMode));
#else
    CHECK_STATUS(mvHwsBearValleySetLinePcsMode(devNum, phyPortNum, portMode, curPortParams.portFecMode));
#endif

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsPortModeCfg ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsPortStartCfg function
* @endinternal
*
* @brief   Unreset MAC and PCS components
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortStartCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    portPcsType;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }
    portPcsType = curPortParams.portPcsType;
#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsPortStartCfg ******\n");
    }
#endif

    if ((portPcsType != GPCS) && (portPcsType != GPCS_NET) && (portPcsType != QSGMII_PCS))
    {
        if((MMPCS == portPcsType) || (CGPCS == portPcsType))
        {
            if(((curPortParams.portStandard == _25GBase_KR) || (curPortParams.portStandard == _25GBase_SR) || (curPortParams.portStandard == _25GBase_KR_C) || (curPortParams.portStandard == _25GBase_CR)|| (curPortParams.portStandard == _25GBase_CR_C))
                && (curPortParams.portFecMode == RS_FEC))
            {
                portPcsType = CGPCS;
            }
            if(HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard) && (curPortParams.portFecMode == FC_FEC))
            {
                /* only slot 0 in MSPG can work with 50G and FC-FEC mode */
                if(curPortParams.portMacNumber % 4 != 0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                portPcsType = MMPCS;
            }
        }

        /* pcs unreset */
        CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, curPortParams.portStandard, portPcsType, UNRESET));
    }

    /* run RF (digital reset / unreset */
    if ((portPcsType == XPCS) || (portPcsType == MMPCS && curPortParams.numOfActLanes > 1))
    {
        CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 1));
    }

    /* In Falcon MAC UNRESET is part of mvHwsPortModeCfg() */
    if(HWS_DEV_SILICON_TYPE(devNum) != Falcon)
    {
        /* mac unreset */
        CHECK_STATUS(mvHwsMacReset(devNum, portGroup,
                        curPortParams.portMacNumber, portMode,
                        curPortParams.portMacType, UNRESET));
    }
#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsPortStartCfg ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsPortStopCfg function
* @endinternal
*
* @brief   Reset MAC and PCS components
*         Port power down on each related serdes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - port power down or reset
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*                                      reset pcs        - reset pcs option
*                                      reset mac        - reset mac option
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortStopCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_ACTION action,
    GT_U32 *curLanesList,
    MV_HWS_RESET reset_pcs,
    MV_HWS_RESET reset_mac
)
{
    GT_U32 i;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_SERDES_CONFIG_STR    serdesConfig;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    portPcsType = curPortParams.portPcsType;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    serdesConfig.baudRate = curPortParams.serdesSpeed;
    serdesConfig.media = curPortParams.serdesMediaType;
    serdesConfig.busWidth = curPortParams.serdes10BitStatus;
    serdesConfig.refClock = _156dot25Mhz;
    serdesConfig.refClockSource = PRIMARY_LINE_SRC;
    serdesConfig.rxEncoding = SERDES_ENCODING_NA;
    serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum));
    serdesConfig.txEncoding = SERDES_ENCODING_NA;


    /* PORT_POWER_DOWN on each related serdes */
    for (i = 0; (PORT_POWER_DOWN == action) && (i < curPortParams.numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesPowerCtrl(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF),
                          GT_FALSE, &serdesConfig));
    }

    if((MMPCS == portPcsType) || (CGPCS == portPcsType))
    {
        if((curPortParams.portFecMode == RS_FEC) &&
           ((portMode == _25GBase_KR) || (portMode == _25GBase_SR) || (portMode == _25GBase_CR) || (portMode == _25GBase_KR_C) || (portMode == _25GBase_CR_C)))
        {
            portPcsType = CGPCS;
        }
        if((curPortParams.portFecMode == FC_FEC) && HWS_TWO_LANES_MODE_CHECK(curPortParams.portStandard))
        {
            /* only slot 0 in MSPG can work with 50G and FC-FEC mode */
            if(curPortParams.portMacNumber % 4 != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            portPcsType = MMPCS;
        }
    }
    CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, portPcsType, reset_pcs));
    CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, reset_mac));

    if ((curPortParams.portMacType == GEMAC_SG) ||
        (curPortParams.portMacType == GEMAC_X))
    {
        CHECK_STATUS(mvHwsMacModeSelectDefaultCfg(devNum, portGroup,
                                    curPortParams.portMacNumber,GEMAC_X));
    }

    return GT_OK;
}


