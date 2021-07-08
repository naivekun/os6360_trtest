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
* @file mvHwsPortCtrlInit.h
*
* @brief Port Control Initialization
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlInit_H
#define __mvHwsPortCtrlInit_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef GT_STATUS (* MV_PORT_CTRL_AP_CALLBACK)
(
    IN GT_U32               devNum,
    IN GT_U16               portNum,
    IN MV_HWS_PORT_STANDARD portMode
);

typedef struct {
    MV_PORT_CTRL_AP_CALLBACK        apHcdFoundClbk;      /* called when resolution is found */
    MV_PORT_CTRL_AP_CALLBACK        apLinkUpDownClbk;    /* called to handle AP port link Up/Down */
    /**
     * Add new callbacks here
     */

}MV_PORT_CTRL_AP_CALLBACKS;

/* Initialization API */

/**
 *
 * mvPortCtrlApCallbacksInit
 *
 * @param apCallbacksPtr
 *
 * @return GT_STATUS
 */
GT_STATUS mvPortCtrlApRegisterCallbacks(MV_PORT_CTRL_AP_CALLBACKS *apCallbacksPtr);

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

#endif /* __mvHwsPortCtrlInit_H */






