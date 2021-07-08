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
* @file mvHwsMMPcsDb.c
*
* @brief
*
* @version   21
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDb.h>

#define MMPCS_IND_OFFSET 0x1000

static MV_CFG_ELEMENT resetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                 0x0,        0x7F}}
};

static MV_CFG_ELEMENT unresetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                 0x1,        0x1}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                 0x2,        0x2}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                 0x4,        0x4}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                 0x20,       0x20}}
};

static MV_CFG_ELEMENT mmpcsLane1ModeSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                         0x0,       (3 << 8) + (1 << 7)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                        (1 << 14),  (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                (4 << 7),   (0xF << 7)}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_REMOVAL_THRESHOLDS,                   (0xA << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_INSERTION_THRESHOLDS,                 (0x6 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_REMOVAL_THRESHOLDS,                   (0xB << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_INSERTION_THRESHOLDS,                 (0x8 << 8),  0xFF00}}
};

static MV_CFG_ELEMENT mmpcsLane2ModeSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                        (1 << 8),   (3 << 8) + (1 << 7)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + MMPCS_IND_OFFSET,      0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + MMPCS_IND_OFFSET,     (1 << 8),   (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                (2 << 7),   (0xF << 7)}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_REMOVAL_THRESHOLDS,                   (0x9 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_INSERTION_THRESHOLDS,                 (0x5 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_REMOVAL_THRESHOLDS,                   (0xB << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_INSERTION_THRESHOLDS,                 (0x8 << 8),  0xFF00}}
};

static MV_CFG_ELEMENT mmpcsLane4ModeSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                        (2 << 8),   (3 << 8) + (1 << 7)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION +     MMPCS_IND_OFFSET,  0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 2 * MMPCS_IND_OFFSET,  0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 3 * MMPCS_IND_OFFSET,  0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION                       ,  0x0,       (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION +     MMPCS_IND_OFFSET, (1 << 8),   (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 2 * MMPCS_IND_OFFSET, (2 << 8),   (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 3 * MMPCS_IND_OFFSET, (3 << 8),   (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                (1 << 7),   (0xF << 7)}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_REMOVAL_THRESHOLDS,                   (0x8 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_INSERTION_THRESHOLDS,                 (0x4 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_REMOVAL_THRESHOLDS,                   (0xC << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_INSERTION_THRESHOLDS,                 (0x8 << 8),  0xFF00}}
};

static MV_CFG_ELEMENT lbNormalSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                         0x0,       (3 << 3) + (1 << 7)}}
};

static MV_CFG_ELEMENT lbTx2RxSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                        (1 << 3),   (3 << 3) + (1 << 7)}}
};

static MV_CFG_ELEMENT lbRx2TxSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                        (2 << 3),   (3 << 3) + (1 << 7)}}
};

static MV_CFG_ELEMENT genNormalSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, TEST_PATTERN_CONFIGURATION,                    0x0,       (1 << 5)}},
    {WRITE_OP, {MMPCS_UNIT, TEST_PATTERN_CONFIGURATION,                    0x0,        0x3}}
};

static MV_CFG_ELEMENT genKrpatSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, TEST_PATTERN_CONFIGURATION,                   (1 << 5),   (1 << 5)}},
    {WRITE_OP, {MMPCS_UNIT, TEST_PATTERN_CONFIGURATION,                   (2 << 2),   (3 << 2)}},
    {WRITE_OP, {MMPCS_UNIT, TEST_PATTERN_CONFIGURATION,                    0x3,        0x3}}
};

static MV_CFG_ELEMENT fabricModeLane1SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                         0x0,       (3 << 8) + (1 << 7)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                        (1 << 14),  (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x400,      0x400}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                (4 << 7),   (0xF << 7)}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_REMOVAL_THRESHOLDS,                   (0xA << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_INSERTION_THRESHOLDS,                 (0x6 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_REMOVAL_THRESHOLDS,                   (0xB << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_INSERTION_THRESHOLDS,                 (0x8 << 8),  0xFF00}}
};

static MV_CFG_ELEMENT fabricModeLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                        (1 << 8),   (3 << 8) + (1 << 7)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x400,                 0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x400,                (1 << 8),   (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                        (1 << 10),  (1 << 10)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x400,                (1 << 10),  (1 << 10)}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                (2 << 7),   (0xF << 7)}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_REMOVAL_THRESHOLDS,                   (0x9 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_INSERTION_THRESHOLDS,                 (0x5 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_REMOVAL_THRESHOLDS,                   (0xB << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_INSERTION_THRESHOLDS,                 (0x8 << 8),  0xFF00}}
};

