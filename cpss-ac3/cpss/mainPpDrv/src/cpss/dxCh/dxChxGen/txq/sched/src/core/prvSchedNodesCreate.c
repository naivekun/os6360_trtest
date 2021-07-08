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
 * @brief TM nodes creation implementation.
 *
* @file tm_nodes_create.c
*
* $Revision: 2.0 $
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

/*#define RESHUFFLING_SHIELD */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesCreate.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedHwConfigurationInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingUtils.h>



/* for validation shaping profiles during node creation */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingInternal.h>


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmStatus.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmChunk.h>



static int port_update_sw_image(PRV_CPSS_SCHED_HANDLE hndl,
                                struct tm_port_params *params,
                                uint32_t min_token,
                                uint32_t max_token,
                                uint32_t divider,
                                uint8_t res_min_bw,
                                uint8_t res_max_bw,
                                uint8_t port_index)
{

    int i;
    struct tm_port *port = NULL;

    TM_CTL(ctl, hndl)

    port = &(ctl->tm_port_array[port_index]);

    /* Update Port SW DB */

    
  
    /* update shaping parameters */
    port->cir_token = (uint16_t)min_token;
    port->eir_token = (uint16_t)max_token;
    port->periods = (uint16_t)divider;
    port->min_token_res = res_min_bw;
    port->max_token_res = res_max_bw;

    port->cir_sw = params->cir_bw;
    port->eir_sw = params->eir_bw;
    if (params->cir_bw < TM_INVAL)
    {
        port->cir_burst_size = params->cbs;
        port->eir_burst_size = params->ebs;
    }
    else
    {
        /* no shaping for port */
        port->cir_burst_size = 0x1FFFF;
        port->eir_burst_size = 0x1FFFF;
    }

    /* DWRR for Port */
    for (i=0; i<8; i++)
        port->dwrr_quantum_per_level[i].quantum = params->quantum[i];


    /* DWRR for C-nodes in Port's range */
    port->dwrr_enable_per_priority_mask = 0;
    for (i=0; i<8; i++)
        port->dwrr_enable_per_priority_mask =
            port->dwrr_enable_per_priority_mask | (params->dwrr_priority[i] << i);

    /* Update sw image with eligible priority function pointer */
    port->elig_prio_func = params->elig_prio_func_ptr;
    return 0;
}




/***************************************************************************
 * Port Creation 
 ***************************************************************************/

/**
 */
static int tm_create_asym_port_ex(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_index,
                        struct tm_port_params *params,
                        uint32_t first_child_in_range)
{
    struct tm_port *port = NULL;
    uint16_t    min_token;
    uint16_t    max_token;
    uint8_t     res_min_bw;
    uint8_t     res_max_bw;
    uint16_t    periods;
    uint32_t    cir_bw, eir_bw;
    uint32_t    cbs;
    uint32_t    ebs;
   

    int rc = 0;
    uint32_t i;

    TM_CTL(ctl, hndl)
   

    if (params==NULL) return -EINVAL;

   

    /* Check parameters validity */

    VALIDATE_NODE_FREE_EX(port,port_index, TM_CONF_PORT_IND_OOR, TM_CONF_PORT_IND_USED)
    if (rc) goto out;

    port = &(ctl->tm_port_array[port_index]);

   
    

    /* Port params */
    /* DWRR for Port */
    for (i=0; i<8; i++)
    {
        VALIDATE_PORT_QUANTUM_VALUE(params->quantum[i],TM_CONF_PORT_QUANTUM_OOR)
        if(rc) goto out;
    }

    /* DWRR for C-nodes in Port's range */
    for (i=0; i<8; i++)
    {
        if ((params->dwrr_priority[i] != TM_DISABLE) &&
            (params->dwrr_priority[i] != TM_ENABLE))
        {
            rc = TM_CONF_PORT_DWRR_PRIO_OOR;
            goto out;
        }
    }

    VALIDATE_ELIG_FUNCTION(params->elig_prio_func_ptr)

    if ((params->num_of_children > prvSchedRmGetMaxChunkSize(ctl->rm,C_LEVEL)) || (params->num_of_children == 0))
    {
        rc = TM_CONF_INVALID_NUM_OF_C_NODES;
        goto out;
    }

   /* if shaping is defined for a port */
    if (ctl->level_data[P_LEVEL].shaping_status==TM_ENABLE)
    {
        if (params->cir_bw != TM_INVAL) cir_bw = params->cir_bw;
        else                            cir_bw = TM_MAX_BW;
        if (params->eir_bw != TM_INVAL) eir_bw = params->eir_bw;
        else                            eir_bw = TM_MAX_BW;
      
        if ((params->cbs > TM_128M_kB ) || (params->ebs > TM_128M_kB ))
        {
            rc = TM_CONF_PORT_BS_OOR;
            goto out;
        }

        if (cir_bw == TM_MAX_BW)
            cbs = TM_MAX_BURST;
        else
            cbs = params->cbs;

        if (eir_bw == TM_MAX_BW)
            ebs = TM_MAX_BURST;
        else
            ebs = params->ebs;
        
        rc = prvSchedShapingUtilsCalculatePortShaping(1000.0*cir_bw/ctl->level_data[P_LEVEL].unit,
                                    1000.0*eir_bw/ctl->level_data[P_LEVEL].unit,
                                    &cbs,
                                    &ebs,
                                    (1 << ctl->level_data[P_LEVEL].token_res_exp),
                                    &periods,
                                    &res_min_bw,
                                    &res_max_bw,
                                    &min_token,
                                    &max_token);
        if (rc)
            goto out;

#if DEBUG
        PRINT_PORT_SHAPING_CALCULATION_RESULT(cir_bw, eir_bw)
#endif
    }
    else
    {   /* no shaping for port */
        periods = 1;     /* Small divider (shared). */
        res_min_bw = 7;  /* Large CIR exponent. */
        res_max_bw = 7;  /* Large EIR exponent. */
        min_token = 0x7FF;
        max_token = 0x7FF;
    }

    port->sym_mode = TM_DISABLE;

    rc=prvSchedNodesCtlAllocateAsymPort(ctl,port_index,params->num_of_children, (uint16_t)first_child_in_range);
    if (rc) goto out;

    /* on params->cir_bw / params->eir_bw = TM_INF_SHP_BW
       TM_MAX_BW is used for shaping calculations and
       TM_INF_SHP_BW is kept in sw image */
    port_update_sw_image(hndl, params,
                         min_token, max_token, periods,
                         res_min_bw, res_max_bw, port_index);
    /* Download to HW */
    rc = prvSchedLowLevelSetPortAllParameters(hndl, port_index);
    if (rc < 0)
        rc = TM_HW_PORT_CONFIG_FAIL;

out:
    if (rc)
    {
        if (rc == TM_HW_PORT_CONFIG_FAIL)
        {
            prvSchedNodesCtlFreeAsymPort(ctl,port_index);
            prvSchedSetSwPortDefault(port,0);
            
        }
    }

    return rc;
}

