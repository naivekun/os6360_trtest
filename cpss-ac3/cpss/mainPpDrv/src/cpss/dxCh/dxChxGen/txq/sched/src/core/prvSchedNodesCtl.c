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
 * @brief tm_nodes_ctl implementation.
 *
* @file tm_nodes_ctl.c
*
* $Revision: 2.0 $
 */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmChunk.h>

#include <stdlib.h>
#include <errno.h>

/**
 */


/**
 */
static int tm_uninstall_queue_proc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t queue_index)
{
    struct tm_queue * queue;
    int rc;

    TM_CTL(ctl, hndl);

    VALIDATE_NODE_ALLOCATED(queue,queue_index);
    if (rc) goto out;
    
    queue = &(ctl->tm_queue_array[queue_index]);

    /* if queue is already uninsalled  - do nothing, exit */
    if (queue->installed == TM_DISABLE) goto out;
    /* set H/W */
    if (rc)
    {
        rc = TM_HW_QUEUE_CONFIG_FAIL;
        goto out;
    }
    queue->installed = TM_DISABLE;
out:
    return rc;
}




/**************************************************************************
 * utilities for allocating / cleaning  nodes
 **************************************************************************/
 
#define CHECK_IF_IN_RANGE(index,node_type)  if (index >= ctl->tm_total_##node_type##s) return -EFAULT;
#define CHECK_IF_USED(node)                 if (node->mapping.nodeStatus != TM_NODE_USED) return -ENODATA;
#define UPDATE_FREE_COUNTER(node_type,size) ctl->tm_free_##node_type##s_count += (size);

