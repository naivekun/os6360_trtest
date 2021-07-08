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
* mvHwsSerdesIf.c
*
* DESCRIPTION:
*       Serdes specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 58 $
*
*******************************************************************************/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

#ifndef CO_CPU_RUN
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2If.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyHDb.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2Db.h>
#endif
#ifdef HWS_DEBUG
extern GT_FILEP fdHws;
#endif
#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>

#endif

/************************* Globals *******************************************************/

static MV_HWS_SERDES_FUNC_PTRS hwsSerdesFuncsPtr[HWS_MAX_DEVICE_NUM][SERDES_LAST];

/**
* @internal mvHwsSerdesPowerCtrl function
* @endinternal
*
* @brief   Init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] powerUp                  - true for power UP
*                                      baudRate  -
*                                      refClock  - ref clock value
*                                      refClockSource - ref cloack source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPowerCtrl
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if (serdesConfigPtr->serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    if (hwsSerdesFuncsPtr[devNum][serdesConfigPtr->serdesType].serdesPowerCntrlFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Power Control: serdes %s.\n",
        hwsSerdesFuncsPtr[devNum][serdesConfigPtr->serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

#ifndef MV_HWS_REDUCED_BUILD
    if(!cpssDeviceRunCheck_onEmulator())
#endif
    {
        CHECK_STATUS(hwsSerdesFuncsPtr[devNum][serdesConfigPtr->serdesType].serdesPowerCntrlFunc(devNum,portGroup,serdesNum,powerUp,serdesConfigPtr));
    }
    return GT_OK;
}

/**
* @internal mvHwsSerdesApPowerCtrl function
* @endinternal
*
* @brief   Init physical port for 802.3AP protocol
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] powerUp                  - true for power UP
*                                      baudRate  -
*                                      refClock  - ref clock value
*                                      refClockSource - ref cloack source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesApPowerCtrl
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
    if (serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if (serdesConfigPtr->serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }

    if (hwsSerdesFuncsPtr[devNum][serdesConfigPtr->serdesType].serdesApPowerCntrlFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    CHECK_STATUS(hwsSerdesFuncsPtr[devNum][serdesConfigPtr->serdesType].serdesApPowerCntrlFunc(devNum,portGroup,serdesNum,powerUp,serdesConfigPtr));

    return GT_OK;
}

#ifndef CO_CPU_RUN
/*******************************************************************************
* mvHwsSerdesManualRxConfig
*
* DESCRIPTION:
*       Per SERDES configure the RX parameters: squelch Threshold, FFE and DFE
*       operation
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       portTuningMode - lane tuning mode (short / long)
*       sqlch     - Squelch threshold
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesManualRxConfig
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    GT_U32              sqlch,
    GT_U32              ffeRes,
    GT_U32              ffeCap,
    GT_BOOL             dfeEn,
    GT_U32              alig
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesManualRxCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }
#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsSerdesManualRxConfigIpc(devNum,portGroup,serdesNum,serdesType,portTuningMode,sqlch,ffeRes,ffeCap,dfeEn,alig));
#endif

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Manual Rx Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    return hwsSerdesFuncsPtr[devNum][serdesType].serdesManualRxCfgFunc(devNum,portGroup,serdesNum,portTuningMode,sqlch,ffeRes,ffeCap,dfeEn,alig);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))*/

