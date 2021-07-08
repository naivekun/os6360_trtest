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
* @file mvHwsXPcsDb.c
*
* @brief
*
* @version   12
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsDb.h>

static MV_CFG_ELEMENT resetSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg0,  0x0,          0x1}}
};

static MV_CFG_ELEMENT unresetSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg0,  0x1,          0x1}}
};

static MV_CFG_ELEMENT modeMiscSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane0_configuration_Reg,    0x0,         (1 << 6)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane1_configuration_Reg,    0x0,         (1 << 6)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane2_configuration_Reg,    0x0,         (1 << 6)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane3_configuration_Reg,    0x0,         (1 << 6)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane4_configuration_Reg,    0x0,         (1 << 6)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane5_configuration_Reg,    0x0,         (1 << 6)}}
};

static MV_CFG_ELEMENT modeLane1SeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg0,     0x0,        (3 << 5)}}
};

static MV_CFG_ELEMENT modeLane2SeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg0,     (4 << 3),   (0xF << 3)}}
};

static MV_CFG_ELEMENT modeLane4SeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg0,     (8 << 3),   (0xF << 3)}}
};

static MV_CFG_ELEMENT lbNormalSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg1,  0x0,         (3 << 1)}}
};

static MV_CFG_ELEMENT lbRx2TxSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg1, (2 << 1),     (3 << 1)}}
};

static MV_CFG_ELEMENT lbTx2RxSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg1, (1 << 1),     (3 << 1)}}
};

static MV_CFG_ELEMENT genPrbs7SeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane0_configuration_Reg,   (0x17 << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane1_configuration_Reg,   (0x17 << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane2_configuration_Reg,   (0x17 << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane3_configuration_Reg,   (0x17 << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane4_configuration_Reg,   (0x17 << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane5_configuration_Reg,   (0x17 << 11), (0x1F << 11)}}
};

static MV_CFG_ELEMENT genPrbs23SeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane0_configuration_Reg,   (0x1B << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane1_configuration_Reg,   (0x1B << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane2_configuration_Reg,   (0x1B << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane3_configuration_Reg,   (0x1B << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane4_configuration_Reg,   (0x1B << 11), (0x1F << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane5_configuration_Reg,   (0x1B << 11), (0x1F << 11)}}
};

static MV_CFG_ELEMENT xpcsGenCjpatSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg1, (0x1 << 10),  (0x1 << 10)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane0_configuration_Reg,   (0x1 << 7),   (0x1 << 7)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane1_configuration_Reg,   (0x1 << 7),   (0x1 << 7)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane2_configuration_Reg,   (0x1 << 7),   (0x1 << 7)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane3_configuration_Reg,   (0x1 << 7),   (0x1 << 7)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane4_configuration_Reg,   (0x1 << 7),   (0x1 << 7)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane5_configuration_Reg,   (0x1 << 7),   (0x1 << 7)}}
};

static MV_CFG_ELEMENT xpcsGenCrpatSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg1, (0x1 << 11),  (0x1 << 11)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane0_configuration_Reg,   (0x1 << 8),   (0x1 << 8)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane1_configuration_Reg,   (0x1 << 8),   (0x1 << 8)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane2_configuration_Reg,   (0x1 << 8),   (0x1 << 8)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane3_configuration_Reg,   (0x1 << 8),   (0x1 << 8)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane4_configuration_Reg,   (0x1 << 8),   (0x1 << 8)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane5_configuration_Reg,   (0x1 << 8),   (0x1 << 8)}}
};

static MV_CFG_ELEMENT genNormalSeqParams[] = {
    {WRITE_OP, {XPCS_UNIT, XPCS_Global_Configuration_Reg1, (0 << 10),    (3 << 10)}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane0_configuration_Reg,    0x0,          0xF980}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane1_configuration_Reg,    0x0,          0xF980}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane2_configuration_Reg,    0x0,          0xF980}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane3_configuration_Reg,    0x0,          0xF980}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane4_configuration_Reg,    0x0,          0xF980}},
    {WRITE_OP, {XPCS_UNIT, XPCS_Lane5_configuration_Reg,    0x0,          0xF980}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsXpcsPscSeqDb[] =
{
    {resetSeqParams,        MV_SEQ_SIZE(resetSeqParams)},        /* XPCS_RESET_SEQ       */
    {unresetSeqParams,      MV_SEQ_SIZE(unresetSeqParams)},      /* XPCS_UNRESET_SEQ     */
    {modeMiscSeqParams,     MV_SEQ_SIZE(modeMiscSeqParams)},     /* XPCS_MODE_MISC_SEQ   */
    {modeLane1SeqParams,    MV_SEQ_SIZE(modeLane1SeqParams)},    /* XPCS_MODE_1_LANE_SEQ */
    {modeLane2SeqParams,    MV_SEQ_SIZE(modeLane2SeqParams)},    /* XPCS_MODE_2_LANE_SEQ */
    {modeLane4SeqParams,    MV_SEQ_SIZE(modeLane4SeqParams)},    /* XPCS_MODE_4_LANE_SEQ */
    {lbNormalSeqParams,     MV_SEQ_SIZE(lbNormalSeqParams)},     /* XPCS_LPBK_NORMAL_SEQ */
    {lbRx2TxSeqParams,      MV_SEQ_SIZE(lbRx2TxSeqParams)},      /* XPCS_LPBK_RX2TX_SEQ  */
    {lbTx2RxSeqParams,      MV_SEQ_SIZE(lbTx2RxSeqParams)},      /* XPCS_LPBK_TX2RX_SEQ  */
    {genPrbs7SeqParams,     MV_SEQ_SIZE(genPrbs7SeqParams)},     /* XPCS_GEN_PRBS7_SEQ   */
    {genPrbs23SeqParams,    MV_SEQ_SIZE(genPrbs23SeqParams)},    /* XPCS_GEN_PRBS23_SEQ  */
    {xpcsGenCjpatSeqParams, MV_SEQ_SIZE(xpcsGenCjpatSeqParams)}, /* XPCS_GEN_CJPAT_SEQ   */
    {xpcsGenCrpatSeqParams, MV_SEQ_SIZE(xpcsGenCrpatSeqParams)}, /* XPCS_GEN_CRPAT_SEQ   */
    {genNormalSeqParams,    MV_SEQ_SIZE(genNormalSeqParams)}     /* XPCS_GEN_NORMAL_SEQ  */
};

GT_STATUS hwsXPscSeqInit(void)
{
    return GT_OK;
}

GT_STATUS hwsXPcsSeqGet(MV_HWS_XPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_XPCS_LAST_SEQ) ||
      (hwsXpcsPscSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsXpcsPscSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}


