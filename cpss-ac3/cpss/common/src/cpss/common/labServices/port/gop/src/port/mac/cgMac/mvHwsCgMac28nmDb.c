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
* @file mvHwsCgMac28nmDb.c
*
* @brief
*
* @version   7
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmDb.h>

static MV_CFG_ELEMENT xlgMacMode1LaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER4,             0x0,        0x1000}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,    0x880F,     0xF81F}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_METAL_FIX,                         0x200,      0x1E00}}
};

static MV_CFG_ELEMENT xlgMacMode2LaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER4,             0xC00,      0x1E00}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,    0x880F,     0xF81F}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_METAL_FIX,                         0x200,      0x1E00}}
};

static MV_CFG_ELEMENT cgMacMode4LaneSeqParams[] = {
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,       CG_COMMAND_CONFIG,                          0x201D3,    0xFFFFFFFF}},
    {WRITE_OP, {CG_UNIT,       CG_FRM_LENGTH,                              0x2820,     0xFFFF}},
    {WRITE_OP, {CG_UNIT,       CG_TX_FIFO_SECTIONS,                        0x5,        0xFF}}
};

static MV_CFG_ELEMENT xlgMacResetLaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER0,             0x0,        0x2}}
};

static MV_CFG_ELEMENT xlgMacUnresetLaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER0,             0x2,        0x2}}
};

static MV_CFG_ELEMENT xlgMacPowerDown1LaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER0,             0x0,        0x1}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER4,             0x1000,     0x1000}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,    0x7803,     0xF81F}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_METAL_FIX,                         0x0,        0x1E00}}
};

static MV_CFG_ELEMENT xlgMacPowerDown2LaneSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER0,             0x0,        0x1}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER4,             0x1200,     0x1E00}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION,    0x7803,     0xF81F}},
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_METAL_FIX,                         0x0,        0x1E00}}
};

static MV_CFG_ELEMENT xlgMacPowerDown4LaneSeqParams[] = {
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,       CG_COMMAND_CONFIG,                          0x0,        0xFFFFFFFF}},
    {WRITE_OP, {CG_UNIT,       CG_FRM_LENGTH,                              0x600,      0xFFFF}},
    {WRITE_OP, {CG_UNIT,       CG_TX_FIFO_SECTIONS,                        0x10,       0xFF}}/*,
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x0,        0x4000000}},*/  /*this reset must be released before accessing MTI specific Register Files*/
};

static MV_CFG_ELEMENT cgMaclbNormalSeqParams[] = {
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,       CG_COMMAND_CONFIG,                          0x0,        0x400}}
};

static MV_CFG_ELEMENT cgMaclbTx2RxSeqParams[] = {
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {CG_UNIT,       CG_COMMAND_CONFIG,                          0x400,      0x400}}
};

static MV_CFG_ELEMENT xlgMaclbNormalSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER1,             0x0,        (3 << 13)}}
};

static MV_CFG_ELEMENT xlgMaclbRx2TxSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER1,             (2 << 13),  (3 << 13)}}
};

static MV_CFG_ELEMENT xlgMaclbTx2RxSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT,   MSM_PORT_MAC_CONTROL_REGISTER1,             (1 << 13),  (3 << 13)}}
};

static MV_CFG_ELEMENT cgMacFcDisableSeqParams[] = {
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {MMPCS_UNIT,    0x600 /* FCA Control register offset */,    (1 << 1),   (1 << 1)}},  /* enable bypass FCA */
    {WRITE_OP, {CG_UNIT,       CG_FC_CONTROL_0,                            0,          0x8000}}
};
static MV_CFG_ELEMENT cgMacFcBothSeqParams[] = {
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {MMPCS_UNIT,    0x600,                                      (0 << 1),   (1 << 1)}},  /* disable bypass FCA */
    {WRITE_OP, {CG_UNIT,       CG_FC_CONTROL_0,                            0x8000,     0x8000}}
};
static MV_CFG_ELEMENT cgMacFcRxOnlySeqParams[] = {
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {MMPCS_UNIT,    0x600,                                      (1 << 1),   (1 << 1)}},  /* enable bypass FCA */
    {WRITE_OP, {CG_UNIT,       CG_FC_CONTROL_0,                            0x8000,     0x8000}}
};
static MV_CFG_ELEMENT cgMacFcTxOnlySeqParams[] = {
    {WRITE_OP, {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}}, /*this reset must be released before accessing MTI specific Register Files*/
    {WRITE_OP, {MMPCS_UNIT,    0x600,                                      (0 << 1),   (1 << 1)}},  /* disable bypass FCA */
    {WRITE_OP, {CG_UNIT,       CG_FC_CONTROL_0,                            0x0000,     0x8000}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsCgMac28nmSeqDb[MV_MAC_CG_LAST_SEQ] =
{
    {xlgMacUnresetLaneSeqParams,       MV_SEQ_SIZE(xlgMacUnresetLaneSeqParams)},       /* CGMAC_UNRESET_SEQ            */
    {xlgMacResetLaneSeqParams,         MV_SEQ_SIZE(xlgMacResetLaneSeqParams)},         /* CGMAC_RESET_SEQ              */
    {cgMacMode4LaneSeqParams,          MV_SEQ_SIZE(cgMacMode4LaneSeqParams)},          /* CGMAC_MODE_SEQ               */
    {cgMaclbNormalSeqParams,           MV_SEQ_SIZE(cgMaclbNormalSeqParams)},           /* CGMAC_LPBK_NORMAL_SEQ        */
    {cgMaclbTx2RxSeqParams,            MV_SEQ_SIZE(cgMaclbTx2RxSeqParams)},            /* CGMAC_LPBK_TX2RX_SEQ         */
    {xlgMaclbNormalSeqParams,          MV_SEQ_SIZE(xlgMaclbNormalSeqParams)},          /* CGMAC_XLGMAC_LPBK_NORMAL_SEQ */
    {xlgMaclbRx2TxSeqParams,           MV_SEQ_SIZE(xlgMaclbRx2TxSeqParams)},           /* CGMAC_XLGMAC_LPBK_RX2TX_SEQ  */
    {xlgMaclbTx2RxSeqParams,           MV_SEQ_SIZE(xlgMaclbTx2RxSeqParams)},           /* CGMAC_XLGMAC_LPBK_TX2RX_SEQ  */
    {xlgMacMode1LaneSeqParams,         MV_SEQ_SIZE(xlgMacMode1LaneSeqParams)},         /* CGMAC_MODE_1_LANE_SEQ        */
    {xlgMacMode2LaneSeqParams,         MV_SEQ_SIZE(xlgMacMode2LaneSeqParams)},         /* CGMAC_MODE_2_LANE_SEQ        */
    {cgMacMode4LaneSeqParams,          MV_SEQ_SIZE(cgMacMode4LaneSeqParams)},          /* CGMAC_MODE_4_LANE_SEQ        */
    {xlgMacPowerDown1LaneSeqParams,    MV_SEQ_SIZE(xlgMacPowerDown1LaneSeqParams)},    /* CGMAC_POWER_DOWN_1_LANE_SEQ  */
    {xlgMacPowerDown2LaneSeqParams,    MV_SEQ_SIZE(xlgMacPowerDown2LaneSeqParams)},    /* CGMAC_POWER_DOWN_2_LANE_SEQ  */
    {xlgMacPowerDown4LaneSeqParams,    MV_SEQ_SIZE(xlgMacPowerDown4LaneSeqParams)},     /* CGMAC_POWER_DOWN_4_LANE_SEQ  */
    {cgMacFcDisableSeqParams,          MV_SEQ_SIZE(cgMacFcDisableSeqParams)},          /* CGMAC_FC_DISABLE_SEQ         */
    {cgMacFcBothSeqParams,             MV_SEQ_SIZE(cgMacFcBothSeqParams)},             /* CGMAC_FC_BOTH_SEQ            */
    {cgMacFcRxOnlySeqParams,           MV_SEQ_SIZE(cgMacFcRxOnlySeqParams)},           /* CGMAC_FC_RX_ONLY_SEQ         */
    {cgMacFcTxOnlySeqParams,           MV_SEQ_SIZE(cgMacFcTxOnlySeqParams)},           /* CGMAC_FC_TX_ONLY_SEQ         */
};

GT_STATUS hwsCgMac28nmSeqInit(GT_U8 devNum)
{
    MV_OP_PARAMS* updateParams;
    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        updateParams = (MV_OP_PARAMS*)&hwsCgMac28nmSeqDb[CGMAC_MODE_4_LANE_SEQ].cfgSeq[0].params;
        updateParams->operData = 0x44000000;
        updateParams->mask = 0x44000000;
    }

    return GT_OK;
}

GT_STATUS hwsCgMac28nmSeqGet(MV_HWS_CG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_MAC_CG_LAST_SEQ) ||
      (hwsCgMac28nmSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsCgMac28nmSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
