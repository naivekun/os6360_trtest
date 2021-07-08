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
* @file mvHwsPortMiscIf.h
*
* @brief
*
* @version   24
********************************************************************************
*/

#ifndef __mvHwsPortMiscIf_H
#define __mvHwsPortMiscIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>

/**
* @enum MV_HWS_PORT_AUTO_TUNE_MODE
 *
 * @brief Defines different port auto tuning modes.
*/
typedef enum
{
    RxTrainingOnly = 1,
    TRxTuneCfg,
    TRxTuneStart,
    TRxTuneStatus,

    /* for AP */
    TRxTuneStatusNonBlocking,
    TRxTuneStop,

    /* for Avago */
    RxTrainingAdative,       /* run DFE_START_ADAPTIVE mode */
    RxTrainingStopAdative,   /* run DFE_STOP_ADAPTIVE  mode */
    RxTrainingWaitForFinish, /* run ICAL and wait for completeness */
    RxTrainingVsr            /* run ICAL-VSR mode, Bypass DFE TAP tuning  */

}MV_HWS_PORT_AUTO_TUNE_MODE;

typedef enum
{
  TUNE_PASS,
  TUNE_FAIL,
  TUNE_NOT_COMPLITED,
  TUNE_READY,
  TUNE_NOT_READY    /* relevant for Avago only. This state indicates */
                    /* that there is no Rx SerDEs Signal to start training */
}MV_HWS_AUTO_TUNE_STATUS;


typedef enum
{
  Neg_3_TAPS,
  Neg_2_TAPS,
  Neg_1_TAPS,
  ZERO_TAPS,
  POS_1_TAPS,
  POS_2_TAPS,
  POS_3_TAPS

}MV_HWS_PPM_VALUE;

/**
* @enum MV_HWS_PORT_MAN_TUNE_MODE
 *
 * @brief Defines different port manual tuning modes.
*/
typedef enum{

    StaticLongReach,

    StaticShortReach

} MV_HWS_PORT_MAN_TUNE_MODE;

typedef struct
{
    MV_HWS_AUTO_TUNE_STATUS rxTune;
    MV_HWS_AUTO_TUNE_STATUS txTune;

}MV_HWS_AUTO_TUNE_STATUS_RES;

typedef struct
{
  GT_U32 ffeR;
  GT_U32 ffeC;
  GT_U32 sampler;
  GT_U32 sqleuch;
  GT_U32 txAmp;      /* In ComPhyH Serdes Amplitude, In Avago Serdes Attenuator [0...31] */
  GT_32  txEmph0;    /* for Lion2 device [0...15]
                        for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
                        for Bobcat3 device: Post-Cursor [0...31] */
  GT_32  txEmph1;    /* for Lion2 device [0...15]
                        for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
                        for Bobcat3 device: Pre-Cursor [0...31] */
  GT_U32 align90;
  GT_32  dfeVals[6];

  /* Lion2B additions */
  GT_U32 txAmpAdj;
  GT_U32 txAmpShft;
  GT_U32 txEmph0En;
  GT_U32 txEmph1En;

  /* Avago */
  GT_U32 DC;
  GT_U32 LF;
  GT_U32 HF;
  GT_U32 BW;
  GT_U32 LB;
  GT_U32 EO;
  GT_32  DFE[13];

}MV_HWS_AUTO_TUNE_RESULTS;

typedef struct
{
    GT_U32      sqlch;
    GT_U32      ffeRes;
    GT_U32      ffeCap;
    GT_BOOL     dfeEn;
    GT_U32      alig;

}MV_HWS_MAN_TUNE_RX_CONFIG_DATA;

typedef struct
{
    GT_U32      txAmp;      /* In ComPhyH Serdes Amplitude, In Avago Serdes Attenuator [0...31] */
    GT_BOOL     txAmpAdj;
    GT_32       txEmph0;    /* for Lion2 device [0...15]
                               for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
                               for Bobcat3 device: Post-Cursor [0...31] */
    GT_32       txEmph1;    /* for Lion2 device [0...15]
                               for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
                               for Bobcat3 device: Pre-Cursor [0...31] */
    GT_BOOL     txAmpShft;

}MV_HWS_MAN_TUNE_TX_CONFIG_DATA;