static MV_CFG_ELEMENT fabricModeLane4SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                        (2 << 8),   (3 << 8) + (1 << 7)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x400,                 0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x600,                 0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x800,                 0x0,       (1 << 14)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                         0x0,       (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x400,                (1 << 8),   (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x600,                (2 << 8),   (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x800,                (3 << 8),   (3 << 8)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                        (1 << 10),  (1 << 10)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x400,                (1 << 10),  (1 << 10)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x600,                (1 << 10),  (1 << 10)}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x800,                (1 << 10),  (1 << 10)}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                (1 << 7),   (0xF << 7)}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_REMOVAL_THRESHOLDS,                   (0x8 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, TX_IDLE_INSERTION_THRESHOLDS,                 (0x4 << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_REMOVAL_THRESHOLDS,                   (0xC << 8),  0xFF00}},
    {WRITE_OP, {MMPCS_UNIT, RX_IDLE_INSERTION_THRESHOLDS,                 (0x8 << 8),  0xFF00}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsMmPscSeqDb[] =
{
    {resetSeqParams,           MV_SEQ_SIZE(resetSeqParams)},           /* MMPCS_RESET_SEQ                 */
    {unresetSeqParams,         MV_SEQ_SIZE(unresetSeqParams)},         /* MMPCS_UNRESET_SEQ               */
    {mmpcsLane1ModeSeqParams,  MV_SEQ_SIZE(mmpcsLane1ModeSeqParams)},  /* MMPCS_MODE_1_LANE_SEQ           */
    {mmpcsLane2ModeSeqParams,  MV_SEQ_SIZE(mmpcsLane2ModeSeqParams)},  /* MMPCS_MODE_2_LANE_SEQ           */
    {mmpcsLane4ModeSeqParams,  MV_SEQ_SIZE(mmpcsLane4ModeSeqParams)},  /* MMPCS_MODE_4_LANE_SEQ           */
    {lbNormalSeqParams,        MV_SEQ_SIZE(lbNormalSeqParams)},        /* MMPCS_LPBK_NORMAL_SEQ           */
    {lbRx2TxSeqParams,         MV_SEQ_SIZE(lbRx2TxSeqParams)},         /* MMPCS_LPBK_RX2TX_SEQ            */
    {lbTx2RxSeqParams,         MV_SEQ_SIZE(lbTx2RxSeqParams)},         /* MMPCS_LPBK_TX2RX_SEQ            */
    {genNormalSeqParams,       MV_SEQ_SIZE(genNormalSeqParams)},       /* MMPCS_GEN_NORMAL_SEQ            */
    {genKrpatSeqParams,        MV_SEQ_SIZE(genKrpatSeqParams)},        /* MMPCS_GEN_KRPAT_SEQ             */
    {fabricModeLane1SeqParams, MV_SEQ_SIZE(fabricModeLane1SeqParams)}, /* MMPCS_FABRIC_MODE_1_LANE_SEQ    */
    {fabricModeLane2SeqParams, MV_SEQ_SIZE(fabricModeLane2SeqParams)}, /* MMPCS_FABRIC_MODE_2_LANE_SEQ    */
    {fabricModeLane4SeqParams, MV_SEQ_SIZE(fabricModeLane4SeqParams)}, /* MMPCS_FABRIC_MODE_4_LANE_SEQ    */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_RESET_1_LANE_SEQ          */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_RESET_2_LANE_SEQ          */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_RESET_4_LANE_SEQ          */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_UNRESET_1_LANE_SEQ        */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_UNRESET_2_LANE_SEQ        */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_UNRESET_4_LANE_SEQ        */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_FABRIC_RESET_2_LANE_SEQ   */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_FABRIC_RESET_4_LANE_SEQ   */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_FABRIC_UNRESET_2_LANE_SEQ */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_FABRIC_UNRESET_4_LANE_SEQ */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_GOP_REV3_UNRESET_1_LANE_SEQ */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_GOP_REV3_UNRESET_2_LANE_SEQ */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_GOP_REV3_UNRESET_4_LANE_SEQ */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_POWER_DOWN_2_LANE_SEQ        */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_POWER_DOWN_4_LANE_SEQ        */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_FABRIC_POWER_DOWN_2_LANE_SEQ */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_MODE_1_LANE_25G_SEQ        */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_MODE_4_LANE_50G_SEQ        */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_RESET_1_LANE_25G_SEQ       */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_RESET_4_LANE_50G_SEQ       */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_POWER_DOWN_1_LANE_25G_SEQ  */
    {NULL,                     MV_SEQ_SIZE(0)},                        /* MMPCS_POWER_DOWN_4_LANE_50G_SEQ  */
    {NULL,                     MV_SEQ_SIZE(0)},                         /* MMPCS_UNRESET_1_LANE_25G_SEQ     */
    {NULL,                     MV_SEQ_SIZE(0)},                         /* MMPCS_START_SEND_FAULT_MULTI_LANE_SEQ     */
    {NULL,                     MV_SEQ_SIZE(0)},                         /* MMPCS_START_SEND_FAULT_MULTI_LANE_SEQ     */
    {NULL,                     MV_SEQ_SIZE(0)},                         /* MMPCS_STOP_SEND_FAULT_MULTI_LANE_RS_FEC_SEQ */
    {NULL,                     MV_SEQ_SIZE(0)},                         /* MMPCS_UNRESET_1_LANE_26_7G_SEQ     */

};

GT_STATUS hwsMMPcsSeqGet(MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if (seqType >= MV_MM_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMmPscSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}


