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
*/
/**
********************************************************************************
* @file main.c
*/
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#ifdef MVKERNELEXT_SYSCALLS
#include <linux/unistd.h>
#endif

#include "mv_KernelExtGlob.h"
#include "mv_util.h"
#include "mv_tasks.h"
#include "mv_sem.h"
#include "mv_msgq.h"

#define MV_DRV_NAME         "mvKernelExt"
static int                  major = 244;
static int                  minor = 1;
static struct cdev          mvKernelExt_cdev;
static struct class*        mvKernelExt_class;
static struct device*       mvKernelExt_device;
static int                  mvKernelExt_opened = 0;

#ifdef CONFIG_SMP
/* spinlock_t mv_giantlock = SPIN_LOCK_UNLOCKED; */
DEFINE_SPINLOCK(mv_giantlock);
#endif

typedef int (*mvInitFunc)(void);
typedef void (*mvCleanupFunc)(void);
typedef int (*mvIoctlFunc)(unsigned int cmd, unsigned long arg);
struct {
    const char     *name;
    mvInitFunc      init;
    mvCleanupFunc   cleanup;
    mvIoctlFunc     ioctl;
} mv_sub[] = {
    { "task",   mvTasksInit, mvTasksCleanup, mvTasksIoctl },
    { "sem",    mvSemInit,   mvSemCleanup,   mvSemIoctl },
    { "msgq",   mvMsgqInit,  mvMsgqCleanup,  mvMsgqIoctl },
    { NULL, NULL, NULL }
};

static int mvKernelExt_open(
        struct inode * inode,
        struct file * filp
)
{
    mv_check_tasks();

    MV_GLOBAL_LOCK();
    mvKernelExt_opened++;
    MV_GLOBAL_UNLOCK();

    return 0;
}


static int mvKernelExt_release(
        struct inode * inode,
        struct file * file
)
{
    printk("mvKernelExt_release\n");

    /*!!! check task list */
    mv_check_tasks();

    MV_GLOBAL_LOCK();
    mv_unregistertask(current);
    mvKernelExt_opened--;
    if (mvKernelExt_opened == 0)
    {
        mvTasks_DeleteAll();
        mvSem_DeleteAll();
        mvMsgq_DeleteAll();
    }
    MV_GLOBAL_UNLOCK();

    return 0;
}

/**
* @internal mvKernelExt_ioctl function
* @endinternal
*
* @brief   The device ioctl() implementation
*/
static long mvKernelExt_ioctl(
        struct file *filp,
        unsigned int cmd,
        unsigned long arg
)
{
    int i, rc;
    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (unlikely(_IOC_TYPE(cmd) != MVKERNELEXT_IOC_MAGIC))
    {
        printk("wrong ioctl magic key\n");
        return -ENOTTY;
    }

    if (cmd == MVKERNELEXT_IOC_NOOP)
        return 0;
    for (i = 0; mv_sub[i].name; i++) {
        rc = mv_sub[i].ioctl(cmd, arg);
        if (rc != -MVKERNELEXT_ENOENT)
            return rc;
    }
    switch(cmd)
    {
        case MVKERNELEXT_IOC_TEST:
            printk("mvKernelExt_TEST()\n");
            return 0;
    }
    printk (KERN_WARNING "Unknown ioctl (0x%x).\n", cmd);
    return -ENOENT;
}

#ifdef MVKERNELEXT_SYSCALLS
/************************************************************************
 *
 * syscall entries for fast calls
 *
 ************************************************************************/
/* fast call to KernelExt ioctl */
asmlinkage long sys_mv_ctl(unsigned int cmd, unsigned long arg)
{
    return mvKernelExt_ioctl(NULL, cmd, arg);
}

extern long sys_call_table[];

#define OWN_SYSCALLS 1

#ifdef __NR_SYSCALL_BASE
#  define __SYSCALL_TABLE_INDEX(name) (__NR_##name-__NR_SYSCALL_BASE)
#else
#  define __SYSCALL_TABLE_INDEX(name) (__NR_##name)
#endif

#define __TBL_ENTRY(name) { __SYSCALL_TABLE_INDEX(name), (long)sys_##name, 0 }
static struct {
    int     entry_number;
    long    own_entry;
    long    saved_entry;
} override_syscalls[OWN_SYSCALLS] = {
    __TBL_ENTRY(mv_ctl)
};
#undef  __TBL_ENTRY


static int mv_OverrideSyscalls(void)
{
    int k;
    for (k = 0; k < OWN_SYSCALLS; k++)
    {
        override_syscalls[k].saved_entry =
            sys_call_table[override_syscalls[k].entry_number];
        sys_call_table[override_syscalls[k].entry_number] =
            override_syscalls[k].own_entry;
    }
    return 0;
}

static int mv_RestoreSyscalls(void)
{
    int k;
    for (k = 0; k < OWN_SYSCALLS; k++)
    {
        if (override_syscalls[k].saved_entry)
            sys_call_table[override_syscalls[k].entry_number] =
                override_syscalls[k].saved_entry;
    }
    return 0;
}
#endif /* MVKERNELEXT_SYSCALLS */

static struct file_operations mvKernelExt_fops =
{
    .unlocked_ioctl  = mvKernelExt_ioctl,
    .open   = mvKernelExt_open,
    .release= mvKernelExt_release /* A.K.A close */
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,69)
static char *mvKernelExt_devnode(struct device *dev, umode_t *mode)
#else /* < 3.4.69 */
static char *mvKernelExt_devnode(struct device *dev, mode_t *mode)
#endif /* < 3.4.69 */
{
	return kasprintf(GFP_KERNEL, "%s", dev->kobj.name);
}
#endif /* >= 2.6.32 */

static void mvKernelExt_cleanup(void)
{
    int i;

#ifdef MVKERNELEXT_SYSCALLS
    mv_RestoreSyscalls();
#endif

    for (i = 0; mv_sub[i].name; i++)
        mv_sub[i].cleanup();

	device_destroy(mvKernelExt_class, MKDEV(major, minor));
	class_destroy(mvKernelExt_class);
	cdev_del(&mvKernelExt_cdev);

	unregister_chrdev_region(MKDEV(major, minor), 1);
}

static int mvKernelExt_init(void)
{
	int result = 0;
    int i;
	dev_t dev = MKDEV(major, minor);

	/* first thing register the device at OS */
	/* Register your major. */
	result = register_chrdev_region(dev, 1, MV_DRV_NAME);
	if (result < 0) {
		printk(MV_DRV_NAME "_init: register_chrdev_region err= %d\n", result);
		return result;
	}

	cdev_init(&mvKernelExt_cdev, &mvKernelExt_fops);
	mvKernelExt_cdev.owner = THIS_MODULE;
	result = cdev_add(&mvKernelExt_cdev, dev, 1);
	if (result) {
		printk(MV_DRV_NAME "_init: cdev_add err= %d\n", result);
error_region:
		unregister_chrdev_region(dev, 1);
		return result;
	}
	mvKernelExt_class = class_create(THIS_MODULE, MV_DRV_NAME);
	if (IS_ERR(mvKernelExt_class)) {
		printk(KERN_ERR "Error creating " MV_DRV_NAME " class.\n");
		cdev_del(&mvKernelExt_cdev);
		result = PTR_ERR(mvKernelExt_class);
		goto error_region;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	mvKernelExt_class->devnode = mvKernelExt_devnode;
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	mvKernelExt_device = device_create(mvKernelExt_class, NULL, dev, NULL, MV_DRV_NAME);
#else
	mvKernelExt_device = device_create(mvKernelExt_class, NULL, dev, MV_DRV_NAME);
#endif

    for (i = 0; mv_sub[i].name; i++)
    {
        result = mv_sub[i].init();
        if (result) {
            for (; i > 0; i--)
                mv_sub[i-1].cleanup();

            device_destroy(mvKernelExt_class, MKDEV(major, minor));
            class_destroy(mvKernelExt_class);
            cdev_del(&mvKernelExt_cdev);
            unregister_chrdev_region(MKDEV(major, minor), 1);
            return result;
        }
    }

#ifdef MVKERNELEXT_SYSCALLS
    mv_OverrideSyscalls();
#endif

	return 0;
}

module_init(mvKernelExt_init);
module_exit(mvKernelExt_cleanup);

module_param(major, int, S_IRUGO);
module_param(minor, int, S_IRUGO);

MODULE_AUTHOR("Marvell Semi.");
MODULE_LICENSE("GPL");



