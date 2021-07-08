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
* @file mvHwsGPcsDb.c
*
* @brief
*
* @version   13
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsDb.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>

static MV_CFG_ELEMENT lbNormalSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER1,  0x0,      (1 << 6)}}
};

static MV_CFG_ELEMENT lbTx2RxSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER1, (1 << 6),  (1 << 6)}}
};

static MV_CFG_ELEMENT genPrbs7SeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER2, (3 << 10), (3 << 10)}}
};

static MV_CFG_ELEMENT genNormalSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER2,  0x0,      (3 << 10)}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsGPscSeqDb[] =
{
    {NULL,               MV_SEQ_SIZE(0)},                 /* GPCS_RESET_SEQ       */
    {NULL,               MV_SEQ_SIZE(0)},                 /* GPCS_UNRESET_SEQ     */
    {lbNormalSeqParams,  MV_SEQ_SIZE(lbNormalSeqParams)}, /* GPCS_LPBK_NORMAL_SEQ */
    {lbTx2RxSeqParams,   MV_SEQ_SIZE(lbTx2RxSeqParams)},  /* GPCS_LPBK_TX2RX_SEQ  */
    {genPrbs7SeqParams,  MV_SEQ_SIZE(genPrbs7SeqParams)}, /* GPCS_GEN_PRBS7_SEQ   */
    {genNormalSeqParams, MV_SEQ_SIZE(genNormalSeqParams)} /* GPCS_GEN_NORMAL_SEQ  */
};

GT_STATUS hwsGPscSeqInit(void)
{
    return GT_OK;
}

GT_STATUS hwsGPcsSeqGet(MV_HWS_GPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_GPCS_LAST_SEQ) ||
      (hwsGPscSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsGPscSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}



