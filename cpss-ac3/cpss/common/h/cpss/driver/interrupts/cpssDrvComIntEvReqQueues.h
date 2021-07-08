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
* @file cpssDrvComIntEvReqQueues.h
*
* @brief This file includes functions for managing and configuring the event
* queues structure for Event Driven mode.
*
* The following sketch shows the data structure involved in the setting
* of two events for one user event handle. The first event has one event
* node waiting in the FIFO.
* The userEvHandle structure is allocated by the coreEventBind routine.
* Each event bound to the user handler is linked to the
* userEvHandle.evListPtr field. Every bounded event also has a pointer
* (userHndlPtr) to the user handler (user in the ISR scan routine). Each
* event entry has a FIFO for the waiting event nodes (evNodeList), the
* evNodeList points to the FIFO head.
*
* uniEvQArr[uniEvent]                   intNodeList[dev]
* |============|<-----------|          |------>|===============|
* | evNodeList |------------+--------------------|    | devNum    |
* | uniEvCause |      |              | intStatus   |
* | userHndlPtr|---------| |   userEvHandle      | intRecStatus |
* | nextPtr  |-----|  |--+--->|============|     | intType    |
* |============|   |  | |  | evType   |     | intCause   |
* | evNodeList |-- |  | |  | semId   |     | uniEvCause  |
* | uniEvCause |   |  | |  | extDataPtr |--    | uniEvExt   |
* | userHndlPtr|   |  | |----| evListPtr |     | intMaskSetFptr|
* | nextPtr  |-- |  |    |============|     | intCbFuncPtr |
* |============|<----|  |                | intMaskReg  |
* | evNodeList |---------+-----------------------|    | intBitMask  |
* | uniEvCause |     |            |    | prevPtr    |--
* | userHndlPtr|---------|            |    | nextPtr    |--
* | nextPtr  |                 |    |===============|
* |============|                 |    |  ...    |
* | evNodeList |--               |    | prevPtr    |--
* | uniEvCause |                 |    | nextPtr    |--
* | userHndlPtr|--               |------>|===============|
* | nextPtr  |--                   |  ...    |
* |============|                     | prevPtr    |--
* |  ...   |                     | nextPtr    |--
* |============|                     |===============|
* |  ...    |
* | prevPtr    |--
* | nextPtr    |--
* |===============|
*
* @version   10
********************************************************************************
*/
#ifndef __cpssDrvComIntEvReqQueuesh
#define __cpssDrvComIntEvReqQueuesh

#include <cpss/generic/events/cpssGenEventRequests.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Forward struct declaration.  */
struct PRV_CPSS_DRV_EV_REQ_NODE_STCT;
struct PRV_CPSS_DRV_EV_REQ_Q_CTRL_STCT;


/* macro get the handle (GT_UINTPTR) of the handler

    handlerPtr is of type :
    PRV_CPSS_DRV_EVENT_HNDL_STC    *userHndlPtr
*/
#define PRV_CPSS_DRV_HANDEL_MAC(handlerPtr) \
    (GT_UINTPTR)(handlerPtr)

/* macro get the semaphore of the handler

    handlerPtr is of type :
    PRV_CPSS_DRV_EVENT_HNDL_STC    *userHndlPtr
*/
#define PRV_CPSS_DRV_HANDEL_SEM_MAC(handlerPtr) \
    (handlerPtr)->hndlBindInfo.semInfo.semId

/* macro get the semaphore signal flag of the handler

    handlerPtr is of type :
    PRV_CPSS_DRV_EVENT_HNDL_STC    *userHndlPtr
*/
#define PRV_CPSS_DRV_HANDEL_SEM_SIGNAL_MAC(handlerPtr) \
    (handlerPtr)->hndlBindInfo.semInfo.semSignal


