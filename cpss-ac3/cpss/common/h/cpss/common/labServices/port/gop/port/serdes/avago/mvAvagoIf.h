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
* mvComPhyH28nmIf.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
*
*******************************************************************************/

#ifndef __mvAvagoIf_H
#define __mvAvagoIf_H

/* General H Files */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

#ifdef __cplusplus
extern "C" {
#endif


/************************* Globals *******************************************************/

/*#define RAVEN_NUM_OF_DEVICES            6*/

#define BOBCAT3_MAX_AVAGO_SERDES_NUMBER 75
#define BOBK_MAX_AVAGO_SERDES_NUMBER    40
#define ALDRIN_MAX_AVAGO_SERDES_NUMBER  40
#define PIPE_MAX_AVAGO_SERDES_NUMBER    16
#define ALDRIN2_MAX_AVAGO_SERDES_NUMBER 73

/*
    Avago DFE table values: (table param - 0x5BXX, param data - 0xXX):
    data_bit[0]- high - disabling automatic one-shot pCal after iCal
    data_bit[3]- high - disable tap1 from the 14 DFE taps
    we using bit[3] for disabling DFE tap1 before running iCal
    and enabling it back after iCal finish successfully to allow
    tap1 for DFE. Tap can still be changed in pCal (DFE) tunning
*/
#define TAP1_AVG_DISABLE                0x5B01
#define TAP1_AVG_ENABLE                 0x5B09

/* timeout for blocking rx training operation */
#define MV_AVAGO_TRAINING_TIMEOUT       6000

#ifndef MAX_AVAGO_SERDES_NUMBER
#ifdef BV_DEV_SUPPORT
#define MAX_AVAGO_SERDES_NUMBER         16
#elif defined(ALDRIN_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         ALDRIN_MAX_AVAGO_SERDES_NUMBER
#elif defined(BOBK_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         BOBK_MAX_AVAGO_SERDES_NUMBER
#elif defined(BC3_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         BOBCAT3_MAX_AVAGO_SERDES_NUMBER
#elif defined(PIPE_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         PIPE_MAX_AVAGO_SERDES_NUMBER
#elif defined(ALDRIN2_DEV_SUPPORT)
#define MAX_AVAGO_SERDES_NUMBER         ALDRIN2_MAX_AVAGO_SERDES_NUMBER
#else /* in CPSS appDemo no xxx_DEV_SUPPORT flags lets take maximum for now,
host CPU memory can live with it */
#define MAX_AVAGO_SERDES_NUMBER         BOBCAT3_MAX_AVAGO_SERDES_NUMBER
#endif
#endif

#ifdef ASIC_SIMULATION
#define FALCON_MAX_AVAGO_SERDES_NUMBER  (256+2)
#undef  MAX_AVAGO_SERDES_NUMBER
#define MAX_AVAGO_SERDES_NUMBER         FALCON_MAX_AVAGO_SERDES_NUMBER
#endif


typedef enum
{
    IGNORE_RX_TRAINING,
    ONE_SHOT_DFE_TUNING,                /* Single time iCal */
    START_CONTINUE_ADAPTIVE_TUNING,     /* adaptive pCal */
    STOP_CONTINUE_ADAPTIVE_TUNING,
    ONE_SHOT_DFE_VSR_TUNING             /* iCal-VSR mode, Bypass DFE TAP tuning */

}MV_HWS_RX_TRAINING_MODES;

typedef enum
{
    IGNORE_TX_TRAINING,
    START_TRAINING,
    STOP_TRAINING

}MV_HWS_TX_TRAINING_MODES;

/**
* @struct MV_HWS_AVG_BLOCK
 *
 * @brief values to read from AVG block
 * to apply VOS corrected values
*/
typedef struct{

    /** @brief decides how the data is arranged.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon).
     */
    GT_U32 revision;

    /** @brief the number of AVG block lines.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon).
     */
    GT_U32 avgBlockLines;

    /** @brief minimum VOS value.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon).
     */
    GT_U32 minVal;

    /** @brief base VOS value.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon).
     */
    GT_U32 baseVal;

    /** @brief number of 7 VOS values per serdes
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon).
     */
    GT_U32 bitsPerSds;

    /** @brief first word in AVG line.
     *  databitsWord2 - second word in AVG line.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon).
     *  Comments:
     *  None
     */
    GT_U32 dataBitsWord1;

    GT_U32 dataBitsWord2;

} MV_HWS_AVG_BLOCK;

/**
* @internal mvHwsAvagoEthDriverInit function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoEthDriverInit(GT_U8 devNum);

/**
* @internal mvHwsAvagoIfInit function
* @endinternal
*
* @brief   Init Avago configuration sequences and IF functions.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoIfInit
(
    GT_U8                   devNum,
    MV_HWS_SERDES_FUNC_PTRS *funcPtrArray
);

/**
* @internal mvHwsAvago16nmIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - array for function registration
*/
GT_STATUS mvHwsAvago16nmIfInit
(
    GT_U8  devNum,
    MV_HWS_SERDES_FUNC_PTRS *funcPtrArray
);


/**
* @internal mvHwsAvagoIfClose function
* @endinternal
*
* @brief   Release Avago allocated memory and close aacs connection.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvHwsAvagoIfClose
(
    GT_U8 devNum
);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesTestGen function
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
GT_STATUS mvHwsAvagoSerdesTestGen
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    MV_HWS_SERDES_TEST_GEN_MODE mode
);

/**
* @internal mvHwsAvagoSerdesTestGenGet function
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
GT_STATUS mvHwsAvagoSerdesTestGenGet
(
    GT_U8                    devNum,
    GT_UOPT                  portGroup,
    GT_UOPT                  serdesNum,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/**
* @internal mvHwsAvagoSerdesTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - Enable/Disable reset the accumulation of error counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTestGenStatus
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
* @internal mvHwsAvagoSerdesPolarityConfig function
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
GT_STATUS mvHwsAvagoSerdesPolarityConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
);

/**
* @internal mvHwsAvagoSerdesPolarityConfigGet function
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
GT_STATUS mvHwsAvagoSerdesPolarityConfigGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *invertTx,
    GT_BOOL           *invertRx
);
#endif

/**
* @internal mvHwsAvagoSerdesReset function
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
GT_STATUS mvHwsAvagoSerdesReset
(
    GT_U8      devNum,
    GT_UOPT    portGroup,
    GT_UOPT    serdesNum,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset
);

/**
* @internal mvHwsAvagoSerdesCoreReset function
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
GT_STATUS mvHwsAvagoSerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
);

/**
* @internal mvHwsAvagoSerdesDigitalReset function
* @endinternal
*
* @brief   Run digital RESET/UNRESET (RF) on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDigitalReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    digitalReset
);

/**
* @internal mvHwsAvagoSerdesPowerCtrl function
* @endinternal
*
* @brief   Power up Serdes number
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - Serdes number
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfig - pointer to Serdes paramters:
*      refClock  - ref clock value
*      refClockSource - ref clock source (primary line or
*      secondary)
*      baudRate - Serdes speed
*      busWidth - Serdes bus modes: 10Bits/20Bits/40Bits
*      media - RXAUI or XAUI
*      encoding - Rx & Tx data encoding NRZ/PAM4
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesPowerCtrl
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

/**
* @internal mvHwsAvagoSerdesAcTerminationCfg function
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
GT_STATUS mvHwsAvagoSerdesAcTerminationCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    GT_BOOL                 acTermEn
);

/**
* @internal mvHwsAvagoSerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Power up / down on list of SERDESes on the same device (core).
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
GT_STATUS mvHwsAvagoSerdesArrayPowerCtrl
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     numOfSer,
    GT_UOPT                     *serdesArr,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesBulkPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesBulkPowerCtrl
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
* @internal mvHwsAvagoSerdesRxAutoTuneStart function
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
GT_STATUS mvHwsAvagoSerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining
);

/**
* @internal mvHwsAvagoSerdesAutoTuneCfg function
* @endinternal
*
* @brief   Per SERDES configure parameters for TX training & Rx Training starting
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneCfg
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStart function
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
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL txTraining
);

/**
* @internal mvHwsAvagoSerdesAutoTuneStart function
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
GT_STATUS mvHwsAvagoSerdesAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

/**
* @internal mvHwsAvagoSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txAmp                    - Tx Attenuator [0...31]
* @param[in] txAmpAdj                 - not used in Avago serdes
* @param[in] emph0                    - Post-Cursor: for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
* @param[in] emph1                    - Pre-Cursor:  for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
* @param[in] slewRate                 - Slew rate  [0...3], 0 is fastest
*                                      (the SlewRate parameter is not supported in Avago Serdes 28nm)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_UREG_DATA    txAmp,
    GT_BOOL         txAmpAdj,
    GT_REG_DATA     emph0,
    GT_REG_DATA     emph1,
    GT_UREG_DATA    slewRate,
    GT_BOOL         txEmphEn
);

/**
* @internal mvHwsAvagoSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] configParams             - Manual Tx params structures:
*                                      txAmp     - Tx Attenuator [0...31]
*                                      txAmpAdj  - not used in Avago serdes
*                                      emph0     - Post-Cursor: for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
*                                      emph1     - Pre-Cursor:  for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
*                                      slewRate  - Slew rate  [0...3], 0 is fastest
*                                      (the SlewRate parameter is not supported in Avago Serdes 28nm)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxConfigGet
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         serdesNum,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
);

/**
* @internal mvHwsAvagoSerdesSignalLiveDetectGet function
* @endinternal
*
* @brief   Per SERDES get live indication is signal detected.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalLiveDetectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalDet
);


/**
* @internal mvHwsAvagoSerdesSignalDetectGet function
* @endinternal
*
* @brief   Per SERDES get indication is signal detected.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalDetectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalDet
);

/**
* @internal mvHwsAvagoSerdesSignalOkChange function
* @endinternal
*
* @brief   Per SERDES get indication check CDR lock and Signal
*          Detect change.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkChange
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalOkChange
);

/**
* @internal mvHwsAvagoSerdesRxSignalCheck function
* @endinternal
*
* @brief   Per SERDES check there is Rx Signal and indicate if Serdes is ready for Tuning or not
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxSignal                 - Serdes is ready for Tuning or not
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxSignalCheck
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_AUTO_TUNE_STATUS     *rxSignal
);

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check the Tx training status
*         This function is necessary for 802.3ap functionality
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStatusShort
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsAvagoSerdesAutoTuneStatus function
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
GT_STATUS mvHwsAvagoSerdesAutoTuneStatus
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Per SERDES stop the TX training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStop
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
);

/*******************************************************************************
* mvHwsAvagoSerdesAutoTuneResult
*
* DESCRIPTION:
*       Per SERDES return the adapted tuning results
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       results - the adapted tuning results.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsAvagoSerdesAutoTuneResult
(
    GT_U8                    devNum,
    GT_UOPT                  portGroup,
    GT_UOPT                  serdesNum,
    MV_HWS_AUTO_TUNE_RESULTS *results
);

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesPpmConfig function
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
GT_STATUS mvHwsAvagoSerdesPpmConfig
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_PPM_VALUE  ppmVal
);

/**
* @internal mvHwsAvagoSerdesPpmConfigGet function
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
GT_STATUS mvHwsAvagoSerdesPpmConfigGet
(
    GT_U8              devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    MV_HWS_PPM_VALUE   *ppmVal
);

/**
* @internal mvHwsAvagoSerdesAdaptPpm function
* @endinternal
*
* @brief   Set the PPM compensation on Serdes after Rx/TRX training in 10G and above.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
*                                      portMode   - port standard metric
*                                      trainingMode -
*                                      GT_FALSE: before Start the training
*                                      GT_TRUE:  after Stop the training
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAdaptPpm
(
    GT_U8              devNum,
    GT_UOPT            portGroup,
    GT_UOPT            serdesNum,
    GT_BOOL            configPpm
);

/**
* @internal mvHwsAvagoSerdesDfeConfig function
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
GT_STATUS mvHwsAvagoSerdesDfeConfig
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_DFE_MODE   dfeMode,
    GT_REG_DATA       *dfeCfg
);

/**
* @internal mvHwsAvagoSerdesDfeStatus function
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
GT_STATUS mvHwsAvagoSerdesDfeStatus
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
* @internal mvHwsAvagoDfeCheck function
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
GT_STATUS mvHwsAvagoDfeCheck
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
);
#endif

#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoSerdesManualRxConfig function
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
GT_STATUS mvHwsAvagoSerdesManualRxConfig
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
* @internal mvHwsAvagoSerdesManualRxConfigGet function
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
GT_STATUS mvHwsAvagoSerdesManualRxConfigGet
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
* @internal mvHwsAvagoSerdesLoopback function
* @endinternal
*
* @brief   Perform an Internal/External SERDES loopback mode for Debug use
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical Serdes number
* @param[in] lbMode    - loopback mode
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode from 'AN_TX_RX' mode,
*          the Rx & Tx Serdes Polarity configuration should be
*          restored, for re-synch again the PCS and MAC units
*          and getting LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS mvHwsAvagoSerdesLoopback
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_SERDES_LB  lbMode
);

/**
* @internal mvHwsAvagoSerdesLoopbackGet function
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
GT_STATUS mvHwsAvagoSerdesLoopbackGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_SERDES_LB  *lbModePtr
);

/**
* @internal mvHwsAvagoSerdesTxEnable function
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
GT_STATUS mvHwsAvagoSerdesTxEnable
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     enable
);

/**
* @internal mvHwsAvagoSerdesTxEnableGet function
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
GT_STATUS mvHwsAvagoSerdesTxEnableGet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL *enablePtr
);
#endif

/**
* @internal mvHwsAvagoSerdesTxIfSelect function
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
GT_STATUS mvHwsAvagoSerdesTxIfSelect
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        serdesTxIfNum
);

/**
* @internal mvHwsAvagoSerdesTxIfSelectGet function
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
GT_STATUS mvHwsAvagoSerdesTxIfSelectGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    GT_U32              *serdesTxIfNum
);

/**
* @internal mvHwsAvagoSerdesCalibrationStatus function
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
GT_STATUS mvHwsAvagoSerdesCalibrationStatus
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    MV_HWS_CALIBRATION_RESULTS  *results
);

/**
* @internal mvHwsAvagoSerdesFixAlign90Start function
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
GT_STATUS mvHwsAvagoSerdesFixAlign90Start
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_ALIGN90_PARAMS   *params
);

/**
* @internal mvHwsAvagoSerdesFixAlign90Status function
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
GT_STATUS mvHwsAvagoSerdesFixAlign90Status
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *trainingStatus
);

/**
* @internal mvHwsAvagoSerdesFixAlign90Stop function
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
GT_STATUS mvHwsAvagoSerdesFixAlign90Stop
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 serdesNum,
    MV_HWS_ALIGN90_PARAMS *params,
    GT_BOOL fixAlignPass
);

/**
* @internal mvHwsAvagoSerdesSamplerOffsetSet function
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
GT_STATUS mvHwsAvagoSerdesSamplerOffsetSet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UREG_DATA     samplSel,
    GT_UREG_DATA     value
);

/**
* @internal mvHwsAvagoSerdesManualCtleConfig function
* @endinternal
*
* @brief   Set the Serdes Manual CTLE config for DFE
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] dcGain                   - DC-Gain value        (rang: 0-255)
* @param[in] lowFrequency             - CTLE Low-Frequency   (rang: 0-15)
* @param[in] highFrequency            - CTLE High-Frequency  (rang: 0-15)
* @param[in] bandWidth                - CTLE Band-width      (rang: 0-15)
* @param[in] loopBandwidth            - CTLE Loop Band-width (rang: 0-15)
* @param[in] squelch                  - Signal OK threshold  (rang: 0-310)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualCtleConfig
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     dcGain,
    GT_UOPT     lowFrequency,
    GT_UOPT     highFrequency,
    GT_UOPT     bandWidth,
    GT_UOPT     loopBandwidth,
    GT_UOPT     squelch
);

/**
* @internal mvHwsAvagoSerdesManualCtleConfigGet function
* @endinternal
*
* @brief   Get the Serdes CTLE (RX) configurations
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical lane number
*
* @param[out] configParams             - Ctle params structures:
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
GT_STATUS mvHwsAvagoSerdesManualCtleConfigGet
(
    GT_U8                               devNum,
    GT_UOPT                             portGroup,
    GT_UOPT                             serdesNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *configParams
);

/**
* @internal mvHwsAvagoSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX or TX data
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfBits                - Number of error bits to inject (max=65535)
* @param[in] serdesDirection          - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesErrorInject
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     numOfBits,
    MV_HWS_SERDES_DIRECTION    serdesDirection
);

/**
* @internal mvHwsAvagoSerdesSignalOkCfg function
* @endinternal
*
* @brief   Set the signal OK threshold on Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] signalThreshold          - Signal OK threshold (0-15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkCfg
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     signalThreshold
);

/**
* @internal mvHwsAvagoSerdesSignalOkThresholdGet function
* @endinternal
*
* @brief   Get the signal OK threshold on Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalThreshold          - Signal OK threshold (0-15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkThresholdGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     *signalThreshold
);

/**
* @internal mvHwsAvagoSerdesShiftSamplePoint function
* @endinternal
*
* @brief   Shift Serdes sampling point earlier in time
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] delay                    - set the  (0-0xF)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesShiftSamplePoint
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_U32      delay
);

/**
* @internal mvHwsAvagoSerdesEnhanceTune function
* @endinternal
*
* @brief   Set the ICAL with shifted sampling point to find best sampling point
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTune
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U8       min_LF,
    GT_U8       max_LF
);

#ifndef MV_HWS_FREE_RTOS
/**
* @internal mvHwsAvagoSerdesEnhanceTuneLite function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] serdesSpeed              - speed of serdes
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
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLite
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    MV_HWS_SERDES_SPEED     serdesSpeed,
    GT_U8                   min_dly,
    GT_U8                   max_dly
);
#endif /* MV_HWS_FREE_RTOS */

/**
* @internal mvHwsAvagoSerdesEnhanceTuneLitePhase1 function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point.
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] serdesSpeed              - speed of serdes
* @param[in] currentDelaySize         - delays array size
* @param[in] currentDelayPtr          - delays array
* @param[in] inOutI                   - iteration index between different phases
* @param[in] best_eye                 - best eyes array to update
* @param[in] best_dly                 - best delays array to update
* @param[in] subPhase                 - assist flag to know which code to execute in this sub-phase
* @param[in] min_dly                  - Minimum delay_cal value: (rang: 0-31)
* @param[in] max_dly                  - Maximum delay_cal value: (rang: 0-31)
*
* @param[out] inOutI                   - iteration index between different phases
* @param[out] best_eye                 - best eyes array to update
* @param[out] best_dly                 - best delays array to update
* @param[out] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLitePhase1
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    MV_HWS_SERDES_SPEED     serdesSpeed,
    GT_U8       currentDelaySize,
    GT_U8       *currentDelayPtr,
    GT_U8       *inOutI,
    GT_U16      *best_eye,
    GT_U8       *best_dly,
    GT_U8       *subPhase,
    GT_U8       min_dly,
    GT_U8       max_dly
);

/**
* @internal mvHwsAvagoSerdesEnhanceTuneLitePhase2 function
* @endinternal
*
* @brief   Set shift sample point to with the best delay_cal value.
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_dly                 - best delay to set on serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLitePhase2
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U8       *best_dly
);

/**
* @internal mvHwsAvagoSerdeCpllOutputRefClkGet function
* @endinternal
*
* @brief   Return the output frequency of CPLL reference clock source of SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] refClk                   - CPLL reference clock frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdeCpllOutputRefClkGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    *refClk
);

/**
* @internal mvHwsAvagoSerdeSpeedGet function
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
GT_STATUS mvHwsAvagoSerdeSpeedGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_SPEED *rate
);

/**
* @internal mvHwsAvagoSerdesEomUiGet function
* @endinternal
*
* @brief   Return SERDES baud rate in Ui.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEomUiGet
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    GT_U32            *baudRatePtr
);

/**
* @internal mvHwsAvagoSerdesPollingSet function
* @endinternal
*
* @brief   Set the Polling on Avago Serdes register address
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - number of SERDESes to configure
* @param[in] regAddr                  - Serdes register address (offset) to access
* @param[in] mask                     - register read mask
* @param[in] numOfLoops               - polling on spico ready
* @param[in] delay                    -  in ms after register get value
* @param[in] expectedValue            - value for stopping the polling on register
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPollingSet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_U32      regAddr,
    GT_U32      mask,
    GT_U32      numOfLoops,
    GT_U32      delay,
    GT_U32      expectedValue
);

/**
* @internal mvHwsAvagoSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Check the Serdes Rx or Tx training status
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                 - Rx-Training status
* @param[out] txStatus                 - Tx-Training status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStatusShort
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

/**
* @internal mvHwsAvagoSerdesVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) from Avago Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number  (currently not used)
*
* @param[out] voltage                  - Serdes  value (in mV)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVoltageGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     *voltage
);

/**
* @internal mvHwsAvagoSerdesAutoTuneResult function
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
GT_STATUS mvHwsAvagoSerdesAutoTuneResult
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_AUTO_TUNE_RESULTS    *results
);

/**
* @struct MV_HWS_AVAGO_SERDES_EYE_GET_RESULT
 *
 * @brief A struct containing the results for Serdes Eye Monitoring
 * For Caelum, Aldrin, AC3X, Bobcat3.
*/
typedef struct{

    char *matrixPtr;

    unsigned int                 x_points;

    unsigned int                 y_points;

    char *vbtcPtr;

    char *hbtcPtr;

    unsigned int                 height_mv;

    unsigned int                 width_mui;

} MV_HWS_AVAGO_SERDES_EYE_GET_RESULT;

/**
* @struct MV_HWS_AVAGO_SERDES_EYE_GET_INPUT
 *
 * @brief A struct containing the input parameters for Serdes Eye Monitoring
 * For Caelum, Aldrin, AC3X, Bobcat3.
*/
typedef struct{

    unsigned int            min_dwell_bits;

    unsigned int            max_dwell_bits;

} MV_HWS_AVAGO_SERDES_EYE_GET_INPUT;

/**
* @internal mvHwsAvagoSerdesMultiLaneCtleBiasConfig function
* @endinternal
*
* @brief   Per SERDES set the CTLE Bias value, according to data base.
*
* @param[in] devNum                   - system device number
*                                      serdesNum       - physical serdes number
* @param[in] numOfLanes               - number of lanes
*                                      ctleBitMapIndex - index of ctle bias bit map
*
* @param[out] ctleBiasPtr              - pointer to ctle bias value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesMultiLaneCtleBiasConfig
(
    GT_U8 devNum,
    unsigned int serdesNum,
    GT_U8 numOfLanes,
    unsigned int ctleBitMapIndex,
    GT_U32 *ctleBiasPtr
);

/*******************************************************************************
* mvHwsAvagoSerdesEyeGet
*
* DESCRIPTION:
*       Per SERDES return the adapted tuning results
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       serdesNum - physical serdes number
*       eye_inputPtr - pointer to input parameters structure
*
* OUTPUTS:
*       eye_resultsPtr - pointer to results structure
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && !defined(CPSS_BLOB)
unsigned int mvHwsAvagoSerdesEyeGet
(
    unsigned char                       devNum,
    unsigned int                        serdesNum,
    MV_HWS_AVAGO_SERDES_EYE_GET_INPUT  *eye_inputPtr,
    MV_HWS_AVAGO_SERDES_EYE_GET_RESULT *eye_resultsPtr
);

#endif

/**
* @internal mvHwsAvagoSerdesVosOverrideModeSet function
* @endinternal
*
* @brief   Set the override mode of the serdeses VOS parameters. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
* @param[in] vosOverride              - GT_TRUE means override, GT_FALSE means no override.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosOverrideModeSet
(
    GT_U8   devNum,
    GT_BOOL vosOverride
);

/**
* @internal mvHwsAvagoSerdesVosOverrideModeGet function
* @endinternal
*
* @brief   Get the override mode of the serdeses VOS parameters. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosOverrideModeGet
(
    GT_U8   devNum,
    GT_BOOL *vosOverridePtr
);

/**
* @internal mvHwsAvagoSerdesManualVosParamsGet function
* @endinternal
*
* @brief   Get the VOS Override parameters from the local DB.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsGet
(
    unsigned char  devNum,
    unsigned long  *vosParamsPtr
);

/**
* @internal mvHwsAvagoSerdesAutoTuneStartExt function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training modes
* @param[in] txTraining               - Tx Training modes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStartExt
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_RX_TRAINING_MODES    rxTraining,
    MV_HWS_TX_TRAINING_MODES    txTraining
);

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)

/**
* @internal mvHwsAvagoSerdesBc3VosConfig function
* @endinternal
*
* @brief   Reading the HDFuse and locating the AVG block where the compressed VOS
*         data parameters are.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesBc3VosConfig
(
    unsigned char devNum
);

/**
* @internal mvHwsAvagoSerdesRxSignalOkGet function
* @endinternal
*
* @brief   Get the rx_signal_ok indication from o_core_status serdes macro.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxSignalOkGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalOk
);

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of edge-detect-by-phase algorithm used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] timeOut                  - timeout value for the edge detection
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  portNum,
    GT_U32  timeOut
);

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase1 function
* @endinternal
*
* @brief   Set DFE dwell_time parameter. This is phase1 of the by-phase edge detect
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase1
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      numOfSerdeses,
    GT_U32      *serdesArr,
    GT_32       threshEO,
    GT_U32      dwellTime
);

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1 function
* @endinternal
*
* @brief   Launching pCal and take starting time for the pCal process. This is
*         phase2_1 of the by-phase edge detect algorithm.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1
(
    GT_U8       devNum,
    GT_U32      portNum,
    GT_UOPT     portGroup,
    GT_U32      numOfSerdeses,
    GT_U32      *serdesArr,
    GT_32       threshEO,
    GT_U32      dwellTime
);

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2 function
* @endinternal
*
* @brief   Calculate pCal time duration, check for timeout or for peer TxAmp change
*         (edge-detection). This is phase2_2 of the by-phase edge detect algorithm.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] dwellTime                - define dwell value
* @param[in] continueExecute          - whether or not algorithm execution should be continued
* @param[in] isInNonBlockWaitPtr      - whether or not algorithm is in non-block waiting time
*
* @param[out] continueExecute          - whether or not algorithm execution should be continued
* @param[out] isInNonBlockWaitPtr      - whether or not algorithm is in non-block waiting time
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2
(
    GT_U8       devNum,
    GT_U32      portNum,
    GT_UOPT     portGroup,
    GT_U32      numOfSerdeses,
    GT_U32      *serdesArr,
    GT_32       threshEO,
    GT_U32      dwellTime,
    GT_BOOL     *continueExecute,
    GT_BOOL     *isInNonBlockWaitPtr
);

/**
* @internal mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of enhance-tune-by-phase algorithm used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  phyPortNum
);

/**
* @internal mvHwsAvagoSerdesOneShotTuneByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of one shot -tune-by-phase algorithm for
* KR2/CR2 mode, used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTuneByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  phyPortNum
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase1 function
* @endinternal
*
* @brief   Move sample point and launch iCal. this is phase1 of the phased enhance-tune
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] currentDelayPtr          - delays attay to set initial values with
* @param[in] currentDelaySize         - delay array size
* @param[in] best_eye                 -  array to set initial values with
*
* @param[out] currentDelayPtr          - delays attay to set initial values with
* @param[out] currentDelaySize         - delay array size
* @param[out] best_eye                 -  array to set initial values with
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase1
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U32      **currentDelayPtr,
    GT_U32      *currentDelaySize,
    GT_U32      *best_eye
);

/**
* @internal mvHwsAvagoSerdesPrePostTuningByPhase function
* @endinternal
*
* @brief   reset/unreset CG_UNIT Rx GearBox Register, and clock
*          from SerDes to GearBox before running iCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] resetEnable              - reset True/False.
* @param[in] fecCorrect               - FEC type.
* @param[in] port                     - Port Mode.
* @param[out] coreStatusReady                 -  array that
*       indicates if the core status of all lanes is ready
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPrePostTuningByPhase
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      phyPortNum,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_BOOL     resetEnable,
    GT_BOOL     *coreStatusReady,
    MV_HWS_PORT_FEC_MODE  fecCorrect,
    MV_HWS_PORT_STANDARD  portMode
);
/**
* @internal mvHwsPortAvagoDfeCfgSet function
* @endinternal
*
* @brief   run training for Avago SerDes.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - phyPortNum.
* @param[in] portMode                 - Port Mode.
* @param[in] dfeMode                  - training mode.

* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoDfeCfgSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_DFE_MODE         dfeMode
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase2 function
* @endinternal
*
* @brief   Getting LF values from serdeses. this is phase2 of the phased enhance-tune
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_LF                  - array to be filled with LF values from serdes
*
* @param[out] best_LF                  - array to be filled with LF values from serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase2
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U32      *best_LF
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase3 function
* @endinternal
*
* @brief   Move sample point, then launch pCal than calculate eye. this is phase3
*         of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] currentDelaySize         - delays array size
* @param[in] currentDelayPtr          - delays array
* @param[in] inOutI                   - iteration index between different phases
* @param[in] best_LF                  - best LF array to update
* @param[in] best_eye                 - best eyes array to update
* @param[in] best_dly                 - best delays array to update
* @param[in] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @param[out] inOutI                   - iteration index between different phases
* @param[out] best_LF                  - best LF array to update
* @param[out] best_eye                 - best eyes array to update
* @param[out] best_dly                 - best delays array to update
* @param[out] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase3
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U32      currentDelaySize,
    GT_U32      *currentDelayPtr,
    GT_U32      *inOutI,
    GT_U32      *best_LF,
    GT_U32      *best_eye,
    GT_U32      *best_dly,
    GT_U8       *subPhase
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase4 function
* @endinternal
*
* @brief   Move sample point to the best delay value that was found in previous phase
*         then launch iCal. this is phase4 of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_dly                 - best delay to set the serdes with
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase4
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U32      *best_dly
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase5 function
* @endinternal
*
* @brief   Get LF values from serdes then launch iCal. this is phase5 of the phased
*         enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] LF1_Arr                  - LF array to fill with values
*
* @param[out] LF1_Arr                  - LF array to fill with values
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase5
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    int         *LF1_Arr
);

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase6 function
* @endinternal
*
* @brief   Read LF and Gain from serdes after iCal was launched in previous phase,
*         than calculate best LF and Gain, write to serdes and launch pCal.
*         this is phase6 of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] LF1_Arr                  - LF array to fill with values
* @param[in] min_LF                   - minimum LF for reference
* @param[in] max_LF                   - maimum LF for reference
* @param[in] LF1_Arr                  - previous LF values for reference
* @param[in] best_eye                 - besy eye for printing purpose
* @param[in] best_dly                 - besy delay for printing purpose
* @param[in] inOutKk                  - iterator index
* @param[in] continueExecute          - whether or not to continue algorithm execution
*
* @param[out] inOutKk                  - iterator index
* @param[out] continueExecute          - whether or not to continue algorithm execution
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase6
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U8       min_LF,
    GT_U8       max_LF,
    int         *LF1_Arr,
    GT_U32      *best_eye,
    GT_U32      *best_dly,
    GT_U8       *inOutKk,
    GT_BOOL     *continueExecute
);

#endif


#ifdef __cplusplus
}
#endif

#endif /* __mvAvagoIf_H */


