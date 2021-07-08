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
* @file mvHwsCgPcs28nmDb.c
*
* @brief
*
* @version   7
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcs28nmDb.h>

static MV_CFG_ELEMENT cgPcsMode1Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x4FFF,         0xFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER0_VL0,                    0x2168c1,       0xFFFFFF}},/* fix AM0 markers for 25G-RS-FEC to IEEE format */
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER1_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER2_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER3_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x68c1,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3321,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x973e,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xccde,         0xFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x1,            0x1}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x10}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x200,          0x200}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x20000,        0x20000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x100,          0x100}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x1000,         0x1000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x2000,         0x2000}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x33,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}}
};

static MV_CFG_ELEMENT cgPcsMode1Lane_1SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x4FFF0000,     0xFFFF0000}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER0_VL0,                    0x2168c1,       0xFFFFFF}},/* fix AM0 markers for 25G-RS-FEC to IEEE format */
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER1_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER2_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER3_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x68c1,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3321,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x973e,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xccde,         0xFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x1,            0x1}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x10}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x400,          0x400}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x40000,        0x40000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x200,          0x200}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x2000,         0x2000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x4000,         0x4000}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x33,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}}
};

static MV_CFG_ELEMENT cgPcsMode1Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x4FFF,         0xFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER0_VL0,                    0x2168c1,       0xFFFFFF}},/* fix AM0 markers for 25G-RS-FEC to IEEE format */
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER1_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER2_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER3_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x68c1,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3321,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x973e,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xccde,         0xFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x2,            0x2}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x10}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x800,          0x800}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x80000,        0x80000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x400,          0x400}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x4000,         0x4000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x8000,         0x8000}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x33,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}}
};

static MV_CFG_ELEMENT cgPcsMode1Lane_3SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x4FFF0000,     0xFFFF0000}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER0_VL0,                    0x2168c1,       0xFFFFFF}},/* fix AM0 markers for 25G-RS-FEC to IEEE format */
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER1_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER2_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONV_MARKER3_VL0,                    0x2168c1,       0xFFFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_0,               0x68c1,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_1,               0x3321,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_2,               0x973e,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        LANE0_ALIGNMENT_MARKER_3,               0xccde,         0xFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x2,            0x2}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x10}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x1000,         0x1000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x100000,       0x100000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x800,          0x800}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x8000,         0x8000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x10000,        0x10000}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x33,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}}
};

static MV_CFG_ELEMENT cgPcsMode2Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x4FFF4FFF,     0xFFFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x604,          0x615}}, /* bit {4} should not be restored to default value in power down if one more 50G port is defined in neighbor slot*/
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x3300,         0x3300}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}}, /* this bit should not be restored to default value in power down if one more 50G port is defined in neighbor slot*/
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x6000,         0x6000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x200,          0x200}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}},
};

static MV_CFG_ELEMENT cgPcsMode2Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x4FFF4FFF,     0xFFFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x1808,         0x181A}}, /* bit {4} should not be restored to default value in power down if one more 50G port is defined in neighbor slot*/
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0xCC00,         0xCC00}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}}, /* this bit should not be restored to default value in power down if one more 50G port is defined in neighbor slot*/
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x18000,        0x18000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x200,          0x200}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}},
};

static MV_CFG_ELEMENT cgPcsMode4LaneSeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0xC10000,       0xC10000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0xFF00,         0xFF00}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x1E010,        0x1FFE10}},
    /* BC3 RM-8564354 - RxFifo Overrun ends with packets corruption entering the Chip Pipe. Need to configure the CG convertor RXFiFo Thd to 26 (0x1A) */
    {WRITE_OP, {CG_UNIT,           CG_DMA_FIFO_CONFIG,                     0x1A,           0x3F}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VIRTUAL_LANE_INTERVAL,               0x3FFF,         0xFFFF}}
};

