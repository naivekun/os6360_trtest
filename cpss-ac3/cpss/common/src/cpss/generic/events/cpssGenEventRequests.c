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
* @file cpssGenEventRequests.c
*
* @brief Includes unified event routine. The routines allow a user application
* to bind, select and receive events from PP, XBAR and FA devices.
*
* @version   15
********************************************************************************
*/

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/cpssPresteraDefs.h>

#include <cpss/generic/events/cpssGenEventUnifyTypes.h>

#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpss/generic/events/cpssGenEventCtrl.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>


static GT_STATUS prvCpssGenEventGppIsrConnect(void);

static PRV_CPSS_EVENT_MASK_DEVICE_SET_FUNC eventMaskDeviceFuncArray[PRV_CPSS_DEVICE_TYPE_LAST_E] =
                                                                        {NULL , NULL , NULL, NULL};

static PRV_CPSS_EVENT_MASK_DEVICE_GET_FUNC eventMaskGetDeviceFuncArray[PRV_CPSS_DEVICE_TYPE_LAST_E] =
                                                                        {NULL , NULL , NULL, NULL};

static PRV_CPSS_EVENT_GENERATE_DEVICE_SET_FUNC eventGenerateDeviceFuncArray[PRV_CPSS_DEVICE_TYPE_LAST_E] =
                                                                        {NULL , NULL , NULL, NULL};

static PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC eventGppIsrConnectFunc =
                                (PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC)prvCpssGenEventGppIsrConnect;

PRV_CPSS_EVENT_EXT_DATA_CONVERT_FUNC prvCpssEventExtDataConvertFunc = NULL;

PRV_CPSS_EVENT_MAC_EXT_DATA_CONVERT_FUNC prvCpssMacEventExtDataConvertFunc = NULL;

/*******************************************************************************
* Internal definitions
*******************************************************************************/

/*#define EV_REQ_DEBUG*/

#ifdef EV_REQ_DEBUG
    #define DBG_INFO(x)     cpssOsPrintf x

    static char * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
    #ifdef ASIC_SIMULATION
        #define DBG_LOG(x)  cpssOsPrintf x
    #else
        extern int logMsg(char *, int, int, int, int, int, int);
        #define DBG_LOG(x)  logMsg x
    #endif
#else
    #define DBG_INFO(x)
    #define DBG_LOG(x)
#endif

static GT_STATUS uniEvHndlSet(GT_U32 uniEvent, PRV_CPSS_DRV_EVENT_HNDL_STC *evHndlPtr);

/*******************************************************************************
* Internal usage variables
*******************************************************************************/

