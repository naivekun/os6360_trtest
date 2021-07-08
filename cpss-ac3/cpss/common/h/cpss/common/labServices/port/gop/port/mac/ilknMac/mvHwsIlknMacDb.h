/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvhwsIlknMacDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvhwsIlknMacDbIf_H
#define __mvhwsIlknMacDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
  INTLKN_MAC_UNRESET_SEQ,
  INTLKN_MAC_RESET_SEQ,
  INTLKN_MAC_MODE_12_LANES_SEQ,
  INTLKN_MAC_MODE_16_LANES_SEQ,
  INTLKN_MAC_MODE_24_LANES_SEQ,
  INTLKN_MAC_LPBK_NORMAL_SEQ,
  INTLKN_MAC_LPBK_TX2RX_SEQ,

  INTLKN_MAC_MODE_4_LANES_SEQ,
  INTLKN_MAC_MODE_8_LANES_SEQ,

  MV_MAC_INTKLN_LAST_SEQ

}MV_HWS_INTKLN_MAC_SUB_SEQ;

/* MAC reset sequences */
extern MV_CFG_SEQ hwsIlknMacSeqDb[MV_MAC_INTKLN_LAST_SEQ];

GT_STATUS hwsIlknMacSeqGet(MV_HWS_INTKLN_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsIlknMacSeqSet(GT_BOOL firstLine, MV_HWS_INTKLN_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvhwsIlknMacDbIf_H */

