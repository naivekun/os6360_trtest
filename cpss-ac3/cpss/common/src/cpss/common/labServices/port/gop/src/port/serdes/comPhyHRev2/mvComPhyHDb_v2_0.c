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
* mvhwsComPhyHRev2Db.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 59 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2Db.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

static MV_CFG_ELEMENT cmPhy_SerdesSdResetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSdUnresetSeq[] = {
    {WRITE_OP, {SERDES_UNIT,  SERDES_EXTERNAL_CONFIGURATION_1, (1 << 2) | (1 << 3), (1 << 2) | (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesRfResetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0,   (1 << 6)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesRfUnresetSeq[] = {
    {WRITE_OP, {SERDES_UNIT,  SERDES_EXTERNAL_CONFIGURATION_1, (1 << 6), (1 << 2) | (1 << 6)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSynceResetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSynceUnresetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, (1 << 3), (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesPowerUpCtrlSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x1802,   0x1802}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x10,     0x10}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1, (1 << 3), (1 << 3)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0x7000,   0x7000}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesPowerDownCtrlSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0, 0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1, 1, 0xFFFF77FF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0, 0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_3, 0x20, 0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0, 0, 0}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesRxintUpSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98, 0x400, 0x400}},
/*    {POLL_OP,  {SERDES_UNIT, SERDES_PHY_REGS + 0x8C, 0x40, 0x40}},*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98, 0, 0x400}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesRxintDownSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1, 0, (1 << 4)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98, 0, 0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1, (1 << 3), (1 << 3)}}
};

static MV_CFG_ELEMENT speed_1_25G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x0330, 0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0x000C, 0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0xF247, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x79F1, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x0FC9, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x1866, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0x1DF,  0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x8CC0, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0xF0C0, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x35F4, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3040, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xE026, 0xFBFF}}, /* ICP=6 */

    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x800,  0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0007, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x0100, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */

    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0xE028, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0xB20,  0xFFFF}}, /* samplr_vcm=0 */

    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFD68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_3_125G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x440,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0xBA79, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x0FD2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x1088, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0x81DF, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x0CF4, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0xF0DA, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x35F0, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3050, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x800,  0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0007, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0xD428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x42C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFD68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_3_3G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x440,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0xBA79, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x0FD2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x1088, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0x81DF, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x0CF4, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0xF0DA, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x35F0, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3060, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x800,  0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0007, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0xD428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x42C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFD68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_3_75G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x4C8,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0xBA79, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x0FD2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x1099, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0x81DF, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x0CF4, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0xF0DA, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x35F0, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3060, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x800,  0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0007, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0xD428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x42C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFD68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_4_25G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x550,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x0DFF, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x0FD2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x10AA, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0xb0b4, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x2494, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0x2094, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x03a4, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3070, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x800,  0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0007, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0xD428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x62C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFD68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_5G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x198,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x0DFF, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x0FD2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x1033, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0x8039, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x24B8, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0xF0DA, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x35F0, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3050, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x800,  0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0007, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0x5428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x62C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFD68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_6_25G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x5D8,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x0DFF, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x0FD2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x10BB, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0x8039, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x24B8, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0xF0DA, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x35F0, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3050, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x800,  0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0007, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0x5428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x62C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFD68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_7_5G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x6E8,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x0DFF, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x0FD2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x10DD, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0xb0b4, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x2494, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0x2094, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x03a4, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3070, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x800,  0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0007, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0x5428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x72C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFD68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_10_3125G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x770,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x0BD8, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x17D2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x10EE, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x51,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0xb0b6, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x24B4, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0x2094, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x0364, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3050, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x1800, 0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0087, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0x5428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x72C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFE68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_11_5625G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x7F8,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x0DFF, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x17D2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x10FF, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0xb0bf, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x24B4, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0x2094, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x03f4, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3050, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x1800, 0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0087, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0x5428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x72C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFE68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_12_5G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x7F8,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x0DFF, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x17D2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x10FF, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0xb0bf, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x24B4, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0x2094, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x03f4, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3050, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x1800, 0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0087, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0x5428, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x72C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFE68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT speed_12_1875G_SeqParams[] =
{
    { WRITE_OP, {SERDES_UNIT, 0x0000,                  0x7F8,  0x07F8}}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    { WRITE_OP, {SERDES_UNIT, 0x0028,                  0xC,    0x001F}}, /* PIN_FREF_SEL=C (156.25MHz) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x004, 0xFC8C, 0xFFFF}}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x018, 0x06BF, 0xFFFF}}, /* SQ_THERSH=6, FFE_R=3, FFE_C=0xF */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x01C, 0x7047, 0xFFFF}}, /* DFE_RES=3mV */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x020, 0xAFC0, 0xFFFF}}, /* DFE update enable */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x034, 0x0DFF, 0xFFFF}}, /* G1_TX_AMP=0x1F, AMP_SHFT=1, AMP_ADJ=1, EMPH1 = 0xB */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x038, 0x17D2, 0xFFFF}}, /* G1_RX_DFE_EN=1, G1_SELMUPF/I=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x084, 0x30,   0xFFFF}}, /* PT_PHYREADY_FORCE=1, PT_TESTMODE=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x088, 0x1026, 0xFFFF}}, /* Password un protect */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x098, 0x10FF, 0xFFFF}}, /* Normal mode, seting TX/RX GEN Table */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F4, 0x52,   0xFFFF}}, /* G1_EMPH0=7 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x2600, 0xFFFF}}, /* tx/rx_impcalth=1 (94?) */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108, 0xb0b6, 0xFFFF}}, /* cal_vdd_continuas_mode=1, rxdig=0x16 for 10.3G and 12.1875G, rxdig=0x14 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C, 0x24B4, 0xFFFF}}, /* VDD calibration, only for 10.3G and 12.5G txclk=20 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110, 0x2094, 0xFFFF}}, /* VDD calibration */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114, 0x0364, 0xFFFF}}, /* VDD calibration, rxsampl=0x1f for 10.3G and 12.1875G, rxsampl=0x16 for lower */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x128, 0x600,  0xFFFF}}, /* ck100k_in_pll_freq_sel=2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134, 0x3050, 0xFFFF}}, /* sellv_rxintp=0, vregrxtx=3,selvthvco_cont=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138, 0x105,  0xFFFF}}, /* vregvco=1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C, 0xA026, 0xFBFF}}, /* ICP=6 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140, 0x1800, 0xFFFF}}, /* CLK_8T=1 in 10.3 and 12.5G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14C, 0x04B4, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x150, 0x8001, 0xFFFF}}, /* force fbdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x154, 0x0087, 0xFFFF}}, /* force rx dig clock divider */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160, 0x010A, 0xFFFF}}, /* force refdiv for 3.33G */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x170, 0x722C, 0xFFFF}}, /* update process calibration threshold */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1D0, 0xD,    0xFFFF}}, /* increase current to 300 uA */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184, 0x1293, 0xFFFF}}, /* DTL_FLOOP_EN=0 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168, 0xE028, 0xFFFF}}, /* Align90=84, for 12_1875G: Align90_Calib = Force */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C8, 0x0FFE, 0xFFFF}}, /* sq_lpf_en=1, sq_lpf=0x3fe */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x4688, 0xFFFF}}, /* DFE Timer coarse EO_UP =6, EO_DN=4 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC, 0x72C,  0xFFFF}}, /* samplr_vcm=0 */
                                                              
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1,    0xFFFF}}, /* page 1 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0F0, 0x38,   0xFFFF}}, /* ffe_cal_vth[1:0]=3 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x0FC, 0xFE68, 0xFFFF}}, /* change gain for FFE0-2 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x100, 0x5A4C, 0xFFFF}}, /* change gain for FFE3-5 */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104, 0x28C0, 0xFFFF}}, /* ffe_gain_in6,7 changes */
    { WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0,    0xFFFF}} /* page 0 */
};

static MV_CFG_ELEMENT ptAfterPatternTestSeqParams[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0x80E0, 0xFFFF}}
};

static MV_CFG_ELEMENT rxtrainingEnSeqParams[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x188,  0x200,  0x200}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94,   0x1000, 0x1000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,   0x0,    0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,   0xF,    0xF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,   0x80,   0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,   0x2000, 0x2000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,   0xFC0,  0xFC0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x40,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x48,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x50,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,   0x0,    0x100}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1A8,  0x4000, 0x4000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,   0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C,   0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C,   0x800,  0x2802}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x130,  0x80,   0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x16C,  0x3C4,  0x3FF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x80,   0x233,  0x7FF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x2,    0x2}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x0,    0x2}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x78,   0x0,    0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x28,   0x2000, 0x2000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x120,  0x40,   0x40}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x120,  0x80,   0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,   0x0,    0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x130,  0x0,    0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC,    0x200,  0x200}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC,    0x0,    0x3F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,  0x0,    0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x198,  0x0,    0xC00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,  0x70,   0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x4000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x600,  0xF00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xEC,   0x6000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xEC,   0xA00,  0xF00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,   0x40,   0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,   0x8,    0xF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x8,    0xF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x40,   0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xEC,   0x80,   0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x80,   0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,  0x1000, 0x1C00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x78,   0x7800, 0x7800}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1AC,  0x1C0,  0x1C0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1AC,  0x38,   0x38}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x7C,   0x7,    0x7}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,  0x7,    0x7}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x7C,   0x1C0,  0x1C0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,  0x1C0,  0x1C0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x7C,   0xE00,  0xE00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x7C,   0x38,   0x38}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,  0x38,   0x38}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x120,  0x14,   0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x40,   0xC0,   0xF7E0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,  0x0,    0x8000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x64,   0x0,    0xC000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C,   0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x84,   0x4874, 0xC87C}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x12C,  0x4000, 0x4000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x188,  0x0,    0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x80,   0x830,  0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x70,   0x8800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, 0xC,                      0x80,   0x80}}
};