/* macro get the application's CB function of the handler

    handlerPtr is of type :
    PRV_CPSS_DRV_EVENT_HNDL_STC    *userHndlPtr
*/
#define PRV_CPSS_DRV_HANDEL_APPL_CB_MAC(handlerPtr) \
    (handlerPtr)->hndlBindInfo.applCbInfo.isrCbPtr

/* macro get the application's cookie needed for  application CB function of the
   handler

    handlerPtr is of type :
    PRV_CPSS_DRV_EVENT_HNDL_STC    *userHndlPtr
*/
#define PRV_CPSS_DRV_HANDEL_APPL_COOKIE_MAC(handlerPtr) \
    (handlerPtr)->hndlBindInfo.applCbInfo.cookie


/*
 * Typedef: PRV_CPSS_DRV_REQ_DRVN_MASK_SET_FUNC
 *
 * Description: Interrupt mask/unmasking function pointer.
 *
 * Fields:
 *
 */
typedef GT_STATUS (*PRV_CPSS_DRV_REQ_DRVN_MASK_SET_FUNC)
(
    IN struct PRV_CPSS_DRV_EV_REQ_NODE_STCT    *evNode,
    IN CPSS_EVENT_MASK_SET_ENT   operation
);

/**
* @enum PRV_CPSS_DRV_PREV_DRVN_INT_STATUS_ENT
 *
 * @brief Defines the different interrupt node states, that an interrupt
 * node may have.
*/
typedef enum{

    /** @brief The interrupt is active and can
     *  be handled, was not set to pending state.
     */
    PRV_CPSS_DRV_EV_DRVN_INT_ACTIVE_E        = 0,

    /** @brief Used to stop interrupt handling
     *  on Hot removal.
     */
    PRV_CPSS_DRV_EV_DRVN_INT_STOPPED_E       = 1

} PRV_CPSS_DRV_PREV_DRVN_INT_STATUS_ENT;


/**
* @enum PRV_CPSS_DRV_EV_DRVN_INT_RECEPTION_STATUS_ENT
 *
 * @brief This enum defines the different types of interrupt receptions.
*/
typedef enum{

    /** @brief The interrupt was received during
     *  regular operation of the system (and was not
     *  handled yet).
     */
    PRV_CPSS_DRV_EV_DRVN_INT_RCVD_E          = 0,

    /** @brief The interrupt was received while
     *  it was masked. (and was not handled yet).
     *  PRV_CPSS_DRV_EV_DRVN_INT_IDLE_READY_E- The event is ready to be removed
     *  from queue.
     */
    PRV_CPSS_DRV_EV_DRVN_INT_MASKED_E        = 1,

    PRV_CPSS_DRV_EV_DRVN_INT_IDLE_READY_E    = 2,

    /** @brief The interrupt was not received
     *  lately. (was already handled or was not
     *  received).
     */
    PRV_CPSS_DRV_EV_DRVN_INT_IDLE_E          = 3

} PRV_CPSS_DRV_EV_DRVN_INT_RECEPTION_STATUS_ENT;


/*
 * typedef: PRV_CPSS_DRV_EV_DRVN_INT_REC_CB
 *
 * Description: Callback function called when an interrupt is received.
 *
 * Fields:
 *      devNum      - Device number.
 *      intIndex    - The interrupt index.
 *
 * Comment:
 */
typedef GT_VOID (*PRV_CPSS_DRV_EV_DRVN_INT_REC_CB)
(
    IN GT_U8  devNum,
    IN GT_U32 intIndex
);


