#ifndef _SCHED_RM_ALLOC_H_
#define _SCHED_RM_ALLOC_H_

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
 * @brief rm_alloc interface.
 *
* @file rm_alloc.h
*
* $Revision: 2.0 $
 */




#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInterface.h>
/* for definition of schedLevel */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>


/** Find free Shaping Profile entry.
 * 
 *   @param[in]		hndl		Resource Manager handle.
 *
 *   @return an integer positive index of found free entry.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is an invalid handle.
 *   @retval -ENOBUFS when no free entry.
 */
int prvCpssRmFindFreeShapingProfile(rmctl_t hndl);












#endif   /* RM_ALLOC_H */
