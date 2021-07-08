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
 * @brief Resource Manager allocation functions implementation.
 *
* @file rm_alloc.c
*
* $Revision: 2.0 $
 */


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInternalTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmAlloc.h>

/** 
 */


/** 
 */ 
int prvCpssRmFindFreeShapingProfile(rmctl_t hndl)
{
    RM_HANDLE(ctl, hndl);
	return prvCpssRmGetFreeResource(&(ctl->rm_profiles[RM_SHAPING_PRF]));
}
/** 
 */









