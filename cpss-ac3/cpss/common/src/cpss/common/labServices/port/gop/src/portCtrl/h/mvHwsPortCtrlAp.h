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
* @file mvHwsPortCtrlAp.h
*
* @brief Port Control AP Definitions
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlAp_H
#define __mvHwsPortCtrlAp_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>

/*******************************************************************************
*                         AP Register Definition                               *
* ============================================================================ *
*******************************************************************************/

#ifdef MARVELL_AP_IP
/* AP Control Regsiters */
/* ==================== */

/* PCS40G Common Control */
#define PCS40G_COMMON_CTRL_REG                                (0x0014)
/* PCS40G Common Control Field definition */
#define PCS40G_COMMON_CTRL_REG_PCS_MODE_MASK                  (0x0300)
#define PCS40G_COMMON_CTRL_REG_PCS_MODE_SHIFT                 (8)

/* PCS40G Common Status */
#define PCS40G_COMMON_STATUS_REG                              (0x0030)
/* PCS40G Common Status Field definition */
#define PCS40G_COMMON_STATUS_REG_PCS_MODE_MASK                (0x0001)

/* AP configuration reg 0 */
#define AP_REG_CFG_0                                          (0x013C)
/* AP configuration reg 0 Field definition */
#define AP_REG_CFG_0_ADV_MASK                                 (0x3F0)
#define AP_REG_CFG_0_ADV_40G_KR4_SHIFT                        (4)
#define AP_REG_CFG_0_ADV_10G_KR_SHIFT                         (5)
#define AP_REG_CFG_0_ADV_10G_KX4_SHIFT                        (6)
#define AP_REG_CFG_0_ADV_1G_KX_SHIFT                          (7)
#define AP_REG_CFG_0_ADV_40G_KR4_MASK                         (0x0010)
#define AP_REG_CFG_0_ADV_10G_KR_MASK                          (0x0020)
#define AP_REG_CFG_0_ADV_10G_KX4_MASK                         (0x0040)
#define AP_REG_CFG_0_ADV_1G_KX_MASK                           (0x0080)
#define AP_REG_CFG_0_ADV_20G_KR2_MASK                         (0x0100)
#define AP_REG_CFG_0_ADV_10G_KX2_MASK                         (0x0200)

#define AP_REG_CFG_0_LOCK_MASK                                (0xF000)
#define AP_REG_CFG_0_LOCK_10G_KX2_MASK                        (0x1000)
#define AP_REG_CFG_0_LOCK_20G_KR2_MASK                        (0x2000)
#define AP_REG_CFG_0_LOCK_40G_KR4_MASK                        (0x4000)
#define AP_REG_CFG_0_LOCK_1G_KX_MASK                          (0x8000)