int prvCpssSchedNodesCreateAsymPort
 (
     IN PRV_CPSS_SCHED_HANDLE hndl, 
     IN uint8_t port_index,
     IN struct tm_port_params *params,
     IN  GT_BOOL                  fixedMappingToCNode
)
{
    return  tm_create_asym_port_ex(hndl, port_index,  params, fixedMappingToCNode?port_index:-1 /* no preallocated range */);
}







/***************************************************************************
 * Queue Creation
 ***************************************************************************/



/**
 */
static int tm_create_queue_to_a_node_ex(PRV_CPSS_SCHED_HANDLE hndl, uint32_t parent_a_node_index,
                              struct tm_queue_params *q_params,
                              uint32_t * queue_index_ptr)
{
    struct tm_a_node *parent_a_node = NULL;
    struct tm_queue *queue = NULL;
    int rc = 0;
    int i;
    tm_node_status_t old_node_status = TM_NODE_USED;
    

    TM_CTL(ctl, hndl)
    

    if (q_params==NULL) return -EINVAL;
    if (queue_index_ptr==NULL) return -EINVAL;

    CONVERT_TO_PHYSICAL(A_LEVEL,parent_a_node_index)

    /*******************************************************************************/
    /** Check parameters validity                                                  */
    /*******************************************************************************/
    VALIDATE_NODE_ALLOCATED_EX(a_node, parent_a_node_index, TM_CONF_A_NODE_IND_OOR, TM_CONF_A_NODE_IND_NOT_EXIST)
    if (rc) goto out;

    parent_a_node = &(ctl->tm_a_node_array[parent_a_node_index]);

    /* Queue params */
    rc= prvCpssSchedShapingCheckShapingProfileValidity(hndl,q_params->shaping_profile_ref, Q_LEVEL);
    if (rc) goto out;

    VALIDATE_ELIG_FUNCTION(q_params->elig_prio_func_ptr)

    
    
    /* check quantum validity*/
    VALIDATE_NODE_QUANTUM_VALUE(q_params->quantum, TM_CONF_Q_QUANTUM_OOR)
    if (rc) goto out;

    /*******************************************************************************/
    /** all input parameters are valid, select queue                         */
    /*******************************************************************************/

    if (*queue_index_ptr !=(uint32_t)-1)
    {
        /*******************************************************************************/
        /**  queue is selected by user - test if itis valid & free                     */
        /*******************************************************************************/
        i = *queue_index_ptr;
        if ((i <  parent_a_node->mapping.childLo)  || (i >  parent_a_node->mapping.childHi))
        {
            /* nothing to do in caseof queues , unrecoverable state */
            rc = -ENOBUFS;
            goto out;
        }
        if (ctl->tm_queue_array[i].mapping.nodeStatus <= TM_NODE_USED)
        {
            /* queue is already allocated,  error state */
            rc = -ENOBUFS;
            goto out;
        }
    }
    else
    {
        /*******************************************************************************/
        /**  queis nor preselected- Find free Queue                            */
        /*******************************************************************************/
        /* looking for reserved or previously deleted nodes in the range */
        for (i= parent_a_node->mapping.childLo ;  i <= parent_a_node->mapping.childHi ; i++)
        {
        if (ctl->tm_queue_array[i].mapping.nodeStatus > TM_NODE_USED) break;
        }
        if (i > parent_a_node->mapping.childHi )   /* all nodes are already used */
        {
            /* nothing to do in caseof queues , unrecoverable state */
            rc = -ENOBUFS;
            goto out;
        }
    }

    /**************************************************************************/
    /** queue selected                                                          */
    /**************************************************************************/
    /* for queues prvSchedNodesCtlAllocateReservedQueue(..) always errorless */
    prvSchedNodesCtlAllocateReservedQueue(ctl,(uint16_t)i, parent_a_node_index, &old_node_status);
    *queue_index_ptr = i;
    queue = &(ctl->tm_queue_array[*queue_index_ptr]);
    /*******************************************************************************/
    /** update queue node parameters                                               */
    /*******************************************************************************/
    /* Update Queue SW DB */
    rc=prvCpssSchedShapingAddNodeToShapingProfile(ctl,q_params->shaping_profile_ref, Q_LEVEL, *queue_index_ptr);
    if (rc < 0) goto out;
    
    queue->elig_prio_func = q_params->elig_prio_func_ptr;

    /* DWRR for Queue - update even if disabled */
    queue->dwrr_quantum = q_params->quantum;

    /**************************************************************************/
    /** queue parameters are set , let update H/W                             */
    /**************************************************************************/
    /* Download Queue to HW */
    if (prvSchedLowLevelSetHwNodeMapping(hndl, Q_LEVEL, *queue_index_ptr)
     || prvSchedLowLevelSetQueueQuantum(hndl, *queue_index_ptr)
     || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, Q_LEVEL, *queue_index_ptr))
    {
        rc = TM_HW_QUEUE_CONFIG_FAIL;
        goto out;
    }
    /* Download Queue shaping parameters (from profile) to HW */
    if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, Q_LEVEL, *queue_index_ptr))
    {
        rc = TM_HW_SHAPING_PROF_FAILED;
        goto out;
    }

    /*******************************************************************************/
    /** here  node successfully created.                                           */
    /*******************************************************************************/
    /** if  virtual numbers on queues should require   - create virtual ID & add entry to dictionary
     GENERATE_VIRTUAL_ID (Q_LEVEL, *queue_index);
     ************************************************/

out:
    if (rc)
    {
        if (rc == TM_HW_QUEUE_CONFIG_FAIL)
        {
            prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, queue->shaping_profile_ref, Q_LEVEL, *queue_index_ptr);
           
            prvSchedSetSw_queue_Default(queue,0); /* reset queue parameters without mapping */
            prvSchedNodesCtlFreeQueue(ctl,*queue_index_ptr,old_node_status);       /* restore queue mapping */
        }
    }
    return rc;
}


int prvCpssSchedNodesCreateQueueToANode(PRV_CPSS_SCHED_HANDLE hndl, uint32_t parent_a_node_index,
                              struct tm_queue_params *q_params,
                              uint32_t * queue_index_ptr)
{
    
    return tm_create_queue_to_a_node_ex(hndl, parent_a_node_index, q_params, queue_index_ptr);
}




/***************************************************************************
 * A-node Creation
 ***************************************************************************/

