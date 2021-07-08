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
* @file mvHwsMMPcsDbRev3.h
*
* @brief Puma3 B0 MMPCS
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMMPcsDbRev3If_H
#define __mvHwsMMPcsDbRev3If_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDb.h>

/* PCS reset sequences */
extern MV_CFG_SEQ hwsMmPscSeqDbRev3[MV_MM_PCS_LAST_SEQ];

GT_STATUS hwsMMPcsRev3SeqGet(MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsMMPcsRev3SeqSet(GT_BOOL firstLine, MV_HWS_MM_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsMMPcsDbIf_H */


