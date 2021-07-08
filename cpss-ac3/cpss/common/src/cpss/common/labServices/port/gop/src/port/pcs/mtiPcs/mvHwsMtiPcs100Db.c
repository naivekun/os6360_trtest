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
* @file mvHwsMtiPcs100Db.c
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

static MV_CFG_ELEMENT mtiPcs100resetSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,     /*dummy write 0x8000*/0x0000,     0x8000}}  /*CONTROL1.Reset.set(1)*/

};

static MV_CFG_ELEMENT mtiPcs100unresetSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000}}  /*CONTROL1.Reset.set(1)*/
   /* {RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x0,     0x8000} */ /*CONTROL1.Reset.set(0)*/

};

static MV_CFG_ELEMENT mtiPcs100PowerDownSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,       0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter;*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x2}}, /*VENDOR_PCS_MODE.Disable_mld;*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1}}, /*VENDOR_PCS_MODE.Ena_clause49;*/

    /* Marker values for 100G-KR4 / 100G-KR2 (no FEC dependency) */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_1,               0x0021,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h21)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_0,               0x719d,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'h719d)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_1,               0x008e,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'h8e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_0,               0x4b59,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h4b59)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_1,               0x00e8,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'he8)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_0,               0x954d,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h954d)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_1,               0x007b,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h7b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL4_0,               0x07f5,     0xFFFF}}, /*VL4_0.Vl4_0.set(16'h7f5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL4_1,               0x0009,     0xFFFF}}, /*VL4_1.Vl4_1.set(8'h9)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL5_0,               0x14dd,     0xFFFF}}, /*VL5_0.Vl5_0.set(16'h14dd)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL5_1,               0x00c2,     0xFFFF}}, /*VL5_1.Vl5_1.set(8'hc2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL6_0,               0x4a9a,     0xFFFF}}, /*VL6_0.Vl6_0.set(16'h4a9a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL6_1,               0x0026,     0xFFFF}}, /*VL6_1.Vl6_1.set(8'h26)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL7_0,               0x457b,     0xFFFF}}, /*VL7_0.Vl7_0.set(16'h457b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL7_1,               0x0066,     0xFFFF}}, /*VL7_1.Vl7_1.set(8'h66)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL8_0,               0x24a0,     0xFFFF}}, /*VL8_0.Vl8_0.set(16'h24a0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL8_1,               0x0076,     0xFFFF}}, /*VL8_1.Vl8_1.set(8'h76)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL9_0,               0xc968,     0xFFFF}}, /*VL9_0.Vl9_0.set(16'hc968)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL9_1,               0x00fb,     0xFFFF}}, /*VL9_1.Vl9_1.set(8'hfb)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL10_0,              0x6cfd,     0xFFFF}}, /*VL10_0.Vl10_0.set(16'h6cfd)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL10_1,              0x0099,     0xFFFF}}, /*VL10_1.Vl10_1.set(8'h99)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL11_0,              0x91b9,     0xFFFF}}, /*VL11_0.Vl11_0.set(16'h91b9)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL11_1,              0x0055,     0xFFFF}}, /*VL11_1.Vl11_1.set(8'h55)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL12_0,              0xb95c,     0xFFFF}}, /*VL12_0.Vl12_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL12_1,              0x00b2,     0xFFFF}}, /*VL12_1.Vl12_1.set(8'hb2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL13_0,              0xf81a,     0xFFFF}}, /*VL13_0.Vl13_0.set(16'hf81a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL13_1,              0x00bd,     0xFFFF}}, /*VL13_1.Vl13_1.set(8'hbd)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL14_0,              0xc783,     0xFFFF}}, /*VL14_0.Vl14_0.set(16'hc783)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL14_1,              0x00ca,     0xFFFF}}, /*VL14_1.Vl14_1.set(8'hca)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL15_0,              0x3635,     0xFFFF}}, /*VL15_0.Vl15_0.set(16'h3635)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL15_1,              0x00cd,     0xFFFF}}, /*VL15_1.Vl15_1.set(8'hcd)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL16_0,              0x31c4,     0xFFFF}}, /*VL16_0.Vl16_0.set(16'h31c4)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL16_1,              0x004c,     0xFFFF}}, /*VL16_1.Vl16_1.set(8'h4c)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL17_0,              0xd6ad,     0xFFFF}}, /*VL17_0.Vl17_0.set(16'hd6ad)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL17_1,              0x00b7,     0xFFFF}}, /*VL17_1.Vl17_1.set(8'hb7)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL18_0,              0x665f,     0xFFFF}}, /*VL18_0.Vl18_0.set(16'h665f)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL18_1,              0x002a,     0xFFFF}}, /*VL18_1.Vl18_1.set(8'h2a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL19_0,              0xf0c0,     0xFFFF}}, /*VL19_0.Vl19_0.set(16'hf0c0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL19_1,              0x00e5,     0xFFFF}}, /*VL19_1.Vl19_1.set(8'he5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_CONTROL,           0x0,        0x200}}, /*RSFEC_CONTROL.Tc_pad_value.set(1'b1)*/

};