/*
 * Typedef: struct PRV_CPSS_DRV_EV_REQ_NODE_STC
 *
 * Description: Includes information of received unhandled interrupts, to be
 *              managed through the event queues structure.
 *
 * Fields:
 *      devNum      - The device number.
 *      portGroupId      - The port group Id. support multi port groups device.
 *                    for non multi-port-groups device use : 0
 *                    the portGroupId to be used in the intMaskSetFptr implementation
 *      intStatus   - The interrupt status.
 *      intRecStatus- Indicates the time by which the interrupt was received.
 *      intType     - The interrupt type - this field not used in cpss code,
 *                    added here only for compatibility while working under PSS (POC)
 *      intCause    - The interrupt cause index
 *      uniEvCause  - The Unified event cause index
 *      uniEvExt    - The Unified event addition information (port number...)
 *      intMaskSetFptr
 *                  - The interrupt mask setting routine
 *      intCbFuncPtr - A pointer to a callback function ,to be called after an
 *                    interrupt was received before signalling the waiting app.
 *      intMaskReg  - Register address of the interrupt_mask register
 *                    representing this interrupt.
 *      intBitMask  - Bit mask in intMaskReg register, which represents the bit
 *                    relevant to this interrupt.
 *      prevPtr     - A pointer to the previous PRV_CPSS_DRV_EV_REQ_NODE_STC
 *                    in the linked-list.
 *      nextPtr     - A pointer to the next PRV_CPSS_DRV_EV_REQ_NODE_STC in the
 *                    linked-list.
 *      counter     - Counting the number of interrupts for this specific int cause;
 *                    this field not used in cpss code, added here only for
 *                    compatibility while working under PSS (POC)
 */
typedef struct PRV_CPSS_DRV_EV_REQ_NODE_STCT
{
    GT_U8                           devNum;
    GT_U32                          portGroupId;
    GT_U8                           intStatus;
    GT_U8                           intRecStatus;
#ifdef CPSS_PRODUCT
    GT_U8                           intType;
#endif
    GT_U32                          intCause;
    GT_U32                          uniEvCause;
    GT_U32                          uniEvExt;
    PRV_CPSS_DRV_REQ_DRVN_MASK_SET_FUNC  intMaskSetFptr;
    PRV_CPSS_DRV_EV_DRVN_INT_REC_CB intCbFuncPtr;
    GT_U32                          intMaskReg;
    GT_U32                          intBitMask;
    struct PRV_CPSS_DRV_EV_REQ_NODE_STCT *prevPtr;
    struct PRV_CPSS_DRV_EV_REQ_NODE_STCT *nextPtr;
#if ((defined CPSS_PRODUCT) && (defined GT_DEBUG_INTERRUPT_CNTR))
    GT_U32                          counter;
#endif
}PRV_CPSS_DRV_EV_REQ_NODE_STC;


/**
* @enum PRV_CPSS_DRV_EVENT_TYPE_ENT
 *
 * @brief Enumeration that defines the user handle type.
*/
typedef enum{

    /** "Tx buffer queue" event handler. */
    PRV_CPSS_DRV_TX_BUFF_QUEUE_EVENT_E      = 0,

    /** Regular event handler. */
    PRV_CPSS_DRV_REGULAR_EVENT_E     = 1

} PRV_CPSS_DRV_EVENT_TYPE_ENT;

/**
* @enum PRV_CPSS_DRV_HANDLE_BIND_TYPE_ENT
 *
 * @brief Enumeration that defines the type of bind that was done for the
 * handler.
*/
typedef enum{

    /** @brief the bind function that
     *  created this handler was cpssEventBind(...).
     *  this mechanism used semaphores and signaling.
     */
    PRV_CPSS_DRV_HANDLE_BIND_USE_SIGNAL_TYPE_E      = 0,

    /** @brief the bind function that
     *  created this handler was cpssEventIsrBind(...).
     *  this mechanism used application's callback (CB) functions .
     */
    PRV_CPSS_DRV_HANDLE_BIND_USE_APPL_CB_TYPE_E     = 1

} PRV_CPSS_DRV_HANDLE_BIND_TYPE_ENT;

