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
* @file mvHwsXlgMac28nmDb.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsXlgMac28nmDbIf_H
#define __mvHwsXlgMac28nmDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>

/* MAC reset sequences */
extern MV_CFG_SEQ hwsXlgMac28nmSeqDb[MV_MAC_XLG_LAST_SEQ];

GT_STATUS hwsXlgMac28nmSeqInit(GT_U8 devNum);
GT_STATUS hwsXlgMac28nmSeqGet(MV_HWS_XLG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvHwsXlgMacDbIf_H */


