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
* mvhwsTcamMacDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvhwsTcamMacDbIf_H
#define __mvhwsTcamMacDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
  TCAMMAC_UNRESET_SEQ,
  TCAMMAC_RESET_SEQ,
  TCAMMAC_MODE_SEQ,

  MV_MAC_TCAM_LAST_SEQ

}MV_HWS_TCAM_MAC_SUB_SEQ;

/* MAC reset sequences */
extern MV_CFG_SEQ hwsTcamMacSeqDb[MV_MAC_TCAM_LAST_SEQ];

GT_STATUS hwsTcamMacSeqGet(MV_HWS_TCAM_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvhwsTcamMacDbIf_H */

