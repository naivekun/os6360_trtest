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
* @file mvHwsTcamPcsDb.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsTcamPcsDbIf_H
#define __mvHwsTcamPcsDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>


typedef enum
{
  TCAMPCS_RESET_SEQ,
  TCAMPCS_UNRESET_SEQ,

  TCAMPCS_MODE_MISC_SEQ,

  MV_TCAMPCS_LAST_SEQ

}MV_HWS_TCAMPCS_SUB_SEQ;

extern MV_CFG_SEQ hwsTcamPcsPscSeqDb[MV_TCAMPCS_LAST_SEQ];

GT_STATUS hwsTcamPcsSeqGet(MV_HWS_TCAMPCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsXPcsDbIf_H */


