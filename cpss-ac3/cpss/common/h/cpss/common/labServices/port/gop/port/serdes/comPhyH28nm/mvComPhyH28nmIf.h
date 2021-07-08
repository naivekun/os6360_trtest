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
* @file mvComPhyH28nmIf.h
*
* @brief
*
* @version   15
********************************************************************************
*/

#ifndef __mvComPhyHRev28nmIf_H
#define __mvComPhyHRev28nmIf_H

/* General H Files */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @internal mvHwsComH28nmIfInit function
* @endinternal
*
* @brief   Init Com_H serdes configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmIfInit(MV_HWS_SERDES_FUNC_PTRS *funcPtrArray);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsComH28nmSerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit ("Other" means HW default - K28.5
*                                      [alternate running disparity])
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesTestGen
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    MV_HWS_SERDES_TEST_GEN_MODE mode
);

/**
* @internal mvHwsComH28nmSerdesTestGenGet function
* @endinternal
*
* @brief   Get configuration of the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] txPatternPtr             - pattern to transmit ("Other" means any mode not
*                                      included explicitly in MV_HWS_SERDES_TX_PATTERN type)
* @param[out] modePtr                  - test mode or normal
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - unexpected pattern
* @retval GT_FAIL                  - HW error
*/
GT_STATUS mvHwsComH28nmSerdesTestGenGet
(
    GT_U8                    devNum,
    GT_UOPT                  portGroup,
    GT_UOPT                  serdesNum,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/**
* @internal mvHwsComH28nmSerdesTestGenStatus function
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
GT_STATUS mvHwsComH28nmSerdesTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
);
#endif

#ifndef CO_CPU_RUN
/**
* @internal mvHwsComH28nmSerdesPolarityConfig function
* @endinternal
*
* @brief   Per serdes invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesPolarityConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
);

/**
* @internal mvHwsComH28nmSerdesPolarityConfigGet function
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
GT_STATUS mvHwsComH28nmSerdesPolarityConfigGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *invertTx,
    GT_BOOL           *invertRx
);
#endif

/**
* @internal mvHwsComH28nmSerdesReset function
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
GT_STATUS mvHwsComH28nmSerdesReset
(
    GT_U8      devNum,
    GT_UOPT    portGroup,
    GT_UOPT    serdesNum,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset
);

/**
* @internal mvHwsComH28nmSerdesCoreReset function
* @endinternal
*
* @brief   Run core reset / unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] coreReset                - core Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
);

/**
* @internal mvHwsComH28nmSerdesDigitalReset function
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
GT_STATUS mvHwsComH28nmSerdesDigitalReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    digitalReset
);

/**
* @internal mvHwsComH28nmSerdesPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      numOfSer  - number of SERDESes to configure
*                                      serdesArr - collection of SERDESes to configure
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
GT_STATUS mvHwsComH28nmSerdesPowerCtrl
(
    GT_U8      devNum,
    GT_UOPT    portGroup,
    GT_UOPT    serdesNum,
    GT_BOOL                 powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

/**
* @internal mvHwsComH28nmSerdesAcTerminationCfg function
* @endinternal
*
* @brief   Configures AC termination on current serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] acTermEn                 - enable or disable AC termination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAcTerminationCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    GT_BOOL                 acTermEn
);

/**
* @internal mvHwsComH28nmSerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Power up / down on list of SERDESes on the same device (core).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSer                 - number of SERDESes to configure
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] powerUp                  - true for power UP
* @param[in] baudRate                 -
* @param[in] refClock                 - ref clock value
* @param[in] refClockSource           - ref clock source (primary line or secondary)
* @param[in] media                    - RXAUI or XAUI
* @param[in] mode                     - 10BIT  (enable/disable)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesArrayPowerCtrl
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 numOfSer,
    GT_UOPT                 *serdesArr,
    GT_BOOL                 powerUp,
    MV_HWS_SERDES_SPEED     baudRate,
    MV_HWS_REF_CLOCK        refClock,
    MV_HWS_REF_CLOCK_SRC    refClockSource,
    MV_HWS_SERDES_MEDIA     media,
    MV_HWS_SERDES_BUS_WIDTH_ENT  mode
);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsComH28nmSerdesBulkPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesBulkPowerCtrl
(
    GT_U8                   numOfSer,
    MV_HWS_ELEMENTS_ARRAY   *serdesArr,
    GT_BOOL                 powerUp,
    MV_HWS_SERDES_SPEED     baudRate,
    MV_HWS_REF_CLOCK        refClock,
    MV_HWS_REF_CLOCK_SRC    refClockSource,
    MV_HWS_SERDES_MEDIA     media,
    MV_HWS_SERDES_BUS_WIDTH_ENT  mode
);
#endif

/**
* @internal mvHwsComH28nmSerdesRxAutoTuneStart function
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
GT_STATUS mvHwsComH28nmSerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining
);

/**
* @internal mvHwsComH28nmSerdesAutoTuneCfg function
* @endinternal
*
* @brief   Per SERDES configure parameters for TX training & Rx Training starting
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAutoTuneCfg
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

/**
* @internal mvHwsComH28nmSerdesTxAutoTuneStart function
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
GT_STATUS mvHwsComH28nmSerdesTxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL txTraining
);

/**
* @internal mvHwsComH28nmSerdesAutoTuneStart function
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
GT_STATUS mvHwsComH28nmSerdesAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

/**
* @internal mvHwsComH28nmSerdesManualTxConfig function
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
GT_STATUS mvHwsComH28nmSerdesManualTxConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_UREG_DATA  txAmp,
    GT_BOOL txAmpAdj,
    GT_REG_DATA   emph0,
    GT_REG_DATA   emph1,
    GT_UREG_DATA  txAmpShft,
    GT_BOOL         txEmphEn
);

/**
* @internal mvHwsComH28nmSerdesAutoTuneStatus function
* @endinternal
*
* @brief   Per SERDES check the Rx & Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAutoTuneStatus
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsComH28nmSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check Rx or Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      isTx      - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAutoTuneStatusShort
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsComH28nmSerdesAutoTuneResult function
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
GT_STATUS mvHwsComH28nmSerdesAutoTuneResult
(
    GT_U8                    devNum,
    GT_UOPT                  portGroup,
    GT_UOPT                  serdesNum,
    MV_HWS_AUTO_TUNE_RESULTS *results
);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsComH28nmSerdesPpmConfig function
* @endinternal
*
* @brief   Per SERDES add ability to add/sub PPM from main baud rate.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesPpmConfig
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_PPM_VALUE  ppmVal
);

/**
* @internal mvHwsComH28nmSerdesPpmConfigGet function
* @endinternal
*
* @brief   Per SERDES add ability to add/sub PPM from main baud rate.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesPpmConfigGet
(
    GT_U8              devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    MV_HWS_PPM_VALUE   *ppmVal
);

/**
* @internal mvHwsComH28nmSerdesAdaptPpm function
* @endinternal
*
* @brief   Set the PPM compensation on Serdes after Rx/TRX training in 10G and above.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - Serdes number
* @param[in] configPpm                - enable/disable the PPM configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesAdaptPpm
(
    GT_U8              devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    GT_BOOL            configPpm
);

/**
* @internal mvHwsComH28nmSerdesDfeConfig function
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
GT_STATUS mvHwsComH28nmSerdesDfeConfig
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_DFE_MODE   dfeMode,
    GT_REG_DATA       *dfeCfg
);

/**
* @internal mvHwsComH28nmSerdesDfeStatus function
* @endinternal
*
* @brief   Per SERDES get the DFE status and parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] dfeLock                  - Locked/Not locked
* @param[out] dfeCfg                   - array of dfe configuration parameters
* @param[out] f0Cfg                    - array of f0 configuration parameters
* @param[out] savCfg                   - array of sav configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesDfeStatus
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *dfeLock,
    GT_REG_DATA       *dfeCfg,
    GT_REG_DATA       *f0Cfg,
    GT_REG_DATA       *savCfg
);

/**
* @internal mvHwsComH28nmDfeCheck function
* @endinternal
*
* @brief   Check DFE values range and start sampler calibration, if needed.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmDfeCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
);
#endif

#ifndef CO_CPU_RUN
/**
* @internal mvHwsComH28nmSerdesManualRxConfig function
* @endinternal
*
* @brief   Per SERDES configure the RX parameters: squelch Threshold, FFE and DFE
*         operation
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical lane number
* @param[in] portTuningMode           - lane tuning mode (short / long)
* @param[in] sqlch                    - Squelch threshold
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesManualRxConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    GT_UREG_DATA  sqlch,
    GT_UREG_DATA  ffeRes,
    GT_UREG_DATA  ffeCap,
    GT_BOOL dfeEn,
    GT_UREG_DATA  alig
);

/**
* @internal mvHwsComH28nmSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Per SERDES returns the RX parameters configurations
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical lane number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesManualRxConfigGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_PORT_MAN_TUNE_MODE *portTuningModePtr,
    GT_U32             *sqlchPtr,
    GT_U32             *ffeResPtr,
    GT_U32             *ffeCapPtr,
    GT_BOOL            *dfeEnPtr,
    GT_U32             *aligPtr
);

/**
* @internal mvHwsComH28nmSerdesLoopback function
* @endinternal
*
* @brief   Perform an internal loopback (SERDES loopback) for debug/BIST use.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      serdesType - type of serdes (LP/COM_PHY...)
* @param[in] lbMode                   - loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesLoopback
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_SERDES_LB  lbMode
);

/**
* @internal mvHwsComH28nmSerdesLoopbackGet function
* @endinternal
*
* @brief   Get status of internal loopback (SERDES loopback).
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      serdesType - type of serdes (LP/COM_PHY...)
*
* @param[out] lbModePtr                - current loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesLoopbackGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_SERDES_LB  *lbModePtr
);

/**
* @internal mvHwsComH28nmSerdesTxEnable function
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
GT_STATUS mvHwsComH28nmSerdesTxEnable
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     enable
);

/**
* @internal mvHwsComH28nmSerdesTxEnableGet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] enablePtr                - GT_TRUE  - Enable transmission of packets in
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
GT_STATUS mvHwsComH28nmSerdesTxEnableGet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL *enablePtr
);
#endif

/**
* @internal mvHwsComH28nmSerdesTxIfSelect function
* @endinternal
*
* @brief   tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      enable    - whether to enable or disable Tx.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesTxIfSelect
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        serdesTxIfNum
);

/**
* @internal mvHwsComH28nmSerdesTxIfSelectGet function
* @endinternal
*
* @brief   Return the SERDES Tx interface select number.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      serdesType  - SERDES types
*
* @param[out] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesTxIfSelectGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    GT_U32              *serdesTxIfNum
);

/**
* @internal mvHwsComH28nmSerdesCalibrationStatus function
* @endinternal
*
* @brief   Per SERDES return the calibration results.
*         Can be run after create port
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] results                  - the calibration results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesCalibrationStatus
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    MV_HWS_CALIBRATION_RESULTS  *results
);

/**
* @internal mvHwsComH28nmSerdesFixAlign90Start function
* @endinternal
*
* @brief   Start fix Align90 process on current SERDES.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] params                   - pointer to structure to store SERDES configuration parameters that must be restore
*                                      during process stop operation
*
* @param[out] params                   - SERDES configuration parameters that must be restore
*                                      during process stop operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesFixAlign90Start
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_ALIGN90_PARAMS   *params
);

/**
* @internal mvHwsComH28nmSerdesFixAlign90Status function
* @endinternal
*
* @brief   Return fix Align90 process current status on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesFixAlign90Status
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *trainingStatus
);

/**
* @internal mvHwsComH28nmSerdesForceAlign90Set function
* @endinternal
*
* @brief   Perform force Align90 value and pulse after updating the Align90
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] forceAlign90             - value of Align90
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsComH28nmSerdesForceAlign90Set
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    GT_U32  forceAlign90
);

/**
* @internal mvHwsComH28nmSerdesFixAlign90Stop function
* @endinternal
*
* @brief   Stop fix Align90 process on current SERDES.
*         Can be run after create port and start Align90.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] params                   - SERDES parameters that must be restored (return by mvHwsComHRev2SerdesFixAlign90Start)
* @param[in] fixAlignPass             - true, if fix Align90 process passed; false otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesFixAlign90Stop
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 serdesNum,
    MV_HWS_ALIGN90_PARAMS *params,
    GT_BOOL fixAlignPass
);

/**
* @internal mvHwsComH28nmSerdesSamplerOffsetSet function
* @endinternal
*
* @brief   Serdes Sampler offset set.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] samplSel                 - sampler select
* @param[in] value                    - offset value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesSamplerOffsetSet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UREG_DATA     samplSel,
    GT_UREG_DATA     value
);

#ifdef __cplusplus
}
#endif

#endif /* __mvComPhyHRev28nmIf_H */


