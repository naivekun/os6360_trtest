/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the "File") is owned and distributed by Marvell
* International Ltd. and/or its affiliates ("Marvell") under the following
* alternative licensing terms.
* If you received this File from Marvell, you may opt to use, redistribute
* and/or modify this File under the following licensing terms.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*  -   Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*  -    Neither the name of Marvell nor the names of its contributors may be
*       used to endorse or promote products derived from this software without
*       specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
*/
/**
********************************************************************************
* @file mvHwsMtiPcs400Db.c
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

static MV_CFG_ELEMENT mtiPcs400resetSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_CONTROL1,      /*dummy write 0x8000*/0x0000,     0x8000}}  /*CONTROL1.Reset.set(1)*/

};

static MV_CFG_ELEMENT mtiPcs400unresetSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_CONTROL1,                     0x8000,        0x8000}}  /*CONTROL1.Reset.set(0)*/

};

static MV_CFG_ELEMENT mtiPcs400PowerDownSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_TX_LANE_THRESH,        0x7,        0xF}},     /*VENDOR_TX_LANE_THRESH.Tx_lanethresh.set(7)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_TX_CDMII_PACE,         0x8,        0xF}},     /*VENDOR_TX_CDMII_PACE.Tx_cdmii_pace.set(8)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_CONTROL1,                     0x0,        0x4}},     /*CONTROL1.Speed_selection.set(mii == _200GMII ? 1 : 0)*/

    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL0_0,               0x7101,     0xFFFF}}, /*VENDOR_VL0_0.set(16'hc0b3)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL0_1,               0xf3,       0xFF}},   /*VENDOR_VL0_1.set(8'h8c)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL1_0,               0xde5a,     0xFFFF}}, /*VENDOR_VL1_0.set(16'hde5a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL1_1,               0x7e,       0xFF}},   /*VENDOR_VL1_1.set(8'h7e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL2_0,               0xf33e,     0xFFFF}}, /*VENDOR_VL2_0.set(16'hf33e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL2_1,               0x56,       0xFF}},   /*VENDOR_VL2_1.set(8'h56)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL3_0,               0x8086,     0xFFFF}}, /*VENDOR_VL3_0.set(16'h8086)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL3_1,               0xd0,       0xFF}},   /*VENDOR_VL3_1.set(8'hd0)*/

    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL4_0,               0x512a,     0xFFFF}}, /*VENDOR_VL4_0.set(16'h512a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL4_1,               0xf2,       0xFF}},   /*VENDOR_VL4_1.set(8'f2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL5_0,               0x4f12,     0xFFFF}}, /*VENDOR_VL5_0.set(16'h4f12)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL5_1,               0xd1,       0xFF}},   /*VENDOR_VL5_1.set(8'hd1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL6_0,               0x9c42,     0xFFFF}}, /*VENDOR_VL6_0.set(16'h9c42)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL6_1,               0xa1,       0xFF}},   /*VENDOR_VL6_1.set(8'ha1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL7_0,               0x76d6,     0xFFFF}}, /*VENDOR_VL7_0.set(16'h76d6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL7_1,               0x5b,       0xFF}},   /*VENDOR_VL7_1.set(8'h5b)*/

};


