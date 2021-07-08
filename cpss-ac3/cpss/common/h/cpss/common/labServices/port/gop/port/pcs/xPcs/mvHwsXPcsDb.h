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
* @file mvHwsXPcsDb.h
*
* @brief
*
* @version   7
********************************************************************************
*/

#ifndef __mvHwsXPcsDbIf_H
#define __mvHwsXPcsDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>


typedef enum
{
  XPCS_RESET_SEQ,
  XPCS_UNRESET_SEQ,

  XPCS_MODE_MISC_SEQ,
  XPCS_MODE_1_LANE_SEQ,
  XPCS_MODE_2_LANE_SEQ,
  XPCS_MODE_4_LANE_SEQ,

  XPCS_LPBK_NORMAL_SEQ,
  XPCS_LPBK_RX2TX_SEQ,
  XPCS_LPBK_TX2RX_SEQ,

  XPCS_GEN_PRBS7_SEQ,
  XPCS_GEN_PRBS23_SEQ,
  XPCS_GEN_CJPAT_SEQ,
  XPCS_GEN_CRPAT_SEQ,
  XPCS_GEN_NORMAL_SEQ,

  MV_XPCS_LAST_SEQ
}MV_HWS_XPCS_SUB_SEQ;

extern MV_CFG_SEQ hwsXpcsPscSeqDb[MV_XPCS_LAST_SEQ];

GT_STATUS hwsXPcsSeqGet(MV_HWS_XPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsXPcsSeqSet(GT_BOOL firstLine, MV_HWS_XPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsXPcsDbIf_H */


