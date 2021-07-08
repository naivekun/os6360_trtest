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
* mvHwsMMPcsDb.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDbRev3.h>

#ifndef CO_CPU_RUN

static MV_CFG_ELEMENT resetLane1SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x840,  0xFFFF}}
};

static MV_CFG_ELEMENT unresetLane1SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x40,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x841,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x845,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x847,  0xFFFF}}
};

static MV_CFG_ELEMENT fabricResetLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x800,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x800,  0xFFFF}}
};

static MV_CFG_ELEMENT fabricUnresetLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x40,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x40,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x841,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x841,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x845,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x845,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x847,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x847,  0xFFFF}}
};

static MV_CFG_ELEMENT fabricResetLane4SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x800,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x800,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x600,          0x800,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x800,          0x800,  0xFFFF}}
};

static MV_CFG_ELEMENT fabricUnresetLane4SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x600,          0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x800,          0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x600,          0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x800,          0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x600,          0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x800,          0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x600,          0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x800,          0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x600,          0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x800,          0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x827,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x827,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x600,          0x827,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x800,          0x827,  0xFFFF}}
};

static MV_CFG_ELEMENT resetLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x800,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x800,  0xFFFF}}
};

static MV_CFG_ELEMENT unresetLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x40,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x40,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x841,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x400,          0x841,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x845,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x845,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x847,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x847,  0xFFFF}}
};

static MV_CFG_ELEMENT resetLane4SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x800,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x800,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,         0x800,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,         0x800,  0xFFFF}}
};

static MV_CFG_ELEMENT unresetLane4SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,         0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,         0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,         0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,         0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,         0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,         0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                  0x827,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,         0x827,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,         0x827,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,         0x827,  0xFFFF}}
};

static MV_CFG_ELEMENT modeLane1SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,          0x3820, 0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,          0xA0,   0xFFFF}}
};

static MV_CFG_ELEMENT modeLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,          0x3920, 0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,          0xA0,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,          0x1A0,  0xFFFF}}
};

static MV_CFG_ELEMENT modeLane4SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,          0x3A20, 0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,          0xA0,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000, 0x1A0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x2000, 0x2A0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x3000, 0x3A0,  0xFFFF}}
};

static MV_CFG_ELEMENT fabricModeLane1SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,          0x3820, 0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,          0xA0,   0xFFFF}}
};

static MV_CFG_ELEMENT fabricModeLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,          0x3920, 0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,          0xA0,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x400,  0x1A0,  0xFFFF}}
};

static MV_CFG_ELEMENT fabricModeLane4SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,          0x3A20, 0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,          0xA0,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x400,  0x1A0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x600,  0x2A0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x800,  0x3A0,  0xFFFF}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsMmPscSeqDbRev3[] =
{
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_RESET_SEQ                 */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_UNRESET_SEQ               */
    {modeLane1SeqParams,          MV_SEQ_SIZE(modeLane1SeqParams)},          /* MMPCS_MODE_1_LANE_SEQ           */
    {modeLane2SeqParams,          MV_SEQ_SIZE(modeLane2SeqParams)},          /* MMPCS_MODE_2_LANE_SEQ           */
    {modeLane4SeqParams,          MV_SEQ_SIZE(modeLane4SeqParams)},          /* MMPCS_MODE_4_LANE_SEQ           */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_LPBK_NORMAL_SEQ           */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_LPBK_RX2TX_SEQ            */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_LPBK_TX2RX_SEQ            */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_GEN_NORMAL_SEQ            */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_GEN_KRPAT_SEQ             */
    {fabricModeLane1SeqParams,    MV_SEQ_SIZE(fabricModeLane1SeqParams)},    /* MMPCS_FABRIC_MODE_1_LANE_SEQ    */
    {fabricModeLane2SeqParams,    MV_SEQ_SIZE(fabricModeLane2SeqParams)},    /* MMPCS_FABRIC_MODE_2_LANE_SEQ    */
    {fabricModeLane4SeqParams,    MV_SEQ_SIZE(fabricModeLane4SeqParams)},    /* MMPCS_FABRIC_MODE_4_LANE_SEQ    */
    {resetLane1SeqParams,         MV_SEQ_SIZE(resetLane1SeqParams)},         /* MMPCS_RESET_1_LANE_SEQ          */
    {resetLane2SeqParams,         MV_SEQ_SIZE(resetLane2SeqParams)},         /* MMPCS_RESET_2_LANE_SEQ          */
    {resetLane4SeqParams,         MV_SEQ_SIZE(resetLane4SeqParams)},         /* MMPCS_RESET_4_LANE_SEQ          */
    {unresetLane1SeqParams,       MV_SEQ_SIZE(unresetLane1SeqParams)},       /* MMPCS_UNRESET_1_LANE_SEQ        */
    {unresetLane2SeqParams,       MV_SEQ_SIZE(unresetLane2SeqParams)},       /* MMPCS_UNRESET_2_LANE_SEQ        */
    {unresetLane4SeqParams,       MV_SEQ_SIZE(unresetLane4SeqParams)},       /* MMPCS_UNRESET_4_LANE_SEQ        */
    {fabricResetLane2SeqParams,   MV_SEQ_SIZE(fabricResetLane2SeqParams)},   /* MMPCS_FABRIC_RESET_2_LANE_SEQ   */
    {fabricResetLane4SeqParams,   MV_SEQ_SIZE(fabricResetLane4SeqParams)},   /* MMPCS_FABRIC_RESET_4_LANE_SEQ   */
    {fabricUnresetLane2SeqParams, MV_SEQ_SIZE(fabricUnresetLane2SeqParams)}, /* MMPCS_FABRIC_UNRESET_2_LANE_SEQ */
    {fabricUnresetLane4SeqParams, MV_SEQ_SIZE(fabricUnresetLane4SeqParams)}, /* MMPCS_FABRIC_UNRESET_4_LANE_SEQ */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_GOP_REV3_UNRESET_1_LANE_SEQ */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_GOP_REV3_UNRESET_2_LANE_SEQ */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_GOP_REV3_UNRESET_4_LANE_SEQ */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_POWER_DOWN_2_LANE_SEQ        */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_POWER_DOWN_4_LANE_SEQ        */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_FABRIC_POWER_DOWN_2_LANE_SEQ */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_MODE_1_LANE_25G_SEQ        */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_MODE_4_LANE_50G_SEQ        */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_RESET_1_LANE_25G_SEQ       */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_RESET_4_LANE_50G_SEQ       */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_POWER_DOWN_1_LANE_25G_SEQ  */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_POWER_DOWN_4_LANE_50G_SEQ  */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_UNRESET_1_LANE_25G_SEQ     */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_START_SEND_FAULT_MULTI_LANE_SEQ */
    {NULL,                        MV_SEQ_SIZE(0)},                            /* MMPCS_START_SEND_FAULT_MULTI_LANE_SEQ */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_STOP_SEND_FAULT_MULTI_LANE_RS_FEC_SEQ */
    {NULL,                        MV_SEQ_SIZE(0)},                           /* MMPCS_UNRESET_1_LANE_26_7G_SEQ     */
};

GT_STATUS hwsMMPcsRev3SeqGet(MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if (seqType >= MV_MM_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  if (hwsMmPscSeqDbRev3[seqType].cfgSeq == NULL)
  {
      /* check prev version */
      return hwsMMPcsSeqGet(seqType, seqLine, lineNum);
  }

  *seqLine = hwsMmPscSeqDbRev3[seqType].cfgSeq[lineNum];
  return GT_OK;
}
#endif