/* AP configuration reg 1 */
#define AP_REG_CFG_1                                          (0x0140)
#define AP_REG_CFG_1_ADV_100G_CR10_SHIFT                      (13)
#define AP_REG_CFG_1_ADV_40G_CR4_SHIFT                        (12)
/* AP configuration reg 1 Field definition */
#define AP_REG_CFG_1_ADV_MASK                                 (0x3000)
#define AP_REG_CFG_1_ADV_100G_CR10_MASK                       (0x2000)
#define AP_REG_CFG_1_ADV_40G_CR4_MASK                         (0x1000)
#define AP_REG_CFG_1_LOCK_10G_KR_MASK                         (0x0200)
#define AP_REG_CFG_1_LOCK_10G_KX4_MASK                        (0x0100)
#define AP_REG_CFG_1_LOCK_40G_CR4_MASK                        (0x0400)
#define AP_REG_CFG_1_LOCK_100G_CR10_MASK                      (0x0800)
#define AP_REG_CFG_1_LOCK_MASK                                (0x0F00)
/* AP configuration reg 2 */
#define AP_REG_CFG_2                                          (0x0150)
#define AP_REG_CFG_2_ADV_100G_KP4_SHIFT                        (0)
#define AP_REG_CFG_2_ADV_100G_KR4_SHIFT                        (1)
#define AP_REG_CFG_2_ADV_100G_CR4_SHIFT                        (2)
#define AP_REG_CFG_2_ADV_25G_S_SHIFT                           (3)
#define AP_REG_CFG_2_ADV_25G_SHIFT                             (4)
/* AP configuration reg 2 Field definition */
#define AP_REG_CFG_2_ADV_MASK                                 (0x001F)
#define AP_REG_CFG_2_ADV_100G_KP4_MASK                        (0x0001)
#define AP_REG_CFG_2_ADV_100G_KR4_MASK                        (0x0002)
#define AP_REG_CFG_2_ADV_100G_CR4_MASK                        (0x0004)
#define AP_REG_CFG_2_ADV_25G_S_MASK                           (0x0008)
#define AP_REG_CFG_2_ADV_25G_MASK                             (0x0010)
/* AP configuration reg 3 */
#define AP_REG_CFG_3                                          (0x0154)
#define AP_REG_CFG_3_LOCK_SHIFT                               (0)
/* AP configuration reg 3 Field definition */
#define AP_REG_CFG_3_LOCK_MASK                                 (0x001F)
#define AP_REG_CFG_3_LOCK_100G_KP4_MASK                        (0x0001)
#define AP_REG_CFG_3_LOCK_100G_KR4_MASK                        (0x0002)
#define AP_REG_CFG_3_LOCK_100G_CR4_MASK                        (0x0004)
#define AP_REG_CFG_3_LOCK_25G_S_MASK                           (0x0008)
#define AP_REG_CFG_3_LOCK_25G_MASK                             (0x0010)

/* AP status reg 0 */
#define AP_REG_ST_0                                           (0x0144)
/* AP status reg 0 Field definition */
#define AP_REG_ST_0_AP_PWT_UP_MASK                            (0x080F)
#define AP_REG_ST_0_AP_PWT_UP_40G_CR4_MASK                    (0x0800)
#define AP_REG_ST_0_AP_PWT_UP_10G_KX4_MASK                    (0x0008)
#define AP_REG_ST_0_AP_PWT_UP_1G_KX_MASK                      (0x0004)
#define AP_REG_ST_0_AP_PWT_UP_40G_KR4_MASK                    (0x0002)
#define AP_REG_ST_0_AP_PWT_UP_10G_KR_MASK                     (0x0001)

#define AP_REG_ST_0_HCD_BASE_R_FEC_MASK                       (0x0001)
#define AP_REG_ST_0_HCD_BASE_R_FEC_SHIFT                      (5)

#define AP_REG_ST_0_HCD_RESOLVED_MASK                         (0x0001)
#define AP_REG_ST_0_HCD_RESOLVED_SHIFT                        (13)

/* AP status reg 1 */
#define AP_REG_ST_1                                           (0x0148)
/* AP status reg 1 Field definition */
#define AP_REG_ST_1_AP_PWT_UP_MASK                            (0x003E)
#define AP_REG_ST_1_AP_PWT_UP_100G_KP4_MASK                   (0x0002)
#define AP_REG_ST_1_AP_PWT_UP_100G_KR4_MASK                   (0x0004)
#define AP_REG_ST_1_AP_PWT_UP_100G_CR4_MASK                   (0x0008)
#define AP_REG_ST_1_AP_PWT_UP_25G_S_MASK                      (0x0010)
#define AP_REG_ST_1_AP_PWT_UP_25G_MASK                        (0x0020)
#define AP_REG_ST_1_LINK_UP_MASK                              (0x0001)
#define AP_REG_ST_1_LINK_UP_SHIFT                               (0)

#define AP_REG_ST_1_HCD_RS_FEC_MASK                           (0x0001)
#define AP_REG_ST_1_HCD_RS_FEC_SHIFT                          (9)

    /* PCS Clock Reset */
