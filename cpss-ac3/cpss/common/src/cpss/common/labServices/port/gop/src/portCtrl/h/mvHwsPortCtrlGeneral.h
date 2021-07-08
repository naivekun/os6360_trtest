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
* @file mvHwsPortCtrlGeneral.h
*
* @brief Port Control General Definitions
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlGeneral_H
#define __mvHwsPortCtrlGeneral_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#ifdef AP_GENERAL_TASK
/**
* @internal mvPortCtrlGenRoutine function
* @endinternal
*
*/
void mvPortCtrlGenRoutine(void* pvParameters);

/**
* @internal mvPortCtrlGenDelay function
* @endinternal
*
*/
void mvPortCtrlGenDelay(void);

/**
* @internal mvPortCtrlGenMsgProcess function
* @endinternal
*
*/
void mvPortCtrlGenMsgProcess(void);
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlGeneral_H */





