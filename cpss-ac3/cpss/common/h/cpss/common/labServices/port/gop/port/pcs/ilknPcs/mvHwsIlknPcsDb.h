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
* @file mvHwsIlknPcsDb.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsIlknPcsDbIf_H
#define __mvHwsIlknPcsDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>


typedef enum
{
  ILKNPCS_RESET_SEQ,
  ILKNPCS_UNRESET_SEQ,

  ILKNPCS_MODE_MISC_SEQ,
  ILKNPCS_MODE_12_LANE_SEQ,
  ILKNPCS_MODE_16_LANE_SEQ,
  ILKNPCS_MODE_24_LANE_SEQ,

  ILKNPCS_MODE_4_LANE_SEQ,
  ILKNPCS_MODE_8_LANE_SEQ,

  MV_ILKNPCS_LAST_SEQ
}MV_HWS_ILKNPCS_SUB_SEQ;

extern MV_CFG_SEQ hwsIlknPcsPscSeqDb[MV_ILKNPCS_LAST_SEQ];

GT_STATUS hwsIlknPcsSeqGet(MV_HWS_ILKNPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsIlknPcsSeqSet(GT_BOOL firstLine, MV_HWS_ILKNPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsXPcsDbIf_H */


