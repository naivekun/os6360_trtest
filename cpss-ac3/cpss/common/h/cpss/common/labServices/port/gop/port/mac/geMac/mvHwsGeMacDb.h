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
*******************************************************************************
* mvHwsMacDbIf.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*
*******************************************************************************/

#ifndef __mvHwsMacDbIf_H
#define __mvHwsMacDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>


typedef enum
{
  GEMAC_UNRESET_SEQ,
  GEMAC_RESET_SEQ,

  GEMAC_X_MODE_SEQ,
  GEMAC_SG_MODE_SEQ,

  GEMAC_LPBK_NORMAL_SEQ,
  GEMAC_LPBK_RX2TX_SEQ,
  GEMAC_LPBK_TX2RX_SEQ,

  GEMAC_NET_X_MODE_SEQ,
  GEMAC_NET_SG_MODE_SEQ,

  GEMAC_FC_DISABLE_SEQ,
  GEMAC_FC_BOTH_SEQ,
  GEMAC_FC_RX_ONLY_SEQ,
  GEMAC_FC_TX_ONLY_SEQ,

  MV_MAC_LAST_SEQ

}MV_HWS_GE_MAC_SUB_SEQ;

extern MV_CFG_SEQ hwsGeMacSeqDb[MV_MAC_LAST_SEQ];

GT_STATUS hwsGeMacSeqInit(GT_U8 devNum);
GT_STATUS hwsGeMacSeqGet(MV_HWS_GE_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsGeMacSeqSet(GT_BOOL firstLine, MV_HWS_GE_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

GT_STATUS hwsGeMacSgSeqInit(GT_U8 devNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvHwsMacDbIf_H */

