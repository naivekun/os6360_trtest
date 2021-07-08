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
 * @brief TM Tree interface.
 *
* @file tm_nodes_tree.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_NODES_TREE_H
#define SCHED_NODES_TREE_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>


/**  Change the tree DeQ status.
 *
 *   @param[in]     hndl        TM lib handle.
 *   @param[in]     status      Tree status.                   
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -EFAULT if status is out of range.
 *   @retval TM_HW_TREE_CONFIG_FAIL if download to HW fails.
 */
int prvCpssSchedNodesTreeChangeStatus(PRV_CPSS_SCHED_HANDLE hndl, uint8_t status);

/**  Retreieve the tree DeQ status.
 *
 *   @param[in]					hndl        TM lib handle.
 *   @param[in]     (pointer to)status      Tree DeQs tatus.                   
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -EFAULT if status is out of range.
 *   @retval TM_HW_TREE_CONFIG_FAIL if download to HW fails.
 */
int prvCpssSchedNodesTreeGetDeqStatus(PRV_CPSS_SCHED_HANDLE hndl, uint8_t * status);

/**  Change the tree DWRR priority.
 *
 *   @brief: set prios[i] = TM_DISABLE, if DWRR for prio i is disabled,
 *           set prios[i] = TM_ENABLE if DWRR for prio i is enabled
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     prios   	    Priority array pointer structure.                 
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval TM_HW_TREE_CONFIG_FAIL if download to HW fails.
 */
int prvCpssSchedNodesTreeSetDwrrPrio(PRV_CPSS_SCHED_HANDLE hndl, uint8_t *prios);

/**  Retrieve tree DWRR priority status.
 *
 *   @brief: prios[i] = TM_DISABLE, if DWRR for prio i is disabled,
 *           prios[i] = TM_ENABLE if DWRR for prio i is enabled
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     prios   	    Priority array pointer structure.                 
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval TM_HW_TREE_CONFIG_FAIL if download to HW fails.
 */
int prvCpssSchedNodesTreeGetDwrrPrioStatus(PRV_CPSS_SCHED_HANDLE hndl, uint8_t *prios);

#endif   /* TM_NODES_TREE_H */

