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
 * @brief TM nodes control internal  utilities.
 *
* @file tm_nodes_ctl_internal.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_NODES_CTL_INTERNAL_H
#define SCHED_NODES_CTL_INTERNAL_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>


#define __VALIDATE_NODE_STATUS(node_type, node_index, desired_status, not_in_range_error, invalid_status_error) \
    if (node_index >= ctl->tm_total_##node_type##s) rc = not_in_range_error;\
    else if (ctl->tm_##node_type##_array[node_index].mapping.nodeStatus != desired_status ) rc = invalid_status_error;\
    else rc=0;

#define VALIDATE_NODE_FREE_EX(node_type, node_index, not_in_range_error, not_free_error) \
        __VALIDATE_NODE_STATUS(node_type, node_index, TM_NODE_FREE, not_in_range_error, not_free_error)

#define VALIDATE_NODE_ALLOCATED_EX(node_type, node_index, not_in_range_error, not_active_error) \
        __VALIDATE_NODE_STATUS(node_type, node_index, TM_NODE_USED, not_in_range_error, not_active_error)
        
#define VALIDATE_NODE_ALLOCATED(node_type, node_index)  VALIDATE_NODE_ALLOCATED_EX(node_type, node_index, -EFAULT, -ENODATA)


/***********************************************************************************************************************************************/

#define VALIDATE_NODE_QUANTUM_VALUE(value_in_chunks,error) \
    { if ((value_in_chunks * ctl->node_quantum_chunk_size_bytes < ctl->min_node_quantum_bytes) || (value_in_chunks * ctl->node_quantum_chunk_size_bytes > ctl->max_node_quantum_bytes))  rc = error ; else rc = 0 ;}

#define VALIDATE_PORT_QUANTUM_VALUE(value_in_chunks,error) \
    { if ((value_in_chunks * ctl->port_quantum_chunk_size_bytes < ctl->min_port_quantum_bytes) || (value_in_chunks * ctl->port_quantum_chunk_size_bytes > ctl->max_port_quantum_bytes))  rc = error ; else rc = 0; }


/** Verify ports bw EIR and CIR by its port speed
 * 
 *   @param[in]     port typw   port speed/type
 *   @param[in out] cir_bw_ptr  cir bw to verify 
 *   @param[in out] eir_bw_ptr  eir bw to verify 
 *  
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval TM_CONF_PORT_BW_OUT_OF_SPEED - on wrong port speed/type
 *   @retval TM_CONF_PORT_SPEED_OOR - if requested ports BWs > available speed
 */

/***********************************************************************************************************************************************/

/** return codes for allocate..() functions :
 *  -EFAULT - node not exists
 *  2       - zero range
 *  1       - free chunk of size range not found
 *  0       - OK
 */
/** return codes for free_allocated..() functions :
 *  -EFAULT     - node not exists
 *  -ENODATA    - node not used
 *  0           - OK
 */

#define NO_PREDEFINED_RANGE -1

int     prvSchedNodesCtlAllocateAsymPort(struct prvCpssDxChTxqSchedShadow *ctl,  uint8_t index, uint16_t children_range, uint16_t first_child_in_range);
int     prvSchedNodesCtlFreeAsymPort(struct prvCpssDxChTxqSchedShadow *ctl, uint32_t port_index);

int     prvSchedNodesCtlAllocateReservedCNode(struct prvCpssDxChTxqSchedShadow *ctl,  uint16_t index, uint8_t parent_index, uint16_t children_range, uint16_t first_child_in_range, tm_node_status_t * old_node_status);
int     prvSchedNodesCtlFreeCNode(struct prvCpssDxChTxqSchedShadow *ctl, uint32_t node_index, tm_node_status_t node_status);

int     prvSchedNodesCtlAllocateReservedBNode(struct prvCpssDxChTxqSchedShadow *ctl,  uint16_t  index, uint32_t parent_index, uint16_t children_range, uint16_t first_child_in_range, tm_node_status_t * old_node_status);
int     prvSchedNodesCtlFreeBNode(struct prvCpssDxChTxqSchedShadow *ctl,  uint32_t node_index, tm_node_status_t node_status);

int     prvSchedNodesCtlAllocateReservedANode(struct prvCpssDxChTxqSchedShadow *ctl,  uint16_t node_index, uint32_t parent_index, uint16_t children_range, uint16_t first_child_in_range, tm_node_status_t * old_node_status);
int     prvSchedNodesCtlFreeANode(struct prvCpssDxChTxqSchedShadow *ctl,  uint32_t node_index, tm_node_status_t node_status);

int     prvSchedNodesCtlAllocateReservedQueue(struct prvCpssDxChTxqSchedShadow *ctl,  uint16_t index, uint32_t parent_index, tm_node_status_t * old_node_status);
int     prvSchedNodesCtlFreeQueue(struct prvCpssDxChTxqSchedShadow *ctl,  uint32_t index, tm_node_status_t node_status);

#define COMPACT_DELETED_NODES

#define ENABLE_REDUCE_NODE_RANGE(node)  (node->mapping.childLo <  node->mapping.childHi)

int prvSchedDeleteANodeProc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index);
int prvSchedDeleteBNodeProc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index);
int prvSchedDeleteCNodeProc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index);

#ifdef VIRTUAL_LAYER

#else
    /** Empty list of reshuffling changes without lock - for internal use 
        *
        *   @param[in]     hndl            TM lib handle.
        *
        *   @return an integer return code.
        *   @retval zero on success.
        *   @retval -@EINVAL if hndl is NULL.
        *   @retval -@EBADF if hndl is invalid.
        */
    int prvSchedCleanReshafflingList(PRV_CPSS_SCHED_HANDLE hndl);
#endif

#endif   /* TM_NODES_CTL_INTERNAL_H */
