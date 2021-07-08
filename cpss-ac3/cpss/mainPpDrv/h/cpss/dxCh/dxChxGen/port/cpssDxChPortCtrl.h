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
* @file cpssDxChPortCtrl.h
*
* @brief CPSS implementation for Port configuration and control facility.
*
* @version   88
********************************************************************************
*/

#ifndef __cpssDxChPortCtrlh
#define __cpssDxChPortCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpss/common/port/private/prvCpssPortManagerTypes.h>

/* definition for the number of PSC lanes per port */
#define CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS 4

#define CPSS_DXCH_PORT_MAX_MRU_CNS 10304  /* jumbo frame + 64  (including 4 bytes CRC) */

#define CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS 32

#define CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS 128

/**
* @enum CPSS_DXCH_PORT_MAC_ERROR_TYPE_ENT
 *
 * @brief Enumeration of the MAC error types.
*/
typedef enum{

    /** Jabber error type */
    CPSS_DXCH_PORT_MAC_ERROR_JABBER_E,

    /** Fragment error type */
    CPSS_DXCH_PORT_MAC_ERROR_FRAGMENTS_E,

    /** Undersize error type */
    CPSS_DXCH_PORT_MAC_ERROR_UNDERSIZE_E,

    /** Oversize error type */
    CPSS_DXCH_PORT_MAC_ERROR_OVERSIZE_E,

    /** Rx error type */
    CPSS_DXCH_PORT_MAC_ERROR_RX_E,

    /** CRC error type */
    CPSS_DXCH_PORT_MAC_ERROR_CRC_E,

    /** Overrun error type */
    CPSS_DXCH_PORT_MAC_ERROR_OVERRUN_E,

    /** No error */
    CPSS_DXCH_PORT_MAC_ERROR_NO_E

} CPSS_DXCH_PORT_MAC_ERROR_TYPE_ENT;

/**
* @enum CPSS_DXCH_PORT_FC_MODE_ENT
 *
 * @brief Enumeration of FC modes.
*/
typedef enum{

    /** 802.3X */
    CPSS_DXCH_PORT_FC_MODE_802_3X_E,

    /** PFC: Priority Flow Control */
    CPSS_DXCH_PORT_FC_MODE_PFC_E,

    /** LL FC: Link Level Flow control */
    CPSS_DXCH_PORT_FC_MODE_LL_FC_E,

    /** Disable Flow control - Falcon only*/
    CPSS_DXCH_PORT_FC_MODE_DISABLE_E

} CPSS_DXCH_PORT_FC_MODE_ENT;

/**
* @enum CPSS_DXCH_PORT_SERDES_SPEED_ENT
 *
 * @brief Defines SERDES speed.
*/
typedef enum{

    /** Serdes speed is 1.25G, used by 1000Base_X , SGMII */
    CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,

    /** Serdes speed is 3.125G, used by XAUI, 2500Base_X/2.5G SGMII */
    CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,

    /** Serdes speed is 3.75G */
    CPSS_DXCH_PORT_SERDES_SPEED_3_75_E,

    /** Serdes speed is 6.25G, used by DXAUI and RXAUI */
    CPSS_DXCH_PORT_SERDES_SPEED_6_25_E,

    /** Serdes speed is 5G */
    CPSS_DXCH_PORT_SERDES_SPEED_5_E,

    /** Serdes speed is 4.25G */
    CPSS_DXCH_PORT_SERDES_SPEED_4_25_E,

    /** Serdes speed is 2.5G */
    CPSS_DXCH_PORT_SERDES_SPEED_2_5_E,

    /** Serdes speed is 5.156G (x8 serdes = 40G data speed) */
    CPSS_DXCH_PORT_SERDES_SPEED_5_156_E,

    /** used by KR, SR_LR, 40G KR4, 40G SR_LR, 20G_KR */
    CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E,

    /** used by HGL 16G */
    CPSS_DXCH_PORT_SERDES_SPEED_3_333_E,

    /** for 12.1G */
    CPSS_DXCH_PORT_SERDES_SPEED_12_5_E,

    /** legacy */
    CPSS_DXCH_PORT_SERDES_SPEED_7_5_E,

    /** legacy */
    CPSS_DXCH_PORT_SERDES_SPEED_11_25_E,

    /** for 22G SR_LR */
    CPSS_DXCH_PORT_SERDES_SPEED_11_5625_E,

    /** legacy */
    CPSS_DXCH_PORT_SERDES_SPEED_10_9375_E,

    /** for 11.8G, 23.6G, 47.2G */
    CPSS_DXCH_PORT_SERDES_SPEED_12_1875_E,

    /** for 5.45G in xCat3 */
    CPSS_DXCH_PORT_SERDES_SPEED_5_625_E,

    /** for 12.5G, 25G and 50G port speed */
    CPSS_DXCH_PORT_SERDES_SPEED_12_8906_E,

    /** for 40G R2 */
    CPSS_DXCH_PORT_SERDES_SPEED_20_625_E,

    /** for 25G, 50G, 100G KR4 and SR_LR4 */
    CPSS_DXCH_PORT_SERDES_SPEED_25_78125_E,

    /** for EDSA compensation mode 107G */
    CPSS_DXCH_PORT_SERDES_SPEED_27_5_E,

    /** for EDSA compensation mode 109G */
    CPSS_DXCH_PORT_SERDES_SPEED_28_05_E,

    /** for 102G */
    CPSS_DXCH_PORT_SERDES_SPEED_26_25_E,

    /** Last member, the speed is not determind */
    CPSS_DXCH_PORT_SERDES_SPEED_NA_E

} CPSS_DXCH_PORT_SERDES_SPEED_ENT;

/**
* @enum CPSS_DXCH_PORT_UNITS_ID_ENT
 *
 * @brief Defines Port Units ID.
*/
typedef enum{

    /** GEMAC UNIT */
    CPSS_DXCH_PORT_UNITS_ID_GEMAC_UNIT_E,

    /** XLGMAC UNIT */
    CPSS_DXCH_PORT_UNITS_ID_XLGMAC_UNIT_E,

    /** HGLMAC UNIT */
    CPSS_DXCH_PORT_UNITS_ID_HGLMAC_UNIT_E,

    /** XPCS UNIT */
    CPSS_DXCH_PORT_UNITS_ID_XPCS_UNIT_E,

    /** MMPCS UNIT */
    CPSS_DXCH_PORT_UNITS_ID_MMPCS_UNIT_E,

    /** CG UNIT */
    CPSS_DXCH_PORT_UNITS_ID_CG_UNIT_E,

    /** INTLKN UNIT */
    CPSS_DXCH_PORT_UNITS_ID_INTLKN_UNIT_E,

    /** INTLKN RF UNIT */
    CPSS_DXCH_PORT_UNITS_ID_INTLKN_RF_UNIT_E,

    /** SERDES UNIT */
    CPSS_DXCH_PORT_UNITS_ID_SERDES_UNIT_E,

    /** SERDES PHY UNIT */
    CPSS_DXCH_PORT_UNITS_ID_SERDES_PHY_UNIT_E,

    /** @brief ETI UNIT
     *  CPSS_DXCH_PORT_UNITS_ID_ETI_ILKN_RF_UNIT_E ETI ILKN RF UNIT
     */
    CPSS_DXCH_PORT_UNITS_ID_ETI_UNIT_E,

    CPSS_DXCH_PORT_UNITS_ID_ETI_ILKN_RF_UNIT_E,

    /** D UNIT (DDR3) */
    CPSS_DXCH_PORT_UNITS_ID_D_UNIT_E

} CPSS_DXCH_PORT_UNITS_ID_ENT;


/**
* @enum CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT
 *
 * @brief Defines Phy H Sub Seq.
*/
typedef enum{

    /** SD RESET */
    CPSS_DXCH_PORT_SERDES_SD_RESET_SEQ_E,

    /** SD UNRESET */
    CPSS_DXCH_PORT_SERDES_SD_UNRESET_SEQ_E,

    /** RF RESET */
    CPSS_DXCH_PORT_SERDES_RF_RESET_SEQ_E,

    /** RF UNRESET */
    CPSS_DXCH_PORT_SERDES_RF_UNRESET_SEQ_E,

    /** SYNCE RESET */
    CPSS_DXCH_PORT_SERDES_SYNCE_RESET_SEQ_E,

    /** SYNCE UNRESET */
    CPSS_DXCH_PORT_SERDES_SYNCE_UNRESET_SEQ_E,

    /** SERDES POWER UP CTRL */
    CPSS_DXCH_PORT_SERDES_SERDES_POWER_UP_CTRL_SEQ_E,

    /** SERDES POWER DOWN CTRL */
    CPSS_DXCH_PORT_SERDES_SERDES_POWER_DOWN_CTRL_SEQ_E,

    /** SERDES RXINT UP */
    CPSS_DXCH_PORT_SERDES_SERDES_RXINT_UP_SEQ_E,

    /** SERDES RXINT DOWN */
    CPSS_DXCH_PORT_SERDES_SERDES_RXINT_DOWN_SEQ_E,

    /** SERDES WAIT PLL */
    CPSS_DXCH_PORT_SERDES_SERDES_WAIT_PLL_SEQ_E,

    /** SPEED 1.25G */
    CPSS_DXCH_PORT_SERDES_SPEED_1_25G_SEQ_E,

    /** SPEED 3.125G */
    CPSS_DXCH_PORT_SERDES_SPEED_3_125G_SEQ_E,

    /** SPEED 3.75G */
    CPSS_DXCH_PORT_SERDES_SPEED_3_75G_SEQ_E,

    /** SPEED 4.25G */
    CPSS_DXCH_PORT_SERDES_SPEED_4_25G_SEQ_E,

    /** SPEED 5G */
    CPSS_DXCH_PORT_SERDES_SPEED_5G_SEQ_E,

    /** SPEED 6.25G */
    CPSS_DXCH_PORT_SERDES_SPEED_6_25G_SEQ_E,

    /** SPEED 7.5G */
    CPSS_DXCH_PORT_SERDES_SPEED_7_5G_SEQ_E,

    /** SPEED 10.3125G */
    CPSS_DXCH_PORT_SERDES_SPEED_10_3125G_SEQ_E,

    /** SD LPBK NORMAL */
    CPSS_DXCH_PORT_SERDES_SD_LPBK_NORMAL_SEQ_E,

    /** SD ANA TX 2 RX */
    CPSS_DXCH_PORT_SERDES_SD_ANA_TX_2_RX_SEQ_E,

    /** SD DIG TX 2 RX */
    CPSS_DXCH_PORT_SERDES_SD_DIG_TX_2_RX_SEQ_E,

    /** SD DIG RX 2 TX */
    CPSS_DXCH_PORT_SERDES_SD_DIG_RX_2_TX_SEQ_E,

    /** PT AFTER PATTERN NORMAL */
    CPSS_DXCH_PORT_SERDES_PT_AFTER_PATTERN_NORMAL_SEQ_E,

    /** PT AFTER PATTERN TEST */
    CPSS_DXCH_PORT_SERDES_PT_AFTER_PATTERN_TEST_SEQ_E,

    /** RX TRAINING ENABLE */
    CPSS_DXCH_PORT_SERDES_RX_TRAINING_ENABLE_SEQ_E,

    /** RX TRAINING DISABLE */
    CPSS_DXCH_PORT_SERDES_RX_TRAINING_DISABLE_SEQ_E,

    /** TX TRAINING ENABLE */
    CPSS_DXCH_PORT_SERDES_TX_TRAINING_ENABLE_SEQ_E,

    /** TX TRAINING DISABLE */
    CPSS_DXCH_PORT_SERDES_TX_TRAINING_DISABLE_SEQ_E,

    /** SPEED 12.5G */
    CPSS_DXCH_PORT_SERDES_SPEED_12_5G_SEQ_E,

    /** SPEED 3.3G */
    CPSS_DXCH_PORT_SERDES_SPEED_3_3G_SEQ_E,

    /** SPEED 11.5625G */
    CPSS_DXCH_PORT_SERDES_SPEED_11_5625G_SEQ_E,

    /** SERDES PARTIAL POWER DOWN */
    CPSS_DXCH_PORT_SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ_E,

    /** SERDES PARTIAL POWER UP */
    CPSS_DXCH_PORT_SERDES_SERDES_PARTIAL_POWER_UP_SEQ_E,

    /** SPEED 11.25G */
    CPSS_DXCH_PORT_SERDES_SPEED_11_25G_SEQ_E,

    /** CORE RESET */
    CPSS_DXCH_PORT_SERDES_CORE_RESET_SEQ_E,

    /** CORE UNRESET */
    CPSS_DXCH_PORT_SERDES_CORE_UNRESET_SEQ_E,

    /** FFE TABLE LR */
    CPSS_DXCH_PORT_SERDES_FFE_TABLE_LR_SEQ_E,

    /** FFE TABLE SR */
    CPSS_DXCH_PORT_SERDES_FFE_TABLE_SR_SEQ_E,

    /** 10.9375G */
    CPSS_DXCH_PORT_SERDES_SPEED_10_9375G_SEQ_E,

    /** 12.1875G */
    CPSS_DXCH_PORT_SERDES_SPEED_12_1875G_SEQ_E,

    /** 5.625G SEQ */
    CPSS_DXCH_PORT_SERDES_SPEED_5_625G_SEQ_E,

    /** 5.15625G */
    CPSS_DXCH_PORT_SERDES_SPEED_5_15625G_SEQ_E

} CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT;

