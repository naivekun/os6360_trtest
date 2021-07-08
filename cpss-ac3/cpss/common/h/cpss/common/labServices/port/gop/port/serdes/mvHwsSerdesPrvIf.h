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
* @file mvHwsSerdesPrvIf.h
*
* @brief
*
* @version   61
********************************************************************************
*/

#ifndef __mvHwsSerdesPrvIf_H
#define __mvHwsSerdesPrvIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>

/* serdes PHY registers offset */
#define COM_H_REV2_PU_TX_INTP_Reg       (0x04)
#define COM_H_REV2_RX_DFE_Reg           (0x08)
#define COM_H_REV2_REG6_Reg             (0x0C)
#define COM_H_REV2_REG7_Reg             (0x10)
#define COM_H_REV2_PPM_Reg              (0x14)
#define COM_H_REV2_RX_FFE_Reg           (0x18)
#define COM_H_REV2_DFE_LOCK_Reg         (0x1C)
#define COM_H_REV2_CFG_Reg              (0x20)
#define COM_H_REV2_CFG_Reg1             (0x24)
#define COM_H_REV2_CFG_Reg2             (0x28)
#define COM_H_REV2_DFE_Reg              (0x2C)
#define COM_H_REV2_DFE_Reg1             (0x30)
#define COM_H_REV2_TX_AMP_Reg           (0x34)
#define COM_H_REV2_GEN1_SET1_Reg        (0x38)
#define COM_H_REV2_PT_PATTERN_SEL_Reg   (0x54)
#define COM_H_REV2_Pattern_Reg          (0x6C)
#define COM_H_REV2_FRAME_Cntr_MSB1_Reg  (0x70)
#define COM_H_REV2_FRAME_Cntr_MSB_Reg   (0x74)
#define COM_H_REV2_FRAME_Cntr_LSB_Reg   (0x78)
#define COM_H_REV2_ERROR_Cntr_MSB_Reg   (0x7C)
#define COM_H_REV2_ERROR_Cntr_LSB_Reg   (0x80)
#define COM_H_REV2_PHY_TEST_OOB_Reg     (0x84)
#define COM_H_REV2_SEL_BITS_Reg         (0x8C)
#define COM_H_REV2_INVERT_Reg           (0x90)
#define COM_H_REV2_Isolate_Reg          (0x98)
#define COM_H_REV2_REG10_Reg            (0xF0)
#define COM_H_REV2_GEN1_SET2_Reg        (0xF4)
#define COM_H_REV2_GEN2_SET2_Reg        (0xF8)

#define COM_H_REV2_TX_AmpDefault_Reg     (0xDC)
#define COM_H_REV2_TX_TRAIN_Reg2        (0x110)
#define COM_H_REV2_TX_TRAIN_Reg3        (0x114)
#define COM_H_REV2_REG1_Reg             (0x11C)
#define COM_H_REV2_REG3_Reg             (0x124)
#define COM_H_REV2_REG4_Reg             (0x128)
#define COM_H_REV2_REG5_Reg             (0x12C)
#define COM_H_REV2_TX_AmpDefault0       (0x138)
#define COM_H_REV2_TX_AmpDefault1       (0x13C)
#define COM_H_REV2_SoftReset_Reg        (0x148)
#define COM_H_REV2_FeedbackDivider_Reg  (0x14C)
#define COM_H_REV2_ForwardDivider_Reg   (0x150)
#define COM_H_REV2_CalibrReg            (0x15C)
#define COM_H_REV2_RxClockAl90          (0x168)
#define COM_H_REV2_TX_EMPH_Reg          (0x178)
#define COM_H_REV2_Pll_Cal_Control_P1   (0x180)
#define COM_H_REV2_OS_PH_OFFSET_Reg     (0x188)
#define COM_H_REV2_DFE_CFG_Reg          (0x19C)
#define COM_H_REV2_OS_PH_Reg            (0x1A0)
#define COM_H_REV2_FFE_Reg              (0x1A4)
#define COM_H_REV2_Ext_Squelch_Calibr   (0x1BC)
#define COM_H_REV2_Pll_Cal_Control0     (0x1C0)
#define COM_H_REV2_REG8_Reg             (0x1E0)
#define COM_H_REV2_REG9_Reg             (0x1E4)
#define COM_H_REV2_CID_Reg              (0x1F8)
#define COM_H_REV2_Loopback_Reg0        (0x28C)
#define COM_H_REV3_G1_SET_3_Reg         (0x440)
#define COM_H_REV3_G1_SET_4_Reg         (0x444)
#define COM_H_REV2_REG2_Reg             (0x1FC)

