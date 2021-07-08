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
* mvKernelExt.h
*
* DESCRIPTION:
*       functions in kernel mode special for mainOs.
*       External definitions
*
* DEPENDENCIES:
*       It is assumed that mv_waitqueue_t defined before this file included
*
*       $Revision: 7.*******************************************************************************/



#define MV_MAX_TASKS                40
#define MV_THREAD_NAME_LEN          16

#define MV_SEMAPHORES_MIN           32
#ifndef LINUX_SIM
#  define MV_SEMAPHORES_DEF         128
#else
#  define MV_SEMAPHORES_DEF         1024
#endif

#define MV_SEMAPTHORE_F_MTX         0x80000000
#define MV_SEMAPTHORE_F_COUNT       0x40000000
#define MV_SEMAPTHORE_F_BINARY      0x20000000
#define MV_SEMAPTHORE_F_TYPE_MASK   0xe0000000
#define MV_SEMAPTHORE_F_OPENEXIST   0x10000000
#define MV_SEMAPTHORE_F_FLAGS_MASK  0x10000000
#define MV_SEMAPTHORE_F_COUNT_MASK  0x0fffffff



#define MV_QUEUES_MIN 32
#define MV_QUEUES_DEF 32




typedef struct {
    char name[MV_THREAD_NAME_LEN];
    int vxw_priority;
    unsigned long int pthread_id;
} mv_registertask_stc;

#define MV_SEM_NAME_LEN  16
typedef struct {
    int flags;
    char name[MV_SEM_NAME_LEN];
} mv_sem_create_stc;

typedef struct {
    int semid;
    unsigned long timeout;
} mv_sem_timedwait_stc;

typedef struct {
    int flags;
    char name[MV_SEM_NAME_LEN];
} mv_sem_opennamed_stc;

typedef struct {
    int taskid;
    int vxw_priority;
} mv_priority_stc;

typedef struct {
    int taskid;
    unsigned long int pthread_id;
} mv_get_pthrid_stc;

#define MV_MSGQ_NAME_LEN 16
typedef struct {
    char name[MV_SEM_NAME_LEN];
    int  maxMsgs;
    int  maxMsgSize;
} mv_msgq_create_stc;

typedef struct {
    int             msgqId;
    void*           message;
    int             messageSize;
    unsigned long   timeOut;
} mv_msgq_sr_stc;


/********************************************************
 *
 * Error codes
 *
 ********************************************************/
#define MVKERNELEXT_EINTR        2
#define MVKERNELEXT_EPERM        3
#define MVKERNELEXT_EINVAL       4
#define MVKERNELEXT_ENOMEM       5
#define MVKERNELEXT_EDELETED     6
#define MVKERNELEXT_ETIMEOUT     7
#define MVKERNELEXT_EBUSY        8
#define MVKERNELEXT_ECONFLICT    9
#define MVKERNELEXT_EEMPTY      10
#define MVKERNELEXT_EFULL       11



/********************************************************
 *
 * IOCTL numbers
 *
 ********************************************************/
#define MVKERNELEXT_IOC_MAGIC 'k'
#define MVKERNELEXT_IOC_NOOP        _IO(MVKERNELEXT_IOC_MAGIC,   0)
#define MVKERNELEXT_IOC_TASKLOCK    _IO(MVKERNELEXT_IOC_MAGIC,   1)
#define MVKERNELEXT_IOC_TASKUNLOCK  _IO(MVKERNELEXT_IOC_MAGIC,   2)
#define MVKERNELEXT_IOC_TASKUNLOCKFORCE _IO(MVKERNELEXT_IOC_MAGIC,   3)
#define MVKERNELEXT_IOC_REGISTER    _IOW(MVKERNELEXT_IOC_MAGIC,  4, mv_registertask_stc)
#define MVKERNELEXT_IOC_UNREGISTER  _IO(MVKERNELEXT_IOC_MAGIC,   5)
#define MVKERNELEXT_IOC_SEMCREATE   _IOW(MVKERNELEXT_IOC_MAGIC,  6, mv_sem_create_stc)
#define MVKERNELEXT_IOC_SEMDELETE   _IOW(MVKERNELEXT_IOC_MAGIC,  7, long)
#define MVKERNELEXT_IOC_SEMSIGNAL   _IOW(MVKERNELEXT_IOC_MAGIC,  8, long)
#define MVKERNELEXT_IOC_SEMWAIT     _IOW(MVKERNELEXT_IOC_MAGIC,  9, long)
#define MVKERNELEXT_IOC_SEMTRYWAIT  _IOW(MVKERNELEXT_IOC_MAGIC, 10, long)
#define MVKERNELEXT_IOC_SEMWAITTMO  _IOW(MVKERNELEXT_IOC_MAGIC, 11, mv_sem_timedwait_stc)
#define MVKERNELEXT_IOC_TEST        _IO(MVKERNELEXT_IOC_MAGIC,  12)
#define MVKERNELEXT_IOC_SET_PRIO    _IOW(MVKERNELEXT_IOC_MAGIC, 13, mv_priority_stc)
#define MVKERNELEXT_IOC_GET_PRIO    _IOW(MVKERNELEXT_IOC_MAGIC, 14, mv_priority_stc)
#define MVKERNELEXT_IOC_SUSPEND     _IOW(MVKERNELEXT_IOC_MAGIC, 15, long)
#define MVKERNELEXT_IOC_RESUME      _IOW(MVKERNELEXT_IOC_MAGIC, 16, long)
#define MVKERNELEXT_IOC_DELETE      _IOW(MVKERNELEXT_IOC_MAGIC, 17, long)
#define MVKERNELEXT_IOC_GET_PTHRID  _IOW(MVKERNELEXT_IOC_MAGIC, 18, mv_get_pthrid_stc)