/**
* @enum CPSS_DXCH_PORT_EL_DB_OPERATION_ENT
 *
 * @brief Defines EL DB Operation Type.
*/
typedef enum{

    /** WRITE OP */
    CPSS_DXCH_PORT_EL_DB_WRITE_OP_E,

    /** DELAY OP */
    CPSS_DXCH_PORT_EL_DB_DELAY_OP_E,

    /** POLLING OP */
    CPSS_DXCH_PORT_EL_DB_POLLING_OP_E,

    /** DUNIT WRITE OP */
    CPSS_DXCH_PORT_EL_DB_DUNIT_WRITE_OP_E,

    /** DUNIT POLLING OP */
    CPSS_DXCH_PORT_EL_DB_DUNIT_POLLING_OP_E

} CPSS_DXCH_PORT_EL_DB_OPERATION_ENT;

/**
* @enum CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT
 *
 * @brief Defines ctle bias mode per port
*/
typedef enum {
    /** default ctle bias mode */
    CPSS_DXCH_PORT_CTLE_BIAS_NORMAL_E,

    /** for short length calbes */
    CPSS_DXCH_PORT_CTLE_BIAS_HIGH_E

} CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT;

/**
* @struct CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC
 *
 * @brief Port SERDES configuration Element.
*/
typedef struct {
    CPSS_DXCH_PORT_EL_DB_OPERATION_ENT  op;
    GT_U16   indexOffset;
    GT_U32   regOffset;
    GT_U32   data;
    GT_U32   mask;
    GT_U16   waitTime;
    GT_U16   numOfLoops;
    GT_U16   delay; /* msec */
} CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC;


/**
* @struct CPSS_DXCH_PORT_SERDES_CONFIG_STC
 *
 * @brief Port SERDES configuration.
*/
typedef struct{

    /** Tx Driver output amplitude(valid values range 0 */
    GT_U32 txAmp;

    /** Pre */
    GT_BOOL txEmphEn;

    /** GT_FALSE */
    GT_BOOL txEmphType;

    /** Emphasis level control bits(valid values range 0 */
    GT_U32 txEmphAmp;

    /** Transmitter Amplitude Adjust(valid values range 0 */
    GT_U32 txAmpAdj;

    /** FFE signal swing control (0..3). */
    GT_U32 ffeSignalSwingControl;

    /** Feed Forward Equalization (FFE) Resistor Select (0..0x7). */
    GT_U32 ffeResistorSelect;

    /** @brief Feed Forward Equalization (FFE) Capacitor Select (0..0xF).
     *  Comments:
     *  1. Relevant fields for DxCh3: txAmp, txEmphEn, txEmphType, txEmphAmp.
     *  2. Relevant fields for xCat and above: txAmp, txEmphAmp, txAmpAdj, txEmphEn,
     *  ffeSignalSwingControl, ffeResistorSelect, ffeCapacitorSelect.
     *  3. For further information, contact your Marvell representative.
     */
    GT_U32 ffeCapacitorSelect;

} CPSS_DXCH_PORT_SERDES_CONFIG_STC;


/* size of DFE values array */
#define CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS    6

#define CPSS_DXCH_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS    13

/**
* @struct *CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR
 *
 * @brief Port SERDES lanes fine tuning values.
*/
typedef struct{

    /** @brief A digital filter controls the gain according to the previous
     *  received bit (one tap) and compensates for interconnect ISI
     *  and ILD (refer to the DFE_F1 field)
     *  (APPLICABLE DEVICES: xCat, xCat3, xCat2, Lion)
     *  (APPLICABLE RANGES: 0..31)
     */
    GT_U32 dfe;

    /** @brief mainly controls the low frequency gain (refer to the
     *  FFE_res_sel field)(APPLICABLE DEVICES: xCat and above)
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 ffeR;

    /** @brief mainly controls the high frequency gain (refer to the
     *  FFE_cap_sel field)(APPLICABLE DEVICES: xCat and above)
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 ffeC;

    /** @brief sampler (refer to the cal_os_ph_rd field)
     *  (APPLICABLE DEVICES: xCat and above)
     */
    GT_U32 sampler;

    /** @brief Threshold that trips the Squelch detector peak
     *  differential amplitude (refer to the SQ_THRESH field)
     *  (APPLICABLE DEVICES: xCat and above)
     */
    GT_U32 sqlch;

    /** @brief txEmph0: Controls the Emphasis Amplitude for Gen0 bit rates
     *  for Lion2 device [0...15]
     *  for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
     *  for Bobcat3 device: Post-Cursor [0...31]
     */
    GT_32 txEmphAmp;

    /** @brief Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
     *  In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
     *  In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
     *  txAmpAdj- Transmitter Amplitude Adjust [0...15]
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
     */
    GT_U32 txAmp;

    GT_U32 txAmpAdj;

    /** @brief FFE signal swing control (APPLICABLE DEVICES: xCat, xCat3, xCat2, Lion)
     *  (APPLICABLE RANGES: 0..3)
     */
    GT_U32 ffeS;

    /** @brief Pre
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
     */
    GT_BOOL txEmphEn;

    /** @brief Controls the emphasis amplitude for Gen1 bit rates
     *  for Lion2 device [0...15]
     *  for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
     *  for Bobcat3 device: Pre-Cursor [0...31]
     */
    GT_32 txEmph1;

    /** @brief Align 90 Calibration Phase Offset(This is the external
     *  value used in place of the autocalibration value for
     *  rxclkalign90). (APPLICABLE RANGES: 0..127)
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_U32 align90;

    /** @brief Controls the emphasis amplitude (GT_TRUE
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
     */
    GT_BOOL txEmphEn1;

    /** @brief Transmitter Amplitude Shift (GT_TRUE
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
     *  dfeVals - Adapted DFE Coefficient. This field indicates the DFE
     *  auto-calibration and auto-trained results.
     *  (APPLICABLE DEVICES: Lion2)
     */
    GT_BOOL txAmpShft;

    GT_32 dfeValsArray[CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS];

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum) */
    GT_U32 DC;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum) */
    GT_U32 LF;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum) */
    GT_U32 HF;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum) */
    GT_U32 BW;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum) */
    GT_U32 LB;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum) */
    GT_U32 EO;

    GT_32 DFE[CPSS_DXCH_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS];

} CPSS_DXCH_PORT_SERDES_TUNE_STC, *CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR;

/**
* @struct CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC
 *
 * @brief Port SERDES TX configuration parameters.
*/
typedef struct{

    /** @brief Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
     *  In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
     *  In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
     */
    GT_U32 txAmp;

    /** @brief Transmitter Amplitude Adjust: (GT_TRUE
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
     */
    GT_BOOL txAmpAdjEn;

    /** @brief Controls the Emphasis Amplitude for Gen0 bit rates
     *  for Lion2 device [0...15]
     *  for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
     *  for Bobcat3 device: Post-Cursor [0...31]
     */
    GT_32 emph0;

    /** @brief Controls the emphasis amplitude for Gen1 bit rates
     *  for Lion2 device [0...15]
     *  for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
     *  for Bobcat3 device: Pre-Cursor [0...31]
     */
    GT_32 emph1;

    /** @brief Transmitter Amplitude Shift (GT_TRUE
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
     */
    GT_BOOL txAmpShft;

    /** @brief Pre
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
     */
    GT_BOOL txEmphEn;

} CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC;

/**
* @struct CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC
 *
 * @brief Port SERDES TX configuration parameters.
*/
typedef struct{

    GT_U32 portMac;
    GT_32 emph0;
    GT_U32 txAmp;
    GT_32 emph1;

} CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC;

/**
* @struct CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC
 *
 * @brief Port SERDES RX configuration parameters.
*/
typedef struct{

    /** @brief Threshold that trips the Squelch detector peak
     *  differential amplitude (refer to the SQ_THRESH field)
     *  (APPLICABLE RANGES: 0..31).
     *  (APPLICABLE DEVICES: Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X) (APPLICABLE RANGES: 0..310).
     */
    GT_U32 sqlch;

    /** @brief mainly controls the low frequency gain (refer to the
     *  FFE_res_sel field) (APPLICABLE RANGES: 0..7)
     *  (APPLICABLE DEVICES: Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X) (APPLICABLE RANGES: 0..15).
     */
    GT_U32 ffeRes;

    /** @brief mainly controls the high frequency gain (refer to the
     *  FFE_cap_sel field)(APPLICABLE RANGES: 0..15)
     */
    GT_U32 ffeCap;

    /** @brief Align 90 Calibration Phase Offset(This is the external
     *  value used in place of the autocalibration value for
     *  rxclkalign90). (APPLICABLE RANGES: 0..127)
     */
    GT_U32 align90;

    /** @brief DC
     *  (APPLICABLE DEVICES: Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X)
     */
    GT_U32 dcGain;

    /** @brief CTLE Band
     *  (APPLICABLE DEVICES: Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X)
     */
    GT_U32 bandWidth;

    /** @brief CTLE Loop Band
     *  (APPLICABLE DEVICES: Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X)
     */
    GT_U32 loopBandwidth;

} CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC;

/**
* @struct CPSS_DXCH_PORT_EOM_MATRIX_STC
 *
 * @brief A struct containing two 32x128 matrices representing
 * the values received from the EOM feature.
*/
typedef struct{

    GT_U32 upperMatrix[CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS]
                         [CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS];
    GT_U32 lowerMatrix[CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS]
                         [CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS];

} CPSS_DXCH_PORT_EOM_MATRIX_STC;


/**
* @struct CPSS_DXCH_PORT_ALIGN90_PARAMS_STC
*/
typedef struct{

    GT_U32 startAlign90;

    /** @brief osDeltaMax;
     *  adaptedFfeR;
     *  adaptedFfeC;
     *  Comments:
     *  None
     */
    GT_U32 rxTrainingCfg;

    GT_U32 osDeltaMax;

    GT_U32 adaptedFfeR;

    GT_U32 adaptedFfeC;

} CPSS_DXCH_PORT_ALIGN90_PARAMS_STC;

/**
* @struct CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC
 *
 * @brief A struct containing parameters to build
 * <TX Config Reg> data for Auto-Negotiation.
*/
typedef struct{

    /** port Link Up if GT_TRUE, Link Down if GT_FALSE; */
    GT_BOOL link;

    /** port speed; */
    CPSS_PORT_SPEED_ENT speed;

    /** @brief port duplex mode;
     *  Comments:
     *  None
     */
    CPSS_PORT_DUPLEX_ENT duplex;

} CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC;

