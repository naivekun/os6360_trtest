/**
********************************************************************************
* @file lmsgqLib.c
*
* @brief pthread implementation of message queues
*
* @version   3
********************************************************************************
*/
/*******************************************************************************
* lmsgqLib.c
*
* DESCRIPTION:
*       pthread implementation of message queues
*
* DEPENDENCIES:
*       CPU independed , and posix threads implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#define _GNU_SOURCE
#include <gtOs/gtOsMsgQ.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

#include <gtOs/osObjIdLib.h>
#include <gtOs/gtOsSharedData.h>
#include "mainOsPthread.h"

/************* Defines ********************************************************/

#ifndef CPSS_CLOCK
#define CPSS_CLOCK CLOCK_REALTIME
#endif

/* enable statistic collection for userspace mutexes */
#undef OS_MESSAGE_QUEUES_STAT

/************ Internal Typedefs ***********************************************/
typedef struct _osMsgQSTC
{
    OS_OBJECT_HEADER_STC    header;
    pthread_mutex_t         mtx;
    pthread_cond_t          rxSem;
    pthread_cond_t          txSem;
    int                     maxMsgs;
    int                     maxMsgSize;
    int                     messages;
    char                    *buffer;
    int                     head;
    int                     tail;
    int                     waitRx;
    int                     waitTx;
} OS_MSGQ_STC;

static OS_OBJECT_LIST_STC messageQueues __SHARED_DATA_MAINOS = {
    NULL,                               /* list */
    0,                                  /* allocated */
    64,                                 /* allocChunk */
    sizeof(OS_MSGQ_STC),                /* objSize */
    V2L_ts_malloc,                      /* allocFuncPtr */
    V2L_ts_free                         /* freeFuncPtr */
};
#ifndef SHARED_MEMORY
static pthread_mutex_t messageQueues_mtx = PTHREAD_MUTEX_INITIALIZER;
#else
static int initialized __SHARED_DATA_MAINOS = 0;
static pthread_mutex_t messageQueues_mtx __SHARED_DATA_MAINOS;
#endif

/************ Implementation defines ******************************************/
#define MSGQ_CHECK() \
    OS_MSGQ_STC   *q; \
    if (msgqId < 1 || msgqId >= messageQueues.allocated) \
        return GT_FAIL; \
    q = (OS_MSGQ_STC*)(messageQueues.list[msgqId]); \
    if (!q || !q->header.type) \
        return GT_FAIL;

/************ Public Functions ************************************************/

/**
* @internal V2L_lmsgQInit function
* @endinternal
*
* @brief   Initialize message queues
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*
* @note This function called from osStartEngine()
*
*/
int V2L_lmsgQInit(void)
{
#ifdef SHARED_MEMORY
    if (!initialized)
    {
        CREATE_MTX(&messageQueues_mtx);
        initialized = 1;
    }
#endif
    return 0;
}

/**
* @internal osMsgQCreate function
* @endinternal
*
* @brief   Create and initialize a message queue.
*
* @param[in] name                     - message queue name
* @param[in] maxMsgs                  - max messages in queue
* @param[in] maxMsgSize               - max length of single message
*
* @param[out] msgqId                   - message queue id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMsgQCreate
(
    IN  const char    *name,
    IN  GT_U32        maxMsgs,
    IN  GT_U32        maxMsgSize,
    OUT GT_MSGQ_ID    *msgqId
)
{
    int qnum;
    OS_OBJECT_HEADER_STC *h;
    OS_MSGQ_STC *q;

    pthread_mutex_lock(&messageQueues_mtx);

    qnum = osObjLibGetNewObject(&messageQueues, 1, name, &h);
    q = (OS_MSGQ_STC*)h;
    if (qnum <= 0)
    {
ret_fail:
        pthread_mutex_unlock(&messageQueues_mtx);
        return GT_FAIL;
    }


    /* align max message size by 4 bytes */
    maxMsgSize = (maxMsgSize+3) & ~3;
    q->buffer = (char*)V2L_ts_malloc((maxMsgSize + sizeof(int))*maxMsgs);
    if (q->buffer == NULL)
    {
        q->header.type = 0;
        goto ret_fail;
    }
    CREATE_MTX(&q->mtx);
    CREATE_COND(&q->rxSem);
    CREATE_COND(&q->txSem);

    q->maxMsgs = maxMsgs;
    q->maxMsgSize = maxMsgSize;
    pthread_mutex_unlock(&messageQueues_mtx);

    *msgqId = (GT_MSGQ_ID)qnum;
    return GT_OK;
}

/**
* @internal osMsgQDelete function
* @endinternal
*
* @brief   Delete message queue
*
* @param[in] msgqId                   - message queue Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMsgQDelete
(
    IN GT_MSGQ_ID msgqId
)
{
    int timeOut;

    MSGQ_CHECK();

    pthread_mutex_lock(&q->mtx);
    q->header.type = 2; /* deleting */
    pthread_mutex_unlock(&q->mtx);

    for (timeOut = 1000; q->waitRx && timeOut; timeOut--)
    {
        pthread_cond_broadcast(&q->rxSem);
        sched_yield();
    }
    for (timeOut = 1000; q->waitTx && timeOut; timeOut--)
    {
        pthread_cond_broadcast(&q->txSem);
        sched_yield();
    }
    V2L_ts_free(q->buffer);
    pthread_mutex_destroy(&q->mtx);
    pthread_cond_destroy(&q->rxSem);
    pthread_cond_destroy(&q->txSem);

    q->header.type = 0;

    return GT_OK;
}

