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
* mvDdr3TrainingIpCentralization.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/

#ifndef __mvDdr3TrainingIP_CENTRALIZATION_H
#define __mvDdr3TrainingIP_CENTRALIZATION_H


typedef struct
{
    GT_U32 regAddr;
    GT_U32 startBit; /* lsb 4 bits */
    GT_U32 endBit;   /* lsb 5th bit */
}centralizationRes;

/******************************************************************************
* Name:     ddr3TipDDR3CentralizationTx.
* Desc:     Centralization TX
* Args:     devNum
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipCentralizationTx
(
    GT_U32    devNum
);

/******************************************************************************
* Name:     ddr3TipDDR3CentralizationRx.
* Desc:     Centralization RX
* Args:     devNum
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipCentralizationRx
(
    GT_U32    devNum
);

/******************************************************************************
* Name:     ddr3TipPrintCentralizationResult.
* Desc:     Print Centralization Results
* Args:     devNum
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipPrintCentralizationResult
(
    GT_U32      devNum
);

/******************************************************************************
* Name:     ddr3TipSpecialRx.
* Desc:     Special RX pattern
* Args:     devNum
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipSpecialRx
(
    GT_U32   devNum
);

#endif /* __mvDdr3TrainingIP_CENTRALIZATION_H */