typedef struct
{
    GT_U32     dcGain;
    GT_U32     lowFrequency;
    GT_U32     highFrequency;
    GT_U32     bandWidth;
    GT_U32     loopBandwidth;
    GT_U32     squelch;

}MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA;

typedef struct
{
    GT_U8     etlMinDelay;
    GT_U8     etlMaxDelay;
    GT_U8     etlEnableOverride;
}MV_HWS_ETL_CONFIG_DATA;

#define MV_HWS_MAN_TUNE_CTLE_CONFIG_SQLCH_CNS           0x1
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_LF_CNS              0x2
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_HF_CNS              0x4
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_DCGAIN_CNS          0x8
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_BANDWIDTH_CNS       0x10
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_LOOPBANDWIDTH_CNS   0x20
#define MV_HWS_MAN_TUNE_ETL_MIN_DELAY_CNS               0x40
#define MV_HWS_MAN_TUNE_ETL_MAX_DELAY_CNS               0x80
#define MV_HWS_MAN_TUNE_ETL_ENABLE_CNS                  0x100

#define MV_HWS_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS       0
#define MV_HWS_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS       1

typedef struct
{
    GT_U32                              serdesSpeed;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    ctleParams;
    MV_HWS_ETL_CONFIG_DATA              etlParams;
    GT_U16                              fieldOverrideBmp;
}MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE;

/* Avago Serdes Tx params */
typedef struct
{
    GT_U32    txAmp;
    GT_U32    txEmph0;
    GT_U32    txEmph1;

}MV_HWS_TX_TUNE_PARAMS;

/* Avago Serdes TxRx params */
typedef struct
{
    MV_HWS_TX_TUNE_PARAMS               txParams;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    ctleParams;

}MV_HWS_TXRX_TUNE_PARAMS;

typedef struct
{
    MV_HWS_SERDES_SPEED                 serdesSpeed;
    MV_HWS_TX_TUNE_PARAMS               txParams;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    ctleParams;

}MV_HWS_SERDES_TXRX_TUNE_PARAMS;


typedef struct
{
    GT_BOOL calDone;
    GT_U32  txImpCal;
    GT_U32  rxImpCal;
    GT_U32  ProcessCal;
    GT_U32  speedPll;
    GT_U32  sellvTxClk;
    GT_U32  sellvTxData;
    GT_U32  sellvTxIntp;
    GT_U32  sellvTxDrv;
    GT_U32  sellvTxDig;
    GT_U32  sellvRxSample;
    GT_BOOL ffeDone;
    GT_32   ffeCal[8];

}MV_HWS_CALIBRATION_RESULTS;

/**
* @enum MV_HWS_PORT_TEST_GEN_PATTERN
 *
 * @brief Defines different port (PCS) test generator patterns.
*/
typedef enum
{
  TEST_GEN_PRBS7,
  TEST_GEN_PRBS23,
  TEST_GEN_CJPAT,
  TEST_GEN_CRPAT,
  TEST_GEN_KRPAT,
  TEST_GEN_Normal,
  TEST_GEN_PRBS31

}MV_HWS_PORT_TEST_GEN_PATTERN;

/**
* @enum MV_HWS_PORT_TEST_GEN_ACTION
 *
 * @brief Defines different port test generator actions.
*/
typedef enum{

    NORMAL_MODE,

    TEST_MODE

} MV_HWS_PORT_TEST_GEN_ACTION;


/**
* @enum MV_HWS_PORT_LB_TYPE
 *
 * @brief Defines different port loop back types.
 * DISABLE_LB - disable loop back
 * RX_2_TX_LB - configure port to send back all received packets
 * TX_2_RX_LB - configure port to receive back all sent packets
*/
typedef enum{

    DISABLE_LB,

    RX_2_TX_LB,

    TX_2_RX_LB,

    TX_2_RX_DIGITAL_LB

} MV_HWS_PORT_LB_TYPE;

/**
* @enum MV_HWS_UNIT
 *
 * @brief Defines different port loop back levels.
*/
typedef enum{

    HWS_MAC,

    HWS_PCS,

    HWS_PMA

} MV_HWS_UNIT;

/*
 * Typedef: struct MV_HWS_TEST_GEN_STATUS
 *
 * Description: Defines port test generator results.
 *
 */
typedef struct
{
  GT_U32  totalErrors;
  GT_U32  goodFrames;
  GT_U32  checkerLock;

}MV_HWS_TEST_GEN_STATUS;

