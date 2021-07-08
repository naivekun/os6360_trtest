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
* @file cpssGenEventRequests.h
*
* @brief Includes unified event routine. The routines allow a user application
* to bind, select and receive events from PP, XBAR and FA devices.
*
* @version   7
********************************************************************************
*/
#ifndef __cpssGenEventRequestsh
#define __cpssGenEventRequestsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>

/**
* @enum CPSS_EVENT_MASK_SET_ENT
 *
 * @brief Defines the different interrupt node states, that an interrupt
 * node may have.
*/
typedef enum{

    /** @brief mask unified event , so application will not get
     *  an event
     */
    CPSS_EVENT_MASK_E          = 0,

    /** @brief UNmask unified event , so application can receive
     *  an event
     */
    CPSS_EVENT_UNMASK_E        = 1

} CPSS_EVENT_MASK_SET_ENT;

/**
* @internal cpssEventBind function
* @endinternal
*
* @brief   This routine binds a user process to unified event. The routine returns
*         a handle that is used when the application wants to wait for the event
*         (cpssEventSelect), receive the event(cpssEventRecv) or transmit a packet
*         using the Network Interface.
*         NOTE : the function does not mask/unmask the HW events in any device.
*         This is Application responsibility to unmask the relevant events
*         on the needed devices , using function cpssEventDeviceMaskSet or
*         cpssEventDeviceMaskWithEvExtDataSet
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] uniEventArr[]            - The unified event list.
* @param[in] arrLength                - The unified event list length.
*
* @param[out] hndlPtr                  - (pointer to) The user handle for the bounded events.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_FULL                  - when trying to set the "tx buffer queue unify event"
*                                       (CPSS_PP_TX_BUFFER_QUEUE_E) with other events in
*                                       the same handler
* @retval GT_ALREADY_EXIST         - one of the unified events already bound to another
*                                       handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)
*/
GT_STATUS cpssEventBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT uniEventArr[],
    IN  GT_U32               arrLength,
    OUT GT_UINTPTR           *hndlPtr
);

/**
* @internal cpssEventUnBind function
* @endinternal
*
* @brief   This routine unbinds a user process from unified events.
*         The routine does not delete the associated handlers.
*         NOTE : the function does not mask/unmask the HW events in any device.
*         This is Application responsibility to unmask the relevant events
*         on the needed devices , using function cpssEventDeviceMaskSet or
*         cpssEventDeviceMaskWithEvExtDataSet
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] uniEventArr[]            - The unified event list.
* @param[in] arrLength                - The unified event list length.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssEventUnBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT  uniEventArr[],
    IN  GT_U32                 arrLength
);

/**
* @internal cpssEventDestroy function
* @endinternal
*
* @brief   This routine destroy the handle , and this handle is not valid any more.
*         The handle could have been create either by cpssEventIsrBind or cpssEventBind
*         API implementation sequence:
*         1. Signal any locked semaphores (so application's task can continue)
*         2. Release dynamic allocations relate to this event.
*         NOTE : the function does not mask/unmask the HW events in any device.
*         This is Application responsibility to unmask the relevant events
*         on the needed devices , using function cpssEventDeviceMaskSet or
*         cpssEventDeviceMaskWithEvExtDataSet
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] hndl                     - The user handle for the bounded events.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_BAD_PARAM             - bad hndl parameter , the hndl parameter is not legal
*                                       (was not returned by cpssEventBind(...)/cpssEventIsrBind(...))
*/
GT_STATUS cpssEventDestroy
(
    IN  GT_UINTPTR          hndl
);

