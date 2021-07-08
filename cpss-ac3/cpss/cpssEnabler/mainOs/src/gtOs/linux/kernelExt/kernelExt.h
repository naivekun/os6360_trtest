/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file kernelExt.h
*
* @brief defines the interface to kernel module
*
* @version   5
********************************************************************************
*/
#ifndef _kernelExt_h
#define _kernelExt_h

#include <sys/ioctl.h>
#ifdef  MVKERNELEXT_SYSCALLS
#   include <linux/unistd.h>
#endif

#include "driver/mv_KernelExtGlob.h"

extern int mvKernelExtFd;

#ifdef  MVKERNELEXT_SYSCALLS

#   if defined(_syscall0)

        int mv_ctl(unsigned int cmd, unsigned long arg);
#       define mv_ctrl(cmd,arg)  mv_ctl((cmd),(unsigned long)(arg))

#   else /* !defined(_syscall0) */

#       include <unistd.h>
#       include <sys/syscall.h>
#       define mv_ctrl(cmd,arg)         syscall(__NR_mv_ctl, (cmd), (arg))

#   endif /* KERNEL_SYSCALLS_INDIRECT */



#else /* !defined MVKERNELEXT_SYSCALLS */
#   include <sys/ioctl.h>
#   define mv_ctrl(cmd,arg)         ioctl(mvKernelExtFd, (cmd), (arg))
#endif /* !defined MVKERNELEXT_SYSCALLS */

#define mv_noop()                mv_ctrl(MVKERNELEXT_IOC_NOOP,0)
#define mv_tasklock()            mv_ctrl(MVKERNELEXT_IOC_TASKLOCK,0)
#define mv_taskunlock()          mv_ctrl(MVKERNELEXT_IOC_TASKUNLOCK,0)
#define mv_taskunlockforce()     mv_ctrl(MVKERNELEXT_IOC_TASKUNLOCKFORCE,0)
#define mv_sem_signal(semid)     mv_ctrl(MVKERNELEXT_IOC_SEMSIGNAL, semid)
#define mv_sem_wait(semid)       mv_ctrl(MVKERNELEXT_IOC_SEMWAIT, semid)
#define mv_sem_trywait(semid)    mv_ctrl(MVKERNELEXT_IOC_SEMTRYWAIT, semid)

#endif /* _kernelExt_h */