/**
* @internal osMsgQSend function
* @endinternal
*
* @brief   Send message to queue
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
* @param[in] message                  -  data pointer
* @param[in] messageSize              - message size
* @param[in] timeOut                  - time out in miliseconds or
*                                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS osMsgQSend
(
    IN GT_MSGQ_ID   msgqId,
    IN GT_PTR       message,
    IN GT_U32       messageSize,
    IN GT_U32       timeOut
)
{
    char    *msg;
    struct timespec ts;

    MSGQ_CHECK();

    if (timeOut != OS_MSGQ_NO_WAIT && timeOut != OS_MSGQ_WAIT_FOREVER)
    {
        struct timespec delta;
        milliseconds2timespec(timeOut, &delta);
        clock_gettime(CPSS_CLOCK, &ts);
        timespec_add(&ts, &delta);
    }

    pthread_mutex_lock(&q->mtx);
    while (q->messages == q->maxMsgs)
    {
        /* queue full */
        if (timeOut == OS_MSGQ_NO_WAIT)
        {
            pthread_mutex_unlock(&q->mtx);
            return GT_FAIL; /* ??? GT_TIMEOUT */
        }
        q->waitTx++;
        if (timeOut == OS_MSGQ_WAIT_FOREVER)
        {
            pthread_cond_wait(&q->txSem, &q->mtx);
        }
        else
        {
            if (pthread_cond_timedwait(&q->txSem, &q->mtx, &ts) != 0)
            {
                /* timeout */
                pthread_mutex_unlock(&q->mtx);
                return GT_TIMEOUT;
            }
        }
        q->waitTx--;
        if (q->header.type != 1)
        {
            /* deleting */
            pthread_mutex_unlock(&q->mtx);
            return GT_FAIL;
        }
    }

    /* put message */
    msg = q->buffer + q->head * (q->maxMsgSize + sizeof(GT_U32));
    if (messageSize > q->maxMsgSize)
        messageSize = q->maxMsgSize;

    *((GT_U32*)msg) = messageSize;
    memcpy(msg+sizeof(GT_U32), message, messageSize);
    q->head++;
    if (q->head >= q->maxMsgs) /* round up */
        q->head = 0;
    q->messages++;

    /* signal to Recv thread if any */
    if (q->waitRx)
    {
        pthread_cond_signal(&q->rxSem);
    }

    pthread_mutex_unlock(&q->mtx);
    return GT_OK;
}

/**
* @internal osMsgQRecv function
* @endinternal
*
* @brief   Receive message from queuee
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
* @param[in,out] messageSize              - size of buffer pointed by message
* @param[in] timeOut                  - time out in miliseconds or
*                                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* @param[out] message                  -  data pointer
* @param[in,out] messageSize              - actual message size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS osMsgQRecv
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_PTR       message,
    INOUT GT_U32       *messageSize,
    IN    GT_U32       timeOut
)
{
    char    *msg;
    GT_U32  msgSize;
    struct timespec ts;

    MSGQ_CHECK();

    if (timeOut != OS_MSGQ_NO_WAIT && timeOut != OS_MSGQ_WAIT_FOREVER)
    {
        struct timespec delta;
        milliseconds2timespec(timeOut, &delta);
        clock_gettime(CPSS_CLOCK, &ts);
        timespec_add(&ts, &delta);
    }

    pthread_mutex_lock(&q->mtx);
    while (q->messages == 0)
    {
        /* queue empty */
        if (timeOut == OS_MSGQ_NO_WAIT)
        {
            pthread_mutex_unlock(&q->mtx);
            return GT_FAIL; /* ??? GT_TIMEOUT */
        }
        q->waitRx++;
        if (timeOut == OS_MSGQ_WAIT_FOREVER)
        {
            pthread_cond_wait(&q->rxSem, &q->mtx);
        }
        else
        {
            if (pthread_cond_timedwait(&q->rxSem, &q->mtx, &ts) != 0)
            {
                /* timeout */
                pthread_mutex_unlock(&q->mtx);
                return GT_TIMEOUT;
            }
        }
        q->waitRx--;
        if (q->header.type != 1)
        {
            /* deleting */
            pthread_mutex_unlock(&q->mtx);
            return GT_FAIL;
        }
    }
    /* get message */
    msg = q->buffer + q->tail * (q->maxMsgSize + sizeof(GT_U32));
    msgSize = *((GT_U32*)msg);
    if (msgSize > *messageSize)
        msgSize = *messageSize;

    memcpy(message, msg+sizeof(GT_U32), msgSize);
    *messageSize = msgSize;
    q->tail++;
    if (q->tail >= q->maxMsgs) /* round up */
        q->tail = 0;
    q->messages--;

    /* signal to Recv thread if any */
    if (q->waitTx)
    {
        pthread_cond_signal(&q->txSem);
    }

    pthread_mutex_unlock(&q->mtx);
    return GT_OK;
}

/**
* @internal osMsgQNumMsgs function
* @endinternal
*
* @brief   Return number of messages pending in queue
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
*
* @param[out] numMessages              - number of messages pending in queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMsgQNumMsgs
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_U32       *numMessages
)
{

    MSGQ_CHECK();

    pthread_mutex_lock(&q->mtx);
    *numMessages = q->messages;
    pthread_mutex_unlock(&q->mtx);

    return GT_OK;
}