static MV_CFG_ELEMENT mtiPcs400R4ModeSeqParams[] = { /* speed  200G R4 */
    /**************** MTI_PCS400::ConfigureCM *************/

    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_TX_LANE_THRESH,        0x9,        0xF}},     /*VENDOR_TX_LANE_THRESH.Tx_lanethresh.set(9)*/
    /*Marker_counter = ((20479 + 1)/5);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_VL_INTVL,              0x1000,     0xFFFF}},  /*VENDOR_VL_INTVL.Marker_counter.set ((20479 + 1)/5);*/
    /* Common Marker values same as 400G*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_AM_0,                  0x4a9a,     0xFFFF}},  /*VENDOR_AM_0.Cm1.set(8'h4a)+VENDOR_AM_0.Cm0.set(8'h9a);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_AM_1,                  0x26,       0xFF}},    /*VENDOR_AM_1.Cm2.set(8'h26);*/

    /*************** MTI_PCS400::SpeedSelect(MII_TYPE _200GMII);********/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_TX_CDMII_PACE,         0x8,        0xF}},     /*VENDOR_TX_CDMII_PACE.Tx_cdmii_pace.set(8)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_CONTROL1,                     0x4,        0x4}},     /*CONTROL1.Speed_selection.set(mii == _200GMII ? 1 : 0)*/


    /*************** MTI_RSFEC::Configure(FEC_TYPE _200G_544_514); ***************/
       /* Unique markers values from rsfec file VL1..VL7 same as 400G */
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL0_0,               0xc0b3,     0xFFFF}},  /*VENDOR_VL0_0.set(16'hc0b3)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL0_1,               0x8c,       0xFF}},    /*VENDOR_VL0_1.set(8'h8c)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL1_0,               0xde5a,     0xFFFF}},  /*VENDOR_VL1_0.set(16'hde5a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL1_1,               0x7e,       0xFF}},    /*VENDOR_VL1_1.set(8'h7e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL2_0,               0xf33e,     0xFFFF}},  /*VENDOR_VL2_0.set(16'hf33e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL2_1,               0x56,       0xFF}},    /*VENDOR_VL2_1.set(8'h56)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL3_0,               0x8086,     0xFFFF}},  /*VENDOR_VL3_0.set(16'h8086)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL3_1,               0xd0,       0xFF}},    /*VENDOR_VL3_1.set(8'hd0)*/

    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL4_0,               0x512a,     0xFFFF}}, /*VENDOR_VL4_0.set(16'h512a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL4_1,               0xf2,       0xFF}},   /*VENDOR_VL4_1.set(8'f2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL5_0,               0x4f12,     0xFFFF}}, /*VENDOR_VL5_0.set(16'h4f12)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL5_1,               0xd1,       0xFF}},   /*VENDOR_VL5_1.set(8'hd1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL6_0,               0x9c42,     0xFFFF}}, /*VENDOR_VL6_0.set(16'h9c42)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL6_1,               0xa1,       0xFF}},   /*VENDOR_VL6_1.set(8'ha1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL7_0,               0x76d6,     0xFFFF}}, /*VENDOR_VL7_0.set(16'h76d6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL7_1,               0x5b,       0xFF}},   /*VENDOR_VL7_1.set(8'h5b)*/
};

static MV_CFG_ELEMENT mtiPcs400R8ModeSeqParams[] = { /* speed  400G R8 */
    /**************** MTI_PCS400::ConfigureCM *************/

    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_TX_LANE_THRESH,        0x9,        0xF}},     /*VENDOR_TX_LANE_THRESH.Tx_lanethresh.set(9)*/
    /*Marker_counter = ((20479 + 1)/5);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_VL_INTVL,              0x2000,     0xFFFF}},  /*VENDOR_VL_INTVL.Marker_counter.set ((20479 + 1)/5);*/
    /* Common Marker values same as 400G*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_AM_0,                  0x4a9a,     0xFFFF}},  /*VENDOR_AM_0.Cm1.set(8'h4a)+VENDOR_AM_0.Cm0.set(8'h9a);*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_AM_1,                  0x26,       0xFF}},    /*VENDOR_AM_1.Cm2.set(8'h26);*/

    /*************** MTI_PCS400::SpeedSelect(MII_TYPE _200GMII);********/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_VENDOR_TX_CDMII_PACE,         0x4,        0xF}},     /*VENDOR_TX_CDMII_PACE.Tx_cdmii_pace.set(4)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_CONTROL1,                     0x0,        0x4}},     /*CONTROL1.Speed_selection.set(mii == _200GMII ? 1 : 0)*/


    /*************** MTI_RSFEC::Configure(FEC_TYPE _400G_544_514); ***************/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL0_0,               0x7101,     0xFFFF}},  /*VENDOR_VL0_0.set(16'h7101)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL0_1,               0xf3,       0xFF}},    /*VENDOR_VL0_1.set(8'hf3)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL1_0,               0xde5a,     0xFFFF}},  /*VENDOR_VL1_0.set(16'hde5a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL1_1,               0x7e,       0xFF}},    /*VENDOR_VL1_1.set(8'h7e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL2_0,               0xf33e,     0xFFFF}},  /*VENDOR_VL2_0.set(16'hf33e)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL2_1,               0x56,       0xFF}},    /*VENDOR_VL2_1.set(8'h56)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL3_0,               0x8086,     0xFFFF}},  /*VENDOR_VL3_0.set(16'h8086)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL3_1,               0xd0,       0xFF}},    /*VENDOR_VL3_1.set(8'hd0)*/

    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL4_0,               0x512a,     0xFFFF}},  /*VENDOR_VL4_0.set(16'h512a)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL4_1,               0xf2,       0xFF}},    /*VENDOR_VL4_1.set(8'f2)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL5_0,               0x4f12,     0xFFFF}},  /*VENDOR_VL5_0.set(16'h4f12)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL5_1,               0xd1,       0xFF}},    /*VENDOR_VL5_1.set(8'hd1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL6_0,               0x9c42,     0xFFFF}},  /*VENDOR_VL6_0.set(16'h9c42)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL6_1,               0xa1,       0xFF}},    /*VENDOR_VL6_1.set(8'ha1)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL7_0,               0x76d6,     0xFFFF}},  /*VENDOR_VL7_0.set(16'h76d6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL7_1,               0x5b,       0xFF}},    /*VENDOR_VL7_1.set(8'h5b)*/

    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL8_0,               0x73e1,     0xFFFF}},  /*VENDOR_VL8_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL8_1,               0x75,       0xFF}},    /*VENDOR_VL8_1.set(8'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL9_0,               0xc471,     0xFFFF}},  /*VENDOR_VL9_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL9_1,               0x3c,       0xFF}},    /*VENDOR_VL9_1.set(8'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL10_0,              0xeb95,     0xFFFF}},  /*VENDOR_VL10_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL10_1,              0xd8,       0xFF}},    /*VENDOR_VL10_1.set(8'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL11_0,              0x6622,     0xFFFF}},  /*VENDOR_VL11_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL11_1,              0x38,       0xFF}},    /*VENDOR_VL11_1.set(8'h)*/

    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL12_0,              0xf6a2,     0xFFFF}},  /*VENDOR_VL12_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL12_1,              0x95,       0xFF}},    /*VENDOR_VL12_1.set(8')*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL13_0,              0x9731,     0xFFFF}},  /*VENDOR_VL13_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL13_1,              0xc3,       0xFF}},    /*VENDOR_VL13_1.set(8'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL14_0,              0xfbca,     0xFFFF}},  /*VENDOR_VL14_0.set(16'h)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL14_1,              0xa6,       0xFF}},    /*VENDOR_VL14_1.set(8'ha6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL15_0,              0xbaa6,     0xFFFF}},  /*VENDOR_VL15_0.set(16'hbaa6)*/
    {WRITE_OP, {RAVEN_UNIT_MTI_RSFEC,  MTI_RSFEC_VENDOR_VL15_1,              0x79,       0xFF}},    /*VENDOR_VL15_1.set(8'h79)*/
};

static MV_CFG_ELEMENT mtiPcs400StartSendFaultSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_EXT, MTIP_EXT_SEG_PORT_CONTROL,               0x2,        0x2}}
};

static MV_CFG_ELEMENT mtiPcs400StopSendFaultSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_EXT, MTIP_EXT_SEG_PORT_CONTROL,               0x0,        0x2}}
};

static MV_CFG_ELEMENT mtiPcs400LbNormalSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_CONTROL1,                     0x0,        0x4000}}
};

static MV_CFG_ELEMENT mtiPcs400LbTx2RxSeqParams[] = {
    {WRITE_OP, {RAVEN_UNIT_MTI_PCS400, MTI_PCS_CONTROL1,                     0x4000,     0x4000}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsMtiPcs400SeqDb[] =
{
    {mtiPcs400resetSeqParams,            MV_SEQ_SIZE(mtiPcs400resetSeqParams)},           /* MTI_PCS_RESET_SEQ            */
    {mtiPcs400unresetSeqParams,          MV_SEQ_SIZE(mtiPcs400unresetSeqParams)},         /* MTI_PCS_UNRESET_SEQ          */
    {mtiPcs400PowerDownSeqParams,        MV_SEQ_SIZE(mtiPcs400PowerDownSeqParams)},       /* MTI_PCS_POWER_DOWN_SEQ       */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XG_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XG_25_RS_FEC_MODE_SEQ */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XLG_50R1_MODE_SEQ    */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XLG_50R2_RS_FEC_MODE_SEQ    */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XlG_MODE_SEQ    */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_CG_MODE_SEQ     */

    {mtiPcs400R4ModeSeqParams,           MV_SEQ_SIZE(mtiPcs400R4ModeSeqParams)},          /* MTI_PCS_200R4_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200R8_MODE_SEQ     */
    {mtiPcs400R8ModeSeqParams,           MV_SEQ_SIZE(mtiPcs400R8ModeSeqParams)},          /* MTI_PCS_400R8_MODE_SEQ     */

    {mtiPcs400StartSendFaultSeqParams,   MV_SEQ_SIZE(mtiPcs400StartSendFaultSeqParams)},  /* MTI_PCS_START_SEND_FAULT_SEQ */
    {mtiPcs400StopSendFaultSeqParams,    MV_SEQ_SIZE(mtiPcs400StopSendFaultSeqParams)},   /* MTI_PCS_STOP_SEND_FAULT_SEQ  */
    {mtiPcs400LbNormalSeqParams,         MV_SEQ_SIZE(mtiPcs400LbNormalSeqParams)},        /* MTI_PCS_LPBK_NORMAL_SEQ      */
    {mtiPcs400LbTx2RxSeqParams,          MV_SEQ_SIZE(mtiPcs400LbTx2RxSeqParams)},         /* MTI_PCS_LPBK_TX2RX_SEQ       */

};

GT_STATUS hwsMtiPcs400SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if (seqType >= MV_MTI_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMtiPcs400SeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