/**
* @internal cpssEventSelect function
* @endinternal
*
* @brief   This function waiting for one of the events ,relate to the handler,
*         to happen , and gets a list of events (in array of bitmaps format) that
*         occurred .
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] hndl                     - The user handle for the bounded events.
* @param[in] timeoutPtr               - (pointer to) Wait timeout in milliseconds
*                                      NULL pointer means wait forever.
* @param[in] evBitmapArrLength        - The bitmap array length (in words).
*
* @param[out] evBitmapArr[]            - The bitmap array of the received events.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_TIMEOUT               - when the "time out" requested by the caller expired and no
*                                       event occurred during this period
* @retval GT_BAD_PARAM             - bad hndl parameter , the hndl parameter is not legal
*                                       (was not returned by cpssEventBind(...))
* @retval GT_BAD_PTR               - evBitmapArr parameter is NULL pointer
*                                       (and evBitmapArrLength != 0)
* @retval GT_EMPTY                 - there are no events to retrieve (relevant when timeoutPtr != NULL).
*/
GT_STATUS cpssEventSelect
(
    IN  GT_UINTPTR          hndl,
    IN  GT_U32              *timeoutPtr,
    OUT GT_U32              evBitmapArr[],
    IN  GT_U32              evBitmapArrLength
);

/**
* @internal cpssEventRecv function
* @endinternal
*
* @brief   This function gets general information about the selected unified event.
*         The function retrieve information about the device number that relate to
*         the event , and extra info about port number / priority queue that
*         relate to the event.
*         The function retrieve the info about the first occurrence of this event
*         in the queue.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] hndl                     - The user handle for the bounded events.
* @param[in] evCause                  - The specified unify event that info about it's first
*                                      waiting occurrence required.
*
* @param[out] evExtDataPtr             (pointer to)The additional data (port num / priority
*                                      queue number) the event was received upon.
* @param[out] evDevPtr                 - (pointer to)The device the event was received upon
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_NO_MORE               - There is no more info to retrieve about the specified
*                                       event .
* @retval GT_BAD_PARAM             - bad hndl parameter ,
*                                    or hndl bound to CPSS_PP_TX_BUFFER_QUEUE_E --> not allowed
*                                    use dedicated "get tx ended info" function instead
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssEventRecv
(
    IN  GT_UINTPTR          hndl,
    IN  CPSS_UNI_EV_CAUSE_ENT evCause,
    OUT GT_U32              *evExtDataPtr,
    OUT GT_U8               *evDevPtr
);


/**
* @internal cpssEventDeviceMaskSet function
* @endinternal
*
* @brief   This routine mask/unmasks an unified event on specific device.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] devNum                   - device number - PP/FA/Xbar device number -
*                                      depend on the uniEvent
*                                      if the uniEvent is in range of PP events , then devNum relate
*                                      to PP
*                                      if the uniEvent is in range of FA events , then devNum relate
*                                      to FA
*                                      if the uniEvent is in range of XBAR events , then devNum relate
*                                      to XBAR
* @param[in] uniEvent                 - The unified event.
* @param[in] operation                - the operation : mask / unmask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event has no interrupts associated with it in
*                                       the device
* @retval GT_NOT_INITIALIZED       - the CPSS was not initialized properly to handle
*                                       this type of event
* @retval GT_HW_ERROR              - on hardware error
*
* @note The OS interrupts and the ExtDrv interrupts are locked during the
*       operation of the function.
*
*/
GT_STATUS cpssEventDeviceMaskSet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN CPSS_EVENT_MASK_SET_ENT  operation
);

/**
* @internal cpssEventDeviceGenerate function
* @endinternal
*
* @brief   This debug routine configures device to generate unified event for
*         specific element in the event associated with extra data.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
* @param[in] devNum                   - device number - PP/FA/Xbar device number -
*                                      depend on the uniEvent
*                                      if the uniEvent is in range of PP events , then devNum relate
*                                      to PP
*                                      if the uniEvent is in range of FA events , then devNum relate
*                                      to FA
*                                      if the uniEvent is in range of XBAR events , then devNum relate
*                                      to XBAR
* @param[in] uniEvent                 - The unified event.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*                                      Use CPSS_PARAM_NOT_USED_CNS to generate events for all supported evExtData.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event has no interrupts associated with it in
*                                       the device Or the 'extra data' has value that not relate
*                                       to the uniEvent
* @retval GT_NOT_INITIALIZED       - the CPSS was not initialized properly to handle
*                                       this type of event
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported unify event
*/
GT_STATUS cpssEventDeviceGenerate
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData
);

/**
* @internal cpssEventDeviceMaskWithEvExtDataSet function
* @endinternal
*
* @brief   This routine mask/unmasks an unified event on specific device , for
*         specific element in the event associated with extra data.
*         Since Each unified event may be associated with multiple HW interrupts,
*         each HW interrupt has different 'extra data' in the context of the
*         relevant unified event
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] devNum                   - device number - PP/FA/Xbar device number -
*                                      depend on the uniEvent
*                                      if the uniEvent is in range of PP events , then devNum relate
*                                      to PP
*                                      if the uniEvent is in range of FA events , then devNum relate
*                                      to FA
*                                      if the uniEvent is in range of XBAR events , then devNum relate
*                                      to XBAR
* @param[in] uniEvent                 - The unified event.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
* @param[in] operation                - the operation : mask / unmask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event has no interrupts associated with it in
*                                       the device Or the 'extra data' has value that not relate
*                                       to the uniEvent
* @retval GT_NOT_INITIALIZED       - the CPSS was not initialized properly to handle
*                                       this type of event
* @retval GT_HW_ERROR              - on hardware error
*
* @note The OS interrupts and the ExtDrv interrupts are locked during the
*       operation of the function.
*
*/
GT_STATUS cpssEventDeviceMaskWithEvExtDataSet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData,
    IN CPSS_EVENT_MASK_SET_ENT  operation
);

/**
* @internal cpssEventDeviceMaskWithEvExtDataGet function
* @endinternal
*
* @brief   This routine reads mask status of given unified event, for specific port
*         or other constrain defined by extended data.
*         This routine gets data if an unified event on specific device masked or unmasked
*         for specific element in the event associated with extra data.
*         Since Each unified event may be associated with multiple HW interrupts,
*         each HW interrupt has different 'extra data' in the context of the
*         relevant unified event
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] devNum                   - device number - PP/FA/Xbar device number -
*                                      depend on the uniEvent
*                                      if the uniEvent is in range of PP events , then devNum relate
*                                      to PP
*                                      if the uniEvent is in range of FA events , then devNum relate
*                                      to FA
*                                      if the uniEvent is in range of XBAR events , then devNum relate
*                                      to XBAR
* @param[in] uniEvent                 - The unified event.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*
* @param[out] maskedPtr                - is interrupt masked or unmasked
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad unify event value or bad device number
* @retval GT_NOT_FOUND             - the unified event has no interrupts associated with it in
*                                       the device Or the 'extra data' has value that not relate
*                                       to the uniEvent
* @retval GT_NOT_INITIALIZED       - the CPSS was not initialized properly to handle
*                                       this type of event
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on bad pointer
*
* @note The OS interrupts and the ExtDrv interrupts are locked during the
*       operation of the function.
*
*/
GT_STATUS cpssEventDeviceMaskWithEvExtDataGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_U32                   evExtData,
    OUT CPSS_EVENT_MASK_SET_ENT *maskedPtr
);

/*******************************************************************************
* CPSS_EVENT_ISR_CB_FUNC
*
* DESCRIPTION:
*       This is a prototype of the functions that need to be bound to handler
*       that aggregate one or more events . this CB was bound by calling
*       function cpssEventIsrBind(...)
*
*       This application's function would be called at Interrupt context !!!
*
*       This allowing the application to use any synchronism mechanism which
*       suits its needs.
*
*
* APPLICABLE DEVICES:  All devices (PP / FA / XBAR)
*
* INPUTS:
*       hndl        - The user handle for the bounded events.
*                     as returned as output by function cpssEventIsrBind(...)
*       cookie      - The user's cookie(application's data) for the bounded events.
*                     as given as input to function cpssEventIsrBind(...)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_VOID (*CPSS_EVENT_ISR_CB_FUNC)
(
    IN GT_UINTPTR   hndl,
    IN GT_VOID*     cookie
);


/**
* @internal cpssEventIsrBind function
* @endinternal
*
* @brief   This routine binds a user process to unified event. The routine returns
*         a handle that is used when the application wants to check if there are
*         waiting events (cpssEventWaitingEventsGet), clear the already served
*         events (cpssEventTreatedEventsClear) , receive the event(cpssEventRecv)
*         This is alternative way of treating events as opposed of the other
*         Select mechanism used with function cpssEventBind(...) (where
*         Application had to wait on binary semaphore, which was released by the
*         relevant interrupt).
*         In the new scheme, the application provides an ISR function which would
*         be called whenever the relevant interrupt(s) have been occurred.
*         Thus, allowing the application to use any synchronism mechanism which
*         suits its needs.
*         The application ISR function would be called at Interrupt context !!!
*         NOTE : 1. the function does not mask/unmask the HW events in any device.
*         This is Application responsibility to unmask the relevant events
*         on the needed devices , using function cpssEventDeviceMaskSet or
*         cpssEventDeviceMaskWithEvExtDataSet
*         2. for this handler the function cpssEventSelect(..) is not
*         applicable.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] uniEventArr[]            - The unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] isrCbPtr                 - (pointer for) application ISR function to be called when
*                                      events are received
* @param[in] cookie                   - storing application data (cookie)
*
* @param[out] hndlPtr                  - (pointer to) The user handle for the bounded events.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_FULL                  - when trying to set the "tx buffer queue unify event"
*                                       (CPSS_PP_TX_BUFFER_QUEUE_E) with other events in the same
*                                       handler
* @retval GT_ALREADY_EXIST         - one of the unified events already bound to another
*                                       handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)
*/
GT_STATUS cpssEventIsrBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT   uniEventArr[],
    IN  GT_U32                  arrLength,
    IN  CPSS_EVENT_ISR_CB_FUNC  isrCbPtr,
    IN  GT_VOID*                cookie,
    OUT GT_UINTPTR              *hndlPtr
);

/**
* @internal cpssEventWaitingEventsGet function
* @endinternal
*
* @brief   This function returns a bitmap reflecting the unified events waiting
*         in queue for the user application.
*         the function get the events from the waiting queue.
*         This function should be called after a call to clear already served
*         events (cpssEventTreatedEventsClear).
*         The application receives a bitmap of events that are waiting.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] hndl                     - The user handle.
* @param[in] evBitmapLength           - The size of bitmap array in words.
*
* @param[out] evBitmapArr[]            - The updated bitmap array.(can be NULL if
*                                        evBitmapLength == 0 )
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - the hndl parameter is not legal (was not returned by
*                                       cpssEventIsrBind(...))
* @retval GT_NO_MORE               - no events are waiting
*/
GT_STATUS cpssEventWaitingEventsGet
(
    IN GT_UINTPTR       hndl,
    OUT GT_U32          evBitmapArr[],
    IN GT_U32           evBitmapLength
);


/**
* @internal cpssEventTreatedEventsClear function
* @endinternal
*
* @brief   remove the events from the waiting queue , that already served
*         (Received) and has no new waiting for All Handler event.
*         The events are unified under a Handler received from the CPSS.
*         NOTE : the function protect itself by internal "interrupts lock" in
*         the start of function , and "interrupts unlock" in the end of
*         function.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] hndl                     - The user handle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - the hndl parameter is not legal
*/
GT_STATUS cpssEventTreatedEventsClear
(
    IN GT_UINTPTR       hndl
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenEventRequestsh */
