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
* @file mvHwsMMPcsDb.h
*
* @brief
*
* @version   9
********************************************************************************
*/

#ifndef __mvHwsMMPcsDbIf_H
#define __mvHwsMMPcsDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    MMPCS_RESET_SEQ,
    MMPCS_UNRESET_SEQ,

    MMPCS_MODE_1_LANE_SEQ,
    MMPCS_MODE_2_LANE_SEQ,
    MMPCS_MODE_4_LANE_SEQ,

    MMPCS_LPBK_NORMAL_SEQ,
    MMPCS_LPBK_RX2TX_SEQ,
    MMPCS_LPBK_TX2RX_SEQ,

    MMPCS_GEN_NORMAL_SEQ,
    MMPCS_GEN_KRPAT_SEQ,

    MMPCS_FABRIC_MODE_1_LANE_SEQ,
    MMPCS_FABRIC_MODE_2_LANE_SEQ,
    MMPCS_FABRIC_MODE_4_LANE_SEQ,

    MMPCS_RESET_1_LANE_SEQ,
    MMPCS_RESET_2_LANE_SEQ,
    MMPCS_RESET_4_LANE_SEQ,
    MMPCS_UNRESET_1_LANE_SEQ,
    MMPCS_UNRESET_2_LANE_SEQ,
    MMPCS_UNRESET_4_LANE_SEQ,

    MMPCS_FABRIC_RESET_2_LANE_SEQ,
    MMPCS_FABRIC_RESET_4_LANE_SEQ,
    MMPCS_FABRIC_UNRESET_2_LANE_SEQ,
    MMPCS_FABRIC_UNRESET_4_LANE_SEQ,

    MMPCS_GOP_REV3_UNRESET_1_LANE_SEQ,
    MMPCS_GOP_REV3_UNRESET_2_LANE_SEQ,
    MMPCS_GOP_REV3_UNRESET_4_LANE_SEQ,

    MMPCS_POWER_DOWN_2_LANE_SEQ,
    MMPCS_POWER_DOWN_4_LANE_SEQ,
    MMPCS_FABRIC_POWER_DOWN_2_LANE_SEQ,

    MMPCS_MODE_1_LANE_25G_SEQ,
    MMPCS_MODE_4_LANE_50G_SEQ,
    MMPCS_RESET_1_LANE_25G_SEQ,
    MMPCS_RESET_4_LANE_50G_SEQ,
    MMPCS_POWER_DOWN_1_LANE_25G_SEQ,
    MMPCS_POWER_DOWN_4_LANE_50G_SEQ,
    MMPCS_UNRESET_1_LANE_25G_SEQ,

    MMPCS_START_SEND_FAULT_MULTI_LANE_SEQ,
    MMPCS_STOP_SEND_FAULT_MULTI_LANE_SEQ,
    MMPCS_STOP_SEND_FAULT_MULTI_LANE_RS_FEC_SEQ,

    MMPCS_UNRESET_1_LANE_26_7G_SEQ,

    MV_MM_PCS_LAST_SEQ

}MV_HWS_MM_PCS_SUB_SEQ;

/* PCS reset sequences */
extern MV_CFG_SEQ hwsMmPscSeqDb[MV_MM_PCS_LAST_SEQ];

GT_STATUS hwsMMPcsSeqGet(MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsMMPcsSeqSet(GT_BOOL firstLine, MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsMMPcsDbIf_H */