/**
* @struct CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC
 *
 * @brief A struct containing the results for Serdes Eye Monitoring
 * For Caelum, Aldrin, AC3X, Bobcat3.
*/
typedef struct{

    GT_CHAR *matrixPtr;

    /** number of columns in matrix */
    GT_U32 x_points;

    /** @brief number of rows in matrix
     *  vbtcPtr     - vbtc text result
     *  hbtcPtr     - hbtc text result
     */
    GT_U32 y_points;

    GT_CHAR *vbtcPtr;

    GT_CHAR *hbtcPtr;

    /** height in mV */
    GT_U32 height_mv;

    /** width in m_UI */
    GT_U32 width_mui;

    /** @brief global serdes lane number in device
     *  Comments:
     *  None
     */
    GT_U32 globalSerdesNum;

} CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC;

/**
* @struct CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC
 *
 * @brief A struct containing the input parameters for Serdes Eye Monitoring
 * For Caelum, Aldrin, AC3X, Bobcat3.
*/
typedef struct{

    /** @brief minimum dwell bits
     *  valid range: 100000 --- 100000000 .If 0 - transform to default 100000.
     */
    GT_U32 min_dwell_bits;

    /** @brief maximum dwell bits
     *  valid range: 100000 --- 100000000 .If 0 - transform to default 100000000.
     *  Comments:
     *  should be min_dwell_bits <= max_dwell_bits
     */
    GT_U32 max_dwell_bits;

} CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC;


/**
* @enum CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT
 *
 * @brief Periodic Flow Control Port Type speed enumeration
*/
typedef enum{

    /** GIG ports */
    CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E,

    /** XG ports */
    CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E

} CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT;

/**
* @enum CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT
 *
 * @brief Enumeration of port's serdes auto tuning modes.
*/
typedef enum
{
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_WAIT_FOR_FINISH_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_WAIT_FOR_FINISH_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E,

    /* Last */
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_LAST_E =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E,

}CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT;

/**
* @enum CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT
 *
 * @brief Enumeration of port's serdes auto tuning modes.
*/
typedef enum{

    /** serdes auto-tuning (rx/tx) succeeded */
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E =

    CPSS_PORT_SERDES_AUTO_TUNE_PASS_E,

    /** serdes auto-tuning (rx/tx) failed */
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_FAIL_E =

    CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E,

    /** @brief serdes auto-tuning (rx/tx)
     *  still in process
     */
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E =

    CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E

} CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT;

/**
* @enum CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT
 *
 * @brief Enumeration of port's serdes loopback modes.
*/
typedef enum
{
    CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E =
        CPSS_PORT_SERDES_LOOPBACK_DISABLE_E,
    CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E =
        CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E,
    CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E =
        CPSS_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E,
    CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E =
        CPSS_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E,
    CPSS_DXCH_PORT_SERDES_LOOPBACK_MAX_E =
        CPSS_PORT_SERDES_LOOPBACK_MAX_E

}CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT;

/**
* @enum CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT
 *
 * @brief Enumeration of port's PCS loopback modes.
*/
typedef enum{

    /** no any loopback on PCS */
    CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E =

    CPSS_PORT_PCS_LOOPBACK_DISABLE_E,

    /** TX to RX loopback */
    CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E =

    CPSS_PORT_PCS_LOOPBACK_TX2RX_E,

    /** RX to TX loopback */
    CPSS_DXCH_PORT_PCS_LOOPBACK_RX2TX_E =

    CPSS_PORT_PCS_LOOPBACK_RX2TX_E

} CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT;


/**
* @enum CPSS_DXCH_PORT_FEC_MODE_ENT
 *
 * @brief Enumeration of port's Forward Error Correction modes.
*/
typedef enum
{
    CPSS_DXCH_PORT_FEC_MODE_ENABLED_E =
        CPSS_PORT_FEC_MODE_ENABLED_E,
    CPSS_DXCH_PORT_FEC_MODE_DISABLED_E =
        CPSS_PORT_FEC_MODE_DISABLED_E,
    CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E =
        CPSS_PORT_RS_FEC_MODE_ENABLED_E,
    CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E =
        CPSS_PORT_BOTH_FEC_MODE_ENABLED_E,
    CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E =
        CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E,
    /* Last */
    CPSS_DXCH_PORT_FEC_MODE_LAST_E =
        CPSS_PORT_FEC_MODE_LAST_E

}CPSS_DXCH_PORT_FEC_MODE_ENT;



/*-----------------------------------------------*/
/*  Pizza Arbiter Unit Description               */
/*-----------------------------------------------*/
/**
* @enum CPSS_DXCH_PA_UNIT_ENT
 *
 * @brief Enumeration of port's Forward Error Correction modes.
*/
typedef enum
{
     CPSS_DXCH_PA_UNIT_UNDEFINED_E = -1
    /* RxDMA Units */
    ,CPSS_DXCH_PA_UNIT_RXDMA_0_E = 0        /*   0  BC2 BobK-Caelum              Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_1_E            /*   1      BobK-Caelum  BobK-Cetus  Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_2_E            /*   2                               Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_3_E            /*   3                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_4_E            /*   4                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_RXDMA_5_E            /*   5                                       BC3 */
    /* TXQ units */
    ,CPSS_DXCH_PA_UNIT_TXQ_0_E              /*   6  BC2 BobK-Caelum  BobK-Cetus  Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_TXQ_1_E              /*   7                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_TXQ_2_E              /*   8                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_TXQ_3_E              /*   9                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_TXQ_4_E              /*  10                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_TXQ_5_E              /*  11                                       BC3 */
    /* TxDMA Units */
    ,CPSS_DXCH_PA_UNIT_TXDMA_0_E            /*  12  BC2 BobK-Caelum              Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_TXDMA_1_E            /*  13      BobK-Caelum  BobK-Cetus  Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_TXDMA_2_E            /*  14                               Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_TXDMA_3_E            /*  15                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_TXDMA_4_E            /*  16                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_TXDMA_5_E            /*  17                                       BC3 */

    /* TxFifo Units */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_0_E          /*  18  BC2 BobK-Caelum              Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_1_E          /*  19      BobK-Caelum  BobK-Cetus  Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_2_E          /*  20                               Aldrin  BC3 */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_3_E          /*  21                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_4_E          /*  22                                       BC3 */
    ,CPSS_DXCH_PA_UNIT_TX_FIFO_5_E          /*  23                                       BC3 */

    /* Eth-TxFifo units */
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E      /*  24 BC2 BobK-Caelum                           */
    ,CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E      /*  25      BobK-Caelum  BobK-Cetus              */
    /* Ilkn-Tx-Fifo units */
    ,CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E       /*  26  BC2                                      */
    /* global (device level)  units */
    ,CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E         /*  27      BobK-Caelum  BobK-Cetus  Aldrin      */
    ,CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E         /*  28      BobK-Caelum  BobK-Cetus  Aldrin      */
    ,CPSS_DXCH_PA_UNIT_MPPM_E               /*  29      BobK-Caelum  BobK-Cetus              */
    ,CPSS_DXCH_PA_UNIT_MAX_E                /* never cross this boundary */
}CPSS_DXCH_PA_UNIT_ENT;


/**
* @enum CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_ENT
 *
 * @brief Enumeration of port's SERDES CPLL input clock.
*/
typedef enum{

    /** input CPLL frequency = 25Mhz */
    CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_25M_E,

    /** input CPLL frequency = 156Mhz */
    CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_156M_E

} CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_ENT;


/**
* @enum CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_ENT
 *
 * @brief Enumeration of port's SERDES CPLL output clock.
*/
typedef enum{

    /** output CPLL frequency = 78Mhz */
    CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_78M_E,

    /** output CPLL frequency = 156Mhz */
    CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_156M_E,

    /** output CPLL frequency = 200Mhz */
    CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_200M_E,

    /** output CPLL frequency = 312.5Mhz */
    CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_312M_E,

    /** output CPLL frequency = 160.9Mhz */
    CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_161M_E,

    /** output CPLL frequency = 164.2Mhz */
    CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_164M_E

} CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_ENT;

/**
* @internal cpssDxChPortCpllCfgInit function
* @endinternal
*
* @brief   CPLL unit initialization routine.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] cpllNum                  - CPLL unit index
*                                      (APPLICABLE RANGES: Caelum            0..2;
*                                      Aldrin, AC3X, Bobcat3; Aldrin2   0)
* @param[in] inputFreq                - CPLL input frequency
* @param[in] outputFreq               - CPLL output frequency
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note API should be called before any port with CPLL as reference clock
*       was created.
*
*/
GT_STATUS cpssDxChPortCpllCfgInit
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       cpllNum,
    IN CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_ENT      inputFreq,
    IN CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_ENT     outputFreq
);

/**
* @internal cpssDxChPortMacSaLsbSet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macSaLsb                 - The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port. The upper 40 bits
*       are configured by cpssDxChPortMacSaBaseSet.
*
*/
GT_STATUS cpssDxChPortMacSaLsbSet
(
    IN GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8    macSaLsb
);

/**
* @internal cpssDxChPortMacSaLsbGet function
* @endinternal
*
* @brief   Get the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macSaLsbPtr              - (pointer to) The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.The upper 40 bits
*       are configured by cpssDxChPortMacSaBaseSet.
*
*/
GT_STATUS cpssDxChPortMacSaLsbGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8    *macSaLsbPtr
);

/**
* @internal cpssDxChPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortEnableGet
(
    IN   GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL   *statePtr
);

/**
* @internal cpssDxChPortDuplexModeSet function
* @endinternal
*
* @brief   Set the port to enable and the port mode to half- or full-duplex mode.
*         When port is disabled it does not transmit pending packets and
*         drops packets received from the line.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] dMode                    - 0 = The port operates in half-duplex mode.
*                                      1 = The port operates in full-duplex mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or dMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported duplex mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The <SetFullDx> bit is meaningless when <ANDuplex> is set.
*       For CPU port, this field must be set to reflect this port Duplex mode.
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortDuplexModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
);

/**
* @internal cpssDxChPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] dModePtr                 - duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortDuplexModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
);

/**
* @internal cpssDxChPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*                                       (For DxCh3, not XG ports only.)
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssDxChPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*       4.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/**
* @internal cpssDxChPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);


/**
* @internal cpssDxChPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssDxChPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the duplex auto negotiation
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortDuplexAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);
/**
* @internal cpssDxChPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssDxChPortDuplexModeSet
*         The function does not configure the PHY connected to the port. It
*         configures only the MAC layer of the port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. CPU port not supports the duplex auto negotiation
*       2. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortDuplexAutoNegEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *statePtr
);

/**
* @internal cpssDxChPortFlowCntrlAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for Flow Control on
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[in] pauseAdvertise           - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the flow control auto negotiation
*       2. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortFlowCntrlAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state,
    IN  GT_BOOL   pauseAdvertise
);
/**
* @internal cpssDxChPortFlowCntrlAutoNegEnableGet function
* @endinternal
*
* @brief   Get Auto-Negotiation enable/disable state for Flow Control per port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 -   GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[out] pauseAdvertisePtr        - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
GT_STATUS cpssDxChPortFlowCntrlAutoNegEnableGet
(
    IN   GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL     *statePtr,
    OUT  GT_BOOL     *pauseAdvertisePtr
);


/**
* @internal cpssDxChPortSpeedAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation of interface speed on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2.CPU port not supports the speed auto negotiation
*
*/
GT_STATUS cpssDxChPortSpeedAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/**
* @internal cpssDxChPortSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of Auto-Negotiation enable on specified port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enabled Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. CPU port not supports the speed auto negotiation
*
*/
GT_STATUS cpssDxChPortSpeedAutoNegEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL  *statePtr
);

/**
* @internal cpssDxChPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed and FE ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       Before calling cpssDxChPortFlowControlEnableSet,
*       cpssDxChPortPeriodicFlowControlCounterSet should be called
*       to set the interval between the transmission of two consecutive
*       Flow Control packets according to port speed
*
*/
GT_STATUS cpssDxChPortFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
);

/**
* @internal cpssDxChPortFlowControlEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x Flow Control on specific logical port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - Pointer to Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortFlowControlEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr
);


/**
* @internal cpssDxChPortPeriodicFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmits of periodic 802.3x flow control.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - periodic 802.3x flow control tramsition state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on unsupported request
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*       Note: In Bobcat2, Caelum, Bobcat3, Aldrin, AC3X CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E
*       option is not supported.
*
*/
GT_STATUS cpssDxChPortPeriodicFcEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   enable
);