#ifdef MV_HWS_REDUCED_BUILD
#define HWS_MAX_SERDES_NUM 10
#else
#define HWS_MAX_SERDES_NUM 96
#endif

#define HWS_MAX_PORT_AVAGO_SERDES_NUM   4

#define MV_HWS_SERDES_NOT_ACTIVE    0xffffffff

#define MV_HWS_SERDES_NUM(laneNum)        (laneNum & 0xFFFF)

typedef enum
{
    SERDES_SD_RESET_SEQ,
    SERDES_SD_UNRESET_SEQ,
    SERDES_RF_RESET_SEQ,
    SERDES_RF_UNRESET_SEQ,
    SERDES_SYNCE_RESET_SEQ,
    SERDES_SYNCE_UNRESET_SEQ,

    SERDES_SERDES_POWER_UP_CTRL_SEQ,
    SERDES_SERDES_POWER_DOWN_CTRL_SEQ,
    SERDES_SERDES_RXINT_UP_SEQ,
    SERDES_SERDES_RXINT_DOWN_SEQ,
    SERDES_SERDES_WAIT_PLL_SEQ,

    SERDES_SPEED_1_25G_SEQ,
    SERDES_SPEED_3_125G_SEQ,
    SERDES_SPEED_3_75G_SEQ,
    SERDES_SPEED_4_25G_SEQ,
    SERDES_SPEED_5G_SEQ,
    SERDES_SPEED_6_25G_SEQ,
    SERDES_SPEED_7_5G_SEQ,
    SERDES_SPEED_10_3125G_SEQ,

    SERDES_SD_LPBK_NORMAL_SEQ,
    SERDES_SD_ANA_TX_2_RX_SEQ,
    SERDES_SD_DIG_TX_2_RX_SEQ,
    SERDES_SD_DIG_RX_2_TX_SEQ,

    SERDES_PT_AFTER_PATTERN_NORMAL_SEQ,
    SERDES_PT_AFTER_PATTERN_TEST_SEQ,

    SERDES_RX_TRAINING_ENABLE_SEQ,
    SERDES_RX_TRAINING_DISABLE_SEQ,
    SERDES_TX_TRAINING_ENABLE_SEQ,
    SERDES_TX_TRAINING_DISABLE_SEQ,

    SERDES_SPEED_12_5G_SEQ,
    SERDES_SPEED_3_3G_SEQ,

    SERDES_SPEED_11_5625G_SEQ,
    SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ,
    SERDES_SERDES_PARTIAL_POWER_UP_SEQ,

    SERDES_SPEED_11_25G_SEQ,
    SERDES_CORE_RESET_SEQ,
    SERDES_CORE_UNRESET_SEQ,

    SERDES_FFE_TABLE_LR_SEQ,
    SERDES_FFE_TABLE_SR_SEQ,
    SERDES_SPEED_10_9375G_SEQ,
    SERDES_SPEED_12_1875G_SEQ,
    SERDES_SPEED_5_625G_SEQ,
    SERDES_SPEED_5_15625G_SEQ,

    MV_SERDES_LAST_SEQ

}MV_HWS_COM_PHY_H_SUB_SEQ;

/******************************** enums ***************************************/
typedef enum
{
    SERDES_SPEED_1_25G,
    SERDES_SPEED_3_125G,
    SERDES_SPEED_3_3G,
    SERDES_SPEED_3_75G,
    SERDES_SPEED_4_25G,
    SERDES_SPEED_5G,
    SERDES_SPEED_6_25G,
    SERDES_SPEED_7_5G,
    SERDES_SPEED_10_3125G,
    SERDES_SPEED_11_5625G,
    SERDES_SPEED_12_5G,
    SERDES_SPEED_10_9375G,
    SERDES_SPEED_12_1875G,
    SERDES_SPEED_5_625G,
    SERDES_SPEED_5_15625G,

    SERDES_LAST_SPEED

} MV_SERDES_SPEED;

