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
* mvHwsXlgMacDb.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDbRev3.h>

static MV_CFG_ELEMENT mode1LaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3, 0x2000, 0xE000}},
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4, 0x210,  0xF10}}
};

static MV_CFG_ELEMENT mode2LaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3, 0x4000, 0xE000}},
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4, 0x410,  0xF10}}
};

static MV_CFG_ELEMENT mode4LaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3, 0x6000, 0xE000}},
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4, 0x410,  0xF10}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsXlgMacSeqDbRev3[MV_MAC_XLG_LAST_SEQ] =
{
     {NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_UNRESET_SEQ     */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_RESET_SEQ       */
    ,{mode1LaneSeqParams, MV_SEQ_SIZE(mode1LaneSeqParams)} /* XLGMAC_MODE_1_Lane_SEQ */
    ,{mode2LaneSeqParams, MV_SEQ_SIZE(mode2LaneSeqParams)} /* XLGMAC_MODE_2_Lane_SEQ */
    ,{mode4LaneSeqParams, MV_SEQ_SIZE(mode4LaneSeqParams)} /* XLGMAC_MODE_4_Lane_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_LPBK_NORMAL_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_LPBK_RX2TX_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_LPBK_TX2RX_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_FC_DISABLE_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_FC_BOTH_SEQ     */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_FC_RX_ONLY_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_FC_TX_ONLY_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_POWER_DOWN_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_1_Lane_25G_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_2_Lane_50G_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_1_Lane_25G_RS_FEC_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_2_Lane_50G_RS_FEC_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_4_Lane_MLG_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_RESET_4_Lane_MLG_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_UNRESET_4_Lane_MLG_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_POWER_DOWN_4_Lane_MLG_SEQ */
};

GT_STATUS hwsXlgMacRev3SeqGet(MV_HWS_XLG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_MAC_XLG_LAST_SEQ) ||
      (hwsXlgMacSeqDbRev3[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsXlgMacSeqDbRev3[seqType].cfgSeq[lineNum];
  return GT_OK;
}
