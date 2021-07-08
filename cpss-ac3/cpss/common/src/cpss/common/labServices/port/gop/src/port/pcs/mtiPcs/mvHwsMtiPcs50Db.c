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
* @file mvHwsMtiPcs50Db.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>

static MV_CFG_ELEMENT mtiPcs50resetSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_CONTROL1,     /*dummy write 0x8000*/0x0000,     0x8000}}  /*CONTROL1.Reset.set(1)*/

};

static MV_CFG_ELEMENT mtiPcs50unresetSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_CONTROL1,            0x8000,     0x8000}}  /*CONTROL1.Reset.set(1)*/
   /* {RAVEN_UNIT_MTI_PCS50, MTI_PCS_CONTROL1,            0x0,     0x8000} */ /*CONTROL1.Reset.set(0)*/

};
static MV_CFG_ELEMENT mtiPcs50PowerDownSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,       0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter;*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x2}}, /*VENDOR_PCS_MODE.Disable_mld;*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1}}, /*VENDOR_PCS_MODE.Ena_clause49;*/

    /* Marker values for 100G-KR4 / 100G-KR2 (no FEC dependency) */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_1,               0x0021,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h21)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_0,               0x719d,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'h719d)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_1,               0x008e,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'h8e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_0,               0x4b59,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h4b59)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_1,               0x00e8,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'he8)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_0,               0x954d,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h954d)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_1,               0x007b,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h7b)*/

};

static MV_CFG_ELEMENT mtiPcs50XgModeSeqParams[] = { /* MTI_PCS_XG_MODE_SEQ - speed up to 25G */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x2,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(1);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
};

static MV_CFG_ELEMENT mtiPcs50Xg25RsFecModeSeqParams[] = { /* MTI_PCS_XG_25_RS_FEC_MODE_SEQ - 25G with rs fec*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(0x4FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_1,               0x0021,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h21)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'he6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_1,               0x009b,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_1,               0x003d,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h3d)*/

};

static MV_CFG_ELEMENT mtiPcs50Xlg50R1ModeSeqParams[] = { /* speed  50G r1*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_0,               0x7690,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_1,               0x0047,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h47)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'he6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_1,               0x009b,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_1,               0x003d,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h3d)*/
};

static MV_CFG_ELEMENT mtiPcs50Xlg50R2RsFecModeSeqParams[] = { /* speed  50G r2 rs fec 528*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_0,               0x7690,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_1,               0x0047,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h47)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'he6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_1,               0x009b,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_1,               0x003d,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h3d)*/
};

static MV_CFG_ELEMENT mtiPcs50XlgModeSeqParams[] = { /* speed  40G,50G */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,     0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_0,               0x7690,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL0_1,               0x0047,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h47)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'he6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL2_1,               0x009b,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_VL3_1,               0x003d,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h3d)*/
};

static MV_CFG_ELEMENT mtiPcs50LbNormalSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_CONTROL1,               0x0,       0x4000}}
};

static MV_CFG_ELEMENT mtiPcs50LbTx2RxSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS50, MTI_PCS_CONTROL1,               0x4000,    0x4000}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsMtiPcs50SeqDb[] =
{
    {mtiPcs50resetSeqParams,            MV_SEQ_SIZE(mtiPcs50resetSeqParams)},           /* MTI_PCS_RESET_SEQ            */
    {mtiPcs50unresetSeqParams,          MV_SEQ_SIZE(mtiPcs50unresetSeqParams)},         /* MTI_PCS_UNRESET_SEQ          */
    {mtiPcs50PowerDownSeqParams,        MV_SEQ_SIZE(mtiPcs50PowerDownSeqParams)},       /* MTI_PCS_POWER_DOWN_SEQ       */

    {mtiPcs50XgModeSeqParams,           MV_SEQ_SIZE(mtiPcs50XgModeSeqParams)},          /* MTI_PCS_XG_MODE_SEQ     */
    {mtiPcs50Xg25RsFecModeSeqParams,    MV_SEQ_SIZE(mtiPcs50Xg25RsFecModeSeqParams)},   /* MTI_PCS_XG_25_RS_FEC_MODE_SEQ */

    {mtiPcs50Xlg50R1ModeSeqParams,      MV_SEQ_SIZE(mtiPcs50Xlg50R1ModeSeqParams)},     /* MTI_PCS_XLG_50R1_MODE_SEQ    */
    {mtiPcs50Xlg50R2RsFecModeSeqParams, MV_SEQ_SIZE(mtiPcs50Xlg50R2RsFecModeSeqParams)},/* MTI_PCS_XLG_50R2_RS_FEC_MODE_SEQ    */
    {mtiPcs50XlgModeSeqParams,          MV_SEQ_SIZE(mtiPcs50XlgModeSeqParams)},         /* MTI_PCS_XlG_MODE_SEQ    */

    {NULL,                              MV_SEQ_SIZE(NULL)},                             /* MTI_PCS_CG_MODE_SEQ     */

    {NULL,                              MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200R4_MODE_SEQ     */
    {NULL,                              MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200R8_MODE_SEQ     */
    {NULL,                              MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_400R8_MODE_SEQ     */

    {NULL,                              MV_SEQ_SIZE(NULL)},                             /* MTI_PCS_START_SEND_FAULT_SEQ */
    {NULL,                              MV_SEQ_SIZE(NULL)},                             /* MTI_PCS_STOP_SEND_FAULT_SEQ  */
    {mtiPcs50LbNormalSeqParams,         MV_SEQ_SIZE(mtiPcs50LbNormalSeqParams)},        /* MTI_PCS_LPBK_NORMAL_SEQ      */
    {mtiPcs50LbTx2RxSeqParams,          MV_SEQ_SIZE(mtiPcs50LbTx2RxSeqParams)},         /* MTI_PCS_LPBK_TX2RX_SEQ       */

};

GT_STATUS hwsMtiPcs50SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if (seqType >= MV_MTI_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMtiPcs50SeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