typedef enum
{
    /* for COM_PHY */
    DFE_DISABLE,
    DFE_ENABLE,
    DFE_FREEZE,
    DFE_FORCE,

    /* for Avago */
    DFE_STOP_ADAPTIVE,
    DFE_START_ADAPTIVE,
    DFE_ICAL,
    DFE_PCAL,
    DFE_ICAL_VSR        /* iCal-VSR mode, Bypass DFE TAP tuning */

}MV_HWS_DFE_MODE;

typedef enum
{
  SERDES_LP_DISABLE,
  SERDES_LP_AN_TX_RX,
  SERDES_LP_DIG_TX_RX,
  SERDES_LP_DIG_RX_TX

}MV_HWS_SERDES_LB;

/* used to hold data required to init the serdes */
typedef struct
{
    GT_U32  regOffset;
    GT_U32  mask;
    GT_U32  regData[SERDES_LAST_SPEED];

} MV_SPEED_CONFIG_REG_DATA;

typedef struct
{
    GT_U16   unitId;
    GT_U32   regOffset;
    GT_U32   regData[SERDES_LAST_SPEED];
    GT_U32   mask;

}MV_OP_SERDES_PARAMS;

typedef struct
{
    MV_EL_DB_OPERATION op;
    MV_OP_SERDES_PARAMS       params;

}MV_CFG_SERDES_ELEMENT;

/******************************** functores ***************************************/
typedef GT_STATUS (*MV_HWS_SERDES_ARRAY_POWER_CNTRL_FUNC_PTR)
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     numOfSer,
    GT_UOPT                     *serdesArr,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_POWER_CNTRL_FUNC_PTR)
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_MANUAL_TX_CFG_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_UREG_DATA    txAmp,
    GT_BOOL         txAmpAdj,
    GT_REG_DATA     emph0,
    GT_REG_DATA     emph1,
    GT_UREG_DATA    txAmpShft,
    GT_BOOL         txEmphEn
);

typedef GT_STATUS (*MV_HWS_SERDES_MANUAL_RX_CFG_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    GT_UREG_DATA         sqlch,
    GT_UREG_DATA         ffeRes,
    GT_UREG_DATA         ffeCap,
    GT_BOOL             dfeEn,
    GT_UREG_DATA         alig
);

typedef GT_STATUS (*MV_HWS_SERDES_LB_CFG_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
);

typedef GT_STATUS (*MV_HWS_SERDES_LB_GET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbModePtr
);

typedef GT_STATUS (*MV_HWS_SERDES_PPM_CFG_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_PPM_VALUE    ppmVal
);

typedef GT_STATUS (*MV_HWS_SERDES_PPM_GET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_PPM_VALUE    *ppmVal
);

typedef GT_STATUS (*MV_HWS_SERDES_TEST_GEN_FUNC_PTR)
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    MV_HWS_SERDES_TEST_GEN_MODE mode
);

typedef GT_STATUS (*MV_HWS_SERDES_TEST_GEN_GET_FUNC_PTR)
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

typedef char* (*MV_HWS_SERDES_TYPE_GET_FUNC_PTR)(void);

typedef GT_STATUS (*MV_HWS_SERDES_TEST_GEN_STATUS_FUNC_PTR)
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
);

typedef GT_STATUS (*MV_HWS_SERDES_EOM_GET_FUNC_PTR)
(
    GT_U8                     devNum,
    GT_UOPT                   portGroup,
    GT_UOPT                   serdesNum,
    GT_UREG_DATA              timeout,
    MV_HWS_SERDES_EOM_RES     *results
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_CFG_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

typedef GT_STATUS (*MV_HWS_SERDES_GEN_BOOL_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_RXTX_START_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_RESULT_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_RESULTS *results
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

typedef GT_STATUS (*MV_HWS_SERDES_RESET_FUNC_PTR)
(
    GT_U8      devNum,
    GT_UOPT    portGroup,
    GT_UOPT    serdesNum,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset
);

typedef GT_STATUS (*MV_HWS_SERDES_RESET_EXT_FUNC_PTR)
(
    GT_U8      devNum,
    GT_UOPT    portGroup,
    GT_UOPT    serdesNum,
    GT_BOOL    analogReset,
    GT_BOOL    digitalReset,
    GT_BOOL    syncEReset,
    GT_BOOL    coreReset
);

typedef GT_STATUS (*MV_HWS_SERDES_DFE_CFG_FUNC_PTR)
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_DFE_MODE   dfeMode,
    GT_REG_DATA       *dfeCfg
);

typedef GT_STATUS (*MV_HWS_SERDES_DFE_STATUS_FUNC_PTR)
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *dfeLock,
    GT_32             *dfeCfg
);