#if !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
static MV_CFG_ELEMENT cgPcsMode4Lane_MLG_SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x1E50,         0x1E50}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0xFF00,         0xFF00}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x800000,       0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x1E000,        0x1E000}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL0_0,                               0x4089,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL0_1,                               0x9F,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL1_0,                               0x8529,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL1_1,                               0x1D,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL2_0,                               0x39AA,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL2_1,                               0xE3,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL3_0,                               0x7EBF,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL3_1,                               0x4D,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL4_0,                               0x6B14,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL4_1,                               0xD7,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL5_0,                               0x8BEE,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL5_1,                               0xBA,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL6_0,                               0xDBE1,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL6_1,                               0x6C,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL7_0,                               0x2D0,          0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL7_1,                               0x39,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL8_0,                               0xB839,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL8_1,                               0x5C,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL9_0,                               0xD2A1,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL9_1,                               0xAB,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL10_0,                              0x594A,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL10_1,                              0x12,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL11_0,                              0x7898,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL11_1,                              0x7,            0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL12_0,                              0xD355,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL12_1,                              0xC6,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL13_0,                              0x9031,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL13_1,                              0xC3,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL14_0,                              0xA2B6,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL14_1,                              0xCF,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL15_0,                              0x89F,          0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL15_1,                              0xB6,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL16_0,                              0x55BB,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL16_1,                              0x9D,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL17_0,                              0x5A8,          0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL17_1,                              0xFC,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL18_0,                              0xA104,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL18_1,                              0x94,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL19_0,                              0x7207,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL19_1,                              0xDB,           0xFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x200,          0x300}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL + 0x2000,         0x200,          0x300}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x10,           0xF0}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x1000,                 0x10,           0xF0}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x2000,                 0x10,           0xF0}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x3000,                 0x10,           0xF0}}
};
#endif

static MV_CFG_ELEMENT cgPcsRsFec2Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x60000,        0x60000}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x4FFF4FFF,     0xFFFFFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x33,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x33,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF}}
};

static MV_CFG_ELEMENT cgPcsRsFec2Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x180000,       0x180000}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x4FFF4FFF,     0xFFFFFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x33,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x33,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x4FFF,         0xFFFF}}
};

static MV_CFG_ELEMENT cgPcsRsFec4LaneSeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x1E0000,       0x1E0000}},
    /* BC3 RM-8564354 - RxFifo Overrun ends with packets corruption entering the Chip Pipe. Need to configure the CG convertor RXFiFo Thd to 26 (0x1A) */
    {WRITE_OP, {CG_UNIT,           CG_DMA_FIFO_CONFIG,                     0x1A,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x0,            0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x0,            0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x0,            0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x0,            0x3F}}
};

static MV_CFG_ELEMENT cgPcsReset1Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x1}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x1000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}}
};

static MV_CFG_ELEMENT cgPcsReset1Lane_1SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x2}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x20}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x2000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}}
};

static MV_CFG_ELEMENT cgPcsReset1Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x40}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}}
};

static MV_CFG_ELEMENT cgPcsReset1Lane_3SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x8}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x80}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x8000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}}
};

static MV_CFG_ELEMENT cgPcsReset2Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x3}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x30}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x3000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}}
};

static MV_CFG_ELEMENT cgPcsReset2Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC0}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}}
};

static MV_CFG_ELEMENT cgPcsReset4LaneSeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xFBFFFFFF}}
};

#if !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
static MV_CFG_ELEMENT cgPcsReset4Lane_MLG_SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xFBFFFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x810,          0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x1000,                 0x810,          0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x2000,                 0x810,          0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x3000,                 0x810,          0xFFF7}}
};
#endif

static MV_CFG_ELEMENT cgPcsPowerDown1Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x1}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x1000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x1100}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x00010,        0x22211}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF,         0xFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}}/*,
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}}*/
};

static MV_CFG_ELEMENT cgPcsPowerDown1Lane_1SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x2}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x20}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x2000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x2200}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x00010,        0x44411}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF0000,     0xFFFF0000}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

static MV_CFG_ELEMENT cgPcsPowerDown1Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x40}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x4400}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x00010,        0x88812}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF,         0xFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

