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
* @file mvHwsMtiPcsDb.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMtiPcsDb_H
#define __mvHwsMtiPcsDb_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    MTI_PCS_RESET_SEQ,
    MTI_PCS_UNRESET_SEQ,
    MTI_PCS_POWER_DOWN_SEQ,

    MTI_PCS_XG_MODE_SEQ,                /* 1G - 25G */
    MTI_PCS_XG_25_RS_FEC_MODE_SEQ,      /* 25G with RS-FEC 528 */
    MTI_PCS_XLG_50R1_MODE_SEQ,          /* 50G R1 */
    MTI_PCS_XLG_50R2_RS_FEC_MODE_SEQ,   /* 50G R2 with RS-FEC 528*/
    MTI_PCS_XLG_MODE_SEQ,               /* 40G - 50G all other*/
    MTI_PCS_CG_MODE_SEQ,                /* 100G  */

    MTI_PCS_200R4_MODE_SEQ,             /* 200G  4 lanes*/
    MTI_PCS_200R8_MODE_SEQ,             /* 200G  8 lanes*/
    MTI_PCS_400R8_MODE_SEQ,             /* 400G  8 lanes*/

    MTI_PCS_START_SEND_FAULT_SEQ,
    MTI_PCS_STOP_SEND_FAULT_SEQ,

    MTI_PCS_LPBK_NORMAL_SEQ,
    MTI_PCS_LPBK_TX2RX_SEQ,

    MV_MTI_PCS_LAST_SEQ

}MV_HWS_MTI_PCS_SUB_SEQ;


/* PCS reset sequences */
extern MV_CFG_SEQ hwsMtiPcs400SeqDb[MV_MTI_PCS_LAST_SEQ];
extern MV_CFG_SEQ hwsMtiPcs200SeqDb[MV_MTI_PCS_LAST_SEQ];
extern MV_CFG_SEQ hwsMtiPcs100SeqDb[MV_MTI_PCS_LAST_SEQ];
extern MV_CFG_SEQ hwsMtiPcs50SeqDb[MV_MTI_PCS_LAST_SEQ];

GT_STATUS hwsMtiPcs400SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsMtiPcs200SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsMtiPcs100SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsMtiPcs50SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

GT_STATUS hwsMtiPcs400SeqSet(GT_BOOL firstLine, MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvHwsMtiPcsDb_H */


