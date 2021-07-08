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
* mvHwsSerdesPrvIf.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*
*******************************************************************************/

#ifndef __mvHwsSerdes28nmPrvIf_H
#define __mvHwsSerdes28nmPrvIf_H

#ifdef __cplusplus
extern "C" {
#endif

#define MV_HWS_SERDES_SCAN_LENGTH              128
#define MV_HWS_SERDES_SCAN_WIN_LENGTH          (MV_HWS_SERDES_SCAN_LENGTH / 2)
#define MV_HWS_COE_FROM_GEN_TABLE               4
/* General H Files */

/*COM_H 28nm SERDES PHY registers offset */
#define COM_H_28NM_PU_PLL_Reg                   (0x04)
#define COM_H_28NM_KVCO_CAL_Reg                 (0x08)
#define COM_H_28NM_IMP_CAL_Reg                  (0x0C)
#define COM_H_28NM_IMP_TXSsc_Reg                (0x10)
#define COM_H_28NM_Trans_Freq_Reg               (0x14)
#define COM_H_28NM_Squelch_FFE_Reg              (0x18)
#define COM_H_28NM_DFE_Res_Reg                  (0x1C)
#define COM_H_28NM_DFE_Update_Reg               (0x20)
#define COM_H_28NM_DFE_F0F2_Reg1                (0x24)
#define COM_H_28NM_DFE_F3F5_Reg2                (0x28)
#define COM_H_28NM_Adapt_DFE_Reg0               (0x2C)
#define COM_H_28NM_Adapt_DFE_Reg1               (0x30)
#define COM_H_28NM_GEN1_SET0_Reg                (0x34)
#define COM_H_28NM_GEN1_SET1_Reg                (0x38)
#define COM_H_28NM_GEN2_SET0_Reg                (0x3C)
#define COM_H_28NM_GEN2_SET1_Reg                (0x40)
#define COM_H_28NM_GEN3_SET0_Reg                (0x44)
#define COM_H_28NM_GEN3_SET1_Reg                (0x48)
#define COM_H_28NM_GEN4_SET0_Reg                (0x4C)
#define COM_H_28NM_GEN4_SET1_Reg                (0x50)
#define COM_H_28NM_PT_PATTERN_SEL_Reg           (0x54)
#define COM_H_28NM_Pattern_Reg                  (0x6C)
#define COM_H_28NM_FRAME_Cntr_MSB1_Reg          (0x70)
#define COM_H_28NM_FRAME_Cntr_MSB_Reg           (0x74)
#define COM_H_28NM_FRAME_Cntr_LSB_Reg           (0x78)
#define COM_H_28NM_ERROR_Cntr_MSB_Reg           (0x7C)
#define COM_H_28NM_ERROR_Cntr_LSB_Reg           (0x80)
#define COM_H_28NM_PHY_TEST_OOB_Reg             (0x84)
#define COM_H_28NM_SEL_BITS_Reg                 (0x8C)
#define COM_H_28NM_Sync_Pattern_Reg             (0x90)
#define COM_H_28NM_PHY_Isolate_Reg              (0x98)
#define COM_H_28NM_DFE_STEP_SIZE_Reg            (0xF0)
#define COM_H_28NM_GEN1_SET2_Reg                (0xF4)
#define COM_H_28NM_GEN2_SET2_Reg                (0xF8)

#define COM_H_28NM_VDD_Cal_Cntrl_Reg2           (0x110)
#define COM_H_28NM_VDD_Cal_Cntrl_Reg3           (0x114)
#define COM_H_28NM_Analog_Reg2                  (0x11C)
#define COM_H_28NM_Lane_Align_Reg               (0x124)
#define COM_H_28NM_Analog_Test_Reg              (0x128)
#define COM_H_28NM_150M_Clock_Reg0              (0x12C)
#define COM_H_28NM_MISC_Control0_Reg            (0x13C)
#define COM_H_28NM_RX_Reg1                      (0x140)
#define COM_H_28NM_Power_Cntrl_Reg              (0x148)
#define COM_H_28NM_FeedbackDivider_Reg          (0x14C)
#define COM_H_28NM_ForwardDivider_Reg           (0x150)
#define COM_H_28NM_Calibr_En_Reg                (0x15C)
#define COM_H_28NM_RxClockAl90_Reg              (0x168)
#define COM_H_28NM_Cal_Reg_7                    (0x16C)
#define COM_H_28NM_TX_EMPH_Reg                  (0x178)
#define COM_H_28NM_SSC_Step_Reg                 (0x180)
#define COM_H_28NM_DTL_Cntrl_Reg                (0x184)
#define COM_H_28NM_OFFSET_Phase_Reg             (0x188)
#define COM_H_28NM_Rx_Freq_Offset_Read_Reg      (0x190)
#define COM_H_28NM_DFE_CFG_Reg                  (0x19C)
#define COM_H_28NM_Data_Phase_Offset_Reg        (0x1A0)
#define COM_H_28NM_FFE_Reg                      (0x1A4)
#define COM_H_28NM_Ext_Squelch_Calibr_Reg       (0x1BC)
#define COM_H_28NM_Pll_Cal_Control0_Reg         (0x1C0)
#define COM_H_28NM_Speed_Pll_Calibr_Reg         (0x1C4)
#define COM_H_28NM_FFE_REG1                     (0x20C)
#define COM_H_28NM_Tx_Train_Reg0                (0x22C)
#define COM_H_28NM_Rx_Train_Reg4                (0x244)
#define COM_H_28NM_Tx_Train_Result0             (0x310)
#define COM_H_28NM_Tx_Train_Result1             (0x314)
#define COM_H_28NM_TX_Train_Preset_Reg          (0x31C)
#define COM_H_28NM_Saved_DFE_Values_Reg0        (0x324)
#define COM_H_28NM_Saved_DFE_Values_Reg1        (0x328)
#define COM_H_28NM_Saved_DFE_Values_Reg2        (0x32C)
#define COM_H_28NM_RX_Train_Reg13               (0x330)
#define COM_H_28NM_Cal_Reg_10                   (0x400)
#define COM_H_28NM_Cal_Reg_11                   (0x404)
#define COM_H_28NM_Cal_Reg_12                   (0x408)
#define COM_H_28NM_Cal_Reg_15                   (0x414)
#define COM_H_28NM_Cal_Reg_23                   (0x418)
#define COM_H_28NM_G1_SET_3_Reg                 (0x440)
#define COM_H_28NM_G1_SET_4_Reg                 (0x444)
#define COM_H_28NM_Squelch_Detect_Reg           (0x580)
#define COM_H_28NM_Align90_Calibr_Reg           (0x584)
#define COM_H_28NM_Internal_CID_Reg             (0x5F8)

#define COM_H_28NM_SamplerCalSelect             (0x16C)
#define COM_H_28NM_SamplerCalRd                 (0x3EC)
#define COM_H_28NM_SamplerCalRead               (0x5F0)
#define COM_H_28NM_Serdes_Rev_Reg               (0x800)

/* Server registers */
#define MV_SERVER_Temp_Reg                      (0xF8078)

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsSerdes28nmPrvIf_H */

