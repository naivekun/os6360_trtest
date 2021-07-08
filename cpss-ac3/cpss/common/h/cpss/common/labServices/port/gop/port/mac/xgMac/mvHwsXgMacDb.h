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
* @file mvHwsXgMacDb.h
*
* @brief
*
* @version   6
********************************************************************************
*/

#ifndef __mvHwsXgMacDbIf_H
#define __mvHwsXgMacDbIf_H
#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    XGMAC_MODE_SEQ,

    MV_MAC_XG_LAST_SEQ

}MV_HWS_XG_MAC_SUB_SEQ;

/* MAC mode sequences */
extern MV_CFG_SEQ hwsXgMacSeqDb[MV_MAC_XG_LAST_SEQ];

GT_STATUS hwsXgMacSeqGet(MV_HWS_XG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsXgMacSeqSet(GT_BOOL firstLine, MV_HWS_XG_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
#ifdef __cplusplus
}
#endif

#endif /* __mvHwsXlgMacDbIf_H */


