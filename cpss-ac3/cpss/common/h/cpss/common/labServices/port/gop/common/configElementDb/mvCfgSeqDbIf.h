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
* @file mvCfgSeqDbIf.h
*
* @brief Create configuration sequence DB APIs
*
* @version   5
********************************************************************************
*/

#ifndef __mvCfgSeqDbIf_H
#define __mvCfgSeqDbIf_H

#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

#ifndef CO_CPU_RUN
/**
* @internal mvCreateWriteOp function
* @endinternal
*
* @brief   Create write operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvCreateWriteOp
(
    MV_CFG_SEQ          *seq,
    GT_U32              curSeqNum,
    MV_WRITE_OP_PARAMS  oper
);

/**
* @internal mvCreateDelayOp function
* @endinternal
*
* @brief   Create delay operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvCreateDelayOp
(
    MV_CFG_SEQ *seq,
    GT_U32   curSeqNum,
    GT_U32   delay
);

/**
* @internal mvCreatePollingOp function
* @endinternal
*
* @brief   Create polling operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvCreatePollingOp
(
    MV_CFG_SEQ            *seq,
    GT_U32                curSeqNum,
    MV_POLLING_OP_PARAMS  oper
);

/**
* @internal mvGetSeqOperation function
* @endinternal
*
* @brief   Get sequence operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvGetSeqOperation
(
    MV_CFG_SEQ     *seq,
    MV_CFG_ELEMENT *seqLine,
    GT_U32         lineNum
);

/**
* @internal mvCreateSeqOperation function
* @endinternal
*
* @brief   Create any operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvCreateSeqOperation
(
    MV_CFG_SEQ     *seq,
    GT_U32         curSeqNum,
    MV_CFG_ELEMENT *seqLine
);

/**
* @internal mvReAllocateSeqOperation function
* @endinternal
*
* @brief   ReAllocate memory for any operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvReAllocateSeqOperation
(
    MV_CFG_SEQ     *seq,
    GT_U32         curSeqNum,
    GT_U32         curSeqSize,
    MV_CFG_ELEMENT *seqLine
);

/**
* @internal mvUpdateSeqOperation function
* @endinternal
*
* @brief   Update any operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvUpdateSeqOperation
(
    MV_CFG_SEQ     *seq,
    GT_U32         curSeqNum,
    MV_CFG_ELEMENT *seqLine
);

/**
* @internal getCfgSeqOp function
* @endinternal
*
* @brief   Returns the configuration operation,
*         according to the waitTime and the numOfLoops fields in params
* @param[in] waitTime                 - the wait time, in case of a delay or polling operation
* @param[in] numOfLoops               - the number of loops, in case of a polling operation
*
* @retval WRITE_OP                 - in case there is no wait (waitTime = 0)
* @retval DELAY_OP                 - in case there are no loops (numOfLoops = 0)
* @retval POLLING_OP               - otherwise
*/
MV_EL_DB_OPERATION getCfgSeqOp
(
    GT_U32 waitTime,
    GT_U32 numOfLoops
);
#endif /* CO_CPU_RUN */
#endif /* __mvCfgSeqDbIf_H */