/**
* @internal mvHwsSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfig
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_U32              txAmp,
    GT_BOOL             txAmpAdj,
    GT_32               emph0,
    GT_32               emph1,
    GT_BOOL             txAmpShft,
    GT_BOOL             txEmphEn
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesManualTxCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }
#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsSerdesManualTxConfigIpc(devNum,portGroup,serdesNum,serdesType,txAmp,txAmpAdj,emph0,emph1,txAmpShft));
#endif

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Manual Tx Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    return hwsSerdesFuncsPtr[devNum][serdesType].serdesManualTxCfgFunc(devNum,portGroup,serdesNum,txAmp, txAmpAdj,emph0,emph1, txAmpShft, txEmphEn);
}

/**
* @internal mvHwsSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] configParams             - Manual Tx params structures:
*                                      txAmp       - Tx amplitude
*                                      txAmpAdj    - TX adjust
*                                      txemph0     - TX emphasis 0
*                                      txemph1     - TX emphasis 1
*                                      txAmpShft   - TX amplitude shift
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfigGet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    MV_HWS_SERDES_TYPE              serdesType,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesManualTxCfgGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Manual Tx Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    return hwsSerdesFuncsPtr[devNum][serdesType].serdesManualTxCfgGetFunc(devNum, portGroup, serdesNum, configParams);
}

/**
* @internal mvHwsSerdesLoopback function
* @endinternal
*
* @brief   Perform an internal loopback (SERDES loopback) for debug/BIST use.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of serdes (LP/COM_PHY...)
* @param[in] lbMode                   - loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesLoopback
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    MV_HWS_SERDES_LB    lbMode
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesLoopbackCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Loopback Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    return hwsSerdesFuncsPtr[devNum][serdesType].serdesLoopbackCfgFunc(devNum,portGroup,serdesNum,lbMode);
}


/**
* @internal mvHwsSerdesLoopbackGet function
* @endinternal
*
* @brief   Get status of internal loopback (SERDES loopback).
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of serdes (LP/COM_PHY...)
*
* @param[out] lbModePtr                - current loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesLoopbackGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    MV_HWS_SERDES_LB    *lbModePtr
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesLoopbackGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Loopback Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif
    return hwsSerdesFuncsPtr[devNum][serdesType].serdesLoopbackGetFunc(devNum,portGroup,serdesNum,lbModePtr);
}

/**
* @internal mvHwsSerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTestGen
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    MV_HWS_SERDES_TYPE        serdesType,
    MV_HWS_SERDES_TEST_GEN_MODE mode
)
{

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesTestGenFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Test Generator: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesTestGenFunc(devNum,portGroup,serdesNum,txPattern,mode);
}

/**
* @internal mvHwsSerdesTestGenGet function
* @endinternal
*
* @brief   Get configuration of the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - type of SerDes
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on wrong serdes type
* @retval GT_FAIL                  - HW error
*/
GT_STATUS mvHwsSerdesTestGenGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TYPE        serdesType,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesTestGenGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Get Test Generator: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesTestGenGetFunc(devNum, portGroup, serdesNum,
                                            txPatternPtr, modePtr);
}

/**
* @internal mvHwsSerdesTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TYPE        serdesType,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
)
{

    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesTestGenStatusFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Test Generator Status: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesTestGenStatusFunc(devNum, portGroup, serdesNum, txPattern, counterAccumulateMode, status);
}

/*******************************************************************************
* mvHwsSerdesEOMGet
*
* DESCRIPTION:
*       Returns the horizontal/vertical Rx eye margin.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       timeout   - wait time in msec
*
* OUTPUTS:
*       horizontal/vertical Rx eye margin and EOM status.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesEOMGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TYPE        serdesType,
    GT_U32                    timeout,
    MV_HWS_SERDES_EOM_RES     *results
)
{
    if ((NULL == results) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesEomGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes EOM: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesEomGetFunc(devNum, portGroup, serdesNum, timeout, results);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/*******************************************************************************
* mvHwsSerdesPpmConfig
*
* DESCRIPTION:
*       Per SERDES add ability to add/sub PPM from main baud rate.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesPpmConfig
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    MV_HWS_PPM_VALUE    ppmVal
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesPpmCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PPM Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesPpmCfgFunc(devNum,portGroup,serdesNum,ppmVal);
}


/**
* @internal mvHwsSerdesPpmConfigGet function
* @endinternal
*
* @brief   Per SERDES read PPM value.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] ppmVal                   - PPM current value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPpmConfigGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    MV_HWS_PPM_VALUE    *ppmVal
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesPpmGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesPpmGetFunc(devNum,portGroup,serdesNum,ppmVal);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */
#endif /* CO_CPU_RUN */