#define MVKERNELEXT_IOC_MSGQCREATE  _IOW(MVKERNELEXT_IOC_MAGIC, 19, mv_msgq_create_stc)
#define MVKERNELEXT_IOC_MSGQDELETE  _IOW(MVKERNELEXT_IOC_MAGIC, 20, long)
#define MVKERNELEXT_IOC_MSGQSEND    _IOW(MVKERNELEXT_IOC_MAGIC, 21, mv_msgq_sr_stc)
#define MVKERNELEXT_IOC_MSGQRECV    _IOW(MVKERNELEXT_IOC_MAGIC, 22, mv_msgq_sr_stc)
#define MVKERNELEXT_IOC_MSGQNUMMSGS _IOW(MVKERNELEXT_IOC_MAGIC, 23, long)


#ifdef  MVKERNELEXT_SYSCALLS
/********************************************************
 *
 * Syscall numbers for
 *
 *      long mv_ctl(unsigned int cmd, unsigned long param)
 *
 ********************************************************/
#if 0
#  define __NR_mv_tasklock        __NR_setxattr
#  define __NR_mv_taskunlock      __NR_getxattr
#  define __NR_mv_taskunlockforce __NR_listxattr
#  define __NR_mv_sem_wait        __NR_removexattr
#  define __NR_mv_sem_trywait     __NR_lsetxattr
#  define __NR_mv_sem_wait_tmo    __NR_lgetxattr
#  define __NR_mv_sem_signal      __NR_llistxattr
#  define __NR_mv_noop            __NR_lremovexattr
#endif
#  define __NR_mv_ctl             __NR_fsetxattr
#endif


#ifdef __KERNEL__

#define MV_TASKLOCK_STAT

/*
 * Typedef: struct mv_task
 *
 * Description: per-task structure
 *
 * Fields:
 *          task            - pointer to kernel's task structure
 *          tasklockflag    - task suspended flag:
 *                            0 - task running
 *                            1 - task suspended for a short period
 *                            2 - task suspended for a long period
 *          name            - task name. Useful for debugging purposes only
 *          vxw_priority    - vxWorks priority value (untranslated priority)
 *          pthread_id      - value for pthread_t pointer (userspace threads)
 *          waitqueue       - queue task suspended in
 *          wait_next       - next task in wait queue
 *
 */
struct mv_task {
    struct task_struct  *task;
    int                 tasklockflag;
    char                name[MV_THREAD_NAME_LEN+1];
    int                 vxw_priority;
    unsigned long int   pthread_id;
    mv_waitqueue_t      *waitqueue;
    struct mv_task      *wait_next;
#ifdef MV_TASKLOCK_STAT
    int                 tasklock_lcount;
    int                 tasklock_wcount;
#endif
};


/***** Static function declarations ************************************/

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
static void mv_waitqueue_init(
        mv_waitqueue_t* queue
);

/**
* @internal mv_waitqueue_cleanup function
* @endinternal
*
* @brief   Cleanup wait queue structure
*
* @param[in] queue                    - pointer to wait  structure
*                                       None
*/
static void mv_waitqueue_cleanup(
        mv_waitqueue_t* queue
);

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
static void mv_waitqueue_add(
        mv_waitqueue_t* queue,
        struct mv_task* tsk
);

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
static void mv_waitqueue_wake_first(
        mv_waitqueue_t* queue
);

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
static void mv_waitqueue_wake_all(
        mv_waitqueue_t* queue
);

/**
* @internal mv_delete_from_waitqueue function
* @endinternal
*
* @brief   remove task from wait queue
*
* @note Interrupts must be disabled when this function called
*
*/
static void mv_delete_from_waitqueue(
        struct mv_task* tsk
);

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
static int mv_do_short_wait_on_queue(
        mv_waitqueue_t* queue,
        struct mv_task* tsk,
        struct task_struct** owner
);

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
static int mv_do_wait_on_queue(
        mv_waitqueue_t* queue,
        struct mv_task* tsk
);

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
static unsigned long mv_do_wait_on_queue_timeout(
        mv_waitqueue_t* queue,
        struct mv_task* tsk,
        unsigned long timeout
);






/************************************************************************
*
* Task lookup functions
*
* These functions required to lookup tasks in task array
************************************************************************/

/**
* @internal mv_check_tasks function
* @endinternal
*
* @brief   Walk through task array and check if task still alive
*         Perform cleanup actions for dead tasks
*/
static void mv_check_tasks(void);

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
#ifdef MVKERNELEXT_TASK_STRUCT
#define gettask(tsk) ((struct mv_task*)tsk->mv_ptr);
#else
static struct mv_task* gettask(
        struct task_struct *tsk
);
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
static struct mv_task* gettask_cr(
        struct task_struct *tsk
);



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
int mvKernelExt_TaskLock(struct task_struct* tsk);

/**
* @internal mvKernelExt_TaskUnlock function
* @endinternal
*
* @brief   unlock scheduller from current task
*/
int mvKernelExt_TaskUnlock(struct task_struct* tsk, int force);






/**
* @internal mv_set_prio function
* @endinternal
*
* @brief   Set task priority
*/
static int mv_set_prio(mv_priority_stc *param);


/************************************************************************
*
* Semaphore functions
*
************************************************************************/

/**
* @internal mvKernelExt_SemInit function
* @endinternal
*
* @brief   Initialize semaphore support, create /proc for semaphores info
*/
static int mvKernelExt_SemInit(void);

/**
* @internal mvKernelExt_SemCleanup function
* @endinternal
*
* @brief   Perform semaphore cleanup actions before module unload
*/
static void mvKernelExt_SemCleanup(void);

/**
* @internal mvKernelExt_SemCreate function
* @endinternal
*
* @brief   Create a new semaphore or open existing one
*
* @retval Positive value           - semaphore ID
*                                       -MVKERNELEXT_EINVAL  - invalid parameter passed
*                                       -MVKERNELEXT_ENOMEM  - semaphore array is full
*                                       -MVKERNELEXT_ECONFLICT - open existing semaphore with different type
*                                       specified
*/
int mvKernelExt_SemCreate(int flags, const char *name);


/**
* @internal mvKernelExt_SemDelete function
* @endinternal
*
* @brief   Destroys semaphore
*/
int mvKernelExt_SemDelete(int semid);

/**
* @internal mvKernelExt_DeleteAll function
* @endinternal
*
* @brief   Destroys all semaphores
*         This is safety action which is executed when all tasks closed
*/
static void mvKernelExt_DeleteAll(void);

/**
* @internal mvKernelExt_SemSignal function
* @endinternal
*
* @brief   Signals to semaphore
*/
int mvKernelExt_SemSignal(int semid);

/**
* @internal mvKernelExt_SemWait function
* @endinternal
*
* @brief   Wait for semaphore
*/
int mvKernelExt_SemWait(int semid);

/**
* @internal mvKernelExt_SemTryWait function
* @endinternal
*
* @brief   Try to acquire semaphore without waiting
*/
int mvKernelExt_SemTryWait(int semid);

/**
* @internal mvKernelExt_SemWaitTimeout function
* @endinternal
*
* @brief   Wait for semaphore
*
* @param[in] semid                    - semaphore ID
*                                      timeout - timeout in milliseconds
*                                       Zero if successful
*                                       -MVKERNELEXT_EINVAL  - bad ID passed
*                                       -MVKERNELEXT_ENOMEM  - current task is not registered
*                                       and task array is full
*                                       -MVKERNELEXT_EINTR  - wait interrupted
*                                       -MVKERNELEXT_ETIMEOUT - wait timeout
*/
int mvKernelExt_SemWaitTimeout(
        int semid,
        unsigned long timeout
);

/**
* @internal mvKernelExt_SemUnlockMutexes function
* @endinternal
*
* @brief   Unlock all mutexes locked by dead task
*/
static void mvKernelExt_SemUnlockMutexes(
        struct task_struct  *owner
);


/**
* @internal mvKernelExt_MsgQInit function
* @endinternal
*
* @brief   Initialize message queues support, create /proc for queues info
*/
static int mvKernelExt_MsgQInit(void);

/**
* @internal mvKernelExt_DeleteAllMsgQ function
* @endinternal
*
* @brief   Destroys all message queues
*         This is safety action which is executed when all tasks closed
*/
static void mvKernelExt_DeleteAllMsgQ(void);

/**
* @internal mvKernelExt_MsgQCleanup function
* @endinternal
*
* @brief   Perform message queues cleanup actions before module unload
*/
static void mvKernelExt_MsgQCleanup(void);

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
);

/**
* @internal mvKernelExt_MsgQDelete function
* @endinternal
*
* @brief   Destroys semaphore
*/
int mvKernelExt_MsgQDelete(int msgqId);

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
);

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
);

/**
* @internal mvKernelExt_MsgQNumMsgs function
* @endinternal
*
* @brief   Return number of messages pending in queue
*
* @retval numMessages              - number of messages pending in queue
*                                       -MVKERNELEXT_EINVAL  - bad ID passed
*/
int mvKernelExt_MsgQNumMsgs(int msgqId);

#endif /* __KERNEL */


