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
* @file cpssDrvComIntEvReqQueues.c
*
* @brief This file includes functions for managing and configuring the interrupt
* queues structure.
*
*
* @version   12
********************************************************************************
*/
/* get the OS , extDrv functions*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
/* get common defs */
#include <cpssCommon/cpssPresteraDefs.h>

/* get the common deriver info */
#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>

/**
* @struct PRV_CPSS_DRV_COMMON_INT_EVENT_REQUEST_QUEUES_STC
*
* @brief the DB of this file
*/
typedef struct{

    PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC *uniEvQArr;

    /** The size of the above array */
    GT_U32 uniEvQArrSize;

} PRV_CPSS_DRV_COMMON_INT_EVENT_REQUEST_QUEUES_STC;

/* the DB of this file */
static PRV_CPSS_DRV_COMMON_INT_EVENT_REQUEST_QUEUES_STC
                prvCpssDrvComIntEvReqQueuesDb = {NULL , 0};

/* #define  EV_DEBUG */

#ifdef EV_DEBUG
#define DUMP(_x)  cpssOsPrintSync _x
#else
#define DUMP(_x)
#endif

#ifdef CHX_FAMILY
GT_STATUS prvCpssDrvInterruptMaskGet
(
    IN  PRV_CPSS_DRV_EV_REQ_NODE_STC              *evNodePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                   *maskedPtr
);
#endif

extern PRV_CPSS_EVENT_EXT_DATA_CONVERT_FUNC prvCpssEventExtDataConvertFunc;
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
    IN  GT_U32      numOfEvents
)
{
    GT_U32      i;

    if (prvCpssDrvComIntEvReqQueuesDb.uniEvQArrSize < numOfEvents)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for(i = 0; i < numOfEvents; i++)
    {
        prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[i].evNodeList     = NULL;
        prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[i].userHndlPtr    = NULL;
        prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[i].nextPtr        = NULL;
        prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[i].uniEvCause     = i;
    }

    return GT_OK;
}


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
    IN GT_U32       numOfEvents
)
{
    /* Check if Initialization has already been performed.
       Should be performed once per system.
       Exist in PP,FA and XBAR modules */
    if(prvCpssDrvComIntEvReqQueuesDb.uniEvQArr != NULL)
    {
        return GT_OK;
    }

    prvCpssDrvComIntEvReqQueuesDb.uniEvQArr =
        cpssOsMalloc(sizeof(PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC) * numOfEvents);

    if(prvCpssDrvComIntEvReqQueuesDb.uniEvQArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvComIntEvReqQueuesDb.uniEvQArrSize = numOfEvents;

    return prvCpssDrvEvReqQReInit(numOfEvents);
}


/*******************************************************************************
* prvCpssDrvEvReqQUserHndlGet
*
* DESCRIPTION:
*       This function returns the user handle pointer for the event.
*
* INPUTS:
*       uniEvCause - The event number.
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
    IN GT_U32       uniEvCause
)
{
    if (prvCpssDrvComIntEvReqQueuesDb.uniEvQArrSize <= uniEvCause)
    {
        return NULL;
    }

    return prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause].userHndlPtr;
}


/**
* @internal prvCpssDrvEvReqQUserHndlSet function
* @endinternal
*
* @brief   This function sets the user handle pointer for the event.
*
* @param[in] uniEvCause               - The event number.
* @param[in] evQNewHndlPtr            - The new user handler
*
* @retval GT_OK   - on success
* @retval GT_FAIL - if failed
*/
GT_STATUS  prvCpssDrvEvReqQUserHndlSet
(
    IN GT_U32               uniEvCause,
    IN PRV_CPSS_DRV_EVENT_HNDL_STC        *evQNewHndlPtr
)
{
    if (prvCpssDrvComIntEvReqQueuesDb.uniEvQArrSize <= uniEvCause)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* link the user handle the new queue event control */
    prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause].nextPtr = evQNewHndlPtr->evListPtr;
    evQNewHndlPtr->evListPtr      = &prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause];

    /* save the user handle in queue control */
    prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause].userHndlPtr = evQNewHndlPtr;

    return GT_OK;
}

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
    IN GT_U32      uniEvCause
)
{
    if (prvCpssDrvComIntEvReqQueuesDb.uniEvQArrSize <= uniEvCause)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause].evNodeList     = NULL;
    prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause].userHndlPtr    = NULL;
    prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause].nextPtr        = NULL;

    return GT_OK;
}


/**
* @internal prvCpssDrvEvReqQInsert function
* @endinternal
*
* @brief   This function inserts a new event node to the tail of the event queue.
*         The active event counter is incremented.
* @param[in] evNodePool[]             - Pointer to the interrupt nodes pool the interrupt belongs
*                                      to.
* @param[in] intIndex                 - The interrupt index.
* @param[in] masked                   - Indicates if the interrupt was received while it was
* @param[in] masked
*
* @retval GT_OK   - on success
* @retval GT_FAIL - if failed
*
* @note This routine is invoked from ISR context !! If Application process needs
*       to invoke the routine, be shure to protect the call with task and
*       inerrupt lock.
*/
GT_STATUS prvCpssDrvEvReqQInsert
(
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC      evNodePool[],
    IN GT_U32           intIndex,
    IN GT_BOOL          masked
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC     *newNode;        /* The new node to be inserted.     */
    PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC   *evQueue;       /* The unified event queue          */

    /* set the node to be inserted */
    newNode = &(evNodePool[intIndex]);

    DUMP((" ISR QInsert int 0x%x stat %d masked %d\n",intIndex, newNode->intRecStatus, masked ));

    switch (newNode->intRecStatus)
    {
        case PRV_CPSS_DRV_EV_DRVN_INT_RCVD_E:
            /* int is allready masked, update the record status */
            newNode->intRecStatus = PRV_CPSS_DRV_EV_DRVN_INT_MASKED_E;
            return GT_OK;

        case PRV_CPSS_DRV_EV_DRVN_INT_MASKED_E:
            /* int is allready masked, do nothing */
            return GT_OK;

        default:
            break;
    }

    /* Insert a new node only if it was unmasked. */
    if (GT_TRUE == masked)
    {
        /* handle IDLE_READY that was not handled in the switch case above */
        if(newNode->intRecStatus == PRV_CPSS_DRV_EV_DRVN_INT_IDLE_READY_E)
        {
            /* int is allready masked, update the record status */
            newNode->intRecStatus = PRV_CPSS_DRV_EV_DRVN_INT_MASKED_E;
        }
        return GT_OK;
    }

    evQueue = &(prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[newNode->uniEvCause]);

    /* don't insert IDLE_READY interrupt to the queue. It already in queue.
      we need only send signal for IDLE_READY interrupt to avoid missing
      of the interrupt  */
    if(newNode->intRecStatus != PRV_CPSS_DRV_EV_DRVN_INT_IDLE_READY_E)
    {
        newNode->intRecStatus = PRV_CPSS_DRV_EV_DRVN_INT_RCVD_E;

        if (NULL == evQueue->evNodeList)
        {
            /* queue is empty */
            evQueue->evNodeList = newNode;
            newNode->nextPtr    = newNode;
            newNode->prevPtr    = newNode;
        }
        else
        {
            /* queue isn`t empty */
            evQueue->evNodeList->prevPtr->nextPtr   = newNode;
            newNode->nextPtr                        = evQueue->evNodeList;
            newNode->prevPtr                        = evQueue->evNodeList->prevPtr;
            evQueue->evNodeList->prevPtr            = newNode;
        }
    }
    else
    {
        /* notify Event Select procedure that event need to be handled
           one more time but not removed from queue */
        newNode->intRecStatus = PRV_CPSS_DRV_EV_DRVN_INT_MASKED_E;
    }

    /* and signal the user process */
    if (NULL != evQueue->userHndlPtr)
    {
        /* and signal the user process ,or */
        /* call the application's CB function */
        prvCpssDrvEvReqNotify(evQueue->userHndlPtr);
    }

    return GT_OK;
}


/*******************************************************************************
* prvCpssDrvEvReqQGet
*
* DESCRIPTION:
*       This function gets the first interrupt node information from the
*       selected unified event queue.
*
* INPUTS:
*       uniEvCause  - The unified event queue.
*
* OUTPUTS:
*       None
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
    IN GT_U32           uniEvCause
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC   *evNodePtr;         /* The event node pointer */
    PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC *evQueuePtr;        /* The event queue pointer*/
    PRV_CPSS_DRV_EV_REQ_NODE_STC   *lastEvNodePtr;     /* The last event in queue*/

    /* perform param and status validity */
    if (prvCpssDrvComIntEvReqQueuesDb.uniEvQArrSize <= uniEvCause)
    {
        return NULL;
    }

    evQueuePtr = &(prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause]);

    PRV_CPSS_INT_SCAN_LOCK();

    if (NULL == evQueuePtr->evNodeList)
    {
        PRV_CPSS_INT_SCAN_UNLOCK();
        /* the queue is empty */
        return NULL;
    }

    /* set the first and last events in queue */
    evNodePtr       = evQueuePtr->evNodeList;
    lastEvNodePtr   = evQueuePtr->evNodeList->prevPtr;

    /* search for the first active event in queue */
    while(1)
    {
        /* treat only received and masked events */
        if (PRV_CPSS_DRV_EV_DRVN_INT_MASKED_E == evNodePtr->intRecStatus ||
            PRV_CPSS_DRV_EV_DRVN_INT_RCVD_E == evNodePtr->intRecStatus)
        {
            /* we found an event, update the record status to */
            /* Idle Ready. This state is used on select to    */
            /* remove event from queue and unmask interrupt.  */
            evNodePtr->intRecStatus = PRV_CPSS_DRV_EV_DRVN_INT_IDLE_READY_E;

            /* update the list to the next event in queue */
            evQueuePtr->evNodeList = evNodePtr->nextPtr;
            break;
        }

        /* check that we did not reach end of queue */
        if (lastEvNodePtr == evNodePtr)
        {
            /* end of a queue */
            evNodePtr = NULL;

            /* make quick finish of the while loop without condition check */
            break;
        }

        /* if we reached here, proceed to the next node */
        evNodePtr = evNodePtr->nextPtr;

    }

    PRV_CPSS_INT_SCAN_UNLOCK();

    return evNodePtr;
}


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
)
{
    GT_U32              evCount;       /* number of new events */
    PRV_CPSS_DRV_EVENT_HNDL_STC *hndlEvPtr;    /* handle event list */
    PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC       *evReqPtr;      /*  */

    evCount             = 0;
    hndlEvPtr           = (PRV_CPSS_DRV_EVENT_HNDL_STC*)hndl;
    evReqPtr            = hndlEvPtr->evListPtr;

    if (NULL != evBitmapArr)
    {
        /* zero out the bitmap */
        cpssOsBzero((GT_VOID*)evBitmapArr, evBitmapLength * sizeof(evBitmapArr[0]));
    }

    if (PRV_CPSS_DRV_TX_BUFF_QUEUE_EVENT_E == hndlEvPtr->evType)
    {
        /* see if there is a TxBufferQueue event waiting in the FIFO for the app */
        if (NULL != ((PRV_CPSS_TX_BUF_QUEUE_FIFO_STC *)hndlEvPtr->extDataPtr)->headPtr)
        {
            /* at least one TxBufferQueue in FIFO */
            if (NULL != evBitmapArr)
            {
                evBitmapArr[evReqPtr->uniEvCause >> 5] |=
                                            (1 << (evReqPtr->uniEvCause & 0x1F));
            }
            evCount = 1;
        }
    }
    else
    {
        if (NULL == evBitmapArr)
        {
            /* count the waiting events only */
            while (evReqPtr)
            {
                if (evReqPtr->evNodeList != NULL)
                {
                    evCount++;
                }
                /* move to the next binded event */
                evReqPtr = evReqPtr->nextPtr;
            }
        }
        else
        {
            /* update the bitmap for all user binded events and count them */
            while (evReqPtr)
            {
                if (evReqPtr->evNodeList != NULL &&
                    (evReqPtr->uniEvCause >> 5) < evBitmapLength)
                {
                    evBitmapArr[evReqPtr->uniEvCause >> 5] |=
                                            (1 << (evReqPtr->uniEvCause & 0x1F));
                    evCount++;
                }
                /* move to the next binded event */
                evReqPtr = evReqPtr->nextPtr;
            }
        }
    }

    return evCount;
}


/**
* @internal prvCpssDrvEvReqQRemoveDev function
* @endinternal
*
* @brief   This function is called upon Hot removal of a device, inorder to remove
*         the interrupt nodes belonging to this device from the interrupts queues.
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvEvReqQRemoveDev
(
    IN  GT_U8           devNum
)
{
    PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC       *evQueuePtr;        /* The event queue pointer      */
    PRV_CPSS_DRV_EV_REQ_NODE_STC         *evNodePtr;         /* The event node pointer       */
    PRV_CPSS_DRV_EV_REQ_NODE_STC         *lastEvNodePtr;     /* The last event in queue      */
    PRV_CPSS_DRV_EV_REQ_NODE_STC         *nextEvNodePtr;     /* The next event node in queue */
    GT_U32              timeOut;            /* The timeout to wait for app  */
    GT_U32              i;                  /* Iterator                     */

    PRV_CPSS_INT_SCAN_LOCK();

    for (i = 0; i < prvCpssDrvComIntEvReqQueuesDb.uniEvQArrSize; i++)
    {
        if (NULL == prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[i].evNodeList)
        {
            continue;
        }

        evQueuePtr = &(prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[i]);

        /* set the first and last events in queue */
        nextEvNodePtr = NULL;
        evNodePtr     = evQueuePtr->evNodeList;
        lastEvNodePtr = evQueuePtr->evNodeList->prevPtr;

        /* set intStatus to pending only for requested event */
        while (evQueuePtr->evNodeList != NULL)
        {
            if (evNodePtr->devNum == devNum)
            {
                /* Set the interrupt to pending state   */
                evNodePtr->intStatus = PRV_CPSS_DRV_EV_DRVN_INT_STOPPED_E;

                /* Wait until the interrupt is handled  */
                timeOut = 1000;

                while (evNodePtr->intRecStatus != PRV_CPSS_DRV_EV_DRVN_INT_IDLE_E)
                {
                    cpssOsTimerWkAfter(1);

                    if (--timeOut == 0)
                    {
                        PRV_CPSS_INT_SCAN_UNLOCK();
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                }

                /* Remove the int node from the interrupt queue          */
                /* The node to be deleted is the first node in the list. */
                if(evNodePtr == evQueuePtr->evNodeList)
                {
                    /* one element in list */
                    if (evNodePtr->nextPtr == evNodePtr)
                    {
                        evQueuePtr->evNodeList = NULL;
                    }
                    else
                    {
                        evQueuePtr->evNodeList          = evNodePtr->nextPtr;
                        evNodePtr->prevPtr->nextPtr     = evQueuePtr->evNodeList;
                        evQueuePtr->evNodeList->prevPtr = evNodePtr->prevPtr;
                    }
                }
                else
                {
                    /* the element is not the first */
                    evNodePtr->prevPtr->nextPtr = evNodePtr->nextPtr;
                    evNodePtr->nextPtr->prevPtr = evNodePtr->prevPtr;
                }

                /* save the deleted node pointer */
                nextEvNodePtr            = evNodePtr->nextPtr;
                evNodePtr->intRecStatus  = PRV_CPSS_DRV_EV_DRVN_INT_IDLE_E;
                evNodePtr->intStatus     = PRV_CPSS_DRV_EV_DRVN_INT_STOPPED_E;
                evNodePtr->nextPtr       = NULL;
                evNodePtr->prevPtr       = NULL;
            }
            else
            {
                /* goto next event */
                nextEvNodePtr            = evNodePtr->nextPtr;
            }

            if (lastEvNodePtr == evNodePtr)
            {
                /* this is the last element in list */
                break;
            }
            else
            {
                /* proceed to the next element in list */
                evNodePtr = nextEvNodePtr;
            }
        }
    }

    PRV_CPSS_INT_SCAN_UNLOCK();

    return GT_OK;
}



/**
* @internal evReqQRcvedEvRemoveOne function
* @endinternal
*
* @brief   This routine removes one user binded event that is active and record
*         status is Idle Ready. All removed events are unmasked.
* @param[in] uniEvCause               - The event number.
*/
static GT_STATUS evReqQRcvedEvRemoveOne
(
    IN GT_U32           uniEvCause
)
{
    PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC       *evQueuePtr;        /* The event queue pointer      */
    PRV_CPSS_DRV_EV_REQ_NODE_STC         *evNodePtr;         /* The event node pointer       */
    PRV_CPSS_DRV_EV_REQ_NODE_STC         *lastEvNodePtr;     /* The last event in queue      */
    PRV_CPSS_DRV_EV_REQ_NODE_STC         *nextEvNodePtr;     /* The next event node in queue */

    evQueuePtr = &(prvCpssDrvComIntEvReqQueuesDb.uniEvQArr[uniEvCause]);

    evNodePtr     = evQueuePtr->evNodeList;
    if (evNodePtr == NULL)
    {
        return GT_OK;
    }
    lastEvNodePtr = evQueuePtr->evNodeList->prevPtr;
    nextEvNodePtr = NULL;

    while (1)
    {
        /* If an interrupt was received while it was masked,    */
        /* re-handle this interrupt and dont remove it.         */
        if (PRV_CPSS_DRV_EV_DRVN_INT_MASKED_E == evNodePtr->intRecStatus ||
            PRV_CPSS_DRV_EV_DRVN_INT_RCVD_E   == evNodePtr->intRecStatus)
        {
            if (NULL == evQueuePtr->evNodeList || lastEvNodePtr == evNodePtr)
            {
                /* this is last event in a queue */
                break;
            }
            evNodePtr = evNodePtr->nextPtr;
            continue;
        }

        /* if we reached here, we need to remove node from list and unmask event */

        /* The node to be deleted is the first node in the list. */
        if(evNodePtr == evQueuePtr->evNodeList)
        {
            /* one element in list */
            if (evNodePtr->nextPtr == evNodePtr)
            {
                evQueuePtr->evNodeList = NULL;
            }
            else
            {
                evQueuePtr->evNodeList          = evNodePtr->nextPtr;
                evNodePtr->prevPtr->nextPtr     = evQueuePtr->evNodeList;
                evQueuePtr->evNodeList->prevPtr = evNodePtr->prevPtr;
            }
        }
        else
        {
            /* the element is not the first */
            evNodePtr->prevPtr->nextPtr = evNodePtr->nextPtr;
            evNodePtr->nextPtr->prevPtr = evNodePtr->prevPtr;
        }

        /* save next node of the deleted node pointer */
        nextEvNodePtr            = evNodePtr->nextPtr;

        /* update fields of deleted event */
        evNodePtr->prevPtr       = NULL;
        evNodePtr->nextPtr       = NULL;
        evNodePtr->intRecStatus  = PRV_CPSS_DRV_EV_DRVN_INT_IDLE_E;

        /* unmask event if it not stopped */
        if (evNodePtr->intStatus != PRV_CPSS_DRV_EV_DRVN_INT_STOPPED_E)
        {
            evNodePtr->intMaskSetFptr(evNodePtr, CPSS_EVENT_UNMASK_E);
        }

        if (NULL == evQueuePtr->evNodeList || lastEvNodePtr == evNodePtr)
        {
            /* this is the last element in list */
            break;
        }
        else
        {
            /* proceed to the next element in list */
            evNodePtr = nextEvNodePtr;
        }
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC       *hndlEvPtr;    /* handle event list                 */


    hndlEvPtr   = ((PRV_CPSS_DRV_EVENT_HNDL_STC*)hndl)->evListPtr;

    PRV_CPSS_INT_SCAN_LOCK();

    while (hndlEvPtr)
    {
        if (hndlEvPtr->evNodeList != NULL)
        {
            evReqQRcvedEvRemoveOne(hndlEvPtr->uniEvCause);
        }
        hndlEvPtr = hndlEvPtr->nextPtr;
    }

    PRV_CPSS_INT_SCAN_UNLOCK();
}


/*******************************************************************************
* prvCpssDrvEvReqUniEvMaskSet
*
* DESCRIPTION:
*       This routine mask/unmask the selected unified event.
*
* INPUTS:
*       intNodesPool    - The event node array for the device.
*       intNodesPoolSize- The number of entries in the intNodesPool.
*       uniEvent        - The unified event to be unmasked.
*       evExtData - The additional data (port num / priority
*                     queue number / other ) the event was received upon.
*                   may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                   to indicate 'ALL interrupts' that relate to this unified
*                   event
*       operation  - the operation : mask / unmask
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK - on success
*       GT_NOT_FOUND - the unified event or the evExtData within the unified
*                      event are not found in this device interrupts
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*******************************************************************************/
#ifdef CHX_FAMILY
extern GT_STATUS prvCpssDxChPortInterruptDisambiguation
(
    IN  GT_U8     devNum,
    IN  GT_U32    portNum,
    OUT GT_BOOL   *extendedModePtr,
    OUT GT_BOOL   *isSupportedPtr
);
#endif
GT_STATUS prvCpssPortManagerIntMaskAdditionalConfiguration
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN CPSS_EVENT_MASK_SET_ENT  operation
);
GT_STATUS prvCpssPortManagerInterruptAdditionals
(
    IN  GT_U8    devNum,
    IN  GT_U32   macPortNum,
    OUT GT_U32   *arrPtr,
    OUT GT_U8    *arrSize
);

