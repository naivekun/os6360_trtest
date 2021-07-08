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
 * @brief tm_nodes_read implementation.
 *
* @file tm_nodes_read.c
*
* $Revision: 2.0 $
 */

#include <stdlib.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesRead.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>



/***************************************************************************
 * Read Configuration 
 ***************************************************************************/

/**
 */
int prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t queue_index,
                                struct tm_queue_params *params)
{
    struct tm_queue *node = NULL;
    struct queue_hw_data_t queue_hw_data;
    int rc;

    TM_CTL(ctl, hndl)

    CONVERT_TO_PHYSICAL(Q_LEVEL,queue_index) /** if supported - inside macro */
    


    /* Check parameters validity */
    VALIDATE_NODE_ALLOCATED(queue,queue_index)
    if (rc) goto out;
    
    node = &(ctl->tm_queue_array[queue_index]);
/*
    params->shaping_profile_ref = node->shaping_profile_ref;
    params->quantum = node->dwrr_quantum;
    params->wred_profile_ref = node->wred_profile_ref;
    params->elig_prio_func_ptr = node->elig_prio_func;
*/
    rc = prvSchedLowLeveGetQueueParams(hndl, queue_index, &queue_hw_data);
    if (rc) goto out;
    params->shaping_profile_ref = node->shaping_profile_ref;
    params->quantum = queue_hw_data.dwrr_quantum;
    params->elig_prio_func_ptr = queue_hw_data.elig_prio_func_ptr;


out:
    return rc;
}


/**
 */
int prvCpssSchedNodesReadANodeConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_index,
                                 struct tm_a_node_params *params)
{
    struct tm_a_node *node = NULL;
    struct a_node_hw_data_t a_node_hw_data;
    int rc = 0;
    int i;

    TM_CTL(ctl, hndl);
    
    CONVERT_TO_PHYSICAL(A_LEVEL,node_index)    /** if supported - inside macro */



    /* Check parameters validity */
    VALIDATE_NODE_ALLOCATED(a_node,node_index)
    if (rc) goto out;
        
    node = &(ctl->tm_a_node_array[node_index]);
   
 
    rc =    prvSchedLowLevelGetANodeParams(hndl, node_index, &a_node_hw_data);
    if (rc) goto out;
        
    params->shaping_profile_ref = node->shaping_profile_ref;

    params->quantum = a_node_hw_data.dwrr_quantum;
    for (i=0; i<8; i++)
    {
        if ((a_node_hw_data.dwrr_priority & (1 << i)) != 0)
            params->dwrr_priority[i] = TM_ENABLE;
        else
            params->dwrr_priority[i] = TM_DISABLE;
    }
    
    params->elig_prio_func_ptr = a_node_hw_data.elig_prio_func_ptr;
    params->num_of_children = a_node_hw_data.last_child_queue - a_node_hw_data.first_child_queue + 1;
out:
    return rc;
}


/**
 */
int prvCpssSchedNodesReadBNodeConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_index,
                                 struct tm_b_node_params *params)
{
    struct tm_b_node *node = NULL;
    struct b_node_hw_data_t b_node_hw_data;
    int rc = 0;
    int i;

    TM_CTL(ctl, hndl)

    CONVERT_TO_PHYSICAL(B_LEVEL,node_index) /** if supported - inside macro */
    

    /* Check parameters validity */
    VALIDATE_NODE_ALLOCATED(b_node,node_index)
    if (rc) goto out;
    
    node = &(ctl->tm_b_node_array[node_index]);
    
    rc =    prvSchedLowLevelGetBNodeParams(hndl, node_index, &b_node_hw_data);
    if (rc) goto out;
    
    params->shaping_profile_ref = node->shaping_profile_ref;
    params->quantum = b_node_hw_data.dwrr_quantum;
    for (i=0; i<8; i++)
    {
        if ((b_node_hw_data.dwrr_priority & (1 << i)) != 0)
            params->dwrr_priority[i] = TM_ENABLE;
        else
            params->dwrr_priority[i] = TM_DISABLE;
    }
    
    params->elig_prio_func_ptr = b_node_hw_data.elig_prio_func_ptr;
    params->num_of_children = b_node_hw_data.last_child_a_node -b_node_hw_data.first_child_a_node + 1;
out:
    return rc;
}


/**
 */
int prvCpssSchedNodesReadCNodeConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_index,
                                 struct tm_c_node_params *params)
{
    struct tm_c_node *node = NULL;
    struct c_node_hw_data_t c_node_hw_data;
    int rc = 0;
    int i;

    TM_CTL(ctl, hndl)

    CONVERT_TO_PHYSICAL(C_LEVEL,node_index)   /** if supported - inside macro */


    /* Check parameters validity */
    VALIDATE_NODE_ALLOCATED(c_node,node_index)
    if (rc) goto out;
    
    node = &(ctl->tm_c_node_array[node_index]);
    
    rc = prvSchedLowLevelGetCNodeParams(hndl, node_index,&c_node_hw_data);
    if (rc) goto out;

    params->shaping_profile_ref = node->shaping_profile_ref;

    params->quantum = c_node_hw_data.dwrr_quantum;
    for (i=0; i<8; i++)
        if ((c_node_hw_data.dwrr_priority & (1 << i)) != 0)
            params->dwrr_priority[i] = TM_ENABLE;
        else
            params->dwrr_priority[i] = TM_DISABLE;
/*   params->wred_cos = c_node_hw_data.wred_cos; */
   

    params->elig_prio_func_ptr = c_node_hw_data.elig_prio_func_ptr;
    params->num_of_children = c_node_hw_data.last_child_b_node - c_node_hw_data.first_child_b_node + 1;
out:
    return rc;
}


/**
 */
int prvCpssSchedNodesReadPortConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t port_index,
                               struct tm_port_params *params)                               
{
    struct tm_port *port = NULL;
     struct port_hw_data_t port_hw_data;
    int rc = 0;
    int i;

    TM_CTL(ctl, hndl)



    /* Check parameters validity */
    VALIDATE_NODE_ALLOCATED(port,port_index)
    if (rc) goto out;
    
    port = &(ctl->tm_port_array[port_index]);

    rc = prvSchedLowLevelGetPortParams(hndl, port_index, &port_hw_data);
    if (rc) goto out;

    /*read from SW*/
    params->cir_bw = port->cir_sw;
    params->eir_bw = port->eir_sw;
    
    params->cbs = port->cir_burst_size ;
    params->ebs = port->eir_burst_size ;

    /* read from H/W */

    for (i = 0; i < 8; i++)
    {
        if ((port_hw_data.dwrr_priority & (1 << i)) != 0)
            params->dwrr_priority[i] = TM_ENABLE;
        else
            params->dwrr_priority[i] = TM_DISABLE;
        params->quantum[i] = port_hw_data.dwrr_quantum[i];
    }

    params->elig_prio_func_ptr = port_hw_data.elig_prio_func_ptr;
    params->num_of_children = port_hw_data.last_child_c_node - port_hw_data.first_child_c_node + 1;

out:
    return rc;
}


#define FLAGGED_ELIG_FUN(node) node->elig_prio_func + 64*node->node_temporary_disabled

