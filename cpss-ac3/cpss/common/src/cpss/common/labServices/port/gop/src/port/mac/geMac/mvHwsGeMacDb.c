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
* @file mvHwsGeMacDb.c
*
* @brief
*
* @version   21
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>

static MV_CFG_ELEMENT unresetSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER2,            0x0,      (1 << 6)}}
};

static MV_CFG_ELEMENT resetSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER2,           (1 << 6),  (1 << 6)}}
};

static MV_CFG_ELEMENT xModeSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,           0x0,      (7 << 13)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,          (1 << 1),  (1 << 1)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0x9268,    0xFFFF,}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)}},
    {WRITE_OP, {GEMAC_UNIT,  MSM_PORT_MAC_CONTROL_REGISTER4,          (1 << 12), (1 << 12)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER2,           0x0,      (1)      }}  /* set InBandAutoNegMode to 1000X */
};

static MV_CFG_ELEMENT sgModeSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,           0x0,      (7 << 13)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,           0x0,      (1 << 1)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0xB0EC,    0xFFFF}},  /* Disable the AN-FlowControl, bit #11=0 */
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER2,           0x1,      (1)     }}  /* set InBandAutoNegMode to SGMII */
};

static MV_CFG_ELEMENT lbNormalSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 5)}}
};

static MV_CFG_ELEMENT lbTx2RxSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,          (1 << 5),  (1 << 5)}}
};

static MV_CFG_ELEMENT netxModeSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,          (1 << 1),  (1 << 1)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0x9268,    0xFFFF}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)}},
    {WRITE_OP, {GEMAC_UNIT,  MSM_PORT_MAC_CONTROL_REGISTER4,          (1 << 12), (1 << 12)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER2,           0x0,      (1)      }}  /* set InBandAutoNegMode to 1000X */
};

static MV_CFG_ELEMENT netsgModeSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,           0x0,      (1 << 1)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0xB0EC,    0xFFFF}},  /* Disable the AN-FlowControl, bit #11=0 */
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)}},
    {WRITE_OP, {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER2,           0x1,      (1)     }} /* set InBandAutoNegMode to SGMII */
};

/* For BC2 and AC3 only for XLG ports RX is configured via MAC - TX is controlled by FCA unit */
static MV_CFG_ELEMENT fcDisableSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER4          ,  (0 << 3),  (3 << 3)}},
    {WRITE_OP, {MMPCS_UNIT , 0x600 /* FCA Control register offset */,  (1 << 1),  (1 << 1)}} /* enable bypass FCA */
};
static MV_CFG_ELEMENT fcBothSeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER4 , (3 << 3),  (3 << 3)}},
    {WRITE_OP, {MMPCS_UNIT , 0x600                         , (0 << 1),  (1 << 1)}} /* disable bypass FCA */
};
static MV_CFG_ELEMENT fcRxOnlySeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER4 , (1 << 3),  (3 << 3)}},
    {WRITE_OP, {MMPCS_UNIT , 0x600                         , (1 << 1),  (1 << 1)}} /* enable bypass FCA */
};
static MV_CFG_ELEMENT fcTxOnlySeqParams[] = {
    {WRITE_OP, {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER4 , (2 << 3),  (3 << 3)}},
    {WRITE_OP, {MMPCS_UNIT , 0x600                         , (0 << 1),  (1 << 1)}} /* disable bypass FCA */
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsGeMacSeqDb[] =
{
    {unresetSeqParams,   MV_SEQ_SIZE(unresetSeqParams)},    /* GEMAC_UNRESET_SEQ     */
    {resetSeqParams,     MV_SEQ_SIZE(resetSeqParams)},      /* GEMAC_RESET_SEQ       */
    {xModeSeqParams,     MV_SEQ_SIZE(xModeSeqParams)},      /* GEMAC_X_MODE_SEQ      */
    {sgModeSeqParams,    MV_SEQ_SIZE(sgModeSeqParams)},     /* GEMAC_SG_MODE_SEQ     */
    {lbNormalSeqParams,  MV_SEQ_SIZE(lbNormalSeqParams)},   /* GEMAC_LPBK_NORMAL_SEQ */
    {NULL,               MV_SEQ_SIZE(0)},                   /* GEMAC_LPBK_RX2TX_SEQ  */
    {lbTx2RxSeqParams,   MV_SEQ_SIZE(lbTx2RxSeqParams)},    /* GEMAC_LPBK_TX2RX_SEQ  */
    {netxModeSeqParams,  MV_SEQ_SIZE(netxModeSeqParams)},   /* GEMAC_NET_X_MODE_SEQ  */
    {netsgModeSeqParams, MV_SEQ_SIZE(netsgModeSeqParams)},  /* GEMAC_NET_SG_MODE_SEQ */
    {fcDisableSeqParams, MV_SEQ_SIZE(fcDisableSeqParams)},  /* GEMAC_FC_DISABLE_SEQ */
    {fcBothSeqParams,    MV_SEQ_SIZE(fcBothSeqParams)},     /* GEMAC_FC_BOTH_SEQ */
    {fcRxOnlySeqParams,  MV_SEQ_SIZE(fcRxOnlySeqParams)},   /* GEMAC_FC_RX_ONLY_SEQ */
    {fcTxOnlySeqParams,  MV_SEQ_SIZE(fcTxOnlySeqParams)}    /* GEMAC_FC_TX_ONLY_SEQ */
};

GT_STATUS hwsGeMacSeqInit(GT_U8 devNum)
{
    /* GE MAC Sequences update */
    if (HWS_DEV_GOP_REV(devNum) < GOP_28NM_REV2)
    {
        hwsGeMacSeqDb[GEMAC_X_MODE_SEQ].cfgSeqSize = 4;
        hwsGeMacSeqDb[GEMAC_NET_X_MODE_SEQ].cfgSeqSize = 3;
    }

    return GT_OK;
}

GT_STATUS hwsGeMacSgSeqInit(GT_U8 devNum)
{
    return hwsGeMacSeqInit(devNum);
}

GT_STATUS hwsGeMacSeqGet(MV_HWS_GE_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
    if ((seqType >= MV_MAC_LAST_SEQ) ||
        (hwsGeMacSeqDb[seqType].cfgSeq == NULL))
    {
        return GT_NO_SUCH;
    }

    *seqLine = hwsGeMacSeqDb[seqType].cfgSeq[lineNum];
    return GT_OK;
}


