/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*******************************************************************************/
/**
********************************************************************************
* @file mcdFwPortCtrlApDefs.h
*
* @brief AP Global definitions
*
* @version   17
********************************************************************************
*/

#ifndef __mcdFwServicesPortCtrlApDefs_H
#define __mcdFwServicesPortCtrlApDefs_H

#ifdef __cplusplus
extern "C" {
#endif

/* Port Control modules definition */
typedef enum
{
    M0_DEBUG           = 0,
    M1_SUPERVISOR      = 1,
    M2_PORT_MNG        = 2,
    M3_PORT_SM         = 3,
    M4_AP_PORT_MNG     = 4,
    M5_AP_PORT_DET     = 5,
    M5_AP_PORT_DET_EXT = 6,
    M6_GENERAL         = 7,
    MAX_MODULE         = 8

}MV_PORT_CTRL_MODULE;

typedef enum
{
    PORT_CTRL_AP_REG_CFG_0                                   = 1,
    PORT_CTRL_AP_REG_CFG_1                                   = 2,
    PORT_CTRL_AP_REG_ST_0                                    = 3,
    PORT_CTRL_AP_INT_REG_802_3_AP_CTRL                       = 4,
    PORT_CTRL_AP_INT_REG_802_3_AP_ST                         = 5,
    PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_1                  = 6,
    PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_2                  = 7,
    PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_3                  = 8,
    PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1         = 9,
    PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2         = 10,
    PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3         = 11,
    PORT_CTRL_AP_INT_REG_ANEG_CTRL_0                         = 12,
    PORT_CTRL_AP_INT_REG_ANEG_CTRL_1                         = 13,
    PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_REG                = 14,
    PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15          = 15,
    PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31         = 16,
    PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG        = 17,
    PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_0_15  = 18,
    PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_16_31 = 19,
    PORT_CTRL_AP_INT_REG_802_3_BP_ETH_ST_REG                 = 20,
    PORT_CTRL_AP_INT_REG_802_3_LP_ADV_REG                    = 21,
    PORT_CTRL_AP_REG_ST_1                                    = 22,
    PORT_CTRL_MAX_AP_REGS

}MV_PORT_CTRL_AP_REGS;


#define AP_CTRL_GET(param, mask, shift) (((param) >> (shift)) & (mask))
#define AP_CTRL_SET(param, mask, shift, val) \
{ \
    param &= (~((mask) << (shift))); \
    param |= (((val) & (mask)) << (shift)); \
}

/*
** AP Configuration
** ================
*/

/*
** Interface Number
** [00:07] Lane Number
** [08:15] PCS Number
** [16:23] MAC Number
** [24:31] Reserved
*/
#define AP_CTRL_LANE_MASK                  (0x7F)
#define AP_CTRL_LANE_SHIFT                 (0)
#define AP_CTRL_LANE_GET(ifNum)            AP_CTRL_GET(ifNum, AP_CTRL_LANE_MASK, AP_CTRL_LANE_SHIFT)
#define AP_CTRL_LANE_SET(ifNum, val)       AP_CTRL_SET(ifNum, AP_CTRL_LANE_MASK, AP_CTRL_LANE_SHIFT, val)

#define AP_CTRL_PCS_MASK                   (0x7F)
#define AP_CTRL_PCS_SHIFT                  (8)
#define AP_CTRL_PCS_GET(ifNum)             AP_CTRL_GET(ifNum, AP_CTRL_PCS_MASK, AP_CTRL_PCS_SHIFT)
#define AP_CTRL_PCS_SET(ifNum, val)        AP_CTRL_SET(ifNum, AP_CTRL_PCS_MASK, AP_CTRL_PCS_SHIFT, val)

#define AP_CTRL_MAC_MASK                   (0x7F)
#define AP_CTRL_MAC_SHIFT                  (16)
#define AP_CTRL_MAC_GET(ifNum)             AP_CTRL_GET(ifNum, AP_CTRL_MAC_MASK, AP_CTRL_MAC_SHIFT)
#define AP_CTRL_MAC_SET(ifNum, val)        AP_CTRL_SET(ifNum, AP_CTRL_MAC_MASK, AP_CTRL_MAC_SHIFT, val)

#define AP_CTRL_QUEUEID_MASK                   (0xFF)
#define AP_CTRL_QUEUEID_SHIFT                  (24)
#define AP_CTRL_QUEUEID_GET(ifNum)             AP_CTRL_GET(ifNum, AP_CTRL_QUEUEID_MASK, AP_CTRL_QUEUEID_SHIFT)
#define AP_CTRL_QUEUEID_SET(ifNum, val)        AP_CTRL_SET(ifNum, AP_CTRL_QUEUEID_MASK, AP_CTRL_QUEUEID_SHIFT, val)

/*
      ** Capability
      ** [00:00] Advertisement 1000BASE-KX                            0x0001
      ** [01:01] Advertisement 10GBase-KX4                            0x0002
      ** [02:02] Advertisement 10GBase-KR                             0x0004
      ** [03:03] Advertisement 40GBASE-KR4                            0x0008
      ** [04:04] Advertisement 40GBASE-CR4                            0x0010
      ** [05:05] Advertisement 100GBASE-CR10                          0x0020
      ** [06:06] Advertisement 100GBASE-KP4                           0x0040
      ** [07:07] Advertisement 100GBASE-KR4                           0x0080
      ** [08:08] Advertisement 100GBASE-CR4                           0x0100
      ** [09:09] Advertisement 25GBASE-KR-S or 25GBASE-CR-S           0x0200
      ** [10:10] Advertisement 25GBASE-KR or 25GBASE-CR               0x0400
      ** [11:11] Advertisement consortium 25GBase KR1                 0x0800
      ** [12:12] Advertisement consortium 25GBase CR1                 0x1000
      ** [13:13] Advertisement consortium 50GBase KR2                 0x2000
      ** [14:14] Advertisement consortium 50GBase CR2                 0x4000
      ** [15:15] Reserved
*/

#define AP_CTRL_ADV_MASK                   (0x1)
#define AP_CTRL_40GBase_KR4_SHIFT          (3)
#define AP_CTRL_40GBase_KR4_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_KR4_SHIFT)
#define AP_CTRL_40GBase_KR4_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_KR4_SHIFT, val)

#define AP_CTRL_10GBase_KR_SHIFT           (2)
#define AP_CTRL_10GBase_KR_GET(adv)        AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KR_SHIFT)
#define AP_CTRL_10GBase_KR_SET(adv, val)   AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KR_SHIFT, val)

#define AP_CTRL_10GBase_KX4_SHIFT          (1)
#define AP_CTRL_10GBase_KX4_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KX4_SHIFT)
#define AP_CTRL_10GBase_KX4_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KX4_SHIFT, val)

#define AP_CTRL_1000Base_KX_SHIFT          (0)
#define AP_CTRL_1000Base_KX_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_1000Base_KX_SHIFT)
#define AP_CTRL_1000Base_KX_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_1000Base_KX_SHIFT, val)

#define AP_CTRL_40GBase_CR4_SHIFT          (4)
#define AP_CTRL_40GBase_CR4_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_CR4_SHIFT)
#define AP_CTRL_40GBase_CR4_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_CR4_SHIFT, val)

#define AP_CTRL_100GBase_KP4_SHIFT           (6)
#define AP_CTRL_100GBase_KP4_GET(adv)        AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_KP4_SHIFT)
#define AP_CTRL_100GBase_KP4_SET(adv, val)   AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_KP4_SHIFT, val)

#define AP_CTRL_100GBase_KR4_SHIFT          (7)
#define AP_CTRL_100GBase_KR4_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_KR4_SHIFT)
#define AP_CTRL_100GBase_KR4_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_KR4_SHIFT, val)

#define AP_CTRL_100GBase_CR4_SHIFT          (8)
#define AP_CTRL_100GBase_CR4_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK,AP_CTRL_100GBase_CR4_SHIFT)
#define AP_CTRL_100GBase_CR4_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_CR4_SHIFT, val)




#define AP_CTRL_25GBase_KR1S_SHIFT         (9)
#define AP_CTRL_25GBase_KR1S_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1S_SHIFT)
#define AP_CTRL_25GBase_KR1S_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1S_SHIFT, val)

#define AP_CTRL_25GBase_KR1_SHIFT          (10)
#define AP_CTRL_25GBase_KR1_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1_SHIFT)
#define AP_CTRL_25GBase_KR1_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1_SHIFT, val)

/* rrr*/
#define AP_CTRL_25GBase_KR1_CONSORTIUM_SHIFT          (11)
#define AP_CTRL_25GBase_KR1_CONSORTIUM_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1_CONSORTIUM_SHIFT)
#define AP_CTRL_25GBase_KR1_CONSORTIUM_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1_CONSORTIUM_SHIFT, val)

#define AP_CTRL_25GBase_CR1_CONSORTIUM_SHIFT          (12)
#define AP_CTRL_25GBase_CR1_CONSORTIUM_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_CR1_CONSORTIUM_SHIFT)
#define AP_CTRL_25GBase_CR1_CONSORTIUM_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_CR1_CONSORTIUM_SHIFT, val)

#define AP_CTRL_50GBase_KR2_CONSORTIUM_SHIFT          (13)
#define AP_CTRL_50GBase_KR2_CONSORTIUM_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_KR2_CONSORTIUM_SHIFT)
#define AP_CTRL_50GBase_KR2_CONSORTIUM_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_KR2_CONSORTIUM_SHIFT, val)

#define AP_CTRL_50GBase_CR2_CONSORTIUM_SHIFT          (14)
#define AP_CTRL_50GBase_CR2_CONSORTIUM_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_CR2_CONSORTIUM_SHIFT)
#define AP_CTRL_50GBase_CR2_CONSORTIUM_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_CR2_CONSORTIUM_SHIFT, val)

#define AP_CTRL_ADV_CONSORTIUM_MASK                   (0xF)
#define AP_CTRL_ADV_CONSORTIUM_SHIFT                  (11)
#define AP_CTRL_ADV_CONSORTIUM_GET(adv)               AP_CTRL_GET(adv, AP_CTRL_ADV_CONSORTIUM_MASK, AP_CTRL_ADV_CONSORTIUM_SHIFT)
#define AP_CTRL_ADV_25G_CONSORTIUM_MASK               (0x3)
#define AP_CTRL_ADV_25G_CONSORTIUM_SHIFT              (11)
#define AP_CTRL_ADV_25G_CONSORTIUM_GET(adv)           AP_CTRL_GET(adv, AP_CTRL_ADV_25G_CONSORTIUM_MASK, AP_CTRL_ADV_25G_CONSORTIUM_SHIFT)
#define AP_CTRL_ADV_50G_CONSORTIUM_MASK               (0x3)
#define AP_CTRL_ADV_50G_CONSORTIUM_SHIFT              (13)
#define AP_CTRL_ADV_50G_CONSORTIUM_GET(adv)           AP_CTRL_GET(adv, AP_CTRL_ADV_50G_CONSORTIUM_MASK, AP_CTRL_ADV_50G_CONSORTIUM_SHIFT)




#define AP_CTRL_ADV_ALL_MASK               (0x7FFF)
#define AP_CTRL_ADV_ALL_SHIFT              (0)
#define AP_CTRL_ADV_ALL_GET(adv)           AP_CTRL_GET(adv, AP_CTRL_ADV_ALL_MASK, AP_CTRL_ADV_ALL_SHIFT)
#define AP_CTRL_ADV_ALL_SET(adv, val)      AP_CTRL_SET(adv, AP_CTRL_ADV_ALL_MASK, AP_CTRL_ADV_ALL_SHIFT, val)

/*
** Options
** [00:00] Flow Control Pause Enable
** [01:01] Flow Control Asm Direction
** [02:02] Remote Flow Control Pause Enable
** [03:03] Remote Flow Control Asm Direction
** [04:04] FEC Suppress En
** [05:05] FEC Request
** [06:07] Reserved  = RS-FEC requested, BASE-R FEC requested (for 25G IEEE)
** [08:08] CTLE Bias value
** [09:09] loopback Enable
** [10:10]consortium (RS-FEC) ability (F1)
** [11:11]consortium (BASE-R FEC) ability (F2)
** [12:12]consortium (RS-FEC) request (F3)
** [13:13]consortium (BASE-R FEC) request (F4)
** [14:15] noPpmMode  0 - MCD_NO_PPM_OFF_MODE         -noPpmMode disabled
**                    1 - MCD_NO_PPM_NORMAL_MODE      -regular noPpmMode
**                    2 - MCD_NO_PPM_RX_SQUELCH_MODE  -squelch noPpmMode - CM3 will squelch the rx clock in case of singal loss
**                    3 - MCD_NO_PPM_SEND_FAULT_MODE  -fault noPpmMode - CM3 will send fault in case of signal loss
**
*/
#define AP_CTRL_FC_PAUSE_MASK              (0x1)
#define AP_CTRL_FC_PAUSE_SHIFT             (0)
#define AP_CTRL_FC_PAUSE_GET(opt)          AP_CTRL_GET(opt, AP_CTRL_FC_PAUSE_MASK, AP_CTRL_FC_PAUSE_SHIFT)
#define AP_CTRL_FC_PAUSE_SET(opt, val)     AP_CTRL_SET(opt, AP_CTRL_FC_PAUSE_MASK, AP_CTRL_FC_PAUSE_SHIFT, val)

#define AP_CTRL_FC_ASM_MASK                (0x1)
#define AP_CTRL_FC_ASM_SHIFT               (1)
#define AP_CTRL_FC_ASM_GET(opt)            AP_CTRL_GET(opt, AP_CTRL_FC_ASM_MASK, AP_CTRL_FC_ASM_SHIFT)
#define AP_CTRL_FC_ASM_SET(opt, val)       AP_CTRL_SET(opt, AP_CTRL_FC_ASM_MASK, AP_CTRL_FC_ASM_SHIFT, val)

#define AP_CTRL_REM_FC_PAUSE_MASK          (0x1)
#define AP_CTRL_REM_FC_PAUSE_SHIFT         (2)
#define AP_CTRL_REM_FC_PAUSE_GET(opt)      AP_CTRL_GET(opt, AP_CTRL_REM_FC_PAUSE_MASK, AP_CTRL_REM_FC_PAUSE_SHIFT)
#define AP_CTRL_REM_FC_PAUSE_SET(opt, val) AP_CTRL_SET(opt, AP_CTRL_REM_FC_PAUSE_MASK, AP_CTRL_REM_FC_PAUSE_SHIFT, val)

#define AP_CTRL_REM_FC_ASM_MASK            (0x1)
#define AP_CTRL_REM_FC_ASM_SHIFT           (3)
#define AP_CTRL_REM_FC_ASM_GET(opt)        AP_CTRL_GET(opt, AP_CTRL_REM_FC_ASM_MASK, AP_CTRL_REM_FC_ASM_SHIFT)
#define AP_CTRL_REM_FC_ASM_SET(opt, val)   AP_CTRL_SET(opt, AP_CTRL_REM_FC_ASM_MASK, AP_CTRL_REM_FC_ASM_SHIFT, val)

#define AP_CTRL_FEC_ABIL_MASK              (0x1)
#define AP_CTRL_FEC_ABIL_SHIFT             (4)
#define AP_CTRL_FEC_ABIL_GET(opt)          AP_CTRL_GET(opt, AP_CTRL_FEC_ABIL_MASK, AP_CTRL_FEC_ABIL_SHIFT)
#define AP_CTRL_FEC_ABIL_SET(opt, val)     AP_CTRL_SET(opt, AP_CTRL_FEC_ABIL_MASK, AP_CTRL_FEC_ABIL_SHIFT, val)

#define AP_CTRL_FEC_REQ_MASK               (0x1)
#define AP_CTRL_FEC_REQ_SHIFT              (5)
#define AP_CTRL_FEC_REQ_GET(opt)           AP_CTRL_GET(opt, AP_CTRL_FEC_REQ_MASK, AP_CTRL_FEC_REQ_SHIFT)
#define AP_CTRL_FEC_REQ_SET(opt, val)      AP_CTRL_SET(opt, AP_CTRL_FEC_REQ_MASK, AP_CTRL_FEC_REQ_SHIFT, val)


#define AP_CTRL_FEC_FC_REQ_MASK            (0x1)
#define AP_CTRL_FEC_FC_REQ_SHIFT           (6)
#define AP_CTRL_FEC_FC_REQ_GET(opt)        AP_CTRL_GET(opt, AP_CTRL_FEC_FC_REQ_MASK, AP_CTRL_FEC_FC_REQ_SHIFT)
#define AP_CTRL_FEC_FC_REQ_SET(opt, val)   AP_CTRL_SET(opt, AP_CTRL_FEC_FC_REQ_MASK, AP_CTRL_FEC_FC_REQ_SHIFT, val)

#define AP_CTRL_FEC_RS_REQ_MASK            (0x1)
#define AP_CTRL_FEC_RS_REQ_SHIFT           (7)
#define AP_CTRL_FEC_RS_REQ_GET(opt)        AP_CTRL_GET(opt, AP_CTRL_FEC_RS_REQ_MASK, AP_CTRL_FEC_RS_REQ_SHIFT)
#define AP_CTRL_FEC_RS_REQ_SET(opt, val)   AP_CTRL_SET(opt, AP_CTRL_FEC_RS_REQ_MASK, AP_CTRL_FEC_RS_REQ_SHIFT, val)

#define AP_CTRL_FEC_ADVANCED_REQ_MASK           (0x3)
#define AP_CTRL_FEC_ADVANCED_REQ_SHIFT          (6)
#define AP_CTRL_FEC_ADVANCED_REQ_GET(opt)       AP_CTRL_GET(opt, AP_CTRL_FEC_ADVANCED_REQ_MASK, AP_CTRL_FEC_ADVANCED_REQ_SHIFT)
#define AP_CTRL_FEC_ADVANCED_REQ_SET(opt, val)  AP_CTRL_SET(opt, AP_CTRL_FEC_ADVANCED_REQ_MASK, AP_CTRL_FEC_ADVANCED_REQ_SHIFT, val)

#define AP_CTRL_CTLE_BIAS_VAL_MASK          (0x1)
#define AP_CTRL_CTLE_BIAS_VAL_SHIFT         (8)
#define AP_CTRL_CTLE_BIAS_VAL_GET(opt)      AP_CTRL_GET(opt, AP_CTRL_CTLE_BIAS_VAL_MASK, AP_CTRL_CTLE_BIAS_VAL_SHIFT)
#define AP_CTRL_CTLE_BIAS_VAL_SET(opt, val) AP_CTRL_SET(opt, AP_CTRL_CTLE_BIAS_VAL_MASK, AP_CTRL_CTLE_BIAS_VAL_SHIFT, val)


#define AP_CTRL_LB_EN_MASK                 (0x1)
#define AP_CTRL_LB_EN_SHIFT                (9)
#define AP_CTRL_LB_EN_GET(opt)             AP_CTRL_GET(opt, AP_CTRL_LB_EN_MASK, AP_CTRL_LB_EN_SHIFT)
#define AP_CTRL_LB_EN_SET(opt, val)        AP_CTRL_SET(opt, AP_CTRL_LB_EN_MASK, AP_CTRL_LB_EN_SHIFT, val)

#define AP_CTRL_RS_FEC_ABIL_CONSORTIUM_MASK          (0x1)
#define AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SHIFT         (10)
#define AP_CTRL_RS_FEC_ABIL_CONSORTIUM_GET(opt)      AP_CTRL_GET(opt, AP_CTRL_RS_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SHIFT)
#define AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SET(opt, val) AP_CTRL_SET(opt, AP_CTRL_RS_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SHIFT, val)

#define AP_CTRL_FC_FEC_ABIL_CONSORTIUM_MASK          (0x1)
#define AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SHIFT         (11)
#define AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(opt)      AP_CTRL_GET(opt, AP_CTRL_FC_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SHIFT)
#define AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SET(opt, val) AP_CTRL_SET(opt, AP_CTRL_FC_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SHIFT, val)

#define AP_CTRL_FEC_ABIL_CONSORTIUM_MASK              (0x3)
#define AP_CTRL_FEC_ABIL_CONSORTIUM_SHIFT             (10)
#define AP_CTRL_FEC_ABIL_CONSORTIUM_GET(opt)          AP_CTRL_GET(opt, AP_CTRL_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_FEC_ABIL_CONSORTIUM_SHIFT)
#define AP_CTRL_FEC_ABIL_CONSORTIUM_SET(opt, val)     AP_CTRL_SET(opt, AP_CTRL_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_FEC_ABIL_CONSORTIUM_SHIFT, val)

#define AP_CTRL_RS_FEC_REQ_CONSORTIUM_MASK           (0x1)
#define AP_CTRL_RS_FEC_REQ_CONSORTIUM_SHIFT          (12)
#define AP_CTRL_RS_FEC_REQ_CONSORTIUM_GET(opt)       AP_CTRL_GET(opt, AP_CTRL_RS_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_RS_FEC_REQ_CONSORTIUM_SHIFT)
#define AP_CTRL_RS_FEC_REQ_CONSORTIUM_SET(opt, val)  AP_CTRL_SET(opt, AP_CTRL_RS_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_RS_FEC_REQ_CONSORTIUM_SHIFT, val)

#define AP_CTRL_FC_FEC_REQ_CONSORTIUM_MASK           (0x1)
#define AP_CTRL_FC_FEC_REQ_CONSORTIUM_SHIFT          (13)
#define AP_CTRL_FC_FEC_REQ_CONSORTIUM_GET(opt)       AP_CTRL_GET(opt, AP_CTRL_FC_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_FC_FEC_REQ_CONSORTIUM_SHIFT)
#define AP_CTRL_FC_FEC_REQ_CONSORTIUM_SET(opt, val)  AP_CTRL_SET(opt, AP_CTRL_FC_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_FC_FEC_REQ_CONSORTIUM_SHIFT, val)

#define AP_CTRL_FEC_REQ_CONSORTIUM_MASK              (0x3)
#define AP_CTRL_FEC_REQ_CONSORTIUM_SHIFT             (12)
#define AP_CTRL_FEC_REQ_CONSORTIUM_GET(opt)          AP_CTRL_GET(opt, AP_CTRL_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_FEC_REQ_CONSORTIUM_SHIFT)
#define AP_CTRL_FEC_REQ_CONSORTIUM_SET(opt, val)     AP_CTRL_SET(opt, AP_CTRL_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_FEC_REQ_CONSORTIUM_SHIFT, val)

#define AP_CTRL_NO_PPM_MODE_MASK                    (0x3)
#define AP_CTRL_NO_PPM_MODE_SHIFT                   (14)
#define AP_CTRL_NO_PPM_MODE_GET(opt)                AP_CTRL_GET(opt, AP_CTRL_NO_PPM_MODE_MASK, AP_CTRL_NO_PPM_MODE_SHIFT)
#define AP_CTRL_NO_PPM_MODE_SET(opt, val)           AP_CTRL_SET(opt, AP_CTRL_NO_PPM_MODE_MASK, AP_CTRL_NO_PPM_MODE_SHIFT, val)

/*
** AP Status Info
** ==============
**/


/*
** HCD Status
** [03:03] Found
** [04:04] ARBSmError
** [05:05] reserved
** [06:06] Flow Control Rx Result
** [07:07] Flow Control Tx Result
** [08:19] Local HCD Type -------------------------------->        Port_1000Base_KX,
**                                                                 Port_10GBase_KX4,
**                                                                 Port_10GBase_R,
**                                                                 Port_25GBASE_KR_S,
**                                                                 Port_25GBASE_KR,
**                                                                 Port_40GBase_R,
**                                                                 Port_40GBASE_CR4,
**                                                                 Port_100GBASE_CR10,
**                                                                 Port_100GBASE_KP4,
**                                                                 Port_100GBASE_KR4,
**                                                                 Port_100GBASE_CR4,
**                                                                 Port_25GBASE_KR1_CONSORTIUM,
**                                                                 Port_25GBASE_CR1_CONSORTIUM,
**                                                                 Port_50GBASE_KR2_CONSORTIUM,
**                                                                 Port_50GBASE_CR2_CONSORTIUM,
** [20:20] Consortium result
** [22:23] FEC type --------------> [0:0] BASE-R FEC
**                                  [1:1] RS-FEC
** [29:29] HCD Interrupt Trigger
** [30:30] Link Interrupt Trigger
** [31:31] Link
**
*/
#define AP_ST_HCD_FOUND_MASK                 (0x1)
#define AP_ST_HCD_FOUND_SHIFT                (3)
#define AP_ST_HCD_FOUND_GET(hcd)             AP_CTRL_GET(hcd, AP_ST_HCD_FOUND_MASK, AP_ST_HCD_FOUND_SHIFT)
#define AP_ST_HCD_FOUND_SET(hcd, val)        AP_CTRL_SET(hcd, AP_ST_HCD_FOUND_MASK, AP_ST_HCD_FOUND_SHIFT, val)

#define AP_ST_AP_ERR_MASK                    (0x1)
#define AP_ST_AP_ERR_SHIFT                   (4)
#define AP_ST_AP_ERR_GET(hcd)                AP_CTRL_GET(hcd, AP_ST_AP_ERR_MASK, AP_ST_AP_ERR_SHIFT)
#define AP_ST_AP_ERR_SET(hcd, val)           AP_CTRL_SET(hcd, AP_ST_AP_ERR_MASK, AP_ST_AP_ERR_SHIFT, val)

#define AP_ST_HCD_FC_RX_RES_MASK             (0x1)
#define AP_ST_HCD_FC_RX_RES_SHIFT            (6)
#define AP_ST_HCD_FC_RX_RES_GET(hcd)         AP_CTRL_GET(hcd, AP_ST_HCD_FC_RX_RES_MASK, AP_ST_HCD_FC_RX_RES_SHIFT)
#define AP_ST_HCD_FC_RX_RES_SET(hcd, val)    AP_CTRL_SET(hcd, AP_ST_HCD_FC_RX_RES_MASK, AP_ST_HCD_FC_RX_RES_SHIFT, val)

#define AP_ST_HCD_FC_TX_RES_MASK             (0x1)
#define AP_ST_HCD_FC_TX_RES_SHIFT            (7)
#define AP_ST_HCD_FC_TX_RES_GET(hcd)         AP_CTRL_GET(hcd, AP_ST_HCD_FC_TX_RES_MASK, AP_ST_HCD_FC_TX_RES_SHIFT)
#define AP_ST_HCD_FC_TX_RES_SET(hcd, val)    AP_CTRL_SET(hcd, AP_ST_HCD_FC_TX_RES_MASK, AP_ST_HCD_FC_TX_RES_SHIFT, val)

#define AP_ST_HCD_TYPE_MASK                  (0xFFF)
#define AP_ST_HCD_TYPE_SHIFT                 (8)
#define AP_ST_HCD_TYPE_GET(hcd)              AP_CTRL_GET(hcd, AP_ST_HCD_TYPE_MASK, AP_ST_HCD_TYPE_SHIFT)
#define AP_ST_HCD_TYPE_SET(hcd, val)         AP_CTRL_SET(hcd, AP_ST_HCD_TYPE_MASK, AP_ST_HCD_TYPE_SHIFT, val)

#define AP_ST_HCD_CONSORTIUM_RES_MASK          (0x1)
#define AP_ST_HCD_CONSORTIUM_RES_SHIFT         (20)
#define AP_ST_HCD_CONSORTIUM_RES_GET(hcd)      AP_CTRL_GET(hcd, AP_ST_HCD_CONSORTIUM_RES_MASK, AP_ST_HCD_CONSORTIUM_RES_SHIFT)
#define AP_ST_HCD_CONSORTIUM_RES_SET(hcd, val) AP_CTRL_SET(hcd, AP_ST_HCD_CONSORTIUM_RES_MASK, AP_ST_HCD_CONSORTIUM_RES_SHIFT, val)

#define AP_ST_HCD_FEC_RES_NONE               (0x0)
#define AP_ST_HCD_FEC_RES_FC                 (0x1)
#define AP_ST_HCD_FEC_RES_RS                 (0x2)
#define AP_ST_HCD_FEC_RES_BOTH               (0x3)
#define AP_ST_HCD_FEC_RES_MASK               (0x3)
#define AP_ST_HCD_FEC_RES_SHIFT              (22)
#define AP_ST_HCD_FEC_RES_GET(hcd)           AP_CTRL_GET(hcd, AP_ST_HCD_FEC_RES_MASK, AP_ST_HCD_FEC_RES_SHIFT)
#define AP_ST_HCD_FEC_RES_SET(hcd, val)      AP_CTRL_SET(hcd, AP_ST_HCD_FEC_RES_MASK, AP_ST_HCD_FEC_RES_SHIFT, val)

/*#define AP_ST_HCD_FEC_CONSORTIUM_MASK          (0x3)
#define AP_ST_HCD_FEC_CONSORTIUM_SHIFT         (24)
#define AP_ST_HCD_FEC_CONSORTIUM_GET(hcd)      AP_CTRL_GET(hcd, AP_ST_HCD_FEC_CONSORTIUM_MASK, AP_ST_HCD_FEC_CONSORTIUM_SHIFT)
#define AP_ST_HCD_FEC_CONSORTIUM_SET(hcd, val) AP_CTRL_SET(hcd, AP_ST_HCD_FEC_CONSORTIUM_MASK, AP_ST_HCD_FEC_CONSORTIUM_SHIFT, val)
*/
#define AP_ST_HCD_INT_TRIG_MASK              (0x1)
#define AP_ST_HCD_INT_TRIG_SHIFT             (29)
#define AP_ST_HCD_INT_TRIG_GET(hcd)          AP_CTRL_GET(hcd, AP_ST_HCD_INT_TRIG_MASK, AP_ST_HCD_INT_TRIG_SHIFT)
#define AP_ST_HCD_INT_TRIG_SET(hcd, val)     AP_CTRL_SET(hcd, AP_ST_HCD_INT_TRIG_MASK, AP_ST_HCD_INT_TRIG_SHIFT, val)

#define AP_ST_LINK_INT_TRIG_MASK             (0x1)
#define AP_ST_LINK_INT_TRIG_SHIFT            (30)
#define AP_ST_LINK_INT_TRIG_GET(hcd)         AP_CTRL_GET(hcd, AP_ST_LINK_INT_TRIG_MASK, AP_ST_LINK_INT_TRIG_SHIFT)
#define AP_ST_LINK_INT_TRIG_SET(hcd, val)    AP_CTRL_SET(hcd, AP_ST_LINK_INT_TRIG_MASK, AP_ST_LINK_INT_TRIG_SHIFT, val)

#define AP_ST_HCD_LINK_MASK                  (0x1)
#define AP_ST_HCD_LINK_SHIFT                 (31)
#define AP_ST_HCD_LINK_GET(hcd)              AP_CTRL_GET(hcd, AP_ST_HCD_LINK_MASK, AP_ST_HCD_LINK_SHIFT)
#define AP_ST_HCD_LINK_SET(hcd, val)         AP_CTRL_SET(hcd, AP_ST_HCD_LINK_MASK, AP_ST_HCD_LINK_SHIFT, val)

/*
** ARBSmStatus
** [00:00] ST_AN_ENABLE
** [01:01] ST_TX_DISABLE
** [02:02] ST_LINK_STAT_CK
** [03:03] ST_PARALLEL_FLT
** [04:04] ST_ABILITY_DET
** [05:05] ST_ACK_DETECT
** [06:06] ST_COMPLETE_ACK
** [07:07] ST_NP_WAIT
** [08:08] ST_AN_GOOD_CK
** [09:09] ST_AN_GOOD
** [10:10] ST_SERDES_WAIT
*/
#define AP_ST_ARB_FSM_MASK                   (0x7FF)
#define AP_ST_ARB_FSM_SHIFT                  (0)
#define AP_ST_ARB_FSM_GET(arb)               AP_CTRL_GET(arb, AP_ST_ARB_FSM_MASK, AP_ST_ARB_FSM_SHIFT)
#define AP_ST_ARB_FSM_SET(arb, val)          AP_CTRL_SET(arb, AP_ST_ARB_FSM_MASK, AP_ST_ARB_FSM_SHIFT, val)

/*
** AP Status
** [00:00] Signal Detect
** [01:01] CDR Lock
** [02:02] PCS Lock
** [03:07] Reserved
*/
#ifdef __cplusplus
}
#endif

#endif /* __mcdFwServicesPortCtrlApDefs_H */




