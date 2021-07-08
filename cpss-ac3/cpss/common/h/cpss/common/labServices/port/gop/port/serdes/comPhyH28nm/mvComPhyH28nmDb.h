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
* mvSerdes28nmDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/

#ifndef __mvSerdes28nmDb_H
#define __mvSerdes28nmDb_H


#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

GT_STATUS hwsComPhyH28nmSeqInit(void);
GT_STATUS hwsComPhy28nmSeqGet(MV_HWS_COM_PHY_H_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);

GT_STATUS hwsComPhyH28nmInitSeqFlow(MV_CFG_SEQ* hwSerdesSeqDbArr);

GT_STATUS hwsComPhyH28nmSpeedSeqInit
(
    MV_CFG_SEQ*               hwSerdesSeqDbArr,
    MV_SPEED_CONFIG_REG_DATA* hwSerdesSpeedExtConfig,
    GT_U32                    extConfigNum,
    MV_SPEED_CONFIG_REG_DATA* hwSerdesSpeedIntConfig,
    GT_U32                    intConfigNum
);

#ifdef __cplusplus
}
#endif

#endif /* __mvSerdes28nmDb_H */

