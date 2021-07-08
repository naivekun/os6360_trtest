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
* @file mvHwsCgMacDb.c
*
* @brief
*
* @version   7
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacDb.h>

static MV_CFG_ELEMENT modeSeqParams[] = {
    {WRITE_OP, {CG_UNIT, CG_RXDMA_CONVERTER_PACKET_SIZE_RESTRICTION, 0x40,       0x3FFF}},
    {WRITE_OP, {CG_UNIT, CG_CREDIT_LIMIT,                            0x1c07050,  0x0}},
    {WRITE_OP, {CG_UNIT, CG_INIT_CREDIT,                             0x1C,       0x0}},
    {WRITE_OP, {CG_UNIT, CG_TX_FIFO_SECTIONS,                        0x6,        0x0}},
    {WRITE_OP, {CG_UNIT, CG_COMMAND_CONFIG,                          0xd3,       0x0}},
    {WRITE_OP, {CG_UNIT, CG_COMMAND_CONFIG,                          0x200d0,    0x0}},
    {WRITE_OP, {CG_UNIT, CG_FRM_LENGTH,                              0x2820,     0x0}}
};

static MV_CFG_ELEMENT resetSeqParams[] = {
    {WRITE_OP, {CG_UNIT, CG_COMMAND_CONFIG,                          0x0,        0x3}},
    {WRITE_OP, {CG_UNIT, CG_RESETS,                                  0x0,        0x0}}
};

static MV_CFG_ELEMENT unresetSeqParams[] = {
    {WRITE_OP, {CG_UNIT, CG_RESETS,                                  0x6FFFFFF,  0x0}},
    {WRITE_OP, {CG_UNIT, CG_RESETS,                                  0x1FFFFFFF, 0x0}},
    {WRITE_OP, {CG_UNIT, CG_CONTROL_0,                               0x100000,   0x100000}},
    {WRITE_OP, {CG_UNIT, CG_COMMAND_CONFIG,                          0x3,        0x3}}
};

static MV_CFG_ELEMENT lbNormalSeqParams[] = {
    {WRITE_OP, {CG_UNIT, CG_COMMAND_CONFIG,                          0x0,        0x400}}
};

static MV_CFG_ELEMENT lbTx2RxSeqParams[] = {
    {WRITE_OP, {CG_UNIT, CG_COMMAND_CONFIG,                          0x400,      0x400}}
};

MV_CFG_SEQ hwsCgMacSeqDb[MV_MAC_CG_LAST_SEQ] = {
    {unresetSeqParams,  MV_SEQ_SIZE(unresetSeqParams) }, /* CGMAC_UNRESET_SEQ            */
    {resetSeqParams,    MV_SEQ_SIZE(resetSeqParams)   }, /* CGMAC_RESET_SEQ              */
    {modeSeqParams,     MV_SEQ_SIZE(modeSeqParams)    }, /* CGMAC_MODE_SEQ               */
    {lbNormalSeqParams, MV_SEQ_SIZE(lbNormalSeqParams)}, /* CGMAC_LPBK_NORMAL_SEQ        */
    {lbTx2RxSeqParams,  MV_SEQ_SIZE(lbTx2RxSeqParams) }, /* CGMAC_LPBK_TX2RX_SEQ         */
    {NULL,              0                             }, /* CGMAC_XLGMAC_LPBK_NORMAL_SEQ */
    {NULL,              0                             }, /* CGMAC_XLGMAC_LPBK_RX2TX_SEQ  */
    {NULL,              0                             }, /* CGMAC_XLGMAC_LPBK_TX2RX_SEQ  */
    {NULL,              0                             }, /* CGMAC_MODE_1_LANE_SEQ        */
    {NULL,              0                             }, /* CGMAC_MODE_2_LANE_SEQ        */
    {NULL,              0                             }, /* CGMAC_MODE_4_LANE_SEQ        */
    {NULL,              0                             }, /* CGMAC_POWER_DOWN_1_LANE_SEQ  */
    {NULL,              0                             }, /* CGMAC_POWER_DOWN_2_LANE_SEQ  */
    {NULL,              0                             }, /* CGMAC_POWER_DOWN_4_LANE_SEQ  */
    {NULL,              0                             }, /* CGMAC_FC_DISABLE_SEQ         */
    {NULL,              0                             }, /* CGMAC_FC_BOTH_SEQ            */
    {NULL,              0                             }, /* CGMAC_FC_RX_ONLY_SEQ         */
    {NULL,              0                             }  /* CGMAC_FC_TX_ONLY_SEQ         */
};

#ifndef MV_HWS_FREE_RTOS

GT_STATUS hwsCgMacSeqGet(MV_HWS_CG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_MAC_CG_LAST_SEQ) ||
      (hwsCgMacSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsCgMacSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}

#endif /*MV_HWS_FREE_RTOS*/

