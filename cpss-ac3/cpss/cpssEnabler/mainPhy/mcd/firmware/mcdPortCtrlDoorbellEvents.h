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
* @file mcdPortCtrlDoorbellEvents.h
*
* @brief Cm3 to CPU Doorbell events support
*
* @version   1
********************************************************************************
*/

#ifndef __mcdPortCtrlDoorbellEvents_h__
#define __mcdPortCtrlDoorbellEvents_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MCD_AP_DOORBELL_EVENT_PORT_802_3_AP(_port)       (1 << (_port))
#define MCD_AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(_port)  (0x0001000 + (1 << (_port)))

/**
* @internal mcdCm3ToHostDoorbell function
* @endinternal
*
* @brief   Raise CM3 to HOST doorbell event
*
* @param[in] event                    - bits to set in Doorbell register
*                                       MCD_OK if successful, MCD_FAIL if not.
*
* @note Usage:
*       mcdCm3ToHostdDoorbell(MCD_AP_DOORBELL_EVENT_PORT_0_802_3_AP_E);
*       or  mcdCm3ToHostdDoorbell(MCD_AP_DOORBELL_EVENT_PORT_802_3_AP(portNumb));
*       or  mcdCm3ToHostdDoorbell(MCD_AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(portNumb));
*
*/
MCD_STATUS mcdCm3ToHostDoorbell
(
    IN  MCD_U32     event
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mcdPortCtrlDoorbellEvents_h_ */