#define ALLOCATE_NODE_PROC(node_type, child_level , child_node_type, first_child_in_range)  \
    struct tm_##node_type *node;\
    int rc;\
    int i;\
    uint32_t start_range_index;\
    CHECK_IF_IN_RANGE(index , node_type)\
    node = &(ctl->tm_##node_type##_array[index]);\
    if (children_range)\
    {\
        if (first_child_in_range != (uint16_t)(NO_PREDEFINED_RANGE))\
        {\
            rc = prvSchedRmGetChunk(ctl->rm, child_level, first_child_in_range , children_range);\
            start_range_index = first_child_in_range;\
        }\
        else \
        {\
            /* find  free range of childs*/ \
            rc = prvSchedRmAllocateChunk(ctl->rm, child_level, children_range, &start_range_index);\
        }\
        if (rc) return 1;\
    }\
    else  return 2 ; /* it isn't possible to create nodes with zero ranges in H/W */\
    node->mapping.childLo=start_range_index;\
    node->mapping.childHi=node->mapping.childLo+children_range-1;\
    /* bound childen to parent */\
    for (i = node->mapping.childLo ; i <= node->mapping.childHi ; i++)\
    {\
        prvSchedSetSw_##child_node_type##_Default(&(ctl->tm_##child_node_type##_array[i]),0);\
        ctl->tm_##child_node_type##_array[i].mapping.nodeStatus=TM_NODE_RESERVED;\
        ctl->tm_##child_node_type##_array[i].mapping.nodeParent=index;\
    }\
    /* set node as allocated (used)*/\
    *old_node_status=node->mapping.nodeStatus;\
    node->mapping.nodeStatus=TM_NODE_USED;\
    /*UPDATE_FREE_COUNTER(child_node_type,-children_range) */

    
#define FREE_NODE_PROC(node_type , child_level, child_node_type) \
    struct tm_##node_type *node;\
    int i;\
    uint32_t range;\
    int rc;\
    CHECK_IF_IN_RANGE(index , node_type)\
    node = &(ctl->tm_##node_type##_array[index]);\
    CHECK_IF_USED(node)\
    /* test if chilren are already released*/\
    for (i = node->mapping.childLo ; i <= node->mapping.childHi ; i++)\
    {\
        if (ctl->tm_##child_node_type##_array[i].mapping.nodeStatus==TM_NODE_USED) return -EBUSY;\
    }\
    /* move resources back to free resource pool */\
    range=node->mapping.childHi - node->mapping.childLo +1;\
    rc = prvSchedRmReleaseChunk(ctl->rm, child_level, range , node->mapping.childLo);\
    if (rc) return rc;\
    /* release previously allocated child nodes */\
    for (i = node->mapping.childLo ; i <= node->mapping.childHi ; i++)\
    {\
        ctl->tm_##child_node_type##_array[i].mapping.nodeStatus=TM_NODE_FREE;\
    }\
    /* update node status (should be one of disabled types)*/\
    node->mapping.nodeStatus = node_status;\
    /*UPDATE_FREE_COUNTER(child_node_type,range)*/
    
#ifdef  COMPACT_DELETED_NODES
    #define FREE_DELETED_NODES(node_type , node_level, parent_node_type) \
            struct tm_##parent_node_type *parent_node;\
            parent_node=&(ctl->tm_##parent_node_type##_array[node->mapping.nodeParent]); \
            if ((int)index ==parent_node->mapping.childHi) /* && (ctl->tm_##node_type##_array[index+1].mapping..nodeStatus == TM_NODE_FREE)) */\
            {\
                while ((node->mapping.nodeStatus == TM_NODE_DELETED) && (ENABLE_REDUCE_NODE_RANGE(parent_node))) \
                {\
                    node->mapping.nodeStatus = TM_NODE_FREE;\
                    prvSchedRmReleaseChunk(ctl->rm, node_level,1, (uint32_t)parent_node->mapping.childHi);\
                    parent_node->mapping.childHi--;\
                    node = &(ctl->tm_##node_type##_array[parent_node->mapping.childHi]); \
                }\
            }\
            if ((int)index == parent_node->mapping.childLo)/* && (ctl->tm_##node_type##_array[index-1].mapping..nodeStatus == TM_NODE_FREE)) */\
            {\
                while ((node->mapping.nodeStatus == TM_NODE_DELETED) && (ENABLE_REDUCE_NODE_RANGE(parent_node)))\
                {\
                    node->mapping.nodeStatus = TM_NODE_FREE;\
                    prvSchedRmReleaseChunk(ctl->rm, node_level, 1, (uint32_t)parent_node->mapping.childLo);\
                    /* next node */\
                    parent_node->mapping.childLo++;\
                    node = &(ctl->tm_##node_type##_array[parent_node->mapping.childLo]); \
                }\
            }
#else
    #define FREE_DELETED_NODES(node_type , node_level, parent_node_type) /* do nothing */
#endif  

int     prvSchedNodesCtlAllocateAsymPort(struct prvCpssDxChTxqSchedShadow *ctl,  uint8_t index, uint16_t children_range, uint16_t first_child_in_range)
{
    /* old node status is not necessary for ports - it can be used or free , do variables below are dummy, needed only for common macro */
    tm_node_status_t dummy_node_status;
    tm_node_status_t * old_node_status=&dummy_node_status;
    ALLOCATE_NODE_PROC(port,C_LEVEL,c_node,first_child_in_range)
    return 0;
}
  
int     prvSchedNodesCtlFreeAsymPort(struct prvCpssDxChTxqSchedShadow *ctl, uint32_t index)
{
    /* only possible status for port is TM_NODE_FREE , so it is set internally below*/
    tm_node_status_t node_status=TM_NODE_FREE;
    FREE_NODE_PROC(port,C_LEVEL ,c_node)    
    return 0;
}


int     prvSchedNodesCtlAllocateReservedCNode(struct prvCpssDxChTxqSchedShadow *ctl,  uint16_t index, uint8_t parent_index, uint16_t children_range, uint16_t first_child_in_range, tm_node_status_t * old_node_status)
{
    ALLOCATE_NODE_PROC(c_node,B_LEVEL,b_node,first_child_in_range)
    node->mapping.nodeParent=parent_index;
    return 0;
}


int     prvSchedNodesCtlFreeCNode(struct prvCpssDxChTxqSchedShadow *ctl,  uint32_t index, tm_node_status_t node_status)
{
    /* in case of invalid/unused node or active chilren the function can exit inside macro below*/
    FREE_NODE_PROC(c_node,B_LEVEL,b_node);
    if (node_status==TM_NODE_DELETED)
    {
        FREE_DELETED_NODES(c_node,C_LEVEL,port)
    }
    return 0;
}

int     prvSchedNodesCtlAllocateReservedBNode(struct prvCpssDxChTxqSchedShadow *ctl,  uint16_t index, uint32_t parent_index, uint16_t children_range, uint16_t first_child_in_range, tm_node_status_t * old_node_status)
{
    ALLOCATE_NODE_PROC(b_node,A_LEVEL,a_node,first_child_in_range)
    node->mapping.nodeParent=(uint16_t)parent_index;
    return 0;
}


int     prvSchedNodesCtlFreeBNode(struct prvCpssDxChTxqSchedShadow *ctl,  uint32_t index, tm_node_status_t node_status)
{
    FREE_NODE_PROC(b_node,A_LEVEL,a_node);
    if (node_status==TM_NODE_DELETED)
    {
        FREE_DELETED_NODES(b_node,B_LEVEL,c_node)
    }   
    return 0;
}

int     prvSchedNodesCtlAllocateReservedANode(struct prvCpssDxChTxqSchedShadow *ctl,  uint16_t index, uint32_t parent_index,   uint16_t children_range, uint16_t first_child_in_range, tm_node_status_t * old_node_status)
{
    ALLOCATE_NODE_PROC(a_node,Q_LEVEL,queue,first_child_in_range)   
    node->mapping.nodeParent=(uint16_t)parent_index;
    return 0;
}


int     prvSchedNodesCtlFreeANode(struct prvCpssDxChTxqSchedShadow *ctl,  uint32_t index, tm_node_status_t node_status)
{
    FREE_NODE_PROC(a_node,Q_LEVEL,queue);   
    if (node_status==TM_NODE_DELETED)
    {
        FREE_DELETED_NODES(a_node , A_LEVEL, b_node)
    }
    return 0;
}


int     prvSchedNodesCtlAllocateReservedQueue(struct prvCpssDxChTxqSchedShadow *ctl,  uint16_t index, uint32_t parent_index, tm_node_status_t * old_node_status)
{
    struct tm_queue     *queue=&(ctl->tm_queue_array[index]);
    *old_node_status = queue->mapping.nodeStatus;
    queue->mapping.nodeStatus = TM_NODE_USED; 
    queue->mapping.nodeParent=(uint16_t)parent_index;
    return 0;
}
int     prvSchedNodesCtlFreeQueue(struct prvCpssDxChTxqSchedShadow *ctl, uint32_t index, tm_node_status_t node_status)
{
    struct tm_queue     *queue=&(ctl->tm_queue_array[index]);
    CHECK_IF_IN_RANGE(index , queue)
    CHECK_IF_USED(queue)
    queue->mapping.nodeStatus = node_status; 
    return 0;
}


/**************************************************************************************************************************
 */
static int tm_delete_port(PRV_CPSS_SCHED_HANDLE hndl, uint8_t index)
{
    struct tm_port *port = NULL;
    int rc;
    

    TM_CTL(ctl, hndl);


        
    rc = prvSchedNodesCtlFreeAsymPort(ctl,index);
    if (rc)  return rc;
    
    port = &(ctl->tm_port_array[index]);


     
    prvSchedSetSwPortDefault(port,0);

    if (prvSchedLowLevelSetPortAllParameters(hndl, index))              return TM_HW_PORT_CONFIG_FAIL;

    /* Clear DWRR Deficit */
    if (prvSchedLowLevelClearPLevelDeficit(hndl, index)) return TM_HW_PORT_CONFIG_FAIL;

    return 0;
}


/**
 */
int prvSchedDeleteCNodeProc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{

    struct tm_c_node *c_node = NULL;
    int rc;
    

    TM_CTL(ctl, hndl);

    rc = prvSchedNodesCtlFreeCNode(ctl,index,TM_NODE_DELETED);
    if (rc)  return rc;
        
    c_node = &(ctl->tm_c_node_array[index]);
    
    prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, c_node->shaping_profile_ref, C_LEVEL, index);
        

    prvSchedSetSw_c_node_Default(c_node,0);
    
    /* Download  parameters to H/W */
    /* node mapping was not changed */
    if (prvSchedLowLevelSet_c_node_Params(hndl, index))                     return TM_HW_C_NODE_CONFIG_FAIL;
    if (prvSchedLowLevelSetNodeEligPriorityFunc(hndl, C_LEVEL, index))  return TM_HW_C_NODE_CONFIG_FAIL;

    /* set shaping parameters to default */
    if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, C_LEVEL, index))               return TM_HW_SHAPING_PROF_FAILED;

    /* Clear DWRR Deficit */
    if (prvSchedLowLevelClearCLevelDeficit(hndl, index))                return TM_HW_C_NODE_CONFIG_FAIL;

    return 0;
}


/**
 */
int prvSchedDeleteBNodeProc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{

    struct tm_b_node *b_node = NULL;
    int rc;

    TM_CTL(ctl, hndl);

    rc = prvSchedNodesCtlFreeBNode(ctl,index,TM_NODE_DELETED);
    if (rc) return rc;

    b_node = &(ctl->tm_b_node_array[index]);
    
    prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, b_node->shaping_profile_ref, B_LEVEL, index);
    
    

    prvSchedSetSw_b_node_Default(b_node,0);

    /* Download  parameters to H/W */
    /* node mapping was not changed - not necessary to update it*/
    if (prvSchedLowLevelSet_b_node_Params(hndl, index))                     return TM_HW_B_NODE_CONFIG_FAIL;
    if (prvSchedLowLevelSetNodeEligPriorityFunc(hndl, B_LEVEL, index))  return TM_HW_B_NODE_CONFIG_FAIL;

    /* set shaping parameters to default */
    if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, B_LEVEL, index))               return TM_HW_SHAPING_PROF_FAILED;

    /* Clear DWRR Deficit */
    if (prvSchedLowLevelClearBLevelDeficit(hndl, index))                return TM_HW_B_NODE_CONFIG_FAIL;

    return 0;
}


