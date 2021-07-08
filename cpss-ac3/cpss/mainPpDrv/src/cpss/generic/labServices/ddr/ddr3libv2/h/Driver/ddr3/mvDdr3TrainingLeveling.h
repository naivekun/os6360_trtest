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
* mvDdr3TrainingLeveling.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/

#ifndef _MV_DDR3_TRAINING_LEVELING_H_
#define _MV_DDR3_TRAINING_LEVELING_H_



#ifdef __cplusplus
   extern "C"
   {
#endif

#define MAX_DQ_READ_LEVELING_DELAY 15

GT_BOOL ddr3TipPrintWLSuppResult
(
    GT_U32 devNum
);

GT_STATUS    ddr3TipCalcCsMask
(
    GT_U32    devNum,
    GT_U32    interfaceId,
    GT_U32    effectiveCs,
    GT_U32    *csMask
);


GT_U32 mvHwsDdr3TipMaxCSGet(GT_U32 devNum);

#ifdef __cplusplus
   }
#endif

/*****************************************************************************/

#endif /* _MV_DDR3_TRAININGIP_H_ */