/**
* @enum MV_HWS_PORT_PRESET_CMD
 *
 * @brief Defines PRESET commands.
*/
typedef enum{

    INIT = 1,

    PRESET = 2,

    PRESET_NA = 0xFFFF

} MV_HWS_PORT_PRESET_CMD;

/*
 * Typedef: struct MV_HWS_SERDES_TUNNIG_PARAMS
 *
 * Description: Defines parameters of port tuning.
 *
 */
typedef struct
{
    MV_HWS_PORT_PRESET_CMD  presetCmdVal;
    GT_BOOL                 eyeCheckEnVal;
    GT_U32                  gen1TrainAmp;
    GT_U32                  gen1TrainEmph0;
    GT_U32                  gen1TrainEmph1;
    GT_BOOL                 gen1AmpAdj;
    GT_U32                  initialAmp;
    GT_U32                  initialEmph0;
    GT_U32                  initialEmph1;
    GT_U32                  presetAmp;
    GT_U32                  presetEmph0;
    GT_U32                  presetEmph1;

}MV_HWS_SERDES_TUNING_PARAMS;

/**
* @enum MV_HWS_PORT_FLOW_CONTROL_ENT
 *
 * @brief Enumeration of Port FC Direction enablers.
*/
typedef enum{

    /** Both disabled */
    MV_HWS_PORT_FLOW_CONTROL_DISABLE_E = GT_FALSE,

    /** Both enabled */
    MV_HWS_PORT_FLOW_CONTROL_RX_TX_E = GT_TRUE,

    /** Only Rx FC enabled */
    MV_HWS_PORT_FLOW_CONTROL_RX_ONLY_E,

    /** Only Tx FC enabled */
    MV_HWS_PORT_FLOW_CONTROL_TX_ONLY_E

} MV_HWS_PORT_FLOW_CONTROL_ENT;

/**
* @internal mvHwsPortAutoTuneSet function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    void *                      results
);

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
);

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
);

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
);

/**
* @internal mvHwsPortEnhanceTuneLitePhase1 function
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
*                                      - for Serdes speed 25G (7...21)
* @param[in] max_dly                  - Maximum delay_cal value:
*                                      - for Serdes speed 10G (30)
*                                      - for Serdes speed 25G (23)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLitePhase1
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U8       min_dly,
    GT_U8       max_dly
);

/**
* @internal mvHwsPortEnhanceTuneLiteByPhaseInitDb function
* @endinternal
*
* @brief   Init HWS DB of EnhanceTuneLite by phase algorithm used for AP port
*
* @param[in] devNum                   - system device number
* @param[in] phyPortIndex             - ap port number index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnhanceTuneLiteByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  phyPortIndex
);

/**
* @internal mvHwsPortAutoTuneSetExt function
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
GT_STATUS mvHwsPortAutoTuneSetExt
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    GT_U32                  optAlgoMask,
    void *                  results
);

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
);

/**
* @internal mvHwsPortAutoTuneOptimization function
* @endinternal
*
* @brief   Run optimization for the training results.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              bit mask of optimization algorithms
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneOptimization
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  optAlgoMask
);

/**
* @internal mvHwsPortAutoTuneDelayInit function
* @endinternal
*
* @brief   Sets the delay values which are used in Serdes training optimization
*         algorithm
* @param[in] dynamicDelayInterval     - determines the number of training iteration in
*                                      which the delay will be executed (DFE algorithm)
* @param[in] dynamicDelayDuration     - delay duration in mSec (DFE algorithm)
* @param[in] staticDelayDuration      - delay duration in mSec (Align90 algorithm)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneDelayInit
(
    GT_U32     dynamicDelayInterval, /* DFE */
    GT_U32     dynamicDelayDuration, /* DFE */
    GT_U32     staticDelayDuration   /* Align90 */
);

