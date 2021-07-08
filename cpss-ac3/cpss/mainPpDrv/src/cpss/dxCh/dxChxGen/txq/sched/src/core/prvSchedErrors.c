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
 * @brief APIs for reading out the TM blocks error related information
 *
* @file tm_errors.c
*
* $Revision: 2.0 $
 */



#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrors.h>
#include <errno.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>




int prvSchedGetErrors(PRV_CPSS_SCHED_HANDLE hndl, struct tm_error_info *info)
{
    int rc;
    TM_CTL(ctl, hndl);
    rc = prvSchedLowLevelGetSchedErrors(ctl, info);
    if (rc)
	{
        rc = TM_HW_SCHED_GET_ERRORS_FAILED;
    }
    return rc;
}




int prvSchedGetErrorStatus
(   
    PRV_CPSS_SCHED_HANDLE hndl, 
    uint32_t * sched_status_ptr
)
{
    int rc;

    TM_CTL(ctl, hndl);

    rc = prvSchedLowLevelGetSchedStatus(ctl, sched_status_ptr);

    return rc;
}



