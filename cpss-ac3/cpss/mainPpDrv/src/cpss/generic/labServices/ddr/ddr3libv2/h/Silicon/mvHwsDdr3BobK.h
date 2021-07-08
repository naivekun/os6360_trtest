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
* mvHwsDdr3BobK.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/

#ifndef __mvHwsDdr3_BOBK_H
#define __mvHwsDdr3_BOBK_H

#include "mvDdr3TrainingIpDef.h"
#if !defined(CPSS_BUILD)
#include "config_marvell.h"     /* Required to identify SOC and Board */
#include "mvSysEnvLib.h"
#endif
#include "mvDdr3TrainingIpStatic.h"
#include "mvDdr3TrainingIp.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************* Definitions ***********************************************/

#define BOBK_NUMBER_OF_BOARDS           (2)
#define NUMBER_OF_PUP                   5
/************************* Enums ***********************************************/

typedef enum {
    TM_EN,
    MSYS_EN
}DDR_IF_ASSIGNMENT;

/************************* Globals ***********************************************/

#ifndef DEFINE_GLOBALS
extern TripDelayElement bobKBoardRoundTripDelayArray[MAX_INTERFACE_NUM*MAX_BUS_NUM*BOBK_NUMBER_OF_BOARDS];
#else
/* this array hold the board round trip delay (DQ and CK) per <interfcae,bus> */
TripDelayElement bobKBoardRoundTripDelayArray[MAX_INTERFACE_NUM*MAX_BUS_NUM*BOBK_NUMBER_OF_BOARDS] =
{
   /* 1st board */
   /*interface bus DQS-delay CK-delay */
   { 3012,   6715 },
   { 2625,   6715 },
   { 3023,   6458 },
   { 2663,   6458 },
   { 2596,   6691 },

   /* 2nd board */
   /*interface bus DQS-delay CK-delay */
   { 3012,   6715 },
   { 2625,   6715 },
   { 3023,   6458 },
   { 2663,   6458 },
   { 2596,   6691 },
};
#endif


#ifndef DEFINE_GLOBALS
extern TripDelayElement bobKPackageRoundTripDelayArray[MAX_INTERFACE_NUM*MAX_BUS_NUM];
#else
/* package trace */
TripDelayElement bobKPackageRoundTripDelayArray[MAX_INTERFACE_NUM*MAX_BUS_NUM] =
{
     /*IF BUS DQ_DELYA CK_DELAY */
    {  362,    813 },
    {  458,    813 },
    {  405,    750 },
    {  446,    750 },
    {  393,    718 }
};
#endif

#ifndef DEFINE_GLOBALS
extern GT_32 bobKSiliconDelayOffset[];
#else
GT_32 bobKSiliconDelayOffset[] =
{
    /* board 0 */
    0,
    /* board 1 */
    0,
    /* board 2 */
    0
};
#endif

/************************* Functions Declarations ***********************************************/

/******************************************************************************
* Name:     ddr3TipInitBobK.
* Desc:     init Training SW DB and updates DDR topology.
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS ddr3TipInitBobK
(
    GT_U32  devNum,
    GT_U32  boardId
) ;

/******************************************************************************
 * Name:     ddr3GetSdramAssignment
 * Desc:     read S@R and return DDR3 assignment ( 0 = TM , 1 = MSYS )
 * Args:
 * Notes:
 * Returns:  required value
 */
DDR_IF_ASSIGNMENT ddr3GetSdramAssignment(GT_U8 devNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsDdr3_BOBK_H */

