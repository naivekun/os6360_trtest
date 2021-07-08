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
* @file mvHwsSerdesIf.h
*
* @brief
*
* @version   23
********************************************************************************
*/

#ifndef __mvHwsSerdesIf_H
#define __mvHwsSerdesIf_H

#ifdef __cplusplus
extern "C" {
#endif


/* General H Files */
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>

/**
* @enum MV_HWS_SERDES_TX_PATTERN
 *
 * @brief Defines different serdes transmit patterns.
 * Comment: Other - as output value means any other mode not mentioned
 * in this enum;
 * as input means HW default for PHY Test Data 5
 * register;
*/
typedef enum
{
  _1T,
  _2T,
  _5T,
  _10T,
  PRBS7,
  PRBS9,
  PRBS15,
  PRBS23,
  PRBS31,
  DFETraining,
  Other,
  PRBS11,
  PRBS13,


  LAST_SERDES_TX_PATTERN

}MV_HWS_SERDES_TX_PATTERN;

/**
* @enum MV_HWS_SERDES_TEST_GEN_MODE
 *
 * @brief Defines different serdes test generator modes.
*/
typedef enum{

    SERDES_NORMAL,

    SERDES_TEST

} MV_HWS_SERDES_TEST_GEN_MODE;

/**
* @enum MV_HWS_SERDES_DIRECTION
 *
 * @brief Defines SERDES direction enumerator.
*/
typedef enum
{
    RX_DIRECTION,
    TX_DIRECTION

}MV_HWS_SERDES_DIRECTION;


/*
 * Typedef: struct MV_HWS_SERDES_TEST_GEN_STATUS
 *
 * Description: Defines serdes test generator results.
 *
 */
typedef struct
{
  GT_U32  errorsCntr;
  GT_U64  txFramesCntr;
  GT_U32  lockStatus;

}MV_HWS_SERDES_TEST_GEN_STATUS;

/*
 * Typedef: struct MV_HWS_SERDES_EOM_RES
 *
 * Description: Defines serdes EOM results.
 *
 */
typedef struct
{
    GT_U32  horizontalEye;
    GT_U32  verticalEye;
    GT_U32  eomMechanism;
    GT_U32  rowSize;
    GT_U32  *lowerMatrix;
    GT_U32  *upperMatrix;
} MV_HWS_SERDES_EOM_RES;

typedef struct
{
    GT_U32 startAlign90;
    GT_U32 rxTrainingCfg;
    GT_U32 osDeltaMax;
    GT_U32 adaptedFfeR;
    GT_U32 adaptedFfeC;

}MV_HWS_ALIGN90_PARAMS;

typedef struct
{
    GT_U32 bestVal;
    GT_U32 positiveVal;
    GT_U32 negativeVal;

}MV_HWS_SAMPLER_RESULTS;

typedef enum
{
    HWS_D2D_DIRECTION_MODE_BOTH_E,
    HWS_D2D_DIRECTION_MODE_RX_E,
    HWS_D2D_DIRECTION_MODE_TX_E
}MV_HWS_D2D_DIRECTION_MODE_ENT;


typedef struct
{
    MV_HWS_D2D_DIRECTION_MODE_ENT       direction;                  /* common parameter */
    MV_HWS_PORT_FEC_MODE                fecMode;                    /* relate to Lane Tester PRBS */
    GT_BOOL                             userDefPatternEnabled;      /* relate to Lane Tester PRBS */
    GT_U32                              userDefPattern[4];          /* relate to Lane Tester PRBS */
    MV_HWS_SERDES_TX_PATTERN            serdesPattern;              /* relate to Lane Tester PRBS */
    MV_HWS_SERDES_TEST_GEN_MODE         mode;                       /* relate to Lane Tester PRBS */
    MV_HWS_PORT_TEST_GEN_PATTERN        portPattern;                /* relate to PCS/PMA PRBS */
}MV_HWS_D2D_PRBS_CONFIG_STC;

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
);

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
);

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
);

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
);

/**
* @internal mvHwsSerdesEOMGet function
* @endinternal
*
* @brief   Returns the horizontal/vertical Rx eye margin.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] timeout                  - wait time in msec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesEOMGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TYPE        serdesType,
    GT_U32                    timeout,
    MV_HWS_SERDES_EOM_RES     *results
);

/**
* @internal mvHwsSerdesPolarityConfig function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
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
);

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
);

/**
* @internal mvHwsSerdesFixAlign90Start function
* @endinternal
*
* @brief   Start fix Align90 process on current SERDES.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] params                   - pointer to structure to store SERDES configuration parameters that must be restore
*                                      during process stop operation
*
* @param[out] params                   - SERDES configuration parameters that must be restore
*                                      during process stop operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFixAlign90Start
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_ALIGN90_PARAMS   *params
);

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
);

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
);

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
);

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
);

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
);

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
GT_STATUS mvHwsPortTxRxDefaultsDbEntryGet
(
    GT_U8                               devNum,
    MV_HWS_PORT_STANDARD                portMode,
    MV_HWS_SERDES_SPEED                 serdesSpeed,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA      *txParamsPtr,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *ctleParamsPtr
);
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsSerdesIf_H */


