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
* mvKernelExt.c
*
* DESCRIPTION:
*       functions in kernel mode special for mainOs.
*
* DEPENDENCIES:
*       mvKernelExt.h
*
*       $Revision: 11$
*******************************************************************************/

#include <linux/sched.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/stop_machine.h>

#include "mv_tasks.h"
#include "mv_sem.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <linux/sched/rt.h>
#endif

/* task locking data */
static struct task_struct*  mv_tasklock_owner = NULL;
static mv_waitqueue_t       mv_tasklock_waitqueue;
static int                  mv_tasklock_count = 0;
#ifdef MV_TASKLOCK_STAT
static int                  mv_tasklock_lcount = 0;
static int                  mv_tasklock_wcount = 0;
#endif

/* task array */
static int                  mv_max_tasks = MV_MAX_TASKS;
static struct mv_task**     mv_tasks = NULL;
static struct mv_task*      mv_tasks_alloc = NULL;
static int                  mv_num_tasks = 0;

module_param(mv_max_tasks, int, S_IRUGO);

#ifdef ENABLE_REALTIME_SCHEDULING_POLICY
#  define MV_PRIO_MIN   1
#  define MV_PRIO_MAX   (MAX_USER_RT_PRIO-20)
#else
#  define MV_PRIO_MIN   MAX_RT_PRIO
#  define MV_PRIO_MAX   MAX_PRIO
#endif

static int mvKernelExt_TaskUnlock(struct task_struct* tsk, int force);

/************************************************************************
 *
 * support functions
 *
 ************************************************************************/
#ifndef MVKERNELEXT_TASK_STRUCT
/*******************************************************************************
* gettask
*
* DESCRIPTION:
*       Search for a mv_task by pointer to kernel's task structure
*
* INPUTS:
*       tsk     - pointer to kernel's task structure
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to mv_task
*       NULL if task is not registered yet
*
* COMMENTS:
*       Interrupts must be disabled when this function called
*
*******************************************************************************/
static struct mv_task* gettask(
        struct task_struct *tsk
)
{
    int k;

    for (k = 0; k < mv_num_tasks; k++)
        if (mv_tasks[k]->task == tsk)
            return mv_tasks[k];

    return NULL;
}
#endif

#ifdef MVKERNELEXT_TASK_STRUCT
static void mv_cleanup(struct task_struct *tsk);
#endif

/*******************************************************************************
* gettask_cr
*
* DESCRIPTION:
*       Search for a mv_task by pointer to kernel's task structure
*       Register task in array if task was not registered yet
*
* INPUTS:
*       tsk     - pointer to kernel's task structure
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to mv_task
*       NULL if task cannot be registered (task array is full)
*
* COMMENTS:
*       Interrupts must be disabled when this function called
*
*******************************************************************************/
struct mv_task* gettask_cr(
        struct task_struct *tsk
)
{
    struct mv_task *p;

    p = gettask(tsk);
    if (unlikely(p == NULL))
    {
        if (mv_num_tasks >= mv_max_tasks)
            return NULL;
        p = mv_tasks[mv_num_tasks++];
        memset(p, 0, sizeof(*p));
        p->task = tsk;
#ifdef MVKERNELEXT_TASK_STRUCT
        tsk->mv_ptr = p;
        tsk->mv_cleanup = mv_cleanup;
#endif
    }
    return p;
}

/************************************************************************
*
* Task lookup functions
*
* These functions required to lookup tasks in task array
************************************************************************/

/**
* @internal mv_check_tasks_unlocked function
* @endinternal
*
* @brief   Walk through task array and check if task still alive
*         Perform cleanup actions for dead tasks
*/
static int mv_check_tasks_unlocked(void* data)
{
    int k;

    (void)data;
    for (k = 0; k < mv_num_tasks; )
    {
        /* search task */
        struct task_struct *p, *g;
        int found = 0;
        do_each_thread(g, p) {
            if (p == mv_tasks[k]->task)
                found = 1;
        } while_each_thread(g, p);
        if (!found)
        {
            /* task not found */
            mv_unregistertask(mv_tasks[k]->task);
            continue;
        }
        if (mv_tasks[k]->task->exit_state)
        {
            mv_unregistertask(mv_tasks[k]->task);
            continue;
        }
        k++;
    }
    return 0;
}
/**
* @internal mv_check_tasks function
* @endinternal
*
* @brief   Walk through task array and check if task still alive
*         Perform cleanup actions for dead tasks
*/
void mv_check_tasks(void)
{
    /* suspend all tasks to prevent schedulling during this check */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
    stop_machine(mv_check_tasks_unlocked, NULL, NULL);
#else
    stop_machine_run(mv_check_tasks_unlocked, NULL, NR_CPUS);
#endif
}

