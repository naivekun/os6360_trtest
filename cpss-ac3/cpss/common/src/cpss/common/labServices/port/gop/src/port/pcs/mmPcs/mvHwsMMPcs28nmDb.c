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
*       $Revision: 5 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmDb.h>

#define MMPCS_LANE2_RESET_SEQ_PARAMS_MAC  \
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x800,  0xFFF7}}, \
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x800,  0xFFF7}}

#define MMPCS_LANE4_RESET_SEQ_PARAMS_MAC  \
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x800,  0xFFF7}}, \
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x800,  0xFFF7}}, \
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x800,  0xFFF7}}, \
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x800,  0xFFF7}}

#define MMPCS_FABRIC_RESET_LANE2_SEQ_PARAMS_MAC  \
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x800,  0xFFFF}}, \
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x800,  0xFFFF}}

static MV_CFG_ELEMENT mmpcsLane1ModeSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x3820, 0xFBFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0xA0,   0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsLane2ModeSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x3920, 0xFBFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0xA0,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x1A0,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsLane4ModeSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x3A20, 0xFBFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0xA0,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x1A0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x2000,        0x2A0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x3000,        0x3A0,  0xFFFF}}
};

/*  for Bobcat3 port modes: 25G, 50G  */
/* ================================== */
static MV_CFG_ELEMENT mmpcsLane1_25G_ModeSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0xB820, 0xFBFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0xA0,   0xFFFF}}
};

/* 2 x 25G, but 4 PCS lanes for FC-FEC mode */
static MV_CFG_ELEMENT mmpcsLane4_50G_ModeSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_ALIGNMENT_CYCLE_CONTROL,        0x3FFF, 0xFFFF}},

    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x600,  0x700}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x1000,        0x600,  0x700}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x2000,        0x600,  0x700}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x3000,        0x600,  0x700}},

    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x100,  0x300}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x2000,        0x200,  0x300}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x3000,        0x300,  0x300}},

    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x10,   0xF0}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x10,   0xF0}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x10,   0xF0}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x10,   0xF0}},

    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL_2,               0x1,    0x1}}
};

static MV_CFG_ELEMENT mmpcsLane1_25G_ResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsLane1_25G_PowerDownSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0,    0x8300}}
};

    /* in 50G PCS mode: 4 PCS lanes are bounded */
static MV_CFG_ELEMENT mmpcsLane4_50G_ResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x800,  0xFF07}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x800,  0xFF07}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x800,  0xFF07}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x800,  0xFF07}}
};

    /* in 50G PCS mode: 4 PCS lanes are bounded */
static MV_CFG_ELEMENT mmpcsLane4_50G_PowerDownSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0,    0x700}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x1000,        0x0,    0x700}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x2000,        0x0,    0x700}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL + 0x3000,        0x0,    0x700}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0x0,    0x300}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x1000,        0x0,    0x300}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x2000,        0x0,    0x300}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + 0x3000,        0x0,    0x300}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x810,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x810,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x810,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0,    0x8300}},
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL_2,               0x0,    0x1}}
};

