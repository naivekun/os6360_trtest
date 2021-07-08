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
 * @brief TM nodes creation interface.
 *
* @file tm_nodes_create.h
*
* $Revision: 2.0 $
 */

#ifndef   	SCHED_NODES_CREATE_H
#define   	SCHED_NODES_CREATE_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>


/***************************************************************************
 * Port Creation 
 ***************************************************************************/




/** Create Port with assymetric sub-tree and download its parameters to HW.
 *
 *   @param[in]     hndl            TM lib handle. 
 *   @param[in]     port_index      Port index.       
 *   @param[in]     params          Port parameters structure pointer.
 *
 *   @note: to indicate that no shaping is needed for port, set
 *   params->cir_bw to TM_INVAL, in this case other shaping
 *   parameters will not be considered.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ENOMEM if memory allocation in RM fails.
 *   @retval -EACCES if number of children nodes is not valid.
 *   @retval TM_CONF_PORT_IND_OOR if port_index is out of range.
 *   @retval TM_CONF_PORT_IND_USED if port_index is already used.
 *
 *   @retval TM_CONF_P_WRED_PROF_REF_OOR.
 *   @retval TM_CONF_PORT_QUANTUM_OOR.
 *   @retval TM_CONF_PORT_DWRR_PRIO_OOR.
 *   @retval TM_CONF_INVALID_NUM_OF_C_NODES.
 *   @retval TM_CONF_ELIG_PRIO_FUNC_ID_OOR if eligible function id is oor
 *
 *   @retval TM_CONF_PORT_BS_OOR   if cbs or ebs is greater than H/W limit    
 *   @retval TM_BW_OUT_OF_RANGE  if  failed to configure desired b/w
 *   @retval TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE	desired cbs or ebs value is too small for desired cir/eir
 *   @retval TM_BW_UNDERFLOW - if cir & eir are too different to configure together
 *   @retval TM_HW_PORT_CONFIG_FAIL if download to HW fails.
 */
int prvCpssSchedNodesCreateAsymPort
(
 IN PRV_CPSS_SCHED_HANDLE hndl, 
 IN uint8_t port_index,
 IN struct tm_port_params *params,
 IN  GT_BOOL                  fixedMappingToCNode
);

/** Create path from Queue to A-node.
 *
 *  @note field shaping_profile_ref in parameters may get
 *  TM_INF_PROFILE value when there is no shaping to the queue/node.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     a_node_index    A-node index to connect queue.      
 *   @param[in]     q_params        Queue parameters structure pointer.        
 *   @param[out]    queue_index     Queue index pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ENOBUFS if no free Queues under the A-node can be added.
 *   @retval TM_CONF_A_NODE_IND_OOR if a_node_index is out of range.
 *   @retval TM_CONF_A_NODE_IND_NOT_EXIST if a_node_index is not in use.
 *
 *   @retval TM_CONF_ELIG_PRIO_FUNC_ID_OOR if eligible function id is oor
 *
 *   @retval TM_CONF_Q_SHAPING_PROF_REF_OOR.
 * 	 @retval TM_WRONG_SHP_PROFILE_LEVEL.
 *   @retval TM_CONF_Q_PRIORITY_OOR.
 *   @retval TM_CONF_Q_QUANTUM_OOR.
 *   @retval TM_CONF_Q_WRED_PROF_REF_OOR.
 *
 *   @retval TM_HW_QUEUE_CONFIG_FAIL if queue download to HW fails.
 */
int prvCpssSchedNodesCreateQueueToANode (PRV_CPSS_SCHED_HANDLE hndl, uint32_t a_node_index,
                               struct tm_queue_params *q_params,
                               uint32_t *queue_index);


/***************************************************************************
 * A-node Creation 
 ***************************************************************************/