void mvTasks_DeleteAll(void)
{
    /* TODO */
    mv_check_tasks();
}

/**
* @internal translate_priority function
* @endinternal
*
* @brief   Translates a v2pthread priority into kernel priority
*
* @param[in] policy                   - scheduler policy
* @param[in] priority                 - vxWorks task priority
*                                       kernel priority
*/
static int translate_priority(
        int policy,
        int priority
)
{
    if (policy == SCHED_NORMAL)
        return 0;

    /*
    **  Validate the range of the user's task priority.
    */
    if (priority < 0 || priority > 255)
        return MV_PRIO_MAX;

#ifdef  CPU_ARM
    if (priority <= 10)
        return MV_PRIO_MAX-1;
#endif
    /* reverse */
    priority = 255 - priority;

    /* translate 0..255 to MAX_RT_PRIO..MAX_PRIO */
    priority *= (MV_PRIO_MAX-MV_PRIO_MIN);
    priority >>= 8;
    priority += MV_PRIO_MIN;
    if (priority >= MV_PRIO_MAX)
        priority = MV_PRIO_MAX-1;

    return( priority );
}

/*******************************************************************************
* get_task_by_id
*
* DESCRIPTION:
*       Search task by task ID
*
* INPUTS:
*       tid    - Task ID
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to mv_task
*       Null if task not found
*
* COMMENTS:
*       Interrupts must be disabled when this function called
*
*******************************************************************************/
static struct mv_task* get_task_by_id(int tid)
{
    int k;

    if (tid == 0)
        return gettask(current);

    for (k = 0; k < mv_num_tasks; k++)
        if (mv_tasks[k]->task->pid == tid)
            return mv_tasks[k];

    return NULL;
}

/**
* @internal mv_get_pthrid function
* @endinternal
*
* @brief   Return pthread ID for task
*         Pthread ID associated with task in mv_registertask
*/
static int mv_get_pthrid(mv_get_pthrid_stc *param)
{
    struct mv_task* p;

    MV_GLOBAL_LOCK();
    p = get_task_by_id(param->taskid);
    MV_GLOBAL_UNLOCK();

    if (p == NULL)
        return -MVKERNELEXT_EINVAL;

    param->pthread_id = p->pthread_id;

    return 0;
}

/**
* @internal mv_get_prio function
* @endinternal
*
* @brief   Set task priority
*/
static int mv_get_prio(mv_priority_stc *param)
{
    struct mv_task* p;

    MV_GLOBAL_LOCK();
    p = get_task_by_id(param->taskid);
    MV_GLOBAL_UNLOCK();

    if (p == NULL)
        return -MVKERNELEXT_EINVAL;

    param->vxw_priority = p->vxw_priority;

    return 0;
}

/**
* @internal mv_suspend function
* @endinternal
*
* @brief   Suspend task execution
*/
static int mv_suspend(int taskid)
{
    struct mv_task* p;

    MV_GLOBAL_LOCK();
    p = get_task_by_id(taskid);
    MV_GLOBAL_UNLOCK();

    if (p == NULL)
        return -MVKERNELEXT_EINVAL;

    if (p->task == current)
        mvKernelExt_TaskUnlock(p->task, 1);

    send_sig(SIGSTOP, p->task, 1);

    return 0;
}

/**
* @internal mv_resume function
* @endinternal
*
* @brief   Resume task execution
*/
static int mv_resume(int taskid)
{
    struct mv_task* p;

    MV_GLOBAL_LOCK();
    p = get_task_by_id(taskid);
    MV_GLOBAL_UNLOCK();

    if (p == NULL)
        return -MVKERNELEXT_EINVAL;

    send_sig(SIGCONT, p->task, 1);

    return 0;
}

