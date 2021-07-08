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
* mvHwsMti100MacDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvhwsMti100MacDbIf_H
#define __mvhwsMti100MacDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    MTI100_MAC_XLG_MODE_SEQ,       /* 50G */
    MTI100_MAC_XLG_POWER_DOWN_SEQ,
    MTI100_MAC_XLG_UNRESET_SEQ,
    MTI100_MAC_XG_MODE_SEQ,        /* 1G - 25G */
    MTI100_MAC_LAST_SEQ
}HWS_MTI_MAC100_SUB_SEQ;

/* MTI MAC sequences */
extern MV_CFG_SEQ hwsMti100MacSeqDb[MTI100_MAC_LAST_SEQ];

GT_STATUS hwsMti100MacSeqGet(HWS_MTI_MAC100_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvhwsMti100MacDbIf_H */

