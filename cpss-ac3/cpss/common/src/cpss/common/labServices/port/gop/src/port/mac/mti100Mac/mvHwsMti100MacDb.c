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
* @file mvHwsMti100MacDb.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacDb.h>

static MV_CFG_ELEMENT xlgMti100MacModeSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_TX_IPG_LENGTH,           0x4FFF0000, 0xFFFF0000}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_COMMAND_CONFIG,          0x2810,     0x2850}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_TX_FIFO_SECTIONS,        0xA,        0xFFFF}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_XIF_MODE,                0x20,       0x21}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_FRM_LENGTH,              0x3178,     0xFFFF}}
};

static MV_CFG_ELEMENT xlgMti100MacPowerDownSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_TX_IPG_LENGTH,           0x0,        0xFFFF0000}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_COMMAND_CONFIG,          0x800,      0x2850}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_TX_FIFO_SECTIONS,        0x8,        0xFFFF}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_RX_FIFO_SECTIONS,        0x8,        0xFFFF}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_XIF_MODE,                0x0,        0x21}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_FRM_LENGTH,              0x600,      0xFFFF}}
};

#if 0
static MV_CFG_ELEMENT xlgMti100MacResetSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_COMMAND_CONFIG,          0x0,        0x3}},
};
#endif
static MV_CFG_ELEMENT xlgMti100MacUnresetSeqParams[] = {
    /* dummy access to MTI MAC in order to "connect" register file and rtl logic,
       should be done for UNRESET sequence for MAC64 */
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_SCRATCH,                 0x0,        0x1}}
};


static MV_CFG_ELEMENT xgMti100MacModeSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_TX_IPG_LENGTH,           0x4FFF0000, 0xFFFF0000}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_COMMAND_CONFIG,          0x2810,     0x2850}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_TX_FIFO_SECTIONS,        0xA,        0xFFFF}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_XIF_MODE,                0x21,       0x21}},
    {WRITE_OP, {RAVEN_UNIT_MTI_MAC100,   MTI_MAC_FRM_LENGTH,              0x3178,     0xFFFF}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsMti100MacSeqDb[] =
{
    {xlgMti100MacModeSeqParams,     MV_SEQ_SIZE(xlgMti100MacModeSeqParams)},          /* MTI100_MAC_XLG_MODE_SEQ */
    {xlgMti100MacPowerDownSeqParams,MV_SEQ_SIZE(xlgMti100MacPowerDownSeqParams)},     /* MTI100_MAC_XLG_POWER_DOWN_SEQ */
#if 0
    {xlgMti100MacResetSeqParams,    MV_SEQ_SIZE(xlgMti100MacResetSeqParams)},         /* MTI100_MAC_XLG_RESET_SEQ */
#endif
    {xlgMti100MacUnresetSeqParams,  MV_SEQ_SIZE(xlgMti100MacUnresetSeqParams)},       /* MTI100_MAC_XLG_UNRESET_SEQ */
    {xgMti100MacModeSeqParams,      MV_SEQ_SIZE(xgMti100MacModeSeqParams)},           /* MTI100_MAC_XG_MODE_SEQ */
};

GT_STATUS hwsMti100MacSeqGet(HWS_MTI_MAC100_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MTI100_MAC_LAST_SEQ) ||
      (hwsMti100MacSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMti100MacSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
