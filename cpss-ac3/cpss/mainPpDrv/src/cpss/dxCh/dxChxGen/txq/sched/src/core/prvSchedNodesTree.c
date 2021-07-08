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
 * @brief tm_nodes_tree implementation.
 *
* @file tm_nodes_tree.c
*
* $Revision: 2.0 $
 */



#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesTree.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <errno.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>


/**
 */
int prvCpssSchedNodesTreeChangeStatus(PRV_CPSS_SCHED_HANDLE hndl, uint8_t status)
{
	int rc;

    TM_CTL(ctl, hndl);
 

    if ((status != TM_ENABLE) && (status != TM_DISABLE))
	{
        rc = -EFAULT;
        goto out;
    }

    ctl->tree_deq_status = status;

    rc = prvSchedLowLevelSetTreeDequeueStatusToHw(hndl);
    if (rc < 0)
        rc = -TM_HW_TREE_CONFIG_FAIL;

out:
    return rc;
}

int prvCpssSchedNodesTreeGetDeqStatus(PRV_CPSS_SCHED_HANDLE hndl, uint8_t * status)
{
	int rc;

    TM_CTL(ctl, hndl);

	rc = prvSchedLowLevelGetTreeDequeueStatusFromHw(hndl);
    if (rc < 0)
	{
        rc = -TM_HW_TREE_CONFIG_FAIL;
		goto out;
	}
    *status = ctl->tree_deq_status;
out:
    return rc;
}

/**
 */
int prvCpssSchedNodesTreeSetDwrrPrio(PRV_CPSS_SCHED_HANDLE hndl, uint8_t * prios)
{
    int rc ;
    int i;

    TM_CTL(ctl, hndl);


    
    ctl->tree_dwrr_enable_per_priority_mask = 0;
    for (i=0; i<8; i++)
	{
        if ((prios[i] != TM_ENABLE) && (prios[i] != TM_DISABLE))
		{
            rc = -EFAULT;
            goto out;
        }
        ctl->tree_dwrr_enable_per_priority_mask = ctl->tree_dwrr_enable_per_priority_mask | (prios[i] << i);
    }

    rc = prvSchedLowLevelSetTreeDwrrPriorityStatusToHw(hndl);
    if (rc) rc = -TM_HW_TREE_CONFIG_FAIL;

out:
    if (rc < 0)	ctl->tree_dwrr_enable_per_priority_mask = 0;
    
    return rc;
}

int prvCpssSchedNodesTreeGetDwrrPrioStatus(PRV_CPSS_SCHED_HANDLE hndl, uint8_t *prios)
{
    int rc ;
    int i;

    TM_CTL(ctl, hndl);


    
    rc = prvSchedLowLevelGetTreeDwrrPriorityStatusFromHw(hndl);
    if (rc) rc = -TM_HW_TREE_CONFIG_FAIL;

    for (i=0; i<8; i++)
	{
        prios[i] = ( (ctl->tree_dwrr_enable_per_priority_mask & (1 << i)) !=0);
    }

    return rc;
}