/**
* @internal mvHwsPortManTuneSet function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManTuneSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    MV_HWS_AUTO_TUNE_RESULTS *tunParams
);

/**
* @internal mvHwsPortManualRxConfig function
* @endinternal
*
* @brief   configures SERDES Rx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      configParams- pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualRxConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    MV_HWS_MAN_TUNE_RX_CONFIG_DATA  *confifParams
);

/**
* @internal mvHwsPortManualTxConfig function
* @endinternal
*
* @brief   configures SERDES tx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] configParams             pointer to array of the config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualTxConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
);

/**
* @internal mvHwsPortManualTxConfigGet function
* @endinternal
*
* @brief   Get SERDES tx parameters for first SERDES lane.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] configParams             pointer to array of the config params structures:
*                                      txAmp     Tx amplitude
*                                      txAmpAdj  TX adjust
*                                      txemph0   TX emphasis 0
*                                      txemph1   TX emphasis 1
*                                      txAmpShft TX amplitude shift
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualTxConfigGet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
);

/**
* @internal mvHwsPortTxDefaultsSet function
* @endinternal
*
* @brief   Set the default Tx parameters on port.
*         Relevant for Bobcat2 and Alleycat3 devices.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxDefaultsSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

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
);

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
);

/**
* @internal mvHwsPortTestGenerator function
* @endinternal
*
* @brief   Activate the port related PCS Tx generator and Rx checker control.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPattern              - port tuning mode
* @param[in] actionMode               -
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTestGenerator
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_TEST_GEN_PATTERN  portPattern,
    MV_HWS_PORT_TEST_GEN_ACTION   actionMode
);

/**
* @internal mvHwsPortTestGeneratorStatus function
* @endinternal
*
* @brief   Get test errors - every get clears the errors.
*         Can be run any time after delete port or after power up
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      portTuningMode - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTestGeneratorStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_TEST_GEN_PATTERN  portPattern,
    MV_HWS_TEST_GEN_STATUS        *status
);

/**
* @internal mvHwsPortPPMSet function
* @endinternal
*
* @brief   Increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPPM                  - limited to +/- 3 taps
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PPM_VALUE        portPPM
);

/**
* @internal mvHwsPortPPMGet function
* @endinternal
*
* @brief   Check the entire line configuration, return ppm value in case of match in all
*         or error in case of different configuration.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] portPPM                  - current PPM
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PPM_VALUE        *portPPM
);

/**
* @internal mvHwsPortLoopbackSet function
* @endinternal
*
* @brief   Activates the port loopback modes.
*         Can be run only after create port not under traffic.
* @param[in] devNum     - system device number
* @param[in] portGroup  - port group (core) number
* @param[in] phyPortNum - physical port number
* @param[in] portMode   - port standard metric
* @param[in] lpPlace    - unit for loopback configuration
* @param[in] lpType     - loopback type
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS mvHwsPortLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_UNIT             lpPlace,
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsPortLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_UNIT             lpPlace,
    MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsPortLinkStatusGet function
* @endinternal
*
* @brief   Returns the port link status.
*         Can be run any time.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLinkStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

/**
* @internal mvHwsPortPolaritySet function
* @endinternal
*
* @brief   Defines the port polarity of the Serdes lanes (Tx/Rx).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] txInvMask                - bitmap of 32 bit, each bit represent Serdes
* @param[in] rxInvMask                - bitmap of 32 bit, each bit represent Serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPolaritySet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_32                   txInvMask,
    GT_32                   rxInvMask
);

/**
* @internal mvHwsPortTxEnable function
* @endinternal
*
* @brief   Turn of the port Tx according to selection.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - enable/disable port Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);


/**
* @internal mvHwsPortTxEnableGet function
* @endinternal
*
* @brief   Retrieve the status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL serdesTxStatus[]
);

/**
* @internal mvHwsPortSignalDetectGet function
* @endinternal
*
* @brief   Retrieve the signal detected status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] signalDet[]              per serdes.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSignalDetectGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 signalDet[]
);

/**
* @internal mvHwsPortCdrLockStatusGet function
* @endinternal
*
* @brief   Retrieve the CDR lock status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] cdrLockStatus[]          per serdes.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 cdrLockStatus[]
);

/**
* @internal mvHwsPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
* @param[in] vosOverride              - GT_TRUE means override, GT_FALSE means no override.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideControlModeSet
(
    GT_U8 devNum,
    GT_BOOL vosOverride
);

/**
* @internal mvHwsPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number.
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideControlModeGet
(
    GT_U8   devNum,
    GT_BOOL *vosOverridePtr
);

/**
* @internal hwsPortFaultSendSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] send                     - send or stop sending
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFaultSendSet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL send
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortMiscIf_H */



