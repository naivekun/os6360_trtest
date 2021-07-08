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
* @file mvHwsHglPcsDb.h
*
* @brief
*
* @version   6
********************************************************************************
*/

#ifndef __mvHwsHglPcsDbIf_H
#define __mvHwsHglPcsDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
  HGLPCS_MODE_MISC_SEQ,
  HGLPCS_MODE_4_LANE_SEQ,
  HGLPCS_MODE_6_LANE_SEQ,

  MV_HGLPCS_LAST_SEQ

}MV_HWS_HGLPCS_SUB_SEQ;


extern MV_CFG_SEQ hwsHglPscSeqDb[MV_HGLPCS_LAST_SEQ];

GT_STATUS hwsHglPcsSeqGet(MV_HWS_HGLPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsHglPcsSeqSet(GT_BOOL firstLine, MV_HWS_HGLPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsHglPcsDbIf_H */