static MV_CFG_ELEMENT cgPcsPowerDown1Lane_3SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x8}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x80}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x8000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x8800}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x00010,        0x111012}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF0000,     0xFFFF0000}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

static MV_CFG_ELEMENT cgPcsReducedPowerDown1Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x1}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x1000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x1100}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x22201}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF,         0xFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}}
};

static MV_CFG_ELEMENT cgPcsReducedPowerDown1Lane_1SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x2}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x20}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x2000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x2200}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x44400}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF0000,     0xFFFF0000}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}}
};

static MV_CFG_ELEMENT cgPcsReducedPowerDown1Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x40}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x4400}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x88802}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF,         0xFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}}
};

static MV_CFG_ELEMENT cgPcsReducedPowerDown1Lane_3SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x8}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x80}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x8000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x8800}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x111000}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF0000,     0xFFFF0000}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}}
};

static MV_CFG_ELEMENT cgPcsPowerDown2Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x3}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x30}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x3000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x3300}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x10,           0x66615}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF3FFF,     0xFFFFFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x0300}}/*,
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}}*/
};

static MV_CFG_ELEMENT cgPcsPowerDown2Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC0}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x10000000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x0,            0x800000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0xCC00}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x10,           0x19981A}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF3FFF,     0xFFFFFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x0300}}/*,
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}}*/
};

/* reduced power down sequence should be executed when at least one more port is defined for the same GC MAC */
static MV_CFG_ELEMENT cgPcsReducedPowerDown2Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x3}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x30}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0x3000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0x3300}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x66605}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_1_0,           0x3FFF3FFF,     0xFFFFFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x0300}}
};

/* reduced power down sequence should be executed when at least one more port is defined for the same GC MAC */
static MV_CFG_ELEMENT cgPcsReducedPowerDown2Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC0}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xC000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0xCC00}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x0,            0x19980A}},
    {WRITE_OP, {CG_UNIT,           CG_VIRTUAL_LANE_INTERVAL_3_2,           0x3FFF3FFF,     0xFFFFFFFF}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_ALIGNMENT_CYCLE_CONTROL,         0x3FFF,         0xFFFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x0,            0xF0}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x0300}}
};

static MV_CFG_ELEMENT cgPcsPowerDown4LaneSeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xFBFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x0,            0xC10000}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x0,            0xFF00}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x10,           0x1FFE10}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0,   0x2A,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1,   0x2A,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2,   0x2A,           0x3F}},
    {WRITE_OP, {CG_RS_FEC_UNIT,    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3,   0x2A,           0x3F}}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};

#if !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
static MV_CFG_ELEMENT cgPcsPowerDown4_MLG_LaneSeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000000,      0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x0,            0xFBFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_0,                           0x10020018,     0xFFFFFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_1,                           0x3F,           0xFFFF}},
    {WRITE_OP, {CG_UNIT,           CG_CONTROL_2,                           0x190,          0xFFFFFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL0_0,                               0x68C1,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL0_1,                               0x21,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL1_0,                               0x719D,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL1_1,                               0x8E,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL2_0,                               0x4B59,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL2_1,                               0xE8,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL3_0,                               0x954D,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL3_1,                               0x7B,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL4_0,                               0x7F5,          0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL4_1,                               0x9,            0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL5_0,                               0x14DD,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL5_1,                               0xC2,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL6_0,                               0x4A9A,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL6_1,                               0x26,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL7_0,                               0x457B,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL7_1,                               0x66,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL8_0,                               0x24A0,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL8_1,                               0x76,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL9_0,                               0xC968,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL9_1,                               0xFB,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL10_0,                              0x6CFD,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL10_1,                              0x99,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL11_0,                              0x91B9,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL11_1,                              0x55,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL12_0,                              0xB95C,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL12_1,                              0xB2,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL13_0,                              0xF81A,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL13_1,                              0xBD,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL14_0,                              0xC783,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL14_1,                              0xCA,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL15_0,                              0x3635,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL15_1,                              0xCD,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL16_0,                              0x31C4,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL16_1,                              0x4C,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL17_0,                              0xD6AD,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL17_1,                              0xB7,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL18_0,                              0x665F,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL18_1,                              0x2A,           0xFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL19_0,                              0xF0C0,         0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT,        CG_VL19_1,                              0xE5,           0xFF}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x810,          0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x1000,                 0x810,          0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x2000,                 0x810,          0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG + 0x3000,                 0x810,          0xFFF7}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL,                  0x0,            0x300}},
    {WRITE_OP, {MMPCS_UNIT,        PCS40G_COMMON_CONTROL + 0x2000,         0x0,            0x300}}/*,
    {CG_UNIT,           CG_RESETS,                              0x0,            0x4000000}*/
};
#endif