/* check if event is CPU SDMA port 'per queue' event */
GT_BOOL prvCpssDrvEventIsCpuSdmaPortPerQueue(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent
)
{
    devNum = devNum;

    switch(uniEvent)
    {
        case CPSS_PP_TX_BUFFER_QUEUE_E          :
        case CPSS_PP_TX_ERR_QUEUE_E             :
        case CPSS_PP_TX_END_E                   :
            /* those events hold 'extData' with 'global queueId' that need convert */
            return GT_TRUE;


        case CPSS_PP_RX_BUFFER_QUEUE0_E         :
        case CPSS_PP_RX_BUFFER_QUEUE1_E         :
        case CPSS_PP_RX_BUFFER_QUEUE2_E         :
        case CPSS_PP_RX_BUFFER_QUEUE3_E         :
        case CPSS_PP_RX_BUFFER_QUEUE4_E         :
        case CPSS_PP_RX_BUFFER_QUEUE5_E         :
        case CPSS_PP_RX_BUFFER_QUEUE6_E         :
        case CPSS_PP_RX_BUFFER_QUEUE7_E         :
        case CPSS_PP_RX_ERR_QUEUE0_E            :
        case CPSS_PP_RX_ERR_QUEUE1_E            :
        case CPSS_PP_RX_ERR_QUEUE2_E            :
        case CPSS_PP_RX_ERR_QUEUE3_E            :
        case CPSS_PP_RX_ERR_QUEUE4_E            :
        case CPSS_PP_RX_ERR_QUEUE5_E            :
        case CPSS_PP_RX_ERR_QUEUE6_E            :
        case CPSS_PP_RX_ERR_QUEUE7_E            :
        /* second CPU SDMA - queues 8..15 */
        case CPSS_PP_RX_BUFFER_QUEUE8_E         :
        case CPSS_PP_RX_BUFFER_QUEUE9_E         :
        case CPSS_PP_RX_BUFFER_QUEUE10_E        :
        case CPSS_PP_RX_BUFFER_QUEUE11_E        :
        case CPSS_PP_RX_BUFFER_QUEUE12_E        :
        case CPSS_PP_RX_BUFFER_QUEUE13_E        :
        case CPSS_PP_RX_BUFFER_QUEUE14_E        :
        case CPSS_PP_RX_BUFFER_QUEUE15_E        :
        case CPSS_PP_RX_ERR_QUEUE8_E            :
        case CPSS_PP_RX_ERR_QUEUE9_E            :
        case CPSS_PP_RX_ERR_QUEUE10_E           :
        case CPSS_PP_RX_ERR_QUEUE11_E           :
        case CPSS_PP_RX_ERR_QUEUE12_E           :
        case CPSS_PP_RX_ERR_QUEUE13_E           :
        case CPSS_PP_RX_ERR_QUEUE14_E           :
        case CPSS_PP_RX_ERR_QUEUE15_E           :
        /* third CPU SDMA - queues 16..23 */
        case CPSS_PP_RX_BUFFER_QUEUE16_E        :
        case CPSS_PP_RX_BUFFER_QUEUE17_E        :
        case CPSS_PP_RX_BUFFER_QUEUE18_E        :
        case CPSS_PP_RX_BUFFER_QUEUE19_E        :
        case CPSS_PP_RX_BUFFER_QUEUE20_E        :
        case CPSS_PP_RX_BUFFER_QUEUE21_E        :
        case CPSS_PP_RX_BUFFER_QUEUE22_E        :
        case CPSS_PP_RX_BUFFER_QUEUE23_E        :
        case CPSS_PP_RX_ERR_QUEUE16_E           :
        case CPSS_PP_RX_ERR_QUEUE17_E           :
        case CPSS_PP_RX_ERR_QUEUE18_E           :
        case CPSS_PP_RX_ERR_QUEUE19_E           :
        case CPSS_PP_RX_ERR_QUEUE20_E           :
        case CPSS_PP_RX_ERR_QUEUE21_E           :
        case CPSS_PP_RX_ERR_QUEUE22_E           :
        case CPSS_PP_RX_ERR_QUEUE23_E           :
        /* forth CPU SDMA - queues 24..31 */
        case CPSS_PP_RX_BUFFER_QUEUE24_E        :
        case CPSS_PP_RX_BUFFER_QUEUE25_E        :
        case CPSS_PP_RX_BUFFER_QUEUE26_E        :
        case CPSS_PP_RX_BUFFER_QUEUE27_E        :
        case CPSS_PP_RX_BUFFER_QUEUE28_E        :
        case CPSS_PP_RX_BUFFER_QUEUE29_E        :
        case CPSS_PP_RX_BUFFER_QUEUE30_E        :
        case CPSS_PP_RX_BUFFER_QUEUE31_E        :
        case CPSS_PP_RX_ERR_QUEUE24_E           :
        case CPSS_PP_RX_ERR_QUEUE25_E           :
        case CPSS_PP_RX_ERR_QUEUE26_E           :
        case CPSS_PP_RX_ERR_QUEUE27_E           :
        case CPSS_PP_RX_ERR_QUEUE28_E           :
        case CPSS_PP_RX_ERR_QUEUE29_E           :
        case CPSS_PP_RX_ERR_QUEUE30_E           :
        case CPSS_PP_RX_ERR_QUEUE31_E           :
        /* CPU SDMA [4..15] - queues 32..127 : 'RX_BUFFER' */
        case CPSS_PP_RX_BUFFER_QUEUE32_E        :
        case CPSS_PP_RX_BUFFER_QUEUE33_E        :
        case CPSS_PP_RX_BUFFER_QUEUE34_E        :
        case CPSS_PP_RX_BUFFER_QUEUE35_E        :
        case CPSS_PP_RX_BUFFER_QUEUE36_E        :
        case CPSS_PP_RX_BUFFER_QUEUE37_E        :
        case CPSS_PP_RX_BUFFER_QUEUE38_E        :
        case CPSS_PP_RX_BUFFER_QUEUE39_E        :
        case CPSS_PP_RX_BUFFER_QUEUE40_E        :
        case CPSS_PP_RX_BUFFER_QUEUE41_E        :
        case CPSS_PP_RX_BUFFER_QUEUE42_E        :
        case CPSS_PP_RX_BUFFER_QUEUE43_E        :
        case CPSS_PP_RX_BUFFER_QUEUE44_E        :
        case CPSS_PP_RX_BUFFER_QUEUE45_E        :
        case CPSS_PP_RX_BUFFER_QUEUE46_E        :
        case CPSS_PP_RX_BUFFER_QUEUE47_E        :
        case CPSS_PP_RX_BUFFER_QUEUE48_E        :
        case CPSS_PP_RX_BUFFER_QUEUE49_E        :
        case CPSS_PP_RX_BUFFER_QUEUE50_E        :
        case CPSS_PP_RX_BUFFER_QUEUE51_E        :
        case CPSS_PP_RX_BUFFER_QUEUE52_E        :
        case CPSS_PP_RX_BUFFER_QUEUE53_E        :
        case CPSS_PP_RX_BUFFER_QUEUE54_E        :
        case CPSS_PP_RX_BUFFER_QUEUE55_E        :
        case CPSS_PP_RX_BUFFER_QUEUE56_E        :
        case CPSS_PP_RX_BUFFER_QUEUE57_E        :
        case CPSS_PP_RX_BUFFER_QUEUE58_E        :
        case CPSS_PP_RX_BUFFER_QUEUE59_E        :
        case CPSS_PP_RX_BUFFER_QUEUE60_E        :
        case CPSS_PP_RX_BUFFER_QUEUE61_E        :
        case CPSS_PP_RX_BUFFER_QUEUE62_E        :
        case CPSS_PP_RX_BUFFER_QUEUE63_E        :
        case CPSS_PP_RX_BUFFER_QUEUE64_E        :
        case CPSS_PP_RX_BUFFER_QUEUE65_E        :
        case CPSS_PP_RX_BUFFER_QUEUE66_E        :
        case CPSS_PP_RX_BUFFER_QUEUE67_E        :
        case CPSS_PP_RX_BUFFER_QUEUE68_E        :
        case CPSS_PP_RX_BUFFER_QUEUE69_E        :
        case CPSS_PP_RX_BUFFER_QUEUE70_E        :
        case CPSS_PP_RX_BUFFER_QUEUE71_E        :
        case CPSS_PP_RX_BUFFER_QUEUE72_E        :
        case CPSS_PP_RX_BUFFER_QUEUE73_E        :
        case CPSS_PP_RX_BUFFER_QUEUE74_E        :
        case CPSS_PP_RX_BUFFER_QUEUE75_E        :
        case CPSS_PP_RX_BUFFER_QUEUE76_E        :
        case CPSS_PP_RX_BUFFER_QUEUE77_E        :
        case CPSS_PP_RX_BUFFER_QUEUE78_E        :
        case CPSS_PP_RX_BUFFER_QUEUE79_E        :
        case CPSS_PP_RX_BUFFER_QUEUE80_E        :
        case CPSS_PP_RX_BUFFER_QUEUE81_E        :
        case CPSS_PP_RX_BUFFER_QUEUE82_E        :
        case CPSS_PP_RX_BUFFER_QUEUE83_E        :
        case CPSS_PP_RX_BUFFER_QUEUE84_E        :
        case CPSS_PP_RX_BUFFER_QUEUE85_E        :
        case CPSS_PP_RX_BUFFER_QUEUE86_E        :
        case CPSS_PP_RX_BUFFER_QUEUE87_E        :
        case CPSS_PP_RX_BUFFER_QUEUE88_E        :
        case CPSS_PP_RX_BUFFER_QUEUE89_E        :
        case CPSS_PP_RX_BUFFER_QUEUE90_E        :
        case CPSS_PP_RX_BUFFER_QUEUE91_E        :
        case CPSS_PP_RX_BUFFER_QUEUE92_E        :
        case CPSS_PP_RX_BUFFER_QUEUE93_E        :
        case CPSS_PP_RX_BUFFER_QUEUE94_E        :
        case CPSS_PP_RX_BUFFER_QUEUE95_E        :
        case CPSS_PP_RX_BUFFER_QUEUE96_E        :
        case CPSS_PP_RX_BUFFER_QUEUE97_E        :
        case CPSS_PP_RX_BUFFER_QUEUE98_E        :
        case CPSS_PP_RX_BUFFER_QUEUE99_E        :
        case CPSS_PP_RX_BUFFER_QUEUE100_E       :
        case CPSS_PP_RX_BUFFER_QUEUE101_E       :
        case CPSS_PP_RX_BUFFER_QUEUE102_E       :
        case CPSS_PP_RX_BUFFER_QUEUE103_E       :
        case CPSS_PP_RX_BUFFER_QUEUE104_E       :
        case CPSS_PP_RX_BUFFER_QUEUE105_E       :
        case CPSS_PP_RX_BUFFER_QUEUE106_E       :
        case CPSS_PP_RX_BUFFER_QUEUE107_E       :
        case CPSS_PP_RX_BUFFER_QUEUE108_E       :
        case CPSS_PP_RX_BUFFER_QUEUE109_E       :
        case CPSS_PP_RX_BUFFER_QUEUE110_E       :
        case CPSS_PP_RX_BUFFER_QUEUE111_E       :
        case CPSS_PP_RX_BUFFER_QUEUE112_E       :
        case CPSS_PP_RX_BUFFER_QUEUE113_E       :
        case CPSS_PP_RX_BUFFER_QUEUE114_E       :
        case CPSS_PP_RX_BUFFER_QUEUE115_E       :
        case CPSS_PP_RX_BUFFER_QUEUE116_E       :
        case CPSS_PP_RX_BUFFER_QUEUE117_E       :
        case CPSS_PP_RX_BUFFER_QUEUE118_E       :
        case CPSS_PP_RX_BUFFER_QUEUE119_E       :
        case CPSS_PP_RX_BUFFER_QUEUE120_E       :
        case CPSS_PP_RX_BUFFER_QUEUE121_E       :
        case CPSS_PP_RX_BUFFER_QUEUE122_E       :
        case CPSS_PP_RX_BUFFER_QUEUE123_E       :
        case CPSS_PP_RX_BUFFER_QUEUE124_E       :
        case CPSS_PP_RX_BUFFER_QUEUE125_E       :
        case CPSS_PP_RX_BUFFER_QUEUE126_E       :
        case CPSS_PP_RX_BUFFER_QUEUE127_E       :
        /* CPU SDMA [4..15] - queues 32..127 : 'RX_ERR' */
        case CPSS_PP_RX_ERR_QUEUE32_E           :
        case CPSS_PP_RX_ERR_QUEUE33_E           :
        case CPSS_PP_RX_ERR_QUEUE34_E           :
        case CPSS_PP_RX_ERR_QUEUE35_E           :
        case CPSS_PP_RX_ERR_QUEUE36_E           :
        case CPSS_PP_RX_ERR_QUEUE37_E           :
        case CPSS_PP_RX_ERR_QUEUE38_E           :
        case CPSS_PP_RX_ERR_QUEUE39_E           :
        case CPSS_PP_RX_ERR_QUEUE40_E           :
        case CPSS_PP_RX_ERR_QUEUE41_E           :
        case CPSS_PP_RX_ERR_QUEUE42_E           :
        case CPSS_PP_RX_ERR_QUEUE43_E           :
        case CPSS_PP_RX_ERR_QUEUE44_E           :
        case CPSS_PP_RX_ERR_QUEUE45_E           :
        case CPSS_PP_RX_ERR_QUEUE46_E           :
        case CPSS_PP_RX_ERR_QUEUE47_E           :
        case CPSS_PP_RX_ERR_QUEUE48_E           :
        case CPSS_PP_RX_ERR_QUEUE49_E           :
        case CPSS_PP_RX_ERR_QUEUE50_E           :
        case CPSS_PP_RX_ERR_QUEUE51_E           :
        case CPSS_PP_RX_ERR_QUEUE52_E           :
        case CPSS_PP_RX_ERR_QUEUE53_E           :
        case CPSS_PP_RX_ERR_QUEUE54_E           :
        case CPSS_PP_RX_ERR_QUEUE55_E           :
        case CPSS_PP_RX_ERR_QUEUE56_E           :
        case CPSS_PP_RX_ERR_QUEUE57_E           :
        case CPSS_PP_RX_ERR_QUEUE58_E           :
        case CPSS_PP_RX_ERR_QUEUE59_E           :
        case CPSS_PP_RX_ERR_QUEUE60_E           :
        case CPSS_PP_RX_ERR_QUEUE61_E           :
        case CPSS_PP_RX_ERR_QUEUE62_E           :
        case CPSS_PP_RX_ERR_QUEUE63_E           :
        case CPSS_PP_RX_ERR_QUEUE64_E           :
        case CPSS_PP_RX_ERR_QUEUE65_E           :
        case CPSS_PP_RX_ERR_QUEUE66_E           :
        case CPSS_PP_RX_ERR_QUEUE67_E           :
        case CPSS_PP_RX_ERR_QUEUE68_E           :
        case CPSS_PP_RX_ERR_QUEUE69_E           :
        case CPSS_PP_RX_ERR_QUEUE70_E           :
        case CPSS_PP_RX_ERR_QUEUE71_E           :
        case CPSS_PP_RX_ERR_QUEUE72_E           :
        case CPSS_PP_RX_ERR_QUEUE73_E           :
        case CPSS_PP_RX_ERR_QUEUE74_E           :
        case CPSS_PP_RX_ERR_QUEUE75_E           :
        case CPSS_PP_RX_ERR_QUEUE76_E           :
        case CPSS_PP_RX_ERR_QUEUE77_E           :
        case CPSS_PP_RX_ERR_QUEUE78_E           :
        case CPSS_PP_RX_ERR_QUEUE79_E           :
        case CPSS_PP_RX_ERR_QUEUE80_E           :
        case CPSS_PP_RX_ERR_QUEUE81_E           :
        case CPSS_PP_RX_ERR_QUEUE82_E           :
        case CPSS_PP_RX_ERR_QUEUE83_E           :
        case CPSS_PP_RX_ERR_QUEUE84_E           :
        case CPSS_PP_RX_ERR_QUEUE85_E           :
        case CPSS_PP_RX_ERR_QUEUE86_E           :
        case CPSS_PP_RX_ERR_QUEUE87_E           :
        case CPSS_PP_RX_ERR_QUEUE88_E           :
        case CPSS_PP_RX_ERR_QUEUE89_E           :
        case CPSS_PP_RX_ERR_QUEUE90_E           :
        case CPSS_PP_RX_ERR_QUEUE91_E           :
        case CPSS_PP_RX_ERR_QUEUE92_E           :
        case CPSS_PP_RX_ERR_QUEUE93_E           :
        case CPSS_PP_RX_ERR_QUEUE94_E           :
        case CPSS_PP_RX_ERR_QUEUE95_E           :
        case CPSS_PP_RX_ERR_QUEUE96_E           :
        case CPSS_PP_RX_ERR_QUEUE97_E           :
        case CPSS_PP_RX_ERR_QUEUE98_E           :
        case CPSS_PP_RX_ERR_QUEUE99_E           :
        case CPSS_PP_RX_ERR_QUEUE100_E          :
        case CPSS_PP_RX_ERR_QUEUE101_E          :
        case CPSS_PP_RX_ERR_QUEUE102_E          :
        case CPSS_PP_RX_ERR_QUEUE103_E          :
        case CPSS_PP_RX_ERR_QUEUE104_E          :
        case CPSS_PP_RX_ERR_QUEUE105_E          :
        case CPSS_PP_RX_ERR_QUEUE106_E          :
        case CPSS_PP_RX_ERR_QUEUE107_E          :
        case CPSS_PP_RX_ERR_QUEUE108_E          :
        case CPSS_PP_RX_ERR_QUEUE109_E          :
        case CPSS_PP_RX_ERR_QUEUE110_E          :
        case CPSS_PP_RX_ERR_QUEUE111_E          :
        case CPSS_PP_RX_ERR_QUEUE112_E          :
        case CPSS_PP_RX_ERR_QUEUE113_E          :
        case CPSS_PP_RX_ERR_QUEUE114_E          :
        case CPSS_PP_RX_ERR_QUEUE115_E          :
        case CPSS_PP_RX_ERR_QUEUE116_E          :
        case CPSS_PP_RX_ERR_QUEUE117_E          :
        case CPSS_PP_RX_ERR_QUEUE118_E          :
        case CPSS_PP_RX_ERR_QUEUE119_E          :
        case CPSS_PP_RX_ERR_QUEUE120_E          :
        case CPSS_PP_RX_ERR_QUEUE121_E          :
        case CPSS_PP_RX_ERR_QUEUE122_E          :
        case CPSS_PP_RX_ERR_QUEUE123_E          :
        case CPSS_PP_RX_ERR_QUEUE124_E          :
        case CPSS_PP_RX_ERR_QUEUE125_E          :
        case CPSS_PP_RX_ERR_QUEUE126_E          :
        case CPSS_PP_RX_ERR_QUEUE127_E          :
            return GT_TRUE;
        default:
            break;
    }

    return GT_FALSE;
}


