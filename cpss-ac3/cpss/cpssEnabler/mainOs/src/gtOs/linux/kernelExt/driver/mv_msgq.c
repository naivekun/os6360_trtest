/*******************************************************************************
* mv_KervelExtMsgQ.c
*
* DESCRIPTION:
*       Message queues
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: $
********************************************************************************/


/************* Defines ********************************************************/

#define MV_MSGQ_STAT
#include "mv_msgq.h"
#include "mv_tasks.h"

/************ Internal Typedefs ***********************************************/
typedef struct _mvMsgQ
{
    int                     flags;
    char                    name[MV_MSGQ_NAME_LEN+1];
    mv_waitqueue_t          rxWaitQueue;
    mv_waitqueue_t          txWaitQueue;
    int                     maxMsgs;
    int                     maxMsgSize;
    int                     messages;
    char                    *buffer;
    int                     head;
    int                     tail;
    int                     waitRx;
    int                     waitTx;
} mvMsgQSTC;

static mvMsgQSTC        *mvMsgQs = NULL;
static int              mv_num_queues = MV_QUEUES_DEF;

module_param(mv_num_queues, int, S_IRUGO);

#ifdef CONFIG_PROC_FS
/**
* @internal mvKernelExtMsgQ_proc_status_show function
* @endinternal
*
* @brief   proc read data rooutine.
*         Use cat /proc/mvKernelExtMsgQ to show message queue list
*/
static int mvKernelExtMsgQ_proc_status_show(struct seq_file *m, void *v)
{
    int k;

    seq_printf(m, "id msgs waitRx waitTx");
#ifdef MV_MSGQ_STAT
    /*
    seq_printf(m, " tcount gcount wcount");
    */
#endif
    seq_printf(m, " name\n");
    for (k = 1; k < mv_num_queues; k++)
    {
        mvMsgQSTC *q;
        struct mv_task *p;

        if (!mvMsgQs[k].flags)
            continue;
        q = mvMsgQs + k;

        seq_printf(m, "%d %d %d %d",
                k, q->messages, q->waitRx, q->waitTx);
#ifdef MV_MSGQ_STAT
        /*
        seq_printf(m, " %d %d %d", sem->tcount, sem->gcount, sem->wcount);
        */
#endif
        if (q->name[0])
            seq_printf(m, " %s", q->name);
        seq_printf(m, "\n");

        for (p = q->rxWaitQueue.first; p; p = p->wait_next)
            seq_printf(m,  "  rq=%d\n", p->task->pid);
        for (p = q->txWaitQueue.first; p; p = p->wait_next)
            seq_printf(m,  "  tq=%d\n", p->task->pid);
    }

    return 0;
}
static int mvKernelExtMsgQ_proc_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, mvKernelExtMsgQ_proc_status_show, NULL);
}
static const struct file_operations mvKernelExtMsgQ_read_proc_operations = {
	.open		= mvKernelExtMsgQ_proc_status_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif /* CONFIG_PROC_FS */

/**
* @internal mvMsgqInit function
* @endinternal
*
* @brief   Initialize message queues support, create /proc for queues info
*/
int mvMsgqInit(void)
{
    if (mv_num_queues < MV_QUEUES_MIN)
        mv_num_queues = MV_QUEUES_MIN;

    mvMsgQs = (mvMsgQSTC*) kmalloc(
            mv_num_queues * sizeof(mvMsgQSTC), GFP_KERNEL);

    if (mvMsgQs == NULL)
    {
        printk("mvMsgqInit: unable to allocate mvMsgQs array\n");
        return -ENOMEM;
    }

    memset(mvMsgQs, 0, mv_num_queues * sizeof(mvMsgQSTC));

    /* create proc entry */
#ifdef CONFIG_PROC_FS
    proc_create("mvKernelExtMsgQ", S_IRUGO, NULL, &mvKernelExtMsgQ_read_proc_operations);
#endif

    return 0;
}

/**
* @internal mvMsgq_DeleteAll function
* @endinternal
*
* @brief   Destroys all message queues
*         This is safety action which is executed when all tasks closed
*/
void mvMsgq_DeleteAll(void)
{
    int k;

    for (k = 1; k < mv_num_queues; k++)
    {
        if (mvMsgQs[k].flags)
        {
            mv_waitqueue_wake_all(&(mvMsgQs[k].rxWaitQueue));
            mv_waitqueue_wake_all(&(mvMsgQs[k].txWaitQueue));
            mv_waitqueue_cleanup(&(mvMsgQs[k].rxWaitQueue));
            mv_waitqueue_cleanup(&(mvMsgQs[k].txWaitQueue));
        }
        mvMsgQs[k].flags = 0;
    }
}

/**
* @internal mvMsgqCleanup function
* @endinternal
*
* @brief   Perform message queues cleanup actions before module unload
*/
void mvMsgqCleanup(void)
{
    MV_GLOBAL_LOCK();

    if (mvMsgQs)
    {
        mvMsgq_DeleteAll();
        kfree(mvMsgQs);
    }

    mvMsgQs = NULL;

    MV_GLOBAL_UNLOCK();

    remove_proc_entry("mvKernelExtMsgQ", NULL);
}

/**
* @internal mvKernelExt_MsgQCreate function
* @endinternal
*
* @brief   Create a new message queue
*
* @retval Positive value           - queue ID
*                                       -MVKERNELEXT_EINVAL  - invalid parameter passed
*                                       -MVKERNELEXT_ENOMEM  - queue array is full
*/
int mvKernelExt_MsgQCreate(
    const char *name,
    int maxMsgs,
    int maxMsgSize
)
{
    int k;
    mvMsgQSTC *q = NULL;

    MV_GLOBAL_LOCK();

    /* create queue */
    for (k = 1; k < mv_num_queues; k++)
    {
        if (mvMsgQs[k].flags == 0)
            break;
    }
    if (k >= mv_num_queues)
    {
        MV_GLOBAL_UNLOCK();
        return -MVKERNELEXT_ENOMEM;
    }
    q = mvMsgQs + k;

    memset(q, 0, sizeof(*q));
    q->flags = 3;
    MV_GLOBAL_UNLOCK();

    /* align max message size by 4 bytes */
    maxMsgSize = (maxMsgSize+3) & ~3;
    q->maxMsgs = maxMsgs;
    q->maxMsgSize = maxMsgSize;
    q->buffer = (char*)kmalloc((maxMsgSize + sizeof(int))*maxMsgs, GFP_KERNEL);
    if (q->buffer == NULL)
    {
        q->flags = 0;
        return -MVKERNELEXT_ENOMEM;
    }

    MV_GLOBAL_LOCK();
    q->flags = 1;
    mv_waitqueue_init(&(q->rxWaitQueue));
    mv_waitqueue_init(&(q->txWaitQueue));

    strncpy(q->name, name, MV_MSGQ_NAME_LEN);
    q->name[MV_MSGQ_NAME_LEN] = 0;

    MV_GLOBAL_UNLOCK();
    return k;
}

#define MSGQ_BY_ID(msgId) \
    MV_GLOBAL_LOCK(); \
    if (unlikely(msgqId == 0 || msgqId >= mv_num_queues)) \
    { \
ret_einval: \
        MV_GLOBAL_UNLOCK(); \
        return -MVKERNELEXT_EINVAL; \
    } \
    q = mvMsgQs + msgqId; \
    if (unlikely(q->flags != 1)) \
        goto ret_einval;

#define CHECK_MSGQ() \
    if (unlikely(q->flags != 1)) \
    { \
        MV_GLOBAL_UNLOCK(); \
        return -MVKERNELEXT_EDELETED; \
    }

/**
* @internal mvKernelExt_MsgQDelete function
* @endinternal
*
* @brief   Destroys semaphore
*/
int mvKernelExt_MsgQDelete(int msgqId)
{
    mvMsgQSTC *q;
    int timeOut;

    MSGQ_BY_ID(msgqId);

    q->flags = 2; /* deleting */

    for (timeOut = HZ; q->waitRx && timeOut; timeOut--)
    {
        mv_waitqueue_wake_all(&(q->rxWaitQueue));
        if (q->waitRx)
        {
            MV_GLOBAL_UNLOCK();
            schedule_timeout(1);
            MV_GLOBAL_LOCK();
        }
    }
    for (timeOut = HZ; q->waitTx && timeOut; timeOut--)
    {
        mv_waitqueue_wake_all(&(q->txWaitQueue));
        if (q->waitTx)
        {
            MV_GLOBAL_UNLOCK();
            schedule_timeout(1);
            MV_GLOBAL_LOCK();
        }
    }

    mv_waitqueue_cleanup(&(q->rxWaitQueue));
    mv_waitqueue_cleanup(&(q->txWaitQueue));

    MV_GLOBAL_UNLOCK();
    kfree(q->buffer);

    q->flags = 0;

    return 0;
}

/**
* @internal mvKernelExt_MsgQSend function
* @endinternal
*
* @brief   Send message to queue
*
* @param[in] msgqId                   - Message queue Id
* @param[in] message                  -  data pointer
* @param[in] messageSize              - message size
* @param[in] timeOut                  - time out in miliseconds or
*                                      -1 for WAIT_FOREVER or 0 for NO_WAIT
* @param[in] userspace                - called from userspace
*                                       Zero if successful
*                                       -MVKERNELEXT_EINVAL  - bad ID passed
*                                       -MVKERNELEXT_ETIMEOUT - on timeout
*                                       -MVKERNELEXT_ENOMEM  - full and no wait
*                                       -MVKERNELEXT_EDELETED - deleted
*/
int mvKernelExt_MsgQSend(
    int     msgqId,
    void*   message,
    int     messageSize,
    int     timeOut,
    int     userspace
)
{
    char    *msg;
    mvMsgQSTC *q;

    MSGQ_BY_ID(msgqId);

    while (q->messages == q->maxMsgs)
    {
        /* queue full */
        if (timeOut == 0)
        {
            MV_GLOBAL_UNLOCK();
            return -MVKERNELEXT_EFULL; /* ??? -MVKERNELEXT_ETIMEOUT */
        }
        else
        {
            TASK_WILL_WAIT(current);
            q->waitTx++;
            if (timeOut != -1)
            {
#if HZ != 1000
                timeOut += 1000 / HZ - 1;
                timeOut /= 1000 / HZ;
#endif
                timeOut = mv_do_wait_on_queue_timeout(&(q->txWaitQueue), p, timeOut);
                CHECK_MSGQ();
                if (timeOut == 0)
                {
                    q->waitTx--;
                    MV_GLOBAL_UNLOCK();
                    return -MVKERNELEXT_ETIMEOUT;
                }
                if (timeOut == (unsigned long)(-1))
                {
                    q->waitTx--;
                    MV_GLOBAL_UNLOCK();
                    return -MVKERNELEXT_EINTR;
                }
            }
            else /* timeOut == -1, wait forever */
            {
                if (unlikely(mv_do_wait_on_queue(&(q->txWaitQueue), p)))
                {
                    q->waitTx--;
                    MV_GLOBAL_UNLOCK();
                    return -MVKERNELEXT_EINTR;
                }
                CHECK_MSGQ();
            }
            q->waitTx--;
        }
    }

    /* put message */
    msg = q->buffer + q->head * (q->maxMsgSize + sizeof(int));
    if (messageSize > q->maxMsgSize)
        messageSize = q->maxMsgSize;

    *((int*)msg) = messageSize;
    if (userspace)
    {
        if (copy_from_user(msg+sizeof(int), message, messageSize))
        {
            MV_GLOBAL_UNLOCK();
            return -MVKERNELEXT_EINVAL;
        }
    }
    else
    {
        memcpy(msg+sizeof(int), message, messageSize);

    }
    q->head++;
    if (q->head >= q->maxMsgs) /* round up */
        q->head = 0;
    q->messages++;

    /* signal to Recv thread if any */
    if (q->waitRx)
    {
        mv_waitqueue_wake_first(&(q->rxWaitQueue));
        /*
        if (unlikely(!q->rxWaitQueue.first))
            q->waitRx = 0;
        */
    }

    MV_GLOBAL_UNLOCK();
    return 0;
}

/**
* @internal mvKernelExt_MsgQRecv function
* @endinternal
*
* @brief   Receive message from queue
*
* @param[in] msgqId                   - Message queue Id
* @param[in] messageSize              - size of buffer pointed by message
* @param[in] timeOut                  - time out in miliseconds or
*                                      -1 for WAIT_FOREVER or 0 for NO_WAIT
* @param[in] userspace                - called from userspace
*
* @param[out] message                  -  data pointer
*                                       message size if successful
*                                       -MVKERNELEXT_EINVAL  - bad ID passed
*                                       -MVKERNELEXT_ETIMEOUT - on timeout
*                                       -MVKERNELEXT_ENOMEM  - empty and no wait
*                                       -MVKERNELEXT_EDELETED - deleted
*/
int mvKernelExt_MsgQRecv(
    int     msgqId,
    void*   message,
    int     messageSize,
    int     timeOut,
    int     userspace
)
{
    char    *msg;
    int  msgSize;
    mvMsgQSTC *q;

    MSGQ_BY_ID(msgqId);

    while (q->messages == 0)
    {
        /* queue empty */
        if (timeOut == 0)
        {
            MV_GLOBAL_UNLOCK();
            return -MVKERNELEXT_EEMPTY; /* ??? -MVKERNELEXT_ETIMEOUT */
        }
        else
        {
            TASK_WILL_WAIT(current);
            q->waitRx++;
            if (timeOut != -1)
            {
#if HZ != 1000
                timeOut += 1000 / HZ - 1;
                timeOut /= 1000 / HZ;
#endif
                timeOut = mv_do_wait_on_queue_timeout(&(q->rxWaitQueue), p, timeOut);
                CHECK_MSGQ();
                if (timeOut == 0)
                {
                    q->waitRx--;
                    MV_GLOBAL_UNLOCK();
                    return -MVKERNELEXT_ETIMEOUT;
                }
                if (timeOut == (unsigned long)(-1))
                {
                    q->waitRx--;
                    MV_GLOBAL_UNLOCK();
                    return -MVKERNELEXT_EINTR;
                }
            }
            else /* timeOut == -1, wait forever */
            {
                if (unlikely(mv_do_wait_on_queue(&(q->rxWaitQueue), p)))
                {
                    q->waitRx--;
                    MV_GLOBAL_UNLOCK();
                    return -MVKERNELEXT_EINTR;
                }
                CHECK_MSGQ();
            }
            q->waitRx--;
        }
    }
    /* get message */
    msg = q->buffer + q->tail * (q->maxMsgSize + sizeof(int));
    msgSize = *((int*)msg);
    if (msgSize > messageSize)
        msgSize = messageSize;

    if (userspace)
    {
        if (copy_to_user(message, msg+sizeof(int), msgSize))
        {
            msgSize = 0;
        }
    }
    else
    {
        memcpy(message, msg+sizeof(int), msgSize);
    }
    q->tail++;
    if (q->tail >= q->maxMsgs) /* round up */
        q->tail = 0;
    q->messages--;

    /* signal to Recv thread if any */
    if (q->waitTx)
    {
        mv_waitqueue_wake_first(&(q->txWaitQueue));
        /*
        if (unlikely(!q->txWaitQueue.first))
            q->waitTx = 0;*/
    }

    MV_GLOBAL_UNLOCK();
    return msgSize;
}

/**
* @internal mvKernelExt_MsgQNumMsgs function
* @endinternal
*
* @brief   Return number of messages pending in queue
*
* @retval numMessages              - number of messages pending in queue
*                                       -MVKERNELEXT_EINVAL  - bad ID passed
*/
int mvKernelExt_MsgQNumMsgs(int msgqId)
{
    int numMessages;
    mvMsgQSTC *q;

    MSGQ_BY_ID(msgqId);
    numMessages = q->messages;
    MV_GLOBAL_UNLOCK();

    return numMessages;
}

/**
* @internal mvMsgqIoctl function
* @endinternal
*
* @brief   The device ioctl() implementation
*/
int mvMsgqIoctl(unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case MVKERNELEXT_IOC_MSGQCREATE:
            {
                mv_msgq_create_stc lparam;
                if (copy_from_user(&lparam,
                            (mv_msgq_create_stc*)arg,
                            sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;
                return mvKernelExt_MsgQCreate(
                        lparam.name,
                        lparam.maxMsgs,
                        lparam.maxMsgSize);
            }
        case MVKERNELEXT_IOC_MSGQDELETE:
            return mvKernelExt_MsgQDelete(arg);
        case MVKERNELEXT_IOC_MSGQSEND:
            {
                mv_msgq_sr_stc lparam;
                if (copy_from_user(&lparam,
                            (mv_msgq_sr_stc*)arg,
                            sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;
                return mvKernelExt_MsgQSend(
                        lparam.msgqId,
                        lparam.message,
                        lparam.messageSize,
                        lparam.timeOut,
                        1/*from userspace*/);
            }
        case MVKERNELEXT_IOC_MSGQRECV:
            {
                mv_msgq_sr_stc lparam;
                if (copy_from_user(&lparam,
                            (mv_msgq_sr_stc*)arg,
                            sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;
                return mvKernelExt_MsgQRecv(
                        lparam.msgqId,
                        lparam.message,
                        lparam.messageSize,
                        lparam.timeOut,
                        1/*to userspace*/);
            }
        case MVKERNELEXT_IOC_MSGQNUMMSGS:
            return mvKernelExt_MsgQNumMsgs(arg);
    }
    return -MVKERNELEXT_ENOENT;
}