static MV_CFG_ELEMENT cgPcsUnreset1Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x1,            0x1}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x1000,         0x1000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x800,          0x800}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x10,           0x10}}
};

static MV_CFG_ELEMENT cgPcsUnreset1Lane_1SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x2,            0x2}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x2000,         0x2000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x800,          0x800}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x20,           0x20}}
};

static MV_CFG_ELEMENT cgPcsUnreset1Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x4000,         0x4000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x800,          0x800}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x40,           0x40}}
};

static MV_CFG_ELEMENT cgPcsUnreset1Lane_3SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x8,            0x8}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x8000,         0x8000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x800,          0x800}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x80,           0x80}}
};

static MV_CFG_ELEMENT cgPcsUnreset2Lane_0SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x3,            0x3}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x3000,         0x3000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x30,           0x30}}
};

static MV_CFG_ELEMENT cgPcsUnreset2Lane_2SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xC,            0xC}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xC000,         0xC000}},
    {WRITE_OP, {MMPCS_UNIT,        PCS_RESET_REG,                          0x4,            0x4}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x10000000,     0x10000000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xC0,           0xC0}}
};

static MV_CFG_ELEMENT cgPcsUnreset4LaneSeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xF,            0xF}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xF0,           0xF0}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xF000,         0xF000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x3b000000,     0x3b000000}}
};

#if !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    /* in 100G_MLG CG modes: 4 CG lanes are bounded */
static MV_CFG_ELEMENT cgPcsUnreset4Lane_MLG_SeqParams[] = {
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xF,            0xF}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xF000,         0xF000}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0x33000000,     0x33000000}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                 0x1,            0x1}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                        0x1,            0x1}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                        0x1,            0x1}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                        0x1,            0x1}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                 0x4,            0x4}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                        0x4,            0x4}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                        0x4,            0x4}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                        0x4,            0x4}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG,                                 0x2,            0x2}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x1000,                        0x2,            0x2}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x2000,                        0x2,            0x2}},
    {WRITE_OP, {MMPCS_UNIT, PCS_RESET_REG + 0x3000,                        0x2,            0x2}},
    {WRITE_OP, {CG_UNIT,           CG_RESETS,                              0xF0,           0xF0}}
};
#endif

static MV_CFG_ELEMENT lbNormalSeqParams[] = {
    {WRITE_OP, {CGPCS_UNIT, CGPCS_CONTROL_1, 0x0,       0x4000}}
};

static MV_CFG_ELEMENT lbTx2RxSeqParams[] = {
    {WRITE_OP, {CGPCS_UNIT, CGPCS_CONTROL_1, 0x4000,    0x4000}}
};

static MV_CFG_ELEMENT cgPcs_StartSendFaultSeqParams[] = {
    {WRITE_OP, {CGPCS_UNIT, CG_VL0_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL0_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL1_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL1_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL2_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL2_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL3_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL3_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL4_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL4_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL5_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL5_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL6_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL6_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL7_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL7_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL8_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL8_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL9_0,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL9_1,         0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL10_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL10_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL11_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL11_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL12_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL12_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL13_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL13_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL14_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL14_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL15_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL15_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL16_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL16_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL17_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL17_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL18_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL18_1,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL19_0,        0x0,  0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL19_1,        0x0,  0xFFFF}},
};

