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
* @file mcdFwPortCtrlInit.h
*
* @brief Port Control Initialization
*
* @version   1
********************************************************************************
*/

#ifndef __mcdFwPortCtrlInit_H
#define __mcdFwPortCtrlInit_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Initialization API */

/**
* @internal mvPortCtrlHwInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlHwInit(void);

/**
* @internal mvPortCtrlSwInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlSwInit(void);

/**
* @internal mvPortCtrlOsResourcesInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlOsResourcesInit(void);

/**
* @internal mvPortCtrlSyncLock function
* @endinternal
*
*/
GT_STATUS mvPortCtrlSyncLock(void);
GT_STATUS mvPortCtrlSyncUnlock(void);

/**
* @internal mvPortCtrlProcessDelay function
* @endinternal
*
*/
GT_STATUS mvPortCtrlProcessDelay(GT_U32 duration);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mcdFwPortCtrlInit_H */







