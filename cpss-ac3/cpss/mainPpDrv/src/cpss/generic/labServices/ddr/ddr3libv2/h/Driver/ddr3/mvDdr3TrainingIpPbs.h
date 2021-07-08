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
* mvDdr3TrainingIP.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 13 $
*
*******************************************************************************/

#ifndef __mvDdr3TrainingIP_PBS_H
#define __mvDdr3TrainingIP_PBS_H

typedef enum
{
    EBA_CONFIG,
    EEBA_CONFIG,
    SBA_CONFIG
} MV_HWS_PBS_STATE;

typedef enum
{
    PBS_RD_ADDR         = 0x2000,
    PBS_WR_ADDR         = 0x2100,
    CENTRALIZATION_ADDR = 0x3000
}MV_HWS_PATTERN_ADDRESS;

typedef enum
{
    TrainingLoadOperation_UNLOAD,
    TrainingLoadOperation_LOAD
}MV_HWS_TrainingLoadOperation;

typedef enum
{
   TRAINING_EDGE_1,
   TRAINING_EDGE_2
}MV_HWS_Edge;

typedef enum
{
   TRAINING_EDGE_MAX,
   TRAINING_EDGE_MIN
}MV_HWS_EdgeSearch;


/******************************************************************************
* Name:     ddr3TipPbsRx.
* Desc:     PBS RX
* Args:     devNum - device number
* Notes:
* Returns:  OK if success, other error code if fail.
*/
/* TBD need arguments to funtion */
GT_STATUS    ddr3TipPbsRx
(
    GT_U32      devNum
);


/*****************************************************************************
Print PBS Result
******************************************************************************/
GT_STATUS    ddr3TipPrintAllPbsResult
(
    GT_U32      devNum
);

/******************************************************************************
* Name:     ddr3TipPbsTx.
* Desc:     PBS TX
* Args:     devNum - device number
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipPbsTx
(
    GT_U32      devNum
);

#endif /* __mvDdr3TrainingIP_PBS_H */


