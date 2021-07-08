#ifndef SCHED_SET_HW_REGISTERS_H_
#define SCHED_SET_HW_REGISTERS_H_


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
 * @brief internal API for read/write  TM H/W 
 *
* @file prvSetHwRegisters.h
*
* $Revision: 2.0 $
 */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>

int prvSchedLowLevelSetPerLevelShapingParameters(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level);
int prvSchedLowLevelGetPerLevelShapingStatusFromHw(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, struct schedPerLevelPeriodicParams *periodic_params );


int prvSchedLowLevelSetDwrrLimit(PRV_CPSS_SCHED_HANDLE hndl);


int prvSchedLowLevelSetHwGenConf(PRV_CPSS_SCHED_HANDLE hndl);















int prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint32_t node_ind);
int prvSchedLowLevelSetNodeShapingParametersToHw(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint32_t node_ind, struct prvCpssDxChTxqSchedShapingProfile *profile);






int prvSchedLowLevelSetHwPeriodicScheme(PRV_CPSS_SCHED_HANDLE hndl);

int prvSchedLowLevelSetHwNodeMapping(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel lvl, uint32_t index);

int prvSchedLowLevelSetNodeEligPriorityFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel node_level, uint32_t node_index);

int prvSchedLowLevelDisableNodeEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel node_level, uint32_t node_index);

int prvSchedLowLevelSetQueueQuantum(PRV_CPSS_SCHED_HANDLE hndl, uint32_t queue_ind);
struct queue_hw_data_t
{
	uint16_t	parent_a_node;
    uint8_t		elig_prio_func_ptr;
    uint16_t	dwrr_quantum;

};
int prvSchedLowLeveGetQueueParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct queue_hw_data_t *queue_hw_data);


int prvSchedLowLevelSet_a_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind);

struct a_node_hw_data_t
{
	uint16_t	parent_b_node;
	uint16_t	first_child_queue;
	uint16_t	last_child_queue;
    uint8_t		elig_prio_func_ptr;
    uint16_t	dwrr_quantum;
	uint8_t		dwrr_priority;
    
};
int prvSchedLowLevelGetANodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct a_node_hw_data_t *a_node_hw_data);


int prvSchedLowLevelSet_b_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind);
struct b_node_hw_data_t
{
	uint16_t	parent_c_node;
	uint16_t	first_child_a_node;
	uint16_t	last_child_a_node;
    uint8_t		elig_prio_func_ptr;
    uint16_t	dwrr_quantum;
	uint8_t		dwrr_priority;
   
};
int prvSchedLowLevelGetBNodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct b_node_hw_data_t *b_node_hw_data);


int prvSchedLowLevelSet_c_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind);
struct c_node_hw_data_t
{
	uint16_t	parent_port;
	uint16_t	first_child_b_node;
	uint16_t	last_child_b_node;
    uint8_t		elig_prio_func_ptr;
    uint16_t	dwrr_quantum;
	uint8_t		dwrr_priority;
    
};
int prvSchedLowLevelGetCNodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct c_node_hw_data_t *c_node_hw_data);

int set_hw_port_elig_prio_func_ptr(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind);

int prvSchedLowLevelSetPortShapingParametersToHw(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind);

int prvSchedLowLevelSetPortScheduling(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind,GT_BOOL quantumOnly);



int prvSchedLowLevelSetPortAllParameters(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind);
struct port_hw_data_t
{
	uint16_t	first_child_c_node;
	uint16_t	last_child_c_node;
    uint8_t		elig_prio_func_ptr;
    uint16_t	dwrr_quantum[8];
	uint8_t		dwrr_priority;
};
int prvSchedLowLevelGetPortParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct port_hw_data_t *port_hw_data);


int prvSchedLowLevelSetTreeDequeueStatusToHw(PRV_CPSS_SCHED_HANDLE hndl);
int prvSchedLowLevelGetTreeDequeueStatusFromHw(PRV_CPSS_SCHED_HANDLE hndl);

int prvSchedLowLevelSetTreeDwrrPriorityStatusToHw(PRV_CPSS_SCHED_HANDLE hndl);
int prvSchedLowLevelGetTreeDwrrPriorityStatusFromHw(PRV_CPSS_SCHED_HANDLE hndl);


int prvSchedLowLevelSetDefaultPortMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_ports);

int prvSchedLowLevelSetDefaultCnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_c_nodes);

int prvSchedLowLevelSetDefaultBnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_b_nodes);

int prvSchedLowLevelSetDefaultAnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_a_nodes);

int prvSchedLowLevelSetDefaultQnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_q_nodes);


int prvSchedLowLevelDisablePortEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_ports);

int prvSchedLowLevelSetQLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl);

int prvSchedLowLevelSetALevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl);

int prvSchedLowLevelSetBLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl);

int prvSchedLowLevelSetCLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl);

int prvSchedLowLevelSetPLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl);

int prvSchedLowLevelSetQLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset);

int prvSchedLowLevelSetALevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset);

int prvSchedLowLevelSetBLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset);

int prvSchedLowLevelSetCLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset);

int prvSchedLowLevelSetPLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset);

int prvSchedLowLevelClearPLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint8_t index);

int prvSchedLowLevelClearCLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index);

int prvSchedLowLevelClearBLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index);

int prvSchedLowLevelClearALevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index);

int prvSchedLowLevelClearQLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index);













/*********************************************************************************/

int prvSchedLowLevelGetGeneralHwParams(PRV_CPSS_SCHED_HANDLE hndl);


int prvSchedLowLevelGetPortStatus(PRV_CPSS_SCHED_HANDLE hndl,
				     uint8_t index,
				     struct tm_port_status *tm_status);

int prvSchedLowLevelGetCNodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
				       uint32_t index,
				       struct schedNodeStatus *tm_status);

int prvSchedLowLevelGetBNodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
				       uint32_t index,
				       struct schedNodeStatus *tm_status);

int prvSchedLowLevelGetANodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
				       uint32_t index,
				       struct schedNodeStatus *tm_status);

int prvSchedLowLevelGetQueueStatus(PRV_CPSS_SCHED_HANDLE hndl,
				       uint32_t index,
				       struct schedNodeStatus *tm_status);




int prvSchedLowLevelDisableAlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_a_nodes);


int prvSchedLowLevelDisableBlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_b_nodes);

int prvSchedLowLevelDisableClevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_c_nodes);


int prvSchedLowLevelDisableQlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_q_nodes);


int prvSchedLowLevelGetSchedErrors(PRV_CPSS_SCHED_HANDLE hndl, struct tm_error_info *info);


int prvSchedLowLevelGetSchedStatus
(
    PRV_CPSS_SCHED_HANDLE hndl, 
    uint32_t * sched_status_ptr
);


/*
 in order to process 64 bit counters on OS without 64 bit support
 following functions assumes that void * counter_address is address of 8 bytes of memory
 where 64 bits of counter will be copied.
*/






/* common macros for  set_hw_registers.c & set_hw_registers_platform.c */
#define NODE_VALIDATION(nodes_count)	\
	if (node_ind >= nodes_count) rc = -EFAULT; 



int prvSchedLowLevelGetNodeEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint16_t func_offset, struct tm_elig_prio_func_node *params);
int prvSchedLowLevelGetQueueEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset, struct tm_elig_prio_func_queue *params);


int prvSchedLowLevelGetNodeShapingParametersFromHw
(   
    PRV_CPSS_SCHED_HANDLE hndl, 
    enum schedLevel level, 
    uint32_t node_ind, 
    struct prvCpssDxChTxqSchedShapingProfile *profile
 );


#endif /* _SET_HW_REGISTERS_H_ */
