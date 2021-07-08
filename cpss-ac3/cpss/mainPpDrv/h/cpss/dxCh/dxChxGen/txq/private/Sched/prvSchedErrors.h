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
 * @brief TM Blocks Error Reading interface.
 *
* @file prvSchedErrors.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_ERRORS_H
#define SCHED_ERRORS_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>





/** Get Scheduler errors
 * @param[in]		hndl	TM lib handle
 * @param[out] 	    info    Scheduler error information 
 *
 * @return an integer return code.
 * @retval zero on success. 
 * @retval -EINVAL if hndl is NULL.
 * @retval -EBADF if hndl is invalid.
 * @retval TM_HW_SCHED_GET_ERRORS_FAILED if reading from HW fails
*/
int prvSchedGetErrors(PRV_CPSS_SCHED_HANDLE hndl, struct tm_error_info *info);



/** Get Qmgr, Drop, Sched, RCB Units error status
 * @param[in]		hndl	        TM lib handle
 * @param[out] 	    units_error_ptr Tm Units error status information 
 *
 * @return an integer return code.
 * @retval zero on success. 
 * @retval -EINVAL if hndl is NULL.
 * @retval -EBADF if hndl is invalid.
 * @retval -EFAULT if reading from HW fails
*/
int prvSchedGetErrorStatus
(   
    PRV_CPSS_SCHED_HANDLE hndl, 
    uint32_t * sched_status_ptr
);




#endif   /* TM_ERRORS_H */

