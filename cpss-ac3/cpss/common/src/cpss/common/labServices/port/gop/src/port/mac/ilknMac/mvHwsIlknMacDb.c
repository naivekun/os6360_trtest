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
* @file mvHwsIlknMacDb.c
*
* @brief
*
* @version   7
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/ilknMac/mvHwsIlknMacDb.h>

#ifndef CO_CPU_RUN

static MV_CFG_ELEMENT mode4LanesSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    RXDMA_converter_control_0, 0x41,       0x57}},
    {WRITE_OP, {INTLKN_UNIT,    TXDMA_converter_control_0, 0x15009,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_0,          0x0,        0x7700}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_0,        0x100F0F0,  0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_1,        0x1FF00,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_2,          0x15000100, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_3,          0x303,      0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_4,          0x60303,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_6,          0x11030000, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,            0x2,        0x2}}
};

static MV_CFG_ELEMENT mode8LanesSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    RXDMA_converter_control_0, 0x42,       0x57}},
    {WRITE_OP, {INTLKN_UNIT,    TXDMA_converter_control_0, 0x1500A,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_0,          0x1100,     0x7700}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_0,        0x100F0F0,  0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_1,        0x1FF00,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_2,          0x15000100, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_3,          0x707,      0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_4,          0x60303,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_6,          0x11030000, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,            0x2,        0x2}}
};

static MV_CFG_ELEMENT mode12LanesSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    RXDMA_converter_control_0, 0x3,        0x7}},
    {WRITE_OP, {INTLKN_UNIT,    TXDMA_converter_control_0, 0x3,        0x7}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_0,          0x2200,     0x7700}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_0,        0x100F0F0,  0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_1,        0x1FF00,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_2,          0x15000100, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_3,          0xB0B,      0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_4,          0x20301,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_6,          0x11030000, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,            0x2,        0x2}}
};

static MV_CFG_ELEMENT mode16LanesSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    RXDMA_converter_control_0, 0x4,        0x7}},
    {WRITE_OP, {INTLKN_UNIT,    TXDMA_converter_control_0, 0x4,        0x7}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_0,          0x3300,     0x7700}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_0,        0x100F0F0,  0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_1,        0x1FF00,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_2,          0x15000100, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_3,          0xF0F,      0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_4,          0x20301,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_6,          0x11030000, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,            0x2,        0x2}}
};

static MV_CFG_ELEMENT mode24LanesSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    RXDMA_converter_control_0, 0x6,        0x7}},
    {WRITE_OP, {INTLKN_UNIT,    TXDMA_converter_control_0, 0x6,        0x7}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_0,          0x5500,     0x7700}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_0,        0x100F0F0,  0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_CH_FC_CFG_1,        0x1FF00,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_2,          0x15000100, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_3,          0x1717,     0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_4,          0x20301,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_MAC_CFG_6,          0x11030000, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,            0x2,        0x2}}
};

static MV_CFG_ELEMENT resetSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    ILKN_RESETS,               0x0,        0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_EN,                 0x0,        0x3}}
};

static MV_CFG_ELEMENT unresetSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    ILKN_RESETS,               0x2F,       0x0}},
    {WRITE_OP, {INTLKN_UNIT,    ILKN_RESETS,               0x3F,       0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_EN,                 0x3,        0x3}},
    {WRITE_OP, {INTLKN_UNIT,    GENX_converter_control_0,  0x1,        0x1}},
    {WRITE_OP, {INTLKN_UNIT,    GENX_converter_control_0,  0x0,        0x1}}
};

static MV_CFG_ELEMENT lbNormalSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    RXDMA_converter_control_0, 0x0,        0x20}}
};

static MV_CFG_ELEMENT lbTx2RxSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    RXDMA_converter_control_0, 0x20,       0x20}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsIlknMacSeqDb[] =
{
    {unresetSeqParams,     MV_SEQ_SIZE(unresetSeqParams)},      /* INTLKN_MAC_UNRESET_SEQ */
    {resetSeqParams,       MV_SEQ_SIZE(resetSeqParams)},        /* INTLKN_MAC_RESET_SEQ   */
    {mode12LanesSeqParams, MV_SEQ_SIZE(mode12LanesSeqParams)},  /* INTLKN_MAC_MODE_12_LAN */
    {mode16LanesSeqParams, MV_SEQ_SIZE(mode16LanesSeqParams)},  /* INTLKN_MAC_MODE_16_LAN */
    {mode24LanesSeqParams, MV_SEQ_SIZE(mode24LanesSeqParams)},  /* INTLKN_MAC_MODE_24_LAN */
    {lbNormalSeqParams,    MV_SEQ_SIZE(lbNormalSeqParams)},     /* INTLKN_MAC_LPBK_NORMAL */
    {lbTx2RxSeqParams,     MV_SEQ_SIZE(lbTx2RxSeqParams)},      /* INTLKN_MAC_LPBK_TX2RX_ */
    {mode8LanesSeqParams,  MV_SEQ_SIZE(mode8LanesSeqParams)},   /* INTLKN_MAC_MODE_4_LANE */
    {mode4LanesSeqParams,  MV_SEQ_SIZE(mode4LanesSeqParams)},   /* INTLKN_MAC_MODE_8_LANE */
};

GT_STATUS hwsIlknMacSeqGet(MV_HWS_INTKLN_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_MAC_INTKLN_LAST_SEQ) ||
      (hwsIlknMacSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsIlknMacSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}

#endif

