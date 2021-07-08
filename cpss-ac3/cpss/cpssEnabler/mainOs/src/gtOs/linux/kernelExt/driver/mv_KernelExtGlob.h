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
* mvKernelExtGlob.h
*
* DESCRIPTION:
*       functions in kernel mode special for mainOs.
*       External definitions
*
* DEPENDENCIES:
*
*       $Revision:8$
*******************************************************************************/
#ifndef __mv_KernelExtGlob_h__
#define __mv_KernelExtGlob_h__

#include <linux/version.h>

#define MV_MAX_TASKS                60
#define MV_THREAD_NAME_LEN          16

#define MV_SEMAPHORES_MIN           32
#ifndef LINUX_SIM
#  define MV_SEMAPHORES_DEF         512
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
#define MV_QUEUES_DEF 64




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
#define MVKERNELEXT_ENOENT      12



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
 * Syscall number for
 *
 *      long mv_ctl(unsigned int cmd, unsigned long param)
 *
 ********************************************************/
#  define __NR_mv_ctl             __NR_fsetxattr
#endif


#endif /* __mv_KernelExtGlob_h__ */

