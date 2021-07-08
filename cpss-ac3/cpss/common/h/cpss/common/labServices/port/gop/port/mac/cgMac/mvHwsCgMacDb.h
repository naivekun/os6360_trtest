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
* mvhwsCgMacDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvhwsCgMacDbIf_H
#define __mvhwsCgMacDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    CGMAC_UNRESET_SEQ,
    CGMAC_RESET_SEQ,
    CGMAC_MODE_SEQ,
    CGMAC_LPBK_NORMAL_SEQ,
    CGMAC_LPBK_TX2RX_SEQ,
    CGMAC_XLGMAC_LPBK_NORMAL_SEQ,
    CGMAC_XLGMAC_LPBK_RX2TX_SEQ,
    CGMAC_XLGMAC_LPBK_TX2RX_SEQ,
    CGMAC_MODE_1_LANE_SEQ,
    CGMAC_MODE_2_LANE_SEQ,
    CGMAC_MODE_4_LANE_SEQ,
    CGMAC_POWER_DOWN_1_LANE_SEQ,
    CGMAC_POWER_DOWN_2_LANE_SEQ,
    CGMAC_POWER_DOWN_4_LANE_SEQ,

    CGMAC_FC_DISABLE_SEQ,
    CGMAC_FC_BOTH_SEQ,
    CGMAC_FC_RX_ONLY_SEQ,
    CGMAC_FC_TX_ONLY_SEQ,

    MV_MAC_CG_LAST_SEQ

}MV_HWS_CG_MAC_SUB_SEQ;

/* MAC reset sequences */
extern MV_CFG_SEQ hwsCgMacSeqDb[MV_MAC_CG_LAST_SEQ];

GT_STATUS hwsCgMacSeqGet(MV_HWS_CG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsCgMacSeqSet(GT_BOOL firstLine, MV_HWS_CG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvhwsCgMacDbIf_H */

