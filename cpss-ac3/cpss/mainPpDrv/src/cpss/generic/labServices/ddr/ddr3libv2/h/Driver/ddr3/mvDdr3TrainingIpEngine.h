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
* mvDdr3TrainingIpEngine.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
*
*******************************************************************************/

#ifndef _MV_DDR3_TRAININGIP_ENGINE_H_
#define _MV_DDR3_TRAININGIP_ENGINE_H_

#include "mvDdr3TrainingIpDef.h"
#include "mvDdr3TrainingIpFlow.h"

#ifdef __cplusplus
   extern "C"
   {
#endif


/******************************** defined ***************************************/
#define EDGE_1 (0)
#define EDGE_2 (1)
#define ALL_PUP_TRAINING                  (0xE)
#define PUP_RESULT_EDGE_1_MASK            (0xff)
#define PUP_RESULT_EDGE_2_MASK            (0xff << 8)
#define PUP_LOCK_RESULT_BIT               (25)
#define GET_TAP_RESULT(reg, edge)         (((edge) == EDGE_1) ? ((reg) & PUP_RESULT_EDGE_1_MASK):(((reg) & PUP_RESULT_EDGE_2_MASK) >> 8));
#define GET_LOCK_RESULT(reg)              (((reg) & (1<<PUP_LOCK_RESULT_BIT)) >> PUP_LOCK_RESULT_BIT)
#define EDGE_FAILURE                      (128)
#define ALL_BITS_PER_PUP                  (128)

#define MIN_WINDOW_SIZE                   (6)
#define MAX_WINDOW_SIZE_RX                (32)
#define MAX_WINDOW_SIZE_TX                (64)

GT_STATUS ddr3TipGetResult(GT_U32 *arResult, MV_HWS_Edge eEdge, MV_HWS_EdgeSearch eEdgeSearch, GT_U32 *pEdgeResult);

/******************************************************************************
* Name:     ddr3TipIpTraining.
* Desc:     IP Training search
* Args:     devNum - channel ID
*           accessType - if access type
*           pupAccessType - pup access
*           pupNum -  pup number
*           resultType - per bit/per byte
*           controlElement - control element (adll/dq/dsq)
*           searchDir - low2high/high2low
*           direction - tx or rx
*           interfaceMask - bitmask for selected interfaces
*           initValue - initial value
*           numIter - number of iterations
*           pattern - pattern type
*           edgeComp - type of compare: PF, FP, FPF, PFP
*           csType - multi/single
*           csNum -  CS number
*           trainStatus  training result
*
* Notes:
* Returns:  OK if success, other error code if fail.
*/

GT_STATUS ddr3TipTrainingIpTest
(
    GT_U32                      devNum,
    MV_HWS_TRAINING_RESULT      resultType,
    MV_HWS_SearchDirection      searchDir,
    MV_HWS_DIRECTION            direction,
    MV_HWS_EdgeCompare          edge,
    GT_U32                      initVal1,
    GT_U32                      initVal2,
    GT_U32                      numOfItertaions,
    GT_U32                      startPattern,
    GT_U32                      endPattern
);

/******************************************************************************
* Name:     ddr3TipLoadPatternToMem.
* Desc:     Load expected Pattern to external memory
* Args:
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipLoadPatternToMem
(
    GT_U32          devNum,
    MV_HWS_PATTERN  pattern
);

/******************************************************************************
* load pattern to memory using ODPG
*/
GT_STATUS    ddr3TipLoadAllPatternToMem
(
    GT_U32          devNum
);

/******************************************************************************
* Name:     ddr3TipReadTrainingResult.
* Desc:     Load/Unload training ip result
* Args:     devNum
*           interfaceId
*           pupAccessType
*           pupNum
*           search
*           direction
*           resultType
*           loadRes
*           isReadFromDB
*
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipReadTrainingResult
(
    GT_U32                          devNum,
    GT_U32                          interfaceId,
    MV_HWS_ACCESS_TYPE              pupAccessType,
    GT_U32                          pupNum,
    GT_U32                          bitNum,
    MV_HWS_SearchDirection          search,
    MV_HWS_DIRECTION                direction,
    MV_HWS_TRAINING_RESULT          resultType,
    MV_HWS_TrainingLoadOperation    operation,
    GT_U32                          csNumType,
    GT_U32                          **loadRes,
    GT_BOOL                         isReadFromDB,
    GT_U8                           consTap,
    GT_BOOL                         isCheckResultValidity
);


/******************************************************************************
* Name:     ddr3TipIpTraining.
* Desc:     IP Training search
* Args:     TBD
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipIpTraining
(
    GT_U32                      devNum,
    MV_HWS_ACCESS_TYPE          accessType,
    GT_U32                      interfaceNum,
    MV_HWS_ACCESS_TYPE          pupAccessType,
    GT_U32                      pupNum,
    MV_HWS_TRAINING_RESULT      resultType,
    MV_HWS_ControlElement       controlElement,
    MV_HWS_SearchDirection      searchDir,
    MV_HWS_DIRECTION            direction,
    GT_U32                      interfaceMask,
    GT_U32                      initValue,
    GT_U32                      numIter,
    MV_HWS_PATTERN              pattern,
    MV_HWS_EdgeCompare          edgeComp,
    MV_HWS_DDR_CS               csType,
    GT_U32                      csNum,
    MV_HWS_TrainingIpStatus*    trainStatus
);

GT_STATUS    ddr3TipIpTrainingWrapper
(
    GT_U32                      devNum,
    MV_HWS_ACCESS_TYPE          accessType,
    GT_U32                      interfaceId,
    MV_HWS_ACCESS_TYPE          pupAccessType,
    GT_U32                      pupNum,
    MV_HWS_TRAINING_RESULT      resultType,
    MV_HWS_ControlElement       controlElement,
    MV_HWS_SearchDirection      searchDir,
    MV_HWS_DIRECTION            direction,
    GT_U32                      interfaceMask,
    GT_U32                      initValue1,
    GT_U32                      initValue2,
    GT_U32                      numIter,
    MV_HWS_PATTERN              pattern,
    MV_HWS_EdgeCompare          edgeComp,
    MV_HWS_DDR_CS               trainCsType,
    GT_U32                      csNum,
    MV_HWS_TrainingIpStatus*    trainStatus
);

GT_STATUS isOdpgAccessDone
(
    GT_U32 devNum,
    GT_U32 interfaceId
);

/******************************************************************************
* print Bist Result
*/
void    ddr3TipPrintBistRes
(
);

/******************************************************************************
* get tables functions
*/
PatternInfo*  ddr3TipGetPatternTable();
GT_U16 *ddr3TipGetMaskResultsDqReg();
GT_U16 *ddr3TipGetMaskResultsPupRegMap();


#ifdef __cplusplus
   }
#endif

/*****************************************************************************/
#endif /* _MV_DDR3_TRAININGIP_ENGINE_H_ */