/**
* @internal cpssDxChPortPeriodicFcEnableGet function
* @endinternal
*
* @brief   Get status of periodic 802.3x flow control transmition.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the periodic 802.3x flow control
*                                      tramsition state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*/
GT_STATUS cpssDxChPortPeriodicFcEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   *enablePtr
);

/**
* @internal cpssDxChPortBackPressureEnableSet function
* @endinternal
*
* @brief   Enable/disable of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for enable Back Pressure, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if feature not supported by port
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
GT_STATUS cpssDxChPortBackPressureEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/**
* @internal cpssDxChPortBackPressureEnableGet function
* @endinternal
*
* @brief   Gets the state of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to back pressure enable/disable state:
*                                      - GT_TRUE to enable Back Pressure
*                                      - GT_FALSE to disable Back Pressure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
GT_STATUS cpssDxChPortBackPressureEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChPortLinkStatusGet function
* @endinternal
*
* @brief   Gets Link Status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] isLinkUpPtr              - GT_TRUE for link up, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortLinkStatusGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *isLinkUpPtr
);

/**
* @internal cpssDxChPortInterfaceModeSet function
* @endinternal
*
* @brief   Sets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - Interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Depends on capability of the specific device. The port's speed
*       should be set after configuring Interface Mode.
*
*/
GT_STATUS cpssDxChPortInterfaceModeSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
);


/**
* @internal cpssDxChPortInterfaceModeGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
GT_STATUS cpssDxChPortInterfaceModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

/**
* @internal cpssDxChPortForceLinkPassEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortForceLinkPassEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/**
* @internal cpssDxChPortForceLinkPassEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - (ptr to) current force link pass state:
*                                      GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortForceLinkPassEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *statePtr
);

/**
* @internal cpssDxChPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortForceLinkDownEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/**
* @internal cpssDxChPortForceLinkDownEnableGet function
* @endinternal
*
* @brief   Get Force Link Down status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - (ponter to) current force link down status:
*                                      GT_TRUE - force link down, GT_FALSE - otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortForceLinkDownEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *statePtr
);

/**
* @internal cpssDxChPortMruSet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] mruSize                  - max receive packet size in bytes.
*                                      (APPLICABLE RANGES: 0..10304).
*                                      max : jumbo frame(10K) + 64 (including 4 bytes CRC).
*                                      Value must be even.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32 mruSize
);


/**
* @internal cpssDxChPortMruGet function
* @endinternal
*
* @brief   Gets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes. 10K+64 (including 4 bytes CRC)
*                                      value must be even
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruGet
(
    IN   GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32 *mruSizePtr
);

/**
* @internal cpssDxChPortMruProfileSet function
* @endinternal
*
* @brief   Set an MRU profile for port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*                                      In eArch devices portNum is default ePort.
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruProfileSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_U32           profileId
);

/**
* @internal cpssDxChPortMruProfileGet function
* @endinternal
*
* @brief   Get an MRU profile for port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*                                      In eArch devices portNum is default ePort.
*
* @param[out] profileIdPtr             - (pointer to) the profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruProfileGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *profileIdPtr
);

/**
* @internal cpssDxChPortProfileMruSizeSet function
* @endinternal
*
* @brief   Set an MRU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..7)
* @param[in] mruSize                  - max receive packet size in bytes
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, profile or MRU size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortProfileMruSizeSet
(
    IN GT_U8    devNum,
    IN GT_U32   profile,
    IN GT_U32   mruSize
);

/**
* @internal cpssDxChPortProfileMruSizeGet function
* @endinternal
*
* @brief   Get an MRU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..7)
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortProfileMruSizeGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  profile,
    OUT GT_U32  *mruSizePtr
);

/**
* @internal cpssDxChPortMruExceptionCommandSet function
* @endinternal
*
* @brief   Set the command assigned to frames that exceed the default ePort MRU
*         size
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruExceptionCommandSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
);

/**
* @internal cpssDxChPortMruExceptionCommandGet function
* @endinternal
*
* @brief   Get the command assigned to frames that exceed the default ePort MRU
*         size
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruExceptionCommandGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
);

/**
* @internal cpssDxChPortMruExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/drop code assigned to a frame which fails the MRU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU/drop code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or cpu/drop code
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruExceptionCpuCodeSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
);

/**
* @internal cpssDxChPortMruExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get the CPU/drop code assigned to a frame which fails the MRU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodePtr               - the CPU/drop code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMruExceptionCpuCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
);

/**
* @internal cpssDxChPortCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable 32-bit the CRC checking.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - If GT_TRUE,  CRC checking
*                                      If GT_FALSE, disable CRC checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortCrcCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChPortCrcCheckEnableGet function
* @endinternal
*
* @brief   Get CRC checking (Enable/Disable) state for received packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the CRS checking state :
*                                      GT_TRUE  - CRC checking is enable,
*                                      GT_FALSE - CRC checking is disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortCrcCheckEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChPortXGmiiModeSet function
* @endinternal
*
* @brief   Sets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortXGmiiModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
);

/**
* @internal cpssDxChPortXGmiiModeGet function
* @endinternal
*
* @brief   Gets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - Pointer to XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortXGmiiModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT    *modePtr
);

/**
* @internal cpssDxChPortIpgBaseSet function
* @endinternal
*
* @brief   Sets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ipgBase                  - IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or ipgBase
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
GT_STATUS cpssDxChPortIpgBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
);

/**
* @internal cpssDxChPortIpgBaseGet function
* @endinternal
*
* @brief   Gets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] ipgBasePtr               - pointer to IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
GT_STATUS cpssDxChPortIpgBaseGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBasePtr
);

/**
* @internal cpssDxChPortIpgSet function
* @endinternal
*
* @brief   Sets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*         Using this API may be required to enable wire-speed in traffic paths
*         that include cascading ports, where it may not be feasible to reduce
*         the preamble length.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] ipg                      - IPG in bytes, acceptable range:
*                                      (APPLICABLE RANGES: 0..511) (APPLICABLE DEVICES DxCh3 and above)
*                                      (APPLICABLE RANGES: 0..15) (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2.)
*                                      Default HW value is 12 bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_OUT_OF_RANGE          - ipg value out of range
* @retval GT_BAD_PARAM             - on bad parameter
*/
GT_STATUS cpssDxChPortIpgSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  ipg
);

/**
* @internal cpssDxChPortIpgGet function
* @endinternal
*
* @brief   Gets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] ipgPtr                   - (pointer to) IPG value in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChPortIpgGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32  *ipgPtr
);

/**
* @internal cpssDxChPortExtraIpgSet function
* @endinternal
*
* @brief   Sets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] number                   -   of words
*                                      (APPLICABLE RANGES: 0..127) (APPLICABLE DEVICES DxCh3 and above)
*                                      (APPLICABLE RANGES: 0..3) (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2.)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortExtraIpgSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8       number
);

/**
* @internal cpssDxChPortExtraIpgGet function
* @endinternal
*
* @brief   Gets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                -  pointer to number of words
*                                      (APPLICABLE RANGES: 0..127) (APPLICABLE DEVICES DxCh3 and above)
*                                      (APPLICABLE RANGES: 0..3) (APPLICABLE DEVICES DxCh1; DxCh1_Diamond; DxCh2.)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortExtraIpgGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8       *numberPtr
);

/**
* @internal cpssDxChPortXgmiiLocalFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected local
*         fault messages.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLocalFaultPtr          - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortXgmiiLocalFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isLocalFaultPtr
);

/**
* @internal cpssDxChPortXgmiiRemoteFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected remote
*         fault messages.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr         - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
);

/**
* @internal cpssDxChPortRemoteFaultSet function
* @endinternal
*
* @brief  Configure the port to start or stop sending fault
*         signals to partner. When port is configured to send,
*         link on both sides will be down.
*         Note: tx_rem_fault takes precedence over
*         tx_loc_fault. for CG mac, know that using tx_rem_fault
*         will cancel tx_loc_fault effect.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum;
*         Bobcat3; Aldrin2; Falcon; Aldrin
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - start sending fault signals to peer (or stop)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRemoteFaultSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         send
);

/**
* @internal cpssDxChPortMacStatusGet function
* @endinternal
*
* @brief   Reads bits that indicate different problems on specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacStatusPtr         - info about port MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacStatusGet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
);

/**
* @internal cpssDxChPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Set the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For port 25 in DX269 (XG/HX port) it's the application responsibility to
*       reconfigure the loopback mode after switching XG/HX mode.
*
*/
GT_STATUS cpssDxChPortInternalLoopbackEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChPortInternalLoopbackEnableGet function
* @endinternal
*
* @brief   Get Internal Loopback
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - Pointer to the Loopback state.
*                                      If GT_TRUE, loopback is enabled
*                                      If GT_FALSE, loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not relevant for the CPU port.
*
*/
GT_STATUS cpssDxChPortInternalLoopbackEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);


/**
* @internal cpssDxChPortInbandAutoNegEnableSet function
* @endinternal
*
* @brief   Configure Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       The change of the Auto-Negotiation causes temporary change of the link
*       to down and up for ports with link up.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssDxChPortInbandAutoNegEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL  enable
);

/**
* @internal cpssDxChPortInbandAutoNegEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssDxChPortInbandAutoNegEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortAttributesOnPortGet function
* @endinternal
*
* @brief   Gets port attributes for particular logical port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
*
* @param[out] portAttributSetArrayPtr  - Pointer to attributes values array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortAttributesOnPortGet
(
    IN    GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
);

/**
* @internal cpssDxChPortPreambleLengthSet function
* @endinternal
*
* @brief   Set the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction and "both directions"
*                                      options (GE ports support only Tx direction)
* @param[in] length                   -  of preamble in bytes
*                                      support only values of 4,8
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
);

/**
* @internal cpssDxChPortPreambleLengthGet function
* @endinternal
*
* @brief   Get the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction
*                                      GE ports support only Tx direction.
*
* @param[out] lengthPtr                - pointer to preamble length in bytes :
*                                      supported length values are : 4,8.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortPreambleLengthGet(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_U32                   *lengthPtr
);

/**
* @internal cpssDxChPortMacSaBaseSet function
* @endinternal
*
* @brief   Sets the base part(40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device. In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
GT_STATUS cpssDxChPortMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
);

/**
* @internal cpssDxChPortMacSaBaseGet function
* @endinternal
*
* @brief   Gets the base part (40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device.In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
GT_STATUS cpssDxChPortMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
);

/**
* @internal cpssDxChPortSerdesConfigGet function
* @endinternal
*
* @brief   Get SERDES configuration per port.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] serdesCfgPtr             - pointer to port serdes configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
GT_STATUS cpssDxChPortSerdesConfigGet
(
    IN GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_SERDES_CONFIG_STC  *serdesCfgPtr
);

/**
* @internal cpssDxChPortPaddingEnableSet function
* @endinternal
*
* @brief   Enable/Disable padding of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortPaddingEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
);

/**
* @internal cpssDxChPortPaddingEnableGet function
* @endinternal
*
* @brief   Gets padding status of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to packet padding status.
*                                      - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortPaddingEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChPortExcessiveCollisionDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note The setting is not relevant in full duplex mode
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS cpssDxChPortExcessiveCollisionDropEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
);

/**
* @internal cpssDxChPortExcessiveCollisionDropEnableGet function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to status of excessive collision packets drop.
*                                      - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by
*                                      device until it is transmitted
*                                      without collisions, regardless of the number
*                                      of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note 1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*       2. Not relevant in full duplex mode
*
*/
GT_STATUS cpssDxChPortExcessiveCollisionDropEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChPortXgLanesSwapEnableSet function
* @endinternal
*
* @brief   Enable/Disable swapping XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Swap Lanes by follows:
*                                      The lanes are swapped by follows:
*                                      Lane 0 PSC Tx data is connected to SERDES Lane 3
*                                      Lane 1 PSC Tx data is connected to SERDES Lane 2
*                                      Lane 2 PSC Tx data is connected to SERDES Lane 1
*                                      Lane 3 PSC Tx data is connected to SERDES Lane 0
*                                      - GT_FALSE - Normal operation (no swapping)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
GT_STATUS cpssDxChPortXgLanesSwapEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
);

