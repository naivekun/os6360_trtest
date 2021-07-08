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
 * @brief tm_nodes_status implementation.
 *
* @file tm_nodes_status.c
*
* $Revision: 2.0 $
 */



#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesStatus.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>


/**
 */
int prvCpssSchedReadStatusPort(PRV_CPSS_SCHED_HANDLE hndl,
                        uint8_t index,
                        struct tm_port_status *tm_status)
{
	int rc;

	TM_CTL(ctl, hndl);


	VALIDATE_NODE_ALLOCATED(port,index)
	if (rc) goto out;
	
	if (prvSchedLowLevelGetPortStatus(ctl, index, tm_status))	rc = -TM_HW_READ_PORT_STATUS_FAIL;

out:
    return rc;
}


/**
 */
int prvCpssSchedReadStatusCNode(PRV_CPSS_SCHED_HANDLE hndl,
                          uint32_t index,
                          struct schedNodeStatus *tm_status)
{
    int rc;

    TM_CTL(ctl, hndl);

	CONVERT_TO_PHYSICAL(C_LEVEL,index)   /** if supported - inside macro */
	


	VALIDATE_NODE_ALLOCATED(c_node,index)
	if (rc) goto out;
	
    if ( prvSchedLowLevelGetCNodeStatus(ctl, index, tm_status))  rc = -TM_HW_READ_C_NODE_STATUS_FAIL;
    
out:
    return rc;
}


/**
 */
int prvCpssSchedReadStatusBNode(PRV_CPSS_SCHED_HANDLE hndl,
                          uint32_t index,
                          struct schedNodeStatus *tm_status)
{
    int rc;

    TM_CTL(ctl, hndl);

	CONVERT_TO_PHYSICAL(B_LEVEL,index)   /** if supported - inside macro */
	


	VALIDATE_NODE_ALLOCATED(b_node,index)
	if (rc) goto out;
	
    if ( prvSchedLowLevelGetBNodeStatus(ctl, index, tm_status)) rc = -TM_HW_READ_B_NODE_STATUS_FAIL;

out:
    return rc;
}


/**
 */
int prvCpssSchedReadStatusANode(PRV_CPSS_SCHED_HANDLE hndl,
                          uint32_t index,
                          struct schedNodeStatus *tm_status)
{
    int rc;

    TM_CTL(ctl, hndl);
	
	CONVERT_TO_PHYSICAL(A_LEVEL,index)   /** if supported - inside macro */


	VALIDATE_NODE_ALLOCATED(a_node,index)
	if (rc) goto out;
	
    if ( prvSchedLowLevelGetANodeStatus(ctl, index, tm_status)) rc = -TM_HW_READ_A_NODE_STATUS_FAIL;

out:
    return rc;
}


/**
 */
int prvCpssSchedReadStatusQueue(PRV_CPSS_SCHED_HANDLE hndl,
                         uint32_t index,
                         struct schedNodeStatus *tm_status)
{
    int rc;

    TM_CTL(ctl, hndl);

	CONVERT_TO_PHYSICAL(Q_LEVEL,index)   /** if supported - inside macro */



	VALIDATE_NODE_ALLOCATED(queue,index)
	if (rc) goto out;
	
    if ( prvSchedLowLevelGetQueueStatus(ctl, index, tm_status)) rc = -TM_HW_READ_QUEUE_STATUS_FAIL;

out:
    return rc;
}



