/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

********************************************************************************
* mvutil.c
*
* DESCRIPTION:
*
* DEPENDENCIES:
*
*       $Revision: 1$
*******************************************************************************/
#include "mv_util.h"
#include "mv_tasks.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <linux/sched/rt.h>
#endif

/************************************************************************
*
* waitqueue support functions
*
* These functions required to suspend thread till some event occurs
************************************************************************/

/**
* @internal mv_waitqueue_init function
* @endinternal
*
* @brief   Initialize wait queue structure
*
* @param[in] queue                    - pointer to wait  structure
*                                       None
*/
void mv_waitqueue_init(
        mv_waitqueue_t* queue
)
{
    memset(queue, 0, sizeof(*queue));
}

/**
* @internal mv_waitqueue_cleanup function
* @endinternal
*
* @brief   Cleanup wait queue structure
*
* @param[in] queue                    - pointer to wait  structure
*                                       None
*/
void mv_waitqueue_cleanup(
        mv_waitqueue_t* queue
)
{
    memset(queue, 0, sizeof(*queue));
}

/**
* @internal mv_waitqueue_add function
* @endinternal
*
* @brief   add task to wait queue
*
* @param[in] queue                    - pointer to wait  structure
*                                      tsk    - pointer to task structure
*                                       None
*
* @note Interrupts must be disabled when this function called
*
*/
void mv_waitqueue_add(
        mv_waitqueue_t* queue,
        struct mv_task* tsk
)
{
    tsk->waitqueue = queue;
    tsk->wait_next = NULL;
    if (queue->first == NULL)
    {
        queue->first = tsk;
    } else {
        queue->last->wait_next = tsk;
    }
    queue->last = tsk;
}

/**
* @internal mv_waitqueue_wake_first function
* @endinternal
*
* @brief   wakeup first task waiting in queue
*
* @param[in] queue                    - pointer to wait  structure
*                                       None
*
* @note Interrupts must be disabled when this function called
*
*/
void mv_waitqueue_wake_first(
        mv_waitqueue_t* queue
)
{
    struct mv_task *p;

    p = queue->first;

    if (!p)
        return;

    if (p->tasklockflag == 1)
        p->task->state &= ~TASK_INTERRUPTIBLE;
    else if (p->tasklockflag == 2)
        wake_up_process(p->task);
    p->tasklockflag = 0;


    p->waitqueue = NULL;
    queue->first = p->wait_next;
    if (queue->first == NULL)
        queue->last = NULL;
}

/**
* @internal mv_waitqueue_wake_all function
* @endinternal
*
* @brief   wakeup all tasks waiting in queue
*
* @param[in] queue                    - pointer to wait  structure
*                                       None
*
* @note Interrupts must be disabled when this function called
*
*/
void mv_waitqueue_wake_all(
        mv_waitqueue_t* queue
)
{
    struct mv_task *p;

    p = queue->first;

    if (!p)
        return;

    while (p)
    {
        if (p->tasklockflag == 1)
            p->task->state &= ~TASK_INTERRUPTIBLE;
        else if (p->tasklockflag == 2)
            wake_up_process(p->task);
        p->tasklockflag = 0;

        p->waitqueue = NULL;
        p = p->wait_next;
    }

    queue->first = queue->last = NULL;
}

/**
* @internal mv_delete_from_waitqueue function
* @endinternal
*
* @brief   remove task from wait queue
*
* @note Interrupts must be disabled when this function called
*
*/
void mv_delete_from_waitqueue(
        struct mv_task* tsk
)
{
    mv_waitqueue_t* queue;
    struct mv_task* p;

    queue = tsk->waitqueue;
    if (!queue)
        return;

    tsk->waitqueue = NULL;

    if (queue->first == tsk)
    {
        queue->first = tsk->wait_next;
        if (queue->first == NULL)
            queue->last = NULL;
        return;
    }

    for (p = queue->first; p; p = p->wait_next)
    {
        if (p->wait_next == tsk)
        {
            p->wait_next = tsk->wait_next;
            if (p->wait_next == NULL)
                queue->last = p;
            return;
        }
    }

    if (p->tasklockflag == 1)
        p->task->state &= ~TASK_INTERRUPTIBLE;
    else if (p->tasklockflag == 2)
        wake_up_process(p->task);
    p->tasklockflag = 0;

}

/**
* @internal mv_do_short_wait_on_queue function
* @endinternal
*
* @brief   Suspend a task on wait queue for a short period.
*         Function has a best performance in a cost of CPU usage
*         This is useful for mutual exclusion semaphores
* @param[in] queue                    - pointer to wait  structure
*                                      tsk    - pointer to task structure
*                                      owner  - resourse owner
*                                       Zero if wait successful
*                                       Non zero if wait interrupted (signal caught)
*
* @note Interrupts must be disabled when this function called
*
*/
int mv_do_short_wait_on_queue(
        mv_waitqueue_t* queue,
        struct mv_task* tsk,
        struct task_struct** owner
)
{
    int cnt = 10;
    mv_waitqueue_add(queue, tsk);

    tsk->tasklockflag = 1;
#ifndef ENABLE_REALTIME_SCHEDULING_POLICY
    while (tsk->waitqueue && cnt--)
    {
        if (unlikely(signal_pending(tsk->task)))
        {
            tsk->tasklockflag = 0;
            mv_delete_from_waitqueue(tsk);
            return -1;
        }
        tsk->task->state |= TASK_INTERRUPTIBLE;
        MV_GLOBAL_UNLOCK();
        yield();
        MV_GLOBAL_LOCK();
    }
#else
    while (tsk->waitqueue && cnt--)
    {
        if (rt_task(tsk->task) && *owner)
        {
            if (tsk->task->prio <= (*owner)->prio)
            {
                /* spin locks are not acceptable */
                break;
            }
        }
        if (unlikely(signal_pending(tsk->task)))
        {
            tsk->tasklockflag = 0;
            mv_delete_from_waitqueue(tsk);
            return -1;
        }
        tsk->task->state |= TASK_INTERRUPTIBLE;
        MV_GLOBAL_UNLOCK();
        yield();
        MV_GLOBAL_LOCK();
    }
#endif
    if (!tsk->waitqueue)
        return 0;

    /* currect task is realtime and has higher prio than resource owner */
    tsk->tasklockflag = 2;
    while (tsk->waitqueue)
    {
        if (unlikely(signal_pending(tsk->task)))
        {
            tsk->tasklockflag = 0;
            mv_delete_from_waitqueue(tsk);
            return -1;
        }
        set_task_state(tsk->task, TASK_INTERRUPTIBLE);
        MV_GLOBAL_UNLOCK();
        schedule();
        MV_GLOBAL_LOCK();
    }
    return 0;
}

/**
* @internal mv_do_wait_on_queue function
* @endinternal
*
* @brief   Suspend a task on wait queue.
*         Function has the same performance as mv_do_short_wait_on_queue when
*         task suspended for short period. After that task state changed to
*         suspended
*         This is useful for binary and counting semaphores
* @param[in] queue                    - pointer to wait  structure
*                                      tsk    - pointer to task structure
*                                       Zero if wait successful
*                                       Non zero if wait interrupted (signal caught)
*
* @note Interrupts must be disabled when this function called
*
*/
int mv_do_wait_on_queue(
        mv_waitqueue_t* queue,
        struct mv_task* tsk
)
{
    int cnt = 10;

    mv_waitqueue_add(queue, tsk);

    tsk->tasklockflag = 1;
    while (tsk->waitqueue && cnt--)
    {
        if (unlikely(signal_pending(tsk->task)))
        {
            tsk->tasklockflag = 0;
            mv_delete_from_waitqueue(tsk);
            return -1;
        }
        tsk->task->state |= TASK_INTERRUPTIBLE;
        MV_GLOBAL_UNLOCK();
        yield();
        MV_GLOBAL_LOCK();
        if (!tsk->waitqueue)
            return 0;
    }

    tsk->tasklockflag = 2;
    while (tsk->waitqueue)
    {
        if (unlikely(signal_pending(tsk->task)))
        {
            tsk->tasklockflag = 0;
            mv_delete_from_waitqueue(tsk);
            return -1;
        }
        set_task_state(tsk->task, TASK_INTERRUPTIBLE);
        MV_GLOBAL_UNLOCK();
        schedule();
        MV_GLOBAL_LOCK();
    }
    return 0;
}

/*******************************************************************************
* mv_do_wait_on_queue_timeout
*
* DESCRIPTION:
*       Suspend a task on wait queue.
*       Return if timer expited.
*
* INPUTS:
*       queue   - pointer to wait queue structure
*       tsk     - pointer to task structure
*       timeout - timeout in scheduller ticks
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Non zero if wait successful
*       Zero if timeout occured
*       -1 if wait interrupted (signal caught)
*
* COMMENTS:
*       Interrupts must be disabled when this function called
*
*******************************************************************************/
unsigned long mv_do_wait_on_queue_timeout(
        mv_waitqueue_t* queue,
        struct mv_task* tsk,
        unsigned long timeout
)
{
    mv_waitqueue_add(queue, tsk);

    tsk->tasklockflag = 2;
    while (tsk->waitqueue && timeout)
    {
        if (unlikely(signal_pending(tsk->task)))
        {
            tsk->tasklockflag = 0;
            mv_delete_from_waitqueue(tsk);
            return -1;
        }
        set_task_state(tsk->task, TASK_INTERRUPTIBLE);
        MV_GLOBAL_UNLOCK();
        timeout = schedule_timeout(timeout);
        MV_GLOBAL_LOCK();
    }
    if (tsk->waitqueue) /* timeout, delete from waitqueue */
    {
        tsk->tasklockflag = 0;
        mv_delete_from_waitqueue(tsk);
    }
    return timeout;
}