/*******************************************************************************
* mvHwsSerdesAutoTuneCfg
*
* DESCRIPTION:
*       Per SERDES configure parameters for TX training & Rx Training starting
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       rxTraining - Rx Training (true/false)
*       txTraining - Tx Training (true/false)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesAutoTuneCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Auto Tune Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneCfgFunc(devNum,portGroup,serdesNum,rxTraining,txTraining);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true/false)
*                                      txTraining - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_BOOL rxTraining
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesRxAutoTuneStartFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run RX Auto Tune Start: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesRxAutoTuneStartFunc(devNum,portGroup,serdesNum,rxTraining);
}

/**
* @internal mvHwsSerdesTxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      rxTraining - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxAutoTuneStart
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_BOOL txTraining
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesTxAutoTuneStartFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run TX Auto Tune Start: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesTxAutoTuneStartFunc(devNum,portGroup,serdesNum,txTraining);
}

/**
* @internal mvHwsSerdesAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStart
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneStartFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Auto Tune Start: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneStartFunc(devNum,portGroup,serdesNum,rxTraining,txTraining);
}


/**
* @internal mvHwsSerdesAutoTuneResult function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] results                  - the adapted tuning results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneResult
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_AUTO_TUNE_RESULTS *results
)
{

    if ((results == NULL) || (serdesType >= SERDES_LAST) )
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneResultFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Auto Tune Result: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneResultFunc(devNum,portGroup,serdesNum,results);
}

/**
* @internal mvHwsSerdesAutoTuneStatus function
* @endinternal
*
* @brief   Per SERDES check the Rx & Tx training status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                 - Rx Training status.
* @param[out] txStatus                 - Tx Training status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneStatusFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }


#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Auto Tune Status: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneStatusFunc(devNum,portGroup,serdesNum,rxStatus,txStatus);
}

/**
* @internal mvHwsSerdesReset function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesReset
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset
)
{
    if (serdesType >= SERDES_LAST)
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_SERDES_IDX_MAC(serdesNum));
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesResetFunc == NULL)
    {
        CHECK_STATUS_EXT(GT_NOT_IMPLEMENTED, LOG_ARG_SERDES_IDX_MAC(serdesNum));
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Reset: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesResetFunc(devNum,portGroup,serdesNum,analogReset,digitalReset,syncEReset);
}

/**
* @internal mvHwsSerdesResetExt function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
* @param[in] coreReset                - Core Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesResetExt
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset,
    GT_BOOL    coreReset
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesResetExtFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Reset Extended: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesResetExtFunc(devNum,portGroup,serdesNum,analogReset,digitalReset,syncEReset, coreReset);
}

/**
* @internal mvHwsSerdesDfeConfig function
* @endinternal
*
* @brief   Per SERDES configure the DFE parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] dfeMode                  - Enable/disable/freeze/Force
* @param[in] dfeCfg                   - array of dfe configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDfeConfig
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    MV_HWS_DFE_MODE   dfeMode,
    GT_32            *dfeCfg
)
{
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeCfgFunc(devNum,portGroup,serdesNum,dfeMode,(GT_REG_DATA *)dfeCfg);
}


/*******************************************************************************
* mvHwsSerdesDfeStatus
*
* DESCRIPTION:
*       Per SERDES get the DFE status and parameters.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       dfeLock   - Locked/Not locked
*       dfeCfg    - array of dfe configuration parameters
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesDfeStatus
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL           *dfeLock,
    GT_32             *dfeCfg
)
{

    if ((dfeCfg == NULL) || (dfeLock == NULL) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeStatusFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Status: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeStatusFunc(devNum,portGroup,serdesNum,dfeLock,dfeCfg);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/*******************************************************************************
* mvHwsSerdesDfeStatusExt
*
* DESCRIPTION:
*       Per SERDES get the of DFE status and parameters (extended API).
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       dfeLock   - Locked/Not locked
*       dfeCfg    - array of dfe configuration parameters
*       f0Cfg     - array of f0 configuration parameters
*       savCfg    - array of sav configuration parameters
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesDfeStatusExt
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL           *dfeLock,
    GT_32             *dfeCfg,
    GT_32             *f0Cfg,
    GT_32             *savCfg
)
{

    if ((dfeCfg == NULL) || (dfeLock == NULL) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeStatusExtFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Status: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeStatusExtFunc(devNum,portGroup,serdesNum,dfeLock,
                                                                (GT_REG_DATA *)dfeCfg, (GT_REG_DATA *)f0Cfg, (GT_REG_DATA *)savCfg);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesPolarityConfig function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[in] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPolarityConfig
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL           invertTx,
    GT_BOOL           invertRx
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesPolarityCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Polarity Config: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesPolarityCfgFunc(devNum,portGroup, serdesNum,
                                                    invertTx, invertRx);
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsSerdesPolarityConfigGet function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[out] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPolarityConfigGet
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL           *invertTx,
    GT_BOOL           *invertRx
)
{
    if ((invertTx == NULL) || (invertRx == NULL) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesPolarityCfgGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Polarity Get: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesPolarityCfgGetFunc(devNum, portGroup, serdesNum,
                                            invertTx, invertRx);
}
#endif

/*******************************************************************************
* mvHwsSerdesDfeOpti
*
* DESCRIPTION:
*       Per SERDES find best FFE_R is the min value for scanned DFE range.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       ffeR   - best value
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesDfeOpti
(
  GT_U8   devNum,
  GT_U32  portGroup,
  GT_U32  serdesNum,
  MV_HWS_SERDES_TYPE serdesType,
  GT_U32  *ffeR
)
{
    if ((ffeR == NULL) || (serdesType >= SERDES_LAST))
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeOptiFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Optimal: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeOptiFunc(devNum, portGroup, serdesNum, (GT_UREG_DATA *)ffeR);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/*******************************************************************************
* mvHwsSerdesFfeConfig
*
* DESCRIPTION:
*       Per SERDES configure FFE.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       ffeRes    - FFE R value
*       ffeCap    - FFE C value
*       align     - sampler value
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesFfeConfig
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_U32  ffeRes,
    GT_U32  ffeCap,
    GT_U32  alig
)
{
    if (serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesFfeCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run DFE Optimal: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesFfeCfgFunc(devNum, portGroup, serdesNum, ffeRes, ffeCap, alig);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/*******************************************************************************
* mvHwsSerdesCalibrationStatus
*
* DESCRIPTION:
*       Per SERDES return the calibration results.
*       Can be run after create port
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       results - the calibration results.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesCalibrationStatus
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    MV_HWS_SERDES_TYPE          serdesType,
    MV_HWS_CALIBRATION_RESULTS  *results
)
{
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesCalibrationStatusFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Calibration Status: serdes %s.\n",
        hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesCalibrationStatusFunc(devNum, portGroup, serdesNum, results);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesTxEnable function
* @endinternal
*
* @brief   Enable/Disable Tx.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] enable                   - whether to  or disable Tx.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxEnable
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_BOOL             enable
)
{
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesTxEnableFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Tx Disable: serdes %s.\n",
        hwsSerdesFuncsPtr[devNum][serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesTxEnableFunc(devNum, portGroup, serdesNum, enable);
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsSerdesTxEnableGet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] enable                   - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, serdesNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
GT_STATUS mvHwsSerdesTxEnableGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL *enable
)
{
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }

    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesTxEnableGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesTxEnableGetFunc(devNum, portGroup, serdesNum, enable);
}

/**
* @internal hwsSerdesSeqGet function
* @endinternal
*
* @brief   Get SERDES sequence one line.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesSeqGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    MV_HWS_COM_PHY_H_SUB_SEQ seqType,
    MV_CFG_ELEMENT *seqLine,
    GT_U32 lineNum
)
{
    MV_HWS_SERDES_TYPE serdesType;
    devNum = devNum;
    portGroup = portGroup;

    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum));
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesSeqGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesSeqGetFunc(seqType, seqLine, lineNum);
}

/**
* @internal hwsSerdesSeqSet function
* @endinternal
*
* @brief   Set SERDES sequence one line.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesSeqSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_BOOL firstLine,
    MV_HWS_COM_PHY_H_SUB_SEQ seqType,
    MV_CFG_ELEMENT *seqLine,
    GT_U32 numOfOp
)
{
    MV_HWS_SERDES_TYPE serdesType;
    devNum = devNum;
    portGroup = portGroup;

    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum));
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesSeqSetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesSeqSetFunc(firstLine, seqType, seqLine, numOfOp);
}
#endif

/**
* @internal mvHwsSerdesTxIfSelect function
* @endinternal
*
* @brief   tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxIfSelect
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_U32              serdesTxIfNum
)
{
    if (serdesType >= SERDES_LAST)
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_SERDES_IDX_MAC(serdesNum));
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesTxIfSelectFunc == NULL)
    {
        CHECK_STATUS_EXT(GT_NOT_IMPLEMENTED, LOG_ARG_SERDES_IDX_MAC(serdesNum));
    }

    CHECK_STATUS_EXT(hwsSerdesFuncsPtr[devNum][serdesType].serdesTxIfSelectFunc(devNum, portGroup, serdesNum, serdesTxIfNum),
                      LOG_ARG_SERDES_IDX_MAC(serdesNum));
    return GT_OK;
}

/**
* @internal mvHwsSerdesTxIfSelectGet function
* @endinternal
*
* @brief   Return the SERDES Tx interface select number.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - SERDES types
*
* @param[out] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxIfSelectGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_U32              *serdesTxIfNum
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesTxIfSelectGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesTxIfSelectGetFunc(devNum, portGroup, serdesNum, serdesTxIfNum);
    return rc;
}

/*******************************************************************************
* mvHwsSerdesSqlchCfg
*
* DESCRIPTION:
*       Configure squelch threshold value.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       serdesType - serdes type
*       sqlchVal  - squelch threshold value.
*
* OUTPUTS:
*       results - the calibration results.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesSqlchCfg
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_U32              sqlchVal
)
{
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesSqlchCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesSqlchCfgFunc(devNum, portGroup, serdesNum, sqlchVal);
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check Rx or Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesAutoTuneStatusShort
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneStatusShortFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneStatusShortFunc(devNum, portGroup, serdesNum, rxStatus, txStatus);
}

/**
* @internal mvHwsSerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Per SERDES stop the TX training & Rx Training process
*         Can be run after create port and training start.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesTxAutoTuneStop
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE  serdesType
)
{
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneStopFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsSerdesFuncsPtr[devNum][serdesType].serdesAutoTuneStopFunc(devNum, portGroup, serdesNum);
}

/*******************************************************************************
* mvHwsSerdesFixAlign90Start
*
* DESCRIPTION:
*       Start fix Align90 process on current SERDES.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       serdesType - serdes type
*       params    - pointer to structure to store SERDES configuration parameters that must be restore
*                   during process stop operation
*
* OUTPUTS:
*       params    - SERDES configuration parameters that must be restore
*                   during process stop operation
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesFixAlign90Start
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_ALIGN90_PARAMS   *params
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesFixAlign90Start == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesFixAlign90Start(devNum, portGroup, serdesNum, params);
    return rc;
}

/**
* @internal mvHwsSerdesFixAlign90Status function
* @endinternal
*
* @brief   Return fix Align90 process current status on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFixAlign90Status
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_AUTO_TUNE_STATUS *trainingStatus
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesFixAlign90Status == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesFixAlign90Status(devNum, portGroup, serdesNum, trainingStatus);
    return rc;
}

/**
* @internal mvHwsSerdesFixAlign90Stop function
* @endinternal
*
* @brief   Stop fix Align90 process on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] params                   - SERDES parameters that must be restored (return by mvHwsComHRev2SerdesFixAlign90Start)
* @param[in] fixAlignPass             - true, if fix Align90 process passed; false otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFixAlign90Stop
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_ALIGN90_PARAMS   *params,
    GT_BOOL                 fixAlignPass
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesFixAlign90Stop == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesFixAlign90Stop(devNum, portGroup, serdesNum, params, fixAlignPass);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesRev2DfeCheck function
* @endinternal
*
* @brief   Check DFE values range and start sampler calibration, if needed.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesRev2DfeCheck
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeCheckFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesDfeCheckFunc(devNum, portGroup, serdesNum);
    return rc;
}

/**
* @internal mvHwsSerdesSpeedGet function
* @endinternal
*
* @brief   Return SERDES baud rate.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSpeedGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    MV_HWS_SERDES_SPEED *rate
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesSpeedGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesSpeedGetFunc(devNum, portGroup, serdesNum, rate);
    return rc;
}

/**
* @internal mvHwsSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Return SERDES RX parameters configured manually .
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
*
* @param[out] sqlchPtr                 - Squelch threshold.
* @param[out] ffeResPtr                - FFE R
* @param[out] ffeCapPtr                - FFE C
* @param[out] aligPtr                  - align 90 value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualRxConfigGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT MV_HWS_PORT_MAN_TUNE_MODE *portTuningModePtr,
    OUT GT_U32             *sqlchPtr,
    OUT GT_U32             *ffeResPtr,
    OUT GT_U32             *ffeCapPtr,
    OUT GT_BOOL            *dfeEnPtr,
    OUT GT_U32             *aligPtr
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesManualRxCfgGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesManualRxCfgGetFunc(devNum, portGroup, serdesNum, portTuningModePtr,
                                                                sqlchPtr, ffeResPtr, ffeCapPtr, dfeEnPtr, aligPtr);
    return rc;
}

/**
* @internal mvHwsSerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      serdesType - serdes type
* @param[in] numOfSer                 - number of SERDESes to configure
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] powerUp                  - true for power UP
*                                      baudRate  -
*                                      refClock  - ref clock value
*                                      refClockSource - ref clock source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesArrayPowerCtrl
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      numOfSer,
    GT_U32                      *serdesArr,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
    GT_STATUS rc = GT_OK;

    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if (serdesConfigPtr->serdesType >= SERDES_LAST)
    {
      return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesConfigPtr->serdesType].serdesArrayPowerCntrlFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run Serdes Power Control: serdes %s.\n",
      hwsSerdesFuncsPtr[devNum][serdesConfigPtr->serdesType].serdesTypeGetFunc());
    osFclose(fdHws);
#endif
#ifndef MV_HWS_REDUCED_BUILD
    if(!cpssDeviceRunCheck_onEmulator())
#endif
    {
        rc = hwsSerdesFuncsPtr[devNum][serdesConfigPtr->serdesType].serdesArrayPowerCntrlFunc(devNum,portGroup,numOfSer,(GT_UOPT *)serdesArr,powerUp,serdesConfigPtr);
    }
    return rc;
}

/*******************************************************************************
* mvHwsSerdesAcTerminationCfg
*
* DESCRIPTION:
*       Configures AC termination on current serdes.
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       serdesNum  - serdes number
*       acTermEn   - enable or disable AC termination
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesAcTerminationCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    GT_BOOL                 acTermEn
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesAcTermCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesAcTermCfgFunc(devNum,portGroup,serdesNum,acTermEn);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/**
* @internal mvHwsSerdesDigitalReset function
* @endinternal
*
* @brief   Run digital reset / unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDigitalReset
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    MV_HWS_RESET        digitalReset
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesDigitalReset == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesDigitalReset(devNum,portGroup,serdesNum,digitalReset);
    return rc;
}

/**
* @internal mvHwsSerdesSqDetectCfg function
* @endinternal
*
* @brief   Serdes sq_detect mask configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] sqDetect                 - true to enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSqDetectCfg
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_BOOL             sqDetect
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesSqDetectCfgFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesSqDetectCfgFunc(devNum,portGroup,serdesNum,sqDetect);
    return rc;
}

/**
* @internal mvHwsSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on PCS (true/false).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
*
* @param[out] signalDet                - if true, signal was detected
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSignalDetectGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    GT_BOOL                 *signalDet
)
{
    GT_STATUS rc;
    *signalDet = GT_FALSE; /* Init the value to avoid "non initialized" problem */
    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesSignalDetectGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }
    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesSignalDetectGetFunc(devNum,portGroup,serdesNum,signalDet);
    return rc;
}

