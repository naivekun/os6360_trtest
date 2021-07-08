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
* mvDdr3TrainingBist.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 16 $
*
*******************************************************************************/




#ifndef _MV_DDR3_TRAINING_BIST_H_
#define _MV_DDR3_TRAINING_BIST_H_

#include "mvDdr3TrainingIp.h"

#ifdef __cplusplus
   extern "C"
   {
#endif

/******************************** enum ***************************************/

typedef enum
{
   BIST_STOP = 0,
   BIST_START = 1
} MV_HWS_BIST_OPERATION;

typedef enum
{
   STRESS_NONE = 0,
   STRESS_ENABLE = 1
} MV_HWS_STRESS_JUMP;

typedef enum
{
   DURATION_SINGLE = 0,
   DURATION_STOP_AT_FAIL = 1,
   DURATION_ADDRESS = 2,
   DURATION_CONT = 4

} MV_HWS_PATTERN_DURATION;

/************************* Structures ***********************************************/

typedef struct
{
   GT_U32                  bistErrorCnt;
   GT_U32                  bistFailLow;
   GT_U32                  bistFailHigh;
   GT_U32                  bistLastFailAddr;
}BistResult;



/******************************************************************************
* Name:     ddr3TipBistReadResult.
* Desc:     Read DDR BIST Result
* Args:     devNum          - device number
*           interfaceId     - interface id
*           pstBistResult   - for returning the values
* Notes:
* Returns:  OK if success, other error code if fail.
*/

GT_STATUS    ddr3TipBistReadResult
(
    GT_U32          devNum,
    GT_U32          interfaceId,
    BistResult      *pstBistResult
);

/******************************************************************************
* Name:     ddr3TipBistActivate.
* Desc:     Activate DDR BIST
* Args:
* Notes:
* Returns:  OK if success, other error code if fail.
*/

GT_STATUS    ddr3TipBistActivate
(
    GT_U32                  devNum,
    MV_HWS_PATTERN          pattern,
    MV_HWS_ACCESS_TYPE      accessType,
    GT_U32                  ifNum,
    MV_HWS_DIRECTION        direction,
    MV_HWS_STRESS_JUMP      addrStressJump,
    MV_HWS_PATTERN_DURATION duration,
    MV_HWS_BIST_OPERATION   operType,
    GT_U32                  offset,
    GT_U32                  csNum,
    GT_U32                  patternAddrLength
);

/******************************************************************************
* Name:     mvHwsDdr3RunBist.
* Desc:     Bist Flow
* Args:
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS    mvHwsDdr3RunBist
(
    GT_U32          devNum,
    MV_HWS_PATTERN  pattern,
    GT_U32          *result,
    GT_U32          csNum
);


/******************************************************************************
* Name:     ddr3TipRunSweepTest.
* Desc:     Bist sweep control
* Args:
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_BOOL ddr3TipRunSweepTest
(
    GT_32 devNum,
    GT_U32 RepeatNum,
    GT_U32 direction,
    GT_U32 mode
);

GT_BOOL ddr3TipRunLevelingSweepTest
(
    GT_32 devNum,
    GT_U32 RepeatNum,
    GT_U32 direction,
    GT_U32 mode
);
/******************************************************************************
* Name:     ddr3TipPrintRegs.
* Desc:     Print Specified Dunit/Pup Registers
* Args:
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS ddr3TipPrintRegs
(
    GT_U32 devNum
);

/******************************************************************************
* Name:     ddr3TipRegDump.
* Desc:     Print Dunit/Pup Registers Dump
* Args:
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS ddr3TipRegDump
(
    GT_U32 devNum
);


/******************************************************************************
* Name:     RunXsbTest.
* Desc:     Run XSB test
* Args:     devNum
*           uiMemAddress
*           startPattern
*           endPattern
*           writeMode
*           readMode
* Notes:
* Returns:  OK if success, other error code if fail.
*/
GT_STATUS
RunXsbTest(GT_U32 devNum, GT_U32 uiMemAddress, GT_U32 writeType, GT_U32 readType, GT_U32 burstLength);

#ifdef __cplusplus
   }
#endif


#endif /* _MV_DDR3_TRAINING_BIST_H_ */


