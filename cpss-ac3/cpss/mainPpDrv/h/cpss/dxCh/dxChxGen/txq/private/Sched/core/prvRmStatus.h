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
 * @brief rm_status interface.
 *
* @file rm_status.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_RM_STATUS_H
#define SCHED_RM_STATUS_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInterface.h>
	

/** Get Shaping Profile status.
 * 
 *   @param[in]		hndl		Resource Manager handle.
 *   @param[in]		entry_ind	Profile index.
 *
 *   @return an integer .
 *   @retval  0   if resource free. 
 * 	 @retval  1   if resource is occupied.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF  if hndl is an invalid handle.
 *   @retval -EFAULT if entry_ind is out of range.
 */
int prvSchedRmShapingProfileStatus(rmctl_t hndl, uint32_t entry_ind);




/* useful macros for testing  resources */

#define TEST_RESOURCE_EXISTS(function, not_in_range_error, is_free_error) \
	switch(function)\
	{\
		case 1: rc=0;break;\
		case 0: rc=is_free_error;break;\
		default:rc=not_in_range_error;break;\
	}
#define TEST_RESOURCE_FREE(function, not_in_range_error, is_used_error) \
	switch(function)\
	{\
		case 1: rc=is_used_error;break;\
		case 0: rc=0;break;\
		default:rc=not_in_range_error;break;\
	}



#endif   /* SCHED_RM_STATUS_H */
