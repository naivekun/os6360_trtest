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
* @file mvHwsGPcsDb.h
*
* @brief
*
* @version   7
********************************************************************************
*/

#ifndef __mvHwsGPcsDbIf_H
#define __mvHwsGPcsDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
  GPCS_RESET_SEQ,
  GPCS_UNRESET_SEQ,

  GPCS_LPBK_NORMAL_SEQ,
  GPCS_LPBK_TX2RX_SEQ,

  GPCS_GEN_PRBS7_SEQ,
  GPCS_GEN_NORMAL_SEQ,

  MV_GPCS_LAST_SEQ
}MV_HWS_GPCS_SUB_SEQ;

extern MV_CFG_SEQ hwsGPscSeqDb[MV_GPCS_LAST_SEQ];

GT_STATUS hwsGPscSeqInit(void);
void hwsGPcsSeqFree(void);
GT_STATUS hwsGPcsSeqGet(MV_HWS_GPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsGPcsSeqSet(GT_BOOL firstLine, MV_HWS_GPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsGPcsDbIf_H */