static MV_CFG_ELEMENT mmpcsLane1_25G_UnResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x827,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsLane1_26_7G_UnResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x10,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x811,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x815,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x817,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsMultiLane_StartSendFaultSeqParams[] = {
    /* Lane 0 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_0,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_1,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_2,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_3,             0x0,  0xFFFF}},

    /* Lane 1 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_0,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_1,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_2,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_3,             0x0,  0xFFFF}},

    /* Lane 2 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_0,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_1,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_2,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_3,             0x0,  0xFFFF}},

    /* Lane 3 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_0,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_1,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_2,             0x0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_3,             0x0,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsMultiLane_StopSendFaultSeqParams[] = {
    /* Lane 0 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_0,             0x7690,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_1,             0x3e47,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_2,             0x896f,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_3,             0xc1b8,  0xFFFF}},

    /* Lane 1 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_0,             0xc4f0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_1,             0x63e6,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_2,             0x3b0f,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_3,             0x9c19,  0xFFFF}},

    /* Lane 2 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_0,             0x65c5,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_1,             0x339b,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_2,             0x9a3a,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_3,             0xcc64,  0xFFFF}},

    /* Lane 3 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_0,             0x79a2,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_1,             0xd83d,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_2,             0x865d,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_3,             0x27c2,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsMultiLane_StopSendFaultRsFecSeqParams[] = {
        /* Lane 0 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_0,             0x68c1,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_1,             0x3321,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_2,             0x973e,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE0_ALIGNMENT_MARKER_3,             0xccde,  0xFFFF}},

    /* Lane 1 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_0,             0xc4f0,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_1,             0x63e6,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_2,             0x3b0f,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE1_ALIGNMENT_MARKER_3,             0x9c19,  0xFFFF}},

    /* Lane 2 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_0,             0x65c5,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_1,             0x339b,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_2,             0x9a3a,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE2_ALIGNMENT_MARKER_3,             0xcc64,  0xFFFF}},

    /* Lane 3 markers */
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_0,             0x79a2,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_1,             0xd83d,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_2,             0x865d,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, LANE3_ALIGNMENT_MARKER_3,             0x27c2,  0xFFFF}}
};

/*  for 10G/20G/40G port modes  */
/* ============================ */
static MV_CFG_ELEMENT mmpcsLane1ResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsLane2ResetSeqParams[] = {
    MMPCS_LANE2_RESET_SEQ_PARAMS_MAC
};

static MV_CFG_ELEMENT mmpcsLane4ResetSeqParams[] = {
    MMPCS_LANE4_RESET_SEQ_PARAMS_MAC
};

static MV_CFG_ELEMENT mmpcsLane2PowerDownSeqParams[] = {
    MMPCS_LANE2_RESET_SEQ_PARAMS_MAC,
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0000, 0x0300}}
};

static MV_CFG_ELEMENT mmpcsLane4PowerDownSeqParams[] = {
    MMPCS_LANE4_RESET_SEQ_PARAMS_MAC,
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0000, 0x0300}}
};

static MV_CFG_ELEMENT mmpcsLane1UnResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x880,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x080,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x880,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x881,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x885,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x887,  0xFFF7}}
};

static MV_CFG_ELEMENT mmpcsLane2UnResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x840,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x40,   0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x840,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x841,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x845,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x847,  0xFFF7}}
};

static MV_CFG_ELEMENT mmpcsLane4UnResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x820,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x820,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x820,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x20,   0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x20,   0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x20,   0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x20,   0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x820,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x820,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x820,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x821,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x821,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x821,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x821,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x825,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x825,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x825,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x825,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x827,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x827,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x827,  0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x827,  0xFFF7}}
};

/* GOP Rev 3 (BobK) */
/* ================ */
static MV_CFG_ELEMENT mmpcsGopRev3Lane1UnResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsGopRev3Lane2UnResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x20,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x820,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x821,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x825,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x827,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x827,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsGopRev3Lane4UnResetSeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x10,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x10,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x10,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x10,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x810,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x811,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x811,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x811,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x811,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x815,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x815,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x815,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x815,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x817,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                0x817,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                0x817,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                0x817,  0xFFFF}}
};

/* AlleyCat3 Port 29 only */
/* ====================== */
static MV_CFG_ELEMENT mmpcsFabricModeLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x3920, 0xFBFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION,                 0xA0,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, CHANNEL_CONFIGURATION + (0x1000 * 21), 0x1A0,  0xFFFF}}
};

static MV_CFG_ELEMENT mmpcsFabricResetLane2SeqParams[] = {
    MMPCS_FABRIC_RESET_LANE2_SEQ_PARAMS_MAC
};

static MV_CFG_ELEMENT mmpcsFabricPowerDownLane2SeqParams[] = {
    MMPCS_FABRIC_RESET_LANE2_SEQ_PARAMS_MAC,
    {WRITE_OP, {MMPCS_UNIT, PCS40G_COMMON_CONTROL,                 0x0000, 0x0300}}
};