/** Create path from A-node to Port.
 *
 *  @note field shaping_profile_ref in parameters may get
 *  TM_INF_PROFILE value when there is no shaping to the queue/node.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     port_index      Port index to connect queue.      
 *   @param[in]     a_params        A-node parameters structure pointer.      
 *   @param[in]     b_params        B-node parameters structure pointer.      
 *   @param[in]     c_params        C-node parameters structure pointer.
 *   @param[out]    a_node_index    A-node index pointer.
 *   @param[out]    b_node_index    B-node index pointer.
 *   @param[out]    c_node_index    C-node index pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ENOBUFS if no free C-nodes under the Port can be added.
 *   @retval TM_CONF_PORT_IND_OOR if port_index is out of range.
 *   @retval TM_CONF_PORT_IND_NOT_EXIST if port_index is not in use.
 *
 *   @retval TM_CONF_A_SHAPING_PROF_REF_OOR.
 * 	 @retval TM_WRONG_SHP_PROFILE_LEVEL.
 *   @retval TM_CONF_A_PRIORITY_OOR.
 *   @retval TM_CONF_A_QUANTUM_OOR.
 *   @retval TM_CONF_A_DWRR_PRIO_OOR.
 *   @retval TM_CONF_A_WRED_PROF_REF_OOR.
 *   @retval TM_CONF_B_SHAPING_PROF_REF_OOR.
 *   @retval TM_CONF_B_PRIORITY_OOR.
 *   @retval TM_CONF_B_QUANTUM_OOR.
 *   @retval TM_CONF_B_DWRR_PRIO_OOR.
 *   @retval TM_CONF_B_WRED_PROF_REF_OOR.
 *   @retval TM_CONF_C_SHAPING_PROF_REF_OOR.
 *   @retval TM_CONF_C_PRIORITY_OOR.
 *   @retval TM_CONF_C_QUANTUM_OOR.
 *   @retval TM_CONF_C_DWRR_PRIO_OOR.
 *   @retval TM_CONF_C_WRED_PROF_REF_OOR.
 *   @retval TM_CONF_INVALID_NUM_OF_QUEUES.
 *
 *   @retval TM_HW_A_NODE_CONFIG_FAIL if A-node download to HW fails.
 *   @retval TM_HW_B_NODE_CONFIG_FAIL if B-node download to HW fails.
 *   @retval TM_HW_C_NODE_CONFIG_FAIL if C-node download to HW fails.
 */
int prvCpssSchedNodesCreateANodeToPort 
(
        PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_index,
        struct tm_a_node_params *a_params,
        GT_BOOL explicitQueueStart,
        GT_U32  queueStart,
        GT_BOOL fixedPortToBnodeMapping,
        struct tm_b_node_params *b_params,
        struct tm_c_node_params *c_params,
         uint32_t *a_node_index_ptr,
         uint32_t *b_node_index_ptr,
          uint32_t *c_node_index_ptr,
         GT_BOOL addToExisting
);





/** Create path from A-node to B-node.
 *
 *  @note field shaping_profile_ref in parameters may get
 *  TM_INF_PROFILE value when there is no shaping to the queue/node.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     b_node_index    B-node index to connect queue.      
 *   @param[in]     a_params        A-node parameters structure pointer.      
 *   @param[out]    a_node_index    A-node index pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ENOBUFS if no free A-nodes under the B-node can be added.
 *   @retval TM_CONF_B_NODE_IND_OOR if b_node_index is out of range.
 *   @retval TM_CONF_B_NODE_IND_NOT_EXIST if b_node_index is not in use.
 *
 *   @retval TM_CONF_A_SHAPING_PROF_REF_OOR.
 * 	 @retval TM_WRONG_SHP_PROFILE_LEVEL.
 *   @retval TM_CONF_A_PRIORITY_OOR.
 *   @retval TM_CONF_A_QUANTUM_OOR.
 *   @retval TM_CONF_A_DWRR_PRIO_OOR.
 *   @retval TM_CONF_A_WRED_PROF_REF_OOR.
 *
 *   @retval TM_CONF_INVALID_NUM_OF_QUEUES.
 *   @retval TM_CONF_ELIG_PRIO_FUNC_ID_OOR if eligible function id is oor
 *
 *   @retval TM_HW_A_NODE_CONFIG_FAIL if A-node download to HW fails.
 */
int prvCpssSchedNodesCreateANodeToBNode (PRV_CPSS_SCHED_HANDLE hndl, uint32_t parent_b_node_index,
                               struct tm_a_node_params *a_params,
                               uint32_t *a_node_index_ptr,
                               GT_BOOL explicitQueueStart,
                               GT_U32  queueStart);





/***************************************************************************
 * B-node Creation 
 ***************************************************************************/

/** Create path from B-node to Port.
 *
 *  @note field shaping_profile_ref in parameters may get
 *  TM_INF_PROFILE value when there is no shaping to the queue/node.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     port_index      Port index to connect queue.      
 *   @param[in]     b_params        B-node parameters structure pointer.      
 *   @param[in]     c_params        C-node parameters structure pointer.
 *   @param[out]    b_node_index    B-node index pointer.
 *   @param[out]    c_node_index    C-node index pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ENOBUFS if no free C-nodes under the Port can be added.
 *   @retval TM_CONF_PORT_IND_OOR if port_index is out of range.
 *   @retval TM_CONF_PORT_IND_NOT_EXIST if port_index is not in use.
 *
 *   @retval TM_CONF_B_SHAPING_PROF_REF_OOR.
 * 	 @retval TM_WRONG_SHP_PROFILE_LEVEL.
 *   @retval TM_CONF_B_PRIORITY_OOR.
 *   @retval TM_CONF_B_QUANTUM_OOR.
 *   @retval TM_CONF_B_DWRR_PRIO_OOR.
 *   @retval TM_CONF_B_WRED_PROF_REF_OOR.
 *   @retval TM_CONF_C_SHAPING_PROF_REF_OOR.
 *   @retval TM_CONF_C_PRIORITY_OOR.
 *   @retval TM_CONF_C_QUANTUM_OOR.
 *   @retval TM_CONF_C_DWRR_PRIO_OOR.
 *   @retval TM_CONF_C_WRED_PROF_REF_OOR.
 *   @retval TM_CONF_INVALID_NUM_OF_A_NODES.
 *
 *   @retval TM_HW_B_NODE_CONFIG_FAIL if B-node download to HW fails.
 *   @retval TM_HW_C_NODE_CONFIG_FAIL if C-node download to HW fails.
 */
