/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvDdr3TrainingAdditional.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef _MV_DDR3_TRAINING_HW_ALGO_H_
#define _MV_DDR3_TRAINING_HW_ALGO_H_



#ifdef __cplusplus
   extern "C"
   {
#endif

/*****************************************************************************
Vref training
******************************************************************************/
GT_STATUS ddr3TipVref
(
    GT_U32 devNum
);


/*****************************************************************************
ODT additional timing
******************************************************************************/
GT_STATUS    ddr3TipWriteAdditionalOdtSetting
(
    GT_U32                  devNum,
    GT_U32                  interfaceId
) ;


/*****************************************************************************
CK/CA Delay
******************************************************************************/
GT_STATUS    ddr3TipCmdAddrInitDelay
(
    GT_U32      devNum,
    GT_U32      adllTap
);

#ifdef __cplusplus
   }
#endif

/*****************************************************************************/

#endif /* _MV_DDR3_TRAINING_HW_ALGO_H_ */


