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
* mvHwsComPhyHDb.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 50 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyHDb.h>

static MV_CFG_ELEMENT cmPhy_SerdesSdResetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSdUnresetSeq[] = {
    {WRITE_OP, {SERDES_UNIT,  SERDES_EXTERNAL_CONFIGURATION_1, (1 << 2) | (1 << 3), (1 << 2) | (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesRfResetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0,   (1 << 6)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesRfUnresetSeq[] = {
    {WRITE_OP, {SERDES_UNIT,  SERDES_EXTERNAL_CONFIGURATION_1, (1 << 6), (1 << 2) | (1 << 6)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSynceResetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSynceUnresetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, (1 << 3), (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesPowerUpCtrlSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x1802,   0x1802}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x10,     0x10}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1, (1 << 3), (1 << 3)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0x7000,   0x7000}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesPowerDownCtrlSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0, 0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1, 1, 0xFFFF77FF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0, 0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_3, 0x20, 0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0, 0, 0}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesRxintUpSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98, 0x400, 0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C, 0x40, 0x40}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98, 0, 0x400}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesRxintDownSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1, 0, (1 << 4)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98, 0, 0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1, (1 << 3), (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_1_25gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x30, 0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x0330, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0,      0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,          0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,          0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,          0xD247, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,          0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,          0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,          0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,          0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,          0x0066, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,          0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,         0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,         0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,         0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,         0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,         0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,         0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,         0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,         0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,         0x0800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,         0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,         0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,         0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,         0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,         0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,         0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0000, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_3_125gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x30, 0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x0440, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0,      0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,          0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,          0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,          0xD247, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,          0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,          0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,          0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,          0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,          0x0088, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,          0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,         0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,         0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,         0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,         0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,         0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,         0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,         0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,         0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,         0x0800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,         0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,         0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,         0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,         0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,         0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,         0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0000, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_3_75gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x30, 0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x04C8, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0,      0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,          0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,          0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,          0xD247, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,          0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,          0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,          0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,          0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,          0x0099, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,          0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,         0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,         0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,         0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,         0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,         0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,         0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,         0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,         0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,         0x0800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,         0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,         0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,         0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,         0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,         0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,         0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0000, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_4_25gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x30, 0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x0550, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0,      0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,          0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,          0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,          0xD247, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,          0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,          0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,          0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,          0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,          0x00AA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,          0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,         0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,         0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,         0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,         0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,         0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,         0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,         0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,         0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,         0x0800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,         0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,         0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,         0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,         0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,         0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,         0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0000, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_5gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x30, 0x70}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x0198, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0,      0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,          0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,          0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,          0xD147, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,          0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,          0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,          0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,          0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,          0x0033, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,          0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,         0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,         0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,         0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,         0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,         0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,         0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,         0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,         0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,         0x0800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,         0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,         0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,         0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,         0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,         0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,         0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0000, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_6_25gSeq[] = {

    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x10, 0x50}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x05D8, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0,      0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,          0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,          0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,          0xD147, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,          0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,          0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,          0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,          0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,          0x00BB, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,          0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,         0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,         0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,         0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,         0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,         0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,         0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,         0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,         0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,         0x0800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,         0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,         0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,         0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,         0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,         0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,         0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0000, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_7_5gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x10, 0x50}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x06E8, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0,      0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,          0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,          0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,          0xD147, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,          0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,          0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,          0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,          0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,          0x00DD, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,          0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,         0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,         0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,         0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,         0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,         0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,         0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,         0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,         0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,         0x0800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,         0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,         0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,         0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,         0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,         0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,         0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0000, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_10_3125gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x0770, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0,      0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,           0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,          0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,          0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,          0xD147, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,          0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,          0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,          0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,          0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,          0x00EE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,          0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,         0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,         0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,         0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,         0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,         0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,         0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,         0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,         0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,         0x1800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,         0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,         0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,         0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,         0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,         0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,         0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,         0x0000, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSdLpbk_normalSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C, 0, (0xF << 12)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0, (1 << 15)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSdAna_tx_2_rxSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C, (1 << 13), (1 << 13)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSdDig_tx_2_rxSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C, (1 << 14), (1 << 14)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSdDig_rx_2_txSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0xD0, 0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0x8000, 0x8000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C, 0x9000, 0x9000}}
};
static MV_CFG_ELEMENT cmPhy_SerdesPtAfterPattern_normal_seq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0xE0, 0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0, 0x8000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, (1 << 14), (1 << 14)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0, (1 << 14)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C, (1 << 11), (1 << 11)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x8C, 0, (1 << 11)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesPtAfterPattern_test_seq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0xE0, 0xF0}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0x8000, 0x8000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x84, 0x2030, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x84, 0x30, 0xFFFF}}
};
static MV_CFG_ELEMENT cmPhy_SerdesRxTrainingEnableSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18, 0, (1 << 7)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,1, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x30, 0xF, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x40, 0xD7, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC,  0, 0x100}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C, 0x7C70, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x70, 0x8800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x78, 0xDF3, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34, 0x400, 0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x80, 0xCABF, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x44, 0x4F4E, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x48, 0x4D4C, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4C, 0x3F3E, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x50, 0x3D3C, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x54, 0x2F2E, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x58, 0x2D2C, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x5C, 0x1F1E, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x60, 0xF0E, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, (1 << 5), (1 << 5)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_3, (1 << 7), (1 << 7)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesRxTrainingDisableSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0, (1 << 5)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_3, 0, (1 << 7)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x1, 0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C,  0x0, 0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC, 0x0, 0x1}}
};
static MV_CFG_ELEMENT cmPhy_SerdesTxTrainingEnableSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94, 0xFFF, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18, 0x0, (1 << 7)}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,0x1, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14, 0x1, 0x1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x28, 0xA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C, 0x7430, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x30, 0xF, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34, 0x400, 0x400}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x40, 0xD7, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC,  0x0, 0x100}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x80, 0xCABF, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x78, 0xDF3, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x68, 0x2C00, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x70, 0x8800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xA0, 0x2000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xA4, 0x4000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xA8, 0x6000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xAC, 0x7000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xB0, 0x9000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xB4, 0xA000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xB8, 0xC000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xBC, 0xF000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC0, 0xF000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC4, 0xF000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC8, 0xF000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xCC, 0xF000, 0xF000}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94, 0xC00, 0x1E00}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x6C, 0x0, 0xE07}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xDC, 0x1, 0xF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xD8, 0xEFF, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,0x0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, (1 << 5), (1 << 5)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesTxTrainingDisableSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94, 0x1FFF, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,0x1, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xC,  0x120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14, 0, 1}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x2C, 0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x68, 0x6C01, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x94, 0x19F4, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x6C, 0x341C, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xDC, 0x0, 0xF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xD8, 0xE7A, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1FC,0x0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2, 0x0, 0x20}},
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_3, 0x0, 0x100}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_12_5gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_0, 0x07F8, 0x07F8}},
    {WRITE_OP, {SERDES_UNIT, SERDES_RESERVED_REGISTER_0, 0x0C, 0x1F}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x4,     0xFC8C, 0xFFFF}}, 
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x14,    0x07FE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x18,    0x00BF, 0xFFFF}},  /* Squelch threshold = 0 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1C,    0xD147, 0xFFFF}},  /* set DFE resolution to 4mV */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x20,    0xAFC0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x34,    0x0DFE, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x38,    0x0BD2, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x88,    0x1026, 0xFFFF}},  /* password unprotected */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x98,    0x00FF, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0xF4,    0x0012, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x104,   0x0200, 0xFFFF}},  /* tximpcal_th[2:0] 0 : For TX_IMP, Rximpcal_th[2:0] 0 For RX_IMP */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x108,   0xC0FA, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x10C,   0x36D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x110,   0x30D6, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x114,   0x33A0, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x134,   0x3113, 0xFFFF}}, /* VREGTXRX[1:0] 0 : For Saving the power (lower regulator voltage), VTHVCOCAL[1:0] 00  : For PLL CAL, SEL_VTHVCOCONT 1*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x138,   0x0104, 0xFFFF}}, /* VREGVCO[1:0] 0 : For Saving the power (lower regulator voltage) */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x13C,   0x0026, 0xFFFF}}, /* ICP = 6 */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x140,   0x1800, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x168,   0xD528, 0xFFFF}}, /* set align90 to 85d */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x184,   0x1093, 0xFFFF}}, /* MSI recommend leaving dtl_sq_ploop_en at 1.*/
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1CC,   0x0B28, 0xFFFF}}, /* SEL_SMPLR_VCM[1:0]=0.5 V */
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,   0x0001, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x008,   0x0600, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x00C,   0x0120, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x02c,   0x7470, 0xFFFF}},
    {WRITE_OP, {SERDES_UNIT, SERDES_PHY_REGS + 0x1fc,   0x0000, 0xFFFF}}

};
#if 0
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_3_3gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_11_5625gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesPartialPowerDownSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSerdesPartialPowerUpSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_11_25gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesCoreResetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesCoreUnresetSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesFfeTableLrSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesFfeTableSrSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_10_9375gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_12_1875gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_5_625gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
static MV_CFG_ELEMENT cmPhy_SerdesSpeed_5_15625gSeq[] = {
    {WRITE_OP, {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,      0,   (1 << 3)}}
};
#endif