int prvCpssSchedNodesCreateBNodeToPort 
(   
        PRV_CPSS_SCHED_HANDLE    hndl, 
        uint8_t port_index,
        struct tm_b_node_params *b_params,
        struct tm_c_node_params *c_params,
        GT_BOOL fixedBnodeToCnodeMapping,
         uint32_t *b_node_index_ptr,
         uint32_t *c_node_index_ptr
 );


/** Create path from B-node to C-node.
 *
 *  @note field shaping_profile_ref in parameters may get
 *  TM_INF_PROFILE value when there is no shaping to the queue/node.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     c_node_index    C-node index to connect queue.      
 *   @param[in]     b_params        B-node parameters structure pointer.      
 *   @param[out]    b_node_index    B-node index pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ENOBUFS if no free B-nodes under the C-node can be added.
 *   @retval TM_CONF_C_NODE_IND_OOR if c_node_index is out of range.
 *   @retval TM_CONF_C_NODE_IND_NOT_EXIST if c_node_index is not in use.
 *
 *   @retval TM_CONF_B_SHAPING_PROF_REF_OOR.
 * 	 @retval TM_WRONG_SHP_PROFILE_LEVEL.
 *   @retval TM_CONF_B_PRIORITY_OOR.
 *   @retval TM_CONF_B_QUANTUM_OOR.
 *   @retval TM_CONF_B_DWRR_PRIO_OOR.
 *   @retval TM_CONF_B_WRED_PROF_REF_OOR.
 *
 *   @retval TM_CONF_INVALID_NUM_OF_A_NODES.
 *   @retval TM_CONF_ELIG_PRIO_FUNC_ID_OOR if eligible function id is oor
 *
 *   @retval TM_HW_B_NODE_CONFIG_FAIL if B-node download to HW fails.
 */
int prvCpssSchedNodesCreateBNodeToCNode (PRV_CPSS_SCHED_HANDLE hndl, uint32_t c_node_index,
                                struct tm_b_node_params *b_params,
                                uint32_t *b_node_index);


/***************************************************************************
 * C-node Creation 
 ***************************************************************************/

/** Create path from C-node to Port.
 *
 *  @note field shaping_profile_ref in parameters may get
 *  TM_INF_PROFILE value when there is no shaping to the queue/node.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     port_index      Port index to connect queue.      
 *   @param[in]     c_params        C-node parameters structure pointer.
 *   @param[out]    c_node_index    C-node index pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ENOBUFS if no free C-nodes under the Port can be added.
 *   @retval TM_CONF_PORT_IND_OOR if port_index is out of range.
 *   @retval TM_CONF_PORT_IND_NOT_EXIST if port_index is not in use.
 *
 *   @retval TM_CONF_C_SHAPING_PROF_REF_OOR.
 * 	 @retval TM_WRONG_SHP_PROFILE_LEVEL.
 *   @retval TM_CONF_C_PRIORITY_OOR.
 *   @retval TM_CONF_C_QUANTUM_OOR.
 *   @retval TM_CONF_C_DWRR_PRIO_OOR.
 *   @retval TM_CONF_B_WRED_PROF_REF_OOR.
 *
 *   @retval TM_CONF_INVALID_NUM_OF_B_NODES.
 *   @retval TM_CONF_ELIG_PRIO_FUNC_ID_OOR if eligible function id is
 *   oor
 *
 *   @retval TM_HW_C_NODE_CONFIG_FAIL if C-node download to HW fails.
 */
int prvCpssSchedNodesCreateCNodeToPort 
(
        PRV_CPSS_SCHED_HANDLE hndl, 
        uint8_t parent_port_index,
        GT_BOOL fixedBnodeToCnodeMapping,
        struct tm_c_node_params *c_params,
        uint32_t *c_node_index_ptr
 );



#endif 	 /* TM_NODES_CREATE_H */

