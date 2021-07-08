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
* @file mvHwsXlgMacDb.h
*
* @brief
*
* @version   6
********************************************************************************
*/

#ifndef __mvHwsXlgMacDbIf_H
#define __mvHwsXlgMacDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    XLGMAC_UNRESET_SEQ,
    XLGMAC_RESET_SEQ,

    XLGMAC_MODE_1_Lane_SEQ,
    XLGMAC_MODE_2_Lane_SEQ,
    XLGMAC_MODE_4_Lane_SEQ,

    XLGMAC_LPBK_NORMAL_SEQ,
    XLGMAC_LPBK_RX2TX_SEQ,
    XLGMAC_LPBK_TX2RX_SEQ,

    XLGMAC_FC_DISABLE_SEQ,
    XLGMAC_FC_BOTH_SEQ,
    XLGMAC_FC_RX_ONLY_SEQ,
    XLGMAC_FC_TX_ONLY_SEQ,

    XLGMAC_POWER_DOWN_SEQ,

    XLGMAC_MODE_1_Lane_25G_SEQ,
    XLGMAC_MODE_2_Lane_50G_SEQ,

    XLGMAC_MODE_1_Lane_25G_RS_FEC_SEQ,
    XLGMAC_MODE_2_Lane_50G_RS_FEC_SEQ,

    XLGMAC_MODE_4_Lane_MLG_SEQ,
    XLGMAC_RESET_4_Lane_MLG_SEQ,
    XLGMAC_UNRESET_4_Lane_MLG_SEQ,
    XLGMAC_POWER_DOWN_4_Lane_MLG_SEQ,

    MV_MAC_XLG_LAST_SEQ

}MV_HWS_XLG_MAC_SUB_SEQ;

/* MAC reset sequences */
extern MV_CFG_SEQ hwsXlgMacSeqDb[MV_MAC_XLG_LAST_SEQ];

GT_STATUS hwsXlgMacSeqGet(MV_HWS_XLG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsXlgMacSeqSet(GT_BOOL firstLine, MV_HWS_XLG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvHwsXlgMacDbIf_H */


