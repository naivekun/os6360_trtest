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
*/
/**
********************************************************************************
* @file mv_util.h
*/
#ifndef __mv_util_h__
#define __mv_util_h__

#define MV_TASKLOCK_STAT

#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "mv_KernelExtGlob.h"

#ifdef CONFIG_PROC_FS
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)
# define proc_create(_nm,_mode,_parent,_fops) \
    { \
        struct proc_dir_entry *entry; \
        entry = create_proc_entry(_nm, _mode, _parent); \
        if (entry) \
            entry->proc_fops = _fops; \
    }
#endif
#endif
/**
* @struct mv_waitqueue_t
 *
 * @brief wait queue
 * This type defined here because it is close to kernel internals
*/
typedef struct {
    struct mv_task      *first;
    struct mv_task      *last;
} mv_waitqueue_t;

#ifndef CONFIG_SMP
#  define MV_GLOBAL_LOCK()      local_irq_disable()
#  define MV_GLOBAL_UNLOCK()    local_irq_enable()
#else /* CONFIG_SMP */
#  include <linux/spinlock.h>
   extern spinlock_t mv_giantlock;
#  define MV_GLOBAL_LOCK()      spin_lock_irq(&mv_giantlock)
#  define MV_GLOBAL_UNLOCK()    spin_unlock_irq(&mv_giantlock)
#endif /* CONFIG_SMP */


#define MV_TASKLOCK_STAT


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
void mv_waitqueue_init(
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
void mv_waitqueue_cleanup(
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
void mv_waitqueue_add(
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
void mv_waitqueue_wake_first(
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
void mv_waitqueue_wake_all(
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
void mv_delete_from_waitqueue(
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
int mv_do_short_wait_on_queue(
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
int mv_do_wait_on_queue(
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
unsigned long mv_do_wait_on_queue_timeout(
        mv_waitqueue_t* queue,
        struct mv_task* tsk,
        unsigned long timeout
);



#define TASK_WILL_WAIT(tsk) \
    struct mv_task *p; \
    if (unlikely((p = gettask_cr(tsk)) == NULL)) \
    { \
        MV_GLOBAL_UNLOCK(); \
        return -MVKERNELEXT_ENOMEM; \
    }




#endif /* __mv_util_h__ */