/* seqId to params array mapping */
MV_CFG_SEQ hwsSerdesSeqDb[] =
{
    {cmPhy_SerdesSdResetSeq,                MV_SEQ_SIZE(cmPhy_SerdesSdResetSeq)},                /* SERDES_SD_RESET_SEQ                  */
    {cmPhy_SerdesSdUnresetSeq,              MV_SEQ_SIZE(cmPhy_SerdesSdUnresetSeq)},              /* SERDES_SD_UNRESET_SEQ                */
    {cmPhy_SerdesRfResetSeq,                MV_SEQ_SIZE(cmPhy_SerdesRfResetSeq)},                /* SERDES_RF_RESET_SEQ                  */
    {cmPhy_SerdesRfUnresetSeq,              MV_SEQ_SIZE(cmPhy_SerdesRfUnresetSeq)},              /* SERDES_RF_UNRESET_SEQ                */
    {cmPhy_SerdesSynceResetSeq,             MV_SEQ_SIZE(cmPhy_SerdesSynceResetSeq)},             /* SERDES_SYNCE_RESET_SEQ               */
    {cmPhy_SerdesSynceUnresetSeq,           MV_SEQ_SIZE(cmPhy_SerdesSynceUnresetSeq)},           /* SERDES_SYNCE_UNRESET_SEQ             */
    {cmPhy_SerdesSerdesPowerUpCtrlSeq,      MV_SEQ_SIZE(cmPhy_SerdesSerdesPowerUpCtrlSeq)},      /* SERDES_SERDES_POWER_UP_CTRL_SEQ      */
    {cmPhy_SerdesSerdesPowerDownCtrlSeq,    MV_SEQ_SIZE(cmPhy_SerdesSerdesPowerDownCtrlSeq)},    /* SERDES_SERDES_POWER_DOWN_CTRL_SEQ    */
    {cmPhy_SerdesSerdesRxintUpSeq,          MV_SEQ_SIZE(cmPhy_SerdesSerdesRxintUpSeq)},          /* SERDES_SERDES_RXINT_UP_SEQ           */
    {cmPhy_SerdesSerdesRxintDownSeq,        MV_SEQ_SIZE(cmPhy_SerdesSerdesRxintDownSeq)},        /* SERDES_SERDES_RXINT_DOWN_SEQ         */
    {NULL,                                  0},                                                  /* SERDES_SERDES_WAIT_PLL_SEQ                */
    {cmPhy_SerdesSpeed_1_25gSeq,            MV_SEQ_SIZE(cmPhy_SerdesSpeed_1_25gSeq)},            /* SERDES_SPEED_1_25G_SEQ               */
    {cmPhy_SerdesSpeed_3_125gSeq,           MV_SEQ_SIZE(cmPhy_SerdesSpeed_3_125gSeq)},           /* SERDES_SPEED_3_125G_SEQ              */
    {cmPhy_SerdesSpeed_3_75gSeq,            MV_SEQ_SIZE(cmPhy_SerdesSpeed_3_75gSeq)},            /* SERDES_SPEED_3_75G_SEQ               */
    {cmPhy_SerdesSpeed_4_25gSeq,            MV_SEQ_SIZE(cmPhy_SerdesSpeed_4_25gSeq)},            /* SERDES_SPEED_4_25G_SEQ               */
    {cmPhy_SerdesSpeed_5gSeq,               MV_SEQ_SIZE(cmPhy_SerdesSpeed_5gSeq)},               /* SERDES_SPEED_5G_SEQ                  */
    {cmPhy_SerdesSpeed_6_25gSeq,            MV_SEQ_SIZE(cmPhy_SerdesSpeed_6_25gSeq)},            /* SERDES_SPEED_6_25G_SEQ               */
    {cmPhy_SerdesSpeed_7_5gSeq,             MV_SEQ_SIZE(cmPhy_SerdesSpeed_7_5gSeq)},             /* SERDES_SPEED_7_5G_SEQ                */
    {cmPhy_SerdesSpeed_10_3125gSeq,         MV_SEQ_SIZE(cmPhy_SerdesSpeed_10_3125gSeq)},         /* SERDES_SPEED_10_3125G_SEQ            */
    {cmPhy_SerdesSdLpbk_normalSeq,          MV_SEQ_SIZE(cmPhy_SerdesSdLpbk_normalSeq)},          /* SERDES_SD_LPBK_NORMAL_SEQ            */
    {cmPhy_SerdesSdAna_tx_2_rxSeq,          MV_SEQ_SIZE(cmPhy_SerdesSdAna_tx_2_rxSeq)},          /* SERDES_SD_ANA_TX_2_RX_SEQ            */
    {cmPhy_SerdesSdDig_tx_2_rxSeq,          MV_SEQ_SIZE(cmPhy_SerdesSdDig_tx_2_rxSeq)},          /* SERDES_SD_DIG_TX_2_RX_SEQ            */
    {cmPhy_SerdesSdDig_rx_2_txSeq,          MV_SEQ_SIZE(cmPhy_SerdesSdDig_rx_2_txSeq)},          /* SERDES_SD_DIG_RX_2_TX_SEQ            */
    {cmPhy_SerdesPtAfterPattern_normal_seq, MV_SEQ_SIZE(cmPhy_SerdesPtAfterPattern_normal_seq)}, /* SERDES_PT_AFTER_PATTERN_NORMAL_SEQ   */
    {cmPhy_SerdesPtAfterPattern_test_seq,   MV_SEQ_SIZE(cmPhy_SerdesPtAfterPattern_test_seq)},   /* SERDES_PT_AFTER_PATTERN_TEST_SEQ     */
    {cmPhy_SerdesRxTrainingEnableSeq,       MV_SEQ_SIZE(cmPhy_SerdesRxTrainingEnableSeq)},       /* SERDES_RX_TRAINING_ENABLE_SEQ        */
    {cmPhy_SerdesRxTrainingDisableSeq,      MV_SEQ_SIZE(cmPhy_SerdesRxTrainingDisableSeq)},      /* SERDES_RX_TRAINING_DISABLE_SEQ       */
    {cmPhy_SerdesTxTrainingEnableSeq,       MV_SEQ_SIZE(cmPhy_SerdesTxTrainingEnableSeq)},       /* SERDES_TX_TRAINING_ENABLE_SEQ        */
    {cmPhy_SerdesTxTrainingDisableSeq,      MV_SEQ_SIZE(cmPhy_SerdesTxTrainingDisableSeq)},      /* SERDES_TX_TRAINING_DISABLE_SEQ       */
    {cmPhy_SerdesSpeed_12_5gSeq,            MV_SEQ_SIZE(cmPhy_SerdesSpeed_12_5gSeq)},            /* SERDES_SPEED_12_5G_SEQ               */
    {NULL,                                  0},                                                   /* SERDES_SPEED_3_3G_SEQ                */
    {NULL,                                  0},                                                   /* SERDES_SPEED_11_5625G_SEQ            */
    {NULL,                                  0},                                                   /* SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ */
    {NULL,                                  0},                                                   /* SERDES_SERDES_PARTIAL_POWER_UP_SEQ   */
    {NULL,                                  0},                                                   /* SERDES_SPEED_11_25G_SEQ              */
    {NULL,                                  0},                                                   /* SERDES_CORE_RESET_SEQ                */
    {NULL,                                  0},                                                   /* SERDES_CORE_UNRESET_SEQ              */
    {NULL,                                  0},                                                   /* SERDES_FFE_TABLE_LR_SEQ              */
    {NULL,                                  0},                                                   /* SERDES_FFE_TABLE_SR_SEQ              */
    {NULL,                                  0},                                                   /* SERDES_SPEED_10_9375G_SEQ            */
    {NULL,                                  0},                                                   /* SERDES_SPEED_12_1875G_SEQ            */
    {NULL,                                  0},                                                   /* SERDES_SPEED_5_625G_SEQ              */
    {NULL,                                  0}                                                    /* SERDES_SPEED_5_15625G_SEQ            */
};


GT_STATUS hwsComPhyHSeqGet(MV_HWS_COM_PHY_H_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if (seqLine == NULL)
  {
    return GT_BAD_PTR;
  }
  if ((seqType >= MV_SERDES_LAST_SEQ) || (hwsSerdesSeqDb[seqType].cfgSeq == NULL))
  {
    return GT_NO_SUCH;
  }

  if (hwsSerdesSeqDb[seqType].cfgSeqSize <= lineNum)
  {
    return GT_NO_MORE;
  }

  *seqLine = hwsSerdesSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}