/**
* @internal mv_delete function
* @endinternal
*
* @brief   Destroy task
*/
static int mv_delete(int taskid)
{
    struct mv_task* p;
    struct task_struct *tsk;

    MV_GLOBAL_LOCK();
    p = get_task_by_id(taskid);
    MV_GLOBAL_UNLOCK();

    if (p == NULL)
        return -MVKERNELEXT_EINVAL;

    tsk = p->task;

    MV_GLOBAL_LOCK();
    mv_unregistertask(tsk);
    MV_GLOBAL_UNLOCK();

    send_sig(SIGRTMIN/*SIGCANCEL*/, tsk, 1);
    return 0;
}


/**
* @internal mv_set_prio function
* @endinternal
*
* @brief   Set task priority
*/
static int mv_set_prio(mv_priority_stc *param)
{
    struct mv_task* p;
    struct sched_param prio;
    int policy;

    MV_GLOBAL_LOCK();
    p = get_task_by_id(param->taskid);
    MV_GLOBAL_UNLOCK();

    if (p == NULL)
        return -MVKERNELEXT_EINVAL;

    p->vxw_priority = param->vxw_priority;

#ifdef ENABLE_REALTIME_SCHEDULING_POLICY
    policy = SCHED_RR;
#else
    policy = SCHED_NORMAL;
#endif
    prio.sched_priority = translate_priority(policy, param->vxw_priority);
    sched_setscheduler(p->task, policy, &prio);

    return 0;
}

/************************************************************************
 *
 * task locking
 *
 ************************************************************************/
/**
* @internal mvKernelExt_TaskLock function
* @endinternal
*
* @brief   lock scheduller to current task
*/
static int mvKernelExt_TaskLock(struct task_struct* tsk)
{
    struct mv_task *p;

    MV_GLOBAL_LOCK();

    p = gettask_cr(tsk);
    if (unlikely(p == NULL))
    {
        MV_GLOBAL_UNLOCK();
        return -MVKERNELEXT_ENOMEM;
    }

    if (mv_tasklock_owner && mv_tasklock_owner != tsk)
    {
#ifdef MV_TASKLOCK_STAT
        mv_tasklock_wcount++;
        p->tasklock_wcount++;
#endif
        do {
            if (mv_do_short_wait_on_queue(&mv_tasklock_waitqueue, p, &mv_tasklock_owner))
            {
                MV_GLOBAL_UNLOCK();
                return -MVKERNELEXT_EINTR;
            }
        } while (mv_tasklock_owner);
    }

#ifdef MV_TASKLOCK_STAT
    mv_tasklock_lcount++;
    p->tasklock_lcount++;
#endif
    mv_tasklock_owner = tsk;
    mv_tasklock_count++;
    tsk->state = TASK_RUNNING;
    MV_GLOBAL_UNLOCK();

    return 0;
}

/**
 * sys_mv_taskunlock - unlock scheduller from current task
 */
/**
* @internal mvKernelExt_TaskUnlock function
* @endinternal
*
* @brief   unlock scheduller from current task
*/
static int mvKernelExt_TaskUnlock(struct task_struct* tsk, int force)
{
    MV_GLOBAL_LOCK();
    if (unlikely(mv_tasklock_owner != tsk))
    {
        MV_GLOBAL_UNLOCK();
        return -MVKERNELEXT_EPERM;
    }

    if (force)
        mv_tasklock_count = 0;
    else
        mv_tasklock_count--;

    if (mv_tasklock_count > 0)
    {
        MV_GLOBAL_UNLOCK();
        return 0;
    }

    mv_waitqueue_wake_all(&mv_tasklock_waitqueue);

    mv_tasklock_owner = NULL;
    MV_GLOBAL_UNLOCK();

    return 0;
}

/**
* @internal mv_registertask function
* @endinternal
*
* @brief   Register current task. Store name for this task
*/
static int mv_registertask(mv_registertask_stc* taskinfo)
{
	struct task_struct *curr;
    struct mv_task *p;

    curr = current;

    MV_GLOBAL_LOCK();

    p = gettask_cr(curr);
    if (unlikely(p == NULL))
    {
        MV_GLOBAL_UNLOCK();
        return -MVKERNELEXT_ENOMEM;
    }

    memcpy(p->name, taskinfo->name, MV_THREAD_NAME_LEN);
    p->name[MV_THREAD_NAME_LEN] = 0;
    p->vxw_priority = taskinfo->vxw_priority;
    p->pthread_id = taskinfo->pthread_id;

    MV_GLOBAL_UNLOCK();
    return 0;
}

/**
* @internal mv_unregistertask function
* @endinternal
*
* @brief   Unregister task. Unlock mutexes locked by task
*
* @note Interrupts must be disabled when this function called
*
*/
int mv_unregistertask(struct task_struct* tsk)
{
    struct mv_task* p = NULL;
    int k;

#ifdef MVKERNELEXT_TASK_STRUCT
    tsk->mv_ptr = NULL;
    tsk->mv_cleanup = NULL;
#endif

    mvSem_UnlockMutexes(tsk);

    if (mv_tasklock_owner == tsk)
    {
        mv_tasklock_count = 0;
        mv_waitqueue_wake_all(&mv_tasklock_waitqueue);
        mv_tasklock_owner = NULL;
    }

    for (k = 0; k < mv_num_tasks; k++)
        if (mv_tasks[k]->task == tsk)
        {
            p = mv_tasks[k];
            break;
        }

    if (p == NULL)
        return 1;

    mv_num_tasks--;
    if (mv_num_tasks > k) /* task was not the last in array */
    {
        mv_tasks[k] = mv_tasks[mv_num_tasks];
        mv_tasks[mv_num_tasks] = p;
    }

    mv_delete_from_waitqueue(p);

    return 0;
}

#ifdef  MVKERNELEXT_TASK_STRUCT
/**
* @internal mv_cleanup function
* @endinternal
*
* @brief   Execute cleanup actions when task finished
*         Called from kernel's do_exit() if patch applied to kernel
*/
static void mv_cleanup(struct task_struct *tsk)
{
    MV_GLOBAL_LOCK();
    mv_unregistertask(tsk);
    MV_GLOBAL_UNLOCK();
}
#endif

/**
* @internal mv_unregister_current_task function
* @endinternal
*
* @brief   Unregister current task
*/
static int mv_unregister_current_task(void)
{
	struct task_struct *curr;

    curr = current;

    MV_GLOBAL_LOCK();

    mv_unregistertask(curr);

    MV_GLOBAL_UNLOCK();
    return 0;
}