/**
* @struct PRV_CPSS_DRV_EVENT_HNDL_STC
 *
 * @brief Includes information for user bounded events.
*/
typedef struct
{
    PRV_CPSS_DRV_EVENT_TYPE_ENT       evType;
    PRV_CPSS_DRV_HANDLE_BIND_TYPE_ENT hndlBindType;
    union{
        struct{
            GT_UINTPTR          semId;
            GT_BOOL             semSignal;
        }semInfo;

        struct{
            CPSS_EVENT_ISR_CB_FUNC isrCbPtr;
            void*                    cookie;
        }applCbInfo;
    }hndlBindInfo;
    GT_VOID             *extDataPtr;
    struct PRV_CPSS_DRV_EV_REQ_Q_CTRL_STCT  *evListPtr;

    struct{
        GT_BOOL     underDestroy;
        GT_BOOL     selectTaskAckDestroy;
    }cpssEventDestroy_info;
} PRV_CPSS_DRV_EVENT_HNDL_STC;


/*
 * Typedef: struct PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC
 *
 * Description: The control block of an interrupt queue.
 *
 * Fields:
 *      evNodeList  - A pointer to the first node in the queue.
 *      uniEvCause  - The unified event cause index.
 *      userHndlPtr - The user handle bound for the event.
 *      nextPtr     - The next bound event control.
 */
typedef struct PRV_CPSS_DRV_EV_REQ_Q_CTRL_STCT
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC  *evNodeList;
    GT_U32                          uniEvCause;
    PRV_CPSS_DRV_EVENT_HNDL_STC    *userHndlPtr;
    struct PRV_CPSS_DRV_EV_REQ_Q_CTRL_STCT  *nextPtr;
} PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC;


/*
 * typedef: struct PRV_CPSS_SW_TX_FREE_DATA_STC
 *
 * Description: This structure is used when working in Event Request Driven mode
 *              The structure saves the transmitted packet cookie, device and
 *              queue number. The structure is a part of per application FIFO.
 *
 * Fields:  userData    - A data to be stored in gtBuf on packet transmit
 *                        request, and returned to user on TxBufferQueue.
 *          devNum      - The transmitted device number.
 *          queueNum    - The transmitted queue number.
 *          nextPtr     - The next packet data in the FIFO.
 *
 */
typedef struct PRV_CPSS_SW_TX_FREE_DATA_STCT
{
    GT_PTR                  userData;
    GT_U8                   devNum;
    GT_U8                   queueNum;
    struct PRV_CPSS_SW_TX_FREE_DATA_STCT    *nextPtr;
} PRV_CPSS_SW_TX_FREE_DATA_STC;


/**
* @struct PRV_CPSS_TX_BUF_QUEUE_FIFO_STC
 *
 * @brief This structure is used when working in Event Request Driven mode.
 * The structure implements a FIFO of transmitted packets.
*/
typedef struct{

    PRV_CPSS_SW_TX_FREE_DATA_STC *headPtr;

    PRV_CPSS_SW_TX_FREE_DATA_STC *tailPtr;

} PRV_CPSS_TX_BUF_QUEUE_FIFO_STC;



/**
* @internal prvCpssDrvEvReqQInit function
* @endinternal
*
* @brief   This function initializes the interrupts queues structure, according to
*         the user defined parameters, or according to the default parameters.
* @param[in] numOfEvents              - the Maximal number of different events that
*                                       this module will support.
*
* @retval GT_OK   - on success
* @retval GT_FAIL - if failed
*
*/
GT_STATUS prvCpssDrvEvReqQInit
(
    GT_U32          numOfEvents
);

/*******************************************************************************
* prvCpssDrvEvReqQUserHndlGet
*
* DESCRIPTION:
*       This function returns the user handle pointer for the event.
*
* INPUTS:
*       uniIntIndex - The event number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       !=NULL on success, or
*       NULL if failed.
*
* COMMENTS:
*
*******************************************************************************/
PRV_CPSS_DRV_EVENT_HNDL_STC  *prvCpssDrvEvReqQUserHndlGet
(
    IN GT_U32           uniIntIndex
);

