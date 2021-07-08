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
* mvComPhyHRev2If.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 24 $
*
*******************************************************************************/

#ifndef __mvComPhyHRev2If_H
#define __mvComPhyHRev2If_H

/* General H Files */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>


/**
* @enum MV_HWS_SERDES_TRAINING_OPT_ALGO
 *
 * @brief Defines different Serdes training algorithms.
 * Comment: Used as a bit mask for determine which algorithms to run
*/
typedef enum{

    DFE_OPT_ALGO     = 0x1,

    FFE_OPT_ALGO     = 0x2,

    Align90_OPT_ALGO = 0x4

} MV_HWS_SERDES_TRAINING_OPT_ALGO;

/**
* @internal mvHwsComHRev2IfInit function
* @endinternal
*
* @brief   Init Com_H serdes configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2IfInit(MV_HWS_SERDES_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsComHRev2SerdesTrainingOptDelayInit function
* @endinternal
*
* @brief   Set the dynamic and static Delays for DFE and align90
*
* @param[in] dynamicDelayInterval     - Interval delay for DFE
* @param[in] dynamicDelayDuration     - Duration delay for DFE
* @param[in] staticDelayDuration      - Duration delay for align90
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesTrainingOptDelayInit
(
    GT_U32  dynamicDelayInterval,
    GT_U32  dynamicDelayDuration,
    GT_U32  staticDelayDuration
);

/**
* @internal mvHwsComHRev2SerdesTestGen function
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
GT_STATUS mvHwsComHRev2SerdesTestGen
(
 GT_U8                     devNum,
 GT_U32                    portGroup,
 GT_U32                    serdesNum,
 MV_HWS_SERDES_TX_PATTERN  txPattern,
 MV_HWS_SERDES_TEST_GEN_MODE mode
);

/*******************************************************************************
* mvHwsComHRev2SerdesTestGenGet
*
* DESCRIPTION:
*       Get configuration of the Serdes test generator/checker.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       txPatternPtr - pattern to transmit ("Other" means any mode not
*                       included explicitly in MV_HWS_SERDES_TX_PATTERN type)
*       modePtr      - test mode or normal
*
* RETURNS:
*       GT_OK  - on success
*       GT_NOT_SUPPORTED - unexpected pattern
*       GT_FAIL - HW error
*
*******************************************************************************/
GT_STATUS mvHwsComHRev2SerdesTestGenGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/**
* @internal mvHwsComHRev2SerdesPowerCtrl function
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
GT_STATUS mvHwsComHRev2SerdesPowerCtrl
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

/**
* @internal mvHwsComHRev2SerdesDfeStatusExt function
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
GT_STATUS mvHwsComHRev2SerdesDfeStatusExt
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    GT_BOOL           *dfeLock,
    GT_32             *dfeCfg,
    GT_32             *f0Cfg,
    GT_32             *savCfg
);

/**
* @internal mvHwsComHRev2SerdesTestGenGet function
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
GT_STATUS mvHwsComHRev2SerdesTestGenGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/**
* @internal mvHwsComHRev2SerdesAutoTuneCfg function
* @endinternal
*
* @brief   Per SERDES configure parameters for TX training & Rx Training starting
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesAutoTuneCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

/**
* @internal mvHwsComHRev2SerdesManualRxConfig function
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
GT_STATUS mvHwsComHRev2SerdesManualRxConfig
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    GT_U32  sqlch,
    GT_U32  ffeRes,
    GT_U32  ffeCap,
    GT_BOOL dfeEn,
    GT_U32  alig
);

/**
* @internal mvHwsComHRev2SerdesManualTxConfig function
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
GT_STATUS mvHwsComHRev2SerdesManualTxConfig
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    GT_U32  txAmp,
    GT_BOOL txAmpAdj,
    GT_32   emph0,
    GT_32   emph1,
    GT_U32  txAmpShft,
    GT_BOOL txEmphEn
);

/**
* @internal mvHwsComHRev2SerdesPpmConfig function
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
GT_STATUS mvHwsComHRev2SerdesPpmConfig
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_PPM_VALUE  ppmVal
);

/**
* @internal mvHwsComHRev2SerdesPpmConfigGet function
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
GT_STATUS mvHwsComHRev2SerdesPpmConfigGet
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_PPM_VALUE  *ppmVal
);

/**
* @internal mvHwsComHRev2SerdesFfeTableCfg function
* @endinternal
*
* @brief   Per SERDES config the FFE table (used by auto tune)
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      rxTraining - Rx Training (true/false)
*                                      txTraining - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesFfeTableCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_PORT_MAN_TUNE_MODE tuneMode
);

/**
* @internal mvHwsComHRev2SerdesRxAutoTuneStart function
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
GT_STATUS mvHwsComHRev2SerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    GT_BOOL rxTraining
);

/**
* @internal mvHwsComHRev2SerdesTxAutoTuneStart function
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
GT_STATUS mvHwsComHRev2SerdesTxAutoTuneStart
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    GT_BOOL txTraining
);

/**
* @internal mvHwsComHRev2SerdesAutoTuneStart function
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
GT_STATUS mvHwsComHRev2SerdesAutoTuneStart
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

/**
* @internal mvHwsComHRev2SerdesAutoTuneStatus function
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
GT_STATUS mvHwsComHRev2SerdesAutoTuneStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsComHRev2SerdesAutoTuneResult function
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
GT_STATUS mvHwsComHRev2SerdesAutoTuneResult
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_RESULTS *results
);

/**
* @internal mvHwsComHRev2SerdesCalibrationStatus function
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
GT_STATUS mvHwsComHRev2SerdesCalibrationStatus
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    MV_HWS_CALIBRATION_RESULTS  *results
);

/**
* @internal mvHwsComHRev2SerdesTxEnable function
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
GT_STATUS mvHwsComHRev2SerdesTxEnable
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     enable
);

/**
* @internal mvHwsComHRev2SerdesTxEnableGet function
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
GT_STATUS mvHwsComHRev2SerdesTxEnableGet
(
 GT_U8   devNum,
 GT_U32  portGroup,
 GT_U32  serdesNum,
 GT_BOOL *enablePtr
 );

/**
* @internal mvHwsComHRev2SerdesTestGenStatus function
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
GT_STATUS mvHwsComHRev2SerdesTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
);

/**
* @internal mvHwsComHRev2SerdesTxIfSelect function
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
GT_STATUS mvHwsComHRev2SerdesTxIfSelect
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    GT_U32              serdesTxIfNum
);

/**
* @internal mvHwsComHRev2SqlchCfg function
* @endinternal
*
* @brief   Configure squelch threshold value.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] sqlchVal                 - squelch threshold value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SqlchCfg
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    GT_U32              sqlchVal
);

/**
* @internal mvHwsComHRev2DfeCheck function
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
GT_STATUS mvHwsComHRev2DfeCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
);

/**
* @internal mvHwsComHRev2SerdesSpeedGet function
* @endinternal
*
* @brief   Return SERDES baud rate.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesSpeedGet
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 serdesNum,
    MV_HWS_SERDES_SPEED *rate
);

/**
* @internal mvHwsComHRev2SerdesManualRxConfigGet function
* @endinternal
*
* @brief   Return SERDES RX parameters configured manually .
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] sqlchPtr                 - Squelch threshold.
* @param[out] ffeResPtr                - FFE R
* @param[out] ffeCapPtr                - FFE C
* @param[out] aligPtr                  - align 90 value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesManualRxConfigGet
(
    IN GT_U8                      devNum,
    IN GT_U32                     portGroup,
    IN GT_U32                     serdesNum,
    OUT MV_HWS_PORT_MAN_TUNE_MODE *portTuningModePtr,
    OUT GT_U32                    *sqlchPtr,
    OUT GT_U32                    *ffeResPtr,
    OUT GT_U32                    *ffeCapPtr,
    OUT GT_BOOL                   *dfeEnPtr,
    OUT GT_U32                    *aligPtr
);

/**
* @internal mvHwsComHRev2SerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
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
GT_STATUS mvHwsComHRev2SerdesArrayPowerCtrl
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      numOfSer,
    GT_U32                      *serdesArr,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

/**
* @internal mvHwsComHRev2SerdesAcTerminationCfg function
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
GT_STATUS mvHwsComHRev2SerdesAcTerminationCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    GT_BOOL                 acTermEn
);

/**
* @internal mvHwsComHRev2DigitalSerdesReset function
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
GT_STATUS mvHwsComHRev2DigitalSerdesReset
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    MV_HWS_RESET    digitalReset
);


/**
* @internal mvHwsComHRev2SerdesFixAlign90Start function
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
GT_STATUS mvHwsComHRev2SerdesFixAlign90Start
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_ALIGN90_PARAMS   *params
);

/**
* @internal mvHwsComHRev2SerdesFixAlign90Status function
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
GT_STATUS mvHwsComHRev2SerdesFixAlign90Status
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *trainingStatus
);

/**
* @internal mvHwsComHRev2SerdesFixAlign90Stop function
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
GT_STATUS mvHwsComHRev2SerdesFixAlign90Stop
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 serdesNum,
    MV_HWS_ALIGN90_PARAMS *params,
    GT_BOOL fixAlignPass
);

/**
* @internal mvHwsComHRev2SerdesTrainingOptimization function
* @endinternal
*
* @brief   Per SERDES Run the Serdes training optimization algorithms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesList               - array of physical Serdes number
* @param[in] numOfActLanes            - number of Serdes.
* @param[in] tuningMode               - Short/Long reach
* @param[in] algoMask                 - algorithm but mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesTrainingOptimization
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      *serdesList,
    GT_U32                      numOfActLanes,
    MV_HWS_PORT_MAN_TUNE_MODE   tuningMode,
    GT_U32                      algoMask
);

/**
* @internal mvHwsComHRev2SerdesOptSetFfeThreshold function
* @endinternal
*
* @brief   Set FFR threshold.
*
* @param[in] ffeFirstTh               - FFE first threshold
* @param[in] ffeFinalTh               - FFE final threshold
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesOptSetFfeThreshold
(
    GT_32     ffeFirstTh,
    GT_32     ffeFinalTh
);

/**
* @internal mvHwsComHRev2SerdesOptSetF0dStopThreshold function
* @endinternal
*
* @brief   Set F0D stop threshold.
*
* @param[in] f0dStopTh                - F0D stop threshold
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesOptSetF0dStopThreshold
(
    GT_U32     f0dStopTh
);

/**
* @internal mvHwsComHRev2SerdesConfigRegulator function
* @endinternal
*
* @brief   Per SERDES Run the Serdes Regulator Configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesList               - array of physical Serdes number
* @param[in] numOfActLanes            - number of Serdes.
* @param[in] setDefault               - set to default value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesConfigRegulator
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      *serdesList,
    GT_U32                      numOfActLanes,
    GT_BOOL                     setDefault
);

/**
* @internal mvHwsComHRev2SerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on PCS (true/false).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      pcsNum    - physical PCS number
* @param[in] signalDet                - if true, signal was detected
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesSignalDetectGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    GT_BOOL                 *signalDet
);

/**
* @internal mvHwsComHRev2SerdesCdrLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - notlocked).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] cdrLock                  - if true, CDR was locked
*
* @param[out] cdrLock                  - if true, CDR was locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    GT_BOOL                 *cdrLock
);

/**
* @internal mvHwsComHRev2SerdesAutoTuneStatusShort function
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
GT_STATUS mvHwsComHRev2SerdesAutoTuneStatusShort
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsComHRev2SerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      rxTraining - Rx Training (true/false)
*                                      txTraining - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesTxAutoTuneStop
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
);

/**
* @internal mvHwsComHRev2SerdesOptAlgoParams function
* @endinternal
*
* @brief   Change default thresholds of:
*         ffe optimizer for first iteration, final iteration and f0d stop threshold
* @param[in] ffeFirstTh               - threshold value for changing ffe optimizer for first iteration
* @param[in] ffeFinalTh               - threshold value for changing ffe optimizer for final iteration
* @param[in] f0dStopTh                - value for changing default of f0d stop threshold
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesOptAlgoParams
(
    GT_32     ffeFirstTh,
    GT_32     ffeFinalTh,
    GT_U32    f0dStopTh
);

/**
* @internal mvHwsComHRev2SerdesSetTuningParam function
* @endinternal
*
* @brief   Setting tuning parameters according to the receive value:
*         Enables/Disabled eye check, Setting PRESET command (INIT=1 or PRESET=2),
*         Gen1 Amplitude values, initialize Amplitude values preset Amplitude values.
*         Parameter will not set if received value not in the range:
*         presetCmdVal  - enum INIT/PRESET
*         eyeCheckEnVal  - GT_TRUE/GT_FALSE
*         gen1TrainAmp  - 0-0x1F
*         gen1TrainEmph0 - 0-0xF
*         gen1TrainEmph1 - 0-0xF
*         gen1AmpAdj   - GT_TRUE/GT_FALSE
*         initialAmp   - 0-0x3F
*         initialEmph0  - 0-0xF
*         initialEmph1  - 0-0xF
*         presetAmp    - 0-0x3F
*         presetEmph0   - 0-0xF
*         presetEmph1   - 0-0xF
* @param[in] params                   - pointer to structure to store tuning parameters for changing default values
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComHRev2SerdesSetTuningParam
(
    MV_HWS_SERDES_TUNING_PARAMS  *params
);

#endif /* __mvComPhyHRev2If_H */