/**
* @internal cpssDxChPortXgLanesSwapEnableGet function
* @endinternal
*
* @brief   Gets status of swapping XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to status of swapping XAUI PHY SERDES Lanes.
*                                      - GT_TRUE  - Swap Lanes by follows:
*                                      The lanes are swapped by follows:
*                                      Lane 0 PSC Tx data is connected to SERDES Lane 3
*                                      Lane 1 PSC Tx data is connected to SERDES Lane 2
*                                      Lane 2 PSC Tx data is connected to SERDES Lane 1
*                                      Lane 3 PSC Tx data is connected to SERDES Lane 0
*                                      - GT_FALSE - Normal operation (no swapping)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
GT_STATUS cpssDxChPortXgLanesSwapEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChPortXgPscLanesSwapSet function
* @endinternal
*
* @brief   Set swapping configuration of XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Rx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
* @param[in] txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Tx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - wrong SERDES lane
* @retval GT_BAD_VALUE             - multiple connection detected
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for 4 PCS lanes for XAUI or HyperG.Stack port modes,
*       2 PCS lanes for RXAUI mode
*
*/
GT_STATUS cpssDxChPortXgPscLanesSwapSet
(
    IN GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS],
    IN GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS]
);

/**
* @internal cpssDxChPortXgPscLanesSwapGet function
* @endinternal
*
* @brief   Get swapping configuration of XAUI or HyperG.Stack port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Rx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
* @param[out] txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Tx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported for XAUI or HyperG.Stack ports only.
*
*/
GT_STATUS cpssDxChPortXgPscLanesSwapGet
(
    IN GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS],
    OUT GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS]
);

/**
* @internal cpssDxChPortInBandAutoNegBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable Auto-Negotiation by pass.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssDxChPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssDxChPortInBandAutoNegBypassEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
);

/**
* @internal cpssDxChPortInBandAutoNegBypassEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation by pass status.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to Auto-Negotiation by pass status.
*                                      - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssDxChPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssDxChPortInBandAutoNegBypassEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChPortMacErrorIndicationPortSet function
* @endinternal
*
* @brief   Sets port monitored for MAC errors.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port monitored for MAC errors.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note MAC source addresses and the error type can be read
*       for the port. See cpssDxChPortMacErrorIndicationGet.
*
*/
GT_STATUS cpssDxChPortMacErrorIndicationPortSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal cpssDxChPortMacErrorIndicationPortGet function
* @endinternal
*
* @brief   Gets port monitored for MAC errors.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] portNumPtr               - pointer to port monitored for MAC errors.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note MAC source addresses and the error type can be read
*       for the port. See cpssDxChPortMacErrorIndicationGet.
*
*/
GT_STATUS cpssDxChPortMacErrorIndicationPortGet
(
    IN  GT_U8      devNum,
    OUT GT_PHYSICAL_PORT_NUM      *portNumPtr
);

/**
* @internal cpssDxChPortMacErrorIndicationGet function
* @endinternal
*
* @brief   Gets MAC source addresses and the error type for the monitored port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] typePtr                  - pointer to MAC error type.
* @param[out] macPtr                   - pointer to MAC SA of the last packet received
*                                      with  a MAC error on MAC error indication port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Port monitored for MAC errors should be set.
*       See cpssDxChPortMacErrorIndicationPortSet.
*
*/
GT_STATUS cpssDxChPortMacErrorIndicationGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PORT_MAC_ERROR_TYPE_ENT   *typePtr,
    OUT GT_ETHERADDR                        *macPtr
);

/**
* @internal cpssDxChPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes according to port capabilities.
*         XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*         HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - may be either CPSS_PORT_DIRECTION_RX_E,
*                                      CPSS_PORT_DIRECTION_TX_E, or
*                                      CPSS_PORT_DIRECTION_BOTH_E.
* @param[in] lanesBmp                 - bitmap of SERDES lanes (bit 0-> lane 0, etc.)
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note DxCh3 and above devices supports only CPSS_PORT_DIRECTION_BOTH_E.
*
*/
GT_STATUS cpssDxChPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
);

/**
* @internal cpssDxChPortGroupSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes per group
*         according to port capabilities.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portSerdesGroup          - port group number
*                                      DxCh3 Giga/2.5 G, xCat GE devices:
*                                      Ports       |    SERDES Group
*                                      0..3        |      0
*                                      4..7        |      1
*                                      8..11       |      2
*                                      12..15      |      3
*                                      16..19      |      4
*                                      20..23      |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      DxCh3 XG devices:
*                                      0           |      0
*                                      4           |      1
*                                      10          |      2
*                                      12          |      3
*                                      16          |      4
*                                      22          |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      xCat FE devices
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      Lion devices: Port == SERDES Group
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, portSerdesGroup
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get SERDES port group according to port by calling to
*       cpssDxChPortSerdesGroupGet.
*
*/
GT_STATUS cpssDxChPortGroupSerdesPowerStatusSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     portSerdesGroup,
    IN  GT_BOOL    powerUp
);

/**
* @internal cpssDxChPortSerdesGroupGet function
* @endinternal
*
* @brief   Get SERDES port group that may be used in per SERDES group APIs.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portSerdesGroupPtr       - Pointer to port group number
*                                      DxCh3 Giga/2.5 G, xCat GE devices:
*                                      Ports       |    SERDES Group
*                                      0..3        |      0
*                                      4..7        |      1
*                                      8..11       |      2
*                                      12..15      |      3
*                                      16..19      |      4
*                                      20..23      |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      DxCh3 XG devices:
*                                      0           |      0
*                                      4           |      1
*                                      10          |      2
*                                      12          |      3
*                                      16          |      4
*                                      22          |      5
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      xCat FE devices
*                                      24          |      6
*                                      25          |      7
*                                      26          |      8
*                                      27          |      9
*                                      Lion devices: Port == SERDES Group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesGroupGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32   *portSerdesGroupPtr
);


/**
* @internal cpssDxChPortSerdesCalibrationStartSet function
* @endinternal
*
* @brief   Start calibration on all SERDES of given port.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2;
*         Falcon; Aldrin; AC3X; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSerdesCalibrationStartSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum
);

/**
* @internal cpssDxChPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] fcMode                   - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Lion and above:
*       This function also configures insertion of DSA tag for PFC frames.
*
*/
GT_STATUS cpssDxChPortFlowControlModeSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
);

/**
* @internal cpssDxChPortFlowControlModeGet function
* @endinternal
*
* @brief   Gets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
*
* @param[out] fcModePtr                - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFlowControlModeGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT   *fcModePtr
);

/**
* @internal cpssDxChPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/**
* @internal cpssDxChPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/**
* @internal cpssDxChPortModeSpeedAutoDetectAndConfig function
* @endinternal
*
* @brief   Autodetect and configure interface mode and speed of a given port
*         If the process succeeded the port gets configuration ready for link,
*         otherwise (none of provided by application options valid) the port
*         is left in reset mode.
*
* @note   APPLICABLE DEVICES:      Lion; xCat; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat3; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port not supported, FE ports not supported)
* @param[in] portModeSpeedOptionsArrayPtr  array of port mode and speed options
*                                      preferred for application,
*                                      NULL  if any of supported options acceptable
* @param[in] optionsArrayLen           length of options array (must be 0 if
* @param[in] portModeSpeedOptionsArrayPtr == NULL)
*
* @param[out] currentModePtr            if succeeded ifMode and speed that were configured
*                                      on the port,
*                                      otherwise previous ifMode and speed restored
*
* @retval GT_OK                    - if process completed without errors (no matter if mode
*                                       configured or not)
* @retval GT_BAD_PARAM             - on wrong port number or device,
*                                       optionsArrayLen more then possible options
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_NOT_SUPPORTED      ifMode/speed pair not supported,
*                                       wrong port type
* @retval GT_HW_ERROR              - HW error
*
* @note For list of supported modes see:
*       lionDefaultPortModeSpeedOptionsArray,
*       xcatDefaultNetworkPortModeSpeedOptionsArray,
*       xcatDefaultStackPortModeSpeedOptionsArray
*       There is no possibility to distinguish if partner has SGMII or
*       1000BaseX, when auto-negotiation disabled. Preference to 1000BaseX given
*       by algorithm.
*       Responsibility of application to mask link change interrupt during
*       auto-detection algorithm run.
*
*/
GT_STATUS cpssDxChPortModeSpeedAutoDetectAndConfig
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   CPSS_PORT_MODE_SPEED_STC *portModeSpeedOptionsArrayPtr,
    IN   GT_U8                    optionsArrayLen,
    OUT  CPSS_PORT_MODE_SPEED_STC *currentModePtr
);

/**
* @internal cpssDxChPortForward802_3xEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of 802.3x Flow Control frames to the ingress
*         pipeline of a specified port. Processing of 802.3x Flow Control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward 802.3x frames to the ingress pipe,
*                                      GT_FALSE: do not forward 802.3x frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the ports packet transmission if it is an XOFF packet, or to
*       resume the ports packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packets Length/EtherType field is 88-08
*       - Packets OpCode field is 00-01
*       - Packets MAC DA is 01-80-C2-00-00-01 or the ports configured MAC Address
*
*/
GT_STATUS cpssDxChPortForward802_3xEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChPortForward802_3xEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x frames forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of 802.3x frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the ports packet transmission if it is an XOFF packet, or to
*       resume the ports packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packets Length/EtherType field is 88-08
*       - Packets OpCode field is 00-01
*       - Packets MAC DA is 01-80-C2-00-00-01 or the ports configured MAC Address
*
*/
GT_STATUS cpssDxChPortForward802_3xEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortForwardUnknownMacControlFramesEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of unknown MAC control frames to the ingress
*         pipeline of a specified port. Processing of unknown MAC control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward unknown MAC control frames to the ingress pipe,
*                                      GT_FALSE: do not forward unknown MAC control frames to
*                                      the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packets Length/EtherType field is 88-08
*       - Packets OpCode field is not 00-01 and not 01-01
*       OR
*       Packets MAC DA is not 01-80-C2-00-00-01 and not the ports configured
*       MAC Address
*
*/
GT_STATUS cpssDxChPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
);


/**
* @internal cpssDxChPortForwardUnknownMacControlFramesEnableGet function
* @endinternal
*
* @brief   Get current status of unknown MAC control frames
*         forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of unknown MAC control frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packets Length/EtherType field is 88-08
*       - Packets OpCode field is not 00-01 and not 01-01
*       OR
*       Packets MAC DA is not 01-80-C2-00-00-01 and not the ports configured
*       MAC Address
*
*/
GT_STATUS cpssDxChPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortSerdesTxEnableSet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
GT_STATUS cpssDxChPortSerdesTxEnableSet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChPortSerdesTxEnableGet function
* @endinternal
*
* @brief   Get Enable / Disable transmission of packets in SERDES layer of a port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - Pointer to transmission of packets in SERDES
*                                      layer of a port.
*                                      - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port.
*                                      - GT_FALSE - Enable transmission of packets in
*                                      SERDES layer of a port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesTxEnableGet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortSerdesTuningSet function
* @endinternal
*
* @brief   SerDes fine tuning values set.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where values
*                                      in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set same tuning parameters for all lanes defined in laneBmp in SW DB,
*       then cpssDxChPortSerdesPowerStatusSet will write them to HW.
*
*/
GT_STATUS cpssDxChPortSerdesTuningSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneBmp,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/**
* @internal cpssDxChPortSerdesTuningGet function
* @endinternal
*
* @brief   SerDes fine tuning values get.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure to put tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reads values saved in SW DB by cpssDxChPortSerdesTuningSet or
*       if it was not called, from default matrix (...SerdesPowerUpSequence).
*
*/
GT_STATUS cpssDxChPortSerdesTuningGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/**
* @internal cpssDxChPortPeriodicFlowControlIntervalSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval.
*
* @note   APPLICABLE DEVICES:     Lion2;Falcon;
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2;  Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                 - physical port number(APPLICABLE DEVICES:Falcon)
* @param[in] portType                 - port type(APPLICABLE DEVICES:Lion; Lion2)
* @param[in] value                    - For Lion and Lion2 - The interval in microseconds between two successive
*                                                       Flow Control frames that are sent periodically by the port.
*                                                       (APPLICABLE RANGES: 0..171798691)
*                                                       For Falcon - The interval is in 512 bit-time(APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portType
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    IN  GT_U32                              value
);

/**
* @internal cpssDxChPortPeriodicFlowControlIntervalGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval.
*
* @note   APPLICABLE DEVICES:     Lion2;Falcon;
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2;  Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                 - physical port number(APPLICABLE DEVICES:Falcon)
* @param[in] portType                 - port type(APPLICABLE DEVICES:Lion; Lion2)
*
* @param[out] valuePtr                    (Pointer to)For Lion and Lion2 - The interval in microseconds between two successive
*                                                       Flow Control frames that are sent periodically by the port.
*                                                       (APPLICABLE RANGES: 0..171798691)
*                                                       For Falcon - The interval is in 512 bit-time(APPLICABLE RANGES: 0..0xFFFF)

*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    OUT GT_U32                              *valuePtr
);


/**
* @internal cpssDxChPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supposed to replace old API's:
*       cpssDxChPortInterfaceModeSet
*       cpssDxChPortSpeedSet
*       cpssDxChPortSerdesPowerStatusSet
*       The API rolls back a port's mode and speed to their last values
*       if they cannot be set together on the device.
*       Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*       Pay attention! Unlike other interfaces which are ready to forward traffic
*       after this API pass, interlaken interfaces require call afterwards
*       cpssDxChPortIlknChannelSpeedSet to configure channel.
*
*/
GT_STATUS cpssDxChPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC             *portsBmpPtr,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal cpssDxChPortExtendedModeEnableSet function
* @endinternal
*
* @brief   Define which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
*                                      (APPLICABLE RANGES: Lion2: 9,11; xCat3: 25,27)
* @param[in] enable                   - extended mode:
*                                      GT_TRUE - use extended MAC;
*                                      GT_FALSE - use local MAC;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now if application interested to implement GE and XG modes of ports 9
*       and 11 of every mini-GOP over extended MAC's, it can call this function
*       at init stage once for port 9 and once for port 11 and enough
*
*/
GT_STATUS cpssDxChPortExtendedModeEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChPortExtendedModeEnableGet function
* @endinternal
*
* @brief   Read which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
*
* @param[out] enablePtr                - extended mode:
*                                      GT_TRUE - use extended MAC;
*                                      GT_FALSE - use local MAC;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - enablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortExtendedModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChPortSerdesAutoTune function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note Directions for TX training execution:
*       1.    As a pre-condition, ports on both sides of the link must be UP.
*       2.    For 2 sides of the link call execute TX_TRAINING_CFG;
*       No need to maintain special timing sequence between them.
*       The CFG phase sets some parameters at the SerDes as a preparation
*       to the training phase.
*       3.    After CFG is done, for both sides of the link, call TX_TRAINING_START;
*       No need to maintain special timing sequence between them.
*       4.    Wait at least 0.5 Sec. (done by API inside).
*       5.    Verify Training status by calling TX_TRAINING_STATUS.
*       No need to maintain special timing sequence between them.
*       This call provides the training status (OK/Failed) and terminates it.
*
*/
GT_STATUS cpssDxChPortSerdesAutoTune
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode
);

/**
* @internal cpssDxChPortSerdesAutoTuneExt function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes including required
*         optimizations.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number, device, serdesOptAlgBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note See cpssDxChPortSerdesAutoTune.
*
*/
GT_STATUS cpssDxChPortSerdesAutoTuneExt
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode,
    IN  GT_U32                                   serdesOptAlgBmp
);

/**
* @internal cpssDxChPortSerdesAutoTuneStatusGet function
* @endinternal
*
* @brief   Get current status of RX and TX serdes auto-tuning on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxTuneStatusPtr          - RX tuning status
* @param[out] txTuneStatusPtr          - TX tuning status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChPortSerdesAutoTuneStatusGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
);

/**
* @internal cpssDxChPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2;
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
*
* @param[out] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
GT_STATUS cpssDxChPortSerdesLanePolarityGet
(
    IN   GT_U8      devNum,
    IN   GT_U32     portGroupNum,
    IN   GT_U32     laneNum,
    OUT  GT_BOOL    *invertTx,
    OUT  GT_BOOL    *invertRx
);

/**
* @internal cpssDxChPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2;
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
* @param[in] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
GT_STATUS cpssDxChPortSerdesLanePolaritySet
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupNum,
    IN  GT_U32      laneNum,
    IN  GT_BOOL     invertTx,
    IN  GT_BOOL     invertRx
);

/**
* @internal cpssDxChPortSerdesPolaritySet function
* @endinternal
*
* @brief   Invert the Tx or Rx serdes polarity.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) to define
*                                      polarity on.
* @param[in] invertTx                   GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE  no invert
* @param[in] invertRx                   GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE  no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note This API should be egaged by application after serdes power up. Important to
*       wrap both steps i.e. "serdes power up" and "serdes polarity set" by port
*       disable and link_change interrupt lock and restore port enable and
*       reenable link change interrupt only after "serdes polarity set" to
*       prevent interrupt toggling during the process.
*
*/
GT_STATUS cpssDxChPortSerdesPolaritySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               laneBmp,
    IN GT_BOOL              invertTx,
    IN GT_BOOL              invertRx
);

/**
* @internal cpssDxChPortSerdesPolarityGet function
* @endinternal
*
* @brief   Get status of the Tx or Rx serdes polarity invert.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.) to
*                                      define polarity on.
*
* @param[out] invertTxPtr               (ptr to) GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE  no invert
* @param[out] invertRxPtr               (ptr to) GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE  no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesPolarityGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  laneNum,
    OUT GT_BOOL *invertTxPtr,
    OUT GT_BOOL *invertRxPtr
);

/**
* @internal cpssDxChPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum  - physical device number
* @param[in] portNum - physical port number
* @param[in] laneBmp - Not used
* @param[in] mode    - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
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
GT_STATUS cpssDxChPortSerdesLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneBmp,
    IN  CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT mode
);

/**
* @internal cpssDxChPortSerdesLoopbackModeGet function
* @endinternal
*
* @brief   Get current mode of loopback on SerDes.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*                                      to read loopback status
*
* @param[out] modePtr                  - current loopback mode or none
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_SUPPORTED         - on not expected mode value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesLoopbackModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneNum,
    OUT CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT *modePtr
);

/**
* @internal cpssDxChPortEomDfeResGet function
* @endinternal
*
* @brief   Returns the current DFE parameters.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; xCat3.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] dfeResPtr                - current DFE V in millivolts
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
GT_STATUS cpssDxChPortEomDfeResGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U32                 *dfeResPtr
);

/**
* @internal cpssDxChPortEomMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
* @param[in] samplingTime             - sampling time in usec, must be a positive value.
*
* @param[out] rowSizePtr               - number of rows in matrix
* @param[out] matrixPtr                - horizontal/vertical Rx eye matrix
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*
* @note The function initializes the EOM mechanism for the serdes,
*       gets the matrix and then closes the mechanism.
*       Due to it's size the CPSS_DXCH_PORT_EOM_MATRIX_STC struct should
*       be allocated on the heap.
*
*/
GT_STATUS cpssDxChPortEomMatrixGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    IN  GT_U32                           serdesNum,
    IN  GT_U32                           samplingTime,
    OUT GT_U32                          *rowSizePtr,
    OUT CPSS_DXCH_PORT_EOM_MATRIX_STC   *matrixPtr
);

/**
* @internal cpssDxChPortSerdesEyeMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix for SERDES.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - SERDES number
* @param[in] eye_resultsPtr           - serdes Eye monitoring results
*
* @param[out] eye_resultsPtr           - serdes Eye monitoring results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - if not supported
* @retval else                     - on error
*
* @note The function allocated memory and gets the SERDES EOM matrix, vbtc and
*       hbtc calculation in text format.
*       After using this function application needs to free allocated memory, for example:
*       cpssOsFree(eye_resultsPtr->matrixPtr);
*       cpssOsFree(eye_resultsPtr->vbtcPtr);
*       cpssOsFree(eye_resultsPtr->hbtcPtr);
*
*/
GT_STATUS cpssDxChPortSerdesEyeMatrixGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                serdesNum,
    IN  CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC  *eye_inputPtr,
    OUT CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
);

/**
* @internal cpssDxChPortEomBaudRateGet function
* @endinternal
*
* @brief   Returns the current system baud rate.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
GT_STATUS cpssDxChPortEomBaudRateGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
    OUT GT_U32                   *baudRatePtr
);

/**
* @internal cpssDxChPortInbandAutoNegRestart function
* @endinternal
*
* @brief   Restart inband auto-negotiation. Relevant only when inband auto-neg.
*         enabled.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortInbandAutoNegRestart
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal cpssDxChPortGePrbsIntReplaceEnableSet function
* @endinternal
*
* @brief   Enable/disable replace <QSGMII PRBS error> interrupt by
*         <Signal Detect triggered> interrupt.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
*                                      On XCAT2 supported only on: 0,4,8,12,16,20,24,25,26,27
* @param[in] enable                   - GT_TRUE - bits of <QSGMII PRBS error> in Port Interrupt Cause
*                                      and Port Interrupt Mask registers used for
*                                      <Signal Detect triggered>
*                                      GT_FALSE - <QSGMII PRBS error> bits used for himself
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGePrbsIntReplaceEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL  enable
);

/**
* @internal cpssDxChPortGePrbsIntReplaceEnableGet function
* @endinternal
*
* @brief   Get status of replace <QSGMII PRBS error> interrupt by
*         <Signal Detect triggered> interrupt.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
*                                      On XCAT2 supported only on: 0,4,8,12,16,20,24,25,26,27
*
* @param[out] enablePtr                - GT_TRUE - bits of <QSGMII PRBS error> in Port Interrupt Cause
*                                      and Port Interrupt Mask registers used for
*                                      <Signal Detect triggered>
*                                      GT_FALSE - <QSGMII PRBS error> bits used for himself
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - enablePtr is NULL
*/
GT_STATUS cpssDxChPortGePrbsIntReplaceEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortInterfaceSpeedSupportGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @param[out] supportedPtr              GT_TRUE  (ifMode; speed) supported
*                                      GT_FALSE  (ifMode; speed) not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortInterfaceSpeedSupportGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
);

/**
* @internal cpssDxChPortSerdesPpmSet function
* @endinternal
*
* @brief   Increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after port configured, not under traffic.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
* @param[in] ppmValue                 - signed value - positive means speed up, negative means slow
*                                      down, 0 - means disable PPM.
*                                      (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - ppmValue out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAD_STATE             - PPM value calculation wrong
*
* @note In Lion2 granularity of register is 30.5ppm, CPSS will translate ppmValue
*       to most close value to provided by application.
*
*/
GT_STATUS cpssDxChPortSerdesPpmSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                ppmValue
);

/**
* @internal cpssDxChPortSerdesPpmGet function
* @endinternal
*
* @brief   Get Tx clock increase/decrease status on port (added/sub ppm).
*         Can be run only after port configured.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - number of physical port all serdeses occupied by it, in
*                                      currently configured interface mode will be configured
*                                      to required PPM
*
* @param[out] ppmValuePtr              - PPM value currently used by given port.
*                                      0 - means PPM disabled.
*                                      (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note In Lion2 granularity of register is 30.5ppm.
*
*/
GT_STATUS cpssDxChPortSerdesPpmGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_32                *ppmValuePtr
);