/**
* @internal mvTasksIoctl function
* @endinternal
*
* @brief   The device ioctl() implementation
*/
int mvTasksIoctl(unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case MVKERNELEXT_IOC_TASKLOCK:
            return mvKernelExt_TaskLock(current);

        case MVKERNELEXT_IOC_TASKUNLOCK:
            return mvKernelExt_TaskUnlock(current, 0);

        case MVKERNELEXT_IOC_TASKUNLOCKFORCE:
            return mvKernelExt_TaskUnlock(current, 1);

        case MVKERNELEXT_IOC_REGISTER:
            {
                mv_registertask_stc lparam;
                if (copy_from_user(&lparam,
                            (mv_registertask_stc*)arg,
                            sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;

                return mv_registertask(&lparam);
            }
        case MVKERNELEXT_IOC_UNREGISTER:
            return mv_unregister_current_task();

        case MVKERNELEXT_IOC_SET_PRIO:
            {
                mv_priority_stc lparam;

                if (copy_from_user(&lparam,
                            (mv_priority_stc*)arg,
                            sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;

                return mv_set_prio(&lparam);
            }
        case MVKERNELEXT_IOC_GET_PRIO:
            {
                mv_priority_stc lparam;
                int retval;

                if (copy_from_user(&lparam,
                            (mv_priority_stc*)arg,
                            sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;

                retval = mv_get_prio(&lparam);

                if (copy_to_user((mv_priority_stc*)arg,
                            &lparam, sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;

                return retval;
            }

        case MVKERNELEXT_IOC_SUSPEND:
            return mv_suspend(arg);
        case MVKERNELEXT_IOC_RESUME:
            return mv_resume(arg);

        case MVKERNELEXT_IOC_GET_PTHRID:
            {
                mv_get_pthrid_stc lparam;
                int retval;

                if (copy_from_user(&lparam,
                            (mv_get_pthrid_stc*)arg,
                            sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;

                retval = mv_get_pthrid(&lparam);

                if (copy_to_user((mv_get_pthrid_stc*)arg,
                        &lparam, sizeof(lparam)))
                    return -MVKERNELEXT_EINVAL;

                return retval;
            }

        case MVKERNELEXT_IOC_DELETE:
            return mv_delete(arg);
    }
    return -MVKERNELEXT_ENOENT;
}

#ifdef CONFIG_PROC_FS
/**
* @internal mvKernelExt_proc_status_show function
* @endinternal
*
* @brief   proc read data rooutine.
*         Use cat /proc/mvKernelExt to show task list and tasklock state
*/
static int mvKernelExt_proc_status_show(struct seq_file *m, void *v)
{
    int k;

#if 0
    seq_printf(m, "mvKernelExt major=%d\n", mvKernelExt_major);
#endif
#ifdef MV_TASKLOCK_STAT
    seq_printf(m, "tasklock count=%d wait=%d\n",
            mv_tasklock_lcount,
            mv_tasklock_wcount);
#endif

    seq_printf(m, "lock owner=%d\n", mv_tasklock_owner?mv_tasklock_owner->pid:0);
    {
        struct mv_task* p;
        for (p = mv_tasklock_waitqueue.first; p; p = p->wait_next)
            seq_printf(m, " wq: %d\n", p->task->pid);
    }
    /* list registered tasks */
    for (k = 0; k < mv_num_tasks; k++)
    {
        seq_printf(m,
                "  task id=%d state=0x%x prio=%d(%d) flag=%d"
#ifdef  MV_TASKLOCK_STAT
                " tlcount=%d tlwait=%d"
#endif
                " name=\"%s\"\n",
                mv_tasks[k]->task->pid,
                (unsigned int)mv_tasks[k]->task->state,
                mv_tasks[k]->task->prio,
                mv_tasks[k]->vxw_priority,
                mv_tasks[k]->tasklockflag,
#ifdef  MV_TASKLOCK_STAT
                mv_tasks[k]->tasklock_lcount,
                mv_tasks[k]->tasklock_wcount,
#endif
                mv_tasks[k]->name);
    }
    return 0;
}
static int mvKernelExt_proc_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, mvKernelExt_proc_status_show, NULL);
}

static const struct file_operations mvKernelExt_read_proc_operations = {
	.open		= mvKernelExt_proc_status_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif /* CONFIG_PROC_FS */

/**
* @internal mvTasksCleanup function
* @endinternal
*
* @brief   Perform cleanup actions while module unloading
*/
void mvTasksCleanup(void)
{
#ifdef MVKERNELEXT_TASK_STRUCT
    int k;
#endif

#ifdef MVKERNELEXT_TASK_STRUCT
    MV_GLOBAL_LOCK();
    for (k = 0; k < mv_num_tasks; k++)
    {
        mv_tasks[k]->task->mv_ptr = NULL;
        mv_tasks[k]->task->mv_cleanup = NULL;
    }
    MV_GLOBAL_UNLOCK();
#endif

    if (mv_tasks)
        kfree(mv_tasks);
    mv_tasks = NULL;

    if (mv_tasks_alloc)
        kfree(mv_tasks_alloc);
    mv_tasks_alloc = NULL;
    mv_waitqueue_cleanup(&mv_tasklock_waitqueue);
}

/**
* @internal mvTasksInit function
* @endinternal
*
* @brief   Module initialization
*/
int mvTasksInit(void)
{
    int         result = 0;

    /* allocate task array for tasklock */
    mv_tasks_alloc = (struct mv_task*) kmalloc(sizeof(struct mv_task) * mv_max_tasks, GFP_KERNEL);
    mv_tasks = (struct mv_task**) kmalloc(sizeof(struct mv_task*) * mv_max_tasks, GFP_KERNEL);
    if (mv_tasks_alloc == NULL || mv_tasks == NULL)
    {
        if (mv_tasks)
            kfree(mv_tasks);
        if (mv_tasks_alloc)
            kfree(mv_tasks_alloc);
        mv_tasks = NULL;
        mv_tasks_alloc = NULL;
        result = -ENOMEM;
        printk("mvKernelExt_init: unable to allocate task array\n");
        return result;
    }
    for (result = 0; result < mv_max_tasks; result++)
        mv_tasks[result] = &(mv_tasks_alloc[result]);
    mv_waitqueue_init(&mv_tasklock_waitqueue);

#ifdef CONFIG_PROC_FS
    proc_create("mvKernelExt", S_IRUGO, NULL, &mvKernelExt_read_proc_operations);
#endif
    return 0;
}