#define PCS_CLOCK_RESET_REG                                   (0x014C)
/* PCS Clock Reset Field definition */
#define PCS_CLOCK_RESET_REG_AP_RESET_MASK                     (0x0008)
#define PCS_CLOCK_RESET_REG_AP_RESET_SHIFT                    (3)

/* AP Internal Registers */
/* ===================== */

/* 802.3ap Auto-Negotiation Control */
#define AP_INT_REG_802_3_AP_CTRL                              (0x0200)
/* 802.3ap Auto-Negotiation Control Field definition */
#define AP_INT_REG_802_3_AP_CTRL_RST_MASK                     (0x8000)
#define AP_INT_REG_802_3_AP_CTRL_RST_SHIFT                    (15)
#define AP_INT_REG_802_3_AP_CTRL_AN_ENA_MASK                  (0x1000)
#define AP_INT_REG_802_3_AP_CTRL_AN_ENA_SHIFT                 (12)
#define AP_INT_REG_802_3_AP_CTRL_AN_RST_MASK                  (0x0200)
#define AP_INT_REG_802_3_AP_CTRL_AN_RST_SHIFT                 (9)

/* 802.3ap Auto-Negotiation Status */
#define AP_INT_REG_802_3_AP_ST                                (0x0204)

/* 802.3ap Auto-Negotiation Advertisement Register 1 */
#define AP_INT_REG_802_3_AP_ADV_REG_1                         (0x0240)
/* 802.3ap Auto-Negotiation Advertisement Register 1 Field definition */
#define AP_INT_REG_802_3_AP_ADV_REG_1_FC_PAUSE_MASK           (0x0C00)
#define AP_INT_REG_802_3_AP_ADV_REG_1_FC_PAUSE_SHIFT          (10)

#define AP_INT_REG_802_3_AP_ADV_REG_1_NP_MASK                 (0x8000)
#define AP_INT_REG_802_3_AP_ADV_REG_1_NP_SHIFT                (15)
/* 802.3ap Auto-Negotiation Advertisement Register 2 */
#define AP_INT_REG_802_3_AP_ADV_REG_2                         (0x0244)

/* 802.3ap Auto-Negotiation Advertisement Register 3 */
#define AP_INT_REG_802_3_AP_ADV_REG_3                         (0x0248)
/* 802.3ap Auto-Negotiation Advertisement Register 3 Field definition */
#define AP_INT_REG_802_3_AP_ADV_REG_3_FEC_MASK                (0xC000)
#define AP_INT_REG_802_3_AP_ADV_REG_3_FEC_SHIFT               (14)
#define AP_INT_REG_802_3_AP_ADV_REG_3_FEC_ADV_MASK            (0x3000)
#define AP_INT_REG_802_3_AP_ADV_REG_3_FEC_ADV_SHIFT           (12)


/* 802.3ap Link Partner Base Page Ability Register 1 */
#define AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1                (0x024C)
/* 802.3ap Link Partner Base Page Ability Register 1 Field definition */
#define AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1_FC_PAUSE_SHIFT (10)
#define AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1_FC_DIR_SHIFT   (11)

/* 802.3ap Link Partner Base Page Ability Register 2 */
#define AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2                (0x0250)

/* 802.3ap Link Partner Base Page Ability Register 3 */
#define AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3                (0x0254)

/* 802.3ap Next Page Transmit Register / Extended Next Page Transmit Register */
#define AP_INT_REG_802_3_NEXT_PG_TX_REG                       (0x0258)
/* 802.3ap Next Page Transmit Register Field definition */
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_CNS   (5)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_NULL_CNS (1)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS (0x203)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK       (0x7FF)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_T_SHIFT               (11)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_T_MASK                (0x800)