GT_STATUS prvCpssDrvEvReqUniEvMaskSet
(
    IN GT_U8                devNum,
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC             intNodesPool[],
    IN GT_U32               intNodesPoolSize,
    IN GT_U32               uniEvent,
    IN GT_U32               evExtData,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC  *nodePoolArr;   /* The event node array   */
    GT_U32                  event;          /* The event index              */
    GT_STATUS               rc;             /* The Return Code              */
    GT_U32                  found;    /* indication whether the event was found */
                                      /* also support event with specific extra data*/
    GT_U32                  oldEvExtData; /* extended data before convertion */
    GT_U32                  evExtDataArr[4];
    GT_U8                   evExtDataArrSize;
    GT_U32                  mismatchExtDataArr;
    GT_U32                  i;

#ifdef CHX_FAMILY
    GT_BOOL                 extendedMode;
    GT_BOOL                 isSupported;
#endif

    evExtDataArrSize = 0;
    mismatchExtDataArr = 0;
    found = 0;
    nodePoolArr = intNodesPool;
    oldEvExtData = evExtData;

    if (prvCpssEventExtDataConvertFunc)
    {
        /* invoke the callback function to convert event's extended data */
        rc = (prvCpssEventExtDataConvertFunc)(devNum, uniEvent,
                                              PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E,
                                              &evExtData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* for certain unified events, additional MACs\PCSs should be configured for mask\unmask */
    if ( (uniEvent == CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E ||
            uniEvent == CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E ||
            uniEvent == CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E) &&
         evExtData != PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS )
    {
        evExtDataArrSize = 0;
        /* failure on port manager extended behavior should not affect
           main logic, thus, no need to check return code */
        prvCpssPortManagerInterruptAdditionals(devNum, evExtData /*portMacNum*/,
                                &evExtDataArr[0], &evExtDataArrSize);
    }

    for (event = 0; event < intNodesPoolSize; event++)
    {
        if (nodePoolArr[event].uniEvCause != uniEvent)
        {
            continue;
        }

        /* for certain unified events, additional MACs\PCSs should be configured for mask\unmask */
        if ( evExtDataArrSize > 0 )
        {
            /* if the iterated event's evExtData is none of the additional, then continue,
               as oposed to other unified events when event's evExtData's should be compared
               to a single evExtData */
            mismatchExtDataArr = 0;
            for (i=0; i<evExtDataArrSize; i++)
            {
                if(nodePoolArr[event].uniEvExt != evExtDataArr[i])
                {
                    mismatchExtDataArr++;
                }
            }
            if (mismatchExtDataArr == evExtDataArrSize)
            {
                continue;
            }
        }
        else if(evExtData != PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS)
        {
            if(nodePoolArr[event].uniEvExt != evExtData)
            {
                /* we need to mask/unmask only the specific interrupts and not all
                   interrupts that connected to the unified event */
                continue;
            }
#ifdef CHX_FAMILY
            switch(uniEvent)
            {
                case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
                case CPSS_PP_PORT_AN_COMPLETED_E:
                case CPSS_PP_PORT_RX_FIFO_OVERRUN_E:
                case CPSS_PP_PORT_TX_FIFO_UNDERRUN_E:
                case CPSS_PP_PORT_TX_FIFO_OVERRUN_E:
                case CPSS_PP_PORT_TX_UNDERRUN_E:
                case CPSS_PP_PORT_ADDRESS_OUT_OF_RANGE_E:
                case CPSS_PP_PORT_PRBS_ERROR_E:
                case CPSS_PP_PORT_SYNC_STATUS_CHANGED_E:
                case CPSS_PP_PORT_ILLEGAL_SEQUENCE_E:
                case CPSS_PP_PORT_FAULT_TYPE_CHANGE_E:
                case CPSS_PP_PORT_FC_STATUS_CHANGED_E:
                case CPSS_PP_PORT_NO_BUFF_PACKET_DROP_E:
                case CPSS_PP_PORT_COUNT_COPY_DONE_E:
                case CPSS_PP_PORT_COUNT_EXPIRED_E:
                case CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E:
                    /* mask and unmask operations should be done on the proper port,
                       if port is in extended mode, then operation should be done on
                       the port that provides the extended abilities */
                    rc = prvCpssDxChPortInterruptDisambiguation(nodePoolArr[event].devNum,
                                                                evExtData, &extendedMode, &isSupported);
                    if(rc != GT_BAD_STATE)
                    {
                        if(GT_TRUE == isSupported)
                        {
                            /* if extended mode is true, then second instance should be operated,
                               if extended mode is false, the first instance should be operated.
                               This is based on the assumption that when building the interrupt
                               tree, the "real" port are prior to the extended ports.*/
                            if( ((0 == found%2) && extendedMode==GT_TRUE) ||
                                ((0 != found%2) && extendedMode==GT_FALSE))
                            {
                                found ++;
                                continue;
                            }
                        }
                    }

                    if((rc != GT_OK) && (rc != GT_BAD_STATE))
                    {
                        return rc;
                    }

                    break;
                default:
                    break;
            }
#endif
        }

        /* we found at least one interrupt associated with the event and extra info */
        found ++;

        /* mask/unmask the HW interrupt */
        rc = nodePoolArr[event].intMaskSetFptr(&nodePoolArr[event], operation);

        if (rc != GT_OK)
        {
            return rc;
        }

        if(CPSS_EVENT_MASK_E == operation)
        {
            nodePoolArr[event].intStatus = PRV_CPSS_DRV_EV_DRVN_INT_STOPPED_E;
        }
        else
        {
            nodePoolArr[event].intStatus = PRV_CPSS_DRV_EV_DRVN_INT_ACTIVE_E;
        }

        /* port spicific events may need additional operations */
        if ( (uniEvent == CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E ||
             uniEvent == CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E ||
             uniEvent == CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E) )
        {
            if(evExtData != PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS)
            {
                /* falure on port manager extended behaviour should not affect
                   main logic, thus, no need to check return code */
                prvCpssPortManagerIntMaskAdditionalConfiguration(devNum, oldEvExtData/*physicalPort*/, operation);
            }
        }
    }

    if(found == 0)
    {
        /* the unified event or the evExtData within the unified event are not
           found in this device interrupts */
        return /* this is not error for the ERROR LOG */GT_NOT_FOUND;
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC  *nodePoolArr;   /* The event node array   */
    GT_U32                  event;          /* The event index              */
    GT_STATUS               rc;             /* The Return Code              */
    GT_U32                  found = 0;/* indication whether the event was found */
                                      /* also support event with specific extra data*/
#ifdef CHX_FAMILY
    GT_BOOL                 extendedMode; /* indicated about extended mode of port */
    GT_BOOL                 isSupported; /* indication of support of extended ports */
#endif

    if(NULL == maskedPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    nodePoolArr = intNodesPool;

    if (prvCpssEventExtDataConvertFunc)
    {
        /* invoke the callback function to convert event's extended data */
        rc = (prvCpssEventExtDataConvertFunc)(devNum, uniEvent,
                                              PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E,
                                              &evExtData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for (event = 0; event < intNodesPoolSize; event++)
    {
        if (nodePoolArr[event].uniEvCause != uniEvent)
        {
            continue;
        }

        if(evExtData != PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS)
        {
            if(nodePoolArr[event].uniEvExt != evExtData)
            {
                /* we need to mask/unmask only the specific interrupts and not all
                   interrupts that connected to the unified event */
                continue;
            }
#ifdef CHX_FAMILY
            switch(uniEvent)
            {
                case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
                case CPSS_PP_PORT_AN_COMPLETED_E:
                case CPSS_PP_PORT_RX_FIFO_OVERRUN_E:
                case CPSS_PP_PORT_TX_FIFO_UNDERRUN_E:
                case CPSS_PP_PORT_TX_FIFO_OVERRUN_E:
                case CPSS_PP_PORT_TX_UNDERRUN_E:
                case CPSS_PP_PORT_ADDRESS_OUT_OF_RANGE_E:
                case CPSS_PP_PORT_PRBS_ERROR_E:
                case CPSS_PP_PORT_SYNC_STATUS_CHANGED_E:
                case CPSS_PP_PORT_ILLEGAL_SEQUENCE_E:
                case CPSS_PP_PORT_FAULT_TYPE_CHANGE_E:
                case CPSS_PP_PORT_FC_STATUS_CHANGED_E:
                case CPSS_PP_PORT_NO_BUFF_PACKET_DROP_E:
                case CPSS_PP_PORT_COUNT_COPY_DONE_E:
                case CPSS_PP_PORT_COUNT_EXPIRED_E:
                case CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E:
                    /* mask and unmask operations should be done on the proper port,
                       if port is in extended mode, then operation should be done on
                       the port that provides the extended abilities */
                    rc = prvCpssDxChPortInterruptDisambiguation(nodePoolArr[event].devNum,
                                                                evExtData, &extendedMode, &isSupported);
                    if(rc != GT_BAD_STATE)
                    {
                        if(GT_TRUE == isSupported)
                        {
                            /* if extended mode is true, then second instance should be operated,
                               if extended mode is false, the first instance should be operated.
                               This is based on the assumption that when building the interrupt
                               tree, the "real" port are prior to the extended ports.*/
                            if( ((0 == found%2) && extendedMode==GT_TRUE) ||
                                ((0 != found%2) && extendedMode==GT_FALSE))
                            {
                                found ++;
                                continue;
                            }
                        }
                    }

                    if((rc != GT_OK) && (rc != GT_BAD_STATE))
                    {
                        return rc;
                    }

                    break;
                default:
                    break;
            }
#endif
        }
        /* we found at least one interrupt associated with the event and extra info */
        found ++;

#ifdef CHX_FAMILY
        rc = prvCpssDrvInterruptMaskGet(&nodePoolArr[event], maskedPtr);

        if (rc != GT_OK)
        {
            return rc;
        }
#else
        rc = GT_NOT_SUPPORTED;
#endif
    }

    if(found == 0)
    {
        /* the unified event or the evExtData within the unified event are not
           found in this device interrupts */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "The uniEvent[%d] with evExtData[0x%x] not supported by the device",
            uniEvent,evExtData);
    }

    return GT_OK;
}

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
)
{
    if(hndlEvPtr->hndlBindType == PRV_CPSS_DRV_HANDLE_BIND_USE_SIGNAL_TYPE_E)
    {
        /* signal the user process if not already signalled by this routine */
        if(GT_FALSE == PRV_CPSS_DRV_HANDEL_SEM_SIGNAL_MAC(hndlEvPtr))
        {
             PRV_CPSS_DRV_HANDEL_SEM_SIGNAL_MAC(hndlEvPtr) = GT_TRUE;
            /* and signal the user process */
            cpssOsSigSemSignal((CPSS_OS_SIG_SEM)PRV_CPSS_DRV_HANDEL_SEM_MAC(hndlEvPtr));
        }
    }
    else
    {
        /* call the application's CB function */
        PRV_CPSS_DRV_HANDEL_APPL_CB_MAC(hndlEvPtr)(  /* CB */
            PRV_CPSS_DRV_HANDEL_MAC(hndlEvPtr),      /* handle */
            PRV_CPSS_DRV_HANDEL_APPL_COOKIE_MAC(hndlEvPtr));/* cookie */
    }

    return;
}

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
)
{
    GT_STATUS rc;

    FREE_PTR_MAC(prvCpssDrvComIntEvReqQueuesDb.uniEvQArr);

    cpssOsMemSet(&prvCpssDrvComIntEvReqQueuesDb,0,sizeof(prvCpssDrvComIntEvReqQueuesDb));

    rc = prvCpssGenEventRequestsDbRelease();

    return rc;
}