int prvCpssSchedNodesReadTraversePortTreeSw(PRV_CPSS_SCHED_HANDLE hndl, unsigned int port_index, SCHED_PORT_TRAVERSE_TREE_FUN dump_fun, void * vpEnv,  void* vpUserData)
{
    int ic, ib, ia, iq;
    struct tm_port *        port_ptr ;
    struct tm_c_node *  c_node_ptr;
    struct tm_b_node *  b_node_ptr;
    struct tm_a_node *  a_node_ptr;
    struct tm_queue *       queue_ptr;
    int result;

    TM_CTL(ctl, hndl);

    port_ptr = &(ctl->tm_port_array[port_index]);
    result = dump_fun(1, vpEnv, P_LEVEL, port_index, port_ptr->mapping.nodeStatus, 0, FLAGGED_ELIG_FUN(port_ptr), vpUserData);
    if (result) return result;
    /* safety check - only USED node have valid childern range */
    if (port_ptr->mapping.nodeStatus == TM_NODE_USED)
    {
        for (ic = port_ptr->mapping.childLo ; ic <= port_ptr->mapping.childHi ; ic++)
        {
            c_node_ptr=&(ctl->tm_c_node_array[ic]);
            result = dump_fun(1, vpEnv, C_LEVEL, ic, c_node_ptr->mapping.nodeStatus, c_node_ptr->mapping.nodeParent, FLAGGED_ELIG_FUN(c_node_ptr), vpUserData);     
            if (result == -1 )  continue ; else if (result > 0 ) return result;
            /* safety check - only USED node have valid childern range */
            if (c_node_ptr->mapping.nodeStatus == TM_NODE_USED)
            {
                for (ib = c_node_ptr->mapping.childLo ; ib <= c_node_ptr->mapping.childHi ; ib++)
                {
                    b_node_ptr=&(ctl->tm_b_node_array[ib]);
                    result = dump_fun(1, vpEnv, B_LEVEL, ib, b_node_ptr->mapping.nodeStatus, b_node_ptr->mapping.nodeParent, FLAGGED_ELIG_FUN(b_node_ptr), vpUserData);
                    if (result == -1 )  continue ; else if (result > 0 ) return result;
                    /* safety check - only USED node have valid childern range */
                    if (b_node_ptr->mapping.nodeStatus == TM_NODE_USED)
                    {                   
                        for (ia = b_node_ptr->mapping.childLo ; ia <= b_node_ptr->mapping.childHi ; ia++)
                        {
                            a_node_ptr=&(ctl->tm_a_node_array[ia]);
                            result = dump_fun(1, vpEnv, A_LEVEL, ia, a_node_ptr->mapping.nodeStatus, a_node_ptr->mapping.nodeParent, FLAGGED_ELIG_FUN(a_node_ptr),vpUserData);
                            if (result == -1 )  continue ; else if (result > 0 ) return result;
                            /* safety check - only USED node have valid childern range */
                            if (a_node_ptr->mapping.nodeStatus == TM_NODE_USED)
                            {
                                for (iq = a_node_ptr->mapping.childLo ; iq <= a_node_ptr->mapping.childHi ; iq++)
                                {
                                    queue_ptr=&(ctl->tm_queue_array[iq]);
                                    result = dump_fun(1, vpEnv, Q_LEVEL, iq, queue_ptr->dwrr_quantum, queue_ptr->mapping.nodeParent,  FLAGGED_ELIG_FUN(queue_ptr),vpUserData);
                                    if (result > 0 ) return result;
                                }
                            }
                            result = dump_fun(0, vpEnv, A_LEVEL, ia, a_node_ptr->mapping.nodeStatus, a_node_ptr->mapping.nodeParent, FLAGGED_ELIG_FUN(a_node_ptr),vpUserData);
                            if (result > 0 ) return result;
                        }
                    }
                    result = dump_fun(0,  vpEnv, B_LEVEL, ib, b_node_ptr->mapping.nodeStatus, b_node_ptr->mapping.nodeParent, FLAGGED_ELIG_FUN(b_node_ptr), vpUserData);
                    if (result == -1 )  continue ; else if (result > 0 ) return result;
                }
            }
            result = dump_fun(0, vpEnv, C_LEVEL, ic, c_node_ptr->mapping.nodeStatus, c_node_ptr->mapping.nodeParent, FLAGGED_ELIG_FUN(c_node_ptr), vpUserData);
            if (result > 0 ) return result;
        }
    }
    result = dump_fun(0, vpEnv, P_LEVEL, port_index, port_ptr->mapping.nodeStatus, 0, FLAGGED_ELIG_FUN(port_ptr) , vpUserData);
    if (result) return result;
    return 0;
}


#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedSetHwRegistersImp.h>