/**
 */
int prvSchedDeleteANodeProc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{

    struct tm_a_node *a_node = NULL;
    int rc;

    TM_CTL(ctl, hndl);

    rc = prvSchedNodesCtlFreeANode(ctl,index,TM_NODE_DELETED);
    if (rc) return rc;
    
    a_node = &(ctl->tm_a_node_array[index]);

    prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, a_node->shaping_profile_ref, A_LEVEL, index);
        
    
    prvSchedSetSw_a_node_Default(a_node,0);

    /* Download  parameters to H/W */
    /* node mapping was not changed - not necessary to update it*/
    if (prvSchedLowLevelSet_a_node_Params(hndl, index))return TM_HW_A_NODE_CONFIG_FAIL;
    if (prvSchedLowLevelSetNodeEligPriorityFunc(hndl, A_LEVEL, index))  return  TM_HW_A_NODE_CONFIG_FAIL;

    /* set shaping parameters to default */
    if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, A_LEVEL, index))               return TM_HW_SHAPING_PROF_FAILED;

    /* Clear DWRR Deficit */
    if (prvSchedLowLevelClearALevelDeficit(hndl, index))                return TM_HW_A_NODE_CONFIG_FAIL;

    return 0;
}


/**
 */
static int tm_delete_queue(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    struct tm_queue *queue = NULL;
    int rc;

    TM_CTL(ctl, hndl);

    rc = prvSchedNodesCtlFreeQueue(ctl,index,TM_NODE_DELETED);
    if (rc) return rc;
    
    queue = &(ctl->tm_queue_array[index]);

    prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, queue->shaping_profile_ref, Q_LEVEL, index);

    prvSchedSetSw_queue_Default(queue,0);


    /* node mapping was not changed - not necessary for update*/
    if (prvSchedLowLevelSetQueueQuantum(hndl, index))                       return TM_HW_QUEUE_CONFIG_FAIL;
    if (prvSchedLowLevelSetNodeEligPriorityFunc(hndl, Q_LEVEL, index))  return TM_HW_QUEUE_CONFIG_FAIL;

    /* set shaping parameters to default */
    if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, Q_LEVEL, index))               return TM_HW_SHAPING_PROF_FAILED;

    /* Clear DWRR Deficit */
    if (prvSchedLowLevelClearQLevelDeficit(hndl, index))                return TM_HW_QUEUE_CONFIG_FAIL;

    return 0;
}

