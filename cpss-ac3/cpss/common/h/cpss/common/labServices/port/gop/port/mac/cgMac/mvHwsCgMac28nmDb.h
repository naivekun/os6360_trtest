/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvhwsCgMac28nmDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvhwsCgMac28nmDbIf_H
#define __mvhwsCgMac28nmDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacDb.h>

/* CG MAC sequences */
extern MV_CFG_SEQ hwsCgMac28nmSeqDb[MV_MAC_CG_LAST_SEQ];

GT_STATUS hwsCgMac28nmSeqInit(GT_U8 devNum);
GT_STATUS hwsCgMac28nmSeqGet(MV_HWS_CG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvhwsCgMac28nmDbIf_H */