/**
* @internal prvCpssEventBind function
* @endinternal
*
* @brief   MODE PRV_CPSS_DRV_HANDLE_BIND_USE_SIGNAL_TYPE_E :
*         This routine binds a user process to unified event. The routine returns
*         a handle that is used when the application wants to wait for the event
*         (cpssEventSelect), receive the event(cpssEventRecv) or transmit a packet
*         using the Network Interface.
*         MODE PRV_CPSS_DRV_HANDLE_BIND_USE_APPL_CB_TYPE_E :
*         This routine binds a user process to unified event. The routine returns
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
*         NOTE : the function does not mask/unmask the HW events in any device.
*         This is Application responsibility to unmask the relevant events
*         on the needed devices , using function cpssEventDeviceMaskSet or
*         cpssEventDeviceMaskWithEvExtDataSet
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] handleBindType           - handle bind type.
* @param[in] uniEventArr[]            - The unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] isrCbPtr                 - (pointer for) application ISR function to be called when
*                                      events are received
*                                      relevant only when PRV_CPSS_DRV_HANDLE_BIND_USE_APPL_CB_TYPE_E
* @param[in] cookie                   - storing application data (cookie)
*                                      relevant only when PRV_CPSS_DRV_HANDLE_BIND_USE_APPL_CB_TYPE_E
*
* @param[out] hndlPtr                  - The user handle for the bounded events.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_FULL                  - when trying to set the "tx buffer queue unify event"
*                                       (CPSS_PP_TX_BUFFER_QUEUE_E) with other events in the same
*                                       handler
* @retval GT_ALREADY_EXIST         - one of the unified events already bound to another
*                                       handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)
*/
static GT_STATUS prvCpssEventBind
(
    IN  PRV_CPSS_DRV_HANDLE_BIND_TYPE_ENT   handleBindType,
    IN  CPSS_UNI_EV_CAUSE_ENT               uniEventArr[],
    IN  GT_U32                              arrLength,
    IN  CPSS_EVENT_ISR_CB_FUNC              isrCbPtr,
    IN  GT_VOID*                            cookie,
    OUT GT_UINTPTR                          *hndlPtr
)
{
    PRV_CPSS_DRV_EVENT_HNDL_STC *evHndlPtr; /* The event handle pointer     */
    CPSS_OS_SIG_SEM         semId=0;        /* The signalling semaphore Id  */
    char                    semName[30];    /* The semaphore name           */
    static GT_U32           semCnt = 0;     /* The semaphore counter        */
    GT_STATUS               rc=GT_OK;       /* The return code              */
    GT_U32                  i;              /* Iterator                     */

    /* validate input */
    CPSS_NULL_PTR_CHECK_MAC(hndlPtr);
    if(arrLength)
    {
        CPSS_NULL_PTR_CHECK_MAC(uniEventArr);
    }

    /* allocate a user handle */
    evHndlPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRV_EVENT_HNDL_STC));

    if (NULL == evHndlPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsBzero((char *)evHndlPtr, sizeof(PRV_CPSS_DRV_EVENT_HNDL_STC));

    evHndlPtr->hndlBindType = handleBindType;

    if(handleBindType == PRV_CPSS_DRV_HANDLE_BIND_USE_SIGNAL_TYPE_E)
    {
        cpssOsSprintf(semName, "EvBind_%d", semCnt++);

        /* create semaphore for signalling the user process a new event arrived */
        rc = cpssOsSigSemBinCreate(semName, CPSS_OS_SEMB_EMPTY_E, &semId);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        /* save the semaphore Id in the user handle */
        PRV_CPSS_DRV_HANDEL_SEM_MAC(evHndlPtr) = (GT_UINTPTR)semId;
        PRV_CPSS_DRV_HANDEL_SEM_SIGNAL_MAC(evHndlPtr) = GT_FALSE;
    }
    else if(handleBindType == PRV_CPSS_DRV_HANDLE_BIND_USE_APPL_CB_TYPE_E)
    {
        /* check that application gave non-NULL pointer */
        CPSS_NULL_PTR_CHECK_MAC(isrCbPtr);

        /* save Application's CB function and cookie */
        PRV_CPSS_DRV_HANDEL_APPL_CB_MAC(evHndlPtr) = isrCbPtr;
        PRV_CPSS_DRV_HANDEL_APPL_COOKIE_MAC(evHndlPtr) = cookie;
    }
    else
    {
        /* should not happen */
        rc = GT_BAD_PARAM;
        goto exit_cleanly_lbl;
    }


    evHndlPtr->evType = PRV_CPSS_DRV_REGULAR_EVENT_E;

    /* set the new handle in the unified event queue and unmask the interrupt */
    for (i = 0; i < arrLength; i++)
    {
        if (uniEventArr[i] == CPSS_PP_TX_BUFFER_QUEUE_E)
        {
            /* Tx buffer queue event type is unique, more than one process can bind  */
            /* the event and the notifying of the event arrival is per      */
            /* process. Allocate a TxBufferQueue FIFO for the ISR usage.    */
            if (i != 0)
            {
                /* TxBufferQueue event MUST be bound alone, without any other events */
                rc = GT_FULL;
                goto exit_cleanly_lbl;
            }

            evHndlPtr->evType     = PRV_CPSS_DRV_TX_BUFF_QUEUE_EVENT_E;
            evHndlPtr->extDataPtr = cpssOsMalloc(sizeof(PRV_CPSS_TX_BUF_QUEUE_FIFO_STC));

            if (NULL == evHndlPtr->extDataPtr)
            {
                rc = GT_OUT_OF_CPU_MEM;
                goto exit_cleanly_lbl;
            }

            ((PRV_CPSS_TX_BUF_QUEUE_FIFO_STC *)evHndlPtr->extDataPtr)->headPtr = NULL;
            ((PRV_CPSS_TX_BUF_QUEUE_FIFO_STC *)evHndlPtr->extDataPtr)->tailPtr = NULL;
        }
        else
        {
            if (PRV_CPSS_DRV_TX_BUFF_QUEUE_EVENT_E == evHndlPtr->evType)
            {
                /* TxBufferQueue event MUST be bound alone, without any other events */
                rc = GT_FULL;
                goto exit_cleanly_lbl;
            }
        }

        /* save the user handle in unified queue control block */
        PRV_CPSS_INT_SCAN_LOCK();
        rc = uniEvHndlSet(uniEventArr[i], evHndlPtr);
        PRV_CPSS_INT_SCAN_UNLOCK();
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

exit_cleanly_lbl:

    if (rc != GT_OK)
    {
        if(evHndlPtr->extDataPtr)
        {
            cpssOsFree(evHndlPtr->extDataPtr);
        }
        cpssOsFree(evHndlPtr);

        *hndlPtr = 0;
        if(semId)
        {
            cpssOsSigSemDelete(semId);
        }

        return rc;
    }

    /* and update the user handle */
    *hndlPtr = (GT_UINTPTR)evHndlPtr;

    return GT_OK;
}


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
* @param[out] hndlPtr                  - The user handle for the bounded events.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_FULL                  - when trying to set the "tx buffer queue unify event"
*                                       (CPSS_PP_TX_BUFFER_QUEUE_E) with other events in the same
*                                       handler
* @retval GT_ALREADY_EXIST         - one of the unified events already bound to another
*                                       handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)
*/
GT_STATUS cpssEventBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT      uniEventArr[],
    IN  GT_U32               arrLength,
    OUT GT_UINTPTR           *hndlPtr
)
{
    return prvCpssEventBind(PRV_CPSS_DRV_HANDLE_BIND_USE_SIGNAL_TYPE_E,
                            uniEventArr,arrLength,
                            NULL,0,/* 2 don't care parameters */
                            hndlPtr);
}

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
)
{
    GT_STATUS rc;
    GT_U32    i;

    /* validate input */
    if(arrLength)
    {
        CPSS_NULL_PTR_CHECK_MAC(uniEventArr);
    }

    for(i = 0; i < arrLength; i++)
    {
        if(NULL == prvCpssDrvEvReqQUserHndlGet(uniEventArr[i]))
        {
            /* skip the event that is not bound currently */
            continue;
        }

        /* unset the event */
        rc = prvCpssDrvEvReqQUserHndlUnSet(uniEventArr[i]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


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
)
{
    PRV_CPSS_DRV_EVENT_HNDL_STC *evHndlPtr; /* The event handle pointer     */
    PRV_CPSS_DRV_HANDLE_BIND_TYPE_ENT hndlBindType; /* Save handle bind type */
    GT_U32  iterator;

    /* validate the handle */
    if (0 == hndl)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    evHndlPtr = (PRV_CPSS_DRV_EVENT_HNDL_STC*)hndl;
    PRV_CPSS_INT_SCAN_LOCK();

    hndlBindType = evHndlPtr->hndlBindType;
    if(hndlBindType != PRV_CPSS_DRV_HANDLE_BIND_USE_SIGNAL_TYPE_E &&
       hndlBindType != PRV_CPSS_DRV_HANDLE_BIND_USE_APPL_CB_TYPE_E)
    {
        /* the hndl parameter is not legal --> was not created by
           cpssEventBind(...) / cpssEventIsrBind(...) */
        PRV_CPSS_INT_SCAN_UNLOCK();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* notify the 'select' that the event handler is going down */
    evHndlPtr->cpssEventDestroy_info.underDestroy = GT_TRUE;
    evHndlPtr->hndlBindType = 0xFF;/* make sure that even if application
            will keep calling with this invalid handler ... it will fail in the
            CPSS */

    /*check if semaphore was created*/
    if((hndlBindType == PRV_CPSS_DRV_HANDLE_BIND_USE_SIGNAL_TYPE_E) &&
        PRV_CPSS_DRV_HANDEL_SEM_MAC(evHndlPtr))
    {
        cpssOsSigSemSignal((CPSS_OS_SIG_SEM)PRV_CPSS_DRV_HANDEL_SEM_MAC(evHndlPtr));
        iterator = 100;
        while(evHndlPtr->cpssEventDestroy_info.selectTaskAckDestroy == GT_FALSE &&
            (iterator--))
        {
            /* wait for the 'select' to be finished*/
            cpssOsTimerWkAfter(10);
        }
        cpssOsSigSemDelete((CPSS_OS_SIG_SEM)PRV_CPSS_DRV_HANDEL_SEM_MAC(evHndlPtr));
    }
    FREE_PTR_MAC(evHndlPtr->extDataPtr);
    FREE_PTR_MAC(evHndlPtr);
    PRV_CPSS_INT_SCAN_UNLOCK();

    return GT_OK;
}


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
* @param[out] evBitmapArr[]            - The bitmap array.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
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
)
{
    PRV_CPSS_DRV_EVENT_HNDL_STC *evHndlPtr; /* The event handle pointer     */
    GT_STATUS               rc;             /* Return Code                  */

    /* validate the handle */
    if (0 == hndl)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* validate bitmap parameters */
    if(evBitmapArrLength != 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(evBitmapArr);
    }

    evHndlPtr = (PRV_CPSS_DRV_EVENT_HNDL_STC*)hndl;

    if(evHndlPtr->hndlBindType != PRV_CPSS_DRV_HANDLE_BIND_USE_SIGNAL_TYPE_E)
    {
        /* the hndl parameter is not legal --> was not created by
           cpssEventBind(...) */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Remove all events that were received by user and did not */
    /* occure since last select.                                */
    prvCpssDrvEvReqQRcvedEvRemoveAll(hndl);

    /* get the event bitmap that occurred */
    if (prvCpssDrvEvReqQBitmapGet(hndl, evBitmapArr, evBitmapArrLength) > 0)
    {
        /* clear the semaphore if signalled */
        cpssOsSigSemWait((CPSS_OS_SIG_SEM)PRV_CPSS_DRV_HANDEL_SEM_MAC(evHndlPtr), CPSS_OS_SEM_NO_WAIT_CNS);

        /* To prevent loss of signal for events which might occur after waiting */
        /* for the semaphore ended until the next waiting period, the "semSignal" */
        /* must be reset here. This prohibit the situation that events are waiting */
        /* in the queue but no signaling was done. The opposite scenario can occur, */
        /* not all events cause signaling, but due to the order of proccessing at */
        /* least one of the waiting events will cause signaling, and as a result */
        /* all other waiting events will be dealt.*/
        PRV_CPSS_DRV_HANDEL_SEM_SIGNAL_MAC(evHndlPtr) = GT_FALSE;

        return GT_OK;
    }

waitForSignal_lbl:
    /* wait on semaphore for event to happen */
    if (NULL == timeoutPtr)
    {
        /* wait forever */
        rc = cpssOsSigSemWait((CPSS_OS_SIG_SEM)PRV_CPSS_DRV_HANDEL_SEM_MAC(evHndlPtr),
                            CPSS_OS_SEM_WAIT_FOREVER_CNS);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        if (*timeoutPtr > 0)
        {
            /* wait for the defined timeout */
            rc = cpssOsSigSemWait((CPSS_OS_SIG_SEM)PRV_CPSS_DRV_HANDEL_SEM_MAC(evHndlPtr), *timeoutPtr);

            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /* no wait for timeout, clear the semaphore */
            cpssOsSigSemWait((CPSS_OS_SIG_SEM)PRV_CPSS_DRV_HANDEL_SEM_MAC(evHndlPtr), CPSS_OS_SEM_NO_WAIT_CNS);
        }
    }

    if(evHndlPtr->cpssEventDestroy_info.underDestroy == GT_TRUE)
    {
        /* The event handler is going down ... so need to return */
        /* but indicate that the 'select' is finished */

        evHndlPtr->cpssEventDestroy_info.selectTaskAckDestroy = GT_TRUE;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ABORTED, LOG_ERROR_NO_MSG);
    }

    /* To prevent loss of signal for events which might occur after waiting */
    /* for the semaphore ended untill the next waiting period, the "semSignal" */
    /* must be reset here. This prohibit the situation that events are waiting */
    /* in the queue but no signaling was done. The opposite scenario can occur, */
    /* not all events cause signaling, but due to the order of proccessing at */
    /* least one of the waiting events will cause signaling, and as a result */
    /* all other waiting events will be dealt.*/
    PRV_CPSS_DRV_HANDEL_SEM_SIGNAL_MAC(evHndlPtr) = GT_FALSE;

    /* we got new events, get the new bitmap */
    if (prvCpssDrvEvReqQBitmapGet(hndl, evBitmapArr, evBitmapArrLength) == 0)
    {
        if(timeoutPtr == NULL)
        {
            /*
                in that case we got indication that there are no events to handle.
                even that the handle was signaled.

                explanation : how did it happened ?

                assume that current handle bound event that represent several
                interrupt bits in HW --> like 'link change' that each port has
                it's own interrupt bit
                assume:
                1. link change on one port --> the ISR will signal the waiting task.
                2. the waiting task will start to work (after ISR finish)
                    a. will start handle the event
                -- BUT during that the link change on other port --> the ISR will signal the waiting task.
                    b. will handle also the second event (before waiting to the semaphore)
                3. finally when coreEventSelect will be called after the events
                   where handle , the function will wait for the semaphore
                -- BUT the semaphore is 'ON' it was signaled already !!!
                --> meaning that the coreEventSelect will continue to run when
                there are no actual event waiting !!!


                that is the reason that we return to wait for the signal.


                NOTE:
                also in the handle of 'tx buffer queue' ('tx ended') may get to
                similar situation (even the handle hold only single event type)

                explanation : how did it happened -- for 'tx buffer queue' ?

                assume that the task handle the 'tx buffer queue'
                1. the event occur due to first TX packet from CPU --> ISR signal the waiting task.
                2. the waiting task will start to work (after ISR finish)
                    a. will start handle the event
                -- BUT during that the same event happen again (due to other TX from cpu)
                    --> the ISR will signal the waiting task.
                    b. will handle also the second event (before waiting to the semaphore)
                3. finally when coreEventSelect will be called after the events
                   where handle , the function will wait for the semaphore
                -- BUT the semaphore is 'ON' it was signaled already !!!
                --> meaning that the coreEventSelect will continue to run when
                there are no actual event waiting !!!

            */

            goto waitForSignal_lbl;
        }
        else
        {
            /* there are no current events to handle */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}


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
    IN  CPSS_UNI_EV_CAUSE_ENT     evCause,
    OUT GT_U32              *evExtDataPtr,
    OUT GT_U8               *evDevPtr
)
{
    PRV_CPSS_DRV_EVENT_HNDL_STC *evHndlPtr; /* The event handle pointer     */
    PRV_CPSS_DRV_EV_REQ_NODE_STC*evNodePtr; /* The event node pointer       */
    GT_STATUS rc;

    /* validate the handle */
    if (0 == hndl)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(evExtDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(evDevPtr);

    evHndlPtr = (PRV_CPSS_DRV_EVENT_HNDL_STC*)hndl;

    if (PRV_CPSS_DRV_TX_BUFF_QUEUE_EVENT_E == evHndlPtr->evType)
    {
        /* Tx End event type does not support this API */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get an event from queue */
    evNodePtr = prvCpssDrvEvReqQGet(evCause);

    if (NULL == evNodePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }

    *evExtDataPtr  = evNodePtr->uniEvExt;
    *evDevPtr      = evNodePtr->devNum;

    if (prvCpssMacEventExtDataConvertFunc)
    {
        if (CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E == evCause ||
            CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E == evCause    ||
            CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E == evCause )
        {
            /* invoke the callback function to convert Mac event's extended data */
            rc = (prvCpssMacEventExtDataConvertFunc)(evNodePtr->devNum, evExtDataPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if (prvCpssEventExtDataConvertFunc)
    {
        /* invoke the callback function to convert event's extended data */
        rc = (prvCpssEventExtDataConvertFunc)(evNodePtr->devNum,
                                              evCause,
                                              PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E,
                                              evExtDataPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal uniEvHndlSet function
* @endinternal
*
* @brief   This routine sets the new user event handle in the queue control.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] uniEvent                 - The unified event index.
* @param[in] evHndlPtr                - The user event handle.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - general failure
* @retval GT_ALREADY_EXIST         - one of the unified events already bound to another
*                                       handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)
*/
static GT_STATUS uniEvHndlSet
(
    IN GT_U32                       uniEvent,
    IN PRV_CPSS_DRV_EVENT_HNDL_STC  *evHndlPtr
)
{
    /* regular events can be bound only once, validate it */
    if (PRV_CPSS_DRV_REGULAR_EVENT_E == evHndlPtr->evType)
    {

        if(NULL != prvCpssDrvEvReqQUserHndlGet(uniEvent))
        {
            DBG_LOG(("uniEvent %s already bound\n",
                     (GT_U32)uniEvName[uniEvent],2,3,4,5,6));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }
    }

    return prvCpssDrvEvReqQUserHndlSet(uniEvent, evHndlPtr);
}

/**
* @internal deviceMaskWithEvExtDataSet function
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
*                                      may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                                      to indicate 'ALL interrupts' that relate to this unified
*                                      event
* @param[in] operation                - the operation: mask / unmask
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
static GT_STATUS deviceMaskWithEvExtDataSet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData,
    IN CPSS_EVENT_MASK_SET_ENT  operation
)
{
    GT_STATUS               rc = GT_BAD_PARAM;/* The Return Code            */
    PRV_CPSS_DEVICE_TYPE_ENT deviceType;/*device type that the event relate to*/

    if(operation != CPSS_EVENT_MASK_E &&
       operation != CPSS_EVENT_UNMASK_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(operation);
    }

    if (uniEvent <= CPSS_SRVCPU_MAX_E)
    {
        /* events of PP */
        deviceType = PRV_CPSS_DEVICE_TYPE_PP_E;
    }
    else if(uniEvent >= CPSS_XBAR_UNI_EV_MIN_E &&
            uniEvent <= CPSS_XBAR_UNI_EV_MAX_E)
    {
        /* events of XBAR */
        deviceType = PRV_CPSS_DEVICE_TYPE_XBAR_E;
    }
    else if(uniEvent >= CPSS_FA_UNI_EV_MIN_E &&
            uniEvent <= CPSS_FA_UNI_EV_MAX_E)
    {
        /* events of FA */
        deviceType = PRV_CPSS_DEVICE_TYPE_FA_E;
    }
    else if(uniEvent >= CPSS_DRAGONITE_UNI_EV_MIN_E &&
            uniEvent <= CPSS_DRAGONITE_UNI_EV_MAX_E)
    {
        /* events of Dragonite */
        deviceType = PRV_CPSS_DEVICE_TYPE_DRAGONITE_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "uniEvent[%d] is not supported",uniEvent);
    }

    if(eventMaskDeviceFuncArray[deviceType] == NULL)
    {
        /* there is no callback function , relate to those events .
        the initialization was not done for it ... */
        return /* this is not error for the ERROR LOG */ GT_NOT_INITIALIZED;
    }

    /* lock section to disable interruption of ISR while mask/unmask event */
    PRV_CPSS_INT_SCAN_LOCK();

    /* invoke the callback function that relate to this type of event */
    rc = (eventMaskDeviceFuncArray[deviceType])(devNum,uniEvent,
            evExtData,operation);

    /* unlock section to enable interruption of ISR when done */
    PRV_CPSS_INT_SCAN_UNLOCK();

    return rc;
}

/**
* @internal deviceMaskWithEvExtDataGet function
* @endinternal
*
* @brief   This routine gets data if an unified event on specific device masked or unmasked
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
*                                      may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                                      to indicate 'ALL interrupts' that relate to this unified
*                                      event
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
static GT_STATUS deviceMaskWithEvExtDataGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_U32                   evExtData,
    OUT CPSS_EVENT_MASK_SET_ENT *maskedPtr
)
{
    GT_STATUS               rc = GT_BAD_PARAM;/* The Return Code            */
    PRV_CPSS_DEVICE_TYPE_ENT deviceType;/*device type that the event relate to*/

    if(NULL == maskedPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (uniEvent <= CPSS_SRVCPU_MAX_E)
    {
        /* events of PP */
        deviceType = PRV_CPSS_DEVICE_TYPE_PP_E;
    }
    else if(uniEvent >= CPSS_XBAR_UNI_EV_MIN_E &&
            uniEvent <= CPSS_XBAR_UNI_EV_MAX_E)
    {
        /* events of XBAR */
        deviceType = PRV_CPSS_DEVICE_TYPE_XBAR_E;
    }
    else if(uniEvent >= CPSS_FA_UNI_EV_MIN_E &&
            uniEvent <= CPSS_FA_UNI_EV_MAX_E)
    {
        /* events of FA */
        deviceType = PRV_CPSS_DEVICE_TYPE_FA_E;
    }
    else if(uniEvent >= CPSS_DRAGONITE_UNI_EV_MIN_E &&
            uniEvent <= CPSS_DRAGONITE_UNI_EV_MAX_E)
    {
        /* events of Dragonite */
        deviceType = PRV_CPSS_DEVICE_TYPE_DRAGONITE_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(eventMaskGetDeviceFuncArray[deviceType] == NULL)
    {
        /* there is no callback function , relate to those events .
        the initialization was not done for it ... */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* lock section to disable interruption of ISR while mask/unmask event */
    PRV_CPSS_INT_SCAN_LOCK();

    /* invoke the callback function that relate to this type of event */
    rc = (eventMaskGetDeviceFuncArray[deviceType])(devNum,uniEvent,
            evExtData, maskedPtr);

    /* unlock section to enable interruption of ISR when done */
    PRV_CPSS_INT_SCAN_UNLOCK();

    return rc;
}



/**
* @internal cpssEventDeviceMaskSet function
* @endinternal
*
* @brief   This routine mask/unmasks an unified event on specific device - in HW.
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
* @retval GT_BAD_PARAM             - bad value on one of the parameters
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
)
{
    GT_STATUS rc;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    /* Treat all interrupts relate to this unified event */
    rc = deviceMaskWithEvExtDataSet(devNum,uniEvent,
            PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS,
            operation);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
)
{
    GT_STATUS rc;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    /* Treat interrupt(s) relate to this unified event and 'extra info' */
    rc = deviceMaskWithEvExtDataSet(devNum,uniEvent,
            evExtData,operation);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssEventDeviceMaskWithEvExtDataGet function
* @endinternal
*
* @brief   This routine gets data if an unified event on specific device masked or unmasked
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
)
{
    GT_STATUS rc;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    /* Treat interrupt(s) relate to this unified event and 'extra info' */
    rc = deviceMaskWithEvExtDataGet(devNum,uniEvent,
            evExtData,maskedPtr);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
)
{
    GT_STATUS                rc = GT_BAD_PARAM;/* The Return Code            */
    PRV_CPSS_DEVICE_TYPE_ENT deviceType;/*device type that the event relate to*/



    if (uniEvent <= CPSS_SRVCPU_MAX_E)
    {
        /* events of PP */
        deviceType = PRV_CPSS_DEVICE_TYPE_PP_E;
    }
    else if(uniEvent >= CPSS_XBAR_UNI_EV_MIN_E &&
            uniEvent <= CPSS_XBAR_UNI_EV_MAX_E)
    {
        /* events of XBAR */
        deviceType = PRV_CPSS_DEVICE_TYPE_XBAR_E;
    }
    else if(uniEvent >= CPSS_FA_UNI_EV_MIN_E &&
            uniEvent <= CPSS_FA_UNI_EV_MAX_E)
    {
        /* events of FA */
        deviceType = PRV_CPSS_DEVICE_TYPE_FA_E;
    }
    else if(uniEvent >= CPSS_DRAGONITE_UNI_EV_MIN_E &&
            uniEvent <= CPSS_DRAGONITE_UNI_EV_MAX_E)
    {
        /* events of Dragonite */
        deviceType = PRV_CPSS_DEVICE_TYPE_DRAGONITE_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(eventGenerateDeviceFuncArray[deviceType] == NULL)
    {
        /* there is no callback function , relate to those events .
         * the initialization was not done for it ... */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    /* lock section to disable interruption of ISR while mask/unmask event */
    PRV_CPSS_INT_SCAN_LOCK();

    /* invoke the callback function that relate to this type of event */
    rc = (eventGenerateDeviceFuncArray[deviceType])(devNum,uniEvent,
            evExtData);

    /* unlock section to enable interruption of ISR when done */
    PRV_CPSS_INT_SCAN_UNLOCK();

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
    IN  CPSS_UNI_EV_CAUSE_ENT       uniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_ISR_CB_FUNC      isrCbPtr,
    IN  GT_VOID*                    cookie,
    OUT GT_UINTPTR                  *hndlPtr
)
{
    return prvCpssEventBind(PRV_CPSS_DRV_HANDLE_BIND_USE_APPL_CB_TYPE_E,
                            uniEventArr,arrLength,
                            isrCbPtr,cookie,hndlPtr);
}

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
)
{
    PRV_CPSS_DRV_EVENT_HNDL_STC *evHndlPtr; /* The event handle pointer     */

    /* validate the handle */
    if (0 == hndl)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    evHndlPtr = (PRV_CPSS_DRV_EVENT_HNDL_STC*)hndl;

    if(evHndlPtr->hndlBindType != PRV_CPSS_DRV_HANDLE_BIND_USE_APPL_CB_TYPE_E)
    {
        /* the hndl parameter is not legal --> was not created by
           cpssEventIsrBind(...) */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(prvCpssDrvEvReqQBitmapGet(hndl,evBitmapArr,evBitmapLength) == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


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
)
{
    /* validate the handle */
    if (0 == hndl)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEvReqQRcvedEvRemoveAll(hndl);

    return GT_OK;
}

/**
* @internal prvCpssGenEventMaskDeviceBind function
* @endinternal
*
* @brief   This routine mask/unmasks an unified event on specific device.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] deviceType               - device Type - PP/FA/Xbar device
* @param[in] funcPtr                  - (pointer to) function that will be called when the
*                                      Application wants to mask/unmask events relate to the
* @param[in] deviceType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad deviceType value or bad device number
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssGenEventMaskDeviceBind
(
    IN PRV_CPSS_DEVICE_TYPE_ENT    deviceType,
    IN PRV_CPSS_EVENT_MASK_DEVICE_SET_FUNC funcPtr
)
{
    /* check that the caller gave non-NULL function pointer ,
       if caller not want to handle events it must give pointer to dummy
       function and not NULL pointer */
    CPSS_NULL_PTR_CHECK_MAC(funcPtr);

    switch(deviceType)
    {
        case PRV_CPSS_DEVICE_TYPE_PP_E:
        case PRV_CPSS_DEVICE_TYPE_FA_E:
        case PRV_CPSS_DEVICE_TYPE_XBAR_E:
        case PRV_CPSS_DEVICE_TYPE_DRAGONITE_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    eventMaskDeviceFuncArray[deviceType] = funcPtr;

    return GT_OK;
}

/**
* @internal prvCpssGenEventMaskGetDeviceBind function
* @endinternal
*
* @brief   This routine prepare getting data about masking/unmasking an unified event on specific device.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] deviceType               - device Type - PP/FA/Xbar device
* @param[in] funcPtr                  - (pointer to) function that will be called when the
*                                      Application wants to mask/unmask events relate to the
* @param[in] deviceType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad deviceType value or bad device number
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssGenEventMaskGetDeviceBind
(
    IN PRV_CPSS_DEVICE_TYPE_ENT    deviceType,
    IN PRV_CPSS_EVENT_MASK_DEVICE_GET_FUNC funcPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(funcPtr);

    switch(deviceType)
    {
        case PRV_CPSS_DEVICE_TYPE_PP_E:
        case PRV_CPSS_DEVICE_TYPE_FA_E:
        case PRV_CPSS_DEVICE_TYPE_XBAR_E:
        case PRV_CPSS_DEVICE_TYPE_DRAGONITE_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    eventMaskGetDeviceFuncArray[deviceType] = funcPtr;

    return GT_OK;
}

/**
* @internal prvCpssGenEventGenerateDeviceBind function
* @endinternal
*
* @brief   This routine binds function that configure device to generate unified event for
*         specific element in the event associated with extra data.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] deviceType               - device Type - PP/FA/Xbar device
* @param[in] funcPtr                  - pointer to function that will be called when the
*                                      Application wants to generate events relate to the
* @param[in] deviceType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad deviceType value or bad device number
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssGenEventGenerateDeviceBind
(
    IN PRV_CPSS_DEVICE_TYPE_ENT    deviceType,
    IN PRV_CPSS_EVENT_GENERATE_DEVICE_SET_FUNC funcPtr
)
{
    /* check that the caller gave non-NULL function pointer ,
       if caller not want to handle events it must give pointer to dummy
       function and not NULL pointer */
    CPSS_NULL_PTR_CHECK_MAC(funcPtr);

    switch(deviceType)
    {
        case PRV_CPSS_DEVICE_TYPE_PP_E:
        case PRV_CPSS_DEVICE_TYPE_FA_E:
        case PRV_CPSS_DEVICE_TYPE_XBAR_E:
        case PRV_CPSS_DEVICE_TYPE_DRAGONITE_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    eventGenerateDeviceFuncArray[deviceType] = funcPtr;

    return GT_OK;
}

/**
* @internal prvCpssGenEventGppIsrConnectBind function
* @endinternal
*
* @brief   This function binds function that connects an Isr for a given Gpp
*         interrupt.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR) with GPP
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*
* @note To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*/
GT_STATUS prvCpssGenEventGppIsrConnectBind
(
    IN PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC funcPtr
)
{
    /* check that the caller gave non-NULL function pointer ,
       if caller not want to handle events it must give pointer to dummy
       function and not NULL pointer */
    CPSS_NULL_PTR_CHECK_MAC(funcPtr);

    eventGppIsrConnectFunc = funcPtr;

    return GT_OK;
}

/**
* @internal prvCpssGenEventExtDataConvertBind function
* @endinternal
*
* @brief   This routine converts extended data for unified event on specific device.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR) with GPP
* @param[in] funcPtr                  - pointer to function that will be called
*                                      to convert extended data for unified event on specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssGenEventExtDataConvertBind
(
    IN PRV_CPSS_EVENT_EXT_DATA_CONVERT_FUNC funcPtr
)
{
    /* check that the caller gave non-NULL function pointer ,
       if caller not want to handle events it must give pointer to dummy
       function and not NULL pointer */
    CPSS_NULL_PTR_CHECK_MAC(funcPtr);

    prvCpssEventExtDataConvertFunc = funcPtr;

    return GT_OK;
}

/**
* @internal prvCpssMacEventExtDataConvertBind function
* @endinternal
*
* @brief   This routine converts extended data for Mac related unified events on specific device.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @param[in] funcPtr                  - pointer to function that will be called
*                                      to convert extended data for Mac unified event on specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssMacEventExtDataConvertBind
(
    IN PRV_CPSS_EVENT_MAC_EXT_DATA_CONVERT_FUNC funcPtr
)
{
    /* check that the caller gave non-NULL function pointer */
    CPSS_NULL_PTR_CHECK_MAC(funcPtr);

    prvCpssMacEventExtDataConvertFunc = funcPtr;

    return GT_OK;
}

/**
* @internal prvCpssGenEventGppIsrConnect function
* @endinternal
*
* @brief   This function connects an Isr for a given Gpp interrupt.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR) with GPP
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*/
static GT_STATUS prvCpssGenEventGppIsrConnect
(
    void
)
{
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}


/**
* @internal cpssGenEventGppIsrConnect function
* @endinternal
*
* @brief   This function connects an Isr for a given Gpp interrupt.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR) with GPP
* @param[in] devNum                   - The Pp device number at which the Gpp device is connected.
* @param[in] gppId                    - The Gpp Id to be connected.
* @param[in] isrFuncPtr               - A pointer to the function to be called on Gpp interrupt
*                                      reception.
* @param[in] cookie                   - A cookie to be passed to the isrFuncPtr when
*                                       it is called.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*/
GT_STATUS cpssGenEventGppIsrConnect
(
    IN  GT_U8                   devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId,
    IN  CPSS_EVENT_ISR_FUNC     isrFuncPtr,
    IN  void                    *cookie
)
{
    return eventGppIsrConnectFunc(devNum,gppId,isrFuncPtr,cookie);
}

/**
* @internal cpssGenEventInterruptConnect function
* @endinternal
*
* @brief   Connect a given interrupt vector to an interrupt routine.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] intVecNum                - The interrupt vector number this device is connected to.
* @param[in] intMask                  - The interrupt mask to enable/disable interrupts on
*                                      this device.
* @param[in] intRoutine               - A pointer to the interrupt routine to be connected to the
*                                      given interrupt line.
* @param[in] cookie                   - A user defined cookie to be passed to the isr
*                                       on interrupt reception.
* @param[in] connectionId             Id to be used for future access to the connected isr.
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
GT_STATUS cpssGenEventInterruptConnect
(
    IN  GT_U32          intVecNum,
    IN  GT_U32          intMask,
    IN  CPSS_EVENT_ISR_FUNC    intRoutine,
    IN  void            *cookie,
    OUT GT_UINTPTR      *connectionId
)
{
    GT_STATUS retVal = GT_OK;
    retVal =  prvCpssDrvInterruptConnect(intVecNum,
                                         intMask,intRoutine,
                                         cookie,connectionId);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    retVal =  prvCpssDrvInterruptEnable(intMask);
    return retVal;
}

/**
* @internal prvCpssGenEventRequestsDbRelease function
* @endinternal
*
* @brief   private (internal) function to release the DB of the event related.
*         NOTE: function 'free' the allocated memory.
*         and restore DB to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssGenEventRequestsDbRelease
(
    void
)
{
    cpssOsMemSet(&eventMaskDeviceFuncArray,0,sizeof(eventMaskDeviceFuncArray));
    cpssOsMemSet(&eventMaskGetDeviceFuncArray,0,sizeof(eventMaskGetDeviceFuncArray));

    /* NOTE: this is application ability (and responsibility) to 'destroy' the
     'handlers' that hold allocated memory and semaphore that created during
     cpssEventBind(...)/cpssEventIsrBind(...)

     ---> see function cpssEventDestroy(...)
     */

    eventGppIsrConnectFunc = (PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC)prvCpssGenEventGppIsrConnect;

    return GT_OK;
}