static MV_CFG_ELEMENT cgPcs_StopSendFaultSeqParams[] = {
    {WRITE_OP, {CGPCS_UNIT, CG_VL0_0,      0x68C1,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL0_1,      0x21,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL1_0,      0x719D,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL1_1,      0x8E,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL2_0,      0x4B59,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL2_1,      0xE8,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL3_0,      0x954D,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL3_1,      0x7B,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL4_0,      0x7F5,    0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL4_1,      0x9,      0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL5_0,      0x14DD,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL5_1,      0xC2,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL6_0,      0x4A9A,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL6_1,      0x26,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL7_0,      0x457B,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL7_1,      0x66,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL8_0,      0x24A0,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL8_1,      0x76,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL9_0,      0xC968,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL9_1,      0xFB,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL10_0,     0x6CFD,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL10_1,     0x99,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL11_0,     0x91B9,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL11_1,     0x55,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL12_0,     0xB95C,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL12_1,     0xB2,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL13_0,     0xF81A,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL13_1,     0xBD,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL14_0,     0xC783,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL14_1,     0xCA,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL15_0,     0x3635,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL15_1,     0xCD,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL16_0,     0x31C4,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL16_1,     0x4C,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL17_0,     0xD6AD,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL17_1,     0xB7,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL18_0,     0x665F,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL18_1,     0x2A,     0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL19_0,     0xF0C0,   0xFFFF}},
    {WRITE_OP, {CGPCS_UNIT, CG_VL19_1,     0xE5,     0xFFFF}},
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsCgPcs28nmSeqDb[] =
{
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_RESET_SEQ        */
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_UNRESET_SEQ      */
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_MODE_MISC_SEQ    */
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_MODE_10_LANE_SEQ */
    {NULL,                              MV_SEQ_SIZE(0)},                    /* CGPCS_MODE_12_LANE_SEQ */
    {lbNormalSeqParams,                 MV_SEQ_SIZE(lbNormalSeqParams)},    /* CGPCS_LPBK_NORMAL_SEQ  */
    {lbTx2RxSeqParams,                  MV_SEQ_SIZE(lbTx2RxSeqParams)},     /* CGPCS_LPBK_TX2RX_SEQ   */

    {cgPcsMode1Lane_0SeqParams,         MV_SEQ_SIZE(cgPcsMode1Lane_0SeqParams)},       /* CGPCS_MODE_1_LANE_0_SEQ       */
    {cgPcsMode1Lane_1SeqParams,         MV_SEQ_SIZE(cgPcsMode1Lane_1SeqParams)},       /* CGPCS_MODE_1_LANE_1_SEQ       */
    {cgPcsMode1Lane_2SeqParams,         MV_SEQ_SIZE(cgPcsMode1Lane_2SeqParams)},       /* CGPCS_MODE_1_LANE_2_SEQ       */
    {cgPcsMode1Lane_3SeqParams,         MV_SEQ_SIZE(cgPcsMode1Lane_3SeqParams)},       /* CGPCS_MODE_1_LANE_3_SEQ       */
    {cgPcsMode2Lane_0SeqParams,         MV_SEQ_SIZE(cgPcsMode2Lane_0SeqParams)},       /* CGPCS_MODE_2_LANE_0_SEQ       */
    {cgPcsMode2Lane_2SeqParams,         MV_SEQ_SIZE(cgPcsMode2Lane_2SeqParams)},       /* CGPCS_MODE_2_LANE_2_SEQ       */
    {cgPcsMode4LaneSeqParams,           MV_SEQ_SIZE(cgPcsMode4LaneSeqParams)},         /* CGPCS_MODE_4_LANE_SEQ         */
    {cgPcsRsFec2Lane_0SeqParams,        MV_SEQ_SIZE(cgPcsRsFec2Lane_0SeqParams)},      /* CGPCS_RS_FEC_2_LANE_0_SEQ     */
    {cgPcsRsFec2Lane_2SeqParams,        MV_SEQ_SIZE(cgPcsRsFec2Lane_2SeqParams)},      /* CGPCS_RS_FEC_2_LANE_2_SEQ     */
    {cgPcsRsFec4LaneSeqParams,          MV_SEQ_SIZE(cgPcsRsFec4LaneSeqParams)},        /* CGPCS_RS_FEC_4_LANE_SEQ       */
    {cgPcsReset1Lane_0SeqParams,        MV_SEQ_SIZE(cgPcsReset1Lane_0SeqParams)},      /* CGPCS_RESET_1_LANE_0_SEQ      */
    {cgPcsReset1Lane_1SeqParams,        MV_SEQ_SIZE(cgPcsReset1Lane_1SeqParams)},      /* CGPCS_RESET_1_LANE_1_SEQ      */
    {cgPcsReset1Lane_2SeqParams,        MV_SEQ_SIZE(cgPcsReset1Lane_2SeqParams)},      /* CGPCS_RESET_1_LANE_2_SEQ      */
    {cgPcsReset1Lane_3SeqParams,        MV_SEQ_SIZE(cgPcsReset1Lane_3SeqParams)},      /* CGPCS_RESET_1_LANE_3_SEQ      */
    {cgPcsReset2Lane_0SeqParams,        MV_SEQ_SIZE(cgPcsReset2Lane_0SeqParams)},      /* CGPCS_RESET_2_LANE_0_SEQ      */
    {cgPcsReset2Lane_2SeqParams,        MV_SEQ_SIZE(cgPcsReset2Lane_2SeqParams)},      /* CGPCS_RESET_2_LANE_2_SEQ      */
    {cgPcsReset4LaneSeqParams,          MV_SEQ_SIZE(cgPcsReset4LaneSeqParams)},        /* CGPCS_RESET_4_LANE_SEQ        */
    {cgPcsUnreset1Lane_0SeqParams,      MV_SEQ_SIZE(cgPcsUnreset1Lane_0SeqParams)},    /* CGPCS_UNRESET_1_LANE_0_SEQ    */
    {cgPcsUnreset1Lane_1SeqParams,      MV_SEQ_SIZE(cgPcsUnreset1Lane_1SeqParams)},    /* CGPCS_UNRESET_1_LANE_1_SEQ    */
    {cgPcsUnreset1Lane_2SeqParams,      MV_SEQ_SIZE(cgPcsUnreset1Lane_2SeqParams)},    /* CGPCS_UNRESET_1_LANE_2_SEQ    */
    {cgPcsUnreset1Lane_3SeqParams,      MV_SEQ_SIZE(cgPcsUnreset1Lane_3SeqParams)},    /* CGPCS_UNRESET_1_LANE_3_SEQ    */
    {cgPcsUnreset2Lane_0SeqParams,      MV_SEQ_SIZE(cgPcsUnreset2Lane_0SeqParams)},    /* CGPCS_UNRESET_2_LANE_0_SEQ    */
    {cgPcsUnreset2Lane_2SeqParams,      MV_SEQ_SIZE(cgPcsUnreset2Lane_2SeqParams)},    /* CGPCS_UNRESET_2_LANE_2_SEQ    */
    {cgPcsUnreset4LaneSeqParams,        MV_SEQ_SIZE(cgPcsUnreset4LaneSeqParams)},      /* CGPCS_UNRESET_4_LANE_SEQ      */
    {cgPcsPowerDown1Lane_0SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown1Lane_0SeqParams)},  /* CGPCS_POWER_DOWN_1_LANE_0_SEQ */
    {cgPcsPowerDown1Lane_1SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown1Lane_1SeqParams)},  /* CGPCS_POWER_DOWN_1_LANE_1_SEQ */
    {cgPcsPowerDown1Lane_2SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown1Lane_2SeqParams)},  /* CGPCS_POWER_DOWN_1_LANE_2_SEQ */
    {cgPcsPowerDown1Lane_3SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown1Lane_3SeqParams)},  /* CGPCS_POWER_DOWN_1_LANE_3_SEQ */
    {cgPcsPowerDown2Lane_0SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown2Lane_0SeqParams)},  /* CGPCS_POWER_DOWN_2_LANE_0_SEQ */
    {cgPcsPowerDown2Lane_2SeqParams,    MV_SEQ_SIZE(cgPcsPowerDown2Lane_2SeqParams)},  /* CGPCS_POWER_DOWN_2_LANE_2_SEQ */
    {cgPcsPowerDown4LaneSeqParams,      MV_SEQ_SIZE(cgPcsPowerDown4LaneSeqParams)},    /* CGPCS_POWER_DOWN_4_LANE_SEQ   */
    {cgPcsReducedPowerDown1Lane_0SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown1Lane_0SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_1_LANE_0_SEQ */
    {cgPcsReducedPowerDown1Lane_1SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown1Lane_1SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_1_LANE_1_SEQ */
    {cgPcsReducedPowerDown1Lane_2SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown1Lane_2SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_1_LANE_2_SEQ */
    {cgPcsReducedPowerDown1Lane_3SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown1Lane_3SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_1_LANE_3_SEQ */
    {cgPcsReducedPowerDown2Lane_0SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown2Lane_0SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_2_LANE_0_SEQ */
    {cgPcsReducedPowerDown2Lane_2SeqParams,   MV_SEQ_SIZE(cgPcsReducedPowerDown2Lane_2SeqParams)}, /* CGPCS_REDUCED_POWER_DOWN_2_LANE_2_SEQ */
#if !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    {cgPcsMode4Lane_MLG_SeqParams,      MV_SEQ_SIZE(cgPcsMode4Lane_MLG_SeqParams)},      /* CGPCS_MODE_4_LANE_MLG_SEQ       */
    {cgPcsReset4Lane_MLG_SeqParams,     MV_SEQ_SIZE(cgPcsReset4Lane_MLG_SeqParams)},     /* CGPCS_RESET_4_LANE_MLG_SEQ      */
    {cgPcsPowerDown4_MLG_LaneSeqParams, MV_SEQ_SIZE(cgPcsPowerDown4_MLG_LaneSeqParams)}, /* CGPCS_POWER_DOWN_4_LANE_MLG_SEQ */
    {cgPcsUnreset4Lane_MLG_SeqParams,   MV_SEQ_SIZE(cgPcsUnreset4Lane_MLG_SeqParams)},   /* CGPCS_UNRESET_4_LANE_MLG_SEQ    */
#endif
    {cgPcs_StartSendFaultSeqParams,     MV_SEQ_SIZE(cgPcs_StartSendFaultSeqParams)},    /* CGPCS_START_SEND_FAULT_SEQ */
    {cgPcs_StopSendFaultSeqParams,      MV_SEQ_SIZE(cgPcs_StopSendFaultSeqParams)},     /* CGPCS_STOP_SEND_FAULT_SEQ  */
};

GT_STATUS hwsCgPcs28nmSeqInit(GT_U8 devNum)
{
    MV_OP_PARAMS* updateParams;

    hwsCgPcsPscSeqDb = &hwsCgPcs28nmSeqDb[0];

    if (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        updateParams = &hwsCgPcs28nmSeqDb[CGPCS_UNRESET_4_LANE_SEQ].cfgSeq[3].params;
        updateParams->operData = 0x7b000000;
        updateParams->mask = 0x7b000000;

        updateParams = &hwsCgPcsPscSeqDb[CGPCS_UNRESET_4_LANE_SEQ].cfgSeq[3].params;
        updateParams->operData = 0x7b000000;
        updateParams->mask = 0x7b000000;
    }

    return GT_OK;
}

GT_STATUS hwsCgPcs28nmSeqGet(MV_HWS_CGPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_CGPCS_LAST_SEQ) ||
      (hwsCgPcs28nmSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsCgPcs28nmSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
