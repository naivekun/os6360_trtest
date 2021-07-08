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
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvComPhyHRev28nmRev3If_H
#define __mvComPhyHRev28nmRev3If_H

/* General H Files */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmIf.h>

#ifdef __cplusplus
extern "C" {
#endif



/**
* @internal mvHwsComH28nmRev3IfInit function
* @endinternal
*
* @brief   Init Com_H serdes configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmRev3IfInit(MV_HWS_SERDES_FUNC_PTRS *funcPtrArray);

#ifndef CO_CPU_RUN

/**
* @internal mvHwsComH28nmRev3SerdesManualRxConfig function
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
GT_STATUS mvHwsComH28nmRev3SerdesManualRxConfig
(
    GT_U8   devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    GT_UREG_DATA  sqlch,
    GT_UREG_DATA  ffeRes,
    GT_UREG_DATA  ffeCap,
    GT_BOOL dfeEn,
    GT_UREG_DATA  alig
);

/**
* @internal mvHwsComH28nmRev3SerdesF0dGet function
* @endinternal
*
* @brief   Returns the F0d value of DFE register
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] F0d                      value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsComH28nmRev3SerdesF0dGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_U32      *F0d
);

/**
* @internal mvHwsComH28nmRev3SerdesSignalDetectGet function
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
GT_STATUS mvHwsComH28nmRev3SerdesSignalDetectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalDet
);

/**
* @internal mvHwsComH28nmRev3SerdesFixAlign90 function
* @endinternal
*
* @brief   Start fix Align90 process on current SERDES.
*         Can be run after create port.
*         For Rev 3.0: SW based search of align90 according to DFE_F1, DFE_F2
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmRev3SerdesFixAlign90
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum
);

/**
* @internal mvHwsComH28nmRev3FindBetterAdaptation function
* @endinternal
*
* @brief   Find a better Align90 adaptation value when temperature on
*         device is changed from cold->hot or from hot->cold.
*         Loops on certain FFE_R and Align90 values and calls to
*         mvHwsComH28nmRev3SerdesF0dGet to evaluate the improvment.
*         This procedure is relevant only when:
*         - Serdes speeds is 10.3125G and above
*         - signal detect is 0
*         - CDR is locked
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] threshDb                 - threshold in db
* @param[in] avgFactor                - number of iterations for getting the average value of F0d
*
* @param[out] donePtr                  - indicate if Better Adaptation algorithm run on
*                                      Serdes or not
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - if Serdes is in Power Down
* @retval GT_NOT_SUPPORTED         - if Serdes speed is wrong
* @retval GT_NOT_READY             - if SQ_detect is not 0 and CDR is not locked
*/
GT_STATUS mvHwsComH28nmRev3FindBetterAdaptation
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_U32      threshDb,
    GT_U32      avgFactor,
    GT_BOOL     *donePtr
);

/**
* @internal mvHwsComH28nmRev3SerdesPartialPowerDown function
* @endinternal
*
* @brief   Enable/Disable Serdes power down on Tx and Rx.
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
GT_STATUS mvHwsComH28nmRev3SerdesPartialPowerDown
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     powerDownRx,
    GT_BOOL     powerDownTx
);

/**
* @internal mvHwsComH28nmRev3SerdesPartialPowerStatusGet function
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
GT_STATUS mvHwsComH28nmRev3SerdesPartialPowerStatusGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     *powerRxStatus,
    GT_BOOL     *powerTxStatus
);

/**
* @internal mvHwsComH28nmRev3SerdesApPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list for 802.3AP protocol.
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
GT_STATUS mvHwsComH28nmRev3SerdesApPowerCtrl
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

/**
* @internal mvHwsComH28nmRev3SerdesSpeedGet function
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
GT_STATUS mvHwsComH28nmRev3SerdesSpeedGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_SPEED *rate
);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __mvComPhyHRev28nmRev3If_H */


