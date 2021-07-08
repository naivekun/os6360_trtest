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
* @file mvHwsMMPcs28nmDb.h
*
* @brief Puma3 B0 MMPCS
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMMPcs28nmDb_H
#define __mvHwsMMPcs28nmDb_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDb.h>

/* PCS reset sequences */
extern MV_CFG_SEQ hwsMmPsc28nmSeqDb[MV_MM_PCS_LAST_SEQ];

GT_STATUS hwsMMPsc28nmSeqInit(void);
void hwsMMPcs28nmSeqFree(void);
GT_STATUS hwsMMPcs28nmSeqGet(MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsMMPcs28nmSeqSet(GT_BOOL firstLine, MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsMMPcs28nmDb_H */