/*******************************************************************************
* mvHwsSerdesCdrLockStatusGet
*
* DESCRIPTION:
*       Return SERDES CDR lock status (true - locked /false - notlocked).
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       serdesNum  - physical PCS number
*       serdesType - serdes PCS type
*       cdrLock    - if true, CDR was locked
*
* OUTPUTS:
*       cdrLock  - if true, CDR was locked
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    GT_BOOL                 *cdrLock
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesCdrLockStatusGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesCdrLockStatusGetFunc(devNum,portGroup,serdesNum,cdrLock);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

/*******************************************************************************
* mvHwsSerdesScanSampler
*
* DESCRIPTION:
*       Scan SERDES sampler.
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       serdesNum  - physical PCS number
*       serdesType - serdes PCS type
*
* OUTPUTS:
*       bestValue  - sampler value
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
GT_STATUS mvHwsSerdesScanSampler
(
    GT_U8                  devNum,
    GT_U8                  portGroup,
    GT_U32                 serdesNum,
    MV_HWS_SERDES_TYPE     serdesType,
    GT_U32                 waitTime,
    GT_U32                 samplerSelect,
    GT_U32                 errorThreshold,
    MV_HWS_SAMPLER_RESULTS *samplerResults
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesScanSamplerFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesScanSamplerFunc(devNum, portGroup, serdesNum, waitTime, samplerSelect, errorThreshold, samplerResults);
    return rc;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

#ifndef VIPS_BUILD
/**
* @internal mvHwsSerdesDigitalRfResetToggleSet function
* @endinternal
*
* @brief   Run digital RF Reset/Unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] waitTime                 - wait time between Reset/Unreset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on error
*/
GT_STATUS mvHwsSerdesDigitalRfResetToggleSet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8                   waitTime
)
{
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8   i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CHECK_STATUS_EXT(GT_NOT_SUPPORTED, LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                             (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum)), RESET));
    }

    hwsOsTimerWkFuncPtr(waitTime);

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                             (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum)), UNRESET));
    }

    return GT_OK;
}
#endif