int prvCpssSchedNodesReadTraversePortTreeHw(PRV_CPSS_SCHED_HANDLE hndl, unsigned int port_index, SCHED_PORT_TRAVERSE_TREE_FUN dump_fun, void * vpEnv,  void* vpUserData)
{
    
    int portChildLo=0,portChildHi=0, portEf=0;
    int cnodeChildLo=0,cnodeChildHi=0, cnodeParent=0, cnodeEf=0;
    int bnodeChildLo=0,bnodeChildHi=0, bnodeParent=0, bnodeEf=0;
    int anodeChildLo=0,anodeChildHi=0, anodeParent=0, anodeEf=0;
    int queueParent=0, queueEf=0,dwrr_quantum=0;
    
    struct tm_port *        port_ptr ;
    struct tm_c_node *  c_node_ptr;
    struct tm_b_node *  b_node_ptr;
    struct tm_a_node *  a_node_ptr;
    
    int ic, ib, ia, iq;
    int result;
    int rc =  -EFAULT;
    

    TM_REGISTER_VAR(TM_Sched_PortRangeMap)
    TM_REGISTER_VAR(TM_Sched_ClvltoPortAndBlvlRangeMap)
    TM_REGISTER_VAR(TM_Sched_BLvltoClvlAndAlvlRangeMap)
    TM_REGISTER_VAR(TM_Sched_ALvltoBlvlAndQueueRangeMap)
    TM_REGISTER_VAR(TM_Sched_QueueAMap)
    TM_REGISTER_VAR(TM_Sched_QueueQuantum)
    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosLo)
    TM_REGISTER_VAR(TM_Sched_AlvlQuantum)

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_PortEligPrioFuncPtr)

    TM_CTL(ctl, hndl)
   

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortRangeMap , port_index, TM_Sched_PortRangeMap)
    if (rc) return -1;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortRangeMap , Lo , portChildLo , (int) )
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortRangeMap , Hi , portChildHi, (int) )
    
    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFuncPtr , port_index , TM_Sched_PortEligPrioFuncPtr)
    if (rc) return -1;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortEligPrioFuncPtr , Ptr , portEf, (uint8_t))

    /*read only priority 0*/
    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortQuantumsPriosLo, port_index, TM_Sched_PortQuantumsPriosLo);
    if (rc)
    return rc;
   TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosLo, Quantum0,dwrr_quantum ,(uint16_t)); 
    
    port_ptr = &(ctl->tm_port_array[port_index]);
    result = dump_fun(1, vpEnv, P_LEVEL, port_index, dwrr_quantum, 0, portEf , vpUserData);
    if (result) return result;
    /* safety check - continue only if node is enabled*/
    if (port_ptr->mapping.nodeStatus == TM_NODE_USED)
    {
        for (ic = portChildLo ; ic <= portChildHi ; ic++)
        {   
     
           TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ClvltoPortAndBlvlRangeMap , ic , TM_Sched_ClvltoPortAndBlvlRangeMap)
                if (rc) return -1;
                TXQ_SCHED_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap , Port , cnodeParent, (int) )
                TXQ_SCHED_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap , BlvlLo , cnodeChildLo, (int)  )
                TXQ_SCHED_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap , BlvlHi , cnodeChildHi, (int) )                                 
            

            TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFuncPtr , ic , TM_Sched_ClvlEligPrioFuncPtr)
            if (rc) return -1;
            TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlEligPrioFuncPtr , Ptr , cnodeEf, (uint8_t))

            c_node_ptr=&(ctl->tm_c_node_array[ic]);
            
            result = dump_fun(1, vpEnv, C_LEVEL, ic, c_node_ptr->mapping.nodeStatus, cnodeParent, cnodeEf, vpUserData);
            if (result == -1 )  continue ; else if (result > 0 ) return result;
            /* safety check - only USED node have valid childern range */
            if (cnodeEf != TM_NODE_DISABLED_FUN) 
            {

                for (ib = cnodeChildLo ; ib <= cnodeChildHi ; ib++)
                {
                    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap , ib , TM_Sched_BLvltoClvlAndAlvlRangeMap)
                    if (rc) return -1;
                    TXQ_SCHED_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap , Clvl , bnodeParent ,(int))
                    TXQ_SCHED_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap , AlvlLo , bnodeChildLo,(int))
                    TXQ_SCHED_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap , AlvlHi , bnodeChildHi,(int))
                    
                    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFuncPtr , ib , TM_Sched_BlvlEligPrioFuncPtr)
                    if (rc) return -1;
                    TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlEligPrioFuncPtr , Ptr , bnodeEf,(uint8_t))

                    b_node_ptr=&(ctl->tm_b_node_array[ib]);

                    result = dump_fun(1, vpEnv, B_LEVEL, ib, b_node_ptr->mapping.nodeStatus, bnodeParent, bnodeEf, vpUserData);
                    if (result == -1 )  continue ; else if (result > 0 ) return result;

                    /* safety check - only USED node have valid childern range */
                    if (bnodeEf != TM_NODE_DISABLED_FUN) 
                    {                   
                        for (ia = bnodeChildLo ; ia <= bnodeChildHi ; ia++)
                        {
                            TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap , ia , TM_Sched_ALvltoBlvlAndQueueRangeMap)
                            if (rc) return -1;
                            TXQ_SCHED_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap , Blvl , anodeParent,(int))
                            TXQ_SCHED_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap , QueueLo , anodeChildLo,(int))
                            TXQ_SCHED_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap , QueueHi , anodeChildHi,(int))
                            
                            TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFuncPtr , ia , TM_Sched_AlvlEligPrioFuncPtr)
                            if (rc) return -1;
                            TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlEligPrioFuncPtr , Ptr , anodeEf, (uint8_t))            

                           TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlQuantum , ia , TM_Sched_AlvlQuantum)

                            if (rc) return -1;

                            TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlQuantum , Quantum ,dwrr_quantum,(uint16_t))

                            a_node_ptr=&(ctl->tm_a_node_array[ia]);
                            
                            result = dump_fun(1, vpEnv, A_LEVEL, ia, dwrr_quantum, anodeParent, anodeEf,vpUserData);
                            if (result == -1 )  continue ; else if (result > 0 ) return result;
                            /* safety check - only USED node have valid childern range */
                            if (anodeEf != TM_NODE_DISABLED_FUN) 
                            {
                                for (iq = anodeChildLo ; iq <= anodeChildHi ; iq++)
                                {
                                    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueAMap , iq , TM_Sched_QueueAMap)
                                    if (rc) return -1;
                                    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueAMap , Alvl , queueParent,(int))
                                    
                                    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFuncPtr , iq , TM_Sched_QueueEligPrioFuncPtr)
                                    if (rc) return -1;
                                    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueEligPrioFuncPtr , Ptr , queueEf, (uint8_t))

                                    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueQuantum , iq , TM_Sched_QueueQuantum)

                                      if (rc) return -1;

                                   TXQ_SCHED_REGISTER_GET(TM_Sched_QueueQuantum , Quantum ,dwrr_quantum,(uint16_t))

                                    
                                    result = dump_fun(1, vpEnv, Q_LEVEL, iq, dwrr_quantum, queueParent,  queueEf,vpUserData);
                                    if (result > 0 ) return result;
                                }
                            }
                            result = dump_fun(0, vpEnv, A_LEVEL, ia, a_node_ptr->mapping.nodeStatus, anodeParent, anodeEf, vpUserData);
                            if (result > 0 ) return result;
                        }
                    }
                    result = dump_fun(0,  vpEnv, B_LEVEL, ib, b_node_ptr->mapping.nodeStatus, bnodeParent, bnodeEf, vpUserData);
                    if (result == -1 )  continue ; else if (result > 0 ) return result;
                }
            }
            result = dump_fun(0, vpEnv, C_LEVEL, ic, c_node_ptr->mapping.nodeStatus, cnodeParent, cnodeEf, vpUserData);
            if (result > 0 ) return result;
        }
    }
    result = dump_fun(0, vpEnv, P_LEVEL, port_index, port_ptr->mapping.nodeStatus, 0, portEf , vpUserData);
    if (result) return result;
    return 0;
}