#define AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_SHIFT            (12)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_MASK             (0x1000)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_SHIFT              (13)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK               (0x2000)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK_SHIFT             (14)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK_MASK              (0x4000)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_SHIFT              (15)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK               (0x8000)
#define AP_INT_REG_802_3_NEXT_PG_TX_REG_CTRL_BIT_MASK         (0xF000)

/* 802.3ap Extended Next Page Transmit Register Unformatted Code Field U0 to U15 */
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15                 (0x025C)
/* 802.3ap Extended Next Page Transmit Register Field definition */
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_SHIFT       (4)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_MASK        (0x30)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_KR1_MASK    (0x10)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_CR1_MASK    (0x20)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_SHIFT       (8)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_MASK        (0x300)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_MASK        (0x300)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_KR2_MASK    (0x100)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_CR2_MASK    (0x200)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_OUI_TAGGED_FORM_CNS (0x353)

/* 802.3ap Extended Next Page Transmit Register Unformatted Code Field U16 to U31 */
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31                  (0x0260)
/* 802.3ap Extended Next Page Transmit Register Field definition */
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_SHIFT        (8)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_MASK         (0xF00)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_ABIL_MASK (0x100)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_ABIL_MASK (0x200)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_REQ_MASK  (0x400)
#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_REQ_MASK  (0x800)

#define AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_OUI_TAGGED_FORM_CNS (0x4DF)

/* 802.3ap Link Partner Next Page Register / Link Partner Extended Next Page Ability Register */
#define AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG               (0x0264)

/* 802.3ap Link Partner Extended Next Page Ability Register Unformatted Code Field U0 to U15 */
#define AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_0_15         (0x0268)

/* 802.3ap Link Partner Extended Next Page Ability Register Unformatted Code Field U16 to U31 */
#define AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_16_31        (0x026C)

/* Backplane Ethernet Status Register */
#define AP_INT_REG_802_3_BP_ETH_ST_REG                        (0x0270)

/* EEE Link Partner Advertisement Register */
#define AP_INT_REG_802_3_LP_ADV_REG                           (0x0278)

/* ANEG Control Register 0 */
#define AP_INT_REG_ANEG_CTRL_0                                (0x0280)
/* ANEG Control Register 0  Field definition */
#define AP_INT_REG_ANEG_CTRL_0_ANEG_MASK                      (0x7FFF)
#define AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_MASK                (0x07FF)
#define AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_SHIFT               (4)

/* ANEG Control Register 1 */
#define AP_INT_REG_ANEG_CTRL_1                                (0x0284)
/* ANEG Control Register 1  Field definition */
#define AP_INT_REG_ANEG_CTRL_1_PD_CAP_MASK                    (0x03C0)
#define AP_INT_REG_ANEG_CTRL_1_LB_NONCE_MATCH_MASK            (0x0400)
#define AP_INT_REG_ANEG_CTRL_1_LB_NONCE_MATCH_SHIFT           (10)

#endif

/* AP ARB State Machine Numbering Definition */
typedef enum
{
    ST_AN_ENABLE    = 0x001,
    ST_TX_DISABLE   = 0x002,
    ST_LINK_STAT_CK = 0x004,
    ST_PARALLEL_FLT = 0x008,
    ST_ABILITY_DET  = 0x010,
    ST_ACK_DETECT   = 0x020,
    ST_COMPLETE_ACK = 0x040,
    ST_NP_WAIT      = 0x080,
    ST_AN_GOOD_CK   = 0x100,
    ST_AN_GOOD      = 0x200,
    ST_SERDES_WAIT  = 0x400

}MV_HWS_AP_SM_STATE;