static MV_CFG_ELEMENT mtiPcs100XgModeSeqParams[] = { /* MTI_PCS_XG_MODE_SEQ - speed up to 25G */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x2,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(1);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
};

static MV_CFG_ELEMENT mtiPcs100Xg25RsFecModeSeqParams[] = { /* MTI_PCS_XG_25_RS_FEC_MODE_SEQ - 25G with rs fec*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(0x4FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x1,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_1,               0x0021,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h21)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'he6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_1,               0x009b,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_1,               0x003d,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h3d)*/

};


static MV_CFG_ELEMENT mtiPcs100Xlg50R1ModeSeqParams[] = { /* speed  50G r1*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/

    /* Marker values for 50G_R2,50G_R1 and 40G_R4 (no FEC dependency) */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_0,               0x7690,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_1,               0x0047,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h47)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'he6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_1,               0x009b,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_1,               0x003d,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h3d)*/

   /* {RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000} */ /*CONTROL1.Reset.set(1)*/
};


static MV_CFG_ELEMENT mtiPcs100Xlg50R2RsFecModeSeqParams[] = { /* speed  50G r2 rs fec 528*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_VL_INTVL,     0x4FFF,     0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/

    /* Marker values for 50G_R2,50G_R1 and 40G_R4 (no FEC dependency) */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_0,               0x7690,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_1,               0x0047,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h47)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'he6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_1,               0x009b,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_1,               0x003d,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h3d)*/

    /************* MTI_RSFEC::Configure(FEC_TYPE _50G_CONSORTIUM); **********/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_CONTROL,           0x200,     0x200}}, /*RSFEC_CONTROL.Tc_pad_value.set(1'b1)*/

   /* {RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000} */ /*CONTROL1.Reset.set(1)*/
};

static MV_CFG_ELEMENT mtiPcs100XlgModeSeqParams[] = { /* speed  40G,50G */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,     0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(4FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}},    /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1}},    /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/

    /* Marker values for 50G_R2,50G_R1 and 40G_R4 (no FEC dependency) */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_0,               0x7690,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h7690)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_1,               0x0047,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h47)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_0,               0xc4f0,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'hc4f0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_1,               0x00e6,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'he6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_0,               0x65c5,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h65c5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_1,               0x009b,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'h9b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_0,               0x79a2,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h79a2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_1,               0x003d,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h3d)*/

   /* {RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000} */ /*CONTROL1.Reset.set(1)*/
};

static MV_CFG_ELEMENT mtiPcs100CgModeSeqParams[] = { /* speed above 50G */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_VL_INTVL,     0x3FFF,       0xFFFF}}, /*VENDOR_VL_INTVL.Marker_counter.set(3FFF);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x2}}, /*VENDOR_PCS_MODE.Disable_mld.set(0);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VENDOR_PCS_MODE,     0x0,        0x1}}, /*VENDOR_PCS_MODE.Ena_clause49.set(0);*/

    /* Marker values for 100G-KR4 / 100G-KR2 (no FEC dependency) */
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_0,               0x68c1,     0xFFFF}}, /*VL0_0.Vl0_0.set(16'h68c1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL0_1,               0x0021,     0xFFFF}}, /*VL0_1.Vl0_1.set(8'h21)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_0,               0x719d,     0xFFFF}}, /*VL1_0.Vl1_0.set(16'h719d)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL1_1,               0x008e,     0xFFFF}}, /*VL1_1.Vl1_1.set(8'h8e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_0,               0x4b59,     0xFFFF}}, /*VL2_0.Vl2_0.set(16'h4b59)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL2_1,               0x00e8,     0xFFFF}}, /*VL2_1.Vl2_1.set(8'he8)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_0,               0x954d,     0xFFFF}}, /*VL3_0.Vl3_0.set(16'h954d)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL3_1,               0x007b,     0xFFFF}}, /*VL3_1.Vl3_1.set(8'h7b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL4_0,               0x07f5,     0xFFFF}}, /*VL4_0.Vl4_0.set(16'h7f5)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL4_1,               0x0009,     0xFFFF}}, /*VL4_1.Vl4_1.set(8'h9)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL5_0,               0x14dd,     0xFFFF}}, /*VL5_0.Vl5_0.set(16'h14dd)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL5_1,               0x00c2,     0xFFFF}}, /*VL5_1.Vl5_1.set(8'hc2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL6_0,               0x4a9a,     0xFFFF}}, /*VL6_0.Vl6_0.set(16'h4a9a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL6_1,               0x0026,     0xFFFF}}, /*VL6_1.Vl6_1.set(8'h26)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL7_0,               0x457b,     0xFFFF}}, /*VL7_0.Vl7_0.set(16'h457b)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL7_1,               0x0066,     0xFFFF}}, /*VL7_1.Vl7_1.set(8'h66)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL8_0,               0x24a0,     0xFFFF}}, /*VL8_0.Vl8_0.set(16'h24a0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL8_1,               0x0076,     0xFFFF}}, /*VL8_1.Vl8_1.set(8'h76)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL9_0,               0xc968,     0xFFFF}}, /*VL9_0.Vl9_0.set(16'hc968)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL9_1,               0x00fb,     0xFFFF}}, /*VL9_1.Vl9_1.set(8'hfb)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL10_0,              0x6cfd,     0xFFFF}}, /*VL10_0.Vl10_0.set(16'h6cfd)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL10_1,              0x0099,     0xFFFF}}, /*VL10_1.Vl10_1.set(8'h99)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL11_0,              0x91b9,     0xFFFF}}, /*VL11_0.Vl11_0.set(16'h91b9)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL11_1,              0x0055,     0xFFFF}}, /*VL11_1.Vl11_1.set(8'h55)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL12_0,              0xb95c,     0xFFFF}}, /*VL12_0.Vl12_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL12_1,              0x00b2,     0xFFFF}}, /*VL12_1.Vl12_1.set(8'hb2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL13_0,              0xf81a,     0xFFFF}}, /*VL13_0.Vl13_0.set(16'hf81a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL13_1,              0x00bd,     0xFFFF}}, /*VL13_1.Vl13_1.set(8'hbd)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL14_0,              0xc783,     0xFFFF}}, /*VL14_0.Vl14_0.set(16'hc783)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL14_1,              0x00ca,     0xFFFF}}, /*VL14_1.Vl14_1.set(8'hca)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL15_0,              0x3635,     0xFFFF}}, /*VL15_0.Vl15_0.set(16'h3635)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL15_1,              0x00cd,     0xFFFF}}, /*VL15_1.Vl15_1.set(8'hcd)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL16_0,              0x31c4,     0xFFFF}}, /*VL16_0.Vl16_0.set(16'h31c4)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL16_1,              0x004c,     0xFFFF}}, /*VL16_1.Vl16_1.set(8'h4c)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL17_0,              0xd6ad,     0xFFFF}}, /*VL17_0.Vl17_0.set(16'hd6ad)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL17_1,              0x00b7,     0xFFFF}}, /*VL17_1.Vl17_1.set(8'hb7)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL18_0,              0x665f,     0xFFFF}}, /*VL18_0.Vl18_0.set(16'h665f)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL18_1,              0x002a,     0xFFFF}}, /*VL18_1.Vl18_1.set(8'h2a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL19_0,              0xf0c0,     0xFFFF}}, /*VL19_0.Vl19_0.set(16'hf0c0)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_VL19_1,              0x00e5,     0xFFFF}} /*VL19_1.Vl19_1.set(8'he5)*/

    /*{RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,            0x8000,     0x8000} */ /*CONTROL1.Reset.set(1)*/

};

static MV_CFG_ELEMENT mtiPcs100LbNormalSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,               0x0,       0x4000}}
};

static MV_CFG_ELEMENT mtiPcs100LbTx2RxSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS100, MTI_PCS_CONTROL1,               0x4000,    0x4000}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsMtiPcs100SeqDb[] =
{
    {mtiPcs100resetSeqParams,            MV_SEQ_SIZE(mtiPcs100resetSeqParams)},           /* MTI_PCS_RESET_SEQ            */
    {mtiPcs100unresetSeqParams,          MV_SEQ_SIZE(mtiPcs100unresetSeqParams)},         /* MTI_PCS_UNRESET_SEQ          */
    {mtiPcs100PowerDownSeqParams,        MV_SEQ_SIZE(mtiPcs100PowerDownSeqParams)},       /* MTI_PCS_POWER_DOWN_SEQ       */

    {mtiPcs100XgModeSeqParams,           MV_SEQ_SIZE(mtiPcs100XgModeSeqParams)},          /* MTI_PCS_XG_MODE_SEQ     */
    {mtiPcs100Xg25RsFecModeSeqParams,    MV_SEQ_SIZE(mtiPcs100Xg25RsFecModeSeqParams)},   /* MTI_PCS_XG_25_RS_FEC_MODE_SEQ */

    {mtiPcs100Xlg50R1ModeSeqParams,      MV_SEQ_SIZE(mtiPcs100Xlg50R1ModeSeqParams)},     /* MTI_PCS_XLG_50R1_MODE_SEQ    */
    {mtiPcs100Xlg50R2RsFecModeSeqParams, MV_SEQ_SIZE(mtiPcs100Xlg50R2RsFecModeSeqParams)},/* MTI_PCS_XLG_50R2_RS_FEC_MODE_SEQ    */
    {mtiPcs100XlgModeSeqParams,          MV_SEQ_SIZE(mtiPcs100XlgModeSeqParams)},         /* MTI_PCS_XlG_MODE_SEQ    */

    {mtiPcs100CgModeSeqParams,           MV_SEQ_SIZE(mtiPcs100CgModeSeqParams)},          /* MTI_PCS_CG_MODE_SEQ     */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200R4_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200R8_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_400R8_MODE_SEQ     */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_START_SEND_FAULT_SEQ */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_STOP_SEND_FAULT_SEQ  */
    {mtiPcs100LbNormalSeqParams,         MV_SEQ_SIZE(mtiPcs100LbNormalSeqParams)},        /* MTI_PCS_LPBK_NORMAL_SEQ      */
    {mtiPcs100LbTx2RxSeqParams,          MV_SEQ_SIZE(mtiPcs100LbTx2RxSeqParams)},         /* MTI_PCS_LPBK_TX2RX_SEQ       */

};

GT_STATUS hwsMtiPcs100SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if (seqType >= MV_MTI_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMtiPcs100SeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}