typedef GT_STATUS (*MV_HWS_SERDES_DFE_STATUS_EXT_FUNC_PTR)
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *dfeLock,
    GT_REG_DATA       *dfeCfg,
    GT_REG_DATA       *f0Cfg,
    GT_REG_DATA       *savCfg
);

typedef GT_STATUS (*MV_HWS_SERDES_POLARITY_CFG_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
);

typedef GT_STATUS (*MV_HWS_SERDES_POLARITY_CFG_GET_FUNC_PTR)
(
  GT_U8             devNum,
  GT_UOPT           portGroup,
  GT_UOPT           serdesNum,
  GT_BOOL           *invertTx,
  GT_BOOL           *invertRx
);

typedef GT_STATUS (*MV_HWS_SERDES_DFE_OPTI_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_UREG_DATA  *ffeR
);

typedef GT_STATUS (*MV_HWS_SERDES_FFE_CFG_FUNC_PTR)
(
  GT_U8   devNum,
  GT_UOPT portGroup,
  GT_UOPT serdesNum,
  GT_UREG_DATA  ffeRes,
  GT_UREG_DATA  ffeCap,
  GT_UREG_DATA  alig
);

typedef GT_STATUS (*MV_HWS_SERDES_CALIBRATION_STATUS_FUNC_PTR)
(
 GT_U8   devNum,
 GT_UOPT portGroup,
 GT_UOPT serdesNum,
 MV_HWS_CALIBRATION_RESULTS *results
);

typedef GT_STATUS (*MV_HWS_SERDES_TX_ENABLE_GET_FUNC_PTR)
(
 GT_U8   devNum,
 GT_UOPT portGroup,
 GT_UOPT serdesNum,
 GT_BOOL *enable
);

typedef GT_STATUS (*MV_HWS_SERDES_TX_IF_SELECT_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        serdesTxIfNum
);

typedef GT_STATUS (*MV_HWS_SERDES_TX_IF_SELECT_GET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        *serdesTxIfNum
);

typedef GT_STATUS (*MV_HWS_SERDES_SQLCH_CFG_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_UREG_DATA  sqlch
);

typedef GT_STATUS (*MV_HWS_SERDES_DIGITAL_RESET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_RESET        digitalReset
);

typedef GT_STATUS (*MV_HWS_SERDES_FIX_AL90_START_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_ALIGN90_PARAMS   *params
);

typedef GT_STATUS (*MV_HWS_SERDES_FIX_AL90_STOP_FUNC_PTR)
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_ALIGN90_PARAMS *params,
    GT_BOOL     fixAlignPass
);

typedef GT_STATUS (*MV_HWS_SERDES_FIX_AL90_STATUS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *trainingStatus
);

typedef GT_STATUS (*MV_HWS_SERDES_SEQ_GET_FUNC_PTR)
(
    MV_HWS_COM_PHY_H_SUB_SEQ  seqType,
    MV_CFG_ELEMENT            *seqLine,
    GT_UOPT                   lineNum
);

typedef GT_STATUS (*MV_HWS_SERDES_SEQ_SET_FUNC_PTR)
(
    GT_BOOL                   firstLine,
    MV_HWS_COM_PHY_H_SUB_SEQ  seqType,
    MV_CFG_ELEMENT            *seqLine,
    GT_UOPT                   numOfOp
);

typedef GT_STATUS (*MV_HWS_SERDES_SHORT_CORE_RESET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_RESET        coreReset
);

typedef GT_STATUS (*MV_HWS_SERDES_AUTO_TUNE_STATUS_SHORT_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
);

typedef GT_STATUS (*MV_HWS_SERDES_GEN_FUNC_PTR)
(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
);

typedef GT_STATUS (*MV_HWS_SERDES_SPEED_GET_FUNC_PTR)
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    MV_HWS_SERDES_SPEED *rate
);

typedef GT_STATUS (*MV_HWS_SERDES_MANUAL_RX_CFG_GET_FUNC_PTR)
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_PORT_MAN_TUNE_MODE *portTuningModePtr,
    GT_U32             *sqlchPtr,
    GT_U32             *ffeResPtr,
    GT_U32             *ffeCapPtr,
    GT_BOOL            *dfeEnPtr,
    GT_U32             *aligPtr
);