/**
* @internal mvHwsSerdesPartialPowerDown function
* @endinternal
*
* @brief   Enable/Disable power down of Tx and Rx on Serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of SERDESes to configure
* @param[in] powerDownRx              - Enable/Disable the power down on Serdes Rx
* @param[in] powerDownTx              - Enable/Disable the power down on Serdes Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPartialPowerDown
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    MV_HWS_SERDES_TYPE     serdesType,
    GT_BOOL     powerDownRx,
    GT_BOOL     powerDownTx
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesPartialPowerDownFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesPartialPowerDownFunc(devNum, portGroup, serdesNum, powerDownRx, powerDownTx);
    return rc;
}

/**
* @internal mvHwsSerdesPartialPowerStatusGet function
* @endinternal
*
* @brief   Get the status of power Tx and Rx on Serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of SERDESes to configure
* @param[in] powerRxStatus            - GT_TRUE - Serdes power Rx is down
*                                      GT_FALSE - Serdes power Rx is up
* @param[in] powerTxStatus            - GT_TRUE - Serdes power Tx is down
*                                      GT_FALSE - Serdes power Tx is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesPartialPowerStatusGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    MV_HWS_SERDES_TYPE     serdesType,
    GT_BOOL     *powerRxStatus,
    GT_BOOL     *powerTxStatus
)
{
    GT_STATUS rc;

    if (serdesType >= SERDES_LAST)
    {
        return GT_BAD_PARAM;
    }
    if (hwsSerdesFuncsPtr[devNum][serdesType].serdesPartialPowerStatusGetFunc == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsSerdesFuncsPtr[devNum][serdesType].serdesPartialPowerStatusGetFunc(devNum, portGroup, serdesNum, powerRxStatus, powerTxStatus);
    return rc;
}

/**
* @internal hwsSerdesGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsSerdesGetFuncPtr
(
    GT_U8                       devNum,
    MV_HWS_SERDES_FUNC_PTRS     **hwsFuncsPtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_NOT_SUPPORTED;
    }

    *hwsFuncsPtr = &hwsSerdesFuncsPtr[devNum][0];

    return GT_OK;
}

/*******************************************************************************
* mvHwsPortTxRxDefaultsDbEntryGet
*
* DESCRIPTION:
*       Get the default Tx and CTLE(Rx) parameters of the defaults DB entry
*       for the proper port mode.
*
* INPUTS:
*       devNum          - device number
*       portMode        - port standard metric - applicable for BC2/AC3
*       serdesSpeed     - SerDes speed/baud rate - applicable for Avago based devices
*       txParamsPtr     - pointer of tx params to store the db entry params in
*       ctleParamsPtr   - pointer of rx (ctle) params to store the db entry params in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
extern MV_HWS_SERDES_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRxTuneParamsArray[];
extern MV_HWS_MAN_TUNE_TX_CONFIG_DATA hwPortTxDefaults[];
GT_STATUS mvHwsPortTxRxDefaultsDbEntryGet
(
    GT_U8                               devNum,
    MV_HWS_PORT_STANDARD                portMode,
    MV_HWS_SERDES_SPEED                 serdesSpeed,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA      *txParamsPtr,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *ctleParamsPtr
)
{
    MV_HWS_SERDES_SPEED i = SPEED_NA;

    if ((NULL == txParamsPtr) && (NULL == ctleParamsPtr))
    {
        return GT_BAD_PTR;
    }

    if ((AVAGO == HWS_DEV_SERDES_TYPE(devNum)) || (AVAGO_16NM == HWS_DEV_SERDES_TYPE(devNum)))
    {
        for (; i < LAST_MV_HWS_SERDES_SPEED; i++)
        {
            if (hwsAvagoSerdesTxRxTuneParamsArray[i].serdesSpeed == serdesSpeed)
                break;
        }

        if(LAST_MV_HWS_SERDES_SPEED == i)
        {
            return GT_NOT_SUPPORTED;
        }
    }

    /* Tx paramteres */
    if (txParamsPtr != NULL)
    {
        /* Tx parameters */
        if ((AVAGO == HWS_DEV_SERDES_TYPE(devNum)) || (AVAGO_16NM == HWS_DEV_SERDES_TYPE(devNum)))
        {
            txParamsPtr->txAmp   =  hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txAmp;
            txParamsPtr->txEmph0 =  (signed int) ((signed char)((hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txEmph0)&0xFF)); /* sign-extend 8bit txEmph0 to GT_32 */
            txParamsPtr->txEmph1 =  (signed int) ((signed char)((hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txEmph1)&0xFF)); /* sign-extend 8bit txEmph1 to GT_32 */
        }
        else
        {
            txParamsPtr->txAmp      =  hwPortTxDefaults[portMode].txAmp;
            txParamsPtr->txAmpAdj   =  hwPortTxDefaults[portMode].txAmpAdj;
            txParamsPtr->txAmpShft  =  hwPortTxDefaults[portMode].txAmpShft;
            txParamsPtr->txEmph0    =  hwPortTxDefaults[portMode].txEmph0;
            txParamsPtr->txEmph1    =  hwPortTxDefaults[portMode].txEmph1;
        }
    }

    /* Rx paramteres - no static Rx parameters exist for non-Avago serdes */
    if ((AVAGO == HWS_DEV_SERDES_TYPE(devNum)) || (AVAGO_16NM == HWS_DEV_SERDES_TYPE(devNum)))
    {
        if (ctleParamsPtr != NULL)
        {
            /* Rx (CTLE) parameters */
            ctleParamsPtr->bandWidth     =    hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.bandWidth;
            ctleParamsPtr->dcGain        =    hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.dcGain;
            ctleParamsPtr->highFrequency =    hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.highFrequency;
            ctleParamsPtr->loopBandwidth =    hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.loopBandwidth;
            ctleParamsPtr->lowFrequency  =    hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.lowFrequency;
            ctleParamsPtr->squelch       =    hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.squelch;
        }
    }

    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */



