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
 * @brief Resource Manager status functions implementation.
 *
* @file rm_status.c
*
* $Revision: 2.0 $
 */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInternalTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmStatus.h>





#define TEST_RESOURCE_STATUS(resource_type) \
	switch (prvCpssRmResourceStatus(&(ctl->rm_profiles[resource_type]), entry_ind))\
	{\
		case  -1: return -EFAULT;\
		case   0: return 0;\
		default: return 1;\
	}\
/** 
 */
int prvSchedRmShapingProfileStatus(rmctl_t hndl, uint32_t entry_ind)
{
    RM_HANDLE(ctl, hndl);
	TEST_RESOURCE_STATUS(RM_SHAPING_PRF);
}
