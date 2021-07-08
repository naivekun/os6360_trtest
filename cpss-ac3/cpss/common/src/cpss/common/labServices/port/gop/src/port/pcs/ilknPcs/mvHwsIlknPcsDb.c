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
* @file mvHwsIlknPcsDb.c
*
* @brief
*
* @version   5
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/ilknPcs/mvHwsIlknPcsDb.h>

MV_CFG_SEQ hwsIlknPcsPscSeqDb[MV_ILKNPCS_LAST_SEQ];

#ifndef CO_CPU_RUN

static MV_CFG_ELEMENT resetSeqParams[] = {
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_EN,                     0x0,       0xC}}
};

static MV_CFG_ELEMENT unresetSeqParams[] = {
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_EN,                     0xC,       0xC}}
};

static MV_CFG_ELEMENT modeMiscSeqParams[] = {
    {WRITE_OP, {INTLKN_UNIT,    SD_FC_LED_converter_control_0, 0x4,       0x0}},
    {WRITE_OP, {INTLKN_UNIT,    SD_FC_LED_converter_control_0, 0xC,       0x0}},
    {WRITE_OP, {INTLKN_UNIT,    ILKN_RESETS,                   0x2,       0x2}}
};

static MV_CFG_ELEMENT modeLane4SeqParams[] = {
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_0,              0x7FF07FF, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_1,              0xF,       0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,                0x3,       0x3}}
};

static MV_CFG_ELEMENT modeLane8SeqParams[] = {
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_0,              0x7FF07FF, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_1,              0xFF,      0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,                0x3,       0x3}}
};

static MV_CFG_ELEMENT modeLane12SeqParams[] = {
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_0,              0x7FF07FF, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_1,              0xFFF,     0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,                0x3,       0x3}}
};

static MV_CFG_ELEMENT modeLane16SeqParams[] = {
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_0,              0x7FF07FF, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_1,              0xFFFF,    0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,                0x3,       0x3}}
};

static MV_CFG_ELEMENT modeLane24SeqParams[] = {
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_0,              0x7FF07FF, 0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_PCS_CFG_1,              0xFFFFFF,  0x0}},
    {WRITE_OP, {INTLKN_RF_UNIT, ILKN_0_STAT_EN,                0x3,       0x3}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsIlknPcsPscSeqDb[] =
{
    {resetSeqParams,    MV_SEQ_SIZE(resetSeqParams)},        /* ILKNPCS_RESET_SEQ        */
    {unresetSeqParams,  MV_SEQ_SIZE(unresetSeqParams)},      /* ILKNPCS_UNRESET_SEQ      */
    {modeMiscSeqParams, MV_SEQ_SIZE(modeMiscSeqParams)},     /* ILKNPCS_MODE_MISC_SEQ    */
    {modeLane12SeqParams, MV_SEQ_SIZE(modeLane12SeqParams)}, /* ILKNPCS_MODE_12_LANE_SEQ */
    {modeLane16SeqParams, MV_SEQ_SIZE(modeLane16SeqParams)}, /* ILKNPCS_MODE_16_LANE_SEQ */
    {modeLane24SeqParams, MV_SEQ_SIZE(modeLane24SeqParams)}, /* ILKNPCS_MODE_24_LANE_SEQ */
    {modeLane4SeqParams, MV_SEQ_SIZE(modeLane4SeqParams)},   /* ILKNPCS_MODE_4_LANE_SEQ  */
    {modeLane8SeqParams, MV_SEQ_SIZE(modeLane8SeqParams)}    /* ILKNPCS_MODE_8_LANE_SEQ  */
};

GT_STATUS hwsIlknPcsSeqGet(MV_HWS_ILKNPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_ILKNPCS_LAST_SEQ) ||
      (hwsIlknPcsPscSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsIlknPcsPscSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
#endif