static MV_CFG_ELEMENT mmpcsFabricUnResetLane2SeqParams[] = {
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x40,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x40,   0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x840,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x841,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x841,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x845,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x845,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                         0x847,  0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + (0x1000 * 21),         0x847,  0xFFFF}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsMmPsc28nmSeqDb[] =
{
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_RESET_SEQ                 */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_UNRESET_SEQ               */
    {mmpcsLane1ModeSeqParams,          MV_SEQ_SIZE(mmpcsLane1ModeSeqParams)},          /* MMPCS_MODE_1_LANE_SEQ           */
    {mmpcsLane2ModeSeqParams,          MV_SEQ_SIZE(mmpcsLane2ModeSeqParams)},          /* MMPCS_MODE_2_LANE_SEQ           */
    {mmpcsLane4ModeSeqParams,          MV_SEQ_SIZE(mmpcsLane4ModeSeqParams)},          /* MMPCS_MODE_4_LANE_SEQ           */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_LPBK_NORMAL_SEQ           */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_LPBK_RX2TX_SEQ            */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_LPBK_TX2RX_SEQ            */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_GEN_NORMAL_SEQ            */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_GEN_KRPAT_SEQ             */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_MODE_1_LANE_SEQ    */
    {mmpcsFabricModeLane2SeqParams,    MV_SEQ_SIZE(mmpcsFabricModeLane2SeqParams)},    /* MMPCS_FABRIC_MODE_2_LANE_SEQ    */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_MODE_4_LANE_SEQ    */
    {mmpcsLane1ResetSeqParams,         MV_SEQ_SIZE(mmpcsLane1ResetSeqParams)},         /* MMPCS_RESET_1_LANE_SEQ          */
    {mmpcsLane2ResetSeqParams,         MV_SEQ_SIZE(mmpcsLane2ResetSeqParams)},         /* MMPCS_RESET_2_LANE_SEQ          */
    {mmpcsLane4ResetSeqParams,         MV_SEQ_SIZE(mmpcsLane4ResetSeqParams)},         /* MMPCS_RESET_4_LANE_SEQ          */
    {mmpcsLane1UnResetSeqParams,       MV_SEQ_SIZE(mmpcsLane1UnResetSeqParams)},       /* MMPCS_UNRESET_1_LANE_SEQ        */
    {mmpcsLane2UnResetSeqParams,       MV_SEQ_SIZE(mmpcsLane2UnResetSeqParams)},       /* MMPCS_UNRESET_2_LANE_SEQ        */
    {mmpcsLane4UnResetSeqParams,       MV_SEQ_SIZE(mmpcsLane4UnResetSeqParams)},       /* MMPCS_UNRESET_4_LANE_SEQ        */
    {mmpcsFabricResetLane2SeqParams,   MV_SEQ_SIZE(mmpcsFabricResetLane2SeqParams)},   /* MMPCS_FABRIC_RESET_2_LANE_SEQ   */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_RESET_4_LANE_SEQ   */
    {mmpcsFabricUnResetLane2SeqParams, MV_SEQ_SIZE(mmpcsFabricUnResetLane2SeqParams)}, /* MMPCS_FABRIC_UNRESET_2_LANE_SEQ */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* MMPCS_FABRIC_UNRESET_4_LANE_SEQ */

    {mmpcsGopRev3Lane1UnResetSeqParams, MV_SEQ_SIZE(mmpcsGopRev3Lane1UnResetSeqParams)}, /* MMPCS_GOP_REV3_UNRESET_1_LANE_SEQ */
    {mmpcsGopRev3Lane2UnResetSeqParams, MV_SEQ_SIZE(mmpcsGopRev3Lane2UnResetSeqParams)}, /* MMPCS_GOP_REV3_UNRESET_2_LANE_SEQ */
    {mmpcsGopRev3Lane4UnResetSeqParams, MV_SEQ_SIZE(mmpcsGopRev3Lane4UnResetSeqParams)}, /* MMPCS_GOP_REV3_UNRESET_4_LANE_SEQ */
    {mmpcsLane2PowerDownSeqParams,       MV_SEQ_SIZE(mmpcsLane2PowerDownSeqParams)},      /* MMPCS_POWER_DOWN_2_LANE_SEQ        */
    {mmpcsLane4PowerDownSeqParams,       MV_SEQ_SIZE(mmpcsLane4PowerDownSeqParams)},      /* MMPCS_POWER_DOWN_4_LANE_SEQ        */
    {mmpcsFabricPowerDownLane2SeqParams, MV_SEQ_SIZE(mmpcsFabricPowerDownLane2SeqParams)}, /* MMPCS_FABRIC_POWER_DOWN_2_LANE_SEQ */

    {mmpcsLane1_25G_ModeSeqParams,      MV_SEQ_SIZE(mmpcsLane1_25G_ModeSeqParams)},        /* MMPCS_MODE_1_LANE_25G_SEQ        */
    {mmpcsLane4_50G_ModeSeqParams,      MV_SEQ_SIZE(mmpcsLane4_50G_ModeSeqParams)},        /* MMPCS_MODE_4_LANE_50G_SEQ        */
    {mmpcsLane1_25G_ResetSeqParams,     MV_SEQ_SIZE(mmpcsLane1_25G_ResetSeqParams)},       /* MMPCS_RESET_1_LANE_25G_SEQ       */
    {mmpcsLane4_50G_ResetSeqParams,     MV_SEQ_SIZE(mmpcsLane4_50G_ResetSeqParams)},       /* MMPCS_RESET_4_LANE_50G_SEQ       */
    {mmpcsLane1_25G_PowerDownSeqParams, MV_SEQ_SIZE(mmpcsLane1_25G_PowerDownSeqParams)},   /* MMPCS_POWER_DOWN_1_LANE_25G_SEQ  */
    {mmpcsLane4_50G_PowerDownSeqParams, MV_SEQ_SIZE(mmpcsLane4_50G_PowerDownSeqParams)},   /* MMPCS_POWER_DOWN_4_LANE_50G_SEQ  */
    {mmpcsLane1_25G_UnResetSeqParams,   MV_SEQ_SIZE(mmpcsLane1_25G_UnResetSeqParams)},      /* MMPCS_UNRESET_1_LANE_25G_SEQ     */

    {mmpcsMultiLane_StartSendFaultSeqParams,   MV_SEQ_SIZE(mmpcsMultiLane_StartSendFaultSeqParams)},   /* MMPCS_START_SEND_FAULT_MULTI_LANE_SEQ */
    {mmpcsMultiLane_StopSendFaultSeqParams,   MV_SEQ_SIZE(mmpcsMultiLane_StopSendFaultSeqParams)},   /* MMPCS_STOP_SEND_FAULT_MULTI_LANE_SEQ */
    {mmpcsMultiLane_StopSendFaultRsFecSeqParams,   MV_SEQ_SIZE(mmpcsMultiLane_StopSendFaultRsFecSeqParams)},   /* MMPCS_STOP_SEND_FAULT_MULTI_LANE_RS_FEC_SEQ */
    {mmpcsLane1_26_7G_UnResetSeqParams,   MV_SEQ_SIZE(mmpcsLane1_26_7G_UnResetSeqParams)}       /* MMPCS_UNRESET_1_LANE_26_7G_SEQ     */
};

GT_STATUS hwsMMPsc28nmSeqInit(void)
{
    return GT_OK;
}

GT_STATUS hwsMMPcs28nmSeqGet(MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if (seqType >= MV_MM_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  if (hwsMmPsc28nmSeqDb[seqType].cfgSeq == NULL)
  {
      /* check prev version */
      return hwsMMPcsSeqGet(seqType, seqLine, lineNum);
  }

  *seqLine = hwsMmPsc28nmSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}

