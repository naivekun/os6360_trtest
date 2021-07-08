/*******************************************************************************
*                Copyright 2014, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file rxEventHandler.h
*
* @brief This module defines the API for CPSS_PP_RX_BUFFER_QUEUE_E and
* CPSS_PP_RX_ERR_QUEUE_E event handling.
*
* @version   1
********************************************************************************
*/
#ifndef __rxEventHandler_h__
#define __rxEventHandler_h__

#include <cpss/common/cpssTypes.h>
#ifdef CHX_FAMILY
#include <cpss/generic/cpssCommonDefs.h>
#endif

#define RX_EV_HANDLER_DEVNUM_ALL 0xff
#define RX_EV_HANDLER_QUEUE_ALL  0xff

/**
* @enum RX_EV_HANDLER_EV_TYPE_ENT
 *
 * @brief Event type
*/
typedef enum{

    /** CPSS_PP_RX_BUFFER_QUEUE_E events */
    RX_EV_HANDLER_TYPE_RX_E,

    /** CPSS_PP_RX_ERR_QUEUE_E events */
    RX_EV_HANDLER_TYPE_RX_ERR_E,

    /** any type of events */
    RX_EV_HANDLER_TYPE_ANY_E

} RX_EV_HANDLER_EV_TYPE_ENT;

/*******************************************************************************
* RX_EV_PACKET_RECEIVE_CB_FUN
*
* DESCRIPTION:
*       Function called to handle incoming Rx packet in the CPU
*
* APPLICABLE DEVICES: ALL Devices
*
* INPUTS:
*       cookie       - a cookie passed to rxEventHandlerAddCallback()
*       evType       - event type
*       devNum       - Device number.
*       queueIdx     - The queue from which this packet was received.
*       numOfBuffPtr - Num of used buffs in packetBuffs.
*       packetBuffs  - The received packet buffers list.
*       buffLen      - List of buffer lengths for packetBuffs.
*       rxParamsPtr  - (pointer to)information parameters of received packets
*
* RETURNS:
*       GT_OK - no error
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*RX_EV_PACKET_RECEIVE_CB_FUN)
(
  IN  GT_UINTPTR                cookie,
  IN  RX_EV_HANDLER_EV_TYPE_ENT evType,
  IN  GT_U8                     devNum,
  IN  GT_U8                     queueIdx,
  IN  GT_U32                    numOfBuff,
  IN  GT_U8                     *packetBuffs[],
  IN  GT_U32                    buffLen[],
  IN  GT_VOID                   *rxParamsPtr
);

/**
* @internal rxEventHandlerLibInit function
* @endinternal
*
* @brief   Initialize library
*/
GT_STATUS rxEventHandlerLibInit(GT_VOID);

/**
* @internal rxEventHandlerInitHandler function
* @endinternal
*
* @brief   Initialize event handler for Rx/RxErr events
*         rxEventHandlerDoCallbacks() must be called from the same process
*         (another thread of this process allowed)
* @param[in] devNum                   - the device number or RX_EV_HANDLER_DEVNUM_ALL
* @param[in] queue                    - the rx  number or RX_EV_HANDLER_QUEUE_ALL
*                                      type    - event type
*
* @param[out] evHandlePtr              - handle
*                                       GT_OK on success
*                                       GT_ALREADY_EXIST success, already initialized
*                                       GT_BAD_PARAM
*                                       GT_FAIL
*/
GT_STATUS rxEventHandlerInitHandler(
  IN  GT_U8                     devNum,
  IN  GT_U8                     queue,
  IN  RX_EV_HANDLER_EV_TYPE_ENT evType,
  OUT GT_UINTPTR                *evHandlePtr
);

/**
* @internal rxEventHandlerAddCallback function
* @endinternal
*
* @brief   Add callback function for Rx events
*
* @param[in] devNum                   - the device number or RX_EV_HANDLER_DEVNUM_ALL
* @param[in] queue                    - the rx  number or RX_EV_HANDLER_QUEUE_ALL
*                                      type    - event type
* @param[in] cbFunc                   - callback function
* @param[in] cookie                   -  value to be passed to callback function
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS rxEventHandlerAddCallback(
  IN  GT_U8                         devNum,
  IN  GT_U8                         queue,
  IN  RX_EV_HANDLER_EV_TYPE_ENT     evType,
  IN  RX_EV_PACKET_RECEIVE_CB_FUN   cbFunc,
  IN  GT_UINTPTR                    cookie
);

/**
* @internal rxEventHandlerRemoveCallback function
* @endinternal
*
* @brief   Add callback function for Rx events
*
* @param[in] devNum                   - the device number or RX_EV_HANDLER_DEVNUM_ALL
* @param[in] queue                    - the rx  number or RX_EV_HANDLER_QUEUE_ALL
* @param[in] evType                   - event type
* @param[in] cbFunc                   - callback function
* @param[in] cookie                   -  passed to rxEventHandlerAddCallback()
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS rxEventHandlerRemoveCallback(
  IN  GT_U8                         devNum,
  IN  GT_U8                         queue,
  IN  RX_EV_HANDLER_EV_TYPE_ENT     evType,
  IN  RX_EV_PACKET_RECEIVE_CB_FUN   cbFunc,
  IN  GT_UINTPTR                    cookie
);


/**
* @internal rxEventHandlerDoCallbacks function
* @endinternal
*
* @brief   Execute all callback routines for event handler
*
* @param[in] handle                   - the of CPSS event handler
* @param[in] evType                   - event type
* @param[in] devNum                   - Device number.
* @param[in] queue                    - The queue from which this packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs.
* @param[in] packetBuffs[]            - The received packet buffers list.
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS rxEventHandlerDoCallbacks
(
    IN GT_UINTPTR                   handle,
    IN RX_EV_HANDLER_EV_TYPE_ENT    evType,
    IN GT_U8                        devNum,
    IN GT_U8                        queue,
    IN GT_U32                       numOfBuff,
    IN GT_U8                        *packetBuffs[],
    IN GT_U32                       buffLen[],
    IN GT_VOID                      *rxParamsPtr
);

#endif /* __rxEventHandler_h__ */

