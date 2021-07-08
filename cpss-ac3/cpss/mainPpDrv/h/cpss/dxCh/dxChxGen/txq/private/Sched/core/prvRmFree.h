/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief rm_free interface.
 *
* @file rm_free.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_RM_FREE_H
#define SCHED_RM_FREE_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInterface.h>


/** Free Shaping Profile entry.
 * 
 *   @param[in]		hndl		Resource Manager handle.
 *   @param[in]		entry_ind	Entry index.
 *
 *   @return an integer return code.
 *   @retval zero on success. 
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is an invalid handle.
 *   @retval -EFAULT if entry_ind is out of range.
 *   @retval -ENOMSG if entry_ind is already free.
 */
int prvSchedRmFreeShapingProfile(rmctl_t hndl, uint32_t entry_ind);






#endif   /* RM_FREE_H */