/**
* @internal prvCpssDrvEvReqQUserHndlSet function
* @endinternal
*
* @brief   This function sets the user handle pointer for the event.
*
* @param[in] uniIntIndex              - The event number.
* @param[in] evQNewHndlPtr            - The new user handler
*
* @retval GT_OK   - on success
* @retval GT_FAIL - if failed
*/
GT_STATUS  prvCpssDrvEvReqQUserHndlSet
(
    IN GT_U32           uniIntIndex,
    IN PRV_CPSS_DRV_EVENT_HNDL_STC    *evQNewHndlPtr
);

/**
* @internal prvCpssDrvEvReqQUserHndlUnSet function
* @endinternal
*
* @brief   This function unsets the user handle pointer for the event.
*
* @param[in] uniEvCause               - The event number.
*
* @retval GT_OK   - on success
* @retval GT_FAIL - if failed
*/
GT_STATUS  prvCpssDrvEvReqQUserHndlUnSet
(
    IN GT_U32    uniEvCause
);

/**
* @internal prvCpssDrvEvReqQInsert function
* @endinternal
*
* @brief   This function inserts a new event node to the tail of the event queue.
*         The active event counter is incremented.
* @param[in] evNodePool[]             - Pointer to the interrupt nodes pool the interrupt belongs
*                                      to.
*                                      intIndex    - The interrupt index.
* @param[in] masked                   - Indicates if the interrupt was received while it was
*                                       masked
*
* @retval GT_OK   - on success
* @retval GT_FAIL - if failed
*
* @note This routine is invoked from ISR context !! If Application process needs
*       to invoke the routine, be sure to protect the call with task and
*       interrupt lock.
*/
GT_STATUS prvCpssDrvEvReqQInsert
(
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC      evNodePool[],
    IN GT_U32           evIndex,
    IN GT_BOOL          masked
);

/*******************************************************************************
* prvCpssDrvEvReqQGet
*
* DESCRIPTION:
*       This function gets the first interrupt node information from the
*       selected interrupt queue. (The selection is made according to the
*       interrupts handling policy).
*
* INPUTS:
*       evQueueId  - The interrupts queue Id.
*
* OUTPUTS:
*       evWeight - The interrupt weight, according to it the interrupt handling
*                   should be repeated.
*       evQueuePriority  - The interrupt queue priority, from which the event
*                           was received in.
*
* RETURNS:
*       A pointer to the selected interrupt node, or NULL if no interrupts left.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DRV_EV_REQ_NODE_STC *prvCpssDrvEvReqQGet
(
    IN GT_U32           evIndex
);

/**
* @internal prvCpssDrvEvReqQBitmapGet function
* @endinternal
*
* @brief   This function returns a bitmap reflecting the unified events waiting
*         in queue for the user application.
* @param[in] hndl                     - The user handle.
* @param[in,out] evBitmapArr[]        - in : the bitmap array.
*                                       out: updated bitmat array. The number
*                                            of new events in bitmap.
* @param[in] evBitmapLength           - The size of bitmap array in words.
*
* @return  The number of new events in bitmap.
*/
GT_U32 prvCpssDrvEvReqQBitmapGet
(
    IN GT_UINTPTR       hndl,
    INOUT GT_U32        evBitmapArr[],
    IN GT_U32           evBitmapLength
);


/**
* @internal prvCpssDrvEvReqQRemoveDev function
* @endinternal
*
* @brief   This function is called upon Hot removal of a device, in order to remove
*         the interrupt nodes belonging to this device from the interrupts queues.
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvEvReqQRemoveDev
(
    IN  GT_U8           devNum
);

/**
* @internal prvCpssDrvEvReqQReInit function
* @endinternal
*
* @brief   This function re-initializes the specified interrupts queues structure,
*         according to the user defined parameters, or according to the default
*         parameters.
* @param[in] numOfEvents              - Number of interrupt events.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvEvReqQReInit
(
    IN  GT_U32          numOfEvents
);

/**
* @internal prvCpssDrvEvReqQRcvedEvRemoveAll function
* @endinternal
*
* @brief   This routine removes all user bound events that are active and record
*         status is Idle Ready. All removed events are unmasked.
* @param[in] hndl                     - The user handle.
*/
GT_VOID prvCpssDrvEvReqQRcvedEvRemoveAll
(
    IN GT_UINTPTR       hndl
);

/* a definition of extra data that can be used in function
    prvCpssDrvEvReqUniEvMaskSet , do indicate that evExtData is not for
    specific interrupt but to all */
#define PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS   0xFFEEFFEE

/**
* @internal prvCpssDrvEvReqUniEvMaskSet function
* @endinternal
*
* @brief   This routine mask/unmask the selected unified event.
*
* @param[in] devNum                   - The device number.
* @param[in] intNodesPool[]           - The event node array for the device.
* @param[in] intNodesPoolSize         The number of entries in the intNodesPool.
* @param[in] uniEvent                 - The unified event to be unmasked.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*                                      may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                                      to indicate 'ALL interrupts' that relate to this unified
*                                      event
* @param[in] operation                - the  : mask / unmask
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - the unified event or the evExtData within the unified
*                                       event are not found in this device interrupts
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*/
GT_STATUS prvCpssDrvEvReqUniEvMaskSet
(
    IN GT_U8                devNum,
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC             intNodesPool[],
    IN GT_U32               intNodesPoolSize,
    IN GT_U32               uniEvent,
    IN GT_U32               evExtData,
    IN CPSS_EVENT_MASK_SET_ENT operation
);

/**
* @internal prvCpssDrvEvReqUniEvMaskGet function
* @endinternal
*
* @brief   This routine gets data if the selected unified event masked or unmasked
*
* @param[in] devNum                   - The device number.
* @param[in] intNodesPool[]           - The event node array for the device.
* @param[in] intNodesPoolSize         The number of entries in the intNodesPool.
* @param[in] uniEvent                 - The unified event to be unmasked.
* @param[in] evExtData                - The additional data (port num / priority
*                                      queue number / other ) the event was received upon.
*                                      may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                                      to indicate 'ALL interrupts' that relate to this unified
*                                      event
*
* @param[out] maskedPtr                - is interrupt masked or unmasked
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - the unified event or the evExtData within the unified
*                                       event are not found in this device interrupts
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on bad pointer
*
* @note This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*/
GT_STATUS prvCpssDrvEvReqUniEvMaskGet
(
    IN  GT_U8                               devNum,
    IN  PRV_CPSS_DRV_EV_REQ_NODE_STC        intNodesPool[],
    IN  GT_U32                              intNodesPoolSize,
    IN  GT_U32                              uniEvent,
    IN  GT_U32                              evExtData,
    OUT CPSS_EVENT_MASK_SET_ENT            *maskedPtr
);

/**
* @internal prvCpssDrvEvReqNotify function
* @endinternal
*
* @brief   This function notify the waiting layer about the occurrence of event.
*         this function will "Signal" the waiting task or will call the
*         Application's call back that is bound to the handler
* @param[in] hndlEvPtr                - (Pointer to) the handler info.
*
* @note This routine is invoked from ISR context !!
*
*/
GT_VOID prvCpssDrvEvReqNotify
(
    IN PRV_CPSS_DRV_EVENT_HNDL_STC      *hndlEvPtr
);

/**
* @internal prvCpssDrvComIntEvReqQueuesDbRelease function
* @endinternal
*
* @brief   private (internal) function to release the DB of the common driver for request driven.
*         NOTE: function 'free' the allocated memory.
*         and restore DB to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssDrvComIntEvReqQueuesDbRelease
(
    void
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDrvComIntEvReqQueuesh */
