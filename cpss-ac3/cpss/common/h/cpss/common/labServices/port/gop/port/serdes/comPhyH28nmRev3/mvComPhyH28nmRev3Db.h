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
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvSerdes28nmRev3Db_H
#define __mvSerdes28nmRev3Db_H


#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

#define IS_SERDES_SPEED_BELOW_10G(data)     ((data != 0x1FFA) && (data != 0x1F72))

GT_STATUS hwsComPhyH28nmRev3SeqInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __mvSerdes28nmDb_H */