/**
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
)
{
    int rc = 0;

    if(addToExisting ==GT_FALSE)
    {

        rc = prvCpssSchedNodesCreateBNodeToPort(hndl, port_index, b_params, c_params,fixedPortToBnodeMapping,
                                      b_node_index_ptr, c_node_index_ptr);
        if (rc)
            return rc;
    }

    rc = prvCpssSchedNodesCreateANodeToBNode(hndl, *b_node_index_ptr, a_params, 
                                a_node_index_ptr,explicitQueueStart,queueStart);
    if (rc)
    {
        prvSchedDeleteBNodeProc(hndl, *b_node_index_ptr);
        prvSchedDeleteCNodeProc(hndl, *c_node_index_ptr);
    }
    return rc;
}




/**
 */
/***********************************************************************************************************************/

/* because code for advanced node allocation for A & B layer is identical  except  layer names  (a_node / b_node  , queue/a_node  A_LEVEL/B_LEVEL)
 * writing subroutine is not impossible (a_node and b_node are different types with different sizes, we deal witkh arrays of them ),
 * copy/paste is error prone
 * I developed  some MACRO below ,which are instaniated by node types */



/* utilizing neghborhoods */

#define UTILIZE_NEGHBORHOOD_DECLARATIONS(node_type,parent_node_type)    \
        struct tm_##node_type           *neighbor_node = NULL;\
        struct tm_##parent_node_type    *neighbor_parent = NULL;\
        int left_parent_neighborhood_index = -1;\
        int right_parent_neighborhood_index = -1;\
        int parent_update = 0;



#define UTILIZE_NEGHBORHOOD( LEVEL,node_type, parent_node_type, parent_node)    \
        if ((!found) && ((uint32_t)parent_node->mapping.childLo >0))\
        {\
            /* left neighbor node */\
            neighbor_node=&(ctl->tm_##node_type##_array[parent_node->mapping.childLo - 1]);\
            switch (neighbor_node->mapping.nodeStatus)\
            {\
                case TM_NODE_DELETED:\
                case TM_NODE_RESERVED:\
                    /* find left neighborhood of the parent node */\
                    left_parent_neighborhood_index=neighbor_node->mapping.nodeParent;\
                    /* remove neighbor_node from it : update it's  child range */\
                    neighbor_parent=&(ctl->tm_##parent_node_type##_array[left_parent_neighborhood_index]);\
                    if (ENABLE_REDUCE_NODE_RANGE( neighbor_parent))\
                    {\
                        neighbor_parent->mapping.childHi--; /* what about zero range ??? */\
                        found=1;\
                    }\
                    break;\
                case TM_NODE_FREE:\
                    /* extract one position from free resources pool - must be chunk left of the node*/\
                    prvSchedRmExpandChunkLeft(ctl->rm,LEVEL,parent_node->mapping.childLo - 1);\
                    found=1;\
                    break;\
                default: break; /*failed , need to continue search*/\
            }\
            if (found)\
            {\
                /* expand parent range left */\
                parent_node->mapping.childLo--;\
                /* assign index of node found */\
                i=(uint16_t)parent_node->mapping.childLo;\
                /* set flag to H/W update of the parent node */\
                parent_update = -1;\
            }\
        }\
        /* test right neighbor*/\
        if ((!found) && ((uint32_t)(parent_node->mapping.childHi) < ctl->tm_total_##parent_node_type##s-1))\
        {\
            /* right neighbor node */\
            neighbor_node=&(ctl->tm_##node_type##_array[parent_node->mapping.childHi +1]);\
            switch (neighbor_node->mapping.nodeStatus)\
            {\
                case TM_NODE_DELETED:\
                case TM_NODE_RESERVED:\
                    /* find left neighborhood of the parent node */\
                    right_parent_neighborhood_index=neighbor_node->mapping.nodeParent;\
                    /* remove neighbor_node from it : update it's  child range */\
                    neighbor_parent=&(ctl->tm_##parent_node_type##_array[right_parent_neighborhood_index]);\
                    if (ENABLE_REDUCE_NODE_RANGE( neighbor_parent))\
                    {\
                        neighbor_parent->mapping.childLo++; \
                        found=1;\
                    }\
                case TM_NODE_FREE:\
                    prvSchedRmExpandChunkRight(ctl->rm,LEVEL,parent_node->mapping.childHi +1);\
                    found=1;\
                    break;\
                default: break;  /*failed , need to continue search*/\
            }\
            if (found)\
            {\
                /* expand parent range right */\
                parent_node->mapping.childHi++;\
                /* assign index of node found */\
                i=(uint16_t)parent_node->mapping.childHi; \
                /* set flag to H/W update of the parent node */\
                parent_update = 1;\
            }\
        }


#define UPDATE_CHILDREN_MAPPING(level, ERROR)
/*
#define UPDATE_CHILDREN_MAPPING(level, ERROR)   \
    for (i = node->mapping.childLo ; i <= node->mapping.childHi; i++)\
    {\
        if (set_hw_node_mapping(hndl,CHILD_LEVEL(level),i))\
        {\
            rc = ERROR;\
            goto out;\
        }\
        if (prvSchedLowLevelSetNodeEligPriorityFunc(hndl,CHILD_LEVEL(level),i))\
        {\
            rc = ERROR;\
            goto out;\
        }\
    }
*/

#define UPDATE_PARENTS_MAPPING(LEVEL, parent_index , ERROR) \
    if (right_parent_neighborhood_index>= 0)\
    {\
        if (prvSchedLowLevelSetHwNodeMapping(hndl,PARENT_LEVEL(LEVEL),right_parent_neighborhood_index))\
        {\
            rc = ERROR;\
            goto out;\
        }\
    }\
    if (left_parent_neighborhood_index >= 0)\
    {\
        if (prvSchedLowLevelSetHwNodeMapping(hndl,PARENT_LEVEL(LEVEL),left_parent_neighborhood_index))\
        {\
            rc = ERROR;\
            goto out;\
        }\
    }\
    if (parent_update)\
    {\
        if (prvSchedLowLevelSetHwNodeMapping(hndl,PARENT_LEVEL(LEVEL),parent_index))\
        {\
            rc = ERROR;\
            goto out;\
        }\
    }


#define RESTORE_PARENTS_MAPPING(parent_node)    \
    /* restore parent neighborhood mapping - if any */\
    if (right_parent_neighborhood_index >= 0)   neighbor_parent->mapping.childLo--; \
    if (left_parent_neighborhood_index  >= 0)   neighbor_parent->mapping.childHi++; \
    /* restore parent mapping - if any */\
    if (parent_update > 0)                      parent_node->mapping.childHi--; \
    if (parent_update < 0)                      parent_node->mapping.childLo++; \


/* minimum impact reshuffling */


#ifdef VIRTUAL_LAYER
    #define UPDATE_INDEXES_DECLARATIONS
    #define UPDATE_INDEXES_IMPLEMENTATION(LEVEL,parent_index)   rc= change_physical_id_for_virtual(ctl->hVirtualLayer, LEVEL, /*old index */level_index+oldChildLo, /*new index*/ level_index+newChildLo);
#else
    #define UPDATE_INDEXES_DECLARATIONS     struct tm_tree_change *change = NULL;
    #define UPDATE_INDEXES_IMPLEMENTATION(LEVEL,parent_index)   \
            change = (struct tm_tree_change *)schedMalloc(sizeof(struct tm_tree_change));\
            if (change == NULL)\
            {\
                rc = -ENOMEM;\
                goto out;\
            }\
            else\
            {\
                change->index = parent_index;\
                change->old_index = level_index+oldChildLo;\
                change->new_index = level_index+newChildLo;\
                change->next = ctl->list.next;\
                ctl->list.next = change;\
            }
#endif


#define MIN_IMPACT_RESHUFFLING(parent_node,parent_index,LEVEL,node_type,child_node_type,ERROR)   \
    {\
            int level_index;\
            int child_index;\
            int oldChildLo;\
            int newChildLo;\
            uint32_t ui;\
            UPDATE_INDEXES_DECLARATIONS\
            /* current range is : parent_node->mapping.childHi - parent_node->mapping.childLo+1;*/\
            range =(uint16_t)( parent_node->mapping.childHi - parent_node->mapping.childLo+1);\
            /* desired range is : parent_node->mapping.childHi - parent_node->mapping.childLo +2 */\
            if (!prvSchedRmAllocateChunk(ctl->rm, LEVEL, range+1, &ui))\
            {\
                /* update  S/W db */\
                oldChildLo=parent_node->mapping.childLo;\
                newChildLo=ui;\
                parent_node->mapping.childLo = newChildLo;\
                parent_node->mapping.childHi = newChildLo+range;\
                for (level_index = 0; level_index < (int)range ; level_index++)\
                {\
                    /* copy old node content to the new one */\
                    schedMemCpy(&(ctl->tm_##node_type##_array[level_index+newChildLo]), &(ctl->tm_##node_type##_array[level_index+oldChildLo]), sizeof(struct tm_##node_type));\
                    \
                    /* switch chilren to new node*/\
                    for (child_index =ctl->tm_##node_type##_array[level_index+ newChildLo].mapping.childLo ;  child_index  <= ctl->tm_##node_type##_array[level_index+ newChildLo].mapping.childHi ; child_index++)\
                    {\
                        ctl->tm_##child_node_type##_array[child_index].mapping.nodeParent=(uint16_t)(level_index+newChildLo);\
                    }\
                    /* reset old node related parameters*/ \
                    \
                }\
                /* update H/W */\
                /* 1. disable all necessary nodes */\
                /* all new nodes are disabled */\
                /* to be sure (currently new nodes have valid default elig function) */\
                for (level_index= 0;  level_index <= (int)range ;  level_index++)\
                {\
                    prvSchedLowLevelDisableNodeEligPrioFunc(hndl,LEVEL,level_index+newChildLo);\
                }\
                /* disable old nodes */\
                for (level_index= 0;  level_index < (int)range ;  level_index++)\
                {\
                    prvSchedLowLevelDisableNodeEligPrioFunc(hndl,LEVEL,level_index+oldChildLo);\
                }\
                /* 2.  update nodes content*/\
                /* optionally : disable parent_node */\
                prvSchedLowLevelDisableNodeEligPrioFunc(hndl,PARENT_LEVEL(LEVEL),parent_index);\
                /* 2.0   update parent node  children range*/\
                prvSchedLowLevelSetHwNodeMapping(hndl, PARENT_LEVEL(LEVEL), parent_index);\
                /* 2.1    update new nodes */\
                for (level_index = 0; level_index < (int)range ; level_index++)\
                {\
                    /* 2.1.0 -  update grandchildren parents for children attached to nodes*/\
                    for (child_index =ctl->tm_##node_type##_array[level_index+ newChildLo].mapping.childLo ;  child_index <= ctl->tm_##node_type##_array[level_index+ newChildLo].mapping.childHi ; child_index++)\
                    {\
                        /* disable child nodee ? */\
                        /*prvSchedLowLevelDisableNodeEligPrioFunc(hndl,CHILD_LEVEL(LEVEL),child_index);*/\
                        if (prvSchedLowLevelSetHwNodeMapping(hndl, CHILD_LEVEL(LEVEL), child_index)) \
                        {\
                            rc = ERROR;\
                            /* rollback TBD */\
                            goto out;\
                        }\
                        /* enable child node ?*/\
                        /*prvSchedLowLevelSetNodeEligPriorityFunc(hndl, CHILD_LEVEL(LEVEL), child_index)) */\
                        /* now  child node in H/W is mapped to new node */\
                    }\
                    /* 2.1.1 -  update node parameters & enable it */\
                    /*  (Optionally : it is still attached to  disabled parent_node) */\
                    if (prvSchedLowLevelSetHwNodeMapping(hndl, LEVEL, level_index+newChildLo)\
                     || prvSchedLowLevelSet_##node_type##_Params(hndl,level_index+newChildLo)\
                     || prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, LEVEL, level_index+newChildLo)\
                     || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, LEVEL, level_index+newChildLo))\
                    {\
                        rc = ERROR;\
                        /* rollback TBD */\
                        goto out;\
                    }\
                }\
                /* Optionally ; enable parent node*/\
                prvSchedLowLevelSetNodeEligPriorityFunc(hndl, PARENT_LEVEL(LEVEL), parent_index);\
                /* post update S/W */\
                /* 1. release old nodes pool */\
                prvSchedRmReleaseChunk(ctl->rm,LEVEL,range,oldChildLo);\
                /* mark old nodes as free */\
                /* & make changes list or update virtual ID */\
                for (level_index = 0; level_index < (int)range ; level_index++)\
                {\
                    prvSchedSetSw_##node_type##_Default(&(ctl->tm_##node_type##_array[level_index+oldChildLo]),1);\
                    UPDATE_INDEXES_IMPLEMENTATION(LEVEL,parent_index)\
                }\
            }\
            found = 1;\
            i=(uint16_t)parent_node->mapping.childHi;\
    }

/***********************************************************************************************************************/


static int tm_create_a_node_to_b_node_ex(PRV_CPSS_SCHED_HANDLE hndl, uint32_t parent_b_node_index,
                               struct tm_a_node_params *a_params, uint32_t first_queue_in_range,
                               uint32_t *a_node_index_ptr)
{
    struct tm_port  *parent_port = NULL;
    struct tm_c_node    *parent_c_node = NULL;
    struct tm_b_node    *parent_b_node = NULL;
    struct tm_a_node    *node = NULL;
    uint32_t range;
    int rc = 0;
    uint16_t i;
#if 0
    int reshuffling = 0;
#endif
    tm_node_status_t old_node_status= TM_NODE_USED;
    int found= 0;
    

    UTILIZE_NEGHBORHOOD_DECLARATIONS(a_node,b_node)

    TM_CTL(ctl, hndl)
    

    if (a_params==NULL) return -EINVAL;
    if (a_node_index_ptr==NULL) return -EINVAL;

    CONVERT_TO_PHYSICAL(B_LEVEL, parent_b_node_index)


    /*******************************************************************************/
    /** Check parameters validity                                                  */
    /*******************************************************************************/
    VALIDATE_NODE_ALLOCATED_EX(b_node, parent_b_node_index, TM_CONF_B_NODE_IND_OOR, TM_CONF_B_NODE_IND_NOT_EXIST)
    if (rc) goto out;

    parent_b_node = &(ctl->tm_b_node_array[parent_b_node_index]);
    parent_c_node = &(ctl->tm_c_node_array[parent_b_node->mapping.nodeParent]);
    parent_port = &(ctl->tm_port_array[parent_c_node->mapping.nodeParent]);
    /* check A-node params */
    rc= prvCpssSchedShapingCheckShapingProfileValidity(hndl,a_params->shaping_profile_ref, A_LEVEL);
    if (rc) goto out;

    VALIDATE_NODE_QUANTUM_VALUE(a_params->quantum, TM_CONF_A_QUANTUM_OOR)
    if (rc) goto out;

    /* DWRR for Queues in A-node's range */
    for (i=0; i<8; i++)
    {
        if ((a_params->dwrr_priority[i] != TM_DISABLE) &&
            (a_params->dwrr_priority[i] != TM_ENABLE)) {
            rc = TM_CONF_A_DWRR_PRIO_OOR;
            goto out;
        }
    }

    VALIDATE_ELIG_FUNCTION(a_params->elig_prio_func_ptr)
        

    /* in case of asymmetric port :
     * 1. the children range  is already defined here
     * 2. some H/W changes can be made during search free node (reshuffling , etc), which  should be rolled back if
     * no enought free pool.
     * testing for enought range can prevent this
     * For symmetric node range is defined when free node is known, but in this case no reshuffling -> no rollback
     */
    if ((parent_port->sym_mode == TM_DISABLE) /* asymetric port */ && (prvSchedRmGetMaxChunkSize(ctl->rm, Q_LEVEL) < (int32_t)a_params->num_of_children))
    {
        rc = TM_CONF_INVALID_NUM_OF_QUEUES;
        goto out;
    }

    /**************************************************************************/
    /** Find free a-node                                                      */
    /**************************************************************************/
    /* looking for reserved or previously deleted nodes in the range */
    for (i= (uint16_t)parent_b_node->mapping.childLo ;  i <= parent_b_node->mapping.childHi ; i++)
    {
        if (ctl->tm_a_node_array[i].mapping.nodeStatus > TM_NODE_USED)
        {
            found=1;
            break;
        }
    }
    /* if (found)  i= number of node founded  */
    if ((!found) && (parent_port->sym_mode == TM_ENABLE))   /* for symmetric pors it's unrecoverable case */
    {
    }
    else /* for asymmetric ports let try additional capabilities */
    {
        if (!found)
        {
            /*try to utilize free/reserved node from right/left side */
            UTILIZE_NEGHBORHOOD(A_LEVEL, a_node, b_node, parent_b_node)
        }
/*  SN only  - #ifdef VIRTUAL_LAYER */
                /*try to reshuffling here */
                if (!found)
                {
                    /* for SN - delayed reshuffling
                    rc = TM_CONF_NEED_REALLOC;
                    goto out;
                    */
                    /* for future use - here */
                    MIN_IMPACT_RESHUFFLING(parent_b_node, parent_b_node_index, A_LEVEL, a_node, queue,TM_HW_A_NODE_CONFIG_FAIL);
#if 0
                    reshuffling = 1;
#endif
                }
/* #endif */
    }
    if ( !found)  /* not found anyway */
    {
        rc = -ENOBUFS;
        goto out;
    }
    /**************************************************************************/
    /** free node found , try to allocate children range and set node as used */
    /**************************************************************************/
    /* calculate range */
    if (parent_port->sym_mode == TM_ENABLE) /* symetric mode under the port */
    {
        /* predefined range is not compatible with symmetric port tree creation */
        if (first_queue_in_range != (uint32_t)(NO_PREDEFINED_RANGE))
        {
            rc = TM_CONF_INVALID_NUM_OF_QUEUES;
            goto out;
        }
        if ( i == parent_b_node->mapping.childHi)   range = parent_port->children_range.last_range[Q_LEVEL];/* last child in range */
        else                                        range = parent_port->children_range.norm_range[Q_LEVEL];
    }
    else range = a_params->num_of_children; /* asymmetric mode under the port */
    /* allocate chidrens & nset node as used*/
    if (prvSchedNodesCtlAllocateReservedANode(ctl,(uint16_t)i, parent_b_node_index, (uint16_t)range, (uint16_t)first_queue_in_range, &old_node_status)) 
    {
        rc = TM_CONF_INVALID_NUM_OF_QUEUES;
        goto out;
    }
    /**************************************************************************/
    /** node allocated ,let update node params                                */
    /**************************************************************************/
    *a_node_index_ptr = i;
    node = &(ctl->tm_a_node_array[i]);
    /* Update A-Node SW DB */
    rc=prvCpssSchedShapingAddNodeToShapingProfile(ctl,a_params->shaping_profile_ref, A_LEVEL, *a_node_index_ptr);
    if (rc < 0) goto out;
    
    
    
    node->elig_prio_func = a_params->elig_prio_func_ptr;

    /* DWRR for A-node - update even if disabled */
    node->dwrr_quantum = a_params->quantum;

    /* DWRR for Queues in A-node's range */
    node->dwrr_enable_per_priority_mask = 0;
    for (i=0; i<8; i++)
        node->dwrr_enable_per_priority_mask =
            node->dwrr_enable_per_priority_mask | (a_params->dwrr_priority[i] << i);

    /**************************************************************************/
    /** node parameters are set , let update H/W                              */
    /**************************************************************************/
#ifdef RESHUFFLING_SHIELD
    {
        int iLo;
        int iHi;
        iLo = parent_b_node->mapping.childLo ;
        iHi = parent_b_node->mapping.childHi ;
        /* reduce childen range to old one */
        if (parent_update > 0 ) iHi--;
        if (parent_update < 0 ) iLo--;
        prvSchedLowLevelDisableNodeEligPrioFunc(hndl, B_LEVEL,parent_b_node_index);
        for (i=iLo ;  i <=iHi ; i++)
        {
            prvSchedLowLevelDisableNodeEligPrioFunc(hndl, A_LEVEL,i);
        }
    }
#endif
    /* here  parent value in registers of allocated children is inconsistent with children range of new node
     * It will be updated during creation child nodes ,  but in order to be consistent we can update mapping here
     * Tradeof :  writing  range* 2 registers
    */
    UPDATE_CHILDREN_MAPPING(A_LEVEL, TM_HW_A_NODE_CONFIG_FAIL);

    /*update parents  mapping - if were changed*/
    /* this action is possible also under traffic - it atomically removes unused child */
    UPDATE_PARENTS_MAPPING(A_LEVEL, parent_b_node_index, TM_HW_A_NODE_CONFIG_FAIL);
    /* Download A-Node to HW */
    if (prvSchedLowLevelSetHwNodeMapping(hndl, A_LEVEL, *a_node_index_ptr)
     || prvSchedLowLevelSet_a_node_Params(hndl, *a_node_index_ptr)
     || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, A_LEVEL, *a_node_index_ptr))
    {
        rc = TM_HW_A_NODE_CONFIG_FAIL;
        goto out;
    }
    /* Download A-node shaping parameters (from profile) to HW */
    if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, A_LEVEL, *a_node_index_ptr))
    {
        rc = TM_HW_SHAPING_PROF_FAILED;
        goto out;
    }
    /*******************************************************************************/
    /** here  node successfully created.                                           */
    /*******************************************************************************/
#ifdef RESHUFFLING_SHIELD
    {
        prvSchedLowLevelSetNodeEligPriorityFunc(hndl,B_LEVEL,parent_b_node_index);
        for (i=parent_b_node->mapping.childLo ;  i <=parent_b_node->mapping.childHi ; i++)
        {
            prvSchedLowLevelSetNodeEligPriorityFunc(hndl, A_LEVEL,i);
        }
    }
#endif
#if 0
        if (reshuffling) {
            int a;
            for (a = parent_b_node->mapping.childLo;
                 a < parent_b_node->mapping.childHi; /* Skip added node! */
                 a++) {
                int q;
                for (q = ctl->tm_a_node_array[a].mapping.childLo;
                     q <= ctl->tm_a_node_array[a].mapping.childHi;
                     q++) {
                    printf("Q%d\n", q);
                    rc = prvSchedLowLevelSetNodeEligPriorityFunc(hndl, Q_LEVEL, q);
                    assert(rc == 0);
                }
                rc = prvSchedLowLevelSetNodeEligPriorityFunc(hndl, A_LEVEL, a);
                assert(rc == 0);
            }
        }
#endif
    /* create virtual ID & add entry to dictionary */
    GENERATE_VIRTUAL_ID (A_LEVEL, *a_node_index_ptr);

out:
    if (rc)
    {
        if (rc == TM_HW_A_NODE_CONFIG_FAIL)
        {
            prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, a_params->shaping_profile_ref, A_LEVEL, *a_node_index_ptr);
            
            prvSchedSetSw_a_node_Default(node ,0); /* reset a_node parameters without mapping */
            prvSchedNodesCtlFreeANode(ctl,*a_node_index_ptr,old_node_status); /* restore node mapping & release children */
            RESTORE_PARENTS_MAPPING(parent_b_node)              /* restore parents  mapping - if any */
        }
    }
    return rc;
}


int prvCpssSchedNodesCreateANodeToBNode(PRV_CPSS_SCHED_HANDLE hndl, uint32_t parent_b_node_index,
                               struct tm_a_node_params *a_params,
                               uint32_t *a_node_index_ptr,
                               GT_BOOL explicitQueueStart,
                               GT_U32  queueStart)
{   

    GT_U32 first_queue_in_range;

    if(explicitQueueStart == GT_FALSE)
    {
            first_queue_in_range = (uint32_t)NO_PREDEFINED_RANGE;
    }
    else
    {
            first_queue_in_range = queueStart;
    }
    
    return tm_create_a_node_to_b_node_ex(hndl,parent_b_node_index, a_params, first_queue_in_range ,a_node_index_ptr);
}

/***************************************************************************
 * B-node Creation
 ***************************************************************************/

/**
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
 )
{
    int rc = 0;

    rc = prvCpssSchedNodesCreateCNodeToPort(hndl, port_index,fixedBnodeToCnodeMapping, c_params, c_node_index_ptr);
    if (rc) return rc;

    rc = prvCpssSchedNodesCreateBNodeToCNode(hndl, *c_node_index_ptr, b_params, b_node_index_ptr);
    if (rc)
    {
        prvSchedDeleteCNodeProc(hndl, *c_node_index_ptr);
    }
    return rc;
}


/**
 */
static int tm_create_b_node_to_c_node_ex(PRV_CPSS_SCHED_HANDLE hndl, uint32_t parent_c_node_index,
                               struct tm_b_node_params *b_params, uint32_t first_child_in_range,
                               uint32_t *b_node_index_ptr)
{
    struct tm_port *parent_port = NULL;
    struct tm_c_node *parent_c_node = NULL;
    struct tm_b_node *node = NULL;
    uint16_t range;
    int rc = 0;
    uint16_t i;
    tm_node_status_t old_node_status= TM_NODE_USED;
    int found= 0;
    

    UTILIZE_NEGHBORHOOD_DECLARATIONS(b_node, c_node)

    TM_CTL(ctl, hndl)

    if (b_params==NULL) return -EINVAL;
    if (b_node_index_ptr==NULL) return -EINVAL;

    CONVERT_TO_PHYSICAL(C_LEVEL,parent_c_node_index)

    


    /*******************************************************************************/
    /** Check parameters validity                                                  */
    /*******************************************************************************/
    VALIDATE_NODE_ALLOCATED_EX(c_node, parent_c_node_index, TM_CONF_C_NODE_IND_OOR, TM_CONF_C_NODE_IND_NOT_EXIST)
    if (rc) goto out;

    parent_c_node = &(ctl->tm_c_node_array[parent_c_node_index]);
    parent_port = &(ctl->tm_port_array[parent_c_node->mapping.nodeParent]);
    /* check B-node params */
    rc= prvCpssSchedShapingCheckShapingProfileValidity(hndl,b_params->shaping_profile_ref, B_LEVEL);
    if (rc) goto out;

    VALIDATE_NODE_QUANTUM_VALUE(b_params->quantum, TM_CONF_B_QUANTUM_OOR)
    if (rc) goto out;

    /* DWRR for A-nodes in B-node's range */
    for (i=0; i<8; i++)
    {
        if ((b_params->dwrr_priority[i] != TM_DISABLE) &&
            (b_params->dwrr_priority[i] != TM_ENABLE))
        {
            rc = TM_CONF_B_DWRR_PRIO_OOR;
            goto out;
        }
    }

    
    

    /* check eligible function validity */
    VALIDATE_ELIG_FUNCTION(b_params->elig_prio_func_ptr)
    
    

    /* in case of asymmetric port :
     * 1. the children range  is already defined here
     * 2. some H/W changes can be made during search free node (reshuffling , etc), which  should be rolled back if
     * no enought free pool.
     * testing for enought range can prevent this
     * For symmetric node range is defined when free node is known, but in this case no reshuffling -> no rollback
     */
    if ((parent_port->sym_mode == TM_DISABLE) /* asymetric port */ && (prvSchedRmGetMaxChunkSize(ctl->rm, A_LEVEL) < b_params->num_of_children))
    {
        rc = TM_CONF_INVALID_NUM_OF_A_NODES;
        goto out;
    }

    /*******************************************************************************/
    /** Find free B node                                                           */
    /*******************************************************************************/
    /* looking for reserved or previously deleted nodes in the range */
    for (i= (uint16_t)parent_c_node->mapping.childLo ;  i <= (uint16_t)parent_c_node->mapping.childHi ; i++)
    {
        if (ctl->tm_b_node_array[i].mapping.nodeStatus > TM_NODE_USED)
        {
            found=1;
            break;
        }
    }
    /* if (found)  i= number of node founded  */
    if ((!found) && (parent_port->sym_mode == TM_ENABLE))   /* for symmetric pors it's unrecoverable case */
    {
    }
    else /* for asymmetric ports let try additional capabilities */
    {
        if (!found)
        {
            /*try to utilize free/reserved node from right/left side */
            UTILIZE_NEGHBORHOOD(B_LEVEL, b_node, c_node, parent_c_node)
        }
/* SN only - #ifdef VIRTUAL_LAYER */
                /*try to reshuffling here */
                if (!found)
                {
                    /*for SN - delayed reshuffling
                    rc = TM_CONF_NEED_REALLOC;
                    goto out;
                    */
                    /* for future use */
                    MIN_IMPACT_RESHUFFLING(parent_c_node, parent_c_node_index, B_LEVEL, b_node, a_node, TM_HW_B_NODE_CONFIG_FAIL);
                }
/* #endif */
    }
    if ( !found)  /* not found anyway*/
    {
        rc = -ENOBUFS;
        goto out;
    }
    /**************************************************************************/
    /** free node found , try to allocate children range and set node as used */
    /**************************************************************************/
    /* calculate range */
    if (parent_port->sym_mode == TM_ENABLE) /* symetric mode under the port */
    {
        if ( i == parent_c_node->mapping.childHi)   range = (uint16_t)parent_port->children_range.last_range[A_LEVEL];/* last child in range */
        else                                        range =(uint16_t)parent_port->children_range.norm_range[A_LEVEL];
    }
    else range = b_params->num_of_children; /* asymmetric mode under the port */
    /* allocate chidrens & set node as used*/
    if (prvSchedNodesCtlAllocateReservedBNode(ctl, i, parent_c_node_index, range,(uint16_t) first_child_in_range, &old_node_status)) 
    {
        rc = TM_CONF_INVALID_NUM_OF_A_NODES;
        goto out;
    }
    /**************************************************************************/
    /** node allocated ,let update node params                                */
    /**************************************************************************/
    *b_node_index_ptr = i;
    node = &(ctl->tm_b_node_array[i]);
    /* Update B-Node SW DB */
    rc=prvCpssSchedShapingAddNodeToShapingProfile(ctl,b_params->shaping_profile_ref, B_LEVEL, *b_node_index_ptr);
    if (rc < 0) goto out;

    node->elig_prio_func = b_params->elig_prio_func_ptr;
    
 
    
    /* DWRR for B-node - update even if disabled */
    node->dwrr_quantum = b_params->quantum;

    /* DWRR for A-nodes in B-node's range */
    node->dwrr_enable_per_priority_mask = 0;
    for (i=0; i<8; i++)
        node->dwrr_enable_per_priority_mask =
            node->dwrr_enable_per_priority_mask | (b_params->dwrr_priority[i] << i);
    /**************************************************************************/
    /** node parameters are set , let update H/W                              */
    /**************************************************************************/
    /* here  parent value in registers of allocated children is inconsistent with children range of new node
     * It will be updated during creation child nodes ,  but in order to be consistent we can update mapping here
     * Tradeof :  writing  range* 2 registers
    */
    UPDATE_CHILDREN_MAPPING(B_LEVEL, TM_HW_B_NODE_CONFIG_FAIL);

    /*update parent neighborhood mapping - if were changed*/
    /* this action is possible also under traffic - it atomically removes unused child */
    UPDATE_PARENTS_MAPPING(B_LEVEL, parent_c_node_index, TM_HW_B_NODE_CONFIG_FAIL);
    /* Download B-Node to HW */
    if (prvSchedLowLevelSetHwNodeMapping(hndl, B_LEVEL, *b_node_index_ptr)
     || prvSchedLowLevelSet_b_node_Params(hndl, *b_node_index_ptr)
     || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, B_LEVEL, *b_node_index_ptr))
    {
        rc = TM_HW_B_NODE_CONFIG_FAIL;
        goto out;
    }
    /* Download B-node shaping parameters (from profile) to HW */
    if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, B_LEVEL, *b_node_index_ptr))
    {
        rc = TM_HW_SHAPING_PROF_FAILED;
        goto out;
    }
    /*******************************************************************************/
    /** here  node successfully created.                                           */
    /*******************************************************************************/
    /* create virtual ID & add entry to dictionary */
    GENERATE_VIRTUAL_ID (B_LEVEL, *b_node_index_ptr)

out:
    if (rc)
    {
        if (rc == TM_HW_B_NODE_CONFIG_FAIL)
        {
            prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, b_params->shaping_profile_ref, B_LEVEL, *b_node_index_ptr);
            /* reset node parameters without mapping */
            prvSchedSetSw_b_node_Default(node,0);
            /* restore node mapping & release children */
            prvSchedNodesCtlFreeBNode(ctl,*b_node_index_ptr,old_node_status);
            RESTORE_PARENTS_MAPPING(parent_c_node)              /* restore parent level  mapping - if any */
      }
    }
    return rc;
}

int prvCpssSchedNodesCreateBNodeToCNode(PRV_CPSS_SCHED_HANDLE hndl, uint32_t parent_c_node_index,
                               struct tm_b_node_params *b_params,
                               uint32_t *b_node_index_ptr)
{
    return  tm_create_b_node_to_c_node_ex(hndl, parent_c_node_index, b_params, (uint32_t)NO_PREDEFINED_RANGE,b_node_index_ptr);
}


/***************************************************************************
 * C-node Creation
 ***************************************************************************/

/**
 */
static int tm_create_c_node_to_port_ex(PRV_CPSS_SCHED_HANDLE hndl, uint8_t parent_port_index,
                             struct tm_c_node_params *c_params, uint32_t first_child_in_range,
                             uint32_t *c_node_index_ptr)
{
    struct tm_port  *parent_port = NULL;
    struct tm_c_node    *node = NULL;
    uint16_t range;
    int rc = 0;
    int i;
    tm_node_status_t old_node_status= TM_NODE_USED;
    

    TM_CTL(ctl, hndl)
     

    if (c_params==NULL) return -EINVAL;
    if (c_node_index_ptr==NULL) return -EINVAL;


    /*******************************************************************************/
    /** Check parameters validity                                                  */
    /*******************************************************************************/
    /* Check port validity */
    VALIDATE_NODE_ALLOCATED_EX(port, parent_port_index, TM_CONF_PORT_IND_OOR, TM_CONF_PORT_IND_NOT_EXIST)
    if (rc) goto out;

    parent_port = &(ctl->tm_port_array[parent_port_index]);

    /* C-node params */
    rc= prvCpssSchedShapingCheckShapingProfileValidity(hndl,c_params->shaping_profile_ref, C_LEVEL);
    if (rc) goto out;
    /* check validity of the Quantum value*/
    VALIDATE_NODE_QUANTUM_VALUE(c_params->quantum, TM_CONF_C_QUANTUM_OOR)
    if (rc) goto out;

    /* check validity of eligible function number */
    VALIDATE_ELIG_FUNCTION(c_params->elig_prio_func_ptr)

    /* check DWRR for B-nodes in C-node's range */
    for (i=0; i<8; i++)
    {
        if ((c_params->dwrr_priority[i] != TM_DISABLE) &&
            (c_params->dwrr_priority[i] != TM_ENABLE))
        {
            rc = TM_CONF_C_DWRR_PRIO_OOR;
            goto out;
        }
    }
#if 0 /* TBD - do we need it? */
    if (c_params->wred_cos == 0) {  /* at least one drop profile
                                     * should be applied */
        rc = TM_CONF_C_WRED_PROF_REF_OOR;
        goto out;
    }
#endif

 
    

    /* in case of creation  of C-nodes resuffling is not implemented , so it is not necessary to preliminary validate children range
     * But if "reshuffling" will be expanded to this level, it's recommended to  validate range here , as done for A & B levels
     */

    /*******************************************************************************/
    /** Find free C-node                                                           */
    /*******************************************************************************/
    /* looking for reserved or previously deleted nodes in the range */
    for (i= parent_port->mapping.childLo ;  i <= parent_port->mapping.childHi ; i++)
    {
        if (ctl->tm_c_node_array[i].mapping.nodeStatus > TM_NODE_USED) break;
    }
    if (i > parent_port->mapping.childHi )   /* all nodes are already used */
    {
        rc = -ENOBUFS;
        goto out;
    }
    /**************************************************************************/
    /** free node found , try to allocate children range and set node as used */
    /**************************************************************************/
    /* calculate range */
    if (parent_port->sym_mode == TM_ENABLE) /* symetric mode under the port */
    {
        if ( i == parent_port->mapping.childHi)     range = (uint16_t)parent_port->children_range.last_range[B_LEVEL];/* last child in range */
        else                                        range = (uint16_t)parent_port->children_range.norm_range[B_LEVEL];
    }
    else /* asymmetric mode under the port */
    {
        range = c_params->num_of_children;
    }
    /* allocate chidrens & set node as used*/
    if (prvSchedNodesCtlAllocateReservedCNode(ctl, (uint8_t)i, parent_port_index, range,(uint16_t) first_child_in_range, &old_node_status))
    {
        rc = TM_CONF_INVALID_NUM_OF_B_NODES;
        goto out;
    }
    /**************************************************************************/
    /** node allocated ,let update node params                                */
    /**************************************************************************/
    *c_node_index_ptr = i;
    node = &(ctl->tm_c_node_array[i]);
    /* Update C-node SW DB */
    rc=prvCpssSchedShapingAddNodeToShapingProfile(ctl,c_params->shaping_profile_ref, C_LEVEL, *c_node_index_ptr);
    if (rc < 0) goto out;

    node->elig_prio_func = c_params->elig_prio_func_ptr;
    
    
    
    
    /* DWRR for C-node - update even if disabled */
    node->dwrr_quantum = c_params->quantum;
    /* DWRR for B-nodes in C-node's range */
    node->dwrr_enable_per_priority_mask = 0;
    for (i=0; i<8; i++)
        node->dwrr_enable_per_priority_mask =
            node->dwrr_enable_per_priority_mask | (c_params->dwrr_priority[i] << i);

    /**************************************************************************/
    /** node parameters are set , let update H/W                              */
    /**************************************************************************/
    /* here  parent value in registers of allocated children is inconsistent with children range of new node
     * It will be updated during creation child nodes ,  but in order to be consistent we can update mapping here
     * Tradeof :  writing  range* 2 registers
    */
    UPDATE_CHILDREN_MAPPING(C_LEVEL, TM_HW_C_NODE_CONFIG_FAIL);

    /* Download C-node to HW */
    /* update node H/W */
    if (prvSchedLowLevelSetHwNodeMapping(hndl, C_LEVEL, *c_node_index_ptr)
     || prvSchedLowLevelSet_c_node_Params(hndl, *c_node_index_ptr)
     || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, C_LEVEL, *c_node_index_ptr))
    {
        rc = TM_HW_C_NODE_CONFIG_FAIL;
        goto out;
    }
    /* Download C-node shaping parameters (from profile) to HW */
    if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, C_LEVEL, *c_node_index_ptr))
    {
        rc = TM_HW_SHAPING_PROF_FAILED;
        goto out;
    }
    /*******************************************************************************/
    /** here  node successfully created.                                           */
    /*******************************************************************************/
    /* create virtual ID & add entry to dictionary */
    /*** currently not supported ********************
     GENERATE_VIRTUAL_ID (C_LEVEL, *c_node_index);
     ************************************************/

out:
    if (rc)
    {
        if (rc == TM_HW_C_NODE_CONFIG_FAIL)
        {
            prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, c_params->shaping_profile_ref, C_LEVEL, *c_node_index_ptr);
  
            prvSchedSetSw_c_node_Default(node,0);/* reset a_node parameters without mapping */
            prvSchedNodesCtlFreeCNode(ctl, *c_node_index_ptr,old_node_status); /* restore node mapping & release children */
       }
    }
    return rc;
}

int prvCpssSchedNodesCreateCNodeToPort
(
        PRV_CPSS_SCHED_HANDLE hndl, 
        uint8_t parent_port_index,
        GT_BOOL fixedBnodeToCnodeMapping,
        struct tm_c_node_params *c_params,
        uint32_t *c_node_index_ptr
 )
{
    return tm_create_c_node_to_port_ex(hndl, parent_port_index, c_params, fixedBnodeToCnodeMapping?parent_port_index:NO_PREDEFINED_RANGE,c_node_index_ptr);
}