/* AP Port Mode Definition */
typedef enum
{
     Port_1000Base_KX,
     Port_10GBase_KX4,
     Port_10GBase_R,
     Port_25GBASE_KR_S,
     Port_25GBASE_KR,
     Port_40GBase_R,
     Port_40GBASE_CR4,
     Port_100GBASE_CR10,
     Port_100GBASE_KP4,
     Port_100GBASE_KR4,
     Port_100GBASE_CR4,
     Port_25GBASE_KR_C,    /*25GBASE_KR_CONSORTIUM*/
     Port_25GBASE_CR_C,    /*25GBASE_CR_CONSORTIUM*/
     Port_50GBASE_KR2_C,   /*50GBASE_KR2_CONSORTIUM*/
     Port_50GBASE_CR2_C,   /*50GBASE_CR2_CONSORTIUM*/
     Port_25GBASE_CR_S,
     Port_25GBASE_CR,
     MODE_NOT_SUPPORTED,
     Port_AP_LAST = MODE_NOT_SUPPORTED
}MV_HWA_AP_PORT_MODE;

/*******************************************************************************
*                         AP API Definition                                    *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mvHwsApDetectStateSet function
* @endinternal
*
*/
void mvHwsApDetectStateSet(GT_U32 state);

/**
* @internal mvHwsApDetectRoutine function
* @endinternal
*
*/
void mvHwsApDetectRoutine(void* pvParameters);

/**
* @internal mvHwsApPortEnableCtrlSet function
* @endinternal
*
*/
void mvHwsApPortEnableCtrlSet(GT_BOOL portEnableCtrl);

/**
* @internal mvHwsApPortEnableCtrlGet function
* @endinternal
*
*/
void mvHwsApPortEnableCtrlGet(GT_BOOL *portEnableCtrl);


/* AP Detection External State machine States API's */
/* ================================================ */

/**
* @internal mvPortCtrlApDetectMsg function
* @endinternal
*
*/
void mvPortCtrlApDetectMsg(void);

/**
* @internal mvPortCtrlApDetectActive function
* @endinternal
*
*/
void mvPortCtrlApDetectActive(void);

/**
* @internal mvPortCtrlApDetectDelay function
* @endinternal
*
*/
void mvPortCtrlApDetectDelay(void);


/* AP Detection Port State machine States API's */
/* ============================================ */

/**
* @internal mvHwsInitialConfiguration function
* @endinternal
*
*/
GT_STATUS mvHwsInitialConfiguration(GT_U8 portIndex);

/**
* @internal mvApPortIdle function
* @endinternal
*
*/
GT_STATUS mvApPortIdle(GT_U8 portIndex);

/**
* @internal mvApPortInit function
* @endinternal
*
*/
GT_STATUS mvApPortInit(GT_U8 portIndex);

/**
* @internal mvApPortTxDisable function
* @endinternal
*
*/
GT_STATUS mvApPortTxDisable(GT_U8 portIndex);

/**
* @internal mvApPortResolution function
* @endinternal
*
*/
GT_STATUS mvApPortResolution(GT_U8 portIndex);

/**
* @internal mvApPortLinkUp function
* @endinternal
*
*/
GT_STATUS mvApPortLinkUp(GT_U8 portIndex);

/**
* @internal mvApPortDeleteValidate function
* @endinternal
*
*/
GT_STATUS mvApPortDeleteValidate(GT_U8 portIndex);

/**
* @internal mvApPortDeleteMsg function
* @endinternal
*
*/
GT_STATUS mvApPortDeleteMsg(GT_U8 portIndex);

/**
* @internal mvHwsApSetPortParameters function
* @endinternal
*
*/
GT_STATUS mvHwsApSetPortParameters(GT_U32 portNum, MV_HWA_AP_PORT_MODE apPortMode);

/**
* @internal mvPortCtrlApPortMsgSend function
* @endinternal
*
*/
GT_STATUS mvPortCtrlApPortMsgSend(GT_U8 portIndex, GT_U8 msgType, GT_U8 queueId,
                                  GT_U8 portNum, GT_U8 portMode, GT_U8 action,
                                  GT_U8 refClk, GT_U8 refClkSrc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlAp_H */

