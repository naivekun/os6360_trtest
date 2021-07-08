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
* @file mvHwsHglMacDb.h
*
* @brief
*
* @version   6
********************************************************************************
*/

#ifndef __mvHwsHglMacDbIf_H
#define __mvHwsHglMacDbIf_H
#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
  HGLMAC_MODE_SEQ,

  MV_MAC_HGL_LAST_SEQ

}MV_HWS_HGL_MAC_SUB_SEQ;


/* MAC mode sequences */
extern MV_CFG_SEQ hwsHglMacSeqDb[MV_MAC_HGL_LAST_SEQ];

GT_STATUS hwsHglMacSeqGet(MV_HWS_HGL_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsHglMacSeqSet(GT_BOOL firstLine, MV_HWS_HGL_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvHwsHglMacDbIf_H */