static MV_CFG_ELEMENT txtrainingEnSeqParams[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x9C,   0x27F,  0xFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94,   0xCC0,  0xFF8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xDC,   0xc71,  0x3FFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,  0x1400, 0x3FFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,  0xC71,  0x3FFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC0,   0x100,  0x100}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x15C,  0x0,    0x2000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x188,  0x200,  0x200}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94,   0x1000, 0x1000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,   0x0,    0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,   0xF,    0xF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,   0x80,   0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,   0x2000, 0x2000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,   0xFC0,  0xFC0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x40,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x48,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x50,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,   0x0,    0x100}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1A8,  0x4000, 0x4000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,   0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2c,   0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C,   0x800,  0x2802}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x130,  0x80,   0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x16C,  0x3C4,  0x3FF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x80,   0x233,  0x7FF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x2,    0x2}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x0,    0x2}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x78,   0x0,    0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x28,   0x2000, 0x2003}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x120,  0x40,   0x40}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x120,  0x80,   0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x9C,   0x27F,  0xFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94,   0xCC0,  0xFF8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94,   0x0,    0x1000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,   0x0,    0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x130,  0x0,    0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x70,   0x800,  0x8800}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x28,   0x80,   0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x11C,  0x202,  0x207}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x28,   0x400,  0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x28,   0x0,    0x40}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x11C,  0x100,  0x100}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,   0x4400, 0xFF80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C,   0xA,    0xE}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC,    0x300,  0x300}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC,    0x7,    0x3F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160,  0x0,    0x8000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x160,  0x0,    0x4000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,  0x0,    0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x198,  0x0,    0xC00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,  0x70,   0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x4000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x600,  0xF00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xEC,   0x3000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xEC,   0xA00,  0xF00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,   0x40,   0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,   0x8,    0xF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x8,    0xF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x80,   0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xEC,   0x80,   0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF0,   0x80,   0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,  0x1000, 0x1C00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x78,   0x7800, 0x7800}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1AC,  0x80,   0x1C0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1AC,  0x18,   0x38}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x7C,   0x6,    0x7}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,  0x7,    0x7}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x7C,   0x80,   0x1C0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,  0x1C0,  0x1C0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x7C,   0x0E00, 0x7E00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x7C,   0x18,   0x38}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,  0x38,   0x38}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x120,  0x18,   0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x40,   0xC0,   0xF7E0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x64,   0x0,    0xC000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x84,   0x840,  0xC87C}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x12C,  0x4000, 0x4000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,  0x0,    0x8000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x188,  0x0,    0x80}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xB8,   0xFFFF, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xB8,   0x0,    0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x3F,   0x3F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC,   0x0,    0x3F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}}
};

static MV_CFG_ELEMENT txtrainingDisSeqParams[] = {
    {WRITE_OP, {SERDES_UNIT, 0x8,                      0x0,    0x20}},
    {WRITE_OP, {SERDES_UNIT, 0xC,                      0x0,    0x100}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94,   0x1FFF, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C,   0x72,   0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C,   0x0,    0x2}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x11C,  0x0,    0x200}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}}
};

static MV_CFG_ELEMENT ffeTableLrSeqParams[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x44,   0xF0E,  0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x48,   0xD0C,  0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4C,   0x1F1E, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x50,   0x1D1C, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54,   0x1b1a, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x58,   0x1918, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x5C,   0x1716, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x60,   0x1514, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}}
};

static MV_CFG_ELEMENT ffeTableSrSeqParams[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x1,    0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x44,   0x2F2E, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x48,   0x2D2C, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4C,   0x3F3E, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x50,   0x3D3C, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54,   0x3b3a, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x58,   0x3938, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x5C,   0x3736, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x60,   0x3534, 0x0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x130,  0x2F,   0x7F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,  0x0,    0x1}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsSerdesRev2SeqDb[] =
{
    {cmPhy_SerdesSdResetSeq,                MV_SEQ_SIZE(cmPhy_SerdesSdResetSeq)},                /* SERDES_SD_RESET_SEQ                  */
    {cmPhy_SerdesSdUnresetSeq,              MV_SEQ_SIZE(cmPhy_SerdesSdUnresetSeq)},              /* SERDES_SD_UNRESET_SEQ                */
    {cmPhy_SerdesRfResetSeq,                MV_SEQ_SIZE(cmPhy_SerdesRfResetSeq)},                /* SERDES_RF_RESET_SEQ                  */
    {cmPhy_SerdesRfUnresetSeq,              MV_SEQ_SIZE(cmPhy_SerdesRfUnresetSeq)},              /* SERDES_RF_UNRESET_SEQ                */
    {cmPhy_SerdesSynceResetSeq,             MV_SEQ_SIZE(cmPhy_SerdesSynceResetSeq)},             /* SERDES_SYNCE_RESET_SEQ               */
    {cmPhy_SerdesSynceUnresetSeq,           MV_SEQ_SIZE(cmPhy_SerdesSynceUnresetSeq)},           /* SERDES_SYNCE_UNRESET_SEQ             */
    {cmPhy_SerdesSerdesPowerUpCtrlSeq,      MV_SEQ_SIZE(cmPhy_SerdesSerdesPowerUpCtrlSeq)},      /* SERDES_SERDES_POWER_UP_CTRL_SEQ      */
    {cmPhy_SerdesSerdesPowerDownCtrlSeq,    MV_SEQ_SIZE(cmPhy_SerdesSerdesPowerDownCtrlSeq)},    /* SERDES_SERDES_POWER_DOWN_CTRL_SEQ    */
    {cmPhy_SerdesSerdesRxintUpSeq,          MV_SEQ_SIZE(cmPhy_SerdesSerdesRxintUpSeq)},          /* SERDES_SERDES_RXINT_UP_SEQ           */
    {cmPhy_SerdesSerdesRxintDownSeq,        MV_SEQ_SIZE(cmPhy_SerdesSerdesRxintDownSeq)},        /* SERDES_SERDES_RXINT_DOWN_SEQ         */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SERDES_WAIT_PLL_SEQ           */
    {speed_1_25G_SeqParams,                 MV_SEQ_SIZE(speed_1_25G_SeqParams)},                 /* SERDES_SPEED_1_25G_SEQ               */
    {speed_3_125G_SeqParams,                MV_SEQ_SIZE(speed_3_125G_SeqParams)},                /* SERDES_SPEED_3_125G_SEQ              */
    {speed_3_75G_SeqParams,                 MV_SEQ_SIZE(speed_3_75G_SeqParams)},                 /* SERDES_SPEED_3_75G_SEQ               */
    {speed_4_25G_SeqParams,                 MV_SEQ_SIZE(speed_4_25G_SeqParams)},                 /* SERDES_SPEED_4_25G_SEQ               */
    {speed_5G_SeqParams,                    MV_SEQ_SIZE(speed_5G_SeqParams)},                    /* SERDES_SPEED_5G_SEQ                  */
    {speed_6_25G_SeqParams,                 MV_SEQ_SIZE(speed_6_25G_SeqParams)},                 /* SERDES_SPEED_6_25G_SEQ               */
    {speed_7_5G_SeqParams,                  MV_SEQ_SIZE(speed_7_5G_SeqParams)},                  /* SERDES_SPEED_7_5G_SEQ                */
    {speed_10_3125G_SeqParams,              MV_SEQ_SIZE(speed_10_3125G_SeqParams)},              /* SERDES_SPEED_10_3125G_SEQ            */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SD_LPBK_NORMAL_SEQ            */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SD_ANA_TX_2_RX_SEQ            */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SD_DIG_TX_2_RX_SEQ            */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SD_DIG_RX_2_TX_SEQ            */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_PT_AFTER_PATTERN_NORMAL_SEQ   */
    {ptAfterPatternTestSeqParams,           MV_SEQ_SIZE(ptAfterPatternTestSeqParams)},           /* SERDES_PT_AFTER_PATTERN_TEST_SEQ     */
    {rxtrainingEnSeqParams,                 MV_SEQ_SIZE(rxtrainingEnSeqParams)},                 /* SERDES_RX_TRAINING_ENABLE_SEQ        */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_RX_TRAINING_DISABLE_SEQ       */
    {txtrainingEnSeqParams,                 MV_SEQ_SIZE(txtrainingEnSeqParams)},                 /* SERDES_TX_TRAINING_ENABLE_SEQ        */
    {txtrainingDisSeqParams,                MV_SEQ_SIZE(txtrainingDisSeqParams)},                /* SERDES_TX_TRAINING_DISABLE_SEQ       */
    {speed_12_5G_SeqParams,                 MV_SEQ_SIZE(speed_12_5G_SeqParams)},                 /* SERDES_SPEED_12_5G_SEQ               */
    {speed_3_3G_SeqParams,                  MV_SEQ_SIZE(speed_3_3G_SeqParams)},                  /* SERDES_SPEED_3_3G_SEQ                */
    {speed_11_5625G_SeqParams,              MV_SEQ_SIZE(speed_11_5625G_SeqParams)},              /* SERDES_SPEED_11_5625G_SEQ            */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SERDES_PARTIAL_POWER_UP_SEQ   */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SPEED_11_25G_SEQ              */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_CORE_RESET_SEQ                */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_CORE_UNRESET_SEQ              */
    {ffeTableLrSeqParams,                   MV_SEQ_SIZE(ffeTableLrSeqParams)},                   /* SERDES_FFE_TABLE_LR_SEQ              */
    {ffeTableSrSeqParams,                   MV_SEQ_SIZE(ffeTableSrSeqParams)},                   /* SERDES_FFE_TABLE_SR_SEQ              */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SPEED_10_9375G_SEQ            */
    {speed_12_1875G_SeqParams,              MV_SEQ_SIZE(speed_12_1875G_SeqParams)},              /* SERDES_SPEED_12_1875G_SEQ            */
    {NULL,                                  MV_SEQ_SIZE(0)},                                     /* SERDES_SPEED_5_625G_SEQ              */
    {NULL,                                  MV_SEQ_SIZE(0)}                                      /* SERDES_SPEED_5_15625G_SEQ            */
};

GT_STATUS hwsComPhyHRev2SeqGet(MV_HWS_COM_PHY_H_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if (seqType >= MV_SERDES_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  if (hwsSerdesRev2SeqDb[seqType].cfgSeq == NULL)
  {
      /* check prev version */
      if (hwsSerdesSeqDb[seqType].cfgSeq == NULL)
      {
          return GT_NOT_INITIALIZED;
      }
      return hwsComPhyHSeqGet(seqType, seqLine, lineNum);
  }

  *seqLine = hwsSerdesRev2SeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}

void hwsChange1GSpeedCfgV43(void)
{
    speed_1_25G_SeqParams[/*4 */6 ].params.operData = 0x0DFF;
    speed_1_25G_SeqParams[/*11*/13].params.operData = 0xb0b4;
    speed_1_25G_SeqParams[/*12*/14].params.operData = 0x2494;
    speed_1_25G_SeqParams[/*13*/15].params.operData = 0x2094;
    speed_1_25G_SeqParams[/*14*/16].params.operData = 0x03a0;
    speed_1_25G_SeqParams[/*16*/18].params.operData = 0x3070;
    speed_1_25G_SeqParams[/*28*/30].params.operData = 0x0B28;
}

void hwsChangeSpeedCfgParams(MV_SERDES_SPEED speed, GT_U32 regOffset, GT_U32 regValue)
{
    GT_U32 i;
    GT_U32 speedSize;
    MV_CFG_ELEMENT *speedParams;

    speedParams = hwsSerdesRev2SeqDb[speed].cfgSeq;
    speedSize   = hwsSerdesRev2SeqDb[speed].cfgSeqSize;

    for (i = 0; i < speedSize; i++)
    {
        if (regOffset != speedParams[i].params.regOffset)
            continue;

        speedParams[i].params.operData = regValue;
        return;
    }
    return;
}

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
    GT_U32  offset,
    GT_U32  data,
    GT_U32  dataMask
)
{
    GT_U32 i;

    if (hwsSerdesRev2SeqDb[seqId].cfgSeq == NULL)
    {
        return GT_NOT_SUPPORTED;
    }

    for (i = 0; i < hwsSerdesRev2SeqDb[seqId].cfgSeqSize; i++)
    {
        if ((hwsSerdesRev2SeqDb[seqId].cfgSeq[i].params.regOffset == offset)&&
            /* check if the mask of this entry appropriate to the write data */
            ((hwsSerdesRev2SeqDb[seqId].cfgSeq[i].params.mask & dataMask) == dataMask))
        {
            /* reset data bits */
            hwsSerdesRev2SeqDb[seqId].cfgSeq[i].params.operData &= ~dataMask;
            /* update data value */
            hwsSerdesRev2SeqDb[seqId].cfgSeq[i].params.operData |= data;

            return GT_OK;
        }
    }

    return GT_NO_SUCH;
}