typedef GT_STATUS (*MV_HWS_SERDES_MANUAL_TX_CFG_GET_FUNC_PTR)
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         serdesNum,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
);

typedef GT_STATUS (*MV_HWS_SERDES_GEN_BOOL_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
);

typedef GT_STATUS (*MV_HWS_SERDES_SCAN_SAMPLER_FUNC_PTR)
(
    GT_U8                  devNum,
    GT_U8                  portGroup,
    GT_U32                 serdesNum,
    GT_U32                 waitTime,
    GT_U32                 samplerSelect,
    GT_U32                 errorThreshold,
    MV_HWS_SAMPLER_RESULTS *samplerResults
);

typedef GT_STATUS (*MV_HWS_SERDES_PARTIAL_POWER_DOWN_FUNC_PTR)
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     powerDownRx,
    GT_BOOL     powerDownTx
);

typedef GT_STATUS (*MV_HWS_SERDES_PARTIAL_POWER_STATUS_FUNC_PTR)
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     *powerDownRx,
    GT_BOOL     *powerDownTx
);


/******************************** Functions Struct ***************************************/

typedef struct
{
  MV_HWS_SERDES_ARRAY_POWER_CNTRL_FUNC_PTR  serdesArrayPowerCntrlFunc;
  MV_HWS_SERDES_POWER_CNTRL_FUNC_PTR        serdesPowerCntrlFunc;
  MV_HWS_SERDES_MANUAL_TX_CFG_FUNC_PTR      serdesManualTxCfgFunc;
  MV_HWS_SERDES_MANUAL_RX_CFG_FUNC_PTR      serdesManualRxCfgFunc;
  MV_HWS_SERDES_LB_CFG_FUNC_PTR             serdesLoopbackCfgFunc;
  MV_HWS_SERDES_LB_GET_FUNC_PTR             serdesLoopbackGetFunc;
  MV_HWS_SERDES_PPM_CFG_FUNC_PTR            serdesPpmCfgFunc;
  MV_HWS_SERDES_PPM_GET_FUNC_PTR            serdesPpmGetFunc;
  MV_HWS_SERDES_TEST_GEN_FUNC_PTR           serdesTestGenFunc;
  MV_HWS_SERDES_TEST_GEN_GET_FUNC_PTR       serdesTestGenGetFunc;
  MV_HWS_SERDES_TYPE_GET_FUNC_PTR           serdesTypeGetFunc;
  MV_HWS_SERDES_TEST_GEN_STATUS_FUNC_PTR    serdesTestGenStatusFunc;
  MV_HWS_SERDES_EOM_GET_FUNC_PTR            serdesEomGetFunc;
  MV_HWS_SERDES_AUTO_TUNE_CFG_FUNC_PTR      serdesAutoTuneCfgFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR           serdesRxAutoTuneStartFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR           serdesTxAutoTuneStartFunc;
  MV_HWS_SERDES_AUTO_TUNE_RXTX_START_FUNC_PTR serdesAutoTuneStartFunc;
  MV_HWS_SERDES_AUTO_TUNE_RESULT_FUNC_PTR   serdesAutoTuneResultFunc;
  MV_HWS_SERDES_AUTO_TUNE_STATUS_FUNC_PTR   serdesAutoTuneStatusFunc;
  MV_HWS_SERDES_RESET_FUNC_PTR              serdesResetFunc;
  MV_HWS_SERDES_DFE_CFG_FUNC_PTR            serdesDfeCfgFunc;
  MV_HWS_SERDES_DFE_STATUS_FUNC_PTR         serdesDfeStatusFunc;
  MV_HWS_SERDES_DFE_STATUS_EXT_FUNC_PTR     serdesDfeStatusExtFunc;
  MV_HWS_SERDES_POLARITY_CFG_FUNC_PTR       serdesPolarityCfgFunc;
  MV_HWS_SERDES_POLARITY_CFG_GET_FUNC_PTR   serdesPolarityCfgGetFunc;
  MV_HWS_SERDES_DFE_OPTI_FUNC_PTR           serdesDfeOptiFunc;
  MV_HWS_SERDES_FFE_CFG_FUNC_PTR            serdesFfeCfgFunc;
  MV_HWS_SERDES_CALIBRATION_STATUS_FUNC_PTR serdesCalibrationStatusFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR           serdesTxEnableFunc;
  MV_HWS_SERDES_TX_ENABLE_GET_FUNC_PTR      serdesTxEnableGetFunc;
  MV_HWS_SERDES_TX_IF_SELECT_FUNC_PTR       serdesTxIfSelectFunc;
  MV_HWS_SERDES_TX_IF_SELECT_GET_FUNC_PTR   serdesTxIfSelectGetFunc;
  MV_HWS_SERDES_SQLCH_CFG_FUNC_PTR          serdesSqlchCfgFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR           serdesAcTermCfgFunc;
  MV_HWS_SERDES_DIGITAL_RESET_FUNC_PTR      serdesDigitalReset;
  MV_HWS_SERDES_FIX_AL90_START_FUNC_PTR     serdesFixAlign90Start;
  MV_HWS_SERDES_FIX_AL90_STOP_FUNC_PTR      serdesFixAlign90Stop;
  MV_HWS_SERDES_FIX_AL90_STATUS_FUNC_PTR    serdesFixAlign90Status;
  MV_HWS_SERDES_RESET_EXT_FUNC_PTR          serdesResetExtFunc;
  MV_HWS_SERDES_SEQ_GET_FUNC_PTR            serdesSeqGetFunc;
  MV_HWS_SERDES_SEQ_SET_FUNC_PTR            serdesSeqSetFunc;
  MV_HWS_SERDES_SHORT_CORE_RESET_FUNC_PTR   serdesCoreReset;
  MV_HWS_SERDES_AUTO_TUNE_STATUS_SHORT_FUNC_PTR serdesAutoTuneStatusShortFunc;
  MV_HWS_SERDES_GEN_FUNC_PTR                serdesAutoTuneStopFunc;
  MV_HWS_SERDES_GEN_FUNC_PTR                serdesDfeCheckFunc;
  MV_HWS_SERDES_SPEED_GET_FUNC_PTR          serdesSpeedGetFunc;
  MV_HWS_SERDES_MANUAL_RX_CFG_GET_FUNC_PTR  serdesManualRxCfgGetFunc;
  MV_HWS_SERDES_MANUAL_TX_CFG_GET_FUNC_PTR  serdesManualTxCfgGetFunc;
  MV_HWS_SERDES_GEN_BOOL_GET_FUNC_PTR       serdesSignalDetectGetFunc;
  MV_HWS_SERDES_GEN_BOOL_GET_FUNC_PTR       serdesCdrLockStatusGetFunc;
  MV_HWS_SERDES_GEN_BOOL_FUNC_PTR           serdesSqDetectCfgFunc;
  MV_HWS_SERDES_SCAN_SAMPLER_FUNC_PTR       serdesScanSamplerFunc;
  MV_HWS_SERDES_PARTIAL_POWER_DOWN_FUNC_PTR serdesPartialPowerDownFunc;
  MV_HWS_SERDES_PARTIAL_POWER_STATUS_FUNC_PTR serdesPartialPowerStatusGetFunc;
  MV_HWS_SERDES_POWER_CNTRL_FUNC_PTR        serdesApPowerCntrlFunc;

}MV_HWS_SERDES_FUNC_PTRS;


