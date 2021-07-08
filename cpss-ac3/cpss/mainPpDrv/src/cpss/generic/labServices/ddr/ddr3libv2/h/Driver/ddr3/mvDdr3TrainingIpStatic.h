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
* mvDdr3TrainingIpStatic.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 11 $
*
*******************************************************************************/




#ifndef _MV_DDR3_TRAININGIP_STATIC_H_
#define _MV_DDR3_TRAININGIP_STATIC_H_

#include "mvDdr3TrainingIpDef.h"
#include "mvDdr3TrainingIp.h"

#ifdef __cplusplus
   extern "C"
   {
#endif


/******************************** defined ***************************************/

/******************************************************************************/
/* Definitions of INTERFACE registers                                             */
/******************************************************************************/


/******************************************************************************/
/* Definitions of PHY registers                                               */
/******************************************************************************/


/************************* Structures ***********************************************/

typedef struct
{
   GT_U32 dqsDelay; /* DQS delay (mSec) */
   GT_U32 ckDelay;  /* CK Delay  (mSec) */

} TripDelayElement;

typedef struct
{
    GT_U32              siliconDelay;
    TripDelayElement    *packageTraceArr;
    TripDelayElement    *boardTraceArr;

} MV_HWS_TIP_STATIC_CONFIG_INFO;


/************************* Declarations ***********************************************/

GT_STATUS    ddr3TipRunStaticAlg
(
    GT_U32  devNum,
    MV_HWS_DDR_FREQ freq
);

GT_STATUS ddr3TipInitStaticConfigDb
(
    GT_U32                          devNum,
    MV_HWS_TIP_STATIC_CONFIG_INFO*  staticConfigInfo
);

GT_STATUS ddr3TipInitSpecificRegConfig
(
    GT_U32              devNum,
    RegData             *regConfigArr
);

GT_STATUS    ddr3TipStaticPhyInitController
(
    GT_U32    devNum
);

#ifdef __cplusplus
}
#endif

#endif /* _MV_DDR3_TRAININGIP_STATIC_H_ */