/**
* @internal cpssDxChPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*         APPLICABLE DEVICES:
*         xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*         INPUTS:
*         devNum - physical device number
*         portNum - physical port number
*         laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*         serdesTxCfgPtr - serdes Tx parameters:
*         txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*         In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*         txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*         APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*         emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*         emph1 - Controls the emphasis amplitude for Gen1 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*         txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*                                      txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*                                      emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*                                      emph1 - Controls the emphasis amplitude for Gen1 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*                                      txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
GT_STATUS cpssDxChPortSerdesManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
);

/**
* @internal cpssDxChPortSerdesRxauiManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW for
*         RXAUI mode.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2;
*         Caelum; ; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfPorts               - Size of input array
* @param[in] serdesRxauiTxCfgPtr      - array of serdes Tx
*       parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_BAD_PTR               - NULL pointer
*
* @note
*
*/
GT_STATUS cpssDxChPortSerdesRxauiManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_U8                                numOfPorts,
    IN CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC  *serdesRxauiTxCfgPtr
);

/**
* @internal cpssDxChPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes TX.
*         APPLICABLE DEVICES:
*         xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*         INPUTS:
*         devNum - physical device number
*         portNum - physical port number
*         laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*         OUTPUTS:
*         serdesTxCfgPtr - serdes Tx parameters:
*         txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*         In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*         txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*         APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*         emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*         emph1 - Controls the emphasis amplitude for Gen1 bit rates
*         In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*         In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*         In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*         txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; Bobcat3 devices: Attenuator
*                                      txAmpAdjEn - Transmitter Amplitude Adjust: (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*                                      emph0 - Controls the Emphasis Amplitude for Gen0 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Post-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Post-Cursor [0...31]
*                                      emph1 - Controls the emphasis amplitude for Gen1 bit rates
*                                      In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: [0...15]
*                                      In Avago Serdes for Caelum; Aldrin; AC3X; devices: Pre-Cursor [-31...31]
*                                      In Avago Serdes for Bobcat3 device: Pre-Cursor [0...31]
*                                      txAmpShft - Transmitter Amplitude Shift (GT_TRUE - enable, GT_FALSE - disable).
*                                      APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; Lion2; Bobcat2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesManualTxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
);
/**
* @internal cpssDxChPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
GT_STATUS cpssDxChPortSerdesManualRxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
);

/**
* @internal cpssDxChPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes RX.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesManualRxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
);

/**
* @internal cpssDxChPortSerdesSquelchSet function
* @endinternal
*
* @brief   Set For port Threshold (Squelch) for signal OK.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] squelch                  - threshold for signal OK (0-15)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS cpssDxChPortSerdesSquelchSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  squelch
);

/**
* @internal cpssDxChPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFecModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FEC_MODE_ENT mode
);

/**
* @internal cpssDxChPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFecModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_FEC_MODE_ENT *modePtr
);

/**
* @internal cpssDxChPortPcsLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPcsLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    mode
);

/**
* @internal cpssDxChPortPcsLoopbackModeGet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPcsLoopbackModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    *modePtr
);


/**
* @internal cpssDxChPortsFastInit function
* @endinternal
*
* @brief   Fast ports configuration (interface, speed, serdes power up) during
*         system initialization.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap (or CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Same like cpssDxChPortModeSpeedSet(powerUp=true) but faster.
*       Purposed for use on system init, so assumes that ports are in HW default
*       i.e. all port's units in reset and serdes in power down.
*
*/
GT_STATUS cpssDxChPortsFastInit
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC             *portsBmpPtr,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal cpssDxChPortSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes (true/false).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS cpssDxChPortSerdesSignalDetectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
);

/**
* @internal cpssDxChPortSerdesStableSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS cpssDxChPortSerdesStableSignalDetectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
);

/**
* @internal cpssDxChPortPcsSyncStableStatusGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS cpssDxChPortPcsSyncStableStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
);

/**
* @internal cpssDxChPortSerdesLaneSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on specific SerDes lane (true/false).
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of local core
* @param[in] laneNum                  - number of required serdes lane
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesLaneSignalDetectGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  laneNum,
    OUT GT_BOOL *signalStatePtr
);

/**
* @internal cpssDxChPortSerdesCDRLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] cdrLockPtr               - CRD lock state on serdes:
*                                      GT_TRUE  - CDR locked;
*                                      GT_FALSE - CDR not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - cdrLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesCDRLockStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *cdrLockPtr
);

/**
* @internal cpssDxChPortPcsGearBoxStatusGet function
* @endinternal
*
* @brief   Return PCS Gear Box lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] gbLockPtr                - Gear Box lock state on serdes:
*                                      GT_TRUE  - locked;
*                                      GT_FALSE - not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - gbLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPcsGearBoxStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *gbLockPtr
);

/**
* @internal cpssDxChPortUnknownMacControlFramesCmdSet function
* @endinternal
*
* @brief   Set the unknown MAC control frames command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or command
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortUnknownMacControlFramesCmdSet
(
    IN GT_U8                    devNum,
    IN CPSS_PACKET_CMD_ENT      command
);

/**
* @internal cpssDxChPortUnknownMacControlFramesCmdGet function
* @endinternal
*
* @brief   Get the unknown MAC control frames command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortUnknownMacControlFramesCmdGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
);

/**
* @internal cpssDxChPortFlowControlPacketsCntGet function
* @endinternal
*
* @brief   Get the number of received and dropped Flow Control packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] receivedCntPtr           - the number of received packets
* @param[out] droppedCntPtr            - the number of dropped packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are cleared on read.
*
*/
GT_STATUS cpssDxChPortFlowControlPacketsCntGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *receivedCntPtr,
    OUT GT_U32      *droppedCntPtr
);

/**
* @internal cpssDxChPortMacTypeGet function
* @endinternal
*
* @brief   Get port MAC type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacTypePtr           - (pointer to) port MAC type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_TYPE_ENT  *portMacTypePtr
);


/**
* @internal cpssDxChPortPeriodicFlowControlCounterSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] value                    - The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The interval in micro seconds between transmission of two consecutive
*       Flow Control packets recommended interval is calculated by the following formula:
*       period (micro seconds) = 33553920 / speed(M)
*       Exception: for 10M, 100M and 10000M Flow Control packets recommended interval is 33500
*       Following are recommended intervals in micro seconds for common port speeds:
*       33500  for speed 10M
*       33500  for speed 100M
*       33500  for speed 1G
*       13421  for speed 2.5G
*       6710  for speed 5G
*       3355  for speed 10G
*       2843  for speed 11.8G
*       2796  for speed 12G
*       2467  for speed 13.6G
*       2236  for speed 15G
*       2097  for speed 16G
*       1677  for speed 20G
*       838   for speed 40G
*       710   for speed 47.2G
*       671   for speed 50G
*       447   for speed 75G
*       335   for speed 100G
*       239   for speed 140G
*
*/
GT_STATUS cpssDxChPortPeriodicFlowControlCounterSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  value
);


/**
* @internal cpssDxChPortPeriodicFlowControlCounterGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] valuePtr                 - (pointer to) The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlCounterGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *valuePtr
);


/**
* @internal cpssDxChPortPcsResetSet function
* @endinternal
*
* @brief   Set/unset the PCS reset for given mode on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Tx/Rx/All
* @param[in] state                    - If GT_TRUE, enable reset
*                                      If GT_FALSE, disable reset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reset only PCS unit used by port for currently configured interface.
*
*/
GT_STATUS cpssDxChPortPcsResetSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PCS_RESET_MODE_ENT   mode,
    IN  GT_BOOL                        state
);

/**
* @internal cpssDxChPortSerdesAutoTuneOptAlgSet function
* @endinternal
*
* @brief   Configure bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API configures field SW DB which will be used by
*       cpssDxChPortSerdesAutoTune.
*
*/
GT_STATUS cpssDxChPortSerdesAutoTuneOptAlgSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
);