/******************************** declarations ***************************************/


/**
* @internal hwsSerdesIfInit function
* @endinternal
*
* @brief   Init all supported serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsSerdesIfInit(GT_U8 deviceNum);

/**
* @internal mvHwsSerdesPowerCtrl function
* @endinternal
*
* @brief   Init physical port.
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
);

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
);

/**
* @internal mvHwsSerdesManualRxConfig function
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
GT_STATUS mvHwsSerdesManualRxConfig
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode,
    GT_U32  sqlch,
    GT_U32  ffeRes,
    GT_U32  ffeCap,
    GT_BOOL dfeEn,
    GT_U32  alig
);

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
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    GT_U32  txAmp,
    GT_BOOL txAmpAdj,
    GT_32   emph0,
    GT_32   emph1,
    GT_BOOL txAmpShft,
    GT_BOOL txEmphEn
);

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
);

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
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_SERDES_LB  lbMode
);

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
);

/**
* @internal mvHwsSerdesPpmConfig function
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
GT_STATUS mvHwsSerdesPpmConfig
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_PPM_VALUE  ppmVal
);

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
);

/**
* @internal mvHwsSerdesAutoTuneCfg function
* @endinternal
*
* @brief   Per SERDES configure parameters for TX training & Rx Training starting
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
GT_STATUS mvHwsSerdesAutoTuneCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
);

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
);

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
);

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
);

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
);

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
);

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
);

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
    GT_32             *dfeCfg
);

/**
* @internal mvHwsSerdesDfeStatus function
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
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDfeStatus
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL           *dfeLock,
    GT_32             *dfeCfg
);

/**
* @internal mvHwsSerdesDfeStatusExt function
* @endinternal
*
* @brief   Per SERDES get the of DFE status and parameters (extended API).
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
);

/**
* @internal mvHwsSerdesDfeOpti function
* @endinternal
*
* @brief   Per SERDES find best FFE_R is the min value for scanned DFE range.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] ffeR                     - best value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesDfeOpti
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_U32  *ffeR
);

/**
* @internal mvHwsSerdesFfeConfig function
* @endinternal
*
* @brief   Per SERDES configure FFE.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] ffeRes                   - FFE R value
* @param[in] ffeCap                   - FFE C value
* @param[in] alig                     - sampler value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesFfeConfig
(
  GT_U8   devNum,
  GT_U32  portGroup,
  GT_U32  serdesNum,
  MV_HWS_SERDES_TYPE serdesType,
  GT_U32  ffeRes,
  GT_U32  ffeCap,
  GT_U32  alig
);

/**
* @internal mvHwsSerdesCalibrationStatus function
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
GT_STATUS mvHwsSerdesCalibrationStatus
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    MV_HWS_SERDES_TYPE          serdesType,
    MV_HWS_CALIBRATION_RESULTS  *results
);

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
);

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
GT_STATUS mvHwsSerdesTxEnableGet
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL *enablePtr
);

/**
* @internal mvHwsSerdesTxIfSelect function
* @endinternal
*
* @brief   tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - SERDES types
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
);

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
);

/**
* @internal mvHwsSerdesSqlchCfg function
* @endinternal
*
* @brief   Configure squelch threshold value.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] sqlchVal                 - squelch threshold value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesSqlchCfg
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    GT_U32              sqlchVal
);

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
);

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
);

/**
* @internal hwsSerdesIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsSerdesIfClose(void);

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
);

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
);

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
);

void hwsChangeSpeedCfgParams(MV_SERDES_SPEED speed, GT_U32 regOffset, GT_U32 regValue);
void hwsChange1GSpeedCfgV43(void);


/*******************************************************************************
* hwsSetSeqParams
*
*       update parameter of sequence according to received ID
*       The function will search the received register offset in the sequence parameters
*       if the register offset was found, the function will check
*       if mask of this offset contain the bits of the received mask.
*       If it contain the received mask,
*       function will update the received data in the sequence parameter
*       If it not found the function will return no such.
*
* INPUTS:
*       seqId:      ID of sequence
*       offset:     register offset, number of the register to update
*       data:       data to update
*       dataMask:   bits to update
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK - on success
*       GT_NO_SUCH/GT_NOT_SUPPORTED - on error
*
*******************************************************************************/
GT_STATUS hwsSetSeqParams
(
    GT_U32  seqId,
    GT_U32  Offset,
    GT_U32  data,
    GT_U32  dataMask
);


/**
* @internal mvHwsSerdesRev2DfeCheck function
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
GT_STATUS mvHwsSerdesRev2DfeCheck
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType
);

/**
* @internal mvHwsSerdesSpeedGet function
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
GT_STATUS mvHwsSerdesSpeedGet
(
    GT_U8               devNum,
    GT_U32              portGroup,
    GT_U32              serdesNum,
    MV_HWS_SERDES_TYPE  serdesType,
    MV_HWS_SERDES_SPEED *rate
);

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
);

/**
* @internal mvHwsSerdesAcTerminationCfg function
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
GT_STATUS mvHwsSerdesAcTerminationCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    GT_BOOL                 acTermEn
);

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
);

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
);

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
* @param[in] signalDet                - if true, signal was detected
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
);

/**
* @internal mvHwsSerdesCdrLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - notlocked).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
* @param[in] cdrLock                  - if true, CDR was locked
*
* @param[out] cdrLock                  - if true, CDR was locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    GT_BOOL                 *cdrLock
);

/**
* @internal mvHwsSerdesScanSampler function
* @endinternal
*
* @brief   Scan SERDES sampler.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical PCS number
* @param[in] serdesType               - serdes PCS type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
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
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsSerdesPrvIf_H */