/**
 */
int prvSchedDeleteNode(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel lvl, uint32_t index)
{
    int rc;
    TM_CTL(ctl, hndl);
    
    /* convert virtual ID to physical  - for supported levels (inside macro)*/
    CONVERT_TO_PHYSICAL(lvl,index)
    
 

    switch (lvl)
    {
    case P_LEVEL:
        rc = tm_delete_port(hndl, (uint8_t)index);
        break;
    case C_LEVEL:
        rc = prvSchedDeleteCNodeProc(hndl, index);
        break;
    case B_LEVEL:
        rc = prvSchedDeleteBNodeProc(hndl, index);
        break;
    case A_LEVEL:
        rc = prvSchedDeleteANodeProc(hndl, index);
        if (rc) goto out;
        break;
    case Q_LEVEL:
        rc = tm_delete_queue(hndl, index);
        break;
    default:
        /* can't happend */
        rc = -ERANGE;
    }
    /** remove virtual index from dictionary - for supported levels (inside macro) */
    REMOVE_VIRTUAL_ID(lvl,index)

out:
    
    return rc;
}


/**
 */
int prvSchedDeleteTransPort(PRV_CPSS_SCHED_HANDLE hndl, uint8_t index)
{
    int  rc;
    int  i;
    int  j;
    int  k;
    int  l;
    struct tm_port  *port = NULL;
    struct tm_c_node    *c_node = NULL;
    struct tm_b_node    *b_node = NULL;
    struct tm_a_node    *a_node = NULL;

   TM_CTL(ctl, hndl);



    if (index >= ctl->tm_total_ports)
    {
        rc = TM_CONF_PORT_IND_OOR;
        goto out;
    }
    
    port = &(ctl->tm_port_array[index]);
    if (port->mapping.nodeStatus != TM_NODE_USED)
    {
        rc = -ENOMSG;
        goto out;
    }
    /* check if port is symmetric   ??? */

    for (i =  port->mapping.childLo; i <= port->mapping.childHi; i++)
    {
        c_node = &(ctl->tm_c_node_array[i]);
        if (c_node->mapping.nodeStatus == TM_NODE_USED)
        {
            for (j =  c_node->mapping.childLo; j <= c_node->mapping.childHi; j++)
            {
                b_node = &(ctl->tm_b_node_array[j]);
                if (b_node->mapping.nodeStatus == TM_NODE_USED)
                {
                    for (k =  b_node->mapping.childLo; k <= b_node->mapping.childHi; k++)
                    {
                        a_node = &(ctl->tm_a_node_array[k]);
                        if (a_node->mapping.nodeStatus == TM_NODE_USED)
                        {
                            for (l = a_node->mapping.childLo; l <= a_node->mapping.childHi; l++)
                            {
                                if (ctl->tm_queue_array[l].mapping.nodeStatus == TM_NODE_USED)
                                {
                                    rc = tm_uninstall_queue_proc(hndl, l);  
                                    if (rc)   goto out;
                                    rc = tm_delete_queue(hndl, l);
                                    if ((rc) && (rc != -ENOMSG))   goto out;
                                }
                            }
                            rc = prvSchedDeleteANodeProc(hndl, k);
                            if (rc) goto out;
                        }
                    }
                    rc = prvSchedDeleteBNodeProc(hndl, j);
                    if (rc)  goto out;
                }
            }
            rc = prvSchedDeleteCNodeProc(hndl, i);
            if (rc) goto out;
        }
    }

    rc = tm_delete_port(hndl, index);

out:
    
    return rc;
}


#ifdef VIRTUAL_LAYER_TO_REMOVE_IN_FUTURE
/*
 */
#else

    /* low level reshuffling procedures
     */
    int prvSchedNodesReadNextChange(PRV_CPSS_SCHED_HANDLE hndl, struct tm_tree_change *change)
    {
        int rc = 0;
#ifdef VIRTUAL_LAYER
/* functions are only for compatibility with cpss wrappers
 */
#else
        struct tm_tree_change *elem = NULL;

        TM_CTL(ctl, hndl)


        elem = ctl->list.next;
        if(elem == NULL)
        {/* empty list */ 
          rc = -ENOBUFS;
          goto out;
        }
          
        ctl->list.next = elem->next;
        change->index = elem->index;
        change->old_index = elem->old_index;
        change->new_index = elem->new_index;
        change->next = NULL;
        schedFree(elem);

    out:
#endif
        return rc;
    }


    int prvSchedCleanReshafflingList(PRV_CPSS_SCHED_HANDLE hndl)
    {

        int rc = 0;
#ifdef VIRTUAL_LAYER
/*
 */
#else
        struct tm_tree_change *elem = NULL;

        TM_CTL(ctl, hndl)

        while(ctl->list.next != NULL)
        {
          elem = ctl->list.next;
          ctl->list.next = elem->next;
          schedFree(elem);
        }
#endif
        return rc;
    }
    /**
     */
    
    

#endif 