/**
* @internal cpssDxChPortSerdesAutoTuneOptAlgGet function
* @endinternal
*
* @brief   Get bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] serdesOptAlgBmpPtr       - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesAutoTuneOptAlgGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               *serdesOptAlgBmpPtr
);

/**
* @internal cpssDxChPortSerdesAutoTuneOptAlgRun function
* @endinternal
*
* @brief   Run training/auto-tuning optimisation algorithms on serdeses of port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSerdesAutoTuneOptAlgRun
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
);

/**
* @internal cpssDxChPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssDxChPortSerdesPowerStatusSet or cpssDxChPortModeSpeedSet will write
*       them to HW.
*
*/
GT_STATUS cpssDxChPortSerdesLaneTuningSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroupNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/**
* @internal cpssDxChPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
GT_STATUS cpssDxChPortSerdesLaneTuningGet
(
    IN   GT_U8                   devNum,
    IN   GT_U32                  portGroupNum,
    IN   GT_U32                  laneNum,
    IN   CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/**
* @internal cpssDxChPortSerdesAlign90StateSet function
* @endinternal
*
* @brief   Start/stop align 90 algorithm on given port's serdes.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE - start
*                                      GT_FALSE - stop
* @param[in,out] serdesParamsPtr          - (ptr to) serdes parameters
* @param[in,out] serdesParamsPtr          - (ptr to) serdes parameters at start
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortSerdesAlign90StateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    INOUT CPSS_DXCH_PORT_ALIGN90_PARAMS_STC *serdesParamsPtr
);

/**
* @internal cpssDxChPortSerdesAlign90StatusGet function
* @endinternal
*
* @brief   Get status of align 90 algorithm on given port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statusPtr                - (ptr to) serdes parameters at start
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortSerdesAlign90StatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *statusPtr
);

/**
* @internal cpssDxChPortSerdesAutoTuneResultsGet function
* @endinternal
*
* @brief   Read the results of SERDES auto tuning.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTunePtr            - serdes Tune parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesAutoTuneResultsGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC       *serdesTunePtr
);
/**
* @internal cpssDxChPortSerdesEnhancedAutoTune function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2,xCat3; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssDxChPortSerdesEnhancedAutoTune
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U8                  min_LF,
    IN  GT_U8                  max_LF
);

/**
* @internal cpssDxChPortSerdesEnhancedTuneLite function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*                                      min_LF   - Minimum LF value that can be set on Serdes (0...15)
*                                      max_LF   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/

GT_STATUS cpssDxChPortSerdesEnhancedTuneLite
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U8                  min_dly,
    IN  GT_U8                  max_dly
);

/**
* @internal cpssDxChPortPeriodicFlowControlIntervalSelectionSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval selection.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - interval selection: use interval 0 tuned by default for GE
*                                      or interval 1 tuned by default for XG
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalSelectionSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType
);

/**
* @internal cpssDxChPortPeriodicFlowControlIntervalSelectionGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval selection.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portTypePtr              - interval selection: use interval 0 tuned by default for GE
*                                      or interval 1 tuned by default for XG
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalSelectionGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT *portTypePtr
);


/**
* @internal cpssDxChPortRefClockSourceOverrideEnableSet function
* @endinternal
*
* @brief   Enables/disables reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] overrideEnable           - override
*                                      GT_TRUE - use preconfigured refClock source;
*                                      GT_FALSE - use default refClock source;
* @param[in] refClockSource           - reference clock source.
*                                      Not relevant when overrideEnable is false.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, refClockSource
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRefClockSourceOverrideEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         overrideEnable,
    IN  CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource
);


/**
* @internal cpssDxChPortRefClockSourceOverrideEnableGet function
* @endinternal
*
* @brief   Gets status of reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] overrideEnablePtr        - (pointer to) override enable status
* @param[out] refClockSourcePtr        - (pointer to) reference clock source.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRefClockSourceOverrideEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *overrideEnablePtr,
    OUT CPSS_PORT_REF_CLOCK_SOURCE_ENT  *refClockSourcePtr
);

/**
* @internal cpssDxChPortUnitInfoGetByAddr function
* @endinternal
*
* @brief   Get unit ID by unit address in device
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  Lion; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] baseAddr                 - unit base address in device
*
* @param[out] unitIdPtr                - unit ID (MAC, PCS, SERDES)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortUnitInfoGetByAddr
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       baseAddr,
    OUT CPSS_DXCH_PORT_UNITS_ID_ENT  *unitIdPtr
);


/**
* @internal cpssDxChPortUnitInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  Lion; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Bobcat2.
*
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @param[out] baseAddrPtr              - unit base address in device
* @param[out] unitIndexPtr             - unit index in device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum, portType
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortUnitInfoGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_PORT_UNITS_ID_ENT unitId,
    OUT GT_U32                      *baseAddrPtr,
    OUT GT_U32                      *unitIndexPtr
);


/**
* @internal cpssDxChPortSerdesSequenceGet function
* @endinternal
*
* @brief   Get SERDES sequence one line.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Bobcat2.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] seqType                  - sequence type
* @param[in] lineNum                  - line number
*
* @param[out] seqLinePtr               - sequence line
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortSerdesSequenceGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  portGroup,
    IN  CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT    seqType,
    IN  GT_U32                                  lineNum,
    OUT CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC *seqLinePtr
);


/**
* @internal cpssDxChPortSerdesSequenceSet function
* @endinternal
*
* @brief   Set SERDES sequence one line.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat3; xCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Bobcat2.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] firstLine                - the first line
* @param[in] seqType                  - sequence type
* @param[in] unitId                   - unit Id
* @param[in] seqLinePtr               - sequence line
* @param[in] numOfOp                  - number of op
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortSerdesSequenceSet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   portGroup,
    IN GT_BOOL                                  firstLine,
    IN CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT     seqType,
    IN CPSS_DXCH_PORT_UNITS_ID_ENT              unitId,
    IN CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC  *seqLinePtr,
    IN GT_U32                                   numOfOp
);

/**
* @internal cpssDxChPortAutoNegAdvertismentConfigGet function
* @endinternal
*
* @brief   Getting <TX Config Reg> data for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
*
* @param[out] portAnAdvertismentPtr    - pointer to structure with port link status, speed and duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on wrong port mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_VALUE             - on wrong speed value in the register
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/

GT_STATUS cpssDxChPortAutoNegAdvertismentConfigGet
(
    IN   GT_U8                               devNum,
    IN   GT_PHYSICAL_PORT_NUM                portNum,
    CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
);

/**
* @internal cpssDxChPortAutoNegAdvertismentConfigSet function
* @endinternal
*
* @brief   Configure <TX Config Reg> for Auto-Negotiation.
*         When Auto-Negotiation Master Mode and code word enable,
*         The device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
* @param[in] portAnAdvertismentPtr    - pointer to structure with port link status, speed and duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssDxChPortAutoNegAdvertismentConfigSet
(
    IN   GT_U8                               devNum,
    IN   GT_PHYSICAL_PORT_NUM                portNum,
    CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
);

/**
* @internal cpssDxChPortPcsSyncStatusGet function
* @endinternal
*
* @brief   Return PCS Sync status from XGKR sync block.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] syncPtr                  - Sync status from XGKR sync block :
*                                      GT_TRUE  - synced;
*                                      GT_FALSE - not synced.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - syncPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPcsSyncStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *syncPtr
);

/**
* @internal cpssDxChPortAutoNegMasterModeEnableSet function
* @endinternal
*
* @brief   Set Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
* @param[in] enable                   - GT_TRUE   -  Auto-Negotiation Master Mode and code word
*                                      GT_FALSE  - ASIC defines Auto-Negotiation code word
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssDxChPortAutoNegMasterModeEnableSet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_BOOL                  enable
);

/**
* @internal cpssDxChPortAutoNegMasterModeEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status for Auto-Negotiation Master Mode and code word.
*         When enable, the device sends out <TX Config Reg> as the
*         Auto-Negotiation code word. This mode is used when there is no PHY
*         between link partners.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not-CPU)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - the request is not supported for this port mode
* @retval GT_NOT_APPLICABLE_DEVICE - the request is not supported for this device
*
* @note Auto-Negotiation Master Mode and code word can be enabled on SGMII.
*
*/
GT_STATUS cpssDxChPortAutoNegMasterModeEnableGet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChPortResourceTmBandwidthSet function
* @endinternal
*
* @brief   Define system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tmBandwidthMbps          - system TM bandwidth
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note:
*       - API should not be called under traffic
*       - zero bandwidth will release resources of TM for non TM ports
*
*/
GT_STATUS cpssDxChPortResourceTmBandwidthSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps
);


/**
* @internal cpssDxChPortResourceTmBandwidthGet function
* @endinternal
*
* @brief   Get system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @param[out] tmBandwidthMbpsPtr       - (pointer to) system TM bandwidth
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortResourceTmBandwidthGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *tmBandwidthMbpsPtr
);

/**
* @internal cpssDxChPortCtleBiasOverrideEnableSet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Puma; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
* @param[in] overrideEnable           - override the CTLE default value
* @param[in] ctleBiasValue            - value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
GT_STATUS cpssDxChPortCtleBiasOverrideEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  GT_BOOL                            overrideEnable,
    IN  CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT  ctleBiasValue
);

/**
* @internal cpssDxChPortCtleBiasOverrideEnableGet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Puma; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
*
* @param[out] overrideEnablePtr        - pointer to override mode
* @param[out] ctleBiasValuePtr         - pointer to value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
GT_STATUS cpssDxChPortCtleBiasOverrideEnableGet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  GT_BOOL                            *overrideEnablePtr,
    IN  CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT  *ctleBiasValuePtr
);

/**
* @internal cpssDxChPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overridden. The default value for the
*         override mode in Caelum rev 1 and above is true, on other devices is false.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Puma; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2.
*
* @param[in] devNum                   - Device Number
* @param[in] vosOverride              - GT_TRUE means to override the VOS parameters for the device, GT_FALSE otherwise.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
GT_STATUS cpssDxChPortVosOverrideControlModeSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     vosOverride
);

/**
* @internal cpssDxChPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Get the override mode of the VOS parameters for all ports.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Lion; Lion2; Puma; Puma3; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2.
*
* @param[in] devNum                   - Device Number
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if vosOverridePtr is NULL pointer
*
* @note In Caelum this API is applicable only in A1 revision and above.
*
*/
GT_STATUS cpssDxChPortVosOverrideControlModeGet
(
    IN   GT_U8       devNum,
    OUT  GT_BOOL     *vosOverridePtr
);

/**
* @internal cpssDxChPortXlgReduceAverageIPGSet function
* @endinternal
*
* @brief   Configure Reduce Average IPG in XLG MAC.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] value                    -  to set to the XLG MAC DIC_PPM_ IPG_Reduce Register (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM  on bad parameters
*
* @note Function should be used to solve the problem:
*       The port BW is few PPMs lower than FWS, and tail drops occur.
*       (FE-7680593)
*
*/
GT_STATUS cpssDxChPortXlgReduceAverageIPGSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          value
);

/**
* @internal cpssDxChPortXlgReduceAverageIPGGet function
* @endinternal
*
* @brief   Get Reduce Average IPG value in XLG MAC.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] valuePtr                 -  pointer to value  content of the XLG MAC DIC_PPM_IPG_Reduce register
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM  on bad parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChPortXlgReduceAverageIPGGet
(
        IN  GT_U8                   devNum,
        IN  GT_PHYSICAL_PORT_NUM    portNum,
        OUT GT_U32                 *valuePtr
);

/**
* @internal cpssDxChPortCascadePfcParametersSet function
* @endinternal
*
* @brief   Set PFC parameters per remoting cascade port.
*         Function is relevant for AC3X systems after cpssDxChCfgRemoteFcModeSet(),
*         Function should be used for advanced customization configuration only.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] timer                    - PFC  (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] tcBitmap                 - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note PFC is sent in uplink direction from remote port.
*
*/
GT_STATUS cpssDxChPortCascadePfcParametersSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      xOffThreshold,
    IN  GT_U32      xOnThreshold,
    IN  GT_U32      timer,
    IN  GT_U32      tcBitmap
);

/**
* @internal cpssDxChPortCascadePfcParametersGet function
* @endinternal
*
* @brief   Get PFC parameters per remoting cascade port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
*
* @param[out] xOffThresholdPtr         - (Pointer to) xOff threshold in buffers
* @param[out] xOnThresholdPtr          - (Pointer to) xOn threshold in buffers
* @param[out] timerPtr                 - (Pointer to) PFC timer
* @param[out] tcBitmapPtr              - (Pointer to) list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChPortCascadePfcParametersGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *xOffThresholdPtr,
    OUT GT_U32      *xOnThresholdPtr,
    OUT GT_U32      *timerPtr,
    OUT GT_U32      *tcBitmapPtr
);

/**
* @internal cpssDxChPortRemoteFcParametersSet function
* @endinternal
*
* @brief   Set FC parameters per remote port.
*         Function is relevant for AC3X systems after cpssDxChCfgRemoteFcModeSet(),
*         Function should be used for advanced customization configuration only.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] portNum                  - physical port number
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Threshold are detected by remoting PHY/MAC,
*       FC is sent in downlink (NW) direction.
*
*/
GT_STATUS cpssDxChPortRemoteFcParametersSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  xOffThreshold,
    IN  GT_U32                  xOnThreshold
);

/**
* @internal cpssDxChPortRemoteFcParametersGet function
* @endinternal
*
* @brief   Get FC parameters per remote port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] portNum                  - physical port number
*
* @param[out] xOffThresholdPtr         - (Pointer to) xOff threshold in buffers
* @param[out] xOnThresholdPtr          - (Pointer to) xOn threshold in buffers
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChPortRemoteFcParametersGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *xOffThresholdPtr,
    OUT GT_U32                  *xOnThresholdPtr
);

/**
* @internal cpssDxChPortSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX data, TX data or both
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat3; xCat2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - number of port group (local core), not used for non-multi-core
* @param[in] serdesNum                - serdes number
* @param[in] numOfBits                - number of bits to inject to serdes data
* @param[in] direction                - whether  is TX, RX or both
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_INIT_ERROR            - on Hws initialization failre
*
* @note Debug function.
*
*/
GT_STATUS cpssDxChPortSerdesErrorInject
(
    IN GT_U8    devNum,
    IN GT_U32   portGroup,
    IN GT_U32   serdesNum,
    IN GT_U32   numOfBits,
    IN CPSS_PORT_DIRECTION_ENT  direction
);

/*************************************************************************
* @internal cpssDxChPortLaneMacToSerdesMuxSet
* @endinternal
 *
* @brief   Set the Port lane Mac to Serdes Mux
 *
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on pointer problem
*
* @param[in] devNum             - system device number
* @param[in] PortNum            - Physical port number
* @param[in] serdesToMacMuxStr  - the setup of the muxing
************************************************************************/
GT_STATUS cpssDxChPortLaneMacToSerdesMuxSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_MAC_TO_SERDES_STC  *macToSerdesMuxStc
);

/*************************************************************************
* @internal cpssDxChPortLaneMacToSerdesMuxGet
* @endinternal
 *
* @brief   Get the Port lane Mac to Serdes Mux
 *
* @note   APPLICABLE DEVICES: Aldrin2.
* @note   NOT APPLICABLE DEVICES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on pointer problem
*
* @param[in] devNum       - system device number
* @param[in] portNum      - physical port number
* @param[out] serdesToMacMuxStr   - struct that will contaion the mux setup from the database
************************************************************************/
GT_STATUS cpssDxChPortLaneMacToSerdesMuxGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_TO_SERDES_STC     *macToSerdesMuxStc
);

/**
* @internal cpssDxChPortDebugLogEntrySet function
* @endinternal
*
* @brief   Set CPSS logging mode for all libs and all log-types and enter (or exit)
*         port lib cpss logger. For debug purposes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   - whether to  or disable logging
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note This API is for debug purposes only. It is public CPSS API in order for
*       the automatic tools to keep generate log data for this API. It is usefull
*       for Hws APIs in which when calling them directly, the Hws debug information
*       will be treated as CPSS log.
*
*/
GT_STATUS cpssDxChPortDebugLogEntrySet
(
    IN  GT_BOOL   enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortCtrlh */

