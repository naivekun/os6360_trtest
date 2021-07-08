/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvCfgSeqDbIf.h
*
* DESCRIPTION:
*       Create configuration sequence DB APIs
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/

#ifndef __mvCfgSeqDbIf_H
#define __mvCfgSeqDbIf_H

#include "gtGenTypes.h"
#include "mvCfgElementDb.h"

#ifndef CO_CPU_RUN
/*******************************************************************************
* mvCreateWriteOp
*
* DESCRIPTION:
*      Create write operation
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void mvCreateWriteOp
(
    MV_CFG_SEQ          *seq,
    GT_U32              curSeqNum,
    MV_WRITE_OP_PARAMS  oper
);

/*******************************************************************************
* mvCreateDelayOp
*
* DESCRIPTION:
*      Create delay operation
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void mvCreateDelayOp
(
    MV_CFG_SEQ *seq,
    GT_U32   curSeqNum,
    GT_U32   delay
);

/*******************************************************************************
* mvCreatePollingOp
*
* DESCRIPTION:
*      Create polling operation
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void mvCreatePollingOp
(
    MV_CFG_SEQ            *seq,
    GT_U32                curSeqNum,
    MV_POLLING_OP_PARAMS  oper
);

/*******************************************************************************
* mvCreateSeqOperation
*
* DESCRIPTION:
*      Create any operation
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void mvCreateSeqOperation
(
    MV_CFG_SEQ            *seq,
    GT_U32                curSeqNum,
    MV_CFG_ELEMENT        *seqLine
);
#endif /* CO_CPU_RUN */
#endif /* __mvCfgSeqDbIf_H */

